// 뒤따르는 엔진 공통 Windows 헤더보다 WinSock2 선언이 먼저 오도록 기존 포함 순서를 유지한다.
#include <WinSock2.h>
// process의 여섯 캐릭터 슬롯과 ID별 저장 상태의 조회·변경을 구현한다.
#include "CharacterRoster.h"

// 저장 전 Is_Valid_PlayerNickname으로 공유 닉네임 규칙을 검사한다.
#include "Network/PacketMessages.h"

// 채워진 슬롯 수를 세고 캐릭터 ID에 해당하는 항목을 찾는다.
#include <algorithm>

namespace
{
	// 정적 vector가 process 동안 슬롯을 소유한다. 이 파일은 생성 후 vector 크기를 바꾸지 않지만 내부 동시 접근 잠금은 없다.
	std::vector<Client::CHARACTER_ROSTER_ENTRY>& Roster()
	{
		// 각 EXE는 빈 슬롯 6개로 시작한다. 개인 파일을 읽거나 쓰거나 삭제하지 않는다.
		static std::vector<Client::CHARACTER_ROSTER_ENTRY> entries(Client::CCharacterRoster::MAX_CHARACTERS);
		return entries;
	}
}

// 선택창과 상태 관리자가 현재 슬롯을 조회한다. 값 snapshot이 아닌 참조이므로 후속 수정은 같은 원소에 반영된다.
const std::vector<Client::CHARACTER_ROSTER_ENTRY>& Client::CCharacterRoster::Get_Entries()
{
	return Roster();
}

// ID가 없는 기본 생성 슬롯을 제외해 실제 생성된 캐릭터 수를 센다.
size_t Client::CCharacterRoster::Get_CharacterCount()
{
	return static_cast<size_t>(std::count_if(Roster().begin(), Roster().end(),
		[](const auto& entry) { return !entry.strCharacterId.empty(); }));
}

// 범위를 먼저 검사해 잘못된 카드 index를 빈 슬롯과 같이 false로 처리한다.
bool_t Client::CCharacterRoster::Is_Occupied(const size_t slot)
{
	return slot < Roster().size() && !Roster()[slot].strCharacterId.empty();
}

// 선택창의 이름 변경이다. 검증 실패는 이름을 보존하고 status만 실패 문구로 바꾼다.
bool_t Client::CCharacterRoster::Rename(
	const size_t iIndex, const std::string& strNickname, std::string& outStatus)
{
	auto& entries = Roster();
	if (!Is_Occupied(iIndex))
	{
		outStatus = "No character in that slot.";
		return false;
	}
	if (!LostArk::Shared::Is_Valid_PlayerNickname(strNickname))
	{
		outStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
		return false;
	}
	entries[iIndex].strNickname = strNickname;
	outStatus = "Name changed for this session.";
	return true;
}

// SelectionState가 서버 응답 또는 마지막 HUD 수신값을 보관한다. ID/bValid만 검사하고 아이템/재화 규칙은 재검증하지 않는다.
bool_t Client::CCharacterRoster::Update_WorldState(
	const std::string& strCharacterId, const CHARACTER_WORLD_STATE& State, std::string& outStatus)
{
	if (strCharacterId.empty() || !State.bValid)
	{
		outStatus = "A valid character identity and authoritative state are required.";
		return false;
	}
	auto& entries = Roster();
	const auto found = std::find_if(entries.begin(), entries.end(),
		[&](const auto& entry) { return entry.strCharacterId == strCharacterId; });
	if (found == entries.end())
	{
		outStatus = "No session character matches.";
		return false;
	}
	found->World = State;
	found->World.bValid = true;
	outStatus = "World state kept for this session.";
	return true;
}

// 같은 local ID와 유효 표시를 찾은 뒤에만 복사한다. 못 찾으면 기존 출력은 그대로다.
bool_t Client::CCharacterRoster::Try_Get_WorldState(
	const std::string& strCharacterId, CHARACTER_WORLD_STATE& outState)
{
	if (strCharacterId.empty())
		return false;
	for (const auto& entry : Roster())
	{
		if (entry.strCharacterId == strCharacterId && entry.World.bValid)
		{
			outState = entry.World;
			return true;
		}
	}
	return false;
}

// Bern 입장 확정이 새 캐릭터를 요청한 슬롯에 추가한다. 모든 명시적 거절은 새 ID 증가와 슬롯 교체 전에 끝난다.
bool_t Client::CCharacterRoster::Add(
	const size_t slot, const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass, const std::string& strNickname,
	const std::string& strAppearanceJson, size_t& outIndex, std::string& outStatus)
{
	auto& entries = Roster();
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(eCharacterClass))
	{
		outStatus = "That class cannot be created.";
		return false;
	}
	if (!LostArk::Shared::Is_Valid_PlayerNickname(strNickname))
	{
		outStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
		return false;
	}
	if (slot >= MAX_CHARACTERS || Is_Occupied(slot))
	{
		outStatus = "The selected character slot is unavailable.";
		return false;
	}
	// 증가하는 process-session 번호를 사용한다. nickname, 정렬, 카드 index와 독립이며 process 재시작 뒤까지 유지되는 ID는 아니다.
	static uint64_t nextIdentity = 0u;
	CHARACTER_ROSTER_ENTRY created{ eCharacterClass, strNickname, strAppearanceJson };
	created.strCharacterId = "session.character." + std::to_string(++nextIdentity);
	outIndex = slot;
	entries[slot] = std::move(created);
	outStatus = "Character created for this session.";
	return true;
}
