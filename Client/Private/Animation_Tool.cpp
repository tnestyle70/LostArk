#include "imgui.h"

#include "Animation_Tool.h"

#include "AnimationTargetService.h"
#include "Character.h"
#include "Model.h"
#include "ProjectDataRoot.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
	constexpr const char_t* EVENT_FILE_MAGIC = "LOSTARK_ANIM_EVENTS";
	/* v1 wrote positional columns ("clip" HIT 14 22). v2 writes key=value pairs so
	rows of different kinds can carry different fields. v3 keeps the pairs but moves
	times from frames to milliseconds and adds the extracted combat fields. All are
	still read; v1/v2 frames are converted with the clip's tick rate on load. */
	constexpr int32_t EVENT_FILE_VERSION = 3;

	/* Used when a clip carries no usable rate, which would otherwise make the
	frame <-> millisecond conversion divide by zero. */
	constexpr f32_t DEFAULT_TICK_RATE = 30.f;

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
	return CAnimationTargetService::Resolve_Model();
}

shared_ptr<Client::CCharacter> Client::CAnimation_Tool::Resolve_Character() const
{
	return CAnimationTargetService::Resolve_Character();
}

void Client::CAnimation_Tool::Sync_AssetName()
{
	const shared_ptr<CCharacter> pCharacter = Resolve_Character();
	const CHARACTER_SPEC* pSpec = nullptr == pCharacter ? nullptr : pCharacter->Get_Spec();
	const char_t* pAssetName = nullptr == pSpec ? nullptr : pSpec->pAssetName;
	if (nullptr == pAssetName || m_AssetName == pAssetName)
		return;

	/* A different class means every loaded file belongs to the wrong asset. Drop
	them rather than mixing two classes' clips in one list. */
	m_AssetName = pAssetName;
	m_Events.clear();
	m_SkillRef.clear();
	m_ClipMap.clear();
	m_ClipNotify.clear();
	m_ClipLength.clear();
	m_ClipSeqs.clear();
	m_iSelectedEvent = -1;
	m_bDirty = false;
	m_bLoadAttempted = false;
	m_bRefLoadAttempted = false;
	m_bClipMapLoadAttempted = false;
	m_bClipNotifyLoadAttempted = false;
	m_bClipSeqLoadAttempted = false;
}

bool_t Client::CAnimation_Tool::Is_Window(EVENT_KIND eKind)
{
	return ETOI(eKind) < ETOI(EVENT_KIND::SOUND);
}

const char_t* Client::CAnimation_Tool::Kind_Name(EVENT_KIND eKind)
{
	switch (eKind)
	{
	case EVENT_KIND::CANCEL:     return "CANCEL";
	case EVENT_KIND::SUPERARMOR: return "SUPERARMOR";
	case EVENT_KIND::INVULN:     return "INVULN";
	case EVENT_KIND::MOVE:       return "MOVE";
	case EVENT_KIND::SOUND:      return "SOUND";
	case EVENT_KIND::EFFECT:     return "EFFECT";
	case EVENT_KIND::SHAKE:      return "SHAKE";
	case EVENT_KIND::HIT:
	default:                     return "HIT";
	}
}

const char_t* Client::CAnimation_Tool::Area_Name(int32_t iAreaType)
{
	switch (iAreaType)
	{
	case 1:  return "box";
	case 2:  return "fan";
	case 3:  return "circle";
	default: return "none";
	}
}

f32_t Client::CAnimation_Tool::Get_ClipTickRate(const shared_ptr<Engine::CModel>& pModel,
	const std::string& clipName)
{
	if (nullptr != pModel)
	{
		const uint32_t iCount = pModel->Get_NumAnimations();
		for (uint32_t i = 0; i < iCount; ++i)
		{
			const char_t* pName = pModel->Get_AnimationName(i);
			if (nullptr != pName && clipName == pName)
			{
				const f32_t fRate = pModel->Get_AnimationTickPerSecond(i);
				if (fRate > 0.f)
					return fRate;
				break;
			}
		}
	}
	return DEFAULT_TICK_RATE;
}

int32_t Client::CAnimation_Tool::Ms_To_Frame(int32_t iMs, f32_t fTickRate)
{
	if (fTickRate <= 0.f)
		fTickRate = DEFAULT_TICK_RATE;

	const f32_t fFrame = static_cast<f32_t>(iMs) * fTickRate / 1000.f;
	return static_cast<int32_t>(fFrame >= 0.f ? fFrame + 0.5f : fFrame - 0.5f);
}

int32_t Client::CAnimation_Tool::Frame_To_Ms(int32_t iFrame, f32_t fTickRate)
{
	if (fTickRate <= 0.f)
		fTickRate = DEFAULT_TICK_RATE;

	const f32_t fMs = static_cast<f32_t>(iFrame) * 1000.f / fTickRate;
	return static_cast<int32_t>(fMs >= 0.f ? fMs + 0.5f : fMs - 0.5f);
}

/* Hit 0 lands at the window start and each repeat iRepeatMs later. The window
width is the tolerance of every hit, not a span the hits are spread across --
that is how the extracted game data is shaped. */
int32_t Client::CAnimation_Tool::Get_TickMs(const ANIM_EVENT& evt, int32_t iTickIndex)
{
	if (iTickIndex <= 0)
		return evt.iStartMs;

	return evt.iStartMs + evt.hit.iRepeatMs * iTickIndex;
}

