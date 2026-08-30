#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>

struct ImVec2;
struct ImDrawCmd;
struct ImDrawList;

NS_BEGIN(Client)

class CHUDLayoutTool final
{
public:
	enum class SLOT_TYPE : int32_t
	{
		CUSTOM = 0,
		EVADE,
		POTION,
		HEALTHBAR,
		MANABAR,
		CLASS_EMBLEM,
		IDENTITY_GAUGE,
		SKILL,
		ITEM,
		/* Extracted Scaleform identity-HUD clips (LanceMaster's stance switch, Warlord's defense/
		protect body+effect, gauge fill, Z/X slot art) play back through CHUDRuntimeView's own
		keyframeAnimationPath, not the tint/frame-strip animation this enum otherwise describes.
		Load() validates every slot's "type" against this enum's range, so a slot using this kind
		without a matching entry here fails that check and takes the whole document down with it --
		which silently emptied the Combat HUD tab's class list down to just "Default". */
		KEYFRAME_ANIMATION,
	};

	/* Monochrome silhouette icons (the top-left bar set) are authored once and colored per state,
	   so each layer carries its own multiply tint instead of needing one image per color. */
	struct TEXTURE_LAYER
	{
		string	strPath;
		string	strHoverPath;
		float	vTint[4] = { 1.f, 1.f, 1.f, 1.f };

		/* Glow/particle art (orbs, gauges, emblems) is authored for additive blending: alpha is low
		   across most of the sprite, and adding it onto the background reads as a bright glow. Drawn
		   with the tool's normal alpha blend instead, those same low-alpha pixels just look dim. */
		bool_t	bAdditive = false;

		/* Directional art (an L-shaped bracket, a wing) needs its mirror image for the opposite side;
		   this samples the same source flipped instead of needing a second exported file. */
		bool_t	bFlipX = false;
	};

	struct HUD_SLOT
	{
		string				strName;
		/* Empty means the slot is shared by every class (health/mana bars, skills, items).
		   Otherwise the slot belongs to that one class and is hidden while editing any other. */
		string				strOwnerClass;
		SLOT_TYPE			eType = SLOT_TYPE::CUSTOM;
		float				fX = 0.f, fY = 0.f;
		float				fSizeX = 40.f, fSizeY = 40.f;
		float				fRotation = 0.f;
		vector<TEXTURE_LAYER>	TextureLayers;

		/* Charge stages: a slot's base art appears once the previewed stage reaches iBaseFromStage,
		   and its lit art (shine + animation) once it reaches iShineFromStage. Defaults reproduce the
		   old on/off behaviour: base always visible, lit from stage 1. */
		int32_t				iBaseFromStage = 0;
		int32_t				iShineFromStage = 1;
		string				strShineTexture;
		bool_t				bShineAdditive = false;
		vector<string>		AnimationFrames;
		float				fAnimationFPS = 10.f;
		float				fAnimationScale = 1.1f;
		float				fAnimationOffsetX = 0.f;
		float				fAnimationOffsetY = 0.f;
		/* Round-tripped so Save() does not silently drop it (same reasoning as
		strKeyframeAnimationPath below). Also drives the canvas preview: a looping slot (the
		default -- torch/fire-style flicker effects this preview's ping-pong blend was built for)
		still cross-fades through every frame, but a one-shot burst (loop=false, e.g. a
		success-flash flipbook that is never meant to actually loop) freezes on a single middle
		frame instead, since animating it that way while positioning things is misleading. */
		bool_t				bAnimationLoop = true;
		/* Additive blend for the whole AnimationFrames flipbook -- for a Scaleform source clip
		placed with a real PlaceObject blendMode (an additive glow/particle flipbook), same meaning
		as TEXTURE_LAYER::bAdditive but scoped to the animation since such a slot's TextureLayers
		stays empty. Round-tripped so Save() does not silently drop it. */
		bool_t				bAnimationAdditive = false;

