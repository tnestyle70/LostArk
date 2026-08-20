#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "ImGuiLayer.h"

#include <filesystem>

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
	like) render as boxes. The project's own LostArk UI font (Resources/Fonts/,
	next to the DirectXTK .spritefont cooked from the same source) matches the
	game's real typeface; Malgun Gothic (ships with Windows) and finally the
	stock default are fallbacks so a missing font file never fails tool startup. */
	{
		wchar_t moduleFilePath[MAX_PATH]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, moduleFilePath, static_cast<DWORD>(std::size(moduleFilePath)));
		const std::filesystem::path lostArkFont = (0 == moduleLength || moduleLength >= std::size(moduleFilePath))
			? std::filesystem::path{}
			: std::filesystem::path(moduleFilePath).parent_path() / L"Resources" / L"Fonts" / L"HANYoonGothic330.ttf";

		/* This checkout's own path can contain non-ASCII (Korean username, ...); narrowing via
		filesystem::path::string() uses the "C" locale and mangles that, and stb_truetype's
		fopen() underneath AddFontFromFileTTF expects the ANSI codepage a Windows char* path
		implies, not UTF-8. Converting through the system ANSI codepage explicitly is what
		actually round-trips a Korean path back to a fopen() that succeeds. */
		char_t lostArkFontAnsi[MAX_PATH]{};
		if (!lostArkFont.empty())
		{
			WideCharToMultiByte(CP_ACP, 0, lostArkFont.c_str(), -1,
				lostArkFontAnsi, static_cast<int32_t>(std::size(lostArkFontAnsi)), nullptr, nullptr);
		}

		const char_t* pFallbackFont = "C:\\Windows\\Fonts\\malgun.ttf";

		if ('\0' != lostArkFontAnsi[0] && std::filesystem::exists(lostArkFont))
			io.Fonts->AddFontFromFileTTF(lostArkFontAnsi, 16.f, nullptr,
				io.Fonts->GetGlyphRangesKorean());
		else if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA(pFallbackFont))
			io.Fonts->AddFontFromFileTTF(pFallbackFont, 16.f, nullptr,
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

const wchar_t* CImGuiLayer::Get_ImeCompositionString()
{
	return ImGui_ImplWin32_GetImeCompositionString();
}
