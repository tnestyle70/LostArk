# 공식 LostArk 밸런스 반영 · 쿨타임 UI 연결 · 데미지 폰트 계획

> 2026-08-05 구현 갱신: field receipt, F1 Balance Tool, damage-event ViewModel consumer와 defense 적용
> 결과는 `2026-08-05_BALANCE_PROVENANCE_IMGUI_MAP_DATA_RESULT.md`를 따른다. 제품 world-space damage
> font renderer와 최종 radial HUD renderer는 이 PLAN의 미완 항목으로 남는다.

작성일 2026-08-05 · 브랜치 `codex/effect-tool-reboot` · 구현 전 계획서

이 문서는 원작 LostArk 클라이언트 데이터에서 5개 class와 발탄의 수치를 실측해 이 프로젝트의
`Data/Balance` 계약으로 옮기고, UI 담당자가 쿨타임을 돌릴 수 있는 읽기 경계를 만들고, 서버가
실제로 계산한 데미지로 데미지 폰트를 띄우기까지를 G 단위로 나눈다.

저장소 코드와 데이터는 이 문서 외에 아무것도 바뀌지 않은 상태다. G00의 측정만 완료이고
G01 이후는 전부 미구현이며, 측정 산출물은 세션 scratchpad에만 있다.

단, G01.7의 publisher 편집과 G01.3~G01.6b의 JSON은 **한 번 실제로 적용해 `-Mode Validate`가
통과하는 것을 확인한 뒤 되돌렸다.** 그래서 그 블록들은 이론적 제안이 아니라 실행된 코드이고,
아래 검증 표에서 그 항목만 "확인됨"으로 표시한다. 되돌린 뒤 `git status`로 4개 밸런스 문서와
publisher, Server/Shared 파일이 전부 원상태임을 확인했다.

---

## 0. 측정 요약과 확정된 사실

### 0.1 원본 데이터 확보 경로

원작 수치는 `EFGame/data2.lpk` 안의 SQLite 테이블 796개에 있다. 컨테이너 해독은 이미 저장소에
있는 도구로 끝난다.

```text
추출기   C:/Users/user/Desktop/LostArk/.codex_tmp/LpkBatchProbe/bin/Release/net8.0/LpkBatchProbe.exe
라이브러리 C:/Users/user/Desktop/Final_LostArk/_work/third_party/lostark-explorer-v1.1.1/
           src/LostArk.Archive.Core/bin/Release/net8.0/LostArk.Archive.Core.dll
명령      LpkBatchProbe.exe extract "C:/ProgramData/Smilegate/Games/LOSTARK/EFGame" <출력루트> "data2.lpk"
결과      796 entries / 852,505,600 bytes / failures=0
```

원본 payload는 Smilegate 저작물이므로 Git에 넣지 않는다. Git에 남기는 것은 이 계획서와
`Data/Balance` JSON뿐이다.

### 0.2 확정한 join

```text
EFTable_PC.PrimaryKey                     = class
   LANCE_MASTER 305 / GUNSLINGER 512 / SLAYER 112 / ARTIST 602 / DIMENSIONMASTER 612

EFTable_Skill.(PrimaryKey, SecondaryKey)  = (skillId, skill level 1..20)
   Cooltime, CostMp, MinRange, MaxRange, LearnClass, Grade, Book

EFTable_SkillEffect.PrimaryKey            = skillId * 10 + effectIndex
EFTable_SkillEffect.SecondaryKey          = skill level
   Key = 1 물리 damage, Key = 2 마법 damage
   ValueA / ValueB = 해당 레벨의 최소 / 최대 damage rate (공격력의 %)
   ValueF          = rate와 독립인 고정 damage 가산값
   ShowDamageFont / GlobalShowDamageFont / DmgFontScale = 데미지 폰트 계약

EFTable_Npc.PrimaryKey 480007             = 발탄
EFTable_NpcBalance.PrimaryKey 480007      = BalanceLevel 1415, StatScaleKey NormalCommander_S2_BOSS_1
EFTable_NpcStat.(BalanceLevel, StatScaleKey) = 절대 Hp / SkillDamage / Def
```

`skillId * 10` 규칙은 세 갈래로 교차 확인했다. `Skill.InstanceSkillEffectId` 값이 문자 그대로
`skillId*10+n`이고, `SkillFeature`가 참조하는 effect id가 같은 블록에 들어가며,
`SkillEffect.SecondaryKey`가 `Skill.SecondaryKey`와 같은 레벨 축이다.

rate의 단위 기준점은 `EFTable_PC(305).WeaponAttackSkillId = 34010`이다. 이 평타의 다섯 행이
전부 `ValueA=100`이므로 **rate 100 = 공격력 1.00배**다.

### 0.3 확정한 발탄 수치

| 항목 | 공식 값 | 출처 |
|---|---|---|
| NPC PrimaryKey | 480007 | `EFTable_Npc`, Model `EFDLCHAR_MN_RPBF_01.MN_RPBF_01` |
| BalanceLevel | 1415 | `EFTable_NpcBalance` (발탄 노말 아이템레벨과 일치) |
| StatScaleKey | `NormalCommander_S2_BOSS_1` | `EFTable_NpcBalance` |
| 기준 Hp | 136,015,677 | `EFTable_NpcStat(1415, NormalCommander_S2_BOSS_1)` |
| Hp 배율 / 바 개수 | 545% / 160 | `EFTable_NpcBalance.Hp`, `.Hp_Count` |
| SkillDamage | 27,204 | `EFTable_NpcStat` |
| AttackPower 배율 | 84% | `EFTable_NpcBalance` |
| Def | 7,401 | `EFTable_NpcStat` |
| 무력화 게이지 | 40,000 | `EFTable_NpcBalance.ParalyzationPointMax` |
| 무력화 지속 | 4500~5500 ms | `ParalyzationTimeMin/Max` |
| 카운터 경직 | 500 ms | `CounterFreezeTime` |
| WalkSpeed / MoveSpeed | 100 / 260 | `EFTable_Npc` |
| PursuitRange / WarRange / SightRange1 | 2000 / 2200 / 4000 | `EFTable_Npc` |
| ModelSize / MoveCollisonHeight | 140% / 280 | `EFTable_Npc` |
| 데미지 폰트 앵커 | `DamageFontPosZRate 5000`, `DmgFontSizeRate 100` | `EFTable_Npc` |

### 0.4 원작 데이터에 **없는** 것

정직하게 기록한다. 아래는 원작 서버에만 있고 배포된 클라이언트 테이블에는 없다.

1. **플레이어의 절대 HP·공격력·마나 총량.** 공식 HP 공식은
   `MaxHp = BaseMaxHp(class, level) + Con × PC.MaxHpCon`인데, `BaseMaxHp`와 `BaseCon`이 사는
   `EFTable_PCStat`이 컬럼 `SourceRow`, `Milestone` 둘만 남기고 **payload가 제거된 채 배포**된다
   (`EFTable_PCZoneStatCoefficient`도 같은 방식으로 제거됨). 공격력도
   `AP = ApStr·Str + ApAgi·Agi + ApInt·Int`인데 Str/Agi/Int가 장비에서 오므로 같은 이유로 복원
   불가다. 마나 총량은 어느 테이블에도 없다(794개 DB 컬럼 전수 검색).
   `EFTable_PC`가 실제로 주는 것은 `MaxHpCon`(2.0~2.2), `DefCoefficient`(0.90~1.10),
   `DamageCoefficient`(41개 class 전부 1.0), `MoveSpeed`(전 class 295)라는 **계수**뿐이다.
2. **rate → 절대 damage 변환식.** 공격력 테이블이 없다. 그래서 이 프로젝트가 공격력을 정한다.
3. **한 번의 시전이 effect 블록 중 어느 행을 쏘는지.** 블록에는 기본 히트와 트라이포드가
   교체·추가하는 히트가 섞여 있고, 발동 순서는 `.loa` animation notify에 있다.
   따라서 "이 스킬의 총 damage"는 이 테이블만으로 확정할 수 없다.
4. **한글 스킬 이름.** `Skill.Name`은 `tip.name.skill_34120` 같은 키이고 문자열 테이블은 이
   덤프에 없다. 다만 `SkillFeature.Name`에는 실제 한글 트라이포드 이름이 UTF-8로 들어 있고,
   기존 `Data/Animation/Reference/*/*.skilltiming`에 이미 한글 스킬 이름이 있다.

### 0.5 이 프로젝트가 정하는 값과 그 이유

| 값 | 선택 | 이유 |
|---|---|---|
| 기준 스킬 레벨 | 10 | 스킬 포인트만으로 도달하는 최고 레벨. `ValueA/ValueB`는 10에서 고정된다. |
| 대표 damage 행 | 블록 index 0 | 16개 스킬 전부에서 index 0이 기본 히트다. 34010은 다섯 index가 콤보 5단이고 전부 100으로 같다. |
| roll | `ValueA`(최소) | 서버가 결정적이어야 harness가 재현 가능하다. 난수 roll은 별도 슬라이스. |
| 공격력 | 100 | `DamageCoefficient`가 전 class 1.0이라 공식 데이터상 class별 차이가 없다. 100이면 rate가 곧 damage라 읽기 쉽다(평타 100 → 100). |
| 기준 Con | 2500 | 공식 공식의 `Con` 자리에 넣는 값. `maximumHp = 2500 × MaxHpCon`이 되어 class 비율은 공식 그대로이고 절대 규모만 기존 5000대에 맞춘다(`BaseMaxHp`는 0으로 둔다). |
| 기준 방어수치 | 100 | `defense = 100 × DefCoefficient`. 같은 이유로 비율만 공식이다. |
| 마나 총량 / 재생 | 1000 / 초당 25 | 레벨 10 공식 `CostMp`가 276~695라 100 풀로는 지불 불가. 1000·25/s면 Q(276·10s쿨)를 쿨마다 한 번 쓸 수 있다. |
| 발탄 HP | 375 × 160 = 60,000 | 공식 바 개수 160을 유지하고 바당 HP만 축소. 창술사 1로테이션(공식 rate 합 ≈ 8,000)이 약 13%를 깎는다. |
| 발탄 충돌 반경 | 3.0 | 공식 사거리는 대상 **표면**까지인데 서버는 중심간 거리를 잰다. `MoveCollison`이 0이라 모델 크기(140%)에 맞춰 정한다. |
| 초각성기 rate 상한 | 각성기 rate × 3 | 34620의 공식 rate 648,936은 34600(5,034)의 129배다. 60,000 HP 보스를 한 방에 지우므로 상한을 건다. 원본 값은 이 문서에 남긴다. |

