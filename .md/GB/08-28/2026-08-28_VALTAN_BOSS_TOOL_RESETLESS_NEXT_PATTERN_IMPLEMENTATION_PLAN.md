# Valtan Boss Tool 무리셋 Next Pattern·독립 도넛 구현 계획

> 2026-08-28 후속 구현은 G11~G14를 따른다. G11의 live Next 및 Reload 재시작,
> G12의 저장 Flow→Product 기본 순서 연결이 최초 isolated-only / document-only 제한을 대체한다.
> G13은 Six Pizza 첫 착지, G14는 도끼 점프의 공중 대기 시간만 수정한다. 이전 구현 이력은 보존한다.

작성일: 2026-08-28
상태: **2026-08-28 사용자 승인 구현 및 최종 자동 검증 진행. main cd120501에서 분기한 codex/valtan-arena-navigation-next에 PR #251 a6871a2f를 통합한 뒤, 확정 변경을 Desktop의 codex/valtan-arena-next-desktop에 반영했다. 선행 worktree의 개별 빌드·Server 계약은 통과했고 최종 Desktop 전체 regression·다른 작업 통합·시각 확인 상태는 RESULT에 기록한다.**

최신 사용자 지시로 Next와 Trash 전원 포획 전멸을 함께 구현한다. 기존 보류 기록은 이번 승인으로 대체한다.
초기 구현 폴더는 `C:/w/valtan-arena-next`다. 원본 checkout의 미커밋 변경은 별도 보존 후 통합했으며
최종 작업·빌드·실행 기준은 `C:/Users/user/Desktop/LostArk/Framework.sln`이다. main에 직접 작업하지 않는다.
기준 main의 protocol 41을 확장한 현재 wire 계약은 42다. 실행·검증 완료 여부는 대응 RESULT가 소유한다.
최종 Desktop Debug/Release 전체 자동 검증과 사용자 시각 확인은 미완료다.
단계별 자동 검증은 재개했으며, 개별 성공과 최종 Desktop 전체 회귀 완료를 구분한다.

사용자가 실행 파일을 종료하고 남은 오류 수정과 전체 Debug/Release 자동 검증 완료를 요청한 현재 상태는
[통합 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_RESULT.md)에 기록했다.

## 0. 구현할 사용자 동작

`Pattern Flow` 패널에 `Add from All Effects...`와 같은 방식의 **`Next Pattern...`** 선택기를 추가한다.
전체 split-owned Server 패턴 목록에서 하나를 고르면 현재 isolated 패턴의 **다음 실행 1칸**으로 예약한다.
Ordered Slots의 선택·배열·저장 여부와 무관하다. 오른쪽 아래에는 **`예약된 다음 패턴`** 카드를 항상 둔다.

```text
Play Selected 또는 Preview Isolated로 A 시작 — 최초의 기존 isolated 준비는 유지
  → Next Pattern...에서 B 예약
  → A 정상 종료 + 마지막 타격/부착/월드 변경 commit
  → 다음 fixed tick에서 현재 위치·현재 맵 상태 그대로 B 시작
  → B 실행 중 C를 예약할 수 있음
```

예약/교체/취소/승격은 중앙 이동, 플레이어 시작 위치 이동, HP/자원/cooldown 초기화, 벽·바닥·prop 복구를
호출하지 않는다. 단, B 자체가 저작된 `MOVE_TO_ARENA_CENTER` 등을 갖는 것은 정상 gameplay다.
금지하는 것은 **Tool 연결 경계가 일괄 수행하는 준비/reset**이다.

이번 범위는 stable-ID isolated audition chain이다. Saved Flow 전체를 Product 전투 중간에 삽입하거나,
끝난 뒤 Product cursor로 복귀시키는 live overlay는 별도 기능이다. 기존 `Start First / Start Here`의
의미를 바꾸지 않는다. `Next Pattern`을 그 버튼의 별칭으로 구현하지 않는다.

독립 도넛은 이미 있는 `VALTAN_FIST_IN_OUT / INNER`의 `animation.mode=NONE + STAGE_CLOCK` 계약을
그대로 Next 선택 대상에 포함한다. body clip에 종속되지 않는 새 효과 패턴도 같은 계약으로 확장한다.

연계 문서:

- [머지 후 시작 순서·보존 목록](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_NEXT_PATTERN_TRASH_WIPE_POST_MERGE_HANDOFF.md)
- [Trash 단일 패턴 통합·전멸 계획](C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_VALTAN_TRASH_CAPTURE_COUNTER_SERVER_FLOW_IMPLEMENTATION_PLAN.md)
- [사용자 Save JSON 검토 결과](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_SAVED_FLOW_JSON_REVIEW_RESULT.md)

## 1. 현재 코드에서 확인한 사실

### 1.1 실행·목록·문서 경계

| 실제 위치·기준 함수 | 변경 전 경계 | 현재 반영 상태 |
|---|---|---|
| [BossTool.cpp:437](C:/Users/user/Desktop/LostArk/Client/Private/BossTool.cpp:437) `Start_Flow` | clean 저장본 전체 슬롯·시작 slotId를 Flow service에 전달 | 기존 전달을 유지하고 Next 소유권과의 충돌 검사 |
| `CBossTool::Preview_SelectedFlowSlotIsolated` | `FLOW_PREVIEW_CONSUMER_ID`로 공용 audition service 호출 | 최초 A 실행 경로 유지 |
| `CBossTool::Submit_SelectedPattern` | `CONSUMER_ID`로 같은 service 호출 | 최초 A/명시적 새 isolated 실행 유지 |
| [ValtanPatternAuditionService.cpp:138](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternAuditionService.cpp:138) `Submit` | current snapshot 하나, `PLAY_PATTERN_ID` | current와 next·control request를 별도로 유지 |
| [GameRoom.cpp:6155](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:6155) `Evaluate_ValtanAudition` | bait 배치 → reset → `PendingPatternIds` | Next는 reset 분기 전에 `Evaluate_ValtanNextPatternControl`로 전달 |
| [GameRoom.cpp:4339](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:4339) `Refresh_ValtanPatternIdAuditionState` | 단일 occurrence 완료 뒤 hold·state clear | 정확한 종료 receipt와 완료 anchor를 확인한 뒤 Next 승격 |
| [ValtanBrain.cpp:1640](C:/Users/user/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1640) `FinishPattern` | 성공과 실패 모두 pattern ID를 비움 | 실제 `completed` 결과를 Server 내부 receipt에 기록 |
| [ValtanPatternTree.cpp:5537](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternTree.cpp:5537) `Build_ToolAuditionInventory` | Core 8 + Animator 20 = 28, 도넛 제외 | FBF 퇴출 통합 후 Core 8 + Animator 19 = 27; Next builder는 별도 사용 |
| `CBossTool::Find_Pattern` / PatternTree CPP 내부 `Find_Pattern` | joined Gimmicks/Rotation exact stable ID 조회 | UI는 BossTool의 기존 조회, builder는 CPP 내부 helper 사용 |

현재 Desktop의 generated Encounter에는 53개 정의가 있고, split gameplay가 직접 소유한 것은 29개다.
`전체 패턴`이라는 UI 문구는 이 작업에서 **검증된 split-owned 29개**를 뜻한다. legacy 정의까지 무조건
재수집하지 않는다. `bAuthoringMasterManaged`로 소유권을 검사하고 count는 runtime 목록 크기로 표시한다.
Trash 보조 세 정의를 보존한다. FBF 퇴출 통합에서는 `slot.000017`만 제거해 28슬롯/ordinal30을
유지했다. 이후 사용자가 저장한 22슬롯/ordinal31과 추가·삭제·재정렬을 그대로 보존한다.
Next의 항목 수는 split 정본에서 동적으로 얻으며 저장 Flow를 고정 개수로 되돌리지 않는다.

### 1.2 Reload Flow는 Server 재생 명령이 아니다

[BossTool.cpp:520](C:/Users/user/Desktop/LostArk/Client/Private/BossTool.cpp:520)의 `Reload_FlowDocument`는
`CValtanPatternFlowDocument::Reload`로 디스크 JSON을 parse/validate하여 draft만 교체한다.
dirty일 때 `Discard & Reload / Keep Draft` 확인을 받는다. reload 실패는 기존 유효 draft를 유지한다.
이 경로는 Server packet을 보내지 않고 발탄·플레이어·맵에 영향을 주지 않는다.

Next도 이 의미를 바꾸지 않는다. Save/Reload/Ordered Slots 편집은 이미 승인된 Next 예약을 취소하지 않는다.
Flow 문서가 손상되거나 미저장이어도 graph와 isolated chain이 유효하면 Next 선택은 가능하다.

### 1.3 기존 완료 판정과 tick 위치는 그대로 사용할 수 없다

`FinishPattern`은 `completed=false`와 `NO_VALID_TARGET`인 실패에서도 ID를 비운다.
따라서 `ACTIVE + 동일 sequence + 빈 pattern ID`만으로는 성공을 판정하지 않는다.
현재 Refresh는 같은 sequence의 `COMPLETED` receipt까지 요구한다.

변경 전 `Update_WorldEntities` 말미에 있던 Refresh 호출은 현재 `Tick`의 attachment 최종 pose,
encounter prop, world destruction commit과 `Drain_BossCombatEvents` 뒤로 이동했다.
이전 단계가 실패하면 Refresh와 Next 승격에 도달하지 않는다.

## 2. 완료 계약·범위

### 2.1 반드시 만족할 계약

1. 예약은 Server room이 소유하는 optional 1칸이다. FIFO 여러 칸이나 JSON 저장 큐를 만들지 않는다.
2. 정상 current A와 승인된 Next B를 동시에 관측한다. B 선택으로 A snapshot을 덮어쓰지 않는다.
3. current가 PENDING/ACTIVE이거나 정상 완료한 isolated hold일 때만 같은 owner가 예약할 수 있다.
4. B를 C로 바꾸는 것은 CAS 교체다. 거절되면 B가 남고, 오래된 취소는 C를 지울 수 없다.
5. A 실패/보스 사망/owner 이탈/world 변경은 Next를 취소한다. 빈 ID를 완료로 추정하지 않는다.
6. 전멸을 정상 수행한 A의 tail이 끝났지만 준비된 플레이어가 없으면 Next는 대기한다. 자동 부활하지 않는다.
7. A→B의 epoch는 같은 isolated chain, patternSequence는 서로 다른 occurrence다.
8. 단일 실행·예약·취소는 공용 service만 packet/result/lifecycle을 소유한다. Tool에서 직접 socket을 다루지 않는다.
9. Release Server는 명시적으로 거절한다. Release Tool UI smoke를 실행했다고 보고하지 않는다.
10. Boss Tool을 닫아도 예약은 유지된다. `CMainApp`의 공용 service update가 계속 수신한다.

