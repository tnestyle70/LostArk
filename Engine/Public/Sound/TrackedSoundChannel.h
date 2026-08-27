#pragma once

namespace Engine
{

// One independent playback owner. A failed replacement releases only its
// staged channel and leaves the currently audible channel unchanged.
template <typename CHANNEL>
class CTrackedSoundChannel final
{
public:
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

	void Stop()
	{
		if (nullptr != m_pChannel)
			m_pChannel->stop();
		m_pChannel = nullptr;
	}

private:
	CHANNEL* m_pChannel = nullptr;
};

}
