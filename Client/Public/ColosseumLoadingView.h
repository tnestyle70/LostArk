#pragma once

#include "Client_Defines.h"
#include "Character.h"
#include "CharacterPortraitRenderer.h"
#include "ClientReplication.h"
#include "GameInstance.h"
#include "LevelTransitionService.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "UITextOcclusion.h"
#include "WorldPlayerNameplateView.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace Client
{
// One product loading view is used by the asset loader and by the active arena's
// presentation-readiness barrier. Fixed team/arrival order never depends on the viewer.
class CColosseumLoadingView final
{
public:
	CColosseumLoadingView(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context, uint32_t level)
		: m_Device(device), m_Context(context), m_View(std::make_unique<CUILayoutRuntime>(
			device, context, level, TEXT("Layer_ColosseumLoading"), L"UI/Colosseum/MatchLoading_Layout.json"))
	{
		m_View->Set_UISortLayer(UI_TEXT_LAYER::MODAL);
		for (const auto& id : m_View->Get_SlotIds()) m_View->Set_SlotCinematicOverlay(id, true);
		CLevelTransitionService::Try_Get_ColosseumMatch(m_Roster);
		for (size_t i = 0; i < m_Roster.Participants.size(); ++i)
		{
			const auto& participant = m_Roster.Participants[i];
			if (participant.iTeam > 1u) continue;
			CARD card;
			card.iArrival = static_cast<uint8_t>(participant.iTeam + m_Cards[participant.iTeam].size() * 2u);
			card.iClass = participant.eCharacterClass;
			card.bLocal = i == m_Roster.iLocalIndex;
			CWorldPlayerNameplateView::Try_ConvertUtf8(participant.strNickname, card.strName);
			m_Cards[participant.iTeam].push_back(std::move(card));
		}
		Layout();
		Set_Progress(0.f);
	}
	~CColosseumLoadingView() { m_View->Release_Sprites(); }
	void Hide() { m_bVisible = false; m_View->Set_AllSlotsVisible(false); }
	bool Is_Visible() const { return m_bVisible; }
	void Set_Progress(float fraction)
	{
		m_View->Set_SlotFillRatio("MatchLoading_ProgressFill", std::clamp(fraction, 0.f, 1.f));
		float x=0,y=0,w=0,h=0,hx=0,hy=0,hw=0,hh=0;
		if (m_View->Get_SlotRect("MatchLoading_ProgressBackground",x,y,w,h) &&
			m_View->Get_SlotRect("MatchLoading_ProgressHead",hx,hy,hw,hh))
			m_View->Set_SlotPosition("MatchLoading_ProgressHead", x+w*fraction-hw*(293.f/356.f),hy);
	}
	void Set_Waiting(const std::wstring& status) { m_Status = status; }
	// Called only from MainApp's portrait submission phase, after Render_Begin.
	void Submit_Portraits(const CClientReplication& replication)
	{
		if (!m_bVisible) return;
		std::vector<REPLICATED_PLAYER_VIEW> players;
		replication.Collect_PlayerViews(players);
		const auto& state = replication.Get_ColosseumMatchState();
		// Wait for the replicated appearance transaction before replacing a captured portrait.
		if (!state.iMatchId || state.iMatchId != m_Roster.iMatchId ||
			!replication.Are_ColosseumCharactersReady()) return;
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
		if (viewport.x <= 0.f || viewport.y <= 0.f) return;
		for (unsigned team=0;team<2;++team)
		{
			if (m_Cards[team].empty()) continue;
			const auto row=std::find_if(state.Participants.begin(),state.Participants.end(),[&](const auto& p){
				return p.bParticipant && p.iArrivalIndex==m_Cards[team].front().iArrival && p.iTeam==team; });
			if (row==state.Participants.end()) continue;
			const auto player=std::find_if(players.begin(),players.end(),[&](const auto& p){
				return p.iNetEntityId==row->iNetEntityId && p.iPlayerId==row->iPlayerId;});
			if (player==players.end()) continue;
			const auto character=player->pCharacter.lock();
			if (!character) continue;
			if (!m_Portraits[team]) m_Portraits[team]=std::make_unique<CCharacterPortraitRenderer>(m_Device,m_Context);
			float eye=0.f;
			if (!CCharacterPortraitRenderer::Try_Measure_EyeHeight(*character,eye)) continue;
			CCharacterPortraitRenderer::CAMERA camera;
			camera.fFovDegrees=30.f; camera.fEyeHeight=camera.fLookHeight=0.767f*eye;
			camera.fDistance=0.71f*eye/(2.f*std::tan(XMConvertToRadians(30.f)*0.5f));
			const auto width=static_cast<uint32_t>((std::max)(1.f,std::round(373.333f*viewport.x/1280.f)));
			const auto height=static_cast<uint32_t>((std::max)(1.f,std::round(324.f*viewport.y/720.f)));
			if (S_OK==m_Portraits[team]->Render(character,width,height,camera,0u,0u))
				if (auto portrait=m_Portraits[team]->Get_SRV())
				{
					m_View->Set_SlotTextureSRV(Prefix(team)+"Portrait",portrait);
					m_View->Set_SlotVisible(Prefix(team)+"Portrait",true);
				}
		}
	}
	void Render()
	{
		if (!m_bVisible) return;
		CUITextLayerScope scope(UI_TEXT_LAYER::MODAL);
		Draw(L"\xC12C\xBA78\xC804 - \xCF5C\xB85C\xC138\xC6C0",960,22,28,{1,1,1,1});
		Draw(m_Status.empty()?L"\xCC38\xAC00\xC790 \xB85C\xB529 \xC911...":m_Status,960,969,17,{1,1,1,1});
		for (unsigned team=0;team<2;++team)
		{
			const auto& cards=m_Cards[team];
			if (cards.empty()) continue;
			const float panel=team?1478.f:442.f;
			Draw(L"\xC8FC\xBAA9\xD560 \xCE90\xB9AD\xD130",panel,122,16,{.55f,.72f,.95f,1});
			Draw(cards.front().strName,panel,186,19,{1,1,1,1});
			for (size_t i=0;i<cards.size();++i)
			{
				const auto& card=cards[i];
				const float x=CardCenter(team,cards.size(),i);
				Draw(Job(card.iClass),x,737,12,{.72f,.72f,.72f,1});
				Draw(card.strName,x,789,15,card.bLocal?float4_t(1,.84f,.35f,1):float4_t(1,1,1,1));
				if (card.bLocal) Draw(L"\xB098",x,851,14,{1,.84f,.35f,1});
			}
		}
	}
private:
	struct CARD { uint8_t iArrival=0; LostArk::Shared::CHARACTER_CLASS_ID iClass=LostArk::Shared::CHARACTER_CLASS_ID::END; std::wstring strName; bool bLocal=false; };
	static std::string Prefix(unsigned team) { return team?"MatchLoading_TeamB_":"MatchLoading_TeamA_"; }
	static float CardCenter(unsigned team,size_t count,size_t index) { return (team?1488.f:451.f)+(static_cast<float>(index)-0.5f*static_cast<float>(count-1u))*260.f; }
	static const wchar_t* Job(LostArk::Shared::CHARACTER_CLASS_ID id)
	{
		using E=LostArk::Shared::CHARACTER_CLASS_ID;
		switch(id) {case E::LANCE_MASTER:return L"\xCC3D\xC220\xC0AC";case E::GUNSLINGER:return L"\xAC74\xC2AC\xB9C1\xC5B4";case E::SLAYER:return L"\xC2AC\xB808\xC774\xC5B4";case E::ARTIST:return L"\xB3C4\xD654\xAC00";case E::DIMENSIONMASTER:return L"\xCC28\xC6D0\xC220\xC0AC";case E::WARLORD:return L"\xC6CC\xB85C\xB4DC";case E::GUARDIANKNIGHT:return L"\xAC00\xB514\xC5B8\xB098\xC774\xD2B8";default:return L"";}
	}

	void Layout()
	{
		for (unsigned team=0;team<2;++team)
		{
			const auto prefix=Prefix(team); const auto count=m_Cards[team].size();
			// Movie portrait slots contain the actual character, never class illustration art.
			m_View->Set_SlotVisible(prefix+"Portrait",false);
			for (const char* part:{"WedgeGlow","PanelBorder","TitleBackground"}) m_View->Set_SlotVisible(prefix+part,count!=0);
			for (unsigned slot=0;slot<3;++slot)
			{
				const auto base=prefix+"Slot"+std::to_string(slot+1)+"_";
				const float original=(team?1098.f:61.f)+slot*260.f+130.f;
				for (const char* part:{"Shield","Plate","Glow","Gauge","Highlight"})
				{
					const std::string id=base+part;
					bool visible=slot<count && (std::string(part)=="Shield" || std::string(part)=="Plate");
					m_View->Set_SlotVisible(id,visible);
					float x=0,y=0,w=0,h=0;
					if (visible && m_View->Get_SlotRect(id,x,y,w,h)) m_View->Set_SlotPosition(id,x+(CardCenter(team,count,slot)-original)*2.f/3.f,y);
				}
			}
			if (count==0) continue;
			if (m_Cards[team].front().bLocal)
				if (auto portrait=CLevelTransitionService::Get_TransferPortraitSRV())
				{
					m_View->Set_SlotTextureSRV(prefix+"Portrait",portrait);
					m_View->Set_SlotVisible(prefix+"Portrait",true);
				}
			for (size_t i=0;i<count;++i) if (m_Cards[team][i].bLocal)
			{
				const unsigned sourceSlot=team?0u:2u;
				for (const char* part:{"Glow","Highlight"})
				{
					const auto id=prefix+"Slot"+std::to_string(sourceSlot+1u)+"_"+part;
					float x=0,y=0,w=0,h=0;
					if (m_View->Get_SlotRect(id,x,y,w,h))
					{
						m_View->Set_SlotPosition(id,x+(CardCenter(team,count,i)-((team?1098.f:61.f)+sourceSlot*260.f+130.f))*2.f/3.f,y);
						m_View->Set_SlotVisible(id,true);
					}
				}
			}
		}
	}
	static void Draw(const std::wstring& text,float x,float y,float pixels,const float4_t& color)
	{
		if (text.empty()) return;
		const auto viewport=CGameInstance::Get().Get_ViewportSize();
		const float sx=viewport.x/1920.f,sy=viewport.y/1080.f;
		float scale=1.f;
		const auto font=UILabelFont::Resolve(L"Font_YoonGasiIIM",pixels*1.25f*(std::min)(sx,sy),scale);
		CGameInstance::Get().Draw_Text(font,text.c_str(),{x*sx,y*sy},XMLoadFloat4(&color),0.f,{.5f,.5f},scale);
	}
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	std::unique_ptr<CUILayoutRuntime> m_View;
	std::unique_ptr<CCharacterPortraitRenderer> m_Portraits[2];
	LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND m_Roster{};
	std::vector<CARD> m_Cards[2];
	std::wstring m_Status;
	bool m_bVisible=true;
};
}
