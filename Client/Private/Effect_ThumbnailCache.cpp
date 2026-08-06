#include "Effect_ThumbnailCache.h"

#include "DirectXTK/DDSTextureLoader.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <vector>

namespace
{
	constexpr uint32_t MAX_LOADS_PER_FRAME = 1u;
	constexpr uint64_t LOAD_INTERVAL_FRAMES = 2u;
	constexpr size_t MAX_TEXTURE_DIMENSION = 256u;
	constexpr size_t MAX_CACHE_ENTRIES = 192u;
}

Client::CEffectThumbnailCache::CEffectThumbnailCache(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffectThumbnailCache::~CEffectThumbnailCache() = default;

void Client::CEffectThumbnailCache::Begin_Frame(
	const uint64_t iFrameNumber)
{
	m_iFrameNumber = iFrameNumber;
	m_iLoadsThisFrame = 0u;
}

Client::CEffectThumbnailCache::RESULT
Client::CEffectThumbnailCache::Request(
	const std::string& strAssetId,
	const EFFECT_RESOURCE_FILE_KIND eFileKind)
{
	const std::string strCacheKey =
		(EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind ? "model:" : "texture:") +
		strAssetId;
	auto Iterator = m_Entries.find(strCacheKey);
	if (Iterator != m_Entries.end())
	{
		Iterator->second.iLastUsedFrame = m_iFrameNumber;
		return {
			Iterator->second.pTextureView.Get(),
			Iterator->second.strError.empty() ?
				nullptr : &Iterator->second.strError };
	}

	static const std::string BudgetStatus = "Thumbnail load budget reached.";
	if (m_iLoadsThisFrame >= MAX_LOADS_PER_FRAME ||
		0u != m_iFrameNumber % LOAD_INTERVAL_FRAMES)
		return { nullptr, &BudgetStatus };
	++m_iLoadsThisFrame;

	ENTRY Staged;
	Staged.iLastUsedFrame = m_iFrameNumber;
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
	{
		Staged.strError = "Thumbnail resource is missing: " + strAssetId;
	}
	else if (EFFECT_RESOURCE_FILE_KIND::TEXTURE == eFileKind)
	{
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(), Path.c_str(), nullptr, &Staged.pTextureView,
			MAX_TEXTURE_DIMENSION)))
		{
			Staged.strError = "DDS thumbnail load failed: " + strAssetId;
		}
	}
	else if (EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind)
	{
		Create_ModelThumbnail(Path, Staged.pTextureView, Staged.strError);
	}
	else
	{
		Staged.strError = "Unsupported thumbnail resource kind.";
	}
	const auto Inserted = m_Entries.emplace(strCacheKey, std::move(Staged));
	return {
		Inserted.first->second.pTextureView.Get(),
		Inserted.first->second.strError.empty() ?
			nullptr : &Inserted.first->second.strError };
}

