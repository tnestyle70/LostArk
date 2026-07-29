#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

struct MAP_ASSET_ENTRY;

class CMapAssetPreview final
{
public:
	static constexpr const wchar_t* SHADER_PROTOTYPE_TAG =
		L"Prototype_Component_Shader_MapAssetPreview";

public:
	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	void Reset_LevelResources();

	HRESULT Select_Asset(const MAP_ASSET_ENTRY& asset);
	HRESULT Render(uint32_t width, uint32_t height);

	void Orbit(f32_t deltaX, f32_t deltaY);
	void Zoom(f32_t wheelDelta);
	void Reset_Camera();

	bool_t Has_Asset() const { return nullptr != m_pModel; }
	ID3D11ShaderResourceView* Get_TextureView() const
	{
		return m_pColorSRV.Get();
	}

	const std::string& Get_AssetId() const { return m_AssetId; }
	const std::string& Get_Label() const { return m_Label; }
	const std::string& Get_ModelPath() const { return m_ModelPath; }
	const std::string& Get_Status() const { return m_Status; }
	uint32_t Get_MeshCount() const;
	float3_t Get_Dimensions() const;

private:
	HRESULT Ensure_RenderTarget(uint32_t width, uint32_t height);
	HRESULT Render_Model();

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };

	ComPtr<ID3D11Texture2D> m_pColorTexture = { nullptr };
	ComPtr<ID3D11RenderTargetView> m_pColorRTV = { nullptr };
	ComPtr<ID3D11ShaderResourceView> m_pColorSRV = { nullptr };
	ComPtr<ID3D11Texture2D> m_pDepthTexture = { nullptr };
	ComPtr<ID3D11DepthStencilView> m_pDepthDSV = { nullptr };
	uint32_t m_iTargetWidth = {};
	uint32_t m_iTargetHeight = {};

	shared_ptr<CShader> m_pShader = { nullptr };
	shared_ptr<CModel> m_pModel = { nullptr };

	std::string m_AssetId;
	std::string m_Label;
	std::string m_ModelPath;
	std::string m_Status = "Select an asset from the Palette.";

	float3_t m_vBoundsCenter = {};
	float3_t m_vHalfExtent = {};
	f32_t m_fBoundsRadius = 1.f;
	f32_t m_fYaw = {};
	f32_t m_fPitch = {};
	f32_t m_fZoom = 1.f;
	bool_t m_bDirty = true;
};

NS_END
