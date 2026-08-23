#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
EFFECT_TOOL_CPP = REPOSITORY_ROOT / "Client/Private/Effect_Tool.cpp"
EFFECT_TOOL_HEADER = REPOSITORY_ROOT / "Client/Public/Effect_Tool.h"
VALTAN_PATTERN_TREE_CPP = REPOSITORY_ROOT / "Client/Private/ValtanPatternTree.cpp"
VALTAN_PATTERN_TREE_HEADER = REPOSITORY_ROOT / "Client/Public/ValtanPatternTree.h"
ENCOUNTER_PATH = (
    REPOSITORY_ROOT / "Data/Encounters/Valtan/ValtanEncounter.json"
)
CUE_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
REFERENCE_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffects.json"
)
BOSS_CATALOG_PATH = REPOSITORY_ROOT / "Data/Actors/BossCatalog.json"
AUTHORED_ROOT = REPOSITORY_ROOT / "Data/Effects/Authored"
SOURCE_CATALOG_PATH = REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json"
RUNTIME_CATALOG_PATH = (
    REPOSITORY_ROOT / "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
)
V1_ALIAS_PATH = (
    REPOSITORY_ROOT
    / "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json"
)


def function_slice(text: str, signature: str, next_signature: str) -> str:
    start = text.index(signature)
    end = text.index(next_signature, start + len(signature))
    return text[start:end]


def validate_source_contract(cpp_text: str, header_text: str) -> None:
    tree = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
        "void Client::CEffect_Tool::Render_AllEffectsWindow(",
    )
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )

    forbidden = (
        'ImGui::BeginCombo("Phase"',
        "Repeat rotation in every phase",
        "Only stages with an Effect",
        "m_iValtanPhaseFilter",
        "m_bValtanRepeatRotationPerPhase",
        "m_bValtanOnlyStagesWithEffect",
        "Refresh_ValtanBossPatternEffects",
        "Unmapped Product Canary",
        "Unmapped Valtan Authoring",
    )
    for token in forbidden:
        if token in cpp_text or token in header_text:
            raise AssertionError(f"legacy Valtan phase UI survived: {token}")

    required_tree = (
        "m_ValtanPatternTree.iIntroRotationIndex",
        "m_ValtanPatternTree.Gimmicks",
        "m_ValtanPatternTree.Rotation",
        'Render_ValtanPatternNode(Pattern, "Gimmick", strSearch)',
    )
    for token in required_tree:
        if token not in tree:
            raise AssertionError(f"flat Valtan pattern tree lost: {token}")
    if "m_ValtanPatternTree.Phases" in tree:
        raise AssertionError("All Effects must not repeat patterns through phase bands")

    ordered_tokens = (
        "Clip.ProductCues",
        "Stage.Effects",
        "Stage.CombatObjectEffects",
        'ImGui::SeparatorText("Saved Unified Effects")',
        'ImGui::SeparatorText("Animations / Semantic Stages")',
        "Render_ValtanStageRow(Stage)",
    )
    for token in ordered_tokens:
        if token not in pattern:
            raise AssertionError(f"Valtan Saved projection lost: {token}")
    positions = [pattern.index(token) for token in ordered_tokens]
    if positions != sorted(positions):
        raise AssertionError(
            "Saved Effect projection must precede animation/stage diagnostics"
        )
    required_pattern = (
        "SavedRowIndices",
        "ProductSources",
        "ReferenceStages",
        "CombatObjectStages",
        'ImGui::SmallButton("Open Saved Effect")',
        'ImGui::SmallButton("Play Saved Effect")',
        "Try_OpenValtanAuthoredEffect",
        "Try_PlayValtanSavedUnifiedEffect",
        "Try_OpenValtanSavedReferenceEffect",
        "pNonProductStage->ClipOccurrences",
    )
    for token in required_pattern:
        if token not in pattern:
            raise AssertionError(f"Valtan Saved row contract lost: {token}")
    if "Try_PlaySavedUnifiedEffect(" in pattern:
        raise AssertionError("Valtan must not use the Player-only saved play path")


