"""LightResources 저작 JSON을 검증하고 runtime 파일 하나를 교체한다. Map 광원은 검증만 하며 게시는 Map 경로가 맡는다."""

from __future__ import annotations

# Validate/Publish와 source·destination·Area 인자를 받는다. 모듈 설명은 CLI 도움말로도 쓰인다.
import argparse
# 중복 key·비표준 숫자를 거부하며 UTF-8 광원 문서를 읽고 저장한다.
import json
import math
# 임시 파일의 fsync와 runtime 파일 교체를 수행한다.
import os
from pathlib import Path
import re
# Client가 사용하는 float32 정밀도로 수치 경계를 비교한다.
import struct
import sys
# 대상과 같은 폴더에 임시 파일을 만들어 교체 전에 다시 검증한다.
import tempfile
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
# 재사용 광원 정의의 저작 정본과 Client 게시 파일 경로다.
RESOURCE_SOURCE = Path("Data/Rendering/Authored/LightResources.json")
RESOURCE_RUNTIME = Path("Client/Bin/DataFiles/Rendering/LightResources.runtime.json")
# 재사용 정의는 local offset·anchor를, Map 배치는 world 위치·group·enabled를 소유한다.
RESOURCE_FIELDS = {
    "lightResourceId", "displayName", "kind", "defaultAnchorKind", "localOffset",
    "localRotationDegrees", "rangeMeters", "falloffExponent", "innerConeDegrees",
    "outerConeDegrees", "color", "brightness",
}
MAP_FIELDS = {
    "lightId", "displayName", "kind", "groupId", "enabled", "position",
    "rotationDegrees", "rangeMeters", "falloffExponent", "innerConeDegrees",
    "outerConeDegrees", "color", "brightness",
}
STABLE_ID = re.compile(r"[A-Za-z0-9_.-]{1,128}\Z")


class LightValidationError(ValueError):
    """Client 광원 reader가 수용할 수 없는 문서의 검증 실패다."""


def _unique_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise LightValidationError(f"Duplicate JSON object key: {key}")
        result[key] = value
    return result


def _reject_constant(value: str) -> None:
    raise LightValidationError(f"Non-standard JSON number: {value}")


# 최상위 형식 검증에 앞서 중첩 object까지 중복 key와 NaN·Infinity를 거부한다.
def parse_document(raw: bytes) -> dict[str, Any]:
    try:
        return json.loads(raw.decode("utf-8-sig"), object_pairs_hook=_unique_object,
                          parse_constant=_reject_constant)
    except (UnicodeError, json.JSONDecodeError) as error:
        raise LightValidationError(f"Invalid light JSON: {error}") from error


def _exact(value: Any, fields: set[str], context: str) -> None:
    if not isinstance(value, dict) or set(value) != fields:
        raise LightValidationError(f"{context} has missing or unexpected fields.")


