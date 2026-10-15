#pragma once

// Server와 공유하는 PLAYER_ID·NET_ENTITY_ID 및 무효 ID 값을 사용한다.
#include "Network/NetworkIds.h"
// 표시 중인 외형 변환 상태 PLAYER_MADNESS_FORM을 기록한다.
#include "Network/PacketMessages.h"
// 직업·제어 종류·voice 기본값과 지원 직업 검사를 사용한다.
#include "Network/PacketType.h"

// handle의 slot·generation과 wire ID에 폭이 고정된 정수를 사용한다.
#include <cstdint>
// uint32_t 최대값을 사용 불가 slot index로 둔다.
#include <limits>
// Registry는 weak_ptr, 조회 결과는 일시적인 shared_ptr로 Character를 연결한다.
#include <memory>
// 닉네임과 외형 JSON·archetype ID를 record의 문자열 값으로 보관한다.
#include <string>
// NetEntityId에서 현재 handle을 찾는 인덱스다. slot과 재사용 목록은 vector가 보관한다.
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
			(std::numeric_limits<std::uint32_t>::max)();
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
		// 현재 반영된 표시 외형이다. 다른 형태의 snapshot은 Character 표현 교체에 사용된다.
		LostArk::Shared::PLAYER_MADNESS_FORM eMadnessForm =
			LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL;

		std::string strNickName;
		// Server가 선택해 전달한 Waterpang AI의 외형 archetype ID다.
		std::string strWaterpangNpcArchetypeId;
		std::uint8_t iVoiceType = LostArk::Shared::MIN_VOICE_TYPE;
		std::string strAppearanceJson;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;

		float fYawDegrees = 0.f;
		LostArk::Shared::PLAYER_CONTROL_KIND eControlKind = LostArk::Shared::PLAYER_CONTROL_KIND::HUMAN;
	};

	// Server record와 실제 presentation을 잃지 않고 묶어 읽는 snapshot이다.
	// Registry의 owner 상태가 아니며, 반환 vector가 살아 있는 동안만 Character 수명을 붙든다.
	// 복사한 shared_ptr가 vector 밖에 남으면 그 참조가 끝날 때까지 수명이 연장된다.
	struct LIVE_NET_PLAYER
	{
		NET_PLAYER_RECORD Record;
		std::shared_ptr<CCharacter> pCharacter;
	};

	// CClientReplication이 값 멤버로 소유한다. 연결 기록만 관리하고 Character 생성·Layer 제거는 호출자에게 맡긴다.
	class CNetObjectRegistry final
	{
	public:
		//새 character를 registry에 등록
		bool Register(
			const NET_PLAYER_RECORD& record,
			const std::shared_ptr<CCharacter>& character,
			OBJECT_HANDLE& outHandle);

		// 같은 entity의 표시 객체와 record만 교체한다. 검사에서 false를 반환하면 기존 record·객체·handle을 유지한다.
		bool Replace(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			const NET_PLAYER_RECORD& record,
			const std::shared_ptr<CCharacter>& character,
			OBJECT_HANDLE& outHandle);

		//server entity ID를 이용해 현재 Handle을 찾는다.
		bool Find_Handle(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			OBJECT_HANDLE& outHandle) const;

		//동일 Entity의 중복 Spawn을 검사할 때 사용한다
		// 반환 주소는 slot 내부 기록의 차용이다. 이후 slot 추가·교체·해제·Reset을 넘겨 보관하지 않는다.
		const NET_PLAYER_RECORD* Find_Record(
			LostArk::Shared::NET_ENTITY_ID netEntityId) const;

		//Handle이 현재 살아있는 객체를 가리키는지 확인한다
		// 유효한 handle도 weak_ptr가 만료됐으면 nullptr다. 성공한 반환 참조가 살아 있는 동안 객체 수명을 붙든다.
		std::shared_ptr<CCharacter> Resolve(
			OBJECT_HANDLE handle)  const;

		//Character와 NetEntityId의 연결을 해제한다.
		bool Unregister(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			std::shared_ptr<CCharacter>* outCharacter = nullptr);

		//live objects 가지고 오기
		std::vector<std::shared_ptr<CCharacter>> Get_LiveObjects() const;

		// Level/UI consumer가 stable record와 살아 있는 presentation을 함께 읽는다.
		std::vector<LIVE_NET_PLAYER> Get_LivePlayers() const;

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

		// 해제한 slot의 세대를 올리되 무효값 0은 건너뛴다.
		static void Advance_Generation(SLOT& slot);

	private:
		// slot 저장 공간과 재사용 가능한 index를 보관한다. Reset도 slot 공간은 유지한다.
		std::vector<SLOT> m_Slots;
		std::vector<std::uint32_t> m_FreeSlotIndices;

		std::unordered_map <LostArk::Shared::NET_ENTITY_ID, OBJECT_HANDLE> m_HandleByEntityId;
	};
}
