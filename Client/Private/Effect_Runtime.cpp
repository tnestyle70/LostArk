#include <cmath>
#include <cfloat>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <mutex>
#include <unordered_map>

#include "Effect_Runtime.h"
#include "BinaryAsset/ModelAssetData.h"
#include "GameInstance.h"
#include "Model.h"
#include "Profiler.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

namespace
{
	using namespace Client;
	using namespace Engine;

	struct EFFECT_PREVIEW_BRIDGE
	{
		mutex Mutex;
		EFFECT_ASSET_DESC AssetDesc;
		float3_t vWorldPosition = {};
		uint64_t iGeneration = {};
		bool_t isEnabled = { false };
	};

	EFFECT_PREVIEW_BRIDGE& Get_PreviewBridge()
	{
		static EFFECT_PREVIEW_BRIDGE Bridge;
		return Bridge;
	}

	float3_t Add(const float3_t& Left, const float3_t& Right)
	{
		return {
			Left.x + Right.x,
			Left.y + Right.y,
			Left.z + Right.z
		};
	}

	float3_t Subtract(const float3_t& Left, const float3_t& Right)
	{
		return {
			Left.x - Right.x,
			Left.y - Right.y,
			Left.z - Right.z
		};
	}

	float3_t Scale(const float3_t& Value, const f32_t fScale)
	{
		return {
			Value.x * fScale,
			Value.y * fScale,
			Value.z * fScale
		};
	}

	f32_t LengthSq(const float3_t& Value)
	{
		return
			Value.x * Value.x +
			Value.y * Value.y +
			Value.z * Value.z;
	}

	float3_t Normalize(const float3_t& Value,
		const float3_t& vFallback = { 1.f, 0.f, 0.f })
	{
		const f32_t fLengthSq = LengthSq(Value);
		if (fLengthSq <= 0.000001f)
			return vFallback;

		return Scale(Value, 1.f / sqrtf(fLengthSq));
	}

	float3_t Cross(const float3_t& Left, const float3_t& Right)
	{
		return {
			Left.y * Right.z - Left.z * Right.y,
			Left.z * Right.x - Left.x * Right.z,
			Left.x * Right.y - Left.y * Right.x
		};
	}

	float3_t Lerp(const float3_t& Left,
		const float3_t& Right,
		const f32_t fRatio)
	{
		return Add(Left, Scale(Subtract(Right, Left), fRatio));
	}

	float4_t Lerp(const float4_t& Left,
		const float4_t& Right,
		const f32_t fRatio)
	{
		return {
			Left.x + (Right.x - Left.x) * fRatio,
			Left.y + (Right.y - Left.y) * fRatio,
			Left.z + (Right.z - Left.z) * fRatio,
			Left.w + (Right.w - Left.w) * fRatio
		};
	}

	bool_t Is_TextureExtension(const filesystem::path& Path)
	{
		wstring_t strExtension = Path.extension().wstring();
		transform(strExtension.begin(), strExtension.end(),
			strExtension.begin(), towlower);
		return
			L".dds" == strExtension ||
			L".png" == strExtension ||
			L".jpg" == strExtension ||
			L".jpeg" == strExtension ||
			L".bmp" == strExtension ||
			L".tif" == strExtension ||
			L".tiff" == strExtension;
	}

	bool_t Is_DDS(const filesystem::path& Path)
	{
		wstring_t strExtension = Path.extension().wstring();
		transform(strExtension.begin(), strExtension.end(),
			strExtension.begin(), towlower);
		return L".dds" == strExtension;
	}

	bool_t Has_Extension(
		const filesystem::path& Path,
		const wchar_t* pExpectedExtension)
	{
		wstring_t strExtension = Path.extension().wstring();
		transform(strExtension.begin(), strExtension.end(),
			strExtension.begin(), towlower);
		return pExpectedExtension == strExtension;
	}
}

CEffect_Runtime::CEffect_Runtime(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CEffect_Runtime::~CEffect_Runtime()
{
}

void CEffect_Runtime::Publish_Preview(
	const EFFECT_ASSET_DESC& AssetDesc,
	const bool_t isEnabled,
	const float3_t& vWorldPosition)
{
	EFFECT_PREVIEW_BRIDGE& Bridge = Get_PreviewBridge();
	lock_guard Lock(Bridge.Mutex);
	Bridge.AssetDesc = AssetDesc;
	Bridge.vWorldPosition = vWorldPosition;
	Bridge.isEnabled = isEnabled;
	++Bridge.iGeneration;
	if (0 == Bridge.iGeneration)
		++Bridge.iGeneration;
}

uint64_t CEffect_Runtime::Get_PublishedGeneration()
{
	EFFECT_PREVIEW_BRIDGE& Bridge = Get_PreviewBridge();
	lock_guard Lock(Bridge.Mutex);
	return Bridge.iGeneration;
}

HRESULT CEffect_Runtime::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffect_Runtime::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderResources()))
		return E_FAIL;

	Consume_PublishedPreview();
	return S_OK;
}

void CEffect_Runtime::Priority_Update(f32_t fTimeDelta)
{
}

void CEffect_Runtime::Update(const f32_t fTimeDelta)
{
	Consume_PublishedPreview();
	if (!m_isPreviewEnabled || fTimeDelta <= 0.f)
		return;

	const float4_t* pCameraPosition =
		CGameInstance::Get().Get_CamPosition();
	const float3_t vOrigin = Get_WorldOrigin();
	const f32_t fCameraDistance =
		(nullptr == pCameraPosition)
		? 0.f
		: sqrtf(
			(pCameraPosition->x - vOrigin.x) *
				(pCameraPosition->x - vOrigin.x) +
			(pCameraPosition->y - vOrigin.y) *
				(pCameraPosition->y - vOrigin.y) +
			(pCameraPosition->z - vOrigin.z) *
				(pCameraPosition->z - vOrigin.z));

	for (EFFECT_EMITTER_RUNTIME& Runtime : m_EmitterRuntimes)
	{
		if (!Runtime.Desc.isEnabled || !Runtime.isSimulationReady)
			continue;

		Runtime.Simulator.Set_LODDistance(fCameraDistance);
		Runtime.Simulator.Update(fTimeDelta);
		if (nullptr != Runtime.pModel &&
			Runtime.pModel->Is_Skinned() &&
			Runtime.pModel->Has_Animations())
		{
			Runtime.pModel->Update_Animation(fTimeDelta);
		}
	}
}

