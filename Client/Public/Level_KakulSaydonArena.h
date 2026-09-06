#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"
#include "ValtanCinematicCameraDocument.h"
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
class CTrigger_Box;
class IPlayerCommandSink;
class IWorldEntityCommandSink;

class CUILayoutRuntime;

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

	/* One authored camera shot. While the local Character stands inside the
	   box the camera holds this exact pose - the reference footage keeps the
	   background pinned while the party walks - and leaving the box hands the
	   camera back to the ordinary follow view. */
	struct KAKUL_CAMERA_SHOT final
	{
		std::string strShotId;
		/* Empty means the box decides. When it names a sequence the
		   shot holds for exactly as long as that sequence plays, so a
		   trigger that starts the sequence also starts the shot. */
		std::string strSequenceInstanceId;
		float3_t vCenter = {};
		float3_t vHalfExtents = {};
		f32_t fYawDegrees = 0.f;
		float3_t vEye = {};
		float3_t vLookAt = {};
		f32_t fFovYDegrees = 60.f;
		uint32_t iBlendInMs = 0u;
		uint32_t iBlendOutMs = 0u;
		uint32_t iPriority = 0u;
		/* A side scrolling stage keeps one framing and slides it with the
		   local Character instead of pinning it in place. Both offsets are
		   added to that Character's position, so the authored eye and lookAt
		   stay as the pose used while no Character exists. */
		bool_t followsPlayer = false;
		float3_t vFollowEyeOffset = {};
		float3_t vFollowLookAtOffset = {};
		/* A shot without a track keeps the single authored pose. With one
		   it is sampled on the bound sequence's own clock by the one
		   cinematic sampler this project owns, so a second easing or
		   spline implementation can never drift from it. */
		bool_t hasCameraTrack = false;
		VALTAN_CINEMATIC_CAMERA_CUE CameraTrack;
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
	void Collect_MinimapMarkers(
		CClientReplication::MINIMAP_MARKER_SNAPSHOT& outSnapshot) const
	{
		m_Replication.Collect_MinimapMarkers(outSnapshot);
	}

#ifdef _DEBUG
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	CPlayerController& Get_DebugPlayerController() { return m_PlayerController; }
	// Applies immediately and remembers this arena's value until process exit.
	bool_t Set_DebugCameraSpeed(f32_t metersPerSecond);
#endif

	// The level owns the replicated player anchor used by local authoring previews.
	bool_t Try_Get_AuthoringPreviewPlacement(
		float3_t& outPosition, std::string& outStatus) const;

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
	const std::vector<KAKUL_CAMERA_SHOT>& Get_CameraShots() const
	{
		return m_CameraShots;
	}

	/* Raises one paper stage bridge: the Deploy prop leaves DESPAWNED and its
	   authored unfold sequence starts on the same frame, so the bridge is
	   never visible in its finished pose before it has unfolded. Playing an
	   already raised bridge is a no-op rather than a rewind. */
	bool_t Request_PaperBridgeUnfold(
		uint64_t leverPlacementId,
		std::string& outStatus);

private:
	/* The cutscene is one show spread over several instances. Starting the
	   named one starts them all and swaps the arena for the cutscene copy. */
	bool_t Start_PopupBookCutscene(
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus);
	void Apply_CutsceneSetVisible(bool_t cutsceneVisible);
	/* The cutscene boss is presentation only, so it is taken off the arena
	   as soon as its sequence stops playing. */
	void Update_CutsceneBossRetire(
		const CWorldSequencePlayer::TARGET_SET& targets);
	bool_t Start_ServerRequestedSequence(
		const std::string& instanceId,
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus);
	bool_t Load_StageMarkers(std::string& outStatus);
	bool_t Load_CameraShots(std::string& outStatus);
	void Update_CameraShots(f32_t fTimeDelta);
	/* The arena and the Mario gimmick are more than a kilometre apart, so a
	   trigger move between them is hidden behind a black screen instead of
	   letting the camera travel that distance on screen. Server owns the
	   move; this only reads the action state it already replicates. */
	void Update_TriggerMoveFade(f32_t fTimeDelta);
	const KAKUL_CAMERA_SHOT* Find_ActiveCameraShot(
		const float3_t& vPosition) const;
	void Release_CameraShot();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

#ifdef _DEBUG
	/* The three arena-side `_go` boxes are the only way into the Mario
	   stages and each is 2x1x2m of empty air, so nobody can find them
	   without a wire. This draws them through the same authoring box the
	   Map Editor and Bern already use; it reads the authored document and
	   owns no gameplay state. */
	bool_t Ready_DebugStageEntryTriggers(const std::string& areaId);
#endif

private:
	CMapPlacementRuntime m_MapRuntime;
	/* The authored deploy catalog carries both paper levers and both paper
	   stage bridges. A bridge stays DESPAWNED until its lever is pulled, so
	   suppress the bridges before the first rendered frame instead of letting
	   them appear already unfolded. */
	CDeployPropRuntime m_DeployRuntime;
	CWorldSequencePlayer m_SequencePlayer;
	bool_t m_bCutsceneBossVisible = false;
	bool_t m_bCutsceneSetVisible = false;
	std::unordered_set<uint64_t> m_RaisedPaperBridges;
	shared_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
	std::vector<KAKUL_STAGE_MARKER> m_StageMarkers;
	std::unordered_set<std::string> m_StageMarkerPlacementIds;
	std::vector<KAKUL_CAMERA_SHOT> m_CameraShots;
	std::string m_strActiveCameraShotId;
	/* The pose written last frame. A hand-over starts from this, so entering,
	   swapping and leaving all begin at what the player already sees. */
	float3_t m_vCameraEyeApplied = {};
	float3_t m_vCameraLookApplied = {};
	f32_t m_fCameraFovApplied = 60.f;
	float3_t m_vCameraEyeFrom = {};
	float3_t m_vCameraLookFrom = {};
	f32_t m_fCameraFovFrom = 60.f;
	float3_t m_vCameraEyeTo = {};
	float3_t m_vCameraLookTo = {};
	f32_t m_fCameraFovTo = 60.f;
	f32_t m_fCameraBlendSeconds = 0.f;
	f32_t m_fCameraBlendElapsed = 0.f;
	bool_t m_bCameraShotHeld = false;
	std::string m_strCameraShotStatus;
	/* One full-screen slot, black, whose alpha is the whole effect. Built
	   hidden so the first rendered frame after activation cannot flash it. */
	unique_ptr<CUILayoutRuntime> m_pTriggerMoveFadeView;
	f32_t m_fTriggerMoveFadeAlpha = 0.f;
	/* Speed gate. The short hops share TRIGGER_MOVE with the stage
	   transition, so the fade arms only once the character is seen moving
	   far faster than any hop can. */
	float3_t m_vTriggerMoveFadeLastPosition = {};
	bool_t m_bTriggerMoveFadeHasLastPosition = false;
	bool_t m_bTriggerMoveFadeArmed = false;

#ifdef _DEBUG
	std::vector<shared_ptr<CTrigger_Box>> m_DebugStageEntryTriggers;
#endif

	static CLevel_KakulSaydonArena* s_pActiveInstance;

public:
	static unique_ptr<CLevel_KakulSaydonArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
