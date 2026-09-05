# 2026-09-05 KoukuSaydon 광기 게이지(플레이어 수치)와 무채색 본체 변신 구현 계획서

> 문서 종류: 구현 계획서 (범위와 변경 단위 합의용)
>
> 상태: 실측 완료 / 사용자 값 확인 대기 / 구현 전
>
> 기준 브랜치: `codex/kouku-shield-stagger` (관문 스폰·HUD·G05 변경 위에 얹는다)
>
> 선행 문서: [MN_RPCT_03 무채색 본체 추출 결과](2026-09-05_KOUKU_SAYDON_MN_RPCT_03_COLORLESS_BODY_EXTRACTION_RESULT.md), [Arena 관문 스폰 계획](2026-09-05_KOUKU_SAYDON_ARENA_GATE_SPAWN_AND_BOSS_HUD_IMPLEMENTATION_PLAN.md)

## 0. 목표와 종료 증거

목표는 세 문장이다.

1. **광기 게이지는 플레이어 수치다.** Server가 플레이어마다 `current/maximum`을 소유하고 snapshot으로 복제하며, HUD가 HP·MP 옆에 세 번째 바로 그린다. Client는 값을 만들지 않는다.
2. **게이지가 가득 차면 세이튼이 무채색 본체로 변신한다.** Server가 대상 세이튼의 gameplay phase를 `2`로 올리고 `colorlessDurationMs` 뒤 `1`로 되돌린다. 변신은 같은 entity·같은 HP·같은 NetEntityId 위의 **형태 전환**이며 despawn/spawn이 아니다.
3. **Client는 phase를 보고 몸체만 바꾼다.** MN_RPCT_05 ↔ MN_RPCT_03은 rig(165본)와 clip 이름(`rpct00_*` 249개)이 같으므로 Product binding·audition·HUD focus가 그대로 유지된다.

종료 증거는 다음 넷이다.

1. `NetworkProtocolHarness` failures 0 (v59 `PLAYER_SNAPSHOT` round-trip), `Server.exe --contract-test` failures 0 (게이지 누적 → trigger → phase 2 → 만료 → phase 1, 리셋 정책).
2. `Publish-GameplayBalance.ps1`이 `BOSSMADNESS` 행을, catalog가 `formBodies`를 admission한다.
3. Debug Product 빌드 통과.
4. 사용자가 실제 Server+Client에서 `1관문 - 세이튼` 스폰 → 게이지 상승 → 가득 찰 때 세이튼이 무채색으로 바뀌고 정해진 시간 뒤 원래 몸체로 돌아오는 것을 눈으로 확인한다. 이 확인 전에는 visual PASS를 기록하지 않는다.

## 1. 현재 실측

### 1.1 플레이어 수치 복제 경로 (재사용)

| 계약 | 위치 | 상태 |
|---|---|---|
| `SERVER_PLAYER::iCurrentHp/iMaximumHp/iCurrentResource/iMaximumResource/iCurrentIdentity/iMaximumIdentity` | `Server/Public/ServerPlayer.h:175~186` | 플레이어 게이지 셋이 같은 꼴로 있음 |
| `PLAYER_SNAPSHOT` 같은 필드 + `Is_Valid_PlayerSnapshot`이 `iCurrentIdentity <= iMaximumIdentity` 검사 | `Shared/Public/Network/PacketMessages.h:713~720`, `Shared/Private/Network/PacketMessages.cpp:88~118` | v58. identity는 "maximum 0이면 HUD가 그리지 않음" 규칙 |
| Server snapshot 조립 | `GameRoom.cpp:9632~9672` (`snapshot.iCurrentIdentity = player.iCurrentIdentity`) | 필드 추가 지점 |
| 플레이어 리셋 지점 (spawn·class 변경·부활) | `GameRoom.cpp:693~695, 2267~2269, 3046~3048` | `iCurrentResource = iMaximumResource` 등과 같은 자리에 광기 0 초기화 |
| HUD ViewModel | `CCombatHUDViewModel::Apply_LocalPlayer` (`CombatHUDViewModel.cpp:204~211`), `HUD_PLAYER_STATE` | HP/MP/identity를 복사만 함 |
| HUD 플레이어 바 | `MainApp.cpp:3534~3537` `Set_SlotFillRatio("HealthBar_Fill"/"ManaBar_Fill")`, `Data/UI/HUD/HUD_Layout.json`의 `ManaBar`(1312행)·`ManaBar_Fill`(4776행) | 같은 형태로 `MadnessBar`/`MadnessBar_Fill` slot 추가 |
| protocol 버전 | `Shared/Public/Network/PacketType.h:39` `NETWORK_PROTOCOL_VERSION = 58` | v59로 올린다 |
| 하네스 fixture | `NetworkProtocolHarness.cpp:2592` `PLAYER_SNAPSHOT first{}` | 새 필드 round-trip 추가 |

