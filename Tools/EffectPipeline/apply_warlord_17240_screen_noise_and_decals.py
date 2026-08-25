#!/usr/bin/env python3
"""Remove Warlord T RGB noise and restore only its four source decals."""

from __future__ import annotations

import argparse
import codecs
import copy
import hashlib
import json
import os
import pathlib
import tempfile
from typing import Any, Callable


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
BA1_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.warlord.skill.17240.ba1.unified.effect.json"
)
BA3_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.warlord.skill.17240.ba3.unified.effect.json"
)
BA1_SOURCE_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Imported/LegacyRuntimeDonors/"
    "effect.warlord.skill.17240.ba1.unified."
    "e6e4b9daec3c478d38289cfc1eaf60de7ea6f035d5024f21b3376c5759cba62f."
    "effect.json"
)
BA3_SOURCE_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Imported/LegacyRuntimeDonors/"
    "effect.warlord.skill.17240.ba3.unified."
    "4565ef2375c333bd4fb031704a439a1d40189002ec84706c2e8b2fbf4e1ade05."
    "effect.json"
)
TRACK_A_RECEIPT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.track-a-restoration-receipt.json"
)

BA1_EFFECT_ID = "effect.warlord.skill.17240.ba1.unified"
BA3_EFFECT_ID = "effect.warlord.skill.17240.ba3.unified"
SCREEN_POST_ID = "authored.source-screenpost.dd9606e656d90f42cbf5ed32"
DECAL_IDS = (
    "authored.source-decal.578a68626d410bd3c097f6bc",  # emitter 55
    "authored.source-decal.bba5efca1ccce9b23fd5fd89",  # emitter 56
    "authored.source-decal.f350be88cb78699ae754b131",  # emitter 58
    "authored.source-decal.fe5c1334a86e6745792a8c43",  # emitter 57
)

EXPECTED_BA1_SOURCE_RAW_SHA256 = (
    "e6e4b9daec3c478d38289cfc1eaf60de7ea6f035d5024f21b3376c5759cba62f"
)
EXPECTED_BA3_SOURCE_RAW_SHA256 = (
    "4565ef2375c333bd4fb031704a439a1d40189002ec84706c2e8b2fbf4e1ade05"
)
EXPECTED_BA1_RETAINED_ROWS_SHA256 = (
    "19d7477420b79a09bbdbd1db7f912b31595b14acf9a06c5bd035997acc8b2eac"
)
EXPECTED_SCREEN_POST_SHA256 = (
    "4f43ca279437b18612547a489e41326e6113250cdb57af0d24582cb2bd34f345"
)
EXPECTED_BA3_RETAINED_ROWS_SHA256 = (
    "92909cd4d85cb48d10e5e42d136237cadc01e6d746de5d7c47498afd35bdc824"
)
EXPECTED_DECAL_ROWS_SHA256 = (
    "5349012cd112d0e209835fee510dc24935777a5ca27d2b687415dbd4a45548d0"
)
EXPECTED_RECEIPT_DECAL_ROWS_SHA256 = (
    "3e8d5f5b4d677e91c69555aaa0aa892df1da70581def2fe1bba10df9f2707228"
)
EXPECTED_DECAL_ROW_SHA256 = {
    DECAL_IDS[0]: "2c4e339b2b9466e12da9e88bd0c9100c99bb6ee186581ca60bd244b64f2cb223",
    DECAL_IDS[1]: "0beb619ec2efb20d1411b224cf09970fe045737addc2c3ba503c1ba40589b205",
    DECAL_IDS[2]: "22361ba6f31fc279b605bbd59d3df6e58cbd2b6daa7dfd5719331853b9d117ad",
    DECAL_IDS[3]: "69fc0783e37971f15859a93bd8348572427f9d13cf28d952df23010fe3873bcc",
}
EXPECTED_SOURCE_RECIPE_SHA256 = {
    DECAL_IDS[0]: "751347f30658b028c4af7e2be4438d5c939be6e2c0ae18f7f69141d07391d687",
    DECAL_IDS[1]: "1af535128b1ebef5eadc0bea527376aa6b055e8f5ef053be4adb67382b15be00",
    DECAL_IDS[2]: "5c0b577da1f73056e32620517f1e96a5d9f60caaec1a38b9dae2fbede05ff5de",
    DECAL_IDS[3]: "ef8e1c5ceda0fce8225e9420d32b6c2fab55fb6b6ec57bc20dffe4dfc709f861",
}
EXPECTED_NORMALIZED_RECIPE_SHA256 = {
    DECAL_IDS[0]: "d3edf66cac7c4007a9c06f101eda34919a86ac65cfc08aa42b8a2e039e497e06",
    DECAL_IDS[1]: "561c335218105bac58dc13af69709332fb5cba514c3b0a49b2231d4efe8bfd99",
    DECAL_IDS[2]: "cedc572a90680deb50629b09745f8a7168cacf5b289cab445b99043e9f17f928",
    DECAL_IDS[3]: "ac9202fd8906755c3d4e47ed03f7e5d12db24bd6652f5e44dbaa2fd4e1e6ba4e",
}
EXPECTED_SOURCE_ELEMENTS = {
    DECAL_IDS[0]: "fx_pc_wgl_07.par_s_wgl_cyclone_decal_01_fail.particlespriteemitter_55",
    DECAL_IDS[1]: "fx_pc_wgl_07.par_s_wgl_cyclone_decal_01_fail.particlespriteemitter_56",
    DECAL_IDS[2]: "fx_pc_wgl_07.par_s_wgl_cyclone_decal_01_fail.particlespriteemitter_58",
    DECAL_IDS[3]: "fx_pc_wgl_07.par_s_wgl_cyclone_decal_01_fail.particlespriteemitter_57",
}
EXPECTED_RESOURCES = [
    {
        "slotId": "base",
        "assetId": "Effect/Warlord/Textures/FX_TEX_02/fx_k_turtlediff_02.dds",
    },
    {
        "slotId": "noise",
        "assetId": "Effect/Warlord/Textures/FX_TEX_04/fx_i_noise_01.dds",
    },
    {
        "slotId": "mask",
        "assetId": "Effect/Warlord/Textures/FX_TEX_02/fx_d_environ_034.dds",
    },
    {
        "slotId": "emissive",
        "assetId": "Effect/Warlord/Textures/FX_TEX_01/fx_c_decal_002_1.dds",
    },
    {
        "slotId": "dissolve",
        "assetId": "Effect/Warlord/Textures/FX_TEX_01/fx_c_decal_002_2.dds",
    },
]
EXPECTED_PARTICLE_POSITIONS = (2.0, 4.25, 8.0, 6.5)
EXPECTED_RANDOM_SEEDS = (30, 31, 33, 32)


