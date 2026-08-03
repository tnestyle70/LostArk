#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "Network/PacketType.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;

struct LobbyCharacterInfo
{
	int32_t m_iSlotIndex = { -1 };
	LostArk::Shared::CHARACTER_CLASS_ID m_eClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	shared_ptr<CCharacter> m_pCharacter = { nullptr };
	string m_strNickName;
};

// 정식 실행의 시작 레벨. 캐릭터 선택과 서버 월드 입장 요청만 소유한다.
class CLevel_Lobby final : public CLevel
{
private:
	CLevel_Lobby(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Lobby();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_CharacterSlots();

	bool_t Select_Character(int32_t iCharacterIndex);
	bool_t Request_EnterWorld(LostArk::Shared::WORLD_ID eWorldId);

	void Render_CharacterSelectPanel();

private:
	vector<LobbyCharacterInfo> m_vecCharacters = {};
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	int32_t m_iSelectedCharacterIndex = { -1 };
	char_t m_szNickName[64] = {};
	bool_t m_isEnterRequested = { false };
	LostArk::Shared::WORLD_ID m_ePendingWorldId =
		LostArk::Shared::WORLD_ID::END;
	string m_strNetworkStatus = {
		"Select a character and enter a nickname."
	};

public:
	static unique_ptr<CLevel_Lobby> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
