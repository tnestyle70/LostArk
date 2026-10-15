# 쿠크 전체 흐름 검증과 현재 입력 기반 테스트 보정

## G00. 실제 조사 상태와 이번 반영 범위

대응 결과는 [KOUKU_FULL_FLOW_VERIFICATION_RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_FLOW_VERIFICATION_RESULT.md)에 기록한다. 사용자의 전체 요청은 Debug·Release 빌드와 쿠크 입장 연출부터 관문 패턴·종료까지의 검토다. 아래 테스트 보정은 그 요청 중 확인된 검사 입력 불일치를 고치는 한 단계이며, 전체 요청의 완료 판정은 RESULT의 `KOUKU_FINAL_VERIFICATION` 구간에서 관리한다.

현재 Action revision2500·Sequence revision183의 저작 Flow와 게시 Encounter를 대조했다. G1은166항목·후속 포함20패턴, G2는85항목·후속 포함24패턴이며 source→raidGates 투영과 Server stage→Client binding 참조가 일치한다. 이는 GPU 화면·사용자 전투 성공 판정이 아니다. 전체 빌드·native Server 검사·실제 Client 확인은 부모 작업이 별도로 기록한다.

이번 G는 제품을 바꾸지 않고 현재 자료를 검사하지 못하던 Python 테스트 두 개와 사용 설명의 준비 제한시간 한 곳을 교정한다. `test_dice_card_contract.py`는 미사용 logic73을 바꾸고 단일4색 행을 기대했다. 현재 P78은 활성 occurrence를 통해 logic125~128 네 개를 소비한다. `test_kouku_saydon_runtime_inputs.py`는 분리된 publisher 함수 위치와 Parent validator 의존성, 이미 변경된 보스 장비·배치 입력을 반영한다. 런타임 코드, 저작 수치, 게시 데이터, 옵션, 프로젝트 등록은 변경하지 않는다.

## G01. 수정 파일과 책임

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/test_dice_card_contract.py | 실제 P78 활성 occurrence와 정의를 join해 카드·시각·피해·행 출력을 검사한다. |
| 수정 | C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py | 현재 publisher 함수와 저작 boss/world 입력의 계약을 검사한다. |
| 수정 | C:/Users/tnest/Desktop/LostArk/CLAUDE.md | 기존 PREPARING 제한을 현재 Server 코드의20분으로 정정한다. |

Dice `setUp`은 저장한 패턴의 활성 PURSUIT_PROJECTILES occurrence만 찾는다. `project`는 기존 draft.prepare를 호출하며 live publish하지 않는다. 서로 다른50~53 피해로 각 source의 실제 소비를 확인하고, 색상 생략의 일반 pursuit 호환·잘못된 색상/개수 거부·PowerShell bootstrap 행 순서를 검사한다. 테스트 snapshot은 임시 out에만 쓰고 종료 시 canonical 네 파일의 바이트 보존을 확인한다.

Runtime inputs 검사는 `Assert-KoukuParentPatternSequence`의 실제 함수를 추가로 불러오며 source-action 검증은 분리된 `KoukuBootstrapRows.ps1`에서 추출한다. 이미 설치된 Albion visual·G1 Saydon staff와 disabled G1 boss template을 기대한다. World revision은 의미 없는 날짜 숫자 고정 대신 양수 정수 계약으로 검사한다. 나머지 잘못된 입력 거부는 유지한다.

CLAUDE의 PREPARING 문단은 Server/Private/GameRoom_KoukuRaidFlow.cpp의 `preparationTicks = 20u * 60u * 30u`와 대조했다. 09-26 준비 진단 RESULT의 실제4인 준비282.752초, 09-29 RELEASE_SEQUENCE_READY RESULT의 입장 준비 재사용과도 구분한다. Server 시간을 늘리는 변경이 아니라 이미 동작하는 값을 문서에 맞춘다.

## G02. 최종 전체 테스트 코드

### Tools/KoukuSaydonPipeline/test_dice_card_contract.py

변경 종류: 전체 교체. 기존 파일의 함수명과 실행 진입점을 유지한다.

```python
"""Per-visual Dice damage metadata uses the canonical projection and row emitter."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/KoukuSaydonPipeline"))
import project_kouku_saydon_composition as projector
import prepare_kouku_draft_play as draft

PATTERN_ID = "KOUKUSAYDON_G1_PATTERN_78"


class DiceCardContractTests(unittest.TestCase):
    def setUp(self):
        self.source = json.loads((ROOT / projector.SOURCE_PATH).read_text(encoding="utf-8-sig"))
        pattern = next(row for row in self.source["patterns"] if row["patternId"] == PATTERN_ID)
        logics = {row["logicId"]: row for row in self.source["logics"]}
        # 실제 활성 occurrence를 통해 찾는다. 미사용 과거 logic을 수정한 검사는
        # 제품 projection이 그 값을 읽는다는 근거가 될 수 없다.
        self.sources = [(row, logics[row["logicId"]]) for row in pattern["logicOccurrences"]
                        if row.get("enabled", True)
                        and logics[row["logicId"]].get("triggerKind") == "PURSUIT_PROJECTILES"]
        self.assertEqual([f"koukusaydon.g1.logic.{i}" for i in range(125, 129)],
                         [logic["logicId"] for _, logic in self.sources])
        self.assertEqual([[symbol] for symbol in ("HEART", "SPADE", "CLUB", "DIAMOND")],
                         [logic["cardSymbols"] for _, logic in self.sources])

    def project(self, source):
        _, artifacts = draft.prepare(source, ROOT, pattern_id=PATTERN_ID)
        encounter = json.loads(artifacts[Path("encounter.json")])
        pattern = next(row for row in encounter["patterns"] if row["patternId"] == PATTERN_ID)
        return pattern["pursuitProjectiles"], artifacts

    def test_symbol_contract_rejects_bad_values_and_preserves_optional_generic_pursuit(self):
        # 현재 카드마다 visual이 하나다. 배열 개수와 symbol 검증을 따로 유지한다.
        for _, logic in self.sources:
            for bad in ([], ["HEART", "SPADE"], ["NONE"], ["red"], [[]]):
                with self.subTest(logic=logic["logicId"], bad=bad), self.assertRaises(projector.CompositionError):
                    projector._pursuit_fields({**logic, "cardSymbols": bad}, "Dice")
            self.assertEqual(logic["cardSymbols"], projector._pursuit_fields(logic, "Dice")["cardSymbols"])
            generic = copy.deepcopy(logic)
            generic.pop("cardSymbols")
            self.assertNotIn("cardSymbols", projector._pursuit_fields(generic, "Generic pursuit"))

    def test_selected_projection_keeps_visual_order_color_and_source_immutable(self):
        # 각 occurrence의 독립 값을 바꿔 읽기 경로를 검증하며 디스크 정본은 건드리지 않는다.
        for index, (_, logic) in enumerate(self.sources):
            logic["projectileHits"][0]["damagePercent"] = 50 + index
        before = copy.deepcopy(self.source)
        rows, _ = self.project(self.source)
        self.assertEqual(before, self.source)
        self.assertEqual([row["occurrenceId"] for row, _ in self.sources],
                         [row["occurrenceId"] for row in rows])
        for (occurrence, logic), row in zip(self.sources, rows):
            with self.subTest(occurrence=occurrence["occurrenceId"]):
                self.assertEqual((occurrence["startMs"], occurrence["durationMs"]),
                                 (row["startMs"], row["durationMs"]))
                self.assertEqual(logic["cardSymbols"], row["cardSymbols"])
                self.assertEqual(len(logic["visualIds"]), len(row["visualIds"]))
                self.assertEqual(logic["projectileHits"][0]["damagePercent"],
                                 row["projectileHits"][0]["damagePercent"])
        generic = copy.deepcopy(self.source)
        source_ids = {logic["logicId"] for _, logic in self.sources}
        for logic in generic["logics"]:
            if logic["logicId"] in source_ids:
                logic.pop("cardSymbols")
        generic_rows, _ = self.project(generic)
        self.assertEqual([{k: v for k, v in row.items() if k != "cardSymbols"} for row in rows], generic_rows)

    def test_selected_projection_rejects_invalid_consumed_card_symbols_without_mutation(self):
        for occurrence, logic in self.sources:
            for invalid in (["NONE"], ["HEART", "SPADE"]):
                candidate = copy.deepcopy(self.source)
                next(row for row in candidate["logics"] if row["logicId"] == logic["logicId"])["cardSymbols"] = invalid
                before = copy.deepcopy(candidate)
                with self.subTest(occurrence=occurrence["occurrenceId"], invalid=invalid), self.assertRaises(projector.CompositionError):
                    self.project(candidate)
                self.assertEqual(before, candidate)

    def test_canonical_rows_emit_symbols_between_parent_and_hit_without_publishing(self):
        for index, (_, logic) in enumerate(self.sources):
            logic["projectileHits"][0]["damagePercent"] = 50 + index
        canonical = [ROOT / projector.SOURCE_PATH, ROOT / projector.ENCOUNTER_PATH,
                     ROOT / projector.PRESENTATION_PATH, ROOT / "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap"]
        before = {path: path.read_bytes() for path in canonical}
        with tempfile.TemporaryDirectory(dir=ROOT / "out", prefix="DiceContract-") as temporary:
            output = Path(temporary)
            source = output / "snapshot.json"
            source.write_text(json.dumps(self.source), encoding="utf-8")
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                str(ROOT / "Tools/KoukuSaydonPipeline/Prepare-KoukuDraftPlay.ps1"), "-RepoRoot", str(ROOT),
                "-SourcePath", str(source), "-OutputDirectory", str(output / "request"), "-PatternId", PATTERN_ID],
                capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=60)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            rows = [line.split("\t") for line in (output / "request/gameplay.rows").read_text().splitlines()]
            selected = [row for row in rows if row[0] in ("PATTERNPURSUITPROJECTILES", "PATTERNPURSUITCARDS", "PATTERNATTACKHIT")]
            self.assertEqual(3 * len(self.sources), len(selected))
            for occurrence, logic in self.sources:
                group = [row for row in selected if row[2] == PATTERN_ID and row[3] == occurrence["occurrenceId"]]
                self.assertEqual(["PATTERNPURSUITPROJECTILES", "PATTERNPURSUITCARDS", "PATTERNATTACKHIT"], [row[0] for row in group])
                self.assertEqual(",".join(logic["cardSymbols"]), group[1][-1])
                self.assertEqual(["MAX_HP_PERCENT", str(logic["projectileHits"][0]["damagePercent"]), "-"], group[2][-3:])
        self.assertEqual(before, {path: path.read_bytes() for path in canonical})


if __name__ == "__main__":
    unittest.main()

```

### Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py

변경 종류: 전체 교체. 기존 파일의 함수명과 실행 진입점을 유지한다.

```python
import shutil
import subprocess
import unittest
from pathlib import Path


from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as subject


ROOT = Path(__file__).resolve().parents[2]
KOUKU_BOSS_ID = "BOSS_KOUKUSAYDON_G1_KOUKU"
KOUKU_ENCOUNTER_ID = "ENCOUNTER_KOUKUSAYDON_G1"
KOUKU_PLACEMENT_ID = "boss.koukusaydon.g1.kouku"
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
$parentAst = [System.Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $repoRoot 'Tools/KoukuSaydonPipeline/KoukuParentSequenceContract.ps1'),
    [ref]$null, [ref]$null)
$parentValidator = $parentAst.Find({param($node)
    $node -is [System.Management.Automation.Language.FunctionDefinitionAst] -and
    $node.Name -eq 'Assert-KoukuParentPatternSequence'
}, $true)
if ($null -eq $parentValidator) { throw 'Parent sequence validator was not found.' }
Invoke-Expression $parentValidator.Extent.Text
$emptyEncounter = 'ENCOUNTER_KOUKUSAYDON_G1'
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
if (-not $profiles.ContainsKey('ENCOUNTER_KOUKUSAYDON_G1')) {
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
    (Join-Path (Get-Location) 'Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1'),
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

    def test_encore_reuses_gate3_body_and_staff(self):
        catalog = load("Data/Actors/BossCatalog.json")
        bingo = unique(catalog["bosses"], "archetypeId", "BOSS_KOUKUSAYDON_BINGO_SAYDON")
        gate3 = unique(catalog["bosses"], "archetypeId", "BOSS_KOUKUSAYDON_G3_SAYDON")
        for key in ("bodyModel", "bodyModelPreScale", "weaponModel", "weaponModelPreScale", "weaponModelPreRotationDegrees"):
            self.assertEqual(gate3[key], bingo[key], key)
        self.assertEqual("Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel", bingo["weaponModel"])

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
                "combatObjectVisuals": [{
                    "combatObjectArchetypeId": "combatobject.kouku.albion.bluecircle",
                    "clientVisualId": "combatvisual.kouku.albion.bluecircle",
                    "effectAssetId": "effect.kouku.albion.bluecircle.warning.impact.runtime",
                }],
                "animationSetId": (
                    "Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel"
                ),
                "serverProfileId": "boss.koukusaydon.g1.kouku.server.v1",
                "clientPresentationId": "boss.koukusaydon.g1.kouku.client.v1",
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
                "BOSS_KOUKUSAYDON_G2_KOUKU",
            ],
            [row["archetypeId"] for row in catalog["bosses"]
             if row["weaponModel"] is None],
        )
        # Gate 1 Saydon now reuses the installed Gate 3 staff model.
        gate1_saydon = unique(catalog["bosses"], "archetypeId", "BOSS_KOUKUSAYDON_G1_SAYDON")
        self.assertEqual("Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel", gate1_saydon["weaponModel"])
        self.assertGreater(gate1_saydon["weaponModelPreScale"], 0.0)
        # Encore reuses Gate 3 body/staff admission; Big Saydon owns its hammer.
        hammer = "Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel"
        bingo = unique(catalog["bosses"], "archetypeId", "BOSS_KOUKUSAYDON_BINGO_SAYDON")
        gate3 = unique(catalog["bosses"], "archetypeId", "BOSS_KOUKUSAYDON_G3_SAYDON")
        for key in ("bodyModel", "bodyModelPreScale", "weaponModel", "weaponModelPreScale", "weaponModelPreRotationDegrees"):
            self.assertEqual(gate3[key], bingo[key], key)
        self.assertEqual("Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel", bingo["weaponModel"])
        big_saydon = unique(
            catalog["bosses"], "archetypeId", "BOSS_KOUKUSAYDON_G2_BIG_SAYDON")
        self.assertEqual(hammer, big_saydon["weaponModel"])
        self.assertGreater(big_saydon["weaponModelPreScale"], 0.0)
        for row in catalog["bosses"]:
            if row["archetypeId"].startswith("BOSS_KOUKUSAYDON_"):
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
        self.assertIsInstance(world["revision"], int)
        self.assertGreater(world["revision"], 0)
        self.assertEqual(
            {
                "placementId": KOUKU_PLACEMENT_ID,
                "kind": "boss",
                "archetypeId": KOUKU_BOSS_ID,
                "encounterId": KOUKU_ENCOUNTER_ID,
                "position": [22, -0.0500000007, -62],
                "yawDegrees": 0,
                "enabled": False,
            },
            unique(world["placements"], "placementId", KOUKU_PLACEMENT_ID),
        )
        self.assertEqual(
            {
                "placementId": "stage.kouku.sl01",
                "kind": "playerSpawn",
                "archetypeId": None,
                "encounterId": None,
                "position": [66, 0.0199999996, -102],
                "yawDegrees": 0,
                "enabled": False,
            },
            unique(world["placements"], "placementId", "stage.kouku.sl01"),
        )
        for placement in world["placements"]:
            for event in placement.get("events", []):
                self.assertNotEqual(KOUKU_PLACEMENT_ID, event.get("bossPlacementId"))


if __name__ == "__main__":
    unittest.main()

```

## G03. CLAUDE.md의 정확한 교체 문단

`Complete Play - Sequences + Pattern Flow` 설명에서 다음 한 줄만 교체한다. 다른10초·5초·Loader timeout은 유지한다.

```text
준비 중에는 위치·보스 상태를 변경하지 않으며, FAILED·20분 timeout·owner STOP은 기존 상태를 보존한다. 입장 종료 전에는 전투 배우를 생성하지 않는다.
```

## G04. 적용과 검증

1. 현재 테스트 두 파일과 승인 후보의 SHA256을 다시 대조한다. 적용 직전 기준 바이트를 out에 보관하고 같은 파일이 바뀌면 덮어쓰지 않는다.
2. 위 두 테스트를 교체한 뒤 CLAUDE의 지정한 한 문구만 교체한다. 기존 인코딩과 각 대상의 줄바꿈을 유지한다.
3. 아래 집중 검사로 실제 설치 테스트를 실행한다. Python과 PowerShell은 사용자/제품 프로세스를 시작하지 않는다.

```powershell
$env:PYTHONPATH=(Get-Location).Path + ';' + (Join-Path (Get-Location).Path 'Tools/KoukuSaydonPipeline')
python -m unittest test_dice_card_contract test_kouku_saydon_runtime_inputs -v
```

4. 테스트 파일 Python parse, 세 변경 파일과 이 PLAN의 `git diff --check`, 최종 SHA256과 후보 코드 일치를 확인한다. 새 C++·project/filter·schema 변경이 없으므로 이 G 자체의 제품 재컴파일은 필요하지 않다. 요청된 Debug/Release 제품 빌드는 부모 작업의 별도 완료 조건이다.
5. 위 테스트 성공은 입장 연출·전투·종료 화면 전체 성공을 의미하지 않는다. 실행 수치와 남은 사용자 화면 확인을 결과에서 분리한다.

## G05. 전체 요청의 남은 완료 조건

1. 동시 변경이 끝난 현재 소스 기준으로 요청된 Debug·Release 빌드를 완료하고, 구성별 명령·종료 코드·오류와 경고·생성된 EXE의 SHA256을 RESULT에 연결한다. 테스트 문서 보정에 대한 컴파일 불필요 판정과 사용자가 요청한 제품 전체 빌드 완료는 구분한다.
2. 최종 Server로 기존 집중 native 검사를 실행하고 입력 데이터와 EXE SHA256을 함께 고정한다. 갱신 전 `release-current-001` 결과를 최신 Server 검증으로 승격하지 않는다. `Run_KoukuBundles`의 본문은 `_DEBUG`에 있으므로 Release의 bootstrap 2개 PASS만으로 bundle 검증을 완료하지 않는다.
3. 실제 Server와 Client에서 입장 준비·연출·전투 시작·관문 전환·미니게임 복귀·빙고·종료를 확인한다. 프로그램 내부 시계 이동과 직접 보스 처치가 있는 계약 검사는 이 실제 플레이 확인을 대신하지 않는다. 도구와 로드 확인도 실제 열어 본 범위와 정적 검토 범위를 구분한다.
4. 추가 결함이 확인되면 정확한 소비 경로와 원인을 먼저 확정하고 이 PLAN의 해당 변경 범위를 갱신한다. 팀장 렌더링 옵션과 다른 작업의 변경은 보존한다.

이 절은 남은 작업의 기준이다. 실제 수행 여부·통과 수치·미완료 사유는 대응 RESULT에만 기록한다.


## G06. Compiled Shader Closure의 Visual Studio 설치 탐색 정비

<!-- KOUKU_SHADER_CLOSURE_PRERELEASE_PLAN -->

수정 파일은 `C:/Users/tnest/Desktop/LostArk/Tools/Build/Test-CompiledShaderClosure.ps1` 하나다.
`Invoke-ProductEffectShaderWarpProbe`의 `$installationPath` 조회에서 `-prerelease`만 추가한다.
현재 일반 조회는 성공 종료하면서 빈 문자열을 반환하고, 해당 옵션을 넣은 동일 조회는 설치된
`C:/Program Files/Microsoft Visual Studio/18/Insiders`를 찾는다. 공식 Product 빌드의
`Invoke-BuildAndRegression.ps1`도 Preview를 포함해 조회하므로 검사의 설치 탐색 범위를 맞춘다.

변경 종류: 아래 3줄의 완전한 조회 블록 교체. 나머지 shader 생산자·소비자·해시·WARP·실패 판정은 보존한다.

```powershell
    $installationPath = [string](@(& $vswhere -latest -products * -prerelease `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath) | Select-Object -First 1)
```

이 스크립트는 `BuildDomains.json`의 Product source fingerprint 입력이다. 진행 중 빌드의
source-before/after 불일치를 숨기지 않으며, 적용 시점을 빌드 담당자와 맞추고 최종 원본 SHA를
CAS로 재확인한 뒤 기존 인코딩·개행을 유지해 교체한다. 현재 원본 SHA256은
`dae9dc2379503d01409b8f44b879a7df8042a652f4c620e2850275dc8da19dce`다.

검증은 PowerShell parse와 한 옵션만 바뀐 diff를 먼저 확인한 뒤 실제 Release·Debug
`Test-CompiledShaderClosure.ps1 -Configuration <구성> -RepositoryRoot C:/Users/tnest/Desktop/LostArk -Modules Product`
실행으로 수행한다. WARP 검사는 독립 `ProductEffectShaderWarpProbe.cpp`와 Effects11 library를
링크하며 Product OBJ·Engine DLL/import library를 링크하지 않는다. 이 설치의 기본 MSVC
14.51.36231로 실행되는 WARP 검사와 Product의 14.44.35207 빌드 증거는 구분한다.
새 C++·project/filter·shader·제품 렌더링 옵션 변경은 없다. 실행 결과는 RESULT에만 기록한다.


## G07. Native Dice의 기존 받는 피해 표본 계약 검사

### G07-01. 현재 실패 원인과 반영 경계

최신 Release Server SHA `af445779082f42da71e125d1a694f4e3b271e98a5ee2f303606c62ac73ce9bc7`의 격리 `--kouku-dice-hit-contract-test`는152 PASS·30 FAIL이었다. 실패는 네 exact-HP 기대 블록에서 발생했다. `ServerCombatHitRuntime.cpp:587`의 기존 ±10% 정수 표본과719의 적용은 2026-09-30 commit `6a988d7037d9ca1f1115f421db26b6d7a5240db8`부터 존재하며, 같은 commit의 Dice 검사에는 정확90%·500 기대가 남아 있다. 현재 팀 gameplay 사용서2817행도 일반 World→Player 피해 표본을 명시한다.

최대HP1000/2000/3000/4000/6000에서 실제 피해829/1732/2832/3469/5814는 entity101/102/103/104/106·tick101·serial1의 기존 표본과 일치한다. 비율·fixed 입력은 `bIgnoreDefense=true`이고 fresh fixture는 buff·shield를 갖지 않는다. 최신 직업 방어·balance profile이 이 차이의 원인이 아니다. Python 데이터 테스트 두 개의 수정도 이 native 입력이나 표본 함수를 바꾸지 않았다.

이번 G는 **기존 C++ 계약 테스트 한 파일만 수정**한다. 제품 피해 계산·Server authority·저작 또는 게시 데이터·옵션·프로젝트 등록은 바꾸지 않는다. out 후보는 아직 제품 소스에 적용하지 않았으며 현재 Product 빌드가 끝난 뒤 부모 작업의 적용 시점에만 반영한다.

### G07-02. 파일·함수·불변식

| 구분 | 절대 경로 | 책임 |
|---|---|---|
| 수정 후보 | C:/Users/tnest/Desktop/LostArk/Server/Private/ServerGameplayContractTests_KoukuRaid.cpp | `Run_KoukuDiceDamageContracts`의 피격 expectation을 기존 표본 계약에 맞춘다. |

`Run_KoukuDiceDamageContracts` 내부 `execute`의 `TESTS tests;` 바로 뒤에 read-only `matchesIncomingDamage` lambda를 추가한다. 입력 player와 event vector는 현재 fixture가 소유하며 lambda는 호출 중에만 차용한다. per-hit 기준량 ± floor(기준량/10), 정확한 대상 event 수, incoming NORMAL flag, event 합=실제 HP 감소, fresh player의 serial=타격 수를 함께 검사한다. RNG 구현이나 관측된 결과값을 expected로 복제하지 않는다. 면역·원거리·다른 조준 대상의 event와 serial은0이어야 한다.

수정 블록은 CONTACT/TIMED 카드, interception, symbol 없는 generic pursuit, fixed damage 반복 tick 네 곳이다. 카드 문양·바인딩 소유권·burst·retirement·실제 P79→P78 흐름의 검사는 유지한다. fixed 검사는 entry300, 중복300, 직전314, 반복315, 종료330마다 event 수와 player별 serial을 확인하여 집계 오차로 타이밍 결함이 가려지지 않게 한다. fixed500은 최대HP와 독립인 기준량이며 각 실제 hit은450..550이다.

### G07-03. 적용 후 전체 C++ 코드 정본

변경 종류: 기존 함수 내부 테스트 보정. 기존 파일의 UTF-8 BOM 없음·CRLF를 유지한다. 아래는 변경하지 않은 나머지 함수까지 포함한 전체 후보이며 신규 C++ 파일이 아니므로 `.vcxproj`·`.filters` 추가가 없다.

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
#include "ServerApp.h"
#include <Windows.h>
#include <array>
#include <filesystem>
#include <fstream>
#include "Gameplay/KoukuArenaReadyAreas.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;

void CServerGameplayContractRunner::Run_KoukuGate3Entry(TESTS& tests)
{
    tests.Require(Is_KoukuGate3EntryAura(-22.20617676f, 25.59f, 954.5942993f) &&
        !Is_KoukuGate3EntryAura(-11.9999292f, 25.59f, 964.54328125f) &&
        !Is_KoukuGate3EntryAura(-22.20617676f, 1.32f, 954.5942993f) &&
        !Is_KoukuGate3EntryAura(std::numeric_limits<float>::quiet_NaN(), 25.59f, 954.5942993f),
        "Gate 3 entry aura admits its source square and excludes the respawn aura, lower floor and nonfinite input");
    tests.Require(Is_KoukuGate3EntryTerrace(-17.509552f, 25.6f, 960.53015f) &&
        !Is_KoukuGate3EntryTerrace(-17.509552f, 1.32f, 960.53015f) &&
        !Is_KoukuGate3EntryTerrace(-31.f, 25.6f, 960.53015f) &&
        !Is_KoukuGate3EntryTerrace(std::numeric_limits<float>::quiet_NaN(), 25.6f, 960.53015f),
        "Gate 3 entry deck predicate admits the authored arrival and excludes the lower floor, outside and nonfinite input");
    C2S_GATE_PROGRESS_PROPOSE wire; wire.iRequestSequence = 1u;
    wire.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; wire.eKind = GATE_PROGRESS_KIND::ENTER_GATE3;
    CPacketWriter writer; tests.Require(Write_Message(writer, wire), "Explicit Gate 3 entry writes the existing gate proposal packet");
    CPacketReader reader{writer.Get_Buffer()}; C2S_GATE_PROGRESS_PROPOSE decoded;
    tests.Require(Read_Message(reader, decoded) && decoded.eKind == GATE_PROGRESS_KIND::ENTER_GATE3,
        "Explicit Gate 3 entry round-trips through the production packet codec");
    auto invalidBytes = writer.Get_Buffer(); invalidBytes.back() = static_cast<std::uint8_t>(GATE_PROGRESS_KIND::END);
    CPacketReader invalidReader{invalidBytes};
    tests.Require(!Read_Message(invalidReader, decoded), "Unknown gate proposal kind remains rejected");
    S2C_GATE_PROGRESS_STATE state; state.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
    state.iGateCount = 4u; state.eKind = GATE_PROGRESS_KIND::ENTER_GATE3;
    state.bClosed = true; state.eResult = GATE_PROGRESS_VOTE_RESULT::CANCELLED;
    CPacketWriter stateWriter; tests.Require(Write_Message(stateWriter, state), "Automatic Gate 3 closed result with no proposal encodes");
    CPacketReader stateReader{stateWriter.Get_Buffer()}; S2C_GATE_PROGRESS_STATE decodedState;
    tests.Require(Read_Message(stateReader, decodedState) && decodedState.eKind == GATE_PROGRESS_KIND::ENTER_GATE3 &&
        decodedState.bClosed && !decodedState.iProposalId && decodedState.eResult == GATE_PROGRESS_VOTE_RESULT::CANCELLED,
        "Automatic Gate 3 failure remains a closed result with no vote through the state packet");

    for (unsigned count = 1u; count <= 4u; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Gate 3 entry fixture loads the actual catalog, world and navigation");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= count + 1u; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = -22.20617676f; player.fPositionY = 25.6f; player.fPositionZ = 954.5942993f;
            if (id > count) { player.fPositionX = 20.f; player.fPositionZ = 970.f; }
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
            if (id <= count) { room->m_PartyIdByPlayerId[id] = 77u; room->m_PartyMembersByPartyId[77u].push_back(id); }
        }
        tests.Require(room->Spawn_GatePlacement("boss.koukusaydon.g2.big-saydon"), "Preservation fixture has an existing disabled-placement boss");
        room->m_GateProgress.iCurrentGate = count % 2u;
        const auto previousGate = room->m_GateProgress.iCurrentGate;
        const auto previousPlayers = room->m_Players;
        const auto previousEntities = room->m_WorldEntities;
        unsigned sequence = 1u;
        const auto manual = [&](unsigned id = 1u) {
            auto request = wire; request.iRequestSequence = sequence++;
            room->Handle_GateProgressPropose(500u + id, request);
        };
        const auto beginCountdown = [&]() {
            auto& leader = room->m_Players.at(1u);
            leader.fPositionY = 1.32f; ++room->m_iServerTick; room->Expire_GateProgressVote();
            leader.fPositionY = 25.6f; ++room->m_iServerTick; room->Expire_GateProgressVote();
            ++room->m_iServerTick; room->Expire_GateProgressVote();
        };
        const auto completeCountdown = [&]() {
            beginCountdown(); room->m_iServerTick += 300u; room->Expire_GateProgressVote();
        };
        const auto preserved = [&]() {
            tests.Require(!room->m_GateProgress.iProposalId && room->m_GateProgress.iCurrentGate == previousGate &&
                room->m_WorldEntities.size() == previousEntities.size(), "Rejected entry preserves current gate and existing boss count");
            for (std::size_t index = 0; index < previousEntities.size() && index < room->m_WorldEntities.size(); ++index)
                tests.Require(room->m_WorldEntities[index].iNetEntityId == previousEntities[index].iNetEntityId &&
                    room->m_WorldEntities[index].iCurrentHp == previousEntities[index].iCurrentHp &&
                    room->m_WorldEntities[index].fPositionX == previousEntities[index].fPositionX,
                    "Rejected entry preserves the exact previous boss identity, HP and position");
            for (const auto& [id, player] : room->m_Players)
            {
                const auto& old = previousPlayers.at(id);
                tests.Require(player.fPositionX == old.fPositionX && player.fPositionY == old.fPositionY && player.fPositionZ == old.fPositionZ,
                    "Rejected entry preserves participant and observer positions");
            }
        };
        manual(); preserved();
        if (count > 1u) { manual(count); preserved(); }
        beginCountdown(); room->m_iServerTick += 299u; manual(); room->Expire_GateProgressVote(); preserved();
        tests.Require(room->m_iArenaAssemblyStartTick && !room->m_bArenaAssemblyAttempted,
            "Gate 3 manual commands cannot bypass the 300-tick Server countdown");
        room->m_Players.at(count).iCurrentHp = 0u; room->Expire_GateProgressVote();
        tests.Require(!room->m_iArenaAssemblyStartTick, "A dead participant resets Gate 3 assembly readiness");
        room->m_Players.at(count).iCurrentHp = 100u; preserved();
        room->m_Players.at(count).bPatternBound = true; completeCountdown();
        room->m_Players.at(count).bPatternBound = false; preserved();
        tests.Require(room->m_bArenaAssemblyAttempted, "A failed automatic admission latches until the leader leaves");
        const auto failedId = room->m_iNextNetEntityId;
        const auto failedFrames = sessions.front()->m_OutboundFrames.size();
        room->m_iServerTick += 30u; room->Expire_GateProgressVote();
        tests.Require(room->m_iNextNetEntityId == failedId && room->m_GateProgress.iCurrentGate == previousGate &&
            sessions.front()->m_OutboundFrames.size() == failedFrames,
            "An occupied failed assembly does not retry or send another failure frame every tick");
        const auto nextId = room->m_iNextNetEntityId; room->m_iNextNetEntityId = INVALID_NET_ENTITY_ID;
        completeCountdown(); room->m_iNextNetEntityId = nextId; preserved();
        const auto navigation = room->m_ServerNavigation; room->m_ServerNavigation = {};
        completeCountdown(); room->m_ServerNavigation = navigation; preserved();
        auto* gatePlacement = const_cast<WORLD_BOOTSTRAP_PLACEMENT*>(room->Find_Placement("boss.koukusaydon.g3.saydon"));
        tests.Require(gatePlacement != nullptr, "Gate 3 staged boss has an authored placement");
        if (gatePlacement)
        {
            const auto archetype = gatePlacement->strArchetypeId; gatePlacement->strArchetypeId = "invalid.gate3.contract";
            completeCountdown(); gatePlacement->strArchetypeId = archetype; preserved();
        }
        if (count > 1u)
        {
            beginCountdown(); room->m_iServerTick += 300u;
            std::swap(room->m_PartyMembersByPartyId[77u].front(), room->m_PartyMembersByPartyId[77u].back());
            room->Expire_GateProgressVote();
            tests.Require(!room->m_iArenaAssemblyStartTick, "Leader changes reset the exact Gate 3 countdown roster");
            std::swap(room->m_PartyMembersByPartyId[77u].front(), room->m_PartyMembersByPartyId[77u].back()); preserved();
            beginCountdown(); room->m_iServerTick += 300u;
            room->m_PartyMembersByPartyId[77u].push_back(count + 1u); room->Expire_GateProgressVote();
            tests.Require(!room->m_iArenaAssemblyStartTick, "Membership changes cannot commit the previous countdown roster");
            room->m_PartyMembersByPartyId[77u].pop_back(); preserved();
        }
        completeCountdown();
        tests.Require(!room->m_GateProgress.iProposalId && room->m_GateProgress.iCurrentGate == 3u && !room->Is_KoukuRaidRunning(),
            "Automatic non-raid entry targets Gate 3 after ten seconds without a proposal");
        tests.Require(std::any_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& entity) {
            return entity.strPlacementId == "boss.koukusaydon.g3.saydon"; }), "Prebuilt Gate 3 boss commits through the existing spawn consumer");
        for (unsigned id = 1u; id <= count; ++id)
        {
            const auto& player = room->m_Players.at(id);
            tests.Require(std::abs(player.fPositionX + 2.45f) < .01f && std::abs(player.fPositionZ - 945.17f) < .01f,
                "Every assembled party participant reaches the original Gate 3 combat spawn");
        }
        const auto& observer = room->m_Players.at(count + 1u);
        tests.Require(observer.fPositionX == previousPlayers.at(count + 1u).fPositionX &&
            observer.fPositionZ == previousPlayers.at(count + 1u).fPositionZ, "Unrelated room observer is never moved by automatic party entry");
        const auto committedId = room->m_iNextNetEntityId; manual(); room->Expire_GateProgressVote();
        tests.Require(!room->m_GateProgress.iProposalId && room->m_iNextNetEntityId == committedId,
            "A duplicate entry command from the combat floor cannot restart the boss");
    }
}

void CServerGameplayContractRunner::Run_KoukuRaidIntegration(TESTS& tests)
{
    const auto begin = [&](CGameRoom& room, const SESSION_ID session, const C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request) {
#ifdef _DEBUG
        room.Handle_KoukuRaidRequest(session, request);
#else
        // The Release entry collider calls this same admission consumer. Debug START stays rejected.
        std::string reason;
        tests.Require(room.Begin_KoukuRaidPreparation(session, request, reason), "Release entry admission pins the actual published raid");
#endif
    };
    const auto finishEntryIdle = [&](CGameRoom& room) {
        auto& run = room.m_KoukuRaid;
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT || run.bEntryRunning) return;
        const auto start = run.State.iStartTick;
        tests.Require(run.iNextEntryTick == start + 90u && !run.bEntryRunning,
            "Every gate enters combat with a three-second idle before its first flow entry");
        room.m_iServerTick = run.iNextEntryTick - 1u;
        room.Update_KoukuRaid(room.m_iServerTick);
        tests.Require(!run.bEntryRunning && std::all_of(room.m_WorldEntities.begin(), room.m_WorldEntities.end(),
            [](const auto& boss) { return boss.strPatternId.empty() && boss.eAction == SERVER_ENTITY_ACTION::IDLE; }),
            "The first attack cannot begin one tick before the entry idle ends");
        room.m_iServerTick = run.iNextEntryTick;
        room.Update_KoukuRaid(room.m_iServerTick);
        tests.Require(run.bEntryRunning, "The first authored flow entry starts at the three-second boundary");
    };
    for (const auto& [marioPattern, bars] : std::array<std::pair<const char*, unsigned>, 4u>{
        {{"KOUKUSAYDON_G1_PATTERN_88", 155u}, {"KOUKUSAYDON_G1_PATTERN_91", 125u}, {"KOUKUSAYDON_G1_PATTERN_92", 80u}, {"KOUKUSAYDON_G1_PATTERN_93", 55u}}})
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        auto& run = room->m_KoukuRaid;
        run.pCatalog = room->m_GameplayCatalog.Get_ActiveGeneration();
        run.iOwnerSessionId = 501u; run.PlayerIds = {1u};
        run.State.iRunEpoch = 1u; run.State.iOwnerPlayerId = 1u;
        run.State.ParticipantPlayerIds = run.PlayerIds;
        run.State.strGateId = "GATE3"; run.State.ePhase = KOUKUSAYDON_RAID_PHASE::CINEMATIC;
        run.State.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        run.State.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*run.pCatalog);
        auto& player = room->m_Players[1u];
        player.iPlayerId = 1u; player.iSessionId = 501u; player.iNetEntityId = 101u;
        player.iCurrentHp = player.iMaximumHp = 100000000u;
        player.isCombatReady = true; player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        player.fPositionX = -2.45f; player.fPositionY = 1.32f; player.fPositionZ = 945.17f;
        room->m_PlayerIdBySessionId[501u] = 1u;
        room->m_iServerTick = 100u;
        const bool spawned = room->Start_KoukuRaidCombat(100u);
        tests.Require(spawned, "The published Gate 3 flow admits its live primary boss before a Mario threshold");
        if (spawned)
        {
            const auto* gate = run.pCatalog->Find_KoukuRaidGate("GATE3");
            const auto mechanic = std::find_if(gate->Entries.begin(), gate->Entries.end(), [&](const auto& entry) {
                return entry.strTargetId == marioPattern; });
            auto* boss = room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.g3.saydon", "BOSS_KOUKUSAYDON_G3_SAYDON");
            tests.Require(boss && mechanic != gate->Entries.end(), "The threshold fixture resolves the real Mario entry and actor");
            if (boss && mechanic != gate->Entries.end())
            {
                const auto bossId = boss->iNetEntityId;
                boss->iCurrentHp = static_cast<std::uint32_t>(static_cast<std::uint64_t>(boss->iMaximumHp) * (bars + 1u) / boss->iMaximumHealthBars);
                run.State.iFlowEntryIndex = static_cast<std::uint32_t>(mechanic - gate->Entries.begin() - 1u);
                bool alive = true, entered = false, penaltyCompleted = false, continuedLoop = false, phaseTwoSeen = false;
                std::uint32_t marioDeadline = 0u, previousFlowIndex = run.State.iFlowEntryIndex;
                for (std::uint32_t tick = 190u; tick < 17500u && alive && !continuedLoop; ++tick)
                {
                    room->m_iServerTick = tick; player.iInvulnerableEndTick = tick + 600u; player.iShield = 200000000u;
                    room->Prepare_KoukuAuditionTick(tick);
                    room->Update_KoukuPatternTails(tick);
                    boss = room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.g3.saydon", "BOSS_KOUKUSAYDON_G3_SAYDON");
                    if (!boss) { alive = false; break; }
                    if (tick > 190u) boss->iCurrentHp = static_cast<std::uint32_t>(
                        static_cast<std::uint64_t>(boss->iMaximumHp) * (bars - 1u) / boss->iMaximumHealthBars);
                    if (room->m_KoukuSaydonPatternAudition.Request.strPatternId == marioPattern)
                    {
                        entered = true;
                        if (!marioDeadline && !room->m_KoukuSaydonPatternAudition.Members.empty())
                        {
                            const auto& member = room->m_KoukuSaydonPatternAudition.Members.front();
                            std::string status;
                            const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, marioPattern, status);
                            if (member.iMarioEntryStartTick && pattern) marioDeadline = member.iMarioEntryStartTick +
                                CKoukuSaydonLogicRuntime::Ticks_FromMs(pattern->iTimelineDurationMs);
                        }
                    }
                    room->Update_KoukuSaydonBoss(*boss, tick);
                    room->Commit_KoukuMechanicTriggers(tick);
                    phaseTwoSeen = phaseTwoSeen || (boss->strPatternId == "KOUKUSAYDON_G1_PATTERN_33" && !player.iCurrentHp);
                    room->Update_KoukuWorldBodies(tick);
                    const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(501u);
                    penaltyCompleted = penaltyCompleted || (entered && !player.iCurrentHp && receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                        receipt->second.LastLifecycle && receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED);
                    room->Update_KoukuRaid(tick);
                    continuedLoop = penaltyCompleted && run.bEntryRunning &&
                        run.State.iFlowEntryIndex < previousFlowIndex &&
                        run.State.iFlowEntryIndex > static_cast<std::uint32_t>(mechanic - gate->Entries.begin());
                    previousFlowIndex = run.State.iFlowEntryIndex;
                    alive = run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                        std::any_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
                            [&](const auto& entity) { return entity.iNetEntityId == bossId && entity.iCurrentHp; });
                }
                tests.Require(alive && entered && penaltyCompleted && continuedLoop && phaseTwoSeen && !player.iCurrentHp &&
                    run.State.iFlowEntryIndex > static_cast<std::uint32_t>(mechanic - gate->Entries.begin()) &&
                    run.bEntryRunning && room->m_KoukuSaydonPatternAudition.Request.strPatternId != marioPattern,
                    "Mario 1/2/3/4 timeout bypasses shield/invulnerability, keeps players dead through phase 2 and repeats normal flow");
                if (!alive || !penaltyCompleted || !continuedLoop) std::cout << "[MARIO ENTRY] pattern=" << marioPattern << " deadline=" << marioDeadline << " hp=" << player.iCurrentHp << " reason=" << run.State.strReason << " status=" << room->m_strStatus << '\n';
            }
        }
    }
    const auto verifyBingoFlow = [&](CGameRoom& room, unsigned firstTick, bool verifySpecial) {
        auto& run = room.m_KoukuRaid;
        const auto* gate = run.pCatalog ? run.pCatalog->Find_KoukuRaidGate("BINGO") : nullptr;
        if (!gate || gate->Entries.empty()) { tests.Require(false, "Bingo has its saved independent entries"); return; }
        const auto loop = std::find_if(gate->Entries.begin(), gate->Entries.end(),
            [&](const auto& entry) { return entry.strEntryId == gate->strLoopStartEntryId; });
        const bool savedFlowValid = gate->Entries.size() == 35u && loop != gate->Entries.end();
        tests.Require(savedFlowValid, "Bingo publishes 35 independent entries with its exact stable loop start");
        if (!savedFlowValid) return;
        std::vector<std::string> expected;
        bool independent = true;
        for (const auto& entry : gate->Entries)
        {
            std::string status;
            const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, entry.strTargetId, status);
            independent = independent && !entry.bBundle && pattern && pattern->ParentChildren.empty();
            expected.push_back(entry.strEntryId);
        }
        for (auto entry = loop; entry != gate->Entries.end(); ++entry) expected.push_back(entry->strEntryId);
        const auto laser = [](const auto& entry) { return entry.strTargetId == "KOUKUSAYDON_G1_PATTERN_123"; };
        tests.Require(independent && std::count_if(gate->Entries.begin(), loop, laser) == 1 &&
            std::count_if(loop, gate->Entries.end(), laser) == 1,
            "Bingo normal flow has independent patterns and one laser in both its prefix and repeated tail");
        const auto raidEpoch = run.State.iRunEpoch;
        NET_ENTITY_ID boardOwner = INVALID_NET_ENTITY_ID;
        unsigned boardEpoch = 0u, previousRequest = 0u;
        bool stableOwner = true, exactRequests = true;
        std::vector<std::string> completed;
        unsigned tick = firstTick, failedTick = 0u;
        std::string advanceFailure;
        const auto rejectAdvance = [&](const char* reason) {
            failedTick = tick; advanceFailure = reason; return false; };
        const auto advance = [&](bool normalOnly) {
            room.m_iServerTick = tick;
            room.m_TickDamageEvents.clear();
            // Isolate ordinary attrition without masking the actual 30-second line reward.
            // The special must still emit its exact successful thirteen-bar damage event.
            for (auto& [id, player] : room.m_Players)
            {
                if (!player.iCurrentHp || player.eAction == PLAYER_ACTION_STATE::DEAD) return rejectAdvance("player already dead");
                player.iCurrentHp = player.iMaximumHp = 100000000u;
                if (normalOnly) player.iInvulnerableEndTick = tick + 60000u;
                else if (!player.iInvulnerableEndTick || CKoukuSaydonLogicRuntime::Has_ReachedTick(tick, player.iInvulnerableEndTick))
                    player.iInvulnerableEndTick = tick + 1u;
            }
            if (room.m_KoukuBingoDuration.iOwnerId)
            {
                room.m_KoukuBingoDuration.iNextHammerTick = tick + 60000u;
                if (normalOnly) room.m_KoukuBingoDuration.iNextBombTick = tick + 60000u;
            }
            room.Prepare_KoukuAuditionTick(tick);
            room.Update_KoukuPatternTails(tick);
            room.Update_KoukuBingo(tick);
            auto* boss = room.Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON");
            if (!boss) return rejectAdvance("Bingo boss missing");
            boss->iCurrentHp = boss->iMaximumHp;
            if (!room.Update_KoukuSaydonBoss(*boss, tick)) return rejectAdvance("boss update rejected");
            room.Commit_KoukuMechanicTriggers(tick);
            if (normalOnly && run.bEntryRunning &&
                room.m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE)
            {
                const auto receipt = room.m_KoukuSaydonPatternAuditionReceiptBySessionId.find(run.iOwnerSessionId);
                if (receipt == room.m_KoukuSaydonPatternAuditionReceiptBySessionId.end() || !receipt->second.LastLifecycle ||
                    receipt->second.LastLifecycle->eState != KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED)
                    return rejectAdvance("normal entry has no completed receipt");
                completed.push_back(run.State.strFlowEntryId);
            }
            room.Update_KoukuRaid(tick);
            if (run.State.iRunEpoch != raidEpoch || run.State.strGateId != "BINGO" ||
                run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) return rejectAdvance("raid epoch, gate or phase changed");
            if (!boardOwner && room.m_KoukuBingoDuration.iOwnerId)
            { boardOwner = room.m_KoukuBingoDuration.iOwnerId; boardEpoch = room.m_iKoukuBingoBoardEpoch; }
            stableOwner = stableOwner && boardOwner && room.m_KoukuBingoDuration.iOwnerId == boardOwner &&
                room.m_iKoukuBingoBoardEpoch == boardEpoch && boardEpoch == raidEpoch &&
                room.m_KoukuBingoDuration.bEncounterOwned && !room.m_KoukuBingoDuration.iEndTick;
            if (run.iAuditionRequestSequence != previousRequest)
            {
                exactRequests = exactRequests && run.iAuditionRequestSequence > previousRequest;
                previousRequest = run.iAuditionRequestSequence;
                if (!run.bBingoSpecialRunning)
                    exactRequests = exactRequests && run.State.iFlowEntryIndex < gate->Entries.size() &&
                        run.State.strFlowEntryId == gate->Entries[run.State.iFlowEntryIndex].strEntryId &&
                        room.m_KoukuSaydonPatternAudition.Request.strPatternId == gate->Entries[run.State.iFlowEntryIndex].strTargetId;
            }
            // Fake connections have no send worker; drain their frames like an acknowledging client.
            for (auto& [id, weakSession] : room.m_Sessions)
                if (const auto session = weakSession.lock())
                { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u; }
            if (!room.Is_Ready() || !std::all_of(room.m_Players.begin(), room.m_Players.end(),
                [](const auto& pair) { return pair.second.iCurrentHp && pair.second.eAction != PLAYER_ACTION_STATE::DEAD; }))
                return rejectAdvance("room failed or a player died during this tick");
            return true;
        };
        bool advanced = true;
        for (; tick < firstTick + 24000u && completed.size() < expected.size() && advanced; ++tick)
            advanced = advance(true);
        tests.Require(advanced && completed == expected && stableOwner && exactRequests,
            "Actual Bingo flow completes its saved independent prefix and one full repeated tail while retaining raid and board ownership");
        if (!advanced || completed != expected || !stableOwner || !exactRequests)
            std::cout << "[RAID BINGO FLOW] completed=" << completed.size() << "/" << expected.size()
                << " entry=" << run.State.strFlowEntryId << " phase=" << unsigned(run.State.ePhase)
                << " failedTick=" << failedTick << " cause=" << advanceFailure
                << " reason=" << run.State.strReason << " status=" << room.m_strStatus << '\n';
        if (!verifySpecial || !advanced || completed != expected) return;

        std::string status;
        const auto* special = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog,
            gate->strBingoSpecialPatternId, status);
        // This authored Parent uses playChildrenSequentially=false. Publication flattens
        // its children into ten stages sharing one clock; runtime ParentChildren is empty.
        const std::array<unsigned, 10u> expectedDurations{5400u, 867u, 2000u, 2167u, 1394u,
            13000u, 200u, 1167u, 5100u, 1333u};
        bool flattened = special && special->ParentChildren.empty() && special->bFixedTimelineClock &&
            special->Stages.size() == expectedDurations.size();
        unsigned stageDurationMs = 0u;
        if (flattened)
            for (unsigned i = 0u; i < expectedDurations.size(); ++i)
            {
                stageDurationMs += special->Stages[i].iDurationMs;
                flattened = flattened && special->Stages[i].iDurationMs == expectedDurations[i];
            }
        // An absent optional tail duration uses the stage sum in the published ABI.
        flattened = flattened && (std::max)(stageDurationMs, special->iTimelineDurationMs) == 32628u;
        const auto detonates = [](const auto& trigger) {
            return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION; };
        flattened = flattened && std::count_if(special->MechanicTriggers.begin(), special->MechanicTriggers.end(), detonates) == 1 &&
            std::any_of(special->MechanicTriggers.begin(), special->MechanicTriggers.end(), [&](const auto& trigger) {
                return detonates(trigger) && trigger.iStartMs == 24828u && trigger.iDurationMs == 34u; }) &&
            std::any_of(special->MechanicTriggers.begin(), special->MechanicTriggers.end(), [](const auto& trigger) {
                return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER && !trigger.iStartMs; }) &&
            std::any_of(special->LogicWindows.begin(), special->LogicWindows.end(), [](const auto& window) {
                return window.eKind == BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS && window.iStartMs == 8567u && window.iDurationMs == 3261u && window.bGazeDuringWindow; }) &&
            std::any_of(special->LogicWindows.begin(), special->LogicWindows.end(), [](const auto& window) {
                return window.eKind == BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES && !window.iStartMs &&
                    window.iDurationMs == 32628u && window.iThreshold == 1u &&
                    std::any_of(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
                        return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY && result.iDurationMs == 30000u; }); });
        tests.Require(flattened,
            "The explicit Bingo special publishes flattened preparation/Medusa/13s hold stages, detonation at 24828ms and a 32628ms line window");
        if (!flattened) return;
        // Exercise rejected special admission while the next normal pattern is active.
        // Only the request's expected revision is invalid; every live owner must survive.
        advanced = advance(false); ++tick;
        tests.Require(advanced && room.m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE,
            "The special preflight fixture retains an active normal Bingo entry");
        if (!advanced) return;
        const auto liveBefore = room.m_WorldEntities;
        const auto boardWhite = room.m_KoukuBingo.Get_WhiteMask(), boardRed = room.m_KoukuBingo.Get_RedMask();
        const auto durationBefore = room.m_KoukuBingoDuration;
        const auto auditionEpochBefore = run.iAuditionEpoch, requestBefore = run.iAuditionRequestSequence;
        const auto flowIndexBefore = run.State.iFlowEntryIndex;
        const auto flowEntryBefore = run.State.strFlowEntryId;
        const auto normalPatternBefore = room.m_KoukuSaydonPatternAudition.Request.strPatternId;
        const auto nextEpochBefore = room.m_iNextKoukuSaydonPatternAuditionEpoch;
        const auto receiptBefore = room.m_KoukuSaydonPatternAuditionReceiptBySessionId.at(run.iOwnerSessionId);
        const auto sourceBefore = run.State.iActionSourceRevision;
        ++run.State.iActionSourceRevision;
        const bool rejected = !room.Start_KoukuBingoSpecialPattern(tick);
        run.State.iActionSourceRevision = sourceBefore;
        const bool livePreserved = room.m_WorldEntities.size() == liveBefore.size() &&
            std::all_of(liveBefore.begin(), liveBefore.end(), [&](const auto& old) {
                const auto current = std::find_if(room.m_WorldEntities.begin(), room.m_WorldEntities.end(),
                    [&](const auto& entity) { return entity.iNetEntityId == old.iNetEntityId; });
                return current != room.m_WorldEntities.end() && current->strPatternId == old.strPatternId &&
                    current->iPatternSequence == old.iPatternSequence && current->iPatternStageIndex == old.iPatternStageIndex &&
                    current->iPatternStartTick == old.iPatternStartTick && current->iActionStartTick == old.iActionStartTick &&
                    current->iCurrentHp == old.iCurrentHp && current->eAction == old.eAction &&
                    current->fPositionX == old.fPositionX && current->fPositionY == old.fPositionY &&
                    current->fPositionZ == old.fPositionZ && current->fYawDegrees == old.fYawDegrees;
            });
        const auto& receiptAfter = room.m_KoukuSaydonPatternAuditionReceiptBySessionId.at(run.iOwnerSessionId);
        const bool lifecyclePreserved = receiptAfter.LastLifecycle.has_value() == receiptBefore.LastLifecycle.has_value() &&
            (!receiptBefore.LastLifecycle || (receiptAfter.LastLifecycle->iRoomAuditionEpoch == receiptBefore.LastLifecycle->iRoomAuditionEpoch &&
                receiptAfter.LastLifecycle->iPatternSequence == receiptBefore.LastLifecycle->iPatternSequence &&
                receiptAfter.LastLifecycle->eState == receiptBefore.LastLifecycle->eState));
        tests.Require(rejected && livePreserved && !run.bBingoSpecialRunning && run.bEntryRunning &&
            run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.State.iRunEpoch == raidEpoch &&
            run.iAuditionEpoch == auditionEpochBefore && run.iAuditionRequestSequence == requestBefore &&
            run.State.iFlowEntryIndex == flowIndexBefore && run.State.strFlowEntryId == flowEntryBefore &&
            room.m_KoukuSaydonPatternAudition.Request.strPatternId == normalPatternBefore && lifecyclePreserved &&
            room.m_iNextKoukuSaydonPatternAuditionEpoch == nextEpochBefore &&
            receiptAfter.Request.iRequestSequence == receiptBefore.Request.iRequestSequence &&
            receiptAfter.Result.eResult == receiptBefore.Result.eResult &&
            room.m_KoukuBingo.Get_WhiteMask() == boardWhite && room.m_KoukuBingo.Get_RedMask() == boardRed &&
            room.m_iKoukuBingoBoardEpoch == boardEpoch &&
            room.m_KoukuBingoDuration.iOwnerId == durationBefore.iOwnerId &&
            room.m_KoukuBingoDuration.iNextBombTick == durationBefore.iNextBombTick &&
            room.m_KoukuBingoDuration.iMarkedBombCount == durationBefore.iMarkedBombCount,
            "Rejected special preflight preserves every live actor, normal occurrence, receipt and encounter-owned board without consuming an epoch");
        // Use the real 30s first mark / 20s repeat clock twice. Complete one
        // row for the third bomb, then one different column for the sixth.
        // Earlier blasts cannot reward incomplete crosses; used red lines stay on the board.
        room.m_KoukuBingo.Reset();
        const std::uint32_t firstLines = Kouku_BingoLineMask(1);
        const std::uint32_t nextLines = Kouku_BingoLineMask(KOUKU_BINGO_SIDE + 2);
        for (auto& [id, player] : room.m_Players)
        {
            player.iInvulnerableEndTick = 0u;
            player.fPositionX = Kouku_BingoCellCenterX(7);
            player.fPositionZ = Kouku_BingoCellCenterZ(7);
        }
        room.m_KoukuBingoDuration.iNextBombTick = tick + CKoukuSaydonLogicRuntime::Ticks_FromMs(
            KOUKU_BINGO_BOMB_INITIAL_DELAY_MS + KOUKU_BINGO_BOMB_INTERVAL_MS);
        const auto firstMarkCount = room.m_KoukuBingoDuration.iMarkedBombCount;
        unsigned starts = 0u, resumes = 0u, interruptedIndex = 0u, lineRewards = 0u;
        unsigned expectedBombTick = room.m_KoukuBingoDuration.iNextBombTick;
        bool exactBombClock = true, exactLineRewards = true, usedLinesRetained = false;
        std::string interruptedEntry;
        bool waitingForResume = false, specialValid = firstMarkCount == 0u;
        std::string specialFailure = specialValid ? "" : "nonzero initial mark count";
        const auto noteSpecialFailure = [&](const char* reason) {
            if (!specialValid && specialFailure.empty()) { specialFailure = reason; failedTick = tick; } };
        std::vector<unsigned> observedStages;
        unsigned specialStartTick = 0u, detonations = 0u;
        const auto specialDeadline = tick + 6000u;
        for (; tick < specialDeadline && resumes < 2u && advanced; ++tick)
        {
            const bool wasSpecial = run.bBingoSpecialRunning;
            const auto beforeEntry = run.State.strFlowEntryId;
            const auto beforeIndex = run.State.iFlowEntryIndex;
            const auto previousMarks = room.m_KoukuBingoDuration.iMarkedBombCount;
            const auto unusedBefore = room.m_KoukuBingo.Count_UnconsumedRowsAndColumns();
            const auto redBefore = room.m_KoukuBingo.Get_RedMask();
            advanced = advance(false);
            const auto marks = room.m_KoukuBingoDuration.iMarkedBombCount;
            if (marks != previousMarks)
            {
                exactBombClock = exactBombClock && marks == previousMarks + 1u && tick == expectedBombTick;
                expectedBombTick = tick + CKoukuSaydonLogicRuntime::Ticks_FromMs(KOUKU_BINGO_BOMB_INTERVAL_MS);
            }
            exactBombClock = exactBombClock && room.m_KoukuBingoDuration.iNextBombTick == expectedBombTick;
            const auto unusedAfter = room.m_KoukuBingo.Count_UnconsumedRowsAndColumns();
            if (unusedAfter < unusedBefore)
            {
                ++lineRewards;
                exactLineRewards = exactLineRewards && lineRewards <= 2u && unusedBefore == 1u && unusedAfter == 0u &&
                    starts == lineRewards && marks == lineRewards * 3u && redBefore == room.m_KoukuBingo.Get_RedMask() &&
                    room.m_KoukuBingo.Count_CompletedRowsAndColumns() == lineRewards &&
                    std::all_of(room.m_Players.begin(), room.m_Players.end(), [&](const auto& pair) {
                        return pair.second.iInvulnerableEndTick == tick + CKoukuSaydonLogicRuntime::Ticks_FromMs(30000u); });
                if (lineRewards == 1u)
                {
                    room.m_KoukuBingo.Fill(firstLines);
                    usedLinesRetained = room.m_KoukuBingo.Get_RedMask() == firstLines &&
                        room.m_KoukuBingo.Count_CompletedRowsAndColumns() == 1u &&
                        room.m_KoukuBingo.Count_UnconsumedRowsAndColumns() == 0u;
                }
            }
            if (lineRewards == 1u && starts == 1u)
                exactLineRewards = exactLineRewards && unusedAfter == 0u;
            if (!wasSpecial && run.bBingoSpecialRunning)
            {
                ++starts;
                specialValid = specialValid && !waitingForResume && room.m_KoukuBingoDuration.iMarkedBombCount == starts * 3u &&
                    run.State.iFlowEntryIndex == beforeIndex && run.State.strFlowEntryId == beforeEntry &&
                    room.m_KoukuSaydonPatternAudition.Request.strPatternId == special->strPatternId;
                noteSpecialFailure("third-mark insertion changed the cursor or selected the wrong request");
                interruptedIndex = beforeIndex; interruptedEntry = beforeEntry;
                exactLineRewards = exactLineRewards && lineRewards + 1u == starts &&
                    room.m_KoukuBingo.Count_UnconsumedRowsAndColumns() == 0u;
                room.m_KoukuBingo.Fill(starts == 1u ? firstLines : nextLines);
                exactLineRewards = exactLineRewards && room.m_KoukuBingo.Count_UnconsumedRowsAndColumns() == 1u &&
                    room.m_KoukuBingo.Get_RedMask() == (starts == 1u ? firstLines : firstLines | nextLines);
                observedStages.clear(); specialStartTick = 0u; detonations = 0u;
            }
            else if (wasSpecial && !run.bBingoSpecialRunning)
            {
                specialValid = specialValid && specialStartTick && observedStages.size() == expectedDurations.size() &&
                    detonations == 1u && tick - specialStartTick == CKoukuSaydonLogicRuntime::Ticks_FromMs(32628u) &&
                    run.State.iFlowEntryIndex == interruptedIndex && run.State.strFlowEntryId == interruptedEntry;
                noteSpecialFailure("special completion duration, stage count, detonation count or cursor mismatch");
                waitingForResume = true;
            }
            if (run.bBingoSpecialRunning)
            {
                const auto* boss = room.Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON");
                if (boss && boss->strPatternId == special->strPatternId)
                {
                    if (!specialStartTick) specialStartTick = boss->iPatternStartTick;
                    const auto elapsed = tick - specialStartTick;
                    unsigned selected = 0u, startMs = 0u;
                    while (selected < expectedDurations.size() &&
                        elapsed >= CKoukuSaydonLogicRuntime::Ticks_FromMs(startMs + expectedDurations[selected]))
                        startMs += expectedDurations[selected++];
                    specialValid = specialValid && selected < expectedDurations.size() &&
                        boss->iPatternStartTick == specialStartTick && boss->iPatternStageIndex == selected &&
                        boss->iActionStartTick == specialStartTick + CKoukuSaydonLogicRuntime::Ticks_FromMs(startMs);
                    noteSpecialFailure("flattened stage index or action clock mismatch");
                    if (observedStages.empty() || observedStages.back() != selected)
                    {
                        specialValid = specialValid && selected == observedStages.size() &&
                            elapsed == CKoukuSaydonLogicRuntime::Ticks_FromMs(startMs);
                        noteSpecialFailure("flattened stage boundary order or tick mismatch");
                        observedStages.push_back(selected);
                    }
                    const auto damage = std::count_if(room.m_TickDamageEvents.begin(), room.m_TickDamageEvents.end(), [&](const auto& event) {
                        return boss->iMaximumHealthBars && event.isOutgoing && event.iTargetNetEntityId == boss->iNetEntityId &&
                            event.iAmount == std::uint64_t(boss->iMaximumHp) * 13u / boss->iMaximumHealthBars; });
                    const bool explosionTick = elapsed == CKoukuSaydonLogicRuntime::Ticks_FromMs(24828u);
                    specialValid = specialValid && damage == (explosionTick ? 1 : 0);
                    noteSpecialFailure("detonation tick or thirteen-bar damage count mismatch");
                    detonations += static_cast<unsigned>(damage);
                }
            }
            if (waitingForResume && !run.bBingoSpecialRunning && run.bEntryRunning)
            {
                specialValid = specialValid && run.State.iFlowEntryIndex == interruptedIndex &&
                    run.State.strFlowEntryId == interruptedEntry &&
                    room.m_KoukuSaydonPatternAudition.Request.strPatternId == gate->Entries[interruptedIndex].strTargetId;
                noteSpecialFailure("normal flow did not resume its interrupted stable entry");
                ++resumes; waitingForResume = false;
            }
        }
        tests.Require(exactLineRewards && lineRewards == 2u && usedLinesRetained &&
            room.m_KoukuBingo.Count_CompletedRowsAndColumns() == 2u && room.m_KoukuBingo.Count_UnconsumedRowsAndColumns() == 0u,
            "Two real special cycles consume one new row and one new column exactly once while retaining both red lines and granting exact thirty-second rewards");
        tests.Require(exactBombClock,
            "Special insertion, line rewards and normal-entry replay preserve every thirty/twenty-second bomb deadline");
        tests.Require(advanced && specialValid && starts == 2u && resumes == 2u && stableOwner && exactRequests,
            "Every third real Bingo mark executes the flattened special on exact stage/detonation clocks and resumes the interrupted stable entry without resetting the raid or board");
        if (!advanced || !specialValid || starts != 2u || resumes != 2u || !stableOwner || !exactRequests)
            std::cout << "[RAID BINGO SPECIAL] marks=" << room.m_KoukuBingoDuration.iMarkedBombCount
                << " starts=" << starts << " resumes=" << resumes << " advanced=" << advanced << " valid=" << specialValid
                << " stableOwner=" << stableOwner << " exactRequests=" << exactRequests << " phase=" << unsigned(run.State.ePhase)
                << " tick=" << tick << " failedTick=" << failedTick << " specialStart=" << specialStartTick
                << " stages=" << observedStages.size() << " detonations=" << detonations
                << " rewards=" << lineRewards << " exactRewards=" << exactLineRewards << " exactBombClock=" << exactBombClock
                << " advanceCause=" << advanceFailure << " specialCause=" << specialFailure
                << " reason=" << run.State.strReason << " status=" << room.m_strStatus << '\n';
    };
    Run_KoukuGate3Entry(tests);
    CGameplayCatalog catalog;
    tests.Require(catalog.Load(), "Raid integration loads the published candidate catalog");
    // Use the actual Release trigger and gate-vote consumers, not a Debug START shortcut.
    for (unsigned participantCount = 2u; participantCount <= 4u; ++participantCount)
    for (unsigned scenario = 0u; scenario < 2u; ++scenario)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Party product admission loads the published world and raid");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= participantCount; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = id == 1u ? 27.8f : 3.29f;
            player.fPositionY = id == 1u ? .45f : 8.64f;
            player.fPositionZ = id == 1u ? -67.7f : -10.69f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            room->m_PartyIdByPlayerId[id] = 77u; room->m_PartyMembersByPartyId[77u].push_back(id);
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        auto& run = room->m_KoukuRaid;
        if (participantCount == 2u && scenario == 0u)
        {
            const auto* publishedGate = catalog.Find_KoukuRaidGate("GATE1");
            tests.Require(publishedGate != nullptr, "Revision rejection fixture uses the actual published G1 raid");
            if (publishedGate)
            {
                const auto source = CKoukuSaydonBrain::Resolve_ProductSourceRevision(catalog);
                C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST stale;
                stale.iRequestSequence = 1u; stale.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
                stale.eOperation = KOUKUSAYDON_RAID_OPERATION::START; stale.strStartGateId = "GATE1";
                stale.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
                stale.iActionSourceRevision = source + 1u; stale.iSequenceSourceRevision = publishedGate->iSequenceRevision;
                const auto nextEpoch = room->m_iNextKoukuRaidEpoch;
                const auto priorProduct = room->m_pKoukuPublishedProductGeneration;
                const auto priorBossCount = room->m_WorldEntities.size();
                std::string reason;
                tests.Require(!room->Begin_KoukuRaidPreparation(501u, stale, reason) &&
                    reason.find("Action revision mismatch") != std::string::npos &&
                    reason.find("requested " + std::to_string(source + 1u)) != std::string::npos &&
                    reason.find("published " + std::to_string(source)) != std::string::npos,
                    "Raid rejects an unpublished Action revision with its exact requested and published values");
                stale.iActionSourceRevision = source; ++stale.iSequenceSourceRevision;
                tests.Require(!room->Begin_KoukuRaidPreparation(501u, stale, reason) &&
                    reason.find("Sequence revision mismatch") != std::string::npos &&
                    reason.find("requested " + std::to_string(stale.iSequenceSourceRevision)) != std::string::npos &&
                    reason.find("published " + std::to_string(publishedGate->iSequenceRevision)) != std::string::npos,
                    "Raid rejects an unpublished Sequence revision with its exact requested and published values");
                tests.Require(!run.State.iRunEpoch && room->m_iNextKoukuRaidEpoch == nextEpoch &&
                    room->m_pKoukuPublishedProductGeneration == priorProduct && room->m_WorldEntities.size() == priorBossCount &&
                    room->m_Players.at(1u).fPositionX == 27.8f && room->m_Players.at(1u).iCurrentHp == 100u,
                    "Revision rejection preserves the party, bosses, catalog and next epoch for the following exact-source retry");
            }
        }
        if (scenario == 0u)
        {
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
            std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
            const auto enter = [&](unsigned tick) {
                room->m_iServerTick = tick;
                room->m_ServerTriggerSystem.Evaluate_Entries(room->m_Players, tick, transfers,
                    [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& target) {
                        return room->Activate_TriggerTarget(kind, target);
                    }, prompts);
            };
            room->m_Players.at(participantCount).TriggerMove.isActive = true;
            const auto unusedEpoch = room->m_iNextKoukuRaidEpoch;
            enter(100u);
            tests.Require(room->m_iNextKoukuRaidEpoch == unusedEpoch, "Temporary contact rejection preserves the next raid epoch");
            tests.Require(!run.State.iRunEpoch && room->m_strStatus == "All room participants must be alive and outside a transfer",
                "A companion still in a G transfer temporarily rejects the actual G1 entry trigger");
            room->m_Players.at(participantCount).TriggerMove.isActive = false;
            enter(101u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.PlayerIds.size() == participantCount &&
                run.strEntryTriggerSequenceId == "world.sequence.instance.circusfinale",
                "G1 retries the same contact after the companion lands without leaving and re-entering");
            const auto epoch = run.State.iRunEpoch;
            enter(102u);
            tests.Require(run.State.iRunEpoch == epoch, "A successful entry contact cannot start a second raid epoch");
        }
        else
        {
            room->m_GateProgress.iCurrentGate = 3u; room->m_GateProgress.iClearedMask = 7u;
            C2S_GATE_PROGRESS_PROPOSE proposal; proposal.iRequestSequence = 1u;
            proposal.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; proposal.eKind = GATE_PROGRESS_KIND::ADVANCE;
            room->Handle_GateProgressPropose(501u, proposal);
            tests.Require(run.State.iRunEpoch == 0u && room->m_GateProgress.Voters.size() == participantCount,
                "The legacy Bingo UI waits for all party voters before resource admission");
            C2S_GATE_PROGRESS_RESPOND response; response.iRequestSequence = 1u;
            response.iProposalId = room->m_GateProgress.iProposalId; response.bAccepted = true;
            for (unsigned id = 2u; id <= participantCount; ++id)
            {
                room->Handle_GateProgressRespond(500u + id, response);
                if (id < participantCount) tests.Require(run.State.iRunEpoch == 0u,
                    "Partial Release UI approval cannot admit Bingo");
            }
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bGateVoteEntry &&
                run.State.strGateId == "BINGO" && room->m_GateProgress.iCurrentGate == 3u &&
                !room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON"),
                "Release Bingo UI admits the saved intro/Flow owner instead of an idle spawned boss");
        }
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING) continue;
        auto ack = run.Request; ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        ack.iExpectedRunEpoch = run.State.iRunEpoch; ack.iRequestSequence += 1u;
        std::string reason;
        tests.Require(room->Apply_KoukuRaidReadiness(501u, ack, reason), "First participant pins the current Action/Sequence product");
        room->m_iServerTick = 103u; room->Update_KoukuRaid(103u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.State.iReadyMask == 1u,
            "One ready client cannot start the party cinematic");
        for (unsigned id = 2u; id <= participantCount; ++id)
        {
            tests.Require(room->Apply_KoukuRaidReadiness(500u + id, ack, reason), "Each participant pins the same Action/Sequence product");
            if (id < participantCount)
            {
                room->Update_KoukuRaid(103u);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                    "The complete two-to-four-client ready mask is required before a cinematic");
            }
        }
        room->m_iServerTick = 104u; room->Update_KoukuRaid(104u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.iReadyMask == (1u << participantCount) - 1u &&
            !run.State.strSequencePatternId.empty(), "All Release clients enter one authoritative cinematic after READY");
        if (scenario == 1u)
        {
            room->m_iServerTick = run.State.iEndTick; room->Update_KoukuRaid(room->m_iServerTick);
            finishEntryIdle(*room);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning &&
                run.State.strGateId == "BINGO" && run.iAuditionEpoch != 0u,
                "Bingo UI intro completion queues its first published independent entry after the idle delay");
            const auto common = room->m_KoukuSaydonPatternAudition.iCommonStartTick;
            room->m_iServerTick = common; room->Prepare_KoukuAuditionTick(common);
            const auto* boss = room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON");
            tests.Require(boss && !boss->strPatternId.empty() &&
                room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE,
                "The real Bingo scheduler starts its first independent pattern at the common Server tick in Release");
            if (participantCount == 4u)
                verifyBingoFlow(*room, common, true);

        }
    }

    // A standalone boss clear/restart must enter the same pinned Sequence owner as the entry collider.
    // Resource preparation is transactional: neither the unanimous vote nor partial READY moves a player.
    for (unsigned participantCount = 2u; participantCount <= 4u; ++participantCount)
    for (unsigned transition = 0u; transition < 5u; ++transition)
    for (const bool failPreparation : { false, true })
    {
        const bool advance = transition < 2u;
        const std::uint8_t sourceGate = static_cast<std::uint8_t>(advance ? transition + 1u : transition - 1u);
        const std::string sourceGateId = "GATE" + std::to_string(sourceGate);
        const std::string targetGateId = "GATE" + std::to_string(advance ? sourceGate + 1u : sourceGate);
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Standalone gate transition loads the actual published world and raid");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= participantCount; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = 27.8f + .2f * id; player.fPositionY = .45f; player.fPositionZ = -67.7f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            room->m_PartyIdByPlayerId[id] = 77u; room->m_PartyMembersByPartyId[77u].push_back(id);
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        const auto* source = catalog.Find_KoukuRaidGate(sourceGateId);
        const auto* target = catalog.Find_KoukuRaidGate(targetGateId);
        tests.Require(source && target && !target->strIntroPatternId.empty(),
            "Standalone gate transition resolves its authored source boss and destination intro");
        if (!source || !target) continue;
        tests.Require(room->Spawn_GatePlacement(source->strPrimaryBossPlacementId),
            "Standalone transition begins with the actual previous gate boss");
        auto boss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [&](const auto& entity) {
            return entity.strPlacementId == source->strPrimaryBossPlacementId;
        });
        if (boss == room->m_WorldEntities.end()) continue;
        // The public Debug spawn path commits the actor, then records its gate.
        room->Note_GatePlacementRaised(source->strPrimaryBossPlacementId);
        tests.Require(room->m_GateProgress.iCurrentGate == sourceGate &&
            !(room->m_GateProgress.iClearedMask & (1u << (sourceGate - 1u))),
            "Standalone source activation records its actual gate with no stale clear bit");
        if (advance)
        {
            boss->iCurrentHp = 0u; boss->eAction = SERVER_ENTITY_ACTION::DEAD;
            const auto dead = *boss;
            room->Notify_KoukuRaidBossDeath(dead, 100u); room->Notify_GateBossDeath(dead);
            tests.Require(room->m_GateProgress.iCurrentGate == sourceGate &&
                (room->m_GateProgress.iClearedMask & (1u << (sourceGate - 1u))) && !room->Is_KoukuRaidRunning(),
                "Standalone G1/G2 defeat records the clear before the next-gate proposal");
        }
        else
        {
            auto& deadPlayer = room->m_Players.at(participantCount);
            deadPlayer.iCurrentHp = 0u; deadPlayer.eAction = PLAYER_ACTION_STATE::DEAD;
        }
        // An in-progress standalone G3 restart keeps its Mario cursor until every client is ready.
        if (!advance && sourceGate == 3u) room->m_iNextMarioEntryStage = 3u;
        const auto previousMarioStage = room->m_iNextMarioEntryStage;
        const auto previousPlayers = room->m_Players;
        const auto previousEntities = room->m_WorldEntities;
        const auto previousGate = room->m_GateProgress.iCurrentGate;
        const auto previousMask = room->m_GateProgress.iClearedMask;
        const auto previousNextId = room->m_iNextNetEntityId;
        const auto preserved = [&]() {
            if (room->m_GateProgress.iCurrentGate != previousGate || room->m_GateProgress.iClearedMask != previousMask ||
                room->m_iNextMarioEntryStage != previousMarioStage ||
                room->m_iNextNetEntityId != previousNextId || room->m_WorldEntities.size() != previousEntities.size() ||
                room->m_Players.size() != previousPlayers.size()) return false;
            for (const auto& [id, old] : previousPlayers)
            {
                const auto found = room->m_Players.find(id);
                if (found == room->m_Players.end()) return false;
                const auto& player = found->second;
                if (player.fPositionX != old.fPositionX || player.fPositionY != old.fPositionY || player.fPositionZ != old.fPositionZ ||
                    player.iCurrentHp != old.iCurrentHp || player.eAction != old.eAction) return false;
            }
            for (std::size_t i = 0; i < previousEntities.size(); ++i)
            {
                const auto& old = previousEntities[i]; const auto& entity = room->m_WorldEntities[i];
                if (entity.iNetEntityId != old.iNetEntityId || entity.strPlacementId != old.strPlacementId ||
                    entity.iCurrentHp != old.iCurrentHp || entity.eAction != old.eAction ||
                    entity.fPositionX != old.fPositionX || entity.fPositionY != old.fPositionY || entity.fPositionZ != old.fPositionZ) return false;
            }
            return true;
        };
        room->m_iServerTick = 100u;
        C2S_GATE_PROGRESS_PROPOSE proposal; proposal.iRequestSequence = 1u;
        proposal.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
        proposal.eKind = advance ? GATE_PROGRESS_KIND::ADVANCE : GATE_PROGRESS_KIND::RESTART;
        room->Handle_GateProgressPropose(501u, proposal);
        auto& run = room->m_KoukuRaid;
        tests.Require(run.State.iRunEpoch == 0u && room->m_GateProgress.Voters.size() == participantCount && preserved(),
            "A standalone gate proposal preserves the complete party and boss until unanimous approval");
        C2S_GATE_PROGRESS_RESPOND response; response.iRequestSequence = 1u;
        response.iProposalId = room->m_GateProgress.iProposalId; response.bAccepted = true;
        for (unsigned id = 2u; id <= participantCount; ++id)
        {
            room->Handle_GateProgressRespond(500u + id, response);
            if (id < participantCount) tests.Require(run.State.iRunEpoch == 0u && preserved(),
                "Partial standalone gate approval neither moves players nor creates or removes bosses");
        }
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bGateVoteEntry &&
            run.State.strGateId == targetGateId && preserved(),
            "Unanimous standalone advance or restart prepares the destination Sequence while preserving the old world");
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING) continue;
        auto ack = run.Request; ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        ack.iExpectedRunEpoch = run.State.iRunEpoch; ++ack.iRequestSequence;
        std::string reason;
        for (unsigned id = 1u; id < participantCount; ++id)
        {
            tests.Require(room->Apply_KoukuRaidReadiness(500u + id, ack, reason),
                "Standalone gate participant acknowledges its exact published resource pin");
            room->m_iServerTick = 101u; room->Update_KoukuRaid(101u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && preserved(),
                "Partial standalone gate READY preserves positions, HP, dead members and exact boss identities");
        }
        if (failPreparation)
        {
            ack.eOperation = KOUKUSAYDON_RAID_OPERATION::FAILED;
            ack.strReason = "contract: destination preparation failed";
            tests.Require(room->Apply_KoukuRaidReadiness(500u + participantCount, ack, reason) &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::ABORTED && preserved() &&
                run.State.strReason.find(ack.strReason) != std::string::npos,
                "A failed destination client preserves the previous gate, clear mask, player state and boss scene");
            room->m_iServerTick = 102u; room->Update_KoukuRaid(102u);
            tests.Require(preserved(), "Aborted standalone preparation does not perform deferred teleport or spawn");
            continue;
        }
        tests.Require(room->Apply_KoukuRaidReadiness(500u + participantCount, ack, reason) && preserved(),
            "Final READY only records readiness until the authoritative transition tick");
        room->m_iServerTick = 102u; room->Update_KoukuRaid(102u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == targetGateId &&
            run.State.strSequencePatternId == target->strIntroPatternId && !run.bEntryRunning &&
            run.State.iReadyMask == (1u << participantCount) - 1u,
            "Standalone G1/G2 advance and G1/G2/G3 restart begin the published intro before combat");
        if (!advance && sourceGate == 3u)
            tests.Require(room->m_iNextMarioEntryStage == 1u,
                "G3 restart resets its prior Mario cursor only after all READY commit the cinematic");
        for (const auto& [id, old] : previousPlayers)
        {
            const auto& player = room->m_Players.at(id);
            tests.Require(player.fPositionX == old.fPositionX && player.fPositionY == old.fPositionY && player.fPositionZ == old.fPositionZ,
                "The first cinematic tick cannot use the legacy direct combat teleport");
        }
    }

    for (unsigned count = 1; count <= 4; ++count)
    {
        BOSS_PATTERN_DEFINITION pattern;
        pattern.strPatternId = "dice.contract";
        BOSS_PATTERN_LOGIC_WINDOW window;
        window.strWindowId = "dice.bind";
        window.eKind = BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND;
        window.iStartMs = 1000u; window.iDurationMs = 2000u;
        pattern.LogicWindows.push_back(window);
        SERVER_WORLD_ENTITY boss;
        boss.iNetEntityId = 90u; boss.iPatternSequence = 7u;
        boss.strPatternId = pattern.strPatternId; boss.iCurrentHp = 100u;
        std::map<PLAYER_ID, SERVER_PLAYER> players;
        for (unsigned id = 1; id <= count; ++id)
        {
            auto& player = players[id]; player.iPlayerId = id;
            player.iNetEntityId = id + 100u; player.iCurrentHp = player.iMaximumHp = 100u;
            player.isCombatReady = true;
        }
        KOUKUSAYDON_LOGIC_LEDGER ledger;
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
        std::vector<DAMAGE_EVENT> damage;
        KOUKUSAYDON_LOGIC_OUTPUT output;
        const auto update = [&](unsigned tick) { CKoukuSaydonLogicRuntime::Update(boss, pattern,
            ledger, players, catalog, nullptr, tick, damage, output); };
        const auto bound = [&]() { return std::count_if(players.begin(), players.end(),
            [](const auto& row) { return row.second.bPatternBound; }); };
        update(129u);
        tests.Require(bound() == 0, "Dice bind does not begin before its authored window");
        update(130u);
        tests.Require(bound() == count - 1u && ledger.Windows.front().iFreePlayerNetEntityId != INVALID_NET_ENTITY_ID,
            "Dice selects exactly one free player for one to four participants");
        const auto target = boss.iTargetEntityId;
        update(170u);
        tests.Require(boss.iTargetEntityId == target && bound() == count - 1u,
            "Dice keeps the same free target for the complete bind lifetime");
        update(190u);
        tests.Require(bound() == 0, "Dice releases every owned bind at the authored end tick");
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 200u, ledger);
        update(230u);
        auto& foreign = players.begin()->second;
        foreign.bPatternBound = true; foreign.iPatternBindOwnerNetEntityId = 999u;
        CKoukuSaydonLogicRuntime::Discard(ledger, players, &boss);
        tests.Require(bound() == 1 && foreign.iPatternBindOwnerNetEntityId == 999u,
            "Dice cancellation releases its own binds and preserves a foreign owner");
    }
    // Exercise the production phase-2 placement and return-pin helper against the published cage anchor.
    for (unsigned count = 1u; count <= 4u; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Legacy G3 clear fixture loads the published raid admission");
        if (!room->Is_Ready()) continue;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1u; id <= count; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u;
            player.strNickName = "Encore-" + std::to_string(id);
            player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.isCombatReady = true;
            player.fPositionX = -2.45f; player.fPositionY = 1.32f; player.fPositionZ = 945.17f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        tests.Require(room->Spawn_GatePlacement("boss.koukusaydon.g3.saydon"),
            "F1 and legacy in-game Gate 3 use the authored primary boss placement");
        auto boss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& e) {
            return e.strPlacementId == "boss.koukusaydon.g3.saydon"; });
        if (boss == room->m_WorldEntities.end()) continue;
        auto dead = *boss; dead.iCurrentHp = 0u; dead.eAction = SERVER_ENTITY_ACTION::DEAD;
        room->m_iServerTick = 100u;
        // A prior manual vote and a dead member must not bypass or block the clear's resource admission.
        room->m_GateProgress.iProposalId = 999u; room->m_GateProgress.Voters = {1u};
        if (count > 1u) { room->m_Players.at(count).iCurrentHp = 0u; room->m_Players.at(count).eAction = PLAYER_ACTION_STATE::DEAD; }
        room->Notify_KoukuRaidBossDeath(dead, 100u); room->Notify_GateBossDeath(dead);
        auto& run = room->m_KoukuRaid;
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bClearedBossPreparation &&
            room->Is_KoukuRaidInputBlocked() && run.iGate3ClearTick == 100u && !room->m_GateProgress.iProposalId &&
            (room->m_GateProgress.iClearedMask & 4u),
            "A standalone G3 kill admits one pinned run, cancels an old vote and shows false clear while resources prepare");
        if (!run.bClearedBossPreparation) { std::cout << "[LEGACY ADMISSION] " << room->m_strStatus << '\n'; continue; }
        const auto epoch = run.State.iRunEpoch;
        room->Notify_KoukuRaidBossDeath(dead, 101u);
        tests.Require(run.State.iRunEpoch == epoch && run.iGate3ClearTick == 100u,
            "Duplicate G3 death cannot create a new epoch or restart the five-second clock");
        CPacketWriter preparationWire;
        tests.Require(Write_Message(preparationWire, run.State), "Legacy clear PREPARING uses the existing Shared packet schema");
        room->m_WorldEntities.erase(boss);
        C2S_GATE_PROGRESS_PROPOSE bypass; bypass.iRequestSequence = 90u;
        bypass.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; bypass.eKind = GATE_PROGRESS_KIND::ADVANCE;
        room->Handle_GateProgressPropose(501u, bypass);
        tests.Require(!room->m_GateProgress.iProposalId && !room->Enter_KoukuRaidCombat(4u, room->m_iServerTick),
            "Manual advance and direct Bingo entry cannot bypass a standalone clear's resource preparation");
        // Exercise timer-first while the shared bounded resource preparation is pending.
        const auto readyTick = count % 2u ? 120u : 390u;
        room->m_iServerTick = readyTick; room->Update_KoukuRaid(readyTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
            "Five seconds alone never starts unprepared Encore resources");
        auto ack = run.Request; ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        ack.iExpectedRunEpoch = epoch; ack.iRequestSequence += 1u;
        for (unsigned id = 1u; id <= count; ++id)
        {
            std::string reason;
            tests.Require(room->Apply_KoukuRaidReadiness(500u + id, ack, reason), "Each legacy clear participant acknowledges the exact resource generation");
            if (id < count) { room->Update_KoukuRaid(readyTick); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                "Partial READY cannot start standalone Encore"); }
        }
        room->Update_KoukuRaid(readyTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.iEndTick == 250u,
            "Prepared legacy clear preserves the original death plus five-second deadline");
        if (readyTick < 250u) { room->Update_KoukuRaid(249u); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
            "Fast resource preparation still waits the complete five-second clear beat"); }
        room->m_iServerTick = (std::max)(250u, readyTick + 1u); room->Update_KoukuRaid(room->m_iServerTick);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == "BINGO" &&
            run.State.strSequencePatternId == run.pCatalog->Find_KoukuRaidGate("BINGO")->strIntroPatternId,
            "Both ready-first and timer-first races converge to the same pinned Encore cinematic");
        const auto start = run.State.iStartTick; room->Update_KoukuRaid(room->m_iServerTick);
        tests.Require(run.State.iStartTick == start, "Repeated safe ticks do not restart Encore playback");
        const auto encoreEndTick = run.State.iEndTick;
        room->m_iServerTick = encoreEndTick - 2u;
        room->Tick(1.f / 30.f);
        tests.Require(room->m_iServerTick == encoreEndTick - 1u &&
            run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC,
            "The real room tick retains Encore until its authored final tick");
        room->Tick(1.f / 30.f);
        tests.Require(room->m_iServerTick == encoreEndTick && run.State.iStartTick == encoreEndTick &&
            run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.State.strGateId == "BINGO" &&
            std::all_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& p) { return p.second.iCurrentHp > 0u; }),
            "Standalone Encore completion enters normal Bingo authority on the active tick and revives its pinned participants");
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT || run.State.strGateId != "BINGO") continue;
        const auto* bingo = run.pCatalog->Find_KoukuRaidGate("BINGO");
        std::string patternStatus;
        const auto* firstPattern = bingo && !bingo->Entries.empty() && !bingo->Entries.front().bBundle ?
            CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, bingo->Entries.front().strTargetId, patternStatus) : nullptr;
        const auto* nextPattern = bingo && bingo->Entries.size() > 1u && !bingo->Entries[1u].bBundle ?
            CKoukuSaydonBrain::Find_AnimationOnlyPattern(*run.pCatalog, bingo->Entries[1u].strTargetId, patternStatus) : nullptr;
        tests.Require(firstPattern && !firstPattern->Stages.empty() && nextPattern && nextPattern->Stages.size() > 1u,
            "Standalone Bingo pins its actual opening and following authored patterns with playable stages");
        if (!firstPattern || firstPattern->Stages.empty() || !nextPattern || nextPattern->Stages.size() < 2u) continue;
        const auto bingoBossId = run.iPrimaryBossId;
        const auto firstEntryTick = encoreEndTick + 90u;
        tests.Require(run.iNextEntryTick == firstEntryTick && !run.bEntryRunning && run.State.strFlowEntryId.empty(),
            "Standalone Bingo schedules the common 90-tick entry idle before its first authored flow");
        // Isolate scheduler progression from ordinary contact damage; keep every production tick and board timer.
        for (auto& [id, player] : room->m_Players) player.iInvulnerableEndTick = firstEntryTick + 600u;
        bool idleHeld = true;
        for (unsigned step = 1u; step < 90u; ++step)
        {
            room->Tick(1.f / 30.f);
            const auto* activeBoss = room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON");
            idleHeld = idleHeld && room->m_iServerTick == encoreEndTick + step &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && !run.bEntryRunning &&
                run.State.strFlowEntryId.empty() && activeBoss && activeBoss->iNetEntityId == bingoBossId &&
                activeBoss->iCurrentHp && activeBoss->strPatternId.empty();
        }
        tests.Require(idleHeld, "One to four resurrected participants remain in Bingo entry idle for all 89 preceding real ticks");
        const auto boardBefore = room->m_KoukuBingoDuration;
        const auto boardEpoch = room->m_iKoukuBingoBoardEpoch;
        tests.Require(boardBefore.iOwnerId == bingoBossId && boardBefore.bEncounterOwned && !boardBefore.iEndTick &&
            boardEpoch == epoch && boardBefore.iNextBombTick > firstEntryTick && boardBefore.iNextHammerTick > firstEntryTick,
            "The actual Bingo board duration belongs to the raid boss and arms persistent bomb and hammer timers during entry idle");
        room->Tick(1.f / 30.f);
        tests.Require(room->m_iServerTick == firstEntryTick && run.bEntryRunning &&
            run.State.strFlowEntryId == bingo->Entries.front().strEntryId && run.State.iFlowEntryIndex == 0u &&
            room->m_KoukuSaydonPatternAudition.Request.strPatternId == firstPattern->strPatternId,
            "The 90th real tick queues the first authored Bingo flow instead of leaving an idle combat shell");
        room->Tick(1.f / 30.f);
        const auto* playing = room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON");
        const bool openingPlaying = playing && playing->iNetEntityId == bingoBossId && playing->iCurrentHp &&
            playing->strPatternId == firstPattern->strPatternId && playing->iPatternSequence &&
            playing->iPatternStageIndex == 0u && playing->strActionId == firstPattern->Stages.front().strActionId &&
            playing->iPatternStageDurationMs == firstPattern->Stages.front().iDurationMs &&
            playing->iPatternStageFirstEvaluationTick == firstEntryTick &&
            playing->iPatternStartTick == firstEntryTick &&
            room->m_KoukuSaydonPatternAudition.iCommonStartTick == firstEntryTick &&
            !room->m_KoukuSaydonPatternAudition.Members.empty() &&
            room->m_KoukuSaydonPatternAudition.Members.front().LogicLedger.iPatternStartTick == firstEntryTick;
        tests.Require(openingPlaying, "The next real room tick starts the exact Bingo opening boss pattern and authored action stage");
        const auto openingSequence = playing ? playing->iPatternSequence : 0u;
        bool nextPlaying = false, nextStagePlaying = false, boardPreserved = true;
        // The published board opener is 34ms; 180 ticks cover it and the following pattern's first stage.
        for (unsigned step = 0u; step < 180u && !nextStagePlaying; ++step)
        {
            room->Tick(1.f / 30.f);
            playing = room->Find_KoukuSaydonArenaBoss("boss.koukusaydon.bingo.saydon", "BOSS_KOUKUSAYDON_BINGO_SAYDON");
            const auto& board = room->m_KoukuBingoDuration;
            boardPreserved = boardPreserved && board.iOwnerId == bingoBossId && board.bEncounterOwned && !board.iEndTick &&
                room->m_iKoukuBingoBoardEpoch == boardEpoch && board.iNextBombTick == boardBefore.iNextBombTick &&
                board.iNextHammerTick == boardBefore.iNextHammerTick;
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT || !playing || !playing->iCurrentHp) break;
            if (playing->strPatternId != nextPattern->strPatternId) continue;
            nextPlaying = run.bEntryRunning && run.State.iFlowEntryIndex == 1u &&
                run.State.strFlowEntryId == bingo->Entries[1u].strEntryId &&
                playing->iNetEntityId == bingoBossId && playing->iPatternSequence > openingSequence;
            nextStagePlaying = nextPlaying && playing->iPatternStageIndex == 1u &&
                playing->strActionId == nextPattern->Stages[1u].strActionId &&
                playing->iPatternStageDurationMs == nextPattern->Stages[1u].iDurationMs &&
                playing->iPatternStageFirstEvaluationTick == room->m_iServerTick;
        }
        tests.Require(nextPlaying && nextStagePlaying,
            "Real ticks complete the Bingo board opener, enter the next saved flow and advance its actual boss animation stage");
        tests.Require(boardPreserved && run.State.iRunEpoch == epoch,
            "Bingo flow and stage changes preserve the same encounter-owned board and its original bomb/hammer schedule");
        if (!openingPlaying || !nextStagePlaying || !boardPreserved)
            std::cout << "[LEGACY BINGO PLAY] players=" << count << " tick=" << room->m_iServerTick
                << " flow=" << run.State.strFlowEntryId << " pattern=" << (playing ? playing->strPatternId : "missing")
                << " reason=" << run.State.strReason << " status=" << room->m_strStatus << '\n';
        // Kill the actual owner on the real fixed tick in both pinned and standalone F1 paths.
        const auto verifyEnding = [&](const bool standalone) {
            if (standalone)
            {
                room->Stop_KoukuRaid("Standalone final clear fixture");
                for (auto& [id, player] : room->m_Players)
                { player.iCurrentHp = player.iMaximumHp; player.eAction = PLAYER_ACTION_STATE::NONE; }
                C2S_SPAWN_WORLD_ENTITY spawn;
                spawn.strPlacementId = "boss.koukusaydon.bingo.saydon";
                room->Handle_SpawnWorldEntity(501u, spawn);
                tests.Require(!(room->m_GateProgress.iClearedMask & 8u) &&
                    std::any_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [&](const auto& entity) {
                        return entity.strPlacementId == spawn.strPlacementId; }),
                    "Standalone F1 Bingo creates the authored boss and resets its previous gate clear before final-clear admission");
            }
            auto endingBoss = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& row) {
                return row.strPlacementId == "boss.koukusaydon.bingo.saydon"; });
            tests.Require(endingBoss != room->m_WorldEntities.end() && endingBoss->iMaximumHealthBars == 77u,
                "The real published Bingo boss spawns with exactly seventy-seven health bars");
            if (endingBoss == room->m_WorldEntities.end()) return;
            for (const auto& session : sessions) { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u; }
            struct ENDING_DELIVERY
            {
                bool sawEnding = false, sawClear = false, correctOrder = true;
                unsigned results = 0u, clearTick = 0u, mvpTick = 0u;
            };
            std::vector<ENDING_DELIVERY> deliveries(sessions.size());
            // The sender drains between simulation ticks in production. Observe decoded
            // deliveries at each tick instead of treating its queue as a packet history.
            const auto endingTick = [&] {
                room->Tick(1.f / 30.f);
                for (std::size_t i = 0u; i < sessions.size(); ++i)
                {
                    const auto& session = sessions[i]; auto& delivery = deliveries[i];
                    for (const auto& frame : session->m_OutboundFrames)
                    {
                        CPacketReader reader(std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES));
                        if (frame.ePacketType == PACKET_TYPE::S2C_KOUKUSAYDON_RAID_STATE)
                        {
                            S2C_KOUKUSAYDON_RAID_STATE state;
                            const bool decoded = Read_Message(reader, state);
                            delivery.correctOrder &= decoded;
                            if (decoded && state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
                                state.strSequencePatternId == "KOUKUSAYDON_G1_PATTERN_9") delivery.sawEnding = true;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_GATE_PROGRESS_STATE)
                        {
                            S2C_GATE_PROGRESS_STATE state;
                            const bool decoded = Read_Message(reader, state);
                            delivery.correctOrder &= decoded;
                            if (decoded && (state.iClearedMask & 8u))
                            {
                                delivery.correctOrder &= delivery.sawEnding; delivery.sawClear = true;
                                delivery.clearTick = room->m_iServerTick;
                            }
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_RAID_MVP_RESULT)
                        {
                            S2C_RAID_MVP_RESULT result; ++delivery.results;
                            delivery.correctOrder &= delivery.sawEnding && delivery.sawClear && Read_Message(reader, result) &&
                                result.iGate == 4u && result.Participants.size() == count;
                            delivery.mvpTick = room->m_iServerTick;
                        }
                    }
                    session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u;
                }
            };
            const auto noMvp = [&] { return std::all_of(deliveries.begin(), deliveries.end(),
                [](const auto& delivery) { return delivery.results == 0u; }); };
            endingBoss->iCurrentHp = 0u; endingBoss->eAction = SERVER_ENTITY_ACTION::DEAD;
            endingTick();
            tests.Require(room->Is_Ready() && !(room->m_GateProgress.iClearedMask & 8u) && noMvp(),
                "Actual Bingo death cannot publish a clear or MVP before its final cinematic");
            if (standalone)
            {
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.bClearedBossPreparation &&
                    run.State.strSequencePatternId == "KOUKUSAYDON_G1_PATTERN_9" && room->Is_KoukuRaidInputBlocked(),
                    "Standalone Bingo death reuses pinned shared resource preparation for its original ending");
                if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING) return;
                auto endingReady = run.Request; endingReady.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
                endingReady.iExpectedRunEpoch = run.State.iRunEpoch; ++endingReady.iRequestSequence;
                for (unsigned id = 1u; id <= count; ++id)
                {
                    std::string reason;
                    tests.Require(room->Apply_KoukuRaidReadiness(500u + id, endingReady, reason),
                        "Every standalone Bingo participant acknowledges the pinned final sequence resources");
                    if (id < count)
                    {
                        endingTick();
                        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && noMvp(),
                            "Partial readiness cannot skip the standalone Bingo ending or publish MVP");
                    }
                }
                endingTick();
            }
            const auto* finalGate = run.pCatalog ? run.pCatalog->Find_KoukuRaidGate("BINGO") : nullptr;
            tests.Require(finalGate && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.bClearCinematic &&
                run.State.strSequencePatternId == "KOUKUSAYDON_G1_PATTERN_9" && room->Is_KoukuRaidInputBlocked() &&
                !run.bEntryRunning && !room->m_KoukuBingoDuration.iOwnerId && noMvp(),
                "The next safe fixed tick starts only the original Bingo ending and retires combat and board timers");
            if (!finalGate || run.State.ePhase != KOUKUSAYDON_RAID_PHASE::CINEMATIC) return;
            const auto finalEnd = run.State.iEndTick;
            tests.Require(finalEnd == run.State.iStartTick + CKoukuSaydonLogicRuntime::Ticks_FromMs(finalGate->iClearDurationMs),
                "Bingo final clear waits for the complete pinned Sequence and sound-tail duration");
            room->m_iServerTick = finalEnd - 2u; endingTick();
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
                !(room->m_GateProgress.iClearedMask & 8u) && noMvp(),
                "The penultimate ending tick still has no gate-clear or MVP publication");
            endingTick();
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strSequencePatternId.empty() &&
                (room->m_GateProgress.iClearedMask & 8u) && !run.bEntryRunning,
                "The exact ending completion tick commits final gate clear without restarting Bingo combat");
            endingTick();
            for (const auto& delivery : deliveries)
                tests.Require(delivery.correctOrder && delivery.sawEnding && delivery.sawClear && delivery.results == 1u &&
                    delivery.clearTick == finalEnd && delivery.mvpTick == finalEnd,
                    "Each participant receives final Sequence then gate clear then exactly one roster MVP on the ending completion tick");
        };
        verifyEnding(false);
        verifyEnding(true);

    }
    for (unsigned count = 1; count <= 4; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Mario formation loads actual Gate 3 navigation");
        if (!room->Is_Ready()) continue;
        auto& audition = room->m_KoukuSaydonPatternAudition;
        audition.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
        audition.pProductGeneration = room->m_GameplayCatalog.Get_ActiveGeneration();
        audition.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        audition.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*audition.pProductGeneration);
        std::string status;
        const auto* phase = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*audition.pProductGeneration, "KOUKUSAYDON_G1_PATTERN_33", status);
        const BOSS_PATTERN_MECHANIC_TRIGGER* formation = nullptr;
        if (phase) for (const auto& trigger : phase->MechanicTriggers)
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS) formation = &trigger;
        SERVER_NAV_POINT cageGround{};
        tests.Require(formation && std::isfinite(formation->fTeleportY) &&
            room->m_ServerNavigation.Sample_Position(formation->fTeleportX, formation->fTeleportZ, cageGround) &&
            std::abs(cageGround.y - formation->fTeleportY) <= 1.f,
            "Phase 2 resolves the published typed Iron Maiden anchor on valid navigation for formation and return");
        if (!formation) continue;
        SERVER_WORLD_ENTITY boss;
        boss.iNetEntityId = 90u; boss.iPatternSequence = 7u; boss.iPatternStartTick = 100u;
        boss.strPatternId = phase->strPatternId; boss.iCurrentHp = 10000u;
        unsigned durationMs = 0u;
        for (const auto& stage : phase->Stages) durationMs += stage.iDurationMs;
        const auto deadline = CKoukuSaydonLogicRuntime::Add_Ticks(boss.iPatternStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(durationMs));
        bool nonFirstCaptive = false;
        for (unsigned seed = 1u; seed <= (count >= 3u ? 16u : 1u); ++seed)
        {
            room->m_Players.clear(); room->m_MarioLayoutRandom.seed(seed);
            for (unsigned id = 1; id <= count; ++id)
            {
                auto& player = room->m_Players[id]; player.iPlayerId = id; player.iNetEntityId = 100u + id;
                player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true;
                player.fPositionX = 20.f + id; player.fPositionY = 1.32f; player.fPositionZ = 950.f;
                if (count >= 2u && id == count) player.iMarioStage = 1u;
            }
            tests.Require(room->Commit_KoukuMarioPhasePlayers(boss, *formation, 130u),
                "The authored Mario phase-2 formation commits for one to four participants");
            unsigned bound = 0u; std::vector<float> positions;
            for (const auto& [id, player] : room->m_Players)
            {
                if (player.iMarioStage)
                {
                    SERVER_NAV_POINT landing;
                    tests.Require(player.fPositionX == 20.f + id && player.fPositionZ == 950.f && player.MarioReturnPosition &&
                        room->Resolve_MarioReturnDestination(player, landing) &&
                        std::abs(landing.x - formation->fTeleportX) < .01f && std::abs(landing.z - formation->fTeleportZ) < .01f,
                        "The Mario entrant stays in its lane and pins a navigation-valid return to the same cage anchor");
                    auto restored = player; restored.Clear_MarioControl(true);
                    tests.Require(restored.MarioReturnPosition && room->Resolve_MarioReturnDestination(restored, landing) &&
                        std::abs(landing.x - formation->fTeleportX) < .01f && std::abs(landing.z - formation->fTeleportZ) < .01f,
                        "Terminal form restoration preserves the same phase-2 return point");
                    continue;
                }
                positions.push_back(player.fPositionX);
                tests.Require(std::abs(player.fPositionZ - formation->fTeleportZ) < .01f,
                    "Each non-entrant uses the authored Iron Maiden row");
                if (!player.bPatternBound) continue;
                ++bound; nonFirstCaptive = nonFirstCaptive || id != 1u;
                tests.Require(std::abs(player.fPositionX - formation->fTeleportX) < .01f &&
                    std::abs(player.fPatternBindRestoreX - formation->fTeleportX) < .01f &&
                    std::abs(player.fPatternBindRestoreZ - formation->fTeleportZ) < .01f &&
                    player.iPatternBindOwnerNetEntityId == boss.iNetEntityId &&
                    player.iPatternBindSequence == boss.iPatternSequence && player.iPatternBindEndTick == deadline,
                    "The randomly chosen captive owns slot zero at the cage until the phase deadline");
            }
            std::sort(positions.begin(), positions.end());
            for (unsigned slot = 0; slot < positions.size(); ++slot)
                tests.Require(std::abs(positions[slot] - (formation->fTeleportX + 1.25f * slot)) < .01f,
                    "Other non-entrants retain distinct 1.25-metre offsets after the captive swap");
            tests.Require(bound == (count >= 3u ? 1u : 0u),
                "Solo and duo formations have no captive; three or four participants bind one non-entrant");
            if (count >= 3u && seed == 1u)
            {
                room->m_WorldEntities.push_back(boss);
                room->m_iServerTick = 130u; room->Update_Players(1.f / 30.f);
                room->m_iServerTick = 131u; room->Update_Players(1.f / 30.f);
                const auto captive = std::find_if(room->m_Players.begin(), room->m_Players.end(),
                    [](const auto& row) { return row.second.bPatternBound; });
                tests.Require(captive != room->m_Players.end() && captive->second.isCombatReady &&
                    captive->second.bPatternBindRestoreCombatReady && !captive->second.hasMoveGoal,
                    "Actual room ticks keep Iron Maiden input locked while retaining authoritative damage eligibility");
                if (captive != room->m_Players.end())
                {
                    auto victim = captive->second;
                    BOSS_PATTERN_LOGIC_RESULT death{}; death.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH;
                    std::vector<DAMAGE_EVENT> damage;
                    CKoukuSaydonLogicRuntime::Apply_Result(victim, death, boss, catalog, nullptr, 132u, damage);
                    tests.Require(victim.iCurrentHp == 0u && victim.eAction == PLAYER_ACTION_STATE::DEAD && !damage.empty(),
                        "The same Server instant-death result kills an Iron Maiden captive after real player updates");
                }
                room->m_WorldEntities.pop_back();
            }
        }
        if (count >= 3u) tests.Require(nonFirstCaptive, "Formation regression exercises a random captive other than the first player");
        // Admit the next formation explicitly so this tests its blocked destination path.
        for (auto& [id, player] : room->m_Players)
            if (player.iCurrentHp && !player.iMarioStage) player.isCombatReady = true;
        const auto saved = room->m_Players;
        auto blocked = *formation; blocked.fTeleportX = 1000000.f;
        tests.Require(!room->Commit_KoukuMarioPhasePlayers(boss, blocked, 131u), "An invalid cage destination rejects the formation");
        for (const auto& [id, player] : room->m_Players)
        {
            const auto& prior = saved.at(id);
            tests.Require(player.fPositionX == prior.fPositionX && player.fPositionY == prior.fPositionY &&
                player.fPositionZ == prior.fPositionZ && player.bPatternBound == prior.bPatternBound &&
                player.MarioReturnPosition == prior.MarioReturnPosition,
                "Rejected formation preserves every participant's position, binding and return pin");
        }
    }
    const bool raidPublished = catalog.Find_KoukuRaidGate("GATE1") &&
        catalog.Find_KoukuRaidGate("GATE2") && catalog.Find_KoukuRaidGate("GATE3");
    tests.Require(raidPublished, "All three gate flows and sequence clocks are published");
    if (!raidPublished) return;
#ifdef _DEBUG
    for (unsigned failure = 0u; failure < 4u; ++failure)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        if (!room->Is_Ready()) continue;
        auto connection = std::make_shared<CClientSession>(501u, INVALID_SOCKET,
            CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        connection->m_isSendRunning.store(true); room->m_Sessions.emplace(501u, connection);
        auto& player = room->m_Players[1u]; player.iPlayerId = 1u; player.iSessionId = 501u; player.iNetEntityId = 101u;
        player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true;
        player.fPositionX = 9.f; player.fPositionZ = -61.f; player.bPatternBound = true;
        room->m_PlayerIdBySessionId[501u] = 1u;
        SERVER_WORLD_ENTITY existing;
        const auto* placement = room->Find_Placement("boss.koukusaydon.g1.saydon");
        if (placement && room->Build_WorldEntity(*placement, 987u, existing)) room->m_WorldEntities.push_back(std::move(existing));
        const auto beforeEntities = room->m_WorldEntities.size();
        C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
        request.iRequestSequence = 1u; request.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; request.strStartGateId = "GATE1";
        request.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        request.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        request.iSequenceSourceRevision = room->m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1")->iSequenceRevision;
        room->m_iServerTick = 20u; room->Handle_KoukuRaidRequest(501u, request);
        tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING, "Abort fixture enters preparation through START");
        request.iRequestSequence = 2u; request.iExpectedRunEpoch = room->m_KoukuRaid.State.iRunEpoch;
        if (failure == 0u) { request.eOperation = KOUKUSAYDON_RAID_OPERATION::FAILED; request.strReason = "Sequence revision unavailable"; room->Handle_KoukuRaidRequest(501u, request); }
        else if (failure == 1u)
        {
            room->m_iServerTick = 320u; room->Update_KoukuRaid(320u);
            tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
                !room->m_KoukuRaid.State.iReadyMask && !room->m_KoukuRaid.State.iStartTick &&
                room->m_WorldEntities.size() == beforeEntities && player.fPositionX == 9.f,
                "Debug resource loading may exceed ten seconds without partially starting actors or the cinematic");
            room->m_iServerTick = room->m_KoukuRaid.State.iEndTick;
            room->Update_KoukuRaid(room->m_iServerTick);
        }
        else if (failure == 2u) { request.eOperation = KOUKUSAYDON_RAID_OPERATION::STOP; room->Handle_KoukuRaidRequest(501u, request); }
        else { room->m_PlayerIdBySessionId.erase(501u); room->Update_KoukuRaid(21u); }
        tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::ABORTED &&
            room->m_WorldEntities.size() == beforeEntities && player.fPositionX == 9.f && player.fPositionZ == -61.f && player.bPatternBound,
            "FAILED, timeout, owner STOP and participant loss preserve pre-preparation actors, position and gameplay state");
        const auto oldEpoch = request.iExpectedRunEpoch;
        room->m_PlayerIdBySessionId[501u] = 1u;
        auto restart = request; restart.eOperation = KOUKUSAYDON_RAID_OPERATION::START;
        restart.iRequestSequence = 3u; restart.iExpectedRunEpoch = 0u; restart.strReason.clear();
        room->Handle_KoukuRaidRequest(501u, restart);
        auto oldAck = restart; oldAck.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        oldAck.iRequestSequence = 4u; oldAck.iExpectedRunEpoch = oldEpoch;
        room->Handle_KoukuRaidRequest(501u, oldAck);
        tests.Require(room->m_KoukuRaid.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
            room->m_KoukuRaid.State.iRunEpoch != oldEpoch && room->m_KoukuRaid.State.iReadyMask == 0u,
            "An old READY cannot acknowledge or abort the next run after preparation failure");
    }
#endif
    for (unsigned count = 1; count <= 4; ++count)
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Raid room loads navigation and world templates");
        if (!room->Is_Ready()) continue;
        auto& run = room->m_KoukuRaid;
        std::vector<std::shared_ptr<CClientSession>> sessions;
        for (unsigned id = 1; id <= count; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 500u + id;
            player.iNetEntityId = 100u + id; player.iCurrentHp = player.iMaximumHp = 100u; player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            player.strNickName = "Raid-" + std::to_string(id);
            player.isCombatReady = true; player.fPositionX = 10.f + id; player.fPositionZ = -60.f;
            room->m_PlayerIdBySessionId[player.iSessionId] = id;
            auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            connection->m_isSendRunning.store(true); room->m_Sessions.emplace(player.iSessionId, connection);
            sessions.push_back(std::move(connection));
        }
        C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST start;
        start.iRequestSequence = 1u; start.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; start.strStartGateId = "GATE1";
        start.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        start.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        start.iSequenceSourceRevision = room->m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1")->iSequenceRevision;
        room->m_iServerTick = 10u;
        const auto originalEntityCount = room->m_WorldEntities.size();
        begin(*room, 501u, start);
        constexpr unsigned preparationTicks = 20u * 60u * 30u;
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && run.State.iStartTick == 0u &&
            run.State.iEndTick == 10u + preparationTicks && run.State.ParticipantPlayerIds.size() == count,
            "Real START command pins the roster and enters bounded resource preparation without starting the cinematic");
        if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING)
        { std::cout << "[RAID START] " << room->m_strStatus << '\n'; continue; }
        auto ack = start; ack.iRequestSequence = 2u; ack.iExpectedRunEpoch = run.State.iRunEpoch;
        ack.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
        if (count > 1u)
        {
            auto foreignStop = ack; foreignStop.eOperation = KOUKUSAYDON_RAID_OPERATION::STOP;
            room->Handle_KoukuRaidRequest(500u + count, foreignStop);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                "A non-owner cannot cancel preparation");
        }
        std::string reason;
        auto stale = ack; ++stale.iSequenceSourceRevision;
        tests.Require(!room->Apply_KoukuRaidReadiness(501u, stale, reason) && !run.State.iReadyMask,
            "A mismatched Sequence revision cannot acknowledge preparation");
        stale = ack; ++stale.iActionSourceRevision;
        tests.Require(!room->Apply_KoukuRaidReadiness(501u, stale, reason) && !run.State.iReadyMask,
            "A mismatched Action revision cannot acknowledge preparation");
        stale = ack; ++stale.iExpectedRunEpoch;
        tests.Require(!room->Apply_KoukuRaidReadiness(501u, stale, reason) && !run.State.iReadyMask,
            "An ACK from a different raid epoch is isolated");
        room->m_PlayerIdBySessionId[999u] = 999u;
        tests.Require(!room->Apply_KoukuRaidReadiness(999u, ack, reason) && !run.State.iReadyMask,
            "Late-join observers cannot acknowledge the fixed START roster");
        room->m_PlayerIdBySessionId.erase(999u);
        for (unsigned id = 1; id <= count; ++id)
        {
            room->Handle_KoukuRaidRequest(500u + id, ack);
            const auto mask = run.State.iReadyMask;
            room->Handle_KoukuRaidRequest(500u + id, ack);
            tests.Require(run.State.iReadyMask == mask, "Duplicate READY is idempotent through the real session command handler");
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
                room->m_WorldEntities.size() == originalEntityCount && room->m_Players.at(id).fPositionX == 10.f + id,
                "Every READY preserves existing actors and positions until the shared fixed-tick commit");
            if (id < count) { room->Update_KoukuRaid(11u); tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING,
                "One missing participant prevents cinematic start"); }
        }
        room->m_iServerTick = 12u; room->Update_KoukuRaid(12u);
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.iStartTick == 13u,
            "All participants READY commits one common future cinematic tick");
        unsigned proposalSequence = 1u;
        const auto propose = [&](GATE_PROGRESS_KIND kind) {
            C2S_GATE_PROGRESS_PROPOSE request; request.iRequestSequence = proposalSequence++;
            request.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; request.eKind = kind;
            room->Handle_GateProgressPropose(501u, request);
        };
        const auto respond = [&](unsigned id, bool accepted) {
            C2S_GATE_PROGRESS_RESPOND request; request.iRequestSequence = proposalSequence++;
            request.iProposalId = room->m_GateProgress.iProposalId; request.bAccepted = accepted;
            room->Handle_GateProgressRespond(500u + id, request);
        };
        for (const auto* gateId : {"GATE1", "GATE2", "GATE3"})
        {
            const auto* gate = run.pCatalog->Find_KoukuRaidGate(gateId);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == gateId,
                "The next gate remains cinematic until its authored end");
            auto end = run.State.iEndTick;
            room->m_iServerTick = end - 1u; room->Update_KoukuRaid(end - 1u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.iPrimaryBossId == INVALID_NET_ENTITY_ID,
                "Combat actor admission cannot run one tick before the sequence ends");
            room->m_iServerTick = end; room->Update_KoukuRaid(end);
            for (const auto& arrival : gate->Arrivals)
            {
                if (arrival.bClear || arrival.iSlot >= count) continue;
                const auto& player = room->m_Players.at(run.PlayerIds[arrival.iSlot]);
                tests.Require(std::abs(player.fPositionX - arrival.Position[0]) < .01f &&
                    std::abs(player.fPositionZ - arrival.Position[2]) < .01f,
                    "Each participating player arrives at its authored sequence slot");
            }
            if (std::string(gateId) == "GATE3")
            {
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !run.bEntryRunning &&
                    run.iPrimaryBossId == INVALID_NET_ENTITY_ID && room->Is_KoukuRaidInputBlocked(),
                    "Gate 3 intro holds the authored arrival positions with gameplay blocked until assembly completes");
                CPacketWriter writer;
                tests.Require(Write_Message(writer, run.State), "Gate 3 entry wait has a valid replicated packet");
                CPacketReader reader{writer.Get_Buffer()}; S2C_KOUKUSAYDON_RAID_STATE decoded;
                tests.Require(Read_Message(reader, decoded) && decoded.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY &&
                    decoded.strGateId == "GATE3", "Gate 3 entry wait survives the Shared writer and reader");
                auto invalid = run.State; invalid.strGateId = "GATE2"; CPacketWriter invalidWriter;
                tests.Require(!Write_Message(invalidWriter, invalid), "Other gates cannot claim Gate 3 entry wait");
                room->m_iServerTick = end + 900u; room->Update_KoukuRaid(end + 900u);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !run.bEntryRunning,
                    "Elapsed cinematic time cannot start Gate 3 combat before the leader gathers in the entry aura");
                const auto firstX = room->m_Players.at(1u).fPositionX;
                room->m_Players.at(count).bPatternBound = true;
                tests.Require(!room->Enter_KoukuRaidCombat(3u, room->m_iServerTick) && room->m_Players.at(1u).fPositionX == firstX &&
                    run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !run.bEntryRunning,
                    "A rejected participant keeps the entire arrival formation and flow unchanged");
                room->m_Players.at(count).bPatternBound = false;
                const auto beforeAdmissionPlayers = room->m_Players;
                const auto beforeAdmissionEntities = room->m_WorldEntities.size();
                const auto beforeAdmissionGate = room->m_GateProgress;
                const auto gameplayPin = run.State.PinnedGameplayRevision;
                run.State.PinnedGameplayRevision = {};
                tests.Require(!room->Enter_KoukuRaidCombat(3u, room->m_iServerTick) && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY &&
                    room->m_WorldEntities.size() == beforeAdmissionEntities && room->m_GateProgress.iCurrentGate == beforeAdmissionGate.iCurrentGate &&
                    room->m_GateProgress.iClearedMask == beforeAdmissionGate.iClearedMask,
                    "Rejected first-flow admission preserves Gate 3 arrival state before any actor or GateProgress commit");
                for (const auto& [id, player] : room->m_Players)
                {
                    const auto& previous = beforeAdmissionPlayers.at(id);
                    tests.Require(player.fPositionX == previous.fPositionX && player.fPositionY == previous.fPositionY &&
                        player.fPositionZ == previous.fPositionZ && player.iCurrentHp == previous.iCurrentHp,
                        "Rejected first-flow admission preserves every Gate 3 player");
                }
                run.State.PinnedGameplayRevision = gameplayPin;
                auto& entryLeader = room->m_Players.at(run.State.iOwnerPlayerId);
                entryLeader.fPositionX = -22.20617676f; entryLeader.fPositionY = 25.6f;
                entryLeader.fPositionZ = 954.5942993f;
                ++room->m_iServerTick; room->Expire_GateProgressVote();
                ++room->m_iServerTick; room->Expire_GateProgressVote();
                room->m_iServerTick += 299u; room->Expire_GateProgressVote();
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY && !room->m_GateProgress.iProposalId,
                    "At 299 Server ticks the prepared Gate 3 raid remains on the terrace without a vote");
                ++room->m_iServerTick; room->Expire_GateProgressVote();
                for (const auto id : run.PlayerIds)
                    tests.Require(std::abs(room->m_Players.at(id).fPositionX + 2.45f) < .01f &&
                        std::abs(room->m_Players.at(id).fPositionZ - 945.17f) < .01f,
                        "Automatic Gate 3 entry moves every player to the existing combat spawn");
                tests.Require(run.bGate3CombatEntered && !room->Is_KoukuRaidInputBlocked(),
                    "First Gate 3 entry enables combat and future restart semantics");
            }
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT)
                std::cout << "[RAID STATUS] " << gateId << ": " << run.State.strReason << '\n';
            finishEntryIdle(*room);
            end = room->m_iServerTick;
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning,
                "Sequence completion starts the saved Server pattern flow");
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) break;
            if (std::string(gateId) == "GATE1" || std::string(gateId) == "GATE3")
            {
                if (count > 1u)
                {
                    C2S_GATE_PROGRESS_PROPOSE unauthorized; unauthorized.iRequestSequence = proposalSequence++;
                    unauthorized.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; unauthorized.eKind = GATE_PROGRESS_KIND::RESTART;
                    room->Handle_GateProgressPropose(500u + count, unauthorized);
                    tests.Require(!room->m_GateProgress.iProposalId, "Only the raid owner can propose a gate vote");
                }
                room->m_Players.at(1u).iCurrentHp = 0u; room->m_Players.at(1u).eAction = PLAYER_ACTION_STATE::DEAD;
                propose(GATE_PROGRESS_KIND::RESTART);
                for (unsigned id = 2u; id <= count; ++id) respond(id, true);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.State.strGateId == gateId &&
                    run.State.strSequencePatternId == gate->strIntroPatternId && room->m_Players.at(1u).iCurrentHp == 100u,
                    "An approved restart revives participants and replays the same gate intro before combat");
                end = run.State.iEndTick; room->m_iServerTick = end; room->Update_KoukuRaid(end);
                finishEntryIdle(*room);
                end = room->m_iServerTick;
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && run.bEntryRunning,
                    "Restarted intro resumes the first authored flow entry");
            }
            SERVER_WORLD_ENTITY dead;
            dead.iNetEntityId = run.iPrimaryBossId; dead.iCurrentHp = 100u;
            room->Notify_KoukuRaidBossDeath(dead, end + 1u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT,
                "Living boss cannot trigger a gate transition");
            if (std::string(gateId) == "GATE2")
            {
                auto supporting = dead; ++supporting.iNetEntityId; supporting.iCurrentHp = 0u;
                supporting.strPlacementId = "boss.koukusaydon.g2.big-saydon";
                room->Notify_KoukuRaidBossDeath(supporting, end + 1u); room->Notify_GateBossDeath(supporting);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && !(room->m_GateProgress.iClearedMask & 2u),
                    "Gate 2 supporting actor cannot clear the pinned primary boss encounter");
            }
            for (const auto& session : sessions)
            { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0u; }
            dead.iCurrentHp = 0u; room->Notify_KoukuRaidBossDeath(dead, end + 2u);
            for (const auto& session : sessions)
            {
                unsigned resultCount = 0u; bool actualRoster = false;
                for (const auto& frame : session->m_OutboundFrames)
                {
                    if (frame.ePacketType != PACKET_TYPE::S2C_RAID_MVP_RESULT) continue;
                    ++resultCount; S2C_RAID_MVP_RESULT result;
                    CPacketReader reader(std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES));
                    actualRoster = Read_Message(reader, result) && result.iGate == room->m_GateProgress.iCurrentGate &&
                        result.Participants.size() == count && std::all_of(result.Participants.begin(), result.Participants.end(),
                            [&](const auto& row) { return room->m_Players.contains(row.iPlayerId) &&
                                row.iNetEntityId == room->m_Players.at(row.iPlayerId).iNetEntityId && !row.iDamage; });
                }
                tests.Require(std::string(gateId) == "GATE3" ? resultCount == 0u : resultCount == 1u && actualRoster,
                    "Immediate Gate 1/2 clear sends the actual zero-damage roster MVP once; Gate 3 false clear sends none");
            }
            const auto currentGate = room->m_GateProgress.iCurrentGate;
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
                (room->m_GateProgress.iClearedMask & (1u << (currentGate - 1u))),
                "Actual primary boss death opens the existing clear UI and waits without a deadline");
            if (currentGate == 3u)
            {
                const auto* encore = run.pCatalog->Find_KoukuRaidGate("BINGO");
                tests.Require(encore && !encore->strIntroPatternId.empty() &&
                    run.State.iEndTick == end + 152u && room->Is_KoukuRaidInputBlocked(),
                    "Gate 3 false clear reserves exactly five Server seconds and blocks gameplay");
                propose(GATE_PROGRESS_KIND::ADVANCE);
                tests.Require(!room->m_GateProgress.iProposalId && !room->Enter_KoukuRaidCombat(4u, room->m_iServerTick),
                    "A vote or direct entry cannot bypass the pending encore");
                continue;
            }
            room->m_iServerTick = end + 9000u; room->Update_KoukuRaid(end + 9000u);
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strGateId == gateId,
                "Other cleared gates retain their explicit next-gate vote");
            if (count > 1u)
            {
                propose(GATE_PROGRESS_KIND::ADVANCE);
                tests.Require(room->m_GateProgress.Voters.size() == count && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "Every pinned participant votes even when the room players have no party object");
                room->m_PlayerIdBySessionId[999u] = 999u;
                C2S_GATE_PROGRESS_RESPOND observer; observer.iRequestSequence = proposalSequence++;
                observer.iProposalId = room->m_GateProgress.iProposalId; observer.bAccepted = true;
                room->Handle_GateProgressRespond(999u, observer);
                room->m_PlayerIdBySessionId.erase(999u);
                tests.Require(room->m_GateProgress.Accepted.size() == 1u, "A late join cannot approve a fixed-roster gate vote");
                respond(2u, false);
                tests.Require(!room->m_GateProgress.iProposalId && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "Declining gate entry preserves the cleared gate and its presentation pin");
                propose(GATE_PROGRESS_KIND::ADVANCE);
                room->m_iServerTick = room->m_GateProgress.iDeadlineTick; room->Expire_GateProgressVote();
                tests.Require(!room->m_GateProgress.iProposalId && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "A gate vote timeout does not start a sequence or despawn the arena");
            }
            propose(GATE_PROGRESS_KIND::ADVANCE);
            for (unsigned id = 2u; id <= count; ++id)
            {
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                    "Partial gate approval cannot start the next sequence");
                respond(id, true);
            }
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC,
                "Only unanimous gate entry approval starts the next cinematic");
            if (std::string(gateId) == "GATE2")
            {
                tests.Require(!run.bClearCinematic && run.State.strGateId == "GATE3" &&
                    run.State.strSequencePatternId == run.pCatalog->Find_KoukuRaidGate("GATE3")->strIntroPatternId,
                    "Approved Gate 2 exit starts only the Gate 3 intro without an extra clear movie");
            }
        }
        tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && room->m_GateProgress.iClearedMask == 7u,
            "One to four participants can clear every gate with explicit UI approvals");
        if (const auto* bingo = run.pCatalog->Find_KoukuRaidGate("BINGO"))
        {
            // The saved encore removes 5000 ms of lead-in while retaining its audio tail.
            tests.Require(bingo->strIntroPatternId == "KOUKUSAYDON_G1_PATTERN_10" && bingo->iIntroDurationMs == 21322u &&
                bingo->Entries.size() == 35u && !bingo->strLoopStartEntryId.empty(),
                "Published Bingo pins the trimmed 21322 ms encore before its saved independent combat flow");
            if (bingo->Entries.empty()) continue;
            const auto encoreStartTick = run.State.iEndTick;
            room->m_iServerTick = encoreStartTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "GATE3" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE,
                "One tick before five seconds the false-clear scene is retained");
            room->m_iServerTick = encoreStartTick; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
                !run.bClearCinematic && run.State.strSequencePatternId == bingo->strIntroPatternId && run.State.iStartTick == encoreStartTick && room->Is_KoukuRaidInputBlocked(),
                "The five-second deadline starts one server-clock encore without a vote");
            CPacketWriter encoreWriter;
            tests.Require(Write_Message(encoreWriter, run.State), "Encore intro is valid on the Shared wire");
            room->m_iServerTick = run.State.iEndTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
            tests.Require(!room->Enter_KoukuRaidCombat(4u, room->m_iServerTick) && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC,
                "Bingo combat cannot cut the encore short by one tick");
            room->m_iServerTick = run.State.iEndTick;
            const auto originalPlayers = room->m_Players;
            const auto originalWorldCount = room->m_WorldEntities.size();
            const auto originalGate = room->m_GateProgress;
            const auto gameplayPin = run.State.PinnedGameplayRevision;
            run.State.PinnedGameplayRevision = {};
            tests.Require(!room->Enter_KoukuRaidCombat(4u, room->m_iServerTick) && run.State.strGateId == "BINGO" &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && room->m_WorldEntities.size() == originalWorldCount &&
                room->m_GateProgress.iCurrentGate == originalGate.iCurrentGate && room->m_GateProgress.iClearedMask == originalGate.iClearedMask,
                "Rejected Bingo audition admission preserves old actors, gate progress and the cleared flow");
            for (const auto& [id, player] : room->m_Players)
            {
                const auto& previous = originalPlayers.at(id);
                tests.Require(player.fPositionX == previous.fPositionX && player.fPositionY == previous.fPositionY &&
                    player.fPositionZ == previous.fPositionZ && player.iCurrentHp == previous.iCurrentHp,
                    "Rejected Bingo audition admission preserves the full formation");
            }
            run.State.PinnedGameplayRevision = gameplayPin;
            const auto originalSupport = room->m_ServerNavigation.Get_RuntimeSupportSurfaces();
            auto blockedSupport = originalSupport;
            blockedSupport.push_back({"contract.bingo.entry.height", -3.4f, 1147.44f, 2.f, 5.f});
            std::string supportStatus;
            tests.Require(room->m_ServerNavigation.Set_RuntimeSupportSurfaces(blockedSupport, supportStatus),
                "Bingo rejection fixture stages an incompatible destination deck");
            tests.Require(!room->Enter_KoukuRaidCombat(4u, room->m_iServerTick) && run.State.strGateId == "BINGO" &&
                run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && room->m_WorldEntities.size() == originalWorldCount,
                "Rejected Bingo navigation preserves the cleared gate, flow and existing world");
            for (const auto& [id, player] : room->m_Players)
            {
                const auto& previous = originalPlayers.at(id);
                tests.Require(player.fPositionX == previous.fPositionX && player.fPositionY == previous.fPositionY &&
                    player.fPositionZ == previous.fPositionZ && player.iCurrentHp == previous.iCurrentHp,
                    "Rejected Bingo entry does not partially move or revive any participant");
            }
            tests.Require(room->m_ServerNavigation.Set_RuntimeSupportSurfaces(originalSupport, supportStatus),
                "Bingo rejection fixture restores the original destination deck");
            room->m_Players.at(count).iCurrentHp = 0u;
            room->m_Players.at(count).eAction = PLAYER_ACTION_STATE::DEAD;
            room->Update_KoukuRaid(room->m_iServerTick);
            finishEntryIdle(*room);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                run.bEntryRunning && room->m_GateProgress.iCurrentGate == 4u && !room->Is_KoukuRaidInputBlocked(),
                "Encore completion revives the full roster and starts the saved Bingo combat flow");
            if (run.State.strGateId != "BINGO" || run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT) continue;
            for (const auto& [id, player] : room->m_Players)
                tests.Require(player.iCurrentHp == player.iMaximumHp && std::abs(player.fPositionX + 3.4f) < .01f &&
                    std::abs(player.fPositionZ - 1147.44f) < .01f,
                    "Bingo entry revives and moves the complete fixed roster to its authored combat spawn");
            verifyBingoFlow(*room, room->m_iServerTick + 1u, false);
            for (auto& [id, player] : room->m_Players) player.iCurrentHp = player.iMaximumHp = 100u;
            for (auto& entity : room->m_WorldEntities)
                if (entity.iNetEntityId == run.iPrimaryBossId)
                { entity.fPositionX = -3.4f; entity.fPositionY = 0.f; entity.fPositionZ = 1147.44f; }
            room->m_Players.at(count).iCurrentHp = 0u;
            room->m_Players.at(count).eAction = PLAYER_ACTION_STATE::DEAD;
            propose(GATE_PROGRESS_KIND::RESTART);
            for (unsigned id = 2u; id <= count; ++id) respond(id, true);
            finishEntryIdle(*room);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                run.State.iFlowEntryIndex == 0u && run.bEntryRunning && room->m_Players.at(count).iCurrentHp == 100u,
                "Bingo restart revives the roster and ignores its soon-despawned boss blocking the original spawn");
            room->Stop_KoukuRaid("Standalone Bingo START contract");
            auto bingoStart = start; bingoStart.iRequestSequence = 100u; bingoStart.strStartGateId = "BINGO";
            CPacketWriter requestWriter;
            tests.Require(Write_Message(requestWriter, bingoStart), "Standalone Bingo START has a valid typed request");
            begin(*room, 501u, bingoStart);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING &&
                run.State.strSequencePatternId.empty() && !run.bEntryRunning,
                "Standalone Bingo START pins its cinematic resources while retaining direct combat entry");
            CPacketWriter prepareWriter;
            tests.Require(Write_Message(prepareWriter, run.State), "No-intro Bingo preparation can be broadcast to real clients");
            CPacketReader prepareReader{prepareWriter.Get_Buffer()}; S2C_KOUKUSAYDON_RAID_STATE preparation;
            tests.Require(Read_Message(prepareReader, preparation) && preparation.strGateId == "BINGO" &&
                preparation.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING && preparation.strSequencePatternId.empty(),
                "No-intro Bingo preparation survives the Shared writer and reader");
            auto bingoReady = bingoStart; bingoReady.iRequestSequence = 101u;
            bingoReady.eOperation = KOUKUSAYDON_RAID_OPERATION::READY; bingoReady.iExpectedRunEpoch = run.State.iRunEpoch;
            for (unsigned id = 1u; id <= count; ++id) room->Handle_KoukuRaidRequest(500u + id, bingoReady);
            ++room->m_iServerTick; room->Update_KoukuRaid(room->m_iServerTick);
            finishEntryIdle(*room);
            tests.Require(run.State.strGateId == "BINGO" && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                run.bEntryRunning, "Standalone Bingo READY starts the same saved combat loop without a cinematic phase");
            if (!bingo->strClearPatternId.empty())
            {
                tests.Require(bingo->strClearPatternId == "KOUKUSAYDON_G1_PATTERN_9" && bingo->iClearDurationMs >= 49083u,
                    "Published Bingo clear reuses its original ending and finite sound tail clock");
                SERVER_WORLD_ENTITY endingBoss;
                endingBoss.iNetEntityId = run.iPrimaryBossId; endingBoss.iCurrentHp = 0u;
                endingBoss.eAction = SERVER_ENTITY_ACTION::DEAD;
                endingBoss.strPlacementId = bingo->strPrimaryBossPlacementId;
                const auto primaryBeforeDeath = run.iPrimaryBossId;
                room->Notify_KoukuRaidBossDeath(endingBoss, ++room->m_iServerTick);
                room->Notify_GateBossDeath(endingBoss);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE &&
                    !(room->m_GateProgress.iClearedMask & 8u) && run.iPrimaryBossId == primaryBeforeDeath,
                    "Bingo death delays clear confirmation and defers actor cleanup beyond the boss iteration");
                room->Update_KoukuRaid(++room->m_iServerTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && run.bClearCinematic &&
                    run.State.strGateId == "BINGO" && run.State.strSequencePatternId == bingo->strClearPatternId &&
                    room->Is_KoukuRaidInputBlocked() && !run.bEntryRunning && !(room->m_GateProgress.iClearedMask & 8u),
                    "The next safe tick starts the pinned Bingo ending and blocks combat until it finishes");
                CPacketWriter endingWriter;
                tests.Require(Write_Message(endingWriter, run.State), "Bingo ending state has a valid Shared wire contract");
                const auto endingTick = run.State.iEndTick;
                room->m_iServerTick = endingTick - 1u; room->Update_KoukuRaid(room->m_iServerTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC && !(room->m_GateProgress.iClearedMask & 8u),
                    "Bingo ending keeps the clear flag pending until the last presentation tick");
                room->m_iServerTick = endingTick; room->Update_KoukuRaid(endingTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strSequencePatternId.empty() &&
                    run.State.iEndTick == 0u && (room->m_GateProgress.iClearedMask & 8u) && run.bClearCinematic,
                    "Bingo ending completes once and commits its clear flag without restarting combat");
                room->m_iServerTick += 9000u; room->Update_KoukuRaid(room->m_iServerTick);
                tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && run.State.strSequencePatternId.empty() && !run.bEntryRunning,
                    "Finished Bingo ending remains idle and cannot replay on subsequent ticks");
            }

        }
        else std::cout << "[RAID BINGO] Published Bingo gate not present; the optional transition contract was not exercised.\n";
    }
}

int CServerGameplayContractRunner::Run_KoukuRaid()
{
    int result = 1;
    const auto execute = [](void* opaque) {
        TESTS tests;
        Run_KoukuRaidIntegration(tests);
        CGameplayCatalog catalog;
        if (catalog.Load()) Run_KoukuFearAndCounterContracts(tests, catalog);
        else tests.Require(false, "Load fear/counter catalog");
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(opaque) = tests.failures ? 1 : 0;
    };
    if (!Run_WithContractWorkerStack(execute, &result)) return 1;
    return result;
}


int CServerGameplayContractRunner::Run_KoukuDiceDamageContracts()
{
    int result = 1;
    const auto execute = [](void* opaque) {
        TESTS tests;
        // 아래 피해 fixture는 매번 새 플레이어 상태에서 시작한다. 제품의 표본 생성기를 복제하지 않고
        // 각 타격의 ±10% 범위, 대상별 event 수·HP 감소 합, 표본 serial을 독립적으로 확인한다.
        // 면역·원거리 대상은 event와 표본이 모두 0이어야 하므로 범위 허용으로 잘못된 타격을 숨기지 않는다.
        const auto matchesIncomingDamage = [](const SERVER_PLAYER& player, std::uint32_t initialHp,
            std::uint32_t baseline, std::size_t expectedCount, const std::vector<DAMAGE_EVENT>& hits) {
            std::uint64_t amount = 0u;
            std::size_t count = 0u;
            for (const auto& event : hits) {
                if (event.iTargetNetEntityId != player.iNetEntityId) continue;
                if (event.isOutgoing || event.eHitFlag != DAMAGE_HIT_FLAG::NORMAL ||
                    event.iAmount < baseline - baseline / 10u || event.iAmount > baseline + baseline / 10u)
                    return false;
                amount += event.iAmount;
                ++count;
            }
            return count == expectedCount && player.iIncomingDamageSampleSerial == expectedCount &&
                player.iCurrentHp <= initialHp && amount == initialHp - player.iCurrentHp;
        };
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Dice damage fixture loads authoritative gameplay");
        if (!room->Is_Ready()) { std::cout << "Dice room: " << room->Get_Status() << '\n'; *static_cast<int*>(opaque) = 1; return; }
        auto& catalog = room->m_GameplayCatalog;
        // Release retains disabled gate templates until admission; this focused fixture owns its actor.
        room->m_WorldEntities.clear(); room->m_WorldEntities.reserve(2u); room->m_WorldEntities.emplace_back();
        auto& boss = room->m_WorldEntities.front();
        boss.iNetEntityId = 700u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1"; boss.strArchetypeId = "BOSS_KOUKUSAYDON_G1_SAYDON";
        boss.iCurrentHp = 100u; boss.eAction = SERVER_ENTITY_ACTION::IDLE;
        boss.strPatternId = "dice.hit.contract"; boss.iPatternSequence = 700u;
        boss.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
        boss.fPositionX = boss.fPositionY = boss.fPositionZ = 0.f;
        boss.iPatternTargetEntityId = 101u;
        const auto seedPlayers = [&](MECHANIC_CARD_COLOR color) {
            room->m_Players.clear();
            for (PLAYER_ID id = 1u; id <= 6u; ++id) {
                auto& p = room->m_Players[id]; p.iPlayerId = id; p.iNetEntityId = 100u + id;
                p.iCurrentHp = p.iMaximumHp = 1000u * id; p.isCombatReady = true;
                p.eMechanicCardSymbol = id <= 4u ? static_cast<MECHANIC_CARD_SYMBOL>(id) : MECHANIC_CARD_SYMBOL::NONE;
                p.eMechanicCardColor = color;
                p.bPatternBound = id == 2u || id == 3u;
                p.iPatternBindOwnerNetEntityId = p.bPatternBound ? boss.iNetEntityId : INVALID_NET_ENTITY_ID;
                p.iPatternBindSequence = p.bPatternBound ? boss.iPatternSequence : 0u;
                if (id == 5u) p.fPositionX = 100.f;
            }
        };
        for (const char* hitKind : {"CONTACT", "TIMED"})
        for (unsigned symbol = 1u; symbol <= 4u; ++symbol)
        for (const auto color : {MECHANIC_CARD_COLOR::RED, MECHANIC_CARD_COLOR::BLACK}) {
            seedPlayers(color); room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
            BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
            trigger.strTriggerId = "dice.card"; trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
            trigger.iDurationMs = 1000u; trigger.ProjectileVisualIds = {"test.card"};
            trigger.ProjectileCardSymbols = {static_cast<MECHANIC_CARD_SYMBOL>(symbol)};
            trigger.strContactVisualId = "test.card.burst"; trigger.fProjectileSpeedMps = 1.f;
            trigger.fProjectileContactRadiusM = 1.f; trigger.iProjectileLifetimeMs = 1000u;
            trigger.iProjectileCountPerWave = 1u; trigger.bProjectileHoming = true;
            ATTACK_HIT_TEMPLATE hit{}; hit.strHitId = "card.contact"; hit.strTrigger = hitKind;
            hit.strShape = "CIRCLE"; hit.strDamageKind = "MAX_HP_PERCENT"; hit.iDamagePercent = 90u;
            hit.fRadiusM = 1.; hit.iRepeatCount = 1u;
            if (hit.strTrigger == "CONTACT") hit.iEndMs = 1000u;
            else hit.iAtMs = 33u;
            trigger.ProjectileHits = {hit};
            BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = boss.strPatternId; pattern.MechanicTriggers = {trigger};
            KOUKUSAYDON_LOGIC_LEDGER ledger{};
            CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
            room->Update_KoukuPlayerTargets(boss, pattern, ledger, catalog, 100u);
            const auto& objects = room->m_CombatObjectRuntime.Get_LiveObjects();
            tests.Require(objects.size() == 1u && objects.front().eDamageImmuneCardSymbol == static_cast<MECHANIC_CARD_SYMBOL>(symbol),
                "The actual pursuit spawn pins its explicit visual-slot symbol on the Server object");
            std::vector<DAMAGE_EVENT> events;
            room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 101u, events);
            bool correct = events.size() == 4u;
            for (const auto& [id, p] : room->m_Players) {
                const auto expectedCount = (id == symbol || id == 5u) ? 0u : 1u;
                correct = correct && matchesIncomingDamage(p, p.iMaximumHp,
                    p.iMaximumHp * 90u / 100u, expectedCount, events);
            }
            if (!correct) {
                std::cout << "Dice case " << hitKind << " symbol=" << symbol << " color=" << static_cast<unsigned>(color)
                    << " objects=" << objects.size() << " events=" << events.size() << " room=" << room->Get_Status();
                for (const auto& [id, player] : room->m_Players) std::cout << " hp" << id << '=' << player.iCurrentHp;
                std::cout << '\n';
            }
            tests.Require(correct, hit.strTrigger == "CONTACT" ?
                "Contact card preserves suit immunity and applies one sampled 90%-HP baseline hit with matching HP/event totals" :
                "Timed card preserves suit immunity, excludes distant players and applies one sampled 90%-HP baseline hit");
            const auto eventCount = events.size();
            room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 102u, events);
            tests.Require(events.size() == eventCount, "A completed card contact does not deal a duplicate hit on the next tick");
        }
        for (unsigned symbol = 1u; symbol <= 4u; ++symbol)
        for (const bool matching : {false, true})
        for (unsigned ownership = 0u; ownership != 3u; ++ownership) {
            seedPlayers(MECHANIC_CARD_COLOR::BLACK); room->m_CombatObjectRuntime.Reset();
            room->m_CombatObjectRuntime.Discard_PendingLifecycle();
            for (auto& [id, player] : room->m_Players) player.fPositionX = player.fPositionZ = 10000.f;
            auto& interceptor = room->m_Players.at(2u);
            interceptor.fPositionX = interceptor.fPositionZ = 0.f;
            if (ownership == 1u) ++interceptor.iPatternBindOwnerNetEntityId;
            if (ownership == 2u) --interceptor.iPatternBindSequence;
            interceptor.eMechanicCardSymbol = matching ? static_cast<MECHANIC_CARD_SYMBOL>(symbol) : MECHANIC_CARD_SYMBOL::NONE;
            BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
            trigger.strTriggerId = "dice.interceptor"; trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
            trigger.iDurationMs = 1000u; trigger.ProjectileVisualIds = {"test.card"};
            trigger.ProjectileCardSymbols = {static_cast<MECHANIC_CARD_SYMBOL>(symbol)};
            trigger.strContactVisualId = "test.card.burst"; trigger.fProjectileSpeedMps = 1.f;
            trigger.fProjectileContactRadiusM = 1.f; trigger.iProjectileLifetimeMs = 1000u;
            trigger.iProjectileCountPerWave = 1u; trigger.bProjectileHoming = true;
            ATTACK_HIT_TEMPLATE hit{}; hit.strHitId = "card.intercept"; hit.strTrigger = "CONTACT";
            hit.strShape = "CIRCLE"; hit.strDamageKind = "MAX_HP_PERCENT"; hit.iDamagePercent = 90u;
            hit.fRadiusM = 1.; hit.iRepeatCount = 1u; hit.iEndMs = 1000u; trigger.ProjectileHits = {hit};
            BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = boss.strPatternId; pattern.MechanicTriggers = {trigger};
            KOUKUSAYDON_LOGIC_LEDGER ledger{}; CKoukuSaydonLogicRuntime::Build(pattern, boss, 150u, ledger);
            room->Update_KoukuPlayerTargets(boss, pattern, ledger, catalog, 150u);
            const auto& live = room->m_CombatObjectRuntime.Get_LiveObjects();
            tests.Require(live.size() == 1u && live.front().iLockedTargetNetEntityId == 101u,
                "Dice card keeps its designated steering target when a bound bystander intercepts");
            std::vector<DAMAGE_EVENT> damage;
            room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 151u, damage);
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
            std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulses;
            std::vector<S2C_COMBAT_OBJECT_DESPAWNED> retired;
            room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, pulses, retired);
            std::erase_if(pulses, [&](const auto& event) { return event.strHitId != trigger.strContactVisualId; });
            tests.Require(matchesIncomingDamage(interceptor, 2000u, 1800u, matching ? 0u : 1u, damage) &&
                matchesIncomingDamage(room->m_Players.at(1u), 1000u, 900u, 0u, damage) &&
                damage.size() == (matching ? 0u : 1u) && pulses.size() == 1u && retired.size() == 1u && live.empty(),
                "Every suit card retires on interception with exact suit immunity, one sampled wrong-suit hit and one burst");
            tests.Require(interceptor.bPatternBound == (!matching || ownership != 0u),
                "Matching interception releases only its occurrence-owned bind and preserves other suits, foreign owners and other pattern sequences");
        }
        // An ordinary pursuit with no card symbols remains an ordinary authored attack.
        seedPlayers(MECHANIC_CARD_COLOR::RED); room->m_CombatObjectRuntime.Reset();
        BOSS_COMBAT_OBJECT_DEFINITION generic{};
        generic.strEncounterId = boss.strEncounterId; generic.strOwnerPatternId = boss.strPatternId;
        generic.strOwnerStageActionId = "generic.card"; generic.strCombatObjectArchetypeId = "combatobject.kouku.pursuit";
        generic.strClientVisualId = "test.generic"; generic.iLifeMs = 1000u;
        ATTACK_HIT_TEMPLATE ordinary{}; ordinary.strHitId = "ordinary"; ordinary.strTrigger = "CONTACT";
        ordinary.strShape = "CIRCLE"; ordinary.strDamageKind = "MAX_HP_PERCENT"; ordinary.iDamagePercent = 50u;
        ordinary.fRadiusM = 1.; ordinary.iEndMs = 1000u; ordinary.iRepeatCount = 1u; generic.AttackTemplates = {ordinary};
        auto transaction = room->m_CombatObjectRuntime.Begin_Transaction(); std::string status;
        const bool staged = room->m_CombatObjectRuntime.Stage_BossCombatObject(transaction, boss, nullptr, generic, nullptr, catalog, 1u, 200u, status);
        tests.Require(staged && room->m_CombatObjectRuntime.Commit(std::move(transaction)), "Generic pursuit stages without optional symbol data");
        std::vector<DAMAGE_EVENT> events;
        room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, 201u, events);
        bool genericDamageCorrect = events.size() == 5u;
        for (const auto& [id, player] : room->m_Players)
            genericDamageCorrect = genericDamageCorrect && matchesIncomingDamage(player, player.iMaximumHp,
                player.iMaximumHp / 2u, id == 5u ? 0u : 1u, events);
        tests.Require(genericDamageCorrect,
            "Absent cardSymbols applies one sampled ordinary hit to each nearby player regardless of card assignment");

        // Complete Play keeps the real raid owner across P79 -> P78. Exercise
        // two connected humans both with and without a sessionless guide.
        for (const bool withGuide : {false, true})
        {
            auto flowRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
            tests.Require(flowRoom->Is_Ready(), "Complete Play dice fixture admits the installed world");
            if (!flowRoom->Is_Ready()) continue;
            std::vector<std::shared_ptr<CClientSession>> sessions;
            for (PLAYER_ID id = 1u; id <= (withGuide ? 3u : 2u); ++id)
            {
                auto& player = flowRoom->m_Players[id];
                player.iPlayerId = id; player.iNetEntityId = 100u + id;
                player.iCurrentHp = player.iMaximumHp = 100000000u;
                player.isCombatReady = true; player.fMoveSpeed = 5.f;
                player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
                player.fPositionX = 10.f + id; player.fPositionY = .45f; player.fPositionZ = -60.f;
                if (id == 3u) { player.eControlKind = PLAYER_CONTROL_KIND::GUIDE_AI; continue; }
                player.iSessionId = 500u + id;
                flowRoom->m_PlayerIdBySessionId[player.iSessionId] = id;
                auto connection = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
                    CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
                connection->m_isSendRunning.store(true);
                flowRoom->m_Sessions.emplace(player.iSessionId, connection); sessions.push_back(connection);
            }
            C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
            request.iRequestSequence = 1u; request.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; request.strStartGateId = "GATE1";
            request.ExpectedGameplayRevision = flowRoom->m_GameplayCatalog.Get_ActiveRevision();
            request.iActionSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(flowRoom->m_GameplayCatalog.Active());
            const auto* gate = flowRoom->m_GameplayCatalog.Active().Find_KoukuRaidGate("GATE1");
            if (!gate) { tests.Require(false, "Complete Play fixture resolves Gate 1"); continue; }
            request.iSequenceSourceRevision = gate->iSequenceRevision;
            flowRoom->m_iServerTick = 10u;
            std::string reason;
            tests.Require(flowRoom->Begin_KoukuRaidPreparation(501u, request, reason),
                "Complete Play uses real pinned preparation rather than a standalone pattern ledger");
            auto& run = flowRoom->m_KoukuRaid;
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::PREPARING) continue;
            auto ready = request; ready.eOperation = KOUKUSAYDON_RAID_OPERATION::READY;
            ready.iRequestSequence = 2u; ready.iExpectedRunEpoch = run.State.iRunEpoch;
            for (const auto session : {501u, 502u})
                tests.Require(flowRoom->Apply_KoukuRaidReadiness(session, ready, reason), "Both human clients acknowledge Complete Play");
            flowRoom->m_iServerTick = 12u; flowRoom->Update_KoukuRaid(12u);
            tests.Require(run.PlayerIds == std::vector<PLAYER_ID>{1u, 2u} && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC,
                "A guide does not join the human roster or delay the Complete Play cinematic");
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::CINEMATIC) continue;
            flowRoom->m_iServerTick = run.State.iEndTick;
            flowRoom->Update_KoukuRaid(flowRoom->m_iServerTick);
            const auto previous = std::find_if(gate->Entries.begin(), gate->Entries.end(), [](const auto& entry) {
                return entry.strTargetId == "KOUKUSAYDON_G1_PATTERN_79"; });
            tests.Require(run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT && previous != gate->Entries.end() &&
                std::next(previous) != gate->Entries.end() && std::next(previous)->strTargetId == "KOUKUSAYDON_G1_PATTERN_78",
                "The installed Complete Play flow transitions from P79 to the dice pattern");
            if (run.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT || previous == gate->Entries.end()) continue;
            // Skip earlier independent entries, then run all actual ticks of the
            // predecessor, transition, dice opening and replicated bind state.
            run.State.iFlowEntryIndex = static_cast<std::uint32_t>(previous - gate->Entries.begin());
            flowRoom->m_iServerTick = run.iNextEntryTick - 1u;
            bool sawPrevious = false, inspected = false;
            for (unsigned step = 0u; step < 2000u && !inspected; ++step)
            {
                flowRoom->Tick(1.f / 30.f);
                auto* liveBoss = flowRoom->Find_KoukuSaydonArenaBoss("boss.koukusaydon.g1.saydon", "BOSS_KOUKUSAYDON_G1_SAYDON");
                if (!liveBoss || !flowRoom->Is_KoukuRaidRunning()) break;
                sawPrevious |= liveBoss->strPatternId == "KOUKUSAYDON_G1_PATTERN_79";
                if (liveBoss->strPatternId != "KOUKUSAYDON_G1_PATTERN_78") continue;
                const auto* member = flowRoom->Find_KoukuAuditionMember(liveBoss->iNetEntityId, liveBoss->iPatternSequence);
                if (!member || std::none_of(member->LogicLedger.Windows.begin(), member->LogicLedger.Windows.end(),
                    [](const auto& window) { return window.bOpened && !window.bClosed && window.iFreePlayerNetEntityId; })) continue;
                inspected = true;
                unsigned bound = 0u, moving = 0u;
                std::map<PLAYER_ID, std::array<float, 2u>> beforeMove;
                for (const auto id : {1u, 2u})
                {
                    auto& player = flowRoom->m_Players.at(id); bound += player.bPatternBound ? 1u : 0u;
                    beforeMove.emplace(id, std::array{player.fPositionX, player.fPositionZ});
                    C2S_MOVE move; move.iClientSequence = 1u; move.fGoalX = player.fPositionX + 1.f; move.fGoalZ = player.fPositionZ;
                    flowRoom->Handle_Move(500u + id, move); moving += player.hasMoveGoal ? 1u : 0u;
                    std::cout << "[DiceCompletePlay] guide=" << withGuide << " player=" << id << " bound=" << player.bPatternBound
                        << " ready=" << player.isCombatReady << " action=" << static_cast<unsigned>(player.eAction)
                        << " move=" << player.hasMoveGoal << " free=" << liveBoss->iPatternTargetEntityId << '\n';
                }
                tests.Require(sawPrevious && run.State.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT &&
                    !flowRoom->Is_KoukuRaidInputBlocked() && bound == 1u && moving == 1u &&
                    (liveBoss->iPatternTargetEntityId == 101u || liveBoss->iPatternTargetEntityId == 102u) &&
                    (!withGuide || !flowRoom->m_Players.at(3u).bPatternBound),
                    "Complete Play P79-to-P78 admits one free human mover and one bound human, never the guide");
                for (unsigned step = 0u; step < 4u; ++step) flowRoom->Tick(1.f / 30.f);
                unsigned advanced = 0u; bool captiveStayed = true;
                for (const auto id : {1u, 2u})
                {
                    const auto& player = flowRoom->m_Players.at(id);
                    const auto& before = beforeMove.at(id);
                    const bool changed = std::hypot(player.fPositionX - before[0], player.fPositionZ - before[1]) > .01f;
                    advanced += changed ? 1u : 0u;
                    captiveStayed = captiveStayed && (!player.bPatternBound || !changed);
                }
                tests.Require(advanced == 1u && captiveStayed,
                    "Real Complete Play navigation and world collision advance only the free human across four ticks");
                flowRoom->Broadcast_WorldSnapshot();
                for (const auto& connection : sessions)
                {
                    const auto frame = std::find_if(connection->m_OutboundFrames.rbegin(), connection->m_OutboundFrames.rend(),
                        [](const auto& item) { return item.ePacketType == PACKET_TYPE::S2C_WORLD_SNAPSHOT; });
                    S2C_WORLD_SNAPSHOT decoded; bool parsed = frame != connection->m_OutboundFrames.rend();
                    if (parsed) {
                        CPacketReader reader{std::span<const std::uint8_t>(frame->Bytes.data() + PACKET_HEADER_BYTES,
                            frame->Bytes.size() - PACKET_HEADER_BYTES)};
                        parsed = Read_Message(reader, decoded);
                    }
                    std::cout << "[DiceCompletePlayWire] parsed=" << parsed << " snapshot=" << (frame != connection->m_OutboundFrames.rend())
                        << " frames=" << connection->m_OutboundFrames.size() << " tick=" << decoded.iServerTick
                        << " expected=" << flowRoom->m_iServerTick << " players=" << decoded.Players.size() << " entities=" << decoded.Entities.size() << '\n';
                    for (const auto& player : decoded.Players) std::cout << "[DiceWirePlayer] id=" << player.iNetEntityId << " bound=" << player.isPatternBound << " kind=" << static_cast<unsigned>(player.eControlKind) << '\n';
                    for (const auto& entity : decoded.Entities) if (entity.hasBossCombatState) std::cout << "[DiceWireBoss] id=" << entity.iNetEntityId << " pattern=" << entity.strPatternId << '\n';
                    tests.Require(parsed && decoded.iServerTick == flowRoom->m_iServerTick &&
                        std::count_if(decoded.Players.begin(), decoded.Players.end(), [](const auto& player) {
                            return player.eControlKind == PLAYER_CONTROL_KIND::HUMAN && player.isPatternBound; }) == 1 &&
                        std::any_of(decoded.Entities.begin(), decoded.Entities.end(), [](const auto& entity) {
                            return entity.strPatternId == "KOUKUSAYDON_G1_PATTERN_78"; }),
                        "Both Client snapshots carry exactly one human bind and the live dice pattern for floor-effect admission");
                }
            }
            tests.Require(inspected, "Real Complete Play ticks reach the authored dice bind window");
            if (!inspected) std::cout << "[DiceCompletePlay] status=" << flowRoom->Get_Status() << " phase=" << static_cast<unsigned>(run.State.ePhase) << '\n';
            for (const auto& connection : sessions) connection->Request_Close();
        }

        // Use the installed P78 definition, its authored clock, actual player update and pursuit spawn.
        const auto* installed = catalog.Find_BossPatterns(boss.strEncounterId);
        const BOSS_PATTERN_DEFINITION* dice = nullptr;
        if (installed) for (const auto& candidate : *installed)
            if (candidate.strPatternId == "KOUKUSAYDON_G1_PATTERN_78") dice = &candidate;
        const BOSS_PATTERN_LOGIC_WINDOW* binding = nullptr;
        if (dice) for (const auto& window : dice->LogicWindows)
            if (window.eKind == BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND) binding = &window;
        const auto* spawn = room->Find_AvailablePlayerSpawn();
        tests.Require(dice && binding && spawn, "Published P78 exposes its typed Dice binding and a real navigation spawn");
        if (dice && binding && spawn) {
            boss.strPatternId = dice->strPatternId;
            const auto start = 100u + CKoukuSaydonLogicRuntime::Ticks_FromMs(binding->iStartMs);
            const auto end = start + CKoukuSaydonLogicRuntime::Ticks_FromMs(binding->iDurationMs);
            const auto seedRoster = [&](unsigned count) {
                seedPlayers(MECHANIC_CARD_COLOR::BLACK);
                std::erase_if(room->m_Players, [count](const auto& row) { return row.first > count; });
                room->m_PlayerIdBySessionId.clear();
                for (auto& [id, player] : room->m_Players) {
                    player.Clear_PatternBindStatus(); player.fMoveSpeed = 5.f;
                    player.fPositionX = spawn->fPositionX; player.fPositionY = spawn->fPositionY; player.fPositionZ = spawn->fPositionZ;
                    room->m_PlayerIdBySessionId[500u + id] = id;
                }
                room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
                room->m_KoukuSaydonPatternAudition.Members.clear();
                room->m_KoukuSaydonPatternAudition.Members.emplace_back();
                auto& member = room->m_KoukuSaydonPatternAudition.Members.back();
                member.iBossEntityId = boss.iNetEntityId; member.iPatternSequence = boss.iPatternSequence;
                CKoukuSaydonLogicRuntime::Build(*dice, boss, 100u, member.LogicLedger);
            };
            const auto boundCount = [&]() { return std::count_if(room->m_Players.begin(), room->m_Players.end(),
                [](const auto& row) { return row.second.bPatternBound; }); };
            for (unsigned count = 1u; count <= 4u; ++count) {
                boss.iPatternSequence = 800u + count;
                seedRoster(count);
                auto& ledger = room->m_KoukuSaydonPatternAudition.Members.back().LogicLedger;
                KOUKUSAYDON_LOGIC_OUTPUT output; std::vector<DAMAGE_EVENT> damage;
                const auto update = [&](unsigned tick) { CKoukuSaydonLogicRuntime::Update(boss, *dice,
                    ledger, room->m_Players, catalog, nullptr, tick, damage, output, &room->m_ServerNavigation); };
                update(start - 1u);
                tests.Require(boundCount() == 0, "Installed Dice does not bind before the exact authored start tick");
                update(start);
                const auto freeId = boss.iPatternTargetEntityId;
                auto free = std::find_if(room->m_Players.begin(), room->m_Players.end(),
                    [freeId](const auto& row) { return row.second.iNetEntityId == freeId; });
                tests.Require(boundCount() == count - 1u && free != room->m_Players.end() && !free->second.bPatternBound,
                    "Installed P78 selects one free participant and binds N minus one for solo through four players");
                room->m_iServerTick = start;
                room->Update_Players(1.f / 30.f); room->m_iServerTick = start + 1u; room->Update_Players(1.f / 30.f);
                unsigned moving = 0u;
                for (auto& [id, player] : room->m_Players) {
                    C2S_MOVE move{}; move.iClientSequence = 1u;
                    move.fGoalX = spawn->fPositionX + 1.f; move.fGoalZ = spawn->fPositionZ;
                    room->Handle_Move(500u + id, move);
                    moving += player.hasMoveGoal ? 1u : 0u;
                }
                tests.Require(boundCount() == count - 1u && moving == 1u &&
                    std::all_of(room->m_Players.begin(), room->m_Players.end(), [](const auto& row) { return row.second.isCombatReady; }),
                    "Two real player ticks and C2S_MOVE admit exactly the one free mover and retain every captive's damage eligibility");
                const auto pursuit = std::find_if(dice->MechanicTriggers.begin(), dice->MechanicTriggers.end(), [](const auto& trigger) {
                    return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES && !trigger.ProjectileCardSymbols.empty(); });
                tests.Require(pursuit != dice->MechanicTriggers.end(), "Published Dice has its authored suit pursuit");
                if (pursuit == dice->MechanicTriggers.end() || free == room->m_Players.end()) continue;
                const auto pursuitTick = 100u + CKoukuSaydonLogicRuntime::Ticks_FromMs(pursuit->iStartMs);
                update(pursuitTick); room->Update_KoukuPlayerTargets(boss, *dice, ledger, catalog, pursuitTick);
                const auto& cards = room->m_CombatObjectRuntime.Get_LiveObjects();
                tests.Require(cards.size() == 1u && cards.front().iLockedTargetNetEntityId == freeId,
                    "Actual published Dice pursuit steers toward the same sole free participant");
                if (cards.empty()) continue;
                auto victim = count == 1u ? free : std::find_if(room->m_Players.begin(), room->m_Players.end(),
                    [](const auto& row) { return row.second.bPatternBound; });
                for (auto& [id, player] : room->m_Players) { player.fPositionX = 10000.f; player.fPositionZ = 10000.f; }
                victim->second.fPositionX = cards.front().LiveState.CurrentPose.fPositionX;
                victim->second.fPositionZ = cards.front().LiveState.CurrentPose.fPositionZ;
                victim->second.eMechanicCardSymbol = cards.front().eDamageImmuneCardSymbol;
                const auto hp = victim->second.iCurrentHp;
                room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, catalog, .034f, pursuitTick + 1u, damage);
                tests.Require(victim->second.iCurrentHp == hp && !victim->second.bPatternBound && damage.empty() && cards.empty(),
                    "Actual P78 card is intercepted without damage and releases a matching captive through the production contact path");
                update(pursuitTick + 2u);
                tests.Require(boundCount() == (count > 1u ? count - 2u : 0u) && boss.iPatternTargetEntityId == freeId,
                    "A rescued captive stays released and does not replace the original card steering target");
                update(end);
                tests.Require(boundCount() == 0u, "The exact published end tick clears all remaining owned Dice binds");
            }
            boss.iPatternSequence = 900u; seedRoster(4u);
            SERVER_WORLD_ENTITY foreignOwner = boss; foreignOwner.iNetEntityId = 999u; foreignOwner.iPatternSequence = 77u;
            room->m_WorldEntities.push_back(foreignOwner);
            for (auto& [id, player] : room->m_Players) {
                player.bPatternBound = true; player.iPatternBindOwnerNetEntityId = 999u; player.iPatternBindSequence = 77u;
                player.iPatternBindEndTick = end + 100u; player.bPatternBindRestoreCombatReady = true;
                player.fPatternBindRestoreX = player.fPositionX; player.fPatternBindRestoreY = player.fPositionY;
                player.fPatternBindRestoreZ = player.fPositionZ;
            }
            auto& ledger = room->m_KoukuSaydonPatternAudition.Members.back().LogicLedger;
            KOUKUSAYDON_LOGIC_OUTPUT output; std::vector<DAMAGE_EVENT> damage;
            CKoukuSaydonLogicRuntime::Update(boss, *dice, ledger, room->m_Players, catalog, nullptr, start, damage, output);
            room->m_iServerTick = start; room->Update_Players(1.f / 30.f);
            const bool noSelectedFree = std::none_of(ledger.Windows.begin(), ledger.Windows.end(),
                [](const auto& window) { return window.iFreePlayerNetEntityId != INVALID_NET_ENTITY_ID; });
            tests.Require(boundCount() == 4u && noSelectedFree && std::all_of(room->m_Players.begin(), room->m_Players.end(),
                [](const auto& row) { return row.second.iPatternBindOwnerNetEntityId == 999u; }),
                "Four pre-existing live foreign binds remain foreign: Dice has zero eligible players and creates no binding or free target");
            room->m_WorldEntities.back().iPatternSequence = 78u;
            room->m_iServerTick = start + 1u; room->Update_Players(1.f / 30.f);
            tests.Require(boundCount() == 0u, "The next player tick retires all four stale foreign occurrences instead of retaining an accidental Dice lock");
            room->m_WorldEntities.pop_back(); room->m_KoukuSaydonPatternAudition.Members.clear();
        }

        // 고정 피해의 기준량은 최대 HP와 독립적이며, 일반 피격의 ±10% 표본·접촉 시계·안전 지대는 공유한다.
        BOSS_PATTERN_LOGIC_RESULT fixed{}; fixed.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE; fixed.iDamageAmount = 500u;
        BOSS_PATTERN_LOGIC_WINDOW contact{}; contact.strWindowId = "fixed.tick";
        contact.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; contact.iDurationMs = 1000u; contact.iRepeatIntervalMs = 500u;
        contact.OnSuccess = {fixed}; BOSS_LOGIC_REGION circle{}; circle.strRegionId = "fixed.region";
        circle.bCircle = true; circle.fRadiusM = 5.f; circle.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD;
        contact.CardRegions = {circle}; BOSS_PATTERN_DEFINITION fixedPattern{};
        fixedPattern.strPatternId = boss.strPatternId; fixedPattern.LogicWindows = {contact};
        seedPlayers(MECHANIC_CARD_COLOR::RED); room->m_Players.erase(1u); room->m_Players.erase(2u);
        KOUKUSAYDON_LOGIC_LEDGER fixedLedger{}; KOUKUSAYDON_LOGIC_OUTPUT output{}; events.clear();
        CKoukuSaydonLogicRuntime::Build(fixedPattern, boss, 300u, fixedLedger);
        bool fixedClockCorrect = true;
        for (unsigned tick : {300u, 300u, 314u, 315u, 330u}) {
            CKoukuSaydonLogicRuntime::Update(boss, fixedPattern, fixedLedger, room->m_Players, catalog, nullptr, tick, events, output);
            const auto expectedCount = tick < 315u ? 1u : 2u;
            fixedClockCorrect = fixedClockCorrect && events.size() == expectedCount * 3u;
            for (const PLAYER_ID id : {3u, 4u, 6u})
                fixedClockCorrect = fixedClockCorrect && matchesIncomingDamage(room->m_Players.at(id),
                    1000u * id, 500u, expectedCount, events);
            fixedClockCorrect = fixedClockCorrect && matchesIncomingDamage(room->m_Players.at(5u),
                5000u, 500u, 0u, events);
        }
        tests.Require(fixedClockCorrect,
            "Fixed fire uses a 500 baseline with per-hit variation only at entry and 500 ms, never duplicate or end ticks");
        auto zone = contact; zone.strWindowId = "fixed.safe"; zone.eKind = BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE;
        zone.iRepeatIntervalMs = 0u; zone.OnSuccess.clear(); fixedPattern.LogicWindows.push_back(zone);
        seedPlayers(MECHANIC_CARD_COLOR::RED); events.clear(); output = {};
        CKoukuSaydonLogicRuntime::Build(fixedPattern, boss, 400u, fixedLedger);
        CKoukuSaydonLogicRuntime::Update(boss, fixedPattern, fixedLedger, room->m_Players, catalog, nullptr, 400u, events, output);
        tests.Require(events.empty() && room->m_Players.at(1u).iCurrentHp == 1000u,
            "Same-pattern safe zones suppress fixed damage through the common result gate");

        std::array<wchar_t, 32768u> candidatePath{};
        const auto length = GetEnvironmentVariableW(L"LOSTARK_KOUKU_DICE_TEST_ROWS", candidatePath.data(), static_cast<DWORD>(candidatePath.size()));
        if (length && length < candidatePath.size()) {
            const std::filesystem::path path(candidatePath.data()); std::ifstream input(path, std::ios::binary);
            const std::string rows((std::istreambuf_iterator<char>(input)), {});
            GameplayDataRevision hash{}; CGameplayCatalog draft;
            const bool admitted = !rows.empty() && CServerApp::Hash_GameplayFileForAdmission(path, hash, status) && draft.Load_DraftKoukuProduct(catalog, rows, hash);
            if (!admitted) std::cout << "Dice draft: " << draft.Get_Status() << '\n';
            tests.Require(admitted, "Actual prepared Dice rows pass the native transactional draft parser");
            if (admitted) {
                const auto* patterns = draft.Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1");
                std::set<MECHANIC_CARD_SYMBOL> exact;
                if (patterns) for (const auto& pattern : *patterns) if (pattern.strPatternId == "KOUKUSAYDON_G1_PATTERN_78")
                    for (const auto& trigger : pattern.MechanicTriggers)
                        if (trigger.ProjectileCardSymbols.size() == 1u && trigger.ProjectileHits.size() == 1u &&
                            trigger.ProjectileHits.front().iDamagePercent == 90u)
                            exact.insert(trigger.ProjectileCardSymbols.front());
                tests.Require(exact == std::set<MECHANIC_CARD_SYMBOL>{MECHANIC_CARD_SYMBOL::HEART, MECHANIC_CARD_SYMBOL::SPADE, MECHANIC_CARD_SYMBOL::CLUB, MECHANIC_CARD_SYMBOL::DIAMOND},
                    "Native P78 parser retains all four authored suit cards and 90% maximum-HP contact damage");
            }
        }
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(opaque) = tests.failures ? 1 : 0;
    };
    if (!Run_WithContractWorkerStack(execute, &result)) return 1;
    return result;
}

int CServerGameplayContractRunner::Run_KoukuJokerAimContracts()
{
    int result = 1;
    const auto execute = [](void* opaque) {
        TESTS tests;
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Joker aim fixture loads the actual published world and Product");
        if (!room->Is_Ready()) { *static_cast<int*>(opaque) = 1; return; }
        auto& catalog = room->m_GameplayCatalog;
        std::string status;
        const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(catalog.Active(), "KOUKUSAYDON_G1_PATTERN_13", status);
        std::vector<const BOSS_PATTERN_MECHANIC_TRIGGER*> selections;
        if (pattern) for (const auto& trigger : pattern->MechanicTriggers)
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION) selections.push_back(&trigger);
        tests.Require(selections.size() == 3u && selections[0]->iStartMs == 0u && selections[0]->iDurationMs == 2658u &&
            selections[1]->iStartMs == 6244u && selections[1]->iDurationMs == 2658u &&
            selections[2]->iStartMs == 12587u && selections[2]->iDurationMs == 5323u,
            "Published P13 preserves all three authored Duration boundaries 0-2658, 6244-8902 and 12587-17910 ms");
        if (selections.size() != 3u) { *static_cast<int*>(opaque) = 1; return; }
        const auto* placement = room->Find_Placement("boss.koukusaydon.g2.big-saydon");
        SERVER_WORLD_ENTITY entity;
        const bool spawned = placement && room->Build_WorldEntity(*placement, room->m_iNextNetEntityId, entity);
        tests.Require(spawned, "Joker uses its actual Gate 2 big Saydon body");
        if (!spawned) { *static_cast<int*>(opaque) = 1; return; }
        ++room->m_iNextNetEntityId; room->m_WorldEntities.push_back(std::move(entity));
        const auto bossId = room->m_WorldEntities.back().iNetEntityId;
        const auto getBoss = [&]() -> SERVER_WORLD_ENTITY* {
            const auto found = std::find_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(),
                [bossId](const auto& boss) { return boss.iNetEntityId == bossId; });
            return found == room->m_WorldEntities.end() ? nullptr : &*found;
        };
        getBoss()->fYawDegrees = 27.f;
        auto& player = room->m_Players[941u]; player.iPlayerId = player.iNetEntityId = 941u;
        player.iCurrentHp = player.iMaximumHp = 1000u; player.isCombatReady = true;
        player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; player.iInvulnerableEndTick = 5000u;
        player.fPositionX = getBoss()->fPositionX + 15.f; player.fPositionY = getBoss()->fPositionY; player.fPositionZ = getBoss()->fPositionZ;
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request;
        request.iRequestSequence = 1u; request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
        request.strPatternId = pattern->strPatternId; request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
        request.Scope.strEncounterId = pattern->strEncounterId; request.Scope.strGateId = "GATE2";
        request.Scope.strBossPlacementId = placement->strPlacementId; request.Scope.strBossArchetypeId = placement->strArchetypeId;
        request.Scope.ExpectedGameplayRevision = catalog.Get_ActiveRevision();
        request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(catalog.Active());
        S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT admitted;
        tests.Require(room->Evaluate_KoukuSaydonPatternAudition(941u, request, admitted) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED,
            "The actual P13 audition admits its published target selection and hammer stages");
        const auto tick = [&]() { room->Update_WorldEntities(1.f / 30.f); ++room->m_iServerTick; };
        const auto advance = [&](unsigned target) { while (room->m_iServerTick < target) tick(); };
        const auto close = [](float a, float b) { return std::abs(std::remainder(a - b, 360.f)) < .001f; };
        tick();
        auto* boss = getBoss();
        if (!boss || boss->strPatternId != pattern->strPatternId) { tests.Require(false, "P13 starts on the actual room tick"); *static_cast<int*>(opaque) = 1; return; }
        const auto sequence = boss->iPatternSequence, start = boss->iPatternStartTick;
        const auto initialYaw = boss->fYawDegrees;
        const std::array<unsigned, 3u> impactMs{4855u, 11225u, 20212u};
        for (unsigned index = 0u; index < selections.size(); ++index) {
            const auto& selection = *selections[index];
            const auto begin = start + CKoukuSaydonLogicRuntime::Ticks_FromMs(selection.iStartMs);
            const auto end = start + CKoukuSaydonLogicRuntime::Ticks_FromMs(selection.iStartMs + selection.iDurationMs);
            advance(begin); boss = getBoss();
            tests.Require(boss && boss->iPatternTargetEntityId == player.iNetEntityId && !boss->Has_KoukuPresentationAim() && close(boss->fYawDegrees, initialYaw),
                "Each authored Duration start retains the selected identity and restores the preceding body yaw");
            if (!boss) break;
            player.fPositionX = boss->fPositionX + 8.f; player.fPositionZ = boss->fPositionZ - 7.f;
            advance(end - 1u); boss = getBoss();
            tests.Require(boss && !boss->Has_KoukuPresentationAim() && close(boss->fYawDegrees, initialYaw),
                "Moving the selected player throughout the Duration does not turn the boss before END");
            if (!boss) break;
            const std::array<float, 3u> aim{boss->fPositionX - 7.f - index, player.fPositionY, boss->fPositionZ + 12.f};
            player.fPositionX = aim[0]; player.fPositionY = aim[1]; player.fPositionZ = aim[2];
            advance(end); boss = getBoss();
            const auto expectedYaw = std::atan2(aim[0] - boss->fPositionX, aim[2] - boss->fPositionZ) * 57.295779513f - 90.f;
            tests.Require(boss->Has_KoukuPresentationAim() && boss->iPatternSequence == sequence &&
                boss->fPatternTargetLastPositionX == aim[0] && boss->fPatternTargetLastPositionY == aim[1] &&
                boss->fPatternTargetLastPositionZ == aim[2] && close(boss->fYawDegrees, expectedYaw),
                "The exact END tick captures current selected XYZ once and publishes the model-X aim through the unchanged occurrence yaw");
            const auto pinYaw = boss->fYawDegrees;
            player.fPositionX += 25.f; player.fPositionZ -= 30.f;
            advance(start + CKoukuSaydonLogicRuntime::Ticks_FromMs(impactMs[index]) + 1u); boss = getBoss();
            tests.Require(boss && boss->Has_KoukuPresentationAim() && boss->iPatternSequence == sequence &&
                boss->fPatternTargetLastPositionX == aim[0] && boss->fPatternTargetLastPositionZ == aim[2] && close(boss->fYawDegrees, pinYaw),
                "The real room and later hammer attack stage preserve the END aim while the player moves elsewhere");
            std::cout << "[JOKER END AIM] window=" << selection.strTriggerId << " start=" << begin << " end=" << end
                << " impact=" << room->m_iServerTick << " yaw=" << pinYaw << " original=" << initialYaw << '\n';
        }
        boss = getBoss();
        if (boss) {
            auto completed = *boss; room->m_KoukuSaydonBrain.Complete_Pattern(completed, room->m_iServerTick);
            tests.Require(!completed.Has_KoukuPresentationAim() && close(completed.fYawDegrees, initialYaw),
                "Natural pattern completion releases the occurrence aim and restores the original yaw");
            room->Clear_KoukuSaydonPatternAudition(false, "Joker aim cancellation contract"); boss = getBoss();
            tests.Require(boss && !boss->Has_KoukuPresentationAim() && close(boss->fYawDegrees, initialYaw),
                "Actual room cancellation and Brain Abort restore yaw before another pattern starts");
            if (boss) {
                tests.Require(room->m_KoukuSaydonBrain.Begin_Pattern(*boss, *pattern, catalog.Get_ActiveRevision(), room->m_iServerTick + 1u, status) &&
                    !boss->Has_KoukuPresentationAim() && close(boss->fYawDegrees, initialYaw),
                    "The next pattern begins in the original basis with no inherited Joker aim");
                KOUKUSAYDON_LOGIC_LEDGER ledger; CKoukuSaydonLogicRuntime::Build(*pattern, *boss, 2000u, ledger);
                auto& window = ledger.PlayerTargetWindows.front(); window.iSelectedTargetNetEntityId = player.iNetEntityId;
                window.LastSelectedTargetPosition = std::array<float, 3u>{boss->fPositionX + 4.f, boss->fPositionY, boss->fPositionZ + 9.f};
                const auto last = *window.LastSelectedTargetPosition;
                player.iCurrentHp = 0u; player.fPositionX += 99.f;
                SERVER_PLAYER other; other.iPlayerId = other.iNetEntityId = 942u; other.iCurrentHp = 100u; other.isCombatReady = true;
                room->m_Players.emplace(other.iPlayerId, other);
                room->Update_KoukuPlayerTargets(*boss, *pattern, ledger, catalog, window.iEndTick);
                tests.Require(boss->iPatternTargetEntityId == 941u && boss->fPatternTargetLastPositionX == last[0] &&
                    boss->fPatternTargetLastPositionZ == last[2] && boss->Has_KoukuPresentationAim(),
                    "A dead selected target retains its last valid aim point instead of selecting another living player");
                boss->Restore_KoukuPresentationAim();
                CKoukuSaydonLogicRuntime::Build(*pattern, *boss, 3000u, ledger);
                auto& departed = ledger.PlayerTargetWindows.front(); departed.iSelectedTargetNetEntityId = 941u; departed.LastSelectedTargetPosition = last;
                room->m_Players.erase(941u);
                room->Update_KoukuPlayerTargets(*boss, *pattern, ledger, catalog, departed.iEndTick);
                tests.Require(boss->iPatternTargetEntityId == 941u && boss->fPatternTargetLastPositionX == last[0] &&
                    boss->fPatternTargetLastPositionZ == last[2] && boss->Has_KoukuPresentationAim(),
                    "A departed selected target retains the same last valid point without retargeting");
            }
        }
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(opaque) = tests.failures ? 1 : 0;
    };
    return Run_WithContractWorkerStack(execute, &result) ? result : 1;
}
```

### G07-04. 적용과 검증

1. 제품 파일 SHA가 `7f229ba81cf9d46c89d34f5c0fc85e8d799cf003e18af9e95b2f770100a07224`인지 적용 직전에 다시 확인한다. 달라졌으면 후보 전체를 덮어쓰지 말고 변경 블록을 최신 저장본과 병합한다. 원본은 out 후보 폴더에 보관하며 적용은 부모가 정한 빌드 사이 시점에 수행한다.
2. 후보 diff·인코딩·project 등록 유지와 독립 검토를 확인한다. 후보의 함수 외 영역은 바이트 불변이어야 한다.
3. 해당 Server를 Release와 Debug로 다시 빌드한 뒤 새 EXE를 복사 격리해 `--kouku-dice-hit-contract-test`를 실행한다. 같은 EXE의 raid/product/bundle/cardmaze/bingo 및 showtime/overlap, Debug support는 부모의 메모리 제한·순차 실행 계획을 따른다.
4. 통과 수·실패 수·종료 코드·EXE/DataFiles SHA와 per-hit 사건/타이밍 증거를 RESULT에 기록한다. Release bundle2개는 bootstrap만 실행하므로 본문 성공으로 세지 않는다. 현재 실패30개를 원인 확인 없이 성공으로 덮어쓰지 않는다.
5. native 검사 통과는 Client 화면·음향·연출·실제 사용자 플레이 성공을 의미하지 않는다. 해당 수동 확인 경계는 G05를 유지한다.


## G08. Support native fixture의 현재 무력화·피격 계약 검사

### G08-01. 실제 관측과 완료 경계

기존 Debug Server SHA `356520682bce380a7d5ef634f75a2c1b8a27898f570c9cedcd14bc799045fd2a`를 out에 복사 격리해 `--kouku-support-surface-contract-test`를 실행했다. 현재 Server/Shared C++·header182개와 주요 obj7개의 시각을 대조했고, 해당 EXE 이후의 변경은 G07 Dice 테스트뿐이었다. 이 실행은 최종 Debug 완료 증거가 아니라 이전 fixture의 오류 재현용이다.

493.547초까지322 PASS·12 FAILURE가 실제 로그에 나타났지만 시스템 free commit이1516.1MiB까지 내려가 실행기가 자신이 만든 support 프로세스만 중단했다. 최종 failure footer에 도달하지 않았으므로 전체 suite 완료나 전체 assertion 수를 주장하지 않는다. published 원본과 격리 DataFiles·격리 EXE는 불변이며, 원본 Debug EXE의 후속 변경은 병행 Product 빌드 결과다. 관련 원본 증거는 `out/KoukuArenaReview20261014/runs/debug-pre-final-support-repro/summary.json`과 `support-variation-candidate/partial-native-repro.json`이다.

처음3개 gauge와 Mario 후속5개 실패는 HP 감소를 무력화 누적으로 간주한 이전 fixture 전제에서 발생한다. 현재 catalog의 RAIDSTAGGER50000이 authored fallback115000/1150000보다 우선하고 Logic은 `iKoukuItemStaggerCredit`만 소비한다. HP 차감으로 credit가 채워지지 않으므로 Mario 성공·owner 정리·P42 진입이 연쇄적으로 성립하지 않았다. 현재 및 재현 복사본의 bootstrap SHA가 같으며 P42의1167+1667+1333=4167ms는 앞 scenario에서 이미 통과했다. 이 계약은2026-09-30 RAID_GAMEPLAY_REPAIR 및 MECHANIC_FAILURE_TIMER RESULT에도 기록되어 있다.

뒤4개 실패는 late Logic의 fresh HP100·10% 기준 타격을 HP90 고정값으로 비교하는 두 assertion이 explicitStop 두 경우에서 반복된 것이다. 현재 일반 피격의 실제 허용량은9..11이다. 이전 로그는 개별 HP/event 수치를 출력하지 않으므로 특정 관측 피해량을 만들어 적지 않는다. 아래 후보의 대상별 event·HP·serial 검사를 실제 새 EXE에서 통과시켜 확인한다.

### G08-02. 수정 파일과 의미 단위

수정 대상은 `C:/Users/tnest/Desktop/LostArk/Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp` 한 파일이다. 제품 피해·무력화 계산, 저작·게시 데이터, 옵션, Client 표현은 변경하지 않는다. 기존 등록 파일이므로 `.vcxproj`·`.filters` 추가도 없다. `ServerCombatHitRuntime.h`를 직접 include하여 이미 승인된 typed hit을 공용 처리기로 전달한다.

`Run_MechanicGaugeContracts`는 순수 Logic 단위 fixture다. active catalog maximum을 표시하는지, HP만 깎아서는 credit가 생기지 않는지, credit가 maximum-1일 때 미완료이고 maximum에 도달할 때 한 번만 완료되는지 검사한다. 이 부분은 의도적으로 credit 입력을 명시하며 실제 아이템 hit 성공으로 설명하지 않는다.

Mario fixture는 설치된 `BATTLE_WHIRLWIND_GRENADE`의 skill/divisor3을 읽고 `Apply_PlayerToWorld`에 raw HP0의 승인된 hit3개를 보낸다. 매 hit의 capped credit와 outgoing event 합, HP10000 불변, 처음2hit 미완료를 확인한다. 그 뒤 기존 Logic의0피해 성공 event·출처 없는 성공 판정·단발·followup·owner별 World 정리·P42 전체4167ms 관찰은 유지한다. 아이템 사용 승인·수량·cooldown·발사체 도형은 이 fixture의 범위가 아니다.

late Logic은 현재 fixture가 소유한 player와 `m_TickDamageEvents`를 lambda가 읽기만 차용한다. 각 대상1개의 incoming NORMAL event가9..11이고 합이100-currentHP와 같으며 serial1인지, 전체 event가2개인지 검사한다. 다음10181 tick에서도 같은 누적 상태를 요구한다. World clock, 이전 sequence 보존, 새 body 위치와 summon 수·소유권·수명 assertion은 그대로 둔다.

### G08-03. 전체 C++ 후보

원본 SHA `ea93c2974b92e92d8bb9bb16537fdecce85235b8333a7a352227d93aa44c443b`, 통합 후보 SHA `94a3649bb3d112a10ad34fc356bc4a6e90780db18b7a6a451623355e2d7747ee`다. UTF-8 BOM 없음·CRLF를 보존했다. 무력화 후보는 lifecycle 작업이, 피격 후보는 gates12 작업이 서로 독립 정적 검토했으며 현재 제품 소스에는 아직 적용하지 않았다.

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "KoukuSaydonBrain.h"
#include "BossCombatRuntime.h"
#include "ServerCombatHitRuntime.h"
#include "GameRoom.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerNavigation.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;



namespace
{
    void Run_MechanicGaugeContracts(TESTS& tests)
    {
        CGameplayCatalog catalog;
        if (!catalog.Load()) { tests.Require(false, "Load gameplay for mechanic gauge verdict contracts"); return; }
        BOSS_PATTERN_DEFINITION pattern;
        pattern.strPatternId = "KOUKUSAYDON_GAUGE_CONTRACT";
        BOSS_PATTERN_LOGIC_WINDOW window;
        window.strWindowId = "gauge.stagger"; window.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
        window.iStartMs = 1000u; window.iDurationMs = 2000u; window.iThreshold = 115000u;
        window.bEndsPatternOnSuccess = true; pattern.LogicWindows.push_back(window);
        SERVER_WORLD_ENTITY boss;
        boss.iNetEntityId = 421u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.strPatternId = pattern.strPatternId; boss.iPatternSequence = 7u;
        boss.iCurrentHp = boss.iMaximumHp = 1000000u;
        KOUKUSAYDON_LOGIC_LEDGER ledger; BOSS_COMBAT_SNAPSHOT view;
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
        std::map<PLAYER_ID, SERVER_PLAYER> players; std::vector<DAMAGE_EVENT> damage;
        KOUKUSAYDON_LOGIC_OUTPUT output;
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, 129u, view);
        tests.Require(view.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::NONE,
            "Stagger gauge stays hidden before the authoritative window opens");
        CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, 130u, damage, output);
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, 130u, view);
        const auto commonMaximum = catalog.Get_RaidStaggerMaximum();
        if (!commonMaximum) { tests.Require(false, "The active raid stagger policy must be nonzero"); return; }
        tests.Require(view.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::STAGGER &&
            view.iCurrentMechanicGauge == commonMaximum && view.iMaximumMechanicGauge == commonMaximum &&
            boss.iKoukuItemStaggerMaximum == commonMaximum && boss.iKoukuItemStaggerCredit == 0u,
            "Stagger opens with the active common maximum and zero independent credit");
        // 이 함수는 Logic 판정의 단위 경계다. HP 변화가 credit 입력을 대신하지 못함을 확인한 뒤,
        // 호출자가 누적한 credit의 문턱 직전/도달 상태를 명시한다. 실제 명중 입력은 아래 Mario fixture가 검증한다.
        boss.iCurrentHp -= 4u * 23000u;
        CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, 131u, damage, output);
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, 131u, view);
        tests.Require(view.iCurrentMechanicGauge == commonMaximum && boss.iKoukuItemStaggerCredit == 0u &&
            !output.bStaggerSuccess && !output.bEndPatternEarly && damage.empty(),
            "HP loss alone never contributes to the independent stagger gauge or verdict");
        const auto hpBeforeCredit = boss.iCurrentHp;
        boss.iKoukuItemStaggerCredit = commonMaximum - 1u;
        CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, 132u, damage, output);
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, 132u, view);
        tests.Require(view.iCurrentMechanicGauge == 1u && !output.bStaggerSuccess &&
            !output.bEndPatternEarly && damage.empty() && boss.iCurrentHp == hpBeforeCredit,
            "One credit below the active maximum preserves the open window without HP damage or success");
        ++boss.iKoukuItemStaggerCredit;
        CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, 133u, damage, output);
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, 133u, view);
        tests.Require(output.bStaggerSuccess && output.bEndPatternEarly &&
            view.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::NONE && ledger.Windows.front().bClosed &&
            boss.iCurrentHp == hpBeforeCredit && !boss.iKoukuItemStaggerMaximum && !boss.iKoukuItemStaggerCredit &&
            damage.size() == 1u && damage.front().isStaggerSuccess && !damage.front().iAmount && !damage.front().iStaggerAmount,
            "Reaching the active maximum closes the window once and clears only its independent credit");
        KOUKUSAYDON_LOGIC_OUTPUT repeated;
        CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, 134u, damage, repeated);
        tests.Require(damage.size() == 1u && !repeated.bStaggerSuccess && !repeated.bEndPatternEarly &&
            !boss.iKoukuItemStaggerMaximum && !boss.iKoukuItemStaggerCredit && boss.iCurrentHp == hpBeforeCredit,
            "A later tick cannot reuse closed-window credit or repeat the success notification");

        pattern.LogicWindows.clear();
        for (const std::uint32_t detonationMs : {13000u, 24828u})
        {
            pattern.MechanicTriggers.clear();
            BOSS_PATTERN_MECHANIC_TRIGGER detonation;
            detonation.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION;
            detonation.iStartMs = detonationMs; pattern.MechanicTriggers.push_back(detonation);
            boss.iCurrentHp = 700000u;
            CKoukuSaydonLogicRuntime::Build(pattern, boss, 200u, ledger);
            const auto start = 200u + CKoukuSaydonLogicRuntime::Ticks_FromMs(detonationMs - 13000u);
            const auto end = 200u + CKoukuSaydonLogicRuntime::Ticks_FromMs(detonationMs);
            CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, start - 1u, view);
            tests.Require(view.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::NONE,
                "Black-hole HP gauge is hidden before its exact standalone or parent interval");
            CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, start, view);
            tests.Require(view.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::BOSS_HP &&
                view.iCurrentMechanicGauge == 700000u && view.iMaximumMechanicGauge == 1000000u,
                "Black-hole gauge projects current boss HP against actual maximum HP");
            boss.iCurrentHp -= 115000u; output = {}; damage.clear();
            CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, start + 1u, damage, output);
            CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, start + 1u, view);
            tests.Require(view.iCurrentMechanicGauge == 585000u && !output.bStaggerSuccess &&
                !output.bEndPatternEarly && damage.empty(),
                "Damage lowers black-hole boss HP and its bar without a stagger verdict or cancellation");
            CKoukuSaydonLogicRuntime::Project_MechanicGauge(boss, pattern, ledger, end, view);
            tests.Require(view.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::NONE &&
                view.iCurrentMechanicGauge == 0u && view.iMaximumMechanicGauge == 0u,
                "Black-hole gauge ends exactly at the detonation tick");
        }
        S2C_WORLD_SNAPSHOT message;
        message.iServerTick = 1u; message.ActiveGameplayRevision = catalog.Get_ActiveRevision();
        message.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
        PLAYER_SNAPSHOT player; player.iNetEntityId = 42u;
        player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        message.Players.push_back(player);
        WORLD_ENTITY_SNAPSHOT entity;
        entity.iNetEntityId = 421u; entity.eAction = WORLD_ENTITY_ACTION::IDLE;
        entity.iCurrentHp = 585000u; entity.iMaximumHp = 1000000u;
        entity.hasBossCombatState = true; entity.BossCombat.iStateRevision = 1u;
        entity.PinnedDefinitionRevision = message.ActiveGameplayRevision;
        entity.BossCombat.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::BOSS_HP;
        entity.BossCombat.iCurrentMechanicGauge = 585000u; entity.BossCombat.iMaximumMechanicGauge = 1000000u;
        message.Entities.push_back(entity);
        CPacketWriter writer; S2C_WORLD_SNAPSHOT decoded;
        bool roundtrip = Write_Message(writer, message);
        if (roundtrip)
        {
            CPacketReader reader(writer.Get_Buffer());
            roundtrip = Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u && decoded.Entities.size() == 1u;
        }
        tests.Require(roundtrip && decoded.Entities.front().BossCombat.eMechanicGaugeKind == BOSS_MECHANIC_GAUGE_KIND::BOSS_HP &&
            decoded.Entities.front().BossCombat.iCurrentMechanicGauge == 585000u &&
            decoded.Entities.front().BossCombat.iMaximumMechanicGauge == 1000000u,
            "Typed mechanic gauge mode and remaining/maximum survive the real snapshot wire");
        for (const auto invalid : {BOSS_MECHANIC_GAUGE_KIND::NONE, static_cast<BOSS_MECHANIC_GAUGE_KIND>(255u)})
        {
            message.Entities.front().BossCombat.eMechanicGaugeKind = invalid;
            CPacketWriter rejected;
            tests.Require(!Write_Message(rejected, message), "Snapshot rejects unknown modes and hidden gauges carrying values");
        }
    }
}

int LostArk::Server::Run_ServerKoukuSupportSurfaceContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests;
	Run_MechanicGaugeContracts(tests);
	namespace fs = std::filesystem;
	const auto fixture = fs::temp_directory_path() / (L"LostArkKoukuSupport-" + std::to_wstring(_getpid()));
	fs::create_directories(fixture / L"Navigation");
	{
		std::ofstream grid(fixture / L"Navigation/NAV_SUPPORT.navgrid", std::ios::binary);
		const std::uint32_t size = 4u;
		const float cell = 4.f, origin = 0.f;
		std::array<std::uint8_t, 16u> walkable; walkable.fill(1u); walkable[1] = 0u;
		std::array<float, 16u> heights; heights.fill(1.f);
		for (const auto value : { size, size }) grid.write(reinterpret_cast<const char*>(&value), sizeof(value));
		for (const auto value : { cell, origin, origin }) grid.write(reinterpret_cast<const char*>(&value), sizeof(value));
		grid.write(reinterpret_cast<const char*>(walkable.data()), walkable.size());
		grid.write(reinterpret_cast<const char*>(heights.data()), heights.size() * sizeof(float));
		std::ofstream(fixture / L"Navigation/NAV_SUPPORT.navpolicy") << R"(LOSTARK_NAVIGATION_POLICY 1 "NAV_SUPPORT" 1
)";
		std::ofstream(fixture / L"Navigation/NAV_SUPPORT.navblockers") << R"(LOSTARK_NAVGRID_BLOCKERS 1 "NAV_SUPPORT" 4 4 4 0 0 1
REGION "blocked" "closed" 0 1
2 0
)";
	}
	std::vector<wchar_t> previous(32768u);
	const DWORD previousLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", previous.data(), static_cast<DWORD>(previous.size()));
	SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", fixture.c_str());
	CServerNavigation nav;
	const bool loaded = nav.Load("NAV_SUPPORT");
	SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", previousLength ? previous.data() : nullptr);
	tests.Require(loaded, "Load isolated four-metre navigation with blocked and missing ground");
	std::string status;
	SERVER_NAVIGATION_SUPPORT_SURFACE floor{ "test.floor", 6.f, 6.f, 2.f, 1.6f };
	SERVER_NAV_POINT point;
	bool applied = loaded && nav.Set_RuntimeSupportSurfaces({ floor }, status);
	tests.Require(applied && nav.Sample_Position(6.f, 6.f, point) && std::abs(point.y - 1.6f) < .00001f,
		"Circular support raises the existing ground");
	tests.Require(nav.Sample_Position(8.f, 6.f, point) && std::abs(point.y - 1.6f) < .00001f &&
		nav.Sample_Position(8.01f, 6.f, point) && point.y == 1.f,
		"Exact circle edge does not expand to the four-metre cell");
	std::vector<SERVER_NAV_POINT> path;
	tests.Require(nav.Find_Path(2.f, 6.f, 10.f, 6.f, path) && path.size() >= 2u &&
		std::abs(path.front().y - 1.6f) < .00001f && nav.Has_LineOfSight(2.f, 6.f, 10.f, 6.f) &&
		nav.Resolve_TraversalStep(3.9f, 6.f, 4.1f, 6.f, point) && std::abs(point.y - 1.6f) < .00001f,
		"A-star, line of sight and live movement use the same supported height");
	{
		// A wall can truncate a root step after crossing the exact raised-floor edge.
		CServerCollisionSystem collision;
		collision.Set_BlockingBodies({ { 4.25f, 6.f, .05f } });
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = "KOUKUSAYDON_ROOT_SUPPORT_CONTRACT";
		pattern.Stages.emplace_back(); pattern.Stages.front().iDurationMs = 1000u;
		pattern.Stages.front().Motion.RootMotion = { {0u, 0.f, 0.f, 0.f}, {1000u, 0.f, .4f, 0.f} };
		SERVER_WORLD_ENTITY boss; boss.strPatternId = pattern.strPatternId;
		boss.fPositionX = 3.9f; boss.fPositionY = 1.f; boss.fPositionZ = 6.f;
		boss.fCollisionRadius = .05f; boss.iPatternStageFirstEvaluationTick = 1u;
		boss.PatternStageRootMotion = pattern.Stages.front().Motion.RootMotion;
		const bool moved = CKoukuSaydonBrain::Apply_StageRootMotion(boss, pattern, 30u, nav, collision, status);
		tests.Require(moved && boss.fPositionX > 4.f && boss.fPositionX < 4.25f &&
			std::abs(boss.fPositionY - 1.6f) < .00001f && boss.fPositionZ == 6.f,
			"A collision-clipped root step uses ground at resolved XZ instead of sinking into the raised support");
	}
	floor.fRadiusM = .25f; floor.fHeightY = 3.f;
	nav.Set_RuntimeSupportSurfaces({ floor }, status);
	tests.Require(!nav.Resolve_TraversalStep(5.f, 6.f, 7.f, 6.f, point) &&
		!nav.Has_LineOfSight(5.f, 6.f, 7.f, 6.f) && !nav.Find_Path(5.f, 6.f, 7.f, 6.f, path),
		"A tall sub-cell disk cannot bypass step policy when both endpoints lie outside it");
	floor.fCenterX = 6.f; floor.fCenterZ = 2.f; floor.fRadiusM = 1.f;
	SERVER_NAVIGATION_SUPPORT_SURFACE blockerFloor{ "blocked.floor", 10.f, 2.f, 1.f, 1.6f };
	nav.Set_RuntimeSupportSurfaces({ floor, blockerFloor }, status);
	tests.Require(!nav.Sample_Position(6.f, 2.f, point) && !nav.Is_PointWalkableExact(6.f, 2.f) &&
		!nav.Is_PointWalkableExact(10.f, 2.f) && !nav.Has_LineOfSight(10.f, 2.f, 10.f, 2.f),
		"Supports do not open missing ground or runtime blockers");
	const auto revision = nav.Get_Revision();
	floor.fRadiusM = -1.f;
	tests.Require(!nav.Set_RuntimeSupportSurfaces({ floor }, status) && nav.Get_Revision() == revision &&
		nav.Get_RuntimeSupportSurfaceCount() == 2u, "Invalid surface replacement preserves the previous committed list");
	nav.Set_RuntimeSupportSurfaces({}, status);
	{
		// This common fixture exercises the published counter in Debug and Release.
		auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
		room->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch = 1u;
		room->m_ServerNavigation = nav;
		room->m_ServerCollisionSystem.Initialize({}, status);
		room->m_WorldEntities.clear();
		SERVER_WORLD_ENTITY boss{};
		boss.iNetEntityId = 700u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		boss.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
		boss.strArchetypeId = "BOSS_KOUKUSAYDON_G1_SAYDON";
		boss.iCurrentHp = 100u; boss.fPositionX = boss.fPositionZ = 6.f;
		room->m_WorldEntities.push_back(std::move(boss));
	for (const unsigned scenario : {0u, 1u, 2u, 3u, 4u})
	{
		const bool withFollowup = scenario != 0u, savedSource = scenario >= 2u, stagger = scenario == 4u;
		const std::string sourceId = stagger ? "KOUKUSAYDON_G1_PATTERN_33" : scenario == 3u ? "KOUKUSAYDON_G1_PATTERN_118" : "KOUKUSAYDON_G1_PATTERN_81";
		const std::string groggyId = scenario >= 3u ? "KOUKUSAYDON_G1_PATTERN_42" : "KOUKUSAYDON_G1_PATTERN_4";
		const std::string pursuitId = scenario >= 3u ? "KOUKUSAYDON_G1_PATTERN_124" : "KOUKUSAYDON_G1_PATTERN_101";
		const auto& sourceCatalog = room->m_GameplayCatalog.Active();
		const auto* source = savedSource ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(sourceCatalog, sourceId, status) : nullptr;
		const auto* groggy = savedSource ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(sourceCatalog, groggyId, status) : nullptr;
		if (savedSource)
		{
			tests.Require(source && groggy && source->strGateId == groggy->strGateId &&
				source->strTargetBossPlacementId == groggy->strTargetBossPlacementId && groggy->Stages.size() == 3u,
				"Published rolling counter and its existing groggy retain the same gate and boss");
			if (!source || !groggy) continue;
		}
		// Counter success closes the WORLD owner with or without a followup outcome.
		auto& counterBoss = room->m_WorldEntities.front();
		const auto counterBossBefore = std::make_unique<SERVER_WORLD_ENTITY>(counterBoss);
		const auto counterTickBefore = room->m_iServerTick;
		const auto counterStatusBefore = room->m_strStatus;
		counterBoss.strPatternId = savedSource ? sourceId : "KOUKUSAYDON_COUNTER_LANDING"; counterBoss.strActionId = "counter.action";
		if (savedSource)
		{
			counterBoss.strArchetypeId = "BOSS_KOUKUSAYDON_" + std::string(scenario == 2u ? "G1" : "G3") + "_SAYDON"; counterBoss.strEncounterId = source->strEncounterId;
			counterBoss.strPlacementId = source->strTargetBossPlacementId; counterBoss.iCurrentHp = stagger ? 10000u : 100u;
		}
		counterBoss.iPatternSequence = 91u; counterBoss.fPositionY = 3.9f; counterBoss.fCollisionRadius = .5f;
		counterBoss.fSpawnPositionX = counterBoss.fPositionX; counterBoss.fSpawnPositionZ = counterBoss.fPositionZ;
		counterBoss.fSpawnPositionY = 1.25f;
		counterBoss.PatternStageRootMotion = {{0u, 0.f, 0.f, 0.f}, {3000u, 0.f, 0.f, 2.6f}};
		counterBoss.bPatternStageRootOriginCaptured = true;
		CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER counterMember;
		counterMember.strMemberId = "counter.member"; counterMember.iBossEntityId = counterBoss.iNetEntityId;
		counterMember.PatternIds = {counterBoss.strPatternId}; counterMember.iPatternSequence = counterBoss.iPatternSequence;
		if (savedSource) { counterMember.PatternIds.push_back(pursuitId); counterMember.TransitionTicks = {0u}; }
		counterMember.WorldCueByInstance["rolling.ball"] = "ball.cue";
		counterMember.WorldCueByOccurrence["counter.world.1"] = "ball.cue";
		room->m_KoukuSaydonPatternAudition.Members = {counterMember};
		S2C_WORLD_SEQUENCE_PLAY ball; ball.iRunEpoch = 1u; ball.strMemberId = counterMember.strMemberId;
		ball.strCueId = "ball.cue"; ball.strOccurrenceId = "counter.world.1"; ball.strSequenceInstanceId = "rolling.ball";
		ball.iBossNetEntityId = counterBoss.iNetEntityId; ball.iPatternSequence = counterBoss.iPatternSequence;
		auto otherBall = ball; otherBall.strMemberId = "other.member"; otherBall.strCueId = "other.cue";
		room->m_KoukuSaydonPatternAudition.WorldPlays = {ball, otherBall};
		if (stagger)
		{
			ball.strCueId = "fatal.blade.cue"; ball.strOccurrenceId = "mario.fatal.blade";
			room->m_KoukuSaydonPatternAudition.WorldPlays.push_back(ball);
		}
		room->m_PendingKoukuMechanicTriggers = {{counterBoss.iNetEntityId, counterBoss.iPatternSequence, {}}, {701u, 1u, {}}};
		BOSS_PATTERN_DEFINITION counterPattern; counterPattern.strPatternId = counterBoss.strPatternId;
		BOSS_PATTERN_LOGIC_WINDOW counterWindow; counterWindow.strWindowId = "counter.window";
		counterWindow.eKind = BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW; counterWindow.iDurationMs = 2837u;
		counterWindow.bEndsPatternOnSuccess = true;
		if (savedSource)
		{
			const auto window = std::find_if(source->LogicWindows.begin(), source->LogicWindows.end(), [stagger](const auto& value)
				{ return value.eKind == (stagger ? BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW : BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW); });
			tests.Require(window != source->LogicWindows.end(), "Published rolling pattern retains its real counter window");
			if (window != source->LogicWindows.end()) counterWindow = *window;
		}
		else if (withFollowup)
			counterWindow.OnSuccess.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN, 0u, 0u, groggyId});
		if (stagger) counterBoss.iCurrentHp = counterBoss.iMaximumHp = 10000u;
		counterPattern.LogicWindows = {counterWindow};
		auto& counterLedger = room->m_KoukuSaydonPatternAudition.Members.front().LogicLedger;
		KOUKUSAYDON_LOGIC_OUTPUT counterOutput; std::vector<DAMAGE_EVENT> counterDamage;
		CKoukuSaydonLogicRuntime::Build(counterPattern, counterBoss, 120u, counterLedger);
		CKoukuSaydonLogicRuntime::Update(counterBoss, counterPattern, counterLedger, room->m_Players,
			room->m_GameplayCatalog, nullptr, 120u, counterDamage, counterOutput, &room->m_ServerNavigation, &room->m_ServerCollisionSystem);
		if (stagger)
		{
			// 실제 아이템 catalog의 무력화 입력을 기존 공용 명중 경로로 보낸다. 아이템 사용/쿨다운이나
			// 투사체 도형을 흉내 내지 않고, 이미 승인된 명중 세 건의 credit 및 Logic 결과만 검증한다.
			const auto* grenade = room->m_ItemCatalog.Find_Item("BATTLE_WHIRLWIND_GRENADE");
			const auto maximum = room->m_GameplayCatalog.Active().Get_RaidStaggerMaximum();
			const bool ready = grenade && grenade->BattleUse.iSkillId &&
				grenade->BattleUse.iStaggerMaximumDivisor == 3u && maximum > 2u &&
				counterBoss.iKoukuItemStaggerMaximum == maximum && !counterBoss.iKoukuItemStaggerCredit;
			tests.Require(ready, "Mario fixture opens the current common stagger policy and admits the published whirlwind input");
			if (ready)
			{
				SERVER_PLAYER_TO_WORLD_HIT hit{};
				hit.iSourcePlayerId = 1u; hit.iSkillId = grenade->BattleUse.iSkillId;
				hit.iStaggerMaximumDivisor = grenade->BattleUse.iStaggerMaximumDivisor; hit.iServerTick = 120u;
				const auto third = maximum / 3u + (maximum % 3u ? 1u : 0u);
				bool creditCorrect = true;
				for (unsigned count = 1u; count <= 3u; ++count)
				{
					const auto result = CServerCombatHitRuntime::Apply_PlayerToWorld(counterBoss, hit, counterDamage);
					const auto expected = (std::min)(maximum, count * third);
					std::uint64_t totalCredit = 0u;
					for (const auto& event : counterDamage) {
						creditCorrect = creditCorrect && event.iTargetNetEntityId == counterBoss.iNetEntityId &&
							event.isOutgoing && !event.iAmount && !event.isStaggerSuccess;
						totalCredit += event.iStaggerAmount;
					}
					creditCorrect = creditCorrect && result == SERVER_COMBAT_HIT_RESULT::LANDED &&
						counterBoss.iCurrentHp == 10000u && counterBoss.iKoukuItemStaggerCredit == expected &&
						counterDamage.size() == count && totalCredit == expected;
					if (count < 3u) {
						CKoukuSaydonLogicRuntime::Update(counterBoss, counterPattern, counterLedger, room->m_Players,
							room->m_GameplayCatalog, nullptr, 120u, counterDamage, counterOutput, &room->m_ServerNavigation, &room->m_ServerCollisionSystem);
						creditCorrect = creditCorrect && !counterOutput.bStaggerSuccess && !counterOutput.bEndPatternEarly;
					}
				}
				tests.Require(creditCorrect, "Three admitted whirlwind hits contribute one capped third each without HP loss, duplicate credit or early success");
			}
		}
		const bool counterHit = stagger || CBossCombatRuntime::Try_TriggerCounter(counterBoss, 121u);
		CKoukuSaydonLogicRuntime::Update(counterBoss, counterPattern, counterLedger, room->m_Players,
			room->m_GameplayCatalog, nullptr, 121u, counterDamage, counterOutput, &room->m_ServerNavigation, &room->m_ServerCollisionSystem);
		if (stagger)
		{
			const auto successCount = [&]() { return std::count_if(counterDamage.begin(), counterDamage.end(), [](const auto& event) { return event.isStaggerSuccess; }); };
			tests.Require(successCount() == 1 && std::any_of(counterDamage.begin(), counterDamage.end(), [&](const auto& event) {
				return event.isStaggerSuccess && event.isOutgoing && !event.isCounterSuccess && !event.iAmount && !event.iStaggerAmount &&
					event.iTargetNetEntityId == counterBoss.iNetEntityId && event.iSourcePlayerId == INVALID_PLAYER_ID &&
					event.fPositionX == counterBoss.fPositionX && event.fPositionY == counterBoss.fPositionY && event.fPositionZ == counterBoss.fPositionZ; }),
				"The authored Mario stagger verdict emits one zero-damage success at its boss without inventing a player contribution");
			KOUKUSAYDON_LOGIC_OUTPUT repeatedOutput;
			CKoukuSaydonLogicRuntime::Update(counterBoss, counterPattern, counterLedger, room->m_Players,
				room->m_GameplayCatalog, nullptr, 122u, counterDamage, repeatedOutput, &room->m_ServerNavigation, &room->m_ServerCollisionSystem);
			tests.Require(successCount() == 1 && !repeatedOutput.bStaggerSuccess && !repeatedOutput.bEndPatternEarly &&
				!counterBoss.iKoukuItemStaggerMaximum && !counterBoss.iKoukuItemStaggerCredit && counterBoss.iCurrentHp == 10000u,
				"A closed authored stagger window never reuses credit or repeats success on later ticks");
		}
		room->m_iServerTick = 121u;
		const bool completed = counterHit && room->Apply_KoukuLogicOutput(counterOutput, counterBoss, 121u);
		const auto& committed = room->m_KoukuSaydonPatternAudition.Members.front();
		tests.Require(completed && (stagger ? counterOutput.bStaggerSuccess :
			(counterOutput.bCounterSuccessLanded && std::abs(counterBoss.fPositionY - 1.25f) < .0001f)) &&
			counterBoss.strPatternId.empty() && counterBoss.PatternStageRootMotion.empty() && !counterBoss.bPatternStageRootOriginCaptured &&
			(savedSource ? (committed.PatternIds == std::vector<std::string>{sourceId, groggyId, pursuitId} &&
				committed.TransitionTicks == std::vector<std::uint32_t>{1u, 0u}) :
				(withFollowup ? (committed.PatternIds.size() == 2u && committed.PatternIds.back() == groggyId &&
					committed.TransitionTicks == std::vector<std::uint32_t>{1u}) :
					(committed.PatternIds.size() == 1u && committed.TransitionTicks.empty()))),
			savedSource ? "Saved rolling counter schedules exactly one same-gate groggy before its next player-pursuit pattern" :
			withFollowup ? "Actual counter verdict lands, clears source root motion and schedules existing groggy one tick later" :
				"A counter with no outcomes lands and ends the Pattern without inventing a followup");
		tests.Require(room->m_KoukuSaydonPatternAudition.WorldPlays.size() == 1u &&
			room->m_KoukuSaydonPatternAudition.WorldPlays.front().strMemberId == "other.member" &&
			committed.WorldCueByInstance.empty() && committed.WorldCueByOccurrence.empty() &&
			room->m_PendingKoukuMechanicTriggers.size() == 1u && room->m_PendingKoukuMechanicTriggers.front().iBossEntityId == 701u,
			stagger ? "Mario stagger success immediately stops normal and instant blade WORLD cues and pending mechanics, preserving another member" :
			"Counter interruption stops only its active ball WORLD owner and pending mechanics, preserving another member");
		if (savedSource)
		{
			const bool started = room->m_KoukuSaydonBrain.Begin_Pattern(counterBoss, *groggy,
				room->m_GameplayCatalog.Get_ActiveRevision(), 122u, status);
			unsigned changes = 0u, completedAt = 0u;
			for (std::uint32_t tick = 123u; started && tick <= 260u; ++tick)
			{
				const auto result = room->m_KoukuSaydonBrain.Update(counterBoss, sourceCatalog, tick, status);
				if (result == KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED) ++changes;
				if (result == KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED) { completedAt = tick; break; }
			}
			tests.Require(started && changes == 2u && completedAt >= 247u && completedAt <= 250u,
				"Saved groggy runs its start, hold and recovery for the full 4167 ms before completion");
		}
		room->m_KoukuSaydonPatternAudition.Members.clear(); room->m_KoukuSaydonPatternAudition.WorldPlays.clear();
		room->m_PendingKoukuMechanicTriggers.clear();
		counterBoss = *counterBossBefore; room->m_iServerTick = counterTickBefore; room->m_strStatus = counterStatusBefore;
	}

	}
	{
		// Reuse one admitted catalog: resetless reload must replace every profile,
		// while a late failure restores both buff indexes, formula and ember data.
		std::vector<wchar_t> root(32768u);
		const DWORD length = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", root.data(), DWORD(root.size()));
		fs::path dataRoot;
		if (length && length < root.size()) dataRoot = root.data();
		else { GetModuleFileNameW(nullptr, root.data(), DWORD(root.size())); dataRoot = fs::path(root.data()).parent_path().parent_path() / L"DataFiles"; }
		std::ifstream input(dataRoot / L"Gameplay/Gameplay.bootstrap", std::ios::binary);
		std::string baseline((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
		if (!baseline.empty() && baseline.back() != '\n') baseline += '\n';
		fs::create_directories(fixture / L"Gameplay");
		CGameplayCatalog profiles;
		const auto load = [&](std::string bytes) {
			const auto end = bytes.find('\n'), count = bytes.rfind('\t', end);
			if (end == std::string::npos || count == std::string::npos) return false;
			bytes.replace(count + 1u, end - count - 1u, std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
			{ std::ofstream output(fixture / L"Gameplay/Gameplay.bootstrap", std::ios::binary | std::ios::trunc); output.write(bytes.data(), bytes.size()); }
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", fixture.c_str());
			const bool result = profiles.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", length ? root.data() : nullptr);
			return result;
		};
		const bool ready = load(baseline);
		const auto* damage = profiles.Find_DamageProfile("damage.player.17030");
		const auto* buff = profiles.Find_SkillBuff(171702u);
		const auto* ember = profiles.Find_EmberProfile(CHARACTER_CLASS_ID::GUARDIANKNIGHT);
		tests.Require(ready && damage && buff && ember, "Load actual Retail damage, skill buff and Guardian ember profiles for reload regression");
		if (ready && damage && buff && ember)
		{
			const auto beforeDamage = *damage;
			const auto beforeBuff = *buff;
			const auto beforeEmber = *ember;
			const auto firstRevision = profiles.Get_ActiveRevision();
			tests.Require(load(baseline) && profiles.Get_ActiveRevision() == firstRevision,
				"The same complete Retail bootstrap reloads twice without accumulating duplicate skill buffs");
			const auto replaceField = [](std::string& bytes, const std::string& prefix, const unsigned field, const std::string& value) {
				auto at = bytes.find(prefix); if (at == std::string::npos) return false;
				const auto rowEnd = bytes.find('\n', at);
				for (unsigned i = 0u; i < field; ++i) { at = bytes.find('\t', at); if (at == std::string::npos || at >= rowEnd) return false; ++at; }
				auto end = bytes.find('\t', at); if (end == std::string::npos || end > rowEnd) end = rowEnd;
				if (end > at && bytes[end - 1u] == '\r') --end;
				bytes.replace(at, end - at, value); return true;
			};
			std::string changed = baseline;
			const bool fieldsChanged = replaceField(changed, "DAMAGE\tdamage.player.17030\t", 4u, std::to_string(beforeDamage.iDamageAddend + 7u)) &&
				replaceField(changed, "SKILLBUFF\t17170\t171702\t", 4u, std::to_string(beforeBuff.iDurationMs + 1u)) &&
				replaceField(changed, "PLAYEREMBER\tGUARDIANKNIGHT\t", 2u, std::to_string(beforeEmber.iGaugeGainPerHit + 1u));
			const auto matches = [&](const unsigned addend, const unsigned duration, const unsigned gain) {
				const auto* d = profiles.Find_DamageProfile("damage.player.17030");
				const auto* b = profiles.Find_SkillBuff(171702u);
				const auto* list = profiles.Find_SkillBuffs(17170u);
				const auto* e = profiles.Find_EmberProfile(CHARACTER_CLASS_ID::GUARDIANKNIGHT);
				return d && d->iDamageAddend == addend && b && b->iDurationMs == duration && e && e->iGaugeGainPerHit == gain && list &&
					std::count_if(list->begin(), list->end(), [&](const auto& row) { return row.iBuffId == 171702u && row.iDurationMs == duration; }) == 1;
			};
			tests.Require(fieldsChanged && load(changed) && matches(beforeDamage.iDamageAddend + 7u, beforeBuff.iDurationMs + 1u, beforeEmber.iGaugeGainPerHit + 1u),
				"A valid reload replaces damage formula, both buff lookup views and ember values together");
			const auto committedRevision = profiles.Get_ActiveRevision();
			tests.Require(!load(baseline + "INVALID_PROFILE_RELOAD_ROW\n") && profiles.Get_ActiveRevision() == committedRevision &&
				matches(beforeDamage.iDamageAddend + 7u, beforeBuff.iDurationMs + 1u, beforeEmber.iGaugeGainPerHit + 1u),
				"A late malformed row restores the complete prior profile generation and revision");
			const auto buffStart = baseline.find("SKILLBUFF\t17170\t171702\t");
			const auto buffEnd = baseline.find('\n', buffStart);
			const auto duplicate = baseline.substr(buffStart, buffEnd - buffStart + 1u);
			tests.Require(!load(baseline + duplicate) && profiles.Get_ActiveRevision() == committedRevision &&
				matches(beforeDamage.iDamageAddend + 7u, beforeBuff.iDurationMs + 1u, beforeEmber.iGaugeGainPerHit + 1u),
				"A real duplicate buff row remains rejected and cannot replace the previous generation");
			tests.Require(load(baseline) && matches(beforeDamage.iDamageAddend, beforeBuff.iDurationMs, beforeEmber.iGaugeGainPerHit),
				"A valid reload after rejection restores the authored baseline without leaked candidate profiles");
		}
	}
#ifdef _DEBUG
	CServerGameplayContractRunner::Run_RuntimeSupportPrediction(tests, nav);
	// The same owned schedule methods used before walking also cover stationary players and cleanup.
	auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
	room->m_ServerNavigation = nav;
	// This isolated grid owns its actors; the real bootstrap may already spawn G1 Kouku.
	room->m_WorldEntities.clear();
	SERVER_PLAYER player; player.iPlayerId = 1u; player.iCurrentHp = 100u;
	player.fPositionX = 6.f; player.fPositionY = 1.f; player.fPositionZ = 6.f;
	room->m_Players[1u] = player;
	auto supportedBoss = std::make_unique<SERVER_WORLD_ENTITY>();
	supportedBoss->iNetEntityId = 700u; supportedBoss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	supportedBoss->strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
	supportedBoss->strArchetypeId = "BOSS_KOUKUSAYDON_G1_SAYDON";
	supportedBoss->iCurrentHp = 100u; supportedBoss->fPositionX = 6.f; supportedBoss->fPositionZ = 6.f; supportedBoss->fPositionY = 1.f;
	room->m_WorldEntities.push_back(std::move(*supportedBoss));
	auto outsideBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
	outsideBoss->iNetEntityId = 701u; outsideBoss->fPositionX = 15.f; outsideBoss->fPositionY = 8.63f;
	room->m_WorldEntities.push_back(std::move(*outsideBoss));
	for (const bool rootMotion : { false, true })
	{
		auto airborneBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
		airborneBoss->iNetEntityId = rootMotion ? 703u : 702u; airborneBoss->fPositionY = 8.63f;
		if (rootMotion) airborneBoss->PatternStageRootMotion.push_back({ 0u, 0.f, 0.f });
		else airborneBoss->fPatternForcedMotionSpeed = 1.f;
		room->m_WorldEntities.push_back(std::move(*airborneBoss));
	}
	room->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch = 1u;
	CGameRoom::KOUKU_SCHEDULED_SUPPORT_SURFACE scheduled;
	scheduled.strMemberId = "member.one"; scheduled.iStartTick = 100u; scheduled.iEndTick = 102u;
	scheduled.Surface = { "1/member.one/pattern/world", 6.f, 6.f, 2.f, 1.6f };
	room->m_KoukuSaydonPatternAudition.SupportSchedule = { scheduled };
	tests.Require(room->Refresh_KoukuSupportSurfaces(99u) && room->m_Players[1u].fPositionY == 1.f &&
		room->Refresh_KoukuSupportSurfaces(100u) && std::abs(room->m_Players[1u].fPositionY - 1.6f) < .00001f &&
		!room->m_Players[1u].hasMoveGoal, "WORLD start tick supports a stationary player without a movement command");
	tests.Require(std::abs(room->m_WorldEntities[0].fPositionY - 1.6f) < .00001f && room->m_WorldEntities[1].fPositionY == 8.63f,
		"The same WORLD surface supports a stationary Saydon and preserves a boss outside its footprint");
	tests.Require(room->m_WorldEntities[2].fPositionY == 8.63f && std::abs(room->m_WorldEntities[3].fPositionY - 9.23f) < .00001f,
		"An active WORLD floor preserves forced motion Y and the source root height above ground");
	tests.Require(room->Refresh_KoukuSupportSurfaces(102u) && room->m_Players[1u].fPositionY == 1.f &&
		room->m_ServerNavigation.Get_RuntimeSupportSurfaceCount() == 0u,
		"WORLD end tick removes the support and restores stationary ground height");
	tests.Require(room->m_WorldEntities[0].fPositionY == 1.f, "WORLD end restores the stationary boss ground height");
	scheduled.iEndTick = 200u;
	auto other = scheduled; other.strMemberId = "member.two"; other.Surface.strOwnerKey = "other.floor";
	other.Surface.fHeightY = 1.8f;
	room->m_KoukuSaydonPatternAudition.SupportSchedule = { scheduled, other };
	room->Refresh_KoukuSupportSurfaces(110u);
	room->Stop_KoukuWorldOwner("member.two");
	tests.Require(room->Refresh_KoukuSupportSurfaces(110u) && room->m_ServerNavigation.Get_RuntimeSupportSurfaceCount() == 1u &&
		std::abs(room->m_Players[1u].fPositionY - 1.6f) < .00001f, "Stopping one WORLD owner preserves another owner's support");
	room->Stop_KoukuWorldOwner();
	tests.Require(room->Refresh_KoukuSupportSurfaces(110u) && room->m_Players[1u].fPositionY == 1.f,
		"Stopping the complete run returns players to the original floor");
	tests.Require(room->m_WorldEntities[0].fPositionY == 1.f && room->m_WorldEntities[1].fPositionY == 8.63f,
		"Stop owner restores only the boss previously supported by the removed floor");
	tests.Require(room->m_WorldEntities[2].fPositionY == 8.63f && std::abs(room->m_WorldEntities[3].fPositionY - 8.63f) < .00001f,
		"Removing the WORLD floor restores the root terrain delta and preserves forced motion Y");

	for (const bool alreadyCaptured : { false, true })
	{
		auto rootRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
		rootRoom->m_ServerNavigation = nav;
		rootRoom->m_WorldEntities.clear();
		auto rootBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
		rootBoss->fPositionY = 1.f; rootBoss->fCollisionRadius = .05f;
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = "KOUKUSAYDON_ROOT_FLOOR_LIFECYCLE"; pattern.bFixedTimelineClock = true;
		pattern.Stages.emplace_back(); pattern.Stages.front().iDurationMs = 1000u;
		pattern.Stages.front().Motion.RootMotion = {{0u, 0.f, 0.f, 0.f}, {1000u, 0.f, 0.f, .3f}};
		rootBoss->strPatternId = pattern.strPatternId; rootBoss->iPatternStartTick = 100u;
		rootBoss->iPatternStageFirstEvaluationTick = 100u;
		rootBoss->PatternStageRootMotion = pattern.Stages.front().Motion.RootMotion;
		rootBoss->bPatternStageRootOriginCaptured = alreadyCaptured;
		rootBoss->fPatternStageOriginX = 6.f; rootBoss->fPatternStageOriginY = 1.f; rootBoss->fPatternStageOriginZ = 6.f;
		rootBoss->fPatternStageRootGroundY = 1.f;
		rootRoom->m_WorldEntities.push_back(std::move(*rootBoss));
		auto& liveRoot = rootRoom->m_WorldEntities.front();
		auto rootFloor = scheduled; rootFloor.iStartTick = 100u; rootFloor.iEndTick = 102u;
		rootRoom->m_KoukuSaydonPatternAudition.SupportSchedule = {rootFloor};
		CServerCollisionSystem collision;
		const bool started = rootRoom->Refresh_KoukuSupportSurfaces(100u) &&
			CKoukuSaydonBrain::Apply_StageRootMotion(liveRoot, pattern, 100u, rootRoom->m_ServerNavigation, collision, status);
		tests.Require(started && std::abs(liveRoot.fPositionY - 1.6f) < .00001f &&
			std::abs(liveRoot.fPatternStageOriginY - liveRoot.fPatternStageRootGroundY) < .00001f,
			alreadyCaptured ? "A floor raises an already captured root without doubling the terrain offset" :
				"A floor raises the boss before its first root capture instead of capturing a negative ground offset");
		const bool next = rootRoom->Refresh_KoukuSupportSurfaces(100u) && rootRoom->Refresh_KoukuSupportSurfaces(101u) &&
			CKoukuSaydonBrain::Apply_StageRootMotion(liveRoot, pattern, 101u, rootRoom->m_ServerNavigation, collision, status);
		tests.Require(next && std::abs(liveRoot.fPositionY - 1.61f) < .00001f,
			"Repeated support refresh applies no extra lift and preserves the native vertical sample");
		const bool removed = rootRoom->Refresh_KoukuSupportSurfaces(102u) &&
			CKoukuSaydonBrain::Apply_StageRootMotion(liveRoot, pattern, 102u, rootRoom->m_ServerNavigation, collision, status);
		tests.Require(removed && std::abs(liveRoot.fPositionY - 1.02f) < .00001f,
			"Support removal returns a running root to arena ground plus its source height, never below the arena");
	}

	for (const bool skippedTicks : { false, true })
	{
		// Production Whirlwind is a 10 m / 3204 ms Logic charge, not source root motion.
		BOSS_PATTERN_DEFINITION chargePattern; chargePattern.strPatternId = "KOUKUSAYDON_G1_PATTERN_24";
		BOSS_PATTERN_LOGIC_WINDOW charge; charge.strWindowId = "KOUKUSAYDON_G1_PATTERN_24.logic.1";
		charge.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; charge.iDurationMs = 3204u; charge.fBossChargeDistanceM = 10.f;
		chargePattern.LogicWindows = {charge};
		auto chargeBoss = std::make_unique<SERVER_WORLD_ENTITY>(room->m_WorldEntities.front());
		chargeBoss->strPatternId = chargePattern.strPatternId; chargeBoss->iPatternSequence = 1u;
		chargeBoss->fPositionX = 2.f; chargeBoss->fPositionY = 1.f; chargeBoss->fPositionZ = 2.f;
		chargeBoss->fCollisionRadius = .05f;
		std::map<PLAYER_ID, SERVER_PLAYER> targets;
		auto& target = targets[1u]; target.iPlayerId = 1u; target.iNetEntityId = 100u;
		target.iCurrentHp = target.iMaximumHp = 100u; target.isCombatReady = true;
		target.fPositionX = 14.f; target.fPositionY = 1.f; target.fPositionZ = 2.f;
		KOUKUSAYDON_LOGIC_LEDGER chargeLedger; KOUKUSAYDON_LOGIC_OUTPUT chargeOutput;
		std::vector<DAMAGE_EVENT> damage; CServerCollisionSystem collision;
		CKoukuSaydonLogicRuntime::Build(chargePattern, *chargeBoss, 100u, chargeLedger);
		bool everyStepNavigable = true;
		for (std::uint32_t tick = 100u; tick <= 197u; ++tick)
		{
			if (skippedTicks && tick > 100u && tick < 197u) continue;
			const float previousX = chargeBoss->fPositionX, previousZ = chargeBoss->fPositionZ;
			CKoukuSaydonLogicRuntime::Update(*chargeBoss, chargePattern, chargeLedger, targets,
				room->m_GameplayCatalog.Active(), nullptr, tick, damage, chargeOutput, &nav, &collision);
			everyStepNavigable = everyStepNavigable && nav.Has_LineOfSight(previousX, previousZ,
				chargeBoss->fPositionX, chargeBoss->fPositionZ);
		}
		tests.Require(everyStepNavigable && chargeBoss->fPositionX > 3.998f && chargeBoss->fPositionX < 4.f &&
			chargeBoss->fPositionZ == 2.f && chargeBoss->fPositionY == 1.f && chargeLedger.Windows.front().bChargeStopped,
			skippedTicks ? "Whirlwind cannot tunnel across blocked intermediate cells even when both endpoints are walkable" :
				"Whirlwind stops within one millimetre of navigation and every committed fixed-tick segment stays navigable");
		const float stoppedX = chargeBoss->fPositionX;
		CKoukuSaydonLogicRuntime::Update(*chargeBoss, chargePattern, chargeLedger, targets,
			room->m_GameplayCatalog.Active(), nullptr, 200u, damage, chargeOutput, &nav, &collision);
		tests.Require(chargeBoss->fPositionX == stoppedX, "Stopped Whirlwind cannot resume beyond navigation on later ticks");
	}


	// Albion uses this same ground authority and the existing combat-object transaction.
	room->m_CombatObjectRuntime.Reset();
	auto& albionOwner = room->m_WorldEntities.front();
	albionOwner.strPatternId = "KOUKUSAYDON_TEST_ALBION";
	albionOwner.iPatternSequence = 1u;
	albionOwner.PinnedDefinitionRevision = room->m_GameplayCatalog.Get_ActiveRevision();
	room->m_Players.clear();
	for (PLAYER_ID id = 1u; id <= 3u; ++id)
	{
		auto& target = room->m_Players[id];
		target.iPlayerId = id; target.iNetEntityId = 100u + id;
		target.iCurrentHp = id == 3u ? 0u : 100u; target.iMaximumHp = 100u;
		target.isCombatReady = true; target.fPositionX = id == 2u ? 10.f : 6.f;
		target.fPositionY = 99.f; target.fPositionZ = 6.f;
	}
	BOSS_PATTERN_MECHANIC_TRIGGER albion{};
	albion.strTriggerId = "test.albion.logic.1";
	albion.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE;
	albion.iCountPerPlayer = 1u; albion.iEffectLifetimeMs = 7000u;
	const auto queueAlbion = [&](const std::uint32_t tick)
	{
		room->m_PendingKoukuMechanicTriggers.push_back({ albionOwner.iNetEntityId, albionOwner.iPatternSequence, albion });
		room->Commit_KoukuMechanicTriggers(tick);
	};
	queueAlbion(1000u);
	const auto& initialObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	tests.Require(initialObjects.size() == 2u && std::all_of(initialObjects.begin(), initialObjects.end(), [&](const auto& object)
		{ return object.Hits.empty() && object.PresentationPulses.size() == 1u && !object.bTrackLockedTargetUntilFirstPulse &&
			object.LiveState.CurrentPose.fPositionY == 1.f && object.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision; }),
		"Albion stages one grounded presentation per living player with no damage or fabricated boss");
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> restored;
	room->m_CombatObjectRuntime.Build_LiveSpawnMessages(1030u, restored);
	std::vector<COMBAT_OBJECT_SNAPSHOT> snapshots;
	bool wireValid = restored.size() == 2u && room->m_CombatObjectRuntime.Build_Snapshots(snapshots) && snapshots.size() == 2u;
	for (const auto& spawn : restored)
	{
		CPacketWriter writer; S2C_COMBAT_OBJECT_SPAWNED roundTrip;
		wireValid = Write_Message(writer, spawn) && wireValid;
		CPacketReader reader{ writer.Get_Buffer() };
		wireValid = Read_Message(reader, roundTrip) && wireValid && roundTrip.iSpawnTick == 1000u && roundTrip.iServerTick == 1030u &&
			roundTrip.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision && roundTrip.fPositionY == 1.f;
	}
	tests.Require(wireValid, "Albion reconnect and snapshot retain the original spawn clock and pinned revision without catalog object lookup");
	room->m_CombatObjectRuntime.Discard_PendingLifecycle();
	const auto nextBeforeFailure = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
	room->m_Players[2u].fPositionX = -1000.f;
	queueAlbion(1001u);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 2u &&
		room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == nextBeforeFailure && room->m_PendingKoukuMechanicTriggers.empty(),
		"Invalid Albion point rolls back the entire volley after valid players stage, preserving objects and IDs");
	room->m_CombatObjectRuntime.Reset();
	room->m_Players[2u].isCombatReady = false;
	albion.iCountPerPlayer = 4u; albion.fPlayerEffectRadiusM = 2.f;
	queueAlbion(1100u);
	const auto& radialObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	tests.Require(radialObjects.size() == 4u && std::all_of(radialObjects.begin(), radialObjects.end(), [](const auto& object)
		{ const auto& pose = object.LiveState.CurrentPose; const float x = pose.fPositionX - 6.f, z = pose.fPositionZ - 6.f;
		  return std::abs(x*x + z*z - 4.f) < .001f && pose.fPositionY == 1.f; }),
		"Albion radial authoring reuses the volley layout and excludes non-ready and dead players");
	std::vector<DAMAGE_EVENT> albionDamage;
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 2.f, 1160u, albionDamage);
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
	std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulses;
	std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
	room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, pulses, despawned);
	tests.Require(albionDamage.empty() && pulses.size() == 4u && std::all_of(pulses.begin(), pulses.end(), [](const auto& pulse)
		{ return pulse.eKind == COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE && pulse.strHitId == "combatpresentation.kouku.albion.started"; }) &&
		room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u,
		"Albion emits only its admitted lifecycle marker and keeps the authored explosion tail alive");
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 5.f, 1310u, albionDamage);
	tests.Require(albionDamage.empty() && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
		"Albion expires at the authored effect lifetime without damage");

	for (const bool completed : { true, false })
	{
		room->m_CombatObjectRuntime.Reset();
		albionOwner.strPatternId = "KOUKUSAYDON_TEST_ALBION";
		albionOwner.iPatternSequence = 2u;
		queueAlbion(1400u);
		room->m_KoukuSaydonPatternAudition.Members.clear();
		auto& member = room->m_KoukuSaydonPatternAudition.Members.emplace_back();
		member.strMemberId = "test.albion.owner"; member.iBossEntityId = albionOwner.iNetEntityId;
		member.iPatternSequence = albionOwner.iPatternSequence; member.PatternIds = { albionOwner.strPatternId };
		room->Clear_KoukuSaydonPatternAudition(completed);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == (completed ? 4u : 0u),
			completed ? "Natural Kouku completion retains the Albion visual tail" : "Explicit Kouku stop cancels the owned Albion objects");
	}

	// A blue-circle wave freezes one selected player and five independent nav points.
	room->m_CombatObjectRuntime.Reset();
	albionOwner.strPatternId = "KOUKUSAYDON_TEST_ALBION";
	albionOwner.iPatternSequence = 3u;
	albionOwner.fSpawnPositionX = 8.f; albionOwner.fSpawnPositionY = 1.f; albionOwner.fSpawnPositionZ = 8.f;
	room->m_ServerNavigation.Set_RuntimeSupportSurfaces({ { "albion.floor", 8.f, 8.f, 30.f, 1.6f } }, status);
	room->m_Players.clear();
	for (PLAYER_ID id = 1u; id <= 7u; ++id)
	{
		auto& target = room->m_Players[id];
		target.iPlayerId = id; target.iNetEntityId = 100u + id;
		target.iCurrentHp = id == 3u ? 0u : 100u; target.iMaximumHp = 100u;
		target.isCombatReady = id != 4u;
		target.eAction = id == 5u ? PLAYER_ACTION_STATE::DEAD :
			id == 6u ? PLAYER_ACTION_STATE::FALLING : PLAYER_ACTION_STATE::NONE;
		target.iMarioStage = id == 7u ? 1u : 0u;
		target.fPositionX = id == 1u ? 6.f : 10.f; target.fPositionY = 99.f; target.fPositionZ = 6.f;
	}
	albion.iCountPerPlayer = 1u; albion.fPlayerEffectRadiusM = 0.f;
	albion.bRandomPlayerOnly = true; albion.iArenaRandomCount = 5u;
	albion.fArenaRandomRadiusM = 6.f; albion.fArenaHeightToleranceM = 1.f; albion.fArenaMinimumSpacingM = 3.2f;
	queueAlbion(1600u);
	const auto readPoses = [&]()
	{
		std::vector<std::array<float, 3u>> result;
		for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
		{
			const auto& pose = object.LiveState.CurrentPose;
			result.push_back({ pose.fPositionX, pose.fPositionY, pose.fPositionZ });
		}
		return result;
	};
	const auto firstPoses = readPoses();
	const auto& sixObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	std::size_t targetedCount = 0u, arenaCount = 0u;
	bool admittedSix = sixObjects.size() == 6u;
	for (const auto& object : sixObjects)
	{
		const auto& pose = object.LiveState.CurrentPose;
		SERVER_NAV_POINT sample{};
		admittedSix = admittedSix && object.Hits.empty() && !object.bTrackLockedTargetUntilFirstPulse &&
			room->m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) &&
			room->m_ServerNavigation.Sample_Position(pose.fPositionX, pose.fPositionZ, sample) && pose.fPositionY == sample.y;
		if (object.iLockedTargetNetEntityId != INVALID_NET_ENTITY_ID)
		{
			++targetedCount;
			admittedSix = admittedSix && (object.iLockedTargetNetEntityId == 101u || object.iLockedTargetNetEntityId == 102u) &&
				pose.fPositionZ == 6.f && pose.fPositionX == (object.iLockedTargetNetEntityId == 101u ? 6.f : 10.f);
		}
		else
		{
			++arenaCount;
			const float x = pose.fPositionX - 8.f, z = pose.fPositionZ - 8.f;
			admittedSix = admittedSix && x*x + z*z <= 36.001f;
			for (const auto& otherObject : sixObjects)
			{
				if (otherObject.iCombatObjectId >= object.iCombatObjectId || otherObject.iLockedTargetNetEntityId != INVALID_NET_ENTITY_ID) continue;
				const float dx = pose.fPositionX - otherObject.LiveState.CurrentPose.fPositionX;
				const float dz = pose.fPositionZ - otherObject.LiveState.CurrentPose.fPositionZ;
				admittedSix = admittedSix && dx*dx + dz*dz + .001f >= 3.2f * 3.2f;
			}
		}
	}
	tests.Require(admittedSix && targetedCount == 1u && arenaCount == 5u,
		"Albion admits exactly five spaced nav points and one living ready non-Mario player on exact supported ground");
	room->m_CombatObjectRuntime.Build_LiveSpawnMessages(1601u, restored);
	tests.Require(restored.size() == 6u && std::all_of(restored.begin(), restored.end(), [&](const auto& spawn)
		{ return spawn.fPositionY == 1.6f && spawn.iSpawnTick == 1600u && spawn.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision; }),
		"The six Albion reconnect spawns preserve their sampled floor, effect clock and pinned revision");
	room->m_Players[1u].fPositionX += 1.f; room->m_Players[2u].fPositionX += 1.f;
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, .1f, 1603u, albionDamage);
	tests.Require(readPoses() == firstPoses && albionDamage.empty(),
		"Moving either eligible player after spawn leaves all six Albion points fixed and deals no damage");
	room->m_Players[1u].fPositionX -= 1.f; room->m_Players[2u].fPositionX -= 1.f;
	room->m_CombatObjectRuntime.Reset();
	queueAlbion(1600u);
	tests.Require(readPoses() == firstPoses, "The same Albion trigger and pattern occurrence resolve a deterministic six-point volley");
	std::set<NET_ENTITY_ID> selectedTargets;
	bool wavesChanged = false;
	for (std::uint32_t wave = 1u; wave <= 12u; ++wave)
	{
		room->m_CombatObjectRuntime.Reset();
		albion.strTriggerId = "test.albion.logic.wave." + std::to_string(wave);
		queueAlbion(1700u);
		wavesChanged = wavesChanged || readPoses() != firstPoses;
		for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
			if (object.iLockedTargetNetEntityId != INVALID_NET_ENTITY_ID) selectedTargets.insert(object.iLockedTargetNetEntityId);
	}
	tests.Require(wavesChanged && selectedTargets == std::set<NET_ENTITY_ID>{ 101u, 102u },
		"Distinct authored Albion triggers vary the wave and can choose either eligible player");
	const auto beforeRejectedWave = readPoses();
	const auto beforeRejectedId = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
	room->m_Players[1u].fPositionX = room->m_Players[2u].fPositionX = -1000.f;
	queueAlbion(1800u);
	tests.Require(readPoses() == beforeRejectedWave && room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeRejectedId,
		"A selected player outside navigation rejects all six newly staged Albion objects and preserves live IDs");
	room->m_Players[1u].fPositionX = 6.f; room->m_Players[2u].fPositionX = 10.f;
	albion.fArenaRandomRadiusM = 1.f; albion.fArenaMinimumSpacingM = 20.f;
	queueAlbion(1801u);
	tests.Require(readPoses() == beforeRejectedWave && room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeRejectedId,
		"An impossible five-point spacing request rejects the whole Albion wave and preserves live IDs");
	albion.fArenaRandomRadiusM = 6.f; albion.fArenaMinimumSpacingM = 3.2f;
	room->m_Players[1u].isCombatReady = room->m_Players[2u].isCombatReady = false;
	albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
	queueAlbion(1802u);
	const auto fallbackPoses = readPoses();
	const auto& fallbackObjects = room->m_CombatObjectRuntime.Get_LiveObjects();
	tests.Require(fallbackPoses.size() == beforeRejectedWave.size() + 6u &&
		std::equal(beforeRejectedWave.begin(), beforeRejectedWave.end(), fallbackPoses.begin()) &&
		fallbackPoses[beforeRejectedWave.size()] == std::array<float, 3u>{6.f, 1.6f, 6.f} &&
		std::all_of(fallbackObjects.begin() + beforeRejectedWave.size(), fallbackObjects.end(), [](const auto& object) {
			return object.iLockedTargetNetEntityId == INVALID_NET_ENTITY_ID && !object.bTrackLockedTargetUntilFirstPulse; }),
		"An empty arena adds one boss-ground circle and five random points while excluding its living Mario player and preserving the previous wave");

	// Pursuit shares the actual room clock and combat-object lifecycle; it owns no damage.
	{
		room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		room->m_Players.clear();
		auto& target = room->m_Players[1u];
		target.iPlayerId = 1u; target.iNetEntityId = 101u; target.iCurrentHp = target.iMaximumHp = 100u;
		target.isCombatReady = true; target.fPositionX = 10000.f; target.fPositionZ = 10000.f;
		albionOwner.iCurrentHp = 100u; albionOwner.eAction = SERVER_ENTITY_ACTION::IDLE;
		albionOwner.strPatternId = "KOUKUSAYDON_TEST_PURSUIT"; albionOwner.iPatternSequence = 70u;
		albionOwner.iPatternTargetEntityId = target.iNetEntityId;
		albionOwner.fPositionX = albionOwner.fPositionZ = 0.f;
		BOSS_PATTERN_MECHANIC_TRIGGER pursuit;
		pursuit.strTriggerId = "test.pursuit"; pursuit.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
		pursuit.iStartMs = 100u; pursuit.iDurationMs = 901u;
		pursuit.ProjectileVisualIds = { "test.heart", "test.clover", "test.diamond", "test.spade" };
		pursuit.strContactVisualId = "test.card.explosion";
		pursuit.fProjectileSpeedMps = 1.f; pursuit.fProjectileContactRadiusM = .4f;
		pursuit.fProjectileSpawnRadiusM = 2.f; pursuit.iProjectileCountPerWave = 4u;
		pursuit.bProjectileHoming = true;
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = albionOwner.strPatternId; pattern.MechanicTriggers = { pursuit };
		KOUKUSAYDON_LOGIC_LEDGER ledger;
		CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6000u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6002u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Pursuit respects the authored spawn start");
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6003u);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6004u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u, "One infinite homing wave creates four distinct visual objects once");
		std::vector<DAMAGE_EVENT> damage;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 601.f, 6005u, damage);
		const auto firstPose = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
		target.fPositionX = -10000.f;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6006u, damage);
		const auto& moving = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(moving.size() == 4u && moving.front().LiveState.CurrentPose.fPositionX < firstPose.fPositionX && damage.empty(),
			"Infinite cards survive the old ten-minute lifetime and turn toward the current authoritative target without damage");
		room->Clear_KoukuPlayerTargets(albionOwner, ledger);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u, "Natural pattern completion leaves persistent cards alive");
		target.iCurrentHp = 0u;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f / 30.f, 6007u, damage);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Target death retires every homing card");
		target.iCurrentHp = 100u;
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		pursuit.iStartMs = 0u; pursuit.iProjectileLifetimeMs = 5000u; pursuit.iSpawnIntervalMs = 300u;
		pursuit.iProjectileCountPerWave = 3u; pursuit.bProjectileHoming = false; pursuit.fProjectileSpeedMps = 15.f;
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6100u, ledger);
		for (std::uint32_t tick = 6100u; tick <= 6128u; ++tick)
			room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, tick);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 12u, "Source 300 ms cadence emits three cards in each of four waves");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 5.1f, 6129u, damage);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Finite spinning cards retire at their authored five-second lifetime");
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> expiredSpawns;
		std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> expiredEvents;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> expiredObjects;
		room->m_CombatObjectRuntime.Drain_Lifecycle(expiredSpawns, expiredEvents, expiredObjects);
		// The same drain includes the admitted birth pulses, not just terminal bursts.
		tests.Require(expiredEvents.size() == 24u && expiredObjects.size() == 12u &&
			std::count_if(expiredEvents.begin(), expiredEvents.end(), [](const auto& event) {
				return event.strHitId == "combatpresentation.kouku.pursuit.started"; }) == 12 &&
			std::count_if(expiredEvents.begin(), expiredEvents.end(), [&](const auto& event) {
				return event.strHitId == pursuit.strContactVisualId; }) == 12,
			"Every finite card emits one birth pulse and its shared burst exactly once at lifetime end");
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		// Source Speed is 8 m/s; MaxDistance is independent of its five-second lifetime.
		pursuit.iDurationMs = 4100u; pursuit.fProjectileSpeedMps = 8.f; pursuit.fProjectileMaxDistanceM = 15.f;
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6150u, ledger);
		for (std::uint32_t tick = 6150u; tick <= 6273u; ++tick)
			room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, tick);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 42u,
			"Current 4.8-second spin extends source cadence to fourteen three-card waves, with no wave past the window");
		const auto distanceOrigin = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6274u, damage);
		const auto& travelling = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(travelling.size() == 42u && std::abs(travelling.front().fRemainingDistanceM - 7.f) < .0001f &&
			std::abs(std::hypot(travelling.front().LiveState.CurrentPose.fPositionX - distanceOrigin.fPositionX,
				travelling.front().LiveState.CurrentPose.fPositionZ - distanceOrigin.fPositionZ) - 8.f) < .0001f,
			"Spinning cards move eight metres in one second and retain the remaining seven metres");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6275u, damage);
		expiredSpawns.clear(); expiredEvents.clear(); expiredObjects.clear();
		room->m_CombatObjectRuntime.Drain_Lifecycle(expiredSpawns, expiredEvents, expiredObjects);
		std::erase_if(expiredEvents, [&](const auto& event) { return event.strHitId != pursuit.strContactVisualId; });
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && expiredEvents.size() == 42u && expiredObjects.size() == 42u &&
			std::abs(std::hypot(expiredEvents.front().fPositionX - distanceOrigin.fPositionX,
				expiredEvents.front().fPositionZ - distanceOrigin.fPositionZ) - 15.f) < .0001f,
			"Distance-limited cards stop at fifteen metres and burst exactly once before the five-second lifetime");
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		pursuit.fProjectileMaxDistanceM = 0.f; pursuit.fProjectileSpeedMps = 15.f;
		pursuit.iSpawnIntervalMs = 0u; pursuit.iProjectileCountPerWave = 1u;
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6200u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6200u);
		const auto contactPose = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
		target.fPositionX = contactPose.fPositionX + contactPose.fDirectionX * 7.5f;
		target.fPositionZ = contactPose.fPositionZ + contactPose.fDirectionZ * 7.5f;
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6201u, damage);
		std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned; std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
		std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
		room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, events, despawned);
		tests.Require(events.size() == 2u && std::count_if(events.begin(), events.end(), [](const auto& event) {
			return event.strHitId == "combatpresentation.kouku.pursuit.started"; }) == 1,
			"Swept card contact preserves its single birth pulse alongside the terminal burst");
		std::erase_if(events, [](const auto& event) { return event.strHitId == "combatpresentation.kouku.pursuit.started"; });
		tests.Require(events.size() == 1u && events.front().strHitId == pursuit.strContactVisualId && despawned.size() == 1u &&
			events.front().fPositionX == target.fPositionX && events.front().fPositionZ == target.fPositionZ &&
			room->m_CombatObjectRuntime.Get_LiveObjects().empty() && damage.empty(), "Swept card contact emits one pinned reliable explosion and retires the object");
		// The new P78 PLAY rows are four independent, finite 4.5-second homing windows.
		room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		target.fPositionX = target.fPositionZ = 10000.f;
		pattern.MechanicTriggers.clear();
		for (std::uint32_t suit = 0u; suit < 4u; ++suit)
		{
			auto card = pursuit; card.strTriggerId = "p78.card." + std::to_string(suit);
			card.iStartMs = 0u; card.iDurationMs = card.iProjectileLifetimeMs = 4500u;
			card.ProjectileVisualIds = {pursuit.ProjectileVisualIds[suit]};
			card.iProjectileCountPerWave = 1u; card.bProjectileHoming = true;
			card.fProjectileContactRadiusM = 1.25f; card.fProjectileSpeedMps = 1.f;
			pattern.MechanicTriggers.push_back(card);
		}
		CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6250u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6250u);
		const auto& finiteCards = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(finiteCards.size() == 4u && std::all_of(finiteCards.begin(), finiteCards.end(), [](const auto& card) {
			return card.bHoming && !card.bPersistentLifetime && card.fRemainingMilliseconds == 4500.f &&
				card.fContactPresentationRadiusM == 1.25f && card.Hits.empty(); }),
			"Four P78 windows each create one finite 4500ms homing card with 1.25m radius and no damage");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 4.499f, 6384u, damage);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().size() == 4u,
			"P78 cards remain alive immediately before their saved 4.5-second lifetime");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, .002f, 6385u, damage);
		spawned.clear(); events.clear(); despawned.clear();
		room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, events, despawned);
		std::erase_if(events, [&](const auto& event) { return event.strHitId != pursuit.strContactVisualId; });
		tests.Require(events.size() == 4u && despawned.size() == 4u && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
			"Each finite P78 card expires with one reliable burst and one despawn at its saved lifetime");
		room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f, 6386u, damage);
		spawned.clear(); events.clear(); despawned.clear();
		room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, events, despawned);
		tests.Require(events.empty() && despawned.empty(), "Retired finite cards cannot emit duplicate terminal events");
		pattern.MechanicTriggers = {pursuit};
		pattern.MechanicTriggers.front().bProjectileHoming = true;
		CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6300u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6300u);
		room->m_CombatObjectRuntime.Cancel_Source(albionOwner.iNetEntityId);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Explicit Stop uses source cancellation for surviving cards");
		room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
		pursuit.iStartMs = 913u; pursuit.iDurationMs = 1u; pursuit.iProjectileLifetimeMs = 0u;
		pursuit.iSpawnIntervalMs = 0u; pursuit.iProjectileCountPerWave = 1u; pursuit.bProjectileHoming = true;
		pursuit.fProjectileMaxDistanceM = 0.f; pursuit.fProjectileSpeedMps = 1.f;
		albionOwner.fYawDegrees = 0.f; albionOwner.fPositionX = albionOwner.fPositionZ = 0.f;
		target.fPositionX = target.fPositionZ = 10000.f; target.iCurrentHp = 100u;
		LostArk::Shared::ATTACK_HIT_TEMPLATE fullHit;
		fullHit.strHitId = "card.full-lifetime"; fullHit.strTrigger = "CONTACT";
		fullHit.iEndMs = 600000u; fullHit.fRadiusM = .4; fullHit.iDamagePercent = 10u;
		auto shortHit = fullHit; shortHit.strHitId = "card.short-window"; shortHit.iEndMs = 1000u;
		pursuit.ProjectileHits = { fullHit, shortHit };
		pattern.MechanicTriggers = { pursuit }; CKoukuSaydonLogicRuntime::Build(pattern, albionOwner, 6400u, ledger);
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6427u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Sub-tick pursuit Trigger never emits before its authored start");
		room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6428u);
		const auto& persistentCards = room->m_CombatObjectRuntime.Get_LiveObjects();
		tests.Require(persistentCards.size() == 1u, "One-ms Trigger produces one card on its quantized birth tick");
		if (!persistentCards.empty())
		{
			const auto& card = persistentCards.front();
			tests.Require(std::abs(card.LiveState.CurrentPose.fPositionX - pursuit.fProjectileSpawnRadiusM) < .001f &&
				std::abs(card.LiveState.CurrentPose.fPositionZ) < .001f, "Pursuit spawn is Saydon +X front, 180 degrees opposite the old rear origin");
			tests.Require(card.Hits.size() == 2u && card.Hits[0].iEndMs == 0u && card.Hits[1].iEndMs == 1000u,
				"Persistent pursuit extends only full-lifetime CONTACT hits and preserves shorter authored windows");
			room->Update_KoukuPlayerTargets(albionOwner, pattern, ledger, room->m_GameplayCatalog, 6429u);
			damage.clear();
			room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 601.f, 6430u, damage);
			if (!room->m_CombatObjectRuntime.Get_LiveObjects().empty())
			{
				const auto pose = room->m_CombatObjectRuntime.Get_LiveObjects().front().LiveState.CurrentPose;
				target.fPositionX = pose.fPositionX; target.fPositionY = pose.fPositionY; target.fPositionZ = pose.fPositionZ;
				room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 1.f / 30.f, 6431u, damage);
			}
			tests.Require(damage.size() == 1u && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
				"Persistent card still damages and despawns on contact after ten minutes");
		}
		room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle(); room->m_Players.clear();
	}
	// Showtime duration uses the same room-owned roster, clock, ground and object lifecycle.
	room->m_CombatObjectRuntime.Reset();
	room->m_ServerNavigation.Set_RuntimeSupportSurfaces({}, status);
	room->m_Players.clear();
	for (PLAYER_ID id = 1u; id <= 3u; ++id)
	{
		auto& target = room->m_Players[id];
		target.iPlayerId = id; target.iNetEntityId = 100u + id;
		target.iCurrentHp = id == 3u ? 0u : 100u; target.iMaximumHp = 100u; target.isCombatReady = true;
		target.fPositionX = id == 2u ? 10.f : 6.f; target.fPositionY = 99.f; target.fPositionZ = 6.f;
		target.fMoveSpeed = id == 2u ? 12.f : 6.f;
	}
	BOSS_PATTERN_DEFINITION showtime;
	showtime.strPatternId = "KOUKUSAYDON_TEST_SHOWTIME";
	BOSS_PATTERN_MECHANIC_TRIGGER targets{};
	targets.strTriggerId = "test.showtime.duration"; targets.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS;
	targets.iDurationMs = 5000u; targets.strFixedVisualId = "test.showtime.fixed";
	targets.strTrackingVisualId = "test.showtime.tracking"; targets.iFixedLifetimeMs = 4384u;
	targets.iSpawnIntervalMs = 2000u; targets.fFollowSpeedScale = .5f;
	showtime.MechanicTriggers.push_back(targets);
	// Saydon model +X is already aimed at the +X target when its body yaw is zero.
	albionOwner.strPatternId = showtime.strPatternId; albionOwner.iPatternSequence = 4u; albionOwner.fYawDegrees = 0.f;
	KOUKUSAYDON_LOGIC_LEDGER showtimeLedger;
	CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2000u, showtimeLedger);
	const auto updateTargets = [&](const std::uint32_t tick) {
		room->Update_KoukuPlayerTargets(albionOwner, showtime, showtimeLedger, room->m_GameplayCatalog, tick);
	};
	const auto countTargetObjects = [&](const bool tracking) {
		return std::count_if(room->m_CombatObjectRuntime.Get_LiveObjects().begin(), room->m_CombatObjectRuntime.Get_LiveObjects().end(),
			[&](const auto& object) { return object.strCombatObjectArchetypeId == (tracking ? "combatobject.kouku.showtime.tracking" : "combatobject.kouku.showtime.fixed"); });
	};
	const auto targetPose = [&](const COMBAT_OBJECT_ID id) {
		for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects()) if (object.iCombatObjectId == id) return object.LiveState.CurrentPose;
		return SERVER_COMBAT_OBJECT_POSE{};
	};
	updateTargets(1999u);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeLedger.MechanicTriggers.empty() &&
		showtimeLedger.PlayerTargetWindows.size() == 1u, "Showtime is a duration window and creates nothing before its start");
	albionOwner.iPatternTargetEntityId = 102u;
	albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
	updateTargets(2000u);
	tests.Require(albionOwner.iPatternTargetEntityId == 102u && std::abs(albionOwner.fYawDegrees) < .0001f,
		"Showtime keeps the existing server pattern target and the already aligned Saydon +X yaw, independently of roster order");
	auto& trackingIds = showtimeLedger.PlayerTargetWindows.front().TrackingObjects;
	tests.Require(countTargetObjects(false) == 2 && countTargetObjects(true) == 2 && trackingIds.size() == 2u &&
		std::all_of(room->m_CombatObjectRuntime.Get_LiveObjects().begin(), room->m_CombatObjectRuntime.Get_LiveObjects().end(),
			[](const auto& object) { return object.Hits.empty() && object.LiveState.CurrentPose.fPositionY == 1.f && object.LiveState.CurrentPose.fYawDegrees == 0.f; }),
		"Showtime tick zero creates a fixed group and one independent grounded tracker for every living player, with no damage or boss yaw");
	const auto firstTracker = trackingIds.at(1u), secondTracker = trackingIds.at(2u);
	const auto firstBirth = targetPose(firstTracker), secondBirth = targetPose(secondTracker);
	tests.Require(firstBirth.fPositionX == albionOwner.fSpawnPositionX && firstBirth.fPositionZ == albionOwner.fSpawnPositionZ &&
		secondBirth.fPositionX == albionOwner.fSpawnPositionX && secondBirth.fPositionZ == albionOwner.fSpawnPositionZ,
		"Each Showtime tracker starts at the authored arena centre rather than at its player");
	room->m_Players[1u].fPositionX = 9.f; room->m_Players[2u].fPositionX = 13.f;
	updateTargets(2001u);
	const auto firstStep = targetPose(firstTracker), secondStep = targetPose(secondTracker);
	tests.Require(std::abs(std::hypot(firstStep.fPositionX - firstBirth.fPositionX, firstStep.fPositionZ - firstBirth.fPositionZ) - .1f) < .0001f &&
		std::abs(std::hypot(secondStep.fPositionX - secondBirth.fPositionX, secondStep.fPositionZ - secondBirth.fPositionZ) - .2f) < .0001f &&
		std::hypot(9.f - firstStep.fPositionX, 6.f - firstStep.fPositionZ) < std::hypot(9.f - firstBirth.fPositionX, 6.f - firstBirth.fPositionZ) &&
		std::hypot(13.f - secondStep.fPositionX, 6.f - secondStep.fPositionZ) < std::hypot(13.f - secondBirth.fPositionX, 6.f - secondBirth.fPositionZ),
		"Each tracker advances toward its own player by half that player's effective speed per 30 Hz tick");
	updateTargets(2001u);
	tests.Require(targetPose(firstTracker).fPositionX == firstStep.fPositionX && targetPose(firstTracker).fPositionZ == firstStep.fPositionZ &&
		targetPose(secondTracker).fPositionX == secondStep.fPositionX && targetPose(secondTracker).fPositionZ == secondStep.fPositionZ &&
		countTargetObjects(false) == 2 && countTargetObjects(true) == 2,
		"Repeating the same server tick neither moves either tracker twice nor duplicates a fixed volley");
	room->m_CombatObjectRuntime.Build_LiveSpawnMessages(2001u, restored);
	wireValid = restored.size() == 4u && room->m_CombatObjectRuntime.Build_Snapshots(snapshots) && snapshots.size() == 4u;
	for (const auto& spawn : restored)
	{
		CPacketWriter writer; S2C_COMBAT_OBJECT_SPAWNED roundTrip;
		wireValid = Write_Message(writer, spawn) && wireValid;
		CPacketReader reader{ writer.Get_Buffer() };
		wireValid = Read_Message(reader, roundTrip) && wireValid && roundTrip.iSpawnTick == 2000u && roundTrip.iServerTick == 2001u &&
			roundTrip.PinnedDefinitionRevision == albionOwner.PinnedDefinitionRevision;
	}
	tests.Require(wireValid, "Showtime fixed and tracking instances round-trip through existing spawn/snapshot and late-join clock metadata");
	room->m_Players[2u].iCurrentHp = 0u;
	auto joinedPlayer = room->m_Players[1u]; joinedPlayer.iPlayerId = 4u; joinedPlayer.iNetEntityId = 104u;
	joinedPlayer.fPositionX = 8.f; joinedPlayer.fPositionZ = 10.f; room->m_Players[4u] = joinedPlayer;
	updateTargets(2002u);
	tests.Require(albionOwner.iPatternTargetEntityId != 102u && albionOwner.bHasPatternTargetLastPosition &&
		(albionOwner.iPatternTargetEntityId == 101u || albionOwner.iPatternTargetEntityId == 104u),
		"Death reselects one living target through the existing random-alive server policy");
	tests.Require(trackingIds.size() == 2u && !trackingIds.contains(2u) && trackingIds.contains(4u) && countTargetObjects(false) == 2,
		"Death removes only that player's tracker; joining creates one tracker without an early fixed volley");
	auto returningPlayer = room->m_Players[1u]; room->m_Players.erase(1u); updateTargets(2003u);
	tests.Require(!trackingIds.contains(1u) && trackingIds.size() == 1u && countTargetObjects(false) == 2, "Leaving removes the tracker and preserves already fixed groups");
	room->m_Players[1u] = returningPlayer; room->m_Players[2u].iCurrentHp = 100u; updateTargets(2004u);
	tests.Require(trackingIds.size() == 3u && trackingIds.at(1u) != firstTracker && trackingIds.at(2u) != secondTracker,
		"Return and revival allocate fresh independent tracking instances");
	room->m_Players[3u].iCurrentHp = 100u; updateTargets(2005u);
	tests.Require(trackingIds.size() == 4u && countTargetObjects(true) == 4,
		"A fourth living server player receives exactly one tracker");
	updateTargets(2059u);
	tests.Require(countTargetObjects(false) == 2, "No fixed group is emitted between the authored two-second ticks");
	updateTargets(2060u); updateTargets(2120u);
	tests.Require(countTargetObjects(false) == 10 && countTargetObjects(true) == 4,
		"Ticks 60 and 120 create one fixed group per currently living player without duplicating trackers");
	bool fixedFrozen = true;
	for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
		if (object.strCombatObjectArchetypeId == "combatobject.kouku.showtime.fixed" && object.iSpawnTick == 2000u)
			fixedFrozen = fixedFrozen && object.LiveState.CurrentPose.fPositionX == (object.iLockedTargetNetEntityId == 101u ? 6.f : 10.f);
	tests.Require(fixedFrozen, "The first fixed groups remain at their sampled positions while their players move");
	albionOwner.fYawDegrees = -123.f;
	updateTargets(2150u);
	tests.Require(albionOwner.fYawDegrees == -123.f, "Showtime stops updating boss yaw at the exclusive duration end");
	tests.Require(countTargetObjects(false) == 10 && countTargetObjects(true) == 0 && trackingIds.empty(),
		"The exclusive duration end creates no volley, removes all trackers and retains authored fixed tails");
	std::vector<DAMAGE_EVENT> showtimeDamage;
	room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 4.4f, 2282u, showtimeDamage);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeDamage.empty(),
		"Fixed groups expire through the ordinary authored lifetime and never apply gameplay damage");

	for (const bool trackingOnly : { false, true })
	{
		room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = targets;
		if (trackingOnly) { showtime.MechanicTriggers.front().strFixedVisualId.clear(); showtime.MechanicTriggers.front().iFixedLifetimeMs = 0u; }
		else showtime.MechanicTriggers.front().strTrackingVisualId.clear();
		albionOwner.fYawDegrees = -123.f;
		const auto targetBeforeDrop = albionOwner.iPatternTargetEntityId;
		CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2300u, showtimeLedger); updateTargets(2300u);
		if (!trackingOnly) tests.Require(albionOwner.fYawDegrees == -123.f && albionOwner.iPatternTargetEntityId == targetBeforeDrop,
			"Fixed-only player drops preserve the authored boss yaw and target during rope motion");
		tests.Require(countTargetObjects(trackingOnly) == 4 && countTargetObjects(!trackingOnly) == 0,
			trackingOnly ? "A tracking-only duration creates no fixed groups" : "A fixed-only duration allocates no trackers");
	}
	room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = targets;
	CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2400u, showtimeLedger);
	const auto targetIdBeforeFailure = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
	room->m_Players[2u].fPositionX = -1000.f; updateTargets(2400u);
	tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() &&
		room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == targetIdBeforeFailure &&
		showtimeLedger.PlayerTargetWindows.front().TrackingObjects.empty(),
		"An invalid later player rolls back all newly staged Showtime instances and preserves stable IDs");
	room->m_Players[2u].fPositionX = 13.f; updateTargets(2401u);
	tests.Require(countTargetObjects(false) == 4 && countTargetObjects(true) == 4,
		"A rejected target transaction retries after valid ground returns");
	albionOwner.iPatternTargetEntityId = 101u; albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
	room->m_Players[1u].fPositionX = 9.f; room->m_Players[1u].fPositionZ = 9.f;
	updateTargets(2402u);
	tests.Require(albionOwner.iPatternTargetEntityId == 101u && std::abs(albionOwner.fYawDegrees + 45.f) < .0001f,
		"The existing SHOWTIME_PLAYER_TARGETS immediately aims Saydon model +X independently of the rotate-only duration");
	for (const bool completed : { true, false })
	{
		room->m_CombatObjectRuntime.Reset(); albionOwner.strPatternId = showtime.strPatternId;
		CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 2500u, showtimeLedger); updateTargets(2500u);
		room->m_KoukuSaydonPatternAudition.Members.clear();
		auto& member = room->m_KoukuSaydonPatternAudition.Members.emplace_back();
		member.strMemberId = "test.showtime.owner"; member.iBossEntityId = albionOwner.iNetEntityId;
		member.iPatternSequence = albionOwner.iPatternSequence; member.PatternIds = { showtime.strPatternId }; member.LogicLedger = showtimeLedger;
		room->Clear_KoukuSaydonPatternAudition(completed);
		tests.Require(countTargetObjects(true) == 0 && countTargetObjects(false) == (completed ? 4 : 0),
			completed ? "Natural completion clears Showtime trackers while retaining fixed tails" : "Explicit stop cancels every owned Showtime instance");
	}


	// Rotation consumes the same duration clock as the target window, not render frames.
	{
		const auto savedPlayers = room->m_Players;
		const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		room->m_Players.clear();
		auto& target = room->m_Players[1u];
		target.iPlayerId = 1u; target.iNetEntityId = 101u; target.iCurrentHp = target.iMaximumHp = 100u;
		target.isCombatReady = true; target.fMoveSpeed = 6.f; target.fPositionY = 1.f;
		const auto setDirection = [&](const float yaw) {
			const double radians = double(yaw) * 3.14159265358979323846 / 180.0;
			target.fPositionX = 8.f + static_cast<float>(std::sin(radians));
			target.fPositionZ = 8.f + static_cast<float>(std::cos(radians));
		};
		const auto beginFacing = [&](const float yaw, const std::uint32_t durationMs) {
			room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = {};
			showtime.MechanicTriggers.front().strTriggerId = "test.boss.track.duration";
			showtime.MechanicTriggers.front().eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
			showtime.MechanicTriggers.front().iDurationMs = durationMs;
			albionOwner.strPatternId = showtime.strPatternId; albionOwner.strArchetypeId = "BOSS_KOUKUSAYDON_G1_KOUKU";
			albionOwner.fPositionX = albionOwner.fPositionZ = 8.f; albionOwner.fYawDegrees = yaw;
			albionOwner.iPatternTargetEntityId = albionOwner.iTargetEntityId = 101u;
			CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 5000u, showtimeLedger);
		};
		const auto nearYaw = [](const float actual, const float expected) {
			return std::abs(std::remainder(actual - expected, 360.f)) < .001f;
		};
		setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(4999u);
		tests.Require(albionOwner.fYawDegrees == 0.f, "Showtime interpolation preserves yaw before the duration begins");
		updateTargets(5000u);
		tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeLedger.MechanicTriggers.empty() &&
			showtimeLedger.PlayerTargetWindows.size() == 1u, "Rotate-only duration uses the actual clock without visual templates or spawned objects");
		tests.Require(nearYaw(albionOwner.fYawDegrees, 30.f), "Rotate-only tracking reacts on the first tick with ten times the previous three-degree step");
		updateTargets(5000u); updateTargets(4999u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 30.f), "Duplicate and older ticks do not rotate the target twice");
		for (std::uint32_t tick = 5001u; tick <= 5014u; ++tick) updateTargets(tick);
		const float singleTickHalfYaw = albionOwner.fYawDegrees;
		tests.Require(nearYaw(singleTickHalfYaw, 89.9910045f), "The accelerated response reaches the fixed target early without overshooting");
		beginFacing(0.f, 1000u); updateTargets(5000u); updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, singleTickHalfYaw), "The same elapsed ticks give the same yaw when updates are grouped");
		updateTargets(5029u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "The last valid duration tick reaches the current target exactly");
		setDirection(-90.f); updateTargets(5030u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "The exclusive duration end cannot turn toward a changed target");
		setDirection(90.f); beginFacing(0.f, 2000u); updateTargets(5000u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 15.f), "A two-second window retains ten times the previous 1.5-degree first step");
		setDirection(-179.f); beginFacing(179.f, 1000u); updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -179.0001999f), "Crossing positive 180 degrees uses the short two-degree arc");
		setDirection(179.f); beginFacing(-179.f, 1000u); updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 179.0001999f), "Crossing negative 180 degrees uses the short reverse arc");
		setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(5014u); setDirection(-90.f); updateTargets(5015u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -30.0029985f) && albionOwner.iPatternTargetEntityId == 101u,
			"A moving selected target is followed from the current yaw using the remaining duration");
		setDirection(90.f); beginFacing(-90.f, 1000u); albionOwner.strArchetypeId = "BOSS_KOUKUSAYDON_G2_BIG_SAYDON"; updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -0.0089955f), "Big Saydon keeps its existing minus-ninety-degree model forward basis during interpolation");
		for (const auto* archetype : { "BOSS_KOUKUSAYDON_G1_SAYDON", "BOSS_KOUKUSAYDON_G3_SAYDON",
			"BOSS_KOUKUSAYDON_BINGO_SAYDON", "BOSS_KOUKUSAYDON_G2_BIG_SAYDON" })
			for (const bool immediate : { false, true })
				for (const float heading : { 0.f, 90.f, 180.f, -90.f })
				{
					setDirection(heading); beginFacing(45.f, 1000u);
					albionOwner.strArchetypeId = archetype;
					if (immediate) { showtime.MechanicTriggers.front() = targets; }
					CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 5000u, showtimeLedger);
					updateTargets(immediate ? 5000u : 5029u);
					const float forward = (albionOwner.fYawDegrees + 90.f) * .017453292519943295f;
					const float targetHeading = heading * .017453292519943295f;
					const float dot = std::sin(forward) * std::sin(targetHeading) + std::cos(forward) * std::cos(targetHeading);
					tests.Require(dot > .99999f && nearYaw(albionOwner.fYawDegrees, heading - 90.f),
						immediate ? "Showtime immediate tracking aims the actual Saydon model +X front at the player" :
						"Stationary Showtime tracking aims the actual Saydon model +X front at the player");
				}
		// Movement-enabled tracking still commits the body yaw before its root/navigation step.
		beginFacing(0.f, 8582u); setDirection(90.f);
		showtime.MechanicTriggers.front().fFollowSpeedScale = 1.f;
		albionOwner.strArchetypeId = "BOSS_KOUKUSAYDON_G1_SAYDON"; albionOwner.fYawDegrees = -90.f;
		albionOwner.fPositionY = 1.f; albionOwner.fCollisionRadius = .1f;
		albionOwner.iPatternStartTick = 5000u; albionOwner.iPatternStageIndex = 0u;
		albionOwner.iPatternStageFirstEvaluationTick = 5000u;
		albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
		albionOwner.AlbionAirborne = {};
		albionOwner.PatternStageRootMotion = {{0u,0.f,0.f,0.f},{10000u,0.f,0.f,0.f}};
		auto trackingPattern = showtime; trackingPattern.Stages.resize(1u); trackingPattern.Stages[0].iDurationMs = 10000u;
		room->m_ServerNavigation.Set_RuntimeSupportSurfaces({}, status);
		room->m_ServerCollisionSystem.Initialize({}, status); room->m_ServerCollisionSystem.Set_BlockingBodies({});
		bool trackingRootValid = true;
		for (std::uint32_t tick = 5000u; tick < 5030u; ++tick)
		{
			trackingRootValid = CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, trackingPattern, tick,
				room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && trackingRootValid;
			target.fPositionX = albionOwner.fPositionX + 100.f; target.fPositionZ = albionOwner.fPositionZ;
			updateTargets(tick);
			if (tick == 5000u) tests.Require(nearYaw(albionOwner.fYawDegrees, -84.f), "Pursuit turns six degrees on its first 30 Hz tick regardless of the 8582 ms lifetime");
			if (tick == 5014u) tests.Require(nearYaw(albionOwner.fYawDegrees, 0.f), "Moving pursuit turns ninety degrees within half a second");
		}
		const float trackingYaw = albionOwner.fYawDegrees;
		tests.Require(trackingRootValid && nearYaw(trackingYaw, 0.f) && albionOwner.fPositionX > 8.f,
			"Moving Saydon tracks with a changing body yaw through thirty real root/navigation ticks");
		tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, trackingPattern, 5030u,
			room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && albionOwner.fYawDegrees == trackingYaw,
			"The next absolute root-motion sample preserves the committed tracking yaw");
		beginFacing(0.f, 20000u); setDirection(90.f); showtime.MechanicTriggers.front().fFollowSpeedScale = 1.f; updateTargets(5000u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 6.f), "A longer pursuit lifetime does not slow its first turn");
		target.fPositionX = albionOwner.fPositionX + 100.f; target.fPositionZ = albionOwner.fPositionZ; updateTargets(5014u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "Grouped pursuit ticks consume the same bounded angular time");
		beginFacing(179.f, 8582u); setDirection(-179.f); showtime.MechanicTriggers.front().fFollowSpeedScale = 1.f; updateTargets(5000u);
		tests.Require(nearYaw(albionOwner.fYawDegrees, -179.f), "Moving pursuit crosses the yaw seam along the shortest arc");
		room->m_CombatObjectRuntime.Reset(); room->m_Players = savedPlayers; albionOwner = *savedOwner;
	}


	// The optional ordered random volley is one global instance, independent of the roster.
	{
		const auto savedPlayers = room->m_Players;
		const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		const auto beginRandom = [&](const std::uint32_t count, const std::uint32_t start, const std::uint32_t duration) {
			room->m_CombatObjectRuntime.Reset(); room->m_Players.clear();
			for (PLAYER_ID id = 1u; id <= count; ++id) {
				auto& player = room->m_Players[id]; player.iPlayerId = id; player.iNetEntityId = 100u + id;
				player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true; player.fMoveSpeed = 6.f;
				player.fPositionX = 7.f + float(id); player.fPositionY = 1.f; player.fPositionZ = 8.f;
			}
			albionOwner.strPatternId = showtime.strPatternId; albionOwner.iPatternSequence = 71u;
			albionOwner.fPositionX = albionOwner.fSpawnPositionX = 8.f;
			albionOwner.fPositionY = albionOwner.fSpawnPositionY = 1.f;
			albionOwner.fPositionZ = albionOwner.fSpawnPositionZ = 8.f;
			showtime.MechanicTriggers = {targets}; auto& definition = showtime.MechanicTriggers.front();
			definition.iDurationMs = duration;
			definition.RandomVolleys = { { {}, "test.random.a", 5000u}, { {}, "test.random.a", 5000u},
				{ {}, "test.random.b", 5000u}, { {}, "test.random.b", 5000u}, { {}, "test.random.c", 5000u}, { {}, "test.random.c", 5000u} };
			definition.iRandomSpawnIntervalMs = 500u; definition.fRandomArenaRadiusM = 2.f; definition.fRandomArenaHeightToleranceM = .2f;
			CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, start, showtimeLedger);
		};
		const auto randomObjects = [&] {
			std::vector<SERVER_COMBAT_OBJECT> result;
			for (const auto& object : room->m_CombatObjectRuntime.Get_LiveObjects())
				if (object.strClientVisualId.starts_with("test.random.")) result.push_back(object);
			return result;
		};
		for (std::uint32_t count = 1u; count <= 4u; ++count) {
			beginRandom(count, 8000u, 3000u); updateTargets(8000u);
			tests.Require(randomObjects().size() == 1u && countTargetObjects(true) == count &&
				countTargetObjects(false) == count + 1u, "Random volley creates one global object while preserving every player's fixed and tracking instances");
			updateTargets(8014u); updateTargets(8015u); updateTargets(8015u);
			tests.Require(randomObjects().size() == 2u && showtimeLedger.PlayerTargetWindows.front().iRandomWaveOrdinal == 2u,
				"Random cadence is fifteen fixed ticks and repeating one tick never creates a second instance");
			for (auto tick : {8030u, 8045u, 8060u, 8075u}) updateTargets(tick);
			auto objects = randomObjects();
			bool positionsValid = objects.size() == 6u, varies = false;
			for (std::size_t i = 0u; i < objects.size(); ++i) {
				const auto& pose = objects[i].LiveState.CurrentPose;
				positionsValid = positionsValid && std::hypot(pose.fPositionX - 8.f, pose.fPositionZ - 8.f) <= 2.0001f &&
					room->m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) && pose.fPositionY == 1.f &&
					pose.fYawDegrees == 0.f && objects[i].iLockedTargetNetEntityId == INVALID_NET_ENTITY_ID && objects[i].Hits.empty() &&
					objects[i].strClientVisualId == (i < 2u ? "test.random.a" : i < 4u ? "test.random.b" : "test.random.c");
				if (i != 0u) varies = varies || pose.fPositionX != objects.front().LiveState.CurrentPose.fPositionX || pose.fPositionZ != objects.front().LiveState.CurrentPose.fPositionZ;
			}
			tests.Require(positionsValid && varies, "Ordered duplicate templates cycle A1 B1 A2 B2 A3 B3 at changing deterministic walkable map positions");
			updateTargets(8090u);
			tests.Require(randomObjects().size() == 6u && countTargetObjects(true) == 0u && countTargetObjects(false) == 6u + count * 2u,
				"Duration end stops new random volleys and tracking but preserves already emitted random and fixed tails");
		}
		beginRandom(1u, 9000u, 5000u); room->m_Players[1u].isCombatReady = false; updateTargets(9000u);
		tests.Require(randomObjects().empty() && showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9015u &&
			showtimeLedger.PlayerTargetWindows.front().iRandomWaveOrdinal == 0u,
			"No alive player consumes schedule time without spawning or consuming the next authored volley");
		room->m_Players[1u].isCombatReady = true; updateTargets(9010u);
		tests.Require(randomObjects().empty(), "Returning before the next deadline does not replay the missed empty-room interval");
		updateTargets(9015u); updateTargets(9080u);
		tests.Require(randomObjects().size() == 2u && showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9090u,
			"A skipped update emits one current volley and advances past missed deadlines without a burst");
		const auto beforeFailureId = room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId;
		albionOwner.fSpawnPositionY = 1000.f; updateTargets(9090u);
		tests.Require(randomObjects().size() == 2u && showtimeLedger.PlayerTargetWindows.front().iRandomWaveOrdinal == 2u &&
			showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9090u &&
			room->m_CombatObjectRuntime.Begin_Transaction().iNextCombatObjectId == beforeFailureId,
			"Invalid arena height preserves random ordinal, deadline, existing objects and next object identity");
		albionOwner.fSpawnPositionY = 1.f; updateTargets(9091u);
		tests.Require(randomObjects().size() == 3u && randomObjects().back().strClientVisualId == "test.random.b" &&
			showtimeLedger.PlayerTargetWindows.front().iNextRandomTick == 9105u,
			"Restoring navigable ground retries the unconsumed ordered volley once on the next tick");
        beginRandom(1u, 9500u, 2000u);
        auto& rain = showtime.MechanicTriggers.front();
        rain.strFixedVisualId.clear(); rain.strTrackingVisualId.clear(); rain.iFixedLifetimeMs = 0u;
        rain.strRandomAnchorKind = "BOSS"; rain.fRandomScaleMin = 1.f; rain.fRandomScaleMax = 2.f;
        rain.fRandomArenaRadiusM = .5f;
        ATTACK_HIT_TEMPLATE cardHit; cardHit.strHitId = "cardrain.impact"; cardHit.iAtMs = 1650u;
        for (auto& volley : rain.RandomVolleys) volley.Hits = {cardHit};
        albionOwner.fPositionX = 12.f; albionOwner.fYawDegrees = 37.f;
        const auto cardRainTargetBefore = albionOwner.iPatternTargetEntityId;
        CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 9500u, showtimeLedger);
        updateTargets(9500u); updateTargets(9515u);
        auto rainObjects = randomObjects();
        bool rainValid = rainObjects.size() == 2u && countTargetObjects(true) == 0u && albionOwner.fYawDegrees == 37.f &&
            albionOwner.iPatternTargetEntityId == cardRainTargetBefore;
        for (const auto& object : rainObjects)
        {
            const auto& pose = object.LiveState.CurrentPose;
            rainValid = rainValid && std::hypot(pose.fPositionX - 12.f, pose.fPositionZ - 8.f) <= .5001f &&
                room->m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) &&
                object.fUniformScale >= 1.f && object.fUniformScale <= 2.f && object.Hits.size() == 1u;
        }
        tests.Require(rainValid, "Card rain owns one nav-projected random object per interval around current Saydon with scale 1..2 and no target-facing mutation");
        tests.Require(rainObjects.size() == 2u && rainObjects[0].fUniformScale != rainObjects[1].fUniformScale,
            "Card rain scale changes deterministically between independently spawned groups");
		beginRandom(2u, 10000u, 2000u);
		auto secondWindow = showtime.MechanicTriggers.front(); secondWindow.strTriggerId += ".second"; secondWindow.iStartMs = 250u; secondWindow.iDurationMs = 1500u;
		showtime.MechanicTriggers.push_back(secondWindow); CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 10000u, showtimeLedger);
		updateTargets(10000u); updateTargets(10008u); updateTargets(10015u); updateTargets(10023u);
		tests.Require(randomObjects().size() == 4u && showtimeLedger.PlayerTargetWindows[0].iRandomWaveOrdinal == 2u &&
			showtimeLedger.PlayerTargetWindows[1].iRandomWaveOrdinal == 2u && countTargetObjects(true) == 4u,
			"Concurrent duration windows keep separate random cadences and their own player trackers");
		room->m_CombatObjectRuntime.Reset(); room->m_Players = savedPlayers; albionOwner = *savedOwner;
	}

	// Typed Albion phases use the actual room transaction and existing root/navigation sweep.
	{
		using Phase = ALBION_AIRBORNE_PHASE;
		const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		const auto savedPlayers = room->m_Players;
		room->m_ServerNavigation.Set_RuntimeSupportSurfaces({}, status);
		room->m_ServerCollisionSystem.Initialize({}, status); room->m_ServerCollisionSystem.Set_BlockingBodies({});
		room->m_Players.clear();
		SERVER_PLAYER target; target.iPlayerId=1u; target.iNetEntityId=401u; target.iCurrentHp=100u; target.isCombatReady=true;
		target.fPositionX=10.f; target.fPositionY=1.f; target.fPositionZ=10.f; room->m_Players[1u]=target;
		BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId="KOUKUSAYDON_AIRBORNE_CONTRACT"; pattern.bFixedTimelineClock=true;
		pattern.Stages.emplace_back(); pattern.Stages.front().iDurationMs=8000u;
		albionOwner.strPatternId=pattern.strPatternId; albionOwner.iPatternSequence=73u; albionOwner.iPatternStageIndex=0u;
		albionOwner.iPatternStartTick=12000u; albionOwner.iActionStartTick=12000u; albionOwner.iPatternStageFirstEvaluationTick=12000u;
		albionOwner.strActionId="keep.source.clip"; albionOwner.fPositionX=6.f; albionOwner.fPositionY=1.f; albionOwner.fPositionZ=6.f;
		albionOwner.fYawDegrees=0.f; albionOwner.fCollisionRadius=.1f; albionOwner.AlbionAirborne={};
		albionOwner.PatternStageRootMotion.clear(); albionOwner.bPatternStageRootOriginCaptured=false; albionOwner.iPatternStageRootLastTick=0u;
		BOSS_PATTERN_MECHANIC_TRIGGER phase; phase.strTriggerId="air.jump"; phase.eKind=BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE;
		phase.eAirbornePhase=Phase::JUMP; phase.fAirborneHeightM=13.6788133f; phase.iAirborneDurationMs=200u;
		const auto rootAt=[&](std::uint32_t tick) { return CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner,pattern,tick,room->m_ServerNavigation,room->m_ServerCollisionSystem,status); };
		const auto beforeInstant = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		phase.iAirborneDurationMs=0u; phase.fAirborneHeightM=11.1910783f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12000u) &&
			std::abs(albionOwner.fPositionY-12.1910783f)<.00001f,
			"Zero-duration JUMP initializes the requested high pose on its first server tick");
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{1000u,0.f,.5f,-5.59553915f},{2000u,0.f,1.f,-11.1910783f},{8000u,0.f,1.f,-10.7271685f}};
		phase.eAirbornePhase=Phase::SLAM; phase.fAirborneHeightM=0.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12000u) &&
			std::abs(albionOwner.fPositionY-12.1910783f)<.00001f && rootAt(12030u) &&
			std::abs(albionOwner.fPositionY-6.59553915f)<.00001f && rootAt(12060u) && albionOwner.fPositionY==1.f &&
			albionOwner.fPositionX==6.f && albionOwner.fPositionZ==6.f,
			"Same-tick high initialization and native descent preserve full height, midpoint and grounded endpoint");
		{
			auto clone = std::make_unique<SERVER_WORLD_ENTITY>(*beforeInstant);
			clone->bKoukuSummonClone = true; clone->iNetEntityId = 790u;
			clone->PatternStageRootMotion = {{0u,0.f,0.f,0.f},{1000u,0.f,0.f,-5.59553915f},{2000u,0.f,0.f,-11.1910783f}};
			auto clonePattern = pattern;
			auto jump = phase; jump.strTriggerId = "clone.jump"; jump.eAirbornePhase = Phase::JUMP; jump.fAirborneHeightM = 11.1910783f;
			auto slam = phase; slam.strTriggerId = "clone.slam";
			clonePattern.MechanicTriggers = {slam, jump}; // Deliberately reverse same-clock authored order.
			tests.Require(room->Update_KoukuSummonTriggers(*clone, clonePattern, 12000u) &&
				clone->KoukuSummonStartedTriggers.size() == 2u && std::abs(clone->fPositionY - 12.1910783f) < .00001f,
				"A summoned actor commits initial JUMP before SLAM even when equal-clock rows are reversed");
			tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(*clone, clonePattern, 12030u,
				room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
				room->Update_KoukuSummonTriggers(*clone, clonePattern, 12030u) &&
				std::abs(clone->fPositionY - 6.59553915f) < .00001f && clone->KoukuSummonStartedTriggers.size() == 2u,
				"Repeated clone ticks do not reapply initial height and preserve native midpoint descent");
			tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(*clone, clonePattern, 12060u,
				room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && clone->fPositionY == 1.f,
				"The summoned actor lands on the same authoritative navigation floor");
		}
		pattern.Stages.emplace_back(); pattern.Stages.back().iDurationMs=1000u;
		albionOwner.iPatternStageIndex=1u; albionOwner.iPatternStageFirstEvaluationTick=12240u;
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{1000u,0.f,2.f,4.f}};
		albionOwner.bPatternStageRootOriginCaptured=false; albionOwner.iPatternStageRootLastTick=0u;
		tests.Require(rootAt(12255u) && std::abs(albionOwner.fPositionY-3.f)<.00001f && std::abs(albionOwner.fPositionX-7.f)<.00001f,
			"A later stage resumes its own native root rise and lateral movement after the landing owner ends");
		pattern.Stages.resize(1u);
		albionOwner=*beforeInstant; phase.eAirbornePhase=Phase::JUMP; phase.fAirborneHeightM=13.6788133f; phase.iAirborneDurationMs=200u;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12000u) && albionOwner.fPositionY==1.f,
			"Albion jump begins at the current ground without an instant vertical snap even on an empty source curve");
		tests.Require(rootAt(12003u) && std::abs(albionOwner.fPositionY-1.f-13.6788133f*.5f)<.00001f,
			"Actual fixed-tick navigation/root consumer reaches half original jump height in 100ms");
		tests.Require(rootAt(12006u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f && rootAt(12015u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"The 200ms ascent reaches the unchanged original height and holds after the shortened ramp");
		phase.eAirbornePhase=Phase::SELECT_PLAYER; phase.fAirborneHeightM=0.f; phase.iAirborneDurationMs=0u;
		const auto beforeMarioSelection = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		for (std::uint8_t marioStage = 1u; marioStage <= 4u; ++marioStage)
		{
			room->m_Players[1u].iMarioStage = marioStage;
			room->m_Players[1u].fPositionX = 10000.f;
			tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12016u) &&
				albionOwner.AlbionAirborne.iSelectedPlayer==INVALID_NET_ENTITY_ID && albionOwner.AlbionAirborne.bHasSelectedGround &&
				albionOwner.AlbionAirborne.SelectedGround.x==6.f && albionOwner.AlbionAirborne.SelectedGround.y==1.f &&
				albionOwner.AlbionAirborne.SelectedGround.z==6.f && albionOwner.AlbionAirborne.ePhase==Phase::JUMP,
				"Albion excludes Mario stages 1 through 4 and captures boss ground without changing the jump phase");
		}
		room->m_Players[1u].iMarioStage = 0u; room->m_Players[1u].fPositionX = 10.f;
		albionOwner = *beforeMarioSelection;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12016u) && albionOwner.AlbionAirborne.iSelectedPlayer==401u && albionOwner.AlbionAirborne.ePhase==Phase::JUMP,
			"SELECT_PLAYER pins an alive server player without changing the active jump phase");
		target.iPlayerId=2u; target.iNetEntityId=402u; target.fPositionX=12.f; target.fPositionZ=12.f; room->m_Players[2u]=target;
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{2000u,0.f,0.f,0.f},{3000u,0.f,0.f,-3.2783114f}};
		phase.eAirbornePhase=Phase::APPEAR_PLAYER; phase.fAirborneHeightM=3.2783114f; phase.iStartMs=600u;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12018u) && albionOwner.fPositionX==10.f && albionOwner.fPositionZ==10.f && std::abs(albionOwner.fPositionY-4.2783114f)<.00001f,
			"Appearance reuses the pinned player XZ and the original landing clip height");
		tests.Require(rootAt(12019u) && albionOwner.fPositionX==10.f && albionOwner.fPositionZ==10.f,
			"The next source root sample retains the teleported XZ instead of returning to the old stage origin");
		room->m_Players[1u].iMarioStage=4u; room->m_Players[1u].fPositionX=10000.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12020u) && albionOwner.AlbionAirborne.iSelectedPlayer==402u && albionOwner.fPositionX==12.f,
			"A pinned target entering Mario is replaced by the remaining arena player instead of moving the boss into Mario");
		room->m_Players[1u].iMarioStage=0u; room->m_Players[1u].iCurrentHp=0u; room->m_Players[1u].fPositionX=10.f;
		phase.eAirbornePhase=Phase::DISAPPEAR; phase.fAirborneHeightM=0.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12021u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"Disappear restores the initial jump height rather than accumulating the landing offset");
		room->m_Players[2u].iCurrentHp=0u; phase.eAirbornePhase=Phase::APPEAR_PLAYER; phase.fAirborneHeightM=3.2783114f;
		const auto beforeNoPlayer=std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12022u) &&
			std::abs(albionOwner.fPositionY-4.2783114f)<.00001f &&
			albionOwner.fPositionX==beforeNoPlayer->fPositionX && albionOwner.fPositionZ==beforeNoPlayer->fPositionZ &&
			albionOwner.AlbionAirborne.ePhase==Phase::APPEAR_PLAYER && albionOwner.AlbionAirborne.bHasSelectedGround &&
			albionOwner.AlbionAirborne.iSelectedPlayer==INVALID_NET_ENTITY_ID && albionOwner.iPatternTargetEntityId==INVALID_NET_ENTITY_ID,
			"No selectable player continues the landing phase at a captured boss-ground point and clears stale player identity");
		phase.eAirbornePhase=Phase::CENTER; phase.fAirborneHeightM=0.f; phase.fTeleportX=6.f; phase.fTeleportY=1.f; phase.fTeleportZ=6.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12023u) && albionOwner.fPositionX==6.f && albionOwner.fPositionZ==6.f &&
			std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"CENTER consumes an absolute navigation-checked XZ at the original jump height");
		phase.fTeleportX=6.f; phase.fTeleportZ=2.f;
		tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12024u) && albionOwner.fPositionZ==6.f && albionOwner.AlbionAirborne.ePhase==Phase::CENTER,
			"Invalid center navigation preserves the current position and airborne phase");
		albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{1000u,0.f,0.f,0.f},{2000u,0.f,0.f,-3.5f},{2501u,0.f,0.f,-7.f},{3000u,0.f,0.f,-6.9f},{8000u,0.f,0.f,-6.9f}};
		phase.eAirbornePhase=Phase::SLAM; phase.iStartMs=1000u; phase.fTeleportX=phase.fTeleportY=phase.fTeleportZ=0.f;
		tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12030u) && std::abs(albionOwner.fPositionY-14.6788133f)<.00001f,
			"SLAM starts from the held height and commits through the existing navigation/body sweep");
		tests.Require(rootAt(12060u) && std::abs(albionOwner.fPositionY-1.f-13.6788133f*.5f)<.00001f,
			"SLAM follows the original remaining source displacement normalized from trigger to source minimum");
		tests.Require(rootAt(12090u) && albionOwner.fPositionY==1.f && rootAt(12230u) && albionOwner.fPositionY==1.f,
			"Skipped minimum samples and an extended Stage tail remain exactly on navigation ground without rebound");
		tests.Require(albionOwner.iPatternStartTick==12000u && albionOwner.iActionStartTick==12000u && albionOwner.iPatternStageIndex==0u &&
			albionOwner.strActionId=="keep.source.clip" && albionOwner.fPositionX==6.f && albionOwner.fPositionZ==6.f,
			"All phases preserve animation, Stage, Pattern clocks and the central slam XZ");
		const auto beforeFlat=std::make_unique<SERVER_WORLD_ENTITY>(albionOwner); albionOwner.PatternStageRootMotion={{0u,0.f,0.f,0.f},{8000u,0.f,0.f,0.f}};
		tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) && albionOwner.fPositionY==beforeFlat->fPositionY &&
			albionOwner.AlbionAirborne.fSourceUpMinimum==beforeFlat->AlbionAirborne.fSourceUpMinimum,
			"A source curve without remaining descent rejects the phase without replacing the prior state");
        // SELECT policy captures navigation ground once and shares the reliable fixed object pivot.
        const auto savedAuditionPhase=room->m_KoukuSaydonPatternAudition.ePhase;
        const auto savedAuditionRevision=room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
        const auto savedAuditionSource=room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
        room->m_KoukuSaydonPatternAudition.ePhase=CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
        room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision=room->m_GameplayCatalog.Get_ActiveRevision();
        room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision=CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog);
        room->m_CombatObjectRuntime.Reset();
        room->m_Players.clear(); target.iPlayerId=1u; target.iNetEntityId=401u;
        target.iCurrentHp=100u; target.fPositionX=10.f; target.fPositionY=99.f; target.fPositionZ=10.f;
        room->m_Players[1u]=target;
        phase.eAirbornePhase=Phase::SELECT_PLAYER; phase.iStartMs=0u; phase.fAirborneHeightM=0.f;
        phase.bCaptureAirborneTargetPosition=true; phase.strSelectedEffectVisualId="test.selected.group"; phase.iSelectedEffectLifetimeMs=2000u;
        tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) &&
            albionOwner.AlbionAirborne.bHasSelectedGround && albionOwner.AlbionAirborne.SelectedGround.x==10.f &&
            albionOwner.AlbionAirborne.SelectedGround.y==1.f && albionOwner.AlbionAirborne.SelectedGround.z==10.f,
            "SELECT policy pins the actual navigation ground rather than the player's transient visual height");
        const auto& selectedObjects=room->m_CombatObjectRuntime.Get_LiveObjects();
        tests.Require(selectedObjects.size()==1u && selectedObjects.front().strClientVisualId=="test.selected.group" &&
            selectedObjects.front().LiveState.CurrentPose.fPositionX==10.f && selectedObjects.front().LiveState.CurrentPose.fPositionY==1.f &&
            selectedObjects.front().LiveState.CurrentPose.fPositionZ==10.f && selectedObjects.front().LiveState.CurrentPose.fYawDegrees==0.f &&
            selectedObjects.front().Hits.empty(), "The single reliable fixed Effect object shares the captured XYZ and has no damage or boss yaw");
        const auto beforeSelectedFailure=albionOwner.AlbionAirborne;
        phase.iSelectedEffectLifetimeMs=0u;
        tests.Require(!room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) &&
            room->m_CombatObjectRuntime.Get_LiveObjects().size()==1u && albionOwner.AlbionAirborne.SelectedGround.x==beforeSelectedFailure.SelectedGround.x,
            "A rejected fixed-object transaction preserves the previous target anchor and live Effect");
        phase.iSelectedEffectLifetimeMs = 2000u;
        LostArk::Shared::ATTACK_HIT_TEMPLATE selectedHit;
        selectedHit.strHitId = "selected.contact"; selectedHit.strTrigger = "CONTACT";
        selectedHit.iAtMs = 1000u; selectedHit.iEndMs = 1100u;
        selectedHit.fRadiusM = 3.0; selectedHit.fRiseHeightM = 2.0; selectedHit.fPushRangeM = 1.0; selectedHit.iPushMs = 1200u;
        phase.FixedHits = {selectedHit};
        tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner, pattern, phase, 12231u) &&
            room->m_CombatObjectRuntime.Get_LiveObjects().back().Hits.size() == 1u &&
            room->m_CombatObjectRuntime.Get_LiveObjects().back().Hits.front().fPushRangeM == 1.f &&
            room->m_CombatObjectRuntime.Get_LiveObjects().back().Hits.front().fRiseHeightM == 2.f &&
            room->m_CombatObjectRuntime.Get_LiveObjects().back().LiveState.CurrentPose.fPositionX == 10.f,
            "Selected ground Effect and its damage/ballistic push share one staged authoritative object");
        phase.FixedHits.clear();
        phase.strSelectedEffectVisualId.clear(); phase.bCaptureAirborneTargetPosition=false;
        phase.eAirbornePhase=Phase::APPEAR_PLAYER; phase.fAirborneHeightM=3.2783114f;
        room->m_Players[1u].fPositionX=12.f; room->m_Players[1u].fPositionZ=12.f; room->m_Players[1u].iCurrentHp=0u;
        tests.Require(room->Commit_KoukuAlbionAirborne(albionOwner,pattern,phase,12231u) &&
            albionOwner.AlbionAirborne.iSelectedPlayer==401u && albionOwner.fPositionX==10.f && albionOwner.fPositionZ==10.f &&
            std::abs(albionOwner.fPositionY-4.2783114f)<.00001f,
            "SELECT policy appearance reuses its exact ground point after player movement and death without replacement");
        room->m_CombatObjectRuntime.Cancel_Source(albionOwner.iNetEntityId);
        room->m_KoukuSaydonPatternAudition.ePhase=savedAuditionPhase;
        room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision=savedAuditionRevision;
        room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision=savedAuditionSource;
        tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Existing Stop/source cleanup removes the captured group object");
        room->m_KoukuSaydonBrain.Abort_Pattern(albionOwner,12232u);
		tests.Require(albionOwner.AlbionAirborne.ePhase==Phase::NONE && albionOwner.AlbionAirborne.iSelectedPlayer==INVALID_NET_ENTITY_ID,
			"Pattern abort clears the airborne phase and pinned player without a second movement authority");
		albionOwner=*savedOwner; room->m_Players=savedPlayers;
	}

	// Teleport rebases only XZ; the next actual animation root sample retains height and clocks.
	{
		room->m_ServerCollisionSystem.Initialize({}, status);
		room->m_ServerCollisionSystem.Set_BlockingBodies({});
		room->m_ServerNavigation.Set_RuntimeSupportSurfaces({ { "teleport.ground", 10.f, 10.f, 2.f, 1.6f } }, status);
		BOSS_PATTERN_DEFINITION teleportPattern; teleportPattern.strPatternId = "KOUKUSAYDON_TEST_TELEPORT_XZ";
		teleportPattern.Stages.emplace_back(); teleportPattern.Stages.front().iDurationMs = 1000u;
		teleportPattern.Stages.front().Motion.RootMotion = { {0u, 0.f, 0.f, 0.f}, {1000u, 0.f, 3.f, 2.f} };
		albionOwner.strPatternId = teleportPattern.strPatternId; albionOwner.iPatternSequence = 22u;
		albionOwner.fPositionX = 6.f; albionOwner.fPositionY = 4.f; albionOwner.fPositionZ = 6.f;
		albionOwner.fYawDegrees = 0.f; albionOwner.fCollisionRadius = .1f;
		albionOwner.iPatternStageIndex = 0u; albionOwner.iPatternStageFirstEvaluationTick = 3000u;
		albionOwner.iPatternStartTick = 3000u; albionOwner.iActionStartTick = 3000u;
		albionOwner.strActionId = "teleport.keep.animation";
		albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
		albionOwner.PatternStageRootMotion = teleportPattern.Stages.front().Motion.RootMotion;
		tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 3003u,
			room->m_ServerNavigation, room->m_ServerCollisionSystem, status), "Capture a running airborne root curve before XZ teleport");
		const auto beforeTeleport = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		BOSS_PATTERN_MECHANIC_TRIGGER teleport;
		teleport.strTriggerId = "test.teleport.xz"; teleport.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ;
		teleport.iDurationMs = 100u; teleport.fTeleportX = 10.f; teleport.fTeleportY = 1.3f; teleport.fTeleportZ = 10.f;
		const auto queueTeleport = [&](const std::uint32_t tick = 3003u) {
			room->m_PendingKoukuMechanicTriggers.push_back({ albionOwner.iNetEntityId, albionOwner.iPatternSequence, teleport });
			room->Commit_KoukuMechanicTriggers(tick);
		};
		queueTeleport();
		tests.Require(albionOwner.fPositionX == 10.f && albionOwner.fPositionZ == 10.f &&
			albionOwner.fPositionY == beforeTeleport->fPositionY && albionOwner.fPatternStageOriginY == beforeTeleport->fPatternStageOriginY &&
			albionOwner.iPatternStageRootLastTick == 3003u && albionOwner.iPatternStartTick == 3000u &&
			albionOwner.iActionStartTick == 3000u && albionOwner.strActionId == beforeTeleport->strActionId &&
			albionOwner.iPatternSequence == 22u && albionOwner.fYawDegrees == beforeTeleport->fYawDegrees,
			"XZ teleport ignores reference Y and preserves airborne height, action, stage and pattern clocks without clones");
		tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 3004u,
			room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
			std::abs(albionOwner.fPositionX - 10.1f) < .0001f && albionOwner.fPositionZ == 10.f &&
			std::abs(albionOwner.fPositionY - beforeTeleport->fPositionY - 2.f / 30.f) < .0001f,
			"The next root frame continues at the new XZ and preserves root-up across different ground heights");
		const auto stablePose = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		room->m_ServerCollisionSystem.Set_BlockingBodies({ {12.f, 8.f, 1.f, albionOwner.fPositionY, 2.f, 999u} });
		teleport.fTeleportX = 12.f; teleport.fTeleportZ = 8.f; queueTeleport();
		const auto preserved = [&] { return albionOwner.fPositionX == stablePose->fPositionX &&
			albionOwner.fPositionY == stablePose->fPositionY && albionOwner.fPositionZ == stablePose->fPositionZ &&
			albionOwner.fPatternStageOriginX == stablePose->fPatternStageOriginX &&
			albionOwner.fPatternStageOriginZ == stablePose->fPatternStageOriginZ &&
			albionOwner.fPatternStageRootGroundY == stablePose->fPatternStageRootGroundY; };
		tests.Require(preserved(), "Occupied destination rejects XZ teleport without partially rebasing the root origin");
		teleport.fTeleportX = 6.f; teleport.fTeleportZ = 2.f; queueTeleport();
		tests.Require(preserved(), "Missing navigation rejects XZ teleport and preserves its full previous pose");
        // The explicit grounded variant lands the source root baseline as one transaction.
        room->m_ServerCollisionSystem.Set_BlockingBodies({});
        albionOwner = *beforeTeleport;
        teleport.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED;
        teleport.fTeleportX = teleport.fTeleportZ = 10.f; teleport.fTeleportY = 1.3f;
        queueTeleport();
        tests.Require(albionOwner.bPatternRootGrounded && albionOwner.fPositionY == 1.6f && albionOwner.fPositionX == 10.f && albionOwner.fPositionZ == 10.f &&
            std::abs(albionOwner.fPatternStageOriginY - (beforeTeleport->fPatternStageOriginY + 1.6f - beforeTeleport->fPositionY)) < .0001f &&
            albionOwner.fYawDegrees == beforeTeleport->fYawDegrees && albionOwner.iPatternStageRootLastTick == 3003u &&
            albionOwner.iActionStartTick == beforeTeleport->iActionStartTick && albionOwner.iPatternSequence == beforeTeleport->iPatternSequence,
            "Grounded teleport samples the actual floor and rebases vertical root without changing yaw or action clock");
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 3004u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
            std::abs(albionOwner.fPositionX - 10.1f) < .0001f && std::abs(albionOwner.fPositionY - (1.6f + 2.f / 30.f)) < .0001f,
            "The next grounded root tick continues only its authored delta and cannot restore the former airborne offset");
        const auto groundedPose = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
        const auto groundedPreserved = [&] { return albionOwner.fPositionX == groundedPose->fPositionX &&
            albionOwner.fPositionY == groundedPose->fPositionY && albionOwner.fPositionZ == groundedPose->fPositionZ &&
            albionOwner.fPatternStageOriginX == groundedPose->fPatternStageOriginX &&
            albionOwner.fPatternStageOriginY == groundedPose->fPatternStageOriginY &&
            albionOwner.fPatternStageOriginZ == groundedPose->fPatternStageOriginZ &&
            albionOwner.fPatternStageRootGroundY == groundedPose->fPatternStageRootGroundY &&
            albionOwner.iPatternStageRootLastTick == groundedPose->iPatternStageRootLastTick &&
            albionOwner.bPatternRootGrounded == groundedPose->bPatternRootGrounded; };
        teleport.fTeleportY = 4.f; queueTeleport();
        tests.Require(groundedPreserved(), "Grounded destination deck mismatch preserves position and every root baseline");
        teleport.fTeleportY = 1.3f; teleport.fTeleportX = 6.f; teleport.fTeleportZ = 2.f; queueTeleport();
        tests.Require(groundedPreserved(), "Missing grounded destination navigation preserves position and every root baseline");
        teleport.fTeleportX = teleport.fTeleportZ = 10.f;
        room->m_ServerCollisionSystem.Set_BlockingBodies({ {10.f, 10.f, 1.f, 1.6f, 2.f, 999u} });
        queueTeleport();
        tests.Require(groundedPreserved(), "Ground-level blocking body rejects the complete teleport before any root update");
        room->m_ServerCollisionSystem.Set_BlockingBodies({});
        albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
        albionOwner.iPatternStageFirstEvaluationTick = albionOwner.iActionStartTick = albionOwner.iPatternStartTick = 4000u;
        albionOwner.fPositionY = 8.f;
        queueTeleport(4000u);
        // This stage clock evaluates its first 1/30-second step on the entry tick.
        constexpr float firstRootUp = 2.f / 30.f;
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 4000u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
            std::abs(albionOwner.fPositionY - (1.6f + firstRootUp)) < .0001f &&
            std::abs(albionOwner.fPatternStageOriginY - 1.6f) < .0001f,
            "A first root sample after grounded teleport captures the landed floor instead of the old airborne height");
        albionOwner.bPatternStageRootOriginCaptured = false; albionOwner.iPatternStageRootLastTick = 0u;
        albionOwner.fPositionY = 1.85f;
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 4000u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
            std::abs(albionOwner.fPositionY - (1.6f + firstRootUp)) < .0001f &&
            std::abs(albionOwner.fPatternStageOriginY - 1.6f) < .0001f,
            "A later grounded stage captures the floor without inheriting the previous clip residual Up");
        albionOwner.PatternStageRootMotion.back().fUp = -2.f;
        tests.Require(CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, teleportPattern, 4001u,
            room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && albionOwner.fPositionY >= 1.6f,
            "Grounded root continuation cannot send the boss below the resolved floor");
        room->m_KoukuSaydonBrain.Abort_Pattern(albionOwner, 4002u);
        tests.Require(!albionOwner.bPatternRootGrounded, "Explicit Stop/abort clears the pattern-local grounded continuation policy");

	}

	// Consume the published Showtime tail through the real stage clock, floor and teleport owner.
	{
		const auto savedBoss = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
		const auto savedNavigation = room->m_ServerNavigation;
		CServerNavigation publishedNavigation;
		const auto* showtime = CKoukuSaydonBrain::Find_AnimationOnlyPattern(room->m_GameplayCatalog,
			"KOUKUSAYDON_G1_PATTERN_35", status);
		const auto center = showtime ? std::find_if(showtime->MechanicTriggers.begin(), showtime->MechanicTriggers.end(),
			[](const auto& trigger) { return trigger.strTriggerId == "KOUKUSAYDON_G1_PATTERN_35.logic.17"; }) :
			std::vector<BOSS_PATTERN_MECHANIC_TRIGGER>::const_iterator{};
		const bool ready = showtime && center != showtime->MechanicTriggers.end() &&
			center->eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED &&
			publishedNavigation.Load("LV_LUT_MIDNIGHTC_ED");
		tests.Require(ready, "Published Showtime final return uses its grounded occurrence and actual G3 navigation");
		if (ready)
		{
			room->m_ServerNavigation = std::move(publishedNavigation);
			room->m_ServerCollisionSystem.Set_BlockingBodies({});
			albionOwner.fPositionX = center->fTeleportX; albionOwner.fPositionZ = center->fTeleportZ;
			albionOwner.fPositionY = 8.f; albionOwner.fCollisionRadius = .1f;
			SERVER_NAV_POINT floor;
			const bool began = room->m_ServerNavigation.Sample_Position(center->fTeleportX, center->fTeleportZ, floor) &&
				room->m_KoukuSaydonBrain.Begin_Pattern(albionOwner, *showtime,
					room->m_GameplayCatalog.Get_ActiveRevision(), 10000u, status);
			tests.Require(began && !albionOwner.bPatternRootGrounded, "New pattern admission clears any former grounded root policy");
			if (began)
			{
				for (std::uint32_t tick = 10001u; tick <= 11676u; ++tick)
					(void)room->m_KoukuSaydonBrain.Update(albionOwner, room->m_GameplayCatalog, tick, status);
				tests.Require(albionOwner.strPatternStageId == "STAGE_11" &&
					CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, *showtime, 11677u,
						room->m_ServerNavigation, room->m_ServerCollisionSystem, status),
					"Showtime 55.869s return samples the actual final Stage 11 curve at its first fixed tick");
				room->m_PendingKoukuMechanicTriggers.push_back({ albionOwner.iNetEntityId, albionOwner.iPatternSequence, *center });
				room->Commit_KoukuMechanicTriggers(11677u);
				tests.Require(albionOwner.bPatternRootGrounded && std::abs(albionOwner.fPositionY - floor.y) < .00001f,
					"The actual published grounded trigger atomically lands on the G3 center floor");
				float minimumUp = 10000.f, maximumUp = -10000.f;
				bool sampled = true;
				for (std::uint32_t tick = 11678u; tick <= 11770u && !albionOwner.strPatternId.empty(); ++tick)
				{
					sampled = CKoukuSaydonBrain::Apply_StageRootMotion(albionOwner, *showtime, tick,
						room->m_ServerNavigation, room->m_ServerCollisionSystem, status) && sampled;
					SERVER_NAV_POINT ground;
					if (room->m_ServerNavigation.Sample_Position(albionOwner.fPositionX, albionOwner.fPositionZ, ground))
					{
						minimumUp = (std::min)(minimumUp, albionOwner.fPositionY - ground.y);
						maximumUp = (std::max)(maximumUp, albionOwner.fPositionY - ground.y);
					}
					else sampled = false;
					(void)room->m_KoukuSaydonBrain.Update(albionOwner, room->m_GameplayCatalog, tick, status);
				}
				SERVER_NAV_POINT endFloor;
				tests.Require(sampled && minimumUp >= -.00001f && maximumUp > 2.15f && maximumUp < 2.23f,
					"The complete published Showtime tail stays above the floor and retains the original 2.19m Stage 12 jump");
				tests.Require(room->m_ServerNavigation.Sample_Position(albionOwner.fPositionX, albionOwner.fPositionZ, endFloor) &&
					std::abs(albionOwner.fPositionY - endFloor.y) < .00001f && albionOwner.strPatternId.empty() && !albionOwner.bPatternRootGrounded,
					"Showtime completion leaves neither residual hover nor a grounded policy on the next pattern");
			}
		}
		albionOwner = *savedBoss;
		room->m_ServerNavigation = savedNavigation;
	}

	// Parse the actual supplemental row through catalog admission, preserving its previous generation on failure.
	std::vector<wchar_t> dataRootBuffer(32768u);
	const auto dataRootLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", dataRootBuffer.data(), static_cast<DWORD>(dataRootBuffer.size()));
	if (dataRootLength && dataRootLength < dataRootBuffer.size())
	{
		std::ifstream baselineInput(fs::path(dataRootBuffer.data()) / L"Gameplay/Gameplay.bootstrap", std::ios::binary);
		std::string baseline((std::istreambuf_iterator<char>(baselineInput)), std::istreambuf_iterator<char>());
		if (!baseline.empty() && baseline.back() != '\n') baseline += '\n';
		const std::string encounter = "ENCOUNTER_KOUKUSAYDON_G1", patternId = "KOUKUSAYDON_G1_SHOWTIME_CONTRACT";
		baseline += "PATTERN\t" + encounter + "\t" + patternId + "\t" + patternId + ".action\tAUDITION_ONLY\t0\t0\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0\n";
		baseline += "PATTERNBOSS\t" + encounter + "\t" + patternId + "\tBOSS_KOUKUSAYDON_G3_SAYDON\n";
		baseline += "PATTERNPOLICY\t" + encounter + "\t" + patternId + "\tNORMAL\t1\t1\tNONE\tNONE\n";
		baseline += "PATTERNSOURCE\t" + encounter + "\t" + patternId + "\t1\t0\t0\t0\t0\t0\t0\n";
		baseline += "PATTERNSTAGE\t" + encounter + "\t" + patternId + "\t0\tSTAGE_1\t" + patternId + ".stage.1\tACTIVE\t5000\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
		baseline += "PATTERNSTAGEBRANCH\t" + encounter + "\t" + patternId + "\t" + patternId + ".stage.1\tTIMEOUT\t-\n";
		baseline += "PATTERNTARGET\t" + patternId + "\tGATE3\tboss.koukusaydon.g3.saydon\n";
		fs::create_directories(fixture / L"Gameplay");
		CGameplayCatalog parsedTargets;
		const auto loadSupplement = [&](const std::string& row) {
			auto bytes = baseline + row + '\n';
			const auto headerEnd = bytes.find('\n'), headerCount = bytes.rfind('\t', headerEnd);
			bytes.replace(headerCount + 1u, headerEnd - headerCount - 1u, std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
			{ std::ofstream output(fixture / L"Gameplay/Gameplay.bootstrap", std::ios::binary | std::ios::trunc); output.write(bytes.data(), bytes.size()); }
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", fixture.c_str());
			const bool loadedTargets = parsedTargets.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", dataRootBuffer.data());
			if (!loadedTargets && row.find("PATTERNSUMMONSPAWN") != std::string::npos) std::cout << "Summon admission: " << parsedTargets.Get_Status() << std::endl;
			return loadedTargets;
		};
		const auto loadTargetRow = [&](const std::string& fields) {
			return loadSupplement("PATTERNSHOWTIMETARGETS\t" + encounter + "\t" + patternId + "\ttest.showtime.duration\t0\t5000\t" + fields);
		};
		const bool parsedBoth = loadTargetRow("test.fixed\ttest.tracking\t4384\t2000\t0.5");
		const auto* parsedPattern = parsedBoth ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(parsedPattern && parsedPattern->MechanicTriggers.size() == 1u &&
			parsedPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
			parsedPattern->MechanicTriggers.front().strFixedVisualId == "test.fixed" && parsedPattern->MechanicTriggers.front().strTrackingVisualId == "test.tracking",
			"The eleven-field Showtime row reaches the actual admitted Kouku pattern and visual consumers");
		tests.Require(loadTargetRow("-\ttest.tracking\t0\t2000\t0.5") && loadTargetRow("test.fixed\t-\t4384\t2000\t0.5"),
			"Catalog admission accepts independent fixed-only and tracking-only duration rows");
		const auto admittedRevision = parsedTargets.Get_ActiveRevision();
		bool rejectedInvalid = true;
		for (const std::string fields : { "-\t-\t0\t2000\t0.5", "test.fixed\ttest.tracking\t0\t2000\t0.5",
			"test.fixed\ttest.fixed\t4384\t2000\t0.5", "test.fixed\ttest.tracking\t4384\t0\t0.5",
			"test.fixed\ttest.tracking\t4384\t2000\tNaN", "test.fixed\ttest.tracking\t4384\t2000\t11" })
			rejectedInvalid = !loadTargetRow(fields) && parsedTargets.Get_ActiveRevision() == admittedRevision && rejectedInvalid;
		tests.Require(rejectedInvalid, "Invalid Showtime identity, lifetime, interval and speed preserve the previous admitted catalog");
		const std::string pursuitRow = "PATTERNPURSUITPROJECTILES\t" + encounter + "\t" + patternId +
			"\ttest.pursuit\t0\t901\ttest.heart\ttest.clover\ttest.diamond\ttest.spade\ttest.explosion\t15\t0.4\t2\t5000\t300\t0\t3";
		const bool parsedPursuit = loadSupplement(pursuitRow);
		const auto* pursuitPattern = parsedPursuit ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(pursuitPattern && pursuitPattern->MechanicTriggers.size() == 1u &&
			pursuitPattern->MechanicTriggers.front().ProjectileVisualIds.size() == 4u &&
			pursuitPattern->MechanicTriggers.front().iProjectileCountPerWave == 3u,
			"Eighteen-field pursuit bootstrap reaches the admitted typed mechanic");
		tests.Require(loadSupplement(pursuitRow + "\t15"), "Nineteen-field pursuit bootstrap accepts an independent distance cap");
		pursuitPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
		tests.Require(pursuitPattern && pursuitPattern->MechanicTriggers.front().fProjectileMaxDistanceM == 15.f,
			"Pursuit distance cap reaches the admitted Server mechanic");
		const auto pursuitRevision = parsedTargets.Get_ActiveRevision();
		for (const std::string distance : { "-1", "1000.1", "NaN", "wrong" })
			tests.Require(!loadSupplement(pursuitRow + "\t" + distance) && parsedTargets.Get_ActiveRevision() == pursuitRevision,
				"Invalid pursuit distance preserves the admitted catalog");
		std::string badPursuit = pursuitRow;
		badPursuit.replace(badPursuit.find("\t5000\t300"), 9u, "\t0\t300");
		tests.Require(!loadSupplement(badPursuit) && parsedTargets.Get_ActiveRevision() == pursuitRevision,
			"Unbounded repeated projectile publication preserves the previous catalog");
		const std::string randomOwner = "PATTERNSHOWTIMETARGETS	" + encounter + "	" + patternId + "	test.random.owner	0	5000	test.fixed	test.tracking	4384	2000	0.5";
		const auto randomRow = [&](const std::string& ownerId, const unsigned ordinal, const std::string& settings) {
			return "PATTERNSHOWTIMERANDOM	" + encounter + "	" + patternId + "	" + ownerId + "	" + std::to_string(ordinal) + "	test.random.a	5000	" + settings;
		};
		const auto randomZero = randomRow("test.random.owner", 0u, "500	2	0.2");
		const auto randomOne = randomRow("test.random.owner", 1u, "500	2	0.2");
		const bool randomAdmitted = loadSupplement(randomOwner + '\n' + randomZero + '\n' + randomOne);
		const auto* randomPattern = randomAdmitted ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(randomPattern && randomPattern->MechanicTriggers.front().RandomVolleys.size() == 2u &&
			randomPattern->MechanicTriggers.front().RandomVolleys[0].strClientVisualId == randomPattern->MechanicTriggers.front().RandomVolleys[1].strClientVisualId &&
			randomPattern->MechanicTriggers.front().iRandomSpawnIntervalMs == 500u,
			"Supplemental random rows exact-join their duration owner and preserve ordered duplicate template IDs");
		const auto randomRevision = parsedTargets.Get_ActiveRevision();
		bool randomRejected = true;
		for (const auto& invalid : std::vector<std::string>{randomZero, randomOwner + '\n' + randomOne,
			randomOwner + '\n' + randomZero + '\n' + randomZero, randomOwner + '\n' + randomZero + '\n' + randomRow("test.random.owner",1u,"1000	2	0.2"),
			randomOwner + '\n' + randomRow("missing.owner",0u,"500	2	0.2"), randomOwner + '\n' + randomRow("test.random.owner",0u,"0	2	0.2"),
			randomOwner + '\n' + randomRow("test.random.owner",0u,"500	NaN	0.2"), randomOwner + '\n' + randomRow("test.random.owner",0u,"500	1001	0.2")})
			randomRejected = !loadSupplement(invalid) && parsedTargets.Get_ActiveRevision() == randomRevision && randomRejected;
		tests.Require(randomRejected, "Invalid random owner, ordinal, duplicate ordinal, drifted cadence or arena bounds preserve the admitted catalog");
		const std::string trackPrefix = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId +
			"\ttest.track.duration\tBOSS_TRACK_TARGET\t612\t1079\t0\t";
		const std::string trackSuffix = "\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		const bool parsedTrack = loadSupplement(trackPrefix + "0\t0\t0" + trackSuffix);
		const auto* trackPattern = parsedTrack ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(trackPattern && trackPattern->MechanicTriggers.size() == 1u &&
			trackPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
			trackPattern->MechanicTriggers.front().iStartMs == 612u && trackPattern->MechanicTriggers.front().iDurationMs == 1079u,
			"The existing 25-field row admits a rotation-only duration with no visual templates");
		const auto trackRevision = parsedTargets.Get_ActiveRevision();
		tests.Require(!loadSupplement(trackPrefix + "1\t0\t0" + trackSuffix) && parsedTargets.Get_ActiveRevision() == trackRevision,
			"Rotation-only duration rejects unrelated teleport values and preserves the admitted generation");
		const std::string airBase="PATTERNMECHANICTRIGGER\t"+encounter+"\t"+patternId+"\tair.jump\tALBION_AIRBORNE\t0\t1000\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		const std::string airPrefix="PATTERNALBIONAIRBORNE\t"+encounter+"\t"+patternId+"\tair.jump\t";
		tests.Require(loadSupplement(airBase+"\n"+airPrefix+"JUMP\t13.6788133\t200"),
			"Full catalog admission resolves the seven-field airborne supplement after its exact base occurrence");
		tests.Require(loadSupplement(airBase+"\n"+airPrefix+"JUMP\t11.1910783\t0"),
			"The unchanged airborne bootstrap row admits explicit zero-duration initial height");
		const auto airRevision=parsedTargets.Get_ActiveRevision();
		bool badAirRejected=true;
		for (const std::string fields : {"UNKNOWN\t13\t200","JUMP\t0\t200","JUMP\t13\t600001","CENTER\t1\t0","SLAM\t0\t0","APPEAR_PLAYER\t3\t0","JUMP\tNaN\t200"})
			badAirRejected=!loadSupplement(airBase+"\n"+airPrefix+fields) && parsedTargets.Get_ActiveRevision()==airRevision && badAirRejected;
		tests.Require(badAirRejected,"Unknown phase, invalid height/duration, missing prior jump and absent source descent reject without replacing the catalog");
		tests.Require(!loadSupplement(airBase) && !loadSupplement(airPrefix+"JUMP\t13\t200\n"+airBase) &&
			!loadSupplement(airBase+"\n"+airPrefix+"JUMP\t13\t200\n"+airPrefix+"JUMP\t13\t200") && parsedTargets.Get_ActiveRevision()==airRevision,
			"Missing, reversed and duplicate airborne supplements preserve the previous catalog generation");
        const std::string selectedFields="SELECT_PLAYER\t0\t0\tSELECT\ttest.selected.group\t2000";
        tests.Require(loadSupplement(airBase+"\n"+airPrefix+selectedFields), "Ten-field SELECT supplement admits a captured group while legacy rows stay valid");
        const auto* selectedPattern=CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets,patternId,status);
        tests.Require(selectedPattern && selectedPattern->MechanicTriggers.front().bCaptureAirborneTargetPosition &&
            selectedPattern->MechanicTriggers.front().strSelectedEffectVisualId=="test.selected.group" &&
            selectedPattern->MechanicTriggers.front().iSelectedEffectLifetimeMs==2000u, "Selected point policy and visual exact-join the existing occurrence");
        const std::string selectedHit = "PATTERNATTACKHIT\t" + encounter + "\t" + patternId +
            "\tair.jump\tSELECTED\t0\t0\tselected.impact\tCONTACT\t1000\t1100\t1\t0\tRING\t4.5\t3\t0\t0\t0\t0\t0\t0\tMAX_HP_PERCENT\t10\t-";
        for (const auto* suffix : {"", "\t2\t1200", "\t2\t1200\t1", "\t0\t250\t1\t0\tAWAY_FROM_BOSS"})
        {
            tests.Require(loadSupplement(airBase + "\n" + airPrefix + selectedFields + "\n" + selectedHit + suffix),
                "Selected target hit admits legacy 25/27/28 fields and the 30-field explicit push policy");
            const auto* hitPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
            const bool explicitPolicy = std::string(suffix).find("AWAY_FROM_BOSS") != std::string::npos;
            const bool hasRange = explicitPolicy || std::string(suffix) == "\t2\t1200\t1";
            tests.Require(hitPattern && hitPattern->MechanicTriggers.front().FixedHits.size() == 1u &&
                hitPattern->MechanicTriggers.front().FixedHits.front().fPushRangeM == (hasRange ? 1.0 : 0.0) &&
                (!explicitPolicy || (hitPattern->MechanicTriggers.front().FixedHits.front().ForcePush == false &&
                 hitPattern->MechanicTriggers.front().FixedHits.front().strPushDirection == "AWAY_FROM_BOSS")),
                "Selected fixed hit parser preserves the source horizontal push and defaults legacy rows to zero");
        }
        const auto selectedRevision=parsedTargets.Get_ActiveRevision();
        tests.Require(!loadSupplement(airBase + "\n" + airPrefix + selectedFields + "\n" + selectedHit + "\t2\t1200\t101") &&
            parsedTargets.Get_ActiveRevision() == selectedRevision,
            "An oversized selected-hit push range preserves the previously admitted catalog");
        bool invalidSelectedRejected=true;
        for (const std::string fields : {"SELECT_PLAYER\t0\t0\tUNKNOWN\ttest.selected.group\t2000", "JUMP\t13\t200\tSELECT\ttest.selected.group\t2000",
            "SELECT_PLAYER\t0\t0\tSELECT\ttest.selected.group\t0", "SELECT_PLAYER\t0\t0\tSELECT\t\t2000", "SELECT_PLAYER\t0\t0\tSELECT\ttest.selected.group\t600001"})
            invalidSelectedRejected=!loadSupplement(airBase+"\n"+airPrefix+fields) && parsedTargets.Get_ActiveRevision()==selectedRevision && invalidSelectedRejected;
        tests.Require(invalidSelectedRejected, "Malformed selected policies, phase owners and lifetime pairs preserve the admitted catalog");
		const std::string teleportPrefix = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId + "\ttest.teleport.xz\tBOSS_TELEPORT_XZ\t0\t1000\t0\t";
		const std::string teleportSuffix = "\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		const bool parsedTeleport = loadSupplement(teleportPrefix + "2.57\t1.3\t952.27" + teleportSuffix);
		const auto* teleportedPattern = parsedTeleport ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
		tests.Require(teleportedPattern && teleportedPattern->MechanicTriggers.size() == 1u &&
			teleportedPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ &&
			teleportedPattern->MechanicTriggers.front().fTeleportY == 1.3f &&
			teleportedPattern->MechanicTriggers.front().fTeleportZ == 952.27f,
			"The existing 25-field mechanic row admits XZ teleport and preserves its authored reference Y");
		const auto teleportRevision = parsedTargets.Get_ActiveRevision();
		tests.Require(!loadSupplement(teleportPrefix + "100001\t1.3\t952.27" + teleportSuffix) &&
			parsedTargets.Get_ActiveRevision() == teleportRevision,
			"Out-of-bounds XZ teleport preserves the previous admitted catalog");
        auto groundedPrefix = teleportPrefix;
        groundedPrefix.replace(groundedPrefix.find("BOSS_TELEPORT_XZ"), std::string("BOSS_TELEPORT_XZ").size(), "BOSS_TELEPORT_GROUNDED");
        const bool parsedGrounded = loadSupplement(groundedPrefix + "-0.07\t1.32\t942.33" + teleportSuffix);
        const auto* groundedPattern = parsedGrounded ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
        tests.Require(groundedPattern && groundedPattern->MechanicTriggers.size() == 1u &&
            groundedPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED &&
            groundedPattern->MechanicTriggers.front().fTeleportY == 1.32f,
            "Grounded teleport joins the existing 25-field mechanic row with its authored reference floor");
        const auto groundedRevision = parsedTargets.Get_ActiveRevision();
        tests.Require(!loadSupplement(groundedPrefix + "-0.07\t100001\t942.33" + teleportSuffix) &&
            parsedTargets.Get_ActiveRevision() == groundedRevision, "Invalid grounded height preserves the admitted product catalog");
		const std::string groupId="world.object.kouku.saydon.circus.split", groupCue="test.circus.group.world.1";
		const std::string groupRow="PATTERNWORLDSEQUENCE\t"+encounter+"\t"+patternId+"\t0\t"+groupId+"\t1\t0\t0\t0\tNONE\t0\t0\t0\t11500\t"+groupCue;
		const std::string groupPlacement="PATTERNWORLDPLACEMENT\t"+encounter+"\t"+patternId+"\t"+groupCue+"\t2\t1\t3\t0\t40\t0\t1\t1\t1";
		const bool groupLoaded=loadSupplement(groupRow+"\n"+groupPlacement);
		const auto* groupPattern=groupLoaded?CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets,patternId,status):nullptr;
		tests.Require(groupPattern && groupPattern->WorldSequences.size()==1u && groupPattern->WorldSequences[0].strInstanceId==groupId &&
			groupPattern->WorldSequences[0].iDurationMs==11500u && groupPattern->WorldSequences[0].Placement.has_value(),
			"One model-less group retains its stable ID, placement and full 63-ball presentation lifetime through actual catalog admission");
		if (groupPattern)
		{
			auto groupBoss=std::make_unique<SERVER_WORLD_ENTITY>(albionOwner); groupBoss->strPatternId=patternId; groupBoss->iPatternSequence=91u;
			KOUKUSAYDON_LOGIC_LEDGER groupLedger; KOUKUSAYDON_LOGIC_OUTPUT groupOutput;
			std::map<PLAYER_ID,SERVER_PLAYER> groupPlayers; std::vector<DAMAGE_EVENT> groupDamage;
			CKoukuSaydonLogicRuntime::Build(*groupPattern,*groupBoss,13000u,groupLedger);
			CKoukuSaydonLogicRuntime::Update(*groupBoss,*groupPattern,groupLedger,groupPlayers,parsedTargets,nullptr,13000u,groupDamage,groupOutput);
			tests.Require(groupOutput.WorldSequencePlays.size()==1u && groupOutput.WorldSequencePlays[0].strInstanceId==groupId &&
				groupOutput.WorldSequencePlays[0].strOccurrenceId==groupCue && groupOutput.WorldSequencePlays[0].iDurationMs==11500u &&
				groupOutput.WorldSequencePlays[0].iStartTick==13000u && groupOutput.WorldSequencePlays[0].Placement->fRotationYDegrees==40.f && groupDamage.empty(),
				"Existing Server Logic emits one owned group play at its authored clock without splitting the world cue or creating damage");
			groupOutput={}; CKoukuSaydonLogicRuntime::Update(*groupBoss,*groupPattern,groupLedger,groupPlayers,parsedTargets,nullptr,13001u,groupDamage,groupOutput);
			tests.Require(groupOutput.WorldSequencePlays.empty(),"Later fixed ticks do not replay or duplicate the group volley");
		}

		// The actual dependent-boss spawn transaction consumes the optional MAP pose and all ten rows.
		{
			const std::string childId = "KOUKUSAYDON_G1_SUMMON_CONTRACT_CHILD";
			const auto start = baseline.find("PATTERN\t" + encounter + "\t" + patternId + "\t");
			std::string childRows = baseline.substr(start);
			for (std::size_t pos = 0u; (pos = childRows.find(patternId, pos)) != std::string::npos; pos += childId.size())
				childRows.replace(pos, patternId.size(), childId);
			const auto category = childRows.find("\tNORMAL\t");
			childRows.replace(category, 8u, "\tMECHANIC\t");
			{
				const std::string prefix = encounter + "\t" + patternId;
				const std::string tailRows = childRows + "PATTERNTIMELINE\t" + prefix + "\t11000\n" +
					"PATTERNWORLDSEQUENCE\t" + prefix + "\t6000\tworld.tail.contract\t1\t0\t0\t0\tNONE\t0\t0\t0\t3000\ttail.world.1\n" +
					"PATTERNMECHANICTRIGGER\t" + prefix + "\ttail.summon\tSUMMON_PATTERNS\t6000\t5000\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\n" +
					"PATTERNSUMMONSPAWN\t" + prefix + "\ttail.summon\ttail.spawn\t" + childId + "\t4\t1\t4\t45\tMAP\n" +
					"PATTERNLOGIC\t" + prefix + "\t0\ttail.hit\tENTER_AREA\t6000\t3000\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\n" +
					"PATTERNLOGICREGION\t" + prefix + "\ttail.hit\t0\ttail.body\tBOSS_SPAWN\tCIRCLE\t0\t0\t0\t0\t1\t1\t1\t2\t45\tNONE\tNONE\n" +
					"PATTERNLOGICREGION\t" + prefix + "\ttail.hit\t1\ttail.follow\tBOSS_CURRENT\tCIRCLE\t0\t0\t0\t0\t1\t1\t1\t2\t45\tNONE\tNONE\n" +
					"PATTERNLOGICOUTCOME\t" + prefix + "\ttail.hit\tSUCCESS\t0\tMAX_HP_PERCENT_DAMAGE\t10\t0\t-";
				const bool tailLoaded = loadSupplement(tailRows);
				tests.Require(tailLoaded, "Catalog admits independent late Logic, World and Summon rows without extending the five-second Stage");
				if (!tailLoaded) std::cout << "Tail admission: " << parsedTargets.Get_Status() << '\n';
				if (tailLoaded) for (const bool explicitStop : {false, true})
				{
					auto tailRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
					tailRoom->m_WorldEntities.clear(); tailRoom->m_ServerNavigation = nav;
					tailRoom->m_ServerCollisionSystem.Initialize({}, status);
					auto& run = tailRoom->m_KoukuSaydonPatternAudition;
					run.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
					run.iRoomAuditionEpoch = 99u;
					run.iOwnerSessionId = 77u; run.Request.Scope.strGateId = "GATE3";
					run.PinnedGameplayRevision = tailRoom->m_GameplayCatalog.Get_ActiveRevision();
					run.pProductGeneration = std::make_shared<CGameplayCatalog>(parsedTargets);
					run.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(parsedTargets);
					const auto* definition = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
					WORLD_BOOTSTRAP_PLACEMENT place;
					place.eKind = WORLD_BOOTSTRAP_KIND::BOSS; place.strPlacementId = "boss.koukusaydon.g3.saydon";
					place.strArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON"; place.strEncounterId = encounter;
					place.fPositionX = place.fPositionZ = 8.f; place.fPositionY = 1.f;
					SERVER_WORLD_ENTITY primary;
					const bool ready = definition && tailRoom->Build_WorldEntity(place, 900u, primary) &&
						tailRoom->m_KoukuSaydonBrain.Begin_Pattern(primary, *definition, run.PinnedGameplayRevision, 10000u, status);
					tests.Require(ready, "Independent row fixture admits the real boss and pinned catalog");
					if (!ready) continue;
					tailRoom->m_WorldEntities.push_back(primary); tailRoom->m_iNextNetEntityId = 1000u;
					auto& member = run.Members.emplace_back(); member.strMemberId = "tail.member";
					member.iBossEntityId = 900u; member.iPatternSequence = primary.iPatternSequence;
					member.PatternIds = {patternId}; member.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
					CKoukuSaydonLogicRuntime::Build(*definition, primary, 10000u, member.LogicLedger);
					auto& player = tailRoom->m_Players[1u]; player.iPlayerId = 1u; player.iNetEntityId = 901u;
					player.iMaximumHp = player.iCurrentHp = 100u; player.isCombatReady = true;
					player.fPositionX = player.fPositionZ = 8.f; player.fPositionY = 1.f;
					auto& follower = tailRoom->m_Players[2u]; follower = player; follower.iPlayerId = 2u; follower.iNetEntityId = 902u;
					follower.fPositionX = follower.fPositionZ = 14.f;
					tailRoom->m_iServerTick = 10150u;
					tests.Require(tailRoom->Update_KoukuSaydonBoss(tailRoom->m_WorldEntities.front(), 10150u) &&
						tailRoom->m_WorldEntities.front().strPatternId.empty() && run.Tails.size() == 1u &&
						run.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE,
						"The five-second Stage finishes and emits natural completion before the eleven-second row lifetime");
					if (run.Tails.empty()) continue;
					// A newer body occurrence must not steal the old row origin, identity or ledger.
					auto& newer = tailRoom->m_WorldEntities.front(); newer.strPatternId = childId;
					newer.iPatternSequence = 77u; newer.fPositionX = newer.fPositionZ = 14.f;
					// 새 플레이어의 10% 기준 피해는 9..11의 일반 피격 표본이다. 구간만 느슨하게 비교하지 않고
					// 각 대상의 event·HP 감소·serial을 함께 확인해 잘못된 대상과 다음 tick의 중복도 거부한다.
					const auto hasOneTailHit = [&](const SERVER_PLAYER& target) {
						std::uint32_t amount = 0u;
						std::size_t count = 0u;
						for (const auto& hit : tailRoom->m_TickDamageEvents) {
							if (hit.iTargetNetEntityId != target.iNetEntityId) continue;
							if (hit.isOutgoing || hit.eHitFlag != DAMAGE_HIT_FLAG::NORMAL || hit.iAmount < 9u || hit.iAmount > 11u)
								return false;
							amount += hit.iAmount; ++count;
						}
						return count == 1u && target.iIncomingDamageSampleSerial == 1u &&
							target.iCurrentHp <= 100u && amount == 100u - target.iCurrentHp;
					};
					tailRoom->Update_KoukuPatternTails(10180u); tailRoom->Commit_KoukuMechanicTriggers(10180u);

					tests.Require(hasOneTailHit(player) && hasOneTailHit(follower) && tailRoom->m_TickDamageEvents.size() == 2u && run.WorldPlays.size() == 1u &&
						run.WorldPlays.front().iPatternSequence == primary.iPatternSequence &&
						run.WorldPlays.front().iStartTick == 10180u && tailRoom->m_WorldEntities.size() == 2u &&
						tailRoom->m_WorldEntities.front().iPatternSequence == 77u && tailRoom->m_WorldEntities.front().fPositionX == 14.f,
						"Late Logic preserves BOSS_SPAWN and follows BOSS_CURRENT; World keeps its clock and Summon leaves the newer Pattern intact");
					tailRoom->Update_KoukuPatternTails(10181u); tailRoom->Commit_KoukuMechanicTriggers(10181u);
					tailRoom->Update_KoukuGazeClones(10181u);
					tests.Require(hasOneTailHit(player) && hasOneTailHit(follower) && tailRoom->m_TickDamageEvents.size() == 2u && run.WorldPlays.size() == 1u && tailRoom->m_WorldEntities.size() == 2u,
						"Later ticks neither duplicate row outputs nor remove the summon when the primary sequence has advanced");
					{
						const auto oldPin = run.PinnedGameplayRevision;
						const bool newGeneration = tailRoom->Stage_GameplayGeneration(1u, oldPin,
							std::make_shared<CGameplayCatalog>(parsedTargets), status) && tailRoom->Commit_GameplayGeneration(1u);
						for (auto& entity : tailRoom->m_WorldEntities) entity.PinnedDefinitionRevision = tailRoom->m_GameplayCatalog.Get_ActiveRevision();
						std::vector<GameplayDataRevision> pins;
						const bool pinned = tailRoom->Build_RequiredPinnedGameplayRevisions(pins);
						tailRoom->m_GameplayCatalog.Collect_Garbage(pins);
						tests.Require(newGeneration && pinned && std::find(pins.begin(), pins.end(), oldPin) != pins.end() &&
							tailRoom->m_GameplayCatalog.Resolve(oldPin) != nullptr,
							"A naturally completed run pins its row generation through catalog replacement and garbage collection");
					}
					{
						auto& oldOwner = *run.Tails.front().pOwner;
						auto& currentBody = tailRoom->m_WorldEntities.front();
						(void)CBossCombatRuntime::Set_Flag(oldOwner.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
						const bool countered = CBossCombatRuntime::Try_TriggerCounter(currentBody, 10182u);
						tests.Require(countered && oldOwner.BossCombat.PendingOutcomes.size() == 1u &&
							oldOwner.BossCombat.PendingOutcomes.front().iPatternSequence == primary.iPatternSequence &&
							currentBody.BossCombat.PendingOutcomes.empty() && currentBody.iPatternSequence == 77u &&
							!CBossCombatRuntime::Try_TriggerCounter(currentBody, 10182u),
							"A retained counter flag consumes once and publishes only to its born Pattern, not the newer actor");
						oldOwner.bKoukuShieldActive = true; oldOwner.fKoukuShieldArcDegrees = 60.f; oldOwner.fYawDegrees = 0.f;
						tests.Require(CKoukuSaydonLogicRuntime::Is_ShieldReflected(currentBody, 14.f, 18.f) &&
							!CKoukuSaydonLogicRuntime::Is_ShieldReflected(currentBody, 14.f, 10.f),
							"The actual damage consumer sees the retained shield at its live BOSS_CURRENT root");
						oldOwner.bKoukuShieldActive = false;
					}

					auto support = scheduled; support.iStartTick = 10150u; support.iEndTick = 10330u; support.strMemberId = "tail.member";
					run.SupportSchedule.push_back(support);
					tailRoom->Stop_KoukuWorldOwner("tail.member", true);
					tests.Require(run.Tails.size() == 1u && run.WorldPlays.size() == 1u && run.SupportSchedule.size() == 1u,
						"FINISH_OWNER preserves independent Logic, World and navigation support rows");
					if (!explicitStop)
					{
						auto& raid = tailRoom->m_KoukuRaid;
						raid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT; raid.State.strGateId = "GATE3";
						raid.iOwnerSessionId = run.iOwnerSessionId; raid.iAuditionEpoch = run.iRoomAuditionEpoch;
						auto& body = tailRoom->m_WorldEntities.front(); body.strPatternId.clear(); body.strActionId.clear();
						C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request;
						request.iRequestSequence = 1u; request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
						request.strPatternId = childId; request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
						request.Scope.strEncounterId = encounter; request.Scope.strGateId = "GATE3";
						request.Scope.strBossPlacementId = "boss.koukusaydon.g3.saydon";
						request.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON";
						request.Scope.ExpectedGameplayRevision = run.PinnedGameplayRevision;
						request.Scope.iExpectedSourceRevision = run.iPinnedSourceRevision;
						S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
						tailRoom->m_iServerTick = 10182u;
						const auto verdict = tailRoom->Evaluate_KoukuSaydonPatternAudition(77u, request, result, true);
						if (verdict != KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED) std::cout << "Continuation admission: " << result.strReason << '\n';
						tailRoom->Prepare_KoukuAuditionTick(10183u);
						tests.Require(verdict == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED && run.iRoomAuditionEpoch == 99u &&
							run.Tails.size() == 1u && run.WorldPlays.size() == 1u && run.SupportSchedule.size() == 1u &&
							tailRoom->m_WorldEntities.front().strPatternId == childId,
							"Automatic next Flow Entry starts the next actor Pattern in the same epoch while prior rows remain alive");
					}
					if (explicitStop)
					{
						tailRoom->Clear_KoukuSaydonPatternAudition(); tailRoom->Update_KoukuGazeClones(10182u);
						tests.Require(run.Tails.empty() && run.WorldPlays.empty() && run.SupportSchedule.empty() && tailRoom->m_WorldEntities.size() == 1u,
							"Explicit Stop cancels tails, pending supports and already spawned summons");
					}
					else
					{
						tailRoom->Update_KoukuPatternTails(10330u); tailRoom->Update_KoukuGazeClones(10330u);
						tailRoom->Update_KoukuPatternTails(10331u);
						tests.Require(run.Tails.empty() && run.WorldPlays.empty() && tailRoom->m_WorldEntities.size() == 1u,
							"Each row and summon expires at its own absolute deadline after the Stage has completed");
					}
				}
			}
			const std::string triggerRow = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId +
				"\ttest.summon\tSUMMON_PATTERNS\t0\t5000\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\n";
			const auto spawnRows = [&](const unsigned count, const std::string& anchor) {
				std::string result = childRows + triggerRow;
				for (unsigned index = 0u; index < count; ++index)
					result += "PATTERNSUMMONSPAWN\t" + encounter + "\t" + patternId + "\ttest.summon\ttest.spawn." +
						std::to_string(index) + "\t" + childId + "\t" + (index < 5u ? "4" : "12") + "\t1\t" +
						std::to_string(4u + 2u * (index % 5u)) + "\t45" + anchor + "\n";
				if (!result.empty() && result.back() == '\n') result.pop_back();
				return result;
			};
			tests.Require(loadSupplement(spawnRows(16u, "\tMAP")), "Catalog admits sixteen bounded MAP Pattern spawn rows");
			const auto beforeInvalidSummon = parsedTargets.Get_ActiveRevision();
			tests.Require(!loadSupplement(spawnRows(17u, "\tMAP")) && parsedTargets.Get_ActiveRevision() == beforeInvalidSummon,
				"A seventeenth Summon spawn preserves the admitted catalog");
			tests.Require(!loadSupplement(spawnRows(10u, "\tBAD")) && parsedTargets.Get_ActiveRevision() == beforeInvalidSummon,
				"An invalid spawn anchor preserves the admitted catalog");
			tests.Require(loadSupplement(spawnRows(10u, "")), "Legacy ten-field summon rows retain the BOSS anchor default");
			tests.Require(loadSupplement(spawnRows(10u, "\tMAP")), "Catalog admits ten independently placed MAP actors");
			auto summonRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
			summonRoom->m_WorldEntities.clear(); summonRoom->m_ServerNavigation = nav;
			summonRoom->m_ServerCollisionSystem.Initialize({}, status); summonRoom->m_ServerCollisionSystem.Set_BlockingBodies({});
			auto& run = summonRoom->m_KoukuSaydonPatternAudition;
			run.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
			run.PinnedGameplayRevision = summonRoom->m_GameplayCatalog.Get_ActiveRevision();
			run.pProductGeneration = std::make_shared<CGameplayCatalog>(parsedTargets);
			run.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(parsedTargets);
			const auto* parent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status);
			WORLD_BOOTSTRAP_PLACEMENT placement;
			placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS; placement.strPlacementId = "boss.koukusaydon.g3.saydon";
			placement.strArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON"; placement.strEncounterId = encounter;
			placement.fPositionX = placement.fPositionZ = 8.f; placement.fPositionY = 1.f; placement.fYawDegrees = 120.f;
			SERVER_WORLD_ENTITY primary;
			const bool ready = parent && parent->MechanicTriggers.size() == 1u &&
				parent->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS &&
				parent->MechanicTriggers.front().PatternSpawns.size() == 10u && summonRoom->Build_WorldEntity(placement, 900u, primary) &&
				summonRoom->m_KoukuSaydonBrain.Begin_Pattern(primary, *parent, run.PinnedGameplayRevision, 13000u, status);
			tests.Require(ready, "The real primary boss admits the Summon parent in its pinned Product");
			if (ready)
			{
				summonRoom->m_WorldEntities.push_back(primary); summonRoom->m_iNextNetEntityId = 1000u;
				auto trigger = parent->MechanicTriggers.front();
				const auto submit = [&](const BOSS_PATTERN_MECHANIC_TRIGGER& value) {
					summonRoom->m_PendingKoukuMechanicTriggers.push_back({900u,primary.iPatternSequence,value});
					summonRoom->Commit_KoukuMechanicTriggers(13000u);
				};
				auto bad = trigger; bad.PatternSpawns.back().PositionOffset = {6.f,1.f,2.f}; submit(bad);
				tests.Require(summonRoom->m_WorldEntities.size() == 1u && summonRoom->m_iNextNetEntityId == 1000u,
					"An invalid tenth navigation point rolls back all ten staged summon actors and IDs");
				bad = trigger; bad.PatternSpawns.back().PositionOffset[1] = 10.f; submit(bad);
				tests.Require(summonRoom->m_WorldEntities.size() == 1u && summonRoom->m_iNextNetEntityId == 1000u,
					"A MAP point on the wrong navigation floor rolls back the entire formation");
				submit(trigger);
				bool poses = summonRoom->m_WorldEntities.size() == 11u;
				for (const auto& actor : summonRoom->m_WorldEntities) if (actor.bKoukuSummonClone)
					poses = poses && actor.iOwnerBossNetEntityId == 900u && actor.strPatternId == childId && actor.fYawDegrees == 45.f &&
						(actor.fPositionX == 4.f || actor.fPositionX == 12.f) && actor.fPositionY == 1.f && actor.KoukuSummonStartedTriggers.empty();
				tests.Require(poses && summonRoom->m_iNextNetEntityId == 1010u,
					"Ten MAP actors commit independent Patterns and absolute poses despite the primary yaw");
				summonRoom->Update_KoukuGazeClones(13001u);
				tests.Require(summonRoom->m_WorldEntities.size() == 11u, "The next clone simulation tick preserves all ten active children");
				summonRoom->m_WorldEntities.front().strPatternId.clear(); summonRoom->Update_KoukuGazeClones(13002u);
				tests.Require(summonRoom->m_WorldEntities.size() == 1u, "Stopping the parent removes all ten owned actors together");
			}
		}
	}
	else tests.Require(false, "Showtime catalog admission requires the configured isolated Server data root");
#endif
	std::error_code cleanupError; fs::remove_all(fixture, cleanupError);
	std::cout << "failures : " << tests.failures << std::endl;
	return tests.failures ? 1 : 0;
}


int LostArk::Server::CServerGameplayContractRunner::Run_ShowtimeBombs()
{
    TESTS tests;
    auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
    tests.Require(room->Is_Ready(), "Showtime bomb fixture admits the installed gameplay catalog");
    if (!room->Is_Ready()) return 1;
    // Exercise the exact eighteen-field row through the real catalog parser.
    {
        namespace fs = std::filesystem;
        wchar_t configured[32768]{};
        const auto length = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", configured, 32768u);
        tests.Require(length && length < 32768u, "Isolated Showtime tests receive an explicit Server data root");
        if (!length || length >= 32768u) return 1;
        std::ifstream input(fs::path(configured) / L"Gameplay/Gameplay.bootstrap", std::ios::binary);
        std::string baseline{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        const std::string owner = "ENCOUNTER_KOUKUSAYDON_G1\tKOUKUSAYDON_G1_SHOWTIME_BOMB_CONTRACT";
        if (!baseline.empty() && baseline.back() != '\n') baseline += '\n';
        baseline += "PATTERN\t" + owner + "\tbomb.action\tAUDITION_ONLY\t0\t0\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0\n";
        baseline += "PATTERNBOSS\t" + owner + "\tBOSS_KOUKUSAYDON_G3_SAYDON\n";
        baseline += "PATTERNPOLICY\t" + owner + "\tNORMAL\t1\t1\tNONE\tNONE\n";
        baseline += "PATTERNSOURCE\t" + owner + "\t1\t0\t0\t0\t0\t0\t0\n";
        baseline += "PATTERNSTAGE\t" + owner + "\t0\tSTAGE_1\tbomb.stage.1\tACTIVE\t5000\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
        baseline += "PATTERNSTAGEBRANCH\t" + owner + "\tbomb.stage.1\tTIMEOUT\t-\n";
        baseline += "PATTERNTARGET\tKOUKUSAYDON_G1_SHOWTIME_BOMB_CONTRACT\tGATE3\tboss.koukusaydon.g3.saydon\n";
        baseline += "PATTERNMECHANICTRIGGER\t" + owner + "\tbomb.track\tBOSS_TRACK_TARGET\t1000\t1000\t0\t0\t0\t0\t-\t0\t0\t0\t1\n";
        const std::string row = "PATTERNTRACKBOMB\t" + owner + "\tbomb.track\t0\tbomb.body\tbomb.explosion\t2801\t12\t1\t8\t0\t0\t0\t90\t24.2\t13.75\t22.5\n";
        const auto fixture = fs::temp_directory_path() / (L"LostArkShowtimeBomb-" + std::to_wstring(_getpid()));
        fs::create_directories(fixture / L"Gameplay");
        CGameplayCatalog parsed;
        const auto load = [&](const std::string& supplement) {
            auto bytes = baseline + supplement;
            const auto headerEnd = bytes.find('\n'), countAt = bytes.rfind('\t', headerEnd);
            if (headerEnd == std::string::npos || countAt == std::string::npos) return false;
            bytes.replace(countAt + 1u, headerEnd - countAt - 1u, std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
            { std::ofstream file(fixture / L"Gameplay/Gameplay.bootstrap", std::ios::binary); file.write(bytes.data(), bytes.size()); }
            SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", fixture.c_str());
            const bool accepted = parsed.Load();
            SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", configured);
            return accepted;
        };
        std::string status;
        const bool accepted = load(row);
        const auto* saved = accepted ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsed, "KOUKUSAYDON_G1_SHOWTIME_BOMB_CONTRACT", status) : nullptr;
        if (!saved) std::cout << "Bomb row admission: " << parsed.Get_Status() << " / " << status << std::endl;
        tests.Require(saved && saved->MechanicTriggers.size() == 1u && saved->MechanicTriggers.front().ShowtimeBomb &&
            saved->MechanicTriggers.front().ShowtimeBomb->strBodyVisualId == "bomb.body" &&
            saved->MechanicTriggers.front().ShowtimeBomb->FanRegion.fYawDegrees == 90.f,
            "Actual eighteen-field PATTERNTRACKBOMB parses into its exact tracking owner and passes brain validation");
        tests.Require(!load(row + row), "A repeated tracking bomb attachment is rejected");
        auto invalid = row; invalid.replace(invalid.rfind("22.5"), 4u, "180");
        tests.Require(!load(invalid), "The parser rejects a full-circle bomb fan");
        invalid = row; invalid.replace(invalid.find("\t24.2\t"), 6u, "\tnan\t");
        tests.Require(!load(invalid), "The parser rejects non-finite bomb fan axes");
        std::error_code ignored;
        fs::remove(fixture / L"Gameplay/Gameplay.bootstrap", ignored); fs::remove(fixture / L"Gameplay", ignored); fs::remove(fixture, ignored);
    }
    room->m_WorldEntities.clear();
    room->m_WorldEntities.emplace_back();
    auto& boss = room->m_WorldEntities.front();
    BOSS_PATTERN_DEFINITION pattern;
    KOUKUSAYDON_LOGIC_LEDGER ledger;
    const std::uint32_t birth = 40000u;
    const auto begin = [&](const unsigned waves, const bool raid) {
        room->m_CombatObjectRuntime.Reset(); room->m_CombatObjectRuntime.Discard_PendingLifecycle();
        room->m_Players.clear(); room->m_TickDamageEvents.clear();
        room->m_KoukuSaydonPatternAudition = {}; room->m_KoukuRaid = {};
        for (PLAYER_ID id = 1u; id <= 5u; ++id)
        {
            auto& player = room->m_Players[id]; player.iPlayerId = id; player.iNetEntityId = 100u + id;
            player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true;
            player.fPositionX = 12.f; player.fPositionY = 1.f; player.fPositionZ = 8.f;
            player.iShield = 1000u; player.iInvulnerableEndTick = birth + 10000u;
        }
        if (raid) { room->m_KoukuRaid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT; room->m_KoukuRaid.PlayerIds = {1u,2u,3u,4u}; }
        boss = {};
        boss.iNetEntityId = 700u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1"; boss.strArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON";
        boss.iCurrentHp = boss.iMaximumHp = 100u; boss.PinnedDefinitionRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        boss.fPositionX = boss.fPositionZ = 8.f; boss.fPositionY = 1.f;
        boss.iPatternSequence = 71u; boss.iPatternStartTick = birth;
        boss.iPatternTargetEntityId = boss.iTargetEntityId = 101u;
        pattern = {}; pattern.strPatternId = boss.strPatternId = "KOUKUSAYDON_TEST_SHOWTIME_BOMBS";
        for (unsigned wave = 0u; wave < waves; ++wave)
        {
            BOSS_PATTERN_MECHANIC_TRIGGER trigger;
            trigger.strTriggerId = "showtime.track." + std::to_string(wave);
            trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
            trigger.iStartMs = wave * 3000u + 1000u; trigger.iDurationMs = 1000u;
            BOSS_SHOWTIME_BOMB_RESOLUTION bomb;
            bomb.iBodyStartMs = wave * 3000u; bomb.strBodyVisualId = "showtime.bomb.body";
            bomb.strExplosionVisualId = "showtime.bomb.explosion"; bomb.iExplosionLifetimeMs = 2801u;
            bomb.BodyPosition = {12.f,1.f,8.f};
            auto& fan = bomb.FanRegion; fan.strRegionId = trigger.strTriggerId;
            fan.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT; fan.bSector = true;
            fan.fRadiusM = fan.fRadiusXM = 24.2f; fan.fRadiusZM = 13.75f;
            fan.fHalfAngleDegrees = 22.5f; fan.fYawDegrees = 90.f;
            trigger.ShowtimeBomb = bomb; pattern.MechanicTriggers.push_back(std::move(trigger));
        }
        CKoukuSaydonLogicRuntime::Build(pattern, boss, birth, ledger);
    };
    const auto update = [&](const std::uint32_t tick) {
        room->Update_KoukuPlayerTargets(boss, pattern, ledger, room->m_GameplayCatalog, tick);
    };
    const auto countVisual = [&](const char* id) {
        return std::count_if(room->m_CombatObjectRuntime.Get_LiveObjects().begin(), room->m_CombatObjectRuntime.Get_LiveObjects().end(),
            [&](const auto& object) { return object.strClientVisualId == id; });
    };
    begin(4u, true); update(birth - 1u);
    tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "Bomb body is absent before its authored birth");
    update(birth); update(birth);
    tests.Require(countVisual("showtime.bomb.body") == 1 && boss.fYawDegrees == 0.f && !ledger.PlayerTargetWindows.front().bClosed,
        "One body plus fuse is born before tracking without rotating or resolving the owner");
    std::vector<S2C_COMBAT_OBJECT_SPAWNED> restored;
    room->m_CombatObjectRuntime.Build_LiveSpawnMessages(birth + 10u, restored);
    bool wire = restored.size() == 1u;
    for (const auto& spawn : restored)
    {
        CPacketWriter writer; S2C_COMBAT_OBJECT_SPAWNED decoded;
        wire = Write_Message(writer, spawn) && wire; CPacketReader reader{writer.Get_Buffer()};
        wire = Read_Message(reader, decoded) && wire && decoded.iSpawnTick == birth && decoded.iServerTick == birth + 10u &&
            decoded.strClientVisualId == "showtime.bomb.body" && decoded.fPositionX == 12.f && decoded.fPositionZ == 8.f;
    }
    tests.Require(wire, "Bomb body uses the existing reliable spawn and late-join clock contract");
    bool finalYaw = true, noEarlyVerdict = true;
    for (unsigned wave = 0u; wave < 4u; ++wave)
    {
        const auto offset = wave * 90u;
        update(birth + offset);
        room->m_Players.at(1u).fPositionX = 4.f;
        update(birth + offset + 30u);
        noEarlyVerdict = noEarlyVerdict && !ledger.PlayerTargetWindows[wave].bClosed;
        room->m_Players.at(1u).fPositionX = 12.f;
        update(birth + offset + 59u); // The last live tracking tick aims exactly at +X.
        const auto yaw = boss.fYawDegrees;
        room->m_Players.at(1u).fPositionX = 4.f;
        update(birth + offset + 60u); update(birth + offset + 60u); update(birth + offset + 59u);
        finalYaw = finalYaw && boss.fYawDegrees == yaw && ledger.PlayerTargetWindows[wave].bClosed &&
            ledger.PlayerTargetWindows[wave].BombInsideAtEnd == true;
    }
    tests.Require(noEarlyVerdict && finalYaw && countVisual("showtime.bomb.body") == 0 && countVisual("showtime.bomb.explosion") == 0 &&
        room->m_TickDamageEvents.empty(), "All four bombs judge only the final fan, preserve end-tick yaw and defuse once without an explosion");
    std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
    std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulses;
    std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
    room->m_CombatObjectRuntime.Drain_Lifecycle(spawned, pulses, despawned);
    tests.Require(spawned.size() == 4u && despawned.size() == 4u, "Four successes replicate four exact body removals without duplicate lifecycle edges");

    begin(1u, true);
    const auto& fan = pattern.MechanicTriggers.front().ShowtimeBomb->FanRegion;
    tests.Require(CKoukuSaydonLogicRuntime::Is_PointInsideRegion(fan, boss, 21.74f, 8.f) &&
        !CKoukuSaydonLogicRuntime::Is_PointInsideRegion(fan, boss, 21.76f, 8.f) &&
        !CKoukuSaydonLogicRuntime::Is_PointInsideRegion(fan, boss, 13.f, 18.f) &&
        !CKoukuSaydonLogicRuntime::Is_PointInsideRegion(fan, boss, std::numeric_limits<float>::quiet_NaN(), 8.f),
        "Final fan preserves authored elliptic axes, semantic +Z, angular bounds and finite point validation");
    auto edgeFan = fan; edgeFan.fYawDegrees = 0.f; edgeFan.fHalfAngleDegrees = 90.f;
    tests.Require(CKoukuSaydonLogicRuntime::Is_PointInsideRegion(edgeFan, boss, 8.f, 8.f) &&
        CKoukuSaydonLogicRuntime::Is_PointInsideRegion(edgeFan, boss, 9.f, 8.f) &&
        CKoukuSaydonLogicRuntime::Is_PointInsideRegion(edgeFan, boss, 7.f, 8.f),
        "Bomb point containment includes the fan apex and both radial edges");
    update(birth); update(birth + 30u);
    tests.Require(room->m_TickDamageEvents.empty(), "Being inside during tracking cannot resolve a bomb early");
    room->m_Players.at(1u).fPositionX = 4.f;
    update(birth + 59u); update(birth + 60u);
    tests.Require(countVisual("showtime.bomb.body") == 0 && countVisual("showtime.bomb.explosion") == 1 &&
        room->m_TickDamageEvents.size() == 4u && room->m_Players.at(5u).iCurrentHp == 100u &&
        std::all_of(room->m_KoukuRaid.PlayerIds.begin(), room->m_KoukuRaid.PlayerIds.end(), [&](auto id) {
            const auto& player = room->m_Players.at(id); return !player.iCurrentHp && !player.iShield &&
                !player.iInvulnerableEndTick && player.eAction == PLAYER_ACTION_STATE::DEAD; }),
        "A final miss creates one original explosion and wipes every raid participant through shields and invulnerability, preserving nonparticipants");
    update(birth + 60u); update(birth + 61u);
    // Run the actual raid update after the wipe. Death alone must not abort the
    // owning boss or discard the already committed finite explosion.
    auto& raid = room->m_KoukuRaid;
    raid.pCatalog = room->m_GameplayCatalog.Get_ActiveGeneration();
    raid.State.strGateId = "GATE3"; raid.State.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
    raid.iOwnerSessionId = 501u; room->m_PlayerIdBySessionId[501u] = 1u;
    raid.iPrimaryBossId = boss.iNetEntityId; raid.bEntryRunning = true;
    room->m_KoukuSaydonPatternAudition.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
    room->Update_KoukuRaid(birth + 60u); room->Update_KoukuRaid(birth + 61u);
    tests.Require(room->Is_KoukuRaidRunning() && countVisual("showtime.bomb.explosion") == 1,
        "The actual same-tick and next-tick raid updates preserve the explosion after a party wipe");
    tests.Require(countVisual("showtime.bomb.explosion") == 1 && room->m_TickDamageEvents.size() == 4u,
        "A resolved miss cannot repeat its explosion or encounter wipe");
    auto& member = room->m_KoukuSaydonPatternAudition.Members.emplace_back();
    member.strMemberId = "showtime.member"; member.iBossEntityId = boss.iNetEntityId;
    member.iPatternSequence = boss.iPatternSequence; member.PatternIds = {pattern.strPatternId}; member.LogicLedger = ledger;
    room->Clear_KoukuSaydonPatternAudition(true);
    tests.Require(countVisual("showtime.bomb.explosion") == 1, "Natural completion retains the failed bomb's finite explosion tail");
    room->m_CombatObjectRuntime.Update(room->m_Players, room->m_WorldEntities, room->m_GameplayCatalog, 3.f, birth + 151u, room->m_TickDamageEvents);
    tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "The failure explosion expires normally after the players have died");

    begin(1u, false); update(birth + 59u); // Late first evaluation keeps the authored body birth.
    room->m_CombatObjectRuntime.Build_LiveSpawnMessages(birth + 59u, restored);
    tests.Require(restored.size() == 1u && restored.front().iSpawnTick == birth, "Late body evaluation preserves the original authored age");
    boss.fYawDegrees = 180.f; update(birth + 60u);
    tests.Require(room->m_TickDamageEvents.size() == 5u && std::all_of(room->m_Players.begin(), room->m_Players.end(),
        [](const auto& entry) { return entry.second.iCurrentHp == 0u; }), "Standalone pattern Play wipes the whole living room on a final miss");

    for (const bool completed : {false, true})
    {
        begin(1u, true); update(birth);
        auto& current = room->m_KoukuSaydonPatternAudition.Members.emplace_back();
        current.strMemberId = "showtime.member"; current.iBossEntityId = boss.iNetEntityId;
        current.iPatternSequence = boss.iPatternSequence; current.PatternIds = {pattern.strPatternId}; current.LogicLedger = ledger;
        room->Clear_KoukuSaydonPatternAudition(completed);
        tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && room->m_TickDamageEvents.empty(),
            completed ? "Natural cleanup removes an unresolved body without inventing a verdict" :
                "Explicit Stop removes body plus fuse without spawning an explosion or wiping players");
    }
    begin(1u, true); update(birth);
    auto stale = ledger; stale.iPatternSequence -= 1u;
    room->Clear_KoukuPlayerTargets(boss, stale);
    tests.Require(countVisual("showtime.bomb.body") == 1, "A stale pattern owner cannot cancel another occurrence's bomb");
    room->Clear_KoukuPlayerTargets(boss, ledger);
    tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(), "The exact retained ledger owner cancels its bomb during tail cleanup");
    std::cout << "Showtime bomb failures : " << tests.failures << std::endl;
    return tests.failures ? 1 : 0;
}
```

### G08-04. 적용과 검증

1. 현재 Product 빌드 중에는 원본을 수정하지 않는다. 부모 작업이 정한 다음 적용 시점에 원본 SHA를 재확인하고 out 원본 백업과 후보를 CAS 방식으로 원자 교체한다. 같은 파일의 외부 변경이 있으면 최신 저장본에 수정 블록만 병합한다.
2. source·candidate diff, UTF-8/CRLF, 독립 리뷰, PLAN 전체 코드 일치 및 `git diff --check`를 확인한다. 공통 제품 함수와 데이터가 바뀌지 않았는지 확인한다.
3. 변경 뒤 Release·Debug Server를 다시 빌드하고 최종 Product receipt 및 source-stability에 새 SHA를 연결한다. 새 EXE에서 support를 격리 실행하며 최소 시작 free commit3GiB, 중단1.5GiB, 자기 private3GiB, 시간1200초 제한을 유지한다. 빌드와 다른 큰 native suite를 함께 실행하지 않는다.
4. 통과 수·실패 수·정상 종료 footer·EXE/DataFiles SHA를 기록한다. 중단된 이전322 PASS·12 FAILURE를 완료 suite로 다시 세지 않는다. 후보가 통과하기 전에는 검증 완료로 표시하지 않는다.
5. 이 검사는 Server 수치·소유권·정리 계약이며 Client의 실제 컷신·음향·HUD·사용자 플레이 전체를 관찰한 결과가 아니다. G05의 사용자 화면 확인 경계를 유지한다.
