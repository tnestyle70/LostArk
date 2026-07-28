#include "Model.h"

#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "Material.h"
#include "Animation.h"

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
    , m_iNumAnimations { Prototype.m_iNumAnimations}
    // , m_Animations { Prototype.m_Animations }
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

HRESULT CModel::Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix)
{
    uint32_t  iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };

    if (MODEL::NONANIM == eType)
        iFlag |= aiProcess_PreTransformVertices;

    m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);
    if (nullptr == m_pAIScene)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

    m_eType = eType;

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

bool_t CModel::Play_Animation(f32_t fTimeDelta)
{
    bool_t      isFinished = { false };
    /* 내가 로드한 애니메이션 중, 
    현재 취해야하는 애니메이션의 포즈뼈들의 m_TransformationMatrix를 갱신해준다. */
    isFinished = m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrix(fTimeDelta, m_Bones, m_isAnimLoop);

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

HRESULT CModel::Ready_Meshes()
{
    m_iNumMeshes = m_pAIScene->mNumMeshes;

    for (size_t i = 0; i < m_iNumMeshes; i++)
    {
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