int32_t Client::CAnimation_Tool::Get_ActiveTick(const ANIM_EVENT& evt, int32_t iMs)
{
	const int32_t iWidth = evt.iEndMs - evt.iStartMs;
	const int32_t iCount = evt.hit.iRepeatCount > 1 ? evt.hit.iRepeatCount : 1;

	for (int32_t i = 0; i < iCount; ++i)
	{
		const int32_t iTick = Get_TickMs(evt, i);
		if (iMs >= iTick && iMs <= iTick + iWidth)
			return i;
	}
	return -1;
}

std::string Client::CAnimation_Tool::Get_EventFilePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Authored") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animevents")).string();
}

std::string Client::CAnimation_Tool::Get_SkillReferencePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".skilltiming")).string();
}

std::string Client::CAnimation_Tool::Get_ClipMapPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipmap")).string();
}

std::string Client::CAnimation_Tool::Get_ClipNotifyPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animnotify")).string();
}

std::string Client::CAnimation_Tool::Get_ClipSeqPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipseq")).string();
}

int32_t Client::CAnimation_Tool::Get_ChainOffsetMs(const CLIP_SEQ& seq, int32_t iIndex) const
{
	f32_t fSeconds = 0.f;
	for (int32_t i = 0; i < iIndex && i < static_cast<int32_t>(seq.clips.size()); ++i)
	{
		const auto it = m_ClipLength.find(seq.clips[i]);
		if (m_ClipLength.end() != it)
			fSeconds += it->second;
	}
	return static_cast<int32_t>(fSeconds * 1000.f + 0.5f);
}

const Client::CAnimation_Tool::CLIP_INFO*
Client::CAnimation_Tool::Find_ClipInfo(const char_t* pClipName) const
{
	if (nullptr == pClipName)
		return nullptr;

	const auto it = m_ClipMap.find(pClipName);
	return m_ClipMap.end() == it ? nullptr : &it->second;
}

void Client::CAnimation_Tool::Select_Clip(const shared_ptr<Engine::CModel>& pModel,
	const std::string& clipName)
{
	const uint32_t iCount = pModel->Get_NumAnimations();
	for (uint32_t i = 0; i < iCount; ++i)
	{
		const char_t* pName = pModel->Get_AnimationName(i);
		if (nullptr != pName && clipName == pName)
		{
			pModel->Set_Animation(i, m_bLoop);
			pModel->Set_AnimTrackPosition(i, 0.f);
			m_iSelectedEvent = -1;
			return;
		}
	}
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

	Sync_AssetName();
	if (m_AssetName.empty())
	{
		ImGui::TextUnformatted("Character spec carries no asset name.");
		ImGui::End();
		return;
	}

	/* Load once the target actually exists, so a missing file is not reported
	before the level is even open. */
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		Load_Events(pModel);
	}

	/* The game-extracted skill timing is optional; a missing file just hides the
	reference panel and never blocks event authoring. */
	if (!m_bRefLoadAttempted)
	{
		m_bRefLoadAttempted = true;
		Load_SkillReference();
	}

	/* Also optional: without it the clip list just shows the English clip names. */
	if (!m_bClipMapLoadAttempted)
	{
		m_bClipMapLoadAttempted = true;
		Load_ClipMap();
	}

	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		Load_ClipSeq();
	}

	if (!m_bClipNotifyLoadAttempted)
	{
		m_bClipNotifyLoadAttempted = true;
		Load_ClipNotify();

		/* Timing kinds default on; effects and sounds are the noisy ones and are
		better pulled in deliberately. */
		m_bImportKind[ETOI(EVENT_KIND::HIT)] = true;
		m_bImportKind[ETOI(EVENT_KIND::CANCEL)] = true;
		m_bImportKind[ETOI(EVENT_KIND::SUPERARMOR)] = true;
		m_bImportKind[ETOI(EVENT_KIND::SHAKE)] = true;
	}

	ImGui::Text("Asset: %s   Animations: %u", m_AssetName.c_str(), pModel->Get_NumAnimations());

	Render_Playback(pModel);
	Render_ClipChain(pModel);
	Render_HitEvents(pModel);
	Render_SkillReference(pModel);

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

	if (const CLIP_INFO* pInfo = Find_ClipInfo(pCurrentName))
	{
		ImGui::SameLine();
		ImGui::TextDisabled("= %s (%d)", pInfo->name.c_str(), pInfo->iSkillId);
	}

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

/* The chains this clip belongs to. A skill plays its clips in a fixed order and
each tripod build takes a different route, so the same clip shows up in several.
Clicking a step both navigates and tells the reference panel where that step
starts on the whole-cast clock. */
void Client::CAnimation_Tool::Render_ClipChain(const shared_ptr<Engine::CModel>& pModel)
{
	if (m_ClipSeqs.empty())
		return;

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;

	int32_t iMatches = 0;
	for (const CLIP_SEQ& seq : m_ClipSeqs)
	{
		int32_t iPos = -1;
		for (int32_t i = 0; i < static_cast<int32_t>(seq.clips.size()); ++i)
		{
			if (seq.clips[i] == pCurrentName)
			{
				iPos = i;
				break;
			}
		}
		if (iPos < 0)
			continue;

		if (0 == iMatches)
			ImGui::SeparatorText("Chain");
		++iMatches;

		ImGui::PushID(seq.iSkillId * 100 + seq.iSeqIndex);

		const int32_t iOffsetMs = Get_ChainOffsetMs(seq, iPos);
		ImGui::Text("%s  seq%d  [%s]  %d/%d", seq.name.c_str(), seq.iSeqIndex,
			seq.sMode.empty() ? "?" : seq.sMode.c_str(),
			iPos + 1, static_cast<int32_t>(seq.clips.size()));

		ImGui::SameLine();
		char_t szUse[64]{};
		snprintf(szUse, sizeof(szUse), "use offset %d ms", iOffsetMs);
		if (ImGui::SmallButton(szUse))
		{
			m_iCastOffsetMs = iOffsetMs;
			m_Status = "Cast offset set to " + std::to_string(iOffsetMs) +
				" ms from " + seq.name + " seq" + std::to_string(seq.iSeqIndex);
		}

		for (int32_t i = 0; i < static_cast<int32_t>(seq.clips.size()); ++i)
		{
			if (i > 0)
				ImGui::SameLine(0.f, 2.f);

			ImGui::PushID(i);
			const bool_t bHere = (i == iPos);
			if (bHere)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

			char_t szStep[16]{};
			snprintf(szStep, sizeof(szStep), "%d", i + 1);
			if (ImGui::SmallButton(szStep))
				Select_Clip(pModel, seq.clips[i]);

			if (bHere)
				ImGui::PopStyleColor();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\nstarts %d ms into the cast",
					seq.clips[i].c_str(), Get_ChainOffsetMs(seq, i));

			ImGui::PopID();
		}

		ImGui::PopID();
	}
}

