#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "EncounterPatternReference.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PlayerController.h"
#include "ValtanCinematicCameraController.h"
#include "ValtanCinematicCameraDocument.h"
#include "WorldDestructionDebrisPresentationDocument.h"
#include "WorldDestructionDebrisPresentationRuntime.h"
#include "WorldDestructionProjectionDocument.h"
#include "WorldPlayerNameplateView.h"

#include <array>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;
class CMapAssetObject;

class CLevel_ValtanArena final : public CLevel
{
private:
	CLevel_ValtanArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_ValtanArena();

	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Ready_CinematicCamera();
	bool_t Ready_ValtanSkyPresentation(std::string& outStatus);
	bool_t Bind_CameraToLocalCharacter();
	void Update_CinematicCamera(f32_t fTimeDelta);
	void Apply_ValtanSkyPresentation(
		const VALTAN_CINEMATIC_SKY_STATE& state);
	void Reset_ValtanSkyPresentation();
	void Clear_ValtanSkyPresentation();
	void End_CinematicCameraOverride();
	void End_CinematicCamera();
	void Update_WorldDestructionPresentation(f32_t fTimeDelta);
	bool_t Apply_EncounterPropPresentation();
#ifdef _DEBUG
	enum class REFERENCE_CAMERA_VIEW : uint8_t
	{
		NONE,
		TOP_DOWN,
		EXTERIOR
	};
	bool_t Begin_ReferenceCamera(REFERENCE_CAMERA_VIEW view);
	bool_t Set_ReferencePhaseProxyVisible(bool_t visible);
	void Update_ReferenceCamera();
	void End_ReferenceCamera(bool_t toggleFollowRequested);
	const char_t* Get_ReferenceCameraViewName() const;
	/* Debug audition of an authored health-bar pattern. The panel only submits
	typed requests and reports what the Server answered; it never starts a
	pattern or breaks a wall on its own. Reference-view buttons below are a
	separate presentation-only camera aid and never submit gameplay state. */
	void Render_AuditionPanel();
	void Update_AuditionTransaction();
	bool_t Submit_Audition(
		LostArk::Shared::VALTAN_AUDITION_OPERATION operation);
	void Request_OrderedAuditionStop(bool_t restartAfterStop);
	struct AUDITION_PENDING_REQUEST final
	{
		uint32_t iSequence = 0u;
		LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
			LostArk::Shared::VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		uint32_t iTargetHealthBar = 0u;
		uint64_t iLastSentAtMilliseconds = 0u;
		uint32_t iRetryCount = 0u;

		[[nodiscard]] bool_t Is_Active() const
		{
			return 0u != iSequence;
		}
	};
	/* One ordered chapter run over the existing audition operations. It resets
	once at the entrance and then only crosses the authored bars, so the
	environment stays cumulative the way the recording shows it. */
	struct ENVIRONMENT_TIMELINE_STEP final
	{
		LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
			LostArk::Shared::VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		uint32_t iTargetHealthBar = 0u;
		bool_t waitForPattern = false;
	};
	void Start_EnvironmentTimeline();
	void Advance_EnvironmentTimeline(bool_t isBossPatternRunning);
#endif

private:
	CMapPlacementRuntime m_MapRuntime;
	CDeployPropRuntime m_DeployRuntime;
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	weak_ptr<CCharacter> m_pCameraTarget;
	weak_ptr<CTransform> m_pCinematicRestoreTarget;
	bool_t m_bCinematicRestoreFollowRequested = false;
	bool_t m_bCinematicCameraApplied = false;
	uint64_t m_iCinematicCameraOwnerId = 0u;
	CEncounterPatternReference m_ValtanEncounterReference;
	CValtanCinematicCameraDocument m_ValtanCinematicCameraDocument;
	CValtanCinematicCameraController m_ValtanCinematicCameraController;
	/* The Server-owned pattern clock only selects this presentation. The six
	   cached map objects never participate in collision or navigation. */
	VALTAN_CINEMATIC_SKY_STATE m_ValtanSkyState;
	struct VALTAN_SKY_PRESENTATION_LAYER final
	{
		shared_ptr<CMapAssetObject> pObject;
		std::string strAssetId;
		uint64_t iPlacementId = 0u;
		float3_t vBasePosition = {};
		float4_t vBaseRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
		float3_t vBaseSignedScale = float3_t(1.f, 1.f, 1.f);
		bool_t bBaseVisible = false;
		VALTAN_CINEMATIC_SKY_LAYER_POLICY Policy;
	};
	static constexpr size_t VALTAN_SKY_LAYER_COUNT = 3u;
	std::array<VALTAN_SKY_PRESENTATION_LAYER, VALTAN_SKY_LAYER_COUNT>
		m_ValtanRedCloudLayers{};
	std::array<VALTAN_SKY_PRESENTATION_LAYER, VALTAN_SKY_LAYER_COUNT>
		m_ValtanBlackApertureLayers{};
	std::string m_strValtanRedCloudSeedAssetId;
	std::string m_strValtanBlackApertureSeedAssetId;
	CWorldDestructionProjectionDocument m_WorldDestructionProjectionDocument;
	CWorldDestructionDebrisPresentationDocument
		m_WorldDestructionDebrisPresentationDocument;
	CWorldDestructionDebrisPresentationRuntime
		m_WorldDestructionDebrisPresentationRuntime;
	uint64_t m_iObservedWorldDestructionPresentationGeneration = 0u;
	uint32_t m_iObservedEncounterPropEpoch = 0u;
	uint32_t m_iObservedEncounterPropServerTick = 0u;
	CClientReplication m_Replication;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
#ifdef _DEBUG
	weak_ptr<CTransform> m_pReferenceCameraRestoreTarget;
	bool_t m_bReferenceCameraRestoreFollowRequested = false;
	bool_t m_bReferenceCameraApplied = false;
	bool_t m_bReferenceSpaceHoleVisible = false;
	REFERENCE_CAMERA_VIEW m_eReferenceCameraView =
		REFERENCE_CAMERA_VIEW::NONE;
	size_t m_iSelectedAuditionBarIndex = 0u;
	uint32_t m_iNextAuditionRequestSequence = 1u;
	AUDITION_PENDING_REQUEST m_PendingAuditionRequest;
	bool_t m_bOrderedAuditionActive = false;
	bool_t m_bStopAuditionQueued = false;
	bool_t m_bRestartOrderedAfterStop = false;
	std::string m_strAuditionStatus;
	std::vector<ENVIRONMENT_TIMELINE_STEP> m_EnvironmentTimeline;
	size_t m_iEnvironmentTimelineStep = 0u;
	bool_t m_bEnvironmentTimelineWaiting = false;
	bool_t m_bEnvironmentTimelinePatternStarted = false;
#endif

public:
	static unique_ptr<CLevel_ValtanArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
