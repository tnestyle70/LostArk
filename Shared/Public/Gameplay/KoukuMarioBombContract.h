#pragma once

// Mario 2~4의 marker/arrival/exit 연결 7개를 고정 크기 정적 테이블로 보관한다.
#include <array>
// 발생 시각의 tick 올림과 현재 시각의 발생 순번 내림을 ceil/floor로 계산한다.
#include <cmath>
// tick·slot·seed와 시작 전 -1을 포함하는 발생 순번의 정수 너비를 고정한다.
#include <cstdint>
// marker와 slot 번호를 합친 인스턴스 ID를 소유 문자열로 반환한다.
#include <string>

namespace LostArk::Shared::KoukuMarioBomb
{
	// Server 접촉과 Client 폭탄 표현이 같은 생성 간격(ms), 이동 속도(m/s), 반경(m)을 사용한다.
	inline constexpr std::uint32_t INTERVAL_MS = 8000u;
	inline constexpr float SPEED_MPS = 3.f;
	inline constexpr float RADIUS_M = .36f;
	inline constexpr float LOW_BOTTOM_M = .05f, HIGH_BOTTOM_M = .90f;

	// 포인터는 아래 정적 문자열을 차용한다. marker/arrival/exit는 placement ID이고 리소스 경로가 아니다.
	struct BINDING
	{
		std::uint8_t stage;
		const char* marker;
		const char* arrival;
		const char* exit;
	};

	inline constexpr std::array<BINDING, 7u> BINDINGS{{{2u, "Mario2_Boom", "Mario2_go", "Mario2_Trigger_2"},
		{2u, "Mario2_Boom_1", "Mario2_Trigger_2", "Mario2_Trigger_4"},
		{2u, "Mario2_Boom_2", "Mario2_Trigger_4", "Mario2_Trigger_7"},
		{3u, "Mario3_Boom", "Mario3_Trigger_4", "Mario3_Trigger_5"},
		{3u, "Mario3_Boom_1", "Mario3_Trigger_4", "Mario3_Trigger_5"},
		{3u, "Mario3_Boom_2", "Mario3_Trigger_8", "Mario3_Trigger_10"},
		{4u, "Mario4_Boom", "Mario4_Tigger_6", "Mario4_Tigger_7"}}};

	// marker와 재사용 slot으로 양쪽 소비자가 같은 인스턴스 ID를 만든다. 세대 birth는 별도로 비교한다.
	// Server 접촉 STOP과 Client sequence 생성이 같은 ID를 사용한다. marker는 유효한 NUL 종료 문자열이어야 하며 slot 범위는 호출자가 정한다.
	// 문자열 할당 예외는 전달되며 이 함수 자체는 sequence를 생성하거나 종료하지 않는다.
	inline std::string InstanceId(const BINDING& binding, const std::uint32_t slot)
	{
		return std::string("world.object.instance.mario.bomb.") + binding.marker + ".slot" + std::to_string(slot);
	}

	// 발생 순번 * 간격 + phase(ms)를 서버 tick으로 올림한다. 음수 세대/0Hz/표현 범위 밖은 0이다.
	// Server STOP의 iStartTick과 Client 수신 세대 검증이 사용한다. 절대 발생 시각을 변환할 뿐 uint32 tick 순환을 보정하지 않는다.
	inline std::uint32_t BirthTick(
		const std::uint32_t phaseMs, const std::int64_t birth, const std::uint32_t tickHz) noexcept
	{
		if (birth < 0 || !tickHz)
			return 0u;

		const double tick = std::ceil((double(birth) * INTERVAL_MS + phaseMs) * tickHz / 1000.);
		return tick > 0. && tick <= 4294967295. ? static_cast<std::uint32_t>(tick) : 0u;
	}

	// marker 문자열의 각 byte를 32-bit FNV-1a로 섞는다. unsigned overflow를 사용하며 암호학적 난수가 아니다.
	// marker는 null이 아닌 NUL 종료 문자열이어야 한다.
	// 양쪽 소비자는 이 값의 INTERVAL_MS 나머지를 발사 phase로, 같은 seed를 Bottom의 높이 선택으로 사용한다. vector index는 seed에 넣지 않는다.
	inline std::uint32_t Seed(const char* marker) noexcept
	{
		std::uint32_t result = 2166136261u;
		for (; *marker; ++marker)
			result = (result ^ static_cast<unsigned char>(*marker)) * 16777619u;
		return result;
	}

	// marker seed와 발생 순번으로 같은 세대의 높이를 재현한다. 반환값은 바닥 offset(m)이며 중심 반경은 소비자가 더한다.
	// birth의 하위 32비트와 seed를 정수 연산으로 섞어 두 높이 중 하나를 고른다. 공유 난수기나 누적 상태를 변경하지 않는다.
	inline float Bottom(const std::uint32_t seed, const std::int64_t birth) noexcept
	{
		std::uint32_t random = seed ^ (static_cast<std::uint32_t>(birth) * 0x9e3779b9u);
		random ^= random << 13;
		random ^= random >> 17;
		random ^= random << 5;
		return random & 1u ? HIGH_BOTTOM_M : LOW_BOTTOM_M;
	}

	// 양쪽 소비자가 Server 시계(ms)에서 slot % count에 해당하는 가장 최근 발생 순번을 구한다.
	// count=0 또는 아직 시작 전인 slot은 -1이다. clockMs는 유한하며,
	// floor((clockMs-phaseMs)/INTERVAL_MS)의 결과가 int64 범위이고 slot<count라는 전제가 필요하다.
	// Server 접촉 검사와 Client 재생/STOP 해석은 같은 count와 phase를 전달한다. 유효한 비음수 반환 순번은 count로 나눈 나머지가 slot이며 같은 slot의 연속 발생 세대 간격은 count다.
	// 호출 간 시간이 적게 흐르면 같은 순번을 반환하고 오래 지나면 여러 세대를 건너뛸 수 있다. 비행 수명과 접촉 중단 여부는 호출자가 age와 세대 기록으로 따로 확인한다.
	inline std::int64_t Birth(
		const double clockMs, const std::uint32_t phaseMs, const std::uint32_t slot, const std::uint32_t count) noexcept
	{
		if (!count)
			return -1;

		const auto latest = static_cast<std::int64_t>(std::floor((clockMs - phaseMs) / INTERVAL_MS));
		return latest < slot ? -1 : latest - (latest - slot) % count;
	}
}
