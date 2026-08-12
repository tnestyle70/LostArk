#include "Model.h"

#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "Material.h"
#include "Animation.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

CModel::CModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
    : CComponent { Prototype }
    , m_pAIScene { Prototype.m_pAIScene }
    , m_eType { Prototype.m_eType }
    , m_iNumMeshes { Prototype.m_iNumMeshes }
    , m_Meshes { Prototype.m_Meshes }
    , m_PreTransformMatrix { Prototype.m_PreTransformMatrix }
    , m_iNumMaterials { Prototype.m_iNumMaterials }
    , m_Materials { Prototype.m_Materials }
   // , m_Bones { Prototype.m_Bones }
    , m_iSkeletonHash { Prototype.m_iSkeletonHash }
    , m_iCurrentAnimIndex { Prototype.m_iCurrentAnimIndex }
    , m_iNumAnimations { Prototype.m_iNumAnimations}
    // , m_Animations { Prototype.m_Animations }
    , m_isAnimLoop { Prototype.m_isAnimLoop }
	, m_isAnimPaused { Prototype.m_isAnimPaused }
	, m_fAnimationSpeed { Prototype.m_fAnimationSpeed }
	, m_iRootMotionBoneIndex { Prototype.m_iRootMotionBoneIndex }
	, m_iRootMotionVerticalAxis { Prototype.m_iRootMotionVerticalAxis }
	, m_vRootMotionRestTranslation { Prototype.m_vRootMotionRestTranslation }
	, m_bHasLocalBounds { Prototype.m_bHasLocalBounds }
	, m_vLocalBoundsMin { Prototype.m_vLocalBoundsMin }
	, m_vLocalBoundsMax { Prototype.m_vLocalBoundsMax }
	, m_bHasSelfConsistentUnauthenticatedGeometryMetadata { Prototype.m_bHasSelfConsistentUnauthenticatedGeometryMetadata }
	, m_iGeometryFormatVersionMajor { Prototype.m_iGeometryFormatVersionMajor }
	, m_iGeometryFormatVersionMinor { Prototype.m_iGeometryFormatVersionMinor }
	, m_iGeometryChannelMask { Prototype.m_iGeometryChannelMask }
	, m_iGeometryEvidenceFlags { Prototype.m_iGeometryEvidenceFlags }
	, m_fGeometryPreScale { Prototype.m_fGeometryPreScale }
	, m_GeometryPayloadSha256 { Prototype.m_GeometryPayloadSha256 }
	, m_GeometryMetadataIdentitySha256 { Prototype.m_GeometryMetadataIdentitySha256 }
{
    for (auto& pPrototype : Prototype.m_Bones)
        m_Bones.push_back(pPrototype->Clone());

    for (auto& pPrototype : Prototype.m_Animations)    
        m_Animations.push_back(pPrototype->Clone());
}

CModel::~CModel()
{
}

matrix_t CModel::Get_BoneMatrix(const char_t* pBoneName)
{
    auto    iter = find_if(m_Bones.begin(), m_Bones.end(), [&](shared_ptr<CBone> pBone)->bool_t {
        if (true == pBone->Compare_Name(pBoneName))
            return true;
        return false;
    });

    if (iter == m_Bones.end())
        return XMMatrixIdentity();

    return (*iter)->Get_CombinedTransformationMatrix();    
}

bool_t CModel::Set_Animation(
    const char_t* pAnimationName,
    bool_t isLoop,
    f32_t fBlendSeconds)
{
    if (nullptr == pAnimationName)
        return false;

    for (uint32_t i = 0; i < m_Animations.size(); ++i)
    {
        if (!m_Animations[i]->Compare_Name(pAnimationName))
            continue;

        if (i != m_iCurrentAnimIndex)
            Begin_AnimBlend(fBlendSeconds);

        m_iCurrentAnimIndex = i;
        m_isAnimLoop = isLoop;
        return true;
    }
    return false;
}

