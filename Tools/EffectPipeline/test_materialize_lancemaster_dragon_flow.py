#!/usr/bin/env python3
"""Focused contracts for the Lance dragon opcode-19 materializer."""

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import shutil
import tempfile

import materialize_lancemaster_dragon_flow as subject


ROOT = Path(__file__).resolve().parents[2]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def copy_fixture(destination: Path) -> None:
    relative_files = [
        Path("Data/Effects/Imported/LanceMaster/Materials/") /
            "skill.34630-34650.dragon.exact-shader-targets.json",
        Path("Data/Effects/Imported/LanceMaster/Materials/") /
            "skill.34630-34650.dragon.exact-material-maps.receipt.json",
        Path("Data/Effects/AuthoredCorrections/Generated/LanceMaster/") /
            "effect.lancemaster.skill.34630-34650.dragon-flow."
            "materialization-receipt.v1.json",
    ]
    relative_files.extend(
        Path("Data/Effects/Authored") / f"{effect_id}.effect.json"
        for effect_id in sorted({row[0] for row in subject.ROWS})
    )
    relative_files.extend(Path("Client/Bin/Resources") / asset_id
                          for asset_id in subject.RESOURCE_SHA256)
    for relative in relative_files:
        source = ROOT / relative
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)


def exact_element(root: Path, effect_id: str, element_id: str) -> dict:
    document = json.loads((
        root / "Data/Effects/Authored" / f"{effect_id}.effect.json"
    ).read_text(encoding="utf-8-sig"))
    matches = [row for row in document["elements"] if row["id"] == element_id]
    require(len(matches) == 1, f"element is not singular: {element_id}")
    return matches[0]


