#include "NetObjectRegistry.h"

#include "Character.h"

#include <cstdint>

bool Client::CNetObjectRegistry::Register(
	const NET_PLAYER_RECORD& record,
	const std::shared_ptr<CCharacter>& character,
	OBJECT_HANDLE& outHandle)
{
	//새 character를 registry에 등록한다.
	//ID, Class, Nickname, Character 검증 -> 동일 netentityid 중복 검사
	//free slot이 있으면 재사용 -> 없으면 새 Slot 생성
	//record와 weak_ptr 저장 -> netentityid->objecthandle 인덱스 등록
	//생성된 handle 반환
	using namespace LostArk::Shared;

	const std::uint8_t rawClass = static_cast<std::uint8_t>(record.eCharacterClass);
	//id class nickname character 검증
	if (record.iPlayerId == INVALID_PLAYER_ID ||
		record.iNetEntityId == INVALID_NET_ENTITY_ID ||
		rawClass >= static_cast<std::uint8_t>(
			CHARACTER_CLASS_ID::END) ||
		record.strNickName.empty() ||
		nullptr == character ||
		m_HandleByEntityId.contains(record.iNetEntityId))
	{
		return false;
	}

	std::uint32_t slotIndex = 0;
	//free slot이 있으면 재사용,
	if (!m_FreeSlotIndices.empty())
	{
		//인덱스의 가장 뒤에 있는 slot index 반환해서 사용
		slotIndex = m_FreeSlotIndices.back();
		//사용했으므로 pop_back
		m_FreeSlotIndices.pop_back();
	}
	//free slot 없으면 새로 생성해서 사용
	else
	{
		// 현재 size는 다음에 추가될 Slot의 0-based index와 같다.
		// 예: Slot이 3개면 기존 index는 0, 1, 2이고 새 Slot index는 3이다.
		slotIndex = static_cast<std::uint32_t>(m_Slots.size());
		// 기본 생성된 SLOT 하나를 vector 뒤에 추가한다.
		m_Slots.emplace_back();
	}
	//조건문을 통해서 얻은 slotindex를 바탕으로, occupy, record, character를 채워넣는다.
	SLOT& slot = m_Slots[slotIndex];
	slot.isOccupied = true;
	slot.record = record;
	slot.pCharacter = character;
	//Handle 생성과 할당
	OBJECT_HANDLE handle{};
	handle.iSlotIndex = slotIndex;
	handle.iGeneration = slot.iGeneration;

	const auto [iter, inserted] =
		m_HandleByEntityId.emplace(
			record.iNetEntityId,
			handle);
	// Register 시작에서 중복 ID를 거르므로 보통 성공한다.
	// false면 예상하지 못한 중복 삽입이므로 Slot 변경을 rollback한다.
	(void)iter;
	// inserted == false는 같은 NetEntityId가 map에 이미 존재한다는 뜻이다.
	if (!inserted)
	{
		slot.isOccupied = false;
		slot.record = {};
		slot.pCharacter.reset();
		m_FreeSlotIndices.push_back(slotIndex);
		return false;
	}
	//handle 값 채워주기
	outHandle = handle;
	return true;
}

bool Client::CNetObjectRegistry::Find_Handle(
	LostArk::Shared::NET_ENTITY_ID netEntityId,
	OBJECT_HANDLE& outHandle) const
{
	//server entity id를 이용해 현재 handle을 찾는다.
	//NetEntityId -> unordered_map 검색 -> ObjectHandle 반환
	const auto iter = m_HandleByEntityId.find(netEntityId);

	if (iter == m_HandleByEntityId.end())
		return false;

	outHandle = iter->second;

	return true;
}

bool Client::CNetObjectRegistry::Replace(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const NET_PLAYER_RECORD& record,
	const std::shared_ptr<CCharacter>& character,
	OBJECT_HANDLE& outHandle)
{
	using namespace LostArk::Shared;
	OBJECT_HANDLE handle{};
	if (INVALID_NET_ENTITY_ID == netEntityId ||
		record.iNetEntityId != netEntityId ||
		INVALID_PLAYER_ID == record.iPlayerId ||
		!Is_Supported_Playable_Character_Class(record.eCharacterClass) ||
		record.strNickName.empty() || nullptr == character ||
		!Find_Handle(netEntityId, handle) ||
		handle.iSlotIndex >= m_Slots.size())
	{
		return false;
	}
	SLOT& slot = m_Slots[handle.iSlotIndex];
	if (!slot.isOccupied || slot.iGeneration != handle.iGeneration ||
		slot.pCharacter.expired())
	{
		return false;
	}
	slot.record = record;
	slot.pCharacter = character;
	outHandle = handle;
	return true;
}

