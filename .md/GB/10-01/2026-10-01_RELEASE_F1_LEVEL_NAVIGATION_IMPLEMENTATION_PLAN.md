# Release F1 Level Navigation 구현 계획

## G00. 현재 코드와 변경 범위

Debug F1에는 Lobby, Character Select, Bern, Valtan, KoukuSaydon 이동 함수가 있으나
구현과 호출이 `_DEBUG` 안에 있어 Release에서는 사용할 수 없다. Kouku 버튼은 현재
Character Select를 거쳐 다시 누르게 하지만 Lobby의 정식 KOUKU_SAYDON admission은
이미 존재한다. MAHARAKA도 같은 admission이 있으며 COLOSSEUM의 Client Resolve만
Debug에 한정되어 있다. 기존 dirty 변경과 사용자 저장 데이터는 보존한다.

## G01. 기존 Client 함수 연결

- `Client/Private/MainApp.cpp`: `RequestDebugLevelNavigation`과
  `RenderDebugLevelNavigation`을 Debug/Release 공통으로 컴파일한다. Lobby 복귀는
  `CLevelTransitionService::Request_Load`, 나머지 여섯 목적지는 기존 Lobby command를
  사용한다. Kouku의 두 번 클릭 분기를 없애고 Maharaka/Colosseum을 같은 경로로 잇는다.
  버튼은 일곱 개를 창 너비에 맞춘 격자로 표시하고 현재 목적지·loading·pending은 막는다.
  F1 내부에서 Lobby의 실제 입장 상태와 거절 이유를 표시한다.
- `Client/Public/Level_CharacterSelect.h`: 기존 `Enter_Stage` wrapper를
  `Submit_StageEntry`와 `Get_NavigationStatus`로 공통 공개한다. 새 상태는 추가하지 않는다.
- `Client/Private/Level_CharacterSelect.cpp`: 기존 stage 이름·transition source switch에
  KoukuSaydon, Maharaka, Colosseum을 추가한다. 선택 class 보존, pending 검사와 token
  rollback은 기존 `Enter_Stage`가 담당한다.
- `Client/Private/Level_Lobby.cpp`: 기존 COLOSSEUM Resolve를 Release에도 허용한다.
  입장 요청·Server 승인·loader activation은 원래 소비자를 그대로 사용한다.

일반 world에서 Lobby로 나갈 때 `Apply_LevelRequest`의 기존
`CCharacterSelectionState::Capture_ActiveWorldState`가 캐릭터별 인벤토리·외형 상태를
보존한다. Character Select 버튼은 기존 Server-approved arena를 의미하며 Lobby의
local roster 창으로 치환하지 않는다. Colosseum 버튼은 기존 직접 입장/저작 미리보기이고
Bern의 인간 4인 매칭을 시작하지 않는다.

## G02. 실패 경계와 검증

UI는 socket/packet/직접 Change_Level을 호출하지 않는다. command queue 또는 Lobby
load 실패는 생성한 token만 취소하고 owner의 상태를 F1에 표시한다. 다른 진행 중 요청과
현재 목적지는 제출하지 않는다. 로딩 중에는 기존 15초 화면 추적 제한을 적용하지 않는다.

새 C++ 파일·자료형·프로젝트 등록·JSON 게시 변경은 없다. 기존 프로젝트/filter 등록과
파일별 UTF-8(BOM 없음)/CRLF를 유지한다. focused navigation 검사로 일곱 mapping,
Release 노출, 같은 목적지·pending 거절, command 실패 token 정리와 class 보존 호출을
확인하고 최종 Product Debug/Release는 통합 담당이 수행한다. Client/UI 실행과 실제
버튼 이동 확인은 사용자가 담당한다. 실제 실행한 검사만 대응 RESULT에 기록한다.
