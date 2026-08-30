#include "EffectV2_Object.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"
#include "Valtan.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>
#include <span>
#include <unordered_map>

namespace
{
	constexpr const char* TEXTURE_CONSTANTS[] = {
		"g_BaseTexture", "g_NoiseTexture", "g_MaskTexture",
		"g_EmissiveTexture", "g_DissolveTexture"
	};
	constexpr const char* TEXTURE_FLAG_CONSTANTS[] = {
		"g_HasBase", "g_HasNoise", "g_HasMask", "g_HasEmissive", "g_HasDissolve"
	};
	constexpr uint32_t MAX_PARTICLE_CAPACITY = 2048u;
	constexpr uint32_t MAX_TRAIL_POINTS = 4096u;
	constexpr f32_t PI_F = 3.14159265358979f;

	f32_t Saturate(const f32_t fValue)
	{
		return (std::min)(1.f, (std::max)(0.f, fValue));
	}

	float3_t To_Float3(const vector_t Value)
	{
		float3_t vResult;
		XMStoreFloat3(&vResult, Value);
		return vResult;
	}

	bool_t Normalize_Safe(const vector_t Value, vector_t& OutNormalized)
	{
		const f32_t fLengthSq = XMVectorGetX(XMVector3LengthSq(Value));
		if (fLengthSq <= 1e-12f || !std::isfinite(fLengthSq))
			return false;
		OutNormalized = XMVector3Normalize(Value);
		return true;
	}
}

float3_t Client::CEffectV2Object::LERP_FLOAT3::Evaluate(const f32_t fLifeRatio) const
{
	if (!bLerp)
		return vStart;
	float3_t vResult;
	XMStoreFloat3(&vResult, XMVectorLerp(
		XMLoadFloat3(&vStart), XMLoadFloat3(&vEnd), fLifeRatio));
	return vResult;
}

Client::CEffectV2Object::CEffectV2Object(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
	XMStoreFloat4x4(&m_PivotWorld, XMMatrixIdentity());
}

Client::CEffectV2Object::~CEffectV2Object() = default;

HRESULT Client::CEffectV2Object::Initialize_Prototype()
{
	return S_OK;
}

std::string Client::CEffectV2Object::s_strLastError;

HRESULT Client::CEffectV2Object::Initialize(void* pArg)
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

	std::string strError;
	switch (m_eShape)
	{
	case SHAPE::MESH:
		if (FAILED(Acquire_Model(m_pDevice, m_pContext, Desc.strMeshAssetId,
			m_pModel, m_bSkinned, strError)))
			return Fail(strError);
		m_Parts.assign(m_pModel->Get_NumMeshes(), PART{});
		if (m_bSkinned && !Desc.bParamsAuthored)
			m_Params.fMeshPreScale = 0.0001f;
		break;
	case SHAPE::SPRITE:
	case SHAPE::DECAL:
	{
		unique_ptr<Engine::CVIBuffer_Rect> Rect =
			Engine::CVIBuffer_Rect::Create(m_pDevice, m_pContext);
		if (nullptr == Rect)
			return Fail("Rect buffer creation failed.");
		m_pRect = std::move(Rect);
		break;
	}
	case SHAPE::PARTICLE:
	{
		const uint32_t iCapacity = (std::min)(MAX_PARTICLE_CAPACITY,
			(std::max)(1u, m_Params.Particle.iMaxParticles));
		unique_ptr<Engine::CVIBuffer_ParticleRect> Buffer =
			Engine::CVIBuffer_ParticleRect::Create(m_pDevice, m_pContext, iCapacity);
		if (nullptr == Buffer)
			return Fail("Particle buffer creation failed.");
		m_pParticleBuffer = std::move(Buffer);
		m_Particles.reserve(iCapacity);
		m_ParticleInstances.reserve(iCapacity);
		m_iRandomState = (std::max)(1u, m_Params.Particle.iRandomSeed);
		break;
	}
	case SHAPE::TRAIL:
	{
		const uint32_t iMaxPoints = (std::min)(MAX_TRAIL_POINTS,
			(std::max)(2u, m_Params.Trail.iMaxPoints));
		unique_ptr<Engine::CVIBuffer_DynamicTrail> Buffer =
			Engine::CVIBuffer_DynamicTrail::Create(m_pDevice, m_pContext, iMaxPoints);
		if (nullptr == Buffer)
			return Fail("Trail buffer creation failed.");
		m_pTrailBuffer = std::move(Buffer);
		m_iTrailBufferPoints = iMaxPoints;
		break;
	}
	case SHAPE::SCREEN_POST:
		break;
	default:
		return Fail("Unknown effect shape.");
	}
	if (FAILED(Acquire_ShapeShader(m_pDevice, m_pContext, m_eShape, m_bSkinned, m_pShader, strError)))
		return Fail(strError);

	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		if (FAILED(Load_Texture(strAssetId,
			Is_ColorInput(static_cast<TEXTURE_INPUT>(iInput)), m_Textures[iInput])))
			return Fail("Texture load failed: " + strAssetId);
	}
	Sync_Animation(true);
	m_strStatus = "Ready";
	Apply_Transform();
	return S_OK;
}

HRESULT Client::CEffectV2Object::Load_Texture(
	const std::string& strAssetId,
	const bool_t bColorTexture,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	return Acquire_Texture(m_pDevice, strAssetId,
		bColorTexture && m_Params.bColorTexturesSRGB, OutView);
}

HRESULT Client::CEffectV2Object::Reload_ColorTextures()
{
	HRESULT hResult = S_OK;
	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		if (!Is_ColorInput(static_cast<TEXTURE_INPUT>(iInput)))
			continue;
		const std::string& strAssetId = m_CreationDesc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(Load_Texture(strAssetId, true, pView)))
		{
			hResult = E_FAIL;
			continue;
		}
		m_Textures[iInput] = std::move(pView);
	}
	for (PART& Part : m_Parts)
	{
		if (Part.strBaseAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(Load_Texture(Part.strBaseAssetId, true, pView)))
		{
			hResult = E_FAIL;
			continue;
		}
		Part.pBaseView = std::move(pView);
	}
	return hResult;
}

namespace
{
	struct MODEL_CACHE_ENTRY final
	{
		shared_ptr<Engine::CModel> pPrototype;
		bool_t bSkinned = false;
	};
	std::unordered_map<std::string, MODEL_CACHE_ENTRY> g_ModelCache;
	std::map<wstring_t, shared_ptr<Engine::CShader>> g_ShaderCache;
	std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> g_TextureCache;
}

