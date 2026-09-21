"""Guard destructive clip-name collisions and repeat installation of WANM sections."""
import unittest

import bake_character_cinematic_clips as bake


class CharacterCinematicAppendTests(unittest.TestCase):
    @staticmethod
    def model():
        wm = bake.wm
        body = wm.MODEL_HEADER.pack(b"WMOD", 0, 0, 0, 0, 0, 0, 0)
        return wm.FILE_HEADER.pack(b"WINT", 1, 5, 0, len(body)) + body

    def test_existing_clip_is_preserved_and_repeat_is_identical(self):
        original = bake.append(self.model(), [("base", b"existing-animation")])
        result, count = bake.append_idempotent(original, [("cinematic", b"baked-animation")])
        self.assertEqual(count, 1)
        self.assertEqual(bake.sections(result)[:1], bake.sections(original))
        self.assertEqual(bake.append_idempotent(result, [("cinematic", b"baked-animation")]), (result, 0))

    def test_different_animation_with_same_name_is_rejected(self):
        original = bake.append(self.model(), [("cinematic", b"user-animation")])
        with self.assertRaisesRegex(ValueError, "name/content collision"):
            bake.append_idempotent(original, [("cinematic", b"replacement-animation")])
        self.assertEqual(bake.animation_payloads(original)["cinematic"], b"user-animation")

    def test_conflicting_additions_are_rejected_before_output(self):
        with self.assertRaisesRegex(ValueError, "name/content collision"):
            bake.append_idempotent(self.model(), [("cinematic", b"a"), ("cinematic", b"b")])

    def test_overlong_name_cannot_be_truncated_into_existing_name(self):
        with self.assertRaisesRegex(ValueError, "Invalid clip name"):
            bake.append_idempotent(self.model(), [("x" * 40, b"a")])


if __name__ == "__main__":
    unittest.main()
