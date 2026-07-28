#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class  CTimer_Manager
{
private:
	CTimer_Manager();
public:
	~CTimer_Manager();

public:
	f32_t Get_TimeDelta(const wstring_t& strTimerTag);
	

public:
	HRESULT Add_Timer(const wstring_t& strTimerTag);
	void Update_TimeDelta(const wstring_t& strTimerTag);



private:
	map<const wstring_t, unique_ptr<class CTimer>>		m_Timers;

private:
	class CTimer* Find_Timer(const wstring_t& strTimerTag);

public:
	static unique_ptr<CTimer_Manager> Create();
};

NS_END