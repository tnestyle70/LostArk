from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import tempfile
from pathlib import Path
from typing import Any, Iterable, Sequence

BASIS = (
    (1.0, 0.0, 0.0),
    (0.0, 0.0, -1.0),
    (0.0, 1.0, 0.0),
)
IMPORTED_ID_BIT = 1 << 63
EDITOR_ID_MASK = IMPORTED_ID_BIT - 1
QUATERNION_EPSILON = 1.0e-6


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"JSON root is not an object: {path}")
    return value


def atomic_write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            output.write(text)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def token(value: str, field: str) -> str:
    if not value or len(value) > 128:
        raise ValueError(f"invalid {field}: {value!r}")
    if any(not (character.isalnum() or character in "_.-") for character in value):
        raise ValueError(f"invalid {field}: {value!r}")
    return value


def quoted(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def mat_transpose(matrix: Sequence[Sequence[float]]) -> tuple[tuple[float, ...], ...]:
    return tuple(tuple(matrix[row][column] for row in range(3)) for column in range(3))


def mat_mul(
    left: Sequence[Sequence[float]], right: Sequence[Sequence[float]]
) -> tuple[tuple[float, ...], ...]:
    return tuple(
        tuple(sum(left[row][k] * right[k][column] for k in range(3)) for column in range(3))
        for row in range(3)
    )


def convert_position(value: dict[str, Any]) -> tuple[float, float, float]:
    x, y, z = float(value["x"]), float(value["y"]), float(value["z"])
    result = (x * 0.01, z * 0.01, -y * 0.01)
    if not all(math.isfinite(component) for component in result):
        raise ValueError("non-finite position")
    return result


def convert_scale(value: dict[str, Any]) -> tuple[float, float, float]:
    x, y, z = float(value["x"]), float(value["y"]), float(value["z"])
    result = (x, z, y)
    if not all(math.isfinite(component) and abs(component) >= 1.0e-6 for component in result):
        raise ValueError("invalid signed scale")
    return result


def scale_flags(scale: Sequence[float]) -> tuple[bool, bool]:
    any_negative = any(component < 0.0 for component in scale)
    reflected = scale[0] * scale[1] * scale[2] < 0.0
    return any_negative, reflected


def ue3_rotation_rows(rotation: dict[str, Any]) -> tuple[tuple[float, ...], ...]:
    pitch = float(rotation["pitch"]) * math.pi / 32768.0
    yaw = float(rotation["yaw"]) * math.pi / 32768.0
    roll = float(rotation["roll"]) * math.pi / 32768.0
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    sr, cr = math.sin(roll), math.cos(roll)
    return (
        (cp * cy, cp * sy, sp),
        (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp),
        (-cr * sp * cy - sr * sy, -cr * sp * sy + sr * cy, cr * cp),
    )


def directx_row_matrix_from_quaternion(
    quaternion: Sequence[float],
) -> tuple[tuple[float, ...], ...]:
    x, y, z, w = quaternion
    return (
        (1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + w * z), 2.0 * (x * z - w * y)),
        (2.0 * (x * y - w * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + w * x)),
        (2.0 * (x * z + w * y), 2.0 * (y * z - w * x), 1.0 - 2.0 * (x * x + y * y)),
    )


