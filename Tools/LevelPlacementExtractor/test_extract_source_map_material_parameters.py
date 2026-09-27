import hashlib
from pathlib import Path
import struct
from types import SimpleNamespace
import unittest
from unittest import mock

import extract_source_map_material_parameters as subject


class SourceMapMaterialTests(unittest.TestCase):
    def test_native_normal29_and_numbered_fname_preserve_original_identity(self):
        base, guid = bytes(range(16)), bytes([35]) * 16
        switch = struct.pack("<iiII", 0, 2, 0, 1) + guid
        normal = struct.pack("<iiBI", 1, 0, 3, 1) + guid
        static = base + struct.pack("<I", 1) + switch + struct.pack("<II", 0, 1) + normal + bytes(4)
        tail = b"head" + static + b"suffix"
        decoded = subject.MapStaticSetDecoder().decode(tail, base, ["switch", "normal"])
        row = decoded["staticParameterSet"]["staticSwitchParameters"][0]
        self.assertEqual(row["parameterName"], "switch_1")
        self.assertEqual(row["sourceFNameNumber"], 2)
        self.assertEqual(row["entryOffset"], 24)
        self.assertIs(row["value"], False)
        normal_row = decoded["staticParameterSet"]["normalParameters"][0]
        self.assertEqual(normal_row["valueOrdinalCandidate"], 3)
        self.assertEqual(decoded["staticParameterSet"]["byteSize"], len(static))
        self.assertEqual(decoded["staticParameterSet"]["rawSha256"], hashlib.sha256(static).hexdigest())
        self.assertEqual(decoded["nativeTailByteCount"], len(tail))

    def test_unknown_static_format_and_missing_resource_fail(self):
        decoder = subject.MapStaticSetDecoder()
        base = bytes(range(16))
        for tail in (b"", b"wrong ID", base + struct.pack("<I", 4097), base + bytes(10)):
            with self.subTest(tail=tail):
                with self.assertRaises(ValueError):
                    decoder.decode(tail, base, ["one"])

    def test_child_explicit_zero_false_null_replace_parent(self):
        row = {"values": {"brightness": 1.0}, "switches": {"enabled": True},
               "textures": {"orm": "base.tex.orm"},
               "parameterSources": {"values": {}, "switches": {}, "textures": {}},
               "unresolvedDefaults": [{"field": "values", "name": "brightness"}]}
        for field, name, value in (("values", "brightness", 0.0), ("switches", "enabled", False), ("textures", "orm", None)):
            subject.assign(row, field, name, value, {"sourceMaterial": "child.mat.mic"})
            self.assertEqual(row[field][name], value)
        self.assertEqual(row["unresolvedDefaults"], [])

    def test_full_package_resolution_distinguishes_local_external_and_null(self):
        package = SimpleNamespace(imports=[], exports=[])
        with mock.patch.object(subject.ue, "package_ref_path", side_effect=["tex.local", "external.tex.import"]):
            self.assertEqual(subject.source_reference(package, "owner", 1), "owner.tex.local")
            self.assertEqual(subject.source_reference(package, "owner", -1), "external.tex.import")
            self.assertIsNone(subject.source_reference(package, "owner", 0))

    def test_parent_cycle_is_detected_before_partial_result_is_cached(self):
        entries = [SimpleNamespace(index=i, class_index=-1, serial_offset=i, serial_size=1) for i in range(2)]
        package = SimpleNamespace(path=Path("source.upk"), exports=entries, imports=[], names=[],
                                  logical=b"ab", summary=SimpleNamespace(version=868))
        resolver = subject.MaterialResolver.__new__(subject.MaterialResolver)
        resolver.materials, resolver.active = {}, []
        resolver.package = lambda _: package
        def properties(raw, _names, _version):
            return {"parent": {"value": 2 if raw == b"a" else 1}}, 0
        with mock.patch.object(subject.ue, "package_ref_path", side_effect=lambda index, *_: "mat.a" if index == 1 else "mat.b"), \
             mock.patch.object(subject.ue, "package_ref_name", return_value="materialinstanceconstant"), \
             mock.patch.object(subject.ue, "parse_tagged_properties", side_effect=properties):
            with self.assertRaisesRegex(ValueError, "parent cycle"):
                resolver.resolve("package.mat.a")
        self.assertEqual(resolver.materials, {})
        self.assertEqual(resolver.active, [])

    def test_partial_mic_parameter_decode_cannot_be_silently_skipped(self):
        props = {"scalarparametervalues": {"value": [{"malformed": True}]}}
        decoded = {"scalarParameters": [], "vectorParameters": [], "textureParameters": []}
        with self.assertRaisesRegex(ValueError, "undecoded MIC parameter"):
            subject.MaterialResolver.validate_instance_rows(props, decoded, "package.mat.mic")

    def test_empty_resource_needs_pinned_parent_map_and_rejects_source_changes(self):
        base = bytes(range(1, 17))
        parsed = subject.shader_maps.parse_static_parameter_set(base + bytes(16), 0, [])
        equality = subject.shader_maps.canonical_json_sha256(
            subject.shader_maps.engine_equivalent_static_parameter_set(parsed))
        full, parent, serial = "package.mat.child", "package.mat.base", b"original MIC"
        package = SimpleNamespace(names=[], sha256="original-package")
        resolved = {"baseId": base.hex(), "switches": {}, "unresolvedDefaults": []}
        props = {"parent": {"value": 1}, "textureparametervalues": {"value": []}}
        resolver = subject.MaterialResolver.__new__(subject.MaterialResolver)
        resolver.parent_static_proofs = {}
        resolver.parent_static_evidence = {"path": "reviewed-proof.json", "sha256": "pinned"}
        with self.assertRaisesRegex(ValueError, "absent"):
            resolver.inherited_static(full, parent, package, serial, b"", props, resolved)
        proof = dict(sourceMaterial=full, parentMaterial=parent,
            sourceSerialSha256=subject.digest(serial), sourcePackageSha256=package.sha256,
            baseMaterialId=base.hex(), sourceNativeTailBytes=0,
            allowedPropertyNames=sorted(props), engineEqualityStaticParameterSetSha256=equality,
            nativeShaderMapContext={"engineEqualityStaticParameterSetSha256": equality},
            sourceLocalVertexFactoryShaders=[{"shaderIdHex": "source-qualified"}])
        resolver.parent_static_proofs[full] = proof
        value = resolver.inherited_static(full, parent, package, serial, b"", props, resolved)
        self.assertEqual(value["status"], "SOURCE_PROVED_PARENT_MAP_INHERITANCE")
        for changed in (
            (serial + b"changed", b"", props, resolved),
            (serial, b"native resource", props, resolved),
            (serial, b"", dict(props, bhasstaticpermutationresource={"value": True}), resolved),
            (serial, b"", props, dict(resolved, switches={"enabled": False})),
        ):
            with self.subTest(changed=changed):
                with self.assertRaises(ValueError):
                    resolver.inherited_static(full, parent, package, *changed)
        proof["nativeShaderMapContext"]["engineEqualityStaticParameterSetSha256"] = "other-key"
        with self.assertRaisesRegex(ValueError, "shader-map key"):
            resolver.inherited_static(full, parent, package, serial, b"", props, resolved)


