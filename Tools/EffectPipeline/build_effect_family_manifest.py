#!/usr/bin/env python3
"""Build the Effect parent-material family manifest.

A family is one UE3 parent Material. Every authored Element that names the same
parent shares that parent's formula, so a family implemented once serves every
skill and class that uses it. This manifest is the join table for that work: it
answers, per family, how many occurrences exist, which carriers and role sets
they use, and whether the extracted parent-material evidence needed to write the
formula is on disk.

Family identity and family admission are deliberately separate. Sharing a parent
makes two occurrences the same family; it does not make them promotable together.
A parent used by both Sprite and Mesh carriers, or bound with different named
texture roles, needs one admission per variant, and this manifest records the
variance rather than averaging it away.

Read-only with respect to the authored corpus. The only file written is the
manifest itself.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
AUTHORED_DIRECTORY = REPOSITORY_ROOT / "Data/Effects/Authored"
SOURCE_CATALOG = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
MANIFEST_RELATIVE_PATH = "Data/Effects/Contracts/effect-family-manifest.v1.json"
DEFAULT_EVIDENCE_ROOT = Path(
    r"C:\Users\user\Desktop\Resource_LostArk\05_Reports"
    r"\EffectExtraction\FourClassMaterials\export"
)

SCHEMA = "lostark.effect-family-manifest"
FORMAT_VERSION = 1
GROUPED_PROFILE = "effect.ue3.grouped-translucent.v1"
MESH_SLOT = "meshModel"


def canonical_json(value: Any) -> str:
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False
    )


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def parent_leaf(parent_material_path: str) -> str:
    """The props export is keyed by the object name, not the package path."""
    return parent_material_path.rsplit(".", 1)[-1].lower()


def character_class(effect_asset_id: str) -> str:
    parts = effect_asset_id.split(".")
    return parts[1] if len(parts) > 2 else "unknown"


def carrier_of(element: dict) -> str:
    slots = {
        resource.get("slotId") for resource in element.get("resources", [])
    }
    return "mesh" if MESH_SLOT in slots else "sprite"


def role_set_of(element: dict) -> tuple[str, ...]:
    return tuple(sorted(
        str(resource.get("slotId"))
        for resource in element.get("resources", [])
        if resource.get("slotId")
    ))


def index_evidence(evidence_root: Path) -> dict[str, Path]:
    """Map parent object name -> its extracted props file."""
    if not evidence_root.is_dir():
        return {}
    index: dict[str, Path] = {}
    for path in evidence_root.rglob("*.props.txt"):
        key = path.name[: -len(".props.txt")].lower()
        index.setdefault(key, path)
    return index


def parse_props(text: str) -> dict[str, Any]:
    """Extract the parent Material facts a family formula has to honour.

    Duplicate parameter names are kept. Some parents genuinely serialize the
    same name twice under different groups and collapsing them would hide a
    real permutation input.
    """
    parsed: dict[str, Any] = {
        "blendMode": None,
        "twoSided": None,
        "isMasked": None,
        "disableDepthTest": None,
        "expressionSlots": None,
        "staticSwitchCount": text.count(
            "MaterialExpressionStaticSwitchParameter"),
        "referencedTextureCount": None,
        "textureParameters": [],
        "scalarParameters": [],
        "vectorParameters": [],
    }
    scalar_fields = (
        ("BlendMode", "blendMode", str),
        ("TwoSided", "twoSided", bool),
        ("bIsMasked", "isMasked", bool),
        ("bDisableDepthTest", "disableDepthTest", bool),
    )
    for key, field, kind in scalar_fields:
        match = re.search(rf"^{key} = (.+)$", text, re.M)
        if match is None:
            continue
        raw = match.group(1).strip()
        parsed[field] = (raw == "true") if kind is bool else raw

    match = re.search(r"^Expressions\[(\d+)\]", text, re.M)
    if match is not None:
        parsed["expressionSlots"] = int(match.group(1))
    match = re.search(r"^ReferencedTextures\[(\d+)\]", text, re.M)
    if match is not None:
        parsed["referencedTextureCount"] = int(match.group(1))

    blocks = (
        ("CollectedTextureParameters", "textureParameters"),
        ("CollectedScalarParameters", "scalarParameters"),
        ("CollectedVectorParameters", "vectorParameters"),
    )
    for block, field in blocks:
        # The export mixes two layouts in one file: scalars are one line with
        # no spaces around '=', textures span lines with spaces. Matching only
        # brace-free bodies picks the innermost entries under either layout and
        # never the enclosing container.
        pattern = rf"{block}\[\d+\]\s*=\s*\{{([^{{}}]*)\}}"
        for match in re.finditer(pattern, text, re.S):
            body = match.group(1)
            name = re.search(r"Name\s*=\s*(\w+)", body)
            if name is None:
                continue
            row: dict[str, Any] = {"name": name.group(1)}
            group = re.search(r"Group\s*=\s*(\w+)", body)
            row["group"] = group.group(1) if group else None
            value = re.search(r"Value\s*=\s*(-?[\d.]+)", body)
            if value is not None:
                row["value"] = float(value.group(1))
            texture = re.search(r"Texture2D'([^']+)'", body)
            if texture is not None:
                row["texture"] = texture.group(1)
            parsed[field].append(row)
    return parsed


def classify_admission(family: dict) -> dict[str, Any]:
    """Separate 'same family' from 'promotable in one transaction'.

    Blockers are facts about the corpus, not opinions: each one names a split
    that a single family implementation cannot serve without choosing for the
    author.
    """
    blockers: list[str] = []
    if family["evidence"]["status"] != "PRESENT":
        blockers.append("PARENT_MATERIAL_EVIDENCE_ABSENT")
    if len(family["carriers"]) > 1:
        blockers.append("CARRIER_SPLIT_SPRITE_AND_MESH")
    if len(family["roleSets"]) > 1:
        blockers.append("NAMED_ROLE_SET_VARIANCE")
    if len(family["renderProfiles"]) > 1:
        blockers.append("RENDER_PROFILE_VARIANCE")

    if not blockers:
        status = "READY_FOR_SINGLE_FAMILY_IMPLEMENTATION"
    elif blockers == ["PARENT_MATERIAL_EVIDENCE_ABSENT"]:
        status = "EVIDENCE_REQUIRED"
    elif "PARENT_MATERIAL_EVIDENCE_ABSENT" in blockers:
        status = "EVIDENCE_REQUIRED_AND_VARIANT_SPLIT_REQUIRED"
    else:
        status = "VARIANT_SPLIT_REQUIRED"
    return {"status": status, "blockers": sorted(blockers)}


def build_manifest(
    repository_root: Path, evidence_root: Path
) -> dict[str, Any]:
    admitted = {
        row["effectAssetId"]
        for row in read_json(
            repository_root / SOURCE_CATALOG.relative_to(REPOSITORY_ROOT)
        )["effects"]
    }
    evidence_index = index_evidence(evidence_root)
    authored = repository_root / AUTHORED_DIRECTORY.relative_to(REPOSITORY_ROOT)

    accumulator: dict[str, dict[str, Any]] = {}
    document_count = 0
    element_count = 0
    profile_count = 0

    for path in sorted(authored.glob("*.effect.json")):
        document = read_json(path)
        effect_asset_id = document.get("effectAssetId", path.stem)
        if effect_asset_id not in admitted:
            continue
        document_count += 1
        klass = character_class(effect_asset_id)
        for element in document.get("elements", []):
            element_count += 1
            material = element.get("material") or {}
            profile = material.get("sourceProfile") or {}
            if not profile.get("enabled"):
                continue
            profile_count += 1
            parent = profile.get("parentMaterialPath") or ""
            if not parent:
                continue
            family = accumulator.setdefault(parent, {
                "occurrences": 0,
                "grouped": 0,
                "carriers": collections.Counter(),
                "classes": collections.Counter(),
                "runtimeShaderProfiles": collections.Counter(),
                "renderProfiles": collections.Counter(),
                "semanticStatus": collections.Counter(),
                "subUVModes": collections.Counter(),
                "roleSets": collections.Counter(),
                "childMaterials": set(),
                "skills": set(),
            })
            family["occurrences"] += 1
            runtime_profile = profile.get("runtimeShaderProfileId") or "(none)"
            if runtime_profile == GROUPED_PROFILE:
                family["grouped"] += 1
            family["carriers"][carrier_of(element)] += 1
            family["classes"][klass] += 1
            family["runtimeShaderProfiles"][runtime_profile] += 1
            family["renderProfiles"][material.get("renderProfile") or "(none)"] += 1
            family["semanticStatus"][profile.get("semanticStatus") or "(none)"] += 1
            family["subUVModes"][profile.get("subUVMode") or "(none)"] += 1
            family["roleSets"][role_set_of(element)] += 1
            family["childMaterials"].add(
                material.get("sourceMaterialPath") or "(none)")
            family["skills"].add(effect_asset_id)

    families = []
    for parent, row in accumulator.items():
        leaf = parent_leaf(parent)
        props_path = evidence_index.get(leaf)
        if props_path is None:
            evidence: dict[str, Any] = {"status": "ABSENT", "objectName": leaf}
        else:
            evidence = {"status": "PRESENT", "objectName": leaf}
            evidence.update(parse_props(
                props_path.read_text(encoding="utf-8", errors="replace")))

        family = {
            "familyId": "family-" + hashlib.sha256(
                parent.encode("utf-8")).hexdigest()[:16],
            "parentMaterialPath": parent,
            "occurrenceCount": row["occurrences"],
            "groupedOccurrenceCount": row["grouped"],
            "skillCount": len(row["skills"]),
            "classes": dict(sorted(row["classes"].items())),
            "carriers": dict(sorted(row["carriers"].items())),
            "runtimeShaderProfiles": dict(
                sorted(row["runtimeShaderProfiles"].items())),
            "renderProfiles": dict(sorted(row["renderProfiles"].items())),
            "semanticStatus": dict(sorted(row["semanticStatus"].items())),
            "subUVModes": dict(sorted(row["subUVModes"].items())),
            "roleSets": [
                {"roles": list(roles), "occurrenceCount": count}
                for roles, count in sorted(
                    row["roleSets"].items(), key=lambda kv: (-kv[1], kv[0]))
            ],
            "childMaterials": sorted(row["childMaterials"]),
            "skills": sorted(row["skills"]),
            "evidence": evidence,
        }
        family["admission"] = classify_admission(family)
        families.append(family)

    families.sort(
        key=lambda item: (-item["occurrenceCount"], item["parentMaterialPath"]))
    for family in families:
        family["rowSha256"] = canonical_sha256(family)

    grouped_total = sum(item["groupedOccurrenceCount"] for item in families)
    ranked = [item for item in families if item["groupedOccurrenceCount"] > 0]
    cumulative = []
    running = 0
    for index, item in enumerate(ranked, start=1):
        running += item["groupedOccurrenceCount"]
        cumulative.append({
            "familyRank": index,
            "cumulativeGroupedOccurrences": running,
            "coverage": round(running / grouped_total, 6)
            if grouped_total else 0.0,
        })

    admission_counts = collections.Counter(
        item["admission"]["status"] for item in families)
    blocker_counts = collections.Counter(
        blocker
        for item in families
        for blocker in item["admission"]["blockers"]
    )

    manifest = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "inputs": {
            "authoredDirectory": str(
                AUTHORED_DIRECTORY.relative_to(REPOSITORY_ROOT)).replace(
                    "\\", "/"),
            "sourceCatalog": str(
                SOURCE_CATALOG.relative_to(REPOSITORY_ROOT)).replace(
                    "\\", "/"),
            "parentMaterialEvidenceIndexed": len(evidence_index),
        },
        "summary": {
            "admittedDocumentCount": document_count,
            "elementCount": element_count,
            "enabledSourceProfileCount": profile_count,
            "familyCount": len(families),
            "groupedOccurrenceCount": grouped_total,
            "familiesWithGroupedOccurrences": len(ranked),
            "familiesMixingCarriers": sum(
                1 for item in families if len(item["carriers"]) > 1),
            "familiesUsedByMultipleClasses": sum(
                1 for item in families if len(item["classes"]) > 1),
            "familiesWithEvidence": sum(
                1 for item in families
                if item["evidence"]["status"] == "PRESENT"),
            "admissionStatusCounts": dict(sorted(admission_counts.items())),
            "blockerCounts": dict(sorted(blocker_counts.items())),
        },
        "groupedCoverageByFamilyRank": cumulative,
        "families": families,
    }
    manifest["artifactSha256"] = canonical_sha256(manifest)
    return manifest


def write_manifest(path: Path, manifest: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mode", choices=("build", "check"), default="build",
        help="build writes the manifest; check fails when it would change.")
    parser.add_argument(
        "--evidence-root", type=Path, default=DEFAULT_EVIDENCE_ROOT,
        help="Extracted parent-material props export root. Optional: when it "
             "is absent every family is reported as EVIDENCE_REQUIRED rather "
             "than failing.")
    parser.add_argument("--output", type=Path, default=None)
    arguments = parser.parse_args(argv)

    output = arguments.output or (REPOSITORY_ROOT / MANIFEST_RELATIVE_PATH)
    manifest = build_manifest(REPOSITORY_ROOT, arguments.evidence_root)

    if arguments.mode == "check":
        if not output.is_file():
            print(f"FAIL: manifest is missing: {output}")
            return 1
        existing = read_json(output)
        if existing.get("artifactSha256") != manifest["artifactSha256"]:
            print("FAIL: manifest is stale.")
            print(f"  on disk : {existing.get('artifactSha256')}")
            print(f"  rebuilt : {manifest['artifactSha256']}")
            return 1
        print(f"PASS: manifest matches the corpus "
              f"({manifest['summary']['familyCount']} families, "
              f"sha256={manifest['artifactSha256'][:16]}).")
        return 0

    write_manifest(output, manifest)
    summary = manifest["summary"]
    print(f"WROTE {output.relative_to(REPOSITORY_ROOT)}")
    print(f"  families                 {summary['familyCount']}")
    print(f"  grouped occurrences      {summary['groupedOccurrenceCount']}")
    print(f"  with parent evidence     {summary['familiesWithEvidence']}")
    print(f"  mixing sprite and mesh   {summary['familiesMixingCarriers']}")
    for status, count in summary["admissionStatusCounts"].items():
        print(f"  {status:<45} {count}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
