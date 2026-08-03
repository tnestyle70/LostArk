#include "imgui.h"

#include "Effect_Tool.h"
#include "Effect_AssetIO.h"
#include "Effect_Runtime.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <DirectXPackedVector.h>
#include <fstream>

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
		case EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER:
			return "Dynamic Parameter";
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

	f32_t CalculatePreviewDuration(const EFFECT_ASSET_DESC& Asset)
	{
		f32_t fPreviewDuration = max(0.f, Asset.fDuration);

		for (const EFFECT_EMITTER_DESC& Emitter : Asset.Emitters)
		{
			if (!Emitter.isEnabled)
				continue;

			f32_t fMaxLifetime = {};
			const EFFECT_MODULE_DESC* pLifetime =
				FindModule(Emitter, EFFECT_MODULE_TYPE::LIFETIME);
			if (nullptr != pLifetime)
			{
				const EFFECT_DISTRIBUTION_FLOAT_DESC& Lifetime =
					pLifetime->Lifetime.Lifetime;
				fMaxLifetime =
					EFFECT_DISTRIBUTION_TYPE::UNIFORM_RANGE ==
					Lifetime.eType
					? max(Lifetime.fMin, Lifetime.fMax)
					: Lifetime.fConstant;
			}

			const uint32_t iPreviewLoopCount =
				EFFECT_LOOP_FOREVER == Emitter.iLoopCount
				? 1u : max(1u, Emitter.iLoopCount);
			const f32_t fEmitterEnd =
				max(0.f, Emitter.fDelay) +
				max(0.f, Emitter.fDuration) *
				static_cast<f32_t>(iPreviewLoopCount) +
				max(0.f, fMaxLifetime);
			fPreviewDuration = max(fPreviewDuration, fEmitterEnd);
		}

		return fPreviewDuration;
	}

	filesystem::path ResolveToolAssetPath(const string& strAssetId)
	{
		if (strAssetId.empty())
			return {};

		string strNormalized = strAssetId;
		replace(strNormalized.begin(), strNormalized.end(), '\\', '/');

		string strRuntimeRelative = strNormalized;
		constexpr string_view RESOURCE_PREFIXES[] = {
			"../Bin/Resources/"
		};
		for (const string_view prefix : RESOURCE_PREFIXES)
		{
			if (strRuntimeRelative.starts_with(prefix))
			{
				strRuntimeRelative.erase(0, prefix.size());
				break;
			}
		}

		const u8string Utf8Relative(
			strRuntimeRelative.begin(), strRuntimeRelative.end());
		const filesystem::path RuntimeRelativePath =
			filesystem::path(Utf8Relative).lexically_normal();
		if (RuntimeRelativePath.is_absolute() ||
			RuntimeRelativePath.has_root_path())
		{
			return {};
		}

		const filesystem::path Candidates[] = {
			CRuntimeAssetRoot::Resolve(RuntimeRelativePath),
			CRuntimeAssetRoot::Resolve(
				filesystem::path(L"Effect") /
					RuntimeRelativePath)
		};

		error_code Error;
		for (const filesystem::path& Candidate : Candidates)
		{
			Error.clear();
			if (filesystem::is_regular_file(Candidate, Error))
				return Candidate.lexically_normal();
		}
		return {};
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
				CRuntimeAssetRoot::Get(), Error);
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
					return ConvertToUtf8(strRelative);
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
				CRuntimeAssetRoot::Get() / L"Effect", Error);
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
	m_strAuthoringPath = CProjectDataRoot::Resolve(
		L"Effects/Editor/working.effect").string();
	m_strBinaryPath = CProjectDataRoot::Resolve(
		L"Effects/Editor/working.weffect").string();
	Create_DefaultAsset();
	Rebuild_Simulators();
	Reload_SourceCatalog();
	Refresh_AuthoredList();

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
	f32_t fSimulationDelta = {};
	if (m_isPlaying)
	{
		fSimulationDelta = fFrameDelta;
		const f32_t fPreviewDuration =
			CalculatePreviewDuration(m_Asset);
		const f32_t fNextPreviewTime =
			m_fPreviewTime + fFrameDelta;

		if (m_isLooping && fPreviewDuration > 0.f &&
			fNextPreviewTime >= fPreviewDuration)
		{
			fSimulationDelta = fmodf(
				fNextPreviewTime, fPreviewDuration);
			Restart_Preview();
			m_fPreviewTime = fSimulationDelta;
		}
		else
		{
			m_fPreviewTime = fNextPreviewTime;
		}

		for (const auto& pSimulator : m_Simulators)
		{
			if (nullptr == pSimulator)
				continue;
			pSimulator->Set_LODDistance(m_fPreviewDistance);
			pSimulator->Update(fSimulationDelta);
		}
	}

	Update_WorldPreview(fSimulationDelta);

	/* ImGui 창 상태와 무관하게 돌아야 하므로 Begin 앞에서 호출한다. */
	Capture_SceneHDR_Readback();

	ImGui::SetNextWindowSize(ImVec2(1180.f, 760.f),
		ImGuiCond_FirstUseEver);
	if (m_isFocusRequested)
	{
		ImGui::SetNextWindowFocus();
		m_isFocusRequested = false;
	}
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
				"Effect/Shared/Cursor/cursoreffect_i5.dds";
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
		// A simulator refuses to initialize when the emitter has no enabled
		// Spawn/Lifetime module or zero max particles. Imported recipes hit
		// that often, so the failed slot is kept as nullptr: everything else
		// pairs simulators with emitters by index, and dropping a slot would
		// silently draw one emitter with another emitter's texture.
		auto pSimulator = make_unique<CEffect_ParticleSimulator>();
		if (!pSimulator->Initialize(Emitter,
			static_cast<uint32_t>(Emitter.iEmitterId)))
			pSimulator.reset();
		m_Simulators.push_back(move(pSimulator));
	}
	m_iSelectedEmitter = min(m_iSelectedEmitter,
		max(0, static_cast<int32_t>(m_Asset.Emitters.size()) - 1));
	m_iSelectedModule = 0;
	m_fPreviewTime = 0.f;
	m_isWorldPreviewDirty = true;
}

