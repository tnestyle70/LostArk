# Boss Tool 패턴 흐름 저작·서버 연속 검증 구현 계획

> 구현 및 자동 검증 완료. 실제 결과와 남은 수동 확인은
> `2026-08-27_BOSS_TOOL_PATTERN_FLOW_AUDITION_RESULT.md`를 따른다.

## 0. 작업 결론

기존 `Boss Tool`은 단일 Pattern의 실제 Server occurrence와 우측 연결 진단을 검증하는 도구로 이미 역할이
명확하다. 이 화면을 순서 편집 기능으로 복잡하게 만들지 않고 다음 두 탭으로만 분리한다.

```text
Boss Verification | Pattern Flow
```

- `Boss Verification`은 현재 `Play Selected`, `Repeat`, `Stop After Current`, 조건부 `Revive Player`,
  Pattern 목록, Stage 선택과 우측 `Why / Advanced diagnostics`를 온전히 보존한다.
- `Pattern Flow`는 같은 All Effects Valtan inventory에서 Pattern을 슬롯에 추가하고 순서를 바꿔 저장한 뒤,
  첫 슬롯 또는 선택 슬롯부터 끝까지 실제 Server 패턴 경로로 재생한다.
- Flow 탭의 `Preview Isolated`도 기존 `CValtanPatternAuditionService::Submit()`을 호출한다. 같은 Pattern을
  골랐을 때 기존 `Play Selected`와 다른 로컬 재생기, 다른 reset, 다른 presentation 경로를 만들지 않는다.
- Flow 전체 재생은 Client가 `Submit()`을 슬롯마다 반복하지 않는다. Server가 목록을 먼저 검증하고 중앙
  full reset을 한 번만 수행한 뒤 기존 ordered pattern runtime을 순서대로 소비한다.

이 변경은 Debug 검증 Flow를 저장하는 기능이다. Product 자동 전투 정본인
`Data/Valtan/Valtan.gameplay.json / decisionModel.scriptedSequence`를 자동 교체하거나 Hot Reload하지 않는다.
검증이 끝난 Flow를 Product 순서로 승격하는 것은 별도 명시적 publisher 변경 단위다.

## 1. 사용자 작업 계약

```text
사용자 작업  : All Effects와 같은 발탄 Pattern 목록에서 필요한 Pattern을 슬롯에 추가하고,
              순서를 조정·저장한 뒤 첫 슬롯 또는 선택 슬롯부터 실제 상호작용을 연속 검증한다.
기존 작업    : 단일 Pattern 선택 -> Play Selected -> 우측 Stage/Effect/Camera/Hit/World 진단은 그대로다.
Flow 저장    : stable flowId와 stable slotId를 가진 Debug authoring JSON이다.
Flow 실행    : stable slotId부터 suffix를 Server가 한 번의 reset으로 실행한다.
실행 권위    : pattern/stage/action/hit/damage/motion/world event는 기존 Server Product runtime이다.
제외         : Client local replay, ImGui의 gameplay JSON 직접 수정, 자동 Product publish, mid-raid checkpoint 합성.
```

첫 구현의 핵심 동작은 다음 여섯 개로 제한한다.

1. `Add from All Effects`
2. `Move Up / Move Down`
3. `Remove`
4. `Save / Reload`
5. `Preview Isolated`
6. `Start First / Start Here / Stop After Current`

## 2. 현재 코드·데이터 실측과 보호 경계

### 2.1 공용 Pattern inventory

현재 Boss Tool과 Effect Tool의 All Effects Valtan tree는 모두 `CValtanPatternTree::Load()`와
`Build_ToolAuditionInventory()`를 소비한다. Boss Tool selector는 현재 admitted
`CorePatternIds + AnimatorPatternIds`, 합계 28개의 stable pattern ID를 표시한다.

Flow 탭은 새 목록, 수동 ID 배열, `scriptedSequence` 순회를 만들지 않는다. 반드시 현재 Boss Tool의
`m_AuditionInventory`를 그대로 소비하며 다음 파일은 이 변경에서 수정하지 않는다.

```text
Client/Public/ValtanPatternTree.h
Client/Private/ValtanPatternTree.cpp
Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
```

특히 현재 strict join에 추가된 `partDamagePolicy`, `counterProxy` typed 보존·검증을 유지한다.

### 2.2 기존 단일 Pattern 재생