### 1.2 보스 형태 복제 경로 (재사용)

| 계약 | 위치 | 상태 |
|---|---|---|
| `SERVER_WORLD_ENTITY::iPhase` → `BOSS_COMBAT_SNAPSHOT::iGameplayPhase` | `CBossCombatRuntime::Set_GameplayPhase` (`BossCombatRuntime.cpp`), `GameRoom.cpp:9755` | phase 변경이 `iStateRevision`을 올려 Client가 같은 revision을 재해석하지 않음 |
| Kouku family 보스의 phase action 기대치 | `GameplayCatalog.cpp` `expectedGameplayPhaseActionCount = 0` (family) | 런타임 `Set_GameplayPhase`는 pattern action이 아니므로 catalog 검증과 무관 |
| pattern의 `minimumPhase/maximumPhase` | Product PATTERN row | 무채색 형태 전용 패턴을 phase 2로 묶을 수 있음(후속) |
| Client Kouku snapshot 경로 | `ClientReplication.cpp` `Is_KoukuSaydonArenaBoss` 분기 → `CNpc::Apply_NetworkState` + action binding | `entity.BossCombat.iGameplayPhase`는 지금 읽지 않음 → 몸체 교체 edge 추가 지점 |
| presentation service | `CKoukuSaydonPresentationAssetService::Ensure_Prototypes(archetype)` | archetype당 body prototype 1개. 형태별 두 번째 body prototype 추가 지점 |
| 몸체 동일성 | `MN_RPCT_03.wmodel` 249 clip == `MN_RPCT_05.wmodel` clip 집합, 165본 동일 (추출 RESULT §3) | binding row는 clip 이름으로 검증되므로 형태 교체 후에도 같은 row가 유효 |

### 1.3 게이지를 올릴 수 있는 지점

| 원인 | 지점 | 지금 가능한가 |
|---|---|---|
| 플레이어 스킬이 세이튼에 적중 | `CServerCombatHitRuntime::Apply_PlayerToWorld` (`hit.iSourcePlayerId` 보유) | **가능** |
| 세이튼 stage hit이 플레이어에 적중 | `Apply_WorldToPlayer(SERVER_WORLD_TO_PLAYER_HIT)` — 호출자는 `ValtanBrain::ApplyPatternHit`, `CombatObjectRuntime`, `MonsterBrain` | Kouku stage hit은 무력화 계획 G04(공용 hit helper) 이후 |
| 시간 경과 / 자연 감소 | Kouku arena 보스 tick (`Update_WorldEntities` arena 분기) | **가능** |

### 1.4 데이터 정본

- `Data/Balance/BossProfiles.json` formatVersion 4, 11개 exact property (`Publish-GameplayBalance.ps1:1164`). 새 optional `madnessPolicy`는 schema v5로 올린다.
- `Data/Actors/BossCatalog.json` formatVersion 7, Client `CActorCatalog`가 row property 15개 exact, `bodyModel` 1개. 형태별 body를 담는 `formBodies`는 v8로 올린다.
- receipt updater는 boss profile의 모든 property를 field로 등록하므로 `madnessPolicy` 객체는 JSON 값 비교로 `PROJECT_TUNED` 등록된다.

## 2. 요구사항 ↔ 반영

