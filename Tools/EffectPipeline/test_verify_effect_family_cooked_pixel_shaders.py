import collections
import hashlib
import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from Tools.EffectPipeline import build_effect_family_shader_map_index as map_index
from Tools.EffectPipeline import extract_effect_family_cooked_pixel_shaders as extractor
from Tools.EffectPipeline import verify_effect_family_cooked_pixel_shaders as verifier


def canonical_sha256(value):
    payload = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()


def raw_sha256(payload):
    return hashlib.sha256(payload).hexdigest()


def attach_artifact(document):
    document = dict(document)
    document.pop("artifactSha256", None)
    document["artifactSha256"] = canonical_sha256(document)
    return document


def attach_row_sha(row):
    row = dict(row)
    row.pop("rowSha256", None)
    row["rowSha256"] = canonical_sha256(row)
    return row


def write_json_lf(path, document):
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(
            document, indent=2, ensure_ascii=False) + "\n")


class ContractPinWriterTests(unittest.TestCase):
    def test_shader_map_index_pins_raw_inputs_and_writes_lf(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            manifest = root / "source_pack_manifest.json"
            cache = root / "cache.upk"
            manifest.write_bytes(b'{"packages":[]}\n')
            cache.write_bytes(b"pinned-cache-bytes")
            parents = {
                "fx.parent": {
                    "occurrenceCount": 3,
                    "classes": collections.Counter({"artist": 3}),
                }
            }
            resolutions = {
                "fx.parent": {
                    "resolvedBy": map_index.RESOLVED_DECLARED,
                    "baseMaterialIdHex": "00" * 16,
                }
            }
            scans = {
                "00" * 16: {
                    "rawHitCount": 1,
                    "materialMapContextCount": 1,
                    "materialMapContexts": [],
                }
            }
            with (
                mock.patch.object(
                    map_index, "collect_parent_materials",
                    return_value=parents),
                mock.patch.object(
                    map_index, "load_package_index",
                    return_value={"fx": root / "fx.upk"}),
                mock.patch.object(
                    map_index, "resolve_parents",
                    return_value=(
                        resolutions,
                        {map_index.RESOLVED_DECLARED: 1})),
                mock.patch.object(
                    map_index, "scan_cache", return_value=scans),
            ):
                document = map_index.build_index(
                    manifest, cache, verbose=False)

            inputs = document["inputs"]
            self.assertEqual(
                inputs["sourcePackManifestRawSha256"],
                raw_sha256(manifest.read_bytes()))
            self.assertEqual(
                inputs["sourcePackManifestByteSize"],
                len(manifest.read_bytes()))
            self.assertEqual(
                inputs["refShaderCacheRawSha256"],
                raw_sha256(cache.read_bytes()))
            self.assertEqual(
                inputs["refShaderCacheByteSize"], len(cache.read_bytes()))

            output = root / "map.json"
            map_index.write_index(output, document)
            self.assertNotIn(b"\r\n", output.read_bytes())

    def test_cooked_receipt_pins_shader_map_and_cache_and_writes_lf(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            cache = root / "cache.upk"
            cache.write_bytes(b"cache")
            rows = [{
                "parentMaterialPath": "fx.parent",
                "occurrenceCount": 1,
                "status": "BLOCKED",
                "blocker": "missing permutation",
            }]
            document = extractor.build_receipt(
                rows,
                {"artifactSha256": "a" * 64},
                {"rawSha256": "b" * 64, "byteSize": 123},
                cache,
                {"rawSha256": raw_sha256(b"cache"), "byteSize": 5},
            )
            inputs = document["inputs"]
            self.assertEqual(inputs["shaderMapArtifactSha256"], "a" * 64)
            self.assertEqual(inputs["shaderMapRawSha256"], "b" * 64)
            self.assertEqual(
                inputs["refShaderCacheRawSha256"], raw_sha256(b"cache"))
            self.assertEqual(inputs["refShaderCacheByteSize"], 5)

            output = root / "receipt.json"
            extractor.write_receipt(output, document)
            self.assertNotIn(b"\r\n", output.read_bytes())


class CookedReceiptVerifierTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.manifest = self.root / "source_pack_manifest.json"
        self.cache = self.root / "cache.upk"
        self.shader_map = self.root / "shader-map.json"
        self.receipt = self.root / "cooked.json"
        self.blobs = self.root / "blobs"
        self.blobs.mkdir()
        self.manifest.write_bytes(b'{"packages":[]}\n')
        self.cache.write_bytes(b"cache-payload")
        self.blob_payload = b"DXBC\x00\x01\x02\x03"
        self.blob_sha = raw_sha256(self.blob_payload)
        (self.blobs / f"{self.blob_sha}.dxbc").write_bytes(
            self.blob_payload)
        self._write_valid_contracts()

    def tearDown(self):
        self.temporary.cleanup()

    def _write_valid_contracts(self):
        rows = []
        for parent, occurrence, base_id in (
            ("fx.extracted", 2, "00" * 16),
            ("fx.blocked", 3, "11" * 16),
        ):
            rows.append(attach_row_sha({
                "familyId": "family-" + parent,
                "parentMaterialPath": parent,
                "occurrenceCount": occurrence,
                "classes": {"artist": occurrence},
                "resolution": {
                    "resolvedBy": "DECLARED_PACKAGE_EXPORT",
                    "baseMaterialIdHex": base_id,
                },
                "cookedEvidence": "COOKED_MATERIAL_MAPS_PRESENT",
                "cacheScan": {
                    "rawHitCount": 1,
                    "materialMapContextCount": 1,
                    "materialMapContexts": [],
                },
            }))
        shader_map = attach_artifact({
            "schema": "lostark.effect-family-shader-map-index",
            "formatVersion": 1,
            "identity": {"admits": "NOTHING"},
            "inputs": {
                "sourcePackManifest": str(self.manifest),
                "sourcePackManifestRawSha256":
                    raw_sha256(self.manifest.read_bytes()),
                "sourcePackManifestByteSize":
                    len(self.manifest.read_bytes()),
                "refShaderCacheFileName": self.cache.name,
                "refShaderCacheRawSha256":
                    raw_sha256(self.cache.read_bytes()),
                "refShaderCacheByteSize": len(self.cache.read_bytes()),
            },
            "summary": {
                "parentMaterialCount": 2,
                "occurrenceCount": 5,
                "resolutionCounts": {"DECLARED_PACKAGE_EXPORT": 2},
                "distinctBaseMaterialIdCount": 2,
                "cookedEvidenceCounts": {
                    "COOKED_MATERIAL_MAPS_PRESENT": 2,
                },
                "materialMapContextCount": 2,
            },
            "families": rows,
        })
        write_json_lf(self.shader_map, shader_map)
        map_raw = self.shader_map.read_bytes()
        receipt = attach_artifact({
            "schema": "lostark.effect-family-cooked-pixel-shaders",
            "formatVersion": 1,
            "identity": {"admits": "COOKED_PROGRAM_ONLY"},
            "inputs": {
                "shaderMapIndex": str(self.shader_map),
                "shaderMapArtifactSha256": shader_map["artifactSha256"],
                "shaderMapRawSha256": raw_sha256(map_raw),
                "refShaderCacheFileName": self.cache.name,
                "refShaderCacheRawSha256":
                    raw_sha256(self.cache.read_bytes()),
                "refShaderCacheByteSize": len(self.cache.read_bytes()),
                "blobDirectory": str(self.blobs),
            },
            "summary": {
                "familyCount": 2,
                "extractedCount": 1,
                "blockedCount": 1,
                "extractedOccurrenceCount": 2,
                "blockerCounts": {"no permutation": 1},
            },
            "families": [{
                "parentMaterialPath": "fx.extracted",
                "occurrenceCount": 2,
                "status": "EXTRACTED",
                "dxbcSha256": self.blob_sha,
                "dxbcByteSize": len(self.blob_payload),
            }, {
                "parentMaterialPath": "fx.blocked",
                "occurrenceCount": 3,
                "status": "BLOCKED",
                "blocker": "no permutation: exact static set absent",
            }],
        })
        write_json_lf(self.receipt, receipt)

    def _verify(self):
        return verifier.verify(
            self.receipt, self.shader_map, self.manifest, self.cache,
            self.blobs)

    def _mutate_contract(self, path, mutation):
        document = json.loads(path.read_text(encoding="utf-8"))
        mutation(document)
        write_json_lf(path, attach_artifact(document))

    def test_valid_chain_passes(self):
        self.assertEqual(self._verify(), {
            "familyCount": 2,
            "extractedCount": 1,
            "blockedCount": 1,
        })

    def test_schema_artifact_and_summary_drift_fail_closed(self):
        mutations = (
            lambda document: document.__setitem__("schema", "wrong"),
            lambda document: document["summary"].__setitem__(
                "extractedCount", 2),
        )
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                self._write_valid_contracts()
                self._mutate_contract(self.receipt, mutation)
                with self.assertRaises(verifier.VerificationError):
                    self._verify()
        self._write_valid_contracts()
        document = json.loads(self.receipt.read_text(encoding="utf-8"))
        document["artifactSha256"] = "f" * 64
        write_json_lf(self.receipt, document)
        with self.assertRaises(verifier.VerificationError):
            self._verify()

    def test_blob_raw_sha_and_byte_size_drift_fail_closed(self):
        blob_path = self.blobs / f"{self.blob_sha}.dxbc"
        blob_path.write_bytes(self.blob_payload + b"drift")
        with self.assertRaises(verifier.VerificationError):
            self._verify()

    def test_duplicate_parent_and_blocked_contradiction_fail_closed(self):
        mutations = (
            lambda document: document["families"][1].__setitem__(
                "parentMaterialPath", "fx.extracted"),
            lambda document: document["families"][1].__setitem__(
                "dxbcSha256", "0" * 64),
        )
        for mutation in mutations:
            with self.subTest(mutation=mutation):
                self._write_valid_contracts()
                self._mutate_contract(self.receipt, mutation)
                with self.assertRaises(verifier.VerificationError):
                    self._verify()

    def test_current_upstream_raw_drift_fails_closed(self):
        drifts = ("manifest", "cache", "shader-map")
        for drift in drifts:
            with self.subTest(drift=drift):
                self._write_valid_contracts()
                if drift == "manifest":
                    self.manifest.write_bytes(
                        self.manifest.read_bytes() + b" ")
                elif drift == "cache":
                    self.cache.write_bytes(self.cache.read_bytes() + b" ")
                else:
                    self.shader_map.write_bytes(
                        self.shader_map.read_bytes() + b" ")
                with self.assertRaises(verifier.VerificationError):
                    self._verify()


if __name__ == "__main__":
    unittest.main()
