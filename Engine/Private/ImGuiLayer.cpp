#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "ImGuiLayer.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

CImGuiLayer::~CImGuiLayer()
{
	Shutdown();
}

bool_t CImGuiLayer::Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (m_bInitialized)
		return true;

	if (nullptr == hWnd || nullptr == pDevice || nullptr == pContext)
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = "imgui.ini";

	ImGui::StyleColorsDark();

	/* The stock atlas is ASCII only, so Korean tool labels (skill names and the
	like) render as boxes. Malgun Gothic ships with Windows and its Korean glyph
	range also carries Latin, so it can serve as the single UI font. If the file
	is somehow absent we keep the default font rather than fail tool startup. */
	{
		const char_t* pKoreanFont = "C:\\Windows\\Fonts\\malgun.ttf";
		if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA(pKoreanFont))
			io.Fonts->AddFontFromFileTTF(pKoreanFont, 16.f, nullptr,
				io.Fonts->GetGlyphRangesKorean());
		else
			io.Fonts->AddFontDefault();
	}

	if (!ImGui_ImplWin32_Init(hWnd))
	{
		ImGui::DestroyContext();
		return false;
	}

	if (!ImGui_ImplDX11_Init(pDevice, pContext))
	{
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		return false;
	}

	m_pContext = pContext;
	m_bInitialized = true;
	return true;
}

void CImGuiLayer::BeginFrame()
{
	if (!m_bInitialized || m_bFrameStarted)
		return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_bFrameStarted = true;
}

void CImGuiLayer::EndFrame()
{
	if (!m_bInitialized || !m_bFrameStarted)
		return;

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ComPtr<ID3D11RenderTargetView> pPreviousRTV;
		ComPtr<ID3D11DepthStencilView> pPreviousDSV;
		if (nullptr != m_pContext)
			m_pContext->OMGetRenderTargets(1, &pPreviousRTV, &pPreviousDSV);

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		if (nullptr != m_pContext)
		{
			ID3D11RenderTargetView* pRTV = pPreviousRTV.Get();
			m_pContext->OMSetRenderTargets(1, &pRTV, pPreviousDSV.Get());
		}
	}

	m_bFrameStarted = false;
}

void CImGuiLayer::CancelFrame()
{
	if (!m_bInitialized || !m_bFrameStarted)
		return;

	ImGui::EndFrame();

	m_bFrameStarted = false;
}

void CImGuiLayer::Shutdown()
{
	if (!m_bInitialized)
		return;

	CancelFrame();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	m_pContext.Reset();
	m_bInitialized = false;
}

bool_t CImGuiLayer::WantsCaptureMouse() const
{
	return m_bInitialized && ImGui::GetIO().WantCaptureMouse;
}

bool_t CImGuiLayer::WantsCaptureKeyboard() const
{
	return m_bInitialized && ImGui::GetIO().WantCaptureKeyboard;
}

bool_t CImGuiLayer::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (nullptr == ImGui::GetCurrentContext())
		return false;

	return 0 != ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}
