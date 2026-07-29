#include "imgui.h"

#include "Effect_Tool.h"

void Client::CEffect_Tool::Render()
{
	ImGui::SetNextWindowSize(
		ImVec2(900.f, 650.f),
		ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk Effect Tool"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("Effect Tool is connected to the current framework.");
	ImGui::Separator();
	ImGui::TextUnformatted("F1: open / close the editor tool workspace");
	ImGui::TextUnformatted("Drag this window onto Map Tool to dock as a tab.");
	ImGui::End();
}
