# 로딩 초기화 실패의 UI rollback과 원인 보존 결과

## G00. 반영한 동작

`CLevel_Loading`은 자신이 생성한 Chrome sprite를 보관하며 소멸 시 숨기고 Layer에서 같은 포인터로 제거한다. Clone 전 vector 용량을 확보해 Layer 등록 후 소유 목록 추가에서 재할당하지 않는다. 성공적인 Level 교체에서 Layer가 이미 정리된 경우도 허용하며 다른 Loading 인스턴스나 다른 UI의 객체는 제거하지 않는다. Renderer가 이미 보유한 shared_ptr도 visible=false를 보므로 뒤늦게 제출되지 않는다.

`CUILayoutRuntime::Release_Sprites()`는 자신의 base·extra·keyframe sprite를 숨기고 제거한 뒤 slot/index를 비운다. 반복 호출이 안전하며 Loading의 Recovery view에서만 명시적으로 호출한다. 모든 UI view의 기존 destructor 정책, 공유 texture prototype과 JSON은 바꾸지 않았다.

`CLoader::Initialize`는 실패 시 정확한 초기화 단계와 상세 상태를 기존 typed recovery에 먼저 기록한다. ActorCatalog의 실제 상태, Server 승인 class 누락, authoring snapshot 누락, Effect 준비 실패와 worker 생성 실패를 구분한다. `CLevel_Loading::Initialize`의 선행 실패도 같은 방식으로 기록한다. 기존 `CLevelTransitionService`가 최초 recovery를 보존하므로 뒤의 `MainApp::Start_Level` 일반 null 메시지가 상세 원인을 덮지 않는다. MainApp 변경은 없다.

## G01. 검증과 증거

- 다섯 C++ 파일의 실제 diff와 호출 경로를 검토했다. `Release_Sprites()`의 호출자는 Loading destructor 하나다.
- `git diff --check` PASS. 다섯 파일 모두 기존 UTF-8 BOM 없음과 CRLF를 유지했다.
- 기존 `.vcxproj`와 `.vcxproj.filters` 등록을 확인했다. 새 제품 C++ 파일은 없다.
- Layer 제거는 객체 shared_ptr의 동일성으로 찾으며 이미 없는 객체/Layer는 실패 반환만 한다. Loading 수명 종료는 이 반환을 정상적으로 허용한다.
- 정상 Engine 종료는 Level Manager를 Object Manager보다 먼저 제거하므로 Loading destructor가 Layer 제거 API를 사용할 수 있다.
- 최초 recovery 보존은 기존 service의 `g_RecoveryDiagnostic.has_value()` guard에서 확인했다. 이번 수정은 그 동작을 변경하지 않는다.

변경 전 파일은 `out/LoadingInitializationRollback20260921/*.before`, 적용 diff는 `scope.diff`, 파일 SHA·인코딩과 실행 범위는 `receipt.json`이다. 제품 Debug 컴파일·링크는 상위 작업이 단일 빌드로 수행한다. 이 하위 작업은 컴파일 완료나 native runtime 검증 성공을 주장하지 않는다.

## G02. 남은 확인

상위 작업의 ActorCatalog 수정과 합친 제품 빌드, 최초 입장과 재시도에서 실제 로딩 제목·팁 및 로비 보존 화면의 확인이 남는다. Client 실행·UI 조작·화면 캡처는 수행하지 않았다. 기존 실행 프로세스는 새 코드 적용을 위해 사용자가 다시 실행해야 한다.
