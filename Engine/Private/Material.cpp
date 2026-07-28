#include "Material.h"
#include "Shader.h"

CMaterial::CMaterial(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{	
}

CMaterial::~CMaterial()
{
}

HRESULT CMaterial::Initialize(const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	char_t			szDrive[MAX_PATH] = {};
	char_t			szDir[MAX_PATH] = {};

	_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDir, MAX_PATH, nullptr, 0, nullptr, 0);

	for (uint32_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
	{
		uint32_t		iNumTextures = pAIMaterial->GetTextureCount(static_cast<aiTextureType>(i));

		m_Textures[i].reserve(iNumTextures);

		for (uint32_t j = 0; j < iNumTextures; j++)
		{
			ComPtr<ID3D11ShaderResourceView>		pSRV = { nullptr };

			aiString			strTextureFilePath = {};

			if (FAILED(pAIMaterial->GetTexture(static_cast<aiTextureType>(i), j, &strTextureFilePath)))
				return E_FAIL;

			
			char_t			szFileName[MAX_PATH] = {};
			char_t			szEXT[MAX_PATH] = {};

			_splitpath_s(strTextureFilePath.C_Str(), nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szEXT, MAX_PATH);

			char_t			szFullPath[MAX_PATH] = {};
			strcpy_s(szFullPath, szDrive);
			strcat_s(szFullPath, szDir);
			strcat_s(szFullPath, szFileName);
			strcat_s(szFullPath, szEXT);

			tchar_t			szTextureFilePath[MAX_PATH] = {};

			MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szTextureFilePath, MAX_PATH);

			HRESULT		hr = {};

			if (false == strcmp(szEXT, ".dds"))
				hr = CreateDDSTextureFromFile(m_pDevice.Get(), szTextureFilePath, nullptr, &pSRV);
			else if (false == strcmp(szEXT, ".tga"))
				hr = E_FAIL;
			else
				hr = CreateWICTextureFromFile(m_pDevice.Get(), szTextureFilePath, nullptr, &pSRV);
			
			m_Textures[i].push_back(pSRV);
		}
	}

	return S_OK;
}

HRESULT CMaterial::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, aiTextureType eType, uint32_t iTextureIndex)
{
	if (eType >= AI_TEXTURE_TYPE_MAX ||
		iTextureIndex >= m_Textures[eType].size())
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[eType][iTextureIndex]);	
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pAIMaterial, pModelFilePath)))
	{
		MSG_BOX("Failed to Created : CMaterial");
		return nullptr;
	}

	return pInstance;
}
