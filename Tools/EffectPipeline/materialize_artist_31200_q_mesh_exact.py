from __future__ import annotations

import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
Q_DOCUMENT_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31200.unified.effect.json"
)
F_DOCUMENT_PATH = (
    ROOT / "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)
Q_GRAPH_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31200.normalized-effect-graph.json"
)
F_GRAPH_PATH = (
    ROOT / "Data/Effects/Imported/Artist/Graphs/skill.31470.normalized-effect-graph.json"
)
F_TYPED_EVIDENCE_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"

Q_INITIAL_SHA256 = "911aba16ba76fc0d92c19181da3fff3a2e8a5f7d42092ea059b13c12202b1941"
F_FROZEN_SHA256 = "32676821df73c772bd313825c6968e2a79f9ada7af445b7734b07f0d40828799"

Q_PARTICLE_MASTER_ID = "authored.source-particle.e705de9acdd96252b98fb6ff"
Q_SPRITEWAVE_ID = "authored.source-particle.8b8cad3aafdd36174d713698"
Q_DECAL_ID = "authored.source-decal.45800d0c0054acd91f11cfb4"
F_SPRITEWAVE_ID = "mesh.646163c341579b56"
VISIBLE_IDS = {Q_PARTICLE_MASTER_ID, Q_SPRITEWAVE_ID, Q_DECAL_ID}

SPRITEWAVE_SOURCE_MATERIAL = "fx_m_mi_m_00.fx_m_pa_spritewave_01_19_tr"
SPRITEWAVE_PARENT_MATERIAL = "fx_m.fx_m_pa_spritewave_01_tr"
SPRITEWAVE_PHYSICAL_PACKAGE = "ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk"
SPRITEWAVE_PHYSICAL_PACKAGE_SHA256 = (
    "6a46989680d244946e2c7910a444da7a403500c4e2f8af1665da196b05fadc3e"
)
SPRITEWAVE_MATERIAL_SERIAL_SHA256 = (
    "f8ffba06e81c39387e168ebb2aace8f1ba84d648f4bb04f064b2d893dc7c74b2"
)