### 2.2 이번에 하지 않는 일

- 저장된 Flow JSON 구조·slot ID 발급·배열 자동 마이그레이션.
- 공용 Tool/All Effects 27개 inventory를 Next의 split 29개 목록으로 교체.
- Product live overlay, 여러 Next queue, 자동 예약 저장/재시작 복원.
- 새 보스 AI, Client local damage, 별도 Effect 재생기, 별도 Level.
- 도넛용 새 mesh/texture 생성, half-donut UV 작업, 기존 도넛 수치 재튜닝.

## G00. 머지 기준선과 작업 준비

**책임:** 선행 작업을 잃지 않고 현재 source contract 위에 구현한다.

초안 조사 기준은 HEAD `d77d7e021edefc435119b23f9aab53315e3f870a`였으며, 분기 기준은 main `cd120501da95204219a696efad1aad2034e1808b`다.
선행 구현 worktree에는 PR #251의 `a6871a2f`도 통합했다. 현재 최종 Desktop 브랜치 역시 이 main을 기준으로 한다.
본문 line number는 탐색 힌트이며, 머지 후에는 함수 이름·정본 키로 다시 찾는다. 다른 세션에서 작업 중인
Boss→Effect exact tuple handoff, Product Effect unlink, NetworkManager endpoint 변경은 통째로 덮어쓰지 않는다.

1. AGENTS/CLAUDE/gotchas와 연결 PLAN/RESULT를 읽는다.
2. LAN sync, `git status --short`, `git fetch`, 현재 branch/HEAD/main 차이를 확인한다.
3. 선행 worktree의 확정 변경과 Desktop 원본 보존 여부를 확인하고, `codex/valtan-arena-next-desktop`의 `C:/Users/user/Desktop/LostArk/Framework.sln`을 최종 기준으로 사용한다.
4. 사용자 Flow JSON과 unlink된 FOUR_SLASH/RECOVERY cue를 보존 대상으로 기록한다.
5. 같은 기능의 Next/terminal receipt가 선행 머지로 이미 생겼으면 중복 구현하지 않고 이 계약과 대조한다.

최종 코드·데이터 대조와 검증은 Desktop에서 수행한다. 기존 사용자 변경을 보존하며 dirty checkout에서 자동 stage/commit/merge하지 않는다.

## G01. Next 목록을 기존 슬롯 inventory와 분리

### G01-H. `Client/Public/ValtanPatternTree.h`, `Client/Public/BossTool.h`

`CValtanPatternTree::Build_ToolAuditionInventory` 선언 바로 아래에 다음 선언이 반영돼 있다.

```cpp
static bool_t Build_NextPatternInventory(
    const VALTAN_PATTERN_TREE_VIEW& View,
    std::vector<std::string>& OutPatternIds,
    std::string& strOutError);
```

소비자는 Boss Tool의 Next popup 하나다. 기존 inventory나 Flow document는 이 함수를 호출하지 않는다.
출력은 stable pattern ID vector이며 pointer·현재 list index를 저장 ID로 사용하지 않는다.

Boss Tool의 기존 inventory/cache 멤버 근처에 `m_NextPatternIds`와 Next 검색 buffer가 반영돼 있다.
검색 buffer는 `std::array<char_t, 128u> m_NextPatternSearch{}`로 패널 수명만 갖는다.
예약된 patternId·owner·epoch는 이 UI 멤버에 저장하지 않고 service snapshot에서 읽는다.

### G01-CPP. `Client/Private/ValtanPatternTree.cpp`

**한 문장 책임:** strict-joined graph의 split-owned 패턴만 안정적인 순서로 반환한다.

호출 흐름: `CBossTool::Reload_Graph`의 stage 성공 → 새 inventory builder → 유효하면 UI cache commit.

1. `View.Gimmicks`와 `View.Rotation`에서 `bAuthoringMasterManaged=true`만 수집한다.
2. ID empty/duplicate, exact lookup 실패, stage/entry action 불일치를 실패로 처리한다.
3. `iSourceSequenceIndex`, stable patternId 순으로 결정적 정렬한다.
4. staged vector가 완성된 뒤에만 Out에 move한다. 실패 시 기존 Out을 부분 교체하지 않는다.
5. `Legacy`를 fallback으로 붙이거나 `Find_AuditionPattern`으로 다시 공용 Tool 27개 admission을 걸지 않는다.

새 count는 UI vector의 size다. 테스트에서 현재 source ID 집합 29개와 일치함을 확인하되 production에
고정 문자열 배열이나 29라는 count assertion을 새 정본으로 만들지 않는다.

### G01-UI 목록 의미

한국어 displayName을 먼저, stable ID는 tooltip/검색 대상으로 보여 준다. `VALTAN_FIST_IN_OUT`에는
`독립 효과 · 애니메이션 없음`을 표시한다. Trash 보조 3개는 수동 호환 audition임을 구분한다.
이 구분은 삭제·alias가 아니다. 보조 행을 선택하면 기존의 해당 stable ID만 명시적으로 예약한다.
본체 통합 재생은 `VALTAN_TRASH`를 선택한다.

## G02. 기존 audition wire family 확장

### G02-H. `Shared/Public/Network/PacketMessages.h`

기존 enum 숫자를 보존하고 `VALTAN_AUDITION_OPERATION::END` 직전에 다음 두 값이 반영돼 있다.

```cpp
QUEUE_NEXT_PATTERN_ID,
CLEAR_NEXT_PATTERN_ID,
```

`C2S_VALTAN_AUDITION_REQUEST`와 `S2C_VALTAN_AUDITION_RESULT`의 stable string 뒤에 다음 필드를
반영했다. 구조체에는 항상 있지만 wire에는 새 두 operation에서만 쓴다.

```cpp
std::uint32_t iPredecessorRoomAuditionEpoch = 0u;
std::uint32_t iPredecessorPatternSequence = 0u;
std::uint32_t iExpectedNextRequestSequence = 0u;
```

| 필드 | 발급자·의미·불변식 |
|---|---|
| `iRequestSequence` | 공용 Client service가 발급하는 command identity; Queue/Clear도 같은 service counter 사용 |
| `strBossPlacementId` | 최초 isolated 실행이 승인한 stable boss placement; vector index 아님 |
| `strPatternId` | Queue는 후보, Clear는 취소하려는 승인된 후보의 exact ID |
| predecessor epoch | Server가 최초 resetful Play에서 발급한 chain epoch; 0 금지 |
| predecessor sequence | 이 예약이 붙는 정확한 A occurrence; 0 금지 |
| expected next request sequence | 빈 슬롯에 예약할 때 0; 교체·취소할 때 현재 승인된 B request sequence |
| `iTargetHealthBar` | 새 두 operation에서는 반드시 0 |

Clear는 expected-next=0 또는 empty patternId를 거절한다. Queue는 A와 같은 patternId도 허용한다.
`A→A`여도 occurrence sequence가 달라야 한다. uint32 발급 공간 소진은 silent wrap/reuse하지 않고 명시적으로
거절하여 새 session/chain 재설정을 요구한다.

결과 enum의 기존 값을 유지하고 `END` 전에 다음 verdict를 반영했다.

```cpp
CLEARED,
REJECTED_STALE_REQUEST,
REJECTED_STALE_AUDITION,
REJECTED_NOT_OWNER,
REJECTED_NEXT_CHANGED,
```

Queue 성공은 기존 `QUEUED`를 사용한다. 거절은 위 값 또는 기존 world/boss/release/unavailable verdict다.
Next에서 `DUPLICATE_IGNORED`를 새 성공으로 해석하지 않는다. exact 재전송은 최초 verdict 자체를 재전송한다.

Lifecycle enum에는 `NEXT_RESERVED`, `WAITING_FOR_PLAYER`가 반영돼 있다. 기존 lifecycle의
request sequence, epoch, pattern sequence, patternId, pinned revision, reason으로 current/next를 구분한다.
새 packet type이나 ServerApp/ROOM_COMMAND type은 만들지 않는다.

### G02-CPP. `Shared/Private/Network/PacketMessages.cpp`

**책임:** operation별 wire shape와 필드 의미를 모두 검사하고 parse 성공시에만 destination에 commit한다.

| operation | request payload |
|---|---|
| 기존 legacy | 현재 `sequence + operation + targetHealthBar` 그대로 |
| PLAY_PATTERN_ID | 위 prefix + boss string + pattern string, 기존 그대로 |
| Queue/Clear Next | 위 prefix + boss string + pattern string + 신규 u32 세 개 |

Result도 기존 prefix 뒤의 strings/세 u32를 같은 조건으로 encode/decode한다. 기존 operation의 새 필드가
0이 아니면 Writer가 거절한다. 전송되지 않는 숨은 값을 허용하지 않는다. stable ID 한도는 기존
`MAX_STABLE_NETWORK_ID_BYTES=128`을 재사용하고 path/잘못된 ID/unknown enum/truncation을 거절한다.
Reader는 `decoded` 임시 객체로 검사한 후 한 번 대입한다.

기준 main의 `Shared/Public/Network/PacketType.h` protocol은 41이었으며, 현재 Desktop의 Next 계약은 42를 사용한다.
초안의 39 → 40 표기는 폐기하며, 기존 파티와 벽 파괴 확장 계약을 그대로 포함한다.

### G02-NET. `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`

기존 `Send_ValtanPatternAuditionById` 바로 아래에 다음 typed send가 반영돼 있다.

```cpp
bool Send_ValtanNextPatternCommand(
    const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& message);
```

공용 service의 `Send_Request`가 predecessor와 CAS를 포함한 전체 request를 전달하고,
NetworkManager만 packet을 작성한다. Boss Tool은 이 함수를 직접 호출하지 않는다.
result routing은 `PLAY_PATTERN_ID`와 Queue/Clear를 같은 전용 queue로 보내며,
service는 `Try_Consume_ValtanPatternAuditionByIdResult`와 `Try_Consume_ValtanAuditionLifecycle`로 소비한다.
legacy Level consumer가 새 결과를 빼앗지 않는 계약을 관련 회귀가 검사한다.

ServerApp의 기존 `C2S_VALTAN_AUDITION_REQUEST` parse → `ROOM_COMMAND::ValtanAudition` 전달은 재사용한다.
새 packet handler를 복제하지 않는다.

## G03. Server 예약 소유권·CAS·정상 완료 receipt

### G03-H1. `Server/Public/GameRoom.h`

`VALTAN_PATTERN_ID_AUDITION_STATE` 바로 뒤에 아래 private Server 상태가 반영돼 있다.

