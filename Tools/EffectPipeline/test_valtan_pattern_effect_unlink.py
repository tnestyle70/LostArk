from __future__ import annotations

import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "Tools" / "ValtanPipeline" / "Remove-ValtanPatternEffectLink.ps1"

PATTERN_ID = "VALTAN_TARGET"
OTHER_PATTERN_ID = "VALTAN_OTHER"
EFFECT_ID = "effect.valtan.shared.product"
CUE_A = "cue.valtan.target.a"
CUE_B = "cue.valtan.target.b"
OTHER_CUE = "cue.valtan.target.other-effect"
SHARED_CUE = "cue.valtan.other.shared-effect"


PRESENTATION_TEXT = """{
  "schema": "lostark.valtan-pattern-presentation-authoring",
  "formatVersion": 1,
  "bossArchetypeId": "BOSS_VALTAN",
  "encounterId": "ENCOUNTER_VALTAN",
  "scope": "PHASE_ONE",
  "patterns": [
    {
      "patternId": "VALTAN_TARGET",
      "stages": [
        {
          "stageId": "WINDUP",
          "effectCues": [
            {
              "cueId": "cue.valtan.target.a",
              "effectAssetId": "effect.valtan.shared.product",
              "occurrenceId": "occurrence.target.a"
            },
            {
              "cueId": "cue.valtan.target.other-effect",
              "effectAssetId": "effect.valtan.other.product",
              "occurrenceId": "occurrence.target.other"
            }
          ]
        },
        {
          "stageId": "IMPACT",
          "effectCues": [
            {
              "cueId": "cue.valtan.target.b",
              "effectAssetId": "effect.valtan.shared.product",
              "occurrenceId": "occurrence.target.b"
            }
          ]
        }
      ]
    },
    {
      "patternId" : "VALTAN_OTHER",
      "marker" : "KEEP-EXACT",
      "stages": [
        {
          "stageId": "ACTIVE",
          "effectCues": [
            {
              "cueId": "cue.valtan.other.shared-effect",
              "effectAssetId": "effect.valtan.shared.product",
              "occurrenceId": "occurrence.other.shared"
            }
          ]
        }
      ]
    }
  ]
}
"""


FAKE_PROJECTOR = r"""[CmdletBinding()]
param(
    [ValidateSet('Validate', 'PublishV2')]
    [string]$Mode,
    [string]$RepositoryRoot,
    [switch]$WriterLockAlreadyHeld,
    [string]$WriterLockOwnerNonce
)
$ErrorActionPreference = 'Stop'
$utf8 = [Text.UTF8Encoding]::new($false)
$source = Join-Path $RepositoryRoot 'Data\Valtan\Valtan.presentation.json'
$log = Join-Path $RepositoryRoot 'projector.log'
[IO.File]::AppendAllText($log, $Mode + "`n", $utf8)
if ($Mode -eq 'Validate') {
    $validationFailureMarker = Join-Path $RepositoryRoot `
        'fail-candidate-validate-once'
    $validationText = [IO.File]::ReadAllText($source, $utf8)
    if ([IO.File]::Exists($validationFailureMarker) -and
        -not $validationText.Contains('cue.valtan.target.a')) {
        [IO.File]::Delete($validationFailureMarker)
        throw 'injected candidate Validate failure'
    }
    $mutationMarker = Join-Path $RepositoryRoot 'mutate-source-during-validate'
    if ([IO.File]::Exists($mutationMarker)) {
        $text = [IO.File]::ReadAllText($source, $utf8)
        $text = $text.Replace(
            '"scope": "PHASE_ONE"',
            '"scope": "CONCURRENT_EDIT"')
        [IO.File]::WriteAllText($source, $text, $utf8)
    }
    return
}
if (-not $WriterLockAlreadyHeld) {
    throw 'PublishV2 must inherit the shared canonical writer admission.'
}
if ($WriterLockOwnerNonce -cnotmatch '^[0-9a-f]{32}$') {
    throw 'PublishV2 did not receive the exact writer admission nonce.'
}
$published = Join-Path $RepositoryRoot 'Published.presentation.json'
[IO.File]::WriteAllBytes($published, [IO.File]::ReadAllBytes($source))
$failureMarker = Join-Path $RepositoryRoot 'fail-candidate-publish-once'
$sourceText = [IO.File]::ReadAllText($source, $utf8)
if ([IO.File]::Exists($failureMarker) -and
    -not $sourceText.Contains('cue.valtan.target.a')) {
    [IO.File]::Delete($failureMarker)
    throw 'injected candidate PublishV2 failure after Product mutation'
}
"""


class ValtanPatternEffectUnlinkTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.repo = Path(self.temp.name)
        (self.repo / "Data" / "Valtan").mkdir(parents=True)
        (self.repo / "Data" / "Effects" / "Authored").mkdir(parents=True)
        (self.repo / "Tools" / "ValtanPipeline").mkdir(parents=True)

        self.presentation = (
            self.repo / "Data" / "Valtan" / "Valtan.presentation.json"
        )
        self.catalog = self.repo / "Data" / "Effects" / "EffectCatalog.json"
        self.effect = (
            self.repo
            / "Data"
            / "Effects"
            / "Authored"
            / f"{EFFECT_ID}.effect.json"
        )
        self.projector = (
            self.repo
            / "Tools"
            / "ValtanPipeline"
            / "Project-ValtanPatternMaster.ps1"
        )

        self.presentation.write_text(PRESENTATION_TEXT, encoding="utf-8")
        self.catalog.write_text(
            json.dumps(
                {
                    "formatVersion": 1,
                    "effects": [
                        {
                            "effectAssetId": EFFECT_ID,
                            "payloadKind": "DIRECT_AUTHORED_DOCUMENT",
                            "authoringPath": (
                                f"Effects/Authored/{EFFECT_ID}.effect.json"
                            ),
                        },
                        {
                            "effectAssetId": "effect.valtan.other.product",
                            "authoringPath": (
                                "Effects/Authored/"
                                "effect.valtan.other.product.effect.json"
                            ),
                        },
                    ],
                },
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )
        self.effect.write_bytes(b'{"sharedProductEffect":true}\n')
        self.projector.write_text(FAKE_PROJECTOR, encoding="utf-8")

        self.original_presentation = self.presentation.read_bytes()
        self.original_catalog = self.catalog.read_bytes()
        self.original_effect = self.effect.read_bytes()

    def tearDown(self) -> None:
        self.temp.cleanup()

    def run_unlink(
        self,
        mode: str,
        *,
        pattern_id: str = PATTERN_ID,
        effect_id: str = EFFECT_ID,
        cue_ids: str = f"{CUE_A},{CUE_B}",
        failure_injection: str = "None",
        writer_lock_timeout_seconds: float = 30.0,
    ) -> subprocess.CompletedProcess[bytes]:
        return subprocess.run(
            [
                "powershell.exe",
                "-NoProfile",
                "-ExecutionPolicy",
                "Bypass",
                "-File",
                str(SCRIPT),
                "-Mode",
                mode,
                "-PatternId",
                pattern_id,
                "-EffectAssetId",
                effect_id,
                "-CueIds",
                cue_ids,
                "-RepositoryRoot",
                str(self.repo),
                "-FailureInjection",
                failure_injection,
                "-WriterLockTimeoutSeconds",
                str(writer_lock_timeout_seconds),
            ],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )

    def projector_modes(self) -> list[str]:
        log = self.repo / "projector.log"
        if not log.exists():
            return []
        return log.read_text(encoding="utf-8").splitlines()

    def assert_shared_product_unchanged(self) -> None:
        self.assertEqual(self.original_catalog, self.catalog.read_bytes())
        self.assertEqual(self.original_effect, self.effect.read_bytes())

    def test_validate_is_read_only_and_requires_the_exact_cue_set(self) -> None:
        result = self.run_unlink("Validate")
        self.assertEqual(0, result.returncode, result.stdout.decode(errors="replace"))
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())
        self.assert_shared_product_unchanged()
        self.assertEqual(["Validate"], self.projector_modes())
        self.assertFalse((self.repo / "Published.presentation.json").exists())

        mismatch = self.run_unlink("Validate", cue_ids=CUE_A)
        self.assertNotEqual(0, mismatch.returncode)
        self.assertIn(b"exact set", mismatch.stdout)
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())
        self.assertEqual(["Validate"], self.projector_modes())

        invalid_id = self.run_unlink("Validate", pattern_id="../VALTAN_TARGET")
        self.assertNotEqual(0, invalid_id.returncode)
        self.assertIn(b"stable ID", invalid_id.stdout)
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())

    def test_apply_unlinks_only_selected_pattern_and_preserves_shared_asset(self) -> None:
        result = self.run_unlink("Apply")
        self.assertEqual(0, result.returncode, result.stdout.decode(errors="replace"))
        document = json.loads(self.presentation.read_text(encoding="utf-8"))
        target = next(
            row for row in document["patterns"] if row["patternId"] == PATTERN_ID
        )
        target_cues = [
            cue for stage in target["stages"] for cue in stage["effectCues"]
        ]
        self.assertEqual([OTHER_CUE], [cue["cueId"] for cue in target_cues])

        other = next(
            row for row in document["patterns"] if row["patternId"] == OTHER_PATTERN_ID
        )
        self.assertEqual(
            [SHARED_CUE],
            [
                cue["cueId"]
                for stage in other["stages"]
                for cue in stage["effectCues"]
            ],
        )
        text = self.presentation.read_text(encoding="utf-8")
        self.assertIn('      "patternId" : "VALTAN_OTHER",', text)
        self.assertIn('      "marker" : "KEEP-EXACT",', text)
        self.assert_shared_product_unchanged()
        self.assertEqual(["PublishV2", "Validate"], self.projector_modes())
        self.assertEqual(
            self.presentation.read_bytes(),
            (self.repo / "Published.presentation.json").read_bytes(),
        )

    def test_publish_failure_rolls_back_source_and_repairs_product(self) -> None:
        published = self.repo / "Published.presentation.json"
        published.write_bytes(self.original_presentation)
        (self.repo / "fail-candidate-publish-once").write_text(
            "fail", encoding="utf-8"
        )

        result = self.run_unlink("Apply")
        self.assertNotEqual(0, result.returncode)
        self.assertIn(b"source was restored", result.stdout)
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())
        self.assertEqual(self.original_presentation, published.read_bytes())
        self.assert_shared_product_unchanged()
        self.assertEqual(
            ["PublishV2", "PublishV2"], self.projector_modes()
        )
        leftovers = list(self.presentation.parent.glob("*.stage.*")) + list(
            self.presentation.parent.glob("*.replace-backup.*")
        )
        self.assertEqual([], leftovers)

    def test_post_publish_validation_failure_repairs_source_and_product(self) -> None:
        (self.repo / "fail-candidate-validate-once").write_text(
            "fail", encoding="utf-8"
        )
        result = self.run_unlink("Apply")
        self.assertNotEqual(0, result.returncode)
        self.assertIn(b"source was restored", result.stdout)
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())
        self.assertEqual(
            self.original_presentation,
            (self.repo / "Published.presentation.json").read_bytes(),
        )
        self.assert_shared_product_unchanged()
        self.assertEqual(
            ["PublishV2", "Validate", "PublishV2"], self.projector_modes()
        )

    def test_post_replace_verification_failure_uses_preserved_backup(self) -> None:
        result = self.run_unlink(
            "Apply", failure_injection="SourceCommitPostReplace"
        )
        self.assertNotEqual(0, result.returncode)
        self.assertIn(b"source was restored", result.stdout)
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())
        self.assert_shared_product_unchanged()
        self.assertEqual([], self.projector_modes())
        leftovers = list(self.presentation.parent.glob("*.stage.*")) + list(
            self.presentation.parent.glob("*.replace-backup.*")
        )
        self.assertEqual([], leftovers)

    def test_concurrent_source_edit_is_not_clobbered_by_rollback(self) -> None:
        (self.repo / "mutate-source-during-validate").write_text(
            "mutate", encoding="utf-8"
        )
        result = self.run_unlink("Apply")
        self.assertNotEqual(0, result.returncode)
        self.assertIn(b"raw-byte CAS", result.stdout)
        concurrent_text = self.presentation.read_text(encoding="utf-8")
        self.assertIn('"scope": "CONCURRENT_EDIT"', concurrent_text)
        self.assertNotEqual(self.original_presentation, self.presentation.read_bytes())
        self.assert_shared_product_unchanged()
        self.assertEqual(["PublishV2", "Validate"], self.projector_modes())

    def test_utf8_bom_is_rejected_without_writes(self) -> None:
        bom_source = b"\xef\xbb\xbf" + self.original_presentation
        self.presentation.write_bytes(bom_source)
        result = self.run_unlink("Validate")
        self.assertNotEqual(0, result.returncode)
        self.assertIn(b"UTF-8 without BOM", result.stdout)
        self.assertEqual(bom_source, self.presentation.read_bytes())
        self.assert_shared_product_unchanged()
        self.assertEqual([], self.projector_modes())

    @unittest.skipUnless(os.name == "nt", "PowerShell/CRT byte-range contention is Windows-only")
    def test_shared_create_pattern_lock_blocks_unlink_without_any_write(self) -> None:
        import msvcrt

        lock_path = (
            self.repo
            / "out"
            / "ValtanPatternTransactions"
            / "create-pattern.lock"
        )
        lock_path.parent.mkdir(parents=True)
        with lock_path.open("a+b") as handle:
            if lock_path.stat().st_size < 1:
                handle.write(b"\0")
                handle.flush()
                os.fsync(handle.fileno())
            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
            try:
                result = self.run_unlink(
                    "Apply", writer_lock_timeout_seconds=0.0
                )
            finally:
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)

        self.assertNotEqual(0, result.returncode)
        self.assertIn(b"canonical writer admission", result.stdout)
        self.assertEqual(self.original_presentation, self.presentation.read_bytes())
        self.assert_shared_product_unchanged()
        self.assertEqual([], self.projector_modes())


if __name__ == "__main__":
    unittest.main()