void Client::CEffect_Tool::Refresh_NextElementId()
{
	uint64_t iHighest = {};
	for (const EFFECT_EMITTER_DESC& Emitter : m_Asset.Emitters)
	{
		iHighest = max(iHighest, Emitter.iEmitterId);
		for (const EFFECT_MODULE_DESC& Module : Emitter.Modules)
			iHighest = max(iHighest, Module.iModuleId);
	}
	m_iNextElementId = iHighest + 1;
}

void Client::CEffect_Tool::Adopt_LoadedAsset(EFFECT_ASSET_DESC&& Loaded)
{
	m_Asset = move(Loaded);
	// Order matters: the counter has to be correct before anything can add a
	// new emitter or module on top of the loaded ids.
	Refresh_NextElementId();
	Rebuild_Simulators();
	m_strSaveAsId = m_Asset.strAssetId;
	// Imported assets span very different sizes, so start at a zoom that shows
	// the whole effect instead of leaving it a few pixels wide.
	m_fPreviewZoom = Estimate_PreviewZoom();
}

filesystem::path Client::CEffect_Tool::Resolve_AuthoredPath(
	const string& strAssetId) const
{
	const u8string Utf8Id(strAssetId.begin(), strAssetId.end());
	filesystem::path RelativePath(Utf8Id);
	RelativePath += L".effect";
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Effects/Authored") /
		RelativePath);
}

