// 폰트 한 개의 소유 수명과 측정/그리기를 구현한다.
#include "CustomFont.h"
// 문자 스프라이트 제출과 .spritefont 자원 로드·문자열 측정을 사용한다.
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"

// 화면 문자열을 그릴 때 현재 제품 viewport 크기를 조회한다.
#include "GameInstance.h"

// 폰트 자원을 만드는 동안 장치/context가 살아 있도록 COM 참조를 보관한다.
CCustomFont::CCustomFont(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

// font/batch와 장치/context는 각 소유 포인터가 해제한다.
CCustomFont::~CCustomFont()
{
}

// Font_Manager의 Create 경로에서 호출한다. 입력을 확인한 뒤 batch/font를 구성하고 예외 실패를 HRESULT로 돌려준다.
HRESULT CCustomFont::Initialize(const tchar_t* pFontFilePath)
{
	if (nullptr == pFontFilePath || nullptr == m_pDevice || nullptr == m_pContext)
		return E_INVALIDARG;

	try
	{
		m_pBatch = make_unique<SpriteBatch>(m_pContext.Get());
		m_pFont = make_unique<SpriteFont>(m_pDevice.Get(), pFontFilePath);
	}
	catch (const std::exception& exception)
	{
		OutputDebugStringA((std::string("Custom font creation failed: ") +
			exception.what() + "\n").c_str());
		m_pFont.reset();
		m_pBatch.reset();
		return E_FAIL;
	}
	catch (...)
	{
		OutputDebugStringA("Custom font creation failed with an unknown exception.\n");
		m_pFont.reset();
		m_pBatch.reset();
		return E_FAIL;
	}

	return S_OK;
}

// Font_Manager의 가시성 판단을 통과한 문자열을 그린다. viewport를 화면 크기로 설정한 뒤 이 함수 안에서 Begin/End를 짝지어 제출한다.
void CCustomFont::Draw(
    const tchar_t* pText, const float2_t& vPosition,
    fvector_t vColor, f32_t fRotation,
    const float2_t& vOrigin, f32_t fScale)
{
    /* SpriteBatch는 현재 viewport에서 투영 크기를 얻는다. 앞선 shadow/bloom pass가
    크기를 바꾸었다가 복구하는 순서에 의존하지 않도록 실제 화면 viewport를 여기서 다시 설정한다. */
    const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
    const D3D11_VIEWPORT ViewportDesc{ 0.f, 0.f, vViewportSize.x, vViewportSize.y, 0.f, 1.f };
    m_pContext->RSSetViewports(1, &ViewportDesc);

    m_pBatch->Begin();

    // 아래는 DrawString 인수 순서를 확인하기 위한 API 선언 참고다.
    /*  
    _In_ SpriteBatch* spriteBatch,
    _In_z_ wchar_t const* text,
    XMFLOAT2 const& position,
    FXMVECTOR color = Colors::White, float rotation = 0, XMFLOAT2 const& origin = Float2Zero, float scale = 1,
    SpriteEffects effects = SpriteEffects_None, float layerDepth = 0

    */

    /* vOrigin은 0~1 원점 비율(0.5는 중앙)이고 DrawString은 문자열 내부의 픽셀 오프셋을 받는다.
    실제 측정 폭·높이를 곱해 서로 다른 길이의 문자열도 요청한 기준점에 맞춰 배치한다. */
    const XMVECTOR vMeasured = m_pFont->MeasureString(pText);
    const float2_t vPixelOrigin(XMVectorGetX(vMeasured) * vOrigin.x, XMVectorGetY(vMeasured) * vOrigin.y);

    m_pFont->DrawString(m_pBatch.get(), pText, vPosition, vColor, fRotation, vPixelOrigin, fScale);

    m_pBatch->End();
}

// 클립 판정과 UI 배치에 쓸 원래 픽셀 크기다. 회전과 배율은 이 측정값에 포함되지 않는다.
float2_t CCustomFont::Measure(const tchar_t* pText) const
{
    if (nullptr == m_pFont)
        return float2_t(0.f, 0.f);

    const XMVECTOR vMeasured = m_pFont->MeasureString(pText);
    return float2_t(XMVectorGetX(vMeasured), XMVectorGetY(vMeasured));
}

// 초기화 실패한 객체를 외부에 등록하지 않는다. 성공한 소유 객체만 호출자에게 이동한다.
unique_ptr<CCustomFont> CCustomFont::Create(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext,
    const tchar_t* pFontFilePath)
{
    auto pInstance = unique_ptr<CCustomFont>(new CCustomFont(pDevice, pContext));

    if (FAILED(pInstance->Initialize(pFontFilePath)))
    {
        MSG_BOX("Failed to Created : CCustomFont");
        return nullptr;
    }

    return pInstance;
}
