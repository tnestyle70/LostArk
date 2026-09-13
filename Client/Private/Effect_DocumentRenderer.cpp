#include "Effect_DocumentRenderer_Internal.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Model.h"
#include "Shader.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

const Client::EFFECT_DOCUMENT_DESC&
Client::CEffectDocumentRenderer::Get_StagedDocument() const
{
	return nullptr != m_pPreparedDocument &&
		nullptr != m_pPreparedDocument->pImmutableDocument ?
		*m_pPreparedDocument->pImmutableDocument : m_Document;
}

Client::CEffectDocumentRenderer::CEffectDocumentRenderer(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffectDocumentRenderer::~CEffectDocumentRenderer() = default;

HRESULT Client::CEffectDocumentRenderer::Initialize()
{
	if (nullptr == m_pDevice || nullptr == m_pContext)
		return E_INVALIDARG;

	const std::shared_ptr<EFFECT_RENDERER_CORE> Core =
		Acquire_RendererCore(m_pDevice, m_pContext);
	if (nullptr == Core)
	{
		m_strStatus = "Effect renderer core resource creation failed.";
		return E_FAIL;
	}

	m_ShaderPrograms = Core->ShaderPrograms;
	m_pMeshShader = Core->pMeshShader;
	m_pAnimatedModelShader = Core->pAnimatedModelShader;
	m_pNativeScreenPostShader = Core->pNativeScreenPostShader;
	m_pRectShader = Core->pRectShader;
	m_pParticleShader = Core->pParticleShader;
	m_pTrailShader = Core->pTrailShader;
	m_pDecalShader = Core->pDecalShader;
	m_pRect = Core->pRect;
	m_pParticleBuffer = Core->pParticleBuffer;
	m_pWhiteTexture = Core->pWhiteTexture;
	m_pBlackTexture = Core->pBlackTexture;
	m_strStatus = "Effect renderer ready.";
	return S_OK;
}
