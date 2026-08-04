#include "MapAssetPreview.h"

#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "Model.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
	constexpr uint32_t MIN_TARGET_SIZE = 256;
	constexpr uint32_t MAX_TARGET_SIZE = 2048;
	constexpr uint32_t TARGET_GRANULARITY = 64;

	uint32_t QuantizeTargetSize(uint32_t value)
	{
		value = (std::max)(MIN_TARGET_SIZE,
			(std::min)(MAX_TARGET_SIZE, value));
		value = (value + TARGET_GRANULARITY - 1) /
			TARGET_GRANULARITY * TARGET_GRANULARITY;
		return (std::min)(MAX_TARGET_SIZE, value);
	}

	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) &&
			std::isfinite(value.y) &&
			std::isfinite(value.z);
	}
}

HRESULT CMapAssetPreview::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	if (nullptr == pDevice || nullptr == pContext)
		return E_FAIL;

	m_pDevice = pDevice;
	m_pContext = pContext;
	Reset_Camera();
	return S_OK;
}

void CMapAssetPreview::Reset_LevelResources()
{
	m_pModel.reset();
	m_pShader.reset();
	m_AssetId.clear();
	m_Label.clear();
	m_ModelPath.clear();
	m_vBoundsCenter = {};
	m_vHalfExtent = {};
	m_fBoundsRadius = 1.f;
	m_Status = "Select an asset from the Palette.";
	Reset_Camera();
}

HRESULT CMapAssetPreview::Select_Asset(
	const uint32_t prototypeLevelIndex,
	const MAP_ASSET_ENTRY& asset)
{
	if (prototypeLevelIndex >= ETOUI(LEVEL::END) ||
		asset.id.empty() || asset.prototypeTag.empty())
		return E_FAIL;

	shared_ptr<CShader> stagedShader = m_pShader;
	if (nullptr == stagedShader)
	{
		stagedShader = dynamic_pointer_cast<CShader>(
			CGameInstance::Get().Clone_Prototype(
				prototypeLevelIndex,
				SHADER_PROTOTYPE_TAG));
		if (nullptr == stagedShader)
		{
			m_Status = "Preview shader clone failed.";
			return E_FAIL;
		}
	}

	shared_ptr<CModel> stagedModel =
		dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				prototypeLevelIndex,
				asset.prototypeTag));
	if (nullptr == stagedModel ||
		!stagedModel->Has_LocalBounds())
	{
		m_Status =
			"Selected CModel clone has no valid local bounds.";
		return E_FAIL;
	}

	const float3_t minimum =
		stagedModel->Get_LocalBoundsMin();
	const float3_t maximum =
		stagedModel->Get_LocalBoundsMax();
	const float3_t center(
		(minimum.x + maximum.x) * 0.5f,
		(minimum.y + maximum.y) * 0.5f,
		(minimum.z + maximum.z) * 0.5f);
	const float3_t halfExtent(
		(maximum.x - minimum.x) * 0.5f,
		(maximum.y - minimum.y) * 0.5f,
		(maximum.z - minimum.z) * 0.5f);
	const f32_t radius = std::sqrt(
		halfExtent.x * halfExtent.x +
		halfExtent.y * halfExtent.y +
		halfExtent.z * halfExtent.z);

	if (!IsFinite(center) || !IsFinite(halfExtent) ||
		!std::isfinite(radius) || radius <= 0.0001f)
	{
		m_Status = "Selected CModel bounds are invalid.";
		return E_FAIL;
	}

	m_pShader = std::move(stagedShader);
	m_pModel = std::move(stagedModel);
	m_AssetId = asset.id;
	m_Label = asset.label;
	m_ModelPath =
		asset.modelRelativePath.generic_string();
	m_vBoundsCenter = center;
	m_vHalfExtent = halfExtent;
	m_fBoundsRadius = radius;
	m_Status = "Preview ready.";
	Reset_Camera();
	return S_OK;
}

