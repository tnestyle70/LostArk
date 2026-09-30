# Release F1 Level Navigation 결과

## G00. 실제 연결

기존 F1 Level Navigation을 Debug/Release 공통으로 연결했다. Lobby, Character Select,
Bern, Valtan, KoukuSaydon, Entrance PvP Arena, Maharaka 일곱 버튼을 창 너비에 따른
1~3열 격자로 표시한다. KoukuSaydon은 기존 Lobby의 KOUKU_SAYDON admission으로 한 번에
요청하며, Character Select에서 다시 누르도록 하던 F1 전용 분기를 제거했다.

`MainApp.cpp`의 기존 request/render 함수와 허브 호출만 공통 빌드로 옮겼다.
`Level_CharacterSelect.h`의 `Submit_StageEntry/Get_NavigationStatus`는 기존 `Enter_Stage`와
상태를 노출하고, CPP의 stage 이름·source switch에 Kouku/Maharaka/Colosseum을 추가했다.
`Level_Lobby.cpp`는 기존 COLOSSEUM Resolve의 Debug 제한만 제거했다. 새 런타임·packet·
프로젝트 등록·JSON 변경은 없다. 네 C++ 파일의 UTF-8(BOM 없음)/CRLF와 선행 dirty 변경을
보존했다. 이번 변경만의 diff·원본 바이트·SHA는 `out/ReleaseF1Navigation20261001`의
`navigation-only.diff`, `before/`, `edit-receipt.json`에 있다.

## G01. 입장·실패 경계

Lobby 복귀는 typed Level request이며 나머지는 `CLobbyCommandService`를 거친 기존
Server 승인 입장이다. Character Select의 선택 class 보존과 일반 world에서 나갈 때의
`Apply_LevelRequest -> Capture_ActiveWorldState`를 유지한다. 요청 실패는 생성한 token만
취소하고 현재 목적지·pending·loading·Lobby 입장 대기 중에는 버튼을 막는다. 로딩 중에는
15초 화면 추적 timeout을 적용하지 않는다. Lobby의 실제 입장 상태·거절 문구는 F1의 Entry에
표시한다. 별도 상시 Release 상태 문구 제거는 같은 날 RELEASE_STATUS_BANNERS 결과를 따른다.

Server의 `Is_Known_World_Id`, shared room 준비, `Acquire_EntrySimulation`,
`Bind_AndEnqueueEntry`, `Stage_PlayerEntry`는 Release에서도 COLOSSEUM을 허용한다.
Entrance PvP Arena는 기존 직접 입장이다. matchId가 없는 이 입장으로 인간 4인 매칭이나
PvP 경기를 새로 시작하지 않으며, Bern의 기존 매칭 경로는 그대로다. Character Select는
Server arena이고 Lobby의 local roster 창으로 대체하지 않았다.

## G02. 실행한 검증

- 실제 `RequestDebugLevelNavigation`, `Enter_Stage`, 두 stage switch,
  `CLobbyCommandService` 함수를 추출한 MSVC native 검사 272 PASS/0 FAIL.
  Lobby/일반 Level의 목적지·token, 일곱 선택 class와 다섯 목적지, 같은 목적지·loading·
  pending 거절, 새 token 취소·기존 token 보존을 확인했다. transition/Lobby/game-instance
  경계는 fake이며 실제 Client·Server·socket 실행 검사가 아니다.
- 기존 focused navigation 검사 4개 PASS. 전체 파일은 변경 전 8개 중 5 PASS/3 ValueError,
  변경 후 10개 중 7 PASS/같은 3 ValueError다. 기존 resource 함수의 위치·이름을 기대하는
  검사이며 전체 PASS라고 기록하지 않는다. 이번 이동과
  무관한 `RefreshDebugAuthoringSources/RenderDebugResourceFiles` 검사는 변경하지 않았다.
- request/render/call/wrapper와 Colosseum Resolve의 Release 조건 경계, 전처리 균형,
  Client project/filter의 기존 등록·XML parse, scoped `git diff --check` PASS.
- 독립 코드 검토에서 추가 수정 사항 없음. 선택 class·token·거절 표시와 상태 문구 삭제,
  캐릭터 생성 modal 안내 보존을 함께 확인했다.

native 근거는 `out/KoukuEntryAudio20261001/navigation-results.json`,
`navigation-results.log`, `navigation-compile.log`, `run-navigation-probe.ps1`이다.
focused 로그는 `navigation-static-tests.log`, 기존 전체 실패는
`navigation-full-suite-baseline-observed.log`와 `navigation-full-suite-current.log`다.
AGENTS, CLAUDE, 팀 사용서에는 바뀐 공통 F1 이동 계약만 갱신했다.

## G03. 빌드·사용자 확인 경계

최종 수정본의 정상 Product Debug/Release Build 모두 PASS이며 skippedBuild=false다.
영수증은 out/BuildPipeline/runs/20260930T194308285Z-debug-product.json과
20260930T194348823Z-release-product.json이다. 양쪽 missingRuntimeInputs와
invalidRuntimeInputs는 빈 배열이다. 로그는 out/ReleaseF1Navigation20261001의
product-debug.log, product-release.log와 build-debug/, build-release/에 있다.

최종 Debug Client는 OBJ65/CSO0/EXE1을 갱신했다. Release Client는 직전 정상 빌드가 만든
현재 소스의 산출물을 증분 재사용하여 이번 Build의 출력 쓰기는0이다. 변경한 MainApp,
WorldSequencePlayer, Level_KakulSaydonArena, Level_CharacterSelect, Level_Lobby의
Release OBJ는 모두 소스 변경 뒤인04:40:26~04:40:53 KST에 생성됐고, Release Client.exe는
04:41:42 KST에 링크됐다. 최종 후속 Build도 이를 최신 상태로 확인했다. 동시에 진행된 다른
기능의 변경도 포함된 작업 폴더이므로 전체 컴파일 수를 이 기능만의 비용으로 해석하지 않는다.
Clean/Rebuild, Client/UI 실행, JSON·Resources publish는 수행하지 않았다.

사용자는 새 실행 파일에서 F1 → Level Navigation의 일곱 목적지와 Server 실패 시 Entry
문구를 확인한다. 실제 화면·버튼 이동 성공은 아직 판정하지 않았다.
