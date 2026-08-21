#!/usr/bin/env python3
"""Verify the Artist D authored -> animevent -> catalog join closure."""

from __future__ import annotations

import argparse
import json
import pathlib
import re
from collections import Counter
from typing import Any


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
SKILL_BINDINGS_PATH = REPOSITORY_ROOT / (
    "Data/Animation/Authored/Artist/Artist.skillbindings.json"
)
ANIMEVENTS_PATH = REPOSITORY_ROOT / (
    "Data/Animation/Authored/Artist/Artist.animevents"
)
CATALOG_PATH = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
AUTHORED_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31490.unified.effect.json"
)
RUNTIME_CATALOG_PATH = REPOSITORY_ROOT / (
    "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
)

SKILL_ID = 31490
CLIP = "sdm_sk_cloudtiger"
EFFECT_ID = "effect.artist.skill.31490.unified"
AUTHORING_PATH = "Effects/Authored/effect.artist.skill.31490.unified.effect.json"
TIGER_OPCODE = 18
TIGER_IDS = (
    "authored.source-particle.763aea38ab1100ba9072dbfb",
    "authored.source-particle.e6c3ffec9fbc27024e2ce78c",
    "authored.source-particle.91392dd3a1710c9d411bfff6",
    "authored.source-particle.382ed3229ddf083cfd22ee11",
    "authored.source-particle.4f0381d175d441978f26ebfc",
    "authored.source-particle.31fa700c084ab0b11447f7c7",
    "authored.source-particle.5571970d95f97aecb889fed7",
    "authored.source-particle.87c8abd0423fcb7e9a725659",
    "authored.source-particle.ac2d4d3e467dc4442cba60c3",
    "authored.source-particle.01c398219f73706b66509e77",
    "authored.source-particle.93420edbc5815b8a01b38ef4",
    "authored.source-particle.76d0b67fe194395ce21c51ab",
)
CHILD5_PATH = "fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_5_ad"
CHILD6_PATH = "fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_6_ad"
EXPECTED_CUE = (
    '"sdm_sk_cloudtiger" EFFECT startms=0 '
    'payload="effect.artist.skill.31490.unified" effectref=asset '
    'anchor="root" follow=follow stop=natural '
    "px=0 py=0 pz=0 rx=0 ry=0 rz=0 sx=1 sy=1 sz=1"
)


class ArtistJoinError(RuntimeError):
    """Raised when any stable product join is missing or ambiguous."""


def _object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ArtistJoinError(f"{label} must be an object")
    return value


def load_json(path: pathlib.Path) -> dict[str, Any]:
    try:
        return _object(json.loads(path.read_text(encoding="utf-8-sig")), str(path))
    except (OSError, json.JSONDecodeError) as error:
        raise ArtistJoinError(f"cannot load {path}: {error}") from error