		/* KEYFRAME_ANIMATION slots (extracted Scaleform identity clips) carry no TextureLayers of
		their own -- CHUDRuntimeView draws them entirely from this document instead. The Tool only
		round-trips the reference so repositioning one here does not silently drop it on Save(). */
		string				strKeyframeAnimationPath;
		float				fKeyframeAnimationScale = 1.f;
	};

public:
	CHUDLayoutTool(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	void Render();
	bool_t Select_Class(const string& classId);
	void Render_RuntimePreview(const string& classId);

private:
	void Render_ClassBar();
	void Render_Palette();
	void Render_ClassTexturePalette();
	void Refresh_ClassTexturePalette();
	void Render_Canvas();
	void Render_Inspector();

	void Add_Slot(const string& strName);
	void Add_Slot_From_Palette(SLOT_TYPE eType, float fDropCenterX, float fDropCenterY);
	string Generate_Unique_Name(const string& strBase, bool_t bRepeatable) const;
	void Delete_Slot(int32_t iIndex);
	void Duplicate_Slot(int32_t iIndex);
	void Move_Slot_Forward(int32_t iIndex);
	void Move_Slot_Backward(int32_t iIndex);

	bool_t Is_Slot_Visible(const HUD_SLOT& Slot) const;
	bool_t Is_Slot_Selected(int32_t iIndex) const;
	void Toggle_Slot_Selection(int32_t iIndex);
	void Select_Slot_Exclusive(int32_t iIndex);

	void Add_Class(const string& strName);
	void Delete_Class(int32_t iIndex);

	bool_t Save(const filesystem::path& path);
	bool_t Load(const filesystem::path& path);
	void Reset_Default();

	filesystem::path Current_Save_Path() const;
	void Switch_Document(int32_t iDocument);

	const string& Current_Class() const;

	ID3D11ShaderResourceView* Get_Or_Load_Texture(const string& strPath);

	/* Wraps a single AddImageQuad in an additive blend state via ImGui draw callbacks, then resets
	   render state back to ImGui's own default so the rest of the frame is unaffected. Flipping is
	   just a UV swap, so it rides along on the same call instead of needing its own wrapper. */
	void Draw_Image_Quad(struct ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
		const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX = false);
	static void Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd);

private:
	ComPtr<ID3D11Device>	m_pDevice;
	ComPtr<ID3D11DeviceContext>	m_pContext;
	ComPtr<ID3D11BlendState>	m_pAdditiveBlendState;
	map<string, ComPtr<ID3D11ShaderResourceView>>	m_TextureCache;

private:
	int32_t				m_iActiveDocument = 0;

	vector<HUD_SLOT>	m_Slots;
	int32_t				m_iSelectedSlot = -1;
	vector<int32_t>		m_SelectedSlots;

	/* Hover art is swapped in for preview: either forced for every slot, or for whichever slot the
	   mouse was over last frame (the interaction pass runs after the visual pass, hence the one-frame carry). */
	bool				m_bPreviewHover = false;
	int32_t				m_iHoveredSlot = -1;

	/* Editor-only visibility aid: redraws base layers with extra additive passes so dark art keeps
	   visible edges to align against. Purely a canvas preview toggle; never written to the cfg. */
	bool				m_bBoostDarkArt = false;

	/* Which charge stage the canvas is previewing; one control drives every slot at once. */
	int32_t				m_iPreviewStage = 0;

	/* "Boss UI" document only: stamps sample data into CCombatHUDViewModel so the boss bar renders
	here without a live Valtan encounter. Debug-only; never written to the cfg. */
	bool				m_bPreviewBossData = false;

	/* "Esther UI" document only: stamps a sample gauge ratio so the fill/label render here without
	a live Server gauge. Debug-only; never written to the cfg. */
	bool				m_bPreviewEstherData = false;

	bool_t				m_bMarqueeActive = false;
	float				m_fMarqueeStartX = 0.f;
	float				m_fMarqueeStartY = 0.f;

	vector<string>		m_ClassNames;
	int32_t				m_iSelectedClass = 0;

	float				m_fCanvasScale = 0.7f;

	char				m_szNewSlotName[64] = "New_Slot";
	char				m_szNewClassName[64] = "New_Class";
	char				m_szNewLayerPathBuffer[260] = {};
	char				m_szNewAnimFramePathBuffer[260] = {};

	vector<string>		m_TextureAssetPaths;
	int32_t				m_iLastScannedClass = -1;
	string				m_strDataStatus;
	string m_strCompletePlayStatus =
		"Complete Play uses the workspace's selected saved Server pattern.";
	/* False whenever the active document's on-disk file failed to parse/validate. Switch_Document's
	auto-save must not fire while this is false, or a bad load (bug or a hand-edited file) would
	silently overwrite the real file with whatever placeholder state is on screen. */
	bool_t				m_bActiveDocumentLoaded = false;

private:
	static constexpr float ms_fRefWidth = 1280.f;
	static constexpr float ms_fRefHeight = 720.f;

	/* The canvas viewport stays a fixed size and scrolls, so zooming in doesn't grow the tool window. */
	static constexpr float ms_fViewportWidth = 912.f;
	static constexpr float ms_fViewportHeight = 520.f;
	static constexpr float ms_fMinZoom = 0.25f;
	static constexpr float ms_fMaxZoom = 8.f;
	static constexpr int32_t ms_iMaxStage = 2;
};

NS_END