```cpp
struct VALTAN_NEXT_PATTERN_RESERVATION final
{
    SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
    LostArk::Shared::NET_ENTITY_ID iBossEntityId =
        LostArk::Shared::INVALID_NET_ENTITY_ID;
    std::uint32_t iRequestSequence = 0u;
    std::uint32_t iRoomAuditionEpoch = 0u;
    std::uint32_t iPredecessorPatternSequence = 0u;
    std::uint32_t iExpectedPatternSequence = 0u;
    std::string strBossPlacementId;
    std::string strPatternId;
    LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
    bool bReportedWaitingForPlayer = false;
};

struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
{
    LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
    LostArk::Shared::VALTAN_AUDITION_RESULT Result =
        LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
    std::uint32_t iCurrentHealthBar = 0u;
};
```

Room은 `std::optional<VALTAN_NEXT_PATTERN_RESERVATION>` 하나와 session별 최신 command receipt map을 소유한다.
빈 문자열을 예약 없음 sentinel로 쓰지 않는다. `Request`는 full payload fingerprint 비교의 정본이다.
receipt map은 기존 session leave/room reset 경계에서 제거하여 무제한 이력을 쌓지 않는다.

| 상태 필드 | 소유·수명 |
|---|---|
| owner session | queue 권한; 다른 player가 움직이거나 잡혔다고 바뀌지 않음 |
| boss entity + placement | 같은 placement가 despawn/recreate된 경우를 구분 |
| predecessor seq | A identity, 승격 직전까지 고정 |
| expected seq | B가 실제 시작할 때의 다음 occurrence; A와 다름 |
| pinned revision | 수락 시 A와 같은 immutable catalog; 대기 중 최신 revision으로 치환 금지 |
| reservation request seq | CAS token 겸 B lifecycle request identity |
| `bReportedWaitingForPlayer` | 승격 전 WAITING_FOR_PLAYER edge를 한 번만 발행하는 예약 상태 |

current phase의 `COMPLETED_HOLD`는 A 정상 완료 identity와 pin을 보존하는 상태이며 완료
lifecycle을 매 tick 재전송하지 않는다. Next가 없어도 같은 owner가 이후 Next를 예약할 수 있다.
단, `COMPLETED_HOLD + Next 없음 + 미확정 command 없음`은 새 resetful Play/Flow Start를 막지 않는다.
명시적 새 실행이 승인되면 이전 anchor를 폐기하고 새 epoch를 발급한다.

### G03-H2. `Server/Public/ServerWorldEntity.h`

`SERVER_ENTITY_ACTION` 뒤, `SERVER_WORLD_ENTITY` 앞에 아래 **Server 내부 종료 증거**가 반영돼 있다.

```cpp
enum class SERVER_BOSS_PATTERN_TERMINAL_RESULT : std::uint8_t
{
    NONE,
    COMPLETED,
    ABORTED
};

struct SERVER_BOSS_PATTERN_TERMINAL_RECEIPT final
{
    std::uint32_t iPatternSequence = 0u;
    SERVER_BOSS_PATTERN_TERMINAL_RESULT eResult =
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::NONE;
};
```

`SERVER_WORLD_ENTITY`가 receipt 하나를 갖는다. snapshot wire에 추가하지 않는다.
`BeginPattern`에서 초기화, `FinishPattern`에서 실제 실행 ID가 있는 경우에만 sequence와 completed 인자를 기록한다.
별도 boss death/강제 clear는 ABORTED다. Brain rollback의 `bossBeforeBrain` 사본에도 포함되므로 commit에
실패한 tick의 성공 receipt가 남지 않아야 한다.

### G03-CPP. `Server/Private/GameRoom.cpp`

**`Evaluate_ValtanNextPatternControl` 책임:** 예약 1칸만 원자적으로 바꾼다.

`Evaluate_ValtanAudition`은 current-audition conflict/reset 검사 전에 새 operation을
`Evaluate_ValtanNextPatternControl`로 분기한다. Play/Flow의 conflict 검사는 부작용 있는 Refresh 대신
순수 `Is_ValtanPatternIdAuditionRunning` predicate를 사용한다.

검사 순서는 다음과 같다.

1. Debug/world/session gate. Release는 명시적으로 reject하고 상태를 바꾸지 않는다.
2. 해당 session 최신 receipt와 full request 비교. exact duplicate는 최초 verdict replay, 동일 sequence
   다른 payload/오래된 sequence는 STALE. 새로운 거절 verdict도 receipt로 보관한다.
3. boss 살아 있음, exact entity/placement, current chain 존재, owner 일치.
4. predecessor epoch/sequence 일치; PENDING/ACTIVE 또는 성공 완료 hold만 허용.
5. 기존 Flow/timeline/fight-page와 충돌 없음.
6. 현재 승인된 next request sequence와 expected CAS 일치.
7. Queue 대상 pattern을 **A pinned catalog**에서 exact resolve. Client graph나 latest catalog로 대체하지 않는다.
8. staged reservation/verdict/lifecycle을 준비한 뒤 한 번 commit한다. reject 시 기존 B 보존.

플레이어 HP/GRABBED/isCombatReady는 **예약 수락 조건이 아니다**. 포획·전멸 중에도 예약/취소할 수 있다.
준비된 플레이어 검사는 실제 승격 시점의 조건이다.

교체 성공은 old B에 ABORTED(`replaced`)를 한 번, C에 NEXT_RESERVED를 한 번 만든다. Clear 성공은
해당 B ABORTED(`cleared`)와 CLEARED verdict다. 실패한 C 요청은 B의 상태를 바꾸지 않는다.
`Queue_ValtanNextPatternLifecycle`은 reservation을 명시적으로 받고,
`Queue_ValtanPatternIdAuditionLifecycle`은 current state를 읽어 공통 `Queue_ValtanAuditionLifecycle`에
owner/request/epoch/sequence/pattern/pin을 전달한다.

## G04. fixed-tick 종료 뒤 승격

### G04-CPP. `CGameRoom::Tick`, `Update_WorldEntities`, `Try_PromoteValtanNextPattern`

**책임:** A의 정상 commit이 끝난 안전 경계에서 승인된 B를 기존 pending 실행 경로로 넘긴다.

`Refresh_ValtanPatternIdAuditionState`는 room `Tick`의 `Drain_BossCombatEvents()` 뒤,
Debug lifecycle flush 직전에 호출한다. `Update_WorldEntities` 말미에서는 호출하지 않는다.

`Commit_DueEncounterProps`는 READY transaction의 Commit 실패와 invalid Prepare를 false로 전달한다.
`Tick`은 이 반환값과 `Commit_DueWorldDestruction`의 성공을 확인하고, 실패하면 room을
not-ready로 두고 종료한다. 정상 no-op은 성공으로 유지한다.

```text
Update_WorldEntities
  → A Brain 결과/EXIT/ENTER/player hit·capture commit
  → grabbed attachment 최종 pose refresh
  → Commit_DueEncounterProps
  → Commit_DueWorldDestruction
  → Drain_BossCombatEvents
  → Refresh_ValtanPatternIdAuditionState
      → exact terminal receipt COMPLETED 확인
      → A COMPLETED 1회 / COMPLETED_HOLD
      → Try_PromoteValtanNextPattern
  → lifecycle flush + snapshot
다음 tick
  → Prepare_ValtanPatternIdAuditionBeforeBrain에서 identity/target 확인
  → 기존 SelectPattern의 forced audition 경로로 B 시작
```

세계 commit 실패로 room not-ready가 되면 Refresh/승격/성공 snapshot에 도달하지 않는다.

`Try_PromoteValtanNextPattern`은 다음 순서로 동작한다.

1. Next 존재, A COMPLETED_HOLD, exact predecessor receipt COMPLETED 확인.
2. owner/boss/pin/target pattern/expected sequence를 다시 확인한다.
3. 기존 engage 범위·living combat-ready target 조건을 평가한다. 없으면 hold를 유지하고
   WAITING_FOR_PLAYER를 상태 edge에서 한 번 보낸다. 반복 로그나 15초 timeout을 만들지 않는다.
4. current A anchor를 B의 PENDING state로 교체하고, B request identity·같은 epoch·같은 pin·다음 sequence를
   설정한다. `PendingPatternIds`에 B를 정확히 한 번 추가한다.
5. audition override는 유지하고 hold만 해제한다. Product 자동 selector/pursuit가 끼어들지 않는다.
6. reservation은 이 승격 transaction에서 소비된다. Client는 B ACTIVE를 받은 뒤 카드의 대상을 current로 옮긴다.

승격 직후 다음 tick 전 target이 사라지는 race는 `Prepare_ValtanPatternIdAuditionBeforeBrain`이 처리한다.
B가 아직 BeginPattern되지 않은 경우 등록된 B pending과 pin을 보존하고 hold/WAITING_FOR_PLAYER로 돌아간다.
Product로 fall-through하거나,
pending ID 삭제를 B 완료로 오판하지 않는다. 이미 B ACTIVE 뒤의 실패는 정상 ABORTED 정책이다.

B PENDING은 예약이 이미 소비됐다는 receipt다. 이후 WAITING_FOR_PLAYER가 오면 Client는 대기 상태를
표시하되 `bReservationConsumed`를 유지하여 취소·교체 packet을 보내지 않는다. A 완료 이전 또는
승격 전 WAITING_FOR_PLAYER의 승인된 예약은 계속 취소·교체할 수 있다. B ACTIVE 이후 새 C 예약을 허용한다.

이 함수가 호출해서는 안 되는 경로:

- `Place_PlayerAtValtanAuditionBait`
- `Build_ValtanBossOnlyAuditionReset`, `Reset_ValtanBossOnlyAuditionState`
- `Reset_ValtanAuditionState`, `Reset_ValtanArenaState`
- destruction/prop/navigation/collision/global combat-object reset

### G04-수명·정리

`Build_RequiredPinnedGameplayRevisions`에는 current completed hold 및 Next pin을 포함한다.
`Resolve_ValtanGameplayCatalog`도 이 chain의 pin을 사용한다. R2 publish/activation이 있어도 이미 예약한 B는 R1이다.

`Cancel_ValtanNextPatternReservation`은 owner leave에서 직접 호출하며, current abort,
boss death/despawn과 명시적 authoritative reset은 `Cancel_ValtanPatternIdAudition`을 거쳐 예약을 취소한다.
예약이 있을 때만 ABORTED edge를 발행하고 비우므로 반복 호출로 같은 edge를 만들지 않는다.
이 취소를 위해 맵 reset을 새로 호출하지 않는다. owner가 떠나도 다른 참가자가 남으면
현재 A는 정상 종료까지 진행할 수 있으나 Next는 실행하지 않고 terminal hold한다. 기존 마지막 player
이탈 시 room 정리 계약은 유지한다.