`moveSpeed`는 공식 값 그대로 간다. 플레이어 295 → **2.95**, 발탄 260 → **2.6**. 현재 값은
각각 6.0과 3.0이므로 **체감 이동 속도가 절반 이하로 떨어진다.** 이것은 데이터 한 줄 되돌리면
끝나는 변경이지만, 적용 후 가장 먼저 눈에 띌 차이이므로 G01 검증 항목에 넣는다.

### 0.6 검토된 대안 기준선

독립 측정이 제안한 다른 배율이 있다. 둘 다 class 비율은 동일하게 공식이고 절대 규모만 다르다.

| | 이 계획 | 대안 |
|---|---|---|
| 기준 Con | 2500 → HP 5000~5500 | 20000 → HP 40000~44000 (원작 50~60렙 실제 대역) |
| 기준 방어수치 | 100 → 90~110 | 1000 → 900~1100 |
| 공격력 | 100 (rate가 곧 damage) | 1000 |
| `CostMp` 기준 레벨 | 10 (276~695) | 1 (53~88) |

이 계획이 2500·레벨 10을 택한 이유는 두 가지다. 첫째, 기존 발탄 HP·contract test 기대값과 같은
규모라 이번 변경이 밸런스 재설계가 아니라 수치 출처 교체로 끝난다. 둘째, damage rate를 레벨 10에서
읽고 있으므로 `CostMp`도 같은 레벨이어야 한 스킬의 비용과 위력이 같은 시점을 가리킨다.

대안이 나은 경우도 명확하다. 나중에 장비·트라이포드·각인처럼 절대 수치가 커지는 축을 붙일
계획이면 40000대에서 시작하는 편이 재조정이 없다.

### 0.7 이번 범위에 넣지 않은 공식 데이터

측정은 끝났지만 소비자가 없어 데이터로 올리지 않는 것들이다. 나중에 필요할 때 이 문서를 근거로
쓴다.

| 항목 | 공식 값 | 왜 지금 안 넣는가 |
|---|---|---|
| class별 대시 쿨다운 | 창술사 34020/6000ms, 슬레이어 45010/7000ms, 도화가 31020/7000ms, 건슬링어 38150/8000ms, 차원술사 2050020/8000ms | 이동기가 아직 어느 `skillKind`에도 없다 |
| 치명타 | `critRate = CritStat / PCLevel.CriticalHitCoefficient`, 50렙 이상 100 Crit당 3.579% | 치명타 판정 자체가 서버에 없다 |
| 이동/공격 속도 상하한 | 이동 −80%~+40%, 공격 −40%~+40% (`PCStatMinMax` stat 80/78) | 버프·디버프 계약이 없다 |
| 기상 무적 | 1500ms + 기상기 500ms (`CombatAdjustment`) | 넉백·다운 상태가 없다 |
| 발탄 무력화 | 게이지 40000, 지속 4500~5500ms, 카운터 경직 500ms | 무력화 시스템이 없다 |
| damage 속성 | `ValueE` 1~7 (화염/얼음/전기/?/대지/독/빛) — 16개 중 슬레이어 `450603`만 1(화염) | 속성 저항 계약이 없다 |
| `ValueF` 고정 damage 가산 | 34120 레벨10에서 21990 | 절대 스탯 규모가 없어 더할 기준이 없다(0.4 참고) |

---

## G00 — 공식 데이터 추출 (측정 완료, 저장소 변경 없음)

### G00.1 목표와 종료 증거

5개 class와 발탄의 공식 수치를 재현 가능한 스크립트로 뽑아 하나의 receipt JSON에 남긴다.
종료 증거는 receipt의 16개 스킬 rate가 독립 측정과 일치하는 것이다.

### G00.2 파일

두 파일 모두 세션 scratchpad에 둔다. 저장소에는 넣지 않는다. 원본 DB 경로가 개인 PC의 게임
설치 경로이고, 산출물 JSON만이 팀이 공유할 계약이기 때문이다.

```text
<scratchpad>/extract_official_balance.py     측정: DB -> receipt JSON
<scratchpad>/map_official_to_project.py      정책: receipt -> Data/Balance 문서
```

측정과 정책을 나눈 이유는 검토자가 "이건 Smilegate 숫자, 이건 우리가 정한 숫자"를 파일 경계로
구분할 수 있게 하기 위해서다.

### G00.3 추출 결과

```text
LANCE_MASTER     skills=28
GUNSLINGER       skills=30
SLAYER           skills=26
ARTIST           skills=24
DIMENSIONMASTER  skills=24
BOSS_VALTAN      npc=480007 balanceLevel=1415 hpPerBar=741,285,440 bars=160
```

현재 배선된 16개 스킬의 기본 히트 rate(레벨 10, index 0):

| skillId | slot | Key | base effect | rate(min) | Cooltime | CostMp@10 | MaxRange | DmgFontScale |
|---|---|---|---|---|---|---|---|---|
| 34120 | Q | 1 | 341200 | 361 | 10000 | 276 | 800 | 0 |
| 34080 | W | 1 | 340800 | 261 | 12000 | 305 | 0 | 0 |
| 34070 | E | 1 | 340700 | 459 | 14000 | 334 | 0 | 0 |
| 34150 | R | 1 | 341500 | 3839 | 24000 | 456 | 0 | 0 |
| 34110 | A | 1 | 341100 | 499 | 18000 | 386 | 1000 | 0 |
| 34090 | S | 1 | 340900 | 1050 | 10000 | 276 | 0 | 0 |
| 34640 | T | 1 | 346400 | 1530 | 50000 | 695 | 0 | 0 |
| 34600 | V | 1 | 346001 | 5034 | 300000 | 0 | 900 | 0 |
| 34620 | ALT_V | 2 | 346200 | 648936 | 300000 | 0 | 600 | **150** |
| 34010 | LMB | 1 | 340100 | 100 | 0 | 0 | 0 | 0 |
| 38020 | Q | 1 | 380200 | 624 | 10000 | 276 | 0 | 0 |
| 38050 | W | 1 | 380501 | 801 | 30000 | 518 | 800 | 0 |
| 45050 | Q | 1 | 450500 | 249 | 12000 | 349 | 0 | 0 |
| 45060 | W | 1 | 450600 | 262 | 14000 | 382 | 0 | 0 |
| 31210 | Q | 2 | 312101 | 1301 | 16000 | 412 | 350 | 0 |
| 31230 | W | 2 | 312300 | 152 | 24000 | 521 | 0 | 0 |

34600·38050·31210은 index 0이 `Key=12`(비damage)라 블록에서 가장 낮은 damage index를 기본
히트로 잡았다.

**현재 `cooldownMs` 16개 값은 공식 `Cooltime`과 이미 100% 일치한다.** 바뀌는 것은
`resourceCost`, `maximumRange`, `hitTimeMs`, 그리고 damage 표현 방식이다.

### G00.4 검증

```powershell
python extract_official_balance.py <TableData> official_balance_receipt.json --book-only --reference-level 10
```

기대: 위 표의 rate 16개가 그대로 나온다. 실제로 독립 측정 에이전트가 같은 DB를 따로 조회해
얻은 표와 16개 전부 일치했다.

---

## G01 — `Data/Balance` 공식화와 계약 확장

### G01.1 목표와 종료 증거

공식 수치를 4개 문서에 반영하고, publisher·Server 파서·Client 파서가 새 스키마를 받아들이게
한다. 종료 증거는 `Publish-GameplayBalance.ps1 -Mode Validate` 성공, Server/Client Debug 빌드
성공, `Server.exe --contract-test` 성공이다.

### G01.2 수정 파일과 이유

| 파일 | 이 G에서 하는 일 |
|---|---|
| `Data/Balance/PlayerProfiles.json` | v2. `resourceRegenPerSecond`, `attackPower`, `defense` 추가. HP·이동속도 공식화 |
| `Data/Balance/PlayerSkills.json` | v2. `resourceCost`, `maximumRange`, `hitTimeMs` 공식화 |
| `Data/Balance/DamageProfiles.json` | v2. `amount`(고정 정수) → `damageRatePercent`(공격력 %) |
| `Data/Balance/BossProfiles.json` | v2. `attackPower`, `collisionRadius` 추가. HP·속도·추적거리 공식화 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | 위 4개 스키마 검증과 bootstrap 행 형식 갱신 |
| `Server/Public/GameplayCatalog.h` | 프로필 구조체 확장, `Find_Damage` → `Find_DamageRatePercent` + `Resolve_Damage` |
| `Server/Private/GameplayCatalog.cpp` | 새 bootstrap 행 파싱, rate 상한, cost를 최대 마나 풀 기준으로 검증 |
| `Server/Private/PlayerSkillSystem.cpp` | damage = 공격력 × rate / 100, 보스 충돌 반경 가산, 마나 재생을 데이터로 |
| `Server/Private/ValtanBrain.cpp` | 보스 damage도 같은 `Resolve_Damage` 경로 |
| `Server/Private/GameRoom.cpp` | spawn 시 새 프로필 필드 적용 |
| `Client/Public/PlayerSkillCatalog.h`, `.cpp` | `formatVersion 2`, `iResourceCost`, damage rate 파싱 |
| `Client/Private/CombatHUDViewModel.cpp` | 표시 damage를 공격력 × rate로 계산 |
| `Server/Private/ServerGameplayContractTests.cpp` | 하드코딩된 HP·damage 기대값 갱신 |

### G01.3 `Data/Balance/PlayerProfiles.json` 전문

```json
{
  "schema": "lostark.player-profiles",
  "formatVersion": 2,
  "players": [
    {
      "characterClass": "LANCE_MASTER",
      "maximumHp": 5500,
      "maximumResource": 1000,
      "resourceRegenPerSecond": 25,
      "attackPower": 100,
      "defense": 105,
      "moveSpeed": 2.95
    },
    {
      "characterClass": "GUNSLINGER",
      "maximumHp": 5000,
      "maximumResource": 1000,
      "resourceRegenPerSecond": 25,
      "attackPower": 100,
      "defense": 95,
      "moveSpeed": 2.95
    },
    {
      "characterClass": "SLAYER",
      "maximumHp": 5500,
      "maximumResource": 1000,
      "resourceRegenPerSecond": 25,
      "attackPower": 100,
      "defense": 110,
      "moveSpeed": 2.95
    },
    {
      "characterClass": "ARTIST",
      "maximumHp": 5000,
      "maximumResource": 1000,
      "resourceRegenPerSecond": 25,
      "attackPower": 100,
      "defense": 90,
      "moveSpeed": 2.95
    },
    {
      "characterClass": "DIMENSIONMASTER",
      "maximumHp": 5250,
      "maximumResource": 1000,
      "resourceRegenPerSecond": 25,
      "attackPower": 100,
      "defense": 100,
      "moveSpeed": 2.95
    }
  ]
}
```

