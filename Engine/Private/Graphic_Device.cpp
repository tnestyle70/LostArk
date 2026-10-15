// 그래픽 장치와 기본 출력 자원을 소유하는 클래스의 구현이다.
#include "..\public\Graphic_Device.h"
// Present 호출을 CPU 측정 구간으로 기록한다.
#include "Profiler.h"
// 현재 엔진이 소유한 Profiler를 Present 측정 동안 차용한다.
#include "GameInstance.h"
// GPU 선호도 순으로 하드웨어 adapter를 조회하는 IDXGIFactory6를 사용한다.
#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

namespace
{
    // 하드웨어 생성 요청에서 고성능 adapter부터 시도한다.
    // 선호 adapter 순회에서는 후보 device/context 생성이 성공한 경우에만 출력 ComPtr를 교체한다.
    // 마지막 기본 하드웨어 생성은 출력 인자를 API에 직접 전달한다.
    HRESULT CreatePreferredHardwareDevice(UINT flags, ComPtr<ID3D11Device>& device,
        D3D_FEATURE_LEVEL& featureLevel, ComPtr<ID3D11DeviceContext>& context)
    {
        ComPtr<IDXGIFactory1> factory;
        ComPtr<IDXGIFactory6> preferredFactory;
        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()))) &&
            SUCCEEDED(factory.As(&preferredFactory)))
        {
            for (UINT index = 0u; ; ++index)
            {
                ComPtr<IDXGIAdapter1> adapter;
                if (FAILED(preferredFactory->EnumAdapterByGpuPreference(index,
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.GetAddressOf()))))
                    break;
                DXGI_ADAPTER_DESC1 description{};
                if (FAILED(adapter->GetDesc1(&description)) ||
                    (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
                    continue;
                ComPtr<ID3D11Device> stagedDevice;
                ComPtr<ID3D11DeviceContext> stagedContext;
                D3D_FEATURE_LEVEL stagedLevel{};
                const HRESULT result = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN,
                    nullptr, flags, nullptr, 0u, D3D11_SDK_VERSION,
                    stagedDevice.GetAddressOf(), &stagedLevel, stagedContext.GetAddressOf());
                if (FAILED(result))
                    continue;
                device = std::move(stagedDevice);
                context = std::move(stagedContext);
                featureLevel = stagedLevel;
                return S_OK;
            }
        }
        // 선호 adapter를 열지 못하거나 구형 DXGI이면 기본 하드웨어 생성을 시도한다.
        // WARP는 호출자가 명시한 경우에만 사용하며 자동으로 전환하지 않는다.
        return D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            nullptr, 0u, D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel,
            context.GetAddressOf());
    }
}

CGraphic_Device::CGraphic_Device()

{	
	
}

CGraphic_Device::~CGraphic_Device()
{


}