PARTICLE_MASTER_TEXTURE_ASSETS = {
    "21.map_c": "Effect/Artist/Textures/fx_bg_lightbeam_falloff_01_ycl.dds",
    "06.map": "Effect/Artist/Textures/fx_m_flow_03_n.dds",
    "11.map_b": "Effect/Artist/Textures/fx_d_atypical_055_1_cl.dds",
    "01.map_a": "Effect/Artist/Textures/fx_d_environ_035.dds",
    "02.map_e": "Effect/Artist/Textures/fx_d_environ_067.dds",
    "12.map_f": "Effect/Artist/Textures/fx_k_liquid_02.dds",
    "42.map_c": "Effect/Artist/Textures/fx_d_fluid_026.dds",
}
PARTICLE_MASTER_TEXTURE_SHA256 = {
    "21.map_c": "949d8b3e231af2de40b89152f53d8169dc61428a24f447b79145e81966d8295b",
    "06.map": "3d02ae77275200075e64f286622ac918a50db5852a55593930841a1e0dc19c80",
    "11.map_b": "ed520b735702a85d4d72c346a3f6af2e0132188e0e6136c4d7dfec38be26e254",
    "01.map_a": "67845b6501a645505f2ccb016d164a221d58f0f5bd3f23e58c83f7c4abaaacd8",
    "02.map_e": "4b1fd65d5584a4c00aa59a02a0abb69c294f3a9dfc280afa194344d8855f9705",
    "12.map_f": "05e4cf42291d549bb02ce28b0f24b916bbe7bc68eed58220a7ca5ef9a9bf79fa",
    "42.map_c": "8c1374da4a341417c11d0732fcac20c33755b8af407843eb0c6b25e4dad19b96",
}
SPRITEWAVE_LANE_SHA256 = {
    "Effect/Artist/Textures/fx_m_trail_004_cl.dds": (
        "5681360a77c21948e854a46cd2b6a547f40f676f3ff31c73902e777f112c30b0"
    ),
    "Effect/Artist/Textures/fx_d_noise_033.dds": (
        "1505396d86385b176ced2a804001deda74ff20fa44578fb5ec23156c34cc48de"
    ),
    "Effect/Artist/Textures/fx_m_atypical_012.dds": (
        "8a05d8f0ed9cb2a8d8b2e49139400b24d554f4c21a58f10ac5c334a55d573bab"
    ),
    "Effect/Artist/Textures/fx_m_noise_001.dds": (
        "19843f9ee15e94e629926f45e1887ad6ca9815bfd785527ed8b4ce63692918b8"
    ),
}


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def canonical(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def element_by_id(document: dict[str, Any], element_id: str) -> dict[str, Any]:
    matches = [row for row in document["elements"] if row["id"] == element_id]
    require(len(matches) == 1, f"expected exactly one element: {element_id}")
    return matches[0]


def material_binding(graph: dict[str, Any], source_path: str) -> dict[str, Any]:
    matches = [
        row
        for row in graph["materialParameterBindings"]
        if row["sourceMaterialPath"] == source_path
    ]
    require(len(matches) == 1, f"expected exactly one material binding: {source_path}")
    return matches[0]


def normalized_binding_values(binding: dict[str, Any]) -> dict[str, Any]:
    return {
        "sourceMaterialPath": binding["sourceMaterialPath"],
        "sourcePhysicalPackage": binding["sourcePhysicalPackage"],
        "parent": binding["parent"],
        "textures": sorted(
            (row["name"], row["texture"]) for row in binding["textures"]
        ),
        "scalars": sorted(
            (row["name"], float(row["value"])) for row in binding["scalars"]
        ),
        "vectors": sorted(
            (row["name"], row["value"]) for row in binding["vectors"]
        ),
    }


def dynamic_parameter_names(element: dict[str, Any]) -> list[str]:
    names: list[tuple[str, str]] = []
    for module in element["sourceRecipe"]["modules"]:
        if module["className"] != "particlemoduleparameterdynamic":
            continue
        for literal in module["literals"]:
            path = literal["propertyPath"]
            if path.endswith(".paramname"):
                names.append((path, literal["value"]))
    return [value for _, value in sorted(names)]


def assert_visibility_guard(document: dict[str, Any]) -> None:
    require(len(document["elements"]) == 31, "Q element cardinality changed")
    actual = {row["id"] for row in document["elements"] if row["visible"] is True}
    require(actual == VISIBLE_IDS, f"Q visible guard changed: {sorted(actual)}")
    require(
        sum(row["visible"] is False for row in document["elements"]) == 28,
        "Q hidden-row cardinality changed",
    )


def assert_resource_sha256(asset_id: str, expected_sha256: str) -> None:
    path = RESOURCES_ROOT / asset_id
    require(path.is_file(), f"missing exact resource: {asset_id}")
    actual = sha256_file(path)
    require(actual == expected_sha256, f"resource SHA mismatch: {asset_id}: {actual}")


def find_typed_spritewave_recipe(evidence: dict[str, Any]) -> dict[str, Any]:
    matches = [
        row
        for row in evidence["materialRecipes"]
        if row["identity"]["canonicalSourceMaterialPath"]
        == SPRITEWAVE_SOURCE_MATERIAL
    ]
    require(len(matches) == 1, "expected one typed SpriteWave material recipe")
    return matches[0]


def compact_object(value: dict[str, Any]) -> str:
    payload = json.dumps(value, ensure_ascii=False, separators=(", ", ": "))
    require(payload.startswith("{") and payload.endswith("}"), "material is not object")
    return "{ " + payload[1:-1] + " }"


def atomic_write_verified_json(path: Path, payload: bytes) -> None:
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

        staged = temporary_path.read_bytes()
        require(
            sha256_bytes(staged) == sha256_bytes(payload),
            "staged Q document hash mismatch",
        )
        json.loads(staged.decode("utf-8-sig"))
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def replace_material_line(
    text: str, element_id: str, material: dict[str, Any]
) -> str:
    lines = text.splitlines(keepends=True)
    id_marker = f'"id": "{element_id}"'
    matches = [index for index, line in enumerate(lines) if id_marker in line]
    require(len(matches) == 1, f"expected one element line: {element_id}")
    start = matches[0]
    next_start = next(
        (index for index in range(start + 1, len(lines)) if '"id": "' in lines[index]),
        len(lines),
    )
    material_lines = [
        index
        for index in range(start + 1, next_start)
        if lines[index].lstrip().startswith('"material": ')
    ]
    require(len(material_lines) == 1, f"expected one material line: {element_id}")
    index = material_lines[0]
    prefix = lines[index][: len(lines[index]) - len(lines[index].lstrip())]
    newline = "\n" if lines[index].endswith("\n") else ""
    trailing_comma = "," if lines[index].rstrip().endswith(",") else ""
    lines[index] = (
        prefix + '"material": ' + compact_object(material) + trailing_comma + newline
    )
    return "".join(lines)


def main() -> int:
    q_bytes = Q_DOCUMENT_PATH.read_bytes()
    q_sha256 = sha256_bytes(q_bytes)
    q_document = json.loads(q_bytes.decode("utf-8-sig"))
    f_document = load_json(F_DOCUMENT_PATH)
    q_graph = load_json(Q_GRAPH_PATH)
    f_graph = load_json(F_GRAPH_PATH)
    typed_evidence = load_json(F_TYPED_EVIDENCE_PATH)

    assert_visibility_guard(q_document)
    require(sha256_file(F_DOCUMENT_PATH) == F_FROZEN_SHA256, "Artist F control changed")

    q_particle_master = element_by_id(q_document, Q_PARTICLE_MASTER_ID)
    q_spritewave = element_by_id(q_document, Q_SPRITEWAVE_ID)
    f_spritewave = element_by_id(f_document, F_SPRITEWAVE_ID)

    before_non_material = {
        row["id"]: canonical({key: value for key, value in row.items() if key != "material"})
        for row in q_document["elements"]
    }
    before_decal = canonical(element_by_id(q_document, Q_DECAL_ID))

    particle_master_binding = material_binding(
        q_graph, "fx_m_mi_00.fx_mi.fx_d_me_master_01_119_dt_ds_tr"
    )
    expected_graph_lanes = {
        "21.map_c": "fx_tex_00.fx_bg_lightbeam_falloff_01_ycl",
        "01.map_a": "fx_tex_02.fx_d_environ_035",
        "11.map_b": "fx_tex_02.fx_d_atypical_055_1_cl",
        "06.map": "fx_tex_05.fx_m_flow_03_n",
        "02.map_e": "fx_tex_02.fx_d_environ_067",
        "12.map_f": "fx_tex_05.fx_k_liquid_02",
    }
    require(
        {row["name"]: row["texture"] for row in particle_master_binding["textures"]}
        == expected_graph_lanes,
        "Q ParticleMaster source lane tuple changed",
    )
    require(
        dynamic_parameter_names(q_particle_master)
        == ["alphadissolve[0-1]", "pan[0-2]", "edgestr[0-x]", "disrotion[0-x]"],
        "Q ParticleMaster dynamic parameter identity changed",
    )

    particle_material = copy.deepcopy(q_particle_master["material"])
    particle_textures = {
        row["name"]: row for row in particle_material["sourceProfile"]["textures"]
    }
    for name, asset_id in PARTICLE_MASTER_TEXTURE_ASSETS.items():
        require(name in particle_textures, f"missing ParticleMaster declared lane: {name}")
        current = particle_textures[name]["assetId"]
        require(current in {"", asset_id}, f"unexpected ParticleMaster lane target: {name}")
        particle_textures[name]["assetId"] = asset_id
        assert_resource_sha256(asset_id, PARTICLE_MASTER_TEXTURE_SHA256[name])

    q_binding = material_binding(q_graph, SPRITEWAVE_SOURCE_MATERIAL)
    f_binding = material_binding(f_graph, SPRITEWAVE_SOURCE_MATERIAL)
    require(
        normalized_binding_values(q_binding) == normalized_binding_values(f_binding),
        "Q/F SpriteWave child, parent, texture, scalar, or vector inputs differ",
    )
    require(q_binding["parent"] == SPRITEWAVE_PARENT_MATERIAL, "SpriteWave parent changed")
    require(
        q_binding["sourcePhysicalPackage"] == SPRITEWAVE_PHYSICAL_PACKAGE,
        "SpriteWave source package changed",
    )

    typed_recipe = find_typed_spritewave_recipe(typed_evidence)
    identity = typed_recipe["identity"]
    require(identity["physicalPackage"] == SPRITEWAVE_PHYSICAL_PACKAGE, "typed package mismatch")
    require(
        identity["physicalPackageSha256"] == SPRITEWAVE_PHYSICAL_PACKAGE_SHA256,
        "typed package SHA mismatch",
    )
    require(
        identity["materialSerialSha256"] == SPRITEWAVE_MATERIAL_SERIAL_SHA256,
        "typed child material serial mismatch",
    )
    require(
        identity["selectedGraphIdentity"]["rawParentReferencePath"]
        == SPRITEWAVE_PARENT_MATERIAL,
        "typed parent material mismatch",
    )
    q_source_profile = q_spritewave["material"].get("sourceProfile") or {}
    if q_spritewave["material"] != f_spritewave["material"]:
        require(
            q_source_profile.get("staticSwitches") == [],
            "Q instance gained occurrence-local static overrides",
        )
    else:
        require(
            q_source_profile == {"enabled": False},
            "materialized Q SpriteWave source profile changed",
        )

    expected_dynamic_names = [
        "maintex_tile_pan",
        "dissolve",
        "uv_noisevelue",
        "uv_sphery_uv_noisepan",
    ]
    require(
        dynamic_parameter_names(q_spritewave) == expected_dynamic_names,
        "Q SpriteWave dynamic parameter identity changed",
    )
    require(
        dynamic_parameter_names(f_spritewave) == expected_dynamic_names,
        "Artist F SpriteWave dynamic parameter identity changed",
    )

    f_execution = f_spritewave["material"]["execution"]
    require(
        f_execution["enabled"] is True
        and f_execution["backend"] == "runtimeMaterialV2"
        and f_execution["opcode"] == 8
        and f_execution["passIndex"] == 1
        and f_execution["staticInputCount"] == 9
        and f_execution["staticSelectedMask"] == 495
        and f_execution["scalarCount"] == 47
        and f_execution["vectorCount"] == 1
        and f_execution["textureLaneCount"] == 4,
        "Artist F exact SpriteWave execution contract changed",
    )
    actual_lane_assets = {
        row["role"]: row["assetId"] for row in f_execution["textureLanes"]
    }
    require(
        actual_lane_assets
        == {
            "maintex": "Effect/Artist/Textures/fx_m_trail_004_cl.dds",
            "uv_noise_tex": "Effect/Artist/Textures/fx_d_noise_033.dds",
            "dissolve_tex_01": "Effect/Artist/Textures/fx_m_atypical_012.dds",
            "noisedissolve_tex": "Effect/Artist/Textures/fx_m_noise_001.dds",
        },
        "Artist F exact SpriteWave lane map changed",
    )
    for asset_id, expected_sha256 in SPRITEWAVE_LANE_SHA256.items():
        assert_resource_sha256(asset_id, expected_sha256)

    spritewave_material = copy.deepcopy(f_spritewave["material"])
    require(
        spritewave_material["sourceMaterialPath"]
        == q_spritewave["material"]["sourceMaterialPath"],
        "Q/F SpriteWave material identity differs",
    )

    already_materialized = (
        q_particle_master["material"] == particle_material
        and q_spritewave["material"] == spritewave_material
    )
    if not already_materialized:
        require(q_sha256 == Q_INITIAL_SHA256, f"unexpected Q preimage SHA: {q_sha256}")

    text = q_bytes.decode("utf-8-sig")
    text = replace_material_line(text, Q_PARTICLE_MASTER_ID, particle_material)
    text = replace_material_line(text, Q_SPRITEWAVE_ID, spritewave_material)
    staged_document = json.loads(text)

    assert_visibility_guard(staged_document)
    require(
        canonical(element_by_id(staged_document, Q_DECAL_ID)) == before_decal,
        "Q decal changed while materializing meshes",
    )
    for row in staged_document["elements"]:
        require(
            canonical({key: value for key, value in row.items() if key != "material"})
            == before_non_material[row["id"]],
            f"non-material Q occurrence fields changed: {row['id']}",
        )

    output = text.encode("utf-8")
    if output != q_bytes:
        atomic_write_verified_json(Q_DOCUMENT_PATH, output)
        action = "updated"
    else:
        action = "stable"
    print(
        json.dumps(
            {
                "status": action,
                "document": str(Q_DOCUMENT_PATH.relative_to(ROOT)),
                "beforeSha256": q_sha256,
                "afterSha256": sha256_bytes(output),
                "visibleElementCount": 3,
                "hiddenElementCount": 28,
                "particleMasterNamedLanes": "2/7 -> 7/7",
                "spriteWaveExecution": "approximate disabled -> runtimeMaterialV2 opcode 8",
                "occurrenceFieldsPreserved": True,
                "decalPreserved": True,
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
