# 2026-08-31 ImGui 게임플레이 UI → 실 런타임(CUIObject) 전환 RESULT

대응 PLAN: `2026-08-31_ImGui_런타임UI_전환_PLAN.md`

## 0. 사용자 확정 범위

- 최종 지시: **"ImGui는 이제 툴 쪽에서만 쓰고 게임플레이 레벨에서 절대 사용하면 안 된다.
  다 확인해서 고쳐라."**
- 따라서 `#ifdef _DEBUG` Developer Tools(F1 허브, MapTool, BalanceTool, EffectTool,
  HUDLayoutTool, CameraTool, Animation_Tool, ActionCompositionWorkbench, BossTool,
  CharacterPreviewPanel, HitAreaWire, Lobby/Loading 진단 창)는 ImGui 유지가 정답이며 이관
  대상이 아니다.
- 명시적 제외(사용자 지시): `ScreenUI.json`, `SkillWindowView`. K키 스킬창 토글은 기능
  자체를 제거했다.

## 1. 인프라 (구현 완료)

| 파일 | 역할 |
|---|---|
| `Client/Public|Private/UILayoutRuntime.*` | `lostark.ui-layout` JSON → 실제 `CUI_Sprite` GameObject 팩토리. `Get_SlotRect`(참조 해상도 단위), `Set_SlotVisible/Tint/Alpha/Texture/Position/Rect/FillRatio`, flipbook `Update` |
| `Client/Public|Private/UIInputRouter.*` | 네이티브 `GetCursorPos`+`ScreenToClient`와 `Get_DIMouseStateRaw` 기반 히트테스트/클릭 엣지/드래그/마우스 클레임. ImGui 미사용 |
| `Client/Public|Private/UI_Sprite.*` | `Set_Tint`/`Set_FlipX`/`Set_Additive`/`Set_Texture`(런타임 SRV 오버라이드)/`Set_FillRatio` |
| `Client/Bin/ShaderFiles/Shader_VtxTex.hlsl` | `g_TintColor`, `g_FlipX`, `g_FillRatio` 전역과 `PS_MAIN_UI`, `UIBlendAdditive` pass |

### 텍스트 입력 (ImGui::InputText 대체)

`CUIInputRouter`에 `Start_TextInput`/`Stop_TextInput`/`Is_TextInputActive`/`On_Char`/
`Take_TypedChars`를 추가하고, `Client/Default/Client.cpp`의 `WndProc`이 `WM_CHAR`를
라우터에 공급한다. 한글 확정 글자는 IME → `WM_IME_CHAR` → `WM_CHAR` 표준 경로로 같은
스트림에 들어오므로 위젯 쪽 IME 처리가 필요 없다. 조합 중 문자열은 기존
`Engine::CImGuiLayer::Get_ImeCompositionString()`을 그대로 읽어 인라인으로 그린다.

**남은 기존 이슈**: OS(TSF)가 띄우는 흰 조합 박스는 이 작업과 무관하게 그대로 뜬다.
위젯 종류(ImGui vs 커스텀)와 무관한 TSF 레벨 문제이며 이전에 3회 시도 후 보류된 상태다.

## 2. 이관 완료 화면

| 화면 | 커밋 | 담당 |
|---|---|---|
| DeadScene, RaidClear | `f1f8f264` | 백그라운드 |
| Inventory 패널(창 드래그/아이템 드래그앤드롭 포함) | `43f51524` | 백그라운드 |
| ClassSelect 로스터(아코디언 6행 신규 슬롯 15개) + CreateCharacterModal chrome | `622ee5e8` | 백그라운드 |
| ItemUpgrade 프리뷰(97슬롯) | `0fd0b1df` | 백그라운드 |
| BossUI(체력바/스태거/구분선/틱플래시/히트글로우) + 닉네임 입력창 ImGui 제거 | `f080c093` | 메인 |
| EstherUI 게이지 | `0f5e2d37` | 백그라운드 |
| RaidEntryPreviewView, Level_Loading 복구 패널, WorldPlayerChatBubble, EstherCutin 시계 | `5aa52b28` | 메인 |
| Character Select 제품 텍스트(`Render_ClassListText`, `Render_ProductStatus`) + `Is_ProductPointerHovered` | `9cc5e210` | 메인 |

### 주요 설계 결정

- **마커 슬롯 보호**: authored tint 알파 0인 위치 전용 슬롯(보스 초상화, 텍스트박스, 티어
  배지 등)에는 `Set_SlotVisible(true)`를 절대 호출하지 않는다. 이 API는 tint를 불투명
  흰색으로 리셋하므로 마커가 흰 박스로 보이게 된다. 표시/숨김 집합에서 명시적으로 제외했다.