void CEffect_Runtime::Late_Update(f32_t fTimeDelta)
{
	if (!m_isPreviewEnabled)
		return;

	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CEffect_Runtime::Render()
{
	if (!m_isPreviewEnabled)
		return S_OK;

	for (const EFFECT_EMITTER_RUNTIME& Runtime : m_EmitterRuntimes)
	{
		if (!Runtime.Desc.isEnabled || !Runtime.isSimulationReady)
			continue;

		HRESULT hResult = E_FAIL;
		switch (Runtime.Desc.eType)
		{
		case EFFECT_EMITTER_TYPE::SPRITE:
			hResult = Render_SpriteEmitter(Runtime);
			break;

		case EFFECT_EMITTER_TYPE::MESH:
			hResult = Render_MeshEmitter(Runtime);
			break;

		case EFFECT_EMITTER_TYPE::BEAM:
		case EFFECT_EMITTER_TYPE::RIBBON:
		case EFFECT_EMITTER_TYPE::ANIM_TRAIL:
			hResult = Render_PrimitiveEmitter(Runtime);
			break;

		default:
			return E_FAIL;
		}

		if (FAILED(hResult))
			return hResult;
	}
	return S_OK;
}

HRESULT CEffect_Runtime::Ready_RenderResources()
{
	auto pSpriteShader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_EffectSprite.hlsl"),
		VTXTEX::Elements,
		VTXTEX::iNumElements);
	if (nullptr == pSpriteShader)
		return E_FAIL;
	m_pSpriteShader = shared_ptr<CShader>(move(pSpriteShader));

	auto pPrimitiveShader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_EffectPrimitive.hlsl"),
		VTXTEX::Elements,
		VTXTEX::iNumElements);
	if (nullptr == pPrimitiveShader)
		return E_FAIL;
	m_pPrimitiveShader = shared_ptr<CShader>(move(pPrimitiveShader));

	/*
	 * The model vertex stride is still selected by CMesh.  This compact input
	 * layout only describes the two attributes consumed by the unlit effect
	 * shader while preserving VTXMESH's TEXCOORD byte offset.
	 */
	const D3D11_INPUT_ELEMENT_DESC MeshElements[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	auto pMeshShader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_EffectPrimitive.hlsl"),
		MeshElements,
		2u);
	if (nullptr == pMeshShader)
		return E_FAIL;
	m_pMeshShader = shared_ptr<CShader>(move(pMeshShader));

	auto pAnimMeshShader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_EffectAnimMesh.hlsl"),
		VTXANIMMESH::Elements,
		VTXANIMMESH::iNumElements);
	if (nullptr == pAnimMeshShader)
		return E_FAIL;
	m_pAnimMeshShader = shared_ptr<CShader>(move(pAnimMeshShader));

	auto pRectBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == pRectBuffer)
		return E_FAIL;
	m_pRectBuffer = shared_ptr<CVIBuffer_Rect>(move(pRectBuffer));

	return Create_FallbackTexture();
}

HRESULT CEffect_Runtime::Create_FallbackTexture()
{
	const uint32_t iWhitePixel = 0xffffffffu;

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = 1;
	TextureDesc.Height = 1;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA InitialData{};
	InitialData.pSysMem = &iWhitePixel;
	InitialData.SysMemPitch = sizeof(iWhitePixel);

	ComPtr<ID3D11Texture2D> pTexture;
	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, &InitialData, &pTexture)))
	{
		return E_FAIL;
	}

	return m_pDevice->CreateShaderResourceView(
		pTexture.Get(), nullptr, &m_pFallbackTextureSRV);
}

bool_t CEffect_Runtime::Consume_PublishedPreview()
{
	EFFECT_ASSET_DESC AssetDesc;
	float3_t vWorldPosition{};
	uint64_t iGeneration{};
	bool_t isEnabled{};

	{
		EFFECT_PREVIEW_BRIDGE& Bridge = Get_PreviewBridge();
		lock_guard Lock(Bridge.Mutex);
		if (Bridge.iGeneration == m_iAppliedGeneration)
			return false;

		AssetDesc = Bridge.AssetDesc;
		vWorldPosition = Bridge.vWorldPosition;
		iGeneration = Bridge.iGeneration;
		isEnabled = Bridge.isEnabled;
	}

	m_iAppliedGeneration = iGeneration;
	m_isPreviewEnabled = isEnabled;
	m_vWorldPosition = vWorldPosition;
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(
			vWorldPosition.x,
			vWorldPosition.y,
			vWorldPosition.z,
			1.f));

	if (!isEnabled)
	{
		m_AssetDesc = {};
		m_EmitterRuntimes.clear();
		return true;
	}

	Rebuild(AssetDesc, vWorldPosition);
	return true;
}

