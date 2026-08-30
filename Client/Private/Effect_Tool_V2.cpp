#include "imgui.h"

#include "Effect_Tool_V2.h"
#include "ActorCatalog.h"
#include "AnimationTargetService.h"
#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "Valtan.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPresentationAssetService.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>

namespace
{
	constexpr const wchar_t* EFFECT_V2_PREVIEW_LAYER_TAG =
		L"Layer_EffectPreviewV2";

	constexpr uint32_t MAX_PREVIEW_LOADS_PER_FRAME = 2u;
	constexpr size_t MAX_PREVIEW_DIMENSION = 512u;
	constexpr uint32_t MODEL_THUMBNAIL_SIZE = 128u;
	constexpr float SLOT_CARD_SIZE = 96.f;
	constexpr float BROWSER_TILE_SIZE = 80.f;
	constexpr float PREVIEW_PANEL_SIZE = 256.f;

	constexpr const char* ASSET_KIND_FOLDERS[] = {
		"meshes", "textures", "models", "animations", "vectorfields"
	};

	std::string To_Lower(std::string Value)
	{
		std::transform(Value.begin(), Value.end(), Value.begin(),
			[](const char Character)
			{
				return static_cast<char>(std::tolower(
					static_cast<unsigned char>(Character)));
			});
		return Value;
	}

	bool_t Is_AssetKindFolder(const std::string& strName)
	{
		const std::string Lower = To_Lower(strName);
		for (const char* pFolder : ASSET_KIND_FOLDERS)
		{
			if (Lower == pFolder)
				return true;
		}
		return false;
	}
}

Client::CEffect_Tool_V2::CEffect_Tool_V2(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffect_Tool_V2::~CEffect_Tool_V2()
{
	Deactivate();
}

void Client::CEffect_Tool_V2::Deactivate()
{
	Stop_ValtanTimeline();
	m_bTestOrbit = false;
	if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
	{
		pPreview->Clear_FollowTarget();
		pPreview->Set_Hidden(true);
		pPreview->Finish();
		CGameInstance::Get().Remove_GameObject_from_Layer(
			CGameInstance::Get().Get_CurrentLevelID(),
			EFFECT_V2_PREVIEW_LAYER_TAG, pPreview);
	}
	m_pPreview.reset();
	Despawn_Target();
	m_fTargetLastClipSeconds = -1.f;
	m_strPreviewStatus =
		"Effect Tool v2 hidden: active preview/playback released; authored draft preserved.";
}

void Client::CEffect_Tool_V2::On_LevelChanged()
{
	Deactivate();
	m_Target.Reset();
	m_TargetBoneNames.clear();
	m_strAttachStatus =
		"Level changed: spawn a new preview target in the active Level.";
}

void Client::CEffect_Tool_V2::Render()
{
	Update_ValtanServerPatternStatus();
	m_iLoadsThisFrame = 0u;
	if (!m_bScanned)
		Scan_Resources();
	if (!Slot_VisibleForType(m_eSelectedSlot))
		m_eSelectedSlot = RESOURCE_SLOT::BASE;

	if (!ImGui::Begin("Effect Tool v2"))
	{
		ImGui::End();
		return;
	}

	Render_TypeSelector();
	ImGui::Separator();
	Render_SlotCards();
	ImGui::Separator();

	if (ImGui::BeginTable("EffectToolV2Body", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Resources",
			ImGuiTableColumnFlags_WidthStretch, 0.62f);
		ImGui::TableSetupColumn("Preview",
			ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_ResourceBrowser();
		ImGui::TableSetColumnIndex(1);
		Render_PreviewPanel();
		ImGui::Separator();
		Render_CreatePanel();
		ImGui::Separator();
		Render_DocumentPanel();
		ImGui::EndTable();
	}

	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();

	Update_Attach(ImGui::GetIO().DeltaTime);
	Render_TuningPanel();
	Render_AttachWindow();
	Render_GroupWindow();
}

void Client::CEffect_Tool_V2::Scan_Resources()
{
	m_bScanned = true;
	m_Resources.clear();
	m_Domains.clear();
	m_bVisibleDirty = true;

	std::error_code Error;
	const std::filesystem::path Root = CRuntimeAssetRoot::Get();
	const std::filesystem::path EffectRoot = Root / "Effect";
	if (!std::filesystem::is_directory(EffectRoot, Error) || Error)
	{
		m_strStatus = "Resources/Effect is missing: " + EffectRoot.string();
		return;
	}

	std::set<std::string> Domains;
	size_t iTextures = 0u;
	size_t iModels = 0u;
	for (std::filesystem::recursive_directory_iterator Iterator(
		EffectRoot,
		std::filesystem::directory_options::skip_permission_denied,
		Error), End; Iterator != End; Iterator.increment(Error))
	{
		if (Error)
		{
			Error.clear();
			continue;
		}
		if (!Iterator->is_regular_file())
			continue;
		const std::string Extension =
			To_Lower(Iterator->path().extension().string());
		RESOURCE_KIND eKind = RESOURCE_KIND::END;
		if (".dds" == Extension)
			eKind = RESOURCE_KIND::TEXTURE;
		else if (".wmodel" == Extension)
			eKind = RESOURCE_KIND::MODEL;
		else
			continue;
		const std::filesystem::path EffectRelative =
			Iterator->path().lexically_relative(EffectRoot);
		if (EffectRelative.empty() || !EffectRelative.has_parent_path())
			continue;
		const std::string Domain = Domain_FromRelativePath(EffectRelative);
		if (Domain.empty())
			continue;
		Domains.insert(Domain);
		(RESOURCE_KIND::TEXTURE == eKind ? iTextures : iModels)++;
		m_Resources.push_back({
			Iterator->path().lexically_relative(Root).generic_string(),
			Domain,
			Iterator->path().filename().string(),
			eKind });
	}
	std::sort(m_Resources.begin(), m_Resources.end(),
		[](const RESOURCE_ENTRY& Left, const RESOURCE_ENTRY& Right)
		{
			return Left.strAssetId < Right.strAssetId;
		});
	m_Domains.assign(Domains.begin(), Domains.end());
	Load_TextureUsage();
	size_t iWithUsage = 0u;
	for (RESOURCE_ENTRY& Entry : m_Resources)
	{
		if (RESOURCE_KIND::TEXTURE != Entry.eKind)
			continue;
		const auto Iterator = m_TextureUsage.find(To_Lower(Entry.strFileName));
		Entry.pUsage = m_TextureUsage.end() == Iterator ? nullptr : &Iterator->second;
		iWithUsage += nullptr != Entry.pUsage ? 1u : 0u;
	}
	m_strStatus = "Scanned " + std::to_string(iTextures) + " DDS (" +
		std::to_string(iWithUsage) + " with slot usage), " +
		std::to_string(iModels) + " WModel in " +
		std::to_string(m_Domains.size()) + " domains. " + m_strUsageStatus;
}

void Client::CEffect_Tool_V2::Load_TextureUsage()
{
	m_TextureUsage.clear();
	const std::filesystem::path Path =
		CProjectDataRoot::Resolve(L"Effects/V2/TextureSlotUsage.v1.json");
	std::ifstream File(Path, std::ios::binary);
	if (!File)
	{
		m_strUsageStatus = "TextureSlotUsage.v1.json not found.";
		return;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(File), std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE Root;
	std::string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		m_strUsageStatus = "TextureSlotUsage parse failed: " + Error;
		return;
	}
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pTextures = Root.Find("textures");
	if (nullptr == pVersion || !pVersion->Is_Number() ||
		1 != static_cast<int32_t>(pVersion->Get_Number()) ||
		nullptr == pTextures || !pTextures->Is_Object())
	{
		m_strUsageStatus = "TextureSlotUsage has an unsupported format.";
		return;
	}
	for (const std::string& Name : pTextures->Get_ObjectInsertionOrder())
	{
		const DATA_JSON_VALUE* pEntry = pTextures->Find(Name);
		if (nullptr == pEntry || !pEntry->Is_Object())
			continue;
		TEXTURE_USAGE Usage;
		for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
			iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
		{
			const DATA_JSON_VALUE* pCount =
				pEntry->Find(To_Lower(Slot_Label(static_cast<RESOURCE_SLOT>(iSlot))));
			if (nullptr != pCount && pCount->Is_Number())
				Usage.Counts[static_cast<size_t>(iSlot)] =
					static_cast<uint32_t>((std::max)(0.0, pCount->Get_Number()));
		}
		if (const DATA_JSON_VALUE* pParams = pEntry->Find("params");
			nullptr != pParams && pParams->Is_Array())
		{
			for (const DATA_JSON_VALUE& Param : pParams->Get_Array())
			{
				if (Param.Is_String())
					Usage.Params.push_back(Param.Get_String());
			}
		}
		m_TextureUsage.emplace(To_Lower(Name), std::move(Usage));
	}
	m_strUsageStatus = "Slot usage: " + std::to_string(m_TextureUsage.size()) +
		" textures.";
}

std::string Client::CEffect_Tool_V2::Domain_FromRelativePath(
	const std::filesystem::path& EffectRelative)
{
	std::string Domain;
	const std::filesystem::path Parent = EffectRelative.parent_path();
	for (const std::filesystem::path& Component : Parent)
	{
		const std::string Name = Component.string();
		if (Is_AssetKindFolder(Name))
			break;
		if (!Domain.empty())
			Domain += '/';
		Domain += Name;
	}
	if (Domain.empty() && Parent.begin() != Parent.end())
		Domain = Parent.begin()->string();
	return Domain;
}

void Client::CEffect_Tool_V2::Rebuild_VisibleResources()
{
	m_bVisibleDirty = false;
	m_VisibleResources.clear();
	const RESOURCE_KIND eKind = Slot_Kind(m_eSelectedSlot);
	m_eVisibleKind = eKind;
	const std::string Filter = To_Lower(m_szNameFilter);
	const bool_t bDedupeByName = m_strDomainFilter.empty();
	std::set<std::string> SeenNames;
	for (size_t iEntry = 0u; iEntry < m_Resources.size(); ++iEntry)
	{
		const RESOURCE_ENTRY& Entry = m_Resources[iEntry];
		if (Entry.eKind != eKind)
			continue;
		if (!m_strDomainFilter.empty() && Entry.strDomain != m_strDomainFilter)
			continue;
		const std::string LowerName = To_Lower(Entry.strFileName);
		if (!Filter.empty() && std::string::npos == LowerName.find(Filter))
			continue;
		if (bDedupeByName && !SeenNames.insert(LowerName).second)
			continue;
		m_VisibleResources.push_back(iEntry);
	}
}

const Client::CEffect_Tool_V2::PREVIEW_ENTRY*
Client::CEffect_Tool_V2::Request_Preview(
	const std::string& strAssetId, const RESOURCE_KIND eKind)
{
	if (strAssetId.empty())
		return nullptr;
	auto Iterator = m_Previews.find(strAssetId);
	if (Iterator != m_Previews.end())
		return &Iterator->second;
	if (m_iLoadsThisFrame >= MAX_PREVIEW_LOADS_PER_FRAME)
		return nullptr;
	m_iLoadsThisFrame += RESOURCE_KIND::MODEL == eKind ?
		MAX_PREVIEW_LOADS_PER_FRAME : 1u;

	PREVIEW_ENTRY Staged;
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	std::error_code Error;
	if (Path.empty() || !std::filesystem::is_regular_file(Path, Error))
	{
		Staged.strError = "Missing: " + strAssetId;
	}
	else if (RESOURCE_KIND::MODEL == eKind)
	{
		if (Create_ModelThumbnail(Path, Staged.pTextureView, Staged.strError,
			Staged.strInfo))
		{
			Staged.iWidth = MODEL_THUMBNAIL_SIZE;
			Staged.iHeight = MODEL_THUMBNAIL_SIZE;
		}
	}
	else
	{
		ComPtr<ID3D11Resource> pResource;
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(), Path.c_str(), &pResource,
			&Staged.pTextureView, MAX_PREVIEW_DIMENSION)))
		{
			Staged.strError = "DDS load failed: " + strAssetId;
		}
		else
		{
			ComPtr<ID3D11Texture2D> pTexture;
			if (SUCCEEDED(pResource.As(&pTexture)))
			{
				D3D11_TEXTURE2D_DESC Desc{};
				pTexture->GetDesc(&Desc);
				Staged.iWidth = Desc.Width;
				Staged.iHeight = Desc.Height;
			}
		}
	}
	return &m_Previews.emplace(strAssetId, std::move(Staged)).first->second;
}