class MicNullTextureOverrideTest(unittest.TestCase):
    """packageIndex 0 is a serialized NULL reference and must not erase an inherited texture."""

    @staticmethod
    def inherited():
        return {
            "textures": {"texture_diffuse": "efmaster_material_prologue.tex.diffuse"},
            "parameterSources": {"textures": {"texture_diffuse": {"kind": "MATERIAL_EXPRESSION_DEFAULT"}}},
            "unresolvedDefaults": [],
        }

    def test_null_override_keeps_parent_default_and_records_provenance(self):
        result = self.inherited()
        subject.apply_mic_texture_override(result, "pkg.mat.child_mi", "Texture_Diffuse", 0, None,
                                           {"sourceMaterial": "pkg.mat.child_mi"})
        self.assertEqual(result["textures"]["texture_diffuse"],
                         "efmaster_material_prologue.tex.diffuse")
        source = result["parameterSources"]["textures"]["texture_diffuse"]
        self.assertEqual(source["kind"], "MATERIAL_EXPRESSION_DEFAULT")
        self.assertEqual(source["micNullOverridesIgnored"],
                         [{"sourceMaterial": "pkg.mat.child_mi", "packageIndex": 0}])

    def test_repeated_null_overrides_accumulate_in_chain_order(self):
        result = self.inherited()
        for material in ("pkg.mat.mid_mi", "pkg.mat.leaf_mi"):
            subject.apply_mic_texture_override(result, material, "texture_diffuse", 0, None, {})
        recorded = result["parameterSources"]["textures"]["texture_diffuse"]["micNullOverridesIgnored"]
        self.assertEqual([row["sourceMaterial"] for row in recorded],
                         ["pkg.mat.mid_mi", "pkg.mat.leaf_mi"])

    def test_null_override_without_inherited_value_stays_unresolved(self):
        result = {"textures": {}, "parameterSources": {"textures": {}}, "unresolvedDefaults": []}
        subject.apply_mic_texture_override(result, "pkg.mat.child_mi", "texture_specular", 0, None,
                                           {"sourceMaterial": "pkg.mat.child_mi"})
        self.assertIsNone(result["textures"]["texture_specular"])
        source = result["parameterSources"]["textures"]["texture_specular"]
        self.assertEqual(source["kind"], "MIC_SERIALIZED_OVERRIDE")
        self.assertEqual(source["packageIndex"], 0)
        self.assertNotIn("micNullOverridesIgnored", source)

    def test_real_override_replaces_the_inherited_value(self):
        result = self.inherited()
        subject.apply_mic_texture_override(result, "pkg.mat.child_mi", "texture_diffuse", 43,
                                           "pkg.tex.floor_d", {"sourceMaterial": "pkg.mat.child_mi"})
        self.assertEqual(result["textures"]["texture_diffuse"], "pkg.tex.floor_d")
        source = result["parameterSources"]["textures"]["texture_diffuse"]
        self.assertEqual(source["kind"], "MIC_SERIALIZED_OVERRIDE")
        self.assertEqual(source["packageIndex"], 43)

    def test_unresolvable_reference_is_an_error_not_a_partial_name(self):
        for bad in ("", "pkg.", None):
            with self.subTest(reference=bad):
                result = self.inherited()
                with self.assertRaisesRegex(ValueError, "unresolvable MIC texture reference"):
                    subject.apply_mic_texture_override(result, "pkg.mat.child_mi",
                                                      "texture_diffuse", 7, bad, {})
                self.assertEqual(result["textures"]["texture_diffuse"],
                                 "efmaster_material_prologue.tex.diffuse")

    def test_override_clears_a_pending_unresolved_default(self):
        result = {"textures": {}, "parameterSources": {"textures": {}},
                  "unresolvedDefaults": [{"field": "textures", "name": "texture_diffuse",
                                          "reason": "DEFAULT_PROPERTY_ABSENT"}]}
        subject.apply_mic_texture_override(result, "pkg.mat.child_mi", "texture_diffuse", 43,
                                           "pkg.tex.floor_d", {})
        self.assertEqual(result["unresolvedDefaults"], [])


if __name__ == "__main__":
    unittest.main()