bool_t CEffect_Runtime::Rebuild(
	const EFFECT_ASSET_DESC& AssetDesc,
	const float3_t& vWorldPosition)
{
	m_AssetDesc = AssetDesc;
	m_vWorldPosition = vWorldPosition;
	m_EmitterRuntimes.clear();
	m_TextureCache.clear();
	m_ModelCache.clear();
	m_EmitterRuntimes.reserve(AssetDesc.Emitters.size());

	uint32_t iEmitterSeed = 0x9e3779b9u;
	uint32_t iRemainingParticleCapacity = MAX_PARTICLES_PER_EFFECT;
	for (const EFFECT_EMITTER_DESC& EmitterDesc : AssetDesc.Emitters)
	{
		EFFECT_EMITTER_RUNTIME Runtime;
		Runtime.Desc = EmitterDesc;

		for (EFFECT_MODULE_DESC& Module : Runtime.Desc.Modules)
		{
			if (!Module.isEnabled ||
				EFFECT_MODULE_TYPE::SPAWN != Module.eType)
			{
				continue;
			}

			const uint32_t iEmitterCapacity = min(
				Module.Spawn.iMaxParticles,
				MAX_PARTICLES_PER_EMITTER);
			Module.Spawn.iMaxParticles = min(
				iEmitterCapacity,
				iRemainingParticleCapacity);
			Module.Spawn.iBurstCount = min(
				Module.Spawn.iBurstCount,
				Module.Spawn.iMaxParticles);
			iRemainingParticleCapacity -=
				Module.Spawn.iMaxParticles;
			break;
		}

		Runtime.isSimulationReady = Runtime.Simulator.Initialize(
			Runtime.Desc,
			iEmitterSeed ^ static_cast<uint32_t>(EmitterDesc.iEmitterId));
		iEmitterSeed += 0x9e3779b9u;

		const EFFECT_MODULE_DESC* pRequired = Find_Module(
			Runtime.Desc, EFFECT_MODULE_TYPE::REQUIRED);
		if (nullptr != pRequired)
		{
			Runtime.pTextureSRV = Load_Texture(
				pRequired->Required.strTextureAssetId);
			Runtime.pOpacityTextureSRV = Load_Texture(
				pRequired->Required.Material.strOpacityTextureAssetId);
			Runtime.pDissolveTextureSRV = Load_Texture(
				pRequired->Required.Material.strDissolveTextureAssetId);
			Runtime.pDistortionTextureSRV =
				pRequired->Required.Material.strDistortionTextureAssetId.empty()
				? Runtime.pTextureSRV
				: Load_Texture(
					pRequired->Required.Material.strDistortionTextureAssetId);

			if (EFFECT_EMITTER_TYPE::MESH == Runtime.Desc.eType)
			{
				const filesystem::path MeshPath =
					Resolve_AssetPath(
						pRequired->Required.strMeshAssetId);
				if (Has_Extension(MeshPath, L".wmesh"))
				{
					Runtime.pModel = Load_BinaryModel(
						pRequired->Required);

					if (nullptr != Runtime.pModel &&
						Runtime.pModel->Is_Skinned() &&
						Runtime.pModel->Has_Animations())
					{
						const EFFECT_MODULE_DESC* pMeshAnimation =
							Find_Module(
								Runtime.Desc,
								EFFECT_MODULE_TYPE::MESH_ANIMATION);
						if (nullptr != pMeshAnimation)
						{
							const uint32_t iAnimationIndex = min(
								pMeshAnimation->
									MeshAnimation.iAnimationIndex,
								Runtime.pModel->
									Get_NumAnimations() - 1u);
							const f32_t fPlayRate = isfinite(
								pMeshAnimation->
									MeshAnimation.fPlayRate)
								? clamp(
									pMeshAnimation->
										MeshAnimation.fPlayRate,
									-16.f,
									16.f)
								: 1.f;
							Runtime.pModel->
								Set_AnimationSpeed(fPlayRate);
							Runtime.pModel->Start_Animation(
								iAnimationIndex,
								pMeshAnimation->
									MeshAnimation.isLoop);
						}
					}
				}
				else
				{
					Runtime.pModel = Load_StaticModel(
						pRequired->Required.strMeshAssetId);
				}
			}
		}

		if (nullptr == Runtime.pTextureSRV.Get())
			Runtime.pTextureSRV = m_pFallbackTextureSRV;
		if (nullptr == Runtime.pOpacityTextureSRV.Get())
			Runtime.pOpacityTextureSRV = m_pFallbackTextureSRV;
		if (nullptr == Runtime.pDissolveTextureSRV.Get())
			Runtime.pDissolveTextureSRV = m_pFallbackTextureSRV;
		if (nullptr == Runtime.pDistortionTextureSRV.Get())
			Runtime.pDistortionTextureSRV = Runtime.pTextureSRV;

		m_EmitterRuntimes.push_back(move(Runtime));
	}

	const f32_t fWarmupTime =
		clamp(AssetDesc.fWarmupTime, 0.f, 10.f);
	if (fWarmupTime > 0.f)
	{
		constexpr f32_t FIXED_STEP = 1.f / 60.f;
		f32_t fRemainingTime = fWarmupTime;
		while (fRemainingTime > 0.f)
		{
			const f32_t fStep = min(FIXED_STEP, fRemainingTime);
			for (EFFECT_EMITTER_RUNTIME& Runtime : m_EmitterRuntimes)
			{
				if (Runtime.isSimulationReady)
				{
					Runtime.Simulator.Update(fStep);
					if (Runtime.Desc.isEnabled &&
						nullptr != Runtime.pModel &&
						Runtime.pModel->Is_Skinned() &&
						Runtime.pModel->Has_Animations())
					{
						Runtime.pModel->
							Update_Animation(fStep);
					}
				}
			}
			fRemainingTime -= fStep;
		}
	}
	return true;
}

const EFFECT_MODULE_DESC* CEffect_Runtime::Find_Module(
	const EFFECT_EMITTER_DESC& EmitterDesc,
	const EFFECT_MODULE_TYPE eType) const
{
	const auto Iter = find_if(
		EmitterDesc.Modules.begin(),
		EmitterDesc.Modules.end(),
		[eType](const EFFECT_MODULE_DESC& Module)
		{
			return Module.isEnabled && eType == Module.eType;
		});
	return
		(EmitterDesc.Modules.end() == Iter)
		? nullptr
		: &(*Iter);
}

