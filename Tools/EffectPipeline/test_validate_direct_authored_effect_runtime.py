#!/usr/bin/env python3

from __future__ import annotations

import copy
from contextlib import redirect_stderr
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest


MODULE_PATH = Path(__file__).with_name("validate_direct_authored_effect_runtime.py")
SPEC = importlib.util.spec_from_file_location("direct_authored_validator", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
validator = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(validator)

EFFECT_ID = "effect.fixture.direct-authored"


def make_authored_payload(effect_id: str = EFFECT_ID) -> bytes:
    return (
        json.dumps(
            {
                "schema": "lostark.effect-authoring",
                "version": 13,
                "effectAssetId": effect_id,
                "displayName": "Direct authored fixture",
                "particleSystem": {},
                "modelCues": [],
                "elements": [],
            },
            ensure_ascii=False,
            indent=2,
        )
        + "\n"
    ).encode("utf-8")


def install_sealed_document(
    runtime_catalog_path: Path,
    payload: bytes,
    effect_id: str = EFFECT_ID,
) -> tuple[str, Path]:
    content_sha = hashlib.sha256(payload).hexdigest()
    relative_path = f"Authored/{effect_id}.{content_sha}.effect.json"
    destination = runtime_catalog_path.parent / relative_path
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(payload)
    return relative_path, destination


def make_catalog(runtime_catalog_path: Path) -> dict[str, object]:
    authored_path, _ = install_sealed_document(
        runtime_catalog_path, make_authored_payload()
    )
    return {
        "schema": "lostark.effect-runtime-catalog",
        "formatVersion": 3,
        "components": [],
        "effects": [
            {
                "payloadKind": "LEGACY_ASSEMBLY_V1",
                "effectAssetId": "effect.fixture.legacy",
                "authoringFormatVersion": 12,
                "contentSha256": "0" * 64,
                "dependencies": [],
                "assembly": {},
            },
            {
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                "effectAssetId": EFFECT_ID,
                "authoringFormatVersion": 13,
                "authoredDocumentPath": authored_path,
            },
        ],
    }


class DirectAuthoredRuntimeValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self._temporary = tempfile.TemporaryDirectory()
        self.runtime_catalog_path = (
            Path(self._temporary.name) / "EffectCatalog.runtime.json"
        )

    def tearDown(self) -> None:
        self._temporary.cleanup()

    def test_valid_direct_row_delegates_non_direct_projection(self) -> None:
        validator.validate_runtime_catalog(
            make_catalog(self.runtime_catalog_path), self.runtime_catalog_path
        )

        marked = make_catalog(self.runtime_catalog_path)
        marked = {
            "schema": marked["schema"],
            "formatVersion": marked["formatVersion"],
            "obsoleteIntegrityRequired": True,
            "obsoleteIntegrityDigest": "c" * 64,
            "components": marked["components"],
            "effects": marked["effects"],
        }
        with self.assertRaisesRegex(validator.ContractError, "fields or order"):
            validator.validate_runtime_catalog(marked, self.runtime_catalog_path)

    def test_version_extra_field_duplicate_and_order_fail_closed(self) -> None:
        mutations: list[dict[str, object]] = []

        wrong_version = copy.deepcopy(make_catalog(self.runtime_catalog_path))
        wrong_version["effects"][1]["authoringFormatVersion"] = 12
        mutations.append(wrong_version)

        extra_integrity_fields = copy.deepcopy(
            make_catalog(self.runtime_catalog_path)
        )
        extra_integrity_fields["effects"][1]["contentSha256"] = "a" * 64
        mutations.append(extra_integrity_fields)

        duplicate_effect = copy.deepcopy(make_catalog(self.runtime_catalog_path))
        duplicate_effect["effects"][0]["effectAssetId"] = EFFECT_ID
        mutations.append(duplicate_effect)

        reordered = copy.deepcopy(make_catalog(self.runtime_catalog_path))
        direct = reordered["effects"][1]
        reordered["effects"][1] = {
            "effectAssetId": direct["effectAssetId"],
            **{key: value for key, value in direct.items() if key != "effectAssetId"},
        }
        mutations.append(reordered)

        for index, mutation in enumerate(mutations):
            with self.subTest(mutation=index):
                with self.assertRaises(validator.ContractError):
                    validator.validate_runtime_catalog(
                        mutation, self.runtime_catalog_path
                    )

    def test_missing_escape_and_wrong_path_are_rejected_without_repair(self) -> None:
        catalog = make_catalog(self.runtime_catalog_path)
        direct = catalog["effects"][1]
        sealed_path = self.runtime_catalog_path.parent / direct["authoredDocumentPath"]
        sealed_payload = sealed_path.read_bytes()

        sealed_path.unlink()
        with self.assertRaisesRegex(validator.ContractError, "could not be resolved"):
            validator.validate_runtime_catalog(catalog, self.runtime_catalog_path)
        self.assertFalse(sealed_path.exists())

        sealed_path.write_bytes(sealed_payload)
        for invalid_path in (
            "../Authored/escaped.effect.json",
            f"Authored/{EFFECT_ID}.wrong.effect.json",
        ):
            mutated = copy.deepcopy(catalog)
            mutated["effects"][1]["authoredDocumentPath"] = invalid_path
            with self.subTest(path=invalid_path):
                with self.assertRaisesRegex(
                    validator.ContractError, "unsafe or does not match"
                ):
                    validator.validate_runtime_catalog(
                        mutated, self.runtime_catalog_path
                    )
                self.assertEqual(sealed_payload, sealed_path.read_bytes())

        drifted_payload = sealed_payload + b" "
        sealed_path.write_bytes(drifted_payload)
        validator.validate_runtime_catalog(catalog, self.runtime_catalog_path)
        self.assertEqual(drifted_payload, sealed_path.read_bytes())

    def test_identity_bom_duplicate_property_and_cli_crlf_are_rejected(self) -> None:
        wrong_identity = make_catalog(self.runtime_catalog_path)
        direct = wrong_identity["effects"][1]
        relative_path, _ = install_sealed_document(
            self.runtime_catalog_path,
            make_authored_payload("effect.fixture.wrong"),
        )
        direct["authoredDocumentPath"] = relative_path
        with self.assertRaisesRegex(validator.ContractError, "identity mismatch"):
            validator.validate_runtime_catalog(
                wrong_identity, self.runtime_catalog_path
            )

        bom_catalog = make_catalog(self.runtime_catalog_path)
        direct = bom_catalog["effects"][1]
        relative_path, _ = install_sealed_document(
            self.runtime_catalog_path, b"\xef\xbb\xbf" + make_authored_payload()
        )
        direct["authoredDocumentPath"] = relative_path
        with self.assertRaisesRegex(validator.ContractError, "without BOM"):
            validator.validate_runtime_catalog(bom_catalog, self.runtime_catalog_path)

        duplicate_catalog = make_catalog(self.runtime_catalog_path)
        direct = duplicate_catalog["effects"][1]
        duplicate_payload = make_authored_payload().replace(
            b'  "version": 13,', b'  "version": 13,\n  "version": 13,'
        )
        relative_path, _ = install_sealed_document(
            self.runtime_catalog_path, duplicate_payload
        )
        direct["authoredDocumentPath"] = relative_path
        with self.assertRaisesRegex(validator.ContractError, "duplicate JSON property"):
            validator.validate_runtime_catalog(
                duplicate_catalog, self.runtime_catalog_path
            )

        cli_catalog = make_catalog(self.runtime_catalog_path)
        self.runtime_catalog_path.write_bytes(
            json.dumps(cli_catalog, ensure_ascii=False)
            .replace("}", "}\r\n", 1)
            .encode("utf-8")
        )
        with redirect_stderr(io.StringIO()):
            self.assertEqual(
                validator.main(["--catalog", str(self.runtime_catalog_path)]), 1
            )


if __name__ == "__main__":
    unittest.main()