def validate_v1_alias_projection_contract(
    cpp_text: str, tree_cpp_text: str, tree_header_text: str
) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    required_tree = (
        "std::string strV1EffectAssetId;",
        "Cue.strV1EffectAssetId = SourceCue.strV1EffectAssetId;",
        "CValtanPatternEffectCueDocument::Load_Source(CueDocument, Error)",
    )
    combined_tree = tree_header_text + "\n" + tree_cpp_text
    for token in required_tree:
        if token not in combined_tree:
            raise AssertionError(f"typed V1 alias propagation lost: {token}")

    required_pattern = (
        "Cue.strV1EffectAssetId",
        "ResolveRow(Cue.strV1EffectAssetId)",
        "V1Row.bV1Alias = true",
        '"[V0] "',
        '"[V1] "',
        "PlaybackCue.strEffectAssetId = Row.strEffectAssetId",
    )
    for token in required_pattern:
        if token not in pattern:
            raise AssertionError(f"paired V0/V1 Saved row contract lost: {token}")
    if re.search(r"strEffectAssetId\s*\+\s*[^;]*v1\.unified", pattern):
        raise AssertionError("V1 Saved rows must consume the typed alias, not infer a suffix")


def validate_drawable_preflight_contract(cpp_text: str) -> None:
    play_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_PlayValtanSavedUnifiedEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
    )
    reference_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
    )
    product_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
        "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree(",
    )

    for name, helper, commit_token in (
        ("play", play_helper, "Is_UnifiedEffectActive(Cache)"),
        ("reference", reference_helper, "Try_LoadDocumentPath("),
        ("product", product_helper, "Try_LoadDocumentPath("),
    ):
        refresh = helper.find("Refresh_UnifiedEffectCache(")
        valid = helper.find("!Cache.bValid")
        drawable = helper.find("!Cache.bDrawable")
        commit = helper.find(commit_token)
        if min(refresh, valid, drawable, commit) < 0:
            raise AssertionError(f"{name} helper lost the full drawable preflight")
        if not (refresh < valid < commit and refresh < drawable < commit):
            raise AssertionError(
                f"{name} helper must reject invalid/non-drawable data before preview mutation"
            )

    refresh_tree = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree(",
        "bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(",
    )
    if "m_ValtanUnifiedEffectCaches.clear();" not in refresh_tree:
        raise AssertionError("explicit Valtan Refresh must reopen repaired documents")

    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    open_button = pattern.find('ImGui::SmallButton("Open Saved Effect")')
    play_button = pattern.find('ImGui::SmallButton("Play Saved Effect")')
    refreshed_lookup = pattern.find("const auto RefreshedCache =", play_button)
    if min(open_button, play_button, refreshed_lookup) < 0 or not (
        open_button < play_button < refreshed_lookup
    ):
        raise AssertionError(
            "Saved row must reacquire its unordered cache iterator after button actions"
        )
    if "ObservedCache->" in pattern[open_button:]:
        raise AssertionError(
            "Saved row retained a possibly invalidated cache iterator across Open/Play"
        )


def validate_animation_stage_metadata_contract(cpp_text: str) -> None:
    sections = (
        (
            "Render_ValtanStageRow",
            function_slice(
                cpp_text,
                "void Client::CEffect_Tool::Render_ValtanStageRow(",
                "void Client::CEffect_Tool::Render_ValtanClipOccurrence(",
            ),
        ),
        (
            "Render_ValtanClipOccurrence",
            function_slice(
                cpp_text,
                "void Client::CEffect_Tool::Render_ValtanClipOccurrence(",
                "void Client::CEffect_Tool::Render_ValtanProductCue(",
            ),
        ),
        (
            "Render_ValtanProductCue",
            function_slice(
                cpp_text,
                "void Client::CEffect_Tool::Render_ValtanProductCue(",
                "void Client::CEffect_Tool::Render_ValtanPatternNode(",
            ),
        ),
    )
    forbidden = (
        "Refresh_UnifiedEffectCache(",
        "Render_UnifiedEffectTree(",
        "Render_ValtanAuthoringOpenButton(",
    )
    for section_name, section in sections:
        for token in forbidden:
            if token in section:
                raise AssertionError(
                    f"{section_name} must remain animation/metadata-only: {token}"
                )