filesystem::path CEffect_Runtime::Resolve_AssetPath(
	const string& strAssetId) const
{
	if (strAssetId.empty())
		return {};

	const u8string Utf8Path(
		strAssetId.begin(), strAssetId.end());
	filesystem::path RequestedPath =
		filesystem::path(Utf8Path).lexically_normal();
	if (RequestedPath.is_absolute() || RequestedPath.has_root_path())
		return {};

	const filesystem::path Candidates[] = {
		CRuntimeAssetRoot::Resolve(RequestedPath),
		CRuntimeAssetRoot::Resolve(
			filesystem::path(L"Effect") / RequestedPath)
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

ComPtr<ID3D11ShaderResourceView> CEffect_Runtime::Load_Texture(
	const string& strTextureAssetId)
{
	const filesystem::path TexturePath =
		Resolve_AssetPath(strTextureAssetId);
	if (TexturePath.empty() || !Is_TextureExtension(TexturePath))
		return m_pFallbackTextureSRV;

	const wstring_t strCacheKey =
		TexturePath.lexically_normal().wstring();
	const auto CacheIter = m_TextureCache.find(strCacheKey);
	if (m_TextureCache.end() != CacheIter)
		return CacheIter->second;

	ComPtr<ID3D11ShaderResourceView> pSRV;
	HRESULT hr = E_FAIL;
	if (Is_DDS(TexturePath))
	{
		hr = CreateDDSTextureFromFile(
			m_pDevice.Get(),
			TexturePath.c_str(),
			nullptr,
			&pSRV);
	}
	else
	{
		hr = CreateWICTextureFromFile(
			m_pDevice.Get(),
			TexturePath.c_str(),
			nullptr,
			&pSRV);
	}

	if (FAILED(hr) || nullptr == pSRV.Get())
		pSRV = m_pFallbackTextureSRV;

	if (m_TextureCache.size() >= MAX_TEXTURE_CACHE_ENTRIES)
		m_TextureCache.erase(m_TextureCache.begin());
	m_TextureCache.emplace(strCacheKey, pSRV);
	return pSRV;
}

shared_ptr<CModel> CEffect_Runtime::Load_StaticModel(
	const string& strMeshAssetId)
{
	const filesystem::path ModelPath =
		Resolve_AssetPath(strMeshAssetId);
	if (ModelPath.empty())
		return nullptr;

	const wstring_t strCacheKey =
		ModelPath.lexically_normal().wstring();
	const auto CacheIter = m_ModelCache.find(strCacheKey);
	if (m_ModelCache.end() != CacheIter)
		return CacheIter->second;

	/*
	 * The extracted effect set contains glTF/bin pairs.  Loading one selected
	 * file through Assimp is intentionally lazy; the 80+ GiB tree is never
	 * scanned or preloaded by this object.
	 */
	const string strModelPath = ModelPath.string();
	auto pModel = CModel::Create(
		m_pDevice,
		m_pContext,
		MODEL::NONANIM,
		strModelPath.c_str(),
		XMMatrixIdentity());
	if (nullptr == pModel)
		return nullptr;

	shared_ptr<CModel> pSharedModel(move(pModel));
	if (m_ModelCache.size() >= MAX_MODEL_CACHE_ENTRIES)
		m_ModelCache.erase(m_ModelCache.begin());
	m_ModelCache.emplace(strCacheKey, pSharedModel);
	return pSharedModel;
}

shared_ptr<CModel> CEffect_Runtime::Load_BinaryModel(
	const EFFECT_REQUIRED_MODULE_DESC& RequiredDesc)
{
	const filesystem::path MeshPath =
		Resolve_AssetPath(RequiredDesc.strMeshAssetId);
	error_code Error;
	if (MeshPath.empty() ||
		!filesystem::is_regular_file(MeshPath, Error) ||
		!Has_Extension(MeshPath, L".wmesh"))
	{
		return nullptr;
	}

	MODEL_ASSET_LOAD_DESC LoadDesc{};
	LoadDesc.assetRoot = CRuntimeAssetRoot::Get();
	LoadDesc.meshPath = MeshPath;

	if (!RequiredDesc.strMaterialAssetId.empty())
	{
		const filesystem::path MaterialPath =
			Resolve_AssetPath(RequiredDesc.strMaterialAssetId);
		Error.clear();
		if (!MaterialPath.empty() &&
			filesystem::is_regular_file(MaterialPath, Error))
		{
			LoadDesc.materialPath = MaterialPath;
		}
	}

	if (LoadDesc.materialPath.empty())
	{
		filesystem::path MaterialPath = MeshPath;
		MaterialPath.replace_extension(L".wmat");
		Error.clear();
		if (filesystem::is_regular_file(MaterialPath, Error))
			LoadDesc.materialPath = MaterialPath;
	}

	filesystem::path SkeletonPath = MeshPath;
	SkeletonPath.replace_extension(L".wskel");
	Error.clear();
	if (filesystem::is_regular_file(SkeletonPath, Error))
	{
		LoadDesc.skeletonPath = SkeletonPath;

		const filesystem::path AnimationDirectory =
			MeshPath.parent_path() / L"anims";
		Error.clear();
		filesystem::directory_iterator Iterator(
			AnimationDirectory,
			filesystem::directory_options::skip_permission_denied,
			Error);
		const filesystem::directory_iterator End;
		while (!Error && Iterator != End)
		{
			error_code EntryError;
			if (Iterator->is_regular_file(EntryError) &&
				!EntryError &&
				Has_Extension(Iterator->path(), L".wanim"))
			{
				LoadDesc.animationPaths.push_back(
					Iterator->path().lexically_normal());
			}
			Iterator.increment(Error);
		}
		sort(
			LoadDesc.animationPaths.begin(),
			LoadDesc.animationPaths.end());
	}

	const filesystem::path FallbackDiffusePath =
		Resolve_AssetPath(RequiredDesc.strTextureAssetId);
	Error.clear();
	if (!FallbackDiffusePath.empty() &&
		Is_TextureExtension(FallbackDiffusePath) &&
		filesystem::is_regular_file(FallbackDiffusePath, Error))
	{
		LoadDesc.fallbackDiffusePath = FallbackDiffusePath;
	}

	const MODEL modelType = LoadDesc.skeletonPath.empty()
		? MODEL::NONANIM : MODEL::ANIM;
	auto pModel = CModel::Create(
		m_pDevice,
		m_pContext,
		modelType,
		LoadDesc,
		XMMatrixIdentity());
	return nullptr == pModel
		? nullptr : shared_ptr<CModel>(move(pModel));
}

HRESULT CEffect_Runtime::Render_SpriteEmitter(
	const EFFECT_EMITTER_RUNTIME& Runtime)
{
	if (nullptr == m_pSpriteShader || nullptr == m_pRectBuffer)
		return E_FAIL;

	if (FAILED(Bind_CommonSpriteResources(Runtime)) ||
		FAILED(m_pRectBuffer->Bind_Resources()))
	{
		return E_FAIL;
	}

	const vector<EFFECT_PARTICLE>& Particles =
		Runtime.Simulator.Get_Particles();
	vector<const EFFECT_PARTICLE*> SortedParticles;
	SortedParticles.reserve(Particles.size());
	for (const EFFECT_PARTICLE& Particle : Particles)
		SortedParticles.push_back(&Particle);

	const float4_t* pCameraPosition =
		CGameInstance::Get().Get_CamPosition();
	switch (Runtime.Desc.eSortMode)
	{
	case EFFECT_SORT_MODE::DISTANCE_TO_VIEW:
		if (nullptr != pCameraPosition)
		{
			sort(SortedParticles.begin(), SortedParticles.end(),
				[this, pCameraPosition](
					const EFFECT_PARTICLE* pLeft,
					const EFFECT_PARTICLE* pRight)
				{
					const float3_t Left = To_WorldPosition(*pLeft);
					const float3_t Right = To_WorldPosition(*pRight);
					const float3_t LeftDelta = {
						Left.x - pCameraPosition->x,
						Left.y - pCameraPosition->y,
						Left.z - pCameraPosition->z
					};
					const float3_t RightDelta = {
						Right.x - pCameraPosition->x,
						Right.y - pCameraPosition->y,
						Right.z - pCameraPosition->z
					};
					return LengthSq(LeftDelta) > LengthSq(RightDelta);
				});
		}
		break;

	case EFFECT_SORT_MODE::AGE_OLDEST_FIRST:
		sort(SortedParticles.begin(), SortedParticles.end(),
			[](const EFFECT_PARTICLE* pLeft,
				const EFFECT_PARTICLE* pRight)
			{
				return pLeft->fAge > pRight->fAge;
			});
		break;

	case EFFECT_SORT_MODE::AGE_NEWEST_FIRST:
		sort(SortedParticles.begin(), SortedParticles.end(),
			[](const EFFECT_PARTICLE* pLeft,
				const EFFECT_PARTICLE* pRight)
			{
				return pLeft->fAge < pRight->fAge;
			});
		break;
	}

	const uint32_t iPass =
		(EFFECT_BLEND_MODE::ADDITIVE == Runtime.Desc.eBlendMode)
		? 1u
		: 0u;

	/*
	 * Cascade ScreenAlignment. The billboard basis already lives in the vertex
	 * shader, so Square only has to equalise the two size axes and Velocity
	 * only has to fold the on-screen motion angle into the existing rotation
	 * constant. No shader change is needed for either.
	 */
	const EFFECT_MODULE_DESC* pAlignSource = Find_Module(
		Runtime.Desc, EFFECT_MODULE_TYPE::REQUIRED);
	const EFFECT_SCREEN_ALIGNMENT eAlignment =
		(nullptr != pAlignSource)
		? pAlignSource->Required.eScreenAlignment
		: EFFECT_SCREEN_ALIGNMENT::RECTANGLE;

	float3_t vCameraRight = { 1.f, 0.f, 0.f };
	float3_t vCameraUp = { 0.f, 1.f, 0.f };
	if (EFFECT_SCREEN_ALIGNMENT::VELOCITY == eAlignment)
	{
		const float4x4_t* pInverseView =
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
		if (nullptr != pInverseView)
		{
			vCameraRight = {
				pInverseView->_11, pInverseView->_12, pInverseView->_13
			};
			vCameraUp = {
				pInverseView->_21, pInverseView->_22, pInverseView->_23
			};
		}
	}

	for (const EFFECT_PARTICLE* pParticle : SortedParticles)
	{
		const float3_t vPosition = To_WorldPosition(*pParticle);
		const float4_t vCenter = {
			vPosition.x, vPosition.y, vPosition.z, 1.f
		};
		const f32_t fSizeX = max(0.0001f, fabsf(pParticle->vSize.x));
		const float4_t vSize = {
			fSizeX,
			(EFFECT_SCREEN_ALIGNMENT::SQUARE == eAlignment)
				? fSizeX
				: max(0.0001f, fabsf(pParticle->vSize.y)),
			0.f,
			0.f
		};
		const float4_t vUVRect =
			Calculate_UVRect(Runtime.Desc, *pParticle);

		f32_t fRotationRadians =
			XMConvertToRadians(pParticle->vRotation.z);
		if (EFFECT_SCREEN_ALIGNMENT::VELOCITY == eAlignment)
		{
			// The shader maps the sprite local +Y axis to (-sin, cos) in the
			// camera plane, so this angle points +Y along the velocity.
			const f32_t fRight =
				pParticle->vVelocity.x * vCameraRight.x +
				pParticle->vVelocity.y * vCameraRight.y +
				pParticle->vVelocity.z * vCameraRight.z;
			const f32_t fUp =
				pParticle->vVelocity.x * vCameraUp.x +
				pParticle->vVelocity.y * vCameraUp.y +
				pParticle->vVelocity.z * vCameraUp.z;
			if (0.000001f < fRight * fRight + fUp * fUp)
				fRotationRadians += atan2f(-fRight, fUp);
		}

		if (FAILED(m_pSpriteShader->Bind_RawValue(
				"g_vParticleCenter", &vCenter, sizeof(vCenter))) ||
			FAILED(m_pSpriteShader->Bind_RawValue(
				"g_vParticleSize", &vSize, sizeof(vSize))) ||
			FAILED(m_pSpriteShader->Bind_RawValue(
				"g_vUVRect", &vUVRect, sizeof(vUVRect))) ||
			FAILED(m_pSpriteShader->Bind_RawValue(
				"g_vTint", &pParticle->vColor,
				sizeof(pParticle->vColor))) ||
			FAILED(m_pSpriteShader->Bind_RawValue(
				"g_fRotation", &fRotationRadians,
				sizeof(fRotationRadians))) ||
			FAILED(Bind_DynamicParameter(
				m_pSpriteShader,
				pParticle->vDynamicParameter)) ||
			FAILED(m_pSpriteShader->Begin(iPass)) ||
			FAILED(m_pRectBuffer->Render()))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CEffect_Runtime::Render_MeshEmitter(
	const EFFECT_EMITTER_RUNTIME& Runtime)
{
	if (nullptr == Runtime.pModel)
		return S_OK;

	const bool_t isSkinned = Runtime.pModel->Is_Skinned();
	const shared_ptr<CShader> pShader =
		isSkinned ? m_pAnimMeshShader : m_pMeshShader;
	if (nullptr == pShader)
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Bind_Transform(
			pShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			pShader, "g_ProjMatrix", D3DTS::PROJ)) ||
		FAILED(Bind_EffectMaterialResources(
			pShader, Runtime)))
	{
		return E_FAIL;
	}

	const uint32_t iPass =
		(EFFECT_BLEND_MODE::ADDITIVE == Runtime.Desc.eBlendMode)
		? 1u
		: 0u;

	for (const EFFECT_PARTICLE& Particle :
		Runtime.Simulator.Get_Particles())
	{
		const float3_t vPosition = To_WorldPosition(Particle);
		const matrix_t WorldMatrix =
			XMMatrixScaling(
				max(0.0001f, fabsf(Particle.vSize.x)),
				max(0.0001f, fabsf(Particle.vSize.y)),
				max(0.0001f, fabsf(Particle.vSize.z))) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Particle.vRotation.x),
				XMConvertToRadians(Particle.vRotation.y),
				XMConvertToRadians(Particle.vRotation.z)) *
			XMMatrixTranslation(
				vPosition.x, vPosition.y, vPosition.z);

		float4x4_t WorldMatrixFloat{};
		XMStoreFloat4x4(&WorldMatrixFloat, WorldMatrix);
		if (FAILED(pShader->Bind_Matrix(
			"g_WorldMatrix", &WorldMatrixFloat)) ||
			FAILED(pShader->Bind_RawValue(
				"g_vTint", &Particle.vColor,
				sizeof(Particle.vColor))) ||
			FAILED(Bind_DynamicParameter(
				pShader,
					Particle.vDynamicParameter)))
		{
			return E_FAIL;
		}

		for (uint32_t i = 0;
			i < Runtime.pModel->Get_NumMeshes();
			++i)
		{
			const EFFECT_MODULE_DESC* pRequired = Find_Module(
				Runtime.Desc, EFFECT_MODULE_TYPE::REQUIRED);
			const bool_t hasRequiredTexture =
				nullptr != pRequired &&
				!pRequired->Required.strTextureAssetId.empty();

			if (hasRequiredTexture)
			{
				if (FAILED(pShader->Bind_Texture(
					"g_Texture",
					(nullptr != Runtime.pTextureSRV.Get())
						? Runtime.pTextureSRV
						: m_pFallbackTextureSRV)))
				{
					return E_FAIL;
				}
			}
			else if (FAILED(Runtime.pModel->Bind_Material(
				pShader,
				"g_Texture",
				i,
				aiTextureType_DIFFUSE)))
			{
				if (FAILED(pShader->Bind_Texture(
					"g_Texture",
					m_pFallbackTextureSRV)))
				{
					return E_FAIL;
				}
			}

			if ((isSkinned && FAILED(Runtime.pModel->Bind_BoneMatrices(
					pShader, "g_BoneMatrices", i))) ||
				FAILED(pShader->Begin(iPass)) ||
				FAILED(Runtime.pModel->Render(i)))
			{
				return E_FAIL;
			}
		}
	}
	return S_OK;
}

