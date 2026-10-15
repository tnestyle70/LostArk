#pragma once

// STATIC Level 구분과 Client 이름 공간을 사용한다.
#include "Client_Defines.h"
// DX11 장치·Context의 ComPtr와 입력 상태 bool_t를 사용한다.
#include "Engine_Defines.h"

// 레이아웃은 unique_ptr로 소유하고 아이템 ID는 string 값으로 보관한다.
#include <memory>
#include <string>

NS_BEGIN(Client)

// 헤더에는 포인터만 선언하고 슬롯 생성·이동은 CPP에서 처리한다.
class CUILayoutRuntime;

// MainApp이 소유하는 아이템·탈것 아이콘 운반 표시다. 선택한 ID 값과 마우스를 따르는 sprite를 보관한다.
// 다음 클릭을 MainApp에 알려 단축 슬롯 등록을 맡기며 인벤토리나 Server 상태를 바꾸지 않는다.
class CQuickSlotDragView final
{
public:
	enum class PAYLOAD { NONE, ITEM, VEHICLE };

public:
	// MainApp 초기화에서 STATIC Layer_UI에 아이콘을 만들고 숨긴다.
	CQuickSlotDragView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// 레이아웃과 ID 값·보유 참조를 정리한다. Layer의 sprite 참조는 별도다.
	~CQuickSlotDragView();

public:
	// 선택한 아이템 ID로 운반을 시작한다. 이미지 경로가 비면 기존 이미지를 유지한다.
	void Begin_ItemCarry(const string& strItemId, const string& strIconPath);
	// 아이템 ID를 비우고 탈것 ID로 운반 종류를 바꾼다.
	void Begin_VehicleCarry(uint32_t iVehicleId, const string& strIconPath);
	// ID·운반 종류를 비우고 아이콘을 숨긴다.
	void Cancel();

	// MainApp이 놓을 슬롯을 찾을 때 읽는 현재 운반 데이터다.
	bool_t Is_Carrying() const { return PAYLOAD::NONE != m_ePayload; }
	PAYLOAD Get_Payload() const { return m_ePayload; }
	const string& Get_ItemId() const { return m_strItemId; }
	uint32_t Get_VehicleId() const { return m_iVehicleId; }

	// 아이콘을 커서로 옮기고 Esc를 처리한다. 운반 시작 직후 첫 Update는 클릭을 알리지 않는다.
	// 이후 왼쪽 눌림이면 true이며, 어디에 놓고 운반을 끝낼지는 MainApp이 결정한다.
	bool_t Update();
	// payload를 보존한 채 아이콘만 숨긴다.
	void Hide();

private:
	// 아이콘 레이아웃을 소유한다. 아이템·탈것 객체의 포인터는 보관하지 않는다.
	unique_ptr<CUILayoutRuntime>	m_pView;
	PAYLOAD							m_ePayload = PAYLOAD::NONE;
	string							m_strItemId;
	uint32_t						m_iVehicleId = 0u;
	// 집어 든 입력을 다시 놓기로 처리하지 않도록 첫 Update와 Esc의 이전 눌림을 기록한다.
	bool_t							m_bBeganThisFrame = false;
	bool_t							m_bEscapeDownLastFrame = false;
};

NS_END