bool_t Client::CEffectThumbnailCache::Create_ModelThumbnail(
	const std::filesystem::path& Path,
	ComPtr<ID3D11ShaderResourceView>& OutTextureView,
	std::string& strOutError)
{
	if (nullptr == m_pModelShader)
	{
		unique_ptr<Engine::CShader> Shader = Engine::CShader::Create(
			m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
			VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == Shader)
		{
			strOutError = "Mesh thumbnail shader creation failed.";
			return false;
		}
		m_pModelShader = std::move(Shader);
	}

	unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
		m_pDevice, m_pContext, MODEL::NONANIM,
		Path.string().c_str(), XMMatrixIdentity());
	if (nullptr == Model || !Model->Has_LocalBounds())
	{
		strOutError = "CModel thumbnail load or bounds failed: " + Path.string();
		return false;
	}

	constexpr uint32_t TARGET_SIZE = 128u;
	D3D11_TEXTURE2D_DESC ColorDesc{};
	ColorDesc.Width = TARGET_SIZE;
	ColorDesc.Height = TARGET_SIZE;
	ColorDesc.MipLevels = 1u;
	ColorDesc.ArraySize = 1u;
	ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorDesc.SampleDesc.Count = 1u;
	ColorDesc.Usage = D3D11_USAGE_DEFAULT;
	ColorDesc.BindFlags = D3D11_BIND_RENDER_TARGET |
		D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> ColorTexture;
	ComPtr<ID3D11RenderTargetView> ColorRTV;
	ComPtr<ID3D11ShaderResourceView> ColorSRV;
	if (FAILED(m_pDevice->CreateTexture2D(
		&ColorDesc, nullptr, &ColorTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(
			ColorTexture.Get(), nullptr, &ColorRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			ColorTexture.Get(), nullptr, &ColorSRV)))
	{
		strOutError = "Mesh thumbnail color target creation failed.";
		return false;
	}

	D3D11_TEXTURE2D_DESC DepthDesc{};
	DepthDesc.Width = TARGET_SIZE;
	DepthDesc.Height = TARGET_SIZE;
	DepthDesc.MipLevels = 1u;
	DepthDesc.ArraySize = 1u;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.SampleDesc.Count = 1u;
	DepthDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ComPtr<ID3D11Texture2D> DepthTexture;
	ComPtr<ID3D11DepthStencilView> DepthDSV;
	if (FAILED(m_pDevice->CreateTexture2D(
		&DepthDesc, nullptr, &DepthTexture)) ||
		FAILED(m_pDevice->CreateDepthStencilView(
			DepthTexture.Get(), nullptr, &DepthDSV)))
	{
		strOutError = "Mesh thumbnail depth target creation failed.";
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
	if (FAILED(m_pDevice->CreateTexture2D(
		&WhiteDesc, &WhiteData, &WhiteTexture)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			WhiteTexture.Get(), nullptr, &WhiteSRV)))
	{
		strOutError = "Mesh thumbnail fallback texture creation failed.";
		return false;
	}

	const float3_t Minimum = Model->Get_LocalBoundsMin();
	const float3_t Maximum = Model->Get_LocalBoundsMax();
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
		strOutError = "Mesh thumbnail bounds are invalid.";
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
	m_pContext->RSGetViewports(
		&iPreviousViewportCount, PreviousViewports.data());

	ID3D11RenderTargetView* pTarget = ColorRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pTarget, DepthDSV.Get());
	const float ClearColor[4] = { 0.035f, 0.045f, 0.06f, 1.f };
	m_pContext->ClearRenderTargetView(ColorRTV.Get(), ClearColor);
	m_pContext->ClearDepthStencilView(
		DepthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<f32_t>(TARGET_SIZE);
	Viewport.Height = static_cast<f32_t>(TARGET_SIZE);
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
		SUCCEEDED(m_pModelShader->Bind_Matrix("g_WorldMatrix", &World)) &&
		SUCCEEDED(m_pModelShader->Bind_Matrix("g_ViewMatrix", &View)) &&
		SUCCEEDED(m_pModelShader->Bind_Matrix("g_ProjMatrix", &Projection)) &&
		SUCCEEDED(m_pModelShader->Bind_RawValue(
			"g_CameraPosition", &CameraPosition, sizeof(CameraPosition))) &&
		SUCCEEDED(m_pModelShader->Bind_RawValue(
			"g_LightDirection", &LightDirection, sizeof(LightDirection)));
	for (uint32_t iMesh = 0u; bRendered && iMesh < Model->Get_NumMeshes(); ++iMesh)
	{
		const uint32_t iHasNormal = Model->Has_MaterialTexture(
			iMesh, aiTextureType_NORMALS) ? 1u : 0u;
		bRendered = SUCCEEDED(m_pModelShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal)));
		if (bRendered && Model->Has_MaterialTexture(iMesh, aiTextureType_DIFFUSE))
			bRendered = SUCCEEDED(Model->Bind_Material(
				m_pModelShader, "g_DiffuseTexture", iMesh, aiTextureType_DIFFUSE));
		else if (bRendered)
			bRendered = SUCCEEDED(m_pModelShader->Bind_Texture(
				"g_DiffuseTexture", WhiteSRV));
		if (bRendered && 0u != iHasNormal)
			bRendered = SUCCEEDED(Model->Bind_Material(
				m_pModelShader, "g_NormalTexture", iMesh, aiTextureType_NORMALS));
		bRendered = bRendered &&
			SUCCEEDED(m_pModelShader->Begin(0u)) &&
			SUCCEEDED(Model->Render(iMesh));
	}

	ID3D11RenderTargetView* pPreviousTarget = PreviousRTV.Get();
	m_pContext->OMSetRenderTargets(
		1u, &pPreviousTarget, PreviousDSV.Get());
	if (0u != iPreviousViewportCount)
		m_pContext->RSSetViewports(
			iPreviousViewportCount, PreviousViewports.data());
	if (!bRendered)
	{
		strOutError = "Mesh thumbnail render failed: " + Path.string();
		return false;
	}
	OutTextureView = std::move(ColorSRV);
	strOutError.clear();
	return true;
}

void Client::CEffectThumbnailCache::Invalidate(
	const uint64_t iCatalogRevision)
{
	if (m_iCatalogRevision == iCatalogRevision)
		return;
	m_iCatalogRevision = iCatalogRevision;
	Clear();
}

void Client::CEffectThumbnailCache::Trim()
{
	if (m_Entries.size() <= MAX_CACHE_ENTRIES)
		return;
	std::vector<std::pair<std::string, uint64_t>> Ages;
	Ages.reserve(m_Entries.size());
	for (const auto& Pair : m_Entries)
		Ages.emplace_back(Pair.first, Pair.second.iLastUsedFrame);
	std::sort(Ages.begin(), Ages.end(),
		[](const auto& Left, const auto& Right)
		{
			return Left.second < Right.second;
		});
	const size_t iRemoveCount = m_Entries.size() - MAX_CACHE_ENTRIES;
	for (size_t iEntry = 0u; iEntry < iRemoveCount; ++iEntry)
		m_Entries.erase(Ages[iEntry].first);
}

void Client::CEffectThumbnailCache::Clear()
{
	m_Entries.clear();
}