`CBossTool::Submit_SelectedPattern()`은 stable boss placement와 selected pattern ID를
`CValtanPatternAuditionService::Submit()`에 제출한다. Server의 단일 Pattern audition은 occurrence마다
플레이어·발탄·아레나를 reset하고 선택 Pattern 하나를 기존 `PendingPatternIds` 경계로 실행한다.

이 경로는 수정하지 않는다. Flow 탭의 고립 재생도 같은 함수를 호출하되 별도 consumer ID를 사용하여
기존 Verification 탭의 Repeat 완료 판정을 소비하지 않게 한다.

### 2.3 Product ordered sequence 드리프트

현재 `Valtan.gameplay.json`의 Product `scriptedSequence`는 28개이고 `VALTAN_DASH_CHARGE`가 index 5에
들어 있다. 기존 Boss Tool 정적 하네스는 27개와 지형 파괴 Pattern index 13/14를 가정해 1개가 실패한다.

Boss Tool Flow는 Product sequence의 count/index 계약이 아니다. Boss Tool 하네스에서 이 검사를 분리하고,
지형 파괴 tuple은 vector index가 아니라 exact pattern ID lookup으로 검증한다. Product sequence 순서와 Dash
포함 여부는 ordered-sequence 전용 validator가 소유한다. 이 구현은 해당 Product 데이터를 임의 교정하지 않는다.

### 2.4 All Effects tree 과거 회귀

과거 별도 worktree의 오래된 전체 파일을 덮어써서 최신 strict join을 잃고
`Effect Tool -> All Effects -> Valtan` tree가 나오지 않은 회귀가 있었다. 이 변경에서는 기존 tree parser나
All Effects refresh 본문을 복사·대체하지 않는다. 변경 전후 focused contract와 no-touch diff를 함께 확인하고
재발 방지 규칙을 `.md/GB/gotchas.md`에 추가한다.

## 3. G00 — 탭과 화면 구조

### 3.1 Boss Verification

현재 `CBossTool::Render()`의 본문을 `Render_BossVerificationTab()`으로 옮기고 함수 호출 순서와 상태를
보존한다.

```text
Live summary
Play Selected / Repeat / Stop After Current / Revive Player
Patterns                                      Selected Pattern / Stage
                                              five connection lanes
                                              Why / Advanced diagnostics
```

`Boss Verification`을 첫 탭으로 두며 기존 selection, Follow Live, Repeat, status는 Flow 상태와 공유하지 않는다.

### 3.2 Pattern Flow

```text
Pattern Flow   SAVED | UNSAVED | EXTERNAL CONFLICT
[Reload] [Save]

┌ Ordered Slots ────────────────────────┬ Selected Slot ────────────────────┐
│ [Add from All Effects...]             │ 06  VALTAN_DASH_CHARGE            │
│ 01  Pattern A                         │ [Preview Isolated]                │
│ 02  Pattern B                         │                                   │
│ 03  Pattern A                         │ Flow Playback                     │
│ ...                                   │ [Start First] [Start Here]        │
│ [Up] [Down] [Remove]                  │ [Stop After Current] [Revive*]    │
└───────────────────────────────────────┴────────────────────────────────────┘
flow lifecycle / exact failure reason
```

- Add popup은 `m_AuditionInventory.CorePatternIds`와 `AnimatorPatternIds`만 같은 순서로 표시한다.
- 서로 다른 슬롯에 같은 `patternId`를 반복 배치할 수 있다.
- 슬롯별 반복 버튼을 만들지 않고 선택 슬롯에만 Up/Down/Remove를 둔다.
- display ordinal은 현재 배열 순서일 뿐 저장·request identity가 아니다.
- Flow 편집 selection은 `m_strSelectedFlowSlotId`로 분리한다.
- Flow가 실행 중이면 Add/Move/Remove/Save/Reload를 잠가 Server에 제출된 revision과 화면을 일치시킨다.
- dirty draft에서도 `Preview Isolated`는 가능하지만 Flow 전체 시작은 마지막으로 저장된 clean revision만 허용한다.
- 빈 Flow는 저장할 수 있으나 Start 버튼은 비활성화한다.

## 4. G01 — Flow authoring 문서와 저장

새 Debug authoring 정본은 기존 52-row health-bar/arena precondition timeline인
`ValtanDebugAudition.json`과 역할을 섞지 않는다.

```text
Data/Encounters/Valtan/ValtanBossAuditionFlows.json
```

첫 format은 하나의 default Flow만 소유한다.