`maximumHp`는 `2500 × MaxHpCon`이다. 창술사·슬레이어 2.2 → 5500, 건슬링어·도화가 2.0 → 5000,
차원술사 2.1 → 5250. `defense`는 `100 × DefCoefficient`다.

### G01.4 `Data/Balance/DamageProfiles.json` 전문

```json
{
  "schema": "lostark.damage-profiles",
  "formatVersion": 2,
  "profiles": [
    { "damageProfileId": "damage.player.31210", "damageRatePercent": 1301 },
    { "damageProfileId": "damage.player.31230", "damageRatePercent": 152 },
    { "damageProfileId": "damage.player.34010", "damageRatePercent": 100 },
    { "damageProfileId": "damage.player.34070", "damageRatePercent": 459 },
    { "damageProfileId": "damage.player.34080", "damageRatePercent": 261 },
    { "damageProfileId": "damage.player.34090", "damageRatePercent": 1050 },
    { "damageProfileId": "damage.player.34110", "damageRatePercent": 499 },
    { "damageProfileId": "damage.player.34120", "damageRatePercent": 361 },
    { "damageProfileId": "damage.player.34150", "damageRatePercent": 3839 },
    { "damageProfileId": "damage.player.34600", "damageRatePercent": 5034 },
    { "damageProfileId": "damage.player.34620", "damageRatePercent": 15102 },
    { "damageProfileId": "damage.player.34640", "damageRatePercent": 1530 },
    { "damageProfileId": "damage.player.38020", "damageRatePercent": 624 },
    { "damageProfileId": "damage.player.38050", "damageRatePercent": 801 },
    { "damageProfileId": "damage.player.45050", "damageRatePercent": 249 },
    { "damageProfileId": "damage.player.45060", "damageRatePercent": 262 },
    { "damageProfileId": "damage.valtan.basic-swing", "damageRatePercent": 350 }
  ]
}
```

`damage.player.34620`의 15102는 공식 648936에 상한을 건 값이다(각성기 5034 × 3). 원본 값은
G00.3 표에 남아 있다. `damage.valtan.basic-swing`의 350은 기존 고정값을 rate로 그대로 옮긴
것이고, 발탄 공식 패턴 damage는 `MN_RPBF_01-1.loa`를 파싱해야 나오므로 G05로 미룬다.

### G01.5 `Data/Balance/BossProfiles.json` 전문

```json
{
  "schema": "lostark.boss-profiles",
  "formatVersion": 2,
  "bosses": [
    {
      "archetypeId": "BOSS_VALTAN",
      "encounterId": "ENCOUNTER_VALTAN",
      "displayName": "발탄",
      "maximumHp": 60000,
      "attackPower": 100,
      "collisionRadius": 3.0,
      "engageDistance": 20.0,
      "moveSpeed": 2.6,
      "phaseTwoHpPercent": 50
    }
  ]
}
```

`engageDistance` 20.0은 공식 `PursuitRange 2000`cm, `moveSpeed` 2.6은 공식 `MoveSpeed 260`이다.

### G01.6 `Data/Balance/PlayerSkills.json` 변경 행

스키마는 `formatVersion`만 2로 오르고 필드 집합은 그대로다. 값만 공식화한다.

| skillId | slot | cooldownMs | resourceCost | maximumRange | hitTimeMs |
|---|---|---|---|---|---|
| 34120 | Q | 10000 (유지) | 15 → **276** | 7.3 → **8.0** | 1295 → **1510** |
| 34080 | W | 12000 (유지) | 17 → **305** | 4.0 → **1.2** | 0 (유지) |
| 34070 | E | 14000 (유지) | 19 → **334** | 9.9 → **3.8** | 300 (유지) |
| 34150 | R | 24000 (유지) | 29 → **456** | 9.2 → **2.8** | 1345 (유지) |
| 34110 | A | 18000 (유지) | 23 → **386** | 9.9 → **10.0** | 1857 (유지) |
| 34090 | S | 10000 (유지) | 15 → **276** | 9.2 → **2.8** | 1730 (유지) |
| 34640 | T | 50000 (유지) | 0 → **695** | 10.6 → **2.6** | 1730 (유지) |
| 34600 | V | 300000 (유지) | 0 (유지) | 12.2 → **9.0** | 1390 (유지) |
| 34620 | ALT_V | 300000 (유지) | 0 (유지) | 18.2 → **6.0** | 1930 (유지) |
| 34010 | LMB | 0 (유지) | 0 (유지) | 3.5 → **1.8** | 470 (유지) |
| 38020 | Q | 10000 (유지) | 15 → **276** | 5.8 → **5.0** | 1000 (유지) |
| 38050 | W | 30000 (유지) | 25 → **518** | 4.0 → **8.0** | 1000 (유지) |
| 45050 | Q | 12000 (유지) | 15 → **349** | 3.5 → **1.8** | 1100 (유지) |
| 45060 | W | 14000 (유지) | 18 → **382** | 5.0 → **2.5** | 500 (유지) |
| 31210 | Q | 16000 (유지) | 18 → **412** | 2.5 → **3.5** | 1445 (유지) |
| 31230 | W | 24000 (유지) | 22 → **521** | 6.5 → **1.8** | 1445 (유지) |

### G01.6b `Data/Balance/PlayerSkills.json` 전문

```json
{
  "schema": "lostark.player-skills",
  "formatVersion": 2,
  "skills": [
    {
      "skillId": 34120,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "Q",
      "displayName": "연환섬",
      "actionId": "lancemaster.skill.34120",
      "skillKind": "ACTIVE",
      "cooldownMs": 10000,
      "actionDurationMs": 2266,
      "hitTimeMs": 1510,
      "resourceCost": 276,
      "movementDistance": 0.0,
      "maximumRange": 8.0,
      "serverDamageProfileId": "damage.player.34120",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34080,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "W",
      "displayName": "일섬각",
      "actionId": "lancemaster.skill.34080",
      "skillKind": "ACTIVE",
      "cooldownMs": 12000,
      "actionDurationMs": 1366,
      "hitTimeMs": 0,
      "resourceCost": 305,
      "movementDistance": 0.0,
      "maximumRange": 1.2,
      "serverDamageProfileId": "damage.player.34080",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34070,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "E",
      "displayName": "회선창",
      "actionId": "lancemaster.skill.34070",
      "skillKind": "ACTIVE",
      "cooldownMs": 14000,
      "actionDurationMs": 2000,
      "hitTimeMs": 300,
      "resourceCost": 334,
      "movementDistance": 0.0,
      "maximumRange": 3.8,
      "serverDamageProfileId": "damage.player.34070",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34150,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "R",
      "displayName": "맹룡열파",
      "actionId": "lancemaster.skill.34150",
      "skillKind": "ACTIVE",
      "cooldownMs": 24000,
      "actionDurationMs": 2266,
      "hitTimeMs": 1345,
      "resourceCost": 456,
      "movementDistance": 0.0,
      "maximumRange": 2.8,
      "serverDamageProfileId": "damage.player.34150",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34110,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "A",
      "displayName": "반월섬",
      "actionId": "lancemaster.skill.34110",
      "skillKind": "ACTIVE",
      "cooldownMs": 18000,
      "actionDurationMs": 3200,
      "hitTimeMs": 1857,
      "resourceCost": 386,
      "movementDistance": 0.0,
      "maximumRange": 10.0,
      "serverDamageProfileId": "damage.player.34110",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34090,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "S",
      "displayName": "철량추",
      "actionId": "lancemaster.skill.34090",
      "skillKind": "ACTIVE",
      "cooldownMs": 10000,
      "actionDurationMs": 2100,
      "hitTimeMs": 1730,
      "resourceCost": 276,
      "movementDistance": 0.0,
      "maximumRange": 2.8,
      "serverDamageProfileId": "damage.player.34090",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34640,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "T",
      "displayName": "맹룡난무",
      "actionId": "lancemaster.skill.34640",
      "skillKind": "ACTIVE",
      "cooldownMs": 50000,
      "actionDurationMs": 3166,
      "hitTimeMs": 1730,
      "resourceCost": 695,
      "movementDistance": 0.0,
      "maximumRange": 2.6,
      "serverDamageProfileId": "damage.player.34640",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34600,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "V",
      "displayName": "은하유성탄",
      "actionId": "lancemaster.skill.34600",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 5600,
      "hitTimeMs": 1390,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 9.0,
      "serverDamageProfileId": "damage.player.34600",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34620,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "ALT_V",
      "displayName": "은하비섬창",
      "actionId": "lancemaster.skill.34620",
      "skillKind": "ACTIVE",
      "cooldownMs": 300000,
      "actionDurationMs": 6300,
      "hitTimeMs": 1930,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 6.0,
      "serverDamageProfileId": "damage.player.34620",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38020,
      "characterClass": "GUNSLINGER",
      "inputSlot": "Q",
      "displayName": "퀵 스텝",
      "actionId": "gunslinger.skill.38020",
      "skillKind": "ACTIVE",
      "cooldownMs": 10000,
      "actionDurationMs": 3533,
      "hitTimeMs": 1000,
      "resourceCost": 276,
      "movementDistance": 0.0,
      "maximumRange": 5.0,
      "serverDamageProfileId": "damage.player.38020",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 38050,
      "characterClass": "GUNSLINGER",
      "inputSlot": "W",
      "displayName": "심판의 시간",
      "actionId": "gunslinger.skill.38050",
      "skillKind": "ACTIVE",
      "cooldownMs": 30000,
      "actionDurationMs": 1767,
      "hitTimeMs": 1000,
      "resourceCost": 518,
      "movementDistance": 0.0,
      "maximumRange": 8.0,
      "serverDamageProfileId": "damage.player.38050",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45050,
      "characterClass": "SLAYER",
      "inputSlot": "Q",
      "displayName": "퓨리 블레이드",
      "actionId": "slayer.skill.45050",
      "skillKind": "ACTIVE",
      "cooldownMs": 12000,
      "actionDurationMs": 4533,
      "hitTimeMs": 1100,
      "resourceCost": 349,
      "movementDistance": 0.0,
      "maximumRange": 1.8,
      "serverDamageProfileId": "damage.player.45050",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 45060,
      "characterClass": "SLAYER",
      "inputSlot": "W",
      "displayName": "와일드 러시",
      "actionId": "slayer.skill.45060",
      "skillKind": "ACTIVE",
      "cooldownMs": 14000,
      "actionDurationMs": 4600,
      "hitTimeMs": 500,
      "resourceCost": 382,
      "movementDistance": 0.0,
      "maximumRange": 2.5,
      "serverDamageProfileId": "damage.player.45060",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31210,
      "characterClass": "ARTIST",
      "inputSlot": "Q",
      "displayName": "필법 : 콩콩이",
      "actionId": "artist.skill.31210",
      "skillKind": "ACTIVE",
      "cooldownMs": 16000,
      "actionDurationMs": 5334,
      "hitTimeMs": 1445,
      "resourceCost": 412,
      "movementDistance": 0.0,
      "maximumRange": 3.5,
      "serverDamageProfileId": "damage.player.31210",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 31230,
      "characterClass": "ARTIST",
      "inputSlot": "W",
      "displayName": "묵법 : 옹달샘",
      "actionId": "artist.skill.31230",
      "skillKind": "ACTIVE",
      "cooldownMs": 24000,
      "actionDurationMs": 1500,
      "hitTimeMs": 1445,
      "resourceCost": 521,
      "movementDistance": 0.0,
      "maximumRange": 1.8,
      "serverDamageProfileId": "damage.player.31230",
      "effectId": "",
      "comboStages": []
    },
    {
      "skillId": 34010,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "LMB",
      "displayName": "긴 창_평타",
      "actionId": "lancemaster.skill.34010",
      "skillKind": "COMBO",
      "cooldownMs": 0,
      "actionDurationMs": 1633,
      "hitTimeMs": 470,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 1.8,
      "serverDamageProfileId": "damage.player.34010",
      "effectId": "",
      "comboStages": [
        {
          "actionDurationMs": 1633,
          "hitTimeMs": 470,
          "inputOpenMs": 329,
          "inputCloseMs": 658
        },
        {
          "actionDurationMs": 1367,
          "hitTimeMs": 356,
          "inputOpenMs": 330,
          "inputCloseMs": 554
        },
        {
          "actionDurationMs": 1533,
          "hitTimeMs": 451,
          "inputOpenMs": 396,
          "inputCloseMs": 752
        },
        {
          "actionDurationMs": 1567,
          "hitTimeMs": 500,
          "inputOpenMs": 0,
          "inputCloseMs": 0
        }
      ]
    }
  ]
}
```

