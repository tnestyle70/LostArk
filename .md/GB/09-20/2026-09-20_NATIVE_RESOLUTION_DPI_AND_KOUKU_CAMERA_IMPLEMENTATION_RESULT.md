# 물리 해상도·DPI·ESC 저장과 쿠크 카메라 영역 수정 결과

## G00. 완료 범위와 해석

브랜치 `codex/native-resolution-dpi-kouku-camera`에서 ESC의 실제 해상도/창 모드 적용과 개인 JSON 저장, PerMonitorV2, Engine resize, 공통 UI 재배치, 쿠크 카메라 구역 판정을 구현했다. Client와 UI를 실행하거나 화면을 캡처하지 않았다. 아래 자동 검증은 사용자 최종 화면 판정을 대신하지 않는다.

선명도와 구도를 구분한다. 기존 제품 소스/Debug·Release embedded manifest에는1280×720 고정 렌더와 DPI 선언 부재가 확인됐고, 이 PC에는100%·150% 모니터가 연결되어 있다.1280×720의150% bitmap 확대는1920×1080이며 관찰과 수치가 맞는다. 당시 Client가 실행 중이지 않았으므로 과거 프레임의 실제 DPI 경로까지 직접 증명한 것은 아니다. 이 확대는 UI뿐 아니라 장면 전체의 선명도에 영향을 줄 수 있다. 같은16:9 화면의 균일 확대는 맵/이펙트의 화면 점유율을 바꾸지 않는다.

참고: [Microsoft High DPI](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows), [DPI와 픽셀](https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels).

## G01. ESC 해상도와 Windows DPI

- 새 `ClientWindowDisplay`가 Win32 창을 소유하고 `CUserSettings::DISPLAY_APPLY`를 통해서만 ESC display 명령을 받는다. 새 H/CPP와 manifest는 기존 Client project/filter에 등록했다.
- `Client.manifest`는 PerMonitorV2/PerMonitor를 선언한다. 선택 해상도는 physical client pixel이며 Windows125%·150%를 화면에 다시 곱하지 않는다. Engine 초기 크기는 실제 `GetClientRect`에서 나온다.
- 일반 창은 선택한 client 해상도, 전체 화면은 DXGI exclusive mode, 전체 창은 현재 모니터 전체 크기를 사용한다. 전체 창에서는 해상도 행을 잠그고 다른 모드의 기존 선택을 보존한다.
- `WM_SIZE`는 렌더 전 resize를 예약한다. `WM_DPICHANGED`는 현재 물리 client 크기를 유지하면서 새 DPI의 창 테두리를 계산한다. 최소화의0 크기는 GPU에 전달하지 않고 기존 자원으로 제한된 background frame을 계속 처리하여 네트워크 큐/커서 잠금/프레임 제출 수명을 유지한다.
- 설정 창 아래에 실제 viewport와 현재 DPI를 표시한다. 지원되지 않는 저장 fullscreen은 startup에서 windowed로 복구하고 메모리의 실제 display도 동기화한다. 파일은 승인 없이 다른 mode로 덮지 않는다.
- 선택 크기가 실제 viewport로 구현되지 않으면 적용을 거절하고 이전 mode/style/placement/viewport 복구를 시도한다. 드라이버 자체가 복구도 거부하는 경우 오류를 남기며 성공으로 기록하지 않는다.

## G02. Engine 렌더 크기

`CGameInstance::Resize_Viewport`는 full-size RT15개, half-size RT5개, depth/scene post color·bloom 자원을 stage한다. backbuffer resize 성공 뒤 기존 RenderTarget wrapper에 교체해 MRT alias가 새 자원을 가리키게 한다. quality, LUT, source environment, shadow 자원과 scene-color capture 요청은 유지한다. viewport, texel size, 화면 quad/ortho를 함께 갱신한다.

Target_Manager의 pass-local backbuffer 참조와 D3D context 바인딩을 해제한 뒤 ResizeBuffers를 호출한다. 실패 시 이전 RTV/크기를 복구하며, RTV 복구까지 실패했는데 같은 크기 요청이라는 이유로 성공 처리하지 않는다. fullscreen mode 전환은 같은 크기여도 backbuffer 갱신을 수행한다. `DXGI_STATUS_MODE_CHANGE_IN_PROGRESS`는 완료가 아니므로 E_PENDING으로 처리하고 최종 fullscreen 상태를 확인한다. Alt+Enter의 별도 암묵 전환은 막는다.

