from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
TARGET_PATH = (
    ROOT
    / "Data/Effects/Authored/"
    / "effect.dimensionmaster.skill.2050120.clip3.unified.effect.json"
)
BASELINE_PATH = (
    ROOT
    / "Data/Effects/Authored/"
    / "effect.dimensionmaster.skill.2050120.effect.json"
)
ARTIST_REFERENCE_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"

PARENT_MATERIAL = "fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr"
ARTIST_REFERENCE_MATERIAL = "fx_m_mi_m_00.fx_m_pa_spritewave_01_19_tr"
ARTIST_REFERENCE_ELEMENT_ID = "mesh.646163c341579b56"
RUNTIME_OPCODE = 15

MATERIALS = {
    "fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_85_tr": {
        "count": 4,
        "mainAddressU": "wrap",
        "mainAddressV": "clamp",
        "noiseDissolve": (
            "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_cloud_022.dds"
        ),
    },
    "fx_m_mi_s_00.fx_s_me_spritewave_01_01_tr": {
        "count": 4,
        "mainAddressU": "wrap",
        "mainAddressV": "wrap",
        "noiseDissolve": (
            "Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_noise_001.dds"
        ),
    },
}

# Exact semantic order consumed by the recovered fx_m_pa_spritewave_01_tr
# equation.  The source profile stores a merged, name-addressed parent/child
# view, so child override ordering never changes this packet ABI.
FORMULA_SCALARS = [
    "disslovetex_01_panspeed_x",
    "disslovetex_01_panspeed_y",
    "disslovetex_01_tile_x",
    "disslovetex_01_tile_y",
    "dissolve_hardness",
    "dissolvetex_rotator",
    "edge_thin",
    "noisedissolvetex_strength",
    "noisetodisslovetex_01_panspeed_x",
    "noisetodisslovetex_01_panspeed_y",
    "noisetodisslovetex_01_tile_x",
    "noisetodisslovetex_01_tile_y",
    "emissive_base",
    "emissive_core_power",
    "emissive_core_strength",
    "maintex_alpha_strength",
    "maintex_dynamicpan_x_velue",
    "maintex_dynamicpan_y_velue",
    "maintex_move_x",
    "maintex_move_y",
    "maintex_panspeed_y",
    "maintex_rotator",
    "maintex_tile_x",
    "maintex_tile_y",
    "uv_noise_velue",
    "uv_noisetex_pan_x",
    "uv_noisetex_pan_y",
    "uv_noisetex_tile_x",
    "uv_noisetex_tile_y",
    "uvnoise_move_x",
    "uvnoise_move_y",
    "spheremask_strength",
    "spheremask_strength_max",
    "spheremask_strength_min",
]

EXPECTED_DYNAMIC_PARAMETERS = [
    "maintex_tile_pan",
    "dissolve",
    "uv_noisevelue",
    "uv_sphery_uv_noisepan",
]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def material_path(element: dict[str, Any]) -> str:
    return str(element.get("material", {}).get("sourceMaterialPath", "")).casefold()


def dynamic_parameter_names(element: dict[str, Any]) -> list[str]:
    rows: list[tuple[str, str]] = []
    for module in element.get("sourceRecipe", {}).get("modules", []):
        if str(module.get("className", "")).casefold() != (
            "particlemoduleparameterdynamic"
        ):
            continue
        for literal in module.get("literals", []):
            property_path = str(literal.get("propertyPath", ""))
            if property_path.casefold().endswith(".paramname"):
                rows.append((property_path, str(literal.get("value", ""))))
    return [value for _, value in sorted(rows)]


def element_by_id(document: dict[str, Any], element_id: str) -> dict[str, Any]:
    matches = [row for row in document["elements"] if row["id"] == element_id]
    require(len(matches) == 1, f"expected one element: {element_id}")
    return matches[0]


