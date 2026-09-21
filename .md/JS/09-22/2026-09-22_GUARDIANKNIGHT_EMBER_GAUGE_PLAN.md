# 가디언나이트 엠버레스 오브 게이지·기운·소켓 — PLAN

브랜치 `feature/player-guardian-knight`. 1차 통합(`../09-21/2026-09-21_GUARDIANKNIGHT_CLASS_INTEGRATION_PLAN.md` §9 "게이지·만충 조건")의 후속.

## 0. 결론과 범위

가디언나이트는 MP 대신 두 자원을 쓴다. 서버가 소유하고 snapshot으로 복제하며 Client는 표시만 한다.

| 자원 | 서버 상태 | 규칙 (2026-09-22 사용자 확정) |
|---|---|---|
| 엠버레스 오브 게이지 | 기존 `iCurrentIdentity / iMaximumIdentity` (0~100) | 일반 상태에서 일반·발현 스킬이 **적중할 때마다** +N. 100일 때만 Z 화신화. 진입 후 100→0으로 **15초 고정** drain, 0이 되거나 Z를 다시 누르면 해제(해제 시 0). |
| 엠버레스의 기운 | `iEmberOrbs` (0~보유 상한) | 발현/화신 스킬 사용 시 DB `CostIgauge0`대로 4/4/5/6개 소모(보유량이 모자라면 있는 만큼만, 스킬은 항상 사용 가능, 원작 규칙). Q/W/E/R/D **사용 시** 2/3/3/4/6개 회복(원작 tooltip 수치). |
| 소켓 잠금 | `iEmberLockedSockets` (0~10) | 일반형 A/S/F 사용 시 1개 잠김 → 보유 상한 10→9→8… 화신화 진입 시 잠금 전부 해제 + 기운 전부 회복. |
| 기운 소모 보너스 | `iEmberSpentOnAction` | 소모한 개수당 피해 +10% (원작 `sys.identity.dragonknight_normal/dragon`). |

MP는 profile `maximumResource 1000`을 유지하되(Shared/publisher/catalog의 `maximumResource != 0` 계약을 건드리지 않음) HUD에서 GK만 마나 바·텍스트를 숨긴다. GK 전용 HUD 아트는 없으므로 마나 텍스트 자리에 `오브 xx%  기운 n/cap` readout을 임시로 그린다(UI 아트는 후속).

원작 근거(`EFTable_GameMsg.db`): `sys.identity.dragonknight_normal`, `_dragon`, `tip.desc.skill_49100~49150`, `EFTable_Skill.CostIgauge0/1`.
원작과 다르게 결정한 것: 게이지 충전은 히트당 고정 +4(원작은 특화 스탯), 화신 지속 15초 고정(원작은 게이지 drain), 기운 회복은 적중이 아니라 사용, 비전투 5초 소켓 회복 없음, 화신 중 0.8초당 기운 회복 없음.

## 1. 데이터 — `Data/Balance/GuardianKnightEmber.json` (신규, receipt 비대상)

`PlayerSkillTargeting.json`과 같은 선택 문서 방식이다. receipt `fieldEntryCount`에 들어가지 않으므로 7 profile·91 skill의 receipt 항목을 늘리지 않는다.

```json
{
  "schema": "lostark.guardian-knight-ember",
  "formatVersion": 1,
  "characterClass": "GUARDIANKNIGHT",
  "orbGauge": { "gainPerHit": 4, "dragonDurationMs": 15000 },
  "ember": { "maximumSockets": 10, "damageBonusPercentPerOrb": 10 },
  "skills": [
    { "skillId": 49100, "emberGain": 2, "emberCost": 0, "locksSocket": false },
    { "skillId": 49110, "emberGain": 3, "emberCost": 0, "locksSocket": false },
    { "skillId": 49120, "emberGain": 3, "emberCost": 0, "locksSocket": false },
    { "skillId": 49130, "emberGain": 4, "emberCost": 0, "locksSocket": false },
    { "skillId": 49150, "emberGain": 6, "emberCost": 0, "locksSocket": false },
    { "skillId": 49200, "emberGain": 0, "emberCost": 4, "locksSocket": true },
    { "skillId": 49220, "emberGain": 0, "emberCost": 4, "locksSocket": true },
    { "skillId": 49260, "emberGain": 0, "emberCost": 5, "locksSocket": true },
    { "skillId": 49210, "emberGain": 0, "emberCost": 4, "locksSocket": false },
    { "skillId": 49230, "emberGain": 0, "emberCost": 4, "locksSocket": false },
    { "skillId": 49270, "emberGain": 0, "emberCost": 5, "locksSocket": false },
    { "skillId": 49330, "emberGain": 0, "emberCost": 6, "locksSocket": false }
  ]
}
```