`COMPLETED_HOLD`의 owner가 사라지면 late Queue를 거절한다. 새 isolated Play는 기존 준비 절차로 새 chain을 만든다.
Map/Level 이동과 Client inbound generation 변경도 이전 예약을 되살리지 못해야 한다.

## G05. Client 공용 service의 current·next·미확정 요청 분리

### G05-H. `Client/Public/ValtanPatternAuditionService.h`

**소유자:** `CValtanPatternAuditionService` singleton, Client main thread. Boss Tool과 Effect Tool은 읽기/명령 호출자다.

기존 current `VALTAN_PATTERN_AUDITION_SNAPSHOT`과 다음 두 상태를 별도로 유지한다.

| 상태 | 필수 내용 | 불변식 |
|---|---|---|
| `VALTAN_NEXT_PATTERN_SNAPSHOT` | 예약 상태, 소비 여부, request seq, Client inbound generation, Server epoch, predecessor seq, expected B seq, boss/pattern/consumer IDs, pinned revision, status | Server가 승인한 B만 표시; C 교체 요청을 보냈다는 이유로 먼저 C로 바꾸지 않음 |
| `VALTAN_NEXT_PATTERN_COMMAND` | full request, consumer ID, generation, `iSentAtMilliseconds`, WAITING_VERDICT 또는 UNCONFIRMED | 미확정 요청을 보내고 받는 동안 기존 승인 B와 병존 |

Next snapshot의 상태는 `NONE / RESERVED / WAITING_FOR_PLAYER / START_PENDING / REJECTED / ABORTED`로
표현한다. 종료 원인은 status에 남기되 REJECTED/ABORTED를 살아 있는 예약으로 판정하지 않는다.
교체 거절은 기존 B snapshot을 terminal로 바꾸지 않고 control status만 거절로 만든다.
이전 예약의 terminal event가 최신 승인 C를 덮어쓰지 않도록 exact identity로 걸러낸다.

기존 `Submit/Update/Get_Snapshot` 옆에 다음 public API가 반영돼 있다.

```cpp
bool Queue_NextPattern(
    std::string_view strConsumerId,
    std::string_view strBossPlacementId,
    std::string_view strPatternId,
    std::string& strOutStatus);
bool Clear_NextPattern(std::string& strOutStatus);
bool Retry_NextPatternCommand(std::string& strOutStatus);

[[nodiscard]] const VALTAN_NEXT_PATTERN_SNAPSHOT& Get_NextSnapshot() const
{
    return m_NextSnapshot;
}
[[nodiscard]] const VALTAN_NEXT_PATTERN_COMMAND& Get_NextCommand() const
{
    return m_NextCommand;
}
[[nodiscard]] bool Has_PendingNextCommand() const
{
    return m_NextCommand.Is_Pending();
}
[[nodiscard]] bool Has_PlaybackOwnership() const
{
    return m_Snapshot.Is_InFlight() || m_NextSnapshot.Is_Live() ||
        Has_PendingNextCommand();
}
```

`Queue_NextPattern`은 호출자가 전달한 `strBossPlacementId`가 current의 boss와 같은지 검사하고,
current의 epoch/sequence와 승인 Next의 CAS token으로 request를 만든다.
Boss Tool의 실제 호출은 `Service.Queue_NextPattern(CONSUMER_ID, BOSS_PLACEMENT_ID, PatternId, m_strNextPatternStatus)`다.
UI는 wire tuple을 조립하지 않는다. queue/clear/retry가 current A를 resetful Submit으로 다시 실행하지 않는다.

`Has_PlaybackOwnership`은 current in-flight, 승인 Next 대기/시작 중, 미확정 control을 포함한다.
`COMPLETED + Next 없음 + 미확정 없음`만 있으면 false다. 이 predicate를 Repeat·새 Play·Flow Start가 공유한다.
Queue 자체는 current in-flight여도 허용해야 하므로 Submit용 predicate를 그대로 Queue 거절 조건으로 쓰지 않는다.

### G05-CPP. `Client/Private/ValtanPatternAuditionService.cpp`

**책임:** Server verdict/lifecycle을 exact identity로 소비하여 UI에 권위 있는 current/next 상태를 제공한다.

`Update`는 result를 `Apply_ServerResult`로, lifecycle을 `Apply_ServerLifecycle`로 전달한다.
timeout은 `Update` 본문에서 `Now_Milliseconds`와 두 timestamp를 사용해 판정한다.
production과 실행형 harness는 이 동일한 구현을 사용한다.

1. disconnect/inbound generation 변경을 먼저 처리하고 current·Next·미확정 요청을 모두 종료한다.
2. 전용 result queue를 끝까지 drain한다. Play와 Queue/Clear를 operation으로 나누고 full echo를 검사한다.
3. Queue QUEUED verdict 또는 exact NEXT_RESERVED lifecycle로 B의 승인 identity를 확정한다.
4. lifecycle queue를 끝까지 drain한다. `Apply_ServerLifecycle`이 먼저 `Apply_NextLifecycle`을 호출한다.
   이 함수는 미확정 Queue candidate의 exact identity를 확인해 승인된 Next로 반영한 다음 Next snapshot에 적용한다.
   Next로 소비되지 않은 lifecycle만 current A의 identity와 대조한다.
5. A COMPLETED 뒤에도 같은 Update를 중단하지 않는다. 이어지는 B PENDING/ACTIVE를 소비한다.
6. B PENDING/WAITING은 카드를 유지한다. PENDING 이후 WAITING은 소비된 예약으로 표시하고 취소·교체를 잠근다.
   B ACTIVE에서 current를 B로 승격하고 그 exact Next를 비운다.
7. B가 C로 대체된 뒤 도착한 B ABORTED는 C와 current를 건드리지 않는다.
8. 이미 끝난 Client generation의 늦은 lifecycle은 새 epoch를 부여하거나 terminal state를 되살리지 못한다.

Lifecycle의 `iPatternSequence`는 NEXT_RESERVED/WAITING/PENDING/ACTIVE 모두 **B의 expected/actual sequence**다.
predecessor는 Client가 보존한 full request에서 가져온다. A와 B가 같은 patternId여도 request/sequence를 함께 비교한다.

HUD의 patternId 변화는 Next 완료/승격 증거가 아니다. 최초 Play의 기존 compatibility fallback은 유지할 수 있지만,
Next chain이 시작되면 authoritative lifecycle만 사용한다. `m_hasAuthoritativeLifecycle`을 B로 승격하면서
false로 되돌려 fallback이 잠깐 실행되는 경로를 만들지 않는다.

### G05-Timeout·재전송

| 상태 | timeout 정책 |
|---|---|
| 최초 resetful Play verdict / initial queued start | 현재 5초 / 15초 유지 |
| Queue/replace/Clear verdict | 5초 후 `승인 여부 미확정`; 기존 예약과 소유권 유지 |
| RESERVED / WAITING_FOR_PLAYER | start timeout 없음 |
| Next-origin START_PENDING | initial Play의 15초 timeout을 재사용하지 않음; Server 상태를 기다림 |

미확정 상태에서는 `같은 요청 재확인` 버튼으로 동일 sequence·동일 full payload를 재전송한다.
새 번호로 같은 Queue를 임의 반복하지 않는다. Server receipt replay가 최초 성공/거절을 그대로 돌려준다.
네트워크 단절/world 변경만 확실한 종료로 처리한다. timeout을 곧바로 `예약 없음`으로 표시하지 않는다.

### G05-Repeat·Flow 충돌

Queue 버튼을 누르면 `m_bRepeat=false`, `m_strRepeatPatternId.clear()`를 즉시 적용한다.
`CBossTool::Update`의 COMPLETED → repeat Submit 조건에서도 Next/미확정 command가 있으면 재제출하지 않는다.
UI checkbox만 disable하는 것으로 끝내지 않는다.

다음 실제 호출자 모두 current-only `Is_InFlight` 검사에서 새 소유권 predicate를 반영한다.

- Boss Tool `Start_Flow`, `Preview_SelectedFlowSlotIsolated`, `Render_ActionBar`, `Render_FlowSelectedSlot`.
- 공용 audition service의 `Submit`.
- `CValtanPatternFlowService::Start`의 진입 검증.

이는 Flow 파일 편집/Save/Reload 잠금이 아니다. 새 재생 소유권 충돌만 막는다.
`Stop After Current`는 해당 실행 방식의 기존 의미를 유지한다. Next 예약 취소는 카드의 명시적 `예약 취소`다.
Flow 중에는 Next 제출을 거절하고 `Flow 실행 중 — 종료 후 isolated 재생에서 예약` 이유를 보여 준다.

## G06. 오른쪽 아래 카드와 선택 popup

### G06-H. `Client/Public/BossTool.h`

`Render_PatternFlowTab` 관련 private 선언 근처에 다음 함수가 반영돼 있다.

```cpp
void Render_NextPatternCard();
void Render_NextPatternPicker();
```

`Render_NextPatternCard`는 service snapshot과 read-only graph만 읽는다. `m_FlowDocument`를 받지 않는다.
`Render_NextPatternPicker`는 stable ID를 service에 제출하는 UI다. Add_Slot/Save/Start_Flow를 호출하지 않는다.

### G06-CPP. `Client/Private/BossTool.cpp`

변경 전 `Render_PatternFlowTab`의 graph/Flow document unavailable return과
`Render_FlowSelectedSlot`의 null-flow return 때문에 함수 끝에 카드만 덧붙일 수 없었다.
현재 문서 영역의 오류는 해당 영역의 empty state로 처리하고, Next 카드는 별도 child로 렌더링한다.

```text
Pattern Flow
┌ Ordered Slots / Add from All Effects ┬ 선택 슬롯 / Preview Isolated ┐
│ 저장된 순서 또는 문서 오류·Reload     │ Flow Playback                │
│                                     │                              │
│                                     ├ 예약된 다음 패턴             │
│                                     │ 현재: A                       │
│                                     │ 다음: B · 서버 예약됨         │
│                                     │ [Next Pattern...] [예약 취소] │
└─────────────────────────────────────┴──────────────────────────────┘
```

오른쪽 위 선택/Flow 영역은 높이가 제한된 child, 아래 카드는 별도 child로 둔다. 충분한 높이에서는 카드를
오른쪽 아래에 고정하고, 작은 창에서는 겹치거나 잘리지 않도록 parent scroll로 접근 가능하게 한다.
이 도식은 배치 설계이지 Client 캡처나 visual PASS 증거가 아니다.

카드는 다음 상태를 구분한다.

