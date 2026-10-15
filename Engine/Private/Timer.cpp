// Timer_Manager가 소유하는 시계의 Win32 측정과 초 단위 경과 시간 계산을 구현한다.
#include "Timer.h"

// 기준 카운터와 결과를 0으로 시작한다. 실제 측정 기준은 Create가 호출하는 Initialize에서 준비한다.
CTimer::CTimer()
	: m_fTimeDelta(0.f)
{
	ZeroMemory(&m_FixTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_LastTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_FrameTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_CpuTick, sizeof(LARGE_INTEGER));
}

CTimer::~CTimer()
{
}

// Create가 기준 카운터와 초당 tick 수를 준비한다. 현재 구현은 Win32 조회의 실패 반환값을 검사하지 않는다.
HRESULT CTimer::Initialize()
{
	QueryPerformanceCounter(&m_FrameTime);
	QueryPerformanceCounter(&m_LastTime);
	QueryPerformanceCounter(&m_FixTime);

	QueryPerformanceFrequency(&m_CpuTick); // cpu tick 값을 얻어오는 함수
	// 보충: QueryPerformanceFrequency는 고해상도 카운터의 초당 증가량이다. 앞서 읽은 Counter 값의 차이를 이 값으로 나누어 경과 초를 구한다.

	return S_OK;
}


// Client의 프레임 루프가 Timer_Manager를 통해 호출한다. 실제 측정 간격을 초로 돌려주며 FPS 제한은 하지 않는다.
void CTimer::Update_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);

	// 이 조건은 주파수 재조회 주기일 뿐이다. delta는 매 호출마다 아래에서 계산한다.
	if (m_FrameTime.QuadPart - m_FixTime.QuadPart >= m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_FixTime = m_FrameTime;
	}

	// 경과 tick / (tick/초) = 초다. 긴 프레임의 delta를 제한하거나 고정 간격으로 바꾸지 않는다.
	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart) / static_cast<f32_t>(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;
}

// 준비가 끝난 시계만 Manager에 전달한다. 실패한 후보의 소유권은 지역 unique_ptr가 정리한다.
unique_ptr<CTimer> CTimer::Create()
{

	auto pInstance = unique_ptr<CTimer>(new CTimer());

	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}