HRESULT CMapAssetPreview::Ensure_RenderTarget(
	uint32_t width, uint32_t height)
{
	width = QuantizeTargetSize(width);
	height = QuantizeTargetSize(height);
	if (width == m_iTargetWidth &&
		height == m_iTargetHeight &&
		nullptr != m_pColorSRV &&
		nullptr != m_pDepthDSV)
		return S_OK;

	D3D11_TEXTURE2D_DESC colorDesc{};
	colorDesc.Width = width;
	colorDesc.Height = height;
	colorDesc.MipLevels = 1;
	colorDesc.ArraySize = 1;
	colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorDesc.SampleDesc.Count = 1;
	colorDesc.Usage = D3D11_USAGE_DEFAULT;
	colorDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET |
		D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> stagedColorTexture;
	ComPtr<ID3D11RenderTargetView> stagedColorRTV;
	ComPtr<ID3D11ShaderResourceView> stagedColorSRV;
	if (FAILED(m_pDevice->CreateTexture2D(
			&colorDesc, nullptr, &stagedColorTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(
			stagedColorTexture.Get(),
			nullptr,
			&stagedColorRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			stagedColorTexture.Get(),
			nullptr,
			&stagedColorSRV)))
		return E_FAIL;

	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> stagedDepthTexture;
	ComPtr<ID3D11DepthStencilView> stagedDepthDSV;
	if (FAILED(m_pDevice->CreateTexture2D(
			&depthDesc, nullptr, &stagedDepthTexture)) ||
		FAILED(m_pDevice->CreateDepthStencilView(
			stagedDepthTexture.Get(),
			nullptr,
			&stagedDepthDSV)))
		return E_FAIL;

	m_pColorTexture = std::move(stagedColorTexture);
	m_pColorRTV = std::move(stagedColorRTV);
	m_pColorSRV = std::move(stagedColorSRV);
	m_pDepthTexture = std::move(stagedDepthTexture);
	m_pDepthDSV = std::move(stagedDepthDSV);
	m_iTargetWidth = width;
	m_iTargetHeight = height;
	m_bDirty = true;
	return S_OK;
}

HRESULT CMapAssetPreview::Render(
	uint32_t width, uint32_t height)
{
	if (nullptr == m_pModel || nullptr == m_pShader)
		return S_FALSE;

	if (FAILED(Ensure_RenderTarget(width, height)))
	{
		m_Status =
			"Preview render target creation failed.";
		return E_FAIL;
	}

	if (!m_bDirty)
		return S_OK;

	ComPtr<ID3D11RenderTargetView> previousRTV;
	ComPtr<ID3D11DepthStencilView> previousDSV;
	m_pContext->OMGetRenderTargets(
		1, &previousRTV, &previousDSV);

	std::array<D3D11_VIEWPORT,
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
		previousViewports{};
	uint32_t previousViewportCount =
		static_cast<uint32_t>(previousViewports.size());
	m_pContext->RSGetViewports(
		&previousViewportCount,
		previousViewports.data());

	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_pContext->PSSetShaderResources(
		0, 1, &nullSRV);

	ID3D11RenderTargetView* previewRTV =
		m_pColorRTV.Get();
	m_pContext->OMSetRenderTargets(
		1, &previewRTV, m_pDepthDSV.Get());

	const float clearColor[4] = {
		0.025f, 0.035f, 0.045f, 1.f
	};
	m_pContext->ClearRenderTargetView(
		m_pColorRTV.Get(), clearColor);
	m_pContext->ClearDepthStencilView(
		m_pDepthDSV.Get(),
		D3D11_CLEAR_DEPTH |
		D3D11_CLEAR_STENCIL,
		1.f,
		0);

	D3D11_VIEWPORT previewViewport{};
	previewViewport.Width =
		static_cast<f32_t>(m_iTargetWidth);
	previewViewport.Height =
		static_cast<f32_t>(m_iTargetHeight);
	previewViewport.MinDepth = 0.f;
	previewViewport.MaxDepth = 1.f;
	m_pContext->RSSetViewports(
		1, &previewViewport);

	const HRESULT result = Render_Model();

	ID3D11RenderTargetView* restoredRTV =
		previousRTV.Get();
	m_pContext->OMSetRenderTargets(
		1, &restoredRTV, previousDSV.Get());
	if (0 != previousViewportCount)
	{
		m_pContext->RSSetViewports(
			previousViewportCount,
			previousViewports.data());
	}

	if (FAILED(result))
	{
		m_Status = "Preview model rendering failed.";
		return E_FAIL;
	}

	m_bDirty = false;
	m_Status = "Preview ready.";
	return S_OK;
}

HRESULT CMapAssetPreview::Render_Model()
{
	const f32_t aspect =
		static_cast<f32_t>(m_iTargetWidth) /
		static_cast<f32_t>(m_iTargetHeight);
	const f32_t verticalFov =
		XMConvertToRadians(45.f);
	const f32_t horizontalFov =
		2.f * std::atan(
			std::tan(verticalFov * 0.5f) * aspect);
	const f32_t limitingHalfFov =
		(std::min)(
			verticalFov,
			horizontalFov) * 0.5f;
	const f32_t distance =
		m_fBoundsRadius /
		(std::max)(
			0.1f,
			std::sin(limitingHalfFov)) *
		m_fZoom;

	const matrix_t worldMatrix =
		XMMatrixTranslation(
			-m_vBoundsCenter.x,
			-m_vBoundsCenter.y,
			-m_vBoundsCenter.z) *
		XMMatrixRotationRollPitchYaw(
			m_fPitch,
			m_fYaw,
			0.f);
	const vector_t eye =
		XMVectorSet(0.f, 0.f, -distance, 1.f);
	const matrix_t viewMatrix =
		XMMatrixLookAtLH(
			eye,
			XMVectorZero(),
			XMVectorSet(0.f, 1.f, 0.f, 0.f));
	const f32_t nearPlane =
		(std::max)(
			0.001f,
			distance - m_fBoundsRadius * 1.25f);
	const f32_t farPlane =
		distance + m_fBoundsRadius * 3.f;
	const matrix_t projectionMatrix =
		XMMatrixPerspectiveFovLH(
			verticalFov,
			aspect,
			nearPlane,
			farPlane);

	float4x4_t world{};
	float4x4_t view{};
	float4x4_t projection{};
	XMStoreFloat4x4(&world, worldMatrix);
	XMStoreFloat4x4(&view, viewMatrix);
	XMStoreFloat4x4(
		&projection, projectionMatrix);

	float3_t cameraPosition{};
	XMStoreFloat3(&cameraPosition, eye);
	const float3_t lightDirection(
		-0.45f, -0.75f, 0.35f);

	if (FAILED(m_pShader->Bind_Matrix(
			"g_WorldMatrix", &world)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ViewMatrix", &view)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrix", &projection)) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_CameraPosition",
			&cameraPosition,
			sizeof(cameraPosition))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_LightDirection",
			&lightDirection,
			sizeof(lightDirection))))
		return E_FAIL;

	for (uint32_t meshIndex = 0;
		meshIndex < m_pModel->Get_NumMeshes();
		++meshIndex)
	{
		const uint32_t hasNormalTexture =
			m_pModel->Has_MaterialTexture(
				meshIndex,
				aiTextureType_NORMALS) ?
				1u : 0u;

		if (FAILED(m_pModel->Bind_Material(
				m_pShader,
				"g_DiffuseTexture",
				meshIndex,
				aiTextureType_DIFFUSE)) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_HasNormalTexture",
				&hasNormalTexture,
				sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture &&
				FAILED(m_pModel->Bind_Material(
					m_pShader,
					"g_NormalTexture",
					meshIndex,
					aiTextureType_NORMALS))) ||
			FAILED(m_pShader->Begin(0)) ||
			FAILED(m_pModel->Render(meshIndex)))
			return E_FAIL;
	}

	return S_OK;
}

