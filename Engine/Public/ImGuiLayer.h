#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class ENGINE_DLL CImGuiLayer final
{
public:
	CImGuiLayer() = default;
	~CImGuiLayer();

	CImGuiLayer(const CImGuiLayer&) = delete;
	CImGuiLayer& operator=(const CImGuiLayer&) = delete;

public:
	bool_t Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void BeginFrame();
	void EndFrame();
	void CancelFrame();
	void Shutdown();

	bool_t WantsCaptureMouse() const;
	bool_t WantsCaptureKeyboard() const;

	static bool_t HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/* Live (uncommitted) IME composition text for whichever InputText currently has focus.
	The OS's own floating composition window is suppressed (see imgui_impl_win32.cpp's
	WM_IME_SETCONTEXT handling), so callers draw this themselves -- typically appended after the
	committed text, right at the caret, with an underline -- to get the same inline-composition
	look every other real application already has instead of Windows' floating box. Empty when
	nothing is being composed. */
	static const wchar_t* Get_ImeCompositionString();

private:
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	bool_t m_bInitialized = false;
	bool_t m_bFrameStarted = false;
};

NS_END
