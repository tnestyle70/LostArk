from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "Client/Public/EffectV2_Catalog.h"
SOURCE = ROOT / "Client/Private/EffectV2_Catalog.cpp"
PROJECT = ROOT / "Client/Default/Client.vcxproj"
FILTERS = ROOT / "Client/Default/Client.vcxproj.filters"
AUTHORED = ROOT / "Data/Effects/V2/Authored"
GROUPS = ROOT / "Data/Effects/V2/Groups"
BOSS_BINDINGS = ROOT / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
VALTAN_PRESENTATION = ROOT / "Data/Valtan/Valtan.presentation.json"
EFFECT_TOOL_V2_HEADER = ROOT / "Client/Public/Effect_Tool_V2.h"
EFFECT_TOOL_V2_SOURCE = ROOT / "Client/Private/Effect_Tool_V2.cpp"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig")


def function_tail(text: str, signature: str, next_signature: str) -> str:
    begin = text.index(signature)
    end = text.index(next_signature, begin + len(signature))
    return text[begin:end]


class EffectV2CatalogContractTests(unittest.TestCase):
    def test_valtan_pattern_view_preserves_last_good_and_separates_display_from_mutation(self) -> None:
        header = read(EFFECT_TOOL_V2_HEADER)
        source = read(EFFECT_TOOL_V2_SOURCE)
        reload_body = function_tail(
            source,
            "bool_t Client::CEffect_Tool_V2::Reload_ValtanTree(",
            "bool_t Client::CEffect_Tool_V2::Build_ValtanTimeline(",
        )
        render_body = function_tail(
            source,
            "void Client::CEffect_Tool_V2::Render_ValtanPatternSection()",
            "bool_t Client::CEffect_Tool_V2::Try_PlayValtanServerPattern()",
        )

        for token in (
            '#include "ValtanViewAdmission.h"',
            "VALTAN_VIEW_ADMISSION m_eValtanTreeAdmission",
            "m_bValtanTreeReloadRetryPending",
            "m_iValtanTreeAutomaticRetryCount",
        ):
            self.assertIn(token, header)
        self.assertNotIn("m_bValtanTreeLoaded", header + source)

        for token in (
            "VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;",
            "CValtanPatternTree::Load(Staged, Diagnostic)",
            "Diagnostic.Is_AutomaticRetryable()",
            "Diagnostic.Requires_ProductProjection()",
            "VALTAN_CANONICAL_READ_DIAGNOSTIC::PRODUCT_PROJECTION_COMMAND",
            "VALTAN_VIEW_ADMISSION::STALE_PRESERVED",
            "STALE_PRESERVED / READ ONLY",
            "m_ValtanTree = std::move(Staged);",
            "m_eValtanTreeAdmission = VALTAN_VIEW_ADMISSION::ADMITTED;",
        ):
            self.assertIn(token, reload_body)
        failure_region = reload_body[
            reload_body.index("if (!CValtanPatternTree::Load(Staged, Diagnostic))") :
            reload_body.index("m_ValtanTree = std::move(Staged);")
        ]
        self.assertNotIn("m_ValtanTree = {}", failure_region)
        self.assertNotIn("m_ValtanPatterns.clear()", failure_region)

        for token in (
            "Can_DisplayValtanView(m_eValtanTreeAdmission)",
            "Can_MutateValtanView(m_eValtanTreeAdmission)",
            '"STALE_PRESERVED / READ ONLY"',
            "!bHasPattern || !bMutationAdmitted",
            '"Refresh Pattern View"',
        ):
            self.assertIn(token, render_body)

        for signature in (
            "bool_t Client::CEffect_Tool_V2::Build_ValtanTimeline(",
            "bool_t Client::CEffect_Tool_V2::Apply_ValtanTimeline(",
            "bool_t Client::CEffect_Tool_V2::Try_PlayValtanServerPattern(",
        ):
            body = source[source.index(signature) :]
            body = body[: body.index("\n}") + 2]
            self.assertIn("Can_MutateValtanView(m_eValtanTreeAdmission)", body)

    def test_valtan_pattern_view_automatic_retry_is_bounded_and_render_driven(self) -> None:
        source = read(EFFECT_TOOL_V2_SOURCE)
        schedule = function_tail(
            source,
            "bool_t Client::CEffect_Tool_V2::Schedule_ValtanTreeReloadRetry()",
            "bool_t Client::CEffect_Tool_V2::Reload_ValtanTree(",
        )
        render = function_tail(
            source,
            "void Client::CEffect_Tool_V2::Render()",
            "void Client::CEffect_Tool_V2::Scan_Resources()",
        )
        for token in (
            "VALTAN_TREE_MAX_AUTOMATIC_RETRY_COUNT",
            "m_iValtanTreeAutomaticRetryCount",
            "m_bValtanTreeReloadRetryPending = false",
            "m_dNextValtanTreeReloadRetrySeconds",
        ):
            self.assertIn(token, schedule)
        self.assertIn("ImGui::GetTime()", schedule)
        self.assertIn("m_bValtanTreeReloadRetryPending", render)
        self.assertIn("Reload_ValtanTree(false)", render)

    def test_effect_attach_never_writes_the_boss_valtan_binding_owner_directly(self) -> None:
        catalog_header = read(HEADER)
        catalog_source = read(SOURCE)
        tool_source = read(EFFECT_TOOL_V2_SOURCE)
        save = function_tail(
            tool_source,
            "bool_t Client::CEffect_Tool_V2::Save_Bindings()",
            "void Client::CEffect_Tool_V2::Render_AttachWindow()",
        )
        render = function_tail(
            tool_source,
            "void Client::CEffect_Tool_V2::Render_AttachWindow()",
            "void Client::CEffect_Tool_V2::Scan_Groups()",
        )

        valtan_guard = save.index(
            "if (VALTAN_TARGET_ARCHETYPE_ID == m_strTargetArchetypeId)"
        )
        generic_write = save.index("CEffectV2Document::Write_AtomicFile(")
        self.assertLess(valtan_guard, generic_write)
        self.assertIn("return false;", save[valtan_guard:generic_write])
        for token in (
            "BOSS_VALTAN bindings are read-only in Effect Attach v2",
            "stage them in Action Composition",
            "one canonical transaction",
            "No binding owner was written",
        ):
            self.assertIn(token, save)
        self.assertIn(
            "CEffectV2Document::Binding_Path(m_strTargetArchetypeId)", save
        )
        self.assertNotIn(
            "CEffectV2Document::Binding_Path(VALTAN_TARGET_ARCHETYPE_ID)",
            tool_source,
        )

        for token in (
            "const bool_t bDirectBindingMutationAllowed =",
            "VALTAN_TARGET_ARCHETYPE_ID != m_strTargetArchetypeId",
            "BOSS_VALTAN bindings are READ ONLY here",
            "!bDirectBindingMutationAllowed",
        ):
            self.assertIn(token, render)

        self.assertNotIn("CEffectV2Document::Write_AtomicFile(", catalog_source)
        self.assertNotIn("bWriteOwner", catalog_header + catalog_source)
        for forbidden in (
            "bool_t Append_BossValtanStageBinding(",
            "bool_t Remove_BossValtanStageBinding(",
            "bool_t Duplicate_BossValtanStageBinding(",
            "bool_t Update_BossValtanStageBindingStart(",
        ):
            self.assertNotIn(forbidden, catalog_header + catalog_source)

        for required in (
            "Stage_AppendBossValtanStageBinding(",
            "Stage_RemoveBossValtanStageBinding(",
            "Stage_DuplicateBossValtanStageBinding(",
            "Stage_UpdateBossValtanStageBindingStart(",
            "Prepare_BossValtanBindingDraftSave(",
            "Accept_BossValtanBindingDraftSave(",
        ):
            self.assertIn(required, catalog_header)

    def test_effect_tool_v2_data_files_lists_groups_and_openable_children(self) -> None:
        header = read(EFFECT_TOOL_V2_HEADER)
        source = read(EFFECT_TOOL_V2_SOURCE)
        panel = function_tail(
            source,
            "void Client::CEffect_Tool_V2::Render_DocumentPanel()",
            "namespace\n{\n\tconstexpr const wchar_t* TARGET_LAYER_TAG",
        )
        scan = function_tail(
            source,
            "void Client::CEffect_Tool_V2::Scan_Groups()",
            "bool_t Client::CEffect_Tool_V2::Load_Group(",
        )
        self.assertIn("std::vector<EFFECT_V2_GROUP> m_GroupLibrary;", header)
        for token in (
            'SeparatorText("Valtan Effect Resources")',
            "for (const EFFECT_V2_GROUP& Group : m_GroupLibrary)",
            "for (const EFFECT_V2_GROUP_CHILD& Child : Group.Children)",
            "Load_Group(Group.strGroupId)",
            "Load_Document(Child.strEffectId)",
        ):
            self.assertIn(token, panel)
        self.assertIn("std::vector<EFFECT_V2_GROUP> StagedGroups;", scan)
        self.assertIn("Load_GroupFile(strGroupId, Group, strError)", scan)
        self.assertLess(scan.index("StagedGroups"), scan.index("m_GroupLibrary ="))

    def test_boss_bindings_are_one_canonical_strict_v2_document(self) -> None:
        document = json.loads(BOSS_BINDINGS.read_text(encoding="utf-8"))
        self.assertEqual(
            set(document), {"schema", "formatVersion", "archetypeId", "bindings"}
        )
        self.assertEqual(document["schema"], "lostark.effect-v2-bindings")
        self.assertEqual(document["formatVersion"], 2)
        self.assertEqual(document["archetypeId"], "BOSS_VALTAN")
        binding_ids = [binding["bindingId"] for binding in document["bindings"]]
        self.assertEqual(binding_ids, sorted(binding_ids))
        self.assertEqual(len(binding_ids), len(set(binding_ids)))
        self.assertGreater(len(binding_ids), 0)
        for binding in document["bindings"]:
            self.assertEqual(
                set(binding),
                {"bindingId", "resource", "scope", "clock", "anchor", "stopPolicy"},
            )
            self.assertEqual(set(binding["resource"]), {"kind", "id"})
            self.assertEqual(
                set(binding["scope"]), {"patternId", "stageId", "actionId"}
            )
            self.assertEqual(
                set(binding["clock"]),
                {"basis", "clipOccurrenceId", "startMs", "repeatPolicy"},
            )
            self.assertEqual(
                set(binding["anchor"]),
                {"slotId", "followPolicy", "rotationBasis", "localTransform"},
            )

    def test_catalog_is_registered_as_one_effect_v2_source_pair(self) -> None:
        project = read(PROJECT)
        filters = read(FILTERS)
        for text in (project, filters):
            self.assertEqual(text.count(r"..\Public\EffectV2_Catalog.h"), 1)
            self.assertEqual(text.count(r"..\Private\EffectV2_Catalog.cpp"), 1)
        self.assertIn("03. Tools\\06. Effect V2", filters)

    def test_snapshot_is_read_only_and_queries_do_no_io(self) -> None:
        header = read(HEADER)
        source = read(SOURCE)
        for token in (
            "std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>",
            "const std::vector<EFFECT_V2_DOCUMENT>&",
            "const std::vector<EFFECT_V2_GROUP>&",
            "const std::vector<EFFECT_V2_BINDING>&",
            "const std::vector<std::string>&",
            "Find_Document(",
            "Find_Group(",
            "Get_Diagnostics()",
            "Has_IsolatedItems()",
            "Can_MutateBossValtanBindings()",
        ):
            self.assertIn(token, header)

        query_region = source[source.index(
            "uint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision()"
        ) : source.index("Client::CEffectV2Catalog& Client::CEffectV2Catalog::Get()")]
        for forbidden in (
            "filesystem",
            "Load_DocumentFile",
            "Load_GroupFile",
            "Load_BindingsFile",
            "directory_iterator",
        ):
            self.assertNotIn(forbidden, query_region)
        self.assertNotIn("Render", header)
        self.assertNotIn("Tick", header)

    def test_explicit_reload_commits_one_strict_snapshot(self) -> None:
        source = read(SOURCE)
        reload_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Reload_BossValtan(",
            "Discard_BossValtanBindingDraftAndReload(",
        )
        ordered = (
            "!Stage_Documents(",
            "!Stage_Groups(",
            "Stage_BossValtanBindings(",
            "Isolate_InvalidCrossReferences(",
            "std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>()",
            "pStaged->m_Diagnostics = std::move(Diagnostics);",
            "pStaged->m_iRevision = iPreviousRevision + 1u;",
            "m_pSnapshot = std::move(pStaged);",
        )
        positions = [reload_body.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertEqual(reload_body.count("m_pSnapshot ="), 1)
        self.assertIn("return false;", reload_body[: positions[5]])
        self.assertIn("const std::lock_guard Lock(m_SnapshotMutex);", reload_body)
        self.assertIn("failed before commit", reload_body)

    def test_explicit_discard_reloads_before_replacing_the_v2_draft(self) -> None:
        source = read(SOURCE)
        discard = function_tail(
            source,
            "Discard_BossValtanBindingDraftAndReload(",
            "bool_t Client::CEffectV2Catalog::Commit_BossValtanBindingsLocked(",
        )
        ordered = (
            "!Stage_Documents(",
            "!Stage_Groups(",
            "!Stage_BossValtanBindings(",
            "!Isolate_InvalidCrossReferences(",
            "std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>()",
            "m_pSnapshot = std::move(pStaged);",
            "m_bBossValtanBindingDraftDirty = false;",
        )
        positions = [discard.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertEqual(discard.count("m_pSnapshot ="), 1)
        self.assertIn("failed before commit", discard)
        self.assertIn("CEffectV2Runtime::Invalidate_Caches();", discard)

    def test_binding_mutation_self_validates_then_stages_for_canonical_commit(self) -> None:
        header = read(HEADER)
        source = read(SOURCE)
        commit_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Commit_BossValtanBindingsLocked(",
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
        )
        ordered = (
            "Cross_Validate(",
            "CEffectV2Document::Serialize_Bindings(",
            "CEffectV2Document::Parse_Bindings(",
            "!Stage_Documents(",
            "!Stage_Groups(",
            "Stage_BossValtanBindings(",
            "Isolate_InvalidCrossReferences(",
            "const bool_t bBindingBaselineMatches =",
            "m_pSnapshot = std::move(pCandidate);",
            "m_bBossValtanBindingDraftDirty = true;",
        )
        positions = [commit_body.index(token) for token in ordered]
        self.assertEqual(positions, sorted(positions))
        self.assertIn("Staged Effect V2 binding", commit_body)
        self.assertIn("Use Save to commit every owner together", commit_body)
        self.assertIn("CEffectV2Runtime::Invalidate_Caches();", commit_body)
        self.assertIn("source changed; reload", commit_body)
        self.assertIn("Matches_DocumentBaseline(", commit_body)
        self.assertIn("Matches_GroupBaseline(", commit_body)
        self.assertIn("DiskDocuments, DiskGroups,", commit_body)
        self.assertIn("Validate_NoLeafGroupClockOverlap(", commit_body)
        self.assertEqual(commit_body.count("m_pSnapshot ="), 1)
        self.assertNotIn("CEffectV2Document::Write_AtomicFile(", commit_body)
        self.assertNotIn("bWriteOwner", header + source)

    def test_binding_owner_is_parsed_whole_and_never_row_isolated(self) -> None:
        source = read(SOURCE)
        documents = function_tail(
            source,
            "\tbool_t Stage_Documents(",
            "\tbool_t Stage_Groups(",
        )
        self.assertIn("Load_DocumentFile(", documents)
        self.assertIn("Skipped Effect V2 document", documents)
        self.assertIn("continue;", documents)

        bindings = function_tail(
            source,
            "\tbool_t Stage_BossValtanBindings(",
            "\tbool_t Cross_Validate(",
        )
        for token in (
            "CEffectV2Document::Parse_Bindings(",
            "strText, BOSS_VALTAN_ARCHETYPE_ID, Staged, strOutError",
            "strict formatVersion 2 binding parse failed before",
            "OutBindings = std::move(Staged);",
            "bOutComplete = true;",
            "return false;",
        ):
            self.assertIn(token, bindings)
        for forbidden in (
            "CDataJson::Parse(",
            "for (size_t iRow",
            "Skipped BOSS_VALTAN Effect V2 binding row",
            "formatVersion\":1",
        ):
            self.assertNotIn(forbidden, bindings)

        isolation = function_tail(
            source,
            "\tbool_t Isolate_InvalidCrossReferences(",
            "\tstd::string Format_IsolationSummary(",
        )
        for token in (
            "ValidGroups",
            "missing authored leaf",
            "formatVersion 2 bindings cannot be admitted as a partial document",
            "Validate_NoLeafGroupClockOverlap(",
            "Groups = std::move(ValidGroups);",
        ):
            self.assertIn(token, isolation)
        self.assertNotIn("ValidBindings", isolation)
        self.assertNotIn("Bindings = std::move(ValidBindings);", isolation)

    def test_valtan_runtime_uses_catalog_revision_and_valid_subset(self) -> None:
        runtime = read(ROOT / "Client/Private/EffectV2_Runtime.cpp")
        ensure = function_tail(
            runtime,
            "\tconst BINDING_SET& Ensure_Bindings(",
            "\tconst DOCUMENT_ENTRY& Ensure_Document(",
        )
        for token in (
            "strArchetypeId == VALTAN_ARCHETYPE_ID",
            "if (Set.bLoaded)",
            "Catalog.Reload_BossValtan(",
            "pSnapshot->Get_Revision()",
            "pSnapshot->Get_BossValtanBindings()",
        ):
            self.assertIn(token, ensure)
        self.assertLess(
            ensure.index("if (Set.bLoaded)"),
            ensure.index("Catalog.Reload_BossValtan("),
        )

    def test_stage_binding_mutations_use_binding_id_identity_and_typed_append(self) -> None:
        header = read(HEADER)
        source = read(SOURCE)
        for token in (
            "struct EFFECT_V2_STAGE_BINDING_KEY final",
            "From_StageBinding(",
            "std::string strBindingId;",
            "std::string strResourceId;",
            "bool_t bGroup = false;",
            "std::string strPatternId;",
            "std::string strStageId;",
            "std::string strActionId;",
            "uint32_t iStartMs = 0u;",
            "Stage_RemoveBossValtanStageBinding(",
            "Stage_DuplicateBossValtanStageBinding(",
            "Stage_UpdateBossValtanStageBindingStart(",
            "Prepare_BossValtanBindingDraftSave(",
            "Accept_BossValtanBindingDraftSave(",
        ):
            self.assertIn(token, header)

        for forbidden in (
            "bool_t Append_BossValtanStageBinding(",
            "bool_t Remove_BossValtanStageBinding(",
            "bool_t Duplicate_BossValtanStageBinding(",
            "bool_t Update_BossValtanStageBindingStart(",
        ):
            self.assertNotIn(forbidden, header + source)

        mutation_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
            "bool_t Client::CEffectV2Catalog::Stage_AppendBossValtanStageBinding(",
        )
        for token in (
            "Resolve_UniqueStageBindingIndex(",
            "CandidateBindings.erase(",
            "EFFECT_V2_BINDING Duplicate =",
            "Duplicate.iStartMs = iTargetStartMs;",
            "Duplicate.strBindingId = Generate_StableBindingId(",
            "CandidateBindings[iSourceIndex].iStartMs = iTargetStartMs;",
            "Commit_BossValtanBindingsLocked(",
        ):
            self.assertIn(token, mutation_body)

        match_body = function_tail(
            source,
            "\tbool_t Matches_StageBindingKey(",
            "\tbool_t Resolve_UniqueStageBindingIndex(",
        )
        self.assertIn("Binding.strBindingId == Key.strBindingId", match_body)
        for forbidden in ("Binding.strStage", "Binding.strClip", "Binding.strBone"):
            self.assertNotIn(forbidden, match_body)

        key_factory = function_tail(
            source,
            "Client::EFFECT_V2_STAGE_BINDING_KEY::From_Binding(",
            "Client::CEffectV2Catalog& Client::CEffectV2Catalog::Get()",
        )
        for field in (
            "Binding.strBindingId",
            "Binding.eResourceKind",
            "Binding.strResourceId",
            "Binding.strPatternId",
            "Binding.strStageId",
            "Binding.strActionId",
            "Binding.iStartMs",
        ):
            self.assertIn(field, key_factory)

        wrapper_region = source[source.index(
            "bool_t Client::CEffectV2Catalog::Stage_AppendBossValtanStageBinding("
        ) : source.index(
            "std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT>"
        )]
        self.assertEqual(wrapper_region.count("Mutate_BossValtanStageBinding("), 4)
        for token in (
            "Stage_AppendBossValtanStageBinding(",
            "Stage_RemoveBossValtanStageBinding(",
            "Stage_DuplicateBossValtanStageBinding(",
            "Stage_UpdateBossValtanStageBindingStart(",
        ):
            self.assertIn(token, wrapper_region)

        append_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Stage_AppendBossValtanStageBinding(",
            "bool_t Client::CEffectV2Catalog::Stage_RemoveBossValtanStageBinding(",
        )
        for token in (
            "EFFECT_V2_STAGE_BINDING_KEY Key{};",
            "Key.strResourceId = strResourceId;",
            "Key.bGroup = bGroup;",
            "Key.strPatternId = strPatternId;",
            "Key.strStageId = strStageId;",
            "Key.strActionId = strActionId;",
            "Key.iStartMs = iStartMs;",
        ):
            self.assertIn(token, append_body)

        signature = "Stage_AppendBossValtanStageBinding("
        declaration = header[
            header.index(signature) : header.index(");", header.index(signature))
        ]
        for parameter in ("strPatternId", "strStageId", "strActionId"):
            self.assertIn(parameter, declaration)

    def test_new_boss_mutations_reject_foreign_subjects_and_overlaps(self) -> None:
        source = read(SOURCE)
        validation = function_tail(
            source,
            "\tbool_t Validate_BossValtanMutationSubject(",
            "\tbool_t Matches_StageBindingKey(",
        )
        self.assertIn('BOSS_VALTAN_RESOURCE_PREFIX = "boss.valtan."', source)
        self.assertIn("starts_with(BOSS_VALTAN_RESOURCE_PREFIX)", validation)

        overlap = function_tail(
            source,
            "\tbool_t Validate_NoLeafGroupClockOverlap(",
            "}\n\nuint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision()",
        )
        for token in (
            "GroupBinding.iStartMs",
            "Child.iStartMs",
            "Child.strResourceId == LeafBinding.strResourceId",
            "iEffectiveStartMs == LeafBinding.iStartMs",
            "leaf overlaps the same leaf inside group",
            "LeafBinding.strPatternId != GroupBinding.strPatternId",
            "LeafBinding.eClockBasis != GroupBinding.eClockBasis",
        ):
            self.assertIn(token, overlap)

        mutation_body = function_tail(
            source,
            "bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(",
            "bool_t Client::CEffectV2Catalog::Stage_AppendBossValtanStageBinding(",
        )
        self.assertIn(
            "BOSS_VALTAN_BINDING_MUTATION::APPEND_BINDING == eMutation",
            mutation_body,
        )
        self.assertIn("Validate_BossValtanMutationSubject(", mutation_body)
        self.assertIn("Validate_NoLeafGroupClockOverlap(", mutation_body)

    def test_cross_validation_joins_documents_groups_and_boss_bindings(self) -> None:
        source = read(SOURCE)
        validation = function_tail(
            source,
            "\tbool_t Cross_Validate(",
            "}\n\nuint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision()",
        )
        for token in (
            "DocumentsById.emplace",
            "groupId collides with an authored effect",
            "Group.Children",
            "Nested Effect V2 groups are not supported",
            "group child has no authored leaf document",
            "Binding.strBindingId",
            "Binding.eResourceKind",
            "Binding.strPatternId",
            "Binding.strStageId",
            "Binding.strActionId",
            "GroupsById.contains(Binding.strResourceId)",
        ):
            self.assertIn(token, validation)

        documents = {
            path.name.removesuffix(".effectv2.json")
            for path in AUTHORED.glob("*.effectv2.json")
        }
        groups = {}
        for path in GROUPS.glob("*.effectv2group.json"):
            value = json.loads(path.read_text(encoding="utf-8"))
            groups[value["groupId"]] = value
            self.assertNotIn(value["groupId"], documents)
            for child in value["children"]:
                self.assertEqual(child["resource"]["kind"], "LEAF")
                self.assertIn(child["resource"]["id"], documents)

        bindings = json.loads(BOSS_BINDINGS.read_text(encoding="utf-8"))
        self.assertEqual(bindings["archetypeId"], "BOSS_VALTAN")
        self.assertGreater(len(bindings["bindings"]), 0)
        for binding in bindings["bindings"]:
            resource = binding["resource"]
            if resource["kind"] == "LEAF":
                self.assertIn(resource["id"], documents)
            else:
                self.assertIn(resource["id"], groups)


if __name__ == "__main__":
    unittest.main()
