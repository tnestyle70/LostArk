#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CTimer final 
{
private:
	CTimer();
public:
	~CTimer();

public:
	f32_t Get_TimeDelta() const { 
		return m_fTimeDelta; }

public:
	HRESULT	Initialize();
	void Update_Timer();

private:
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_FixTime = {};
	LARGE_INTEGER		m_LastTime = {};
	LARGE_INTEGER		m_CpuTick = {};
	f32_t				m_fTimeDelta = {};

public:
	static unique_ptr<CTimer> Create();
};

NS_END