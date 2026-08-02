#pragma once

#include "Network/NetworkIds.h"
#include "Network/PacketType.h"

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

//server의 netentityid와 client engine의 ccharcter 수명을 안전하게 연결하기 위해 존재한다
//단순하게 unordered_map으로 shared_ptr로 저장하지 않는 이유는, registry가 shared_ptr을
//장기 보관하면, Layer에서 Character를 삭제해도, registry 때문에 객체가 계속 살아남을 수 있다.
//따라서 Registry는 weak_ptr만 보관

// NetEntityId는 Server와 Client가 공유하는 네트워크 객체 식별자다.
// OBJECT_HANDLE은 Client 내부에서만 사용하며 Server로 전송하지 않는다.
// Registry가 NetEntityId를 slot index + generation Handle로 변환하여,
// 삭제된 Handle이 재사용된 다른 CCharacter를 가리키지 않게 한다.

namespace Client
{
	//character 전방 선언
	class CCharacter;
	//slot index와 generation으로 객체를 식별
	//character - a slot - 0, generation - 1 삭제시 generation 2로 증가
	//character - b가 slot - 0 재사용, generation이 다르기 떄문에 A의 handle이
	//B를 가리킬 수 없다.
	struct OBJECT_HANDLE
	{
		static constexpr std::uint32_t INVALID_SLOT =
			(std::numeric_limits<std::uint32_t>::max());
		//slot과 generation 두 개를 통해서, 객체의 수명을 관리
		std::uint32_t iSlotIndex = INVALID_SLOT;
		std::uint32_t iGeneration = 0;

		[[nodiscard]] bool Is_Valid() const
		{
			return iSlotIndex != INVALID_SLOT &&
				iGeneration != 0;
		}
	};

	//서버가 알려준 player의 표면 정보를 보관한다.
	//playerid, netentityid, class, nickname, spawn position, yaw
	//해당 record는 engine 객체가 아니라, 네트워크 상태의 client 측 기록이다.
	struct NET_PLAYER_RECORD
	{
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;

		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;

		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;

		std::string strNickName;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;

		float fYawDegrees = 0.f;
	};

	class CNetObjectRegistry final
	{
	public:
		//새 character를 registry에 등록
		bool Register(
			const NET_PLAYER_RECORD& record,
			const std::shared_ptr<CCharacter>& character,
			OBJECT_HANDLE& outHandle);

		//server entity ID를 이용해 현재 Handle을 찾는다.
		bool Find_Handle(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			OBJECT_HANDLE& outHandle) const;

		//동일 Entity의 중복 Spawn을 검사할 때 사용한다
		const NET_PLAYER_RECORD* Find_Record(
			LostArk::Shared::NET_ENTITY_ID netEntityId) const;

		//Handle이 현재 살아있는 객체를 가리키는지 확인한다
		std::shared_ptr<CCharacter> Resolve(
			OBJECT_HANDLE handle)  const;

		//Character와 NetEntityId의 연결을 해제한다.
		bool Unregister(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			std::shared_ptr<CCharacter>* outCharacter = nullptr);

		//live objects 가지고 오기
		std::vector<std::shared_ptr<CCharacter>> Get_LiveObjects() const;

		//접속이 끊기거나 월드 상태를 비울 때, 모든 slot을 무효화 시킨다.
		void Reset();

	private:
		//generation과 occupy 상태, player record, character를 가지는 객체 슬롯 1개
		struct SLOT
		{
			std::uint32_t iGeneration = 1;
			bool isOccupied = false;
			NET_PLAYER_RECORD record;
			std::weak_ptr<CCharacter> pCharacter;
		};

		static void Advance_Generation(SLOT& slot);

	private:
		std::vector<SLOT> m_Slots;
		std::vector<std::uint32_t> m_FreeSlotIndices;

		std::unordered_map <LostArk::Shared::NET_ENTITY_ID, OBJECT_HANDLE> m_HandleByEntityId;
	};
}
