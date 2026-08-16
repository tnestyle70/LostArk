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

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;

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
	bool_t Bind_CameraToLocalCharacter();
	void Update_CinematicCamera(f32_t fTimeDelta);
	void End_CinematicCamera();
	void Update_WorldDestructionPresentation(f32_t fTimeDelta);
#ifdef _DEBUG
	/* Debug audition of an authored health-bar pattern. The panel only submits
	typed requests and reports what the Server answered; it never starts a
	pattern, moves the camera or breaks a wall on its own. */
	void Render_AuditionPanel();
	void Submit_Audition(
		LostArk::Shared::VALTAN_AUDITION_OPERATION operation);
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
	CWorldDestructionProjectionDocument m_WorldDestructionProjectionDocument;
	CWorldDestructionDebrisPresentationDocument
		m_WorldDestructionDebrisPresentationDocument;
	CWorldDestructionDebrisPresentationRuntime
		m_WorldDestructionDebrisPresentationRuntime;
	uint64_t m_iObservedWorldDestructionPresentationGeneration = 0u;
	CClientReplication m_Replication;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
#ifdef _DEBUG
	size_t m_iSelectedAuditionBarIndex = 0u;
	uint32_t m_iNextAuditionRequestSequence = 1u;
	uint32_t m_iPendingAuditionRequestSequence = 0u;
	std::string m_strAuditionStatus;
#endif

public:
	static unique_ptr<CLevel_ValtanArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
