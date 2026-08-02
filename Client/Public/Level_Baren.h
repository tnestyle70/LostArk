#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "Level.h"
#include "MapPlacementRuntime.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;

class CLevel_Baren final : public CLevel
{
private:
	CLevel_Baren(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Baren();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Camera(
		const wstring_t& strLayerTag);

	bool_t Bind_CameraToLocalCharacter();

private:
	/*
	 * 베른성 맵 객체들의 생성과 제거는 기존 Map Runtime이 담당한다.
	 * Network Player 수명과 섞지 않는다.
	 */
	CMapPlacementRuntime m_MapRuntime;

	/*
	 * Camera의 실제 수명은 Layer_Camera가 소유한다.
	 * Level은 카메라 설정 변경을 위해 shared_ptr을 보관한다.
	 */
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	/*
	 * Character의 실제 수명은 Layer_Player가 소유한다.
	 * Level은 현재 카메라 대상 확인을 위해 weak_ptr만 보관한다.
	 */
	weak_ptr<CCharacter> m_pCameraTarget;

	/*
	 * Network Event를 Main Thread의 Character 생성·제거로 변환한다.
	 */
	CClientReplication m_Replication;

public:
	static unique_ptr<CLevel_Baren> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END