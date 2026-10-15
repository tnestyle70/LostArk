// 제품 UI의 사각형 가림 정보를 Engine의 글자 clip-out 목록으로 전달한다.
#include "UITextOcclusion.h"

// viewport를 조회하고 글자 제외 영역을 초기화·등록한다.
#include "GameInstance.h"
// 슬롯 사각형과 문서 기준 해상도를 읽는다.
#include "UILayoutRuntime.h"

// 첫 호출 때 공용 객체가 생성되고 정상 프로세스 종료 때 값 목록도 소멸한다.
Client::CUITextOcclusion& Client::CUITextOcclusion::Get()
{
	static CUITextOcclusion s_Instance;
	return s_Instance;
}

// MainApp의 프레임 시작에서 이전 표면 목록을 버린다. 창 수명에는 관여하지 않는다.
void Client::CUITextOcclusion::Begin_Frame()
{
	m_Occluders.clear();
}

// 표면의 현재 layer와 Client 픽셀 사각형을 복사해 보관한다.
void Client::CUITextOcclusion::Add_Occluder(const int32_t iLayer,
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight)
{
	if (fWidth <= 0.f || fHeight <= 0.f)
		return;
	m_Occluders.push_back(OCCLUDER{ iLayer, fX, fY, fWidth, fHeight });
}

// 없는 슬롯·잘못된 기준 해상도는 등록하지 않는다.
void Client::CUITextOcclusion::Add_SlotOccluder(const int32_t iLayer,
	const CUILayoutRuntime& View, const char* pSlotId)
{
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (nullptr == pSlotId || !View.Get_SlotRect(pSlotId, fX, fY, fW, fH) ||
		View.Get_ResolutionWidth() <= 0.f || View.Get_ResolutionHeight() <= 0.f)
		return;
	// 문서 기준 좌표에 축별 viewport 배율을 곱해 실제 글자 좌표와 맞춘다.
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / View.Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / View.Get_ResolutionHeight();
	Add_Occluder(iLayer, fX * fScaleX, fY * fScaleY, fW * fScaleX, fH * fScaleY);
}

// 텍스트 패스와 layer scope가 호출한다. 같은 layer는 가리지 않고 앞쪽 표면만 등록한다.
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
