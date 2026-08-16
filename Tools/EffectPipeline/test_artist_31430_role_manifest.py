from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import unittest


ROOT = Path(__file__).resolve().parents[2]
DOCUMENT_PATH = ROOT / "Data/Effects/Authored/effect.artist.skill.31430.unified.effect.json"
MANIFEST_PATH = (
    ROOT
    / "Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31430.role-manifest.json"
)
SOURCE_RECEIPT_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/CurrentCombat/skill.31430.source-receipt.json"
)
NORMALIZED_GRAPH_PATH = (
    ROOT
    / "Data/Effects/Imported/Artist/CurrentCombat/Graphs/skill.31430.normalized-effect-graph.json"
)
RESOURCES_ROOT = ROOT / "Client/Bin/Resources"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def normalized_source_emitter_path(element: dict) -> str:
    source_node = element["sourceNode"]
    marker = "|element:"
    if marker not in source_node:
        raise AssertionError(f"missing source emitter identity: {element['id']}")
    return re.sub(
        r"\.event_source-event-\d+$", "", source_node.split(marker, 1)[1]
    ).casefold()


def source_event_id(element: dict) -> str:
    presentation = element.get("sourcePresentation") or {}
    event_id = presentation.get("sourceEventId")
    if not isinstance(event_id, str) or not event_id:
        raise AssertionError(f"missing source event identity: {element['id']}")
    return event_id


def renderer_payload(element: dict) -> str:
    payload = copy.deepcopy(element)
    for field in (
        "id",
        "displayName",
        "sourceNode",
        "sourcePresentation",
        "visible",
    ):
        payload.pop(field, None)
    return json.dumps(payload, sort_keys=True, separators=(",", ":"))


def screen_alignment(element: dict) -> str:
    required = next(
        module
        for module in element["sourceRecipe"]["modules"]
        if module["className"] == "particlemodulerequired"
    )
    for literal in required.get("literals") or []:
        if literal.get("propertyPath") == "screenalignment":
            return str(literal["value"])
    return "camera-facing"


def texture_asset_ids(element: dict) -> list[str]:
    result = {
        binding["assetId"]
        for binding in element.get("resources") or []
        if binding.get("assetId")
    }
    source_profile = (element.get("material") or {}).get("sourceProfile") or {}
    result.update(
        texture["assetId"]
        for texture in source_profile.get("textures") or []
        if texture.get("assetId")
    )
    return sorted(result)


def particle_window(element: dict) -> tuple[float, float]:
    detail = element["detail"]
    recipe = element["sourceRecipe"]
    start = float(detail["timing"]["startDelaySeconds"]) + float(
        recipe["emitterDelaySeconds"]
    )
    emitter_duration = float(recipe["emitterDurationSeconds"])
    if emitter_duration > 0.0:
        emission = emitter_duration * max(int(recipe["emitterLoopCount"]), 1)
    else:
        emission = float(detail["timing"]["lifeTimeSeconds"])
    particle_tail = max(float(value) for value in detail["particle"]["lifeTimeSeconds"])
    return start, start + emission + particle_tail


def peak_particle_cap(elements: list[dict]) -> int:
    boundaries = sorted({value for row in elements for value in particle_window(row)})
    return max(
        sum(
            int(row["detail"]["particle"]["maxParticles"])
            for row in elements
            if particle_window(row)[0] <= boundary < particle_window(row)[1]
        )
        for boundary in boundaries
    )


def burst_maximum_total(elements: list[dict]) -> int:
    return sum(
        int(burst["countMaximum"])
        for row in elements
        for burst in row["sourceRecipe"].get("bursts") or []
    )


