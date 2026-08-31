# 2026-08-31 ImGui 게임플레이 UI → 실 런타임(CUIObject) 전환 PLAN

## 0. 배경과 범위

사용자가 라이드 입장창 작업 중 "게임에서 ImGui로 뭔가를 띄우는 게 맞는 방향이냐"고 문제
제기했고, 확인 결과 실제로 아니다 — `CLAUDE.md` 자신도 "ImGui는 authoring/debug 전용, 제품
UI는 별도 `CUIObject` 런타임"이라고 이미 명시하고 있는데, 콤뱃HUD/보스체력바/사망화면/
레이드클리어/아이템강화창/아이템획득토스트/파티창/채팅창/이번 라이드입장창까지 전부
`CHUDRuntimeView` + ImGui `GetForegroundDrawList()`로 그려지는 "임시(interim)" 상태다.
사용자가 "지금 싹 다 바꿔"라고 확정했다.

**중요 확인**: 텍스트 드로우(`CGameInstance::Draw_Text`)는 이미 `CFont_Manager`(DirectXTK
SpriteFont) 기반이라 ImGui가 아니다 — 손 안 대도 된다. 진짜 바꿔야 하는 건 **이미지 쿼드
렌더링**과 **마우스 히트테스트/클릭 소비**뿐이다.

## 1. 진행 순서 (사용자 확정)

1. 인프라 3종 구현
2. 가장 작고 새로 만든 화면(이번 라이드 입장창)에 파일럿으로 붙여 끝까지 검증
3. 검증되면 나머지 ImGui 기반 화면(콤뱃HUD 등 위험도 높은 건 마지막)을 하나씩 이관

## 2. 실측: 이미 있는 기반

- `Engine/Public/UIObject.h`의 `CUIObject`(추상, `CGameObject` 파생) — 위치/크기/트랜스폼만
  있는 얇은 기반 클래스. 이미 존재, 그대로 사용.
- `Client/Public/UI_Sprite.h`/`.cpp`의 `CUI_Sprite`(`CUIObject` 유일한 실 서브클래스) —
  단일 텍스처 사각형. 로딩화면 배경/프로그레스바가 이미 실사용 중. `Shader_VtxTex.hlsl`의
  `UIBlend` pass(alpha blend, depth 샘플 없음)로 그림.
- `CUI_Sprite`의 텍스처는 `UI_SPRITE_DESC.strTextureTag`를 `CGameInstance::Add_Component`로
  넘겨 `CPrototype_Manager`에서 그 태그로 등록된 Texture 프로토타입을 **엄격히 조회+Clone**한다
  (`Clone_Prototype`은 미등록 태그면 그냥 nullptr — fallback 없음). 로딩화면은
  `CMainApp::Ready_Prototype_For_LoadingChrome()`가 `UI/Loading/LoadingLayout.json`을 미리
  읽어 참조되는 모든 텍스처 경로를 부팅 시점에 **문서 전용으로 한 번** 프로토타입 등록해둔다.
  즉 지금은 "이 JSON 문서가 쓰는 텍스처는 미리 등록해둬야 한다"는 계약이 문서마다
  하드코딩돼 있다 — 범용 팩토리에는 못 쓴다.

## 3. Phase 1 설계 결정

### 3.1 CUI_Sprite 확장 (완료)

`Shader_VtxTex.hlsl`에 `g_TintColor`(float4, 기본 흰색)와 `g_FlipX`(bool)를 추가하고
`PS_MAIN_UI`라는 새 픽셀 셰이더로 `UIBlend`/신규 `UIBlendAdditive` pass 둘 다 이걸 쓰게
바꿨다. 두 상수 다 기본값이 "효과 없음"이라 기존 유일한 소비자(로딩배경/프로그레스바)는
동작이 그대로다.

`CUI_Sprite`에 `Set_Tint`/`Set_FlipX`/`Set_Additive`/`Set_Texture(ComPtr<SRV>)` 추가.
`Set_Texture`는 **이미 resolve된 SRV를 받는다** — `CUI_Sprite` 자신은 2번째 텍스처 캐시를
만들지 않고 얇은 렌더 프리미티브로 남는다. 로딩/캐시 책임은 호출자(팩토리)가 진다.

