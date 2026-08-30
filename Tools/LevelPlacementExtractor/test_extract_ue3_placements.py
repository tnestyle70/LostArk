import hashlib
import json
import struct
import unittest

from extract_ue3_placements import (
    ExtractionError,
    ExportEntry,
    ImportEntry,
    Reader,
    material_override_slots,
    resolve_component_actor,
)


class ReaderFStringTests(unittest.TestCase):
    def test_utf16_fstring_removes_only_the_wide_terminator(self) -> None:
        value = "Actor"
        encoded = value.encode("utf-16-le") + b"\0\0"
        reader = Reader(struct.pack("<i", -(len(value) + 1)) + encoded)

        self.assertEqual(value, reader.fstring())
        self.assertEqual(4 + len(encoded), reader.offset)

    def test_utf16_fstring_preserves_non_ascii_code_units(self) -> None:
        value = "맵"
        encoded = value.encode("utf-16-le") + b"\0\0"
        reader = Reader(struct.pack("<i", -2) + encoded)

        self.assertEqual(value, reader.fstring())


class PlacementActorResolutionTests(unittest.TestCase):
    @staticmethod
    def export(
        index: int,
        class_index: int,
        package_index: int,
        object_name: str,
    ) -> ExportEntry:
        return ExportEntry(
            index=index,
            class_index=class_index,
            super_index=0,
            package_index=package_index,
            object_name=object_name,
            archetype_index=0,
            serial_size=0,
            serial_offset=0,
            export_flags=0,
        )

    def test_lostark_motion_static_mesh_actor_is_an_exact_owner(self) -> None:
        imports = [
            ImportEntry(0, "Core", "Class", 0, "EFMotionStaticMeshActor")
        ]
        actor = self.export(0, -1, 0, "EFMotionStaticMeshActor_0")
        component = self.export(1, 0, 1, "StaticMeshComponent_0")

        self.assertIs(
            actor,
            resolve_component_actor(component, imports, [actor, component]),
        )

    def test_unknown_component_owner_is_not_replaced_with_identity(self) -> None:
        imports = [ImportEntry(0, "Core", "Class", 0, "UnknownActor")]
        actor = self.export(0, -1, 0, "UnknownActor_0")
        component = self.export(1, 0, 1, "StaticMeshComponent_0")

        self.assertIsNone(
            resolve_component_actor(component, imports, [actor, component])
        )


class MaterialOverrideTests(unittest.TestCase):
    @staticmethod
    def material_imports(
        class_name: str = "MaterialInstanceConstant",
    ) -> list[ImportEntry]:
        return [
            ImportEntry(0, "Engine", class_name, -2, "Curtain_MI"),
            ImportEntry(1, "Core", "Package", -3, "Material"),
            ImportEntry(2, "Core", "Package", 0, "BG_RAD_KOUKUSATON_A"),
        ]

    def test_absent_and_empty_material_arrays_have_the_same_signature(self) -> None:
        absent = material_override_slots({}, [], [])
        empty = material_override_slots({"Materials": {"value": []}}, [], [])

        expected = hashlib.sha256(b"[]").hexdigest()
        self.assertFalse(absent["propertyPresent"])
        self.assertTrue(empty["propertyPresent"])
        self.assertEqual([], absent["slots"])
        self.assertEqual(expected, absent["signatureSha256"])
        self.assertEqual(expected, empty["signatureSha256"])

    def test_order_and_authored_null_slots_are_preserved(self) -> None:
        result = material_override_slots(
            {"Materials": {"value": [-1, 0]}}, self.material_imports(), []
        )

        canonical = [
            "materialinstanceconstant|bg_rad_koukusaton_a.material.curtain_mi",
            None,
        ]
        expected = hashlib.sha256(
            json.dumps(canonical, separators=(",", ":")).encode("utf-8")
        ).hexdigest()
        self.assertEqual([0, 1], [slot["slot"] for slot in result["slots"]])
        self.assertEqual(
            "BG_RAD_KOUKUSATON_A.Material.Curtain_MI",
            result["slots"][0]["objectPath"],
        )
        self.assertIsNone(result["slots"][1]["objectPath"])
        self.assertEqual(expected, result["signatureSha256"])

    def test_non_material_override_reference_is_rejected(self) -> None:
        with self.assertRaisesRegex(ExtractionError, "Material/MIC"):
            material_override_slots(
                {"Materials": {"value": [-1]}},
                self.material_imports("Texture2D"),
                [],
            )


if __name__ == "__main__":
    unittest.main()
