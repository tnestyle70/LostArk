#!/usr/bin/env python3
"""Materialize portable Track A particle execution into Artist F authored JSON.

This is a publisher/materializer, not a runtime or UI harness.  It preserves the
existing authored document (including user Decals and transforms), replaces only
the 29 Mesh/Sprite sourceRecipe carriers, and aligns typed ArtistVisualV4 particle
color metadata with the shader opcode ABI.
"""

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
SOURCE_PATH = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "effect.artist.skill.31470.native-v14.source-contract-candidate.effect.json"
)
PROGRAM_PATH = ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
AUTHORED_PATH = ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json"
)

SOURCE_ONLY_RECIPE_FIELDS = {
    "sourceContractProfileId",
    "sourceContractSha256",
    "sourceGraphSha256",
    "sourceClosureSha256",
    "sourceMaterialClosureSha256",
    "sourcePeakActiveParticles",
    "localReferenceBindings",
    "moduleCoverage",
    "compilerEvidence",
    "compiledExecutionAdmission",
    "materialAdmission",
    "geometryBinding",
}
SOURCE_ONLY_DISTRIBUTION_FIELDS = {
    "referenceId",
    "occurrenceId",
    "payloadStatus",
    "fidelity",
    "executionAdmission",
    "parameterBinding",
    "parameterName",
}

# This is the bounded ordinary-v13 subset already interpreted by Playback.
# Unknown source classes must not be silently copied for later ignore.
PORTABLE_MODULE_CLASSES = {
    "particlemoduleacceleration",
    "particlemodulecameraoffset",
    "particlemodulecolor",
    "particlemodulecoloroverlife",
    "particlemodulecolorscaleoverlife",
    "particlemodulelifetime",
    "particlemodulelocation",
    "particlemodulelocationdirect",
    "particlemodulelocationonground",
    "particlemodulelocationprimitivecylinder",
    "particlemodulelocationprimitivecylinderspin",
    "particlemodulelocationprimitivesphere",
    "particlemodulemeshrotation",
    "particlemodulemeshrotationrate",
    "particlemodulemeshrotationratemultiplylife",
    "particlemodulemeshrotationrateoverlife",
    "particlemoduleorientationaxislock",
    "particlemoduleparameterdynamic",
    "particlemodulerequired",
    "particlemodulerotation",
    "particlemodulerotationrate",
    "particlemodulesize",
    "particlemodulesizemultiplylife",
    "particlemodulespawn",
    "particlemodulespawnperunit",
    "particlemodulesubuv",
    "particlemoduletypedatamesh",
    "particlemodulevelocity",
    "particlemodulevelocityoverlifetime",
}

PORTABLE_DISTRIBUTION_PROPERTIES = {
    "particlemoduleacceleration": {"acceleration"},
    "particlemodulecameraoffset": {"cameraoffset"},
    "particlemodulecolor": {"startalpha", "startcolor"},
    "particlemodulecoloroverlife": {"alphaoverlife", "coloroverlife"},
    "particlemodulecolorscaleoverlife": {
        "alphascaleoverlife", "colorscaleoverlife"
    },
    "particlemodulelifetime": {"lifetime"},
    "particlemodulelocation": {"startlocation"},
    "particlemodulelocationdirect": {
        "direction", "location", "locationoffset", "scalefactor"
    },
    "particlemodulelocationonground": {"adjustlocation", "skiplocation"},
    "particlemodulelocationprimitivecylinder": {
        "startheight", "startlocation", "startradius", "velocityscale"
    },
    "particlemodulelocationprimitivecylinderspin": {
        "spinangle", "startcylinderrot", "startheight", "startlocation",
        "startradius", "velocityscale"
    },
    "particlemodulelocationprimitivesphere": {
        "startlocation", "startradius", "velocityscale"
    },
    "particlemodulemeshrotation": {"startrotation"},
    "particlemodulemeshrotationrate": {"startrotationrate"},
    "particlemodulemeshrotationratemultiplylife": {"lifemultiplier"},
    "particlemodulemeshrotationrateoverlife": {"rotrate"},
    "particlemoduleparameterdynamic": {
        "dynamicparams[0].paramvalue", "dynamicparams[1].paramvalue",
        "dynamicparams[2].paramvalue", "dynamicparams[3].paramvalue"
    },
    "particlemodulerequired": {"spawnrate"},
    "particlemodulerotation": {"startrotation"},
    "particlemodulerotationrate": {"startrotationrate"},
    "particlemodulesize": {"startsize"},
    "particlemodulesizemultiplylife": {"lifemultiplier"},
    "particlemodulespawn": {"rate", "ratescale"},
    "particlemodulespawnperunit": {"spawnperunit"},
    "particlemodulesubuv": {"subimageindex"},
    "particlemodulevelocity": {"startvelocity", "startvelocityradial"},
    "particlemodulevelocityoverlifetime": {"veloverlife"},
}

