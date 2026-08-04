#!/usr/bin/env python3
"""Create a converter selection catalog from an effect intake manifest."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--family", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    source = json.loads(args.manifest.read_text(encoding="utf-8"))
    rows = []
    seen: set[str] = set()
    for candidate in source.get("particleSystems", []):
        name = str(candidate.get("objectName") or "")
        if not name or name.casefold() in seen:
            continue
        seen.add(name.casefold())
        rows.append(
            {
                "family": args.family,
                "object_name": name,
                "recipe_file": candidate.get("recipeFile"),
                "is_old": bool(candidate.get("isOld")),
                "is_test": bool(candidate.get("isTest")),
                "is_camera": bool(candidate.get("isCamera")),
            }
        )
    rows.sort(key=lambda row: row["object_name"].casefold())
    result = {
        "schemaVersion": 1,
        "sourceManifest": str(args.manifest),
        "family": args.family,
        "count": len(rows),
        "rows": rows,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps({"output": str(args.output), "count": len(rows)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
