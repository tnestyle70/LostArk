#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

/* Read-only identity for the render-target topology that is active while a
   render object submits its actual draw.  Client code can observe the typed
   contract but cannot select or mutate a render target. */
enum class RENDER_OUTPUT_CONTRACT : uint8_t
{
	NONE,
	SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION,
	END
};

class ENGINE_DLL CRenderOutputContract final
{
public:
	static RENDER_OUTPUT_CONTRACT Get_Active();
	static bool_t Matches_ActiveRenderTargets(
		ID3D11DeviceContext* pContext);

private:
	friend class CRenderer;
	friend class CRenderOutputContractScope;
};

class ENGINE_DLL CRenderOutputContractScope final
{
private:
	friend class CRenderer;
	CRenderOutputContractScope(
		RENDER_OUTPUT_CONTRACT eContract,
		ID3D11DeviceContext* pContext);

public:
	~CRenderOutputContractScope();
	CRenderOutputContractScope(const CRenderOutputContractScope&) = delete;
	CRenderOutputContractScope& operator=(
		const CRenderOutputContractScope&) = delete;

private:
	RENDER_OUTPUT_CONTRACT m_ePrevious = RENDER_OUTPUT_CONTRACT::NONE;
	ID3D11DeviceContext* m_pPreviousContext = nullptr;
	ID3D11RenderTargetView* m_pPreviousSceneColor = nullptr;
	ID3D11RenderTargetView* m_pPreviousDistortion = nullptr;
	ID3D11DepthStencilView* m_pPreviousDepthStencil = nullptr;
};

NS_END