void Client::CAnimation_Tool::Render_HitEvents(const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Events");

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

	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	const int32_t iFrame = static_cast<int32_t>(fPosition);
	const int32_t iNowMs = Frame_To_Ms(iFrame, fRate);

	auto Add_Event = [&](EVENT_KIND eKind)
	{
		ANIM_EVENT evt{};
		evt.clipName = pCurrentName;
		evt.eKind = eKind;
		evt.iStartMs = iNowMs;
		evt.iEndMs = iNowMs;
		m_Events.push_back(evt);
		m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
		m_PayloadEdit[0] = '\0';
		m_bDirty = true;
	};

	/* Everything is added at the playhead; window kinds then grow an end. */
	if (ImGui::Button("Hit"))
		Add_Event(EVENT_KIND::HIT);
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		Add_Event(EVENT_KIND::CANCEL);
	ImGui::SameLine();
	if (ImGui::Button("SuperArmor"))
		Add_Event(EVENT_KIND::SUPERARMOR);
	ImGui::SameLine();
	if (ImGui::Button("Invuln"))
		Add_Event(EVENT_KIND::INVULN);
	ImGui::SameLine();
	if (ImGui::Button("Move"))
		Add_Event(EVENT_KIND::MOVE);

	if (ImGui::Button("Sound"))
		Add_Event(EVENT_KIND::SOUND);
	ImGui::SameLine();
	if (ImGui::Button("Effect"))
		Add_Event(EVENT_KIND::EFFECT);

	ImGui::SameLine();
	if (ImGui::Button("Save"))
		Save_Events();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		m_iSelectedEvent = -1;
		Load_Events(pModel);
	}

	ImGui::SameLine();
	ImGui::TextDisabled("%.0f fps", fRate);

	/* Original notifies for this clip, lifted from the game's Action table. */
	const auto itNotify = m_ClipNotify.find(pCurrentName);
	if (m_ClipNotify.end() != itNotify)
	{
		int32_t iAvailable = 0;
		for (const NOTIFY_ROW& row : itNotify->second)
		{
			if (m_bImportKind[ETOI(row.eKind)])
				++iAvailable;
		}

		char_t szImport[96]{};
		snprintf(szImport, sizeof(szImport), "Import original (%d of %d)",
			iAvailable, static_cast<int32_t>(itNotify->second.size()));

		if (ImGui::Button(szImport))
		{
			const int32_t iAdded = Import_Notifies(pCurrentName, fRate);
			m_Status = "Imported " + std::to_string(iAdded) +
				" original notify event(s) onto " + pCurrentName;
		}

		ImGui::SameLine();
		ImGui::TextDisabled("|");

		for (int32_t k = 0; k < ETOI(EVENT_KIND::END); ++k)
		{
			const EVENT_KIND eKind = static_cast<EVENT_KIND>(k);
			if (EVENT_KIND::INVULN == eKind || EVENT_KIND::MOVE == eKind)
				continue;

			ImGui::SameLine();
			ImGui::Checkbox(Kind_Name(eKind), &m_bImportKind[k]);
		}
	}

	if (m_bDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}

	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());

	if (!ImGui::BeginChild("##eventlist", ImVec2(0.f, 170.f), ImGuiChildFlags_Borders))
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

		const bool_t bWindow = Is_Window(evt.eKind);
		const bool_t bHit = EVENT_KIND::HIT == evt.eKind;

		int32_t iStartFrame = Ms_To_Frame(evt.iStartMs, fRate);
		int32_t iEndFrame = Ms_To_Frame(evt.iEndMs, fRate);

		/* A window highlights over its span (and over every repeat); a point event
		only on its own frame. */
		const int32_t iActiveTick = (bWindow && bHasTrack) ? Get_ActiveTick(evt, iNowMs) : -1;
		const bool_t bActive = bWindow ? (iActiveTick >= 0) : (iStartFrame == iFrame);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

		char_t szLabel[192]{};
		if (bHit && evt.hit.iRepeatCount > 1)
			snprintf(szLabel, sizeof(szLabel), "HIT %d - %d  x%d every %dms",
				iStartFrame, iEndFrame, evt.hit.iRepeatCount, evt.hit.iRepeatMs);
		else if (bWindow)
			snprintf(szLabel, sizeof(szLabel), "%s %d - %d  %s",
				Kind_Name(evt.eKind), iStartFrame, iEndFrame, evt.sPayload.c_str());
		else
			snprintf(szLabel, sizeof(szLabel), "%s %d  %s",
				Kind_Name(evt.eKind), iStartFrame,
				evt.sPayload.empty() ? "(unset)" : evt.sPayload.c_str());

		if (bWindow && bActive && evt.hit.iRepeatCount > 1)
		{
			char_t szActive[32]{};
			snprintf(szActive, sizeof(szActive), "  <== tick %d/%d",
				iActiveTick + 1, evt.hit.iRepeatCount);
			strcat_s(szLabel, szActive);
		}

		/* Imported rows are replaced wholesale on the next import, so they are
		worth telling apart from hand-authored ones. */
		if (evt.bImported)
			strcat_s(szLabel, "  *orig");

		if (ImGui::Selectable(szLabel, i == m_iSelectedEvent))
		{
			m_iSelectedEvent = i;
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, static_cast<f32_t>(iStartFrame));
			/* Prime the payload editor with the row just opened. */
			strncpy_s(m_PayloadEdit, evt.sPayload.c_str(), _TRUNCATE);
		}

		if (bActive)
			ImGui::PopStyleColor();

		if (i == m_iSelectedEvent)
		{
			const int32_t iMaxFrame = bHasTrack ? static_cast<int32_t>(fDuration) : 0;

			/* Frames are the editing unit but milliseconds are what is stored, so
			only a frame the user actually moved is written back. Converting both
			ways every frame would round the stored value away. */
			if (bWindow)
			{
				ImGui::SetNextItemWidth(200.f);
				if (ImGui::DragIntRange2("start / end", &iStartFrame, &iEndFrame,
					0.2f, 0, iMaxFrame, "%d", "%d"))
				{
					evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
					evt.iEndMs = Frame_To_Ms(iEndFrame, fRate);
					m_bDirty = true;
				}
			}
			else
			{
				ImGui::SetNextItemWidth(120.f);
				if (ImGui::DragInt("frame", &iStartFrame, 0.2f, 0, iMaxFrame))
				{
					evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
					evt.iEndMs = evt.iStartMs;
					m_bDirty = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Remove"))
				iRemoveIndex = i;

			ImGui::SameLine();
			ImGui::TextDisabled("%d - %d ms", evt.iStartMs, evt.iEndMs);

			if (bHit)
				Render_HitDetail(evt);
			else
			{
				const char_t* pHint = "effect / particle key";
				if (EVENT_KIND::SOUND == evt.eKind)
					pHint = "sound cue key";
				else if (bWindow)
					pHint = "what this window is for";

				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::InputTextWithHint("##payload", pHint,
					m_PayloadEdit, sizeof(m_PayloadEdit)))
				{
					evt.sPayload = m_PayloadEdit;
					m_bDirty = true;
				}
			}
		}

		ImGui::PopID();
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No event on this clip.");

	ImGui::EndChild();

	if (iRemoveIndex >= 0)
	{
		m_Events.erase(m_Events.begin() + iRemoveIndex);
		m_iSelectedEvent = -1;
		m_bDirty = true;
	}
}