`PlayerProfiles.json` GUARDIANKNIGHT: `maximumIdentity 0 → 100` (나머지 identity 필드 0 유지). receipt는 `Update-BalanceProvenanceReceipt.ps1`로 이 한 필드만 `PROJECT_TUNED` 동기화.
`Client.vcxproj` `96.DataFiles`: `PlayerSkillTargeting.json` None 항목 바로 아래 `GuardianKnightEmber.json` 추가.

## 2. G01 publisher — `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`

- 위치: `SKILLTARGET` 블록(`$targetingDocument`…`$skillRows.Add('SKILLTARGET'…)`) 바로 아래.
- 검증: schema/formatVersion 1, `characterClass` = `GUARDIANKNIGHT`이고 profile에 존재하며 `maximumIdentity > 0`, `gainPerHit 1..maximumIdentity`, `dragonDurationMs 1000..600000`, `maximumSockets 1..64`, `damageBonusPercentPerOrb 0..1000`. skills 행: owner skill 존재·같은 class, `emberGain`/`emberCost` 중 하나만 0이 아님, 둘 다 `≤ maximumSockets`, `locksSocket`는 `emberCost>0`이고 owner `requiredStance == GUARDIANKNIGHT_HUMAN`일 때만 true, skillId 중복 금지.
- 행: `$skillRows`에 `SKILLEMBER <skillId> <gain> <cost> <lock 0/1>`, `$playerRows`에 `PLAYEREMBER <class> <gainPerHit> <dragonDurationMs> <maximumSockets> <bonusPercent>`. (`$rows = damage + skill + player + …` 순서라 SKILLEMBER은 SKILL 뒤, PLAYEREMBER은 PLAYER 뒤에 온다.)
- "Player identity gauge never spends" 검사: ember profile이 있는 class는 화신 drain이 spend이므로 제외.

## 3. G02 Server catalog — `GameplayCatalog.h/.cpp`

- `PLAYER_SKILL_DEFINITION`: `fRootMotionScale` 바로 아래 `std::uint32_t iEmberGain = 0; std::uint32_t iEmberCost = 0; bool locksEmberSocket = false;`
- `PLAYER_RUNTIME_PROFILE` 닫는 `};` 바로 아래 새 struct `GUARDIAN_EMBER_PROFILE { iGaugeGainPerHit, iDragonDurationMs, iMaximumSockets, iDamageBonusPercentPerOrb }`.
- `CGameplayCatalog`: `Find_Player` 선언 아래 `const GUARDIAN_EMBER_PROFILE* Find_EmberProfile(CHARACTER_CLASS_ID) const;`, `m_Players` 아래 `std::unordered_map<CHARACTER_CLASS_ID, GUARDIAN_EMBER_PROFILE> m_EmberProfiles;`.
- 파서: `SKILLTARGET` 분기 바로 아래 `SKILLEMBER`(5필드, owner join, 중복 금지, gain/cost 배타), `PLAYER` 분기 바로 아래 `PLAYEREMBER`(6필드, 중복 금지). post-load: ember profile class는 `m_Players`에 있고 `iMaximumIdentity>0`, `iGaugeGainPerHit ≤ iMaximumIdentity`; SKILLEMBER owner class == ember class; gain/cost ≤ maximumSockets; "never spends" 검사에서 ember class 제외.

## 4. G03 Server state·판정 — `ServerPlayer.h`, `PlayerSkillSystem.h/.cpp`, reset 5곳

`SERVER_PLAYER`: `iIdentityAccumulator` 바로 아래
```cpp
std::uint32_t iEmberOrbs = 0;
std::uint32_t iEmberLockedSockets = 0;
std::uint32_t iEmberSpentOnAction = 0;
```

