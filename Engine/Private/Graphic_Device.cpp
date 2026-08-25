#include "..\public\Graphic_Device.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <chrono>

CGraphic_Device::CGraphic_Device()

{	
	
}

CGraphic_Device::~CGraphic_Device()
{


}

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

	HRESULT hDeviceResult = D3D11CreateDevice(nullptr, eDriverType, 0, iFlag, nullptr, 0,
		D3D11_SDK_VERSION, &m_pDevice, &FeatureLV, &m_pDeviceContext);
#ifdef _DEBUG
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

	pOutDevice = m_pDevice;
	pOutContext = m_pDeviceContext;


	return S_OK;
}

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

HRESULT CGraphic_Device::Clear_DepthStencil_View()
{
	if (nullptr == m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	return S_OK;
}

HRESULT CGraphic_Device::Present()
{
	if (nullptr == m_pSwapChain)
		return E_FAIL;
	
	/* 전면 버퍼와 후면 버퍼를 교체하여 후면 버퍼를 전면으로 보여주는 역할을 한다. */
	/* 후면 버퍼를 직접 화면에 보여줄게. */
	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (nullptr == pProfiler || !pProfiler->Is_Enabled())
		return m_pSwapChain->Present(0, 0);

	Engine::CProfilerScope scope(pProfiler, "Client.Present");
	const auto begin = std::chrono::steady_clock::now();
	const HRESULT result = m_pSwapChain->Present(0, 0);
	const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::steady_clock::now() - begin).count();
	pProfiler->Set_Counter(
		Engine::EProfilerCounter::FramePresentMicroseconds,
		elapsed > 0 ? static_cast<uint64_t>(elapsed) : 0u);
	return result;
}

void CGraphic_Device::Shutdown()
{
	if (nullptr != m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
	}


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

	/* 백버퍼라는 텍스처(ID3D11Texture2D)를 생성했다. */	
	if (FAILED(pFactory->CreateSwapChain(m_pDevice.Get(), &SwapChain, &m_pSwapChain)))
		return E_FAIL;





	return S_OK;
}


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