def standard_column_quaternion(matrix: Sequence[Sequence[float]]) -> tuple[float, float, float, float]:
    trace = matrix[0][0] + matrix[1][1] + matrix[2][2]
    if trace > 0.0:
        root = math.sqrt(trace + 1.0) * 2.0
        result = (
            (matrix[2][1] - matrix[1][2]) / root,
            (matrix[0][2] - matrix[2][0]) / root,
            (matrix[1][0] - matrix[0][1]) / root,
            0.25 * root,
        )
    elif matrix[0][0] > matrix[1][1] and matrix[0][0] > matrix[2][2]:
        root = math.sqrt(1.0 + matrix[0][0] - matrix[1][1] - matrix[2][2]) * 2.0
        result = (
            0.25 * root,
            (matrix[0][1] + matrix[1][0]) / root,
            (matrix[0][2] + matrix[2][0]) / root,
            (matrix[2][1] - matrix[1][2]) / root,
        )
    elif matrix[1][1] > matrix[2][2]:
        root = math.sqrt(1.0 + matrix[1][1] - matrix[0][0] - matrix[2][2]) * 2.0
        result = (
            (matrix[0][1] + matrix[1][0]) / root,
            0.25 * root,
            (matrix[1][2] + matrix[2][1]) / root,
            (matrix[0][2] - matrix[2][0]) / root,
        )
    else:
        root = math.sqrt(1.0 + matrix[2][2] - matrix[0][0] - matrix[1][1]) * 2.0
        result = (
            (matrix[0][2] + matrix[2][0]) / root,
            (matrix[1][2] + matrix[2][1]) / root,
            0.25 * root,
            (matrix[1][0] - matrix[0][1]) / root,
        )
    length = math.sqrt(sum(component * component for component in result))
    if length < QUATERNION_EPSILON:
        raise ValueError("zero rotation quaternion")
    normalized = tuple(component / length for component in result)
    if normalized[3] < 0.0:
        normalized = tuple(-component for component in normalized)
    return normalized


def convert_rotation(rotation: dict[str, Any]) -> tuple[float, float, float, float]:
    ue3 = ue3_rotation_rows(rotation)
    client = mat_mul(mat_mul(mat_transpose(BASIS), ue3), BASIS)
    quaternion = standard_column_quaternion(mat_transpose(client))
    round_trip = directx_row_matrix_from_quaternion(quaternion)
    error = max(abs(round_trip[row][column] - client[row][column]) for row in range(3) for column in range(3))
    if error > 1.0e-5:
        raise ValueError(f"rotation round-trip failed: {error}")
    return quaternion


def imported_id(source_placement_id: str) -> int:
    digest = hashlib.sha256(source_placement_id.encode("utf-8")).digest()
    return int.from_bytes(digest[:8], "big") | IMPORTED_ID_BIT


def classify_non_visual_asset(asset: dict[str, Any]) -> str | None:
    root_import = str(asset.get("rootImport", "")).casefold()
    logical_package = str(asset.get("logicalPackage", "")).casefold()
    object_path = str(asset.get("fullPath", "")).casefold()
    object_name = str(asset.get("objectName", "")).casefold()
    if (
        root_import == "lv_navimesh"
        or logical_package == "lv_navimesh"
        or object_path.startswith("lv_navimesh.")
    ):
        return "nav-helper"
    if root_import == "lv_module" or logical_package == "lv_module":
        return "module-proxy"
    if (
        (root_import == "lv_lut_heartrb" or logical_package == "lv_lut_heartrb")
        and object_name.startswith("lv_lut_heartrb_water")
    ):
        return "deferred-water"
    if (
        root_import.startswith("bfx_")
        or logical_package.startswith("bfx_")
        or object_name.startswith("bfm_")
        or "cloudplane" in object_name
    ):
        return "deferred-fx"
    return None


def is_non_visual_helper_asset(asset: dict[str, Any]) -> bool:
    return classify_non_visual_asset(asset) is not None


def finite_vector(
    value: Sequence[Any], length: int, field: str
) -> tuple[float, ...]:
    if len(value) != length:
        raise ValueError(f"invalid {field} length")
    result = tuple(float(component) for component in value)
    if not all(math.isfinite(component) for component in result):
        raise ValueError(f"non-finite {field}")
    return result


def normalize_quaternion(value: Sequence[Any]) -> tuple[float, float, float, float]:
    quaternion = finite_vector(value, 4, "overlay quaternion")
    length = math.sqrt(sum(component * component for component in quaternion))
    if length < QUATERNION_EPSILON:
        raise ValueError("zero overlay quaternion")
    result = tuple(component / length for component in quaternion)
    if result[3] < 0.0:
        result = tuple(-component for component in result)
    return result


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def iter_placements(paths: Iterable[Path]) -> Iterable[dict[str, Any]]:
    for path in sorted(paths, key=lambda value: value.name):
        document = load_json(path)
        if document.get("schemaVersion") != 1 or document.get("propertyErrors"):
            raise ValueError(f"invalid placement source: {path}")
        for placement in document.get("placements", []):
            yield placement


