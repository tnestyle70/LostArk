#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import materialize_valtan_carrier_v1 as materializer
import valtan_carrier_v1_successor_lineage as lineage


def _successor_live_owner_rows(
    historical_rows: list[dict],
    receipt: dict,
    projection_name: str,
) -> list[dict]:
    """Reverse only the sealed owner transfers for successor-layer testing."""

    rows = copy.deepcopy(historical_rows)
    projection = receipt["historicalOutputProjection"][projection_name]
    owner_fields = ("patternId", "semanticStageId", "gameplayActionId")
    for transfer in projection["ownerTransfers"]:
        historical_owner = tuple(
            transfer["historicalOwner"][field] for field in owner_fields
        )
        live_owner = tuple(
            transfer["liveOwner"][field] for field in owner_fields
        )
        matches = [
            row
            for row in rows
            if tuple(row[field] for field in owner_fields)
            == historical_owner
        ]
        live_action = transfer["liveOwner"]["gameplayActionId"]
        split_clip = {
            "valtan.attack.triple-slash.active": (
                "valtan.attack.four-slash.active.clip.01"
            ),
            "valtan.attack.rotation-slash.active": (
                "valtan.attack.four-slash.active.clip.02"
            ),
        }.get(live_action)
        if split_clip is not None:
            matches = [
                row for row in matches
                if row.get("clipOccurrenceId") == split_clip
            ]
        if len(matches) != transfer["rowCount"]:
            raise AssertionError(
                "successor owner transfer denominator drifted: "
                + transfer["transferId"]
            )
        for row in matches:
            for field, value in zip(owner_fields, live_owner, strict=True):
                row[field] = value
    return rows