bool_t Client::CEffect_Tool_V2::Create_ModelThumbnail(
	const std::filesystem::path& Path,
	ComPtr<ID3D11ShaderResourceView>& OutTextureView,
	std::string& strOutError,
	std::string& strOutInfo)
{
	unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
		m_pDevice, m_pContext, MODEL::NONANIM,
		Path.string().c_str(), XMMatrixIdentity());
	bool_t bSkinned = false;
	if (nullptr == Model)
	{
		Model = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::ANIM,
			Path.string().c_str(), XMMatrixIdentity());
		bSkinned = nullptr != Model;
	}
	if (nullptr == Model)
	{
		strOutError = "WModel load failed: " + Path.string() + "\n" +
			CModelDecoderRegistry::Get().Get_LastReport().error;
		return false;
	}
	float3_t Minimum{};
	float3_t Maximum{};
	if (Model->Has_LocalBounds())
	{
		Minimum = Model->Get_LocalBoundsMin();
		Maximum = Model->Get_LocalBoundsMax();
	}
	else if (!Compute_SkinnedBounds(Path, *Model, Minimum, Maximum, strOutInfo))
	{
		strOutError = "WModel bounds failed: " + Path.string() + "\n" +
			CModelDecoderRegistry::Get().Get_LastReport().error;
		return false;
	}
	{
		char szBounds[192] = {};
		std::snprintf(szBounds, sizeof(szBounds),
			"%s | meshes=%u | min(%.2f %.2f %.2f) max(%.2f %.2f %.2f)",
			bSkinned ? "skinned" : "static", Model->Get_NumMeshes(),
			Minimum.x, Minimum.y, Minimum.z, Maximum.x, Maximum.y, Maximum.z);
		strOutInfo = strOutInfo.empty() ? szBounds : szBounds + ("\n" + strOutInfo);
	}

	shared_ptr<Engine::CShader>& pShader =
		bSkinned ? m_pAnimModelShader : m_pModelShader;
	if (nullptr == pShader)
	{
		unique_ptr<Engine::CShader> Shader = bSkinned ?
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements) :
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == Shader)
		{
			strOutError = bSkinned ?
				"Skinned mesh preview shader creation failed." :
				"Mesh preview shader creation failed.";
			return false;
		}
		pShader = std::move(Shader);
	}

	D3D11_TEXTURE2D_DESC ColorDesc{};
	ColorDesc.Width = MODEL_THUMBNAIL_SIZE;
	ColorDesc.Height = MODEL_THUMBNAIL_SIZE;
	ColorDesc.MipLevels = 1u;
	ColorDesc.ArraySize = 1u;
	ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorDesc.SampleDesc.Count = 1u;
	ColorDesc.Usage = D3D11_USAGE_DEFAULT;
	ColorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> ColorTexture;
	ComPtr<ID3D11RenderTargetView> ColorRTV;
	ComPtr<ID3D11ShaderResourceView> ColorSRV;
	if (FAILED(m_pDevice->CreateTexture2D(&ColorDesc, nullptr, &ColorTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(
			ColorTexture.Get(), nullptr, &ColorRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			ColorTexture.Get(), nullptr, &ColorSRV)))
	{
		strOutError = "Mesh preview color target creation failed.";
		return false;
	}

	D3D11_TEXTURE2D_DESC DepthDesc = ColorDesc;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ComPtr<ID3D11Texture2D> DepthTexture;
	ComPtr<ID3D11DepthStencilView> DepthDSV;
	if (FAILED(m_pDevice->CreateTexture2D(&DepthDesc, nullptr, &DepthTexture)) ||
		FAILED(m_pDevice->CreateDepthStencilView(
			DepthTexture.Get(), nullptr, &DepthDSV)))
	{
		strOutError = "Mesh preview depth target creation failed.";
		return false;
	}

	const uint32_t WhitePixel = 0xffffffffu;
	D3D11_TEXTURE2D_DESC WhiteDesc{};
	WhiteDesc.Width = 1u;
	WhiteDesc.Height = 1u;
	WhiteDesc.MipLevels = 1u;
	WhiteDesc.ArraySize = 1u;
	WhiteDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WhiteDesc.SampleDesc.Count = 1u;
	WhiteDesc.Usage = D3D11_USAGE_IMMUTABLE;
	WhiteDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA WhiteData{};
	WhiteData.pSysMem = &WhitePixel;
	WhiteData.SysMemPitch = sizeof(WhitePixel);
	ComPtr<ID3D11Texture2D> WhiteTexture;
	ComPtr<ID3D11ShaderResourceView> WhiteSRV;
	if (FAILED(m_pDevice->CreateTexture2D(&WhiteDesc, &WhiteData, &WhiteTexture)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			WhiteTexture.Get(), nullptr, &WhiteSRV)))
	{
		strOutError = "Mesh preview fallback texture creation failed.";
		return false;
	}

	const float3_t Center(
		(Minimum.x + Maximum.x) * 0.5f,
		(Minimum.y + Maximum.y) * 0.5f,
		(Minimum.z + Maximum.z) * 0.5f);
	const float3_t HalfExtent(
		(Maximum.x - Minimum.x) * 0.5f,
		(Maximum.y - Minimum.y) * 0.5f,
		(Maximum.z - Minimum.z) * 0.5f);
	const f32_t Radius = std::sqrt(
		HalfExtent.x * HalfExtent.x +
		HalfExtent.y * HalfExtent.y +
		HalfExtent.z * HalfExtent.z);
	if (!std::isfinite(Radius) || Radius <= 0.0001f)
	{
		strOutError = "Mesh preview bounds are invalid.";
		return false;
	}

	ComPtr<ID3D11RenderTargetView> PreviousRTV;
	ComPtr<ID3D11DepthStencilView> PreviousDSV;
	m_pContext->OMGetRenderTargets(1u, &PreviousRTV, &PreviousDSV);
	std::array<D3D11_VIEWPORT,
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
		PreviousViewports{};
	uint32_t iPreviousViewportCount =
		static_cast<uint32_t>(PreviousViewports.size());
	m_pContext->RSGetViewports(&iPreviousViewportCount, PreviousViewports.data());

	ID3D11RenderTargetView* pTarget = ColorRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pTarget, DepthDSV.Get());
	const float ClearColor[4] = { 0.035f, 0.045f, 0.06f, 1.f };
	m_pContext->ClearRenderTargetView(ColorRTV.Get(), ClearColor);
	m_pContext->ClearDepthStencilView(
		DepthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<f32_t>(MODEL_THUMBNAIL_SIZE);
	Viewport.Height = static_cast<f32_t>(MODEL_THUMBNAIL_SIZE);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;
	m_pContext->RSSetViewports(1u, &Viewport);

	const f32_t Distance = Radius / std::sin(XMConvertToRadians(22.5f));
	const matrix_t WorldMatrix =
		XMMatrixTranslation(-Center.x, -Center.y, -Center.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(18.f), XMConvertToRadians(-32.f), 0.f);
	const vector_t Eye = XMVectorSet(0.f, 0.f, -Distance, 1.f);
	const matrix_t ViewMatrix = XMMatrixLookAtLH(
		Eye, XMVectorZero(), XMVectorSet(0.f, 1.f, 0.f, 0.f));
	const matrix_t ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.f), 1.f,
		(std::max)(0.001f, Distance - Radius * 1.25f),
		Distance + Radius * 3.f);
	float4x4_t World{};
	float4x4_t View{};
	float4x4_t Projection{};
	XMStoreFloat4x4(&World, WorldMatrix);
	XMStoreFloat4x4(&View, ViewMatrix);
	XMStoreFloat4x4(&Projection, ProjectionMatrix);
	float3_t CameraPosition{};
	XMStoreFloat3(&CameraPosition, Eye);
	const float3_t LightDirection(-0.45f, -0.75f, 0.35f);

	bool_t bRendered =
		SUCCEEDED(pShader->Bind_Matrix("g_WorldMatrix", &World)) &&
		SUCCEEDED(pShader->Bind_Matrix("g_ViewMatrix", &View)) &&
		SUCCEEDED(pShader->Bind_Matrix("g_ProjMatrix", &Projection)) &&
		SUCCEEDED(pShader->Bind_RawValue(
			"g_CameraPosition", &CameraPosition, sizeof(CameraPosition))) &&
		SUCCEEDED(pShader->Bind_RawValue(
			"g_LightDirection", &LightDirection, sizeof(LightDirection)));
	for (uint32_t iMesh = 0u; bRendered && iMesh < Model->Get_NumMeshes(); ++iMesh)
	{
		const uint32_t iHasNormal =
			Model->Has_MaterialTexture(iMesh, aiTextureType_NORMALS) ? 1u : 0u;
		bRendered = SUCCEEDED(pShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal)));
		if (bRendered && bSkinned)
			bRendered = SUCCEEDED(Model->Bind_BoneMatrices(
				pShader, "g_BoneMatrices", iMesh));
		if (bRendered && Model->Has_MaterialTexture(iMesh, aiTextureType_DIFFUSE))
			bRendered = SUCCEEDED(Model->Bind_Material(
				pShader, "g_DiffuseTexture", iMesh, aiTextureType_DIFFUSE));
		else if (bRendered)
			bRendered = SUCCEEDED(pShader->Bind_Texture(
				"g_DiffuseTexture", WhiteSRV));
		if (bRendered && 0u != iHasNormal)
			bRendered = SUCCEEDED(Model->Bind_Material(
				pShader, "g_NormalTexture", iMesh, aiTextureType_NORMALS));
		bRendered = bRendered &&
			SUCCEEDED(pShader->Begin(0u)) &&
			SUCCEEDED(Model->Render(iMesh));
	}

	ID3D11RenderTargetView* pPreviousTarget = PreviousRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pPreviousTarget, PreviousDSV.Get());
	if (0u != iPreviousViewportCount)
		m_pContext->RSSetViewports(iPreviousViewportCount, PreviousViewports.data());
	if (!bRendered)
	{
		strOutError = "Mesh preview render failed: " + Path.string();
		return false;
	}
	OutTextureView = std::move(ColorSRV);
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool_V2::Compute_SkinnedBounds(
	const std::filesystem::path& Path,
	const Engine::CModel& Model,
	float3_t& OutMinimum,
	float3_t& OutMaximum,
	std::string& strOutInfo)
{
	MODEL_ASSET_LOAD_DESC Desc{};
	Desc.meshPath = Path.lexically_normal();
	std::error_code Error;
	const std::filesystem::path Absolute =
		std::filesystem::absolute(Desc.meshPath, Error).lexically_normal();
	for (std::filesystem::path Current = Absolute.parent_path();
		!Current.empty() && Current != Current.parent_path();
		Current = Current.parent_path())
	{
		if (L"Resources" == Current.filename())
		{
			Desc.assetRoot = Current;
			break;
		}
	}
	MODEL_ASSET_DATA Asset{};
	if (!CModelDecoderRegistry::Get().Decode(Desc, Asset) || Asset.meshes.empty())
		return false;

	const f32_t fMax = (std::numeric_limits<f32_t>::max)();
	float3_t Minimum(fMax, fMax, fMax);
	float3_t Maximum(-fMax, -fMax, -fMax);
	bool_t bAny = false;
	const auto Include = [&](const float3_t& Position)
	{
		if (!std::isfinite(Position.x) || !std::isfinite(Position.y) ||
			!std::isfinite(Position.z))
			return;
		Minimum.x = (std::min)(Minimum.x, Position.x);
		Minimum.y = (std::min)(Minimum.y, Position.y);
		Minimum.z = (std::min)(Minimum.z, Position.z);
		Maximum.x = (std::max)(Maximum.x, Position.x);
		Maximum.y = (std::max)(Maximum.y, Position.y);
		Maximum.z = (std::max)(Maximum.z, Position.z);
		bAny = true;
	};
	const size_t iBoneCount = Asset.skeleton.bones.size();
	std::vector<matrix_t> SkinMatrices(iBoneCount, XMMatrixIdentity());
	for (size_t iBone = 0u; iBone < iBoneCount; ++iBone)
	{
		matrix_t Combined = XMMatrixIdentity();
		if (!Model.Get_BoneCombinedMatrix(static_cast<uint32_t>(iBone), Combined))
			return false;
		SkinMatrices[iBone] =
			XMLoadFloat4x4(&Asset.skeleton.bones[iBone].inverseBind) * Combined;
	}
	for (const MODEL_MESH_DATA& Mesh : Asset.meshes)
	{
		for (const VTXANIMMESH& Vertex : Mesh.skinnedVertices)
		{
			const uint32_t Indices[4] = {
				Vertex.vBlendIndices.x, Vertex.vBlendIndices.y,
				Vertex.vBlendIndices.z, Vertex.vBlendIndices.w };
			const f32_t Weights[4] = {
				Vertex.vBlendWeights.x, Vertex.vBlendWeights.y,
				Vertex.vBlendWeights.z, Vertex.vBlendWeights.w };
			matrix_t Skin = XMMatrixSet(
				0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
			f32_t fTotalWeight = 0.f;
			for (uint32_t iInfluence = 0u; iInfluence < 4u; ++iInfluence)
			{
				if (Weights[iInfluence] <= 0.f || Indices[iInfluence] >= iBoneCount)
					continue;
				Skin += SkinMatrices[Indices[iInfluence]] * Weights[iInfluence];
				fTotalWeight += Weights[iInfluence];
			}
			if (fTotalWeight <= 0.f)
			{
				Include(Vertex.vPosition);
				continue;
			}
			float3_t Position{};
			XMStoreFloat3(&Position, XMVector3TransformCoord(
				XMLoadFloat3(&Vertex.vPosition), Skin));
			Include(Position);
		}
		for (const VTXMESH& Vertex : Mesh.vertices)
			Include(Vertex.vPosition);
	}
	if (!bAny)
		return false;
	OutMinimum = Minimum;
	OutMaximum = Maximum;

	strOutInfo = "bones=" + std::to_string(Asset.skeleton.bones.size()) +
		" anims=" + std::to_string(Asset.animations.size()) +
		" materials=" + std::to_string(Asset.materials.size());
	for (size_t iMesh = 0u; iMesh < Asset.meshes.size(); ++iMesh)
	{
		const MODEL_MESH_DATA& Mesh = Asset.meshes[iMesh];
		strOutInfo += "\n[" + std::to_string(iMesh) + "] " + Mesh.name +
			" v=" + std::to_string(Mesh.skinnedVertices.size() + Mesh.vertices.size()) +
			" mat=" + std::to_string(Mesh.materialIndex);
		if (Mesh.materialIndex < Asset.materials.size())
		{
			strOutInfo += " diffuse=" +
				Asset.materials[Mesh.materialIndex].diffusePath.filename().string();
		}
	}
	return true;
}

void Client::CEffect_Tool_V2::Render_TypeSelector()
{
	ImGui::TextUnformatted("Effect Type");
	for (int32_t iType = 0; iType < static_cast<int32_t>(EFFECT_TYPE::END);
		++iType)
	{
		if (0 != iType)
			ImGui::SameLine();
		const EFFECT_TYPE eType = static_cast<EFFECT_TYPE>(iType);
		if (ImGui::RadioButton(Type_Label(eType), m_eType == eType))
		{
			m_eType = eType;
			m_bVisibleDirty = true;
		}
	}
}

void Client::CEffect_Tool_V2::Render_SlotCards()
{
	ImGui::Text("%s Slots", Type_Label(m_eType));
	SLOT_BINDINGS& Bindings = Current_Bindings();
	bool_t bFirst = true;
	for (int32_t iSlot = 0; iSlot < static_cast<int32_t>(RESOURCE_SLOT::END);
		++iSlot)
	{
		const RESOURCE_SLOT eSlot = static_cast<RESOURCE_SLOT>(iSlot);
		if (!Slot_VisibleForType(eSlot))
			continue;
		std::string& strAssetId = Bindings[static_cast<size_t>(iSlot)];
		if (!bFirst)
			ImGui::SameLine();
		bFirst = false;
		ImGui::PushID(iSlot);
		ImGui::BeginGroup();

		const PREVIEW_ENTRY* pPreview =
			Request_Preview(strAssetId, Slot_Kind(eSlot));
		bool_t bClicked = false;
		if (nullptr != pPreview && nullptr != pPreview->pTextureView)
		{
			bClicked = ImGui::ImageButton("slot",
				pPreview->pTextureView.Get(),
				ImVec2(SLOT_CARD_SIZE, SLOT_CARD_SIZE));
		}
		else
		{
			const char* pLabel = strAssetId.empty() ? "Empty" :
				(nullptr == pPreview ? "Loading" : "Error");
			bClicked = ImGui::Button(pLabel,
				ImVec2(SLOT_CARD_SIZE + 8.f, SLOT_CARD_SIZE + 8.f));
			if (nullptr != pPreview && !pPreview->strError.empty() &&
				ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", pPreview->strError.c_str());
			}
		}
		if (m_eSelectedSlot == eSlot)
		{
			ImGui::GetWindowDrawList()->AddRect(
				ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
				ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 3.f);
		}
		if (bClicked && m_eSelectedSlot != eSlot)
		{
			m_eSelectedSlot = eSlot;
			m_bVisibleDirty = true;
		}

		ImGui::TextUnformatted(Slot_Label(eSlot));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", Slot_Description(eSlot));
		if (strAssetId.empty())
		{
			ImGui::TextDisabled("(none)");
		}
		else
		{
			std::string Name =
				std::filesystem::path(strAssetId).filename().string();
			if (Name.size() > 12u)
				Name = Name.substr(0u, 10u) + "..";
			ImGui::TextDisabled("%s", Name.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", strAssetId.c_str());
			if (ImGui::SmallButton("Clear"))
				strAssetId.clear();
		}
		ImGui::EndGroup();
		ImGui::PopID();
	}
}

void Client::CEffect_Tool_V2::Render_ResourceBrowser()
{
	const RESOURCE_KIND eKind = Slot_Kind(m_eSelectedSlot);
	ImGui::Text("Resource Library (%s) -> %s / %s",
		RESOURCE_KIND::MODEL == eKind ? "WModel" : "DDS",
		Type_Label(m_eType), Slot_Label(m_eSelectedSlot));

	if (ImGui::Button("Rescan"))
		Scan_Resources();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(180.f);
	if (ImGui::BeginCombo("Domain",
		m_strDomainFilter.empty() ? "All" : m_strDomainFilter.c_str()))
	{
		if (ImGui::Selectable("All", m_strDomainFilter.empty()))
		{
			m_strDomainFilter.clear();
			m_bVisibleDirty = true;
		}
		for (const std::string& Domain : m_Domains)
		{
			if (ImGui::Selectable(Domain.c_str(), m_strDomainFilter == Domain))
			{
				m_strDomainFilter = Domain;
				m_bVisibleDirty = true;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(200.f);
	if (ImGui::InputTextWithHint("##NameFilter", "filename filter",
		m_szNameFilter, sizeof(m_szNameFilter)))
	{
		m_bVisibleDirty = true;
	}

	if (m_bVisibleDirty || m_eVisibleKind != eKind)
		Rebuild_VisibleResources();
	ImGui::TextDisabled("%zu shown", m_VisibleResources.size());

	if (!ImGui::BeginChild("ResourceGrid", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}
	const float fTileWidth = BROWSER_TILE_SIZE + 12.f;
	const int32_t iColumns = (std::max)(1,
		static_cast<int32_t>(ImGui::GetContentRegionAvail().x / fTileWidth));
	const int32_t iRows = static_cast<int32_t>(
		(m_VisibleResources.size() + iColumns - 1u) / iColumns);
	std::string& strBoundAssetId = Current_SlotAssetId();

	ImGuiListClipper Clipper;
	Clipper.Begin(iRows, BROWSER_TILE_SIZE + 56.f);
	while (Clipper.Step())
	{
		for (int32_t iRow = Clipper.DisplayStart; iRow < Clipper.DisplayEnd;
			++iRow)
		{
			for (int32_t iColumn = 0; iColumn < iColumns; ++iColumn)
			{
				const size_t iVisible = static_cast<size_t>(
					iRow * iColumns + iColumn);
				if (iVisible >= m_VisibleResources.size())
					break;
				const RESOURCE_ENTRY& Entry =
					m_Resources[m_VisibleResources[iVisible]];
				if (0 != iColumn)
					ImGui::SameLine();
				ImGui::PushID(Entry.strAssetId.c_str());
				ImGui::BeginGroup();

				const PREVIEW_ENTRY* pPreview =
					Request_Preview(Entry.strAssetId, Entry.eKind);
				bool_t bClicked = false;
				if (nullptr != pPreview && nullptr != pPreview->pTextureView)
				{
					bClicked = ImGui::ImageButton("tile",
						pPreview->pTextureView.Get(),
						ImVec2(BROWSER_TILE_SIZE, BROWSER_TILE_SIZE));
				}
				else
				{
					bClicked = ImGui::Button(
						nullptr == pPreview ? "..." :
							(RESOURCE_KIND::MODEL == Entry.eKind ? "Mesh" : "DDS"),
						ImVec2(BROWSER_TILE_SIZE + 8.f, BROWSER_TILE_SIZE + 8.f));
					if (nullptr != pPreview && !pPreview->strError.empty() &&
						ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", pPreview->strError.c_str());
					}
				}
				if (Entry.strAssetId == strBoundAssetId)
				{
					ImGui::GetWindowDrawList()->AddRect(
						ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 3.f);
				}
				std::string Name = Entry.strFileName;
				if (Name.size() > 12u)
					Name = Name.substr(0u, 10u) + "..";
				ImGui::TextUnformatted(Name.c_str());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", Entry.strAssetId.c_str());
				if (nullptr != Entry.pUsage)
				{
					std::string Badge;
					for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
						iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
					{
						const uint32_t iCount =
							Entry.pUsage->Counts[static_cast<size_t>(iSlot)];
						if (0u == iCount)
							continue;
						if (!Badge.empty())
							Badge += ' ';
						Badge += Slot_Label(static_cast<RESOURCE_SLOT>(iSlot))[0];
						Badge += std::to_string(iCount);
					}
					ImGui::TextDisabled("%s", Badge.c_str());
					if (ImGui::IsItemHovered())
					{
						std::string Tip = "Source usage (B=Base N=Noise M=Mask E=Emissive D=Dissolve)";
						for (const std::string& Param : Entry.pUsage->Params)
							Tip += "\n  " + Param;
						ImGui::SetTooltip("%s", Tip.c_str());
					}
				}
				ImGui::EndGroup();
				if (bClicked)
				{
					strBoundAssetId = Entry.strAssetId;
					m_strStatus = std::string("Bound ") + Slot_Label(m_eSelectedSlot) +
						" <- " + Entry.strAssetId;
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
}

void Client::CEffect_Tool_V2::Render_PreviewPanel()
{
	ImGui::Text("Preview: %s / %s",
		Type_Label(m_eType), Slot_Label(m_eSelectedSlot));
	const std::string& strAssetId = Current_SlotAssetId();
	if (strAssetId.empty())
	{
		ImGui::TextDisabled("Select a slot card, then click a resource in the library.");
		return;
	}
	const PREVIEW_ENTRY* pPreview =
		Request_Preview(strAssetId, Slot_Kind(m_eSelectedSlot));
	if (nullptr == pPreview)
	{
		ImGui::TextDisabled("Loading...");
		return;
	}
	if (nullptr == pPreview->pTextureView)
	{
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s",
			pPreview->strError.c_str());
		return;
	}
	const float fAvail = (std::max)(64.f,
		(std::min)(PREVIEW_PANEL_SIZE, ImGui::GetContentRegionAvail().x));
	float fWidth = fAvail;
	float fHeight = fAvail;
	if (pPreview->iWidth > 0u && pPreview->iHeight > 0u)
	{
		const float fAspect = static_cast<float>(pPreview->iWidth) /
			static_cast<float>(pPreview->iHeight);
		if (fAspect >= 1.f)
			fHeight = fAvail / fAspect;
		else
			fWidth = fAvail * fAspect;
	}
	ImGui::Image(pPreview->pTextureView.Get(), ImVec2(fWidth, fHeight));
	ImGui::TextWrapped("%s", strAssetId.c_str());
	if (RESOURCE_KIND::MODEL == Slot_Kind(m_eSelectedSlot))
		ImGui::TextDisabled("WModel thumbnail %u px", pPreview->iWidth);
	else
		ImGui::TextDisabled("%u x %u", pPreview->iWidth, pPreview->iHeight);
	if (!pPreview->strInfo.empty())
		ImGui::TextWrapped("%s", pPreview->strInfo.c_str());
}

void Client::CEffect_Tool_V2::Render_CreatePanel()
{
	ImGui::TextUnformatted("World Preview");
	const SLOT_BINDINGS& Bindings = Current_Bindings();
	const bool_t bMeshType = EFFECT_TYPE::MESH == m_eType;
	const bool_t bSupportedType = true;
	const bool_t bHasBase = EFFECT_TYPE::SCREEN_POST == m_eType ||
		!Bindings[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty();
	const bool_t bHasMesh =
		!Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)].empty();
	const bool_t bCanCreate =
		bSupportedType && bHasBase && (!bMeshType || bHasMesh);
	ImGui::BeginDisabled(!bCanCreate);
	if (ImGui::Button("Create Effect"))
		Try_CreatePreview();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_pPreview.expired());
	if (ImGui::Button("Open Tuning"))
		m_bTuningWindowOpen = true;
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Attach..."))
		m_bAttachWindowOpen = true;
	ImGui::SameLine();
	if (ImGui::Button("Group..."))
		m_bGroupWindowOpen = true;
	if (!bHasBase)
		ImGui::TextDisabled("Bind a Base texture first.");
	else if (bMeshType && !bHasMesh)
		ImGui::TextDisabled("Bind a Mesh first.");
	if (!m_strPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strPreviewStatus.c_str());
}

bool_t Client::CEffect_Tool_V2::Try_CreatePreview()
{
	const SLOT_BINDINGS& Bindings = Current_Bindings();
	CEffectV2Object::DESC Desc{};
	Desc.eShape = CEffectV2Document::Shape_ForType(m_eType);
	Desc.strMeshAssetId = Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)];
	for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
		iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
	{
		Desc.TextureAssetIds[static_cast<size_t>(
			iSlot - static_cast<int32_t>(RESOURCE_SLOT::BASE))] =
			Bindings[static_cast<size_t>(iSlot)];
	}
	return Spawn_Preview(Desc, {}, std::string());
}

bool_t Client::CEffect_Tool_V2::Spawn_Preview(
	const CEffectV2Object::DESC& SourceDesc,
	const std::vector<PART_OVERRIDE>& Parts,
	const std::string& strAnimationClip)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (!m_bPreviewPrototypeRegistered)
	{
		unique_ptr<CEffectV2Object> pPrototype =
			CEffectV2Object::Create(m_pDevice, m_pContext);
		if (nullptr == pPrototype)
		{
			m_strPreviewStatus = "Preview prototype creation failed.";
			return false;
		}
		const HRESULT hResult = GameInstance.Add_Prototype(
			ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectPreviewV2",
			std::move(pPrototype));
		if (FAILED(hResult))
		{
			m_strPreviewStatus =
				"Prototype registration returned failure (already registered or STATIC level unavailable); continuing.";
		}
		m_bPreviewPrototypeRegistered = true;
	}
	if (const std::shared_ptr<CEffectV2Object> pPrevious = m_pPreview.lock())
		pPrevious->Set_Hidden(true);

	CEffectV2Object::DESC Desc = SourceDesc;
	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4x4_t* pCameraWorld = GameInstance.Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == pCameraPosition || nullptr == pCameraWorld)
	{
		m_strPreviewStatus = "Camera is not available in this level.";
		return false;
	}
	const vector_t Look = XMVector3Normalize(XMLoadFloat4x4(pCameraWorld).r[2]);
	const vector_t Spawn = XMLoadFloat4(pCameraPosition) + Look * 3.f;
	XMStoreFloat4x4(&Desc.PivotWorld, XMMatrixTranslationFromVector(Spawn));

	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectPreviewV2",
		GameInstance.Get_CurrentLevelID(), EFFECT_V2_PREVIEW_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strPreviewStatus = "Create failed: " +
			(CEffectV2Object::Last_Error().empty() ?
				std::string("prototype clone or layer add failed.") :
				CEffectV2Object::Last_Error());
		return false;
	}
	const std::shared_ptr<CEffectV2Object> pPreview =
		std::dynamic_pointer_cast<CEffectV2Object>(pGameObject);
	if (nullptr == pPreview)
	{
		m_strPreviewStatus = "Create failed: unexpected object type.";
		return false;
	}
	for (uint32_t iPart = 0u;
		iPart < Parts.size() && iPart < pPreview->Part_Count(); ++iPart)
	{
		pPreview->Part_Visible(iPart) = Parts[iPart].bVisible;
		if (!Parts[iPart].strBaseAssetId.empty() &&
			FAILED(pPreview->Set_PartBase(iPart, Parts[iPart].strBaseAssetId)))
		{
			m_strPreviewStatus = "Part texture load failed: " + Parts[iPart].strBaseAssetId;
		}
	}
	if (!strAnimationClip.empty())
	{
		bool_t bFound = false;
		for (uint32_t iClip = 0u; iClip < pPreview->Animation_Count(); ++iClip)
		{
			const char_t* pName = pPreview->Animation_Name(iClip);
			if (nullptr != pName && strAnimationClip == pName)
			{
				pPreview->Params().iAnimationIndex = iClip;
				bFound = true;
				break;
			}
		}
		if (!bFound)
			m_strPreviewStatus = "Animation clip not found in model: " + strAnimationClip;
	}
	m_pPreview = pPreview;
	m_ePreviewType = m_eType;
	m_bTuningWindowOpen = true;
	if (m_strPreviewStatus.empty() || 0 == m_strPreviewStatus.rfind("Pivot", 0))
		m_strPreviewStatus = "Pivot spawned at camera forward 3 m.";
	return true;
}

void Client::CEffect_Tool_V2::Scan_Documents()
{
	m_bDocumentsScanned = true;
	m_Documents.clear();
	std::error_code Error;
	const std::filesystem::path Directory = CEffectV2Document::Document_Directory();
	if (Directory.empty() || !std::filesystem::is_directory(Directory, Error))
		return;
	for (const std::filesystem::directory_entry& Entry :
		std::filesystem::directory_iterator(Directory, Error))
	{
		if (!Entry.is_regular_file(Error))
			continue;
		const std::string strName = Entry.path().filename().string();
		constexpr const char* SUFFIX = ".effectv2.json";
		const size_t iSuffix = std::strlen(SUFFIX);
		if (strName.size() <= iSuffix ||
			strName.compare(strName.size() - iSuffix, iSuffix, SUFFIX) != 0)
			continue;
		m_Documents.push_back(strName.substr(0u, strName.size() - iSuffix));
	}
	std::sort(m_Documents.begin(), m_Documents.end());
}

bool_t Client::CEffect_Tool_V2::Save_Document()
{
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	const std::string strEffectId = m_szEffectId;
	if (nullptr == pPreview)
	{
		m_strDocumentStatus = "Create Effect first; Save writes the live preview.";
		return false;
	}
	if (!CEffectV2Document::Is_ValidEffectId(strEffectId))
	{
		m_strDocumentStatus = "Effect ID must be 1-80 chars of [A-Za-z0-9._-].";
		return false;
	}
	EFFECT_V2_DOCUMENT Document;
	Document.strEffectId = strEffectId;
	Document.eType = m_ePreviewType;
	Document.Desc = pPreview->Creation_Desc();
	Document.Desc.Params = pPreview->Params();
	Document.Desc.bParamsAuthored = true;
	const char_t* pClip = pPreview->Animation_Name(pPreview->Params().iAnimationIndex);
	Document.strAnimationClip = nullptr != pClip ? pClip : "";
	for (uint32_t iPart = 0u; iPart < pPreview->Part_Count(); ++iPart)
	{
		EFFECT_V2_PART_OVERRIDE Part;
		Part.bVisible = pPreview->Part_Visible(iPart);
		Part.strBaseAssetId = pPreview->Part_BaseAssetId(iPart);
		Document.Parts.push_back(std::move(Part));
	}
	std::string strError;
	if (!CEffectV2Document::Write_AtomicFile(
		CEffectV2Document::Document_Path(strEffectId),
		CEffectV2Document::Serialize_Document(Document), strError))
	{
		m_strDocumentStatus = strError;
		return false;
	}
	CEffectV2Runtime::Invalidate_Caches();
	m_bDocumentsScanned = false;
	m_strDocumentStatus = "Saved " + strEffectId + ".effectv2.json";
	return true;
}

bool_t Client::CEffect_Tool_V2::Load_Document(const std::string& strEffectId)
{
	EFFECT_V2_DOCUMENT Document;
	std::string strError;
	if (!CEffectV2Document::Load_DocumentFile(strEffectId, Document, strError))
	{
		m_strDocumentStatus = "Load rejected (" + strEffectId + "): " + strError;
		return false;
	}
	const EFFECT_TYPE ePreviousType = m_eType;
	const SLOT_BINDINGS PreviousBindings = m_SlotBindings[static_cast<size_t>(Document.eType)];
	m_eType = Document.eType;
	SLOT_BINDINGS& Bindings = Current_Bindings();
	Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)] = Document.Desc.strMeshAssetId;
	for (size_t iInput = 0u; iInput < Document.Desc.TextureAssetIds.size(); ++iInput)
	{
		Bindings[static_cast<size_t>(RESOURCE_SLOT::BASE) + iInput] =
			Document.Desc.TextureAssetIds[iInput];
	}
	if (!Spawn_Preview(Document.Desc, Document.Parts, Document.strAnimationClip))
	{
		m_SlotBindings[static_cast<size_t>(Document.eType)] = PreviousBindings;
		m_eType = ePreviousType;
		m_strDocumentStatus = "Load failed (" + strEffectId + "): " + m_strPreviewStatus;
		return false;
	}
	std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", strEffectId.c_str());
	m_bVisibleDirty = true;
	m_strDocumentStatus = "Loaded " + strEffectId;
	return true;
}

