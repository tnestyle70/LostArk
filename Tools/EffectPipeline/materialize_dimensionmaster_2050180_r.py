from __future__ import annotations

import argparse
import copy
import json
import os
from pathlib import Path
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TARGET_PATH = (
    ROOT
    / "Data/Effects/Authored"
    / "effect.dimensionmaster.skill.2050180.unified.effect.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"

PARTICLE_MASTER_MATERIAL = "fx_m_mi_w_00.mi.fx_w_pa_master_01_05_dt_tr"
PARTICLE_MASTER_PROFILE = (
    "ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b"
)
PARTICLE_MASTER_PARENT = "fx_m_mi_00.fx_m.fx_d_pa_master_01_tr"
PARTICLE_MASTER_IDS = (
    "authored.source-particle.1827b45c2ca593ef046cf19a",
    "authored.source-particle.3d37c9d7cc96e563a4fe1b55",
    "authored.source-particle.f8534d1e810568b7ce3b7ee2",
)

SPRITEWAVE_MATERIAL = "fx_m_mi_w_00.mi.fx_w_pa_spritewave_01_05_tr"
SPRITEWAVE_PROFILE = (
    "ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92"
)
SPRITEWAVE_PARENT = "fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr"
SPRITEWAVE_OCCURRENCES = {
    "authored.source-particle.cc4d20091ad0ed409617f51f": (0.5, 1),
    "authored.source-particle.333341329ff3992ea8c7f0a1": (0.7, 1),
    "authored.source-particle.f2e42e062ca87d93f0a477e3": (1.05, 3),
}
HELIX_ID = "authored.source-particle.98639f5f2e65e0f0193c09fe"


def lane(
    name: str,
    source_object_path: str,
    asset_id: str,
    group: str,
) -> dict[str, Any]:
    return {
        "name": name,
        "sourceObjectPath": source_object_path,
        "assetId": asset_id,
        "addressU": "wrap",
        "addressV": "wrap",
        "colorSpace": "linear",
        "samplingEvidence": "source_material_role",
        "group": group,
    }


# fx_d_pa_master_01 assigns coverage to Map C.  The compact resource tuple is
# ordered E, D, F, C, so resource slot names cannot be used as shader roles.
PARTICLE_MASTER_TEXTURES = (
    lane(
        "21.map_c",
        "fx_tex_00.fx_a_cloud_021",
        "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_cloud_021.dds",
        "01_alpha",
    ),
    lane(
        "06.map",
        "fx_tex_02.fx_d_noise_009",
        "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_009.dds",
        "08_uvdistort",
    ),
    lane(
        "02.map_e",
        "fx_tex_04.fx_i_noise_03",
        "Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_noise_03.dds",
        "02_emission",
    ),
    lane(
        "12.map_f",
        "fx_tex_06.fx_j_environment_tile_02",
        "Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_environment_tile_02.dds",
        "02_emission",
    ),
)

# Child overrides and inherited parent lanes from the exact
# fx_w_pa_spritewave_01_05_tr source-material evidence.  MainTex owns the
# carrier; dissolve gates and the source sphere mask own the card boundary.
SPRITEWAVE_TEXTURES = (
    lane(
        "maintex",
        "fx_tex_05.fx_m_trail_007",
        "Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_007.dds",
        "maintex",
    ),
    lane(
        "uv_noise_tex",
        "fx_tex_02.fx_d_noise_030",
        "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds",
        "uv_noise",
    ),
    lane(
        "dissolve_tex_01",
        "fx_tex_04.fx_i_atypical_03_1_ycl",
        "Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_atypical_03_1_ycl.dds",
        "dissolve",
    ),
    lane(
        "noisedissolve_tex",
        "fx_tex_05.fx_k_auratile_02",
        "Effect/DimensionMaster/Textures/FX_TEX_05/fx_k_auratile_02.dds",
        "dissolve",
    ),
    lane(
        "dissolve_tex02",
        "fx_tex_02.fx_d_noise_030",
        "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds",
        "dissolve",
    ),
    lane(
        "emissivetex02",
        "fx_tex_05.fx_m_noise_008",
        "Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_008.dds",
        "emissive",
    ),
    lane(
        "uv_noise_tex_02",
        "fx_tex_02.fx_d_noise_030",
        "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds",
        "uv_noise",
    ),
    lane(
        "umodel_dependency",
        "fx_tex_00.fx_a_line_005",
        "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_line_005.dds",
        "",
    ),
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load_json_text(path: Path) -> tuple[str, dict[str, Any]]:
    text = path.read_text(encoding="utf-8-sig")
    return text, json.loads(text)


def element_by_id(document: dict[str, Any], element_id: str) -> dict[str, Any]:
    matches = [row for row in document["elements"] if row["id"] == element_id]
    require(len(matches) == 1, f"expected one element: {element_id}")
    return matches[0]


def resource_map(element: dict[str, Any]) -> dict[str, str]:
    return {
        str(row.get("slotId", "")): str(row.get("assetId", ""))
        for row in element.get("resources", [])
    }


def require_source_profile(
    element: dict[str, Any],
    material_path: str,
    profile_id: str,
    parent_path: str,
) -> dict[str, Any]:
    material = element.get("material") or {}
    profile = material.get("sourceProfile") or {}
    require(
        material.get("sourceMaterialPath") == material_path,
        f"source material changed: {element['id']}",
    )
    require(profile.get("enabled") is True, f"source profile disabled: {element['id']}")
    require(profile.get("profileId") == profile_id, f"profile id changed: {element['id']}")
    require(
        profile.get("parentMaterialPath") == parent_path,
        f"parent material changed: {element['id']}",
    )
    require(
        profile.get("runtimeShaderProfileId")
        == "effect.ue3.grouped-translucent.v1",
        f"source carrier profile changed: {element['id']}",
    )
    return profile


def validate_assets(lanes: tuple[dict[str, Any], ...]) -> None:
    for row in lanes:
        asset_id = str(row["assetId"])
        require((RESOURCES_ROOT / asset_id).is_file(), f"missing DDS: {asset_id}")


def validate_and_mutate(document: dict[str, Any]) -> None:
    require(
        document.get("effectAssetId") == "effect.dimensionmaster.skill.2050180.unified",
        "DimensionMaster R document identity changed",
    )
    validate_assets(PARTICLE_MASTER_TEXTURES)
    validate_assets(SPRITEWAVE_TEXTURES)

    expected_master_resources = {
        "base": "Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_noise_03.dds",
        "noise": "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_009.dds",
        "mask": (
            "Effect/DimensionMaster/Textures/FX_TEX_06/"
            "fx_j_environment_tile_02.dds"
        ),
        "emissive": (
            "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_cloud_021.dds"
        ),
    }
    for element_id in PARTICLE_MASTER_IDS:
        element = element_by_id(document, element_id)
        require(element.get("kind") == "particle", f"non-particle: {element_id}")
        require(
            (element.get("sourceRecipe") or {}).get("rendererShape") == "sprite",
            f"ParticleMaster carrier changed: {element_id}",
        )
        require(resource_map(element) == expected_master_resources, f"E/D/F/C tuple changed: {element_id}")
        profile = require_source_profile(
            element,
            PARTICLE_MASTER_MATERIAL,
            PARTICLE_MASTER_PROFILE,
            PARTICLE_MASTER_PARENT,
        )
        profile["textures"] = copy.deepcopy(PARTICLE_MASTER_TEXTURES)

    expected_slash_resources = {
        "meshModel": "Effect/DimensionMaster/Meshes/fm_h_swing_05.wmodel",
        "base": "Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_007.dds",
        "dissolve": (
            "Effect/DimensionMaster/Textures/FX_TEX_04/"
            "fx_i_atypical_03_1_ycl.dds"
        ),
        "noise": "Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds",
    }
    for element_id, (source_time, burst_count) in SPRITEWAVE_OCCURRENCES.items():
        element = element_by_id(document, element_id)
        require(element.get("kind") == "particle", f"non-particle: {element_id}")
        require(
            (element.get("sourceRecipe") or {}).get("rendererShape") == "mesh",
            f"SpriteWave carrier changed: {element_id}",
        )
        require(resource_map(element) == expected_slash_resources, f"slash tuple changed: {element_id}")
        profile = require_source_profile(
            element,
            SPRITEWAVE_MATERIAL,
            SPRITEWAVE_PROFILE,
            SPRITEWAVE_PARENT,
        )
        profile["textures"] = copy.deepcopy(SPRITEWAVE_TEXTURES)
        require(
            abs(float((element.get("sourcePresentation") or {}).get("sourceTimeSeconds")) - source_time)
            < 1.0e-6,
            f"source occurrence time changed: {element_id}",
        )
        bursts = (element.get("sourceRecipe") or {}).get("bursts") or []
        require(len(bursts) == 1, f"slash burst cardinality changed: {element_id}")
        bursts[0]["countMinimum"] = burst_count
        bursts[0]["countMaximum"] = burst_count

        spawn_modules = [
            row
            for row in element["sourceRecipe"].get("modules", [])
            if str(row.get("className", "")).casefold() == "particlemodulespawn"
        ]
        require(len(spawn_modules) == 1, f"slash spawn module changed: {element_id}")
        count_literals = [
            row
            for row in spawn_modules[0].get("literals", [])
            if str(row.get("propertyPath", "")).casefold() == "burstlist[0].count"
        ]
        require(len(count_literals) == 1, f"slash burst literal changed: {element_id}")
        count_literals[0]["value"] = float(burst_count)

    helix = element_by_id(document, HELIX_ID)
    require(
        resource_map(helix).get("meshModel")
        == "Effect/DimensionMaster/Meshes/fm_d_helix_015_1.wmodel",
        "R helix mesh identity changed",
    )
    require(
        abs(float(helix["detail"]["timing"]["startDelaySeconds"]) - 0.01)
        < 1.0e-6,
        "R helix timing changed",
    )
    require(
        helix["sourceRecipe"]["bursts"]
        == [{"timeSeconds": 0, "countMinimum": 3, "countMaximum": 3}],
        "R helix source burst changed",
    )
    type_data_modules = [
        row
        for row in helix["sourceRecipe"].get("modules", [])
        if str(row.get("className", "")).casefold()
        == "particlemoduletypedatamesh"
    ]
    require(len(type_data_modules) == 1, "R helix TypeDataMesh module changed")
    pitch_literals = [
        row
        for row in type_data_modules[0].get("literals", [])
        if str(row.get("propertyPath", "")).casefold() == "pitch"
    ]
    require(
        len(pitch_literals) == 1
        and float(pitch_literals[0].get("value")) == -90.0,
        "R helix source TypeData pitch changed",
    )
    helix["detail"]["mesh"]["sourceTypeDataRotationDegrees"] = [0, -90, 0]


def compact_json(value: Any) -> str:
    return json.dumps(value, ensure_ascii=False, separators=(",", ":"))


def patch_preserving_layout(original: str, document: dict[str, Any]) -> str:
    lines = original.splitlines(keepends=True)
    current_id = ""
    seen_master: set[str] = set()
    seen_slash: set[str] = set()
    master_textures = compact_json(PARTICLE_MASTER_TEXTURES)
    slash_textures = compact_json(SPRITEWAVE_TEXTURES)

    for index, line_text in enumerate(lines):
        for element_id in (
            *PARTICLE_MASTER_IDS,
            *SPRITEWAVE_OCCURRENCES,
            HELIX_ID,
        ):
            if f'"id": "{element_id}"' in line_text or f'"id":"{element_id}"' in line_text:
                current_id = element_id
                break

        if current_id in PARTICLE_MASTER_IDS and PARTICLE_MASTER_MATERIAL in line_text:
            old = '"textures": []'
            expected = f'"textures": {master_textures}'
            if expected not in line_text:
                require(line_text.count(old) == 1, f"master textures token changed: {current_id}")
                lines[index] = line_text.replace(old, expected, 1)
            seen_master.add(current_id)
            current_id = ""
            continue

        if current_id in SPRITEWAVE_OCCURRENCES and SPRITEWAVE_MATERIAL in line_text:
            element_id = current_id
            burst_count = SPRITEWAVE_OCCURRENCES[element_id][1]
            old_profile = '"semanticStatus":"reconstructed_profile","scalars":'
            if '"textures":[' not in line_text:
                require(
                    line_text.count(old_profile) == 1,
                    f"slash profile token changed: {element_id}",
                )
                line_text = line_text.replace(
                    old_profile,
                    f'"semanticStatus":"reconstructed_profile","textures":{slash_textures},"scalars":',
                    1,
                )
            old_burst = (
                '"bursts":[{"timeSeconds":0.0,"countMinimum":3,'
                '"countMaximum":3}]'
            )
            new_burst = (
                f'"bursts":[{{"timeSeconds":0.0,"countMinimum":{burst_count},'
                f'"countMaximum":{burst_count}}}]'
            )
            if new_burst not in line_text:
                require(line_text.count(old_burst) == 1, f"slash burst token changed: {element_id}")
                line_text = line_text.replace(old_burst, new_burst, 1)
            old_literal = (
                '"propertyPath":"burstlist[0].count","kind":"number",'
                '"value":3.0}'
            )
            new_literal = (
                '"propertyPath":"burstlist[0].count","kind":"number",'
                f'"value":{float(burst_count):.1f}}}'
            )
            if new_literal not in line_text:
                require(line_text.count(old_literal) == 1, f"slash burst literal token changed: {element_id}")
                line_text = line_text.replace(old_literal, new_literal, 1)
            lines[index] = line_text
            seen_slash.add(element_id)
            current_id = ""

        if current_id == HELIX_ID and '"sourceTypeDataRotationDegrees"' in line_text:
            old_rotation = '"sourceTypeDataRotationDegrees": [0, 0, 0]'
            new_rotation = '"sourceTypeDataRotationDegrees": [0, -90, 0]'
            if new_rotation not in line_text:
                require(
                    line_text.count(old_rotation) == 1,
                    "R helix TypeData rotation token changed",
                )
                lines[index] = line_text.replace(
                    old_rotation, new_rotation, 1
                )
            current_id = ""

    require(seen_master == set(PARTICLE_MASTER_IDS), f"master rows incomplete: {seen_master}")
    require(seen_slash == set(SPRITEWAVE_OCCURRENCES), f"slash rows incomplete: {seen_slash}")
    return "".join(lines)


def atomic_write(path: Path, text: str) -> None:
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary_path = Path(stream.name)
            stream.write(text)
            stream.flush()
            os.fsync(stream.fileno())
        json.loads(temporary_path.read_text(encoding="utf-8"))
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()

    original, document = load_json_text(TARGET_PATH)
    before = copy.deepcopy(document)
    validate_and_mutate(document)

    # Preserve every unrelated element and every field outside the exact
    # source-role/burst changes above.
    allowed_ids = (
        set(PARTICLE_MASTER_IDS) | set(SPRITEWAVE_OCCURRENCES) | {HELIX_ID}
    )
    for element in before["elements"]:
        if element["id"] not in allowed_ids:
            require(
                element == element_by_id(document, element["id"]),
                f"unrelated element changed: {element['id']}",
            )

    patched = patch_preserving_layout(original, document)
    parsed = json.loads(patched)
    validate_and_mutate(parsed)
    require(parsed == document, "layout-preserving patch differs from staged document")
    changed = patched != original
    if args.write and changed:
        atomic_write(TARGET_PATH, patched)

    print(
        json.dumps(
            {
                "status": (
                    "updated" if args.write and changed else
                    "would-update" if changed else "stable"
                ),
                "document": TARGET_PATH.relative_to(ROOT).as_posix(),
                "particleMasterOccurrences": len(PARTICLE_MASTER_IDS),
                "spriteWaveOccurrences": len(SPRITEWAVE_OCCURRENCES),
                "spriteWaveBurstCounts": [
                    SPRITEWAVE_OCCURRENCES[element_id][1]
                    for element_id in SPRITEWAVE_OCCURRENCES
                ],
                "coverageOwners": {
                    "particleMaster": "21.map_c",
                    "spriteWave": "maintex+dissolve+sphere-mask",
                },
            },
            ensure_ascii=False,
            indent=2,
        )
    )
    return 0 if args.write or not changed else 1


if __name__ == "__main__":
    raise SystemExit(main())