HRESULT Client::CEffectV2Object::Acquire_Model(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const std::string& strAssetId,
	shared_ptr<Engine::CModel>& OutModel,
	bool_t& bOutSkinned,
	std::string& strOutError)
{
	auto Found = g_ModelCache.find(strAssetId);
	if (Found == g_ModelCache.end())
	{
		const std::filesystem::path MeshPath =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (MeshPath.empty() || !std::filesystem::is_regular_file(MeshPath))
		{
			strOutError = "Mesh asset is missing: " + strAssetId;
			return E_FAIL;
		}
		MODEL_CACHE_ENTRY Entry;
		unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
			pDevice, pContext, MODEL::NONANIM,
			MeshPath.string().c_str(), XMMatrixIdentity());
		if (nullptr == Model)
		{
			Model = Engine::CModel::Create(
				pDevice, pContext, MODEL::ANIM,
				MeshPath.string().c_str(), XMMatrixIdentity());
			Entry.bSkinned = nullptr != Model;
		}
		if (nullptr == Model)
		{
			strOutError = "Mesh load failed: " + strAssetId + " | " +
				CModelDecoderRegistry::Get().Get_LastReport().error;
			return E_FAIL;
		}
		Entry.pPrototype = std::move(Model);
		Found = g_ModelCache.emplace(strAssetId, std::move(Entry)).first;
	}
	const shared_ptr<Engine::CModel> pClone =
		std::static_pointer_cast<Engine::CModel>(Found->second.pPrototype->Clone(nullptr));
	if (nullptr == pClone)
	{
		strOutError = "Mesh clone failed: " + strAssetId;
		return E_FAIL;
	}
	OutModel = pClone;
	bOutSkinned = Found->second.bSkinned;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_Shader(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const wstring_t& strFilePath,
	const D3D11_INPUT_ELEMENT_DESC* pElements,
	const uint32_t iNumElements,
	shared_ptr<Engine::CShader>& OutShader)
{
	auto Found = g_ShaderCache.find(strFilePath);
	if (Found == g_ShaderCache.end())
	{
		unique_ptr<Engine::CShader> Shader = Engine::CShader::Create(
			pDevice, pContext, strFilePath.c_str(), pElements, iNumElements);
		if (nullptr == Shader)
			return E_FAIL;
		Found = g_ShaderCache.emplace(strFilePath, std::move(Shader)).first;
	}
	OutShader = Found->second;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_ShapeShader(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const SHAPE eShape,
	const bool_t bSkinned,
	shared_ptr<Engine::CShader>& OutShader,
	std::string& strOutError)
{
	const wchar_t* pFile = nullptr;
	const D3D11_INPUT_ELEMENT_DESC* pElements = nullptr;
	uint32_t iNumElements = 0u;
	switch (eShape)
	{
	case SHAPE::MESH:
		pFile = bSkinned ?
			TEXT("../Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl") :
			TEXT("../Bin/ShaderFiles/Shader_EffectMeshV2.hlsl");
		pElements = bSkinned ? VTXANIMMESH::Elements : VTXMESH::Elements;
		iNumElements = bSkinned ? VTXANIMMESH::iNumElements : VTXMESH::iNumElements;
		break;
	case SHAPE::SPRITE:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectRectV2.hlsl");
		pElements = VTXTEX::Elements;
		iNumElements = VTXTEX::iNumElements;
		break;
	case SHAPE::PARTICLE:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectParticleV2.hlsl");
		pElements = Engine::VTXEFFECT_PARTICLE::Elements;
		iNumElements = Engine::VTXEFFECT_PARTICLE::iNumElements;
		break;
	case SHAPE::DECAL:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectDecalV2.hlsl");
		pElements = VTXTEX::Elements;
		iNumElements = VTXTEX::iNumElements;
		break;
	case SHAPE::TRAIL:
		pFile = TEXT("../Bin/ShaderFiles/Shader_EffectTrailV2.hlsl");
		pElements = Engine::VTXEFFECT_TRAIL::Elements;
		iNumElements = Engine::VTXEFFECT_TRAIL::iNumElements;
		break;
	case SHAPE::SCREEN_POST:
		OutShader.reset();
		return S_OK;
	default:
		strOutError = "Unknown effect shape.";
		return E_FAIL;
	}
	if (FAILED(Acquire_Shader(pDevice, pContext, pFile, pElements, iNumElements, OutShader)))
	{
		strOutError = "Effect v2 shader compile failed: " +
			std::filesystem::path(pFile).filename().string();
		return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_Texture(
	const ComPtr<ID3D11Device>& pDevice,
	const std::string& strAssetId,
	const bool_t bSRGB,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	const std::string strCacheKey = strAssetId + (bSRGB ? "|srgb" : "|linear");
	auto Found = g_TextureCache.find(strCacheKey);
	if (Found == g_TextureCache.end())
	{
		const std::filesystem::path Path =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (Path.empty() || !std::filesystem::is_regular_file(Path))
			return E_FAIL;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(DirectX::CreateDDSTextureFromFileEx(
			pDevice.Get(), Path.c_str(), 0u, D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
			bSRGB ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB,
			nullptr, &pView)))
			return E_FAIL;
		Found = g_TextureCache.emplace(strCacheKey, std::move(pView)).first;
	}
	OutView = Found->second;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Prewarm(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const DESC& Desc,
	std::string& strOutError)
{
	bool_t bSkinned = false;
	if (SHAPE::MESH == Desc.eShape)
	{
		shared_ptr<Engine::CModel> pModel;
		if (FAILED(Acquire_Model(pDevice, pContext, Desc.strMeshAssetId, pModel, bSkinned, strOutError)))
			return E_FAIL;
	}
	shared_ptr<Engine::CShader> pShader;
	if (FAILED(Acquire_ShapeShader(pDevice, pContext, Desc.eShape, bSkinned, pShader, strOutError)))
		return E_FAIL;
	for (size_t iInput = 0u; iInput < Desc.TextureAssetIds.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		const bool_t bSRGB = Desc.Params.bColorTexturesSRGB &&
			Is_ColorInput(static_cast<TEXTURE_INPUT>(iInput));
		if (FAILED(Acquire_Texture(pDevice, strAssetId, bSRGB, pView)))
		{
			strOutError = "Texture load failed: " + strAssetId;
			return E_FAIL;
		}
	}
	return S_OK;
}

void Client::CEffectV2Object::Clear_ResourceCache()
{
	g_ModelCache.clear();
	g_ShaderCache.clear();
	g_TextureCache.clear();
}

const std::string& Client::CEffectV2Object::Part_Name(const uint32_t iIndex) const
{
	static const std::string strEmpty;
	if (nullptr == m_pModel || iIndex >= m_Parts.size())
		return strEmpty;
	return m_pModel->Get_MaterialName(iIndex);
}

HRESULT Client::CEffectV2Object::Set_PartBase(
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
	if (FAILED(Load_Texture(strAssetId, true, pView)))
		return E_FAIL;
	Part.strBaseAssetId = strAssetId;
	Part.pBaseView = std::move(pView);
	return S_OK;
}

void Client::CEffectV2Object::Restart()
{
	m_fTime = 0.f;
	m_vDisplacement = { 0.f, 0.f, 0.f };
	m_bFinished = false;
	m_bEmissionStopped = false;
	m_Particles.clear();
	m_fSpawnAccumulator = 0.f;
	m_bBurstPending = true;
	m_iRandomState = (std::max)(1u, m_Params.Particle.iRandomSeed);
	m_TrailPoints.clear();
	m_fTrailSampleAccumulator = 0.f;
	m_fTrailCumulativeDistance = 0.f;
	Sync_Animation(true);
}

uint32_t Client::CEffectV2Object::Animation_Count() const
{
	if (!m_bSkinned || nullptr == m_pModel)
		return 0u;
	return m_pModel->Get_NumAnimations();
}

const char_t* Client::CEffectV2Object::Animation_Name(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return nullptr;
	return m_pModel->Get_AnimationName(iIndex);
}

f32_t Client::CEffectV2Object::Animation_DurationSeconds(const uint32_t iIndex) const
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

bool_t Client::CEffectV2Object::Animation_Progress(
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

void Client::CEffectV2Object::Sync_Animation(const bool_t bRestart)
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

f32_t Client::CEffectV2Object::Life_Ratio() const
{
	if (m_Params.fLifetime <= 0.f)
		return 0.f;
	return Saturate(m_fTime / m_Params.fLifetime);
}

f32_t Client::CEffectV2Object::Dissolve_Amount() const
{
	if (m_Params.fLifetime <= 0.f)
		return 0.f;
	const f32_t fRatio = Life_Ratio();
	const f32_t fInEnd = Saturate(m_Params.fDissolveInEnd);
	if (0.f < fInEnd && fRatio < fInEnd)
		return Saturate(1.f - fRatio / fInEnd);
	const f32_t fStart = (std::max)(Saturate(m_Params.fDissolveStart), fInEnd);
	if (fStart >= 1.f)
		return 0.f;
	return Saturate((fRatio - fStart) / (1.f - fStart));
}

Client::EFFECT_V2_TARGET Client::EFFECT_V2_TARGET::From_Npc(
	const std::shared_ptr<CNpc>& pNpc)
{
	EFFECT_V2_TARGET Target;
	if (nullptr == pNpc)
		return Target;
	Target.eKind = EFFECT_V2_TARGET_KIND::NPC;
	Target.pOwner = pNpc;
	Target.pKey = pNpc.get();
	return Target;
}

Client::EFFECT_V2_TARGET Client::EFFECT_V2_TARGET::From_Valtan(
	const std::shared_ptr<CValtan>& pValtan)
{
	EFFECT_V2_TARGET Target;
	if (nullptr == pValtan)
		return Target;
	Target.eKind = EFFECT_V2_TARGET_KIND::VALTAN;
	Target.pOwner = pValtan;
	Target.pKey = pValtan.get();
	return Target;
}

void Client::CEffectV2Object::Set_FollowTarget(
	const EFFECT_V2_TARGET& Target,
	std::string strBone,
	const PIVOT_ROTATION eRotation)
{
	m_FollowTarget = Target;
	m_strFollowBone = std::move(strBone);
	m_eFollowRotation = eRotation;
	m_bFollowTarget = m_FollowTarget.Is_Valid();
}

void Client::CEffectV2Object::Clear_FollowTarget()
{
	m_bFollowTarget = false;
	m_FollowTarget.Reset();
	m_strFollowBone.clear();
	XMStoreFloat4x4(&m_FollowLocal, XMMatrixIdentity());
}

bool_t Client::CEffectV2Object::Resolve_TargetView(
	const EFFECT_V2_TARGET& Target,
	EFFECT_V2_TARGET_VIEW& OutView)
{
	const std::shared_ptr<CGameObject> pOwner = Target.pOwner.lock();
	if (nullptr == pOwner)
		return false;
	switch (Target.eKind)
	{
	case EFFECT_V2_TARGET_KIND::NPC:
	{
		const std::shared_ptr<CNpc> pNpc = std::static_pointer_cast<CNpc>(pOwner);
		if (nullptr == pNpc->Get_Model() || nullptr == pNpc->Get_Transform())
			return false;
		OutView.pModel = pNpc->Get_Model();
		OutView.BoneRoot = *pNpc->Get_Transform()->Get_WorldMatrixPtr();
		OutView.YawBasis = OutView.BoneRoot;
		return true;
	}
	case EFFECT_V2_TARGET_KIND::VALTAN:
	{
		const std::shared_ptr<CValtan> pValtan = std::static_pointer_cast<CValtan>(pOwner);
		if (nullptr == pValtan->Get_BodyModel() || nullptr == pValtan->Get_Transform() ||
			!pValtan->Try_Get_PresentationRootMatrix(&OutView.BoneRoot))
		{
			return false;
		}
		OutView.pModel = pValtan->Get_BodyModel();
		OutView.YawBasis = *pValtan->Get_Transform()->Get_WorldMatrixPtr();
		return true;
	}
	default:
		return false;
	}
}

bool_t Client::CEffectV2Object::Resolve_TargetPivot(
	const EFFECT_V2_TARGET_VIEW& View,
	const std::string& strBone,
	const PIVOT_ROTATION eRotation,
	float4x4_t& OutPivot)
{
	if (nullptr == View.pModel)
		return false;
	const matrix_t BoneRoot = XMLoadFloat4x4(&View.BoneRoot);
	const matrix_t TargetWorld = XMLoadFloat4x4(&View.YawBasis);
	matrix_t Pivot = BoneRoot;
	if (!strBone.empty())
	{
		if (!View.pModel->Has_Bone(strBone.c_str()))
			return false;
		Pivot = View.pModel->Get_BoneMatrix(strBone.c_str()) * BoneRoot;
	}
	const vector_t Translation = XMVectorSetW(Pivot.r[3], 1.f);
	if (PIVOT_ROTATION::BONE == eRotation)
	{
		const vector_t Right = XMVector3Normalize(Pivot.r[0]);
		const vector_t Up = XMVector3Normalize(Pivot.r[1]);
		const vector_t Look = XMVector3Normalize(Pivot.r[2]);
		if (XMVectorGetX(XMVector3LengthSq(Right)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Up)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Look)) > 0.f)
		{
			Pivot.r[0] = Right;
			Pivot.r[1] = Up;
			Pivot.r[2] = Look;
		}
		else
			Pivot = XMMatrixIdentity();
	}
	else if (PIVOT_ROTATION::TARGET_YAW == eRotation)
	{
		const vector_t Right = XMVector3Normalize(TargetWorld.r[0]);
		const vector_t Up = XMVector3Normalize(TargetWorld.r[1]);
		const vector_t Look = XMVector3Normalize(TargetWorld.r[2]);
		Pivot.r[0] = Right;
		Pivot.r[1] = Up;
		Pivot.r[2] = Look;
	}
	else
		Pivot = XMMatrixIdentity();
	Pivot.r[3] = Translation;
	XMStoreFloat4x4(&OutPivot, Pivot);
	return true;
}

void Client::CEffectV2Object::Stop_Emission()
{
	if (SHAPE::PARTICLE == m_eShape || SHAPE::TRAIL == m_eShape)
		m_bEmissionStopped = true;
	else
		m_bFinished = true;
}

void Client::CEffectV2Object::Update(const f32_t fTimeDelta)
{
	if (m_bFollowTarget)
	{
		EFFECT_V2_TARGET_VIEW View;
		float4x4_t Pivot;
		if (!Resolve_TargetView(m_FollowTarget, View) ||
			!Resolve_TargetPivot(View, m_strFollowBone, m_eFollowRotation, Pivot))
		{
			m_bFinished = true;
		}
		else
		{
			XMStoreFloat4x4(&m_PivotWorld,
				XMLoadFloat4x4(&m_FollowLocal) * XMLoadFloat4x4(&Pivot));
		}
	}
	if (!m_bFinished)
	{
		const f32_t fStep = fTimeDelta * m_Params.fPlayRate;
		if (!m_bEmissionStopped)
		{
			const float3_t vVelocity = m_Params.Velocity.Evaluate(Life_Ratio());
			m_vDisplacement.x += vVelocity.x * fStep;
			m_vDisplacement.y += vVelocity.y * fStep;
			m_vDisplacement.z += vVelocity.z * fStep;
			m_fTime += fStep;
			Sync_Animation(false);
			if (0u != Animation_Count())
				m_pModel->Play_Animation(fStep);
		}
		Apply_Transform();
		if (SHAPE::PARTICLE == m_eShape)
			Update_Particles(fStep);
		else if (SHAPE::TRAIL == m_eShape)
			Update_Trail(fStep);
		if (!m_bEmissionStopped)
			Advance_Lifetime(fStep);
		else if ((SHAPE::PARTICLE == m_eShape && m_Particles.empty()) ||
			(SHAPE::TRAIL == m_eShape && m_TrailPoints.empty()))
			m_bFinished = true;
	}
	Apply_Transform();
}

void Client::CEffectV2Object::Advance_Lifetime(const f32_t fStep)
{
	UNREFERENCED_PARAMETER(fStep);
	if (m_Params.fLifetime <= 0.f || m_fTime < m_Params.fLifetime)
		return;
	if (m_Params.bLoop)
	{
		m_fTime = std::fmod(m_fTime, m_Params.fLifetime);
		m_vDisplacement = { 0.f, 0.f, 0.f };
		m_bBurstPending = true;
		Sync_Animation(true);
		return;
	}
	if (SHAPE::PARTICLE == m_eShape || SHAPE::TRAIL == m_eShape)
		m_bEmissionStopped = true;
	else
		m_bFinished = true;
}

void Client::CEffectV2Object::Apply_Transform()
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

f32_t Client::CEffectV2Object::Random_01()
{
	uint32_t iState = m_iRandomState;
	iState ^= iState << 13;
	iState ^= iState >> 17;
	iState ^= iState << 5;
	m_iRandomState = 0u == iState ? 1u : iState;
	return static_cast<f32_t>(m_iRandomState & 0x00FFFFFFu) / 16777216.f;
}

f32_t Client::CEffectV2Object::Random_Range(const f32_t fMinimum, const f32_t fMaximum)
{
	return fMinimum + (fMaximum - fMinimum) * Random_01();
}

void Client::CEffectV2Object::Spawn_Particle()
{
	const PARTICLE_PARAMS& P = m_Params.Particle;
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY, (std::max)(1u, P.iMaxParticles));
	if (m_Particles.size() >= iMax)
		return;

	vector_t Position = XMVectorZero();
	switch (P.eSpawnShape)
	{
	case PARTICLE_SPAWN_SHAPE::SPHERE:
	{
		const f32_t fZ = Random_Range(-1.f, 1.f);
		const f32_t fPhi = Random_Range(0.f, 2.f * PI_F);
		const f32_t fRing = std::sqrt((std::max)(0.f, 1.f - fZ * fZ));
		const f32_t fRadius = P.fSpawnInnerRadius +
			(P.fSpawnRadius - P.fSpawnInnerRadius) * std::cbrt(Random_01());
		Position = XMVectorSet(fRing * std::cos(fPhi), fZ, fRing * std::sin(fPhi), 0.f) * fRadius;
		break;
	}
	case PARTICLE_SPAWN_SHAPE::RING:
	{
		const f32_t fAngle = XMConvertToRadians(Random_Range(0.f, P.fSpawnArcDegrees));
		const f32_t fRadius = P.fSpawnInnerRadius +
			(P.fSpawnRadius - P.fSpawnInnerRadius) * std::sqrt(Random_01());
		Position = XMVectorSet(std::cos(fAngle) * fRadius, 0.f, std::sin(fAngle) * fRadius, 0.f);
		break;
	}
	case PARTICLE_SPAWN_SHAPE::BOX:
		Position = XMVectorSet(
			Random_Range(-P.vSpawnExtents.x, P.vSpawnExtents.x),
			Random_Range(-P.vSpawnExtents.y, P.vSpawnExtents.y),
			Random_Range(-P.vSpawnExtents.z, P.vSpawnExtents.z), 0.f);
		break;
	default:
		break;
	}

	vector_t Velocity = XMVectorZero();
	switch (P.eVelocityMode)
	{
	case PARTICLE_VELOCITY_MODE::FIXED:
		Velocity = XMVectorSet(
			Random_Range(P.vVelocityMin.x, P.vVelocityMax.x),
			Random_Range(P.vVelocityMin.y, P.vVelocityMax.y),
			Random_Range(P.vVelocityMin.z, P.vVelocityMax.z), 0.f);
		break;
	case PARTICLE_VELOCITY_MODE::OUTWARD:
	{
		vector_t Direction;
		if (!Normalize_Safe(Position, Direction))
		{
			const f32_t fZ = Random_Range(-1.f, 1.f);
			const f32_t fPhi = Random_Range(0.f, 2.f * PI_F);
			const f32_t fRing = std::sqrt((std::max)(0.f, 1.f - fZ * fZ));
			Direction = XMVectorSet(fRing * std::cos(fPhi), fZ, fRing * std::sin(fPhi), 0.f);
		}
		Velocity = Direction * Random_Range(P.vSpeedRange.x, P.vSpeedRange.y);
		break;
	}
	case PARTICLE_VELOCITY_MODE::CONE:
	{
		const f32_t fCosHalf = std::cos(XMConvertToRadians(
			(std::min)(180.f, (std::max)(0.f, P.fConeAngleDegrees))));
		const f32_t fCosTheta = 1.f + (fCosHalf - 1.f) * Random_01();
		const f32_t fSinTheta = std::sqrt((std::max)(0.f, 1.f - fCosTheta * fCosTheta));
		const f32_t fPhi = Random_Range(0.f, 2.f * PI_F);
		Velocity = XMVectorSet(fSinTheta * std::cos(fPhi), fCosTheta, fSinTheta * std::sin(fPhi), 0.f) *
			Random_Range(P.vSpeedRange.x, P.vSpeedRange.y);
		break;
	}
	default:
		break;
	}

	if (!P.bLocalSpace)
	{
		const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
		Position = XMVector3TransformCoord(Position, World);
		Velocity = XMVector3TransformNormal(Velocity, World);
	}

	PARTICLE Particle;
	Particle.vPosition = To_Float3(Position);
	Particle.vVelocity = To_Float3(Velocity);
	Particle.fAge = 0.f;
	Particle.fLifetime = (std::max)(0.001f, Random_Range(P.vLifetime.x, P.vLifetime.y));
	Particle.fRotationDegrees = Random_Range(P.vRotationRange.x, P.vRotationRange.y);
	Particle.fSpinDegrees = Random_Range(P.vSpinRange.x, P.vSpinRange.y);
	m_Particles.push_back(Particle);
}

void Client::CEffectV2Object::Update_Particles(const f32_t fStep)
{
	const PARTICLE_PARAMS& P = m_Params.Particle;
	if (fStep > 0.f)
	{
		for (PARTICLE& Particle : m_Particles)
			Particle.fAge += fStep;
		m_Particles.erase(std::remove_if(m_Particles.begin(), m_Particles.end(),
			[](const PARTICLE& Particle) { return Particle.fAge >= Particle.fLifetime; }),
			m_Particles.end());
		const f32_t fDragFactor = (std::max)(0.f, 1.f - P.fDrag * fStep);
		for (PARTICLE& Particle : m_Particles)
		{
			Particle.vVelocity.x = (Particle.vVelocity.x + P.vAcceleration.x * fStep) * fDragFactor;
			Particle.vVelocity.y = (Particle.vVelocity.y + P.vAcceleration.y * fStep) * fDragFactor;
			Particle.vVelocity.z = (Particle.vVelocity.z + P.vAcceleration.z * fStep) * fDragFactor;
			Particle.vPosition.x += Particle.vVelocity.x * fStep;
			Particle.vPosition.y += Particle.vVelocity.y * fStep;
			Particle.vPosition.z += Particle.vVelocity.z * fStep;
			Particle.fRotationDegrees += Particle.fSpinDegrees * fStep;
		}
	}
	if (m_bEmissionStopped)
		return;
	if (m_bBurstPending)
	{
		for (uint32_t iIndex = 0u; iIndex < P.iBurstCount; ++iIndex)
			Spawn_Particle();
		m_bBurstPending = false;
	}
	m_fSpawnAccumulator += (std::max)(0.f, P.fSpawnRate) * fStep;
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY, (std::max)(1u, P.iMaxParticles));
	while (m_fSpawnAccumulator >= 1.f && m_Particles.size() < iMax)
	{
		Spawn_Particle();
		m_fSpawnAccumulator -= 1.f;
	}
	m_fSpawnAccumulator = (std::min)(m_fSpawnAccumulator, 1.f);
}