def main() -> int:
    passed = 0

    changed, receipt = subject.run(ROOT, "check")
    require(not changed and receipt["runtimeMaterial"]["opcode"] == 19,
            "checked-in materialization is stale")
    print("[PASS] checked-in five-document materialization converges")
    passed += 1

    all_rows = []
    for effect_id in sorted({row[0] for row in subject.ROWS}):
        document = json.loads((
            ROOT / "Data/Effects/Authored" / f"{effect_id}.effect.json"
        ).read_text(encoding="utf-8-sig"))
        target_ids = {row[1] for row in subject.ROWS if row[0] == effect_id}
        all_rows.extend(row for row in document["elements"]
                        if row["id"] in target_ids)
    require(len(all_rows) == 12, "typed dragon denominator changed")
    require(sum(row["material"]["sourceMaterialPath"] ==
                subject.VARIANTS["body"]["child"] for row in all_rows) == 6,
            "body occurrence denominator changed")
    require(sum(row["material"]["sourceMaterialPath"] ==
                subject.VARIANTS["head"]["child"] for row in all_rows) == 6,
            "head occurrence denominator changed")
    require(all(
        row["material"]["execution"]["opcode"] == 19
        and row["material"]["execution"]["textureLaneCount"] == 5
        and row["material"]["execution"]["dynamicConsumedMask"] == 8
        and row["material"]["execution"]["particleColorConsumedMask"] == 15
        and row["material"]["renderProfile"] == "alpha_one_sided_depth_read"
        and row["sourceRecipe"]["rendererShape"] == "mesh"
        for row in all_rows
    ), "typed dragon packet/carrier contract changed")
    print("[PASS] exact six-body six-head opcode-19 denominator remains sealed")
    passed += 1

    for variant_id, variant in subject.VARIANTS.items():
        packet = subject.expected_execution(variant)
        roles = [lane["role"] for lane in packet["textureLanes"]]
        channels = [lane["sourceChannel"] for lane in packet["textureLanes"]]
        require(roles == ["normal_map", "alpha_map", "emission_map",
                          "diffuse_map", "specular_map"],
                f"lane order changed: {variant_id}")
        require(channels == ["RG", "R", "RGB", "RGB", "RGB"],
                f"lane channel contract changed: {variant_id}")
        require(packet["scalars"][5:11] == [
            {"name": name, "packedIndex": index + 5, "value": value}
            for index, (name, value) in enumerate(subject.SCALARS[5:11])
        ], f"independent alpha UV block changed: {variant_id}")
    print("[PASS] five named linear-wrap lanes and independent alpha UV remain exact")
    passed += 1

    exact = json.loads(subject.EXACT_RECEIPT.read_text(encoding="utf-8"))
    require(exact["summary"]["runtimeAdmissionCount"] == 0,
            "native runtime boundary changed")
    require(receipt["nativeOracle"]["runtimeAdmission"] is False,
            "semantic replay was mislabeled native")
    require(receipt["runtimeExecutor"] == "TYPED_HLSL_SEMANTIC_REPLAY"
            and receipt["runtimeFidelityClaim"] ==
            "TYPED_SEMANTIC_REPLAY_NOT_SOURCE_EXACT",
            "semantic replay was mislabeled source-exact runtime")
    require(receipt["boundedReplay"] == {
        "sceneColorAndDistortionMrtOnly": True,
        "sourceBlendIntent": "MASKED_ONE_SIDED_DEPTH_WRITE",
        "runtimeRenderState":
            "ALPHA_ONE_SIDED_DEPTH_READ_WITH_EXPLICIT_CLIP",
        "knownDifference": "NO_SOURCE_DEFERRED_GBUFFER_OR_DEPTH_WRITE",
    }, "bounded render limitation changed")
    print("[PASS] deferred native DXBC remains oracle-only and bounded replay is explicit")
    passed += 1

    checked_receipt = copy.deepcopy(receipt)
    artifact = checked_receipt.pop("artifactSha256")
    require(artifact == subject.canonical_sha256(checked_receipt),
            "receipt artifact hash changed")
    require(set(receipt["runtimeMaterial"]["resourceIdentity"]) ==
            set(subject.RESOURCE_SHA256), "resource receipt denominator changed")
    print("[PASS] generated receipt and eleven resource identities are content addressed")
    passed += 1

    before_hashes = {
        path: hashlib.sha256(path.read_bytes()).hexdigest()
        for path in [
            ROOT / "Data/Effects/Authored" / f"{effect_id}.effect.json"
            for effect_id in sorted({row[0] for row in subject.ROWS})
        ] + [subject.OUTPUT_RECEIPT]
    }
    wrote, _ = subject.run(ROOT, "write")
    after_hashes = {
        path: hashlib.sha256(path.read_bytes()).hexdigest()
        for path in before_hashes
    }
    require(not wrote and before_hashes == after_hashes,
            "idempotent write changed Product bytes")
    print("[PASS] idempotent write preserves all Product and receipt bytes")
    passed += 1

    with tempfile.TemporaryDirectory(prefix="lance-dragon-materializer-") as raw:
        fixture = Path(raw)
        copy_fixture(fixture)
        second = subject.ROWS[1]
        path = fixture / "Data/Effects/Authored" / f"{second[0]}.effect.json"
        document = json.loads(path.read_text(encoding="utf-8-sig"))
        target = next(row for row in document["elements"] if row["id"] == second[1])
        target["sourceNode"] += ".drift"
        path.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                        encoding="utf-8")
        before = path.read_bytes()
        try:
            subject.run(fixture, "write")
        except subject.DragonMaterializationError as error:
            require("sourceNode changed" in str(error),
                    "second-row sourceNode failure was not specific")
        else:
            raise AssertionError("second-row sourceNode drift was accepted")
        require(path.read_bytes() == before,
                "failed second-row materialization rewrote its Product document")
    print("[PASS] malformed second occurrence fails before any document commit")
    passed += 1

    with tempfile.TemporaryDirectory(prefix="lance-dragon-signature-") as raw:
        fixture = Path(raw)
        copy_fixture(fixture)
        second = subject.ROWS[1]
        path = fixture / "Data/Effects/Authored" / f"{second[0]}.effect.json"
        baseline = json.loads(path.read_text(encoding="utf-8-sig"))
        for field, value, expected_message in (
            ("renderProfile", "alpha_two_sided_depth_read",
             "post-promotion render profile changed"),
            ("rendererShape", "sprite", "MeshParticle carrier changed"),
        ):
            document = copy.deepcopy(baseline)
            target = next(row for row in document["elements"]
                          if row["id"] == second[1])
            if field == "renderProfile":
                target["material"][field] = value
            else:
                target["sourceRecipe"][field] = value
            path.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                            encoding="utf-8")
            try:
                subject.run(fixture, "check")
            except subject.DragonMaterializationError as error:
                require(expected_message in str(error),
                        f"signature failure was not specific: {field}/{error}")
            else:
                raise AssertionError(f"signature drift was accepted: {field}")
        path.write_text(json.dumps(baseline, ensure_ascii=False, indent=2) + "\n",
                        encoding="utf-8")
    print("[PASS] renderProfile and rendererShape signature drift fail closed")
    passed += 1

    print(f"{passed}/8 tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
