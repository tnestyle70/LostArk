"""Append the requested golden F-derived strikes without rebuilding tuned V rows.

The four extra waves are project-authored user tuning, not recovered V counts.
Existing elements (including positions/rotations), F, and the V startup clip are
preserved. Re-running is a no-op and never overwrites a subsequently tuned wave.
"""
from pathlib import Path
import copy
import hashlib
import json


ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / "Data/Effects/Authored"
OUT = ROOT / "out/ArtistWarlordVisualFollowup20260910/Warlord"
PREFIX = "authored.warlord.v.golden-guardian-lightning."


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
        # Keep the approved F geometry, WPO/dissolve, alpha and lifetime curves.
        # Only these new occurrences receive a fixed yellow hue and ring radius.
        for module in element["sourceRecipe"]["modules"]:
            for index, distribution in enumerate(module["distributions"]):
                name = distribution["propertyPath"]
                if name == "startcolor":
                    module["distributions"][index] = constant(distribution, [1.0, 0.72, 0.08])
                elif name == "colorscaleoverlife":
                    module["distributions"][index] = constant(distribution, [1.0, 1.0, 1.0])
                elif name == "startradius":
                    distribution["lookupTable"] = list(radius) * 3
        element["sourcePresentation"].update(
            sourceActionCueId="", sourceEventId="", sourceOccurrenceIndex=0,
            sourceTimeSeconds=start)
        waves.append(element)

    receipt = {"classification": "USER_REQUESTED_PROJECT_TUNING",
               "source": str(source_path.relative_to(ROOT)).replace("\\", "/"),
               "sourceSha256": hashlib.sha256(source_bytes).hexdigest(),
               "newWaveCount": len(waves), "sourceBurstPerWave": 4,
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
        for wave in selected:
            if wave["id"] in existing_ids:
                continue
            element = copy.deepcopy(wave)
            element["detail"]["timing"]["startDelaySeconds"] = round(
                element["detail"]["timing"]["startDelaySeconds"] - offset, 7)
            document["elements"].append(element)
            inserted.append(element["id"])
        assert document["elements"][:len(before_elements)] == before_elements
        if inserted:
            # Avoid clobbering an editor save that arrived after this read.
            assert path.read_bytes() == before_bytes, f"Document changed during patch: {path}"
            path.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                            encoding="utf-8")
        receipt["documents"].append({
            "path": str(path.relative_to(ROOT)).replace("\\", "/"),
            "beforeSha256": hashlib.sha256(before_bytes).hexdigest(),
            "afterSha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "preservedElementCount": len(before_elements), "insertedIds": inserted,
            "allPreviousElementFieldsPreserved": True})
    assert source_path.read_bytes() == source_bytes
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "v-lightning-patch.json").write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt, ensure_ascii=True))


if __name__ == "__main__":
    main()
