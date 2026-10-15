# G1·G2. 진짜 세이튼·조커 위치 랜덤 — 직접 반영 코드

작성 기준: 2026-10-14, 구조 분리 뒤의 파일 경로. 이 폴더의 C++은 수동 구현용 후보이며 제품 소스·Data·Bin에는 반영하지 않았다. 실행된 검증과 남은 확인을 아래에 구분한다. 다른 세션 변경을 보존하기 위해 `manifest.json`의 before hash와 직접 반영 직전 현재 파일을 비교해야 한다. 전체 파일을 오래된 사본으로 덮어쓰지 말고 불일치하면 해당 변경 블록만 최신 코드에 병합한다.

## G1. GameRoom_KoukuMechanics.cpp — 본체 한 자리와 분신 세 자리를 함께 결정

파일: `Server/Private/GameRoom_KoukuMechanics.cpp`.
작업: 기존 `CGameRoom::Commit_KoukuMechanicTriggers` 마지막 REAL_GAZE_TELEPORT 처리 블록 교체.
기준점: `clonePattern`과 `baseCatalog`, `trigger.ClockHours.size() != 3u` 검사를 통과한 바로 다음의 `// 진짜 세이튼의 저작 위치`부터 시작한다.

이 함수는 Room이 pending trigger의 owner ID·pattern sequence를 재확인한 뒤, 개체 순회 밖에서 분신을 생성하는 기존 경계다. Brain이나 Client에 별도 랜덤 상태를 추가하지 않는다. 기존 헤더와 project/filter 등록은 유지한다. 분리 파일에 `<array>`, `<cstdint>`, `<cstddef>` 및 GameRoom_Internal.h가 이미 있어 새 include도 필요하지 않다.

작성 순서는 다음 네 교체 블록이다. 각각 `.before.txt`에 기존 블록, `.after.txt`에 생략 없는 교체 블록이 있다. 전체 파일 정본은 `candidate/Server/Private/GameRoom_KoukuMechanics.cpp`다.

| 순서 | 교체 블록 | 기준점과 책임 |
|---|---|---|
| 1 | G1-position-selection | 기존 radius/anchor 계산부터 `std::vector<SERVER_WORLD_ENTITY> clones;` 바로 앞까지. 네 후보 검증과 한 번의 본체 선택 |
| 2 | G1-clone-loop | `for (const std::uint32_t hour : trigger.ClockHours)`와 다음 angle 선언. 네 슬롯 중 본체 슬롯만 건너뜀 |
| 3 | G1-clone-position | 기존 placement.fPositionX/Y/Z 세 줄. 같은 후보 배열을 분신 배치에도 사용 |
| 4 | G1-owner-commit | reserve 후 기존 `committedOwner` 좌표/yaw 대입. 선택한 본체 위치와 그 자리의 중앙 방향 반영 |

`clockHours={1, ClockHours[0], ClockHours[1], ClockHours[2]}`는 저작 기준점과 상대 자리다. 현재 Data는 `{1,4,7,10}`이므로 90도 간격이다. 배열 index는 이번 함수의 선택용일 뿐 저장 ID가 아니다. `positions`는 월드 m 좌표이고 중심은 owner의 spawn XZ다. 원래 1시 좌표는 sin/cos로 다시 계산하지 않아 float 값까지 보존한다. `anchor=atan2(dx,dz)`는 원래 자리의 월드 XZ 각도이고 `hour-1`에 30도를 곱해 나머지를 구한다.

`selectionTick`은 pattern start tick+저작 trigger startMs의 30Hz 올림값이다. 호출 지연으로 바뀔 수 있는 현재 serverTick 대신 이 예정 tick을 seed에 넣는다. `Hash_StableId(trigger.strTriggerId)`, 서버 owner NetEntityId, 서버 증가 pattern sequence와 함께 기존 `Mix_DeterministicRandom`으로 섞고 네 슬롯 중 하나를 선택한다. 같은 실행을 다시 계산하면 같은 선택이며, 서로 다른 실행에도 같은 자리가 연속으로 나올 수 있다. 보안 난수나 매번 다른 자리 보장이 아니다.

본체와 분신 모두 `atan2(centerX-positionX,centerZ-positionZ)*180/pi + faceCenterYawOffsetDegrees`를 사용한다. 현재 -90도는 Saydon 모델 정면과 yaw의 기존 보정이다. 기존 P5 분신 애니메이션, 뒤돌기 clip, 재생 시작 tick은 유지한다. `strPlacementId`는 기존 owner placement+pattern sequence+실제 clock hour이며, 새로 비는 1시도 `.1`로 유일하게 구분한다.

네 위치의 finite/시계 중복/navigation 검증 → 분신 세 개의 Build_WorldEntity·Begin_Pattern·spawn payload 준비 → vector reserve → owner 재획득 → 본체 위치와 분신들 commit 순서다. 정상적인 검증 실패에서는 본체 좌표·분신 목록·next NetEntityId가 바뀌지 않는다. 기존 trigger 소비 규칙은 실패한 pending을 재시도하지 않는다. 기존 allocation/network 예외까지 새로 rollback하는 변경은 아니다.

`fSpawnPosition`은 중심으로 계속 쓰므로 바꾸지 않는다. 새 root-motion 원점 상태도 필요하지 않다. 현재 KoukuSaydonBrain::Validate_AnimationOnlyPattern은 REAL_GAZE_TELEPORT와 RootMotion 혼용을 거부한다(현재 CPP 146~166행). 실제 gaze 판정은 LogicRuntime::Judge_Gaze가 boss의 현재 위치를 읽는다. 본체는 기존 WorldEntity snapshot, 분신은 기존 spawn/snapshot 경로로 전달하므로 Client에 재추첨을 넣지 않는다.

### G1 기존 제품 계약 테스트의 고정 좌표 기대도 바꾸기

`Server/Private/ServerGameplayContractTests_KoukuProduct.cpp`의 `exerciseProductionGaze`는 현재 owner XZ가 반드시 teleport 기준점과 같다고 검사한다(기준본 2128~2130행). 랜덤 이후에는 이 기대가 틀리다. 전체 수정 후보도 candidate에 보관했다.

`G1-test-slot-oracle`, `G1-test-owner-position`, `G1-test-unique-clones` 순서로 교체한다. 네 저작 슬롯 중 하나인지 검사하고 본체+분신 세 개의 위치 중복을 거부한다. 기존 navigation, inward yaw, spawn owner/sequence, P5 시작 시각, one-shot, 분신 준비 실패 보존, 종료 시 제거 검사는 유지한다. 이 제품 테스트 후보는 문법 검사 대상으로 준비했으며 실제 제품 실행은 하지 않았다.

## G2. KoukuSaydonLogicRuntime.h/.cpp — 조커 ID를 유지하고 배치와 판정을 함께 이동

파일: `Server/Public/KoukuSaydonLogicRuntime.h`와 `Server/Private/KoukuSaydonLogicRuntime.cpp`.
작업: 기존 ledger에 실행별 offset map을 추가하고 Build/Update의 두 소비 지점을 연결한다.
전체 파일은 candidate에 있다. 새 파일·패킷·publisher/schema·Client 선언은 필요하지 않다. H는 현재 UTF-8 BOM·CRLF, CPP는 UTF-8 무BOM·CRLF를 유지했다.

| 순서 | 교체 블록 | 정확한 기준점 |
|---|---|---|
| 1 | G2-ledger-state | KOUKUSAYDON_LOGIC_LEDGER의 기존 `std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;` 바로 위 |
| 2 | G2-direct-includes | CPP의 기존 `#include <algorithm>` 교체. helper가 직접 쓰는 array/cstddef/cstdint/iterator/map/string/utility 명시 |
| 3 | G2-build-helper | CPP 최초 익명 namespace의 `float Wrap180(float degrees) noexcept` 바로 위. 기존 Mix 바로 뒤 |
| 4 | G2-build-commit | CKoukuSaydonLogicRuntime::Build 끝 `outLedger = std::move(ledger);` 바로 위에 helper 호출 |
| 5 | G2-world-play-position | Update의 최초 world cue에서 `play.Placement = sequence.Placement;` 바로 다음 |
| 6 | G2-contact-position | OBJECT_CONTACT의 기존 `BODY_CIRCLE_XZ circle{ target.fWorldX, target.fWorldZ, target.fRadiusM };` 한 줄 교체 |

`WorldPositionOffsets`는 stable World occurrence ID→이번 실행의 월드 XYZ 이동량(m) map이다. 정의 Catalog를 변경하지 않는다. Room이 소유하는 pattern ledger의 생성/종료 수명을 따르고 Room tick 스레드만 접근한다. Build는 먼저 새 지역 ledger를 만들기 때문에 재시작할 때 이전 실행 map이 남지 않는다.

helper는 displayName·vector 순번·P13 문자열을 기준으로 고르지 않는다. OBJECT_CONTACT의 COMPLETE_LOGIC_WINDOW 결과가 가리키는 occurrence가 joker_card인지 확인하고, 같은 ContactTargets의 일곱 ID로 WorldSequences를 찾는다. 현재 실데이터의 조커는 `KOUKUSAYDON_G1_PATTERN_13.world.11`; 일반 카드는 world.5~10이다. parent pattern에 occurrence prefix가 붙어도 전달된 ID 그대로 찾는다.

현재 후보는 일반 카드 6개+조커 1개, 고유 occurrence, 명시 Placement, 유한 양수 scale, 같은 rotation/scale/startMs/durationMs, 별도 spawn anchor/offset/CombatBody/SupportWindows 없음이라는 정적 카드 계약만 허용한다. 현재 P13은 이 조건을 만족한다. 이 제한 덕분에 접촉 원에 template local offset이 구워져 있더라도 평행이동으로 보존할 수 있다. 앞으로 회전·배율이 서로 다른 카드를 허용하려면 기존 world 접촉점을 원래 TRS의 역변환으로 local로 되돌린 후 새 TRS로 보내고 radius/shape 정책까지 정해야 한다. 현재 코드는 그 경우 원래 배치를 유지한다.

일곱 카드 후보를 stable ID 순서의 map으로 정렬하고 조커 ID byte hash+owner NetEntityId+pattern sequence+startTick으로 한 슬롯을 선택한다. 원래 조커 자리도 후보이므로 전체 7자리다. 조커와 선택 일반 카드의 XYZ만 교환하고, 모든 occurrence ID·model instance ID·flip/hop instance ID·성공 result ID는 그대로 둔다. 지역 staged map을 모두 준비한 뒤 ledger map에 넘긴다. 누락/중복/지원하지 않는 변환이면 그 카드 묶음에 offset을 하나도 넣지 않고 기존 저작 배치를 유지한다. Build의 반환형을 바꾸거나 패턴 전체를 취소하는 정책은 추가하지 않았다.

Server 최초 WorldSequencePlays는 원래 Placement 복사본에 offset을 더한다. OBJECT_CONTACT의 target 중심에도 같은 occurrence map의 XZ를 더한다. 카드의 반경과 motion/result 선택은 그대로다. 따라서 새 위치의 조커를 타격하면 world.11 성공이 발생하고, 원래 조커 자리에 옮겨온 일반 카드는 일반 ID로 판정한다. 같은 pattern의 반복 hammer/hop 창들도 같은 map을 읽는다.

실제 기존 복제 연결은 `GameRoom_KoukuPlayerCommands.cpp:892~930`이다. message에 Placement XYZ·rotation·scale을 넣고 WorldCueByOccurrence에 stable ID→cue를 보관한 뒤 Broadcast_OwnedWorldSequence로 전달한다. Client의 `Level_KoukuSaydonArena.cpp:121` WorldPlacementFromCue와 `:6320` 이후 PrepareCompositionWorld/PlayCompositionWorld가 그 좌표를 소비한다. 이후 flip/hop은 strTargetWorldOccurrenceId로 기존 cue를 찾아 같은 객체의 motion을 교체한다. Client 랜덤은 필요하지 않다. 현재 큐별 복제 계약을 재사용하며 새 7카드 네트워크 원자 커밋 메시지는 추가하지 않는다.

## 확인된 검증과 남은 확인

- `check-syntax.cmd`: 실제 VS 18 Insiders MSVC `/std:c++20 /Zs`로 후보 CPP를 컴파일 문법 검사한다. 결과는 `syntax.log`. out 후보에 대한 검사이며 제품 링크/배포 완료라는 뜻은 아니다.
- `check-contract.cmd`: 실제 후보 helper 함수 원문과 현재 Encounter P13의 카드 위치/접촉 ID로 만든 C++ 실행 테스트. 512 seed에서 일곱 자리 모두 도달, 같은 입력 반복 안정성, 카드 위치 중복 없음, 입력 vector 역순에도 같은 결과, 누락 Placement/중복 ID/회전 불일치/CombatBody의 실패 보존, parent prefix ID를 확인했다. `contract.log`에 PASS. 이 검증은 helper이며 전체 Room tick·Client 화면 검증은 아니다.
- 제품 고정 좌표 기대 테스트의 교체 코드는 준비했다. G1 반영 후 기존 Kouku product 계약 테스트와 실제 네 후보 navigation/spawn 소유·sequence·gaze 방향·중단/재시작을 확인한다. 강제 slot 반복 검증 또는 여러 실행에서 4자리 coverage 확인이 필요하다.
- G2 반영 후 현재 P13 실 Room에서 새 조커 위치 성공, 옛 위치 일반 카드, flip/hop priority, timeout, 재시작, 늦은 접속의 cue 재생 위치를 확인한다. 기존 JokerAim 테스트가 고정 타격 방향을 사용하는 과정에서 조커를 조기 발견할 수 있으므로 실제 결과를 보고 fixture 목적에 맞는 방향/완료 기대를 검토한다. 실패를 숨기기 위해 조커 성공을 비활성화하지 않는다.
- Client/UI 실행과 실제 gameplay 변경은 하지 않았다. Data/JSON은 변경하지 않았으므로 이 두 후보만 적용할 때 domain publish가 필요한 변경은 없다. 사용자 적용 후 Server를 다시 빌드하고 실행 중 프로세스와 새 EXE의 구분을 확인해야 한다.

`manifest.json`은 전체 후보 네 파일과 기준본 hash를 보관한다. 제품 파일을 자동 적용하는 기능은 이 폴더의 스크립트에 없다.


## 파일별 교체 기준과 코드

이후 블록은 제품에 미적용이다. G0 구조 분리를 완료한 현재 파일을 기준으로 한다. 뒤의 전체 파일은 대조용 정본이며, 다른 수정이 생기면 아래의 변경 블록을 최신 파일에 병합한다.


### Server/Private/GameRoom_KoukuMechanics.cpp

실제 수정 대상: [GameRoom_KoukuMechanics.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuMechanics.cpp)


#### G1-position-selection

현재 기준 블록:

```cpp
        // 진짜 세이튼의 저작 위치를 원의 기준점으로 삼고 가짜 세 배우의 상대 시계 방향을 계산
        const float dx = trigger.fTeleportX - owner->fSpawnPositionX;
        const float dz = trigger.fTeleportZ - owner->fSpawnPositionZ;
        const float radius = std::sqrt(dx * dx + dz * dz);
        if (!std::isfinite(radius) || radius <= 0.01f ||
            (m_ServerNavigation.Is_Loaded() &&
             !m_ServerNavigation.Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ)))
        {
            m_strStatus = "KoukuSaydon teleport preserved the boss: target is not walkable";
            continue;
        }
        const float anchor = std::atan2(dx, dz);
```

교체 블록:

```cpp
        // 저작 1시 좌표와 spawn 중심으로 네 자리를 준비한다. 순번은 이번 계산용이며 저장 ID가 아니다.
        const float dx = trigger.fTeleportX - owner->fSpawnPositionX;
        const float dz = trigger.fTeleportZ - owner->fSpawnPositionZ;
        const float radius = std::sqrt(dx * dx + dz * dz);
        if (!std::isfinite(radius) || radius <= 0.01f)
        {
            m_strStatus = "KoukuSaydon teleport preserved the boss: radius is invalid";
            continue;
        }
        const float anchor = std::atan2(dx, dz);
        const std::array<std::uint32_t, 4u> clockHours{
            1u, trigger.ClockHours[0u], trigger.ClockHours[1u], trigger.ClockHours[2u]};
        const auto positionAtHour = [&](const std::uint32_t hour)
        {
            // 원래 1시는 삼각함수로 재계산하지 않아 기존 저작 좌표의 float 값을 보존한다.
            if (hour == 1u)
            {
                return SERVER_NAV_POINT{trigger.fTeleportX, trigger.fTeleportY, trigger.fTeleportZ};
            }
            const float angle = anchor + static_cast<float>(hour - 1u) * 0.52359877559829887f;
            return SERVER_NAV_POINT{owner->fSpawnPositionX + std::sin(angle) * radius,
                                    trigger.fTeleportY,
                                    owner->fSpawnPositionZ + std::cos(angle) * radius};
        };
        std::array<SERVER_NAV_POINT, 4u> positions{};
        bool positionsValid = std::isfinite(trigger.fFaceCenterYawOffsetDegrees);
        for (std::size_t slot = 0u; slot < clockHours.size(); ++slot)
        {
            const auto hour = clockHours[slot];
            if (hour < 1u || hour > 12u ||
                std::find(clockHours.begin(), clockHours.begin() + slot, hour) != clockHours.begin() + slot)
            {
                positionsValid = false;
                break;
            }
            positions[slot] = positionAtHour(hour);
            const auto& position = positions[slot];
            if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
                (m_ServerNavigation.Is_Loaded() &&
                 !m_ServerNavigation.Is_PointWalkableExact(position.x, position.z)))
            {
                positionsValid = false;
                break;
            }
        }
        if (!positionsValid)
        {
            m_strStatus = "KoukuSaydon teleport preserved all actors: a candidate position is invalid";
            continue;
        }
        // 처리 지연 tick 대신 패턴 시작과 저작 trigger 시각을 사용한다. 같은 실행을 재계산해도 선택은 같다.
        const std::uint32_t selectionTick = CKoukuSaydonLogicRuntime::Add_Ticks(
            owner->iPatternStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iStartMs));
        const std::uint64_t seed = Mix_DeterministicRandom(
            Hash_StableId(trigger.strTriggerId) ^ (static_cast<std::uint64_t>(owner->iNetEntityId) << 32u) ^
            owner->iPatternSequence ^ selectionTick);
        const std::size_t realSlot = static_cast<std::size_t>(seed % positions.size());
        const auto& realPosition = positions[realSlot];
```

#### G1-clone-loop

현재 기준 블록:

```cpp
        for (const std::uint32_t hour : trigger.ClockHours)
        {
            const float angle = anchor + static_cast<float>(hour - 1u) * 0.52359877559829887f;
```

교체 블록:

```cpp
        for (std::size_t slot = 0u; slot < clockHours.size(); ++slot)
        {
            if (slot == realSlot)
            {
                continue;
            }
            const std::uint32_t hour = clockHours[slot];
            const auto& position = positions[slot];
```

#### G1-clone-position

현재 기준 블록:

```cpp
            placement.fPositionX = owner->fSpawnPositionX + std::sin(angle) * radius;
            placement.fPositionY = trigger.fTeleportY;
            placement.fPositionZ = owner->fSpawnPositionZ + std::cos(angle) * radius;
```

교체 블록:

```cpp
            placement.fPositionX = position.x;
            placement.fPositionY = position.y;
            placement.fPositionZ = position.z;
```

#### G1-owner-commit

현재 기준 블록:

```cpp
        // reserve 뒤 다시 찾은 본체에 저작 위치를 적용하고 준비된 세 분신을 추가
        committedOwner.fPositionX = trigger.fTeleportX;
        committedOwner.fPositionY = trigger.fTeleportY;
        committedOwner.fPositionZ = trigger.fTeleportZ;
        committedOwner.fYawDegrees = std::atan2(-dx, -dz) * 57.29577951308232f + trigger.fFaceCenterYawOffsetDegrees;
```

교체 블록:

```cpp
        // 세 분신 준비가 모두 성공한 뒤 선택한 본체 위치를 적용한다. spawn 중심은 다음 실행에도 유지한다.
        committedOwner.fPositionX = realPosition.x;
        committedOwner.fPositionY = realPosition.y;
        committedOwner.fPositionZ = realPosition.z;
        committedOwner.fYawDegrees =
            std::atan2(committedOwner.fSpawnPositionX - realPosition.x,
                       committedOwner.fSpawnPositionZ - realPosition.z) * 57.29577951308232f +
            trigger.fFaceCenterYawOffsetDegrees;
```


#### GameRoom_KoukuMechanics.cpp 전체 코드

```cpp
#include "GameRoom.h"
#include "GameRoom_Internal.h"
#include "KoukuSaydonBrain.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerCombatHitRuntime.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace GameRoomDetail;

// 기믹 commit 또는 소환 배우의 시작 trigger가 호출한다. 플레이어 선택·도약·등장·착지를 같은 패턴 실행 상태로 진행한다.
// 보스 사본에서 navigation·몸체·root motion을 검증하고 성공 위치를 본체에 반영한다. 실패 이유는 m_strStatus에 남긴다.
bool LostArk::Server::CGameRoom::Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
                                                            const BOSS_PATTERN_DEFINITION& pattern,
                                                            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
                                                            const std::uint32_t serverTick)
{
    using Phase = ALBION_AIRBORNE_PHASE;
    using namespace LostArk::Shared;
    const auto reject = [&](const char* reason)
    {
        m_strStatus = std::string("Albion airborne preserved its pose: ") + reason;
        return false;
    };
    if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
        boss.strPatternId != pattern.strPatternId || boss.iPatternStageIndex >= pattern.Stages.size() || !serverTick)
    {
        return reject("owner or stage is unavailable");
    }
    // 현재 boss를 직접 이동하기 전에 후보 상태로 높이와 root 기준을 검증
    auto staged = std::make_unique<SERVER_WORLD_ENTITY>(boss);
    auto& state = staged->AlbionAirborne;
    if (state.iPatternSequence != boss.iPatternSequence)
    {
        state = {};
        state.iPatternSequence = boss.iPatternSequence;
    }
    const auto eligible = [](const SERVER_PLAYER& player)
    {
        return player.Is_Human() && player.isCombatReady && player.iCurrentHp && player.iMarioStage == 0u &&
               player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING &&
               player.eAction != PLAYER_ACTION_STATE::GRABBED;
    };
    const auto selectedPlayer = [&]() -> SERVER_PLAYER*
    {
        for (auto& [id, player] : m_Players)
        {
            if (player.iNetEntityId == state.iSelectedPlayer && eligible(player))
            {
                return &player;
            }
        }
        return nullptr;
    };
    if (trigger.eAirbornePhase == Phase::SELECT_PLAYER)
    {
        auto* selected = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "albion.airborne.player", serverTick);
        state.iSelectedPlayer = selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
        state.bHasSelectedGround = false;
        if (trigger.bCaptureAirborneTargetPosition || !selected)
        {
            // An empty arena still completes its authored sequence at a fixed ground
            // point. Mario players belong to another area and never supply this anchor.
            const float x = selected ? selected->fPositionX : boss.fPositionX;
            const float z = selected ? selected->fPositionZ : boss.fPositionZ;
            if (!m_ServerNavigation.Is_Loaded() || !m_ServerNavigation.Is_PointWalkableExact(x, z) ||
                !m_ServerNavigation.Sample_Position(x, z, state.SelectedGround) ||
                !std::isfinite(state.SelectedGround.y))
            {
                return reject("selected navigation ground is unavailable");
            }
            state.SelectedGround.x = x;
            state.SelectedGround.z = z;
            state.bHasSelectedGround = true;
            if (!trigger.strSelectedEffectVisualId.empty())
            {
                const auto* catalog = Resolve_KoukuProductCatalog();
                if (!catalog)
                {
                    return reject("selected Effect catalog is unavailable");
                }
                SERVER_COMBAT_OBJECT_LOCKED_TARGET target;
                target.iNetEntityId = state.iSelectedPlayer;
                target.fPositionX = state.SelectedGround.x;
                target.fPositionY = state.SelectedGround.y;
                target.fPositionZ = state.SelectedGround.z;
                BOSS_COMBAT_OBJECT_DEFINITION definition;
                definition.strEncounterId = boss.strEncounterId;
                definition.strOwnerPatternId = pattern.strPatternId;
                definition.strOwnerStageActionId = trigger.strTriggerId;
                definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
                definition.strClientVisualId = trigger.strSelectedEffectVisualId;
                definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
                definition.iLifeMs = trigger.iSelectedEffectLifetimeMs;
                definition.AttackTemplates = trigger.FixedHits;
                definition.PresentationPulses.push_back({"combatpresentation.kouku.showtime.started", 0u});
                auto transaction = m_CombatObjectRuntime.Begin_Transaction();
                std::string status;
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  boss,
                                                                  &target,
                                                                  definition,
                                                                  nullptr,
                                                                  *catalog,
                                                                  1u,
                                                                  serverTick,
                                                                  status))
                {
                    m_strStatus = "Selected airborne Effect preserved capture: " + status;
                    return false;
                }
                auto& object = transaction.Objects.back();
                object.LiveState.CurrentPose.fYawDegrees = 0.f;
                object.LiveState.CurrentPose.fDirectionX = 0.f;
                object.LiveState.CurrentPose.fDirectionZ = 1.f;
                object.LiveState.PreviousPose = object.LiveState.CurrentPose;
                transaction.Spawned.back().fYawDegrees = 0.f;
                if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
                {
                    return reject("selected Effect transaction changed");
                }
            }
        }
        boss.AlbionAirborne = state;
        boss.iTargetEntityId = boss.iPatternTargetEntityId = state.iSelectedPlayer;
        return true;
    }
    SERVER_NAV_POINT oldGround{}, newGround{};
    if (!m_ServerNavigation.Is_Loaded() ||
        !m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, oldGround))
    {
        return reject("current navigation ground is unavailable");
    }
    const double patternMs = CKoukuSaydonBrain::Pattern_ElapsedMs(boss, serverTick);
    const double stageMs = CKoukuSaydonBrain::Stage_RootTimeMs(boss, pattern, serverTick);
    const float sourceUp = CKoukuSaydonBrain::Sample_StageRootMotion(boss.PatternStageRootMotion, stageMs).fUp;
    state.ePhase = trigger.eAirbornePhase;
    state.iStartMs = trigger.iStartMs;
    state.iDurationMs = trigger.iAirborneDurationMs;
    state.iSourceStageIndex = boss.iPatternStageIndex;
    state.fStartHeightM = boss.fPositionY - oldGround.y;
    state.fPhaseHeightM = trigger.fAirborneHeightM;
    state.fLandingProgress = 0.f;
    float x = boss.fPositionX, z = boss.fPositionZ;
    if (state.ePhase == Phase::JUMP)
    {
        if (!std::isfinite(trigger.fAirborneHeightM) || trigger.fAirborneHeightM <= 0.f ||
            trigger.fAirborneHeightM > 100000.f || trigger.iAirborneDurationMs > 600000u)
        {
            return reject("jump height or duration is invalid");
        }
        state.fJumpHeightM = trigger.fAirborneHeightM;
        // Reuse the actual navigation/body sweep. The same source clock still owns XZ.
        staged->iPatternStageRootLastTick = 0u;
        std::string status;
        if (!CKoukuSaydonBrain::Apply_StageRootMotion(*staged,
                                                      pattern,
                                                      serverTick,
                                                      m_ServerNavigation,
                                                      m_ServerCollisionSystem,
                                                      status))
        {
            m_strStatus = "Albion jump preserved its pose: " + status;
            return false;
        }
    }
    else
    {
        if (!std::isfinite(state.fJumpHeightM) || state.fJumpHeightM <= 0.f)
        {
            return reject("initial jump has not committed");
        }
        if (state.ePhase == Phase::APPEAR_PLAYER)
        {
            if (state.fPhaseHeightM <= 0.f || !std::isfinite(state.fPhaseHeightM))
            {
                return reject("appearance height is invalid");
            }
            if (state.bHasSelectedGround)
            {
                x = state.SelectedGround.x;
                z = state.SelectedGround.z;
            }
            else
            {
                auto* selected = selectedPlayer();
                if (!selected)
                {
                    selected =
                        Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "albion.airborne.player", serverTick);
                }
                state.iSelectedPlayer = selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
                if (selected)
                {
                    x = selected->fPositionX;
                    z = selected->fPositionZ;
                }
                else
                {
                    state.SelectedGround = oldGround;
                    state.SelectedGround.x = x;
                    state.SelectedGround.z = z;
                    state.bHasSelectedGround = true;
                }
            }
        }
        else if (state.ePhase == Phase::CENTER)
        {
            x = trigger.fTeleportX;
            z = trigger.fTeleportZ;
        }
        else if (state.ePhase == Phase::SLAM)
        {
            std::uint64_t stageStart = 0u;
            for (std::uint32_t index = 0u; index < boss.iPatternStageIndex; ++index)
            {
                stageStart += pattern.Stages[index].iDurationMs;
            }
            const double triggerAge = (std::max)(0.0, double(trigger.iStartMs) - double(stageStart));
            state.fSourceUpAtStart =
                CKoukuSaydonBrain::Sample_StageRootMotion(boss.PatternStageRootMotion, triggerAge).fUp;
            state.fSourceUpMinimum = state.fSourceUpAtStart;
            for (const auto& sample : boss.PatternStageRootMotion)
            {
                if (sample.iTimeMs >= triggerAge)
                {
                    state.fSourceUpMinimum = (std::min)(state.fSourceUpMinimum, sample.fUp);
                }
            }
            if (state.fSourceUpAtStart - state.fSourceUpMinimum <= .000001f)
            {
                return reject("source animation has no remaining descent");
            }
        }
        else if (state.ePhase != Phase::DISAPPEAR)
        {
            return reject("unknown motion phase");
        }
        if (state.ePhase == Phase::SLAM)
        {
            staged->iPatternStageRootLastTick = 0u;
            std::string status;
            if (!CKoukuSaydonBrain::Apply_StageRootMotion(*staged,
                                                          pattern,
                                                          serverTick,
                                                          m_ServerNavigation,
                                                          m_ServerCollisionSystem,
                                                          status))
            {
                m_strStatus = "Albion slam preserved its pose: " + status;
                return false;
            }
        }
        else
        {
            if (!std::isfinite(x) || !std::isfinite(z) || !m_ServerNavigation.Is_PointWalkableExact(x, z) ||
                !m_ServerNavigation.Sample_Position(x, z, newGround))
            {
                return reject("destination navigation is unavailable");
            }
            if (state.ePhase == Phase::CENTER &&
                (!std::isfinite(trigger.fTeleportY) || std::abs(trigger.fTeleportY - newGround.y) > 1.f))
            {
                return reject("center reference height does not match navigation");
            }
            float height = 0.f;
            if (!CKoukuSaydonBrain::Sample_AlbionAirborneHeight(state,
                                                                patternMs,
                                                                boss.iPatternStageIndex,
                                                                sourceUp,
                                                                height))
            {
                return reject("phase height is invalid");
            }
            // A captured ground point owns the complete anchor, including terrain height.
            if (state.ePhase == Phase::APPEAR_PLAYER && state.bHasSelectedGround)
            {
                newGround.y = state.SelectedGround.y;
            }
            const float y = newGround.y + height;
            if (!std::isfinite(y) || !m_ServerCollisionSystem.Is_CirclePositionClear(x,
                                                                                     y,
                                                                                     z,
                                                                                     boss.fCollisionRadius,
                                                                                     boss.fCollisionRadius,
                                                                                     boss.fCollisionRadius,
                                                                                     boss.iNetEntityId))
            {
                return reject("destination body overlap is invalid");
            }
            // Rebase the existing stage origin, including terrain, so its next sample
            // cannot undo the teleport or introduce a second vertical displacement.
            if (staged->bPatternStageRootOriginCaptured)
            {
                staged->fPatternStageOriginX += x - boss.fPositionX;
                staged->fPatternStageOriginY += y - boss.fPositionY;
                staged->fPatternStageOriginZ += z - boss.fPositionZ;
                staged->fPatternStageRootGroundY += newGround.y - oldGround.y;
            }
            staged->fPositionX = x;
            staged->fPositionY = y;
            staged->fPositionZ = z;
            staged->iPatternStageRootLastTick = serverTick;
        }
    }
    // 검증된 후보를 적용하고 실제 룸 actor일 때만 blocking body 위치를 갱신
    staged->iTargetEntityId = staged->iPatternTargetEntityId = state.iSelectedPlayer;
    boss = std::move(*staged);
    if (std::any_of(m_WorldEntities.begin(),
                    m_WorldEntities.end(),
                    [&](const auto& body)
                    {
                        return &body == &boss;
                    }))
    {
        (void)m_ServerCollisionSystem.Update_BlockingBody(boss.iNetEntityId,
                                                          boss.fPositionX,
                                                          boss.fPositionY + boss.fCollisionRadius,
                                                          boss.fPositionZ);
    }
    return true;
}

// Update_WorldEntities의 entity 순회가 끝난 뒤 호출한다. Logic이 보관한 owner ID·sequence를 다시 확인해 기믹을 반영한다.
// 소환은 m_WorldEntities를 늘릴 수 있으므로 순회 중 실행하지 않는다. 실패한 요청은 기존 상태 보존 경계에서 건너뛴다.
// 본체와 detached tail의 권한을 구분하고, 모든 pending 요청 처리 뒤 큐를 비운다.
void LostArk::Server::CGameRoom::Commit_KoukuMechanicTriggers(const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    // Equal-clock player selection precedes appearance regardless of authored row order.
    std::stable_sort(m_PendingKoukuMechanicTriggers.begin(),
                     m_PendingKoukuMechanicTriggers.end(),
                     [](const auto& a, const auto& b)
                     {
                         if (a.iBossEntityId != b.iBossEntityId)
                         {
                             return a.iBossEntityId < b.iBossEntityId;
                         }
                         if (a.iPatternSequence != b.iPatternSequence)
                         {
                             return a.iPatternSequence < b.iPatternSequence;
                         }
                         if (a.Trigger.iStartMs != b.Trigger.iStartMs)
                         {
                             return a.Trigger.iStartMs < b.Trigger.iStartMs;
                         }
                         const auto priority = [](const auto& t)
                         {
                             return t.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
                                            t.eAirbornePhase == ALBION_AIRBORNE_PHASE::SELECT_PLAYER
                                        ? 0
                                        : 1;
                         };
                         return priority(a.Trigger) < priority(b.Trigger);
                     });
    // This executes outside the world iteration: vector growth cannot invalidate a boss reference.
    for (const auto& pending : m_PendingKoukuMechanicTriggers)
    {
        auto liveOwner = std::find_if(m_WorldEntities.begin(),
                                      m_WorldEntities.end(),
                                      [&pending](const SERVER_WORLD_ENTITY& entity)
                                      {
                                          return entity.iNetEntityId == pending.iBossEntityId;
                                      });
        auto* owner = Find_KoukuOccurrenceOwner(pending.iBossEntityId, pending.iPatternSequence);
        if (!owner || liveOwner == m_WorldEntities.end())
        {
            continue;
        }
        const bool detached = owner != &*liveOwner;
        const auto& trigger = pending.Trigger;
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
        {
            // A Trigger selects exactly once. Detached tails cannot retarget the
            // actor after another Pattern has taken ownership.
            if (detached || !owner->iCurrentHp || owner->eAction == SERVER_ENTITY_ACTION::DEAD)
            {
                continue;
            }
            const auto* selected =
                Select_BossRandomAliveTarget(*owner,
                                             trigger.strTriggerId,
                                             trigger.iStartMs == 0u ? "boss.target.pattern" : "boss.target.random.next",
                                             serverTick);
            owner->iTargetEntityId = owner->iPatternTargetEntityId =
                selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
            owner->bHasPatternTargetLastPosition = selected != nullptr;
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
            {
                owner->Restore_KoukuPresentationAim();
                const auto* product = Resolve_KoukuProductCatalog();
                std::string status;
                const auto* pattern =
                    product ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*product, owner->strPatternId, status)
                            : nullptr;
                if (auto* member = Find_KoukuAuditionMember(owner->iNetEntityId, owner->iPatternSequence))
                {
                    for (auto& window : member->LogicLedger.PlayerTargetWindows)
                    {
                        if (!pattern || window.iTriggerIndex >= pattern->MechanicTriggers.size() ||
                            pattern->MechanicTriggers[window.iTriggerIndex].strTriggerId != trigger.strTriggerId)
                        {
                            continue;
                        }
                        window.iSelectedTargetNetEntityId = selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
                        if (selected)
                        {
                            window.LastSelectedTargetPosition =
                                std::array<float, 3u>{selected->fPositionX, selected->fPositionY, selected->fPositionZ};
                        }
                    }
                }
            }
            if (!selected)
            {
                continue;
            }
            owner->fPatternTargetLastPositionX = selected->fPositionX;
            owner->fPatternTargetLastPositionY = selected->fPositionY;
            owner->fPatternTargetLastPositionZ = selected->fPositionZ;
            // Tracking is presentation-only until END pins the selected point.
            // The following Duration and pattern termination restore this body basis.
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
            {
                continue;
            }
            const float dx = selected->fPositionX - owner->fPositionX, dz = selected->fPositionZ - owner->fPositionZ;
            if (dx * dx + dz * dz > .000001f)
            {
                owner->fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
                if (owner->strArchetypeId == "BOSS_KOUKUSAYDON_G2_BIG_SAYDON" ||
                    owner->strArchetypeId == "BOSS_KOUKUSAYDON_G2_KOUKU")
                {
                    owner->fYawDegrees -= 90.f;
                }
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD)
        {
            Begin_KoukuBingoDuration(*owner, trigger, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            if (!catalog || detached || owner->strArchetypeId != "BOSS_KOUKUSAYDON_BINGO_SAYDON" ||
                !owner->iMaximumHealthBars)
            {
                continue;
            }
            for (auto& [id, player] : m_Players)
            {
                if (!player.iCurrentHp || player.eAction == PLAYER_ACTION_STATE::DEAD)
                {
                    continue;
                }
                if (Is_KoukuRaidRunning() &&
                    std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), id) ==
                        m_KoukuRaid.PlayerIds.end())
                {
                    continue;
                }
                // Only an unexpired earned line or Inanna zone satisfies this mechanic.
                // Generic immunity and item protection cannot turn a failed verdict into success.
                const auto protectedUntil = [&](const std::uint32_t until)
                {
                    return until && !CKoukuSaydonLogicRuntime::Has_ReachedTick(serverTick, until);
                };
                if (protectedUntil(player.iKoukuBingoLineProtectionEndTick) ||
                    protectedUntil(player.iEstherZoneProtectionEndTick))
                {
                    // Emit the same deduplicated combat-text pulse as a safe-zone verdict.
                    player.iInvulnerabilityZoneContactTick = serverTick;
                    player.iInvulnerabilityZonePulseTick = serverTick;
                    continue;
                }
                SERVER_WORLD_TO_PLAYER_HIT wipe{};
                wipe.iRawDamage = player.iCurrentHp;
                wipe.iServerTick = serverTick;
                wipe.bEncounterWipe = true;
                wipe.fSourceX = owner->fPositionX;
                wipe.fSourceZ = owner->fPositionZ;
                (void)CServerCombatHitRuntime::Apply_WorldToPlayer(player, wipe, *catalog, m_TickDamageEvents);
            }
            if (!m_KoukuBingoDuration.bLastLineCompletionSucceeded)
            {
                continue;
            }
            const auto damage = static_cast<std::uint32_t>(
                (std::min)(std::uint64_t(owner->iCurrentHp),
                           std::uint64_t(owner->iMaximumHp) * 13u / owner->iMaximumHealthBars));
            owner->iCurrentHp -= damage;
            if (damage)
            {
                DAMAGE_EVENT event;
                event.iTargetNetEntityId = owner->iNetEntityId;
                event.iAmount = damage;
                event.fPositionX = owner->fPositionX;
                event.fPositionY = owner->fPositionY;
                event.fPositionZ = owner->fPositionZ;
                event.isOutgoing = true;
                m_TickDamageEvents.push_back(event);
            }
            if (!owner->iCurrentHp)
            {
                owner->eAction = SERVER_ENTITY_ACTION::DEAD;
                Notify_KoukuRaidBossDeath(*owner, serverTick);
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            std::string status;
            const auto* pattern =
                catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
            if (!pattern)
            {
                m_strStatus = "Albion airborne preserved its pose: pinned pattern is unavailable: " + status;
                continue;
            }
            (void)Commit_KoukuAlbionAirborne(*owner, *pattern, trigger, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS)
        {
            (void)Commit_KoukuMarioPhasePlayers(*owner, trigger, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER)
        {
            SERVER_NAV_POINT oldGround{}, newGround{};
            const bool faceCenter = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER;
            const bool grounded = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED;
            // Match the existing FaceCenter/Albion center deck admission tolerance.
            constexpr float destinationHeightToleranceM = 1.f;
            const float referenceY = (faceCenter || grounded) ? trigger.fTeleportY : owner->fPositionY;
            if (!std::isfinite(referenceY) || !std::isfinite(trigger.fTeleportX) ||
                !std::isfinite(trigger.fTeleportZ) ||
                !m_ServerNavigation.Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ) ||
                !m_ServerNavigation.Sample_Position(owner->fPositionX, owner->fPositionZ, oldGround) ||
                !m_ServerNavigation.Sample_Position(trigger.fTeleportX, trigger.fTeleportZ, newGround) ||
                ((faceCenter || grounded) && std::abs(referenceY - newGround.y) > destinationHeightToleranceM))
            {
                m_strStatus = "Boss teleport preserved its pose: destination navigation or height is invalid";
                continue;
            }
            const float destinationY = grounded ? newGround.y : referenceY;
            if (!m_ServerCollisionSystem.Is_CirclePositionClear(trigger.fTeleportX,
                                                                destinationY + owner->fCollisionRadius,
                                                                trigger.fTeleportZ,
                                                                owner->fCollisionRadius,
                                                                owner->fCollisionRadius,
                                                                owner->fCollisionRadius,
                                                                owner->iNetEntityId))
            {
                m_strStatus = "Boss XZ teleport preserved its pose: destination navigation or body overlap is invalid";
                continue;
            }
            const float rootX = owner->fPatternStageOriginX + trigger.fTeleportX - owner->fPositionX;
            const float rootZ = owner->fPatternStageOriginZ + trigger.fTeleportZ - owner->fPositionZ;
            const float rootGroundY = owner->fPatternStageRootGroundY + newGround.y - oldGround.y;
            const float rootY = owner->fPatternStageOriginY + (grounded ? destinationY - owner->fPositionY : 0.f);
            if (owner->bPatternStageRootOriginCaptured && (!std::isfinite(rootX) || !std::isfinite(rootZ) ||
                                                           !std::isfinite(rootY) || !std::isfinite(rootGroundY)))
            {
                m_strStatus = "Boss XZ teleport preserved its pose: root origin is invalid";
                continue;
            }
            // XZ keeps root-up; grounded rebases Y by the same committed floor delta.
            // Both preserve stage/action clocks and continue the existing root curve.
            if (owner->bPatternStageRootOriginCaptured)
            {
                owner->fPatternStageOriginX = rootX;
                owner->fPatternStageOriginZ = rootZ;
                owner->fPatternStageRootGroundY = rootGroundY;
                if (grounded)
                {
                    owner->fPatternStageOriginY = rootY;
                }
            }
            if (grounded)
            {
                owner->bPatternRootGrounded = true;
            }
            owner->fPositionX = trigger.fTeleportX;
            owner->fPositionZ = trigger.fTeleportZ;
            if (grounded)
            {
                owner->fPositionY = destinationY;
            }
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER)
            {
                owner->fPositionY = trigger.fTeleportY;
                const bool bingo = owner->strArchetypeId == "BOSS_KOUKUSAYDON_BINGO_SAYDON";
                const float dx = owner->fSpawnPositionX - owner->fPositionX,
                            dz = owner->fSpawnPositionZ - owner->fPositionZ;
                if (dx * dx + dz * dz > .000001f)
                {
                    owner->fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES -
                                         (bingo || owner->strArchetypeId == "BOSS_KOUKUSAYDON_G1_SAYDON" ||
                                                  owner->strArchetypeId == "BOSS_KOUKUSAYDON_G3_SAYDON" ||
                                                  owner->strArchetypeId == "BOSS_KOUKUSAYDON_G2_BIG_SAYDON"
                                              ? 90.f
                                              : 0.f);
                }
                owner->fPatternStageOriginYawDegrees = owner->fYawDegrees;
            }

            if (!detached)
            {
                m_ServerCollisionSystem.Update_BlockingBody(owner->iNetEntityId,
                                                            owner->fPositionX,
                                                            owner->fPositionY + owner->fCollisionRadius,
                                                            owner->fPositionZ);
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            const auto* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
            std::string status;
            const auto* parent =
                catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
            std::size_t selected = 0u;
            std::array<std::uint32_t, 4u> cloneDurations{};
            if (!parent || !baseCatalog || owner->KoukuDirectionPlayback ||
                owner->iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID || !m_ServerNavigation.Is_Loaded() ||
                !CKoukuSaydonBrain::Select_CrossDirection(*owner,
                                                          *parent,
                                                          trigger,
                                                          *catalog,
                                                          selected,
                                                          cloneDurations,
                                                          status))
            {
                m_strStatus = "Cross direction preserved all actors: " + status;
                continue;
            }
            auto real = std::make_shared<SERVER_WORLD_ENTITY>(*owner);
            m_KoukuSaydonBrain.Abort_Pattern(*real, serverTick);
            const auto* realPattern =
                CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.DirectionPatternIds[selected], status);
            if (!realPattern || !m_KoukuSaydonBrain.Begin_Pattern(*real,
                                                                  *realPattern,
                                                                  owner->PinnedDefinitionRevision,
                                                                  serverTick,
                                                                  status))
            {
                m_strStatus = "Cross direction preserved all actors: " + status;
                continue;
            }
            std::vector<SERVER_WORLD_ENTITY> clones;
            NET_ENTITY_ID nextId = m_iNextNetEntityId;
            for (std::size_t index = 0u; index < 4u; ++index)
            {
                if (index == selected)
                {
                    continue;
                }
                const auto* child =
                    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.DirectionPatternIds[index], status);
                WORLD_BOOTSTRAP_PLACEMENT placement{};
                placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
                placement.strPlacementId = owner->strPlacementId + ".cross." + std::to_string(owner->iPatternSequence) +
                                           "." + std::to_string(index);
                placement.strArchetypeId = owner->strArchetypeId;
                placement.strEncounterId = owner->strEncounterId;
                placement.fPositionX = owner->fPositionX;
                placement.fPositionY = owner->fPositionY;
                placement.fPositionZ = owner->fPositionZ;
                placement.fYawDegrees = owner->fYawDegrees;
                SERVER_WORLD_ENTITY clone{};
                if (!child || nextId == INVALID_NET_ENTITY_ID ||
                    !Build_WorldEntity(placement,
                                       nextId,
                                       clone,
                                       baseCatalog,
                                       owner->iNetEntityId,
                                       owner->iPatternSequence))
                {
                    status = "a cross direction clone failed dependent-owner admission";
                    break;
                }
                clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
                clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
                clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
                clone.fYawDegrees = placement.fYawDegrees;
                clone.bKoukuGazeClone = clone.bKoukuSummonClone = true;
                clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
                clone.iKoukuCloneEndTick =
                    CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
                                                        CKoukuSaydonLogicRuntime::Ticks_FromMs(cloneDurations[index]));
                if (!m_KoukuSaydonBrain
                         .Begin_Pattern(clone, *child, owner->PinnedDefinitionRevision, serverTick, status))
                {
                    break;
                }
                std::vector<std::uint8_t> payload;
                if (!Build_WorldEntitySpawnedPayload(clone, payload))
                {
                    status = "a cross direction clone failed spawn serialization";
                    break;
                }
                clones.push_back(std::move(clone));
                ++nextId;
            }
            if (clones.size() != 3u)
            {
                m_strStatus = "Cross direction preserved all actors: " + status;
                continue;
            }
            const auto ownerIndex = static_cast<std::size_t>(std::distance(m_WorldEntities.begin(), liveOwner));
            m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
            auto& committedOwner = detached ? *owner : m_WorldEntities[ownerIndex];
            committedOwner.KoukuDirectionPlayback = std::move(real);
            committedOwner.bKoukuDirectionPlaybackComplete = false;
            committedOwner.iKoukuDirectionEndTick = CKoukuSaydonLogicRuntime::Add_Ticks(
                committedOwner.iPatternStartTick,
                CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
            for (auto& clone : clones)
            {
                m_WorldEntities.push_back(std::move(clone));
                Broadcast_WorldEntitySpawned(m_WorldEntities.back());
            }
            m_iNextNetEntityId = nextId;
            m_strStatus = "Cross direction selected real Pattern " + trigger.DirectionPatternIds[selected] +
                          "; three clones end at " + trigger.strCloneEndStageId;
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_RAIN_SOLDIERS)
        {
            if (!detached)
            {
                (void)Spawn_KoukuCardRainSoldiers(owner->iNetEntityId, serverTick, &trigger);
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            const auto* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
            std::string status;
            const auto* ownerPattern =
                catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
            if (!ownerPattern || !baseCatalog || owner->iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID ||
                !m_ServerNavigation.Is_Loaded() || trigger.PatternSpawns.empty() ||
                trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
            {
                m_strStatus =
                    "Summon Patterns preserved all actors: owner, catalog or spawn list is unavailable: " + status;
                continue;
            }
            const auto authoredTrigger =
                std::find_if(ownerPattern->MechanicTriggers.begin(),
                             ownerPattern->MechanicTriggers.end(),
                             [&](const auto& row)
                             {
                                 return row.strTriggerId == trigger.strTriggerId &&
                                        row.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS;
                             });
            if (authoredTrigger == ownerPattern->MechanicTriggers.end())
            {
                m_strStatus = "Summon Patterns preserved all actors: the owning occurrence is unavailable";
                continue;
            }
            const float yaw = owner->fYawDegrees * 0.01745329251994329577f;
            std::vector<SERVER_WORLD_ENTITY> clones;
            NET_ENTITY_ID nextId = m_iNextNetEntityId;
            bool admitted = true;
            for (const auto& spawn : trigger.PatternSpawns)
            {
                const auto* childPattern =
                    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, spawn.strPatternId, status);
                if (!childPattern ||
                    !CKoukuSaydonBrain::Validate_SummonedPattern(*ownerPattern, *childPattern, status, true))
                {
                    admitted = false;
                    break;
                }
                std::uint64_t childDurationMs = 0u;
                for (const auto& stage : childPattern->Stages)
                {
                    childDurationMs += stage.iDurationMs;
                }
                // A fixed Parent clock shortens pending duration to its absolute deadline.
                // Admission compares against the original authored window, before tick rounding.
                if (childDurationMs > authoredTrigger->iDurationMs)
                {
                    status = "a summoned Pattern exceeds its occurrence lifetime";
                    admitted = false;
                    break;
                }
                WORLD_BOOTSTRAP_PLACEMENT placement{};
                placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
                placement.strPlacementId = owner->strPlacementId + ".summon." +
                                           std::to_string(owner->iPatternSequence) + "." + trigger.strTriggerId + "." +
                                           spawn.strSpawnId;
                placement.strArchetypeId = owner->strArchetypeId;
                placement.strEncounterId = owner->strEncounterId;
                const bool mapAnchor = spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP;
                placement.fPositionX = mapAnchor ? spawn.PositionOffset[0]
                                                 : owner->fPositionX + spawn.PositionOffset[0] * std::cos(yaw) +
                                                       spawn.PositionOffset[2] * std::sin(yaw);
                placement.fPositionY =
                    mapAnchor ? spawn.PositionOffset[1] : owner->fPositionY + spawn.PositionOffset[1];
                placement.fPositionZ = mapAnchor ? spawn.PositionOffset[2]
                                                 : owner->fPositionZ - spawn.PositionOffset[0] * std::sin(yaw) +
                                                       spawn.PositionOffset[2] * std::cos(yaw);
                placement.fYawDegrees =
                    std::remainder((mapAnchor ? 0.f : owner->fYawDegrees) + spawn.fYawOffsetDegrees, 360.f);
                SERVER_NAV_POINT mapGround{};
                if (mapAnchor &&
                    (!m_ServerNavigation.Sample_Position(placement.fPositionX, placement.fPositionZ, mapGround) ||
                     !std::isfinite(mapGround.y) || std::abs(mapGround.y - placement.fPositionY) > 1.f))
                {
                    status = "a MAP summon position does not match its navigation floor";
                    admitted = false;
                    break;
                }
                SERVER_WORLD_ENTITY clone{};
                if (nextId == INVALID_NET_ENTITY_ID || !std::isfinite(placement.fPositionX) ||
                    !std::isfinite(placement.fPositionY) || !std::isfinite(placement.fPositionZ) ||
                    !std::isfinite(placement.fYawDegrees) ||
                    !m_ServerNavigation.Is_PointWalkableExact(placement.fPositionX, placement.fPositionZ) ||
                    !Build_WorldEntity(placement,
                                       nextId,
                                       clone,
                                       baseCatalog,
                                       owner->iNetEntityId,
                                       owner->iPatternSequence))
                {
                    status = "a summoned actor failed navigation or dependent-owner admission";
                    admitted = false;
                    break;
                }
                clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
                clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
                clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
                clone.fYawDegrees = placement.fYawDegrees;
                // Reuse the existing dependent clone lifetime and primary-loop exclusion.
                clone.bKoukuGazeClone = true;
                clone.bKoukuSummonClone = true;
                clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
                clone.iKoukuCloneEndTick =
                    CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
                                                        CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
                if (!m_KoukuSaydonBrain
                         .Begin_Pattern(clone, *childPattern, owner->PinnedDefinitionRevision, serverTick, status) ||
                    !CKoukuSaydonBrain::Apply_StageRootMotion(clone,
                                                              *childPattern,
                                                              serverTick,
                                                              m_ServerNavigation,
                                                              m_ServerCollisionSystem,
                                                              status) ||
                    !Update_KoukuSummonTriggers(clone, *childPattern, serverTick))
                {
                    if (status.empty())
                    {
                        status = m_strStatus;
                    }
                    admitted = false;
                    break;
                }
                std::vector<std::uint8_t> payload;
                if (!Build_WorldEntitySpawnedPayload(clone, payload))
                {
                    status = "a summoned actor failed spawn wire validation";
                    admitted = false;
                    break;
                }
                clones.push_back(std::move(clone));
                ++nextId;
            }
            if (!admitted || clones.size() != trigger.PatternSpawns.size())
            {
                m_strStatus = "Summon Patterns preserved all actors: " + status;
                continue;
            }
            m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
            for (auto& clone : clones)
            {
                m_WorldEntities.push_back(std::move(clone));
                Broadcast_WorldEntitySpawned(m_WorldEntities.back());
            }
            m_iNextNetEntityId = nextId;
            m_strStatus = "Summon Patterns started " + std::to_string(trigger.PatternSpawns.size()) +
                          " independently animated clones";
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE)
        {
            const CGameplayCatalog* catalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
            if (!catalog || !m_ServerNavigation.Is_Loaded() || trigger.iCountPerPlayer < 1u ||
                trigger.iCountPerPlayer > 8u || !std::isfinite(trigger.fPlayerEffectRadiusM) ||
                trigger.fPlayerEffectRadiusM < 0.f || trigger.fPlayerEffectRadiusM > 20.f ||
                ((trigger.iCountPerPlayer == 1u) != (trigger.fPlayerEffectRadiusM == 0.f)) ||
                (trigger.bRandomPlayerOnly && trigger.iCountPerPlayer != 1u) || trigger.iArenaRandomCount > 32u ||
                !std::isfinite(trigger.fArenaRandomRadiusM) || !std::isfinite(trigger.fArenaHeightToleranceM) ||
                !std::isfinite(trigger.fArenaMinimumSpacingM) ||
                (trigger.iArenaRandomCount == 0u
                     ? (trigger.fArenaRandomRadiusM != 0.f || trigger.fArenaHeightToleranceM != 0.f ||
                        trigger.fArenaMinimumSpacingM != 0.f)
                     : (trigger.fArenaRandomRadiusM <= 0.f || trigger.fArenaRandomRadiusM > 100.f ||
                        trigger.fArenaHeightToleranceM <= 0.f || trigger.fArenaHeightToleranceM > 10.f ||
                        trigger.fArenaMinimumSpacingM <= 0.f || trigger.fArenaMinimumSpacingM > 20.f)) ||
                trigger.iEffectLifetimeMs < 1u || trigger.iEffectLifetimeMs > 600000u)
            {
                m_strStatus = "Albion volley preserved existing objects: invalid catalog, navigation or layout";
                continue;
            }
            BOSS_COMBAT_OBJECT_DEFINITION definition{};
            definition.strEncounterId = owner->strEncounterId;
            definition.strOwnerPatternId = owner->strPatternId;
            definition.strOwnerStageActionId = trigger.strTriggerId;
            definition.strCombatObjectArchetypeId = "combatobject.kouku.albion.bluecircle";
            definition.strClientVisualId = "combatvisual.kouku.albion.bluecircle";
            definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
            definition.iLifeMs = trigger.iEffectLifetimeMs;
            // Hit templates share each authoritative circle instance and its warning clock.
            definition.AttackTemplates = trigger.FixedHits;
            definition.PresentationPulses.push_back({"combatpresentation.kouku.albion.started", 0u});
            BOSS_COMBAT_OBJECT_VOLLEY volley{};
            volley.ePolicy = BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
            volley.iCountPerResolvedTarget = trigger.iCountPerPlayer;
            volley.iMaximumTotalObjects = 64u;
            volley.fRadiusM = trigger.fPlayerEffectRadiusM;
            volley.iArenaRandomCount = trigger.iArenaRandomCount;
            volley.fArenaRandomRadiusM = trigger.fArenaRandomRadiusM;
            volley.fArenaHeightToleranceM = trigger.fArenaHeightToleranceM;
            if (trigger.iArenaRandomCount != 0u)
            {
                volley.eArenaAnchorPolicy = BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
            }
            if (trigger.iCountPerPlayer > 1u)
            {
                volley.eLayout = BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
                volley.fAngleStepDegrees = 360.f / static_cast<float>(trigger.iCountPerPlayer);
            }
            const auto eligiblePlayer = [](const SERVER_PLAYER& player)
            {
                return player.Is_Human() && player.isCombatReady && player.iCurrentHp != 0u &&
                       player.iMarioStage == 0u && player.eAction != PLAYER_ACTION_STATE::DEAD &&
                       player.eAction != PLAYER_ACTION_STATE::FALLING;
            };
            PLAYER_ID selectedPlayerId = INVALID_PLAYER_ID;
            if (trigger.bRandomPlayerOnly)
            {
                const std::uint64_t seed = Mix_DeterministicRandom(
                    Hash_StableId(trigger.strTriggerId) ^ (static_cast<std::uint64_t>(owner->iNetEntityId) << 32u) ^
                    owner->iPatternSequence);
                std::uint64_t selectedRank = (std::numeric_limits<std::uint64_t>::max)();
                for (const auto& [playerId, player] : m_Players)
                {
                    if (!eligiblePlayer(player))
                    {
                        continue;
                    }
                    const std::uint64_t rank = Mix_DeterministicRandom(seed ^ static_cast<std::uint64_t>(playerId));
                    if (selectedPlayerId == INVALID_PLAYER_ID || rank < selectedRank)
                    {
                        selectedPlayerId = playerId;
                        selectedRank = rank;
                    }
                }
            }
            BOSS_PATTERN_STAGE_ACTION arenaAction{};
            arenaAction.strTargetId = trigger.strTriggerId;
            arenaAction.Volley = volley;
            std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> arenaOrigins;
            if (!Resolve_ArenaRandomVolleyOrigins(*owner,
                                                  arenaAction,
                                                  definition,
                                                  0u,
                                                  arenaOrigins,
                                                  trigger.fArenaMinimumSpacingM))
            {
                m_strStatus = "Albion volley preserved existing objects: " + m_strStatus;
                continue;
            }
            auto transaction = m_CombatObjectRuntime.Begin_Transaction();
            std::string failure;
            bool admitted = true;
            for (const auto& [playerId, player] : m_Players)
            {
                if (!eligiblePlayer(player) || (trigger.bRandomPlayerOnly && playerId != selectedPlayerId))
                {
                    continue;
                }
                SERVER_COMBAT_OBJECT_LOCKED_TARGET target{};
                target.iNetEntityId = player.iNetEntityId;
                target.fPositionX = player.fPositionX;
                target.fPositionY = player.fPositionY;
                target.fPositionZ = player.fPositionZ;
                target.bTrackUntilFirstPulse = false;
                if (transaction.Objects.size() + trigger.iCountPerPlayer + arenaOrigins.size() >
                    volley.iMaximumTotalObjects)
                {
                    failure = "the resolved volley exceeds the object limit";
                    admitted = false;
                    break;
                }
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  *owner,
                                                                  &target,
                                                                  definition,
                                                                  &volley,
                                                                  *catalog,
                                                                  trigger.iCountPerPlayer,
                                                                  serverTick,
                                                                  failure))
                {
                    admitted = false;
                    break;
                }
            }
            if (admitted && transaction.Objects.empty())
            {
                // Preserve the authored player volley at the boss ground when the
                // arena is empty; the resulting objects never follow later movement.
                SERVER_COMBAT_OBJECT_LOCKED_TARGET fallback{};
                fallback.fPositionX = owner->fPositionX;
                fallback.fPositionY = owner->fPositionY;
                fallback.fPositionZ = owner->fPositionZ;
                admitted = m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                        *owner,
                                                                        &fallback,
                                                                        definition,
                                                                        &volley,
                                                                        *catalog,
                                                                        trigger.iCountPerPlayer,
                                                                        serverTick,
                                                                        failure);
            }
            for (const auto& origin : arenaOrigins)
            {
                if (!admitted)
                {
                    break;
                }
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  *owner,
                                                                  &origin,
                                                                  definition,
                                                                  nullptr,
                                                                  *catalog,
                                                                  1u,
                                                                  serverTick,
                                                                  failure))
                {
                    admitted = false;
                    break;
                }
            }
            // Validate the actual staged radial poses, then keep spawn/reconnect snapshots on the same floor.
            for (std::size_t index = 0u; admitted && index < transaction.Objects.size(); ++index)
            {
                auto& object = transaction.Objects[index];
                auto& pose = object.LiveState.CurrentPose;
                SERVER_NAV_POINT ground{};
                if (!m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) ||
                    !m_ServerNavigation.Sample_Position(pose.fPositionX, pose.fPositionZ, ground) ||
                    !std::isfinite(ground.y))
                {
                    failure = "a staged point is outside navigable ground";
                    admitted = false;
                    break;
                }
                pose.fPositionY = ground.y;
                object.LiveState.PreviousPose = pose;
                transaction.Spawned[index].fPositionY = ground.y;
            }
            if (!admitted)
            {
                m_strStatus = "Albion volley preserved existing objects: " + failure;
                continue;
            }
            if (!transaction.Objects.empty() && !m_CombatObjectRuntime.Commit(std::move(transaction)))
            {
                m_strStatus = "Albion volley preserved existing objects: transaction revision changed";
            }
            continue;
        }
        const CGameplayCatalog* catalog = Resolve_KoukuProductCatalog();
        const CGameplayCatalog* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
        std::string status;
        const BOSS_PATTERN_DEFINITION* clonePattern =
            nullptr == catalog
                ? nullptr
                : CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.strClonePatternId, status);
        if (nullptr == clonePattern || nullptr == baseCatalog || trigger.ClockHours.size() != 3u ||
            std::find(clonePattern->AuditionBossArchetypeIds.begin(),
                      clonePattern->AuditionBossArchetypeIds.end(),
                      owner->strArchetypeId) == clonePattern->AuditionBossArchetypeIds.end())
        {
            m_strStatus = "KoukuSaydon teleport preserved the boss: clone pattern/body is unavailable: " + status;
            continue;
        }
        // 저작 1시 좌표와 spawn 중심으로 네 자리를 준비한다. 순번은 이번 계산용이며 저장 ID가 아니다.
        const float dx = trigger.fTeleportX - owner->fSpawnPositionX;
        const float dz = trigger.fTeleportZ - owner->fSpawnPositionZ;
        const float radius = std::sqrt(dx * dx + dz * dz);
        if (!std::isfinite(radius) || radius <= 0.01f)
        {
            m_strStatus = "KoukuSaydon teleport preserved the boss: radius is invalid";
            continue;
        }
        const float anchor = std::atan2(dx, dz);
        const std::array<std::uint32_t, 4u> clockHours{
            1u, trigger.ClockHours[0u], trigger.ClockHours[1u], trigger.ClockHours[2u]};
        const auto positionAtHour = [&](const std::uint32_t hour)
        {
            // 원래 1시는 삼각함수로 재계산하지 않아 기존 저작 좌표의 float 값을 보존한다.
            if (hour == 1u)
            {
                return SERVER_NAV_POINT{trigger.fTeleportX, trigger.fTeleportY, trigger.fTeleportZ};
            }
            const float angle = anchor + static_cast<float>(hour - 1u) * 0.52359877559829887f;
            return SERVER_NAV_POINT{owner->fSpawnPositionX + std::sin(angle) * radius,
                                    trigger.fTeleportY,
                                    owner->fSpawnPositionZ + std::cos(angle) * radius};
        };
        std::array<SERVER_NAV_POINT, 4u> positions{};
        bool positionsValid = std::isfinite(trigger.fFaceCenterYawOffsetDegrees);
        for (std::size_t slot = 0u; slot < clockHours.size(); ++slot)
        {
            const auto hour = clockHours[slot];
            if (hour < 1u || hour > 12u ||
                std::find(clockHours.begin(), clockHours.begin() + slot, hour) != clockHours.begin() + slot)
            {
                positionsValid = false;
                break;
            }
            positions[slot] = positionAtHour(hour);
            const auto& position = positions[slot];
            if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
                (m_ServerNavigation.Is_Loaded() &&
                 !m_ServerNavigation.Is_PointWalkableExact(position.x, position.z)))
            {
                positionsValid = false;
                break;
            }
        }
        if (!positionsValid)
        {
            m_strStatus = "KoukuSaydon teleport preserved all actors: a candidate position is invalid";
            continue;
        }
        // 처리 지연 tick 대신 패턴 시작과 저작 trigger 시각을 사용한다. 같은 실행을 재계산해도 선택은 같다.
        const std::uint32_t selectionTick = CKoukuSaydonLogicRuntime::Add_Ticks(
            owner->iPatternStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iStartMs));
        const std::uint64_t seed = Mix_DeterministicRandom(
            Hash_StableId(trigger.strTriggerId) ^ (static_cast<std::uint64_t>(owner->iNetEntityId) << 32u) ^
            owner->iPatternSequence ^ selectionTick);
        const std::size_t realSlot = static_cast<std::size_t>(seed % positions.size());
        const auto& realPosition = positions[realSlot];
        std::vector<SERVER_WORLD_ENTITY> clones;
        NET_ENTITY_ID nextId = m_iNextNetEntityId;
        bool admitted = true;
        for (std::size_t slot = 0u; slot < clockHours.size(); ++slot)
        {
            if (slot == realSlot)
            {
                continue;
            }
            const std::uint32_t hour = clockHours[slot];
            const auto& position = positions[slot];
            WORLD_BOOTSTRAP_PLACEMENT placement{};
            placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
            placement.strPlacementId =
                owner->strPlacementId + ".gaze." + std::to_string(owner->iPatternSequence) + "." + std::to_string(hour);
            placement.strArchetypeId = owner->strArchetypeId;
            placement.strEncounterId = owner->strEncounterId;
            placement.fPositionX = position.x;
            placement.fPositionY = position.y;
            placement.fPositionZ = position.z;
            placement.fYawDegrees = std::atan2(owner->fSpawnPositionX - placement.fPositionX,
                                               owner->fSpawnPositionZ - placement.fPositionZ) *
                                        57.29577951308232f +
                                    trigger.fFaceCenterYawOffsetDegrees;
            SERVER_WORLD_ENTITY clone{};
            if (INVALID_NET_ENTITY_ID == nextId ||
                (m_ServerNavigation.Is_Loaded() &&
                 !m_ServerNavigation.Is_PointWalkableExact(placement.fPositionX, placement.fPositionZ)) ||
                !Build_WorldEntity(placement, nextId, clone, baseCatalog, owner->iNetEntityId, owner->iPatternSequence))
            {
                admitted = false;
                break;
            }
            // Static placement admission projects to cell centres; retain this admitted exact circle.
            clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
            clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
            clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
            clone.fYawDegrees = placement.fYawDegrees;
            clone.bKoukuGazeClone = true;
            clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
            clone.iKoukuCloneEndTick =
                CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
                                                    CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
            clone.PinnedDefinitionRevision = owner->PinnedDefinitionRevision;
            if (!m_KoukuSaydonBrain
                     .Begin_Pattern(clone, *clonePattern, owner->PinnedDefinitionRevision, serverTick, status))
            {
                admitted = false;
                break;
            }
            std::vector<std::uint8_t> payload;
            if (!Build_WorldEntitySpawnedPayload(clone, payload))
            {
                admitted = false;
                break;
            }
            clones.push_back(std::move(clone));
            ++nextId;
        }
        if (!admitted || clones.size() != 3u)
        {
            m_strStatus = "KoukuSaydon teleport preserved all actors: clone admission failed: " + status;
            continue;
        }
        const auto ownerIndex = static_cast<std::size_t>(std::distance(m_WorldEntities.begin(), liveOwner));
        m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
        auto& committedOwner = detached ? *owner : m_WorldEntities[ownerIndex];
        // 세 분신 준비가 모두 성공한 뒤 선택한 본체 위치를 적용한다. spawn 중심은 다음 실행에도 유지한다.
        committedOwner.fPositionX = realPosition.x;
        committedOwner.fPositionY = realPosition.y;
        committedOwner.fPositionZ = realPosition.z;
        committedOwner.fYawDegrees =
            std::atan2(committedOwner.fSpawnPositionX - realPosition.x,
                       committedOwner.fSpawnPositionZ - realPosition.z) * 57.29577951308232f +
            trigger.fFaceCenterYawOffsetDegrees;
        for (auto& clone : clones)
        {
            m_WorldEntities.push_back(std::move(clone));
            Broadcast_WorldEntitySpawned(m_WorldEntities.back());
        }
        m_iNextNetEntityId = nextId;
        m_strStatus = "KoukuSaydon real boss teleported; three inward-facing clones started";
    }
    m_PendingKoukuMechanicTriggers.clear();
    Commit_KoukuMarioEntries();
}

// 소환 생성·분신 갱신이 호출한다. 시작 시각에 도달한 미실행 trigger를 정렬하고 성공한 항목만 실행 목록에 기록한다.
// clone의 시작 목록이 중복 실행을 막으며, 실패는 false로 상위 소환/수명 처리에 전달한다.
bool LostArk::Server::CGameRoom::Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
                                                            const BOSS_PATTERN_DEFINITION& pattern,
                                                            const std::uint32_t serverTick)
{
    using Clock = CKoukuSaydonLogicRuntime;
    std::vector<std::size_t> due;
    for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
    {
        const auto& trigger = pattern.MechanicTriggers[index];
        // The actor ledger consumes this facing Trigger before its contact windows.
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET && trigger.iDurationMs > 0u &&
            trigger.iDurationMs <= 34u && trigger.fFollowSpeedScale == 0.f)
        {
            continue;
        }
        if (std::find(clone.KoukuSummonStartedTriggers.begin(), clone.KoukuSummonStartedTriggers.end(), index) ==
                clone.KoukuSummonStartedTriggers.end() &&
            Clock::Has_ReachedTick(serverTick,
                                   Clock::Add_Ticks(clone.iPatternStartTick, Clock::Ticks_FromMs(trigger.iStartMs))))
        {
            due.push_back(index);
        }
    }
    std::stable_sort(due.begin(),
                     due.end(),
                     [&](const auto a, const auto b)
                     {
                         const auto& left = pattern.MechanicTriggers[a];
                         const auto& right = pattern.MechanicTriggers[b];
                         if (left.iStartMs != right.iStartMs)
                         {
                             return left.iStartMs < right.iStartMs;
                         }
                         return left.eAirbornePhase == ALBION_AIRBORNE_PHASE::JUMP &&
                                right.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP;
                     });
    for (const auto index : due)
    {
        const auto& trigger = pattern.MechanicTriggers[index];
        if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
            (trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP &&
             trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::SLAM) ||
            !Commit_KoukuAlbionAirborne(clone, pattern, trigger, serverTick))
        {
            return false;
        }
        clone.KoukuSummonStartedTriggers.push_back(index);
    }
    return true;
}

// 소환 배우와 교차 방향 배우 갱신이 호출한다. 배우가 소유한 접촉 ledger를 준비해 실제 actor 위치로 Logic을 평가한다.
// 정의는 pinned catalog에서 차용하고 ledger는 actor에 남긴다. 일반 본체의 audition ledger와 합치지 않는다.
void LostArk::Server::CGameRoom::Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor,
                                                           const BOSS_PATTERN_DEFINITION& pattern,
                                                           const CGameplayCatalog& product,
                                                           const std::uint32_t serverTick)
{
    if (pattern.LogicWindows.empty() && pattern.MechanicTriggers.empty())
    {
        return;
    }
    const auto* base = m_GameplayCatalog.Resolve(actor.PinnedDefinitionRevision);
    if (!base)
    {
        return;
    }
    if (!actor.KoukuContactLedger)
    {
        actor.KoukuContactLedger = std::make_shared<KOUKUSAYDON_LOGIC_LEDGER>();
        CKoukuSaydonLogicRuntime::Build(pattern, actor, actor.iPatternStartTick, *actor.KoukuContactLedger);
    }
    // Root motion has already placed this particular body. Face its own target
    // before evaluating BOSS-anchored contacts; siblings keep their original yaw.
    Update_KoukuPlayerTargets(actor, pattern, *actor.KoukuContactLedger, *base, serverTick);
    KOUKUSAYDON_LOGIC_OUTPUT output;
    // Summoned children admit actor-local facing and contact damage; they cannot submit
    // room transitions, follow-up patterns, WORLD changes or player ownership.
    CKoukuSaydonLogicRuntime::Update(actor,
                                     pattern,
                                     *actor.KoukuContactLedger,
                                     m_Players,
                                     *base,
                                     product.Find_KoukuMadnessPolicy(actor.strEncounterId),
                                     serverTick,
                                     m_TickDamageEvents,
                                     output,
                                     &m_ServerNavigation,
                                     &m_ServerCollisionSystem);
}

// 월드 본체 순회 전에 호출한다. owner 실행과 수명이 유효한 분신의 애니메이션·root motion·접촉을 진행한다.
// 유효하지 않거나 재생이 실패한 분신은 기존 despawn 알림 뒤 현재 iterator로 제거해 다음 actor를 안전하게 계속한다.
void LostArk::Server::CGameRoom::Update_KoukuGazeClones(const std::uint32_t serverTick)
{
    for (auto clone = m_WorldEntities.begin(); clone != m_WorldEntities.end();)
    {
        if (!clone->bKoukuGazeClone)
        {
            ++clone;
            continue;
        }
        const auto* owner = Find_KoukuOccurrenceOwner(clone->iOwnerBossNetEntityId, clone->iKoukuCloneOwnerSequence);
        bool live = owner != nullptr && owner->iCurrentHp > 0u &&
                    !CKoukuSaydonLogicRuntime::Has_ReachedTick(serverTick, clone->iKoukuCloneEndTick);
        const auto* catalog = Resolve_KoukuProductCatalog();
        if (live && nullptr != catalog && !clone->strPatternId.empty())
        {
            std::string status;
            if (clone->bKoukuSummonClone)
            {
                const auto* pattern =
                    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, clone->strPatternId, status);
                live = pattern && CKoukuSaydonBrain::Apply_StageRootMotion(*clone,
                                                                           *pattern,
                                                                           serverTick,
                                                                           m_ServerNavigation,
                                                                           m_ServerCollisionSystem,
                                                                           status);
                if (live)
                {
                    Update_KoukuActorContacts(*clone, *pattern, *catalog, serverTick);
                }
            }
            if (live)
            {
                const auto result = m_KoukuSaydonBrain.Update(*clone, *catalog, serverTick, status);
                live = KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION != result &&
                       KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD != result;
                if (live && clone->bKoukuSummonClone && !clone->strPatternId.empty())
                {
                    const auto* pattern =
                        CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, clone->strPatternId, status);
                    live = pattern && Update_KoukuSummonTriggers(*clone, *pattern, serverTick);
                    if (!live && status.empty())
                    {
                        status = m_strStatus;
                    }
                }
            }
            if (!live)
            {
                m_strStatus = "KoukuSaydon clone playback failed: " + status;
            }
        }
        if (live)
        {
            ++clone;
            continue;
        }
        Broadcast_WorldEntityDespawned(clone->iNetEntityId);
        clone = m_WorldEntities.erase(clone);
    }
}
```


### Server/Public/KoukuSaydonLogicRuntime.h

실제 수정 대상: [KoukuSaydonLogicRuntime.h](C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h)


#### G2-ledger-state

현재 기준 블록:

```cpp
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;
```

교체 블록:

```cpp
        // Build가 이번 실행의 조커 카드 위치 차이(월드 m)를 한 번 결정한다. ID·회전·scale은 저작 정의를 유지한다.
        // 최초 World PLAY와 OBJECT_CONTACT가 같이 읽고 ledger 종료 시 함께 폐기한다. Room tick 스레드만 접근한다.
        std::map<std::string, std::array<float, 3u>> WorldPositionOffsets;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;
```


#### KoukuSaydonLogicRuntime.h 전체 코드

```cpp
#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "ServerNavigation.h"

#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <string>
#include <vector>

namespace LostArk::Server
{
	class CServerCollisionSystem;
	/* 판정 window의 플레이어별 응답이다. NONE은 연속 window에서는 미응답, 종료 tick 판정에서는 아직 판정 전이다. */
	enum class KOUKUSAYDON_LOGIC_ANSWER : std::uint8_t
	{
		NONE,
		SUCCESS,
		FAIL,
		TIMEOUT
	};

	/* Build가 패턴 시작 tick을 기준으로 만든 window 진행 상태다. Update가 저작 pattern과 이 상태를 함께 소비한다. */
	struct KOUKUSAYDON_LOGIC_WINDOW_STATE final
	{
		std::uint32_t iWindowIndex = 0u;
		std::uint32_t iStartTick = 0u;
		std::uint32_t iEndTick = 0u;
        std::uint32_t iHoldStartTick = 0u, iHoldEndTick = 0u;
		bool bOpened = false;
		bool bClosed = false;
		bool bChargeCaptured = false;
		bool bChargeStopped = false;
		std::optional<BOSS_PATTERN_BOSS_MOTION> ChargeMotion;
		std::set<std::pair<std::string, std::string>> AppliedWorldMotions;
		std::set<std::string> ContactedWorldOccurrences;
		// 주기적으로 접촉하는 공·인형의 WORLD 소유자를 구분한다. 나머지 피해는 HP 기반 광기 증가를 따른다.
		std::uint8_t iMadnessSource = 0u;
		std::uint32_t iMadnessContactIntervalMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER> Answers;
		std::set<LostArk::Shared::PLAYER_ID> InsidePlayers;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> NextContactHitTicks;
		std::map<std::string, BOSS_LOGIC_REGION> FrozenRegions;
		LostArk::Shared::NET_ENTITY_ID iFreePlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Shared::NET_ENTITY_ID> BoundPlayers;
	};

	/* 예정된 Server tick을 기다리는 저작 world sequence·scene profile cue다. */
	struct KOUKUSAYDON_LOGIC_CUE_STATE final
	{
		std::uint32_t iIndex = 0u;
		std::uint32_t iStartTick = 0u;
		bool bStarted = false;
	};

	struct KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE final
	{
		std::uint32_t iTriggerIndex = 0u;
		std::uint32_t iStartTick = 0u, iEndTick = 0u, iNextFixedTick = 0u;
		std::uint32_t iLastUpdateTick = 0u;
		std::uint32_t iNextRandomTick = 0u, iRandomWaveOrdinal = 0u;
		std::uint32_t iBombStartTick = 0u;
		LostArk::Shared::COMBAT_OBJECT_ID iBombBodyObjectId = 0u;
		std::optional<bool> BombInsideAtEnd;
		LostArk::Shared::NET_ENTITY_ID iSelectedTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::optional<std::array<float, 3u>> LastSelectedTargetPosition;
		bool bClosed = false;
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Shared::COMBAT_OBJECT_ID> TrackingObjects;
	};

	/* 룸이 패턴 발생별로 소유하는 Logic 진행 기록이다. 패턴 시작 때 만들고 종료 때 버린다. */
	struct KOUKUSAYDON_LOGIC_LEDGER final
	{
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iPatternStartTick = 0u;
		bool bDanceActive = false;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> ScheduledDanceIntervals;
		LostArk::Shared::KOUKU_HUD_MODE eHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		std::vector<KOUKUSAYDON_LOGIC_WINDOW_STATE> Windows;
		std::set<std::string> RetiredWorldOccurrences;
		std::uint32_t iBingoLineJudgementTick = 0u, iBingoCompletedLines = 0u;
		bool bBingoLineRewardApplied = false;
		std::vector<std::uint32_t> ContactWindowOrder;
		std::set<std::tuple<std::string, std::uint32_t, std::string>> ConsumedContactGroups;
		// 이번 패턴에서 먼저 적용된 높은 우선순위의 카드 상태를 낮은 우선순위 반응이 덮지 못하게 한다.
		std::map<std::pair<std::string, std::string>, std::uint32_t> AppliedContactMotionPriorities;
        // Build가 이번 실행의 조커 카드 위치 차이(월드 m)를 한 번 결정한다. ID·회전·scale은 저작 정의를 유지한다.
        // 최초 World PLAY와 OBJECT_CONTACT가 같이 읽고 ledger 종료 시 함께 폐기한다. Room tick 스레드만 접근한다.
        std::map<std::string, std::array<float, 3u>> WorldPositionOffsets;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> MechanicTriggers;
		std::vector<KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE> PlayerTargetWindows;
		bool bTrackingTargetReached = false;
		// 입장 시 roster를 world X 내림차순, 동률이면 PlayerId 순으로 한 번 캡처한다.
		bool bCardMazeEntryRosterCaptured = false;
		std::vector<LostArk::Shared::PLAYER_ID> CardMazeEntryPlayers;
		std::size_t iCardMazeNextHiddenPlayer = 0u;

		[[nodiscard]] bool Is_Active() const noexcept { return !strPatternId.empty(); }
	};

	struct KOUKUSAYDON_LOGIC_WORLD_PLAY final
	{
		std::string strInstanceId;
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		std::uint32_t iDurationMs = 0u;
		std::string strTargetSequenceInstanceId;
		std::string strOccurrenceId;
		std::uint32_t iStartTick = 0u;
		std::string strTargetWorldOccurrenceId;
		bool bAuthoredMadness = false;
		std::optional<BOSS_PATTERN_WORLD_COMBAT_BODY> CombatBody;
		std::optional<BOSS_PATTERN_WORLD_PLACEMENT> Placement;
	};

    struct KOUKUSAYDON_LOGIC_CAPTURE_REQUEST final
    {
        LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        std::uint32_t iHoldEndTick = 0u;
        std::uint32_t iWindowIndex = 0u;
    };

	/* Update가 룸에 요청하는 송신·후속 패턴·포획 결과다. 플레이어·보스 상태는 직접 갱신하지만 송신과 실행 목록 소유권은 룸에 둔다. */
	struct KOUKUSAYDON_LOGIC_OUTPUT final
	{
		std::vector<KOUKUSAYDON_LOGIC_WORLD_PLAY> WorldSequencePlays;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<std::string> FollowupPatternIds;
        std::vector<KOUKUSAYDON_LOGIC_CAPTURE_REQUEST> CaptureRequests;
		bool bEndPatternEarly = false;
		// 패턴을 끝내는 counter의 검증된 착지가 commit된 뒤에만 설정한다.
		bool bCounterSuccessLanded = false;
		bool bStaggerSuccess = false;
		std::optional<bool> BingoLineCompletion;
		std::string strStatus;
	};

	/* 고정 Server tick에서 저작 Logic의 RESULT를 판정·적용한다. */
	// 쿠크 패턴의 Logic window와 Server tick을 비교해 플레이어·보스 상태와 룸에 돌려줄 결과를 계산한다.
	// 발생별 진행 상태는 호출자가 넘기는 ledger에 보관하고, 송신·패턴 선택·stage 전환은 룸이 맡는다.
	class CKoukuSaydonLogicRuntime final
	{
	public:
        // Only physical contact hazards are offered to companion avoidance.
        [[nodiscard]] static float Predict_ContactRisk(const SERVER_WORLD_ENTITY& boss,
            const BOSS_PATTERN_DEFINITION& pattern, const KOUKUSAYDON_LOGIC_LEDGER& ledger,
            const SERVER_PLAYER& probe, std::uint32_t serverTick, std::uint32_t horizonMs);

		static constexpr std::uint32_t DANCE_POSE_COUNT = 4u;

		static void Assign_EncounterCard(SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID encounterOwnerId, std::uint32_t serverTick,
			std::uint8_t unavailableSymbols = 0u);

		static bool Can_EnterMarioEntry(const SERVER_PLAYER& player) noexcept;
		// MARIO_ENTER만 성공 결과로 가진 영역 진입 window 또는 PATTERN_COMPLETION_COUNT chain root를 찾는다.
		static bool Has_MarioEntry(const BOSS_PATTERN_DEFINITION& pattern) noexcept;
		// 영역이 있는 첫 Mario 진입 결과를 반환한다. chain root만으로 진입하는 패턴이면 nullptr다.
		static const BOSS_PATTERN_LOGIC_RESULT* Find_MarioEntryResult(const BOSS_PATTERN_DEFINITION& pattern) noexcept;
		// 자식 패턴 실행 중에도 룸이 보관한 root 영역으로 검사한다. honorWindowEnd가 true일 때만 저작 종료 시각에 portal을 닫는다.
		static bool Is_InsideMarioEntry(const BOSS_PATTERN_DEFINITION& root,
			const SERVER_WORLD_ENTITY& anchor, const SERVER_PLAYER& player,
			std::uint32_t elapsedTicks, bool honorWindowEnd = false, std::uint8_t* matchedStage = nullptr);
		// 접촉 판정과 같은 저작 영역 변환과 타원 부채꼴 경계를 사용한다.
		[[nodiscard]] static bool Is_PointInsideRegion(const BOSS_LOGIC_REGION& region,
			const SERVER_WORLD_ENTITY& boss, float worldX, float worldZ) noexcept;
		// 현재 권위 mechanic 구간을 읽어 snapshot의 gauge로 투영한다.
		static void Project_MechanicGauge(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::uint32_t serverTick, LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot);
		static void Build(
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t startTick,
			KOUKUSAYDON_LOGIC_LEDGER& outLedger);
		/* 추가 판정 없이 ledger를 비우고 보스 방어·결과 및 이 발생이 소유한 플레이어 bind를 정리한다. */
		static void Discard(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			SERVER_WORLD_ENTITY* pBoss);
		static void Update(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
			const CServerNavigation* navigation = nullptr,
			const CServerCollisionSystem* collision = nullptr);
		/* 플레이어에게 복제한 slot 배치로 DANCE 입력을 해석한다. 열린 window의 첫 응답만 기록한다. */
		static bool Record_InteractionSlot(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_PLAYER& player,
			LostArk::Shared::INTERACTION_SLOT slot,
			std::string& outStatus);
		/* 플레이어 HUD·slot·광기 상한·광대 만료를 갱신한다. dance slot은 패턴 발생별 한 번 섞어 HUD와 판정이 같은 배치를 읽는다. */
		static void Update_PlayerModes(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const KOUKUSAYDON_LOGIC_LEDGER* pActiveLedger,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick);
		static bool Update_PlayerFear(SERVER_PLAYER& player, std::uint32_t serverTick);
		static void Apply_Result(
			SERVER_PLAYER& player,
			const BOSS_PATTERN_LOGIC_RESULT& result,
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const std::array<float, 2u>* pContactCenter = nullptr, bool encounterWipe = true);
		/* 살아 있는 일반 플레이어를 광대로 바꾼다. durationMs가 0이면 encounter 유지 시간, 없으면 기본 시간을 쓴다. */
		static void Transform_ToClown(
			SERVER_PLAYER& player,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::uint32_t durationMs);
		/* 공격 원점 방향이 활성 방패 각도 안이면 true다. 호출자는 원래 피해 대신 반사를 처리한다. */
		[[nodiscard]] static bool Is_ShieldReflected(
			const SERVER_WORLD_ENTITY& boss,
			float sourceX,
			float sourceZ) noexcept;
		[[nodiscard]] static bool Has_ReachedTick(
			std::uint32_t serverTick, std::uint32_t targetTick) noexcept;
		[[nodiscard]] static std::uint32_t Add_Ticks(
			std::uint32_t tick, std::uint32_t ticks) noexcept;
		[[nodiscard]] static std::uint32_t Ticks_FromMs(std::uint32_t ms) noexcept;

	private:
		static void Capture_CardMazeEntryRoster(KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Reveal_CardMazeEntryPlayers(const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static bool Stage_CardMazeEntryPlayers(const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus);
		static bool Enter_CardMaze(const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus);
		static void Capture_StartRegions(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window, KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			std::uint32_t patternElapsedTicks);
		static void Open_Window(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Close_Window(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Apply_Results(
			const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			SERVER_PLAYER* pPlayer,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
			const std::set<LostArk::Shared::PLAYER_ID>& invulnerablePlayers,
			const std::array<float, 2u>* pContactCenter = nullptr,
			bool spatialContact = false);
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Roulette(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Gaze(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static bool Is_Judgeable(const SERVER_PLAYER& player) noexcept;
		static bool Can_ReceiveSpatialContact(const SERVER_PLAYER& player) noexcept;
	};

	/* 룸이 소유하는 빙고 상태다. 폭탄·망치 단계와 보드 진행을 값으로 보관한다. */
	/* 25비트 white는 모든 해골 칸, red는 완성된 행·열의 영구 칸이다. 보상 소비 여부는 별도 Server line mask로 관리한다. */
	class CKoukuBingoRuntime final
	{
	public:
		void Reset() noexcept
		{
			m_iWhiteMask = 0u;
			m_iRedMask = 0u;
			m_iConsumedLineMask = 0u;
			m_Bombs = {};
			m_Hammer = {};
		}
		/* 보드 안의 지정 칸을 채우고 완성된 행·열을 승격한다. 보드 밖 비트는 무시한다. */
		void Fill(std::uint32_t cellMask) noexcept;
		/* red 칸은 유지하고 폭발 범위의 나머지 해골 칸을 반전한 뒤 행·열을 승격한다. white에는 red도 포함하며 대각선은 승격하지 않는다. */
		void Detonate(std::uint32_t cellMask) noexcept;
		[[nodiscard]] std::uint32_t Get_WhiteMask() const noexcept { return m_iWhiteMask; }
		[[nodiscard]] std::uint32_t Get_RedMask() const noexcept { return m_iRedMask; }
		[[nodiscard]] std::uint32_t Count_CompletedRowsAndColumns() const noexcept;
		[[nodiscard]] std::uint32_t Count_UnconsumedRowsAndColumns() const noexcept;
		/* 행·열의 고정 순서로 미소비 완성 줄을 count개 소비한다. count가 0이거나 줄이 부족하면 상태를 유지한다. */
		bool Consume_CompletedRowsAndColumns(std::uint32_t count) noexcept;
		/* 해당 위치의 보드 칸이 완성된 red 줄에 속하는지 검사한다. 전멸 공격의 안전 구역 판정에 사용한다. */
		[[nodiscard]] bool Is_Safe(float x, float z) const noexcept;

		/* 폭탄 slot은 운반자 ID와 예정 tick을 보관한다. 실제 플레이어 조회와 낙하 위치 결정은 룸이 담당한다. */
		struct BOMB final
		{
			LostArk::Shared::BINGO_BOMB_PHASE ePhase =
				LostArk::Shared::BINGO_BOMB_PHASE::NONE;
			LostArk::Shared::NET_ENTITY_ID iCarrierNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iDetonateTick = 0u;
			// 머리 표식이 사라진 뒤 지면 생성 전까지 slot을 예약하는 tick이다.
			std::uint32_t iPlantTick = 0u;
			std::uint32_t iMarkOrdinal = 0u;
			// 폭탄마다 시간을 캡처해 뒤의 보드 설정 변경이 대기 중인 낙하·폭발을 바꾸지 못하게 한다.
			std::uint32_t iDropDelayMs = LostArk::Shared::KOUKU_BINGO_BOMB_DROP_DELAY_MS;
			std::uint32_t iFuseMs = LostArk::Shared::KOUKU_BINGO_BOMB_FUSE_MS;
		};
		using BOMB_SLOTS = std::array<BOMB,
			static_cast<std::size_t>(LostArk::Shared::KOUKU_BINGO_MAX_BOMBS)>;

		/* 운반자 ID가 유효하고 빈 slot이 있으면 표식한다. 같은 운반자의 MARKED가 이미 있으면 거절해 표식 시간을 재시작하지 않는다. */
		bool Start_Bomb(LostArk::Shared::NET_ENTITY_ID carrier,
			std::uint32_t detonateTick, std::uint32_t markOrdinal = 0u,
			std::uint32_t dropDelayMs = LostArk::Shared::KOUKU_BINGO_BOMB_DROP_DELAY_MS,
			std::uint32_t fuseMs = LostArk::Shared::KOUKU_BINGO_BOMB_FUSE_MS) noexcept;
		/* MARKED 또는 낙하 예약 slot을 지정 위치에 설치하고 fuseTick을 폭발 tick으로 저장한다. */
		void Plant_Bomb(std::size_t slot, float x, float z,
			std::uint32_t fuseTick) noexcept;
		void Queue_BombPlant(std::size_t slot, float x, float z, std::uint32_t plantTick) noexcept;
		void Clear_Bomb(std::size_t slot) noexcept;
		[[nodiscard]] const BOMB_SLOTS& Get_Bombs() const noexcept { return m_Bombs; }

		/* 망치가 없을 때 유효한 anchor와 시간으로 시작한다. anchor 선택은 룸이 맡는다. */
		bool Start_Hammer(std::int32_t anchor, std::uint32_t startTick,
			std::uint32_t endTick) noexcept;
		/* 도달한 종료 tick에 망치를 한 단계 전진시키고 마지막 단계 뒤에는 비운다. 망치가 남아 있으면 true다. */
		bool Advance_Hammer(std::uint32_t tick) noexcept;
		[[nodiscard]] const LostArk::Shared::BINGO_HAMMER_SNAPSHOT&
			Get_Hammer() const noexcept { return m_Hammer; }

	private:
		/* Promotes every line that is now completely white. */
		void Promote_Lines() noexcept;

		std::uint32_t m_iWhiteMask = 0u;
		std::uint32_t m_iRedMask = 0u;
		std::uint32_t m_iConsumedLineMask = 0u;
		BOMB_SLOTS m_Bombs{};
		LostArk::Shared::BINGO_HAMMER_SNAPSHOT m_Hammer{};
	};

	// 미로의 역할·문양·처치 진행과 대기 참가자를 소유한다. entity 생성·피격·제거와 송신은 룸이 담당한다.
	class CKoukuCardMazeRuntime final
	{
	public:
		enum class PHASE : std::uint8_t
		{
			INACTIVE,
			HUNTING,
			COMPLETE
		};

		struct SPAWN_REQUEST final
		{
			LostArk::Shared::MECHANIC_CARD_SYMBOL eSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
		};

		/* What one landed hammer hit changed. */
		struct HIT_OUTCOME final
		{
			bool bStartMarch = false;
			bool bKillCounted = false;
			bool bHunterComplete = false;
			bool bAllComplete = false;
		};

		static constexpr std::uint8_t KILL_TARGET = 1u;
		static constexpr std::uint32_t TARGETS_PER_SUIT = 1u;
		/* 일반 망치의 피해 판정은 입력 후 30Hz 기준 30tick이다. Hammer_HitTickOffset은 interactionIndex 1에만 12tick을 적용한다. */
		static constexpr std::uint32_t HAMMER_HIT_TICK_OFFSET = 30u;
		static constexpr std::uint32_t Hammer_HitTickOffset(std::uint32_t interactionIndex) noexcept
		{ return 1u == interactionIndex ? 12u : HAMMER_HIT_TICK_OFFSET; }
		static constexpr float HAMMER_RANGE_M = 2.4f;
		/* cos(60 degrees): the swing covers a 120 degree arc in front. */
		static constexpr float HAMMER_HALF_ANGLE_COS = 0.5f;
		/* 일반 망치 입력 피해량이다. 미로 표적의 피격 adapter는 별도로 남은 HP 전부를 소비한다. */
		static constexpr std::uint32_t HAMMER_RAW_DAMAGE = 100u;
		/* MAZE HUD의 망치 skill ID다. 미로 표적은 다른 skill의 타격을 거절한다. */
		static constexpr LostArk::Shared::SKILL_ID HAMMER_SKILL_ID = 56411u;
		static constexpr const char* SPAWN_GROUP_TAG = "cardmaze.targets";
		/* The box a hammer swing must reach to take the telescope. */
		static constexpr const char* TELESCOPE_PLACEMENT_ID = "cardmaze.telescope";
		/* 입장 30tick 뒤 망원경 위에 생성하는 광대 상자다. 별도 group으로 일반 몬스터 피해를 받고 파괴 뒤 망원경을 연다. */
		static constexpr const char* CLOWN_BOX_ARCHETYPE_ID = "MONSTER_KOUKU_CLOWN_BOX";
		static constexpr const char* CLOWN_BOX_SPAWN_GROUP_TAG = "cardmaze.clownbox";
		static constexpr std::uint32_t CLOWN_BOX_SPAWN_DELAY_TICKS = 30u;
		/* CardMiro 영역의 world m 경계다. 0.5m 격자 116×114칸과 망원경 중심 좌표에 대응한다. */
		static constexpr float MAZE_MIN_X = -24.97f;
		static constexpr float MAZE_MAX_X = 33.03f;
		static constexpr float MAZE_MIN_Z = 1323.35f;
		static constexpr float MAZE_MAX_Z = 1380.35f;
		static constexpr float CENTER_X = 0.28f;
		static constexpr float CENTER_Z = 1351.65f;
		static constexpr float CENTER_KEEPOUT_M = 5.f;
		static constexpr float SPAWN_SPACING_M = 3.f;
		static constexpr float PLAYER_KEEPOUT_M = 4.f;
		static constexpr std::uint32_t SAMPLE_ATTEMPTS = 256u;

		static const char* Archetype_ForSuit(LostArk::Shared::MECHANIC_CARD_SYMBOL suit) noexcept;

		/* 역할·문양·표적 위치를 대기 상태로 준비한다. spawn 성공 뒤 Commit이 플레이어에 적용하고 실패 시 Abort로 버린다. */
		bool Plan(
			LostArk::Shared::PLAYER_ID claimantId,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation& navigation,
			std::uint32_t serverTick,
			std::vector<SPAWN_REQUEST>& outSpawns,
			std::string& outStatus);
		void Register_Target(LostArk::Shared::NET_ENTITY_ID entityId,
			LostArk::Shared::MECHANIC_CARD_SYMBOL suit);
		void Commit(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		void Abort();
		[[nodiscard]] bool Is_Target(LostArk::Shared::NET_ENTITY_ID entityId) const noexcept;
		/* A hammer hit the room already landed on a registered target. */
		HIT_OUTCOME On_TargetHit(SERVER_PLAYER& hunter, const SERVER_WORLD_ENTITY& target, bool killed);
		/* 진행 중인 사냥에서 문양을 가진 참가자가 모두 나갔으면 true를 반환해 룸의 종료 정리를 요청한다. */
		bool Remove_Player(LostArk::Shared::PLAYER_ID playerId);
		void Reset(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		bool Can_Hit(const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Retire_Target(LostArk::Shared::NET_ENTITY_ID targetId);
		bool Sample_Corridor(LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
			std::uint32_t seed, SPAWN_REQUEST& out, bool avoidPersonalExits = false) const;
		void Reset_Progress(SERVER_PLAYER& player);
		bool Toggle_Telescope(SERVER_PLAYER& player);
		[[nodiscard]] bool Is_SoloHunter(LostArk::Shared::PLAYER_ID playerId) const noexcept;
		void Mark_Escaped(SERVER_PLAYER& player);
		bool All_LivingCentral(const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players) const;
		static bool In_SafeZone(float x, float z) noexcept;
		void Complete() { m_ePhase = PHASE::COMPLETE; }
		[[nodiscard]] PHASE Get_Phase() const noexcept { return m_ePhase; }
		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_TelescopeOwner() const noexcept { return m_iTelescopeOwner; }
		[[nodiscard]] const std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::MECHANIC_CARD_SYMBOL>&
			Get_Targets() const noexcept { return m_Targets; }

	private:
		struct PARTICIPANT final
		{
			LostArk::Shared::CARD_MAZE_ROLE eRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
			LostArk::Shared::MECHANIC_CARD_SYMBOL eSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			std::uint8_t iKills = 0u;
			bool escaped = false;
		};
		static void Apply_ToPlayer(const PARTICIPANT& participant, SERVER_PLAYER& player) noexcept;

		PHASE m_ePhase = PHASE::INACTIVE;
		LostArk::Shared::PLAYER_ID m_iTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
		std::map<LostArk::Shared::PLAYER_ID, PARTICIPANT> m_Participants;
		LostArk::Shared::PLAYER_ID m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
		std::map<LostArk::Shared::PLAYER_ID, PARTICIPANT> m_PendingParticipants;
		/* Raised targets by entity id and the suit they show. */
		std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::MECHANIC_CARD_SYMBOL> m_Targets;
		/* Each felled target counts once, however long its body stays. */
		std::set<LostArk::Shared::NET_ENTITY_ID> m_CountedKills;
		bool m_bMarchStarted = false;
	};
}
```


### Server/Private/KoukuSaydonLogicRuntime.cpp

실제 수정 대상: [KoukuSaydonLogicRuntime.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp)


#### G2-direct-includes

현재 기준 블록:

```cpp
#include <algorithm>
```

교체 블록:

```cpp
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <utility>
```

#### G2-build-helper

현재 기준 블록:

```cpp
	float Wrap180(float degrees) noexcept
```

교체 블록:

```cpp
    // Build 전용: 성공 RESULT가 가리키는 조커와 같은 접촉 목록의 카드 7개를 찾는다.
    // Catalog는 읽기 전용이다. 검증 중 실패하면 해당 묶음의 원래 배치를 유지하고 부분 이동값은 넣지 않는다.
    void Build_JokerCardOffsets(
        const LostArk::Server::BOSS_PATTERN_DEFINITION& pattern,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss,
        const std::uint32_t startTick,
        std::map<std::string, std::array<float, 3u>>& outOffsets)
    {
        using namespace LostArk::Server;
        for (const auto& window : pattern.LogicWindows)
        {
            if (window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT || window.ContactTargets.size() != 7u)
            {
                continue;
            }
            for (const auto& result : window.OnSuccess)
            {
                if (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW ||
                    result.strContactTargetWorldOccurrenceId.empty() ||
                    outOffsets.contains(result.strContactTargetWorldOccurrenceId))
                {
                    continue;
                }
                const std::string& jokerId = result.strContactTargetWorldOccurrenceId;
                std::map<std::string, const BOSS_PATTERN_WORLD_SEQUENCE*> cards;
                bool valid = true;
                std::size_t jokerCount = 0u;
                for (const auto& target : window.ContactTargets)
                {
                    const BOSS_PATTERN_WORLD_SEQUENCE* sequence = nullptr;
                    for (const auto& candidate : pattern.WorldSequences)
                    {
                        if (candidate.strOccurrenceId != target.strWorldOccurrenceId)
                        {
                            continue;
                        }
                        if (sequence)
                        {
                            valid = false;
                            break;
                        }
                        sequence = &candidate;
                    }
                    if (!valid || !sequence || !sequence->Placement || sequence->CombatBody ||
                        !sequence->SupportWindows.empty() || sequence->bAuthoredMadness || sequence->bAnchorBossSpawn ||
                        sequence->fPositionOffsetX != 0.f || sequence->fPositionOffsetY != 0.f || sequence->fPositionOffsetZ != 0.f ||
                        sequence->strInstanceId != target.strWorldInstanceId ||
                        !std::isfinite(target.fWorldX) || !std::isfinite(target.fWorldZ) ||
                        !std::isfinite(target.fRadiusM) || target.fRadiusM <= 0.f ||
                        outOffsets.contains(target.strWorldOccurrenceId) ||
                        !cards.emplace(target.strWorldOccurrenceId, sequence).second)
                    {
                        valid = false;
                        break;
                    }
                    const bool isJoker = sequence->strInstanceId == "world.object.instance.kouku.joker_card";
                    if ((!isJoker && sequence->strInstanceId != "world.object.instance.kouku.card") ||
                        isJoker != (sequence->strOccurrenceId == jokerId))
                    {
                        valid = false;
                        break;
                    }
                    jokerCount += isJoker ? 1u : 0u;
                    const auto& p = *sequence->Placement;
                    if (!std::isfinite(p.fPositionX) || !std::isfinite(p.fPositionY) || !std::isfinite(p.fPositionZ) ||
                        !std::isfinite(p.fRotationXDegrees) || !std::isfinite(p.fRotationYDegrees) || !std::isfinite(p.fRotationZDegrees) ||
                        !std::isfinite(p.fScaleX) || !std::isfinite(p.fScaleY) || !std::isfinite(p.fScaleZ) ||
                        p.fScaleX <= 0.f || p.fScaleY <= 0.f || p.fScaleZ <= 0.f)
                    {
                        valid = false;
                        break;
                    }
                }
                if (!valid || jokerCount != 1u || cards.size() != 7u || !cards.contains(jokerId))
                {
                    continue;
                }
                const auto& jokerSequence = *cards.at(jokerId);
                const auto& joker = *jokerSequence.Placement;
                // 같은 회전·배율·출현 시각의 정적 카드에만 평행이동 공식을 적용한다.
                // 다른 orientation/scale을 허용하려면 구워진 접촉 중심도 inverse TRS→새 TRS로 바꾸는 별도 계약이 필요하다.
                for (const auto& [id, sequence] : cards)
                {
                    (void)id;
                    const auto& p = *sequence->Placement;
                    if (p.fRotationXDegrees != joker.fRotationXDegrees || p.fRotationYDegrees != joker.fRotationYDegrees ||
                        p.fRotationZDegrees != joker.fRotationZDegrees || p.fScaleX != joker.fScaleX ||
                        p.fScaleY != joker.fScaleY || p.fScaleZ != joker.fScaleZ ||
                        sequence->iStartMs != jokerSequence.iStartMs || sequence->iDurationMs != jokerSequence.iDurationMs)
                    {
                        valid = false;
                        break;
                    }
                }
                if (!valid)
                {
                    continue;
                }
                // stable occurrence 문자열을 해시한다. vector 순서나 displayName은 실행 identity가 아니다.
                std::uint64_t identity = 14695981039346656037ull;
                for (const unsigned char value : jokerId)
                {
                    identity = (identity ^ value) * 1099511628211ull;
                }
                const std::uint64_t seed = Mix(identity ^ (static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
                    boss.iPatternSequence ^ startTick);
                auto selected = cards.begin();
                std::advance(selected, static_cast<std::ptrdiff_t>(seed % cards.size()));
                const auto& slot = *selected->second->Placement;
                std::map<std::string, std::array<float, 3u>> staged;
                for (const auto& [id, sequence] : cards)
                {
                    (void)sequence;
                    staged.emplace(id, std::array<float, 3u>{0.f, 0.f, 0.f});
                }
                staged.at(jokerId) = {slot.fPositionX - joker.fPositionX, slot.fPositionY - joker.fPositionY, slot.fPositionZ - joker.fPositionZ};
                staged.at(selected->first) = {joker.fPositionX - slot.fPositionX, joker.fPositionY - slot.fPositionY, joker.fPositionZ - slot.fPositionZ};
                // 필요한 노드는 이미 지역 map에 있다. Room 소유 ledger에 묶음 전체를 넘겨 표시와 판정이 같은 결과를 읽게 한다.
                outOffsets.merge(staged);
            }
        }
    }

	float Wrap180(float degrees) noexcept
```

#### G2-build-commit

현재 기준 블록:

```cpp
	outLedger = std::move(ledger);
```

교체 블록:

```cpp
    Build_JokerCardOffsets(pattern, boss, startTick, ledger.WorldPositionOffsets);
	outLedger = std::move(ledger);
```

#### G2-world-play-position

현재 기준 블록:

```cpp
		play.Placement = sequence.Placement;
```

교체 블록:

```cpp
		play.Placement = sequence.Placement;
        const auto positionOffset = ledger.WorldPositionOffsets.find(sequence.strOccurrenceId);
        if (play.Placement && positionOffset != ledger.WorldPositionOffsets.end())
        {
            // packet에 실리는 Placement도 접촉 판정과 같은 이번 실행의 이동값을 사용한다.
            play.Placement->fPositionX += positionOffset->second[0u];
            play.Placement->fPositionY += positionOffset->second[1u];
            play.Placement->fPositionZ += positionOffset->second[2u];
        }
```

#### G2-contact-position

현재 기준 블록:

```cpp
				const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ circle{ target.fWorldX, target.fWorldZ, target.fRadiusM };
```

교체 블록:

```cpp
                const auto positionOffset = ledger.WorldPositionOffsets.find(target.strWorldOccurrenceId);
                const float offsetX = positionOffset == ledger.WorldPositionOffsets.end() ? 0.f : positionOffset->second[0u];
                const float offsetZ = positionOffset == ledger.WorldPositionOffsets.end() ? 0.f : positionOffset->second[2u];
                // 저작 target 중심에 구워진 template local offset을 유지한 채 카드와 같은 평행이동을 적용한다.
                const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ circle{
                    target.fWorldX + offsetX, target.fWorldZ + offsetZ, target.fRadiusM};
```


#### KoukuSaydonLogicRuntime.cpp 전체 코드

```cpp
#include "KoukuSaydonLogicRuntime.h"

#include "ServerCombatHitRuntime.h"
#include "PlayerSkillSystem.h"
#include "KoukuSaydonBrain.h"
#include "ServerCollisionSystem.h"
#include "ServerNavigation.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <cmath>
#include <limits>
//Ananymous Namespace 안에 존재하는 변수 함수, 왜 LostArk::Server 내부에 넣으면 안 되는 이유가 있을까?
namespace
{
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr double DEGREES_PER_RADIAN = 57.295779513082320876;
	constexpr float DEFAULT_CLOWN_HOLD_MS = 15000.f;

    bool Has_LethalResult(const std::vector<LostArk::Server::BOSS_PATTERN_LOGIC_RESULT>& results)
    {
        using namespace LostArk::Server;

        return std::any_of(results.begin(), results.end(), [](const auto& result) {
            return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
                (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
        });
    }

	bool Try_CounterLanding(const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::CServerNavigation* navigation,
		const LostArk::Server::CServerCollisionSystem* collision, float& landingY)
	{
		using namespace LostArk::Server;
		SERVER_NAV_POINT ground{}, spawnGround{};
		if (!navigation || !collision || !std::isfinite(boss.fPositionY) ||
			!std::isfinite(boss.fSpawnPositionY) ||
			!navigation->Is_PointWalkableExact(boss.fPositionX, boss.fPositionZ) ||
			!navigation->Sample_Position(boss.fPositionX, boss.fPositionZ, ground) ||
			!navigation->Sample_Position(boss.fSpawnPositionX, boss.fSpawnPositionZ, spawnGround)) return false;
		// Restore the placement's ground-relative height, not an airborne clip's last up.
		// This preserves intentionally raised boss placements in other gates.
		landingY = ground.y + (boss.fSpawnPositionY - spawnGround.y);
		return std::isfinite(landingY) && collision->Is_CirclePositionClear(
			boss.fPositionX, landingY, boss.fPositionZ, boss.fCollisionRadius,
			boss.fCollisionRadius, boss.fCollisionRadius, boss.iNetEntityId);
	}

	/* Stable server-owned card selection for one encounter admission. */
	std::uint64_t Mix(std::uint64_t value) noexcept
	{
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
		return value ^ (value >> 31);
	}

    // Build 전용: 성공 RESULT가 가리키는 조커와 같은 접촉 목록의 카드 7개를 찾는다.
    // Catalog는 읽기 전용이다. 검증 중 실패하면 해당 묶음의 원래 배치를 유지하고 부분 이동값은 넣지 않는다.
    void Build_JokerCardOffsets(
        const LostArk::Server::BOSS_PATTERN_DEFINITION& pattern,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss,
        const std::uint32_t startTick,
        std::map<std::string, std::array<float, 3u>>& outOffsets)
    {
        using namespace LostArk::Server;
        for (const auto& window : pattern.LogicWindows)
        {
            if (window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT || window.ContactTargets.size() != 7u)
            {
                continue;
            }
            for (const auto& result : window.OnSuccess)
            {
                if (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW ||
                    result.strContactTargetWorldOccurrenceId.empty() ||
                    outOffsets.contains(result.strContactTargetWorldOccurrenceId))
                {
                    continue;
                }
                const std::string& jokerId = result.strContactTargetWorldOccurrenceId;
                std::map<std::string, const BOSS_PATTERN_WORLD_SEQUENCE*> cards;
                bool valid = true;
                std::size_t jokerCount = 0u;
                for (const auto& target : window.ContactTargets)
                {
                    const BOSS_PATTERN_WORLD_SEQUENCE* sequence = nullptr;
                    for (const auto& candidate : pattern.WorldSequences)
                    {
                        if (candidate.strOccurrenceId != target.strWorldOccurrenceId)
                        {
                            continue;
                        }
                        if (sequence)
                        {
                            valid = false;
                            break;
                        }
                        sequence = &candidate;
                    }
                    if (!valid || !sequence || !sequence->Placement || sequence->CombatBody ||
                        !sequence->SupportWindows.empty() || sequence->bAuthoredMadness || sequence->bAnchorBossSpawn ||
                        sequence->fPositionOffsetX != 0.f || sequence->fPositionOffsetY != 0.f || sequence->fPositionOffsetZ != 0.f ||
                        sequence->strInstanceId != target.strWorldInstanceId ||
                        !std::isfinite(target.fWorldX) || !std::isfinite(target.fWorldZ) ||
                        !std::isfinite(target.fRadiusM) || target.fRadiusM <= 0.f ||
                        outOffsets.contains(target.strWorldOccurrenceId) ||
                        !cards.emplace(target.strWorldOccurrenceId, sequence).second)
                    {
                        valid = false;
                        break;
                    }
                    const bool isJoker = sequence->strInstanceId == "world.object.instance.kouku.joker_card";
                    if ((!isJoker && sequence->strInstanceId != "world.object.instance.kouku.card") ||
                        isJoker != (sequence->strOccurrenceId == jokerId))
                    {
                        valid = false;
                        break;
                    }
                    jokerCount += isJoker ? 1u : 0u;
                    const auto& p = *sequence->Placement;
                    if (!std::isfinite(p.fPositionX) || !std::isfinite(p.fPositionY) || !std::isfinite(p.fPositionZ) ||
                        !std::isfinite(p.fRotationXDegrees) || !std::isfinite(p.fRotationYDegrees) || !std::isfinite(p.fRotationZDegrees) ||
                        !std::isfinite(p.fScaleX) || !std::isfinite(p.fScaleY) || !std::isfinite(p.fScaleZ) ||
                        p.fScaleX <= 0.f || p.fScaleY <= 0.f || p.fScaleZ <= 0.f)
                    {
                        valid = false;
                        break;
                    }
                }
                if (!valid || jokerCount != 1u || cards.size() != 7u || !cards.contains(jokerId))
                {
                    continue;
                }
                const auto& jokerSequence = *cards.at(jokerId);
                const auto& joker = *jokerSequence.Placement;
                // 같은 회전·배율·출현 시각의 정적 카드에만 평행이동 공식을 적용한다.
                // 다른 orientation/scale을 허용하려면 구워진 접촉 중심도 inverse TRS→새 TRS로 바꾸는 별도 계약이 필요하다.
                for (const auto& [id, sequence] : cards)
                {
                    (void)id;
                    const auto& p = *sequence->Placement;
                    if (p.fRotationXDegrees != joker.fRotationXDegrees || p.fRotationYDegrees != joker.fRotationYDegrees ||
                        p.fRotationZDegrees != joker.fRotationZDegrees || p.fScaleX != joker.fScaleX ||
                        p.fScaleY != joker.fScaleY || p.fScaleZ != joker.fScaleZ ||
                        sequence->iStartMs != jokerSequence.iStartMs || sequence->iDurationMs != jokerSequence.iDurationMs)
                    {
                        valid = false;
                        break;
                    }
                }
                if (!valid)
                {
                    continue;
                }
                // stable occurrence 문자열을 해시한다. vector 순서나 displayName은 실행 identity가 아니다.
                std::uint64_t identity = 14695981039346656037ull;
                for (const unsigned char value : jokerId)
                {
                    identity = (identity ^ value) * 1099511628211ull;
                }
                const std::uint64_t seed = Mix(identity ^ (static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
                    boss.iPatternSequence ^ startTick);
                auto selected = cards.begin();
                std::advance(selected, static_cast<std::ptrdiff_t>(seed % cards.size()));
                const auto& slot = *selected->second->Placement;
                std::map<std::string, std::array<float, 3u>> staged;
                for (const auto& [id, sequence] : cards)
                {
                    (void)sequence;
                    staged.emplace(id, std::array<float, 3u>{0.f, 0.f, 0.f});
                }
                staged.at(jokerId) = {slot.fPositionX - joker.fPositionX, slot.fPositionY - joker.fPositionY, slot.fPositionZ - joker.fPositionZ};
                staged.at(selected->first) = {joker.fPositionX - slot.fPositionX, joker.fPositionY - slot.fPositionY, joker.fPositionZ - slot.fPositionZ};
                // 필요한 노드는 이미 지역 map에 있다. Room 소유 ledger에 묶음 전체를 넘겨 표시와 판정이 같은 결과를 읽게 한다.
                outOffsets.merge(staged);
            }
        }
    }

	float Wrap180(float degrees) noexcept
	{
		degrees = std::fmod(degrees, 360.f);
		if (degrees > 180.f)
			degrees -= 360.f;
		if (degrees <= -180.f)
			degrees += 360.f;
		return degrees;
	}

	float Wrap360(float degrees) noexcept
	{
		degrees = std::fmod(degrees, 360.f);
		if (degrees < 0.f)
			degrees += 360.f;
		return degrees;
	}

	std::uint32_t Percent_Of(const std::uint32_t value, const std::uint32_t percent) noexcept
	{
		const std::uint64_t scaled =
			(static_cast<std::uint64_t>(value) * percent + 50u) / 100u;
		return static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			scaled, (std::numeric_limits<std::uint32_t>::max)()));
	}


}

std::uint32_t LostArk::Server::CKoukuSaydonLogicRuntime::Ticks_FromMs(
	const std::uint32_t ms) noexcept
{
	const std::uint64_t ticks =
		(static_cast<std::uint64_t>(ms) * SERVER_TICK_HZ + 999u) / 1000u;
	return static_cast<std::uint32_t>((std::min<std::uint64_t>)(
		ticks, (std::numeric_limits<std::uint32_t>::max)() - 1u));
}

std::uint32_t LostArk::Server::CKoukuSaydonLogicRuntime::Add_Ticks(
	const std::uint32_t tick, const std::uint32_t ticks) noexcept
{
	// Tick 0 is the reserved "never" value, so a wrap skips it.
	const std::uint32_t result = tick + ticks;
	return 0u == result ? 1u : result;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Has_ReachedTick(
	const std::uint32_t serverTick, const std::uint32_t targetTick) noexcept
{
	if (0u == targetTick || 0u == serverTick)
		return false;
	return static_cast<std::int32_t>(serverTick - targetTick) >= 0;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_Judgeable(
	const SERVER_PLAYER& player) noexcept
{
	return player.Is_Human() && Can_ReceiveSpatialContact(player);
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Can_ReceiveSpatialContact(
	const SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	return 0u != player.iCurrentHp && player.isCombatReady &&
		PLAYER_ACTION_STATE::DEAD != player.eAction &&
		PLAYER_ACTION_STATE::FALLING != player.eAction &&
		PLAYER_ACTION_STATE::GRABBED != player.eAction;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Build(
	const BOSS_PATTERN_DEFINITION& pattern,
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t startTick,
	KOUKUSAYDON_LOGIC_LEDGER& outLedger)
{
	KOUKUSAYDON_LOGIC_LEDGER ledger{};
	ledger.strPatternId = pattern.strPatternId;
	ledger.iPatternSequence = boss.iPatternSequence;
	ledger.iPatternStartTick = startTick;
	std::map<std::string, std::pair<std::uint32_t, std::uint32_t>> scheduledDance;
	for (std::size_t index = 0u; index < pattern.LogicWindows.size(); ++index)
	{
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[index];
		KOUKUSAYDON_LOGIC_WINDOW_STATE state{};
		state.iWindowIndex = static_cast<std::uint32_t>(index);
		if (window.iRepeatIntervalMs && !window.strOwnerWorldOccurrenceId.empty())
			for (const auto& world : pattern.WorldSequences)
				if (world.strOccurrenceId == window.strOwnerWorldOccurrenceId && world.bAuthoredMadness)
				{
					if (world.strInstanceId == "world.object.instance.kouku.mario_circus_ball.aura") state.iMadnessSource = 1u;
					else if (world.strInstanceId == "world.object.instance.kouku.odd_doll.large.att_battle_2_01" ||
						world.strInstanceId == "world.object.instance.kouku.odd_doll.att_battle_2_01") state.iMadnessSource = 2u;
					state.iMadnessContactIntervalMs = window.iRepeatIntervalMs;
					break;
				}
		state.iStartTick = Add_Ticks(startTick, Ticks_FromMs(window.iStartMs));
		state.iEndTick = Add_Ticks(startTick,
			Ticks_FromMs(window.iStartMs + window.iDurationMs));
        if (!window.strHoldLogicOccurrenceId.empty())
        {
            const auto hold = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [&](const auto& row) {
                return row.strWindowId == window.strHoldLogicOccurrenceId && row.eKind == BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD;
            });
            if (hold != pattern.LogicWindows.end())
            {
                state.iHoldStartTick = Add_Ticks(startTick, Ticks_FromMs(hold->iStartMs));
                state.iHoldEndTick = Add_Ticks(startTick, Ticks_FromMs(hold->iStartMs + hold->iDurationMs));
            }
        }
		if (BOSS_PATTERN_LOGIC_KIND::POSE_INPUT == window.eKind)
		{
			ledger.bDanceActive = true;
			const auto parentScope = window.strWindowId.find(".pattern.");
			const auto lane = window.strWindowId.rfind(".logic.");
			if (parentScope != std::string::npos && lane != std::string::npos && lane > parentScope)
			{
				const auto scope = window.strWindowId.substr(0u, lane);
				const auto end = window.iStartMs + window.iDurationMs;
				const auto found = scheduledDance.find(scope);
				if (found == scheduledDance.end()) scheduledDance.emplace(scope, std::make_pair(window.iStartMs, end));
				else
				{
					found->second.first = (std::min)(found->second.first, window.iStartMs);
					found->second.second = (std::max)(found->second.second, end);
				}
			}
			else ledger.eHudMode = LostArk::Shared::KOUKU_HUD_MODE::DANCE;
		}
		ledger.Windows.push_back(std::move(state));
		if (window.eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT)
			ledger.ContactWindowOrder.push_back(static_cast<std::uint32_t>(index));
	}
	for (const auto& [scope, interval] : scheduledDance)
	{
		(void)scope;
		ledger.ScheduledDanceIntervals.emplace_back(Add_Ticks(startTick, Ticks_FromMs(interval.first)),
			Add_Ticks(startTick, Ticks_FromMs(interval.second)));
	}
	std::sort(ledger.ContactWindowOrder.begin(), ledger.ContactWindowOrder.end(), [&](const auto a, const auto b) {
		const auto& first = pattern.LogicWindows[a]; const auto& second = pattern.LogicWindows[b];
		return first.iContactPriority != second.iContactPriority ? first.iContactPriority > second.iContactPriority : first.strWindowId < second.strWindowId;
	});
	for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
	{
		const auto& trigger = pattern.MechanicTriggers[index];
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
		{
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE state;
			state.iTriggerIndex = static_cast<std::uint32_t>(index);
			state.iStartTick = Add_Ticks(startTick, Ticks_FromMs(trigger.iStartMs));
			state.iEndTick = Add_Ticks(startTick, Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
			// A one-shot pursuit Trigger owns a birth tick, even when its visual box is shorter than one tick.
			if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES && trigger.iSpawnIntervalMs == 0u &&
				!Has_ReachedTick(state.iEndTick, Add_Ticks(state.iStartTick, 1u)))
				state.iEndTick = Add_Ticks(state.iStartTick, 1u);
			if (trigger.ShowtimeBomb)
				state.iBombStartTick = Add_Ticks(startTick, Ticks_FromMs(trigger.ShowtimeBomb->iBodyStartMs));
			state.iNextFixedTick = state.iStartTick;
			state.iNextRandomTick = state.iStartTick;
			ledger.PlayerTargetWindows.push_back(std::move(state));
			// Presentation targeting still selects exactly once through its start cue.
			if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION) continue;
		}
		KOUKUSAYDON_LOGIC_CUE_STATE cue{};
		cue.iIndex = static_cast<std::uint32_t>(index);
		cue.iStartTick = Add_Ticks(startTick, Ticks_FromMs(pattern.MechanicTriggers[index].iStartMs));
		ledger.MechanicTriggers.push_back(cue);
	}
	std::stable_sort(ledger.MechanicTriggers.begin(), ledger.MechanicTriggers.end(), [&](const auto& a, const auto& b) {
		return pattern.MechanicTriggers[a.iIndex].iStartMs < pattern.MechanicTriggers[b.iIndex].iStartMs;
	});
	for (std::size_t index = 0u; index < pattern.WorldSequences.size(); ++index)
	{
		KOUKUSAYDON_LOGIC_CUE_STATE cue{};
		cue.iIndex = static_cast<std::uint32_t>(index);
		cue.iStartTick = Add_Ticks(startTick,
			Ticks_FromMs(pattern.WorldSequences[index].iStartMs));
		ledger.WorldSequences.push_back(cue);
	}
    Build_JokerCardOffsets(pattern, boss, startTick, ledger.WorldPositionOffsets);
	outLedger = std::move(ledger);
}

namespace
{
	const LostArk::Server::BOSS_LOGIC_REGION& Window_Region(
		const LostArk::Server::BOSS_LOGIC_REGION& region, const LostArk::Server::KOUKUSAYDON_LOGIC_WINDOW_STATE& state)
	{
		const auto found = state.FrozenRegions.find(region.strRegionId);
		return found == state.FrozenRegions.end() ? region : found->second;
	}

	struct LOGIC_REGION_TRANSFORM final
	{
		float centerX = 0.f, centerZ = 0.f, yaw = 0.f;
		float halfX = 0.f, halfZ = 0.f, radius = 0.f, radiusX = 0.f, radiusZ = 0.f;
		float centerY = 0.f, halfY = 0.f;
		bool hasGrip = false;
		std::array<float, 3u> grip{};
	};

	bool Resolve_LogicRegionTransform(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const double patternElapsedTicks, LOGIC_REGION_TRANSFORM& outTransform, const bool requireVisible = true) noexcept
	{
		using namespace LostArk::Server;
		// BOSS_START is resolved transactionally when its owning window opens.
		if (region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_START) return false;
		float centerX = region.fCenterX, centerZ = region.fCenterZ, yaw = region.fYawDegrees;
		float halfX = region.fHalfX, halfZ = region.fHalfZ, radius = region.fRadiusM;
		float radiusX = region.fRadiusXM > 0.f ? region.fRadiusXM : radius;
		float radiusZ = region.fRadiusZM > 0.f ? region.fRadiusZM : radius;
		float centerY = region.fCenterY, halfY = region.fHalfY;
		if (region.bLinearMotion)
		{
			if (region.iMotionDurationMs == 0u) return false;
			const double elapsedMs = patternElapsedTicks * (1000.0 / 30.0);
			const float alpha = static_cast<float>(std::clamp(
				(elapsedMs - region.iMotionStartMs) / region.iMotionDurationMs, 0.0, 1.0));
			centerX += region.EndPositionOffset[0] * alpha;
			centerY += region.EndPositionOffset[1] * alpha;
			centerZ += region.EndPositionOffset[2] * alpha;
			const float sx = 1.f + (region.EndScale[0] - 1.f) * alpha;
			const float sy = 1.f + (region.EndScale[1] - 1.f) * alpha;
			const float sz = 1.f + (region.EndScale[2] - 1.f) * alpha;
			halfX *= sx; halfY *= sy; halfZ *= sz;
			radius *= (std::max)(sx, sz); radiusX *= sx; radiusZ *= sz;
			if (region.bCircle || region.bCylinder) radius = (std::max)(radiusX, radiusZ);
		}
		bool hasGrip = false; std::array<float, 3u> grip{};
		if (region.WorldTrack.bEnabled)
		{
			const auto& track = region.WorldTrack;
			if (track.Keys.empty()) return false;
			const std::uint32_t startTicks = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs);
			if (patternElapsedTicks < startTicks) return false;
			const double delayed = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region.eAnchor ?
				static_cast<double>(patternElapsedTicks) * (1000.0 / 30.0) - track.iStartMs - track.iStartDelayMs :
				static_cast<double>(patternElapsedTicks - startTicks) * (1000.0 / 30.0) - track.iStartDelayMs;
			if (delayed < 0.0) return false;
			const double timeMs = (std::min)(static_cast<double>(track.iDurationMs), delayed * track.fPlaybackSpeed);
			const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), timeMs,
				[](const double time, const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) { return time < key.iTimeMs; });
			const auto& left = next == track.Keys.begin() ? track.Keys.front() : *(next - 1);
			const auto& right = next == track.Keys.end() ? left : *next;
			if (requireVisible && !left.bVisible) return false;
			float factor = right.iTimeMs > left.iTimeMs ?
				static_cast<float>((timeMs - left.iTimeMs) / (right.iTimeMs - left.iTimeMs)) : 0.f;
			factor = (std::clamp)(factor, 0.f, 1.f);
			if (track.bSmoothStep) factor = factor * factor * (3.f - 2.f * factor);
			const auto mix = [factor](const float a, const float b) { return a + (b - a) * factor; };
			const float sx = track.fBaselineScaleX * mix(left.fScaleX, right.fScaleX);
			const float sy = track.fBaselineScaleY * mix(left.fScaleY, right.fScaleY);
			const float sz = track.fBaselineScaleZ * mix(left.fScaleZ, right.fScaleZ);
			if (!(sx > 0.f && sz > 0.f)) return false;
			const float baselineRadians = track.fBaselineYawDegrees * 0.017453292519943295f;
			const float ox = mix(left.fOffsetX, right.fOffsetX), oz = mix(left.fOffsetZ, right.fOffsetZ);
			float qy = right.fRotationY, qw = right.fRotationW;
			float dot = left.fRotationY * qy + left.fRotationW * qw;
			if (dot < 0.f) { qy = -qy; qw = -qw; dot = -dot; }
			dot = (std::clamp)(dot, -1.f, 1.f);
			float leftWeight = 1.f - factor, rightWeight = factor;
			if (dot < 0.9995f)
			{
				const float omega = std::acos(dot), sine = std::sin(omega);
				leftWeight = std::sin((1.f - factor) * omega) / sine;
				rightWeight = std::sin(factor * omega) / sine;
			}
			qy = left.fRotationY * leftWeight + qy * rightWeight;
			qw = left.fRotationW * leftWeight + qw * rightWeight;
			const float qLength = std::sqrt(qy * qy + qw * qw);
			if (!(qLength > 0.f)) return false;
			qy /= qLength; qw /= qLength;
			const float worldYaw = track.fBaselineYawDegrees +
				std::atan2(2.f * qw * qy, 1.f - 2.f * qy * qy) * 57.29577951308232f;
			const float worldRadians = worldYaw * 0.017453292519943295f;
			const float localCenterX = centerX, localCenterY = centerY, localCenterZ = centerZ;
			centerX = track.fBaselineX + std::cos(baselineRadians) * ox + std::sin(baselineRadians) * oz +
				std::cos(worldRadians) * localCenterX * sx + std::sin(worldRadians) * localCenterZ * sz;
			centerZ = track.fBaselineZ - std::sin(baselineRadians) * ox + std::cos(baselineRadians) * oz -
				std::sin(worldRadians) * localCenterX * sx + std::cos(worldRadians) * localCenterZ * sz;
			yaw += worldYaw; halfX *= sx; halfZ *= sz; radius *= sx; radiusX *= sx; radiusZ *= sz;
			if (region.bCircle || region.bCylinder) radius = (std::max)(radiusX, radiusZ);
			centerY = track.fBaselineY + mix(left.fOffsetY, right.fOffsetY) + localCenterY * sy;
			halfY *= sy;
			hasGrip = left.bHasGripPosition && right.bHasGripPosition;
			if (hasGrip) for (size_t axis = 0; axis < 3u; ++axis) grip[axis] = mix(left.GripPosition[axis], right.GripPosition[axis]);
		}
		if (BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region.eAnchor)
		{
			// A baked bone tip is boss-local. Apply the live authoritative root once,
			// after its track and the authored TARGET_YAW offset have been combined.
			const float radians = boss.fYawDegrees * 0.017453292519943295f;
			const float localX = centerX, localZ = centerZ;
			centerX = boss.fPositionX + std::cos(radians) * localX + std::sin(radians) * localZ;
			centerZ = boss.fPositionZ - std::sin(radians) * localX + std::cos(radians) * localZ;
			yaw += boss.fYawDegrees;
			if (region.bCylinder || region.bLinearMotion) centerY += boss.fPositionY;
		}
		if (BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN == region.eAnchor)
		{
			centerX += boss.fSpawnPositionX;
			centerZ += boss.fSpawnPositionZ;
			if (region.bCylinder || region.bLinearMotion) centerY += boss.fSpawnPositionY;
		}
		outTransform = { centerX, centerZ, yaw, halfX, halfZ, radius, radiusX, radiusZ, centerY, halfY, hasGrip, grip };
		return true;
	}

	bool Contains_LogicRegion(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::SERVER_PLAYER& player, const std::uint32_t patternElapsedTicks = 0u) noexcept
	{
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform)) return false;
		const auto [centerX, centerZ, yaw, halfX, halfZ, radius, radiusX, radiusZ, centerY, halfY, hasGrip, grip] = transform;
		if (region.bCylinder || region.bLinearMotion)
		{
			using namespace LostArk::Shared::WorldCollision;
			using namespace LostArk::Shared::CombatCollision;
			if (!std::isfinite(player.fPositionY) || !std::isfinite(centerY) || !(halfY > 0.f) ||
				std::abs(player.fPositionY + PLAYER_CENTER_OFFSET_Y - centerY) >
					halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN) return false;
			const BODY_CIRCLE_XZ body{player.fPositionX, player.fPositionZ, PLAYER_HALF_EXTENT_X + CONTACT_MARGIN};
			const float inner = region.fInnerRadiusM * radius / region.fRadiusM;
			if (inner > 0.f && !Circle_IntersectsRing(body, centerX, centerZ, inner, radius)) return false;
			if (region.bCircle || region.bCylinder) return Circles_Overlap(CIRCLE_XZ{centerX, centerZ, radius}, body);
			const float radians = yaw * 0.017453292519943295f;
			const float fx = std::sin(radians), fz = std::cos(radians);
			if (region.bSector) return Circle_IntersectsEllipticSector(body, centerX, centerZ, fx, fz,
				radiusX, radiusZ, region.fHalfAngleDegrees * 2.f, region.bReverseSector);
			return Circle_IntersectsForwardBox(body, centerX - fx * halfZ, centerZ - fz * halfZ, fx, fz, 2.f * halfZ, halfX);
		}
		if (hasGrip && !region.bSector && !region.bCircle)
		{
			using namespace LostArk::Shared::WorldCollision;
			using namespace LostArk::Shared::CombatCollision;
			if (!std::isfinite(player.fPositionY) ||
				std::abs(player.fPositionY + PLAYER_CENTER_OFFSET_Y - centerY) >
					halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN) return false;
			const float radians = yaw * 0.017453292519943295f;
			const float forwardX = std::sin(radians), forwardZ = std::cos(radians);
			return Circle_IntersectsForwardBox(
				{ player.fPositionX, player.fPositionZ, PLAYER_HALF_EXTENT_X + CONTACT_MARGIN },
				centerX - forwardX * halfZ, centerZ - forwardZ * halfZ,
				forwardX, forwardZ, 2.f * halfZ, halfX);
		}
		if (hasGrip && std::abs(player.fPositionY - centerY) > halfY) return false;
		const float dx = player.fPositionX - centerX, dz = player.fPositionZ - centerZ;
		if (!std::isfinite(dx) || !std::isfinite(dz)) return false;
		const float radians = yaw * 0.017453292519943295f;
		const float localX = std::cos(radians) * dx - std::sin(radians) * dz;
		const float localZ = std::sin(radians) * dx + std::cos(radians) * dz;
		if (!region.bSector && !region.bCircle)
			return std::fabs(localX) <= halfX && std::fabs(localZ) <= halfZ;
		const float distanceSq = dx * dx + dz * dz;
		const float inner = region.fInnerRadiusM * radius / region.fRadiusM;
		if (inner > 0.f && distanceSq < inner * inner) return false;
		if (region.bCircle) return distanceSq <= radius * radius;
		const float nx = localX / radiusX, nz = localZ / radiusZ;
		if (nx * nx + nz * nz > 1.f) return false;
		if (region.bReverseSector && region.fHalfAngleDegrees <= 0.f) return true;
		if (region.bReverseSector && region.fHalfAngleDegrees >= 180.f) return false;
		if (distanceSq < 0.000001f) return false;
		const float angle = std::atan2(nx, nz) * 57.29577951308232f;
		// A shared radial edge belongs to exactly one sector.
		const bool inside = angle >= -region.fHalfAngleDegrees && angle < region.fHalfAngleDegrees;
		return region.bReverseSector ? !inside : inside;
	}

	// Clip the fixed-tick interval against vertical slabs, then the horizontal
	// swept volume. A thin rising pillar must still hit between two sampled poses.
	bool Swept_LinearColliderContainsPlayer(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const LostArk::Server::SERVER_PLAYER& player,
		const std::uint32_t elapsedTicks, const std::uint32_t windowStartTicks, const std::uint32_t windowEndTicks) noexcept
	{
		using namespace LostArk::Shared::WorldCollision;
		if (!region.bLinearMotion || region.WorldTrack.bEnabled || region.bSector || elapsedTicks <= windowStartTicks) return false;
		const double begin = (std::max)(double(elapsedTicks) - 1.0, double(windowStartTicks));
		const double end = (std::min)(double(elapsedTicks), double(windowEndTicks));
		LOGIC_REGION_TRANSFORM a, b;
		if (end <= begin || !Resolve_LogicRegionTransform(region, boss, begin, a) ||
			!Resolve_LogicRegionTransform(region, boss, end, b)) return false;
		double low = 0.0, high = 1.0;
		const auto bound = [&](double first, double last) {
			if (!std::isfinite(first) || !std::isfinite(last)) return false;
			const double delta = last - first;
			if (std::abs(delta) < 1e-12) return first <= 0.0;
			const double crossing = -first / delta;
			if (delta > 0.0) high = (std::min)(high, crossing); else low = (std::max)(low, crossing);
			return low <= high;
		};
		const auto slab = [&](double first, double last, double firstHalf, double lastHalf) {
			return bound(first - firstHalf, last - lastHalf) && bound(-first - firstHalf, -last - lastHalf);
		};
		const double bodyY = player.fPositionY + PLAYER_CENTER_OFFSET_Y;
		if (!slab(bodyY - a.centerY, bodyY - b.centerY,
			a.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN, b.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN)) return false;
		const double bodyRadius = PLAYER_HALF_EXTENT_X + CONTACT_MARGIN;
		const double ax = player.fPositionX - a.centerX, az = player.fPositionZ - a.centerZ;
		const double bx = player.fPositionX - b.centerX, bz = player.fPositionZ - b.centerZ;
		if (region.bCylinder || region.bCircle)
		{
			// Hollow circles keep the sampled test: sweeping only the outer disc would fill the hole.
			if (region.fInnerRadiusM > 0.f) return false;
			const double r = a.radius + bodyRadius, dr = b.radius - a.radius, dx = bx - ax, dz = bz - az;
			const double q2 = dx * dx + dz * dz - dr * dr;
			const double q1 = 2.0 * (ax * dx + az * dz - r * dr), q0 = ax * ax + az * az - r * r;
			const auto q = [&](double t) { return (q2 * t + q1) * t + q0; };
			double nearest = (std::min)(q(low), q(high));
			if (q2 > 1e-12) nearest = (std::min)(nearest, q(std::clamp(-q1 / (2.0 * q2), low, high)));
			return nearest <= 0.0;
		}
		const double yaw = a.yaw * .017453292519943295;
		const double x0 = std::cos(yaw) * ax - std::sin(yaw) * az, x1 = std::cos(yaw) * bx - std::sin(yaw) * bz;
		const double z0 = std::sin(yaw) * ax + std::cos(yaw) * az, z1 = std::sin(yaw) * bx + std::cos(yaw) * bz;
		// Split where the nearest box edge changes; within each interval distance is quadratic.
		std::array<double, 6u> cuts{low, high}; size_t count = 2u;
		const auto split = [&](double first, double last) {
			const double delta = last - first;
			if (std::abs(delta) < 1e-12) return;
			const double t = -first / delta;
			if (t > low && t < high) cuts[count++] = t;
		};
		split(x0 - a.halfX, x1 - b.halfX); split(-x0 - a.halfX, -x1 - b.halfX);
		split(z0 - a.halfZ, z1 - b.halfZ); split(-z0 - a.halfZ, -z1 - b.halfZ);
		std::sort(cuts.begin(), cuts.begin() + count);
		const auto gap = [](double first, double last, double halfFirst, double halfLast, double t) {
			return (std::max)(0.0, std::abs(first + (last - first) * t) - (halfFirst + (halfLast - halfFirst) * t));
		};
		for (size_t piece = 1u; piece < count; ++piece)
		{
			const double begin = cuts[piece - 1u], end = cuts[piece];
			const double x = gap(x0, x1, a.halfX, b.halfX, begin), z = gap(z0, z1, a.halfZ, b.halfZ, begin);
			const double dx = gap(x0, x1, a.halfX, b.halfX, end) - x, dz = gap(z0, z1, a.halfZ, b.halfZ, end) - z;
			const double length = dx * dx + dz * dz;
			const double t = length > 1e-18 ? std::clamp(-(x * dx + z * dz) / length, 0.0, 1.0) : 0.0;
			if ((x + dx * t) * (x + dx * t) + (z + dz * t) * (z + dz * t) <= bodyRadius * bodyRadius) return true;
		}
		return false;
	}

	// Sweep only the existing centered WORLD circle contract. Each visible
	// key interval is independent, so a hidden gap, delayed birth or expired
	// lifetime cannot become a phantom segment between two visible samples.
	bool Swept_CenteredWorldCircleContainsPlayer(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::SERVER_PLAYER& player, const std::uint32_t elapsedTicks,
		const std::uint32_t windowStartTicks, const std::uint32_t windowEndTicks) noexcept
	{
		using namespace LostArk::Server;
		using namespace LostArk::Shared::CombatCollision;
		if (region.bCylinder || region.bLinearMotion || !region.bCircle || region.fInnerRadiusM > 0.f || !region.WorldTrack.bEnabled || region.fCenterX != 0.f || region.fCenterZ != 0.f ||
			region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT || elapsedTicks <= windowStartTicks ||
			!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionZ)) return false;
		const auto& track = region.WorldTrack;
		if (track.Keys.size() < 2u || !(track.fPlaybackSpeed > 0.f)) return false;
		const double beginTick = (std::max)(double(elapsedTicks) - 1.0, double(windowStartTicks));
		const double endTick = (std::min)(double(elapsedTicks), double(windowEndTicks));
		const double originTick = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs) + track.iStartDelayMs * .03;
		const double beginMs = (beginTick - originTick) / .03 * track.fPlaybackSpeed;
		const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), beginMs,
			[](const double time, const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) { return time < key.iTimeMs; });
		const auto firstIndex = next == track.Keys.begin() ? 0u : static_cast<std::size_t>(next - track.Keys.begin() - 1u);
		for (std::size_t index = firstIndex; index + 1u < track.Keys.size(); ++index)
		{
			const auto& left = track.Keys[index];
			const auto& right = track.Keys[index + 1u];
			if (originTick + left.iTimeMs * .03 / track.fPlaybackSpeed >= endTick) break;
			if (!left.bVisible || left.iTimeMs >= right.iTimeMs) continue;
			const double begin = (std::max)(beginTick, originTick + left.iTimeMs * .03 / track.fPlaybackSpeed);
			const double end = (std::min)(endTick, originTick + right.iTimeMs * .03 / track.fPlaybackSpeed);
			if (end <= begin) continue;
			LOGIC_REGION_TRANSFORM first, last;
			// At the terminal key the pose is still the preceding visible segment's
			// endpoint. Its visibility applies only to the next segment.
			if (!Resolve_LogicRegionTransform(region, boss, begin, first, false) ||
				!Resolve_LogicRegionTransform(region, boss, end, last, false) ||
				std::abs(first.radius - last.radius) > .00001f) continue;
			if (Segment_IntersectsCircle(first.centerX, first.centerZ, last.centerX, last.centerZ,
				CIRCLE_XZ{player.fPositionX, player.fPositionZ, first.radius})) return true;
		}
		return false;
	}

    // Same visible key intervals as the circle sweep. New Object Tool floor
    // boxes keep a fixed yaw; translation and dimensions interpolate per key.
    bool Swept_WorldBoxContainsPlayer(const LostArk::Server::BOSS_LOGIC_REGION& region,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss, const LostArk::Server::SERVER_PLAYER& player,
        const uint32_t elapsedTicks, const uint32_t windowStartTicks, const uint32_t windowEndTicks) noexcept
    {
        using namespace LostArk::Server;
        if (region.bCylinder || region.bLinearMotion || region.bCircle || region.bSector || !region.WorldTrack.bEnabled ||
            region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT || elapsedTicks <= windowStartTicks) return false;
        const auto& track = region.WorldTrack;
        if (track.Keys.size() < 2u || track.bSmoothStep || !(track.fPlaybackSpeed > 0.f)) return false;
        const double beginTick = (std::max)(double(elapsedTicks) - 1.0, double(windowStartTicks));
        const double endTick = (std::min)(double(elapsedTicks), double(windowEndTicks));
        const double originTick = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs) + track.iStartDelayMs * .03;
        const double beginMs = (beginTick - originTick) / .03 * track.fPlaybackSpeed;
        const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), beginMs,
            [](double time, const auto& key) { return time < key.iTimeMs; });
        const size_t firstIndex = next == track.Keys.begin() ? 0u : size_t(next - track.Keys.begin() - 1u);
        for (size_t i = firstIndex; i + 1u < track.Keys.size(); ++i)
        {
            const auto& left = track.Keys[i]; const auto& right = track.Keys[i + 1u];
            if (originTick + left.iTimeMs * .03 / track.fPlaybackSpeed >= endTick) break;
            if (!left.bVisible || left.iTimeMs >= right.iTimeMs) continue;
            const double begin = (std::max)(beginTick, originTick + left.iTimeMs * .03 / track.fPlaybackSpeed);
            const double end = (std::min)(endTick, originTick + right.iTimeMs * .03 / track.fPlaybackSpeed);
            LOGIC_REGION_TRANSFORM a, b;
            if (end <= begin || !Resolve_LogicRegionTransform(region, boss, begin, a, false) ||
                !Resolve_LogicRegionTransform(region, boss, end, b, false) || std::abs(Wrap180(a.yaw - b.yaw)) > .0001f) continue;
            double low = 0.0, high = 1.0;
            const auto bound = [&](double first, double last) {
                if (!std::isfinite(first) || !std::isfinite(last)) return false;
                const double delta = last - first;
                if (std::abs(delta) < 1e-12) return first <= 0.0;
                const double cross = -first / delta;
                if (delta > 0.0) high = (std::min)(high, cross); else low = (std::max)(low, cross);
                return low <= high;
            };
            const auto axis = [&](double first, double last, double halfFirst, double halfLast) {
                return bound(first - halfFirst, last - halfLast) && bound(-first - halfFirst, -last - halfLast);
            };
            const double radians = a.yaw * .017453292519943295;
            const double cosine = std::cos(radians), sine = std::sin(radians);
            const double ax = player.fPositionX - a.centerX, az = player.fPositionZ - a.centerZ;
            const double bx = player.fPositionX - b.centerX, bz = player.fPositionZ - b.centerZ;
            if (a.hasGrip)
            {
                using namespace LostArk::Shared::WorldCollision;
                // Clip to the part of this visible interval where the body's
                // vertical span intersects the hook, then test its round XZ body.
                if (!axis(player.fPositionY + PLAYER_CENTER_OFFSET_Y - a.centerY,
                    player.fPositionY + PLAYER_CENTER_OFFSET_Y - b.centerY,
                    a.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN,
                    b.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN)) continue;
                const double x0 = cosine * ax - sine * az, x1 = cosine * bx - sine * bz;
                const double z0 = sine * ax + cosine * az, z1 = sine * bx + cosine * bz;
                std::array<double, 6u> cuts{ low, high };
                std::size_t count = 2u;
                const auto split = [&](double first, double last) {
                    const double delta = last - first;
                    if (std::abs(delta) > 1e-12)
                    {
                        const double t = -first / delta;
                        if (t > low && t < high) cuts[count++] = t;
                    }
                };
                split(x0 - a.halfX, x1 - b.halfX); split(-x0 - a.halfX, -x1 - b.halfX);
                split(z0 - a.halfZ, z1 - b.halfZ); split(-z0 - a.halfZ, -z1 - b.halfZ);
                std::sort(cuts.begin(), cuts.begin() + count);
                const auto gap = [](double first, double last, double halfFirst, double halfLast, double t) {
                    return (std::max)(0.0, std::abs(first + (last - first) * t) -
                        (halfFirst + (halfLast - halfFirst) * t));
                };
                const double radius = PLAYER_HALF_EXTENT_X + CONTACT_MARGIN;
                for (std::size_t piece = 1u; piece < count; ++piece)
                {
                    const double begin = cuts[piece - 1u], end = cuts[piece];
                    const double gx = gap(x0, x1, a.halfX, b.halfX, begin);
                    const double gz = gap(z0, z1, a.halfZ, b.halfZ, begin);
                    const double dx = gap(x0, x1, a.halfX, b.halfX, end) - gx;
                    const double dz = gap(z0, z1, a.halfZ, b.halfZ, end) - gz;
                    const double lengthSquared = dx * dx + dz * dz;
                    const double t = lengthSquared > 1e-18 ?
                        std::clamp(-(gx * dx + gz * dz) / lengthSquared, 0.0, 1.0) : 0.0;
                    if ((gx + dx * t) * (gx + dx * t) + (gz + dz * t) * (gz + dz * t) <= radius * radius)
                        return true;
                }
                continue;
            }
            if (axis(cosine * ax - sine * az, cosine * bx - sine * bz, a.halfX, b.halfX) &&
                axis(sine * ax + cosine * az, sine * bx + cosine * bz, a.halfZ, b.halfZ) &&
                (!a.hasGrip || axis(player.fPositionY - a.centerY, player.fPositionY - b.centerY, a.halfY, b.halfY))) return true;
        }
        return false;
    }

	bool Intersects_LogicRegion(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target,
		const std::uint32_t patternElapsedTicks) noexcept
	{
		using namespace LostArk::Shared::CombatCollision;
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform)) return false;
		const auto [centerX, centerZ, yaw, halfX, halfZ, radius, radiusX, radiusZ, centerY, halfY, hasGrip, grip] = transform;
		const float inner = region.fInnerRadiusM * radius / region.fRadiusM;
		if (inner > 0.f && !Circle_IntersectsRing(target, centerX, centerZ, inner, radius)) return false;
		if (region.bCircle || region.bCylinder) return Circles_Overlap(CIRCLE_XZ{ centerX, centerZ, radius }, target);
		const float radians = yaw * 0.017453292519943295f;
		const float forwardX = std::sin(radians), forwardZ = std::cos(radians);
		if (region.bSector && inner > 0.f)
			return region.fHalfAngleDegrees >= 180.f || Circle_IntersectsCone(target, centerX, centerZ,
				forwardX, forwardZ, radius, region.fHalfAngleDegrees * 2.f);
		if (region.bSector)
			return Circle_IntersectsEllipticSector(target, centerX, centerZ, forwardX, forwardZ,
				radiusX, radiusZ, region.fHalfAngleDegrees * 2.f, region.bReverseSector);
		// The shared box starts at its rear edge and extends along local +Z.
		return Circle_IntersectsForwardBox(target, centerX - forwardX * halfZ,
			centerZ - forwardZ * halfZ, forwardX, forwardZ, halfZ * 2.f, halfX);
	}

    const LostArk::Server::BOSS_LOGIC_WORLD_TRANSFORM_KEY* Resolve_HookReleaseKey(
        const LostArk::Server::BOSS_LOGIC_REGION& region) noexcept
    {
        using namespace LostArk::Server;
        const auto& track = region.WorldTrack;
        if (!track.bEnabled || track.Keys.size() < 2u) return nullptr;
        auto last = track.Keys.size() - 1u;
        while (last && !track.Keys[last].bVisible) --last;
        if (!track.Keys[last].bVisible) return nullptr;
        const auto position = [](const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) {
            return key.bHasGripPosition ? key.GripPosition :
                std::array<float, 3u>{key.fOffsetX, key.fOffsetY, key.fOffsetZ};
        };
        const auto end = position(track.Keys[last]);
        auto arrived = last;
        while (arrived)
        {
            const auto previous = position(track.Keys[arrived - 1u]);
            if (!track.Keys[arrived - 1u].bVisible || std::abs(previous[0] - end[0]) > .001f ||
                std::abs(previous[1] - end[1]) > .001f || std::abs(previous[2] - end[2]) > .001f) break;
            --arrived;
        }
        // A stationary carrier retains its authored deadline. A baked hook's final
        // upward exit carries only the hook: release at the beginning of that ascent.
        auto ascent = arrived;
        // Strict rise stops at the end of a level approach/hold; allowing equal
        // heights would rewind through that approach and drop the player early.
        while (region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && ascent &&
            track.Keys[ascent].bHasGripPosition && track.Keys[ascent - 1u].bVisible &&
            track.Keys[ascent - 1u].bHasGripPosition &&
            track.Keys[ascent - 1u].GripPosition[1] < track.Keys[ascent].GripPosition[1])
            --ascent;
        if (ascent < arrived && end[1] - track.Keys[ascent].GripPosition[1] >
            2.f * LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y)
            return &track.Keys[ascent];
        return arrived ? &track.Keys[arrived] : nullptr;
    }

    double Hook_TrackTimeMs(const LostArk::Server::BOSS_LOGIC_REGION& region,
        const std::uint32_t patternElapsedTicks) noexcept
    {
        const auto& track = region.WorldTrack;
        const auto startTicks = LostArk::Server::CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs);
        const double elapsed = region.eAnchor == LostArk::Server::BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT ?
            double(patternElapsedTicks) * (1000.0 / 30.0) - track.iStartMs :
            double(patternElapsedTicks - (std::min)(patternElapsedTicks, startTicks)) * (1000.0 / 30.0);
        return (elapsed - track.iStartDelayMs) * track.fPlaybackSpeed;
    }

	/* Puts the player on the region that just caught them. The region rides a
	World Object's transform track, so this is what "hanging from the hook"
	means on the Server: one attachment naming that region, and a deadline the
	room enforces even if this pattern stops running. A baked grip supplies
	XYZ; legacy regions without a grip keep the existing ground-drag height. */
	bool Hang_PlayerOnRegion(LostArk::Server::SERVER_PLAYER& player,
		const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const std::uint32_t patternElapsedTicks,
		const std::uint32_t windowIndex, const std::uint32_t regionIndex,
		const std::uint32_t releaseTick, const std::uint32_t serverTick,
		const LostArk::Server::CServerNavigation* navigation) noexcept
	{
		using namespace LostArk::Shared;
        const auto* release = Resolve_HookReleaseKey(region);
        if (release && release->bHasGripPosition && !region.WorldTrack.Keys.empty() &&
            region.WorldTrack.Keys.back().bHasGripPosition &&
            region.WorldTrack.Keys.back().GripPosition[1] - release->GripPosition[1] >
                2.f * LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y &&
            Hook_TrackTimeMs(region, patternElapsedTicks) >= release->iTimeMs) return false;
		LOGIC_REGION_TRANSFORM transform;
		if (0u == releaseTick || 0u == player.iCurrentHp ||
			PLAYER_ACTION_STATE::DEAD == player.eAction || PLAYER_ACTION_STATE::FEAR == player.eAction ||
			!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform) ||
			!std::isfinite(transform.centerX) || !std::isfinite(transform.centerZ) ||
			!std::isfinite(transform.yaw))
			return false;
		/* The hook lanes cross cells this Area never baked as walkable, so a
		navigation refusal must not cancel the catch. Sample the floor when the
		grid can answer and otherwise keep the height the runner already had. */
		LostArk::Server::SERVER_NAV_POINT ground{};
		const bool hasGround = nullptr != navigation &&
			navigation->Resolve_TraversalStep(player.fPositionX, player.fPositionZ,
				transform.centerX, transform.centerZ, ground);
		player.iAttachmentOwnerNetEntityId = boss.iNetEntityId;
		player.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP;
		player.iAttachmentPatternSequence = boss.iPatternSequence;
		player.fAttachmentLocalOffsetX = 0.f;
		player.fAttachmentLocalOffsetY = 0.f;
		player.fAttachmentLocalOffsetZ = 0.f;
		// Hook travels on local +X; player animations face local +Z.
		player.fAttachmentYawOffsetDegrees = 90.f;
		player.iAttachmentWindowIndex = windowIndex;
		player.iAttachmentRegionIndex = regionIndex;
		player.iAttachmentReleaseTick = releaseTick;
		player.eAction = PLAYER_ACTION_STATE::GRABBED;
		player.iActionStartTick = serverTick;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.hasReleasedHold = false;
		player.PendingCommand.Clear();
		player.TriggerMove = {};
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		player.isCombatReady = false;
		player.fPositionX = transform.hasGrip ? transform.grip[0] : transform.centerX;
		if (transform.hasGrip) player.fPositionY = transform.grip[1];
		else if (hasGround) player.fPositionY = ground.y;
		player.fPositionZ = transform.hasGrip ? transform.grip[2] : transform.centerZ;
		player.fYawDegrees = Wrap180(transform.yaw + 90.f);
		return true;
	}

    bool Resolve_HookReleaseGround(const LostArk::Server::CServerNavigation* navigation,
        const std::array<float, 3u>& grip, LostArk::Server::SERVER_NAV_POINT& ground) noexcept
    {
        if (!navigation) return false;
        // Grip height is above the deck. An unbaked cell under it has no valid
        // same-level reference, so admit the ordinary spawn projection only
        // within one player body of the endpoint, on this exact navigation grid.
        constexpr float maximumDistance = 2.f * LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y;
        const auto nearbyFloor = [&] {
            return std::isfinite(ground.x) && std::isfinite(ground.y) && std::isfinite(ground.z) &&
                std::abs(ground.y - grip[1]) <= maximumDistance &&
                std::hypot(ground.x - grip[0], ground.z - grip[2]) <= maximumDistance &&
                navigation->Is_InSameNavigationGrid(grip[0], grip[2], ground.x, ground.z) &&
                navigation->Is_PointWalkableExact(ground.x, ground.z, ground.y);
        };
        if (navigation->Project_PointOnSameLevel(grip[0], grip[2], ground, grip[1]) && nearbyFloor()) return true;
        return navigation->Project_Point(grip[0], grip[2], ground, grip[1]) && nearbyFloor();
    }

    /* The grip carries the player until its terminal ascent, not until its
    airborne final key. The room still owns the actual attachment release. */
    bool Drag_HookedPlayer(LostArk::Server::SERVER_PLAYER& player,
        const LostArk::Server::BOSS_LOGIC_REGION& region,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss,
        const std::uint32_t patternElapsedTicks, const std::uint32_t serverTick,
        const LostArk::Server::CServerNavigation* navigation) noexcept
    {
        using namespace LostArk::Server;
        const auto* release = Resolve_HookReleaseKey(region);
        if (region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && release && release->bHasGripPosition &&
            Hook_TrackTimeMs(region, patternElapsedTicks) >= release->iTimeMs)
        {
            // Baked grip XYZ is already in world metres, just as in
            // Resolve_LogicRegionTransform; never apply the boss basis again.
            const auto& position = release->GripPosition;
            SERVER_NAV_POINT ground{};
            if (Resolve_HookReleaseGround(navigation, position, ground))
            {
                player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
                player.hasMoveGoal = false;
                return false;
            }
            // A failed floor admission must not resume the upward carrier or release
            // an airborne body. Retry this same boundary while the carrier owns it.
            player.fPositionX = position[0]; player.fPositionY = position[1]; player.fPositionZ = position[2];
            // Player attachment deadlines run before Kouku logic on the next
            // room tick, so keep ownership through that pre-logic update.
            player.iAttachmentReleaseTick = CKoukuSaydonLogicRuntime::Add_Ticks(serverTick, 2u);
            return true;
        }
        LOGIC_REGION_TRANSFORM transform;
        const bool visible = Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform);
        if ((!visible && !Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform, false)) ||
            !std::isfinite(transform.centerX) || !std::isfinite(transform.centerZ) || !std::isfinite(transform.yaw))
            return false;
        SERVER_NAV_POINT ground{};
        const bool hasGround = navigation && navigation->Resolve_TraversalStep(player.fPositionX,
            player.fPositionZ, transform.centerX, transform.centerZ, ground);
        player.fPositionX = transform.hasGrip ? transform.grip[0] : transform.centerX;
        if (transform.hasGrip) player.fPositionY = transform.grip[1];
        else if (hasGround) player.fPositionY = ground.y;
        player.fPositionZ = transform.hasGrip ? transform.grip[2] : transform.centerZ;
        player.fYawDegrees = Wrap180(transform.yaw + player.fAttachmentYawOffsetDegrees);
        player.hasMoveGoal = false;
        player.isCombatReady = false;
        if (!visible) return false;
        return !release || Hook_TrackTimeMs(region, patternElapsedTicks) < release->iTimeMs;
    }
}

float LostArk::Server::CKoukuSaydonLogicRuntime::Predict_ContactRisk(
    const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
    const KOUKUSAYDON_LOGIC_LEDGER& ledger, const SERVER_PLAYER& probe,
    const std::uint32_t serverTick, const std::uint32_t horizonMs)
{
    if (!ledger.Is_Active() || !boss.iCurrentHp || ledger.iPatternSequence != boss.iPatternSequence)
        return 0.f;
    float risk = 0.f;
    const auto horizon = Ticks_FromMs((std::min)(horizonMs, 5000u));
    for (const auto& state : ledger.Windows)
    {
        if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size()) continue;
        const auto& window = pattern.LogicWindows[state.iWindowIndex];
        if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
            window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP) continue;
        if (!window.strOwnerWorldOccurrenceId.empty() && ledger.RetiredWorldOccurrences.contains(window.strOwnerWorldOccurrenceId)) continue;
        if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.bRearmOnExit &&
            !window.bRepeatAfterKnockback && !window.iRepeatIntervalMs)
        {
            const auto answer = state.Answers.find(probe.iPlayerId);
            if (answer != state.Answers.end() && answer->second == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS) continue;
        }
        const auto& results = window.bInsideIsFail ? window.OnFail : window.OnSuccess;
        float severity = 0.f;
        for (const auto& result : results)
        {
            if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH) severity = 5.f;
            else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT)
                severity = (std::max)(severity, 1.f);
        }
        if (!severity) continue;
        bool contact = false;
        for (std::uint32_t step = 0u; step <= horizon && !contact; ++step)
        {
            const auto tick = Add_Ticks(serverTick, step);
            if (!Has_ReachedTick(tick, state.iStartTick)) continue;
            const bool end = Has_ReachedTick(tick, state.iEndTick);
            if (end && tick != state.iEndTick) break;
            if (window.iRepeatIntervalMs && end) break;
            if (window.eKind == BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP && !window.iRepeatIntervalMs && !end) continue;
            const auto elapsed = tick - ledger.iPatternStartTick;
            for (const auto& authored : window.CardRegions)
            {
                const auto& region = Window_Region(authored, state);
                if (Contains_LogicRegion(region, boss, probe, elapsed) ||
                    Swept_LinearColliderContainsPlayer(region, boss, probe, elapsed,
                        state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
                    Swept_CenteredWorldCircleContainsPlayer(region, boss, probe, elapsed,
                        state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
                    Swept_WorldBoxContainsPlayer(region, boss, probe, elapsed,
                        state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick))
                { contact = true; break; }
            }
        }
        if (contact) risk += severity;
    }
    return risk;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Assign_EncounterCard(
	SERVER_PLAYER& player, const LostArk::Shared::NET_ENTITY_ID encounterOwnerId,
	const std::uint32_t serverTick, const std::uint8_t unavailableSymbols)
{
	using namespace LostArk::Shared;
	if (player.Is_Guide()) return;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		INVALID_NET_ENTITY_ID == encounterOwnerId)
		return;
	if (MECHANIC_CARD_SYMBOL::NONE != player.eMechanicCardSymbol &&
		MECHANIC_CARD_COLOR::NONE != player.eMechanicCardColor)
		return;
	const std::uint64_t roll = Mix((static_cast<std::uint64_t>(encounterOwnerId) << 32) ^
		(static_cast<std::uint64_t>(player.iPlayerId) << 8) ^ serverTick);
	std::uint8_t available[4]{};
	std::uint32_t count = 0u;
	for (std::uint8_t symbol = 1u; symbol <= 4u; ++symbol)
		if (0u == (unavailableSymbols & (1u << (symbol - 1u)))) available[count++] = symbol;
	if (!count) return;
	player.eMechanicCardSymbol = static_cast<MECHANIC_CARD_SYMBOL>(available[roll % count]);
	// Color is independent of the remaining suit deck.
	player.eMechanicCardColor = 0u == (Mix(roll) & 1u) ? MECHANIC_CARD_COLOR::RED : MECHANIC_CARD_COLOR::BLACK;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Discard(
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	SERVER_WORLD_ENTITY* const pBoss)
{
	using namespace LostArk::Shared;
	if (!ledger.Is_Active())
		return;
	Reveal_CardMazeEntryPlayers(ledger, players);
	if (nullptr != pBoss)
	{
		pBoss->iKoukuItemStaggerMaximum = 0u;
		pBoss->iKoukuItemStaggerCredit = 0u;
		pBoss->iKoukuDamageReductionWindows = 0u;
		pBoss->bKoukuShieldActive = false;
		pBoss->fKoukuShieldArcDegrees = 0.f;
		pBoss->KoukuShieldRegions.clear();
        (void)CBossCombatRuntime::Set_Flag(pBoss->BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
        CBossCombatRuntime::Clear_PatternOutcomes(*pBoss);
	}
	for (const auto& state : ledger.Windows)
		for (const auto& [id, entityId] : state.BoundPlayers)
		{
			const auto found = players.find(id);
			if (found != players.end() && found->second.iNetEntityId == entityId && pBoss &&
				found->second.iPatternBindOwnerNetEntityId == pBoss->iNetEntityId &&
				found->second.iPatternBindSequence == ledger.iPatternSequence)
				found->second.Clear_PatternBindStatus();
		}
	if (pBoss)
		for (auto& [id, player] : players)
			if (player.bPatternBound && player.iPatternBindOwnerNetEntityId == pBoss->iNetEntityId &&
				player.iPatternBindSequence == ledger.iPatternSequence) player.Clear_PatternBindStatus();
	ledger = {};
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Capture_CardMazeEntryRoster(
	KOUKUSAYDON_LOGIC_LEDGER& ledger, const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	if (ledger.bCardMazeEntryRosterCaptured) return;
	ledger.bCardMazeEntryRosterCaptured = true;
	for (const auto& [id, player] : players)
		if (player.Is_Human() && (Is_Judgeable(player) || !player.iCurrentHp) && player.eCardMazeRole == LostArk::Shared::CARD_MAZE_ROLE::NONE)
			ledger.CardMazeEntryPlayers.push_back(id);
	std::sort(ledger.CardMazeEntryPlayers.begin(), ledger.CardMazeEntryPlayers.end(), [&](auto left, auto right) {
		const float lx = players.at(left).fPositionX, rx = players.at(right).fPositionX;
		return lx != rx ? lx > rx : left < right;
	});
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Reveal_CardMazeEntryPlayers(
	const KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	for (const auto id : ledger.CardMazeEntryPlayers)
		if (auto found = players.find(id); found != players.end())
			found->second.CardMaze.flags &= static_cast<std::uint8_t>(~LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN);
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Stage_CardMazeEntryPlayers(
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus)
{
	using namespace LostArk::Shared;
	Capture_CardMazeEntryRoster(ledger, players);
	const auto reject = [&](const char* reason) {
		Reveal_CardMazeEntryPlayers(ledger, players);
		// Keep the captured flag: later hide/enter cues must not recapture a rejected run.
		ledger.CardMazeEntryPlayers.clear();
		ledger.iCardMazeNextHiddenPlayer = 0u;
		outStatus = std::string("Card maze staging preserved all player positions: ") + reason;
		return false;
	};
	if (!navigation || !navigation->Is_Loaded() || trigger.PlayerEntryPositions.empty() ||
		trigger.PlayerEntryPositions.size() > 4u || ledger.CardMazeEntryPlayers.empty() ||
		ledger.CardMazeEntryPlayers.size() > trigger.PlayerEntryPositions.size())
		return reject("navigation, roster or destination count is unavailable");
	std::vector<SERVER_NAV_POINT> grounds;
	for (const auto& position : trigger.PlayerEntryPositions)
	{
		SERVER_NAV_POINT ground{};
		if (std::any_of(position.begin(), position.end(), [](float value) { return !std::isfinite(value) || std::abs(value) > 100000.f; }) ||
			!navigation->Is_PointWalkableExact(position[0], position[2]) ||
			!navigation->Sample_Position(position[0], position[2], ground) || std::abs(ground.y - position[1]) > 1.f)
			return reject("a destination is not on its authored navigation floor");
		grounds.push_back(ground);
	}
	std::vector<std::pair<PLAYER_ID, SERVER_PLAYER>> staged;
	for (std::size_t slot = 0u; slot < ledger.CardMazeEntryPlayers.size(); ++slot)
	{
		const auto id = ledger.CardMazeEntryPlayers[slot];
		const auto found = players.find(id);
		if (found == players.end()) return reject("an entry participant is no longer present");
		const auto& source = found->second;
		const auto& position = trigger.PlayerEntryPositions[slot];
		if ((source.iCurrentHp && !Is_Judgeable(source)) || source.bPatternBound || source.TriggerMove.isActive || source.iMarioStage ||
			source.bArenaEjectionActive || source.CardMaze.transferStartTick || source.eCardMazeRole != CARD_MAZE_ROLE::NONE ||
			(collision && !collision->Is_PlayerPositionClear(position[0], grounds[slot].y, position[2], source.iNetEntityId)))
			return reject("a participant or destination is unavailable");
		SERVER_PLAYER player = source;
		player.Clear_CardMazeState(); player.Clear_KoukuInteractionState();
		player.fPositionX = position[0]; player.fPositionY = grounds[slot].y; player.fPositionZ = position[2];
		player.hasMoveGoal = false; player.MovePath.clear(); player.iMovePathIndex = 0u;
		player.PendingCommand.Clear(); player.Clear_SkillTarget(); player.Projectiles.clear();
		player.eAction = player.iCurrentHp ? PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD; player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0u; player.fActionElapsedSeconds = 0.f;
		player.iComboStage = 0u; player.hasBufferedComboInput = false;
		player.fKnockbackRemainingSeconds = 0.f;
		staged.emplace_back(id, std::move(player));
	}
	for (auto& [id, player] : staged) players.at(id) = std::move(player);
	ledger.iCardMazeNextHiddenPlayer = 0u;
	outStatus = "Card maze cinematic staging committed";
	return true;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Enter_CardMaze(
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus)
{
	using namespace LostArk::Shared;
	Capture_CardMazeEntryRoster(ledger, players);
	const auto reject = [&](const char* reason) {
		Reveal_CardMazeEntryPlayers(ledger, players);
		outStatus = std::string("Card maze entry preserved all player positions: ") + reason;
		return false;
	};
	SERVER_NAV_POINT ground{};
	if (!navigation || !navigation->Is_Loaded() ||
		!navigation->Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ) ||
		!navigation->Sample_Position(trigger.fTeleportX, trigger.fTeleportZ, ground) ||
		std::abs(ground.y - trigger.fTeleportY) > 1.f ||
		!CKoukuCardMazeRuntime::In_SafeZone(trigger.fTeleportX, trigger.fTeleportZ))
		return reject("destination is not on the maze central navigation");
	std::vector<std::pair<PLAYER_ID, SERVER_PLAYER>> staged;
	for (const auto id : ledger.CardMazeEntryPlayers)
	{
		const auto found = players.find(id);
		if (found == players.end()) continue;
		const auto& source = found->second;
		if ((source.iCurrentHp && !Is_Judgeable(source)) || source.bPatternBound || source.TriggerMove.isActive ||
			source.bArenaEjectionActive || source.CardMaze.transferStartTick ||
			source.eCardMazeRole != CARD_MAZE_ROLE::NONE ||
			(collision && !collision->Is_PlayerPositionClear(trigger.fTeleportX, ground.y, trigger.fTeleportZ, source.iNetEntityId)))
			return reject("a participant or destination is unavailable");
		SERVER_PLAYER player = source;
		player.Clear_CardMazeState();
		player.Clear_KoukuInteractionState();
		player.eKoukuAreaHudMode = KOUKU_HUD_MODE::MAZE;
		player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		player.fPositionX = trigger.fTeleportX; player.fPositionY = ground.y; player.fPositionZ = trigger.fTeleportZ;
		player.hasMoveGoal = false; player.MovePath.clear(); player.iMovePathIndex = 0u;
		player.PendingCommand.Clear(); player.Clear_SkillTarget(); player.Projectiles.clear();
		player.eAction = player.iCurrentHp ? PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD; player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0u; player.fActionElapsedSeconds = 0.f;
		player.iComboStage = 0u; player.hasBufferedComboInput = false;
		player.fKnockbackRemainingSeconds = 0.f;
		staged.emplace_back(id, std::move(player));
	}
	if (staged.empty()) return reject("no entry participant remains");
	for (auto& [id, player] : staged) players.at(id) = std::move(player);
	Reveal_CardMazeEntryPlayers(ledger, players);
	outStatus = "Card maze entry committed; strike the central telescope with Q to begin";
	return true;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_PointInsideRegion(
	const BOSS_LOGIC_REGION& region, const SERVER_WORLD_ENTITY& boss,
	const float worldX, const float worldZ) noexcept
{
	if (!std::isfinite(worldX) || !std::isfinite(worldZ) || !std::isfinite(boss.fPositionX) ||
		!std::isfinite(boss.fPositionZ) || !std::isfinite(boss.fYawDegrees)) return false;
	LOGIC_REGION_TRANSFORM transform;
	if (!region.bSector || region.bReverseSector || region.bLinearMotion || region.WorldTrack.bEnabled ||
		!Resolve_LogicRegionTransform(region, boss, 0u, transform) ||
		!(transform.radiusX > 0.f) || !(transform.radiusZ > 0.f)) return false;
	// A bomb is a point, so no player radius or shared-edge ownership is added.
	// This is the inclusive point-interior branch of the shared elliptic sector.
	const double radians = double(transform.yaw) * 0.017453292519943295;
	const double dx = double(worldX) - transform.centerX, dz = double(worldZ) - transform.centerZ;
	const double nx = (std::cos(radians) * dx - std::sin(radians) * dz) / transform.radiusX;
	const double nz = (std::sin(radians) * dx + std::cos(radians) * dz) / transform.radiusZ;
	return nx * nx + nz * nz <= 1.0 &&
		std::abs(std::atan2(nx, nz)) <= double(region.fHalfAngleDegrees) * 0.017453292519943295;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Capture_StartRegions(
	const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state, const std::uint32_t patternElapsedTicks)
{
	for (const auto& region : window.CardRegions)
	{
		if (region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_START ||
			state.FrozenRegions.contains(region.strRegionId)) continue;
		const auto captureStartMs = region.iAnchorCaptureStartMs == 0xffffffffu ? window.iStartMs : region.iAnchorCaptureStartMs;
		if (patternElapsedTicks < Ticks_FromMs(captureStartMs)) continue;
		auto source = region;
		source.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
		source.bLinearMotion = false;
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(source, boss, double(captureStartMs) * .03, transform)) continue;
		auto frozen = region;
		frozen.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD;
		frozen.WorldTrack = {};
		frozen.fCenterX = transform.centerX; frozen.fCenterZ = transform.centerZ;
		frozen.fCenterY = transform.centerY + ((source.bCylinder || source.bLinearMotion) ? 0.f : boss.fPositionY);
		frozen.fYawDegrees = transform.yaw;
		frozen.fHalfX = transform.halfX; frozen.fHalfY = transform.halfY; frozen.fHalfZ = transform.halfZ;
		frozen.fRadiusM = transform.radius; frozen.fRadiusXM = transform.radiusX; frozen.fRadiusZM = transform.radiusZ;
		const float yaw = (transform.yaw - region.fYawDegrees) * .017453292519943295f;
		const float x = region.EndPositionOffset[0], z = region.EndPositionOffset[2];
		frozen.EndPositionOffset[0] = std::cos(yaw) * x + std::sin(yaw) * z;
		frozen.EndPositionOffset[2] = -std::sin(yaw) * x + std::cos(yaw) * z;
		state.FrozenRegions.emplace(region.strRegionId, std::move(frozen));
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Open_Window(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	const KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	(void)ledger;
	state.bOpened = true;
	state.Answers.clear();
	state.InsidePlayers.clear();
	state.NextContactHitTicks.clear();

	Capture_StartRegions(boss, window, state, Ticks_FromMs(window.iStartMs));
	switch (window.eKind)
	{
	case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
	{
		std::vector<SERVER_PLAYER*> eligible;
		for (auto& [id, player] : players)
			if (Is_Judgeable(player) && !player.bPatternBound && !player.iMarioStage &&
				!player.TriggerMove.isActive && player.eCardMazeRole == CARD_MAZE_ROLE::NONE)
				eligible.push_back(&player);
		if (eligible.empty()) break;
		auto* freePlayer = eligible[Mix((static_cast<std::uint64_t>(boss.iPatternSequence) << 32u) ^
			boss.iNetEntityId ^ state.iStartTick) % eligible.size()];
		state.iFreePlayerNetEntityId = freePlayer->iNetEntityId;
		boss.iTargetEntityId = boss.iPatternTargetEntityId = freePlayer->iNetEntityId;
		for (auto* player : eligible)
		{
			if (player == freePlayer) continue;
			player->fPatternBindRestoreX = player->fPositionX;
			player->fPatternBindRestoreY = player->fPositionY;
			player->fPatternBindRestoreZ = player->fPositionZ;
			player->fPatternBindRestoreYawDegrees = player->fYawDegrees;
			player->bPatternBindRestoreCombatReady = player->isCombatReady;
			player->bPatternBound = true;
			player->iPatternBindOwnerNetEntityId = boss.iNetEntityId;
			player->iPatternBindSequence = boss.iPatternSequence;
			player->iPatternBindEndTick = state.iEndTick;
			player->eAction = PLAYER_ACTION_STATE::NONE;
			player->iCurrentSkillId = INVALID_SKILL_ID;
			player->iActionStartTick = 0u;
			player->fActionElapsedSeconds = 0.f;
			player->Clear_SkillTarget();
			player->iComboStage = 0u;
			player->hasBufferedComboInput = false;
			player->hasReleasedHold = false;
			player->PendingCommand.Clear();
			player->hasMoveGoal = false;
			player->MovePath.clear();
			player->iMovePathIndex = 0u;
			player->iFearEndTick = 0u;
			player->strFearPresentationId.clear();
			player->iKnockdownEndTick = 0u;
			player->fKnockbackRemainingSeconds = 0.f;
			state.BoundPlayers.emplace(player->iPlayerId, player->iNetEntityId);
		}
		break;
	}
	case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
		for (auto& [playerId, player] : players)
			if (Is_Judgeable(player))
				state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::NONE;
		break;
    case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
        (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
        break;
	case BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION:
		++boss.iKoukuDamageReductionWindows;
		break;
	case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
		boss.iKoukuItemStaggerMaximum = window.iThreshold;
		boss.iKoukuItemStaggerCredit = 0u;
		boss.bKoukuShieldActive = window.fShieldArcDegrees > 0.f;
		boss.fKoukuShieldArcDegrees = window.fShieldArcDegrees;
		boss.fKoukuShieldNormalYawOffsetDegrees = window.fNormalYawOffsetDegrees;
		boss.KoukuShieldRegions = window.CardRegions;
		break;
	case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
	case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
	case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
    case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
		// One world object is judged independently of the party roster.
		break;
	case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
	case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
	default:
		for (auto& [playerId, player] : players)
		{
			if (Is_Judgeable(player))
				state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::NONE;
		}
		break;
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Close_Window(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	if (state.bClosed) return;
	state.bClosed = true;
	if (state.bOpened && window.eKind == BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION && boss.iKoukuDamageReductionWindows)
		--boss.iKoukuDamageReductionWindows;
    if (window.eKind == BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW)
        (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
	for (const auto& [id, entityId] : state.BoundPlayers)
	{
		const auto found = players.find(id);
		if (found != players.end() && found->second.iNetEntityId == entityId &&
			found->second.iPatternBindOwnerNetEntityId == boss.iNetEntityId &&
			found->second.iPatternBindSequence == boss.iPatternSequence)
			found->second.Clear_PatternBindStatus();
	}
	state.BoundPlayers.clear();
	(void)players;
	if (BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW == window.eKind)
	{
		boss.iKoukuItemStaggerMaximum = 0u;
		boss.iKoukuItemStaggerCredit = 0u;
		boss.bKoukuShieldActive = false;
		boss.fKoukuShieldArcDegrees = 0.f;
		boss.KoukuShieldRegions.clear();
	}
}

LostArk::Server::KOUKUSAYDON_LOGIC_ANSWER
LostArk::Server::CKoukuSaydonLogicRuntime::Judge_Roulette(
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	if (!window.CardRegions.empty())
	{
		const BOSS_LOGIC_REGION* matched = nullptr;
		for (const auto& region : window.CardRegions)
			if (Contains_LogicRegion(region, boss, player))
			{
				if (nullptr != matched) return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
				matched = &region;
			}
		if (nullptr == matched) return KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT;
		return matched->eCardSymbol == player.eMechanicCardSymbol &&
			matched->eCardColor == player.eMechanicCardColor ?
			KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	}
	if (window.iSectorCount == 0u ||
		window.SectorSymbols.size() != window.iSectorCount ||
		MECHANIC_CARD_SYMBOL::NONE == player.eMechanicCardSymbol)
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float dx = player.fPositionX - window.fCenterX;
	const float dz = player.fPositionZ - window.fCenterZ;
	if (!std::isfinite(dx) || !std::isfinite(dz))
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	if (window.fOuterRadiusM > 0.f &&
		dx * dx + dz * dz > window.fOuterRadiusM * window.fOuterRadiusM)
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float degrees = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
	const float relative = Wrap360(degrees - window.fStopYawDegrees);
	const float sectorSpan = 360.f / static_cast<float>(window.iSectorCount);
	std::size_t sector = static_cast<std::size_t>(relative / sectorSpan);
	if (sector >= window.iSectorCount)
		sector = window.iSectorCount - 1u;
	return window.SectorSymbols[sector] == player.eMechanicCardSymbol ?
		KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
}

LostArk::Server::KOUKUSAYDON_LOGIC_ANSWER
LostArk::Server::CKoukuSaydonLogicRuntime::Judge_Gaze(
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_PLAYER& player) noexcept
{
	const float dx = boss.fPositionX - player.fPositionX;
	const float dz = boss.fPositionZ - player.fPositionZ;
	if (!std::isfinite(dx) || !std::isfinite(dz) || !std::isfinite(player.fYawDegrees))
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const auto insideAnswer = window.bInsideIsFail ?
		KOUKUSAYDON_LOGIC_ANSWER::FAIL : KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
	const auto outsideAnswer = window.bInsideIsFail ?
		KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float distanceSq = dx * dx + dz * dz;
	// Standing on the boss counts as facing it: no direction to judge.
	if (distanceSq < 0.01f)
		return insideAnswer;
	if (window.fMaxDistanceM > 0.f &&
		distanceSq > window.fMaxDistanceM * window.fMaxDistanceM)
		return outsideAnswer;
	const float toBoss = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
	const float difference = Wrap180(toBoss - player.fYawDegrees);
	return std::fabs(difference) <= window.fHalfAngleDegrees ? insideAnswer : outsideAnswer;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_ShieldReflected(
	const SERVER_WORLD_ENTITY& boss,
	const float sourceX,
	const float sourceZ) noexcept
{
	for (const auto& retained : boss.KoukuRetainedLogicOwners)
		if (const auto owner = retained.lock(); owner && Is_ShieldReflected(*owner, sourceX, sourceZ))
			return true;
	if (!boss.bKoukuShieldActive || boss.fKoukuShieldArcDegrees <= 0.f)
		return false;
	const auto reflects = [sourceX, sourceZ](const float centerX, const float centerZ,
		const float yaw, const float halfAngle, const float radiusX = 1.f, const float radiusZ = 1.f)
	{
		const float dx = sourceX - centerX, dz = sourceZ - centerZ;
		if (!std::isfinite(dx) || !std::isfinite(dz) || dx * dx + dz * dz < 0.0001f)
			return false;
		const float radians = yaw * .017453292519943295f;
		const float x = (std::cos(radians) * dx - std::sin(radians) * dz) / radiusX;
		const float z = (std::sin(radians) * dx + std::cos(radians) * dz) / radiusZ;
		return std::fabs(static_cast<float>(std::atan2(x, z) * DEGREES_PER_RADIAN)) <= halfAngle;
	};
	if (!boss.KoukuShieldRegions.empty())
	{
		const float radians = boss.fYawDegrees * 0.017453292519943295f;
		for (const auto& region : boss.KoukuShieldRegions)
		{
			// A ranged attacker is reflected by direction too; the sector radius
			// is its debug drawing extent, not an attack-distance restriction.
			if (!region.bSector || region.bReverseSector || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT)
				continue;
			const float centerX = boss.fPositionX + std::cos(radians) * region.fCenterX + std::sin(radians) * region.fCenterZ;
			const float centerZ = boss.fPositionZ - std::sin(radians) * region.fCenterX + std::cos(radians) * region.fCenterZ;
			if (reflects(centerX, centerZ, boss.fYawDegrees + region.fYawDegrees, region.fHalfAngleDegrees,
				region.fRadiusXM > 0.f ? region.fRadiusXM : region.fRadiusM,
				region.fRadiusZM > 0.f ? region.fRadiusZM : region.fRadiusM))
				return true;
		}
		return false;
	}
	return reflects(boss.fPositionX, boss.fPositionZ,
		boss.fYawDegrees + boss.fKoukuShieldNormalYawOffsetDegrees, boss.fKoukuShieldArcDegrees * 0.5f);
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Transform_ToClown(
	SERVER_PLAYER& player,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	const std::uint32_t durationMs)
{
	using namespace LostArk::Shared;
	if (player.Is_Guide() || 0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction)
		return;
	const std::uint32_t holdMs = 0u != durationMs ? durationMs :
		(nullptr != pMadnessPolicy && 0u != pMadnessPolicy->iClownHoldMs ?
			pMadnessPolicy->iClownHoldMs :
			static_cast<std::uint32_t>(DEFAULT_CLOWN_HOLD_MS));
	player.bKoukuPatternOwnsClown = false;
	player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
	player.iMadnessFormEndTick = Add_Ticks(serverTick, Ticks_FromMs(holdMs));
	player.iCurrentMadness = 0u; player.dMadnessRemainder = 0.;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Update_PlayerFear(
    SERVER_PLAYER& player, const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    if (PLAYER_ACTION_STATE::FEAR == player.eAction &&
        (Has_ReachedTick(serverTick, player.iFearEndTick) || player.iCurrentHp == 0u))
    {
        player.eAction = player.iCurrentHp == 0u ? PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
        player.iActionStartTick = 0u;
        player.fActionElapsedSeconds = 0.f;
        player.PendingCommand.Clear();
    }
    if (PLAYER_ACTION_STATE::FEAR != player.eAction)
    {
        player.iFearEndTick = 0u;
        player.strFearPresentationId.clear();
        return false;
    }
    return true;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Apply_Result(
	SERVER_PLAYER& player,
	const BOSS_PATTERN_LOGIC_RESULT& result,
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	const std::array<float, 2u>* const pContactCenter, const bool encounterWipe)
{
	using namespace LostArk::Shared;
	switch (result.eKind)
	{
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY:
	{
		if (!player.iCurrentHp || !result.iDurationMs) break;
		const auto until = Add_Ticks(serverTick, Ticks_FromMs(result.iDurationMs));
		if (!player.iInvulnerableEndTick || Has_ReachedTick(until, player.iInvulnerableEndTick)) player.iInvulnerableEndTick = until;
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE:
	{
		if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE && !result.iDamageAmount) break;
		SERVER_WORLD_TO_PLAYER_HIT hit{};
		hit.iRawDamage =
			BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH == result.eKind ?
				(std::max)(1u, player.iCurrentHp) :
			BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE == result.eKind ? result.iDamageAmount :
				(std::max)(1u, Percent_Of(player.iMaximumHp, result.iPercent));
		hit.fSourceX = boss.fPositionX;
		hit.fSourceZ = boss.fPositionZ;
		hit.fPushRangeM = result.fPushRangeM;
		hit.iPushMs = result.iPushMs;
		hit.bForcePush = result.bForcePush;
		hit.bPushCanLeaveArena = result.bPushCanLeaveArena;
		hit.bPushBallistic = result.bPushBallistic;
		hit.fPushHeightM = result.fPushHeightM;
		if (result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT)
		{
			if (!pContactCenter) break;
			hit.fSourceX = (*pContactCenter)[0];
			hit.fSourceZ = (*pContactCenter)[1];
		}
		if (result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD)
		{
			hit.bUsePushDirection = true;
			const float radians = (boss.fYawDegrees + result.fPushYawOffsetDegrees) * 0.017453292519943295f;
			hit.fPushDirectionX = std::sin(radians);
			hit.fPushDirectionZ = std::cos(radians);
		}
		hit.iServerTick = serverTick;
		hit.bIgnoreDefense = true;
		hit.bIgnoreCounter = true;
		hit.bInstantDeath = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
			(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
		hit.bEncounterWipe = encounterWipe && hit.bInstantDeath;
		if (pContactCenter && Is_CurrentBossHandCapture(player, boss.iNetEntityId, boss.iPatternSequence, serverTick))
		{
			hit.iCaptureOwnerId = boss.iNetEntityId;
			hit.iCapturePatternSequence = boss.iPatternSequence;
		}
		(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
			player, hit, catalog, outDamageEvents);
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT:
	{
		if (player.Is_Guide() || 0u == player.iMaximumMadness)
			break;
		const std::uint32_t gain = Percent_Of(player.iMaximumMadness, result.iPercent);
		const std::uint64_t total =
			static_cast<std::uint64_t>(player.iCurrentMadness) + gain;
		player.iCurrentMadness = static_cast<std::uint32_t>(
			(std::min<std::uint64_t>)(total, player.iMaximumMadness));
		if (player.iCurrentMadness >= player.iMaximumMadness)
			Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR:
        if (!player.Has_TimeStop(serverTick) && !player.Has_HolyCharmProtection(serverTick) &&
            Can_ReceiveSpatialContact(player) && !player.bPatternBound &&
            PLAYER_ACTION_STATE::GRABBED != player.eAction &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
			PLAYER_ACTION_STATE::WALL_CLIMB != player.eAction &&
            PLAYER_ACTION_STATE::FEAR != player.eAction)
        {
            player.eAction = PLAYER_ACTION_STATE::FEAR;
            player.iActionStartTick = serverTick == 0u ? 1u : serverTick;
            player.iFearEndTick = Add_Ticks(player.iActionStartTick, Ticks_FromMs(result.iDurationMs));
            player.strFearPresentationId = result.strFearPresentationId;
            player.iCurrentSkillId = INVALID_SKILL_ID;
            player.Clear_SkillTarget();
            player.iComboStage = 0u;
            player.hasBufferedComboInput = false;
            player.hasReleasedHold = false;
            player.PendingCommand.Clear();
            player.hasMoveGoal = false;
            player.MovePath.clear();
            player.iMovePathIndex = 0u;
            player.fActionElapsedSeconds = 0.f;
            player.iKnockdownEndTick = 0u;
            player.fKnockbackRemainingSeconds = 0.f;
        }
        break;
	case BOSS_PATTERN_LOGIC_RESULT_KIND::CLOWN_TRANSFORM:
		Transform_ToClown(player, pMadnessPolicy, serverTick, result.iDurationMs);
		break;
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW:
	/* The grab needs the exact region that caught this player, which only the
	ENTER_AREA seam in Update knows, so it is applied there. */
	case BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::NONE:
	default:
		break;
	}
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Can_EnterMarioEntry(
	const SERVER_PLAYER& player) noexcept
{
	return Is_Judgeable(player) && player.eMadnessForm == LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN &&
		!player.iMarioStage && !player.TriggerMove.isActive &&
		!player.bPatternBound && !player.bArenaEjectionActive &&
		player.iAttachmentOwnerNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID;
}

const LostArk::Server::BOSS_PATTERN_LOGIC_RESULT* LostArk::Server::CKoukuSaydonLogicRuntime::Find_MarioEntryResult(
	const BOSS_PATTERN_DEFINITION& pattern) noexcept
{
	for (const auto& window : pattern.LogicWindows)
		if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.CardRegions.empty() &&
			window.OnSuccess.size() == 1u && window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER)
			return &window.OnSuccess.front();
	return nullptr;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Has_MarioEntry(
	const BOSS_PATTERN_DEFINITION& pattern) noexcept
{
	return Find_MarioEntryResult(pattern) != nullptr ||
		std::any_of(pattern.LogicWindows.begin(), pattern.LogicWindows.end(),
			[](const auto& window) { return window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT; });
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(
	const BOSS_PATTERN_DEFINITION& root, const SERVER_WORLD_ENTITY& anchor,
	const SERVER_PLAYER& player, const std::uint32_t elapsedTicks, const bool honorWindowEnd, std::uint8_t* const matchedStage)
{
	using namespace LostArk::Shared::WorldCollision;
	if (!Can_EnterMarioEntry(player)) return false;
	// Movement stops one contact margin short of a blocking body. Admit the
	// player's body at that same boundary rather than waiting for its centre.
	const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ body{
		player.fPositionX, player.fPositionZ, PLAYER_HALF_EXTENT_X + CONTACT_MARGIN };
	for (const auto& window : root.LogicWindows)
	{
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || elapsedTicks < Ticks_FromMs(window.iStartMs) ||
			(honorWindowEnd && elapsedTicks >= Ticks_FromMs(window.iStartMs + window.iDurationMs)) ||
			window.OnSuccess.size() != 1u || window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER) continue;
		for (const auto& region : window.CardRegions)
		{
			// Mario admission outlives the pattern ledger; the room already owns
			// its frozen birth root in MarioEntryAnchor and passes it as anchor.
			auto sampled = region;
			if (sampled.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_START)
				sampled.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			LOGIC_REGION_TRANSFORM transform;
			if (!Resolve_LogicRegionTransform(sampled, anchor, elapsedTicks, transform) ||
				((transform.hasGrip || sampled.bCylinder || sampled.bLinearMotion) &&
				 std::abs(player.fPositionY + PLAYER_CENTER_OFFSET_Y - transform.centerY) >
					transform.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN)) continue;
			if (Intersects_LogicRegion(sampled, anchor, body, elapsedTicks))
			{
				if (matchedStage) *matchedStage = window.OnSuccess.front().iMarioEntryStage;
				return true;
			}
		}
	}
	return false;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Apply_Results(
	const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	SERVER_PLAYER* const pPlayer,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
	const std::set<LostArk::Shared::PLAYER_ID>& invulnerablePlayers,
	const std::array<float, 2u>* const pContactCenter,
	const bool spatialContact)
{
	const auto isDamage = [](const BOSS_PATTERN_LOGIC_RESULT& result) {
		return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
			result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
			result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE;
	};
	const bool hasDamage = std::any_of(results.begin(), results.end(), isDamage);
	const bool hasAuthoredMadness = std::any_of(results.begin(), results.end(), [](const auto& result) {
		return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
	});
	const bool independentMadness = spatialContact && pContactCenter && state.iMadnessSource != 0u;
	std::set<LostArk::Shared::PLAYER_ID> damagedPlayers;
	// Ordinary explicit gauge follows landed HP damage. Ball/doll contact owns its
	// gauge independently of damage mitigation, shields and damage-blocking effects.
	// Explicit amounts still replace automatic HP-proportional gain for this verdict.
	std::vector<const BOSS_PATTERN_LOGIC_RESULT*> ordered;
	ordered.reserve(results.size());
	for (const auto& result : results)
		if (!hasDamage || result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT) ordered.push_back(&result);
	if (hasDamage) for (const auto& result : results)
		if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT) ordered.push_back(&result);
	for (const auto* resultPointer : ordered)
	{
		const auto& result = *resultPointer;
		// A companion may touch a hazard but cannot answer or complete a raid mechanic.
		if (pPlayer && pPlayer->Is_Guide() && (!spatialContact ||
			(!isDamage(result) && result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR &&
			 result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER &&
			 result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT))) continue;
        if (BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER == result.eKind)
        {
            if (pPlayer && Can_ReceiveSpatialContact(*pPlayer) && pPlayer->eAction != LostArk::Shared::PLAYER_ACTION_STATE::FEAR && state.iHoldEndTick &&
                Has_ReachedTick(serverTick, state.iHoldStartTick) && !Has_ReachedTick(serverTick, state.iHoldEndTick) &&
                std::none_of(outOutput.CaptureRequests.begin(), outOutput.CaptureRequests.end(), [&](const auto& request) {
                    return request.iPlayerNetEntityId == pPlayer->iNetEntityId;
                }))
                outOutput.CaptureRequests.push_back({pPlayer->iNetEntityId, result.eAttachmentSlot, state.iHoldEndTick, state.iWindowIndex});
            continue;
        }
		if (BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind)
		{
			if (state.AppliedWorldMotions.emplace(result.strTargetWorldInstanceId, result.strMotionInstanceId).second)
			{
				KOUKUSAYDON_LOGIC_WORLD_PLAY play;
				play.strInstanceId = result.strMotionInstanceId;
				play.strTargetSequenceInstanceId = result.strTargetWorldInstanceId;
				play.iStartTick = serverTick;
				outOutput.WorldSequencePlays.push_back(std::move(play));
			}
			continue;
		}
		if (BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind)
		{
			if (!result.strPatternId.empty())
				outOutput.FollowupPatternIds.push_back(result.strPatternId);
			continue;
		}
		const bool lethal = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
			(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
		const auto apply = [&](SERVER_PLAYER& player) {
			// A solved same-pattern safe zone cancels its verdict before generic lethal-hit bypasses.
			if (invulnerablePlayers.contains(player.iPlayerId) &&
				(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
				 result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
				 result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE ||
				 result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR)) return;
			if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT &&
				((independentMadness && !Can_ReceiveSpatialContact(player)) ||
				 (hasDamage && ((!independentMadness && !damagedPlayers.contains(player.iPlayerId)) ||
				  !player.iCurrentHp || player.eMadnessForm != LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL)))) return;
			const auto automaticGain = player.iMadnessDamageGainPercent;
			const auto hpBefore = player.iCurrentHp;
			const auto firstDamageEvent = outDamageEvents.size();
			if (hasAuthoredMadness && isDamage(result)) player.iMadnessDamageGainPercent = 0u;
			if (state.iMadnessSource && pMadnessPolicy && pMadnessPolicy->iSpecialIntervalMs &&
				result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT)
			{
				const bool ball = state.iMadnessSource == 1u;
				const double gain = static_cast<double>(player.iMaximumMadness) *
					(ball ? pMadnessPolicy->iBallGainPercent : pMadnessPolicy->iDollGainPercent) / 100. *
					(ball ? pMadnessPolicy->iBallMultiplierPercent : pMadnessPolicy->iDollMultiplierPercent) / 100. *
					state.iMadnessContactIntervalMs / pMadnessPolicy->iSpecialIntervalMs;
				CServerCombatHitRuntime::Add_MadnessGauge(player, gain);
				if (player.iMaximumMadness && player.iCurrentMadness >= player.iMaximumMadness)
					Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
			}
			else Apply_Result(player, result, boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, pContactCenter,
				!spatialContact && lethal);
			player.iMadnessDamageGainPercent = automaticGain;
			// Special gauge-only contact has no combat-text occurrence when shielded.
			// Preserve actual HP damage and every event from earlier results/targets.
			if (independentMadness && isDamage(result))
				outDamageEvents.erase(std::remove_if(outDamageEvents.begin() + firstDamageEvent, outDamageEvents.end(),
					[&](const auto& event) { return !event.isOutgoing && event.iTargetNetEntityId == player.iNetEntityId &&
						event.eHitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB; }), outDamageEvents.end());
			if (isDamage(result) && player.iCurrentHp < hpBefore) damagedPlayers.insert(player.iPlayerId);
		};
		if (nullptr != pPlayer)
		{
			apply(*pPlayer);
			continue;
		}
		/* Boss-level windows punish or reward the whole living raid. */
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (Is_Judgeable(player) || (!spatialContact && player.Is_Human() && player.iCurrentHp && lethal))
				apply(player);
		}
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Update(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision)
{
	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
		return;
	// Parent deadlines cancel a child before contact, charge, or end-tick judgement.
	for (auto& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size()) continue;
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if ((!window.strOwnerWorldOccurrenceId.empty() && ledger.RetiredWorldOccurrences.contains(window.strOwnerWorldOccurrenceId)) ||
			(window.bCancelAtEnd && Has_ReachedTick(serverTick, state.iEndTick)))
		{
			state.bChargeStopped = true;
			Close_Window(boss, window, state, players);
		}
	}
	/* Anyone already hanging rides the same authored region that caught them.
	The region is catalog data, so this keeps working after the window that
	judged them closed, and the moment the World Object's track hides the hook
	the deadline collapses to now and the room releases them. */
	for (auto& [hangingPlayerId, hanging] : players)
	{
		(void)hangingPlayerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != hanging.eAction ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP != hanging.eAttachmentSlot ||
			hanging.iAttachmentOwnerNetEntityId != boss.iNetEntityId ||
			hanging.iAttachmentPatternSequence != boss.iPatternSequence ||
			hanging.iAttachmentWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& carrier = pattern.LogicWindows[hanging.iAttachmentWindowIndex];
		if (hanging.iAttachmentRegionIndex >= carrier.CardRegions.size())
			continue;
		if (!Drag_HookedPlayer(hanging, carrier.CardRegions[hanging.iAttachmentRegionIndex],
			boss, serverTick - ledger.iPatternStartTick, serverTick, navigation))
			hanging.iAttachmentReleaseTick = serverTick;
	}
	for (KOUKUSAYDON_LOGIC_CUE_STATE& cue : ledger.MechanicTriggers)
	{
		if (cue.bStarted || cue.iIndex >= pattern.MechanicTriggers.size() ||
			!Has_ReachedTick(serverTick, cue.iStartTick))
			continue;
		cue.bStarted = true;
		const auto& trigger = pattern.MechanicTriggers[cue.iIndex];
		if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER == trigger.eKind)
			ledger.eHudMode = trigger.eHudMode;
		else if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS == trigger.eKind)
			(void)Stage_CardMazeEntryPlayers(trigger, ledger, players, navigation, collision, outOutput.strStatus);
		else if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_HIDE_NEXT == trigger.eKind)
		{
			Capture_CardMazeEntryRoster(ledger, players);
			if (ledger.iCardMazeNextHiddenPlayer < ledger.CardMazeEntryPlayers.size())
			{
				const auto found = players.find(ledger.CardMazeEntryPlayers[ledger.iCardMazeNextHiddenPlayer++]);
				if (found != players.end() && Is_Judgeable(found->second))
					found->second.CardMaze.flags |= LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN;
			}
		}
		else if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_ENTER == trigger.eKind)
			(void)Enter_CardMaze(trigger, ledger, players, navigation, collision, outOutput.strStatus);
		else
		{
			auto scheduled = trigger;
			if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES &&
				Has_ReachedTick(serverTick, Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs))))
				continue;
			if (pattern.bFixedTimelineClock &&
				(trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT ||
				 trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS))
			{
				const auto deadline = Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
				if (Has_ReachedTick(serverTick, deadline)) continue;
				const auto remainingTicks = deadline >= serverTick ? deadline - serverTick :
					(std::numeric_limits<std::uint32_t>::max)() - serverTick + deadline;
				// Clone expiry is relative to actual spawn; preserve the absolute Parent deadline.
				scheduled.iDurationMs = static_cast<std::uint32_t>(static_cast<std::uint64_t>(remainingTicks) * 1000u / SERVER_TICK_HZ);
			}
			outOutput.MechanicTriggers.push_back(std::move(scheduled));
		}
	}
	for (KOUKUSAYDON_LOGIC_CUE_STATE& cue : ledger.WorldSequences)
	{
		if (cue.bStarted || cue.iIndex >= pattern.WorldSequences.size() ||
			!Has_ReachedTick(serverTick, cue.iStartTick))
			continue;
		cue.bStarted = true;
		const BOSS_PATTERN_WORLD_SEQUENCE& sequence = pattern.WorldSequences[cue.iIndex];
		KOUKUSAYDON_LOGIC_WORLD_PLAY play;
		play.strInstanceId = sequence.strInstanceId; play.fPlaybackSpeed = sequence.fPlaybackSpeed;
		play.iDurationMs = sequence.iDurationMs; play.strOccurrenceId = sequence.strOccurrenceId; play.iStartTick = cue.iStartTick;
		play.Placement = sequence.Placement;
        const auto positionOffset = ledger.WorldPositionOffsets.find(sequence.strOccurrenceId);
        if (play.Placement && positionOffset != ledger.WorldPositionOffsets.end())
        {
            // packet에 실리는 Placement도 접촉 판정과 같은 이번 실행의 이동값을 사용한다.
            play.Placement->fPositionX += positionOffset->second[0u];
            play.Placement->fPositionY += positionOffset->second[1u];
            play.Placement->fPositionZ += positionOffset->second[2u];
        }
		play.CombatBody = sequence.CombatBody;
		play.bAuthoredMadness = sequence.bAuthoredMadness;
		if (!play.Placement)
		{
			play.fPositionOffsetX = sequence.fPositionOffsetX + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionX - sequence.fAnchorPositionX : 0.f);
			play.fPositionOffsetY = sequence.fPositionOffsetY + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionY - sequence.fAnchorPositionY : 0.f);
			play.fPositionOffsetZ = sequence.fPositionOffsetZ + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionZ - sequence.fAnchorPositionZ : 0.f);
		}
		outOutput.WorldSequencePlays.push_back(std::move(play));
	}
    // Capture a charge once at trigger entry, then sample the existing boss-motion
    // path before the body-follow contact regions inspect this tick's position.
    for (auto& state : ledger.Windows)
    {
        if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
            !Has_ReachedTick(serverTick, state.iStartTick)) continue;
        const auto& window = pattern.LogicWindows[state.iWindowIndex];
        if (window.fBossChargeDistanceM <= 0.f || state.bChargeStopped) continue;
        if (!state.bChargeCaptured)
        {
            state.bChargeCaptured = true;
            std::vector<const SERVER_PLAYER*> candidates;
            for (const auto& [id, player] : players)
                if (Is_Judgeable(player)) candidates.push_back(&player);
            if (candidates.empty())
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge has no living target"; continue; }
            const auto* target = candidates[Mix(static_cast<std::uint64_t>(boss.iPatternSequence) << 32u ^
                boss.iNetEntityId ^ state.iStartTick) % candidates.size()];
            const float dx = target->fPositionX - boss.fPositionX, dz = target->fPositionZ - boss.fPositionZ;
            const float length = std::sqrt(dx * dx + dz * dz);
            if (!std::isfinite(length) || length <= .0001f)
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge target has no direction"; continue; }
            BOSS_PATTERN_BOSS_MOTION motion;
            motion.iStartMs = window.iStartMs; motion.iEndMs = window.iStartMs + window.iDurationMs;
            motion.StartPosition = {boss.fPositionX, boss.fPositionY, boss.fPositionZ};
            motion.EndPosition = {boss.fPositionX + dx / length * window.fBossChargeDistanceM,
                boss.fPositionY, boss.fPositionZ + dz / length * window.fBossChargeDistanceM};
            // The authored clip may face a different native axis; its body yaw
            // must not rotate the captured world-space travel vector.
            motion.fYawDegrees = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN) + window.fChargeYawOffsetDegrees;
            state.ChargeMotion = motion;
            boss.iTargetEntityId = target->iNetEntityId;
            boss.iPatternTargetEntityId = target->iNetEntityId;
            boss.bHasPatternTargetLastPosition = true;
            boss.fPatternTargetLastPositionX = target->fPositionX;
            boss.fPatternTargetLastPositionY = target->fPositionY;
            boss.fPatternTargetLastPositionZ = target->fPositionZ;
        }
        if (!state.ChargeMotion) continue;
        const auto& motion = *state.ChargeMotion;
        const auto position = CKoukuSaydonBrain::Sample_BossMotion(motion, serverTick - ledger.iPatternStartTick);
        SERVER_NAV_POINT destination{position[0], position[1], position[2]};
        bool navigationBlocked = false;
        if (navigation && (!navigation->Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
            !navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, destination.x, destination.z, destination)))
        {
            // Endpoint heights alone can skip a blocked cell or clip its corner.
            // Stop at the same last navigable point used by native root-motion recoil.
            const double dx = double(destination.x) - boss.fPositionX, dz = double(destination.z) - boss.fPositionZ;
            const double length = std::hypot(dx, dz);
            double low = 0., high = 1.;
            bool found = false;
            SERVER_NAV_POINT clamped{};
            for (unsigned iteration = 0u; iteration < 40u && (high - low) * length > .001; ++iteration)
            {
                const double middle = (low + high) * .5;
                const float x = static_cast<float>(boss.fPositionX + dx * middle);
                const float z = static_cast<float>(boss.fPositionZ + dz * middle);
                SERVER_NAV_POINT candidate{};
                if (navigation->Has_LineOfSight(boss.fPositionX, boss.fPositionZ, x, z) &&
                    navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, x, z, candidate))
                { low = middle; clamped = candidate; found = true; }
                else high = middle;
            }
            if (!found || low * length < .001)
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge stopped at navigation boundary"; continue; }
            destination = clamped;
            navigationBlocked = true;
        }
        bool blocked = false;
        if (collision && !collision->Resolve_CircleMove(boss.fPositionX, boss.fPositionY, boss.fPositionZ,
            destination.x, destination.y, destination.z, boss.fCollisionRadius, boss.fCollisionRadius,
            boss.fCollisionRadius, destination.x, destination.y, destination.z, blocked, boss.iNetEntityId, false))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge collision resolution failed"; continue; }
        if (navigation && (!navigation->Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
            !navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, destination.x, destination.z, destination)))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge collision destination is not navigable"; continue; }
        boss.fPositionX = destination.x; boss.fPositionY = destination.y; boss.fPositionZ = destination.z;
        boss.fYawDegrees = motion.fYawDegrees;
        state.bChargeStopped = navigationBlocked || blocked || Has_ReachedTick(serverTick, state.iEndTick);
        if (navigationBlocked) outOutput.strStatus = "boss charge stopped at navigation boundary";
        else if (blocked) outOutput.strStatus = "boss charge stopped at collision boundary";
    }
	// Fixed Effect anchors can be born before their colliders. Capture on the
	// same Server clock before the later window opens, retaining one immutable basis.
	for (auto& state : ledger.Windows)
		if (!state.bClosed && state.iWindowIndex < pattern.LogicWindows.size())
			Capture_StartRegions(boss, pattern.LogicWindows[state.iWindowIndex], state,
				serverTick - ledger.iPatternStartTick);
	// Gather protection before any result, independently of authored window order.
	// Protection belongs to this pattern execution; only presentation pulses form a room-wide union.
	std::set<LostArk::Shared::PLAYER_ID> invulnerablePlayers;
	for (const auto& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
			!Has_ReachedTick(serverTick, state.iStartTick) ||
			(Has_ReachedTick(serverTick, state.iEndTick) && serverTick != state.iEndTick)) continue;
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE || window.iThreshold > 4u) continue;
		for (const auto& region : window.CardRegions)
		{
			std::vector<LostArk::Shared::PLAYER_ID> occupants;
			for (const auto& [playerId, player] : players)
				if (Is_Judgeable(player) && Contains_LogicRegion(Window_Region(region, state), boss, player,
					serverTick - ledger.iPatternStartTick)) occupants.push_back(playerId);
			// Every Collider owns its exact count; overlapping valid regions form a union.
			if (window.iThreshold == 0u || occupants.size() == window.iThreshold)
				invulnerablePlayers.insert(occupants.begin(), occupants.end());
		}
	}
	for (const auto playerId : invulnerablePlayers)
	{
		auto& player = players.at(playerId);
		const bool continuous = player.iInvulnerabilityZoneContactTick != 0u &&
			(player.iInvulnerabilityZoneContactTick == serverTick ||
			 Add_Ticks(player.iInvulnerabilityZoneContactTick, 1u) == serverTick);
		if (!continuous || player.iInvulnerabilityZonePulseTick == 0u ||
			Has_ReachedTick(serverTick, Add_Ticks(player.iInvulnerabilityZonePulseTick, SERVER_TICK_HZ * 2u)))
			player.iInvulnerabilityZonePulseTick = serverTick;
		player.iInvulnerabilityZoneContactTick = serverTick;
	}
	// Resolve all contacts before any duration timeout, including the final tick.
	// A group's priority order is fixed at Build; one card consumes one result per strike.
	std::set<std::string> completedWindows;
	for (const auto index : ledger.ContactWindowOrder)
	{
		auto& state = ledger.Windows[index];
		const auto& window = pattern.LogicWindows[index];
		if (state.bClosed || !Has_ReachedTick(serverTick, state.iStartTick)) continue;
		if (!state.bOpened) Open_Window(boss, window, state, ledger, players);
		const bool reachedEnd = Has_ReachedTick(serverTick, state.iEndTick);
		if (!reachedEnd || serverTick == state.iEndTick)
			for (const auto& target : window.ContactTargets)
			{
				if (state.ContactedWorldOccurrences.contains(target.strWorldOccurrenceId)) continue;
				const auto motionPriority = ledger.AppliedContactMotionPriorities.find({window.strContactGroupId, target.strWorldOccurrenceId});
				if (!window.strContactGroupId.empty() && motionPriority != ledger.AppliedContactMotionPriorities.end() &&
					motionPriority->second > window.iContactPriority) continue;
                const auto positionOffset = ledger.WorldPositionOffsets.find(target.strWorldOccurrenceId);
                const float offsetX = positionOffset == ledger.WorldPositionOffsets.end() ? 0.f : positionOffset->second[0u];
                const float offsetZ = positionOffset == ledger.WorldPositionOffsets.end() ? 0.f : positionOffset->second[2u];
                // 저작 target 중심에 구워진 template local offset을 유지한 채 카드와 같은 평행이동을 적용한다.
                const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ circle{
                    target.fWorldX + offsetX, target.fWorldZ + offsetZ, target.fRadiusM};
				if (!std::any_of(window.CardRegions.begin(), window.CardRegions.end(), [&](const auto& region) {
					return Intersects_LogicRegion(Window_Region(region, state), boss, circle, serverTick - ledger.iPatternStartTick);
				})) continue;
				state.ContactedWorldOccurrences.insert(target.strWorldOccurrenceId);
				if (!window.strContactGroupId.empty() && !ledger.ConsumedContactGroups.emplace(
					window.strContactGroupId, state.iStartTick, target.strWorldOccurrenceId).second) continue;
				for (const auto& result : window.OnSuccess)
				{
					if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION)
					{
						const auto motion = std::find_if(result.ContactMotions.begin(), result.ContactMotions.end(),
							[&](const auto& row) { return row.strTargetWorldOccurrenceId == target.strWorldOccurrenceId; });
						if (motion != result.ContactMotions.end() && state.AppliedWorldMotions.emplace(
							target.strWorldOccurrenceId, motion->strMotionInstanceId).second)
						{
							KOUKUSAYDON_LOGIC_WORLD_PLAY play;
							play.strInstanceId = motion->strMotionInstanceId; play.strTargetSequenceInstanceId = target.strWorldInstanceId;
							play.strTargetWorldOccurrenceId = target.strWorldOccurrenceId; play.iStartTick = serverTick;
							outOutput.WorldSequencePlays.push_back(std::move(play));
							if (!window.strContactGroupId.empty())
								ledger.AppliedContactMotionPriorities[{window.strContactGroupId, target.strWorldOccurrenceId}] = window.iContactPriority;
						}
					}
					else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW &&
						(result.strContactTargetWorldOccurrenceId.empty() || result.strContactTargetWorldOccurrenceId == target.strWorldOccurrenceId))
						completedWindows.insert(result.strTargetLogicOccurrenceId);
				}
			}
		if (reachedEnd) Close_Window(boss, window, state, players);
	}
	for (auto& state : ledger.Windows)
	{
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (state.bClosed || window.eKind != BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL ||
			!completedWindows.contains(window.strWindowId) || !Has_ReachedTick(serverTick, state.iStartTick) ||
			(Has_ReachedTick(serverTick, state.iEndTick) && serverTick != state.iEndTick)) continue;
		if (!state.bOpened) Open_Window(boss, window, state, ledger, players);
		Close_Window(boss, window, state, players);
		Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog, pMadnessPolicy,
			serverTick, outDamageEvents, outOutput, invulnerablePlayers);
		outOutput.bEndPatternEarly |= window.bEndsPatternOnSuccess;
		outOutput.strStatus = "external signal window succeeded";
	}
	for (KOUKUSAYDON_LOGIC_WINDOW_STATE& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[state.iWindowIndex];
		if (!state.bOpened)
		{
			if (!Has_ReachedTick(serverTick, state.iStartTick))
				continue;
			Open_Window(boss, window, state, ledger, players);
		}
		const bool reachedEnd = Has_ReachedTick(serverTick, state.iEndTick);
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES:
			if (ledger.iBingoLineJudgementTick && Has_ReachedTick(ledger.iBingoLineJudgementTick, state.iStartTick) &&
				!Has_ReachedTick(ledger.iBingoLineJudgementTick, state.iEndTick) && Has_ReachedTick(serverTick, ledger.iBingoLineJudgementTick))
			{
				const bool passed = ledger.iBingoCompletedLines >= window.iThreshold;
				auto results = passed ? window.OnSuccess : window.OnFail;
				if (passed && ledger.bBingoLineRewardApplied)
					std::erase_if(results, [](const auto& result) {
						return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY; });
				for (auto& [id, player] : players)
					if (player.iCurrentHp && player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::DEAD)
					{
						Apply_Results(results, state, &player, players, boss, catalog, pMadnessPolicy,
							serverTick, outDamageEvents, outOutput, invulnerablePlayers);
						if (passed) for (const auto& result : results)
							if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY && result.iDurationMs)
							{
								const auto until = Add_Ticks(serverTick, Ticks_FromMs(result.iDurationMs));
								if (!player.iKoukuBingoLineProtectionEndTick || Has_ReachedTick(until, player.iKoukuBingoLineProtectionEndTick))
									player.iKoukuBingoLineProtectionEndTick = until;
							}
					}
				outOutput.BingoLineCompletion = passed;
				Close_Window(boss, window, state, players);
			}
			else if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		case BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION:
		case BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE:
			if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
			break; // Contact phase above owns the complete short window.
		case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
            if (reachedEnd) Close_Window(boss, window, state, players);
            break;
		case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
			if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog, pMadnessPolicy,
					serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "external signal window timed out";
			}
			break;
		case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
			if (reachedEnd) Close_Window(boss, window, state, players);
			else boss.iTargetEntityId = boss.iPatternTargetEntityId = state.iFreePlayerNetEntityId;
			break;
        case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
        {
            auto& signals = boss.BossCombat.PendingOutcomes;
            const auto counter = std::find_if(signals.begin(), signals.end(), [&](const auto& signal) {
                return signal.iPatternSequence == ledger.iPatternSequence &&
                    signal.strPatternId == boss.strPatternId &&
                    signal.eOutcome == BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT &&
                    Has_ReachedTick(signal.iServerTick, state.iStartTick) &&
                    !Has_ReachedTick(signal.iServerTick, Add_Ticks(state.iEndTick, 1u));
            });
            if (counter != signals.end())
            {
                float landingY = boss.fPositionY;
                if (window.bEndsPatternOnSuccess && !Try_CounterLanding(boss, navigation, collision, landingY))
                {
                    // The hit was already applied once by combat. Keep only its signal
                    // pending; retrying the landing must never reapply damage or stagger.
                    if (reachedEnd)
                    {
                        signals.erase(counter);
                        Close_Window(boss, window, state, players);
                        Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
                            pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                    }
                    outOutput.strStatus = reachedEnd ? "counter window timed out: landing rejected by navigation or collision" :
                        "counter landing rejected by navigation or collision";
                    break;
                }
                if (window.bEndsPatternOnSuccess)
                {
                    boss.fPositionY = landingY;
                    outOutput.bCounterSuccessLanded = true;
                }
                signals.erase(counter);
                Close_Window(boss, window, state, players);
                Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog,
                    pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                if (window.bEndsPatternOnSuccess) outOutput.bEndPatternEarly = true;
                outOutput.strStatus = "counter window succeeded";
            }
            else if (reachedEnd)
            {
                Close_Window(boss, window, state, players);
                Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
                    pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                outOutput.strStatus = "counter window timed out";
            }
            break;
        }
		case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
		{
			const auto maximum = catalog.Get_RaidStaggerMaximum() ? catalog.Get_RaidStaggerMaximum() : window.iThreshold;
			boss.iKoukuItemStaggerMaximum = maximum;
			const std::uint64_t progress = boss.iKoukuItemStaggerCredit;
			if (maximum > 0u && progress >= maximum)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				if (window.bEndsPatternOnSuccess)
				{
					outOutput.bEndPatternEarly = true;
					outOutput.bStaggerSuccess = true;
				}
				// Existing skill HP contribution and item-only stagger credit share this window.
				// Publish its one-shot success without inventing damage or a contributor.
				if (outDamageEvents.size() < LostArk::Shared::MAX_DAMAGE_EVENTS)
				{
					LostArk::Shared::DAMAGE_EVENT success{};
					success.iTargetNetEntityId = boss.iNetEntityId;
					success.fPositionX = boss.fPositionX; success.fPositionY = boss.fPositionY; success.fPositionZ = boss.fPositionZ;
					success.isOutgoing = true; success.isStaggerSuccess = true;
					outDamageEvents.push_back(success);
				}
				outOutput.strStatus = "stagger window succeeded";
			}
			else if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "stagger window timed out";
			}
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
		case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
		{
            if (window.bGazeDuringWindow)
            {
                // Use the same player-facing judgement as the end-tick gaze, within [start, end).
                if (reachedEnd) { Close_Window(boss, window, state, players); break; }
                for (auto& [playerId, player] : players)
                {
                    if ((!Is_Judgeable(player) && !(player.Is_Human() && player.iCurrentHp && Has_LethalResult(window.OnFail))) ||
                        state.Answers[playerId] == KOUKUSAYDON_LOGIC_ANSWER::FAIL ||
                        Judge_Gaze(window, boss, player) != KOUKUSAYDON_LOGIC_ANSWER::FAIL) continue;
                    state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::FAIL;
                    Apply_Results(window.OnFail, state, &player, players, boss, catalog, pMadnessPolicy,
                        serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                }
                break;
            }
			if (!reachedEnd)
				break;
			/* Judge before the cards go back so the card that was dealt is the
			card that is compared. */
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER>> verdicts;
			for (auto& [playerId, player] : players)
			{
				if (!player.Is_Human() || !player.iCurrentHp) continue;
				const KOUKUSAYDON_LOGIC_ANSWER answer =
					BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH == window.eKind ?
						Judge_Roulette(window, boss, player) : Judge_Gaze(window, boss, player);
                const auto& results = answer == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS ? window.OnSuccess :
                    answer == KOUKUSAYDON_LOGIC_ANSWER::FAIL ? window.OnFail : window.OnTimeout;
                if (!Is_Judgeable(player) && !Has_LethalResult(results)) continue;
				state.Answers[playerId] = answer;
				verdicts.emplace_back(playerId, answer);
			}
			Close_Window(boss, window, state, players);
			for (const auto& [playerId, answer] : verdicts)
			{
				const auto found = players.find(playerId);
				if (players.end() == found)
					continue;
				Apply_Results(
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer ? window.OnSuccess :
					(KOUKUSAYDON_LOGIC_ANSWER::FAIL == answer ? window.OnFail : window.OnTimeout),
					state, &found->second, players, boss, catalog, pMadnessPolicy, serverTick,
					outDamageEvents, outOutput, invulnerablePlayers);
			}
			outOutput.strStatus = "end-tick window judged";
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
		{
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ target{
				window.fTargetWorldX, window.fTargetWorldZ, window.fTargetRadiusM };
			const bool inside = !window.strTargetWorldInstanceId.empty() &&
				std::any_of(window.CardRegions.begin(), window.CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& region) {
						return Intersects_LogicRegion(Window_Region(region, state), boss, target, serverTick - ledger.iPatternStartTick);
					});
			if (inside)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.bInsideIsFail ? window.OnFail : window.OnSuccess,
					state, nullptr, players, boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "object overlap window judged";
			}
			else if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "object overlap window timed out";
			}
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP:
		case BOSS_PATTERN_LOGIC_KIND::ENTER_AREA:
		{
			const bool enter = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA == window.eKind;
			// Repeating contact emits only inside [birth, expiry), matching the visible Collider lifetime.
			if (window.iRepeatIntervalMs && reachedEnd)
			{ Close_Window(boss, window, state, players); break; }
			// Mario admission and its party-wide failure are committed by the room.
			// Other participants must not time out merely because one player entered.
			if (enter && window.OnSuccess.size() == 1u &&
				window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER)
			{
				if (reachedEnd) Close_Window(boss, window, state, players);
				break;
			}
			if (!enter && !window.iRepeatIntervalMs && !reachedEnd) break;
			const auto harmful = [](const auto& results) {
				return std::any_of(results.begin(), results.end(), [](const auto& result) {
					return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
						result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
						result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE;
				});
			};
			const auto& contactResults = window.bInsideIsFail ? window.OnFail : window.OnSuccess;
			const bool lethalContact = std::any_of(contactResults.begin(), contactResults.end(), [](const auto& result) {
				return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
					(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
			});
			const bool captureContact = enter && harmful(contactResults);
			for (auto& [playerId, player] : players)
			{
				const bool ownedCapture = captureContact && Is_CurrentBossHandCapture(
					player, boss.iNetEntityId, boss.iPatternSequence, serverTick);
                const bool receivesContact = !player.Has_TimeStop(serverTick) && (Can_ReceiveSpatialContact(player) || ownedCapture);
                const bool lethalTimeout = reachedEnd && player.Is_Human() && Has_LethalResult(window.OnTimeout);
				if (!player.iCurrentHp || (!lethalContact && !receivesContact && !lethalTimeout) || (enter && !window.bRearmOnExit && !window.bRepeatAfterKnockback && !window.iRepeatIntervalMs &&
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == state.Answers[playerId]))
					continue;
				const auto caught = std::find_if(window.CardRegions.begin(), window.CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& region) {
						const auto elapsed = serverTick - ledger.iPatternStartTick;
						const auto& sampledRegion = Window_Region(region, state);
						return Contains_LogicRegion(sampledRegion, boss, player, elapsed) ||
							(enter && (Swept_LinearColliderContainsPlayer(sampledRegion, boss, player, elapsed,
								state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
								Swept_CenteredWorldCircleContainsPlayer(sampledRegion, boss, player, elapsed,
								state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
                                Swept_WorldBoxContainsPlayer(sampledRegion, boss, player, elapsed,
                                    state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick)));
					});
				const bool inside = window.CardRegions.end() != caught;
                // A timeout exception may reach this point solely for an outside verdict.
                if (inside && !lethalContact && !receivesContact) continue;
				if (player.Is_Guide() && !inside)
                { state.InsidePlayers.erase(playerId); continue; }
				const bool captureCandidate = inside && enter && !window.OnSuccess.empty() &&
					(window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER ||
					 window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT);
				if (window.iRepeatIntervalMs)
				{
					if (inside)
					{
						const auto next = state.NextContactHitTicks.find(playerId);
						if (next != state.NextContactHitTicks.end() && !Has_ReachedTick(serverTick, next->second)) continue;
					}
					else if (const auto previous = state.Answers.find(playerId); previous != state.Answers.end() &&
						previous->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
				}
				if (enter && window.bRepeatAfterKnockback)
				{
					if (inside)
					{
						const auto next = state.NextContactHitTicks.find(playerId);
						if (!CPlayerSkillSystem::Can_ArmPlayerHitReaction(player, serverTick, window.OnSuccess.front().bForcePush) ||
							(next != state.NextContactHitTicks.end() && !Has_ReachedTick(serverTick, next->second))) continue;
					}
					else
					{
						const auto answer = state.Answers.find(playerId);
						if (answer != state.Answers.end() && answer->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
					}
				}
				if (enter && window.bRearmOnExit)
				{
					if (inside)
					{
						if (captureCandidate)
						{
							if (state.InsidePlayers.contains(playerId)) continue;
						}
						else if (!state.InsidePlayers.insert(playerId).second) continue;
					}
					else
					{
						state.InsidePlayers.erase(playerId);
						// Timeout belongs only to players who never entered this window.
						const auto answer = state.Answers.find(playerId);
						if (answer != state.Answers.end() && answer->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
					}
				}
				if (!inside && !reachedEnd) continue;
				// Capture is committed by the room; an ineligible candidate remains retryable.
				if (!captureCandidate)
					state.Answers[playerId] = inside ? (window.bInsideIsFail ? KOUKUSAYDON_LOGIC_ANSWER::FAIL :
						KOUKUSAYDON_LOGIC_ANSWER::SUCCESS) : KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT;
				const std::vector<BOSS_PATTERN_LOGIC_RESULT>& answered = inside ?
					(window.bInsideIsFail ? window.OnFail : window.OnSuccess) : window.OnTimeout;
				LOGIC_REGION_TRANSFORM contactTransform{};
				std::array<float, 2u> contactCenter{};
				const bool hasContactCenter = inside && Resolve_LogicRegionTransform(Window_Region(*caught, state), boss,
					serverTick - ledger.iPatternStartTick, contactTransform, false);
				if (hasContactCenter) contactCenter = { contactTransform.centerX, contactTransform.centerZ };
				Apply_Results(answered, state, &player, players,
					boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers,
					hasContactCenter ? &contactCenter : nullptr, true);
				if (inside && window.iRepeatIntervalMs)
					state.NextContactHitTicks[playerId] = Add_Ticks(serverTick, Ticks_FromMs(window.iRepeatIntervalMs));
				if (inside && enter && window.bRepeatAfterKnockback && !answered.empty())
					state.NextContactHitTicks[playerId] = Add_Ticks(serverTick, Ticks_FromMs(answered.front().iPushMs));
				/* Only this seam knows which region caught this player, and that
				region is the thing that will carry them, so the grab happens here
				rather than in the typed result switch. */
				if (inside && std::any_of(answered.begin(), answered.end(),
					[](const BOSS_PATTERN_LOGIC_RESULT& result)
					{ return BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT == result.eKind; }))
					if (Hang_PlayerOnRegion(player, *caught, boss,
						serverTick - ledger.iPatternStartTick, state.iWindowIndex,
						static_cast<std::uint32_t>(caught - window.CardRegions.begin()),
						(!caught->WorldTrack.Keys.empty() && caught->WorldTrack.Keys.front().bHasGripPosition) ?
                            Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(caught->WorldTrack.iStartMs + caught->WorldTrack.iDurationMs)) :
                            state.iEndTick, serverTick, navigation))
					{
						state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
						state.InsidePlayers.insert(playerId);
					}
			}
			if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
		{
			if (!reachedEnd)
				break;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER>> verdicts;
			for (auto& [playerId, player] : players)
			{
				if (!player.Is_Human() || !player.iCurrentHp) continue;
				const auto answer = state.Answers.find(playerId);
                const auto value = state.Answers.end() == answer ? KOUKUSAYDON_LOGIC_ANSWER::NONE : answer->second;
                const auto& results = value == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS ? window.OnSuccess :
                    value == KOUKUSAYDON_LOGIC_ANSWER::FAIL ? window.OnFail : window.OnTimeout;
                if (!Is_Judgeable(player) && !Has_LethalResult(results)) continue;
				verdicts.emplace_back(playerId, value);
			}
			Close_Window(boss, window, state, players);
			for (const auto& [playerId, answer] : verdicts)
			{
				const auto found = players.find(playerId);
				if (players.end() == found)
					continue;
				const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results =
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer ? window.OnSuccess :
					(KOUKUSAYDON_LOGIC_ANSWER::FAIL == answer ? window.OnFail : window.OnTimeout);
				Apply_Results(results, state, &found->second, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
			}
			outOutput.strStatus = "pose window judged";
			break;
		}
		default:
			if (reachedEnd)
				Close_Window(boss, window, state, players);
			break;
		}
	}
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Record_InteractionSlot(
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	const BOSS_PATTERN_DEFINITION& pattern,
	const SERVER_PLAYER& player,
	const LostArk::Shared::INTERACTION_SLOT slot,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
	{
		outStatus = "no running pattern owns a dance window";
		return false;
	}
	if (!Is_Valid_InteractionSlot(slot))
	{
		outStatus = "slot is out of range";
		return false;
	}
	const std::int8_t pose = player.ModeSkillIndexBySlot[static_cast<std::size_t>(slot)];
	if (pose < 0)
	{
		outStatus = "empty slot";
		return false;
	}
	for (KOUKUSAYDON_LOGIC_WINDOW_STATE& state : ledger.Windows)
	{
		if (!state.bOpened || state.bClosed ||
			state.iWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[state.iWindowIndex];
		if (BOSS_PATTERN_LOGIC_KIND::POSE_INPUT != window.eKind)
			continue;
		auto answer = state.Answers.find(player.iPlayerId);
		if (state.Answers.end() == answer)
			answer = state.Answers.emplace(player.iPlayerId, KOUKUSAYDON_LOGIC_ANSWER::NONE).first;
		if (KOUKUSAYDON_LOGIC_ANSWER::NONE != answer->second)
		{
			outStatus = "already answered";
			return false;
		}
		answer->second = static_cast<std::uint32_t>(pose) == window.iPoseIndex ?
			KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
		outStatus = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer->second ?
			"pose matched" : "pose mismatched";
		return true;
	}
	outStatus = "no open dance window";
	return false;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Update_PlayerModes(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const KOUKUSAYDON_LOGIC_LEDGER* const pActiveLedger,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	KOUKU_HUD_MODE ledgerMode = nullptr != pActiveLedger && pActiveLedger->Is_Active() ?
		pActiveLedger->eHudMode : KOUKU_HUD_MODE::NONE;
	if (ledgerMode == KOUKU_HUD_MODE::NONE && pActiveLedger && pActiveLedger->Is_Active() &&
		std::any_of(pActiveLedger->ScheduledDanceIntervals.begin(), pActiveLedger->ScheduledDanceIntervals.end(),
			[&](const auto& interval) { return Has_ReachedTick(serverTick, interval.first) && !Has_ReachedTick(serverTick, interval.second); }))
		ledgerMode = KOUKU_HUD_MODE::DANCE;
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		if (player.Is_Guide())
		{
			player.Clear_KoukuInteractionState();
			player.Clear_KoukuAssignedCard();
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			player.iCurrentMadness = player.iMadnessDamageGainPercent = 0u;
			player.dMadnessRemainder = 0.;
			continue;
		}
		const KOUKU_HUD_MODE patternMode = nullptr != pActiveLedger &&
			0u != player.iKoukuSuppressedPatternSequence &&
			player.iKoukuSuppressedPatternSequence == pActiveLedger->iPatternSequence ?
			KOUKU_HUD_MODE::NONE : ledgerMode;
		player.iMadnessDamageGainPercent = pMadnessPolicy ? pMadnessPolicy->iDamageGainPercent : 0u;
		if (!pMadnessPolicy) player.dMadnessRemainder = 0.;
		if (nullptr != pMadnessPolicy && 0u != pMadnessPolicy->iMaximum)
		{
			player.iMaximumMadness = pMadnessPolicy->iMaximum;
			player.iCurrentMadness = (std::min)(player.iCurrentMadness, player.iMaximumMadness);
			if (player.iCurrentMadness >= player.iMaximumMadness)
				Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
		}
		if (PLAYER_MADNESS_FORM::CLOWN == player.eMadnessForm &&
			0u != player.iMadnessFormEndTick &&
			Has_ReachedTick(serverTick, player.iMadnessFormEndTick))
		{
			if (KOUKU_HUD_MODE::NONE == patternMode &&
				KOUKU_HUD_MODE::NONE == player.eKoukuAreaHudMode &&
				KOUKU_HUD_MODE::NONE == player.eDebugKoukuHudModeOverride)
			{
				player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
				player.iMadnessFormEndTick = 0u;
			}
			// Retain an expired hold through Mario so leaving the mode restores
			// Normal on the next tick instead of creating a permanent clown.
		}
		for (std::int8_t& index : player.ModeSkillIndexBySlot)
			index = KOUKU_HUD_SLOT_EMPTY;
		const bool alive = 0u != player.iCurrentHp && PLAYER_ACTION_STATE::DEAD != player.eAction;
		if (!alive)
		{
			// The area's HUD presentation also applies to dead spectators. Keep
			// its mode while clearing every actionable slot and polymorph state.
			const auto areaMode = player.eKoukuAreaHudMode == KOUKU_HUD_MODE::MAZE ?
				KOUKU_HUD_MODE::MAZE : KOUKU_HUD_MODE::NONE;
			player.Clear_KoukuInteractionState();
			player.eKoukuAreaHudMode = areaMode;
			player.eKoukuHudMode = patternMode == KOUKU_HUD_MODE::DANCE ? patternMode : areaMode;
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			continue;
		}
		KOUKU_HUD_MODE mode = KOUKU_HUD_MODE::NONE != patternMode ?
			patternMode : (KOUKU_HUD_MODE::NONE != player.eKoukuAreaHudMode ?
				player.eKoukuAreaHudMode : player.eDebugKoukuHudModeOverride);
		if (KOUKU_HUD_MODE::NONE != patternMode &&
			PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm)
		{
			player.bKoukuPatternOwnsClown = true;
			player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		}
		else if (KOUKU_HUD_MODE::NONE == patternMode && player.bKoukuPatternOwnsClown)
		{
			player.bKoukuPatternOwnsClown = false;
			if (KOUKU_HUD_MODE::NONE == mode && 0u == player.iMadnessFormEndTick)
				player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		}
		if (KOUKU_HUD_MODE::NONE == mode && PLAYER_MADNESS_FORM::CLOWN == player.eMadnessForm)
			mode = KOUKU_HUD_MODE::POLYMORPH;
		// A mode change cannot reinterpret an interaction animation already in flight.
		if (player.eKoukuHudMode != mode && (PLAYER_ACTION_STATE::INTERACTION == player.eAction ||
			PLAYER_ACTION_STATE::SKILL == player.eAction))
		{
			player.eAction = PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iComboStage = 0u;
			player.hasBufferedComboInput = false;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
		}
		player.eKoukuHudMode = mode;
		std::uint32_t count = 0u;
		switch (mode)
		{
		case KOUKU_HUD_MODE::DANCE: count = 4u; break;
		case KOUKU_HUD_MODE::POLYMORPH: count = 3u; break;
		case KOUKU_HUD_MODE::MARIO: count = 2u; break;
		case KOUKU_HUD_MODE::MAZE: count = 2u; break;
		case KOUKU_HUD_MODE::NONE: break;
		default: break;
		}
		for (std::uint32_t index = 0u; index < count; ++index)
			player.ModeSkillIndexBySlot[index] = static_cast<std::int8_t>(index);
	}
}

namespace
{
	constexpr LostArk::Shared::MECHANIC_CARD_SYMBOL CARD_MAZE_SUITS[4] = {
		LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND,
	};

	/* Server-owned xorshift seeded from the claim tick and claimant, so a
	deal is reproducible from the room's own clock and nothing else. */
	struct CARD_MAZE_RANDOM final
	{
		std::uint64_t iState;
		std::uint32_t Next() noexcept
		{
			iState ^= iState << 13;
			iState ^= iState >> 7;
			iState ^= iState << 17;
			return static_cast<std::uint32_t>(iState >> 32);
		}
		float Unit() noexcept
		{
			return static_cast<float>(Next() & 0xFFFFFFu) / 16777215.f;
		}
	};

	float DistanceXZ(const float ax, const float az, const float bx, const float bz) noexcept
	{
		const float dx = ax - bx;
		const float dz = az - bz;
		return std::sqrt(dx * dx + dz * dz);
	}
}

void LostArk::Server::CKoukuBingoRuntime::Promote_Lines() noexcept
{
	using namespace LostArk::Shared;
	for (std::int32_t line = 0; line < KOUKU_BINGO_SIDE * 2; ++line)
	{
		const std::uint32_t lineMask = Kouku_BingoLineMask(line);
		if (0u != lineMask && lineMask == (m_iWhiteMask & lineMask))
			m_iRedMask |= lineMask;
	}
}

void LostArk::Server::CKoukuBingoRuntime::Fill(const std::uint32_t cellMask) noexcept
{
	using namespace LostArk::Shared;
	m_iWhiteMask |= cellMask & KOUKU_BINGO_ALL_CELLS_MASK;
	Promote_Lines();
}

void LostArk::Server::CKoukuBingoRuntime::Detonate(const std::uint32_t cellMask) noexcept
{
	using namespace LostArk::Shared;
	/* A finished bingo is never disturbed, so red cells drop out first and
	the rest is decided against the board as it stands right now. */
	const std::uint32_t affected =
		(cellMask & KOUKU_BINGO_ALL_CELLS_MASK) & ~m_iRedMask;
	m_iWhiteMask ^= affected;
	Promote_Lines();
}

std::uint32_t LostArk::Server::CKoukuBingoRuntime::Count_CompletedRowsAndColumns() const noexcept
{
	std::uint32_t count = 0u;
	for (std::uint32_t line = 0u; line < LostArk::Shared::KOUKU_BINGO_SIDE * 2u; ++line)
	{
		const auto mask = LostArk::Shared::Kouku_BingoLineMask(line);
		if ((m_iRedMask & mask) == mask) ++count;
	}
	return count;
}

std::uint32_t LostArk::Server::CKoukuBingoRuntime::Count_UnconsumedRowsAndColumns() const noexcept
{
	std::uint32_t count = 0u;
	for (std::uint32_t line = 0u; line < LostArk::Shared::KOUKU_BINGO_SIDE * 2u; ++line)
	{
		const auto mask = LostArk::Shared::Kouku_BingoLineMask(line);
		if (!(m_iConsumedLineMask & (1u << line)) && (m_iRedMask & mask) == mask) ++count;
	}
	return count;
}

bool LostArk::Server::CKoukuBingoRuntime::Consume_CompletedRowsAndColumns(const std::uint32_t count) noexcept
{
	if (!count || Count_UnconsumedRowsAndColumns() < count) return false;
	auto remaining = count;
	for (std::uint32_t line = 0u; line < LostArk::Shared::KOUKU_BINGO_SIDE * 2u && remaining; ++line)
	{
		const auto bit = 1u << line;
		const auto mask = LostArk::Shared::Kouku_BingoLineMask(line);
		if (!(m_iConsumedLineMask & bit) && (m_iRedMask & mask) == mask)
		{ m_iConsumedLineMask |= bit; --remaining; }
	}
	return true;
}

bool LostArk::Server::CKoukuBingoRuntime::Start_Bomb(
	const LostArk::Shared::NET_ENTITY_ID carrier,
	const std::uint32_t detonateTick, const std::uint32_t markOrdinal,
	const std::uint32_t dropDelayMs, const std::uint32_t fuseMs) noexcept
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == carrier)
		return false;
	std::size_t freeSlot = m_Bombs.size();
	for (std::size_t slot = 0u; slot < m_Bombs.size(); ++slot)
	{
		const BOMB& bomb = m_Bombs[slot];
		if (BINGO_BOMB_PHASE::NONE == bomb.ePhase && !bomb.iPlantTick)
		{
			if (freeSlot == m_Bombs.size())
				freeSlot = slot;
			continue;
		}
		if (BINGO_BOMB_PHASE::MARKED == bomb.ePhase &&
			bomb.iCarrierNetEntityId == carrier)
		{
			return false;
		}
	}
	if (freeSlot == m_Bombs.size())
		return false;
	BOMB& started = m_Bombs[freeSlot];
	started = {};
	started.ePhase = BINGO_BOMB_PHASE::MARKED;
	started.iCarrierNetEntityId = carrier;
	started.iDetonateTick = detonateTick;
	started.iMarkOrdinal = markOrdinal;
	started.iDropDelayMs = dropDelayMs;
	started.iFuseMs = fuseMs;
	return true;
}

void LostArk::Server::CKoukuBingoRuntime::Plant_Bomb(
	const std::size_t slot, const float x, const float z,
	const std::uint32_t fuseTick) noexcept
{
	using namespace LostArk::Shared;
	if (slot >= m_Bombs.size() ||
		(BINGO_BOMB_PHASE::MARKED != m_Bombs[slot].ePhase && !m_Bombs[slot].iPlantTick))
	{
		return;
	}
	BOMB& bomb = m_Bombs[slot];
	bomb.ePhase = BINGO_BOMB_PHASE::PLANTED;
	bomb.iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
	bomb.fPositionX = x;
	bomb.fPositionZ = z;
	bomb.iDetonateTick = fuseTick;
	bomb.iPlantTick = 0u;
}

void LostArk::Server::CKoukuBingoRuntime::Queue_BombPlant(
	const std::size_t slot, const float x, const float z, const std::uint32_t plantTick) noexcept
{
	using namespace LostArk::Shared;
	if (slot >= m_Bombs.size() || m_Bombs[slot].ePhase != BINGO_BOMB_PHASE::MARKED || !plantTick) return;
	auto& bomb = m_Bombs[slot];
	bomb.ePhase = BINGO_BOMB_PHASE::NONE;
	bomb.iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
	bomb.fPositionX = x; bomb.fPositionZ = z;
	bomb.iPlantTick = plantTick; bomb.iDetonateTick = 0u;
}

void LostArk::Server::CKoukuBingoRuntime::Clear_Bomb(
	const std::size_t slot) noexcept
{
	if (slot < m_Bombs.size())
		m_Bombs[slot] = {};
}

bool LostArk::Server::CKoukuBingoRuntime::Start_Hammer(
	const std::int32_t anchor, const std::uint32_t startTick,
	const std::uint32_t endTick) noexcept
{
	using namespace LostArk::Shared;
	if (BINGO_HAMMER_PHASE::NONE != m_Hammer.ePhase ||
		!Is_KoukuBingoHammerAnchor(anchor) || endTick <= startTick)
	{
		return false;
	}
	m_Hammer.iAnchor = anchor;
	m_Hammer.ePhase = BINGO_HAMMER_PHASE::RAISED;
	m_Hammer.iPhaseStartTick = startTick;
	m_Hammer.iPhaseEndTick = endTick;
	return true;
}

bool LostArk::Server::CKoukuBingoRuntime::Advance_Hammer(
	const std::uint32_t tick) noexcept
{
	using namespace LostArk::Shared;
	if (BINGO_HAMMER_PHASE::NONE == m_Hammer.ePhase)
		return false;
	if (tick < m_Hammer.iPhaseEndTick)
		return true;
	/* Each phase hands its end tick to the next one, so the three windows
	stay contiguous however late this tick arrives. */
	const auto ticksFor = [](const std::uint32_t milliseconds) noexcept
	{
		return (milliseconds * 30u + 999u) / 1000u;
	};
	m_Hammer.iPhaseStartTick = m_Hammer.iPhaseEndTick;
	if (BINGO_HAMMER_PHASE::RAISED == m_Hammer.ePhase)
	{
		m_Hammer.ePhase = BINGO_HAMMER_PHASE::DESCENDING;
		m_Hammer.iPhaseEndTick +=
			ticksFor(KOUKU_BINGO_HAMMER_DESCEND_MS);
		return true;
	}
	if (BINGO_HAMMER_PHASE::DESCENDING == m_Hammer.ePhase)
	{
		m_Hammer.ePhase = BINGO_HAMMER_PHASE::SWEEPING;
		m_Hammer.iPhaseEndTick +=
			ticksFor(KOUKU_BINGO_HAMMER_SWEEP_MS);
		return true;
	}
	m_Hammer = {};
	return false;
}

bool LostArk::Server::CKoukuBingoRuntime::Is_Safe(
	const float x, const float z) const noexcept
{
	using namespace LostArk::Shared;
	const std::int32_t cell = Kouku_BingoCellAt(x, z);
	if (!Is_KoukuBingoCell(cell))
		return false;
	return 0u != (m_iRedMask & (1u << cell));
}

const char* LostArk::Server::CKoukuCardMazeRuntime::Archetype_ForSuit(
	const LostArk::Shared::MECHANIC_CARD_SYMBOL suit) noexcept
{
	using namespace LostArk::Shared;
	switch (suit)
	{
	case MECHANIC_CARD_SYMBOL::HEART: return "MONSTER_KOUKU_CARD_HEART";
	case MECHANIC_CARD_SYMBOL::SPADE: return "MONSTER_KOUKU_CARD_SPADE";
	case MECHANIC_CARD_SYMBOL::CLUB: return "MONSTER_KOUKU_CARD_CLUB";
	case MECHANIC_CARD_SYMBOL::DIAMOND: return "MONSTER_KOUKU_CARD_DIAMOND";
	default: return "";
	}
}

bool LostArk::Server::CKoukuCardMazeRuntime::Plan(
	const LostArk::Shared::PLAYER_ID claimantId,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation& navigation,
	const std::uint32_t serverTick,
	std::vector<SPAWN_REQUEST>& outSpawns,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	outSpawns.clear();
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = INVALID_PLAYER_ID;
	const auto claimant = players.find(claimantId);
	if (players.end() == claimant || claimant->second.Is_Guide() || 0u == claimant->second.iCurrentHp)
	{
		outStatus = "Card maze claimant is not a living player";
		return false;
	}
	if (PHASE::HUNTING == m_ePhase)
	{
		outStatus = "Card maze is already running";
		return false;
	}
	/* Hunters are every other living player, in PlayerId order so the same
	room deals the same way whatever the map iteration happens to be. */
	std::vector<PLAYER_ID> hunters;
	for (const auto& [playerId, player] : players)
	{
		if (player.Is_Human() && playerId != claimantId && 0u != player.iCurrentHp)
			hunters.push_back(playerId);
	}
	if (hunters.size() > 3u)
		hunters.resize(3u);
	// F1 and normal entry use the same single-player rehearsal in both builds.
	// Dead members of a multiplayer room must not silently change its role rules.
	const bool soloTest = std::count_if(players.begin(), players.end(),
        [](const auto& entry) { return entry.second.Is_Human(); }) == 1;
	if (hunters.empty() && !soloTest)
	{
		outStatus = "Card maze needs at least one other living player to hunt";
		return false;
	}
	CARD_MAZE_RANDOM random{
		Mix((static_cast<std::uint64_t>(serverTick) << 32) ^ static_cast<std::uint64_t>(claimantId)) | 1ull };
	MECHANIC_CARD_SYMBOL deck[4] = {
		CARD_MAZE_SUITS[0], CARD_MAZE_SUITS[1], CARD_MAZE_SUITS[2], CARD_MAZE_SUITS[3] };
	for (std::uint32_t index = 3u; index > 0u; --index)
	{
		const std::uint32_t swapWith = random.Next() % (index + 1u);
		std::swap(deck[index], deck[swapWith]);
	}
	PARTICIPANT owner{};
	owner.eRole = CARD_MAZE_ROLE::TELESCOPE;
	// Multiplayer telescope owners remain suitless; solo is a hunter with overhead access.
	if (soloTest)
	{
		owner.eRole = CARD_MAZE_ROLE::HUNTER;
		owner.eSuit = deck[0];
	}
	std::vector<std::pair<PLAYER_ID, MECHANIC_CARD_SYMBOL>> dealt;
	if (soloTest) dealt.emplace_back(claimantId, owner.eSuit);
	for (std::size_t hunterIndex = 0u; hunterIndex < hunters.size(); ++hunterIndex)
		dealt.emplace_back(hunters[hunterIndex], deck[hunterIndex]);
	m_PendingParticipants[claimantId] = owner;
	std::vector<std::pair<float, float>> taken;
	for (const auto& [dealtId, dealtSuit] : dealt)
	{
		if (dealtId != claimantId)
		{
			PARTICIPANT hunter{};
			hunter.eRole = CARD_MAZE_ROLE::HUNTER;
			hunter.eSuit = dealtSuit;
			m_PendingParticipants[dealtId] = hunter;
		}
		for (std::uint32_t target = 0u; target < TARGETS_PER_SUIT; ++target)
		{
			bool placed = false;
			for (std::uint32_t attempt = 0u; attempt < SAMPLE_ATTEMPTS && !placed; ++attempt)
			{
				/* A random point in the maze rectangle projected onto the
				nearest walkable cell: the CardMiro region is 0.5 m, so the
				result is a corridor cell, never the top of a card wall. */
				const float sampleX = MAZE_MIN_X + (MAZE_MAX_X - MAZE_MIN_X) * random.Unit();
				const float sampleZ = MAZE_MIN_Z + (MAZE_MAX_Z - MAZE_MIN_Z) * random.Unit();
				SERVER_NAV_POINT point{};
				if (!navigation.Project_Point(sampleX, sampleZ, point) ||
					point.x < MAZE_MIN_X || point.x > MAZE_MAX_X ||
					point.z < MAZE_MIN_Z || point.z > MAZE_MAX_Z ||
					std::abs(point.y + .01f) > .5f ||
					DistanceXZ(point.x, point.z, CENTER_X, CENTER_Z) < CENTER_KEEPOUT_M)
				{
					continue;
				}
				bool clear = true;
				for (const auto& [playerId, player] : players)
				{
					(void)playerId;
					if (0u != player.iCurrentHp &&
						DistanceXZ(point.x, point.z, player.fPositionX, player.fPositionZ) < PLAYER_KEEPOUT_M)
					{
						clear = false;
						break;
					}
				}
				for (std::size_t takenIndex = 0u; clear && takenIndex < taken.size(); ++takenIndex)
				{
					if (DistanceXZ(point.x, point.z, taken[takenIndex].first, taken[takenIndex].second) < SPAWN_SPACING_M)
						clear = false;
				}
				if (!clear)
					continue;
				std::vector<SERVER_NAV_POINT> route;
				if (!navigation.Find_Path(CENTER_X, CENTER_Z, point.x, point.z, route) || route.empty() ||
					DistanceXZ(route.back().x, route.back().z, point.x, point.z) > .75f) continue;
				SPAWN_REQUEST request{};
				request.eSuit = dealtSuit;
				request.fPositionX = point.x;
				request.fPositionY = point.y;
				request.fPositionZ = point.z;
				request.fYawDegrees = 360.f * random.Unit();
				outSpawns.push_back(request);
				taken.emplace_back(point.x, point.z);
				placed = true;
			}
			if (!placed)
			{
				outStatus = "Card maze found no free corridor cell for a target";
				m_PendingParticipants.clear();
				outSpawns.clear();
				return false;
			}
		}
	}
	m_iPendingTelescopeOwner = claimantId;
	outStatus = "Card maze planned";
	return true;
}

void LostArk::Server::CKoukuCardMazeRuntime::Register_Target(
	const LostArk::Shared::NET_ENTITY_ID entityId,
	const LostArk::Shared::MECHANIC_CARD_SYMBOL suit)
{
	m_Targets[entityId] = suit;
}

void LostArk::Server::CKoukuCardMazeRuntime::Commit(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	m_Participants = m_PendingParticipants;
	m_iTelescopeOwner = m_iPendingTelescopeOwner;
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_CountedKills.clear();
	m_bMarchStarted = false;
	m_ePhase = PHASE::HUNTING;
	for (const auto& [playerId, participant] : m_Participants)
	{
		const auto player = players.find(playerId);
		if (players.end() != player)
		{
			player->second.CardMaze = {};
			player->second.eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::MAZE;
			player->second.CardMaze.flags = playerId == m_iTelescopeOwner ? 1u : 0u;
			Apply_ToPlayer(participant, player->second);
		}
	}
}

void LostArk::Server::CKoukuCardMazeRuntime::Abort()
{
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_Targets.clear();
}

bool LostArk::Server::CKoukuCardMazeRuntime::Is_Target(
	const LostArk::Shared::NET_ENTITY_ID entityId) const noexcept
{
	return m_Targets.end() != m_Targets.find(entityId);
}

void LostArk::Server::CKoukuCardMazeRuntime::Apply_ToPlayer(
	const PARTICIPANT& participant, SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	player.eCardMazeRole = participant.eRole;
	player.eCardMazeSuit = participant.eSuit;
	player.iCardMazeKills = participant.iKills;
	player.iCardMazeKillTarget = MECHANIC_CARD_SYMBOL::NONE != participant.eSuit ? KILL_TARGET : 0u;
}

LostArk::Server::CKoukuCardMazeRuntime::HIT_OUTCOME
LostArk::Server::CKoukuCardMazeRuntime::On_TargetHit(
	SERVER_PLAYER& hunter, const SERVER_WORLD_ENTITY& target, const bool killed)
{
	using namespace LostArk::Shared;
	HIT_OUTCOME outcome{};
	const auto registered = m_Targets.find(target.iNetEntityId);
	const auto participant = m_Participants.find(hunter.iPlayerId);
	/* Only a dealt suit striking its own targets advances anything: a wrong
	suit, or the suitless owner, takes the damage the room already applied
	and nothing more. */
	if (PHASE::HUNTING != m_ePhase || m_Targets.end() == registered ||
		m_Participants.end() == participant ||
		participant->second.eSuit != registered->second)
	{
		return outcome;
	}
	if (!m_bMarchStarted)
	{
		m_bMarchStarted = true;
		outcome.bStartMarch = true;
	}
	if (killed && m_CountedKills.insert(target.iNetEntityId).second &&
		participant->second.iKills < KILL_TARGET)
	{
		++participant->second.iKills;
		outcome.bKillCounted = true;
		if (KILL_TARGET == participant->second.iKills)
		{
			outcome.bHunterComplete = true;
			// The room stages a separate walkable exit after the matching kill.
			// Failed corridor selection keeps this earned progress for the next tick.
			bool allComplete = true;
			for (const auto& [playerId, other] : m_Participants)
			{
				(void)playerId;
				if (MECHANIC_CARD_SYMBOL::NONE != other.eSuit && other.iKills < KILL_TARGET)
				{
					allComplete = false;
					break;
				}
			}
			// The matching kill unlocks a personal exit; central arrival completes the run.
			(void)allComplete;
		}
		Apply_ToPlayer(participant->second, hunter);
	}
	return outcome;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Remove_Player(
	const LostArk::Shared::PLAYER_ID playerId)
{
	using namespace LostArk::Shared;
	if (PHASE::INACTIVE == m_ePhase)
		return false;
	m_Participants.erase(playerId);
	if (playerId == m_iTelescopeOwner)
		m_iTelescopeOwner = INVALID_PLAYER_ID;
	if (PHASE::HUNTING != m_ePhase)
		return false;
	std::size_t hunters = 0u;
	std::size_t hunting = 0u;
	for (const auto& [id, participant] : m_Participants)
	{
		(void)id;
		if (MECHANIC_CARD_SYMBOL::NONE == participant.eSuit)
			continue;
		++hunters;
		if (participant.iKills < KILL_TARGET)
			++hunting;
	}
	if (0u == hunters)
		return true;
	(void)hunting;
	return false;
}

bool LostArk::Server::CKoukuCardMazeRuntime::In_SafeZone(float x, float z) noexcept
{
	return DistanceXZ(x, z, CENTER_X, CENTER_Z) <= CENTER_KEEPOUT_M;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Can_Hit(
	const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const
{
	const auto p = m_Participants.find(player.iPlayerId);
	const auto t = m_Targets.find(targetId);
	return m_ePhase == PHASE::HUNTING && p != m_Participants.end() && t != m_Targets.end() &&
		!p->second.escaped && p->second.iKills < KILL_TARGET && p->second.eSuit == t->second;
}

void LostArk::Server::CKoukuCardMazeRuntime::Retire_Target(LostArk::Shared::NET_ENTITY_ID id)
{
	m_Targets.erase(id);
	m_CountedKills.erase(id);
}

bool LostArk::Server::CKoukuCardMazeRuntime::Sample_Corridor(
	LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
	std::uint32_t seed, SPAWN_REQUEST& out, const bool avoidPersonalExits) const
{
	CARD_MAZE_RANDOM random{ Mix(static_cast<std::uint64_t>(seed) ^ (static_cast<std::uint64_t>(suit) << 32)) | 1ull };
	for (std::uint32_t attempt = 0u; attempt < SAMPLE_ATTEMPTS; ++attempt)
	{
		const float x = MAZE_MIN_X + (MAZE_MAX_X - MAZE_MIN_X) * random.Unit();
		const float z = MAZE_MIN_Z + (MAZE_MAX_Z - MAZE_MIN_Z) * random.Unit();
		SERVER_NAV_POINT point{};
		if (!navigation.Is_PointWalkableExact(x, z) || !navigation.Sample_Position(x, z, point) ||
			std::abs(point.y + .01f) > .5f || In_SafeZone(point.x, point.z)) continue;
		bool clear = true;
		for (const auto& [id, player] : players)
		{
			(void)id;
			if (player.iCurrentHp && DistanceXZ(point.x, point.z, player.fPositionX, player.fPositionZ) < PLAYER_KEEPOUT_M)
			{ clear = false; break; }
			if (avoidPersonalExits && player.iCurrentHp && (player.CardMaze.flags & 4u) &&
				DistanceXZ(point.x, point.z, player.CardMaze.exitX, player.CardMaze.exitZ) < SPAWN_SPACING_M)
			{ clear = false; break; }
		}
		for (const auto& entity : entities)
			if (entity.iCurrentHp && Is_Target(entity.iNetEntityId) &&
				DistanceXZ(point.x, point.z, entity.fPositionX, entity.fPositionZ) < SPAWN_SPACING_M) clear = false;
		if (!clear) continue;
		std::vector<SERVER_NAV_POINT> route;
		if (!navigation.Find_Path(CENTER_X, CENTER_Z, point.x, point.z, route) || route.empty() ||
			DistanceXZ(route.back().x, route.back().z, point.x, point.z) > .75f) continue;
		out = {suit, point.x, point.y, point.z, 360.f * random.Unit()};
		return true;
	}
	return false;
}

void LostArk::Server::CKoukuCardMazeRuntime::Reset_Progress(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end() || p->second.escaped) return;
	p->second.iKills = 0u;
	p->second.escaped = false;
	player.CardMaze.flags &= static_cast<std::uint8_t>(~6u);
	player.CardMaze.exitX = player.CardMaze.exitY = player.CardMaze.exitZ = 0.f;
	Apply_ToPlayer(p->second, player);
}

bool LostArk::Server::CKoukuCardMazeRuntime::Toggle_Telescope(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end() || player.iPlayerId != m_iTelescopeOwner ||
		!In_SafeZone(player.fPositionX, player.fPositionZ)) return false;
	player.CardMaze.flags ^= 1u;
	player.hasMoveGoal = false;
	player.MovePath.clear();
	return true;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Is_SoloHunter(
	LostArk::Shared::PLAYER_ID playerId) const noexcept
{
	const auto p = m_Participants.find(playerId);
	return playerId == m_iTelescopeOwner && m_Participants.size() == 1u && p != m_Participants.end() &&
		p->second.eRole == LostArk::Shared::CARD_MAZE_ROLE::HUNTER;
}

void LostArk::Server::CKoukuCardMazeRuntime::Mark_Escaped(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end()) return;
	p->second.escaped = true;
	player.CardMaze.flags = static_cast<std::uint8_t>((player.CardMaze.flags | 2u) & ~5u);
}

bool LostArk::Server::CKoukuCardMazeRuntime::All_LivingCentral(
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players) const
{
	bool anyLiving = false;
	for (const auto& [id, participant] : m_Participants)
	{
		const auto p = players.find(id);
		if (p == players.end() || !p->second.iCurrentHp) continue;
		anyLiving = true;
		if (!In_SafeZone(p->second.fPositionX, p->second.fPositionZ) || p->second.CardMaze.transferStartTick ||
			(participant.eSuit != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE && !participant.escaped)) return false;
	}
	return anyLiving;
}

void LostArk::Server::CKoukuCardMazeRuntime::Reset(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	for (const auto& [playerId, participant] : m_Participants)
	{
		(void)participant;
		const auto player = players.find(playerId);
		if (players.end() != player)
			player->second.Clear_CardMazeState();
	}
	m_Participants.clear();
	m_PendingParticipants.clear();
	m_iTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_Targets.clear();
	m_CountedKills.clear();
	m_bMarchStarted = false;
	m_ePhase = PHASE::INACTIVE;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Project_MechanicGauge(
	const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
	const KOUKUSAYDON_LOGIC_LEDGER& ledger, const std::uint32_t serverTick,
	LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot)
{
	using LostArk::Shared::BOSS_MECHANIC_GAUGE_KIND;
	snapshot.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::NONE;
	snapshot.iCurrentMechanicGauge = snapshot.iMaximumMechanicGauge = 0u;
	if (!boss.iCurrentHp || !ledger.Is_Active() || boss.strPatternId != pattern.strPatternId ||
		ledger.strPatternId != pattern.strPatternId || ledger.iPatternSequence != boss.iPatternSequence)
		return;
	for (const auto& state : ledger.Windows)
	{
		if (!state.bOpened || state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
			!Has_ReachedTick(serverTick, state.iStartTick) || Has_ReachedTick(serverTick, state.iEndTick)) continue;
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW || !window.iThreshold) continue;
		const auto maximum = boss.iKoukuItemStaggerMaximum;
		snapshot.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::STAGGER;
		snapshot.iMaximumMechanicGauge = maximum;
		snapshot.iCurrentMechanicGauge = maximum - (std::min)(boss.iKoukuItemStaggerCredit, maximum);
		return;
	}
	// The typed detonation is scheduled in parent time too: its preceding 13 s
	// are the black-hole damage interval. This never judges or cancels a pattern.
	constexpr std::uint32_t blackHoleMs = 13000u;
	for (const auto& cue : ledger.MechanicTriggers)
	{
		if (cue.bStarted || cue.iIndex >= pattern.MechanicTriggers.size()) continue;
		const auto& trigger = pattern.MechanicTriggers[cue.iIndex];
		if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION ||
			trigger.iStartMs < blackHoleMs || !boss.iMaximumHp) continue;
		const auto start = Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(trigger.iStartMs - blackHoleMs));
		if (!Has_ReachedTick(serverTick, start) || Has_ReachedTick(serverTick, cue.iStartTick)) continue;
		snapshot.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::BOSS_HP;
		snapshot.iCurrentMechanicGauge = (std::min)(boss.iCurrentHp, boss.iMaximumHp);
		snapshot.iMaximumMechanicGauge = boss.iMaximumHp;
		return;
	}
}
```


### Server/Private/ServerGameplayContractTests_KoukuProduct.cpp

실제 수정 대상: [ServerGameplayContractTests_KoukuProduct.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerGameplayContractTests_KoukuProduct.cpp)


#### G1-test-slot-oracle

현재 기준 블록:

```cpp
				std::vector<std::uint8_t> ownerPayload;
```

교체 블록:

```cpp
                // 본체는 저작 기준점에서 90도씩 떨어진 네 후보 중 하나여야 한다.
                const auto isAuthoredGazeSlot = [&](const SERVER_WORLD_ENTITY& entity)
                {
                    if (!expectedTeleport || expectedTeleport->ClockHours.size() != 3u)
                    {
                        return false;
                    }
                    const float sourceX = expectedTeleport->fTeleportX - centerX;
                    const float sourceZ = expectedTeleport->fTeleportZ - centerZ;
                    const float sourceRadius = std::sqrt(sourceX * sourceX + sourceZ * sourceZ);
                    const float sourceAngle = std::atan2(sourceX, sourceZ);
                    const std::array<std::uint32_t, 4u> hours{1u, expectedTeleport->ClockHours[0u],
                        expectedTeleport->ClockHours[1u], expectedTeleport->ClockHours[2u]};
                    for (const auto hour : hours)
                    {
                        const float angle = sourceAngle + static_cast<float>(hour - 1u) * 0.52359877559829887f;
                        const float x = hour == 1u ? expectedTeleport->fTeleportX : centerX + std::sin(angle) * sourceRadius;
                        const float z = hour == 1u ? expectedTeleport->fTeleportZ : centerZ + std::cos(angle) * sourceRadius;
                        if (std::abs(entity.fPositionX - x) < .001f && std::abs(entity.fPositionZ - z) < .001f)
                        {
                            return true;
                        }
                    }
                    return false;
                };
                std::vector<std::array<float, 2u>> occupied{{owner->fPositionX, owner->fPositionZ}};
				std::vector<std::uint8_t> ownerPayload;
```

#### G1-test-owner-position

현재 기준 블록:

```cpp
					std::abs(owner->fPositionX - expectedTeleport->fTeleportX) < 0.001f &&
					std::abs(owner->fPositionY - expectedTeleport->fTeleportY) < 0.001f &&
					std::abs(owner->fPositionZ - expectedTeleport->fTeleportZ) < 0.001f &&
```

교체 블록:

```cpp
                    isAuthoredGazeSlot(*owner) &&
                    std::abs(owner->fPositionY - expectedTeleport->fTeleportY) < 0.001f &&
```

#### G1-test-unique-clones

현재 기준 블록:

```cpp
						std::abs(clone.fPositionY - owner->fPositionY) < 0.001f && looksAtCenter(clone);
```

교체 블록:

```cpp
                        std::abs(clone.fPositionY - owner->fPositionY) < 0.001f && looksAtCenter(clone) &&
                        isAuthoredGazeSlot(clone) &&
                        std::none_of(occupied.begin(), occupied.end(), [&](const auto& position)
                        {
                            return std::hypot(clone.fPositionX - position[0u], clone.fPositionZ - position[1u]) < .001f;
                        });
                    occupied.push_back({clone.fPositionX, clone.fPositionZ});
```


#### ServerGameplayContractTests_KoukuProduct.cpp 전체 코드

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "KoukuSaydonBrain.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerApp.h"
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
#include <random>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuMarioEntryContact(TESTS& tests)
{
    for (const bool maze : { false, true })
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        SERVER_PLAYER entrant{};
        entrant.iPlayerId = 99701u; entrant.iNetEntityId = 99702u;
        entrant.iCurrentHp = entrant.iMaximumHp = 1000u; entrant.isCombatReady = true;
        entrant.iShield = 100000u; entrant.iInvulnerableEndTick = 10000u;
        entrant.iMarioStage = maze ? 0u : 2u;
        entrant.eKoukuAreaHudMode = maze ? KOUKU_HUD_MODE::MAZE : KOUKU_HUD_MODE::MARIO;
        room->m_Players.emplace(entrant.iPlayerId, entrant);
        auto partner = entrant; partner.iPlayerId = 99703u; partner.iNetEntityId = 99704u;
        partner.iMarioStage = 0u; partner.eKoukuAreaHudMode = KOUKU_HUD_MODE::NONE;
        partner.isCombatReady = false;
        room->m_Players.emplace(partner.iPlayerId, partner);
        room->Update_KoukuPlayerModes(100u);
        tests.Require(room->m_Players.at(entrant.iPlayerId).iKoukuMinigameEndTick == 2800u &&
            !room->m_Players.at(partner.iPlayerId).iKoukuMinigameEndTick,
            "The Server starts one ninety-second minigame deadline only for its participant");
        room->Update_KoukuPlayerModes(2799u);
        tests.Require(room->m_Players.at(entrant.iPlayerId).iCurrentHp == 1000u,
            "The minigame participant remains alive one tick before the ninety-second deadline");
        room->Update_KoukuPlayerModes(2800u);
        tests.Require(!room->m_Players.at(entrant.iPlayerId).iCurrentHp &&
            !room->m_Players.at(entrant.iPlayerId).iKoukuMinigameEndTick &&
            (maze ? !room->m_Players.at(partner.iPlayerId).iCurrentHp : room->m_Players.at(partner.iPlayerId).iCurrentHp == 1000u),
            "Mario timeout kills only its entrant; card maze timeout kills the party through shields and invulnerability");
        auto& escaped = room->m_Players.at(entrant.iPlayerId);
        escaped.iCurrentHp = 1000u; escaped.iMarioStage = 0u;
        escaped.eKoukuAreaHudMode = KOUKU_HUD_MODE::NONE; escaped.iKoukuMinigameEndTick = 2900u;
        room->Update_KoukuPlayerModes(2899u);
        tests.Require(!escaped.iKoukuMinigameEndTick && escaped.iCurrentHp == 1000u,
            "Leaving the minigame clears its deadline instead of carrying it into the next run");
    }
    for (const std::uint8_t source : { 1u, 2u })
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        const auto* policy = room->m_GameplayCatalog.Active().Find_KoukuMadnessPolicy("ENCOUNTER_KOUKUSAYDON_G1");
        tests.Require(policy && policy->iBallGainPercent == 10u && policy->iBallMultiplierPercent == 200u &&
            policy->iDollGainPercent == 10u && policy->iDollMultiplierPercent == 200u && policy->iSpecialIntervalMs == 1000u,
            "The published ball and doll policy independently owns twenty percent madness per second");
        if (!policy) continue;
        SERVER_WORLD_ENTITY boss{}; boss.iNetEntityId = 99601u; boss.iPatternSequence = 1u;
        boss.iCurrentHp = boss.iMaximumHp = 1000u;
        BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "contract.shield.madness";
        auto& world = pattern.WorldSequences.emplace_back();
        world.strOccurrenceId = "contract.special.world"; world.bAuthoredMadness = true;
        world.strInstanceId = source == 1u ? "world.object.instance.kouku.mario_circus_ball.aura" :
            "world.object.instance.kouku.odd_doll.att_battle_2_01";
        auto& window = pattern.LogicWindows.emplace_back(); window.strWindowId = "contract.special.contact";
        window.strOwnerWorldOccurrenceId = world.strOccurrenceId;
        window.eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP; window.iDurationMs = 1000u;
        window.iRepeatIntervalMs = 100u;
        auto& region = window.CardRegions.emplace_back(); region.strRegionId = "contract.special.region";
        region.bCircle = true; region.fRadiusM = 2.f; region.fHalfX = region.fHalfZ = 2.f;
        BOSS_PATTERN_LOGIC_RESULT hit{}; hit.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE; hit.iDamageAmount = 10u;
        BOSS_PATTERN_LOGIC_RESULT madness{}; madness.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
        madness.iPercent = 99u; window.OnSuccess = { hit, madness };
        SERVER_PLAYER player{}; player.iPlayerId = 99602u; player.iNetEntityId = 99603u;
        player.iCurrentHp = player.iMaximumHp = 1000u; player.iShield = 100000u;
        player.iMaximumMadness = 100u; player.isCombatReady = true; player.iMadnessDamageGainPercent = 99u;
        std::map<PLAYER_ID, SERVER_PLAYER> players{{player.iPlayerId, player}};
        KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output; std::vector<DAMAGE_EVENT> damage;
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
        tests.Require(ledger.Windows.front().iMadnessSource == source,
            "Authored special contact resolves its exact WORLD occurrence and ball/doll instance identity");
        for (std::uint32_t tick = 100u; tick < 130u; tick += 3u)
            CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, room->m_GameplayCatalog.Active(),
                policy, tick, damage, output);
        const auto& target = players.at(player.iPlayerId);
        tests.Require(target.iCurrentHp == 1000u && target.iShield < 100000u && target.iCurrentMadness == 20u &&
            target.iMadnessDamageGainPercent == 99u && damage.empty(),
            "Ten shield-absorbed 100-ms ball/doll hits add twenty percent madness without HP damage, combat text or double gain");
        enum class CONTACT_CASE { INVULNERABLE, HOLY_CHARM, ZERO_DAMAGE, OUTSIDE, TIME_STOP, DEAD, NOT_READY, CLOWN };
        for (const auto scenario : { CONTACT_CASE::INVULNERABLE, CONTACT_CASE::HOLY_CHARM, CONTACT_CASE::ZERO_DAMAGE,
            CONTACT_CASE::OUTSIDE, CONTACT_CASE::TIME_STOP, CONTACT_CASE::DEAD, CONTACT_CASE::NOT_READY, CONTACT_CASE::CLOWN })
        {
            auto& contactPlayer = players.at(player.iPlayerId);
            contactPlayer = player;
            window.OnSuccess.front().iDamageAmount = 10u;
            switch (scenario)
            {
            case CONTACT_CASE::INVULNERABLE: contactPlayer.iInvulnerableEndTick = 1000u; break;
            case CONTACT_CASE::HOLY_CHARM: contactPlayer.iHolyCharmProtectionEndTick = 1000u; break;
            case CONTACT_CASE::ZERO_DAMAGE: window.OnSuccess.front().iDamageAmount = 0u; break;
            case CONTACT_CASE::OUTSIDE: contactPlayer.fPositionX = 20.f; break;
            case CONTACT_CASE::TIME_STOP: contactPlayer.iTimeStopEndTick = 1000u; break;
            case CONTACT_CASE::DEAD: contactPlayer.iCurrentHp = 0u; break;
            case CONTACT_CASE::NOT_READY: contactPlayer.isCombatReady = false; break;
            case CONTACT_CASE::CLOWN: contactPlayer.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN; break;
            }
            damage.clear(); output = {};
            CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
            for (std::uint32_t tick = 100u; tick < 130u; ++tick)
                CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, room->m_GameplayCatalog.Active(),
                    policy, tick, damage, output);
            const bool independentGain = scenario == CONTACT_CASE::INVULNERABLE ||
                scenario == CONTACT_CASE::HOLY_CHARM || scenario == CONTACT_CASE::ZERO_DAMAGE;
            tests.Require(contactPlayer.iCurrentMadness == (independentGain ? 20u : 0u) && damage.empty() &&
                contactPlayer.iCurrentHp == (scenario == CONTACT_CASE::DEAD ? 0u : 1000u) &&
                contactPlayer.iMadnessDamageGainPercent == 99u &&
                (!independentGain || contactPlayer.iShield == player.iShield),
                "Special contact charges without HP/shield loss or text; outside, time stop, dead, unready and clown do not charge");
        }
    }
#ifdef _DEBUG
    for (const bool cancelled : { false, true })
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        SERVER_WORLD_ENTITY boss{}; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.iNetEntityId = 99891u; boss.iCurrentHp = boss.iMaximumHp = 1000u;
        boss.strArchetypeId = "BOSS_KOUKUSAYDON_G2_KOUKU";
        boss.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
        boss.strPlacementId = "contract.world.combat.owner"; boss.fCollisionRadius = 1.f;
        boss.PinnedDefinitionRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        room->m_WorldEntities.clear(); room->m_WorldEntities.push_back(boss);
        room->m_iNextNetEntityId = 99892u;
        S2C_WORLD_SEQUENCE_PLAY play{};
        play.strSequenceInstanceId = "world.object.contract.combat";
        play.iRunEpoch = 9u; play.strMemberId = "contract.member"; play.strCueId = "contract.cue";
        play.strOccurrenceId = "contract.occurrence"; play.iBossNetEntityId = boss.iNetEntityId;
        play.iStartTick = 1u; play.bUntilDestroyed = true;
        BOSS_PATTERN_WORLD_COMBAT_BODY body; body.iMaximumHp = 100u; body.fRadiusM = 1.f;
        const bool staged = room->Stage_KoukuWorldBody(body, play);
        tests.Require(staged && play.iCombatBodyNetEntityId == 99892u &&
            room->m_KoukuDamageableWorldCues.size() == 1u &&
            room->m_KoukuDamageableWorldCues.front().Play.iCombatBodyNetEntityId == play.iCombatBodyNetEntityId,
            "Only admitted Server WORLD bodies stamp their exact target identity into the owned PLAY");
        if (!staged) continue;
        room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        room->m_KoukuSaydonPatternAudition.WorldPlays.push_back(play);
        CGameRoom::STAGED_PLAYER_ENTRY entry;
        entry.Player.iPlayerId = 99893u; entry.Player.iNetEntityId = 99894u;
        std::string status;
        const auto countCues = [&](bool& duplicateBody) {
            std::size_t count = 0u; duplicateBody = false;
            for (const auto& frame : entry.Frames)
            {
                CPacketReader reader(frame.Payload);
                if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY)
                {
                    S2C_WORLD_SEQUENCE_PLAY decoded;
                    if (Read_Message(reader, decoded) && decoded.iCombatBodyNetEntityId == play.iCombatBodyNetEntityId) ++count;
                }
                else if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED)
                {
                    S2C_WORLD_ENTITY_SPAWNED decoded;
                    if (Read_Message(reader, decoded) && decoded.iNetEntityId == play.iCombatBodyNetEntityId) duplicateBody = true;
                }
            }
            return count;
        };
        bool duplicateBody = false;
        tests.Require(room->Build_PlayerEntryFrames(entry, {}, status) && countCues(duplicateBody) == 1u && !duplicateBody,
            "Late join replays one live combat WORLD cue with its body ID and no duplicate world-entity spawn");
        room->Update_KoukuWorldBodies(2u);
        if (cancelled) room->Cancel_KoukuWorldBodies(play.strMemberId);
        else for (auto& entity : room->m_WorldEntities)
            if (entity.iNetEntityId == play.iCombatBodyNetEntityId) entity.iCurrentHp = 0u;
        tests.Require(room->Build_PlayerEntryFrames(entry, {}, status) && countCues(duplicateBody) == 0u,
            "Late join does not revive a cancelled or zero-HP WORLD combat body before cleanup");
        room->Update_KoukuWorldBodies(3u);
        tests.Require(room->m_KoukuDamageableWorldCues.empty() && room->m_KoukuSaydonPatternAudition.WorldPlays.empty() &&
            std::none_of(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [&](const auto& entity) {
                return entity.iNetEntityId == play.iCombatBodyNetEntityId; }),
            "Death and explicit cancellation retire the exact combat body and owned cue together");
    }
    for (const std::uint8_t source : {1u, 2u}) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        SERVER_WORLD_ENTITY boss{}; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.iNetEntityId = 99001u; boss.iCurrentHp = boss.iMaximumHp = 1000u;
        boss.strPatternId = "test.damageable.contact"; boss.iPatternSequence = 3u;
        SERVER_WORLD_ENTITY body{}; body.eKind = WORLD_BOOTSTRAP_KIND::WORLD_OBJECT;
        body.iNetEntityId = 99002u; body.iCurrentHp = body.iMaximumHp = 1000u;
        room->m_WorldEntities.push_back(boss); room->m_WorldEntities.push_back(body);
        CGameRoom::KOUKU_DAMAGEABLE_WORLD_CUE cue{};
        cue.Play.iBossNetEntityId = boss.iNetEntityId; cue.iBodyId = body.iNetEntityId;
        cue.Play.iRunEpoch = 7u; cue.Play.iPatternSequence = boss.iPatternSequence;
        cue.Play.strMemberId = "contact.member"; cue.Play.strOccurrenceId = "contact.world";
        cue.iMadnessSource = source; cue.iNextMadnessTick = 30u;
        cue.MadnessPolicy.iMaximum = 100u; cue.MadnessPolicy.iClownHoldMs = 15000u;
        room->m_KoukuDamageableWorldCues.push_back(cue);
        room->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch = 7u;
        auto& member = room->m_KoukuSaydonPatternAudition.Members.emplace_back(); member.strMemberId = cue.Play.strMemberId;
        BOSS_PATTERN_DEFINITION ownedContact; ownedContact.strPatternId = boss.strPatternId;
        auto& ownedWindow = ownedContact.LogicWindows.emplace_back(); ownedWindow.strOwnerWorldOccurrenceId = cue.Play.strOccurrenceId;
        ownedWindow.eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP; ownedWindow.iDurationMs = 10000u; ownedWindow.iRepeatIntervalMs = 100u;
        auto& ownedRegion = ownedWindow.CardRegions.emplace_back(); ownedRegion.fHalfX = ownedRegion.fHalfZ = 10.f;
        BOSS_PATTERN_LOGIC_RESULT burn; burn.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE; burn.iDamageAmount = 10u;
        ownedWindow.OnSuccess.push_back(burn);
        CKoukuSaydonLogicRuntime::Build(ownedContact, boss, 1u, member.LogicLedger);
        SERVER_PLAYER player{}; player.iPlayerId = 99003u; player.iNetEntityId = 99004u;
        player.iCurrentHp = player.iMaximumHp = 1000u; player.isCombatReady = true;
        player.iMaximumMadness = 100u; player.fPositionZ = source == 1u ? 1.f : 3.f;
        room->m_Players.emplace(player.iPlayerId, player);
        auto outside = player; outside.iPlayerId = 99005u; outside.fPositionX = 3.f; outside.fPositionZ = 0.f;
        room->m_Players.emplace(outside.iPlayerId, outside);
        auto upper = player; upper.iPlayerId = 99006u; upper.fPositionY = 4.f;
        room->m_Players.emplace(upper.iPlayerId, upper);
        room->Update_KoukuWorldBodies(29u);
        tests.Require(!room->m_Players.at(player.iPlayerId).iCurrentMadness, "Special Madness contact respects its first interval");
        room->Update_KoukuWorldBodies(30u); room->Update_KoukuWorldBodies(30u);
        tests.Require(room->m_Players.at(player.iPlayerId).iCurrentMadness == 20u &&
            !room->m_Players.at(outside.iPlayerId).iCurrentMadness && !room->m_Players.at(upper.iPlayerId).iCurrentMadness,
            "Ball circle and doll sector apply tuned gain once per interval and exclude outside/other-floor players");
        room->Update_KoukuWorldBodies(60u);
        tests.Require(room->m_Players.at(player.iPlayerId).iCurrentMadness == 40u, "A surviving special object repeats contact gain on the Server clock");
        room->m_WorldEntities.back().iCurrentHp = 0u;
        room->Update_KoukuWorldBodies(90u);
        tests.Require(room->m_Players.at(player.iPlayerId).iCurrentMadness == 40u && room->m_KoukuDamageableWorldCues.empty(),
            "Destroying a special object cancels its Madness contact and its cue together");
        std::vector<DAMAGE_EVENT> ownedDamage; KOUKUSAYDON_LOGIC_OUTPUT ownedOutput;
        CKoukuSaydonLogicRuntime::Update(boss, ownedContact, member.LogicLedger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 90u, ownedDamage, ownedOutput);
        tests.Require(member.LogicLedger.RetiredWorldOccurrences.contains(cue.Play.strOccurrenceId) &&
            member.LogicLedger.Windows.front().bClosed && ownedDamage.empty() && room->m_Players.at(player.iPlayerId).iCurrentHp == 1000u,
            "WORLD body destruction retires the exact authored damage/madness window before any subsequent contact result");
    }
    BOSS_PATTERN_DEFINITION root{};
    BOSS_PATTERN_LOGIC_WINDOW entry{};
    entry.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
    entry.iStartMs = 2998u;
    BOSS_PATTERN_LOGIC_RESULT result{};
    result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER;
    entry.OnSuccess.push_back(result);
    BOSS_LOGIC_REGION region{};
    region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
    region.fHalfX = region.fHalfY = region.fHalfZ = 1.f;
    entry.CardRegions.push_back(region);
    root.LogicWindows.push_back(entry);
    SERVER_WORLD_ENTITY anchor{};
    anchor.fPositionX = -.07f; anchor.fPositionY = 1.32f; anchor.fPositionZ = 942.33f;
    const auto contactTick = CKoukuSaydonLogicRuntime::Ticks_FromMs(entry.iStartMs);
    CServerCollisionSystem collision;
    std::string status;
    tests.Require(collision.Initialize({}, status), "Initialize Mario contact movement collision");
    collision.Set_BlockingBodies({{anchor.fPositionX, anchor.fPositionZ, 1.f,
        anchor.fPositionY + 1.f, 1.f, 990u}});
    const std::array interruptible{PLAYER_ACTION_STATE::NONE, PLAYER_ACTION_STATE::SKILL,
        PLAYER_ACTION_STATE::INTERACTION, PLAYER_ACTION_STATE::KNOCKDOWN,
        PLAYER_ACTION_STATE::FEAR, PLAYER_ACTION_STATE::ESTHER_CAST};
    for (const float yaw : {0.f, 43.f, 90.f}) {
        anchor.fYawDegrees = yaw;
        const float radians = yaw * .017453292519943295f;
        const float rightX = std::cos(radians), rightZ = -std::sin(radians);
        SERVER_PLAYER player{};
        player.iPlayerId = 991u; player.iNetEntityId = 992u; player.iCurrentHp = 100u; player.isCombatReady = true;
        player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
        player.fPositionX = anchor.fPositionX + rightX * 1.5f;
        player.fPositionY = anchor.fPositionY; player.fPositionZ = anchor.fPositionZ + rightZ * 1.5f;
        const float proposedX = anchor.fPositionX + rightX * 1.4f;
        const float proposedZ = anchor.fPositionZ + rightZ * 1.4f;
        float x = 0.f, y = 0.f, z = 0.f; bool blocked = false;
        const bool moved = collision.Resolve_PlayerMove(player, proposedX, anchor.fPositionY,
            proposedZ, x, y, z, blocked);
        player.fPositionX = x; player.fPositionY = y; player.fPositionZ = z;
        const bool constrained = std::hypot(x - proposedX, z - proposedZ) > 0.001f;
        const bool beforeEntry = CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, player, contactTick - 1u);
        const bool touchingEntry = CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, player, contactTick);
        const auto oldPrecision = std::cout.precision(9);
        std::cout << "[MarioContactProbe] yaw=" << yaw << " proposed=(" << proposedX << ',' << proposedZ
            << ") actual=(" << x << ',' << z << ") moved=" << moved << " blocked=" << blocked
            << " constrained=" << constrained << " before=" << beforeEntry << " touching=" << touchingEntry << '\n';
        std::cout.precision(oldPrecision);
        // A successful tangent slide clears blocked; the clipped destination
        // proves the body's collision was consumed during this short tick.
        tests.Require(moved && constrained && !beforeEntry && touchingEntry,
            "A real boss-body stop point touches the Mario entry on its first active tick, including movement skin");
        auto outside = player; outside.fPositionX += rightX * .01f; outside.fPositionZ += rightZ * .01f;
        tests.Require(!CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, outside, contactTick),
            "Mario body contact does not admit a player still outside the contact margin");
        for (const auto action : interruptible) {
            player.eAction = action;
            tests.Require(CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, player, contactTick),
                "Mario body contact does not wait for ordinary actions, knockdown or fear to finish");
        }
        for (int invalid = 0; invalid < 9; ++invalid) {
            auto rejected = player;
            switch (invalid) {
            case 0: rejected.iCurrentHp = 0u; break;
            case 1: rejected.eAction = PLAYER_ACTION_STATE::FALLING; break;
            case 2: rejected.eAction = PLAYER_ACTION_STATE::GRABBED; break;
            case 3: rejected.bPatternBound = true; break;
            case 4: rejected.iAttachmentOwnerNetEntityId = 7u; break;
            case 5: rejected.bArenaEjectionActive = true; break;
            case 6: rejected.TriggerMove.isActive = true; break;
            case 7: rejected.iMarioStage = 1u; break;
            case 8: rejected.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL; break;
            }
            tests.Require(!CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(root, anchor, rejected, contactTick),
                "Mario entry requires Clown and preserves death, fall, attachment, binding and existing-transfer ownership");
        }
        // A non-chain entry honours the window end; a chain root (honorWindowEnd false) stays open past it.
        {
            auto ended = root; ended.LogicWindows.front().iDurationMs = 1000u;
            const auto afterEnd = CKoukuSaydonLogicRuntime::Ticks_FromMs(entry.iStartMs + 1000u);
            tests.Require(CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(ended, anchor, player, afterEnd) &&
                !CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(ended, anchor, player, afterEnd, true) &&
                CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(ended, anchor, player, afterEnd - 1u, true),
                "The room can require every Mario entry to close at its authored ENTER_AREA deadline");
        }
    }
    {
        BOSS_PATTERN_DEFINITION chainOnly{}, none{};
        BOSS_PATTERN_LOGIC_WINDOW chain{}; chain.eKind = BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT;
        chainOnly.LogicWindows.push_back(chain);
        tests.Require(CKoukuSaydonLogicRuntime::Find_MarioEntryResult(root) == &root.LogicWindows.front().OnSuccess.front() &&
            CKoukuSaydonLogicRuntime::Has_MarioEntry(root) && CKoukuSaydonLogicRuntime::Has_MarioEntry(chainOnly) &&
            !CKoukuSaydonLogicRuntime::Find_MarioEntryResult(chainOnly) && !CKoukuSaydonLogicRuntime::Has_MarioEntry(none),
            "A Mario entry pattern is a chain root or an ENTER_AREA window whose sole Success is MARIO_ENTER with regions");
    }
    const auto seedOwnedObject = [&](CGameRoom& room, const SERVER_PLAYER& player) {
        auto transaction = room.m_CombatObjectRuntime.Begin_Transaction();
        SERVER_COMBAT_OBJECT object{};
        object.iCombatObjectId = 993u; object.iSourceNetEntityId = player.iNetEntityId;
        object.iSourcePlayerId = player.iPlayerId; object.fRemainingMilliseconds = 1000.f;
        transaction.Objects.push_back(object);
        return room.m_CombatObjectRuntime.Commit(std::move(transaction));
    };
    for (std::uint8_t stage = 1u; stage <= 4u; ++stage) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        tests.Require(room->Is_Ready(), "Load actual Mario stage navigation for contact admission");
        for (const auto action : interruptible) {
            SERVER_PLAYER player{};
            player.iPlayerId = 994u; player.iNetEntityId = 995u; player.iCurrentHp = player.iMaximumHp = 100u;
            player.isCombatReady = true; player.eAction = action; player.iCurrentSkillId = 34010u;
        player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
            player.iActionStartTick = 1u; player.fActionElapsedSeconds = .1f; player.iComboStage = 2u;
            player.Projectiles.emplace_back(); player.hasMoveGoal = true; player.fKnockbackRemainingSeconds = 1.f;
            player.iKnockdownEndTick = player.iFearEndTick = 60u; player.strFearPresentationId = "contact-test";
            player.CooldownEndTickBySkillId.emplace(34010u, 600u);
            tests.Require(seedOwnedObject(*room, player), "Stage the entry action's delayed combat object");
            const bool entered = room->Enter_MarioFromPattern(player, stage);
            tests.Require(entered && player.iMarioStage == stage && player.eAction == PLAYER_ACTION_STATE::NONE &&
                player.iCurrentSkillId == INVALID_SKILL_ID && player.Projectiles.empty() && !player.hasMoveGoal &&
                player.fKnockbackRemainingSeconds == 0.f && !player.iKnockdownEndTick && !player.iFearEndTick &&
                player.strFearPresentationId.empty() && player.isCombatReady && player.iCurrentHp == 100u &&
                player.CooldownEndTickBySkillId.at(34010u) == 600u && room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
                "All four Mario stages interrupt the contacting action and cancel its objects only after admission");
            tests.Require(!room->Enter_MarioFromPattern(player, stage), "Committed Mario entry remains one-shot");
        }
    }
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        SERVER_PLAYER player{};
        player.iPlayerId = 996u; player.iNetEntityId = 997u; player.iCurrentHp = 100u; player.isCombatReady = true;
        player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
        player.eAction = PLAYER_ACTION_STATE::SKILL; player.iCurrentSkillId = 34010u;
        player.Projectiles.emplace_back(); player.fPositionX = 5.f; player.fPositionZ = 942.f;
        tests.Require(seedOwnedObject(*room, player), "Stage a live action before a rejected Mario layout");
        auto& groups = const_cast<std::vector<SPAWN_GROUP_DEFINITION>&>(room->m_SpawnGroupBootstrap.Get_Groups());
        const auto group = std::find_if(groups.begin(), groups.end(), [](const auto& row) { return row.strSpawnGroupId == "spawn.mario1.source"; });
        if (group == groups.end() || group->Waves.empty() || group->Waves.front().Entries.empty()) {
            tests.Require(false, "Resolve the actual Mario source anchor for failed-admission preservation");
        } else group->Waves.front().Entries.front().strAnchorId = "missing.mario.contact.test.anchor";
        tests.Require(!room->Enter_MarioFromPattern(player, 1u) && !player.iMarioStage &&
            player.eAction == PLAYER_ACTION_STATE::SKILL && player.iCurrentSkillId == 34010u &&
            player.Projectiles.size() == 1u && player.fPositionX == 5.f && player.fPositionZ == 942.f &&
            room->m_CombatObjectRuntime.Get_LiveObjects().size() == 1u,
            "A post-destination Mario layout failure preserves the live action, position and delayed combat object");
    }
    for (const auto& [id, stage] : std::array<std::pair<const char*, std::uint8_t>, 2u>{
        { {"Mario1_Trigger_1", 1u}, {"Mario4_Tigger_3", 4u} }}) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        const auto* trigger = room->Find_Placement(id);
        if (!trigger) { tests.Require(false, "Resolve the exact authored Mario lane trigger ID"); continue; }
        SERVER_PLAYER player{};
        player.iPlayerId = 998u; player.iNetEntityId = 999u; player.iCurrentHp = 100u; player.isCombatReady = true;
        player.iMarioStage = stage; player.bMarioRailReady = true; player.strMarioRailArrivalId = "preserved";
        player.fPositionX = trigger->fPositionX; player.fPositionY = trigger->fPositionY; player.fPositionZ = trigger->fPositionZ;
        for (const auto action : {PLAYER_ACTION_STATE::SKILL, PLAYER_ACTION_STATE::INTERACTION, PLAYER_ACTION_STATE::TRIGGER_MOVE}) {
            auto contacting = player; contacting.eAction = action; contacting.iCurrentSkillId = 34010u;
            contacting.TriggerMove.isActive = action == PLAYER_ACTION_STATE::TRIGGER_MOVE;
            contacting.TriggerMove.strSourcePlacementId.clear(); // Debug Mario jump has no trigger owner.
            tests.Require(room->Begin_MarioTriggerMove(*trigger, contacting, 100u) == SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED &&
                contacting.TriggerMove.isActive && contacting.iCurrentSkillId == INVALID_SKILL_ID &&
                contacting.iMarioStage == stage && contacting.bMarioRailReady && contacting.strMarioRailArrivalId == "preserved",
                "Actual Mario lane contacts interrupt skill, interaction and jump while preserving the current rail");
            contacting.TriggerMove.strSourcePlacementId = id;
            contacting.TriggerMove.fElapsedSeconds = .1f;
            tests.Require(room->Begin_MarioTriggerMove(*trigger, contacting, 101u) == SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED &&
                contacting.TriggerMove.fElapsedSeconds == .1f, "The same active Mario trigger cannot restart its transfer");
        }
        CServerTriggerSystem triggers;
        triggers.Set_WorldId(WORLD_ID::KOUKUSAYDON_ARENA);
        // Exercise automatic contact policy independently of the installed interaction flag.
        auto contactTrigger = *trigger;
        contactTrigger.requiresInteract = false;
        contactTrigger.isTriggerOnce = false;
        tests.Require(triggers.Initialize({contactTrigger}, status), "Load actual Mario geometry with repeatable automatic contact policy");
        player.bPatternBound = true;
        std::map<PLAYER_ID, SERVER_PLAYER> players{{player.iPlayerId, player}};
        std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
        std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
        const auto move = [&](const auto& box, auto& target, auto tick) { return room->Begin_MarioTriggerMove(box, target, tick); };
        // Mario crossings and exits are G boxes; the G press goes through the same room-owned admission.
        (void)triggers.Activate_Interact(player.iPlayerId, contactTrigger.strPlacementId, players, 102u, transfers, {}, move);
        tests.Require(!players.at(player.iPlayerId).TriggerMove.isActive, "An owned Mario G press preserves a temporary authority lock");
        players.at(player.iPlayerId).bPatternBound = false;
        (void)triggers.Activate_Interact(player.iPlayerId, contactTrigger.strPlacementId, players, 103u, transfers, {}, move);
        tests.Require(players.at(player.iPlayerId).TriggerMove.isActive,
            "A rejected Mario G press succeeds on the next press after unlock without leaving the box");
        auto unrelated = *trigger; unrelated.strPlacementId = "ordinary.movePlayer";
        tests.Require(room->Begin_MarioTriggerMove(unrelated, player, 104u) == SERVER_TRIGGER_MOVE_ENTRY_RESULT::USE_DEFAULT,
            "Mario action interruption does not change unrelated world movePlayer contracts");
    }
#endif
}

int LostArk::Server::CServerGameplayContractRunner::Run_KoukuDraft()
{
    TESTS tests{}; CGameplayCatalog catalog;
    if (!catalog.Load()) { std::cout << catalog.Get_Status() << '\n'; return 1; }
    Run_KoukuDraftContracts(tests, catalog);
    std::cout << "failures : " << tests.failures << '\n';
    return tests.failures == 0 ? 0 : 1;
}

int LostArk::Server::CServerGameplayContractRunner::Run_KoukuProductOnly()
{
    int result = 1;
    const auto execute = [](void* output) {
        std::cout << std::unitbuf;
        TESTS tests{}; CGameplayCatalog catalog;
        if (!catalog.Load()) { std::cout << catalog.Get_Status() << '\n'; return; }
        Run_KoukuSaydonLogicRuntimeContracts(tests, catalog);
        Run_KoukuMarioEntryContact(tests);
        Run_KoukuProduct(tests, catalog);
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(output) = tests.failures == 0 ? 0 : 1;
    };
    return Run_WithContractWorkerStack(execute, &result) ? result : 1;
}

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuDraftContracts(TESTS& tests, CGameplayCatalog& catalog)
{
    {
        KOUKU_RAID_GATE_DEFINITION flow;
        flow.Entries.resize(6u);
        flow.EntryGroups.push_back({"normal", 0u, 2u, 130u, false});
        flow.EntryGroups.push_back({"mechanic", 3u, 3u, std::nullopt, false});
        flow.EntryGroups.push_back({"final", 4u, 5u, 0u, false});
        tests.Require(flow.Resolve_NextCompletedEntry(0u, 600000u, 600000u, 160u) == 1u &&
            flow.Resolve_NextCompletedEntry(2u, 487501u, 600000u, 160u) == 0u,
            "HP normal group advances and repeats above its exact threshold");
        tests.Require(flow.Resolve_NextCompletedEntry(0u, 487500u, 600000u, 160u) == 3u &&
            flow.Resolve_NextCompletedEntry(3u, 100000u, 600000u, 160u) == 4u,
            "HP threshold enters its mechanic once after a completed pattern");
        flow.EntryGroups[0].bTransitionAtGroupEnd = true;
        tests.Require(flow.Resolve_NextCompletedEntry(0u, 1u, 600000u, 160u) == 1u &&
            flow.Resolve_NextCompletedEntry(2u, 1u, 600000u, 160u) == 3u,
            "Authored group-end boundary finishes the remaining normal patterns");
        tests.Require(flow.Resolve_NextCompletedEntry(5u, 1u, 600000u, 160u) == 4u &&
            flow.Resolve_NextCompletedEntry(5u, 0u, 600000u, 160u) == 6u &&
            flow.Resolve_NextCompletedEntry(9u, 0u, 0u, 0u) == 6u,
            "Final HP group repeats until death and bounds invalid cursors");
        flow.Entries.resize(10u);
        flow.EntryGroups = {{"normal130", 0u, 1u, 130u, false}, {"mechanic130", 2u, 2u, std::nullopt, false},
            {"normal110", 3u, 4u, 110u, true}, {"mechanic110", 5u, 5u, std::nullopt, false},
            {"normal85", 6u, 7u, 85u, false}, {"mechanic85", 8u, 8u, std::nullopt, false}, {"final", 9u, 9u, 0u, false}};
        tests.Require(flow.Resolve_ReadyEntry(0u, 375000u, 600000u, 160u) == 2u &&
            flow.Resolve_NextCompletedEntry(2u, 375000u, 600000u, 160u) == 5u &&
            flow.Resolve_NextCompletedEntry(2u, 262500u, 600000u, 160u) == 5u &&
            flow.Resolve_NextCompletedEntry(5u, 262500u, 600000u, 160u) == 8u &&
            flow.Resolve_NextCompletedEntry(8u, 262500u, 600000u, 160u) == 9u,
            "Crossing multiple HP thresholds skips unstarted normal groups and preserves each mechanic in order");
        tests.Require(flow.Resolve_NextCompletedEntry(3u, 262500u, 600000u, 160u) == 4u &&
            flow.Resolve_NextCompletedEntry(4u, 262500u, 600000u, 160u) == 5u,
            "Already running group-end repeats finish their remaining patterns before the mechanic");
    }
    {
        namespace fs = std::filesystem;
        std::vector<wchar_t> buffer(32768u); fs::path dataRoot;
        const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer.data(), static_cast<DWORD>(buffer.size()));
        if (configured && configured < buffer.size()) dataRoot = buffer.data();
        else { GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size())); dataRoot = fs::path(buffer.data()).parent_path().parent_path() / L"DataFiles"; }
        std::ifstream input(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", std::ios::binary);
        std::string rows, allKoukuRows, line;
        // One selected audition carries only its dependencies. P1's real stagger
        // success references P4; the complete published product is a different contract.
        const std::set<std::string> selectedIds{"KOUKUSAYDON_G1_PATTERN_1", "KOUKUSAYDON_G1_PATTERN_4"};
        const auto* patterns = catalog.Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1");
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            const auto tab = line.find('\t'), next = line.find('\t', tab + 1u);
            if (tab == std::string::npos) continue;
            const auto kind = line.substr(0u, tab), owner = line.substr(tab + 1u, next - tab - 1u);
            bool keep = owner == "ENCOUNTER_KOUKUSAYDON_G1" &&
                (kind.starts_with("PATTERN") || kind == "KOUKUMADNESS" || kind == "KOUKUSAYDONPRODUCTREVISION" || kind == "ENCOUNTERINTRO");
            if (kind == "RAIDGATE" || kind == "RAIDFLOWSTEP" || kind == "RAIDFLOWGROUP" || kind == "RAIDARRIVAL" || kind == "RAIDBINGOSPECIAL") keep = true;
            if (kind == "PATTERNTARGET" && patterns) keep = std::any_of(patterns->begin(), patterns->end(), [&](const auto& p) { return p.strPatternId == owner; });
            if (kind == "PATTERNBUNDLE" || kind == "PATTERNBUNDLEMEMBER") {
                const auto* bundle = catalog.Find_BossPatternBundle(owner);
                keep = bundle && bundle->strEncounterId == "ENCOUNTER_KOUKUSAYDON_G1";
            }
            if (keep) allKoukuRows += line + "\n";
            bool selected = owner == "ENCOUNTER_KOUKUSAYDON_G1" &&
                (kind == "KOUKUMADNESS" || kind == "KOUKUSAYDONPRODUCTREVISION");
            if (owner == "ENCOUNTER_KOUKUSAYDON_G1" && kind.starts_with("PATTERN") && next != std::string::npos) {
                const auto third = line.find('\t', next + 1u);
                selected = selectedIds.contains(line.substr(next + 1u, third - next - 1u));
            }
            if (kind == "PATTERNTARGET") selected = selectedIds.contains(owner);
            if (selected) rows += line + "\n";
        }
        rows += "PATTERNSEQUENCE\tENCOUNTER_KOUKUSAYDON_G1\tKOUKUSAYDON_G1_PLAY_ALL\tORDERED_ONCE_THEN_IDLE\t100\t2\n"
            "PATTERNSEQUENCESTEP\tENCOUNTER_KOUKUSAYDON_G1\tKOUKUSAYDON_G1_PLAY_ALL\t0\tKOUKUSAYDON_G1_PATTERN_1\t100\n"
            "PATTERNSEQUENCESTEP\tENCOUNTER_KOUKUSAYDON_G1\tKOUKUSAYDON_G1_PLAY_ALL\t1\tKOUKUSAYDON_G1_PATTERN_4\t0\n";
        const DWORD candidateLength = GetEnvironmentVariableW(L"LOSTARK_KOUKU_DRAFT_TEST_ROWS", buffer.data(), static_cast<DWORD>(buffer.size()));
        if (candidateLength && candidateLength < buffer.size()) {
            std::ifstream candidate(fs::path(buffer.data()), std::ios::binary);
            rows.assign(std::istreambuf_iterator<char>(candidate), std::istreambuf_iterator<char>());
            tests.Require(!rows.empty(), "Read the actual prepared memory draft candidate for native admission");
        }
        const auto directory = fs::temp_directory_path() / (L"LostArkDraftContract-" + std::to_wstring(GetCurrentProcessId()));
        std::error_code error; fs::create_directories(directory, error); const auto path = directory / L"Draft.rows";
        std::string status;
        const auto hashRows = [&](const std::string& bytes, GameplayDataRevision& hash) {
            { std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(bytes.data(), bytes.size()); }
            return !error && CServerApp::Hash_GameplayFileForAdmission(path, hash, status);
        };
        GameplayDataRevision rowsHash; CGameplayCatalog draft;
        const bool admitted = hashRows(rows, rowsHash) && draft.Load_DraftKoukuProduct(catalog, rows, rowsHash);
        if (!admitted) std::cout << "[DraftAdmission] " << draft.Get_Status() << '\n';
        tests.Require(admitted && draft.Has_SameNonKoukuGameplay(catalog) && draft.Get_ActiveRevision() == catalog.Get_ActiveRevision(),
            "Memory draft reparses only Kouku rows against the active player/boss balance without replacing its gameplay identity");
        if (admitted) {
            const auto originalSource = CKoukuSaydonBrain::Resolve_ProductSourceRevision(draft);
            if (allKoukuRows.size() > MAX_KOUKUSAYDON_DRAFT_BYTES) {
                GameplayDataRevision productRowsHash;
                tests.Require(hashRows(allKoukuRows, productRowsHash) &&
                    !draft.Load_DraftKoukuProduct(catalog, allKoukuRows, productRowsHash) &&
                    draft.Has_SameNonKoukuGameplay(catalog) &&
                    CKoukuSaydonBrain::Resolve_ProductSourceRevision(draft) == originalSource,
                    "A complete product above 16 MiB rejects as a single draft and preserves the admitted selection");
            }
            const auto madnessStart = rows.find("KOUKUMADNESS\t");
            const auto madnessEnd = rows.find('\n', madnessStart);
            tests.Require(madnessStart != std::string::npos && madnessEnd != std::string::npos, "Resolve actual Madness bootstrap row for admission variants");
            if (madnessStart != std::string::npos && madnessEnd != std::string::npos) {
                const auto validateMadness = [&](const std::string& fields, const bool expected) {
                    auto candidateRows = rows;
                    candidateRows.replace(madnessStart, madnessEnd - madnessStart,
                        "KOUKUMADNESS\tENCOUNTER_KOUKUSAYDON_G1\t100\t15000" + fields);
                    GameplayDataRevision candidateHash; CGameplayCatalog candidate;
                    const bool accepted = hashRows(candidateRows, candidateHash) && candidate.Load_DraftKoukuProduct(catalog, candidateRows, candidateHash);
                    tests.Require(accepted == expected, "v37 Madness admits exactly its 4/12-column variants and rejects malformed tuning");
                    if (accepted) {
                        const auto* source = catalog.Find_KoukuMadnessPolicy("ENCOUNTER_KOUKUSAYDON_G1");
                        const auto* active = candidate.Find_KoukuMadnessPolicy("ENCOUNTER_KOUKUSAYDON_G1");
                        tests.Require(source && active && active->iDamageGainPercent == source->iDamageGainPercent &&
                            active->iBallMultiplierPercent == source->iBallMultiplierPercent && active->iDollMultiplierPercent == source->iDollMultiplierPercent,
                            "Sequence draft preserves admitted Balance Test multipliers instead of replacing them with draft defaults");
                    }
                };
                validateMadness("", true);
                validateMadness("\t123\t10\t250\t2.5\t10\t350\t4\t1000", true);
                validateMadness("\t123\t10\t250\t2.5\t10\t350\t4", false);
                validateMadness("\t123\t10\t250\t2.5\t10\t350\t4\t1000\t0", false);
                validateMadness("\t123\t10\t250\tnan\t10\t350\t4\t1000", false);
                validateMadness("\t10001\t10\t250\t2\t10\t350\t4\t1000", false);
            }
            auto wrongHash = rowsHash; wrongHash.Bytes.front() ^= 1u;
            const std::string wrongDomain = rows + "PLAYER\tnot.a.kouku.row\n";
            GameplayDataRevision wrongDomainHash;
            const bool rejectedChecksum = !draft.Load_DraftKoukuProduct(catalog, rows, wrongHash);
            const bool rejectedDomain = hashRows(wrongDomain, wrongDomainHash) && !draft.Load_DraftKoukuProduct(catalog, wrongDomain, wrongDomainHash);
            tests.Require(rejectedChecksum && rejectedDomain && CKoukuSaydonBrain::Resolve_ProductSourceRevision(draft) == originalSource && draft.Has_SameNonKoukuGameplay(catalog),
                "Incorrect checksum and out-of-domain rows reject transactionally and preserve the previous draft generation");
            if (const auto* gate = draft.Find_KoukuRaidGate("GATE1"); gate && !gate->Entries.empty() && gate->strLoopStartEntryId.empty())
            {
                std::istringstream inputGroups(rows); std::string baseRows, row;
                while (std::getline(inputGroups, row))
                    if (!row.starts_with("RAIDFLOWGROUP\tGATE1\t")) baseRows += row + "\n";
                const std::string prefix = "RAIDFLOWGROUP\tGATE1\t0\tcontract.hp.group\t" + gate->Entries.front().strEntryId + "\t";
                const std::string validRows = baseRows + prefix + gate->Entries.back().strEntryId + "\t130\tPATTERN_END\n";
                GameplayDataRevision groupHash; CGameplayCatalog grouped;
                const bool groupsAdmitted = hashRows(validRows, groupHash) && grouped.Load_DraftKoukuProduct(catalog, validRows, groupHash);
                const auto* loaded = groupsAdmitted ? grouped.Find_KoukuRaidGate("GATE1") : nullptr;
                tests.Require(loaded && loaded->EntryGroups.size() == 1u && loaded->EntryGroups.front().iFirstEntry == 0u &&
                    loaded->EntryGroups.front().iLastEntry + 1u == loaded->Entries.size() && loaded->EntryGroups.front().RepeatUntilHealthBars == 130u,
                    "Native catalog resolves HP group stable endpoints against saved flow rows");
                const std::string invalidRows = baseRows + prefix + "missing.entry\t130\tPATTERN_END\n";
                const bool groupsRejected = hashRows(invalidRows, groupHash) && !grouped.Load_DraftKoukuProduct(catalog, invalidRows, groupHash);
                loaded = grouped.Find_KoukuRaidGate("GATE1");
                tests.Require(groupsAdmitted && groupsRejected && loaded && loaded->EntryGroups.size() == 1u &&
                    grouped.Has_SameNonKoukuGameplay(catalog), "Invalid HP group reference preserves the last admitted catalog");
            }
            C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK chunk;
            chunk.iRequestSequence = 1u; chunk.iTotalBytes = static_cast<std::uint32_t>(rows.size()); chunk.RowsRevision = rowsHash;
            chunk.strBytes = rows.substr(0u, (std::min)(MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES, (std::max)(std::size_t{1u}, rows.size() / 2u)));
            CPacketWriter writer; bool codec = Write_Message(writer, chunk);
            C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK decoded; CPacketReader reader(writer.Get_Buffer());
            codec = codec && Read_Message(reader, decoded) && decoded.strBytes == chunk.strBytes && decoded.RowsRevision == rowsHash && reader.Get_RemainingSize() == 0u;
            auto oversized = chunk; oversized.iTotalBytes = static_cast<std::uint32_t>(MAX_KOUKUSAYDON_DRAFT_BYTES + 1u); CPacketWriter invalidWriter;
            tests.Require(codec && !Write_Message(invalidWriter, oversized), "Draft chunks round-trip exact bytes/hash and reject an oversized declared upload");
            auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
            constexpr SESSION_ID session = 8133u;
            room->m_PlayerIdBySessionId[session] = 8134u;
            room->Handle_KoukuSaydonDraftChunk(session, chunk);
#ifndef _DEBUG
            tests.Require(room->m_KoukuDraftUploads.empty(), "Release discards debug draft chunks without staging gameplay");
#else
            auto newer = chunk; newer.iRequestSequence = 2u;
            room->Handle_KoukuSaydonDraftChunk(session, newer);
            auto stale = chunk; stale.iOffsetBytes = static_cast<std::uint32_t>(chunk.strBytes.size());
            stale.strBytes = rows.substr(stale.iOffsetBytes, MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES);
            room->Handle_KoukuSaydonDraftChunk(session, stale);
            tests.Require(room->m_KoukuDraftUploads.contains(session) && room->m_KoukuDraftUploads.at(session).iRequestSequence == 2u,
                "Late chunks from an older request cannot discard the newer staged draft");
            room->m_KoukuDraftUploads.erase(session);
            room->Handle_KoukuSaydonDraftChunk(session, chunk);
            auto gap = chunk; gap.iOffsetBytes = static_cast<std::uint32_t>(chunk.strBytes.size() + 1u);
            gap.strBytes = rows.substr(gap.iOffsetBytes, MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES);
            room->Handle_KoukuSaydonDraftChunk(session, gap);
            tests.Require(room->m_KoukuDraftUploads.empty(), "A non-contiguous draft upload is discarded before admission");
            room->m_eWorldId = WORLD_ID::BERN;
            room->Handle_KoukuSaydonDraftChunk(session, chunk);
            tests.Require(room->m_KoukuDraftUploads.empty(), "Draft upload ignores a different joined world");
            room->m_eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
            room->Handle_KoukuSaydonDraftChunk(session, chunk);
            room->m_KoukuDraftUploads.at(session).iStartedAtMs = 0u;
            auto nextChunk = chunk; nextChunk.iOffsetBytes = static_cast<std::uint32_t>(chunk.strBytes.size());
            nextChunk.strBytes = rows.substr(nextChunk.iOffsetBytes, MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES);
            room->Handle_KoukuSaydonDraftChunk(session, nextChunk);
            tests.Require(room->m_KoukuDraftUploads.empty(), "Expired draft uploads cannot resume from a later chunk");
            for (std::size_t offset = 0u; offset < rows.size(); offset += MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES) {
                chunk.iOffsetBytes = static_cast<std::uint32_t>(offset); chunk.strBytes = rows.substr(offset, MAX_KOUKUSAYDON_DRAFT_CHUNK_BYTES);
                room->Handle_KoukuSaydonDraftChunk(session, chunk);
            }
            const auto* definitions = draft.Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1");
            const BOSS_PATTERN_DEFINITION* selected = nullptr;
            if (definitions) for (const auto& pattern : *definitions)
                if (!pattern.strGateId.empty() && !pattern.strTargetBossPlacementId.empty()) { selected = &pattern; break; }
            auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
            const auto* placement = selected ? room->Find_Placement(selected->strTargetBossPlacementId) : nullptr;
            bool built = placement && room->Build_WorldEntity(*placement, room->m_iNextNetEntityId++, *boss);
            if (built) {
                std::erase_if(room->m_WorldEntities, [&](const auto& entity) { return entity.strPlacementId == placement->strPlacementId; });
                room->m_WorldEntities.push_back(*boss);
                C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request;
                request.iRequestSequence = 1u; request.strPatternId = selected->strPatternId;
                request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; request.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
                request.Scope.strGateId = selected->strGateId; request.Scope.strBossPlacementId = placement->strPlacementId;
                request.Scope.strBossArchetypeId = placement->strArchetypeId; request.Scope.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
                request.Scope.iExpectedSourceRevision = originalSource; request.Scope.DraftRowsRevision = rowsHash;
                const auto priorPublished = room->m_pKoukuPublishedProductGeneration;
                S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
                const auto verdict = room->Evaluate_KoukuSaydonPatternAudition(session, request, result);
                if (verdict != KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED) std::cout << "[DraftRun] " << result.strReason << '\n';
                tests.Require(verdict == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED && room->m_KoukuDraftUploads.empty() &&
                    room->m_KoukuSaydonPatternAudition.pProductGeneration && room->m_pKoukuPublishedProductGeneration == priorPublished && result.Scope.DraftRowsRevision == rowsHash,
                    "Draft admission pins one run, consumes its upload, echoes exact content identity and preserves the published catalog");
                S2C_KOUKUSAYDON_BUNDLE_STATE state, decodedState; CPacketWriter stateWriter;
                bool stateOk = room->Build_KoukuBundleState(state) && Write_Message(stateWriter, state);
                CPacketReader stateReader(stateWriter.Get_Buffer());
                stateOk = stateOk && Read_Message(stateReader, decodedState) && decodedState.DraftRowsRevision == rowsHash;
                tests.Require(stateOk, "Persistent run snapshots bind draft presentation to its exact uploaded hash");
                tests.Require(room->Evaluate_KoukuSaydonPatternAudition(session, request, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED,
                    "An exact draft request retry reuses its receipt without requiring another upload");
                request.Scope.DraftRowsRevision = wrongHash;
                tests.Require(room->Evaluate_KoukuSaydonPatternAudition(session, request, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST,
                    "A reused draft request sequence cannot substitute a different content hash");
                room->Clear_KoukuSaydonPatternAudition();
            }
            tests.Require(built, "Draft contract resolves the selected pattern's real admitted gate boss placement");
#endif
        }
        fs::remove(path, error); fs::remove(directory, error);
    }
}

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuProduct(TESTS& tests, CGameplayCatalog& catalog)
{
    Run_KoukuDraftContracts(tests, catalog);
#ifdef _DEBUG
    {
        namespace fs = std::filesystem;
        std::vector<wchar_t> buffer(32768u); fs::path dataRoot;
        const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer.data(), static_cast<DWORD>(buffer.size()));
        if (configured && configured < buffer.size()) dataRoot = buffer.data();
        else { GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size())); dataRoot = fs::path(buffer.data()).parent_path().parent_path() / L"DataFiles"; }
        std::ifstream input(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", std::ios::binary);
        std::string base((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (!base.empty() && base.back() != '\n') base += '\n';
        const auto baseRows = static_cast<std::uint32_t>(std::count(base.begin(), base.end(), '\n') - 1);
        const auto setCount = [](std::string& bytes, const std::uint32_t count) {
            const auto end = bytes.find('\n'), column = bytes.rfind('\t', end);
            bytes.replace(column + 1u, end - column - 1u, std::to_string(count));
        };
        const auto directory = fs::temp_directory_path() / (L"LostArkBootstrapCapacity-" + std::to_wstring(GetCurrentProcessId()));
        std::error_code error; fs::create_directories(directory, error);
        const auto path = directory / L"Gameplay.bootstrap";
        auto generation = std::make_unique<CGameplayCatalog>(catalog);
        const auto load = [&](const std::string& bytes) {
            { std::ofstream file(path, std::ios::binary | std::ios::trunc); file.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); }
            GameplayDataRevision hash; std::string status;
            return CServerApp::Hash_GameplayFileForAdmission(path, hash, status) && generation->Load_FromBootstrap(fs::canonical(path), hash, hash);
        };
        tests.Require(!base.empty() && baseRows < GAMEPLAY_BOOTSTRAP_MAX_ROWS && !error,
            "Bootstrap capacity fixture starts from the valid published product");
        if (!base.empty() && baseRows < GAMEPLAY_BOOTSTRAP_MAX_ROWS && !error)
        {
            std::string rows = base;
            for (auto index = baseRows; index < GAMEPLAY_BOOTSTRAP_MAX_ROWS; ++index)
                rows += "DAMAGE\tcontract.capacity." + std::to_string(index) + "\t1\n";
            setCount(rows, GAMEPLAY_BOOTSTRAP_MAX_ROWS);
            tests.Require(load(rows), "The complete catalog admits exactly 131072 valid bounded rows");
            auto accepted = generation->Get_ActiveRevision();
            rows += "DAMAGE\tcontract.capacity.overflow\t1\n";
            setCount(rows, GAMEPLAY_BOOTSTRAP_MAX_ROWS + 1u);
            tests.Require(!load(rows) && generation->Get_ActiveRevision() == accepted &&
                generation->Find_DamageRatePercent("contract.capacity." + std::to_string(baseRows)) == 1u,
                "One row beyond the shared bound is rejected without replacing the admitted catalog");
            rows.clear(); rows.shrink_to_fit();

            std::string bytes = base + "DAMAGE\tcontract.capacity.bytes\t1\n";
            setCount(bytes, baseRows + 1u);
            // Leading zeroes keep this numeric field equal to one while exercising the exact byte boundary.
            bytes.insert(bytes.size() - 2u, GAMEPLAY_BOOTSTRAP_MAX_BYTES - bytes.size(), '0');
            tests.Require(bytes.size() == GAMEPLAY_BOOTSTRAP_MAX_BYTES && load(bytes),
                "The complete catalog admits an exact 64 MiB valid UTF-8 bootstrap");
            accepted = generation->Get_ActiveRevision();
            bytes += '\n';
            tests.Require(!load(bytes) && generation->Get_ActiveRevision() == accepted &&
                generation->Find_DamageRatePercent("contract.capacity.bytes") == 1u &&
                generation->Get_Status().find("byte size") != std::string::npos,
                "One byte beyond 64 MiB is rejected before admission and preserves the prior catalog");
            bytes = base + "DAMAGE\tcontract.capacity.trailing\t1\n";
            tests.Require(!load(bytes) && generation->Get_ActiveRevision() == accepted,
                "Raising capacity still rejects undeclared trailing rows and preserves the catalog");
            bytes = base; setCount(bytes, baseRows + 1u);
            tests.Require(!load(bytes) && generation->Get_ActiveRevision() == accepted,
                "Raising capacity still rejects a truncated declared row set and preserves the catalog");
            const auto* bingo = catalog.Find_KoukuRaidGate("BINGO");
            const std::string specialRow = bingo ? "RAIDBINGOSPECIAL\tBINGO\t" + bingo->strBingoSpecialPatternId : std::string{};
            const auto specialAt = specialRow.empty() ? std::string::npos : base.find(specialRow);
            tests.Require(bingo && !bingo->strBingoSpecialPatternId.empty() && specialAt != std::string::npos,
                "Published Bingo flow pins one explicit stable special Parent");
            if (specialAt != std::string::npos) {
                const auto specialEnd = base.find('\n', specialAt);
                auto missing = base; missing.erase(specialAt, specialEnd + 1u - specialAt); setCount(missing, baseRows - 1u);
                tests.Require(!load(missing) && generation->Get_ActiveRevision() == accepted,
                    "Missing Bingo special binding rejects and preserves the admitted catalog");
                for (const auto& row : {std::string{"RAIDBINGOSPECIAL\tGATE3\t"} + bingo->strBingoSpecialPatternId,
                        std::string{"RAIDBINGOSPECIAL\tBINGO\tmissing.special.parent"},
                        std::string{"RAIDBINGOSPECIAL\tBINGO\tKOUKUSAYDON_G1_PATTERN_94"}}) {
                    auto invalid = base; invalid.replace(specialAt, specialEnd - specialAt, row);
                    tests.Require(!load(invalid) && generation->Get_ActiveRevision() == accepted,
                        "Wrong gate, missing pattern and non-Parent special references reject transactionally");
                }
                auto duplicate = base + specialRow + "\n"; setCount(duplicate, baseRows + 1u);
                tests.Require(!load(duplicate) && generation->Get_ActiveRevision() == accepted,
                    "Duplicate Bingo special owner rows reject without replacing the pinned generation");
            }
        }
        fs::remove_all(directory, error);
    }
    // These use the published composition, real room and the same command
    // admission/terminal receipts consumed by Complete Play and its F1 test.
    enum class MARIO_SCENARIO { NO_ENTRY, RETURN_AFTER_CHAIN, RETURN_EARLY, PARTY, DEATH, DISCONNECT, LAST_TICK_ENTRY, NO_FOLLOWUP, CANCEL };
    std::uint32_t noEntryCompletionTick = 0u;
    const std::set<std::string> marioCandidates{
        "KOUKUSAYDON_G1_PATTERN_38", "KOUKUSAYDON_G1_PATTERN_39", "KOUKUSAYDON_G1_PATTERN_40",
        "KOUKUSAYDON_G1_PATTERN_43", "KOUKUSAYDON_G1_PATTERN_52", "KOUKUSAYDON_G1_PATTERN_46"};
    const auto marioScenario = [&](const MARIO_SCENARIO scenario, const std::uint32_t seed = 71023u, const bool immediateRevive = false) {
        const bool noFollowup = scenario == MARIO_SCENARIO::NO_FOLLOWUP || scenario == MARIO_SCENARIO::CANCEL;
        const bool enterPortal = scenario != MARIO_SCENARIO::NO_ENTRY && !noFollowup;
        const bool interruptedScenario = scenario == MARIO_SCENARIO::DEATH || scenario == MARIO_SCENARIO::DISCONNECT;
        const std::uint8_t marioStage = scenario == MARIO_SCENARIO::DEATH ? 2u : 3u;
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        const auto* placement = room->Find_Placement("boss.koukusaydon.g3.saydon");
        SERVER_WORLD_ENTITY boss{};
        if (!room->Is_Ready() || !placement || !room->Build_WorldEntity(*placement, room->m_iNextNetEntityId++, boss)) {
            tests.Require(false, "Load Mario scenario from the published Gate 3 placement"); return std::vector<std::string>{};
        }
        // Fix the test's two-completion/six-candidate scenario explicitly. Live authoring may
        // select a different count or pool without changing this optional runtime contract.
        {
            auto generation = std::make_shared<CGameplayCatalog>(room->m_GameplayCatalog.Active());
            auto* definitions = const_cast<std::vector<BOSS_PATTERN_DEFINITION>*>(generation->Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1"));
            if (definitions) for (auto& definition : *definitions) if (definition.strPatternId == "KOUKUSAYDON_G1_PATTERN_34")
                for (auto& window : definition.LogicWindows) if (window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT) {
                    window.iCompletionCount = 2u;
                    window.PatternIds = {"KOUKUSAYDON_G1_PATTERN_38", "KOUKUSAYDON_G1_PATTERN_39", "KOUKUSAYDON_G1_PATTERN_40",
                        "KOUKUSAYDON_G1_PATTERN_43", "KOUKUSAYDON_G1_PATTERN_52", "KOUKUSAYDON_G1_PATTERN_46"};
                    window.OnSuccess.clear();
                    if (!noFollowup) {
                        BOSS_PATTERN_LOGIC_RESULT followup{};
                        followup.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN;
                        followup.strPatternId = "KOUKUSAYDON_G1_PATTERN_33";
                        window.OnSuccess = {followup};
                    }
                }
            // This fixture exercises optional success with no entrant, independently of
            // a designer-authored failure policy on the current product's entry window.
            if (definitions) for (auto& definition : *definitions) if (definition.strPatternId == "KOUKUSAYDON_G1_PATTERN_34")
                for (auto& window : definition.LogicWindows) if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA) {
                    window.OnTimeout.clear();
                    window.iDurationMs = 100000u;
                    definition.iTimelineDurationMs = (std::max)(definition.iTimelineDurationMs, window.iStartMs + window.iDurationMs);
                }
            room->m_pKoukuPublishedProductGeneration = std::move(generation);
        }
        const auto bossId = boss.iNetEntityId;
        room->m_WorldEntities.push_back(boss);
        SERVER_PLAYER player{};
        player.iPlayerId = 923u; player.iNetEntityId = 924u; player.iSessionId = 925u;
        player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
        player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
        player.fPositionX = boss.fPositionX; player.fPositionY = boss.fPositionY; player.fPositionZ = boss.fPositionZ;
        if (!enterPortal || scenario == MARIO_SCENARIO::LAST_TICK_ENTRY) player.fPositionX += 25.f;
        room->m_Players.emplace(player.iPlayerId, player);
        if (interruptedScenario) {
            // A disconnected transport may already be expired; the room bindings still own departure cleanup.
            room->m_Sessions.emplace(player.iSessionId, std::weak_ptr<CClientSession>{});
            room->m_PlayerIdBySessionId.emplace(player.iSessionId, player.iPlayerId);
            room->m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
        }
        if (scenario == MARIO_SCENARIO::PARTY) {
            auto other = player; other.iPlayerId = 926u; other.iNetEntityId = 927u; other.iSessionId = 928u;
            other.fPositionX += 25.f; room->m_Players.emplace(other.iPlayerId, other);
        }
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
        request.iRequestSequence = 1u;
        request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
        request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
        request.Scope.strGateId = "GATE3"; request.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
        request.Scope.strBossPlacementId = "boss.koukusaydon.g3.saydon";
        request.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON";
        request.Scope.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        request.strPatternId = "KOUKUSAYDON_G1_PATTERN_34";
        request.iMarioTestStartStage = marioStage; request.iMarioTestSeed = seed;
        CPacketWriter requestWriter;
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST decoded{};
        bool codec = Write_Message(requestWriter, request);
        CPacketReader requestReader(requestWriter.Get_Buffer());
        codec = codec && Read_Message(requestReader, decoded) && decoded.iMarioTestStartStage == marioStage && decoded.iMarioTestSeed == seed;
        auto oldPayload = requestWriter.Get_Buffer(); oldPayload.erase(oldPayload.begin(), oldPayload.begin() + 5u);
        CPacketReader oldReader(oldPayload);
        codec = codec && !Read_Message(oldReader, decoded) && decoded.iMarioTestStartStage == marioStage;
        auto invalid = request; invalid.iMarioTestStartStage = 5u; CPacketWriter invalidWriter;
        tests.Require(codec && !Write_Message(invalidWriter, invalid), "Round-trip Mario test state and reject old or invalid request payload transactionally");
        S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
        const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED == room->Evaluate_KoukuSaydonPatternAudition(925u, request, result);
        if (!queued) { std::cerr << "Mario admission: " << result.strReason << '\n'; tests.Require(false, "Admit actual Mario phase 1 with its dependency closure"); return std::vector<std::string>{}; }
        std::vector<std::string> selected;
        std::uint32_t completed = 0u, phase2Tick = 0u, completionTick = 0u, returnTick = 0u;
        bool bounded = true, retained = false, wire = false, returnStarted = false, aborted = false, waited = false;
        bool entryCommitted = false, landedBeforeCount = false, terminalCompleted = false, mechanicFailed = false;
        std::uint32_t finalChildStartTick = 0u, finalChildDurationTicks = 0u;
        for (std::uint32_t tick = 1u; tick < 2400u; ++tick) {
            room->m_iServerTick = tick;
            if (returnStarted && room->m_Players.contains(player.iPlayerId) && room->m_Players.at(player.iPlayerId).TriggerMove.isActive)
                room->Update_Players(1.f / 30.f);
            if (scenario == MARIO_SCENARIO::LAST_TICK_ENTRY && tick == noEntryCompletionTick &&
                !room->m_KoukuSaydonPatternAudition.Members.empty()) {
                const auto& anchor = room->m_KoukuSaydonPatternAudition.Members.front().MarioEntryAnchor;
                if (anchor) {
                    auto& entrant = room->m_Players.at(player.iPlayerId);
                    entrant.fPositionX = anchor->fPositionX; entrant.fPositionY = anchor->fPositionY; entrant.fPositionZ = anchor->fPositionZ;
                }
            }
            room->Prepare_KoukuAuditionTick(tick);
            auto* live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            if (room->m_KoukuSaydonPatternAudition.Members.empty()) {
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                if (scenario == MARIO_SCENARIO::DISCONNECT) {
                    aborted = room->Is_Ready() && room->m_Players.empty() && room->m_Sessions.empty() &&
                        room->m_PlayerIdBySessionId.empty() && room->m_PlayerIdByEntityId.empty() && !live &&
                        room->m_iNextMarioEntryStage == 1u && room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty() &&
                        receipt == room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                        room->Get_Status() == "Replayable arena reset after the room became empty";
                } else {
                    aborted = scenario == MARIO_SCENARIO::DEATH && room->Get_Status().find("entrant died") != std::string::npos &&
                        receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() && receipt->second.LastLifecycle &&
                        receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED &&
                        receipt->second.LastLifecycle->strReason == room->Get_Status();
                }
                bounded = bounded && aborted; break;
            }
            if (scenario == MARIO_SCENARIO::DEATH &&
                room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) {
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                mechanicFailed = live && live->iNetEntityId == bossId && live->iCurrentHp &&
                    receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() && receipt->second.LastLifecycle &&
                    receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED &&
                    !room->m_KoukuSaydonPatternAudition.Members.front().bCompletionChainSuccessQueued;
                bounded = bounded && mechanicFailed;
                break;
            }
            if (!live || !room->Update_KoukuSaydonBoss(*live, tick)) { bounded = false; break; }
            if (room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) {
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                terminalCompleted = noFollowup && receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                    receipt->second.LastLifecycle && receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED;
                bounded = bounded && terminalCompleted && completed == 1u && finalChildDurationTicks &&
                    tick + 1u >= finalChildStartTick + finalChildDurationTicks && live->strPatternId.empty() && room->m_PendingKoukuMarioEntries.empty();
                if (terminalCompleted) { completed = 2u; completionTick = tick; }
                break;
            }
            room->Commit_KoukuMarioEntries();
            // Committing the entry can append Mario objects and reallocate world entities.
            live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            if (!live) { bounded = false; break; }
            const auto& member = room->m_KoukuSaydonPatternAudition.Members.front();
            if (noFollowup && member.bCompletionChainStarted && member.iCompletionChainCompleted == 1u &&
                member.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE && !finalChildStartTick) {
                finalChildStartTick = live->iPatternStartTick;
                std::string childStatus;
                const auto* child = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*room->Resolve_KoukuProductCatalog(), live->strPatternId, childStatus);
                if (child) {
                    std::uint32_t totalMs = 0u;
                    for (const auto& stage : child->Stages) {
                        totalMs += stage.iDurationMs;
                        finalChildDurationTicks += CKoukuSaydonLogicRuntime::Ticks_FromMs(stage.iDurationMs);
                    }
                    if (child->bFixedTimelineClock) finalChildDurationTicks = CKoukuSaydonLogicRuntime::Ticks_FromMs(totalMs);
                }
            }
            entryCommitted = entryCommitted || member.bMarioEntryConsumed;
            if (member.bCompletionChainStarted && selected.empty())
                selected.assign(member.PatternIds.begin() + member.iCompletionChainFirstIndex,
                    member.PatternIds.begin() + member.iCompletionChainFirstIndex + member.iCompletionChainCount);
            bounded = bounded && member.iCompletionChainCompleted >= completed && member.iCompletionChainCompleted <= completed + 1u;
            completed = member.iCompletionChainCompleted;
            if (completed == 2u && !completionTick) completionTick = tick;
            if (member.bMarioReturnCompleted && !returnTick) { returnTick = tick; landedBeforeCount = completed < 2u; }
            if (completed < 2u && member.bCompletionChainStarted) {
                bounded = bounded && std::find(member.PatternIds.begin(), member.PatternIds.end(), "KOUKUSAYDON_G1_PATTERN_33") == member.PatternIds.end();
                retained = retained || member.MarioEntryAnchor.has_value();
                S2C_KOUKUSAYDON_BUNDLE_STATE state{}, decodedState{};
                CPacketWriter writer;
                if (room->Build_KoukuBundleState(state) && Write_Message(writer, state)) {
                    CPacketReader reader(writer.Get_Buffer());
                    wire = Read_Message(reader, decodedState) && decodedState.Members.front().iBossNetEntityId == bossId &&
                        (member.ePhase != CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE ||
                         decodedState.Members.front().iStartTick == live->iPatternStartTick) &&
                        (member.bMarioEntryConsumed ? decodedState.Members.front().strMarioEntryPatternId.empty() :
                            decodedState.Members.front().strMarioEntryPatternId == request.strPatternId && decodedState.Members.front().iMarioEntryStage == marioStage);
                    auto truncated = writer.Get_Buffer(); truncated.pop_back(); CPacketReader shortReader(truncated);
                    wire = wire && !Read_Message(shortReader, decodedState) && decodedState.iRunEpoch == state.iRunEpoch;
                    auto oldState = writer.Get_Buffer();
                    oldState.resize(oldState.size() - (27u + state.Members.front().strMarioEntryPatternId.size()));
                    CPacketReader oldStateReader(oldState);
                    auto invalidState = state; invalidState.Members.front().iMarioEntryStage = 5u;
                    CPacketWriter invalidStateWriter;
                    wire = wire && !Read_Message(oldStateReader, decodedState) && !Write_Message(invalidStateWriter, invalidState);
                }
            }
            if (scenario == MARIO_SCENARIO::CANCEL && member.bCompletionChainStarted && !completed) {
                room->Clear_KoukuSaydonPatternAudition(false, "Mario chain contract cancellation");
                const auto receipt = room->m_KoukuSaydonPatternAuditionReceiptBySessionId.find(player.iSessionId);
                tests.Require(room->m_KoukuSaydonPatternAudition.Members.empty() && receipt != room->m_KoukuSaydonPatternAuditionReceiptBySessionId.end() &&
                    receipt->second.LastLifecycle && receipt->second.LastLifecycle->eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
                    "Cancellation before child completion aborts rather than completing the two-pattern chain");
                return selected;
            }
            if (member.bCompletionChainAwaitingReturn && member.bMarioSoloReturnRequired && !member.bMarioReturnCompleted) {
                waited = waited || tick > completionTick;
                bounded = bounded && live->strPatternId.empty() && !member.bCompletionChainSuccessQueued &&
                    std::find(member.PatternIds.begin(), member.PatternIds.end(), "KOUKUSAYDON_G1_PATTERN_33") == member.PatternIds.end();
                if (interruptedScenario && tick == completionTick + 5u) {
                    if (scenario == MARIO_SCENARIO::DEATH) {
                        auto& entrant = room->m_Players.at(player.iPlayerId);
                        entrant.iCurrentHp = 0u; entrant.eAction = PLAYER_ACTION_STATE::DEAD;
                        if (immediateRevive) {
                            room->Update_MarioControlState(entrant);
                            entrant.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
                            C2S_REVIVE_PLAYER revive{}; revive.iClientSequence = 1u;
                            room->Handle_RevivePlayer(player.iSessionId, revive);
                            tests.Require(entrant.iCurrentHp == entrant.iMaximumHp && entrant.isCombatReady,
                                "The next command drain can revive a returned Mario corpse before audition preparation");
                        }
                    }
                    else room->Leave(player.iSessionId, PLAYER_DESPAWN_REASON::DISCONNECTED, false);
                    continue;
                }
            }
            const bool returnEarly = scenario == MARIO_SCENARIO::RETURN_EARLY && member.bMarioEntryConsumed;
            const bool returnAfter = (scenario == MARIO_SCENARIO::RETURN_AFTER_CHAIN || scenario == MARIO_SCENARIO::LAST_TICK_ENTRY) &&
                completionTick && tick >= completionTick + 60u;
            if (!returnStarted && (returnEarly || returnAfter)) {
                auto& entrant = room->m_Players.at(player.iPlayerId);
                // This fixture owns chain/return ordering; ball-hit admission is covered
                // by DebugTeleport. Complete the assigned published ball challenge before
                // asking the real typed command to begin its authoritative return motion.
                for (const auto& ball : room->m_WorldBootstrap.Get_MarioBalls())
                    if (ball.stage == entrant.iMarioStage && ball.layout == entrant.iMarioLayoutVariant &&
                        ball.color + 1u == entrant.iMarioRequiredColor && room->Mario_MatchingBallCount(entrant) < 3u)
                        room->m_MarioPoppedBalls[entrant.iMarioStage] |= static_cast<std::uint16_t>(1u << ball.slot);
                tests.Require(room->Mario_MatchingBallCount(entrant) == 3u,
                    "Prepare three assigned-colour balls before the real Mario chain return command");
                bool began = false;
                if (returnEarly) {
                    C2S_MARIO_RETURN command{}; command.iClientSequence = 1u; command.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
                    began = room->Apply_MarioReturn(entrant, command).eResult == MARIO_RETURN_RESULT::ACCEPTED;
                    const auto motion = entrant.TriggerMove;
                    began = began && room->Apply_MarioReturn(entrant, command).eResult == MARIO_RETURN_RESULT::ACCEPTED &&
                        entrant.TriggerMove.fElapsedSeconds == motion.fElapsedSeconds;
                } else {
                    // The current product pins a pattern-owned landing. The typed Return
                    // command consumes it; replaying the static exit action would bypass it.
                    C2S_MARIO_RETURN command{}; command.iClientSequence = 1u; command.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
                    began = room->Apply_MarioReturn(entrant, command).eResult == MARIO_RETURN_RESULT::ACCEPTED;
                }
                tests.Require(began && entrant.TriggerMove.isActive && !member.bMarioReturnCompleted,
                    "Start terminal return through authored motion and keep the solo gate closed before landing");
                if (!began) { bounded = false; break; }
                room->Complete_KoukuMarioReturn(entrant, "Mario3_Trigger_12", tick);
                bounded = bounded && !member.bMarioReturnCompleted;
                returnStarted = true;
            }
            if (member.PatternIds[member.iPatternIndex] == "KOUKUSAYDON_G1_PATTERN_33") {
                phase2Tick = tick;
                if (member.bMarioReturnCompleted) {
                    room->Complete_KoukuMarioReturn(room->m_Players.at(player.iPlayerId), "Mario3_Trigger_12", tick);
                    room->Prepare_KoukuAuditionTick(tick);
                }
                bounded = bounded && std::count(member.PatternIds.begin(), member.PatternIds.end(), "KOUKUSAYDON_G1_PATTERN_33") == 1;
                break;
            }
        }
        if (scenario == MARIO_SCENARIO::NO_ENTRY) noEntryCompletionTick = completionTick;

        tests.Require(bounded && retained && wire && selected.size() == 2u && std::set<std::string>(selected.begin(), selected.end()).size() == 2u &&
            std::all_of(selected.begin(), selected.end(), [&](const auto& id) { return marioCandidates.contains(id); }) &&
            completed == 2u && (noFollowup ? terminalCompleted && !phase2Tick : (interruptedScenario ? (scenario == MARIO_SCENARIO::DEATH ? phase2Tick > completionTick : aborted && !phase2Tick) : phase2Tick > completionTick)),
            "Count two distinct Server-selected real pattern completions and preserve optional terminal or followup behavior");
        const auto expectedNextStage = scenario == MARIO_SCENARIO::DISCONNECT ? 1u : (enterPortal ? marioStage + 1u : marioStage);
        tests.Require(entryCommitted == enterPortal && room->m_iNextMarioEntryStage == expectedNextStage,
            "Advance the successful-entry count once and reset it after the last real session leaves");
        if (scenario == MARIO_SCENARIO::RETURN_AFTER_CHAIN || scenario == MARIO_SCENARIO::LAST_TICK_ENTRY)
            tests.Require(waited && returnTick > completionTick && phase2Tick >= returnTick && !room->m_Players.at(player.iPlayerId).iMarioStage,
                "Wait for the solo entrant's actual terminal landing even when entry commits on the last child tick");
        if (scenario == MARIO_SCENARIO::RETURN_EARLY)
            tests.Require(landedBeforeCount && phase2Tick == completionTick + 1u,
                "A solo return completed first still waits for two patterns and queues phase 2 once");
        if (scenario == MARIO_SCENARIO::NO_ENTRY || scenario == MARIO_SCENARIO::PARTY)
            tests.Require(!waited && !returnStarted && phase2Tick == completionTick + 1u,
                "No entrant and a multiplayer entry both keep the existing immediate two-pattern success path");
        if (scenario == MARIO_SCENARIO::DISCONNECT)
            tests.Require(waited && aborted && !phase2Tick, "A solo entrant disconnect retains explicit departure cleanup without phase-2 success");
        if (scenario == MARIO_SCENARIO::DEATH) {
            tests.Require(waited && phase2Tick > completionTick &&
                room->m_KoukuSaydonPatternAudition.Members.front().bCompletionChainSuccessQueued,
                "A dead or immediately revived Mario entrant releases the solo return gate and continues phase 2");
            auto& raid = room->m_KoukuRaid;
            raid.pCatalog = room->m_pKoukuPublishedProductGeneration;
            raid.iOwnerSessionId = player.iSessionId; raid.PlayerIds = {player.iPlayerId};
            raid.iPrimaryBossId = bossId; raid.bEntryRunning = true;
            raid.iAuditionEpoch = result.iRoomAuditionEpoch;
            raid.State.iRunEpoch = 1u; raid.State.strGateId = "GATE3";
            raid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT;
            raid.State.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
            auto& entrant = room->m_Players.at(player.iPlayerId);
            room->Update_MarioControlState(entrant);
            entrant.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            C2S_REVIVE_PLAYER revive{}; revive.iClientSequence = immediateRevive ? 2u : 1u;
            room->Handle_RevivePlayer(player.iSessionId, revive);
            const auto* live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            tests.Require(entrant.iCurrentHp == entrant.iMaximumHp && entrant.isCombatReady &&
                entrant.eAction == PLAYER_ACTION_STATE::NONE && !entrant.iMarioStage &&
                room->Is_KoukuRaidRunning() && live && live->iNetEntityId == bossId && live->iCurrentHp &&
                live->strPatternId == "KOUKUSAYDON_G1_PATTERN_33",
                "Typed revival after Mario death preserves the same living boss and the running phase-2 occurrence");
        }
        return selected;
    };
    (void)marioScenario(MARIO_SCENARIO::NO_FOLLOWUP);
    (void)marioScenario(MARIO_SCENARIO::CANCEL);
    // Select a reproducible Server seed that includes the summon Parent; the
    // actual room still performs the selection and runs its published duration.
    std::uint32_t summonSeed = 0u;
    for (; summonSeed < 1000u; ++summonSeed) {
        std::vector<std::string> candidates{
            "KOUKUSAYDON_G1_PATTERN_38", "KOUKUSAYDON_G1_PATTERN_39", "KOUKUSAYDON_G1_PATTERN_40",
            "KOUKUSAYDON_G1_PATTERN_43", "KOUKUSAYDON_G1_PATTERN_52", "KOUKUSAYDON_G1_PATTERN_46"};
        std::mt19937 random(summonSeed);
        for (std::size_t i = candidates.size(); i > 1u; --i) std::swap(candidates[i - 1u], candidates[random() % i]);
        if (candidates[0] == "KOUKUSAYDON_G1_PATTERN_52" || candidates[1] == "KOUKUSAYDON_G1_PATTERN_52") break;
    }
    const auto summonOrder = marioScenario(MARIO_SCENARIO::NO_FOLLOWUP, summonSeed);
    tests.Require(std::find(summonOrder.begin(), summonOrder.end(), "KOUKUSAYDON_G1_PATTERN_52") != summonOrder.end(),
        "The actual two-pattern chain can select and complete the timed summon Parent");
    const auto firstMarioOrder = marioScenario(MARIO_SCENARIO::NO_ENTRY);
    const auto secondMarioOrder = marioScenario(MARIO_SCENARIO::RETURN_AFTER_CHAIN);
    tests.Require(!firstMarioOrder.empty() && firstMarioOrder == secondMarioOrder, "Replay the same Mario test seed through the same Server completion path");
    (void)marioScenario(MARIO_SCENARIO::RETURN_EARLY);
    (void)marioScenario(MARIO_SCENARIO::PARTY);
    (void)marioScenario(MARIO_SCENARIO::DEATH);
    (void)marioScenario(MARIO_SCENARIO::DEATH, 71023u, true);
    (void)marioScenario(MARIO_SCENARIO::DISCONNECT);
    (void)marioScenario(MARIO_SCENARIO::LAST_TICK_ENTRY);
    for (const auto& [patternId, expectedStage] : std::array<std::pair<const char*, std::uint8_t>, 2u>{
        {{"KOUKUSAYDON_G1_PATTERN_88", 1u}, {"KOUKUSAYDON_G1_PATTERN_91", 2u}}})
    {
        // Exercise the unmodified saved Parent, including its damaging WORLD rows.
        // Its 4.667-second opening is followed by real children through 41.109 seconds;
        // the late portal must admit one entrant and preserve the authored P33 handoff.
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        const auto* placement = room->Find_Placement("boss.koukusaydon.g3.saydon");
        SERVER_WORLD_ENTITY boss{}; std::string status;
        const auto* parent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(room->m_GameplayCatalog.Active(),
            patternId, status);
        const auto entry = parent ? std::find_if(parent->LogicWindows.begin(), parent->LogicWindows.end(), [](const auto& window) {
            return window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.CardRegions.empty() &&
                window.OnSuccess.size() == 1u && window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER;
        }) : std::vector<BOSS_PATTERN_LOGIC_WINDOW>::const_iterator{};
        const bool sourceReady = room->Is_Ready() && placement && parent && entry != parent->LogicWindows.end() &&
            !parent->ParentChildren.empty() && entry->OnSuccess.front().strPatternId == "KOUKUSAYDON_G1_PATTERN_33" &&
            room->Build_WorldEntity(*placement, room->m_iNextNetEntityId++, boss);
        tests.Require(sourceReady, "Published P88 retains its real Parent children, late Mario window and P33 success handoff");
        if (sourceReady) {
            const auto portal = entry->CardRegions.front();
            const auto enterMs = entry->iStartMs + 500u;
            room->m_WorldEntities.push_back(boss);
            for (unsigned ordinal = 0u; ordinal < 2u; ++ordinal) {
                SERVER_PLAYER player{}; player.iPlayerId = 970u + ordinal; player.iNetEntityId = 980u + ordinal;
                player.iSessionId = 990u + ordinal; player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
                player.eMadnessForm = ordinal ? PLAYER_MADNESS_FORM::NORMAL : PLAYER_MADNESS_FORM::CLOWN;
                player.fPositionX = boss.fPositionX + 25.f + ordinal; player.fPositionY = boss.fPositionY; player.fPositionZ = boss.fPositionZ;
                room->m_Players.emplace(player.iPlayerId, player);
            }
            C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
            request.iRequestSequence = 1u; request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
            request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; request.Scope.strGateId = "GATE3";
            request.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
            request.Scope.strBossPlacementId = placement->strPlacementId; request.Scope.strBossArchetypeId = placement->strArchetypeId;
            request.Scope.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
            request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
            request.strPatternId = parent->strPatternId;
            S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
            const bool queued = room->Evaluate_KoukuSaydonPatternAudition(990u, request, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
            bool moved = false, entered = false, reachedPhaseTwo = false, lateChild = false;
            std::uint32_t rootStart = 0u, entryTick = 0u;
            for (std::uint32_t tick = 1u; queued && tick < 2400u; ++tick) {
                room->m_iServerTick = tick; room->Prepare_KoukuAuditionTick(tick);
                room->Update_KoukuPatternTails(tick);
                auto* live = room->Find_KoukuSaydonArenaBoss(placement->strPlacementId, placement->strArchetypeId);
                if (!live || room->m_KoukuSaydonPatternAudition.Members.empty()) break;
                const auto& member = room->m_KoukuSaydonPatternAudition.Members.front();
                if (member.MarioEntryAnchor && !rootStart) rootStart = member.iMarioEntryStartTick;
                if (rootStart && !moved && tick - rootStart >= CKoukuSaydonLogicRuntime::Ticks_FromMs(enterMs)) {
                    auto& player = room->m_Players.at(970u);
                    player.fPositionX = portal.fCenterX; player.fPositionY = portal.fCenterY; player.fPositionZ = portal.fCenterZ;
                    lateChild = live->strPatternId != parent->strPatternId && !live->strPatternId.empty(); moved = true;
                }
                reachedPhaseTwo = live->strPatternId == "KOUKUSAYDON_G1_PATTERN_33";
                if (reachedPhaseTwo) break;
                room->Prepare_KoukuAuditionTick(tick);
                room->Commit_KoukuMarioEntries();
                live = room->Find_KoukuSaydonArenaBoss(placement->strPlacementId, placement->strArchetypeId);
                if (!live || !room->Update_KoukuSaydonBoss(*live, tick)) break;
                room->Commit_KoukuMechanicTriggers(tick); room->Commit_KoukuMarioEntries();
                if (room->m_Players.at(970u).iMarioStage && !entered) { entered = true; entryTick = tick; }
                if (room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) break;
            }
            const auto& entrant = room->m_Players.at(970u);
            if (!(queued && lateChild && entered && reachedPhaseTwo))
                std::cout << "[SavedP88] queued=" << queued << " child=" << lateChild << " entered=" << entered
                    << " phaseTwo=" << reachedPhaseTwo << " tick=" << room->m_iServerTick << " root=" << rootStart
                    << " entry=" << entryTick << " hp=" << entrant.iCurrentHp << " form=" << unsigned(entrant.eMadnessForm)
                    << " status=" << room->Get_Status() << '\n';
            tests.Require(queued && lateChild && entered && reachedPhaseTwo && entrant.iMarioStage == expectedStage &&
                room->m_iNextMarioEntryStage == expectedStage + 1u && entryTick - rootStart == CKoukuSaydonLogicRuntime::Ticks_FromMs(enterMs),
                "Actual Mario 1 and 2 commit on the first contacting tick during their saved child, before child damage, then reach P33");
        }
    }
    // Plain entry is an optional runtime contract, separate from the saved P88 Parent.
    // Use a small synthetic generation so authored children, fire and clip clocks remain unchanged.
    // LAST_TICK ends the plain entry through stagger on the very tick the entrant is judged inside the
    // Mario window, so the queued entry and the pattern completion race within one boss update.
    enum class PARENT_ENTRY { AUTHORED_STAGE, LIVE_COUNTER, REQUEST_STAGE, AFTER_WINDOW, LAST_TICK, LAST_TICK_SINGLE, MULTI_WINDOW };
    const auto parentMarioEntry = [&](const PARENT_ENTRY scenario) {
        const std::string parentId = "KOUKUSAYDON_CONTRACT_PLAIN_ENTRY";
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        const auto* placement = room->Find_Placement("boss.koukusaydon.g3.saydon");
        SERVER_WORLD_ENTITY boss{};
        if (!room->Is_Ready() || !placement || !room->Build_WorldEntity(*placement, room->m_iNextNetEntityId++, boss)) {
            tests.Require(false, "Load parent Mario entry scenario from the published Gate 3 placement"); return;
        }
        const std::uint8_t authoredStage = scenario == PARENT_ENTRY::LIVE_COUNTER ? 0u : 1u;
        const std::uint8_t expectedStage = scenario == PARENT_ENTRY::LIVE_COUNTER ? 3u : scenario == PARENT_ENTRY::REQUEST_STAGE ? 2u : scenario == PARENT_ENTRY::MULTI_WINDOW ? 4u : 1u;
        const bool finalTick = scenario == PARENT_ENTRY::LAST_TICK || scenario == PARENT_ENTRY::LAST_TICK_SINGLE;
        auto generation = std::make_shared<CGameplayCatalog>(room->m_GameplayCatalog.Active());
        auto* definitions = const_cast<std::vector<BOSS_PATTERN_DEFINITION>*>(generation->Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1"));
        if (definitions) {
            const auto source = std::find_if(definitions->begin(), definitions->end(), [](const auto& pattern) {
                return pattern.strPatternId == "KOUKUSAYDON_G1_PATTERN_88"; });
            if (source != definitions->end() && !source->Stages.empty()) {
                const auto entry = std::find_if(source->LogicWindows.begin(), source->LogicWindows.end(), [](const auto& window) {
                    return window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.CardRegions.empty(); });
                if (entry != source->LogicWindows.end()) {
                    auto plain = *source; auto portal = *entry;
                    plain.strPatternId = parentId; plain.strActionId = "contract.plain.mario.entry";
                    plain.ParentChildren.clear(); plain.strParentLoopStartOccurrenceId.clear();
                    plain.WorldSequences.clear(); plain.MechanicTriggers.clear(); plain.SceneProfiles.clear();
                    plain.BossMotion.reset(); plain.bResetBossToSpawn = false; plain.ResetBossYawDegrees.reset();
                    plain.Stages.resize(1u); plain.iExpectedStageCount = 1u;
                    plain.Stages.front().iDurationMs = 3000u; plain.Stages.front().Motion = {};
                    plain.Stages.front().Actions.clear(); plain.Stages.front().Branches.clear();
                    plain.iTimelineDurationMs = 3000u;
                    portal.strWindowId = parentId + ".logic.1";
                    portal.iStartMs = 100u; portal.iDurationMs = 1000u; portal.OnTimeout.clear();
                    plain.LogicWindows = {portal};
                    definitions->push_back(std::move(plain));
                }
            }
        }
        BOSS_LOGIC_REGION region{}; std::uint32_t windowStartMs = 0u, windowEndMs = 0u; bool authored = false, chainFree = false;
        if (definitions) for (auto& definition : *definitions) if (definition.strPatternId == parentId) {
            chainFree = std::none_of(definition.LogicWindows.begin(), definition.LogicWindows.end(),
                [](const auto& window) { return window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT; });
            for (auto& window : definition.LogicWindows) if (window.strWindowId == parentId + ".logic.1" &&
                window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.CardRegions.empty()) {
                BOSS_PATTERN_LOGIC_RESULT entry{};
                entry.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER; entry.iMarioEntryStage = authoredStage;
                window.OnSuccess = {entry}; region = window.CardRegions.front();
                windowStartMs = window.iStartMs; windowEndMs = window.iStartMs + window.iDurationMs; authored = true;
            }
            if (scenario == PARENT_ENTRY::MULTI_WINDOW && authored) {
                const auto first = std::find_if(definition.LogicWindows.begin(), definition.LogicWindows.end(),
                    [&](const auto& window) { return window.strWindowId == parentId + ".logic.1"; });
                auto second = *first; second.strWindowId = parentId + ".logic.second-entry";
                second.OnSuccess.front().iMarioEntryStage = 4u;
                for (auto& firstRegion : first->CardRegions) firstRegion.fCenterX += 50.f;
                definition.LogicWindows.push_back(std::move(second));
            }
            if (finalTick && authored) {
                BOSS_PATTERN_LOGIC_WINDOW stagger{};
                stagger.strWindowId = parentId + ".logic.lasttick"; stagger.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
                stagger.iStartMs = windowStartMs; stagger.iDurationMs = windowEndMs - windowStartMs;
                stagger.iThreshold = 1000u; stagger.bEndsPatternOnSuccess = true;
                definition.LogicWindows.push_back(stagger);
            }
        }
        std::string status;
        const auto* parent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, parentId, status);
        tests.Require(authored && chainFree && parent && CKoukuSaydonLogicRuntime::Has_MarioEntry(*parent) &&
            CKoukuSaydonLogicRuntime::Find_MarioEntryResult(*parent) && CKoukuSaydonLogicRuntime::Find_MarioEntryResult(*parent)->iMarioEntryStage == authoredStage,
            "An isolated plain ENTER_AREA pattern carries authored Mario entry independently of the saved P88 Parent");
        if (!authored) return;
        room->m_pKoukuPublishedProductGeneration = std::move(generation);
        room->m_WorldEntities.push_back(boss);
        SERVER_PLAYER player{};
        player.iPlayerId = 940u; player.iNetEntityId = 941u; player.iSessionId = 942u;
        player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
        player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
        player.fPositionX = boss.fPositionX + 25.f; player.fPositionY = boss.fPositionY; player.fPositionZ = boss.fPositionZ;
        room->m_Players.emplace(player.iPlayerId, player);
        if (scenario == PARENT_ENTRY::LIVE_COUNTER) room->m_iNextMarioEntryStage = 3u;
        C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
        request.iRequestSequence = 1u;
        request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
        request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
        request.Scope.strGateId = "GATE3"; request.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
        request.Scope.strBossPlacementId = "boss.koukusaydon.g3.saydon";
        request.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON";
        request.Scope.ExpectedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room->m_GameplayCatalog.Active());
        request.strPatternId = parentId;
        if (scenario == PARENT_ENTRY::REQUEST_STAGE) { request.iMarioTestStartStage = 2u; request.iMarioTestSeed = 7u; }
        S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
        const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED == room->Evaluate_KoukuSaydonPatternAudition(player.iSessionId, request, result);
        if (!queued) std::cerr << "Parent Mario admission: " << result.strReason << '\n';
        tests.Require(queued, "Admit a chain-free Gate 3 Parent as a Mario entry pattern, including the request's test stage");
        if (!queued) return;
        const std::uint32_t enterMs = scenario == PARENT_ENTRY::AFTER_WINDOW ? windowEndMs + 1000u : windowStartMs + 500u;
        bool bounded = true, spliced = false, moved = false, stageOk = false, wire = false, anchorCleared = false, pendingClear = false, secondReached = false;
        bool singleCompletionQueued = false;
        std::uint32_t memberStart = 0u, entryTick = 0u, completionTick = 0u;
        for (std::uint32_t tick = 1u; tick < 6000u; ++tick) {
            room->m_iServerTick = tick;
            room->Prepare_KoukuAuditionTick(tick);
            auto* live = room->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
            if (room->m_KoukuSaydonPatternAudition.Members.empty()) { bounded = false; break; }
            if (!live || !room->Update_KoukuSaydonBoss(*live, tick)) { bounded = false; break; }
            if (room->m_KoukuSaydonPatternAudition.Members.empty()) { bounded = false; break; }
            if (scenario == PARENT_ENTRY::LAST_TICK_SINGLE && room->m_KoukuSaydonPatternAudition.Members.front().bCompleted) {
                singleCompletionQueued = room->m_KoukuSaydonPatternAudition.Members.size() == 1u &&
                    room->m_PendingKoukuMarioEntries.size() == 1u && room->m_KoukuSaydonPatternAudition.Members.front().MarioEntryAnchor.has_value();
                completionTick = tick;
            }
            room->Commit_KoukuMarioEntries();
            if (room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) {
                if (scenario == PARENT_ENTRY::LAST_TICK_SINGLE && singleCompletionQueued) {
                    entryTick = room->m_Players.at(player.iPlayerId).iMarioStage ? tick : 0u;
                    secondReached = true; anchorCleared = true; pendingClear = room->m_PendingKoukuMarioEntries.empty();
                }
                else bounded = false;
                break;
            }
            auto& member = room->m_KoukuSaydonPatternAudition.Members.front();
            if (!spliced && member.MarioEntryAnchor && member.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE) {
                // The single-member regression deliberately ends the whole run on the entry tick.
                if (scenario != PARENT_ENTRY::LAST_TICK_SINGLE) {
                    member.PatternIds.push_back("KOUKUSAYDON_G1_PATTERN_38");
                    member.TransitionTicks.resize(member.PatternIds.size() - 1u, 1u);
                }
                spliced = true; memberStart = member.iMarioEntryStartTick;
                stageOk = member.iMarioEntryStage == (scenario == PARENT_ENTRY::MULTI_WINDOW ? authoredStage : expectedStage) && member.strMarioEntryPatternId == parentId && !member.bMarioEntryConsumed;
                S2C_KOUKUSAYDON_BUNDLE_STATE state{}, decoded{}; CPacketWriter writer;
                wire = room->Build_KoukuBundleState(state) && Write_Message(writer, state);
                if (wire) { CPacketReader reader(writer.Get_Buffer()); wire = Read_Message(reader, decoded) && !decoded.Members.empty() &&
                    decoded.Members.front().strMarioEntryPatternId.empty() && decoded.Members.front().iMarioEntryHoldMs == 0u; }
            }
            if (spliced && !moved && tick - memberStart >= CKoukuSaydonLogicRuntime::Ticks_FromMs(enterMs)) {
                auto& entrant = room->m_Players.at(player.iPlayerId);
                entrant.fPositionX = region.fCenterX; entrant.fPositionY = region.fCenterY; entrant.fPositionZ = region.fCenterZ;
                // The stagger threshold is crossed on the same next tick that judges the entrant inside the region.
                if (finalTick) live->iKoukuItemStaggerCredit = live->iKoukuItemStaggerMaximum;
                moved = true;
            }
            if (member.bMarioEntryConsumed && !entryTick) entryTick = tick;
            if (member.iPatternIndex == 1u) {
                secondReached = true; completionTick = tick;
                anchorCleared = !member.MarioEntryAnchor.has_value(); pendingClear = room->m_PendingKoukuMarioEntries.empty();
                room->Clear_KoukuSaydonPatternAudition(false, "Parent Mario entry contract end");
                break;
            }
        }
        const auto& entrant = room->m_Players.at(player.iPlayerId);
        if (!(bounded && spliced && stageOk && wire && secondReached && anchorCleared && pendingClear) ||
            (scenario != PARENT_ENTRY::AFTER_WINDOW && !entryTick))
            std::cout << "[ParentMario] scenario=" << static_cast<unsigned>(scenario)
                << " tick=" << room->m_iServerTick << " start=" << memberStart << " entry=" << entryTick
                << " completion=" << completionTick << " bounded=" << bounded << " spliced=" << spliced
                << " stageOk=" << stageOk << " wire=" << wire << " moved=" << moved
                << " second=" << secondReached << " anchorCleared=" << anchorCleared << " pendingClear=" << pendingClear
                << " hp=" << entrant.iCurrentHp << " ready=" << entrant.isCombatReady
                << " form=" << static_cast<unsigned>(entrant.eMadnessForm) << " action=" << static_cast<unsigned>(entrant.eAction)
                << " xyz=" << entrant.fPositionX << ',' << entrant.fPositionY << ',' << entrant.fPositionZ
                << " status=" << room->Get_Status() << '\n';
        tests.Require(bounded && spliced && stageOk && wire && secondReached && anchorCleared && pendingClear,
            "A chain-free entry Parent pins its own anchor without a Client hold and drops the portal when the pattern completes");
        if (scenario == PARENT_ENTRY::AFTER_WINDOW)
            tests.Require(moved && !entryTick && entrant.iMarioStage == 0u && room->m_iNextMarioEntryStage == 1u,
                "A chain-free Mario entry closes when its ENTER_AREA window ends");
        else if (finalTick)
            tests.Require(moved && entryTick && entryTick == completionTick &&
                (scenario != PARENT_ENTRY::LAST_TICK_SINGLE || singleCompletionQueued) &&
                entrant.iMarioStage == expectedStage && room->m_iNextMarioEntryStage == static_cast<std::uint8_t>(expectedStage + 1u),
                "An entry queued on the pattern's final tick still commits before the portal closes");
        else
            tests.Require(moved && entryTick && entryTick - memberStart >= CKoukuSaydonLogicRuntime::Ticks_FromMs(windowStartMs) &&
                entrant.iMarioStage == expectedStage && room->m_iNextMarioEntryStage == static_cast<std::uint8_t>(expectedStage + 1u),
                "Authored stage, live counter and request test stage each enter the expected Mario stage and advance the counter once");
    };
    parentMarioEntry(PARENT_ENTRY::AUTHORED_STAGE);
    parentMarioEntry(PARENT_ENTRY::LIVE_COUNTER);
    parentMarioEntry(PARENT_ENTRY::REQUEST_STAGE);
    parentMarioEntry(PARENT_ENTRY::AFTER_WINDOW);
    parentMarioEntry(PARENT_ENTRY::LAST_TICK);
    parentMarioEntry(PARENT_ENTRY::LAST_TICK_SINGLE);
    parentMarioEntry(PARENT_ENTRY::MULTI_WINDOW);
    {
        // PATTERNLOGICOUTCOME: an 11th field on MARIO_ENTER is the authored stage 1..4; other kinds keep the fear-only rule.
        namespace fs = std::filesystem;
        std::vector<wchar_t> buffer(32768u); fs::path dataRoot;
        const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer.data(), static_cast<DWORD>(buffer.size()));
        if (configured && configured < buffer.size()) dataRoot = buffer.data();
        else { GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size())); dataRoot = fs::path(buffer.data()).parent_path().parent_path() / L"DataFiles"; }
        std::ifstream input(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", std::ios::binary);
        std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (!bytes.empty() && bytes.back() != '\n') bytes += '\n';
        const std::string row = "PATTERNLOGICOUTCOME\tENCOUNTER_KOUKUSAYDON_G1\tKOUKUSAYDON_G1_PATTERN_88\tKOUKUSAYDON_G1_PATTERN_88.logic.1\tSUCCESS\t0\t";
        // Replace this fixture's exact outcome ordinal; the saved product may already own it.
        for (auto at = bytes.find(row); at != std::string::npos; at = bytes.find(row)) {
            const auto end = bytes.find('\n', at); bytes.erase(at, (end == std::string::npos ? bytes.size() : end + 1u) - at);
        }
        unsigned fixture = 0u;
        const auto load = [&](const std::string& extra, std::uint8_t& outStage) {
            std::string copy = bytes + extra;
            const auto headerEnd = copy.find('\n');
            const auto headerCount = copy.rfind('\t', headerEnd);
            copy.replace(headerCount + 1u, headerEnd - headerCount - 1u, std::to_string(std::count(copy.begin(), copy.end(), '\n') - 1u));
            const fs::path directory = fs::temp_directory_path() / (L"LostArkMarioStageContract-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(++fixture));
            std::error_code error; fs::create_directories(directory, error); const fs::path path = directory / L"Gameplay.bootstrap";
            { std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(copy.data(), static_cast<std::streamsize>(copy.size())); }
            GameplayDataRevision revision; std::string status; CGameplayCatalog generation;
            const bool loaded = !error && CServerApp::Hash_GameplayFileForAdmission(path, revision, status) && generation.Load_FromBootstrap(fs::canonical(path), revision, revision);
            outStage = 255u;
            const auto* pattern = loaded ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(generation, "KOUKUSAYDON_G1_PATTERN_88", status) : nullptr;
            const auto* entry = pattern ? CKoukuSaydonLogicRuntime::Find_MarioEntryResult(*pattern) : nullptr;
            if (entry) outStage = entry->iMarioEntryStage;
            if (!loaded) std::cout << "[STATUS] Mario stage fixture " << fixture << ": " << generation.Get_Status() << '\n';
            fs::remove_all(directory, error);
            return loaded;
        };
        std::uint8_t stage = 255u;
        tests.Require(!bytes.empty() && load(row + "MARIO_ENTER\t0\t0\t-\t2\n", stage) && stage == 2u,
            "An 11-field MARIO_ENTER outcome row carries the authored Mario stage");
        tests.Require(load(row + "MARIO_ENTER\t0\t0\t-\n", stage) && stage == 0u,
            "A 10-field MARIO_ENTER outcome row keeps the live room counter");
        tests.Require(!load(row + "MARIO_ENTER\t0\t0\t-\t5\n", stage) && !load(row + "MARIO_ENTER\t0\t0\t-\t0\n", stage) &&
            !load(row + "INSTANT_DEATH\t0\t0\t-\t2\n", stage),
            "Mario stages outside 1..4 and an 11th field on a non-fear, non-Mario outcome are rejected");
    }
    for (std::uint8_t stage = 1u; stage <= 4u; ++stage) {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
        SERVER_PLAYER player{}; player.iPlayerId = 930u; player.iNetEntityId = 931u;
        player.iCurrentHp = player.iMaximumHp = 100000u; player.isCombatReady = true;
        player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
        const bool entered = room->Enter_MarioFromPattern(player, stage);
        const auto after = player;
        const bool duplicateRejected = !room->Enter_MarioFromPattern(player, stage) && player.iMarioStage == after.iMarioStage && player.fPositionX == after.fPositionX;
        if (!entered) std::cerr << "Mario stage " << unsigned(stage) << ": " << room->Get_Status() << '\n';
        tests.Require(entered && duplicateRejected && player.iMarioStage == stage,
            "Enter each published Mario Intro through existing mode/layout setup and reject duplicate entry");
    }
#endif



	{
		const auto* koukuParts =
			catalog.Find_BossParts("BOSS_KOUKUSAYDON_G1_KOUKU");
		const auto* valtanParts = catalog.Find_BossParts("BOSS_VALTAN");
		const auto* patterns =
			catalog.Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1");
		const auto* sequence =
			catalog.Find_BossPatternSequence("ENCOUNTER_KOUKUSAYDON_G1");
		const std::uint32_t productSourceRevision =
			CKoukuSaydonBrain::Resolve_ProductSourceRevision(catalog);
		/* The Gate 1 Saydon owns every Product pattern; the first one in the
		authored Play All order is the reference occurrence these tests run. */
		const std::string firstProductId =
			nullptr != sequence && !sequence->PatternIds.empty() ?
				sequence->PatternIds.front() : std::string{};
		const auto firstProduct = nullptr == patterns ?
			std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{} :
			std::find_if(patterns->begin(), patterns->end(),
				[&firstProductId](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return firstProductId == pattern.strPatternId;
				});
		const std::size_t firstProductStageCount =
			nullptr != patterns && patterns->end() != firstProduct ?
				firstProduct->Stages.size() : 0u;
		const std::uint32_t firstProductLastStage = 0u == firstProductStageCount ?
			0u : static_cast<std::uint32_t>(firstProductStageCount - 1u);
		std::string animationStatus;
		const bool exactProduct = 0u != productSourceRevision && nullptr != patterns &&
			patterns->end() != firstProduct && nullptr != sequence &&
			"KOUKUSAYDON_G1_PLAY_ALL" == sequence->strSequenceId &&
			!sequence->PatternIds.empty() && !firstProductId.empty() &&
			CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
				*firstProduct, animationStatus);
		tests.Require(
			(nullptr == koukuParts || koukuParts->empty()) &&
			nullptr != valtanParts && !valtanParts->empty(),
			"Admit an unarmoured KoukuSaydon boss while retaining Valtan exact parts");
		tests.Require(exactProduct,
			"Admit the exact KoukuSaydon Saydon animation-only Product sequence");

		if (exactProduct)
		{
			SERVER_WORLD_ENTITY boss{};
			boss.iNetEntityId = 777u;
			boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss.eAction = SERVER_ENTITY_ACTION::IDLE;
			boss.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
			boss.strArchetypeId = "BOSS_KOUKUSAYDON_G1_KOUKU";
			boss.strPlacementId = "boss.koukusaydon.g1.kouku";
			boss.iCurrentHp = 1000000u;
			boss.iMaximumHp = 1000000u;
			boss.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
			CKoukuSaydonBrain brain;
			std::string status;
			const bool began = brain.Begin_Pattern(
				boss, *firstProduct, catalog.Get_ActiveRevision(), 1u, status);
			std::vector<std::string> visitedActions;
			if (began)
				visitedActions.push_back(boss.strActionId);
			KOUKUSAYDON_BRAIN_UPDATE_RESULT terminal =
				KOUKUSAYDON_BRAIN_UPDATE_RESULT::IDLE;
			for (std::uint32_t tick = 1u; began && tick < 700u; ++tick)
			{
				const std::string previousAction = boss.strActionId;
				terminal = brain.Update(boss, catalog, tick, status);
				if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED == terminal &&
					boss.strActionId != previousAction)
				{
					visitedActions.push_back(boss.strActionId);
				}
				if (KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED == terminal ||
					KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION ==
						terminal)
				{
					break;
				}
			}
			tests.Require(began && firstProductStageCount == visitedActions.size() &&
				KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED == terminal &&
				boss.strPatternId.empty() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
					boss.PatternTerminalReceipt.eResult &&
				boss.PatternTerminalReceipt.iPatternSequence == boss.iPatternSequence,
				"Run every stage of the first KoukuSaydon Product on the fixed Server clock");

			BOSS_PATTERN_DEFINITION unsupported = *firstProduct;
			unsupported.Stages.front().Actions.push_back({});
			const bool rejectsAction =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().eHitShape = BOSS_PATTERN_HIT_SHAPE::CIRCLE;
			const bool rejectsHit =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().eHitActivationKind =
				BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW;
			const bool rejectsHitWindow =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().Motion.eKind =
				BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD;
			const bool rejectsMotion =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			unsupported = *firstProduct;
			unsupported.Stages.front().Branches.push_back({
				BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT,
				unsupported.Stages[1u].strActionId, {} });
			const bool rejectsBranch =
				!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
					unsupported, status);
			tests.Require(rejectsAction && rejectsHit && rejectsHitWindow && rejectsMotion &&
				rejectsBranch,
				"Fail closed on KoukuSaydon action hit motion and non-timeout branch data");

			{
				/* Different boss bodies share one authored Product order. Selection
				must retain that order and each retained pattern's outgoing pause. */
				std::vector<BOSS_PATTERN_DEFINITION> mixedPatterns(3u, patterns->front());
				mixedPatterns[0].strPatternId = "KOUKUSAYDON_TEST_KOUKU_FIRST";
				mixedPatterns[1].strPatternId = "KOUKUSAYDON_TEST_SAYDON";
				mixedPatterns[2].strPatternId = "KOUKUSAYDON_TEST_KOUKU_LAST";
				mixedPatterns[0].AuditionBossArchetypeIds = { "BOSS_KOUKUSAYDON_G2_KOUKU" };
				mixedPatterns[1].AuditionBossArchetypeIds = { "BOSS_KOUKUSAYDON_G1_SAYDON" };
				mixedPatterns[2].AuditionBossArchetypeIds = { "BOSS_KOUKUSAYDON_G2_KOUKU" };
				BOSS_PATTERN_SEQUENCE_DEFINITION mixedSequence = *sequence;
				mixedSequence.PatternIds = { mixedPatterns[0].strPatternId,
					mixedPatterns[1].strPatternId, mixedPatterns[2].strPatternId };
				mixedSequence.iExpectedStepCount = 3u;
				mixedSequence.TransitionPursuitTicks = { 7u, 19u };
				std::vector<std::string> selectedIds;
				std::vector<std::uint32_t> selectedTransitions;
				std::string selectionStatus;
				const bool koukuSelected = CKoukuSaydonBrain::Select_AnimationOnlySequence(
					mixedPatterns, mixedSequence, "BOSS_KOUKUSAYDON_G2_KOUKU",
					selectedIds, selectedTransitions, selectionStatus) &&
					selectedIds == std::vector<std::string>{ mixedPatterns[0].strPatternId,
						mixedPatterns[2].strPatternId } &&
					selectedTransitions == std::vector<std::uint32_t>{ 7u };
				const bool saydonSelected = CKoukuSaydonBrain::Select_AnimationOnlySequence(
					mixedPatterns, mixedSequence, "BOSS_KOUKUSAYDON_G1_SAYDON",
					selectedIds, selectedTransitions, selectionStatus) &&
					selectedIds == std::vector<std::string>{ mixedPatterns[1].strPatternId } &&
					selectedTransitions.empty();
				const bool emptyPreserved = !CKoukuSaydonBrain::Select_AnimationOnlySequence(
					mixedPatterns, mixedSequence, "BOSS_KOUKUSAYDON_G2_BIG_SAYDON",
					selectedIds, selectedTransitions, selectionStatus) &&
					!selectionStatus.empty() &&
					selectedIds == std::vector<std::string>{ mixedPatterns[1].strPatternId } &&
					selectedTransitions.empty();
				tests.Require(koukuSelected && saydonSelected && emptyPreserved,
					"Select Play All by boss body in authored order and preserve selection on no compatible Product");
			}

			{
				/* G2 Big Saydon is tuned above the floor. A saved Y must survive
				the same entity build and fixed ticks that consume the world bootstrap. */
				auto heightRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
				const auto* bigPlacement = heightRoom->Find_Placement("boss.koukusaydon.g2.big-saydon");
				const auto* regularPlacement = heightRoom->Find_Placement("boss.koukusaydon.g1.saydon");
				const bool heightFixtureReady = heightRoom->Is_Ready() &&
					nullptr != bigPlacement && nullptr != regularPlacement &&
					heightRoom->m_ServerNavigation.Is_Loaded();
				tests.Require(heightFixtureReady, "Load the saved Big Saydon height contract placements and navigation");
				if (heightFixtureReady)
				{
					constexpr float authoredHeight = 8.63f;
					WORLD_BOOTSTRAP_PLACEMENT raisedPlacement = *bigPlacement;
					raisedPlacement.fPositionY = authoredHeight;
					auto raisedBoss = std::make_unique<SERVER_WORLD_ENTITY>();
					SERVER_NAV_POINT bigGround{};
					const bool raisedBuilt = heightRoom->m_ServerNavigation.Is_PointWalkableExact(
							raisedPlacement.fPositionX, raisedPlacement.fPositionZ) &&
						heightRoom->m_ServerNavigation.Sample_Position(
							raisedPlacement.fPositionX, raisedPlacement.fPositionZ, bigGround) &&
						heightRoom->Build_WorldEntity(raisedPlacement, heightRoom->m_iNextNetEntityId, *raisedBoss);
					tests.Require(raisedBuilt && std::abs(bigGround.y - authoredHeight) > 0.1f &&
						std::abs(raisedBoss->fPositionX - raisedPlacement.fPositionX) < 0.001f &&
						std::abs(raisedBoss->fPositionZ - raisedPlacement.fPositionZ) < 0.001f &&
						std::abs(raisedBoss->fPositionY - authoredHeight) < 0.001f &&
						std::abs(raisedBoss->fSpawnPositionY - authoredHeight) < 0.001f,
						"Keep Big Saydon saved Y 8.63 in current and spawn transforms without shifting walkable XZ");
					bool heightPersisted = raisedBuilt;
					if (raisedBuilt)
					{
						const NET_ENTITY_ID raisedId = raisedBoss->iNetEntityId;
						++heightRoom->m_iNextNetEntityId;
						heightRoom->m_WorldEntities.push_back(std::move(*raisedBoss));
						for (std::uint32_t tick = 0u; heightPersisted && tick < 90u; ++tick)
						{
							++heightRoom->m_iServerTick;
							heightRoom->Update_WorldEntities(1.f / 30.f);
							const auto live = std::find_if(heightRoom->m_WorldEntities.begin(),
								heightRoom->m_WorldEntities.end(), [raisedId](const SERVER_WORLD_ENTITY& candidate)
								{ return candidate.iNetEntityId == raisedId; });
							heightPersisted = heightRoom->m_WorldEntities.end() != live &&
								SERVER_ENTITY_ACTION::IDLE == live->eAction && live->strPatternId.empty() &&
								std::abs(live->fPositionY - authoredHeight) < 0.001f &&
								std::abs(live->fSpawnPositionY - authoredHeight) < 0.001f;
						}
					}
					tests.Require(heightPersisted, "Keep the spawned Big Saydon authored height through 90 arena fixed ticks");
					WORLD_BOOTSTRAP_PLACEMENT regularRaised = *regularPlacement;
					regularRaised.fPositionY = authoredHeight;
					auto regularBoss = std::make_unique<SERVER_WORLD_ENTITY>();
					SERVER_NAV_POINT regularGround{};
					const bool regularBuilt = heightRoom->m_ServerNavigation.Sample_Position(
							regularRaised.fPositionX, regularRaised.fPositionZ, regularGround) &&
						heightRoom->Build_WorldEntity(regularRaised, heightRoom->m_iNextNetEntityId, *regularBoss);
					tests.Require(regularBuilt && std::abs(regularGround.y - authoredHeight) > 0.1f &&
						std::abs(regularBoss->fPositionY - regularGround.y) < 0.001f &&
						std::abs(regularBoss->fSpawnPositionY - regularGround.y) < 0.001f,
						"Keep ordinary arena bosses on navigation height despite a different authored Y");
				}
			}

			{
				/* Every published gate boss placement is disabled, including Gate 1
				Kouku. Explicitly activate that original actor before testing that
				an audition owned by another boss keeps it idle. All-despawn removes
				both disabled-placement actors, while preserving unrelated Esther. */
				auto arenaRoom = std::make_unique<CGameRoom>(
					WORLD_ID::KOUKUSAYDON_ARENA);
				const WORLD_BOOTSTRAP_PLACEMENT* gateSaydon =
					arenaRoom->Find_Placement("boss.koukusaydon.g1.saydon");
				const WORLD_BOOTSTRAP_PLACEMENT* gateKouku =
					arenaRoom->Find_Placement("boss.koukusaydon.g1.kouku");
				WORLD_BOOTSTRAP_PLACEMENT enabledTemplate{};
				if (gateKouku) enabledTemplate = *gateKouku;
				enabledTemplate.isEnabled = true;
				const bool placementsAdmitted = arenaRoom->Is_Ready() &&
					nullptr != gateSaydon && nullptr != gateKouku &&
					CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::KOUKUSAYDON_ARENA, *gateSaydon) &&
					CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::KOUKUSAYDON_ARENA, *gateKouku) &&
					!CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::KOUKUSAYDON_ARENA, enabledTemplate) &&
					nullptr == arenaRoom->Find_KoukuSaydonAuditionBoss() &&
					!CKoukuSaydonBrain::Is_ArenaBossPlacement(
						WORLD_ID::CHARACTER_SELECT_ARENA, *gateSaydon);
				tests.Require(placementsAdmitted,
					"Admit only disabled KoukuSaydon gate boss placements for the arena Debug spawn");

				SERVER_WORLD_ENTITY originalKouku{};
				const bool originalBuilt = placementsAdmitted && arenaRoom->Build_WorldEntity(
					*gateKouku, arenaRoom->m_iNextNetEntityId, originalKouku);
				if (originalBuilt)
				{
					++arenaRoom->m_iNextNetEntityId;
					arenaRoom->m_WorldEntities.push_back(std::move(originalKouku));
				}
				SERVER_WORLD_ENTITY gateEsther{};
				gateEsther.iNetEntityId = arenaRoom->m_iNextNetEntityId++;
				gateEsther.eKind = WORLD_BOOTSTRAP_KIND::NPC;
				gateEsther.strPlacementId = "fixture.kouku.gate.esther";
				gateEsther.isEstherSummon = true;
				gateEsther.iCurrentHp = gateEsther.iMaximumHp = 1u;
				gateEsther.iEstherStrikeMs = 600000u;
				const NET_ENTITY_ID gateEstherId = gateEsther.iNetEntityId;
				arenaRoom->m_WorldEntities.push_back(std::move(gateEsther));
				const auto estherRemoved = [&]() {
					return std::none_of(arenaRoom->m_WorldEntities.begin(), arenaRoom->m_WorldEntities.end(),
						[gateEstherId](const SERVER_WORLD_ENTITY& entity) {
							return entity.iNetEntityId == gateEstherId && entity.isEstherSummon;
						});
				};
				SERVER_WORLD_ENTITY gateBoss{};
				const bool built = originalBuilt &&
					arenaRoom->Build_WorldEntity(
						*gateSaydon, arenaRoom->m_iNextNetEntityId, gateBoss);
				bool idle = built;
				NET_ENTITY_ID gateId = INVALID_NET_ENTITY_ID;
				if (built)
				{
					gateId = gateBoss.iNetEntityId;
					++arenaRoom->m_iNextNetEntityId;
					arenaRoom->m_WorldEntities.push_back(gateBoss);
					for (std::uint32_t tick = 0u; idle && tick < 90u; ++tick)
					{
						/* Advance the room clock the way the fixed tick does, so
						the brain sees 90 distinct ticks rather than one. */
						++arenaRoom->m_iServerTick;
						arenaRoom->Update_WorldEntities(1.f / 30.f);
						const auto live = std::find_if(
							arenaRoom->m_WorldEntities.begin(),
							arenaRoom->m_WorldEntities.end(),
							[gateId](const SERVER_WORLD_ENTITY& candidate)
							{
								return candidate.iNetEntityId == gateId;
							});
						const SERVER_WORLD_ENTITY* auditionBoss =
							arenaRoom->Find_KoukuSaydonAuditionBoss();
						idle = arenaRoom->m_WorldEntities.end() != live &&
							live->strPatternId.empty() &&
							SERVER_ENTITY_ACTION::IDLE == live->eAction &&
							live->iCurrentHp == live->iMaximumHp &&
							CKoukuSaydonBrain::Is_ArenaBoss(
								WORLD_ID::KOUKUSAYDON_ARENA, *live) &&
							!CKoukuSaydonBrain::Is_GateOneBoss(
								WORLD_ID::KOUKUSAYDON_ARENA, *live) &&
							nullptr != auditionBoss &&
							auditionBoss->iNetEntityId != gateId;
					}
				}
				tests.Require(built && idle,
					"Keep a Debug-activated KoukuSaydon gate boss idle without a brain or pattern");

				/* Match the actual F1 Gate 1 destination in Client Get_DebugGates
				and Server KOUKU_GATES. A farther player-spawn placement would
				miss a collision introduced by snapping the raised boss. */
				arenaRoom->Refresh_PlayerBlockingBodies();
				tests.Require(built &&
					std::abs(gateBoss.fPositionX - gateSaydon->fPositionX) < 0.001f &&
					std::abs(gateBoss.fPositionZ - gateSaydon->fPositionZ) < 0.001f &&
					arenaRoom->m_ServerCollisionSystem.Is_PlayerPositionClear(
						-2.45f, 1.32f, 740.37f, INVALID_NET_ENTITY_ID),
					"Keep a raised KoukuSaydon gate boss on its authored XZ so the Gate 1 player position stays clear");

				{
					/* The Debug clown toggle swaps only the replicated madness
					form; the gauge, HP and class stay untouched, and a dead
					player keeps the body its death was authored on. */
					auto formStorage = std::make_unique<SERVER_PLAYER>();
					SERVER_PLAYER& formPlayer = *formStorage;
					formPlayer.iPlayerId = 321u;
					formPlayer.iNetEntityId = 654u;
					formPlayer.iCurrentHp = formPlayer.iMaximumHp = 100u;
					C2S_DEBUG_SET_MADNESS_FORM formRequest{};
					formRequest.iRequestSequence = 1u;
					formRequest.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
					formRequest.eForm = PLAYER_MADNESS_FORM::CLOWN;
					auto formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
#ifndef _DEBUG
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::REJECTED_DISABLED == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::NORMAL == formPlayer.eMadnessForm,
						"Release refuses the Debug madness form toggle");
#else
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::ACCEPTED == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::CLOWN == formVerdict.eActiveForm &&
						PLAYER_MADNESS_FORM::CLOWN == formPlayer.eMadnessForm &&
						0u == formPlayer.iCurrentMadness && 100u == formPlayer.iCurrentHp,
						"Accept the Debug clown form without touching gauge or HP");
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::ACCEPTED == formVerdict.eResult,
						"Replay of the same madness form sequence returns the stored verdict");
					formRequest.iRequestSequence = 2u;
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::REJECTED_SAME_FORM == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::CLOWN == formPlayer.eMadnessForm,
						"Reject a madness form the player already presents");
					formRequest.iRequestSequence = 3u;
					formRequest.eForm = PLAYER_MADNESS_FORM::NORMAL;
					formPlayer.iCurrentHp = 0u;
					formPlayer.eAction = PLAYER_ACTION_STATE::DEAD;
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::REJECTED_PLAYER_STATE == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::CLOWN == formPlayer.eMadnessForm,
						"Reject a madness form change on a dead player");
					formPlayer.iCurrentHp = 100u;
					formPlayer.eAction = PLAYER_ACTION_STATE::NONE;
					formRequest.iRequestSequence = 4u;
					formVerdict = arenaRoom->Apply_DebugMadnessForm(formPlayer, formRequest);
					tests.Require(
						DEBUG_MADNESS_FORM_RESULT::ACCEPTED == formVerdict.eResult &&
						PLAYER_MADNESS_FORM::NORMAL == formPlayer.eMadnessForm,
						"Return the player body through the same Debug toggle");
#endif
				}

#ifdef _DEBUG
				{
					/* The audition scope may name the explicitly activated Gate 1 Kouku, but a
					Saydon-body pattern must not play on the Kouku body. */
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST saydonRequest{};
					saydonRequest.iRequestSequence = 1u;
					saydonRequest.eOperation =
						KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
					saydonRequest.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
					saydonRequest.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
					saydonRequest.Scope.strBossPlacementId = "boss.koukusaydon.g1.kouku";
					saydonRequest.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G1_KOUKU";
					saydonRequest.Scope.ExpectedGameplayRevision =
						arenaRoom->m_GameplayCatalog.Get_ActiveRevision();
					saydonRequest.Scope.iExpectedSourceRevision =
						CKoukuSaydonBrain::Resolve_ProductSourceRevision(
							arenaRoom->m_GameplayCatalog.Active());
					saydonRequest.strPatternId = firstProductId;
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT saydonResult{};
					const KOUKUSAYDON_PATTERN_AUDITION_RESULT saydonVerdict =
						built && idle ?
							arenaRoom->Evaluate_KoukuSaydonPatternAudition(
								8102u, saydonRequest, saydonResult) :
							KOUKUSAYDON_PATTERN_AUDITION_RESULT::END;
					tests.Require(
						KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNSUPPORTED_PATTERN ==
							saydonVerdict &&
						CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
							arenaRoom->m_KoukuSaydonPatternAudition.ePhase,
						"Reject a Saydon-body pattern audition on the Gate 1 Kouku boss");
				}

				/* Exercise the room dispatch with the original Kouku still first
				in its entity list. Updating that idle actor must never abort an
				accepted occurrence owned by a later, Debug-activated boss. */
				const WORLD_BOOTSTRAP_PLACEMENT* secondKoukuPlacement =
					arenaRoom->Find_Placement("boss.koukusaydon.g3.saydon");
				auto secondKouku = std::make_unique<SERVER_WORLD_ENTITY>();
				const bool secondBuilt = nullptr != secondKoukuPlacement &&
					arenaRoom->Build_WorldEntity(*secondKoukuPlacement,
						arenaRoom->m_iNextNetEntityId, *secondKouku);
				const NET_ENTITY_ID secondKoukuId = secondKouku->iNetEntityId;
				if (secondBuilt)
				{
					++arenaRoom->m_iNextNetEntityId;
					arenaRoom->m_WorldEntities.push_back(std::move(*secondKouku));
				}
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST secondRequest{};
				secondRequest.iRequestSequence = 1u;
				secondRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				secondRequest.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
				secondRequest.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
				secondRequest.Scope.strBossPlacementId = "boss.koukusaydon.g3.saydon";
				secondRequest.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON";
				secondRequest.Scope.ExpectedGameplayRevision =
					arenaRoom->m_GameplayCatalog.Get_ActiveRevision();
				secondRequest.Scope.iExpectedSourceRevision =
					CKoukuSaydonBrain::Resolve_ProductSourceRevision(arenaRoom->m_GameplayCatalog.Active());
				// Product admission now pins the exact Gate and placement as well as the body.
				secondRequest.Scope.strGateId = "GATE3";
				const BOSS_PATTERN_DEFINITION* gateThreePattern = nullptr;
				for (const auto& id : sequence->PatternIds)
				{
					const auto found = std::find_if(patterns->begin(), patterns->end(),
						[&](const auto& pattern) { return pattern.strPatternId == id &&
							pattern.strGateId == secondRequest.Scope.strGateId &&
							pattern.strTargetBossPlacementId == secondRequest.Scope.strBossPlacementId; });
					if (found != patterns->end()) { gateThreePattern = &*found; break; }
				}
				secondRequest.strPatternId = gateThreePattern ? gateThreePattern->strPatternId : std::string{};
				std::uint32_t gateThreeCompletionTicks = 30u;
				if (gateThreePattern)
					for (const auto& stage : gateThreePattern->Stages)
						gateThreeCompletionTicks += CKoukuSaydonLogicRuntime::Ticks_FromMs(stage.iDurationMs);
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT secondResult{};
				const bool secondQueued = secondBuilt && gateThreePattern &&
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
						arenaRoom->Evaluate_KoukuSaydonPatternAudition(8103u, secondRequest, secondResult);
				bool originalStayedIdle = secondQueued;
				for (std::uint32_t tick = 0u; originalStayedIdle && tick < gateThreeCompletionTicks; ++tick)
				{
					arenaRoom->Update_WorldEntities(1.f / 30.f);
					++arenaRoom->m_iServerTick;
					const SERVER_WORLD_ENTITY* original = arenaRoom->Find_KoukuSaydonAuditionBoss();
					originalStayedIdle = nullptr != original && original->strPatternId.empty() &&
						SERVER_ENTITY_ACTION::IDLE == original->eAction;
					if (CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
						arenaRoom->m_KoukuSaydonPatternAudition.ePhase)
						break;
				}
				const SERVER_WORLD_ENTITY* completedKouku = arenaRoom->Find_KoukuSaydonArenaBoss(
					secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId);
				const auto& lifecycle = arenaRoom->m_PendingKoukuSaydonPatternAuditionLifecycle;
				const bool completed = secondQueued && originalStayedIdle && nullptr != completedKouku &&
					completedKouku->strPatternId.empty() &&
					SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED ==
						completedKouku->PatternTerminalReceipt.eResult &&
					std::any_of(lifecycle.begin(), lifecycle.end(),
						[secondKoukuId](const auto& entry)
						{
							return secondKoukuId == entry.Message.iBossNetEntityId &&
								KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED == entry.Message.eState;
						}) &&
					std::none_of(lifecycle.begin(), lifecycle.end(), [](const auto& entry)
						{ return KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED == entry.Message.eState; });
				tests.Require(completed,
					"Complete a raised Gate 3 Saydon pattern through room ticks while the original Kouku stays idle");

				secondRequest.iRequestSequence = 2u;
				secondRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL;
				secondRequest.strPatternId.clear();
				const bool allQueued = completed &&
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
						arenaRoom->Evaluate_KoukuSaydonPatternAudition(8103u, secondRequest, secondResult);
				std::uint32_t stageBeforeDespawn = 0u;
				std::uint32_t sequenceBeforeDespawn = 0u;
				const std::uint32_t firstStageTicks = gateThreePattern && !gateThreePattern->Stages.empty() ?
					CKoukuSaydonLogicRuntime::Ticks_FromMs(gateThreePattern->Stages.front().iDurationMs) + 30u : 0u;
				for (std::uint32_t tick = 0u; allQueued && tick < firstStageTicks; ++tick)
				{
					arenaRoom->Update_WorldEntities(1.f / 30.f);
					++arenaRoom->m_iServerTick;
					const SERVER_WORLD_ENTITY* playing = arenaRoom->Find_KoukuSaydonArenaBoss(
						secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId);
					if (nullptr != playing)
					{
						stageBeforeDespawn = playing->iPatternStageIndex;
						sequenceBeforeDespawn = playing->iPatternSequence;
					}
					if (stageBeforeDespawn > 0u)
						break;
				}
				const bool runningBeforeDespawn = allQueued && stageBeforeDespawn > 0u &&
					CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE ==
						arenaRoom->m_KoukuSaydonPatternAudition.ePhase;
				const bool removedActive = runningBeforeDespawn &&
					arenaRoom->Despawn_KoukuSaydonArenaDebugEntities() &&
					CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
						arenaRoom->m_KoukuSaydonPatternAudition.ePhase &&
					nullptr == arenaRoom->Find_KoukuSaydonArenaBoss(
						secondRequest.Scope.strBossPlacementId, secondRequest.Scope.strBossArchetypeId) &&
					nullptr == arenaRoom->Find_KoukuSaydonAuditionBoss() && estherRemoved() &&
					std::any_of(lifecycle.begin(), lifecycle.end(),
						[secondKoukuId, sequenceBeforeDespawn](const auto& entry)
						{
							return secondKoukuId == entry.Message.iBossNetEntityId &&
								// Run termination identifies its owner occurrence; it has no member stage.
								sequenceBeforeDespawn == entry.Message.iPatternSequence &&
								0u == entry.Message.iStageIndex && entry.Message.strMemberId.empty() &&
								2u == entry.Message.iRequestSequence &&
								KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED == entry.Message.eState;
						});
				tests.Require(removedActive,
					"Abort the active raised-boss Play All owner and remove disabled gate bosses and end the prior gate Esther summon");
#endif

				const bool reverted = built && idle &&
					arenaRoom->Despawn_KoukuSaydonArenaDebugEntities() &&
					arenaRoom->m_WorldEntities.end() == std::find_if(
						arenaRoom->m_WorldEntities.begin(),
						arenaRoom->m_WorldEntities.end(),
						[gateId](const SERVER_WORLD_ENTITY& candidate)
						{
							return candidate.iNetEntityId == gateId;
						}) &&
					nullptr == arenaRoom->Find_KoukuSaydonAuditionBoss() && estherRemoved();
				tests.Require(reverted,
					"Despawn all Debug-activated gate bosses and end the prior gate Esther summon");
			}

#ifdef _DEBUG
			auto room = std::make_unique<CGameRoom>(
				WORLD_ID::KOUKUSAYDON_ARENA);
			/* Every Product plays on the Saydon body, so the audition target is
			the Gate 1 Saydon raised from its disabled placement. */
			const auto raiseGateSaydon = [](CGameRoom& target) -> SERVER_WORLD_ENTITY*
			{
				const WORLD_BOOTSTRAP_PLACEMENT* placement =
					target.Find_Placement("boss.koukusaydon.g1.saydon");
				SERVER_WORLD_ENTITY raised{};
				if (nullptr == placement ||
					!target.Build_WorldEntity(*placement, target.m_iNextNetEntityId, raised))
					return nullptr;
				++target.m_iNextNetEntityId;
				target.m_WorldEntities.push_back(raised);
				return target.Find_KoukuSaydonArenaBoss(
					"boss.koukusaydon.g1.saydon", "BOSS_KOUKUSAYDON_G1_SAYDON");
			};
			/* Use the published room, navigation and patterns through the same
			request -> fixed tick -> factory -> wire admission as Complete Play. */
			const auto exerciseProductionGaze = [&](const bool exhaustCloneIds)
			{
				auto gazeRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
				SERVER_WORLD_ENTITY* raised = raiseGateSaydon(*gazeRoom);
				const bool ready = gazeRoom->Is_Ready() && nullptr != raised;
				if (!ready)
				{
					tests.Require(false, "Build the production Saydon gaze room and owner");
					return;
				}
				const NET_ENTITY_ID ownerId = raised->iNetEntityId;
				const float centerX = raised->fSpawnPositionX;
				const float centerZ = raised->fSpawnPositionZ;
				const float beforeY = raised->fPositionY;
				const float beforeYaw = raised->fYawDegrees;
				const std::size_t beforeCount = gazeRoom->m_WorldEntities.size();
				if (exhaustCloneIds)
					gazeRoom->m_iNextNetEntityId = (std::numeric_limits<NET_ENTITY_ID>::max)() - 1u;
				const NET_ENTITY_ID beforeNextId = gazeRoom->m_iNextNetEntityId;
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
				request.iRequestSequence = 1u;
				request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
				request.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
				request.Scope.strBossPlacementId = "boss.koukusaydon.g1.saydon";
				request.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G1_SAYDON";
				request.Scope.ExpectedGameplayRevision = gazeRoom->m_GameplayCatalog.Get_ActiveRevision();
				request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(
					gazeRoom->m_GameplayCatalog.Active());
				request.strPatternId = "KOUKUSAYDON_G1_PATTERN_2";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
				const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					gazeRoom->Evaluate_KoukuSaydonPatternAudition(8199u, request, result);
				bool triggerFired = false;
				for (std::uint32_t tick = 0u; queued && !triggerFired && tick < 120u; ++tick)
				{
					gazeRoom->Update_WorldEntities(1.f / 30.f);
					++gazeRoom->m_iServerTick;
					const auto& cues = gazeRoom->m_KoukuSaydonPatternAudition.Members.front().LogicLedger.MechanicTriggers;
					triggerFired = std::any_of(cues.begin(), cues.end(),
						[](const auto& cue) { return cue.bStarted; });
				}
				const auto findOwner = [&]() -> SERVER_WORLD_ENTITY*
				{
					return gazeRoom->Find_KoukuSaydonArenaBoss(
						"boss.koukusaydon.g1.saydon", "BOSS_KOUKUSAYDON_G1_SAYDON");
				};
				SERVER_WORLD_ENTITY* owner = findOwner();
				if (exhaustCloneIds)
				{
					/* Two valid children stage before the third would get the zero
					ID. No actor or ID counter may commit from that partial set. */
					tests.Require(queued && triggerFired && nullptr != owner &&
						gazeRoom->m_WorldEntities.size() == beforeCount &&
						gazeRoom->m_iNextNetEntityId == beforeNextId &&
						owner->fPositionX == centerX && owner->fPositionY == beforeY &&
						owner->fPositionZ == centerZ && owner->fYawDegrees == beforeYaw &&
						gazeRoom->m_PendingKoukuMechanicTriggers.empty() &&
						gazeRoom->Get_Status().find("clone admission failed") != std::string::npos,
						"Preserve the production gaze owner and IDs when the third staged clone cannot be admitted");
					return;
				}
				bool admitted = queued && triggerFired && nullptr != owner &&
					gazeRoom->m_WorldEntities.size() == beforeCount + 3u &&
					gazeRoom->m_iNextNetEntityId == beforeNextId + 3u;
				if (!admitted)
				{
					std::cerr << "Production gaze admission: " << gazeRoom->Get_Status() << '\n';
					tests.Require(false, "Commit three production gaze clones at the teleport trigger tick");
					return;
				}
				const float dx = owner->fPositionX - centerX;
				const float dz = owner->fPositionZ - centerZ;
				const float radius = std::sqrt(dx * dx + dz * dz);
				const auto looksAtCenter = [centerX, centerZ](const SERVER_WORLD_ENTITY& entity)
				{
					const float x = centerX - entity.fPositionX;
					const float z = centerZ - entity.fPositionZ;
					const float distance = std::sqrt(x * x + z * z);
					const float yaw = (entity.fYawDegrees + 90.f) * 0.017453292519943295f;
					return distance > 0.f && (std::sin(yaw) * x + std::cos(yaw) * z) / distance > 0.9999f;
				};
				const auto gazeDefinition = std::find_if(patterns->begin(), patterns->end(),
					[](const auto& pattern) { return pattern.strPatternId == "KOUKUSAYDON_G1_PATTERN_2"; });
				const BOSS_PATTERN_MECHANIC_TRIGGER* expectedTeleport = nullptr;
				if (gazeDefinition != patterns->end())
					for (const auto& trigger : gazeDefinition->MechanicTriggers)
						if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT)
							expectedTeleport = &trigger;
                // 본체는 저작 기준점에서 90도씩 떨어진 네 후보 중 하나여야 한다.
                const auto isAuthoredGazeSlot = [&](const SERVER_WORLD_ENTITY& entity)
                {
                    if (!expectedTeleport || expectedTeleport->ClockHours.size() != 3u)
                    {
                        return false;
                    }
                    const float sourceX = expectedTeleport->fTeleportX - centerX;
                    const float sourceZ = expectedTeleport->fTeleportZ - centerZ;
                    const float sourceRadius = std::sqrt(sourceX * sourceX + sourceZ * sourceZ);
                    const float sourceAngle = std::atan2(sourceX, sourceZ);
                    const std::array<std::uint32_t, 4u> hours{1u, expectedTeleport->ClockHours[0u],
                        expectedTeleport->ClockHours[1u], expectedTeleport->ClockHours[2u]};
                    for (const auto hour : hours)
                    {
                        const float angle = sourceAngle + static_cast<float>(hour - 1u) * 0.52359877559829887f;
                        const float x = hour == 1u ? expectedTeleport->fTeleportX : centerX + std::sin(angle) * sourceRadius;
                        const float z = hour == 1u ? expectedTeleport->fTeleportZ : centerZ + std::cos(angle) * sourceRadius;
                        if (std::abs(entity.fPositionX - x) < .001f && std::abs(entity.fPositionZ - z) < .001f)
                        {
                            return true;
                        }
                    }
                    return false;
                };
                std::vector<std::array<float, 2u>> occupied{{owner->fPositionX, owner->fPositionZ}};
				std::vector<std::uint8_t> ownerPayload;
				S2C_WORLD_ENTITY_SPAWNED ownerMessage{};
				const bool ownerWritten = gazeRoom->Build_WorldEntitySpawnedPayload(*owner, ownerPayload);
				CPacketReader ownerReader{ ownerPayload };
				admitted = admitted && ownerWritten && Read_Message(ownerReader, ownerMessage) &&
					expectedTeleport &&
                    isAuthoredGazeSlot(*owner) &&
                    std::abs(owner->fPositionY - expectedTeleport->fTeleportY) < 0.001f &&
					gazeRoom->m_ServerNavigation.Is_PointWalkableExact(owner->fPositionX, owner->fPositionZ) &&
					looksAtCenter(*owner);
				std::size_t cloneCount = 0u;
				for (const auto& clone : gazeRoom->m_WorldEntities)
				{
					if (!clone.bKoukuGazeClone)
						continue;
					++cloneCount;
					std::vector<std::uint8_t> payload;
					S2C_WORLD_ENTITY_SPAWNED message{};
					const bool written = gazeRoom->Build_WorldEntitySpawnedPayload(clone, payload);
					CPacketReader reader{ payload };
					const float cloneX = clone.fPositionX - centerX;
					const float cloneZ = clone.fPositionZ - centerZ;
					admitted = admitted && written && Read_Message(reader, message) &&
						Is_Valid_WorldEntitySpawnOwner(message, &ownerMessage) &&
						clone.iOwnerBossNetEntityId == ownerId &&
						clone.iKoukuCloneOwnerSequence == owner->iPatternSequence &&
						clone.strPatternId == "KOUKUSAYDON_G1_PATTERN_5" &&
						clone.iActionStartTick == gazeRoom->m_iServerTick &&
						clone.iPatternStartTick == gazeRoom->m_iServerTick &&
						clone.PinnedDefinitionRevision == owner->PinnedDefinitionRevision &&
						gazeRoom->m_ServerNavigation.Is_PointWalkableExact(clone.fPositionX, clone.fPositionZ) &&
						std::abs(std::sqrt(cloneX * cloneX + cloneZ * cloneZ) - radius) < 0.002f &&
                        std::abs(clone.fPositionY - owner->fPositionY) < 0.001f && looksAtCenter(clone) &&
                        isAuthoredGazeSlot(clone) &&
                        std::none_of(occupied.begin(), occupied.end(), [&](const auto& position)
                        {
                            return std::hypot(clone.fPositionX - position[0u], clone.fPositionZ - position[1u]) < .001f;
                        });
                    occupied.push_back({clone.fPositionX, clone.fPositionZ});
				}
				tests.Require(admitted && 3u == cloneCount,
					"Commit production Pattern 2 into three Pattern 5 clones on the real navgrid with inward yaw and valid spawn owners");
				gazeRoom->Update_WorldEntities(1.f / 30.f);
				++gazeRoom->m_iServerTick;
				tests.Require(gazeRoom->m_WorldEntities.size() == beforeCount + 3u &&
					gazeRoom->m_iNextNetEntityId == beforeNextId + 3u,
					"Keep the production gaze trigger one-shot across the next room tick");
				owner = findOwner();
				if (nullptr != owner)
					gazeRoom->m_KoukuSaydonBrain.Complete_Pattern(*owner, gazeRoom->m_iServerTick);
				gazeRoom->Update_WorldEntities(1.f / 30.f);
				++gazeRoom->m_iServerTick;
				tests.Require(nullptr != findOwner() && 0u == findOwner()->iPatternStartTick &&
					gazeRoom->m_WorldEntities.size() == beforeCount &&
					std::none_of(gazeRoom->m_WorldEntities.begin(), gazeRoom->m_WorldEntities.end(),
						[](const auto& entity) { return entity.bKoukuGazeClone; }),
					"Remove all three production gaze clones on the owning pattern termination edge");
			};
			exerciseProductionGaze(false);
			exerciseProductionGaze(true);
			for (const char* patternId : { "KOUKUSAYDON_G1_PATTERN_6", "KOUKUSAYDON_G1_PATTERN_7" })
			{
				auto centeredRoom = std::make_unique<CGameRoom>(WORLD_ID::KOUKUSAYDON_ARENA);
				auto* centeredBoss = raiseGateSaydon(*centeredRoom);
				if (nullptr == centeredBoss)
				{
					tests.Require(false, "Raise the production Dance/Roulette owner");
					continue;
				}
				SERVER_PLAYER cardPlayer{};
				cardPlayer.iPlayerId = 8197u;
				cardPlayer.iNetEntityId = centeredRoom->m_iNextNetEntityId++;
				cardPlayer.iCurrentHp = cardPlayer.iMaximumHp = 100u;
				centeredRoom->m_Players.emplace(cardPlayer.iPlayerId, cardPlayer);
				centeredRoom->Apply_KoukuGateEntryCard(centeredRoom->m_Players.at(cardPlayer.iPlayerId), *centeredBoss);
				const auto assignedSuit = centeredRoom->m_Players.at(cardPlayer.iPlayerId).eMechanicCardSymbol;
				const auto assignedColor = centeredRoom->m_Players.at(cardPlayer.iPlayerId).eMechanicCardColor;
				const float centerX = centeredBoss->fSpawnPositionX;
				const float centerY = centeredBoss->fSpawnPositionY;
				const float centerZ = centeredBoss->fSpawnPositionZ;
				centeredBoss->fPositionX += 5.f;
				centeredBoss->fPositionZ -= 4.f;
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
				request.iRequestSequence = 1u;
				request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				request.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
				request.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
				request.Scope.strBossPlacementId = "boss.koukusaydon.g1.saydon";
				request.Scope.strBossArchetypeId = "BOSS_KOUKUSAYDON_G1_SAYDON";
				request.Scope.ExpectedGameplayRevision = centeredRoom->m_GameplayCatalog.Get_ActiveRevision();
				request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(centeredRoom->m_GameplayCatalog.Active());
				request.strPatternId = patternId;
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
				const bool queued = KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					centeredRoom->Evaluate_KoukuSaydonPatternAudition(8197u, request, result);
				// Spawn reset commits before the first stage's root motion advances in this tick.
				centeredRoom->Prepare_KoukuAuditionTick(centeredRoom->m_iServerTick + 1u);
				const bool resetAtStartCommit = centeredBoss->fPositionX == centerX &&
					centeredBoss->fPositionY == centerY && centeredBoss->fPositionZ == centerZ &&
					centeredBoss->iPatternStartTick == centeredRoom->m_iServerTick + 1u;
				centeredRoom->Update_WorldEntities(1.f / 30.f);
				++centeredRoom->m_iServerTick;
				centeredBoss = centeredRoom->Find_KoukuSaydonArenaBoss(request.Scope.strBossPlacementId, request.Scope.strBossArchetypeId);
				const bool rootStartedAtSpawn = centeredBoss &&
					(centeredBoss->PatternStageRootMotion.empty() ||
						(centeredBoss->bPatternStageRootOriginCaptured &&
							centeredBoss->fPatternStageOriginX == centerX &&
							centeredBoss->fPatternStageOriginY == centerY &&
							centeredBoss->fPatternStageOriginZ == centerZ));
				tests.Require(queued && resetAtStartCommit && rootStartedAtSpawn &&
					centeredBoss->iPatternStartTick == centeredRoom->m_iServerTick &&
					MECHANIC_CARD_SYMBOL::NONE != assignedSuit && MECHANIC_CARD_COLOR::NONE != assignedColor,
					"Reset the production Dance or Roulette boss to spawn before root motion and publish its fixed start clock with an encounter card");
				SERVER_WORLD_ENTITY otherGate{};
				otherGate.strArchetypeId = "BOSS_KOUKUSAYDON_G3_SAYDON";
				centeredRoom->Apply_KoukuGateEntryCard(centeredRoom->m_Players.at(cardPlayer.iPlayerId), otherGate);
				centeredRoom->Despawn_KoukuSaydonArenaDebugEntities();
				centeredRoom->Update_KoukuPlayerModes(2u);
				const auto& cleared = centeredRoom->m_Players.at(cardPlayer.iPlayerId);
				tests.Require(MECHANIC_CARD_SYMBOL::NONE == cleared.eMechanicCardSymbol &&
					MECHANIC_CARD_COLOR::NONE == cleared.eMechanicCardColor,
					"Remove the assigned suit and color when the Gate 1 Saydon owner leaves");
			}

			SERVER_WORLD_ENTITY* liveBoss = raiseGateSaydon(*room);
			C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST selected{};
			selected.iRequestSequence = 2u;
			selected.eOperation =
				KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
			selected.Scope.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
			selected.Scope.strGateId = "GATE1";
			selected.Scope.strEncounterId = "ENCOUNTER_KOUKUSAYDON_G1";
			selected.Scope.strBossPlacementId =
				"boss.koukusaydon.g1.saydon";
			selected.Scope.strBossArchetypeId =
				"BOSS_KOUKUSAYDON_G1_SAYDON";
			selected.Scope.ExpectedGameplayRevision =
				room->m_GameplayCatalog.Get_ActiveRevision();
			selected.Scope.iExpectedSourceRevision =
				CKoukuSaydonBrain::Resolve_ProductSourceRevision(
					room->m_GameplayCatalog.Active());
			selected.strPatternId = firstProductId;

			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT rejected{};
			auto wrongScope = selected;
			wrongScope.iRequestSequence = 1u;
			wrongScope.Scope.strBossPlacementId = "boss.valtan.center";
			const bool exactScopeRejected = room->Is_Ready() && nullptr != liveBoss &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SCOPE_MISMATCH ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, wrongScope, rejected) &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty();

			auto wrongGameplayRevision = selected;
			wrongGameplayRevision.iRequestSequence = 1u;
			wrongGameplayRevision.Scope.ExpectedGameplayRevision.Bytes.front() ^=
				0xffu;
			const bool gameplayRevisionRejected = exactScopeRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_REVISION_MISMATCH ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, wrongGameplayRevision, rejected) &&
				rejected.eResult == KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_REVISION_MISMATCH &&
				rejected.Scope.ExpectedGameplayRevision ==
					wrongGameplayRevision.Scope.ExpectedGameplayRevision &&
				rejected.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				rejected.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				0u == rejected.iRoomAuditionEpoch &&
				INVALID_NET_ENTITY_ID == rejected.iBossNetEntityId &&
				rejected.strResolvedPatternId.empty() &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty();

			auto wrongSourceRevision = selected;
			wrongSourceRevision.iRequestSequence = 1u;
			wrongSourceRevision.Scope.iExpectedSourceRevision =
				(std::numeric_limits<std::uint32_t>::max)() ==
					selected.Scope.iExpectedSourceRevision ?
					selected.Scope.iExpectedSourceRevision - 1u :
					selected.Scope.iExpectedSourceRevision + 1u;
			const bool sourceRevisionRejected = gameplayRevisionRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_SOURCE_REVISION_MISMATCH ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, wrongSourceRevision, rejected) &&
				rejected.eResult == KOUKUSAYDON_PATTERN_AUDITION_RESULT::
					REJECTED_SOURCE_REVISION_MISMATCH &&
				rejected.Scope.iExpectedSourceRevision ==
					wrongSourceRevision.Scope.iExpectedSourceRevision &&
				rejected.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				rejected.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				0u == rejected.iRoomAuditionEpoch &&
				INVALID_NET_ENTITY_ID == rejected.iBossNetEntityId &&
				rejected.strResolvedPatternId.empty() &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.empty();

			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT selectedResult{};
			const bool selectedQueued = sourceRevisionRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, selected, selectedResult) &&
				selectedResult.Scope.eWorldId == selected.Scope.eWorldId &&
				selectedResult.Scope.strEncounterId ==
					selected.Scope.strEncounterId &&
				selectedResult.Scope.strBossPlacementId ==
					selected.Scope.strBossPlacementId &&
				selectedResult.Scope.strBossArchetypeId ==
					selected.Scope.strBossArchetypeId &&
				selectedResult.Scope.ExpectedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				selectedResult.Scope.iExpectedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				selectedResult.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				selectedResult.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				firstProductId ==
					selectedResult.strResolvedPatternId &&
				1u == room->m_PendingKoukuSaydonPatternAuditionLifecycle.size() &&
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING ==
					room->m_PendingKoukuSaydonPatternAuditionLifecycle.front().
						Message.eState;

			bool runtimeAdvanced = selectedQueued;
			for (std::uint32_t tick = 1u; runtimeAdvanced && tick < 700u; ++tick)
			{
				// The room prepares all due members before any per-boss update.
				room->Prepare_KoukuAuditionTick(tick);
				runtimeAdvanced = room->Update_KoukuSaydonBoss(*liveBoss, tick);
				room->m_iServerTick = tick;
				if (CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase)
				{
					break;
				}
			}
			std::vector<std::uint32_t> liveStageIndices;
			std::size_t patternCompletedCount = 0u;
			std::size_t completedCount = 0u;
			bool exactLifecycleIdentity = selectedQueued;
			std::uint32_t occurrenceSequence = 0u;
			for (const auto& targeted :
				room->m_PendingKoukuSaydonPatternAuditionLifecycle)
			{
				const auto& lifecycle = targeted.Message;
				exactLifecycleIdentity = exactLifecycleIdentity &&
					8101u == targeted.iSessionId &&
					selected.iRequestSequence == lifecycle.iRequestSequence &&
					selected.eOperation == lifecycle.eOperation &&
					selected.Scope.eWorldId == lifecycle.Scope.eWorldId &&
					selected.Scope.strEncounterId ==
						lifecycle.Scope.strEncounterId &&
					selected.Scope.strBossPlacementId ==
						lifecycle.Scope.strBossPlacementId &&
					selected.Scope.strBossArchetypeId ==
						lifecycle.Scope.strBossArchetypeId &&
					selected.Scope.ExpectedGameplayRevision ==
						lifecycle.Scope.ExpectedGameplayRevision &&
					selected.Scope.ExpectedGameplayRevision ==
						lifecycle.PinnedGameplayRevision &&
					selected.Scope.iExpectedSourceRevision ==
						lifecycle.Scope.iExpectedSourceRevision &&
					selected.Scope.iExpectedSourceRevision ==
						lifecycle.iPinnedSourceRevision &&
					firstProductId == lifecycle.strPatternId;
				if (KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
					lifecycle.eState)
				{
					liveStageIndices.push_back(lifecycle.iStageIndex);
					if (0u == occurrenceSequence)
						occurrenceSequence = lifecycle.iPatternSequence;
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence;
				}
				else if (KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::
					PATTERN_COMPLETED == lifecycle.eState)
				{
					++patternCompletedCount;
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence &&
						firstProductLastStage == lifecycle.iStageIndex;
				}
				else if (KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED ==
					lifecycle.eState)
				{
					++completedCount;
					// PATTERN_COMPLETED above carries the last stage; COMPLETED closes the run.
					exactLifecycleIdentity = exactLifecycleIdentity &&
						occurrenceSequence == lifecycle.iPatternSequence &&
						0u == lifecycle.iStageIndex && lifecycle.strMemberId.empty();
				}
			}
			std::vector<std::uint32_t> expectedLiveStages;
			for (std::uint32_t stage = 0u; stage < firstProductStageCount; ++stage)
				expectedLiveStages.push_back(stage);
			tests.Require(runtimeAdvanced && exactLifecycleIdentity &&
				0u != occurrenceSequence &&
				expectedLiveStages == liveStageIndices &&
				1u == patternCompletedCount && 1u == completedCount &&
				CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE ==
					room->m_KoukuSaydonPatternAudition.ePhase &&
				liveBoss->strPatternId.empty() &&
				SERVER_ENTITY_ACTION::IDLE == liveBoss->eAction,
				"Publish exact KoukuSaydon Pending Active stage Live and terminal lifecycle edges");

			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT duplicateResult{};
			const std::size_t lifecycleCountBeforeStaleRequest =
				room->m_PendingKoukuSaydonPatternAuditionLifecycle.size();
			auto staleSourceIdentity = selected;
			staleSourceIdentity.Scope.iExpectedSourceRevision =
				wrongSourceRevision.Scope.iExpectedSourceRevision;
			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT staleSourceResult{};
			const bool staleSourceRejected =
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, staleSourceIdentity, staleSourceResult) &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST ==
					staleSourceResult.eResult &&
				lifecycleCountBeforeStaleRequest ==
					room->m_PendingKoukuSaydonPatternAuditionLifecycle.size();
			const bool duplicateReconciled =
				staleSourceRejected &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, selected, duplicateResult) &&
				duplicateResult.Scope.ExpectedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				duplicateResult.Scope.iExpectedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				duplicateResult.PinnedGameplayRevision ==
					selected.Scope.ExpectedGameplayRevision &&
				duplicateResult.iPinnedSourceRevision ==
					selected.Scope.iExpectedSourceRevision &&
				occurrenceSequence == duplicateResult.iPatternSequence &&
				// Duplicate verdicts reconcile to the last run-terminal lifecycle edge.
				0u == duplicateResult.iStageIndex;

			auto playAll = selected;
			playAll.iRequestSequence = 3u;
			playAll.eOperation =
				KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL;
			playAll.strPatternId.clear();
			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT playAllResult{};
			std::vector<std::string> expectedGateOneOrder;
			std::vector<std::uint32_t> expectedGateOneTransitions;
			std::vector<std::size_t> expectedGateOneIndices;
			for (std::size_t index = 0; index < sequence->PatternIds.size(); ++index)
			{
				const auto definition = std::find_if(patterns->begin(), patterns->end(),
					[&](const auto& pattern) { return pattern.strPatternId == sequence->PatternIds[index]; });
				if (definition != patterns->end() && definition->strGateId == playAll.Scope.strGateId &&
					definition->strTargetBossPlacementId == playAll.Scope.strBossPlacementId)
				{
					expectedGateOneOrder.push_back(definition->strPatternId);
					expectedGateOneIndices.push_back(index);
				}
			}
			// Only selected adjacent occurrences have a transition; the final one has none.
			for (std::size_t index = 0; index + 1 < expectedGateOneIndices.size(); ++index)
				expectedGateOneTransitions.push_back(sequence->TransitionPursuitTicks[expectedGateOneIndices[index]]);
			const bool playAllQueued = duplicateReconciled && !expectedGateOneOrder.empty() &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					room->Evaluate_KoukuSaydonPatternAudition(
						8101u, playAll, playAllResult) &&
				expectedGateOneOrder ==
					room->m_KoukuSaydonPatternAudition.Members.front().PatternIds &&
				expectedGateOneTransitions ==
					room->m_KoukuSaydonPatternAudition.Members.front().TransitionTicks &&
				playAllResult.Scope.ExpectedGameplayRevision ==
					playAll.Scope.ExpectedGameplayRevision &&
				playAllResult.Scope.iExpectedSourceRevision ==
					playAll.Scope.iExpectedSourceRevision &&
				playAllResult.PinnedGameplayRevision ==
					playAll.Scope.ExpectedGameplayRevision &&
				playAllResult.iPinnedSourceRevision ==
					playAll.Scope.iExpectedSourceRevision &&
				expectedGateOneOrder.front() ==
					playAllResult.strResolvedPatternId;
			tests.Require(exactScopeRejected && gameplayRevisionRejected &&
				sourceRevisionRejected && staleSourceRejected &&
				selectedQueued &&
				duplicateReconciled && playAllQueued,
				"Scope Play Selected retries and Play All consume only the Server Product sequence");

			/* Evaluate stages PENDING locally, while Handle sends the result frame
			   immediately. The room can only flush PENDING/ACTIVE after the fixed
			   world update, so the real reliable session FIFO must expose the verdict
			   before either lifecycle edge. */
			constexpr SESSION_ID FIFO_SESSION = 8102u;
			auto fifoRoom = std::make_unique<CGameRoom>(
				WORLD_ID::KOUKUSAYDON_ARENA);
			auto fifoSession = std::make_shared<CClientSession>(
				FIFO_SESSION, INVALID_SOCKET,
				CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
			fifoSession->m_isSendRunning.store(true);
			fifoRoom->m_Sessions.emplace(FIFO_SESSION, fifoSession);
			(void)raiseGateSaydon(*fifoRoom);
			auto fifoRequest = selected;
			fifoRequest.iRequestSequence = 1u;
			fifoRequest.Scope.ExpectedGameplayRevision =
				fifoRoom->m_GameplayCatalog.Get_ActiveRevision();
			fifoRequest.Scope.iExpectedSourceRevision =
				CKoukuSaydonBrain::Resolve_ProductSourceRevision(
					fifoRoom->m_GameplayCatalog.Active());
			fifoRoom->Handle_KoukuSaydonPatternAudition(
				FIFO_SESSION, fifoRequest);
			fifoRoom->Tick(1.f / 30.f);

			struct KOUKU_FIFO_EDGE final
			{
				bool isResult = false;
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result{};
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Lifecycle{};
			};
			std::vector<KOUKU_FIFO_EDGE> fifoEdges;
			for (const auto& outbound : fifoSession->m_OutboundFrames)
			{
				if (PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT !=
						outbound.ePacketType &&
					PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE !=
						outbound.ePacketType)
				{
					continue;
				}
				PACKET_HEADER header{};
				if (!Read_Packet_Header(outbound.Bytes, header) ||
					outbound.Bytes.size() < PACKET_HEADER_BYTES)
				{
					continue;
				}
				CPacketReader reader{ std::span<const std::uint8_t>(
					outbound.Bytes.data() + PACKET_HEADER_BYTES,
					outbound.Bytes.size() - PACKET_HEADER_BYTES) };
				KOUKU_FIFO_EDGE edge{};
				if (PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT ==
					header.ePacketType)
				{
					edge.isResult = Read_Message(reader, edge.Result) &&
						0u == reader.Get_RemainingSize();
					if (edge.isResult)
						fifoEdges.push_back(std::move(edge));
				}
				else if (PACKET_TYPE::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE ==
						header.ePacketType &&
					Read_Message(reader, edge.Lifecycle) &&
					0u == reader.Get_RemainingSize())
				{
					fifoEdges.push_back(std::move(edge));
				}
			}
			const bool resultBeforeLifecycle = fifoRoom->Is_Ready() &&
				3u == fifoEdges.size() && fifoEdges[0u].isResult &&
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
					fifoEdges[0u].Result.eResult &&
				!fifoEdges[1u].isResult &&
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING ==
					fifoEdges[1u].Lifecycle.eState &&
				!fifoEdges[2u].isResult &&
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE ==
					fifoEdges[2u].Lifecycle.eState &&
				0u == fifoEdges[2u].Lifecycle.iStageIndex &&
				firstProductId ==
					fifoEdges[2u].Lifecycle.strPatternId &&
				fifoRequest.iRequestSequence ==
					fifoEdges[0u].Result.iRequestSequence &&
				fifoEdges[0u].Result.iRequestSequence ==
					fifoEdges[1u].Lifecycle.iRequestSequence &&
				fifoEdges[0u].Result.iRequestSequence ==
					fifoEdges[2u].Lifecycle.iRequestSequence;
			const bool fifoExactRevisionEcho = resultBeforeLifecycle &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[0u].Result.Scope.ExpectedGameplayRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[0u].Result.PinnedGameplayRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[0u].Result.Scope.iExpectedSourceRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[0u].Result.iPinnedSourceRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[1u].Lifecycle.Scope.ExpectedGameplayRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[1u].Lifecycle.PinnedGameplayRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[1u].Lifecycle.Scope.iExpectedSourceRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[1u].Lifecycle.iPinnedSourceRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[2u].Lifecycle.Scope.ExpectedGameplayRevision &&
				fifoRequest.Scope.ExpectedGameplayRevision ==
					fifoEdges[2u].Lifecycle.PinnedGameplayRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[2u].Lifecycle.Scope.iExpectedSourceRevision &&
				fifoRequest.Scope.iExpectedSourceRevision ==
					fifoEdges[2u].Lifecycle.iPinnedSourceRevision;
			tests.Require(resultBeforeLifecycle,
				"Send KoukuSaydon QUEUED result before PENDING and first Active Live edge");
			tests.Require(fifoExactRevisionEcho,
				"Echo exact KoukuSaydon gameplay and Product source pins on every FIFO edge");
			fifoSession->Request_Close();
#endif
		}
	}
}
```


## 사용자 반영 후 검증 명령

새 C++ 파일은 없으므로 project/filter 등록은 추가하지 않는다. 기존 Shared 패킷과 저장 schema를 재사용하며 Data publish 대상은 없다. 패턴 변경은 새로 빌드한 Server 프로세스를 시작해야 사용된다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MaxCompilerProcesses 4 -BuildLogDirectory out/KoukuRandomManual/Build
& .\Server\Bin\Debug\Server.exe --kouku-product-contract-test
& .\Server\Bin\Debug\Server.exe --kouku-object-overlap-contract-test
```

이 문서의 후보에 실행한 것은 MSVC 문법 검사와 out 전용 helper 검사다. 위 Product 빌드와 제품 계약 검사는 **사용자가 패턴을 반영한 뒤 실행할 명령**이다. G0 구조 분리의 빌드·회귀 결과를 이 랜덤 코드의 제품 실행 결과로 대신하지 않는다.

Client 화면에서는 기존 F1 게시 패턴 재생으로 P2와 P13을 사용자가 직접 확인한다. P2는 네 후보·세 분신의 뒤돌기와 중앙 방향, P13은 선택 위치 타격·원래 위치의 일반 카드·flip/hop·재시작을 확인한다.
