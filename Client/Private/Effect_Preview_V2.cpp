#include "Effect_Preview_V2.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <cmath>
#include <filesystem>

namespace
{
	constexpr const char* TEXTURE_CONSTANTS[] = {
		"g_BaseTexture", "g_NoiseTexture", "g_MaskTexture",
		"g_EmissiveTexture", "g_DissolveTexture"
	};
	constexpr const char* TEXTURE_FLAG_CONSTANTS[] = {
		"g_HasBase", "g_HasNoise", "g_HasMask", "g_HasEmissive", "g_HasDissolve"
	};
}

Client::CEffectPreviewV2::CEffectPreviewV2(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
}

Client::CEffectPreviewV2::~CEffectPreviewV2() = default;

HRESULT Client::CEffectPreviewV2::Initialize_Prototype()
{
	return S_OK;
}

std::string Client::CEffectPreviewV2::s_strLastError;

HRESULT Client::CEffectPreviewV2::Initialize(void* pArg)
{
	const auto Fail = [this](std::string strReason)
	{
		m_strStatus = std::move(strReason);
		s_strLastError = m_strStatus;
		return E_FAIL;
	};
	s_strLastError.clear();
	if (nullptr == pArg)
		return Fail("Preview desc is null.");
	if (FAILED(__super::Initialize(pArg)))
		return Fail("Transform component creation failed.");
	const DESC& Desc = *static_cast<const DESC*>(pArg);
	m_eShape = Desc.eShape;
	m_Params = Desc.Params;
	m_vPosition = Desc.vPosition;
	m_vRotationDegrees = Desc.vRotationDegrees;
	m_vScale = Desc.vScale;

	if (SHAPE::MESH == m_eShape)
	{
		const std::filesystem::path MeshPath =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(Desc.strMeshAssetId));
		if (MeshPath.empty() || !std::filesystem::is_regular_file(MeshPath))
			return Fail("Mesh asset is missing: " + Desc.strMeshAssetId);
		unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::NONANIM,
			MeshPath.string().c_str(), XMMatrixIdentity());
		m_bSkinned = false;
		if (nullptr == Model)
		{
			Model = Engine::CModel::Create(
				m_pDevice, m_pContext, MODEL::ANIM,
				MeshPath.string().c_str(), XMMatrixIdentity());
			m_bSkinned = nullptr != Model;
		}
		if (nullptr == Model)
			return Fail("Mesh load failed: " + Desc.strMeshAssetId);
		m_pModel = std::move(Model);
		unique_ptr<Engine::CShader> Shader = m_bSkinned ?
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements) :
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectMeshV2.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == Shader)
		{
			return Fail(m_bSkinned ?
				"Shader_EffectAnimMeshV2.hlsl compile failed." :
				"Shader_EffectMeshV2.hlsl compile failed.");
		}
		m_pShader = std::move(Shader);
	}
	else
	{
		unique_ptr<Engine::CVIBuffer_Rect> Rect =
			Engine::CVIBuffer_Rect::Create(m_pDevice, m_pContext);
		if (nullptr == Rect)
			return Fail("Rect buffer creation failed.");
		m_pRect = std::move(Rect);
		unique_ptr<Engine::CShader> Shader = Engine::CShader::Create(
			m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_EffectRectV2.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
		if (nullptr == Shader)
			return Fail("Shader_EffectRectV2.hlsl compile failed.");
		m_pShader = std::move(Shader);
	}

	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		if (FAILED(Load_Texture(strAssetId, m_Textures[iInput])))
			return Fail("Texture load failed: " + strAssetId);
	}
	m_strStatus = "Ready";
	Apply_Transform();
	return S_OK;
}

HRESULT Client::CEffectPreviewV2::Load_Texture(
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	return DirectX::CreateDDSTextureFromFile(
		m_pDevice.Get(), Path.c_str(), nullptr, &OutView);
}

void Client::CEffectPreviewV2::Restart()
{
	m_fTime = 0.f;
	m_bFinished = false;
}

void Client::CEffectPreviewV2::Update(const f32_t fTimeDelta)
{
	if (!m_bFinished)
	{
		m_fTime += fTimeDelta * m_Params.fPlayRate;
		if (m_Params.fLifetime > 0.f && m_fTime >= m_Params.fLifetime)
		{
			if (m_Params.bLoop)
				m_fTime = std::fmod(m_fTime, m_Params.fLifetime);
			else
				m_bFinished = true;
		}
	}
	Apply_Transform();
}

