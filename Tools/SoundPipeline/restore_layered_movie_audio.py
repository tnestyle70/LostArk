"""Restore reviewed simultaneous Movie stems without clipping them to PCM16.

Preparation is offline: WEM -> Ogg -> FFmpeg float32 PCM -> shared linked
lookahead gain envelope -> separate float32 WAVs. No audio device is opened.
Install admits only the reviewed source timing/gain and changes assetId fields
under the Movie writer lock; publish remains a separate operation.
"""
from __future__ import annotations

import argparse
from collections import deque
import copy
import ctypes
from ctypes import wintypes
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import subprocess
import time

import numpy as np
import wwise_audio_package as wwise
import wwise_vorbis_to_ogg as vorbis

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RECIPE = Path(__file__).with_name("artist_selection_bus.recipe.json")


def sha(data):
    return hashlib.sha256(data).hexdigest()


def unique(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("Duplicate JSON key: " + key)
        result[key] = value
    return result


def parse(data):
    return json.loads(data, object_pairs_hook=unique,
        parse_constant=lambda value: (_ for _ in ()).throw(ValueError(value)))


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def read_wave(path):
    raw = path.read_bytes()
    if raw[:4] != b"RIFF" or raw[8:12] != b"WAVE":
        raise ValueError("Not RIFF WAVE: " + str(path))
    cursor, fmt, pcm = 12, None, None
    while cursor + 8 <= len(raw):
        tag, size = struct.unpack_from("<4sI", raw, cursor)
        data = raw[cursor + 8:cursor + 8 + size]
        if len(data) != size:
            raise ValueError("Truncated WAV chunk")
        if tag == b"fmt ":
            fmt = struct.unpack_from("<HHIIHH", data)
            if fmt[0] == 65534 and len(data) >= 40:
                fmt = (struct.unpack_from("<H", data, 24)[0],) + fmt[1:]
        elif tag == b"data":
            pcm = data
        cursor += 8 + size + (size & 1)
    if fmt is None or pcm is None or (fmt[0], fmt[5]) not in ((1, 16), (3, 32)):
        raise ValueError("Expected PCM16 or IEEE float32 WAV")
    kind, channels, rate, _, align, bits = fmt
    if channels != 2 or align != channels * bits // 8 or not pcm:
        raise ValueError("Expected nonempty stereo WAV")
    data = np.frombuffer(pcm, dtype="<f4" if kind == 3 else "<i2").reshape(-1, channels)
    result = data.astype(np.float64) / (32768.0 if kind == 1 else 1.0)
    if not np.isfinite(result).all():
        raise ValueError("Nonfinite WAV samples")
    return rate, result, kind


def write_float_wave(path, rate, samples):
    data = np.asarray(samples, dtype="<f4")
    if data.ndim != 2 or data.shape[1] != 2 or not np.isfinite(data).all():
        raise ValueError("Expected finite stereo samples")
    pcm = data.tobytes()
    header = (b"RIFF" + struct.pack("<I", 36 + len(pcm)) + b"WAVEfmt " +
        struct.pack("<IHHIIHH", 16, 3, 2, rate, rate * 8, 8, 32) +
        b"data" + struct.pack("<I", len(pcm)))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + pcm)


