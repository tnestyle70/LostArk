// 정점 네 개의 사각형 원형과 같은 GPU 자원을 공유하는 clone 생성을 구현한다.
#include "VIBuffer_Rect.h"
// VB stride와 초기 정점 배열에 위치/UV를 갖는 VTXTEX 형식을 사용한다.
#include "Engine_VertexTypes.h"

CVIBuffer_Rect::CVIBuffer_Rect(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Rect::~CVIBuffer_Rect()
{
}

// POSITION/TEXCOORD 정점 4개와 R16_UINT 인덱스 6개를 준비한다. CPU 임시 배열은 CreateBuffer 반환 뒤 필요 없다.
HRESULT CVIBuffer_Rect::Initialize_Prototype()
{
	// 정점·인덱스의 원소 수, byte 크기와 IA 해석 규칙을 먼저 맞춘다. 아래 버퍼 할당과 공통 Bind_Resources가 함께 소비한다.
#pragma region PUBLIC_DATA
	m_iVertexStride = sizeof(VTXTEX);
	m_iNumVertices = 4;
	m_iIndexStride = 2;
	m_iNumIndices = 6;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		 
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
	unique_ptr<VTXTEX[]> pVertices = make_unique<VTXTEX[]>(m_iNumVertices);
		
	// 로컬 중심은 원점, 좌표 범위는 x/y의 -0.5~0.5, z=0이다. UV는 위쪽 y에 v=0을 배치한다.
	pVertices[0].vPosition = float3_t(-0.5f, 0.5f, 0.f);
	pVertices[0].vTexcoord = float2_t(0.f, 0.f);

	pVertices[1].vPosition = float3_t(0.5f, 0.5f, 0.f);
	pVertices[1].vTexcoord = float2_t(1.f, 0.f);

	pVertices[2].vPosition = float3_t(0.5f, -0.5f, 0.f);
	pVertices[2].vTexcoord = float2_t(1.f, 1.f);

	pVertices[3].vPosition = float3_t(-0.5f, -0.5f, 0.f);
	pVertices[3].vTexcoord = float2_t(0.f, 1.f);

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	// D3D11이 초기 데이터를 GPU 버퍼로 복사한다. 실패 시 nullptr 반환 경로에서 부분 자원을 정리한다.
	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;

#pragma endregion

	// 정점 버퍼 준비 뒤 삼각형 인덱스를 만든다. 이 단계의 HRESULT 실패는 앞서 만든 VB를 되돌리지 않으며 Create가 실패 후보 전체를 버린다.
#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint16_t[]>		pIndices = make_unique<uint16_t[]>(m_iNumIndices);
	// 두 삼각형은 대각선 0-2를 공유한다. 이 순서와 실제 rasterizer culling 상태가 앞면 표시를 결정한다.
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

// Clone에서 호출하지만 추가 할당이나 인스턴스별 버퍼 수정은 하지 않는다.
HRESULT CVIBuffer_Rect::Initialize(void* pArg)
{
	return S_OK;
}

// 지역 unique_ptr로 부분 생성 자원도 소유한다. HRESULT 실패는 메시지 표시 후 nullptr로 반환한다.
unique_ptr<CVIBuffer_Rect> CVIBuffer_Rect::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CVIBuffer_Rect>(new CVIBuffer_Rect(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Rect");
		return nullptr;
	}

	return pInstance;
}

// 복사 생성으로 장치와 VB/IB 참조를 공유한다. clone 객체만 별도로 만들며 원본 prototype을 변경하지 않는다.
shared_ptr<CPrototype> CVIBuffer_Rect::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CVIBuffer_Rect>(new CVIBuffer_Rect(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVIBuffer_Rect");
		return nullptr;
	}

	return pInstance;
}