HRESULT CEffect_Runtime::Render_PrimitiveEmitter(
	const EFFECT_EMITTER_RUNTIME& Runtime)
{
	vector<float3_t> Points;
	vector<float4_t> Colors;
	f32_t fWidth = 1.f;

	if (EFFECT_EMITTER_TYPE::BEAM == Runtime.Desc.eType)
	{
		const EFFECT_MODULE_DESC* pBeam = Find_Module(
			Runtime.Desc, EFFECT_MODULE_TYPE::BEAM);
		if (nullptr != pBeam)
		{
			const float3_t vOrigin = Get_WorldOrigin();
			const float3_t vSource =
				Add(vOrigin, pBeam->Beam.vSourceOffset);
			const float3_t vTarget =
				Add(vOrigin, pBeam->Beam.vTargetOffset);
			const uint32_t iSegments =
				clamp(pBeam->Beam.iSegments, 1u, 256u);
			fWidth = max(0.001f, pBeam->Beam.fWidth);

			const float3_t vLineDirection =
				Normalize(Subtract(vTarget, vSource));
			const float3_t vNoiseAxis = Normalize(
				Cross(vLineDirection, { 0.f, 1.f, 0.f }),
				{ 1.f, 0.f, 0.f });

			Points.reserve(iSegments + 1);
			Colors.reserve(iSegments + 1);
			for (uint32_t i = 0; i <= iSegments; ++i)
			{
				const f32_t fRatio =
					static_cast<f32_t>(i) /
					static_cast<f32_t>(iSegments);
				const f32_t fNoise =
					sinf(fRatio * XM_2PI * 3.f) *
					pBeam->Beam.fNoiseAmplitude *
					sinf(fRatio * XM_PI);
				Points.push_back(Add(
					Lerp(vSource, vTarget, fRatio),
					Scale(vNoiseAxis, fNoise)));
				Colors.push_back({ 1.f, 1.f, 1.f, 1.f });
			}
		}
	}

	if (Points.size() < 2)
	{
		const EFFECT_MODULE_DESC* pTrail = Find_Module(
			Runtime.Desc, EFFECT_MODULE_TYPE::TRAIL);
		f32_t fPointLifetime = FLT_MAX;
		uint32_t iMaxPoints = MAX_TRAIL_POINTS;
		if (nullptr != pTrail)
		{
			fWidth = max(0.001f, pTrail->Trail.fWidth);
			fPointLifetime =
				max(0.001f, pTrail->Trail.fPointLifetime);
			iMaxPoints = clamp(
				pTrail->Trail.iMaxPoints,
				2u,
				MAX_TRAIL_POINTS);
		}

		const vector<EFFECT_PARTICLE>& Particles =
			Runtime.Simulator.Get_Particles();
		const size_t iFirstParticle =
			(Particles.size() > iMaxPoints)
			? Particles.size() - iMaxPoints
			: 0;
		for (size_t i = iFirstParticle; i < Particles.size(); ++i)
		{
			const EFFECT_PARTICLE& Particle = Particles[i];
			if (Particle.fAge > fPointLifetime)
				continue;
			Points.push_back(To_WorldPosition(Particle));
			Colors.push_back(Particle.vColor);
		}
	}

	if (Points.size() < 2)
		return S_OK;

	const float4_t* pCameraPosition =
		CGameInstance::Get().Get_CamPosition();
	const float3_t vCameraPosition =
		(nullptr != pCameraPosition)
		? float3_t{
			pCameraPosition->x,
			pCameraPosition->y,
			pCameraPosition->z
		}
		: float3_t{ 0.f, 0.f, -1.f };

	vector<VTXTEX> Vertices;
	Vertices.reserve((Points.size() - 1) * 6);
	for (size_t i = 0; i + 1 < Points.size(); ++i)
	{
		const float3_t vStart = Points[i];
		const float3_t vEnd = Points[i + 1];
		const float3_t vDirection =
			Normalize(Subtract(vEnd, vStart));
		const float3_t vMidPoint =
			Scale(Add(vStart, vEnd), 0.5f);
		const float3_t vViewDirection =
			Normalize(Subtract(vCameraPosition, vMidPoint),
				{ 0.f, 0.f, -1.f });
		const float3_t vSide = Scale(
			Normalize(Cross(vDirection, vViewDirection),
				{ 1.f, 0.f, 0.f }),
			fWidth * 0.5f);

		const float3_t vStartLeft = Subtract(vStart, vSide);
		const float3_t vStartRight = Add(vStart, vSide);
		const float3_t vEndLeft = Subtract(vEnd, vSide);
		const float3_t vEndRight = Add(vEnd, vSide);
		const f32_t fU0 =
			static_cast<f32_t>(i) /
			static_cast<f32_t>(Points.size() - 1);
		const f32_t fU1 =
			static_cast<f32_t>(i + 1) /
			static_cast<f32_t>(Points.size() - 1);

		VTXTEX Quad[6]{};
		Quad[0].vPosition = vStartLeft;
		Quad[0].vTexcoord = { fU0, 0.f };
		Quad[1].vPosition = vStartRight;
		Quad[1].vTexcoord = { fU0, 1.f };
		Quad[2].vPosition = vEndRight;
		Quad[2].vTexcoord = { fU1, 1.f };
		Quad[3].vPosition = vStartLeft;
		Quad[3].vTexcoord = { fU0, 0.f };
		Quad[4].vPosition = vEndRight;
		Quad[4].vTexcoord = { fU1, 1.f };
		Quad[5].vPosition = vEndLeft;
		Quad[5].vTexcoord = { fU1, 0.f };
		Vertices.insert(Vertices.end(), begin(Quad), end(Quad));
	}

	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	if (!Colors.empty())
	{
		vTint = {};
		for (const float4_t& vColor : Colors)
		{
			vTint.x += vColor.x;
			vTint.y += vColor.y;
			vTint.z += vColor.z;
			vTint.w += vColor.w;
		}
		const f32_t fInverseCount =
			1.f / static_cast<f32_t>(Colors.size());
		vTint.x *= fInverseCount;
		vTint.y *= fInverseCount;
		vTint.z *= fInverseCount;
		vTint.w *= fInverseCount;
	}

	float4_t vDynamicParameter = { 1.f, 1.f, 0.f, 1.f };
	const vector<EFFECT_PARTICLE>& DynamicParticles =
		Runtime.Simulator.Get_Particles();
	if (!DynamicParticles.empty())
	{
		vDynamicParameter = {};
		for (const EFFECT_PARTICLE& Particle : DynamicParticles)
		{
			vDynamicParameter.x += Particle.vDynamicParameter.x;
			vDynamicParameter.y += Particle.vDynamicParameter.y;
			vDynamicParameter.z += Particle.vDynamicParameter.z;
			vDynamicParameter.w += Particle.vDynamicParameter.w;
		}
		const f32_t fInverseCount =
			1.f / static_cast<f32_t>(DynamicParticles.size());
		vDynamicParameter.x *= fInverseCount;
		vDynamicParameter.y *= fInverseCount;
		vDynamicParameter.z *= fInverseCount;
		vDynamicParameter.w *= fInverseCount;
	}

	return Draw_PrimitiveVertices(
		Vertices, Runtime, vTint, vDynamicParameter);
}