void CModel::Begin_AnimBlend(f32_t fBlendSeconds)
{
    if (fBlendSeconds <= 0.f || m_Bones.empty())
    {
        m_fBlendDuration = 0.f;
        m_fBlendElapsed = 0.f;
        return;
    }

    m_BlendFromPose.resize(m_Bones.size());
    for (size_t i = 0; i < m_Bones.size(); ++i)
        XMStoreFloat4x4(
            &m_BlendFromPose[i],
            m_Bones[i]->Get_TransformationMatrix());

    m_fBlendDuration = fBlendSeconds;
    m_fBlendElapsed = 0.f;
}

void CModel::Update_AnimBlend(f32_t fTimeDelta)
{
    if (m_fBlendElapsed >= m_fBlendDuration ||
        m_BlendFromPose.size() != m_Bones.size())
        return;

    m_fBlendElapsed += fTimeDelta;
    const f32_t fRatio = m_fBlendDuration > 0.f ?
        (min)(m_fBlendElapsed / m_fBlendDuration, 1.f) : 1.f;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        m_Bones[i]->Blend_TransformationMatrix(
            XMLoadFloat4x4(&m_BlendFromPose[i]),
            fRatio);
    }
}

bool_t CModel::Has_Bone(const char_t* pBoneName)
{
    if (nullptr == pBoneName || '\0' == pBoneName[0])
        return false;

    return m_Bones.end() != find_if(
        m_Bones.begin(),
        m_Bones.end(),
        [&](const shared_ptr<CBone>& pBone)
        {
            return nullptr != pBone && pBone->Compare_Name(pBoneName);
        });
}

int32_t CModel::Find_BoneIndex(const char_t* pBoneName) const
{
    if (nullptr == pBoneName || '\0' == pBoneName[0])
        return -1;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        if (nullptr != m_Bones[i] && m_Bones[i]->Compare_Name(pBoneName))
            return static_cast<int32_t>(i);
    }
    return -1;
}

int32_t CModel::Get_BoneParentIndex(const uint32_t iBoneIndex) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return -1;

    return m_Bones[iBoneIndex]->Get_ParentBoneIndex();
}

bool_t CModel::Get_BoneLocalMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    outMatrix = m_Bones[iBoneIndex]->Get_TransformationMatrix();
    return true;
}

bool_t CModel::Get_BoneCombinedMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    outMatrix = m_Bones[iBoneIndex]->Get_CombinedTransformationMatrix();
    return true;
}

bool_t CModel::Set_BoneLocalMatrix(
    const uint32_t iBoneIndex, fmatrix_t Matrix)
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    m_Bones[iBoneIndex]->Update_TransformationMatrix(Matrix);
    return true;
}

