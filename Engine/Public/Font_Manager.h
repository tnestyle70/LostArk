#pragma once

// 장치/context 참조, 태그 map과 화면 사각형의 수학 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 태그별 폰트 저장소와 문자열 가시성 조건이다.
// 클립 상태는 관리자 전체에 공유되므로 각 UI 소비자가 자신의 그리기 경계에서 설정/해제한다.
class CFont_Manager final 
{
private:
	CFont_Manager(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	~CFont_Manager();

public:
	// 중복 태그를 거부하고 폰트 생성 성공 후 등록한다. 생성 실패는 기존 등록 항목을 바꾸지 않는다.
	HRESULT Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath);
	// 태그를 찾고 문자열 전체의 클립 통과 여부를 검사한 뒤 CCustomFont에 제출한다.
	// 회전은 아래 사각형 판정에 반영하지 않으며 GPU scissor로 글자의 일부를 자르는 기능은 아니다.
	void Draw(
		const wstring& strFontTag, const tchar_t* pText,
		const float2_t& vPosition, fvector_t vColor,
		f32_t fRotation, const float2_t& vOrigin, f32_t fScale);
	// 배율 전 픽셀 크기를 조회한다. 없는 태그는 0 크기다.
	float2_t Measure(const wstring& strFontTag, const tchar_t* pText);
	/* 다른 창이 가리는 화면 픽셀 영역이다. Draw는 이 영역과 겹치는 문자열 전체를 생략한다.
	텍스트가 스프라이트 뒤에 그려지므로 UI 소유자가 앞쪽 창 영역을 설정하고 자신의 표시 경계에서 해제한다. */
	void Set_ClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	/* 앞쪽 창이 여러 개이면 사각형을 누적한다. Clear_ClipOutRect까지 모두 적용되며,
	Set_ClipOutRect는 기존 목록을 비운 뒤 하나를 추가한다. */
	void Add_ClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_ClipOutRect()
	{
		m_isClipOutEnabled = false;
		m_ClipOutRects.clear();
	}
	/* 문자열 전체가 들어와야 하는 화면 픽셀 영역이다. 일부라도 벗어나면 Draw를 생략해
	스크롤 목록의 글자가 위아래 프레임 밖으로 나오지 않도록 한다. */
	void Set_ClipInRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_ClipInRect()
	{
		m_isClipInEnabled = false;
	}

private:
	// 폰트 생성에 사용할 장치/context와 등록 폰트의 소유 참조다.
	ComPtr<ID3D11Device>										m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>									m_pContext = { nullptr };
	map<const wstring_t, shared_ptr<class CCustomFont>>			m_Fonts;
	bool_t														m_isClipOutEnabled = false;
	vector<float4_t>											m_ClipOutRects;			/* 화면 픽셀의 좌상단 x/y와 폭/높이 */
	bool_t														m_isClipInEnabled = false;
	float4_t													m_vClipInRect = {};		/* 화면 픽셀의 좌상단 x/y와 폭/높이 */


private:
	// 등록 항목의 shared_ptr 복사본을 반환해 호출 중 수명을 유지한다. 내부 동시 접근 잠금은 없다.
	shared_ptr<class CCustomFont> Find_Font(const wstring& strFontTag);

public:
	// 빈 저장소를 만든다. 폰트 파일은 Add_Font에서 요청할 때 읽는다.
	static unique_ptr<CFont_Manager> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

};

NS_END