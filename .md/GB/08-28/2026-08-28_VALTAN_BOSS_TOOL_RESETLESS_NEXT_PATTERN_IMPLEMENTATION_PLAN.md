# Valtan Boss Tool 무리셋 Next Pattern·독립 도넛 구현 계획

작성일: 2026-08-28  
상태: **구현 전 초안. 통합 protocol 41 기준 재확인 전이며 이번 병합의 runtime 구현 범위가 아니다.**

사용자 최신 지시에 따라 Trash는 보류했다. 저장 Flow 테스트 보강만 별도 구현했으며,
이 문서의 Next C++·게임 데이터·빌드는 변경하거나 검증 완료하지 않았다.

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

| 실제 위치·기준 함수 | 현재 동작 | 변경 방향 |
|---|---|---|
| [BossTool.cpp:419](C:/Users/user/Desktop/LostArk/Client/Private/BossTool.cpp:419) `Start_Flow` | clean 저장본 전체 슬롯·시작 slotId를 Flow service에 전달 | 유지; Next 소유권과의 충돌 검사만 보강 |
| `CBossTool::Preview_SelectedFlowSlotIsolated` | `FLOW_PREVIEW_CONSUMER_ID`로 공용 audition service 호출 | 최초 A 실행 경로 유지 |
| `CBossTool::Submit_SelectedPattern` | `CONSUMER_ID`로 같은 service 호출 | 최초 A/명시적 새 isolated 실행 유지 |
| [ValtanPatternAuditionService.cpp:91](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternAuditionService.cpp:91) `Submit` | current snapshot 하나, `PLAY_PATTERN_ID` | current와 next·control request를 분리 |
| [GameRoom.cpp:5135](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:5135) `Evaluate_ValtanAudition` | bait 배치 → reset → `PendingPatternIds` | Next는 이 reset 분기 전에 별도 evaluator로 보냄 |
| [GameRoom.cpp:3198](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:3198) `Refresh_ValtanPatternIdAuditionState` | 단일 occurrence 완료 뒤 hold·state clear | 정확한 종료 receipt, 완료 anchor, Next 승격 추가 |
| [ValtanBrain.cpp:1640](C:/Users/user/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1640) `FinishPattern` | 성공과 실패 모두 pattern ID를 비움 | 실제 `completed` 결과를 Server 내부 receipt에 기록 |
| [ValtanPatternTree.cpp:5482](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternTree.cpp:5482) `Build_ToolAuditionInventory` | Core 8 + Animator 20 = 28, 도넛 제외 | 이 authoring/Flow inventory는 유지 |
| `CBossTool::Find_Pattern` / PatternTree CPP 내부 `Find_Pattern` | joined Gimmicks/Rotation exact stable ID 조회 | UI는 BossTool의 기존 조회, builder는 CPP 내부 helper 사용 |

현재 generated Encounter에는 53개 정의가 있고, split gameplay가 직접 소유한 것은 29개다.
`전체 패턴`이라는 UI 문구는 이 작업에서 **검증된 split-owned 29개**를 뜻한다. legacy 24개까지 무조건
재수집하지 않는다. `bAuthoringMasterManaged`로 소유권을 검사하고 count는 runtime 목록 크기로 표시한다.
Trash 보조 세 정의를 호환용으로 보존하므로 이번 기능 뒤에도 이 목록 수는 29개다.

### 1.2 Reload Flow는 Server 재생 명령이 아니다

[BossTool.cpp:502](C:/Users/user/Desktop/LostArk/Client/Private/BossTool.cpp:502)의 `Reload_FlowDocument`는
`CValtanPatternFlowDocument::Reload`로 디스크 JSON을 parse/validate하여 draft만 교체한다.
dirty일 때 `Discard & Reload / Keep Draft` 확인을 받는다. reload 실패는 기존 유효 draft를 유지한다.
이 경로는 Server packet을 보내지 않고 발탄·플레이어·맵에 영향을 주지 않는다.

