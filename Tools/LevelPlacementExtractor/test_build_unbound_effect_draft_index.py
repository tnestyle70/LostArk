import unittest

from build_unbound_effect_draft_index import build_index


class UnboundEffectDraftIndexTests(unittest.TestCase):
    @staticmethod
    def catalog() -> dict:
        return {
            "characterClass": "WARLORD",
            "sourceSystems": [
                {
                    "sourceAsset": "FX_PC_WGL_00.par_j_bash_01",
                    "logicalPackage": "FX_PC_WGL_00",
                    "objectName": "par_j_bash_01",
                    "graph": {
                        "summary": {"featureClasses": ["ParticleModuleTypeDataMesh"]},
                        "resourceBindings": [
                            {"role": "mesh", "objectPath": "FX_SM_00.BashMesh"},
                            {"role": "material", "objectPath": "FX_MI_00.BashMaterial"},
                        ],
                    },
                }
            ],
            "materialParameterBindings": [
                {
                    "sourceMaterialPath": "FX_MI_00.BashMaterial",
                    "textures": [
                        {"name": "Diffuse", "texture": "FX_T_00.BashDiffuse"}
                    ],
                }
            ],
            "unresolvedMaterialBindings": [
                {
                    "sourceMaterialPath": "FX_DM_00.UnsupportedDecal",
                    "resolutionStatus": "UNSUPPORTED_DECAL_MATERIAL",
                }
            ],
        }

    def test_resolves_resources_without_claiming_skill_ownership(self) -> None:
        cook_receipt = {
            "assets": [
                {
                    "sourceAssetPath": "FX_SM_00.BashMesh",
                    "runtimeAssetId": "Effect/Warlord/Mesh/bash.wmodel",
                },
                {
                    "sourceAssetPath": "FX_T_00.BashDiffuse",
                    "runtimeAssetId": "Effect/Warlord/Texture/bash.dds",
                },
            ]
        }

        document = build_index(self.catalog(), cook_receipt)

        self.assertEqual("WARLORD", document["characterClass"])
        self.assertEqual(1, document["summary"]["sourceSystemCount"])
        self.assertEqual(0, document["summary"]["skillBoundSourceSystemCount"])
        self.assertEqual(
            0, document["summary"]["missingOrAmbiguousRuntimeResourceCount"]
        )
        system = document["sourceSystems"][0]
        self.assertEqual("UNBOUND_TO_SKILL_NOTIFY", system["bindingStatus"])
        self.assertEqual(
            "NEEDS_SKILL_NOTIFY_AND_EMITTER_PARTITION_MAPPING",
            system["conversionStatus"],
        )
        self.assertEqual(
            ["mesh", "particle"], system["candidateKinds"]
        )
        self.assertEqual(
            {
                "Effect/Warlord/Mesh/bash.wmodel",
                "Effect/Warlord/Texture/bash.dds",
            },
            {
                row["runtimeAssetId"] for row in system["resourceCandidates"]
            },
        )
        self.assertEqual(
            "UNSUPPORTED_DECAL_MATERIAL",
            document["unsupportedMaterialBindings"][0]["resolutionStatus"],
        )

    def test_counts_missing_runtime_resource_without_fallback(self) -> None:
        document = build_index(self.catalog(), {"assets": []})

        self.assertEqual(
            2, document["summary"]["missingOrAmbiguousRuntimeResourceCount"]
        )
        self.assertTrue(
            all(
                row["runtimeAssetId"] is None
                for row in document["sourceSystems"][0]["resourceCandidates"]
            )
        )


if __name__ == "__main__":
    unittest.main()
