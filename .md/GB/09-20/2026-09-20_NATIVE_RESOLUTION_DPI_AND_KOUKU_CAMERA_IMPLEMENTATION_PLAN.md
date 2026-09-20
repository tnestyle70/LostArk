# 물리 해상도·DPI·ESC 저장과 쿠크 카메라 영역 수정

## G00. 목표와 실측

사용자가 ESC 설정에서 해상도·창 모드를 바꾸고 JSON에 저장하여 재실행에도 사용하도록 요청했다. 현재 Client는1280×720 고정 backbuffer/viewport이며 DPI manifest, main swapchain resize 경로가 없다. 연결 모니터에는100%와150% 배율이 있다. CUILayoutRuntime과 Loading chrome에도 초기 크기만 쓰는 경계가 있다. 쿠크의19m 근거 volume은 Z[-86.428,1.409]m로 실제 세이튼 전장 Z737m를 포함하지 않는다.

기능 브랜치는 `codex/native-resolution-dpi-kouku-camera`다. 맵/캐릭터/이펙트의 world scale은 이번 구현 대상이 아니다. 공 먹기 이펙트의 다른 source asset 연결은 조사 결과에 남기고 이 변경에서 임의 교체하지 않는다.

## G01. 창과 DPI

Client manifest를 PerMonitorV2로 선언하고 창 생성 전 사용자 설정을 읽는다. 새 ClientWindowDisplay가 기존 Win32 창의 physical client rect, windowed/borderless/exclusive mode, DPI 이동과 최소화를 소유한다. UI 명령은 CUserSettings의 typed display callback으로 전달하고, OS 창과 Engine resize가 실패하면 기존 display 상태를 복원한다. MainApp은 실제 client 크기로 Engine을 초기화하며 message loop의 렌더 전 경계에서 수동 창 변경을 반영한다.

새 H/CPP는 Client 프로젝트와 기존 관련 filter에만 등록한다. manifest를 .vcxproj AdditionalManifestFiles와 .filters에 등록하고 빌드 EXE의 실제 embedded manifest를 확인한다.

## G02. Engine render targets

기존 CGameInstance/CGraphic_Device/CRenderer/CTarget_Manager를 확장한다. Resize_Viewport는0 및 잘못된 크기를 거부하고 새 MRT/scene HDR/postprocess/half Bloom·SSAO/depth를 stage한 뒤 swapchain resize가 성공하면 viewport와 함께 commit한다. target wrapper와 MRT alias, quality, shadow 및 Level 상태를 유지한다. 실패하면 기존 크기를 보존하고 실패 이유를 caller에 돌려준다. DXGI fullscreen 전환은 별도 typed API로 처리하고 Alt+Enter의 암묵 전환은 막는다.

## G03. ESC 설정과 JSON

기존 resolution/window-mode 행을 실제 지원 선택값과 연결한다. USER_DISPLAY_SETTINGS는 physical width/height/mode를 명시하고 UI reference resolution과 분리한다. 적용/확인은 typed display 적용과 원자적 JSON 저장이 성공해야 committed settings를 바꾼다. 취소는 미저장 preview만 되돌린다. 설정 정본은 사용자 LocalAppData의 LostArk/UserSettings.json이며 source Data/UI를 개인설정으로 덮지 않는다. unknown row를 보존하고 손상·외부 수정·실패 때 기존 파일/설정을 유지한다.

## G04. UI와 카메라

공통 CUI_Sprite는 좌표 기준 해상도를 보유하고 현재 viewport로 geometry와 projection을 계산한다. CUILayoutRuntime·Loading chrome의 JSON rect/keyframe/progress/text가 같은 reference 계약을 사용한다. 기존 UIInputRouter의 reference→viewport picking과 현재 pixel font atlas 선택을 유지한다. ImGui는 현재 backend의 DPI font 지원을 사용한다.

쿠크 source baseline은 공통16m를 사용하고 원본 entrance volume 안에서만19m를 적용한다. 원본 native 최종 카메라 전체를 복구했다고 주장하지 않는다. optional useSourceCameraRegions로 source region 사용과 사용자 manual pose를 분리한다. 기존 필드 없는 문서는 manual로 보존하고, F1 manual pose 편집은 region override를 끈다. 현재 실효값을 표시하고 cinematic/free-camera 복귀가 같은 resolver를 사용하게 한다.

## G05. 검증과 완료 경계

설정 parse/roundtrip/손상·외부수정 거절, display size와 UI/picking 수치, source camera 영역 안/밖과 이전 JSON 호환을 비UI로 검사한다. 변경 JSON/XML parse와 git diff --check, 정상 Debug 및 필요한 Release Product 증분 빌드를 수행한다. 가능하면 기존 Engine에 대한 비UI WARP resize 검증으로 크기·실패 보존을 확인한다. Client 실행·화면 캡처·최종 육안 판정은 사용자가 직접 수행한다. RESULT에 코드/빌드/자동 검사/사용자 미확인을 분리한다.
