#!/usr/bin/env python3
"""Prepare source-backed Kouku SOUND additions without writing live authoring data.

Action LOA notify -> exact animation occurrence source window -> existing SOUND
presentation. Matinee AkEvent keys use the same source clock as the installed
camera/world animation. Wwiser's finite TXTP variants preserve layers, sequence,
delay and gain; unsupported control/state/continuous trees stay explicit holdouts.
No Client, server, audio device, publisher or live Resources mutation is used.
"""
from __future__ import annotations

import argparse
import collections
import copy
import hashlib
import itertools
import json
import math
from pathlib import Path
import re
import shutil
import subprocess
import struct
import sys
import wave
from fractions import Fraction

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = Path("C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829")
COMPOSITION = Path("Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json")
CATALOG = Path("Data/Sound/CharacterSoundCatalog.json")
VGMSTREAM = Path("C:/Users/user/Desktop/LOL_Tools/vgmstream/vgmstream-cli.exe")


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def stamp(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()[:20]


class Holdout(ValueError):
    pass


def txpt_layout(text, media):
    """Read only an explicit finite subset; reject every unknown directive.

    Wwiser expands random alternatives into separate TXTPs. Each file selects
    exactly one R child; an L group keeps all children and S concatenates them.
    Each stack node is (duration seconds, [(media ID, start seconds, gain)]).
    """
    if re.search(r"CAkSwitchCntr|CAkMusic|StateChunk|\[Sequence\]", text):
        raise Holdout("state-switch-or-music-tree")
    nodes = []
    for original in text.splitlines():
        line = original.strip()
        if not line or line.startswith("#"):
            continue
        if "#@loop" in line or "##loop" in line or "#E" in line or "#l " in line:
            raise Holdout("continuous-or-loop-txtp")
        line = line.split("##", 1)[0].replace("#@layer-v", "").strip()
        group = re.fullmatch(r"group\s*=\s*-([LRS])(\d+)(?:>(\d+))?\s*", line)
        if group:
            kind, amount, selected = group.groups()
            amount = int(amount)
            if amount < 1 or amount > len(nodes):
                raise Holdout("invalid-group-size")
            children, nodes = nodes[-amount:], nodes[:-amount]
            if kind == "R":
                if selected is None or not 1 <= int(selected) <= amount:
                    raise Holdout("unresolved-random-selection")
                nodes.append(children[int(selected) - 1])
            elif kind == "L":
                nodes.append((max(c[0] for c in children), [v for c in children for v in c[1]]))
            else:
                duration, voices = 0.0, []
                for length, child in children:
                    voices += [(identity, start + duration, gain) for identity, start, gain in child]
                    duration += length
                nodes.append((duration, voices))
            continue
        source = re.match(r"^(?:.*[/\\])?\??(\d+)\.wem\s*(.*)$", line)
        if not source:
            raise Holdout("unsupported-txtp-line:" + line)
        identity, options = int(source[1]), source[2].strip()
        if identity not in media:
            raise Holdout("missing-decoded-media:" + str(identity))
        delay, gain = 0.0, 1.0
        options = re.sub(r"#i(?=\s|$)", "", options).strip()
        while options:
            option = re.match(r"#([vp])\s+(-?[0-9.]+)(dB)?\s*", options)
            if not option:
                raise Holdout("unsupported-media-directive:" + options)
            value = float(option[2])
            if option[1] == "v":
                gain *= 10 ** (value / 20) if option[3] else value
            else:
                if option[3] or value < 0:
                    raise Holdout("unsupported-padding")
                delay += value
            options = options[option.end():].strip()
        nodes.append((media[identity]["durationSeconds"] + delay, [(identity, delay, gain)]))
    if len(nodes) != 1:
        raise Holdout("unresolved-root-group")
    if not 0 < nodes[0][0] <= 600:
        raise Holdout("invalid-event-duration")
    return nodes[0]


def source_index(source):
    exact, clip_rows, events, evidence = {}, collections.defaultdict(list), set(), []
    for path in sorted((source / "RemainingCharacterExtraction-20260829/ActionNameSources").glob("*.action-effects.json")):
        document = read(path)
        profile = document["profileId"]
        ref = read(ROOT / f"Data/Animation/Reference/KoukuSaydon/{profile}.actionreference.json")
        refs = {(a["sourceActionId"], s["stageOrdinal"]): s for a in ref["actions"] for s in a["stages"]}
        count = 0
        for action in document["actions"]:
            for stage in action["stages"]:
                sounds = [n for n in stage["notifies"] if n["sourceType"] == "AKEvent"]
                count += len(sounds)
                for n in sounds:
                    events.update(a["objectPath"].lower() for a in n["assetReferences"] if a["className"] == "AkEvent")
                for slot in refs.get((action["actionId"], stage["stageIndex"]), {}).get("slots", []):
                    animations = [n for n in stage["notifies"] if n["sourceType"] == "Anim" and n.get("clipName") == slot["extractedClip"]]
                    if len(animations) != 1:
                        continue
                    origin = animations[0]["localTimeSeconds"]
                    rows = [(n["notifyId"], a["objectPath"].lower(), (n["localTimeSeconds"] - origin) * 1000)
                            for n in sounds for a in n["assetReferences"] if a["className"] == "AkEvent"]
                    key = (profile, action["actionId"], f"stage-{stage['stageIndex']:03}", slot["slotId"])
                    record = dict(key=key, rows=rows, nativeMs=slot["playMs"], clip=slot["runtimeClip"])
                    exact[key] = record
                    clip_rows[profile, slot["runtimeClip"]].append(record)
        evidence.append(dict(path=str(path), sha256=digest(path), profile=profile, akEventNotifies=count))
    return exact, clip_rows, events, evidence


def animation_cues(document, exact, clips, holdouts):
    cues = []
    for pattern in document["patterns"]:
        stage_start = 0
        for stage in pattern["stages"]:
            for animation in stage["animationOccurrences"]:
                key = (animation["profileId"], animation["sourceActionId"], animation["sourceStageId"], animation["sourceSlotId"])
                record = exact.get(key)
                basis = "exact-action-stage-slot"
                if record is None:
                    alternatives = clips.get((animation["profileId"], animation["runtimeClip"]), [])
                    signatures = {tuple((e, round(t, 3)) for _, e, t in r["rows"]) for r in alternatives}
                    if len(signatures) == 1:
                        record = alternatives[0]
                        basis = "unanimous-source-clip-notifies"
                    else:
                        holdouts.append(dict(kind="animation-without-unique-source", occurrenceId=animation["occurrenceId"],
                                             runtimeClip=animation["runtimeClip"], sourceCandidates=len(alternatives), signatures=len(signatures)))
                        continue
                start = stage_start + animation["startOffsetMs"]
                rate, play_ms, source_in = animation["playRate"], animation["playMs"], animation["sourceStartMs"]
                native = record["nativeMs"]
                loop = animation["endPolicy"] == "LOOP_TO_WINDOW"
                for notify, event, source_ms in record["rows"]:
                    # The source Anim starts at its own notify clock. Trimming
                    # never replays a sound preceding the selected source-in.
                    if source_ms < source_in - .5 or source_ms >= native + .5:
                        continue
                    local = (source_ms - source_in) / rate
                    cycle = (native - source_in) / rate
                    repeat = 0
                    while 0 <= local < play_ms - .5:
                        cues.append(dict(patternId=pattern["patternId"], event=event,
                            startMs=start + max(0, round(local)), sourceKind="animation", sourceOccurrenceId=animation["occurrenceId"],
                            sourceNotifyId=notify, sourceActionKey=list(record["key"]), basis=basis, loopIndex=repeat))
                        if not loop or cycle <= 0:
                            break
                        local += cycle
                        repeat += 1
            stage_start += stage["durationMs"]
    return cues


SCENES = [
    ("kouku.gate1.full", "SCENE03A", 857),
    ("kouku.gate2.intro", "SCENE04A", 394),
    ("kouku.gate2.clear", "SCENE02A", 117),
    ("kouku.gate3.intro", "SCENE02A", 117),
    ("kouku.gate2.maze", "SCENE04A", 393),
    ("kouku.gate3.showtime", "SCENE02B", 49),
    ("kouku.bingo.ending", "SCENE01B", 45),
]


def scene_cues(document, holdouts):
    cues, evidence, all_events = [], [], set()
    resources = {r["resourceId"]: r for r in document["presentationResources"]}
    for prefix, scene, data_id in SCENES:
        path = next((ROOT / "out" / folder / f"LV_LUT_MIDNIGHTC_ED_{scene}.json"
                     for folder in ("KoukuSourceSequenceRestore20260912", "KoukuFireworks20260911")
                     if (ROOT / "out" / folder / f"LV_LUT_MIDNIGHTC_ED_{scene}.json").exists()), None)
        if path is None:
            holdouts.append(dict(kind="missing-scene-source", scene=scene)); continue
        source = read(path)
        rows, imports = source["rows"], source["imports"]
        data = rows[str(data_id)]["p"]
        tracks = [(t, rows[str(t)]) for g in data["interpgroups"] for t in rows[str(g)]["p"].get("interptracks", [])]
        sound_tracks = [(t, r) for t, r in tracks if r["cls"] == "interptrackakevent" and not r["p"].get("bdisabletrack", False)]
        slomo = [r for _, r in tracks if r["cls"] == "interptrackslomo" and not r["p"].get("bdisabletrack", False)]
        patterns = [p for p in document["patterns"] if any(resources.get(b["resourceId"], {}).get("assetId", "").startswith(prefix + ".camera.")
                    or b["resourceId"].startswith("presentation." + prefix + ".camera.") for b in p.get("presentationOccurrences", []))]
        evidence.append(dict(scene=scene, dataExport=data_id, path=str(path), sha256=digest(path), patternIds=[p["patternId"] for p in patterns],
                             clock="MATINEE_SOURCE_MS (same as installed World/Camera)", globalSlomo=slomo))
        for track, row in sound_tracks:
            for ordinal, key in enumerate(row["p"].get("akevents", [])):
                event = imports.get(str(key["event"]), "").lower()
                if not event:
                    holdouts.append(dict(kind="unresolved-scene-event", scene=scene, track=track, key=ordinal)); continue
                all_events.add(event)
                for pattern in patterns:
                    native_ms = round(key["time"] * 1000)
                    clock_ms = native_ms
                    source_in = 0
                    basis = "exact-active-matinee-akevent"
                    if prefix == "kouku.gate1.full":
                        # Existing approved popup light/carrier clock explicitly
                        # uses this source mapping. Do not integrate GlobalSlomo
                        # again: doing so would diverge from installed visuals.
                        landmarks = [(0., 0.), (8.752, 8.752), (19.741, 18.352),
                                     (22.244, 20.852), (26.919, 25.981),
                                     (33.583, 35.409), (41.488, 46.552)]
                        t = key["time"]
                        pair = next(((a, x, b, y) for (a, x), (b, y) in zip(landmarks, landmarks[1:]) if a <= t <= b), None)
                        if pair is None:
                            holdouts.append(dict(kind="authored-scene-clock-unresolved", patternId=pattern["patternId"], sourceMs=native_ms)); continue
                        a, x, b, y = pair
                        clock_ms = 12258 + round((x + (t-a)*(y-x)/(b-a))*1000)
                        basis = "existing-build_gate1_popup_lights-authored-source-clock"
                    elif prefix == "kouku.gate3.intro":
                        # This canonical sequence retains SCENE02A's latter window.
                        # The already active scene soundtrack resumes at its media
                        # age rather than replaying its first 16.71 seconds.
                        camera_boxes = [b for b in pattern["presentationOccurrences"] if
                                        resources.get(b["resourceId"], {}).get("kind") == "CAMERA"]
                        if camera_boxes and resources[camera_boxes[0]["resourceId"]]["assetId"].endswith(".11") and camera_boxes[0]["startMs"] == 0:
                            clock_ms = native_ms - 16710
                            source_in = max(0, -clock_ms)
                            clock_ms = max(0, clock_ms)
                            basis = "existing-build_source_sequences-source-window-16710"
                    cues.append(dict(patternId=pattern["patternId"], event=event, startMs=clock_ms, soundSourceStartMs=source_in,
                        sourceKind="matinee", sourceScene=scene, sourceTrack=track, sourceKey=ordinal,
                        sourceTimeMs=native_ms, basis=basis))
    return cues, evidence, all_events


def media_index(source):
    receipt = source / "SoundPlayableExtraction-20260830/LV_LUT_MIDNIGHTC_ED.sound-extraction.receipt.json"
    result = {}
    for row in read(receipt)["files"]:
        row = dict(row)
        row["sourcePath"] = source / "SoundPlayableExtraction-20260830/wav" / row["relativePath"]
        row["assetId"] = "Sound/KoukuSaton/" + row["relativePath"]
        installed = ROOT / "Client/Bin/Resources" / row["assetId"]
        # A closure can rename an alias; identify installed media by ID below.
        row["installedPath"] = installed if installed.exists() else None
        result.setdefault(row["mediaId"], row)
    by_id = collections.defaultdict(list)
    for path in (ROOT / "Client/Bin/Resources/Sound/KoukuSaton").rglob("*.wav"):
        match = re.search(r"__(\d+)\.wav$", path.name)
        if match:
            by_id[int(match[1])].append(path)
    for identity, row in result.items():
        if by_id[identity]:
            row["installedPath"] = sorted(by_id[identity])[0]
            row["assetId"] = row["installedPath"].relative_to(ROOT / "Client/Bin/Resources").as_posix()
    return result, receipt


def txtp_index(source):
    result = collections.defaultdict(list)
    roots = [source / "SoundRuntimeExtraction-20260830/txtp-complete", source / "SoundBuffExtraction-20260830/txtp-complete"]
    for root in roots:
        for path in sorted(root.glob("*/*.txtp")):
            text = path.read_text(encoding="utf-8-sig")
            match = re.search(r"# \* full name: ([A-Za-z0-9_]+)", text)
            if match:
                result[path.parent.name.lower() + "." + match[1].lower()].append((path, text))
    return result


def random_layouts(text, media):
    """Enumerate each native random branch, not wwiser's global rN selector.

    wwiser writes the same rN into nested groups, which may exceed a smaller
    inner group. Independent group choices avoid dropping real nested voices.
    Source probabilities are read from the bank playlist, never inferred here.
    """
    expression = re.compile(r"(group\s*=\s*-R)(\d+)>\d+")
    amounts = [int(m[2]) for m in expression.finditer(text)]
    if not amounts: return [(txpt_layout(text, media), text)]
    if math.prod(amounts) > 4096: raise Holdout("random-tree-enumeration-limit")
    layouts = {}
    for choices in itertools.product(*(range(1, n + 1) for n in amounts)):
        selected = iter(choices)
        variant = expression.sub(lambda m: m[1] + m[2] + ">" + str(next(selected)), text)
        layout = txpt_layout(variant, media)
        layouts[stamp(layout)] = (layout, variant)
    return list(layouts.values())


class BankWeights:
    def __init__(self, source):
        self.paths = {p.name: p for folder in ("SoundRuntimeExtraction-20260830", "SoundBuffExtraction-20260830", "SoundStandaloneExtraction-20260830")
                      for p in (source / folder / "banks").rglob("*.bnk")}
        self.cache = {}
        self.provenance = {}
        self.playlists = {}

    def resolve(self, event, text):
        import wwise_audio_package as wwise
        names = re.findall(r"# - .*[\\/](\d+\.bnk)", text)
        objects = {}
        for name in names:
            if name not in self.paths: raise Holdout("missing-source-bank:" + name)
            if name not in self.cache:
                self.cache[name] = wwise.hirc_objects(self.paths[name].read_bytes())
                self.provenance[name] = dict(path=str(self.paths[name]), sha256=digest(self.paths[name]))
            objects.update(self.cache[name])
        if not objects: raise Holdout("missing-source-bank-provenance")

        def combine(left, right):
            result = collections.defaultdict(Fraction)
            for a, pa in left.items():
                for b, pb in right.items(): result[tuple(sorted(a + b))] += pa * pb
            if len(result) > 4096: raise Holdout("weighted-variant-limit")
            return result

        def visit(identity, ancestors=()):
            if identity in ancestors or identity not in objects: raise Holdout("unresolved-hirc-child")
            kind, payload = objects[identity]
            if kind == wwise.HIRC_SOUND: return {(wwise.sound_source_id(payload),): Fraction(1)}
            if kind not in (wwise.HIRC_RANDOM_SEQUENCE, wwise.HIRC_LAYER, wwise.HIRC_ACTOR_MIXER):
                raise Holdout("non-plain-hirc-tree")
            children = wwise.container_children(payload, objects)
            if not children: raise Holdout("empty-hirc-container")
            ancestors += (identity,)
            if kind == wwise.HIRC_RANDOM_SEQUENCE:
                # The version-134 playlist is the exact trailing uint16 count
                # plus (child object ID, uint32 weight) records.
                size = 2 + 8 * len(children)
                tail = payload[-size:]
                if len(tail) != size or struct.unpack_from('<H', tail)[0] != len(children):
                    raise Holdout("unknown-random-playlist-layout")
                playlist = [struct.unpack_from('<II', tail, 2 + i * 8) for i in range(len(children))]
                if set(c for c, _ in playlist) != set(children) or not sum(p for _, p in playlist):
                    raise Holdout("invalid-random-playlist")
                result = collections.defaultdict(Fraction)
                self.playlists[str(identity)] = dict(bankFiles=names, childrenAndWeights=playlist)
                total = sum(p for _, p in playlist)
                for child, weight in playlist:
                    for sources, probability in visit(child, ancestors).items(): result[sources] += probability * Fraction(weight, total)
                return result
            result = {(): Fraction(1)}
            for child in children: result = combine(result, visit(child, ancestors))
            return result

        row = objects.get(wwise.fnv1_32(event.rsplit('.', 1)[-1]))
        if row is None or row[0] != wwise.HIRC_EVENT: raise Holdout("missing-hirc-event")
        result, play_count = {(): Fraction(1)}, 0
        for action in wwise.event_action_ids(row[1]):
            if action not in objects: raise Holdout("missing-hirc-action")
            kind, target = wwise.action_fields(objects[action][1])
            if kind != wwise.ACTION_PLAY: continue
            result = combine(result, visit(target)); play_count += 1
        if not play_count: raise Holdout("control-only-event")
        return result


def render_layout(layout, media, output, identity, copied, rendered):
    import numpy as np
    from scipy.io import wavfile
    length, voices = layout
    if len(voices) == 1 and voices[0][1] == 0 and voices[0][2] == 1:
        row = media[voices[0][0]]
        source = row["installedPath"] or row["sourcePath"]
        if digest(source).lower() != row["sha256"].lower():
            raise Holdout("media-sha256-mismatch:" + str(source))
        if row["installedPath"] is None:
            destination = output / "Resources" / row["assetId"]
            destination.parent.mkdir(parents=True, exist_ok=True)
            if not destination.exists(): shutil.copyfile(source, destination)
            copied[row["assetId"]] = dict(source=str(source), sha256=digest(source), bytes=source.stat().st_size)
        return row["assetId"], math.ceil(length * 1000)
    asset = "Sound/KoukuSaton/Events/" + identity + ".wav"
    destination = output / "Resources" / asset
    destination.parent.mkdir(parents=True, exist_ok=True)
    data = np.zeros((math.ceil(length * 48000) + 2, 2), dtype=np.float32)
    evidence = []
    for media_id, delay, gain in voices:
        row = media[media_id]
        path = row["installedPath"] or row["sourcePath"]
        if digest(path).lower() != row["sha256"].lower(): raise Holdout("media-sha256-mismatch:" + str(path))
        rate, samples = wavfile.read(path)
        if rate != 48000 or samples.dtype != np.int16: raise Holdout("unsupported-decoded-pcm")
        samples = samples.astype(np.float32) / 32768.0
        if samples.ndim == 1: samples = np.repeat(samples[:, None], 2, axis=1)
        if samples.shape[1] != 2: raise Holdout("unsupported-channel-count")
        start = round(delay * rate)
        data[start:start + len(samples)] += samples * gain
        evidence.append(dict(mediaId=media_id, source=str(path), sha256=row["sha256"], delaySeconds=delay, gain=gain))
    # Float WAV preserves additive source layers without clipping or loudness normalization.
    wavfile.write(destination, 48000, data)
    rendered[asset] = dict(sources=evidence, sha256=digest(destination), bytes=destination.stat().st_size,
                           peak=float(np.max(np.abs(data))), format="IEEE_FLOAT32_48000_STEREO")
    return asset, math.ceil(length * 1000)


def render_source_txtp(path, text, media, output, rendered, decoder, bounded_window_ms=0):
    """Use the installed offline decoder for native nested random/music rules.

    No loop is fabricated. A source infinite loop or unresolved state/switch
    still needs the gameplay lifetime/state owner and stays out of this import.
    """
    if re.search(r"CAkSwitchCntr|CAkMusicSwitchCntr", text):
        raise Holdout("state-switch-tree")
    source_loop = "#@loop" in text or "##loop" in text or bool(re.search(r"#l\s|#E(?:\s|$)", text))
    if source_loop and not bounded_window_ms:
        raise Holdout("continuous-loop-requires-control-owner")
    if not decoder.is_file():
        raise Holdout("finite-txtp-needs-vgmstream")
    sources, lines = [], []
    for line in text.splitlines():
        if not line.lstrip().startswith("#"):
            match = re.match(r"^(\s*)(?:.*[/\\])?\??(\d+)\.wem(.*)$", line)
            if match:
                row = media.get(int(match[2]))
                if row is None: raise Holdout("missing-decoded-media:" + match[2])
                source = row["installedPath"] or row["sourcePath"]
                if digest(source).lower() != row["sha256"].lower(): raise Holdout("media-sha256-mismatch:" + str(source))
                line = match[1] + source.as_posix() + match[3]
                sources.append(dict(mediaId=int(match[2]), path=str(source), sha256=row["sha256"]))
        lines.append(line)
    identity = "event." + stamp([digest(path), text, sources])
    staged = output / "txtp" / (identity + ".txtp")
    staged.parent.mkdir(parents=True, exist_ok=True)
    staged.write_text("\n".join(lines) + "\n", encoding="utf-8")
    asset = "Sound/KoukuSaton/Events/" + identity + ".wav"
    destination = output / "Resources" / asset
    destination.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([str(decoder), "-i", "-W", "4", "-o", str(destination), str(staged)],
                            capture_output=True, text=True, timeout=60)
    if result.returncode or not destination.is_file():
        raise Holdout("txtp-decode-failed:" + result.stderr.strip())
    from scipy.io import wavfile
    rate, data = wavfile.read(destination)
    duration = math.ceil(len(data) * 1000 / rate)
    if not 0 < duration <= 600000: raise Holdout("invalid-rendered-duration")
    if source_loop and duration < bounded_window_ms:
        raise Holdout("loop-window-exceeds-first-source-cycle")
    rendered[asset] = dict(sources=sources, sourceTxtp=str(path), sourceTxtpSha256=digest(path),
                           decoder=str(decoder), sha256=digest(destination), bytes=destination.stat().st_size,
                           format=str(data.dtype), decoderOutput=result.stdout)
    if source_loop:
        rendered[asset].update(sourceLoops=True, boundedOwnerWindowMs=bounded_window_ms,
                               policy="First source cycle fully covers every existing finite Matinee owner window; owner end stops the cue.")
    return asset, duration


def build(output, source=DEFAULT_SOURCE, decoder=VGMSTREAM, composition=COMPOSITION):
    document = read(ROOT / composition)
    before = copy.deepcopy(document)
    catalog = read(ROOT / CATALOG)
    exact, clips, events, action_evidence = source_index(source)
    holdouts = []
    cues = animation_cues(document, exact, clips, holdouts)
    scenes, scene_evidence, scene_events = scene_cues(document, holdouts)
    cues += scenes
    events |= scene_events
    media, receipt = media_index(source)
    txpts = txtp_index(source)
    bank_weights = BankWeights(source)
    copied, rendered, admitted, event_evidence = {}, {}, {}, []
    bucket = catalog.setdefault("classes", {}).setdefault("KoukuSaydon", {})
    needed = {c["event"] for c in cues}
    owner_durations = {p["patternId"]: p.get("durationMs", 0) or sum(s["durationMs"] for s in p["stages"]) for p in document["patterns"]}
    for event in sorted(events):
        variants, durations, rejected, evidence = [], [], [], []
        source_variants = txpts.get(event, [])
        uses = [c for c in cues if c["event"] == event]
        bounded_ms = max((owner_durations[c["patternId"]] - c["startMs"] + c.get("soundSourceStartMs", 0) for c in uses), default=0) if all(c["sourceKind"] == "matinee" for c in uses) else 0
        weighted = []
        if source_variants:
            path, text = source_variants[0]
            try:
                probabilities = bank_weights.resolve(event, text)
                layouts = random_layouts(text, media)
                if {tuple(sorted(v[0] for v in layout[1])) for layout, _ in layouts} != set(probabilities):
                    raise Holdout("txtp-hirc-playlist-mismatch")
                denominator = math.lcm(*(p.denominator for p in probabilities.values()))
                if denominator > 10000: raise Holdout("catalog-weight-resolution-limit")
                for layout, variant_text in layouts:
                    probability = probabilities[tuple(sorted(v[0] for v in layout[1]))]
                    if not probability: continue
                    if len(layout[1]) == 1 and layout[1][0][1:] == (0.0, 1.0):
                        asset, duration = render_layout(layout, media, output, "event." + stamp([event, layout]), copied, rendered)
                    else:
                        asset, duration = render_source_txtp(path, variant_text, media, output, rendered, decoder)
                    weighted.extend([asset] * int(probability * denominator))
                    durations.append(duration)
                    evidence.append(dict(path=str(path), sha256=digest(path), layout=layout, assetId=asset,
                                         sourceProbability=str(probability), weight=int(probability * denominator)))
                variants = weighted
            except Holdout:
                # Music and fully qualified advanced finite TXTP use the native
                # decoder below. It still never invents a state or loop lifetime.
                variants, durations, evidence = [], [], []
        for path, text in ([] if variants else source_variants):
            try:
                try:
                    layout = txpt_layout(text, media)
                    if len(layout[1]) != 1 or layout[1][0][1:] != (0.0, 1.0):
                        raise Holdout("native-layer-delay-gain-render")
                    asset, duration = render_layout(layout, media, output, "event." + stamp([event, layout]), copied, rendered)
                except Holdout:
                    asset, duration = render_source_txtp(path, text, media, output, rendered, decoder, bounded_ms)
                    layout = "native finite TXTP offline render"
                if asset not in variants: variants.append(asset); durations.append(duration)
                evidence.append(dict(path=str(path), sha256=digest(path), layout=layout, assetId=asset))
            except (Holdout, FileNotFoundError) as exc:
                rejected.append(str(exc))
        # Partial random alternatives silently change the distribution; hold out the whole event.
        if rejected or not variants:
            holdouts.append(dict(kind="unadmitted-event", event=event, used=event in needed,
                                 reasons=sorted(set(rejected)) or ["no-playable-txtp (control-only or source unavailable)"]))
            continue
        key = event
        if len(key) > 128: key = "sound.kouku." + stamp(event)
        bucket[key] = variants
        resource_id = "sound.kouku." + stamp(event)
        admitted[event] = dict(resourceId=resource_id, soundEvent=key, assets=variants, durationMs=max(durations))
        event_evidence.append(dict(event=event, soundEvent=key, resourceId=resource_id, variants=evidence))
    additions, bindings = [], []
    resource_ids = {r["resourceId"] for r in document["presentationResources"]}
    for event, value in admitted.items():
        if value["resourceId"] in resource_ids: continue
        resource = dict(resourceId=value["resourceId"], displayName="쿠크 원본 사운드 / " + event.rsplit(".", 1)[-1],
                        kind="SOUND", assetId=value["assets"][0], soundEvent=value["soundEvent"],
                        resourceKind="", elementId="", durationMs=value["durationMs"], defaultAnchorKind="BOSS",
                        shape="BOX", colliderKind="GEOMETRY", halfExtents=[1, 1, 1], radiusM=3, halfAngleDegrees=45)
        document["presentationResources"].append(resource)
        additions.append(resource)
    patterns = {p["patternId"]: p for p in document["patterns"]}
    for cue in cues:
        event = admitted.get(cue["event"])
        if event is None: continue
        pattern = patterns[cue["patternId"]]
        # A malformed existing visual box must never extend the authored owner.
        end = pattern.get("durationMs", 0) or sum(s["durationMs"] for s in pattern["stages"])
        source_in = cue.get("soundSourceStartMs", 0)
        duration = min(event["durationMs"] - source_in, end - cue["startMs"])
        if duration <= 0: continue
        existing = next((r for r in pattern.get("presentationOccurrences", []) if r["resourceId"] == event["resourceId"] and r["startMs"] == cue["startMs"]), None)
        if existing: continue
        if len(pattern.get("presentationOccurrences", [])) >= 1024:
            holdouts.append(dict(kind="pattern-cue-capacity", **cue)); continue
        number = pattern.get("nextPresentationOccurrenceOrdinal", 1)
        row = dict(occurrenceId=f"{pattern['patternId']}.presentation.{number}", resourceId=event["resourceId"],
                   startMs=cue["startMs"], durationMs=duration, soundSourceStartMs=source_in,
                   volume=1, anchorKind="WORLD" if cue["sourceKind"] == "matinee" else "BOSS", followBoss=False)
        pattern.setdefault("presentationOccurrences", []).append(row)
        pattern["nextPresentationOccurrenceOrdinal"] = number + 1
        bindings.append(dict(**cue, occurrenceId=row["occurrenceId"], resourceId=row["resourceId"], durationMs=duration))
    if bindings or additions: document["revision"] += 1
    write(output / "candidate" / composition, document)
    write(output / "candidate" / CATALOG, catalog)
    patch = dict(schema="lostark.kouku-sound-addition-candidate", formatVersion=1,
                 compositionPath=composition.as_posix(), compositionBaselineSha256=digest(ROOT / composition),
                 baselineRevision=before["revision"], catalogPath=CATALOG.as_posix(), catalogBaselineSha256=digest(ROOT / CATALOG),
                 resources=additions, catalogEntries=bucket,
                 patterns=[dict(patternId=p["patternId"], baseline=old, candidate=p) for old, p in zip(before["patterns"], document["patterns"]) if p != old])
    write(output / "additions.json", patch)
    summary = dict(sourceActionNotifies=sum(x["akEventNotifies"] for x in action_evidence), sourceEventIdentities=len(events),
        admittedEventIdentities=len(admitted), addedResources=len(additions), addedOccurrences=len(bindings),
        animationOccurrences=sum(b["sourceKind"] == "animation" for b in bindings), matineeOccurrences=sum(b["sourceKind"] == "matinee" for b in bindings),
        affectedPatterns=len(patch["patterns"]), installedMediaReused=len({asset for a in admitted.values() for asset in a["assets"] if asset not in copied and asset not in rendered}),
        copiedSourceMedia=len(copied), renderedEventVariants=len(rendered), newlyDecodedMedia=0,
        holdoutReasons=dict(collections.Counter(x["kind"] for x in holdouts)), outputBytes=sum(x["bytes"] for x in copied.values()) + sum(x["bytes"] for x in rendered.values()))
    manifest = dict(summary=summary, baselineRevision=before["revision"], sources=action_evidence,
                    mediaReceipt=dict(path=str(receipt), sha256=digest(receipt)), scenes=scene_evidence,
                    bankSources=bank_weights.provenance, sourceRandomPlaylists=bank_weights.playlists,
                    events=event_evidence, bindings=bindings, copiedMedia=copied, renderedVariants=rendered, holdouts=holdouts)
    write(output / "manifest.json", manifest)
    print(json.dumps(summary, ensure_ascii=False))
    return manifest


def merge_additions(current, patch):
    """In-memory stable-ID merge for the authorized final installer.

    The caller must still recheck file hashes immediately before atomic replace
    and keep a backup/rollback. This function never writes a live file. Timing
    changes require rebuilding from the latest saved animation/source clock;
    unrelated new boxes, placement edits and annotations are retained.
    """
    merged = copy.deepcopy(current)
    resources = {r["resourceId"]: r for r in merged["presentationResources"]}
    for resource in patch["resources"]:
        old = resources.get(resource["resourceId"])
        if old is not None and old != resource: raise ValueError("Conflicting sound resource: " + resource["resourceId"])
        if old is None:
            merged["presentationResources"].append(copy.deepcopy(resource)); resources[resource["resourceId"]] = resource
    patterns = {p["patternId"]: p for p in merged["patterns"]}
    for change in patch["patterns"]:
        pattern = patterns.get(change["patternId"])
        if pattern is None: raise ValueError("Pattern was removed: " + change["patternId"])
        old, candidate = change["baseline"], change["candidate"]
        for field in ("stages", "durationMs", "patternOccurrences"):
            if pattern.get(field) != old.get(field): raise ValueError("Rebuild sound candidate after timing edit: " + pattern["patternId"] + "." + field)
        old_ids = {b["occurrenceId"] for b in old.get("presentationOccurrences", [])}
        for addition in candidate["presentationOccurrences"]:
            if addition["occurrenceId"] in old_ids: continue
            if any(b["resourceId"] == addition["resourceId"] and b["startMs"] == addition["startMs"] and
                   b.get("soundSourceStartMs", 0) == addition.get("soundSourceStartMs", 0)
                   for b in pattern.get("presentationOccurrences", [])): continue
            addition = copy.deepcopy(addition)
            ordinal = pattern.get("nextPresentationOccurrenceOrdinal", 1)
            ids = {b["occurrenceId"] for b in pattern.get("presentationOccurrences", [])}
            while f"{pattern['patternId']}.presentation.{ordinal}" in ids: ordinal += 1
            addition["occurrenceId"] = f"{pattern['patternId']}.presentation.{ordinal}"
            pattern.setdefault("presentationOccurrences", []).append(addition)
            pattern["nextPresentationOccurrenceOrdinal"] = ordinal + 1
    if merged != current: merged["revision"] = current["revision"] + 1
    return merged


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, default=ROOT / "out/KoukuSoundRestore20260918")
    parser.add_argument("--source-root", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--vgmstream", type=Path, default=VGMSTREAM)
    parser.add_argument("--composition", type=Path, default=COMPOSITION)
    args = parser.parse_args()
    output = args.out.resolve()
    if output == ROOT or ROOT / "out" not in output.parents:
        raise ValueError("Candidates must stay in a repository out/ subdirectory")
    build(output, args.source_root, args.vgmstream, args.composition)


if __name__ == "__main__":
    main()