def linked_envelope(stems, rate, bus):
    """Parameter-matched offline approximation, not Wwise DSP emulation.

    Detect the maximum absolute channel of the sum, not individual voices.
    Forecast the next lookahead window; attack immediately, release with a
    one-pole gain recovery. No samples or duration are shifted by lookahead.
    """
    if not stems or len({s.shape for s in stems}) != 1 or not len(stems[0]) or rate <= 0:
        raise ValueError("Layer stems must share a nonempty sample grid")
    if not bus["stereoLinked"] or bus["ratio"] < 1 or bus["releaseMs"] <= 0 or bus["lookAheadMs"] < 0:
        raise ValueError("Unsupported linked dynamics parameters")
    pre_gain = 10 ** (bus["gainDb"] / 20)
    mix = np.sum(stems, axis=0) * pre_gain
    peak = np.max(np.abs(mix), axis=1)
    window = round(rate * bus["lookAheadMs"] / 1000)
    future = np.empty_like(peak)
    maxima = deque()
    for frame in range(len(peak) - 1, -1, -1):
        while maxima and maxima[0] > frame + window:
            maxima.popleft()
        while maxima and peak[maxima[-1]] <= peak[frame]:
            maxima.pop()
        maxima.append(frame)
        future[frame] = peak[maxima[0]]
    threshold = 10 ** (bus["thresholdDb"] / 20)
    required = np.ones_like(future)
    loud = future > threshold
    required[loud] = (future[loud] / threshold) ** (1 / bus["ratio"] - 1)
    release = math.exp(-1 / (rate * bus["releaseMs"] / 1000))
    gain = 1.0
    envelope = np.empty_like(required)
    for frame, target in enumerate(required):
        gain = float(target) if target < gain else release * gain + (1 - release) * float(target)
        envelope[frame] = gain * pre_gain * 10 ** (bus["outputDb"] / 20)
    restored = [stem * envelope[:, None] for stem in stems]
    if not np.isfinite(envelope).all():
        raise ValueError("Nonfinite linked dynamics result")
    return restored, envelope


def asset_path(value):
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or "\\" in value or ":" in value or path.parts[0] != "Sound" or path.suffix != ".wav":
        raise ValueError("Invalid sound asset ID: " + value)
    return Path(*path.parts)


def admitted_rows(document, recipe):
    rows = [row for row in document["templates"] if row["sequenceId"] == recipe["sequenceId"]]
    if len(rows) != 1:
        raise ValueError("Movie sequence stable identity must occur once")
    sounds = rows[0].get("soundTracks", [])
    if {row["soundTrackId"] for row in sounds} != {s["soundTrackId"] for s in recipe["stems"]} or len(sounds) != len(recipe["stems"]):
        raise ValueError("Current layer set differs from reviewed simultaneous pair")
    for stem in recipe["stems"]:
        row = next(s for s in sounds if s["soundTrackId"] == stem["soundTrackId"])
        if row["assetId"] not in (stem["originalAssetId"], stem["restoredAssetId"]):
            raise ValueError("Current sound asset changed: " + stem["soundTrackId"])
        for key, value in recipe["expectedTrack"].items():
            fallback = {"sourceStartMs": 0, "loopToDuration": False, "volume": 1}.get(key)
            if row.get(key, fallback) != value:
                raise ValueError("Current relative sound timing/gain changed: " + key)
    return sounds


