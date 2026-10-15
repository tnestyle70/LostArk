// ImGui context·frame·입력 capture와 docking/viewport 설정을 사용한다.
#include "imgui.h"
// GPU backend의 초기화·frame 준비·draw 제출과 profiler 연결을 사용한다.
#include "imgui_impl_dx11.h"
// 창 입력·platform 창 갱신과 프로젝트 IME 조합 문자열을 소비한다.
#include "imgui_impl_win32.h"

// GameInstance가 소유하는 ImGui 수명/frame 경계를 구현한다.
#include "ImGuiLayer.h"
// 현재 frame profiler를 조회한다.
#include "GameInstance.h"
// UI CPU 구간과 window/viewport 개수를 기록한다.
#include "Profiler.h"

// 실행 파일 옆 프로젝트 font 경로를 조합하고 파일 존재를 확인한다.
#include <filesystem>

// Win32 backend의 WndProc 진입점을 호출한다. platform 메시지 처리의 실제 구현은 backend에 있다.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

// 정상 초기화된 context/backend의 종료 누락을 방지한다.
CImGuiLayer::~CImGuiLayer()
{
	Shutdown();
}

// GameInstance 초기화가 창과 장치를 전달한다. backend 준비 실패 시 이 함수가 만든 context와 먼저 준비한 backend를 정리한다.
bool_t CImGuiLayer::Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (m_bInitialized)
		return true;

	if (nullptr == hWnd || nullptr == pDevice || nullptr == pContext)
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// 키보드 탐색/docking/분리 viewport를 활성화하고 도구 배치 저장 위치를 설정한다. 제품 렌더링 품질 옵션은 변경하지 않는다.
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = "imgui.ini";
	io.ConfigDpiScaleFonts = true;
	io.ConfigDpiScaleViewports = true;

	ImGui::StyleColorsDark();

	/* 한글 도구 label에 필요한 glyph를 위해 실행 파일 옆 프로젝트 UI TTF를 먼저 선택한다.
	파일이 없으면 Windows 맑은 고딕, 그마저 없으면 기본 font를 사용한다.
	이 분기는 파일 존재 기준이다. AddFontFromFileTTF의 실제 로드 실패 결과를 확인해
	다음 font로 재시도하는 경로는 아니므로 모든 font 실패 복구를 보장하지 않는다. */
	{
		wchar_t moduleFilePath[MAX_PATH]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, moduleFilePath, static_cast<DWORD>(std::size(moduleFilePath)));
		const std::filesystem::path lostArkFont = (0 == moduleLength || moduleLength >= std::size(moduleFilePath))
			? std::filesystem::path{}
			: std::filesystem::path(moduleFilePath).parent_path() / L"Resources" / L"Fonts" / L"HANYoonGothic330.ttf";

		/* 현재 구현은 프로젝트 font의 wide 경로를 CP_ACP로 좁혀 전달한다.
		번들 ImGui의 Windows ImFileOpen은 입력을 UTF-8로 해석하므로 ACP가 UTF-8이 아닌
		환경에서는 비ASCII 경로가 올바르게 열림을 이 변환만으로 보장하지 못한다.
		가독성 정비에서는 변환 방식과 font 선택의 기존 동작을 유지한다. */
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

	// Win32 준비 실패면 context를 제거한다. DX11 실패면 이미 준비한 Win32 backend도 먼저 종료한다.
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

	// 두 backend가 모두 준비된 뒤에만 layer를 활성화하고 context 참조를 보관한다.
	m_pContext = pContext;
	m_bInitialized = true;
	return true;
}

// GameInstance/MainApp frame 흐름에서 한 번 연다. DX11→Win32→core 순으로 준비한 뒤 열린 상태를 기록한다.
void CImGuiLayer::BeginFrame()
{
	if (!m_bInitialized || m_bFrameStarted)
		return;

	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	ImGui_ImplDX11_SetProfiler(pProfiler);
	CProfilerScope scope(pProfiler, "ImGui.NewFrame");
	{
		CProfilerScope backendScope(pProfiler, "ImGui.NewFrame.DX11");
		ImGui_ImplDX11_NewFrame();
	}
	{
		CProfilerScope platformScope(pProfiler, "ImGui.NewFrame.Win32");
		ImGui_ImplWin32_NewFrame();
	}
	{
		CProfilerScope coreScope(pProfiler, "ImGui.NewFrame.Core");
		ImGui::NewFrame();
	}

	m_bFrameStarted = true;
}