| 표시 | 의미·가능 동작 |
|---|---|
| 예약 없음 | 현재 isolated chain이 있으면 Next 선택, 없으면 먼저 Play Selected/Preview Isolated 안내 |
| 서버에 예약 요청 중 | 현재 A와 승인된 B는 유지, 중복 Queue 차단 |
| 서버 예약됨 | B 한국어 이름; 선택기로 교체 또는 exact 예약 취소 |
| 플레이어 준비 대기 | B 유지; 자동 부활/이동 없음, 기존 Revive Player 동작 안내 |
| 다음 패턴 시작 대기 | B의 PENDING, 반복/다른 start 차단 |
| 승인 여부 미확정 | `같은 요청 재확인`; 이전 승인 예약은 계속 표시 |
| 예약 거절/취소/중단 | 구체적 이유; 기존 B가 있으면 B를 그대로 표시 |

Graph reload 실패여도 카드의 current/next 텍스트와 exact 취소/재전송은 표시한다.
새 선택 목록만 disable하고 `Graph를 다시 읽어 주세요` 이유를 둔다. Flow 문서 오류는 Next 버튼 조건에 넣지 않는다.
같은 PC의 Effect Tool에서 시작한 isolated audition도 공용 service identity가 있으면 같은 owner의 chain이다.
다른 Client가 시작한 audition은 소유권을 추정해 가져오지 않는다.

Picker는 `Next Pattern...` 클릭 → 검색 입력과 전체 split-owned 목록 → 선택 행 클릭으로 제출한다.
선택된 row는 Ordered Slot을 바꾸지 않는다. 빈 검색은 모든 admitted 후보를 보여 준다. 슬롯 ordinal/flow revision은
표시/전송하지 않으며, queue 수락 시 pin과 실제 예약 결과는 카드에서 확인한다.

WIP `Open Editor` exact product handoff 버튼과 tuple 조회 코드는 보존한다. 이번 변경이 그 tuple을
`patternId` 단독 검색으로 축소하거나 Effect Tool preview를 자동 재생하게 만들면 안 된다.

## G07. 도넛을 애니메이션 독립 패턴으로 연결

### G07-현재 정본과 의미

| 정본 | 현재 값 |
|---|---|
| [Valtan.gameplay.json:825](C:/Users/user/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:825) | `VALTAN_FIST_IN_OUT`, entry `valtan.attack.fist-in-out.inner`, INNER 한 stage 2600ms |
| hit | `RING`, inner 8m / outer 16m, 1600ms에 1회, `damage.valtan.fist-in-out` |
| [Valtan.presentation.json:669](C:/Users/user/Desktop/LostArk/Data/Valtan/Valtan.presentation.json:669) | `animation: { mode: NONE }` |
| effect cue | `cue.valtan.carrier-v1.attack.fist-in-out.inner.clip-01` |
| effect asset | `effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01` |
| timing | `STAGE_CLOCK`, stageOffsetMs 0, repeat once |
| anchor/lifetime | root / snapshot / natural |
| scale | GAMEPLAY_FOOTPRINT, worldScale `[1.5,1.5,1.5]` |
| independent row | `valtan.independent-effect.donut-in-out`, ownership SERVER_PATTERN_STAGE |

현재 `CValtan::Apply_PatternPresentationSample`은 present-empty binding을 NONE으로 인식하고 직전 pose를
유지하도록 animation을 pause한다. 단순히 clip을 못 찾은 것과 다르다. 다음 animation stage에서는 pause를 해제한다.
`Load_PatternEffectCues`와 `Spawn_DuePatternEffectCues`는 이 empty binding에서 Server action age로
STAGE_CLOCK cue를 실행한다. 효과 시작에 body animation notify가 필요하지 않다.

따라서 이 G의 production 변경은 **목록 연결과 진단/회귀 보강**이다. 동일한 기능을 다시 만드는 JSON
`independent=true`, local Effect-only boss, 새 timed-spawn manager는 추가하지 않는다.

### G07-독립성의 정확한 범위

- 애니메이션과 독립: clip/notify 종료를 기다리지 않고 Server stage clock으로 재생한다.
- Server 패턴과 독립은 아님: 시작 identity·damage timing·stage 종료는 기존 Server 패턴이 소유한다.
- snapshot anchor: 시작 위치를 잡고 이후 boss 이동에 따라 Effect를 다시 중앙으로 옮기지 않는다.
- natural stop: 기존 active natural Effect는 다음 action으로 넘어가도 자신의 수명까지 남을 수 있다.
- boss death/despawn/level teardown: 기존 owner cleanup을 따른다.
- hit authority: Client Effect 모양·지속 시간은 damage 판정자가 아니다.

Next A→도넛→C에서 기대하는 것은 `A가 끝난 현재 위치에서 독립 도넛 stage가 시작`하는 것이다.
원본 fixture의 오래된 WINDUP/OUTER/RECOVERY body chain을 복원하거나 19_02/19_04를 붙이지 않는다.
generated binding의 retired NONE tombstone은 ordinal 호환을 위해 유지한다.

### G07-새 효과 패턴으로 확장하는 절차

1. split gameplay에 stable patternId, stageId/actionId, duration, 필요하면 Server hit를 저작한다.
2. 같은 identity의 presentation stage를 `animation.mode=NONE`으로 둔다.
3. EffectCatalog에 이미 admission된 asset ID를 `timingBasis=STAGE_CLOCK`, `stageOffsetMs < durationMs`,
   `repeatPolicy=once`로 연결한다. clipOccurrenceId/sourceStartMs/sourceEndMs를 섞지 않는다.
4. root/snapshot 또는 허용된 기존 anchor/follow/stop 계약을 명시한다.
5. 기존 joined projector로 publish/validate하고 Next inventory가 동적으로 수집하는지 검사한다.

현재 validator는 `STAGE_CLOCK + CLIP_SEQUENCE` 혼합을 거절한다. 이번 요구는 NONE variant로 닫히므로
혼합 허용이나 clip 기반 cue의 의미 변경은 하지 않는다. 기존 Effect Tool independent row도 보존한다.

### G07-필요 resource와 Git 경계

현재 asset의 실제 authoring document는
[effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.effect.json](C:/Users/user/Desktop/LostArk/Data/Effects/Authored/effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.effect.json)이다.
새 binary가 필요하지 않다. 현재 직접 소비하는 resource 상대 ID는 다음과 같다.

```text
Effect/Valtan/Textures/FX_TEX_01/fx_c_ring_002.dds
Effect/Valtan/Textures/FX_TEX_03/fx_e_ring_001_cl.dds
Effect/Valtan/Textures/FX_TEX_00/fx_b_ring_001.dds
Effect/Valtan/Textures/FX_H_W_01/fx_h_wave_04.dds
Effect/Valtan/Textures/FX_TEX_01/fx_c_noise_005.dds
Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_026.dds
```

물리 위치는 `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/` 아래 동일 상대 경로다.
compilerAssetId 참고 값과 실제 `elements.resources[].assetId` 소비 closure를 혼동하지 않는다.
이번 PR은 새 resource closure를 추가하지 않으며 기존 팀장 관리 Resources를 사용한다. 각 파일의 현재 추적 여부는
handoff 시 `git ls-files`로 확인한다. Git pull만으로 binary 재현을 추가 보장한다고 보고하지 않는다.

## G08. 실행형 검증과 project 등록

### G08-1. Shared protocol harness

기존 [NetworkProtocolHarness.cpp](C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp)의
`Test_ValtanAuditionProtocol` 옆의 `Test_ValtanNextPatternProtocol`과 main 호출이 반영돼 있다.

필수 fixture:

- Queue/Clear roundtrip, RESERVED/WAITING lifecycle roundtrip.
- legacy/PLAY_PATTERN_ID golden bytes 불변.
- old operation의 숨은 신규 field, zero predecessor, Clear token 0, unknown ID/op/result/state 거절.
- 두 string 및 각 u32 위치에서 truncated payload 거절.
- parse 실패시 destination 불변, 과대 길이/잘못된 stable ID 거절.
- operation에 맞지 않는 결과 조합 거절.

### G08-2. Server contract

`ServerGameplayContractTests.cpp`의 stable-ID ownership/lifecycle 테스트 옆에 같은 room fixture를 확장한다.
실제 `FinishPattern`과 fixed tick을 사용하고 ID를 임의 clear하여 성공 완료를 흉내 내지 않는다.

| fixture | 통과 조건 |
|---|---|
| A ACTIVE 중 B Queue | evaluator 전후 boss/player/world/prop/combat-object semantic state 동일 |
| A 마지막 hit/world event와 B | commit이 선행, B start는 terminal tick 뒤의 tick, exactly once |
| A→A | 같은 ID지만 서로 다른 request/sequence, 한 번씩 실행 |
| B→C 교체 성공/거절 | 성공시 B abort+C reserved, 거절시 B 그대로 |
| stale Clear / duplicate / rejected duplicate | 새 예약 보존, 최초 verdict replay, 중복 enqueue 0 |
| current NO_VALID_TARGET abort / receipt 없는 clear | B 시작 0 |
| world commit 실패 / stage rollback | 성공 lifecycle·B 승격·partial snapshot 0 |
| boss death / owner leave / world reset | Next 취소, reset을 새로 호출하지 않음 |
| all-grab execution tail 완료 | 정상 A 완료 후 B WAITING, 사용자 revive 뒤 B 1회 |
| promotion 뒤 target 소실 | pending B 보존, Product fallback 0 |
| pin R1에서 R2 activation | B는 R1 유지, pin 조기 GC 없음 |
| 완료 hold에서 늦게 Queue | 재배치 없이 다음 안전 tick에 실행 |
| Release | 명시적 거절, 상태 불변 |

`memcmp`로 C++ struct 전체를 비교하지 않는다. padding/container를 제외한 명시적 semantic fields 또는
기존 snapshot을 비교한다. A→B 동안 HP/cooldown 전체가 불변이라고 검사하지 않는다. A 마지막 피해와
cooldown 정상 감소는 허용하고, **reset/teleport/sequence rewind가 없다는 것**을 검증한다.

### G08-3. 실제 Client service 실행형 harness 추가

현재 `LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS` 후크를 사용하는 독립 프로젝트가 반영돼 있다.
정적 문자열 테스트만으로 lifecycle 경합이 해결됐다고 판정하지 않으며 실제 실행 결과는 RESULT에 기록한다.

반영한 파일은 다음 세 개다. 모두 harness에 실제 소비자가 있다.

```text
Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp
Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
```

프로젝트는 x64 Debug/Release, Console, v143, C++20, PCH 미사용, `/utf-8`로 구성한다. 현재
ActionPresentationTimelineHarness처럼 production CPP를 link item으로 직접 컴파일하되 복사본을 만들지 않는다.
`Client/Private/ValtanPatternAuditionService.cpp`와 harness main이 compile item이며 Shared project를 reference한다.
include는 Client/Public, Shared/Public을 사용한다. service의 scalar 타입은 표준 bool/char로 두어
EngineSDK/renderer 의존성을 제거한다. runtime socket/renderer/Client.exe는 실행하지 않는다.

