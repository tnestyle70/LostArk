// 렌더 target 등록·MRT 순서·pass 출력 복구를 구현한다.
#include "Target_Manager.h"

// 태그 map과 MRT가 공유하는 GPU 자원 객체의 생성/resize/바인딩을 호출한다.
#include "RenderTarget.h"

// 장치/context를 강한 참조로 보관한다. 아직 target이나 MRT는 생성하지 않는다.
CTarget_Manager::CTarget_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

// 공유 target map/list와 저장 출력 참조는 멤버 소멸로 반환한다.
CTarget_Manager::~CTarget_Manager()
{
}

// GameInstance/Renderer의 resize 준비 경로가 호출한다. 현재 객체와 후보를 쌍으로 보관해 하나의 실패도 기존 화면 크기에 반영되지 않게 한다.
HRESULT CTarget_Manager::Stage_Resize(const vector<RESIZE_REQUEST>& requests,
    RESIZE_STATE& output) const
{
    RESIZE_STATE staged;
    staged.Targets.reserve(requests.size());

    for (const auto& request : requests)
    {
        // 없는 태그와 같은 객체의 중복 요청을 먼저 거부한 뒤 새 크기 자원을 준비한다.
        auto current = Find_RenderTarget(request.TargetTag);
        if (!current || any_of(staged.Targets.begin(), staged.Targets.end(),
            [&current](const auto& item) { return item.first == current; }))
            return E_INVALIDARG;

        shared_ptr<CRenderTarget> candidate;
        const HRESULT result = current->Stage_Resize(request.Width, request.Height, candidate);
        if (FAILED(result))
            return result;

        staged.Targets.emplace_back(std::move(current), std::move(candidate));
    }

    // 모든 GPU 후보가 준비된 경우에만 호출자의 후보 묶음을 교체한다.
    output = std::move(staged);
    return S_OK;
}

// 상위 resize 성공 경계가 한 번 호출한다. 목록의 객체 identity는 유지하고 각 객체의 GPU 자원만 교환한다.
void CTarget_Manager::Commit_Resize(RESIZE_STATE& staged,
    f32_t viewportDeltaX, f32_t viewportDeltaY) noexcept
{
    for (auto& target : staged.Targets)
        target.first->Commit_Resize(*target.second, viewportDeltaX, viewportDeltaY);
}

// 종료/resize 또는 pass 복구 뒤 저장한 출력 COM 참조를 내려놓는다. context 자체의 참조는 별도 해제가 필요하다.
void CTarget_Manager::Release_OutputReferences() noexcept
{
    m_pBackBufferRTV.Reset();
    m_pOriginalDSV.Reset();
}

// 중복과 생성 실패를 확인한 뒤 등록한다. 기존 태그의 자원을 덮어쓰지 않는다.
HRESULT CTarget_Manager::Add_RenderTarget(
	const wstring_t& strTargetTag,
	uint32_t iWidth, uint32_t iHeight,
	DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
	if (nullptr != Find_RenderTarget(strTargetTag))
		return E_FAIL;

	auto		pRenderTarget = CRenderTarget::Create(m_pDevice, m_pContext, iWidth, iHeight, ePixelFormat, vClearColor);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	m_RenderTargets.emplace(strTargetTag, pRenderTarget);

	return S_OK;
}

// 등록된 target을 MRT의 다음 출력 슬롯으로 추가한다. 그룹이 없으면 list를 새로 만든다.
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

// 상위 렌더 pass가 순차 호출한다. 입력/출력 충돌을 피하도록 PS SRV를 먼저 비우고, 기존 RTV0/DSV를 복구용으로 보관한다.
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

	// 등록 순서가 출력 슬롯 순서다. 현재 고정 배열의 상한은 8이며 Add_MRT 소비자가 개수와 자원 호환성을 보장한다.
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

// 저장된 첫 색상 target과 depth만 다시 바인딩하고 저장 참조를 해제한다. Begin 이전의 다른 MRT 슬롯은 복원하지 않는다.
HRESULT CTarget_Manager::End_MRT()
{
	ID3D11RenderTargetView* RenderTargets[8] = { m_pBackBufferRTV.Get(), };

	m_pContext->OMSetRenderTargets(8, RenderTargets, m_pOriginalDSV.Get());
	// 저장 출력 참조는 이번 pass 복구까지만 유지하고 반환한다.
	Release_OutputReferences();

	return S_OK;
}

// 그림자 등 색상 출력이 필요 없는 pass다. DSV 부재는 상태 변경 전 실패하고, 유효하면 depth만 clear한다.
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

// MRT와 같은 단일 복구 슬롯을 사용하므로 동일한 종료 경로에 위임한다.
HRESULT CTarget_Manager::End_DepthOnly()
{
	return End_MRT();
}

// 태그 조회 성공 후 읽기 view 바인딩을 해당 target에 맡긴다.
HRESULT CTarget_Manager::Bind_SRV(
	const wstring_t& strTargetTag,
	shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	auto		pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Bind_SRV(pShader, pConstantName);
}

// 현재 view를 외부 소비자에게 강한 참조로 전달한다. 존재하지 않는 태그는 빈 참조다.
ComPtr<ID3D11ShaderResourceView> CTarget_Manager::Get_SRV(
	const wstring_t& strTargetTag) const
{
	auto pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return {};

	return pRenderTarget->Get_SRV();
}

// 이름으로 찾은 target을 원본으로 복사한다. 목적지 텍스처 검증은 이 관리자에서 추가하지 않는다.
HRESULT CTarget_Manager::Copy_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D)
{
	auto		pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Copy_Resource(pTexture2D);
}

#ifdef _DEBUG

// Debug 화면 배치를 target에 저장한다. GPU 자원 크기나 실제 render 옵션은 변경하지 않는다.
HRESULT CTarget_Manager::Ready_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY)
{
	auto		pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Ready_DebugDesc(fX, fY, fSizeX, fSizeY);	
}

// 등록된 Debug MRT가 전제다. 현재 개별 Render의 반환값은 버리므로 S_OK를 각 draw의 성공 검증으로 사용하지 않는다.
HRESULT CTarget_Manager::Render_MRT(
	const wstring_t& strMRTTag,
	shared_ptr<class CShader> pShader,
	shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	auto		pMRTList = Find_MRT(strMRTTag);

	for (auto& pRenderTarget : *pMRTList)
	{
		pRenderTarget->Render(pShader, pVIBuffer);
	}

	return S_OK;
}

#endif

// map의 소유 참조를 복사한다. 조회 자체로 target을 만들지 않는다.
shared_ptr<class CRenderTarget> CTarget_Manager::Find_RenderTarget(
	const wstring_t& strTargetTag) const
{
	auto		iter = m_RenderTargets.find(strTargetTag);
	if (iter == m_RenderTargets.end())
		return nullptr;

	return iter->second;	
}

// 관리자 내부 list를 차용한다. 호출자는 삭제/동시 편집 없이 사용해야 한다.
list<shared_ptr<class CRenderTarget>>* CTarget_Manager::Find_MRT(const wstring_t& strMRTTag)
{
	auto	iter = m_MRTs.find(strMRTTag);
	if (iter == m_MRTs.end())
		return nullptr;

	return &iter->second;
}

// GameInstance가 소유할 빈 관리자를 만든다. 장치/context 참조 외에는 초기화 실패 단계를 갖지 않는다.
unique_ptr<CTarget_Manager> CTarget_Manager::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	return unique_ptr<CTarget_Manager>(new CTarget_Manager(pDevice, pContext));
}
