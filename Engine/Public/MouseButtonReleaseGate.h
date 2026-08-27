#pragma once

namespace Engine
{

// UI owns the whole physical press, even after its popup/capture disappears.
// Filtered input is never used to infer release.
class CMouseButtonReleaseGate final
{
public:
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
	bool m_suppressedUntilRelease = false;
};

}
