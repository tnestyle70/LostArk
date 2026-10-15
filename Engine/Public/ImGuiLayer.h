#pragma once

// Win32 창 메시지와 D3D11 context COM 참조, 엔진 공통 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 도구 UI의 ImGui context/backend 수명과 frame 경계다.
// 메인 UI/렌더 스레드의 직렬 호출을 전제로 하며 내부 잠금은 없다.
class ENGINE_DLL CImGuiLayer final
{
public:
	CImGuiLayer() = default;
	~CImGuiLayer();

	CImGuiLayer(const CImGuiLayer&) = delete;
	CImGuiLayer& operator=(const CImGuiLayer&) = delete;

public:
	// 유효한 창/device/context로 context와 backend를 준비한다. 이미 초기화됐으면 기존 상태를 유지한다.
	bool_t Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	// 미초기화/중복 시작을 무시한다. backend 준비 뒤 ImGui frame을 열고 MainApp이 widget을 작성한다.
	void BeginFrame();
	// 열린 frame을 그리며 분리 viewport도 갱신한다. 보조 창 렌더 뒤 RTV0/DSV만 복구한다.
	void EndFrame();
	// 열린 frame을 draw 없이 끝내고 platform 창 상태는 갱신한다. 이미 닫힌 frame은 무시한다.
	void CancelFrame();
	// 열린 frame을 취소하고 backend/context 및 소유 COM 참조를 정리한다. 소멸자도 호출한다.
	void Shutdown();

	// ImGui의 현재 capture 요청을 읽는다. gameplay 차단 적용은 MainApp/Input_Device가 담당한다.
	bool_t WantsCaptureMouse() const;
	bool_t WantsCaptureKeyboard() const;

	// Client WndProc가 backend에 메시지를 전달한다. context가 없으면 false며 직접 gameplay 입력을 처리하지 않는다.
	static bool_t HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/* 제품의 chat/nickname 입력란이 직접 그릴 미확정 IME 조합 문자열이다.
	Win32 backend는 해당 입력란에서만 OS 조합 창을 숨긴다. 일반 ImGui InputText는
	caret 위치의 OS 조합/후보 창을 사용한다. 조합 중이 아니면 빈 문자열이다. */
	static const wchar_t* Get_ImeCompositionString();

private:
	// 보조 창 렌더 전후 출력 복구에 쓰는 강한 COM 참조다. HWND 수명은 Client가 관리한다.
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	bool_t m_bInitialized = false;
	bool_t m_bFrameStarted = false;
};

NS_END