| # | 사용자 요구 | 반영 | G |
|---|---|---|---|
| R1 | 광기 게이지를 플레이어 수치에 적용 | `SERVER_PLAYER::iCurrentMadness/iMaximumMadness`, `PLAYER_SNAPSHOT` v59 필드, HUD `MadnessBar`. 최대치는 arena 세이튼 profile의 `madnessPolicy.maximum`에서 오고, 세이튼이 없는 world는 0(바 숨김) | G01~G03, G04 |
| R2 | 가득 찼을 때를 기준으로 무채색 본체로 변신 | `triggerPolicy`(`ANY_PLAYER_FULL` 기본 / `ALL_PLAYERS_FULL`)가 만족되면 Server가 세이튼 `Set_GameplayPhase(2)` | G02 |
| R3 | 일정 시간 동안 | `colorlessDurationMs` 만료 tick에 `Set_GameplayPhase(1)`. 그 동안 `resetPolicy`대로 게이지를 0으로 | G02 |
| R4 | 무채색 본체 | catalog `formBodies[{ gameplayPhase 2, bodyModel MN_RPCT_03 }]`. Client가 phase edge에서 같은 entity의 CNpc 몸체를 교체하고 되돌림 | G01, G04 |

## 3. 변경할 파일

새 C++ 파일 2개(`Server/Public/KoukuSaydonMadnessRuntime.h`, `Server/Private/KoukuSaydonMadnessRuntime.cpp`)를 추가하며 `Server.vcxproj`/`.filters`에 `ClInclude`/`ClCompile` 항목을 등록한다. Client 새 파일은 없다.

| G | 파일 | 역할 |
|---|---|---|
| G01 | `Data/Balance/BossProfiles.json` (v5) | 세이튼 3 row(`G1_SAYDON`, `G3_SAYDON`, `BINGO_SAYDON`)에 `madnessPolicy` |
| G01 | `Data/Actors/BossCatalog.json` (v8) | 같은 3 row에 `formBodies`, 나머지 row는 `[]` |
| G01 | `Data/UI/HUD/HUD_Layout.json` | `MadnessBar`, `MadnessBar_Fill` slot (HUD Layout Tool로 배치) |
| G01 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | v5 property, `madnessPolicy` 검증, `BOSSMADNESS` 행, catalog v8 `formBodies` 검증 |
| G01 | `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1` | 변경 없음(자동 등록) — 실행만 |
| G01 | `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | v5/v8 기대값 |
| G02 | `Shared/Public/Network/PacketType.h`, `PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp` | v59, `PLAYER_SNAPSHOT::iCurrentMadness/iMaximumMadness`, writer/reader/validator |
| G02 | `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | round-trip·invalid(current > maximum) 검사 |
| G02 | `Server/Public/ServerPlayer.h`, `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | 플레이어 필드, `BOSS_MADNESS_POLICY`, `BOSSMADNESS` parse |
| G02 | `Server/Public|Private/KoukuSaydonMadnessRuntime.*` (새 파일) | 누적·감소·trigger·만료를 tick 함수로 |
| G02 | `Server/Private/ServerCombatHitRuntime.cpp`, `Server/Public/ServerCombatHitRuntime.h` | 플레이어→보스 적중 source 기록, `SERVER_WORLD_TO_PLAYER_HIT::iMadnessGain` |
| G02 | `Server/Private/GameRoom.cpp` | arena 분기에서 runtime 호출, snapshot 필드, 리셋 3곳, 대상 세이튼 phase 변경 |
| G02 | `Server/Private/ServerGameplayContractTests.cpp` | runtime 검사 |
| G03 | `Client/Public|Private/CombatHUDViewModel.*`, `Client/Private/MainApp.cpp` | `HUD_PLAYER_STATE` 필드, `MadnessBar_Fill` ratio |
| G04 | `Client/Private/ActorCatalog.cpp`, `Client/Public/ActorCatalog.h` | v8 `formBodies` parse |
| G04 | `Client/Public|Private/KoukuSaydonPresentationAssetService.*` | 형태별 body prototype(`…_Form2`), `Get_ModelPrototypeTag(archetype, phase)` |
| G04 | `Client/Private/ClientReplication.cpp`, `Client/Public/ClientReplication.h` | phase edge에서 CNpc 몸체 교체 transaction |

## 4. 데이터와 호출 흐름

### 4.1 `madnessPolicy` (BossProfiles v5, 세이튼 row에만)

```json
"madnessPolicy": {
  "maximum": 100,
  "gainPerPlayerHit": 2,
  "gainPerBossHit": 15,
  "gainPerSecond": 0,
  "decayPerSecond": 0,
  "triggerPolicy": "ANY_PLAYER_FULL",
  "colorlessPhase": 2,
  "colorlessDurationMs": 15000,
  "resetPolicy": "TRIGGERING_PLAYER",
  "freezeGaugesWhileColorless": true
}
```

- `maximum ≥ 1`, gain/decay ≥ 0, `colorlessPhase` 2..255, `colorlessDurationMs` 1..600000.
- `triggerPolicy`: `ANY_PLAYER_FULL`(한 명이라도 가득) / `ALL_PLAYERS_FULL`(살아 있는 전원).
- `resetPolicy`: `TRIGGERING_PLAYER`(가득 찬 플레이어만 0) / `ALL_PLAYERS`(전원 0).
- 값은 첫 값이며 사용자가 확정한다(§7). publisher가 `BOSSMADNESS <archetype> <maximum> <gainPlayerHit> <gainBossHit> <gainPerSecond> <decayPerSecond> <trigger> <phase> <durationMs> <reset> <freeze>` 행으로 emit하고 Server `BOSS_RUNTIME_PROFILE::MadnessPolicy`(optional, `bHasMadnessPolicy`)에 담는다.

### 4.2 `formBodies` (BossCatalog v8)

```json
"formBodies": [
  {
    "gameplayPhase": 2,
    "bodyModel": "Character/KoukuSaton/MN_RPCT_03/MN_RPCT_03.wmodel",
    "bodyModelPreScale": 0.017,
    "weaponModel": null,
    "weaponModelPreScale": null
  }
]
```

- phase 1은 기존 `bodyModel`. 다른 row는 `formBodies: []`.
- Client `CActorCatalog`는 property 16개 exact, 각 form의 phase 유일성·2..255, bodyModel resource ID, preScale (0,100]을 검사한다. publisher도 같은 규칙.
- 무기 규칙은 기본 몸체와 같다(빙고 세이튼이 무채색 형태에서도 뿅망치를 들지 여부는 §7).

### 4.3 Server 실행 흐름

```text
플레이어 스킬 적중 (CPlayerSkillSystem → Apply_PlayerToWorld, LANDED)
→ target이 BOSS면 target.MadnessHitSourcesThisTick.push_back(hit.iSourcePlayerId)   (catalog 무관, 값 없음)

