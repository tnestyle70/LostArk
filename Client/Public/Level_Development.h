#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CMapLightPresentationRuntime;
class IPlayerCommandSink;
class CMaharakaWaterpangPresentation;
class CInteractKeyPromptView;

class CLevel_Development final : public CLevel
{
private:
	CLevel_Development(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel);

public:
	virtual ~CLevel_Development();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	// The current Development/Training or Maharaka owner exposes its existing camera.
	static CLevel_Development* Get_Active(LEVEL level)
	{
		return s_pActiveInstance && s_pActiveInstance->m_eLevel == level ?
			s_pActiveInstance : nullptr;
	}
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera.lock(); }
#ifdef _DEBUG
	// Borrow the existing Maharaka map; the Level remains its owner.
	CMapPlacementRuntime& Get_MapAuthoringRuntime() { return m_MapRuntime; }
	CDeployPropRuntime& Get_MapAuthoringDeploy() { return m_WaterpangDeploy; }
	const ComPtr<ID3D11Device>& Get_MapAuthoringDevice() const { return m_pDevice; }
	const ComPtr<ID3D11DeviceContext>& Get_MapAuthoringContext() const { return m_pContext; }
	void Set_MapAuthoringActive(bool_t active);
	std::shared_ptr<CCharacter> Get_DebugLocalCharacter() const { return m_Replication.Get_LocalCharacter(); }
	// Debug F1 typed Server requests (Waterpang forced patterns) go through the level's sink.
	std::shared_ptr<IPlayerCommandSink> Get_DebugCommandSink() const { return m_pPlayerCommandSink; }
	std::shared_ptr<CNpc> Find_MapAuthoringNpc(const std::string& placementId) const
	{ return m_Replication.Find_NpcPlacement(placementId); }
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records)
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
#endif

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	// Registry entry this instance plays; only DEVELOPMENT may open the Map Editor.
	LEVEL m_eLevel = LEVEL::DEVELOPMENT;
	CMapPlacementRuntime m_MapRuntime;
	CDeployPropRuntime m_WaterpangDeploy;
#ifdef _DEBUG
	bool_t m_bMapAuthoringActive = false;
#endif
	// Maharaka only: the published source lights of the island, submitted every frame.
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	bool_t m_isMapEditorWorkspace = false;
	weak_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
	std::unique_ptr<CMaharakaWaterpangPresentation> m_Waterpang;
	std::unique_ptr<CInteractKeyPromptView> m_InteractPrompt;
	static CLevel_Development* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Development> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel = LEVEL::DEVELOPMENT);
};

NS_END
