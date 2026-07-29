#include "imgui.h"

#include "MapTool.h"
#include "BinaryAsset/ModelDecoderRegistry.h"

void Client::CMapTool::Toggle()
{
	m_bOpen = !m_bOpen;
}

void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(560.f, 330.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("ImGui Win32/DX11 runtime is active.");
	ImGui::Separator();
	ImGui::TextUnformatted("F1: open / close");
	ImGui::TextUnformatted("Next slice: asset palette, picking, transform inspector");
	ImGui::Text("Keyboard capture: %s", ImGui::GetIO().WantCaptureKeyboard ? "ON" : "OFF");
	ImGui::Text("Mouse capture: %s", ImGui::GetIO().WantCaptureMouse ? "ON" : "OFF");

	const float fFrameRate = ImGui::GetIO().Framerate;
	if (fFrameRate > 0.f)
		ImGui::Text("Frame: %.3f ms (%.1f FPS)", 1000.f / fFrameRate, fFrameRate);
	else
		ImGui::TextUnformatted("Frame: warming up");

	ImGui::SeparatorText("Binary decoder");
	const MODEL_DECODE_REPORT report = CModelDecoderRegistry::Get().Get_LastReport();
	if (report.meshPath.empty())
	{
		ImGui::TextUnformatted("No binary model decode has been requested yet.");
	}
	else
	{
		ImGui::Text("Status: %s", report.succeeded ? "LOADED" : "FAILED");
		ImGui::Text("Decoder: %s", report.decoderName.empty() ? "not recognized" : report.decoderName.c_str());
		ImGui::TextWrapped("Source: %s", report.meshPath.string().c_str());
		if (report.succeeded)
		{
			ImGui::Text("Meshes: %u  Materials: %u", report.meshCount, report.materialCount);
			ImGui::Text("Vertices: %llu  Indices: %llu",
				static_cast<unsigned long long>(report.vertexCount),
				static_cast<unsigned long long>(report.indexCount));
			ImGui::Text("Skinned bind-pose fallback: %s", report.usedSkinnedBindPose ? "YES" : "NO");
		}
		else
		{
			ImGui::TextWrapped("Reason: %s", report.error.c_str());
			ImGui::TextUnformatted("Set LOSTARK_SHARED_ASSET_ROOT or sync the dev-lol-annie pack.");
		}
	}

	ImGui::End();
}

bool Client::CMapTool::IsOpen() const
{
	return m_bOpen;
}
