#pragma once

#include <memory>

//장치·Context 참조와 월드 상자·색·행렬 타입을 사용한다.
#include "Engine_Defines.h"

//DirectXTK 구현은 Engine CPP에서만 포함한다.
namespace DirectX
{
    inline namespace DX11
    {
        template<typename TVertex>
        class PrimitiveBatch;

        struct VertexPositionColor;
        class BasicEffect;
    }
}

NS_BEGIN(Engine)

//월드 방향성 상자를 선으로 그린다. 표시 여부와 색의 의미는 호출자가 정한다.
class ENGINE_DLL CWireBoxRenderer final
{
private:
    explicit CWireBoxRenderer(ComPtr<ID3D11DeviceContext> pContext);
    CWireBoxRenderer(const CWireBoxRenderer&) = delete;
    CWireBoxRenderer& operator=(const CWireBoxRenderer&) = delete;

public:
    //DirectXTK 자원의 완전한 타입이 있는 CPP에서 정리한다.
    ~CWireBoxRenderer();

public:
    //원형과 복제본이 같은 렌더러를 공유할 수 있다. 준비 실패는 nullptr다.
    static std::shared_ptr<CWireBoxRenderer> Create(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);

    //한 렌더 스레드에서 순서대로 호출한다. 입력은 호출 중만 읽고 보관하지 않는다.
    HRESULT Render(const BoundingOrientedBox& bounds, const float4_t& color,
        const float4x4_t& view, const float4x4_t& projection);

private:
    HRESULT Initialize(ComPtr<ID3D11Device> pDevice);

private:
    //마지막 공유 참조가 사라질 때 자원을 함께 정리한다. Context는 COM 참조를 보유한다.
    ComPtr<ID3D11DeviceContext> m_pContext;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_pBatch;
    std::unique_ptr<DirectX::BasicEffect> m_pEffect;
    ComPtr<ID3D11InputLayout> m_pInputLayout;
};

NS_END