void CModel::Refresh_BoneCombinedMatrices()
{
    for (auto& pBone : m_Bones)
    {
        pBone->Update_CombinedTransformationMatrix(
            m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

bool_t CModel::Enable_RootMotionSuppression(
    const char_t* pBoneName, const int32_t iVerticalAxis)
{
    if (nullptr == pBoneName || '\0' == pBoneName[0] ||
        iVerticalAxis < -1 || iVerticalAxis > 2)
        return false;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        if (nullptr == m_Bones[i] || !m_Bones[i]->Compare_Name(pBoneName))
            continue;

        float4x4_t rest{};
        XMStoreFloat4x4(&rest, m_Bones[i]->Get_TransformationMatrix());
        m_vRootMotionRestTranslation = { rest._41, rest._42, rest._43 };
        m_iRootMotionBoneIndex = static_cast<int32_t>(i);
        m_iRootMotionVerticalAxis = iVerticalAxis;
        return true;
    }
    return false;
}

bool_t CModel::Start_Animation(
	const uint32_t iAnimIndex,
	const bool_t isLoop)
{
	if (iAnimIndex >= m_Animations.size())
		return false;
	m_iCurrentAnimIndex = iAnimIndex;
	m_isAnimLoop = isLoop;
	m_isAnimPaused = false;
	m_Animations[iAnimIndex]->Set_TrackPosition(0.f);
	Play_Animation(0.f);
	return true;
}

bool_t CModel::Start_Animation(
	const char_t* pAnimationName,
	const bool_t isLoop)
{
	if (!Set_Animation(pAnimationName, isLoop))
		return false;
	return Start_Animation(m_iCurrentAnimIndex, isLoop);
}

void CModel::Stop_Animation()
{
	m_isAnimPaused = true;
}

void CModel::Set_AnimationSpeed(const f32_t speed)
{
	m_fAnimationSpeed = isfinite(speed)
		? clamp(speed, -16.f, 16.f) : 1.f;
}

bool_t CModel::Update_Animation(const f32_t fTimeDelta)
{
	if (!isfinite(fTimeDelta))
		return false;
	return Play_Animation(fTimeDelta * m_fAnimationSpeed);
}

const char_t* CModel::Get_AnimationName(uint32_t iAnimIndex) const
{
    if (iAnimIndex >= m_Animations.size())
        return nullptr;

    return m_Animations[iAnimIndex]->Get_Name();
}

bool_t CModel::Get_AnimationProgress(uint32_t iAnimIndex, f32_t& fOutPosition, f32_t& fOutDuration) const
{
    if (iAnimIndex >= m_Animations.size())
        return false;

    fOutPosition = m_Animations[iAnimIndex]->Get_CurrentTrackPosition();
    fOutDuration = m_Animations[iAnimIndex]->Get_Duration();
    return true;
}

/* 트랙 위치(틱)를 시간으로 환산할 때 쓴다. 유효하지 않으면 0을 돌려주므로
호출부가 자체 기본값을 쓸지 판단할 수 있다. */
f32_t CModel::Get_AnimationTickPerSecond(uint32_t iAnimIndex) const
{
    if (iAnimIndex >= m_Animations.size())
        return 0.f;

    return m_Animations[iAnimIndex]->Get_TickPerSecond();
}

bool_t CModel::Set_AnimTrackPosition(uint32_t iAnimIndex, f32_t fTrackPosition)
{
    if (iAnimIndex >= m_Animations.size())
        return false;

    m_Animations[iAnimIndex]->Set_TrackPosition(fTrackPosition);
    return true;
}

HRESULT CModel::Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix)
{
    if (nullptr == pModelFilePath)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
    m_eType = eType;
	Reset_LocalBounds();

    string extension = filesystem::path(pModelFilePath).extension().string();
    transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) { return static_cast<char_t>(tolower(value)); });
    if (".wmodel" == extension)
        return Ready_BinaryModel(pModelFilePath);

    uint32_t  iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };

    if (MODEL::NONANIM == eType)
        iFlag |= aiProcess_PreTransformVertices;

    m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);
    if (nullptr == m_pAIScene)
        return E_FAIL;

    if (FAILED(Ready_Bones(m_pAIScene->mRootNode)))
        return E_FAIL;

    if (FAILED(Ready_Meshes()))
        return E_FAIL;

    if (FAILED(Ready_Materials(pModelFilePath)))
        return E_FAIL;

    if (FAILED(Ready_Animations()))
        return E_FAIL;

    return S_OK;
}

HRESULT CModel::Initialize_Prototype(
	const MODEL eType,
	const MODEL_ASSET_LOAD_DESC& loadDesc,
	fmatrix_t PreTransformMatrix)
{
	if (loadDesc.meshPath.empty())
		return E_INVALIDARG;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
	m_eType = eType;
	Reset_LocalBounds();
	return Ready_BinaryModel(loadDesc);
}

HRESULT CModel::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CModel::Render(uint32_t iMeshIndex)
{
    if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_Meshes[iMeshIndex]->Render()))
        return E_FAIL;


    return S_OK;
}

