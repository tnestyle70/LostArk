#!/usr/bin/env python3
"""Promote authored occurrences of one parent-material family to a typed profile.

A family implemented once should reach every occurrence that shares its parent,
but only where the occurrence actually matches what the family expects. This
tool decides that per occurrence and reports the reason it did not promote,
rather than promoting on the parent name alone.

The only field it ever writes is
``element.material.sourceProfile.runtimeShaderProfileId``. Visibility, timing,
transform, particle values, resources and every hand tuning stay exactly as the
author left them, so a promotion never silently reverts manual work.

Default mode is report. Nothing is written without ``--mode apply``.
"""

from __future__ import annotations

import argparse
import collections
import json
import sys
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
SOURCE_CATALOG = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
MANIFEST = REPOSITORY_ROOT / "Data/Effects/Contracts/effect-family-manifest.v1.json"
GROUPED_PROFILE = "effect.ue3.grouped-translucent.v1"
MESH_SLOT = "meshModel"


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def carrier_of(element: dict) -> str:
    slots = {r.get("slotId") for r in element.get("resources", [])}
    return "mesh" if MESH_SLOT in slots else "sprite"


def role_set_of(element: dict) -> tuple[str, ...]:
    return tuple(sorted(
        str(r.get("slotId")) for r in element.get("resources", [])
        if r.get("slotId")))


def evaluate(
    element: dict,
    parent: str,
    allowed_carriers: set[str],
    allowed_role_sets: set[tuple[str, ...]],
    allowed_render_profiles: set[str],
    target_profile: str,
) -> tuple[bool, list[str]]:
    """Why this occurrence may or may not take the family profile."""
    material = element.get("material") or {}
    profile = material.get("sourceProfile") or {}
    reasons: list[str] = []

    if not profile.get("enabled"):
        reasons.append("SOURCE_PROFILE_DISABLED")
    if (profile.get("parentMaterialPath") or "") != parent:
        reasons.append("PARENT_MISMATCH")
    current = profile.get("runtimeShaderProfileId") or ""
    if current == target_profile:
        reasons.append("ALREADY_PROMOTED")
    elif current != GROUPED_PROFILE:
        # Never overwrite an existing typed profile: that is authored work.
        reasons.append(f"NON_GROUPED_CURRENT_PROFILE:{current}")
    if allowed_carriers and carrier_of(element) not in allowed_carriers:
        reasons.append(f"CARRIER_NOT_ALLOWED:{carrier_of(element)}")
    if allowed_role_sets and role_set_of(element) not in allowed_role_sets:
        reasons.append(
            "ROLE_SET_NOT_ALLOWED:" + "+".join(role_set_of(element)))
    render_profile = material.get("renderProfile") or ""
    if allowed_render_profiles and render_profile not in allowed_render_profiles:
        reasons.append(f"RENDER_PROFILE_NOT_ALLOWED:{render_profile}")
    if material.get("execution", {}).get("enabled"):
        reasons.append("EXECUTION_PACKET_OWNS_THIS_ELEMENT")

    return (not reasons), reasons


def collect(
    parent: str,
    target_profile: str,
    allowed_carriers: set[str],
    allowed_role_sets: set[tuple[str, ...]],
    allowed_render_profiles: set[str],
    exclude_assets: set[str],
) -> dict[str, Any]:
    admitted = {row["effectAssetId"]
                for row in read_json(SOURCE_CATALOG)["effects"]}
    promote: list[dict[str, Any]] = []
    skip: list[dict[str, Any]] = []

    for path in sorted(AUTHORED_DIRECTORY.glob("*.effect.json")):
        document = read_json(path)
        asset_id = document.get("effectAssetId", path.stem)
        if asset_id not in admitted:
            continue
        for element in document.get("elements", []):
            profile = (element.get("material") or {}).get("sourceProfile") or {}
            if (profile.get("parentMaterialPath") or "") != parent:
                continue
            row = {
                "effectAssetId": asset_id,
                "documentFile": path.name,
                "elementId": element.get("id"),
                "displayName": element.get("displayName"),
                "visible": element.get("visible"),
                "carrier": carrier_of(element),
                "roleSet": list(role_set_of(element)),
                "renderProfile": (element.get("material") or {}).get(
                    "renderProfile"),
                "currentProfile": profile.get("runtimeShaderProfileId"),
            }
            if asset_id in exclude_assets:
                row["reasons"] = ["EXCLUDED_BY_REQUEST"]
                skip.append(row)
                continue
            ok, reasons = evaluate(
                element, parent, allowed_carriers, allowed_role_sets,
                allowed_render_profiles, target_profile)
            if ok:
                promote.append(row)
            else:
                row["reasons"] = reasons
                skip.append(row)
    return {"promote": promote, "skip": skip}