def validate_stage_reference_sequence_contract(cpp_text: str) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    reference_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
    )

    vector_parameter = re.search(
        r"const\s+std::vector\s*<\s*VALTAN_CLIP_OCCURRENCE_VIEW\s*>\s*&\s*"
        r"([A-Za-z_][A-Za-z0-9_]*)",
        reference_helper,
    )
    if vector_parameter is None:
        raise AssertionError(
            "saved stage-reference helper must receive the complete clip vector"
        )
    clips_parameter = vector_parameter.group(1)
    if re.search(
        rf"Play_ValtanStageSequence\s*\(\s*{re.escape(clips_parameter)}\s*\)",
        reference_helper,
    ) is None:
        raise AssertionError(
            "saved stage-reference helper must play the complete clip vector"
        )

    forbidden_pattern_tokens = (
        "1u == Stage.ClipOccurrences.size()",
        "Stage.ClipOccurrences.front()",
        "bAmbiguousReferenceAnimation",
        "choose a clip below",
    )
    for token in forbidden_pattern_tokens:
        if token in pattern:
            raise AssertionError(
                f"saved stage-reference rows must not select/disable by clip count: {token}"
            )
    if "Play_ValtanClipOccurrence(" in reference_helper or ".front()" in reference_helper:
        raise AssertionError(
            "saved stage-reference helper must not collapse a sequence to its first clip"
        )

    reference_calls = re.findall(
        r"Try_OpenValtanSavedReferenceEffect\s*\((.*?)\);",
        pattern,
        flags=re.DOTALL,
    )
    if not reference_calls:
        raise AssertionError("saved stage-reference Open/Play actions are missing")
    for call in reference_calls:
        if re.search(
            r"(?:ClipOccurrences|Row\.[A-Za-z_][A-Za-z0-9_]*Clips[A-Za-z0-9_]*)",
            call,
        ) is None:
            raise AssertionError(
                "saved stage-reference Open/Play must pass all stage clip occurrences"
            )


