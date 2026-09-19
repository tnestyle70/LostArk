#include "UITextOcclusion.h"

#include "GameInstance.h"
#include "UILayoutRuntime.h"

Client::CUITextOcclusion& Client::CUITextOcclusion::Get()
{
	static CUITextOcclusion s_Instance;
	return s_Instance;
}

void Client::CUITextOcclusion::Begin_Frame()
{
	m_Occluders.clear();
}

void Client::CUITextOcclusion::Add_Occluder(const int32_t iLayer,
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight)
{
	if (fWidth <= 0.f || fHeight <= 0.f)
		return;
	m_Occluders.push_back(OCCLUDER{ iLayer, fX, fY, fWidth, fHeight });
}

void Client::CUITextOcclusion::Add_SlotOccluder(const int32_t iLayer,
	const CUILayoutRuntime& View, const char* pSlotId)
{
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (nullptr == pSlotId || !View.Get_SlotRect(pSlotId, fX, fY, fW, fH) ||
		View.Get_ResolutionWidth() <= 0.f || View.Get_ResolutionHeight() <= 0.f)
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / View.Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / View.Get_ResolutionHeight();
	Add_Occluder(iLayer, fX * fScaleX, fY * fScaleY, fW * fScaleX, fH * fScaleY);
}

void Client::CUITextOcclusion::Apply(const int32_t iTextLayer) const
{
	CGameInstance& Instance = CGameInstance::Get();
	Instance.Clear_TextClipOutRect();
	for (const OCCLUDER& Occluder : m_Occluders)
	{
		if (Occluder.iLayer > iTextLayer)
			Instance.Add_TextClipOutRect(Occluder.fX, Occluder.fY, Occluder.fWidth, Occluder.fHeight);
	}
}
