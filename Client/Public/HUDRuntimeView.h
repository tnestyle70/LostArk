#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <string>
#include <vector>

struct ImDrawList;
struct ImDrawCmd;
struct ImVec2;

NS_BEGIN(Client)

/* Runtime (Release-safe) presentation for the HUD Layout Tool's authored JSON. The tool
(_DEBUG only) edits Data/UI/HUD/HUD_Layout.json; this class is the first actual in-game
consumer of that data -- it loads the document once, then every frame draws whichever slots
match the local player's class and current charge stage, via ImGui's foreground draw list.
This intentionally mirrors CHUDLayoutTool's own per-slot draw logic (same source data, same
visual rules) instead of inventing a second layout format. */
class CHUDRuntimeView final
{
public:
	CHUDRuntimeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CHUDRuntimeView();

public:
	/* strOwnerClass: HUD document class name ("LanceMaster", "Gunslinger", ...). Slots with no
	owner (shared/common chrome) always draw; slots owned by a different class are skipped.
	iStage: 0 = base only, 1 = base + stage-1 layers, 2 = base + stage-1 + stage-2 layers
	(monotonic reveal -- a slot visible from stage N stays visible at every stage after N). */
	void Render(const string& strOwnerClass, int32_t iStage);

private:
	struct TEXTURE_LAYER
	{
		string	strPath;
		float	vTint[4] = { 1.f, 1.f, 1.f, 1.f };
		bool_t	bAdditive = false;
		bool_t	bFlipX = false;
	};

	struct HUD_SLOT
	{
		string					strOwnerClass;
		f32_t					fX = 0.f, fY = 0.f, fSizeX = 0.f, fSizeY = 0.f;
		f32_t					fRotation = 0.f;
		int32_t					iBaseFromStage = 0;
		int32_t					iShineFromStage = 1;
		vector<TEXTURE_LAYER>	Layers;
		string					strShineTexture;
		bool_t					bShineAdditive = false;
	};

private:
	HRESULT Load();
	ID3D11ShaderResourceView* Get_Or_Load_Texture(const string& strPath);
	void Draw_Image_Quad(ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
		const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX);
	static void Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	ComPtr<ID3D11BlendState>		m_pAdditiveBlendState;
	map<string, ComPtr<ID3D11ShaderResourceView>>	m_TextureCache;

	vector<HUD_SLOT>	m_Slots;
	f32_t				m_fResolutionWidth = 1280.f;
	f32_t				m_fResolutionHeight = 720.f;
};

NS_END