Next도 이 의미를 바꾸지 않는다. Save/Reload/Ordered Slots 편집은 이미 승인된 Next 예약을 취소하지 않는다.
Flow 문서가 손상되거나 미저장이어도 graph와 isolated chain이 유효하면 Next 선택은 가능하다.

### 1.3 기존 완료 판정과 tick 위치는 그대로 사용할 수 없다

현재 `FinishPattern(false, NO_VALID_TARGET)`도 ID를 비우므로 Refresh의
`ACTIVE + 동일 sequence + 빈 pattern ID`는 성공 완료 증거가 아니다. 이 조건만으로 Next를 붙이면
중단된 A 뒤에도 B가 시작된다.

현재 Refresh 호출은 `Update_WorldEntities` 말미다. 그 뒤 room tick에서 attachment 최종 pose,
encounter prop, world destruction commit이 이어진다. 따라서 Next 승격은 그 뒤로 옮겨야 한다.

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
- 기존 28개 All Effects authoring inventory를 29개로 교체.
- Product live overlay, 여러 Next queue, 자동 예약 저장/재시작 복원.
- 새 보스 AI, Client local damage, 별도 Effect 재생기, 별도 Level.
- 도넛용 새 mesh/texture 생성, half-donut UV 작업, 기존 도넛 수치 재튜닝.

## G00. 머지 기준선과 작업 준비

**책임:** 선행 작업을 잃지 않고 현재 source contract 위에 구현한다.

조사 기준은 HEAD `d77d7e021edefc435119b23f9aab53315e3f870a`와 당시 dirty worktree다.
본문 line number는 탐색 힌트이며, 머지 후에는 함수 이름·정본 키로 다시 찾는다. 다른 세션에서 작업 중인
Boss→Effect exact tuple handoff, Product Effect unlink, NetworkManager endpoint 변경은 통째로 덮어쓰지 않는다.

1. AGENTS/CLAUDE/gotchas와 연결 PLAN/RESULT를 읽는다.
2. LAN sync, `git status --short`, `git fetch`, 현재 branch/HEAD/main 차이를 확인한다.
3. 선행 PR이 main에 들어온 clean 작업 기반에서 `codex/valtan-next-pattern` 기능 브랜치를 사용한다.
4. 사용자 Flow JSON과 unlink된 FOUR_SLASH/RECOVERY cue를 보존 대상으로 기록한다.
5. 같은 기능의 Next/terminal receipt가 선행 머지로 이미 생겼으면 중복 구현하지 않고 이 계약과 대조한다.

실제 코드·데이터 구현은 이 문서 작업과 별개다. 현재 dirty worktree에서 자동 stage/commit/merge하지 않는다.

## G01. Next 목록을 기존 슬롯 inventory와 분리

### G01-H. `Client/Public/ValtanPatternTree.h`, `Client/Public/BossTool.h`

`CValtanPatternTree::Build_ToolAuditionInventory` 선언 바로 아래에 다음 **새 선언**을 둔다.

```cpp
static bool_t Build_NextPatternInventory(
    const VALTAN_PATTERN_TREE_VIEW& View,
    std::vector<std::string>& OutPatternIds,
    std::string& strOutError);
```

소비자는 Boss Tool의 Next popup 하나다. 기존 inventory나 Flow document는 이 함수를 호출하지 않는다.
출력은 stable pattern ID vector이며 pointer·현재 list index를 저장 ID로 사용하지 않는다.

Boss Tool의 기존 inventory/cache 멤버 근처에 `m_NextPatternIds`와 Next 검색 buffer를 추가한다.
검색 buffer는 `std::array<char_t, 128u> m_NextPatternSearch{}`로 패널 수명만 갖는다.
예약된 patternId·owner·epoch는 이 UI 멤버에 저장하지 않고 service snapshot에서 읽는다.

### G01-CPP. `Client/Private/ValtanPatternTree.cpp`

**한 문장 책임:** strict-joined graph의 split-owned 패턴만 안정적인 순서로 반환한다.

호출 흐름: `CBossTool::Reload_Graph`의 stage 성공 → 새 inventory builder → 유효하면 UI cache commit.

