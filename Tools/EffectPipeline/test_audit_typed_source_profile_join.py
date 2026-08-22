#!/usr/bin/env python3
"""Regression tests for the strict typed source-profile identity join."""

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

SCRIPT_PATH = Path(__file__).resolve().with_name(
    "audit_typed_source_profile_join.py")
SPEC = importlib.util.spec_from_file_location(
    "audit_typed_source_profile_join", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
auditor = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(auditor)

REPO_ROOT = Path(__file__).resolve().parents[2]

HEADER_SAMPLE = """inline EFFECT_STRICT_TYPED_SOURCE_PROFILE Resolve_EffectStrictTypedSourceProfile(
\tconst std::string_view strSourceMaterialPath,
\tconst EFFECT_SOURCE_MATERIAL_DESC& Source)
{
\tif (Source.strProfileId ==
\t\t\t"ue3.material.alpha" &&
\t\tSource.strParentMaterialPath ==
\t\t\t"parent.alpha")
\t{
\t\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::ALPHA;
\t}
\tif ((Source.strProfileId ==
\t\t\t"ue3.material.beta.one" &&
\t\t Source.strParentMaterialPath ==
\t\t\t"parent.beta.one") ||
\t\t(Source.strProfileId ==
\t\t\t"ue3.material.beta.two" &&
\t\t Source.strParentMaterialPath ==
\t\t\t"parent.beta.two"))
\t{
\t\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::BETA;
\t}
\treturn EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE;
}
"""


def _element(profile_id, parent_path, enabled=True):
    return {
        "id": "element",
        "material": {
            "sourceProfile": {
                "enabled": enabled,
                "profileId": profile_id,
                "parentMaterialPath": parent_path,
            }
        },
    }


class TypedSourceProfileJoinTest(unittest.TestCase):
    def test_reads_single_and_alternated_pairs(self):
        with tempfile.TemporaryDirectory() as directory:
            header = Path(directory) / "Effect_MaterialTemplate.h"
            header.write_text(HEADER_SAMPLE, encoding="utf-8")
            pairs = auditor.read_classifier_pairs(header)
        self.assertEqual(
            pairs,
            [("ALPHA", "ue3.material.alpha", "parent.alpha"),
             ("BETA", "ue3.material.beta.one", "parent.beta.one"),
             ("BETA", "ue3.material.beta.two", "parent.beta.two")])

    def test_disabled_source_profiles_are_not_counted(self):
        with tempfile.TemporaryDirectory() as directory:
            authored = Path(directory)
            (authored / "effect.sample.effect.json").write_text(
                json.dumps({"elements": [
                    _element("ue3.material.alpha", "parent.alpha"),
                    _element("ue3.material.alpha", "parent.alpha", False),
                ]}),
                encoding="utf-8")
            counts = auditor.read_authored_pairs(authored)
        self.assertEqual(
            counts[("ue3.material.alpha", "parent.alpha")], 1)

    def test_repository_join_has_no_dead_identity(self):
        """A mistyped literal drops a whole family to grouped with no error."""
        report = auditor.audit(REPO_ROOT)
        self.assertEqual(report["malformedBranches"], [])
        self.assertEqual(
            report["deadPairs"], [],
            "classifier pairs that match no authored element: %r"
            % (report["deadPairs"],))

    def test_master_material_and_lensflare_families_are_joined(self):
        """The two families the LanceMaster D/F expansion depends on."""
        report = auditor.audit(REPO_ROOT)
        joined = report["joinedPairs"]
        basic_ad = joined[(
            "MM_BASIC01",
            "ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99",
            "fx_mastermaterial.fx_mm.fx_mm_basic_01_ad")]
        basic_tr = joined[(
            "MM_BASIC01",
            "ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77",
            "fx_mastermaterial.fx_mm.fx_mm_basic_01_tr")]
        lensflare_alias = joined[(
            "ARTIST_LENSFLARE01",
            "ue3.material.fx.m.fx.c.pa.lensflare.01.ad.ed326b13c7b3",
            "fx_m.fx_c_pa_lensflare_01_ad")]
        self.assertGreater(basic_ad, 0)
        self.assertGreater(basic_tr, 0)
        self.assertGreater(lensflare_alias, 0)


if __name__ == "__main__":
    unittest.main()