HRESULT Client::CEffectV2Object::Build_ParticleInstances()
{
	const PARTICLE_PARAMS& P = m_Params.Particle;
	const uint32_t iMax = (std::min)(MAX_PARTICLE_CAPACITY, (std::max)(1u, P.iMaxParticles));
	if (nullptr == m_pParticleBuffer || m_pParticleBuffer->Get_Capacity() < iMax)
	{
		unique_ptr<Engine::CVIBuffer_ParticleRect> Buffer =
			Engine::CVIBuffer_ParticleRect::Create(m_pDevice, m_pContext, iMax);
		if (nullptr == Buffer)
			return E_FAIL;
		m_pParticleBuffer = std::move(Buffer);
	}
	const float4x4_t* pCameraWorld = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == pCameraWorld)
		return E_FAIL;
	const matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
	const vector_t CameraRight = XMVector3Normalize(CameraWorld.r[0]);
	const vector_t CameraUp = XMVector3Normalize(CameraWorld.r[1]);
	const vector_t CameraLook = XMVector3Normalize(CameraWorld.r[2]);
	const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const uint32_t iColumns = (std::max)(1u, P.iTileColumns);
	const uint32_t iRows = (std::max)(1u, P.iTileRows);
	const uint32_t iFrames = iColumns * iRows;

	m_ParticleInstances.clear();
	for (const PARTICLE& Particle : m_Particles)
	{
		const f32_t fLife = Saturate(Particle.fAge / Particle.fLifetime);
		vector_t Position = XMLoadFloat3(&Particle.vPosition);
		vector_t Velocity = XMLoadFloat3(&Particle.vVelocity);
		if (P.bLocalSpace)
		{
			Position = XMVector3TransformCoord(Position, World);
			Velocity = XMVector3TransformNormal(Velocity, World);
		}
		vector_t Right = CameraRight;
		vector_t Up = CameraUp;
		vector_t Look = CameraLook;
		if (PARTICLE_ALIGNMENT::VELOCITY == P.eAlignment)
		{
			const vector_t Planar = Velocity - CameraLook * XMVector3Dot(Velocity, CameraLook);
			vector_t Axis;
			if (Normalize_Safe(Planar, Axis))
			{
				Up = Axis;
				Right = XMVector3Normalize(XMVector3Cross(Up, CameraLook));
			}
		}
		else if (PARTICLE_ALIGNMENT::HORIZONTAL == P.eAlignment)
		{
			Right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
			Up = XMVectorSet(0.f, 0.f, 1.f, 0.f);
			Look = XMVectorSet(0.f, -1.f, 0.f, 0.f);
		}
		const f32_t fRoll = XMConvertToRadians(Particle.fRotationDegrees);
		const f32_t fCos = std::cos(fRoll);
		const f32_t fSin = std::sin(fRoll);
		const vector_t RolledRight = Right * fCos + Up * fSin;
		const vector_t RolledUp = Up * fCos - Right * fSin;
		const f32_t fSizeX = P.vSizeStart.x + (P.vSizeEnd.x - P.vSizeStart.x) * fLife;
		const f32_t fSizeY = P.vSizeStart.y + (P.vSizeEnd.y - P.vSizeStart.y) * fLife;

		Engine::VTXEFFECT_PARTICLE Instance;
		matrix_t InstanceWorld;
		InstanceWorld.r[0] = XMVectorSetW(RolledRight * fSizeX, 0.f);
		InstanceWorld.r[1] = XMVectorSetW(RolledUp * fSizeY, 0.f);
		InstanceWorld.r[2] = XMVectorSetW(Look, 0.f);
		InstanceWorld.r[3] = XMVectorSetW(Position, 1.f);
		XMStoreFloat4x4(&Instance.World, InstanceWorld);
		XMStoreFloat4(&Instance.Color, XMVectorLerp(
			XMLoadFloat4(&P.vColorStart), XMLoadFloat4(&P.vColorEnd), fLife));
		const uint32_t iFrame = P.bSubUVOverLife ?
			(std::min)(iFrames - 1u, static_cast<uint32_t>(fLife * static_cast<f32_t>(iFrames))) : 0u;
		Instance.UVTransform = float4_t(
			1.f / static_cast<f32_t>(iColumns), 1.f / static_cast<f32_t>(iRows),
			static_cast<f32_t>(iFrame % iColumns) / static_cast<f32_t>(iColumns),
			static_cast<f32_t>(iFrame / iColumns) / static_cast<f32_t>(iRows));
		Instance.UVTransformNext = Instance.UVTransform;
		Instance.ParticleData = float2_t(fLife, 0.f);
		m_ParticleInstances.push_back(Instance);
	}
	return m_pParticleBuffer->Update_Instances(
		std::span<const Engine::VTXEFFECT_PARTICLE>(m_ParticleInstances));
}

