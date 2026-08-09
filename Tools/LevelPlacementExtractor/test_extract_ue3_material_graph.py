import unittest

from extract_ue3_material_graph import (
    classify_topology,
    expression_input,
    expression_inputs,
    is_effect_material_class,
)


class ExtractUE3MaterialGraphTests(unittest.TestCase):
    def test_decal_material_uses_the_same_cooked_graph_extractor(self) -> None:
        self.assertTrue(is_effect_material_class("Material"))
        self.assertTrue(is_effect_material_class("DecalMaterial"))
        self.assertFalse(is_effect_material_class("MaterialInstanceConstant"))

    def test_expression_input_preserves_channel_contract(self) -> None:
        value = {
            "properties": {
                "expression": {"value": 42},
                "outputindex": {"value": 2},
                "mask": {"value": 1},
                "maskr": {"value": 1},
            }
        }
        self.assertEqual(
            {
                "packageIndex": 42,
                "outputindex": 2,
                "mask": 1,
                "maskr": 1,
            },
            expression_input(value),
        )

    def test_expression_inputs_do_not_invent_plain_properties(self) -> None:
        properties = {
            "a": {"value": {"properties": {"expression": {"value": 7}}}},
            "defaultvalue": {"value": 1.25},
        }
        self.assertEqual(
            [{"input": "a", "packageIndex": 7}],
            expression_inputs(properties),
        )

    def test_null_cooked_nodes_are_not_runtime_exact(self) -> None:
        self.assertEqual(
            "COOKED_PARTIAL",
            classify_topology(
                344, 105,
                {"opacity": {"packageIndex": 10}},
                0,
            ),
        )

    def test_complete_surviving_graph_can_be_exact_candidate(self) -> None:
        self.assertEqual(
            "SURVIVING_GRAPH_CAPTURED",
            classify_topology(
                4, 4,
                {"opacity": {"packageIndex": 10}},
                0,
            ),
        )


if __name__ == "__main__":
    unittest.main()
