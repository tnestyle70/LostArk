#include "Timer_Manager.h"
#include "Timer.h"

CTimer_Manager::CTimer_Manager()
{
}

CTimer_Manager::~CTimer_Manager()
{	
}

f32_t CTimer_Manager::Get_TimeDelta(const wstring_t& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return 0.f;

	return pTimer->Get_TimeDelta();
}

void CTimer_Manager::Update_TimeDelta(const wstring_t& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return;

	pTimer->Update_Timer();
}

HRESULT CTimer_Manager::Add_Timer(const wstring_t& strTimerTag)
{
	if (nullptr != Find_Timer(strTimerTag))
		return E_FAIL;

	unique_ptr<CTimer>		pTimer = CTimer::Create();	
	if (nullptr == pTimer)
		return E_FAIL;

	m_Timers.emplace(strTimerTag, std::move(pTimer));

	/*unique_ptr<CTimer>		pTimer = CTimer::Create();	
	if (nullptr == pTimer)
		return E_FAIL;

	m_Timers.emplace(strTimerTag, std::move(pTimer));*/

	return S_OK;
}

CTimer* CTimer_Manager::Find_Timer(const wstring_t& strTimerTag)
{
	//auto		iter = find_if(m_mapTimer.begin(), m_mapTimer.end(),	// 
	//	CTag_Finder(pTimerTag));

	auto	iter = m_Timers.find(strTimerTag);
	if (iter == m_Timers.end())
		return nullptr;

	return iter->second.get();
}

unique_ptr<CTimer_Manager> CTimer_Manager::Create()
{
	return unique_ptr<CTimer_Manager>(new CTimer_Manager());
}