#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CAnimation_Tool final
{
private:
	/* Point events (sound, effect spawn) will reuse this record with
	iEndFrame ignored once the asset catalogs they need exist. */
	enum class EVENT_KIND
	{
		HIT,
	};

	struct ANIM_EVENT
	{
		std::string clipName;
		EVENT_KIND eKind = EVENT_KIND::HIT;
		int32_t iStartFrame = {};
		int32_t iEndFrame = {};
		/* Hits fired across the window, spread evenly. 1 means a single hit. */
		int32_t iHitCount = { 1 };
	};

public:
	void Render();

private:
	shared_ptr<Engine::CModel> Resolve_Model() const;
	void Render_Playback(const shared_ptr<Engine::CModel>& pModel);
	void Render_HitEvents(const shared_ptr<Engine::CModel>& pModel);
	void Render_AnimationList(const shared_ptr<Engine::CModel>& pModel);

	bool_t Save_Events();
	bool_t Load_Events();
	std::string Get_EventFilePath() const;

	static int32_t Get_TickFrame(const ANIM_EVENT& evt, int32_t iTickIndex);
	static int32_t Get_ActiveTick(const ANIM_EVENT& evt, int32_t iFrame);

private:
	char m_Filter[128]{};
	bool_t m_bLoop = true;

	std::vector<ANIM_EVENT> m_Events;
	std::string m_AssetName = "LanceMaster";
	std::string m_Status;
	bool_t m_bDirty = false;
	bool_t m_bLoadAttempted = false;
	int32_t m_iSelectedEvent = -1;
};

NS_END
