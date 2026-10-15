#pragma once

// 장치/context의 참조와 Prototype/Clone 기반 컴포넌트 수명을 상속한다.
#include "Component.h"

/* 내가 지정해둔 텍스쳐여러장을 한번에 로드하여 보관한다. */
/* 쉐이더에 텍스쳐 리소스를 던져준다. */

NS_BEGIN(Engine)

// UI 등의 텍스처 묶음을 소유하고 CShader의 리소스 변수에 연결한다.
// Clone은 같은 GPU 텍스처의 COM 참조를 공유하며 파일을 다시 읽지 않는다.
class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CTexture();

public:
	// Create가 번호를 넣을 경로 서식과 개수를 전달한다. 현재 구현은 개별 이미지 로드 실패를 반환값으로 전파하지 않는다.
	virtual HRESULT Initialize_Prototype(const tchar_t* pTextureFilePath, uint32_t iNumTextures);
	// Clone의 공통 초기화 진입점이다. 현재 pArg와 인스턴스별 추가 상태는 사용하지 않는다.
	virtual HRESULT Initialize(void* pArg) override;
public:
	// 렌더링 호출자가 지정한 한 장을 Effect 변수에 설정한다. 인덱스 범위를 벗어나면 실패한다.
	HRESULT Bind_ShaderResource(class shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iTextureIndex);
	// 보유한 전체 SRV를 배열 변수에 설정한다. 유효한 shader와 비어 있지 않은 텍스처 묶음이 전제다.
	HRESULT Bind_ShaderResources(class shared_ptr<class CShader> pShader, const char_t* pConstantName);

private:
	// 각 원소가 SRV의 강한 참조를 소유한다. 개수는 Initialize_Prototype에서 입력받은 값이다.
	vector<ComPtr<ID3D11ShaderResourceView>>	m_Textures;
	uint32_t									m_iNumTextures = {};

private:
	// 배열 바인딩 직전에 m_Textures에서 채우는 차용 포인터다. 별도 Release를 수행하지 않는다.
	vector<ID3D11ShaderResourceView*>			m_SRVs;

public:
	// Prototype 등록용 소유 객체를 만든다. 초기화가 실패하면 임시 객체의 COM 참조도 함께 정리된다.
	static unique_ptr<CTexture> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pTextureFilePath, uint32_t iNumTextures);	
	// 컴포넌트 복사 후 Initialize를 호출한다. 호출자가 반환된 shared_ptr로 인스턴스 수명을 소유한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END

