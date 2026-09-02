from __future__ import annotations

import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PATTERN_TREE_SOURCE = (
    REPOSITORY_ROOT / "Client" / "Private" / "ValtanPatternTree.cpp"
)
PATTERN_TREE_HEADER = (
    REPOSITORY_ROOT / "Client" / "Public" / "ValtanPatternTree.h"
)
VALTAN_SOURCE = REPOSITORY_ROOT / "Client" / "Private" / "Valtan.cpp"
LOADING_SOURCE = REPOSITORY_ROOT / "Client" / "Private" / "Level_Loading.cpp"
CHARACTER_SELECT_SOURCE = (
    REPOSITORY_ROOT / "Client" / "Private" / "Level_CharacterSelect.cpp"
)
BALANCE_SOURCE = REPOSITORY_ROOT / "Client" / "Private" / "BalanceTool.cpp"
BALANCE_HEADER = REPOSITORY_ROOT / "Client" / "Public" / "BalanceTool.h"
WORKBENCH_SOURCE = (
    REPOSITORY_ROOT / "Client" / "Private" / "ActionCompositionWorkbench.cpp"
)


def function_body(source: str, signature: str) -> str:
    begin = source.index(signature)
    brace = source.index("{", begin)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[begin : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


class _Overlapped(ctypes.Structure):
    _fields_ = [
        ("Internal", ctypes.c_size_t),
        ("InternalHigh", ctypes.c_size_t),
        ("Offset", wintypes.DWORD),
        ("OffsetHigh", wintypes.DWORD),
        ("hEvent", wintypes.HANDLE),
    ]


@unittest.skipUnless(os.name == "nt", "Valtan Product writer lock is Windows-only")
class ValtanPatternTreeTransactionReadGateTests(unittest.TestCase):
    def test_reusable_admission_stages_graph_without_nested_lock(self) -> None:
        source = PATTERN_TREE_SOURCE.read_text(encoding="utf-8")
        header = PATTERN_TREE_HEADER.read_text(encoding="utf-8")
        load_begin = source.index("bool_t Client::CValtanPatternTree::Load(")
        admitted_begin = source.index(
            "bool_t Client::CValtanPatternTree::Load_WhileAdmitted(",
            load_begin,
        )
        authoring_begin = source.index(
            "bool_t Client::CValtanPatternTree::Load_FromAuthoringPaths(",
            admitted_begin,
        )
        load_body = source[load_begin:admitted_begin]
        admitted_body = source[admitted_begin:authoring_begin]

        self.assertIn("class CValtanCanonicalProductReadAdmission final", header)
        self.assertIn(
            'L"out\\\\ValtanPatternTransactions\\\\create-pattern.lock"',
            source,
        )
        self.assertIn("LockFileEx(m_hFile, LOCKFILE_FAIL_IMMEDIATELY", source)
        self.assertNotIn(
            "LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY", source
        )
        self.assertIn("VALTAN_CANONICAL_READ_DIAGNOSTIC final", header)
        self.assertIn("Is_AutomaticRetryable() const", header)
        self.assertIn("Requires_ProductProjection() const", header)
        self.assertIn("CValtanCanonicalProductReadAdmission Admission;", load_body)
        self.assertIn("VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic", load_body)
        self.assertIn("Admission.Acquire(OutDiagnostic)", load_body)
        self.assertIn("Load_WhileAdmitted(Admission", load_body)
        self.assertNotIn("Try_Acquire", load_body)
        self.assertEqual(
            admitted_body.count("Admission.Validate_StillCurrent("), 2
        )
        self.assertIn("VALTAN_PATTERN_TREE_VIEW StagedView;", admitted_body)
        self.assertIn("OutView = std::move(StagedView);", admitted_body)
        self.assertLess(
            admitted_body.rindex("Admission.Validate_StillCurrent("),
            admitted_body.index("OutView = std::move(StagedView);"),
        )

    def test_valtan_aggregate_stages_every_presentation_component(self) -> None:
        source = VALTAN_SOURCE.read_text(encoding="utf-8")
        begin = source.index("bool_t CValtan::Reload_PatternPresentationAuthoring(")
        end = source.index("bool_t CValtan::Apply_PatternPresentationSample(", begin)
        body = source[begin:end]
        self.assertIn(
            "CValtanCanonicalProductReadAdmission CanonicalAdmission;", body
        )
        self.assertIn(
            "CValtanPresentationGenerationReadAdmission GenerationAdmission;",
            body,
        )
        self.assertIn("GenerationAdmission.Acquire_Receipt(", body)
        self.assertNotIn("GenerationAdmission.Acquire_ExactReceipt(", body)
        self.assertIn(
            "VALTAN_CANONICAL_READ_DIAGNOSTIC CanonicalDiagnostic", body
        )
        self.assertIn("CanonicalAdmission.Acquire(CanonicalDiagnostic)", body)
        for component in (
            "Reload_PatternBindings_WhileAdmitted",
            "Reload_PatternEffectCues_WhileAdmitted",
            "Reload_PatternSoundCues_WhileAdmitted",
            "Reload_CombatObjectSoundCues_WhileAdmitted",
            "Reload_PatternShakeCues_WhileAdmitted",
        ):
            self.assertIn(component, body)
        self.assertIn("auto StagedBindings = std::move", body)
        self.assertIn("auto StagedCombatObjectSoundCues =", body)
        rollback_before_validate = body.index(
            "RestorePrevious();\n\tif (bExact ?"
        )
        exact_validate = body.index(
            "GenerationAdmission.Validate_StillCurrent", rollback_before_validate
        )
        preview_validate = body.index(
            "CanonicalAdmission.Validate_StillCurrent", rollback_before_validate
        )
        aggregate_commit = body.index(
            "m_PatternClipByActionId = std::move(StagedBindings)"
        )
        self.assertLess(rollback_before_validate, exact_validate)
        self.assertLess(rollback_before_validate, preview_validate)
        self.assertLess(exact_validate, aggregate_commit)
        self.assertLess(preview_validate, aggregate_commit)
        self.assertIn(
            "m_PresentationGenerationReceipt =\n"
            "\t\t\tstd::move(CurrentPresentationReceipt);",
            body,
        )

    def test_valtan_effect_prewarm_is_admission_aware(self) -> None:
        for path in (LOADING_SOURCE, CHARACTER_SELECT_SOURCE):
            source = path.read_text(encoding="utf-8")
            cue_load = source.index(
                "CValtanPatternEffectCueDocument::Load_ForProductPrewarm("
            )
            acquire = source.rindex("ProductAdmission.Acquire(", 0, cue_load)
            validate = source.index(
                "ProductAdmission.Validate_StillCurrent(", cue_load
            )
            queue = source.index(
                "CEffectPresentationService::Queue_ProductTargets_Priority(",
                cue_load,
            )
            self.assertLess(acquire, cue_load)
            self.assertLess(cue_load, validate)
            self.assertLess(validate, queue)

    def test_balance_and_workbench_pin_one_exact_canonical_generation(self) -> None:
        balance = BALANCE_SOURCE.read_text(encoding="utf-8")
        balance_header = BALANCE_HEADER.read_text(encoding="utf-8")
        workbench = WORKBENCH_SOURCE.read_text(encoding="utf-8")

        reload_body = function_body(
            balance, "bool Client::CBalanceTool::Reload()"
        )
        acquire = reload_body.index("CanonicalAdmission.Acquire(")
        first_read = reload_body.index('ReadJson(L"Balance/PlayerProfiles.json"')
        tree_load = reload_body.index("ReloadValtanPatternAuthoring(CanonicalAdmission")
        source_query = reload_body.index("QueryValtanSourceRevision(")
        final_validate = reload_body.index(
            "CanonicalAdmission.Validate_StillCurrent(", source_query
        )
        commit = reload_body.index("m_players = std::move(players)")
        self.assertLess(acquire, first_read)
        self.assertLess(tree_load, source_query)
        self.assertLess(source_query, final_validate)
        self.assertLess(final_validate, commit)

        authoring_reload = function_body(
            balance,
            "bool Client::CBalanceTool::ReloadValtanPatternAuthoring(",
        )
        self.assertIn(
            "CValtanPatternTree::Load_WhileAdmitted(", authoring_reload
        )
        self.assertIn("canonicalAdmission, stagedTree", authoring_reload)
        self.assertNotIn("CValtanPatternTree::Load(stagedTree", authoring_reload)

        self.assertIn("std::string repositoryRevision;", balance_header)
        self.assertIn(
            "Verify_ValtanCanonicalSourceRevision_WhileAdmitted(",
            balance_header,
        )
        verify_body = function_body(
            balance,
            "Verify_ValtanCanonicalSourceRevision_WhileAdmitted(",
        )
        self.assertIn("QueryValtanSourceRevision(", verify_body)
        self.assertIn(
            "SourceJoin.repositoryRevision != expectedRepositoryRevision",
            verify_body,
        )

        workbench_reload = function_body(
            workbench,
            "bool_t Client::CActionCompositionWorkbench::Reload_Canonical()",
        )
        balance_reload = workbench_reload.index(
            "m_pBalanceTool->Reload_ValtanSource("
        )
        expected_identity = workbench_reload.index(
            "Get_ValtanCanonicalSourceRevision("
        )
        workbench_acquire = workbench_reload.index(
            "CanonicalAdmission.Acquire("
        )
        workbench_tree = workbench_reload.index(
            "CValtanPatternTree::Load_WhileAdmitted("
        )
        identity_verify = workbench_reload.index(
            "Verify_ValtanCanonicalSourceRevision_WhileAdmitted("
        )
        # The first assignment is the explicit Product-only diagnostic fallback.
        # The final assignment is the FULL_JOIN commit covered by the source
        # identity verification below.
        view_commit = workbench_reload.rindex("m_CanonicalView = std::move(Staged)")
        # One shared admission must cover Product load, typed source reload and
        # the source-identity join.  Taking it before either read is stronger
        # than the retired ordering which sampled Balance first.
        self.assertLess(workbench_acquire, workbench_tree)
        self.assertLess(workbench_tree, balance_reload)
        self.assertLess(balance_reload, expected_identity)
        self.assertLess(expected_identity, identity_verify)
        self.assertLess(identity_verify, view_commit)
        self.assertLess(
            workbench_reload.rindex(
                "CanonicalAdmission.Validate_StillCurrent(", 0, view_commit
            ),
            view_commit,
        )

        save_body = function_body(
            workbench,
            "bool_t Client::CActionCompositionWorkbench::Save_Reload()",
        )
        self.assertIn("Get_ValtanCanonicalSourceRevision(", save_body)
        self.assertIn(
            "CurrentCanonicalSourceRevision != m_strPinnedCanonicalSourceRevision",
            save_body,
        )
        self.assertLess(
            save_body.index("CurrentCanonicalSourceRevision !="),
            save_body.index("Save_ValtanCompositionProduct("),
        )

    def test_python_exclusive_writer_conflicts_with_win32_shared_reader(self) -> None:
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
            lock_path = Path(temporary) / "create-pattern.lock"
            lock_path.write_bytes(b"\0")
            child_script = r"""
import msvcrt
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
with path.open("a+b") as handle:
    handle.seek(0)
    msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
    print("READY", flush=True)
    sys.stdin.read(1)
    handle.seek(0)
    msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
"""
            child = subprocess.Popen(
                [sys.executable, "-c", child_script, str(lock_path)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            try:
                self.assertIsNotNone(child.stdout)
                self.assertEqual(child.stdout.readline().strip(), "READY")
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
                self.assertNotEqual(handle, invalid_handle)
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
                    self.assertEqual(ctypes.get_last_error(), 33)
                finally:
                    close_handle(handle)
            finally:
                if child.stdin is not None:
                    child.stdin.write("x")
                    child.stdin.flush()
                    child.stdin.close()
                stdout, stderr = child.communicate(timeout=5)
                self.assertEqual(
                    child.returncode,
                    0,
                    msg=f"writer child failed; stdout={stdout!r} stderr={stderr!r}",
                )


if __name__ == "__main__":
    unittest.main()
