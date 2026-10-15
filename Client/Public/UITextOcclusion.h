#pragma once

// Client 이름 공간과 기존 공통 선언을 사용한다.
#include "Client_Defines.h"
// 사각형의 f32_t와 NS_BEGIN 등 엔진 공통 타입·매크로를 사용한다.
#include "Engine_Defines.h"

// 가림 순서는 int32_t, 프레임의 사각형 목록은 vector 값으로 보관한다.
#include <cstdint>
#include <vector>

NS_BEGIN(Client)

// 참조로 슬롯 정보를 읽기 때문에 헤더에는 이름만 선언한다.
class CUILayoutRuntime;

// UI의 앞뒤 순서를 이미지와 글자가 공유한다. 숫자가 큰 표면이 낮은 layer의 글자를 가린다.
// 범위 밖의 글자는 WORLD로 처리해 모든 UI 표면 아래에 둔다.
namespace UI_TEXT_LAYER
{
	constexpr int32_t WORLD = 0;
	constexpr int32_t HUD = 100;
	constexpr int32_t WINDOW = 200;
	constexpr int32_t MODAL = 300;
	constexpr int32_t PAGE = 400;

	// 각 창의 이미지·글자가 함께 사용하는 창 사이의 앞뒤 순서다.
	constexpr int32_t WINDOW_INVENTORY = WINDOW + 0;
	constexpr int32_t WINDOW_CHARACTER_INFO = WINDOW + 1;
	constexpr int32_t WINDOW_AVATAR_BOOK = WINDOW + 2;
	constexpr int32_t WINDOW_VEHICLE = WINDOW + 3;
	constexpr int32_t WINDOW_HONOR_TITLE = WINDOW + 4;
	constexpr int32_t WINDOW_WORLD_MAP = WINDOW + 5;
	constexpr int32_t WINDOW_SYSTEM_OPTION = WINDOW + 6;
	constexpr int32_t WINDOW_ITEM_UPGRADE = WINDOW + 7;
	constexpr int32_t WINDOW_REPAIR = WINDOW + 8;
	constexpr int32_t WINDOW_SHOP = WINDOW + 9;
}

// 창 객체를 소유하지 않고 이번 프레임의 layer와 사각형 값만 모은다.
class CUITextOcclusion final
{
public:
	// 함수 지역 static으로 보관하는 공용 객체를 차용한다.
	static CUITextOcclusion& Get();

	// MainApp::Update에서 창 영역을 다시 등록하기 전에 이전 목록을 비운다.
	void Begin_Frame();
	// Client 영역 픽셀로 가릴 사각형을 등록한다. 폭·높이가 0 이하면 무시한다.
	void Add_Occluder(int32_t iLayer, f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	// 슬롯의 문서 기준 사각형을 현재 viewport 픽셀로 바꿔 등록한다.
	void Add_SlotOccluder(int32_t iLayer, const CUILayoutRuntime& View, const char* pSlotId);
	// 현재 글자보다 앞에 있는 표면으로 Engine의 글자 제외 영역을 다시 만든다.
	void Apply(int32_t iTextLayer) const;

private:
	struct OCCLUDER
	{
		int32_t iLayer = 0;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	};
	// Begin_Frame에서 비우고 열린 표면들이 다시 채우는 값 목록이다.
	std::vector<OCCLUDER> m_Occluders;
};

// 블록 안에는 지정 layer를 적용하고, 소멸 시 WORLD로 바꾼다. 이전 layer를 저장하지 않는다.
class CUITextLayerScope final
{
public:
	explicit CUITextLayerScope(int32_t iLayer) { CUITextOcclusion::Get().Apply(iLayer); }
	~CUITextLayerScope() { CUITextOcclusion::Get().Apply(UI_TEXT_LAYER::WORLD); }
	CUITextLayerScope(const CUITextLayerScope&) = delete;
	CUITextLayerScope& operator=(const CUITextLayerScope&) = delete;
};

NS_END