service CPP는 NetworkManager/HUD/clock 호출을 transport seam으로 분리한다. harness define에서는
결정적 clock, connected/generation, presentation revision availability, 수신 result/lifecycle queues,
send 성공/실패와 발신 request vector를 주입한다. **Submit/Queue/Clear/Retry/Apply_ServerResult/
Apply_ServerLifecycle/timeout/소유권 판정 본문은 production과 동일 코드**다. harness 전용 가짜 상태 머신은 금지한다.
`Harness_Reset`, `Harness_ObserveBoss`, `Harness_Input`, `Harness_SetNextRequestSequence`가
이 입력 seam을 제어하며 production build에서는 노출되지 않는다.

`Framework.sln`에는 새 프로젝트와 Debug/Release x64 configuration mapping이 등록돼 있으며,
`.filters`에는 실제 Private 소스·linked Client 소스와 header가 있다. 기존 filter는 재배치하지 않는다.
`Tools/Build/Invoke-BuildAndRegression.ps1`의 build와 실행 gate도 이 프로젝트를 소비한다.
Client 기존 service 파일은 이미 Client.vcxproj/.filters에 있으므로 중복 등록하지 않는다.

필수 입력 순서:

```text
A ACTIVE → B 요청 → A COMPLETED → B verdict → B PENDING → B ACTIVE
B verdict → A COMPLETED → B PENDING → B ACTIVE
B 승인 → C 교체 거절 → 늦은 B lifecycle
B 승인 → C 교체 성공 → 늦은 B ABORTED
Clear timeout → 동일 payload retry → 최초 verdict replay
generation 변경 → 이전 generation의 ACTIVE/COMPLETED 도착
```

모든 순서에서 최초 resetful Play 전송은 한 번이어야 한다. 이미 존재하는 승인 B가 거절/timeout으로 사라지거나,
종료된 chain이 late packet으로 되살아나면 실패다.

### G08-4. Flow 저장 회귀와 정적 Tool 검증

저장본/seed 분리는 사용자 추가 요청으로 먼저 구현했고 73 focused tests가 통과했다.
[실제 수정 결과 §8](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_SAVED_FLOW_JSON_REVIEW_RESULT.md)을
보존하며 Next 구현에서 다시 현재 JSON 순서에 고정하지 않는다. 아래 Next UI 관련 회귀도 기존 suite에 반영돼 있다.

[test_valtan_boss_tool_pattern_flow_contract.py:159](C:/Users/user/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py:159)의
editable JSON과 초기 seed의 assertion을 분리하여 다음 계약을 검사한다.

- 실제 저장본 테스트: schema/admission/unique slot/max32/monotonic ordinal만 검증.
- seed 테스트: 별도 in-memory seed fixture를 생성해 초기 inventory 순서 검증.
- mutation fixture: reorder, duplicate pattern, slot 삭제, 빈 draft, 새 ID 발급·Save/Reload roundtrip 검증.
- 사용자 현재 저장 Flow(현재 22슬롯/ordinal31)를 공용 Tool 27개 seed나 Next 29개 목록 순서로 강제 재작성하지 않음.

기존 boss_tool_contract/pattern_flow_contract/balance_tool_contract는 new UI/service route, Repeat guard,
Flow 오류 상태 카드 존속, Next→Add_Slot 미호출, Clear full tuple, Release gate를 검사한다.
기존 pattern_tree와 All Effects inventory 테스트는 FBF 퇴출을 반영한 27개 계약으로 유지한다.

도넛은 기존 `test_fist_in_out_projects_one_animation_free_stage_and_stage_clock_cue`와
`test_animation_none_and_stage_clock_cue_tagged_unions_fail_closed`를 유지하고 Next inventory 포함,
NONE pose 유지 뒤 다음 clip resume, duplicate cue occurrence 방지와 current anchor 사용을 보강한다.

## G09. 빌드·검증·인계

### 자동 검증

아래는 **최종 소스 고정 후 Desktop에서 실행할 명령**이다. 단계별 빌드·계약 검증 결과는 RESULT에 기록하며,
아직 완료하지 않은 Debug/Release 전체 회귀와 사용자 시각 검증을 이 계획서에서 PASS로 선언하지 않는다.

```powershell
Set-Location -LiteralPath 'C:/Users/user/Desktop/LostArk'
python -B -m unittest Tools.ValtanPipeline.test_valtan_boss_tool_contract Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Sync-EffectDataProject.ps1 -Check
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git diff --check
```

`C:/Users/user/Desktop/LostArk/Framework.sln`에 등록된 정본 build가 Engine → UpdateLib → Shared/ProtocolHarness → Server/contract → Client와 등록된 harness를
실제로 통과했는지 configuration별로 기록한다. JSON/XML parse, new project/filter registration도 검사한다.
이미 알려진 unrelated failure가 있으면 exact 이름·로그·범위를 분리하고 전체 PASS로 바꾸지 않는다.

### 사용자 직접 확인 절차

에이전트는 Client/UI를 자율 실행·조작·캡처하지 않는다. 준비된 Debug Server+Client를 사용자가 직접
`C:/Users/user/Desktop/LostArk/Framework.sln`에서 `Ctrl+F5`로 시작한다. Client 작업 디렉터리는 `Client/Default`다.

1. Lobby → Server 승인 Valtan → F1 → Boss Tool → Pattern Flow.
2. A를 Preview Isolated로 시작하고, 이동/파괴가 있는 A 도중 Next Pattern에서 B를 고른다.
3. 오른쪽 아래 current A / 예약 B / 서버 예약됨이 동시에 보이는지 확인한다.
4. A 종료 뒤 별도 중앙 이동·플레이어 이동·벽/바닥 복구 없이 B가 시작하는지 본다.
5. B를 교체·취소하고, Ordered Slots 선택·순서·Save/Reload와 서로 독립인지 확인한다.
6. 도넛을 Next로 선택해 body clip 시작 없이 현재 위치에 Effect와 Server 판정이 연결되는지 본다.
7. 창을 줄이거나 Flow 오류 상태로 만들어도 예약 카드와 취소가 접근 가능한지 확인한다.
8. Trash 전멸 뒤 대기 문구, 사용자 Revive 이후 Next 시작을 확인한다.

성공 여부는 사용자의 서면 관찰로 RESULT에 남긴다. source test/빌드 성공을 visual PASS로 바꾸지 않는다.

### 변경 파일과 commit 단위

production 변경은 Shared packet/header, NetworkManager, 기존 audition/Flow service, BossTool, PatternTree,
GameRoom, ValtanBrain, ServerWorldEntity와 관련 tests다. 새 C++은 G08의 실제 실행형 harness main 하나다.
도넛 source는 현재 계약이 이미 있으므로 불필요한 rewrite 없이 회귀로 보존한다.

이 Next 수직 슬라이스와 테스트·project 등록·대응 RESULT를 하나의 검증 단위로 묶는다. Trash 단일 패턴
통합은 연결된 별도 계획의 단위로 진행하되 공통 terminal receipt를 다시 만들지 않는다.
public 문서 `.md/TEAM/보스툴.md`는 구현 후 Next의 split 29개 목록과 공용 Tool 27개 authoring 목록의 차이를 갱신한다.
AGENTS/CLAUDE에 날짜별 작업 로그를 추가하지 않는다.

## G10. 사용자 후속 요청: Six Pizza 카메라와 점프찍기 휠윈드 방향

Six Pizza는 split presentation의 camera invocation과 직접 런타임이 읽는
`ValtanCinematicCamera.json`의 해당 cue를 함께 제거한다. 다른 입장·109·사망 camera는
보존한다. 기존 Camera Python 계약과 Action native harness에서 Six Pizza cue 부재를 확인한다.

`VALTAN_ATTACK_WHIRLWIND`는 `NEAREST_EACH_TICK + TRACK_TARGET_EACH_TICK`으로 저장돼
Server `UpdatePatternTargetAndAim`이 점프 중에도 yaw를 계속 바꾼다. 해당 패턴만
`LOCK_NEAREST_ON_START + LOCK_FACING_ON_START`로 바꿔 STEP_01 점프, STEP_02 준비,
STEP_03 휠윈드의 방향을 유지한다. STEP_04 종료 동작 ENTER에서 기존
`RETARGET_RANDOM_ALIVE` 이벤트로 살아 있는 플레이어를 한 번 바라본다. 이후 종료 동작에서도
다시 추적하지 않는다. 새로운 회전 runtime이나 Client local LookAt을 추가하지 않는다.

정본 gameplay와 재저작 helper의 해당 블록을 함께 수정하며 나머지 패턴, 타격 시간, animation,
Effect는 보존한다. publisher로 Product·bootstrap을 생성하고 기존 Server 실행형 fixture에
공격 중 방향 유지, 회전 종료 경계의 재조준, 살아 있는 대상 부재, 새 occurrence를 확인한다.
새 C++ 파일이나 project/filter 등록은 필요하지 않다. 결과와 실제 빌드 상태는 RESULT에 기록한다.

## G11. main 기준 live Next 선택과 저장 Flow 재시작

### 목표와 종료 증거

최초 main 38cc82c8과 동일한 Desktop에서 codex/valtan-flow-reload-next-fix로 분기한다.
후속 main PR #255의 CROSS 추가는 1a9bce42까지 보존 통합한다. 이에 따라 현재 split 30 /
공용 Tool 28(8 core + 20 animator) / Product 54 정의를 사용하고, 이 문서 G00~G10의
과거 29/27 수량으로 되돌리지 않는다. 저장 Flow 22슬롯은 inventory 확장과 별개로 그대로 둔다.
사용자가 첨부한 화면의 저장 revision은 현재 JSON의 3a0e831a8454ef09ae05c1023b251d607cce60d0702e3cb732fb7e9b952b79d5와 같다.
현재 22슬롯, 첫 행 WHIRLWIND, 중간의 별도 WHIRLWIND 슬롯과 nextSlotOrdinal 31을 보존한다.
slot ID 숫자를 정렬 기준으로 쓰지 않으며 사용자 Effect 및 맵 수정도 이 작업에 섞지 않는다.

기존 Next가 IDLE에서 비활성인 이유는 Client와 Server가 isolated predecessor epoch만 받기 때문이다.
기존 Reload는 문서만 읽으므로 Product 자동 순서 또는 이미 승인된 이전 Flow를 교체하지 않는다.
G11은 최초 해석의 Debug 재시작 범위다. 사용자 최종 확인에 따라 실제 Product 기본 순서 연결은 G12가 추가로 소유한다.

### G11-H. 실제 변경 파일과 소유권

