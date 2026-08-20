#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "Level.h"
#include "MapPlacementRuntime.h"

#include "PlayerController.h"
#include "WorldPlayerNameplateView.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CHUDRuntimeView;
class CTrigger_Box;
class IPlayerCommandSink;

class CLevel_Bern final : public CLevel
{
private:
	CLevel_Bern(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Bern();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	/* CGameInstance::Draw_Text submits immediately (SpriteBatch), but
	Render_ValtanEntryModal's popup art composites later inside
	CImGuiLayer::EndFrame() -- same reason Level_CharacterSelect splits
	Render_ArenaSpawnLabels() out. m_pValtanEntryView is private to this level,
	so CMainApp reaches it through Get_Active() instead of a second
	CHUDRuntimeView of its own. */
	void Render_ValtanEntryModalText();
	static CLevel_Bern* Get_Active() { return s_pActiveInstance; }

private:
	HRESULT Ready_Layer_Camera(
		const wstring_t& strLayerTag);

	bool_t Bind_CameraToLocalCharacter();

	/* Loads the Bern authoring document once and keeps only the two known
	Valtan-entry guide NPCs' authored positions (npc.bern.beda.guide,
	npc.bern.aylara) -- the same static, server-uninvolved position lookup
	Ready_DebugLevelChangeTriggers already does for its own trigger boxes, just
	not _DEBUG-only since the interaction it drives is a real product path. */
	bool_t Ready_ValtanEntryNpcs(const std::string& areaId);
	/* Right-click hit-test against m_ValtanEntryNpcs using the same ground-plane
	unproject CPlayerController::Try_PickGroundPlane already does for movement --
	opens the confirm window on a hit instead of forwarding to a move goal. Runs
	before m_PlayerController.Update() each frame so a click that opens the
	window also suppresses that frame's move command (gameplayCommandsEnabled
	already goes false while the window is open). */
	void Update_ValtanEntryInteraction();
	/* BeginPopupModal + OpenPopup both live in here, in the same call site every
	frame, so the popup's ID always resolves against the same (no-window)
	context -- Level_CharacterSelect's Create Character button broke once by
	splitting OpenPopup() into a different window's Begin/End than the modal
	check ran inside. */
	void Render_ValtanEntryModal();

#ifdef _DEBUG
	bool_t Ready_DebugLevelChangeTriggers(const std::string& areaId);
#endif

private:
	/*베른성 맵 객체들의 생성과 제거는 기존 Map Runtime이 담당한다.
	Network Player 수명과 섞지 않는다.*/
	CMapPlacementRuntime m_MapRuntime;

	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	weak_ptr<CCharacter> m_pCameraTarget;

	CClientReplication m_Replication;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	//PlayerController 추가
	CPlayerController m_PlayerController;

	struct VALTAN_ENTRY_NPC
	{
		std::string strPlacementId;
		float3_t vPosition{};
	};
	std::vector<VALTAN_ENTRY_NPC> m_ValtanEntryNpcs;
	unique_ptr<CHUDRuntimeView> m_pValtanEntryView;
	bool_t m_isValtanEntryModalOpen = false;
	bool_t m_hasValtanEntryModalJustOpened = false;
	std::string m_strValtanEntryNpcPlacementId;
	bool_t m_wasRightMouseDownForNpcInteract = false;
	std::uint32_t m_iNextNpcEntryConfirmSequence = 1u;

#ifdef _DEBUG
	std::vector<shared_ptr<CTrigger_Box>> m_DebugLevelChangeTriggers;
#endif

	static CLevel_Bern* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Bern> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