def validate_documents(
    skill_bindings: dict[str, Any],
    animevents_text: str,
    catalog: dict[str, Any],
    authored: dict[str, Any],
) -> None:
    bindings = [
        row
        for row in skill_bindings.get("bindings", [])
        if isinstance(row, dict) and row.get("skillId") == SKILL_ID
    ]
    if len(bindings) != 1 or bindings[0].get("clips") != [CLIP]:
        raise ArtistJoinError("Artist 31490 must bind exactly sdm_sk_cloudtiger")

    lines = animevents_text.splitlines()
    if not lines:
        raise ArtistJoinError("Artist animevents is empty")
    header = re.fullmatch(r'LOSTARK_ANIM_EVENTS\s+([56])\s+"Artist"\s+(\d+)', lines[0])
    if header is None or int(header.group(2)) != len(lines) - 1:
        raise ArtistJoinError("Artist animevents header/count changed")
    product_cues = [
        line
        for line in lines[1:]
        if line.startswith(f'"{CLIP}" EFFECT ')
        and " effectref=asset " in f" {line} "
    ]
    if product_cues != [EXPECTED_CUE]:
        raise ArtistJoinError("Artist 31490 Product cue is missing or duplicated")

    rows = [
        row
        for row in catalog.get("effects", [])
        if isinstance(row, dict) and row.get("effectAssetId") == EFFECT_ID
    ]
    if rows != [
        {
            "effectAssetId": EFFECT_ID,
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": AUTHORING_PATH,
        }
    ]:
        raise ArtistJoinError("Artist 31490 catalog row is missing or ambiguous")

    if authored.get("effectAssetId") != EFFECT_ID or authored.get("version") != 13:
        raise ArtistJoinError("Artist 31490 authored identity/version changed")
    elements = authored.get("elements")
    if not isinstance(elements, list) or len(elements) != 68:
        raise ArtistJoinError("Artist 31490 authored cardinality must remain 68")
    element_ids = [row.get("id") for row in elements if isinstance(row, dict)]
    if len(element_ids) != 68 or len(set(element_ids)) != 68 or any(
        not isinstance(element_id, str) or not element_id for element_id in element_ids
    ):
        raise ArtistJoinError("Artist 31490 element IDs must be unique and stable")
    kinds = Counter(row.get("kind") for row in elements)
    if kinds != Counter({"particle": 60, "decal": 8}):
        raise ArtistJoinError("Artist 31490 60 particle + 8 decal closure changed")

    elements_by_id = {row["id"]: row for row in elements}
    if not set(TIGER_IDS) <= set(elements_by_id):
        raise ArtistJoinError("Artist 31490 tiger occurrence allowlist is incomplete")
    child_counts: Counter[str] = Counter()
    for element_id in TIGER_IDS:
        element = elements_by_id[element_id]
        material = element.get("material")
        recipe = element.get("sourceRecipe")
        execution = material.get("execution") if isinstance(material, dict) else None
        lanes = execution.get("textureLanes") if isinstance(execution, dict) else None
        if (
            element.get("kind") != "particle"
            or element.get("visible") is not True
            or not isinstance(recipe, dict)
            or recipe.get("enabled") is not True
            or recipe.get("rendererShape") != "sprite"
            or not isinstance(material, dict)
            or material.get("templateId") != "effect.standard"
            or material.get("renderProfile") != "additive_two_sided_depth_read"
            or material.get("sourceProfile") != {"enabled": False}
            or not isinstance(execution, dict)
            or execution.get("enabled") is not True
            or execution.get("backend") != "runtimeMaterialV2"
            or execution.get("opcode") != TIGER_OPCODE
            or execution.get("passIndex") != 2
            or execution.get("textureLaneCount") != 3
            or execution.get("textureMask") != 7
            or not isinstance(lanes, list)
            or [row.get("role") for row in lanes]
            != ["maintex", "uv_noise_tex", "dissolve_tex_01"]
            or [row.get("sourceChannel") for row in lanes] != ["RGB", "RG", "R"]
            or [row.get("colorSpace") for row in lanes]
            != ["linear", "linear", "linear"]
            or execution.get("dynamicConsumedMask") != 15
            or execution.get("dynamicSuppressedMask") != 0
            or execution.get("particleColorConsumedMask") != 15
            or execution.get("particleColorSuppressedMask") != 0
            or execution.get("staticInputCount") != 0
            or execution.get("renderConsumedMask") != 47
            or execution.get("renderSuppressedMask") != 16
        ):
            raise ArtistJoinError(
                f"Artist 31490 tiger typed packet changed: {element_id}"
            )
        child_path = material.get("sourceMaterialPath")
        expected_scalar_count = 28 if child_path == CHILD5_PATH else 24
        if child_path not in {CHILD5_PATH, CHILD6_PATH} or (
            execution.get("scalarCount") != expected_scalar_count
            or execution.get("inputCount") != expected_scalar_count
        ):
            raise ArtistJoinError(
                f"Artist 31490 tiger child packet changed: {element_id}"
            )
        child_counts[child_path] += 1
    if child_counts != Counter({CHILD6_PATH: 8, CHILD5_PATH: 4}):
        raise ArtistJoinError("Artist 31490 tiger child5/child6 split changed")
    escaped_opcode = [
        row["id"]
        for row in elements
        if row["id"] not in TIGER_IDS
        and row.get("material", {}).get("execution", {}).get("opcode")
        == TIGER_OPCODE
    ]
    if escaped_opcode:
        raise ArtistJoinError(
            "Artist 31490 tiger opcode escaped its exact allowlist: "
            + ", ".join(escaped_opcode)
        )


def runtime_has_target(runtime_catalog: dict[str, Any]) -> bool:
    rows = runtime_catalog.get("effects")
    if not isinstance(rows, list):
        raise ArtistJoinError("runtime catalog effects must be an array")
    matches = [
        row
        for row in rows
        if isinstance(row, dict) and row.get("effectAssetId") == EFFECT_ID
    ]
    if len(matches) > 1:
        raise ArtistJoinError("runtime catalog duplicates Artist 31490")
    return len(matches) == 1


def run(*, require_runtime: bool = False) -> bool:
    try:
        animevents_text = ANIMEVENTS_PATH.read_text(encoding="utf-8-sig")
    except OSError as error:
        raise ArtistJoinError(f"cannot load {ANIMEVENTS_PATH}: {error}") from error
    validate_documents(
        load_json(SKILL_BINDINGS_PATH),
        animevents_text,
        load_json(CATALOG_PATH),
        load_json(AUTHORED_PATH),
    )
    published = False
    if RUNTIME_CATALOG_PATH.exists():
        published = runtime_has_target(load_json(RUNTIME_CATALOG_PATH))
    if require_runtime and not published:
        raise ArtistJoinError("Artist 31490 is authored/cued but not runtime published")
    return published


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--require-runtime", action="store_true")
    arguments = parser.parse_args()
    try:
        published = run(require_runtime=arguments.require_runtime)
    except ArtistJoinError as error:
        print(f"ERROR: {error}")
        return 1
    print(
        "PASS: Artist 31490 authored/cue/catalog closure "
        + ("is runtime published" if published else "is ready for full publish")
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
