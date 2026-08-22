from __future__ import annotations

import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
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

DIRECT_RUNTIME_MODULE_PATH = Path(__file__).with_name(
    "validate_direct_authored_effect_runtime.py"
)
DIRECT_RUNTIME_SPEC = importlib.util.spec_from_file_location(
    "direct_effect_runtime", DIRECT_RUNTIME_MODULE_PATH
)
assert DIRECT_RUNTIME_SPEC is not None and DIRECT_RUNTIME_SPEC.loader is not None
direct_runtime = importlib.util.module_from_spec(DIRECT_RUNTIME_SPEC)
DIRECT_RUNTIME_SPEC.loader.exec_module(direct_runtime)

REPO_ROOT = Path(__file__).resolve().parents[2]
PUBLISHER = Path(__file__).with_name("Publish-Effects.ps1")
SCHEMA_PATH = Path(__file__).with_name("Schemas") / (
    "lostark.effect-derived-artifact-contract.schema.json"
)
RECONSTRUCTED_CANDIDATE = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.reconstructed-runtime-program.candidate.json"
)
RECONSTRUCTED_SIDECAR = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.reconstructed-render-resource-authority.receipt.json"
)
VISUAL_PROGRAM_SIDECAR = REPO_ROOT / (
    "Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
)
MATERIAL_PROGRAM_REGISTRY = REPO_ROOT / (
    "Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json"
)


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = (json.dumps(value, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
    path.write_bytes(payload)


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
        zero_execution = derived._make_execution_contract((), ())
        values = {
            "sourceContract": {
                "schema": "lostark.effect-authoring",
                "version": 14,
                "effectAssetId": effect_id + ".source",
                "displayName": "Read-only source fixture",
                "documentRole": "READ_ONLY_SOURCE_CONTRACT",
                "readOnly": True,
                "runtimeSemanticAuthority": "EVIDENCE_ONLY_NOT_RUNTIME_SEMANTICS",
                "derivedTargetEffectAssetId": effect_id,
                "executionContract": zero_execution,
                "evidenceRows": [],
            },
        }
        for input_name, (schema, role) in derived.UPSTREAM_INPUT_CONTRACTS.items():
            values[input_name] = {
                "schema": schema,
                "formatVersion": 1,
                "effectAssetId": effect_id + "." + input_name,
                "contractRole": role,
                "runtimeSemanticAuthority": "EVIDENCE_ONLY_NOT_RUNTIME_SEMANTICS",
                "derivedTargetEffectAssetId": effect_id,
                "executionContract": zero_execution,
                "evidenceRows": [],
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
        identity = derived._make_derived_identity(
            {
                identity_name: inputs[input_name]["canonicalSha256"]
                for input_name, identity_name in derived.INPUT_TO_IDENTITY
            }
        )
        compiled_ir = {
            "schema": "lostark.effect-compiled-ir",
            "formatVersion": 1,
            "effectAssetId": effect_id,
            "artifactRevision": artifact_revision,
            "compilerRevision": compiler_revision,
            "runtimeSemanticAuthority": "IMMUTABLE_COMPILED_IR",
            "derivedIdentity": identity,
            "executionContract": zero_execution,
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
        compiler_receipt = {
            "schema": "lostark.effect-compiler-receipt",
            "formatVersion": 1,
            "effectAssetId": effect_id,
            "artifactRevision": artifact_revision,
            "compilerRevision": compiler_revision,
            "runtimeSemanticAuthority": "IMMUTABLE_COMPILED_IR",
            "receiptAuthority": "TYPED_CASCADE_COMPILER_RECEIPT_V1",
            "derivedIdentity": identity,
            "compilerInputHash": identity["compilerInputHash"],
            "compiledIrSha256": inputs["compiledIr"]["canonicalSha256"],
            "executionContract": zero_execution,
        }
        compiler_receipt["compilerReceiptTokenSha256"] = (
            derived.make_compiler_receipt_token(compiler_receipt)
        )
        compiler_receipt_path = self.input_root / f"{effect_id}.compiler-receipt.json"
        write_json(compiler_receipt_path, compiler_receipt)
        inputs["compilerReceipt"] = {
            "path": compiler_receipt_path.relative_to(self.input_root).as_posix(),
            "rawSha256": raw_sha(compiler_receipt_path),
            "canonicalSha256": hashlib.sha256(
                derived.canonical_json_bytes(compiler_receipt)
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
        self,
        data_root: Path,
        resource_root: Path,
        output: Path,
        mode: str = "Publish",
        fault: str = "None",
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
                "-TestFaultInjection",
                fault,
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
        visual_program = data_root / (
            "Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
        )
        visual_program.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(VISUAL_PROGRAM_SIDECAR, visual_program)
        material_program_registry = data_root / (
            "Effects/MaterialPrograms/effect-material-program-registry.v1.json"
        )
        fixture_material_programs = json.loads(
            MATERIAL_PROGRAM_REGISTRY.read_text(encoding="utf-8")
        )
        fixture_material_programs["bindings"] = []
        write_json(material_program_registry, fixture_material_programs)
        return data_root, resource_root, runtime_output, outputs

    def _build_reconstructed_publisher_fixture(
        self,
    ) -> tuple[Path, Path, Path, Path]:
        data_root = self.root / "ReconstructedData"
        resource_root = self.root / "ReconstructedResources"
        runtime_output = self.root / "ReconstructedRuntime" / (
            "EffectCatalog.runtime.json"
        )
        for relative in ("Assemblies", "Authored", "Components"):
            (data_root / "Effects" / relative).mkdir(parents=True)
        candidate = data_root / (
            "Effects/Imported/Artist/Candidates/"
            "skill.31470.reconstructed-runtime-program.candidate.json"
        )
        candidate.parent.mkdir(parents=True)
        shutil.copyfile(RECONSTRUCTED_CANDIDATE, candidate)
        sidecar = data_root / (
            "Effects/Imported/Artist/Materials/"
            "skill.31470.reconstructed-render-resource-authority.receipt.json"
        )
        sidecar.parent.mkdir(parents=True)
        shutil.copyfile(RECONSTRUCTED_SIDECAR, sidecar)
        resource_root.mkdir(parents=True)
        write_json(
            data_root / "Effects" / "EffectCatalog.json",
            {
                "formatVersion": 1,
                "effects": [
                    {
                        "effectAssetId": derived.RECONSTRUCTED_EFFECT_ID,
                        "payloadKind": derived.RECONSTRUCTED_PAYLOAD_KIND,
                        "reconstructedRuntimeProgramPath": (
                            "Effects/Imported/Artist/Candidates/"
                            "skill.31470.reconstructed-runtime-program.candidate.json"
                        ),
                        "reconstructedRenderResourceAuthorityPath": (
                            "Effects/Imported/Artist/Materials/"
                            "skill.31470.reconstructed-render-resource-authority.receipt.json"
                        ),
                    }
                ],
            },
        )
        visual_program = data_root / (
            "Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
        )
        visual_program.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(VISUAL_PROGRAM_SIDECAR, visual_program)
        material_program_registry = data_root / (
            "Effects/MaterialPrograms/effect-material-program-registry.v1.json"
        )
        fixture_material_programs = json.loads(
            MATERIAL_PROGRAM_REGISTRY.read_text(encoding="utf-8")
        )
        fixture_material_programs["bindings"] = []
        write_json(material_program_registry, fixture_material_programs)
        return data_root, resource_root, runtime_output, candidate

    def _build_generic_bundle_for_effect(
        self, data_root: Path, effect_id: str
    ) -> dict[str, Path]:
        request = self._make_request(
            effect_id, self.compiler_revision, self.artifact_revision
        )
        outputs = {
            "authoring": data_root
            / "Effects"
            / "Authored"
            / f"{effect_id}.effect.json",
            "assembly": data_root
            / "Effects"
            / "Assemblies"
            / "Fixture"
            / f"{effect_id}.assembly.json",
            "artifact": data_root
            / "Effects"
            / "Compiled"
            / f"{effect_id}.compiled-effect.json",
            "receipt": data_root
            / "Effects"
            / "Compiled"
            / f"{effect_id}.compiled-effect.receipt.json",
        }
        documents = derived.build_bundle_documents(request, self.input_root)
        derived.write_bundle_transactionally(documents, outputs)
        return outputs

    def test_generic_schema_stays_product_false_and_reconstructed_profile_is_exact(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(
            schema["$defs"]["derivedRuntimeEntry"]["properties"]["productAdmission"]["const"],
            False,
        )
        self.assertEqual(
            schema["$defs"]["compilerReceiptInput"]["properties"]["receiptAuthority"]["const"],
            "TYPED_CASCADE_COMPILER_RECEIPT_V1",
        )
        self.assertIn(
            "compilerReceipt",
            schema["$defs"]["buildRequest"]["properties"]["inputs"]["required"],
        )
        self.assertEqual(
            derived.RECONSTRUCTED_ENTRY_KEYS,
            (
                "payloadKind",
                "effectAssetId",
                "artifactRevision",
                "compilerRevision",
                "sourceExact",
                "runtimeExecutionAdmission",
                "productAdmission",
                "publishReceiptSha256",
                "publishReceipt",
                "reconstructedRuntimeProgram",
                "renderResourcePublishReceiptSha256",
                "renderResourcePublishReceipt",
                "reconstructedRenderResourceAuthority",
            ),
        )
        self.assertEqual(len(derived.RECONSTRUCTED_LINK_KEYS), 16)
        self.assertEqual(len(derived.RECONSTRUCTED_RECEIPT_KEYS), 25)
        self.assertEqual(len(derived.RECONSTRUCTED_RENDER_RESOURCE_LINK_KEYS), 21)
        self.assertEqual(
            len(derived.RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_KEYS), 26
        )
        self.assertEqual(
            [
                row[0]
                for row in derived.RECONSTRUCTED_RENDER_RESOURCE_TOOL_DEPENDENCIES
            ],
            [
                "RECONSTRUCTED_RENDER_RESOURCE_INDEPENDENT_PINS",
                "RECONSTRUCTED_RENDER_RESOURCE_CATALOG_VALIDATOR",
                "EFFECT_PUBLISHER",
            ],
        )

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

        # Reproduce the coordinated-rehash attack from the frozen review: all
        # six inputs and the compiled IR honestly report a blocker while the
        # requester attempts to summarize them as empty.
        request = copy.deepcopy(self.request)
        blocked = derived._make_execution_contract((), ("FIXTURE_BLOCKER",))
        for input_name, identity_name in derived.INPUT_TO_IDENTITY:
            reference = request["inputs"][input_name]
            path = self.input_root / reference["path"]
            value = derived.load_json(path)
            value["evidenceRows"] = [
                {
                    "evidenceId": input_name + ".blocked",
                    "evidenceSha256": "1" * 64,
                    "executionContract": blocked,
                }
            ]
            value["executionContract"] = blocked
            write_json(path, value)
            reference["rawSha256"] = raw_sha(path)
            reference["canonicalSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(value)
            ).hexdigest()

        identity = derived._make_derived_identity(
            {
                identity_name: request["inputs"][input_name]["canonicalSha256"]
                for input_name, identity_name in derived.INPUT_TO_IDENTITY
            }
        )
        compiled_reference = request["inputs"]["compiledIr"]
        compiled_path = self.input_root / compiled_reference["path"]
        compiled = derived.load_json(compiled_path)
        compiled["derivedIdentity"] = identity
        compiled["executionContract"] = blocked
        compiled["program"]["handlerReceipts"] = [
            {
                "handlerId": "fixture.blocked",
                "handlerSha256": "2" * 64,
                "executionContract": blocked,
            }
        ]
        write_json(compiled_path, compiled)
        compiled_reference["rawSha256"] = raw_sha(compiled_path)
        compiled_reference["canonicalSha256"] = hashlib.sha256(
            derived.canonical_json_bytes(compiled)
        ).hexdigest()

        receipt_reference = request["inputs"]["compilerReceipt"]
        receipt_path = self.input_root / receipt_reference["path"]
        compiler_receipt = derived.load_json(receipt_path)
        compiler_receipt["derivedIdentity"] = identity
        compiler_receipt["compilerInputHash"] = identity["compilerInputHash"]
        compiler_receipt["compiledIrSha256"] = compiled_reference[
            "canonicalSha256"
        ]
        compiler_receipt["executionContract"] = blocked
        compiler_receipt["compilerReceiptTokenSha256"] = (
            derived.make_compiler_receipt_token(compiler_receipt)
        )
        write_json(receipt_path, compiler_receipt)
        receipt_reference["rawSha256"] = raw_sha(receipt_path)
        receipt_reference["canonicalSha256"] = hashlib.sha256(
            derived.canonical_json_bytes(compiler_receipt)
        ).hexdigest()

        with self.assertRaisesRegex(derived.ContractError, "authenticated upstream union"):
            self._build(request=request, outputs=outputs)
        request["executionBlockerSet"] = ["FIXTURE_BLOCKER"]
        with self.assertRaisesRegex(derived.ContractError, "authenticated upstream evidence"):
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
        semantic_value["contractRole"] = "FORGED_ROLE"
        write_json(semantic, semantic_value)
        self.request["inputs"]["sourceSemanticClosure"]["rawSha256"] = raw_sha(
            semantic
        )
        with self.assertRaisesRegex(derived.ContractError, "canonical SHA mismatch"):
            derived.build_bundle_documents(self.request, self.input_root)
        semantic_value["contractRole"] = "SOURCE_SEMANTIC_CLOSURE"
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
        request = copy.deepcopy(self.request)
        semantic_reference = request["inputs"]["sourceSemanticClosure"]
        semantic_path = self.input_root / semantic_reference["path"]
        semantic_original = derived.load_json(semantic_path)
        semantic_forged = copy.deepcopy(semantic_original)
        semantic_forged["schema"] = "lostark.effect-forged-closure"
        write_json(semantic_path, semantic_forged)
        semantic_reference["rawSha256"] = raw_sha(semantic_path)
        semantic_reference["canonicalSha256"] = hashlib.sha256(
            derived.canonical_json_bytes(semantic_forged)
        ).hexdigest()
        with self.assertRaisesRegex(derived.ContractError, "schema mismatch"):
            derived.build_bundle_documents(request, self.input_root)
        write_json(semantic_path, semantic_original)
        request = copy.deepcopy(self.request)
        receipt_reference = request["inputs"]["compilerReceipt"]
        receipt_path = self.input_root / receipt_reference["path"]
        receipt = derived.load_json(receipt_path)
        receipt["compilerReceiptTokenSha256"] = "0" * 64
        write_json(receipt_path, receipt)
        receipt_reference["rawSha256"] = raw_sha(receipt_path)
        receipt_reference["canonicalSha256"] = hashlib.sha256(
            derived.canonical_json_bytes(receipt)
        ).hexdigest()
        with self.assertRaisesRegex(derived.ContractError, "authentication token"):
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

    def test_reconstructed_entry_is_exact_product_false_and_embedded(self) -> None:
        entry = derived.prepare_reconstructed_runtime_entry(
            RECONSTRUCTED_CANDIDATE, RECONSTRUCTED_SIDECAR
        )
        self.assertEqual(tuple(entry), derived.RECONSTRUCTED_ENTRY_KEYS)
        self.assertEqual(
            tuple(entry["reconstructedRuntimeProgram"]),
            derived.RECONSTRUCTED_LINK_KEYS,
        )
        self.assertEqual(
            tuple(entry["publishReceipt"]),
            derived.RECONSTRUCTED_RECEIPT_KEYS,
        )
        self.assertEqual(
            tuple(entry["reconstructedRenderResourceAuthority"]),
            derived.RECONSTRUCTED_RENDER_RESOURCE_LINK_KEYS,
        )
        self.assertEqual(
            tuple(entry["renderResourcePublishReceipt"]),
            derived.RECONSTRUCTED_RENDER_RESOURCE_RECEIPT_KEYS,
        )
        base = {
            key: entry[key] for key in derived.RECONSTRUCTED_BASE_ENTRY_KEYS
        }
        self.assertEqual(
            hashlib.sha256(derived.canonical_json_bytes(base)).hexdigest(),
            derived.RECONSTRUCTED_BASE_ENTRY_CANONICAL_SHA256,
        )
        self.assertFalse(entry["sourceExact"])
        self.assertFalse(entry["runtimeExecutionAdmission"])
        self.assertFalse(entry["productAdmission"])
        self.assertEqual(
            entry["reconstructedRuntimeProgram"]["candidateUtf8Json"].encode(
                "utf-8"
            ),
            RECONSTRUCTED_CANDIDATE.read_bytes(),
        )
        resource_link = entry["reconstructedRenderResourceAuthority"]
        self.assertEqual(
            resource_link["sidecarUtf8Json"].encode("utf-8"),
            RECONSTRUCTED_SIDECAR.read_bytes(),
        )
        for false_gate in (
            "sourceExact",
            "runtimeExecutionAdmission",
            "executeAdmission",
            "submitAdmission",
            "renderAdmission",
            "productAdmission",
        ):
            self.assertIs(resource_link[false_gate], False)
        catalog = {
            "schema": "lostark.effect-runtime-catalog",
            "formatVersion": 3,
            "components": [],
            "effects": [entry],
        }
        derived.validate_runtime_catalog(catalog)

        reordered = copy.deepcopy(catalog)
        row = reordered["effects"][0]
        reordered["effects"][0] = {
            "effectAssetId": row["effectAssetId"],
            **{key: value for key, value in row.items() if key != "effectAssetId"},
        }
        with self.assertRaisesRegex(
            derived.ContractError, "fields or order|key order"
        ):
            derived.validate_runtime_catalog(reordered)

        wrong_integer = copy.deepcopy(catalog)
        wrong_integer["effects"][0]["reconstructedRuntimeProgram"][
            "programVersion"
        ] = True
        with self.assertRaises(derived.ContractError):
            derived.validate_runtime_catalog(wrong_integer)

    def test_reconstructed_external_inputs_clean_checkout_are_frozen_lf(self) -> None:
        inputs = (
            (
                RECONSTRUCTED_CANDIDATE,
                derived.RECONSTRUCTED_CANDIDATE_BYTE_COUNT,
                derived.RECONSTRUCTED_CANDIDATE_RAW_SHA256,
                "ddef21a5314eb8c3db891d36f702cfeda3149f20",
            ),
            (
                RECONSTRUCTED_SIDECAR,
                derived.RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_BYTE_COUNT,
                derived.RECONSTRUCTED_RENDER_RESOURCE_SIDECAR_RAW_SHA256,
                "0b6a68f82495fca2cd846bd2264b8a519f2c3c24",
            ),
        )
        relatives = [
            path.relative_to(REPO_ROOT).as_posix()
            for path, _, _, _ in inputs
        ]
        attributes = subprocess.run(
            ["git", "check-attr", "text", "eol", "--", *relatives],
            cwd=REPO_ROOT,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.splitlines()
        self.assertEqual(
            attributes,
            [line for relative in relatives for line in (
                f"{relative}: text: set",
                f"{relative}: eol: lf",
            )],
        )

        for source, byte_count, digest, authority_commit in inputs:
            with self.subTest(source=source.name):
                relative = source.relative_to(REPO_ROOT).as_posix()
                payload = subprocess.run(
                    ["git", "cat-file", "blob", f"{authority_commit}:{relative}"],
                    cwd=REPO_ROOT,
                    check=True,
                    capture_output=True,
                ).stdout
                self.assertEqual(len(payload), byte_count)
                self.assertEqual(hashlib.sha256(payload).hexdigest(), digest)
                self.assertFalse(payload.startswith(b"\xef\xbb\xbf"))
                self.assertNotIn(b"\r", payload)

    def test_reconstructed_candidate_crlf_and_coordinated_reseals_are_rejected(self) -> None:
        candidate = self.root / "candidate.json"
        candidate.write_bytes(
            RECONSTRUCTED_CANDIDATE.read_bytes().replace(b"\n", b"\r\n")
        )
        with self.assertRaisesRegex(derived.ContractError, "byte count|LF-only"):
            derived.prepare_reconstructed_runtime_entry(
                candidate, RECONSTRUCTED_SIDECAR
            )

        entry = derived.prepare_reconstructed_runtime_entry(
            RECONSTRUCTED_CANDIDATE, RECONSTRUCTED_SIDECAR
        )

        def reseal(row: dict) -> None:
            link = row["reconstructedRuntimeProgram"]
            receipt = row["publishReceipt"]
            for field in (
                "effectAssetId",
                "candidateBuilderCommitId",
                "candidateBuilderTreeId",
                "candidateBlobId",
                "resourceBindingHash",
                "inputArtifactCount",
                "inputArtifactsOrderedSha256",
                "programId",
                "programVersion",
                "programSha256",
                "candidateRawSha256",
                "candidateByteCount",
            ):
                receipt[field] = link[field]
            receipt["effectAssetId"] = row["effectAssetId"]
            receipt["reconstructedRuntimeProgramSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(link)
            ).hexdigest()
            unsigned = dict(receipt)
            del unsigned["receiptSha256"]
            receipt["receiptSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(unsigned)
            ).hexdigest()
            row["publishReceiptSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(receipt)
            ).hexdigest()

        mutations = []

        forged_resource = copy.deepcopy(entry)
        forged_resource["reconstructedRuntimeProgram"]["resourceBindingHash"] = (
            "0" * 64
        )
        reseal(forged_resource)
        mutations.append(forged_resource)

        forged_program = copy.deepcopy(entry)
        forged_program["reconstructedRuntimeProgram"]["programSha256"] = "1" * 64
        reseal(forged_program)
        mutations.append(forged_program)

        forged_asset = copy.deepcopy(entry)
        forged_asset["effectAssetId"] = "effect.artist.skill.31471"
        forged_asset["reconstructedRuntimeProgram"]["effectAssetId"] = (
            forged_asset["effectAssetId"]
        )
        reseal(forged_asset)
        mutations.append(forged_asset)

        forged_candidate = copy.deepcopy(entry)
        link = forged_candidate["reconstructedRuntimeProgram"]
        link["candidateUtf8Json"] = link["candidateUtf8Json"].replace(
            '"inputSlot": "F"', '"inputSlot": "Q"', 1
        )
        forged_payload = link["candidateUtf8Json"].encode("utf-8")
        link["candidateRawSha256"] = hashlib.sha256(forged_payload).hexdigest()
        link["candidateByteCount"] = len(forged_payload)
        reseal(forged_candidate)
        mutations.append(forged_candidate)

        forged_tool = copy.deepcopy(entry)
        forged_tool["publishReceipt"]["toolDependencies"][1]["sha256"] = "2" * 64
        reseal(forged_tool)
        mutations.append(forged_tool)

        for forged in mutations:
            catalog = {
                "schema": "lostark.effect-runtime-catalog",
                "formatVersion": 3,
                "components": [],
                "effects": [forged],
            }
            with self.assertRaises(derived.ContractError):
                derived.validate_runtime_catalog(catalog)

    def test_render_resource_sidecar_eol_object_and_coordinated_reseals_reject(
        self,
    ) -> None:
        crlf_sidecar = self.root / "render-resource-sidecar.json"
        crlf_sidecar.write_bytes(
            RECONSTRUCTED_SIDECAR.read_bytes().replace(b"\n", b"\r\n")
        )
        with self.assertRaisesRegex(derived.ContractError, "byte count|LF-only"):
            derived.prepare_reconstructed_runtime_entry(
                RECONSTRUCTED_CANDIDATE, crlf_sidecar
            )

        entry = derived.prepare_reconstructed_runtime_entry(
            RECONSTRUCTED_CANDIDATE, RECONSTRUCTED_SIDECAR
        )

        def reseal_bridge(row: dict) -> None:
            resource_link = row["reconstructedRenderResourceAuthority"]
            receipt = row["renderResourcePublishReceipt"]
            receipt["effectAssetId"] = resource_link["effectAssetId"]
            receipt["programId"] = resource_link["programId"]
            receipt["programVersion"] = resource_link["programVersion"]
            receipt["programSha256"] = resource_link["programSha256"]
            receipt["sidecarRawSha256"] = resource_link["sidecarRawSha256"]
            receipt["sidecarReceiptSha256"] = resource_link[
                "sidecarReceiptSha256"
            ]
            receipt["sidecarDecisionProjectionSha256"] = resource_link[
                "sidecarDecisionProjectionSha256"
            ]
            receipt["renderResourceAuthorityLinkSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(resource_link)
            ).hexdigest()
            unsigned = dict(receipt)
            del unsigned["receiptSha256"]
            receipt["receiptSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(unsigned)
            ).hexdigest()
            row["renderResourcePublishReceiptSha256"] = hashlib.sha256(
                derived.canonical_json_bytes(receipt)
            ).hexdigest()

        coordinated_link = copy.deepcopy(entry)
        resource_link = coordinated_link["reconstructedRenderResourceAuthority"]
        resource_link["sidecarRawSha256"] = "3" * 64
        reseal_bridge(coordinated_link)

        coordinated_self = copy.deepcopy(entry)
        coordinated_self["reconstructedRenderResourceAuthority"][
            "sidecarReceiptSha256"
        ] = "5" * 64
        reseal_bridge(coordinated_self)

        coordinated_decision = copy.deepcopy(entry)
        coordinated_decision["reconstructedRenderResourceAuthority"][
            "sidecarDecisionProjectionSha256"
        ] = "6" * 64
        reseal_bridge(coordinated_decision)

        coordinated_object = copy.deepcopy(entry)
        resource_link = coordinated_object["reconstructedRenderResourceAuthority"]
        resource_link["sidecarUtf8Json"] = resource_link[
            "sidecarUtf8Json"
        ].replace('"inputSlot": "F"', '"inputSlot": "Q"', 1)
        mutated_payload = resource_link["sidecarUtf8Json"].encode("utf-8")
        resource_link["sidecarByteCount"] = len(mutated_payload)
        resource_link["sidecarRawSha256"] = hashlib.sha256(
            mutated_payload
        ).hexdigest()
        mutated_sidecar = json.loads(resource_link["sidecarUtf8Json"])
        resource_link["sidecarReceiptSha256"] = mutated_sidecar["receiptSha256"]
        resource_link["sidecarDecisionProjectionSha256"] = mutated_sidecar[
            "decisionProjectionSha256"
        ]
        reseal_bridge(coordinated_object)

        coordinated_receipt = copy.deepcopy(entry)
        receipt = coordinated_receipt["renderResourcePublishReceipt"]
        receipt["baseRuntimeEntryProjectionSha256"] = "4" * 64
        unsigned = dict(receipt)
        del unsigned["receiptSha256"]
        receipt["receiptSha256"] = hashlib.sha256(
            derived.canonical_json_bytes(unsigned)
        ).hexdigest()
        coordinated_receipt["renderResourcePublishReceiptSha256"] = (
            hashlib.sha256(derived.canonical_json_bytes(receipt)).hexdigest()
        )

        for forged in (
            coordinated_link,
            coordinated_self,
            coordinated_decision,
            coordinated_object,
            coordinated_receipt,
        ):
            with self.assertRaises(derived.ContractError):
                derived.validate_reconstructed_runtime_entry(forged)

    def test_render_resource_sidecar_is_read_once_and_same_buffer_is_embedded(
        self,
    ) -> None:
        approved = RECONSTRUCTED_SIDECAR.read_bytes()
        forged = approved.replace(b'"inputSlot": "F"', b'"inputSlot": "Q"', 1)

        class SplitReadSidecar:
            def __init__(self) -> None:
                self.read_count = 0

            def read_bytes(self) -> bytes:
                self.read_count += 1
                return approved if self.read_count == 1 else forged

            def __str__(self) -> str:
                return "split-read-sidecar"

        split = SplitReadSidecar()
        entry = derived.prepare_reconstructed_runtime_entry(
            RECONSTRUCTED_CANDIDATE, split  # type: ignore[arg-type]
        )
        self.assertEqual(split.read_count, 1)
        self.assertEqual(
            entry["reconstructedRenderResourceAuthority"][
                "sidecarUtf8Json"
            ].encode("utf-8"),
            approved,
        )

    def test_render_resource_bridge_rejects_missing_extra_reorder_and_types(
        self,
    ) -> None:
        entry = derived.prepare_reconstructed_runtime_entry(
            RECONSTRUCTED_CANDIDATE, RECONSTRUCTED_SIDECAR
        )
        extra_outer = copy.deepcopy(entry)
        extra_outer["unknownAdmission"] = False

        missing_link = copy.deepcopy(entry)
        del missing_link["reconstructedRenderResourceAuthority"]["renderAdmission"]

        reordered_link = copy.deepcopy(entry)
        link = reordered_link["reconstructedRenderResourceAuthority"]
        reordered_link["reconstructedRenderResourceAuthority"] = {
            "effectAssetId": link["effectAssetId"],
            **{key: value for key, value in link.items() if key != "effectAssetId"},
        }

        bool_version = copy.deepcopy(entry)
        bool_version["reconstructedRenderResourceAuthority"]["formatVersion"] = True

        integer_gate = copy.deepcopy(entry)
        integer_gate["reconstructedRenderResourceAuthority"][
            "executeAdmission"
        ] = 0

        extra_receipt = copy.deepcopy(entry)
        extra_receipt["renderResourcePublishReceipt"]["catalogSha256"] = "0" * 64

        for forged in (
            extra_outer,
            missing_link,
            reordered_link,
            bool_version,
            integer_gate,
            extra_receipt,
        ):
            with self.assertRaises(derived.ContractError):
                derived.validate_reconstructed_runtime_entry(forged)

    def test_public_reconstructed_validators_reject_coordinated_tool_reseal(
        self,
    ) -> None:
        entry = derived.prepare_reconstructed_runtime_entry(
            RECONSTRUCTED_CANDIDATE, RECONSTRUCTED_SIDECAR
        )
        for dependency_index in range(3):
            with self.subTest(dependency_index=dependency_index):
                forged = copy.deepcopy(entry)
                receipt = forged["renderResourcePublishReceipt"]
                receipt["toolDependencies"][dependency_index]["sha256"] = (
                    "2" * 64
                )
                unsigned = dict(receipt)
                del unsigned["receiptSha256"]
                receipt["receiptSha256"] = hashlib.sha256(
                    derived.canonical_json_bytes(unsigned)
                ).hexdigest()
                forged["renderResourcePublishReceiptSha256"] = hashlib.sha256(
                    derived.canonical_json_bytes(receipt)
                ).hexdigest()
                base = {
                    key: forged[key]
                    for key in derived.RECONSTRUCTED_BASE_ENTRY_KEYS
                }

                with self.assertRaisesRegex(
                    derived.ContractError, "current source hash"
                ):
                    derived.validate_reconstructed_render_resource_publish_receipt(
                        receipt,
                        base,
                        forged["reconstructedRenderResourceAuthority"],
                    )
                with self.assertRaisesRegex(
                    derived.ContractError, "current source hash"
                ):
                    derived.validate_reconstructed_runtime_entry(forged)

    def test_reserved_artist_runtime_id_rejects_legacy_and_generic_payloads(
        self,
    ) -> None:
        reserved = derived.RECONSTRUCTED_EFFECT_ID
        outputs = self._build_generic_bundle_for_effect(
            self.root / "ReservedRuntime", reserved
        )
        generic = derived.prepare_runtime_entry(
            outputs["authoring"],
            outputs["assembly"],
            outputs["artifact"],
            outputs["receipt"],
        )
        legacy = {
            "payloadKind": derived.LEGACY_PAYLOAD_KIND,
            "effectAssetId": reserved,
            "authoringFormatVersion": 12,
            "contentSha256": "0" * 64,
            "dependencies": [],
            "assembly": {},
        }
        for entry in (legacy, generic):
            catalog = {
                "schema": "lostark.effect-runtime-catalog",
                "formatVersion": 3,
                "components": [],
                "effects": [entry],
            }
            with self.assertRaisesRegex(
                derived.ContractError, "reserved Artist 31470"
            ):
                derived.validate_runtime_catalog(catalog)

    def test_reconstructed_publisher_rolls_back_and_embedded_load_needs_no_candidate(
        self,
    ) -> None:
        data_root, resources, output, candidate = (
            self._build_reconstructed_publisher_fixture()
        )
        result = self._run_publisher(data_root, resources, output)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        runtime = derived.load_json(output)
        direct_runtime.validate_runtime_catalog(runtime, output)
        self.assertEqual(runtime["formatVersion"], 4)
        self.assertEqual(runtime["materialPrograms"]["bindings"], [])
        self.assertEqual(len(runtime["effects"]), 1)
        self.assertEqual(
            runtime["effects"][0]["payloadKind"],
            derived.RECONSTRUCTED_PAYLOAD_KIND,
        )
        committed = output.read_bytes()
        visual_output = output.with_name("EffectVisualPrograms.runtime.json")
        committed_visual = visual_output.read_bytes()
        self.assertEqual(committed_visual, VISUAL_PROGRAM_SIDECAR.read_bytes())
        self.assertTrue(committed.endswith(b"\n"))
        self.assertNotIn(b"\r", committed)
        crlf_catalog = output.with_name("EffectCatalog.runtime.crlf.json")
        crlf_catalog.write_bytes(committed.replace(b"\n", b"\r\n"))
        crlf_validation = subprocess.run(
            [
                sys.executable,
                "-B",
                str(MODULE_PATH),
                "validate-runtime-catalog",
                "--catalog",
                str(crlf_catalog),
            ],
            text=True,
            capture_output=True,
            cwd=REPO_ROOT,
        )
        self.assertNotEqual(crlf_validation.returncode, 0)
        self.assertIn(
            "LF-only", crlf_validation.stderr + crlf_validation.stdout
        )
        for fault in (
            "AfterBackupMove", "AfterCommitMove", "AfterSidecarCommitMove"
        ):
            faulted = self._run_publisher(
                data_root, resources, output, fault=fault
            )
            self.assertNotEqual(faulted.returncode, 0)
            self.assertEqual(output.read_bytes(), committed)
            self.assertEqual(visual_output.read_bytes(), committed_visual)
            self.assertEqual(list(output.parent.glob("*.tmp")), [])
            self.assertEqual(list(output.parent.glob("*.bak")), [])

        sidecar = data_root / (
            "Effects/Imported/Artist/Materials/"
            "skill.31470.reconstructed-render-resource-authority.receipt.json"
        )
        candidate.unlink()
        sidecar.unlink()
        direct_runtime.validate_runtime_catalog(runtime, output)
        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertEqual(output.read_bytes(), committed)
        self.assertEqual(visual_output.read_bytes(), committed_visual)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_reconstructed_publisher_rejects_source_identity_substitution_without_overwrite(
        self,
    ) -> None:
        data_root, resources, output, _ = (
            self._build_reconstructed_publisher_fixture()
        )
        baseline = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline.returncode, 0, baseline.stderr + baseline.stdout)
        committed_runtime = output.read_bytes()

        source_catalog_path = data_root / "Effects" / "EffectCatalog.json"
        source_catalog = derived.load_json(source_catalog_path)
        source_catalog["effects"][0]["effectAssetId"] = (
            "effect.artist.skill.31471"
        )
        write_json(source_catalog_path, source_catalog)
        substituted_source = source_catalog_path.read_bytes()

        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn(
            "effectAssetId must be effect.artist.skill.31470",
            rejected.stderr + rejected.stdout,
        )
        self.assertEqual(source_catalog_path.read_bytes(), substituted_source)
        self.assertEqual(output.read_bytes(), committed_runtime)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_reconstructed_publisher_rejects_sidecar_missing_and_escape_before_write(
        self,
    ) -> None:
        data_root, resources, output, _ = (
            self._build_reconstructed_publisher_fixture()
        )
        baseline = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline.returncode, 0, baseline.stderr + baseline.stdout)
        committed_runtime = output.read_bytes()
        source_catalog_path = data_root / "Effects" / "EffectCatalog.json"
        approved_source = source_catalog_path.read_bytes()
        sidecar = data_root / (
            "Effects/Imported/Artist/Materials/"
            "skill.31470.reconstructed-render-resource-authority.receipt.json"
        )

        sidecar.unlink()
        missing = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(missing.returncode, 0)
        self.assertEqual(source_catalog_path.read_bytes(), approved_source)
        self.assertEqual(output.read_bytes(), committed_runtime)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

        shutil.copyfile(RECONSTRUCTED_SIDECAR, sidecar)
        source = derived.load_json(source_catalog_path)
        source["effects"][0]["reconstructedRenderResourceAuthorityPath"] = (
            "Effects/Imported/Artist/Materials/../"
            "skill.31470.reconstructed-render-resource-authority.receipt.json"
        )
        write_json(source_catalog_path, source)
        escaped_source = source_catalog_path.read_bytes()
        escaped = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(escaped.returncode, 0)
        self.assertIn("Unsafe derived Effect path segment", escaped.stderr + escaped.stdout)
        self.assertEqual(source_catalog_path.read_bytes(), escaped_source)
        self.assertEqual(output.read_bytes(), committed_runtime)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_reconstructed_publisher_rejects_reserved_generic_fallback_without_overwrite(
        self,
    ) -> None:
        data_root, resources, output, _ = (
            self._build_reconstructed_publisher_fixture()
        )
        baseline = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline.returncode, 0, baseline.stderr + baseline.stdout)
        committed_runtime = output.read_bytes()
        reserved = derived.RECONSTRUCTED_EFFECT_ID
        self._build_generic_bundle_for_effect(data_root, reserved)
        source_catalog_path = data_root / "Effects" / "EffectCatalog.json"
        write_json(
            source_catalog_path,
            {
                "formatVersion": 1,
                "effects": [
                    {
                        "effectAssetId": reserved,
                        "authoringPath": (
                            f"Effects/Authored/{reserved}.effect.json"
                        ),
                        "compiledArtifactPath": (
                            f"Effects/Compiled/{reserved}.compiled-effect.json"
                        ),
                        "compiledReceiptPath": (
                            "Effects/Compiled/"
                            f"{reserved}.compiled-effect.receipt.json"
                        ),
                    }
                ],
            },
        )
        fallback_source = source_catalog_path.read_bytes()

        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn(
            "Reserved Artist 31470 source entry must use the reconstructed payload kind",
            rejected.stderr + rejected.stdout,
        )
        self.assertEqual(source_catalog_path.read_bytes(), fallback_source)
        self.assertEqual(output.read_bytes(), committed_runtime)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_reconstructed_publisher_rejects_nested_duplicate_owner_keys_without_overwrite(
        self,
    ) -> None:
        data_root, resources, output, _ = (
            self._build_reconstructed_publisher_fixture()
        )
        baseline = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline.returncode, 0, baseline.stderr + baseline.stdout)
        committed_runtime = output.read_bytes()
        source_catalog_path = data_root / "Effects" / "EffectCatalog.json"
        path = (
            "Effects/Imported/Artist/Candidates/"
            "skill.31470.reconstructed-runtime-program.candidate.json"
        )

        for duplicate_key in ("effectAssetId", "\\u0065ffectAssetId"):
            with self.subTest(duplicate_key=duplicate_key):
                duplicate_source = (
                    '{"formatVersion":1,"effects":[{'
                    '"effectAssetId":"effect.artist.skill.31471",'
                    f'"{duplicate_key}":"effect.artist.skill.31470",'
                    '"payloadKind":"IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM",'
                    f'"reconstructedRuntimeProgramPath":"{path}"'
                    '}]}\n'
                ).encode("utf-8")
                source_catalog_path.write_bytes(duplicate_source)
                rejected = self._run_publisher(data_root, resources, output)
                self.assertNotEqual(rejected.returncode, 0)
                self.assertIn(
                    "duplicate JSON object key 'effectAssetId'",
                    rejected.stderr + rejected.stdout,
                )
                self.assertEqual(source_catalog_path.read_bytes(), duplicate_source)
                self.assertEqual(output.read_bytes(), committed_runtime)
                self.assertEqual(list(output.parent.glob("*.tmp")), [])
                self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_duplicate_scanner_walks_nested_mixed_token_corpus_without_overwrite(
        self,
    ) -> None:
        data_root, resources, output, _ = (
            self._build_reconstructed_publisher_fixture()
        )
        baseline = self._run_publisher(data_root, resources, output)
        self.assertEqual(baseline.returncode, 0, baseline.stderr + baseline.stdout)
        committed_runtime = output.read_bytes()
        source_catalog_path = data_root / "Effects" / "EffectCatalog.json"
        path = (
            "Effects/Imported/Artist/Candidates/"
            "skill.31470.reconstructed-runtime-program.candidate.json"
        )
        nested_duplicate_source = (
            '{"formatVersion":1,"scannerProbe":{'
            '"escaped":"quote \\" braces {} brackets [] slash \\\\ unicode \\u0041",'
            '"fractionExponent":-1.25e+2,'
            '"truth":true,"falsehood":false,"nothing":null,'
            '"nested":[{"owner":1,"\\u006fwner":2}]},'
            '"effects":[{'
            '"effectAssetId":"effect.artist.skill.31470",'
            '"payloadKind":"IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM",'
            f'"reconstructedRuntimeProgramPath":"{path}"'
            '}]}\n'
        ).encode("utf-8")
        source_catalog_path.write_bytes(nested_duplicate_source)

        rejected = self._run_publisher(data_root, resources, output)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn(
            "duplicate JSON object key 'owner'",
            rejected.stderr + rejected.stdout,
        )
        self.assertEqual(
            source_catalog_path.read_bytes(), nested_duplicate_source
        )
        self.assertEqual(output.read_bytes(), committed_runtime)
        self.assertEqual(list(output.parent.glob("*.tmp")), [])
        self.assertEqual(list(output.parent.glob("*.bak")), [])

    def test_publisher_format4_round_trip_keeps_product_false(self) -> None:
        data_root, resources, output, _ = self._build_publisher_fixture()
        result = self._run_publisher(data_root, resources, output)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        runtime = derived.load_json(output)
        direct_runtime.validate_runtime_catalog(runtime, output)
        self.assertEqual(runtime["formatVersion"], 4)
        self.assertEqual(runtime["materialPrograms"]["bindings"], [])
        self.assertEqual(len(runtime["effects"]), 1)
        self.assertFalse(runtime["effects"][0]["productAdmission"])
        self.assertEqual(runtime["effects"][0]["payloadKind"], "IMMUTABLE_COMPILED_IR")
        committed = output.read_bytes()
        visual_output = output.with_name("EffectVisualPrograms.runtime.json")
        committed_visual = visual_output.read_bytes()
        self.assertEqual(committed_visual, VISUAL_PROGRAM_SIDECAR.read_bytes())
        for fault in (
            "AfterBackupMove", "AfterCommitMove", "AfterSidecarCommitMove"
        ):
            faulted = self._run_publisher(
                data_root, resources, output, fault=fault
            )
            self.assertNotEqual(faulted.returncode, 0)
            self.assertEqual(output.read_bytes(), committed)
            self.assertEqual(visual_output.read_bytes(), committed_visual)
            self.assertEqual(list(output.parent.glob("*.tmp")), [])
            self.assertEqual(list(output.parent.glob("*.bak")), [])
        validate_result = self._run_publisher(
            data_root, resources, output, mode="Validate"
        )
        self.assertEqual(
            validate_result.returncode,
            0,
            validate_result.stderr + validate_result.stdout,
        )
        self.assertEqual(output.read_bytes(), committed)
        self.assertEqual(visual_output.read_bytes(), committed_visual)

        visual_source = data_root / (
            "Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
        )
        source_bytes = visual_source.read_bytes()
        visual_source.unlink()
        missing = self._run_publisher(
            data_root, resources, output, mode="Validate"
        )
        self.assertNotEqual(missing.returncode, 0)
        self.assertEqual(output.read_bytes(), committed)
        self.assertEqual(visual_output.read_bytes(), committed_visual)

        visual_source.parent.mkdir(parents=True, exist_ok=True)
        visual_source.write_bytes(source_bytes)
        changed_visual = json.loads(source_bytes.decode("utf-8"))
        changed_visual["denominators"]["visualRowCount"] += 1
        write_json(visual_source, changed_visual)
        stale = self._run_publisher(
            data_root, resources, output, mode="Validate"
        )
        self.assertNotEqual(stale.returncode, 0)
        self.assertIn(
            "runtime publish artifact.artifactSha256 is stale",
            stale.stderr + stale.stdout,
        )
        self.assertEqual(output.read_bytes(), committed)
        self.assertEqual(visual_output.read_bytes(), committed_visual)

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
