# 2026-08-19 발탄 아레나 낙사 결과

계획 정본은 같은 폴더의 `2026-08-19_VALTAN_ARENA_FALL_DEATH_PLAN.md`다.
이 문서는 실제로 반영한 것, 계획과 달랐던 것, 실행한 검증과 실행하지 않은 검증을 분리해 적는다.

## 0. 결론

무너진 바닥 위에 남은 플레이어가 떨어져 죽고 기존 부활 경로로 돌아온다. 서버 권위이며
`PLAYER_SNAPSHOT`에 새 필드는 없고 프로토콜 버전도 `23` 그대로다.

코드 반영은 끝났고 빌드 없이 가능한 검증은 전부 통과했다. **빌드와 실행형 harness, 화면 확인은
아직 하지 않았다**(§4, §5).

## 1. 반영한 것

| 구분 | 경로 | 변경 |
|---|---|---|
| 수정 | `Shared/Public/Network/PacketMessages.h` | `PLAYER_ACTION_STATE::FALLING`을 `DEAD` 뒤에 추가 |
| 수정 | `Shared/Private/Network/PacketMessages.cpp` | `Is_Valid_PlayerSnapshot`에 FALLING 갈래 추가 |
| 수정 | `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | FALLING round-trip 1건 + 거부 2건, 낡은 버전 pin 교정 |
| 수정 | `Server/Public/WorldDestructionRuntime.h` | `WORLD_DESTRUCTION_MUTATION_DESCRIPTOR::bRemovesGround` |
| 수정 | `Server/Private/WorldDestructionBootstrap.cpp` | bootstrap format `1 -> 2`, mutation 행 8 field |
| 수정 | `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1` | `navPolarity`에서 `RemovesGround` 발행, 헤더 v2, 불변식·개수 검사 |
| 수정 | `Server/Public/ServerNavigation.h` | `VoidCounts`, `Set_VoidConditions`, `Is_PointInVoidRegion`, `Is_CellVoid` |
| 수정 | `Server/Private/ServerNavigation.cpp` | void cell count를 block count와 같은 트랜잭션으로 유지 |
| 수정 | `Server/Public/ServerPlayer.h` | `fFallVelocityY`, `iFallDeathTick` (wire 아님) |
| 수정 | `Server/Public/GameRoom.h` | `Update_PlayerFall` 선언 |
| 수정 | `Server/Private/GameRoom.cpp` | 낙사 상수, `Update_PlayerFall`, `Update_Players` 진입, admission 연결, 부활 위치 보정, audition reset 정리 |
| 수정 | `Server/Private/PlayerSkillSystem.cpp` | FALLING 조기 반환 |
| 수정 | `Server/Private/ServerGameplayContractTests.cpp` | 낙사 계약 6건 |
| 수정 | `Server/Private/WorldDestructionBootstrapContractTests.cpp` | fixture v2, removed-ground 계약 3건 |
| 수정 | `Client/Private/Character.cpp` | FALLING presentation 분기 |
| 생성물 | `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction{,presentation}.json` | publisher 재발행 |

`Server/Bin/DataFiles/World/VALTAN_ARENA.worlddestructionbootstrap`도 v2로 재발행했다.
이 파일은 `.gitignore` 대상이며 `Server.vcxproj`의 pre-build가
`Publish-ValtanWorldDestruction.ps1 -Mode Publish`를 돌려 각 PC에서 다시 만든다.

## 2. 계획과 달랐던 것

### 2.1 `removesGround`의 근거가 groupId 접두사가 아니었다

계획 §5.5는 publisher의 `destroyable.group.valtan.floor` 접두사를 쓰려 했다. 실측해 보니
`Data/Encounters/Valtan/ValtanWorldEvents.json`의 group마다 `navPolarity`가 이미 저작돼
있었고(`BLOCK_WHILE_INTACT` 99개, `BLOCK_WHILE_FRACTURED` 6개), publisher가
`Publish-ValtanWorldDestruction.ps1:900`에서 이미 이 값으로 navigation region polarity를
검증하고 있었다. 문자열 접두사 대신 이 typed 값을 그대로 썼다.

### 2.2 절대 개수 검사를 compile 함수에 둘 수 없었다

`-Mode ContractTest`가 합성 fixture로 `Compile-ValtanWorldDestruction`을 호출한다. compile
안에 "removesGround는 정확히 6"을 넣으면 fixture가 통과할 수 없어 실패했다. compile에는
per-mutation 불변식만 남기고, 절대 개수는 실제 정본을 compile한 결과에 대해
`Invoke-ContractTests`가 검사하도록 옮겼다.

### 2.3 사망 tick 계산에 기존 helper를 쓸 수 없었다

`Add_ServerTicksSkippingReservedZero`는 `GameRoom.cpp`의 `#ifdef _DEBUG` 구역 안에 있다.
낙사는 제품 동작이므로 Release에서도 컴파일되어야 해서, 상수와 사망 tick 계산을 모두 그
구역 **밖**에 두고 deadline은 직접 계산했다.

