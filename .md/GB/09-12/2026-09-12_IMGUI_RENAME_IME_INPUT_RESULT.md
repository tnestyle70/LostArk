# ImGui Rename 한영 입력 경로 수정 결과

## G00. 요청과 현재 상태

2026-09-12 사용자가 Composition Patterns의 Rename에서 한영 전환이 되지 않는다고 보고했다. 다른 Rename도 사용하는 공통 Win32 IME 경로를 수정했다. `KoukuSaydonActionWorkbench`의 Parent/Append Pattern 변경은 별도 작업이며 이 수정에서 해당 CPP/H를 편집하지 않았다.

**소스 적용·CPU 검증·Engine/Client 최종 Debug 빌드 완료, 사용자 화면 입력 확인은 남아 있다.** Client/UI를 실행하거나 조작하지 않았다. 이 기록은 실제 사용자 PC의 한영 전환 성공 판정이 아니다. 최종 Product 빌드는 18:00:06에 Engine·Shared·Server·Client 모두 PASS로 완료됐다. Engine 원본과 Client 배포 DLL SHA 일치 및 backend object 최신성을 확인했다. 근거는 `out/KoukuPopupFinale20260912/implementation/final-build-verification.json`이다. 사용자의 별도 명시 요청으로 기존 실행본 복사본만 시작했으며 새 IME 기능의 UI 검증은 하지 않았다.

## G01. 실제 호출 경로와 확인한 결함

`CKoukuSaydonActionWorkbench::Render_RenameControl`은 UTF-8 `char[256]`에 일반 `ImGui::InputText`를 사용한다. `EnterReturnsTrue | AutoSelectAll`만 사용하며 영문 전용 필터나 매 프레임 이름 초기화는 없다. `WM_CHAR`는 `Client/Default/Client.cpp::WndProc` → `CImGuiLayer::HandleWindowMessage` → `ImGui_ImplWin32_WndProcHandlerEx`를 거쳐 ImGui에 들어간다. 한영키를 Rename 전용 코드에서 가로채는 경로도 발견하지 않았다.

공통 backend에는 다음 세 결함이 있었다.