```json
{
  "schema": "lostark.valtan-boss-audition-flows",
  "formatVersion": 1,
  "flows": [
    {
      "flowId": "flow.valtan.boss-tool.default",
      "nextSlotOrdinal": 29,
      "interStepPursuitMs": 1000,
      "slots": [
        {
          "slotId": "flow.valtan.boss-tool.default.slot.000001",
          "patternId": "VALTAN_ARENA_BREAK_109"
        }
      ]
    }
  ]
}
```

### 4.1 ID와 validation

- `flowId`, `slotId`, `patternId`는 bounded non-empty stable ID다.
- `slots`는 0..32개다. playback request는 1..32개만 허용한다.
- `slotId`는 문서와 request 안에서 unique다.
- `patternId` 중복은 허용한다.
- 모든 `patternId`는 현재 `m_AuditionInventory`에서 exact resolve되어야 한다.
- 배열 순서가 playback 순서이고 move 뒤에도 `slotId`는 유지된다.
- 삭제한 slot ordinal은 재사용하지 않으며 `nextSlotOrdinal`은 monotonic이다.
- `interStepPursuitMs`는 Product sequence와 같은 100..10000ms다.
- 잘못된 schema/version/property/type/ID/unknown pattern/overflow는 전체 admission 실패다.

### 4.2 코드 책임

새 파일은 다음 둘로 제한한다.

```text
Client/Public/ValtanPatternFlowDocument.h
Client/Private/ValtanPatternFlowDocument.cpp
```

이 클래스는 codec과 authoring transaction을 소유한다.

- `Load`: bytes read -> exact parse -> validate against admitted inventory -> stage -> draft/baseline commit
- `Add/Move/Remove`: stable slot ID 기반 draft mutation
- `Save`: disk baseline CAS -> draft full validate -> temp durable write -> atomic replace -> re-read/revalidate -> baseline commit
- Reload 실패는 현재 admitted draft/baseline을 보존한다.
- Save 실패와 external conflict는 disk와 현재 draft를 보존한다.
- dirty Reload는 명시적 discard confirmation 뒤 실행한다.
- Boss Tool은 JSON parse, stream write, atomic replace를 직접 하지 않는다.

새 C++는 UTF-8 BOM 없음으로 만들고 `Client.vcxproj/.filters`에 물리 폴더와 일치하게 등록한다.
Git 관리 Data 원본도 `96.DataFiles`의 `None` 항목으로만 등록한다.

## 5. G02 — Shared typed Flow protocol

기존 단일 `PLAY_PATTERN_ID` packet을 확장하지 않고 Debug Flow 전용 packet을 둔다.

```text
C2S_DEBUG_VALTAN_PATTERN_FLOW_START
S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT
C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT
S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE
```

Start request의 최소 payload는 다음이다.

```text
requestSequence
bossPlacementId
flowId
flowRevision
startSlotId
interStepPursuitMs
slots[1..32] { slotId, patternId }
```

Server result/lifecycle는 request identity, `roomFlowEpoch`, pinned gameplay revision, 현재 `slotId`,
`patternId`, ordinal/count와 exact terminal reason을 echo한다.

```text
PENDING -> ACTIVE(slot) <-> PAUSED_FOR_REVIVE
        -> COMPLETED_HOLD | STOPPED_HOLD | REJECTED | ABORTED
```

- encode/decode는 max 32와 각 ID length를 bounded 처리한다.
- malformed/truncated/oversize/unknown enum은 destination을 부분 mutation하지 않고 실패한다.
- stop request는 `controlSequence + flowId + roomFlowEpoch`로 stale flow를 제어하지 못하게 한다.
- Release Server는 Debug Flow start/stop을 명시적으로 거부한다.

## 6. G03 — Client playback service

새 `CValtanPatternFlowService`가 panel 수명과 packet queue를 분리한다.

```text
Client/Public/ValtanPatternFlowService.h
Client/Private/ValtanPatternFlowService.cpp
```

- `Start(flow, startSlotId)`는 clean saved revision과 전체 ordered slots를 typed request로 보낸다.
- `Stop_AfterCurrent()`는 현재 flow/epoch identity를 보낸다.
- `Update()`가 result/lifecycle queue를 유일하게 drain한다.
- snapshot은 state, flow revision, current slot/pattern, ordinal/count, room epoch, pinned revision, reason을 보존한다.
- world inbound generation이 바뀌면 stale in-flight lifecycle을 abort한다.
- Boss Tool은 service를 호출하고 snapshot만 읽으며 `CNetworkManager`를 직접 include/send하지 않는다.
- `CMainApp` main-thread update에 service를 연결하므로 Boss Tool 창이 닫혀도 lifecycle을 잃지 않는다.
- 단일 Pattern의 `CValtanPatternAuditionService`는 변경하지 않는다.

