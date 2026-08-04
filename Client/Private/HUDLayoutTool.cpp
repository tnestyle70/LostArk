#include "imgui.h"

#include "HUDLayoutTool.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <fstream>
#include <filesystem>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <unordered_set>

namespace
{
	struct DOCUMENT_DEF
	{
		const char*	szLabel;
		const char*	szDataPath;
		const char*	szTextureRoot;
		bool		bPerClass;
	};

	constexpr DOCUMENT_DEF g_Documents[] =
	{
		{ "Combat HUD",     "UI/HUD/HUD_Layout.json",           "UI/HUD/",      true  },
		{ "Screen UI",      "UI/ScreenUI/ScreenUI.json",        "UI/ScreenUI/", false },
		{ "Loading Screen", "UI/Loading/LoadingLayout.json", "UI/Loading/",  false },
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

	bool_t Is_ValidIdentifier(const string& value)
	{
		if (value.empty() || value.size() > 128u)
			return false;
		return none_of(value.begin(), value.end(), [](const char character)
		{
			return 0 != iscntrl(static_cast<unsigned char>(character));
		});
	}

	bool_t Is_SafeUIResourceId(const string& value)
	{
		if (value.empty() || value.size() > 512u ||
			!value.starts_with("UI/") ||
			string::npos != value.find('\\') ||
			string::npos != value.find(':'))
		{
			return false;
		}

		size_t segmentStart = {};
		while (segmentStart < value.size())
		{
			const size_t separator = value.find('/', segmentStart);
			const size_t segmentEnd = string::npos == separator
				? value.size() : separator;
			const string_view segment(
				value.data() + segmentStart,
				segmentEnd - segmentStart);
			if (segment.empty() || "." == segment || ".." == segment)
				return false;
			segmentStart = segmentEnd + 1u;
		}
		return true;
	}

	const DATA_JSON_VALUE* Find_Member(
		const DATA_JSON_VALUE& object,
		const string_view key,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* pValue = object.Find(key);
		return nullptr != pValue && pValue->Get_Type() == type
			? pValue : nullptr;
	}

	bool_t Read_Number(
		const DATA_JSON_VALUE& object,
		const string_view key,
		f32_t& outValue,
		const f32_t minimum,
		const f32_t maximum)
	{
		const DATA_JSON_VALUE* pValue = Find_Member(
			object, key, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < minimum ||
			pValue->Get_Number() > maximum)
		{
			return false;
		}
		outValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_Integer(
		const DATA_JSON_VALUE& object,
		const string_view key,
		int32_t& outValue,
		const int32_t minimum,
		const int32_t maximum)
	{
		const DATA_JSON_VALUE* pValue = Find_Member(
			object, key, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !isfinite(pValue->Get_Number()))
			return false;
		const double number = pValue->Get_Number();
		if (number != floor(number) || number < minimum || number > maximum)
			return false;
		outValue = static_cast<int32_t>(number);
		return true;
	}

	bool_t Read_Boolean(
		const DATA_JSON_VALUE& object,
		const string_view key,
		bool_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = Find_Member(
			object, key, DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pValue)
			return false;
		outValue = pValue->Get_Boolean();
		return true;
	}

	void Write_String(ostream& output, const string_view value)
	{
		output << '"' << CDataJson::Escape(value) << '"';
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

Client::CHUDLayoutTool::CHUDLayoutTool(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	/* CreateWICTextureFromFile needs COM on the calling thread; the main thread never initializes it (only the level-loading worker thread does). */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	/* Glow/particle art is authored for additive blending (see TEXTURE_LAYER::bAdditive); this is the
	   blend state Draw_Image_Quad switches to for layers that opt in. */
	D3D11_BLEND_DESC BlendDesc{};
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_pDevice->CreateBlendState(&BlendDesc, &m_pAdditiveBlendState);

	Load(Current_Save_Path());

	if (m_Slots.empty())
		Reset_Default();
}

void Client::CHUDLayoutTool::Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	auto pTool = static_cast<CHUDLayoutTool*>(pCmd->UserCallbackData);
	pTool->m_pContext->OMSetBlendState(pTool->m_pAdditiveBlendState.Get(), nullptr, 0xffffffff);
}

void Client::CHUDLayoutTool::Draw_Image_Quad(ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
	const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX)
{
	if (bAdditive)
		pDrawList->AddCallback(&CHUDLayoutTool::Enable_Additive_Blend, this);

	/* Flipping is just swapping which U coordinate each corner samples; the on-screen quad and
	   winding stay the same, only the texture reads mirrored. */
	if (bFlipX)
		pDrawList->AddImageQuad(pSRV, Corners[0], Corners[1], Corners[2], Corners[3],
			ImVec2(1, 0), ImVec2(0, 0), ImVec2(0, 1), ImVec2(1, 1), iTint);
	else
		pDrawList->AddImageQuad(pSRV, Corners[0], Corners[1], Corners[2], Corners[3],
			ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), iTint);

	if (bAdditive)
		pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

filesystem::path Client::CHUDLayoutTool::Current_Save_Path() const
{
	return CProjectDataRoot::Resolve(
		g_Documents[m_iActiveDocument].szDataPath);
}

void Client::CHUDLayoutTool::Switch_Document(int32_t iDocument)
{
	if (iDocument < 0 || iDocument >= g_iDocumentCount)
		return;
	if (iDocument == m_iActiveDocument)
		return;

	/* Persist the document we are leaving so unsaved placement isn't lost on switch -- but only if
	it actually loaded successfully. If it never loaded (JSON contract rejected, unknown class,
	etc.), what's on screen is a placeholder, not the real document; saving it would silently
	destroy the file this session never actually opened. */
	if (m_bActiveDocumentLoaded)
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
	ImGui::TextDisabled("%s", m_strDataStatus.c_str());
	ImGui::SameLine();
	if (g_Documents[m_iActiveDocument].bPerClass && ImGui::Button("Reset to Default Layout"))
		Reset_Default();
	ImGui::SameLine();
	ImGui::Checkbox("Preview Hover (all)", &m_bPreviewHover);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Forces every slot to its hover art.\nWithout this, mousing over a slot on the canvas previews just that one.");
	ImGui::SameLine();
	ImGui::Checkbox("Boost Dark Art", &m_bBoostDarkArt);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Editor-only: layers under a dark background lose their edges when aligning by eye.\nThis redraws them with extra additive passes just for visibility here; it changes nothing saved to the cfg or seen in-game.");

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

bool_t Client::CHUDLayoutTool::Select_Class(const string& classId)
{
	const auto found = find(m_ClassNames.begin(), m_ClassNames.end(), classId);
	if (m_ClassNames.end() != found)
	{
		const int32_t selected = static_cast<int32_t>(
			distance(m_ClassNames.begin(), found));
		if (m_iSelectedClass != selected)
		{
			m_iSelectedClass = selected;
			m_iLastScannedClass = -1;
		}
		return true;
	}

	const auto defaultClass = find(
		m_ClassNames.begin(), m_ClassNames.end(), "Default");
	if (m_ClassNames.end() != defaultClass)
	{
		m_iSelectedClass = static_cast<int32_t>(
			distance(m_ClassNames.begin(), defaultClass));
		m_iLastScannedClass = -1;
	}
	return false;
}

void Client::CHUDLayoutTool::Render_RuntimePreview(const string& classId)
{
	Select_Class(classId);
	if (m_Slots.empty() || m_ClassNames.empty())
		return;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr == viewport || viewport->WorkSize.x <= 0.f ||
		viewport->WorkSize.y <= 0.f)
	{
		return;
	}

	/* Foreground, not background: the combat HUD ImGui stat panel (semi-transparent black) sits at
	SetNextWindowBgAlpha(0.78f) over roughly the same screen region, and a background draw list
	renders underneath every window -- that panel's backdrop was painting over this preview and
	reading as "everything is too dark" even though the source art itself is at full brightness. */
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(viewport);
	const float scaleX = viewport->WorkSize.x / ms_fRefWidth;
	const float scaleY = viewport->WorkSize.y / ms_fRefHeight;
	/* Was hardcoded to 1, forcing lit/shine art on for every preview regardless of the tool's own
	Animation Stage toggle -- share that toggle instead so this defaults to 0 (off) like the real
	runtime view, and only shows charged art when the user actually steps the stage up. */
	const int32_t previewStage = m_iPreviewStage;

	for (HUD_SLOT& slot : m_Slots)
	{
		if (!Is_Slot_Visible(slot))
			continue;

		const ImVec2 topLeft(
			viewport->WorkPos.x + slot.fX * scaleX,
			viewport->WorkPos.y + slot.fY * scaleY);
		const ImVec2 bottomRight(
			topLeft.x + slot.fSizeX * scaleX,
			topLeft.y + slot.fSizeY * scaleY);
		ImVec2 corners[4];
		Get_Rotated_Rect_Corners(
			topLeft, bottomRight, slot.fRotation, corners);

		if (previewStage >= slot.iShineFromStage &&
			!slot.AnimationFrames.empty())
		{
			const int32_t frameCount = static_cast<int32_t>(
				slot.AnimationFrames.size());
			const int32_t frameIndex = frameCount <= 1 ? 0 :
				static_cast<int32_t>(
					ImGui::GetTime() * slot.fAnimationFPS) % frameCount;
			ID3D11ShaderResourceView* pFrame = Get_Or_Load_Texture(
				slot.AnimationFrames[frameIndex]);
			if (nullptr != pFrame)
			{
				const ImVec2 center(
					(topLeft.x + bottomRight.x) * 0.5f +
						slot.fAnimationOffsetX * scaleX,
					(topLeft.y + bottomRight.y) * 0.5f +
						slot.fAnimationOffsetY * scaleY);
				const float halfWidth =
					(bottomRight.x - topLeft.x) * 0.5f *
					slot.fAnimationScale;
				const float halfHeight =
					(bottomRight.y - topLeft.y) * 0.5f *
					slot.fAnimationScale;
				ImVec2 animationCorners[4];
				Get_Rotated_Rect_Corners(
					ImVec2(center.x - halfWidth, center.y - halfHeight),
					ImVec2(center.x + halfWidth, center.y + halfHeight),
					slot.fRotation,
					animationCorners);
				pDrawList->AddImageQuad(
					pFrame,
					animationCorners[0], animationCorners[1],
					animationCorners[2], animationCorners[3]);
			}
		}

		if (previewStage >= slot.iBaseFromStage)
		{
			for (const TEXTURE_LAYER& layer : slot.TextureLayers)
			{
				ID3D11ShaderResourceView* pTexture =
					Get_Or_Load_Texture(layer.strPath);
				if (nullptr == pTexture)
					continue;
				const ImU32 tint = ImGui::ColorConvertFloat4ToU32(
					ImVec4(
						layer.vTint[0], layer.vTint[1],
						layer.vTint[2], layer.vTint[3]));
				Draw_Image_Quad(
					pDrawList, pTexture, corners, tint,
					layer.bAdditive, layer.bFlipX);
			}
		}

		if (previewStage >= slot.iShineFromStage &&
			!slot.strShineTexture.empty())
		{
			ID3D11ShaderResourceView* pShine =
				Get_Or_Load_Texture(slot.strShineTexture);
			if (nullptr != pShine)
			{
				Draw_Image_Quad(
					pDrawList, pShine, corners,
					IM_COL32(255, 255, 255, 255),
					slot.bShineAdditive);
			}
		}
	}
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
	const u8string folderUtf8(strFolder.begin(), strFolder.end());
	const filesystem::path Folder = CRuntimeAssetRoot::Resolve(
		filesystem::path(folderUtf8));
	if (Folder.empty())
		return;
	const filesystem::path ResourceRoot = CRuntimeAssetRoot::Get();

	auto Fn_Collect = [this, &ResourceRoot](
		const filesystem::directory_entry& Entry)
	{
		if (!Entry.is_regular_file())
			return;

		wstring strExt = Entry.path().extension().wstring();
		for (wchar_t& ch : strExt)
			ch = static_cast<wchar_t>(towlower(ch));

		if (L".png" != strExt && L".dds" != strExt && L".jpg" != strExt)
			return;

		error_code relativeError;
		const filesystem::path relativePath = filesystem::relative(
			Entry.path(), ResourceRoot, relativeError);
		if (!relativeError && !relativePath.empty() &&
			relativePath.native().find(L"..") != 0)
		{
			m_TextureAssetPaths.push_back(
				Path_To_Utf8(relativePath.generic_wstring()));
		}
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

	/* A flat mid-gray fill let dark class art (near-black emblems, orbs) blend into the
	   background and lose contrast. A checkerboard reads as "transparent" the same way
	   external atlas viewers do, so dark art keeps its edges visible against it. */
	const float fCheckerSize = 32.f * m_fCanvasScale;
	const ImU32 iCheckerLight = IM_COL32(150, 150, 156, 255);
	const ImU32 iCheckerDark = IM_COL32(120, 120, 126, 255);
	int32_t iRow = 0;
	for (float fY = vOrigin.y; fY < vCanvasEnd.y; fY += fCheckerSize, ++iRow)
	{
		int32_t iCol = 0;
		for (float fX = vOrigin.x; fX < vCanvasEnd.x; fX += fCheckerSize, ++iCol)
		{
			const bool_t bLight = (0 == ((iRow + iCol) % 2));
			const ImVec2 vCellMax((min)(fX + fCheckerSize, vCanvasEnd.x), (min)(fY + fCheckerSize, vCanvasEnd.y));
			pDrawList->AddRectFilled(ImVec2(fX, fY), vCellMax, bLight ? iCheckerLight : iCheckerDark);
		}
	}
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

				Draw_Image_Quad(pDrawList, pLayerSRV, Corners, iTint, Layer.bAdditive, Layer.bFlipX);
				bAnyLayerDrawn = true;

				/* Extra additive passes of the same art lift dark pixels toward visible without
				   touching the layer's own (correct) alpha-blended look; already-additive layers
				   are already at full visibility and skip this. */
				if (m_bBoostDarkArt && !Layer.bAdditive)
				{
					Draw_Image_Quad(pDrawList, pLayerSRV, Corners, iTint, true, Layer.bFlipX);
					Draw_Image_Quad(pDrawList, pLayerSRV, Corners, iTint, true, Layer.bFlipX);
				}
			}
		}

		/* Lit state layers its variant over the base (white orb -> blue orb, gauge -> shine). */
		if (bLitVisible && !Slot.strShineTexture.empty())
		{
			ID3D11ShaderResourceView* pShineSRV = Get_Or_Load_Texture(Slot.strShineTexture);
			if (nullptr != pShineSRV)
			{
				Draw_Image_Quad(pDrawList, pShineSRV, Corners, IM_COL32(255, 255, 255, 255), Slot.bShineAdditive);
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

			/* Glow art (orbs, gauges, emblems) is authored for additive blending; alpha-blended it
			   just looks dim, since most of the sprite is low-alpha halo rather than solid color. */
			ImGui::Checkbox("Additive", &Layer.bAdditive);
			ImGui::SameLine();
			/* For a mirrored copy of directional art (an L-bracket, a wing) instead of a second file. */
			ImGui::Checkbox("Flip X", &Layer.bFlipX);

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
		ImGui::Checkbox("Shine Additive", &Slot.bShineAdditive);

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

	const u8string utf8Path(strPath.begin(), strPath.end());
	const filesystem::path resolvedPath =
		CRuntimeAssetRoot::Resolve(filesystem::path(utf8Path));
	if (resolvedPath.empty())
		return nullptr;
	const filesystem::path Ext = resolvedPath.extension();

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	HRESULT hr = {};
	if (0 == _wcsicmp(Ext.c_str(), L".dds"))
		hr = CreateDDSTextureFromFile(
			m_pDevice.Get(), resolvedPath.c_str(), nullptr, &pSRV);
	else
		hr = CreateWICTextureFromFile(
			m_pDevice.Get(), resolvedPath.c_str(), nullptr, &pSRV);

	m_TextureCache[strPath] = pSRV;

	return FAILED(hr) ? nullptr : pSRV.Get();
}

bool_t Client::CHUDLayoutTool::Save(const filesystem::path& path)
{
	if (path.empty() || L".json" != path.extension())
	{
		m_strDataStatus = "Save rejected: JSON path required";
		return false;
	}

	unordered_set<string> classSet;
	for (const string& className : m_ClassNames)
	{
		if (!Is_ValidIdentifier(className) ||
			!classSet.insert(className).second)
		{
			m_strDataStatus = "Save rejected: invalid class ID";
			return false;
		}
	}

	unordered_set<string> slotSet;
	for (const HUD_SLOT& slot : m_Slots)
	{
		if (!Is_ValidIdentifier(slot.strName) ||
			!slotSet.insert(slot.strName).second ||
			(!slot.strOwnerClass.empty() &&
				!classSet.contains(slot.strOwnerClass)) ||
			!isfinite(slot.fX) || !isfinite(slot.fY) ||
			!isfinite(slot.fSizeX) || !isfinite(slot.fSizeY) ||
			slot.fSizeX <= 0.f || slot.fSizeY <= 0.f)
		{
			m_strDataStatus = "Save rejected: invalid slot contract";
			return false;
		}
		for (const TEXTURE_LAYER& layer : slot.TextureLayers)
		{
			if (!Is_SafeUIResourceId(layer.strPath) ||
				(!layer.strHoverPath.empty() &&
					!Is_SafeUIResourceId(layer.strHoverPath)))
			{
				m_strDataStatus = "Save rejected: invalid UI resource ID";
				return false;
			}
		}
		if ((!slot.strShineTexture.empty() &&
				!Is_SafeUIResourceId(slot.strShineTexture)) ||
			any_of(slot.AnimationFrames.begin(),
				slot.AnimationFrames.end(), [](const string& frame)
				{
					return !Is_SafeUIResourceId(frame);
				}))
		{
			m_strDataStatus = "Save rejected: invalid animation resource ID";
			return false;
		}
	}

	error_code directoryError;
	filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		m_strDataStatus = "Save failed: could not create data directory";
		return false;
	}

	filesystem::path temporaryPath = path;
	temporaryPath += L".tmp";
	ofstream file(temporaryPath, ios::binary | ios::trunc);
	if (!file)
	{
		m_strDataStatus = "Save failed: could not open temporary file";
		return false;
	}
	file.imbue(locale::classic());
	file << setprecision(9);
	file << "{\n  \"schema\": \"lostark.ui-layout\",\n"
		<< "  \"formatVersion\": 1,\n"
		<< "  \"resolution\": { \"width\": " << ms_fRefWidth
		<< ", \"height\": " << ms_fRefHeight << " },\n"
		<< "  \"classes\": [";
	for (size_t index = 0; index < m_ClassNames.size(); ++index)
	{
		if (0u != index)
			file << ", ";
		Write_String(file, m_ClassNames[index]);
	}
	file << "],\n  \"slots\": [\n";

	for (size_t slotIndex = 0; slotIndex < m_Slots.size(); ++slotIndex)
	{
		const HUD_SLOT& slot = m_Slots[slotIndex];
		file << "    {\n      \"id\": ";
		Write_String(file, slot.strName);
		file << ",\n      \"ownerClass\": ";
		if (slot.strOwnerClass.empty())
			file << "null";
		else
			Write_String(file, slot.strOwnerClass);
		file << ",\n      \"type\": "
			<< static_cast<int32_t>(slot.eType)
			<< ",\n      \"rect\": { \"x\": " << slot.fX
			<< ", \"y\": " << slot.fY
			<< ", \"width\": " << slot.fSizeX
			<< ", \"height\": " << slot.fSizeY
			<< " },\n      \"rotation\": " << slot.fRotation
			<< ",\n      \"stages\": { \"baseFrom\": "
			<< slot.iBaseFromStage << ", \"shineFrom\": "
			<< slot.iShineFromStage << " },\n"
			<< "      \"layers\": [";

		for (size_t layerIndex = 0;
			layerIndex < slot.TextureLayers.size(); ++layerIndex)
		{
			const TEXTURE_LAYER& layer = slot.TextureLayers[layerIndex];
			if (0u != layerIndex)
				file << ',';
			file << "\n        { \"path\": ";
			Write_String(file, layer.strPath);
			file << ", \"hoverPath\": ";
			if (layer.strHoverPath.empty())
				file << "null";
			else
				Write_String(file, layer.strHoverPath);
			file << ", \"tint\": [" << layer.vTint[0] << ", "
				<< layer.vTint[1] << ", " << layer.vTint[2] << ", "
				<< layer.vTint[3] << "], \"additive\": "
				<< (layer.bAdditive ? "true" : "false")
				<< ", \"flipX\": "
				<< (layer.bFlipX ? "true" : "false") << " }";
		}
		if (!slot.TextureLayers.empty())
			file << '\n' << "      ";
		file << "],\n      \"shine\": { \"texture\": ";
		if (slot.strShineTexture.empty())
			file << "null";
		else
			Write_String(file, slot.strShineTexture);
		file << ", \"additive\": "
			<< (slot.bShineAdditive ? "true" : "false")
			<< " },\n      \"animation\": { \"fps\": "
			<< slot.fAnimationFPS << ", \"scale\": "
			<< slot.fAnimationScale << ", \"offset\": { \"x\": "
			<< slot.fAnimationOffsetX << ", \"y\": "
			<< slot.fAnimationOffsetY << " }, \"frames\": [";
		for (size_t frameIndex = 0;
			frameIndex < slot.AnimationFrames.size(); ++frameIndex)
		{
			if (0u != frameIndex)
				file << ", ";
			Write_String(file, slot.AnimationFrames[frameIndex]);
		}
		file << "] }\n    }";
		if (slotIndex + 1u != m_Slots.size())
			file << ',';
		file << '\n';
	}
	file << "  ]\n}\n";
	file.flush();
	const bool_t writeSucceeded = file.good();
	file.close();
	if (!writeSucceeded ||
		!MoveFileExW(temporaryPath.c_str(), path.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		m_strDataStatus = "Save failed: atomic promote failed";
		return false;
	}

	m_strDataStatus = "JSON saved";
	return true;
}

bool_t Client::CHUDLayoutTool::Load(const filesystem::path& path)
{
	/* Every early-out below leaves this false, so a failed load on an existing (but invalid) file
	is distinguishable from a freshly-created empty document -- Switch_Document uses this to avoid
	auto-saving placeholder state over the real file. */
	m_bActiveDocumentLoaded = false;

	ifstream file(path, ios::binary);
	if (!file)
	{
		m_strDataStatus = "JSON not found";
		return false;
	}
	const string text{
		istreambuf_iterator<char>(file),
		istreambuf_iterator<char>()};
	DATA_JSON_VALUE root;
	string parseError;
	if (!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_strDataStatus = "JSON parse failed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Find_Member(
		root, "schema", DATA_JSON_TYPE::STRING);
	int32_t formatVersion = {};
	const DATA_JSON_VALUE* pResolution = Find_Member(
		root, "resolution", DATA_JSON_TYPE::OBJECT);
	f32_t width = {};
	f32_t height = {};
	if (nullptr == pSchema || "lostark.ui-layout" != pSchema->Get_String() ||
		!Read_Integer(root, "formatVersion", formatVersion, 1, 1) ||
		nullptr == pResolution ||
		!Read_Number(*pResolution, "width", width, 1.f, 16384.f) ||
		!Read_Number(*pResolution, "height", height, 1.f, 16384.f) ||
		fabsf(width - ms_fRefWidth) > 0.01f ||
		fabsf(height - ms_fRefHeight) > 0.01f)
	{
		m_strDataStatus = "JSON contract or resolution mismatch";
		return false;
	}

	const DATA_JSON_VALUE* pClasses = Find_Member(
		root, "classes", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* pSlots = Find_Member(
		root, "slots", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pClasses || pClasses->Get_Array().empty() ||
		pClasses->Get_Array().size() > 64u || nullptr == pSlots ||
		pSlots->Get_Array().empty() || pSlots->Get_Array().size() > 4096u)
	{
		m_strDataStatus = "JSON collection limits rejected";
		return false;
	}

	vector<string> stagedClasses;
	unordered_set<string> classSet;
	for (const DATA_JSON_VALUE& classValue : pClasses->Get_Array())
	{
		if (!classValue.Is_String() ||
			!Is_ValidIdentifier(classValue.Get_String()) ||
			!classSet.insert(classValue.Get_String()).second)
		{
			m_strDataStatus = "JSON contains invalid class IDs";
			return false;
		}
		stagedClasses.push_back(classValue.Get_String());
	}

	vector<HUD_SLOT> stagedSlots;
	unordered_set<string> slotSet;
	for (const DATA_JSON_VALUE& slotValue : pSlots->Get_Array())
	{
		if (!slotValue.Is_Object())
		{
			m_strDataStatus = "JSON slot is not an object";
			return false;
		}

		HUD_SLOT slot;
		const DATA_JSON_VALUE* pId = Find_Member(
			slotValue, "id", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pOwner = slotValue.Find("ownerClass");
		int32_t type = {};
		const DATA_JSON_VALUE* pRect = Find_Member(
			slotValue, "rect", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pStages = Find_Member(
			slotValue, "stages", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pLayers = Find_Member(
			slotValue, "layers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pShine = Find_Member(
			slotValue, "shine", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pAnimation = Find_Member(
			slotValue, "animation", DATA_JSON_TYPE::OBJECT);

		if (nullptr == pId || !Is_ValidIdentifier(pId->Get_String()) ||
			!slotSet.insert(pId->Get_String()).second || nullptr == pOwner ||
			!Read_Integer(slotValue, "type", type, 0,
				static_cast<int32_t>(SLOT_TYPE::ITEM)) ||
			nullptr == pRect || nullptr == pStages || nullptr == pLayers ||
			pLayers->Get_Array().size() > 32u || nullptr == pShine ||
			nullptr == pAnimation)
		{
			m_strDataStatus = "JSON slot contract rejected";
			return false;
		}

		slot.strName = pId->Get_String();
		if (pOwner->Is_String())
		{
			if (!classSet.contains(pOwner->Get_String()))
			{
				m_strDataStatus = "JSON slot owner is unknown";
				return false;
			}
			slot.strOwnerClass = pOwner->Get_String();
		}
		else if (!pOwner->Is_Null())
		{
			m_strDataStatus = "JSON ownerClass must be string or null";
			return false;
		}
		slot.eType = static_cast<SLOT_TYPE>(type);
		if (!Read_Number(*pRect, "x", slot.fX, -10000.f, 10000.f) ||
			!Read_Number(*pRect, "y", slot.fY, -10000.f, 10000.f) ||
			!Read_Number(*pRect, "width", slot.fSizeX, 0.01f, 10000.f) ||
			!Read_Number(*pRect, "height", slot.fSizeY, 0.01f, 10000.f) ||
			!Read_Number(slotValue, "rotation", slot.fRotation,
				-36000.f, 36000.f) ||
			!Read_Integer(*pStages, "baseFrom", slot.iBaseFromStage,
				0, ms_iMaxStage) ||
			!Read_Integer(*pStages, "shineFrom", slot.iShineFromStage,
				0, ms_iMaxStage))
		{
			m_strDataStatus = "JSON slot numeric bounds rejected";
			return false;
		}

		for (const DATA_JSON_VALUE& layerValue : pLayers->Get_Array())
		{
			if (!layerValue.Is_Object())
			{
				m_strDataStatus = "JSON layer is not an object";
				return false;
			}
			const DATA_JSON_VALUE* pPath = Find_Member(
				layerValue, "path", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pHover = layerValue.Find("hoverPath");
			const DATA_JSON_VALUE* pTint = Find_Member(
				layerValue, "tint", DATA_JSON_TYPE::ARRAY);
			TEXTURE_LAYER layer;
			if (nullptr == pPath || !Is_SafeUIResourceId(pPath->Get_String()) ||
				nullptr == pHover || nullptr == pTint ||
				4u != pTint->Get_Array().size() ||
				!Read_Boolean(layerValue, "additive", layer.bAdditive) ||
				!Read_Boolean(layerValue, "flipX", layer.bFlipX))
			{
				m_strDataStatus = "JSON layer contract rejected";
				return false;
			}
			layer.strPath = pPath->Get_String();
			if (pHover->Is_String())
			{
				if (!Is_SafeUIResourceId(pHover->Get_String()))
				{
					m_strDataStatus = "JSON hover resource ID rejected";
					return false;
				}
				layer.strHoverPath = pHover->Get_String();
			}
			else if (!pHover->Is_Null())
			{
				m_strDataStatus = "JSON hoverPath must be string or null";
				return false;
			}
			for (size_t tintIndex = 0; tintIndex < 4u; ++tintIndex)
			{
				const DATA_JSON_VALUE& tint = pTint->Get_Array()[tintIndex];
				if (!tint.Is_Number() || !isfinite(tint.Get_Number()) ||
					tint.Get_Number() < 0.0 || tint.Get_Number() > 64.0)
				{
					m_strDataStatus = "JSON tint rejected";
					return false;
				}
				layer.vTint[tintIndex] = static_cast<f32_t>(tint.Get_Number());
			}
			slot.TextureLayers.push_back(move(layer));
		}

		const DATA_JSON_VALUE* pShineTexture = pShine->Find("texture");
		if (nullptr == pShineTexture ||
			!Read_Boolean(*pShine, "additive", slot.bShineAdditive))
		{
			m_strDataStatus = "JSON shine contract rejected";
			return false;
		}
		if (pShineTexture->Is_String())
		{
			if (!Is_SafeUIResourceId(pShineTexture->Get_String()))
			{
				m_strDataStatus = "JSON shine resource ID rejected";
				return false;
			}
			slot.strShineTexture = pShineTexture->Get_String();
		}
		else if (!pShineTexture->Is_Null())
		{
			m_strDataStatus = "JSON shine texture must be string or null";
			return false;
		}

		const DATA_JSON_VALUE* pOffset = Find_Member(
			*pAnimation, "offset", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pFrames = Find_Member(
			*pAnimation, "frames", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pOffset || nullptr == pFrames ||
			pFrames->Get_Array().size() > 128u ||
			!Read_Number(*pAnimation, "fps", slot.fAnimationFPS,
				0.1f, 240.f) ||
			!Read_Number(*pAnimation, "scale", slot.fAnimationScale,
				0.01f, 100.f) ||
			!Read_Number(*pOffset, "x", slot.fAnimationOffsetX,
				-10000.f, 10000.f) ||
			!Read_Number(*pOffset, "y", slot.fAnimationOffsetY,
				-10000.f, 10000.f))
		{
			m_strDataStatus = "JSON animation contract rejected";
			return false;
		}
		for (const DATA_JSON_VALUE& frame : pFrames->Get_Array())
		{
			if (!frame.Is_String() ||
				!Is_SafeUIResourceId(frame.Get_String()))
			{
				m_strDataStatus = "JSON animation frame rejected";
				return false;
			}
			slot.AnimationFrames.push_back(frame.Get_String());
		}
		stagedSlots.push_back(move(slot));
	}

	m_Slots = move(stagedSlots);
	m_ClassNames = move(stagedClasses);
	m_iSelectedClass = 0;
	m_iSelectedSlot = -1;
	m_SelectedSlots.clear();
	m_iLastScannedClass = -1;
	m_strDataStatus = "JSON loaded";
	m_bActiveDocumentLoaded = true;
	return true;
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