참고: [ResizeBuffers](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers), [DXGI status](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-status).

## G03. 저장과 실패 보존

개인 정본은 `%LOCALAPPDATA%/LostArk/UserSettings.json`이다. schema `lostark.user-settings`, formatVersion1의 `display.width/height/mode`와 `values` numeric row를 사용한다. 알 수 없는 정상 numeric row도 보존한다. source Data/UI는 개인설정으로 덮지 않는다.

창 생성 전에 저장값을 읽고 Engine 초기화 후 음량/커서 등 기존 설정 소비자를 적용한다. ESC 일반 옵션 변경은 Preview이며 display와 파일은 바꾸지 않는다. 적용/확인은 후보 직렬화/parse 검증과 임시 파일 flush를 먼저 끝내고 실제 display 적용, 최신 디스크 bytes 재확인, backup을 동반한 atomic replace 순서로 저장한다. 저장 실패는 display를 되돌리고 draft를 유지한다. 취소는 마지막 Apply 당시의 snapshot으로 preview를 되돌린다. 손상 문서/외부 동시 변경은 원본을 보존하고 거절한다.

## G04.125%에서 저작한 기존 UI

현재 resolution을 명시한 UI JSON43개는 모두1280×720 reference다. HUDLayoutTool은 마우스 이동을 canvasScale로 나누어 reference 좌표로 저장하며 OS DPI를 저장 rect에 곱하지 않는다. 따라서125% 환경에서 저작했다는 이유로 기존 JSON을 일괄 ÷1.25 할 근거는 없다.

`CUILayoutRuntime`은 rect/keyframe을 reference 좌표로 유지하고 `CUI_Sprite`가 현재 viewport로 geometry와 projection을 갱신한다. 기존 UIInputRouter는 physical mouse를 같은 reference 좌표로 역변환한다. Loading 이미지·progress·글자와 쿠크 안내3개도 현재 viewport를 따른다. 탈것 설명의 fitting 캐시는 가로뿐 아니라 세로 변경도 감지한다. ImGui DPI font 지원을 켰다.

1920×1080에서는 sx=sy=1.5로 그림·배치·클릭 영역이 대응한다. 글자는 현재 목표 픽셀 크기에 가까운 atlas를 사용한다. 현재 정책은 축별 scaling이며21:9의 자동 anchor/letterbox는 구현하지 않았다. 예를 들어2560×1080에서는 sx2/sy1.5라 이미지의 가로 모양이 늘어날 수 있다. 이미지 자체가 낮은 해상도이거나 이미 흐리게 저장된 경우 디테일이 자동 복원되지는 않는다.

## G05. 쿠크 카메라의 원본 근거와 수정

원본 PS `534P5WP4TCKLJK6DPE4PSL4PXI.upk` export43의 ViewDistance1900cm/FOV50은 직접 확인했다. 그러나 BrushComponent의8정점과 actor 위치로 복원한 실제 convex 영역은 Z[-86.428,1.409]m이다. 세이튼 전장 Z737m와 G1 player(-2.45,1.32,740.37)는 이 영역 밖이다. 같은 원본 package의 minimap/mesh도 Z742~765m에 있으므로 제품의 전역 좌표 이동으로 설명되지 않는다.09-14 RESULT의 '1관문 전체19m' 해석을 정정했다.

원본 공통 IsometricCamera CDO50도/1600cm를 바깥 baseline으로 사용하고 원본 entrance 영역 안에서만19m를 선택한다.16m가 첨부 원작 프레임의 최종 카메라값이라는 보장은 없다. 원본 native update/zoom/content camera의 최종 조합과 화면 비교는 남는다.

optional `useSourceCameraRegions`를 도입했다. 필드가 없는 기존 문서는 manual pose를 유지하고 현재 Kouku JSON과 Source baseline은 region 사용을 켠다. F1의 FOV/거리/pose 수동 수정은 region을 끄며 캐릭터 크기 수정은 유지한다. authored profile과 effective profile을 분리하여 실제 값을 표시한다. 초기 spawn, 순간이동, F6/연출 복귀가 같은 resolver를 사용한다. 원본 BlendParam3초를 사용하며 smoothstep와0.25m exit hysteresis는 프로젝트 전환 정책이다. 마리오/카드미로/컷신의 개별 소유권은 유지한다.