void Client::CEffectV2Object::Update_Trail(const f32_t fStep)
{
	const TRAIL_PARAMS& T = m_Params.Trail;
	if (fStep > 0.f)
	{
		const f32_t fAgeStep = fStep / (std::max)(0.001f, T.fPointLifetime);
		for (TRAIL_POINT& Point : m_TrailPoints)
			Point.fAge += fAgeStep;
		m_TrailPoints.erase(std::remove_if(m_TrailPoints.begin(), m_TrailPoints.end(),
			[](const TRAIL_POINT& Point) { return Point.fAge >= 1.f; }),
			m_TrailPoints.end());
	}
	if (m_bEmissionStopped)
		return;
	m_fTrailSampleAccumulator += fStep;
	if (m_fTrailSampleAccumulator < T.fSampleInterval && !m_TrailPoints.empty())
		return;
	m_fTrailSampleAccumulator = 0.f;
	const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const vector_t Center = XMVectorSetW(World.r[3], 1.f);
	const vector_t Edge = TRAIL_EDGE_MODE::LOCAL_OFFSET == T.eEdgeMode ?
		XMVector3TransformCoord(XMLoadFloat3(&T.vEdgeOffset), World) : Center;
	f32_t fDistance = 0.f;
	if (!m_TrailPoints.empty())
	{
		fDistance = XMVectorGetX(XMVector3Length(
			Center - XMLoadFloat3(&m_TrailPoints.back().vCenter)));
		if (fDistance < T.fMinDistance)
			return;
	}
	m_fTrailCumulativeDistance += fDistance;
	TRAIL_POINT Point;
	Point.vCenter = To_Float3(Center);
	Point.vEdge = To_Float3(Edge);
	Point.fAge = 0.f;
	Point.fCumulativeDistance = m_fTrailCumulativeDistance;
	m_TrailPoints.push_back(Point);
	const uint32_t iMaxPoints = (std::min)(MAX_TRAIL_POINTS, (std::max)(2u, T.iMaxPoints));
	while (m_TrailPoints.size() > iMaxPoints)
		m_TrailPoints.erase(m_TrailPoints.begin());
}