세이튼 stage hit 적중 (G04 공용 hit helper 이후) → SERVER_WORLD_TO_PLAYER_HIT.iMadnessGain = policy.gainPerBossHit
→ Apply_WorldToPlayer가 LANDED일 때 player.iCurrentMadness += iMadnessGain (maximum clamp)

Update_WorldEntities arena 분기 (Is_ArenaBoss) 매 tick
→ CKoukuSaydonMadnessRuntime::Update(boss, players, policy, tick, status)
   ├─ policy 없음 → return
   ├─ 살아 있는 플레이어의 iMaximumMadness = policy.maximum (world 진입·class 변경·부활 시에도 같은 값)
   ├─ boss.MadnessHitSourcesThisTick 소비: 해당 player += gainPerPlayerHit
   ├─ freezeGaugesWhileColorless가 아니거나 phase 1이면 gainPerSecond/decayPerSecond를 tick 비율로 적용
   ├─ phase 1이고 triggerPolicy 만족 → CBossCombatRuntime::Set_GameplayPhase(boss, colorlessPhase),
   │    boss.iMadnessColorlessEndTick = tick + durationMs*30/1000, resetPolicy대로 게이지 0
   └─ phase == colorlessPhase이고 endTick 도달 → Set_GameplayPhase(boss, 1), endTick 0