void Client::CEffectPreviewV2::Apply_Transform()
{
	const f32_t fPreScale =
		SHAPE::MESH == m_eShape ? (std::max)(0.0001f, m_Params.fMeshPreScale) : 1.f;
	const matrix_t Scale = XMMatrixScaling(
		m_vScale.x * fPreScale, m_vScale.y * fPreScale, m_vScale.z * fPreScale);
	matrix_t Rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(m_vRotationDegrees.x),
		XMConvertToRadians(m_vRotationDegrees.y),
		XMConvertToRadians(m_vRotationDegrees.z));
	if (SHAPE::SPRITE == m_eShape && m_Params.bBillboard)
	{
		const float4x4_t* pCameraWorld =
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
		if (nullptr != pCameraWorld)
		{
			matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
			CameraWorld.r[0] = XMVector3Normalize(CameraWorld.r[0]);
			CameraWorld.r[1] = XMVector3Normalize(CameraWorld.r[1]);
			CameraWorld.r[2] = XMVector3Normalize(CameraWorld.r[2]);
			CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			Rotation = Rotation * CameraWorld;
		}
	}
	const matrix_t World = Scale * Rotation *
		XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_pTransformCom->Set_State(STATE::RIGHT, World.r[0]);
	m_pTransformCom->Set_State(STATE::UP, World.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, World.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, World.r[3]);
}

void Client::CEffectPreviewV2::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_bHidden || m_bFinished || nullptr == m_pShader)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT Client::CEffectPreviewV2::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	const PARAMS& P = m_Params;
	if (FAILED(pShader->Bind_RawValue("g_Time", &m_fTime, sizeof(m_fTime))) ||
		FAILED(pShader->Bind_RawValue("g_Tint", &P.vTint, sizeof(P.vTint))) ||
		FAILED(pShader->Bind_RawValue("g_UVScale", &P.vUVScale, sizeof(P.vUVScale))) ||
		FAILED(pShader->Bind_RawValue("g_BasePan", &P.vBasePan, sizeof(P.vBasePan))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseStrength", &P.fNoiseStrength, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseScale", &P.fNoiseScale, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoisePan", &P.vNoisePan, sizeof(P.vNoisePan))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveIntensity", &P.fEmissiveIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount", &P.fDissolveAmount, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveSoftness", &P.fDissolveSoftness, sizeof(f32_t))))
	{
		return E_FAIL;
	}
	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const uint32_t iHas = nullptr != m_Textures[iInput] ? 1u : 0u;
		if (FAILED(pShader->Bind_RawValue(
			TEXTURE_FLAG_CONSTANTS[iInput], &iHas, sizeof(iHas))))
			return E_FAIL;
		if (0u != iHas &&
			FAILED(pShader->Bind_Texture(TEXTURE_CONSTANTS[iInput], m_Textures[iInput])))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectPreviewV2::Render()
{
	if (FAILED(Bind_Common(m_pShader)))
	{
		m_strStatus = "Shader bind failed.";
		return E_FAIL;
	}
	const uint32_t iPass =
		static_cast<uint32_t>(m_Params.eBlend) + (m_Params.bDepthTest ? 0u : 2u);
	if (SHAPE::MESH == m_eShape)
	{
		for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
		{
			if (m_bSkinned && FAILED(m_pModel->Bind_BoneMatrices(
				m_pShader, "g_BoneMatrices", iMesh)))
			{
				m_strStatus = "Bone matrix bind failed.";
				return E_FAIL;
			}
			if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pModel->Render(iMesh)))
			{
				m_strStatus = "Mesh draw failed.";
				return E_FAIL;
			}
		}
		return S_OK;
	}
	if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pRect->Render()))
	{
		m_strStatus = "Sprite draw failed.";
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<Client::CEffectPreviewV2> Client::CEffectPreviewV2::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectPreviewV2> Instance(new CEffectPreviewV2(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectPreviewV2::Clone(void* pArg)
{
	shared_ptr<CEffectPreviewV2> Instance(new CEffectPreviewV2(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