/* The combat half of a HIT. Values here come straight from the extracted game
tables, so they stay in the source units (milliseconds and raw game distance)
rather than being normalised into frames or metres. */
void Client::CAnimation_Tool::Render_HitDetail(ANIM_EVENT& evt)
{
	HIT_PARAMS& p = evt.hit;

	if (ImGui::TreeNodeEx("##hitdetail", ImGuiTreeNodeFlags_DefaultOpen, "hit detail"))
	{
		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("repeat", &p.iRepeatCount, 0.1f, 1, 64))
		{
			if (p.iRepeatCount < 1)
				p.iRepeatCount = 1;
			m_bDirty = true;
		}

		if (p.iRepeatCount > 1)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("every (ms)", &p.iRepeatMs, 1.f, 0, 5000))
			{
				if (p.iRepeatMs < 0)
					p.iRepeatMs = 0;
				m_bDirty = true;
			}

			/* Spell the hit times out so they can be checked against the pose. */
			std::string ticks;
			for (int32_t t = 0; t < p.iRepeatCount && t < 16; ++t)
			{
				if (!ticks.empty())
					ticks += ", ";
				ticks += std::to_string(Get_TickMs(evt, t));
			}
			if (p.iRepeatCount > 16)
				ticks += ", ...";
			ImGui::TextWrapped("hits at: %s ms", ticks.c_str());
		}

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("freeze (ms)", &p.iFreezeMs, 1.f, 0, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragInt("in", &p.iFreezeInMs, 1.f, 0, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragInt("out", &p.iFreezeOutMs, 1.f, 0, 5000))
			m_bDirty = true;

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("push (ms)", &p.iPushMs, 1.f, -5000, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("distance", &p.iPushRange, 1.f, -5000, 5000))
			m_bDirty = true;
		if (p.iPushMs < 0)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(pull)");
		}

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::SliderInt("area", &p.iAreaType, 0, 3, Area_Name(p.iAreaType)))
			m_bDirty = true;

		if (p.iAreaType > 0)
		{
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("range", &p.iAreaRange, 1.f, 0, 10000))
				m_bDirty = true;

			/* A box has no sweep angle; the game data leaves it at 0. */
			if (1 != p.iAreaType)
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(110.f);
				if (ImGui::DragInt("angle", &p.iAreaAngle, 1.f, 0, 720))
					m_bDirty = true;
			}

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("height", &p.iAreaHeight, 1.f, 0, 10000))
				m_bDirty = true;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("offset X", &p.iAreaOffsetX, 1.f, -10000, 10000))
				m_bDirty = true;

			if (3 == p.iAreaType)
			{
				ImGui::SetNextItemWidth(110.f);
				if (ImGui::DragInt("inner", &p.iAreaInner, 1.f, 0, 10000))
					m_bDirty = true;
			}

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("max targets", &p.iMaxTargets, 0.1f, 0, 64))
				m_bDirty = true;
		}

		ImGui::TreePop();
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
		if (nullptr == pName)
			continue;

		/* The clip names are English and the skill names Korean, so the filter has
		to look at both or a Korean search finds nothing. */
		const CLIP_INFO* pInfo = Find_ClipInfo(pName);
		if (!Contains_NoCase(pName, m_Filter) &&
			(nullptr == pInfo || !Contains_NoCase(pInfo->name.c_str(), m_Filter)))
			continue;

		++iShown;

		/* Mark clips that already carry events so authored work is easy to find. */
		int32_t iEventCount = 0;
		for (const ANIM_EVENT& evt : m_Events)
		{
			if (evt.clipName == pName)
				++iEventCount;
		}

		char_t szLabel[MAX_PATH + 160]{};
		snprintf(szLabel, sizeof(szLabel), "%3u  %s", i, pName);

		if (nullptr != pInfo)
		{
			char_t szSkill[128]{};
			snprintf(szSkill, sizeof(szSkill), "   %s (%d)",
				pInfo->name.c_str(), pInfo->iSkillId);
			strcat_s(szLabel, szSkill);
		}

		if (iEventCount > 0)
		{
			char_t szCount[16]{};
			snprintf(szCount, sizeof(szCount), "  [%d]", iEventCount);
			strcat_s(szLabel, szCount);
		}

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

	error_code directoryError;
	filesystem::create_directories(
		filesystem::path(path).parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Save failed to create authoring directory: " +
			directoryError.message();
		return false;
	}

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
		const char_t* pKind = Kind_Name(evt.eKind);

		if (EVENT_KIND::HIT == evt.eKind)
		{
			const HIT_PARAMS& p = evt.hit;
			fprintf(pFile, "\"%s\" %s startms=%d endms=%d rep=%d repms=%d "
				"fz=%d fzin=%d fzout=%d push=%d pushr=%d "
				"area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d%s\n",
				evt.clipName.c_str(), pKind, evt.iStartMs, evt.iEndMs,
				p.iRepeatCount, p.iRepeatMs,
				p.iFreezeMs, p.iFreezeInMs, p.iFreezeOutMs, p.iPushMs, p.iPushRange,
				p.iAreaType, p.iAreaRange, p.iAreaAngle, p.iAreaHeight, p.iAreaOffsetX,
				p.iAreaInner, p.iMaxTargets, evt.bImported ? " src=orig" : "");
		}
		else if (Is_Window(evt.eKind))
			/* State windows carry their span plus, for cancels, what they are for. */
			fprintf(pFile, "\"%s\" %s startms=%d endms=%d payload=\"%s\"%s\n",
				evt.clipName.c_str(), pKind, evt.iStartMs, evt.iEndMs,
				evt.sPayload.c_str(), evt.bImported ? " src=orig" : "");
		else
			/* Point events store only the instant and the cue/effect key. */
			fprintf(pFile, "\"%s\" %s startms=%d payload=\"%s\"%s\n",
				evt.clipName.c_str(), pKind, evt.iStartMs, evt.sPayload.c_str(),
				evt.bImported ? " src=orig" : "");
	}

	fclose(pFile);

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(m_Events.size()) + " event(s) to " + path;
	return true;
}

