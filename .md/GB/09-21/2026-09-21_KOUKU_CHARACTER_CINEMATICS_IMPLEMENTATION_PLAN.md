# 쿠크 연출 애니메이션 Character 본체 통합 구현 계획

## G00. 실측과 범위

사용자는 카드미로의 Map donor 연결로 입장이 실패한 원인을 확인한 뒤 전체 쿠크 연출을 Character 보스 모델에 굽도록 요청했다. 현재 branch는 `codex/spider-pattern-fear-sound`, HEAD는 `979601d0f62fc60265457678c192fbb37ccf68b7`이며 기존 대규모 미커밋 변경은 보존한다. Client/Server는 종료되어 있으며 MSBuild nodeReuse 프로세스만 남아 있다.

현재 Character 본체는 MN_RPCZ_00 91clip, MN_RPCT_05 249clip, MN_RPCT_06 34clip이다. 별도 Map 연출 모델은 2관문 입장 2개, 1관문 통합 3개, 2관문 클리어 3개, 카드미로 1개, 3관문 입장 1개, 빙고 앵콜 1개로 총11개다. 쇼타임 `rpct00_evt2_rpct_showtime_01`은 Character에 이미 있다. donor11개와 대응 Character 본체의 skeleton/material section은 동일하다. 최신 Character geometry를 보존하고 animation section만 추가한다.

## G01. Character 모델과 재생 참조

Tools/KoukuSaydonPipeline의 재실행 가능한 통합 도구로 기존 각 section payload를 그대로 보존하고11개 WANM을 해당3개 Character 본체에 합친 후보를 만든다. 같은 이름·같은 payload는 무변경이며 다른 payload는 거부한다. 원본·후보 hash, clip 목록, 길이, 골격과 실제 CModel pose를 비교한다. 후보 검증 뒤 최신 실물 hash를 다시 확인하고 백업·원자 교체·실패 rollback으로 설치한다.

WorldSequence의 보스 objectResources는 stable objectId를 기준으로 해당 Character 본체를 참조하도록 수정한다. Transform, modelPreScale, animation track, visibility, camera, sound, effect와 actor lifecycle은 보존한다. P77은 이미 선택한 기존 보스 재사용과 bossMotion을 유지한다. BossCatalog G2 쿠크 animationSetId는 같은 Character 본체로 복귀한다. 최초 경로 검사를 완화하지 않는다. 원본 시퀀스 생성 도구도 같은 Character 출력·참조를 사용해 재추출 시 Map 보스 파생 경로를 되살리지 않는다. 무대·소품 등 맵 고유 모델은 기존 경로를 유지한다.

## G02. 실패 표시와 부분 UI 정리

Level_Loading은 자신이 등록한 Chrome 객체만 소유 목록으로 정리하고, CUILayoutRuntime의 명시적 sprite 해제 함수를 Loading recovery view에서 호출한다. 전체 LOADING layer를 지워 다른 owner의 객체를 제거하지 않는다. Loader와 Level_Loading 초기화 실패 지점에서 기존 typed recovery 경로에 첫 구체적 사유를 보존한다. MainApp의 generic null 표시가 최초 원인을 덮지 않는 기존 계약을 사용한다. 새 C++ 파일과 프로젝트 등록은 없다.

## G03. 실제 소비와 게시·빌드

실제 Client ActorCatalog C++ reader로 변경 전 실패를 재현하고 변경 후 전체 catalog 승인을 확인한다. 변경 WorldSequence와11개 clip의 실제 모델 소비, 기존 clip 보존, 같은 입력 재실행 무변경을 검사한다. 필요한 Kouku/Map publisher로 실행 데이터를 생성하고 authoring·게시본의 참조가 일치하는지 확인한다. JSON/XML parse와 scoped git diff --check 뒤 정상 Debug Product Build를 실행한다. Client/UI 실행과 화면 판정은 사용자에게 남긴다. Resources는 Git에 추가하지 않고 필요한 Character3개 물리 파일을 결과에 기록한다.

## G04. 실행 중 발견된 ImGui 취소 경로

사용자 Debug 실행에서 ImGui11572 assertion이 발견됐다. 앞선 UI 오진 때 MainApp에 추가된 LOADING 조기 CancelFrame/return 블록만 제거하고 정상 EndFrame 경로로 복귀한다. 기존 CancelFrame 자체도 EndFrame 뒤 ViewportsEnable이면 UpdatePlatformWindows를 호출해 렌더 없이 플랫폼 프레임을 닫는다. 실제 DLL을 쓰는 headless 회귀와 Debug 제품 재빌드로 확인한다. vendor imgui 코드와 assertion은 변경하지 않는다.