MESH_ONLY_MODULE_CLASSES = {
    "particlemodulemeshrotation",
    "particlemodulemeshrotationrate",
    "particlemodulemeshrotationratemultiplylife",
    "particlemodulemeshrotationrateoverlife",
    "particlemoduletypedatamesh",
}
SPRITE_ONLY_MODULE_CLASSES = {
    "particlemoduleorientationaxislock",
    "particlemodulesubuv",
}
TWO_INSTANCE_MODULE_CLASSES = {
    "particlemoduleacceleration",
    "particlemodulecolorscaleoverlife",
    "particlemodulelocation",
    "particlemodulelocationprimitivecylinderspin",
    "particlemodulesizemultiplylife",
    "particlemodulevelocity",
}

FAMILY = {
    "MeshParticle": ("MeshParticle", "mesh"),
    "SpriteParticle": ("SpriteParticle", "sprite"),
}


class MaterializeError(RuntimeError):
    pass


def load_json(path: Path) -> dict[str, Any]:
    try:
        return json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        raise MaterializeError(f"cannot read {path}: {error}") from error


def normalized_module_class(value: str) -> str:
    result = value
    if result.startswith("efparticlemodule"):
        result = result[2:]
    if result.endswith("_seeded"):
        result = result[:-7]
    return result


def stable_target_id(renderer_type: str, emitter_id: str) -> str:
    try:
        label, prefix = FAMILY[renderer_type]
    except KeyError as error:
        raise MaterializeError(f"unsupported authored Family: {renderer_type}") from error
    digest = hashlib.sha256(f"{label}\n{emitter_id}".encode("utf-8")).hexdigest()
    return f"{prefix}.{digest[:16]}"


def is_null_cdo_distribution(distribution: dict[str, Any]) -> bool:
    zeros = [0.0, 0.0, 0.0, 0.0]
    return (
        distribution.get("sourceClass", "") == ""
        and distribution.get("sourceObjectPath", "") == ""
        and distribution.get("componentCount") == 1
        and distribution.get("operation") == 1
        and distribution.get("randomLockAxes") == 0
        and distribution.get("lookupTableChunkSize") == 0
        and distribution.get("lookupTableNumElements") == 0
        and float(distribution.get("lookupTableTimeScale", 0)) == 0.0
        and float(distribution.get("lookupTableStartTime", 0)) == 0.0
        and distribution.get("defaultMinimum") == zeros
        and distribution.get("defaultMaximum") == zeros
        and distribution.get("lookupTable") == []
        and distribution.get("keys") == []
    )