def _number(value: Any, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise LightValidationError(f"{context} must be a JSON number.")
    try:
        number = float(value)
    except OverflowError as error:
        raise LightValidationError(f"{context} is outside the numeric range.") from error
    if not math.isfinite(number):
        raise LightValidationError(f"{context} must be finite.")
    return number


def _uint(value: Any, context: str) -> int:
    number = _number(value, context)
    if number < 1 or number > 4294967294 or not number.is_integer():
        raise LightValidationError(f"{context} must be an integer in [1, 4294967294].")
    return int(number)


def _float32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def _finite32(value: Any, minimum: float, maximum: float, context: str) -> float:
    # C++ 저장값의 경계가 다시 읽힐 수 있도록 값과 허용 범위를 모두 binary32로 줄여 비교한다.
    number = _number(value, context)
    try:
        narrowed = _float32(number)
    except OverflowError as error:
        raise LightValidationError(f"{context} does not fit binary32.") from error
    if not math.isfinite(narrowed) or not _float32(minimum) <= narrowed <= _float32(maximum):
        raise LightValidationError(f"{context} must be in [{minimum}, {maximum}].")
    return narrowed


def _vector(value: Any, count: int, minimum: float, maximum: float, context: str) -> list[float]:
    if not isinstance(value, list) or len(value) != count:
        raise LightValidationError(f"{context} must contain {count} numbers.")
    return [_finite32(component, minimum, maximum, f"{context}[{index}]")
            for index, component in enumerate(value)]


def _stable(value: Any, context: str) -> str:
    if not isinstance(value, str) or STABLE_ID.fullmatch(value) is None:
        raise LightValidationError(f"{context} must be a stable ASCII ID of 1 to 128 bytes.")
    return value


def _name(value: Any, context: str) -> None:
    if not isinstance(value, str):
        raise LightValidationError(f"{context} must be UTF-8 text.")
    try:
        length = len(value.encode("utf-8", errors="strict"))
    except UnicodeError as error:
        raise LightValidationError(f"{context} contains invalid Unicode.") from error
    if not 1 <= length <= 256:
        raise LightValidationError(f"{context} must contain 1 to 256 UTF-8 bytes.")


# 종류별 범위·원뿔·방향광 제약을 검사한다. range는 m, rotation과 cone은 degree다.
def _light_values(row: dict[str, Any], offset: str, rotation: str, context: str) -> None:
    kind = row["kind"]
    if kind not in ("POINT", "SPOT", "DIRECTIONAL"):
        raise LightValidationError(f"{context}.kind is unsupported.")
    position = _vector(row[offset], 3, -100000, 100000, f"{context}.{offset}")
    _vector(row[rotation], 3, -100000, 100000, f"{context}.{rotation}")
    _vector(row["color"], 4, 0, 1, f"{context}.color")
    _finite32(row["falloffExponent"], 0.01, 64, f"{context}.falloffExponent")
    _finite32(row["brightness"], 0, 64, f"{context}.brightness")
    radius = _finite32(row["rangeMeters"], 0 if kind == "DIRECTIONAL" else 0.01,
                       1000, f"{context}.rangeMeters")
    inner = _finite32(row["innerConeDegrees"], 0, 89.9, f"{context}.innerConeDegrees")
    outer = _finite32(row["outerConeDegrees"], 0, 89.9, f"{context}.outerConeDegrees")
    if kind == "SPOT":
        if not (0 <= inner <= outer and outer > 0):
            raise LightValidationError(f"{context} needs 0 <= inner cone <= outer cone and a positive outer cone.")
    elif inner != 0 or outer != 0:
        raise LightValidationError(f"{context} non-spot cone values must be zero.")
    if kind == "DIRECTIONAL" and (radius != 0 or any(position)):
        raise LightValidationError(f"{context} directional range and offset must be zero.")


# 재사용할 lightResourceId의 고유성·다음 ordinal·anchor와 광원 값을 검사한다.
def validate_resources(document: Any) -> dict[str, Any]:
    _exact(document, {"schema", "formatVersion", "revision", "nextLightResourceOrdinal", "lights"}, "root")
    if document["schema"] != "lostark.light-resources" or _number(document["formatVersion"], "formatVersion") != 1:
        raise LightValidationError("Unsupported light resource schema or version.")
    _uint(document["revision"], "revision")
    ordinal = _uint(document["nextLightResourceOrdinal"], "nextLightResourceOrdinal")
    if not isinstance(document["lights"], list) or len(document["lights"]) > 4096:
        raise LightValidationError("lights must contain 0 to 4096 resources.")
    ids: set[str] = set()
    for row in document["lights"]:
        _exact(row, RESOURCE_FIELDS, "light resource")
        identity = _stable(row["lightResourceId"], "lightResourceId")
        if identity in ids:
            raise LightValidationError(f"Duplicate light resource ID: {identity}")
        ids.add(identity)
        _name(row["displayName"], f"{identity}.displayName")
        if row["defaultAnchorKind"] not in ("MAP", "PLAYER", "BOSS"):
            raise LightValidationError(f"{identity}.defaultAnchorKind is unsupported.")
        if identity.startswith("light.runtime."):
            suffix = identity[len("light.runtime."):]
            if not re.fullmatch(r"[1-9][0-9]*", suffix) or int(suffix) >= ordinal:
                raise LightValidationError(f"{identity} exceeds nextLightResourceOrdinal.")
        _light_values(row, "localOffset", "localRotationDegrees", identity)
    return document


# 게시할 Area의 개별 광원을 검사한다. receiver·staticShadowChannel은 있을 때만 검사한다.
def validate_map_lights_v2(document: Any, area_id: str) -> dict[str, Any]:
    _exact(document, {"schema", "formatVersion", "areaId", "provenance", "nextLightOrdinal", "lights"}, "map light root")
    if document["schema"] != "lostark.map-light-presentation" or _number(document["formatVersion"], "formatVersion") != 2:
        raise LightValidationError("Unsupported map light schema or version.")
    if _stable(document["areaId"], "areaId") != area_id:
        raise LightValidationError("Map light areaId does not match the publishing Area.")
    if document["provenance"] != "PROJECT_AUTHORED":
        raise LightValidationError("Map light v2 provenance must be PROJECT_AUTHORED.")
    _uint(document["nextLightOrdinal"], "nextLightOrdinal")
    if not isinstance(document["lights"], list) or len(document["lights"]) > 512:
        raise LightValidationError("Map lights must contain 0 to 512 entries.")
    ids: set[str] = set()
    for row in document["lights"]:
        fields = MAP_FIELDS | ({"receiver"} if isinstance(row, dict) and "receiver" in row else set())
        if isinstance(row, dict) and "staticShadowChannel" in row:
            fields |= {"staticShadowChannel"}
            channel = row["staticShadowChannel"]
            if isinstance(channel, bool) or not isinstance(channel, (int, float)) or not math.isfinite(channel) or channel < 1 or channel > 15 or int(channel) != channel:
                raise LightValidationError("Static shadow light channel must be an integer in [1,15].")
        _exact(row, fields, "map light")
        if row.get("receiver", "ALL") not in ("ALL", "SOURCE_CHARACTER", "UNBAKED"):
            raise LightValidationError("Map light receiver must be ALL, SOURCE_CHARACTER or UNBAKED.")
        identity = _stable(row["lightId"], "lightId")
        if identity in ids:
            raise LightValidationError(f"Duplicate map light ID: {identity}")
        ids.add(identity)
        _name(row["displayName"], f"{identity}.displayName")
        _stable(row["groupId"], f"{identity}.groupId")
        if not isinstance(row["enabled"], bool):
            raise LightValidationError(f"{identity}.enabled must be a boolean.")
        _light_values(row, "position", "rotationDegrees", identity)
    return document


# 파싱·검증 → 임시 저장·재검증 → source 변경 검사 → 단일 runtime 교체 순서다.
def publish_resources(source: Path, destination: Path, *, publish: bool) -> dict[str, Any]:
    source = source.resolve()
    destination = destination.resolve()
    if source == destination:
        raise LightValidationError("Source and runtime destination must be different files.")
    original = source.read_bytes()
    document = validate_resources(parse_document(original))
    # Validate 모드는 읽기만 한다. Publish 실패 시 남은 임시 파일은 finally에서 정리한다.
    if publish:
        payload = (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")
        destination.parent.mkdir(parents=True, exist_ok=True)
        temporary: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(mode="wb", dir=destination.parent,
                                             prefix=f".{destination.name}.", suffix=".tmp", delete=False) as stream:
                temporary = Path(stream.name)
                stream.write(payload)
                stream.flush()
                os.fsync(stream.fileno())
            validate_resources(parse_document(temporary.read_bytes()))
            if source.read_bytes() != original:
                raise LightValidationError("Light source changed during publish; runtime preserved.")
            os.replace(temporary, destination)
        finally:
            if temporary is not None and temporary.exists():
                temporary.unlink()
    return {"mode": "Publish" if publish else "Validate", "revision": document["revision"],
            "lightCount": len(document["lights"]), "source": str(source), "destination": str(destination)}


def main(arguments: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("Validate", "Publish"), default="Validate")
    parser.add_argument("--source", type=Path, default=REPOSITORY_ROOT / RESOURCE_SOURCE)
    parser.add_argument("--destination", type=Path, default=REPOSITORY_ROOT / RESOURCE_RUNTIME)
    parser.add_argument("--map-lights-area", help="Validate a maplights v2 layer for this Area; never publish it separately.")
    options = parser.parse_args(arguments)
    try:
        if options.map_lights_area is not None:
            if options.mode != "Validate":
                raise LightValidationError("Map lights must be published through Publish-MapAuthoring.ps1.")
            document = validate_map_lights_v2(parse_document(options.source.read_bytes()), options.map_lights_area)
            result = {"mode": "Validate", "areaId": options.map_lights_area, "lightCount": len(document["lights"])}
        else:
            result = publish_resources(options.source, options.destination, publish=options.mode == "Publish")
        print(json.dumps(result, ensure_ascii=True))
        return 0
    except (OSError, ValueError) as error:
        print(f"Light validation/publish failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
