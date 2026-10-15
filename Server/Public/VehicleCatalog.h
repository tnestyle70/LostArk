#pragma once

// 탈것 스킬의 시간별 이동 표에도 공통 ROOT_MOTION_SAMPLE 형식을 사용한다.
#include "GameplayCatalog.h"
// 복제/명령 경계와 같은 VEHICLE_ID·SKILL_ID·무효 ID를 사용한다.
#include "Network/PacketMessages.h"

// 최근 bootstrap 로드 진단 문자열을 catalog가 소유한다.
#include <string>
// 탈것 ID별 정의를 소유하고 룸의 탑승·스킬·복제 조회를 지원한다.
#include <unordered_map>
// 탈것별 스킬과 각 스킬의 시간순 root motion 표본을 소유한다.
#include <vector>

namespace LostArk::Server
{
	// 게시 입력 슬롯 SPACE/Q/W/E를 Server 분기로 연결한다. bit 위치로도 사용하므로 선언 순서를 유지한다.
	enum class VEHICLE_SKILL_SLOT : std::uint8_t
	{
		SPACE,
		Q,
		W,
		E,
	};

	// 탈것 입력 한 칸의 권위 스킬 정의다. SPACE는 이동/선박 boost, Q/W/E는 게시된 추가 동작을 연결한다.
	// GameRoom이 지속시간·cooldown·이동을 판정하고 Client는 복제된 동작을 표시한다. 비행 E와 선박 SPACE는 룸의 별도 분기를 사용한다.
	struct SERVER_VEHICLE_SKILL
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		VEHICLE_SKILL_SLOT eSlot = VEHICLE_SKILL_SLOT::SPACE;
		std::uint32_t iCooldownMs = 0u;
		std::uint32_t iActionDurationMs = 0u;
		// 시간은 ms, forward/lateral/up은 누적 이동 m다. 일반 탈것 이동은 forward/lateral 차이를 시전 방향의 전방/오른쪽으로 월드 XZ에 더한다.
		// 현재 GameRoom의 일반 탈것 sampler는 저장된 up을 적용하지 않으며 비행 높이는 아래 flight profile로 계산한다.
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
	};

	// 하나의 탈것 정의를 값으로 소유한다. 실제 탑승자 위치·비행 단계·boost 종료 tick·cooldown은 SERVER_PLAYER에 따로 보관한다.
	struct SERVER_VEHICLE_DEFINITION
	{
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		// 일반 탑승 이동 속도(m/s)다. 룸의 현재 단계에 따라 비행/boost 속도가 대신 선택될 수 있다.
		float fMoveSpeed = 0.f;
		// 선택 비행 profile이다. 이륙/착륙은 초, 높이는 기준 지면에서의 m, 수평/수직 속도는 m/s다.
		// 게시 profile이 없으면 여섯 값이 0이며 Load는 비행이 켜진 Ancient Sea와 비행 없는 행을 구분해 검증한다.
		float fFlightTakeoffSeconds = 0.f, fFlightLandingSeconds = 0.f;
		float fFlightHoverHeight = 0.f, fFlightMaximumHeight = 0.f;
		float fFlightSpeed = 0.f, fFlightVerticalSpeed = 0.f;
		// 이륙 시간이 양수인지 판별하는 값 조회다. 나머지 profile의 유효성은 Load가 검사한다.
		bool Has_Flight() const
		{
			return fFlightTakeoffSeconds > 0.f;
		}
		// 선박 빠른 항해 동안 일반 속도 대신 사용하는 게시 속도(m/s)다. 0이면 boost가 없다.
		// 시작 허용·기간·cooldown은 룸이 SPACE 스킬의 시간을 읽어 판정하며 이 값 자체는 플레이어 상태를 바꾸지 않는다.
		float fBoostMoveSpeed = 0.f;
		// boost 속도가 양수인지 판별한다. 일반 속도보다 빠른지와 비행 겸용 금지는 Load가 검사한다.
		bool Has_Boost() const
		{
			return fBoostMoveSpeed > 0.f;
		}
		std::vector<SERVER_VEHICLE_SKILL> Skills;

		// 룸의 스킬 시작/진행이 ID로 첫 스킬을 차용한다. 없으면 nullptr이며 이 정의/Skills의 수명을 넘겨 보관하지 않는다.
		const SERVER_VEHICLE_SKILL* Find_Skill(const LostArk::Shared::SKILL_ID skillId) const
		{
			for (const SERVER_VEHICLE_SKILL& skill : Skills)
				if (skill.iSkillId == skillId)
					return &skill;
			return nullptr;
		}
	};

	// GameRoom이 값으로 소유하고 초기화 때 로드한 뒤 룸 worker가 읽는다. Load와 조회를 동시 실행할 자체 잠금은 없다.
	class CVehicleCatalog final
	{
	  public:
		// Vehicles.bootstrap version 2~4를 지역 map에 모아 행/스킬 검증이 모두 끝나면 교체한다.
		// 명시적인 false 반환은 이전 탈것 map을 보존하지만 일부 flight/boost 실패는 진단 문자열을 갱신하지 않는다.
		bool Load();

		// 탑승·이동·스킬·입장·복제 경로가 ID 정의를 차용한다. nullptr은 미등록이며 호출자는 반환값을 해제하지 않는다.
		// 성공한 재로드 또는 catalog 소멸을 넘겨 포인터를 보관하지 않는다.
		const SERVER_VEHICLE_DEFINITION* Find_Vehicle(LostArk::Shared::VEHICLE_ID vehicleId) const;

		// catalog가 소유한 최근 기록 진단의 참조다. Load의 모든 실패가 새 문구를 남기는 것은 아니다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	  private:
		// 성공한 로드만 교체하는 정의 저장소다. 각 스킬/root motion vector도 이 map의 값이 소유한다.
		std::unordered_map<LostArk::Shared::VEHICLE_ID, SERVER_VEHICLE_DEFINITION> m_Vehicles;
		std::string m_strStatus;
	};
}
