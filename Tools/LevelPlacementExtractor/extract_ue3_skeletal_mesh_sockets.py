#!/usr/bin/env python3
"""Extract a portable runtime socket contract from UModel props text."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
from pathlib import Path
from typing import Any


SOCKET_BLOCK = re.compile(
    r"Sockets\[(?P<index>\d+)\]\s*=\s*\{(?P<body>.*?)\n\s*\}",
    re.DOTALL,
)
COMPACT_SOCKET = re.compile(
    r"^\s*Sockets\[(?P<index>\d+)\]\s*=\s*\{\s*"
    r"Name=(?P<socket>[^,}\s]+)\s*,\s*Bone=(?P<bone>[^,}\s]+)\s*\}\s*$",
    re.MULTILINE,
)
NAME_FIELD = re.compile(r"^\s*(?P<name>SocketName|BoneName)\s*=\s*(?P<value>\S+)\s*$", re.MULTILINE)
VECTOR_FIELD = re.compile(
    r"^\s*(?P<name>RelativeLocation|RelativeRotation|RelativeScale)\s*=\s*"
    r"\{\s*(?P<values>[^}]*)\}\s*$",
    re.MULTILINE,
)
COMPONENT = re.compile(r"(?P<name>[A-Za-z]+)\s*=\s*(?P<value>[-+0-9.eE]+)")


def parse_vector(body: str, field: str) -> dict[str, float]:
    match = next(
        (row for row in VECTOR_FIELD.finditer(body) if row.group("name") == field),
        None,
    )
    if match is None:
        raise ValueError(f"socket has no {field}")
    result = {
        component.group("name").casefold(): float(component.group("value"))
        for component in COMPONENT.finditer(match.group("values"))
    }
    if not all(math.isfinite(value) for value in result.values()):
        raise ValueError(f"socket {field} contains a non-finite value")
    return result


def parse_socket_contract(path: Path) -> dict[str, Any]:
    raw = path.read_bytes()
    text = raw.decode("utf-8-sig")
    sockets = []
    for match in SOCKET_BLOCK.finditer(text):
        body = match.group("body")
        names = {
            field.group("name"): field.group("value")
            for field in NAME_FIELD.finditer(body)
        }
        # UModel's compact summary wraps all one-line entries in a
        # ``Sockets[count]`` block.  That outer array is not a socket.
        if "Sockets[" in body:
            continue
        if set(names) != {"SocketName", "BoneName"}:
            raise ValueError(
                f"socket {match.group('index')} identity is incomplete"
            )
        location = parse_vector(body, "RelativeLocation")
        rotation = parse_vector(body, "RelativeRotation")
        scale = parse_vector(body, "RelativeScale")
        sockets.append(
            {
                "sourceIndex": int(match.group("index")),
                "socketName": names["SocketName"],
                "boneName": names["BoneName"],
                "sourceTransform": {
                    "positionUeUnits": [
                        location.get("x", 0.0),
                        location.get("y", 0.0),
                        location.get("z", 0.0),
                    ],
                    "rotationUnrealUnits": [
                        rotation.get("pitch", 0.0),
                        rotation.get("yaw", 0.0),
                        rotation.get("roll", 0.0),
                    ],
                    "scale": [
                        scale.get("x", 1.0),
                        scale.get("y", 1.0),
                        scale.get("z", 1.0),
                    ],
                },
                "runtimeLocalTransform": {
                    "position": [
                        location.get("x", 0.0) * 0.01,
                        location.get("y", 0.0) * 0.01,
                        location.get("z", 0.0) * 0.01,
                    ],
                    "rotationDegrees": [
                        rotation.get("pitch", 0.0) * 360.0 / 65536.0,
                        rotation.get("yaw", 0.0) * 360.0 / 65536.0,
                        rotation.get("roll", 0.0) * 360.0 / 65536.0,
                    ],
                    "scale": [
                        scale.get("x", 1.0),
                        scale.get("y", 1.0),
                        scale.get("z", 1.0),
                    ],
                },
                "transformEvidence": "EXPLICIT_SOCKET_PROPERTIES",
            }
        )

    detailed_indices = {row["sourceIndex"] for row in sockets}
    for match in COMPACT_SOCKET.finditer(text):
        source_index = int(match.group("index"))
        if source_index in detailed_indices:
            continue
        sockets.append(
            {
                "sourceIndex": source_index,
                "socketName": match.group("socket"),
                "boneName": match.group("bone"),
                "sourceTransform": {
                    "positionUeUnits": [0.0, 0.0, 0.0],
                    "rotationUnrealUnits": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
                "runtimeLocalTransform": {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
                "transformEvidence": "UMODEL_COMPACT_DEFAULT_TRANSFORM",
            }
        )
    sockets.sort(key=lambda row: row["sourceIndex"])
    declared = re.search(r"^\s*Sockets\[(\d+)\]\s*=\s*$", text, re.MULTILINE)
    if declared is None or int(declared.group(1)) != len(sockets):
        raise ValueError(
            "socket array count does not match parsed socket entries"
        )
    folded_names = [row["socketName"].casefold() for row in sockets]
    if len(folded_names) != len(set(folded_names)):
        raise ValueError("socket names are not unique case-insensitively")
    return {
        "schema": "lostark.ue3-skeletal-mesh-sockets",
        "formatVersion": 1,
        "source": {
            "fileName": path.name,
            "sha256": hashlib.sha256(raw).hexdigest(),
            "positionUnitScale": 0.01,
            "rotationUnitScaleDegrees": 360.0 / 65536.0,
        },
        "sockets": sockets,
    }


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    contract = parse_socket_contract(args.input)
    write_json_atomic(args.output, contract)
    print(json.dumps({"socketCount": len(contract["sockets"]), "output": str(args.output)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
