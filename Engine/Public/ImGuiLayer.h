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

	/* Live (uncommitted) IME text for custom runtime chat/nickname fields to draw inline.
	The Win32 backend suppresses the OS composition window only for those fields;
	ordinary ImGui InputText uses the OS composition/candidate UI at its caret.
	Empty when nothing is being composed. */
	static const wchar_t* Get_ImeCompositionString();

private:
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	bool_t m_bInitialized = false;
	bool_t m_bFrameStarted = false;
};

NS_END
