#pragma once

// 시계 tag의 wstring_t, tag별 map/unique_ptr와 실패를 전달할 HRESULT를 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 tag별 시계 저장소다. Client 프레임 루프에서 등록·갱신하고 소비자는 delta를 조회한다.
class  CTimer_Manager
{
private:
	CTimer_Manager();
public:
	~CTimer_Manager();

public:
	// 없는 tag는 0초다. 등록되지 않은 시계와 실제 0초 간격을 이 반환값만으로 구별하지 않는다.
	f32_t Get_TimeDelta(const wstring_t& strTimerTag);
public:
	// 중복 tag/생성 실패는 E_FAIL이고 기존 시계를 유지한다. 갱신에서 없는 tag는 무시한다.
	HRESULT Add_Timer(const wstring_t& strTimerTag);
	void Update_TimeDelta(const wstring_t& strTimerTag);


private:
	// 각 Timer의 유일한 소유자다. 내부 잠금은 없으므로 등록·갱신·조회는 호출자가 직렬화한다.
	map<const wstring_t, unique_ptr<class CTimer>>		m_Timers;

private:
	// map 소유 객체를 호출 동안 차용한다. Manager 파괴 후에는 사용할 수 없다.
	class CTimer* Find_Timer(const wstring_t& strTimerTag);

public:
	// CGameInstance가 소유할 빈 저장소를 만든다. Timer_60 같은 개별 시계 등록은 Client 시작 경로에서 수행한다.
	static unique_ptr<CTimer_Manager> Create();
};

NS_END