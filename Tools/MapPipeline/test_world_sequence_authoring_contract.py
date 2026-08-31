from __future__ import annotations

import re
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


class WorldSequenceAuthoringContractTests(unittest.TestCase):
    """Fast source/project integration guards; the Product build compiles behavior."""

    def setUp(self) -> None:
        self.document_h = read("Client/Public/WorldSequenceDocument.h")
        self.document_cpp = read("Client/Private/WorldSequenceDocument.cpp")
        self.panel_h = read("Client/Public/WorldSequenceToolPanel.h")
        self.panel_cpp = read("Client/Private/WorldSequenceToolPanel.cpp")
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")

    def test_client_project_registers_each_source_once(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        includes = [
            item.attrib.get("Include", "")
            for kind in ("ClInclude", "ClCompile")
            for item in project.findall(f".//msb:{kind}", namespace)
        ]
        expected = (
            r"..\Public\WorldSequenceDocument.h",
            r"..\Public\WorldSequenceToolPanel.h",
            r"..\Private\WorldSequenceDocument.cpp",
            r"..\Private\WorldSequenceToolPanel.cpp",
        )
        for path in expected:
            self.assertEqual(1, includes.count(path), path)
        panel_compile = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\WorldSequenceToolPanel.cpp']",
            namespace,
        )
        self.assertIsNotNone(panel_compile)
        options = " ".join(
            value.text or ""
            for value in panel_compile.findall("msb:AdditionalOptions", namespace)
        )
        self.assertIn("/utf-8", options)

    def test_document_is_strict_versioned_and_transactional(self) -> None:
        self.assertIn('SCHEMA = "lostark.world-sequences"', self.document_cpp)
        self.assertIn("FORMAT_VERSION = 1", self.document_cpp)
        self.assertIn("Is_ExactObject", self.document_cpp)
        self.assertIn("parse -> exact schema validation", read(
            ".md/GB/08-31/2026-08-31_WORLD_SEQUENCE_MAP_TOOL_PLAN.md"
        ))
        self.assertIn("ReplaceFileW", self.document_cpp)
        self.assertIn("MoveFileExW", self.document_cpp)
        self.assertIn("writeSucceeded = writeSucceeded && !output.fail()", self.document_cpp)

    def test_document_reads_are_bounded_and_display_text_is_safe_utf8(self) -> None:
        self.assertIn("MAX_DOCUMENT_BYTES = 16u * 1024u * 1024u", self.document_cpp)
        self.assertIn("std::filesystem::file_size", self.document_cpp)
        self.assertIn("World sequence document exceeds the 16 MiB parse limit", self.document_cpp)
        self.assertIn("catch (const std::bad_alloc&)", self.document_cpp)
        self.assertIn("input.gcount()", self.document_cpp)
        self.assertIn("input.peek()", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.displayName)", self.document_cpp)
        self.assertIn("Is_ValidUtf8DisplayText(value.category)", self.document_cpp)

    def test_document_rejects_invalid_refs_enums_and_runtime_scales(self) -> None:
        self.assertIn("availablePlacements.find(targetId)", self.document_cpp)
        self.assertIn("availableDeployPlacements.find(targetId)", self.document_cpp)
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT != binding.targetKind",
            self.document_cpp,
        )
        self.assertIn(
            "WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT !=",
            self.document_cpp,
        )
        self.assertIn("animationTargetSupported", self.document_cpp)
        self.assertIn("WORLD_SEQUENCE_INTERPOLATION::LINEAR != value.interpolation", self.document_cpp)
        self.assertIn("MIN_RUNTIME_SCALE_DETERMINANT", self.document_cpp)
        self.assertIn("std::isfinite(composedX)", self.document_cpp)
        self.assertIn("Sequence scale would create a singular map transform", self.document_cpp)
        self.assertIn("boundTargets.insert(uniqueTarget).second", self.document_cpp)
        self.assertIn("sequenceTargetSupported", self.document_cpp)
        self.assertIn("MAP_ASSET_RENDER_MODE::BACKGROUND", self.panel_cpp)

    def test_preview_uses_runtime_baseline_and_never_edits_map_records(self) -> None:
        self.assertIn("Try_GetRuntimeVisible", self.panel_cpp)
        self.assertIn("baseline->runtimeVisible || !baseline->baseline.visible", self.panel_cpp)
        self.assertIn("restoredRecord.visible = target.runtimeVisible", self.panel_cpp)
        self.assertIn("Stop_AndRestore", self.panel_cpp)
        self.assertNotRegex(self.panel_cpp, r"entry\.record\s*=")
        runtime_h = read("Client/Public/MapPlacementRuntime.h")
        batch_h = read("Client/Public/MapStaticBatchObject.h")
        self.assertIn("Try_GetRuntimeVisible", runtime_h)
        self.assertIn("Try_GetInstanceVisible", batch_h)

    def test_paused_live_edit_is_validated_then_resampled(self) -> None:
        update = re.search(
            r"void Client::CWorldSequenceToolPanel::Update\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(update)
        body = update.group(0)
        self.assertIn("m_bPreviewNeedsRefresh", body)
        self.assertIn("Validate(catalog, placements, deployRuntime, validation)", body)
        self.assertIn("Apply_Preview", body)
        self.assertLess(
            body.index("Validate(catalog, placements, deployRuntime, validation)"),
            body.index("Apply_Preview"),
        )
        self.assertIn("if (m_bPreviewActive)\n\t\tm_bPreviewNeedsRefresh = true", self.panel_cpp)

    def test_selection_cannot_orphan_an_active_preview(self) -> None:
        template_list = re.search(
            r"void Client::CWorldSequenceToolPanel::Render_TemplateList\(\).*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(template_list)
        self.assertIn("ImGui::BeginDisabled(m_bPreviewActive)", template_list.group(0))
        self.assertIn("Is_PreviewActive", self.panel_h)
        self.assertIn("sequenceOwnsPreviewTargets", self.map_tool_cpp)
        self.assertIn("연출 미리보기를 Stop / Restore한 뒤", self.map_tool_cpp)

    def test_map_and_sequence_save_share_validation_and_rollback(self) -> None:
        self.assertIn("Save_PlacementsAndWorldSequences", self.map_tool_h)
        self.assertIn("PrepareAuthoringBackup", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("WriteAuthoringTransactionMarker", self.map_tool_cpp)
        self.assertIn("RecoverAuthoringTransaction", self.map_tool_cpp)
        self.assertIn("Linked save verification failed", self.map_tool_cpp)
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("m_pWorldSequenceToolPanel->Validate", body)
        self.assertIn("Save_PlacementsAndWorldSequences", body)
        toolbar = re.search(
            r"void Client::CMapTool::Render_Toolbar\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(toolbar)
        self.assertIn('ImGui::Button("Save")', toolbar.group(0))
        self.assertIn("Save_AllAuthoring()", toolbar.group(0))

    def test_linked_save_uses_area_lock_and_stale_source_cas(self) -> None:
        self.assertIn("SCOPED_AUTHORING_SAVE_LOCK", self.map_tool_cpp)
        self.assertIn(".linked-save.lock", self.map_tool_cpp)
        self.assertIn("FILE_FLAG_DELETE_ON_CLOSE", self.map_tool_cpp)
        self.assertIn("GetLastError()", self.map_tool_cpp)
        self.assertIn("ERROR_SHARING_VIOLATION", self.map_tool_cpp)
        self.assertIn("Could not open Area authoring lock", self.map_tool_cpp)
        self.assertIn("Matches_LinkedSourceBaseline", self.panel_h)
        self.assertIn("m_PlacementBaselineBytes", self.panel_h)
        self.assertIn("m_SequenceBaselineBytes", self.panel_h)
        self.assertIn("Save conflict: linked map/sequence source changed after Reload", self.panel_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("Matches_LinkedSourceBaseline"))
        self.assertLess(body.index("Matches_LinkedSourceBaseline"),
                        body.index("PrepareAuthoringBackup"))

        map_save = re.search(
            r"bool_t Client::CMapTool::Save_Placements\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(map_save)
        map_body = map_save.group(0)
        self.assertIn("return Save_PlacementsAndWorldSequences()", map_body)
        self.assertLess(map_body.index("return Save_PlacementsAndWorldSequences()"),
                        map_body.index("authoringLock.Acquire"))
        self.assertNotIn("Refresh_LinkedSourceBaseline", map_body)

    def test_linked_save_verifies_exact_intended_content(self) -> None:
        self.assertIn("AreExactlySamePlacementRecords", self.map_tool_cpp)
        self.assertIn("Has_SameDocument", self.panel_h)
        self.assertIn("Is_Equivalent", self.document_h)
        self.assertIn("Linked save verification found different Map Placement content", self.map_tool_cpp)
        self.assertIn("Linked save verification found different World Sequence content", self.map_tool_cpp)
        self.assertIn("stableVerifiedPlacements", self.map_tool_cpp)
        self.assertIn("return left == right", self.map_tool_cpp)
        self.assertIn("return left == right", self.document_cpp)
        self.assertIn("outStoredRecords", read("Client/Public/MapPlacementDocument.h"))
        placement_document = read("Client/Private/MapPlacementDocument.cpp")
        self.assertIn("UNIT_QUATERNION_TOLERANCE", placement_document)
        self.assertIn("std::abs(length - 1.f) <=", placement_document)
        self.assertIn("Linked save verification source changed during final read", self.map_tool_cpp)
        self.assertIn("RestoreAuthoringBackup", self.map_tool_cpp)
        self.assertIn("MAX_TRANSACTION_MARKER_BYTES = 512u", self.map_tool_cpp)
        self.assertIn("transaction marker exceeds its bounded read limit", self.map_tool_cpp)
        linked_save = re.search(
            r"bool_t Client::CMapTool::Save_PlacementsAndWorldSequences\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(linked_save)
        body = linked_save.group(0)
        self.assertIn("Adopt_VerifiedLinkedSourceBaseline", body)
        self.assertLess(body.rindex("ClearAuthoringTransactionMarker"),
                        body.index("Adopt_VerifiedLinkedSourceBaseline"))
        self.assertIn("editor baseline stayed unchanged", body)

    def test_reload_stages_placements_and_sequences_before_commit(self) -> None:
        reload_body = re.search(
            r"bool_t Client::CMapTool::Load_Placements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(reload_body)
        body = reload_body.group(0)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedDocument", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)
        self.assertLess(body.index("stagedWorldSequencePanel->Load_Area"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertLess(body.index("Stage_PlacementRuntime"),
                        body.index("Remove_PlacementRuntime(m_Placements"))
        self.assertIn("Is_PreviewActive", body)
        render = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(render)
        self.assertIn("Consume_ReloadAllRequest", render.group(0))
        self.assertIn("Load_Placements()", render.group(0))
        self.assertLess(render.group(0).index("Load_Placements()"),
                        render.group(0).index("return;", render.group(0).index("Load_Placements()")))

    def test_initial_area_load_recovers_an_interrupted_linked_save(self) -> None:
        switch = re.search(
            r"bool_t Client::CMapTool::Switch_EditorArea\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(switch)
        body = switch.group(0)
        self.assertIn("worldSequencePath", body)
        self.assertLess(body.index("authoringLock.Acquire"),
                        body.index("RecoverAuthoringTransactionUnderLock"))
        self.assertLess(body.index("RecoverAuthoringTransactionUnderLock"),
                        body.index("CMapPlacementDocument::Read"))
        self.assertLess(body.index("CMapPlacementDocument::Read"),
                        body.index("stagedWorldSequencePanel->Load_Area"))
        self.assertIn("stableLinkedRecords", body)
        self.assertIn("Matches_LinkedSourceBaseline", body)

    def test_ui_rejects_duplicate_and_background_bindings(self) -> None:
        self.assertIn("A map object can be bound to only one target slot", self.panel_cpp)
        self.assertIn("IsSequenceTargetSupported", self.panel_cpp)
        self.assertIn("[Background - unavailable]", self.panel_cpp)
        self.assertIn("sequenceTargetSupported", self.document_h)

    def test_loaded_selection_keeps_template_and_instance_coherent(self) -> None:
        load = re.search(
            r"bool_t Client::CWorldSequenceToolPanel::Load_Area\(.*?\n\}",
            self.panel_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(load)
        body = load.group(0)
        self.assertIn("m_SelectedInstanceId", body)
        self.assertIn("m_Document.Get_Instances().front().templateId", body)
        self.assertIn("placementBefore", body)
        self.assertIn("placementAfter", body)
        self.assertIn("Linked map/sequence source changed while the Area was loading", body)

    def test_visible_labels_are_english_and_help_is_korean(self) -> None:
        for label in (
            "New Sequence",
            "Sequence List",
            "Placed Instances",
            "Map Objects",
            "Add Target Track",
            "Add Key at Preview Time",
            "Stop / Restore",
        ):
            self.assertIn(label, self.panel_cpp)
        self.assertIn("ShowKoreanHelp", self.panel_cpp)
        self.assertIn("새 재사용 연출 템플릿을 만듭니다", self.panel_cpp)
        self.assertIn("미리보기는 원본 맵 배치를 수정하지 않으며", self.panel_cpp)

    def test_product_runtime_boundary_is_explicit(self) -> None:
        self.assertIn("Authoring preview only; product runtime publish is separate", self.panel_cpp)
        self.assertIn("서버 상호작용·제품 재생·길 개방은 아직 연결되지 않았습니다", self.panel_cpp)


class AnimatedPropAuthoringContractTests(unittest.TestCase):
    """MapTool owns creating the Deploy ANIM placements a sequence binds to."""

    def setUp(self) -> None:
        self.map_tool_h = read("Client/Public/MapTool.h")
        self.map_tool_cpp = read("Client/Private/MapTool.cpp")
        self.catalog_h = read("Client/Public/DeployPropCatalog.h")
        self.catalog_cpp = read("Client/Private/DeployPropCatalog.cpp")

    def test_every_map_tool_declaration_has_a_definition(self) -> None:
        declared = set()
        for match in re.finditer(
            r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?;",
            self.map_tool_h,
            flags=re.DOTALL,
        ):
            declared.add(match.group(1))
        defined = set(re.findall(r"CMapTool::([A-Za-z_]\w*)", self.map_tool_cpp))
        ignored = {"ETOI", "ETOUI", "float2_t", "float3_t", "float4_t"}
        missing = sorted(declared - defined - ignored)
        self.assertEqual([], missing)

    def test_animated_prop_authoring_is_reachable_from_the_sequence_mode(self) -> None:
        panel = re.search(
            r"void Client::CMapTool::Render_WorldSequencePanel\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(panel)
        self.assertIn("Render_AnimatedPropsAuthoring();", panel.group(0))
        for label in (
            "Animated Props (Deploy ANIM)",
            "Place In Viewport",
            "Apply Transform",
            "Remove Placement",
            "Save Animated Props",
        ):
            self.assertIn(label, self.map_tool_cpp)

    def test_armed_viewport_click_places_only_in_sequence_mode(self) -> None:
        interaction = re.search(
            r"void Client::CMapTool::Update_WorldInteraction\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(interaction)
        body = interaction.group(0)
        self.assertIn(
            "if (m_bAnimatedPropPlacementArmed && mousePressed)\n"
            "\t\t\t(void)Try_PlaceSelectedDeploy();",
            body,
        )
        self.assertIn("m_bAnimatedPropPlacementArmed = false;", body)
        consumes = re.search(
            r"bool_t Client::CMapTool::ConsumesWorldLeftMouse\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(consumes)
        self.assertIn("m_bAnimatedPropPlacementArmed", consumes.group(0))

    def test_project_authored_placement_ids_stay_in_the_editor_domain(self) -> None:
        allocate = re.search(
            r"uint64_t Client::CMapTool::Allocate_AnimatedPropPlacementId\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(allocate)
        body = allocate.group(0)
        self.assertEqual(
            2, body.count("CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID")
        )
        self.assertIn("0u : candidate", body)
        # A project row may never carry extractor-only evidence fields.
        self.assertIn("0u == row.deployActorId && 0u == row.propDefinitionId", self.catalog_cpp)
        self.assertIn("PROJECT_AUTHORED", self.catalog_h)

    def test_deploy_placement_save_is_atomic_and_read_back_verified(self) -> None:
        self.assertIn("MoveFileExW", self.catalog_cpp)
        self.assertIn("MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH", self.catalog_cpp)
        save = re.search(
            r"bool_t Client::CMapTool::Save_DeployPlacements\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save)
        body = save.group(0)
        self.assertIn("CDeployPropCatalog verification;", body)
        self.assertIn("DeployProp placement save verification found different content", body)
        self.assertLess(body.index("verification.Load"), body.index("m_bDeployDirty = false"))

    def test_unsaved_deploy_authoring_blocks_and_saves_before_sequences(self) -> None:
        unsaved = re.search(
            r"bool_t Client::CMapTool::Has_UnsavedAuthoring\(\) const.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(unsaved)
        self.assertIn("m_bDeployDirty", unsaved.group(0))
        save_all = re.search(
            r"bool_t Client::CMapTool::Save_AllAuthoring\(\).*?\n\treturn true;\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(save_all)
        body = save_all.group(0)
        self.assertIn("if (m_bDeployDirty && !Save_DeployPlacements())", body)
        self.assertLess(
            body.index("Save_DeployPlacements()"),
            body.index("Save_PlacementsAndWorldSequences()"),
        )

    def test_removing_a_bound_animated_prop_rolls_the_runtime_back(self) -> None:
        remove = re.search(
            r"bool_t Client::CMapTool::Remove_SelectedAnimatedProp\(\).*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(remove)
        body = remove.group(0)
        self.assertIn("CDeployPropCatalog restore = m_DeployRuntime.Get_Catalog();", body)
        self.assertIn("m_pWorldSequenceToolPanel->Validate(", body)
        self.assertIn("Commit_DeployCatalog(std::move(restore)", body)
        self.assertLess(body.index("Validate("), body.index("std::move(restore)"))
        self.assertIn("Source-extracted Deploy placements cannot be removed", body)

    def test_commit_releases_preview_seams_before_rebuilding_the_runtime(self) -> None:
        commit = re.search(
            r"bool_t Client::CMapTool::Commit_DeployCatalog\(.*?\n\}",
            self.map_tool_cpp,
            flags=re.DOTALL,
        )
        self.assertIsNotNone(commit)
        body = commit.group(0)
        self.assertIn("Stop_AndRestore(", body)
        self.assertIn("Restore_DestructionPreview();", body)
        self.assertIn("m_pDestructionSimulationController->Clear();", body)
        self.assertLess(body.index("Stop_AndRestore("), body.index("stagedRuntime.Load("))
        self.assertLess(
            body.index("stagedRuntime.Load("),
            body.index("m_DeployRuntime = std::move(stagedRuntime);"),
        )

    def test_korean_help_is_escaped_utf8_because_map_tool_has_no_utf8_flag(self) -> None:
        project = ET.parse(ROOT / "Client/Default/Client.vcxproj")
        namespace = {"msb": "http://schemas.microsoft.com/developer/msbuild/2003"}
        map_tool = project.find(
            ".//msb:ClCompile[@Include='..\\Private\\MapTool.cpp']", namespace
        )
        self.assertIsNotNone(map_tool)
        self.assertEqual(0, len(list(map_tool)), "MapTool.cpp must stay on the default charset")

        blocks = re.findall(
            r"static const char_t\* const (ANIMATED_PROP_HELP_\w+)\s*=\s*"
            r"((?:\s*\"(?:[^\"\\]|\\.)*\")+)\s*;",
            self.map_tool_cpp,
        )
        self.assertEqual(5, len(blocks))
        for name, body in blocks:
            data = bytearray()
            for part in re.findall(r"\"((?:[^\"\\]|\\.)*)\"", body):
                index = 0
                while index < len(part):
                    if part[index] == "\\" and part[index + 1] == "x":
                        data.append(int(part[index + 2:index + 4], 16))
                        index += 4
                    elif part[index] == "\\":
                        data.append(ord(part[index + 1]))
                        index += 2
                    else:
                        data.append(ord(part[index]))
                        index += 1
                # An \xHH escape followed by another hex digit would swallow it.
                self.assertIsNone(
                    re.search(r"\\x[0-9A-Fa-f]{2}[0-9A-Fa-f]", part), name
                )
            text = data.decode("utf-8")
            self.assertTrue(any("\uac00" <= ch <= "\ud7a3" for ch in text), name)



if __name__ == "__main__":
    unittest.main(verbosity=2)