HRESULT Client::CEffectV2Object::Build_TrailGeometry()
{
	const TRAIL_PARAMS& T = m_Params.Trail;
	const uint32_t iMaxPoints = (std::min)(MAX_TRAIL_POINTS, (std::max)(2u, T.iMaxPoints));
	if (m_TrailPoints.size() < 2u)
		return S_FALSE;
	const float4_t* pCameraPosition = CGameInstance::Get().Get_CamPosition();
	const vector_t CameraPosition = nullptr != pCameraPosition ?
		XMLoadFloat4(pCameraPosition) : XMVectorZero();
	const size_t iCount = m_TrailPoints.size();
	m_TrailVertices.clear();
	m_TrailIndices.clear();
	for (size_t iPoint = 0u; iPoint < iCount; ++iPoint)
	{
		const TRAIL_POINT& Point = m_TrailPoints[iPoint];
		const vector_t Center = XMLoadFloat3(&Point.vCenter);
		vector_t First = Center;
		vector_t Second = XMLoadFloat3(&Point.vEdge);
		if (TRAIL_EDGE_MODE::LOCAL_OFFSET != T.eEdgeMode)
		{
			const vector_t Previous = XMLoadFloat3(
				&m_TrailPoints[iPoint > 0u ? iPoint - 1u : iPoint].vCenter);
			const vector_t Next = XMLoadFloat3(
				&m_TrailPoints[iPoint + 1u < iCount ? iPoint + 1u : iPoint].vCenter);
			vector_t Tangent;
			if (!Normalize_Safe(Next - Previous, Tangent))
				continue;
			vector_t Side;
			if (TRAIL_EDGE_MODE::CENTERLINE_CAMERA == T.eEdgeMode)
			{
				if (!Normalize_Safe(XMVector3Cross(CameraPosition - Center, Tangent), Side))
					continue;
			}
			else if (!Normalize_Safe(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), Tangent), Side) &&
				!Normalize_Safe(XMVector3Cross(XMVectorSet(1.f, 0.f, 0.f, 0.f), Tangent), Side))
				continue;
			const f32_t fWidth = T.fStartWidth + (T.fEndWidth - T.fStartWidth) * Saturate(Point.fAge);
			First = Center - Side * (fWidth * 0.5f);
			Second = Center + Side * (fWidth * 0.5f);
		}
		const f32_t fU = T.fTilingDistance > 0.f ?
			Point.fCumulativeDistance / T.fTilingDistance : static_cast<f32_t>(iPoint);
		const f32_t fAlpha = T.bFadeWithAge ? 1.f - Saturate(Point.fAge) : 1.f;
		Engine::VTXEFFECT_TRAIL Vertex;
		Vertex.vColor = float4_t(1.f, 1.f, 1.f, fAlpha);
		Vertex.vPosition = To_Float3(First);
		Vertex.vTexcoord = float2_t(fU, 0.f);
		m_TrailVertices.push_back(Vertex);
		Vertex.vPosition = To_Float3(Second);
		Vertex.vTexcoord = float2_t(fU, 1.f);
		m_TrailVertices.push_back(Vertex);
	}
	if (m_TrailVertices.size() < 4u)
		return S_FALSE;
	for (uint32_t iBase = 0u; iBase + 3u < m_TrailVertices.size(); iBase += 2u)
	{
		m_TrailIndices.push_back(iBase);
		m_TrailIndices.push_back(iBase + 1u);
		m_TrailIndices.push_back(iBase + 2u);
		m_TrailIndices.push_back(iBase + 1u);
		m_TrailIndices.push_back(iBase + 3u);
		m_TrailIndices.push_back(iBase + 2u);
	}
	if (nullptr == m_pTrailBuffer || m_iTrailBufferPoints < iMaxPoints)
	{
		unique_ptr<Engine::CVIBuffer_DynamicTrail> Buffer =
			Engine::CVIBuffer_DynamicTrail::Create(m_pDevice, m_pContext, iMaxPoints);
		if (nullptr == Buffer)
			return E_FAIL;
		m_pTrailBuffer = std::move(Buffer);
		m_iTrailBufferPoints = iMaxPoints;
	}
	return m_pTrailBuffer->Update_Geometry(
		std::span<const Engine::VTXEFFECT_TRAIL>(m_TrailVertices),
		std::span<const uint32_t>(m_TrailIndices));
}

