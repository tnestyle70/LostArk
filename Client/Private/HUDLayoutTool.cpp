#include "imgui.h"

#include "HUDLayoutTool.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>

namespace
{
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
}

Client::CHUDLayoutTool::CHUDLayoutTool(ComPtr<ID3D11Device> pDevice)
	: m_pDevice { pDevice }
{
	/* CreateWICTextureFromFile needs COM on the calling thread; the main thread never initializes it (only the level-loading worker thread does). */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	Load(ms_szDefaultSavePath);

	if (m_Slots.empty())
		Reset_Default();
}

void Client::CHUDLayoutTool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(1200.f, 640.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk HUD Layout Tool"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("F1: open / close the editor tool workspace   |   drag box body to move, drag yellow corner to resize.");
	ImGui::TextUnformatted("Drag this window onto Map Tool to dock as a tab.");

	if (ImGui::Button("Save"))
		Save(ms_szDefaultSavePath);
	ImGui::SameLine();
	if (ImGui::Button("Load"))
		Load(ms_szDefaultSavePath);
	ImGui::SameLine();
	if (ImGui::Button("Reset to Default Layout"))
		Reset_Default();

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
	ImGui::BeginChild("Palette", ImVec2(190.f, ms_fRefHeight * m_fCanvasScale + 16.f), true);

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

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Render_Canvas()
{
	ImGui::BeginChild("HUDCanvas",
		ImVec2(ms_fRefWidth * m_fCanvasScale + 16.f, ms_fRefHeight * m_fCanvasScale + 16.f),
		true, ImGuiWindowFlags_NoScrollbar);

	const ImVec2 vOrigin = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	const ImVec2 vCanvasEnd(vOrigin.x + ms_fRefWidth * m_fCanvasScale, vOrigin.y + ms_fRefHeight * m_fCanvasScale);
	pDrawList->AddRectFilled(vOrigin, vCanvasEnd, IM_COL32(110, 110, 116, 255));
	pDrawList->AddRect(vOrigin, vCanvasEnd, IM_COL32(200, 200, 205, 255));

	ImGui::SetCursorScreenPos(vOrigin);
	ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("CanvasBackground", ImVec2(vCanvasEnd.x - vOrigin.x, vCanvasEnd.y - vOrigin.y));

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

	for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
	{
		HUD_SLOT& Slot = m_Slots[i];

		const ImVec2 vTopLeft(vOrigin.x + Slot.fX * m_fCanvasScale, vOrigin.y + Slot.fY * m_fCanvasScale);
		const ImVec2 vBotRight(vTopLeft.x + Slot.fSizeX * m_fCanvasScale, vTopLeft.y + Slot.fSizeY * m_fCanvasScale);

		const bool_t bSelected = (i == m_iSelectedSlot);
		const ImU32 iFillColor = Slot.bPerClass
			? IM_COL32(120, 70, 170, bSelected ? 200 : 120)
			: IM_COL32(60, 130, 190, bSelected ? 200 : 110);
		const ImU32 iBorderColor = bSelected ? IM_COL32(255, 220, 90, 255) : IM_COL32(200, 200, 210, 180);

		bool_t bAnyLayerDrawn = false;

		for (const string& strLayerPath : Slot.TextureLayers)
		{
			if (strLayerPath.empty())
				continue;

			ID3D11ShaderResourceView* pLayerSRV = Get_Or_Load_Texture(strLayerPath);
			if (nullptr == pLayerSRV)
				continue;

			pDrawList->AddImage(pLayerSRV, vTopLeft, vBotRight);
			bAnyLayerDrawn = true;
		}

		if (Slot.bPerClass)
		{
			auto ClassTexIter = Slot.ClassTextures.find(Current_Class());
			if (Slot.ClassTextures.end() != ClassTexIter && !ClassTexIter->second.empty())
			{
				ID3D11ShaderResourceView* pClassSRV = Get_Or_Load_Texture(ClassTexIter->second);
				if (nullptr != pClassSRV)
				{
					pDrawList->AddImage(pClassSRV, vTopLeft, vBotRight);
					bAnyLayerDrawn = true;
				}
			}
		}

		if (!bAnyLayerDrawn)
			pDrawList->AddRectFilled(vTopLeft, vBotRight, iFillColor);

		pDrawList->AddRect(vTopLeft, vBotRight, iBorderColor);
		pDrawList->AddText(ImVec2(vTopLeft.x + 3.f, vTopLeft.y + 2.f), IM_COL32(255, 255, 255, 255), Slot.strName.c_str());

		ImGui::PushID(i);

		ImGui::SetCursorScreenPos(vTopLeft);
		ImGui::InvisibleButton("body", ImVec2(vBotRight.x - vTopLeft.x, vBotRight.y - vTopLeft.y));
		if (ImGui::IsItemActivated())
			m_iSelectedSlot = i;
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			const ImVec2 vDelta = ImGui::GetIO().MouseDelta;
			Slot.fX += vDelta.x / m_fCanvasScale;
			Slot.fY += vDelta.y / m_fCanvasScale;
		}

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

		ImGui::PopID();
	}

	ImGui::EndChild();
}