`maximumRange`는 `max(공식 MaxRange, 기본 히트 AreaRange) / 100`이다. 근접 스킬이 1.2~2.8로
작아 보이지만 서버 판정은 여기에 보스 `collisionRadius` 3.0을 더하므로 실효 사거리는
4.2~5.8이 되어 기존 손튜닝 값과 비슷하다. 이 가산은 G01.9에서 넣는다.

`hitTimeMs`는 기본 히트가 `HitTypeTimeMin`을 가질 때만 그 값으로 바꾼다. 34120만 해당하고
(1295는 `Key=38` 반응 전용 행의 시각이었고 실제 damage 행은 1510) 나머지는 기존 `.skilltiming`
값을 유지한다.

### G01.7 `Publish-GameplayBalance.ps1` 교체 블록

damage 문서 블록을 통째로 교체한다. 삽입 위치는 현재 37~51행이다.

```powershell
# A damage profile stores the official Lost Ark damage rate as a percentage of
# the caster's attack power, not a finished number: EFTable_SkillEffect only ever
# supplies the rate, and the caster's attack power is what turns it into damage.
# The 34010 basic attack is rate 100, so 100 is exactly one attack power.
$maximumDamageRatePercent = 100000
$damageDocument = Read-JsonDocument 'Data/Balance/DamageProfiles.json'
Assert-ExactProperties $damageDocument @('schema','formatVersion','profiles') 'damage document'
if ($damageDocument.schema -ne 'lostark.damage-profiles' -or $damageDocument.formatVersion -ne 2) {
    throw 'Damage profile header is invalid.'
}
$damageIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$damageRows = [Collections.Generic.List[string]]::new()
foreach ($profile in @($damageDocument.profiles)) {
    Assert-ExactProperties $profile @('damageProfileId','damageRatePercent') 'damage profile'
    Assert-StableId $profile.damageProfileId 'damageProfileId'
    $ratePercent = [uint32]$profile.damageRatePercent
    if (-not $damageIds.Add([string]$profile.damageProfileId) -or $ratePercent -eq 0 -or
        $ratePercent -gt $maximumDamageRatePercent) {
        throw "Duplicate, zero, or out-of-range damage profile: $($profile.damageProfileId)"
    }
    $damageRows.Add("DAMAGE`t$($profile.damageProfileId)`t$ratePercent")
}
```

skill/player 문서 헤더의 `formatVersion -ne 1`을 `-ne 2`로 바꾼다(현재 55행, 61행).

player 프로필 루프를 교체한다. 삽입 위치는 현재 73~85행이다.

```powershell
foreach ($player in @($playerDocument.players)) {
	Assert-ExactProperties $player @(
		'characterClass','maximumHp','maximumResource','resourceRegenPerSecond',
		'attackPower','defense','moveSpeed') 'player profile'
	Assert-StableId $player.characterClass 'player characterClass'
	# attackPower is the multiplicand every damage rate is applied to, so a zero
	# would silently disarm the class rather than fail loudly at load.
	if ($player.characterClass -notin $supportedPlayerClasses -or
		-not $playerClasses.Add([string]$player.characterClass) -or
		[uint32]$player.maximumHp -eq 0 -or [uint32]$player.maximumResource -eq 0 -or
		[uint32]$player.attackPower -eq 0 -or [uint32]$player.defense -eq 0 -or
		[uint32]$player.resourceRegenPerSecond -eq 0 -or
		[uint32]$player.resourceRegenPerSecond -gt [uint32]$player.maximumResource) {
		throw "Player profile is invalid: $($player.characterClass)"
	}
	$playerRows.Add((@(
		'PLAYER', $player.characterClass, [uint32]$player.maximumHp,
		[uint32]$player.maximumResource, [uint32]$player.resourceRegenPerSecond,
		[uint32]$player.attackPower, [uint32]$player.defense,
		(Format-InvariantFloat $player.moveSpeed 'player moveSpeed')) -join "`t"))
}
```

class 완전성 검사 직후(현재 89행 뒤)에 최대 마나 풀을 구한다.

```powershell
# Official CostMp at skill level 10 runs into the hundreds, so a skill's cost is
# bounded by the largest pool any class actually has rather than by a literal.
$maximumPlayerResource = (@($playerDocument.players) |
	ForEach-Object { [uint32]$_.maximumResource } | Measure-Object -Maximum).Maximum
```

skill 검증의 `resourceCost -gt 100`(현재 123행)을 바꾼다.

```powershell
        [uint32]$skill.resourceCost -gt $maximumPlayerResource -or
        -not $damageIds.Contains([string]$skill.serverDamageProfileId)) {
```

boss 문서 블록을 교체한다. 삽입 위치는 현재 179~203행이다.

```powershell
if ($bossDocument.schema -ne 'lostark.boss-profiles' -or $bossDocument.formatVersion -ne 2) {
    throw 'Boss profile header is invalid.'
}
$bossIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$bossRows = [Collections.Generic.List[string]]::new()
foreach ($boss in @($bossDocument.bosses)) {
    Assert-ExactProperties $boss @(
		'archetypeId','encounterId','displayName','maximumHp','attackPower','collisionRadius',
		'engageDistance','moveSpeed','phaseTwoHpPercent') 'boss profile'
    Assert-StableId $boss.archetypeId 'boss archetypeId'
    Assert-StableId $boss.encounterId 'boss encounterId'
	if ([string]::IsNullOrWhiteSpace([string]$boss.displayName) -or ([string]$boss.displayName).Length -gt 64) {
		throw "Boss displayName is invalid: $($boss.archetypeId)"
	}
	# collisionRadius closes the gap between an official skill range, which is
	# measured to the edge of the target, and this server's centre-to-centre reach
	# test. A zero would put every melee skill out of range of its own boss.
    if (-not $bossIds.Add([string]$boss.archetypeId) -or [uint32]$boss.maximumHp -eq 0 -or
        [uint32]$boss.attackPower -eq 0 -or [double]$boss.collisionRadius -le 0.0 -or
        [uint32]$boss.phaseTwoHpPercent -eq 0 -or [uint32]$boss.phaseTwoHpPercent -ge 100) {
        throw "Boss profile is invalid: $($boss.archetypeId)"
    }
    $bossRows.Add((@(
        'BOSS', $boss.archetypeId, $boss.encounterId, [uint32]$boss.maximumHp,
        [uint32]$boss.attackPower,
        (Format-InvariantFloat $boss.collisionRadius 'boss collisionRadius'),
        (Format-InvariantFloat $boss.engageDistance 'boss engageDistance'),
        (Format-InvariantFloat $boss.moveSpeed 'boss moveSpeed'),
        [uint32]$boss.phaseTwoHpPercent) -join "`t"))
}
```

이 상태에서 `-Mode Validate`가 실제로 통과하는 것을 확인했다.

```text
Gameplay balance Validate succeeded: 5 player profiles, 20 skills, 17 damage profiles, 1 bosses.
```

### G01.8 `Server/Public/GameplayCatalog.h` 변경 계약

`BOSS_RUNTIME_PROFILE`에 두 멤버를 추가한다. `iAttackPower`는 이 보스가 쏘는 모든 rate의
피승수이고, `fCollisionRadius`는 공식 사거리와 서버의 중심간 판정 사이의 간격을 메운다.

`PLAYER_RUNTIME_PROFILE`에 `iResourceRegenPerSecond`, `iAttackPower`, `iDefense`를 추가한다.
마나 풀이 공식 `CostMp`에 맞춰 커졌으므로 재생 속도도 데이터여야 한다.

`Find_Damage` → `Find_DamageRatePercent`로 이름을 바꾼다. 반환값의 의미가 "완성된 damage"에서
"공격력의 %"로 바뀌었으므로 이름이 따라가야 한다. 그리고 rate가 숫자가 되는 유일한 지점으로
`static Resolve_Damage(attackPower, ratePercent)`를 추가한다. static인 이유는 catalog 상태를
읽지 않고 두 수만 쓰기 때문이고, 여기에 모으는 이유는 플레이어 스킬과 보스 패턴이 서로 다른
공식으로 갈라지지 않게 하기 위해서다.

멤버 `m_DamageByProfileId` → `m_DamageRatePercentByProfileId`.

```cpp
	struct BOSS_RUNTIME_PROFILE
	{
		std::string strArchetypeId;
		std::string strEncounterId;
		std::uint32_t iMaximumHp = 0;
		/* Multiplicand for every damage rate this boss casts. */
		std::uint32_t iAttackPower = 0;
		/* Added to a skill's reach because official ranges stop at the target's
		edge while the server measures centre to centre. */
		float fCollisionRadius = 0.f;
		float fEngageDistance = 0.f;
		float fMoveSpeed = 0.f;
		std::uint32_t iPhaseTwoHpPercent = 0;
	};

	struct PLAYER_RUNTIME_PROFILE
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumResource = 0;
		/* Resource restored per wall-clock second while not casting. The pool is
		sized to official CostMp, so the regen rate has to be data too. */
		std::uint32_t iResourceRegenPerSecond = 0;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		float fMoveSpeed = 0.f;
	};
