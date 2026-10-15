#include "EncounterPropRuntime.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string_view>

namespace
{
	using LostArk::Shared::ENCOUNTER_PROP_STATE;
	using LostArk::Shared::WORLD_ID;

	// 소품 파일 이름에 허용되는 world만 변환한다. 미지원 world는 빈 view이며 loader가 false로 처리한다.
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
		default:
			return {};
		}
	}

	// 명시한 Server data root를 우선한다. 사용할 값이 없으면 EXE 상위 두 단계의 DataFiles를 사용한다.

	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
		                                                       configured,
		                                                       static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();
		wchar_t modulePath[32768]{};
		const DWORD moduleLength =
		    GetModuleFileNameW(nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() / L"DataFiles";
	}

	// line 메모리를 차용하는 탭 필드 view다. 다음 getline 전에 저장할 ID를 소유 string으로 복사한다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> result;
		std::string_view view(line);
		size_t start = 0;
		while (true)
		{
			const size_t tab = view.find('	', start);
			result.push_back(view.substr(start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return result;
	}

	// 필드 전체를 숫자로 해석했을 때만 성공한다. 접미사 오류로 false여도 outValue가 바뀔 수 있어 지역 후보에만 사용한다.
	template <typename T> bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto parsed = std::from_chars(value.data(), value.data() + value.size(), outValue);
		return std::errc{} == parsed.ec && parsed.ptr == value.data() + value.size();
	}

	// getline으로 읽은 행 끝의 CR 한 글자만 제거한다. 필드 내부의 공백이나 CR은 정규화하지 않는다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '' == line.back())
			line.pop_back();
	}

	// 같은 값 또는 uint32 반 주기보다 짧은 전진 차이를 허용한다. tick과 occurrence의 역행 검사에 공통 사용한다.
	bool Is_ForwardTick(const std::uint32_t candidate, const std::uint32_t reference)
	{
		if (candidate == reference)
			return true;
		const std::uint32_t forward = candidate - reference;
		return forward < ((std::numeric_limits<std::uint32_t>::max)() / 2u);
	}
}

