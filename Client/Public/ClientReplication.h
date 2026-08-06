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

//Network Event瑜??ㅼ젣 Engine GameObject ?앹꽦 ?쒓굅濡?踰덉뿭?섎뒗 ???섎굹??main-thread 寃쎄퀎

namespace Client
{
	class CCharacter;
	class CNpc;
	class CValtan;

	class CClientReplication final
	{
	public:
		//replication??character瑜??대뒓 layer? prototype???앹꽦?댁빞 ?섎뒗吏瑜??꾨떖?쒕떎.
		//rpelication??level baren???섎뱶肄붾뵫???꾩슂媛 ?녿떎.
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
		bool Apply_WorldEntityDespawn(
			const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned);
		//snapshot??netentityid瑜??ㅼ젣 client character濡??댁꽍?섎뒗 ?⑥닔
		bool Apply_WorldSnapshot(
			const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot);

		void Reset_World();

	private:
		//?대뼡 layer怨?prototype???앹꽦?섏뼱???섎뒗吏
		DESC m_Desc;
		//slot怨?freeslotindex, handlebyentityid瑜?留ㅺ컻蹂?섎줈 ?ㅺ퀬 ?덈뒗 net 媛앹껜
		CNetObjectRegistry m_Registry;
		//index slot, slotindex, generation
		OBJECT_HANDLE m_LocalCharacterHandle;
		bool m_isInitialized = false;
		bool m_wasConnected = false;
		bool m_hasPendingConnectionLoss = false;
		//留덉?留됱쑝濡??곸슜??snapshot tick
		std::uint32_t m_iLastServerTick = 0;

		struct WORLD_ENTITY_PRESENTATION
		{
			LostArk::Shared::WORLD_ENTITY_KIND eKind =
				LostArk::Shared::WORLD_ENTITY_KIND::END;
			std::string strArchetypeId;
			std::string strEncounterId;
			std::string strCurrentClip;
			std::weak_ptr<CNpc> pNpc;
			std::weak_ptr<CValtan> pValtan;
		};
		std::unordered_map<
			LostArk::Shared::NET_ENTITY_ID,
			WORLD_ENTITY_PRESENTATION> m_WorldEntities;
	};
}
