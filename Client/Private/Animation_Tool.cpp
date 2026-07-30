#include "imgui.h"

#include "Animation_Tool.h"

#include "GameInstance.h"
#include "Model.h"

#include <cstdio>
#include <cstring>

namespace
{
	/* The character the tool drives lives on TEST_LEVEL2 as a container/part pair.
	This resolution is the only place that knows the target; once the shared asset
	catalog lands it replaces the body of Resolve_Model and nothing else. */
	constexpr const tchar_t* LAYER_TAG = TEXT("Layer_Player");
	/* CCharacter numbers its part tags so std::map updates them in the right
	order (see the 07-30 RESULT doc), hence the "00" rather than plain "Part_Body". */
	constexpr const tchar_t* PART_TAG = TEXT("Part_00_Body");
	constexpr const tchar_t* COMPONENT_TAG = TEXT("Com_Model");

	constexpr const char_t* EVENT_FILE_MAGIC = "LOSTARK_ANIM_EVENTS";
	/* v1 wrote positional columns ("clip" HIT 14 22). v2 writes key=value pairs so
	rows of different kinds can carry different fields. Both are still read. */
	constexpr int32_t EVENT_FILE_VERSION = 2;

	void Skip_Space(const char_t*& p)
	{
		while (' ' == *p || '\t' == *p)
			++p;
	}

	bool_t Read_Quoted(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		if ('\"' != *p)
			return false;
		++p;
		out.clear();
		while ('\0' != *p && '\"' != *p)
			out.push_back(*p++);
		if ('\"' != *p)
			return false;
		++p;
		return true;
	}

	bool_t Read_Token(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		out.clear();
		while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			out.push_back(*p++);
		return !out.empty();
	}

	/* key=value, where value may be quoted so paths with spaces survive. */
	bool_t Read_Pair(const char_t*& p, std::string& key, std::string& value)
	{
		Skip_Space(p);
		key.clear();
		while ('\0' != *p && '=' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			key.push_back(*p++);
		if ('=' != *p || key.empty())
			return false;
		++p;

		value.clear();
		if ('\"' == *p)
		{
			++p;
			while ('\0' != *p && '\"' != *p)
				value.push_back(*p++);
			if ('\"' == *p)
				++p;
		}
		else
		{
			while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
				value.push_back(*p++);
		}
		return true;
	}

	bool_t Contains_NoCase(const char_t* pText, const char_t* pNeedle)
	{
		if (nullptr == pText)
			return false;
		if ('\0' == pNeedle[0])
			return true;

		for (const char_t* p = pText; *p != '\0'; ++p)
		{
			const char_t* a = p;
			const char_t* b = pNeedle;
			while (*a != '\0' && *b != '\0' &&
				tolower(static_cast<unsigned char>(*a)) == tolower(static_cast<unsigned char>(*b)))
			{
				++a;
				++b;
			}
			if ('\0' == *b)
				return true;
		}
		return false;
	}
}

shared_ptr<Engine::CModel> Client::CAnimation_Tool::Resolve_Model() const
{
	if (ETOUI(LEVEL::TEST_LEVEL2) != CGameInstance::Get().Get_CurrentLevelID())
		return nullptr;

	shared_ptr<CComponent> pComponent = CGameInstance::Get().Get_Component(
		ETOUI(LEVEL::TEST_LEVEL2), LAYER_TAG, PART_TAG, COMPONENT_TAG, 0);

	return dynamic_pointer_cast<Engine::CModel>(pComponent);
}

int32_t Client::CAnimation_Tool::Get_TickFrame(const ANIM_EVENT& evt, int32_t iTickIndex)
{
	if (evt.iHitCount <= 1 || iTickIndex <= 0)
		return evt.iStartFrame;
	if (iTickIndex >= evt.iHitCount - 1)
		return evt.iEndFrame;

	const int32_t iSpan = evt.iEndFrame - evt.iStartFrame;
	return evt.iStartFrame + (iSpan * iTickIndex) / (evt.iHitCount - 1);
}

/* Which hit the given frame belongs to, or -1 when outside the window. */
int32_t Client::CAnimation_Tool::Get_ActiveTick(const ANIM_EVENT& evt, int32_t iFrame)
{
	if (iFrame < evt.iStartFrame || iFrame > evt.iEndFrame)
		return -1;
	if (evt.iHitCount <= 1)
		return 0;

	int32_t iActive = 0;
	for (int32_t i = 0; i < evt.iHitCount; ++i)
	{
		if (iFrame >= Get_TickFrame(evt, i))
			iActive = i;
	}
	return iActive;
}

std::string Client::CAnimation_Tool::Get_EventFilePath() const
{
	return "../Bin/DataFiles/Anim/" + m_AssetName + ".animevents";
}

void Client::CAnimation_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(460.f, 720.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk Animation Tool"))
	{
		ImGui::End();
		return;
	}

	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		ImGui::TextUnformatted("No animated character resolved.");
		ImGui::Separator();
		ImGui::TextUnformatted("Enter TEST_LEVEL2 with F3 from the logo level.");
		ImGui::End();
		return;
	}

