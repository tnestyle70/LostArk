// SRV 묶음을 소유하는 CTexture의 구현과 상속 타입을 가져온다.
#include "Texture.h"
// DDS 압축 텍스처와 WIC 지원 이미지에서 D3D11 SRV를 생성한다.
// DDS 파일의 압축 형식과 mip을 읽는 로더를 사용한다.
#include "DirectXTK/DDSTextureLoader.h"
// DDS와 TGA 분기를 제외한 경로를 WIC 이미지 로더에 전달한다.
#include "DirectXTK/WICTextureLoader.h"

// 기존 GameInstance/Prototype_Manager include 경로를 통해 CShader의 바인딩 API 선언을 가져온다.
#include "GameInstance.h"

// 기반 컴포넌트가 장치와 context의 COM 참조를 보관한다.
CTexture::CTexture(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

// m_Textures의 ComPtr가 소유 참조를 해제한다. m_SRVs의 차용 포인터에는 별도 해제가 없다.
CTexture::~CTexture()
{
}

// Create가 호출하는 파일 로딩 단계다. 번호별 경로로 GPU 자원을 만들어 텍스처 묶음에 보관한다.
// 현재 hr는 검사하지 않으므로 일부 로드가 실패해도 S_OK를 반환하고 빈 SRV가 남을 수 있다.
HRESULT CTexture::Initialize_Prototype(const tchar_t* pTextureFilePath, uint32_t iNumTextures)
{
	m_iNumTextures = iNumTextures;

	m_Textures.reserve(iNumTextures);

	for (size_t i = 0; i < m_iNumTextures; i++)
	{

		ComPtr<ID3D11ShaderResourceView>		pSRV = { nullptr };

		tchar_t			szTextureFilePath[MAX_PATH] = {};

		// 호출자가 제공한 경로 서식의 번호 위치에 현재 인덱스를 넣고 확장자로 로더를 고른다.
		wsprintf(szTextureFilePath, pTextureFilePath, i);
		
		tchar_t			szEXT[MAX_PATH] = {};

		_wsplitpath_s(szTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

		HRESULT		hr = {};

		/* UI 스프라이트의 원본 바이트를 유지하도록 sRGB 해석을 억제한다.
		   스킬 지면 표시도 같은 로더를 사용하므로 최종 색 공간 처리는 각 렌더 경로가 맡는다. */
		if (false == lstrcmp(szEXT, TEXT(".dds")))
		{
			hr = CreateDDSTextureFromFileEx(
				m_pDevice.Get(), szTextureFilePath, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		else if (false == lstrcmp(szEXT, TEXT(".tga")))
		{
			hr = E_FAIL;
		}
		else
		{
			hr = CreateWICTextureFromFileEx(
				m_pDevice.Get(), szTextureFilePath, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}

		// 현재 구현은 로더의 hr와 무관하게 원소를 추가한다. 실패한 원소를 성공한 텍스처로 간주해서는 안 된다.
		m_Textures.push_back(move(pSRV));
	}

	m_SRVs.resize(m_iNumTextures);

	return S_OK;
}

// Clone 초기화 훅이며 현재 추가로 읽거나 변경하는 상태는 없다.
HRESULT CTexture::Initialize(void* pArg)
{
	return S_OK;
}

// 단일 텍스처 인덱스만 여기서 검사하고 Effect 변수의 유효성과 결과는 CShader에 맡긴다.
HRESULT CTexture::Bind_ShaderResource(
	shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[iTextureIndex]);	
}

// 소유 SRV에서 차용 포인터 배열을 다시 만든 뒤 Effect 배열 변수에 전달한다.
// 배열을 만드는 m_SRVs는 이 인스턴스의 작업 공간이므로 같은 인스턴스의 동시 호출을 동기화하지 않는다.
HRESULT CTexture::Bind_ShaderResources(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	uint32_t		iNumTextures = {};

	for (auto& pTexture : m_Textures)
		m_SRVs[iNumTextures++] = pTexture.Get();

	return pShader->Bind_Textures(pConstantName, &m_SRVs.front(), m_iNumTextures);
}

// MainApp 등의 Prototype 등록이 사용한다. Initialize_Prototype의 실패 반환에 대해서만 nullptr로 정리한다.
unique_ptr<CTexture> CTexture::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const tchar_t* pTextureFilePath, uint32_t iNumTextures)
{
	auto pInstance = unique_ptr<CTexture>(new CTexture(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures)))
	{
		MSG_BOX("Failed to Created : CTexture");
		return nullptr;
	}

	return pInstance;
}

// 기본 복사로 SRV 참조를 늘리고 새 컴포넌트를 반환한다. 이미지 파일을 다시 읽는 단계는 없다.
shared_ptr<CPrototype> CTexture::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CTexture>(new CTexture(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTexture");
		return nullptr;
	}

	return pInstance;
}

