import copy
import ctypes
from ctypes import wintypes
import json
import os
import pathlib
import re
import subprocess
import sys
import tempfile
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]


class _Overlapped(ctypes.Structure):
    _fields_ = [
        ("Internal", ctypes.c_size_t),
        ("InternalHigh", ctypes.c_size_t),
        ("Offset", wintypes.DWORD),
        ("OffsetHigh", wintypes.DWORD),
        ("hEvent", wintypes.HANDLE),
    ]


def stage_sound_dependencies_accept(
    cues: list[dict],
    pattern_id: str,
    baseline_stage: dict,
    candidate_stage: dict,
    *,
    model_durations_available: bool,
) -> bool:
    rows = [
        cue
        for cue in cues
        if cue["patternId"] == pattern_id
        and cue["stageId"] == baseline_stage["stageId"]
    ]
    # This mirrors the C++ admission boundary: an admitted empty dependency
    # set does not invent a model-duration requirement.
    if not rows:
        return True
    if not model_durations_available:
        return False
    for cue in rows:
        if cue["actionId"] != baseline_stage["actionId"]:
            return False
        if cue["actionId"] != candidate_stage["actionId"]:
            return False
        baseline_matches = [
            row
            for row in baseline_stage["animation"]["occurrences"]
            if row["clipOccurrenceId"] == cue["clipOccurrenceId"]
        ]
        candidate_matches = [
            row
            for row in candidate_stage["animation"]["occurrences"]
            if row["clipOccurrenceId"] == cue["clipOccurrenceId"]
        ]
        if len(baseline_matches) != 1 or len(candidate_matches) != 1:
            return False
        baseline = baseline_matches[0]
        candidate = candidate_matches[0]
        if baseline["clip"] != candidate["clip"]:
            return False
    return True


class ActionCompositionSoundOwnerContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.animation_h = (
            REPO_ROOT / "Client/Public/Animation_Tool.h"
        ).read_text(encoding="utf-8")
        cls.animation_cpp = (
            REPO_ROOT / "Client/Private/Animation_Tool.cpp"
        ).read_text(encoding="utf-8")
        cls.workbench_h = (
            REPO_ROOT / "Client/Public/ActionCompositionWorkbench.h"
        ).read_text(encoding="utf-8")
        cls.workbench_cpp = (
            REPO_ROOT / "Client/Private/ActionCompositionWorkbench.cpp"
        ).read_text(encoding="utf-8")
        cls.boss_cpp = (
            REPO_ROOT / "Client/Private/BossTool.cpp"
        ).read_text(encoding="utf-8")
        cls.client_replication_h = (
            REPO_ROOT / "Client/Public/ClientReplication.h"
        ).read_text(encoding="utf-8")
        cls.client_replication_cpp = (
            REPO_ROOT / "Client/Private/ClientReplication.cpp"
        ).read_text(encoding="utf-8")
        cls.pattern_sound_cpp = (
            REPO_ROOT / "Client/Private/ValtanPatternSoundCueDocument.cpp"
        ).read_text(encoding="utf-8")
        cls.pattern_sound_h = (
            REPO_ROOT / "Client/Public/ValtanPatternSoundCueDocument.h"
        ).read_text(encoding="utf-8")
        cls.valtan_h = (REPO_ROOT / "Client/Public/Valtan.h").read_text(
            encoding="utf-8"
        )
        cls.valtan_cpp = (REPO_ROOT / "Client/Private/Valtan.cpp").read_text(
            encoding="utf-8"
        )
        cls.presentation = json.loads(
            (REPO_ROOT / "Data/Valtan/Valtan.presentation.json").read_text(
                encoding="utf-8"
            )
        )
        cls.sound_source = json.loads(
            (
                REPO_ROOT
                / "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json"
            ).read_text(encoding="utf-8")
        )
        cls.encounter = json.loads(
            (REPO_ROOT / "Data/Encounters/Valtan/ValtanEncounter.json").read_text(
                encoding="utf-8"
            )
        )
        cls.pattern_bindings = json.loads(
            (
                REPO_ROOT
                / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
            ).read_text(encoding="utf-8")
        )

    def test_workbench_borrows_one_animation_tool_sound_draft(self) -> None:
        self.assertNotIn("m_PatternSounds", self.workbench_h)
        self.assertNotIn("m_bPatternSoundsReady", self.workbench_h)
        for token in (
            "Get_ValtanCompositionPatternSoundDraft(",
            "Reload_ValtanCompositionPatternSounds(",
            "Retry_ValtanCompositionPatternSoundRuntimeApply(",
            "Is_ValtanCompositionPatternSoundRuntimeReady(",
            "Patch_ValtanCompositionPatternSound(",
            "Add_ValtanCompositionPatternSound(",
            "Remove_ValtanCompositionPatternSound(",
            "Save_ValtanCompositionPatternSounds(",
            "Retry_ValtanCompositionPatternSoundRuntimeApply(",
            "Is_ValtanCompositionPatternSoundRuntimeReady(",
        ):
            self.assertIn(token, self.workbench_cpp)

    def test_public_adapter_preserves_typed_source_boundary(self) -> None:
        for token in (
            "const VALTAN_PATTERN_SOUND_CUE_DOCUMENT*",
            "Resolve_ValtanCompositionPatternSoundWindow(",
            "Patch_ValtanCompositionPatternSound(",
            "Add_ValtanCompositionPatternSound(",
            "Remove_ValtanCompositionPatternSound(",
            "Save_ValtanCompositionPatternSounds(",
        ):
            self.assertIn(token, self.animation_h)
        self.assertIn(
            "CValtanPatternSoundCueDocument::Save_Atomic(",
            self.animation_cpp,
        )
        self.assertIn(
            "Pattern Sound SOURCE SAVED (separate from Pattern Save & Apply)",
            self.animation_cpp,
        )
        self.assertNotIn(
            "CValtanPatternSoundCueDocument::Save_Atomic(",
            self.workbench_cpp,
        )

    def test_patch_is_staged_and_occurrence_qualified(self) -> None:
        patch_start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Patch_ValtanCompositionPatternSound("
        )
        patch_end = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds(",
            patch_start,
        )
        patch_body = self.animation_cpp[patch_start:patch_end]
        for token in (
            "Cue.strOccurrenceId == strOccurrenceId",
            "Cue.strPatternId == Pattern.strPatternId",
            "Cue.strStageId == Stage.strStageId",
            "Cue.strActionId == Stage.strActionId",
            "VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged",
            "m_ValtanPatternSoundCues = std::move(Staged)",
            "m_bValtanPatternSoundCuesDirty = true",
        ):
            self.assertIn(token, patch_body)

    def test_detail_exposes_event_start_repeat_and_separate_save(self) -> None:
        for token in (
            "Pattern Sound Typed Source",
            "Sound Event",
            "Source startMs",
            "Repeat Policy",
            "Save Sound Owner",
            "Save & Apply",
            "Add Sound Row",
            "Remove Selected Sound Row",
            "Save & Apply does not silently save Sound",
            "Effect timing and Sound timing remain unsaved drafts",
        ):
            self.assertIn(token, self.workbench_cpp)

        sound_owner = re.search(
            r'OwnerButton\("Sound",\s*DETAIL_OWNER::SOUND,([\s\S]*?)\);',
            self.workbench_cpp,
        )
        self.assertIsNotNone(sound_owner)
        assert sound_owner is not None
        self.assertIn("!pStage->ClipOccurrences.empty()", sound_owner.group(1))
        self.assertNotIn("!strFirstSoundOccurrence.empty()", sound_owner.group(1))

    def test_add_remove_use_typed_row_identity_and_reselect_new_row(self) -> None:
        add_start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Add_ValtanCompositionPatternSound("
        )
        remove_start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Remove_ValtanCompositionPatternSound("
        )
        save_start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds("
        )
        add_body = self.animation_cpp[add_start:remove_start]
        remove_body = self.animation_cpp[remove_start:save_start]
        for token in (
            "CValtanPatternSoundCueDocument::Add_AuthoringRow(",
            "VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged",
            "OutCreatedRowId = CreatedRowId",
            "m_bValtanPatternSoundCuesDirty = true",
        ):
            self.assertIn(token, add_body)
        for token in (
            "Cue.strBindingId == RowId.strBindingId",
            "Cue.strOccurrenceId == RowId.strOccurrenceId",
            "Cue.strPatternId == Pattern.strPatternId",
            "Cue.strStageId == Stage.strStageId",
            "CValtanPatternSoundCueDocument::Remove_AuthoringRow(",
        ):
            self.assertIn(token, remove_body)
        self.assertIn(
            "m_strSelectedStableId = CreatedRowId.strOccurrenceId",
            self.workbench_cpp,
        )
        add_call = self.workbench_cpp.index(
            "Add_ValtanCompositionPatternSound("
        )
        timeline_invalidation = self.workbench_cpp.index(
            "Invalidate_TimelineCache();", add_call
        )
        self.assertLess(add_call, timeline_invalidation)

    def test_cross_owner_dirty_state_blocks_dangling_dependency_writes(self) -> None:
        sequence_start = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage("
        )
        sequence_end = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Seek_EffectivePreview(",
            sequence_start,
        )
        sequence_body = self.workbench_cpp[sequence_start:sequence_end]
        self.assertIn("Is_PatternSoundDraftDirty(SoundStatus)", sequence_body)
        self.assertLess(
            sequence_body.index("Is_PatternSoundDraftDirty(SoundStatus)"),
            sequence_body.index(
                "SetValtanStageDraftWithSoundDependencyAdmission("
            ),
        )

        save_start = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Save_Publish_Reload()"
        )
        save_end = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(",
            save_start,
        )
        save_body = self.workbench_cpp[save_start:save_end]
        self.assertIn("Is_PatternSoundDraftDirty(SoundStatus)", save_body)
        self.assertLess(
            save_body.index("Is_PatternSoundDraftDirty(SoundStatus)"),
            save_body.index("Save_ValtanCanonicalProduct(SaveStatus)"),
        )
        for token in (
            "bPatternMutationAdmitted",
            "SOUND OWNER DRAFT / PATTERN MUTATION LOCKED",
            "Create Pattern is blocked until the Pattern Sound source draft",
            "bMutationAdmitted && !m_bAuthoringDraftDirty",
            "Tune / Remove Existing Server Collider / Hit Schedule",
            "Add Manual Audition Server Collider / Hit Schedule",
            "View Collider Authority (New Add Unavailable)",
        ):
            self.assertIn(token, self.workbench_cpp)
        self.assertNotIn(
            'ImGui::BeginDisabled(!bHasServerCollider)',
            self.workbench_cpp,
        )

    def test_reload_discard_and_runtime_apply_are_fail_closed(self) -> None:
        reload_start = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()"
        )
        reload_end = self.workbench_cpp.index(
            "void Client::CActionCompositionWorkbench::Select_Pattern(",
            reload_start,
        )
        reload_body = self.workbench_cpp[reload_start:reload_end]
        self.assertIn("Is_PatternSoundDraftDirty(SoundDraftStatus)", reload_body)
        self.assertLess(
            reload_body.index("Is_PatternSoundDraftDirty(SoundDraftStatus)"),
            reload_body.index("m_pBalanceTool->Reload_ValtanSource("),
        )
        self.assertIn(
            "Reload_ValtanCompositionPatternSounds(", reload_body
        )
        self.assertIn(
            "Retry_ValtanCompositionPatternSoundRuntimeApply(", reload_body
        )
        self.assertIn(
            "Get_ServerActivePatternRevision(", reload_body
        )
        self.assertIn("ExpectedSoundRuntimeRevision", reload_body)
        self.assertLess(
            reload_body.index("Reload_ValtanCompositionPatternSounds("),
            reload_body.index("Get_ServerActivePatternRevision("),
        )
        self.assertLess(
            reload_body.index("Get_ServerActivePatternRevision("),
            reload_body.index("Retry_ValtanCompositionPatternSoundRuntimeApply("),
        )
        self.assertGreaterEqual(
            reload_body.count("Get_ServerActivePatternRevision("), 2
        )
        self.assertLess(
            reload_body.index("Retry_ValtanCompositionPatternSoundRuntimeApply("),
            reload_body.rindex("Get_ServerActivePatternRevision("),
        )
        self.assertIn(
            "Invalidate_ValtanCompositionPatternSoundRuntimeApply(", reload_body
        )
        self.assertNotIn(
            "Ensure_ValtanCompositionPatternSounds(", reload_body
        )

        for token in (
            "Review Discard Sound Draft",
            "Confirm Discard + Reload Sound Owner",
            "Cancel Discard",
            "Retry Apply Saved Sound",
            "m_bConfirmDiscardPatternSoundDraft",
        ):
            self.assertIn(token, self.workbench_cpp + self.workbench_h)
        self.assertLess(
            self.workbench_cpp.index("Review Discard Sound Draft"),
            self.workbench_cpp.index("Confirm Discard + Reload Sound Owner"),
        )

        save_start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds("
        )
        save_end = self.animation_cpp.index(
            "\nvoid Client::CAnimation_Tool::Render_ValtanCompositionPatternCreator(",
            save_start,
        )
        save_body = self.animation_cpp[save_start:save_end]
        for token in (
            "m_bValtanPatternSoundRuntimeApplyReady",
            "ACTIVE CONSUMER APPLY DEFERRED",
            "return bDraftReloaded;",
        ):
            self.assertIn(token, save_body)
        self.assertNotIn(
            "Apply_ValtanCompositionPatternSoundsToActiveConsumers(", save_body
        )
        self.assertIn("ApplySoundToExactRuntime", self.workbench_cpp)
        self.assertIn(
            "Get_ServerActivePatternRevision(", self.workbench_cpp
        )
        toolbar_start = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar("
        )
        toolbar_end = self.workbench_cpp.index(
            "void Client::CActionCompositionWorkbench::Render_Browser(",
            toolbar_start,
        )
        toolbar = self.workbench_cpp[toolbar_start:toolbar_end]
        self.assertIn("bSoundRuntimeReady", toolbar)
        self.assertIn("ExpectedServerRevision", toolbar)
        self.assertLess(
            toolbar.index("Observe_ServerActivePatternRevision("),
            toolbar.index("Is_ValtanCompositionPatternSoundRuntimeReady("),
        )
        self.assertNotIn("Get_ServerActivePatternRevision(", toolbar)
        self.assertIn("Server playback blocked by Pattern Sound", toolbar)

        for token in (
            "const LostArk::Shared::GameplayDataRevision& ExpectedRevision",
            "m_ValtanPatternSoundRuntimeAppliedRevision",
            "Consumer reload receipt is provisional until Boss Tool revalidates",
            "Invalidate_ValtanCompositionPatternSoundRuntimeApply(",
            "Format_GameplayDataRevision(ExpectedRevision)",
        ):
            self.assertIn(token, self.animation_cpp + self.animation_h)
        apply_consumer_start = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Apply_ValtanCompositionPatternSoundsToActiveConsumers("
        )
        apply_consumer_end = self.animation_cpp.index(
            "\nbool_t Client::CAnimation_Tool::Retry_ValtanCompositionPatternSoundRuntimeApply(",
            apply_consumer_start,
        )
        apply_consumer_body = self.animation_cpp[
            apply_consumer_start:apply_consumer_end
        ]
        self.assertIn("bool_t bArenaReloaded = false;", apply_consumer_body)
        self.assertIn("CLevel_ValtanArena::Get_Active()", apply_consumer_body)
        self.assertIn(
            "Get_ServerActivePatternRevision(", self.boss_cpp
        )
        apply_start = self.workbench_cpp.index(
            "const auto ApplySoundToExactRuntime ="
        )
        apply_end = self.workbench_cpp.index(
            "if (m_bAuthoringDraftDirty)", apply_start
        )
        apply_body = self.workbench_cpp[apply_start:apply_end]
        self.assertGreaterEqual(
            apply_body.count("Get_ServerActivePatternRevision("), 2
        )
        self.assertLess(
            apply_body.index("Retry_ValtanCompositionPatternSoundRuntimeApply("),
            apply_body.rindex("Get_ServerActivePatternRevision("),
        )
        self.assertIn(
            "Invalidate_ValtanCompositionPatternSoundRuntimeApply(", apply_body
        )

    def test_runtime_consumer_receipt_is_enforced_at_boss_command_boundary(self) -> None:
        gate_start = self.client_replication_h.index(
            "class CPrimaryValtanPresentationFreshnessGate final"
        )
        gate_end = self.client_replication_h.index("\n\t//", gate_start)
        gate = self.client_replication_h[gate_start:gate_end]
        for token in (
            "GameplayDataRevision& Revision",
            "GameplayDataRevision& ExpectedRevision",
            "m_Revision == ExpectedRevision",
            "m_Revision = {};",
        ):
            self.assertIn(token, gate)

        can_play_start = self.boss_cpp.index(
            "bool_t Client::CBossTool::Can_Play_ServerPattern("
        )
        can_play_end = self.boss_cpp.index(
            "bool_t Client::CBossTool::Get_ServerActivePatternRevision(",
            can_play_start,
        )
        can_play = self.boss_cpp[can_play_start:can_play_end]
        self.assertIn("Acquire_ServerPlaybackAdmission(", can_play)

        revision_start = self.boss_cpp.index(
            "bool_t Client::CBossTool::Acquire_ServerPlaybackAdmission("
        )
        revision_end = self.boss_cpp.index(
            "bool_t Client::CBossTool::Get_ServerPatternOptions(", revision_start
        )
        revision_gate = self.boss_cpp[revision_start:revision_end]
        for token in (
            "PreAdmissionRevision",
            "PostAdmissionRevision != PreAdmissionRevision",
            "CLevel_ValtanArena::Get_Active()",
            "Can_Play_PrimaryValtanPresentation(",
            "Get_PrimaryValtanPatternSoundSourceReceipt(",
            "SoundAdmission.Acquire(CurrentSoundReceipt",
            "CurrentSoundReceipt != ConsumerSoundReceipt",
        ):
            self.assertIn(token, revision_gate)

        submit_start = self.boss_cpp.index(
            "bool_t Client::CBossTool::Submit_SelectedPattern()"
        )
        submit_end = self.boss_cpp.index(
            "bool_t Client::CBossTool::Restart_SelectedPattern()", submit_start
        )
        submit = self.boss_cpp[submit_start:submit_end]
        self.assertLess(
            submit.index("Acquire_ServerPlaybackAdmission("),
            submit.index("CValtanPatternAuditionService::Get().Submit("),
        )
        self.assertIn("expectedActiveRevision", submit)
        self.assertIn("CValtanPatternSoundSourceReadAdmission", submit)

        self.assertIn("spawned.PinnedDefinitionRevision", self.client_replication_cpp)
        self.assertIn("JoinedReloadStatus", self.client_replication_cpp)

        for token in (
            "VALTAN_PATTERN_SOUND_SOURCE_RECEIPT",
            "CValtanPatternSoundSourceReadAdmission",
            "bool_t Acquire(",
        ):
            self.assertIn(token, self.pattern_sound_h)
        for token in (
            "FILE_SHARE_READ",
            "BuildSoundSourceReceipt(",
            "SOUND_AUTHORING_SAVE_MUTEX",
            "OutReceipt = std::move(StagedReceipt)",
        ):
            self.assertIn(token, self.pattern_sound_cpp)
        self.assertIn("m_PatternSoundSourceReceipt", self.valtan_h)
        self.assertIn(
            "m_PatternSoundSourceReceipt = std::move(SourceReceipt)",
            self.valtan_cpp,
        )
        self.assertIn(
            "Get_PrimaryValtanPatternSoundSourceReceipt(",
            self.client_replication_cpp,
        )

        for function_name in (
            "bool_t Client::CAnimation_Tool::Patch_ValtanCompositionPatternSound(",
            "bool_t Client::CAnimation_Tool::Add_ValtanCompositionPatternSound(",
            "bool_t Client::CAnimation_Tool::Remove_ValtanCompositionPatternSound(",
            "bool_t Client::CAnimation_Tool::Save_ValtanCompositionPatternSounds(",
        ):
            start = self.animation_cpp.index(function_name)
            end = self.animation_cpp.find(
                "\nbool_t Client::CAnimation_Tool::", start + len(function_name)
            )
            if end < 0:
                end = len(self.animation_cpp)
            body = self.animation_cpp[start:end]
            self.assertIn("Get_ValtanAuthoringState(", body)
            self.assertIn("bCanonicalDraftDirty", body)
            self.assertIn(
                "Is_ValtanCompositionPatternTransactionActive()", body
            )

    def test_sound_save_holds_canonical_dependency_generation_admission(self) -> None:
        save_start = self.pattern_sound_cpp.index(
            "bool_t Client::CValtanPatternSoundCueDocument::Save_Atomic("
        )
        save_body = self.pattern_sound_cpp[save_start:]
        for token in (
            '#include "ValtanPatternTree.h"',
            "CValtanCanonicalProductReadAdmission CanonicalAdmission;",
            "CanonicalAdmission.Acquire(CanonicalAdmissionStatus)",
            "CanonicalAdmission.Validate_StillCurrent(CanonicalAdmissionStatus)",
            "SOUND_JOINED_OWNER_COMMIT_GUARD CommitGuard;",
            "Commit_Temporary(Destination, Temporary)",
        ):
            self.assertIn(token, self.pattern_sound_cpp)
        self.assertLess(
            save_body.index(
                "CanonicalAdmission.Acquire(CanonicalAdmissionStatus)"
            ),
            save_body.index("Read_File(Destination, PreviousBytes)"),
        )
        self.assertLess(
            save_body.index(
                "CanonicalAdmission.Validate_StillCurrent(CanonicalAdmissionStatus)"
            ),
            save_body.index("Commit_Temporary(Destination, Temporary)"),
        )

    @unittest.skipUnless(
        os.name == "nt", "canonical publisher/Sound lock oracle is Windows-only"
    )
    def test_canonical_publisher_writer_excludes_sound_save_admission(self) -> None:
        kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
        create_file = kernel32.CreateFileW
        create_file.argtypes = [
            wintypes.LPCWSTR,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.LPVOID,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.HANDLE,
        ]
        create_file.restype = wintypes.HANDLE
        lock_file = kernel32.LockFileEx
        lock_file.argtypes = [
            wintypes.HANDLE,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.DWORD,
            wintypes.DWORD,
            ctypes.POINTER(_Overlapped),
        ]
        lock_file.restype = wintypes.BOOL
        close_handle = kernel32.CloseHandle
        close_handle.argtypes = [wintypes.HANDLE]
        close_handle.restype = wintypes.BOOL

        with tempfile.TemporaryDirectory() as temporary:
            pipeline_directory = REPO_ROOT / "Tools/ValtanPipeline"
            child_script = "\n".join(
                (
                    "from pathlib import Path",
                    "import sys",
                    f"sys.path.insert(0, {str(pipeline_directory)!r})",
                    "import valtan_tuning_pipeline as pipeline",
                    "root = Path(sys.argv[1])",
                    "with pipeline._exclusive_canonical_writer_admission(root):",
                    '    print("PUBLISHER_LOCKED", flush=True)',
                    "    sys.stdin.read(1)",
                )
            )
            child = subprocess.Popen(
                [sys.executable, "-c", child_script, temporary],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            try:
                self.assertIsNotNone(child.stdout)
                self.assertEqual(
                    "PUBLISHER_LOCKED", child.stdout.readline().strip()
                )
                lock_path = (
                    pathlib.Path(temporary)
                    / "out/ValtanPatternTransactions/create-pattern.lock"
                )
                generic_read = 0x80000000
                generic_write = 0x40000000
                file_share_read = 0x00000001
                file_share_write = 0x00000002
                file_share_delete = 0x00000004
                open_existing = 3
                file_attribute_normal = 0x00000080
                handle = create_file(
                    str(lock_path),
                    generic_read | generic_write,
                    file_share_read | file_share_write | file_share_delete,
                    None,
                    open_existing,
                    file_attribute_normal,
                    None,
                )
                invalid_handle = ctypes.c_void_p(-1).value
                self.assertNotEqual(invalid_handle, handle)
                try:
                    overlap = _Overlapped()
                    lockfile_fail_immediately = 0x00000001
                    ctypes.set_last_error(0)
                    acquired = lock_file(
                        handle,
                        lockfile_fail_immediately,
                        0,
                        1,
                        0,
                        ctypes.byref(overlap),
                    )
                    self.assertFalse(acquired)
                    self.assertEqual(33, ctypes.get_last_error())
                finally:
                    close_handle(handle)
            finally:
                if child.poll() is None and child.stdin is not None:
                    child.stdin.write("x")
                    child.stdin.flush()
                stdout, stderr = child.communicate(timeout=5)
                self.assertEqual(
                    0,
                    child.returncode,
                    msg=(
                        "canonical publisher lock child failed; "
                        f"stdout={stdout!r} stderr={stderr!r}"
                    ),
                )

    def test_every_saved_sound_row_resolves_one_physical_pattern_stage_action_clip(self) -> None:
        patterns = {row["patternId"]: row for row in self.encounter["patterns"]}
        bindings = {
            row["actionId"]: row for row in self.pattern_bindings["bindings"]
        }
        self.assertGreater(len(self.sound_source["cues"]), 500)
        for cue in self.sound_source["cues"]:
            pattern = patterns[cue["patternId"]]
            stages = [
                row
                for row in pattern["stages"]
                if row["stageId"] == cue["stageId"]
            ]
            self.assertEqual(1, len(stages), cue["occurrenceId"])
            stage = stages[0]
            self.assertEqual(stage["actionId"], cue["actionId"])
            binding = bindings[stage["actionId"]]
            clips = [
                row
                for row in binding["clips"]
                if row["clipOccurrenceId"] == cue["clipOccurrenceId"]
            ]
            self.assertEqual(1, len(clips), cue["occurrenceId"])

    def test_occurrence_remove_and_clip_identity_reuse_are_fail_closed(self) -> None:
        cue = self.sound_source["cues"][0]
        pattern = next(
            row
            for row in self.encounter["patterns"]
            if row["patternId"] == cue["patternId"]
        )
        encounter_stage = next(
            row for row in pattern["stages"] if row["stageId"] == cue["stageId"]
        )
        binding = next(
            row
            for row in self.pattern_bindings["bindings"]
            if row["actionId"] == encounter_stage["actionId"]
        )
        baseline = copy.deepcopy(encounter_stage)
        baseline["animation"] = {"occurrences": copy.deepcopy(binding["clips"])}

        removed = copy.deepcopy(baseline)
        removed["animation"]["occurrences"] = [
            row
            for row in removed["animation"]["occurrences"]
            if row["clipOccurrenceId"] != cue["clipOccurrenceId"]
        ]
        self.assertFalse(
            stage_sound_dependencies_accept(
                self.sound_source["cues"],
                cue["patternId"],
                baseline,
                removed,
                model_durations_available=True,
            )
        )

        duplicated = copy.deepcopy(baseline)
        duplicated["animation"]["occurrences"].append(
            copy.deepcopy(
                next(
                    row
                    for row in baseline["animation"]["occurrences"]
                    if row["clipOccurrenceId"] == cue["clipOccurrenceId"]
                )
            )
        )
        self.assertFalse(
            stage_sound_dependencies_accept(
                self.sound_source["cues"],
                cue["patternId"],
                baseline,
                duplicated,
                model_durations_available=True,
            )
        )

        rebound = copy.deepcopy(baseline)
        rebound_clip = next(
            row
            for row in rebound["animation"]["occurrences"]
            if row["clipOccurrenceId"] == cue["clipOccurrenceId"]
        )
        rebound_clip["clip"] += ".different"
        self.assertFalse(
            stage_sound_dependencies_accept(
                self.sound_source["cues"],
                cue["patternId"],
                baseline,
                rebound,
                model_durations_available=True,
            )
        )

    def test_soundless_stage_does_not_require_model_durations(self) -> None:
        sound_stage_keys = {
            (row["patternId"], row["stageId"])
            for row in self.sound_source["cues"]
        }
        bindings = {
            row["actionId"]: row for row in self.pattern_bindings["bindings"]
        }
        pattern, encounter_stage = next(
            (pattern, stage)
            for pattern in self.encounter["patterns"]
            for stage in pattern["stages"]
            if (pattern["patternId"], stage["stageId"]) not in sound_stage_keys
            and stage["actionId"] in bindings
        )
        stage = copy.deepcopy(encounter_stage)
        stage["animation"] = {
            "occurrences": copy.deepcopy(bindings[stage["actionId"]]["clips"])
        }
        self.assertTrue(
            stage_sound_dependencies_accept(
                self.sound_source["cues"],
                pattern["patternId"],
                stage,
                copy.deepcopy(stage),
                model_durations_available=False,
            )
        )

    def test_every_stage_mutation_and_canonical_save_use_dependency_admission(self) -> None:
        for token in (
            "Validate_ValtanCompositionPatternSoundStageDependencies(",
            "Validate_ValtanCompositionPatternSoundGraphDependencies(",
        ):
            self.assertIn(token, self.animation_h)
            self.assertIn(token, self.animation_cpp)

        stage_admission = self.animation_cpp.index(
            "Validate_ValtanCompositionPatternSoundStageDependencies("
        )
        stage_graph = self.animation_cpp.index(
            "Validate_ValtanCompositionPatternSoundGraphDependencies("
        )
        stage_body = self.animation_cpp[stage_admission:stage_graph]
        self.assertLess(stage_body.index("if (Rows.empty())"), stage_body.index(
            "Resolve_ValtanCompositionPatternSoundWindow("
        ))
        for token in (
            "Cue.strActionId != CandidateStage.strActionId",
            "1u != iCandidateClipCount",
            "CandidateStage.ClipOccurrences.end() == CandidateClip",
            "BaselineClip->strClipName != CandidateClip->strClipName",
            "Cue.iStartMs < iMinimumStartMs",
            "Cue.iStartMs > iMaximumStartMs",
            "Cue.eRepeatPolicy && !bLoop",
        ):
            self.assertIn(token, stage_body)

        # The only raw BalanceTool Stage setter is inside the one dependency
        # wrapper; Replace/Append, Details, and timeline trim all call that
        # wrapper instead of bypassing Sound admission. Effect cue edits now
        # use their explicit typed cue APIs and do not mutate Animation slots.
        self.assertEqual(1, self.workbench_cpp.count("Set_ValtanStageDraft("))
        self.assertGreaterEqual(
            self.workbench_cpp.count(
                "SetValtanStageDraftWithSoundDependencyAdmission("
            ),
            5,
        )
        legacy_start = self.animation_cpp.index(
            "void Client::CAnimation_Tool::Render_ValtanStageDraftInspector("
        )
        legacy_end = self.animation_cpp.index(
            "bool_t Client::CAnimation_Tool::Render_ValtanPatternSoundInspector(",
            legacy_start,
        )
        legacy_body = self.animation_cpp[legacy_start:legacy_end]
        self.assertLess(
            legacy_body.index(
                "Validate_ValtanCompositionPatternSoundStageDependencies("
            ),
            legacy_body.index("Set_ValtanStageDraft("),
        )
        save_start = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Save_Publish_Reload()"
        )
        save_end = self.workbench_cpp.index(
            "bool_t Client::CActionCompositionWorkbench::Render_Toolbar(", save_start
        )
        save_body = self.workbench_cpp[save_start:save_end]
        self.assertLess(
            save_body.index(
                "Validate_ValtanCompositionPatternSoundGraphDependencies("
            ),
            save_body.index("Save_ValtanCanonicalProduct(SaveStatus)"),
        )

    def test_sound_window_native_durations_are_cached_per_model(self) -> None:
        self.assertIn(
            "m_ValtanPatternSoundDurationModel", self.animation_h
        )
        self.assertIn(
            "m_ValtanPatternSoundClipDurations", self.animation_h
        )
        start = self.animation_cpp.index(
            "Resolve_ValtanCompositionPatternSoundWindow("
        )
        end = self.animation_cpp.index(
            "Validate_ValtanCompositionPatternSoundStageDependencies(", start
        )
        window = self.animation_cpp[start:end]
        self.assertIn(
            "m_ValtanPatternSoundDurationModel.lock() != pModel", window
        )
        self.assertEqual(
            1, window.count("CollectModelClipSourceDurationSeconds(pModel)")
        )
        self.assertIn("if (nullptr == pModel)", window)
        self.assertIn("m_ValtanPatternSoundClipDurations.clear()", window)


if __name__ == "__main__":
    unittest.main()