### 2.4 Client 하네스 테스트를 넣지 않았다

`ClientFrontendHarness`는 `Client/Private/Character.cpp`를 컴파일하지 않고 `PLAYER_ACTION_STATE`를
쓰는 코드도 0줄이다. 테스트를 넣으려면 `ClientFrontendHarness.vcxproj`에 source를 추가해야
하는데 `.vcxproj`/`.filters`는 이번 변경에서 건드리지 않기로 했다. Client 분기 검증은 빌드
성공과 사용자의 화면 확인이다.

### 2.5 이번 작업과 무관한 기존 실패를 하나 고쳤다

`NetworkProtocolHarness.cpp:1925`가 `21u == NETWORK_PROTOCOL_VERSION`을 단언하고 있었다.
PR #122(인벤토리)가 버전을 `21 -> 23`으로 올리면서 이 줄을 갱신하지 않아, harness를 다시
빌드하면 `World Destruction Protocol V21 Packet Types`가 실패한다. 이번 G1의 종료 조건이
`failures : 0`이라 `23u`로 교정했다. **이번 낙사 작업이 만든 실패가 아니다.**

## 3. 설계 요지

### 3.1 구멍의 정본

`.navblockers`의 바닥 region 6개(84단계 672 cell, 30단계 1577 cell)가 그대로 낙사 범위다.
`Gameplay.world.json`에 `fallVolume` kind를 새로 만들지 않았다.

벽 region과 구분하기 위해 destruction bootstrap의 mutation이 `removesGround`를 선언하고,
admission에서 그 조건 ID 집합을 `CServerNavigation::Set_VoidConditions`로 넘긴다. navigation은
`m_BlockCounts`와 같은 방식으로 `m_VoidCounts`를 유지하므로 질의는 O(1)이다.

`Set_VoidConditions`는 조건이 실재하고, 그 조건을 쓰는 region이 전부
`activateWhenConditionTrue == true`(부서진 뒤 막힘)일 때만 받는다. 벽 polarity를 구멍이라고
주장하면 방 admission이 실패한다.

### 3.2 판정 시점

`Update_Players` 진입에서 매 tick `Is_PointInVoidRegion(x, z)` 한 번이다. 서 있다 무너지는
경우, 밀려 들어가는 경우, 스킬 이동으로 들어가는 경우가 한 코드로 닫힌다. 붕괴 커밋은 tick
끝에서 일어나므로 낙하는 다음 tick(33 ms 뒤)에 시작한다.

### 3.3 입력·피격 차단

`Handle_Move`와 `CPlayerSkillSystem::Try_Start`가 이미 `NONE != eAction`에서 닫히므로 새 가드가
필요 없었다. 타게팅과 광역 피격 네 곳은 전부 `!isCombatReady`를 보므로, 낙하 시작에서
`isCombatReady = false` 하나만 세워 네 곳을 함께 닫았다.

### 3.4 부활