### 3.2 텍스처 프로토타입 등록 — 범용화

`Ready_Prototype_For_LoadingChrome()`과 같은 패턴을 **문서 하나당 함수 하나**로 반복하지
않는다. 대신 `CUILayoutRuntime`(신규) 생성자가 자신이 읽은 JSON의 slot마다 참조하는 경로를
발견하는 즉시 `CGameInstance::Get().Add_Prototype(LEVEL::STATIC, widePath,
CTexture::Create(...))`를 호출해 **그 자리에서 등록**한다 (이미 등록돼 있으면 스킵 —
`Add_Prototype`은 실행 시점 아무 때나 호출 가능, 로더 전용이 아님을 확인함). 여러
`CUILayoutRuntime` 인스턴스가 같은 경로를 참조해도 `LEVEL::STATIC` 프로토타입 네임스페이스를
공유하므로 안전하게 재사용된다.

hover 텍스처처럼 "같은 슬롯이 런타임에 다른 그림으로 바뀌는" 경우는 이 프로토타입 등록
경로를 쓰지 않는다. 확인해보니 `Client/Public/UITextureCache.h`(경로별 캐시 SRV 로더,
`ChatWindowView`/`PartyWindowView`/`SkillWindowView`가 이미 실사용 중, 주석에 "new runtime
views use this one instead of adding a third copy"라고 명시)이 정확히 이 용도로 이미 있어서
그대로 재사용했다 — 처음엔 `CHUDRuntimeView::Get_Or_Load_Texture`와 같은 로직을 또 중복
구현했다가, `Client.vcxproj.filters`에서 `UITextureCache.cpp` 항목을 발견하고 그걸로
교체했다.

### 3.3 CUILayoutRuntime (진행 중)

`Client/Public/UILayoutRuntime.h` + `Private/UILayoutRuntime.cpp`. `lostark.ui-layout`
JSON에서 `CHUDRuntimeView::Load()`와 같은 필드를 읽되, 1차 파일럿(라이드 입장창)이 실제
쓰는 부분만 실행한다: `id`, `rect`, `layers[0]`(`path`/`tint`/`additive`/`flipX`).
`shine`/`animation.frames`/`keyframeAnimationPath`는 이번 phase 범위 밖(레이드클리어 화면
등 나중에 이관할 문서가 그 기능을 쓸 때 추가).

공개 API(= `CHUDRuntimeView`가 이미 제공하는 것 중 라이드 입장창이 실제로 부르는 것만):
`Get_SlotRect`, `Set_SlotVisible`, `Set_SlotTint`(dim backdrop 알파용), `Set_SlotTexture`
(hover swap용, 위 3.2의 raw SRV 경로).

### 3.4 2D 입력 라우터 (예정)

지금은 ImGui `BeginPopupModal`이 열리면 `m_pImGuiLayer->WantsCaptureMouse()`가 자동으로
`true`가 돼서 `CMainApp::Update()`의 `CGameInstance::Get().SetInputBlocked(false, mouseCaptured)`
가 게임플레이 마우스를 자동으로 막아준다 (`Client/Private/MainApp.cpp` 626-634줄 부근).
ImGui popup을 안 쓰면 이걸 대신할 게 없다 — 새 라우터가 화면좌표→기준해상도 변환, 보이는
위젯 draw-order 역순 히트테스트, 클릭 소비, 소비한 프레임엔 직접
`CGameInstance::Get().SetInputBlocked(false, true)`를 호출해야 한다.

## 4. 다음 단계

1. `CUILayoutRuntime` 완성 (텍스처 프로토타입 자동 등록 + `CUI_Sprite` 생성 + Get/Set API).
2. 2D 입력 라우터 구현.
3. `CRaidEntryPreviewView`를 이 둘로 재작성 (ImGui `BeginPopupModal`/`GetForegroundDrawList`
   제거, hover/클릭 로직을 라우터로 교체).
4. Engine 헤더는 안 건드렸으므로(Client만 수정) Client만 재빌드하면 되지만, `CTexture::Create`
   기반 프로토타입 등록이 실제 리소스 로드 실패를 조용히 삼키지 않는지 확인 필요.
5. 사용자 실행 검증(F1/O키 프리뷰) 후 나머지 ImGui 화면 이관 순서를 다시 정한다.
