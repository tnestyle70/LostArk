"""Restore F lightning color and duplicate the requested AltV lightning rows.

The four extra waves are project-authored user tuning, not recovered V counts.
Existing transforms/times, F, and the V startup clip are preserved. The two old
fixed-color overrides are restored only while equal to this script's original
values. User-edited distributions and already duplicated rows are never replaced.
"""
from pathlib import Path
import copy
import hashlib
import json
import os


ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / "Data/Effects/Authored"
OUT = ROOT / "out/ArtistWarlordVisualFollowup20260910/Warlord"
PREFIX = "authored.warlord.v.golden-guardian-lightning."
ALT_LIGHTNING_IDS = (
    "981227451bbcb3791340", "522d5a0e988cf10a2c8a", "a7e449ba34d000861ce8",
    "8bce29d1c389261ef8bd", "ce6fd53e38bad2acbe3b", "da9e99a88cbcaee64d0e",
)


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def constant(distribution, values):
    result = copy.deepcopy(distribution)
    result.update(componentCount=len(values), operation=1, randomLockAxes=0,
                  lookupTableChunkSize=len(values), lookupTableNumElements=1,
                  lookupTableTimeScale=0.0, lookupTableStartTime=0.0,
                  defaultMinimum=list(values) + [0.0] * (4 - len(values)),
                  defaultMaximum=list(values) + [0.0] * (4 - len(values)),
                  lookupTable=[min(values), max(values)] + list(values) * 2,
                  keys=[])
    return result


def install(path, before_bytes, document):
    backup = OUT / "lightning_resume_before" / path.name
    backup.parent.mkdir(parents=True, exist_ok=True)
    if not backup.exists():
        backup.write_bytes(before_bytes)
    temporary = path.with_name(path.name + ".lightning.tmp")
    temporary.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                         encoding="utf-8")
    if path.read_bytes() != before_bytes:
        temporary.unlink()
        raise RuntimeError(f"Document changed during lightning patch: {path}")
    os.replace(temporary, path)


def restore_f_color(element, original):
    source_distributions = {
        d["propertyPath"]: d for m in original["sourceRecipe"]["modules"]
        for d in m["distributions"]
        if d["propertyPath"] in ("startcolor", "colorscaleoverlife")
    }
    repaired = []
    for module in element["sourceRecipe"]["modules"]:
        for index, distribution in enumerate(module["distributions"]):
            name = distribution["propertyPath"]
            if name not in source_distributions:
                continue
            source = source_distributions[name]
            legacy = constant(source, [1.0, 0.72, 0.08] if name == "startcolor"
                              else [1.0, 1.0, 1.0])
            if distribution == legacy:
                module["distributions"][index] = copy.deepcopy(source)
                repaired.append(name)
    return repaired


def duplicate_altv_lightning(receipt):
    path = AUTHORED / "effect.warlord.skill.17250.clip1.full.restore.effect.json"
    before_bytes = path.read_bytes()
    document = read(path)
    before = copy.deepcopy(document)
    existing = {e["id"]: e for e in document["elements"]}
    inserted = []
    for index, suffix in enumerate(ALT_LIGHTNING_IDS, 1):
        original_id = "authored.source-particle.full-warlord-alt_v." + suffix
        original = existing[original_id]
        assert original["material"]["sourceProfile"]["runtimeShaderProfileId"] == \
            "effect.ue3.warlord-1166-native.v1"
        assert any(r["assetId"].endswith("/fx_e_electric_005.dds")
                   for r in original["resources"])
        duplicate_id = "authored.copy." + original_id + ".lightning2"
        assert len(duplicate_id) <= 128
        if duplicate_id in existing:
            continue
        extra = copy.deepcopy(original)
        extra.update(id=duplicate_id, sourceNode="authored-copy:" + original_id,
                     displayName=original["displayName"] + " Lightning Extra")
        # Portable authored copies preserve the source RNG identity. A distinct
        # seed is therefore required to add spatially independent particles.
        extra["detail"]["particle"]["randomSeed"] = 172500 + index
        assert extra["detail"]["particle"]["randomSeed"] != \
            original["detail"]["particle"]["randomSeed"]
        extra["sourcePresentation"].update(
            enabled=False, profileId="", status="unresolved", sourceObjectPath="",
            sourceActionCueId="", sourceEventId="", sourceOccurrenceIndex=0,
            sourceTimeSeconds=0.0, parameters=[])
        document["elements"].append(extra)
        inserted.append({"id": duplicate_id, "sourceId": original_id,
                         "randomSeed": extra["detail"]["particle"]["randomSeed"],
                         "time": extra["detail"]["timing"]["startDelaySeconds"]})
    assert document["elements"][:len(before["elements"])] == before["elements"]
    assert {k: v for k, v in document.items() if k != "elements"} == \
        {k: v for k, v in before.items() if k != "elements"}
    if inserted:
        install(path, before_bytes, document)
    receipt["altV"] = {
        "path": str(path.relative_to(ROOT)).replace("\\", "/"),
        "sourceOccurrenceCount": 6, "targetOccurrenceCount": 12,
        "burstParticlesBefore": 24, "burstParticlesAfter": 48,
        "preservedElementCount": len(before["elements"]),
        "finalElementCount": len(document["elements"]), "inserted": inserted,
        "beforeSha256": hashlib.sha256(before_bytes).hexdigest(),
        "afterSha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        "allPreviousElementFieldsPreserved": True,
    }


