"""Offline stable numeric-source bindings for the native Server balance store.

No gameplay file is published. World collider identities reuse the existing
publisher's finite clock expansion; geometry samples are irrelevant to identity.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
import project_kouku_saydon_composition as kouku
import world_object_collider as collider

COMPOSITION = "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json"
ENCOUNTER = "Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json"
WORLD = "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"
WORLD_PRODUCT = "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json"
VALTAN = "Data/Valtan/Valtan.gameplay.json"
VALTAN_PRODUCT = "Data/Encounters/Valtan/ValtanEncounter.json"


def walk(value, path=()):
    yield path, value
    if isinstance(value, dict):
        for key, child in value.items():
            yield from walk(child, path + (key,))
    elif isinstance(value, list):
        for index, child in enumerate(value):
            yield from walk(child, path + (index,))


def generate(root: Path):
    documents, original = {}, {}
    def read(name):
        if name not in documents:
            original[name] = (root / name).read_bytes()
            documents[name] = json.loads(original[name])
        return documents[name]
    def address(name, path):
        value = read(name)
        guards = []
        for index, step in enumerate(path):
            value = value[step]
            if isinstance(value, dict):
                for identity in ("patternId", "logicId", "occurrenceId", "windowId", "triggerId", "hitId", "sequenceId", "colliderTrackId", "actionId"):
                    if identity in value and isinstance(value[identity], str):
                        guards.append({"path": list(path[:index + 1] + (identity,)), "value": value[identity]})
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"Not a numeric source: {name}/{path}")
        return {"document": name, "path": list(path), "guards": guards}

    source, product, world = read(COMPOSITION), read(ENCOUNTER), read(WORLD)
    definitions = {row["logicId"]: (i, row) for i, row in enumerate(source["logics"])}
    boxes = {box["occurrenceId"]: box for pattern in source["patterns"] for box in pattern["logicOccurrences"]}
    product_patterns = {row["patternId"]: (i, row) for i, row in enumerate(product["patterns"])}
    bootstrap = (root / "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap").read_text(encoding="utf-8-sig")
    rows = [row.split("\t") for row in bootstrap.splitlines()]
    damage_rows = [r for r in rows if r[0] == "PATTERNLOGICOUTCOME" and r[6] in ("MAX_HP_PERCENT_DAMAGE", "FIXED_DAMAGE") or
                   r[0] == "PATTERNATTACKHIT" and r[22] == "MAX_HP_PERCENT"]

    # Obtain exactly the stable identities emitted by the existing world-clock
    # expander. Sampling cannot influence an identity, and no fake geometry is
    # returned to any product consumer or validation result.
    captures = {}
    real_hashlib, real_sample = collider.hashlib, collider.sample_object
    def record_hash(payload):
        digest = hashlib.sha256(payload)
        text = payload.decode()
        parts = text.split("|")
        if len(parts) == 6:
            captures["object.collider.window." + digest.hexdigest()[:32]] = parts[2]
        return digest
    def identity_sample(*args):
        return ([0., 0., 0.], [0., 0., 0.], [1., 1., 1.], 0., True)
    def unused_model(*args):
        raise AssertionError("Identity expansion must not load models")
    try:
        collider.hashlib = SimpleNamespace(sha256=record_hash)
        collider.sample_object = identity_sample
        admitted = kouku._publication_candidate(source, set(product_patterns))
        expanded = kouku._expand_parent_patterns(admitted)
        worlds = {row["worldId"]: row for row in expanded["worlds"]}
        object_patterns = {r[2] for r in damage_rows if r[0] == "PATTERNLOGICOUTCOME" and r[3].startswith("object.collider.")}
        for pattern in expanded["patterns"]:
            if pattern["patternId"] not in object_patterns:
                continue
            enabled = [box for box in pattern.get("worldOccurrences", []) if box.get("enabled", True)]
            collider.bake_windows(world, worlds, enabled, unused_model,
                                  pattern_end_ms=kouku._pattern_duration(pattern))
    finally:
        collider.hashlib, collider.sample_object = real_hashlib, real_sample
    track_paths = {track["colliderTrackId"]: ("templates", i, "colliderTracks", j, "damagePercent")
                   for i, template in enumerate(world["templates"]) for j, track in enumerate(template.get("colliderTracks", []))}
    entries = []
    for row in damage_rows:
        sources, mirrors = [], []
        if row[0] == "PATTERNLOGICOUTCOME":
            identity = "O|" + "|".join(row[1:6])
            field = "fixedDamage" if row[6] == "FIXED_DAMAGE" else "maxHpDamagePercent"
            product_field = "damageAmount" if row[6] == "FIXED_DAMAGE" else "percent"
            pi, pattern = product_patterns[row[2]]
            wi = next(i for i, box in enumerate(pattern["logicWindows"]) if box["windowId"] == row[3])
            outcome = {"SUCCESS": "onSuccess", "FAIL": "onFail", "TIMEOUT": "onTimeout"}[row[4]]
            oi = int(row[5])
            mirrors.append(address(ENCOUNTER, ("patterns", pi, "logicWindows", wi, outcome, oi, product_field)))
            if row[3].startswith("object.collider."):
                path = track_paths[captures[row[3]]]
                sources.append(address(WORLD, path))
                # Authoring and installed Map share the exact stable template/track join.
                installed = read(WORLD_PRODUCT)
                template_id = world["templates"][path[1]]["sequenceId"]
                ti = next(i for i, t in enumerate(installed["templates"]) if t["sequenceId"] == template_id)
                ci = next(i for i, t in enumerate(installed["templates"][ti]["colliderTracks"]) if t["colliderTrackId"] == captures[row[3]])
                mirrors.append(address(WORLD_PRODUCT, ("templates", ti, "colliderTracks", ci, "damagePercent")))
            else:
                box = boxes[row[3]]
                logic_id = box[outcome + "LogicIds"][oi]
                li, _ = definitions[logic_id]
                sources.append(address(COMPOSITION, ("logics", li, product_field)))
        else:
            identity = "H|" + "|".join(row[1:7]); field = "maxHpDamagePercent"
            if row[1] == "ENCOUNTER_KAKULSAYDON_G1":
                li, logic = definitions[boxes[row[3]]["logicId"]]
                key = "randomVolleyHits" if row[4] == "RANDOM" else "projectileHits" if row[4] == "PROJECTILE" else "fixedHits"
                prefix = ("logics", li, key) + ((int(row[5]),) if key == "randomVolleyHits" else ())
                hits = logic[key][int(row[5])] if key == "randomVolleyHits" else logic[key]
                hi = next(i for i, hit in enumerate(hits) if hit["hitId"] == row[7])
                sources.append(address(COMPOSITION, prefix + (hi, "damagePercent")))
                pi, pattern = product_patterns[row[2]]
                # The Product nests these typed templates in logicWindows,
                # mechanicTriggers or targeted effect selection sets.
                matches = [(p, obj) for p, obj in walk(pattern) if isinstance(obj, dict) and obj.get("hitId") == row[7] and obj.get("damageKind") == "MAX_HP_PERCENT"]
                if row[4] == "RANDOM":
                    matches = [(p, obj) for p, obj in matches if "randomVolleys" in p and p[p.index("randomVolleys") + 1] == int(row[5])]
                elif row[4] != "PROJECTILE":
                    matches = [(p, obj) for p, obj in matches if "randomVolleys" not in p]
                # Restrict the template's owning window/trigger, not repeated hit IDs elsewhere.
                matches = [(p, obj) for p, obj in matches if any(isinstance(a, dict) and (a.get("windowId") == row[3] or a.get("triggerId") == row[3] or a.get("occurrenceId") == row[3])
                    for ap, a in walk(pattern) if p[:len(ap)] == ap)]
                if len(matches) != 1:
                    raise ValueError(f"Ambiguous Product template {identity}: {len(matches)}")
                mirrors.append(address(ENCOUNTER, ("patterns", pi) + matches[0][0] + ("damagePercent",)))
            else:
                authored, projected = read(VALTAN), read(VALTAN_PRODUCT)
                for name, doc, kind in ((VALTAN, authored, sources), (VALTAN_PRODUCT, projected, mirrors)):
                    matches = [(p, v) for p, v in walk(doc) if isinstance(v, dict) and v.get("hitId") == row[7]]
                    if len(matches) != 1: raise ValueError(f"Ambiguous Valtan hit source: {identity}")
                    kind.append(address(name, matches[0][0] + ("damagePercent",)))
        value = float(row[10] if field == "fixedDamage" else row[7] if row[0] == "PATTERNLOGICOUTCOME" else row[23])
        for location in sources + mirrors:
            candidate = read(location["document"])
            for step in location["path"]: candidate = candidate[step]
            if candidate != value: raise ValueError(f"Stale numeric source/Product: {identity}: {candidate} != {value}")
        alias = hashlib.sha256(json.dumps(sources, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
        entries.append({"id": identity, "field": field, "alias": alias, "sources": sources, "mirrors": mirrors})
    if len(entries) != len({(e["id"], e["field"]) for e in entries}): raise ValueError("Duplicate numeric bindings")
    output = {"schema": "lostark.numeric-source-bindings", "formatVersion": 1,
              "files": [{"path": path, "sha256": hashlib.sha256(content).hexdigest()} for path, content in sorted(original.items())],
              "entries": sorted(entries, key=lambda e: (e["id"], e["field"]))}
    return output, original


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output, original = generate(args.root)
    content = (json.dumps(output, ensure_ascii=False, separators=(",", ":")) + "\n").encode()
    path = args.root / "Data/Balance/NumericSourceBindings.json"
    for name, before in original.items():
        if (args.root / name).read_bytes() != before: raise RuntimeError("Source changed during binding generation: " + name)
    if args.check:
        if path.read_bytes() != content: raise RuntimeError("Numeric source bindings are stale")
    else:
        descriptor, temporary = tempfile.mkstemp(prefix=".NumericSourceBindings.", dir=path.parent)
        try:
            with os.fdopen(descriptor, "wb") as stream:
                stream.write(content); stream.flush(); os.fsync(stream.fileno())
            os.replace(temporary, path)
        finally:
            if Path(temporary).exists(): Path(temporary).unlink()
    print(f"Numeric source bindings: {len(output['entries'])} fields, {len(output['files'])} source/Product files")


if __name__ == "__main__":
    main()