```

```cpp
		/* Percent of the caster's attack power, straight from the official
		EFTable_SkillEffect rate. Zero means the profile is unknown. */
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;

		/* The one place a rate becomes a number, so player skills and boss
		patterns cannot drift apart. Always at least 1 for a known profile: a hit
		that connects should never read as a miss. */
		static std::uint32_t Resolve_Damage(
			std::uint32_t attackPower, std::uint32_t damageRatePercent);
```

### G01.9 `Server/Private/GameplayCatalog.cpp` 변경 함수

`Load()`가 새로 하는 일만 적는다.

- 익명 namespace 맨 위에 `MAXIMUM_DAMAGE_RATE_PERCENT = 100000u`를 둔다. publisher의
  `$maximumDamageRatePercent`와 같은 값이어야 한쪽만 통과하는 rate가 생기지 않는다.
- `DAMAGE` 행: `amount` → `ratePercent`, 상한 검사 추가, 맵 이름 변경.
- `SKILL` 행: `skill.iResourceCost > 100u` 조건을 **삭제**한다. 행 단위로는 어느 class의 풀과
  비교해야 하는지 알 수 없기 때문이다. 대신 모든 행을 읽은 뒤 최대 풀과 비교한다.
- `BOSS` 행: 7필드 → 9필드. `iAttackPower`, `fCollisionRadius` 파싱과 0 검사.
- `PLAYER` 행: 5필드 → 8필드. 재생·공격력·방어력 파싱과 0 검사, 재생이 풀을 넘지 않는지 검사.
- 마지막 검증 루프: `Find_Damage` → `Find_DamageRatePercent`, 그리고 최대 풀 대비 cost 검사 추가.

```cpp
	/* Rows arrive sorted, so a skill's cost cannot be checked against a pool while
	the skill row is being parsed. The largest pool any class has is the only bound
	that makes a cost payable by somebody. */
	std::uint32_t largestResourcePool = 0;
	for (const auto& [characterClass, player] : m_Players)
	{
		(void)characterClass;
		largestResourcePool =
			std::max(largestResourcePool, player.iMaximumResource);
	}
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		if (0u == Find_DamageRatePercent(skill.strDamageProfileId))
		{
			m_strStatus = "Player skill references missing damage profile";
			return false;
		}
		if (skill.iResourceCost > largestResourcePool)
		{
			m_strStatus = "Player skill costs more than any class can hold";
			return false;
		}
	}
```

`Resolve_Damage`는 새 함수다. 64비트로 곱해 `uint32` 오버플로를 막고, 0으로 내려가지 않게
최소 1을 보장한다. 명중한 타격이 0으로 표시되면 데미지 폰트가 "빗나감"처럼 읽히기 때문이다.

```cpp
std::uint32_t LostArk::Server::CGameplayCatalog::Resolve_Damage(
	const std::uint32_t attackPower,
	const std::uint32_t damageRatePercent)
{
	if (0u == attackPower || 0u == damageRatePercent)
		return 0u;
	const std::uint64_t scaled =
		(static_cast<std::uint64_t>(attackPower) *
			static_cast<std::uint64_t>(damageRatePercent)) / 100ull;
	return scaled < 1ull ? 1u :
		static_cast<std::uint32_t>(std::min<std::uint64_t>(
			scaled, std::numeric_limits<std::uint32_t>::max()));
}
```

`<limits>` include를 추가한다.

### G01.10 `Server/Private/PlayerSkillSystem.cpp` 변경 함수

`Update()`가 새로 하는 일만 적는다.

- 마나 재생: `if (0u == serverTick % 6u) ++player.iCurrentResource;`를 프로필의
  `iResourceRegenPerSecond` 기반으로 바꾼다. 30 Hz 고정 tick이므로 tick당 증가량은
  `(regen + 29) / 30`이 아니라 누적 오차 없이 `serverTick % (30 / regen)` 방식이 아니라,
  `player.iResourceAccumulator += regen; while (accumulator >= 30) { ++resource; accumulator -= 30; }`
  로 정수만 써서 초당 정확히 regen이 되게 한다. `SERVER_PLAYER`에 `iResourceAccumulator` 추가.
- 보스 탐색 반경: `skill->fMaximumRange` 대신
  `skill->fMaximumRange + bossProfile->fCollisionRadius`를 쓴다. 보스 프로필은 entity의
  `strArchetypeId`로 조회한다.
- damage: `catalog.Find_Damage(...)` 대신
  `CGameplayCatalog::Resolve_Damage(playerProfile->iAttackPower, catalog.Find_DamageRatePercent(...))`.

이 세 곳 외의 콤보·쿨다운·이동 로직은 건드리지 않는다.

### G01.11 `Client` 파서 변경

`CPlayerSkillCatalog::Load`는 `formatVersion` 2를 받고 `iResourceCost`를 채운다. 현재 Client는
`resourceCost`를 아예 파싱하지 않아서, 마나가 부족해 서버가 거절한 스킬을 HUD가 "사용 가능"으로
보여준다. `PLAYER_SKILL_DEFINITION`에 `std::uint32_t iResourceCost`를 추가한다.

`iDamage`는 `damageRatePercent`를 읽어 `attackPower × rate / 100`으로 계산한다. Client가
`PlayerProfiles.json`의 `attackPower`도 읽어야 하므로 `CCombatHUDViewModel::Initialize_Definitions`의
`PLAYER_PROFILE_DEFINITION`에 `iAttackPower`를 추가한다. 이 값은 **표시용**이며 실제 판정은
서버만 한다.

### G01.12 `ServerGameplayContractTests.cpp` 갱신 대상

독립 감사가 확인한 하드코딩 기대값 두 곳이 반드시 깨진다.

| 위치 | 현재 | 변경 후 근거 |
|---|---|---|
| `:194-195` | 보스 HP `10000 - 650 = 9350` | 34120 rate 361 × 공격력 100 / 100 = **361** → `9639` |
| `:314-315` | 플레이어 HP `1000 - 350 = 650` | 발탄 rate 350 × 공격력 100 / 100 = **350** → 변화 없음 |
| `:176` | 보스 HP 설정 10000 | 유지(테스트 전용 값) |
| `:288` | 플레이어 HP 설정 1000 | 유지(테스트 전용 값) |

여기에 새 테스트를 추가한다.

- `Resolve_Damage(100, 361) == 361`, `Resolve_Damage(100, 100) == 100`, `Resolve_Damage(0, 361) == 0`,
  `Resolve_Damage(1, 1) == 1`(0으로 내려가지 않음).
- `collisionRadius`를 더해야만 닿는 거리에서 근접 스킬이 명중하는지.
- `resourceCost`가 풀을 넘는 bootstrap을 거부하는지.

### G01.12b ProjectAudit에서 깨질 수 있는 검사

`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`이 밸런스 문서를 직접 읽는 곳은 세 군데다.

| 줄 | 검사 | 이번 변경의 영향 |
|---|---|---|
| 1020~1036 | `gameplay.playable-qw-contract` — 4개 class의 Q/W가 정확히 하나씩이고 DIMENSIONMASTER 행이 **0개**여야 한다 | 안전. 이번 G는 스킬 행을 추가하지 않는다. 차원술사 스킬을 넣는 순간 이 검사가 깨지므로 별도 슬라이스에서 같이 고친다 |
| 1104 | `Publish-GameplayBalance.ps1 -Mode Validate` 실행 | G01.7 편집이 끝나야 통과한다 |
| 1227~1231 | `gameplay.skill-binding-is-data` — `CPlayerSkillCatalog`가 `Balance/PlayerSkills.json`과 `Balance/DamageProfiles.json`을 읽고, `PlayerController.cpp`에 `\b3\d{4}\b` 패턴의 skill ID 하드코딩이 없어야 한다 | Client 파서를 고칠 때 두 경로 문자열을 지우지 않도록 주의한다 |
| 1016 | `hud.selected-class-boundary` — `CombatHUDViewModel.cpp`가 `Balance/PlayerProfiles.json`을 읽어야 한다 | `attackPower` 추가 시 이 경로 문자열이 유지되는지 확인 |

### G01.13 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
& "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" Framework.sln /t:Shared;Server /p:Configuration=Debug /p:Platform=x64
Server/Bin/Debug/Server.exe --contract-test
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

runtime smoke에서 확인할 것: **이동 속도가 절반으로 느려진 것이 의도한 변경인지**, Q 한 번에
발탄 HP가 361 줄어드는지, 마나가 276 줄고 약 11초에 회복되는지.

---

## G02 — 서버 damage 이벤트 (데미지 폰트의 전제)

### G02.1 목표와 종료 증거

서버가 이미 계산하고 버리는 damage 값을 snapshot에 실어 보낸다. 종료 증거는
`NetworkProtocolHarness` 왕복 테스트 통과와, Client가 한 tick의 피격 금액을 읽을 수 있는 것이다.

### G02.2 왜 필요한가 (감사 결과)

독립 감사와 그 반증 검증이 확인한 현재 상태다.

- 서버는 `PlayerSkillSystem.cpp:221-233`에서 damage를 계산해 보스 HP에서 빼고 **지역 변수로
  버린다.**
- `PLAYER_SNAPSHOT`과 `WORLD_ENTITY_SNAPSHOT`에는 delta, 공격자, 피격 시각, 피격 위치 필드가
  하나도 없다. `S2C_WORLD_SNAPSHOT`이 HP를 나르는 **유일한** 메시지다.
- 저장소 전체에서 `S2C_DAMAGE|DAMAGE_EVENT|FloatingDamage|DamageNumber` 검색 결과는
  `MainApp.cpp:389` 한 줄뿐이고, 그것은 폰트 **등록** 줄이다.

따라서 Client가 HP 레벨 차이로 damage를 역산하는 것은 불가능하다. 한 tick에 500이 한 번인지
250이 두 번인지 구분할 수 없고, 숫자를 띄울 위치도 알 수 없다.

### G02.3 왜 snapshot에 싣는가

새 packet type을 만들지 않는다. snapshot은 이미 30 Hz로 모든 player에게 나가고, damage는
snapshot을 만드는 바로 그 tick에 확정된다. 별도 메시지를 만들면 순서 보장과 재전송 정책을
새로 정해야 하는데, damage 숫자는 **놓쳐도 HP는 어긋나지 않는** 표현 전용 데이터다.
snapshot의 유일한 edge-triggered 필드라는 점을 주석으로 못박는다.

### G02.4 `Shared/Public/Network/PacketMessages.h` 추가 계약

```cpp
	// Lance Master alone authors nine ACTIVE skills and can hold all nine on
	// cooldown at once, so eight silently dropped one tile from the HUD.
	inline constexpr std::size_t MAX_PLAYER_COOLDOWNS = 16;
	// One tick applies at most one player hit and one boss hit per actor, so this
	// bounds a 30 Hz frame rather than a fight.
	inline constexpr std::size_t MAX_DAMAGE_EVENTS = 64;
