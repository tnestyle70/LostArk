#include "Prototype.h"

CPrototype::CPrototype(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_isCloned { false }
{
}

CPrototype::CPrototype(const CPrototype& Prototype)
	: m_pDevice{ Prototype.m_pDevice }
	, m_pContext{ Prototype.m_pContext }
	, m_isCloned{ true }
{
}


CPrototype::~CPrototype()
{
}