`Handle_RevivePlayer`는 원래 제자리 부활이라 구멍에서 되살아나 무한 낙사가 된다. 부활 시점에
현재 셀이 walkable이 아니면 `strSpawnPlacementId`의 spawn을 navigation projection한 위치로
되돌린다. 다른 사인의 죽음은 기존대로 그 자리에서 부활한다.

## 4. 실행한 검증

| 검증 | 결과 |
|---|---|
| `Publish-ValtanWorldDestruction.ps1 -Mode Validate` | PASS, groups=105 bindings=117 emitters=105 |
| `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest` | PASS |
| `Publish-ValtanWorldDestruction.ps1 -Mode Publish` | PASS, revision `6346a764... -> 7dc81a3a...` |
| 발행된 bootstrap 실측 | 헤더 `2`, `removesGround=1`이 정확히 6행, 벽 행은 전부 `0` |
| PowerShell 파서 | PASS |
| 변경 JSON 2개 parse | PASS |
| 편집한 C++ 14개 파일 괄호 수지 | 전부 중괄호 `+0` 소괄호 `+0` |
| `git diff --check` | PASS |
| 낙사 테스트 좌표 실측 | `(155.25, -107.25)`는 `floor84.rail.7000000000000000001` 전용 cell, 아레나 코어 `(154.296, -125.219)`는 어느 region에도 속하지 않음 |

## 5. 실행하지 않은 검증

Shared·Server·Client를 모두 바꿨으므로 **세 프로젝트를 다시 빌드해야 한다.** 순서는
`Shared -> NetworkProtocolHarness -> Server -> Client`다. 프로토콜 버전을 올리지 않았으므로
한쪽만 빌드하면 접속은 되지만 첫 낙사 snapshot이 버려진다.

빌드 뒤에 돌려야 하는 것:

```text
NetworkProtocolHarness.exe          기대: failures : 0 (신규 3건 포함)
Server.exe --contract-test          기대: failures : 0 (낙사 6건, removed-ground 3건 포함)
ClientFrontendHarness.exe           기대: 신규 실패 0건
```

## 6. 사용자 확인 절차 (에이전트가 대신 판정하지 않음)

`Play Selected (Keep Broken)`은 플레이어를 순간이동시키지 않으므로 원하는 자리에 서 있다가
무너뜨릴 수 있다.

```text
1. Server + Client 실행, Lobby -> Valtan
2. Reset + Break Every Wall (Keep Floor)     벽만 사라지고 바닥 6개는 그대로
3. 우클릭으로 바깥 테두리 링 위로 이동        (반경 13~16 m)
4. 체력바 84 선택 -> Play Selected (Keep Broken)
   기대: 발밑 링이 사라지고 아래로 떨어진다. 떨어지는 동안 피격 경직 자세, 1.5초 뒤 사망 자세
5. 부활                                       기대: 구멍이 아니라 진입 spawn 위치에서 살아난다
6. 벽돌 링 위(반경 7~14 m)에서 체력바 30으로 같은 확인
7. 중앙 코어에 선 채로 84/30 재생             기대: 떨어지지 않는다
8. 낙하 도중 Reset Arena State                기대: 바닥이 돌아오고 그 자리에 다시 선다
```

## 7. 남는 경계

- **낙하 1.5초와 즉사는 원작 근거가 없는 결정이다.** 근거가 나오면 `FALL_DEATH_TICKS` 하나로
  교정할 수 있다.
- **낙하 clip이 없어 피격 경직(`CHARACTER_ANIM::HIT`) 루프로 대신한다.** 6 class 전부 낙하
  clip이 없다는 것은 clipmap 전수 검색으로 확인했다.
- **보스와 몬스터는 떨어지지 않는다.** Valtan은 무너진 바닥 위를 계속 지나갈 수 있다.
- **Client 표현에는 자동 회귀가 없다**(§2.4).
- 이 변경은 Shared를 건드리므로 PR 설명에 **팀원은 pull 뒤 Shared -> Server -> Client를 모두
  다시 빌드해야 한다**고 적어야 한다.