- **알파 페이드 슬롯**: `Set_SlotVisible(true)` 다음 줄에서 `Set_SlotAlpha`를 호출하면 알파가
  덮어써진다(BossUI 틱플래시/히트글로우에서 실제로 발생했던 버그). 보일 때는 알파/틴트 자체가
  가시성이고, 숨길 때만 `Set_SlotVisible(false)`를 부른다.
- **드로우 순서**: `RENDERGROUP::UI`는 `Add_RenderObject` 호출 순서대로 그린다. 이는 레이어
  내 객체 생성 순서이며, `LEVEL::STATIC`(HUD/인벤/보스바)이 현재 Level UI보다 먼저 그려진다.
  따라서 Level 소유 팝업이 STATIC HUD 위에 올라오며, 이는 기존 ImGui foreground-drawlist
  순서 요구사항과 동일한 결과다. Bern/CharacterSelect 모두 팝업 뷰가 배경 뷰보다 나중에
  생성되는 것을 확인했다.
- **텍스트**: `CGameInstance::Draw_Text`는 원래부터 ImGui가 아닌 DirectXTK SpriteFont라
  그대로 쓴다. ImGui 드로우리스트 텍스트만 이쪽으로 옮겼다. `Draw_Text`에는 워드랩이 없어
  긴 상태 문구는 authored 폭에 맞춰 축소한다.
- **Escape**: ImGui 모달이 소비하던 Escape를 팝업이 직접 down-edge로 처리한다.

## 3. 입력 게이트 통합

`ImGui::GetIO().WantTextInput`을 읽던 모든 게임플레이 게이트에 `CUIInputRouter::Get()
.Is_TextInputActive()`를 함께 검사하도록 추가했다(디버그 툴 타이핑은 여전히 ImGui가
소유하므로 둘 다 필요하다).

- `MainApp`: I/P/1~4/Enter 키바인드, `keyboardCaptured` → `SetInputBlocked`
- `PlayerController`: raw-keyboard passthrough 경로
- `Camera_Free`: `Update_Shortcuts`, `Update_FreeCamera`
- `Level_CharacterSelect`: O키 디버그 프리뷰
- `Level_ValtanArena`: F6 레퍼런스 카메라 해제, O키 디버그 레이드클리어
- `Level_Bern`: O키 디버그 프리뷰

## 4. 미완료 (백그라운드 진행 중)

- 콤뱃 HUD(`HUD_Layout.json`, `m_pHUDRuntimeView`) — 키프레임 애니메이션/클래스별 슬롯 전환
  포함, 최고 위험도
- `m_pLobbyBackgroundView`(BACKGROUND 드로우 타겟)
- `Render_ItemQuickSlots`, 플로팅 데미지 넘버 등 `MainApp` 잔여 제품 드로우
- `InventoryView` 수량 텍스트 + `ImGui::SetTooltip` 툴팁
- `PartyWindowView` 전체
- `ChatWindowView` — 제품 경로 마지막 `ImGui::InputText`
- `SkillWindowView` — K키 제거로 도달 불가능한 죽은 코드. `MainApp`의 생성/`Render()` 호출
  제거 예정(파일 자체는 보존)
- `MainApp`의 `ImGui::GetTime()` 시계 사용 → `std::chrono::steady_clock` 교체

## 5. 검증 상태

- **자동**: `git diff --check` 통과. 전체 `Client/Private/*.cpp`를 전처리기 `#ifdef _DEBUG`
  중첩을 추적하는 스크립트로 스캔해 제품 경로의 ImGui 드로우/입력 호출 잔존 여부를 확인했다.
  현재 잔존은 전부 위 "미완료" 목록(백그라운드 담당)과 주석뿐이다.
- **빌드**: 이 세션에서 실행하지 않았다(사용자가 직접 빌드하는 워크플로).
- **실행/시각 판정**: 사용자 전용. 에이전트는 판정하지 않는다.

### 사용자 확인 경로

1. Character Select: 클래스 목록 텍스트/아코디언, 캐릭터 생성 창(한글·영문 타이핑, 백스페이스,
   엔터 확정, ESC 취소, 열려 있는 동안 P/I/WASD 차단)
2. Bern: 발탄 입장 NPC → 레이드 입장창(호버/입장하기/닫기/Esc, 2단계 수락·거절)
3. Valtan: 보스 체력바(색상 순환, 스태거, 구분선, 피격 글로우), 사망 화면, 레이드 클리어
4. 로딩 실패 복구 패널(Lobby 로드 실패 시), 월드 채팅 말풍선
