#pragma once

#include <memory>
#include <string>

//Client 이름 공간과 공통 Level 선언을 사용한다.
#include "Client_Defines.h"
//Layer/Prototype 수명과 장치·Transform 초기화를 제공하는 CGameObject를 상속한다.
#include "GameObject.h"

//Engine의 선 그리기 자원을 포인터로 보관한다. 실제 정의는 CPP에서 사용한다.
namespace Engine
{
	class CWireBoxRenderer;
}

NS_BEGIN(Client)

//MapTool과 Level 진단에 사용하는 방향성 상자다. 실제 gameplay 충돌을 판정하지 않는다.
class CTrigger_Box final : public CGameObject
{
public:
	//Clone 입력이다. stable placement ID와 월드 중심·반크기·Y축 각도,
	//활성화 여부, collision box 여부 저장
	struct TRIGGER_BOX_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		std::string placementId;
		float3_t position = {};
		float3_t halfExtents = float3_t(1.f, 1.f, 1.f);
		f32_t yawDegrees = {};
		bool_t isEnabled = false;
		bool_t isCollisionBox = false;
	};

private:
	//Create/Clone에서만 생성해 기반 객체에 장치·Context 참조를 전달한다.
	CTrigger_Box(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	//멤버 smart pointer와 기반 객체가 보유한 렌더 자원 참조를 정리한다.
	virtual ~CTrigger_Box();

	//원형과 복제본이 공유할 Engine의 선 그리기 자원을 준비한다.
	virtual HRESULT Initialize_Prototype() override;

	//기반 객체 초기화 뒤 descriptor를 검사하고 월드 상자 경계를 만든다.
	virtual HRESULT Initialize(void* pArg) override;

	//저작 표시가 켜진 복제본만 NONLIGHT 렌더 그룹에 등록한다.
	virtual void Late_Update(f32_t fTimeDelta) override;

	//선택·활성·collision 구분으로 색을 골라 Engine에 상자 그리기를 요청한다.
	virtual HRESULT Render() override;

	//유효한 descriptor만 교체한다. 검증 실패는 기존 배치와 경계를 보존한다.
	bool_t Apply_Descriptor(const TRIGGER_BOX_DESC& desc);

	//MapTool의 표시·선택 변경을 받는다. 원본 데이터의 활성 여부는 바꾸지 않는다.
	void Set_AuthoringVisible(bool_t isVisible);
	void Set_Selected(bool_t isSelected);

	//MapTool이 대응 문서 행을 찾는 stable ID의 차용 참조다.
	const std::string& Get_PlacementId() const;

private:
	//ID 존재, 유한 위치·각도와 양수 반크기를 검사한다.
	static bool_t Is_ValidDescriptor(const TRIGGER_BOX_DESC& desc);

	//descriptor의 yaw degree를 quaternion으로 바꿔 BoundingOrientedBox에 반영한다.
	void Rebuild_Bounds();

private:
	//배치 입력과 그 입력에서 만든 월드 경계의 값 사본이다.
	TRIGGER_BOX_DESC m_Desc;
	BoundingOrientedBox m_Bounds;
	bool_t m_bAuthoringVisible = false;
	bool_t m_bSelected = false;

	//원형에서 준비한 선 그리기 자원을 복제본들이 공유한다. 제출은 Client 렌더 스레드에서 한다.
	std::shared_ptr<Engine::CWireBoxRenderer> m_pWireBoxRenderer = { nullptr };

public:
	//Loader가 등록할 원형을 만든다. 자원 초기화 실패는 nullptr다.
	static std::unique_ptr<CTrigger_Box> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	//MapTool의 Layer 등록이 요청한 복제본이다. 개별 입력 초기화 실패는 nullptr다.
	virtual std::shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