HRESULT CModel::Render_Instanced(uint32_t iMeshIndex,
    ID3D11Buffer* pInstanceBuffer, uint32_t iInstanceStride, uint32_t iNumInstances)
{
    if (iMeshIndex >= m_Meshes.size() ||
        nullptr == m_Meshes[iMeshIndex])
    {
        return E_INVALIDARG;
    }

    return m_Meshes[iMeshIndex]->Render_Instanced(
        pInstanceBuffer, iInstanceStride, iNumInstances);
}

bool_t CModel::Play_Animation(f32_t fTimeDelta)
{
    if (m_Animations.empty() || m_iCurrentAnimIndex >= m_Animations.size())
        return false;

    bool_t      isFinished = { false };
    /* 내가 로드한 애니메이션 중, 
    현재 취해야하는 애니메이션의 포즈뼈들의 m_TransformationMatrix를 갱신해준다. */
    /* 일시정지 중에는 재생 위치를 전진시키지 않되, 뼈 행렬은 그대로 다시 계산해
    현재 프레임의 포즈를 유지한다. */
    isFinished = m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrix(
        m_isAnimPaused ? 0.f : fTimeDelta, m_Bones, m_isAnimLoop);

    Update_AnimBlend(m_isAnimPaused ? 0.f : fTimeDelta);

    if (m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size())
    {
        const shared_ptr<CBone>& pRoot = m_Bones[m_iRootMotionBoneIndex];
        if (nullptr != pRoot)
        {
            float4x4_t local{};
            XMStoreFloat4x4(&local, pRoot->Get_TransformationMatrix());
            if (0 != m_iRootMotionVerticalAxis)
                local._41 = m_vRootMotionRestTranslation.x;
            if (1 != m_iRootMotionVerticalAxis)
                local._42 = m_vRootMotionRestTranslation.y;
            if (2 != m_iRootMotionVerticalAxis)
                local._43 = m_vRootMotionRestTranslation.z;
            pRoot->Update_TransformationMatrix(XMLoadFloat4x4(&local));
        }
    }

    /* 뼈들 자체 행렬은 갱신이 됐지만, 최종행렬은 아직 미완성(m_Transformation * Parent`s CombinedTransfor4mationMatrix). */
    for (auto& pBone : m_Bones)
    {
        pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }

    return isFinished;
}

HRESULT CModel::Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex)
{
   return m_Meshes[iMeshIndex]->Bind_Resource(pShader, pConstantName, m_Bones);    
}

HRESULT CModel::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex)
{    
    uint32_t        iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();

    if (iMaterialIndex >= m_iNumMaterials)
        return E_FAIL;

    return m_Materials[iMaterialIndex]->Bind_Material(pShader, pConstantName, eType, iTextureIndex);
}

bool_t CModel::Has_MaterialTexture(uint32_t iMeshIndex,
    aiTextureType eType,
    uint32_t iTextureIndex) const
{
    if (iMeshIndex >= m_Meshes.size())
        return false;

    const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    return materialIndex < m_Materials.size() &&
        m_Materials[materialIndex]->Has_Texture(eType, iTextureIndex);
}

const MODEL_COLOR_TINT* CModel::Get_MaterialColorTint(
    uint32_t iMeshIndex) const
{
    if (iMeshIndex >= m_Meshes.size())
        return nullptr;

    const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size())
        return nullptr;

    return &m_Materials[materialIndex]->Get_ColorTint();
}

const string& CModel::Get_MaterialName(uint32_t iMeshIndex) const
{
	static const string Empty;
	if (iMeshIndex >= m_Meshes.size())
		return Empty;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Get_Name() : Empty;
}

uint64_t CModel::Get_MaterialNameHash(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return 0u;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Get_NameHash() : 0u;
}