// GameInstance의 초기화가 창·물리 픽셀 크기·driver 종류를 전달한다.
// 장치 생성 → swap chain → RTV/DSV → 기본 출력 바인딩을 마친 뒤 출력 참조를 채운다.
HRESULT CGraphic_Device::Initialize(HWND hWnd, WINMODE eWinMode, D3D_DRIVER_TYPE eDriverType,
	int32_t iWinSizeX, int32_t iWinSizeY, ComPtr<ID3D11Device>& pOutDevice,
	ComPtr<ID3D11DeviceContext>& pOutContext)
{
	int32_t		iFlag = 0;

	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

#ifdef _DEBUG
	iFlag = D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL			FeatureLV;

	/* dx9 : 장치 초기화를 하기위한 설정을 쭈욱 하고나서 최종적으로 장치객체를 생성한다. */
	/* dx11 : 우선적으로 장치 객체를 생성하고 장치객체를 통해서 기타 초기화작업 및 설정을 해나간다. */

	/* 그래픽 장치를 초기화한다. */
	if (eDriverType != D3D_DRIVER_TYPE_HARDWARE &&
		eDriverType != D3D_DRIVER_TYPE_WARP)
		return E_INVALIDARG;

	// 일반 하드웨어는 선호 adapter를 사용한다. 명시 WARP 요청은 별도 장치 생성 경로로 처리한다.
	HRESULT hDeviceResult = eDriverType == D3D_DRIVER_TYPE_HARDWARE ?
		CreatePreferredHardwareDevice(iFlag, m_pDevice, FeatureLV, m_pDeviceContext) :
		D3D11CreateDevice(nullptr, eDriverType, 0, iFlag, nullptr, 0,
			D3D11_SDK_VERSION, &m_pDevice, &FeatureLV, &m_pDeviceContext);
#ifdef _DEBUG
	// Debug WARP 생성에 실패하면 디버그 계층만 제외해 재시도한다. 하드웨어 요청을 WARP로 바꾸지 않는다.
	if (FAILED(hDeviceResult) && eDriverType == D3D_DRIVER_TYPE_WARP &&
		0 != (iFlag & D3D11_CREATE_DEVICE_DEBUG))
	{
		iFlag &= ~D3D11_CREATE_DEVICE_DEBUG;
		hDeviceResult = D3D11CreateDevice(nullptr, eDriverType, 0, iFlag, nullptr, 0,
			D3D11_SDK_VERSION, &m_pDevice, &FeatureLV, &m_pDeviceContext);
	}
#endif
	if (FAILED(hDeviceResult))
		return hDeviceResult;




	/* SwapChain : 더블버퍼링. 전면과 후면버퍼를 번갈아가며 화면에 보여준다.(Present) */


	/* 스왑체인객체를 생성하였고 생성한 스왑체인 객체가 백버퍼를 내장한다. 백버퍼를 생성하기 위한 ID3D11Texture2D 만든거야. */
	/* 스왑체인 객체를 만들면서 백버퍼에 해당하는 ID3D11Texture2D객체를 만들어 스왑체인 객체가 내장한다. */
	if (FAILED(Ready_SwapChain(hWnd, eWinMode, iWinSizeX, iWinSizeY)))
		return E_FAIL;

	/* 스왑체인이 들고 있는 텍스쳐 2D를 가져와서 이를 바탕으로 백버퍼 렌더타겟 뷰를 만든다.*/
	if (FAILED(Ready_BackBufferRenderTargetView()))
		return E_FAIL;

	if (FAILED(Ready_DepthStencilView(iWinSizeX, iWinSizeY)))
		return E_FAIL;

	/* 장치에 바인드해놓을 렌더 타겟들과 뎁스스텐실뷰를 세팅한다. */
	/* 장치는 동시에 최대 4->8개의 렌더타겟을 들고 있을 수 있다. */
	if (FAILED(Bind_MainRenderTarget()))
		return E_FAIL;

	// 모든 준비가 성공한 뒤 호출자와 장치/context의 COM 수명을 공유한다.
	pOutDevice = m_pDevice;
	pOutContext = m_pDeviceContext;


	return S_OK;
}

// Render_Begin에서 기본 출력 대상을 재설정한 뒤 지정 RGBA로 색상을 지운다. 입력 포인터는 호출 동안 유효해야 한다.
HRESULT CGraphic_Device::Clear_BackBuffer_View(const float4_t* pClearColor)
{
	if (nullptr == m_pDeviceContext)
		return E_FAIL;

	if (FAILED(Bind_MainRenderTarget()))
		return E_FAIL;

	/* DX9기준 : Clear함수는 백버퍼, 깊이스텐실버퍼를 한꺼번에 지운다.  */
	// m_pGraphic_Device->Clear(어떤 영역만큼 지울까, 어떤 것들을 지울까? , 뭘로 지울가. );	

	/* 백버퍼를 초기화한다.  */
	m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV.Get(), reinterpret_cast<const f32_t*>(pClearColor));

 	return S_OK;
}