class ValtanCarrierV1SuccessorLineageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.historical = lineage.read_json(materializer.RECEIPT_PATH)
        cls.successor = lineage.read_json(materializer.SUCCESSOR_RECEIPT_PATH)
        cls.catalog = lineage.read_json(materializer.CATALOG_PATH)
        cls.cues = lineage.read_json(materializer.CUE_PATH)
        cls.bindings = lineage.read_json(
            materializer.source_inventory.PATTERN_BINDINGS_PATH
        )
        cls.encounter = lineage.read_json(
            materializer.source_inventory.ENCOUNTER_PATH
        )
        cls.selection = lineage.read_json(
            materializer.reviewed_candidates.SELECTION_PATH
        )
        (
            cls.base_bindings,
            cls.base_encounter,
            cls.base_selection,
            cls.additive_input_evidence,
        ) = materializer.project_successor_base_inputs(
            cls.successor,
            cls.bindings,
            cls.encounter,
            cls.selection,
        )
        cls.base_catalog, cls.base_cues = (
            materializer._validate_successor_base_product(
                cls.successor, cls.catalog, cls.cues
            )
        )
        cls.blockers = {
            "reviewedProjectionLedger": _successor_live_owner_rows(
                cls.historical["reviewedProjectionLedger"],
                cls.successor,
                "reviewedProjectionLedger",
            ),
            "reviewedSourceOnlyOccurrences": _successor_live_owner_rows(
                cls.historical["reviewedSourceOnlyOccurrences"],
                cls.successor,
                "reviewedSourceOnlyOccurrences",
            ),
        }
        cls.state, cls.writes, _ = materializer.build_outputs()

    def test_historical_receipt_is_immutable_and_live_successors_validate(self) -> None:
        self.assertEqual(
            "92a102cb0b25d0d669014fdce0c4a4e08912b68530960b533769d7d10442cc99",
            lineage.canonical_sha256(self.historical),
        )
        lineage.validate_receipt(
            root=materializer.ROOT,
            receipt=self.successor,
            historical_receipt=self.historical,
            catalog=self.base_catalog,
            cues=self.base_cues,
        )
        expected_counts = {
            "effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01": 24,
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-01": 7,
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-02": 2,
            "effect.valtan.carrier-v1.attack.high-jump.land.clip-01": 24,
            "effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01": 3,
            "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01": 3,
            "effect.valtan.sky-axe.active": 6,
        }
        self.assertEqual(expected_counts, {
            effect_id: entry["finalDocument"]["elementCount"]
            for effect_id, entry in lineage.successor_documents(
                self.successor
            ).items()
        })

    def test_historical_input_and_output_projections_are_exact(self) -> None:
        projected_bindings = lineage.project_historical_pattern_bindings(
            self.base_bindings, self.successor
        )
        projected_encounter = lineage.project_historical_encounter(
            self.base_encounter, self.successor
        )
        projected_selection = lineage.project_historical_selection_manifest(
            self.base_selection, self.successor
        )
        projected_catalog = lineage.project_historical_catalog(
            self.base_catalog, self.successor
        )
        projected_cues = lineage.project_historical_cues(
            self.base_cues, self.successor
        )
        self.assertEqual(131, len(projected_bindings["bindings"]))
        self.assertEqual(137, sum(
            len(row["clips"]) for row in projected_bindings["bindings"]
        ))
        self.assertEqual(33, len(projected_encounter["patterns"]))
        self.assertEqual(24, len(projected_selection["selections"]))
        self.assertEqual(46, len(projected_catalog["effects"]))
        self.assertEqual(44, len(projected_cues["cues"]))
        self.assertEqual(
            self.historical["reviewedProjectionLedger"],
            lineage.project_historical_reviewed_projection_ledger(
                self.blockers["reviewedProjectionLedger"], self.successor
            ),
        )
        self.assertEqual(
            self.historical["reviewedSourceOnlyOccurrences"],
            lineage.project_historical_reviewed_source_only_occurrences(
                self.blockers["reviewedSourceOnlyOccurrences"],
                self.successor,
            ),
        )
        self.assertEqual(197, len(self.blockers["reviewedSourceOnlyOccurrences"]))

    def test_check_preserves_every_registered_successor_document(self) -> None:
        self.assertEqual("APPLIED", self.state)
        self.assertEqual({}, materializer._changed_outputs(self.writes))
        successor_paths = {
            (materializer.ROOT / entry["path"]).resolve()
            for entry in lineage.successor_documents(
                self.successor
            ).values()
        }
        self.assertTrue(successor_paths.isdisjoint(
            {path.resolve() for path in self.writes}
        ))

    def test_historical_receipt_drift_fails_closed(self) -> None:
        drifted = copy.deepcopy(self.historical)
        drifted["summary"]["materializedProjectionCount"] -= 1
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError,
            "immutable historical Carrier V1 receipt drifted",
        ):
            lineage.validate_receipt(
                root=materializer.ROOT,
                receipt=self.successor,
                historical_receipt=drifted,
                catalog=self.base_catalog,
                cues=self.base_cues,
            )

        summary_drift = copy.deepcopy(self.successor)
        summary_drift["summary"]["finalSuccessorElementCount"] += 1
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError, "successor lineage summary drifted"
        ):
            lineage.validate_receipt(
                root=materializer.ROOT,
                receipt=summary_drift,
                historical_receipt=self.historical,
                catalog=self.base_catalog,
                cues=self.base_cues,
            )

    def test_document_hash_and_baseline_denominator_drift_fail_closed(self) -> None:
        entry = copy.deepcopy(self.successor["successorDocuments"][0])
        document = lineage.read_json(materializer.ROOT / entry["path"])
        hash_drift = copy.deepcopy(document)
        hash_drift["elements"][0]["visible"] = not hash_drift["elements"][0].get(
            "visible", True
        )
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError, "final document seal drifted"
        ):
            lineage._validate_document_lineage(
                entry=entry,
                document=hash_drift,
                historical_receipt=self.historical,
            )

        incomplete = copy.deepcopy(entry)
        incomplete["lineage"].pop()
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError,
            "historical baseline denominator is not closed",
        ):
            lineage._validate_document_lineage(
                entry=incomplete,
                document=document,
                historical_receipt=self.historical,
            )

    def test_unknown_successor_element_fails_closed(self) -> None:
        entry = copy.deepcopy(self.successor["successorDocuments"][0])
        document = lineage.read_json(materializer.ROOT / entry["path"])
        entry["lineage"][0]["successorElementIds"] = ["missing.element"]
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError, "successor mapping is invalid"
        ):
            lineage._validate_document_lineage(
                entry=entry,
                document=document,
                historical_receipt=self.historical,
            )

    def test_selection_and_owner_rebound_drift_fail_closed(self) -> None:
        selection = copy.deepcopy(self.base_selection)
        selection["selections"][0]["reviewBasis"] += " drift"
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError,
            "historical source selection projection seal drifted",
        ):
            lineage.project_historical_selection_manifest(
                selection, self.successor
            )

        receipt = copy.deepcopy(self.successor)
        receipt["historicalOutputProjection"]["reviewedProjectionLedger"][
            "ownerTransfers"
        ][0]["rowCount"] += 1
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError, "owner transfer drifted"
        ):
            lineage.project_historical_reviewed_projection_ledger(
                self.blockers["reviewedProjectionLedger"], receipt
            )

    def test_live_catalog_and_cue_drift_fail_closed(self) -> None:
        catalog = copy.deepcopy(self.base_catalog)
        excluded_id = self.successor["historicalInputProjection"]["catalog"][
            "excludedLiveRows"
        ][0]["effectAssetId"]
        next(
            row for row in catalog["effects"]
            if row["effectAssetId"] == excluded_id
        )["authoringPath"] += ".drift"
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError, "live-only Valtan catalog rows drifted"
        ):
            lineage.project_historical_catalog(catalog, self.successor)

        cues = copy.deepcopy(self.base_cues)
        live_row = self.successor["historicalInputProjection"]["cues"][
            "rowRebounds"
        ][0]["liveRow"]
        next(
            row for row in cues["cues"]
            if row["bindingId"] == live_row["bindingId"]
        )["actionId"] += ".drift"
        with self.assertRaisesRegex(
            lineage.SuccessorLineageError, "live cue rebound drifted"
        ):
            lineage.project_historical_cues(cues, self.successor)

    def test_crlf_checkout_is_not_a_stale_semantic_output(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "output.json"
            path.write_bytes(b'{\r\n  "value": 1\r\n}\r\n')
            writes = {path: b'{\n  "value": 1\n}\n'}
            self.assertEqual({}, materializer._changed_outputs(writes))
            writes[path] = b'{\n  "value": 2\n}\n'
            self.assertEqual({path: writes[path]}, materializer._changed_outputs(writes))


if __name__ == "__main__":
    unittest.main()