def validate_saved_row_aggregation_contract(cpp_text: str) -> None:
    pattern = function_slice(
        cpp_text,
        "void Client::CEffect_Tool::Render_ValtanPatternNode(",
        "void Client::CEffect_Tool::Render_ValtanPatternTreeSection(",
    )
    if "SeenEffectAssetIds" in pattern:
        raise AssertionError(
            "saved rows must aggregate duplicate provenance, not use a first-wins set"
        )
    if re.search(
        r"\.insert\s*\([^;]*strEffectAssetId[^;]*\.second\s*\)\s*continue\s*;",
        pattern,
        flags=re.DOTALL,
    ) is not None:
        raise AssertionError(
            "saved rows must not discard later provenance after duplicate insertion"
        )
    if re.search(
        r"std::(?:unordered_)?map\s*<\s*std::string\s*,",
        pattern,
    ) is None:
        raise AssertionError(
            "saved rows need an effectAssetId index that resolves to an aggregate row"
        )

    row_start = pattern.find("struct SAVED_VALTAN_EFFECT_ROW")
    row_end = pattern.find("};", row_start)
    if row_start < 0 or row_end < 0:
        raise AssertionError("saved Valtan row structure is missing")
    row_structure = pattern[row_start:row_end]
    provenance_members = re.findall(
        r"std::vector\s*<[^;]+>\s+([A-Za-z_][A-Za-z0-9_]*)\s*;",
        row_structure,
    )
    provenance_members = [
        member
        for member in provenance_members
        if re.search(
            r"(?:provenance|source|origin|link|occurrence|cue|reference|combat)",
            member,
            flags=re.IGNORECASE,
        )
    ]
    if not provenance_members:
        raise AssertionError(
            "each saved row must retain aggregated cue/reference/combat provenance"
        )
    if not any(
        re.search(rf"\.{re.escape(member)}\.push_back\s*\(", pattern)
        for member in provenance_members
    ):
        raise AssertionError("saved row provenance is declared but never aggregated")

    has_path_fallback = False
    for empty_check in re.finditer(
        r"\b([A-Za-z_][A-Za-z0-9_]*)\.Path\.empty\s*\(\s*\)",
        pattern,
    ):
        owner = empty_check.group(1)
        fallback_window = pattern[empty_check.end() : empty_check.end() + 500]
        if re.search(rf"\b{re.escape(owner)}\.Path\s*=", fallback_window):
            has_path_fallback = True
            break
    if not has_path_fallback:
        raise AssertionError(
            "later provenance must fill an aggregate row whose saved path is still empty"
        )
    if (
        "The catalog path is authoritative" not in pattern
        or "Row.Path = Editable->second.Path" not in pattern
    ):
        raise AssertionError(
            "EffectCatalog must override a legacy reference path after provenance merge"
        )


def validate_saved_reference_load_guard_contract(cpp_text: str) -> None:
    reference_helper = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(",
        "bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(",
    )
    load_guard = reference_helper.find("Try_LoadDocumentPath(")
    if load_guard < 0:
        raise AssertionError("saved reference helper lost its unsaved-document load guard")
    target_selection = re.search(
        r"m_pCharacterPreviewPanel->Select_TargetAsset\s*\(",
        reference_helper,
    )
    if target_selection is not None and target_selection.start() < load_guard:
        raise AssertionError(
            "saved reference helper must not switch target before the load guard commits"
        )


def validate_pending_reference_preview_contract(cpp_text: str) -> None:
    execute = function_slice(
        cpp_text,
        "bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(",
        "bool_t Client::CEffect_Tool::Refresh_AllEffects(",
    )
    required = (
        "Pending.ValtanReferenceClips.has_value()",
        "Play_ValtanStageSequence(Clips)",
        "Try_PlayActiveUnifiedEffect()",
        "CompleteValtanPreviewPartial",
    )
    for token in required:
        if token not in execute:
            raise AssertionError(
                f"pending saved-reference preview contract lost: {token}"
            )


def stage_effect_asset_id(pattern: dict[str, object], stage: dict[str, object]) -> str:
    pattern_action = str(pattern["actionId"])
    stage_action = str(stage["actionId"])
    pieces = pattern_action.split(".", 2)
    pattern_slug = pieces[2] if len(pieces) == 3 else pattern_action
    prefix = pattern_action + "."
    if stage_action.startswith(prefix) and len(stage_action) > len(prefix):
        stage_slug = stage_action[len(prefix) :]
    else:
        stage_slug = str(stage["stageId"]).lower().replace("_", "-")
    return f"effect.valtan.{pattern_slug}.{stage_slug}"