// 초기화·리사이즈·프레임 시작이 공유하는 기본 출력 바인딩이다. 세 필수 자원이 없으면 상태 설정 전에 실패한다.
HRESULT CGraphic_Device::Bind_MainRenderTarget()
{
	if (nullptr == m_pDeviceContext ||
		nullptr == m_pBackBufferRTV ||
		nullptr == m_pDepthStencilView)
		return E_FAIL;

	ID3D11RenderTargetView* pRTVs[] = {
		m_pBackBufferRTV.Get(),
	};

	m_pDeviceContext->OMSetRenderTargets(
		1,
		pRTVs,
		m_pDepthStencilView.Get());

	D3D11_VIEWPORT ViewPortDesc{};
	ViewPortDesc.TopLeftX = 0.f;
	ViewPortDesc.TopLeftY = 0.f;
	ViewPortDesc.Width = static_cast<f32_t>(m_iWinSizeX);
	ViewPortDesc.Height = static_cast<f32_t>(m_iWinSizeY);
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pDeviceContext->RSSetViewports(1, &ViewPortDesc);

	return S_OK;
}

// Render_Begin이 색상 초기화 다음 호출한다. 준비된 DSV를 사용하며 context 부재는 실패한다.
HRESULT CGraphic_Device::Clear_DepthStencil_View()
{
	if (nullptr == m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	return S_OK;
}

// Render_End가 한 프레임의 화면 제출 결과를 받는다. 측정 구간에는 DXGI Present의 CPU 대기 시간도 포함된다.
HRESULT CGraphic_Device::Present()
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.Present");
	if (nullptr == m_pSwapChain)
		return E_FAIL;
	
	/* 전면 버퍼와 후면 버퍼를 교체하여 후면 버퍼를 전면으로 보여주는 역할을 한다. */
	/* 후면 버퍼를 직접 화면에 보여줄게. */	
	return m_pSwapChain->Present(0, 0);	
}

// GameInstance의 프레임 사이 크기 변경이다. 상위 렌더 타깃 준비가 끝난 뒤 기본 출력 크기를 바꾼다.
// 실패 복원을 시도하지만 복원 API도 실패할 수 있어 모든 실패에서 이전 화면을 보장하지 않는다.
HRESULT CGraphic_Device::Resize_BackBuffer(uint32_t width, uint32_t height)
{
    if (!m_pDevice || !m_pDeviceContext || !m_pSwapChain || width == 0u || height == 0u ||
        width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
        return E_INVALIDARG;
    if (!m_bBackBufferResizePending && m_pBackBufferRTV && m_pDepthStencilView &&
        width == static_cast<uint32_t>(m_iWinSizeX) && height == static_cast<uint32_t>(m_iWinSizeY))
        return S_OK;

    DXGI_SWAP_CHAIN_DESC swapDesc{};
    const HRESULT description = m_pSwapChain->GetDesc(&swapDesc);
    if (FAILED(description))
        return description;
    // 기존 백버퍼 참조를 놓기 전에 독립적으로 만들 수 있는 새 깊이 자원부터 준비한다.
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = desc.SampleDesc.Count = 1u;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11DepthStencilView> depth;
    HRESULT result = m_pDevice->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
    if (SUCCEEDED(result))
        result = m_pDevice->CreateDepthStencilView(texture.Get(), nullptr, depth.GetAddressOf());
    if (FAILED(result))
        return result;

    // 호출자는 Target_Manager가 보관한 출력 참조를 이미 해제했다.
    // 렌더 패스 사이의 호출이므로 context 바인딩을 비운 뒤 백버퍼를 교체한다.
    m_bBackBufferResizePending = true;
    m_pDeviceContext->ClearState();
    m_pBackBufferRTV.Reset();
    result = m_pSwapChain->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, swapDesc.Flags);
    if (FAILED(result))
    {
        const HRESULT restore = Ready_BackBufferRenderTargetView();
        if (SUCCEEDED(restore))
            (void)Bind_MainRenderTarget();
        return FAILED(restore) ? restore : result;
    }

    result = Ready_BackBufferRenderTargetView();
    if (FAILED(result))
    {
        m_pBackBufferRTV.Reset();
        const HRESULT rollback = m_pSwapChain->ResizeBuffers(0u, static_cast<uint32_t>(m_iWinSizeX),
            static_cast<uint32_t>(m_iWinSizeY), DXGI_FORMAT_UNKNOWN, swapDesc.Flags);
        if (FAILED(rollback))
            return rollback;
        const HRESULT restore = Ready_BackBufferRenderTargetView();
        if (FAILED(restore))
            return restore;
        (void)Bind_MainRenderTarget();
        return result;
    }
    // 새 백버퍼 RTV가 준비된 뒤 깊이 자원과 크기를 채택한다. 이전 깊이 참조는 로컬 값의 소멸로 해제된다.
    m_pDepthStencilView.Swap(depth);
    m_iWinSizeX = static_cast<int32_t>(width);
    m_iWinSizeY = static_cast<int32_t>(height);
    m_bBackBufferResizePending = false;
    return Bind_MainRenderTarget();
}