// GameRoom이 게시된 World/<world>.encounterpropsbootstrap을 읽는다. 시작 시 outSets를 비우므로 false에서도 기존 출력은 보존되지 않는다.
// 지역 후보의 개수/참조를 확인한 뒤 결과를 옮긴다. 파일·경로·할당 예외와 스트림 종료 원인의 별도 검사는 제공하지 않는다.
bool LostArk::Server::Load_EncounterPropSets(const LostArk::Shared::WORLD_ID worldId,
                                             std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR>& outSets,
                                             std::string& status)
{
	outSets.clear();
	const std::string_view worldName = World_ToString(worldId);
	if (worldName.empty())
	{
		status = "Unknown encounter prop world ID";
		return false;
	}
	const std::filesystem::path path =
	    Resolve_DataRoot() / L"World" /
	    std::filesystem::path(std::string(worldName) + ".encounterpropsbootstrap");
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		// 파일이 없는 world를 허용한다. 현재 조건은 부재뿐 아니라 다른 열기 실패도 같은 빈 목록 성공으로 처리한다.
		status = "No encounter props for world";
		return true;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		status = "Empty encounter prop bootstrap";
		return false;
	}
	StripCarriageReturn(line);
	const auto header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t revision = 0u;
	std::uint32_t setCount = 0u;
	std::uint32_t slotCount = 0u;
	// 버전/양수 revision/집합·슬롯 개수를 확인한다. header의 area 필드는 저장하지 않으며 revision도 반환 정의에는 남기지 않는다.
	if (7u != header.size() || "LOSTARK_ENCOUNTER_PROP_BOOTSTRAP" != header[0] ||
	    !ParseNumber(header[1], version) || 1u != version || header[2] != worldName ||
	    !ParseNumber(header[4], revision) || 0u == revision || !ParseNumber(header[5], setCount) ||
	    0u == setCount || setCount > 8u || !ParseNumber(header[6], slotCount) || 0u == slotCount ||
	    slotCount > setCount * LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS)
	{
		status = "Encounter prop bootstrap header is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR> staged;
	std::uint32_t observedSlots = 0u;
	while (std::getline(input, line))
	{
		StripCarriageReturn(line);
		if (line.empty())
			continue;
		const auto fields = SplitTabs(line);
		// PROPSET이 먼저 등장해야 이후 PROPSLOT이 소유 집합을 찾을 수 있다. 집합 ID 중복은 여기서, 슬롯 ID 중복은 Initialize에서 거른다.
		if (!fields.empty() && "PROPSET" == fields[0])
		{
			ENCOUNTER_PROP_SET_DESCRIPTOR set{};
			if (5u != fields.size() || fields[1].empty() || fields[2].empty() || fields[3].empty() ||
			    !ParseNumber(fields[4], set.fCoverRadiusMeters) || !std::isfinite(set.fCoverRadiusMeters) ||
			    set.fCoverRadiusMeters <= 0.f)
			{
				status = "Encounter prop set row is invalid";
				return false;
			}
			set.strPropSetId = std::string(fields[1]);
			set.strEncounterId = std::string(fields[2]);
			if (std::any_of(staged.begin(),
			                staged.end(),
			                [&set](const ENCOUNTER_PROP_SET_DESCRIPTOR& existing)
			                {
				                return existing.strPropSetId == set.strPropSetId;
			                }))
			{
				status = "Duplicate encounter prop set: " + set.strPropSetId;
				return false;
			}
			staged.push_back(std::move(set));
		}
		else if (!fields.empty() && "PROPSLOT" == fields[0])
		{
			ENCOUNTER_PROP_SLOT_DESCRIPTOR slot{};
			if (5u != fields.size() || fields[1].empty() || fields[2].empty() ||
			    !ParseNumber(fields[3], slot.fPositionX) || !ParseNumber(fields[4], slot.fPositionZ) ||
			    !std::isfinite(slot.fPositionX) || !std::isfinite(slot.fPositionZ))
			{
				status = "Encounter prop slot row is invalid";
				return false;
			}
			slot.strSlotId = std::string(fields[2]);
			const auto owner = std::find_if(staged.begin(),
			                                staged.end(),
			                                [&fields](const ENCOUNTER_PROP_SET_DESCRIPTOR& candidate)
			                                {
				                                return candidate.strPropSetId == fields[1];
			                                });
			if (staged.end() == owner || owner->Slots.size() >= LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS)
			{
				status = "Encounter prop slot has no set owner: " + slot.strSlotId;
				return false;
			}
			owner->Slots.push_back(std::move(slot));
			++observedSlots;
		}
		else
		{
			status = "Unknown encounter prop bootstrap row";
			return false;
		}
	}
	if (staged.size() != setCount || observedSlots != slotCount ||
	    std::any_of(staged.begin(),
	                staged.end(),
	                [](const ENCOUNTER_PROP_SET_DESCRIPTOR& set)
	                {
		                return set.Slots.empty();
	                }))
	{
		status = "Encounter prop bootstrap counts do not match its header";
		return false;
	}
	outSets = std::move(staged);
	status = "Encounter prop bootstrap loaded";
	return true;
}