HRESULT CModel::Ready_Meshes()
{
    m_iNumMeshes = m_pAIScene->mNumMeshes;

    for (size_t i = 0; i < m_iNumMeshes; i++)
    {
		if (MODEL::NONANIM == m_eType)
		{
			const aiMesh* pAIMesh = m_pAIScene->mMeshes[i];
			for (uint32_t vertexIndex = 0; vertexIndex < pAIMesh->mNumVertices; ++vertexIndex)
			{
				float3_t position{};
				memcpy(&position, &pAIMesh->mVertices[vertexIndex], sizeof(float3_t));
				Include_LocalPosition(XMVector3TransformCoord(
					XMLoadFloat3(&position), XMLoadFloat4x4(&m_PreTransformMatrix)));
			}
		}

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_pAIScene->mMeshes[i], m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;

        m_Meshes.push_back(pMesh);
    }

    return S_OK;
}

HRESULT CModel::Ready_Materials(const char_t* pModelFilePath)
{
    m_iNumMaterials = m_pAIScene->mNumMaterials;

    for (uint32_t i = 0; i < m_iNumMaterials; i++)
    {
        auto        pMaterial = CMaterial::Create(m_pDevice, m_pContext, m_pAIScene->mMaterials[i], pModelFilePath);
        if (nullptr == pMaterial)
            return E_FAIL;

        m_Materials.push_back(pMaterial);
    }

    return S_OK;
}

HRESULT CModel::Ready_Bones(const aiNode* pAINode, int32_t iParentBoneIndex)
{
    auto        pBone = CBone::Create(pAINode, iParentBoneIndex);
    if (nullptr == pBone)
        return E_FAIL;

    m_Bones.push_back(pBone);

    int32_t     iParentIndex = m_Bones.size() - 1;

    for (uint32_t i = 0; i < pAINode->mNumChildren; ++i)
    {
        Ready_Bones(pAINode->mChildren[i], iParentIndex);
    }

    return S_OK;
}

HRESULT CModel::Ready_Animations()
{
    m_iNumAnimations = m_pAIScene->mNumAnimations;

    for (uint32_t i = 0; i < m_iNumAnimations; i++)
    {
        auto      pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;

        m_Animations.push_back(pAnimation);
    }

    return S_OK;
}

HRESULT CModel::Ready_BinaryModel(const char_t* pModelFilePath)
{
    MODEL_ASSET_LOAD_DESC desc{};
    desc.meshPath = filesystem::path(pModelFilePath).lexically_normal();

    filesystem::path absolutePath = filesystem::absolute(desc.meshPath).lexically_normal();
    for (filesystem::path current = absolutePath.parent_path();
        !current.empty(); current = current.parent_path())
    {
        if (L"Resources" == current.filename())
        {
            desc.assetRoot = current;
            break;
        }
        if (current == current.root_path())
            break;
    }

    return Ready_BinaryModel(desc);
}

