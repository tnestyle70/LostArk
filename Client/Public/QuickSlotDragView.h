#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Click-to-carry for the combat HUD quick slots: a click on an inventory item or a vehicle
window row picks its icon up, the icon rides the mouse (one CUI_Sprite slot,
Data/UI/Common/QuickSlotDrag_Layout.json, constructed after every other runtime window so it
draws on top of them), and the next click either lands on a quick slot (CMainApp binds the
payload there) or lets go of it. Esc lets go too. What is being carried is data only -- this
class never touches the HUD, the inventory or the Server. */
class CQuickSlotDragView final
{
public:
	enum class PAYLOAD { NONE, ITEM, VEHICLE };

public:
	CQuickSlotDragView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CQuickSlotDragView();

public:
	void Begin_ItemCarry(const string& strItemId, const string& strIconPath);
	void Begin_VehicleCarry(uint32_t iVehicleId, const string& strIconPath);
	void Cancel();

	bool_t Is_Carrying() const { return PAYLOAD::NONE != m_ePayload; }
	PAYLOAD Get_Payload() const { return m_ePayload; }
	const string& Get_ItemId() const { return m_strItemId; }
	uint32_t Get_VehicleId() const { return m_iVehicleId; }

	/* Per frame. Moves the icon under the mouse, claims the mouse for the carry and lets go on
	Esc. Returns true on the frame a left-click edge happens while carrying -- the drop click;
	the click that started the carry (same frame as Begin_*) is not reported. */
	bool_t Update();
	/* Forces the sprite invisible without changing the payload (HUD gate closed). */
	void Hide();

private:
	unique_ptr<CUILayoutRuntime>	m_pView;
	PAYLOAD							m_ePayload = PAYLOAD::NONE;
	string							m_strItemId;
	uint32_t						m_iVehicleId = 0u;
	bool_t							m_bBeganThisFrame = false;
	bool_t							m_bEscapeDownLastFrame = false;
};

NS_END