def project_saved_rows(include_v1_aliases: bool = True) -> tuple[dict[str, list[str]], int]:
    encounter = json.loads(ENCOUNTER_PATH.read_text(encoding="utf-8"))
    cue_document = json.loads(CUE_PATH.read_text(encoding="utf-8"))
    reference_document = json.loads(REFERENCE_PATH.read_text(encoding="utf-8"))
    boss_catalog = json.loads(BOSS_CATALOG_PATH.read_text(encoding="utf-8"))
    alias_document = json.loads(V1_ALIAS_PATH.read_text(encoding="utf-8"))
    v1_aliases = {
        row["effectAssetId"]: row["v1EffectAssetId"]
        for row in alias_document["aliases"]
    }

    cues_by_stage: dict[tuple[str, str, str], list[str]] = {}
    for cue in cue_document["cues"]:
        key = (cue["patternId"], cue["stageId"], cue["actionId"])
        cues_by_stage.setdefault(key, []).append(cue["effectAssetId"])
    references_by_action = {
        binding["actionId"]: binding["effectAssetId"]
        for binding in reference_document["bindings"]
    }
    valtan = next(
        boss
        for boss in boss_catalog["bosses"]
        if boss["archetypeId"] == "BOSS_VALTAN"
    )
    combat_visuals = {
        visual["combatObjectArchetypeId"]: visual["effectAssetId"]
        for visual in valtan["combatObjectVisuals"]
    }

    projected: dict[str, list[str]] = {}
    raw_links = 0
    for pattern in encounter["patterns"]:
        ordered: list[str] = []
        seen: set[str] = set()

        def add(effect_id: str) -> None:
            nonlocal raw_links
            raw_links += 1
            if effect_id not in seen:
                seen.add(effect_id)
                ordered.append(effect_id)

        for stage in pattern["stages"]:
            key = (pattern["patternId"], stage["stageId"], stage["actionId"])
            product_ids = cues_by_stage.get(key, [])
            for effect_id in product_ids:
                add(effect_id)
                if include_v1_aliases and effect_id in v1_aliases:
                    add(v1_aliases[effect_id])

            explicit = references_by_action.get(stage["actionId"])
            if explicit:
                add(explicit)

            combat_ids: list[str] = []
            for action in stage.get("actions", []):
                if action.get("kind") != "SPAWN_COMBAT_OBJECT":
                    continue
                effect_id = combat_visuals.get(action.get("targetId"))
                if effect_id:
                    combat_ids.append(effect_id)

            if not product_ids and not combat_ids:
                candidate = stage_effect_asset_id(pattern, stage)
                if (AUTHORED_ROOT / f"{candidate}.effect.json").is_file():
                    add(candidate)
            for effect_id in combat_ids:
                add(effect_id)

        projected[pattern["patternId"]] = ordered
    return projected, raw_links