def patch_document(original, recipe):
    """Change only stable sound assetId values and the required root revision."""
    bom = b"\xef\xbb\xbf" if original.startswith(b"\xef\xbb\xbf") else b""
    text = original[len(bom):].decode("utf-8")
    before = parse(text)
    expected = copy.deepcopy(before)
    rows = admitted_rows(expected, recipe)
    spans, stack, quoted, escaped = [], [], False, False
    for at, char in enumerate(text):
        if quoted:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                quoted = False
        elif char == '"':
            quoted = True
        elif char == "{":
            stack.append(at)
        elif char == "}":
            spans.append((stack.pop(), at + 1))
    def owner(at):
        return min((span for span in spans if span[0] < at < span[1]), key=lambda span: span[1] - span[0])
    changes = []
    for stem in recipe["stems"]:
        row = next(s for s in rows if s["soundTrackId"] == stem["soundTrackId"])
        if row["assetId"] == stem["restoredAssetId"]:
            continue
        matches = list(re.finditer(r'"soundTrackId"\s*:\s*' + re.escape(json.dumps(stem["soundTrackId"])), text))
        if len(matches) != 1:
            raise ValueError("Ambiguous sound stable ID byte span")
        begin, end = owner(matches[0].start())
        if parse(text[begin:end]) != row:
            raise ValueError("Sound row byte/semantic mismatch")
        fields = [m for m in re.finditer(r'"assetId"\s*:\s*', text[begin:end])
                  if owner(begin + m.start()) == (begin, end)]
        if len(fields) != 1:
            raise ValueError("Ambiguous assetId field")
        at = begin + fields[0].end()
        value, length = json.JSONDecoder().raw_decode(text[at:])
        if value != stem["originalAssetId"]:
            raise ValueError("Sound byte value changed")
        changes.append((at, at + length, json.dumps(stem["restoredAssetId"])))
        row["assetId"] = stem["restoredAssetId"]
    if not changes:
        return original
    root = max(spans, key=lambda span: span[1] - span[0])
    revisions = [m for m in re.finditer(r'"revision"\s*:\s*', text) if owner(m.start()) == root]
    revision = before["revision"]
    if len(revisions) != 1 or type(revision) is not int or not 1 <= revision < 0xFFFFFFFF:
        raise ValueError("Invalid source revision")
    at = revisions[0].end()
    value, length = json.JSONDecoder().raw_decode(text[at:])
    if value != revision:
        raise ValueError("Revision byte mismatch")
    changes.append((at, at + length, str(revision + 1)))
    expected["revision"] = revision + 1
    for begin, end, value in sorted(changes, reverse=True):
        text = text[:begin] + value + text[end:]
    output = bom + text.encode("utf-8")
    if parse(output) != expected:
        raise ValueError("Patch changed an unrelated field")
    return output


