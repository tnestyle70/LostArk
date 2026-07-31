#include "imgui.h"

#include "HUDLayoutTool.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <cmath>

namespace
{
	struct DOCUMENT_DEF
	{
		const char*	szLabel;
		const char*	szPath;
		const char*	szTextureRoot;
		bool		bPerClass;
	};

	constexpr DOCUMENT_DEF g_Documents[] =
	{
		{ "Combat HUD", "../Bin/Resources/LostArk/UI/HUD/HUD_Layout.cfg",      "../Bin/Resources/LostArk/UI/HUD/",      true  },
		{ "Screen UI",  "../Bin/Resources/LostArk/UI/ScreenUI/ScreenUI.cfg",   "../Bin/Resources/LostArk/UI/ScreenUI/", false },
	};

	constexpr int32_t g_iDocumentCount = static_cast<int32_t>(sizeof(g_Documents) / sizeof(g_Documents[0]));

	struct PALETTE_ENTRY
	{
		Client::CHUDLayoutTool::SLOT_TYPE	eType;
		const char*							szLabel;
		float								fSizeX;
		float								fSizeY;
		bool								bPerClass;
		bool								bRepeatable;
	};

	constexpr PALETTE_ENTRY g_PaletteEntries[] =
	{
		{ Client::CHUDLayoutTool::SLOT_TYPE::EVADE,          "Evade",         48.f,  48.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::POTION,         "Potion",        48.f,  24.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::HEALTHBAR,      "HealthBar",     260.f, 20.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::MANABAR,        "ManaBar",       260.f, 20.f,  false, false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::CLASS_EMBLEM,   "ClassEmblem",   140.f, 140.f, true,  false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::IDENTITY_GAUGE, "IdentityGauge", 200.f, 24.f,  true,  false },
		{ Client::CHUDLayoutTool::SLOT_TYPE::SKILL,          "Skill",         44.f,  44.f,  false, true  },
		{ Client::CHUDLayoutTool::SLOT_TYPE::ITEM,           "Item",          46.f,  46.f,  false, true  },
	};

	const PALETTE_ENTRY* Find_Palette_Entry(Client::CHUDLayoutTool::SLOT_TYPE eType)
	{
		for (const PALETTE_ENTRY& Entry : g_PaletteEntries)
			if (Entry.eType == eType)
				return &Entry;

		return nullptr;
	}

	const char* SlotType_Label(Client::CHUDLayoutTool::SLOT_TYPE eType)
	{
		const PALETTE_ENTRY* pEntry = Find_Palette_Entry(eType);
		return pEntry ? pEntry->szLabel : "Custom";
	}

	wstring Utf8_To_Wide(const string& strUtf8)
	{
		if (strUtf8.empty())
			return wstring();

		const int32_t iLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLen <= 0)
			return wstring();

		wstring strWide(iLen - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.data(), iLen);

		return strWide;
	}

	string Path_To_Utf8(const filesystem::path& Path)
	{
		const u8string strU8 = Path.u8string();
		return string(strU8.begin(), strU8.end());
	}

	void Get_Rotated_Rect_Corners(const ImVec2& vTopLeft, const ImVec2& vBotRight, float fDegrees, ImVec2 outCorners[4])
	{
		outCorners[0] = vTopLeft;
		outCorners[1] = ImVec2(vBotRight.x, vTopLeft.y);
		outCorners[2] = vBotRight;
		outCorners[3] = ImVec2(vTopLeft.x, vBotRight.y);

		if (0.f == fDegrees)
			return;

		const ImVec2 vCenter((vTopLeft.x + vBotRight.x) * 0.5f, (vTopLeft.y + vBotRight.y) * 0.5f);
		const float fRadians = fDegrees * (3.14159265f / 180.f);
		const float fCos = cosf(fRadians);
		const float fSin = sinf(fRadians);

		for (int32_t i = 0; i < 4; ++i)
		{
			const float fDX = outCorners[i].x - vCenter.x;
			const float fDY = outCorners[i].y - vCenter.y;
			outCorners[i].x = vCenter.x + fDX * fCos - fDY * fSin;
			outCorners[i].y = vCenter.y + fDX * fSin + fDY * fCos;
		}
	}
}

Client::CHUDLayoutTool::CHUDLayoutTool(ComPtr<ID3D11Device> pDevice)
	: m_pDevice { pDevice }
{
	/* CreateWICTextureFromFile needs COM on the calling thread; the main thread never initializes it (only the level-loading worker thread does). */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	Load(Current_Save_Path());

	if (m_Slots.empty())
		Reset_Default();
}

const char* Client::CHUDLayoutTool::Current_Save_Path() const
{
	return g_Documents[m_iActiveDocument].szPath;
}

void Client::CHUDLayoutTool::Switch_Document(int32_t iDocument)
{
	if (iDocument < 0 || iDocument >= g_iDocumentCount)
		return;
	if (iDocument == m_iActiveDocument)
		return;

	/* Persist the document we are leaving so unsaved placement isn't lost on switch. */
	Save(Current_Save_Path());

	m_iActiveDocument = iDocument;

	/* Start from an empty document; Load() fills it if the target file exists. */
	m_Slots.clear();
	m_ClassNames = vector<string>{ "Default" };
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;
	m_SelectedSlots.clear();
	m_iLastScannedClass = -1;

	Load(Current_Save_Path());
}

