# Character Select 유휴 Debug 성능 개선 구현 계획

## G00. 목표와 현재 기준

사용자는 같은 commit, Debug x64, 같은 해상도, Ctrl+F5에서 이 PC 약 50fps와 다른 PC 약 10fps를 보고했다. 후속 목표는 이 PC Character Select에서 스킬을 쓰지 않는 상태의 100fps 이상이다. 이 PC는 i5-13500 / RTX 4070이며 다른 PC 사양과 현재 유휴 캡처는 제공되지 않았다. 기존 저장 캡처의 장면과 계측 분모를 확인한 뒤 참고하며 다른 PC 원인을 확정하지 않는다.

시작 HEAD는 `50e4c2579d98cedf3dd25a01a6428463f9623965`, 작업 브랜치는 `codex/character-select-idle-performance`다. 기존 이펙트·animevents·문서 미커밋 변경을 보존하고 자동 stage/commit하지 않는다. LAN 설정은 server-host, TCP 7777 방화벽 정상, endpoint not-listening이다.

## G01. Client.cpp의 60fps 제한과 메시지 처리

`Client/Default/Client.cpp::wWinMain`은 `Timer_Default`의 delta를 busy-loop에서 더하고 1/60초가 지난 경우에만 Update/Render를 호출한다. 60fps보다 빠른 렌더를 원천적으로 막으며 limiter 대기는 Profiler Begin_Frame 밖에 있다. 이 제한을 제거하고 대기 중 Windows 메시지를 먼저 처리한 뒤 실제 프레임 delta로 Update/Render를 한 번 수행한다. WM_QUIT와 Render 실패 종료는 유지한다.

`Timer_60`은 `RaidEntryPreviewView::Render`가 소비하는 프레임 delta 이름이므로 기존 이름을 유지한다. 사용하지 않게 되는 Timer_Default는 제거하고 RaidEntryPreviewView의 설명을 현재 계약으로 교정한다. Server fixed tick, gameplay authority와 애니메이션 초 단위 시간은 변경하지 않는다.

## G02. CUILayoutRuntime의 반복 ID 검색

`MainApp::Update_CombatHUD`, Character Select class 목록과 닫힌 CharacterInfo/AvatarBook 창의 Hide는 매 프레임 다수의 Set/Get을 호출한다. 현재 `Client/Private/UILayoutRuntime.cpp`의 각 호출은 `m_Slots`를 처음부터 문자열 비교한다.

`Client/Public/UILayoutRuntime.h`와 대응 CPP에 runtime-only ID 검색 인덱스를 두고 Load와 Ensure_RuntimeSlot의 실제 추가 지점에서 함께 갱신한다. vector는 문서 draw 순서의 owner로 유지한다. 인덱스는 저장 ID가 아니며 pointer를 저장하지 않아 vector 확장 뒤에도 유효하다. 동일 ID의 인덱스 목록을 문서 순서로 보존해 일반 setter의 첫 일치와 flipbook/keyframe의 첫 적격 항목 선택을 모두 유지한다. unknown ID no-op/false와 null sprite 판정도 보존한다. 공개 API, JSON, visibility override 순서는 바꾸지 않는다.

닫힌 `CharacterInfoWindowView`, `AvatarBookWindowView`, `InventoryView`의 Hide는 해당 view 전체 슬롯을 숨기는 호출이다. 각 CPP의 Hide에서 기존 비가시성 외 정리 상태는 유지하고 `Set_AllSlotsVisible(false)` 한 번으로 처리해 개별 ID 구성과 조회도 제거한다. 실제 authoring/runtime 슬롯 범위와 기존/변경 visibility 결과를 대조한다.

## G03. Deferred 조명의 대상 아닌 픽셀 조기 제외

`Engine/Private/Renderer.cpp::Render_Lights`는 화면에 제출된 source character material row마다 기존 조명 pass를 다시 실행한다. `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`은 기존 row/marker discard 전에 directional shadow 및 local world-position 계산을 수행한다.

같은 marker와 row 조건을 먼저 검사해 해당 pass가 처리하지 않는 픽셀을 비싼 조명 계산 전에 제외한다. 실제 대상 픽셀의 재질·그림자·감쇠 식, pass 번호, MRT와 리소스 binding은 유지한다. 원본/후보를 독립 headless 출력 비교로 확인하며 화면 품질을 낮춰 목표를 맞추지 않는다.

## G04. 검증과 사용자 실행 경계

기존 파일의 인코딩과 줄바꿈을 보존한다. 신규 제품 C++ 파일이 없으므로 vcxproj/filters의 항목 추가는 없다. ID 조회는 실제 함수와 runtime slot 추가·중복·누락 조건을 CPU 검사하고 같은 Debug 조건으로 비용을 비교한다. 변경된 C++ 최소 컴파일, shader compile/수치 비교와 git diff --check를 실행한다. 다른 빌드와 출력 경로가 겹치면 독립 out 경로에서 컴파일하고 최종 제품 빌드는 직렬화한다.

Client/UI는 에이전트가 실행·조작·캡처하지 않는다. 최종 실제 100fps 달성과 다른 PC 개선율은 사용자 수동 실행으로 확인한다. Profiler CPU 처리 시간과 프레임 간격, GPU 구간, Capture 자체 비용을 구분하고 미측정 FPS를 완료로 기록하지 않는다.