HRESULT CEffect_Runtime::Bind_CommonSpriteResources(
	const EFFECT_EMITTER_RUNTIME& Runtime)
{
	if (FAILED(CGameInstance::Get().Bind_Transform(
		m_pSpriteShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pSpriteShader, "g_ProjMatrix", D3DTS::PROJ)) ||
		FAILED(CGameInstance::Get().Bind_InverseTransform(
			m_pSpriteShader, "g_ViewMatrixInverse", D3DTS::VIEW)) ||
		FAILED(m_pSpriteShader->Bind_Texture(
			"g_Texture",
			(nullptr != Runtime.pTextureSRV.Get())
			? Runtime.pTextureSRV
			: m_pFallbackTextureSRV)) ||
		FAILED(Bind_EffectMaterialResources(
			m_pSpriteShader, Runtime)))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CEffect_Runtime::Bind_CommonPrimitiveResources(
	const EFFECT_EMITTER_RUNTIME& Runtime,
	const float4_t& vTint,
	fmatrix_t WorldMatrix)
{
	float4x4_t WorldMatrixFloat{};
	XMStoreFloat4x4(&WorldMatrixFloat, WorldMatrix);

	if (FAILED(m_pPrimitiveShader->Bind_Matrix(
		"g_WorldMatrix", &WorldMatrixFloat)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pPrimitiveShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pPrimitiveShader, "g_ProjMatrix", D3DTS::PROJ)) ||
		FAILED(m_pPrimitiveShader->Bind_RawValue(
			"g_vTint", &vTint, sizeof(vTint))) ||
		FAILED(m_pPrimitiveShader->Bind_Texture(
			"g_Texture",
			(nullptr != Runtime.pTextureSRV.Get())
			? Runtime.pTextureSRV
			: m_pFallbackTextureSRV)) ||
		FAILED(Bind_EffectMaterialResources(
			m_pPrimitiveShader, Runtime)))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CEffect_Runtime::Bind_EffectMaterialResources(
	shared_ptr<Engine::CShader> pShader,
	const EFFECT_EMITTER_RUNTIME& Runtime)
{
	if (nullptr == pShader)
		return E_FAIL;

	EFFECT_MATERIAL_DESC Material{};
	const EFFECT_MODULE_DESC* pRequired = Find_Module(
		Runtime.Desc, EFFECT_MODULE_TYPE::REQUIRED);
	if (nullptr != pRequired)
		Material = pRequired->Required.Material;

	const float4_t vDefaultDynamicParameter = {
		1.f, 1.f, 0.f, 1.f
	};
	const f32_t fEffectTime = Runtime.Simulator.Get_ElapsedTime();
	const float2_t vViewportSize =
		CGameInstance::Get().Get_ViewportSize();

	if (FAILED(pShader->Bind_Texture(
			"g_OpacityTexture",
			(nullptr != Runtime.pOpacityTextureSRV.Get())
			? Runtime.pOpacityTextureSRV
			: m_pFallbackTextureSRV)) ||
		FAILED(pShader->Bind_Texture(
			"g_DissolveTexture",
			(nullptr != Runtime.pDissolveTextureSRV.Get())
			? Runtime.pDissolveTextureSRV
			: m_pFallbackTextureSRV)) ||
		FAILED(pShader->Bind_Texture(
			"g_DistortionTexture",
			(nullptr != Runtime.pDistortionTextureSRV.Get())
			? Runtime.pDistortionTextureSRV
			: m_pFallbackTextureSRV)) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), pShader, "g_EffectDepthTexture")) ||
		FAILED(pShader->Bind_RawValue(
			"g_vUVTiling", &Material.vUVTiling,
			sizeof(Material.vUVTiling))) ||
		FAILED(pShader->Bind_RawValue(
			"g_vUVOffset", &Material.vUVOffset,
			sizeof(Material.vUVOffset))) ||
		FAILED(pShader->Bind_RawValue(
			"g_vUVPanner", &Material.vUVPanner,
			sizeof(Material.vUVPanner))) ||
		FAILED(pShader->Bind_RawValue(
			"g_vEffectViewportSize", &vViewportSize,
			sizeof(vViewportSize))) ||
		FAILED(pShader->Bind_RawValue(
			"g_vDissolveEdgeColor", &Material.vDissolveEdgeColor,
			sizeof(Material.vDissolveEdgeColor))) ||
		FAILED(pShader->Bind_RawValue(
			"g_vDynamicParameter", &vDefaultDynamicParameter,
			sizeof(vDefaultDynamicParameter))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fEffectTime", &fEffectTime,
			sizeof(fEffectTime))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fEmissiveStrength", &Material.fEmissiveStrength,
			sizeof(Material.fEmissiveStrength))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fOpacityMaskThreshold",
			&Material.fOpacityMaskThreshold,
			sizeof(Material.fOpacityMaskThreshold))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fDissolveAmount", &Material.fDissolveAmount,
			sizeof(Material.fDissolveAmount))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fDissolveEdgeWidth", &Material.fDissolveEdgeWidth,
			sizeof(Material.fDissolveEdgeWidth))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fSoftParticleDistance", &Material.fSoftParticleDistance,
			sizeof(Material.fSoftParticleDistance))) ||
		FAILED(pShader->Bind_RawValue(
			"g_fDistortionStrength", &Material.fDistortionStrength,
			sizeof(Material.fDistortionStrength))))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CEffect_Runtime::Bind_DynamicParameter(
	shared_ptr<Engine::CShader> pShader,
	const float4_t& vDynamicParameter)
{
	if (nullptr == pShader)
		return E_FAIL;
	return pShader->Bind_RawValue(
		"g_vDynamicParameter", &vDynamicParameter,
		sizeof(vDynamicParameter));
}

