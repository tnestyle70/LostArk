#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Retail's durability indicator (durability.gfx DurabilityFrame), the armour silhouette that
turns red part by part as gear wears out. Drawn from Data/UI/Durability/DurabilityUI.json as
real CUI_Sprite GameObjects, parked under the minimap on the right edge.

The source contract is DurabilityFrame.as `set setDurabilityPart`: each part is normal
(overlay hidden), damaged or destroyed, and the silhouette itself has two variants depending on
whether bracers are enabled. No Server message carries item durability yet, so every part reads
NORMAL and only the silhouette draws -- which is exactly what retail shows on undamaged gear.
Set_PartState is the single entry the later vertical slice fills in. */
class CDurabilityHudView final
{
public:
	enum class PART : uint8_t
	{
		WEAPON, HELMET, TOP, GLOVES, BOTTOMS, SHOULDER, LIFE_TOOL, BRACER, END
	};
	enum class PART_STATE : uint8_t { NORMAL, DAMAGED, DESTROYED };

	CDurabilityHudView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CDurabilityHudView();

public:
	/* Shows the silhouette and every part whose state is not NORMAL. */
	void Update();
	/* Forces every owned sprite invisible -- these live under LEVEL::STATIC and keep their last
	state across a level change unless told otherwise. */
	void Hide();

	/* The one input. Nothing calls it with a non-NORMAL value yet. */
	void Set_PartState(PART ePart, PART_STATE eState);
	/* Retail swaps the silhouette and the glove overlay together off the account's
	BracerEquipEnabled feature flag. Defaults to true, the frame the authored document ships. */
	void Set_BracerEnabled(bool_t bEnabled);

private:
	/* Applies one part's current state: its own art and authored rect, or hidden when NORMAL. */
	void Apply_Part(PART ePart);

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	PART_STATE m_PartStates[ETOUI(PART::END)] = {};
	/* Zero until the first successful read of the unmodified authored rectangle. */
	f32_t m_PartAuthoredScales[ETOUI(PART::END)] = {};
	bool_t m_bBracerEnabled = true;
};

NS_END