// 로드한 정의를 Room의 live 상태로 만든다. 유효성 검사를 지역 슬롯 목록에서 마친 뒤 멤버를 교체한다.
bool LostArk::Server::CEncounterPropRuntime::Initialize(const ENCOUNTER_PROP_SET_DESCRIPTOR& descriptor,
                                                        std::string& status,
                                                        const std::uint32_t initialServerTick)
{
	if (descriptor.strPropSetId.empty() || descriptor.strEncounterId.empty() || descriptor.Slots.empty() ||
	    descriptor.Slots.size() > LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS ||
	    !std::isfinite(descriptor.fCoverRadiusMeters) || descriptor.fCoverRadiusMeters <= 0.f ||
	    0u == initialServerTick)
	{
		status = "Encounter prop descriptor is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PROP_SLOT_STATE> staged;
	staged.reserve(descriptor.Slots.size());
	for (const ENCOUNTER_PROP_SLOT_DESCRIPTOR& slot : descriptor.Slots)
	{
		const std::string& slotId = slot.strSlotId;
		if (slotId.empty() || !std::isfinite(slot.fPositionX) || !std::isfinite(slot.fPositionZ))
		{
			status = "Encounter prop slot identity is invalid";
			return false;
		}
		if (std::any_of(staged.begin(),
		                staged.end(),
		                [&slotId](const ENCOUNTER_PROP_SLOT_STATE& existing)
		                {
			                return existing.strSlotId == slotId;
		                }))
		{
			status = "Duplicate encounter prop slot: " + slotId;
			return false;
		}
		ENCOUNTER_PROP_SLOT_STATE state{};
		state.strSlotId = slotId;
		state.eState = ENCOUNTER_PROP_STATE::HIDDEN;
		state.iStateVersion = 1u;
		state.iStateStartTick = initialServerTick;
		state.iOccurrenceSequence = 0u;
		state.fPositionX = slot.fPositionX;
		state.fPositionZ = slot.fPositionZ;
		staged.push_back(std::move(state));
	}
	// 복제할 슬롯 순서를 stable ID 오름차순으로 고정한다. 정의의 원래 vector 순서를 실행 식별자로 사용하지 않는다.
	std::sort(staged.begin(),
	          staged.end(),
	          [](const ENCOUNTER_PROP_SLOT_STATE& left, const ENCOUNTER_PROP_SLOT_STATE& right)
	          {
		          return left.strSlotId < right.strSlotId;
	          });

	m_Descriptor = descriptor;
	m_Slots = std::move(staged);
	m_iEncounterEpoch = 1u;
	m_iOccurrenceSequence = 0u;
	m_bInitialized = true;
	status = "Encounter prop runtime initialized";
	return true;
}

// GameRoom의 전투 정리가 슬롯 version을 1, occurrence를 0으로 초기화하고 epoch를 올려 이전 준비 결과와 구분한다.
bool LostArk::Server::CEncounterPropRuntime::Reset(std::string& status, const std::uint32_t resetServerTick)
{
	if (!m_bInitialized || 0u == resetServerTick)
	{
		status = "Encounter prop runtime is not initialized";
		return false;
	}
	for (ENCOUNTER_PROP_SLOT_STATE& slot : m_Slots)
	{
		slot.eState = ENCOUNTER_PROP_STATE::HIDDEN;
		slot.iStateVersion = 1u;
		slot.iStateStartTick = resetServerTick;
		slot.iOccurrenceSequence = 0u;
	}
	m_iEncounterEpoch =
	    (std::numeric_limits<std::uint32_t>::max)() == m_iEncounterEpoch ? 1u : m_iEncounterEpoch + 1u;
	m_iOccurrenceSequence = 0u;
	status = "Encounter prop runtime reset";
	return true;
}

// 소환/전체 파괴가 공유하는 준비 단계다. 출력 transaction만 만들며 현재 슬롯/회차는 Commit 전까지 바꾸지 않는다.
LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT LostArk::Server::CEncounterPropRuntime::Prepare_Transition(
    const ENCOUNTER_PROP_STATE fromState,
    const ENCOUNTER_PROP_STATE toState,
    const std::uint32_t occurrenceSequence,
    const std::uint32_t serverTick,
    ENCOUNTER_PROP_TRANSACTION& transaction,
    std::string& status) const
{
	transaction = ENCOUNTER_PROP_TRANSACTION{};
	if (!m_bInitialized || 0u == serverTick || 0u == occurrenceSequence)
	{
		status = "Encounter prop transition input is invalid";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}
	if (!Is_ForwardTick(occurrenceSequence, m_iOccurrenceSequence))
	{
		status = "Encounter prop occurrence sequence went backwards";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}

	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iCommitTick = serverTick;
	transaction.iOccurrenceSequence = occurrenceSequence;
	for (const ENCOUNTER_PROP_SLOT_STATE& slot : m_Slots)
	{
		// 같은 회차에서 목적 상태에 이미 도달한 슬롯은 중복 후보를 만들지 않는다.
		if (slot.eState == toState && slot.iOccurrenceSequence == occurrenceSequence)
		{
			continue;
		}
		if (slot.eState != fromState)
		{
			status = "Encounter prop slot is not in the expected state: " + slot.strSlotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		// 변경 슬롯의 다음 version/시작 tick/회차를 기록한다. version 최댓값에서는 여기서 1을 만들지만 Commit의 +1 검사와는 별도다.
		ENCOUNTER_PROP_SLOT_STATE next = slot;
		next.eState = toState;
		next.iStateVersion = (std::numeric_limits<std::uint32_t>::max)() == slot.iStateVersion
		                         ? 1u
		                         : slot.iStateVersion + 1u;
		next.iStateStartTick = serverTick;
		next.iOccurrenceSequence = occurrenceSequence;
		transaction.Slots.push_back(std::move(next));
	}
	if (transaction.Slots.empty())
	{
		status = "Encounter prop transition is already applied";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	status = "Encounter prop transition prepared";
	return ENCOUNTER_PROP_PREPARE_RESULT::READY;
}

// 현재 회차가 0이 아닐 때 그 이하 요청은 NO_CHANGE로 끝낸다. 새 회차는 모든 슬롯이 HIDDEN일 때만 INTACT 전이를 준비할 수 있다.
LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT LostArk::Server::CEncounterPropRuntime::Prepare_Spawn(
    const std::uint32_t occurrenceSequence,
    const std::uint32_t serverTick,
    ENCOUNTER_PROP_TRANSACTION& transaction,
    std::string& status) const
{
	// 회차의 일반 정수 비교가 현재 이하 요청을 먼저 건너뛴다. 이후 Prepare_Transition이 tick식 전진 검사도 수행한다.
	if (0u != m_iOccurrenceSequence && occurrenceSequence <= m_iOccurrenceSequence)
	{
		transaction = ENCOUNTER_PROP_TRANSACTION{};
		status = "Encounter prop spawn repeated an occurrence";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	return Prepare_Transition(ENCOUNTER_PROP_STATE::HIDDEN,
	                          ENCOUNTER_PROP_STATE::INTACT,
	                          occurrenceSequence,
	                          serverTick,
	                          transaction,
	                          status);
}

// 현재 호출자가 전달한 회차로 INTACT -> BREAKING을 준비한다. 동일 회차에서 이미 BREAKING인 슬롯은 중복 후보로 만들지 않는다.
LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT LostArk::Server::CEncounterPropRuntime::Prepare_Break(
    const std::uint32_t occurrenceSequence,
    const std::uint32_t serverTick,
    ENCOUNTER_PROP_TRANSACTION& transaction,
    std::string& status) const
{
	return Prepare_Transition(ENCOUNTER_PROP_STATE::INTACT,
	                          ENCOUNTER_PROP_STATE::BREAKING,
	                          occurrenceSequence,
	                          serverTick,
	                          transaction,
	                          status);
}

// Room의 stage가 지정한 stable slot ID만 파괴 후보로 만든다. 요청 중복/누락/숨겨진 슬롯이면 live는 그대로 두고 거절한다.
LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT LostArk::Server::CEncounterPropRuntime::Prepare_BreakSlots(
    const std::vector<std::string>& slotIds,
    const std::uint32_t occurrenceSequence,
    const std::uint32_t serverTick,
    ENCOUNTER_PROP_TRANSACTION& transaction,
    std::string& status) const
{
	transaction = ENCOUNTER_PROP_TRANSACTION{};
	if (!m_bInitialized || 0u == serverTick || 0u == occurrenceSequence || slotIds.empty())
	{
		status = "Encounter prop partial break input is invalid";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}
	if (!Is_ForwardTick(occurrenceSequence, m_iOccurrenceSequence))
	{
		status = "Encounter prop occurrence sequence went backwards";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}

	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iCommitTick = serverTick;
	transaction.iOccurrenceSequence = occurrenceSequence;
	for (const std::string& slotId : slotIds)
	{
		// 한 요청에서 같은 slot ID를 두 번 지목하면 거절한다. 하나의 사건에 같은 슬롯 후보가 중복되는 것을 막는다.
		const bool duplicated = std::count(slotIds.begin(), slotIds.end(), slotId) > 1;
		if (duplicated)
		{
			status = "Encounter prop partial break names a slot twice: " + slotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		const auto found = std::find_if(m_Slots.begin(),
		                                m_Slots.end(),
		                                [&slotId](const ENCOUNTER_PROP_SLOT_STATE& candidate)
		                                {
			                                return candidate.strSlotId == slotId;
		                                });
		if (m_Slots.end() == found)
		{
			status = "Encounter prop partial break names an unknown slot: " + slotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		// 같은 회차에서 이미 BREAKING이면 이 요청에 다시 넣지 않는다. HIDDEN까지 제거된 슬롯은 이 중복 조건에 해당하지 않는다.
		if (ENCOUNTER_PROP_STATE::BREAKING == found->eState &&
		    found->iOccurrenceSequence == occurrenceSequence)
		{
			continue;
		}
		if (ENCOUNTER_PROP_STATE::INTACT != found->eState)
		{
			status = "Encounter prop slot is not raised: " + slotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		ENCOUNTER_PROP_SLOT_STATE next = *found;
		next.eState = ENCOUNTER_PROP_STATE::BREAKING;
		next.iStateVersion = (std::numeric_limits<std::uint32_t>::max)() == found->iStateVersion
		                         ? 1u
		                         : found->iStateVersion + 1u;
		next.iStateStartTick = serverTick;
		next.iOccurrenceSequence = occurrenceSequence;
		transaction.Slots.push_back(std::move(next));
	}
	if (transaction.Slots.empty())
	{
		status = "Encounter prop partial break is already applied";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	status = "Encounter prop partial break prepared";
	return ENCOUNTER_PROP_PREPARE_RESULT::READY;
}

// BREAKING 시작부터 breakingTicks가 지난 슬롯을 HIDDEN으로 준비한다. 기간의 단위는 ms가 아니라 서버 tick이다.
LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT LostArk::Server::CEncounterPropRuntime::Prepare_DueRemoval(
    const std::uint32_t serverTick,
    const std::uint32_t breakingTicks,
    ENCOUNTER_PROP_TRANSACTION& transaction,
    std::string& status) const
{
	transaction = ENCOUNTER_PROP_TRANSACTION{};
	if (!m_bInitialized || 0u == serverTick)
	{
		status = "Encounter prop removal input is invalid";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}
	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iCommitTick = serverTick;
	transaction.iOccurrenceSequence = m_iOccurrenceSequence;
	for (const ENCOUNTER_PROP_SLOT_STATE& slot : m_Slots)
	{
		if (ENCOUNTER_PROP_STATE::BREAKING != slot.eState)
			continue;
		const std::uint32_t elapsed = serverTick - slot.iStateStartTick;
		if (!Is_ForwardTick(serverTick, slot.iStateStartTick) || elapsed < breakingTicks)
		{
			continue;
		}
		ENCOUNTER_PROP_SLOT_STATE next = slot;
		next.eState = ENCOUNTER_PROP_STATE::HIDDEN;
		next.iStateVersion = (std::numeric_limits<std::uint32_t>::max)() == slot.iStateVersion
		                         ? 1u
		                         : slot.iStateVersion + 1u;
		next.iStateStartTick = serverTick;
		transaction.Slots.push_back(std::move(next));
	}
	if (transaction.Slots.empty())
	{
		status = "No encounter prop removal is due";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	status = "Encounter prop removal prepared";
	return ENCOUNTER_PROP_PREPARE_RESULT::READY;
}

// 준비 후 live version이 달라졌는지 모든 변경 슬롯을 확인한다. Prepare가 만든 후보라는 전제 아래 확인 성공 후 일괄 대입한다.
bool LostArk::Server::CEncounterPropRuntime::Commit(const ENCOUNTER_PROP_TRANSACTION& transaction,
                                                    std::string& status)
{
	if (!m_bInitialized || transaction.Slots.empty() || transaction.iEncounterEpoch != m_iEncounterEpoch ||
	    0u == transaction.iCommitTick)
	{
		status = "Encounter prop transaction does not match the live runtime";
		return false;
	}
	// 모든 변경 슬롯의 ID와 다음 version을 먼저 검사한다. 하나라도 달라졌으면 live에 대입하기 전에 false로 끝낸다.
	for (const ENCOUNTER_PROP_SLOT_STATE& staged : transaction.Slots)
	{
		const auto live = std::find_if(m_Slots.begin(),
		                               m_Slots.end(),
		                               [&staged](const ENCOUNTER_PROP_SLOT_STATE& slot)
		                               {
			                               return slot.strSlotId == staged.strSlotId;
		                               });
		// 이 +1 비교는 uint32 덧셈 그대로다. Prepare의 최댓값 -> 1 규칙을 별도로 승인하는 분기는 없다.
		if (m_Slots.end() == live || staged.iStateVersion != live->iStateVersion + 1u)
		{
			status = "Encounter prop slot changed before commit: " + staged.strSlotId;
			return false;
		}
	}
	// 명시적 실패 검사는 끝났지만 string 대입 예외까지 복원하는 transaction은 아니다. 실제 방송은 호출자가 성공 뒤 수행한다.
	for (const ENCOUNTER_PROP_SLOT_STATE& staged : transaction.Slots)
	{
		const auto live = std::find_if(m_Slots.begin(),
		                               m_Slots.end(),
		                               [&staged](const ENCOUNTER_PROP_SLOT_STATE& slot)
		                               {
			                               return slot.strSlotId == staged.strSlotId;
		                               });
		*live = staged;
	}
	if (0u != transaction.iOccurrenceSequence)
		m_iOccurrenceSequence = transaction.iOccurrenceSequence;
	status = "Encounter prop transaction committed";
	return true;
}
