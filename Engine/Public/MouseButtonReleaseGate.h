#pragma once

namespace Engine
{

// Input_Device가 버튼별로 소유하는 두 상태의 gate다. 차단 중 시작한 누름은 해제될 때까지 소비된 상태로 유지한다.
// UI capture가 먼저 사라져도 같은 누름으로 gameplay 클릭이 발생하지 않게 한다.
class CMouseButtonReleaseGate final
{
public:
	// 필터를 거치기 전 눌림과 현재 차단을 받는다. false는 미눌림 또는 소비 중, true는 gameplay에 전달 가능한 눌림이다.
	// 입력 focus/장치 준비 검증은 호출자가 담당하고, 이 함수는 관찰한 해제 때만 자기 대기를 지운다.
	constexpr bool Observe(const bool physicallyDown, const bool blocked)
	{
		if (!physicallyDown)
		{
			m_suppressedUntilRelease = false;
			return false;
		}
		m_suppressedUntilRelease = m_suppressedUntilRelease || blocked;
		return !m_suppressedUntilRelease;
	}

private:
	// 버튼별 입력 객체가 소유하는 해제 대기 한 bit다. 외부 입력이나 다른 버튼 상태를 보관하지 않는다.
	bool m_suppressedUntilRelease = false;
};

}
