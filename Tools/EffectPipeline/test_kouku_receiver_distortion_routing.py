"""Focused regeneration checks for the Mario/Showtime distortion MRT split."""
import unittest

from install_kouku_gate1_native_shaders import route_receiver_isolated_distortion


class ReceiverDistortionRoutingTests(unittest.TestCase):
    def block(self, program, distortion=True):
        body = (f'const float4 accumulated=ArtistNative{program}Distortion(input);\n'
                'output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);\n'
                if distortion else '')
        return (f'#if EFFECT_NATIVE_PROFILE_GROUP == {program // 64 * 64}\n'
                f'case {program}u:\n{{\n'
                f'nativeColor=ArtistNative{program}(input);\n'
                'output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,1.f);\n'
                + body + 'return output;\n}\n#endif\n')

    def test_reviewed_noise_moves_only_offset_channels(self):
        for program in (2461, 2587, 3682):
            with self.subTest(program=program):
                before = self.block(program)
                after = route_receiver_isolated_distortion(before)
                self.assertEqual(after, before.replace(
                    'float4(accumulated.xy-accumulated.zw,0.f,0.f)',
                    'float4(0.f,0.f,accumulated.xy-accumulated.zw)'))
                self.assertEqual(route_receiver_isolated_distortion(after), after)

    def test_ordinary_and_source_zero_passes_stay_identical(self):
        for program in (2310, 2314, 2429, 2456, 2457, 2459, 2462,
                        2465, 2469, 2589, 3680, 3683, 3686):
            with self.subTest(program=program):
                before = self.block(program)
                self.assertEqual(route_receiver_isolated_distortion(before), before)

    def test_color_only_and_order_are_preserved(self):
        before = self.block(2584, False) + self.block(2587) + self.block(3687, False)
        self.assertEqual(route_receiver_isolated_distortion(before), before.replace(
            'float4(accumulated.xy-accumulated.zw,0.f,0.f)',
            'float4(0.f,0.f,accumulated.xy-accumulated.zw)'))

    def test_missing_or_changed_source_pass_is_rejected(self):
        for before in (self.block(2587, False), self.block(2587).replace(
                'accumulated.xy-accumulated.zw,0.f,0.f', 'accumulated')):
            with self.assertRaises(AssertionError):
                route_receiver_isolated_distortion(before)


if __name__ == '__main__':
    unittest.main()
