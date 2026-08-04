#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "MapPlacementRuntime.h"
#include "Network/PacketType.h"

#include <array>

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;

class CLevel_CharacterSelect final : public CLevel
{
private:
	CLevel_CharacterSelect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_CharacterSelect();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_Preview(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool_t Select_Preview(size_t index);
	bool_t Enter_Stage(LOBBY_STAGE eStage);
	void Render_SelectionPanel();

private:
	static constexpr std::array<
		LostArk::Shared::CHARACTER_CLASS_ID, 5> SUPPORTED_CLASSES =
	{
		LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
		LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
		LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
		LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER
	};

	CMapPlacementRuntime m_MapRuntime;
	size_t m_iPreviewIndex = 0;
	shared_ptr<CCharacter> m_pPreviewCharacter = { nullptr };
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	string m_strStatus =
		"Choose a class and a stage. Enter joins Test.";

public:
	static unique_ptr<CLevel_CharacterSelect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
