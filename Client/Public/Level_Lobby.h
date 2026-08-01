#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "Network/PacketType.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
//우선 존재하는 Lance Master를 사용해서, 캐릭터를 선택하고, 베른성으로 
//입장할 수 있도록 한다. 

struct LobbyCharacterInfo
{
	int32_t m_iSlotIndex = { -1 };
	//선택 가능한 클래스
	LostArk::Shared::CHARACTER_CLASS_ID
		m_eClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	shared_ptr<CCharacter> m_pCharacter = { nullptr };
	string m_strNickName;
};

//Packet? 패킷이 있어야 하는 상황인 건가? 뭐가 있어야 하는 거지?

class CLevel_Lobby final : public CLevel
{
private:
	CLevel_Lobby(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel_Lobby();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	//로비에서 선택할 수 있는 캐릭터들 나열 
	HRESULT Ready_CharacterSlots();

	bool_t Select_Character(int32_t iCharacterIndex);
	bool_t Request_EnterWorld();
#ifdef _DEBUG
	void Render_CharacterSelectPanel();
#endif
	//HRESULT Ready_Server 이런 느낌으로? 서버를 Ready 시킨다?
	
	//마우스 입력을 통한 닉네임 변경창 나오고, 이름 입력을 할 수 있도록 하고, 해당 이름을 
	//저장한 다음에, 캐릭터 아래에 띄운다. 캐릭터 이름 저장을 위한 변수를 만들어야 한다. 
	//그리고 이거를 character 별로 존재하게 해야 하니까, map<int, shared_ptr<CCharacter>>
	//이런 식으로 들고있어야 하나? 


	//캐릭터 4명. 4명 아래에 class 이름과 nickname 존재. 창 클릭하면 닉네임 변경할 수 있는 창이 뜬다. 
	//이름 입력하면 캐릭터에게 반영됨.

	//이름 선택과 입력 받아서 characterClassNickname에 저장 시키고, character 아래에 띄울 수 있게 한다.
	//Update 내부 함수에서 호출을 하는 방식으로 진행한다.

	//마우스 입력과 닉네임의 변경을 둘 다 가능하게 한다.
	//LeftMouse True? NickName Change Okay
	//void UpdateChangeNickName(); 
	//
	//마우스 클릭 및 Level_Lobby에서 구현해야 하는 기능 전부
	//void UpdateMouseClick();
	//void ChangeCharacter();
	//void ShowPopup();
	//void SaveNickName();
	//void RenderTexture();
	//void SendPacket();

private:
	//Level_Lobby에서 수행해야 하는 작업 - character 띄우고, 이름 입력과 선택을 통해서,
	//Loading scene 진입 이후 해당 캐릭터의 이름과 정보를 서버에 등록과 전송, Baren Scene으로 진입해서,
	//두 개 클라이언트 동기화 되고 이름까지 띄우는 것 확인 이후 파티와 이펙트 동기화 등등 구현
	//이거 결국 지금은 lancer master를 띄워야 하는 거잖아. 어떻게 띄우지?
	
	//캐릭터들을 vector 컨테이너에 저장한다.그 캐릭터의 클래스와 닉네임을 같이 저장한다. 
	//이거 가지고 다 사용한다.? SelectNickName() 내부에서 Update 내부에서 
	//vector<LobbyCharacterInfo> m_vecCharactersInfo = {};

	vector<LobbyCharacterInfo> m_vecCharacters = {};

	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	int32_t m_iSelectedCharacterIndex = { -1 };
	char_t m_szNickName[64] = {};
	bool_t m_isEnterRequested = { false };
	string m_strNetworkStatus = { "Select a character and enter a nickname." };

public:
	static unique_ptr<CLevel_Lobby> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
