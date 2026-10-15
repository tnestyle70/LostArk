#pragma once

// Level 객체 수명과 기본 Transform 컴포넌트, 렌더 그룹 진입점을 상속한다.
#include "GameObject.h"

NS_BEGIN(Engine)

// 화면 픽셀 위치·크기를 UI용 Transform/직교 투영에 연결하는 공통 기반이다.
// 실제 이미지·텍스트·입력 명령 처리는 Client의 파생 객체와 UI 서비스가 맡는다.
class ENGINE_DLL CUIObject abstract : public CGameObject
{
public:
	// 기본 객체 설명에 UI 폭·높이와 화면 기준 중심 위치를 더한다. Initialize 호출 동안 유효한 입력을 전달해야 한다.
	typedef struct tagUIObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		f32_t		fSizeX, fSizeY;		
		f32_t		fX, fY;
	}UIOBJECT_DESC;

protected:
	CUIObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CUIObject();

public:
	// 현재 기본 구현은 추가 자원을 만들지 않는다. 파생 Prototype의 자원 준비는 별도로 수행한다.
	virtual HRESULT Initialize_Prototype() override;
	// UIOBJECT_DESC를 읽고 기본 객체 초기화 뒤 Transform과 VIEW/PROJ를 구성한다. null 입력이나 실패 rollback은 이 함수가 처리하지 않는다.
	// 기본 클래스는 초기화 시점의 viewport만 사용한다. 이후 크기 변경에 맞춘 재배치는 Client 파생 객체의 책임이다.
	virtual HRESULT Initialize(void* pArg) override;
	// 기본 update 훅들은 비어 있다. 파생 객체가 프레임 처리와 렌더 등록을 담당한다.
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	// 기본 구현은 draw 없이 S_OK다. 실제 리소스 바인딩과 draw는 파생 Render에서 수행한다.
	virtual HRESULT Render();
	/* RENDERGROUP::UI 내부의 표시 순서다. 화면 소유자가 정한 layer로 Renderer가 정렬해
	객체 생성 순서가 화면의 앞뒤 관계를 바꾸지 않도록 한다. */
	void Set_UISortLayer(int32_t iLayer)
	{
		m_iUISortLayer = iLayer;
	}
	virtual int32_t Get_UISortLayer() const override
	{
		return m_iUISortLayer;
	}

protected:
	// Renderer가 작은 layer부터 그리고 같은 layer 안에서는 제출 순서를 유지한다.
	int32_t				m_iUISortLayer = 0;
	f32_t				m_fX{}, m_fY{}, m_fSizeX{}, m_fSizeY{};
	// UI가 소유한 행렬 저장소다. 기본 Initialize는 VIEW/PROJ만 채우고 월드 변환은 Transform 컴포넌트가 소유한다.
	float4x4_t			m_TransformMatrices[ETOUI(D3DTS::END)] = {};

protected:
	// 파생 Render가 지정한 행렬 슬롯을 CShader에 전달한다. 슬롯 범위 검증·전치·viewport 재계산은 수행하지 않는다.
	HRESULT Bind_ShaderResource(
		shared_ptr<class CShader> pShader,
		const char_t* pConstantName, D3DTS eType);
};

NS_END