// MainApp이 widget 작성을 끝내면 호출한다. main draw data를 제출하고 분리 도구 창의 갱신/렌더/표시를 이어 수행한다.
void CImGuiLayer::EndFrame()
{
	if (!m_bInitialized || !m_bFrameStarted)
		return;

	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	{
		CProfilerScope scope(pProfiler, "ImGui.FinalizeDrawData");
		ImGui::Render();
	}
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (nullptr != pProfiler)
	{
		pProfiler->Set_Counter(EProfilerCounter::ImGuiRenderWindows, io.MetricsRenderWindows);
		pProfiler->Set_Counter(EProfilerCounter::ImGuiActiveWindows, io.MetricsActiveWindows);
	}
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		CProfilerScope scope(pProfiler, "ImGui.PlatformUpdate");
		ImGui::UpdatePlatformWindows();
	}
	const int32_t viewportCount = ImGui::GetPlatformIO().Viewports.Size;
	if (nullptr != pProfiler)
		pProfiler->Set_Counter(EProfilerCounter::ImGuiPlatformViewports, viewportCount);
	// 분리 창 생성/제거를 위한 Update는 계속 수행한다. main viewport만 있으면
	// 아래 보조 렌더와 출력 저장/복구는 필요하지 않다.
	if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && viewportCount > 1)
	{
		CProfilerScope scope(pProfiler, "ImGui.PlatformRender");
		// 보조 viewport가 바꿀 출력의 RTV0와 DSV를 강한 참조로 보관한다. 다른 MRT slot을 저장하는 범용 상태 stack은 아니다.
		ComPtr<ID3D11RenderTargetView> pPreviousRTV;
		ComPtr<ID3D11DepthStencilView> pPreviousDSV;
		if (nullptr != m_pContext)
			m_pContext->OMGetRenderTargets(1, &pPreviousRTV, &pPreviousDSV);

		// 최소화되지 않은 보조 창의 platform/renderer callback을 호출한다. main viewport의 렌더/표시는 애플리케이션이 맡는다.
		ImGuiPlatformIO& platform = ImGui::GetPlatformIO();
		for (int32_t i = 1; i < viewportCount; ++i)
		{
			ImGuiViewport* viewport = platform.Viewports[i];
			if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
				continue;
			if (platform.Platform_RenderWindow)
				platform.Platform_RenderWindow(viewport, nullptr);
			if (platform.Renderer_RenderWindow)
				platform.Renderer_RenderWindow(viewport, nullptr);
		}
		// 보조 창의 표시 callback 시작 순서를 frame마다 회전시킨다. 장치가 바쁠 때
		// 늘 같은 마지막 창만 제출에서 밀리지 않게 하며 main viewport는 애플리케이션이 맡는다.
		const int32_t secondaryCount = viewportCount - 1;
		const int32_t first = ImGui::GetFrameCount() % secondaryCount;
		for (int32_t offset = 0; offset < secondaryCount; ++offset)
		{
			ImGuiViewport* viewport = platform.Viewports[1 + (first + offset) % secondaryCount];
			if (viewport->Flags & ImGuiViewportFlags_IsMinimized)
				continue;
			if (platform.Platform_SwapBuffers)
				platform.Platform_SwapBuffers(viewport, nullptr);
			if (platform.Renderer_SwapBuffers)
				platform.Renderer_SwapBuffers(viewport, nullptr);
		}

		// 보조 창 처리 후 저장한 단일 RTV/DSV를 다시 바인딩한다. 이 복구가 모든 파이프라인 상태를 포함하는 것은 아니다.
		if (nullptr != m_pContext)
		{
			ID3D11RenderTargetView* pRTV = pPreviousRTV.Get();
			m_pContext->OMSetRenderTargets(1, &pRTV, pPreviousDSV.Get());
		}
	}

	m_bFrameStarted = false;
}

// 렌더 실패/종료가 열린 frame을 폐기한다. draw data는 제출하지 않지만 분리 창 생성/제거 bookkeeping은 수행한다.
void CImGuiLayer::CancelFrame()
{
	if (!m_bInitialized || !m_bFrameStarted)
		return;

	ImGui::EndFrame();
	// draw를 취소한 frame도 platform 창 상태의 갱신은 수행한다.
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		ImGui::UpdatePlatformWindows();

	m_bFrameStarted = false;
}

// CancelFrame 이후 profiler 차용 연결을 끊고 DX11→Win32→core context 순서로 정리한다.
void CImGuiLayer::Shutdown()
{
	if (!m_bInitialized)
		return;

	CancelFrame();

	ImGui_ImplDX11_SetProfiler(nullptr);
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	m_pContext.Reset();
	m_bInitialized = false;
}

// 미초기화 시 false다. 현재 ImGui 요청을 조회하며 입력 차단 상태를 직접 바꾸지 않는다.
bool_t CImGuiLayer::WantsCaptureMouse() const
{
	return m_bInitialized && ImGui::GetIO().WantCaptureMouse;
}

// 키보드 capture 요청을 전달한다. 실제 키 snapshot이나 focus gate는 Input_Device가 소유한다.
bool_t CImGuiLayer::WantsCaptureKeyboard() const
{
	return m_bInitialized && ImGui::GetIO().WantCaptureKeyboard;
}

// Client 창 메시지를 현재 ImGui context에 전달한다. backend의 0/nonzero 결과를 bool로 돌려준다.
bool_t CImGuiLayer::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (nullptr == ImGui::GetCurrentContext())
		return false;

	return 0 != ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}

// backend의 전역 조합 buffer를 차용한다. 다음 IME 메시지가 내용을 바꿀 수 있으므로 저장할 소비자는 복사한다.
const wchar_t* CImGuiLayer::Get_ImeCompositionString()
{
	return ImGui_ImplWin32_GetImeCompositionString();
}
