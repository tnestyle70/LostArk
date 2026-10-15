// navigation cell의 삼각형 윤곽선 버퍼를 구현한다.
#include "VIBuffer_Cell.h"
// float3 위치와 동일한 VTXPOS layout으로 입력 점을 GPU에 복사한다.
#include "Engine_VertexTypes.h"

// 장치/context의 소유 참조를 기반 CVIBuffer로 전달한다.
CVIBuffer_Cell::CVIBuffer_Cell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Cell::~CVIBuffer_Cell()
{
}

// Create가 유효한 세 점을 전달한다. 초기화 중 실패는 부분 자원을 남기며 factory가 임시 객체를 폐기해 정리한다.
HRESULT CVIBuffer_Cell::Initialize_Prototype(const float3_t* pPoints)
{
#pragma region PUBLIC_DATA
	// 정점 세 개와 마지막 시작점으로 돌아오는 index 네 개를 LINESTRIP으로 그린다.
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumVertices = 3;
	m_iIndexStride = 2;
	m_iNumIndices = 4;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		 
#pragma endregion

#pragma region VERTEX_BUFFER
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;	


	// unique_ptr<VTXTEX[]> pVertices = unique_ptr<VTXTEX[]>(new VTXTEX[m_iNumVertices]());
	unique_ptr<VTXPOS[]> pVertices = make_unique<VTXPOS[]>(m_iNumVertices);
		
	// GPU 생성 호출이 끝날 때까지만 필요한 CPU 사본이다. 입력 pPoints를 멤버에 보관하지 않는다.
	memcpy(pVertices.get(), pPoints, sizeof(VTXPOS) * 3);

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;

#pragma endregion

#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint16_t[]>		pIndices = make_unique<uint16_t[]>(m_iNumIndices);
	// 0→1→2→0 순서로 닫힌 삼각형 경계를 만든다. 면을 채우는 triangle list가 아니다.
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 0;
	
	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

// clone의 추가 초기화는 없다. 인자를 읽거나 buffer를 다시 만들지 않는다.
HRESULT CVIBuffer_Cell::Initialize(void* pArg)
{
	return S_OK;
}

// 완성한 buffer만 CCell의 소유자에게 넘기며 실패한 임시 자원은 지역 unique_ptr가 정리한다.
unique_ptr<CVIBuffer_Cell> CVIBuffer_Cell::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const float3_t* pPoints)
{
	auto pInstance = unique_ptr<CVIBuffer_Cell>(new CVIBuffer_Cell(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pPoints)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Cell");
		return nullptr;
	}

	return pInstance;
}


// 기반 클래스의 COM 참조를 복사해 GPU 자원을 공유한다. 이 타입의 clone은 점 데이터를 변경하지 않는다.
shared_ptr<CPrototype> CVIBuffer_Cell::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CVIBuffer_Cell>(new CVIBuffer_Cell(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVIBuffer_Cell");
		return nullptr;
	}

	return pInstance;
}

