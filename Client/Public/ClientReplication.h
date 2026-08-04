#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ClientReplicationEvent.h"
#include "NetObjectRegistry.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

//Network Event를 실제 Engine GameObject 생성 제거로 번역되는 단 하나의 main-thread 경계

namespace Client
{
	class CCharacter;
	class CValtan;

	class CClientReplication final
	{
	public:
		//replication이 character를 어느 layer와 prototype에 생성해야 하는지를 전달한다.
		//rpelication이 level baren을 하드코딩할 필요가 없다.
		struct DESC
		{
			ComPtr<ID3D11Device> pDevice;
			ComPtr<ID3D11DeviceContext> pContext;
			std::uint32_t iPrototypeLevelIndex = 0;
			std::uint32_t iLayerLevelIndex = 0;
			std::wstring strPlayerLayerTag;
			std::wstring strWorldEntityLayerTag;
		};

	public:
		bool Initialize(const DESC& desc);
		bool Update();
		bool Has_PendingConnectionLoss() const;
		void Acknowledge_ConnectionLoss();
		void Reset();
		bool Has_WorldEntity(std::string_view archetypeId) const;

		std::shared_ptr<CCharacter> Get_LocalCharacter() const;

	private:
		bool Create_Character(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			std::string_view nickName,
			const float3_t& position,
			f32_t yawDegrees,
			bool_t isLocallyControlled,
			std::shared_ptr<CCharacter>& outCharacter);
		bool Apply_Spawn(
			const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned);

		bool Apply_Despawn(
			const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned);
		bool Apply_WorldEntitySpawn(
			const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned);
		//snapshot의 netentityid를 실제 client character로 해석하는 함수
		bool Apply_WorldSnapshot(
			const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot);

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
		bool m_hasPendingConnectionLoss = false;
		//마지막으로 적용한 snapshot tick
		std::uint32_t m_iLastServerTick = 0;

		struct WORLD_ENTITY_PRESENTATION
		{
			LostArk::Shared::WORLD_ENTITY_KIND eKind =
				LostArk::Shared::WORLD_ENTITY_KIND::END;
			std::string strArchetypeId;
			std::string strEncounterId;
			std::weak_ptr<CValtan> pValtan;
		};
		std::unordered_map<
			LostArk::Shared::NET_ENTITY_ID,
			WORLD_ENTITY_PRESENTATION> m_WorldEntities;
	};
}