bool_t Client::CAnimation_Tool::Load_Events(const shared_ptr<Engine::CModel>& pModel)
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

		ANIM_EVENT evt{};
		evt.clipName = clip;

		/* v1/v2 carried frames; they are converted once the kind and clip are known. */
		int32_t iStartFrame = 0;
		int32_t iEndFrame = 0;

		/* Unknown kinds are skipped rather than treated as fatal, so a file written
		by a still newer tool keeps loading the kinds this build understands. */
		if ("HIT" == kind)
			evt.eKind = EVENT_KIND::HIT;
		else if ("CANCEL" == kind)
			evt.eKind = EVENT_KIND::CANCEL;
		else if ("SUPERARMOR" == kind)
			evt.eKind = EVENT_KIND::SUPERARMOR;
		else if ("INVULN" == kind)
			evt.eKind = EVENT_KIND::INVULN;
		else if ("MOVE" == kind)
			evt.eKind = EVENT_KIND::MOVE;
		else if ("SOUND" == kind)
			evt.eKind = EVENT_KIND::SOUND;
		else if ("EFFECT" == kind)
			evt.eKind = EVENT_KIND::EFFECT;
		else if ("SHAKE" == kind)
			evt.eKind = EVENT_KIND::SHAKE;
		else
			continue;

		if (1 == iVersion)
		{
			/* v1 only ever wrote HIT rows as two bare positional columns. */
			std::string startToken, endToken;
			if (!Read_Token(p, startToken) || !Read_Token(p, endToken))
				continue;
			iStartFrame = atoi(startToken.c_str());
			iEndFrame = atoi(endToken.c_str());
		}
		else
		{
			HIT_PARAMS& hp = evt.hit;
			std::string key, value;
			int32_t iInterval = 0;
			while (Read_Pair(p, key, value))
			{
				const int32_t v = atoi(value.c_str());

				/* Unknown keys are ignored so new fields do not break old builds. */
				if ("start" == key)            /* v2, frames */
					iStartFrame = v;
				else if ("end" == key)         /* v2, frames */
					iEndFrame = v;
				else if ("count" == key)       /* v2 */
					hp.iRepeatCount = v;
				else if ("interval" == key)    /* v2, frames */
					iInterval = v;
				else if ("startms" == key)
					evt.iStartMs = v;
				else if ("endms" == key)
					evt.iEndMs = v;
				else if ("rep" == key)
					hp.iRepeatCount = v;
				else if ("repms" == key)
					hp.iRepeatMs = v;
				else if ("fz" == key)
					hp.iFreezeMs = v;
				else if ("fzin" == key)
					hp.iFreezeInMs = v;
				else if ("fzout" == key)
					hp.iFreezeOutMs = v;
				else if ("push" == key)
					hp.iPushMs = v;
				else if ("pushr" == key)
					hp.iPushRange = v;
				else if ("area" == key)
					hp.iAreaType = v;
				else if ("ar" == key)
					hp.iAreaRange = v;
				else if ("aa" == key)
					hp.iAreaAngle = v;
				else if ("ah" == key)
					hp.iAreaHeight = v;
				else if ("ax" == key)
					hp.iAreaOffsetX = v;
				else if ("arem" == key)
					hp.iAreaInner = v;
				else if ("maxt" == key)
					hp.iMaxTargets = v;
				else if ("payload" == key)
					evt.sPayload = value;
				else if ("src" == key)
					evt.bImported = ("orig" == value);
			}

			/* v2 spread its hits evenly across the window; v3 repeats them at a
			fixed interval. Convert the spread into the interval it worked out to
			so authored timing is not silently moved. */
			if (iVersion < 3)
			{
				const f32_t fRate = Get_ClipTickRate(pModel, evt.clipName);

				if (iInterval > 0 && hp.iRepeatCount <= 1)
					hp.iRepeatCount = (iEndFrame - iStartFrame) / iInterval + 1;

				if (hp.iRepeatCount > 1)
				{
					/* Divide in milliseconds rather than frames: a span that does
					not split evenly into frames still lands its last hit where v2
					put it instead of drifting a frame early. */
					hp.iRepeatMs = iInterval > 0
						? Frame_To_Ms(iInterval, fRate)
						: Frame_To_Ms(iEndFrame - iStartFrame, fRate) / (hp.iRepeatCount - 1);

					/* The v2 window was the whole spread; v3 wants one hit's tolerance. */
					iEndFrame = iStartFrame;
				}
			}
		}

		if (iVersion < 3)
		{
			/* Frames are only meaningful against the clip they were authored on. */
			const f32_t fRate = Get_ClipTickRate(pModel, evt.clipName);
			evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
			evt.iEndMs = Frame_To_Ms(iEndFrame, fRate);
		}

		if (evt.hit.iRepeatCount < 1)
			evt.hit.iRepeatCount = 1;

		/* A point event has no window; keep end pinned to start. */
		if (!Is_Window(evt.eKind))
			evt.iEndMs = evt.iStartMs;
		if (evt.iEndMs < evt.iStartMs)
			evt.iEndMs = evt.iStartMs;

		loaded.push_back(evt);
	}

	fclose(pFile);

	m_Events = std::move(loaded);
	m_bDirty = false;
	m_Status = "Loaded " + std::to_string(m_Events.size()) + " event(s) from " + path;

	/* An older file was just converted from frames. Rounding a hit that sat on a
	fractional frame can move it by one, so say so rather than let it pass as an
	ordinary load; saving writes the converted times back. */
	if (iVersion < EVENT_FILE_VERSION)
		m_Status += "  [converted from v" + std::to_string(iVersion) +
			" frames to ms - check multi-hit clips before saving]";

	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipMap()
{
	m_ClipMap.clear();

	const std::string path = Get_ClipMapPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[1024]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_MAP")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string clip;
		if (!Read_Quoted(p, clip))
			continue;

		CLIP_INFO info{};
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("skill" == key)
				info.iSkillId = atoi(value.c_str());
			else if ("name" == key)
				info.name = value;
		}

		m_ClipMap[clip] = info;
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipNotify()
{
	m_ClipNotify.clear();

	const std::string path = Get_ClipNotifyPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_ANIM_NOTIFY")
	{
		fclose(pFile);
		return false;
	}

	std::vector<NOTIFY_ROW>* pCurrent = nullptr;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		/* A clip header opens with a quoted name; its rows start with "n". */
		std::string token;
		const char_t* pProbe = p;
		if (Read_Quoted(pProbe, token))
		{
			pCurrent = &m_ClipNotify[token];

			/* The header also carries the clip's own length, which the chain
			offsets are summed from. */
			std::string hkey, hvalue;
			while (Read_Pair(pProbe, hkey, hvalue))
			{
				if ("len" == hkey)
					m_ClipLength[token] = static_cast<f32_t>(atof(hvalue.c_str()));
			}
			continue;
		}

		if (!Read_Token(p, token) || "n" != token || nullptr == pCurrent)
			continue;

		NOTIFY_ROW row{};
		bool_t bKnown = false;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("t" == key)
				row.fTime = static_cast<f32_t>(atof(value.c_str()));
			else if ("d" == key)
				row.fDuration = static_cast<f32_t>(atof(value.c_str()));
			else if ("asset" == key)
				row.sAsset = value;
			else if ("label" == key)
				row.sLabel = value;
			else if ("kind" == key)
			{
				bKnown = true;
				if ("HIT" == value)
					row.eKind = EVENT_KIND::HIT;
				else if ("CANCEL" == value)
					row.eKind = EVENT_KIND::CANCEL;
				else if ("SUPERARMOR" == value)
					row.eKind = EVENT_KIND::SUPERARMOR;
				else if ("SOUND" == value)
					row.eKind = EVENT_KIND::SOUND;
				else if ("EFFECT" == value)
					row.eKind = EVENT_KIND::EFFECT;
				else if ("SHAKE" == value)
					row.eKind = EVENT_KIND::SHAKE;
				else
					bKnown = false;
			}
		}

		if (bKnown)
			pCurrent->push_back(row);
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipSeq()
{
	m_ClipSeqs.clear();

	const std::string path = Get_ClipSeqPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_SEQ")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken) || !Read_Quoted(p, name))
			continue;

		CLIP_SEQ seq{};
		seq.iSkillId = atoi(idToken.c_str());
		seq.name = name;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				seq.iSeqIndex = atoi(value.c_str());
			else if ("mode" == key)
				seq.sMode = value;
			else if ("clips" == key)
			{
				/* comma separated, in playback order */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);
					if (!one.empty())
						seq.clips.push_back(one);
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		/* One-clip chains are kept: they are still what the skill casts, and the
		runtime reads the same file. */
		if (!seq.clips.empty())
			m_ClipSeqs.push_back(seq);
	}

	fclose(pFile);
	return true;
}