def portable_recipe(source_recipe: dict[str, Any]) -> dict[str, Any]:
    if not source_recipe.get("enabled"):
        raise MaterializeError("source particle recipe is disabled")
    staged = copy.deepcopy(source_recipe)
    for field in SOURCE_ONLY_RECIPE_FIELDS:
        staged.pop(field, None)

    # The authored Detail timing already contains schedule + emitter delay.
    staged["emitterDelaySeconds"] = 0
    module_ids: set[str] = set()
    class_counts: dict[str, int] = {}
    required_count = 0
    mesh_type_data_count = 0
    for module in staged.get("modules", []):
        source_class = module.get("className")
        if not isinstance(source_class, str):
            raise MaterializeError("source module class is missing")
        normalized = normalized_module_class(source_class)
        if normalized not in PORTABLE_MODULE_CLASSES:
            raise MaterializeError(f"unsupported source module class: {source_class}")
        if normalized in {
            "particlemodulerequired",
            "particlemodulespawn",
            "particlemoduletypedatamesh",
        } and source_class != normalized:
            raise MaterializeError(
                f"module requires the exact Playback class identity: {source_class}"
            )
        stable_id = module.get("stableId")
        if not isinstance(stable_id, str) or not stable_id or stable_id in module_ids:
            raise MaterializeError(f"missing/duplicate source module ID: {stable_id}")
        module_ids.add(stable_id)
        class_counts[normalized] = class_counts.get(normalized, 0) + 1
        required_count += normalized == "particlemodulerequired"
        mesh_type_data_count += normalized == "particlemoduletypedatamesh"
        property_paths: set[str] = set()
        for literal in module.get("literals", []):
            property_path = literal.get("propertyPath")
            if not isinstance(property_path, str) or property_path in property_paths:
                raise MaterializeError(
                    f"missing/duplicate source literal property: {source_class}/{property_path}"
                )
            property_paths.add(property_path)
        for distribution in module.get("distributions", []):
            property_path = distribution.get("propertyPath")
            if not isinstance(property_path, str) or property_path in property_paths:
                raise MaterializeError(
                    "missing/duplicate source distribution property: "
                    f"{source_class}/{property_path}"
                )
            property_paths.add(property_path)
            binding = distribution.get("parameterBinding", "none")
            parameter_name = distribution.get("parameterName", "")
            if binding != "none" or parameter_name:
                raise MaterializeError(
                    "ActionCue-bound distribution cannot become an authored carrier: "
                    f"{source_class}/{distribution.get('propertyPath', '<missing>')}"
                )
            allowed_properties = PORTABLE_DISTRIBUTION_PROPERTIES.get(
                normalized, set()
            )
            if property_path not in allowed_properties:
                raise MaterializeError(
                    "unsupported source distribution capability: "
                    f"{source_class}/{property_path}"
                )
            ignored_null_cdo = (
                (normalized == "particlemodulerequired" and property_path == "spawnrate")
                or (normalized == "particlemodulespawn" and property_path == "ratescale")
            )
            if ignored_null_cdo and not is_null_cdo_distribution(distribution):
                raise MaterializeError(
                    f"ignored source distribution is not null-CDO identity: {source_class}/{property_path}"
                )
            for field in SOURCE_ONLY_DISTRIBUTION_FIELDS:
                distribution.pop(field, None)
        actual_properties = {
            distribution["propertyPath"] for distribution in module.get("distributions", [])
        }
        if actual_properties != PORTABLE_DISTRIBUTION_PROPERTIES.get(normalized, set()):
            raise MaterializeError(
                f"source distribution capability is incomplete: {source_class}"
            )
    shape = staged.get("rendererShape")
    for source_class, count in class_counts.items():
        maximum = 2 if source_class in TWO_INSTANCE_MODULE_CLASSES else 1
        if (
            count > maximum
            or (source_class in MESH_ONLY_MODULE_CLASSES and shape != "mesh")
            or (source_class in SPRITE_ONLY_MODULE_CLASSES and shape != "sprite")
        ):
            raise MaterializeError(
                f"source module Family/cardinality is unsupported: {source_class}"
            )
    if (
        required_count != 1
        or class_counts.get("particlemodulelifetime", 0) != 1
        or class_counts.get("particlemodulesize", 0) != 1
        or class_counts.get("particlemodulespawn", 0) != 1
        or (shape == "mesh") != (mesh_type_data_count == 1)
    ):
        raise MaterializeError(
            "portable Required/TypeDataMesh cardinality does not match renderer shape"
        )
    return staged


def close(left: float, right: float) -> bool:
    return abs(left - right) <= 1.0e-5 * max(1.0, abs(left), abs(right))


def normalize_legacy_generated_color(order: int, element: dict[str, Any]) -> None:
    detail = element["detail"]
    multiply = detail["color"]["multiply"]
    linear = detail["linearLerp"]
    if order in {2, 19, 31} and all(
        close(float(multiply[index]), expected)
        for index, expected in enumerate((1.0, 1.0, 1.0, 50.0))
    ) and not linear["colorMultiply"]:
        multiply[3] = 1
    if order == 23 and all(
        close(float(multiply[index]), expected)
        for index, expected in enumerate((0.5, 0.7, 0.5, 0.3))
    ) and linear["colorMultiply"] and all(
        close(float(linear["endColorMultiply"][index]), expected)
        for index, expected in enumerate((0.5, 0.7, 0.5, 0.0))
    ):
        detail["color"]["multiply"] = [1, 1, 1, 1]
        linear["colorMultiply"] = False
        linear["endColorMultiply"] = [1, 1, 1, 1]


def artist_v4_color_abi(opcode: int) -> tuple[int, int]:
    if opcode in {1, 2, 3, 6, 7, 8}:
        return 2, 0xF
    if opcode == 4:
        return 1, 0x8
    if opcode == 5:
        return 0, 0
    raise MaterializeError(f"ArtistVisualV4 opcode has no particle color ABI: {opcode}")