class EffectToolValtanSavedRowsTests(unittest.TestCase):
    def test_source_contract_is_flat_lazy_and_valtan_specific(self) -> None:
        validate_source_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            EFFECT_TOOL_HEADER.read_text(encoding="utf-8"),
        )

    def test_typed_v1_aliases_project_as_paired_saved_rows(self) -> None:
        validate_v1_alias_projection_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8"),
            VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8"),
            VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8"),
        )

    def test_saved_open_and_play_fail_closed_before_preview_mutation(self) -> None:
        validate_drawable_preflight_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_animation_stage_rows_do_not_decode_or_open_saved_documents(self) -> None:
        validate_animation_stage_metadata_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_stage_reference_play_uses_the_complete_clip_sequence(self) -> None:
        validate_stage_reference_sequence_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_saved_rows_aggregate_all_provenance_and_path_fallbacks(self) -> None:
        validate_saved_row_aggregation_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_saved_reference_load_guard_precedes_target_selection(self) -> None:
        validate_saved_reference_load_guard_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_pending_saved_reference_replays_animation_and_effect(self) -> None:
        validate_pending_reference_preview_contract(
            EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        )

    def test_live_data_projects_every_owned_saved_document_once_per_pattern(self) -> None:
        projected, raw_links = project_saved_rows()
        self.assertEqual(33, len(projected))
        self.assertEqual(113, raw_links)
        self.assertEqual(112, sum(len(rows) for rows in projected.values()))
        self.assertEqual(4, len(projected["VALTAN_DASH_CHARGE"]))
        self.assertEqual(3, len(projected["VALTAN_FRONT_BACK_FRONT"]))
        self.assertEqual(4, len(projected["VALTAN_FOUR_SLASH"]))
        self.assertEqual(4, len(projected["VALTAN_WHIRLWIND"]))
        self.assertEqual(
            len(projected["VALTAN_WHIRLWIND"]),
            len(set(projected["VALTAN_WHIRLWIND"])),
        )

    def test_base_saved_projection_inventory_requires_explicit_migration(self) -> None:
        # This is a named UI inventory snapshot, not the full authored corpus.
        # A legitimate Product-coverage migration updates these counts explicitly.
        projected, raw_links = project_saved_rows(include_v1_aliases=False)
        self.assertEqual(107, raw_links)
        self.assertEqual(106, sum(len(rows) for rows in projected.values()))
        catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        nonempty_candidates = 0
        empty_shells = 0
        for effect_ids in projected.values():
            for effect_id in effect_ids:
                entry = catalog.get(effect_id)
                path = (
                    REPOSITORY_ROOT / "Data" / entry["authoringPath"]
                    if entry is not None
                    else AUTHORED_ROOT / f"{effect_id}.effect.json"
                )
                document = json.loads(path.read_text(encoding="utf-8"))
                if document.get("elements") or document.get("modelCues"):
                    nonempty_candidates += 1
                else:
                    empty_shells += 1
        self.assertEqual(49, nonempty_candidates)
        self.assertEqual(57, empty_shells)

    def test_all_declared_v0_product_cues_remain_published_and_nonempty(self) -> None:
        cues = json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        source_catalog = json.loads(
            SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
        )["effects"]
        runtime_catalog = json.loads(
            RUNTIME_CATALOG_PATH.read_text(encoding="utf-8")
        )["effects"]
        effect_ids = [cue["effectAssetId"] for cue in cues]
        self.assertEqual(44, len(effect_ids))
        self.assertEqual(44, len(set(effect_ids)))
        visible_elements = 0
        hidden_elements = 0
        model_cues = 0
        for effect_id in effect_ids:
            source_matches = [
                row for row in source_catalog if row["effectAssetId"] == effect_id
            ]
            runtime_matches = [
                row for row in runtime_catalog if row["effectAssetId"] == effect_id
            ]
            self.assertEqual(1, len(source_matches), effect_id)
            self.assertEqual(1, len(runtime_matches), effect_id)
            source_entry = source_matches[0]
            source_path = REPOSITORY_ROOT / "Data" / source_entry["authoringPath"]
            source_document = json.loads(source_path.read_text(encoding="utf-8"))
            self.assertEqual(effect_id, source_document["effectAssetId"])
            self.assertTrue(
                source_document.get("elements") or source_document.get("modelCues"),
                effect_id,
            )
            visible_elements += sum(
                1 for element in source_document.get("elements", [])
                if element.get("visible", True)
            )
            hidden_elements += sum(
                1 for element in source_document.get("elements", [])
                if not element.get("visible", True)
            )
            model_cues += len(source_document.get("modelCues", []))
            self.assertTrue(
                any(
                    element.get("visible", True)
                    for element in source_document.get("elements", [])
                ) or source_document.get("modelCues"),
                effect_id,
            )
            runtime_entry = runtime_matches[0]
            runtime_path = (
                REPOSITORY_ROOT
                / "Client/Bin/DataFiles/Effect"
                / runtime_entry["authoredDocumentPath"]
            )
            self.assertTrue(runtime_path.is_file(), effect_id)
            runtime_bytes = runtime_path.read_bytes()
            hash_match = re.search(
                r"\.([0-9a-f]{64})\.effect\.json$", runtime_path.name
            )
            self.assertIsNotNone(hash_match, effect_id)
            self.assertEqual(
                hash_match.group(1), hashlib.sha256(runtime_bytes).hexdigest()
            )
            runtime_document = json.loads(runtime_bytes.decode("utf-8"))
            self.assertEqual(source_document, runtime_document, effect_id)
        self.assertEqual(657, visible_elements)
        self.assertEqual(4, hidden_elements)
        self.assertEqual(0, model_cues)

    def test_v1_alias_sidecar_is_exactly_six_valid_pairs(self) -> None:
        aliases = json.loads(V1_ALIAS_PATH.read_text(encoding="utf-8"))["aliases"]
        cue_ids = {
            cue["effectAssetId"]
            for cue in json.loads(CUE_PATH.read_text(encoding="utf-8"))["cues"]
        }
        source_catalog = {
            row["effectAssetId"]: row
            for row in json.loads(
                SOURCE_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        runtime_ids = {
            row["effectAssetId"]
            for row in json.loads(
                RUNTIME_CATALOG_PATH.read_text(encoding="utf-8")
            )["effects"]
        }
        self.assertEqual(6, len(aliases))
        self.assertEqual(6, len({row["effectAssetId"] for row in aliases}))
        self.assertEqual(6, len({row["v1EffectAssetId"] for row in aliases}))
        for row in aliases:
            self.assertIn(row["effectAssetId"], cue_ids)
            self.assertIn(row["effectAssetId"], source_catalog)
            self.assertIn(row["v1EffectAssetId"], source_catalog)
            self.assertIn(row["v1EffectAssetId"], runtime_ids)
            self.assertTrue(row["v1EffectAssetId"].endswith(".v1.unified"))
            v1_path = (
                REPOSITORY_ROOT
                / "Data"
                / source_catalog[row["v1EffectAssetId"]]["authoringPath"]
            )
            v1_document = json.loads(v1_path.read_text(encoding="utf-8"))
            self.assertTrue(
                v1_document.get("elements") or v1_document.get("modelCues"),
                row["v1EffectAssetId"],
            )

    def test_ui_contract_mutations_fail_closed(self) -> None:
        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        header_text = EFFECT_TOOL_HEADER.read_text(encoding="utf-8")
        mutations = (
            cpp_text.replace("Clip.ProductCues", "Clip.RemovedCues"),
            cpp_text.replace("Stage.Effects", "Stage.RemovedEffects"),
            cpp_text.replace("Open Saved Effect", "Open Effect"),
            cpp_text.replace(
                "Try_PlayValtanSavedUnifiedEffect",
                "Try_PlaySavedUnifiedEffect",
            ),
        )
        for mutation in mutations:
            with self.subTest(mutation=mutations.index(mutation)):
                with self.assertRaises(AssertionError):
                    validate_source_contract(mutation, header_text)

    def test_v1_and_drawable_gate_mutations_fail_closed(self) -> None:
        cpp_text = EFFECT_TOOL_CPP.read_text(encoding="utf-8")
        tree_cpp_text = VALTAN_PATTERN_TREE_CPP.read_text(encoding="utf-8")
        tree_header_text = VALTAN_PATTERN_TREE_HEADER.read_text(encoding="utf-8")
        v1_mutations = (
            cpp_text.replace(
                "ResolveRow(Cue.strV1EffectAssetId)",
                "ResolveRow(Cue.strEffectAssetId)",
            ),
            cpp_text.replace(
                "PlaybackCue.strEffectAssetId = Row.strEffectAssetId",
                "PlaybackCue.strEffectAssetId = pProductSource->pCue->strEffectAssetId",
            ),
        )
        for mutation in v1_mutations:
            with self.assertRaises(AssertionError):
                validate_v1_alias_projection_contract(
                    mutation, tree_cpp_text, tree_header_text
                )
        with self.assertRaises(AssertionError):
            validate_drawable_preflight_contract(
                cpp_text.replace("!Cache.bDrawable", "false", 1)
            )
        with self.assertRaises(AssertionError):
            validate_drawable_preflight_contract(
                cpp_text.replace("RefreshedCache", "ObservedCache")
            )


if __name__ == "__main__":
    unittest.main()