class Warlord17240CorrectionError(RuntimeError):
    """Raised when the reviewed Warlord T source or target has drifted."""


def _canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _raw_sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _load_text(path: pathlib.Path) -> tuple[bytes, str, dict[str, Any]]:
    raw = path.read_bytes()
    text = raw.decode("utf-8-sig")
    try:
        document = json.loads(text)
    except json.JSONDecodeError as error:
        raise Warlord17240CorrectionError(f"invalid JSON in {path}: {error}") from error
    if not isinstance(document, dict):
        raise Warlord17240CorrectionError(f"{path} root must be an object")
    return raw, text, document


def _require_unique_elements(document: dict[str, Any], label: str) -> list[dict[str, Any]]:
    elements = document.get("elements")
    if not isinstance(elements, list) or any(
        not isinstance(element, dict) for element in elements
    ):
        raise Warlord17240CorrectionError(f"{label} elements must be object rows")
    ids = [element.get("id") for element in elements]
    if any(not isinstance(element_id, str) or not element_id for element_id in ids):
        raise Warlord17240CorrectionError(f"{label} has an invalid element ID")
    if len(set(ids)) != len(ids):
        raise Warlord17240CorrectionError(f"{label} has duplicate element IDs")
    return elements


def _validate_screen_post(row: dict[str, Any]) -> None:
    if _canonical_sha256(row) != EXPECTED_SCREEN_POST_SHA256:
        raise Warlord17240CorrectionError("Warlord T RGB-noise source row changed")
    screen_post = row.get("detail", {}).get("screenPost", {})
    timing = row.get("detail", {}).get("timing", {})
    if (
        row.get("id") != SCREEN_POST_ID
        or row.get("kind") != "screenPost"
        or row.get("sourcePresentation", {}).get("sourceEventId") != "source-event-007"
        or screen_post.get("profileId") != "screen.rgb-noise.reconstructed.v1"
        or screen_post.get("enabled") is not True
        or timing.get("startDelaySeconds") != 0.3162
    ):
        raise Warlord17240CorrectionError("Warlord T removable row is not RGBNoise")


