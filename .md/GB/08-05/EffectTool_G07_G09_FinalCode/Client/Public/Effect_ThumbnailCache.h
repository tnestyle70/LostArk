#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <unordered_map>

NS_BEGIN(Client)

class CEffectThumbnailCache final
{
public:
	struct RESULT final
	{
		ID3D11ShaderResourceView* pTextureView = nullptr;
		const std::string* pError = nullptr;
	};

private:
	struct ENTRY final
	{
		ComPtr<ID3D11ShaderResourceView> pTextureView;
		std::string strError;
		uint64_t iLastUsedFrame = 0u;
	};

public:
	CEffectThumbnailCache(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffectThumbnailCache();

	void Begin_Frame(uint64_t iFrameNumber);
	RESULT Request(const std::string& strAssetId);
	void Invalidate(uint64_t iCatalogRevision);
	void Trim();
	void Clear();

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	std::unordered_map<std::string, ENTRY> m_Entries;
	uint64_t m_iFrameNumber = 0u;
	uint64_t m_iCatalogRevision = 0u;
	uint32_t m_iLoadsThisFrame = 0u;
};

NS_END