// ClientWindowDisplay가 프레임 사이 모드 변경을 요청한다. 이전 상태를 보관해 실패 시 복원에 사용한다.
HRESULT CGraphic_Device::Set_FullscreenMode(bool fullscreen, uint32_t width, uint32_t height)
{
    if (!m_pSwapChain || (fullscreen && (width == 0u || height == 0u ||
        width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)))
        return E_INVALIDARG;
    BOOL previousFullscreen = FALSE;
    ComPtr<IDXGIOutput> previousOutput;
    DXGI_SWAP_CHAIN_DESC previousDesc{};
    HRESULT result = m_pSwapChain->GetFullscreenState(&previousFullscreen, previousOutput.GetAddressOf());
    if (SUCCEEDED(result))
        result = m_pSwapChain->GetDesc(&previousDesc);
    if (FAILED(result))
        return result;
    if (!fullscreen && !previousFullscreen)
        return S_OK;

    // DXGI의 모드 변경 진행 중 값은 성공 HRESULT지만 아직 전환 완료가 아니므로 E_PENDING으로 해석한다.
    const auto completed = [](HRESULT status) -> HRESULT
    {
        return status == DXGI_STATUS_MODE_CHANGE_IN_PROGRESS ? E_PENDING : status;
    };
    ComPtr<IDXGIOutput> desiredOutput;
    if (fullscreen)
    {
        result = m_pSwapChain->GetContainingOutput(desiredOutput.GetAddressOf());
        if (FAILED(result))
            return result;
        result = completed(m_pSwapChain->SetFullscreenState(TRUE, desiredOutput.Get()));
        if (SUCCEEDED(result))
        {
            DXGI_MODE_DESC mode = previousDesc.BufferDesc;
            mode.Width = width;
            mode.Height = height;
            mode.RefreshRate = {0u, 0u};
            mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            result = completed(m_pSwapChain->ResizeTarget(&mode));
        }
    }
    else
        result = completed(m_pSwapChain->SetFullscreenState(FALSE, nullptr));
    if (SUCCEEDED(result))
    {
        BOOL currentFullscreen = FALSE;
        result = m_pSwapChain->GetFullscreenState(&currentFullscreen, nullptr);
        if (SUCCEEDED(result) && (currentFullscreen != FALSE) != fullscreen)
            result = E_PENDING;
    }
    if (FAILED(result))
    {
        m_bBackBufferResizePending = true;
        const HRESULT rollback = completed(m_pSwapChain->SetFullscreenState(previousFullscreen, previousOutput.Get()));
        if (FAILED(rollback))
            return rollback;
        DXGI_MODE_DESC mode = previousDesc.BufferDesc;
        const HRESULT restore = completed(m_pSwapChain->ResizeTarget(&mode));
        if (FAILED(restore))
            return restore;
        BOOL restoredFullscreen = FALSE;
        const HRESULT verify = m_pSwapChain->GetFullscreenState(&restoredFullscreen, nullptr);
        if (FAILED(verify))
            return verify;
        return (restoredFullscreen != FALSE) == (previousFullscreen != FALSE) ? result : E_PENDING;
    }
    // 모드가 바뀌면 픽셀 크기가 같아도 다음 Resize_BackBuffer에서 출력 버퍼를 다시 준비한다.
    m_bBackBufferResizePending = true;
    return S_OK;
}

