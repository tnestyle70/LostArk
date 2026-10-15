#pragma once

namespace Engine
{

// 음악/반복 효과음이 각각 추적하는 재생 채널이다. CHANNEL 객체의 메모리는 backend가 관리한다.
// 별도 새 채널 준비가 false이면 그 임시 채널만 stop하고 기존 채널 포인터는 유지한다.
// 자체 소멸자가 stop을 호출하지 않으므로 소유 manager가 종료 시 Stop을 호출해야 한다.
template <typename CHANNEL>
class CTrackedSoundChannel final
{
public:
	// manager가 새 채널을 준비하는 동기 callback을 전달한다. 기존 채널과 다른 새 채널을 출력해야 한다.
	// callback이 성공한 뒤 기존 채널을 stop하고 교체한다. callback 예외나 stop 오류의 rollback은 제공하지 않는다.
	template <typename PREPARE>
	bool Try_Replace(PREPARE&& prepare)
	{
		CHANNEL* staged = nullptr;
		if (!prepare(staged) || nullptr == staged)
		{
			if (nullptr != staged)
				staged->stop();
			return false;
		}
		Stop();
		m_pChannel = staged;
		return true;
	}

	// backend의 stop을 요청한 뒤 오류 반환과 무관하게 추적 포인터를 비운다. delete/release로 채널을 소유하지 않는다.
	void Stop()
	{
		if (nullptr != m_pChannel)
			m_pChannel->stop();
		m_pChannel = nullptr;
	}

private:
	// backend 수명 안에서만 유효한 차용 포인터다. 복사본을 여러 재생 소유자로 사용하거나 동시에 변경하지 않는다.
	CHANNEL* m_pChannel = nullptr;
};

}
