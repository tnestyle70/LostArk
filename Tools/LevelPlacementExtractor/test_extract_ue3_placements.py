import struct
import unittest

from extract_ue3_placements import (
    ExportEntry,
    ImportEntry,
    Reader,
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


if __name__ == "__main__":
    unittest.main()
