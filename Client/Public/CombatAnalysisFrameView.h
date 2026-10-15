#pragma once

// Client 이름 공간과 STATIC Level을 사용한다.
#include "Client_Defines.h"
// 장치 ComPtr와 엔진의 기본 수치·문자열 타입을 사용한다.
#include "Engine_Defines.h"
// 제목줄 드래그 상태를 값 멤버로 갖는다.
#include "UIWindowDrag.h"

// 레이아웃을 unique_ptr로 단독 소유한다.
#include <memory>
// 통계 ID와 제목·단위 문구를 보관한다.
#include <string>
// 함께 움직일 슬롯과 통계 항목·단위를 순서대로 보관한다.
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
struct HUD_PLAYER_STATE;

// MainApp 소유 전투 분석 HUD다. ViewModel의 피해·DPS·무력화·카운터 집계를 표시한다.
// 제목줄 드래그와 닫기를 처리하며 닫힌 전투의 시작 tick이 바뀌면 다시 보여 준다.
class CCombatAnalysisFrameView final
{
public:
	// MainApp 초기화에서 생성해 STATIC Level의 레이아웃과 문구 데이터를 읽고 숨긴다.
	CCombatAnalysisFrameView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// 레이아웃과 보유 COM 참조를 정리한다. Layer에 남은 sprite 참조는 별도다.
	~CCombatAnalysisFrameView();

public:
	// MainApp의 HUD 표시 조건을 통과한 프레임에서 Server tick과 창 입력을 갱신한다.
	void Update(f32_t fTimeDelta, const HUD_PLAYER_STATE& Player);
	// MainApp이 EndFrame 뒤 HUD 글자 순서에서 호출한다. 집계값은 ViewModel에서 다시 읽는다.
	void Render_Text();
	// HUD 표시 조건이 닫힐 때 모든 슬롯을 숨긴다. 닫기 버튼의 전투별 숨김 상태는 보존한다.
	void Hide();

private:
	struct STAT_TILE
	{
		string strId;
		wstring strTitle;
		bool_t bCount = false;
	};

private:
	// Data/UI의 통계 제목·ID·단위를 읽는다. 파일/parse 실패 시 통계 항목은 비어 있다.
	void Load_Data();
	// 닫기 버튼 hover 이미지를 바꾸고 클릭한 전투의 시작 tick을 저장한다.
	void Update_Chrome();
	// 1.23만 / 4.56억처럼 단위를 붙이며 만 미만은 정수로 표시한다. 전투 전 문구는 호출부가 고른다.
	wstring Format_Amount(uint64_t iValue) const;
	// 이동한 배경 슬롯을 기준으로 글자 위치를 계산한다.
	bool_t Get_FrameOrigin(f32_t& fX, f32_t& fY) const;
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	void Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);

private:
	// 장치·Context는 COM 참조, 레이아웃은 단독 소유이며 통계 데이터는 값 사본이다.
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	CUIWindowDrag					m_Drag;
	vector<STAT_TILE>				m_Stats;
	wstring							m_strValueNull;
	vector<wstring>					m_Units;

	// CA_Bg 폭으로 계산한 원본 픽셀→레이아웃 기준 좌표 배율이다.
	f32_t m_fRetailScale = 2.f / 3.f;
	bool_t m_bHidden = false;
	// 닫기 버튼을 누른 전투의 시작 tick이다. 다른 시작 tick이면 숨김을 해제한다.
	uint32_t m_iHiddenFightStartTick = 0u;
	// 텍스트 갱신에 쓸 최신 Server tick과 현재 HUD 표시 상태다.
	uint32_t m_iServerTick = 0u;
	bool_t m_bShown = false;
};

NS_END
