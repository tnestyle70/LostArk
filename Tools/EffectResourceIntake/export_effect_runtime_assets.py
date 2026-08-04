#!/usr/bin/env python3
"""Export exact effect textures/meshes and stage only runtime-ready payloads."""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path


TEXTURE_EXTENSIONS = (".dds", ".tga", ".png")


@dataclass(frozen=True)
class Target:
    kind: str
    logical_package: str
    object_name: str
    physical_package: str

    @property
    def key(self) -> str:
        return f"{self.kind}|{self.logical_package}|{self.object_name}|{self.physical_package}"

    @property
    def short_id(self) -> str:
        return hashlib.sha256(self.key.encode("utf-8")).hexdigest()[:16]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--raw-root", type=Path, required=True)
    parser.add_argument("--runtime-root", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--converter", type=Path, required=True)
    parser.add_argument("--workers", type=int, default=6)
    parser.add_argument("--timeout-seconds", type=int, default=300)
    return parser.parse_args()


def normalized_targets(document: dict) -> list[Target]:
    targets: dict[tuple[str, str, str], Target] = {}
    for section, kind in (("textures", "Texture2D"), ("meshes", "StaticMesh")):
        for row in document.get(section, []):
            if not row.get("resolved"):
                continue
            target = Target(
                kind=kind,
                logical_package=str(row["logicalPackage"]),
                object_name=str(row["objectName"]),
                physical_package=str(row["physicalPackage"]),
            )
            identity = (
                target.kind.casefold(),
                target.logical_package.casefold(),
                target.object_name.casefold(),
            )
            previous = targets.get(identity)
            if previous and previous.physical_package.casefold() != target.physical_package.casefold():
                raise ValueError(
                    "one logical object resolved to multiple physical packages: "
                    f"{previous.key} / {target.key}"
                )
            targets[identity] = target
    return sorted(
        targets.values(),
        key=lambda item: (item.kind, item.logical_package.casefold(), item.object_name.casefold()),
    )


def find_target_file(target: Target, output_root: Path) -> Path | None:
    if target.kind == "Texture2D":
        candidates = [
            path for path in output_root.rglob("*")
            if path.is_file()
            and path.stem.casefold() == target.object_name.casefold()
            and path.suffix.casefold() in TEXTURE_EXTENSIONS
            and path.parent.name.casefold() == "texture2d"
        ]
        candidates.sort(key=lambda path: (
            TEXTURE_EXTENSIONS.index(path.suffix.casefold()),
            0 if target.logical_package.casefold() in {
                part.casefold() for part in path.parts
            } else 1,
            len(path.parts),
        ))
        return candidates[0] if candidates else None
    candidates = [
        path for path in output_root.rglob("*.gltf")
        if path.stem.casefold() == target.object_name.casefold()
        and path.parent.name.casefold() in {"staticmesh", "staticmesh3"}
    ]
    candidates.sort(key=lambda path: (
        0 if target.logical_package.casefold() in {
            part.casefold() for part in path.parts
        } else 1,
        len(path.parts),
    ))
    return candidates[0] if candidates else None


def run_export(target: Target, args: argparse.Namespace) -> dict:
    target_root = args.raw_root / target.kind / target.short_id
    target_root.mkdir(parents=True, exist_ok=True)
    log_path = target_root / "umodel.log.txt"
    source = find_target_file(target, target_root)
    started = time.monotonic()
    command = [
        str(args.umodel), "-export", "-game=lostark", "-kr", "-nameresolve",
        "-dds", f"-path={args.package_root}", f"-out={target_root}",
        f"-obj={target.object_name}",
        str(args.package_root / target.physical_package),
    ]
    if target.kind == "StaticMesh":
        command.insert(5, "-gltf")
    exit_code = 0
    timed_out = False
    if source is None:
        try:
            completed = subprocess.run(
                command, capture_output=True, text=True, encoding="utf-8",
                errors="replace", timeout=args.timeout_seconds,
                creationflags=(subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0),
            )
            exit_code = completed.returncode
            log_path.write_text(completed.stdout + completed.stderr, encoding="utf-8")
        except subprocess.TimeoutExpired as error:
            timed_out = True
            exit_code = -1
            stdout = error.stdout.decode("utf-8", "replace") if isinstance(error.stdout, bytes) else (error.stdout or "")
            stderr = error.stderr.decode("utf-8", "replace") if isinstance(error.stderr, bytes) else (error.stderr or "")
            log_path.write_text(stdout + stderr, encoding="utf-8")
        source = find_target_file(target, target_root)
    return {
        "kind": target.kind,
        "logicalPackage": target.logical_package,
        "objectName": target.object_name,
        "physicalPackage": target.physical_package,
        "rawRoot": str(target_root),
        "sourceFile": str(source) if source else None,
        "exitCode": exit_code,
        "timedOut": timed_out,
        "success": exit_code == 0 and source is not None,
        "seconds": round(time.monotonic() - started, 3),
    }


def safe_segment(value: str) -> str:
    result = "".join("_" if character in '<>:"/\\|?*' else character for character in value)
    if not result or result in {".", ".."}:
        raise ValueError(f"invalid asset path segment: {value!r}")
    return result


def asset_id(destination: Path, runtime_root: Path) -> str:
    return destination.relative_to(runtime_root.parent.parent).as_posix()


def stage_texture(row: dict, runtime_root: Path) -> dict:
    source = Path(row["sourceFile"])
    destination = (
        runtime_root / "Textures" / safe_segment(row["logicalPackage"]).upper()
        / (safe_segment(row["objectName"]) + source.suffix.lower())
    )
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)
    row["runtimeFile"] = str(destination)
    row["assetId"] = asset_id(destination, runtime_root)
    return row