## 7. G04 — Server의 임시 ordered-sequence override

### 7.1 선택한 runtime seam

`PendingPatternIds`에 32개를 넣거나 timeline runner를 복제하지 않는다. 활성 Debug Flow를
ephemeral `BOSS_PATTERN_SEQUENCE_DEFINITION`으로 stage하고, 해당 Flow 동안만 `CValtanBrain`이 Product
sequence 대신 이 ordered-sequence view를 소비한다.

이 경로는 기존 구현의 다음 계약을 그대로 재사용한다.

- stable pattern definition lookup
- `BeginPattern`과 모든 Stage/action/hit/damage/motion/world event
- 성공 완료 때만 cursor advance
- Pattern 사이 pursuit
- 플레이어 전멸 시 stage/action/hit clock pause와 revive 뒤 resume
- terminal IDLE hold

Flow가 없으면 `catalog.Find_BossPatternSequence()` 결과를 그대로 사용하여 Product 자동 전투 동작을 바꾸지 않는다.

### 7.2 Start transaction

Server fixed-tick command는 다음 전체를 먼저 validate/stage한다.

- Debug build / Valtan Arena / exact boss placement / owner session·player
- newer request sequence
- 1..32 slots, unique slot ID, exact start slot membership, every pattern definition resolve
- active one-shot audition, timeline, running Flow와 충돌 없음
- active gameplay catalog generation과 revision pin 가능
- player bait placement와 navigation projection 가능

하나라도 실패하면 boss/player/arena/world event를 바꾸지 않는다. 모두 성공하면
`Reset_ValtanAuditionState()`를 정확히 한 번 호출하고 다음을 한 commit으로 적용한다.

- 플레이어를 기존 Valtan 중앙 audition 시작 위치에 배치하고 full combat state로 복원
- 발탄과 arena/world mutation을 fresh audition state로 reset
- selected start slot ordinal부터 Flow cursor 설정
- Flow ID, slot list, gameplay revision, owner/session, room epoch pin
- 첫 Pattern을 기존 ordered runtime에서 시작

Pattern 사이에는 boss/player/arena를 다시 reset하지 않는다.

### 7.3 Start Here의 의미

선택 슬롯부터 시작해도 prefix Pattern이 만들었을 phase/world state를 합성하지 않는다. 항상 fresh phase-1
audition reset 뒤 raw suffix를 실행한다. 따라서 `ARENA_BREAK_109` 이후의 벽 파괴 상태 같은 mid-raid
checkpoint fidelity 검증은 prefix를 포함해 Start First로 재생해야 한다. checkpoint ID가 필요해지면 별도 계약이다.

### 7.4 stop, death, terminal, pin

- `Stop After Current`: active occurrence는 끝까지 실행하고 다음 slot 전에 `STOPPED_HOLD`; pursuit 중이면 즉시 hold.
- player death: 현재 slot/stage/tick/cursor를 고정하고 기존 `Revive Player` 뒤 같은 occurrence를 resume.
- natural end: `COMPLETED_HOLD`에서 boss를 IDLE로 유지하고 Product sequence/weighted fallback을 재개하지 않는다.
- 새 Flow start 또는 단일 Pattern start만 terminal hold를 authoritative reset으로 교체한다.
- owner leave/disconnect는 ABORTED, audition reset, pinned generation release다.
- Flow 전체가 시작 gameplay revision을 사용하도록 required revision collection과 catalog resolve에 pin을 포함한다.

## 8. G05 — 파일 변경 범위

### 신규

```text
Client/Public/ValtanPatternFlowDocument.h
Client/Private/ValtanPatternFlowDocument.cpp
Client/Public/ValtanPatternFlowService.h
Client/Private/ValtanPatternFlowService.cpp
Data/Encounters/Valtan/ValtanBossAuditionFlows.json
Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py
.md/GB/08-27/2026-08-27_BOSS_TOOL_PATTERN_FLOW_AUDITION_RESULT.md
```

### 수정