1. `ImGui_ImplWin32_PlatformSetImeData`가 `WantVisible`에 따라 `ImmAssociateContextEx(hwnd, nullptr, ...)`를 호출해 HWND의 IME context를 분리했다가 재연결했다. caret 위치 갱신이 입력기 사용 가능 상태까지 변경한다. 저장소에 함께 있는 ImGui 기본 구현의 동일 호출은 이미 주석 처리돼 있다. IME context 분리는 Windows 입력기에 영향을 주며, upstream도 이 호출의 일부 환경 멈춤 문제를 다뤘다. [Microsoft API 계약](https://learn.microsoft.com/en-us/windows/win32/api/imm/nf-imm-immassociatecontextex), [ImGui 이슈 5535](https://github.com/ocornut/imgui/issues/5535).
2. `WM_IME_SETCONTEXT`에서 조합창과 모든 후보창 표시 비트를 무조건 제거했다. 채팅·닉네임은 `Get_ImeCompositionString`을 직접 그리지만 Rename의 일반 InputText는 확정된 WM_CHAR만 그린다. 따라서 Rename에서는 아직 확정되지 않은 한글 조합이 보이지 않는 구조였다.
3. backend가 `DefWindowProcW`를 직접 호출한 뒤 그 결과 0을 그대로 반환하면 `CImGuiLayer`는 이를 미처리로 해석하고 Client WndProc 또는 분리 viewport WndProc가 같은 메시지를 다시 기본 처리한다. SETCONTEXT, 일부 COMPOSITION, ENDCOMPOSITION이 해당됐다.

소스에서 위 결함을 확인했으며, 사용자 PC에서 어느 결함이 한영 전환 실패를 직접 일으켰는지는 UI 재현 없이 확정하지 않는다.

## G02. 적용한 변경

| 파일 | 적용 내용 |
|---|---|
| `Engine/External/imgui/backends/imgui_impl_win32.cpp` | caret callback의 context 분리/재연결 제거. 기존 Windows IME context와 사용자의 한영 모드를 유지하고 caret/candidate 위치만 갱신한다. |
| 같은 CPP의 `WM_IME_SETCONTEXT` | ImGui text input이면 OS 조합창을 허용한다. runtime 자체 입력이면 조합창만 숨기고 후보창은 유지한다. 기본 처리를 한 번 실행한 뒤 handled를 반환한다. |
| 같은 CPP의 `WM_IME_STARTCOMPOSITION` | 같은 HWND에서 채팅/닉네임과 Rename를 오가도 조합 시작 시 UI 표시 정책을 다시 적용한다. 일반 InputText는 기본 START 처리로 조합창을 열고, 자체 inline 입력은 START를 소비한다. context나 conversion mode는 변경하지 않는다. |
| 같은 CPP의 COMPOSITION/ENDCOMPOSITION | 기존 조합 버퍼와 확정 WM_CHAR 경로를 보존하고 중복 기본 처리를 막는다. |
| `Engine/Public/ImGuiLayer.h` | 기존 `Get_ImeCompositionString` 주석을 일반 ImGui 입력과 runtime 자체 입력의 실제 소비 경계에 맞게 수정했다. API/ABI 변경은 없다. |

조합 문자를 직접 그리는 앱은 STARTCOMPOSITION을 처리하고, 기본 조합창을 쓰는 경우 DefWindowProc로 전달한다. SETCONTEXT의 조합창 표시 비트도 이 구분을 따른다. [Microsoft STARTCOMPOSITION](https://learn.microsoft.com/en-us/windows/win32/intl/wm-ime-startcomposition), [Microsoft SETCONTEXT](https://learn.microsoft.com/en-us/windows/win32/intl/wm-ime-setcontext).

기존 파일의 UTF-8 BOM 없음과 작업공간 줄바꿈을 유지했다. backend는 CRLF이며 header의 기존 17행 LF도 보존했다. 프로젝트 등록이나 새 C++ 파일은 필요하지 않다. 변경된 backend는 기존 Engine 프로젝트의 컴파일 항목이므로 배포에 새 Engine.dll이 필요하다.

## G03. 실행한 검증

- `cl /std:c++20 /EHsc /utf-8 /Zs /D ENGINE_EXPORTS`로 실제 `imgui_impl_win32.cpp` 구문 검사 통과. 최초 독립 명령은 Engine export 정의를 빠뜨려 dllimport 정의 오류가 났고, 실제 Engine 프로젝트 정의를 넣은 명령에서 통과했다.
- `out/KoukuPopupFinale20260912/ime-message-check.cpp`에 수정한 실제 callback과 IME switch case를 추출하고 Win32 API만 mock한 CPU 검사를 컴파일·실행했다. ImGui/runtime SETCONTEXT, STARTCOMPOSITION 두 경로, 한글 조합 캡처, 확정·취소 초기화, 기본 처리 1회, caret/후보창 위치, context 분리 호출 0회, 분리 viewport 좌표까지 **10개 검사 통과**했다. HWND나 UI 창을 생성하지 않았다.
- 두 수정 파일 대상 `git diff --check` 통과. Git의 header 줄바꿈 경고는 원래 남아 있던 LF에 관한 안내이고 whitespace 오류는 없다.
- Win32 API mock 검증은 실제 Microsoft IME·TSF의 입력 성공을 대신하지 않는다.

## G04. 남은 확인

사용자가 새 Engine/Client 빌드로 직접 Composition Patterns → Rename에서 한영키 전환, 한글 조합 표시, Enter 확정, Apply name → Save를 확인한다. 같은 공통 입력기를 사용하는 다른 Rename와 분리 창에서도 확인하고, 채팅/닉네임의 기존 inline 조합에 중복 조합창이 생기지 않는지 비교한다. 이 단계 전에는 사용자 화면 PASS로 기록하지 않는다.
