# Character Select Server Arena 결과

## 결론

Character Select의 `Enter Test`는 로컬 preview나 Development Map Editor로 우회하지 않는다.
선택 class를 확정한 뒤 Lobby의 기존 Server 승인 경로로
`WORLD_ID::CHARACTER_SELECT_ARENA`에 입장하고, 같은
`LV_LOBBY_CLASSSELECT_SL00` visual map을 Server gameplay Level로 다시 연다.

재진입한 Character Select는 preview character를 만들지 않는다.
`CClientReplication -> CPlayerController -> IPlayerCommandSink` 경로로 우클릭 이동과
quick-slot 입력을 Server command로 제출하고, HUD와 캐릭터 상태는 Server snapshot을 소비한다.
Server 연결 실패·거부·timeout에는 자동 local fallback이 없으며 Lobby에 남는다.

Lobby의 일반 `Test` 명령은 별도 목적값인 `MAP_EDITOR_WORKSPACE`를 사용해 기존처럼
Server 승인을 거친 뒤 `LEVEL::DEVELOPMENT` Map Editor로 들어간다. 두 Test 경로는
동일한 문자열을 사용하더라도 command purpose와 target world로 분리된다.

## 반영한 계약

- Shared protocol version을 7로 올리고 `WORLD_ID::CHARACTER_SELECT_ARENA`를 추가했다.
- Server가 Character Select arena 전용 room, world bootstrap, navigation을 로드한다.
- Character Select `Enter Test`가 `SERVER_GAMEPLAY` handoff를 stage한다.
- Lobby가 Character Select gameplay와 Development Map Editor 목적을 분리해 승인한다.
- gameplay 재진입은 replication, network command sink, player controller, HUD를 연결한다.
- Character Select 월드의 class-neutral player spawn 4개와 Server nav grid authoring을 추가했다.
- World/Navigation publisher, Server contract test, protocol harness, Client frontend harness,
  ProjectAudit 계약을 함께 갱신했다.

## 자동 검증

다음 검증을 2026-08-04에 실행했다.

- ClientFrontendHarness Debug x64: failures 0
- Server Debug x64 build: 성공
- `Server.exe --contract-test`: failures 0
- Client Debug x64 build: 성공
- NetworkProtocolHarness Debug x64: failures 0
- World gameplay publisher Validate: Character Select arena 포함 성공
- Server navigation publisher Validate: Character Select 42 x 60 grid 포함 성공
- 변경 JSON 3개 parse: 성공
- Client project/filter XML parse: 성공
- 변경 범위 `git diff --check`: 오류 없음

ProjectAudit의 Effect 전용 G1 및 이번 Character Select 계약은 통과했다. 전체 감사에는
폐기하기로 한 resource pack lock inventory 불일치 한 건이 남아 있으며, 이번 변경에서
immutable pack을 다시 만들거나 lock을 갱신하지 않았다.

## 수동 검증 대기

사용자가 직접 foreground에서 다음 순서로 확인한다. 이 문서는 아직 수동 runtime PASS를
주장하지 않는다.

1. `Server/Default`를 working directory로 `Server/Bin/Debug/Server.exe`를 실행한다.
2. `Client/Default`를 working directory로 `Client/Bin/Debug/Client.exe`를 실행한다.
3. Lobby에서 Character Select로 들어가 class를 고른 뒤 `Enter Test`를 누른다.
4. 같은 경기장이 `LostArk Character Select Arena - Server Gameplay` 제목으로 다시 열리는지 확인한다.
5. 선택 class 캐릭터와 해당 HUD가 Server snapshot으로 표시되는지 확인한다.
6. 바닥 우클릭으로 이동하고 Q/W 등 class quick slot으로 스킬을 사용한다.
7. Server 종료 시 replicated state가 제거되고 Lobby로 복귀하는지 확인한다.

현재 Server navigation은 경기장 범위를 덮는 uniform 42 x 60 grid다. 이동 command 검증에는
사용할 수 있지만, 장식물과 비보행 영역을 반영한 최종 geometry bake/nav paint는 별도
MapTool authoring 작업으로 남아 있다.

### 2026-08-04 입력 회귀 교정

첫 수동 smoke에서 Character Select arena의 Q/W 입력이 애니메이션으로 이어지지 않았다.
Server action snapshot과 `CCharacter::Apply_NetworkAction` 연결은 존재했지만, arena의
`Back to Lobby` ImGui 디버그 창이 키보드 navigation focus를 소유했다. 전역 입력 차단이
`ImGui::GetIO().WantCaptureKeyboard`를 gameplay 차단으로 변환하므로 command 제출 전에
키 입력이 제거되고 있었다.

arena 디버그 창에 `ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav`를 적용했다.
버튼은 마우스로 계속 사용할 수 있지만 Q/W/E/R/A/S/T/V/Alt+V는 gameplay controller로
전달된다. 이 교정 뒤의 실제 animation 재생은 사용자의 두 번째 foreground smoke 대기다.

## Git 경계

`Client/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid`는 publisher가 만든 runtime
산출물이며 Git 정본이 아니다. Git 정본은 `Data/Navigation`과 `Data/Worlds`의 authoring 문서다.
현재 작업 트리에는 다른 세션 변경도 있으므로 이 결과 작성 시 자동 stage/commit을 하지 않았다.
