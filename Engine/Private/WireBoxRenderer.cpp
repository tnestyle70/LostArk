#include "WireBoxRenderer.h"

#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/Effects.h"

//기존 공용 도형 제출을 사용해 상자의 선분 생성 코드를 중복하지 않는다.
#include "DebugDraw.h"

NS_BEGIN(Engine)

CWireBoxRenderer::CWireBoxRenderer(ComPtr<ID3D11DeviceContext> pContext)
    : m_pContext(pContext)
{
}

CWireBoxRenderer::~CWireBoxRenderer() = default;

HRESULT CWireBoxRenderer::Initialize(ComPtr<ID3D11Device> pDevice)
{
    if (!pDevice || !m_pContext)
    {
        return E_INVALIDARG;
    }

    //위치·색 정점을 모아 그릴 Batch와 기본 행렬 셰이더를 준비한다.
    m_pBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_pContext.Get());
    m_pEffect = std::make_unique<BasicEffect>(pDevice.Get());
    m_pEffect->SetVertexColorEnabled(true);

    //선택한 셰이더가 읽는 정점 형식으로 입력 레이아웃을 만든다.
    const void* pVertexShaderByteCode = nullptr;
    size_t iByteCodeLength = 0;
    m_pEffect->GetVertexShaderBytecode(&pVertexShaderByteCode, &iByteCodeLength);

    return pDevice->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        pVertexShaderByteCode,
        iByteCodeLength,
        m_pInputLayout.GetAddressOf());
}

HRESULT CWireBoxRenderer::Render(const BoundingOrientedBox& bounds, const float4_t& color,
    const float4x4_t& view, const float4x4_t& projection)
{
    if (!m_pContext || !m_pBatch || !m_pEffect || !m_pInputLayout)
    {
        return E_FAIL;
    }

    //상자는 이미 월드 좌표다. 호출자가 넘긴 카메라 행렬만 추가로 적용한다.
    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(XMLoadFloat4x4(&view));
    m_pEffect->SetProjection(XMLoadFloat4x4(&projection));
    m_pContext->IASetInputLayout(m_pInputLayout.Get());
    m_pEffect->Apply(m_pContext.Get());

    //공유 Batch를 이번 호출 안에서만 열고 닫는다. 다음 호출은 앞 호출이 끝난 뒤 실행한다.
    const vector_t vColor = XMLoadFloat4(&color);
    m_pBatch->Begin();
    DX::Draw(m_pBatch.get(), bounds, vColor);
    m_pBatch->End();
    return S_OK;
}

std::shared_ptr<CWireBoxRenderer> CWireBoxRenderer::Create(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    if (!pDevice || !pContext)
    {
        return nullptr;
    }

    //준비가 끝난 객체만 돌려준다. 중간 실패의 자원은 지역 소유자가 정리한다.
    try
    {
        auto pInstance = std::shared_ptr<CWireBoxRenderer>(new CWireBoxRenderer(pContext));
        if (FAILED(pInstance->Initialize(pDevice)))
        {
            return nullptr;
        }

        return pInstance;
    }
    catch (...)
    {
        return nullptr;
    }
}

NS_END