| 파일 | 변경 책임 |
|---|---|
| Client/Public/BossTool.h, Client/Private/BossTool.cpp | Reload 성공 뒤 첫 슬롯 Start 제출, pending/실제 runtime/저장 revision 분리, live Next 선택과 거절 이유 표시 |
| Client/Public/ValtanPatternFlowService.h, Client/Private/ValtanPatternFlowService.cpp | 승인된 Flow와 pending Start 분리, 교체 거절 rollback, exact retry, old lifecycle 계속 소비 |
| Client/Public/ValtanPatternFlowDocument.h | 실제 service harness에서 renderer 없이 정의를 소비하도록 bool alias와 불필요 Engine include만 분리; 저장 계약 불변 |
| Client/Public/ValtanPatternAuditionService.h, Client/Private/ValtanPatternAuditionService.cpp | HUD 관측 predecessor에 대한 live Next 요청과 새 epoch lifecycle 채택 |
| Client/Private/NetworkManager.cpp | 새 typed operation 송신과 공용 audition result queue routing |
| Shared/Public/Network/PacketType.h, Shared/Public/Network/PacketMessages.h, Shared/Private/Network/PacketMessages.cpp | protocol 43 및 QUEUE_NEXT_LIVE_PATTERN_ID의 strict codec |
| Server/Public/GameRoom.h, Server/Private/GameRoom.cpp | 실제 current 채택, idle/active 완료 경계, 같은 owner Flow 재시작 transaction |
| Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp, Server/Private/ServerGameplayContractTests.cpp | wire, live Next, restart 및 순서/실패 회귀 |
| Tools/ValtanPatternAuditionServiceHarness | 기존 actual-service 실행 파일에 FlowService CPP와 Flow test TU 추가 및 project/filter 등록 |
| Tools/ValtanPipeline의 대응 contract tests, 팀 보스툴 문서 | 새 사용자 동작을 검사하고 public 실행 절차 갱신 |

새 게임 런타임이나 UI 실행기는 추가하지 않는다. Flow 정의의 저장/재생 ID, JSON schema, Product 순서는 바꾸지 않는다.

### G11-CPP. Reload 및 saved Flow 실행

1. Reload는 dirty draft일 때만 Discard & Reload 확인을 유지한다. Keep Draft 및 parse/validation 실패는 Server 명령을 보내지 않는다.
2. 읽기 성공 후 selection은 항상 배열 첫 slotId다. Start_Flow(false)가 그 저장 revision과 전체 배열을 같은 기존 FLOW_START 경로로 보낸다.
3. FlowService의 기존 Start signature는 유지한다. pending Start command가 full request, Client generation, 송신 시간과 WAITING/UNCONFIRMED를 소유한다.
4. Get_Snapshot은 마지막 Server 승인 run을 유지한다. Has_PendingStart/Get_PendingStart/Has_PlaybackOwnership/Retry_Start를 실제 Tool과 audition service가 소비한다.
5. pending 동안 기존 Flow lifecycle과 Stop 소유권을 유지한다. 새 요청 거절/송신 실패는 기존 run을 버리지 않는다. exact 승인 또는 lifecycle 뒤에만 새 run으로 교체한다.
6. 5초 응답 부재는 미확정으로 표시하고 같은 요청만 재전송한다. Server가 승인한 PENDING 또는 부활 대기를 15초 뒤 Client 단독 ABORTED로 만들지 않는다.
7. Server는 같은 session의 Flow/isolated/Next만 교체할 수 있다. 타 owner 및 Timeline 충돌은 계속 거절한다. 모든 새 슬롯/리소스/reset preflight 뒤 기존 reset을 한 번 commit하고 새 epoch를 발급한다.
8. Save는 파일만 저장한다. 화면에는 현재 Server 실행 출처와 saved/runtime revision을 구분해 표시하고 Reload가 첫 행을 재생한다는 뜻을 명시한다.

### G11-CPP. live Next와 완료 경계

기존 QUEUE/CLEAR의 nonzero predecessor epoch와 CAS 계약은 유지한다. 새 QUEUE_NEXT_LIVE_PATTERN_ID만
epoch=0, expected-next-token=0과 HUD에서 관측한 실제 predecessor sequence를 받는다. sequence 0은 실제 idle만 허용한다.
Server는 같은 placement/entity의 실제 sequence, owner, 살아 있는 boss, pinned catalog와 split-owned 대상 ID를 검증한다.

현재 Product 패턴 또는 같은 owner Flow의 현재 패턴이 있으면 이를 새 Server epoch의 선행 occurrence로 채택한다.
선행 current의 가짜 lifecycle은 보내지 않는다. NEXT_RESERVED의 exact request/pattern/expected sequence와 실제 epoch를
Client가 채택하고 B의 PENDING/ACTIVE부터 기존 current service로 승격한다.
현재 패턴이 없으면 별도 idle anchor로 다음 tick에 B를 시작하며 정상 완료 receipt를 합성하지 않는다.
플레이어가 없으면 기존 resetless PENDING/WAITING 경계를 유지하고 자동 부활하지 않는다.

Flow 중 Next 선택은 남은 Flow 재생을 종료하고 현재 occurrence가 실제 완료된 뒤 선택한 one-shot을 시작한다.
현재 occurrence가 소비하던 Flow sequence/pin은 그 종료까지 보존한다. 저장 배열에 삽입하거나 Product cursor로 자동 복귀하지 않는다.
예약 채택과 승격에서 player/boss 위치, HP, resource, cooldown, wall/floor/prop/navigation을 reset하지 않는다.
stale sequence, 다른 owner, 손상된 ledger와 unknown pattern은 실패 이유를 보존하고 기존 실행을 유지한다.

### G11 검증과 사용자 확인

- Shared: 새 op roundtrip, zero/nonzero 필드 조합, 잘못된 ID/version, truncation, destination rollback, 기존 Queue/Clear strictness.
- Server: live Product/owned Flow/idle의 Next, current 완료 후 다음 tick 시작, same-ID occurrence, player 대기, 잘못된 owner/sequence/ID 및 반복 요청.
- Server Flow: sparse slotId의 서로 다른 A/B/C에서 새 revision C/A/B로 재시작하여 실제 관측 순서를 비교한다. 실패/타 owner/stale Stop/중복 restart에서 이전 상태와 reset 횟수를 확인한다.
- Client 실제 service: old lifecycle 수신 중 replacement, 거절/송신 실패 보존, exact retry, world 변경, 15초를 넘긴 Server 대기, live lifecycle의 actual epoch와 request identity 검증.
- 관련 Python contract, Project-ValtanPatternMaster ValidateV2, gameplay publisher Validate, JSON/XML parse, Debug/Release 정본 회귀, git diff --check를 실행한 결과만 RESULT에 기록한다.
- 기존 actual-service harness project/filter에 새 Flow test TU와 production CPP를 함께 등록하고 Debug/Release에서 실행한다.
- 에이전트는 Client/UI를 실행하거나 캡처하지 않는다. 사용자가 Server + Client profile로 실행하여 F1 → Boss Tool → Pattern Flow에서 Reload Flow의 01 시작, live Next 선택, 저장 순서와 실제 진행을 확인한다.


## G12. 사용자 최종 확인: 저장 Flow를 실제 발탄 기본 순서에 연결

### 요청과 확인한 원인

사용자가 말한 차이는 같은 ID의 animation clip 차이가 아니다. Boss Verification이 관찰하는 실제 발탄이
Pattern Flow에 저장한 커스텀 순서를 다음 실행에 사용해야 한다. 현재 Save는 22개 slot 문서만 바꾸고,
제품은 별도 gameplay.scriptedSequence의 28개 고정 항목을 읽어 저장한 변경을 소비하지 않는다.
두 첨부 화면의 WHIRLWIND와 SEQUENCE_WHIRLWIND는 별개 정의이며 이번 수정에서 합치지 않는다.

저장 순서에는 WHIRLWIND가 두 번 있다. 중복을 지우거나 입장 패턴을 숨겨서 앞에 넣으면 사용자 순서와
달라지므로 배열 순서와 반복 occurrence를 그대로 보존한다. 취소된 Ordered Slots 현재/Next 표시 확장은 하지 않는다.

### 단일 저장 정본과 실제 소비자

1. 기존 `ValtanBossAuditionFlows.json`의 default flow가 순서와 inter-step pursuit의 단일 정본이다.
2. `Valtan.gameplay.json`의 scriptedSequence는 sequenceId/mode/flowId로 그 정본을 참조한다.
   임의 파일 경로를 받지 않으며 flowId는 고정된 Flow 문서에서 정확히 한 번 resolve한다.
3. Python pipeline과 Client PatternTree는 참조를 strict parse/validate한 뒤 기존 inline sequence IR로 바꾼다.
   generated Product/Server bootstrap은 기존 PATTERNSEQUENCE/PATTERNSEQUENCESTEP 경로를 그대로 쓴다.
   inline shape는 기존 immutable/migration 입력 호환을 유지한다. 두 shape를 섞거나 unknown flow를 받지 않는다.
4. canonical source manifest는 Flow 파일 hash도 포함한다. 저장 후 낡은 authoring overlay는 CAS 실패로 알리며
   다른 Balance 편집을 자동 폐기하거나 새 Flow에 몰래 합치지 않는다.
5. 순서의 반복 patternId는 서로 다른 occurrence다. slotId 중복은 계속 거부하고, Product ordinal과 unknown ID
   검증도 유지한다. 현재 22슬롯을 덮어쓰거나 재정렬하지 않는다.
6. 기존 입장 cinematic 정의는 지원하는 entry-only 정의로 남긴다. 커스텀 순서에서 생략하면 자동 삽입하지 않는다.
   명시적으로 사용하는 경우 첫 행에서만 허용하며, 임의의 unowned pattern 허용으로 넓히지 않는다.

### Save, 적용, Reload의 의미

Flow 파일의 기존 durable Save를 유지한다. 저장 뒤 기존 Product publisher와 immutable candidate 생성 경로를
호출하고, 기존 typed DATA_REVISION prepare/Client READY/Server tick commit을 통해 실제 기본 순서를 적용한다.
새 파일 watcher, Client local AI 또는 두 번째 boss sequence runtime은 만들지 않는다.

- Tool은 SAVED와 PUBLISHING/APPLY_PENDING/COMMITTED/FAILED를 분리한다.
- publisher 또는 Server 실패는 저장된 draft를 잃지 않으며, 이전 Server revision이 계속 실행 중임을 표시한다.
- 같은 request sequence/candidate/connection generation의 실제 COMMITTED만 적용 성공이다.
  이미 같은 candidate를 사용하는 Server는 ALREADY ACTIVE로 별도 표시하고 새 commit receipt를 합성하지 않는다.
