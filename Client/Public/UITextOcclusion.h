#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Stacking order of runtime UI text. Every UI sprite is drawn in RENDERGROUP::UI and every
Draw_Text runs after that pass, so without help any label would sit on top of every window.
Each UI surface registers its screen rect with the layer its sprites draw on; text is drawn
inside a layer scope, and the font clip-out list then holds exactly the registered rects of
higher layers. A window's labels disappear wherever a window above covers that window, and
text drawn outside any scope counts as world text (nameplates, chat bubbles, damage numbers):
it never shows through any UI surface.

Layers are ordered integers so a group can stack its members (WINDOW + index). */
namespace UI_TEXT_LAYER
{
	constexpr int32_t WORLD = 0;
	constexpr int32_t HUD = 100;
	constexpr int32_t WINDOW = 200;
	constexpr int32_t MODAL = 300;
	constexpr int32_t PAGE = 400;
}

class CUITextOcclusion final
{
public:
	static CUITextOcclusion& Get();

	/* Once per frame, before any surface registers (CMainApp, next to the input router). */
	void Begin_Frame();
	/* Screen pixels. */
	void Add_Occluder(int32_t iLayer, f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	/* A slot of a layout document in its reference resolution, converted to screen pixels. */
	void Add_SlotOccluder(int32_t iLayer, const CUILayoutRuntime& View, const char* pSlotId);
	/* Rebuilds the font clip-out list for text of iTextLayer. */
	void Apply(int32_t iTextLayer) const;

private:
	struct OCCLUDER
	{
		int32_t iLayer = 0;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	};
	std::vector<OCCLUDER> m_Occluders;
};

/* Text of one layer: applies that layer on entry, back to world text on exit. */
class CUITextLayerScope final
{
public:
	explicit CUITextLayerScope(int32_t iLayer) { CUITextOcclusion::Get().Apply(iLayer); }
	~CUITextLayerScope() { CUITextOcclusion::Get().Apply(UI_TEXT_LAYER::WORLD); }
	CUITextLayerScope(const CUITextLayerScope&) = delete;
	CUITextLayerScope& operator=(const CUITextLayerScope&) = delete;
};

NS_END