void Client::CHUDLayoutTool::Render_Inspector()
{
	ImGui::BeginChild("Inspector", ImVec2(300.f, ms_fRefHeight * m_fCanvasScale + 16.f), true);

	ImGui::SeparatorText("Slot List");
	{
		ImGui::BeginChild("SlotList", ImVec2(0.f, 180.f), true);
		for (int32_t i = 0; i < static_cast<int32_t>(m_Slots.size()); ++i)
		{
			ImGui::PushID(i);
			const bool_t bSelected = (i == m_iSelectedSlot);
			if (ImGui::Selectable(m_Slots[i].strName.c_str(), bSelected))
				m_iSelectedSlot = i;
			ImGui::PopID();
		}
		ImGui::EndChild();
	}

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

		ImGui::SeparatorText("Texture Layers (bottom to top)");
		for (int32_t iLayer = 0; iLayer < static_cast<int32_t>(Slot.TextureLayers.size()); ++iLayer)
		{
			ImGui::PushID(iLayer);
			ImGui::TextWrapped("%d: %s", iLayer, Slot.TextureLayers[iLayer].c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove"))
			{
				Slot.TextureLayers.erase(Slot.TextureLayers.begin() + iLayer);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}

		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputText("##NewLayerPath", m_szNewLayerPathBuffer, sizeof(m_szNewLayerPathBuffer));
		if (ImGui::Button("Add Layer") && '\0' != m_szNewLayerPathBuffer[0])
		{
			Slot.TextureLayers.push_back(m_szNewLayerPathBuffer);
			m_szNewLayerPathBuffer[0] = '\0';
		}

		ImGui::Checkbox("Per-Class Texture", &Slot.bPerClass);

		if (Slot.bPerClass && !m_ClassNames.empty())
		{
			const string& strClass = Current_Class();
			string& strTexture = Slot.ClassTextures[strClass];

			strncpy_s(m_szTexturePathBuffer, strTexture.c_str(), sizeof(m_szTexturePathBuffer) - 1);

			ImGui::Text("Class: %s", strClass.c_str());
			if (ImGui::InputText("Class Texture Path", m_szTexturePathBuffer, sizeof(m_szTexturePathBuffer)))
				strTexture = m_szTexturePathBuffer;
		}

		if (ImGui::Button("Duplicate Slot (same rect)"))
			Duplicate_Slot(m_iSelectedSlot);
		ImGui::SameLine();
		if (ImGui::Button("Delete Slot"))
		{
			Delete_Slot(m_iSelectedSlot);
			m_iSelectedSlot = -1;
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
	Slot.bPerClass = pEntry->bPerClass;
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
	Slot.ClassTextures.clear();

	m_Slots.push_back(move(Slot));
	m_iSelectedSlot = static_cast<int32_t>(m_Slots.size()) - 1;
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

	for (HUD_SLOT& Slot : m_Slots)
		Slot.ClassTextures.erase(strRemoved);

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
		File << "SLOT " << Slot.strName << " "
			<< Slot.fX << " " << Slot.fY << " "
			<< Slot.fSizeX << " " << Slot.fSizeY << " "
			<< (Slot.bPerClass ? 1 : 0) << " "
			<< static_cast<int32_t>(Slot.eType) << "\n";

		for (const string& strLayerPath : Slot.TextureLayers)
		{
			if (strLayerPath.empty())
				continue;

			File << "TEXTURE " << Slot.strName << " " << strLayerPath << "\n";
		}

		for (const pair<const string, string>& ClassTexture : Slot.ClassTextures)
		{
			if (ClassTexture.second.empty())
				continue;

			File << "CLASSTEX " << Slot.strName << " " << ClassTexture.first << " " << ClassTexture.second << "\n";
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
			int32_t iPerClass = 0;
			int32_t iType = 0;
			Stream >> Slot.strName >> Slot.fX >> Slot.fY >> Slot.fSizeX >> Slot.fSizeY >> iPerClass >> iType;
			Slot.bPerClass = (0 != iPerClass);
			Slot.eType = static_cast<SLOT_TYPE>(iType);
			Slots.push_back(move(Slot));
		}
		else if ("TEXTURE" == strTag)
		{
			string strSlotName, strTexturePath;
			Stream >> strSlotName;
			Stream >> ws;
			getline(Stream, strTexturePath);

			for (HUD_SLOT& Slot : Slots)
			{
				if (Slot.strName == strSlotName)
				{
					Slot.TextureLayers.push_back(strTexturePath);
					break;
				}
			}
		}
		else if ("CLASSTEX" == strTag)
		{
			string strSlotName, strClass, strTexturePath;
			Stream >> strSlotName >> strClass;
			Stream >> ws;
			getline(Stream, strTexturePath);

			for (HUD_SLOT& Slot : Slots)
			{
				if (Slot.strName == strSlotName)
				{
					Slot.ClassTextures[strClass] = strTexturePath;
					break;
				}
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

	auto Fn_AddSlot = [this](const string& strName, float fX, float fY, float fW, float fH, bool_t bPerClass = false)
	{
		HUD_SLOT Slot{};
		Slot.strName = strName;
		Slot.fX = fX;
		Slot.fY = fY;
		Slot.fSizeX = fW;
		Slot.fSizeY = fH;
		Slot.bPerClass = bPerClass;
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

	Fn_AddSlot("Identity", 592.f, 590.f, 96.f, 96.f, true);

	Fn_AddSlot("ManaBar", 712.f, 600.f, 258.f, 20.f);
	Fn_AddSlot("Skill_F1", 968.f, 628.f, 46.f, 46.f);

	for (int32_t i = 0; i < 5; ++i)
		Fn_AddSlot("Item_" + to_string(i + 1), 1018.f + i * 50.f, 628.f, 46.f, 46.f);

	for (int32_t i = 0; i < 4; ++i)
		Fn_AddSlot("Item_" + to_string(i + 6), 1018.f + i * 50.f, 682.f, 46.f, 46.f);
}
