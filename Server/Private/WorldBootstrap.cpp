// 게시된 worldbootstrap의 배치·트리거·NPC 논리와 쿠크 미니게임 투영 데이터를 Server 값 타입으로 읽는다.
// 실제 entity 생성/트리거 실행/충돌 판정은 CGameRoom이 이 결과를 소비하는 단계에서 수행한다.
#include "WorldBootstrap.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string_view>
#include <unordered_set>

namespace
{
	using namespace LostArk::Server;
	using LostArk::Shared::WORLD_ID;

	// 파일 이름과 header의 world 이름에 쓰는 정적 literal view다. 미지원 ID는 빈 view로 반환한다.
	std::string_view World_ToString(const WORLD_ID worldId)
	{
		switch (worldId)
		{
		case WORLD_ID::BERN:
			return "BERN";
		case WORLD_ID::VALTAN_ARENA:
			return "VALTAN_ARENA";
		case WORLD_ID::TRAINING_GROUND:
			return "TRAINING_GROUND";
		case WORLD_ID::CHARACTER_SELECT_ARENA:
			return "CHARACTER_SELECT_ARENA";
		case WORLD_ID::KOUKUSAYDON_ARENA:
			return "KOUKUSAYDON_ARENA";
		case WORLD_ID::MAHARAKA:
			return "MAHARAKA";
		case WORLD_ID::COLOSSEUM:
			return "COLOSSEUM";
		default:
			return {};
		}
	}

	// 명시한 Server data root를 먼저 사용하고, 없으면 EXE 상위 두 단계 아래 DataFiles를 찾는다.
	// 경로를 조합할 뿐 존재 여부는 여기서 검사하지 않는다. 이 계층은 저작 Data JSON을 직접 읽지 않는다.
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	// 탭으로 나눈 view는 line의 문자 메모리를 차용한다. 빈 필드와 끝의 빈 필드도 남긴다.
	// 다음 getline/문자열 변경 전에 모두 소비하거나 소유 string으로 복사해야 한다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> values;
		std::string_view view(line);
		size_t start = 0u;
		while (true)
		{
			const size_t tab = view.find('\t', start);
			values.push_back(view.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return values;
	}

	// binary stream의 getline이 남긴 마지막 CR 한 개만 제거한다. 필드 안 공백/CR은 정규화하지 않는다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	// from_chars가 전체 필드를 소비했을 때만 성공이다. 숫자 뒤 잘못된 접미사가 있으면 outValue는 바뀌어도 false일 수 있다.
	// 호출자는 지역 후보 필드에 읽고 행 전체 실패 시 버린다. 실수의 finite 여부와 도메인 범위는 각 행에서 추가 검사한다.
	template<typename T>
	bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), outValue);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	// 저장 행으로 받을 다섯 종류만 연결한다. enum에 있는 MONSTER/WORLD_OBJECT는 이 parser에서 받아들이지 않는다.
	bool ParseKind(
		const std::string_view value,
		WORLD_BOOTSTRAP_KIND& outKind)
	{
		if ("playerSpawn" == value)
			outKind = WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN;
		else if ("npc" == value)
			outKind = WORLD_BOOTSTRAP_KIND::NPC;
		else if ("boss" == value)
			outKind = WORLD_BOOTSTRAP_KIND::BOSS;
		else if ("triggerBox" == value)
			outKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		else if ("collisionBox" == value)
			outKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		else
			return false;
		return true;
	}

	// NPC 이동 정책 이름만 enum으로 변환한다. 잘못된 이름이면 출력 enum은 그대로 두고 실패한다.
	bool ParseNpcBehaviorMode(
		const std::string_view value,
		NPC_BEHAVIOR_MODE& outMode)
	{
		if ("stationary" == value)
			outMode = NPC_BEHAVIOR_MODE::STATIONARY;
		else if ("patrol" == value)
			outMode = NPC_BEHAVIOR_MODE::PATROL;
		else if ("wander" == value)
			outMode = NPC_BEHAVIOR_MODE::WANDER;
		else
			return false;
		return true;
	}

	// waypoint 경로의 반복 정책을 읽는다. 실제 다음 waypoint 선택은 NPC runtime의 책임이다.
	bool ParseNpcRouteMode(
		const std::string_view value,
		NPC_ROUTE_MODE& outMode)
	{
		if ("loop" == value)
			outMode = NPC_ROUTE_MODE::LOOP;
		else if ("pingPong" == value)
			outMode = NPC_ROUTE_MODE::PING_PONG;
		else if ("once" == value)
			outMode = NPC_ROUTE_MODE::ONCE;
		else
			return false;
		return true;
	}

	// 논리 action을 순차/가중 선택할지를 읽는다. clip 이름이나 Client 재생 설정을 이 데이터에 포함하지 않는다.
	bool ParseNpcActionSelection(
		const std::string_view value,
		NPC_ACTION_SELECTION& outSelection)
	{
		if ("sequence" == value)
			outSelection = NPC_ACTION_SELECTION::SEQUENCE;
		else if ("weighted" == value)
			outSelection = NPC_ACTION_SELECTION::WEIGHTED;
		else
			return false;
		return true;
	}

	// changeLevel이 가리킬 수 있는 목적지만 받는다. 현재 world와 같은지 여부는 action 행에서 별도로 거부한다.
	bool ParseTriggerTargetWorld(
		const std::string_view value,
		WORLD_ID& outWorldId)
	{
		if ("BERN" == value)
			outWorldId = WORLD_ID::BERN;
		else if ("VALTAN_ARENA" == value)
			outWorldId = WORLD_ID::VALTAN_ARENA;
		else if ("KOUKUSAYDON_ARENA" == value)
			outWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
		else if ("MAHARAKA" == value)
			outWorldId = WORLD_ID::MAHARAKA;
		else if ("COLOSSEUM" == value)
			outWorldId = WORLD_ID::COLOSSEUM;
		else
			return false;
		return true;
	}

	// 최대 128-byte ID를 현재 C 문자 locale의 영숫자 및 _.-로 제한한다. catalog에 실제 존재하는지는 검사하지 않는다.
	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}
}