const NET_PLAYER_RECORD* Client::CNetObjectRegistry::Find_Record(
	LostArk::Shared::NET_ENTITY_ID netEntityId) const
{
	//동일 entity의 중복 spawn을 검사할 떄 사용한다.
	//동일한 내용의 중복 spawn : 이미 처리된 이벤트 이므로 no-op
	//같은 entityid인데 내용이 다르다 : protocol conflict
	OBJECT_HANDLE handle{};
	//handle을 찾을 수 없거나, 슬롯 인덱스가 전체 슬롯 사이즈보다 더 큰 경우
	if (!Find_Handle(netEntityId, handle) ||
		handle.iSlotIndex >= m_Slots.size())
	{
		return nullptr;
	}

	const SLOT& slot = m_Slots[handle.iSlotIndex];

	if (!slot.isOccupied ||
		slot.iGeneration != handle.iGeneration)
	{
		return nullptr;
	}

	return &slot.record;
}

std::shared_ptr<CCharacter> Client::CNetObjectRegistry::Resolve(OBJECT_HANDLE handle) const
{
	//Handle이 현재 살아 있는 객체를 가리키는지 확인한다.
	//Handle 유효성 -> Slot 범위 검사 -> occupied 검사 -> generation 일치 검사
	//weak_ptr lock -> shared_ptr 반환
	if (!handle.Is_Valid() ||
		handle.iSlotIndex >= m_Slots.size())
	{
		return nullptr;
	}

	const SLOT& slot = m_Slots[handle.iSlotIndex];

	if (!slot.isOccupied ||
		slot.iGeneration != handle.iGeneration)
	{
		return nullptr;
	}

	return slot.pCharacter.lock();
}

bool Client::CNetObjectRegistry::Unregister(
	LostArk::Shared::NET_ENTITY_ID netEntityId, std::shared_ptr<CCharacter>* outCharacter)
{
	//netentityid로 handle 검색 -> handle의 slotindex와 generation검증
	//slot이 occupied 상태인지 확인 -> 필요하면 weak_ptr 결과를 outcharacter 반환
	//slot의 record 제거 -> weak_ptr reset -> occupied false
	//generation 증가 -> slot index를 free slot 목록에 반환
	//netentityid 인덱스 제거

	const auto iter = m_HandleByEntityId.find(netEntityId);

	if (iter == m_HandleByEntityId.end())
		return false;

	const OBJECT_HANDLE handle = iter->second;

	if (!handle.Is_Valid() ||
		handle.iSlotIndex >= m_Slots.size())
	{
		return false;
	}

	SLOT& slot = m_Slots[handle.iSlotIndex];

	if (!slot.isOccupied ||
		slot.iGeneration != handle.iGeneration)
	{
		return false;
	}

	if (nullptr != outCharacter)
		*outCharacter = slot.pCharacter.lock();

	slot.isOccupied = false;
	slot.record = {};
	slot.pCharacter.reset();
	Advance_Generation(slot);

	m_FreeSlotIndices.push_back(handle.iSlotIndex);
	m_HandleByEntityId.erase(iter);

	return true;
}

std::vector<std::shared_ptr<CCharacter>> Client::CNetObjectRegistry::Get_LiveObjects() const
{
	// 호출자가 순회하는 동안 Character가 소멸하지 않도록 shared_ptr 묶음으로 반환한다.
	// Registry 자체는 weak_ptr만 보관하므로 객체 수명을 영구적으로 연장하지 않는다.
	std::vector<std::shared_ptr<CCharacter>> objects;
	objects.reserve(m_HandleByEntityId.size());

	for (const SLOT& slot : m_Slots)
	{
		if (!slot.isOccupied)
			continue;
		// lock()은 객체가 살아 있으면 임시 shared_ptr을, 이미 삭제됐으면 nullptr을 반환한다.
		if (std::shared_ptr<CCharacter> character = slot.pCharacter.lock())
		{
			objects.push_back(std::move(character));
		}
	}

	return objects;
}

void Client::CNetObjectRegistry::Reset()
{
	//접속이 끊기거나 월드 상태를 비울 때 모든 slot을 무효화한다.
	m_HandleByEntityId.clear();
	m_FreeSlotIndices.clear();
	//slot clear하고 다시 reserve로 할당
	m_FreeSlotIndices.reserve(m_Slots.size());
	//slot for문으로 돌면서, slot 비워주기
	for (std::uint32_t index = 0;
		index < static_cast<std::uint32_t>(m_Slots.size()); ++index)
	{
		SLOT& slot = m_Slots[index];
		slot.isOccupied = false;
		slot.record = {};
		slot.pCharacter.reset();
		Advance_Generation(slot);
		m_FreeSlotIndices.push_back(index);
	}
}

void Client::CNetObjectRegistry::Advance_Generation(SLOT & slot)
{
	++slot.iGeneration;

	if (0 == slot.iGeneration)
		++slot.iGeneration;
}