HRESULT CModel::Ready_BinaryModel(
	const MODEL_ASSET_LOAD_DESC& loadDesc)
{
	MODEL_ASSET_DATA asset{};
	if (!CModelDecoderRegistry::Get().Decode(loadDesc, asset) ||
		asset.meshes.empty() ||
		((MODEL::ANIM == m_eType) != asset.hasSkeleton))
	{
		return E_FAIL;
	}
	m_iSkeletonHash = asset.hasSkeleton ? asset.skeleton.skeletonHash : 0;

	m_bHasSelfConsistentUnauthenticatedGeometryMetadata =
		asset.geometryMetadata.present;
	m_iGeometryFormatVersionMajor = {};
	m_iGeometryFormatVersionMinor = {};
	m_iGeometryChannelMask = {};
	m_iGeometryEvidenceFlags = {};
	m_fGeometryPreScale = 1.f;
	m_GeometryPayloadSha256.fill(0);
	m_GeometryMetadataIdentitySha256.fill(0);
	if (m_bHasSelfConsistentUnauthenticatedGeometryMetadata)
	{
		m_iGeometryFormatVersionMajor = asset.geometryMetadata.versionMajor;
		m_iGeometryFormatVersionMinor = asset.geometryMetadata.versionMinor;
		m_iGeometryChannelMask = asset.geometryMetadata.channelMask;
		m_iGeometryEvidenceFlags = asset.geometryMetadata.evidenceFlags;
		m_fGeometryPreScale = asset.geometryMetadata.geometryPreScale;
		m_GeometryPayloadSha256 = asset.geometryMetadata.payloadSha256;
		m_GeometryMetadataIdentitySha256 =
			asset.geometryMetadata.metadataIdentitySha256;
	}

	if (FAILED(Ready_Bones(asset)) ||
		FAILED(Ready_Meshes(asset)) ||
		FAILED(Ready_Materials(asset)) ||
		FAILED(Ready_Animations(asset)))
	{
		return E_FAIL;
	}

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(
			m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));

	if (!asset.animations.empty())
	{
		uint32_t animationIndex = {};
		if (!loadDesc.defaultAnimationName.empty())
		{
			const auto iterator = find_if(
				asset.animations.begin(), asset.animations.end(),
				[&loadDesc](const MODEL_ANIMATION_DATA& animation)
				{
					return animation.name ==
						loadDesc.defaultAnimationName;
				});
			if (iterator == asset.animations.end())
				return E_FAIL;
			animationIndex = static_cast<uint32_t>(distance(
				asset.animations.begin(), iterator));
		}
		if (!Start_Animation(
			animationIndex,
			asset.animations[animationIndex].defaultLoop))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CModel::Ready_Meshes(const MODEL_ASSET_DATA& asset)
{
    m_iNumMeshes = static_cast<uint32_t>(asset.meshes.size());
    m_Meshes.reserve(m_iNumMeshes);
	for (const MODEL_MESH_DATA& mesh : asset.meshes)
	{
		if (MODEL::NONANIM == m_eType)
		{
			if (mesh.embeddedBounds.present)
			{
				const float3_t& minimum = mesh.embeddedBounds.minimum;
				const float3_t& maximum = mesh.embeddedBounds.maximum;
				for (uint32_t corner = 0; corner < 8; ++corner)
				{
					const float3_t position = {
						0 != (corner & 1) ? maximum.x : minimum.x,
						0 != (corner & 2) ? maximum.y : minimum.y,
						0 != (corner & 4) ? maximum.z : minimum.z,
					};
					Include_LocalPosition(XMVector3TransformCoord(
						XMLoadFloat3(&position),
						XMLoadFloat4x4(&m_PreTransformMatrix)));
				}
			}
			else
			{
				for (const VTXMESH& vertex : mesh.vertices)
				{
					Include_LocalPosition(XMVector3TransformCoord(
						XMLoadFloat3(&vertex.vPosition),
						XMLoadFloat4x4(&m_PreTransformMatrix)));
				}
			}
		}

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType,
            mesh, asset.skeleton, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;
        m_Meshes.push_back(pMesh);
    }
    return S_OK;
}

void CModel::Reset_LocalBounds()
{
	const f32_t maximum = (numeric_limits<f32_t>::max)();
	m_vLocalBoundsMin = float3_t(maximum, maximum, maximum);
	m_vLocalBoundsMax = float3_t(-maximum, -maximum, -maximum);
	m_bHasLocalBounds = false;
}

void CModel::Include_LocalPosition(fvector_t vPosition)
{
	float3_t position{};
	XMStoreFloat3(&position, vPosition);
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
		return;

	m_vLocalBoundsMin.x = (min)(m_vLocalBoundsMin.x, position.x);
	m_vLocalBoundsMin.y = (min)(m_vLocalBoundsMin.y, position.y);
	m_vLocalBoundsMin.z = (min)(m_vLocalBoundsMin.z, position.z);
	m_vLocalBoundsMax.x = (max)(m_vLocalBoundsMax.x, position.x);
	m_vLocalBoundsMax.y = (max)(m_vLocalBoundsMax.y, position.y);
	m_vLocalBoundsMax.z = (max)(m_vLocalBoundsMax.z, position.z);
	m_bHasLocalBounds = true;
}

HRESULT CModel::Ready_Materials(const MODEL_ASSET_DATA& asset)
{
    m_iNumMaterials = static_cast<uint32_t>(asset.materials.size());
    m_Materials.reserve(m_iNumMaterials);
    for (const MODEL_MATERIAL_DATA& material : asset.materials)
    {
        auto pMaterial = CMaterial::Create(m_pDevice, m_pContext, material);
        if (nullptr == pMaterial)
            return E_FAIL;
        m_Materials.push_back(pMaterial);
    }
    return S_OK;
}

HRESULT CModel::Ready_Bones(const MODEL_ASSET_DATA& asset)
{
    m_Bones.reserve(asset.skeleton.bones.size());
    for (const MODEL_BONE_DATA& bone : asset.skeleton.bones)
    {
        auto pBone = CBone::Create(bone);
        if (nullptr == pBone)
            return E_FAIL;
        m_Bones.push_back(pBone);
    }
    return S_OK;
}

HRESULT CModel::Ready_Animations(const MODEL_ASSET_DATA& asset)
{
    m_iNumAnimations = static_cast<uint32_t>(asset.animations.size());
    m_Animations.reserve(m_iNumAnimations);
    for (const MODEL_ANIMATION_DATA& animation : asset.animations)
    {
        auto pAnimation = CAnimation::Create(animation, m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;
        m_Animations.push_back(pAnimation);
    }
    return S_OK;
}

HRESULT CModel::Attach_AnimationSet(const CModel& animationSet)
{
	if (MODEL::ANIM != m_eType || MODEL::ANIM != animationSet.m_eType ||
		m_Bones.empty() || animationSet.m_Bones.empty() ||
		0 == m_iSkeletonHash ||
		m_iSkeletonHash != animationSet.m_iSkeletonHash ||
		m_Bones.size() != animationSet.m_Bones.size())
	{
		return E_FAIL;
	}

	for (const auto& pIncoming : animationSet.m_Animations)
	{
		for (const auto& pExisting : m_Animations)
		{
			if (pExisting->Compare_Name(pIncoming->Get_Name()))
				return E_FAIL;
		}
	}

	m_Animations.reserve(
		m_Animations.size() + animationSet.m_Animations.size());
	for (const auto& pIncoming : animationSet.m_Animations)
		m_Animations.push_back(pIncoming->Clone());
	m_iNumAnimations = static_cast<uint32_t>(m_Animations.size());
	return S_OK;
}

unique_ptr<CModel> CModel::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix)
{
    auto pInstance = unique_ptr<CModel>(new CModel(pDevice, pContext));

    if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
    {
        OutputDebugStringA("[CModel] Prototype creation failed.\n");
        return nullptr;
    }

    return pInstance;
}

unique_ptr<CModel> CModel::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const MODEL eType,
	const MODEL_ASSET_LOAD_DESC& loadDesc,
	fmatrix_t PreTransformMatrix)
{
	auto pInstance = unique_ptr<CModel>(
		new CModel(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype(
		eType, loadDesc, PreTransformMatrix)))
	{
		OutputDebugStringA("[CModel] Binary prototype creation failed.\n");
		return nullptr;
	}
	return pInstance;
}


shared_ptr<CPrototype> CModel::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CPrototype>(new CModel(*this));

    if (FAILED(pInstance->Initialize(pArg)))
    {
        OutputDebugStringA("[CModel] Clone failed.\n");
        return nullptr;
    }

    return pInstance;
}

void CModel::Free()
{
    if (false == m_isCloned)
        m_Importer.FreeScene();    
}