def baseline_profiles(document: dict[str, Any]) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for element in document["elements"]:
        path = material_path(element)
        if path not in MATERIALS:
            continue
        profile = element["material"].get("sourceProfile") or {}
        require(profile.get("enabled") is True, f"baseline profile is disabled: {path}")
        require(
            str(profile.get("parentMaterialPath", "")).casefold()
            == PARENT_MATERIAL,
            f"baseline parent changed: {path}",
        )
        require(profile.get("staticSwitches") == [], f"static override appeared: {path}")
        prior = result.get(path)
        if prior is not None:
            require(canonical(prior) == canonical(profile), f"profile split: {path}")
        result[path] = profile
    require(set(result) == set(MATERIALS), "baseline SpriteWave profiles are incomplete")
    return result


def source_resource_map(element: dict[str, Any]) -> dict[str, str]:
    result = {
        str(row.get("slotId", "")): str(row.get("assetId", ""))
        for row in element.get("resources", [])
    }
    require(
        all(result.get(slot) for slot in ("base", "noise", "dissolve")),
        f"required SpriteWave lane missing: {element['id']}",
    )
    return result


def lane_from_reference(
    reference_lane: dict[str, Any], asset_id: str, address_u: str, address_v: str
) -> dict[str, Any]:
    lane = copy.deepcopy(reference_lane)
    lane["assetId"] = asset_id
    lane["sampler"]["addressU"] = address_u
    lane["sampler"]["addressV"] = address_v
    path = RESOURCES_ROOT / asset_id
    require(path.is_file(), f"missing runtime DDS: {asset_id}")
    return lane


def build_execution(
    reference: dict[str, Any],
    profile: dict[str, Any],
    resources: dict[str, str],
    material_config: dict[str, Any],
) -> dict[str, Any]:
    execution = copy.deepcopy(reference)
    require(
        execution.get("enabled") is True
        and execution.get("backend") == "runtimeMaterialV2"
        and execution.get("opcode") == 8
        and execution.get("textureLaneCount") == 4
        and execution.get("scalarCount") == 47
        and execution.get("vectorCount") == 1,
        "Artist SpriteWave packet ABI changed",
    )

    by_role = {row["role"]: row for row in execution["textureLanes"]}
    require(
        set(by_role) == {
            "maintex",
            "uv_noise_tex",
            "dissolve_tex_01",
            "noisedissolve_tex",
        },
        "Artist SpriteWave texture ABI changed",
    )
    execution["opcode"] = RUNTIME_OPCODE
    execution["passIndex"] = 3
    execution["renderState"] = {
        "rasterizer": "RS_Default",
        "depthStencil": "DSS_ReadOnly",
        "blend": "BS_EffectAlpha",
        "stencilReference": 0,
    }
    execution["textureLanes"] = [
        lane_from_reference(
            by_role["maintex"],
            resources["base"],
            material_config["mainAddressU"],
            material_config["mainAddressV"],
        ),
        lane_from_reference(
            by_role["uv_noise_tex"], resources["noise"], "wrap", "wrap"
        ),
        lane_from_reference(
            by_role["dissolve_tex_01"], resources["dissolve"], "wrap", "wrap"
        ),
        lane_from_reference(
            by_role["noisedissolve_tex"],
            material_config["noiseDissolve"],
            "wrap",
            "wrap",
        ),
    ]

    values = {
        str(row["name"]).casefold(): float(row["value"])
        for row in profile.get("scalars", [])
    }
    missing = [name for name in FORMULA_SCALARS if name.casefold() not in values]
    require(not missing, f"missing SpriteWave scalar semantics: {missing}")
    formula_values = [values[name.casefold()] for name in FORMULA_SCALARS]
    reference_tail = [
        float(row["value"]) for row in execution["scalars"][len(FORMULA_SCALARS) :]
    ]
    require(len(reference_tail) == 13, "SpriteWave packet tail changed")
    packed_values = formula_values + reference_tail
    execution["scalars"] = [
        {"name": f"scalar.{index}", "packedIndex": index, "value": value}
        for index, value in enumerate(packed_values)
    ]

    vectors = profile.get("vectors", [])
    edge_rows = [row for row in vectors if str(row.get("name", "")).casefold() == "edge_color"]
    require(len(edge_rows) == 1, "SpriteWave edge_color is not singular")
    edge_value = edge_rows[0].get("value")
    require(isinstance(edge_value, list) and len(edge_value) == 4, "invalid edge_color")
    execution["vectors"] = [
        {"name": "vector.0", "packedIndex": 0, "value": [float(x) for x in edge_value]}
    ]
    return execution