	/* Load once the target actually exists, so a missing file is not reported
	before the level is even open. */
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		Load_Events();
	}

	ImGui::Text("Asset: %s   Animations: %u", m_AssetName.c_str(), pModel->Get_NumAnimations());

	Render_Playback(pModel);
	Render_HitEvents(pModel);

	ImGui::SeparatorText("Clips");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##filter", "filter by name", m_Filter, sizeof(m_Filter));

	Render_AnimationList(pModel);

	ImGui::End();
}

void Client::CAnimation_Tool::Render_Playback(const shared_ptr<Engine::CModel>& pModel)
{
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const bool_t bPaused = pModel->Is_AnimPaused();

	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	ImGui::Text("%s [%u] %s", bPaused ? "Paused:" : "Playing:",
		iCurrentIndex, nullptr != pCurrentName ? pCurrentName : "(none)");

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack =
		pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;

	/* Track positions are in ticks. Blender exports one key per frame, so 1 tick == 1 frame. */
	if (bHasTrack)
	{
		f32_t fScrub = fPosition;

		/* SliderFloat feeds only the slider value to its format string, so the
		duration has to be baked in and the value slot escaped as %%. */
		char_t szScrubFormat[64]{};
		snprintf(szScrubFormat, sizeof(szScrubFormat), "frame %%.1f / %.0f", fDuration);

		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::SliderFloat("##scrub", &fScrub, 0.f, fDuration, szScrubFormat))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, fScrub);
		}
	}
	else
	{
		ImGui::TextUnformatted("Track: n/a");
	}

	if (ImGui::Button(bPaused ? "Play" : "Pause"))
		pModel->Set_AnimPaused(!bPaused);

	ImGui::SameLine();
	ImGui::BeginDisabled(!bHasTrack);
	if (ImGui::Button("< Frame"))
	{
		pModel->Set_AnimPaused(true);
		pModel->Set_AnimTrackPosition(iCurrentIndex, fPosition - 1.f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Frame >"))
	{
		pModel->Set_AnimPaused(true);
		pModel->Set_AnimTrackPosition(iCurrentIndex, fPosition + 1.f);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Restart"))
		pModel->Set_AnimTrackPosition(iCurrentIndex, 0.f);

	ImGui::SameLine();
	if (ImGui::Checkbox("Loop", &m_bLoop))
		pModel->Set_Animation(iCurrentIndex, m_bLoop);
}

void Client::CAnimation_Tool::Render_HitEvents(const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Hit windows");

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
	{
		ImGui::TextUnformatted("No clip selected.");
		return;
	}

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack =
		pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;
	const int32_t iFrame = static_cast<int32_t>(fPosition);

	if (ImGui::Button("Add window at current frame"))
	{
		ANIM_EVENT evt{};
		evt.clipName = pCurrentName;
		evt.eKind = EVENT_KIND::HIT;
		evt.iStartFrame = iFrame;
		evt.iEndFrame = iFrame;
		m_Events.push_back(evt);
		m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
		m_bDirty = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Save"))
		Save_Events();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		m_iSelectedEvent = -1;
		Load_Events();
	}

	if (m_bDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}

	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());

	if (!ImGui::BeginChild("##hitlist", ImVec2(0.f, 150.f), ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}

	int32_t iShown = 0;
	int32_t iRemoveIndex = -1;

	for (int32_t i = 0; i < static_cast<int32_t>(m_Events.size()); ++i)
	{
		ANIM_EVENT& evt = m_Events[i];
		if (evt.clipName != pCurrentName)
			continue;

		++iShown;
		ImGui::PushID(i);

		const int32_t iActiveTick = bHasTrack ? Get_ActiveTick(evt, iFrame) : -1;
		const bool_t bActive = iActiveTick >= 0;
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

		char_t szLabel[96]{};
		if (evt.iHitCount > 1)
			snprintf(szLabel, sizeof(szLabel), "HIT %d - %d  x%d%s",
				evt.iStartFrame, evt.iEndFrame, evt.iHitCount,
				bActive ? "" : "");
		else
			snprintf(szLabel, sizeof(szLabel), "HIT %d - %d",
				evt.iStartFrame, evt.iEndFrame);

		if (bActive)
		{
			char_t szActive[32]{};
			snprintf(szActive, sizeof(szActive), "  <== tick %d/%d",
				iActiveTick + 1, evt.iHitCount);
			strcat_s(szLabel, szActive);
		}

		if (ImGui::Selectable(szLabel, i == m_iSelectedEvent))
		{
			m_iSelectedEvent = i;
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, static_cast<f32_t>(evt.iStartFrame));
		}

		if (bActive)
			ImGui::PopStyleColor();

		if (i == m_iSelectedEvent)
		{
			const int32_t iMaxFrame = bHasTrack ? static_cast<int32_t>(fDuration) : 0;
			ImGui::SetNextItemWidth(200.f);
			if (ImGui::DragIntRange2("start / end", &evt.iStartFrame, &evt.iEndFrame,
				0.2f, 0, iMaxFrame, "%d", "%d"))
				m_bDirty = true;

			ImGui::SameLine();
			if (ImGui::Button("Remove"))
				iRemoveIndex = i;

			ImGui::SetNextItemWidth(120.f);
			if (ImGui::DragInt("hits", &evt.iHitCount, 0.1f, 1, 64))
			{
				if (evt.iHitCount < 1)
					evt.iHitCount = 1;
				m_bDirty = true;
			}

			if (evt.iHitCount > 1)
			{
				ImGui::SameLine();
				const int32_t iSpan = evt.iEndFrame - evt.iStartFrame;
				ImGui::Text("every %.1f frames", static_cast<f32_t>(iSpan) /
					static_cast<f32_t>(evt.iHitCount - 1));

				/* Spell the tick frames out so they can be checked against the pose. */
				std::string ticks;
				for (int32_t t = 0; t < evt.iHitCount && t < 16; ++t)
				{
					if (!ticks.empty())
						ticks += ", ";
					ticks += std::to_string(Get_TickFrame(evt, t));
				}
				if (evt.iHitCount > 16)
					ticks += ", ...";
				ImGui::TextWrapped("ticks: %s", ticks.c_str());
			}
		}

		ImGui::PopID();
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No hit window on this clip.");

	ImGui::EndChild();

	if (iRemoveIndex >= 0)
	{
		m_Events.erase(m_Events.begin() + iRemoveIndex);
		m_iSelectedEvent = -1;
		m_bDirty = true;
	}
}

void Client::CAnimation_Tool::Render_AnimationList(const shared_ptr<Engine::CModel>& pModel)
{
	if (!ImGui::BeginChild("##cliplist", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}

	const uint32_t iNumAnimations = pModel->Get_NumAnimations();
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	uint32_t iShown = 0;

	for (uint32_t i = 0; i < iNumAnimations; ++i)
	{
		const char_t* pName = pModel->Get_AnimationName(i);
		if (nullptr == pName || !Contains_NoCase(pName, m_Filter))
			continue;

		++iShown;

		/* Mark clips that already carry events so authored work is easy to find. */
		int32_t iEventCount = 0;
		for (const ANIM_EVENT& evt : m_Events)
		{
			if (evt.clipName == pName)
				++iEventCount;
		}

		char_t szLabel[MAX_PATH + 32]{};
		if (iEventCount > 0)
			snprintf(szLabel, sizeof(szLabel), "%3u  %s  [%d]", i, pName, iEventCount);
		else
			snprintf(szLabel, sizeof(szLabel), "%3u  %s", i, pName);

		if (ImGui::Selectable(szLabel, i == iCurrentIndex))
		{
			/* Always show a newly picked clip from its start, but keep the pause state. */
			pModel->Set_Animation(i, m_bLoop);
			pModel->Set_AnimTrackPosition(i, 0.f);
			m_iSelectedEvent = -1;
		}
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No clip matches the filter.");

	ImGui::EndChild();
}

bool_t Client::CAnimation_Tool::Save_Events()
{
	const std::string path = Get_EventFilePath();

	/* DataFiles/Anim is not created by a clean clone, so make it on demand. */
	CreateDirectoryA("../Bin/DataFiles/Anim", nullptr);

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "w") || nullptr == pFile)
	{
		m_Status = "Save failed: " + path;
		return false;
	}

	fprintf(pFile, "%s %d \"%s\" %d\n", EVENT_FILE_MAGIC, EVENT_FILE_VERSION,
		m_AssetName.c_str(), static_cast<int32_t>(m_Events.size()));

	for (const ANIM_EVENT& evt : m_Events)
	{
		fprintf(pFile, "\"%s\" HIT start=%d end=%d count=%d\n",
			evt.clipName.c_str(), evt.iStartFrame, evt.iEndFrame, evt.iHitCount);
	}

	fclose(pFile);

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(m_Events.size()) + " event(s) to " + path;
	return true;
}

bool_t Client::CAnimation_Tool::Load_Events()
{
	const std::string path = Get_EventFilePath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
	{
		m_Status = "No event file yet: " + path;
		return false;
	}

	char_t szLine[1024]{};
	std::string magic, versionToken, owner, countToken;

	const char_t* p = nullptr;
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		m_Status = "Empty event file: " + path;
		return false;
	}

	p = szLine;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != EVENT_FILE_MAGIC)
	{
		fclose(pFile);
		m_Status = "Unrecognized event file header: " + path;
		return false;
	}

	const int32_t iVersion = atoi(versionToken.c_str());
	if (iVersion < 1 || iVersion > EVENT_FILE_VERSION)
	{
		fclose(pFile);
		m_Status = "Unsupported event file version " + versionToken + ": " + path;
		return false;
	}

	std::vector<ANIM_EVENT> loaded;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string clip, kind;
		if (!Read_Quoted(p, clip) || !Read_Token(p, kind))
			continue;

		/* Unknown kinds are skipped rather than treated as fatal, so a file written
		by a newer tool with sound/effect rows still loads its hit windows. */
		if ("HIT" != kind)
			continue;

		ANIM_EVENT evt{};
		evt.clipName = clip;
		evt.eKind = EVENT_KIND::HIT;

		if (1 == iVersion)
		{
			/* v1 stored the two frames as bare positional columns. */
			std::string startToken, endToken;
			if (!Read_Token(p, startToken) || !Read_Token(p, endToken))
				continue;
			evt.iStartFrame = atoi(startToken.c_str());
			evt.iEndFrame = atoi(endToken.c_str());
		}
		else
		{
			std::string key, value;
			int32_t iInterval = 0;
			while (Read_Pair(p, key, value))
			{
				/* Unknown keys are ignored so new fields do not break old builds. */
				if ("start" == key)
					evt.iStartFrame = atoi(value.c_str());
				else if ("end" == key)
					evt.iEndFrame = atoi(value.c_str());
				else if ("count" == key)
					evt.iHitCount = atoi(value.c_str());
				else if ("interval" == key)
					iInterval = atoi(value.c_str());
			}

			/* interval is accepted as a hand-authored alternative to count and is
			normalised to count on the next save. */
			if (iInterval > 0 && 1 == evt.iHitCount)
				evt.iHitCount = (evt.iEndFrame - evt.iStartFrame) / iInterval + 1;
			if (evt.iHitCount < 1)
				evt.iHitCount = 1;
		}

		loaded.push_back(evt);
	}

	fclose(pFile);

	m_Events = std::move(loaded);
	m_bDirty = false;
	m_Status = "Loaded " + std::to_string(m_Events.size()) + " event(s) from " + path;
	return true;
}
