#pragma once

// D3D11 텍스처/view와 COM 소유 포인터, 화면 변환에 사용할 수학 타입을 가져온다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// 렌더링 결과를 기록하는 텍스처 한 개와 RTV/SRV를 소유한다.
// Target_Manager의 태그 목록과 MRT 목록이 같은 객체를 공유하며 resize는 객체 identity를 유지한다.
class CRenderTarget final 
{
private:
	CRenderTarget(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	~CRenderTarget();

public:
	// view의 강한 COM 참조를 돌려준다. 이후 resize에서 객체의 view가 교체되어도 이미 받은 참조는 이전 view를 가리킨다.
	ComPtr<ID3D11RenderTargetView> Get_RTV()
	{
		return m_pRTV;
	}
	// 텍스처를 읽는 소비자에게 현재 SRV의 강한 참조를 전달한다.
	ComPtr<ID3D11ShaderResourceView> Get_SRV() const;

public:
	// Create 또는 별도 임시 resize 객체에서 호출한다. 멤버를 순차 생성하므로 이 함수 자체가 기존 자원을 보존하는 재초기화 계약은 아니다.
	HRESULT Initialize(
		uint32_t iWidth, uint32_t iHeight,
		DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	// 기존 형식/clear 색으로 새 크기의 임시 객체를 만든다. 실패는 현재 자원과 output을 보존한다.
	HRESULT Stage_Resize(uint32_t width, uint32_t height, shared_ptr<CRenderTarget>& output) const;
	// 검증된 임시 객체와 GPU 자원을 교환한다. 이전 자원은 staged가 소유하며 같은 후보를 다시 적용하면 재교환되므로 한 번만 소비한다.
	void Commit_Resize(CRenderTarget& staged, f32_t viewportDeltaX, f32_t viewportDeltaY) noexcept;
	// 읽기용 view를 CShader의 지정 변수에 연결한다. 출력으로 사용 중인 자원의 해제 순서는 상위 pass가 맡는다.
	HRESULT Bind_SRV(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	// 초기화 때 저장한 색으로 현재 RTV를 지운다. viewport나 depth는 변경하지 않는다.
	void Clear();
	// 이 target의 텍스처를 입력 pTexture2D에 복사한다. 목적지 크기/형식 호환성과 유효성은 호출자가 보장한다.
	HRESULT Copy_Resource(ComPtr<ID3D11Texture2D> pTexture2D);

#ifdef _DEBUG
public:
	// Debug 미리보기 사각형의 중심/크기를 화면 픽셀에서 UI 중심 좌표로 변환한다.
	HRESULT Ready_DebugDesc(f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	// Debug 사각형에 SRV를 바인딩하고 DEFERRED::DEBUG pass로 그린다. view/projection과 출력 pass 준비는 호출자 책임이다.
	HRESULT Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);

#endif

private:
	// target이 장치/context의 COM 참조를 보유한다. Clear/Copy/Debug draw의 호출 순서와 스레드 접근은 상위 렌더 경로가 관리한다.
	ComPtr<ID3D11Device>						m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>					m_pContext = { nullptr };

	// 하나의 GPU 텍스처를 출력 RTV와 입력 SRV가 함께 참조한다. 모두 이 객체가 COM 소유 참조를 유지한다.
	ComPtr<ID3D11Texture2D>						m_pTexture2D = { nullptr };
	ComPtr<ID3D11RenderTargetView>				m_pRTV = { nullptr };
	ComPtr<ID3D11ShaderResourceView>			m_pSRV = { nullptr };

private:
	float4_t			m_vClearColor = {};

#ifdef _DEBUG
private:
	float4x4_t			m_WorldMatrix = {};
#endif

public:
	// 초기화 성공 객체만 반환한다. 실패한 임시 객체와 부분 생성 자원은 소유 포인터 소멸로 정리한다.
	static shared_ptr<CRenderTarget> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iWidth, uint32_t iHeight,
		DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
};

NS_END