// CGameRoom 생성 시 world별 게시 파일을 한 번 해석한다. 명시적인 false 경로는 기존 collection/area/revision을 유지하고 상태 문자열만 바꾼다.
// 파일·경로·메모리 할당 예외를 catch하지 않으며 마지막 멤버 교체 전체에 대한 예외 rollback/동시 접근 잠금은 제공하지 않는다.
bool LostArk::Server::CWorldBootstrap::Load(
	const LostArk::Shared::WORLD_ID worldId)
{
	// 1. world와 게시 경로를 결정하고 header를 읽는다. 버전 8~12에 따라 뒤에 이어질 table의 개수가 달라진다.
	const std::string_view worldName = World_ToString(worldId);
	if (worldName.empty())
	{
		m_strStatus = "Unknown world ID";
		return false;
	}
	const std::filesystem::path dataRoot = Resolve_DataRoot();
	if (dataRoot.empty())
	{
		m_strStatus = "Could not resolve server data root";
		return false;
	}
	const std::filesystem::path path = dataRoot / L"World" /
		std::filesystem::path(std::string(worldName) + ".worldbootstrap");
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_strStatus = "Missing world bootstrap: " + path.string();
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Empty world bootstrap";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t revision = 0;
	std::uint32_t count = 0;
	std::uint32_t sequenceCount = 0;
	std::uint32_t laneCount = 0;
	std::uint32_t ballCount = 0;
	std::uint32_t bouncingBallCount = 0;
	// v8 배치, v9 sequence, v10 0/36개 lane, v11 source ball, v12 bouncing ball의 선언 개수를 검사한다.
	// 각 버전의 정확한 필드 수를 먼저 확인하는 단락 평가 순서를 유지해 header 인덱스 접근을 보호한다.
	if ((6u != header.size() && 7u != header.size() && 8u != header.size() && 9u != header.size() && 10u != header.size()) ||
		"LOSTARK_WORLD_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || (8u != version && 9u != version && 10u != version && 11u != version && 12u != version) ||
		(8u == version && 6u != header.size()) ||
		(9u == version && (7u != header.size() || !ParseNumber(header[6], sequenceCount) || sequenceCount > 4096u)) ||
		(10u == version && (8u != header.size() || !ParseNumber(header[6], sequenceCount) || sequenceCount > 4096u ||
			!ParseNumber(header[7], laneCount) || (laneCount != 0u && laneCount != 36u))) ||
		(11u == version && (9u != header.size() || !ParseNumber(header[6], sequenceCount) || sequenceCount > 4096u ||
			!ParseNumber(header[7], laneCount) || (laneCount != 0u && laneCount != 36u) ||
			!ParseNumber(header[8], ballCount) || ballCount > 4096u)) ||
		(12u == version && (10u != header.size() || !ParseNumber(header[6], sequenceCount) || sequenceCount > 4096u ||
			!ParseNumber(header[7], laneCount) || (laneCount != 0u && laneCount != 36u) ||
			!ParseNumber(header[8], ballCount) || ballCount > 4096u ||
			!ParseNumber(header[9], bouncingBallCount) || bouncingBallCount > 128u)) ||
		header[2] != worldName || !IsStableId(header[3]) ||
		!ParseNumber(header[4], revision) || 0u == revision ||
		!ParseNumber(header[5], count) || count > 4096u)
	{
		m_strStatus = "World bootstrap header is invalid";
		return false;
	}
	// header view는 line을 차용한다. 이후 getline이 같은 버퍼를 재사용하기 전에 area ID만 소유 string으로 복사한다.
	const std::string stagedAreaId{ header[3] };

	// 2. 배치 행을 지역 후보에 누적한다. row의 view는 이 반복 안에서만 읽고 저장 ID는 string으로 복사한다.
	std::vector<WORLD_BOOTSTRAP_PLACEMENT> staged;
	std::unordered_set<std::string> placementIds;
	staged.reserve(count);
	for (std::uint32_t index = 0; index < count; ++index)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "World bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		WORLD_BOOTSTRAP_PLACEMENT placement;
		int enabled = 0;
		if (fields.size() < 9u ||
			!IsStableId(fields[0]) || !ParseKind(fields[1], placement.eKind) ||
			("-" != fields[2] && !IsStableId(fields[2])) ||
			("-" != fields[3] && !IsStableId(fields[3])) ||
			!ParseNumber(fields[4], placement.fPositionX) ||
			!ParseNumber(fields[5], placement.fPositionY) ||
			!ParseNumber(fields[6], placement.fPositionZ) ||
			!ParseNumber(fields[7], placement.fYawDegrees) ||
			!ParseNumber(fields[8], enabled) || (0 != enabled && 1 != enabled) ||
			!std::isfinite(placement.fPositionX) ||
			!std::isfinite(placement.fPositionY) ||
			!std::isfinite(placement.fPositionZ) ||
			!std::isfinite(placement.fYawDegrees))
		{
			m_strStatus = "World bootstrap placement is invalid at row " +
				std::to_string(index);
			return false;
		}
		placement.strPlacementId = fields[0];
		placement.isEnabled = 1 == enabled;
		// 공통 위치는 world 좌표, yaw는 degree다. 종류별 추가 필드 수와 반쪽 크기/실행 payload를 여기서 분기한다.
		if (WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind)
		{
			if (12u != fields.size() || "-" != fields[2] || "-" != fields[3] ||
				!ParseNumber(fields[9], placement.fHalfExtentX) ||
				!ParseNumber(fields[10], placement.fHalfExtentY) ||
				!ParseNumber(fields[11], placement.fHalfExtentZ) ||
				!std::isfinite(placement.fHalfExtentX) ||
				!std::isfinite(placement.fHalfExtentY) ||
				!std::isfinite(placement.fHalfExtentZ) ||
				placement.fHalfExtentX <= 0.f || placement.fHalfExtentX > 1000.f ||
				placement.fHalfExtentY <= 0.f || placement.fHalfExtentY > 1000.f ||
				placement.fHalfExtentZ <= 0.f || placement.fHalfExtentZ > 1000.f)
			{
				m_strStatus = "World collision shape is invalid at row " +
					std::to_string(index);
				return false;
			}
		}
		else if (WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind)
		{
			// enabled trigger는 action 정확히 한 개, disabled trigger는 0~1개다. 상호작용 여부는 독립 bool 필드로 보관한다.
			int triggerOnce = 0;
			int requiresInteract = 0;
			std::uint32_t actionCount = 0;
			if (fields.size() < 15u || "-" != fields[2] || "-" != fields[3] ||
				!ParseNumber(fields[9], placement.fHalfExtentX) ||
				!ParseNumber(fields[10], placement.fHalfExtentY) ||
				!ParseNumber(fields[11], placement.fHalfExtentZ) ||
				!ParseNumber(fields[12], triggerOnce) ||
				(0 != triggerOnce && 1 != triggerOnce) ||
				!ParseNumber(fields[13], actionCount) || actionCount > 1u ||
				!ParseNumber(fields[14], requiresInteract) ||
				(0 != requiresInteract && 1 != requiresInteract) ||
				(placement.isEnabled && 1u != actionCount) ||
				!std::isfinite(placement.fHalfExtentX) ||
				!std::isfinite(placement.fHalfExtentY) ||
				!std::isfinite(placement.fHalfExtentZ) ||
				placement.fHalfExtentX <= 0.f || placement.fHalfExtentX > 1000.f ||
				placement.fHalfExtentY <= 0.f || placement.fHalfExtentY > 1000.f ||
				placement.fHalfExtentZ <= 0.f || placement.fHalfExtentZ > 1000.f)
			{
				m_strStatus = "World trigger shape is invalid at row " +
					std::to_string(index);
				return false;
			}
			placement.isTriggerOnce = 1 == triggerOnce;
			placement.requiresInteract = 1 == requiresInteract;
			std::size_t actionCursor = 15u;
			for (std::uint32_t actionIndex = 0; actionIndex < actionCount; ++actionIndex)
			{
				std::uint32_t payloadCount = 0;
				if (actionCursor + 2u > fields.size() ||
					!ParseNumber(fields[actionCursor + 1u], payloadCount) ||
					actionCursor + 2u + payloadCount > fields.size())
				{
					m_strStatus = "World trigger action payload is invalid at row " +
						std::to_string(index);
					return false;
				}
				// kind 뒤의 payloadCount가 지정한 필드 범위 안에서 action을 읽고, 끝에서 cursor가 행 끝과 같은지 확인한다.
				WORLD_TRIGGER_ACTION action{};
				if ("movePlayer" == fields[actionCursor])
				{
					// 기본 이동은 목적지/지속 시간(초)/arc 높이다. 6필드는 쿠크 도착 HUD mode, WALL_CLIMB는 발탄의 시간 표본을 추가한다.
					const bool wallClimbPayload = payloadCount >= 16u &&
						"WALL_CLIMB" == fields[actionCursor + 7u];
					if ((5u != payloadCount && 6u != payloadCount && !wallClimbPayload) ||
						!ParseNumber(fields[actionCursor + 2u], action.fTargetX) ||
						!ParseNumber(fields[actionCursor + 3u], action.fTargetY) ||
						!ParseNumber(fields[actionCursor + 4u], action.fTargetZ) ||
						!ParseNumber(fields[actionCursor + 5u], action.fDurationSeconds) ||
						!ParseNumber(fields[actionCursor + 6u], action.fArcHeight) ||
						!std::isfinite(action.fTargetX) || !std::isfinite(action.fTargetY) ||
						!std::isfinite(action.fTargetZ) ||
						!std::isfinite(action.fDurationSeconds) ||
						!std::isfinite(action.fArcHeight) ||
						std::abs(action.fTargetX) > 100000.f ||
						std::abs(action.fTargetY) > 100000.f ||
						std::abs(action.fTargetZ) > 100000.f ||
						action.fDurationSeconds < 0.05f ||
						action.fDurationSeconds > 10.f ||
						action.fArcHeight < 0.f || action.fArcHeight > 1000.f)
					{
						m_strStatus = "World movePlayer action is invalid at row " +
							std::to_string(index);
						return false;
					}
					if (wallClimbPayload)
					{
						std::uint32_t sampleCount = 0u;
						if (worldId != LostArk::Shared::WORLD_ID::VALTAN_ARENA ||
							!ParseNumber(fields[actionCursor + 8u], action.fFacingYawDegrees) ||
							!ParseNumber(fields[actionCursor + 9u], sampleCount) ||
							sampleCount < 2u || sampleCount > 181u ||
							payloadCount != 8u + 4u * sampleCount ||
							!std::isfinite(action.fFacingYawDegrees) ||
							std::abs(action.fFacingYawDegrees) > 360.f)
						{
							m_strStatus = "World movePlayer wall climb header is invalid";
							return false;
						}
						// 2~181개 표본은 0ms부터 엄격히 증가해야 한다. 위치는 유한성만 보고 각 표본의 절댓값 상한은 별도 검사하지 않는다.
						action.eMoveStyle = WORLD_TRIGGER_MOVE_STYLE::WALL_CLIMB;
						action.TrackSamples.reserve(sampleCount);
						std::uint32_t previousTimeMs = 0u;
						for (std::uint32_t sampleIndex = 0u; sampleIndex < sampleCount; ++sampleIndex)
						{
							WORLD_TRIGGER_MOVE_SAMPLE sample{};
							const std::size_t sampleCursor = actionCursor + 10u + 4u * sampleIndex;
							if (!ParseNumber(fields[sampleCursor], sample.iTimeMs) ||
								!ParseNumber(fields[sampleCursor + 1u], sample.fPositionX) ||
								!ParseNumber(fields[sampleCursor + 2u], sample.fPositionY) ||
								!ParseNumber(fields[sampleCursor + 3u], sample.fPositionZ) ||
								!std::isfinite(sample.fPositionX) || !std::isfinite(sample.fPositionY) ||
								!std::isfinite(sample.fPositionZ) ||
								(0u == sampleIndex ? sample.iTimeMs != 0u : sample.iTimeMs <= previousTimeMs))
							{
								m_strStatus = "World movePlayer wall climb sample is invalid";
								return false;
							}
							previousTimeMs = sample.iTimeMs;
							action.TrackSamples.push_back(sample);
						}
						// 마지막 ms를 초로 바꿔 duration과 0.001초 이내인지, 마지막 위치가 목표와 각 축 0.001 이내인지 대조한다.
						if (std::abs(action.fDurationSeconds - static_cast<float>(previousTimeMs) / 1000.f) > .001f)
						{
							m_strStatus = "World movePlayer wall climb duration is invalid";
							return false;
						}
						const WORLD_TRIGGER_MOVE_SAMPLE& lastSample = action.TrackSamples.back();
						if (std::abs(action.fTargetX - lastSample.fPositionX) > .001f ||
							std::abs(action.fTargetY - lastSample.fPositionY) > .001f ||
							std::abs(action.fTargetZ - lastSample.fPositionZ) > .001f)
						{
							m_strStatus = "World movePlayer wall climb target is invalid";
							return false;
						}
					}
					else if (6u == payloadCount)
					{
						const auto mode = fields[actionCursor + 7u];
						if (worldId != LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA ||
							(mode != "MARIO" && mode != "MAZE" && mode != "NONE"))
						{
							m_strStatus = "World movePlayer Kouku mode is invalid";
							return false;
						}
						action.eKoukuHudModeOnArrival = mode == "MARIO" ? LostArk::Shared::KOUKU_HUD_MODE::MARIO :
							(mode == "MAZE" ? LostArk::Shared::KOUKU_HUD_MODE::MAZE : LostArk::Shared::KOUKU_HUD_MODE::NONE);
					}
					action.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
				}
				else if ("changeLevel" == fields[actionCursor])
				{
					// 1필드는 목적 world, 2필드는 그 world에서 착지할 placement ID까지 저장한다. 여기서는 ID 형식만 확인한다.
					// 목적지 bootstrap의 실제 placement 존재/입장 가능 여부는 world 이동을 처리하는 room이 확인한다.
					if ((1u != payloadCount && 2u != payloadCount) ||
						!ParseTriggerTargetWorld(
							fields[actionCursor + 2u], action.eTargetWorldId) ||
						action.eTargetWorldId == worldId ||
						(2u == payloadCount && !IsStableId(fields[actionCursor + 3u])))
					{
						m_strStatus = "World changeLevel action is invalid at row " +
							std::to_string(index);
						return false;
					}
					if (2u == payloadCount)
						action.strTargetId = fields[actionCursor + 3u];
					action.eKind = WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL;
				}
				// 아래 target은 stable ID 형식만 읽는다. 실제 spawn group/encounter/sequence의 존재와 실행은 소비자 경계다.
				else if ("activateSpawnGroup" == fields[actionCursor])
				{
					if (1u != payloadCount ||
						!IsStableId(fields[actionCursor + 2u]))
					{
						m_strStatus = "World activateSpawnGroup action is invalid at row " +
							std::to_string(index);
						return false;
					}
					action.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
					action.strTargetId = fields[actionCursor + 2u];
				}
				else if ("activateEncounter" == fields[actionCursor])
				{
					if (1u != payloadCount ||
						!IsStableId(fields[actionCursor + 2u]))
					{
						m_strStatus = "World activateEncounter action is invalid at row " +
							std::to_string(index);
						return false;
					}
					action.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER;
					action.strTargetId = fields[actionCursor + 2u];
				}
				else if ("playSequence" == fields[actionCursor])
				{
					if (1u != payloadCount ||
						!IsStableId(fields[actionCursor + 2u]))
					{
						m_strStatus = "World playSequence action is invalid at row " +
							std::to_string(index);
						return false;
					}
					action.eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
					action.strTargetId = fields[actionCursor + 2u];
				}
				// 쿠크 world 전용 kind/target을 저장한다. 이 분기 자체는 requiresInteract와 target의 구체 역할을 검사하지 않는다.
				else if ("claimCardMazeTelescope" == fields[actionCursor])
				{
					if (1u != payloadCount ||
						!IsStableId(fields[actionCursor + 2u]) ||
						worldId != LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA)
					{
						m_strStatus = "World claimCardMazeTelescope action is invalid at row " +
							std::to_string(index);
						return false;
					}
					action.eKind = WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE;
					action.strTargetId = fields[actionCursor + 2u];
				}
				else
				{
					m_strStatus = "Unknown world trigger action at row " +
						std::to_string(index);
					return false;
				}
				placement.TriggerActions.push_back(action);
				actionCursor += 2u + payloadCount;
			}
			if (actionCursor != fields.size())
			{
				m_strStatus = "World trigger row has trailing action fields at row " +
					std::to_string(index);
				return false;
			}
		}
		else
		{
			if (WORLD_BOOTSTRAP_KIND::NPC != placement.eKind)
			{
				if (9u != fields.size())
				{
					m_strStatus = "World actor placement is invalid at row " +
						std::to_string(index);
					return false;
				}
			}
			else
			{
				int hasBehavior = 0;
				if (fields.size() < 10u ||
					!ParseNumber(fields[9], hasBehavior) ||
					(0 != hasBehavior && 1 != hasBehavior))
				{
					m_strStatus = "World NPC behavior presence is invalid at row " +
						std::to_string(index);
					return false;
				}
				placement.bHasNpcBehavior = 1 == hasBehavior;
				if (!placement.bHasNpcBehavior)
				{
					if (10u != fields.size())
					{
						m_strStatus = "Static world NPC has trailing behavior fields at row " +
							std::to_string(index);
						return false;
					}
				}
				else
				{
					// NPC 논리 설정: 이동 속도/반경, ms 단위 시작·대기 시간, waypoint와 action 목록을 한 행에서 읽는다.
					WORLD_NPC_BEHAVIOR_DESCRIPTOR& behavior =
						placement.NpcBehavior;
					std::uint32_t waypointCount = 0u;
					if (fields.size() < 22u ||
						!ParseNpcBehaviorMode(fields[10], behavior.eMode) ||
						!ParseNpcRouteMode(fields[11], behavior.eRouteMode) ||
						!ParseNpcActionSelection(
							fields[12], behavior.eActionSelection) ||
						!ParseNumber(fields[13], behavior.fMoveSpeed) ||
						!ParseNumber(fields[14], behavior.fWanderRadius) ||
						!ParseNumber(fields[15], behavior.iRandomSeed) ||
						!ParseNumber(fields[16], behavior.iStartDelayMs) ||
						!ParseNumber(fields[17], behavior.iIdleMinMs) ||
						!ParseNumber(fields[18], behavior.iIdleMaxMs) ||
						("-" != fields[19] && !IsStableId(fields[19])) ||
						!ParseNumber(fields[20], waypointCount) ||
						waypointCount > 64u ||
						!std::isfinite(behavior.fMoveSpeed) ||
						!std::isfinite(behavior.fWanderRadius) ||
						behavior.fMoveSpeed < 0.1f ||
						behavior.fMoveSpeed > 10.f ||
						behavior.fWanderRadius < 0.f ||
						behavior.fWanderRadius > 100.f ||
						0u == behavior.iRandomSeed ||
						behavior.iStartDelayMs > 600000u ||
						behavior.iIdleMinMs > 600000u ||
						behavior.iIdleMaxMs > 600000u ||
						behavior.iIdleMinMs > behavior.iIdleMaxMs)
					{
						m_strStatus = "World NPC behavior header is invalid at row " +
							std::to_string(index);
						return false;
					}
					behavior.strLookTargetPlacementId =
						"-" == fields[19] ? "" : std::string(fields[19]);
					// waypoint는 ID·XYZ·wait(ms)·yaw 존재·yaw(degree)의 7필드다. 개수와 ID 중복을 검사하며 순서는 그대로 보존한다.
					std::size_t cursor = 21u;
					std::unordered_set<std::string> waypointIds;
					behavior.Waypoints.reserve(waypointCount);
					for (std::uint32_t waypointIndex = 0u;
						waypointIndex < waypointCount; ++waypointIndex)
					{
						if (cursor + 7u > fields.size())
						{
							m_strStatus = "World NPC waypoint row is truncated at row " +
								std::to_string(index);
							return false;
						}
						WORLD_NPC_BEHAVIOR_WAYPOINT waypoint{};
						int hasLookYaw = 0;
						if (!IsStableId(fields[cursor]) ||
							!ParseNumber(fields[cursor + 1u], waypoint.fPositionX) ||
							!ParseNumber(fields[cursor + 2u], waypoint.fPositionY) ||
							!ParseNumber(fields[cursor + 3u], waypoint.fPositionZ) ||
							!ParseNumber(fields[cursor + 4u], waypoint.iWaitMs) ||
							!ParseNumber(fields[cursor + 5u], hasLookYaw) ||
							(0 != hasLookYaw && 1 != hasLookYaw) ||
							!ParseNumber(fields[cursor + 6u], waypoint.fLookYawDegrees) ||
							!std::isfinite(waypoint.fPositionX) ||
							!std::isfinite(waypoint.fPositionY) ||
							!std::isfinite(waypoint.fPositionZ) ||
							!std::isfinite(waypoint.fLookYawDegrees) ||
							(1 == hasLookYaw &&
								(waypoint.fLookYawDegrees < -360.f ||
								 waypoint.fLookYawDegrees > 360.f)) ||
							std::abs(waypoint.fPositionX) > 100000.f ||
							std::abs(waypoint.fPositionY) > 100000.f ||
							std::abs(waypoint.fPositionZ) > 100000.f ||
							waypoint.iWaitMs > 600000u)
						{
							m_strStatus = "World NPC waypoint is invalid at row " +
								std::to_string(index);
							return false;
						}
						waypoint.strWaypointId = fields[cursor];
						waypoint.bHasLookYaw = 1 == hasLookYaw;
						if (!waypoint.bHasLookYaw &&
							std::abs(waypoint.fLookYawDegrees) > 0.000001f)
						{
							m_strStatus = "World NPC waypoint absent yaw must be zero at row " +
								std::to_string(index);
							return false;
						}
						if (!waypointIds.insert(waypoint.strWaypointId).second)
						{
							m_strStatus = "Duplicate world NPC waypoint ID at row " +
								std::to_string(index);
							return false;
						}
						behavior.Waypoints.push_back(std::move(waypoint));
						cursor += 7u;
					}
					// waypoint 뒤 action 수를 읽고 ID·duration(ms)·waitAfter(ms)·weight 4필드씩 소비한다.
					std::uint32_t actionCount = 0u;
					if (cursor >= fields.size() ||
						!ParseNumber(fields[cursor], actionCount) ||
						actionCount > 32u)
					{
						m_strStatus = "World NPC action count is invalid at row " +
							std::to_string(index);
						return false;
					}
					++cursor;
					std::unordered_set<std::string> actionIds;
					behavior.Actions.reserve(actionCount);
					for (std::uint32_t actionIndex = 0u;
						actionIndex < actionCount; ++actionIndex)
					{
						if (cursor + 4u > fields.size())
						{
							m_strStatus = "World NPC action row is truncated at row " +
								std::to_string(index);
							return false;
						}
						WORLD_NPC_BEHAVIOR_ACTION action{};
						if (!IsStableId(fields[cursor]) ||
							!ParseNumber(fields[cursor + 1u], action.iDurationMs) ||
							!ParseNumber(fields[cursor + 2u], action.iWaitAfterMs) ||
							!ParseNumber(fields[cursor + 3u], action.iWeight) ||
							0u == action.iDurationMs ||
							action.iDurationMs > 600000u ||
							action.iWaitAfterMs > 600000u ||
							0u == action.iWeight || action.iWeight > 100000u)
						{
							m_strStatus = "World NPC action is invalid at row " +
								std::to_string(index);
							return false;
						}
						action.strActionId = fields[cursor];
						if ("npc.idle" == action.strActionId ||
							"npc.move.walk" == action.strActionId)
						{
							m_strStatus =
								"World NPC action ID is reserved by runtime at row " +
								std::to_string(index);
							return false;
						}
						if (!actionIds.insert(action.strActionId).second)
						{
							m_strStatus = "Duplicate world NPC action ID at row " +
								std::to_string(index);
							return false;
						}
						behavior.Actions.push_back(std::move(action));
						cursor += 4u;
					}
					// 모든 필드를 소비했는지와 stationary/patrol/wander별 waypoint·반경 조합을 확인한다. 실행 경로 탐색은 하지 않는다.
					if (cursor != fields.size() ||
						(NPC_BEHAVIOR_MODE::STATIONARY == behavior.eMode &&
							(!behavior.Waypoints.empty() ||
							 behavior.fWanderRadius != 0.f)) ||
						(NPC_BEHAVIOR_MODE::PATROL == behavior.eMode &&
							(behavior.Waypoints.size() < 2u ||
							 behavior.fWanderRadius != 0.f)) ||
						(NPC_BEHAVIOR_MODE::WANDER == behavior.eMode &&
							(!behavior.Waypoints.empty() ||
							 behavior.fWanderRadius < 0.5f)))
					{
						m_strStatus = "World NPC behavior shape is invalid at row " +
							std::to_string(index);
						return false;
					}
				}
			}
			placement.strArchetypeId =
				"-" == fields[2] ? "" : std::string(fields[2]);
			placement.strEncounterId = "-" == fields[3] ? "" : std::string(fields[3]);
		}
		// kind별 archetype/encounter 필수·금지 규칙과 placement ID 중복을 확인한 뒤 이 행을 후보에 확정한다.
		const bool isBoss = WORLD_BOOTSTRAP_KIND::BOSS == placement.eKind;
		const bool isPlayerSpawn =
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
		const bool isTrigger =
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind;
		const bool isCollision =
			WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind;
		if ((isPlayerSpawn && !placement.strArchetypeId.empty()) ||
			(!isPlayerSpawn && !isTrigger && !isCollision &&
				placement.strArchetypeId.empty()) ||
			((isTrigger || isCollision) && (!placement.strArchetypeId.empty() ||
				!placement.strEncounterId.empty())))
		{
			m_strStatus = "World bootstrap archetype contract is invalid at row " +
				std::to_string(index);
			return false;
		}
		if (isBoss && placement.strEncounterId.empty())
		{
			m_strStatus = "World bootstrap boss encounter is missing at row " +
				std::to_string(index);
			return false;
		}
		if (!placementIds.insert(placement.strPlacementId).second)
		{
			m_strStatus = "Duplicate world bootstrap placement ID";
			return false;
		}
		staged.push_back(std::move(placement));
	}
	// 3. 후속 sequence ID 목록을 읽는다. 아래 lane은 이 목록과 조인하지만 trigger의 모든 target을 여기서 조인하지는 않는다.
	std::vector<std::string> sequences;
	std::unordered_set<std::string> sequenceIds;
	for (std::uint32_t i = 0; i < sequenceCount; ++i)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "World sequence ID list is truncated";
			return false;
		}
		StripCarriageReturn(line);
		if (!IsStableId(line) || !sequenceIds.insert(line).second)
		{
			m_strStatus = "World sequence ID is invalid or duplicated";
			return false;
		}
		sequences.push_back(line);
	}
	// 4. 쿠크 행진 lane은 sequence ID를 참조하며 지연/지속 시간은 ms, 시작·끝 위치는 world 좌표다.
	std::vector<CARD_MAZE_MARCH_LANE> lanes;
	std::unordered_set<std::string> laneIds;
	for (std::uint32_t i = 0u; i < laneCount; ++i)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Card maze lanes truncated";
			return false;
		}
		StripCarriageReturn(line);
		const auto f = SplitTabs(line);
		CARD_MAZE_MARCH_LANE lane;
		if (worldId != WORLD_ID::KOUKUSAYDON_ARENA || f.size() != 9u ||
			!sequenceIds.contains(std::string(f[0])) || !laneIds.insert(std::string(f[0])).second ||
			!ParseNumber(f[1], lane.delayMs) || lane.delayMs > 120000u ||
			!ParseNumber(f[2], lane.durationMs) || lane.durationMs == 0u || lane.durationMs > 120000u ||
			!ParseNumber(f[3], lane.startX) || !ParseNumber(f[4], lane.startY) || !ParseNumber(f[5], lane.startZ) ||
			!ParseNumber(f[6], lane.endX) || !ParseNumber(f[7], lane.endY) || !ParseNumber(f[8], lane.endZ) ||
			!std::isfinite(lane.startX) || !std::isfinite(lane.startY) || !std::isfinite(lane.startZ) ||
			!std::isfinite(lane.endX) || !std::isfinite(lane.endY) || !std::isfinite(lane.endZ))
		{
			m_strStatus = "Card maze lane is invalid";
			return false;
		}
		lane.instanceId = f[0];
		lanes.push_back(std::move(lane));
	}
	// 5. 마리오 공은 stage/layout별 slot이 0부터 빈틈없이 들어오는지 확인한다. 그룹별 전체 개수를 채웠는지는 검사하지 않는다.
	std::vector<MARIO_SOURCE_BALL> balls;
	std::vector<std::uint8_t> ballSlots(4u * 3u, 0u);
	for (std::uint32_t i = 0u; i < ballCount; ++i)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Mario ball rows truncated";
			return false;
		}
		StripCarriageReturn(line);
		const auto f = SplitTabs(line);
		MARIO_SOURCE_BALL ball;
		// (stage, layout)별 0,1,2... slot은 Client binding과 popped mask bit를 연결한다. stage 4는 최대 12개, 나머지는 9개다.
		if (worldId != WORLD_ID::KOUKUSAYDON_ARENA || f.size() != 9u || "MARIOBALL" != f[0] ||
			!ParseNumber(f[1], ball.stage) || ball.stage < 1u || ball.stage > 4u ||
			!ParseNumber(f[2], ball.layout) || ball.layout < 1u || ball.layout > 3u ||
			!ParseNumber(f[3], ball.slot) || ball.slot != ballSlots[(ball.stage - 1u) * 3u + ball.layout - 1u] ||
			ball.slot >= (4u == ball.stage ? 12u : 9u) ||
			!ParseNumber(f[4], ball.color) || ball.color > 2u ||
			!ParseNumber(f[5], ball.placementId) || 0u == ball.placementId ||
			!ParseNumber(f[6], ball.x) || !ParseNumber(f[7], ball.y) || !ParseNumber(f[8], ball.z) ||
			!std::isfinite(ball.x) || !std::isfinite(ball.y) || !std::isfinite(ball.z))
		{
			m_strStatus = "Mario ball row is invalid";
			return false;
		}
		++ballSlots[(ball.stage - 1u) * 3u + ball.layout - 1u];
		balls.push_back(ball);
	}
	// 6. 바운싱 공은 placement ID를 중복 없이 저장하고 양수 주기(ms)/속도 및 엄격 증가 key로 아래 보간 함수의 전제를 만든다.
	std::vector<MARIO_BOUNCING_BALL> bouncingBalls;
	std::unordered_set<std::uint64_t> bouncingIds;
	for (std::uint32_t i = 0u; i < bouncingBallCount; ++i)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Mario bouncing balls truncated";
			return false;
		}
		StripCarriageReturn(line);
		const auto f = SplitTabs(line);
		MARIO_BOUNCING_BALL ball;
		std::uint32_t keyCount = 0u;
		if (worldId != WORLD_ID::KOUKUSAYDON_ARENA || f.size() < 9u || f[0] != "MARIOBOUNCE" ||
			!ParseNumber(f[1], ball.placementId) || !ball.placementId || !bouncingIds.insert(ball.placementId).second ||
			!ParseNumber(f[2], ball.x) || !ParseNumber(f[3], ball.y) || !ParseNumber(f[4], ball.z) ||
			!ParseNumber(f[5], ball.radius) || !ParseNumber(f[6], ball.durationMs) || !ParseNumber(f[7], ball.playbackSpeed) ||
			!std::isfinite(ball.x) || !std::isfinite(ball.y) || !std::isfinite(ball.z) ||
			!std::isfinite(ball.radius) || ball.radius <= 0.f || ball.radius > 10.f ||
			!std::isfinite(ball.durationMs) || ball.durationMs < 1.f || ball.durationMs > 120000.f ||
			!std::isfinite(ball.playbackSpeed) || ball.playbackSpeed < .1f || ball.playbackSpeed > 4.f ||
			!ParseNumber(f[8], keyCount) || keyCount < 2u || keyCount > 4096u || f.size() != 9u + 2u * keyCount)
		{
			m_strStatus = "Mario bouncing ball is invalid";
			return false;
		}
		for (std::uint32_t key = 0u; key < keyCount; ++key)
		{
			MARIO_BOUNCING_BALL::KEY value;
			if (!ParseNumber(f[9u + key * 2u], value.timeMs) || !ParseNumber(f[10u + key * 2u], value.offsetY) ||
				!std::isfinite(value.timeMs) || !std::isfinite(value.offsetY) || std::abs(value.offsetY) > 100.f ||
				value.timeMs < 0.f || value.timeMs > ball.durationMs ||
				(key && value.timeMs <= ball.keys.back().timeMs))
			{
				m_strStatus = "Mario bouncing ball key is invalid";
				return false;
			}
			ball.keys.push_back(value);
		}
		// 0과 duration의 두 끝 key가 있어야 하고 offset 차이는 0.0001 이내여야 한다. 주기 경계의 위치 연속성을 근사 허용한다.
		if (ball.keys.front().timeMs != 0.f || ball.keys.back().timeMs != ball.durationMs ||
			std::abs(ball.keys.front().offsetY - ball.keys.back().offsetY) > .0001f)
		{
			m_strStatus = "Mario bouncing ball loop is discontinuous";
			return false;
		}
		bouncingBalls.push_back(std::move(ball));
	}
	// 7. 선언한 모든 행 뒤에는 빈 행도 허용하지 않는다. getline 실패가 EOF인지 I/O 오류인지 별도로 구분하지 않는다.
	if (std::getline(input, line))
	{
		m_strStatus = "World bootstrap has trailing rows";
		return false;
	}
	// NPC 시선 대상은 모든 배치를 읽은 뒤 선형 검색한다. enabled NPC이며 자기 자신이 아니어야 한다.
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : staged)
	{
		if (!placement.bHasNpcBehavior ||
			placement.NpcBehavior.strLookTargetPlacementId.empty())
		{
			continue;
		}
		const auto target = std::find_if(
			staged.begin(), staged.end(),
			[&placement](const WORLD_BOOTSTRAP_PLACEMENT& candidate)
			{
				return candidate.strPlacementId ==
					placement.NpcBehavior.strLookTargetPlacementId;
			});
		if (target == staged.end() ||
			target->eKind != WORLD_BOOTSTRAP_KIND::NPC ||
			!target->isEnabled ||
			target->strPlacementId == placement.strPlacementId)
		{
			m_strStatus = "World NPC look target reference is invalid: " +
				placement.strPlacementId;
			return false;
		}
	}

	// 8. 모든 명시적 검증을 통과한 후보만 멤버로 교체한다. 아래 문자열 대입/상태 작성의 할당 예외까지 원자적인 교체는 아니다.
	m_Placements = std::move(staged);
	m_SequenceInstanceIds = std::move(sequences);
	m_CardMazeLanes = std::move(lanes);
	m_MarioBalls = std::move(balls);
	m_MarioBouncingBalls = std::move(bouncingBalls);
	m_strAreaId = stagedAreaId;
	m_iRevision = revision;
	m_strStatus = "Loaded world bootstrap: " +
		std::to_string(m_Placements.size()) + " placements";
	return true;
}