def atomic_write(path: Path, payload: bytes) -> None:
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            dir=path.parent,
            prefix=f".{path.name}.",
            suffix=".tmp",
            delete=False,
        ) as stream:
            temporary_path = Path(stream.name)
            stream.write(payload)
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

    target = load_json(TARGET_PATH)
    baseline = load_json(BASELINE_PATH)
    artist = load_json(ARTIST_REFERENCE_PATH)
    profiles = baseline_profiles(baseline)
    reference_element = element_by_id(artist, ARTIST_REFERENCE_ELEMENT_ID)
    require(
        material_path(reference_element) == ARTIST_REFERENCE_MATERIAL,
        "Artist SpriteWave reference identity changed",
    )
    reference_execution = reference_element["material"]["execution"]

    before_non_material = {
        row["id"]: canonical({key: value for key, value in row.items() if key != "material"})
        for row in target["elements"]
    }
    counts = {path: 0 for path in MATERIALS}
    resource_hashes: dict[str, str] = {}
    for element in target["elements"]:
        path = material_path(element)
        if path not in MATERIALS:
            continue
        counts[path] += 1
        require(element.get("kind") == "particle", f"non-particle carrier: {element['id']}")
        require(
            not any(row.get("slotId") == "meshModel" for row in element.get("resources", [])),
            f"SpriteWave occurrence became a mesh carrier: {element['id']}",
        )
        require(
            dynamic_parameter_names(element) == EXPECTED_DYNAMIC_PARAMETERS,
            f"Dynamic Parameter ABI changed: {element['id']}",
        )
        resources = source_resource_map(element)
        execution = build_execution(
            reference_execution, profiles[path], resources, MATERIALS[path]
        )
        material = element["material"]
        require(
            str(material.get("renderProfile", "")) == "alpha_one_sided_depth_read",
            f"unexpected SpriteWave render profile: {element['id']}",
        )
        # The fixed RuntimeMaterialV2 packet owns execution for this occurrence.
        # Keep sourceMaterialPath as provenance, but do not advertise the raw
        # UE3 profile as a second runtime owner.  This is the same single-owner
        # contract used by the existing authored execution materializers.
        material["templateId"] = "effect.standard"
        material["sourceProfile"] = {"enabled": False}
        material["execution"] = execution
        for lane in execution["textureLanes"]:
            resource_hashes[lane["assetId"]] = sha256_file(
                RESOURCES_ROOT / lane["assetId"]
            )

    require(
        counts == {path: int(config["count"]) for path, config in MATERIALS.items()},
        f"SpriteWave occurrence cardinality changed: {counts}",
    )
    for element in target["elements"]:
        require(
            canonical({key: value for key, value in element.items() if key != "material"})
            == before_non_material[element["id"]],
            f"non-material occurrence changed: {element['id']}",
        )

    output = (json.dumps(target, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    before = TARGET_PATH.read_bytes()
    changed = output != before
    if args.write and changed:
        atomic_write(TARGET_PATH, output)
    print(
        json.dumps(
            {
                "status": "updated" if args.write and changed else ("would-update" if changed else "stable"),
                "document": TARGET_PATH.relative_to(ROOT).as_posix(),
                "opcode": RUNTIME_OPCODE,
                "family": PARENT_MATERIAL,
                "occurrenceCount": sum(counts.values()),
                "occurrencesByMaterial": counts,
                "resourceSha256": dict(sorted(resource_hashes.items())),
                "nonMaterialFieldsPreserved": True,
            },
            ensure_ascii=False,
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
