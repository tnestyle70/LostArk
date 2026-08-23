#include "Target_Manager.h"

#include "RenderTarget.h"

CTarget_Manager::CTarget_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CTarget_Manager::~CTarget_Manager()
{
}

HRESULT CTarget_Manager::Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
	if (nullptr != Find_RenderTarget(strTargetTag))
		return E_FAIL;

	auto		pRenderTarget = CRenderTarget::Create(m_pDevice, m_pContext, iWidth, iHeight, ePixelFormat, vClearColor);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	m_RenderTargets.emplace(strTargetTag, pRenderTarget);

	return S_OK;
}

HRESULT CTarget_Manager::Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag)
{
	auto	pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	auto	pMRTList = Find_MRT(strMRTTag);

	if (nullptr == pMRTList)
	{
		list<shared_ptr<CRenderTarget>>		MRTList;

		MRTList.push_back(pRenderTarget);

		m_MRTs.emplace(strMRTTag, MRTList);
	}
	else
	{
		pMRTList->push_back(pRenderTarget);
	}

	return S_OK;
}

HRESULT CTarget_Manager::Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV)
{

	ID3D11ShaderResourceView* pSRV[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {
		nullptr
	};

	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pSRV);


	auto		pMRTList = Find_MRT(strMRTTag);
	if (nullptr == pMRTList)
		return E_FAIL;

	m_pContext->OMGetRenderTargets(1, &m_pBackBufferRTV, &m_pOriginalDSV);

	ID3D11RenderTargetView* RenderTargets[8] = {};

	uint32_t		iNumRenderTargets = {};

	for (auto& pRenderTarget : *pMRTList)
	{
		pRenderTarget->Clear();
		RenderTargets[iNumRenderTargets++] = pRenderTarget->Get_RTV().Get();
	}

	if (nullptr != pDSV)
		m_pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	m_pContext->OMSetRenderTargets(iNumRenderTargets, RenderTargets, nullptr == pDSV ? m_pOriginalDSV.Get() : pDSV.Get());

	return S_OK;
}

HRESULT CTarget_Manager::End_MRT()
{


	ID3D11RenderTargetView* RenderTargets[8] = { m_pBackBufferRTV.Get(), };

	m_pContext->OMSetRenderTargets(8, RenderTargets, m_pOriginalDSV.Get());

	return S_OK;
}

HRESULT CTarget_Manager::Begin_DepthOnly(
	ComPtr<ID3D11DepthStencilView> pDSV)
{
	if (nullptr == pDSV)
		return E_INVALIDARG;

	ID3D11ShaderResourceView* pSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pSRVs);

	m_pContext->OMGetRenderTargets(
		1, &m_pBackBufferRTV, &m_pOriginalDSV);
	m_pContext->ClearDepthStencilView(
		pDSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
	m_pContext->OMSetRenderTargets(0, nullptr, pDSV.Get());

	return S_OK;
}

HRESULT CTarget_Manager::End_DepthOnly()
{
	return End_MRT();
}

HRESULT CTarget_Manager::Bind_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	auto		pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Bind_SRV(pShader, pConstantName);
}

ComPtr<ID3D11ShaderResourceView> CTarget_Manager::Get_SRV(
	const wstring_t& strTargetTag) const
{
	auto pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return {};

	return pRenderTarget->Get_SRV();
}

HRESULT CTarget_Manager::Copy_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D)
{
	auto		pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Copy_Resource(pTexture2D);
}

#ifdef _DEBUG

HRESULT CTarget_Manager::Ready_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY)
{
	auto		pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Ready_DebugDesc(fX, fY, fSizeX, fSizeY);	
}

HRESULT CTarget_Manager::Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	auto		pMRTList = Find_MRT(strMRTTag);

	for (auto& pRenderTarget : *pMRTList)
	{
		pRenderTarget->Render(pShader, pVIBuffer);
	}

	return S_OK;
}

#endif

shared_ptr<class CRenderTarget> CTarget_Manager::Find_RenderTarget(
	const wstring_t& strTargetTag) const
{
	auto		iter = m_RenderTargets.find(strTargetTag);
	if (iter == m_RenderTargets.end())
		return nullptr;

	return iter->second;	
}

list<shared_ptr<class CRenderTarget>>* CTarget_Manager::Find_MRT(const wstring_t& strMRTTag)
{
	auto	iter = m_MRTs.find(strMRTTag);
	if (iter == m_MRTs.end())
		return nullptr;

	return &iter->second;
}

unique_ptr<CTarget_Manager> CTarget_Manager::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	return unique_ptr<CTarget_Manager>(new CTarget_Manager(pDevice, pContext));
}
