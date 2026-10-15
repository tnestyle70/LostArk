#pragma once

// D3D11 기기·즉시 문맥·texture의 COM 소유 도구와 float4/요청 ID 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 Target_PickPos의 한 픽셀 GPU 읽기 경로다. world 위치를 얻으며 화면 UI hit test와 구분한다.
// device/context와 staging texture는 ComPtr로 소유하고, 입력 HWND는 차용한다.
class CPicking final
{
private:
	CPicking(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CPicking();

public:
	// Create가 즉시 문맥과 소유 스레드를 기록하고 동기/비동기용 staging texture를 각각 준비한다.
	HRESULT Initialize(HWND hWnd);
	// 현재 커서 위치를 동기 읽는다. GPU copy가 끝날 때 Map이 기다릴 수 있으며 실패하면 vOut을 보존한다.
	bool_t Picking(float4_t& vOut);
	// 현재 픽셀의 비동기 copy를 요청해 ID를 반환한다. 0은 실패다. 새 요청은 이전 미소비 요청을 대체한다.
	uint64_t Request_Picking();
	// S_OK만 좌표 완료다. S_FALSE는 GPU 처리 중이므로 같은 ID로 재시도하고, 실패는 상태에 따라 요청을 폐기한다.
	// 모든 요청/조회/취소는 Initialize를 수행한 immediate-context 소유 스레드에서 해야 한다.
	HRESULT Poll_Picking(uint64_t requestId, float4_t& vOut);
	// 일치하는 ID의 결과 수용을 취소한다. 이미 제출한 GPU copy 자체를 취소하는 함수는 아니다.
	void Cancel_Picking(uint64_t requestId);

private:
	// 동기 Picking만 호출한다. 입력 texture와 Profiler는 호출 동안 차용하며 Map 성공 뒤에는 결과 유효성과 관계없이 Unmap한다.
	bool_t Read_Pixel(ID3D11Texture2D* pSource, uint32_t x, uint32_t y,
		float4_t& vOut, class CProfiler* pProfiler);

private:
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>			m_pContext = { nullptr };

	ComPtr<ID3D11Texture2D>				m_pTexture2D = { nullptr };
	ComPtr<ID3D11Texture2D> m_pAsyncTexture;
	// 동시에 보관하는 미소비 요청은 하나다. 0은 요청 없음이며 새 번호 발급 시 건너뛴다.
	uint64_t m_iNextRequestId = 0u;
	uint64_t m_iPendingRequestId = 0u;
	HWND m_hWnd = {};

	// 즉시 문맥을 사용하는 스레드 ID다. 내부 잠금 대신 모든 읽기/요청/조회/취소가 이 스레드인지 검사한다.
	DWORD m_iOwnerThreadId = 0u;

public:
	// GameInstance가 소유권을 받는다. texture 생성 중 실패한 후보도 COM 참조와 함께 정리한다.
	static unique_ptr<CPicking> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, HWND hWnd);
};

NS_END