int32_t Client::CAnimation_Tool::Import_Notifies(const char_t* pClipName, f32_t fTickRate)
{
	const auto it = m_ClipNotify.find(pClipName);
	if (m_ClipNotify.end() == it)
		return 0;

	/* Drop only what a previous import put here so hand-authored rows survive. */
	for (auto e = m_Events.begin(); e != m_Events.end(); )
		e = (e->bImported && e->clipName == pClipName) ? m_Events.erase(e) : e + 1;

	int32_t iAdded = 0;
	for (const NOTIFY_ROW& row : it->second)
	{
		if (!m_bImportKind[ETOI(row.eKind)])
			continue;

		ANIM_EVENT evt{};
		evt.clipName = pClipName;
		evt.eKind = row.eKind;
		evt.bImported = true;
		evt.iStartMs = static_cast<int32_t>(row.fTime * 1000.f + 0.5f);
		evt.iEndMs = Is_Window(row.eKind)
			? evt.iStartMs + static_cast<int32_t>(row.fDuration * 1000.f + 0.5f)
			: evt.iStartMs;
		/* Point kinds name what they fire; a window instead carries the game's
		note on what it is for, which is the only thing distinguishing a move
		cancel from a dodge cancel. */
		evt.sPayload = Is_Window(row.eKind) ? row.sLabel : row.sAsset;

		/* The game spawns the same particle from several attach points; for
		authoring those collapse to one row. */
		bool_t bDuplicate = false;
		for (const ANIM_EVENT& other : m_Events)
		{
			if (other.bImported && other.clipName == evt.clipName &&
				other.eKind == evt.eKind && other.iStartMs == evt.iStartMs &&
				other.iEndMs == evt.iEndMs && other.sPayload == evt.sPayload)
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate)
			continue;

		m_Events.push_back(evt);
		++iAdded;
	}

	m_iSelectedEvent = -1;
	m_bDirty = true;
	return iAdded;
}

