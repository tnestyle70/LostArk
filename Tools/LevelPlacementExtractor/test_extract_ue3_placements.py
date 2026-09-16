import hashlib
import json
import struct
import unittest
from types import SimpleNamespace
from dataclasses import replace

from extract_ue3_placements import (
    ExtractionError,
    ExportEntry,
    ImportEntry,
    Reader,
    material_override_slots,
    resolve_component_actor,
    source_visibility_from_chains,
    SourceVisibilityResolver,
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


class SourceVisibilityTests(unittest.TestCase):
    @staticmethod
    def chain(flags=None, name="Instance"):
        return [{"objectPath": name, "physicalPackage": "source.upk", "packageSha256": "a" * 64,
                 "exportIndex": 488, "serializedFlags": flags or {}}]

    def test_navigation_participation_is_not_hidden(self):
        actor = self.chain({"EFNavMeshTerrain": True})
        result = source_visibility_from_chains(actor, self.chain())
        self.assertTrue(result["visible"])
        self.assertEqual(result["actorHidden"]["origin"], "zero-default")
        self.assertIsNone(result["componentVisible"]["value"])

    def test_actor_and_both_component_hidden_spellings(self):
        self.assertFalse(source_visibility_from_chains(self.chain({"bHidden": True}), self.chain())["visible"])
        for field in ("HiddenGame", "bHiddenGame", "hiddengame"):
            self.assertFalse(source_visibility_from_chains(self.chain(), self.chain({field: True}))["visible"])
        self.assertFalse(source_visibility_from_chains(self.chain(), self.chain({"bVisible": False}))["visible"])

    def test_instance_false_overrides_inherited_true(self):
        inherited = self.chain({"HiddenGame": True}, "Default__StaticMeshComponent")
        result = source_visibility_from_chains(self.chain(), self.chain({"HiddenGame": False}) + inherited)
        self.assertTrue(result["visible"])
        inherited_result = source_visibility_from_chains(self.chain(), self.chain() + inherited)
        self.assertFalse(inherited_result["visible"])
        self.assertEqual(inherited_result["componentHiddenGame"]["sourceIndex"], 1)

    def test_invalid_and_conflicting_flags_rejected(self):
        for flags in ({"HiddenGame": 1}, {"HiddenGame": True, "bHiddenGame": False}):
            with self.assertRaises(ExtractionError):
                source_visibility_from_chains(self.chain(), self.chain(flags))
        with self.assertRaises(ExtractionError):
            source_visibility_from_chains([], self.chain())

    def test_actual_tagged_archetype_chain_inherits_hidden_and_detects_cycles(self):
        names = ["None", "HiddenGame", "BoolProperty"]
        empty = struct.pack("<iii", 0, 0, 0)
        hidden = struct.pack("<7i", 0, 1, 0, 2, 0, 0, 0) + b"\x01" + struct.pack("<ii", 0, 0)
        instance = PlacementActorResolutionTests.export(0, 0, 0, "Component")
        default = PlacementActorResolutionTests.export(1, 0, 0, "Default__StaticMeshComponent")
        instance = replace(instance, archetype_index=2)
        package = {"path": "source.upk", "summary": SimpleNamespace(version=868), "names": names,
                   "imports": [], "exports": [instance, default], "sha256": "a" * 64,
                   "read": lambda entry: empty if entry.index == 0 else hidden}
        resolver = object.__new__(SourceVisibilityResolver)
        resolver.cache = {}
        chain = resolver.chain(package, instance)
        result = source_visibility_from_chains(self.chain(), chain)
        self.assertFalse(result["visible"])
        self.assertEqual(result["componentHiddenGame"]["origin"], "archetype-cdo")
        package["exports"][1] = replace(default, archetype_index=1)
        resolver.cache = {}
        with self.assertRaisesRegex(ExtractionError, "cycle"):
            resolver.chain(package, instance)

    def test_missing_or_ambiguous_external_cdo_is_not_a_visible_default(self):
        resolver = object.__new__(SourceVisibilityResolver)
        resolver.scripts, resolver.script_errors = [], []
        preferred = {"exports": [], "imports": []}
        with self.assertRaisesRegex(ExtractionError, "unresolved or ambiguous"):
            resolver._default_object("Engine.Default__Actor", preferred)
        default = PlacementActorResolutionTests.export(0, 0, 0, "Default__Actor")
        resolver.scripts = [{"defaultPaths": {"default__actor": default}}, {"defaultPaths": {"default__actor": default}}]
        with self.assertRaisesRegex(ExtractionError, "matches=2"):
            resolver._default_object("Engine.Default__Actor", preferred)


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