void Client::CEffect_Tool_V2::Render_DocumentPanel()
{
	if (!m_bDocumentsScanned)
		Scan_Documents();
	ImGui::TextUnformatted("Document (Data/Effects/V2/Authored)");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##EffectId", "Effect ID (e.g. esther.wei.dochul)",
		m_szEffectId, sizeof(m_szEffectId));
	ImGui::BeginDisabled(m_pPreview.expired() || '\0' == m_szEffectId[0]);
	if (ImGui::Button("Save"))
		Save_Document();
	ImGui::EndDisabled();
	if (m_pPreview.expired())
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(Create Effect first)");
	}
	ImGui::SameLine();
	if (ImGui::Button("Rescan"))
		Scan_Documents();
	if (m_Documents.empty())
		ImGui::TextDisabled("No saved documents.");
	else if (ImGui::BeginListBox("##Documents", ImVec2(-1.f, 88.f)))
	{
		for (const std::string& strDocument : m_Documents)
		{
			const bool_t bSelected = strDocument == m_szEffectId;
			if (ImGui::Selectable(strDocument.c_str(), bSelected))
				std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", strDocument.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				Load_Document(strDocument);
		}
		ImGui::EndListBox();
	}
	ImGui::BeginDisabled('\0' == m_szEffectId[0]);
	if (ImGui::Button("Load"))
		Load_Document(m_szEffectId);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("(double-click a row to load)");
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

namespace
{
	constexpr const wchar_t* TARGET_LAYER_TAG = L"Layer_EffectPreviewV2Target";
	constexpr f32_t BINDING_FRAME_RATE = 30.f;
	constexpr const char* VALTAN_TARGET_ARCHETYPE_ID = "BOSS_VALTAN";

	bool_t Resolve_ValtanStageAction(
		const std::string_view strStageKind,
		LostArk::Shared::WORLD_ENTITY_ACTION& eOutAction)
	{
		using LostArk::Shared::WORLD_ENTITY_ACTION;
		if ("WINDUP" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		else if ("ACTIVE" == strStageKind || "GROGGY" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		else if ("RECOVERY" == strStageKind || "PART_BREAK" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		else
			return false;
		return true;
	}
}

bool_t Client::CEffect_Tool_V2::Collect_BoneNames(
	const std::string& strModelAssetId,
	std::vector<std::string>& OutNames)
{
	OutNames.clear();
	MODEL_ASSET_LOAD_DESC Desc{};
	Desc.meshPath = CRuntimeAssetRoot::Resolve(std::filesystem::path(strModelAssetId));
	if (Desc.meshPath.empty())
		return false;
	Desc.assetRoot = CRuntimeAssetRoot::Get();
	MODEL_ASSET_DATA Asset{};
	if (!CModelDecoderRegistry::Get().Decode(Desc, Asset))
		return false;
	for (const MODEL_BONE_DATA& Bone : Asset.skeleton.bones)
		OutNames.push_back(Bone.name);
	return !OutNames.empty();
}

bool_t Client::CEffect_Tool_V2::Spawn_Target(const std::string& strArchetypeId)
{
	Despawn_Target();
	float3_t vPosition{ 0.f, 0.f, 0.f };
	if (const std::shared_ptr<CCharacter> pCharacter =
		CAnimationTargetService::Resolve_SceneCharacter();
		nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
	{
		XMStoreFloat3(&vPosition,
			pCharacter->Get_Transform()->Get_State(STATE::POSITION) +
			XMVectorSet(2.5f, 0.f, 0.f, 0.f));
	}
	const bool_t bSpawned = VALTAN_TARGET_ARCHETYPE_ID == strArchetypeId ?
		Spawn_ValtanTarget(vPosition) : Spawn_NpcTarget(strArchetypeId, vPosition);
	if (!bSpawned)
		return false;

	EFFECT_V2_TARGET_VIEW View;
	if (!Resolve_TargetView(View))
	{
		Despawn_Target();
		m_strAttachStatus = "Target spawn returned an unexpected object.";
		return false;
	}
	CEffectV2Runtime::Set_Ignored(m_Target, !m_bRuntimeOnTarget);
	m_strTargetArchetypeId = strArchetypeId;
	m_vTargetPosition = vPosition;
	m_fTargetYawDegrees = 0.f;
	m_fTargetLastClipSeconds = -1.f;
	if (m_strPivotBone.empty() || !View.pModel->Has_Bone(m_strPivotBone.c_str()))
	{
		m_strPivotBone = View.pModel->Has_Bone("b_effectroot") ? "b_effectroot" :
			(m_TargetBoneNames.empty() ? std::string() : m_TargetBoneNames.front());
	}
	Load_Bindings(strArchetypeId);
	m_strAttachStatus = "Target " + strArchetypeId + " spawned beside the scene character.";
	return true;
}

bool_t Client::CEffect_Tool_V2::Spawn_NpcTarget(
	const std::string& strArchetypeId,
	const float3_t& vPosition)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const uint32_t iLevel = GameInstance.Get_CurrentLevelID();
	const NPC_ACTOR_ENTRY* pActor = CActorCatalog::Find_Npc(strArchetypeId);
	const wstring_t strModelTag =
		CNpcPresentationAssetService::Get_ModelPrototypeTag(strArchetypeId);
	if (nullptr == pActor || strModelTag.empty())
	{
		m_strAttachStatus = "Unknown NPC archetype: " + strArchetypeId;
		return false;
	}
	if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
		m_pDevice, m_pContext, iLevel, strArchetypeId)))
	{
		m_strAttachStatus = "NPC presentation prototypes failed: " + strArchetypeId;
		return false;
	}

	CNpc::NPC_DESC Desc{};
	Desc.iPrototypeLevelIndex = iLevel;
	Desc.strModelTag = strModelTag;
	Desc.strShaderTag = pActor->shaderProfile == "esther" ?
		TEXT("Prototype_Component_Shader_VtxEstherNpc") :
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
	Desc.pIdleClip = pActor->idleClip.c_str();
	Desc.isLoop = true;
	Desc.vPosition = vPosition;
	Desc.fYawDegree = 0.f;
	Desc.fCollisionRadius = 0.f;
	if (pActor->shaderProfile == "esther")
		Desc.fOutlineWidth = CNpc::ESTHER_OUTLINE_WIDTH;
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		iLevel, TEXT("Prototype_GameObject_Npc"), iLevel, TARGET_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strAttachStatus = "Target spawn failed: " + strArchetypeId;
		return false;
	}
	const std::shared_ptr<CNpc> pNpc = std::dynamic_pointer_cast<CNpc>(pGameObject);
	if (nullptr == pNpc || nullptr == pNpc->Get_Model())
	{
		GameInstance.Remove_GameObject_from_Layer(iLevel, TARGET_LAYER_TAG, pGameObject);
		m_strAttachStatus = "Target spawn returned an unexpected object.";
		return false;
	}
	m_Target = EFFECT_V2_TARGET::From_Npc(pNpc);

	std::vector<std::string> BoneNames;
	Collect_BoneNames(pActor->modelAssetId, BoneNames);
	m_TargetBoneNames.clear();
	for (const std::string& strBone : BoneNames)
	{
		if (pNpc->Get_Model()->Has_Bone(strBone.c_str()))
			m_TargetBoneNames.push_back(strBone);
	}

	const auto Strike = pActor->actionClips.find("esther.strike");
	if (Strike != pActor->actionClips.end() && !Strike->second.empty())
		Play_TargetClip(Strike->second.front().c_str(), m_bTargetClipLoop);
	return true;
}