def materialize(
    source: dict[str, Any],
    program: dict[str, Any],
    authored: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, int]]:
    if source.get("effectAssetId") != (
        "effect.artist.skill.31470.native-v14.source-contract-candidate"
    ):
        raise MaterializeError("unexpected source effect identity")
    if authored.get("effectAssetId") != "effect.artist.skill.31470.unified":
        raise MaterializeError("unexpected authored effect identity")

    source_by_id = {element["id"]: element for element in source["elements"]}
    authored_by_id = {element["id"]: element for element in authored["elements"]}
    if len(source_by_id) != len(source["elements"]):
        raise MaterializeError("duplicate source Element ID")
    if len(authored_by_id) != len(authored["elements"]):
        raise MaterializeError("duplicate authored Element ID")

    staged = copy.deepcopy(authored)
    staged_by_id = {element["id"]: element for element in staged["elements"]}
    counts = {"recipes": 0, "modules": 0, "distributions": 0, "artistV4": 0}
    joined_targets: set[str] = set()
    for emitter in program.get("emitters", []):
        renderer_type = emitter.get("rendererType")
        if not emitter.get("visible") or renderer_type not in FAMILY:
            continue
        target_id = stable_target_id(renderer_type, emitter["emitterId"])
        if target_id in joined_targets:
            raise MaterializeError(f"duplicate target join: {target_id}")
        joined_targets.add(target_id)
        try:
            source_element = source_by_id[emitter["sourceElementId"]]
            target = staged_by_id[target_id]
        except KeyError as error:
            raise MaterializeError(f"missing source/target join: {error.args[0]}") from error
        if target.get("kind") != "particle":
            raise MaterializeError(f"portable target is not a particle: {target_id}")
        expected_shape = "mesh" if renderer_type == "MeshParticle" else "sprite"
        mesh_binding_count = sum(
            resource.get("slotId") == "meshModel" for resource in target["resources"]
        )
        if mesh_binding_count != (1 if expected_shape == "mesh" else 0):
            raise MaterializeError(f"target Mesh binding cardinality mismatch: {target_id}")
        recipe = portable_recipe(source_element["sourceRecipe"])
        if recipe.get("rendererShape") != expected_shape:
            raise MaterializeError(f"renderer shape mismatch: {target_id}")
        target["sourceRecipe"] = recipe
        normalize_legacy_generated_color(int(emitter["order"]), target)

        execution = target["material"].get("execution", {})
        if execution.get("enabled") and execution.get("backend") == "artistVisualV4":
            policy, mask = artist_v4_color_abi(int(execution["opcode"]))
            execution["particleColorPolicy"] = policy
            execution["particleColorConsumedMask"] = mask
            execution["particleColorSuppressedMask"] = 0
            counts["artistV4"] += 1

        counts["recipes"] += 1
        counts["modules"] += len(recipe["modules"])
        counts["distributions"] += sum(
            len(module["distributions"]) for module in recipe["modules"]
        )

    expected = {"recipes": 29, "modules": 350, "distributions": 564, "artistV4": 10}
    if counts != expected:
        raise MaterializeError(f"portable carrier denominator changed: {counts} != {expected}")
    return staged, counts


def serialized(document: dict[str, Any]) -> str:
    return json.dumps(document, ensure_ascii=False, indent=2) + "\n"


def atomic_write(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as stream:
            stream.write(value)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_name, path)
    except Exception:
        try:
            os.unlink(temporary_name)
        except OSError:
            pass
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    args = parser.parse_args()

    source = load_json(SOURCE_PATH)
    program = load_json(PROGRAM_PATH)
    authored = load_json(AUTHORED_PATH)
    staged, counts = materialize(source, program, authored)
    expected_text = serialized(staged)
    expected_sha = hashlib.sha256(expected_text.encode("utf-8")).hexdigest()

    if args.check:
        current = AUTHORED_PATH.read_text(encoding="utf-8-sig")
        if json.loads(current) != staged:
            raise MaterializeError(
                "authored Artist F document is not the current portable-carrier projection"
            )
        print(
            "Artist F portable particle carrier check PASS: "
            f"{counts}, canonicalSha256={expected_sha}"
        )
        return 0

    atomic_write(AUTHORED_PATH, expected_text)
    # Prove the atomic output is parseable and exactly the staged projection.
    if load_json(AUTHORED_PATH) != staged:
        raise MaterializeError("atomic authored output changed after commit")
    print(
        "Artist F portable particle carriers materialized: "
        f"{counts}, canonicalSha256={expected_sha}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except MaterializeError as error:
        print(f"ERROR: {error}")
        raise SystemExit(1)