bool_t Client::CAnimation_Tool::Load_SkillReference()
{
	m_SkillRef.clear();

	const std::string path = Get_SkillReferencePath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_SKILL_TIMING")
	{
		fclose(pFile);
		return false;
	}

	/* Set once the row being read gets a hit line of its own, so the windows read
	off the skill line are only a stand-in for v1 files. */
	bool_t bCurrentHasDetail = false;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken))
			continue;

		/* A v2 "hit" continuation line details the skill line above it. v1 files
		have none, and an older build drops them because they carry no quoted
		name where it expects one. */
		if ("hit" == idToken && !m_SkillRef.empty())
		{
			if (!bCurrentHasDetail)
			{
				bCurrentHasDetail = true;
				m_SkillRef.back().hits.clear();
			}

			SKILL_HIT hit{};
			HIT_PARAMS& hp = hit.hit;

			std::string key, value;
			while (Read_Pair(p, key, value))
			{
				const int32_t v = atoi(value.c_str());

				if ("t" == key)
					hit.iTimeMs = v;
				else if ("w" == key)
					hit.iWidthMs = v;
				else if ("timed" == key)
					hit.bTimed = (0 != v);
				else if ("rep" == key)
					hp.iRepeatCount = v;
				else if ("repms" == key)
					hp.iRepeatMs = v;
				else if ("fz" == key)
					hp.iFreezeMs = v;
				else if ("fzin" == key)
					hp.iFreezeInMs = v;
				else if ("fzout" == key)
					hp.iFreezeOutMs = v;
				else if ("push" == key)
					hp.iPushMs = v;
				else if ("pushr" == key)
					hp.iPushRange = v;
				else if ("area" == key)
					hp.iAreaType = v;
				else if ("ar" == key)
					hp.iAreaRange = v;
				else if ("aa" == key)
					hp.iAreaAngle = v;
				else if ("ah" == key)
					hp.iAreaHeight = v;
				else if ("ax" == key)
					hp.iAreaOffsetX = v;
				else if ("arem" == key)
					hp.iAreaInner = v;
				else if ("maxt" == key)
					hp.iMaxTargets = v;
			}

			if (hp.iRepeatCount < 1)
				hp.iRepeatCount = 1;

			m_SkillRef.back().hits.push_back(hit);
			continue;
		}

		if (!Read_Quoted(p, name))
			continue;

		SKILL_TIMING row{};
		row.iSkillId = atoi(idToken.c_str());
		row.name = name;

		/* v1 has no hit lines, so its windows stand in as bare hits with no combat
		detail. A following hit line replaces them. */
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("sa" == key)
				row.iSuperArmor = atoi(value.c_str());
			else if ("move" == key)
				row.iMoveSpeed = atoi(value.c_str());
			else if ("base" == key)
				row.iBaseSkillId = atoi(value.c_str());
			else if ("hits" == key)
			{
				/* value is "a-b,c-d": comma-separated second windows. */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string span = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);

					const size_t dash = span.find('-');
					if (std::string::npos != dash)
					{
						const f32_t a = static_cast<f32_t>(atof(span.substr(0, dash).c_str()));
						const f32_t b = static_cast<f32_t>(atof(span.substr(dash + 1).c_str()));

						SKILL_HIT hit{};
						hit.iTimeMs = static_cast<int32_t>(a * 1000.f + 0.5f);
						hit.iWidthMs = static_cast<int32_t>((b - a) * 1000.f + 0.5f);
						row.hits.push_back(hit);
					}

					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		m_SkillRef.push_back(row);
		bCurrentHasDetail = false;
	}

	fclose(pFile);
	return true;
}

