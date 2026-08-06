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
	enum class DRAW_TARGET
	{
		/* Always on top of every ImGui window (the combat HUD's own use, independent of any
		specific window). */
		FOREGROUND,
		/* The currently-open ImGui window's draw list -- art lands behind that window's own
		widgets instead of over every window in the game. Call Render() between that window's
		Begin()/End() when using this. */
		CURRENT_WINDOW,
		/* Always behind every ImGui window and the 3D scene alike -- a level's own backdrop
		(the lobby's title background, ...), never covering any window's UI. */
		BACKGROUND,
	};

	/* strDocumentPath: Data-root-relative layout JSON to load ("UI/HUD/HUD_Layout.json" if
	omitted). CHUDLayoutTool authors any lostark.ui-layout document at any such path (Combat
	HUD, Screen UI, Loading Screen, Skill Window, Lobby, ...); a second CHUDRuntimeView
	instance pointed at a different path is how a second document gets a runtime consumer
	without a second copy of this parse/draw logic. */
	CHUDRuntimeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		const wstring& strDocumentPath = L"UI/HUD/HUD_Layout.json",
		DRAW_TARGET eDrawTarget = DRAW_TARGET::FOREGROUND);
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
		/* A non-empty list plays as a looping flipbook instead of drawing Layers -- CHUDLayoutTool
		already authors this same "animation.frames"/"animation.fps" pair for its own canvas
		preview, so a slot built for a login/lobby background frame sequence just needs a runtime
		consumer, not a new document field. */
		vector<string>			AnimationFrames;
		f32_t					fAnimationFPS = 10.f;
	};

private:
	HRESULT Load();
	ID3D11ShaderResourceView* Get_Or_Load_Texture(const string& strPath);
	void Draw_Image_Quad(ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
		const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX);
	static void Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd);

private:
	wstring							m_strDocumentPath;
	DRAW_TARGET						m_eDrawTarget = DRAW_TARGET::FOREGROUND;
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	ComPtr<ID3D11BlendState>		m_pAdditiveBlendState;
	map<string, ComPtr<ID3D11ShaderResourceView>>	m_TextureCache;

	vector<HUD_SLOT>	m_Slots;
	f32_t				m_fResolutionWidth = 1280.f;
	f32_t				m_fResolutionHeight = 720.f;
};

NS_END