bool_t Client::CEffect_Tool_V2::Spawn_ValtanTarget(const float3_t& vPosition)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const uint32_t iLevel = GameInstance.Get_CurrentLevelID();
	const BOSS_ACTOR_ENTRY* pBoss = CActorCatalog::Find_Boss(VALTAN_TARGET_ARCHETYPE_ID);
	if (nullptr == pBoss || pBoss->clientPresentationId != "boss.valtan.client.v1")
	{
		m_strAttachStatus = "Valtan presentation contract is not admitted.";
		return false;
	}
	if (!CValtanPresentationAssetService::Is_Ready(iLevel) &&
		FAILED(CValtanPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, iLevel)))
	{
		m_strAttachStatus = "Valtan presentation prototypes failed.";
		return false;
	}

	CValtan::VALTAN_DESC Desc{};
	Desc.iPrototypeLevelIndex = iLevel;
	Desc.vPosition = vPosition;
	Desc.fScale = pBoss->presentationScale;
	Desc.fCollisionRadius = 0.f;
	Desc.isServerAuthoritative = false;
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		iLevel, TEXT("Prototype_GameObject_Valtan"), iLevel, TARGET_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strAttachStatus = "Valtan target spawn failed.";
		return false;
	}
	const std::shared_ptr<CValtan> pValtan = std::dynamic_pointer_cast<CValtan>(pGameObject);
	float4x4_t PresentationRoot{};
	if (nullptr == pValtan || nullptr == pValtan->Get_BodyModel() ||
		!pValtan->Try_Get_PresentationRootMatrix(&PresentationRoot))
	{
		GameInstance.Remove_GameObject_from_Layer(iLevel, TARGET_LAYER_TAG, pGameObject);
		m_strAttachStatus = "Valtan target did not expose its body presentation root.";
		return false;
	}
	m_Target = EFFECT_V2_TARGET::From_Valtan(pValtan);

	std::vector<std::string> BoneNames;
	Collect_BoneNames(pBoss->bodyModel, BoneNames);
	m_TargetBoneNames.clear();
	for (const std::string& strBone : BoneNames)
	{
		if (pValtan->Get_BodyModel()->Has_Bone(strBone.c_str()))
			m_TargetBoneNames.push_back(strBone);
	}

	if (!pBoss->presentationClips.idle.empty())
		Play_TargetClip(pBoss->presentationClips.idle.c_str(), m_bTargetClipLoop);
	return true;
}

bool_t Client::CEffect_Tool_V2::Resolve_TargetView(EFFECT_V2_TARGET_VIEW& OutView) const
{
	return m_Target.Is_Valid() && CEffectV2Object::Resolve_TargetView(m_Target, OutView);
}

void Client::CEffect_Tool_V2::Play_TargetClip(const char_t* pClipName, const bool_t bLoop)
{
	EFFECT_V2_TARGET_VIEW View;
	if (nullptr == pClipName || !Resolve_TargetView(View))
		return;
	View.pModel->Set_AnimationSpeed(1.f);
	if (!View.pModel->Set_Animation(pClipName, bLoop))
		return;
	View.pModel->Set_AnimTrackPosition(View.pModel->Get_CurrentAnimIndex(), 0.f);
	m_fTargetLastClipSeconds = -1.f;
	CEffectV2Runtime::Notify_Clip(m_Target, pClipName);
}

void Client::CEffect_Tool_V2::Despawn_Target()
{
	m_bValtanTimelineActive = false;
	m_bValtanTimelinePaused = false;
	m_fValtanTimelineSeconds = 0.f;
	m_iValtanTimelineStage = static_cast<size_t>(-1);
	m_ValtanTimeline.clear();
	if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
		pPreview->Clear_FollowTarget();
	m_ePivotMode = PIVOT_MODE::WORLD;
	if (const std::shared_ptr<CGameObject> pOwner = m_Target.pOwner.lock())
	{
		CEffectV2Runtime::Set_Ignored(m_Target, false);
		CGameInstance::Get().Remove_GameObject_from_Layer(
			CGameInstance::Get().Get_CurrentLevelID(), TARGET_LAYER_TAG, pOwner);
	}
	m_Target.Reset();
	m_TargetBoneNames.clear();
}

void Client::CEffect_Tool_V2::Move_Target(const float3_t& vPosition, const f32_t fYawDegrees)
{
	const std::shared_ptr<CGameObject> pOwner = m_Target.pOwner.lock();
	if (nullptr == pOwner)
		return;
	if (EFFECT_V2_TARGET_KIND::NPC == m_Target.eKind)
	{
		if (!std::static_pointer_cast<CNpc>(pOwner)->Apply_NetworkState(vPosition, fYawDegrees))
			return;
	}
	else if (EFFECT_V2_TARGET_KIND::VALTAN == m_Target.eKind)
	{
		const std::shared_ptr<CTransform> pTransform =
			std::static_pointer_cast<CValtan>(pOwner)->Get_Transform();
		if (nullptr == pTransform)
			return;
		pTransform->Set_State(STATE::POSITION,
			XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 1.f));
		pTransform->Rotation(0.f, fYawDegrees, 0.f);
	}
	else
		return;
	m_vTargetPosition = vPosition;
	m_fTargetYawDegrees = fYawDegrees;
}

void Client::CEffect_Tool_V2::Update_Attach(const f32_t fTimeDelta)
{
	Update_ValtanTimeline(fTimeDelta);
	CEffectV2Runtime::Advance_FreeGroups(fTimeDelta, m_pDevice, m_pContext);
	if (0u != m_iGroupPreviewHandle &&
		CEffectV2Runtime::Group_Seconds(m_iGroupPreviewHandle) < 0.f)
		m_iGroupPreviewHandle = 0u;
	EFFECT_V2_TARGET_VIEW View;
	const bool_t bHasTarget = Resolve_TargetView(View);
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (nullptr != pPreview)
	{
		if (PIVOT_MODE::TARGET_BONE == m_ePivotMode && bHasTarget &&
			!m_strPivotBone.empty())
		{
			pPreview->Set_FollowTarget(m_Target, m_strPivotBone, m_ePivotRotation);
		}
		else if (pPreview->Has_FollowTarget())
			pPreview->Clear_FollowTarget();
	}
	if (m_bTestOrbit && nullptr != pPreview && PIVOT_MODE::WORLD == m_ePivotMode)
	{
		m_fTestOrbitAngle += fTimeDelta * m_fTestOrbitSpeed;
		pPreview->PivotWorld()._41 = m_vTestOrbitCenter.x + std::cos(m_fTestOrbitAngle) * m_fTestOrbitRadius;
		pPreview->PivotWorld()._42 = m_vTestOrbitCenter.y;
		pPreview->PivotWorld()._43 = m_vTestOrbitCenter.z + std::sin(m_fTestOrbitAngle) * m_fTestOrbitRadius;
	}
	const bool_t bSnapOnRestart = PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode;
	if (!bHasTarget)
		return;
	const std::shared_ptr<Engine::CModel>& pModel = View.pModel;

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iClip);
	if (fTickPerSecond <= 0.f || !pModel->Get_AnimationProgress(iClip, fPosition, fDuration))
		return;
	const f32_t fSeconds = fPosition / fTickPerSecond;
	const f32_t fSpawnSeconds = static_cast<f32_t>(m_iSpawnFrame) / BINDING_FRAME_RATE;
	if (nullptr != pPreview && !pModel->Is_AnimPaused() &&
		m_fTargetLastClipSeconds >= 0.f && m_fTargetLastClipSeconds < fSpawnSeconds &&
		fSeconds >= fSpawnSeconds)
	{
		if (bSnapOnRestart)
			Snap_PivotToTarget();
		pPreview->Restart();
	}
	if (!m_bValtanTimelineActive && fSeconds < m_fTargetLastClipSeconds &&
		nullptr != pPreview && 0 == m_iSpawnFrame)
	{
		if (bSnapOnRestart)
			Snap_PivotToTarget();
		pPreview->Restart();
	}
	m_fTargetLastClipSeconds = fSeconds;
}

void Client::CEffect_Tool_V2::Snap_PivotToTarget()
{
	EFFECT_V2_TARGET_VIEW View;
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (!Resolve_TargetView(View) || nullptr == pPreview)
		return;
	float4x4_t Pivot;
	if (!CEffectV2Object::Resolve_TargetPivot(View, m_strPivotBone, m_ePivotRotation, Pivot))
	{
		m_strAttachStatus = "Snap failed: bone not found " + m_strPivotBone;
		return;
	}
	pPreview->Clear_FollowTarget();
	pPreview->PivotWorld() = Pivot;
}

bool_t Client::CEffect_Tool_V2::Ensure_ValtanTree()
{
	if (m_bValtanTreeLoaded)
		return !m_ValtanPatterns.empty();
	m_bValtanTreeLoaded = true;
	m_ValtanPatterns.clear();
	m_iValtanPatternSelection = -1;
	VALTAN_PATTERN_TREE_VIEW Staged;
	if (!CValtanPatternTree::Load(Staged, m_strValtanTreeStatus))
	{
		m_ValtanTree = {};
		return false;
	}
	m_ValtanTree = std::move(Staged);
	for (const VALTAN_PATTERN_VIEW& Pattern : m_ValtanTree.Gimmicks)
	{
		if (Pattern.bAuthoringMasterManaged && !Pattern.Stages.empty())
			m_ValtanPatterns.push_back(&Pattern);
	}
	for (const VALTAN_PATTERN_VIEW& Pattern : m_ValtanTree.Rotation)
	{
		if (Pattern.bAuthoringMasterManaged && !Pattern.Stages.empty())
			m_ValtanPatterns.push_back(&Pattern);
	}
	m_strValtanTreeStatus = "Loaded " + std::to_string(m_ValtanPatterns.size()) +
		" master pattern(s).";
	return !m_ValtanPatterns.empty();
}

bool_t Client::CEffect_Tool_V2::Build_ValtanTimeline(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	m_ValtanTimeline.clear();
	m_iValtanTimelineDurationMs = 0u;
	m_iValtanTimelineStage = static_cast<size_t>(-1);
	std::vector<const VALTAN_STAGE_VIEW*> Path;
	std::string strError;
	std::string strPathNote;
	if (!CValtanPatternTree::Build_PreviewStagePath(Pattern, ePath, Path, strError))
	{
		/* A cyclic branch graph (catch loops) has no linear preview path. The
		   authored stage order still names every Server stage once, which is
		   all a stage-keyed binding needs. */
		Path.clear();
		for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			Path.push_back(&Stage);
		strPathNote = " (no linear path: " + strError + "; authored stage order)";
	}
	uint64_t iCursorMs = 0u;
	for (const VALTAN_STAGE_VIEW* pStage : Path)
	{
		if (nullptr == pStage)
			continue;
		LostArk::Shared::WORLD_ENTITY_ACTION eAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::END;
		if (!Resolve_ValtanStageAction(pStage->strStageKind, eAction))
		{
			m_strAttachStatus = "Stage kind has no arena action: " + pStage->strStageKind;
			m_ValtanTimeline.clear();
			return false;
		}
		VALTAN_TIMELINE_STAGE Stage;
		Stage.strActionId = pStage->strActionId;
		Stage.strStageId = pStage->strStageId;
		Stage.strStageKind = pStage->strStageKind;
		Stage.iStartMs = static_cast<uint32_t>(iCursorMs);
		Stage.iDurationMs = pStage->iDurationMs;
		iCursorMs += pStage->iDurationMs;
		m_ValtanTimeline.push_back(std::move(Stage));
	}
	if (m_ValtanTimeline.empty() || 0u == iCursorMs)
	{
		m_strAttachStatus = "Pattern has no timed stage on this path.";
		m_ValtanTimeline.clear();
		return false;
	}
	m_iValtanTimelineDurationMs = static_cast<uint32_t>(iCursorMs);
	m_strAttachStatus = "Pattern timeline " + std::to_string(m_iValtanTimelineDurationMs) +
		" ms over " + std::to_string(m_ValtanTimeline.size()) + " stage(s)" + strPathNote;
	return true;
}

bool_t Client::CEffect_Tool_V2::Try_ResolveValtanTimelineStage(
	const uint32_t iTimelineMs,
	size_t& iOutStage,
	uint32_t& iOutStageOffsetMs) const
{
	for (size_t iStage = 0u; iStage < m_ValtanTimeline.size(); ++iStage)
	{
		const VALTAN_TIMELINE_STAGE& Stage = m_ValtanTimeline[iStage];
		const bool_t bLast = iStage + 1u == m_ValtanTimeline.size();
		if (iTimelineMs >= Stage.iStartMs &&
			(iTimelineMs < Stage.iStartMs + Stage.iDurationMs || bLast))
		{
			iOutStage = iStage;
			iOutStageOffsetMs = (std::min)(iTimelineMs - Stage.iStartMs, Stage.iDurationMs);
			return true;
		}
	}
	return false;
}

bool_t Client::CEffect_Tool_V2::Apply_ValtanTimeline(const f32_t fSeconds, const bool_t bForceEdge)
{
	const std::shared_ptr<CGameObject> pOwner = m_Target.pOwner.lock();
	if (EFFECT_V2_TARGET_KIND::VALTAN != m_Target.eKind || nullptr == pOwner ||
		m_ValtanTimeline.empty() || !std::isfinite(fSeconds))
	{
		return false;
	}
	const std::shared_ptr<CValtan> pValtan = std::static_pointer_cast<CValtan>(pOwner);
	const uint32_t iTimelineMs = static_cast<uint32_t>(
		std::lround((std::max)(0.f, fSeconds) * 1000.f));
	size_t iStage = 0u;
	uint32_t iOffsetMs = 0u;
	if (!Try_ResolveValtanTimelineStage(iTimelineMs, iStage, iOffsetMs))
		return false;
	const VALTAN_TIMELINE_STAGE& Stage = m_ValtanTimeline[iStage];
	LostArk::Shared::WORLD_ENTITY_ACTION eAction =
		LostArk::Shared::WORLD_ENTITY_ACTION::END;
	if (!Resolve_ValtanStageAction(Stage.strStageKind, eAction))
		return false;
	const bool_t bEdge = bForceEdge || iStage != m_iValtanTimelineStage;
	if (!pValtan->Apply_LocalPatternPresentationSample(
		eAction, Stage.strActionId, static_cast<f32_t>(iOffsetMs) * 0.001f, bEdge))
	{
		return false;
	}
	if (const std::shared_ptr<Engine::CModel> pModel = pValtan->Get_BodyModel())
		pModel->Set_AnimPaused(true);
	m_iValtanTimelineStage = iStage;
	return true;
}

void Client::CEffect_Tool_V2::Update_ValtanTimeline(const f32_t fTimeDelta)
{
	if (!m_bValtanTimelineActive)
		return;
	if (EFFECT_V2_TARGET_KIND::VALTAN != m_Target.eKind || !m_Target.Is_Valid() ||
		m_ValtanTimeline.empty())
	{
		Stop_ValtanTimeline();
		return;
	}
	const f32_t fDuration = static_cast<f32_t>(m_iValtanTimelineDurationMs) * 0.001f;
	const f32_t fPrevious = m_fValtanTimelineSeconds;
	if (!m_bValtanTimelinePaused && std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
	{
		m_fValtanTimelineSeconds += fTimeDelta;
		if (m_fValtanTimelineSeconds >= fDuration)
		{
			if (m_bValtanTimelineLoop)
				m_fValtanTimelineSeconds = 0.f;
			else
			{
				m_fValtanTimelineSeconds = fDuration;
				m_bValtanTimelinePaused = true;
			}
		}
	}
	Apply_ValtanTimeline(m_fValtanTimelineSeconds, false);
	if (m_bValtanTimelinePaused)
		return;
	const f32_t fSpawn = static_cast<f32_t>(m_iValtanSpawnTimelineMs) * 0.001f;
	const bool_t bWrapped = m_fValtanTimelineSeconds < fPrevious;
	const bool_t bCrossed = bWrapped ?
		m_fValtanTimelineSeconds >= fSpawn :
		(fPrevious < fSpawn && m_fValtanTimelineSeconds >= fSpawn);
	if (bCrossed)
	{
		if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
		{
			if (PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode)
				Snap_PivotToTarget();
			pPreview->Restart();
		}
	}
}

void Client::CEffect_Tool_V2::Stop_ValtanTimeline()
{
	const bool_t bWasActive = m_bValtanTimelineActive;
	m_bValtanTimelineActive = false;
	m_bValtanTimelinePaused = false;
	m_fValtanTimelineSeconds = 0.f;
	m_iValtanTimelineStage = static_cast<size_t>(-1);
	if (!bWasActive)
		return;
	const std::shared_ptr<CGameObject> pOwner = m_Target.pOwner.lock();
	if (EFFECT_V2_TARGET_KIND::VALTAN != m_Target.eKind || nullptr == pOwner)
		return;
	std::static_pointer_cast<CValtan>(pOwner)->Reset_LocalPatternPresentationSample();
	const BOSS_ACTOR_ENTRY* pBoss = CActorCatalog::Find_Boss(VALTAN_TARGET_ARCHETYPE_ID);
	if (nullptr != pBoss && !pBoss->presentationClips.idle.empty())
		Play_TargetClip(pBoss->presentationClips.idle.c_str(), m_bTargetClipLoop);
}

bool_t Client::CEffect_Tool_V2::Try_LocateValtanStage(
	const std::string& strActionId,
	const uint32_t iOffsetMs)
{
	if (EFFECT_V2_TARGET_KIND::VALTAN != m_Target.eKind || !Ensure_ValtanTree())
		return false;
	constexpr VALTAN_PATTERN_PREVIEW_PATH Paths[] = {
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
		VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
		VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK
	};
	for (size_t iPattern = 0u; iPattern < m_ValtanPatterns.size(); ++iPattern)
	{
		const VALTAN_PATTERN_VIEW& Pattern = *m_ValtanPatterns[iPattern];
		const bool_t bOwnsStage = std::any_of(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&strActionId](const VALTAN_STAGE_VIEW& Stage)
			{ return Stage.strActionId == strActionId; });
		if (!bOwnsStage)
			continue;
		for (const VALTAN_PATTERN_PREVIEW_PATH ePath : Paths)
		{
			if (!Build_ValtanTimeline(Pattern, ePath))
				continue;
			for (const VALTAN_TIMELINE_STAGE& Stage : m_ValtanTimeline)
			{
				if (Stage.strActionId != strActionId)
					continue;
				m_iValtanPatternSelection = static_cast<int32_t>(iPattern);
#ifdef _DEBUG
				if (CMainApp* const pApp = CMainApp::Get_Active())
					(void)pApp->Debug_SelectCompletePlayPattern(
						Pattern.strPatternId);
#endif
				m_eValtanPath = ePath;
				m_iValtanSpawnTimelineMs = static_cast<int32_t>(
					Stage.iStartMs + (std::min)(iOffsetMs, Stage.iDurationMs));
				m_bValtanTimelineActive = true;
				m_bValtanTimelinePaused = true;
				m_fValtanTimelineSeconds = static_cast<f32_t>(m_iValtanSpawnTimelineMs) * 0.001f;
				Apply_ValtanTimeline(m_fValtanTimelineSeconds, true);
				return true;
			}
		}
	}
	m_ValtanTimeline.clear();
	m_strAttachStatus = "No master pattern stage owns actionId " + strActionId;
	return false;
}