def cook_mesh(row: dict, runtime_root: Path, args: argparse.Namespace) -> dict:
    source = Path(row["sourceFile"])
    destination = runtime_root / "Meshes" / (safe_segment(row["objectName"]) + ".wmodel")
    destination.parent.mkdir(parents=True, exist_ok=True)
    completed = subprocess.run(
        [
            str(args.converter), str(source), "-o", str(destination),
            "--pretransform", "--no-auto-textures", "--scale", "100",
        ],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=args.timeout_seconds,
        creationflags=(subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0),
    )
    row["cookExitCode"] = completed.returncode
    row["cookOutput"] = completed.stdout + completed.stderr
    row["runtimeFile"] = str(destination) if destination.is_file() else None
    row["assetId"] = asset_id(destination, runtime_root) if destination.is_file() else None
    row["success"] = row["success"] and completed.returncode == 0 and destination.is_file()
    return row


def main() -> int:
    args = parse_args()
    for required in (args.manifest, args.umodel, args.package_root, args.converter):
        if not required.exists():
            raise SystemExit(f"required path does not exist: {required}")
    if args.workers < 1:
        raise SystemExit("--workers must be positive")
    targets = normalized_targets(json.loads(args.manifest.read_text(encoding="utf-8")))
    args.raw_root.mkdir(parents=True, exist_ok=True)
    args.runtime_root.mkdir(parents=True, exist_ok=True)
    rows: list[dict] = []
    print(f"exporting {len(targets)} exact effect objects with {args.workers} workers", flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(run_export, target, args): target for target in targets}
        for index, future in enumerate(concurrent.futures.as_completed(futures), 1):
            try:
                rows.append(future.result())
            except Exception as error:
                target = futures[future]
                rows.append({
                    "kind": target.kind,
                    "logicalPackage": target.logical_package,
                    "objectName": target.object_name,
                    "physicalPackage": target.physical_package,
                    "success": False,
                    "error": str(error),
                })
            if index % 25 == 0 or index == len(targets):
                failures = sum(not row.get("success", False) for row in rows)
                print(f"export {index}/{len(targets)}; failures={failures}", flush=True)
    successful = [row for row in rows if row.get("success")]
    textures = [
        stage_texture(row, args.runtime_root)
        for row in successful if row["kind"] == "Texture2D"
    ]
    meshes = [row for row in successful if row["kind"] == "StaticMesh"]
    cooked_meshes: list[dict] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(args.workers, 4)) as pool:
        futures = [pool.submit(cook_mesh, row, args.runtime_root, args) for row in meshes]
        for future in concurrent.futures.as_completed(futures):
            cooked_meshes.append(future.result())
    final_rows = sorted(
        [*textures, *cooked_meshes, *[row for row in rows if not row.get("success")]],
        key=lambda row: (
            row.get("kind", ""),
            row.get("logicalPackage", "").casefold(),
            row.get("objectName", "").casefold(),
        ),
    )
    failures = [row for row in final_rows if not row.get("success")]
    result = {
        "schemaVersion": 1,
        "manifest": str(args.manifest),
        "runtimeRoot": str(args.runtime_root),
        "summary": {
            "targetCount": len(targets),
            "textureCount": sum(row.get("success", False) and row.get("kind") == "Texture2D" for row in final_rows),
            "meshCount": sum(row.get("success", False) and row.get("kind") == "StaticMesh" for row in final_rows),
            "failureCount": len(failures),
        },
        "rows": final_rows,
    }
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result["summary"]), flush=True)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
