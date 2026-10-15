#pragma once

// DirectX 벡터·평면, POINT/LINE 순서와 엔진 타입·COM/shared_ptr 선언을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// CNavigation의 기존 삼각형 모드가 공유하는 한 셀이다. 정점·평면·변별 이웃 index를 값으로 보관한다.
// 이웃은 CCell 포인터를 소유하지 않으며, index의 범위와 데이터의 정점 순서는 Navigation이 맞춘다.
// 초기 구성 이후 질의에 사용한다. 내부 잠금이 없으므로 구성과 조회를 동시에 수행하지 않는다.
class CCell final 
{
private:
	CCell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CCell();

public:
	// 세 정점의 산술 평균을 위치 벡터(w=1)로 반환한다. Navigation의 초기 배치가 소비한다.
	vector_t Get_Center()
	{
		vector_t		vCenter = {};
		for (size_t i = 0; i < 3; i++)
			vCenter += XMLoadFloat3(&m_vPoints[i]);
		
		return XMVectorSetW(vCenter / 3.f, 1.f);
	}

	// 복사한 정점 값을 반환한다. 유효한 POINT만 전달해야 하며 이 함수는 범위를 검사하지 않는다.
	vector_t Get_Point(POINT ePoint)
	{
		return XMLoadFloat3(&m_vPoints[ETOUI(ePoint)]);
	}

	// 공유 변을 찾은 Navigation이 호출한다. non-null 이웃의 index만 복사하고 참조를 보관하지 않는다.
	void Set_Neighbor(LINE eLine, shared_ptr<CCell> pNeighbor)
	{
		m_iNeighborIndices[ETOUI(eLine)] = pNeighbor->m_iIndex;
	}
	// 파일 로더가 검증한 AB/BC/CA 세 index를 복사한다. 입력 배열 수명은 호출 동안만 필요하다.
	void Set_Neighbor(int32_t* pNeighborIndices)
	{
		memcpy(m_iNeighborIndices, pNeighborIndices, sizeof(int32_t) * 3);
	}

public:
	// Create가 세 정점을 복사하고 평면·변 법선을 만든다. Debug 선 버퍼 실패는 E_FAIL이다.
	// 정점 유효성·퇴화 삼각형은 검사하지 않으며, 실패 전에 복사한 멤버는 되돌리지 않는다.
	HRESULT Initialize(const float3_t* pPoints, int32_t iIndex);
	// Navigation이 후보 위치의 XZ 포함 여부를 질의한다. 처음 벗어난 변의 이웃 index를 출력한다.
	// 내부이면 true이고 출력은 유지된다. 외부일 때 쓸 수 있는 non-null 출력 포인터가 필요하다.
	bool_t isIn(fvector_t vResultPos, int32_t* pNeighborIndex);

	// 두 입력이 이 셀의 서로 다른 정점 슬롯 값과 정확히 일치하는지 검사한다. 허용 오차 비교가 아니다.
	bool_t Compare_Points(fvector_t vSourPoint, fvector_t vDestPoint);
	// 같은 좌표계의 XZ를 평면식에 넣어 Y를 구한다. m_vPlane.y가 0인 수직/퇴화 평면은 방어하지 않는다.
	f32_t Compute_Height(fvector_t vPosition);

#ifdef _DEBUG
public:
	// Debug Navigation이 설정한 shader 상태에서 셀 선 버퍼를 그린다. bind/draw 실패를 반환한다.
	HRESULT Render();
#endif

private:
	// COM 참조를 보유하므로 Debug 버퍼를 만들고 사용하는 동안 device/context 수명을 유지한다.
	ComPtr<ID3D11Device>				m_pDevice = {};
	ComPtr<ID3D11DeviceContext>			m_pContext = {};

	// 정점·법선·평면은 동일 좌표계다. 법선은 Y=0인 변 바깥쪽 판정용이며 정점 winding에 의존한다.
	float3_t				m_vPoints[ETOUI(POINT::END)] = {};
	int32_t					m_iIndex = { -1 };	

	float3_t				m_vNormals[ETOUI(LINE::END)] = {};

	int32_t					m_iNeighborIndices[ETOUI(LINE::END)] = { -1, -1, -1 };

	float4_t				m_vPlane = {};

#ifdef _DEBUG
private:
	shared_ptr<class CVIBuffer_Cell>			m_pVIBuffer = { nullptr };

#endif

public:
	// Navigation이 공유할 셀을 만든다. Initialize 실패는 메시지 표시 후 nullptr로 반환하고 임시 객체를 해제한다.
	static shared_ptr<CCell> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const float3_t* pPoints, int32_t iIndex);
};

NS_END