`CPlayerSkillSystem`
- `static void Reset_Gauges(SERVER_PLAYER&, const CGameplayCatalog&)`: identity = ember class면 0, 아니면 max; accumulator 0; orbs = maximumSockets, locked 0, spent 0. 호출: `GameRoom_Admission.cpp:143`, `GameRoom_Helpers.cpp:577`, `GameRoom_PlayerCommands.cpp:452`, `GameRoom_GateProgress.cpp:471`, `GameRoom_KoukuRaidFlow.cpp:405`의 `iCurrentIdentity = iMaximumIdentity; iIdentityAccumulator = 0` 두 줄을 교체.
- `Try_StartInternal`: cooldown/resource/identity 검사 조건에 `ember && eSetsStance != default && eSetsStance != eStance && iCurrentIdentity < iMaximumIdentity → false` 추가(화신화 만충 조건). 비용 차감(`iCurrentIdentity -= iIdentityCost`) 바로 아래 `Apply_EmberOnStart(player, *skill, *ember)`: spent = min(orbs, cost); orbs -= spent; locks면 locked = min(locked+1, max); capacity = max-locked; orbs = min(orbs+gain, capacity); `iEmberSpentOnAction = spent`.
- `Commit_StanceChange`: stance가 실제로 바뀔 때 ember class면 — DRAGON 진입: locked=0, orbs=max, accumulator 0; default 복귀: identity=0, accumulator 0.
- `Update_Identity`: 맨 앞에서 ember profile이 있으면 `Update_EmberGauge`로 분기. DRAGON 유지 중: `acc += iMaximumIdentity; threshold = dragonDurationMs*SERVER_TICK_HZ/1000; while acc>=threshold: --identity`; 0이면 stance=default. HUMAN: 변화 없음(회복은 히트로만).
- 히트 게이지: `Update`의 shape 히트 루프에서 `targets`가 비지 않으면, fallback 분기에서 `closestBoss`가 있으면 `Gain_EmberGauge(player, ember)` (stance == default이고 identity<max일 때 +gainPerHit, max에서 clamp).
- 피해 보너스: `resolveRawDamage`의 결과에 `(100 + bonus × iEmberSpentOnAction)/100` 곱(ember class만).

## 5. G04 Shared — protocol 99

`PLAYER_SNAPSHOT` `iMaximumIdentity` 바로 아래
```cpp
std::uint8_t iEmberOrbs = 0;
std::uint8_t iEmberLockedSockets = 0;
std::uint8_t iEmberMaximumSockets = 0;
```
writer/reader: `iMaximumIdentity` 직후 U8×3. validator: `iEmberOrbs + iEmberLockedSockets <= iEmberMaximumSockets`. `NETWORK_PROTOCOL_VERSION 98 → 99` + 주석 한 줄. `GameRoom_Replication.cpp:587` 아래에서 ember profile이 있으면 세 값 채움. 하네스: 첫 player snapshot에 7/2/10 설정 + decode 단언, 8/3/10 invalid 단언.

## 6. G05 Client — `CombatHUDViewModel.h/.cpp`, `MainApp.cpp`

- `HUD_PLAYER_STATE` `iMaximumIdentity` 아래 세 필드, `Apply_LocalPlayer`에서 복사.
- `Update_PlayerHealthManaBar`: GK면 `ManaBar_Fill` 숨김. 마나 텍스트 분기: GK면 `L"오브 xx%  기운 n/cap"`을 같은 위치에 그림.

## 7. 검증

1. `Publish-GameplayBalance.ps1 -Mode Publish` → `Gameplay.bootstrap`에 `PLAYEREMBER 1행 + SKILLEMBER 12행`.
2. 계약 테스트(PlayerActions): Q 사용 → orbs 2; A 사용 → orbs 0(=min), locked 1; identity<100에서 Z 거부; identity=100에서 Z 승인 → commit 후 locked 0/orbs 10; DRAGON 15초 Update 후 identity 0·stance HUMAN.
3. 하네스 failures 0. Debug Product 빌드. Server/Client 재시작.
4. 사용자: 가디언나이트로 Q/W/E/R/D 적중 → 게이지 상승, A/S/F → 기운 소모·소켓 감소 readout, 100에서 Z → 날개, 15초 후 자동 해제, 화신 중 Z → 해제.
