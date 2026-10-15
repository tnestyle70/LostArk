#pragma once

// Client namespace와 UI의 기반 클래스 CUIObject를 사용한다.
#include "Client_Defines.h"
#include "UIObject.h"

// Component는 shared_ptr 멤버로만 보관하므로 이름을 먼저 선언한다.
NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

// CUIObject를 상속한 화면 이미지·caption 객체다. 같은 원형에서 다른 텍스처의 UI를 복제한다.
class CUI_Sprite final : public CUIObject
{
public:
	// Clone의 입력이다. 기본 UI 중심·크기에 이미지 tag 또는 caption과 기준 해상도를 더한다.
	typedef struct tagUISpriteDesc : public CUIObject::UIOBJECT_DESC
	{
		wstring_t	strTextureTag;
        wstring_t caption, captionFont;
        float2_t referenceResolution = {0.f, 0.f};
	}UI_SPRITE_DESC;

private:
	// 외부 직접 생성을 제한하고 Create 또는 Clone에서 장치 참조를 기반 CUIObject에 전달한다.
	CUI_Sprite(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	// Component·대체 SRV의 보유 참조와 기반 객체 상태를 자동 정리한다.
	virtual ~CUI_Sprite();

public:
	// 원형 등록 준비는 추가 자원 없이 성공한다.
	virtual HRESULT Initialize_Prototype() override;
	// Clone의 UI_SPRITE_DESC를 읽어 기반 Transform과 이미지 Component 또는 caption을 준비한다.
	virtual HRESULT Initialize(void* pArg) override;
	// Layer 갱신에서 보이는 sprite를 UI 또는 SCENE_UI 렌더 목록에 등록한다.
	virtual void Late_Update(f32_t fTimeDelta) override;
	// Renderer가 호출해 caption이나 이미지 quad를 그린다.
	virtual HRESULT Render() override;
	// SCENE_UI의 렌더 실패는 이 sprite를 숨겨 격리하고 나머지는 오류를 전달한다.
	virtual HRESULT Render_Group(RENDERGROUP group) override;

public:
	// 문서 기준 중심과 크기를 바꾸고 Transform을 갱신한다.
	void Set_Rect(f32_t fCenterX, f32_t fCenterY, f32_t fSizeX, f32_t fSizeY);

	// 레이아웃과 화면 소유자가 실행 중 색·반전·합성 방식을 바꾼다. JSON 원본은 수정하지 않는다.
	void Set_Tint(const float4_t& vTint);
	// 원래 색에 곱할 RGBA다. 흰색을 주면 원래 색을 그대로 사용한다.
	void Set_TintMultiplier(const float4_t& vTintMultiplier);
	void Set_FlipX(bool_t bFlipX);
	void Set_Additive(bool_t bAdditive);
	// U축의 표시 비율이다. 1은 전체 표시이며 quad 크기를 줄이지 않고 셰이더에서 잘라낸다.
	void Set_FillRatio(f32_t fFillRatio);
	// 12시에서 시계 방향으로 표시할 부채꼴 비율이다. 스킬 쿨타임 표현에 사용한다.
	void Set_ArcRatio(f32_t fArcRatio);
	// offset과 scale로 이미지의 UV 영역을 골라 미니맵을 이동·확대한다.
	void Set_UVWindow(const float2_t& vOffset, const float2_t& vScale);
	// UV 영역을 radian 단위로 회전한다. aspect는 텍스처가 나타내는 월드 폭/높이다.
	void Set_UVRotation(f32_t fRadians, f32_t fAspect);
	// 사각형 중심을 기준으로 화면 시계 방향의 degree 회전을 지정한다.
	void Set_Rotation(f32_t fDegrees);
	// false면 렌더 등록·그리기를 생략한다. 색·텍스처·Transform 값은 보존한다.
	void Set_Visible(bool_t bVisible);
	// 컷신 fade 등 지정된 UI는 일반 HUD가 숨겨져도 표시할 수 있다.
	void Set_CinematicOverlay(bool_t overlay) { m_bCinematicOverlay = overlay; }
	void Set_ScenePresentation(bool_t enabled) { m_bScenePresentation = enabled; }
	// 이미 로드한 SRV를 ComPtr로 보유한다. nullptr면 Component의 원래 텍스처를 사용한다.
	void Set_Texture(ComPtr<ID3D11ShaderResourceView> pOverrideSRV);

private:
	wstring_t						m_strTextureTag;

	// Component 참조는 shared_ptr로 보유하고 대체 이미지 SRV는 아래 ComPtr로 보유한다.
	shared_ptr<CShader>				m_pShaderCom = { nullptr };
	shared_ptr<CTexture>			m_pTextureCom = { nullptr };
	shared_ptr<CVIBuffer_Rect>		m_pVIBufferCom = { nullptr };

	// 문서 해상도와 마지막 적용 viewport를 기억해 창 크기가 바뀌면 다시 배치한다.
	float2_t m_vReferenceResolution = {0.f, 0.f};
	float2_t m_vAppliedViewport = {0.f, 0.f};
	float4_t						m_vTint = float4_t(1.f, 1.f, 1.f, 1.f);
	float4_t						m_vTintMultiplier = float4_t(1.f, 1.f, 1.f, 1.f);
	bool_t							m_bFlipX = false;
	bool_t							m_bAdditive = false;
	f32_t							m_fFillRatio = 1.f;
	f32_t							m_fArcRatio = 1.f;
	float2_t						m_vUVOffset = float2_t(0.f, 0.f);
	float2_t						m_vUVScale = float2_t(1.f, 1.f);
	f32_t							m_fUVRotation = 0.f;
	f32_t							m_fUVAspect = 1.f;
	f32_t							m_fRotationDeg = 0.f;
	bool_t							m_bVisible = true;
	bool_t m_bCinematicOverlay = false;
	bool_t m_bScenePresentation = false;
	// 장면 UI 렌더 실패는 해당 sprite를 재생성할 때까지 숨긴다.
	bool_t m_bScenePresentationFailed = false;
    wstring_t m_Caption, m_CaptionFont;
	ComPtr<ID3D11ShaderResourceView>	m_pOverrideTextureSRV;

private:
	// 이미지 경로의 Texture·Shader·사각형 버퍼 원형을 복제한다.
	HRESULT Ready_Components();
	// 현재 viewport의 행렬·이미지·색·UV 값을 셰이더 입력에 연결한다.
	HRESULT Bind_ShaderResources();
	// 현재 사각형과 회전으로 축을 다시 만든다. Set_Rect와 Set_Rotation이 함께 사용한다.
	void Apply_Transform();

public:
	// 원형 등록용 unique_ptr를 만든다. 현재 구현은 준비 실패 메시지 뒤에도 객체를 반환한다.
	static unique_ptr<CUI_Sprite> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// 원형을 복사하고 인스턴스 입력을 적용한다. 현재 구현은 초기화 실패 뒤에도 객체를 반환한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
