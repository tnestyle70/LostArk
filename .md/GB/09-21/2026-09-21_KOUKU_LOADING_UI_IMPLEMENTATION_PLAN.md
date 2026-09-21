# 로딩 화면의 도구·자막 오버레이 숨김 구현 계획

## G00. 현재 경로와 원인

MainApp의 일반 runtime 글자는 Is_RuntimeUIScreenSuppressed로 LOADING을 차단하지만, ImGui build/EndFrame과 별도 cinematic subtitle 호출은 이 guard 밖에 있다. Engine ImGuiLayer는 Client Level을 알지 않고 DX11/platform draw data를 그대로 제출한다. MapTool WorldSequence 자막도 활성 Level과 무관하게 수집될 수 있다. 이번 쿠크 저작/Complete Play 수정은 기존 suppression 조건을 바꾸지 않았다.

## G01. 최소 변경

Client/Private/MainApp.cpp의 Render에서 CGameInstance::Render가 성공하여 로딩 화면 자체가 그려진 직후 현재 Level이 LOADING이면 시작된 ImGui frame을 CancelFrame으로 닫고, UIInputRouter::End_Frame을 호출한 뒤 Render_End로 종료한다. 로딩 자신의 제목·팁·오류/Retry 표시는 기존 Level_Loading이 소유한다. 일반 전투/컷신에서는 기존 도구, UI, 자막과 frame lifecycle을 유지한다. Engine에 Level 조건을 넣거나 다른 UI dirty 변경을 덮어쓰지 않는다. 새 C++ 파일/프로젝트 등록은 없다.

## G02. 확인

현재 MainApp dirty 전체를 out/KoukuLoadingUi20260921/MainApp.cpp.before에 보존하고, 단일 scope diff와 diff --check를 확인한다. 사용자의 최신 지시에 따라 Debug compile/link와 화면 확인은 사용자가 직접 수행한다. 별도 제품 실행·UI 조작은 하지 않으며 실제 화면 판정은 사용자 확인으로 남긴다.

## G03. 생성 실패 후 로딩 UI 소유 객체 rollback

사용자가 제시한 실제 실패는 LOADING이 활성화되기 전이다. Level_Loading::Initialize는 Chrome와 RecoveryView를 LOADING Layer에 등록한 뒤 CLoader::Create를 호출한다. Loader의 owner-thread ActorCatalog::Initialize가 실패하면 Create는 nullptr를 반환한다. 기존 Level_Loading 소멸자는 Effect job만 취소하고 UI를 제거하지 않았으며 Object_Manager는 모든 Level의 Layer를 업데이트하므로 로비 위에 로딩 이미지가 남는다. G00~G02의 LOADING render guard는 별도 오버레이 정책이며 이 생성 실패의 원인 수정은 아니다.

Level_Loading.cpp/.h에서 생성한 Chrome sprite의 강한 참조 목록을 소유하고 소멸 시 숨긴 뒤 기존 Remove_GameObject_from_Layer로 자신의 객체만 제거한다. UILayoutRuntime.cpp/.h에는 명시적인 Remove_FromLayer를 추가해 base/extra/keyframe sprite를 같은 방식으로 제거하고 slot 목록과 lookup을 비운다. Loading 소멸자만 이를 호출하며 UILayoutRuntime의 공용 소멸자 정책은 바꾸지 않는다. 실패한 Create, profile activation 실패, 정상 Level 퇴장 모두 소유 객체 해제 경로를 공유한다. 이미 Layer가 해제됐으면 remove의 미존재 결과를 무시하고 자신의 참조만 비우므로 재호출해도 기존 화면 객체를 삭제하지 않는다. 공용 STATIC texture prototype은 캐시이므로 제거하지 않는다.

기존 네 파일을 out/KoukuLoadingUi20260921/rollback/before에 백업한다. 두 TU를 out 아래 scratch compile하고 실제 cleanup 함수의 CPU probe에서 같은 layer의 다른 owner 보존, 세 sprite 종류 제거, 재호출, Layer 선해제를 확인한다. 전체 제품 빌드와 Client/UI 실행은 하지 않는다. 기존 파일만 수정하므로 프로젝트/filter 등록은 없다.