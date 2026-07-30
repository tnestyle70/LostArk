#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

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
	};

	/* Monochrome silhouette icons (the top-left bar set) are authored once and colored per state,
	   so each layer carries its own multiply tint instead of needing one image per color. */
	struct TEXTURE_LAYER
	{
		string	strPath;
		string	strHoverPath;
		float	vTint[4] = { 1.f, 1.f, 1.f, 1.f };
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

		bool				bAnimationOn = false;
		string				strShineTexture;
		vector<string>		AnimationFrames;
		float				fAnimationFPS = 10.f;
		float				fAnimationScale = 1.1f;
		float				fAnimationOffsetX = 0.f;
		float				fAnimationOffsetY = 0.f;
	};

public:
	CHUDLayoutTool(ComPtr<ID3D11Device> pDevice);

public:
	void Render();

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

	void Save(const string& strPath);
	void Load(const string& strPath);
	void Reset_Default();

	const char* Current_Save_Path() const;
	void Switch_Document(int32_t iDocument);

	const string& Current_Class() const;

	ID3D11ShaderResourceView* Get_Or_Load_Texture(const string& strPath);

private:
	ComPtr<ID3D11Device>	m_pDevice;
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

private:
	static constexpr float ms_fRefWidth = 1280.f;
	static constexpr float ms_fRefHeight = 720.f;
};

NS_END
