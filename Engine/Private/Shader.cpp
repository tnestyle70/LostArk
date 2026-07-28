#include "Shader.h"

CShader::CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CShader::~CShader()
{
}

HRESULT CShader::Initialize_Prototype(const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements)
{
	/* 쉐이더파일을 컴파일하고 ID3DX11Effect객체를 생성한다. */
	uint32_t		iFlag = {};

#ifdef _DEBUG
	iFlag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	iFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

	if (FAILED(D3DX11CompileEffectFromFile(pShaderFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iFlag, 0, m_pDevice.Get(), &m_pEffect, nullptr)))
		return E_FAIL;

	ComPtr<ID3DX11EffectTechnique>		pTechnique = m_pEffect->GetTechniqueByIndex(0);
	if (nullptr == pTechnique)
		return E_FAIL;

	D3DX11_TECHNIQUE_DESC	TechniqueDesc{};
	pTechnique->GetDesc(&TechniqueDesc);

	m_iNumPasses = TechniqueDesc.Passes;

	m_InputLayouts.reserve(m_iNumPasses);

	for (size_t i = 0; i < m_iNumPasses; i++)
	{

		ComPtr<ID3D11InputLayout>	pInputLayout = { nullptr };

		ComPtr<ID3DX11EffectPass> pPass = pTechnique->GetPassByIndex(i);
		if (nullptr == pPass)
			return E_FAIL;

		D3DX11_PASS_DESC		PassDesc{};
		pPass->GetDesc(&PassDesc);

		if (FAILED(m_pDevice->CreateInputLayout(pElements, iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
			return E_FAIL;

		m_InputLayouts.push_back(pInputLayout);		
	}

	return S_OK;
}

HRESULT CShader::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CShader::Bind_RawValue(const char_t* pConstantName, const void* pData, uint32_t iLength)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	return pVariable->SetRawValue(pData, 0, iLength);	
}

HRESULT CShader::Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectMatrixVariable>		pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrix(reinterpret_cast<const float_t*>(pMatrix));	
}

HRESULT CShader::Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectMatrixVariable>		pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrixArray(reinterpret_cast<const float_t*>(pMatrices), 0, iNumMatrices);
}

HRESULT CShader::Bind_Texture(const char_t* pConstantName, ComPtr<ID3D11ShaderResourceView> pSRV)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectShaderResourceVariable>	pSRVVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVVariable)
		return E_FAIL;

	return pSRVVariable->SetResource(pSRV.Get());	
}

HRESULT CShader::Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectShaderResourceVariable>	pSRVVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVVariable)
		return E_FAIL;

	return pSRVVariable->SetResourceArray(ppSRV, 0, iNumSRVs);
}



HRESULT CShader::Begin(uint32_t iPassIndex)
{
	if (iPassIndex >= m_iNumPasses)
		return E_FAIL;

	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex].Get());

	return m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, m_pContext.Get());	
}

unique_ptr<CShader> CShader::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements)
{
	auto pInstance = unique_ptr<CShader>(new CShader(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pShaderFilePath, pElements, iNumElements)))
	{
		MSG_BOX("Failed to Created : CShader");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CShader::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CShader>(new CShader(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CShader");
		return nullptr;
	}

	return pInstance;
}
