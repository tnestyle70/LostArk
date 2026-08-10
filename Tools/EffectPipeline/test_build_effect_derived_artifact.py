from __future__ import annotations

import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


MODULE_PATH = Path(__file__).with_name("build_effect_derived_artifact.py")
SPEC = importlib.util.spec_from_file_location("effect_derived", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
derived = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(derived)

REPO_ROOT = Path(__file__).resolve().parents[2]
PUBLISHER = Path(__file__).with_name("Publish-Effects.ps1")
SCHEMA_PATH = Path(__file__).with_name("Schemas") / (
    "lostark.effect-derived-artifact-contract.schema.json"
)


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def raw_sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class DerivedArtifactContractTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="lostark-derived-test-")
        self.root = Path(self.temporary.name)
        self.input_root = self.root / "inputs"
        self.effect_id = "effect.fixture.derived"
        self.compiler_revision = "fixture.compiler.v1"
        self.artifact_revision = 7
        self.request_path = self.root / "request.json"
        self.input_root.mkdir(parents=True)
        self.request = self._make_request(
            self.effect_id, self.compiler_revision, self.artifact_revision
        )
        write_json(self.request_path, self.request)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _make_request(
        self, effect_id: str, compiler_revision: str, artifact_revision: int
    ) -> dict:
        values = {
            "sourceContract": {
                "schema": "lostark.effect-authoring",
                "version": 14,
                "effectAssetId": effect_id + ".source",
                "displayName": "Read-only source fixture",
                "elements": [],
            },
            "sourceSemanticClosure": {"schema": "fixture.semantic", "closed": True},
            "geometryContract": {"schema": "fixture.geometry", "carriers": []},
            "materialContract": {"schema": "fixture.material", "recipes": []},
            "resourceBinding": {"schema": "fixture.resources", "bindings": []},
            "compilerInput": {"schema": "fixture.compiler-input", "inputs": []},
        }
        inputs: dict[str, dict[str, str]] = {}
        for name, value in values.items():
            path = self.input_root / f"{effect_id}.{name}.json"
            write_json(path, value)
            inputs[name] = {
                "path": path.relative_to(self.input_root).as_posix(),
                "rawSha256": raw_sha(path),
                "canonicalSha256": hashlib.sha256(
                    derived.canonical_json_bytes(value)
                ).hexdigest(),
            }
        compiled_ir = {
            "schema": "lostark.effect-compiled-ir",
            "formatVersion": 1,
            "effectAssetId": effect_id,
            "artifactRevision": artifact_revision,
            "compilerRevision": compiler_revision,
            "runtimeSemanticAuthority": "IMMUTABLE_COMPILED_IR",
            "program": {
                "opcodes": [],
                "resourceBindings": [],
                "handlerReceipts": [],
            },
        }
        compiled_path = self.input_root / f"{effect_id}.compiled-ir.json"
        write_json(compiled_path, compiled_ir)
        inputs["compiledIr"] = {
            "path": compiled_path.relative_to(self.input_root).as_posix(),
            "rawSha256": raw_sha(compiled_path),
            "canonicalSha256": hashlib.sha256(
                derived.canonical_json_bytes(compiled_ir)
            ).hexdigest(),
        }
        return {
            "schema": "lostark.effect-derived-build-request",
            "formatVersion": 1,
            "effectAssetId": effect_id,
            "displayName": "Derived fixture",
            "inputs": inputs,
            "compilerRevision": compiler_revision,
            "artifactRevision": artifact_revision,
            "artifactBindingBlockerSet": [],
            "executionBlockerSet": [],
        }

    def _outputs(self, root: Path | None = None, effect_id: str | None = None) -> dict[str, Path]:
        target = root or (self.root / "outputs")
        identity = effect_id or self.effect_id
        return {
            "authoring": target / "Authored" / f"{identity}.effect.json",
            "assembly": target / "Assemblies" / f"{identity}.assembly.json",
            "artifact": target / "Compiled" / f"{identity}.compiled-effect.json",
            "receipt": target / "Compiled" / f"{identity}.compiled-effect.receipt.json",
        }

    def _build(self, request: dict | None = None, outputs: dict[str, Path] | None = None):
        documents = derived.build_bundle_documents(request or self.request, self.input_root)
        targets = outputs or self._outputs()
        derived.write_bundle_transactionally(documents, targets)
        return documents, targets

    def _run_publisher(
        self, data_root: Path, resource_root: Path, output: Path, mode: str = "Publish"
    ) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                "powershell",
                "-NoProfile",
                "-ExecutionPolicy",
                "Bypass",
                "-File",
                str(PUBLISHER),
                "-Mode",
                mode,
                "-DataRoot",
                str(data_root),
                "-ResourceRoot",
                str(resource_root),
                "-OutputPath",
                str(output),
            ],
            text=True,
            capture_output=True,
            cwd=REPO_ROOT,
        )

    def _build_publisher_fixture(self) -> tuple[Path, Path, Path, dict[str, Path]]:
        data_root = self.root / "Data"
        resource_root = self.root / "Resources"
        runtime_output = self.root / "Runtime" / "EffectCatalog.runtime.json"
        (data_root / "Effects" / "Components").mkdir(parents=True)
        resource_root.mkdir(parents=True)
        outputs = {
            "authoring": data_root / "Effects" / "Authored" / f"{self.effect_id}.effect.json",
            "assembly": data_root / "Effects" / "Assemblies" / "Fixture" / f"{self.effect_id}.assembly.json",
            "artifact": data_root / "Effects" / "Compiled" / f"{self.effect_id}.compiled-effect.json",
            "receipt": data_root / "Effects" / "Compiled" / f"{self.effect_id}.compiled-effect.receipt.json",
        }
        self._build(outputs=outputs)
        catalog = {
            "formatVersion": 1,
            "effects": [
                {
                    "effectAssetId": self.effect_id,
                    "authoringPath": f"Effects/Authored/{self.effect_id}.effect.json",
                    "compiledArtifactPath": (
                        f"Effects/Compiled/{self.effect_id}.compiled-effect.json"
                    ),
                    "compiledReceiptPath": (
                        f"Effects/Compiled/{self.effect_id}.compiled-effect.receipt.json"
                    ),
                }
            ],
        }
        write_json(data_root / "Effects" / "EffectCatalog.json", catalog)
        return data_root, resource_root, runtime_output, outputs

    def test_schema_and_production_code_have_no_artist_fixture_hardcode(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(
            schema["$defs"]["derivedRuntimeEntry"]["properties"]["productAdmission"]["const"],
            False,
        )
        production = MODULE_PATH.read_text(encoding="utf-8") + PUBLISHER.read_text(
            encoding="utf-8"
        )
        for forbidden in ("31470", "7/35", "399", "629", "effect.artist"):
            self.assertNotIn(forbidden, production)

    def test_zero_blocker_bundle_round_trip_and_identity_only_carriers(self) -> None:
        _, outputs = self._build()
        authoring, assembly, artifact, receipt = derived.validate_bundle_files(
            outputs["authoring"],
            outputs["assembly"],
            outputs["artifact"],
            outputs["receipt"],
        )
        self.assertEqual(authoring["elements"], [])
        self.assertNotIn("particleSystem", authoring)
        self.assertNotIn("componentCues", assembly)
        self.assertTrue(artifact["executionAdmission"])
        self.assertFalse(artifact["productAdmission"])
        self.assertEqual(receipt["executionBlockerCount"], 0)
        self.assertEqual(receipt["artifactBindingBlockerCount"], 0)
        self.assertFalse(receipt["productAdmission"])
        self.assertEqual(
            [row["role"] for row in receipt["toolDependencies"]],
            [
                "DERIVED_ARTIFACT_GENERATOR",
                "DERIVED_ARTIFACT_SCHEMA",
                "EFFECT_PUBLISHER",
            ],
        )
        identity_keys = list(authoring["derivedIdentity"])
        self.assertEqual(
            identity_keys,
            ["schema", "formatVersion", *derived.IDENTITY_FIELDS],
        )

    def test_cli_build_and_prepare_runtime_entry(self) -> None:
        outputs = self._outputs()
        command = [
            sys.executable,
            "-B",
            str(MODULE_PATH),
            "build",
            "--request",
            str(self.request_path),
            "--input-root",
            str(self.input_root),
        ]
        for name in ("authoring", "assembly", "artifact", "receipt"):
            command.extend((f"--{name}-output", str(outputs[name])))
        result = subprocess.run(command, text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        entry_path = self.root / "entry.json"
        result = subprocess.run(
            [
                sys.executable,
                "-B",
                str(MODULE_PATH),
                "prepare-runtime-entry",
                "--authoring",
                str(outputs["authoring"]),
                "--assembly",
                str(outputs["assembly"]),
                "--artifact",
                str(outputs["artifact"]),
                "--receipt",
                str(outputs["receipt"]),
                "--output",
                str(entry_path),
            ],
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        entry = derived.load_json(entry_path)
        self.assertEqual(entry["payloadKind"], "IMMUTABLE_COMPILED_IR")
        self.assertFalse(entry["productAdmission"])
        self.assertNotIn("authoringCarrier", entry)
        self.assertNotIn("assembly", entry)

    def test_execution_or_artifact_blocker_writes_no_output(self) -> None:
        outputs = self._outputs()
        for path in outputs.values():
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"preserved")
        before = {key: path.read_bytes() for key, path in outputs.items()}
        for field in ("executionBlockerSet", "artifactBindingBlockerSet"):
            request = copy.deepcopy(self.request)
            request[field] = ["FIXTURE_BLOCKER"]
            with self.assertRaises(derived.ContractError):
                self._build(request=request, outputs=outputs)
            self.assertEqual(before, {key: path.read_bytes() for key, path in outputs.items()})

    def test_input_and_compiled_ir_hash_mutations_are_rejected(self) -> None:
        semantic = self.input_root / self.request["inputs"]["sourceSemanticClosure"]["path"]
        semantic.write_bytes(semantic.read_bytes() + b" ")
        with self.assertRaisesRegex(derived.ContractError, "raw SHA mismatch"):
            derived.build_bundle_documents(self.request, self.input_root)
        semantic_value = derived.load_json(semantic)
        lf_payload = (json.dumps(semantic_value, indent=2) + "\n").encode("utf-8")
        crlf_payload = lf_payload.replace(b"\n", b"\r\n")
        self.assertNotEqual(hashlib.sha256(lf_payload).digest(), hashlib.sha256(crlf_payload).digest())
        self.assertEqual(
            derived.canonical_text_bytes(lf_payload, "fixture"),
            derived.canonical_text_bytes(crlf_payload, "fixture"),
        )
        semantic.write_bytes(crlf_payload)
        self.request["inputs"]["sourceSemanticClosure"]["rawSha256"] = raw_sha(
            semantic
        )
        derived.build_bundle_documents(self.request, self.input_root)
        semantic_value["closed"] = False
        write_json(semantic, semantic_value)
        self.request["inputs"]["sourceSemanticClosure"]["rawSha256"] = raw_sha(
            semantic
        )
        with self.assertRaisesRegex(derived.ContractError, "canonical SHA mismatch"):
            derived.build_bundle_documents(self.request, self.input_root)
        semantic_value["closed"] = True
        write_json(semantic, semantic_value)
        self.request["inputs"]["sourceSemanticClosure"]["rawSha256"] = raw_sha(
            semantic
        )
        compiled = self.input_root / self.request["inputs"]["compiledIr"]["path"]
        value = derived.load_json(compiled)
        value["program"]["opcodes"].append({"opcode": "FORGED"})
        write_json(compiled, value)
        self.request["inputs"]["compiledIr"]["rawSha256"] = raw_sha(compiled)
        with self.assertRaisesRegex(derived.ContractError, "canonical SHA mismatch"):
            derived.build_bundle_documents(self.request, self.input_root)
        semantic.write_text('{"schema":"first","schema":"second"}\n', encoding="utf-8")
        self.request["inputs"]["sourceSemanticClosure"]["rawSha256"] = raw_sha(
            semantic
        )
        with self.assertRaisesRegex(derived.ContractError, "duplicate JSON key"):
            derived.build_bundle_documents(self.request, self.input_root)
        semantic.write_bytes(b"\xef\xbb\xbf" + b'{"schema":"fixture.semantic"}\n')
        self.request["inputs"]["sourceSemanticClosure"]["rawSha256"] = raw_sha(
            semantic
        )
        with self.assertRaisesRegex(derived.ContractError, "without BOM"):
            derived.build_bundle_documents(self.request, self.input_root)

    def test_revision_and_json_integer_type_mismatch_are_rejected(self) -> None:
        for replacement in (True, 7.0, 8):
            request = copy.deepcopy(self.request)
            request["artifactRevision"] = replacement
            with self.assertRaises(derived.ContractError):
                derived.build_bundle_documents(request, self.input_root)
        request = copy.deepcopy(self.request)
        request["compilerRevision"] = "fixture.compiler.v2"
        with self.assertRaisesRegex(derived.ContractError, "compilerRevision mismatch"):
            derived.build_bundle_documents(request, self.input_root)
        for unsafe in (
            "/absolute.json",
            "C:/drive.json",
            "../escape.json",
            "bad\\path.json",
            "bad//path.json",
            "space path.json",
        ):
            request = copy.deepcopy(self.request)
            request["inputs"]["geometryContract"]["path"] = unsafe
            with self.assertRaises(derived.ContractError):
                derived.build_bundle_documents(request, self.input_root)

    def test_carrier_raw_semantics_and_runtime_carrier_embedding_are_rejected(self) -> None:
        _, outputs = self._build()
        carrier = derived.load_json(outputs["authoring"])
        carrier["particleSystem"] = {"forged": True}
        write_json(outputs["authoring"], carrier)
        with self.assertRaisesRegex(derived.ContractError, "keys mismatch"):
            derived.prepare_runtime_entry(
                outputs["authoring"],
                outputs["assembly"],
                outputs["artifact"],
                outputs["receipt"],
            )
        self._build(outputs=outputs)
        entry = derived.prepare_runtime_entry(
            outputs["authoring"], outputs["assembly"], outputs["artifact"], outputs["receipt"]
        )
        entry["authoringCarrier"] = derived.load_json(outputs["authoring"])
        with self.assertRaisesRegex(derived.ContractError, "keys mismatch"):
            derived.validate_derived_runtime_entry(entry)

    def test_compiled_a_carrier_b_swap_is_rejected(self) -> None:
        _, outputs_a = self._build()
        other_id = "effect.fixture.derived.other"
        other_request = self._make_request(other_id, "fixture.compiler.v2", 8)
        outputs_b = self._outputs(self.root / "outputs-b", other_id)
        self._build(other_request, outputs_b)
        with self.assertRaises(derived.ContractError):
            derived.prepare_runtime_entry(
                outputs_b["authoring"],
                outputs_b["assembly"],
                outputs_a["artifact"],
                outputs_a["receipt"],
            )

    def test_bundle_raw_mutations_and_resealed_revision_are_rejected(self) -> None:
        _, outputs = self._build()
        artifact = derived.load_json(outputs["artifact"])
        artifact["artifactRevision"] += 1
        artifact["compiledIr"]["artifactRevision"] += 1
        write_json(outputs["artifact"], artifact)
        receipt = derived.load_json(outputs["receipt"])
        receipt["artifactRevision"] += 1
        receipt["compiledArtifactSha256"] = raw_sha(outputs["artifact"])
        write_json(outputs["receipt"], receipt)
        with self.assertRaises(derived.ContractError):
            derived.prepare_runtime_entry(
                outputs["authoring"],
                outputs["assembly"],
                outputs["artifact"],
                outputs["receipt"],
            )
        self._build(outputs=outputs)
        receipt = derived.load_json(outputs["receipt"])
        generator_payload = MODULE_PATH.read_bytes()
        generator_lf = derived.canonical_text_bytes(generator_payload, "generator")
        receipt["toolDependencies"][0]["rawSha256"] = hashlib.sha256(
            generator_lf.replace(b"\n", b"\r\n")
        ).hexdigest()
        outputs["receipt"].write_bytes(derived.pretty_json_bytes(receipt))
        derived.prepare_runtime_entry(
            outputs["authoring"],
            outputs["assembly"],
            outputs["artifact"],
            outputs["receipt"],
        )
        receipt["toolDependencies"][0]["canonicalSha256"] = "0" * 64
        outputs["receipt"].write_bytes(derived.pretty_json_bytes(receipt))
        with self.assertRaisesRegex(derived.ContractError, "tool dependency"):
            derived.prepare_runtime_entry(
                outputs["authoring"],
                outputs["assembly"],
                outputs["artifact"],
                outputs["receipt"],
            )

    def test_multi_file_commit_rolls_back_after_partial_replace(self) -> None:
        first = self.root / "first.json"
        second = self.root / "second.json"
        staged_first = self.root / "staged-first.json"
        staged_second = self.root / "staged-second.json"
        first.write_bytes(b"old-first")
        second.write_bytes(b"old-second")
        staged_first.write_bytes(b"new-first")
        staged_second.write_bytes(b"new-second")
        real_replace = os.replace
        calls = 0

        def fail_fourth(source, destination):
            nonlocal calls
            calls += 1
            if calls == 4:
                raise OSError("injected commit failure")
            return real_replace(source, destination)

        with mock.patch.object(derived.os, "replace", side_effect=fail_fourth):
            with self.assertRaises(OSError):
                derived._commit_staged_files(
                    [(staged_first, first), (staged_second, second)]
                )
        self.assertEqual(first.read_bytes(), b"old-first")
        self.assertEqual(second.read_bytes(), b"old-second")

    def test_runtime_catalog_validator_rejects_identity_and_product_mutation(self) -> None:
        _, outputs = self._build()
        entry = derived.prepare_runtime_entry(
            outputs["authoring"], outputs["assembly"], outputs["artifact"], outputs["receipt"]
        )
        catalog = {
            "schema": "lostark.effect-runtime-catalog",
            "formatVersion": 3,
            "components": [],
            "effects": [entry],
        }
        derived.validate_runtime_catalog(catalog)
        for mutate in (
            lambda row: row.__setitem__("productAdmission", True),
            lambda row: row.__setitem__("compilerRevision", "fixture.compiler.swapped"),
            lambda row: row["derivedIdentity"].__setitem__("compilerInputHash", "0" * 64),
        ):
            forged = copy.deepcopy(catalog)
            mutate(forged["effects"][0])
            with self.assertRaises(derived.ContractError):
                derived.validate_runtime_catalog(forged)

    def test_publisher_format3_round_trip_keeps_product_false(self) -> None:
        data_root, resources, output, _ = self._build_publisher_fixture()
        result = self._run_publisher(data_root, resources, output)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        runtime = derived.load_json(output)
        derived.validate_runtime_catalog(runtime)
        self.assertEqual(runtime["formatVersion"], 3)
        self.assertEqual(len(runtime["effects"]), 1)
        self.assertFalse(runtime["effects"][0]["productAdmission"])
        self.assertEqual(runtime["effects"][0]["payloadKind"], "IMMUTABLE_COMPILED_IR")
        committed = output.read_bytes()
        validate_result = self._run_publisher(
            data_root, resources, output, mode="Validate"
        )
        self.assertEqual(
            validate_result.returncode,
            0,
            validate_result.stderr + validate_result.stdout,
        )
        self.assertEqual(output.read_bytes(), committed)

    def test_publisher_rejects_v14_direct_and_missing_compiled_pair_without_overwrite(self) -> None:
        data_root, resources, output, outputs = self._build_publisher_fixture()
        baseline_result = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline_result.returncode, 0, baseline_result.stderr)
        committed = output.read_bytes()

        source_path = self.input_root / self.request["inputs"]["sourceContract"]["path"]
        outputs["authoring"].write_bytes(source_path.read_bytes())
        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertEqual(output.read_bytes(), committed)

        self._build(outputs=outputs)
        catalog_path = data_root / "Effects" / "EffectCatalog.json"
        outputs["artifact"].unlink()
        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertEqual(output.read_bytes(), committed)

        self._build(outputs=outputs)
        catalog = derived.load_json(catalog_path)
        del catalog["effects"][0]["compiledReceiptPath"]
        write_json(catalog_path, catalog)
        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertEqual(output.read_bytes(), committed)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_publisher_rejects_compiled_a_carrier_b_swap_without_overwrite(self) -> None:
        data_root, resources, output, outputs = self._build_publisher_fixture()
        baseline_result = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline_result.returncode, 0, baseline_result.stderr)
        committed = output.read_bytes()

        other_id = "effect.fixture.other"
        other_request = self._make_request(other_id, "fixture.compiler.v2", 9)
        other_outputs = self._outputs(self.root / "other", other_id)
        self._build(other_request, other_outputs)
        outputs["artifact"].write_bytes(other_outputs["artifact"].read_bytes())
        outputs["receipt"].write_bytes(other_outputs["receipt"].read_bytes())
        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertEqual(output.read_bytes(), committed)


if __name__ == "__main__":
    unittest.main()
