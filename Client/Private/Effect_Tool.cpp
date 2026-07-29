#include "imgui.h"

#include "Effect_Tool.h"
#include "Effect_AssetIO.h"
#include "Effect_Runtime.h"

namespace
{
	using namespace Client;

	const char* GetEmitterTypeName(const EFFECT_EMITTER_TYPE eType)
	{
		static const char* Names[] = {
			"Sprite", "Mesh", "Beam", "Ribbon", "Anim Trail"
		};
		const uint32_t iIndex = static_cast<uint32_t>(eType);
		return iIndex < static_cast<uint32_t>(EFFECT_EMITTER_TYPE::END)
			? Names[iIndex] : "Unknown";
	}

	const char* GetModuleTypeName(const EFFECT_MODULE_TYPE eType)
	{
		switch (eType)
		{
		case EFFECT_MODULE_TYPE::REQUIRED:
			return "Required";
		case EFFECT_MODULE_TYPE::SPAWN:
			return "Spawn";
		case EFFECT_MODULE_TYPE::LIFETIME:
			return "Lifetime";
		case EFFECT_MODULE_TYPE::INITIAL_LOCATION:
			return "Initial Location";
		case EFFECT_MODULE_TYPE::INITIAL_VELOCITY:
			return "Initial Velocity";
		case EFFECT_MODULE_TYPE::INITIAL_SIZE:
			return "Initial Size";
		case EFFECT_MODULE_TYPE::INITIAL_COLOR:
			return "Initial Color";
		case EFFECT_MODULE_TYPE::SIZE_OVER_LIFE:
			return "Size Over Life";
		case EFFECT_MODULE_TYPE::ALPHA_OVER_LIFE:
			return "Alpha Over Life";
		case EFFECT_MODULE_TYPE::VELOCITY_OVER_LIFE:
			return "Velocity Over Life";
		case EFFECT_MODULE_TYPE::SUB_UV:
			return "Sub UV";
		case EFFECT_MODULE_TYPE::COLLISION:
			return "Collision";
		case EFFECT_MODULE_TYPE::EVENT:
			return "Event";
		case EFFECT_MODULE_TYPE::LOD:
			return "LOD";
		case EFFECT_MODULE_TYPE::INITIAL_ROTATION:
			return "Initial Rotation";
		case EFFECT_MODULE_TYPE::ROTATION_RATE:
			return "Rotation Rate";
		case EFFECT_MODULE_TYPE::COLOR_OVER_LIFE:
			return "Color Over Life";
		case EFFECT_MODULE_TYPE::MESH_ANIMATION:
			return "Mesh Animation";
		case EFFECT_MODULE_TYPE::BEAM:
			return "Beam";
		case EFFECT_MODULE_TYPE::TRAIL:
			return "Trail";
		default:
			return "Unknown";
		}
	}

	const EFFECT_MODULE_DESC* FindModule(
		const EFFECT_EMITTER_DESC& Emitter,
		const EFFECT_MODULE_TYPE eType)
	{
		const auto Iter = find_if(Emitter.Modules.begin(),
			Emitter.Modules.end(),
			[eType](const EFFECT_MODULE_DESC& Module)
			{
				return Module.eType == eType && Module.isEnabled;
			});
		return Iter == Emitter.Modules.end() ? nullptr : &*Iter;
	}

	wstring ConvertToWide(const string& strValue)
	{
		if (strValue.empty())
			return {};

		int32_t iLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strValue.c_str(), -1, nullptr, 0);
		uint32_t iCodePage = CP_UTF8;
		uint32_t iFlags = MB_ERR_INVALID_CHARS;
		if (0 == iLength)
		{
			iCodePage = CP_ACP;
			iFlags = 0;
			iLength = MultiByteToWideChar(iCodePage, iFlags,
				strValue.c_str(), -1, nullptr, 0);
		}
		if (iLength <= 0)
			return {};
		wstring strWide(static_cast<size_t>(iLength), L'\0');
		MultiByteToWideChar(iCodePage, iFlags, strValue.c_str(), -1,
			strWide.data(), iLength);
		strWide.pop_back();
		return strWide;
	}

	string ConvertToUtf8(const wstring& strValue)
	{
		if (strValue.empty())
			return {};

		const int32_t iLength = WideCharToMultiByte(
			CP_UTF8, 0, strValue.c_str(), -1,
			nullptr, 0, nullptr, nullptr);
		if (iLength <= 0)
			return {};

		string strUtf8(static_cast<size_t>(iLength), '\0');
		WideCharToMultiByte(
			CP_UTF8, 0, strValue.c_str(), -1,
			strUtf8.data(), iLength, nullptr, nullptr);
		strUtf8.pop_back();
		return strUtf8;
	}

	string MakePortableAssetPath(const filesystem::path& SelectedPath)
	{
		error_code Error;
		const filesystem::path ResourceRoot =
			filesystem::weakly_canonical(
				"../Bin/Resources/LostArk/Effect/Effect_Tool", Error);
		if (!Error)
		{
			Error.clear();
			const filesystem::path RelativePath =
				filesystem::relative(
					SelectedPath, ResourceRoot, Error);
			if (!Error && !RelativePath.empty())
			{
				const wstring strRelative =
					RelativePath.generic_wstring();
				if (!strRelative.starts_with(L".."))
				{
					return "../Bin/Resources/LostArk/Effect/Effect_Tool/" +
						ConvertToUtf8(strRelative);
				}
			}
		}

		return ConvertToUtf8(
			SelectedPath.lexically_normal().generic_wstring());
	}

	bool_t BrowseResourceFile(const wchar_t* pFilter,
		string& OutAssetPath)
	{
		wchar_t szPath[32768] = {};
		error_code Error;
		const filesystem::path InitialDirectory =
			filesystem::weakly_canonical(
				"../Bin/Resources/LostArk/Effect/Effect_Tool", Error);
		const wstring strInitialDirectory =
			Error ? wstring{} : InitialDirectory.wstring();

		OPENFILENAMEW Dialog{};
		Dialog.lStructSize = sizeof(Dialog);
		Dialog.hwndOwner = g_hWnd;
		Dialog.lpstrFile = szPath;
		Dialog.nMaxFile = static_cast<DWORD>(size(szPath));
		Dialog.lpstrFilter = pFilter;
		Dialog.nFilterIndex = 1;
		Dialog.lpstrInitialDir = strInitialDirectory.empty()
			? nullptr : strInitialDirectory.c_str();
		Dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST |
			OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

		if (!GetOpenFileNameW(&Dialog))
			return false;

		OutAssetPath = MakePortableAssetPath(szPath);
		return !OutAssetPath.empty();
	}

	string FormatLoadError(const HRESULT hResult)
	{
		char szBuffer[64] = {};
		sprintf_s(szBuffer, "Texture load failed (HRESULT 0x%08X)",
			static_cast<uint32_t>(hResult));
		return szBuffer;
	}

	void EditString(const char* pLabel, string& strValue)
	{
		char szBuffer[256] = {};
		strncpy_s(szBuffer, strValue.c_str(), _TRUNCATE);
		if (ImGui::InputText(pLabel, szBuffer, sizeof(szBuffer)))
			strValue = szBuffer;
	}

	ImU32 ToImColor(const float4_t& vColor)
	{
		return ImGui::ColorConvertFloat4ToU32(
			ImVec4(vColor.x, vColor.y, vColor.z, vColor.w));
	}

	void EditCurve(EFFECT_CURVE_FLOAT_DESC& Curve)
	{
		for (int32_t i = 0;
			i < static_cast<int32_t>(Curve.Keys.size()); ++i)
		{
			ImGui::PushID(i);
			ImGui::SetNextItemWidth(100.f);
			ImGui::DragFloat("Time", &Curve.Keys[i].fTime,
				0.01f, 0.f, 1.f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.f);
			ImGui::DragFloat("Value", &Curve.Keys[i].fValue,
				0.02f, -10.f, 10.f);
			ImGui::SameLine();
			if (ImGui::SmallButton("X") && Curve.Keys.size() > 2)
			{
				Curve.Keys.erase(Curve.Keys.begin() + i);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}
		if (ImGui::Button("+ Key"))
			Curve.Keys.push_back({ 0.5f, 1.f });
		sort(Curve.Keys.begin(), Curve.Keys.end(),
			[](const EFFECT_CURVE_KEY& Left,
				const EFFECT_CURVE_KEY& Right)
			{
				return Left.fTime < Right.fTime;
			});

		const ImVec2 vOrigin = ImGui::GetCursorScreenPos();
		const ImVec2 vSize(max(120.f, ImGui::GetContentRegionAvail().x),
			80.f);
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		pDrawList->AddRectFilled(vOrigin,
			ImVec2(vOrigin.x + vSize.x, vOrigin.y + vSize.y),
			IM_COL32(18, 22, 30, 255));
		for (size_t i = 1; i < Curve.Keys.size(); ++i)
		{
			auto Point = [&](const EFFECT_CURVE_KEY& Key)
			{
				return ImVec2(
					vOrigin.x + clamp(Key.fTime, 0.f, 1.f) * vSize.x,
					vOrigin.y + vSize.y -
					clamp(Key.fValue, 0.f, 2.f) * 0.5f * vSize.y);
			};
			pDrawList->AddLine(Point(Curve.Keys[i - 1]),
				Point(Curve.Keys[i]), IM_COL32(80, 190, 255, 255), 2.f);
		}
		ImGui::InvisibleButton("CurveCanvas", vSize);
	}
}

