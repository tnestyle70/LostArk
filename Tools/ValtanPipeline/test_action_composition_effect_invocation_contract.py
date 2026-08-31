#!/usr/bin/env python3
from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BALANCE_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_CPP = ROOT / "Client/Private/BalanceTool.cpp"
AUTHORING_H = ROOT / "Client/Public/ValtanPatternEffectCueAuthoring.h"
AUTHORING_CPP = ROOT / "Client/Private/ValtanPatternEffectCueAuthoring.cpp"
WORKBENCH_CPP = ROOT / "Client/Private/ActionCompositionWorkbench.cpp"


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


class ActionCompositionEffectInvocationContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.balance_h = BALANCE_H.read_text(encoding="utf-8")
        cls.balance_cpp = BALANCE_CPP.read_text(encoding="utf-8")
        cls.authoring_h = AUTHORING_H.read_text(encoding="utf-8")
        cls.authoring_cpp = AUTHORING_CPP.read_text(encoding="utf-8")
        cls.workbench = WORKBENCH_CPP.read_text(encoding="utf-8")

    def test_explicit_typed_apis_preserve_broad_stage_inventory_boundary(self) -> None:
        for token in (
            "Add_ValtanStageEffectCue",
            "Update_ValtanStageEffectCue",
            "Remove_ValtanStageEffectCue",
        ):
            self.assertIn(token, self.balance_h)
            self.assertIn(token, self.balance_cpp)
        broad = body(
            self.balance_cpp,
            "bool Client::CBalanceTool::Set_ValtanStageDraft(",
        )
        self.assertIn("joined action/effect inventory is read-only", broad)

    def test_mutations_are_fail_closed_copy_validate_commit(self) -> None:
        balance_signatures = (
            ("bool Client::CBalanceTool::Add_ValtanStageEffectCue(", "::Add("),
            ("bool Client::CBalanceTool::Update_ValtanStageEffectCue(", "::Update("),
            ("bool Client::CBalanceTool::Remove_ValtanStageEffectCue(", "::Remove("),
        )
        for signature, delegated_call in balance_signatures:
            method = body(self.balance_cpp, signature)
            self.assertIn("Require_ValtanAuthoringAdmission", method)
            self.assertIn("CValtanPatternEffectCueAuthoring", method)
            self.assertIn(delegated_call, method)
            self.assertIn("MarkDirty(changed)", method)
        self.assertNotIn("ValidateValtanEffectCueDraft", self.balance_cpp)
        self.assertNotIn("ValidateValtanEffectCueMirrors", self.balance_cpp)

        for signature in (
            "bool_t Client::CValtanPatternEffectCueAuthoring::Add(",
            "bool_t Client::CValtanPatternEffectCueAuthoring::Update(",
            "bool_t Client::CValtanPatternEffectCueAuthoring::Remove(",
        ):
            method = body(self.authoring_cpp, signature)
            self.assertIn("RequireAdmission", method)
            self.assertIn("VALTAN_PATTERN_TREE_VIEW Staged = InOutTree", method)
            self.assertIn("Validate_Mirrors", method)
            self.assertIn("InOutTree = std::move(Staged)", method)
            self.assertIn("bOutChanged = true", method)
            self.assertLess(method.index("Validate_Mirrors"), method.index("InOutTree = std::move(Staged)"))
        validator = body(self.authoring_cpp, "bool_t ValidateCue(")
        for token in (
            '"WAIT" == Stage.strSequenceRole',
            "Stage.ClipOccurrences.empty()",
            "Context.QuerySourceMembership",
            '"effect.valtan."',
            '"pattern.target.snapshot"',
            '"arena.center"',
            '"arena.center.facing"',
            '"each_loop"',
            '(1.f != Cue.vWorldScale.x || 1.f != Cue.vWorldScale.y',
        ):
            self.assertIn(token, validator)
        self.assertIn('"LOCK_FACING_ON_START" == Pattern.strAimPolicy', validator)
        self.assertIn('"LOCK_RANDOM_ALIVE_ON_START" == Pattern.strTargetPolicy', validator)

    def test_patch_uses_source_owner_effect_ops_and_exact_payload(self) -> None:
        patch = body(
            self.balance_cpp,
            "bool Client::CBalanceTool::BuildValtanDraftPatch(",
        )
        for token in (
            "REMOVE_EFFECT_CUE",
            "ADD_EFFECT_CUE",
            "UPDATE_EFFECT_CUE",
            "cueId",
            "occurrenceId",
            "effectAssetId",
            "clipOccurrenceId",
            "sourceStartMs",
            "sourceEndMs",
            "anchorSlotId",
            "followPolicy",
            "stopPolicy",
            "repeatPolicy",
            "localTransform",
            "scalePolicy",
            "mappingBasis",
        ):
            self.assertIn(token, patch)
        self.assertLess(
            patch.index("effectCueRemoveOperations.begin()"),
            patch.index("operations.begin()"),
        )
        self.assertLess(
            patch.index("operations.begin()"),
            patch.index("effectCueUpsertOperations.begin()"),
        )
        self.assertNotIn("Valtan.patterneffectcues.json", patch)

    def test_workbench_uses_semantic_catalog_and_full_typed_details(self) -> None:
        semantic = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Reload_SemanticValtanEffects(",
        )
        self.assertIn('rfind("effect.valtan.", 0u)', semantic)
        self.assertIn("CEffectCatalog::Is_DirectAuthoredDocument", semantic)
        details = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        for token in (
            '"Filter authored effect.valtan.*..."',
            '"Add Effect Invocation"',
            '"Update Invocation"',
            '"Remove Invocation"',
            '"Source start (ms)"',
            '"Source end (ms)"',
            '"pattern.target.snapshot"',
            '"arena.center"',
            '"arena.center.facing"',
            '"Tune / Remove Existing Server Collider / Hit Schedule"',
            '"Add Manual Audition Server Collider / Hit Schedule"',
            '"View Collider Authority (New Add Unavailable)"',
            "Add_ValtanStageEffectCue",
            "Update_ValtanStageEffectCue",
            "Remove_ValtanStageEffectCue",
        ):
            self.assertIn(token, details)
        self.assertIn('"WAIT" != pStage->strSequenceRole', details)
        self.assertIn("!pStage->ClipOccurrences.empty()", details)
        self.assertIn("bArenaFacingAnchorAdmitted", details)
        self.assertNotIn("Valtan.patterneffectcues.json is an editable", details)

    def test_effect_body_deep_link_requires_one_exact_admitted_saved_occurrence(self) -> None:
        equality = body(self.workbench, "bool_t SameSavedValtanEffectCue(")
        for token in (
            "strBindingId",
            "strOccurrenceId",
            "strPatternId",
            "strStageId",
            "strActionId",
            "strClipOccurrenceId",
            "strEffectAssetId",
            "strAnchorSlotId",
            "strFollowPolicy",
            "strStopPolicy",
            "strRepeatPolicy",
            "strScalePolicy",
            "vWorldScale",
            "iSourceStartMs",
            "iSourceEndMs",
            "LocalTransform.vPosition",
            "LocalTransform.vRotationDegrees",
            "LocalTransform.vScale",
        ):
            self.assertIn(token, equality)

        details = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        gate = details[details.index("const VALTAN_PATTERN_VIEW* pSavedPattern") :]
        for token in (
            "ADMISSION_STATE::ADMITTED == m_eAdmission",
            "Find_SelectedPattern()",
            "iSavedOccurrenceCount",
            "1u == iSavedOccurrenceCount",
            "SameSavedValtanEffectCue(*Found, *savedCue)",
            "bEffectBodyDeepLinkAdmitted",
            '"Open Saved Product Effect Body"',
            '"Save + Reload Before Opening Effect Body"',
            "Request_EffectOwner(*pSavedPattern, *pSavedStage, *pSavedCue)",
            "draft-only Add/Update rows cannot open an asset-only fallback",
        ):
            self.assertIn(token, gate)
        self.assertIn(
            "ImGui::BeginDisabled(!bEffectBodyDeepLinkAdmitted)", gate
        )
        self.assertNotIn(
            "Request_EffectOwner(*pPattern, *pStage, *Found)", gate
        )

    def test_collider_add_is_manual_only_while_existing_hits_are_tunable(self) -> None:
        gameplay = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(",
        )
        for token in (
            'const bool_t bHasExistingServerHit = "NONE" != Draft.hitShape',
            "Pattern.bManualServerAudition",
            '"WAIT" != Stage.strSequenceRole && Draft.hitEditable',
            "ImGui::BeginDisabled(!bManualColliderAddAdmitted)",
            "if (bHasExistingServerHit && Draft.hitEditable)",
            '"Add Server Collider"',
            '"Remove Server Collider"',
            "New Collider Add is unavailable on a canonical Stage with no existing hit contract",
            "Existing Server hit contract: tune its typed geometry/schedule",
        ):
            self.assertIn(token, gameplay)

        def capabilities(*, manual: bool, wait: bool, has_hit: bool) -> tuple[bool, bool]:
            hit_editable = (not wait) and (manual or has_hit)
            add = manual and (not wait) and hit_editable and (not has_hit)
            tune_or_remove = has_hit and hit_editable
            return add, tune_or_remove

        self.assertEqual((False, False), capabilities(manual=False, wait=False, has_hit=False))
        self.assertEqual((False, True), capabilities(manual=False, wait=False, has_hit=True))
        self.assertEqual((True, False), capabilities(manual=True, wait=False, has_hit=False))
        self.assertEqual((False, False), capabilities(manual=True, wait=True, has_hit=False))

    def test_generated_cue_and_occurrence_identity_share_the_160_byte_bound(self) -> None:
        allocator = body(self.workbench, "std::string BuildNextCompositionEffectCueId(")
        self.assertIn('OCCURRENCE_SUFFIX = ".occurrence.01"', allocator)
        self.assertIn("candidate.size() + OCCURRENCE_SUFFIX.size() > 160u", allocator)
        self.assertEqual(allocator.count("FindFreeId("), 2)
        self.assertIn("return FindFreeId(compact.str())", allocator)

    def test_sequencer_effect_row_selects_typed_detail_without_fake_collider(self) -> None:
        timeline = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
        )
        self.assertIn(
            "DETAIL_OWNER::EFFECT, TIMELINE_LANE::EFFECT", timeline
        )
        self.assertIn("Cue.strOccurrenceId", timeline)
        details = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_Details(",
        )
        self.assertIn("never infers hit geometry", details)
        self.assertNotIn("Cue.LocalTransform.vScale.x *", details)

    def test_stale_preview_is_stop_only_and_reload_clears_effect_edit_copy(self) -> None:
        reload_body = body(
            self.workbench,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        self.assertIn("Reset_EffectCueEditor();", reload_body)
        self.assertIn("m_SemanticValtanEffectAssetIds.clear();", reload_body)
        self.assertIn("m_bSemanticValtanEffectLoadAttempted = false;", reload_body)
        self.assertNotIn("Reload_SemanticValtanEffects();", reload_body)
        preview = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_Preview(",
        )
        self.assertIn("Stop is the sole stale-safe transport command", preview)
        self.assertIn("nullptr == m_pAnimationTool || !Preview.bPlaying", preview)
        sequence_browser = body(
            self.workbench,
            "void Client::CActionCompositionWorkbench::Render_SequenceBrowser(",
        )
        use_index = sequence_browser.index('"Use for Create New Pattern"')
        self.assertIn("!bMutationAdmitted", sequence_browser[:use_index])


if __name__ == "__main__":
    unittest.main()