```text
Client/Public/BossTool.h
Client/Private/BossTool.cpp
Client/Public/NetworkManager.h
Client/Private/NetworkManager.cpp
Client/Private/MainApp.cpp
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
Shared/Public/Network/PacketType.h
Shared/Public/Network/PacketMessages.h
Shared/Private/Network/PacketMessages.cpp
Server/Public/RoomCommand.h
Server/Public/GameRoom.h
Server/Private/GameRoom.cpp
Server/Public/ValtanBrain.h
Server/Private/ValtanBrain.cpp
Server/Private/ServerApp.cpp
Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp
Server/Private/ServerGameplayContractTests.cpp
Tools/ValtanPipeline/test_valtan_boss_tool_contract.py
.md/GB/gotchas.md
```

실제 호출 seam 조사 중 불필요한 파일은 추가하지 않는다. 기존 dirty 변경을 덮어쓰지 않고 현재 파일에서
최소 delta만 patch한다.

## 9. G06 — 실행형 검증

### 9.1 문서/Tool contract

- load/save round trip과 canonical revision
- move 뒤 slot ID 유지
- distinct slot의 duplicate pattern 허용
- duplicate slot ID, unknown pattern, overflow, wrong schema/version/property 거부
- stale baseline save가 기존 disk와 draft를 보존
- failed reload가 admitted draft를 보존
- Boss Verification 탭에 기존 action/list/right diagnostics marker가 모두 남음
- Pattern Flow inventory가 `m_AuditionInventory`만 소비하고 graph/sequence를 직접 순회하지 않음
- isolated preview가 기존 audition `Submit`을 사용하고 Flow start가 단일 Submit loop를 만들지 않음
- 신규 C++/Data가 project/filter에 정확히 한 번 등록됨

### 9.2 network protocol

- 1/32-slot round trip
- empty/33/truncated/oversize/unknown state atomic reject
- stop epoch/flow identity round trip
- duplicate pattern ID round trip 허용, duplicate slot ID는 Server semantic reject

### 9.3 Server contract

1. 3-slot Flow를 slot 2부터 시작해 2/3 exact order로 실행하고 reset은 한 번뿐이다.
2. 같은 pattern ID를 다른 slot ID로 두 번 실행한다.
3. exact pursuit tick과 기존 chase를 쓰며 Product pattern이 끼지 않는다.
4. active stop은 current 완료 뒤, pursuit stop은 즉시 terminal hold다.
5. death 동안 stage/hit/cursor가 고정되고 revive 뒤 같은 slot이 한 번만 완료된다.
6. natural terminal 뒤 수십 tick에도 Product sequence가 재개되지 않는다.
7. Pattern 사이 boss/player/world state가 reset되지 않는다.
8. invalid request는 boss/player/arena/world state를 전혀 바꾸지 않는다.
9. global catalog generation이 바뀌어도 시작 pinned revision으로 완료한다.
10. Flow 없는 Product ordered sequence와 기존 single `PLAY_PATTERN_ID` regression이 그대로 통과한다.
11. owner leave, duplicate/stale request, Release request를 명시적으로 닫는다.

### 9.4 All Effects 회귀 방지

다음 focused contract를 변경 전후 함께 실행한다.

```powershell
python -m unittest `
  Tools.ValtanPipeline.test_valtan_pattern_tree_contract `
  Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract `
  Tools.ValtanPipeline.test_valtan_boss_tool_contract `
  Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract
```

또한 no-touch 세 파일의 시작 diff와 종료 diff가 동일한지 비교한다.

### 9.5 build/regression

```text
NetworkProtocolHarness Debug/Release + 실행
Server Debug/Release + Server.exe --contract-test
Client Debug/Release
관련 JSON parse / PowerShell AST / git diff --check
```

Client와 UI는 에이전트가 실행·조작하지 않는다. 빌드 뒤 사용자가 다음을 직접 확인한다.

1. `F1 -> Effect Tool -> All Effects -> Valtan` tree가 28개와 Stage를 정상 표시한다.
2. `F1 -> Boss Tool -> Boss Verification`의 기존 단일 재생과 우측 진단이 변하지 않았다.
3. `Pattern Flow`에서 저장/reload, duplicate pattern, move, selected suffix start가 동작한다.
4. Flow 시작 시 중앙 reset은 한 번이고 이후 Pattern 상호작용·벽/HP 상태가 순서 사이 유지된다.
5. 죽으면 멈추고 기존 `Revive Player` 뒤 같은 slot부터 이어진다.

자동 검증은 animation/Effect의 최종 visual fidelity를 대신 판정하지 않는다.