def apply_promotions(
    parent: str, target_profile: str, promote: list[dict[str, Any]]
) -> int:
    by_document: dict[str, set[str]] = collections.defaultdict(set)
    for row in promote:
        by_document[row["documentFile"]].add(row["elementId"])

    changed = 0
    for file_name, element_ids in sorted(by_document.items()):
        path = AUTHORED_DIRECTORY / file_name
        document = read_json(path)
        touched = 0
        for element in document.get("elements", []):
            if element.get("id") not in element_ids:
                continue
            profile = element["material"]["sourceProfile"]
            if (profile.get("parentMaterialPath") or "") != parent:
                continue
            profile["runtimeShaderProfileId"] = target_profile
            touched += 1
        if touched:
            path.write_text(
                json.dumps(document, indent=2, ensure_ascii=False) + "\n",
                encoding="utf-8")
            changed += touched
    return changed


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--parent", required=True,
                        help="Exact parentMaterialPath from the manifest.")
    parser.add_argument("--target-profile", required=True,
                        help="runtimeShaderProfileId the family implements.")
    parser.add_argument("--carrier", action="append", default=[],
                        choices=("sprite", "mesh"),
                        help="Carrier the family supports. Repeatable.")
    parser.add_argument("--role-set", action="append", default=[],
                        help="Allowed slot set, comma separated, e.g. 'base'.")
    parser.add_argument("--render-profile", action="append", default=[],
                        help="Allowed renderProfile. Repeatable.")
    parser.add_argument("--exclude-asset", action="append", default=[],
                        help="effectAssetId to leave untouched.")
    parser.add_argument("--mode", choices=("report", "apply"),
                        default="report")
    parser.add_argument("--report-out", type=Path, default=None)
    arguments = parser.parse_args(argv)

    allowed_role_sets = {
        tuple(sorted(part.strip() for part in entry.split(",") if part.strip()))
        for entry in arguments.role_set
    }
    result = collect(
        arguments.parent, arguments.target_profile,
        set(arguments.carrier), allowed_role_sets,
        set(arguments.render_profile), set(arguments.exclude_asset))

    promote, skip = result["promote"], result["skip"]
    print(f"parent          {arguments.parent}")
    print(f"target profile  {arguments.target_profile}")
    print(f"PROMOTE         {len(promote)}")
    print(f"SKIP            {len(skip)}")

    if promote:
        by_skill = collections.Counter(row["effectAssetId"] for row in promote)
        print(f"\n--- promote: {len(by_skill)} documents ---")
        for asset_id, count in sorted(by_skill.items()):
            print(f"  {count:>4}  {asset_id}")

    if skip:
        reasons = collections.Counter(
            reason for row in skip for reason in row["reasons"])
        print("\n--- skip reasons ---")
        for reason, count in reasons.most_common():
            print(f"  {count:>4}  {reason}")

    if arguments.report_out is not None:
        payload = {
            "schema": "lostark.effect-family-promotion-report",
            "formatVersion": 1,
            "parentMaterialPath": arguments.parent,
            "targetRuntimeShaderProfileId": arguments.target_profile,
            "allowedCarriers": sorted(arguments.carrier),
            "allowedRoleSets": [list(r) for r in sorted(allowed_role_sets)],
            "allowedRenderProfiles": sorted(arguments.render_profile),
            "promoteCount": len(promote),
            "skipCount": len(skip),
            "promote": promote,
            "skip": skip,
        }
        arguments.report_out.parent.mkdir(parents=True, exist_ok=True)
        arguments.report_out.write_text(
            json.dumps(payload, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8")
        print(f"\nreport -> {arguments.report_out}")

    if arguments.mode == "apply":
        if not promote:
            print("\nNothing to apply.")
            return 0
        changed = apply_promotions(
            arguments.parent, arguments.target_profile, promote)
        print(f"\nAPPLIED runtimeShaderProfileId on {changed} occurrences. "
              f"No other field was written.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