def _validate_decal(row: dict[str, Any], index: int) -> None:
    element_id = DECAL_IDS[index]
    if _canonical_sha256(row) != EXPECTED_DECAL_ROW_SHA256[element_id]:
        raise Warlord17240CorrectionError(f"source decal row changed: {element_id}")
    detail = row.get("detail", {})
    material = row.get("material", {})
    attachment = row.get("actionCueAttachment", {})
    recipe = row.get("sourceRecipe", {})
    particle = detail.get("particle", {})
    if row.get("id") != element_id or row.get("kind") != "decal":
        raise Warlord17240CorrectionError(f"source decal identity changed: {element_id}")
    if row.get("resources") != EXPECTED_RESOURCES:
        raise Warlord17240CorrectionError(f"source decal lanes changed: {element_id}")
    if (
        material.get("templateId") != "effect.standard"
        or material.get("sourceMaterialPath")
        != "fx_m_mi_04.fx_mi.fx_d_de_master_01_81_tr"
        or material.get("renderProfile") != "alpha_two_sided_depth_read"
        or material.get("sourceProfile") != {"enabled": False}
    ):
        raise Warlord17240CorrectionError(f"source decal material changed: {element_id}")
    if (
        attachment.get("enabled") is not True
        or attachment.get("follow") is not False
        or attachment.get("runtimeAnchorSlotId") != "root"
    ):
        raise Warlord17240CorrectionError(f"source decal attachment changed: {element_id}")
    if detail.get("timing") != {
        "startDelaySeconds": 0.1200000000000001,
        "lifeTimeSeconds": 2.0,
        "afterImageSeconds": 0.0,
        "dissolveStartNormalized": 1.0,
    }:
        raise Warlord17240CorrectionError(f"source decal timing changed: {element_id}")
    if detail.get("decal") != {"size": [4.5, 4.5], "depth": 0.25}:
        raise Warlord17240CorrectionError(f"source decal projection changed: {element_id}")
    position = EXPECTED_PARTICLE_POSITIONS[index]
    if (
        particle.get("randomSeed") != EXPECTED_RANDOM_SEEDS[index]
        or particle.get("lifeTimeSeconds") != [4.0, 5.0]
        or particle.get("initialPositionMin") != [position, 0.0, -0.0]
        or particle.get("initialPositionMax") != [position, 0.0, -0.0]
        or particle.get("startSize") != [4.5, 4.5]
    ):
        raise Warlord17240CorrectionError(f"source decal placement changed: {element_id}")
    if (
        recipe.get("enabled") is not True
        or recipe.get("rendererShape") != "decal"
        or recipe.get("emitterDelaySeconds") != 0
        or recipe.get("emitterDurationSeconds") != 2.0
        or recipe.get("emitterLoopCount") != 1
        or recipe.get("bursts")
        != [{"timeSeconds": 0.0, "countMinimum": 1, "countMaximum": 1}]
        or _canonical_sha256(recipe)
        != EXPECTED_NORMALIZED_RECIPE_SHA256[element_id]
    ):
        raise Warlord17240CorrectionError(f"source decal recipe changed: {element_id}")