def compile_scene(args: argparse.Namespace) -> dict[str, Any]:
    area_id = token(args.area_id, "areaId")
    asset_manifest = load_json(args.asset_manifest)
    runtime_manifest = load_json(args.runtime_manifest)
    overlay_manifest = (
        load_json(args.overlay_manifest) if args.overlay_manifest is not None else None
    )
    assets = asset_manifest.get("assets", [])
    runtime_assets = runtime_manifest.get("assets", [])
    if asset_manifest.get("areaId") != area_id or runtime_manifest.get("areaId") != area_id:
        raise ValueError("manifest areaId mismatch")
    if overlay_manifest is not None and (
        overlay_manifest.get("schemaVersion") != 1
        or overlay_manifest.get("areaId") != area_id
    ):
        raise ValueError("overlay manifest schema/areaId mismatch")

    assets_by_path = {str(asset["fullPath"]).lower(): asset for asset in assets}
    runtime_by_id = {str(asset["assetId"]): asset for asset in runtime_assets}
    if len(assets_by_path) != len(assets) or len(runtime_by_id) != len(runtime_assets):
        raise ValueError("duplicate asset key")
    if set(str(asset["assetId"]) for asset in assets) != set(runtime_by_id):
        raise ValueError("asset/runtime manifest set mismatch")

    catalog_rows: list[str] = []
    catalog_ids: set[str] = set()
    prototype_tags: set[str] = set()
    for asset in sorted(assets, key=lambda value: str(value["assetId"])):
        asset_id = token(str(asset["assetId"]), "assetId")
        runtime = runtime_by_id[asset_id]
        model_relative = Path(str(runtime["model"]))
        model_absolute = args.runtime_root / model_relative
        if not model_absolute.is_file():
            raise ValueError(f"runtime model is missing: {model_absolute}")
        with model_absolute.open("rb") as model_stream:
            model_magic = model_stream.read(4)
        if model_magic not in (b"WINT", b"WMOD"):
            raise ValueError(f"invalid runtime model: {model_absolute}")
        model_path = (Path("Map") / area_id / model_relative).as_posix()
        source_group = token(str(asset["sourceCategory"]).lower(), "groupId")
        evidence = "UE3 ImportTable exact: " + str(asset["fullPath"])
        prototype_tag = "Prototype_Component_Model_" + asset_id
        if asset_id in catalog_ids or prototype_tag in prototype_tags:
            raise ValueError(f"duplicate generated catalog key: {asset_id}")
        catalog_ids.add(asset_id)
        prototype_tags.add(prototype_tag)
        catalog_rows.append(
            " ".join((
                quoted(asset_id), quoted(str(asset["objectName"])), quoted(model_path),
                quoted(prototype_tag), "1 1 1 Origin",
                quoted(source_group), quoted(str(asset["logicalPackage"])), quoted(evidence),
            ))
        )

    overlay_assets = [] if overlay_manifest is None else overlay_manifest.get("assets", [])
    if not isinstance(overlay_assets, list):
        raise ValueError("overlay assets must be an array")
    if overlay_assets and args.runtime_asset_root is None:
        raise ValueError("runtime-asset-root is required for overlay assets")
    for asset in sorted(overlay_assets, key=lambda value: str(value["assetId"])):
        asset_id = token(str(asset["assetId"]), "overlay assetId")
        prototype_tag = token(str(asset["prototypeTag"]), "overlay prototypeTag")
        group_id = token(str(asset["groupId"]), "overlay groupId")
        model_path = Path(str(asset["modelPath"])).as_posix()
        model_relative = Path(model_path)
        if model_relative.is_absolute() or model_relative.suffix.casefold() != ".wmodel":
            raise ValueError(f"invalid overlay model path: {model_path}")
        model_absolute = args.runtime_asset_root / model_relative
        if not model_absolute.is_file():
            raise ValueError(f"overlay runtime model is missing: {model_absolute}")
        with model_absolute.open("rb") as model_stream:
            if model_stream.read(4) not in (b"WINT", b"WMOD"):
                raise ValueError(f"invalid overlay runtime model: {model_absolute}")
        default_scale = finite_vector(asset.get("defaultScale", []), 3, "overlay defaultScale")
        if any(component <= 0.0 for component in default_scale):
            raise ValueError(f"invalid overlay default scale: {asset_id}")
        anchor = str(asset.get("anchor", ""))
        if anchor not in ("Origin", "BottomCenter"):
            raise ValueError(f"invalid overlay anchor: {asset_id}")
        if asset_id in catalog_ids or prototype_tag in prototype_tags:
            raise ValueError(f"duplicate overlay catalog key: {asset_id}")
        catalog_ids.add(asset_id)
        prototype_tags.add(prototype_tag)
        catalog_rows.append(
            " ".join((
                quoted(asset_id), quoted(str(asset["label"])), quoted(model_path),
                quoted(prototype_tag),
                " ".join(format(value, ".9g") for value in default_scale),
                anchor, quoted(group_id), quoted(str(asset["groupLabel"])),
                quoted(str(asset["evidence"])),
            ))
        )

    placement_rows: list[dict[str, Any]] = []
    seen_runtime_ids: dict[int, str] = {}
    seen_source_ids: set[str] = set()
    any_negative_scale_count = 0
    reflected_count = 0
    hidden_helper_count = 0
    hidden_category_counts: dict[str, int] = {}
    source_level_counts: dict[str, int] = {}
    central_result: dict[str, Any] | None = None
    for placement in iter_placements(args.placements_dir.glob("*.placements.json")):
        source_id = str(placement["placementId"])
        if not source_id or source_id in seen_source_ids:
            raise ValueError(f"duplicate/empty source placement ID: {source_id!r}")
        seen_source_ids.add(source_id)
        runtime_id = imported_id(source_id)
        previous = seen_runtime_ids.get(runtime_id)
        if previous is not None and previous != source_id:
            raise ValueError(f"runtime ID collision: {previous!r} / {source_id!r}")
        seen_runtime_ids[runtime_id] = source_id
        object_path = str(placement["asset"]["objectPath"]).lower()
        asset = assets_by_path.get(object_path)
        if asset is None:
            raise ValueError(f"asset join missing: {object_path}")
        hidden_category = classify_non_visual_asset(asset)
        visible = hidden_category is None
        hidden_helper_count += int(hidden_category == "nav-helper")
        if hidden_category is not None:
            hidden_category_counts[hidden_category] = (
                hidden_category_counts.get(hidden_category, 0) + 1
            )
        transform = placement["transform"]
        position = convert_position(transform["position"])
        rotation = convert_rotation(transform["rotation"])
        scale = convert_scale(transform["scale3D"])
        any_negative, reflected = scale_flags(scale)
        any_negative_scale_count += int(any_negative)
        reflected_count += int(reflected)
        source_level = token(str(placement["levelPackage"]), "sourceLevel")
        source_level_counts[source_level] = source_level_counts.get(source_level, 0) + 1
        transform_source = token(str(transform["source"]), "transformSource")
        if transform_source not in ("actor", "component"):
            raise ValueError(f"unexpected exact transform source: {transform_source}")
        values = (*position, *rotation, *scale)
        placement_rows.append(
            {
                "sourcePlacementId": source_id,
                "sourceLevel": source_level,
                "anyNegative": any_negative,
                "reflected": reflected,
                "hiddenHelper": hidden_category == "nav-helper",
                "hiddenCategory": hidden_category,
                "isOverlay": False,
                "text": (
                    f"{runtime_id} {quoted(source_id)} {quoted(source_level)} "
                    f"{quoted(transform_source)} {quoted(str(asset['assetId']))} "
                    + " ".join(format(value, ".9g") for value in values)
                    + (" 1" if visible else " 0")
                ),
            }
        )
        if source_id == args.golden_placement_id:
            central_result = {
                "runtimeId": runtime_id,
                "assetId": asset["assetId"],
                "position": position,
                "quaternion": rotation,
                "signedScale": scale,
            }

    overlay_rows: list[dict[str, Any]] = []
    overlay_placements = (
        [] if overlay_manifest is None else overlay_manifest.get("placements", [])
    )
    if not isinstance(overlay_placements, list):
        raise ValueError("overlay placements must be an array")
    for placement in overlay_placements:
        runtime_id = int(placement["placementId"])
        source_id = str(placement["sourcePlacementId"])
        if not source_id or source_id in seen_source_ids:
            raise ValueError(f"duplicate/empty overlay source ID: {source_id!r}")
        seen_source_ids.add(source_id)
        if runtime_id <= 0 or runtime_id > EDITOR_ID_MASK:
            raise ValueError(f"overlay placement ID is outside its domain: {runtime_id}")
        previous = seen_runtime_ids.get(runtime_id)
        if previous is not None:
            raise ValueError(f"overlay runtime ID collision: {previous!r} / {source_id!r}")
        seen_runtime_ids[runtime_id] = source_id
        asset_id = token(str(placement["assetId"]), "overlay placement assetId")
        if asset_id not in catalog_ids:
            raise ValueError(f"overlay placement asset join missing: {asset_id}")
        source_level = token(str(placement["sourceLevel"]), "overlay sourceLevel")
        transform_source = token(
            str(placement.get("transformSource", "overlay")),
            "overlay transformSource",
        )
        if transform_source != "overlay":
            raise ValueError("overlay placement transformSource must be overlay")
        position = finite_vector(placement.get("position", []), 3, "overlay position")
        rotation = normalize_quaternion(placement.get("quaternion", []))
        scale = finite_vector(placement.get("scale", []), 3, "overlay scale")
        if any(abs(component) < 1.0e-6 for component in scale):
            raise ValueError(f"overlay placement has a zero scale axis: {source_id}")
        any_negative, reflected = scale_flags(scale)
        visible = bool(placement.get("visible", True))
        values = (*position, *rotation, *scale)
        overlay_rows.append(
            {
                "sourcePlacementId": source_id,
                "sourceLevel": source_level,
                "anyNegative": any_negative,
                "reflected": reflected,
                "hiddenHelper": False,
                "hiddenCategory": None if visible else "overlay-hidden",
                "isOverlay": True,
                "text": (
                    f"{runtime_id} {quoted(source_id)} {quoted(source_level)} "
                    f"{quoted(transform_source)} {quoted(asset_id)} "
                    + " ".join(format(value, ".9g") for value in values)
                    + (" 1" if visible else " 0")
                ),
            }
        )

    if args.expect_assets is not None and len(catalog_rows) != args.expect_assets:
        raise ValueError(f"asset count mismatch: {len(catalog_rows)}")
    if args.expect_source_placements is not None and len(placement_rows) != args.expect_source_placements:
        raise ValueError(f"source placement count mismatch: {len(placement_rows)}")
    if args.expect_any_negative is not None and any_negative_scale_count != args.expect_any_negative:
        raise ValueError(f"any-negative scale count mismatch: {any_negative_scale_count}")
    if args.expect_reflected is not None and reflected_count != args.expect_reflected:
        raise ValueError(f"reflected count mismatch: {reflected_count}")
    if args.expect_hidden_helpers is not None and hidden_helper_count != args.expect_hidden_helpers:
        raise ValueError(f"hidden helper count mismatch: {hidden_helper_count}")
    if (
        args.expect_overlay_assets is not None
        and len(overlay_assets) != args.expect_overlay_assets
    ):
        raise ValueError(f"overlay asset count mismatch: {len(overlay_assets)}")
    if (
        args.expect_overlay_placements is not None
        and len(overlay_rows) != args.expect_overlay_placements
    ):
        raise ValueError(f"overlay placement count mismatch: {len(overlay_rows)}")
    expected_hidden_categories: dict[str, int] = {}
    for specification in args.expect_hidden_category:
        category, separator, raw_count = specification.partition("=")
        if not separator or not category:
            raise ValueError(f"invalid expected hidden category: {specification}")
        if category in expected_hidden_categories:
            raise ValueError(f"duplicate expected hidden category: {category}")
        expected_hidden_categories[category] = int(raw_count)
    if (
        expected_hidden_categories
        and expected_hidden_categories != hidden_category_counts
    ):
        raise ValueError(
            f"hidden category counts mismatch: {hidden_category_counts}"
        )
    expected_level_counts: dict[str, int] = {}
    for specification in args.expect_level_count:
        level, separator, raw_count = specification.partition("=")
        if not separator:
            raise ValueError(f"invalid expected level count: {specification}")
        level = token(level, "expected sourceLevel")
        if level in expected_level_counts:
            raise ValueError(f"duplicate expected sourceLevel: {level}")
        expected_level_counts[level] = int(raw_count)
    if expected_level_counts and expected_level_counts != source_level_counts:
        raise ValueError(
            f"source level counts mismatch: {source_level_counts}"
        )
    if args.golden_placement_id and central_result is None:
        raise ValueError("golden placement is missing")

    requested_ids = set(args.include_source_id)
    requested_levels = set(args.include_level)
    if requested_ids and requested_levels:
        raise ValueError("include-source-id and include-level are mutually exclusive")
    placement_rows.extend(overlay_rows)
    placement_rows.sort(key=lambda row: str(row["sourcePlacementId"]))
    if requested_ids:
        selected_rows = [
            row for row in placement_rows
            if str(row["sourcePlacementId"]) in requested_ids
        ]
        found_ids = {str(row["sourcePlacementId"]) for row in selected_rows}
        if found_ids != requested_ids:
            raise ValueError(f"fixture source IDs are missing: {sorted(requested_ids - found_ids)}")
        output_scope = "fixture"
    elif requested_levels:
        selected_rows = [
            row for row in placement_rows
            if str(row["sourceLevel"]) in requested_levels
        ]
        found_levels = {str(row["sourceLevel"]) for row in selected_rows}
        if found_levels != requested_levels:
            raise ValueError(f"source levels are missing: {sorted(requested_levels - found_levels)}")
        output_scope = "levels"
    else:
        selected_rows = placement_rows
        output_scope = "all"

    if args.expect_output_placements is not None and len(selected_rows) != args.expect_output_placements:
        raise ValueError(f"output placement count mismatch: {len(selected_rows)}")
    output_any_negative = sum(bool(row["anyNegative"]) for row in selected_rows)
    output_reflected = sum(bool(row["reflected"]) for row in selected_rows)
    output_hidden_helper = sum(bool(row["hiddenHelper"]) for row in selected_rows)
    output_hidden_category_counts: dict[str, int] = {}
    for row in selected_rows:
        category = row["hiddenCategory"]
        if category is not None:
            output_hidden_category_counts[category] = (
                output_hidden_category_counts.get(category, 0) + 1
            )
    if args.expect_output_any_negative is not None and output_any_negative != args.expect_output_any_negative:
        raise ValueError(f"output any-negative count mismatch: {output_any_negative}")
    if args.expect_output_reflected is not None and output_reflected != args.expect_output_reflected:
        raise ValueError(f"output reflected count mismatch: {output_reflected}")
    if (
        args.expect_output_hidden_helpers is not None
        and output_hidden_helper != args.expect_output_hidden_helpers
    ):
        raise ValueError(f"output hidden helper count mismatch: {output_hidden_helper}")

    catalog_text = (
        f"LOSTARK_MAP_ASSET_CATALOG 2 {quoted(area_id)} {len(catalog_rows)}\n"
        + "\n".join(catalog_rows) + "\n"
    )
    placement_text = (
        f"LOSTARK_MAP_PLACEMENTS 2 {quoted(area_id)} {len(selected_rows)}\n"
        + "\n".join(str(row["text"]) for row in selected_rows) + "\n"
    )
    atomic_write_text(args.catalog_output, catalog_text)
    atomic_write_text(args.placement_output, placement_text)
    receipt = {
        "schemaVersion": 1,
        "areaId": area_id,
        "outputScope": output_scope,
        "assetCount": len(catalog_rows),
        "exactAssetCount": len(assets),
        "overlayAssetCount": len(overlay_assets),
        "sourcePlacementCount": len(placement_rows) - len(overlay_rows),
        "overlayPlacementCount": len(overlay_rows),
        "placementCount": len(selected_rows),
        "sourceAnyNegativeScaleCount": any_negative_scale_count,
        "sourceReflectedCount": reflected_count,
        "sourceHiddenHelperCount": hidden_helper_count,
        "sourceHiddenCategoryCounts": dict(sorted(hidden_category_counts.items())),
        "anyNegativeScaleCount": output_any_negative,
        "reflectedCount": output_reflected,
        "hiddenHelperCount": output_hidden_helper,
        "hiddenCategoryCounts": dict(sorted(output_hidden_category_counts.items())),
        "sourceLevelCounts": dict(sorted(source_level_counts.items())),
        "goldenPlacement": central_result,
        "inputs": {
            "assetManifest": sha256(args.asset_manifest),
            "runtimeManifest": sha256(args.runtime_manifest),
            "placements": {
                path.name: sha256(path)
                for path in sorted(
                    args.placements_dir.glob("*.placements.json"),
                    key=lambda value: value.name,
                )
            },
            "overlayManifest": (
                sha256(args.overlay_manifest)
                if args.overlay_manifest is not None
                else None
            ),
        },
        "outputs": {
            "catalog": sha256(args.catalog_output),
            "placements": sha256(args.placement_output),
        },
    }
    atomic_write_text(args.receipt_output, json.dumps(receipt, ensure_ascii=False, indent=2) + "\n")
    return receipt


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--asset-manifest", type=Path, required=True)
    parser.add_argument("--runtime-manifest", type=Path, required=True)
    parser.add_argument("--runtime-root", type=Path, required=True)
    parser.add_argument("--runtime-asset-root", type=Path)
    parser.add_argument("--overlay-manifest", type=Path)
    parser.add_argument("--placements-dir", type=Path, required=True)
    parser.add_argument("--catalog-output", type=Path, required=True)
    parser.add_argument("--placement-output", type=Path, required=True)
    parser.add_argument("--receipt-output", type=Path, required=True)
    parser.add_argument("--golden-placement-id", default="")
    parser.add_argument("--include-source-id", action="append", default=[])
    parser.add_argument("--include-level", action="append", default=[])
    parser.add_argument("--expect-assets", type=int)
    parser.add_argument("--expect-source-placements", type=int)
    parser.add_argument("--expect-output-placements", type=int)
    parser.add_argument("--expect-output-any-negative", type=int)
    parser.add_argument("--expect-output-reflected", type=int)
    parser.add_argument("--expect-any-negative", type=int)
    parser.add_argument("--expect-reflected", type=int)
    parser.add_argument("--expect-hidden-helpers", type=int)
    parser.add_argument("--expect-output-hidden-helpers", type=int)
    parser.add_argument("--expect-overlay-assets", type=int)
    parser.add_argument("--expect-overlay-placements", type=int)
    parser.add_argument("--expect-hidden-category", action="append", default=[])
    parser.add_argument("--expect-level-count", action="append", default=[])
    return parser.parse_args()


def main() -> int:
    receipt = compile_scene(parse_args())
    print(json.dumps(receipt, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
