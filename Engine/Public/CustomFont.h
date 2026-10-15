#pragma once

// 장치/context 참조, 수학 타입과 소유 포인터를 공통 엔진 정의로 가져온다.
#include "Engine_Defines.h"
// Draw의 기본 색상 Colors::White를 제공한다.
#include <DirectXColors.h>

// 공개 헤더에는 이름만 선언하고 DirectXTK 구현 헤더는 CPP에서 포함한다.
namespace DirectX { inline namespace DX11 { class SpriteBatch; class SpriteFont; } }

NS_BEGIN(Engine)

// 폰트 파일 한 개와 그리기용 SpriteBatch를 소유한다. CFont_Manager가 태그별로 관리한다.
class CCustomFont final 
{
private:
	CCustomFont(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	~CCustomFont();

public:
	// Create가 SpriteBatch/Font를 구성한다. 예외가 발생하면 두 멤버를 비우므로 재초기화 실패가 이전 폰트를 보존하는 계약은 아니다.
	// null 입력·장치 검사는 자원 교체 전에 끝난다. 이 입력 실패에서는 기존 멤버를 건드리지 않는다.
	HRESULT Initialize(const tchar_t* pFontFilePath);
	// 화면 픽셀 위치에 문자열을 그린다. vOrigin은 문자열 크기에 대한 비율이고 fRotation은 라디안이다.
	// 호출자는 초기화 성공·유효한 문자열과 context의 렌더 스레드 접근을 보장한다.
	void Draw(
		const tchar_t* pText, const float2_t& vPosition,
		fvector_t vColor = Colors::White, f32_t fRotation = 0.f,
		const float2_t& vOrigin = float2_t(0.f, 0.f), f32_t fScale = 1.f);
	// 배율 적용 전 문자열 픽셀 크기를 반환한다. 폰트가 없으면 0 크기다.
	float2_t Measure(const tchar_t* pText) const;

private:
	// 장치/context는 강한 COM 참조이며 batch/font는 이 객체가 단독 소유한다.
	ComPtr<ID3D11Device>					m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>				m_pContext = { nullptr };
	unique_ptr<SpriteBatch>					m_pBatch = { nullptr };
	unique_ptr<SpriteFont>					m_pFont = { nullptr };

public:
	// 완성한 폰트만 관리자에 넘긴다. 실패 시 임시 객체를 정리하고 nullptr를 반환한다.
	static unique_ptr<CCustomFont> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pFontFilePath);
};

NS_END