def main():
    source_path = AUTHORED / "effect.warlord.skill.17140.full.restore.effect.json"
    source_bytes = source_path.read_bytes()
    source = read(source_path)
    original = next(e for e in source["elements"] if e["id"] ==
                    "authored.source-particle.full-warlord-f.7ba81a7899d33b48c48c")
    assert original["material"]["sourceProfile"]["runtimeShaderProfileId"] == \
        "effect.ue3.warlord-446-native.v1"

    waves = []
    for ordinal, (start, radius) in enumerate(
            [(1.1667, (200.0, 220.0)), (1.5, (360.0, 400.0)),
             (1.8334, (200.0, 220.0)), (2.5, (360.0, 400.0))], 1):
        element = copy.deepcopy(original)
        element.update(id=PREFIX + str(ordinal),
                       displayName=f"Golden Guardian Lightning {ordinal}",
                       groupId="authored.warlord.v.golden-guardian-lightning",
                       sourceNode="project-authored:warlord-v-golden-lightning|"
                       f"derived-from:{original['id']}|wave:{ordinal}")
        element["detail"]["timing"]["startDelaySeconds"] = start
        element["detail"]["particle"]["randomSeed"] = 171700 + ordinal
        # Keep the approved F HDR color, geometry, WPO/dissolve and lifetime.
        # These new occurrences change only timing, radius and seed.
        for module in element["sourceRecipe"]["modules"]:
            for index, distribution in enumerate(module["distributions"]):
                name = distribution["propertyPath"]
                if name == "startradius":
                    distribution["lookupTable"] = list(radius) * 3
        element["sourcePresentation"].update(
            sourceActionCueId="", sourceEventId="", sourceOccurrenceIndex=0,
            sourceTimeSeconds=start)
        waves.append(element)

    receipt = {"classification": "USER_REQUESTED_PROJECT_TUNING",
               "source": str(source_path.relative_to(ROOT)).replace("\\", "/"),
               "sourceSha256": hashlib.sha256(source_bytes).hexdigest(),
               "requestedVWaveCount": len(waves), "sourceBurstPerWave": 4,
               "unchangedF": True, "documents": []}
    for suffix, selected, offset in [
            ("full", waves, 0.0), ("clip2.full", waves[:3], 1.1667),
            ("clip3.full", waves[3:], 2.5)]:
        path = AUTHORED / f"effect.warlord.skill.17170.{suffix}.restore.effect.json"
        before_bytes = path.read_bytes()
        document = read(path)
        before_elements = copy.deepcopy(document["elements"])
        existing_ids = {e["id"] for e in before_elements}
        inserted = []
        repaired = []
        expected_existing = copy.deepcopy(before_elements)
        for element, expected in zip(document["elements"], expected_existing):
            if element["id"].startswith(PREFIX):
                fields = restore_f_color(element, original)
                assert restore_f_color(expected, original) == fields
                if fields:
                    repaired.append({"id": element["id"], "distributions": fields})
        for wave in selected:
            if wave["id"] in existing_ids:
                continue
            element = copy.deepcopy(wave)
            element["detail"]["timing"]["startDelaySeconds"] = round(
                element["detail"]["timing"]["startDelaySeconds"] - offset, 7)
            document["elements"].append(element)
            inserted.append(element["id"])
        assert document["elements"][:len(before_elements)] == expected_existing
        if inserted or repaired:
            install(path, before_bytes, document)
        receipt["documents"].append({
            "path": str(path.relative_to(ROOT)).replace("\\", "/"),
            "beforeSha256": hashlib.sha256(before_bytes).hexdigest(),
            "afterSha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "existingElementCount": len(before_elements), "insertedIds": inserted,
            "restoredFColor": repaired,
            "allOtherPreviousElementFieldsPreserved": True})
    duplicate_altv_lightning(receipt)
    assert source_path.read_bytes() == source_bytes
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "v-altv-lightning-resume.json").write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt, ensure_ascii=True))


if __name__ == "__main__":
    main()