void Client::CHUDLayoutTool::Render()
{
	/* Wide enough for palette + canvas viewport + inspector side by side. */
	ImGui::SetNextWindowSize(ImVec2(1450.f, 700.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk HUD Layout Tool"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("F1: open / close the editor tool workspace   |   drag box body to move, drag yellow corner to resize.");
	ImGui::TextUnformatted("Drag this window onto Map Tool to dock as a tab.");

	if (ImGui::BeginTabBar("HUDDocuments"))
	{
		for (int32_t i = 0; i < g_iDocumentCount; ++i)
		{
			if (ImGui::BeginTabItem(g_Documents[i].szLabel))
			{
				if (i != m_iActiveDocument)
					Switch_Document(i);
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

	if (ImGui::Button("Save"))
		Save(Current_Save_Path());
	ImGui::SameLine();
	if (ImGui::Button("Load"))
		Load(Current_Save_Path());
	ImGui::SameLine();
	if (g_Documents[m_iActiveDocument].bPerClass && ImGui::Button("Reset to Default Layout"))
		Reset_Default();
	ImGui::SameLine();
	ImGui::Checkbox("Preview Hover (all)", &m_bPreviewHover);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Forces every slot to its hover art.\nWithout this, mousing over a slot on the canvas previews just that one.");

	/* One control drives every slot, so a charge state can be judged as a whole. */
	if (ImGui::Button("Animation Stage"))
		m_iPreviewStage = (m_iPreviewStage + 1) % (ms_iMaxStage + 1);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Click to cycle 0 -> 1 -> 2 -> 0.\nEach slot sets which stage its base and lit art appear at.");
	ImGui::SameLine();
	ImGui::Text("%d / %d   (0 = empty, higher = more charged)", m_iPreviewStage, ms_iMaxStage);

	ImGui::SetNextItemWidth(160.f);
	ImGui::SliderFloat("Zoom", &m_fCanvasScale, ms_fMinZoom, ms_fMaxZoom, "%.2fx");
	ImGui::SameLine();
	if (ImGui::Button("100%"))
		m_fCanvasScale = 1.f;
	ImGui::SameLine();
	if (ImGui::Button("Fit"))
		m_fCanvasScale = ms_fViewportWidth / ms_fRefWidth;
	ImGui::SameLine();
	ImGui::TextDisabled("(ctrl+wheel to zoom at cursor, middle-drag to pan)");

	if (g_Documents[m_iActiveDocument].bPerClass)
		Render_ClassBar();
	ImGui::Separator();

	Render_Palette();
	ImGui::SameLine();
	Render_Canvas();
	ImGui::SameLine();
	Render_Inspector();

	ImGui::End();
}

void Client::CHUDLayoutTool::Render_ClassBar()
{
	ImGui::SeparatorText("Character Class");

	if (!m_ClassNames.empty())
	{
		if (m_iSelectedClass >= static_cast<int32_t>(m_ClassNames.size()))
			m_iSelectedClass = 0;

		if (ImGui::BeginCombo("Current Class", m_ClassNames[m_iSelectedClass].c_str()))
		{
			for (int32_t i = 0; i < static_cast<int32_t>(m_ClassNames.size()); ++i)
			{
				const bool_t bSelected = (i == m_iSelectedClass);
				if (ImGui::Selectable(m_ClassNames[i].c_str(), bSelected))
					m_iSelectedClass = i;
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Class") && m_ClassNames.size() > 1)
			Delete_Class(m_iSelectedClass);
	}

	ImGui::SetNextItemWidth(160.f);
	ImGui::InputText("##NewClassName", m_szNewClassName, sizeof(m_szNewClassName));
	ImGui::SameLine();
	if (ImGui::Button("Add Class"))
		Add_Class(m_szNewClassName);
}

void Client::CHUDLayoutTool::Render_Palette()
{
	ImGui::BeginChild("Palette", ImVec2(190.f, ms_fViewportHeight), true);

	ImGui::SeparatorText("Slot Palette");
	ImGui::TextWrapped("Drag a slot onto the canvas to place it.");
	ImGui::Spacing();

	for (const PALETTE_ENTRY& Entry : g_PaletteEntries)
	{
		ImGui::PushID(static_cast<int32_t>(Entry.eType));

		const ImVec2 vBoxSize(122.f, 32.f);
		ImGui::InvisibleButton(Entry.szLabel, vBoxSize);

		const ImVec2 vMin = ImGui::GetItemRectMin();
		const ImVec2 vMax = ImGui::GetItemRectMax();
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		const ImU32 iFillColor = Entry.bPerClass
			? IM_COL32(120, 70, 170, 160)
			: IM_COL32(60, 130, 190, 150);

		pDrawList->AddRectFilled(vMin, vMax, iFillColor);
		pDrawList->AddRect(vMin, vMax, IM_COL32(200, 200, 210, 200));
		pDrawList->AddText(ImVec2(vMin.x + 4.f, vMin.y + 8.f), IM_COL32(255, 255, 255, 255), Entry.szLabel);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			const int32_t iType = static_cast<int32_t>(Entry.eType);
			ImGui::SetDragDropPayload("HUD_PALETTE_SLOT", &iType, sizeof(iType));
			ImGui::Text("%s", Entry.szLabel);
			ImGui::EndDragDropSource();
		}

		ImGui::PopID();
		ImGui::Spacing();
	}

	Render_ClassTexturePalette();

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Render_ClassTexturePalette()
{
	if (m_iSelectedClass != m_iLastScannedClass)
	{
		Refresh_ClassTexturePalette();
		m_iLastScannedClass = m_iSelectedClass;
	}

	const bool_t bPerClass = g_Documents[m_iActiveDocument].bPerClass;

	ImGui::SeparatorText(bPerClass ? ("Textures: " + Current_Class()).c_str() : "Textures");
	ImGui::TextWrapped("Drag a thumbnail onto a placed slot to add it as a layer.");

	if (m_TextureAssetPaths.empty())
	{
		ImGui::TextWrapped("(no images found for this document)");
		return;
	}

	for (int32_t i = 0; i < static_cast<int32_t>(m_TextureAssetPaths.size()); ++i)
	{
		const string& strPath = m_TextureAssetPaths[i];
		const string strFileName = Path_To_Utf8(filesystem::path(Utf8_To_Wide(strPath)).filename());

		ImGui::PushID(i);

		ID3D11ShaderResourceView* pSRV = Get_Or_Load_Texture(strPath);

		const ImVec2 vThumbSize(40.f, 40.f);
		if (nullptr != pSRV)
			ImGui::Image(pSRV, vThumbSize);
		else
			ImGui::Dummy(vThumbSize);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("HUD_TEXTURE_ASSET", &i, sizeof(i));
			if (nullptr != pSRV)
				ImGui::Image(pSRV, vThumbSize);
			ImGui::TextUnformatted(strFileName.c_str());
			ImGui::EndDragDropSource();
		}

		ImGui::SameLine();
		ImGui::TextWrapped("%s", strFileName.c_str());

		ImGui::PopID();
	}
}

void Client::CHUDLayoutTool::Refresh_ClassTexturePalette()
{
	m_TextureAssetPaths.clear();

	const DOCUMENT_DEF& Document = g_Documents[m_iActiveDocument];

	/* Per-class documents browse just the current class's folder; global ones browse the whole
	   document tree, since their art is grouped by purpose (QuickMenu, Top Menu, Currency) not by class. */
	string strFolder = Document.szTextureRoot;
	if (Document.bPerClass)
	{
		const string& strClass = Current_Class();
		if (strClass.empty())
			return;

		strFolder += strClass;
	}

	error_code ec;
	const filesystem::path Folder = Utf8_To_Wide(strFolder);

	auto Fn_Collect = [this](const filesystem::directory_entry& Entry)
	{
		if (!Entry.is_regular_file())
			return;

		wstring strExt = Entry.path().extension().wstring();
		for (wchar_t& ch : strExt)
			ch = static_cast<wchar_t>(towlower(ch));

		if (L".png" != strExt && L".dds" != strExt && L".jpg" != strExt)
			return;

		m_TextureAssetPaths.push_back(Path_To_Utf8(Entry.path()));
	};

	if (Document.bPerClass)
	{
		for (const filesystem::directory_entry& Entry : filesystem::directory_iterator(Folder, ec))
			Fn_Collect(Entry);
	}
	else
	{
		for (const filesystem::directory_entry& Entry : filesystem::recursive_directory_iterator(Folder, ec))
			Fn_Collect(Entry);
	}
}

void Client::CHUDLayoutTool::Render_Canvas()
{
	/* Fixed viewport with scrollbars: the zoomed canvas can exceed it and is panned instead of
	   forcing the whole tool window to grow. */
	ImGui::BeginChild("HUDCanvas", ImVec2(ms_fViewportWidth, ms_fViewportHeight),
		true, ImGuiWindowFlags_HorizontalScrollbar);

	const ImVec2 vOrigin = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	/* Hit-testing happens after drawing, so this frame's art uses last frame's hover result. */
	const int32_t iHoveredLastFrame = m_iHoveredSlot;
	m_iHoveredSlot = -1;

	/* Ctrl + wheel zooms about the cursor: the content point under the mouse stays put. */
	if (ImGui::IsWindowHovered() && ImGui::GetIO().KeyCtrl && 0.f != ImGui::GetIO().MouseWheel)
	{
		const ImVec2 vMouse = ImGui::GetMousePos();
		const float fContentX = (vMouse.x - vOrigin.x) / m_fCanvasScale;
		const float fContentY = (vMouse.y - vOrigin.y) / m_fCanvasScale;

		const float fOldScale = m_fCanvasScale;
		m_fCanvasScale *= (1.f + ImGui::GetIO().MouseWheel * 0.12f);
		if (m_fCanvasScale < ms_fMinZoom)
			m_fCanvasScale = ms_fMinZoom;
		if (m_fCanvasScale > ms_fMaxZoom)
			m_fCanvasScale = ms_fMaxZoom;

		const float fScaleDelta = m_fCanvasScale - fOldScale;
		ImGui::SetScrollX(ImGui::GetScrollX() + fContentX * fScaleDelta);
		ImGui::SetScrollY(ImGui::GetScrollY() + fContentY * fScaleDelta);
	}

	/* Middle-drag pans, which beats chasing scrollbars while zoomed in. */
	if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
	{
		const ImVec2 vDelta = ImGui::GetIO().MouseDelta;
		ImGui::SetScrollX(ImGui::GetScrollX() - vDelta.x);
		ImGui::SetScrollY(ImGui::GetScrollY() - vDelta.y);
	}

	const ImVec2 vCanvasEnd(vOrigin.x + ms_fRefWidth * m_fCanvasScale, vOrigin.y + ms_fRefHeight * m_fCanvasScale);
	pDrawList->AddRectFilled(vOrigin, vCanvasEnd, IM_COL32(110, 110, 116, 255));
	pDrawList->AddRect(vOrigin, vCanvasEnd, IM_COL32(200, 200, 205, 255));

	ImGui::SetCursorScreenPos(vOrigin);
	ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("CanvasBackground", ImVec2(vCanvasEnd.x - vOrigin.x, vCanvasEnd.y - vOrigin.y));

	if (ImGui::IsItemActivated())
	{
		m_SelectedSlots.clear();
		m_iSelectedSlot = -1;
		m_bMarqueeActive = true;
		const ImVec2 vMouse = ImGui::GetMousePos();
		m_fMarqueeStartX = vMouse.x;
		m_fMarqueeStartY = vMouse.y;
	}

	if (m_bMarqueeActive && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		const ImVec2 vMouse = ImGui::GetMousePos();
		const ImVec2 vMin((min)(m_fMarqueeStartX, vMouse.x), (min)(m_fMarqueeStartY, vMouse.y));
		const ImVec2 vMax((max)(m_fMarqueeStartX, vMouse.x), (max)(m_fMarqueeStartY, vMouse.y));
		pDrawList->AddRectFilled(vMin, vMax, IM_COL32(90, 160, 255, 60));
		pDrawList->AddRect(vMin, vMax, IM_COL32(90, 160, 255, 200));
	}

	if (m_bMarqueeActive && ImGui::IsItemDeactivated())
	{
		const ImVec2 vMouse = ImGui::GetMousePos();
		const ImVec2 vMin((min)(m_fMarqueeStartX, vMouse.x), (min)(m_fMarqueeStartY, vMouse.y));
		const ImVec2 vMax((max)(m_fMarqueeStartX, vMouse.x), (max)(m_fMarqueeStartY, vMouse.y));

		if (fabsf(vMouse.x - m_fMarqueeStartX) > 3.f || fabsf(vMouse.y - m_fMarqueeStartY) > 3.f)
		{
			for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
			{
				const HUD_SLOT& Slot = m_Slots[i];
				if (!Is_Slot_Visible(Slot))
					continue;

				const ImVec2 vSlotMin(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
				const ImVec2 vSlotMax(vSlotMin.x + Slot.fSizeX * m_fCanvasScale, vSlotMin.y + Slot.fSizeY * m_fCanvasScale);

				const bool_t bOverlaps = (vSlotMin.x <= vMax.x && vSlotMax.x >= vMin.x &&
					vSlotMin.y <= vMax.y && vSlotMax.y >= vMin.y);

				if (bOverlaps)
					m_SelectedSlots.push_back(i);
			}

			m_iSelectedSlot = m_SelectedSlots.empty() ? -1 : m_SelectedSlots.back();
		}

		m_bMarqueeActive = false;
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HUD_PALETTE_SLOT"))
		{
			const int32_t iType = *static_cast<const int32_t*>(pPayload->Data);
			const ImVec2 vMouse = ImGui::GetMousePos();

			const float fDropX = (vMouse.x - vOrigin.x) / m_fCanvasScale;
			const float fDropY = (vMouse.y - vOrigin.y) / m_fCanvasScale;

			Add_Slot_From_Palette(static_cast<SLOT_TYPE>(iType), fDropX, fDropY);
		}

		ImGui::EndDragDropTarget();
	}

	/* Visual pass: back-to-front (array order), draws images/labels only. */
	for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
	{
		HUD_SLOT& Slot = m_Slots[i];
		if (!Is_Slot_Visible(Slot))
			continue;

		const ImVec2 vTopLeft(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
		const ImVec2 vBotRight(vTopLeft.x + Slot.fSizeX * m_fCanvasScale, vTopLeft.y + Slot.fSizeY * m_fCanvasScale);

		ImVec2 Corners[4];
		Get_Rotated_Rect_Corners(vTopLeft, vBotRight, Slot.fRotation, Corners);

		const bool_t bSelected = Is_Slot_Selected(i);
		const ImU32 iBorderColor = IM_COL32(255, 220, 90, 255);

		bool_t bAnyLayerDrawn = false;

		const bool_t bBaseVisible = (m_iPreviewStage >= Slot.iBaseFromStage);
		const bool_t bLitVisible = (m_iPreviewStage >= Slot.iShineFromStage);

		/* Fire animation renders first (furthest back), enlarged, only once the slot is lit. */
		if (bLitVisible)
		{
			if (!Slot.AnimationFrames.empty())
			{
				const vector<string>& Frames = Slot.AnimationFrames;
				const int32_t iFrameCount = static_cast<int32_t>(Frames.size());

				/* Continuous ping-pong position in [0, N-1], then cross-fade the two bracketing frames.
				   With only a few source frames a hard cut reads as a strobe/flicker; the dissolve makes it flow. */
				float fPos = 0.f;
				if (iFrameCount > 1)
				{
					const float fPeriod = 2.f * (iFrameCount - 1);
					float fCycle = fmodf(static_cast<float>(ImGui::GetTime()) * Slot.fAnimationFPS, fPeriod);
					if (fCycle < 0.f)
						fCycle += fPeriod;
					fPos = (fCycle <= iFrameCount - 1) ? fCycle : (fPeriod - fCycle);
				}

				const int32_t iFrameA = static_cast<int32_t>(fPos);
				const int32_t iFrameB = (iFrameA + 1 < iFrameCount) ? iFrameA + 1 : iFrameA;
				const float fBlend = fPos - iFrameA;

				const ImVec2 vSlotCenter((vTopLeft.x + vBotRight.x) * 0.5f, (vTopLeft.y + vBotRight.y) * 0.5f);
				const ImVec2 vFireCenter(
					vSlotCenter.x + Slot.fAnimationOffsetX * m_fCanvasScale,
					vSlotCenter.y + Slot.fAnimationOffsetY * m_fCanvasScale);

				const float fHalfWidth = (vBotRight.x - vTopLeft.x) * 0.5f * Slot.fAnimationScale;
				const float fHalfHeight = (vBotRight.y - vTopLeft.y) * 0.5f * Slot.fAnimationScale;

				const ImVec2 vFireTopLeft(vFireCenter.x - fHalfWidth, vFireCenter.y - fHalfHeight);
				const ImVec2 vFireBotRight(vFireCenter.x + fHalfWidth, vFireCenter.y + fHalfHeight);

				ImVec2 FireCorners[4];
				Get_Rotated_Rect_Corners(vFireTopLeft, vFireBotRight, Slot.fRotation, FireCorners);

				/* Base frame at full opacity, next frame dissolved on top by the blend factor. */
				ID3D11ShaderResourceView* pFrameA = Get_Or_Load_Texture(Frames[iFrameA]);
				if (nullptr != pFrameA)
					pDrawList->AddImageQuad(pFrameA, FireCorners[0], FireCorners[1], FireCorners[2], FireCorners[3]);

				if (iFrameB != iFrameA && fBlend > 0.f)
				{
					ID3D11ShaderResourceView* pFrameB = Get_Or_Load_Texture(Frames[iFrameB]);
					if (nullptr != pFrameB)
					{
						const int32_t iAlpha = static_cast<int32_t>(fBlend * 255.f);
						pDrawList->AddImageQuad(pFrameB, FireCorners[0], FireCorners[1], FireCorners[2], FireCorners[3],
							ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32(255, 255, 255, iAlpha));
					}
				}
			}
		}

		const bool_t bShowHover = m_bPreviewHover || (i == iHoveredLastFrame);

		if (bBaseVisible)
		{
			for (const TEXTURE_LAYER& Layer : Slot.TextureLayers)
			{
				/* Layers without hover art (backgrounds, frames) keep their base image. */
				const string& strDrawPath = (bShowHover && !Layer.strHoverPath.empty())
					? Layer.strHoverPath : Layer.strPath;

				if (strDrawPath.empty())
					continue;

				ID3D11ShaderResourceView* pLayerSRV = Get_Or_Load_Texture(strDrawPath);
				if (nullptr == pLayerSRV)
					continue;

				const ImU32 iTint = ImGui::ColorConvertFloat4ToU32(
					ImVec4(Layer.vTint[0], Layer.vTint[1], Layer.vTint[2], Layer.vTint[3]));

				pDrawList->AddImageQuad(pLayerSRV, Corners[0], Corners[1], Corners[2], Corners[3],
					ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), iTint);
				bAnyLayerDrawn = true;
			}
		}

		/* Lit state layers its variant over the base (white orb -> blue orb, gauge -> shine). */
		if (bLitVisible && !Slot.strShineTexture.empty())
		{
			ID3D11ShaderResourceView* pShineSRV = Get_Or_Load_Texture(Slot.strShineTexture);
			if (nullptr != pShineSRV)
			{
				pDrawList->AddImageQuad(pShineSRV, Corners[0], Corners[1], Corners[2], Corners[3]);
				bAnyLayerDrawn = true;
			}
		}

		/* A slot with nothing to show at this stage gets a faint outline so it stays findable while
		   editing, but that marker is editor chrome: clicking the background clears the selection and
		   with it every outline, leaving a clean preview of the layout itself. */
		if (!bAnyLayerDrawn && !m_SelectedSlots.empty())
			pDrawList->AddQuad(Corners[0], Corners[1], Corners[2], Corners[3], IM_COL32(150, 150, 160, 80));

		if (bSelected)
		{
			pDrawList->AddQuad(Corners[0], Corners[1], Corners[2], Corners[3], iBorderColor);
			pDrawList->AddText(ImVec2(Corners[0].x + 3.f, Corners[0].y + 2.f), IM_COL32(255, 255, 255, 255), Slot.strName.c_str());
		}
	}

	/* Interaction pass: front-to-back (reverse array order) so the topmost slot claims the click first. */
	for (int32_t i = static_cast<int32_t>(m_Slots.size()) - 1; i >= 0; --i)
	{
		HUD_SLOT& Slot = m_Slots[i];
		if (!Is_Slot_Visible(Slot))
			continue;

		const ImVec2 vTopLeft(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
		const ImVec2 vBotRight(vTopLeft.x + Slot.fSizeX * m_fCanvasScale, vTopLeft.y + Slot.fSizeY * m_fCanvasScale);

		const bool_t bSelected = Is_Slot_Selected(i);

		ImGui::PushID(i);

		ImGui::SetCursorScreenPos(vTopLeft);
		ImGui::InvisibleButton("body", ImVec2(vBotRight.x - vTopLeft.x, vBotRight.y - vTopLeft.y));
		/* Front-to-back pass, so the first hit is the topmost slot. */
		if (-1 == m_iHoveredSlot && ImGui::IsItemHovered())
			m_iHoveredSlot = i;
		if (ImGui::IsItemActivated())
		{
			if (ImGui::GetIO().KeyCtrl)
				Toggle_Slot_Selection(i);
			else if (!bSelected || m_SelectedSlots.size() <= 1)
				Select_Slot_Exclusive(i);
		}
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			ImVec2 vDelta = ImGui::GetIO().MouseDelta;

			if (ImGui::GetIO().KeyShift)
			{
				const ImVec2 vTotalDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				if (fabsf(vTotalDelta.x) >= fabsf(vTotalDelta.y))
					vDelta.y = 0.f;
				else
					vDelta.x = 0.f;
			}

			if (Is_Slot_Selected(i) && m_SelectedSlots.size() > 1)
			{
				for (int32_t iSelected : m_SelectedSlots)
				{
					m_Slots[iSelected].fX += vDelta.x / m_fCanvasScale;
					m_Slots[iSelected].fY += vDelta.y / m_fCanvasScale;
				}
			}
			else
			{
				Slot.fX += vDelta.x / m_fCanvasScale;
				Slot.fY += vDelta.y / m_fCanvasScale;
			}
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HUD_TEXTURE_ASSET"))
			{
				const int32_t iAssetIndex = *static_cast<const int32_t*>(pPayload->Data);
				if (iAssetIndex >= 0 && iAssetIndex < static_cast<int32_t>(m_TextureAssetPaths.size()))
				{
					TEXTURE_LAYER Layer{};
					Layer.strPath = m_TextureAssetPaths[iAssetIndex];
					Slot.TextureLayers.push_back(move(Layer));
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (bSelected && 1 == m_SelectedSlots.size())
		{
			const float fHandle = 8.f;
			const ImVec2 vHandleTopLeft(vBotRight.x - fHandle, vBotRight.y - fHandle);
			ImGui::SetCursorScreenPos(vHandleTopLeft);
			ImGui::InvisibleButton("resize", ImVec2(fHandle, fHandle));
			pDrawList->AddRectFilled(vHandleTopLeft, vBotRight, IM_COL32(255, 220, 90, 220));
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				const ImVec2 vDelta = ImGui::GetIO().MouseDelta;
				Slot.fSizeX += vDelta.x / m_fCanvasScale;
				Slot.fSizeY += vDelta.y / m_fCanvasScale;
				if (Slot.fSizeX < 8.f)
					Slot.fSizeX = 8.f;
				if (Slot.fSizeY < 8.f)
					Slot.fSizeY = 8.f;
			}
		}

		ImGui::PopID();
	}

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Render_Inspector()
{
	ImGui::BeginChild("Inspector", ImVec2(300.f, ms_fViewportHeight), true);

	ImGui::SeparatorText("Slot List");
	{
		ImGui::BeginChild("SlotList", ImVec2(0.f, 180.f), true);
		for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
		{
			if (!Is_Slot_Visible(m_Slots[i]))
				continue;

			ImGui::PushID(i);
			const bool_t bSelected = Is_Slot_Selected(i);
			if (ImGui::Selectable(m_Slots[i].strName.c_str(), bSelected))
				Select_Slot_Exclusive(i);
			ImGui::PopID();
		}
		ImGui::EndChild();
	}

	if (m_SelectedSlots.size() > 1)
		ImGui::TextWrapped("%d slots selected (drag any of them together; edit fields below apply to the last-clicked one).", static_cast<int32_t>(m_SelectedSlots.size()));

	ImGui::SetNextItemWidth(160.f);
	ImGui::InputText("##NewSlotName", m_szNewSlotName, sizeof(m_szNewSlotName));
	ImGui::SameLine();
	if (ImGui::Button("Add Slot"))
		Add_Slot(m_szNewSlotName);

	if (m_iSelectedSlot >= 0 && m_iSelectedSlot < static_cast<int32_t>(m_Slots.size()))
	{
		HUD_SLOT& Slot = m_Slots[m_iSelectedSlot];

		ImGui::SeparatorText("Selected Slot");

		ImGui::Text("Type: %s", SlotType_Label(Slot.eType));

		char szNameBuf[64] = {};
		strncpy_s(szNameBuf, Slot.strName.c_str(), sizeof(szNameBuf) - 1);
		if (ImGui::InputText("Name", szNameBuf, sizeof(szNameBuf)))
			Slot.strName = szNameBuf;

		ImGui::DragFloat("X", &Slot.fX, 1.f, 0.f, ms_fRefWidth);
		ImGui::DragFloat("Y", &Slot.fY, 1.f, 0.f, ms_fRefHeight);
		ImGui::DragFloat("Width", &Slot.fSizeX, 1.f, 4.f, ms_fRefWidth);
		ImGui::DragFloat("Height", &Slot.fSizeY, 1.f, 4.f, ms_fRefHeight);
		ImGui::DragFloat("Rotation", &Slot.fRotation, 1.f, -180.f, 180.f, "%.1f deg");

		ImGui::SeparatorText("Texture Layers (bottom to top)");
		for (int32_t iLayer = 0; iLayer < static_cast<int32_t>(Slot.TextureLayers.size()); ++iLayer)
		{
			TEXTURE_LAYER& Layer = Slot.TextureLayers[iLayer];

			ImGui::PushID(iLayer);

			/* Tint swatch first: monochrome mask icons are colored here rather than by swapping images. */
			ImGui::ColorEdit4("##LayerTint", Layer.vTint,
				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);

			ImGui::SameLine();
			char szLayerBuf[260] = {};
			strncpy_s(szLayerBuf, Layer.strPath.c_str(), sizeof(szLayerBuf) - 1);
			ImGui::SetNextItemWidth(150.f);
			if (ImGui::InputText("##LayerPath", szLayerBuf, sizeof(szLayerBuf)))
				Layer.strPath = szLayerBuf;

			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
			{
				Slot.TextureLayers.erase(Slot.TextureLayers.begin() + iLayer);
				ImGui::PopID();
				break;
			}

			/* Optional hover swap for this layer; left blank the layer stays put on hover. */
			char szHoverBuf[260] = {};
			strncpy_s(szHoverBuf, Layer.strHoverPath.c_str(), sizeof(szHoverBuf) - 1);
			ImGui::SetNextItemWidth(150.f);
			if (ImGui::InputText("hover", szHoverBuf, sizeof(szHoverBuf)))
				Layer.strHoverPath = szHoverBuf;

			ImGui::PopID();
		}

		ImGui::SetNextItemWidth(160.f);
		ImGui::InputText("##NewLayerPath", m_szNewLayerPathBuffer, sizeof(m_szNewLayerPathBuffer));
		ImGui::SameLine();
		if (ImGui::Button("Add Layer") && '\0' != m_szNewLayerPathBuffer[0])
		{
			TEXTURE_LAYER Layer{};
			Layer.strPath = m_szNewLayerPathBuffer;
			Slot.TextureLayers.push_back(move(Layer));
			m_szNewLayerPathBuffer[0] = '\0';
		}

		ImGui::SeparatorText("Charge Stages");
		ImGui::SliderInt("Base from stage", &Slot.iBaseFromStage, 0, ms_iMaxStage);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stage at which this slot's base texture starts showing.\n0 = always visible.");
		ImGui::SliderInt("Lit from stage", &Slot.iShineFromStage, 0, ms_iMaxStage);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stage at which the shine texture and animation frames kick in.");

		if (g_Documents[m_iActiveDocument].bPerClass)
		{
			/* Owner decides which class this slot belongs to; shared slots show for every class. */
			const char* szOwnerLabel = Slot.strOwnerClass.empty() ? "(shared by all classes)" : Slot.strOwnerClass.c_str();
			if (ImGui::BeginCombo("Owner", szOwnerLabel))
			{
				if (ImGui::Selectable("(shared by all classes)", Slot.strOwnerClass.empty()))
					Slot.strOwnerClass.clear();

				for (const string& strClass : m_ClassNames)
				{
					const bool_t bSelectedOwner = (Slot.strOwnerClass == strClass);
					if (ImGui::Selectable(strClass.c_str(), bSelectedOwner))
						Slot.strOwnerClass = strClass;
				}
				ImGui::EndCombo();
			}
		}

		char szShineBuf[260] = {};
		strncpy_s(szShineBuf, Slot.strShineTexture.c_str(), sizeof(szShineBuf) - 1);
		if (ImGui::InputText("Shine Texture (on-state)", szShineBuf, sizeof(szShineBuf)))
			Slot.strShineTexture = szShineBuf;

		ImGui::DragFloat("Fire Animation Scale", &Slot.fAnimationScale, 0.01f, 0.5f, 3.f);
		ImGui::DragFloat("Fire Offset X", &Slot.fAnimationOffsetX, 0.5f, -100.f, 100.f);
		ImGui::DragFloat("Fire Offset Y", &Slot.fAnimationOffsetY, 0.5f, -100.f, 100.f);

		ImGui::SeparatorText("Animation Frames (plays behind, e.g. flickering flame)");

		vector<string>& Frames = Slot.AnimationFrames;
		for (int32_t iFrame = 0; iFrame < static_cast<int32_t>(Frames.size()); ++iFrame)
		{
			ImGui::PushID(iFrame);

			char szFrameBuf[260] = {};
			strncpy_s(szFrameBuf, Frames[iFrame].c_str(), sizeof(szFrameBuf) - 1);
			ImGui::SetNextItemWidth(200.f);
			if (ImGui::InputText("##FramePath", szFrameBuf, sizeof(szFrameBuf)))
				Frames[iFrame] = szFrameBuf;

			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
			{
				Frames.erase(Frames.begin() + iFrame);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}

		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputText("##NewAnimFramePath", m_szNewAnimFramePathBuffer, sizeof(m_szNewAnimFramePathBuffer));
		if (ImGui::Button("Add Frame") && '\0' != m_szNewAnimFramePathBuffer[0])
		{
			Frames.push_back(m_szNewAnimFramePathBuffer);
			m_szNewAnimFramePathBuffer[0] = '\0';
		}
		if (!Frames.empty())
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			ImGui::DragFloat("FPS", &Slot.fAnimationFPS, 0.5f, 1.f, 60.f);
		}

		if (ImGui::Button("Move Backward"))
			Move_Slot_Backward(m_iSelectedSlot);
		ImGui::SameLine();
		if (ImGui::Button("Move Forward"))
			Move_Slot_Forward(m_iSelectedSlot);

		if (ImGui::Button("Duplicate Slot (same rect)"))
			Duplicate_Slot(m_iSelectedSlot);
		ImGui::SameLine();
		if (ImGui::Button("Delete Slot"))
		{
			Delete_Slot(m_iSelectedSlot);
			m_iSelectedSlot = -1;
			m_SelectedSlots.clear();
		}
	}

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Add_Slot(const string& strName)
{
	HUD_SLOT Slot{};
	Slot.strName = strName.empty() ? "New_Slot" : strName;
	Slot.fX = ms_fRefWidth * 0.5f - 20.f;
	Slot.fY = ms_fRefHeight * 0.5f - 20.f;
	Slot.fSizeX = 40.f;
	Slot.fSizeY = 40.f;

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
}

void Client::CHUDLayoutTool::Add_Slot_From_Palette(SLOT_TYPE eType, float fDropCenterX, float fDropCenterY)
{
	const PALETTE_ENTRY* pEntry = Find_Palette_Entry(eType);
	if (nullptr == pEntry)
		return;

	HUD_SLOT Slot{};
	Slot.strName = Generate_Unique_Name(pEntry->szLabel, pEntry->bRepeatable);
	Slot.eType = eType;
	Slot.fSizeX = pEntry->fSizeX;
	Slot.fSizeY = pEntry->fSizeY;
	/* Class-specific palette entries (emblem, identity gauge) are born owned by the class being edited. */
	if (pEntry->bPerClass && g_Documents[m_iActiveDocument].bPerClass)
		Slot.strOwnerClass = Current_Class();
	Slot.fX = fDropCenterX - Slot.fSizeX * 0.5f;
	Slot.fY = fDropCenterY - Slot.fSizeY * 0.5f;

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
}

string Client::CHUDLayoutTool::Generate_Unique_Name(const string& strBase, bool_t bRepeatable) const
{
	auto Fn_Exists = [this](const string& strName)
	{
		for (const HUD_SLOT& Slot : m_Slots)
			if (Slot.strName == strName)
				return true;
		return false;
	};

	if (!bRepeatable && !Fn_Exists(strBase))
		return strBase;

	int32_t iSuffix = 1;
	string strCandidate = strBase + "_" + to_string(iSuffix);
	while (Fn_Exists(strCandidate))
	{
		++iSuffix;
		strCandidate = strBase + "_" + to_string(iSuffix);
	}

	return strCandidate;
}

void Client::CHUDLayoutTool::Delete_Slot(int32_t iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(m_Slots.size()))
		return;

	m_Slots.erase(m_Slots.begin() + iIndex);
}

void Client::CHUDLayoutTool::Duplicate_Slot(int32_t iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(m_Slots.size()))
		return;

	HUD_SLOT Slot = m_Slots[iIndex];
	Slot.strName = Generate_Unique_Name(m_Slots[iIndex].strName, true);
	Slot.TextureLayers.clear();
	Slot.strShineTexture.clear();
	Slot.AnimationFrames.clear();

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
}

void Client::CHUDLayoutTool::Move_Slot_Forward(int32_t iIndex)
{
	if (iIndex < 0 || iIndex + 1 >= static_cast<int32_t>(m_Slots.size()))
		return;

	swap(m_Slots[iIndex], m_Slots[iIndex + 1]);

	for (int32_t& iSelected : m_SelectedSlots)
	{
		if (iSelected == iIndex)
			iSelected = iIndex + 1;
		else if (iSelected == iIndex + 1)
			iSelected = iIndex;
	}

	if (m_iSelectedSlot == iIndex)
		m_iSelectedSlot = iIndex + 1;
	else if (m_iSelectedSlot == iIndex + 1)
		m_iSelectedSlot = iIndex;
}

void Client::CHUDLayoutTool::Move_Slot_Backward(int32_t iIndex)
{
	if (iIndex <= 0 || iIndex >= static_cast<int32_t>(m_Slots.size()))
		return;

	swap(m_Slots[iIndex], m_Slots[iIndex - 1]);

	for (int32_t& iSelected : m_SelectedSlots)
	{
		if (iSelected == iIndex)
			iSelected = iIndex - 1;
		else if (iSelected == iIndex - 1)
			iSelected = iIndex;
	}

	if (m_iSelectedSlot == iIndex)
		m_iSelectedSlot = iIndex - 1;
	else if (m_iSelectedSlot == iIndex - 1)
		m_iSelectedSlot = iIndex;
}

bool_t Client::CHUDLayoutTool::Is_Slot_Visible(const HUD_SLOT& Slot) const
{
	/* Documents without classes (Screen UI) show everything; otherwise shared + current class only. */
	if (!g_Documents[m_iActiveDocument].bPerClass)
		return true;

	return Slot.strOwnerClass.empty() || Slot.strOwnerClass == Current_Class();
}

bool_t Client::CHUDLayoutTool::Is_Slot_Selected(int32_t iIndex) const
{
	for (int32_t iSelected : m_SelectedSlots)
		if (iSelected == iIndex)
			return true;

	return false;
}

void Client::CHUDLayoutTool::Toggle_Slot_Selection(int32_t iIndex)
{
	for (auto Iter = m_SelectedSlots.begin(); Iter != m_SelectedSlots.end(); ++Iter)
	{
		if (*Iter == iIndex)
		{
			m_SelectedSlots.erase(Iter);
			m_iSelectedSlot = m_SelectedSlots.empty() ? -1 : m_SelectedSlots.back();
			return;
		}
	}

	m_SelectedSlots.push_back(iIndex);
	m_iSelectedSlot = iIndex;
}

void Client::CHUDLayoutTool::Select_Slot_Exclusive(int32_t iIndex)
{
	m_SelectedSlots.clear();
	m_SelectedSlots.push_back(iIndex);
	m_iSelectedSlot = iIndex;
}

void Client::CHUDLayoutTool::Add_Class(const string& strName)
{
	if (strName.empty())
		return;

	for (const string& strExisting : m_ClassNames)
		if (strExisting == strName)
			return;

	m_ClassNames.push_back(strName);
}

void Client::CHUDLayoutTool::Delete_Class(int32_t iIndex)
{
	if (iIndex < 0 || iIndex >= static_cast<int32_t>(m_ClassNames.size()))
		return;
	if (m_ClassNames.size() <= 1)
		return;

	const string strRemoved = m_ClassNames[iIndex];
	m_ClassNames.erase(m_ClassNames.begin() + iIndex);

	/* Slots owned by the removed class would become unreachable, so drop them with it. */
	for (auto Iter = m_Slots.begin(); Iter != m_Slots.end(); )
		Iter = (Iter->strOwnerClass == strRemoved) ? m_Slots.erase(Iter) : Iter + 1;

	m_iSelectedSlot = -1;
	m_SelectedSlots.clear();

	if (m_iSelectedClass >= static_cast<int32_t>(m_ClassNames.size()))
		m_iSelectedClass = static_cast<int32_t>(m_ClassNames.size()) - 1;
}

const string& Client::CHUDLayoutTool::Current_Class() const
{
	static const string strEmpty;

	if (m_ClassNames.empty())
		return strEmpty;

	return m_ClassNames[m_iSelectedClass];
}

ID3D11ShaderResourceView* Client::CHUDLayoutTool::Get_Or_Load_Texture(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	auto Iter = m_TextureCache.find(strPath);
	if (m_TextureCache.end() != Iter)
		return Iter->second.Get();

	const wstring strWidePath = Utf8_To_Wide(strPath);
	const filesystem::path Ext = filesystem::path(strWidePath).extension();

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	HRESULT hr = {};
	if (0 == _wcsicmp(Ext.c_str(), L".dds"))
		hr = CreateDDSTextureFromFile(m_pDevice.Get(), strWidePath.c_str(), nullptr, &pSRV);
	else
		hr = CreateWICTextureFromFile(m_pDevice.Get(), strWidePath.c_str(), nullptr, &pSRV);

	m_TextureCache[strPath] = pSRV;

	return FAILED(hr) ? nullptr : pSRV.Get();
}

void Client::CHUDLayoutTool::Save(const string& strPath)
{
	const filesystem::path Path = strPath;

	error_code ec;
	filesystem::create_directories(Path.parent_path(), ec);

	ofstream File(Path);
	if (!File.is_open())
		return;

	File << "RESOLUTION " << ms_fRefWidth << " " << ms_fRefHeight << "\n";

	for (const string& strClass : m_ClassNames)
		File << "CLASS " << strClass << "\n";

	for (const HUD_SLOT& Slot : m_Slots)
	{
		/* "-" stands for a shared slot: an empty token can't survive whitespace-delimited parsing. */
		File << "SLOT " << Slot.strName << " "
			<< Slot.fX << " " << Slot.fY << " "
			<< Slot.fSizeX << " " << Slot.fSizeY << " "
			<< (Slot.strOwnerClass.empty() ? "-" : Slot.strOwnerClass) << " "
			<< static_cast<int32_t>(Slot.eType) << " "
			<< Slot.fRotation << " "
			<< Slot.iBaseFromStage << " "
			<< Slot.fAnimationScale << " "
			<< Slot.fAnimationOffsetX << " "
			<< Slot.fAnimationOffsetY << " "
			<< Slot.iShineFromStage << "\n";

		for (int32_t iLayer = 0; iLayer < static_cast<int32_t>(Slot.TextureLayers.size()); ++iLayer)
		{
			const TEXTURE_LAYER& Layer = Slot.TextureLayers[iLayer];
			if (Layer.strPath.empty())
				continue;

			File << "TEXTURE " << Slot.strName << " " << Layer.strPath << "\n";

			if (!Layer.strHoverPath.empty())
				File << "LAYERHOVER " << Slot.strName << " " << iLayer << " " << Layer.strHoverPath << "\n";

			/* Only written when tinted, so untinted layouts stay readable and older files keep loading. */
			if (1.f != Layer.vTint[0] || 1.f != Layer.vTint[1] ||
				1.f != Layer.vTint[2] || 1.f != Layer.vTint[3])
			{
				File << "LAYERTINT " << Slot.strName << " " << iLayer << " "
					<< Layer.vTint[0] << " " << Layer.vTint[1] << " "
					<< Layer.vTint[2] << " " << Layer.vTint[3] << "\n";
			}
		}

		if (!Slot.strShineTexture.empty())
			File << "SHINETEX " << Slot.strName << " " << Slot.strShineTexture << "\n";

		if (!Slot.AnimationFrames.empty())
			File << "ANIMFPS " << Slot.strName << " " << Slot.fAnimationFPS << "\n";

		for (const string& strFramePath : Slot.AnimationFrames)
		{
			if (strFramePath.empty())
				continue;

			File << "ANIMFRAME " << Slot.strName << " " << strFramePath << "\n";
		}
	}
}

void Client::CHUDLayoutTool::Load(const string& strPath)
{
	ifstream File(strPath);
	if (!File.is_open())
		return;

	vector<HUD_SLOT> Slots;
	vector<string> ClassNames;

	string strLine;
	while (getline(File, strLine))
	{
		istringstream Stream(strLine);
		string strTag;
		Stream >> strTag;

		if ("CLASS" == strTag)
		{
			string strClass;
			Stream >> strClass;
			if (!strClass.empty())
				ClassNames.push_back(strClass);
		}
		else if ("SLOT" == strTag)
		{
			HUD_SLOT Slot{};
			string strOwner;
			int32_t iType = 0;
			Stream >> Slot.strName >> Slot.fX >> Slot.fY >> Slot.fSizeX >> Slot.fSizeY >> strOwner >> iType >> Slot.fRotation
				>> Slot.iBaseFromStage >> Slot.fAnimationScale >> Slot.fAnimationOffsetX >> Slot.fAnimationOffsetY
				>> Slot.iShineFromStage;
			if ("-" != strOwner)
				Slot.strOwnerClass = strOwner;
			Slot.eType = static_cast<SLOT_TYPE>(iType);
			Slots.push_back(move(Slot));
		}
		/* Everything below decorates the slot most recently declared, so different classes may reuse
		   the same slot name without their attributes bleeding into each other. */
		else if (!Slots.empty())
		{
			HUD_SLOT& Slot = Slots.back();

			if ("TEXTURE" == strTag)
			{
				string strSlotName, strTexturePath;
				Stream >> strSlotName;
				Stream >> ws;
				getline(Stream, strTexturePath);

				TEXTURE_LAYER Layer{};
				Layer.strPath = strTexturePath;
				Slot.TextureLayers.push_back(move(Layer));
			}
			else if ("LAYERHOVER" == strTag)
			{
				string strSlotName, strHoverPath;
				int32_t iLayer = -1;
				Stream >> strSlotName >> iLayer;
				Stream >> ws;
				getline(Stream, strHoverPath);

				if (iLayer >= 0 && iLayer < static_cast<int32_t>(Slot.TextureLayers.size()))
					Slot.TextureLayers[iLayer].strHoverPath = strHoverPath;
			}
			else if ("LAYERTINT" == strTag)
			{
				string strSlotName;
				int32_t iLayer = -1;
				float vTint[4] = { 1.f, 1.f, 1.f, 1.f };
				Stream >> strSlotName >> iLayer >> vTint[0] >> vTint[1] >> vTint[2] >> vTint[3];

				if (iLayer >= 0 && iLayer < static_cast<int32_t>(Slot.TextureLayers.size()))
				{
					TEXTURE_LAYER& Layer = Slot.TextureLayers[iLayer];
					Layer.vTint[0] = vTint[0];
					Layer.vTint[1] = vTint[1];
					Layer.vTint[2] = vTint[2];
					Layer.vTint[3] = vTint[3];
				}
			}
			else if ("SHINETEX" == strTag)
			{
				string strSlotName, strTexturePath;
				Stream >> strSlotName;
				Stream >> ws;
				getline(Stream, strTexturePath);

				Slot.strShineTexture = strTexturePath;
			}
			else if ("ANIMFRAME" == strTag)
			{
				string strSlotName, strFramePath;
				Stream >> strSlotName;
				Stream >> ws;
				getline(Stream, strFramePath);

				Slot.AnimationFrames.push_back(strFramePath);
			}
			else if ("ANIMFPS" == strTag)
			{
				string strSlotName;
				float fFPS = 10.f;
				Stream >> strSlotName >> fFPS;

				Slot.fAnimationFPS = fFPS;
			}
		}
	}

	if (Slots.empty())
		return;

	m_Slots = move(Slots);
	m_ClassNames = ClassNames.empty() ? vector<string>{ "Default" } : move(ClassNames);
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;
}

void Client::CHUDLayoutTool::Reset_Default()
{
	m_Slots.clear();
	m_ClassNames.clear();
	m_ClassNames.push_back("Default");
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;

	auto Fn_AddSlot = [this](const string& strName, float fX, float fY, float fW, float fH, const string& strOwnerClass = string())
	{
		HUD_SLOT Slot{};
		Slot.strName = strName;
		Slot.fX = fX;
		Slot.fY = fY;
		Slot.fSizeX = fW;
		Slot.fSizeY = fH;
		Slot.strOwnerClass = strOwnerClass;
		m_Slots.push_back(move(Slot));
	};

	Fn_AddSlot("Evade", 328.f, 600.f, 50.f, 50.f);
	Fn_AddSlot("Potion", 332.f, 656.f, 42.f, 42.f);

	Fn_AddSlot("HealthBar", 390.f, 600.f, 270.f, 20.f);
	Fn_AddSlot("Skill_Q", 392.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_W", 446.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_E", 500.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_R", 554.f, 628.f, 50.f, 50.f);
	Fn_AddSlot("Skill_A", 420.f, 682.f, 46.f, 46.f);
	Fn_AddSlot("Skill_S", 470.f, 682.f, 46.f, 46.f);
	Fn_AddSlot("Skill_D", 520.f, 682.f, 46.f, 46.f);
	Fn_AddSlot("Skill_F", 570.f, 682.f, 46.f, 46.f);

	Fn_AddSlot("Identity", 592.f, 590.f, 96.f, 96.f, "Default");

	Fn_AddSlot("ManaBar", 712.f, 600.f, 258.f, 20.f);
	Fn_AddSlot("Skill_F1", 968.f, 628.f, 46.f, 46.f);

	for (int32_t i = 0; i < 5; ++i)
		Fn_AddSlot("Item_" + to_string(i + 1), 1018.f + i * 50.f, 628.f, 46.f, 46.f);

	for (int32_t i = 0; i < 4; ++i)
		Fn_AddSlot("Item_" + to_string(i + 6), 1018.f + i * 50.f, 682.f, 46.f, 46.f);
}
