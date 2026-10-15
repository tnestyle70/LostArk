#pragma once

// Client 이름 공간과 기존 공통 선언을 사용한다.
#include "Client_Defines.h"
// DX11 장치·SRV의 ComPtr와 bool_t·f32_t를 사용한다.
#include "Engine_Defines.h"

// 문자열 asset 경로를 키로 SRV와 픽셀 크기를 각각 찾는다.
#include <map>
#include <string>

NS_BEGIN(Client)

// Resources 상대 경로별 UI 이미지와 원본 픽셀 크기를 캐시한다. ComPtr가 장치·SRV 참조를 보유한다.
class CUITextureCache final
{
public:
	// CUILayoutRuntime 등이 생성·소유하며 슬롯 사이에서 같은 이미지 참조를 재사용한다.
	CUITextureCache(ComPtr<ID3D11Device> pDevice);
	// 멤버 map과 ComPtr 소멸이 보유 COM 참조를 해제한다.
	~CUITextureCache();

public:
	// 캐시가 소유한 SRV를 차용한다. 빈 경로·로드 실패는 nullptr이며 실패도 캐시해 다시 읽지 않는다.
	ID3D11ShaderResourceView* Get_Or_Load(const string& strPath);
	// 필요하면 먼저 로드하고 Texture2D의 원본 픽셀 크기를 반환한다. 실패하면 출력값을 보존한다.
	bool_t Get_Texture_Size(const string& strPath, f32_t& outWidth, f32_t& outHeight);
	// 캐시에 넣지 않고 DDS·WIC 로더의 새 참조를 반환한다. 경로·로딩 실패는 nullptr다.
	static ComPtr<ID3D11ShaderResourceView> Load_Texture(ID3D11Device* pDevice, const string& strPath);
	// 사전 로드한 참조를 등록한다. 기존 정상 참조는 보존하고 실패 항목만 교체할 수 있다.
	void Adopt(const string& strPath, ComPtr<ID3D11ShaderResourceView> pSRV);

private:
	struct TEXTURE_SIZE { f32_t fWidth = 0.f, fHeight = 0.f; };

	// 장치와 성공한 SRV의 수명을 유지한다. nullptr 항목은 이미 실패한 경로다.
	ComPtr<ID3D11Device> m_pDevice;
	map<string, ComPtr<ID3D11ShaderResourceView>> m_TextureCache;
	// SRV의 Texture2D에서 읽은 원본 크기 값이다. GPU 객체를 추가 소유하지 않는다.
	map<string, TEXTURE_SIZE> m_TextureSizeCache;
};

NS_END
