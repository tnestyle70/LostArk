#pragma once

#include "Network/PacketMessages.h"
#include "Network/PacketType.h"

#include <array>
#include <cstdint>
#include <string>

namespace LostArk::Server
{
	// GameRoom별 공유 에스더 게이지와 world별 슬롯 정의다. session마다 별도 게이지를 만들지 않는다.
	// 아래 roster와 충전 수치는 현재 코드 상수이며, 시전자 상태/방 입장/소환/피해 적용은 GameRoom의 책임이다.

	// 소환 entity가 복제할 action ID다. Client는 NpcCatalog의 actionClips 연결로 실제 animation을 고른다.
	inline constexpr const char* ESTHER_ACTION_STRIKE = "esther.strike";

	// GameRoom::Begin_EstherCall이 소환을 예약할 지연 초와 조준 방향 전방 거리 m다.
	// 예약 때 위치/방향을 고정하고 목표 navigation 표본을 얻지 못하면 시전자 위치를 유지한다.
	inline constexpr float ESTHER_SUMMON_DELAY_SECONDS = 1.f;
	inline constexpr float ESTHER_SUMMON_FORWARD_METERS = 2.f;

	// GameRoom이 시전자의 ESTHER_CAST action을 유지하는 ms다.
	// player update는 1500ms를 30Hz tick 수로 올림 변환해 iActionStartTick 기준 만료를 검사한다.
	inline constexpr std::uint32_t ESTHER_CAST_DURATION_MS = 1500u;

	// 소환할 에스더의 ID, archetype ID와 strike 유지 ms다. 이름 문자열은 아래 constexpr 정의의 정적 수명을 가진다.
	// GameRoom이 NPC를 이 action으로 생성하고 경과 시간이 iStrikeMs에 도달하면 despawn한다.
	struct ESTHER_ROSTER_ENTRY
	{
		LostArk::Shared::ESTHER_ID eEstherId;
		const char* pArchetypeId;
		std::uint32_t iStrikeMs;
	};
	inline constexpr ESTHER_ROSTER_ENTRY ESTHER_DEFINITIONS[] =
	{
		{ LostArk::Shared::ESTHER_ID::SILLIAN, "NPC_59030", 5300u },
		{ LostArk::Shared::ESTHER_ID::WEI, "NPC_58700", 7100u },
		{ LostArk::Shared::ESTHER_ID::BAHUNTUR, "NPC_59060", 4100u },
		{ LostArk::Shared::ESTHER_ID::NINAV, "NPC_59504", 4900u },
		{ LostArk::Shared::ESTHER_ID::INANNA, "NPC_59620", 4100u },
	};

	// world별 슬롯 순서다. Valtan과 Character Select는 실리안/웨이/바훈투르, Kouku는 니나브/웨이/이난나다.
	// Debug의 ID 직접 소환은 이 슬롯 선택/게이지 소비를 우회하지만 같은 정의와 GameRoom 소환 경로를 사용한다.
	inline constexpr std::size_t ESTHER_ROSTER_SLOT_COUNT =
		LostArk::Shared::MAX_ESTHER_SLOT_INDEX -
		LostArk::Shared::MIN_ESTHER_SLOT_INDEX + 1u;
	using ESTHER_WORLD_ROSTER =
		std::array<LostArk::Shared::ESTHER_ID, ESTHER_ROSTER_SLOT_COUNT>;
	inline constexpr ESTHER_WORLD_ROSTER ESTHER_ROSTER_VALTAN =
	{
		LostArk::Shared::ESTHER_ID::SILLIAN,
		LostArk::Shared::ESTHER_ID::WEI,
		LostArk::Shared::ESTHER_ID::BAHUNTUR,
	};
	inline constexpr ESTHER_WORLD_ROSTER ESTHER_ROSTER_KOUKUSAYDON =
	{
		LostArk::Shared::ESTHER_ID::NINAV,
		LostArk::Shared::ESTHER_ID::WEI,
		LostArk::Shared::ESTHER_ID::INANNA,
	};

	// slot 소비와 Debug 직접 소환이 ID로 정적 정의를 찾는다. 없으면 nullptr이며 반환 행의 수명은 process 전체다.
	[[nodiscard]] const ESTHER_ROSTER_ENTRY* Find_EstherDefinition(
		LostArk::Shared::ESTHER_ID estherId);

	enum class ESTHER_USE_REJECTION : std::uint8_t
	{
		NONE,
		DISABLED_WORLD,
		UNSUPPORTED_SLOT,
		GAUGE_NOT_FULL
	};

	// 한 GameRoom이 게이지/활성 roster를 값으로 소유한다. 방의 tick/명령 경로가 순차 호출하며 이 클래스에는 내부 잠금이 없다.
	// party leader 여부를 입력으로 받지 않는다. 실제 호출자의 자격은 GameRoom::Find_EstherCaster가 먼저 검사한다.
	class CEstherSkillSystem final
	{
	public:
		// Valtan/Kouku/Character Select에서 활성화하고 world별 roster를 고른 뒤 게이지를 0으로 초기화한다.
		// 그 외 world는 Get_GaugeMaximum이 0이며 snapshot이 에스더 비활성 상태로 전달한다.
		void Initialize(LostArk::Shared::WORLD_ID worldId);

		[[nodiscard]] bool Is_Enabled() const
		{
			return m_isEnabled;
		}
		[[nodiscard]] std::uint32_t Get_Gauge() const
		{
			return m_iGauge;
		}
		[[nodiscard]] std::uint32_t Get_GaugeMaximum() const
		{
			return m_isEnabled ? GAUGE_MAXIMUM : 0u;
		}

		// GameRoom의 fixed tick이 충전을 요청한다. 인간 플레이어가 없으면 현재 게이지를 유지한 채 충전만 멈춘다.
		void Update(float fixedDeltaSeconds, bool hasPlayers);
		// 게이지와 소수 잔여량만 지운다. world 활성 여부와 roster를 유지하며 소환 entity/예약 정리는 GameRoom이 별도로 한다.
		void Reset();

		// world/slot/가득 찬 게이지를 검사한다. NONE이면 게이지와 소수 잔여량을 0으로 하고 outEntry에 정적 roster 행을 돌려준다.
		// 거부하면 게이지와 outEntry를 모두 보존한다. 실제 시전자 자격/소환 성공을 보장하는 함수는 아니다.
		[[nodiscard]] ESTHER_USE_REJECTION Try_Consume(
			std::uint8_t slotIndex,
			const ESTHER_ROSTER_ENTRY*& outEntry);

	private:
		// 현재 최대 1000과 초당 200 충전의 코드 정책이다. Update 입력은 GameRoom의 작은 fixed delta를 전제로 한다.
		static constexpr std::uint32_t GAUGE_MAXIMUM = 1000u;
		static constexpr float REGEN_PER_SECOND = 200.f;

		bool m_isEnabled = false;
		ESTHER_WORLD_ROSTER m_Roster{};
		std::uint32_t m_iGauge = 0u;
		// 매 tick의 소수점 이하 충전량을 다음 tick에 넘겨 정수 변환 때마다 버리는 일을 막는다. 부동소수점 연산 오차 자체를 없애지는 않는다.
		float m_fRegenRemainder = 0.f;
	};
}
