#include "Bounding.h"

CBounding::CBounding(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
{
}

CBounding::~CBounding()
{
}

HRESULT CBounding::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
    return S_OK;
}
