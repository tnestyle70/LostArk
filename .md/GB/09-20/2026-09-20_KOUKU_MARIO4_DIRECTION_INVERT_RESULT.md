# 쿠크 마리오4: `Mario4_Tigger_2` 이동 뒤 방향키 좌우 반전 RESULT

작성일: 2026-09-20
브랜치: `feature/maharaka-island-level` (PR #417이 열려 있고, 이 문서의 수정은 커밋하지 않은 작업 폴더 변경이다)

## 1. 결론 먼저

- **원인(수치로 확정)**: 서버의 마리오 lane 표에서 `Mario4_Tigger_2 -> Mario4_Tigger_5` 행의 `rightSign`이 `+1`인데, 그 자리를 비추는 카메라 `4Mario.follow.floor2.near`의 화면 오른쪽은 `-X`라서 → 입력이 화면 왼쪽으로 움직였다. 18개 lane 중 이 lane만 내적이 -0.999(정반대)였고 나머지 17개는 +0.99 이상이었다.
- **수정**: `Server/Private/GameRoom_Internal.h`의 그 행 부호를 `1.f`에서 `-1.f`로 바꿨다(한 줄 + 이유 주석 2줄). 서버 계약 테스트에는 빠져 있던 18번째 lane 행과 방향 단언을 추가했다.
- **오늘 우리 변경 때문이 아니다**: `origin/main`(`8dc16688`) 데이터·표로 같은 계산을 해도 같은 -0.999이고, 부호는 이 표가 헤더로 들어온 09-13 커밋 `3fc23750`부터 그대로다.
- **하지 않은 것**: 게임에서 실제 방향 확인, Server 빌드, 테스트 실행. 반영하려면 Server 재빌드와 재시작이 필요하다(게시 불필요, Client 변경 없음).

## 2. 동작 경로 (소스로 읽은 사실)

1. Client: ←/→ 물리 키만으로 `MARIO_DIRECTION::LEFT/RIGHT/STOP`을 만든다. lane이나 카메라에 따른 반전은 없다(`PlayerController.cpp:1038-1083` `Update_MarioControls`).
2. Server: `Handle_MarioMove`가 `방향 = fMarioRailRight * (RIGHT ? +1 : -1)`로 이동 목표를 만든다(`GameRoom_KoukuPlayerCommands.cpp:1224-1226`).
3. `fMarioRailRight`는 `Configure_MarioRail`이 정한다: `normalize(출구 박스 위치 - 플레이어 현재 위치) * lane->rightSign`(같은 파일 `:888-900`).
4. 트리거 이동이 끝나면 `Configure_MarioRail(player, 이동을 시작한 트리거 id)`가 호출된다(`GameRoom_PlayerSimulation.cpp:677-684`). `Mario4_Tigger_2`를 밟고 이동을 마치면 lane `{4, "Mario4_Tigger_2", "Mario4_Tigger_5", 부호}`가 선택된다. 밟는 경로(자동/G)와 무관하다.
5. Client 카메라: 겹치는 shot 중 우선순위가 가장 높은 것(동률이면 목록 앞)을 고르고, follow shot이면 눈 위치를 `플레이어 위치 + eyeOffset`(월드 축 그대로)로 둔다(`Level_KakulSaydonArena.cpp:3873-3911`, `:4527-4534`). 엔진은 왼손 좌표계(`XMMatrixLookAtLH`)다.

## 3. 방향 검증 (수치)

- **계산식**: 서버 쪽 `→ 방향 = normalize(출구 - 착지 위치) * rightSign`. Client 쪽 화면 오른쪽 = 카메라 forward 수평 성분 `(fx, fz) = lookAtOffset - eyeOffset`에서 `(fz, -fx)`(왼손 좌표계, Y up). 둘의 내적이 +1에 가까우면 → = 화면 오른쪽이다.
- **이 lane의 카메라**: 착지 위치 (-1643.85, -17.93, -1418.61)를 포함하는 shot은 `4Mario.follow.floor2.near` 하나뿐이다(우선순위 20, follow, sequence 없음). eyeOffset (0.553, 1.607, 8.850), lookAtOffset (0.310, 1.674, 7.451)이라 카메라는 -Z 쪽을 바라보고 화면 오른쪽은 (-0.985, +0.171)이다.
- **서버 레일 오른쪽**: 착지에서 출구 `Mario4_Tigger_5`(-1628.63, -17.93, -1420.72)까지는 (+0.9905, -0.1376). 부호 +1이면 → 가 +X로, 화면에서는 왼쪽으로 간다.
- **수정 전 18개 lane 전수(`origin/main` 스냅샷)**: 17개는 내적 +0.992 ~ +1.000, `Mario4_Tigger_2 -> Mario4_Tigger_5`만 -0.999(착지와 출구 양쪽 모두).
- **수정 후**: 이 lane은 부호 -1, 내적 +0.999. 나머지 17개는 수정 전후 값이 완전히 같다(자동 대조). 모든 lane의 최소 내적은 +0.992다.
- **왼손 좌표계 가정의 검증**: 문제가 보고되지 않은 17개 lane이 모두 같은 공식에서 +1에 가깝게 나오므로 손잡이와 오프셋 해석은 맞을 가능성이 높다(그 17개를 게임에서 직접 확인한 것은 아니다).
- **재실행**: `out/MarioDirectionCheck/mario_direction_check.py`(환경 변수 `MARIO_ROOT`로 다른 스냅샷을 읽을 수 있다). 이 저장소에는 넣지 않았다(git 무시 폴더).

## 4. 우리 변경과의 관계 (구분)

- **원래부터 있던 결함**: `origin/main` 스냅샷 재현 + git 이력(`3fc23750`, tnestyle70, 2026-09-13)으로 확인했다. 이 lane 행의 부호는 그 뒤 한 번도 바뀌지 않았다.
- **오늘 변경은 이 경로를 건드리지 않았다**: `Configure_MarioRail` / `Handle_MarioMove`가 있는 `GameRoom_KoukuPlayerCommands.cpp`는 변경 0건이다. `GameRoom_PlayerSimulation.cpp`(:610, :717, :722)와 `PlayerController.cpp`(:1146-1152)의 오늘 hunk는 스퀘어홀과 G 요청 쪽이다.
- `Mario4_Tigger_2`는 저작 데이터에서 원래부터 G 전용(`requiresInteract`)이었고, 이동 뒤 레일 설정은 자동/G 두 경로가 같다. 이 결함이 언제 처음 눈에 띄었는지는 알 수 없다.
- **왜 테스트가 못 잡았나**: 서버 계약 테스트의 lane 표(`ServerGameplayContractTests_DebugTeleport.cpp`)는 서버 표의 복사본인데 17행뿐이고 이 lane이 없다(메시지도 "All 17 Mario lanes"). 이 lane의 부호는 한 번도 단언된 적이 없다.

## 5. 바꾼 것

- `Server/Private/GameRoom_Internal.h`: `{4u, "Mario4_Tigger_2", "Mario4_Tigger_5", 1.f}` -> `-1.f`, 위에 이유 주석 2줄. 바이트 검증: ASCII, CRLF 484 -> 486(추가한 두 줄), LF-only 0.
- `Server/Private/ServerGameplayContractTests_DebugTeleport.cpp`: 표에 `{4u,"Mario4_Tigger_2","Mario4_Tigger_5",-1.f}` 추가, 메시지 17 -> 18, 이 lane에서 `fMarioRailRightX < -0.9f && fMarioRailRightZ > 0.f`를 단언(설명 주석 포함). 바이트 검증: ASCII, CRLF 1319 -> 1326, LF-only 0.
- 게시본으로도 확인: `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.worldbootstrap`의 위치로 레일을 계산하면 부호 -1에서 (-0.9905, +0.1376)이라 새 단언이 성립하고, 옛 부호 +1이면 성립하지 않는다.
- 데이터(`Gameplay.world.json`, 카메라 shot), Client, Shared, 프로토콜은 바꾸지 않았다.

## 6. 검증 티어

- **수치 계산**: 18개 lane 전수 내적, 수정 전(`origin/main`)과 후 자동 대조(바뀐 lane 1개, 동일 17개).
- **소스로 직접 확인**: 경로 1~5, 오늘 커밋이 이 경로를 건드리지 않았음, 부호를 쓰는 다른 곳(패트롤 축은 부호 미사용, `Project_MarioRailPoint`는 부호 무관, 디버그 점프는 같은 레일 벡터를 써서 함께 일관되게 바뀜).
- **데이터 실측**: 카메라 shot 선택과 오프셋, 게시본의 위치.
- **구문 검사만**: `cl /Zs`로 `GameRoom_KoukuPlayerCommands.cpp`, `GameRoom_WorldDestruction.cpp`, `GameRoom_PlayerSimulation.cpp`, `ServerGameplayContractTests_DebugTeleport.cpp`를 Debug와 Release로 검사, 모두 exit=0. 링크는 하지 않았다.
- **하지 않음**: Server 빌드, `Server.exe --contract-test` 실행, 게임에서 방향 확인.

## 7. 반영하려면

1. VS에서 Server를 다시 빌드한다(`GameRoom_Internal.h`는 서버 번역 단위 20개가 포함하므로 여러 파일이 다시 컴파일된다). Client는 바뀌지 않았다.
2. 데이터 게시(`Publish-WorldGameplay`)는 필요 없다.
3. Server를 재시작하고 쿠크에서 마리오4로 들어가 `Mario4_Tigger_2`를 밟아 이동한 뒤 ←/→를 눌러 본다. 기대: ←는 화면 왼쪽(출구 `Mario4_Tigger_5` 쪽), →는 화면 오른쪽. **이 확인은 사용자만 할 수 있다.**

## 8. 알려진 한계와 후속

- 카메라는 착지 직후 0.9초 동안 이전 shot에서 이 shot으로 블렌딩된다(`blendInMs`). 그 사이에는 화면이 회전 중이라 순간적으로 반대로 느껴질 수 있다. 정지 상태의 화면 기준으로 부호를 맞췄다.
- 부호는 카메라에 손으로 맞춘 값이다. 카메라 shot이나 트리거 위치를 다시 옮기면 다른 lane도 어긋날 수 있으니 위 검증 스크립트로 전수를 다시 계산한다.
- 이번에는 lane 표와 테스트 표의 중복을 없애는 구조 변경은 하지 않았다(범위 밖). 표를 바꿀 때 두 곳을 함께 고쳐야 한다.