HRESULT CEffect_Runtime::Ensure_PrimitiveBuffer(
	const uint32_t iVertexCount)
{
	if (0 == iVertexCount)
		return S_OK;
	if (iVertexCount > MAX_PRIMITIVE_VERTICES)
		return E_FAIL;
	if (nullptr != m_pPrimitiveVertexBuffer.Get() &&
		iVertexCount <= m_iPrimitiveVertexCapacity)
	{
		return S_OK;
	}

	uint32_t iNewCapacity = max(64u, m_iPrimitiveVertexCapacity);
	while (iNewCapacity < iVertexCount)
	{
		if (iNewCapacity > MAX_PRIMITIVE_VERTICES / 2u)
		{
			iNewCapacity = MAX_PRIMITIVE_VERTICES;
			break;
		}
		iNewCapacity *= 2u;
	}
	if (iNewCapacity < iVertexCount ||
		iNewCapacity > MAX_PRIMITIVE_VERTICES)
	{
		return E_FAIL;
	}

	D3D11_BUFFER_DESC BufferDesc{};
	const size_t iBufferByteWidth =
		sizeof(VTXTEX) * static_cast<size_t>(iNewCapacity);
	if (iBufferByteWidth > UINT32_MAX)
		return E_FAIL;
	BufferDesc.ByteWidth = static_cast<uint32_t>(iBufferByteWidth);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> pNewBuffer;
	if (FAILED(m_pDevice->CreateBuffer(
		&BufferDesc, nullptr, &pNewBuffer)))
	{
		return E_FAIL;
	}

	m_pPrimitiveVertexBuffer = pNewBuffer;
	m_iPrimitiveVertexCapacity = iNewCapacity;
	return S_OK;
}