1. `View.Gimmicks`와 `View.Rotation`에서 `bAuthoringMasterManaged=true`만 수집한다.
2. ID empty/duplicate, exact lookup 실패, stage/entry action 불일치를 실패로 처리한다.
3. `iSourceSequenceIndex`, stable patternId 순으로 결정적 정렬한다.
4. staged vector가 완성된 뒤에만 Out에 move한다. 실패 시 기존 Out을 부분 교체하지 않는다.
5. `Legacy`를 fallback으로 붙이거나 `Find_AuditionPattern`으로 다시 28개 admission을 걸지 않는다.

새 count는 UI vector의 size다. 테스트에서 현재 source ID 집합 29개와 일치함을 확인하되 production에
29개 문자열 배열이나 29라는 count assertion을 새 정본으로 만들지 않는다.

### G01-UI 목록 의미

한국어 displayName을 먼저, stable ID는 tooltip/검색 대상으로 보여 준다. `VALTAN_FIST_IN_OUT`에는
`독립 효과 · 애니메이션 없음`을 표시한다. Trash 보조 3개는 수동 호환 audition임을 구분한다.
이 구분은 삭제·alias가 아니다. 보조 행을 선택하면 기존의 해당 stable ID만 명시적으로 예약한다.
본체 통합 재생은 `VALTAN_TRASH`를 선택한다.

## G02. 기존 audition wire family 확장

### G02-H. `Shared/Public/Network/PacketMessages.h`

기존 enum 숫자를 보존하고 `VALTAN_AUDITION_OPERATION::END` 직전에 두 값을 추가한다.

```cpp
QUEUE_NEXT_PATTERN_ID,
CLEAR_NEXT_PATTERN_ID,
```

`C2S_VALTAN_AUDITION_REQUEST`와 `S2C_VALTAN_AUDITION_RESULT`의 stable string 뒤에 다음 필드를
추가한다. 구조체에는 항상 있지만 wire에는 새 두 operation에서만 쓴다.

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

결과 enum의 기존 값을 유지하고 `END` 전에 다음 verdict를 추가한다.

```cpp
CLEARED,
REJECTED_STALE_REQUEST,
REJECTED_STALE_AUDITION,
REJECTED_NOT_OWNER,
REJECTED_NEXT_CHANGED,
```

Queue 성공은 기존 `QUEUED`를 사용한다. 거절은 위 값 또는 기존 world/boss/release/unavailable verdict다.
Next에서 `DUPLICATE_IGNORED`를 새 성공으로 해석하지 않는다. exact 재전송은 최초 verdict 자체를 재전송한다.

Lifecycle enum에는 `NEXT_RESERVED`, `WAITING_FOR_PLAYER`를 추가한다. 기존 lifecycle의
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

`Shared/Public/Network/PacketType.h`의 현재 protocol은 39다. 머지 뒤 then-current version을 확인해
그 다음 값으로 한 번 증가시킨다. 문서의 39를 근거로 다른 PR의 version을 40으로 되돌리지 않는다.

### G02-NET. `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`

기존 stable-ID send 바로 아래에 Queue/Clear를 보내는 typed send 함수를 추가한다. 인자는 위 exact tuple이며
packet 작성은 NetworkManager만 한다. Boss Tool은 이 함수를 직접 호출하지 않는다.
기존 result routing의 `PLAY_PATTERN_ID` 조건에 Queue/Clear를 포함하여 동일 전용 result queue로 보낸다.
legacy Level consumer가 새 결과를 빼앗지 않도록 static/runtime contract를 추가한다.

ServerApp의 기존 `C2S_VALTAN_AUDITION_REQUEST` parse → `ROOM_COMMAND::ValtanAudition` 전달은 재사용한다.
새 packet handler를 복제하지 않는다.

## G03. Server 예약 소유권·CAS·정상 완료 receipt

### G03-H1. `Server/Public/GameRoom.h`

현재 `VALTAN_PATTERN_ID_AUDITION_STATE` 근처에 아래 상태를 추가한다. 새 값은 private Server 상태다.

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
};

struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
{
    LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
    LostArk::Shared::VALTAN_AUDITION_RESULT Result;
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

current phase에는 `COMPLETED_HOLD`를 추가한다. A 정상 완료 identity와 pin을 보존하는 상태이며 완료
lifecycle을 매 tick 재전송하지 않는다. Next가 없어도 같은 owner가 이후 Next를 예약할 수 있다.
단, `COMPLETED_HOLD + Next 없음 + 미확정 command 없음`은 새 resetful Play/Flow Start를 막지 않는다.
명시적 새 실행이 승인되면 이전 anchor를 폐기하고 새 epoch를 발급한다.

### G03-H2. `Server/Public/ServerWorldEntity.h`

기존 boss mechanic 타입 근처에 아래 **Server 내부 종료 증거**를 추가한다.

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

**`Evaluate_ValtanNextPatternCommand` 책임:** 예약 1칸만 원자적으로 바꾼다.

기존 `Evaluate_ValtanAudition`의 current-audition conflict/reset 검사 전에 새 operation을 분기한다.
기존 conflict 검사에서 부작용 있는 Refresh를 호출하지 않고 순수
`Is_ValtanPatternIdAuditionRunning` predicate를 사용하도록 두 호출부(Play/Flow)를 교체한다.

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
Lifecycle helper는 현재 state만 암묵적으로 읽지 말고 명시적 current/reservation identity를 받아 작성한다.

## G04. fixed-tick 종료 뒤 승격

### G04-CPP. `CGameRoom::Tick`, `Update_WorldEntities`, `Try_PromoteValtanNextPattern`

**책임:** A의 정상 commit이 끝난 안전 경계에서 승인된 B를 기존 pending 실행 경로로 넘긴다.

현재 `Update_WorldEntities` 말미의 `Refresh_ValtanPatternIdAuditionState` 호출을 제거하고 room tick의
`Drain_BossCombatEvents()` 뒤, Debug lifecycle flush 직전으로 이동한다.

현재 `Tick`은 `Commit_DueEncounterProps` 반환값을 버리고 그 함수도 일부 Prepare/Commit 실패를 삼킨다.
이 경로도 함께 교정하여 READY transaction의 Commit 실패와 invalid Prepare를 false로 전달하고,
Tick이 false면 not-ready로 종료하게 한다. 정상 no-op은 성공으로 유지한다. 이 전파 없이 prop 실패에도
승격이 차단된다고 주장하면 안 된다.

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
  → 기존 SelectPattern의 forced audition 경로로 B 시작
```

세계 commit 실패로 room not-ready가 되면 Refresh/승격/성공 snapshot에 도달하지 않는다.

`Try_PromoteValtanNextPattern`은 다음 순서로 구현한다.

1. Next 존재, A COMPLETED_HOLD, exact predecessor receipt COMPLETED 확인.
2. owner/boss/pin/target pattern/expected sequence를 다시 확인한다.
3. 기존 engage 범위·living combat-ready target 조건을 평가한다. 없으면 hold를 유지하고
   WAITING_FOR_PLAYER를 상태 edge에서 한 번 보낸다. 반복 로그나 15초 timeout을 만들지 않는다.
4. current A anchor를 B의 PENDING state로 교체하고, B request identity·같은 epoch·같은 pin·다음 sequence를
   설정한다. `PendingPatternIds`에 B를 정확히 한 번 추가한다.
5. audition override는 유지하고 hold만 해제한다. Product 자동 selector/pursuit가 끼어들지 않는다.
6. reservation은 이 승격 transaction에서 소비된다. Client는 B ACTIVE를 받은 뒤 카드의 대상을 current로 옮긴다.

승격 직후 다음 tick 전 target이 사라지는 race도 처리한다. B가 아직 BeginPattern되지 않은 경우에는
등록된 B pending과 pin을 보존하고 hold/WAITING_FOR_PLAYER로 돌아간다. Product로 fall-through하거나,
pending ID 삭제를 B 완료로 오판하지 않는다. 이미 B ACTIVE 뒤의 실패는 정상 ABORTED 정책이다.

이 함수가 호출해서는 안 되는 경로:

- `Place_PlayerAtValtanAuditionBait`
- `Build_ValtanBossOnlyAuditionReset`, `Reset_ValtanBossOnlyAuditionState`
- `Reset_ValtanAuditionState`, `Reset_ValtanArenaState`
- destruction/prop/navigation/collision/global combat-object reset

### G04-수명·정리

`Build_RequiredPinnedGameplayRevisions`에는 current completed hold 및 Next pin을 포함한다.
`Resolve_ValtanGameplayCatalog`도 이 chain의 pin을 사용한다. R2 publish/activation이 있어도 이미 예약한 B는 R1이다.

`Cancel_ValtanNextPattern`은 owner leave, current abort, boss death/despawn, 명시적 authoritative reset에서
한 번 호출한다. 이 취소를 위해 맵 reset을 새로 호출하지 않는다. owner가 떠나도 다른 참가자가 남으면
현재 A는 정상 종료까지 진행할 수 있으나 Next는 실행하지 않고 terminal hold한다. 기존 마지막 player
이탈 시 room 정리 계약은 유지한다.

`COMPLETED_HOLD`의 owner가 사라지면 late Queue를 거절한다. 새 isolated Play는 기존 준비 절차로 새 chain을 만든다.
Map/Level 이동과 Client inbound generation 변경도 이전 예약을 되살리지 못해야 한다.

## G05. Client 공용 service의 current·next·미확정 요청 분리

### G05-H. `Client/Public/ValtanPatternAuditionService.h`

**소유자:** `CValtanPatternAuditionService` singleton, Client main thread. Boss Tool과 Effect Tool은 읽기/명령 호출자다.

기존 current `VALTAN_PATTERN_AUDITION_SNAPSHOT`을 유지하고 다음 두 상태를 별도로 추가한다.

| 상태 | 필수 내용 | 불변식 |
|---|---|---|
| `VALTAN_NEXT_PATTERN_SNAPSHOT` | 예약 상태, request seq, Client inbound generation, Server epoch, predecessor seq, expected B seq, boss/pattern/consumer IDs, pinned revision, status | Server가 승인한 B만 표시; C 교체 요청을 보냈다는 이유로 먼저 C로 바꾸지 않음 |
| `VALTAN_NEXT_PATTERN_PENDING_COMMAND` | full request, consumer ID, generation, 최초/재시도 시각, WAITING_VERDICT 또는 UNCONFIRMED | 미확정 요청을 보내고 받는 동안 기존 승인 B와 병존 |

Next snapshot의 상태는 `NONE / RESERVED / WAITING_FOR_PLAYER / START_PENDING / REJECTED / ABORTED`로
표현한다. 종료 원인은 status에 남기되 REJECTED/ABORTED를 살아 있는 예약으로 판정하지 않는다.
교체 거절은 기존 B snapshot을 terminal로 바꾸지 않고 control status만 거절로 만든다.
이전 예약의 terminal event가 최신 승인 C를 덮어쓰지 않도록 exact identity로 걸러낸다.

기존 `Submit/Update/Get_Snapshot` 선언 옆에 다음 API를 추가한다.

```cpp
bool_t Queue_NextPattern(
    std::string_view strConsumerId,
    std::string_view strPatternId,
    std::string& strOutStatus);
bool_t Clear_NextPattern(std::string& strOutStatus);
bool_t Retry_NextPatternCommand(std::string& strOutStatus);
const VALTAN_NEXT_PATTERN_SNAPSHOT& Get_NextSnapshot() const;
bool_t Has_PlaybackOwnership() const;
bool_t Has_PendingNextCommand() const;
```

`Queue_NextPattern`은 current의 exact boss/epoch/sequence와 승인 Next의 CAS token을 사용한다.
UI가 임의 tuple을 조립하지 않는다. queue/clear/retry가 current A를 resetful Submit으로 다시 실행하지 않는다.

`Has_PlaybackOwnership`은 current in-flight, 승인 Next 대기/시작 중, 미확정 control을 포함한다.
`COMPLETED + Next 없음 + 미확정 없음`만 있으면 false다. 이 predicate를 Repeat·새 Play·Flow Start가 공유한다.
Queue 자체는 current in-flight여도 허용해야 하므로 Submit용 predicate를 그대로 Queue 거절 조건으로 쓰지 않는다.

### G05-CPP. `Client/Private/ValtanPatternAuditionService.cpp`

**책임:** Server verdict/lifecycle을 exact identity로 소비하여 UI에 권위 있는 current/next 상태를 제공한다.

현재 `Update`의 result/lifecycle 처리부를 `Apply_ServerResult`, `Apply_ServerLifecycle`, `Advance_Timeouts`
의 세 의미 단위로 나눈다. production Update와 실행형 harness가 같은 함수를 사용한다.

1. disconnect/inbound generation 변경을 먼저 처리하고 current·Next·미확정 요청을 모두 종료한다.
2. 전용 result queue를 끝까지 drain한다. Play와 Queue/Clear를 operation으로 나누고 full echo를 검사한다.
3. Queue QUEUED verdict 또는 exact NEXT_RESERVED lifecycle로 B의 승인 identity를 확정한다.
4. lifecycle queue를 끝까지 drain한다. current A, 승인 B, 아직 verdict를 기다리는 exact candidate 순서로 매칭한다.
5. A COMPLETED 뒤에도 같은 Update를 중단하지 않는다. 이어지는 B PENDING/ACTIVE를 소비한다.
6. B PENDING/WAITING은 카드의 예약을 유지한다. B ACTIVE에서 current를 B로 승격하고 그 exact Next를 비운다.
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

`Render_PatternFlowTab` 관련 private 선언 근처에 다음 함수를 추가한다.

```cpp
void Render_NextPatternCard();
void Render_NextPatternPicker();
```

`Render_NextPatternCard`는 service snapshot과 read-only graph만 읽는다. `m_FlowDocument`를 받지 않는다.
`Render_NextPatternPicker`는 stable ID를 service에 제출하는 UI다. Add_Slot/Save/Start_Flow를 호출하지 않는다.

### G06-CPP. `Client/Private/BossTool.cpp`

현재 `Render_PatternFlowTab`의 graph unavailable return, Flow document unavailable return과
`Render_FlowSelectedSlot`의 null-flow return 때문에 함수 끝에 카드만 덧붙이면 요구를 만족하지 못한다.
문서 영역의 오류는 그 영역의 empty state로 처리하고, 오른쪽 열 자체를 return하지 않게 나눈다.

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
| [Valtan.gameplay.json:750](C:/Users/user/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:750) | `VALTAN_FIST_IN_OUT`, entry `valtan.attack.fist-in-out.inner`, INNER 한 stage 2600ms |
| hit | `RING`, inner 8m / outer 16m, 1600ms에 1회, `damage.valtan.fist-in-out` |
| [Valtan.presentation.json:563](C:/Users/user/Desktop/LostArk/Data/Valtan/Valtan.presentation.json:563) | `animation: { mode: NONE }` |
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
`Test_ValtanAuditionProtocol` 옆에 `Test_ValtanNextPatternProtocol`을 추가하고 main 호출에 등록한다.

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

현재 header의 `LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS` 후크는 있지만 이를 실제 빌드하는 독립
프로젝트는 확인하지 못했다. 정적 문자열 테스트만으로 lifecycle 경합이 해결됐다고 판정하지 않는다.

추가할 파일은 다음 세 개다. 모두 harness에 실제 소비자가 있다.

```text
Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp
Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
```

프로젝트는 x64 Debug/Release, Console, v143, C++20, PCH 미사용, `/utf-8`로 구성한다. 현재
ActionPresentationTimelineHarness처럼 production CPP를 link item으로 직접 컴파일하되 복사본을 만들지 않는다.
`Client/Private/ValtanPatternAuditionService.cpp`와 harness main이 compile item이며 Shared project를 reference한다.
include는 Client/Public, Shared/Public, 기존 EngineSDK/Inc를 사용한다. runtime socket/renderer/Client.exe는 실행하지 않는다.

service CPP에서 NetworkManager/HUD/clock 호출만 작은 transport seam으로 분리한다. harness define에서는
결정적 clock, connected/generation, presentation revision availability, 수신 result/lifecycle queues,
send 성공/실패와 발신 request vector를 주입한다. **Submit/Queue/Clear/Retry/Apply_ServerResult/
Apply_ServerLifecycle/timeout/소유권 판정 본문은 production과 동일 코드**다. harness 전용 가짜 상태 머신은 금지한다.
기존 두 Harness method를 이 입력 seam으로 확장하고 production build에서는 노출하지 않는다.

`Framework.sln`에 새 프로젝트와 Debug/Release x64 configuration mapping을 등록하고 `.filters`에는 실제
Private 소스·linked Client 소스만 추가한다. 기존 filter를 재배치하지 않는다.
`Tools/Build/Invoke-BuildAndRegression.ps1`에 build와 실행 gate를 등록하여 수동으로만 실행되는 테스트를 만들지 않는다.
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
보존하며 Next 구현에서 다시 현재 JSON 순서에 고정하지 않는다. 아래 Next UI 관련 회귀는 아직 구현 전이다.

[test_valtan_boss_tool_pattern_flow_contract.py:159](C:/Users/user/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py:159)의
현재 editable JSON=초기 seed 강제 assertion을 분리한다.

- 실제 저장본 테스트: schema/admission/unique slot/max32/monotonic ordinal만 검증.
- seed 테스트: 별도 in-memory seed fixture를 생성해 초기 inventory 순서 검증.
- mutation fixture: reorder, duplicate pattern, slot 삭제, 빈 draft, 새 ID 발급·Save/Reload roundtrip 검증.
- 사용자 현재 JSON을 28개 seed나 이번 29개 순서로 강제 재작성하지 않음.

기존 boss_tool_contract/pattern_flow_contract/balance_tool_contract에는 new UI/service route, Repeat guard,
Flow 오류 상태 카드 존속, Next→Add_Slot 미호출, Clear full tuple, Release gate를 추가한다.
기존 pattern_tree와 All Effects 28개 inventory 테스트는 그대로 통과시킨다.

도넛은 기존 `test_fist_in_out_projects_one_animation_free_stage_and_stage_clock_cue`와
`test_animation_none_and_stage_clock_cue_tagged_unions_fail_closed`를 유지하고 Next inventory 포함,
NONE pose 유지 뒤 다음 clip resume, duplicate cue occurrence 방지와 current anchor 사용을 보강한다.

## G09. 빌드·검증·인계

### 자동 검증

아래는 **구현 후 실행할 명령**이다. 이 계획서에서 PASS로 선언하지 않는다.

```powershell
python -B -m unittest Tools.ValtanPipeline.test_valtan_boss_tool_contract Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Sync-EffectDataProject.ps1 -Check
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git diff --check
```

정본 build가 Engine → UpdateLib → Shared/ProtocolHarness → Server/contract → Client와 등록된 harness를
실제로 통과했는지 configuration별로 기록한다. JSON/XML parse, new project/filter registration도 검사한다.
이미 알려진 unrelated failure가 있으면 exact 이름·로그·범위를 분리하고 전체 PASS로 바꾸지 않는다.

### 사용자 직접 확인 절차

에이전트는 Client/UI를 자율 실행·조작·캡처하지 않는다. 준비된 Debug Server+Client를 사용자가 직접
`Ctrl+F5`로 시작한다. Client 작업 디렉터리는 `Client/Default`다.

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
public 문서 `.md/TEAM/보스툴.md`는 구현 후 Next 목록과 기존 28개 authoring 목록의 차이를 갱신한다.
AGENTS/CLAUDE에 날짜별 작업 로그를 추가하지 않는다.
