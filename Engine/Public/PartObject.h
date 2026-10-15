#pragma once

// 부착 객체가 자기 Transform과 Component를 가지면서 기존 생성·복제·렌더 계약을 따르도록 한다.
#include "GameObject.h"

NS_BEGIN(Engine)

// 부모 행렬을 차용해 자기 local 행렬과 합성하는 부착 객체의 기반이다. 소유자는 보통 ContainerObject다.
class ENGINE_DLL CPartObject abstract : public CGameObject
{
public:
	typedef struct tagPartObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		// 초기화 이후에도 매 갱신에서 읽는다. 부모는 Part가 쓰는 동안 이 메모리의 주소와 수명을 유지해야 한다.
		const float4x4_t* pParentMatrix = { nullptr };
	}PARTOBJECT_DESC;

protected:
	CPartObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CPartObject();

public:
	// 기반 Part에는 공유 자원 준비가 없어 S_OK다. 실제 메시/재질 준비는 파생 객체의 책임이다.
	virtual HRESULT Initialize_Prototype() override;
	// pArg는 유효한 PARTOBJECT_DESC여야 한다. nullptr/잘못된 타입을 여기서 검사하지 않는다.
	virtual HRESULT Initialize(void* pArg) override;
	// 기반 세 callback은 비어 있다. 부모 Container가 경과 초를 전달하며 부착 행렬 계산 시점은 구체 Part가 정한다.
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	// 기반 Render는 그리기 없이 성공한다. 합성 행렬을 사용하는 실제 draw는 구체 Part가 구현한다.
	virtual HRESULT Render();

protected:
	// 결과 행렬은 이 Part가 소유하고, 부모 행렬 포인터는 소유하지 않는다.
	float4x4_t			m_CombinedWorldMatrix = {};
	const float4x4_t*	m_pParentMatrix = {};

protected:
	// 행 벡터 규약으로 ChildMatrix * ParentMatrix를 계산한다. 두 입력은 같은 좌표계/단위를 사용해야 한다.
	// 보충: ChildMatrix의 출력 공간이 ParentMatrix의 입력 공간으로 이어져야 한다. 자식 local→parent→world 합성이며 두 행렬의 축 규약과 길이 단위가 맞아야 한다.
	HRESULT Update_CombinedWorldMatrix(fmatrix_t ChildMatrix);
	// 직전에 계산한 합성 행렬을 지정 shader 변수로 전달한다. shader와 이름의 유효성은 호출자가 보장한다.
	HRESULT Bind_WorldMatrix(shared_ptr<class CShader> pShader, const char_t* pConstantName);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;


};

NS_END