```

`MAX_PLAYER_COOLDOWNS`를 8에서 16으로 올리는 것은 이 G가 고치는 **실제 버그**다. 창술사는
ACTIVE 스킬이 9개이고, 긴 쿨다운부터 순서대로 쓰면 9개가 동시에 쿨다운에 걸린다.
`GameRoom.cpp:525-534`가 `unordered_map` 순회 중에 8개에서 잘라내고 **그 뒤에** 정렬하므로,
어느 스킬이 HUD에서 사라질지가 해시 순서에 달려 있다.

```cpp
	// One resolved hit. HP in the snapshots above is a level, so a client that
	// only sees levels cannot tell 500 damage from two 250s inside one tick, and
	// cannot place a number where the hit landed. The server already computes this
	// value to subtract it; this carries the same number rather than letting the
	// client re-derive one it has no authority for.
	struct DAMAGE_EVENT
	{
		// Whoever took the damage: a player or a world entity, both of which live
		// in the same NET_ENTITY_ID space.
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iAmount = 0;
		// Where to anchor the number, in world units. Taken from the target at the
		// moment of the hit so a number does not follow the target afterwards.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		// True when a player dealt it. Presentation styles incoming and outgoing
		// damage differently, and only the server knows which is which.
		bool isOutgoing = false;
	};
```

`S2C_WORLD_SNAPSHOT`에 `std::vector<DAMAGE_EVENT> DamageEvents;`를 추가한다.

`PacketType.h`의 `NETWORK_PROTOCOL_VERSION`을 7에서 **8**로 올린다. snapshot의 wire layout이
바뀌기 때문이다. 이 파일에는 다른 세션의 미커밋 변경(`CHARACTER_SELECT_ARENA` 추가)이 있으므로
버전 상수 한 줄만 건드린다.

버전을 숫자로 박아 둔 곳은 없다. harness는 전부 상수를 참조한다
(`NetworkProtocolHarness.cpp:209, 284, 610, 633, 707-708`). `MAX_PLAYER_COOLDOWNS`를 16으로
올리는 것도 wire 상 개수는 `Write_U8`(최대 255)이라 형식 변경 없이 들어간다.

### G02.5 `Shared/Private/Network/PacketMessages.cpp` 변경 함수

익명 namespace에 검증자를 추가한다. 0 금액은 타격이 아니므로 표현 계층까지 가면 안 된다.

```cpp
	// A zero amount is not a hit, so it must not reach presentation as one; the
	// server clamps every resolved hit to at least 1.
	bool Is_Valid_DamageEvent(
		const LostArk::Shared::DAMAGE_EVENT& damage)
	{
		return
			damage.iTargetNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			0 != damage.iAmount &&
			std::isfinite(damage.fPositionX) &&
			std::isfinite(damage.fPositionY) &&
			std::isfinite(damage.fPositionZ);
	}
```

`Write_Message(S2C_WORLD_SNAPSHOT)`: 선검증에 `DamageEvents.size() > MAX_DAMAGE_EVENTS`와
`Is_Valid_DamageEvent` 루프를 더하고, 헤더에 `Write_U8(DamageEvents.size())`를 entity 개수
뒤에 넣고, entity 루프 뒤에 이벤트를 기록한다.

```cpp
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		writer.Write_U32(damage.iTargetNetEntityId);
		writer.Write_U32(damage.iAmount);
		writer.Write_F32(damage.fPositionX);
		writer.Write_F32(damage.fPositionY);
		writer.Write_F32(damage.fPositionZ);
		writer.Write_U8(damage.isOutgoing ? 1u : 0u);
	}
```

`Read_Message`: `damageEventCount`를 헤더에서 읽고 상한을 검사한 뒤 entity 루프 뒤에서 복원한다.
`rawOutgoing > 1u`를 거부해 bool 필드로 임의 바이트가 들어오지 않게 한다.

```cpp
	for (std::uint8_t i = 0; i < damageEventCount; ++i)
	{
		DAMAGE_EVENT damage{};
		std::uint8_t rawOutgoing = 0;
		if (!reader.Read_U32(damage.iTargetNetEntityId) ||
			!reader.Read_U32(damage.iAmount) ||
			!reader.Read_F32(damage.fPositionX) ||
			!reader.Read_F32(damage.fPositionY) ||
			!reader.Read_F32(damage.fPositionZ) ||
			!reader.Read_U8(rawOutgoing) ||
			rawOutgoing > 1u)
		{
			return false;
		}
		damage.isOutgoing = 0u != rawOutgoing;
		if (!Is_Valid_DamageEvent(damage))
			return false;
		decoded.DamageEvents.push_back(damage);
	}
```

### G02.6 Server 발생 지점

`CPlayerSkillSystem::Update`와 `CValtanBrain::Update`는 지금 `void`를 반환하고 out 파라미터가
없다. 두 함수에 `std::vector<DAMAGE_EVENT>& outDamageEvents`를 추가한다. 소유자는
`CGameRoom`이고, `Tick`이 매 tick 시작에 `clear()`, `Broadcast_WorldSnapshot`이 소비한 뒤
비운다. Room이 소유하는 이유는 한 tick의 이벤트가 그 tick의 snapshot 하나에만 실려야 하기
때문이다.

호출 지점은 각각 **한 곳뿐**이므로 시그니처 변경의 파급이 닫혀 있다.

```text
Server/Private/GameRoom.cpp:705   m_PlayerSkillSystem.Update(...)
Server/Private/GameRoom.cpp:764   m_ValtanBrain.Update(...)
선언                              Server/Public/PlayerSkillSystem.h:22
                                  Server/Public/ValtanBrain.h:15
```

`CValtanBrain::Update`는 `GameRoom.cpp:761-762`의 `m_ServerNavigation.Is_Loaded()` 게이트 안에
있고 `Update_Players`에는 그 게이트가 없다. navigation이 없는 Area에서는 보스 damage 이벤트가
아예 발생하지 않는다는 뜻이고, 이는 기존 동작 그대로다.

플레이어가 보스를 때린 지점(`PlayerSkillSystem.cpp:221-232`)에서 HP를 빼는 **직후**에
`{ closestBoss->iNetEntityId, damage, closestBoss 위치, true }`를 push한다. 보스가 플레이어를
때린 지점(`ValtanBrain.cpp:155-181`)에서는 `isOutgoing = false`로 push한다.

`GameRoom.cpp:525-534`의 쿨다운 잘라내기는 **정렬을 먼저 하고 나서** 자르도록 순서를 바꾼다.
상한을 16으로 올려도 미래에 스킬이 더 늘면 같은 문제가 나므로, 자를 때 결정적이어야 한다.

### G02.7 harness

`Tools/NetworkProtocolHarness`의 `Test_WorldSnapshotRoundTrip`에 damage 이벤트를 넣는다.
현재 14개 테스트 함수가 있고 `main()`이 14개를 호출한다.

- 이벤트 0개 왕복(기존 경로가 깨지지 않는지)
- 이벤트 2개 왕복(금액·위치·`isOutgoing` 보존)
- `iAmount = 0` 거부
- `rawOutgoing = 2` 거부
- `MAX_DAMAGE_EVENTS + 1` 거부
- 쿨다운 9개 왕복(예전 상한 8에서 잘리던 경우)

### G02.8 검증

```powershell
& MSBuild.exe Framework.sln /t:Shared;NetworkProtocolHarness;Server /p:Configuration=Debug /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
Server/Bin/Debug/Server.exe --contract-test
```

기대: harness failures 0.

---

## G03 — 데미지 폰트 (Client 표현)

### G03.1 목표와 종료 증거

G02가 보낸 금액을 피격 위치 위에 떠오르는 숫자로 그린다. 종료 증거는 실제 Server+Client 실행에서
발탄을 때렸을 때 서버가 계산한 값과 같은 숫자가 보이는 것이다.

### G03.2 이미 있는 것 (감사 결과)

- **폰트가 이미 등록되어 있다.** `MainApp.cpp:389`가 `Font_EventDamage` 태그로
  `BMKkubulim.spritefont`를 로드한다. 등록만 되고 한 번도 그려지지 않는다. 새 폰트 작업은 없다.
- `CGameInstance::Draw_Text` / `CFont_Manager`가 이미 공개 API다. `CCustomFont::Draw`는
  `MeasureString` 기반 0..1 origin을 받아 중앙 정렬을 지원한다.
- `CLevel_Loading::Render()`가 `Draw_Text`를 호출하는 유일한 곳이며, Level의 `Render()`에서
  텍스트를 그리면 모든 render group 뒤에 나온다(`GameInstance.cpp:137-146`).
- 발탄의 world transform은 **이미 공개 접근 가능하다.** `CGameObject::Get_Component`가 public이라
  `Get_Component(g_strTransformComTag)`로 읽을 수 있다. 엔진 변경이 필요 없다.
  (첫 감사가 "보스 위치 접근자가 없다"고 한 것은 반증 검증에서 오류로 확인됐다.)

### G03.3 없는 것

- world → screen 투영 헬퍼. `CPipeLine`은 VIEW/PROJ만 저장하고 `Project` 함수가 없다.
  DirectXTK `SimpleMath.h:1001-1002`에 `Viewport::Project`가 있지만 저장소에서 아무도 안 쓴다.
- 수명·페이드를 가진 임시 UI 객체. `CUI_Sprite`가 유일한 `CUIObject` 파생인데 수명 개념이 없다.
- damage 이벤트를 Client까지 나르는 경로(G02가 해결).

### G03.4 새 파일

```text
Client/Public/DamageNumberOverlay.h
Client/Private/DamageNumberOverlay.cpp
```

`CDamageNumberOverlay`는 **떠 있는 damage 숫자들의 수명과 화면 위치만** 소유한다. 소유하지
않는 것: 금액(서버가 준 값을 복사만 한다), 폰트 리소스(`CFont_Manager` 소유), 카메라 행렬
(`CPipeLine` 소유), 게임 상태.

`CUIObject`를 상속하지 않는 이유는, `CUIObject`는 레이어에 등록되어 매 프레임 갱신되는 게임
오브젝트인데 damage 숫자는 수십 개가 생겼다 사라지는 표현 요소라 각각을 GameObject로 만들면
레이어와 prototype 등록이 실익 없이 늘어나기 때문이다. `CLevel_Loading`이 텍스트를 직접 그리는
기존 선례와 같은 층에 둔다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include "Network/PacketMessages.h"

#include <cstdint>
#include <vector>

NS_BEGIN(Client)

/* Floating damage numbers. Owns only the lifetime and the screen placement of
numbers the server already resolved -- never the amount, which arrives in
S2C_WORLD_SNAPSHOT::DamageEvents and is copied verbatim, and never the font,
which CFont_Manager owns.

This is not a CUIObject: a damage number is a short-lived presentation element,
not a game object, and giving each one a prototype tag and a layer slot would add
registration without buying anything. CLevel_Loading already draws text straight
from a level Render(), and this sits at that same layer. */
class CDamageNumberOverlay final
{
public:
	/* Called once per applied snapshot by CClientReplication with that tick's
	events. World positions come from the server so a number stays where the hit
	landed instead of following a moving target. */
	void Push_Events(
		const vector<LostArk::Shared::DAMAGE_EVENT>& events);

	/* Ages every live number and retires the expired ones. */
	void Update(f32_t fTimeDelta);

	/* Projects each live number with the current view/projection and draws it.
	Called from the gameplay level's Render() so the numbers land after every
	render group. */
	void Render();

	void Clear();

private:
	struct FLOATING_NUMBER
	{
		uint32_t	iAmount = 0;
		/* World anchor at the moment of the hit, plus the upward drift applied
		so far. Screen position is derived every frame because the camera moves. */
		float3_t	vWorldPosition{};
		f32_t		fElapsedSeconds = 0.f;
		bool_t		bOutgoing = false;
	};

private:
	vector<FLOATING_NUMBER>	m_Numbers;
};

NS_END
```

