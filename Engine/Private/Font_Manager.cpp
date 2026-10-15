// 태그별 폰트 등록과 화면 문자열 가시성 조건을 구현한다.
#include "Font_Manager.h"

// 실제 폰트 파일 생성·측정·SpriteBatch 제출을 위임할 소유 타입이다.
#include "CustomFont.h"

// 폰트 생성이 끝날 때까지 사용할 장치/context의 강한 참조를 유지한다.
CFont_Manager::CFont_Manager(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

// 등록된 폰트와 장치/context는 map 및 COM 소유 멤버의 소멸로 정리한다.
CFont_Manager::~CFont_Manager()
{
}

// GameInstance의 등록 경로가 호출한다. 중복 태그와 생성 실패를 먼저 확인해 기존 map을 보존한다.
HRESULT CFont_Manager::Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath)
{
	if (nullptr != Find_Font(strFontTag))
		return E_FAIL;

	auto		pFont = CCustomFont::Create(m_pDevice.Get(), m_pContext.Get(), pFontFilePath);
	if (nullptr == pFont)
		return E_FAIL;

	m_Fonts.emplace(strFontTag, move(pFont));

	return S_OK;
}

// GameInstance가 요청한 태그를 찾아 클립 조건을 검사한다. 빠진 폰트나 숨겨진 문자열은 제출하지 않는다.
void CFont_Manager::Draw(
	const wstring& strFontTag, const tchar_t* pText,
	const float2_t& vPosition, fvector_t vColor,
	f32_t fRotation, const float2_t& vOrigin, f32_t fScale)
{
	auto		pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return;

	if (m_isClipOutEnabled || m_isClipInEnabled)
	{
		// 측정 픽셀에 배율과 0~1 원점 비율을 적용해 화면상의 사각형을 만든다.
		// 회전각은 계산에 포함하지 않으므로 회전된 글자의 정확한 윤곽 판정은 아니다.
		const float2_t vMeasured = pFont->Measure(pText);
		const f32_t fWidth = vMeasured.x * fScale;
		const f32_t fHeight = vMeasured.y * fScale;
		const f32_t fLeft = vPosition.x - fWidth * vOrigin.x;
		const f32_t fTop = vPosition.y - fHeight * vOrigin.y;

		// 가리는 창 중 하나와 면적이 겹치면 문자열 전체를 생략한다. 경계만 접하는 경우는 겹침으로 보지 않는다.
		if (m_isClipOutEnabled)
		{
			for (const float4_t& vRect : m_ClipOutRects)
			{
				const bool_t isOverlapping =
					fLeft < vRect.x + vRect.z && fLeft + fWidth > vRect.x &&
					fTop < vRect.y + vRect.w && fTop + fHeight > vRect.y;
				if (isOverlapping)
					return;
			}
		}

		// 내부 표시 영역을 설정했다면 전체 문자열 사각형이 포함되어야 그린다.
		if (m_isClipInEnabled)
		{
			const bool_t isInside =
				fLeft >= m_vClipInRect.x && fLeft + fWidth <= m_vClipInRect.x + m_vClipInRect.z &&
				fTop >= m_vClipInRect.y && fTop + fHeight <= m_vClipInRect.y + m_vClipInRect.w;
			if (!isInside)
				return;
		}
	}

	pFont->Draw(pText, vPosition, vColor, fRotation, vOrigin, fScale);
}

// 기존 가림 목록을 지우고 한 사각형으로 교체한다. ClipIn 상태와는 독립이다.
void CFont_Manager::Set_ClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	m_ClipOutRects.clear();
	Add_ClipOutRect(fX, fY, fWidth, fHeight);
}

// 가림 조건을 활성화하고 화면 픽셀 사각형을 추가한다. 위치/크기 값의 정규화는 호출자가 맡는다.
void CFont_Manager::Add_ClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	m_isClipOutEnabled = true;
	m_ClipOutRects.push_back(float4_t(fX, fY, fWidth, fHeight));
}

// 문자열 전체가 들어와야 하는 화면 픽셀 영역을 교체하고 활성화한다.
void CFont_Manager::Set_ClipInRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	m_isClipInEnabled = true;
	m_vClipInRect = float4_t(fX, fY, fWidth, fHeight);
}

// 없는 태그는 0 크기이며, 등록된 폰트는 자체 측정값을 반환한다.
float2_t CFont_Manager::Measure(const wstring& strFontTag, const tchar_t* pText)
{
	auto		pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return float2_t(0.f, 0.f);

	return pFont->Measure(pText);
}

// 태그는 map에서 소유한다. 찾은 폰트의 공유 소유 참조를 돌려주며 새 폰트를 생성하지 않는다.
shared_ptr<class CCustomFont> CFont_Manager::Find_Font(const wstring& strFontTag)
{
	auto	iter = m_Fonts.find(strFontTag);

	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;	
}

// 추가 로드 없이 빈 관리자를 구성해 GameInstance의 소유권으로 넘긴다.
unique_ptr<CFont_Manager> CFont_Manager::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	return unique_ptr<CFont_Manager>(new CFont_Manager(pDevice, pContext));
}