// Load가 검증한 2개 이상 증가 key/양수 duration·speed와 0 이상인 유한한 Server clock(ms)를 전제로 주기 내 Y offset을 구한다.
// 시간에 speed를 곱해 fmod로 위상을 만든 뒤 upper_bound가 처음으로 phase보다 큰 key를 찾는다.
// 탐색은 O(log key 수), 양끝 밖은 끝값을 쓰고 사이에서는 바로 이전/다음 key를 선형 보간한다. 전제를 여기서 재검사하지 않는다.
float LostArk::Server::MARIO_BOUNCING_BALL::Sample_OffsetY(const double clockMs) const
{
	const float phase = static_cast<float>(std::fmod(clockMs * playbackSpeed, durationMs));
	const auto next = std::upper_bound(keys.begin(), keys.end(), phase,
		[](float time, const KEY& key)
		{
			return time < key.timeMs;
		});
	if (next == keys.begin())
		return keys.front().offsetY;
	if (next == keys.end())
		return keys.back().offsetY;
	const auto& before = *std::prev(next);
	return before.offsetY + (next->offsetY - before.offsetY) *
		(phase - before.timeMs) / (next->timeMs - before.timeMs);
}

// CGameRoom의 Mario 충돌이 이전 tick~현재 tick의 Y 이동 범위를 얻는다. 유한한 fromMs<=toMs와 Sample_OffsetY의 key 전제를 따른다.
// low/high는 입력값을 누적하지 않고 새 최소/최대로 덮어쓴다. world Y는 소비자가 ball.y와 radius를 더해 구성한다.
void LostArk::Server::MARIO_BOUNCING_BALL::Sample_SweptOffsetY(
	const double fromMs, const double toMs, float& low, float& high) const
{
	const float first = Sample_OffsetY(fromMs),
		last = Sample_OffsetY(toMs);
	low = (std::min)(first, last);
	high = (std::max)(first, last);
	const double begin = fromMs * playbackSpeed,
		end = toMs * playbackSpeed;
	// 양끝만 보면 tick 사이의 빠른 낙하/반등을 놓칠 수 있어 구간 안의 key 극값과 주기 경계도 포함한다.
	// 각 key에서 begin 이상인 첫 주기 발생 시각을 ceil로 찾는다. 같은 key는 매 주기 같은 높이라 한 번 포함하면 충분하다.
	// key 사이가 선형이라는 전제에서 전체 key를 O(key 수) 순회하며, 플레이어의 연속 이동 궤적까지 푸는 함수는 아니다.
	for (const auto& key : keys)
	{
		const double occurrence = key.timeMs + std::ceil((begin - key.timeMs) / durationMs) * durationMs;
		if (occurrence < begin || occurrence > end)
			continue;
		low = (std::min)(low, key.offsetY);
		high = (std::max)(high, key.offsetY);
	}
}
