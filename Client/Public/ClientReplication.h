#pragma once

#include "ClientReplicationEvent.h"
#include "NetObjectRegistry.h"

#include <cstdint>
#include <memory>
#include <string>

//Network Event를 실제 Engine GameObject 생성 제거로 번역되는 단 하나의 main-thread 경계

namespace Client
{
	class CCharacter;

	class CClientReplication final
	{
	public:
		//replication이 character를 어느 layer와 prototype에 생성해야 하는지를 전달한다.
		//rpelication이 level baren을 하드코딩할 필요가 없다.
		struct DESC
		{
			std::uint32_t iPrototypeLevelIndex = 0;
			std::uint32_t iLayerLevelIndex = 0;
			std::wstring strPlayerLayerTag;
		};

	public:
		bool Initialize(const DESC& desc);
		bool Update();

		std::shared_ptr<CCharacter> Get_LocalCharacter() const;

	private:
		bool Apply_Spawn(
			const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned);

		bool Apply_Despawn(
			const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned);

		void Reset_World();

	private:
		//어떤 layer과 prototype에 생성되어야 하는지
		DESC m_Desc;
		//slot과 freeslotindex, handlebyentityid를 매개변수로 들고 있는 net 객체
		CNetObjectRegistry m_Registry;
		//index slot, slotindex, generation
		OBJECT_HANDLE m_LocalCharacterHandle;
		bool m_isInitialized = false;
		bool m_wasConnected = false;
	};
}