HRESULT CEffect_Runtime::Draw_PrimitiveVertices(
	const vector<VTXTEX>& Vertices,
	const EFFECT_EMITTER_RUNTIME& Runtime,
	const float4_t& vTint,
	const float4_t& vDynamicParameter)
{
	if (Vertices.empty())
		return S_OK;
	if (Vertices.size() > MAX_PRIMITIVE_VERTICES)
		return E_FAIL;
	if (FAILED(Ensure_PrimitiveBuffer(
		static_cast<uint32_t>(Vertices.size()))))
	{
		return E_FAIL;
	}

	D3D11_MAPPED_SUBRESOURCE MappedResource{};
	if (FAILED(m_pContext->Map(
		m_pPrimitiveVertexBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&MappedResource)))
	{
		return E_FAIL;
	}
	memcpy(
		MappedResource.pData,
		Vertices.data(),
		sizeof(VTXTEX) * Vertices.size());
	m_pContext->Unmap(m_pPrimitiveVertexBuffer.Get(), 0);

	if (FAILED(Bind_CommonPrimitiveResources(
		Runtime, vTint, XMMatrixIdentity())) ||
		FAILED(Bind_DynamicParameter(
			m_pPrimitiveShader, vDynamicParameter)))
	{
		return E_FAIL;
	}

	ID3D11Buffer* pVertexBuffers[] = {
		m_pPrimitiveVertexBuffer.Get()
	};
	const uint32_t iStrides[] = { sizeof(VTXTEX) };
	const uint32_t iOffsets[] = { 0 };
	m_pContext->IASetVertexBuffers(
		0, 1, pVertexBuffers, iStrides, iOffsets);
	m_pContext->IASetIndexBuffer(
		nullptr, DXGI_FORMAT_UNKNOWN, 0);
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const uint32_t iPass =
		(EFFECT_BLEND_MODE::ADDITIVE == Runtime.Desc.eBlendMode)
		? 1u
		: 0u;
	if (FAILED(m_pPrimitiveShader->Begin(iPass)))
		return E_FAIL;

	if (Engine::CProfiler* pProfiler =
		Engine::CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(Engine::EProfilerCounter::DrawCalls);
	}

	m_pContext->Draw(
		static_cast<uint32_t>(Vertices.size()), 0);
	return S_OK;
}

float3_t CEffect_Runtime::Get_WorldOrigin() const
{
	return m_vWorldPosition;
}

float3_t CEffect_Runtime::To_WorldPosition(
	const EFFECT_PARTICLE& Particle) const
{
	/*
	 * The editor preview object is stationary.  Both local and world-space
	 * emitters therefore share the same initial origin in this vertical slice.
	 * The distinction remains in the serialized data for a later moving-owner
	 * binding, where world particles retain their spawn transform.
	 */
	return Add(Get_WorldOrigin(), Particle.vPosition);
}

float4_t CEffect_Runtime::Calculate_UVRect(
	const EFFECT_EMITTER_DESC& EmitterDesc,
	const EFFECT_PARTICLE& Particle) const
{
	const EFFECT_MODULE_DESC* pSubUV = Find_Module(
		EmitterDesc, EFFECT_MODULE_TYPE::SUB_UV);
	if (nullptr == pSubUV)
		return { 0.f, 0.f, 1.f, 1.f };

	const uint32_t iColumns =
		max(1u, pSubUV->SubUV.iColumns);
	const uint32_t iRows =
		max(1u, pSubUV->SubUV.iRows);
	const uint32_t iFrameCount = iColumns * iRows;
	const uint32_t iFrame =
		(0 == iFrameCount)
		? 0u
		: Particle.iSubUVFrame % iFrameCount;
	const uint32_t iColumn = iFrame % iColumns;
	const uint32_t iRow = iFrame / iColumns;
	const f32_t fInvColumns =
		1.f / static_cast<f32_t>(iColumns);
	const f32_t fInvRows =
		1.f / static_cast<f32_t>(iRows);

	return {
		static_cast<f32_t>(iColumn) * fInvColumns,
		static_cast<f32_t>(iRow) * fInvRows,
		static_cast<f32_t>(iColumn + 1) * fInvColumns,
		static_cast<f32_t>(iRow + 1) * fInvRows
	};
}

unique_ptr<CEffect_Runtime> CEffect_Runtime::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CEffect_Runtime>(
		new CEffect_Runtime(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CPrototype> CEffect_Runtime::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CEffect_Runtime>(
		new CEffect_Runtime(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}
