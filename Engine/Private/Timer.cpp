#include "Timer.h"

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
	int a = 10;
}

HRESULT CTimer::Initialize()
{
	QueryPerformanceCounter(&m_FrameTime);			// 1077
	QueryPerformanceCounter(&m_LastTime);			// 1085
	QueryPerformanceCounter(&m_FixTime);			// 1090

	QueryPerformanceFrequency(&m_CpuTick);		// cpu tick 값을 얻어오는 함수

	return S_OK;
}

void CTimer::Update_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);			// 1500

	if (m_FrameTime.QuadPart - m_FixTime.QuadPart >= m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_FixTime = m_FrameTime;
	}

	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart) / static_cast<f32_t>(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;
}

unique_ptr<CTimer> CTimer::Create()
{
	auto pInstance = unique_ptr<CTimer>(new CTimer());

	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}

