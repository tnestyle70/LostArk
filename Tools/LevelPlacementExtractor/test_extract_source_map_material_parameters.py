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


if __name__ == "__main__":
    unittest.main()
