import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


class ValtanPatternSoundOccurrencePinContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.audition_h = read("Client/Public/ValtanPatternAuditionService.h")
        cls.audition_cpp = read("Client/Private/ValtanPatternAuditionService.cpp")
        cls.flow_h = read("Client/Public/ValtanPatternFlowService.h")
        cls.flow_cpp = read("Client/Private/ValtanPatternFlowService.cpp")
        cls.boss_cpp = read("Client/Private/BossTool.cpp")
        cls.animation_cpp = read("Client/Private/Animation_Tool.cpp")
        cls.workbench_cpp = read("Client/Private/ActionCompositionWorkbench.cpp")
        cls.replication_cpp = read("Client/Private/ClientReplication.cpp")
        cls.valtan_cpp = read("Client/Private/Valtan.cpp")
        cls.audition_test = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanPatternAuditionServiceHarness.cpp"
        )
        cls.flow_test = read(
            "Tools/ValtanPatternAuditionServiceHarness/Private/"
            "ValtanPatternFlowServiceTests.cpp"
        )

    def test_s_is_value_pinned_in_every_audition_control_owner(self) -> None:
        self.assertGreaterEqual(
            self.audition_h.count("PinnedPatternSoundSourceReceipt"), 6
        )
        for token in (
            "m_Snapshot.PinnedPatternSoundSourceReceipt =",
            "m_RestartFallback.PinnedPatternSoundSourceReceipt",
            "m_NextCommand.PinnedPatternSoundSourceReceipt =",
            "m_NextSnapshot.PinnedPatternSoundSourceReceipt =",
            "m_Snapshot.PinnedPatternSoundSourceReceipt =\n\t\tm_NextSnapshot",
            "Verify_PatternSoundSourceReceipt(",
        ):
            self.assertIn(token, self.audition_cpp)

    def test_flow_pins_s_across_pending_accept_and_slot_lifecycle(self) -> None:
        self.assertGreaterEqual(
            self.flow_h.count("PinnedPatternSoundSourceReceipt"), 3
        )
        self.assertIn(
            "Staged.PinnedPatternSoundSourceReceipt =\n\t\tPinnedPatternSoundSourceReceipt",
            self.flow_cpp,
        )
        self.assertIn(
            "Staged.PinnedPatternSoundSourceReceipt =\n\t\tm_PendingStart.PinnedPatternSoundSourceReceipt",
            self.flow_cpp,
        )
        self.assertIn("Verify_PatternSoundSourceReceipt(", self.flow_cpp)

    def test_boss_tool_passes_locked_s_to_all_submission_paths(self) -> None:
        self.assertIn(
            "OutSoundReceipt = CurrentSoundReceipt", self.boss_cpp
        )
        self.assertGreaterEqual(self.boss_cpp.count("PinnedSoundReceipt"), 20)
        for call in (
            "expectedActiveRevision,\n\t\t\tPinnedSoundReceipt",
            "ExpectedRevision, PinnedSoundReceipt, QueueStatus",
            "ExpectedRevision,\n\t\t\tPinnedSoundReceipt,\n\t\t\tStatus",
            "Retry_NextPatternCommand(\n\t\t\t\t\tPinnedSoundReceipt",
            "Retry_UnconfirmedRestart(\n\t\t\t\t\tPinnedSoundReceipt",
        ):
            self.assertIn(call, self.boss_cpp)

    def test_sound_commit_and_consumer_reload_have_backend_and_ui_barriers(self) -> None:
        for owner in (
            "Save_ValtanCompositionPatternSounds",
            "Retry_ValtanCompositionPatternSoundRuntimeApply",
            "Apply_ValtanCompositionPatternSoundsToActiveConsumers",
            "Reload_ValtanPatternSoundCues",
        ):
            start = self.animation_cpp.index(owner)
            body = self.animation_cpp[start : start + 1800]
            self.assertIn(
                "Can_CommitValtanCompositionPatternSoundGeneration", body
            )
        self.assertIn(
            "Can_ReloadValtanPresentationWithoutResettingLiveSound(strOutStatus)",
            self.replication_cpp,
        )
        self.assertGreaterEqual(
            self.replication_cpp.count(
                "Can_ReloadValtanPresentationWithoutResettingLiveSound(strOutStatus)"
            ),
            2,
        )
        self.assertIn("bSoundSourceCommitAdmitted", self.workbench_cpp)
        self.assertIn("LIVE OCCURRENCE PIN", self.workbench_cpp)

    def test_idle_s_apply_preserves_other_lane_attempt_state(self) -> None:
        self.assertIn(
            "bJoinedPresentationGenerationUnchanged", self.valtan_cpp
        )
        self.assertIn("bPatternSoundGenerationUnchanged", self.valtan_cpp)
        self.assertIn(
            "if (!bJoinedPresentationGenerationUnchanged)", self.valtan_cpp
        )
        self.assertIn(
            "if (!bPatternSoundGenerationUnchanged)", self.valtan_cpp
        )
        joined_reset = self.valtan_cpp.index(
            "if (!bJoinedPresentationGenerationUnchanged)"
        )
        sound_reset = self.valtan_cpp.index(
            "if (!bPatternSoundGenerationUnchanged)"
        )
        self.assertLess(joined_reset, sound_reset)

    def test_native_harness_covers_mismatch_retry_and_auto_next(self) -> None:
        for token in (
            "VerifyPatternSoundReceiptPinsOccurrenceRestartAndAutoNext",
            "PinnedPatternSoundSourceReceipt == SoundA",
            "Retry_UnconfirmedRestart(SoundB",
            "auto-promoted Next occurrence lost its pinned S receipt",
        ):
            self.assertIn(token, self.audition_test)
        for token in (
            "VerifyPatternSoundReceiptPinsFlowAndEverySlotOccurrence",
            "Retry_Start(SoundB",
            "Flow auto-next slot transition lost its pinned S receipt",
        ):
            self.assertIn(token, self.flow_test)


if __name__ == "__main__":
    unittest.main()