→ snapshot: PLAYER_SNAPSHOT.iCurrentMadness/iMaximumMadness, BOSS_COMBAT_SNAPSHOT.iGameplayPhase(기존)
```

- 리셋: spawn/class 변경/부활 지점 3곳에서 `iCurrentMadness = 0`. `iMaximumMadness`는 runtime이 다음 tick에 다시 채운다.
- 세이튼이 죽거나 despawn되면 그 world의 플레이어 `iMaximumMadness = 0`(바 숨김). Debug despawn(`Despawn_KoukuSaydonArenaDebugEntities`)도 같은 정리.
- 무채색 중 세이튼이 받는 피해 배율은 이번 범위 밖(§7 질문). 필요하면 `damageTakenPercentWhileColorless`를 policy에 추가하고 `Apply_PlayerToWorld` 앞단에서 곱한다.

### 4.4 Client 실행 흐름

```text
PLAYER_SNAPSHOT → CCombatHUDViewModel::Apply_LocalPlayer → HUD_PLAYER_STATE.iCurrentMadness/iMaximumMadness
→ CMainApp::Update_PlayerHealthManaBar: MadnessBar_Fill ratio, maximum 0이면 MadnessBar/MadnessBar_Fill 숨김

WORLD_ENTITY_SNAPSHOT (Kouku family BOSS) → ClientReplication Kouku 분기
→ iter->second.iPresentedPhase != entity.BossCombat.iGameplayPhase 이면
   ├─ CKoukuSaydonPresentationAssetService::Ensure_Prototypes(archetype) — form body prototype도 같이 admission
   ├─ 새 CNpc(desc: 같은 위치/yaw/collision, strModelTag = Get_ModelPrototypeTag(archetype, phase), weapon 규칙 유지) 생성
   ├─ 성공: 기존 CNpc Remove, pNpc 교체, iPresentedPhase 갱신, 현재 action clip을 Try_Resolve_Action으로 다시 재생(없으면 idle)
   └─ 실패: 기존 CNpc 유지, iPresentedPhase는 그대로(다음 snapshot에서 재시도), m_strPendingPresentationFailure 기록