def prepare(args, recipe):
    output = args.out.resolve()
    if not output.is_relative_to(ROOT / "out"):
        raise ValueError("Candidates must remain under repository out/")
    output.mkdir(parents=True, exist_ok=True)
    source = ROOT / recipe["worldDocument"]
    original = source.read_bytes()
    admitted_rows(parse(original), recipe)
    packages = wwise.load_packages(wwise.find_packages(args.package_root, recipe["packageFilter"]))
    objects = wwise.merged_objects(packages)
    init = wwise.load_packages(wwise.find_packages(args.package_root, "INIT"))
    if [sha(p.payload(b)) for p in init for b in p.banks] != recipe["reviewedInitBankSha256"]:
        raise ValueError("INIT defaults/bus source bank changed")
    all_objects = {**objects, **wwise.merged_objects(init)}
    for identity, reviewed in recipe["reviewedHirc"].items():
        if all_objects.get(int(identity)) != (reviewed["kind"], bytes.fromhex(reviewed["payloadHex"])):
            raise ValueError("Source Wwise graph changed: " + identity)
    sources = recipe.get("sources", recipe["stems"])
    media, missing = wwise.resolve_event(recipe["event"], objects)
    if missing or set(media) != {s["mediaId"] for s in sources}:
        raise ValueError("Source event is no longer the reviewed pair")
    ffmpeg = shutil.which(args.ffmpeg)
    if not ffmpeg:
        raise ValueError("Float-preserving FFmpeg decoder is required")
    stems, receipts, rate = [], [], None
    for stem in sources:
        matches = [(p, p.stream_by_id(stem["mediaId"])) for p in packages if p.stream_by_id(stem["mediaId"])]
        if len(matches) != 1:
            raise ValueError("Source media has no unique package")
        package, entry = matches[0]
        raw = output / "Raw" / f'{recipe["event"]}__{stem["mediaId"]}.wem'
        raw.parent.mkdir(parents=True, exist_ok=True)
        raw.write_bytes(package.payload(entry))
        if sha(raw.read_bytes()) != stem["reviewedWemSha256"]:
            raise ValueError("Reviewed source media changed")
        vorbis.convert(raw, raw.with_suffix(".ogg"), vorbis.CODEBOOK_LIBRARY)
        decoded = raw.with_suffix(".float.wav")
        subprocess.run([ffmpeg, "-nostdin", "-hide_banner", "-loglevel", "error", "-y", "-i",
            str(raw.with_suffix(".ogg")), "-c:a", "pcm_f32le", str(decoded)], check=True)
        sample_rate, samples, kind = read_wave(decoded)
        if kind != 3 or (rate is not None and sample_rate != rate):
            raise ValueError("Float decode format mismatch")
        rate = sample_rate
        old = ROOT / "Client/Bin/Resources" / asset_path(stem["originalAssetId"])
        if sha(old.read_bytes()) != stem["reviewedOriginalAssetSha256"]:
            raise ValueError("Original resource changed since source review")
        old_rate, old_samples, old_kind = read_wave(old)
        native_frames = len(samples)
        tail_pad = stem.get("reviewedTailPaddingFrames", 0)
        if tail_pad:
            if (type(tail_pad) is not int or not 0 < tail_pad <= 1 or old_rate != rate or
                    len(samples) + tail_pad != len(old_samples) or np.any(old_samples[-tail_pad:])):
                raise ValueError("Reviewed zero-only terminal padding no longer matches")
            samples = np.pad(samples, ((0, tail_pad), (0, 0)))
        if old_rate != rate or old_samples.shape != samples.shape:
            raise ValueError("Restoration would shift source sample grid")
        receipts.append(dict(**stem, sourcePackage=str(package.path), packageSha256=sha(package.path.read_bytes()),
            sourceLanguage=entry["language"], wemSha256=sha(raw.read_bytes()), floatSourceSha256=sha(decoded.read_bytes()),
            originalAssetSha256=sha(old.read_bytes()), originalFormat=old_kind,
            nativeSourceFrames=native_frames, terminalZeroPaddingFrames=tail_pad,
            originalRailSamples=int(np.sum((old_samples == -1) | (old_samples == 32767 / 32768))),
            sourcePeak=float(np.max(abs(samples))), sourceOverFullScale=int(np.sum(abs(samples) > 1))))
        fade_frames = round(rate * recipe.get("fadeInMs", 0) / 1000)
        if fade_frames:
            samples = samples * np.minimum(np.arange(len(samples)) / fade_frames, 1.0)[:, None]
        stems.append(samples)
    restored, envelope = linked_envelope(stems, rate, recipe["bus"])
    if recipe.get("mixToSingleTrack", False):
        if len(recipe["stems"]) != 1:
            raise ValueError("Single-track mix requires exactly one output contract")
        rendered = [np.sum(restored, axis=0)]
        comparison_sources = [np.sum(stems, axis=0)]
    else:
        rendered, comparison_sources = restored, stems
    if len(rendered) != len(recipe["stems"]):
        raise ValueError("Source/output stem count mismatch")
    installed = []
    rendered_receipts = []
    for specification, samples in zip(recipe["stems"], rendered):
        record = dict(specification, sourceMediaIds=[s["mediaId"] for s in sources])
        target = output / "Resources" / asset_path(record["restoredAssetId"])
        write_float_wave(target, rate, samples)
        sr, readback, kind = read_wave(target)
        if sr != rate or kind != 3 or not np.array_equal(readback.astype(np.float32), samples.astype(np.float32)):
            raise ValueError("Float WAV write changed samples")
        installed.append(readback)
        record.update(candidate=str(target), sha256=sha(target.read_bytes()), sampleRate=rate,
            frames=len(samples), peak=float(np.max(abs(readback))))
        rendered_receipts.append(record)
    final_mix = np.sum(installed, axis=0)
    if np.max(abs(final_mix)) >= 1 or not np.isfinite(final_mix).all():
        raise ValueError("Restored mix still clips")
    same_envelope_error = max(float(np.max(abs(saved - original_stem * envelope[:, None])))
        for saved, original_stem in zip(installed, comparison_sources))
    if same_envelope_error > 1e-7:
        raise ValueError("Stems did not preserve the common linked envelope")
    recipe_hash = sha(args.recipe.read_bytes())
    receipt = dict(schema="lostark.layered-movie-audio-candidate.v1", recipe=str(args.recipe.resolve()),
        recipeSha256=recipe_hash, sourceDocumentSha256=sha(original), decoder=str(ffmpeg),
        decoderVersion=subprocess.check_output([ffmpeg, "-version"], text=True).splitlines()[0],
        scope=recipe["scope"], decodedSources=receipts, renderedStems=rendered_receipts, bus=recipe["bus"],
        fadeInMs=recipe.get("fadeInMs", 0), mixToSingleTrack=recipe.get("mixToSingleTrack", False),
        nativeDspBitIdentical=False, processing="Bus gain then stereo-linked sum peak detector; future-window maximum; instantaneous attack; one-pole gain release; output gain. Recipe supplies all values. Offline lookahead is latency-compensated with unchanged sample count.",
        statistics=dict(frames=len(final_mix),sampleRate=rate, durationMs=len(final_mix)*1000/rate,
            restoredSumPeak=float(np.max(abs(final_mix))), restoredClippedSamples=int(np.sum(abs(final_mix)>=1)),
            sourceFloatSumPeak=float(np.max(abs(np.sum(stems,axis=0)))),sameEnvelopeMaxError=same_envelope_error),
        liveFilesChanged=False, publisherExecuted=False)
    write_json(output / "candidate-receipt.json", receipt)
    (output / "world.candidate.json").write_bytes(patch_document(original, recipe))
    print(json.dumps(receipt["statistics"]))


