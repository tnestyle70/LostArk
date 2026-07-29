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

	struct HUD_SLOT
	{
		string				strName;
		SLOT_TYPE			eType = SLOT_TYPE::CUSTOM;
		float				fX = 0.f, fY = 0.f;
		float				fSizeX = 40.f, fSizeY = 40.f;
		bool				bPerClass = false;
		vector<string>		TextureLayers;
		map<string, string>	ClassTextures;
	};

public:
	CHUDLayoutTool(ComPtr<ID3D11Device> pDevice);

public:
	void Render();

private:
	void Render_ClassBar();
	void Render_Palette();
	void Render_Canvas();
	void Render_Inspector();

	void Add_Slot(const string& strName);
	void Add_Slot_From_Palette(SLOT_TYPE eType, float fDropCenterX, float fDropCenterY);
	string Generate_Unique_Name(const string& strBase, bool_t bRepeatable) const;
	void Delete_Slot(int32_t iIndex);
	void Duplicate_Slot(int32_t iIndex);

	void Add_Class(const string& strName);
	void Delete_Class(int32_t iIndex);

	void Save(const string& strPath);
	void Load(const string& strPath);
	void Reset_Default();

	const string& Current_Class() const;

	ID3D11ShaderResourceView* Get_Or_Load_Texture(const string& strPath);

private:
	ComPtr<ID3D11Device>	m_pDevice;
	map<string, ComPtr<ID3D11ShaderResourceView>>	m_TextureCache;

private:
	vector<HUD_SLOT>	m_Slots;
	int32_t				m_iSelectedSlot = -1;

	vector<string>		m_ClassNames;
	int32_t				m_iSelectedClass = 0;

	float				m_fCanvasScale = 0.7f;

	char				m_szNewSlotName[64] = "New_Slot";
	char				m_szNewClassName[64] = "New_Class";
	char				m_szTexturePathBuffer[260] = {};
	char				m_szNewLayerPathBuffer[260] = {};

private:
	static constexpr float ms_fRefWidth = 1280.f;
	static constexpr float ms_fRefHeight = 720.f;
	static constexpr const char* ms_szDefaultSavePath = "../Bin/Resources/LostArk/UI/HUD_Layout.cfg";
};

NS_END
