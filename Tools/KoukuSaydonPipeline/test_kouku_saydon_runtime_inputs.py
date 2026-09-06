import shutil
import subprocess
import unittest
from pathlib import Path


from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


ROOT = Path(__file__).resolve().parents[2]
KOUKU_BOSS_ID = "BOSS_KAKULSAYDON_G1_KOUKU"
KOUKU_ENCOUNTER_ID = "ENCOUNTER_KAKULSAYDON_G1"
KOUKU_PLACEMENT_ID = "boss.kakulsaydon.g1.kouku"
AUDITION_POLICY_ID = "koukusaydon-animation-audition-admission-v1"
AUDITION_NOTE = (
    "Non-final preview survival and admission value; "
    "combat fidelity is not claimed."
)


def load(relative: str):
    return subject.load_json(ROOT / relative)


def unique(rows, field: str, value: str):
    matches = [row for row in rows if row.get(field) == value]
    if len(matches) != 1:
        raise AssertionError(
            f"expected exactly one {field}={value!r}, found {len(matches)}"
        )
    return matches[0]


class KoukuSaydonRuntimeInputTests(unittest.TestCase):
    def run_publisher_fragment(self, script):
        powershell = shutil.which("powershell") or shutil.which("pwsh")
        if not powershell:
            self.skipTest("PowerShell is required for publisher consumer checks")
        result = subprocess.run(
            [powershell, "-NoProfile", "-NonInteractive", "-Command", script],
            cwd=ROOT, capture_output=True, text=True, errors="replace", timeout=30,
        )
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_world_publisher_accepts_raw_kouku_but_keeps_valtan_source_ids(self):
        # Execute the real consumer functions without publishing or modifying
        # tracked inputs. RAW clips deliberately have no extracted action IDs.
        self.run_publisher_fragment(r"""
$ErrorActionPreference = 'Stop'
$repoRoot = (Get-Location).Path
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $repoRoot 'Tools/WorldPipeline/Publish-WorldGameplay.ps1'),
    [ref]$null, [ref]$null)
$names = @('Assert-ExactProperties','Assert-StableId','Assert-JsonInteger',
    'Assert-JsonNumber','Assert-JsonString','Get-EncounterProfiles')
foreach ($function in $ast.FindAll({param($node)
    $node -is [System.Management.Automation.Language.FunctionDefinitionAst]
}, $false)) {
    if ($function.Name -in $names) { Invoke-Expression $function.Extent.Text }
}
$emptyEncounter = 'ENCOUNTER_KAKULSAYDON_G1'
function Read-ProjectJson {
    param([string]$RelativePath)
    $document = Get-Content -LiteralPath (Join-Path $repoRoot $RelativePath) `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($document.encounterId -eq $emptyEncounter) {
        $document.patterns[0].sourceActionIds = @()
    }
    return $document
}
$profiles = Get-EncounterProfiles
if (-not $profiles.ContainsKey('ENCOUNTER_KAKULSAYDON_G1')) {
    throw 'RAW-only Kouku Product was not admitted.'
}
$emptyEncounter = (Get-Content -LiteralPath `
    (Join-Path $repoRoot 'Data/Encounters/Valtan/ValtanEncounter.json') `
    -Raw -Encoding UTF8 | ConvertFrom-Json).encounterId
$rejected = $false
try { $null = Get-EncounterProfiles }
catch { $rejected = $_.Exception.Message -like 'Encounter pattern timing or range is invalid:*' }
if (-not $rejected) { throw 'Valtan must still require extracted source action IDs.' }
""")

    def test_balance_publisher_accepts_referenced_zero_and_rejects_invalid_ids(self):
        # Exercise the publisher's actual foreach block, including its numeric
        # range and duplicate checks, rather than duplicating those rules here.
        self.run_publisher_fragment(r"""
$ErrorActionPreference = 'Stop'
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    (Join-Path (Get-Location) 'Tools/GameplayPipeline/Publish-GameplayBalance.ps1'),
    [ref]$null, [ref]$null)
$validator = $ast.Find({param($node)
    $node -is [System.Management.Automation.Language.FunctionDefinitionAst] -and
    $node.Name -eq 'Assert-JsonInteger'
}, $true)
Invoke-Expression $validator.Extent.Text
$loop = $ast.Find({param($node)
    $node -is [System.Management.Automation.Language.ForEachStatementAst] -and
    $node.Condition.Extent.Text -eq '@($koukuPattern.sourceActionIds)'
}, $true)
if ($null -eq $loop) { throw 'Kouku source action consumer was not found.' }
$koukuPattern = [pscustomobject]@{patternId='REFERENCE_ZERO'; sourceActionIds=@(0,4219714)}
$koukuSourceActionIds = [Collections.Generic.HashSet[uint32]]::new()
Invoke-Expression $loop.Extent.Text
foreach ($invalidIds in @(@(-1), @(0,0))) {
    $koukuPattern.sourceActionIds = $invalidIds
    $koukuSourceActionIds = [Collections.Generic.HashSet[uint32]]::new()
    $rejected = $false
    try { Invoke-Expression $loop.Extent.Text }
    catch { $rejected = $true }
    if (-not $rejected) { throw 'Invalid source action IDs were admitted.' }
}
""")

    def test_boss_catalog_admits_only_kouku_without_a_weapon(self):
        catalog = load("Data/Actors/BossCatalog.json")
        self.assertEqual("lostark.boss-catalog", catalog["schema"])
        self.assertEqual(8, catalog["formatVersion"])

        kouku = unique(catalog["bosses"], "archetypeId", KOUKU_BOSS_ID)
        self.assertEqual(
            {
                "visualAssetId": "KOUKUSAYDON_MN_RPCZ_00",
                "presentationScale": 1.0,
                "bodyModel": (
                    "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel"
                ),
                "weaponModel": None,
                "armorModels": [],
                "armorParts": [],
                "combatObjectVisuals": [],
                "animationSetId": (
                    "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel"
                ),
                "serverProfileId": "boss.kakulsaydon.g1.kouku.server.v1",
                "clientPresentationId": "boss.kakulsaydon.g1.kouku.client.v1",
                "presentationStatus": "complete",
                "presentationClips": {
                    "idle": "rpcz00_idle_battle_1",
                    "chase": "rpcz00_run_battle_1",
                    "patternWindup": "rpcz00_att_battle_3_01",
                    "patternActive": "rpcz00_att_battle_3_07",
                    "patternRecovery": "rpcz00_att_battle_3_09",
                    "dead": "rpcz00_dead_1",
                },
                "bodyModelPreScale": 0.017,
                "weaponModelPreScale": None,
                "weaponModelPreRotationDegrees": None,
            },
            {key: value for key, value in kouku.items() if key != "archetypeId"},
        )

        exact_valtan_models = {
            "BOSS_VALTAN": (
                "Character/Valtan/MN_RPBF_01.wmodel",
                "Character/Valtan/ValtanWeapon.wmodel",
            ),
            "BOSS_VALTAN_GHOST": (
                "Character/Valtan/Ghost/MN_RPBF_02.wmodel",
                "Character/Valtan/ValtanWeapon.wmodel",
            ),
        }
        for archetype_id, expected_models in exact_valtan_models.items():
            row = unique(catalog["bosses"], "archetypeId", archetype_id)
            self.assertEqual(expected_models, (row["bodyModel"], row["weaponModel"]))
            self.assertGreater(row["bodyModelPreScale"], 0.0)
            self.assertGreater(row["weaponModelPreScale"], 0.0)
        self.assertEqual(
            [
                KOUKU_BOSS_ID,
                "BOSS_KAKULSAYDON_G1_SAYDON",
                "BOSS_KAKULSAYDON_G2_KOUKU",
                "BOSS_KAKULSAYDON_G3_SAYDON",
            ],
            [row["archetypeId"] for row in catalog["bosses"]
             if row["weaponModel"] is None],
        )
        # The bingo Saydon and the big Saydon both hold the hammer. The big
        # body is admitted at 0.1 (not 0.017), so its hammer keeps its own
        # pre-scale; both values are tuned from the F1 slice.
        hammer = "Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel"
        bingo = unique(
            catalog["bosses"], "archetypeId", "BOSS_KAKULSAYDON_BINGO_SAYDON")
        self.assertEqual(hammer, bingo["weaponModel"])
        self.assertGreater(bingo["weaponModelPreScale"], 0.0)
        big_saydon = unique(
            catalog["bosses"], "archetypeId", "BOSS_KAKULSAYDON_G2_BIG_SAYDON")
        self.assertEqual(hammer, big_saydon["weaponModel"])
        self.assertGreater(big_saydon["weaponModelPreScale"], 0.0)
        for row in catalog["bosses"]:
            if row["archetypeId"].startswith("BOSS_KAKULSAYDON_"):
                self.assertTrue(
                    row["bodyModel"].startswith("Character/KoukuSaton/"))

    def test_boss_profile_is_explicitly_non_final_audition_tuning(self):
        profiles = load("Data/Balance/BossProfiles.json")
        kouku = unique(profiles["bosses"], "archetypeId", KOUKU_BOSS_ID)
        self.assertEqual(
            {
                "archetypeId": KOUKU_BOSS_ID,
                "encounterId": KOUKU_ENCOUNTER_ID,
                "displayName": "쿠크세이튼",
                "maximumHp": 1_000_000,
                "maximumHealthBars": 1,
                "attackPower": 1,
                "collisionRadius": 1.0,
                "engageDistance": 100.0,
                "moveSpeed": 1.0,
                "phasePolicy": {"kind": "AUTHORED_PATTERN_EVENT"},
                "armorPlates": [],
            },
            kouku,
        )

        receipt = load(
            "Data/Balance/Reference/Official/"
            "2026-08-05.balance-provenance.receipt.json"
        )
        receipt_rows = [
            row for row in receipt["entries"]
            if row["targetDocument"] == "Data/Balance/BossProfiles.json"
            and row["targetId"] == f"boss:{KOUKU_BOSS_ID}"
        ]
        self.assertEqual(set(kouku), {row["targetField"] for row in receipt_rows})
        self.assertEqual(len(receipt["entries"]), receipt["coverage"]["fieldEntryCount"])
        self.assertEqual(8, receipt["coverage"]["bossProfileCount"])
        for row in receipt_rows:
            field = row["targetField"]
            self.assertEqual("PROJECT_TUNED", row["basis"])
            self.assertEqual(
                {"type": "project-policy", "policyId": AUDITION_POLICY_ID},
                row["source"],
            )
            self.assertEqual(kouku[field], row["sourceValue"])
            self.assertEqual(kouku[field], row["resultValue"])
            self.assertEqual(AUDITION_NOTE, row["note"])

    def test_project_tuned_world_anchor_preserves_physical_alias(self):
        world = load("Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json")
        self.assertEqual("lostark.world-gameplay", world["schema"])
        self.assertEqual(6, world["formatVersion"])
        self.assertEqual("LV_LUT_MIDNIGHTC_ED", world["areaId"])
        self.assertEqual(3686, world["revision"])
        self.assertEqual(
            {
                "placementId": KOUKU_PLACEMENT_ID,
                "kind": "boss",
                "archetypeId": KOUKU_BOSS_ID,
                "encounterId": KOUKU_ENCOUNTER_ID,
                "position": [22, -0.05, -62],
                "yawDegrees": 0,
                "enabled": True,
            },
            unique(world["placements"], "placementId", KOUKU_PLACEMENT_ID),
        )
        self.assertEqual(
            {
                "placementId": "stage.kakul.sl01",
                "kind": "playerSpawn",
                "archetypeId": None,
                "encounterId": None,
                "position": [66, 0.0199999996, -102],
                "yawDegrees": 0,
                "enabled": False,
            },
            unique(world["placements"], "placementId", "stage.kakul.sl01"),
        )
        for placement in world["placements"]:
            for event in placement.get("events", []):
                self.assertNotEqual(KOUKU_PLACEMENT_ID, event.get("bossPlacementId"))


if __name__ == "__main__":
    unittest.main()