void CMapAssetPreview::Orbit(
	f32_t deltaX, f32_t deltaY)
{
	m_fYaw += deltaX * 0.01f;
	m_fPitch += deltaY * 0.01f;
	m_fPitch = (std::max)(
		XMConvertToRadians(-80.f),
		(std::min)(
			XMConvertToRadians(80.f),
			m_fPitch));
	m_bDirty = true;
}

void CMapAssetPreview::Zoom(f32_t wheelDelta)
{
	m_fZoom *= std::pow(0.9f, wheelDelta);
	m_fZoom = (std::max)(
		0.55f,
		(std::min)(4.f, m_fZoom));
	m_bDirty = true;
}

void CMapAssetPreview::Reset_Camera()
{
	m_fYaw = XMConvertToRadians(-35.f);
	m_fPitch = XMConvertToRadians(20.f);
	m_fZoom = 1.f;
	m_bDirty = true;
}

uint32_t CMapAssetPreview::Get_MeshCount() const
{
	return nullptr == m_pModel ?
		0u : m_pModel->Get_NumMeshes();
}

float3_t CMapAssetPreview::Get_Dimensions() const
{
	return float3_t(
		m_vHalfExtent.x * 2.f,
		m_vHalfExtent.y * 2.f,
		m_vHalfExtent.z * 2.f);
}
