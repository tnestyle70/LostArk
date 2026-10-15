// MainApp의 Profiler 입력·보조 표시·저장 UI를 같은 세션 상태에 연결한다.
// 프레임 호출 순서, ProfilerTool 실제 창 렌더, 초기화·종료는 MainApp.cpp가 계속 소유한다.
// Win32보다 WinSock2를 먼저 포함해 MainApp의 네트워크 선언과 Windows 기본 헤더가 충돌하지 않게 한다.
#include <WinSock2.h>
#include <Windows.h>

// F7의 ImGui 텍스트 입력 상태와 보조 overlay/settings widget을 읽고 그린다.
#include "imgui.h"
#include "MainApp.h"
// Engine의 기존 측정기와 live 통계는 MainApp이 소유하지 않고 호출 동안 차용한다.
#include "GameInstance.h"
#include "Profiler.h"
// 창 객체와 비동기 저장 진행은 MainApp의 unique_ptr가 소유한 기존 도구로 처리한다.
#include "ProfilerTool.h"
// 제품 UI의 텍스트 입력도 F7보다 먼저 보호한다.
#include "UIInputRouter.h"
#include <memory>

// Client 이름 공간의 MainApp 멤버 정의다. 계측 데이터는 Engine Profiler가, 표시와 저장 작업 상태는 ProfilerTool이 소유한다.
using namespace Client;

// Update의 InputAndUI 단계가 Sync_CinematicUI 다음, F1 처리 전에 호출한다.
// 매 프레임 저장 완료를 확인하고 키를 누른 프레임에만 표시를 바꾼다. profiler 측정이나 Capture는 자동 시작하지 않는다.
// ImGui 보조 viewport도 같은 process 창이면 허용하며, 입력을 막은 프레임에도 key 상태를 기록해 반복 토글을 막는다.
// 메인 Update가 저장 진행을 확인하고 F7의 상승 edge로 profiler 창을 토글한다.
// 제품·ImGui 텍스트 입력 중에는 단축키를 소비하지 않는다.
void CMainApp::UpdateProfilerRuntime()
{
	// F7은 profiler 창 표시만 바꾼다. Capture 시작과 저장은 창 안의 명시적 동작이다.
	if (m_pProfilerTool)
		m_pProfilerTool->Update_SaveState();
	// down은 지금의 키 상태이고 m_bF7Down은 직전 프레임의 상태다.
	// 상승 순간만 처리하므로 키를 누른 채 여러 프레임이 지나도 창을 매번 뒤집지 않는다.
	const bool_t down = IsWindowOwnedByCurrentProcess(GetForegroundWindow()) && 0 != (GetAsyncKeyState(VK_F7) & 0x8000);
	if (down && !m_bF7Down && !ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive())
	{
		bool_t visible = m_bRuntimeProfilerVisible;
		visible |= m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::PROFILER);
		m_bRuntimeProfilerVisible = !visible;
		if (m_bRuntimeProfilerVisible)
		{
			if (!m_pProfilerTool)
			{
				m_pProfilerTool = make_unique<CProfilerTool>(m_pDevice.Get());
			}
			m_pProfilerTool->Open();
		}
		else
			SetDebugToolVisible(DEBUG_TOOL::PROFILER, false);
	}
	m_bF7Down = down;
}

#ifdef _DEBUG

// Debug Render가 m_bProfilerVisible일 때 호출하는 입력 없는 보조 overlay다. F7 창의 표시 플래그와 별개다.
// live 통계는 값으로 읽으며 측정기가 없으면 그리지 않는다. ImGui FPS는 표시하고 CPU/GPU는 유효한 통계가 있을 때만 추가한다.
void CMainApp::RenderProfilerOverlay()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.ProfilerOverlay");
	if (!m_bProfilerVisible)
		return;

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (nullptr == pProfiler)
		return;

	// Get_LiveStats는 Engine 내부 mutex 아래에서 통계를 이 지역 값으로 복사한다.
	// 반환 뒤 UI는 history container의 주소를 계속 붙잡지 않는다. 최신 CPU 프레임과 가장 최근 완료 GPU 프레임은 번호가 다를 수 있다.
	Engine::FProfilerLiveStats stats{};
	const bool_t hasStats = pProfiler->Get_LiveStats(stats);
	const ImGuiIO& io = ImGui::GetIO();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(viewport->WorkPos.x + 10.f, viewport->WorkPos.y + 30.f),
		ImGuiCond_Always);
	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;
	if (ImGui::Begin("##LostArkPerfOverlay", nullptr, flags))
	{
		// 1000 / FPS는 ImGui의 프레임률에서 환산한 평균 간격(ms)이다.
		// 아래 CPU/GPU scope 측정 시간과 같은 값으로 해석하지 않으며 FPS가 0이면 나누지 않는다.
		ImGui::Text("FPS: %.1f  (%.3f ms)",
			io.Framerate,
			io.Framerate > 0.f ? 1000.f / io.Framerate : 0.f);
		if (hasStats)
		{
			ImGui::Text("CPU: %.3f ms", stats.CpuFrameMs);
			ImGui::Text("GPU: %s",
				stats.GpuValid ? "available" : "warming up");
			if (stats.GpuValid)
				ImGui::Text("GPU time: %.3f ms", stats.GpuFrameMs);
		}
	}
	ImGui::End();
}

// Debug Render가 overlay 다음에 호출한다. Reset은 현재 측정 이력을 즉시 비우고 Save는 기존 ProfilerTool에 요청한다.
// 저장 중 중복 클릭은 막는다. 측정기나 도구 준비가 없으면 상태 문구만 남기며 Save 성공·실패는 도구의 비동기 상태로 표시한다.
void CMainApp::RenderProfilerSettings()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.ProfilerDetails");
	if (!m_bProfilerVisible)
		return;

	if (!ImGui::Begin(
		"LostArk Profiler Details",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (ImGui::Button("Reset profiler history") && nullptr != pProfiler)
		pProfiler->Reset_History();
	ImGui::SameLine();
	// 현재 저장 여부를 보고 중복 요청 버튼을 막는다. Request_Save도 exporter 상태를 확인한다.
	// 저장 시 ProfilerTool이 완료 프레임 snapshot을 값으로 만들고 exporter에 이동시켜 이후 live history 변경과 분리한다.
	ImGui::BeginDisabled(m_pProfilerTool && m_pProfilerTool->Is_Saving());
	if (ImGui::Button("Save profiler JSON"))
	{
		if (nullptr == pProfiler)
		{
			m_strProfilerCaptureStatus = "Profiler is not available.";
		}
		else
		{
			// F7 창과 이 보조 설정 창은 같은 ProfilerTool 비동기 저장 경로를 사용한다.
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::PROFILER)) && m_pProfilerTool)
			{
				m_pProfilerTool->Request_Save(*pProfiler);
				m_strProfilerCaptureStatus.clear();
			}
			else m_strProfilerCaptureStatus = "Composition Profiler could not be opened.";
		}
	}
	ImGui::EndDisabled();
	// 버튼을 누른 사실이 저장 성공은 아니다. Update_SaveState가 exporter 완료 결과를 Poll한 뒤
	// 성공 경로 또는 오류를 도구 상태에 기록하며 여기서는 그 상태를 표시한다.
	if (m_pProfilerTool && !m_pProfilerTool->Get_CaptureStatus().empty())
		ImGui::TextWrapped("%s", m_pProfilerTool->Get_CaptureStatus().c_str());
	if (!m_strProfilerCaptureStatus.empty())
		ImGui::TextWrapped("%s", m_strProfilerCaptureStatus.c_str());
	ImGui::End();
}

#endif