Client::CEffect_Tool::CEffect_Tool(ComPtr<ID3D11Device> pDevice)
	: m_pDevice{ move(pDevice) },
	m_Phase2Validation{ Run_Phase2ParticleCountValidation() }
{
	Create_DefaultAsset();
	Rebuild_Simulators();
	Reload_SourceCatalog();

	if (nullptr != m_pDevice)
	{
		ComPtr<ID3D11DeviceContext> pContext;
		m_pDevice->GetImmediateContext(&pContext);
		auto pPrototype = CEffect_Runtime::Create(m_pDevice, pContext);
		if (nullptr != pPrototype)
		{
			m_pWorldPreview = dynamic_pointer_cast<CEffect_Runtime>(
				pPrototype->Clone(nullptr));
		}
	}

	string strError;
	m_strFileStatus = CEffect_AssetIO::Validate_RoundTrip(
		m_Asset, &strError) ? "Memory round trip: PASS" : strError;
}

Client::CEffect_Tool::~CEffect_Tool()
{
	CEffect_Runtime::Publish_Preview(
		m_Asset, false, m_vWorldPreviewPosition);
}

void Client::CEffect_Tool::Render()
{
	const f32_t fFrameDelta = max(
		0.f, ImGui::GetIO().DeltaTime * m_fTimeScale);
	if (m_isPlaying)
	{
		m_fPreviewTime += fFrameDelta;
		for (const auto& pSimulator : m_Simulators)
		{
			pSimulator->Set_LODDistance(m_fPreviewDistance);
			pSimulator->Update(fFrameDelta);
		}

		if (m_isLooping && m_Asset.fDuration > 0.f &&
			m_fPreviewTime >= m_Asset.fDuration)
		{
			Restart_Preview();
		}
	}

	Update_WorldPreview(m_isPlaying ? fFrameDelta : 0.f);

	ImGui::SetNextWindowSize(ImVec2(1180.f, 760.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool"))
	{
		ImGui::End();
		Render_SourceCatalogPanel();
		return;
	}

	Render_Toolbar();
	ImGui::Separator();
	ImGui::BeginChild("EmitterPanel", ImVec2(220.f, 0.f), true);
	Render_EmitterPanel();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("ModulePanel", ImVec2(240.f, 0.f), true);
	Render_ModulePanel();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::BeginChild("PropertyPanel", ImVec2(0.f, 270.f), true);
	Render_PropertyPanel();
	ImGui::EndChild();
	ImGui::BeginChild("PreviewPanel", ImVec2(0.f, 0.f), true);
	Render_PreviewPanel();
	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::End();
	Render_SourceCatalogPanel();
}

void Client::CEffect_Tool::Create_DefaultAsset()
{
	m_Asset = {};
	m_Asset.strAssetId = "bard_glaivier_preview";
	m_Asset.strName = "Bard Glaivier Preview";
	m_Asset.eProvenance = EFFECT_PROVENANCE::AUTHORED_VARIANT;
	m_Asset.fDuration = 4.f;

	EFFECT_EMITTER_DESC Emitter;
	Emitter.iEmitterId = m_iNextElementId++;
	Emitter.strName = "Sprite Emitter 0";
	Emitter.fDuration = 4.f;
	Emitter.iLoopCount = EFFECT_LOOP_FOREVER;

	auto AddModule = [this, &Emitter](const EFFECT_MODULE_TYPE eType)
	{
		EFFECT_MODULE_DESC Module;
		Module.iModuleId = m_iNextElementId++;
		Module.eType = eType;
		Module.strName = GetModuleTypeName(eType);
		Emitter.Modules.push_back(Module);
	};

	for (uint32_t i = 0;
		i < static_cast<uint32_t>(EFFECT_MODULE_TYPE::END); ++i)
		AddModule(static_cast<EFFECT_MODULE_TYPE>(i));

	for (EFFECT_MODULE_DESC& Module : Emitter.Modules)
	{
		switch (Module.eType)
		{
		case EFFECT_MODULE_TYPE::REQUIRED:
			Module.Required.strTextureAssetId =
				filesystem::is_regular_file(
					"../Bin/Resources/LostArk/Effect/Effect_Tool/"
					"LOSTARK_EFFECT_EXPORT_2026-07-29/"
					"09_EffectAssets_SHARED_GLTF_DDS_PNG/"
					"EFUI_CURSOREFFECT/Texture2D/cursoreffect_i5.dds")
				? "../Bin/Resources/LostArk/Effect/Effect_Tool/"
					"LOSTARK_EFFECT_EXPORT_2026-07-29/"
					"09_EffectAssets_SHARED_GLTF_DDS_PNG/"
					"EFUI_CURSOREFFECT/Texture2D/cursoreffect_i5.dds"
				: "../Bin/Resources/LostArk/Effect/"
				"LOSTARK_EFFECT_EXPORT_2026-07-29/"
				"09_EffectAssets_SHARED_GLTF_DDS_PNG/"
				"EFUI_CURSOREFFECT/Texture2D/cursoreffect_i5.dds";
			break;
		case EFFECT_MODULE_TYPE::SPAWN:
			Module.Spawn.fRatePerSecond = 28.f;
			Module.Spawn.iBurstCount = 8;
			Module.Spawn.iMaxParticles = 300;
			break;
		case EFFECT_MODULE_TYPE::LIFETIME:
			Module.Lifetime.Lifetime.fConstant = 1.5f;
			break;
		case EFFECT_MODULE_TYPE::INITIAL_LOCATION:
			Module.InitialLocation.Location.eType =
				EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE;
			Module.InitialLocation.Location.vMin = { -6.f, -2.f, 0.f };
			Module.InitialLocation.Location.vMax = { 6.f, 2.f, 0.f };
			break;
		case EFFECT_MODULE_TYPE::INITIAL_VELOCITY:
			Module.InitialVelocity.Velocity.eType =
				EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE;
			Module.InitialVelocity.Velocity.vMin = { -2.f, 4.f, 0.f };
			Module.InitialVelocity.Velocity.vMax = { 2.f, 9.f, 0.f };
			break;
		case EFFECT_MODULE_TYPE::INITIAL_SIZE:
			Module.InitialSize.Size.vConstant = { 0.45f, 0.45f, 0.45f };
			break;
		case EFFECT_MODULE_TYPE::INITIAL_COLOR:
			Module.InitialColor.Color.eType =
				EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE;
			Module.InitialColor.Color.vMin = { 0.35f, 0.55f, 1.f, 0.85f };
			Module.InitialColor.Color.vMax = { 0.95f, 0.45f, 1.f, 1.f };
			break;
		case EFFECT_MODULE_TYPE::SUB_UV:
			Module.SubUV.iColumns = 1;
			Module.SubUV.iRows = 1;
			Module.SubUV.iStartFrame = 0;
			Module.SubUV.iEndFrame = 0;
			break;
		default:
			break;
		}
	}
	m_Asset.Emitters.push_back(Emitter);
}

void Client::CEffect_Tool::Rebuild_Simulators()
{
	// A rebuild invalidates preview resources. They are loaded again lazily
	// from the current Required module path on the next preview draw.
	m_TextureResources.clear();
	m_Simulators.clear();
	for (const EFFECT_EMITTER_DESC& Emitter : m_Asset.Emitters)
	{
		auto pSimulator = make_unique<CEffect_ParticleSimulator>();
		if (pSimulator->Initialize(Emitter,
			static_cast<uint32_t>(Emitter.iEmitterId)))
			m_Simulators.push_back(move(pSimulator));
	}
	m_iSelectedEmitter = min(m_iSelectedEmitter,
		max(0, static_cast<int32_t>(m_Asset.Emitters.size()) - 1));
	m_iSelectedModule = 0;
	m_fPreviewTime = 0.f;
	m_isWorldPreviewDirty = true;
}

void Client::CEffect_Tool::Restart_Preview()
{
	m_fPreviewTime = 0.f;
	for (const auto& pSimulator : m_Simulators)
		pSimulator->Reset();
	m_isWorldPreviewDirty = true;
}

void Client::CEffect_Tool::Render_Toolbar()
{
	EditString("Asset", m_Asset.strName);
	ImGui::SameLine();
	if (ImGui::Button(m_isPlaying ? "Pause" : "Play"))
		m_isPlaying = !m_isPlaying;
	ImGui::SameLine();
	if (ImGui::Button("Restart"))
		Restart_Preview();
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_isLooping);
	ImGui::SameLine();
	if (ImGui::Checkbox("World Preview", &m_isWorldPreviewEnabled))
		m_isWorldPreviewDirty = true;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.f);
	ImGui::DragFloat("Speed", &m_fTimeScale, 0.05f, 0.f, 4.f);
	ImGui::SameLine();
	ImGui::Text("%.2f / %.2f sec", m_fPreviewTime, m_Asset.fDuration);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.f);
	ImGui::DragFloat("LOD Distance", &m_fPreviewDistance,
		1.f, 0.f, 10000.f);
	if (m_isWorldPreviewEnabled)
	{
		if (ImGui::DragFloat3("World Position",
			reinterpret_cast<float*>(&m_vWorldPreviewPosition), 0.1f))
		{
			m_isWorldPreviewDirty = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled(
			"Rendered by CEffect_Runtime in RENDERGROUP::BLEND");
	}

	if (ImGui::Button("Save JSON"))
	{
		string strError;
		m_strFileStatus = CEffect_AssetIO::Save_Json(
			m_strJsonPath, m_Asset, &strError)
			? "JSON saved + round trip ready" : strError;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load JSON"))
	{
		string strError;
		EFFECT_ASSET_DESC Loaded;
		if (CEffect_AssetIO::Load_Json(
			m_strJsonPath, Loaded, &strError))
		{
			m_Asset = move(Loaded);
			Rebuild_Simulators();
			m_strFileStatus = "JSON loaded";
		}
		else
			m_strFileStatus = strError;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cook .weffect"))
	{
		string strError;
		m_strFileStatus = CEffect_AssetIO::Save_Binary(
			m_strBinaryPath, m_Asset, &strError)
			? ".weffect cooked" : strError;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load .weffect"))
	{
		string strError;
		EFFECT_ASSET_DESC Loaded;
		if (CEffect_AssetIO::Load_Binary(
			m_strBinaryPath, Loaded, &strError))
		{
			m_Asset = move(Loaded);
			Rebuild_Simulators();
			m_strFileStatus = ".weffect loaded";
		}
		else
			m_strFileStatus = strError;
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(m_strFileStatus.c_str());
	ImGui::SameLine();
	if (ImGui::Button("Source Catalog"))
		m_isSourceCatalogOpen = !m_isSourceCatalogOpen;
}

void Client::CEffect_Tool::Reload_SourceCatalog()
{
	const filesystem::path Candidates[] = {
		"../Bin/Resources/LostArk/Effect/Effect_Tool/SourceCatalog/"
			"particle_systems.csv",
		"Bin/Resources/LostArk/Effect/Effect_Tool/SourceCatalog/"
			"particle_systems.csv",
		"Client/Bin/Resources/LostArk/Effect/Effect_Tool/SourceCatalog/"
			"particle_systems.csv"
	};

	string strError;
	for (const filesystem::path& Candidate : Candidates)
	{
		if (m_SourceCatalog.Load(Candidate, &strError))
		{
			m_strSourceCatalogStatus =
				to_string(m_SourceCatalog.Get_Entries().size()) +
				" particle names loaded";
			Refresh_SourceCatalogResults();
			return;
		}
	}

	m_SourceCatalogResults.clear();
	m_iSelectedSourceCatalogResult = -1;
	m_strSourceCatalogStatus = strError;
}

void Client::CEffect_Tool::Refresh_SourceCatalogResults()
{
	m_SourceCatalogResults = m_SourceCatalog.Find(
		m_strSourceCatalogGroup,
		m_strSourceCatalogFilter);
	m_iSelectedSourceCatalogResult = -1;
}

void Client::CEffect_Tool::Render_SourceCatalogPanel()
{
	if (!m_isSourceCatalogOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(700.f, 480.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Source Catalog",
		&m_isSourceCatalogOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Discovery metadata only: the extracted ParticleSystem names do not "
		"contain the original Cascade module/material dependency graph.");

	const char* pGroups[] = { "Bard", "Glaivier", "Common", "All" };
	int32_t iGroup = 0;
	if ("Glaivier" == m_strSourceCatalogGroup)
		iGroup = 1;
	else if ("Common" == m_strSourceCatalogGroup)
		iGroup = 2;
	else if (m_strSourceCatalogGroup.empty())
		iGroup = 3;

	bool_t isFilterChanged = false;
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::Combo("Class", &iGroup, pGroups,
		static_cast<int32_t>(size(pGroups))))
	{
		m_strSourceCatalogGroup =
			(3 == iGroup) ? string{} : pGroups[iGroup];
		isFilterChanged = true;
	}

	char szFilter[192] = {};
	strncpy_s(szFilter, m_strSourceCatalogFilter.c_str(), _TRUNCATE);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(260.f);
	if (ImGui::InputText("Name / tag", szFilter, sizeof(szFilter)))
	{
		m_strSourceCatalogFilter = szFilter;
		isFilterChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload CSV"))
		Reload_SourceCatalog();
	else if (isFilterChanged)
		Refresh_SourceCatalogResults();

	ImGui::Text("%s | %zu matches",
		m_strSourceCatalogStatus.c_str(),
		m_SourceCatalogResults.size());
	ImGui::Separator();

	ImGui::BeginChild("SourceCatalogList", ImVec2(0.f, 260.f), true);
	const vector<EFFECT_SOURCE_PARTICLE_ENTRY>& Entries =
		m_SourceCatalog.Get_Entries();
	ImGuiListClipper Clipper;
	Clipper.Begin(static_cast<int32_t>(m_SourceCatalogResults.size()));
	while (Clipper.Step())
	{
		for (int32_t i = Clipper.DisplayStart;
			i < Clipper.DisplayEnd; ++i)
		{
			const size_t iEntry = m_SourceCatalogResults[i];
			if (iEntry >= Entries.size())
				continue;

			const EFFECT_SOURCE_PARTICLE_ENTRY& Entry = Entries[iEntry];
			const string strLabel =
				Entry.strLogicalPackage + "." +
				Entry.strObjectName + "##" + to_string(iEntry);
			if (ImGui::Selectable(strLabel.c_str(),
				i == m_iSelectedSourceCatalogResult))
			{
				m_iSelectedSourceCatalogResult = i;
			}
		}
	}
	ImGui::EndChild();

	if (m_iSelectedSourceCatalogResult >= 0 &&
		m_iSelectedSourceCatalogResult <
		static_cast<int32_t>(m_SourceCatalogResults.size()))
	{
		const size_t iEntry = m_SourceCatalogResults[
			m_iSelectedSourceCatalogResult];
		if (iEntry < Entries.size())
		{
			const EFFECT_SOURCE_PARTICLE_ENTRY& Entry = Entries[iEntry];
			ImGui::Text("Group: %s | Role: %s",
				Entry.strGroup.c_str(), Entry.strRole.c_str());
			ImGui::TextWrapped("Object: %s.%s",
				Entry.strLogicalPackage.c_str(),
				Entry.strObjectName.c_str());
			ImGui::TextWrapped("Package: %s | Export: %u | Size: %llu",
				Entry.strPhysicalPackage.c_str(),
				Entry.iExportIndex,
				static_cast<unsigned long long>(Entry.iExportSize));
			ImGui::TextWrapped("Tags: %s",
				Entry.strCandidateTags.c_str());
			if (ImGui::Button("Use Name For Authored Asset"))
			{
				m_Asset.strAssetId =
					Entry.strGroup + "/" + Entry.strObjectName;
				m_Asset.strName = Entry.strObjectName;
				m_Asset.eProvenance =
					EFFECT_PROVENANCE::RECONSTRUCTED;
				m_strFileStatus =
					"Source name selected; assign extracted "
					"texture/mesh resources manually";
			}
		}
	}

	ImGui::End();
}

void Client::CEffect_Tool::Update_WorldPreview(
	const f32_t fTimeDelta)
{
	if (m_isWorldPreviewDirty)
	{
		CEffect_Runtime::Publish_Preview(
			m_Asset,
			m_isWorldPreviewEnabled,
			m_vWorldPreviewPosition);
		m_isWorldPreviewDirty = false;
	}

	if (nullptr == m_pWorldPreview)
		return;

	m_pWorldPreview->Update(fTimeDelta);
	m_pWorldPreview->Late_Update(fTimeDelta);
}

void Client::CEffect_Tool::Render_EmitterPanel()
{
	ImGui::TextUnformatted("Emitters");
	ImGui::Separator();
	for (int32_t i = 0; i < static_cast<int32_t>(m_Asset.Emitters.size()); ++i)
	{
		EFFECT_EMITTER_DESC& Emitter = m_Asset.Emitters[i];
		ImGui::PushID(i);
		ImGui::Checkbox("##enabled", &Emitter.isEnabled);
		ImGui::SameLine();
		const string strLabel = Emitter.strName + " [" +
			GetEmitterTypeName(Emitter.eType) + "]";
		if (ImGui::Selectable(strLabel.c_str(), i == m_iSelectedEmitter))
		{
			m_iSelectedEmitter = i;
			m_iSelectedModule = 0;
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	if (ImGui::Button("+ Emitter") && m_Asset.Emitters.size() < 32)
	{
		EFFECT_EMITTER_DESC NewEmitter;
		if (!m_Asset.Emitters.empty())
		{
			NewEmitter = m_Asset.Emitters.front();
			for (EFFECT_MODULE_DESC& Module : NewEmitter.Modules)
				Module.iModuleId = m_iNextElementId++;
		}
		else
		{
			auto AddModule = [this, &NewEmitter](
				const EFFECT_MODULE_TYPE eType)
			{
				EFFECT_MODULE_DESC Module;
				Module.iModuleId = m_iNextElementId++;
				Module.eType = eType;
				Module.strName = GetModuleTypeName(eType);
				NewEmitter.Modules.push_back(move(Module));
			};
			AddModule(EFFECT_MODULE_TYPE::REQUIRED);
			AddModule(EFFECT_MODULE_TYPE::SPAWN);
			AddModule(EFFECT_MODULE_TYPE::LIFETIME);
			NewEmitter.Modules[1].Spawn.fRatePerSecond = 10.f;
			NewEmitter.Modules[1].Spawn.iMaxParticles = 1000;
		}
		NewEmitter.iEmitterId = m_iNextElementId++;
		NewEmitter.strName = "Emitter " + to_string(m_Asset.Emitters.size());
		m_Asset.Emitters.push_back(NewEmitter);
		m_iSelectedEmitter =
			static_cast<int32_t>(m_Asset.Emitters.size()) - 1;
		Rebuild_Simulators();
	}
	ImGui::SameLine();
	if (ImGui::Button("- Emitter") && m_Asset.Emitters.size() > 1)
	{
		m_Asset.Emitters.erase(
			m_Asset.Emitters.begin() + m_iSelectedEmitter);
		Rebuild_Simulators();
	}
}

void Client::CEffect_Tool::Render_ModulePanel()
{
	if (m_Asset.Emitters.empty())
		return;
	EFFECT_EMITTER_DESC& Emitter = m_Asset.Emitters[m_iSelectedEmitter];
	ImGui::TextUnformatted("Modules");
	ImGui::Separator();
	for (int32_t i = 0; i < static_cast<int32_t>(Emitter.Modules.size()); ++i)
	{
		EFFECT_MODULE_DESC& Module = Emitter.Modules[i];
		ImGui::PushID(i);
		ImGui::Checkbox("##enabled", &Module.isEnabled);
		ImGui::SameLine();
		if (ImGui::Selectable(Module.strName.c_str(),
			i == m_iSelectedModule))
			m_iSelectedModule = i;
		ImGui::PopID();
	}
	ImGui::Separator();
	if (ImGui::Button("Apply / Restart"))
		Rebuild_Simulators();
}

void Client::CEffect_Tool::Render_PropertyPanel()
{
	if (m_Asset.Emitters.empty())
		return;
	EFFECT_EMITTER_DESC& Emitter = m_Asset.Emitters[m_iSelectedEmitter];
	EditString("Emitter Name", Emitter.strName);
	int iEmitterType = static_cast<int>(Emitter.eType);
	if (ImGui::Combo("Type", &iEmitterType,
		"Sprite\0Mesh\0Beam\0Ribbon\0Anim Trail\0"))
		Emitter.eType = static_cast<EFFECT_EMITTER_TYPE>(iEmitterType);
	ImGui::DragFloat("Delay", &Emitter.fDelay, 0.05f, 0.f, 60.f);
	ImGui::DragFloat("Duration", &Emitter.fDuration, 0.05f, 0.01f, 60.f);

	if (Emitter.Modules.empty())
		return;
	m_iSelectedModule = min(m_iSelectedModule,
		static_cast<int32_t>(Emitter.Modules.size()) - 1);
	EFFECT_MODULE_DESC& Module = Emitter.Modules[m_iSelectedModule];
	ImGui::SeparatorText(GetModuleTypeName(Module.eType));

	switch (Module.eType)
	{
	case EFFECT_MODULE_TYPE::REQUIRED:
		EditString("Texture Asset ID", Module.Required.strTextureAssetId);
		ImGui::SameLine();
		if (ImGui::Button("Browse Texture") &&
			BrowseResourceFile(
				L"Effect Textures\0*.dds;*.png;*.jpg;*.jpeg;*.bmp;"
				L"*.tif;*.tiff\0All Files\0*.*\0\0",
				Module.Required.strTextureAssetId))
		{
			Reload_TextureResource(Emitter);
		}
		EditString("Mesh Asset ID", Module.Required.strMeshAssetId);
		ImGui::SameLine();
		if (ImGui::Button("Browse Mesh") &&
			BrowseResourceFile(
				L"Effect Meshes\0*.wmesh;*.gltf;*.fbx\0"
					L"All Files\0*.*\0\0",
				Module.Required.strMeshAssetId))
		{
			m_isWorldPreviewDirty = true;
		}
		EditString("Material Asset ID", Module.Required.strMaterialAssetId);
		ImGui::SameLine();
		if (ImGui::Button("Browse Material") &&
			BrowseResourceFile(
				L"Effect Materials\0*.wmat;*.mat\0"
					L"All Files\0*.*\0\0",
				Module.Required.strMaterialAssetId))
		{
			m_isWorldPreviewDirty = true;
		}
		if (ImGui::Button("Reload Resource"))
			Reload_TextureResource(Emitter);
		ImGui::SameLine();
		ImGui::TextWrapped("%s",
			Get_TextureResourceStatus(Emitter).c_str());
		break;
	case EFFECT_MODULE_TYPE::SPAWN:
		ImGui::DragFloat("Rate / sec", &Module.Spawn.fRatePerSecond,
			0.5f, 0.f, 10000.f);
		ImGui::DragScalar("Burst", ImGuiDataType_U32,
			&Module.Spawn.iBurstCount, 1.f);
		ImGui::DragScalar("Max Particles", ImGuiDataType_U32,
			&Module.Spawn.iMaxParticles, 1.f);
		break;
	case EFFECT_MODULE_TYPE::LIFETIME:
		ImGui::DragFloat("Lifetime",
			&Module.Lifetime.Lifetime.fConstant, 0.05f, 0.01f, 60.f);
		break;
	case EFFECT_MODULE_TYPE::INITIAL_LOCATION:
		ImGui::DragFloat3("Location Min",
			reinterpret_cast<float*>(&Module.InitialLocation.Location.vMin), 0.1f);
		ImGui::DragFloat3("Location Max",
			reinterpret_cast<float*>(&Module.InitialLocation.Location.vMax), 0.1f);
		break;
	case EFFECT_MODULE_TYPE::INITIAL_VELOCITY:
		ImGui::DragFloat3("Velocity Min",
			reinterpret_cast<float*>(&Module.InitialVelocity.Velocity.vMin), 0.1f);
		ImGui::DragFloat3("Velocity Max",
			reinterpret_cast<float*>(&Module.InitialVelocity.Velocity.vMax), 0.1f);
		break;
	case EFFECT_MODULE_TYPE::INITIAL_SIZE:
		ImGui::DragFloat3("Size",
			reinterpret_cast<float*>(&Module.InitialSize.Size.vConstant), 0.05f);
		break;
	case EFFECT_MODULE_TYPE::INITIAL_COLOR:
		ImGui::ColorEdit4("Color Min",
			reinterpret_cast<float*>(&Module.InitialColor.Color.vMin));
		ImGui::ColorEdit4("Color Max",
			reinterpret_cast<float*>(&Module.InitialColor.Color.vMax));
		break;
	case EFFECT_MODULE_TYPE::SIZE_OVER_LIFE:
		EditCurve(Module.SizeOverLife.Curve);
		break;
	case EFFECT_MODULE_TYPE::ALPHA_OVER_LIFE:
		EditCurve(Module.AlphaOverLife.Curve);
		break;
	case EFFECT_MODULE_TYPE::VELOCITY_OVER_LIFE:
		EditCurve(Module.VelocityOverLife.Curve);
		break;
	case EFFECT_MODULE_TYPE::SUB_UV:
		ImGui::DragScalar("Columns", ImGuiDataType_U32,
			&Module.SubUV.iColumns, 1.f);
		ImGui::DragScalar("Rows", ImGuiDataType_U32,
			&Module.SubUV.iRows, 1.f);
		ImGui::DragFloat("Frames / sec",
			&Module.SubUV.fFramesPerSecond, 0.25f, 0.f, 240.f);
		ImGui::DragScalar("Start Frame", ImGuiDataType_U32,
			&Module.SubUV.iStartFrame, 1.f);
		ImGui::DragScalar("End Frame", ImGuiDataType_U32,
			&Module.SubUV.iEndFrame, 1.f);
		ImGui::Checkbox("Loop Frames", &Module.SubUV.isLoop);
		ImGui::Checkbox("Use Particle Relative Time",
			&Module.SubUV.useParticleRelativeTime);
		break;
	case EFFECT_MODULE_TYPE::COLLISION:
		ImGui::DragFloat3("Bounds Min",
			reinterpret_cast<float*>(&Module.Collision.vMinBounds), 0.1f);
		ImGui::DragFloat3("Bounds Max",
			reinterpret_cast<float*>(&Module.Collision.vMaxBounds), 0.1f);
		ImGui::SliderFloat("Restitution",
			&Module.Collision.fRestitution, 0.f, 1.f);
		break;
	case EFFECT_MODULE_TYPE::EVENT:
		EditString("Event Name", Module.Event.strEventName);
		ImGui::SliderFloat("Normalized Time",
			&Module.Event.fNormalizedTime, 0.f, 1.f);
		break;
	case EFFECT_MODULE_TYPE::LOD:
		ImGui::DragFloat("Near Distance",
			&Module.LOD.fNearDistance, 1.f, 0.f, 10000.f);
		ImGui::DragFloat("Far Distance",
			&Module.LOD.fFarDistance, 1.f, 0.f, 10000.f);
		ImGui::SliderFloat("Far Spawn Scale",
			&Module.LOD.fFarSpawnScale, 0.f, 1.f);
		break;
	case EFFECT_MODULE_TYPE::INITIAL_ROTATION:
	{
		int32_t iDistributionType = static_cast<int32_t>(
			Module.InitialRotation.RotationDegrees.eType);
		if (ImGui::Combo("Distribution", &iDistributionType,
			"Constant\0Uniform Range\0"))
		{
			Module.InitialRotation.RotationDegrees.eType =
				static_cast<EFFECT_DISTRIBUTION_TYPE>(
					iDistributionType);
		}
		if (Module.InitialRotation.RotationDegrees.eType ==
			EFFECT_DISTRIBUTION_TYPE::CONSTANT)
		{
			ImGui::DragFloat3("Rotation (deg)",
				reinterpret_cast<float*>(
					&Module.InitialRotation.RotationDegrees.vConstant),
				0.5f);
		}
		else
		{
			ImGui::DragFloat3("Rotation Min (deg)",
				reinterpret_cast<float*>(
					&Module.InitialRotation.RotationDegrees.vMin),
				0.5f);
			ImGui::DragFloat3("Rotation Max (deg)",
				reinterpret_cast<float*>(
					&Module.InitialRotation.RotationDegrees.vMax),
				0.5f);
		}
		break;
	}
	case EFFECT_MODULE_TYPE::ROTATION_RATE:
	{
		int32_t iDistributionType = static_cast<int32_t>(
			Module.RotationRate.RotationRateDegreesPerSecond.eType);
		if (ImGui::Combo("Distribution", &iDistributionType,
			"Constant\0Uniform Range\0"))
		{
			Module.RotationRate.RotationRateDegreesPerSecond.eType =
				static_cast<EFFECT_DISTRIBUTION_TYPE>(
					iDistributionType);
		}
		if (Module.RotationRate.RotationRateDegreesPerSecond.eType ==
			EFFECT_DISTRIBUTION_TYPE::CONSTANT)
		{
			ImGui::DragFloat3("Rate (deg/sec)",
				reinterpret_cast<float*>(
					&Module.RotationRate.
					RotationRateDegreesPerSecond.vConstant),
				0.5f);
		}
		else
		{
			ImGui::DragFloat3("Rate Min (deg/sec)",
				reinterpret_cast<float*>(
					&Module.RotationRate.
					RotationRateDegreesPerSecond.vMin),
				0.5f);
			ImGui::DragFloat3("Rate Max (deg/sec)",
				reinterpret_cast<float*>(
					&Module.RotationRate.
					RotationRateDegreesPerSecond.vMax),
				0.5f);
		}
		break;
	}
	case EFFECT_MODULE_TYPE::COLOR_OVER_LIFE:
		ImGui::TextUnformatted("Red Multiplier");
		ImGui::PushID("ColorOverLifeRed");
		EditCurve(Module.ColorOverLife.Red);
		ImGui::PopID();
		ImGui::TextUnformatted("Green Multiplier");
		ImGui::PushID("ColorOverLifeGreen");
		EditCurve(Module.ColorOverLife.Green);
		ImGui::PopID();
		ImGui::TextUnformatted("Blue Multiplier");
		ImGui::PushID("ColorOverLifeBlue");
		EditCurve(Module.ColorOverLife.Blue);
		ImGui::PopID();
		ImGui::TextUnformatted("Alpha Multiplier");
		ImGui::PushID("ColorOverLifeAlpha");
		EditCurve(Module.ColorOverLife.Alpha);
		ImGui::PopID();
		break;
	case EFFECT_MODULE_TYPE::MESH_ANIMATION:
		ImGui::DragScalar("Animation Index", ImGuiDataType_U32,
			&Module.MeshAnimation.iAnimationIndex, 1.f);
		ImGui::DragFloat("Play Rate",
			&Module.MeshAnimation.fPlayRate, 0.05f, 0.f, 20.f);
		ImGui::Checkbox("Loop Animation",
			&Module.MeshAnimation.isLoop);
		break;
	case EFFECT_MODULE_TYPE::BEAM:
		ImGui::DragFloat3("Source Offset",
			reinterpret_cast<float*>(&Module.Beam.vSourceOffset),
			0.1f);
		ImGui::DragFloat3("Target Offset",
			reinterpret_cast<float*>(&Module.Beam.vTargetOffset),
			0.1f);
		ImGui::DragFloat("Width", &Module.Beam.fWidth,
			0.05f, 0.001f, 1000.f);
		ImGui::DragScalar("Segments", ImGuiDataType_U32,
			&Module.Beam.iSegments, 1.f);
		ImGui::DragFloat("Noise Amplitude",
			&Module.Beam.fNoiseAmplitude, 0.05f, 0.f, 1000.f);
		break;
	case EFFECT_MODULE_TYPE::TRAIL:
		ImGui::DragFloat("Width", &Module.Trail.fWidth,
			0.05f, 0.001f, 1000.f);
		ImGui::DragFloat("Point Lifetime",
			&Module.Trail.fPointLifetime, 0.01f, 0.001f, 60.f);
		ImGui::DragScalar("Max Points", ImGuiDataType_U32,
			&Module.Trail.iMaxPoints, 1.f);
		EditString("Source Bone", Module.Trail.strSourceBone);
		EditString("Target Bone", Module.Trail.strTargetBone);
		break;
	default:
		break;
	}
}

void Client::CEffect_Tool::Render_PreviewPanel()
{
	uint32_t iAliveCount = {};
	uint64_t iCollisionCount = {};
	uint64_t iEventCount = {};
	for (const auto& pSimulator : m_Simulators)
	{
		iAliveCount += pSimulator->Get_Stats().iAliveCount;
		iCollisionCount += pSimulator->Get_Stats().iCollisionCount;
		iEventCount += pSimulator->Get_Stats().iEventCount;
	}
	ImGui::Text("Preview | %s | Alive %u | Hit %llu | Event %llu | Phase2 %s",
		m_isPlaying ? "Playing" : "Paused", iAliveCount,
		iCollisionCount, iEventCount,
		m_Phase2Validation.isPassed ? "PASS" : "FAIL");
	const ImVec2 vOrigin = ImGui::GetCursorScreenPos();
	const ImVec2 vSize = ImGui::GetContentRegionAvail();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	pDrawList->AddRectFilled(vOrigin,
		ImVec2(vOrigin.x + vSize.x, vOrigin.y + vSize.y),
		IM_COL32(15, 18, 25, 255));
	Draw_Particles(pDrawList, vOrigin, vSize, 18.f);
	ImGui::InvisibleButton("PreviewCanvas", vSize);
}

void Client::CEffect_Tool::Draw_Particles(
	ImDrawList* pDrawList, const ImVec2& vOrigin,
	const ImVec2& vSize, const f32_t fScale)
{
	const ImVec2 vCenter(vOrigin.x + vSize.x * 0.5f,
		vOrigin.y + vSize.y * 0.65f);
	pDrawList->AddLine(ImVec2(vOrigin.x, vCenter.y),
		ImVec2(vOrigin.x + vSize.x, vCenter.y),
		IM_COL32(55, 60, 70, 255));
	pDrawList->AddLine(ImVec2(vCenter.x, vOrigin.y),
		ImVec2(vCenter.x, vOrigin.y + vSize.y),
		IM_COL32(55, 60, 70, 255));

	for (size_t iEmitter = 0;
		iEmitter < m_Simulators.size() &&
		iEmitter < m_Asset.Emitters.size(); ++iEmitter)
	{
		const EFFECT_EMITTER_DESC& Emitter = m_Asset.Emitters[iEmitter];
		if (!Emitter.isEnabled)
			continue;

		ID3D11ShaderResourceView* pTextureResource = nullptr;
		uint32_t iSubUVColumns = 1;
		uint32_t iSubUVRows = 1;
		if (Emitter.eType == EFFECT_EMITTER_TYPE::SPRITE)
		{
			pTextureResource = Resolve_TextureResource(Emitter);
			if (const EFFECT_MODULE_DESC* pSubUV =
				FindModule(Emitter, EFFECT_MODULE_TYPE::SUB_UV))
			{
				iSubUVColumns = max(1u, pSubUV->SubUV.iColumns);
				iSubUVRows = max(1u, pSubUV->SubUV.iRows);
			}
		}

		vector<ImVec2> TrailPoints;
		for (const EFFECT_PARTICLE& Particle :
			m_Simulators[iEmitter]->Get_Particles())
		{
			const ImVec2 vPosition(
				vCenter.x + Particle.vPosition.x * fScale,
				vCenter.y - Particle.vPosition.y * fScale);
			const float fRadius = max(2.f,
				Particle.vSize.x * fScale * 0.5f);
			const ImU32 iColor = ToImColor(Particle.vColor);
			switch (Emitter.eType)
			{
			case EFFECT_EMITTER_TYPE::MESH:
				pDrawList->AddTriangleFilled(
					ImVec2(vPosition.x, vPosition.y - fRadius),
					ImVec2(vPosition.x - fRadius, vPosition.y + fRadius),
					ImVec2(vPosition.x + fRadius, vPosition.y + fRadius),
					iColor);
				break;
			case EFFECT_EMITTER_TYPE::BEAM:
				pDrawList->AddLine(vCenter, vPosition, iColor,
					max(1.f, fRadius * 0.3f));
				break;
			case EFFECT_EMITTER_TYPE::RIBBON:
			case EFFECT_EMITTER_TYPE::ANIM_TRAIL:
				TrailPoints.push_back(vPosition);
				break;
			default:
				if (nullptr != pTextureResource)
				{
					const uint32_t iFrameCount =
						iSubUVColumns * iSubUVRows;
					const uint32_t iFrame =
						Particle.iSubUVFrame % max(1u, iFrameCount);
					const uint32_t iColumn = iFrame % iSubUVColumns;
					const uint32_t iRow = iFrame / iSubUVColumns;
					const float fU0 =
						static_cast<float>(iColumn) / iSubUVColumns;
					const float fV0 =
						static_cast<float>(iRow) / iSubUVRows;
					const float fU1 =
						static_cast<float>(iColumn + 1) / iSubUVColumns;
					const float fV1 =
						static_cast<float>(iRow + 1) / iSubUVRows;
					const ImTextureRef Texture(
						static_cast<ImTextureID>(
							reinterpret_cast<intptr_t>(
								pTextureResource)));
					const float fHalfWidth = max(2.f,
						fabsf(Particle.vSize.x) * fScale * 0.5f);
					const float fHalfHeight = max(2.f,
						fabsf(Particle.vSize.y) * fScale * 0.5f);
					const float fRotationRadians =
						XMConvertToRadians(Particle.vRotation.z);
					const float fCos = cosf(fRotationRadians);
					const float fSin = sinf(fRotationRadians);
					const auto RotatePoint =
						[&](const float fX, const float fY)
						{
							return ImVec2(
								vPosition.x + fX * fCos - fY * fSin,
								vPosition.y + fX * fSin + fY * fCos);
						};

					pDrawList->AddImageQuad(Texture,
						RotatePoint(-fHalfWidth, -fHalfHeight),
						RotatePoint(fHalfWidth, -fHalfHeight),
						RotatePoint(fHalfWidth, fHalfHeight),
						RotatePoint(-fHalfWidth, fHalfHeight),
						ImVec2(fU0, fV0), ImVec2(fU1, fV0),
						ImVec2(fU1, fV1), ImVec2(fU0, fV1),
						iColor);
				}
				else
				{
					// Keep the old proxy visible when the selected resource
					// cannot be decoded by DDS/WIC.
					pDrawList->AddCircleFilled(
						vPosition, fRadius, iColor, 12);
				}
				break;
			}
		}
		if (TrailPoints.size() > 1)
			pDrawList->AddPolyline(TrailPoints.data(),
				static_cast<int>(TrailPoints.size()),
				IM_COL32(120, 190, 255, 220), 0,
				Emitter.eType == EFFECT_EMITTER_TYPE::ANIM_TRAIL ? 6.f : 3.f);
	}
}

ID3D11ShaderResourceView*
Client::CEffect_Tool::Resolve_TextureResource(
	const EFFECT_EMITTER_DESC& Emitter)
{
	const EFFECT_MODULE_DESC* pRequired =
		FindModule(Emitter, EFFECT_MODULE_TYPE::REQUIRED);
	const string strAssetId = nullptr != pRequired
		? pRequired->Required.strTextureAssetId : string{};

	TEXTURE_PREVIEW_RESOURCE& Resource =
		m_TextureResources[Emitter.iEmitterId];
	if (Resource.strAssetId != strAssetId)
	{
		Resource = {};
		Resource.strAssetId = strAssetId;
	}

	if (Resource.isLoadAttempted)
		return Resource.pShaderResourceView.Get();

	Resource.isLoadAttempted = true;
	if (strAssetId.empty())
	{
		Resource.strStatus = "No Texture Asset ID";
		return nullptr;
	}
	if (nullptr == m_pDevice)
	{
		Resource.strStatus = "Texture load failed: D3D11 device is null";
		return nullptr;
	}

	const wstring strPath = ConvertToWide(strAssetId);
	if (strPath.empty())
	{
		Resource.strStatus =
			"Texture load failed: invalid UTF-8/ANSI path";
		return nullptr;
	}

	const size_t iExtensionPosition = strAssetId.find_last_of('.');
	const string strExtension =
		iExtensionPosition == string::npos
		? string{} : strAssetId.substr(iExtensionPosition);
	HRESULT hResult = E_FAIL;
	if (0 == _stricmp(strExtension.c_str(), ".dds"))
	{
		hResult = DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(), strPath.c_str(), nullptr,
			Resource.pShaderResourceView.ReleaseAndGetAddressOf());
	}
	else
	{
		hResult = DirectX::CreateWICTextureFromFile(
			m_pDevice.Get(), strPath.c_str(), nullptr,
			Resource.pShaderResourceView.ReleaseAndGetAddressOf());
	}

	if (FAILED(hResult))
	{
		Resource.pShaderResourceView.Reset();
		Resource.strStatus = FormatLoadError(hResult) + ": " + strAssetId;
		return nullptr;
	}

	Resource.strStatus = "Loaded: " + strAssetId;
	return Resource.pShaderResourceView.Get();
}

const string& Client::CEffect_Tool::Get_TextureResourceStatus(
	const EFFECT_EMITTER_DESC& Emitter) const
{
	static const string strNotLoaded = "Not loaded";
	const auto Iter = m_TextureResources.find(Emitter.iEmitterId);
	return Iter == m_TextureResources.end()
		? strNotLoaded : Iter->second.strStatus;
}

void Client::CEffect_Tool::Reload_TextureResource(
	const EFFECT_EMITTER_DESC& Emitter)
{
	m_TextureResources.erase(Emitter.iEmitterId);
	Resolve_TextureResource(Emitter);
	m_isWorldPreviewDirty = true;
}
