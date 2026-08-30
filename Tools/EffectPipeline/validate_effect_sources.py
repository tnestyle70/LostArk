#!/usr/bin/env python3
"""Validate the Effect Product source contract without producing artifacts."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any


DIRECT_KIND = "DIRECT_AUTHORED_DOCUMENT"
BASE_KEYS = ("effectAssetId", "payloadKind", "authoringPath")
OVERLAY_KEYS = BASE_KEYS + ("screenOverlayPresentationPath",)
AUDITION_KEYS = BASE_KEYS + (
    "runtimeAdmission",
    "fidelityClass",
    "sourceEffectAssetId",
    "sourceDocumentRawSha256",
)
AUDITION_OVERLAY_KEYS = AUDITION_KEYS + ("screenOverlayPresentationPath",)
ALLOWED_DIRECT_KEY_ORDERS = {
    BASE_KEYS,
    OVERLAY_KEYS,
    AUDITION_KEYS,
    AUDITION_OVERLAY_KEYS,
}
STABLE_ID = re.compile(r"^[a-z0-9][a-z0-9._-]{0,127}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")
SUPPORTED_PRODUCT_DOCUMENT_VERSIONS = frozenset({13, 15})
RUNTIME_RESOURCE_ID_KEYS = frozenset({"assetId", "modelAssetId", "textureAssetId"})
RUNTIME_RESOURCE_SUFFIXES = frozenset({".dds", ".wmodel"})
RUNTIME_EXTENSION_KEYS = ("formatVersion", "bakedEdgeHistories")
BAKED_HISTORY_KEYS = (
    "historyId",
    "coordinateBasis",
    "sourceEndTimeSeconds",
    "playbackClampSeconds",
    "samples",
)
BAKED_SAMPLE_KEYS = (
    "relativeTimeSeconds",
    "firstEdgeUE3Cm",
    "controlPointUE3Cm",
    "secondEdgeUE3Cm",
)
CARRIER_KEYS = {
    "cascadeRibbonV1": (
        "formatVersion",
        "kind",
        "admission",
        "typeDataModuleStableId",
    ),
    "animationTrailBakedEdgeV1": (
        "formatVersion",
        "kind",
        "admission",
        "historyId",
    ),
    "lightBakedEdgeAttachmentV1": (
        "formatVersion",
        "kind",
        "admission",
        "historyId",
        "edgeLane",
    ),
}
PLAYER_EFFECT_EVENT_PATHS = (
    "Data/Animation/Authored/Artist/Artist.animevents",
    "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents",
    "Data/Animation/Authored/LanceMaster/LanceMaster.animevents",
    "Data/Animation/Authored/Warlord/Warlord.animevents",
)
VALTAN_CUE_PATH = "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
VALTAN_V1_ALIAS_PATH = (
    "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json"
)
VALTAN_BOSS_CATALOG_PATH = "Data/Actors/BossCatalog.json"
PLAYER_SKILLS_PATH = "Data/Balance/PlayerSkills.json"
VALTAN_DRAFT_PATH = "Data/Effects/ValtanPatternAuthoringEffects.json"
EFFECT_ASSET_PAYLOAD = re.compile(
    r'\bpayload="([^"]+)"[^\r\n]*\beffectref=asset\b'
)


class ContractError(RuntimeError):
    pass


@dataclass(frozen=True)
class ValidationReport:
    direct_source_count: int
    unbound_reference_count: int
    source_bytes: int
    resource_file_count: int
    resource_bytes: int
    allow_local_resources: bool = True
    local_untracked_resource_count: int = 0

    def as_json(self) -> str:
        return json.dumps(
            {
                "schema": "lostark.effect-source-validation-result",
                "formatVersion": 1,
                "directSourceCount": self.direct_source_count,
                "unboundReferenceCount": self.unbound_reference_count,
                "sourceBytes": self.source_bytes,
                "resourceFileCount": self.resource_file_count,
                "resourceBytes": self.resource_bytes,
                "allowLocalResources": self.allow_local_resources,
                "localUntrackedResourceCount": self.local_untracked_resource_count,
                "generatedArtifactCount": 0,
            },
            ensure_ascii=False,
            separators=(",", ":"),
        )


@dataclass(frozen=True)
class RuntimeResourceIdentity:
    size: int
    sha256: str
    git_blob_sha1: str
    git_blob_sha256: str


@dataclass(frozen=True)
class GitIndexEntry:
    mode: str
    object_id: str
    stage: int


def _read_json(path: Path) -> tuple[dict[str, Any], bytes]:
    try:
        raw = path.read_bytes()
    except OSError as exc:
        raise ContractError(f"missing or unreadable JSON: {path}: {exc}") from exc
    if not raw or len(raw) > 64 * 1024 * 1024:
        raise ContractError(f"JSON size is invalid: {path}")
    try:
        value = json.loads(raw.decode("utf-8-sig"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"invalid UTF-8 JSON: {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value, raw


def _require_runtime_resource_id(value: Any, field: str) -> str:
    if not isinstance(value, str) or not value:
        raise ContractError(f"{field} must be a non-empty runtime resource ID")
    parts = value.split("/")
    path = PurePosixPath(value)
    if (
        "\\" in value
        or ":" in value
        or path.is_absolute()
        or path.as_posix() != value
        or any(not part or part in {".", ".."} for part in parts)
    ):
        raise ContractError(f"unsafe Resources-relative runtime resource ID: {field}: {value!r}")
    if path.suffix.casefold() not in RUNTIME_RESOURCE_SUFFIXES:
        raise ContractError(
            f"runtime resource ID must end in .dds or .wmodel: {field}: {value!r}"
        )
    return value


def _collect_runtime_resource_ids(value: Any, owner: str) -> set[str]:
    resource_ids: set[str] = set()

    def visit(node: Any, field: str) -> None:
        if isinstance(node, dict):
            for key, child in node.items():
                child_field = f"{field}.{key}"
                if key in RUNTIME_RESOURCE_ID_KEYS and child != "":
                    resource_ids.add(_require_runtime_resource_id(child, child_field))
                if isinstance(child, (dict, list)):
                    visit(child, child_field)
        elif isinstance(node, list):
            for index, child in enumerate(node):
                visit(child, f"{field}[{index}]")

    visit(value, owner)
    return resource_ids


def _run_git(root: Path, arguments: list[str], *, input_bytes: bytes | None = None) -> bytes:
    try:
        completed = subprocess.run(
            ["git", "-C", str(root), *arguments],
            check=False,
            input=input_bytes,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except OSError as exc:
        raise ContractError(f"Git resource closure command could not start: {exc}") from exc
    if completed.returncode != 0:
        detail = completed.stderr.decode("utf-8", errors="replace").strip()
        raise ContractError(
            f"Git resource closure command failed ({' '.join(arguments)}): {detail}"
        )
    return completed.stdout


def _git_index_resource_entries(
    root: Path,
) -> dict[str, list[GitIndexEntry]] | None:
    if not (root / ".git").exists():
        return None
    output = _run_git(
        root,
        [
            "ls-files",
            "--stage",
            "--full-name",
            "-z",
            "--",
            "Client/Bin/Resources",
        ],
    )
    result: dict[str, list[GitIndexEntry]] = {}
    for record in output.split(b"\0"):
        if not record:
            continue
        try:
            metadata, encoded_path = record.split(b"\t", 1)
            mode, object_id, stage_text = metadata.decode("ascii").split(" ")
            path = encoded_path.decode("utf-8", errors="surrogateescape")
            stage = int(stage_text)
        except (UnicodeDecodeError, ValueError) as exc:
            raise ContractError("Git resource index entry is malformed") from exc
        result.setdefault(path, []).append(
            GitIndexEntry(mode=mode, object_id=object_id, stage=stage)
        )
    return result


def _git_filter_attributes(root: Path, paths: set[str]) -> dict[str, str]:
    if not paths:
        return {}
    encoded_paths = b"\0".join(
        path.encode("utf-8", errors="surrogateescape") for path in sorted(paths)
    ) + b"\0"
    output = _run_git(
        root,
        ["check-attr", "-z", "--cached", "--stdin", "filter"],
        input_bytes=encoded_paths,
    )
    fields = output.split(b"\0")
    if fields and fields[-1] == b"":
        fields.pop()
    if len(fields) % 3 != 0:
        raise ContractError("Git resource filter attribute output is malformed")
    result: dict[str, str] = {}
    for index in range(0, len(fields), 3):
        path = fields[index].decode("utf-8", errors="surrogateescape")
        attribute = fields[index + 1].decode("ascii", errors="replace")
        value = fields[index + 2].decode("utf-8", errors="replace")
        if attribute != "filter" or path in result:
            raise ContractError("Git resource filter attribute output is malformed")
        result[path] = value
    if set(result) != paths:
        raise ContractError("Git resource filter attributes are incomplete")
    return result


def _git_blob_payloads(root: Path, object_ids: set[str]) -> dict[str, bytes]:
    if not object_ids:
        return {}
    request = b"".join(
        object_id.encode("ascii") + b"\n" for object_id in sorted(object_ids)
    )
    output = _run_git(root, ["cat-file", "--batch"], input_bytes=request)
    offset = 0
    result: dict[str, bytes] = {}
    for requested_id in sorted(object_ids):
        header_end = output.find(b"\n", offset)
        if header_end < 0:
            raise ContractError("Git resource blob batch output is truncated")
        header = output[offset:header_end].decode("ascii", errors="replace")
        offset = header_end + 1
        fields = header.split(" ")
        if len(fields) != 3 or fields[1] != "blob":
            raise ContractError(
                f"Git resource index object is not a readable blob: {requested_id}"
            )
        returned_id, _, size_text = fields
        try:
            size = int(size_text)
        except ValueError as exc:
            raise ContractError("Git resource blob size is malformed") from exc
        payload_end = offset + size
        if (
            returned_id != requested_id
            or payload_end >= len(output)
            or output[payload_end : payload_end + 1] != b"\n"
        ):
            raise ContractError("Git resource blob batch output is malformed")
        result[requested_id] = output[offset:payload_end]
        offset = payload_end + 1
    if offset != len(output):
        raise ContractError("Git resource blob batch output has trailing data")
    return result


def _git_blob_oid(payload: bytes, algorithm: str) -> str:
    digest = hashlib.new(algorithm)
    digest.update(f"blob {len(payload)}\0".encode("ascii"))
    digest.update(payload)
    return digest.hexdigest()


def _read_runtime_resource_identity(
    resource_path: Path, resource_id: str
) -> RuntimeResourceIdentity:
    try:
        payload = resource_path.read_bytes()
    except OSError as exc:
        raise ContractError(
            f"Effect runtime resource is unreadable: {resource_id}: {exc}"
        ) from exc
    suffix = PurePosixPath(resource_id).suffix.casefold()
    if suffix == ".dds":
        if payload.startswith(b"version https://git-lfs.github.com/spec/v1"):
            raise ContractError(
                f"Effect DDS working copy is an unhydrated Git LFS pointer: {resource_id}"
            )
        if not payload.startswith(b"DDS "):
            raise ContractError(f"Effect DDS magic is invalid: {resource_id}")
    elif not payload.startswith(b"WINT"):
        raise ContractError(f"Effect WModel magic is invalid: {resource_id}")
    return RuntimeResourceIdentity(
        size=len(payload),
        sha256=hashlib.sha256(payload).hexdigest(),
        git_blob_sha1=_git_blob_oid(payload, "sha1"),
        git_blob_sha256=_git_blob_oid(payload, "sha256"),
    )


def _parse_lfs_pointer(payload: bytes, path: str) -> tuple[str, int]:
    try:
        text = payload.decode("ascii")
    except UnicodeDecodeError as exc:
        raise ContractError(f"Effect DDS index blob is not an LFS pointer: {path}") from exc
    match = re.fullmatch(
        r"version https://git-lfs\.github\.com/spec/v1\n"
        r"oid sha256:([0-9a-f]{64})\n"
        r"size ([0-9]+)\n",
        text,
    )
    if match is None:
        raise ContractError(f"Effect DDS index blob is not a canonical LFS pointer: {path}")
    return match.group(1), int(match.group(2))


def _validate_runtime_resource_closure(
    root: Path,
    resource_ids: set[str],
    *,
    resources_root: Path | None = None,
    allow_local_resources: bool = True,
) -> tuple[int, int, int]:
    resources_root = (
        resources_root.resolve()
        if resources_root is not None
        else (root / "Client/Bin/Resources").resolve()
    )
    resource_bytes = 0
    missing: list[str] = []
    identities: dict[str, RuntimeResourceIdentity] = {}
    for resource_id in sorted(resource_ids):
        resource_path = resources_root.joinpath(*PurePosixPath(resource_id).parts)
        if not resource_path.is_file():
            missing.append(resource_id)
            continue
        identity = _read_runtime_resource_identity(resource_path, resource_id)
        identities[resource_id] = identity
        resource_bytes += identity.size
    if missing:
        raise ContractError(
            "Effect runtime resource files are missing: " + ", ".join(missing[:5])
        )

    local_untracked_resource_count = 0
    index_entries = _git_index_resource_entries(root)
    if index_entries is not None:
        expected_by_path = {
            f"Client/Bin/Resources/{resource_id}": resource_id
            for resource_id in resource_ids
        }
        expected_paths = set(expected_by_path)
        untracked = sorted(expected_paths - set(index_entries))
        if untracked and not allow_local_resources:
            raise ContractError(
                "Effect runtime resource closure is not tracked by Git: "
                + ", ".join(untracked[:5])
            )
        # Every local file was read and checked above. Opting into shared local
        # resources does not relax staged/LFS identity checks for tracked files.
        local_untracked_resource_count = len(untracked)
        expected_paths.difference_update(untracked)
        staged_entries: dict[str, GitIndexEntry] = {}
        for path in sorted(expected_paths):
            entries = index_entries[path]
            if (
                len(entries) != 1
                or entries[0].stage != 0
                or entries[0].mode != "100644"
                or re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", entries[0].object_id)
                is None
                or set(entries[0].object_id) == {"0"}
            ):
                raise ContractError(
                    f"Effect runtime resource is not a real staged Git file entry: {path}"
                )
            staged_entries[path] = entries[0]

        attributes = _git_filter_attributes(root, expected_paths)
        for path in sorted(expected_paths):
            suffix = PurePosixPath(path).suffix.casefold()
            expected_filter = "lfs" if suffix == ".dds" else "unspecified"
            if attributes[path] != expected_filter:
                raise ContractError(
                    f"Effect runtime resource Git filter must be {expected_filter}: "
                    f"{path}: {attributes[path]}"
                )

        dds_object_ids = {
            staged_entries[path].object_id
            for path in expected_paths
            if PurePosixPath(path).suffix.casefold() == ".dds"
        }
        dds_blobs = _git_blob_payloads(root, dds_object_ids)
        for path in sorted(expected_paths):
            resource_id = expected_by_path[path]
            identity = identities[resource_id]
            entry = staged_entries[path]
            suffix = PurePosixPath(path).suffix.casefold()
            if suffix == ".dds":
                pointer_sha256, pointer_size = _parse_lfs_pointer(
                    dds_blobs[entry.object_id], path
                )
                if (
                    pointer_sha256 != identity.sha256
                    or pointer_size != identity.size
                ):
                    raise ContractError(
                        f"Effect DDS staged LFS pointer does not match working bytes: {path}"
                    )
            else:
                expected_oid = (
                    identity.git_blob_sha1
                    if len(entry.object_id) == 40
                    else identity.git_blob_sha256
                )
                if entry.object_id != expected_oid:
                    raise ContractError(
                        f"Effect WModel staged Git blob does not match working bytes: {path}"
                    )
    return len(resource_ids), resource_bytes, local_untracked_resource_count


def _require_stable_id(value: Any, field: str) -> str:
    if not isinstance(value, str) or STABLE_ID.fullmatch(value) is None:
        raise ContractError(f"{field} is not a stable Effect ID")
    return value


def _finite_number(value: Any, field: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ContractError(f"{field} must be a finite number")
    result = float(value)
    if not math.isfinite(result):
        raise ContractError(f"{field} must be a finite number")
    return result


def _finite_vec3(value: Any, field: str) -> None:
    if not isinstance(value, list) or len(value) != 3:
        raise ContractError(f"{field} must be a three-number array")
    for index, component in enumerate(value):
        _finite_number(component, f"{field}[{index}]")


def _validate_authored_material_color_space(source: dict[str, Any], relative: str) -> None:
    """Opt-in color decoding must not override source-owned material execution."""
    for index, element in enumerate(source.get("elements", [])):
        if not isinstance(element, dict):
            continue
        material = element.get("material", {})
        if not isinstance(material, dict) or "colorTexturesSRGB" not in material:
            continue
        owner = f"{relative}.elements[{index}].material.colorTexturesSRGB"
        enabled = material["colorTexturesSRGB"]
        if not isinstance(enabled, bool):
            raise ContractError(f"{owner} must be a boolean")
        if not enabled:
            continue
        source_profile = material.get("sourceProfile", {})
        execution = material.get("execution", {})
        source_recipe = element.get("sourceRecipe", {})
        if (
            material.get("templateId") != "effect.standard"
            or not isinstance(source_profile, dict)
            or not isinstance(execution, dict)
            or not isinstance(source_recipe, dict)
            or source_profile.get("enabled", False) is not False
            or source_recipe.get("enabled", False) is not False
            or execution.get("enabled", False) is not False
            or execution.get("failClosed", False) is not False
        ):
            raise ContractError(f"{owner} requires an ordinary authored standard material")


def _validate_attachment_orientations(source: dict[str, Any], relative: str) -> None:
    anchors: dict[str, tuple[Any, ...]] = {}
    for element in source.get("elements", []):
        if not isinstance(element, dict) or "actionCueAttachment" not in element:
            continue
        attachment = element["actionCueAttachment"]
        if not isinstance(attachment, dict):
            raise ContractError(f"actionCueAttachment must be an object: {relative}")
        orientation = attachment.get("orientation", "bone")
        if not isinstance(orientation, str) or orientation not in ("bone", "owner_yaw"):
            raise ContractError(f"attachment orientation must be bone or owner_yaw: {relative}")
        if orientation == "owner_yaw":
            recipe = element.get("sourceRecipe", {})
            if (attachment.get("enabled") is not True or
                    attachment.get("follow") is not True or
                    source.get("version", 13) not in SUPPORTED_PRODUCT_DOCUMENT_VERSIONS or
                    (isinstance(recipe, dict) and recipe.get("enabled", False)) or
                    element.get("runtimeCarrier")):
                raise ContractError(f"owner_yaw requires an enabled native follow attachment: {relative}")
        if attachment.get("enabled") is not True or attachment.get("follow") is not True:
            continue
        slot = attachment.get("runtimeAnchorSlotId")
        bone = attachment.get("runtimeBoneName")
        if any(not isinstance(value, str) or not value.strip() or len(value.encode("utf-8")) > 128
               for value in (slot, bone)):
            raise ContractError(f"follow attachment requires stable slot and bone names: {relative}")
        socket = attachment.get("socketLocalTransform")
        if not isinstance(socket, dict):
            raise ContractError(f"follow attachment requires a socket transform: {relative}")
        values: list[float] = []
        for field in ("position", "rotationDegrees", "scale"):
            vector = socket.get(field)
            if (not isinstance(vector, list) or len(vector) != 3 or
                    any(isinstance(value, bool) or not isinstance(value, (int, float)) or
                        not math.isfinite(value) or (field == "scale" and value <= 0)
                        for value in vector)):
                raise ContractError(f"follow attachment socket {field} is invalid: {relative}")
            values.extend(vector)
        descriptor = (bone, orientation, *values)
        previous = anchors.setdefault(slot, descriptor)
        if previous != descriptor:
            raise ContractError(f"follow anchor ID has conflicting bone, socket, or orientation: {relative}: {slot}")


def _validate_native_sprite_particle_options(source: dict[str, Any], relative: str) -> None:
    """Keep optional manual particle controls out of source-owned execution."""
    option_keys = {"drag", "rotationRangeDegrees", "spinRangeDegreesPerSecond", "subUVOverLife"}
    for index, element in enumerate(source.get("elements", [])):
        if not isinstance(element, dict):
            continue
        detail = element.get("detail", {})
        particle = detail.get("particle", {}) if isinstance(detail, dict) else {}
        if not isinstance(particle, dict):
            continue
        velocity = particle.get("initialVelocity", {})
        if not isinstance(velocity, dict):
            velocity = {}
        if not option_keys.intersection(particle) and "uniformSolidAngle" not in velocity:
            continue
        owner = f"{relative}.elements[{index}].detail.particle"

        def ordered_pair(key: str) -> list[float]:
            raw = particle.get(key, [0, 0])
            if not isinstance(raw, list) or len(raw) != 2:
                raise ContractError(f"{owner}.{key} must be a two-number range")
            values = [_finite_number(value, f"{owner}.{key}") for value in raw]
            if not -3600 <= values[0] <= values[1] <= 3600:
                raise ContractError(f"{owner}.{key} range is invalid")
            return values

        drag = _finite_number(particle.get("drag", 0), f"{owner}.drag")
        if not 0 <= drag <= 1000:
            raise ContractError(f"{owner}.drag must be within 0..1000")
        rotation = ordered_pair("rotationRangeDegrees")
        spin = ordered_pair("spinRangeDegreesPerSecond")
        life_uv = particle.get("subUVOverLife", False)
        solid_angle = velocity.get("uniformSolidAngle", False)
        if not isinstance(life_uv, bool) or not isinstance(solid_angle, bool):
            raise ContractError(f"{owner} life UV and solid-angle options must be booleans")
        if not (drag or any(rotation) or any(spin) or life_uv or solid_angle):
            continue
        material = element.get("material", {})
        if not isinstance(material, dict):
            raise ContractError(f"{owner} material must be an object")
        execution = material.get("execution", {})
        source_profile = material.get("sourceProfile", {})
        source_recipe = element.get("sourceRecipe", {})
        source_presentation = element.get("sourcePresentation", {})
        resources = element.get("resources", [])
        if (
            not all(isinstance(value, dict) for value in (
                execution, source_profile, source_recipe, source_presentation
            ))
            or not isinstance(resources, list)
            or not all(isinstance(binding, dict) for binding in resources)
        ):
            raise ContractError(f"{owner} material/source/resource contracts are malformed")
        source_node = element.get("sourceNode", "")
        if (
            element.get("kind") != "particle"
            or particle.get("billboard") is not True
            or element.get("renderer")
            or not isinstance(source_node, str)
            or (source_node and not source_node.startswith("authored-copy:"))
            or any(binding.get("slotId") == "meshModel" for binding in resources)
            or source_recipe.get("enabled", False)
            or source_presentation.get("enabled", False)
            or material.get("templateId", "effect.standard") != "effect.standard"
            or material.get("sourceMaterialPath", "")
            or source_profile.get("enabled", False)
            or execution.get("enabled", False)
            or execution.get("failClosed", False)
            or execution.get("authoringApproximate", False)
        ):
            raise ContractError(f"{owner} options require a direct-authored effect.standard sprite particle")
        if solid_angle:
            speed = velocity.get("speed", [0, 0])
            if (
                velocity.get("mode") != "cone"
                or not isinstance(speed, list)
                or len(speed) != 2
                or not 0 <= _finite_number(speed[0], f"{owner}.initialVelocity.speed")
                <= _finite_number(speed[1], f"{owner}.initialVelocity.speed")
                or not 0 <= _finite_number(
                    velocity.get("coneAngleDegrees", 0), f"{owner}.initialVelocity.coneAngleDegrees"
                ) <= 180
            ):
                raise ContractError(f"{owner} uniformSolidAngle requires a cone and nonnegative speed range")
        if life_uv:
            uv = detail.get("uv", {})
            if not isinstance(uv, dict):
                raise ContractError(f"{owner} life SubUV requires an object UV block")
            columns, rows = uv.get("tileColumns", 1), uv.get("tileRows", 1)
            if (
                type(columns) is not int or type(rows) is not int
                or columns <= 0 or rows <= 0 or not 1 < columns * rows <= 0xFFFFFFFF
                or uv.get("sequence", False) is not False
                or uv.get("loop", False) is not False
                or type(uv.get("tileIndex", 0)) is not int
                or uv.get("tileIndex", 0) != 0
            ):
                raise ContractError(f"{owner}.subUVOverLife requires an atlas, tileIndex 0, and no emitter UV sequence/loop")


def _validate_v15_runtime_extensions(source: dict[str, Any], relative: str) -> None:
    extensions = source.get("runtimeExtensions")
    if not isinstance(extensions, dict) or tuple(extensions.keys()) != RUNTIME_EXTENSION_KEYS:
        raise ContractError(f"v15 runtimeExtensions fields/order are invalid: {relative}")
    if extensions.get("formatVersion") != 1 or isinstance(
        extensions.get("formatVersion"), bool
    ):
        raise ContractError(f"v15 runtimeExtensions formatVersion is invalid: {relative}")

    histories_value = extensions.get("bakedEdgeHistories")
    if not isinstance(histories_value, list):
        raise ContractError(f"v15 bakedEdgeHistories must be an array: {relative}")
    histories: dict[str, dict[str, Any]] = {}
    total_samples = 0
    for history_index, history in enumerate(histories_value):
        label = f"{relative}.runtimeExtensions.bakedEdgeHistories[{history_index}]"
        if not isinstance(history, dict) or tuple(history.keys()) != BAKED_HISTORY_KEYS:
            raise ContractError(f"v15 baked history fields/order are invalid: {label}")
        history_id = _require_stable_id(history.get("historyId"), f"{label}.historyId")
        if history_id in histories:
            raise ContractError(f"duplicate v15 baked history ID: {history_id}")
        if history.get("coordinateBasis") != "UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS":
            raise ContractError(f"unsupported v15 baked history coordinate basis: {history_id}")
        source_end = _finite_number(
            history.get("sourceEndTimeSeconds"), f"{label}.sourceEndTimeSeconds"
        )
        clamp = _finite_number(
            history.get("playbackClampSeconds"), f"{label}.playbackClampSeconds"
        )
        samples = history.get("samples")
        if source_end <= 0 or clamp <= 0 or clamp > source_end or not isinstance(samples, list):
            raise ContractError(f"v15 baked history timing is invalid: {history_id}")
        if len(samples) < 2:
            raise ContractError(f"v15 baked history needs interpolation samples: {history_id}")
        previous_time = -math.inf
        for sample_index, sample in enumerate(samples):
            sample_label = f"{label}.samples[{sample_index}]"
            if not isinstance(sample, dict) or tuple(sample.keys()) != BAKED_SAMPLE_KEYS:
                raise ContractError(f"v15 baked sample fields/order are invalid: {sample_label}")
            sample_time = _finite_number(
                sample.get("relativeTimeSeconds"), f"{sample_label}.relativeTimeSeconds"
            )
            if sample_time < 0 or sample_time <= previous_time:
                raise ContractError(f"v15 baked sample times are not strictly increasing: {history_id}")
            previous_time = sample_time
            for vector_name in BAKED_SAMPLE_KEYS[1:]:
                _finite_vec3(sample.get(vector_name), f"{sample_label}.{vector_name}")
        if previous_time > source_end + 1.0e-4 or clamp >= previous_time:
            raise ContractError(f"v15 baked history clamp/sample closure is invalid: {history_id}")
        total_samples += len(samples)
        histories[history_id] = history
    if total_samples > 4096:
        raise ContractError(f"v15 runtime extension sample budget exceeded: {relative}")

    elements = source.get("elements")
    if not isinstance(elements, list):
        raise ContractError(f"v15 elements must be an array: {relative}")
    element_ids: set[str] = set()
    used_histories: set[str] = set()
    carrier_count = 0
    for element_index, element in enumerate(elements):
        if not isinstance(element, dict):
            raise ContractError(f"v15 element must be an object: {relative}[{element_index}]")
        element_id = _require_stable_id(
            element.get("id"), f"{relative}.elements[{element_index}].id"
        )
        if element_id in element_ids:
            raise ContractError(f"duplicate v15 element ID: {element_id}")
        element_ids.add(element_id)
        carrier = element.get("runtimeCarrier")
        if carrier is None:
            continue
        carrier_count += 1
        if not isinstance(carrier, dict):
            raise ContractError(f"v15 runtimeCarrier must be an object: {element_id}")
        kind = carrier.get("kind")
        expected_keys = CARRIER_KEYS.get(kind)
        if expected_keys is None or tuple(carrier.keys()) != expected_keys:
            raise ContractError(f"v15 runtimeCarrier fields/kind are invalid: {element_id}")
        if carrier.get("formatVersion") != 1 or isinstance(
            carrier.get("formatVersion"), bool
        ) or carrier.get("admission") != "bounded":
            raise ContractError(f"v15 runtimeCarrier admission is invalid: {element_id}")
        if not element.get("visible", True):
            raise ContractError(f"v15 runtimeCarrier target must be visible: {element_id}")

        source_recipe = element.get("sourceRecipe")
        source_recipe = source_recipe if isinstance(source_recipe, dict) else {}
        if kind == "cascadeRibbonV1":
            if element.get("kind") != "trail" or source_recipe.get("enabled") is not True:
                raise ContractError(f"v15 Cascade carrier target is invalid: {element_id}")
            stable_id = carrier.get("typeDataModuleStableId")
            if (
                not isinstance(stable_id, str)
                or not stable_id
                or len(stable_id) > 256
                or any(character.isspace() for character in stable_id)
            ):
                raise ContractError(
                    f"v15 Cascade TypeData stable ID is invalid: {element_id}"
                )
            modules = source_recipe.get("modules")
            matches = [
                module
                for module in modules if isinstance(module, dict)
                and module.get("stableId") == stable_id
                and "typedataribbon" in str(module.get("className", "")).casefold()
            ] if isinstance(modules, list) else []
            if len(matches) != 1:
                raise ContractError(f"v15 Cascade TypeData join is not unique: {element_id}")
        else:
            history_id = _require_stable_id(
                carrier.get("historyId"), f"{element_id}.runtimeCarrier.historyId"
            )
            if history_id not in histories:
                raise ContractError(f"v15 runtimeCarrier history is missing: {element_id}")
            used_histories.add(history_id)
            if source_recipe.get("enabled") is not False:
                raise ContractError(f"v15 baked carrier sourceRecipe must be disabled: {element_id}")
            if kind == "animationTrailBakedEdgeV1" and element.get("kind") != "trail":
                raise ContractError(f"v15 animation Trail target kind is invalid: {element_id}")
            if kind == "lightBakedEdgeAttachmentV1":
                light = element.get("detail", {}).get("light", {})
                if (
                    element.get("kind") != "light"
                    or carrier.get("edgeLane") != "firstEdge"
                    or not isinstance(light, dict)
                    or light.get("enabled") is not True
                ):
                    raise ContractError(f"v15 baked Light target is invalid: {element_id}")
    if carrier_count == 0:
        raise ContractError(f"v15 Product document has no runtime carriers: {relative}")
    if used_histories != set(histories):
        raise ContractError(f"v15 baked histories are orphaned: {relative}")


def _validate_active_consumer_guard(root: Path) -> None:
    catalog_source = root / "Client/Private/Effect_Catalog.cpp"
    try:
        catalog_text = catalog_source.read_text(encoding="utf-8-sig")
    except OSError as exc:
        raise ContractError(f"active Effect catalog source is unavailable: {exc}") from exc
    load_marker = "bool_t Client::CEffectCatalog::Load"
    next_marker = "bool_t Client::CEffectCatalog::Stage_DebugDirectAuthoredReplacement"
    begin = catalog_text.find(load_marker)
    end = catalog_text.find(next_marker, begin + len(load_marker))
    if begin < 0 or end < 0:
        raise ContractError("active Effect catalog load boundary could not be located")
    active_load = catalog_text[begin:end]

    guarded_files = {
        "Client/Private/Effect_Catalog.cpp::Load": active_load,
    }
    for relative in ("Client/Private/Effect_Tool.cpp",):
        path = root / relative
        try:
            guarded_files[relative] = path.read_text(encoding="utf-8-sig")
        except OSError as exc:
            raise ContractError(f"active Effect consumer is unavailable: {relative}: {exc}") from exc

    pipeline_root = root / "Tools/EffectPipeline"
    excluded_pipeline_files = {
        Path(__file__).name,
        "test_validate_effect_sources.py",
    }
    if pipeline_root.is_dir():
        for path in sorted(pipeline_root.iterdir()):
            if (
                not path.is_file()
                or path.suffix.lower() not in {".py", ".ps1"}
                or path.name in excluded_pipeline_files
            ):
                continue
            try:
                guarded_files[path.relative_to(root).as_posix()] = path.read_text(
                    encoding="utf-8-sig"
                )
            except OSError as exc:
                raise ContractError(
                    f"active Effect pipeline source is unavailable: {path}: {exc}"
                ) from exc

    forbidden = (
        "EffectCatalog.runtime.json",
        "Client/Bin/DataFiles/Effect",
        "Data/Effects/VisualPrograms",
        "Publish-Effects.ps1",
        "LOSTARK_EFFECT_RUNTIME_CATALOG_FIXTURE",
    )
    for owner, text in guarded_files.items():
        for token in forbidden:
            if token in text:
                raise ContractError(
                    f"active Effect consumer still depends on retired runtime input: "
                    f"{owner}: {token}"
                )


def _validate_player_skill_effect_references(
    root: Path, product_effect_ids: set[str]
) -> set[str]:
    path = root / PLAYER_SKILLS_PATH
    if not path.is_file():
        return set()
    document, _ = _read_json(path)
    skills = document.get("skills")
    if not isinstance(skills, list):
        raise ContractError("PlayerSkills.skills must be an array")

    references: set[str] = set()
    for index, skill in enumerate(skills):
        if not isinstance(skill, dict):
            raise ContractError(f"PlayerSkills row {index} must be an object")
        effect_id = skill.get("effectId")
        if not isinstance(effect_id, str):
            raise ContractError(f"PlayerSkills row {index}.effectId must be a string")
        if not effect_id:
            continue
        references.add(
            _require_stable_id(effect_id, f"PlayerSkills row {index}.effectId")
        )

    missing = sorted(references - product_effect_ids)
    if missing:
        raise ContractError(
            f"PlayerSkills Effect references are missing from catalog: {missing[:5]}"
        )
    return references


def _validate_declared_draft_effects(
    root: Path, product_effect_ids: set[str]
) -> tuple[set[str], bool]:
    path = root / VALTAN_DRAFT_PATH
    if not path.is_file():
        return set(), False
    document, _ = _read_json(path)
    if tuple(document.keys()) != (
        "schema",
        "formatVersion",
        "bossArchetypeId",
        "bindings",
    ):
        raise ContractError("Valtan draft Effect root fields/order are invalid")
    if (
        document.get("schema") != "lostark.valtan-pattern-authoring-effects"
        or document.get("formatVersion") != 1
        or document.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise ContractError("Valtan draft Effect document identity is invalid")
    bindings = document.get("bindings")
    if not isinstance(bindings, list):
        raise ContractError("Valtan draft Effect bindings must be an array")

    draft_ids: set[str] = set()
    pattern_ids: set[str] = set()
    for index, binding in enumerate(bindings):
        if not isinstance(binding, dict) or tuple(binding.keys()) != (
            "patternId",
            "effectAssetId",
            "authoringPath",
            "state",
        ):
            raise ContractError(f"Valtan draft Effect binding {index} is invalid")
        pattern_id = binding.get("patternId")
        if not isinstance(pattern_id, str) or not pattern_id:
            raise ContractError(f"Valtan draft Effect binding {index}.patternId is invalid")
        if pattern_id in pattern_ids:
            raise ContractError(f"duplicate Valtan draft pattern ID: {pattern_id}")
        pattern_ids.add(pattern_id)
        effect_id = _require_stable_id(
            binding.get("effectAssetId"),
            f"Valtan draft Effect binding {index}.effectAssetId",
        )
        if effect_id in draft_ids:
            raise ContractError(f"duplicate Valtan draft Effect ID: {effect_id}")
        draft_ids.add(effect_id)
        expected_path = f"Effects/Authored/{effect_id}.effect.json"
        if binding.get("authoringPath") != expected_path:
            raise ContractError(
                f"Valtan draft Effect path must be identity-derived: {effect_id}"
            )
        if binding.get("state") != "DRAFT_ATTACHED":
            raise ContractError(f"Valtan draft Effect state is invalid: {effect_id}")

    collisions = sorted(draft_ids & product_effect_ids)
    if collisions:
        raise ContractError(
            f"Valtan draft Effects also appear in Product catalog: {collisions[:5]}"
        )
    return draft_ids, True


def _read_effect_asset_payloads(path: Path, owner: str) -> set[str]:
    try:
        text = path.read_text(encoding="utf-8-sig")
    except (OSError, UnicodeError) as exc:
        raise ContractError(f"{owner} is unreadable: {path}: {exc}") from exc
    references: set[str] = set()
    for line_number, line in enumerate(text.splitlines(), start=1):
        if "effectref=asset" not in line:
            continue
        match = EFFECT_ASSET_PAYLOAD.search(line)
        if match is None:
            raise ContractError(
                f"{owner} asset Effect row has no parseable payload: line {line_number}"
            )
        references.add(
            _require_stable_id(match.group(1), f"{owner} line {line_number} payload")
        )
    return references


def _validate_product_effect_reachability(
    root: Path,
    product_effect_ids: set[str],
    player_skill_effect_ids: set[str],
) -> None:
    relative_paths = (
        *PLAYER_EFFECT_EVENT_PATHS,
        VALTAN_CUE_PATH,
        VALTAN_BOSS_CATALOG_PATH,
        VALTAN_V1_ALIAS_PATH,
    )
    existing = [relative for relative in relative_paths if (root / relative).is_file()]
    if not existing:
        return
    missing_contracts = [
        relative for relative in relative_paths if not (root / relative).is_file()
    ]
    if missing_contracts:
        raise ContractError(
            f"Effect reachability contracts are missing: {missing_contracts[:5]}"
        )

    reachable_ids = set(player_skill_effect_ids)
    for relative in PLAYER_EFFECT_EVENT_PATHS:
        reachable_ids.update(
            _read_effect_asset_payloads(root / relative, relative)
        )

    cue_document, _ = _read_json(root / VALTAN_CUE_PATH)
    cues = cue_document.get("cues")
    if not isinstance(cues, list):
        raise ContractError("Valtan pattern Effect cues must be an array")
    for index, cue in enumerate(cues):
        if not isinstance(cue, dict):
            raise ContractError(f"Valtan pattern Effect cue {index} must be an object")
        reachable_ids.add(
            _require_stable_id(
                cue.get("effectAssetId"),
                f"Valtan pattern Effect cue {index}.effectAssetId",
            )
        )

    boss_document, _ = _read_json(root / VALTAN_BOSS_CATALOG_PATH)
    bosses = boss_document.get("bosses")
    if not isinstance(bosses, list):
        raise ContractError("BossCatalog.bosses must be an array")
    valtan_rows = [
        boss for boss in bosses
        if isinstance(boss, dict) and boss.get("archetypeId") == "BOSS_VALTAN"
    ]
    if len(valtan_rows) != 1:
        raise ContractError("BossCatalog must contain exactly one BOSS_VALTAN row")
    visuals = valtan_rows[0].get("combatObjectVisuals")
    if not isinstance(visuals, list):
        raise ContractError("BOSS_VALTAN combatObjectVisuals must be an array")
    for index, visual in enumerate(visuals):
        if not isinstance(visual, dict):
            raise ContractError(f"BOSS_VALTAN combatObjectVisual {index} is invalid")
        reachable_ids.add(
            _require_stable_id(
                visual.get("effectAssetId"),
                f"BOSS_VALTAN combatObjectVisual {index}.effectAssetId",
            )
        )

    alias_document, _ = _read_json(root / VALTAN_V1_ALIAS_PATH)
    aliases = alias_document.get("aliases")
    if not isinstance(aliases, list):
        raise ContractError("Valtan V1 Effect aliases must be an array")
    for index, alias in enumerate(aliases):
        if not isinstance(alias, dict):
            raise ContractError(f"Valtan V1 Effect alias {index} is invalid")
        for field in ("effectAssetId", "v1EffectAssetId"):
            reachable_ids.add(
                _require_stable_id(
                    alias.get(field), f"Valtan V1 Effect alias {index}.{field}"
                )
            )

    missing_product = sorted(reachable_ids - product_effect_ids)
    if missing_product:
        raise ContractError(
            f"runtime Effect references are missing from catalog: {missing_product[:5]}"
        )
    unreachable_product = sorted(product_effect_ids - reachable_ids)
    if unreachable_product:
        raise ContractError(
            f"catalog Effect IDs have no runtime consumer: {unreachable_product[:5]}"
        )


def validate_repository(
    root: Path,
    *,
    resource_root: Path | None = None,
    allow_local_resources: bool = True,
) -> ValidationReport:
    root = root.resolve()
    data_root = root / "Data/Effects"
    catalog_path = data_root / "EffectCatalog.json"
    authored_root = data_root / "Authored"
    catalog, _ = _read_json(catalog_path)
    if tuple(catalog.keys()) != ("formatVersion", "effects"):
        raise ContractError("EffectCatalog root fields/order must be formatVersion, effects")
    if catalog["formatVersion"] != 1 or isinstance(catalog["formatVersion"], bool):
        raise ContractError("EffectCatalog formatVersion must be integer 1")
    effects = catalog["effects"]
    if not isinstance(effects, list):
        raise ContractError("EffectCatalog.effects must be an array")

    effect_ids: set[str] = set()
    authoring_paths: set[str] = set()
    resource_ids: set[str] = set()
    source_bytes = 0
    for index, row in enumerate(effects):
        if not isinstance(row, dict):
            continue
        payload_kind = row.get("payloadKind")
        if payload_kind is not None and payload_kind != DIRECT_KIND:
            raise ContractError(
                f"catalog row {index} uses retired Effect payloadKind: {payload_kind!r}"
            )
        if payload_kind != DIRECT_KIND:
            continue
        keys = tuple(row.keys())
        if keys not in ALLOWED_DIRECT_KEY_ORDERS:
            raise ContractError(f"direct row {index} has unsupported fields/order: {keys}")
        effect_id = _require_stable_id(row.get("effectAssetId"), f"direct row {index}.effectAssetId")
        if effect_id in effect_ids:
            raise ContractError(f"duplicate direct Effect ID: {effect_id}")
        effect_ids.add(effect_id)

        expected_relative = f"Effects/Authored/{effect_id}.effect.json"
        relative = row.get("authoringPath")
        if relative != expected_relative:
            raise ContractError(
                f"direct source path must be identity-derived: {effect_id}: {relative!r}"
            )
        if relative in authoring_paths:
            raise ContractError(f"duplicate direct authoring path: {relative}")
        authoring_paths.add(relative)

        source_path = root / "Data" / relative
        source, raw = _read_json(source_path)
        source_bytes += len(raw)
        if source.get("schema") != "lostark.effect-authoring":
            raise ContractError(f"direct source schema is invalid: {relative}")
        version = source.get("version")
        if version not in SUPPORTED_PRODUCT_DOCUMENT_VERSIONS or isinstance(version, bool):
            raise ContractError(
                f"direct source must be editable authoring v13 or typed-extension v15: {relative}"
            )
        if source.get("effectAssetId") != effect_id:
            raise ContractError(f"direct source Effect ID mismatches its catalog row: {relative}")
        if not isinstance(source.get("elements"), list):
            raise ContractError(f"direct source elements must be an array: {relative}")
        _validate_authored_material_color_space(source, relative)
        _validate_native_sprite_particle_options(source, relative)
        _validate_attachment_orientations(source, relative)
        resource_ids.update(_collect_runtime_resource_ids(source, relative))
        if version == 15:
            _validate_v15_runtime_extensions(source, relative)
        elif "runtimeExtensions" in source or any(
            isinstance(element, dict) and "runtimeCarrier" in element
            for element in source["elements"]
        ):
            raise ContractError(
                f"v13 source cannot carry v15 runtime extensions: {relative}"
            )

        if keys in (AUDITION_KEYS, AUDITION_OVERLAY_KEYS):
            if row.get("runtimeAdmission") != "REGISTRY_BOUND_AUDITION_ONLY":
                raise ContractError(f"audition row admission is invalid: {effect_id}")
            if row.get("fidelityClass") != "PROJECT_TUNED_APPROX":
                raise ContractError(f"audition row fidelity is invalid: {effect_id}")
            source_effect_id = _require_stable_id(
                row.get("sourceEffectAssetId"),
                f"direct row {index}.sourceEffectAssetId",
            )
            if source_effect_id == effect_id:
                raise ContractError(f"audition row cannot source itself: {effect_id}")
            if not isinstance(row.get("sourceDocumentRawSha256"), str) or SHA256.fullmatch(
                row["sourceDocumentRawSha256"]
            ) is None:
                raise ContractError(f"audition row source hash is invalid: {effect_id}")

        if keys in (OVERLAY_KEYS, AUDITION_OVERLAY_KEYS):
            expected_overlay = f"Effects/ScreenOverlays/{effect_id}.screen-overlay.json"
            if row.get("screenOverlayPresentationPath") != expected_overlay:
                raise ContractError(f"screen-overlay path is not identity-derived: {effect_id}")
            overlay, overlay_raw = _read_json(root / "Data" / expected_overlay)
            source_bytes += len(overlay_raw)
            if (
                overlay.get("schema") != "lostark.effect-screen-overlay"
                or overlay.get("formatVersion") != 1
                or overlay.get("presentationId") != f"{effect_id}.screen-overlay"
            ):
                raise ContractError(f"screen-overlay identity is invalid: {effect_id}")
            resource_ids.update(
                _collect_runtime_resource_ids(overlay, expected_overlay)
            )

    if not effect_ids:
        raise ContractError("EffectCatalog has no direct-authored Product rows")

    player_skill_effect_ids = _validate_player_skill_effect_references(
        root, effect_ids
    )

    authored_ids = {
        path.name[: -len(".effect.json")]
        for path in authored_root.glob("*.effect.json")
        if path.is_file()
    }
    missing = sorted(effect_ids - authored_ids)
    if missing:
        raise ContractError(f"catalog direct source files are missing: {missing[:5]}")
    draft_effect_ids, draft_contract_present = _validate_declared_draft_effects(
        root, effect_ids
    )
    if draft_contract_present:
        missing_drafts = sorted(draft_effect_ids - authored_ids)
        if missing_drafts:
            raise ContractError(
                f"declared Valtan draft Effect files are missing: {missing_drafts[:5]}"
            )
        orphaned = sorted(authored_ids - effect_ids - draft_effect_ids)
        if orphaned:
            raise ContractError(
                f"Authored Effect files have no Product or draft owner: {orphaned[:5]}"
            )

    _validate_product_effect_reachability(
        root, effect_ids, player_skill_effect_ids
    )

    retired_paths = (
        root / "Client/Bin/DataFiles/Effect",
        root / "Data/Effects/VisualPrograms",
        root / "Tools/EffectPipeline/Publish-Effects.ps1",
        root / "Tools/EffectPipeline/Publish-SelectedDirectAuthoredEffects.ps1",
        root / "Tools/EffectPipeline/Test-PublishSelectedDirectAuthoredEffects.ps1",
    )
    present_retired = [
        path.relative_to(root).as_posix()
        for path in retired_paths
        if path.is_file()
        or (path.is_dir() and any(child.is_file() for child in path.rglob("*")))
    ]
    if present_retired:
        raise ContractError(
            "retired Effect publish artifact/tool still exists: " + ", ".join(present_retired)
        )

    _validate_active_consumer_guard(root)
    resource_file_count, resource_bytes, local_count = _validate_runtime_resource_closure(
        root,
        resource_ids,
        resources_root=resource_root,
        allow_local_resources=allow_local_resources,
    )
    return ValidationReport(
        direct_source_count=len(effect_ids),
        unbound_reference_count=len(authored_ids - effect_ids),
        source_bytes=source_bytes,
        resource_file_count=resource_file_count,
        resource_bytes=resource_bytes,
        allow_local_resources=allow_local_resources,
        local_untracked_resource_count=local_count,
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument(
        "--allow-local-resources",
        action="store_true",
        default=True,
        help=(
            "Compatibility flag. Drive-owned local Resources are accepted by "
            "default; tracked identity checks remain enabled when a referenced "
            "file is present in the Git index."
        ),
    )
    parser.add_argument(
        "--resource-root",
        type=Path,
        help="Optional physical Client/Bin/Resources root, used by lightweight Git worktrees without duplicated binary assets.",
    )
    args = parser.parse_args(argv)
    try:
        report = validate_repository(
            args.repository_root,
            resource_root=args.resource_root,
            allow_local_resources=args.allow_local_resources,
        )
    except ContractError as exc:
        print(f"Effect source validation failed: {exc}", file=sys.stderr)
        return 1
    print(report.as_json())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