`.vcxproj`와 `.vcxproj.filters` 등록이 필요하다. 물리 경로가 `Client/Public`, `Client/Private`
이므로 기존 필터 구조를 재배치하지 않고 항목만 추가한다.

### G03.5 world → screen 투영

`CPipeLine`에 헬퍼를 새로 만들지 않는다. Engine public 헤더를 바꾸면 `UpdateLib.bat`과 Client
전체 재빌드가 따라오고, 이 투영은 지금 소비자가 한 곳뿐이다. `CDamageNumberOverlay::Render`
안에서 `CGameInstance::Get().Get_Transform(D3DTS::VIEW)`와 `...(D3DTS::PROJ)`를 읽는다.
두 함수는 이미 public이고 `const float4x4_t*`를 돌려준다.

`XMVector3TransformCoord`는 w로 나눈 결과만 주고 w 부호를 잃어 카메라 뒤의 점이 화면 안으로
접혀 들어온다. 따라서 `XMVector3Transform`으로 clip space를 얻어 `w`를 직접 보고, `w <= 0`이면
그리지 않은 뒤 나머지에 대해서만 `x/w`, `y/w`로 NDC를 만든다. 화면 좌표는
`(ndc.x * 0.5f + 0.5f) * g_iWinSizeX`, `(-ndc.y * 0.5f + 0.5f) * g_iWinSizeY`다.

텍스트는 `CGameInstance::Draw_Text(TEXT("Font_EventDamage"), ...)`로 그린다. 이 태그는
`MainApp.cpp`가 이미 등록해 두고 한 번도 쓰지 않은 폰트다. `vOrigin`에 `float2_t(0.5f, 0.5f)`를
넘기면 `CCustomFont::Draw`가 `MeasureString`으로 가운데 정렬한다.

소비자가 둘 이상 생기면 그때 Engine으로 올린다.

소비자가 둘 이상 생기면 그때 Engine으로 올린다.

### G03.6 호출 흐름

```text
S2C_WORLD_SNAPSHOT 수신
-> CClientReplication::Apply_WorldSnapshot
   -> (기존) 플레이어/보스 상태 적용
   -> CDamageNumberOverlay::Push_Events(snapshot.DamageEvents)   <- 이 G가 새로 잇는 지점
-> CLevel_ValtanArena::Update  -> Overlay::Update(fTimeDelta)
-> CLevel_ValtanArena::Render  -> Overlay::Render()
```

`CLevel_ValtanArena`는 이미 `Render()`를 override로 선언하고 있다. 레벨 이탈 시 `Clear()`를
불러 남은 숫자가 다음 레벨로 새지 않게 한다.

### G03.7 공식 데미지 폰트 계약 반영

원작이 정의하는 폰트 파라미터를 그대로 쓴다.

| 원작 컬럼 | 값 | 이 프로젝트에서 |
|---|---|---|
| `SkillEffect.ShowDamageFont` | 16개 스킬 전부 1 | 지금은 전부 표시. 0인 효과가 생기면 그때 스킬별 플래그를 데이터로 올린다 |
| `SkillEffect.DmgFontScale` | 34620만 150, 31230 일부 110/130, 나머지 0(=100) | 이번 범위에서는 100 고정. scale을 쓰려면 damage 이벤트에 스킬 식별자가 필요하고 그것은 별도 슬라이스 |
| `Npc.DamageFontPosZRate` | 발탄 5000 | 숫자 앵커 높이. 보스 위 오프셋의 근거 |
| `Npc.DmgFontSizeRate` | 발탄 100 | 배율 1.0 |
| `SkillEffect.GlobalShowDamageFont` | 16개 전부 0 | 자기 타격만 표시 |

### G03.8 검증

```powershell
& MSBuild.exe Framework.sln /t:Client /p:Configuration=Debug /p:Platform=x64
```

runtime: `Framework.slnLaunch`의 `Server + Client`로 Lobby → Character Select → Enter Test →
발탄 진입 후 Q를 쓴다. 기대는 보스 위에 **361**이 뜨고 같은 tick에 보스 HP가 361 줄어드는 것.
서버 로그의 계산값과 화면 숫자가 같아야 한다.

---

## G04 — 쿨타임 UI 연결

### G04.1 목표와 종료 증거

UI 담당자가 `HUD_Layout.json`의 스킬 슬롯 하나에 이미지 위젯을 붙이고 매 프레임 0..1 진행률을
받을 수 있게 한다. 종료 증거는 UI 코드가 packet·socket·`CPlayerSkillCatalog`를 직접 만지지 않고
`CCombatHUDViewModel`만 읽어 쿨타임이 도는 것이다.

### G04.2 현재 상태 (감사 결과)

이미 되어 있는 것:

- 쿨다운은 **스킬별**로 서버가 소유한다(`SERVER_PLAYER::CooldownEndTickBySkillId`).
- snapshot이 스킬별 `iCooldownEndTick`을 나른다(`SKILL_COOLDOWN_SNAPSHOT`).
- `CCombatHUDViewModel`이 `HUD_SKILL_STATE`에 `iCooldownDurationTicks`와 `iCooldownEndTick`을
  채운다. 남은 tick 계산식은 `max(0, end - serverTick)`으로 문서화되어 있다.

없는 것:

- `iCooldownDurationTicks`는 **쓰이는 곳이 없다.** 저장소 전체 검색 결과가 선언 한 곳과 쓰기
  한 곳뿐이다. `HUD_SKILL_STATE::Is_Ready`도 호출되는 곳이 없다.
- 슬롯 이름으로 스킬을 찾는 접근자가 없다. `Get_Player()`가 구조체 전체를 주고 `Skills`는
  `strInputSlot`으로만 정렬되어 있다.
- `CHUDRuntimeView`의 `HUD_SLOT`에 스킬 바인딩 필드가 없고, `Load()`는 JSON의 `id`와 `type`을
  **읽지도 않는다.**
- `Build_PlayerSkills`가 `COMBO` 스킬을 건너뛰어 좌클릭 평타는 HUD에 아예 오지 않는다.
- Client는 `resourceCost`를 파싱하지 않아 마나 부족을 표시할 수 없다(G01에서 해결).

### G04.3 이미 있는 선례를 쓴다

`Data/UI/Loading/LoadingLayout.json` + `Client/Private/Level_Loading.cpp`가 이 G가 필요로 하는
패턴을 **이미 전부** 구현하고 있다.

```text
Level_Loading.cpp:229  CProjectDataRoot::Resolve(L"UI/Loading/LoadingLayout.json")
Level_Loading.cpp:250  슬롯의 id 필드를 읽는다
Level_Loading.cpp:300  Add_GameObject_to_Layer(..., TEXT("Prototype_GameObject_UI_Sprite"), ...)
Level_Loading.cpp:318  ProgressFill 을 stable id 로 바인딩
Level_Loading.cpp:76   fFillWidth = m_fProgressTrackWidth * m_fDisplayProgress
UI_Sprite.cpp:36       Add_RenderObject(RENDERGROUP::UI, ...)   <- ImGui 아님
```

따라서 "layout JSON → `CUIObject` 런타임 팩토리"를 새로 발명하지 않는다. 같은 패턴을 HUD 문서에
적용한다.

### G04.4 슬롯 id가 이미 바인딩 키다

`HUD_Layout.json`의 84개 슬롯을 실측했다. `type` 분포는
`{0:24, 3:2, 4:2, 5:11, 6:23, 7:18, 8:4}`이고 `type: 7`(SKILL) 18개 중 14개가 소유 class 없는
공용 슬롯이다.

```text
Skill_Q  Skill_W  Skill_E  Skill_R  Skill_A  Skill_S  Skill_D  Skill_F
SpecialSkill_1 .. SpecialSkill_6
```

나머지 4개는 도화가 전용(`Yin_Skill_Z`, `Yin_Skill_X`, `Yin_Skill_Z_BG`, `Yin_Skill_X_BG`)이다.