class Artist31430RoleManifestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = load_json(DOCUMENT_PATH)
        cls.manifest = load_json(MANIFEST_PATH)
        cls.source_receipt = load_json(SOURCE_RECEIPT_PATH)
        cls.normalized_graph = load_json(NORMALIZED_GRAPH_PATH)
        cls.elements_by_id = {row["id"]: row for row in cls.document["elements"]}
        cls.events_by_id = {
            row["eventId"]: row for row in cls.source_receipt["timeline"]["events"]
        }
        cls.graph_nodes_by_path = {
            f"{row['package']}.{row['objectPath']}".casefold(): row
            for row in cls.normalized_graph["nodes"]
        }

    def test_manifest_covers_exact_duplicate_source_emitters(self) -> None:
        self.assertEqual(self.manifest["schema"], "lostark.effect-role-manifest")
        self.assertEqual(self.manifest["version"], 1)
        self.assertEqual(
            self.manifest["effectAssetId"], self.document["effectAssetId"]
        )
        evidence = self.manifest["sourceEvidence"]
        self.assertEqual(evidence["sourceReceiptSha256"], sha256_file(SOURCE_RECEIPT_PATH))
        self.assertEqual(
            evidence["normalizedGraphSha256"], sha256_file(NORMALIZED_GRAPH_PATH)
        )

        entries = self.manifest["entries"]
        self.assertEqual(len(entries), 22)
        paths = [row["sourceEmitterPath"] for row in entries]
        self.assertEqual(paths, sorted(paths))
        self.assertEqual(len(set(paths)), 22)

        all_manifest_ids: set[str] = set()
        for entry in entries:
            canonical_id = entry["canonicalElementId"]
            duplicate_id = entry["duplicateElementId"]
            self.assertNotIn(canonical_id, all_manifest_ids)
            all_manifest_ids.add(canonical_id)
            self.assertNotIn(duplicate_id, all_manifest_ids)
            all_manifest_ids.add(duplicate_id)

            canonical = self.elements_by_id[canonical_id]
            duplicate = self.elements_by_id[duplicate_id]
            expected_path = entry["sourceEmitterPath"].casefold()
            self.assertEqual(normalized_source_emitter_path(canonical), expected_path)
            self.assertEqual(normalized_source_emitter_path(duplicate), expected_path)
            self.assertEqual(renderer_payload(canonical), renderer_payload(duplicate))
            self.assertEqual(source_event_id(canonical), entry["canonicalEventId"])
            self.assertEqual(source_event_id(duplicate), entry["duplicateEventId"])

            canonical_event = self.events_by_id[entry["canonicalEventId"]]
            duplicate_event = self.events_by_id[entry["duplicateEventId"]]
            self.assertLess(
                int(entry["canonicalEventId"].rsplit("-", 1)[1]),
                int(entry["duplicateEventId"].rsplit("-", 1)[1]),
            )
            self.assertEqual(
                canonical_event["globalTimeSeconds"],
                duplicate_event["globalTimeSeconds"],
            )
            system_path = expected_path.rsplit(".", 1)[0]
            self.assertEqual(canonical_event["sourceSystemId"].casefold(), system_path)
            self.assertEqual(duplicate_event["sourceSystemId"].casefold(), system_path)

            graph_node = self.graph_nodes_by_path[expected_path]
            self.assertEqual(
                graph_node["properties"]["emittername"]["value"],
                entry["sourceEmitterName"],
            )
            self.assertEqual(canonical["sourceRecipe"]["rendererShape"], "sprite")
            self.assertEqual(
                canonical["material"]["sourceProfile"]["profileId"],
                entry["materialProfileId"],
            )
            self.assertEqual(
                canonical["detail"]["timing"]["startDelaySeconds"],
                entry["sourceStartSeconds"],
            )
            for actual, expected in zip(
                canonical["detail"]["particle"]["lifeTimeSeconds"],
                entry["particleLifetimeSeconds"],
                strict=True,
            ):
                self.assertAlmostEqual(actual, expected, places=7)
            for actual, expected in zip(
                canonical["detail"]["particle"]["startSize"],
                entry["startSize"],
                strict=True,
            ):
                self.assertAlmostEqual(actual, expected, places=7)
            self.assertEqual(screen_alignment(canonical), entry["screenAlignment"])

        self.assertEqual(len(all_manifest_ids), 44)

    def test_selected_visibility_and_companion_texture_lanes(self) -> None:
        entries = self.manifest["entries"]
        selected = [row for row in entries if row["decision"] == "SELECTED"]
        suppressed = [row for row in entries if row["decision"] == "SUPPRESSED"]
        self.assertEqual(len(selected), 2)
        self.assertEqual(len(suppressed), 20)
        self.assertEqual(
            sorted(row["role"] for row in selected),
            ["BRUSH_FORWARD_FAN", "GROUND_INK_DECAL_SPRITE"],
        )

        expected_visible = {row["canonicalElementId"] for row in selected}
        actual_visible = {
            row["id"] for row in self.document["elements"] if row["visible"] is True
        }
        self.assertEqual(actual_visible, expected_visible)
        for entry in entries:
            canonical = self.elements_by_id[entry["canonicalElementId"]]
            duplicate = self.elements_by_id[entry["duplicateElementId"]]
            self.assertEqual(canonical["visible"], entry["decision"] == "SELECTED")
            self.assertFalse(duplicate["visible"])

        full = 0
        approximate = 0
        for entry in selected:
            element = self.elements_by_id[entry["canonicalElementId"]]
            execution = (element.get("material") or {}).get("execution") or {}
            if execution.get("authoringApproximate") is True:
                approximate += 1
            else:
                full += 1
            self.assertEqual(
                texture_asset_ids(element), entry["requiredTextureAssetIds"]
            )
            for asset_id in entry["requiredTextureAssetIds"]:
                relative = PurePosixPath(asset_id)
                self.assertFalse(relative.is_absolute())
                self.assertNotIn("..", relative.parts)
                self.assertTrue(
                    (RESOURCES_ROOT.joinpath(*relative.parts)).is_file(), asset_id
                )

        self.assertEqual(full, self.manifest["target"]["fullVisibleElementCount"])
        self.assertEqual(
            approximate,
            self.manifest["target"]["authoringApproximateVisibleElementCount"],
        )
        self.assertEqual(
            len(self.document["elements"]),
            self.manifest["baseline"]["documentElementCount"],
        )

    def test_playback_budget_reduction_is_exact(self) -> None:
        historical_ids = {
            element_id
            for entry in self.manifest["entries"]
            for element_id in (
                entry["canonicalElementId"],
                entry["duplicateElementId"],
            )
        }
        historical = [self.elements_by_id[element_id] for element_id in historical_ids]
        selected = [row for row in self.document["elements"] if row["visible"] is True]

        baseline = self.manifest["baseline"]
        target = self.manifest["target"]
        self.assertEqual(len(historical), baseline["historicalVisibleElementCount"])
        self.assertEqual(peak_particle_cap(historical), baseline["peakParticleCap"])
        self.assertEqual(
            burst_maximum_total(historical), baseline["burstMaximumTotal"]
        )
        self.assertEqual(len(selected), target["visibleElementCount"])
        self.assertEqual(peak_particle_cap(selected), target["peakParticleCap"])
        self.assertEqual(burst_maximum_total(selected), target["burstMaximumTotal"])


if __name__ == "__main__":
    unittest.main()