void Client::CEffect_Tool_V2::Render_ValtanPatternSection()
{
	if (EFFECT_V2_TARGET_KIND::VALTAN != m_Target.eKind || !m_Target.Is_Valid())
		return;
	ImGui::SeparatorText("Pattern Timeline (Valtan master)");
	if (!Ensure_ValtanTree())
	{
		ImGui::TextWrapped("%s", m_strValtanTreeStatus.c_str());
		ImGui::SameLine();
		if (ImGui::SmallButton("Retry"))
			m_bValtanTreeLoaded = false;
		return;
	}

#ifdef _DEBUG
	if (CMainApp* const pApp = CMainApp::Get_Active())
	{
		const std::string& strSharedPatternId =
			pApp->Debug_GetSelectedCompletePlayPatternId();
		if (!strSharedPatternId.empty())
		{
			const auto Found = std::find_if(
				m_ValtanPatterns.begin(), m_ValtanPatterns.end(),
				[&strSharedPatternId](const VALTAN_PATTERN_VIEW* pPattern)
				{
					return nullptr != pPattern &&
						pPattern->strPatternId == strSharedPatternId;
				});
			m_iValtanPatternSelection =
				m_ValtanPatterns.end() == Found ? -1 :
				static_cast<int32_t>(
					std::distance(m_ValtanPatterns.begin(), Found));
		}
	}
#endif
	const char* pPatternLabel =
		m_iValtanPatternSelection >= 0 &&
		m_iValtanPatternSelection < static_cast<int32_t>(m_ValtanPatterns.size()) ?
		m_ValtanPatterns[static_cast<size_t>(m_iValtanPatternSelection)]->strPatternId.c_str() :
		"(select)";
	if (ImGui::BeginCombo("Pattern", pPatternLabel))
	{
		for (int32_t iIndex = 0; iIndex < static_cast<int32_t>(m_ValtanPatterns.size()); ++iIndex)
		{
			const VALTAN_PATTERN_VIEW& Pattern = *m_ValtanPatterns[static_cast<size_t>(iIndex)];
			const std::string strLabel = Pattern.strPatternId + "  (" +
				std::to_string(Pattern.Stages.size()) + " stages)";
			if (ImGui::Selectable(strLabel.c_str(), iIndex == m_iValtanPatternSelection))
			{
				m_iValtanPatternSelection = iIndex;
#ifdef _DEBUG
				if (CMainApp* const pApp = CMainApp::Get_Active())
					(void)pApp->Debug_SelectCompletePlayPattern(
						Pattern.strPatternId);
#endif
				if (m_bValtanTimelineActive)
					Stop_ValtanTimeline();
			}
		}
		ImGui::EndCombo();
	}
	int32_t iPath = static_cast<int32_t>(m_eValtanPath);
	if (ImGui::Combo("Branch Path", &iPath, "NORMAL\0WALL_GROGGY\0PART_BREAK\0"))
	{
		m_eValtanPath = static_cast<VALTAN_PATTERN_PREVIEW_PATH>(iPath);
		if (m_bValtanTimelineActive)
			Stop_ValtanTimeline();
	}
	const bool_t bHasPattern = m_iValtanPatternSelection >= 0 &&
		m_iValtanPatternSelection < static_cast<int32_t>(m_ValtanPatterns.size());
	ImGui::BeginDisabled(!bHasPattern);
	if (ImGui::Button(m_bValtanTimelineActive ?
		(m_bValtanTimelinePaused ? "Resume Offline" : "Pause Offline") :
		"Pattern Offline"))
	{
		if (!m_bValtanTimelineActive)
		{
			if (Build_ValtanTimeline(
				*m_ValtanPatterns[static_cast<size_t>(m_iValtanPatternSelection)], m_eValtanPath))
			{
				m_bValtanTimelineActive = true;
				m_bValtanTimelinePaused = false;
				m_fValtanTimelineSeconds = 0.f;
				m_iValtanSpawnTimelineMs = (std::min)(
					m_iValtanSpawnTimelineMs, static_cast<int32_t>(m_iValtanTimelineDurationMs));
				Apply_ValtanTimeline(0.f, true);
				if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
					nullptr != pPreview && 0 == m_iValtanSpawnTimelineMs)
				{
					pPreview->Restart();
				}
			}
		}
		else
			m_bValtanTimelinePaused = !m_bValtanTimelinePaused;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bHasPattern);
	if (ImGui::Button("Complete Play (Server/Arena)"))
		(void)Try_PlayValtanServerPattern();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bValtanTimelineActive);
	if (ImGui::Button("Stop"))
		Stop_ValtanTimeline();
	ImGui::SameLine();
	if (ImGui::Button("Restart Pattern"))
	{
		m_fValtanTimelineSeconds = 0.f;
		m_bValtanTimelinePaused = false;
		Apply_ValtanTimeline(0.f, true);
		if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
			nullptr != pPreview && 0 == m_iValtanSpawnTimelineMs)
		{
			pPreview->Restart();
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Checkbox("Loop##Pattern", &m_bValtanTimelineLoop);
	if (!m_strValtanServerPatternStatus.empty())
		ImGui::TextWrapped("Server: %s", m_strValtanServerPatternStatus.c_str());
	ImGui::TextDisabled(
		"Arena playback uses the saved V2 document/bindings on the replicated boss. Unsaved tuning remains in the local model preview.");
	if (!m_bValtanTimelineActive)
	{
		ImGui::TextDisabled("Play a pattern to scrub its stages and place the spawn point.");
		return;
	}

	int32_t iTimelineMs = static_cast<int32_t>(std::lround(m_fValtanTimelineSeconds * 1000.f));
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::SliderInt("##PatternScrub", &iTimelineMs, 0,
		static_cast<int32_t>(m_iValtanTimelineDurationMs), "%d ms"))
	{
		m_bValtanTimelinePaused = true;
		m_fValtanTimelineSeconds = static_cast<f32_t>(iTimelineMs) * 0.001f;
		Apply_ValtanTimeline(m_fValtanTimelineSeconds, true);
	}
	size_t iStage = 0u;
	uint32_t iStageOffsetMs = 0u;
	if (Try_ResolveValtanTimelineStage(
		static_cast<uint32_t>((std::max)(0, iTimelineMs)), iStage, iStageOffsetMs))
	{
		const VALTAN_TIMELINE_STAGE& Stage = m_ValtanTimeline[iStage];
		ImGui::Text("Stage %zu/%zu  %s [%s]  +%u / %u ms",
			iStage + 1u, m_ValtanTimeline.size(), Stage.strActionId.c_str(),
			Stage.strStageKind.c_str(), iStageOffsetMs, Stage.iDurationMs);
	}
	if (ImGui::Button("Spawn Here"))
		m_iValtanSpawnTimelineMs = (std::max)(0, iTimelineMs);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(140.f);
	ImGui::InputInt("Spawn Point (ms)", &m_iValtanSpawnTimelineMs);
	m_iValtanSpawnTimelineMs = (std::max)(0,
		(std::min)(m_iValtanSpawnTimelineMs, static_cast<int32_t>(m_iValtanTimelineDurationMs)));
	size_t iSpawnStage = 0u;
	uint32_t iSpawnOffsetMs = 0u;
	if (Try_ResolveValtanTimelineStage(
		static_cast<uint32_t>(m_iValtanSpawnTimelineMs), iSpawnStage, iSpawnOffsetMs))
	{
		ImGui::TextDisabled("Binding will store stage %s +%u ms (Server stage clock).",
			m_ValtanTimeline[iSpawnStage].strActionId.c_str(), iSpawnOffsetMs);
	}
}

bool_t Client::CEffect_Tool_V2::Try_PlayValtanServerPattern()
{
	if (!Ensure_ValtanTree() || m_iValtanPatternSelection < 0 ||
		m_iValtanPatternSelection >= static_cast<int32_t>(m_ValtanPatterns.size()))
	{
		m_strValtanServerPatternStatus =
			"Select one admitted Valtan pattern before Server playback.";
		return false;
	}
	const VALTAN_PATTERN_VIEW& Pattern =
		*m_ValtanPatterns[static_cast<size_t>(m_iValtanPatternSelection)];
#ifdef _DEBUG
	CMainApp* const pApp = CMainApp::Get_Active();
	if (nullptr == pApp)
	{
		m_strValtanServerPatternStatus =
			"Complete Play workspace is unavailable.";
		return false;
	}
	if (!pApp->Debug_SelectCompletePlayPattern(Pattern.strPatternId))
	{
		m_strValtanServerPatternStatus =
			"The selected V2 pattern is not in the shared Server inventory.";
		return false;
	}
	return pApp->Debug_CompletePlaySelected(
		m_strValtanServerPatternStatus);
#else
	m_strValtanServerPatternStatus =
		"Complete Play is available only in a Debug authoring build.";
	return false;
#endif
}

void Client::CEffect_Tool_V2::Update_ValtanServerPatternStatus()
{
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& snapshot =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	if (snapshot.strPatternId.empty() ||
		m_iValtanPatternSelection < 0 ||
		m_iValtanPatternSelection >=
			static_cast<int32_t>(m_ValtanPatterns.size()) ||
		nullptr == m_ValtanPatterns[
			static_cast<size_t>(m_iValtanPatternSelection)] ||
		snapshot.strPatternId != m_ValtanPatterns[
			static_cast<size_t>(m_iValtanPatternSelection)]->strPatternId)
	{
		return;
	}
	m_strValtanServerPatternStatus =
		std::string(Describe_ValtanPatternAuditionState(snapshot.eState)) +
		" | " + snapshot.strStatus;
}

bool_t Client::CEffect_Tool_V2::Load_Bindings(const std::string& strArchetypeId)
{
	m_Bindings.clear();
	std::error_code Error;
	const std::filesystem::path Target = CEffectV2Document::Binding_Path(strArchetypeId);
	if (Target.empty() || !std::filesystem::is_regular_file(Target, Error))
	{
		m_strAttachStatus = "No bindings yet for " + strArchetypeId + ".";
		return true;
	}
	std::string strError;
	std::vector<EFFECT_BINDING> Bindings;
	if (!CEffectV2Document::Load_BindingsFile(strArchetypeId, Bindings, strError))
	{
		m_strAttachStatus = "Bindings rejected (" + strArchetypeId + "): " + strError;
		return false;
	}
	m_Bindings = std::move(Bindings);
	m_strAttachStatus = "Loaded " + std::to_string(m_Bindings.size()) +
		" binding(s) for " + strArchetypeId + ".";
	return true;
}

bool_t Client::CEffect_Tool_V2::Save_Bindings()
{
	if (m_strTargetArchetypeId.empty())
	{
		m_strAttachStatus = "Spawn a target first.";
		return false;
	}
	std::string strError;
	if (!CEffectV2Document::Write_AtomicFile(
		CEffectV2Document::Binding_Path(m_strTargetArchetypeId),
		CEffectV2Document::Serialize_Bindings(m_strTargetArchetypeId, m_Bindings), strError))
	{
		m_strAttachStatus = strError;
		return false;
	}
	CEffectV2Runtime::Invalidate_Caches();
	m_strAttachStatus = "Saved " + m_strTargetArchetypeId + ".effectv2bindings.json";
	return true;
}