void Client::CAnimation_Tool::Render_SkillReference(const shared_ptr<Engine::CModel>& pModel)
{
	if (m_SkillRef.empty())
		return;

	/* Its own floating window instead of a header embedded in the main tool, so it
	can be dragged out and placed wherever (e.g. next to the event list). */
	ImGui::SetNextWindowSize(ImVec2(520.f, 420.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(500.f, 60.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Skill Timing Reference (game-extracted)"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("Times run from the start of the whole skill cast (all its clips in "
		"sequence). For a mid-cast clip set the offset to where that clip begins in the "
		"cast so a stamp lands on the right local time.");

	ImGui::SetNextItemWidth(160.f);
	ImGui::DragInt("cast offset (ms)", &m_iCastOffsetMs, 5.f, 0, 100000);
	if (m_iCastOffsetMs < 0)
		m_iCastOffsetMs = 0;

	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##reffilter", "filter by id or name", m_RefFilter, sizeof(m_RefFilter));

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	const f32_t fRate = Get_ClipTickRate(pModel,
		nullptr != pCurrentName ? pCurrentName : "");

	if (!ImGui::BeginChild("##reflist", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		ImGui::End();
		return;
	}

	for (const SKILL_TIMING& row : m_SkillRef)
	{
		char_t szId[32]{};
		snprintf(szId, sizeof(szId), "%d", row.iSkillId);
		if (!Contains_NoCase(szId, m_RefFilter) && !Contains_NoCase(row.name.c_str(), m_RefFilter))
			continue;

		ImGui::PushID(row.iSkillId);

		ImGui::Text("%d  %s", row.iSkillId, row.name.empty() ? "(no name)" : row.name.c_str());
		if (row.iSuperArmor > 0 || row.iMoveSpeed > 0)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("superarmor %d  move %d", row.iSuperArmor, row.iMoveSpeed);
		}

		/* The clips this skill actually plays, so the right one can be opened
		without guessing which English clip name matches the Korean skill. */
		const int32_t iOwner = row.iBaseSkillId > 0 ? row.iBaseSkillId : row.iSkillId;
		int32_t iClipButton = 0;
		for (const auto& entry : m_ClipMap)
		{
			if (entry.second.iSkillId != iOwner)
				continue;

			if (iClipButton > 0 && (iClipButton % 3) != 0)
				ImGui::SameLine();
			++iClipButton;

			ImGui::PushID(iClipButton);
			if (ImGui::SmallButton(entry.first.c_str()))
				Select_Clip(pModel, entry.first);
			ImGui::PopID();
		}

		for (size_t h = 0; h < row.hits.size(); ++h)
		{
			const SKILL_HIT& src = row.hits[h];

			/* Cast time minus where the current clip starts in the cast. */
			int32_t iStartMs = src.iTimeMs - m_iCastOffsetMs;
			if (iStartMs < 0)
				iStartMs = 0;
			const int32_t iEndMs = iStartMs + src.iWidthMs;

			ImGui::PushID(static_cast<int32_t>(h));

			char_t szBtn[192]{};
			if (src.bTimed)
				snprintf(szBtn, sizeof(szBtn), "stamp  %d-%d ms  ->  frame %d  %s%d",
					src.iTimeMs, src.iTimeMs + src.iWidthMs,
					Ms_To_Frame(iStartMs, fRate),
					Area_Name(src.hit.iAreaType), src.hit.iAreaRange);
			else
				/* No time in the source data; it lands at the playhead instead and
				only the combat values are worth copying. */
				snprintf(szBtn, sizeof(szBtn), "stamp (untimed)  %s%d  freeze %d",
					Area_Name(src.hit.iAreaType), src.hit.iAreaRange, src.hit.iFreezeMs);

			ImGui::BeginDisabled(nullptr == pCurrentName);
			if (ImGui::Button(szBtn))
			{
				ANIM_EVENT evt{};
				evt.clipName = pCurrentName;
				evt.eKind = EVENT_KIND::HIT;
				evt.hit = src.hit;

				if (src.bTimed)
				{
					evt.iStartMs = iStartMs;
					evt.iEndMs = iEndMs;
				}
				else
				{
					f32_t fPosition = 0.f;
					f32_t fDuration = 0.f;
					pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration);
					evt.iStartMs = Frame_To_Ms(static_cast<int32_t>(fPosition), fRate);
					evt.iEndMs = evt.iStartMs;
				}

				m_Events.push_back(evt);
				m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
				m_PayloadEdit[0] = '\0';
				m_bDirty = true;
				m_Status = "Stamped skill " + std::to_string(row.iSkillId) +
					" onto " + pCurrentName;
			}
			ImGui::EndDisabled();

			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::End();
}
