#include "Model.h"

#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/WModelDecoder.h"
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
    , m_iCurrentAnimIndex { Prototype.m_iCurrentAnimIndex }
    , m_iNumAnimations { Prototype.m_iNumAnimations}
    // , m_Animations { Prototype.m_Animations }
    , m_isAnimLoop { Prototype.m_isAnimLoop }
	, m_bHasLocalBounds { Prototype.m_bHasLocalBounds }
	, m_vLocalBoundsMin { Prototype.m_vLocalBoundsMin }
	, m_vLocalBoundsMax { Prototype.m_vLocalBoundsMax }
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

bool_t CModel::Set_Animation(const char_t* pAnimationName, bool_t isLoop)
{
    if (nullptr == pAnimationName)
        return false;

    for (uint32_t i = 0; i < m_Animations.size(); ++i)
    {
        if (!m_Animations[i]->Compare_Name(pAnimationName))
            continue;

        m_iCurrentAnimIndex = i;
        m_isAnimLoop = isLoop;
        return true;
    }
    return false;
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
        if (L"LostArk" == current.filename())
        {
            desc.assetRoot = current;
            break;
        }
        if (current == current.root_path())
            break;
    }

    MODEL_ASSET_DATA asset{};
    MODEL_DECODE_REPORT report{};
    if (!CWModelDecoder{}.Decode(desc, asset, report))
        return E_FAIL;

    if ((MODEL::ANIM == m_eType) != asset.hasSkeleton)
        return E_FAIL;

    if (FAILED(Ready_Bones(asset)) ||
        FAILED(Ready_Meshes(asset)) ||
        FAILED(Ready_Materials(asset)) ||
        FAILED(Ready_Animations(asset)))
        return E_FAIL;

    for (auto& pBone : m_Bones)
        pBone->Update_CombinedTransformationMatrix(
            m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));

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
			for (const VTXMESH& vertex : mesh.vertices)
			{
				Include_LocalPosition(XMVector3TransformCoord(
					XMLoadFloat3(&vertex.vPosition), XMLoadFloat4x4(&m_PreTransformMatrix)));
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

unique_ptr<CModel> CModel::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix)
{
    auto pInstance = unique_ptr<CModel>(new CModel(pDevice, pContext));

    if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
    {
        MSG_BOX("Failed to Created : CModel");
        return nullptr;
    }

    return pInstance;
}


shared_ptr<CPrototype> CModel::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CPrototype>(new CModel(*this));

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CModel");
        return nullptr;
    }

    return pInstance;
}

void CModel::Free()
{
    if (false == m_isCloned)
        m_Importer.FreeScene();    
}
