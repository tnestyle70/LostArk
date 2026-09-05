#!/usr/bin/env python3
from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
VALTAN_H = ROOT / "Client/Public/Valtan.h"
VALTAN_CPP = ROOT / "Client/Private/Valtan.cpp"
BALANCE_TOOL_H = ROOT / "Client/Public/BalanceTool.h"
BALANCE_TOOL_CPP = ROOT / "Client/Private/BalanceTool.cpp"


def function_body(source: str, signature: str) -> str:
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


def compact(source: str) -> str:
    return " ".join(source.split())


class ValtanStageHitActivationAuthoringTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.header = VALTAN_H.read_text(encoding="utf-8-sig")
        cls.source = VALTAN_CPP.read_text(encoding="utf-8-sig")
        cls.balance_header = BALANCE_TOOL_H.read_text(encoding="utf-8-sig")
        cls.balance_source = BALANCE_TOOL_CPP.read_text(encoding="utf-8-sig")

    def test_stage_draft_owns_hit_authority_and_split_admission(self) -> None:
        begin = self.balance_header.index("struct PATTERN_STAGE_EDIT")
        end = self.balance_header.index("};", begin)
        draft = self.balance_header[begin:end]
        for token in (
            "bool hasHitAnchor = false;",
            'std::string hitAnchorKind = "BOSS_CURRENT";',
            "double hitAnchorForwardOffsetM = 0.0;",
            "double hitAnchorRightOffsetM = 0.0;",
            "double hitAnchorYawOffsetDegrees = 0.0;",
            "bool hasHitActivation = false;",
            "std::uint32_t hitActivationStartMs = 0u;",
            "std::uint32_t hitActivationLifetimeMs = 0u;",
            "bool colliderAddAdmitted = false;",
            "bool colliderTuneAdmitted = false;",
            "bool colliderRemoveAdmitted = false;",
        ):
            self.assertIn(token, draft)

    def test_stage_draft_copies_and_writes_back_hit_authority(self) -> None:
        build = compact(
            function_body(
                self.balance_source,
                "CBalanceTool::PATTERN_STAGE_EDIT BuildValtanStageDraft(",
            )
        )
        for token in (
            "draft.hasHitAnchor = stage.bHasHitAnchor;",
            "draft.hitAnchorKind = stage.strHitAnchorKind;",
            "draft.hitAnchorForwardOffsetM = stage.fHitAnchorForwardOffsetM;",
            "draft.hitAnchorRightOffsetM = stage.fHitAnchorRightOffsetM;",
            "draft.hitAnchorYawOffsetDegrees = stage.fHitAnchorYawOffsetDegrees;",
            "draft.hasHitActivation = stage.bHasHitActivation;",
            "draft.hitActivationStartMs = stage.iHitActivationStartMs;",
            "draft.hitActivationLifetimeMs = stage.iHitActivationLifetimeMs;",
            "draft.colliderAddAdmitted = !isWaitStage && pattern.bManualServerAudition && !hasCollider;",
            "draft.colliderTuneAdmitted = !isWaitStage && hasCollider;",
            'pattern.bManualServerAudition && hasCollider && "CAPTURE" != stage.strPlayerResponse;',
        ):
            self.assertIn(token, build)

        setter = compact(
            function_body(
                self.balance_source,
                "bool Client::CBalanceTool::Set_ValtanStageDraft(",
            )
        )
        for token in (
            "candidate.hasHitAnchor != current.hasHitAnchor",
            "candidate.hitAnchorKind != current.hitAnchorKind",
            "candidate.hasHitActivation != current.hasHitActivation",
            "candidate.hitActivationStartMs != current.hitActivationStartMs",
            "candidate.hitActivationLifetimeMs != current.hitActivationLifetimeMs",
            "const bool hitActivationValid = candidate.hasHitActivation ?",
            "candidateHasCollider && 0u == candidate.hitCount",
            "0u == candidate.hitIntervalMs && 0u == candidate.hitDelayMs",
            "candidate.hitOffsetsMs.empty()",
            "candidate.hitActivationLifetimeMs >= 1u",
            "static_cast<std::uint64_t>(candidate.hitActivationStartMs) + candidate.hitActivationLifetimeMs <= candidate.durationMs",
            "0u == candidate.hitActivationStartMs && 0u == candidate.hitActivationLifetimeMs",
            "stage->bHasHitAnchor = candidate.hasHitAnchor;",
            "stage->strHitAnchorKind = candidate.hitAnchorKind;",
            "stage->bHasHitActivation = candidate.hasHitActivation;",
            "stage->iHitActivationStartMs = candidate.hitActivationStartMs;",
            "stage->iHitActivationLifetimeMs = candidate.hitActivationLifetimeMs;",
        ):
            self.assertIn(token, setter)
        for token in (
            '"BOSS_CURRENT" == candidate.hitAnchorKind',
            "0.0 == candidate.hitAnchorForwardOffsetM",
            "0.0 == candidate.hitAnchorRightOffsetM",
            "0.0 == candidate.hitAnchorYawOffsetDegrees",
        ):
            self.assertIn(token, setter)

    def test_stage_hit_patch_emits_activation_or_schedule_and_anchor(self) -> None:
        patch = compact(
            function_body(
                self.balance_source,
                "bool Client::CBalanceTool::BuildValtanDraftPatch(",
            )
        )
        for token in (
            "stage.bHasHitActivation != loadedStage->bHasHitActivation",
            "stage.iHitActivationStartMs != loadedStage->iHitActivationStartMs",
            "stage.iHitActivationLifetimeMs != loadedStage->iHitActivationLifetimeMs",
            "if (stage.bHasHitActivation)",
            'hit << ", \\"activation\\": { \\"kind\\": \\"ACTIVE_WINDOW\\", "',
            '<< ", \\"perTargetPolicy\\": \\"ONCE\\" }";',
            "else if (!stage.HitOffsetsMs.empty())",
            'hit << ", \\"schedule\\": { \\"kind\\": \\"EXPLICIT_OFFSETS\\", \\"offsetsMs\\": [";',
            'hit << ", \\"schedule\\": { \\"kind\\": \\"INTERVAL\\", "',
            "if (stage.bHasHitAnchor)",
            'hit << ", \\"anchor\\": { \\"kind\\": "',
        ):
            self.assertIn(token, patch)
        self.assertLess(
            patch.index("if (stage.bHasHitActivation)"),
            patch.index("else if (!stage.HitOffsetsMs.empty())"),
        )

    def test_saved_authoring_restore_preserves_joined_extended_hit(self) -> None:
        restore = compact(
            function_body(
                self.balance_source,
                "bool Client::CBalanceTool::RestoreValtanSavedAuthoring(",
            )
        )
        for token in (
            "CValtanPatternTree::Load_FromAuthoringPaths(",
            "patternTree = std::move(savedPatternTree);",
            "targetStage->iDurationMs != durationMs",
            "const bool_t bJoinedTreeOwnsExtendedHit =",
            'nullptr != hit->Find("anchor")',
            'nullptr != hit->Find("activation")',
            'nullptr != hit->Find("playerResponse")',
            'nullptr != hit->Find("attachmentSlot")',
            "if (!bJoinedTreeOwnsExtendedHit) { targetStage->fHitOuterRadius = 0.f;",
            "if (bJoinedTreeOwnsExtendedHit) { if (targetStage->strHitShape != shapeKind)",
        ):
            self.assertIn(token, restore)
        self.assertNotIn("targetStage->strHitShape = shapeKind;", restore)
        self.assertLess(
            restore.index("patternTree = std::move(savedPatternTree);"),
            restore.index("const bool_t bJoinedTreeOwnsExtendedHit ="),
        )

    def test_damage_rate_uses_existing_typed_draft_owner(self) -> None:
        for token in (
            "bool Get_ValtanDamageRateDraft(",
            "bool Get_ValtanDamageProfileStageUserCountDraft(",
            "bool Set_ValtanDamageRateDraft(",
        ):
            self.assertIn(token, self.balance_header)
        getter = compact(
            function_body(
                self.balance_source,
                "bool Client::CBalanceTool::Get_ValtanDamageRateDraft(",
            )
        )
        setter = compact(
            function_body(
                self.balance_source,
                "bool Client::CBalanceTool::Set_ValtanDamageRateDraft(",
            )
        )
        for body in (getter, setter):
            self.assertIn("Require_ValtanAuthoringAdmission(", body)
            self.assertIn('damageProfileId.rfind("damage.valtan.", 0u)', body)
            self.assertIn("FindDamageRate(damageProfileId)", body)
        self.assertIn("ratePercent < 1u || ratePercent > 100000u", setter)
        self.assertIn("MarkDirty(true);", setter)

    def test_damage_profile_stage_user_count_uses_joined_pattern_draft(self) -> None:
        counter = compact(
            function_body(
                self.balance_source,
                "bool Client::CBalanceTool::Get_ValtanDamageProfileStageUserCountDraft(",
            )
        )
        for token in (
            "Require_ValtanAuthoringAdmission(",
            'damageProfileId.rfind("damage.valtan.", 0u)',
            "nullptr == FindDamageRate(damageProfileId)",
            "&m_valtanPatternTree.Gimmicks",
            "&m_valtanPatternTree.Rotation",
            "pattern.Stages.begin(), pattern.Stages.end()",
            "stage.strServerDamageProfileId == damageProfileId",
            "stageUserCount = count;",
        ):
            self.assertIn(token, counter)

    def test_balance_pattern_panel_cannot_bypass_typed_collider_writer(self) -> None:
        panel = compact(
            function_body(
                self.balance_source,
                "void Client::CBalanceTool::RenderValtanManagedPattern(",
            )
        )
        for token in (
            "if (stage.bHasHitActivation)",
            '"Timing ACTIVE_WINDOW | start %u ms | lifetime %u ms | per target ONCE"',
            '"Collider geometry, timing, response, and damage are written only through Valtan Action Workbench typed Details."',
        ):
            self.assertIn(token, panel)
        for bypass in (
            'EditFloat("Hit outer radius m"',
            'EditFloat("Hit inner radius m"',
            'EditFloat("Hit angle degrees"',
            'EditFloat("Hit length m"',
            'EditFloat("Hit half width m"',
            'EditU32("Hit count"',
            'EditU32("First hit offset ms"',
            'EditU32("Hit interval ms"',
            'EditU32("Damage rate %"',
            'EditFloat("Push range m (negative pulls)"',
        ):
            self.assertNotIn(bypass, panel)

    def test_debug_metadata_retains_active_window_clock(self) -> None:
        begin = self.header.index("struct PATTERN_HIT_AREA_DEBUG")
        end = self.header.index("};", begin)
        metadata = self.header[begin:end]
        for token in (
            "bool_t bHasActivation = false;",
            "uint32_t iActivationStartMs = 0u;",
            "uint32_t iActivationLifetimeMs = 0u;",
        ):
            self.assertIn(token, metadata)

    def test_product_debug_loader_admits_zero_pulse_active_window(self) -> None:
        load = compact(
            function_body(
                self.source,
                "void CValtan::Load_PatternHitAreaDebug()",
            )
        )
        for token in (
            '"NONE" != stage.hitShape && (0u != stage.iHitCount || stage.bHasHitActivation)',
            "area.bHasActivation = stage.bHasHitActivation;",
            "area.iActivationStartMs = stage.iHitActivationStartMs;",
            "area.iActivationLifetimeMs = stage.iHitActivationLifetimeMs;",
        ):
            self.assertIn(token, load)

    def test_local_preview_retains_zero_pulse_active_window(self) -> None:
        preview = compact(
            function_body(
                self.source,
                "bool_t CValtan::Stage_LocalPatternAuthoringPreview(",
            )
        )
        for token in (
            '"NONE" != Stage.strHitShape && (0u != Stage.iHitCount || Stage.bHasHitActivation)',
            "Area.bHasActivation = Stage.bHasHitActivation;",
            "Area.iActivationStartMs = Stage.iHitActivationStartMs;",
            "Area.iActivationLifetimeMs = Stage.iHitActivationLifetimeMs;",
        ):
            self.assertIn(token, preview)

    def test_live_wire_uses_server_half_open_active_window(self) -> None:
        draw = compact(
            function_body(
                self.source,
                "void CValtan::Draw_PatternHitAreaDebug() const",
            )
        )
        for token in (
            "if (area.bHasActivation)",
            "const uint64_t iActivationEndMs =",
            "static_cast<uint64_t>(area.iActivationStartMs) + area.iActivationLifetimeMs;",
            "fAgeMs >= static_cast<f32_t>(area.iActivationStartMs)",
            "fAgeMs < static_cast<f32_t>(iActivationEndMs)",
            "isHitWindow = true;",
        ):
            self.assertIn(token, draw)
        self.assertNotIn(
            "fAgeMs <= static_cast<f32_t>(iActivationEndMs)",
            draw,
        )


if __name__ == "__main__":
    unittest.main()
