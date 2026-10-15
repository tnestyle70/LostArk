// GameInstance가 위임하는 tag별 시계 등록·조회·갱신을 구현한다.
#include "Timer_Manager.h"
// 시계를 생성하고 측정 결과를 읽는다. 소멸자 정의에서도 unique_ptr가 해제할 완전한 타입이 필요하다.
#include "Timer.h"

CTimer_Manager::CTimer_Manager()
{
}

CTimer_Manager::~CTimer_Manager()
{
}

// GameInstance와 raid-entry 표현이 저장된 delta를 읽는다. 여기서 QPC를 새로 측정하지 않는다.
f32_t CTimer_Manager::Get_TimeDelta(const wstring_t& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return 0.f;

	return pTimer->Get_TimeDelta();
}

// Client 렌더 프레임마다 Timer_60을 갱신한다. 이름이 Timer_60이어도 60Hz 고정 타이머는 아니다.
void CTimer_Manager::Update_TimeDelta(const wstring_t& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return;

	pTimer->Update_Timer();
}

// 기존 tag를 먼저 확인하고 준비된 unique_ptr만 map으로 이동한다.
HRESULT CTimer_Manager::Add_Timer(const wstring_t& strTimerTag)
{
	if (nullptr != Find_Timer(strTimerTag))
		return E_FAIL;

	unique_ptr<CTimer>		pTimer = CTimer::Create();
	if (nullptr == pTimer)
		return E_FAIL;

	m_Timers.emplace(strTimerTag, std::move(pTimer));

	return S_OK;
}

// 등록·조회·갱신의 공통 탐색이다. 주소만 차용하며 찾는 과정에서 새 tag나 시계를 만들지 않는다.
CTimer* CTimer_Manager::Find_Timer(const wstring_t& strTimerTag)
{
	auto	iter = m_Timers.find(strTimerTag);
	if (iter == m_Timers.end())
		return nullptr;

	return iter->second.get();
}

// CGameInstance로 빈 관리자의 소유권을 넘긴다. 객체 할당 예외는 별도 처리하지 않는다.
unique_ptr<CTimer_Manager> CTimer_Manager::Create()
{
	return unique_ptr<CTimer_Manager>(new CTimer_Manager());
}