void Client::CEffectV2Object::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_bHidden || m_bFinished)
		return;
	if (SHAPE::SCREEN_POST == m_eShape)
	{
		if (FAILED(Engine::CPresentation_Manager::Get().Add_FrameProvider(
			static_pointer_cast<Engine::IPresentationProvider>(
				static_pointer_cast<CEffectV2Object>(shared_from_this())))))
		{
			m_strStatus = "Presentation provider budget exceeded.";
		}
		return;
	}
	if (nullptr == m_pShader)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

f32_t Client::CEffectV2Object::ScreenPost_Intensity() const
{
	const SCREEN_POST_PARAMS& S = m_Params.ScreenPost;
	if (!S.bIntensityLerp)
		return (std::max)(0.f, S.fIntensityStart);
	return (std::max)(0.f,
		S.fIntensityStart + (S.fIntensityEnd - S.fIntensityStart) * Life_Ratio());
}

HRESULT Client::CEffectV2Object::Submit_Presentation()
{
	m_ePresentationFailureScope = Engine::PRESENTATION_FAILURE_SCOPE::NONE;
	Engine::CPresentation_Manager& Presentation = Engine::CPresentation_Manager::Get();
	Presentation.Register_ProviderSubmissionExpectation(0u, 0u, 1u, 1u);
	const SCREEN_POST_PARAMS& S = m_Params.ScreenPost;
	Engine::PRESENTATION_SCREEN_POST_DESC Post;
	switch (S.eProfile)
	{
	case SCREEN_POST_PROFILE::RGB_NOISE:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED;
		break;
	case SCREEN_POST_PROFILE::FILM_NOISE:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED;
		break;
	default:
		Post.eProfile = Engine::PRESENTATION_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED;
		break;
	}
	Post.iSourceOrder = 0u;
	Post.iRandomSeed = (std::max)(1u, S.iRandomSeed);
	Post.fSampleTimeSeconds = (std::max)(0.f, m_fTime);
	Post.fIntensity = ScreenPost_Intensity();
	Post.fSecondaryIntensity = (std::max)(0.f, S.fSecondaryIntensity);
	Post.fFrequency = (std::max)(0.f, S.fFrequency);
	Post.vTint = S.vTint;
	const HRESULT hResult = Presentation.Add_ScreenPost(Post);
	if (FAILED(hResult))
	{
		m_ePresentationFailureScope = Presentation.Get_LastFailureScope();
		m_strStatus = "Screen post submission failed.";
	}
	return hResult;
}