```

- HUD focus·audition target·Complete Play는 archetype 기준이라 형태 교체와 무관하다.
- Effect/Sound cue는 이번 범위 밖(무채색 진입 연출은 Effect 저작).

## 5. G별 구현 범위

### G01 — 데이터·publisher

- `BossProfiles.json` v5: 세이튼 3 row에 `madnessPolicy`. publisher는 v5 exact property에 `madnessPolicy`를 optional로 두고(없으면 `BOSSMADNESS` 행 없음), 값 범위·enum을 검사한다.
- `BossCatalog.json` v8: `formBodies`. publisher는 family row만 비어 있지 않을 수 있고 phase 유일·body 경로 prefix·preScale 범위를 검사한다.
- `HUD_Layout.json`: `MadnessBar`(type 4, `ManaBar` 아래)와 `MadnessBar_Fill`(`ManaBar_Fill`과 같은 fill 계약) slot을 HUD Layout Tool로 배치·저장. 이미지는 임시로 `UI/HUD/Common/Empty bar reverse.png`와 기존 fill 이미지를 tint로 재사용하고 UI 담당이 교체한다.
- 검증: receipt updater → balance publish → `test_kouku_saydon_runtime_inputs` → bootstrap `BOSSMADNESS` 3행 grep.

### G02 — Shared·Server

- `PacketType.h` v59. `PLAYER_SNAPSHOT`에 `iCurrentMadness`, `iMaximumMadness`(`iMaximumIdentity` 바로 아래). writer/reader/`Is_Valid_PlayerSnapshot`(`current <= maximum`). 하네스에 round-trip과 `current > maximum` 거부 검사.
- `SERVER_PLAYER::iCurrentMadness/iMaximumMadness`(`iMaximumIdentity` 아래). `SERVER_WORLD_ENTITY::MadnessHitSourcesThisTick`(vector<PLAYER_ID>), `iMadnessColorlessEndTick`.
- `BOSS_MADNESS_POLICY` struct + `BOSS_RUNTIME_PROFILE::MadnessPolicy/bHasMadnessPolicy`, `BOSSMADNESS` parse(profile 존재·family·값 범위·중복 금지).
- `CKoukuSaydonMadnessRuntime`(새 파일): §4.3의 순수 tick 함수. GameRoom·CBossCombatRuntime·SERVER_PLAYER만 알고 Client/asset을 모른다.
- `Apply_PlayerToWorld`: LANDED/KILLED일 때 BOSS target에 source player 기록. `SERVER_WORLD_TO_PLAYER_HIT::iMadnessGain`과 `Apply_WorldToPlayer` 누적(clamp).
- `GameRoom.cpp`: arena 분기에서 runtime 호출(audition 중인 보스도 포함, brain 앞), snapshot 필드, 리셋 3곳, 세이튼 사망/despawn 시 maximum 0.
- contract test: (a) policy 있는 세이튼 + 플레이어 2명, 적중 N회로 한 명이 가득 → phase 2, endTick 설정, 해당 플레이어 0; (b) durationMs 경과 → phase 1; (c) `ALL_PLAYERS_FULL`이면 한 명만으로는 전환 없음; (d) freeze 중 게이지 불변; (e) despawn 뒤 maximum 0.

### G03 — Client HUD

- `HUD_PLAYER_STATE::iCurrentMadness/iMaximumMadness`, `Apply_LocalPlayer` 복사.
- `Update_PlayerHealthManaBar`에 `MadnessBar_Fill` ratio와 maximum 0 숨김.

### G04 — Client 형태 교체

- `CActorCatalog` v8 parse(`BOSS_ACTOR_ENTRY::FormBodies`).
- presentation service: `Ensure_Prototypes`가 family row의 form body마다 `Prototype_Component_Model_KoukuSaydon_<archetype>_Form<phase>`를 만들고, `Get_ModelPrototypeTag(archetype, phase)`가 phase 1이면 기존 tag를 준다. binding은 기본 몸체 clip으로 한 번 검증(집합 동일).
- `ClientReplication` Kouku 분기: `WORLD_ENTITY_PRESENTATION::iPresentedPhase` 추가, §4.4 교체 transaction. 실패는 기존 몸체 보존.

### 후속 (이번 범위 밖)

- 무채색 형태 전용 패턴 저작: `MN_RPCT_00.loa`의 "광기를 잃은 쿠크세이튼_*" 67 action은 actionreference intake가 없다. `MN_RPCT_00.actionreference.json` intake와 `MN_RPCT_00 → MN_RPCT_05` alias(07과 같은 규칙)를 추가한 뒤 pattern `minimumPhase/maximumPhase = 2`로 묶는다.
- Kouku stage hit(무력화 계획 G04) 전에는 `gainPerBossHit`이 실제로 발생하지 않는다. 그때까지는 `gainPerPlayerHit`·`gainPerSecond`로 동작을 확인한다.
- 무채색 진입/해제 Effect·Sound, 피해 배율, 앵콜 phase 3.

## 6. 검증 요약

| 단계 | 명령 / 절차 | 기대 |
|---|---|---|
| G01 | receipt updater → `Publish-GameplayBalance.ps1` → python runtime inputs | `BOSSMADNESS` 3행, catalog v8 통과 |
| G02 | Debug Product 빌드, `NetworkProtocolHarness`, `Server.exe --contract-test` | 전부 failures 0 |
| G03/G04 | Debug Product 빌드, `git diff --check` | 오류 0 |
| runtime (사용자) | Server+Client 재시작 → `1관문 - 세이튼` → 스킬로 세이튼 타격 | 플레이어 HUD 세 번째 바가 오르고, 가득 차면 세이튼이 무채색으로 바뀌며 `colorlessDurationMs` 뒤 원래 몸체로 돌아온다. HUD 보스 phase 값이 2 → 1 |

Server와 Client는 v59로 함께 빌드·배포해야 한다.

## 7. 사용자 결정이 필요한 값

1. `maximum`, `gainPerPlayerHit`, `gainPerBossHit`, `gainPerSecond`, `decayPerSecond` 첫 값(위 예시는 임의값).
2. `triggerPolicy`: 한 명이 가득 차면 변신인지, 전원인지.
3. `colorlessDurationMs`(예시 15초)와 무채색 동안 다른 플레이어 게이지를 멈출지(`freezeGaugesWhileColorless`).
4. `resetPolicy`: 가득 찬 사람만 0인지 전원 0인지.
5. 무채색 동안 세이튼이 받는 피해 배율(버스트 창)을 둘지.
6. 빙고 세이튼이 무채색 형태에서도 뿅망치를 드는지.
7. "앵콜을 외친 쿠크세이튼"을 phase 3으로 둘지, 별도 archetype으로 둘지.
