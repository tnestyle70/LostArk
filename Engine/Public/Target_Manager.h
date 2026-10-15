#pragma once

// 태그 map/list, 공유 소유 포인터와 D3D 출력 view 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 렌더 target/MRT 저장소다. 렌더 스레드에서 pass 전환을 순차 처리한다.
// 출력 복구 슬롯은 한 세트이므로 Begin/End를 중첩 호출하는 스택으로 사용하지 않는다.
class CTarget_Manager final 
{
private:
	CTarget_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext); 
public:
	~CTarget_Manager();

public:
    // 태그별 새 픽셀 크기다. 같은 target을 묶음 안에서 두 번 요청하면 준비 단계에서 거부한다.
    struct RESIZE_REQUEST final
    {
        wstring_t TargetTag;
        uint32_t Width = 0u, Height = 0u;
    };
    // 각 pair의 첫 원소는 현재 객체, 둘째는 준비 객체다. 성공 commit 후 둘째에는 이전 GPU 자원이 남는다.
    struct RESIZE_STATE final
    {
        vector<pair<shared_ptr<class CRenderTarget>, shared_ptr<class CRenderTarget>>> Targets;
    };
    // 모든 요청을 임시 묶음에 준비한 뒤 output을 교체한다. 하나라도 실패하면 현재 target과 output을 보존한다.
    HRESULT Stage_Resize(const vector<RESIZE_REQUEST>& requests, RESIZE_STATE& output) const;
    // 준비된 객체마다 GPU 자원을 교환한다. 후보 목록을 비우지 않으므로 같은 staged는 한 번만 적용한다.
    void Commit_Resize(RESIZE_STATE& staged, f32_t viewportDeltaX, f32_t viewportDeltaY) noexcept;
    // 저장해 둔 RTV0/DSV의 소유 참조만 반환한다. context의 실제 바인딩 해제는 수행하지 않는다.
    void Release_OutputReferences() noexcept;

	// 중복 태그를 거부하고 GPU 자원 생성 성공 후 target map에 등록한다.
	HRESULT Add_RenderTarget(
		const wstring_t& strTargetTag,
		uint32_t iWidth, uint32_t iHeight,
		DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	// 등록된 target을 MRT 순서에 추가한다. 현재 중복/8개 상한/크기 호환 검사는 호출자가 보장해야 한다.
	HRESULT Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag);
	// PS SRV를 비운 뒤 현재 RTV0/DSV를 저장하고 MRT를 clear/바인딩한다. 없는 태그여도 선행 SRV 해제는 이미 수행된다.
	HRESULT Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV = nullptr);
	// 저장한 RTV0와 DSV를 복구하고 나머지 RTV 슬롯을 비운다. 모든 기존 MRT를 저장/복원하는 기능은 아니다.
	HRESULT End_MRT();
	// 유효한 DSV로 depth만 clear하고 색상 target 없이 시작한다. PS SRV를 비우고 RTV0/DSV를 같은 복구 슬롯에 보관한다.
	HRESULT Begin_DepthOnly(ComPtr<ID3D11DepthStencilView> pDSV);
	// End_MRT와 동일한 출력 복구 경로를 사용한다.
	HRESULT End_DepthOnly();
	// 이름으로 target을 찾아 Shader 변수에 현재 읽기 view를 설정한다.
	HRESULT Bind_SRV(
		const wstring_t& strTargetTag,
		shared_ptr<class CShader> pShader, const char_t* pConstantName);
	// 현재 view의 강한 참조를 돌려준다. 없는 태그는 빈 ComPtr다.
	ComPtr<ID3D11ShaderResourceView> Get_SRV(const wstring_t& strTargetTag) const;
	// 지정 target에서 호출자의 목적지 텍스처로 복사를 요청한다. 형식/크기 호환성은 호출자 책임이다.
	HRESULT Copy_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D);

#ifdef _DEBUG
public:
	// 지정 target의 화면 미리보기 사각형을 설정한다. 없는 태그는 실패한다.
	HRESULT Ready_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	// Debug에서 이미 존재하는 MRT를 순서대로 표시한다. 현재 구현은 null 목록과 개별 Render 실패를 검사하지 않는다.
	HRESULT Render_MRT(
		const wstring_t& strMRTTag,
		shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer);

#endif

private:
	// 관리자가 장치/context의 COM 참조를 보유한다. target과 pass의 접근 순서는 상위 렌더 호출자가 관리한다.
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

	// Begin이 확보하고 End/명시 해제가 반환하는 pass 한 번의 복구 참조다. swapchain의 영구 소유자가 아니다.
	ComPtr<ID3D11RenderTargetView>	m_pBackBufferRTV = { nullptr };
	ComPtr<ID3D11DepthStencilView>  m_pOriginalDSV = { nullptr };

private:
	// 태그 map과 MRT list가 같은 target의 shared_ptr를 보관한다. target 교체 대신 내부 자원을 교환하는 이유다.
	map<const wstring_t, shared_ptr<class CRenderTarget>>			m_RenderTargets;
	map<const wstring_t, list<shared_ptr<class CRenderTarget>>>		m_MRTs;
private:
	// 조회 성공 시 호출 동안 객체를 유지할 공유 참조를 반환한다.
	shared_ptr<class CRenderTarget> Find_RenderTarget(const wstring_t& strTargetTag) const;
	// 내부 list를 차용한다. 관리자 수명과 단일 렌더 스레드 접근 안에서 사용한다.
	list<shared_ptr<class CRenderTarget>>* Find_MRT(const wstring_t& strMRTTag);

public:
	// 빈 관리자만 만든다. 실제 GPU target 생성은 Add_RenderTarget에서 수행한다.
	static unique_ptr<CTarget_Manager> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