HRESULT Client::CEffectV2Object::Bind_Common(
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
	const f32_t fLifeRatio = Life_Ratio();
	float4_t vColorMul = P.vColorMul;
	float4_t vColorOffset = P.vColorOffset;
	if (P.bColorMulLerp)
		XMStoreFloat4(&vColorMul, XMVectorLerp(
			XMLoadFloat4(&P.vColorMul), XMLoadFloat4(&P.vColorMulEnd), fLifeRatio));
	if (P.bColorOffsetLerp)
		XMStoreFloat4(&vColorOffset, XMVectorLerp(
			XMLoadFloat4(&P.vColorOffset), XMLoadFloat4(&P.vColorOffsetEnd), fLifeRatio));
	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4_t vCameraPosition =
		nullptr != pCameraPosition ? *pCameraPosition : float4_t(0.f, 0.f, 0.f, 1.f);
	if (FAILED(pShader->Bind_RawValue("g_vCamPosition", &vCameraPosition, sizeof(vCameraPosition))) ||
		FAILED(pShader->Bind_RawValue("g_RimColor", &P.vRimColor, sizeof(P.vRimColor))) ||
		FAILED(pShader->Bind_RawValue("g_RimPower", &P.fRimPower, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_RimIntensity", &P.fRimIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_GhostAlpha", &P.fGhostAlpha, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_Time", &m_fTime, sizeof(m_fTime))) ||
		FAILED(pShader->Bind_RawValue("g_ColorMul", &vColorMul, sizeof(vColorMul))) ||
		FAILED(pShader->Bind_RawValue("g_ColorOffset", &vColorOffset, sizeof(vColorOffset))) ||
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
		FAILED(pShader->Bind_RawValue("g_DissolveSoftness", &P.fDissolveSoftness, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_SoftFadeDistance", &P.fSoftFadeDistance, sizeof(f32_t))))
	{
		return E_FAIL;
	}
	if (0.f < P.fSoftFadeDistance &&
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Depth"), pShader, "g_DepthTexture")))
		return E_FAIL;
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

HRESULT Client::CEffectV2Object::Render_Decal(const uint32_t iPass)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const float4x4_t* pViewInverse = GameInstance.Get_InverseTransform(D3DTS::VIEW);
	const float4x4_t* pProjInverse = GameInstance.Get_InverseTransform(D3DTS::PROJ);
	if (nullptr == pViewInverse || nullptr == pProjInverse)
		return E_FAIL;
	const matrix_t World = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	vector_t Determinant = XMMatrixDeterminant(World);
	if (std::fabs(XMVectorGetX(Determinant)) <= 1e-12f)
		return E_FAIL;
	float4x4_t WorldInverse;
	XMStoreFloat4x4(&WorldInverse, XMMatrixInverse(&Determinant, World));
	const DECAL_PARAMS& D = m_Params.Decal;
	float3_t vDecalUp;
	XMStoreFloat3(&vDecalUp, XMVector3Normalize(World.r[1]));
	if (FAILED(m_pShader->Bind_Matrix("g_DecalWorldInverse", &WorldInverse)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", pViewInverse)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", pProjInverse)) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalSize", &D.vSize, sizeof(D.vSize))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalDepth", &D.fDepth, sizeof(f32_t))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalEdgeFade", &D.fEdgeFade, sizeof(f32_t))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalUp", &vDecalUp, sizeof(vDecalUp))) ||
		FAILED(m_pShader->Bind_RawValue("g_DecalNormalCutoff", &D.fNormalCutoff, sizeof(f32_t))) ||
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(GameInstance.Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;
	const uint32_t iDecalPass = BLEND_MODE::ADDITIVE == m_Params.eBlend ? 1u :
		BLEND_MODE::MULTIPLY == m_Params.eBlend ? 2u : 0u;
	UNREFERENCED_PARAMETER(iPass);
	if (FAILED(m_pShader->Begin(iDecalPass)) ||
		FAILED(m_pRect->Bind_Resources()) ||
		FAILED(m_pRect->Render()))
		return E_FAIL;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Render()
{
	if (SHAPE::SCREEN_POST == m_eShape || nullptr == m_pShader)
		return S_OK;
	if (FAILED(Bind_Common(m_pShader)))
	{
		m_strStatus = "Shader bind failed.";
		return E_FAIL;
	}
	const uint32_t iPass = BLEND_MODE::SOLID == m_Params.eBlend ? 4u :
		BLEND_MODE::MULTIPLY == m_Params.eBlend ? (m_Params.bDepthTest ? 5u : 6u) :
		static_cast<uint32_t>(m_Params.eBlend) + (m_Params.bDepthTest ? 0u : 2u);
	switch (m_eShape)
	{
	case SHAPE::MESH:
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
		if (m_Params.fOutlineWidth > 0.f && m_Params.vOutlineColor.w > 0.f)
		{
			if (FAILED(m_pShader->Bind_RawValue("g_OutlineWidth",
					&m_Params.fOutlineWidth, sizeof(f32_t))) ||
				FAILED(m_pShader->Bind_RawValue("g_OutlineColor",
					&m_Params.vOutlineColor, sizeof(m_Params.vOutlineColor))))
			{
				m_strStatus = "Outline bind failed.";
				return E_FAIL;
			}
			for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
			{
				if (iMesh < m_Parts.size() && !m_Parts[iMesh].bVisible)
					continue;
				if (m_bSkinned && FAILED(m_pModel->Bind_BoneMatrices(
					m_pShader, "g_BoneMatrices", iMesh)))
				{
					m_strStatus = "Bone matrix bind failed.";
					return E_FAIL;
				}
				if (FAILED(m_pShader->Begin(7u)) || FAILED(m_pModel->Render(iMesh)))
				{
					m_strStatus = "Outline draw failed.";
					return E_FAIL;
				}
			}
		}
		return S_OK;
	case SHAPE::SPRITE:
		if (FAILED(m_pShader->Begin(iPass)) ||
			FAILED(m_pRect->Bind_Resources()) ||
			FAILED(m_pRect->Render()))
		{
			m_strStatus = "Sprite draw failed.";
			return E_FAIL;
		}
		return S_OK;
	case SHAPE::PARTICLE:
	{
		if (FAILED(Build_ParticleInstances()))
		{
			m_strStatus = "Particle instance upload failed.";
			return E_FAIL;
		}
		if (0u == m_pParticleBuffer->Get_InstanceCount())
			return S_OK;
		if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pParticleBuffer->Render()))
		{
			m_strStatus = "Particle draw failed.";
			return E_FAIL;
		}
		return S_OK;
	}
	case SHAPE::TRAIL:
	{
		const HRESULT hGeometry = Build_TrailGeometry();
		if (FAILED(hGeometry))
		{
			m_strStatus = "Trail geometry upload failed.";
			return E_FAIL;
		}
		if (S_FALSE == hGeometry)
			return S_OK;
		if (FAILED(m_pShader->Begin(iPass)) ||
			FAILED(m_pTrailBuffer->Bind_Resources()) ||
			FAILED(m_pTrailBuffer->Render()))
		{
			m_strStatus = "Trail draw failed.";
			return E_FAIL;
		}
		return S_OK;
	}
	case SHAPE::DECAL:
		if (FAILED(Render_Decal(iPass)))
		{
			m_strStatus = "Decal draw failed.";
			return E_FAIL;
		}
		return S_OK;
	case SHAPE::SCREEN_POST:
		return S_OK;
	default:
		return E_FAIL;
	}
}

unique_ptr<Client::CEffectV2Object> Client::CEffectV2Object::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectV2Object> Instance(new CEffectV2Object(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectV2Object::Clone(void* pArg)
{
	shared_ptr<CEffectV2Object> Instance(new CEffectV2Object(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
