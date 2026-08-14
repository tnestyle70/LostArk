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


def make_catalog() -> dict[str, object]:
    authored_text = json.dumps(
        {
            "schema": "lostark.effect-authoring",
            "version": 13,
            "effectAssetId": "effect.fixture.direct-authored",
            "displayName": "Direct authored fixture",
            "particleSystem": {},
            "modelCues": [],
            "elements": [],
        },
        ensure_ascii=False,
        indent=2,
    ) + "\n"
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
                "effectAssetId": "effect.fixture.direct-authored",
                "authoringFormatVersion": 13,
                "contentSha256": hashlib.sha256(
                    authored_text.encode("utf-8")
                ).hexdigest(),
                "dependencies": [
                    {"assetId": "Effect/Fixture/base.dds", "sha256": "a" * 64},
                    {"assetId": "Effect/Fixture/mesh.wmodel", "sha256": "b" * 64},
                ],
                "authoredDocumentUtf8": authored_text,
            },
        ],
    }


class DirectAuthoredRuntimeValidatorTests(unittest.TestCase):
    def test_valid_direct_row_delegates_non_direct_projection(self) -> None:
        validator.validate_runtime_catalog(make_catalog())

    def test_hash_version_identity_path_duplicate_and_order_fail_closed(self) -> None:
        mutations: list[dict[str, object]] = []

        wrong_hash = copy.deepcopy(make_catalog())
        wrong_hash["effects"][1]["contentSha256"] = "f" * 64
        mutations.append(wrong_hash)

        wrong_version = copy.deepcopy(make_catalog())
        wrong_version["effects"][1]["authoringFormatVersion"] = 12
        mutations.append(wrong_version)

        wrong_identity = copy.deepcopy(make_catalog())
        direct = wrong_identity["effects"][1]
        direct["authoredDocumentUtf8"] = direct["authoredDocumentUtf8"].replace(
            "effect.fixture.direct-authored", "effect.fixture.wrong"
        )
        direct["contentSha256"] = hashlib.sha256(
            direct["authoredDocumentUtf8"].encode("utf-8")
        ).hexdigest()
        mutations.append(wrong_identity)

        unsafe_path = copy.deepcopy(make_catalog())
        unsafe_path["effects"][1]["dependencies"][0]["assetId"] = (
            "Effect/Fixture/../base.dds"
        )
        mutations.append(unsafe_path)

        duplicate_dependency = copy.deepcopy(make_catalog())
        duplicate_dependency["effects"][1]["dependencies"][1]["assetId"] = (
            "Effect/Fixture/base.dds"
        )
        mutations.append(duplicate_dependency)

        duplicate_effect = copy.deepcopy(make_catalog())
        duplicate_effect["effects"][0]["effectAssetId"] = (
            "effect.fixture.direct-authored"
        )
        mutations.append(duplicate_effect)

        reordered = copy.deepcopy(make_catalog())
        direct = reordered["effects"][1]
        reordered["effects"][1] = {
            "effectAssetId": direct["effectAssetId"],
            **{key: value for key, value in direct.items() if key != "effectAssetId"},
        }
        mutations.append(reordered)

        for mutation in mutations:
            with self.subTest(mutation=mutations.index(mutation)):
                with self.assertRaises(validator.ContractError):
                    validator.validate_runtime_catalog(mutation)

    def test_embedded_duplicate_property_and_cli_crlf_are_rejected(self) -> None:
        catalog = make_catalog()
        direct = catalog["effects"][1]
        direct["authoredDocumentUtf8"] = direct["authoredDocumentUtf8"].replace(
            '"version": 13,', '"version": 13,\n  "version": 13,'
        )
        direct["contentSha256"] = hashlib.sha256(
            direct["authoredDocumentUtf8"].encode("utf-8")
        ).hexdigest()
        with self.assertRaisesRegex(validator.ContractError, "duplicate JSON property"):
            validator.validate_runtime_catalog(catalog)

        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "runtime.json"
            path.write_bytes(
                json.dumps(make_catalog(), ensure_ascii=False).replace("}", "}\r\n", 1).encode("utf-8")
            )
            with redirect_stderr(io.StringIO()):
                self.assertEqual(validator.main(["--catalog", str(path)]), 1)


if __name__ == "__main__":
    unittest.main()
