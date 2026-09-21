"""Named source overrides preserve original packing and reject guessed slots."""
import unittest
import build_vehicle_source_material as source


def fixture(body, tail=''):
    return ('inline bool Configure(const std::string& family, const PARAMETER_VALUES& parameters,\n'
            '    Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& result)\n{\n'
            '    if (family == "source.fixture.v1")\n    {\n'
            '        staged.program = 80u;\n' + body + '\n    }\n' + tail +
            '\n}\ninline bool Configure(const std::string& family, const DATA_JSON_VALUE& parameters,\n')


class NamedSourceVectorPatchTests(unittest.TestCase):
    def test_direct_copy_and_overwrite_order(self):
        header = fixture('''        staged.baseConstants[4] = vector(parameter("transcolor"));
        staged.baseConstants[5] = vector(parameter("buffcolor"));
        staged.lightConstants = staged.baseConstants;
        staged.baseConstants[4] = vector(Value{});
        staged.lightConstants[6] = staged.lightConstants[4];''')
        row = source.named_vector_bindings(header)[80]
        self.assertEqual(row['transcolor'], {'base': [], 'light': [4, 6]})
        self.assertEqual(row['buffcolor'], {'base': [5], 'light': [5]})

    def test_common_packing_is_applied_after_family(self):
        header = fixture('        staged.baseConstants[4] = vector(parameter("transcolor"));', '''    if (staged.program == 80u)
    {
        staged.baseConstants[39] = vector(parameter("transcolor"));
    }
    if (staged.program >= 80u && staged.program <= 83u)
    {
        staged.lightConstants = staged.baseConstants;
    }''')
        self.assertEqual(source.named_vector_bindings(header)[80]['transcolor'], {'base': [4, 39], 'light': [4, 39]})

    def test_non_direct_parameter_fails_closed(self):
        with self.assertRaises(SystemExit):
            source.named_vector_bindings(fixture('        staged.baseConstants[4] = vector(multiply(parameter("transcolor"), Value{}));'))

    def test_partial_write_and_indirect_copy_fail_closed(self):
        for extra in ('staged.baseConstants[4].x = 1.f;',
                      'staged.lightConstants[6] = float4_t(staged.baseConstants[4].x, 0.f, 0.f, 0.f);'):
            with self.subTest(extra=extra), self.assertRaises(SystemExit):
                source.named_vector_bindings(fixture('        staged.baseConstants[4] = vector(parameter("transcolor"));\n        ' + extra))

    def test_unrecognized_common_parameter_fails_closed(self):
        with self.assertRaises(SystemExit):
            source.named_vector_bindings(fixture('', '    if (unknown) staged.baseConstants[4] = vector(parameter("transcolor"));'))

    def test_installed_helper_matches_current_packing_and_is_idempotent(self):
        header = source.read_text(source.PARAMETER_HEADER)
        self.assertEqual(source.refresh_named_vector_patch(header), header)
        rows = source.named_vector_bindings(header)
        self.assertNotIn(0, rows)
        self.assertNotIn(33, rows)
        self.assertEqual(rows[99]['transcolor'], {'base': [10], 'light': []})
        self.assertEqual(rows[101]['buffcolor'], {'base': [13], 'light': []})
        self.assertEqual(rows[80]['transcolor'], {'base': [7, 39], 'light': []})


if __name__ == '__main__':
    unittest.main()
