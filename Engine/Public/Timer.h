#pragma once

// 고해상도 Win32 카운터의 LARGE_INTEGER/HRESULT, 초 단위 f32_t와 단독 소유 포인터를 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// QueryPerformanceCounter의 차분을 초 단위 frame delta로 바꾸는 개별 시계다. Timer_Manager가 소유한다.
class CTimer final
{
private:
	CTimer();
public:
	~CTimer();

public:
	// 마지막 Update_Timer의 결과만 읽는다. 조회 자체는 시계를 갱신하지 않는다.
	f32_t Get_TimeDelta() const
	{
		return m_fTimeDelta;
	}

public:
	// Create가 기준 카운터와 주파수를 읽는다. 현재 Win32 반환값은 검사하지 않고 S_OK를 반환한다.
	HRESULT	Initialize();
	// Timer_Manager가 측정 시점을 전진시킨다. 다음 Get_TimeDelta는 이 호출 사이의 경과 초를 반환한다.
	// 같은 시계의 갱신과 조회를 위한 내부 잠금은 없으므로 호출자가 접근 순서를 보장한다.
	void Update_Timer();

private:
	// FrameTime/LastTime은 이번·직전 QPC 값, FixTime은 주파수를 다시 읽은 기준 시점이다.
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_FixTime = {};
	LARGE_INTEGER		m_LastTime = {};
	// CpuTick은 초당 QPC tick 수다. CPU 코어의 클럭 주파수를 뜻하지 않는다.
	LARGE_INTEGER		m_CpuTick = {};
	f32_t				m_fTimeDelta = {};

public:
	// 초기화를 끝낸 시계의 소유권을 넘긴다. 메모리 할당 예외를 nullptr로 바꾸는 catch는 없다.
	static unique_ptr<CTimer> Create();
};

NS_END