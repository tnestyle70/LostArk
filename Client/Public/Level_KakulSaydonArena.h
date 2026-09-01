#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"
#include "WorldSequencePlayer.h"

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;
class IWorldEntityCommandSink;

class CLevel_KakulSaydonArena final : public CLevel
{
public:
	struct KAKUL_STAGE_MARKER final
	{
		std::string strStageId;
		std::string strPlacementId;
		std::string strDisplayNameKo;
		std::string strSourceLevelId;
	};

private:
	CLevel_KakulSaydonArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_KakulSaydonArena();

	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	static CLevel_KakulSaydonArena* Get_Active()
	{
		return s_pActiveInstance;
	}

	/* The F1 stage selector submits only stable authored placement IDs through
	   the typed Server command sink. Until an authored StageMarkers contract is
	   loaded, the empty allow-list rejects every request instead of inventing a
	   waypoint or teleporting the local Character. */
	bool_t Request_StageTeleport(
		std::uint32_t requestSequence,
		std::string_view placementId,
		std::string& outStatus);
	const std::vector<KAKUL_STAGE_MARKER>& Get_StageMarkers() const
	{
		return m_StageMarkers;
	}

	/* Raises one paper stage bridge: the Deploy prop leaves DESPAWNED and its
	   authored unfold sequence starts on the same frame, so the bridge is
	   never visible in its finished pose before it has unfolded. Playing an
	   already raised bridge is a no-op rather than a rewind. */
	bool_t Request_PaperBridgeUnfold(
		uint64_t leverPlacementId,
		std::string& outStatus);

private:
	bool_t Start_ServerRequestedSequence(
		const std::string& instanceId,
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus);
	bool_t Load_StageMarkers(std::string& outStatus);
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	CMapPlacementRuntime m_MapRuntime;
	/* The authored deploy catalog carries both paper levers and both paper
	   stage bridges. A bridge stays DESPAWNED until its lever is pulled, so
	   suppress the bridges before the first rendered frame instead of letting
	   them appear already unfolded. */
	CDeployPropRuntime m_DeployRuntime;
	CWorldSequencePlayer m_SequencePlayer;
	std::unordered_set<uint64_t> m_RaisedPaperBridges;
	shared_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
	std::vector<KAKUL_STAGE_MARKER> m_StageMarkers;
	std::unordered_set<std::string> m_StageMarkerPlacementIds;
	static CLevel_KakulSaydonArena* s_pActiveInstance;

public:
	static unique_ptr<CLevel_KakulSaydonArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
