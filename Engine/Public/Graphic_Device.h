#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

/* DX9 */
/* 1. 내 장치의 성능을 조사하는기능 */
/* 2. 지원해주는 범위내에서 내가 셋팅하고싶은 데이터를 전달해서 초기화할 준비를 할 수 있도록 */
/* 3. 디바이스 객체를 생성해준다.(Idirect3DDevice9(장치객체 + 백버퍼+ 깊이버퍼) 객체를 만들었다.) */
/* 3_1. Idirect3DDevice9객체는 상당히 많은 기능을 가진다*/

/* 1. ID3D11Device, ID3D11DeviceContext라는 객체를 우선 생성한다. */
/* 2. IDXGISwapChain를 생성한다.(백버퍼(ID3D11Texture2D)도 같이 생성한거야.) */
/* 3. 백버퍼뷰를 생성한다. */
/* 4. 깊이버퍼 텍스쳐를 생성하고 깊이버퍼 뷰를 생성한다. */

class CGraphic_Device final
{		
private:
	CGraphic_Device();
public:
	~CGraphic_Device();

public:
	/* 그래픽 디바이스의 초기화. */
	/* 장치객체를 생성한다. */
	HRESULT Initialize(HWND hWnd, WINMODE eWinMode, D3D_DRIVER_TYPE eDriverType,
		int32_t iWinSizeX, int32_t iWinSizeY, ComPtr<ID3D11Device>& pOutDevice,
		ComPtr<ID3D11DeviceContext>& pOutContext);
	
	/* 백버퍼를 지운다. */
	HRESULT Clear_BackBuffer_View(const float4_t* pClearColor);

	/* 깊이버퍼 + 스텐실버퍼를 지운다. */
	HRESULT Clear_DepthStencil_View();

	/* 후면 버퍼를 전면버퍼로 교체한다.(백버퍼를 화면에 직접 보여준다.) */
	HRESULT Present();

	void Shutdown();

private:	
	// IDirect3DDevice9* == LPDIRECT3DDEVICE9 == ID3D11Device + ID3D11DeviceContext 	

	/* 스레드를 생성한다 : 내 코드를 읽는 흐름을 하나 더 만들어준다. */
	/* 내 코드를 읽는 프로세스가 두개구나. (X) */
	/* 하나의 프로세스로 읽는 흐름을 다수 생성하는 작업(O) */
	/* 메인 스레드가 출력시에 메인 스레드는 논다. 이 때 놀고 있는 메인스레드로 다른 역활을 좀 더 수행하고싶다.(추가 스레드를 생성하여 처리하는것이다.)*/


	/* 리소스 로드 및 출력. */
	/* 연산. */


	/* 메모리 할당. (정점버퍼, 인덱스버퍼, 텍스쳐로드, 쉐이더객체를 생성한다. ) 컴객체의 생성과 관련된 역할 */
	/* 추가적으로 생성된 모든 스레드에서 사용하는데 전혀 문제가 없다. */
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
		
	/* 기능실행.(바인딩작업, 정점버퍼를 SetVertexBuffers(), SetIndexBuffer(), Apply() */
	/* 그린다. DrawIndexed() */
	/* 컨텍스트 객체를 생성해낸 스레드 외에 스레드에서는 사용해서는 안된다. */
	/* 고정기능렌더링파이프라인 : 월드, 뷰, 투영행렬을 바인딩 + 텍스쳐 정보를 바인딩. */

	/* 생성된 스레드로 그리면 안돼?(X) */
	ComPtr<ID3D11DeviceContext>		m_pDeviceContext = { nullptr };

	/* 후면버퍼와 전면버퍼를 교체해가면서 화면에 보여주는 역할 */
	ComPtr<IDXGISwapChain>				m_pSwapChain = { nullptr };

	
	

	/* IDirect3DTexture9* == LPDIRECT3DTEXTURE9 */
	/* ID3D11Texture2D : 텍스쳐를 표현하는 사전객체 타입이다. 실제 이 타입으로는 아무 역할도 못함. 
	why? 용도에 맞는 실질적으로 사용하기 위한 텍스쳐객체를 만들어내기위해.  */	
	/* ID3D11Texture2D를 통해 픽셀의 락언락을 통해 색을 강제로 바꾸거나 파일로 출력하거나 등등의 일은 가능 */
	/* ID3D11Texture2D를 통해 픽셀의 색을 샘플링해서 화면에 그리는 작업(x), 렌더타겟용으로 사용(x), 깊이 버퍼용으로 사용(x)  */

	/* dx11에서는 실제 사용하기위한 텍스쳐타입뒤에 View를 붙여놨다. */
	/* ID3D11ShaderResourceView : 셰이더에 전달될 수 있는 텍스처 타입. */	
	/* ID3D11RenderTargetView : 렌더타겟용으로 사용될 수 있는 텍스처 타입. */
	/* ID3D11DepthStencilView : 깊이스텐실 버퍼로서 사용될 수 있는 타입.  */
	ComPtr<ID3D11RenderTargetView>		m_pBackBufferRTV = { nullptr };
	ComPtr<ID3D11DepthStencilView>		m_pDepthStencilView = { nullptr };

	int32_t								m_iWinSizeX = {};
	int32_t								m_iWinSizeY = {};

private:
	/* 스왑체인에게 필수적으로 필요한 데이터는 백버퍼가 필요하여 백버퍼를 생성하기위한 정보를 던져준다. */
	/* 스왑체인객체를 만들었다 == 백버퍼(텍스쳐)가 생성된다. */
	HRESULT Ready_SwapChain(HWND hWnd, WINMODE isWindowed, int32_t iWinCX, int32_t iWinCY);
	HRESULT Ready_BackBufferRenderTargetView();	
	HRESULT Ready_DepthStencilView(int32_t iWinCX, int32_t iWinCY);
	HRESULT Bind_MainRenderTarget();


public:
	static unique_ptr<CGraphic_Device> Create(HWND hWnd, WINMODE eWinMode, D3D_DRIVER_TYPE eDriverType,
		int32_t iWinSizeX, int32_t iWinSizeY, ComPtr<ID3D11Device>& pOutDevice,
		ComPtr<ID3D11DeviceContext>& pOutContext);
	
};

NS_END