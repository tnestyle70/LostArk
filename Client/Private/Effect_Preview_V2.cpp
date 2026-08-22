#include "Effect_Preview_V2.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
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

	f32_t Saturate(const f32_t fValue)
	{
		return (std::min)(1.f, (std::max)(0.f, fValue));
	}
}

float3_t Client::CEffectPreviewV2::LERP_FLOAT3::Evaluate(const f32_t fLifeRatio) const
{
	if (!bLerp)
		return vStart;
	float3_t vResult;
	XMStoreFloat3(&vResult, XMVectorLerp(
		XMLoadFloat3(&vStart), XMLoadFloat3(&vEnd), fLifeRatio));
	return vResult;
}

Client::CEffectPreviewV2::CEffectPreviewV2(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
	XMStoreFloat4x4(&m_PivotWorld, XMMatrixIdentity());
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
	m_CreationDesc = Desc;
	m_eShape = Desc.eShape;
	m_Params = Desc.Params;
	m_PivotWorld = Desc.PivotWorld;

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
		{
			return Fail("Mesh load failed: " + Desc.strMeshAssetId + " | " +
				CModelDecoderRegistry::Get().Get_LastReport().error);
		}
		m_pModel = std::move(Model);
		m_Parts.assign(m_pModel->Get_NumMeshes(), PART{});
		if (m_bSkinned && !Desc.bParamsAuthored)
			m_Params.fMeshPreScale = 0.0001f;
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
	Sync_Animation(true);
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

const std::string& Client::CEffectPreviewV2::Part_Name(const uint32_t iIndex) const
{
	static const std::string strEmpty;
	if (nullptr == m_pModel || iIndex >= m_Parts.size())
		return strEmpty;
	return m_pModel->Get_MaterialName(iIndex);
}

HRESULT Client::CEffectPreviewV2::Set_PartBase(
	const uint32_t iIndex, const std::string& strAssetId)
{
	if (iIndex >= m_Parts.size())
		return E_INVALIDARG;
	PART& Part = m_Parts[iIndex];
	if (strAssetId.empty())
	{
		Part.strBaseAssetId.clear();
		Part.pBaseView.Reset();
		return S_OK;
	}
	ComPtr<ID3D11ShaderResourceView> pView;
	if (FAILED(Load_Texture(strAssetId, pView)))
		return E_FAIL;
	Part.strBaseAssetId = strAssetId;
	Part.pBaseView = std::move(pView);
	return S_OK;
}

void Client::CEffectPreviewV2::Restart()
{
	m_fTime = 0.f;
	m_vDisplacement = { 0.f, 0.f, 0.f };
	m_bFinished = false;
	Sync_Animation(true);
}

uint32_t Client::CEffectPreviewV2::Animation_Count() const
{
	if (!m_bSkinned || nullptr == m_pModel)
		return 0u;
	return m_pModel->Get_NumAnimations();
}

const char_t* Client::CEffectPreviewV2::Animation_Name(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return nullptr;
	return m_pModel->Get_AnimationName(iIndex);
}

f32_t Client::CEffectPreviewV2::Animation_DurationSeconds(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return 0.f;
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = m_pModel->Get_AnimationTickPerSecond(iIndex);
	if (fTickPerSecond <= 0.f ||
		!m_pModel->Get_AnimationProgress(iIndex, fPosition, fDuration))
		return 0.f;
	return fDuration / fTickPerSecond;
}

bool_t Client::CEffectPreviewV2::Animation_Progress(
	f32_t& fOutSeconds, f32_t& fOutDurationSeconds) const
{
	const uint32_t iIndex = m_Params.iAnimationIndex;
	if (iIndex >= Animation_Count())
		return false;
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = m_pModel->Get_AnimationTickPerSecond(iIndex);
	if (fTickPerSecond <= 0.f ||
		!m_pModel->Get_AnimationProgress(iIndex, fPosition, fDuration))
		return false;
	fOutSeconds = fPosition / fTickPerSecond;
	fOutDurationSeconds = fDuration / fTickPerSecond;
	return true;
}

void Client::CEffectPreviewV2::Sync_Animation(const bool_t bRestart)
{
	const uint32_t iCount = Animation_Count();
	if (0u == iCount)
		return;
	if (m_Params.iAnimationIndex >= iCount)
		m_Params.iAnimationIndex = iCount - 1u;
	const uint32_t iIndex = m_Params.iAnimationIndex;
	if (bRestart || iIndex != m_iAppliedAnimationIndex)
	{
		m_pModel->Start_Animation(iIndex, m_Params.bAnimationLoop);
		m_iAppliedAnimationIndex = iIndex;
		return;
	}
	m_pModel->Set_Animation(iIndex, m_Params.bAnimationLoop);
}

f32_t Client::CEffectPreviewV2::Life_Ratio() const
{
	if (m_Params.fLifetime <= 0.f)
		return 0.f;
	return Saturate(m_fTime / m_Params.fLifetime);
}

f32_t Client::CEffectPreviewV2::Dissolve_Amount() const
{
	const f32_t fStart = Saturate(m_Params.fDissolveStart);
	if (fStart >= 1.f)
		return 0.f;
	return Saturate((Life_Ratio() - fStart) / (1.f - fStart));
}

void Client::CEffectPreviewV2::Update(const f32_t fTimeDelta)
{
	if (!m_bFinished)
	{
		const f32_t fStep = fTimeDelta * m_Params.fPlayRate;
		const float3_t vVelocity = m_Params.Velocity.Evaluate(Life_Ratio());
		m_vDisplacement.x += vVelocity.x * fStep;
		m_vDisplacement.y += vVelocity.y * fStep;
		m_vDisplacement.z += vVelocity.z * fStep;
		m_fTime += fStep;
		Sync_Animation(false);
		if (0u != Animation_Count())
			m_pModel->Play_Animation(fStep);
		if (m_Params.fLifetime > 0.f && m_fTime >= m_Params.fLifetime)
		{
			if (m_Params.bLoop)
			{
				m_fTime = std::fmod(m_fTime, m_Params.fLifetime);
				m_vDisplacement = { 0.f, 0.f, 0.f };
				Sync_Animation(true);
			}
			else
				m_bFinished = true;
		}
	}
	Apply_Transform();
}

void Client::CEffectPreviewV2::Apply_Transform()
{
	const f32_t fRatio = Life_Ratio();
	const float3_t vPosition = m_Params.Position.Evaluate(fRatio);
	const float3_t vRotation = m_Params.Rotation.Evaluate(fRatio);
	const float3_t vScale = m_Params.Scale.Evaluate(fRatio);
	const f32_t fPreScale =
		SHAPE::MESH == m_eShape ? (std::max)(0.0001f, m_Params.fMeshPreScale) : 1.f;
	const matrix_t Scale = XMMatrixScaling(
		vScale.x * fPreScale, vScale.y * fPreScale, vScale.z * fPreScale);
	const matrix_t Rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vRotation.x),
		XMConvertToRadians(vRotation.y),
		XMConvertToRadians(vRotation.z));
	const matrix_t LocalTranslation = XMMatrixTranslation(
		vPosition.x + m_vDisplacement.x,
		vPosition.y + m_vDisplacement.y,
		vPosition.z + m_vDisplacement.z);
	const matrix_t Pivot = XMLoadFloat4x4(&m_PivotWorld);
	matrix_t World = Scale * Rotation * LocalTranslation * Pivot;
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
			World = Scale * Rotation * CameraWorld *
				XMMatrixTranslationFromVector(World.r[3]);
		}
	}
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
	const uint32_t iColorClipChannel = static_cast<uint32_t>(P.eColorClipChannel);
	const f32_t fDissolveAmount = Dissolve_Amount();
	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4_t vCameraPosition =
		nullptr != pCameraPosition ? *pCameraPosition : float4_t(0.f, 0.f, 0.f, 1.f);
	if (FAILED(pShader->Bind_RawValue("g_vCamPosition", &vCameraPosition, sizeof(vCameraPosition))) ||
		FAILED(pShader->Bind_RawValue("g_RimColor", &P.vRimColor, sizeof(P.vRimColor))) ||
		FAILED(pShader->Bind_RawValue("g_RimPower", &P.fRimPower, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_RimIntensity", &P.fRimIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_GhostAlpha", &P.fGhostAlpha, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_Time", &m_fTime, sizeof(m_fTime))) ||
		FAILED(pShader->Bind_RawValue("g_ColorMul", &P.vColorMul, sizeof(P.vColorMul))) ||
		FAILED(pShader->Bind_RawValue("g_ColorOffset", &P.vColorOffset, sizeof(P.vColorOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClip", &P.fColorClip, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClipChannel", &iColorClipChannel, sizeof(iColorClipChannel))) ||
		FAILED(pShader->Bind_RawValue("g_BloomIntensity", &P.fBloomIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DistortionIntensity", &P.fDistortionIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_UVStart", &P.vUVStart, sizeof(P.vUVStart))) ||
		FAILED(pShader->Bind_RawValue("g_UVSpeed", &P.vUVSpeed, sizeof(P.vUVSpeed))) ||
		FAILED(pShader->Bind_RawValue("g_UVTileCount", &P.vUVTileCount, sizeof(P.vUVTileCount))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseStrength", &P.fNoiseStrength, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseScale", &P.fNoiseScale, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoisePan", &P.vNoisePan, sizeof(P.vNoisePan))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount", &fDissolveAmount, sizeof(f32_t))) ||
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
	const uint32_t iPass = BLEND_MODE::SOLID == m_Params.eBlend ? 4u :
		static_cast<uint32_t>(m_Params.eBlend) + (m_Params.bDepthTest ? 0u : 2u);
	if (SHAPE::MESH == m_eShape)
	{
		for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
		{
			if (iMesh < m_Parts.size() && !m_Parts[iMesh].bVisible)
				continue;
			const ComPtr<ID3D11ShaderResourceView>& pBase =
				(iMesh < m_Parts.size() && nullptr != m_Parts[iMesh].pBaseView) ?
				m_Parts[iMesh].pBaseView :
				m_Textures[static_cast<size_t>(TEXTURE_INPUT::BASE)];
			const uint32_t iHasBase = nullptr != pBase ? 1u : 0u;
			if (FAILED(m_pShader->Bind_RawValue("g_HasBase", &iHasBase, sizeof(iHasBase))) ||
				(0u != iHasBase && FAILED(m_pShader->Bind_Texture("g_BaseTexture", pBase))))
			{
				m_strStatus = "Part base bind failed.";
				return E_FAIL;
			}
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