void Client::CEffect_Tool_V2::Render_AttachWindow()
{
	if (!m_bAttachWindowOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(460.f, 640.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Attach v2", &m_bAttachWindowOpen))
	{
		ImGui::End();
		return;
	}
	EFFECT_V2_TARGET_VIEW View;
	const bool_t bHasTarget = Resolve_TargetView(View);
	const std::shared_ptr<Engine::CModel> pModel = bHasTarget ? View.pModel : nullptr;

	ImGui::SeparatorText("Target (NPC archetype / Valtan)");
	const std::vector<NPC_ACTOR_ENTRY>& Npcs = CActorCatalog::Get_Npcs();
	if (ImGui::BeginCombo("Archetype",
		m_strSelectedArchetypeId.empty() ? "(select)" : m_strSelectedArchetypeId.c_str()))
	{
		if (ImGui::Selectable("BOSS_VALTAN  (Valtan)",
			VALTAN_TARGET_ARCHETYPE_ID == m_strSelectedArchetypeId))
		{
			m_strSelectedArchetypeId = VALTAN_TARGET_ARCHETYPE_ID;
		}
		ImGui::Separator();
		for (const NPC_ACTOR_ENTRY& Entry : Npcs)
		{
			if (Entry.runtimeStatus != "supported")
				continue;
			const std::string strLabel = Entry.archetypeId + "  (" +
				std::filesystem::path(Entry.modelAssetId).stem().string() + ")";
			if (ImGui::Selectable(strLabel.c_str(), Entry.archetypeId == m_strSelectedArchetypeId))
				m_strSelectedArchetypeId = Entry.archetypeId;
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(m_strSelectedArchetypeId.empty());
	if (ImGui::Button("Spawn Target"))
		Spawn_Target(m_strSelectedArchetypeId);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bHasTarget);
	if (ImGui::Button("Despawn"))
		Despawn_Target();
	ImGui::EndDisabled();
	if (bHasTarget)
	{
		ImGui::Text("Live: %s", m_strTargetArchetypeId.c_str());
		if (ImGui::Checkbox("Runtime spawns on target", &m_bRuntimeOnTarget))
		{
			CEffectV2Runtime::Set_Ignored(m_Target, !m_bRuntimeOnTarget);
			if (m_bRuntimeOnTarget && nullptr != pModel)
			{
				const char_t* pClip = pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex());
				if (nullptr != pClip)
					CEffectV2Runtime::Notify_Clip(m_Target, pClip);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Let CEffectV2Runtime apply the saved bindings to this tool target (in-game behaviour check). Hide the preview to avoid doubles.");
		float3_t vPosition = m_vTargetPosition;
		f32_t fYaw = m_fTargetYawDegrees;
		if (ImGui::DragFloat3("Target Position", &vPosition.x, 0.05f))
			Move_Target(vPosition, fYaw);
		if (ImGui::DragFloat("Target Yaw (deg)", &fYaw, 1.f, -360.f, 360.f))
			Move_Target(vPosition, fYaw);
		if (ImGui::Button("Beside Character"))
		{
			if (const std::shared_ptr<CCharacter> pCharacter =
				CAnimationTargetService::Resolve_SceneCharacter();
				nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
			{
				XMStoreFloat3(&vPosition,
					pCharacter->Get_Transform()->Get_State(STATE::POSITION) +
					XMVectorSet(2.5f, 0.f, 0.f, 0.f));
				Move_Target(vPosition, fYaw);
			}
		}
	}

	if (nullptr != pModel && 0u < pModel->Get_NumAnimations())
	{
		ImGui::SeparatorText("Target Playback");
		const uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
		const char_t* pCurrentName = pModel->Get_AnimationName(iCurrent);
		if (ImGui::BeginCombo("Target Clip", nullptr != pCurrentName ? pCurrentName : "(none)"))
		{
			for (uint32_t iClip = 0u; iClip < pModel->Get_NumAnimations(); ++iClip)
			{
				const char_t* pName = pModel->Get_AnimationName(iClip);
				if (nullptr == pName)
					continue;
				if (ImGui::Selectable(pName, iClip == iCurrent))
					Play_TargetClip(pName, m_bTargetClipLoop);
			}
			ImGui::EndCombo();
		}
		f32_t fPosition = 0.f;
		f32_t fDuration = 0.f;
		const bool_t bHasTrack =
			pModel->Get_AnimationProgress(iCurrent, fPosition, fDuration) && fDuration > 0.f;
		const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iCurrent);
		if (bHasTrack)
		{
			f32_t fScrub = fPosition;
			char_t szFormat[64]{};
			std::snprintf(szFormat, sizeof(szFormat), "frame %%.1f / %.0f", fDuration);
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::SliderFloat("##TargetScrub", &fScrub, 0.f, fDuration, szFormat))
			{
				pModel->Set_AnimPaused(true);
				pModel->Set_AnimTrackPosition(iCurrent, fScrub);
			}
			if (fTickPerSecond > 0.f)
				ImGui::TextDisabled("%.2f / %.2f s", fPosition / fTickPerSecond, fDuration / fTickPerSecond);
		}
		const bool_t bPaused = pModel->Is_AnimPaused();
		if (ImGui::Button(bPaused ? "Play" : "Pause"))
			pModel->Set_AnimPaused(!bPaused);
		ImGui::SameLine();
		ImGui::BeginDisabled(!bHasTrack);
		if (ImGui::Button("< Frame"))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrent, (std::max)(0.f, fPosition - 1.f));
		}
		ImGui::SameLine();
		if (ImGui::Button("Frame >"))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrent, (std::min)(fDuration, fPosition + 1.f));
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Restart Clip"))
		{
			pModel->Set_AnimTrackPosition(iCurrent, 0.f);
			m_fTargetLastClipSeconds = -1.f;
			if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
				pPreview->Restart();
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Loop", &m_bTargetClipLoop))
			pModel->Set_Animation(iCurrent, m_bTargetClipLoop);
	}

	Render_ValtanPatternSection();

	ImGui::SeparatorText("Effect Pivot");
	int32_t iMode = static_cast<int32_t>(m_ePivotMode);
	if (ImGui::Combo("Pivot Mode", &iMode, "World\0Target Bone (follow)\0Target Bone (snap once)\0"))
	{
		m_ePivotMode = static_cast<PIVOT_MODE>(iMode);
		if (PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode)
			Snap_PivotToTarget();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("follow: pivot tracks the bone every frame. snap once: pivot is captured from the bone at spawn/restart and then stays put (ground decals, summoning circles).");
	if (PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode)
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Snap Now"))
			Snap_PivotToTarget();
	}
	ImGui::BeginDisabled(m_TargetBoneNames.empty());
	if (ImGui::BeginCombo("Pivot Bone", m_strPivotBone.empty() ? "(none)" : m_strPivotBone.c_str()))
	{
		for (const std::string& strBone : m_TargetBoneNames)
		{
			if (ImGui::Selectable(strBone.c_str(), strBone == m_strPivotBone))
				m_strPivotBone = strBone;
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();
	int32_t iRotation = static_cast<int32_t>(m_ePivotRotation);
	if (ImGui::Combo("Pivot Rotation", &iRotation, "Bone\0Target Yaw\0World\0"))
		m_ePivotRotation = static_cast<PIVOT_ROTATION>(iRotation);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Bone: inherit bone orientation. Target Yaw: bone position + target facing (default). World: bone position only.");
	if (PIVOT_MODE::TARGET_BONE == m_ePivotMode && !bHasTarget)
		ImGui::TextDisabled("Spawn a target to follow a bone.");
	ImGui::InputInt("Spawn Frame (30 fps)", &m_iSpawnFrame);
	m_iSpawnFrame = (std::max)(0, (std::min)(m_iSpawnFrame, 18000));
	ImGui::TextDisabled("Effect restarts when the target clip passes the spawn frame (= %u ms).",
		static_cast<uint32_t>(static_cast<f32_t>(m_iSpawnFrame) * 1000.f / BINDING_FRAME_RATE));

	ImGui::SeparatorText("Bindings (Data/Effects/V2/Bindings)");
	const char_t* pClipForBinding =
		nullptr != pModel ? pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex()) : nullptr;
	std::string strStageForBinding;
	uint32_t iStageOffsetForBinding = 0u;
	if (m_bValtanTimelineActive)
	{
		size_t iSpawnStage = 0u;
		if (Try_ResolveValtanTimelineStage(
			static_cast<uint32_t>((std::max)(0, m_iValtanSpawnTimelineMs)),
			iSpawnStage, iStageOffsetForBinding))
		{
			strStageForBinding = m_ValtanTimeline[iSpawnStage].strActionId;
		}
	}
	const bool_t bStageBinding = !strStageForBinding.empty();
	if (!m_bGroupsScanned)
		Scan_Groups();
	const std::string strBindAsLabel =
		m_strBindingGroupId.empty() ? "(effect document)" : "group: " + m_strBindingGroupId;
	if (ImGui::BeginCombo("Bind As", strBindAsLabel.c_str()))
	{
		if (ImGui::Selectable("(effect document)", m_strBindingGroupId.empty()))
			m_strBindingGroupId.clear();
		for (const std::string& strGroup : m_Groups)
		{
			if (ImGui::Selectable(strGroup.c_str(), strGroup == m_strBindingGroupId))
				m_strBindingGroupId = strGroup;
		}
		ImGui::EndCombo();
	}
	const bool_t bGroupBinding = !m_strBindingGroupId.empty();
	ImGui::Text("%s: %s | %s: %s | Bone: %s",
		bGroupBinding ? "Group" : "Effect",
		bGroupBinding ? m_strBindingGroupId.c_str() :
			('\0' == m_szEffectId[0] ? "(none)" : m_szEffectId),
		bStageBinding ? "Stage" : "Clip",
		bStageBinding ? strStageForBinding.c_str() :
			(nullptr != pClipForBinding ? pClipForBinding : "(none)"),
		m_strPivotBone.empty() ? "(none)" : m_strPivotBone.c_str());
	ImGui::BeginDisabled((!bGroupBinding && '\0' == m_szEffectId[0]) || !bHasTarget ||
		(!bStageBinding && nullptr == pClipForBinding));
	if (ImGui::Button("Add / Update Binding"))
	{
		EFFECT_BINDING Binding;
		if (bGroupBinding)
			Binding.strGroupId = m_strBindingGroupId;
		else
			Binding.strEffectId = m_szEffectId;
		if (bStageBinding)
		{
			Binding.strStage = strStageForBinding;
			Binding.iStartMs = iStageOffsetForBinding;
		}
		else
		{
			Binding.strClip = pClipForBinding;
			Binding.iStartMs = static_cast<uint32_t>(
				static_cast<f32_t>(m_iSpawnFrame) * 1000.f / BINDING_FRAME_RATE);
		}
		Binding.strBone = PIVOT_MODE::WORLD != m_ePivotMode ? m_strPivotBone : std::string();
		Binding.bFollowBone = PIVOT_MODE::TARGET_BONE == m_ePivotMode;
		Binding.eRotation = m_ePivotRotation;
		bool_t bReplaced = false;
		for (EFFECT_BINDING& Existing : m_Bindings)
		{
			if (Existing.strEffectId == Binding.strEffectId &&
				Existing.strGroupId == Binding.strGroupId &&
				Existing.strClip == Binding.strClip && Existing.strStage == Binding.strStage)
			{
				Binding.bStopWithClip = Existing.bStopWithClip;
				Existing = Binding;
				bReplaced = true;
				break;
			}
		}
		if (!bReplaced)
			m_Bindings.push_back(Binding);
		m_strAttachStatus = bReplaced ? "Binding updated (unsaved)." : "Binding added (unsaved).";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_strTargetArchetypeId.empty());
	if (ImGui::Button("Save Bindings"))
		Save_Bindings();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
		Load_Bindings(m_strTargetArchetypeId);
	ImGui::EndDisabled();
	if (m_Bindings.empty())
		ImGui::TextDisabled("No bindings.");
	for (size_t iIndex = 0u; iIndex < m_Bindings.size(); ++iIndex)
	{
		EFFECT_BINDING& Binding = m_Bindings[iIndex];
		ImGui::PushID(static_cast<int32_t>(iIndex));
		char_t szRow[256]{};
		const std::string strRowName = Binding.strGroupId.empty() ?
			Binding.strEffectId : "group:" + Binding.strGroupId;
		std::snprintf(szRow, sizeof(szRow), "%s @ %s%s +%ums %s%s",
			strRowName.c_str(),
			Binding.strStage.empty() ? "" : "stage:",
			Binding.strStage.empty() ? Binding.strClip.c_str() : Binding.strStage.c_str(),
			Binding.iStartMs,
			Binding.strBone.empty() ? "(world)" : Binding.strBone.c_str(),
			Binding.bFollowBone ? "" : " [snap once]");
		/* The row spans the full line, so the overlapping SameLine checkbox and
		Remove button need AllowOverlap or the row swallows their clicks. */
		if (ImGui::Selectable(szRow, false, ImGuiSelectableFlags_AllowOverlap))
		{
			if (Binding.strGroupId.empty())
			{
				m_strBindingGroupId.clear();
				std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", Binding.strEffectId.c_str());
			}
			else
			{
				m_strBindingGroupId = Binding.strGroupId;
				m_bGroupWindowOpen = true;
				Load_Group(Binding.strGroupId);
			}
			m_ePivotMode = Binding.strBone.empty() ? PIVOT_MODE::WORLD :
				(Binding.bFollowBone ? PIVOT_MODE::TARGET_BONE : PIVOT_MODE::TARGET_BONE_FIXED);
			m_ePivotRotation = Binding.eRotation;
			if (!Binding.strBone.empty())
				m_strPivotBone = Binding.strBone;
			if (PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode)
				Snap_PivotToTarget();
			if (!Binding.strStage.empty())
			{
				Try_LocateValtanStage(Binding.strStage, Binding.iStartMs);
			}
			else
			{
				if (m_bValtanTimelineActive)
					Stop_ValtanTimeline();
				m_iSpawnFrame = static_cast<int32_t>(
					static_cast<f32_t>(Binding.iStartMs) * BINDING_FRAME_RATE / 1000.f + 0.5f);
				Play_TargetClip(Binding.strClip.c_str(), m_bTargetClipLoop);
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Stop w/ clip", &Binding.bStopWithClip);
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove"))
		{
			m_Bindings.erase(m_Bindings.begin() + static_cast<std::ptrdiff_t>(iIndex));
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
	}
	if (!m_strAttachStatus.empty())
		ImGui::TextWrapped("%s", m_strAttachStatus.c_str());
	ImGui::End();
}

void Client::CEffect_Tool_V2::Scan_Groups()
{
	m_bGroupsScanned = true;
	m_Groups.clear();
	std::error_code Error;
	const std::filesystem::path Directory = CEffectV2Document::Group_Directory();
	if (Directory.empty() || !std::filesystem::is_directory(Directory, Error))
		return;
	for (const std::filesystem::directory_entry& Entry :
		std::filesystem::directory_iterator(Directory, Error))
	{
		if (!Entry.is_regular_file(Error))
			continue;
		const std::string strName = Entry.path().filename().string();
		constexpr const char* SUFFIX = ".effectv2group.json";
		const size_t iSuffix = std::strlen(SUFFIX);
		if (strName.size() <= iSuffix ||
			strName.compare(strName.size() - iSuffix, iSuffix, SUFFIX) != 0)
			continue;
		m_Groups.push_back(strName.substr(0u, strName.size() - iSuffix));
	}
	std::sort(m_Groups.begin(), m_Groups.end());
}

bool_t Client::CEffect_Tool_V2::Load_Group(const std::string& strGroupId)
{
	EFFECT_V2_GROUP Group;
	std::string strError;
	if (!CEffectV2Document::Load_GroupFile(strGroupId, Group, strError))
	{
		m_strGroupStatus = "Load rejected (" + strGroupId + "): " + strError;
		return false;
	}
	Stop_GroupPreview();
	m_Group = std::move(Group);
	std::snprintf(m_szGroupId, sizeof(m_szGroupId), "%s", strGroupId.c_str());
	m_strGroupStatus = "Loaded " + strGroupId;
	return true;
}

bool_t Client::CEffect_Tool_V2::Save_Group()
{
	const std::string strGroupId = m_szGroupId;
	if (!CEffectV2Document::Is_ValidEffectId(strGroupId))
	{
		m_strGroupStatus = "Group ID must be 1-80 chars of [A-Za-z0-9._-].";
		return false;
	}
	if (m_Group.Children.empty())
	{
		m_strGroupStatus = "Add at least one child before saving.";
		return false;
	}
	for (const EFFECT_V2_GROUP_CHILD& Child : m_Group.Children)
	{
		if (!CEffectV2Document::Is_ValidEffectId(Child.strEffectId) ||
			Child.strEffectId == strGroupId)
		{
			m_strGroupStatus = "Every child needs an effect ID different from the group.";
			return false;
		}
	}
	m_Group.strGroupId = strGroupId;
	std::error_code Error;
	std::filesystem::create_directories(CEffectV2Document::Group_Directory(), Error);
	std::string strError;
	if (!CEffectV2Document::Write_AtomicFile(
		CEffectV2Document::Group_Path(strGroupId),
		CEffectV2Document::Serialize_Group(m_Group), strError))
	{
		m_strGroupStatus = strError;
		return false;
	}
	CEffectV2Runtime::Invalidate_Caches();
	m_bGroupsScanned = false;
	m_strGroupStatus = "Saved " + strGroupId + ".effectv2group.json";
	return true;
}

bool_t Client::CEffect_Tool_V2::Play_GroupPreview()
{
	Stop_GroupPreview();
	const std::string strGroupId = m_szGroupId;
	std::error_code Error;
	if (!std::filesystem::is_regular_file(CEffectV2Document::Group_Path(strGroupId), Error))
	{
		m_strGroupStatus = "Save the group first; preview plays the saved file.";
		return false;
	}
	float4x4_t Pivot;
	EFFECT_V2_TARGET_VIEW View;
	if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
		Pivot = pPreview->PivotWorld();
	else if (!Resolve_TargetView(View) ||
		!CEffectV2Object::Resolve_TargetPivot(View, m_strPivotBone, m_ePivotRotation, Pivot))
	{
		m_strGroupStatus = "Create a preview effect or spawn a target to place the group.";
		return false;
	}
	m_iGroupPreviewHandle = CEffectV2Runtime::Play_Group(strGroupId, Pivot, m_pDevice, m_pContext);
	if (0u == m_iGroupPreviewHandle)
	{
		m_strGroupStatus = "Group preview failed: " + CEffectV2Runtime::Last_Error();
		return false;
	}
	m_strGroupStatus = "Playing " + strGroupId;
	return true;
}

void Client::CEffect_Tool_V2::Stop_GroupPreview()
{
	if (0u == m_iGroupPreviewHandle)
		return;
	CEffectV2Runtime::Stop_Group(m_iGroupPreviewHandle);
	m_iGroupPreviewHandle = 0u;
}

void Client::CEffect_Tool_V2::Render_GroupWindow()
{
	if (!m_bGroupWindowOpen)
		return;
	if (!m_bGroupsScanned)
		Scan_Groups();
	if (!m_bDocumentsScanned)
		Scan_Documents();
	ImGui::SetNextWindowSize(ImVec2(560.f, 560.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Group v2", &m_bGroupWindowOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::SeparatorText("Group (Data/Effects/V2/Groups)");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##GroupId", "Group ID (e.g. boss.valtan.portal)",
		m_szGroupId, sizeof(m_szGroupId));
	if (ImGui::Button("New"))
	{
		Stop_GroupPreview();
		m_Group = {};
		m_Group.strGroupId = m_szGroupId;
		m_strGroupStatus = "New group (unsaved).";
	}
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_szGroupId[0]);
	if (ImGui::Button("Load"))
		Load_Group(m_szGroupId);
	ImGui::SameLine();
	if (ImGui::Button("Save"))
		Save_Group();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Rescan"))
		Scan_Groups();
	if (m_Groups.empty())
		ImGui::TextDisabled("No saved groups.");
	else if (ImGui::BeginListBox("##Groups", ImVec2(-1.f, 72.f)))
	{
		for (const std::string& strGroup : m_Groups)
		{
			if (ImGui::Selectable(strGroup.c_str(), strGroup == m_szGroupId))
				std::snprintf(m_szGroupId, sizeof(m_szGroupId), "%s", strGroup.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				Load_Group(strGroup);
		}
		ImGui::EndListBox();
	}
	int32_t iGroupDuration = static_cast<int32_t>(m_Group.iDurationMs);
	if (ImGui::InputInt("Group Duration (ms, 0 = last child)", &iGroupDuration))
		m_Group.iDurationMs = static_cast<uint32_t>(std::clamp(iGroupDuration, 0, 600000));

	ImGui::SeparatorText("Children");
	ImGui::BeginDisabled('\0' == m_szEffectId[0]);
	if (ImGui::Button("Add Child (current Effect ID)"))
	{
		EFFECT_V2_GROUP_CHILD Child;
		Child.strEffectId = m_szEffectId;
		m_Group.Children.push_back(std::move(Child));
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Effect ID comes from the Document panel.");
	uint32_t iEndMs = m_Group.iDurationMs;
	for (size_t iIndex = 0u; iIndex < m_Group.Children.size(); ++iIndex)
	{
		EFFECT_V2_GROUP_CHILD& Child = m_Group.Children[iIndex];
		ImGui::PushID(static_cast<int32_t>(iIndex));
		if (ImGui::BeginCombo("Effect", Child.strEffectId.empty() ? "(select)" : Child.strEffectId.c_str()))
		{
			for (const std::string& strDocument : m_Documents)
			{
				if (ImGui::Selectable(strDocument.c_str(), strDocument == Child.strEffectId))
					Child.strEffectId = strDocument;
			}
			ImGui::EndCombo();
		}
		int32_t iStart = static_cast<int32_t>(Child.iStartMs);
		int32_t iDuration = static_cast<int32_t>(Child.iDurationMs);
		if (ImGui::InputInt("Start (ms)", &iStart))
			Child.iStartMs = static_cast<uint32_t>(std::clamp(iStart, 0, 600000));
		if (ImGui::InputInt("Duration (ms, 0 = own lifetime)", &iDuration))
			Child.iDurationMs = static_cast<uint32_t>(std::clamp(iDuration, 0, 600000));
		int32_t iStop = static_cast<int32_t>(Child.eStop);
		if (ImGui::Combo("Stop", &iStop, "Kill\0Deactivate\0"))
			Child.eStop = static_cast<EFFECT_V2_CHILD_STOP>(iStop);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Kill: remove at once. Deactivate: particles/trails stop spawning and drain; other shapes end.");
		ImGui::DragFloat3("Offset (m)", &Child.vOffset.x, 0.05f);
		ImGui::DragFloat("Yaw (deg)", &Child.fYawDegrees, 1.f, -360.f, 360.f);
		if (ImGui::SmallButton("Remove"))
		{
			m_Group.Children.erase(m_Group.Children.begin() + static_cast<std::ptrdiff_t>(iIndex));
			ImGui::PopID();
			break;
		}
		ImGui::Separator();
		ImGui::PopID();
		iEndMs = (std::max)(iEndMs,
			Child.iDurationMs > 0u ? Child.iStartMs + Child.iDurationMs : Child.iStartMs);
	}

	ImGui::SeparatorText("Timeline");
	const f32_t fTotalMs = static_cast<f32_t>((std::max)(1000u, iEndMs));
	ImDrawList* pDraw = ImGui::GetWindowDrawList();
	const ImVec2 Origin = ImGui::GetCursorScreenPos();
	const f32_t fWidth = (std::max)(64.f, ImGui::GetContentRegionAvail().x);
	constexpr f32_t ROW_HEIGHT = 16.f;
	for (size_t iIndex = 0u; iIndex < m_Group.Children.size(); ++iIndex)
	{
		const EFFECT_V2_GROUP_CHILD& Child = m_Group.Children[iIndex];
		const f32_t fY = Origin.y + static_cast<f32_t>(iIndex) * ROW_HEIGHT;
		const f32_t fX0 = Origin.x + static_cast<f32_t>(Child.iStartMs) / fTotalMs * fWidth;
		const f32_t fX1 = Child.iDurationMs > 0u ?
			Origin.x + static_cast<f32_t>(Child.iStartMs + Child.iDurationMs) / fTotalMs * fWidth :
			Origin.x + fWidth;
		pDraw->AddRectFilled(ImVec2(fX0, fY + 2.f), ImVec2((std::max)(fX1, fX0 + 3.f), fY + ROW_HEIGHT - 2.f),
			Child.iDurationMs > 0u ? IM_COL32(90, 170, 255, 200) : IM_COL32(120, 120, 140, 160));
		pDraw->AddText(ImVec2(fX0 + 3.f, fY), IM_COL32(255, 255, 255, 255), Child.strEffectId.c_str());
	}
	const f32_t fPreviewSeconds = 0u != m_iGroupPreviewHandle ?
		CEffectV2Runtime::Group_Seconds(m_iGroupPreviewHandle) : -1.f;
	if (fPreviewSeconds >= 0.f)
	{
		const f32_t fX = Origin.x + (std::min)(1.f, fPreviewSeconds * 1000.f / fTotalMs) * fWidth;
		pDraw->AddLine(ImVec2(fX, Origin.y), ImVec2(fX, Origin.y + ROW_HEIGHT * static_cast<f32_t>(m_Group.Children.size())),
			IM_COL32(255, 200, 60, 255), 2.f);
	}
	ImGui::Dummy(ImVec2(fWidth, ROW_HEIGHT * static_cast<f32_t>(m_Group.Children.size()) + 4.f));
	ImGui::TextDisabled("0 ms .. %.0f ms", fTotalMs);
	if (ImGui::Button(0u != m_iGroupPreviewHandle ? "Restart Preview" : "Play Preview"))
		Play_GroupPreview();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u == m_iGroupPreviewHandle);
	if (ImGui::Button("Stop Preview"))
		Stop_GroupPreview();
	ImGui::EndDisabled();
	if (fPreviewSeconds >= 0.f)
	{
		ImGui::SameLine();
		ImGui::Text("%.2f s", fPreviewSeconds);
	}
	if (!m_strGroupStatus.empty())
		ImGui::TextWrapped("%s", m_strGroupStatus.c_str());
	ImGui::End();
}

namespace
{
	void Draw_LerpTrack(
		const char* szLabel,
		Client::CEffectV2Object::LERP_FLOAT3& Track,
		const float fSpeed,
		const float fMin,
		const float fMax)
	{
		ImGui::PushID(szLabel);
		ImGui::Checkbox("##Lerp", &Track.bLerp);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Interpolate Start -> End over Lifetime.");
		ImGui::SameLine();
		ImGui::DragFloat3(szLabel, &Track.vStart.x, fSpeed, fMin, fMax);
		if (Track.bLerp)
		{
			ImGui::Indent(24.f);
			ImGui::DragFloat3("End", &Track.vEnd.x, fSpeed, fMin, fMax);
			ImGui::Unindent(24.f);
		}
		ImGui::PopID();
	}
}

namespace
{
	void Draw_ColorTracks(Client::CEffectV2Object::PARAMS& P, const bool_t bShowOffset)
	{
		ImGui::Checkbox("##ColorMulLerp", &P.bColorMulLerp);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Lerp Color Mul from Start to End over the lifetime.");
		ImGui::SameLine();
		ImGui::ColorEdit4(P.bColorMulLerp ? "Color Mul Start" : "Color Mul",
			&P.vColorMul.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("HDR: values above 1 push the effect past the bloom threshold (type into the fields).");
		if (P.bColorMulLerp)
		{
			ImGui::Indent(24.f);
			ImGui::ColorEdit4("Color Mul End", &P.vColorMulEnd.x,
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::Unindent(24.f);
		}
		if (bShowOffset)
		{
			ImGui::Checkbox("##ColorOffsetLerp", &P.bColorOffsetLerp);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Lerp Color Offset from Start to End over the lifetime.");
			ImGui::SameLine();
			ImGui::DragFloat4(P.bColorOffsetLerp ? "Color Offset Start" : "Color Offset",
				&P.vColorOffset.x, 0.01f, -1.f, 1.f);
			if (P.bColorOffsetLerp)
			{
				ImGui::Indent(24.f);
				ImGui::DragFloat4("Color Offset End", &P.vColorOffsetEnd.x, 0.01f, -1.f, 1.f);
				ImGui::Unindent(24.f);
			}
		}
		if ((P.bColorMulLerp || (bShowOffset && P.bColorOffsetLerp)) && P.fLifetime <= 0.f)
			ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f), "Lifetime is 0: color stays at Start.");
	}

	void Draw_PlaybackSection(Client::CEffectV2Object::PARAMS& P, const bool_t bFinished)
	{
		ImGui::SeparatorText("Playback");
		ImGui::DragFloat("Lifetime (s, 0 = infinite)", &P.fLifetime, 0.05f, 0.f, 600.f);
		ImGui::Checkbox("Loop", &P.bLoop);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.f);
		ImGui::DragFloat("Play Rate", &P.fPlayRate, 0.01f, 0.f, 16.f);
		if (bFinished)
			ImGui::TextDisabled("Finished (lifetime reached). Restart to replay.");
	}
}

void Client::CEffect_Tool_V2::Render_TuningPanel()
{
	if (!m_bTuningWindowOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(420.f, 720.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Tuning v2", &m_bTuningWindowOpen))
	{
		ImGui::End();
		return;
	}
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (nullptr == pPreview)
	{
		ImGui::TextDisabled("No live preview. Create Effect to spawn one.");
		ImGui::End();
		return;
	}
	CEffectV2Object::PARAMS& P = pPreview->Params();
	const CEffectV2Object::SHAPE eShape = pPreview->Shape();
	constexpr const char* SHAPE_LABELS[] = { "Mesh", "Sprite", "Particle", "Decal", "Trail", "Screen Post" };
	static_assert(_countof(SHAPE_LABELS) == static_cast<size_t>(CEffectV2Object::SHAPE::END));
	ImGui::Text("%s | %.2fs | life %.2f | %s",
		SHAPE_LABELS[static_cast<size_t>(eShape)],
		pPreview->Time(), pPreview->Life_Ratio(), pPreview->Status().c_str());
	if (CEffectV2Object::SHAPE::PARTICLE == eShape)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("| %u particles", pPreview->Particle_Count());
	}
	else if (CEffectV2Object::SHAPE::TRAIL == eShape)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("| %u points", pPreview->Trail_PointCount());
	}
	if (ImGui::Button("Restart"))
		pPreview->Restart();
	ImGui::SameLine();
	bool_t bVisible = !pPreview->Is_Hidden();
	if (ImGui::Checkbox("Visible", &bVisible))
		pPreview->Set_Hidden(!bVisible);
	ImGui::SameLine();
	if (ImGui::Button("Bring To Camera"))
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
		const float4x4_t* pCameraWorld = GameInstance.Get_InverseTransform(D3DTS::VIEW);
		m_ePivotMode = PIVOT_MODE::WORLD;
		if (nullptr != pCameraPosition && nullptr != pCameraWorld)
		{
			const vector_t Look = XMVector3Normalize(XMLoadFloat4x4(pCameraWorld).r[2]);
			XMStoreFloat4x4(&pPreview->PivotWorld(), XMMatrixTranslationFromVector(
				XMLoadFloat4(pCameraPosition) + Look * 3.f));
		}
	}
	if (CEffectV2Object::SHAPE::SCREEN_POST == eShape)
	{
		CEffectV2Object::SCREEN_POST_PARAMS& S = P.ScreenPost;
		ImGui::SeparatorText("Screen Post (full-screen)");
		int32_t iProfile = static_cast<int32_t>(S.eProfile);
		if (ImGui::Combo("Profile", &iProfile, "Zoom Blur\0RGB Noise (chromatic)\0Film Noise\0"))
			S.eProfile = static_cast<CEffectV2Object::SCREEN_POST_PROFILE>(iProfile);
		const char* pIntensityLabel = "Intensity";
		const char* pSecondaryLabel = "Secondary";
		const char* pHint = "";
		switch (S.eProfile)
		{
		case CEffectV2Object::SCREEN_POST_PROFILE::RGB_NOISE:
			pIntensityLabel = "Chromatic Offset (0-8)";
			pSecondaryLabel = "Grain Amount (0-8)";
			pHint = "R/B channels shift by a per-pixel noise * Intensity; Secondary adds tinted grain. Frequency = noise refresh rate.";
			break;
		case CEffectV2Object::SCREEN_POST_PROFILE::FILM_NOISE:
			pIntensityLabel = "Grain (0-8)";
			pSecondaryLabel = "Scanline (0-8)";
			pHint = "Tinted grain + horizontal scanlines. Frequency scales both.";
			break;
		default:
			pIntensityLabel = "Blur Strength (0-8)";
			pSecondaryLabel = "Mix (0-1, 0 = use strength)";
			pHint = "8-tap radial blur from screen centre. Tint multiplies the blurred layer.";
			break;
		}
		ImGui::Checkbox("Intensity Lerp over Lifetime", &S.bIntensityLerp);
		ImGui::DragFloat(S.bIntensityLerp ? "Intensity Start" : pIntensityLabel,
			&S.fIntensityStart, 0.01f, 0.f, 8.f);
		if (S.bIntensityLerp)
		{
			ImGui::DragFloat("Intensity End", &S.fIntensityEnd, 0.01f, 0.f, 8.f);
			if (P.fLifetime <= 0.f)
				ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f),
					"Lifetime is 0: intensity stays at Start.");
		}
		ImGui::DragFloat(pSecondaryLabel, &S.fSecondaryIntensity, 0.01f, 0.f, 8.f);
		ImGui::DragFloat("Frequency", &S.fFrequency, 0.01f, 0.f, 64.f);
		ImGui::ColorEdit4("Tint", &S.vTint.x, ImGuiColorEditFlags_Float);
		int32_t iSeed = static_cast<int32_t>(S.iRandomSeed);
		if (ImGui::InputInt("Random Seed", &iSeed))
			S.iRandomSeed = static_cast<uint32_t>((std::max)(1, iSeed));
		ImGui::TextDisabled("%s", pHint);
		ImGui::TextDisabled("Now: intensity %.2f", pPreview->ScreenPost_Intensity());
		Draw_PlaybackSection(P, pPreview->Is_Finished());
		ImGui::End();
		return;
	}

	const bool_t bLifetimeKnown = P.fLifetime > 0.f;
	const bool_t bAnyLerp =
		P.Position.bLerp || P.Rotation.bLerp || P.Scale.bLerp || P.Velocity.bLerp;
	if (!bLifetimeKnown && (bAnyLerp || pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::DISSOLVE)))
		ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f),
			"Lifetime is 0: Lerp tracks and Dissolve Start stay at their start values.");

	ImGui::SeparatorText("Transform (relative to pivot)");
	if (PIVOT_MODE::TARGET_BONE == m_ePivotMode)
	{
		ImGui::TextDisabled("Pivot follows target bone '%s' (Attach window). World pivot is read-only.",
			m_strPivotBone.c_str());
	}
	ImGui::BeginDisabled(PIVOT_MODE::TARGET_BONE == m_ePivotMode);
	ImGui::DragFloat3("Pivot (world)", &pPreview->PivotWorld()._41, 0.05f);
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Parent pivot. Position/Rotation/Scale/Velocity below are relative to it.");
	Draw_LerpTrack("Position", P.Position, 0.05f, -1000.f, 1000.f);
	const bool_t bTrailCenterline = CEffectV2Object::SHAPE::TRAIL == eShape &&
		CEffectV2Object::TRAIL_EDGE_MODE::LOCAL_OFFSET != P.Trail.eEdgeMode;
	ImGui::BeginDisabled(bTrailCenterline);
	Draw_LerpTrack("Rotation (deg)", P.Rotation, 1.f, -3600.f, 3600.f);
	Draw_LerpTrack("Scale", P.Scale, 0.01f, 0.001f, 1000.f);
	ImGui::EndDisabled();
	if (bTrailCenterline)
		ImGui::TextDisabled("Centerline trail samples the pivot position only: use Start/End Width for thickness, Point Lifetime for length.");
	Draw_LerpTrack("Velocity (m/s)", P.Velocity, 0.05f, -1000.f, 1000.f);
	if (CEffectV2Object::SHAPE::MESH == pPreview->Shape())
	{
		ImGui::DragFloat("Mesh Pre-Scale", &P.fMeshPreScale, 0.00005f, 0.00001f, 10.f, "%.5f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("WModel unit conversion applied before Scale. Static FX meshes are cm (0.01 = metres); NPC-pipeline skinned cooks carry a x100 armature node, so 0.0001 = metres.");
		ImGui::SameLine();
		if (ImGui::SmallButton("1"))
			P.fMeshPreScale = 1.f;
		ImGui::SameLine();
		if (ImGui::SmallButton("0.01"))
			P.fMeshPreScale = 0.01f;
		ImGui::SameLine();
		if (ImGui::SmallButton("0.0001"))
			P.fMeshPreScale = 0.0001f;
	}

	if (CEffectV2Object::SHAPE::PARTICLE == eShape)
	{
		CEffectV2Object::PARTICLE_PARAMS& E = P.Particle;
		ImGui::SeparatorText("Particle Emitter");
		int32_t iMax = static_cast<int32_t>(E.iMaxParticles);
		if (ImGui::DragInt("Max Particles", &iMax, 1.f, 1, 2048))
			E.iMaxParticles = static_cast<uint32_t>((std::max)(1, iMax));
		ImGui::DragFloat("Spawn Rate (/s)", &E.fSpawnRate, 0.5f, 0.f, 2048.f);
		int32_t iBurst = static_cast<int32_t>(E.iBurstCount);
		if (ImGui::DragInt("Burst Count", &iBurst, 1.f, 0, 2048))
			E.iBurstCount = static_cast<uint32_t>((std::max)(0, iBurst));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Spawned at once on Restart and on every lifetime loop.");
		ImGui::DragFloat2("Particle Lifetime (s min/max)", &E.vLifetime.x, 0.01f, 0.01f, 60.f);
		if (E.vLifetime.y < E.vLifetime.x)
			E.vLifetime.y = E.vLifetime.x;
		int32_t iSeed = static_cast<int32_t>(E.iRandomSeed);
		if (ImGui::InputInt("Random Seed", &iSeed))
			E.iRandomSeed = static_cast<uint32_t>((std::max)(1, iSeed));
		ImGui::Checkbox("Local Space", &E.bLocalSpace);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("On: particles move with the pivot/transform. Off: particles are left behind in the world.");

		ImGui::SeparatorText("Spawn Shape");
		int32_t iShape = static_cast<int32_t>(E.eSpawnShape);
		if (ImGui::Combo("Shape", &iShape, "Point\0Sphere\0Ring\0Box\0"))
			E.eSpawnShape = static_cast<CEffectV2Object::PARTICLE_SPAWN_SHAPE>(iShape);
		if (CEffectV2Object::PARTICLE_SPAWN_SHAPE::SPHERE == E.eSpawnShape ||
			CEffectV2Object::PARTICLE_SPAWN_SHAPE::RING == E.eSpawnShape)
		{
			ImGui::DragFloat("Radius", &E.fSpawnRadius, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("Inner Radius", &E.fSpawnInnerRadius, 0.01f, 0.f, 100.f);
			if (E.fSpawnInnerRadius > E.fSpawnRadius)
				E.fSpawnInnerRadius = E.fSpawnRadius;
		}
		if (CEffectV2Object::PARTICLE_SPAWN_SHAPE::RING == E.eSpawnShape)
			ImGui::DragFloat("Arc (deg)", &E.fSpawnArcDegrees, 1.f, 0.f, 360.f);
		if (CEffectV2Object::PARTICLE_SPAWN_SHAPE::BOX == E.eSpawnShape)
			ImGui::DragFloat3("Half Extents", &E.vSpawnExtents.x, 0.01f, 0.f, 100.f);

		ImGui::SeparatorText("Initial Velocity");
		int32_t iVelocity = static_cast<int32_t>(E.eVelocityMode);
		if (ImGui::Combo("Mode", &iVelocity, "Fixed (random box)\0Outward\0Cone (+Y)\0"))
			E.eVelocityMode = static_cast<CEffectV2Object::PARTICLE_VELOCITY_MODE>(iVelocity);
		if (CEffectV2Object::PARTICLE_VELOCITY_MODE::FIXED == E.eVelocityMode)
		{
			ImGui::DragFloat3("Velocity Min (m/s)", &E.vVelocityMin.x, 0.05f, -100.f, 100.f);
			ImGui::DragFloat3("Velocity Max (m/s)", &E.vVelocityMax.x, 0.05f, -100.f, 100.f);
		}
		else
		{
			ImGui::DragFloat2("Speed (m/s min/max)", &E.vSpeedRange.x, 0.05f, 0.f, 100.f);
			if (E.vSpeedRange.y < E.vSpeedRange.x)
				E.vSpeedRange.y = E.vSpeedRange.x;
		}
		if (CEffectV2Object::PARTICLE_VELOCITY_MODE::CONE == E.eVelocityMode)
			ImGui::DragFloat("Cone Half Angle (deg)", &E.fConeAngleDegrees, 0.5f, 0.f, 180.f);
		ImGui::DragFloat3("Acceleration (m/s^2)", &E.vAcceleration.x, 0.05f, -100.f, 100.f);
		ImGui::DragFloat("Drag", &E.fDrag, 0.01f, 0.f, 20.f);

		ImGui::SeparatorText("Particle Size / Rotation");
		ImGui::DragFloat2("Size Start (m)", &E.vSizeStart.x, 0.005f, 0.f, 100.f);
		ImGui::DragFloat2("Size End (m)", &E.vSizeEnd.x, 0.005f, 0.f, 100.f);
		ImGui::DragFloat2("Rotation (deg min/max)", &E.vRotationRange.x, 1.f, -360.f, 360.f);
		ImGui::DragFloat2("Spin (deg/s min/max)", &E.vSpinRange.x, 1.f, -3600.f, 3600.f);
		int32_t iAlignment = static_cast<int32_t>(E.eAlignment);
		if (ImGui::Combo("Alignment", &iAlignment, "Camera\0Velocity\0Horizontal\0"))
			E.eAlignment = static_cast<CEffectV2Object::PARTICLE_ALIGNMENT>(iAlignment);

		ImGui::SeparatorText("Particle Color / Sub-UV");
		ImGui::ColorEdit4("Color Start", &E.vColorStart.x,
			ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit4("Color End", &E.vColorEnd.x,
			ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Multiplied with Color Mul below over each particle's life.");
		int32_t iColumns = static_cast<int32_t>(E.iTileColumns);
		int32_t iRows = static_cast<int32_t>(E.iTileRows);
		ImGui::SetNextItemWidth(90.f);
		if (ImGui::InputInt("##TileColumns", &iColumns))
			E.iTileColumns = static_cast<uint32_t>((std::max)(1, iColumns));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		if (ImGui::InputInt("Atlas Columns x Rows", &iRows))
			E.iTileRows = static_cast<uint32_t>((std::max)(1, iRows));
		ImGui::Checkbox("Play Atlas Over Life", &E.bSubUVOverLife);
	}
	else if (CEffectV2Object::SHAPE::DECAL == eShape)
	{
		CEffectV2Object::DECAL_PARAMS& D = P.Decal;
		ImGui::SeparatorText("Decal Projection");
		ImGui::DragFloat2("Size X/Z (m)", &D.vSize.x, 0.01f, 0.01f, 100.f);
		ImGui::DragFloat("Depth Y (m)", &D.fDepth, 0.01f, 0.01f, 100.f);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Projection thickness along the pivot's local Y. Surfaces outside +-Depth/2 are not painted.");
		ImGui::SliderFloat("Edge Fade", &D.fEdgeFade, 0.f, 1.f);
		ImGui::SliderFloat("Normal Cutoff (-1 = off)", &D.fNormalCutoff, -1.f, 1.f);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Skip surfaces whose normal deviates from the decal's up axis. 0.5 keeps ground/slopes and drops legs/walls; -1 paints everything inside the box.");
		ImGui::TextDisabled("Projects onto the opaque scene depth. Rotation/Scale tracks tilt and scale the box.");
	}
	else if (CEffectV2Object::SHAPE::TRAIL == eShape)
	{
		CEffectV2Object::TRAIL_PARAMS& T = P.Trail;
		ImGui::SeparatorText("Trail Ribbon");
		int32_t iPoints = static_cast<int32_t>(T.iMaxPoints);
		if (ImGui::DragInt("Max Points", &iPoints, 1.f, 2, 4096))
			T.iMaxPoints = static_cast<uint32_t>((std::max)(2, iPoints));
		ImGui::DragFloat("Point Lifetime (s)", &T.fPointLifetime, 0.01f, 0.01f, 30.f);
		ImGui::DragFloat("Sample Interval (s)", &T.fSampleInterval, 0.001f, 0.f, 1.f, "%.3f");
		ImGui::DragFloat("Min Distance (m)", &T.fMinDistance, 0.001f, 0.f, 10.f, "%.3f");
		int32_t iEdge = static_cast<int32_t>(T.eEdgeMode);
		if (ImGui::Combo("Edge Mode", &iEdge, "Centerline (face camera)\0Centerline (world up)\0Local Offset (pivot -> offset)\0"))
			T.eEdgeMode = static_cast<CEffectV2Object::TRAIL_EDGE_MODE>(iEdge);
		if (CEffectV2Object::TRAIL_EDGE_MODE::LOCAL_OFFSET == T.eEdgeMode)
		{
			ImGui::DragFloat3("Edge Offset (pivot local, m)", &T.vEdgeOffset.x, 0.01f, -100.f, 100.f);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Second ribbon edge = this offset in pivot space (e.g. blade tip). Width fields are ignored.");
		}
		else
		{
			ImGui::DragFloat("Start Width (m)", &T.fStartWidth, 0.01f, 0.f, 100.f);
			ImGui::DragFloat("End Width (m)", &T.fEndWidth, 0.01f, 0.f, 100.f);
		}
		ImGui::DragFloat("Tiling Distance (m, 0 = per point)", &T.fTilingDistance, 0.01f, 0.f, 100.f);
		ImGui::Checkbox("Fade With Age", &T.bFadeWithAge);
		ImGui::TextDisabled("A trail only appears while the pivot moves: attach to a bone, give it a Velocity, or use Test Orbit.");
		if (ImGui::Checkbox("Test Orbit (pivot circles in place)", &m_bTestOrbit) && m_bTestOrbit)
		{
			m_vTestOrbitCenter = float3_t(
				pPreview->PivotWorld()._41, pPreview->PivotWorld()._42, pPreview->PivotWorld()._43);
			m_fTestOrbitAngle = 0.f;
			m_ePivotMode = PIVOT_MODE::WORLD;
		}
		if (m_bTestOrbit)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			ImGui::DragFloat("Radius", &m_fTestOrbitRadius, 0.05f, 0.1f, 20.f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			ImGui::DragFloat("rad/s", &m_fTestOrbitSpeed, 0.05f, 0.1f, 20.f);
		}
	}

	if (pPreview->Is_Skinned())
	{
		ImGui::SeparatorText("Animation");
		const uint32_t iClipCount = pPreview->Animation_Count();
		if (0u == iClipCount)
			ImGui::TextDisabled("Skinned mesh without clips (bind pose only).");
		else
		{
			const char_t* pCurrentName = pPreview->Animation_Name(P.iAnimationIndex);
			char_t szCurrent[160]{};
			snprintf(szCurrent, sizeof(szCurrent), "[%u] %s",
				P.iAnimationIndex, nullptr != pCurrentName ? pCurrentName : "(none)");
			if (ImGui::BeginCombo("Clip", szCurrent))
			{
				for (uint32_t iClip = 0u; iClip < iClipCount; ++iClip)
				{
					const char_t* pName = pPreview->Animation_Name(iClip);
					char_t szItem[160]{};
					snprintf(szItem, sizeof(szItem), "[%u] %s (%.2fs)",
						iClip, nullptr != pName ? pName : "(none)",
						pPreview->Animation_DurationSeconds(iClip));
					const bool_t bSelected = iClip == P.iAnimationIndex;
					if (ImGui::Selectable(szItem, bSelected))
						P.iAnimationIndex = iClip;
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::Checkbox("Clip Loop", &P.bAnimationLoop);
			ImGui::SameLine();
			if (ImGui::Button("Lifetime = Clip Length"))
				P.fLifetime = pPreview->Animation_DurationSeconds(P.iAnimationIndex);
			f32_t fClipSeconds = 0.f;
			f32_t fClipDuration = 0.f;
			if (pPreview->Animation_Progress(fClipSeconds, fClipDuration))
				ImGui::TextDisabled("Clip %.2f / %.2f s (Play Rate applies)", fClipSeconds, fClipDuration);
		}
	}

	ImGui::SeparatorText("Color");
	Draw_ColorTracks(P, true);
	if (ImGui::Checkbox("Base/Emissive are sRGB (gamma) textures", &P.bColorTexturesSRGB))
	{
		if (FAILED(pPreview->Reload_ColorTextures()))
			m_strPreviewStatus = "Color texture reload failed.";
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("On (default): colour textures are decoded to linear before the HDR scene, matching the engine materials. Off: sampled raw (washed-out mid-tones for painted textures; correct for data/mask images used as Base).");
	int32_t iClipChannel = static_cast<int32_t>(P.eColorClipChannel);
	ImGui::SetNextItemWidth(90.f);
	if (ImGui::Combo("##ClipChannel", &iClipChannel, "RGB\0Alpha\0"))
		P.eColorClipChannel = static_cast<CEffectV2Object::COLOR_CLIP_CHANNEL>(iClipChannel);
	ImGui::SameLine();
	ImGui::SliderFloat("Color Clip", &P.fColorClip, 0.f, 1.f);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Pixels whose max(RGB) or A is <= this value are discarded. 0 = off.");

	if (CEffectV2Object::SHAPE::MESH == pPreview->Shape())
	{
		ImGui::SeparatorText("Rim (Fresnel)");
		ImGui::ColorEdit3("Rim Color", &P.vRimColor.x, ImGuiColorEditFlags_Float);
		ImGui::DragFloat("Rim Power", &P.fRimPower, 0.05f, 0.1f, 16.f);
		ImGui::DragFloat("Rim Intensity", &P.fRimIntensity, 0.05f, 0.f, 8.f);
		ImGui::SliderFloat("Ghost Alpha", &P.fGhostAlpha, 0.f, 1.f);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("0 = alpha untouched, 1 = alpha multiplied by the fresnel term (only silhouettes remain).");

		ImGui::SeparatorText("Outline (inverted hull)");
		ImGui::DragFloat("Outline Width (m, 0 = off)", &P.fOutlineWidth, 0.001f, 0.f, 0.5f, "%.3f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Front-culled copy pushed along the normals, stencil-tested against the body so it only shows at the silhouette. Follows the Dissolve texture.");
		ImGui::ColorEdit4("Outline Color", &P.vOutlineColor.x,
			ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
	}

	ImGui::SeparatorText("Bloom / Distortion");
	ImGui::BeginDisabled(!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::EMISSIVE));
	ImGui::DragFloat("Bloom Intensity", &P.fBloomIntensity, 0.05f, 0.f, 32.f);
	ImGui::EndDisabled();
	if (!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::EMISSIVE))
		ImGui::TextDisabled("Bind an Emissive texture to use Bloom Intensity.");
	ImGui::BeginDisabled(!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::NOISE));
	ImGui::DragFloat("Distortion Intensity", &P.fDistortionIntensity, 0.001f, 0.f, 0.1f, "%.3f");
	ImGui::DragFloat("Noise Strength", &P.fNoiseStrength, 0.005f, 0.f, 2.f);
	ImGui::DragFloat("Noise Scale", &P.fNoiseScale, 0.01f, 0.01f, 64.f);
	ImGui::DragFloat2("Noise Pan (uv/s)", &P.vNoisePan.x, 0.01f, -10.f, 10.f);
	ImGui::EndDisabled();
	if (!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::NOISE))
		ImGui::TextDisabled("Bind a Noise texture to use Distortion and Noise.");

	ImGui::SeparatorText("UV");
	ImGui::DragFloat2("UV Start", &P.vUVStart.x, 0.01f, -10.f, 10.f);
	ImGui::DragFloat2("UV Speed (uv/s)", &P.vUVSpeed.x, 0.01f, -10.f, 10.f);
	ImGui::DragFloat2("UV TileCount", &P.vUVTileCount.x, 0.01f, 0.01f, 64.f);

	ImGui::SeparatorText("Dissolve");
	ImGui::BeginDisabled(!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::DISSOLVE));
	ImGui::SliderFloat("Dissolve In End (life 0-1, 0 = off)", &P.fDissolveInEnd, 0.f, 1.f);
	ImGui::SliderFloat("Dissolve Start (life 0-1)", &P.fDissolveStart, 0.f, 1.f);
	ImGui::SliderFloat("Dissolve Softness", &P.fDissolveSoftness, 0.f, 0.5f);
	ImGui::EndDisabled();
	if (pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::DISSOLVE))
	{
		ImGui::TextDisabled("Dissolve amount now %.2f", pPreview->Dissolve_Amount());
		if (P.fLifetime <= 0.f)
			ImGui::TextDisabled("Lifetime is infinite, so Dissolve stays at 0.");
		else if (0.f < P.fDissolveInEnd && P.fDissolveStart < P.fDissolveInEnd)
			ImGui::TextDisabled(
				"Dissolve Start clamped to %.2f so the reveal finishes first.",
				P.fDissolveInEnd);
	}
	else
		ImGui::TextDisabled("Bind a Dissolve texture to use Dissolve In End and Start.");

	if (CEffectV2Object::SHAPE::MESH == pPreview->Shape() && 0u < pPreview->Part_Count())
	{
		ImGui::SeparatorText("Parts");
		ImGui::TextDisabled("Base slot now: %s",
			Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty() ?
			"(none)" :
			std::filesystem::path(Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)])
				.filename().string().c_str());
		for (uint32_t iPart = 0u; iPart < pPreview->Part_Count(); ++iPart)
		{
			ImGui::PushID(static_cast<int32_t>(iPart));
			ImGui::Checkbox("##Visible", &pPreview->Part_Visible(iPart));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Visible");
			ImGui::SameLine();
			const std::string& strOverride = pPreview->Part_BaseAssetId(iPart);
			ImGui::Text("#%u %s", iPart, pPreview->Part_Name(iPart).c_str());
			ImGui::SameLine();
			ImGui::TextDisabled("%s", strOverride.empty() ? "(shared Base)" :
				std::filesystem::path(strOverride).filename().string().c_str());
			ImGui::SameLine();
			ImGui::BeginDisabled(
				Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty());
			if (ImGui::SmallButton("<- Base slot"))
			{
				if (FAILED(pPreview->Set_PartBase(iPart,
					Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)])))
				{
					m_strPreviewStatus = "Part texture load failed.";
				}
			}
			ImGui::EndDisabled();
			if (!strOverride.empty())
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Clear"))
					pPreview->Set_PartBase(iPart, std::string());
			}
			ImGui::PopID();
		}
	}

	ImGui::SeparatorText("Blend");
	int32_t iBlend = static_cast<int32_t>(P.eBlend);
	if (ImGui::Combo("Blend", &iBlend, "Alpha\0Additive\0Opaque\0Multiply\0"))
		P.eBlend = static_cast<CEffectV2Object::BLEND_MODE>(iBlend);
	ImGui::BeginDisabled(CEffectV2Object::BLEND_MODE::SOLID == P.eBlend ||
		CEffectV2Object::SHAPE::DECAL == eShape);
	ImGui::Checkbox("Depth Test", &P.bDepthTest);
	ImGui::EndDisabled();
	if (CEffectV2Object::SHAPE::DECAL == eShape)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(decal: Alpha/Additive/Multiply, no depth test)");
	}
	if (CEffectV2Object::SHAPE::SPRITE == pPreview->Shape())
	{
		ImGui::SameLine();
		ImGui::Checkbox("Billboard", &P.bBillboard);
	}
	ImGui::BeginDisabled(CEffectV2Object::SHAPE::DECAL == eShape);
	ImGui::DragFloat("Soft Fade (world units, 0 = off)",
		&P.fSoftFadeDistance, 0.01f, 0.f, 10.f, "%.2f");
	ImGui::EndDisabled();
	if (CEffectV2Object::SHAPE::DECAL == eShape)
		ImGui::TextDisabled("Decals fade through Decal Projection > Edge Fade instead.");
	else if (0.f < P.fSoftFadeDistance && !P.bDepthTest)
		ImGui::TextDisabled(
			"Depth Test is off, but Soft Fade still reads scene depth: "
			"this effect now dims behind geometry.");

	ImGui::SeparatorText("Playback");
	ImGui::DragFloat("Lifetime (s, 0 = infinite)", &P.fLifetime, 0.05f, 0.f, 600.f);
	ImGui::Checkbox("Loop", &P.bLoop);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::DragFloat("Play Rate", &P.fPlayRate, 0.01f, 0.f, 16.f);
	if (pPreview->Is_Finished())
		ImGui::TextDisabled("Finished (lifetime reached). Restart to replay.");
	ImGui::End();
}