19→16m는 동일 focus 부근에서 선형 크기 약18.75% 증가에 해당하지만 지면 전체가 같은 배율은 아니다. 맵·캐릭터·보스·이펙트의 world scale은 이번 변경에서 바꾸지 않았다.

## G06. 별도로 남는 이펙트·색감 근거

G1 바닥 주요2개는 원본 정점/배치와 설치 WModel을 비교해 반경13.4387203↔13.4387197m,6.4679908↔6.4679907m로 일치했다. 이 두 메시에서 공통 축소는 발견하지 못했으며 전체 asset 검증 완료로 확대하지 않는다.

공 먹기 P79의5837ms 십자는 현재 `effect.kouku.albion.cross.electric.impact`(G3 기반56요소)에 연결된다. 보존된 원본 action4219804/stage2/notify9~12 기록은 G1 `fx_mn_rpct_05_l.par_l_rpct_05_sk_04_2_loc_int`를18요소씩4회, scale.8로 호출한다. 동일 asset이라는 전제가 맞지 않으므로 이번 해상도/카메라 변경으로 해당 이펙트의 정확한 복원을 완료했다고 기록하지 않는다. 원본 action 최신 재추출과 사용자가 지칭한 exact occurrence 대조 후 별도 수정이 필요하다.

현재 G1 원본 LUT02/source tone은 이미 연결되어 있으며 노출·ambient·fog·일부 baked/shadow 입력의 원작 동일성은 별도 문제다. DPI 수정으로 색감/조명/재질까지 원작과 같아진다고 판단하지 않는다.

## G07. 실행한 검증

- 첫 Debug/Release Product compile/deploy 성공. 마지막 리뷰 보완분의 최종 빌드 기록은 아래 추가한다.
- 실제 제품 `UserSettingsDocument.cpp`와 `DataJson.cpp`를 직접 컴파일한 비UI 회귀64개 PASS. 저장/reload, malformed6종, unknown row, exact backup, Preview 비저장, callback 실패, 외부 저장, 실제 파일 잠금으로 atomic save 실패·display rollback·재시도를 검사했다. `Tools/UserSettingsContractHarness`에서 재실행한다. 실제 사용자 LocalAppData는 사용하지 않는다.
- 실제 최신 Engine method30개를 추출한 비UI WARP 및 주입형 DXGI 경계 검사406개 PASS. full/half RT,1920×1080/홀수/1pixel, stage/commit, alias, depth, quality 보존과 실패 복구를 검사했다. 실제 HWND/드라이버 exclusive 전환은 이 검사를 대체하지 않는다.
- 실제 ArenaCameraProfile/제품 parser를 링크한 비UI 검사97개 PASS. 원본 convex 꼭짓점, AABB와의 차이, G1/2/3 제외, height/hysteresis, legacy JSON, source flag save/load를 확인했다.
- project/filter/manifest XML, camera JSON parse, UI43개 reference 실측 PASS. diff-check PASS.
- Debug 실제 embedded manifest에서 PerMonitorV2 선언을 추출 확인했다.

검증 로그는 `out/NativeResolution20260920`, `out/UserSettingsContractHarness`, `out/NativeResolutionEngine20260920`, `out/CameraScaleInvestigation20260920`에 있다. 소스 문서/빌드/비UI 검사와 실제 사용자 화면을 구분한다.

## G08. 사용자 실행 확인

사용자가 새 Client에서 ESC → 환경설정의 해상도/창 모드를 선택하고 적용한다.1920×1080에서 하단 현재 픽셀이1920×1080인지, HUD/설정창/글자/클릭 위치, 최소화/복귀,100%·125%·150% 모니터 이동을 확인한다. 종료 후 재실행하여 저장된 mode/해상도/설정 복원을 확인한다. 전체 창 모드는 현재 monitor 크기가 기준이며 일반 창1920×1080은 테두리 때문에 같은 크기 desktop보다 바깥 창이 클 수 있다.

쿠크 G1에서 F1 → Player Follow Camera의 Source regions/Effective distance를 보고 고정된 바닥 무늬와 원작을 비교한다. source mode는 전장16m, 검증한 입구 volume 내부19m이며 manual pose/F6/연출 복귀도 확인한다. 현재 아레나의 최종 구도, UI 선명도, 드라이버 fullscreen/모니터 이동은 사용자 미확인 상태다.