void Client::CEffect_Tool::Capture_SceneHDR_Readback()
{
	if (!m_isHDRReadbackRequested || m_isHDRReadbackDone ||
		nullptr == m_pDevice)
		return;

	/*
	 * 월드 프리뷰는 등록 시점 때문에 한 프레임 뒤에 그려진다. 첫 프레임을
	 * 읽으면 빈 타깃이 나오므로 몇 프레임 지난 뒤에 한 번만 읽는다.
	 */
	/*
	 * 레벨 전환 전에는 SceneHDR이 비어 있으므로 한 번 쏘고 끝내지 않는다.
	 * 1을 넘는 값을 실제로 볼 때까지 주기적으로 다시 읽는다.
	 */
	// Loading and Logo do not provide the camera used by the world preview.
	// Start the bounded wait only after the automated run reaches TEST_LEVEL2.
	if (ETOUI(LEVEL::DEVELOPMENT) !=
		CGameInstance::Get().Get_CurrentLevelID())
		return;

	constexpr uint32_t READBACK_INTERVAL = 30;
	constexpr uint32_t READBACK_FRAME_LIMIT = 900;
	constexpr uint64_t MIN_MATCHED_PIXELS = 16;
	const uint32_t iReadbackFrame = ++m_iHDRReadbackFrame;
	const bool_t isTimedOut = iReadbackFrame >= READBACK_FRAME_LIMIT;
	if (!isTimedOut && 0 != iReadbackFrame % READBACK_INTERVAL)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const uint32_t iWidth = static_cast<uint32_t>(vViewportSize.x);
	const uint32_t iHeight = static_cast<uint32_t>(vViewportSize.y);
	f32_t fThirdMax[3]{};
	f32_t fFrameMax = {};
	const f32_t fExpected[3] = { 1.f, 5.f, 25.f };
	const f32_t fTolerance[3] = { 0.25f, 0.75f, 3.75f };
	f32_t fClosest[3]{};
	f32_t fClosestDifference[3] = { 65504.f, 65504.f, 65504.f };
	uint64_t iMatchedPixels[3]{};
	uint64_t iFiniteScenePixels = {};
	f32_t fBloomMax = {};
	uint64_t iBloomNonZeroPixels = {};
	string strBloomStatus = "BLOOM_NOT_MEASURED";
	string strBloomReason;
	f32_t fDistortionMax = {};
	uint64_t iDistortionNonZeroPixels = {};
	string strDistortionStatus = "DISTORTION_NOT_MEASURED";
	string strDistortionReason;

	auto WriteReport = [&](const string& strResult,
		const string& strReason, const bool_t isFinal)
	{
		error_code Error;
		const filesystem::path TempRoot =
			filesystem::temp_directory_path(Error);
		filesystem::path ReportPath;
		if (!Error)
		{
			ReportPath = TempRoot /
				"lostark_effect_roundtrip" / "hdr_readback.txt";
			filesystem::create_directories(
				ReportPath.parent_path(), Error);
		}

		ofstream Report(ReportPath, ios::trunc);
		if (Report)
		{
			Report << "asset = " << m_Asset.strAssetId << "\n";
			Report << "frame = " << iReadbackFrame << " / "
				<< READBACK_FRAME_LIMIT << "\n";
			Report << "target = Target_SceneHDR "
				"(R16G16B16A16_FLOAT)\n";
			Report << "viewport = " << iWidth << " x " << iHeight << "\n";
			Report << "finite scene pixels = " << iFiniteScenePixels << "\n";
			Report << "frame max = " << fFrameMax << "\n";
			const char* pThirdNames[3] = { "left", "middle", "right" };
			for (uint32_t i = 0; i < 3; ++i)
			{
				Report << pThirdNames[i]
					<< " expected=" << fExpected[i]
					<< " tolerance=" << fTolerance[i]
					<< " closest=" << fClosest[i]
					<< " max=" << fThirdMax[i]
					<< " matched_pixels=" << iMatchedPixels[i]
					<< "\n";
			}
			Report << "bloom target = Target_BloomResult "
				"(R11G11B10_FLOAT)\n";
			Report << "bloom max = " << fBloomMax << "\n";
			Report << "bloom nonzero pixels = "
				<< iBloomNonZeroPixels << "\n";
			Report << "BLOOM_RESULT = " << strBloomStatus << "\n";
			if (!strBloomReason.empty())
				Report << "bloom reason = " << strBloomReason << "\n";
			Report << "distortion target = Target_Distortion "
				"(R16G16B16A16_FLOAT)\n";
			Report << "distortion abs max = " << fDistortionMax << "\n";
			Report << "distortion nonzero pixels = "
				<< iDistortionNonZeroPixels << "\n";
			Report << "DISTORTION_RESULT = "
				<< strDistortionStatus << "\n";
			if (!strDistortionReason.empty())
				Report << "distortion reason = "
					<< strDistortionReason << "\n";
			Report << "RESULT = " << strResult << "\n";
			Report << "reason = " << strReason << "\n";
		}

		if (!isFinal)
			return;

		m_isHDRReadbackDone = true;
		m_strFileStatus = ReportPath.empty()
			? "SceneHDR readback finished; report path unavailable"
			: "SceneHDR readback written: " + ReportPath.string();
	};

	if (0 == iWidth || 0 == iHeight)
	{
		WriteReport(
			isTimedOut ? "NO_GEOMETRY" : "WAITING",
			"viewport size is zero", isTimedOut);
		return;
	}

	auto EnsureStagingTexture = [this](
		ComPtr<ID3D11Texture2D>& pTexture,
		const uint32_t iTextureWidth,
		const uint32_t iTextureHeight,
		const DXGI_FORMAT eFormat)
	{
		if (nullptr != pTexture.Get())
		{
			D3D11_TEXTURE2D_DESC ExistingDesc{};
			pTexture->GetDesc(&ExistingDesc);
			if (ExistingDesc.Width == iTextureWidth &&
				ExistingDesc.Height == iTextureHeight &&
				ExistingDesc.Format == eFormat)
			{
				return true;
			}
			pTexture.Reset();
		}

		D3D11_TEXTURE2D_DESC Desc{};
		Desc.Width = iTextureWidth;
		Desc.Height = iTextureHeight;
		Desc.MipLevels = 1;
		Desc.ArraySize = 1;
		Desc.Format = eFormat;
		Desc.SampleDesc.Count = 1;
		Desc.Usage = D3D11_USAGE_STAGING;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		return SUCCEEDED(m_pDevice->CreateTexture2D(
			&Desc, nullptr, pTexture.GetAddressOf()));
	};

	if (!EnsureStagingTexture(
		m_pSceneHDRStaging, iWidth, iHeight,
		DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		WriteReport("READBACK_ERROR",
			"failed to create SceneHDR staging texture", true);
		return;
	}

	if (FAILED(CGameInstance::Get().Copy_RT_Resource(
		TEXT("Target_SceneHDR"), m_pSceneHDRStaging)))
	{
		WriteReport(
			isTimedOut ? "READBACK_ERROR" : "WAITING",
			"failed to copy Target_SceneHDR", isTimedOut);
		return;
	}

	ComPtr<ID3D11DeviceContext> pContext;
	m_pDevice->GetImmediateContext(&pContext);
	if (nullptr == pContext.Get())
	{
		WriteReport("READBACK_ERROR",
			"immediate device context is unavailable", true);
		return;
	}

	D3D11_MAPPED_SUBRESOURCE Mapped{};
	if (FAILED(pContext->Map(
		m_pSceneHDRStaging.Get(), 0, D3D11_MAP_READ, 0, &Mapped)))
	{
		WriteReport(
			isTimedOut ? "READBACK_ERROR" : "WAITING",
			"failed to map Target_SceneHDR staging texture", isTimedOut);
		return;
	}

	const auto* pBytes = static_cast<const uint8_t*>(Mapped.pData);
	for (uint32_t iY = 0; iY < iHeight; ++iY)
	{
		const auto* pRow = reinterpret_cast<const uint16_t*>(
			pBytes + static_cast<size_t>(iY) * Mapped.RowPitch);
		for (uint32_t iX = 0; iX < iWidth; ++iX)
		{
			using DirectX::PackedVector::XMConvertHalfToFloat;
			const f32_t fR = XMConvertHalfToFloat(pRow[iX * 4 + 0]);
			const f32_t fG = XMConvertHalfToFloat(pRow[iX * 4 + 1]);
			const f32_t fB = XMConvertHalfToFloat(pRow[iX * 4 + 2]);
			if (!isfinite(fR) || !isfinite(fG) || !isfinite(fB))
				continue;

			++iFiniteScenePixels;
			const f32_t fPixelMax = max(fR, max(fG, fB));
			const f32_t fPixelMin = min(fR, min(fG, fB));
			const f32_t fSignal = (fR + fG + fB) / 3.f;
			fFrameMax = max(fFrameMax, fPixelMax);
			const uint32_t iThird = min(2u, iX * 3u / iWidth);
			fThirdMax[iThird] = max(fThirdMax[iThird], fPixelMax);
			const f32_t fDifference =
				fabsf(fSignal - fExpected[iThird]);
			if (fDifference < fClosestDifference[iThird])
			{
				fClosestDifference[iThird] = fDifference;
				fClosest[iThird] = fSignal;
			}
			if (fDifference <= fTolerance[iThird] &&
				fPixelMax - fPixelMin <= fTolerance[iThird])
			{
				++iMatchedPixels[iThird];
			}
		}
	}
	pContext->Unmap(m_pSceneHDRStaging.Get(), 0);

	const uint32_t iBloomWidth = max(1u, iWidth / 2u);
	const uint32_t iBloomHeight = max(1u, iHeight / 2u);
	if (!EnsureStagingTexture(
		m_pBloomResultStaging, iBloomWidth, iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT))
	{
		strBloomReason = "failed to create bloom staging texture";
	}
	else if (FAILED(CGameInstance::Get().Copy_RT_Resource(
		TEXT("Target_BloomResult"), m_pBloomResultStaging)))
	{
		strBloomReason = "failed to copy Target_BloomResult";
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE BloomMapped{};
		if (FAILED(pContext->Map(
			m_pBloomResultStaging.Get(), 0,
			D3D11_MAP_READ, 0, &BloomMapped)))
		{
			strBloomReason =
				"failed to map Target_BloomResult staging texture";
		}
		else
		{
			const auto* pBloomBytes =
				static_cast<const uint8_t*>(BloomMapped.pData);
			for (uint32_t iY = 0; iY < iBloomHeight; ++iY)
			{
				const auto* pRow = reinterpret_cast<
					const DirectX::PackedVector::XMFLOAT3PK*>(
					pBloomBytes +
					static_cast<size_t>(iY) * BloomMapped.RowPitch);
				for (uint32_t iX = 0; iX < iBloomWidth; ++iX)
				{
					DirectX::XMFLOAT3 vBloom{};
					DirectX::XMStoreFloat3(
						&vBloom,
						DirectX::PackedVector::XMLoadFloat3PK(
							&pRow[iX]));
					if (!isfinite(vBloom.x) ||
						!isfinite(vBloom.y) ||
						!isfinite(vBloom.z))
					{
						continue;
					}
					const f32_t fPixelMax = max(
						vBloom.x, max(vBloom.y, vBloom.z));
					fBloomMax = max(fBloomMax, fPixelMax);
					if (0.000001f < fPixelMax)
						++iBloomNonZeroPixels;
				}
			}
			pContext->Unmap(m_pBloomResultStaging.Get(), 0);
			strBloomStatus = 0 < iBloomNonZeroPixels
				? "BLOOM_OK" : "BLOOM_ZERO";
			strBloomReason = 0 < iBloomNonZeroPixels
				? "bright-pass and blur produced non-zero energy"
				: "bloom result contains no non-zero pixels";
		}
	}

	if (!EnsureStagingTexture(
		m_pDistortionStaging, iWidth, iHeight,
		DXGI_FORMAT_R16G16B16A16_FLOAT))
	{
		strDistortionReason =
			"failed to create distortion staging texture";
	}
	else if (FAILED(CGameInstance::Get().Copy_RT_Resource(
		TEXT("Target_Distortion"), m_pDistortionStaging)))
	{
		strDistortionReason = "failed to copy Target_Distortion";
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE DistortionMapped{};
		if (FAILED(pContext->Map(
			m_pDistortionStaging.Get(), 0,
			D3D11_MAP_READ, 0, &DistortionMapped)))
		{
			strDistortionReason =
				"failed to map Target_Distortion staging texture";
		}
		else
		{
			const auto* pDistortionBytes =
				static_cast<const uint8_t*>(DistortionMapped.pData);
			for (uint32_t iY = 0; iY < iHeight; ++iY)
			{
				const auto* pRow = reinterpret_cast<const uint16_t*>(
					pDistortionBytes +
					static_cast<size_t>(iY) * DistortionMapped.RowPitch);
				for (uint32_t iX = 0; iX < iWidth; ++iX)
				{
					using DirectX::PackedVector::XMConvertHalfToFloat;
					const f32_t fX = XMConvertHalfToFloat(
						pRow[iX * 4 + 0]);
					const f32_t fY = XMConvertHalfToFloat(
						pRow[iX * 4 + 1]);
					if (!isfinite(fX) || !isfinite(fY))
						continue;
					const f32_t fAbsMax = max(fabsf(fX), fabsf(fY));
					fDistortionMax = max(fDistortionMax, fAbsMax);
					if (0.000001f < fAbsMax)
						++iDistortionNonZeroPixels;
				}
			}
			pContext->Unmap(m_pDistortionStaging.Get(), 0);
			strDistortionStatus = 0 < iDistortionNonZeroPixels
				? "DISTORTION_OK" : "DISTORTION_ZERO";
			strDistortionReason = 0 < iDistortionNonZeroPixels
				? "effect shader wrote signed UV offsets"
				: "distortion target contains no non-zero pixels";
		}
	}

	const bool_t hasAllReferenceLevels =
		iMatchedPixels[0] >= MIN_MATCHED_PIXELS &&
		iMatchedPixels[1] >= MIN_MATCHED_PIXELS &&
		iMatchedPixels[2] >= MIN_MATCHED_PIXELS;
	const bool_t hasBloomOutput = 0 < iBloomNonZeroPixels;
	const bool_t hasDistortionOutput = 0 < iDistortionNonZeroPixels;
	if (hasAllReferenceLevels && 1.f < fFrameMax &&
		hasBloomOutput && hasDistortionOutput)
	{
		WriteReport("HDR_OK",
			"SceneHDR preserved 1, 5, and 25; bloom and distortion "
			"targets both contain non-zero output", true);
		return;
	}

	if (!isTimedOut)
	{
		string strWaitReason;
		if (0.f >= fFrameMax)
			strWaitReason = "waiting for scene geometry and effect render";
		else if (1.f >= fFrameMax)
			strWaitReason =
				"scene is visible but no HDR value above 1 is present yet";
		else if (!hasAllReferenceLevels)
			strWaitReason =
				"HDR is present but one or more reference levels are missing";
		else if (!hasBloomOutput)
			strWaitReason = "waiting for non-zero bloom output";
		else
			strWaitReason = "waiting for non-zero distortion output";
		WriteReport("WAITING", strWaitReason, false);
		return;
	}

	if (0.f >= fFrameMax)
	{
		WriteReport("NO_GEOMETRY",
			"frame limit reached and Target_SceneHDR max is zero", true);
	}
	else if (1.f >= fFrameMax)
	{
		WriteReport("CLAMPED",
			"frame limit reached and nothing above 1 was stored", true);
	}
	else if (!hasAllReferenceLevels)
	{
		WriteReport("TIMEOUT_MISSING_REFERENCE_LEVELS",
			"frame limit reached before all 1, 5, and 25 levels matched",
			true);
	}
	else if (!hasBloomOutput)
	{
		WriteReport("BLOOM_FAILED",
			strBloomReason.empty()
			? "frame limit reached with zero bloom output"
			: strBloomReason,
			true);
	}
	else if (!hasDistortionOutput)
	{
		WriteReport("DISTORTION_FAILED",
			strDistortionReason.empty()
			? "frame limit reached with zero distortion output"
			: strDistortionReason,
			true);
	}
	else
	{
		WriteReport("READBACK_ERROR",
			"unexpected HDR validation state", true);
	}
}

void Client::CEffect_Tool::Refresh_AuthoredList()
{
	m_AuthoredAssets.clear();
	m_iSelectedAuthoredAsset = -1;

	error_code Error;
	const filesystem::path Root = CProjectDataRoot::Resolve(
		L"Effects/Authored");
	if (!filesystem::exists(Root, Error))
		return;

	// Recursive so a skill can own a folder: the converter writes one folder
	// per skill family and the id keeps the folder, e.g. "talonstrike/par_...".
	for (const filesystem::directory_entry& Entry :
		filesystem::recursive_directory_iterator(Root, Error))
	{
		if (!Entry.is_regular_file() || ".effect" != Entry.path().extension())
			continue;
		filesystem::path Relative =
			filesystem::relative(Entry.path(), Root, Error);
		if (Error)
			continue;
		Relative.replace_extension();
		string strId = Relative.generic_string();
		if (!strId.empty())
			m_AuthoredAssets.push_back(move(strId));
	}
	sort(m_AuthoredAssets.begin(), m_AuthoredAssets.end());
}

f32_t Client::CEffect_Tool::Estimate_PreviewZoom() const
{
	// Half of a typical canvas. The zoom only has to land in the right order
	// of magnitude; the slider covers the rest.
	constexpr f32_t TARGET_HALF_PIXELS = 180.f;
	f32_t fExtent = 0.01f;

	for (const EFFECT_EMITTER_DESC& Emitter : m_Asset.Emitters)
	{
		if (!Emitter.isEnabled)
			continue;

		f32_t fLifetime = 1.f;
		for (const EFFECT_MODULE_DESC& Module : Emitter.Modules)
		{
			if (EFFECT_MODULE_TYPE::LIFETIME == Module.eType)
			{
				fLifetime = max(fLifetime,
					max(Module.Lifetime.Lifetime.fConstant,
						Module.Lifetime.Lifetime.fMax));
			}
		}

		for (const EFFECT_MODULE_DESC& Module : Emitter.Modules)
		{
			if (!Module.isEnabled)
				continue;

			switch (Module.eType)
			{
			case EFFECT_MODULE_TYPE::INITIAL_LOCATION:
			{
				const float3_t& vMax = Module.InitialLocation.Location.vMax;
				fExtent = max(fExtent, max(fabsf(vMax.x),
					max(fabsf(vMax.y), fabsf(vMax.z))));
				if (EFFECT_LOCATION_SHAPE::BOX !=
					Module.InitialLocation.eShape)
				{
					fExtent = max(fExtent,
						max(Module.InitialLocation.fRadius,
							Module.InitialLocation.fHeight * 0.5f));
				}
				break;
			}
			case EFFECT_MODULE_TYPE::INITIAL_SIZE:
			{
				const float3_t& vMax = Module.InitialSize.Size.vMax;
				fExtent = max(fExtent, max(fabsf(vMax.x), fabsf(vMax.y)));
				break;
			}
			case EFFECT_MODULE_TYPE::INITIAL_VELOCITY:
			{
				const float3_t& vMax = Module.InitialVelocity.Velocity.vMax;
				const f32_t fSpeed = max(fabsf(vMax.x),
					max(fabsf(vMax.y), fabsf(vMax.z)));
				fExtent = max(fExtent, fSpeed * fLifetime);
				break;
			}
			}
		}
	}
	return min(4000.f, max(1.f, TARGET_HALF_PIXELS / fExtent));
}

void Client::CEffect_Tool::Restart_Preview()
{
	m_fPreviewTime = 0.f;
	for (const auto& pSimulator : m_Simulators)
		if (nullptr != pSimulator)
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
	ImGui::Text("%.2f / %.2f sec",
		m_fPreviewTime, CalculatePreviewDuration(m_Asset));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(90.f);
	ImGui::DragFloat("LOD Distance", &m_fPreviewDistance,
		1.f, 0.f, 10000.f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(110.f);
	ImGui::DragFloat("Zoom px/m", &m_fPreviewZoom, 5.f, 1.f, 4000.f,
		"%.0f", ImGuiSliderFlags_Logarithmic);
	ImGui::SameLine();
	if (ImGui::Button("Fit"))
		m_fPreviewZoom = Estimate_PreviewZoom();
	ImGui::SameLine();
	if (ImGui::Button("Validate Scene HDR"))
	{
		m_iHDRReadbackFrame = 0;
		m_isHDRReadbackDone = false;
		m_isHDRReadbackRequested = true;
		m_pSceneHDRStaging.Reset();
		m_pBloomResultStaging.Reset();
		m_pDistortionStaging.Reset();
		m_strFileStatus = "SceneHDR validation requested.";
	}
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

	if (ImGui::Button("Save"))
	{
		string strError;
		m_strFileStatus = CEffect_AssetIO::Save_Authoring(
			m_strAuthoringPath, m_Asset, &strError)
			? "Saved to working.effect" : strError;
	}
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{
		string strError;
		EFFECT_ASSET_DESC Loaded;
		if (CEffect_AssetIO::Load_Authoring(
			m_strAuthoringPath, Loaded, &strError))
		{
			Adopt_LoadedAsset(move(Loaded));
			m_strFileStatus = "Loaded working.effect";
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
			Adopt_LoadedAsset(move(Loaded));
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

	// Save As / Open by asset id. Each asset gets its own file so building
	// several skills no longer means overwriting the single working file.
	ImGui::SetNextItemWidth(240.f);
	EditString("Asset Id", m_strSaveAsId);
	ImGui::SameLine();
	if (ImGui::Button("Save As"))
	{
		if (m_strSaveAsId.empty())
			m_strFileStatus = "Asset Id is empty.";
		else
		{
			string strError;
			if (CEffect_AssetIO::Save_Authoring(
				Resolve_AuthoredPath(m_strSaveAsId), m_Asset, &strError))
			{
				m_strFileStatus = "Saved " + m_strSaveAsId + ".effect";
				Refresh_AuthoredList();
			}
			else
				m_strFileStatus = strError;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh List"))
		Refresh_AuthoredList();

	ImGui::SetNextItemWidth(240.f);
	if (ImGui::BeginListBox("Authored", ImVec2(240.f, 68.f)))
	{
		for (int32_t i = 0;
			i < static_cast<int32_t>(m_AuthoredAssets.size()); ++i)
		{
			const bool_t isSelected = (i == m_iSelectedAuthoredAsset);
			if (ImGui::Selectable(m_AuthoredAssets[i].c_str(), isSelected))
				m_iSelectedAuthoredAsset = i;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}
	ImGui::SameLine();
	if (ImGui::Button("Open Selected") &&
		m_iSelectedAuthoredAsset >= 0 &&
		m_iSelectedAuthoredAsset <
			static_cast<int32_t>(m_AuthoredAssets.size()))
	{
		const string& strAssetId =
			m_AuthoredAssets[m_iSelectedAuthoredAsset];
		string strError;
		EFFECT_ASSET_DESC Loaded;
		if (CEffect_AssetIO::Load_Authoring(
			Resolve_AuthoredPath(strAssetId), Loaded, &strError))
		{
			Adopt_LoadedAsset(move(Loaded));
			m_strFileStatus = "Opened " + strAssetId + ".effect";
		}
		else
			m_strFileStatus = strError;
	}
}

void Client::CEffect_Tool::Reload_SourceCatalog()
{
	const filesystem::path Candidates[] = {
		CProjectDataRoot::Resolve(
			L"Effects/SourceCatalog/particle_systems.csv")
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
	{
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
		{
			// Cascade ScreenAlignment. Square forces one size on both axes,
			// Rectangle keeps X/Y, Velocity turns the sprite along its motion.
			static const char* AlignmentNames[] = {
				"Square", "Rectangle", "Velocity"
			};
			int32_t iAlignment = static_cast<int32_t>(
				Module.Required.eScreenAlignment);
			if (ImGui::Combo("Screen Alignment", &iAlignment,
				AlignmentNames, IM_ARRAYSIZE(AlignmentNames)))
			{
				Module.Required.eScreenAlignment =
					static_cast<EFFECT_SCREEN_ALIGNMENT>(iAlignment);
				m_isWorldPreviewDirty = true;
			}
		}
		if (ImGui::Button("Reload Resource"))
			Reload_TextureResource(Emitter);
		ImGui::SameLine();
		ImGui::TextWrapped("%s",
			Get_TextureResourceStatus(Emitter).c_str());

		ImGui::SeparatorText("Effect Material");
		bool_t isMaterialChanged = false;
		auto EditMaterialString = [&isMaterialChanged](
			const char* pLabel, string& strValue)
		{
			const string strBefore = strValue;
			EditString(pLabel, strValue);
			isMaterialChanged |= strBefore != strValue;
		};
		EFFECT_MATERIAL_DESC& Material = Module.Required.Material;

		EditMaterialString("Opacity Texture",
			Material.strOpacityTextureAssetId);
		ImGui::SameLine();
		if (ImGui::Button("Browse Opacity") &&
			BrowseResourceFile(
				L"Effect Textures\0*.dds;*.png;*.jpg;*.jpeg;*.bmp;"
					L"*.tif;*.tiff\0All Files\0*.*\0\0",
				Material.strOpacityTextureAssetId))
		{
			isMaterialChanged = true;
		}

		EditMaterialString("Dissolve Texture",
			Material.strDissolveTextureAssetId);
		ImGui::SameLine();
		if (ImGui::Button("Browse Dissolve") &&
			BrowseResourceFile(
				L"Effect Textures\0*.dds;*.png;*.jpg;*.jpeg;*.bmp;"
					L"*.tif;*.tiff\0All Files\0*.*\0\0",
				Material.strDissolveTextureAssetId))
		{
			isMaterialChanged = true;
		}

		EditMaterialString("Distortion Texture",
			Material.strDistortionTextureAssetId);
		ImGui::SameLine();
		if (ImGui::Button("Browse Distortion") &&
			BrowseResourceFile(
				L"Effect Textures\0*.dds;*.png;*.jpg;*.jpeg;*.bmp;"
					L"*.tif;*.tiff\0All Files\0*.*\0\0",
				Material.strDistortionTextureAssetId))
		{
			isMaterialChanged = true;
		}

		isMaterialChanged |= ImGui::DragFloat2("UV Tiling",
			reinterpret_cast<float*>(&Material.vUVTiling),
			0.01f, -100.f, 100.f);
		isMaterialChanged |= ImGui::DragFloat2("UV Offset",
			reinterpret_cast<float*>(&Material.vUVOffset), 0.01f);
		isMaterialChanged |= ImGui::DragFloat2("UV Panner / sec",
			reinterpret_cast<float*>(&Material.vUVPanner), 0.01f);
		isMaterialChanged |= ImGui::DragFloat("Emissive Strength",
			&Material.fEmissiveStrength, 0.05f, 0.f, 10000.f);
		isMaterialChanged |= ImGui::DragFloat("Opacity Threshold",
			&Material.fOpacityMaskThreshold, 0.005f, 0.f, 1.f);
		isMaterialChanged |= ImGui::DragFloat("Dissolve Amount",
			&Material.fDissolveAmount, 0.005f, 0.f, 1.f);
		isMaterialChanged |= ImGui::DragFloat("Dissolve Edge Width",
			&Material.fDissolveEdgeWidth, 0.0025f, 0.0001f, 1.f);
		isMaterialChanged |= ImGui::ColorEdit4("Dissolve Edge Color",
			reinterpret_cast<float*>(&Material.vDissolveEdgeColor),
			ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
		isMaterialChanged |= ImGui::DragFloat("Soft Particle Distance",
			&Material.fSoftParticleDistance, 0.01f, 0.f, 1000.f);
		isMaterialChanged |= ImGui::DragFloat("Distortion Strength (UV)",
			&Material.fDistortionStrength, 0.0001f, -1.f, 1.f,
			"%.4f");
		if (isMaterialChanged)
			m_isWorldPreviewDirty = true;
		break;
	}
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
	{
		static const char* ShapeNames[] = { "Box", "Sphere", "Cylinder" };
		int32_t iShape =
			static_cast<int32_t>(Module.InitialLocation.eShape);
		if (ImGui::Combo("Shape", &iShape,
			ShapeNames, IM_ARRAYSIZE(ShapeNames)))
		{
			Module.InitialLocation.eShape =
				static_cast<EFFECT_LOCATION_SHAPE>(iShape);
			m_isWorldPreviewDirty = true;
		}
		ImGui::DragFloat3("Location Min",
			reinterpret_cast<float*>(&Module.InitialLocation.Location.vMin), 0.1f);
		ImGui::DragFloat3("Location Max",
			reinterpret_cast<float*>(&Module.InitialLocation.Location.vMax), 0.1f);
		if (EFFECT_LOCATION_SHAPE::BOX != Module.InitialLocation.eShape)
		{
			ImGui::DragFloat("Radius",
				&Module.InitialLocation.fRadius, 0.05f, 0.f, 10000.f);
			ImGui::DragFloat("Inner Radius",
				&Module.InitialLocation.fInnerRadius, 0.05f, 0.f, 10000.f);
			if (EFFECT_LOCATION_SHAPE::CYLINDER ==
				Module.InitialLocation.eShape)
			{
				ImGui::DragFloat("Height",
					&Module.InitialLocation.fHeight, 0.05f, 0.f, 10000.f);
			}
			ImGui::Checkbox("Surface Only",
				&Module.InitialLocation.isSurfaceOnly);
			ImGui::TextDisabled(
				"Min/Max is the spawn offset when a shape is used.");
		}
		break;
	}
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
			reinterpret_cast<float*>(&Module.InitialColor.Color.vMin),
			ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Color Max",
			reinterpret_cast<float*>(&Module.InitialColor.Color.vMax),
			ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
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
	case EFFECT_MODULE_TYPE::DYNAMIC_PARAMETER:
		ImGui::TextUnformatted("X: Emissive multiplier");
		ImGui::PushID("DynamicParameterX");
		EditCurve(Module.DynamicParameter.X);
		ImGui::PopID();
		ImGui::TextUnformatted("Y: Opacity multiplier");
		ImGui::PushID("DynamicParameterY");
		EditCurve(Module.DynamicParameter.Y);
		ImGui::PopID();
		ImGui::TextUnformatted("Z: Dissolve offset");
		ImGui::PushID("DynamicParameterZ");
		EditCurve(Module.DynamicParameter.Z);
		ImGui::PopID();
		ImGui::TextUnformatted("W: Distortion multiplier");
		ImGui::PushID("DynamicParameterW");
		EditCurve(Module.DynamicParameter.W);
		ImGui::PopID();
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
		if (nullptr == pSimulator)
			continue;
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
	Draw_Particles(pDrawList, vOrigin, vSize, m_fPreviewZoom);
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
		if (!Emitter.isEnabled || nullptr == m_Simulators[iEmitter])
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

	const filesystem::path TexturePath =
		ResolveToolAssetPath(strAssetId);
	if (TexturePath.empty())
	{
		Resource.strStatus =
			"Texture load failed: file not found: " + strAssetId;
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
			m_pDevice.Get(), TexturePath.c_str(), nullptr,
			Resource.pShaderResourceView.ReleaseAndGetAddressOf());
	}
	else
	{
		hResult = DirectX::CreateWICTextureFromFile(
			m_pDevice.Get(), TexturePath.c_str(), nullptr,
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
