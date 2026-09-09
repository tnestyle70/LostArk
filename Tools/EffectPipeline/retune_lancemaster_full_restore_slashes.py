#!/usr/bin/env python3
"""Apply the user's A-swing substitution while preserving unrelated saved elements."""
from __future__ import annotations
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / "Data/Effects/Authored"
OUT = ROOT / "out/LanceMasterSlashFollowup20260910"


def read(path):
    return json.loads(path.read_bytes())


def source_system(element):
    return element.get("sourceNode", "").split("|source:")[-1].rsplit(".particlespriteemitter_", 1)[0]


def start(element):
    return element["detail"]["timing"]["startDelaySeconds"]


def copy_at(element, target, occurrence, time, label):
    value = copy.deepcopy(element)
    digest = hashlib.sha256(f"{target}|{occurrence}|{element['id']}".encode()).hexdigest()[:24]
    value["id"] = "authored.lance.user-swing." + digest
    value["sourceNode"] = "authored-copy:" + element["id"]
    value["displayName"] = label + " / " + element["displayName"]
    value["detail"]["timing"]["startDelaySeconds"] = time
    value["sourcePresentation"] = {"enabled": False}
    return value


def atomic_write(path, value, expected):
    output = json.dumps(value, ensure_ascii=False, indent=2).encode("utf8")
    fd, temp = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(output)
        if path.read_bytes() != expected:
            raise ValueError("Concurrent saved document edit: " + str(path))
        os.replace(temp, path)
    finally:
        if os.path.exists(temp):
            os.unlink(temp)
    return output


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    a_path = AUTHORED / "effect.lancemaster.skill.34140.ba1.clip1.full.restore.effect.json"
    a = read(a_path)
    template = [e for e in a["elements"] if source_system(e) == "fx_pc_flm_02.par_o_flm_chestdestruction_01_1"]
    assert len(template) == 4 and all(e["sourceRecipe"]["rendererShape"] == "mesh" for e in template)
    v_path = AUTHORED / "effect.lancemaster.skill.34610.clip1.full.restore.effect.json"
    v = read(v_path)
    spear = [e for e in v["elements"] if source_system(e) == "fx_pc_flm_00.par_k_flm_squalllance_spear_00"]
    assert len(spear) == 11
    receipts, staged = [], []
    rules = []
    for stage in range(1, 5):
        rules.append((f"34010.ba{stage}", "par_m_flm_pyungmtrail_01", "all"))
    for stage in range(1, 3):
        rules.append((f"34040.clip{stage}", "par_m_flm_mtrail_02", "all"))
    rules += [("34090", "par_s_flm_dragon_trail_01_1", "mesh"),
              ("34100.clip1", "par_k_flm_cycloniclance_shoulder_00", "append"),
              ("34100.clip2", "par_v_flm_cycloniclance_atk_00", "E2"),
              ("34100.clip3", "par_v_flm_dragon_trail_00", "all"),
              ("34160.ba1", "par_s_flm_riseup_trail_01_1", "mesh"),
              ("34160.ba2", "par_s_flm_riseup_trail_02_1", "mesh"),
              ("34120.clip1", "par_s_flm_ttstrike_trail_01_1", "all"),
              ("34120.clip2", "par_s_flm_ttstrike_trail_01_2", "all"),
              ("34120.clip3", "par_s_flm_ttstrike_trail_01_2", "all")]
    for suffix, system, mode in rules:
        path = AUTHORED / f"effect.lancemaster.skill.{suffix}.full.restore.effect.json"
        original_bytes = path.read_bytes()
        document = json.loads(original_bytes)
        if any(e["id"].startswith("authored.lance.user-swing.") for e in document["elements"]):
            raise ValueError("Already tuned; do not overwrite subsequent user edits: " + suffix)
        group = [e for e in document["elements"] if source_system(e).endswith("." + system)]
        assert group, (suffix, system)
        if mode == "append":
            removed = []
        elif mode == "mesh":
            removed = [e for e in group if e["sourceRecipe"]["rendererShape"] == "mesh"]
        elif mode == "E2":
            removed = [e for e in group if e["material"]["sourceProfile"]["runtimeShaderProfileId"] in
                       {"effect.ue3.lance-va-1226-native.v1", "effect.ue3.lance-va-596-native.v1",
                        "effect.ue3.lance-va-615-native.v1"}]
        else:
            removed = group
        # Recipe delay is an intra-emitter offset. The original Action timing is
        # the shared Detail start, so repeated R/S occurrences remain distinct.
        times = sorted({start(e) for e in group})
        if suffix == "34160.ba1":
            times = sorted({start(e) for e in group if "fswing" in e["material"]["sourceMaterialPath"]})
        if suffix.startswith("34120"):
            expected = 2 if suffix.endswith("clip3") else 1
            assert len(times) == expected, (suffix, times)
        else:
            assert len(times) == (5 if suffix == "34160.ba1" else 1), (suffix, times)
        removed_ids = {e["id"] for e in removed}
        updated = copy.deepcopy(document)
        updated["elements"] = [e for e in updated["elements"] if e["id"] not in removed_ids]
        added = [copy_at(e, document["effectAssetId"], f"A-swing-{index + 1}", time,
                         f"A swing {index + 1}") for index, time in enumerate(times) for e in template]
        updated["elements"].extend(added)
        retained = [e for e in document["elements"] if e["id"] not in removed_ids]
        assert updated["elements"][:-len(added)] == retained
        assert {k: val for k, val in updated.items() if k != "elements"} == {
            k: val for k, val in document.items() if k != "elements"}
        staged.append((path, original_bytes, updated))
        receipts.append(dict(document=document["effectAssetId"], policy="USER_REQUESTED_A_FIRST_SWING_SUBSTITUTION",
                             sourceTemplateDocument=a["effectAssetId"], templateElements=[e["id"] for e in template],
                             sourceCueTimes=times, removedElements=[e["id"] for e in removed],
                             addedElements=[e["id"] for e in added], preservedElements=len(retained)))
    path = AUTHORED / "effect.lancemaster.skill.34610.clip3.full.restore.effect.json"
    original_bytes = path.read_bytes()
    document = json.loads(original_bytes)
    assert not any(e["id"].startswith("authored.lance.user-swing.") for e in document["elements"])
    updated = copy.deepcopy(document)
    dash = next(e for e in document["elements"] if source_system(e).endswith(".par_k_flm_squalllance_dash_00"))
    time = start(dash)
    added = [copy_at(e, document["effectAssetId"], "first-spear-at-final-dash", time, "V first spear / final dash") for e in spear]
    updated["elements"].extend(added)
    staged.append((path, original_bytes, updated))
    receipts.append(dict(document=document["effectAssetId"], policy="USER_REQUESTED_FIRST_SPEAR_AT_FINAL_DASH",
                         sourceTemplateDocument=v["effectAssetId"], templateElements=[e["id"] for e in spear],
                         sourceCueTimes=[time], removedElements=[], addedElements=[e["id"] for e in added],
                         preservedElements=len(document["elements"])))
    # Preflight every target before writing the first one.
    for path, before, updated in staged:
        ids = {e["id"] for e in updated["elements"]}
        assert len(ids) == len(updated["elements"])
        for e in updated["elements"]:
            inheritance = e.get("transformInheritance", {})
            assert not inheritance.get("enabled") or inheritance["masterElementId"] in ids
        backup = OUT / "before" / path.relative_to(ROOT)
        if backup.exists() and backup.read_bytes() != before:
            raise ValueError("Existing baseline differs: " + str(backup))
        backup.parent.mkdir(parents=True, exist_ok=True)
        backup.write_bytes(before)
    for (path, before, updated), receipt in zip(staged, receipts):
        after = atomic_write(path, updated, before)
        receipt.update(beforeSha256=hashlib.sha256(before).hexdigest(),
                       afterSha256=hashlib.sha256(after).hexdigest(), finalElements=len(updated["elements"]))
    (OUT / "slash-materialize-result.json").write_text(json.dumps(receipts, indent=2), encoding="utf8")
    print(json.dumps(dict(documents=len(receipts), removed=sum(len(r["removedElements"]) for r in receipts),
                         added=sum(len(r["addedElements"]) for r in receipts))))


if __name__ == "__main__":
    main()