// GameInstance가 다른 렌더 자원을 정리한 뒤 장치 수명을 종료한다.
// 바인딩 해제와 Flush 후 자체 COM 참조를 놓으며, 다른 소유자가 가진 참조까지 강제로 해제하지 않는다.
void CGraphic_Device::Shutdown()
{
	if (nullptr != m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
	}


	if (m_pSwapChain)
	    (void)m_pSwapChain->SetFullscreenState(FALSE, nullptr);
	m_pSwapChain.Reset();
	m_pBackBufferRTV.Reset();
	m_pDepthStencilView.Reset();
	m_pDeviceContext.Reset();



#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug* d3dDebug = nullptr;
	HRESULT hr = nullptr == m_pDevice ? E_FAIL :
		m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hr))
	{
		OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
		OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker \r ");
		OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");

		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

		OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
		OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker END \r ");
		OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
	}
	if (d3dDebug != nullptr)            d3dDebug->Release();
#endif


	m_pDevice.Reset();
}


// 현재 장치의 adapter/factory 계통에서 swap chain을 만든다.
// 이전 API 학습 메모는 보존하며 실제 출력 형식·샘플 수·표시 모드는 아래 descriptor를 따른다.
HRESULT CGraphic_Device::Ready_SwapChain(HWND hWnd, WINMODE isWindowed, int32_t iWinCX, int32_t iWinCY)
{
	ComPtr<IDXGIDevice>			pDevice = nullptr;
	m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)pDevice.GetAddressOf());

	ComPtr < IDXGIAdapter>			pAdapter = nullptr;
	pDevice->GetParent(__uuidof(IDXGIAdapter), (void**)pAdapter.GetAddressOf());

	ComPtr < IDXGIFactory>			pFactory = nullptr;
	pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)pFactory.GetAddressOf());

	/* 스왑체인을 생성한다. = 텍스쳐를 생성하는 행위 + 스왑하는 형태  */
	DXGI_SWAP_CHAIN_DESC		SwapChain;
	ZeroMemory(&SwapChain, sizeof(DXGI_SWAP_CHAIN_DESC));
			
	/* 백버퍼 == 텍스쳐 */
	/*텍스처(백버퍼 == ID3D11Texture2D)를 생성하는 행위*/
	SwapChain.BufferDesc.Width = iWinCX;	/* 가로 픽셀 수 */
	SwapChain.BufferDesc.Height = iWinCY;	/* 세로 픽셀 수 */

	/* float4(1.f, 1.f, 1.f, 1.f) */
	/* float4(1.f, 0.f, 0.f, 1.f) */
	
	SwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; /*D3DFMT_A8R8G8B8*/ /* 만든 픽셀하나의 데이터 정보 : 32BIT픽셀생성하되 부호가 없는 정규화된 수를 저장할께 */
	SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	/* 스케치북에 사과를 그릴꺼야. */
	/* RENDER_TARGET : 그림을 당하는 대상. 스케치북 */
	SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChain.BufferCount = 1;

	/*스왑하는 형태 : 모니터 주사율에 따라 조절해도 됨. */
	SwapChain.BufferDesc.RefreshRate.Numerator = 60;
	SwapChain.BufferDesc.RefreshRate.Denominator = 1;

	/* 멀티샘플링 : 안티얼라이징 (계단현상방지) */
	/* 나중에 후처리 렌더링 : 멀티샘플링 지원(x) */
	SwapChain.SampleDesc.Quality = 0;
	SwapChain.SampleDesc.Count = 1;	

	SwapChain.OutputWindow = hWnd;	
	SwapChain.Windowed = static_cast<BOOL>(isWindowed);
	SwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	/* 백버퍼라는 텍스처(ID3D11Texture2D)를 생성했다. */	
	if (FAILED(pFactory->CreateSwapChain(m_pDevice.Get(), &SwapChain, &m_pSwapChain)))
		return E_FAIL;
	// 화면 모드는 앱 설정이 소유한다. DXGI의 암묵적인 Alt+Enter 모드 전환을 막는다.
	if (FAILED(pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)))
		return E_FAIL;





	return S_OK;
}