Client::CEffect_Tool_V2::SLOT_BINDINGS& Client::CEffect_Tool_V2::Current_Bindings()
{
	return m_SlotBindings[static_cast<size_t>(m_eType)];
}

std::string& Client::CEffect_Tool_V2::Current_SlotAssetId()
{
	return Current_Bindings()[static_cast<size_t>(m_eSelectedSlot)];
}

bool_t Client::CEffect_Tool_V2::Slot_VisibleForType(const RESOURCE_SLOT eSlot) const
{
	if (EFFECT_TYPE::SCREEN_POST == m_eType)
		return false;
	if (RESOURCE_SLOT::MESH != eSlot)
		return true;
	return EFFECT_TYPE::MESH == m_eType;
}

Client::CEffect_Tool_V2::RESOURCE_KIND Client::CEffect_Tool_V2::Slot_Kind(
	const RESOURCE_SLOT eSlot)
{
	return RESOURCE_SLOT::MESH == eSlot ?
		RESOURCE_KIND::MODEL : RESOURCE_KIND::TEXTURE;
}

const char* Client::CEffect_Tool_V2::Type_Label(const EFFECT_TYPE eType)
{
	switch (eType)
	{
	case EFFECT_TYPE::MESH: return "Mesh";
	case EFFECT_TYPE::TEXTURE: return "Texture";
	case EFFECT_TYPE::PARTICLE: return "Particle";
	case EFFECT_TYPE::DECAL: return "Decal";
	case EFFECT_TYPE::TRAIL: return "Trail";
	case EFFECT_TYPE::SCREEN_POST: return "Screen Post";
	default: return "Unknown";
	}
}

const char* Client::CEffect_Tool_V2::Slot_Label(const RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case RESOURCE_SLOT::MESH: return "Mesh";
	case RESOURCE_SLOT::BASE: return "Base";
	case RESOURCE_SLOT::NOISE: return "Noise";
	case RESOURCE_SLOT::MASK: return "Mask";
	case RESOURCE_SLOT::EMISSIVE: return "Emissive";
	case RESOURCE_SLOT::DISSOLVE: return "Dissolve";
	default: return "Unknown";
	}
}

const char* Client::CEffect_Tool_V2::Slot_Description(const RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case RESOURCE_SLOT::MESH: return "Mesh: one WModel carrier shape.";
	case RESOURCE_SLOT::BASE: return "Base: RGB color, A opacity.";
	case RESOURCE_SLOT::NOISE: return "Noise: UV distortion source.";
	case RESOURCE_SLOT::MASK: return "Mask: R channel multiplies opacity.";
	case RESOURCE_SLOT::EMISSIVE: return "Emissive: RGB added as glow.";
	case RESOURCE_SLOT::DISSOLVE: return "Dissolve: R channel threshold over lifetime.";
	default: return "";
	}
}