def winapi():
    api = ctypes.WinDLL("kernel32", use_last_error=True)
    api.CreateFileW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p,
        wintypes.DWORD, wintypes.DWORD, wintypes.HANDLE]
    api.CreateFileW.restype = wintypes.HANDLE
    api.CloseHandle.argtypes = [wintypes.HANDLE]
    api.ReplaceFileW.argtypes = [wintypes.LPCWSTR, wintypes.LPCWSTR, wintypes.LPCWSTR,
        wintypes.DWORD, ctypes.c_void_p, ctypes.c_void_p]
    api.ReplaceFileW.restype = wintypes.BOOL
    api.MoveFileExW.argtypes = [wintypes.LPCWSTR, wintypes.LPCWSTR, wintypes.DWORD]
    api.MoveFileExW.restype = wintypes.BOOL
    return api


def write_temp(path, data):
    with path.open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())


def install(args, recipe):
    output = args.out.resolve()
    receipt = parse((output / "candidate-receipt.json").read_bytes())
    if receipt["recipeSha256"] != sha(args.recipe.read_bytes()):
        raise ValueError("Recipe changed since preparation")
    expected_stems = {s["soundTrackId"]: s for s in recipe["stems"]}
    if len(receipt["renderedStems"]) != len(expected_stems):
        raise ValueError("Candidate stem count changed")
    seen = set()
    for stem in receipt["renderedStems"]:
        identity = stem["soundTrackId"]
        if identity in seen or identity not in expected_stems:
            raise ValueError("Candidate stable stem set changed")
        seen.add(identity)
        if any(stem.get(k) != v for k, v in expected_stems[identity].items()):
            raise ValueError("Candidate stem source contract changed")
        old = ROOT / "Client/Bin/Resources" / asset_path(stem["originalAssetId"])
        if sha(old.read_bytes()) != stem["reviewedOriginalAssetSha256"]:
            raise ValueError("Original WAV changed since preparation")
    target = ROOT / recipe["worldDocument"]
    api = winapi()
    lock = api.CreateFileW(str(target) + ".writer.lock", 0xC0000000, 0, None, 1, 0x04000100, None)
    if lock == ctypes.c_void_p(-1).value:
        raise RuntimeError("Another Movie writer is active")
    suffix = f".movie-audio-{os.getpid()}-{time.time_ns()}"
    temporary = Path(str(target) + suffix + ".tmp")
    backup = output / (target.name + suffix + ".bak")
    created, temps, committed, candidate = [], [], False, None
    try:
        original = target.read_bytes()
        candidate = patch_document(original, recipe)
        entries = []
        for stem in receipt["renderedStems"]:
            source = Path(stem["candidate"]).resolve()
            if not source.is_relative_to(output / "Resources") or sha(source.read_bytes()) != stem["sha256"]:
                raise ValueError("Candidate WAV changed")
            for root in (ROOT / "Client/Bin/Resources", args.mirror.resolve()):
                destination = root / asset_path(stem["restoredAssetId"])
                if not destination.resolve().is_relative_to(root.resolve()):
                    raise ValueError("Resource path escapes root")
                entries.append(dict(source=str(source),target=str(destination),assetId=stem["restoredAssetId"],sha256=stem["sha256"]))
                if destination.exists():
                    if sha(destination.read_bytes()) != stem["sha256"]:
                        raise ValueError("Existing restored asset differs: " + str(destination))
                    continue
                destination.parent.mkdir(parents=True, exist_ok=True)
                staged = Path(str(destination) + suffix + ".tmp")
                temps.append(staged)
                write_temp(staged, source.read_bytes())
                if not api.MoveFileExW(str(staged), str(destination), 8):
                    raise ctypes.WinError(ctypes.get_last_error())
                created.append(dict(path=destination, inode=destination.stat().st_ino,
                    sha256=stem["sha256"], assetId=stem["restoredAssetId"]))
        if target.read_bytes() != original:
            raise ValueError("World document changed before replacement")
        if candidate != original:
            write_temp(temporary, candidate)
            if target.read_bytes() != original:
                raise ValueError("World document changed while staging")
            if not api.ReplaceFileW(str(target), str(temporary), str(backup), 0, None, None):
                raise ctypes.WinError(ctypes.get_last_error())
            committed = True
            if backup.read_bytes() != original or target.read_bytes() != candidate:
                raise ValueError("Concurrent source change during replacement")
        for entry in entries:
            if sha(Path(entry["target"]).read_bytes()) != entry["sha256"]:
                raise ValueError("Installed WAV changed during verification")
        result = dict(result="PASS", resources=entries, baselineSha256=sha(original), installedSha256=sha(candidate),
            backupPath=str(backup) if committed else None, assetIdsOnly=True, unrelatedBytesPreserved=True,
            writerLock=True, atomicReplacement=True, publisherExecuted=False)
        write_json(output / "install-receipt.json", result)
        print(json.dumps(result, indent=2))
    except Exception as error:
        rolled_back = not committed
        if committed and target.read_bytes() == candidate and backup.exists():
            rolled_back = bool(api.ReplaceFileW(str(target), str(backup), None, 0, None, None))
        live = target.read_bytes()
        for item in reversed(created):
            path = item["path"]
            if (path.exists() and path.stat().st_ino == item["inode"] and sha(path.read_bytes()) == item["sha256"]
                    and item["assetId"].encode() not in live):
                path.unlink()
        write_json(output / "failed-install-receipt.json", dict(error=str(error), sourceRolledBack=rolled_back))
        raise
    finally:
        for path in [temporary] + temps:
            if path.exists():
                path.unlink()
        api.CloseHandle(lock)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("prepare", "install"))
    parser.add_argument("--recipe", type=Path, default=DEFAULT_RECIPE)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, default=wwise.DEFAULT_PACKAGE_ROOT)
    parser.add_argument("--ffmpeg", default="ffmpeg")
    parser.add_argument("--mirror", type=Path, default=ROOT.parent / "GBResources")
    args = parser.parse_args()
    recipe = parse(args.recipe.read_bytes())
    if recipe["schema"] != "lostark.layered-movie-audio-restoration.v1":
        raise ValueError("Unsupported recipe schema")
    (prepare if args.mode == "prepare" else install)(args, recipe)


if __name__ == "__main__":
    main()
