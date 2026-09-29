# 마리오 색별 머리 표식 원본 추출 계획

## G00. 목적과 범위

사용자가 확인한 4인 테스트의 노란 표식 반복 현상과 관련해 원작의 빨강·파랑·노랑
표식이 사용하는 텍스처를 직접 추출한다. 설치된 원작 package를 읽기 전용으로 사용하고,
기존 공통 얼굴 이미지에 임의 색을 입힌 결과를 원본 추출이라고 기록하지 않는다.
최초 요청의 산출물은 확인 가능한 이미지와 출처다. 후속 구현 요청에 따라 아래 G03~G05에서
원본 색상·도형을 제품에 반영하고 서버 진행값을 읽는 HUD 텍스트를 연결한다.

## G01. 확인한 현재 연결

현재 `effect.kouku.mario.marker.red/blue/yellow`는 모두 같은
`fx_l_symbol_63_cl` 얼굴을 사용한다. 원작 `FX_MN_RPCT_05_X`에는
`Par_X_RPCT_Mark_04_Blue`, `Par_X_RPCT_Mark_04_Yellow`,
`Par_X_RPCT_Mark_04_Red_LOC_INT`가 별도로 존재한다.
원본 particle graph에서 각 variant의 실제 material 참조를 추적한다.
색별 suffix만 전체 표식으로 간주하지 않는다. 실제 buff의 ParticleSoundNew LOA에서
동시에 호출하는 공통 얼굴과 도형, Color/ColorStr 인스턴스 override를 함께 추적한다.

## G02. 추출과 검증

원본 ParticleSystem → Material Instance → Texture2D의 참조를 보존한다.
기존 UPK reader와 texture decoder로 PNG를 내보내고 원본 package/object 이름,
이미지 크기·alpha·해시와 명령을 `out/MarioMarkerExtraction20260929`에 기록한다.
색별 파일이 실제로 다른지 확인하고 원본 이미지를 열어 어떤 표식인지 확인한다.
atlas에서 사용하는 영역이 따로 있으면 원본 UV 근거와 영역을 함께 남긴다.
원본 buff의 binary parameter를 타입과 byte offset으로 확인하고 추출한 LOA를
설치 LPK에서 다시 읽어 바이트가 일치하는지 검증한다. 얼굴 색과 외곽 밝기에 적용되는
파라미터의 실제 emitter 소비자를 구분한다.

추출 단계에는 새 C++ 파일·프로젝트 등록·제품 빌드가 필요하지 않다. JSON 기록의 parse와
`git diff --check`를 검사한다. Client 실행·화면 캡처·최종 게임 표시 판정은 수행하지 않는다.

## G03. Server와 Shared의 목표색·진행 개수

`Shared/Public/Network/PacketMessages.h`의 SNAPSHOT_PLAYER Mario marker 뒤에
`iMarioRequiredColor`(0 없음, 1 빨강, 2 파랑, 3 노랑)와
`iMarioMatchingBallCount`(0..3)를 추가한다. marker는 외부 대상 머리에 표시할 색이며,
새 required 필드는 이 snapshot 입장자가 터뜨려야 하는 공 색이다.
`Server/Private/GameRoom_Replication.cpp`에서 현재 Server player의 requiredColor와
기존 `Mario_MatchingBallCount`의 결과를 최대3으로 제한해 채운다. Mario 밖에서는 두 값0이다.
랜덤 배정·공 파괴·탈출 판정은 기존 Server 계산을 유지한다.

`Shared/Private/Network/PacketMessages.cpp`의 검증/쓰기/읽기와 `PacketType.h`의
protocol120→121을 함께 변경한다. 범위를 벗어난 값, 목표색0에 양수 count,
Mario 밖의 목표색/count는 거절한다. snapshot parse 실패 시 기존 output을 보존한다.
NetworkProtocolHarness와 기존 Server Mario contract test에서 3색 진행, 다른 색 무시,
marker/목표색 분리, 잘못된 packet 및 복귀 후 초기화를 확인한다.

## G04. Client 진행 문구

`Client/Public/CombatHUDViewModel.h`의 HUD_PLAYER_STATE와 `.cpp`의 Apply_Player에서
새 목표색/count를 복사한다. `Update_MarioBallPresentation`은 일치 개수가 증가할 때만
알림을 시작한다. 진입·배치/목표색 변경·복귀는 조용히 기준값을 초기화하며 같은 snapshot과
다른 색 공 파괴는 알림을 재시작하지 않는다. `Level_KakulSaydonArena.cpp::Render`는
기존 UILabelFont/drawPrompt 경로로 화면 중앙 하단 `[1 / 3] 파란색 공` 형태로 표시한다.
사용자의 후속 요청에 따라 1초 유지 후0.4초 투명도 fade로 제거한다. SpriteBatch의
premultiplied alpha 계약에 맞춰 글자 RGB와 alpha를 함께 줄인다. 상시 표시는 하지 않는다.
폰트 코드페이지를 보존하도록 기존 방식의 Unicode escape를 사용한다.
카운트를 Client에서 다시 판정하지 않으며 기존 G/Up 안내와 중앙 저주 해제 문구를 보존한다.
`Level_KakulSaydonArena.h`는 표시용 이전 색/개수와 타이머만 소유한다.
새 C++ 파일이나 project/filter 등록은 없다. 기존 미커밋 HP/로딩 변경을 보존한다.

## G05. 원본 표식 연결과 제품 검증

기존 marker red/blue/yellow의 stable effect/element ID, 머리 위치·외부 Sample 재생 경로를
유지하면서 face Color를 원본 LOA값으로 복원한다. 외곽의 ColorStr 소비는 별도 처리한다.
위쪽 도형은 원본 texture의 빨강 별·파랑 마름모·노랑 삼각형과 RGB/alpha를 연결한다.
미등록 원본 MIC를 다른 native shader ID로 위장하지 않으며 기존 일반 particle material의
texture mask 렌더링을 사용한다. 원본 전체 shader 복원은 이번 변경에 포함하지 않는다.
원본 추출 DDS만 Resources에 설치하며 임의로 색칠한 PNG로 대체하지 않는다.
현재 디스크 파일 hash 확인, 백업 및 원자적 교체를 사용한다. Effect는 Authored 직접 소비이므로
별도 publish/복사본을 만들지 않고 기존 Effect validator로 대상 문서와 resource를 검증한다.
팀장 FXAA와 다른 렌더링 옵션은 이 변경 대상이 아니다.

변경한 JSON parse와 Effect validator 검증, Shared/Server/Client Product 컴파일,
focused protocol/Server contract 검증 및 `git diff --check`를 수행한다.
코드/설치/게시/현재 실행 프로세스 반영/사용자 화면 확인을 RESULT에서 구분한다.
public snapshot 소비 계약은 팀 인터페이스 사용서, 반복 방지는 gotchas와 복원 가이드에 갱신한다.
