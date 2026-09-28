#!/usr/bin/env python3
"""Restore original projectile event variants into a reviewable out directory.

The native bank tree owns gain, layers, random weights and the source loop cycle.
This utility does not edit gameplay/Composition or publish. --install atomically
merges only its catalog keys and copies only verified candidate WAVs to mirrors.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import time
import uuid

import build_kouku_sound_candidates as shared
import wwise_audio_package as wwise
import wwise_vorbis_to_ogg as vorbis

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "KoukuSaydonPipeline"))
import install_raid_candidate as atomic

ROOT = Path(__file__).resolve().parents[2]
EVENTS = [
    "G_KoukuSatan1_Attack33_ProjExp1", "G_KoukuSatan1_Attack11_ProjExp1",
    "G_Satan1_Attack05_Proj1", "G_Satan1_Attack05_ProjExp1",
    "G_Satan1_Attack06_ProjExp1", "G_KoukuSatan1_Attack23_Proj1",
    "G_KoukuSatan1_Attack23_Proj2", "G_KoukuSatan1_Attack10_Proj1",
    "G_KoukuSatan1_Attack10_Proj2", "G_KoukuSatan1_Attack10_Proj3",
    "G_KoukuSatan1_Attack10_ProjExp1",
]


def save(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def read_payload(path, offset, size):
    with path.open("rb") as stream:
        assert stream.read(4) == wwise.CONTAINER_MAGIC, path
        stream.seek(4 + offset)
        return wwise.decrypt(stream.read(size), offset)


def package_index(path):
    prefix = read_payload(path, 0, 28)
    assert prefix[:4] == b"AKPK", path
    _, _, language_size, bank_size, stream_size, _ = struct.unpack_from("<6I", prefix, 4)
    header = read_payload(path, 0, 28 + language_size + bank_size + stream_size)

    def lut(offset):
        rows = []
        for index in range(struct.unpack_from("<I", header, offset)[0]):
            identity, block, size, start, language = struct.unpack_from("<5I", header, offset + 4 + 20 * index)
            rows.append(dict(id=identity, offset=start * (block or 1), size=size, language=language))
        return rows

    return dict(path=str(path), name=wwise.decode_package_name(path.stem.split(".")[0]),
                banks=lut(28 + language_size), streams=lut(28 + language_size + bank_size))


def extract(out, package_root, events, wwiser):
    packages = [package_index(path) for path in wwise.find_packages(package_root, "SOUND_MOB_GLOBAL3")]
    objects, origins, bank_data, streams = {}, {}, {}, {}
    targets = {wwise.fnv1_32(event): event for event in events}
    for package in packages:
        path = Path(package["path"])
        for row in package["streams"]:
            streams.setdefault(row["id"], (path, row))
        for row in package["banks"]:
            blob = read_payload(path, row["offset"], row["size"])
            found = wwise.hirc_objects(blob)
            if not targets.keys() & found.keys():
                continue
            objects.update(found)
            bank_data[row["id"]] = blob
            for identity in targets.keys() & found.keys():
                origins[identity] = dict(package=str(path), packageName=package["name"], bank=row)
    records = []
    decoder = vorbis.open_wav_decoder()
    if decoder is None:
        raise RuntimeError("FMOD offline decoder unavailable")
    try:
        for event in events:
            identity = wwise.fnv1_32(event)
            media, unresolved = wwise.resolve_event(event, objects)
            if unresolved or not media:
                raise RuntimeError((event, "unresolved native event", unresolved))
            record = dict(event=event, eventId=identity, source=origins[identity], media=[])
            for media_id in media:
                package, entry = streams[media_id]
                wem = out / "decoded" / f"{media_id}.wem"
                wem.parent.mkdir(parents=True, exist_ok=True)
                data = read_payload(package, entry["offset"], entry["size"])
                wem.write_bytes(data)
                vorbis.convert(wem, wem.with_suffix(".ogg"))
                if not vorbis.write_wav(decoder, wem.with_suffix(".ogg")):
                    raise RuntimeError((event, media_id, "decode failed"))
                record["media"].append(dict(mediaId=media_id, package=str(package), entry=entry,
                                            wemSha256=hashlib.sha256(data).hexdigest()))
            records.append(record)
    finally:
        decoder.close()
    bank_paths = []
    for identity, blob in bank_data.items():
        path = out / "banks" / f"{identity}.bnk"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(blob)
        bank_paths.append(path)
    names = out / "banks" / "wwnames.txt"
    names.write_text("\n".join(events) + "\n", encoding="utf-8")
    cmd = [sys.executable, str(wwiser), "-d", "xml", "-dn", str(out / "native-bank"),
           "-g", "-gra", "-gwd", "-gv", "1.0", "-go", str(out / "native-txtp"),
           "-gw", str(out / "decoded"), "-nl", str(names)]
    cmd += [str(path) for path in bank_paths] + ["-gf"] + [str(identity) for identity in targets]
    result = subprocess.run(cmd, capture_output=True)
    (out / "wwiser.log").write_bytes(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError("wwiser failed: " + str(result.returncode))
    save(out / "source.json", records)
    return records


def render(out, records, decoder):
    from scipy.io import wavfile
    media = {}
    for path in (out / "decoded").glob("*.wav"):
        rate, samples = wavfile.read(path)
        media[int(path.stem)] = dict(installedPath=None, sourcePath=path.resolve(),
                                    sha256=shared.digest(path), durationSeconds=len(samples) / rate)
    by_id = {record["eventId"]: record for record in records}
    weights = shared.BankWeights(out)
    weights.paths = {path.name: path for path in (out / "banks").glob("*.bnk")}
    selected = {}
    for path in sorted((out / "native-txtp").glob("*.txtp")):
        text = path.read_text(encoding="utf-8-sig")
        match = re.search(r"CAkEvent\[\d+\] (\d+)", text)
        if match and int(match[1]) in by_id:
            selected.setdefault(int(match[1]), (path, text))
    catalog, rows, layer_recipes = {}, [], {}
    for event_id, record in by_id.items():
        path, text = selected[event_id]
        text = re.sub(r"(group\s*=\s*-R\d+)>-", r"\g<1>>1", text)
        probabilities = weights.resolve(record["event"], text)
        source_loop = "#@loop" in text or "##loop" in text or bool(re.search(r"#l\s|#E(?:\s|$)", text))
        # A single original cycle is retained for lifetime-owned runtime looping.
        layout_text = text.replace("#@loop", "").replace("##loop", "")
        layout_text = re.sub(r"#E(?:\s|$)", "", layout_text)
        # Native group delays remain in the rendered TXTP. They do not change
        # the selected media set used to validate branch probabilities.
        layout_text = re.sub(r"(group[^\n]+?)\s+#p\s+[0-9.]+", r"\1", layout_text)
        layouts = shared.random_layouts(layout_text, media)
        denominator = math.lcm(*(value.denominator for value in probabilities.values()))
        if denominator > 10000:
            raise RuntimeError("native random weight resolution too large")
        assets, seen = [], set()
        for ordinal, (layout, _) in enumerate(layouts, 1):
            # Retain native loop tags for the offline renderer, but resolve every
            # independent random node using the same branch assignment as layout.
            choices = re.findall(r"group\s*=\s*-R\d+>(\d+)", layouts[ordinal - 1][1])
            choice_iter = iter(choices)
            variant = re.sub(r"(group\s*=\s*-R\d+)>\d+", lambda m: m[1] + ">" + next(choice_iter), text)
            key = tuple(sorted(voice[0] for voice in layout[1]))
            if key not in probabilities:
                raise RuntimeError((record["event"], "unproven branch", key))
            rendered = {}
            asset, duration = shared.render_source_txtp(path, variant, media, out, rendered, decoder,
                                                         bounded_window_ms=1 if source_loop else 0)
            target = f"Sound/KoukuSaton/Events/{record['event']}.variant{ordinal:02}.wav"
            destination = out / "Resources" / target
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(out / "Resources" / asset, destination)
            probability = probabilities[key]
            # Identical media sets with different timing/gain are unexpected here.
            if key in seen:
                raise RuntimeError((record["event"], "duplicate media-set layout"))
            seen.add(key)
            assets += [target] * int(probability * denominator)
            evidence = rendered[asset]
            evidence.update(event=record["event"], eventId=event_id, assetId=target, durationMs=duration,
                            probability=str(probability), sourceLoops=source_loop, voices=layout[1])
            rows.append(evidence)
        if seen != set(probabilities):
            raise RuntimeError((record["event"], "native branch coverage incomplete"))
        catalog["s_mob_g_koukusatan1." + record["event"].lower()] = assets
        if record["event"] in {"G_KoukuSatan1_Attack10_Proj1", "G_KoukuSatan1_Attack10_Proj2", "G_KoukuSatan1_Attack10_Proj3"}:
            # Native saws have a one-shot birth and independently delayed loop
            # layers. World tracks own each layer so birth is never repeated.
            components, pending = [], []
            for line in text.splitlines():
                if re.match(r"\s*\?\d+\.wem", line):
                    pending.append(line)
                elif re.match(r"\s*group\s*=\s*-R", line):
                    delay = re.search(r"#p\s+([0-9.]+)", line)
                    components.append((float(delay[1]) if delay else 0.0, "#@loop" in line, pending))
                    pending = []
            recipe = []
            for layer_index, (delay, loop, alternatives) in enumerate(sorted(components), 1):
                layer_assets = []
                for ordinal, line in enumerate(alternatives, 1):
                    rendered = {}
                    asset, duration = shared.render_source_txtp(path, line + "\n", media, out, rendered, decoder)
                    target = f"Sound/KoukuSaton/Events/{record['event']}.layer{layer_index}.variant{ordinal:02}.wav"
                    destination = out / "Resources" / target
                    shutil.copyfile(out / "Resources" / asset, destination)
                    evidence = rendered[asset]
                    evidence.update(event=record["event"], eventId=event_id, assetId=target, durationMs=duration,
                                    sourceLoops=loop, sourceLayer=layer_index, sourceDelayMs=round(delay * 1000))
                    rows.append(evidence)
                    layer_assets.append(target)
                recipe.append(dict(layer=layer_index, startMs=round(delay * 1000), loopToDuration=loop, assets=layer_assets))
            layer_recipes[record["event"]] = recipe
    save(out / "catalog-additions.json", catalog)
    save(out / "layer-recipes.json", layer_recipes)
    save(out / "render-receipt.json", dict(variants=rows, playlists=weights.playlists,
         loopPolicy="One native cycle; runtime owner must opt into looping and stop at its lifetime.",
         randomPolicy="Native weights preserved; current cue selection is stable per occurrence."))
    return catalog, rows


def install(out, additions, rows, mirrors):
    catalog_path = ROOT / "Data/Sound/CharacterSoundCatalog.json"
    before = catalog_path.read_bytes()
    catalog = json.loads(before.decode("utf-8-sig"))
    bucket = catalog["classes"].setdefault("KoukuSaydon", {})
    for event, assets in additions.items():
        if event in bucket and bucket[event] != assets:
            raise RuntimeError("Existing catalog event changed: " + event)
        bucket[event] = assets
    installs, pending = [], []
    identity = uuid.uuid4().hex
    for row in rows:
        relative = atomic.relative(row["assetId"])
        if not relative.startswith("Sound/"):
            raise ValueError(relative)
        source = atomic.bounded(out / "Resources", relative)
        if shared.digest(source) != row["sha256"]:
            raise RuntimeError("Candidate WAV hash changed")
        for root in mirrors:
            dest = atomic.bounded(root, relative)
            current = atomic.sha(dest)
            if current not in {None, row["sha256"]}:
                raise RuntimeError("Existing WAV collision: " + str(dest))
            pending.append((source, dest, row["sha256"]))
    backup = out / "backups" / f"CharacterSoundCatalog.{time.time_ns()}.json"
    backup.parent.mkdir(parents=True, exist_ok=True)
    backup.write_bytes(before)
    data = (json.dumps(catalog, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    transaction = out / "install-candidates" / identity
    transaction.mkdir(parents=True)
    staged_catalog = transaction / "CharacterSoundCatalog.json"
    staged_catalog.write_bytes(data)
    save(transaction / "manifest.json", dict(catalogSha256=shared.digest(staged_catalog), events=list(additions)))
    entry = atomic.Entry("Data/Sound/CharacterSoundCatalog.json", staged_catalog, catalog_path,
                         hashlib.sha256(before).hexdigest(), shared.digest(staged_catalog), "data")
    plan = atomic.Plan(ROOT, transaction, shared.digest(transaction / "manifest.json"), [entry],
                       dict(ready=True, conflicts=[], files=[], media=[]))
    created, rollback, temporaries = [], [], []
    report = dict(catalogBeforeSha256=hashlib.sha256(before).hexdigest(), backup=str(backup),
                  events=list(additions), installs=installs, rollback=rollback, status="preparing")
    try:
        for source, dest, expected in pending:
            current = atomic.sha(dest)
            if current == expected:
                installs.append(dict(path=str(dest), sha256=expected, state="already-installed"))
                continue
            if current is not None:
                raise RuntimeError("Concurrent WAV change preserved: " + str(dest))
            dest.parent.mkdir(parents=True, exist_ok=True)
            staged = dest.with_name("." + dest.name + ".audio-" + identity + ".stage")
            temporaries.append(staged)
            atomic.durable_copy(source, staged)
            if shared.digest(staged) != expected:
                raise RuntimeError("Staged WAV hash changed: " + str(dest))
            try:
                # Unlike replace, hard-link creation cannot overwrite a late writer.
                os.link(staged, dest)
                created.append((dest, expected))
                state = "installed"
            except FileExistsError:
                if atomic.sha(dest) != expected:
                    raise RuntimeError("Concurrent WAV creation preserved: " + str(dest))
                state = "concurrent-identical"
            staged.unlink()
            installs.append(dict(path=str(dest), sha256=expected, state=state))
        # Reuse the reviewed installer, including displaced-byte verification and
        # rollback races. Its no-op entries do not rewrite identical saved bytes.
        catalog_report = atomic.install(plan)
        report["catalogTransaction"] = catalog_report
        if catalog_report["status"] != "installed":
            raise RuntimeError("Catalog transaction failed: " + str(catalog_report.get("error", catalog_report["status"])))
        report.update(status="installed", catalogAfterSha256=shared.digest(catalog_path))
    except BaseException as error:
        report.update(status="failed", error=str(error))
        for dest, expected in reversed(created):
            result = dict(path=str(dest))
            try:
                if atomic.sha(dest) != expected:
                    result["status"] = "preserved-concurrent-edit"
                else:
                    captured = dest.with_name("." + dest.name + ".audio-" + identity + ".capture")
                    os.rename(dest, captured)
                    if shared.digest(captured) == expected:
                        captured.unlink()
                        result["status"] = "removed-owned-new-file"
                    else:
                        # Preserve a writer that raced with our final hash check.
                        recovery = transaction / (uuid.uuid4().hex + ".captured")
                        atomic.durable_copy(captured, recovery)
                        try:
                            os.link(captured, dest)
                        except FileExistsError:
                            pass
                        captured.unlink()
                        result.update(status="preserved-concurrent-edit", recoverySource=str(recovery))
            except OSError as failure:
                result.update(status="recovery-required", error=str(failure))
            rollback.append(result)
        raise
    finally:
        for staged in temporaries:
            if staged.exists():
                staged.unlink()
        save(out / "install-receipt.json", report)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, default=wwise.DEFAULT_PACKAGE_ROOT)
    parser.add_argument("--wwiser", type=Path, required=True)
    parser.add_argument("--vgmstream", type=Path, default=shared.VGMSTREAM)
    parser.add_argument("--event", action="append")
    parser.add_argument("--install", action="store_true")
    parser.add_argument("--mirror", type=Path, action="append", default=[])
    parser.add_argument("--reuse-extraction", action="store_true")
    args = parser.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    records = shared.read(out / "source.json") if args.reuse_extraction else extract(
        out, args.package_root, args.event or EVENTS, args.wwiser.resolve())
    additions, rows = render(out, records, args.vgmstream)
    if args.install:
        install(out, additions, rows, [ROOT / "Client/Bin/Resources", *[p.resolve() for p in args.mirror]])
    print(json.dumps(dict(events=len(additions), variants=len(rows), installed=args.install)))


if __name__ == "__main__":
    main()