// 현재 swap chain의 백버퍼로 RTV를 만든다. 리사이즈의 새 버퍼 준비와 실패 복원도 같은 경로를 사용한다.
HRESULT CGraphic_Device::Ready_BackBufferRenderTargetView()
{
	if (nullptr == m_pDevice)
		return E_FAIL;

	/* 내가 앞으로 사용 하기위한 용도의 텍스쳐를 생성하기위한 베이스 데이터를 가지고 있는 객체이다. */
	/* 내가 앞으로 사용 하기위한 용도의 텍스쳐 : ID3D11RenderTargetView, ID3D11ShaderResoureView, ID3D11DepthStencilView */
	ComPtr<ID3D11Texture2D>		pBackBufferTexture = nullptr;

	/* 스왑체인이 들고있던 텍스처를 가져와봐. */
	if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)pBackBufferTexture.GetAddressOf())))
		return E_FAIL;

	/* 실제 렌더타겟용도로 사용할 수 있는 텍스쳐 타입(ID3D11RenderTargetView)의 객체를 생성한다. */
	if (FAILED(m_pDevice->CreateRenderTargetView(pBackBufferTexture.Get(), nullptr, m_pBackBufferRTV.GetAddressOf())))
		return E_FAIL;	


	return S_OK;
}

// 초기화 때 출력 크기에 맞는 깊이/스텐실 자원을 만든다. texture는 로컬이고 DSV가 GPU 자원 참조를 유지한다.
HRESULT CGraphic_Device::Ready_DepthStencilView(int32_t iWinCX, int32_t iWinCY)
{
	if (nullptr == m_pDevice)
		return E_FAIL;

	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };
	
	D3D11_TEXTURE2D_DESC	TextureDesc{};	

	/* 깊이 버퍼의 픽셀은 백버퍼의 픽셀과 갯수가 동일해야만 깊이 테스트가 가능해진다. */
	/* 픽셀의 수가 다르면 아에 렌더링을 못함. */
	TextureDesc.Width = iWinCX;
	TextureDesc.Height = iWinCY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	/* 동적? 정적?  */
	TextureDesc.Usage = D3D11_USAGE_DEFAULT /* 정적 */;
	/* 추후에 어떤 용도로 바인딩 될 수 있는 View타입의 텍스쳐를 만들기위한 Texture2D입니까? */
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL
		/*| D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE*/;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
		return E_FAIL;


	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture.Get(), nullptr, m_pDepthStencilView.GetAddressOf())))
		return E_FAIL;	



	return S_OK;
}

// 소유 객체를 먼저 만들고 초기화한다. 실패하면 지역 unique_ptr가 부분 준비된 COM 멤버까지 정리한다.
unique_ptr<CGraphic_Device> CGraphic_Device::Create(HWND hWnd, WINMODE eWinMode, D3D_DRIVER_TYPE eDriverType,
	int32_t iWinSizeX, int32_t iWinSizeY, ComPtr<ID3D11Device>& pOutDevice,
	ComPtr<ID3D11DeviceContext>& pOutContext)
{
	auto		pInstance = unique_ptr<CGraphic_Device>(new CGraphic_Device());

	if (FAILED(pInstance->Initialize(hWnd, eWinMode, eDriverType, iWinSizeX, iWinSizeY,
		pOutDevice, pOutContext)))
		return nullptr;

	return pInstance;
}
//
//void CGraphic_Device::Free()
//{
//	Safe_Release(m_pSwapChain);
//	Safe_Release(m_pDepthStencilView);
//	Safe_Release(m_pBackBufferRTV);
//	Safe_Release(m_pDeviceContext);
//
//

//
//
//	Safe_Release(m_pDevice);
//}
