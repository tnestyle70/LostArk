# 로딩 초기화 실패의 UI rollback과 원인 보존 구현 계획

## G00. 현재 실패와 작업 경계

`CLevel_Loading::Initialize`는 로딩 Chrome과 Recovery UI를 전역 Layer에 등록한 뒤 Loader를 만든다. Loader 생성 실패는 Level 전환 전에 반환하지만 생성된 UI는 Layer의 shared_ptr로 남는다. 현재 Level은 Lobby이므로 로비 글자와 로딩 이미지가 함께 표시된다. `MainApp::Start_Level`은 마지막에 일반적인 null 메시지만 보고한다.

이번 작업은 Loading이 직접 만든 UI만 제거하고 초기 실패 사유를 기존 recovery 소비자까지 보존한다. BossCatalog 경로 검증 수정은 상위 작업이 담당한다. Data, Resources, MainApp과 무관한 미커밋 변경은 수정하지 않는다.

## G01. Loading이 생성한 UI의 수명

`Client/Public/Level_Loading.h`의 Chrome 포인터 목록은 해당 Loading 인스턴스가 직접 생성한 sprite만 소유한다. `Client/Private/Level_Loading.cpp`는 성공한 Clone 직후 목록에 저장하고 소멸 시 각 sprite를 숨긴 뒤 같은 포인터로 Layer에서 제거한다. 이미 Level 자원 정리로 제거된 경우도 정상이며 Layer 전체를 비우지 않는다.

`Client/Public/UILayoutRuntime.h`와 `Client/Private/UILayoutRuntime.cpp`에는 명시적 `Release_Sprites()`를 추가한다. 자기 slot의 base, extra, keyframe sprite만 숨기고 제거하고 포인터를 비우므로 반복 호출이 안전하다. 기존 모든 view의 destructor 정책은 그대로 두고 Loading destructor만 이 함수를 호출한다. Level Manager가 Object Manager보다 먼저 정리되는 Engine 종료 순서도 유지한다.

## G02. 초기 실패의 최초 원인

`Loader.cpp`의 owner-thread 초기화 실패는 target, effect job, actor catalog, Server 승인 class, character authoring snapshot, worker 생성 단계의 source와 detail을 `CLevelTransitionService::Report_Recovery`에 먼저 기록한다. Loading의 선행 초기화 실패도 같은 typed `CLIENT_LOADING_START_FAILED`를 사용한다. 기존 service의 최초 recovery 보존 규칙 때문에 뒤의 MainApp 일반 메시지가 앞선 상세 원인을 덮지 않는다. 기존 public signature와 worker-thread 실패 처리는 바꾸지 않는다.

## G03. 적용과 검증

변경 전 파일을 `out/LoadingInitializationRollback20260921`에 보존하고 기존 UTF-8 BOM 여부와 CRLF를 유지한다. 새 제품 C++ 파일은 없으며 기존 project/filter 항목을 사용한다. 자기 sprite 식별, 중복 제거 호출, 이미 지워진 Layer, 정상 Level 교체의 수명과 최초 진단 보존을 현재 소비자로 검토한다. `git diff --check`와 구조 검증을 수행하며 제품 Debug 컴파일·링크는 상위 작업의 단일 빌드로 검증한다. Client 실행과 최종 화면 확인은 사용자에게 남긴다.