`Skill_<슬롯>` 형태 8개는 `inputSlot`과 이름이 그대로 대응하므로 id 접두사로 바인딩할 수 있다.

**그러나 이것만으로는 부족하다.** 창술사가 쓰는 9개 슬롯은 `Q W E R A S T V ALT_V`인데
`Skill_T`, `Skill_V`, `Skill_ALT_V`가 **없다.** 반대로 `Skill_D`, `Skill_F`는 지금 어느 class도
쓰지 않는다. 각성기 3개(T·V·ALT_V)를 담을 자리는 `SpecialSkill_1..6`이지만 그 id에는 슬롯
이름이 들어 있지 않다.

따라서 이름 규칙만으로는 닫히지 않고, 둘 중 하나를 골라야 한다.

| 안 | 방법 | 장점 | 단점 |
|---|---|---|---|
| A | `HUD_Layout.json` slot에 `"inputSlot": "T"` 필드를 추가하고 layout schema version을 올린다 | 이름 규칙 의존이 사라지고 UI 담당자가 슬롯을 자유롭게 재배치할 수 있다 | schema version과 `CHUDLayoutTool` writer/reader를 같이 고쳐야 한다 |
| B | `Skill_T`, `Skill_V`, `Skill_ALT_V`를 `CHUDLayoutTool`에서 새로 만들고 이름 규칙만 쓴다 | 코드 변경이 가장 작다 | id 이름이 계약이라 오타가 조용히 슬롯을 죽인다. `SpecialSkill_*` 6개는 계속 미사용 |

**A를 권장한다.** `AGENTS.md`가 "저장 ID에 pointer/prototype tag/vector index를 쓰지 않는다"를
요구하는 것과 같은 이유로, 바인딩도 이름 규칙 추론이 아니라 명시 필드여야 한다.
`slot.id`는 계속 저장·런타임 identity로 남고, `inputSlot`은 그 슬롯이 무엇을 보여주는지를
선언하는 별도 필드다.

어느 쪽이든 파싱은 코드 한 곳(`Try_Resolve_SkillSlot`)에 가두고, 해석되지 않는 `type: 7` 슬롯은
조용히 무시하지 말고 상태 문자열에 남겨 UI 담당자가 오타를 즉시 본다.

### G04.5 `CCombatHUDViewModel` 추가 계약

UI가 쓸 읽기 함수를 추가한다. 지금 UI는 `Get_Player()`로 구조체 전체를 받아 직접 뒤져야 한다.

```cpp
		/* The skill a class has on one quick slot, or nullptr when that slot is
		empty. Slot names are the inputSlot strings in the balance document, so a
		UI widget matches on "Q" or "ALT_V" and never on a vector index. */
		const HUD_SKILL_STATE* Find_SkillBySlot(
			const std::string& inputSlot) const;

		/* 0 when the skill is ready, 1 at the instant it went on cooldown, and a
		linear fall in between. This is the number a cooldown sweep multiplies by;
		the UI must not run its own timer, because the server tick is the only
		clock that matches the authority that granted the cooldown. */
		float Get_CooldownProgress(const std::string& inputSlot) const;

		/* False when the class has the skill but cannot pay for it right now.
		Separate from cooldown so a widget can grey out for the right reason. */
		bool Has_ResourceFor(const std::string& inputSlot) const;
```

`Get_CooldownProgress`가 `iCooldownDurationTicks`의 첫 소비자가 된다. 지금은 쓰이지 않는
멤버라 잘못돼도 아무도 모르지만, 분모가 되는 순간 틀리면 눈에 보인다.

tick 비교는 **wrap-safe**로 고친다. 감사가 `GameRoom.cpp:528`,
`PlayerSkillSystem.cpp:82`, `ClientReplication.cpp:403` 세 곳의 부호 없는 비교를 지적했고,
`PlayerSkillSystem.cpp:17`에 이미 `static_cast<int32_t>(a - b) > 0` 관용구가 있는데 tick에는
안 쓰이고 있다. 30 Hz에서 `uint32` 랩은 약 4.7년이라 실제로 만날 일은 없지만, 같은 파일에 올바른
관용구가 이미 있으므로 새 비교는 그것을 쓴다.

### G04.6 `Build_PlayerSkills`의 COMBO 제외를 없앤다

현재 주석은 "콤보는 쿨다운이 없으니 퀵슬롯 타일을 차지하지 않는다"이다. 그러나 LMB 평타도
HUD가 아이콘·마나·콤보 단계를 보여줄 대상이고, `iComboStage`는 이미 snapshot에 있다.
COMBO를 포함시키되 `iCooldownDurationTicks = 0`이면 `Get_CooldownProgress`가 0을 돌려주도록
한다. 0으로 나누지 않게 분모 0을 먼저 검사한다.

### G04.7 `CHUDRuntimeView` 변경

`HUD_SLOT`에 두 필드를 추가한다.

```cpp
		/* Stable slot id from the layout document. Load() currently discards it;
		it becomes the binding key here. */
		string	strSlotId;
		/* inputSlot parsed out of a "Skill_<slot>" id, empty for chrome. */
		string	strInputSlot;
```

`Load()`가 `id`를 읽어 저장하고 `Try_Parse_SkillSlotId`로 `inputSlot`을 뽑는다.

`Render()`가 `inputSlot`이 있는 슬롯에 대해 `CCombatHUDViewModel::Get_CooldownProgress`를 읽어
쿨다운 오버레이를 그린다. 현재 `CHUDRuntimeView`에는 `AddImageQuad`만 있고 UV 스크롤·마스크·
방사형 프리미티브가 없으므로, 첫 구현은 **아래에서 위로 차오르는 사각형 마스크**로 한다.
`PathArcTo` 기반 방사형 스윕은 ImGui draw list로 가능하지만 별도 항목으로 둔다.

`Resources/UI/HUD/Common/`에는 쿨다운용 아트가 없다는 것을 확인했다. 첫 구현은 반투명 검은
사각형이고, UI 담당자가 아트를 넣으면 layout JSON의 layer로 교체된다.

### G04.8 UI 담당자에게 열리는 계약

이 G가 끝나면 UI 담당자가 하는 일은 이것뿐이다.

1. `CHUDLayoutTool`에서 슬롯의 이미지 layer를 채우고, 그 슬롯이 어느 quick slot을 보여줄지
   `inputSlot`으로 지정한다(G04.4 A안). 기존 `Skill_Q` 같은 id는 그대로 둔다.
2. 런타임이 그 값으로 바인딩한다. C++ 수정 없음.
3. 새 class의 스킬을 `PlayerSkills.json`에 `inputSlot`과 함께 넣으면 같은 슬롯이 자동으로 산다.

UI 담당자가 알아야 할 현재 공백을 명시한다. `Skill_D`와 `Skill_F`는 지금 어느 class도 쓰지
않고, 창술사의 `T`·`V`·`ALT_V`는 대응 슬롯이 아직 없다. 이 세 개는 `SpecialSkill_1..3`에
`inputSlot`을 지정해 붙이는 것이 가장 적은 작업이다.

UI 코드는 `CCombatHUDViewModel`의 세 함수만 부른다. packet·socket·`Change_Level`·
`CPlayerSkillCatalog` 직접 호출은 여전히 금지다.

### G04.9 검증

```powershell
& MSBuild.exe Framework.sln /t:Client /p:Configuration=Debug /p:Platform=x64
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

runtime: 발탄에서 창술사 9개 ACTIVE를 긴 쿨다운부터 연속으로 쓴다. 기대는 **9개 전부** 쿨다운
표시가 남아 있는 것(예전 상한 8에서는 하나가 임의로 사라졌다). 마나가 부족한 스킬은 쿨다운이
끝났어도 사용 불가로 보여야 한다.

---

## G05 — 발탄 패턴 damage 공식화 (후속)

이번 범위에 넣지 않는다. 발탄의 패턴별 damage는 `EFTable_SkillEffect`가 아니라 원작 액션 파일
`data3.lpk :: EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_01-1.loa`(366,062 bytes, 복호화
확인 완료)에 있는 notify 스케줄과 엮여 있다. 기존 5개 class의 `.animnotify`/`.skilltiming`을
만든 것과 같은 파서를 발탄에 적용해 `Data/Animation/Reference/Valtan/`을 만드는 것이 선행
조건이다.

지금은 `damage.valtan.basic-swing`을 기존 350을 rate로 옮긴 값으로 둔다.

---

## 6. 이 계획의 열린 결정

구현 전에 확인이 필요한 항목만 남긴다.

1. **이동 속도.** 공식 값 2.95는 현재 6.0의 절반 이하다. 원작 충실도를 택할지, 현재 조작감을
   유지하고 공식 값은 문서에만 남길지.
2. **초각성기 상한.** 34620의 공식 rate 648,936은 60,000 HP 보스에 의미가 없다. 각성기의 3배
   (15,102)로 상한을 걸었는데, 다른 배수를 원하면 그 값만 바꾸면 된다.
3. **마나 규모.** 공식 `CostMp`(276~695)를 쓰려면 풀이 1000이어야 한다. 100 풀을 유지하고
   `CostMp`를 비율로 축소하는 선택지도 있다. 전자를 택한 이유는 공식 숫자를 그대로 보여주는
   쪽이 "공식 기준"이라는 요청에 맞기 때문이다.
4. **damage roll.** `ValueA`(최소)만 쓴다. `ValueA..ValueB` 난수 roll은 서버 결정성과
   harness 재현성을 깨므로 별도 슬라이스로 둔다.

---

## 7. 검증 상태 구분

| 항목 | 상태 |
|---|---|
| 원작 DB 796개 복호화 | **완료** (data2 796 entries / data3 12,304 entries, failures 0) |
| 5 class + 발탄 수치 추출 | **완료** (독립 측정 에이전트와 16개 스킬 rate 전부 일치) |
| `Publish-GameplayBalance.ps1 -Mode Validate` 새 스키마 통과 | **완료** (실행 확인 후 되돌림) |
| G01.3~G01.6b JSON 4종 | **작성 완료, 저장소 미반영** |
| G01.7 publisher 편집 | **작성·검증 완료, 저장소 미반영** |
| 계획서 자체 비평 | **완료** (4개 관점 비평 + 각 비평의 반증 검증) |
| G01 Server/Client C++ | **미구현** |
| G02 damage 이벤트 | **미구현** |
| G03 데미지 폰트 | **미구현** |
| G04 쿨타임 UI | **미구현** |
| Debug 빌드 / contract test / harness | **미실행** |
| Server+Client runtime smoke | **미실행** |

이 계획서에 코드가 적혀 있다는 사실은 구현 완료가 아니다.