def load_source_evidence(
    *,
    ba1_source_path: pathlib.Path = BA1_SOURCE_PATH,
    ba3_source_path: pathlib.Path = BA3_SOURCE_PATH,
    receipt_path: pathlib.Path = TRACK_A_RECEIPT_PATH,
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    if _raw_sha256(ba1_source_path) != EXPECTED_BA1_SOURCE_RAW_SHA256:
        raise Warlord17240CorrectionError("sealed Warlord T BA1 evidence changed")
    _, _, ba1_source = _load_text(ba1_source_path)
    if ba1_source.get("effectAssetId") != BA1_EFFECT_ID:
        raise Warlord17240CorrectionError("sealed Warlord T BA1 identity changed")
    ba1_rows = _require_unique_elements(ba1_source, "sealed Warlord T BA1")
    if (
        len(ba1_rows) != 25
        or _canonical_sha256(ba1_rows[:24]) != EXPECTED_BA1_RETAINED_ROWS_SHA256
        or ba1_rows[-1].get("id") != SCREEN_POST_ID
    ):
        raise Warlord17240CorrectionError("sealed Warlord T BA1 denominator changed")
    _validate_screen_post(ba1_rows[-1])

    if _raw_sha256(ba3_source_path) != EXPECTED_BA3_SOURCE_RAW_SHA256:
        raise Warlord17240CorrectionError("sealed Warlord T BA3 evidence changed")
    _, _, ba3_source = _load_text(ba3_source_path)
    if ba3_source.get("effectAssetId") != BA3_EFFECT_ID:
        raise Warlord17240CorrectionError("sealed Warlord T BA3 identity changed")
    ba3_rows = _require_unique_elements(ba3_source, "sealed Warlord T BA3")
    indexed = {row["id"]: row for row in ba3_rows}
    try:
        decal_rows = [indexed[element_id] for element_id in DECAL_IDS]
    except KeyError as error:
        raise Warlord17240CorrectionError(
            f"sealed Warlord T BA3 decal is missing: {error.args[0]}"
        ) from error
    if _canonical_sha256(decal_rows) != EXPECTED_DECAL_ROWS_SHA256:
        raise Warlord17240CorrectionError("sealed Warlord T BA3 decal set changed")
    for index, row in enumerate(decal_rows):
        _validate_decal(row, index)

    _, _, receipt = _load_text(receipt_path)
    matches = [
        target
        for target in receipt.get("targets", [])
        if isinstance(target, dict)
        and target.get("targetEffectAssetId") == BA3_EFFECT_ID
    ]
    if len(matches) != 1:
        raise Warlord17240CorrectionError(
            "Track-A receipt must own exactly one Warlord T BA3 target"
        )
    receipt_rows = matches[0].get("sourceDecalRows")
    if (
        not isinstance(receipt_rows, list)
        or _canonical_sha256(receipt_rows) != EXPECTED_RECEIPT_DECAL_ROWS_SHA256
        or [row.get("targetElementId") for row in receipt_rows] != list(DECAL_IDS)
    ):
        raise Warlord17240CorrectionError("Track-A Warlord T decal receipt changed")
    for row, decal in zip(receipt_rows, decal_rows):
        element_id = row["targetElementId"]
        if (
            row.get("sourceEffectAssetId") != "effect.warlord.skill.17240.imported"
            or row.get("sourceElementId") != EXPECTED_SOURCE_ELEMENTS[element_id]
            or row.get("sourceEventId") != "source-event-019"
            or row.get("sourceRecipeCanonicalSha256")
            != EXPECTED_SOURCE_RECIPE_SHA256[element_id]
            or row.get("normalizedRecipeCanonicalSha256")
            != EXPECTED_NORMALIZED_RECIPE_SHA256[element_id]
            or row.get("normalizedRecipeCanonicalSha256")
            != _canonical_sha256(decal["sourceRecipe"])
            or row.get("sourceBindings") != EXPECTED_RESOURCES
            or row.get("targetBindings") != EXPECTED_RESOURCES
            or row.get("baseStatus") != "SOURCE_OR_ARTIST_BOUND"
        ):
            raise Warlord17240CorrectionError(
                f"Track-A Warlord T decal evidence changed: {element_id}"
            )
    return copy.deepcopy(ba1_rows[-1]), copy.deepcopy(decal_rows)


def validate_ba1(document: dict[str, Any], screen_post: dict[str, Any]) -> bool:
    if document.get("effectAssetId") != BA1_EFFECT_ID:
        raise Warlord17240CorrectionError("Warlord T BA1 identity changed")
    elements = _require_unique_elements(document, "Warlord T BA1")
    if len(elements) not in (24, 25):
        raise Warlord17240CorrectionError("Warlord T BA1 must contain 24 + RGBNoise")
    if _canonical_sha256(elements[:24]) != EXPECTED_BA1_RETAINED_ROWS_SHA256:
        raise Warlord17240CorrectionError("Warlord T BA1 retained 24 rows changed")
    if len(elements) == 24:
        if any(row["id"] == SCREEN_POST_ID for row in elements):
            raise Warlord17240CorrectionError("Warlord T RGBNoise displaced a retained row")
        return False
    if elements[-1] != screen_post or elements[-1]["id"] != SCREEN_POST_ID:
        raise Warlord17240CorrectionError("Warlord T BA1 extra row is not exact RGBNoise")
    return True


def validate_ba3(document: dict[str, Any], decals: list[dict[str, Any]]) -> bool:
    if document.get("effectAssetId") != BA3_EFFECT_ID:
        raise Warlord17240CorrectionError("Warlord T BA3 identity changed")
    elements = _require_unique_elements(document, "Warlord T BA3")
    if len(elements) not in (8, 12):
        raise Warlord17240CorrectionError("Warlord T BA3 must contain 8 + four decals")
    if _canonical_sha256(elements[:8]) != EXPECTED_BA3_RETAINED_ROWS_SHA256:
        raise Warlord17240CorrectionError("Warlord T BA3 retained eight rows changed")
    if len(elements) == 8:
        if any(row["id"] in DECAL_IDS for row in elements):
            raise Warlord17240CorrectionError("Warlord T decal displaced a retained row")
        return False
    if elements[8:] != decals:
        raise Warlord17240CorrectionError(
            "Warlord T BA3 decal set is duplicated, reordered, or changed"
        )
    return True


def _elements_array_bounds(text: str) -> tuple[int, int]:
    marker = text.find('"elements"')
    opening = text.find("[", marker)
    if marker < 0 or opening < 0:
        raise Warlord17240CorrectionError("elements array is missing")
    depth = 0
    in_string = False
    escaped = False
    for index in range(opening, len(text)):
        character = text[index]
        if in_string:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
            continue
        if character == '"':
            in_string = True
        elif character == "[":
            depth += 1
        elif character == "]":
            depth -= 1
            if depth == 0:
                return opening + 1, index
    raise Warlord17240CorrectionError("elements array is unterminated")


def _top_level_object_spans(text: str, start: int, end: int) -> list[tuple[int, int]]:
    spans: list[tuple[int, int]] = []
    cursor = start
    while cursor < end:
        while cursor < end and (text[cursor].isspace() or text[cursor] == ","):
            cursor += 1
        if cursor >= end:
            break
        if text[cursor] != "{":
            raise Warlord17240CorrectionError("elements array contains a non-object row")
        object_start = cursor
        depth = 0
        in_string = False
        escaped = False
        while cursor < end:
            character = text[cursor]
            if in_string:
                if escaped:
                    escaped = False
                elif character == "\\":
                    escaped = True
                elif character == '"':
                    in_string = False
            elif character == '"':
                in_string = True
            elif character == "{":
                depth += 1
            elif character == "}":
                depth -= 1
                if depth == 0:
                    cursor += 1
                    spans.append((object_start, cursor))
                    break
            cursor += 1
        else:
            raise Warlord17240CorrectionError("elements row is unterminated")
    return spans


def build_ba1_text(
    original_text: str,
    document: dict[str, Any],
    screen_post: dict[str, Any],
) -> str:
    if not validate_ba1(document, screen_post):
        return original_text
    start, end = _elements_array_bounds(original_text)
    spans = _top_level_object_spans(original_text, start, end)
    if len(spans) != 25:
        raise Warlord17240CorrectionError("Warlord T BA1 text row count changed")
    object_start, object_end = spans[-1]
    comma = object_start - 1
    while comma >= start and original_text[comma].isspace():
        comma -= 1
    if comma < start or original_text[comma] != ",":
        raise Warlord17240CorrectionError("Warlord T RGBNoise separator changed")
    result = original_text[:comma] + original_text[object_end:]
    try:
        staged = json.loads(result)
    except json.JSONDecodeError as error:
        raise Warlord17240CorrectionError(
            f"staged Warlord T BA1 JSON changed: {error}"
        ) from error
    if validate_ba1(staged, screen_post):
        raise Warlord17240CorrectionError("Warlord T RGBNoise was not removed")
    return result


def build_ba3_text(
    original_text: str,
    document: dict[str, Any],
    decals: list[dict[str, Any]],
) -> str:
    if validate_ba3(document, decals):
        return original_text
    start, end = _elements_array_bounds(original_text)
    array_text = original_text[start:end]
    body_end = len(array_text.rstrip())
    if body_end == 0 or array_text[:body_end][-1] != "}":
        raise Warlord17240CorrectionError("Warlord T BA3 array layout changed")
    newline = "\r\n" if "\r\n" in original_text else "\n"
    rendered_rows = []
    for row in decals:
        rendered = json.dumps(row, ensure_ascii=False, indent=2, allow_nan=False)
        rendered_rows.append(newline.join("    " + line for line in rendered.splitlines()))
    migrated_array = (
        array_text[:body_end]
        + ","
        + newline
        + ("," + newline).join(rendered_rows)
        + array_text[body_end:]
    )
    result = original_text[:start] + migrated_array + original_text[end:]
    try:
        staged = json.loads(result)
    except json.JSONDecodeError as error:
        raise Warlord17240CorrectionError(
            f"staged Warlord T BA3 JSON changed: {error}"
        ) from error
    if not validate_ba3(staged, decals):
        raise Warlord17240CorrectionError("Warlord T decals were not admitted")
    return result


def _stage_text(path: pathlib.Path, text: str, *, bom: bool) -> pathlib.Path:
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    temporary = pathlib.Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            if bom:
                stream.write(codecs.BOM_UTF8)
            stream.write(text.encode("utf-8"))
            stream.flush()
            os.fsync(stream.fileno())
    except Exception:
        temporary.unlink(missing_ok=True)
        raise
    return temporary


def _transactional_replace(
    updates: list[tuple[pathlib.Path, str, bool]],
    *,
    replace: Callable[[os.PathLike[str], os.PathLike[str]], None] = os.replace,
) -> None:
    staged: dict[pathlib.Path, pathlib.Path] = {}
    originals = {path: path.read_bytes() for path, _, _ in updates}
    committed: list[pathlib.Path] = []
    try:
        for path, text, bom in updates:
            staged[path] = _stage_text(path, text, bom=bom)
        for path, _, _ in updates:
            replace(staged[path], path)
            staged.pop(path, None)
            committed.append(path)
    except Exception as error:
        rollback_errors: list[str] = []
        for path in reversed(committed):
            restore = _stage_text(
                path,
                originals[path].decode("utf-8-sig"),
                bom=originals[path].startswith(codecs.BOM_UTF8),
            )
            try:
                os.replace(restore, path)
            except OSError as rollback_error:
                restore.unlink(missing_ok=True)
                rollback_errors.append(f"{path}: {rollback_error}")
        if rollback_errors:
            raise Warlord17240CorrectionError(
                "Warlord T update failed and rollback was incomplete: "
                + "; ".join(rollback_errors)
            ) from error
        raise
    finally:
        for temporary in staged.values():
            temporary.unlink(missing_ok=True)


def run(
    *,
    write: bool,
    ba1_path: pathlib.Path = BA1_PATH,
    ba3_path: pathlib.Path = BA3_PATH,
    ba1_source_path: pathlib.Path = BA1_SOURCE_PATH,
    ba3_source_path: pathlib.Path = BA3_SOURCE_PATH,
    receipt_path: pathlib.Path = TRACK_A_RECEIPT_PATH,
    replace: Callable[[os.PathLike[str], os.PathLike[str]], None] = os.replace,
) -> bool:
    screen_post, decals = load_source_evidence(
        ba1_source_path=ba1_source_path,
        ba3_source_path=ba3_source_path,
        receipt_path=receipt_path,
    )
    ba1_raw, ba1_original, ba1 = _load_text(ba1_path)
    ba3_raw, ba3_original, ba3 = _load_text(ba3_path)
    ba1_rendered = build_ba1_text(ba1_original, ba1, screen_post)
    ba3_rendered = build_ba3_text(ba3_original, ba3, decals)
    updates = []
    if ba1_rendered != ba1_original:
        updates.append((ba1_path, ba1_rendered, ba1_raw.startswith(codecs.BOM_UTF8)))
    if ba3_rendered != ba3_original:
        updates.append((ba3_path, ba3_rendered, ba3_raw.startswith(codecs.BOM_UTF8)))
    if updates and not write:
        raise Warlord17240CorrectionError(
            "Warlord T correction is missing; rerun with --write"
        )
    if updates:
        _transactional_replace(updates, replace=replace)
    _, _, committed_ba1 = _load_text(ba1_path)
    _, _, committed_ba3 = _load_text(ba3_path)
    if validate_ba1(committed_ba1, screen_post):
        raise Warlord17240CorrectionError("committed Warlord T BA1 still has RGBNoise")
    if not validate_ba3(committed_ba3, decals):
        raise Warlord17240CorrectionError("committed Warlord T BA3 lacks source decals")
    return bool(updates)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    arguments = parser.parse_args()
    try:
        changed = run(write=arguments.write)
    except (Warlord17240CorrectionError, OSError) as error:
        print(f"ERROR: {error}")
        return 1
    print(
        ("updated" if changed else "check passed")
        + ": Warlord T RGBNoise removed and four exact source decals restored"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
