#include "imgui.h"

#include "Effect_Tool_V2.h"
#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "DataJson.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>

namespace
{
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

Client::CEffect_Tool_V2::~CEffect_Tool_V2() = default;

void Client::CEffect_Tool_V2::Render()
{
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
		ImGui::EndTable();
	}

	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
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
	const size_t iSlot = static_cast<size_t>(m_eSelectedSlot);
	const bool_t bUsageFilter =
		RESOURCE_KIND::TEXTURE == eKind && !m_bShowAllTextures;
	for (size_t iEntry = 0u; iEntry < m_Resources.size(); ++iEntry)
	{
		const RESOURCE_ENTRY& Entry = m_Resources[iEntry];
		if (Entry.eKind != eKind)
			continue;
		if (bUsageFilter &&
			(nullptr == Entry.pUsage || 0u == Entry.pUsage->Counts[iSlot]))
			continue;
		if (!m_strDomainFilter.empty() && Entry.strDomain != m_strDomainFilter)
			continue;
		if (!Filter.empty() &&
			std::string::npos == To_Lower(Entry.strFileName).find(Filter))
			continue;
		m_VisibleResources.push_back(iEntry);
	}
	if (RESOURCE_KIND::TEXTURE == eKind)
	{
		std::stable_sort(m_VisibleResources.begin(), m_VisibleResources.end(),
			[this, iSlot](const size_t iLeft, const size_t iRight)
			{
				const TEXTURE_USAGE* pLeft = m_Resources[iLeft].pUsage;
				const TEXTURE_USAGE* pRight = m_Resources[iRight].pUsage;
				const uint32_t iLeftCount = nullptr == pLeft ? 0u : pLeft->Counts[iSlot];
				const uint32_t iRightCount = nullptr == pRight ? 0u : pRight->Counts[iSlot];
				return iLeftCount > iRightCount;
			});
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
		strOutError = "WModel load failed: " + Path.string();
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
		strOutError = "WModel bounds failed: " + Path.string();
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

	if (RESOURCE_KIND::TEXTURE == eKind)
	{
		ImGui::SameLine();
		if (ImGui::Checkbox("Show all", &m_bShowAllTextures))
			m_bVisibleDirty = true;
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Off: only textures that the imported/authored source documents bound to this slot at least once.\nOn: every DDS.");
		}
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

Client::CEffect_Tool_V2::SLOT_BINDINGS& Client::CEffect_Tool_V2::Current_Bindings()
{
	return m_Bindings[static_cast<size_t>(m_eType)];
}

std::string& Client::CEffect_Tool_V2::Current_SlotAssetId()
{
	return Current_Bindings()[static_cast<size_t>(m_eSelectedSlot)];
}

bool_t Client::CEffect_Tool_V2::Slot_VisibleForType(const RESOURCE_SLOT eSlot) const
{
	if (RESOURCE_SLOT::MESH != eSlot)
		return true;
	return EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::PARTICLE == m_eType;
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