- pipeline 작업은 hidden owned process로 실행하고 UI Update에서 결과를 소비한다. 미확정 중 중복 publish를 막는다.
  120초는 미확정 경고 기준이다. 내부 Product publisher도 timeout으로 강제 종료하지 않아 commit/rollback을 끝내게 한다.
- 기존 Balance Tool과 Flow 적용이 하나의 typed revision request sequence 소유자를 공유한다.
- 진행 중 Product 순서 전체는 시작 catalog generation을 끝까지 pin한다. 새 순서는 다음 encounter/reset에서 사용한다.
  현재 run의 중간 ordinal에 다른 revision 배열을 끼워 넣지 않는다.
- 명시적 Reload Flow는 적용 작업이 확정된 뒤 저장된 첫 slot(화면 01)부터 기존 FLOW_START로 재시작한다.
- 기존 Next는 현재 occurrence 완료 뒤 reset 없이 다음 선택을 재생한다. Next 자체는 저장 기본 순서를 변경하지 않는다.

### 파일과 검증

Python `valtan_tuning_pipeline.py`, 기존 PowerShell publisher, 해당 schema/계약 tests가 Flow 참조와 candidate를 소유한다.
Client PatternTree는 같은 정본을 소비한다. 새 typed tuning command service의 public header/private cpp는
Client project와 filters, 필요한 실제 service harness에 함께 등록하고 BossTool 및 BalanceTool이 호출한다.
Server GameplayCatalog의 반복 step admission과 기존 Brain의 Product sequence pin을 수정한다.

기존 source/Project ValidateV2/Gameplay Validate와 actual-service, protocol, Server harness를 확장한다.
검증은 saved A/B/A와 reordered C/A/B가 Product/bootstrap에 같은 순서로 투영되는지, 틀린 schema/flowId/slotId,
빈 default/unknown pattern/CAS drift/중간 publisher 실패에서 이전 실행 보존, stale revision 결과 무시,
active sequence old pin 유지와 reset 뒤 새 순서 시작을 포함한다. 현재 22슬롯도 삭제·중복 제거 없이 검사한다.
Debug/Release 정본 회귀와 git diff --check 결과를 RESULT에 기록한다. 사용자 UI 조작/시각 확인은 수동 미검증으로 남긴다.

## G13. Six Pizza 첫 착지만 도끼 점프 시간에 맞추기

사용자는 마지막 확인에서 상승이 아니라 첫 착지가 느리다고 정정했다. 현재 첫 착지는
`VALTAN_SIX_PIZZA_106.serverMotion.travelStageId=STEP_03`, `travelStartMs=0`,
`travelEndMs=700`이다. 도끼 점프 `VALTAN_HIGH_JUMP`의 LAND 이동은 0~267ms다.
Server `Advance_ArenaBreakLeap`는 이 구간 동안 높이와 착지 위치를 보간한다. 첫 STEP_03 이후에는
leap apex를 해제하므로 이후 애니메이션 구간 전체의 재생 속도를 바꾸지 않는다.

`Valtan.gameplay.json`과 기존 재저작 helper `author_valtan_phase_two_mechanics.py`에서
Six Pizza의 `travelEndMs`만 267로 맞춘다. 높이 10m, 상승 800~1100ms, STEP_03 전체 길이 1200ms,
후속 단계, presentation clip/rate와 사용자 미커밋 Effect JSON은 보존한다. generated Product는
기존 publisher로 생성하며 직접 편집하지 않는다. 새 C++ 파일이나 project/filter 등록은 없다.

기존 PatternTree Python 회귀에 첫 착지 시간의 도끼 점프 일치와 상승/단계 시간 보존을 추가한다.
기존 Server leap 실행 fixture를 확장해 실제 fixed tick에서 STEP_03 진입 후 267ms 경계까지
anchor 착지가 끝나고 이후 stage의 이동을 다시 적용하지 않는지 검증한다. publisher validation과
Debug/Release 하네스 결과를 RESULT에 기록하며 착지의 최종 시각 판정은 사용자가 한다.

## G14. 추적 도끼 점프의 공중 대기 2.5초 연장

추가 요청의 중간 텀은 `VALTAN_HIGH_JUMP/AIRBORNE`다. 기존 4000ms에서 6500ms로 늘린다.
TAKEOFF 1933ms, LAND 3200ms, RECOVERY 400ms, 상승/착지 이동 window, 도끼 생성 0/1333/2666ms,
최초 타격 1200ms는 변경하지 않는다. 기존 publisher는 managed 도끼의 보존 시간을 owner stage에서
생성하므로 Product `lifeMs`도 4000→6500ms로 이어진다. 이는 타격 횟수나 추적 시간을 늘리지 않는다.
AIRBORNE presentation은 이미
`LOOP_TO_STAGE_END`이므로 애니메이션 전체 속도를 변경하지 않고 Server stage 시간을 따른다.

정본 gameplay와 v1 migration의 현재 HIGH_JUMP 투영값을 6500ms로 갱신한다. 기존 publisher와
Server catalog에는 owner 검증에 4000ms equality가 있어 길어진 유효 stage도 거부하고 있었다.
세 웨이브의 최소 4000ms를 유지하면서 더 긴 대기를 허용하도록 고친다. 이는 기존 4000ms immutable
candidate를 계속 읽을 수 있게 하며, 새 기본값 6500ms와 minimum/maximum 및 spawn schedule 경계는
기존 validator와 실제 catalog로 검증한다. snapshot revision/pin을 우회하는 runtime 값 교체는 하지 않는다.

기존 HIGH_JUMP projection/Server leap 회귀에 6500ms 체류와 기존 도끼 횟수·간격, 267ms 착지를
함께 확인한다. overflow fixture는 바뀐 stage duration을 기준으로 만든다. Product/bootstrap은 publisher로
생성하고 Debug/Release 회귀 및 사용자 미검증 상태를 같은 RESULT에 기록한다.

## G15. 고정 개수를 없애고 Boss Tool·Flow·Play/Next의 패턴 목록 통합

### 현재 데이터와 결함

29는 필수 개수가 아니다. 현재 split gameplay에는 31개 정의가 있으며, 기존 Tool의 하드코딩된
Core 8개와 Animator 20개, 새 Derived 1개만 합치면 29개가 된다. 정상 정의인 FIST_IN_OUT과
ENTRANCE_CINEMATIC은 이 별도 목록에서 빠진다. Next는 이미 strict join된 전체 split 정의를
열거하지만 Flow publisher는 Core ID 상수와 MANUAL_SERVER_AUDITION만 합쳐 28개를 요구한다.
따라서 툴에서 추가한 GHOST_FINALE를 저장해도 실제 적용 단계에서 거부되는 불일치가 있다.

### 정본과 변경 경로

패턴 선택 정본은 `Data/Valtan/Valtan.gameplay.json.patterns`의 stable patternId와 대응 presentation이다.
새 allowlist JSON, 개수 상수 또는 별도 재생 런타임을 추가하지 않는다. Client의 strict joined graph에서
`bAuthoringMasterManaged`인 정의만 기존 entry/stage identity 검증을 거쳐 공통 재생 목록으로 만든다.
Tool은 이 목록을 Core, Animator, Derived로 표시하되 manualAuditions의 실제 admissionState로 분류한다.
분류별 개수와 전체 개수는 표시값이며 승인 조건이 아니다. Next·Play Selected·All Effects·저장 Flow가
같은 정의 집합을 소비한다. legacy Product만 존재하는 정의는 자동으로 이 집합에 넣지 않는다.

Python Flow resolver와 Client의 source-local Flow resolver는 실제 gameplay patterns에서 ID를 읽는다.
Core ID 상수, 정확히 20개 manual 요구, 28/29개 equality를 삭제한다. 새 정의를 추가하거나 기존 정의를
제거한 뒤에도 유효한 남은 Flow를 읽을 수 있어야 한다. 등록된 Derived 정의에는 animation intake의
고정 목록을 강제하지 않으며, 기존 MANUAL_SERVER_AUDITION의 source chain/clip 연결 검증은 유지한다.

### 유지할 실행 계약

patternId·stageId·actionId의 유효성과 중복, gameplay/presentation/Product 조인의 일치, 소유자 충돌,
알 수 없는 ID, source-local snapshot, 실패 시 기존 상태 보존은 계속 검사한다. entry cinematic의
첫 슬롯 1회 규칙은 Product의 기존 의미이므로 Client 저장 검사도 일치시킨다. 이는 일반 패턴의
반복 occurrence를 제한하지 않는다. 한 Flow의 1~255슬롯 U8 범위와 문서/packet 크기는 별도의
저장·전송 용량이며 256슬롯은 모든 소비 경계에서 거부한다. 이 용량은
전체 등록 패턴 수 제한으로 사용하지 않는다. 기존 사용자 22슬롯을 자동 채움·재정렬하지 않는다.

새 패턴은 stable ID로 gameplay와 presentation을 함께 추가하고, 기존 자동 선택/기믹 소유자 또는
manualAuditions의 MANUAL_SERVER_AUDITION/DERIVED_SERVER_PATTERN으로 실행 소유자를 선언한다.
기존 clip을 조합한 독립 패턴은 새 patternId/actionId/stageId/occurrenceId를 사용한다. 단순 재생 순서의
결합은 Flow 슬롯 참조로 표현한다. 새 Server 행동 종류가 필요하면 해당 typed 실행 계약과 검증을
함께 확장한다. 기존 행동을 조합하는 경우 C++ 목록이나 숫자를 추가로 고치지 않는다.

### 파일과 검증

기존 PatternTree h/cpp, FlowDocument cpp, 필요 시 BossTool/Effect Tool의 목록 소비부,
`valtan_tuning_pipeline.py`와 기존 Python/EffectRender/Flow service 하네스를 수정한다.
새 C++ 파일이 없으므로 project/filter 등록은 추가하지 않는다. 다른 작업의 ghost/portal/잡기 구현,
protocol 44, Effect 및 맵 편집은 보존하며 다른 작업에 메시지를 보내지 않는다.

회귀는 현재 전체 정의와 Tool/Next의 집합 일치, 임의 개수의 유효한 신규 Core/manual/derived 추가 및
제거, GHOST_FINALE 저장→join→Product sequence, FIST_IN_OUT과 optional entry 선택을 검사한다.
등록 패턴이 32개를 넘어도 일부만 선택한 Flow가 동작해야 한다. 잘못된 ID·중복·entry 위치·중간
실패에서는 이전 목록/문서/실행이 유지되어야 한다. 기존 Python suite와 publisher Validate를 우선
실행하고 관련 실제 C++ harness 및 필요한 빌드를 검증한다. 실행하지 않은 검사와 사용자 화면
확인은 PASS로 기록하지 않는다. 최종 인계는 대응 RESULT의 G15 결과로 제공한다.
