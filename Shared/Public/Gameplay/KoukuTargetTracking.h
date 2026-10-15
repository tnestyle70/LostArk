#pragma once

// 경과 tick을 남은 tick으로 제한하고 매 tick의 회전 비율을 1 이하로 묶는다.
#include <algorithm>
// 묶어 계산하는 경과/잔여 tick 수를 64비트 정수로 받는다.
#include <cstdint>
// 응답 배율과 yaw의 유한성을 검사하고 remainder로 최단 각도 차이를 만든다.
#include <cmath>

namespace LostArk::Shared::KoukuTargetTracking
{
	// 제자리 추적은 남은 각도의 일부를 매 tick 소비하며 그 응답 배율은 10이다.
	// 이동 허용 추적의 180 degree/초 제한은 소비자의 다른 경로이므로 여기서 적용하지 않는다.
	inline constexpr std::uint32_t ROTATE_ONLY_RESPONSE_SCALE = 10u;

	// Valtan/Kouku의 Server 조준과 Client 표현이 공유한다. elapsed/remaining은 같은 고정 tick 단위다.
	// 이번 구간에서 남은 각도의 얼마를 소비할지 0~1 비율로 계산하며 입력 상태는 바꾸지 않는다.
	// Server 보스 조준과 Client Kouku 고정 tick preview가 회전량에 곱한다.
	// remainingTicks는 이번 묶음 시작 때 남은 수이며, elapsedTicks가 이를 넘으면 남은 구간까지만 계산한다. 시간 진행·목표 선택은 호출자 책임이다.
	inline double RotateOnlyFraction(
		const std::uint64_t elapsedTicks, const std::uint64_t remainingTicks, const double responseScale = 1.0) noexcept
	{
		if (!elapsedTicks || !remainingTicks || !std::isfinite(responseScale) || responseScale <= 0.0)
			return 0.0;

		// 임의 배율은 각 tick 뒤 남는 각도 비율을 직접 곱한다. 남은 tick 수가 줄어드는 응답이므로 단순한 초당 회전 속도로 해석하지 않는다.
		if (responseScale != 1.0)
		{
			// 매 tick 뒤 남는 비율을 곱한다. 목표와 배율이 고정된 구간에서는 여러 tick을
			// 묶어 계산해도 한 tick씩 적용한 결과와 수학적으로 같으며 부동소수 반올림 차이는 가능하다.
			double retained = 1.0;
			const auto elapsed = (std::min)(elapsedTicks, remainingTicks);
			for (std::uint64_t tick = 0u; tick < elapsed; ++tick)
			{
				const double remaining = static_cast<double>(remainingTicks - tick);
				const double fraction = (std::min)(1.0, double(ROTATE_ONLY_RESPONSE_SCALE) * responseScale / remaining);
				retained *= 1.0 - fraction;
				if (retained <= 0.0)
					return 1.0;
			}
			return 1.0 - retained;
		}

		const auto elapsed = (std::min)(elapsedTicks, remainingTicks);
		// 배율이 정확히 1일 때만 아래 최대 10개 인자 형태를 사용한다. gain을 잔여 tick 이하로 제한해 unsigned 뺄셈과 분모 범위를 유지한다.
		const auto gain = (std::min<std::uint64_t>)(ROTATE_ONLY_RESPONSE_SCALE, remainingTicks);
		if (elapsed >= remainingTicks - gain + 1u)
			return 1.0;
		// tick별 잔여 비율 (remaining - gain) / remaining의 곱에서 공통 인자를 소거한다.
		// 배율 1일 때 elapsed 횟수 대신 최대 gain(10)개 인자로 같은 잔여 비율을 구한다.
		double retained = 1.0;
		for (std::uint64_t index = 0u; index < gain; ++index)
			retained *= double(remainingTicks - elapsed - index) / double(remainingTicks - index);
		return 1.0 - retained;
	}

	// yaw와 결과는 degree다. remainder로 최단 각도 차이를 택한 뒤 위 비율만큼 회전한다.
	// yaw가 유한하지 않으면 currentYaw를 그대로 반환한다. 유효한 범위 검증과 tick 경과 계산은 호출자 책임이다.
	// ValtanBrain과 Client Valtan의 로컬 조준 preview가 결과를 자신의 yaw에 대입한다.
	// 이 helper는 좌표 위치나 yaw 입력을 변경하지 않으며 보간 중 목표가 바뀌면 그 새 목표까지의 최단 차이로 다시 계산한다.
	inline double RotateOnlyYaw(const double currentYaw, const double targetYaw, const std::uint64_t elapsedTicks,
		const std::uint64_t remainingTicks, const double responseScale = 1.0) noexcept
	{
		if (!std::isfinite(currentYaw) || !std::isfinite(targetYaw))
			return currentYaw;

		const double turn = std::remainder(targetYaw - currentYaw, 360.0);
		return std::remainder(
			currentYaw + turn * RotateOnlyFraction(elapsedTicks, remainingTicks, responseScale), 360.0);
	}
}
