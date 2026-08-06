#include "imgui.h"

#include "Animation_Tool.h"

#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "Effect_Catalog.h"
#include "EffectAuthoringTransfer.h"
#include "GameInstance.h"
#include "Model.h"
#include "Part_Body.h"
#include "ProjectDataRoot.h"
#include "Transform.h"

#include <charconv>
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <io.h>
#include <system_error>
#include <unordered_set>

namespace
{
	constexpr const char_t* EVENT_FILE_MAGIC = "LOSTARK_ANIM_EVENTS";
	/* v5 adds admitted Effect anchor/follow/stop/local-transform fields. */
	constexpr int32_t EVENT_FILE_VERSION = 5;
	constexpr int32_t MAX_EVENT_COUNT = 100000;

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
			if ('\"' != *p)
				return false;
			++p;
		}
		else
		{
			while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
				value.push_back(*p++);
		}
		return true;
	}

	bool_t Parse_Integer(const std::string& text, int32_t& outValue)
	{
		if (text.empty())
			return false;

		const char_t* begin = text.data();
		const char_t* end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end;
	}

	bool_t Parse_Float(const std::string& text, f32_t& outValue)
	{
		if (text.empty())
			return false;
		char_t* pEnd = nullptr;
		outValue = std::strtof(text.c_str(), &pEnd);
		return pEnd == text.c_str() + text.size() &&
			std::isfinite(outValue);
	}

	bool_t Has_RemainingToken(const char_t* p)
	{
		Skip_Space(p);
		return '\0' != *p && '\r' != *p && '\n' != *p;
	}

	bool_t Is_SafeQuotedText(const std::string& text)
	{
		return std::string::npos == text.find_first_of("\"\r\n");
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

Client::CAnimation_Tool::CAnimation_Tool(
	shared_ptr<CCharacterPreviewPanel> pPreviewPanel)
	: m_pPreviewPanel(std::move(pPreviewPanel))
{
}

Client::CAnimation_Tool::~CAnimation_Tool()
{
	if (nullptr != m_pPreviewPanel)
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, false, {});
}

shared_ptr<Engine::CModel> Client::CAnimation_Tool::Resolve_Model() const
{
	return CAnimationTargetService::Resolve_Model();
}

shared_ptr<Client::CCharacter> Client::CAnimation_Tool::Resolve_Character() const
{
	return CAnimationTargetService::Resolve_Character();
}

bool_t Client::CAnimation_Tool::Is_AnyDocumentDirty() const
{
	return m_bDirty || m_bSkillBindingDirty;
}

bool_t Client::CAnimation_Tool::Sync_AssetName()
{
	const string assetName =
		CAnimationTargetService::Resolve_AssetName();
	if (assetName.empty())
	{
		if (Is_AnyDocumentDirty())
		{
			m_PendingAssetName.clear();
			m_Status =
				"Animation target disappeared while an authored document is dirty. "
				"The unsaved document is preserved.";
			return false;
		}
		return true;
	}
	if (m_AssetName == assetName)
	{
		m_PendingAssetName.clear();
		return true;
	}
	if (Is_AnyDocumentDirty() && !m_AssetName.empty())
	{
		m_PendingAssetName = assetName;
		m_Status =
			"Target changed while an Animation document is dirty. "
			"Return to the original target and Save, or discard explicitly.";
		return false;
	}

	Adopt_AssetName(assetName);
	return true;
}

void Client::CAnimation_Tool::Adopt_AssetName(
	const std::string& assetName)
{
	/* A different class means every loaded file belongs to the wrong asset. Drop
	them rather than mixing two classes' clips in one list. */
	m_AssetName = assetName;
	m_PendingAssetName.clear();
	m_Events.clear();
	m_SkillRef.clear();
	m_ClipMap.clear();
	m_ClipNotify.clear();
	m_ClipLength.clear();
	m_ClipSeqs.clear();
	m_SkillBindingDocument = {};
	m_iSelectedEvent = -1;
	m_bDirty = false;
	m_bSkillBindingDirty = false;
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_SkillBindingStatus.clear();
	m_bLoadAttempted = false;
	m_bRefLoadAttempted = false;
	m_bClipMapLoadAttempted = false;
	m_bClipNotifyLoadAttempted = false;
	m_bClipSeqLoadAttempted = false;
	m_bSkillBindingLoadAttempted = false;
}

void Client::CAnimation_Tool::Render_TargetConflict()
{
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Text("Unsaved document: %s", m_AssetName.c_str());
	if (!m_PendingAssetName.empty())
	{
		ImGui::Text("Pending target: %s", m_PendingAssetName.c_str());
		if (ImGui::Button("Discard Unsaved Animation Documents and Switch"))
			Adopt_AssetName(m_PendingAssetName);
		return;
	}

	ImGui::TextUnformatted("The original target is no longer available.");
	if (ImGui::Button("Discard Orphaned Animation Documents"))
		Adopt_AssetName(std::string{});
}

/* Preview lifecycle now belongs to CCharacterPreviewPanel so Effect authoring
   can read the same target. The tool keeps only the dirty-document policy that
   decides when the target may change. */

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
	const bool_t isTargetLocked = Is_AnyDocumentDirty();
	m_pPreviewPanel->Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL,
		isTargetLocked,
		"Save or discard Animation Events and Skill Bindings before changing target.");
	m_pPreviewPanel->Refresh_Level();

	if (!ImGui::Begin("LostArk Animation Tool"))
	{
		ImGui::End();
		return;
	}

	m_pPreviewPanel->Render_Selector(false, {});
	if (!Sync_AssetName())
	{
		Render_TargetConflict();
		ImGui::End();
		return;
	}

	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		ImGui::TextUnformatted("No animated character resolved.");
		ImGui::Separator();
		ImGui::TextUnformatted(
			"Enter Character Select or Development from the Lobby.");
		ImGui::End();
		return;
	}

	if (m_AssetName.empty())
	{
		ImGui::TextUnformatted("The selected target carries no asset name.");
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
	Consume_EffectTransfer(pModel);

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
	Render_SkillBindings(pModel, Resolve_Character());
	Render_HitEvents(pModel);
	Render_SkillReference(pModel);

	ImGui::SeparatorText("Clips");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##filter", "filter by name", m_Filter, sizeof(m_Filter));

	Render_AnimationList(pModel);

	ImGui::End();
}

void Client::CAnimation_Tool::Consume_EffectTransfer(
	const shared_ptr<Engine::CModel>& pModel)
{
	EFFECT_AUTHORING_CUE_TRANSFER Transfer;
	if (!CEffectAuthoringTransfer::Consume(Transfer))
		return;
	if (Transfer.iTargetGeneration !=
		CAnimationTargetService::Resolve_TargetGeneration() ||
		Transfer.strAnimationAssetId != m_AssetName ||
		Transfer.strClipName.empty() ||
		!CEffectCatalog::Contains(Transfer.strEffectAssetId) ||
		Transfer.ePivotKind >= EFFECT_CUE_PIVOT_KIND::END)
	{
		m_Status = "Effect cue transfer rejected because the animation target or admitted Effect changed.";
		return;
	}

	ANIM_EVENT Event;
	Event.clipName = Transfer.strClipName;
	Event.eKind = EVENT_KIND::EFFECT;
	Event.iStartMs = static_cast<int32_t>(Transfer.iTimeMs);
	Event.iEndMs = EFFECT_STOP_POLICY::CUE_END == Transfer.eStopPolicy ?
		Event.iStartMs + static_cast<int32_t>(Transfer.iDurationMs) :
		Event.iStartMs;
	Event.sPayload = Transfer.strEffectAssetId;
	Event.eEffectReferenceKind = EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
	Event.sAnchorSlotId = Transfer.strAnchorSlotId;
	Event.eFollowPolicy = Transfer.eFollowPolicy;
	Event.eStopPolicy = Transfer.eStopPolicy;
	Event.EffectLocalTransform = Transfer.LocalTransform;

	const auto Duplicate = std::find_if(
		m_Events.begin(), m_Events.end(),
		[&Event](const ANIM_EVENT& Existing)
		{
			return Existing.eKind == EVENT_KIND::EFFECT &&
				Existing.eEffectReferenceKind ==
					EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID &&
				Existing.clipName == Event.clipName &&
				Existing.iStartMs == Event.iStartMs &&
				Existing.sPayload == Event.sPayload &&
				Existing.sAnchorSlotId == Event.sAnchorSlotId;
		});
	if (Duplicate != m_Events.end())
	{
		m_Status = "Effect cue transfer rejected because the same cue already exists.";
		return;
	}

	std::vector<ANIM_EVENT> Staged = m_Events;
	Staged.push_back(Event);
	std::string ValidationStatus;
	if (!Validate_Events(pModel, Staged, ValidationStatus))
	{
		m_Status = "Effect cue transfer rejected: " + ValidationStatus;
		return;
	}
	m_Events = std::move(Staged);
	m_iSelectedEvent = static_cast<int32_t>(m_Events.size() - 1u);
	strncpy_s(m_PayloadEdit, Event.sPayload.c_str(), _TRUNCATE);
	m_bDirty = true;
	Select_Clip(pModel, Event.clipName);
	const f32_t fTickRate = Get_ClipTickRate(pModel, Event.clipName);
	pModel->Set_AnimPaused(true);
	pModel->Set_AnimTrackPosition(
		pModel->Get_CurrentAnimIndex(),
		static_cast<f32_t>(Event.iStartMs) * fTickRate / 1000.f);
	m_iSelectedEvent = static_cast<int32_t>(m_Events.size() - 1u);
	m_Status = "Added admitted Effect cue from Effect Tool; review and Save explicitly.";
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

std::vector<std::string> Client::CAnimation_Tool::Collect_ClipNames(
	const shared_ptr<Engine::CModel>& pModel) const
{
	std::vector<std::string> clips;
	if (nullptr == pModel)
		return clips;
	clips.reserve(pModel->Get_NumAnimations());
	for (uint32_t index = 0; index < pModel->Get_NumAnimations(); ++index)
	{
		const char_t* clipName = pModel->Get_AnimationName(index);
		if (nullptr != clipName)
			clips.emplace_back(clipName);
	}
	return clips;
}

Client::ANIMATION_SKILL_BINDING*
Client::CAnimation_Tool::Find_SkillBinding(
	const LostArk::Shared::SKILL_ID skillId)
{
	for (ANIMATION_SKILL_BINDING& binding :
		m_SkillBindingDocument.Bindings)
	{
		if (binding.iSkillId == skillId)
			return &binding;
	}
	return nullptr;
}

bool_t Client::CAnimation_Tool::Load_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	std::string status;
	if (!CAnimationSkillBindingDocument::Load(
		m_AssetName,
		characterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		staged,
		status))
	{
		m_SkillBindingStatus =
			"Load rejected; current Skill Bindings preserved: " + status;
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = false;
	m_SkillBindingStatus = status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		m_SkillBindingStatus =
			"Skill Bindings can only be saved for the selected Scene Character.";
		return false;
	}

	std::string status;
	if (!CAnimationSkillBindingDocument::Save_Atomic(
		m_SkillBindingDocument,
		m_AssetName,
		pCharacter->Get_Spec()->eCharacterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		status))
	{
		m_SkillBindingStatus =
			"Save rejected; destination and current bindings preserved: " + status;
		return false;
	}

	m_bSkillBindingDirty = false;
	if (!pCharacter->Reload_SkillAnimationBindings())
	{
		m_SkillBindingStatus = status +
			" [saved, but the live Character kept its previous binding set]";
		return true;
	}
	m_SkillBindingStatus = status + " [live Character refreshed]";
	return true;
}

bool_t Client::CAnimation_Tool::Create_SkillBindingDraft(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (nullptr == pModel || m_AssetName.empty())
		return false;
	const char_t* currentClip = pModel->Get_AnimationName(
		pModel->Get_CurrentAnimIndex());
	if (nullptr == currentClip)
	{
		m_SkillBindingStatus =
			"Select one model clip before creating a repair draft.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	staged.strAnimationAssetId = m_AssetName;
	staged.eCharacterClass = characterClass;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass != characterClass)
			continue;
		ANIMATION_SKILL_BINDING binding;
		binding.iSkillId = definition.iSkillId;
		const std::size_t clipCount =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			definition.eSkillKind ? definition.iComboStageCount : 1u;
		binding.Clips.assign(
			clipCount, ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
		staged.Bindings.push_back(std::move(binding));
	}
	if (staged.Bindings.empty())
	{
		m_SkillBindingStatus =
			"PlayerSkills has no definitions for this Character class.";
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = 0;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = true;
	m_SkillBindingStatus =
		"Created a complete repair draft. Every row currently uses " +
		std::string(currentClip) +
		"; assign intended clips before Save.";
	return true;
}

void Client::CAnimation_Tool::Render_SkillBindingReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (m_bSkillBindingReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Skill Animation Bindings?");
		m_bSkillBindingReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal(
		"Discard unsaved Skill Animation Bindings?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	ImGui::TextUnformatted(
		"Reload replaces only the unsaved key/skill animation binding document.");
	if (ImGui::Button("Discard Bindings and Reload"))
	{
		if (Load_SkillBindings(pModel, characterClass))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

void Client::CAnimation_Tool::Render_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	ImGui::SeparatorText("Key -> Skill Animation");
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		ImGui::TextDisabled(
			"Select Scene Character to author gameplay key bindings."
			" Reference-only preview assets are not playable classes.");
		return;
	}

	const CHARACTER_SPEC* spec = pCharacter->Get_Spec();
	if (!m_bSkillBindingLoadAttempted)
	{
		m_bSkillBindingLoadAttempted = true;
		Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_SkillBindingDocument.Bindings.empty())
	{
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());
		ImGui::TextWrapped(
			"The Character remains available even when this document is missing or "
			"invalid. Create a complete in-memory repair draft from the currently "
			"selected model clip, then assign each skill and Save.");
		if (ImGui::Button("Create Repair Draft from Current Clip"))
			Create_SkillBindingDraft(pModel, spec->eCharacterClass);
		return;
	}

	ImGui::TextWrapped(
		"PlayerSkills owns key -> skillId and Server timing. This panel saves only "
		"the approved skillId -> ordered presentation clips. BA1/BA2/... are "
		"indexed directly by the replicated comboStage.");
	if (ImGui::Button("Save Skill Bindings"))
		Save_SkillBindings(pModel, pCharacter);
	ImGui::SameLine();
	if (ImGui::Button("Reload Skill Bindings"))
	{
		if (m_bSkillBindingDirty)
			m_bSkillBindingReloadConfirmationRequested = true;
		else
			Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_bSkillBindingDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}
	Render_SkillBindingReloadConfirmation(pModel, spec->eCharacterClass);
	if (!m_SkillBindingStatus.empty())
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());

	const uint32_t currentAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* currentClip =
		pModel->Get_AnimationName(currentAnimationIndex);
	ImGui::TextDisabled(
		"Current clip: %s",
		nullptr != currentClip ? currentClip : "(none)");

	std::vector<const PLAYER_SKILL_DEFINITION*> classSkills;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass == spec->eCharacterClass)
			classSkills.push_back(&definition);
	}
	const auto slotRank = [](const std::string& slot)
	{
		constexpr const char_t* preferred[] =
		{
			"Q", "W", "E", "R", "A", "S", "D", "F",
			"T", "V", "ALT_V", "LMB"
		};
		for (int32_t index = 0; index < static_cast<int32_t>(std::size(preferred)); ++index)
		{
			if (slot == preferred[index])
				return index;
		}
		return static_cast<int32_t>(std::size(preferred));
	};
	std::sort(classSkills.begin(), classSkills.end(),
		[&](const PLAYER_SKILL_DEFINITION* left,
			const PLAYER_SKILL_DEFINITION* right)
		{
			const int32_t leftRank = slotRank(left->strInputSlot);
			const int32_t rightRank = slotRank(right->strInputSlot);
			return leftRank != rightRank ? leftRank < rightRank :
				left->strInputSlot < right->strInputSlot;
		});

	if (!ImGui::BeginChild(
		"##skillbindings",
		ImVec2(0.f, 300.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}
	for (const PLAYER_SKILL_DEFINITION* definition : classSkills)
	{
		const std::string slotLabel =
			"ALT_V" == definition->strInputSlot ? "ALT+V" :
			("LMB" == definition->strInputSlot ? "BA / LMB" :
				definition->strInputSlot);
		ANIMATION_SKILL_BINDING* binding =
			Find_SkillBinding(definition->iSkillId);
		if (nullptr == binding)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.35f, 1.f),
				"%-8s  %u missing authored binding",
				slotLabel.c_str(),
				definition->iSkillId);
			continue;
		}
		const int32_t bindingIndex = static_cast<int32_t>(
			binding - m_SkillBindingDocument.Bindings.data());
		ImGui::PushID(bindingIndex);
		const bool_t isCombo =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			definition->eSkillKind;
		char_t header[192]{};
		snprintf(
			header,
			sizeof(header),
			"%s  %u  %s  [%s]",
			slotLabel.c_str(),
			definition->iSkillId,
			definition->strDisplayName.c_str(),
			isCombo ? "COMBO" : "ACTIVE");
		if (ImGui::TreeNodeEx("##binding", ImGuiTreeNodeFlags_DefaultOpen, "%s", header))
		{
			for (int32_t clipIndex = 0;
				clipIndex < static_cast<int32_t>(binding->Clips.size());
				++clipIndex)
			{
				ImGui::PushID(clipIndex);
				char_t clipLabel[MAX_PATH + 32]{};
				snprintf(
					clipLabel,
					sizeof(clipLabel),
					isCombo ? "BA%d  %s" : "clip%d  %s",
					clipIndex + 1,
					binding->Clips[clipIndex].strClipName.c_str());
				const bool_t selected =
					m_iSelectedSkillBinding == bindingIndex &&
					m_iSelectedSkillClip == clipIndex;
				if (ImGui::Selectable(clipLabel, selected))
				{
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillClip = clipIndex;
					Select_Clip(pModel, binding->Clips[clipIndex].strClipName);
				}
				ImGui::PopID();
			}

			int32_t selectedClip =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillClip : 0;
			if (selectedClip < 0 ||
				selectedClip >= static_cast<int32_t>(binding->Clips.size()))
			{
				selectedClip = 0;
			}
			ImGui::BeginDisabled(nullptr == currentClip);
			if (ImGui::Button(isCombo ?
				"Assign Current Clip to Selected BA Stage" :
				"Assign Current Clip to Selected Step"))
			{
				binding->Clips[selectedClip].strClipName = currentClip;
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillClip = selectedClip;
				m_bSkillBindingDirty = true;
				m_SkillBindingStatus =
					"Assigned " + std::string(currentClip) + " to " +
					slotLabel + (isCombo ?
						(" BA" + std::to_string(selectedClip + 1)) :
						(" clip" + std::to_string(selectedClip + 1)));
			}
			if (!isCombo)
			{
				ImGui::SameLine();
				if (ImGui::Button("Add Current Step") &&
					binding->Clips.size() < 16u)
				{
					binding->Clips.insert(
						binding->Clips.begin() + selectedClip + 1,
						ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillClip = selectedClip + 1;
					m_bSkillBindingDirty = true;
				}
			}
			ImGui::EndDisabled();

			if (!isCombo && binding->Clips.size() > 1u)
			{
				if (ImGui::SmallButton("Up") && selectedClip > 0)
				{
					std::swap(
						binding->Clips[selectedClip],
						binding->Clips[selectedClip - 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillClip = selectedClip - 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Down") &&
					selectedClip + 1 < static_cast<int32_t>(binding->Clips.size()))
				{
					std::swap(
						binding->Clips[selectedClip],
						binding->Clips[selectedClip + 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillClip = selectedClip + 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove Step"))
				{
					binding->Clips.erase(binding->Clips.begin() + selectedClip);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillClip = std::min(
						selectedClip,
						static_cast<int32_t>(binding->Clips.size()) - 1);
					m_bSkillBindingDirty = true;
				}
			}
			else if (isCombo)
			{
				ImGui::TextDisabled(
					"BA stage count is fixed by PlayerSkills comboStages (%zu).",
					definition->iComboStageCount);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
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
		if (EVENT_KIND::EFFECT == eKind)
		{
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
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
		Save_Events(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		if (m_bDirty)
			m_bReloadConfirmationRequested = true;
		else
			Load_Events(pModel);
	}
	Render_ReloadConfirmation(pModel);

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
		const bool_t bEffectCueWindow =
			EVENT_KIND::EFFECT == evt.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				evt.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == evt.eStopPolicy;

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
			if (bWindow || bEffectCueWindow)
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
				if (EVENT_KIND::EFFECT == evt.eKind)
				{
					const bool_t bAsset =
						EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
							evt.eEffectReferenceKind;
					if (ImGui::RadioButton("Source Reference", !bAsset))
					{
						evt.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
						m_bDirty = true;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Admitted EffectAssetId", bAsset))
					{
						evt.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
						const vector<string> IDs =
							CEffectCatalog::Get_EffectAssetIds();
						if (!IDs.empty() &&
							!CEffectCatalog::Contains(evt.sPayload))
						{
							evt.sPayload = IDs.front();
							strncpy_s(m_PayloadEdit,
								evt.sPayload.c_str(), _TRUNCATE);
						}
						m_bDirty = true;
					}
					if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
						evt.eEffectReferenceKind)
					{
						const vector<string> IDs =
							CEffectCatalog::Get_EffectAssetIds();
						if (ImGui::BeginCombo("Effect Asset",
							evt.sPayload.empty() ? "Select admitted Effect" :
								evt.sPayload.c_str()))
						{
							for (const string& ID : IDs)
							{
								if (ImGui::Selectable(ID.c_str(),
									ID == evt.sPayload))
								{
									evt.sPayload = ID;
									strncpy_s(m_PayloadEdit,
										ID.c_str(), _TRUNCATE);
									m_bDirty = true;
								}
							}
							ImGui::EndCombo();
						}
						char_t anchor[129]{};
						strncpy_s(anchor, evt.sAnchorSlotId.c_str(), _TRUNCATE);
						if (ImGui::InputText("Anchor (root or bone)",
							anchor, sizeof(anchor)))
						{
							evt.sAnchorSlotId = anchor;
							m_bDirty = true;
						}
						int32_t follow = ETOI(evt.eFollowPolicy);
						if (ImGui::RadioButton("Follow", 0 == follow))
						{
							evt.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton("Snapshot", 1 == follow))
						{
							evt.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
							m_bDirty = true;
						}
						int32_t stop = ETOI(evt.eStopPolicy);
						if (ImGui::RadioButton("Natural Stop", 0 == stop))
						{
							evt.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
							evt.iEndMs = evt.iStartMs;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton("Cue End Stop", 1 == stop))
						{
							evt.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
							if (evt.iEndMs <= evt.iStartMs)
								evt.iEndMs = evt.iStartMs + 100;
							m_bDirty = true;
						}
						m_bDirty |= ImGui::InputFloat3("Effect Local Position",
							&evt.EffectLocalTransform.vPosition.x, "%.3f");
						m_bDirty |= ImGui::InputFloat3("Effect Local Rotation",
							&evt.EffectLocalTransform.vRotationDegrees.x, "%.3f");
						m_bDirty |= ImGui::InputFloat3("Effect Local Scale",
							&evt.EffectLocalTransform.vScale.x, "%.3f");
					}
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

bool_t Client::CAnimation_Tool::Write_EventsToPath(
	const std::filesystem::path& path,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open temporary Animation Events file.";
		return false;
	}

	bool_t writeSucceeded = 0 <= fprintf(
		file,
		"%s %d \"%s\" %d\n",
		EVENT_FILE_MAGIC,
		EVENT_FILE_VERSION,
		m_AssetName.c_str(),
		static_cast<int32_t>(events.size()));

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		const char_t* kindName = Kind_Name(event.eKind);
		if (EVENT_KIND::HIT == event.eKind)
		{
			const HIT_PARAMS& hit = event.hit;
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d rep=%d repms=%d "
				"fz=%d fzin=%d fzout=%d push=%d pushr=%d "
				"area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs,
				hit.iRepeatCount, hit.iRepeatMs,
				hit.iFreezeMs, hit.iFreezeInMs, hit.iFreezeOutMs,
				hit.iPushMs, hit.iPushRange,
				hit.iAreaType, hit.iAreaRange, hit.iAreaAngle,
				hit.iAreaHeight, hit.iAreaOffsetX, hit.iAreaInner,
				hit.iMaxTargets,
				event.bImported ? " src=orig" : "");
		}
		else if (Is_Window(event.eKind))
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
		else if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d endms=%d payload=\"%s\" "
					"effectref=asset anchor=\"%s\" follow=%s stop=%s "
					"px=%.9g py=%.9g pz=%.9g rx=%.9g ry=%.9g rz=%.9g "
					"sx=%.9g sy=%.9g sz=%.9g%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.iEndMs,
					event.sPayload.c_str(), event.sAnchorSlotId.c_str(),
					EFFECT_FOLLOW_POLICY::FOLLOW == event.eFollowPolicy ?
						"follow" : "snapshot",
					EFFECT_STOP_POLICY::NATURAL == event.eStopPolicy ?
						"natural" : "cue_end",
					event.EffectLocalTransform.vPosition.x,
					event.EffectLocalTransform.vPosition.y,
					event.EffectLocalTransform.vPosition.z,
					event.EffectLocalTransform.vRotationDegrees.x,
					event.EffectLocalTransform.vRotationDegrees.y,
					event.EffectLocalTransform.vRotationDegrees.z,
					event.EffectLocalTransform.vScale.x,
					event.EffectLocalTransform.vScale.y,
					event.EffectLocalTransform.vScale.z,
					event.bImported ? " src=orig" : "");
			}
			else
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d payload=\"%s\" "
					"effectref=source%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.sPayload.c_str(),
					event.bImported ? " src=orig" : "");
			}
		}
		else
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
	}

	const bool_t flushSucceeded = 0 == fflush(file);
	const bool_t commitSucceeded =
		flushSucceeded && 0 == _commit(_fileno(file));
	const bool_t closeSucceeded = 0 == fclose(file);
	if (!writeSucceeded || !flushSucceeded ||
		!commitSucceeded || !closeSucceeded)
	{
		outStatus = "Could not flush Animation Events temporary file.";
		return false;
	}

	return true;
}

bool_t Client::CAnimation_Tool::Validate_Events(
	const shared_ptr<Engine::CModel>& pModel,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();
	if (nullptr == pModel || m_AssetName.empty() ||
		events.size() > static_cast<size_t>(MAX_EVENT_COUNT))
	{
		outStatus = "Animation event document header is invalid.";
		return false;
	}

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		bool_t hasClip = false;
		for (uint32_t index = 0;
			index < pModel->Get_NumAnimations();
			++index)
		{
			const char_t* clipName =
				pModel->Get_AnimationName(index);
			if (nullptr != clipName && event.clipName == clipName)
			{
				hasClip = true;
				break;
			}
		}

		if (!hasClip ||
			!Is_SafeQuotedText(event.clipName) ||
			!Is_SafeQuotedText(event.sPayload) ||
			event.iStartMs < 0 ||
			event.iEndMs < event.iStartMs ||
			(!Is_Window(event.eKind) &&
				event.iEndMs != event.iStartMs &&
				!(bAdmittedEffect &&
					EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy)) ||
			ETOI(event.eKind) < 0 ||
			ETOI(event.eKind) >= ETOI(EVENT_KIND::END))
		{
			outStatus =
				"Animation event clip, time, kind, or payload is invalid.";
			return false;
		}

		if (EVENT_KIND::HIT == event.eKind &&
			(event.hit.iRepeatCount < 1 ||
				event.hit.iRepeatCount > 1024 ||
				event.hit.iRepeatMs < 0))
		{
			outStatus = "Animation HIT repeat contract is invalid.";
			return false;
		}

		if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::NONE ==
				event.eEffectReferenceKind)
			{
				outStatus =
					"Animation EFFECT row has no reference kind.";
				return false;
			}
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				const EFFECT_TRANSFORM_DESC& local =
					event.EffectLocalTransform;
				if (!CEffectCatalog::Contains(event.sPayload) ||
					event.sAnchorSlotId.empty() ||
					!Is_SafeQuotedText(event.sAnchorSlotId) ||
					EFFECT_FOLLOW_POLICY::END == event.eFollowPolicy ||
					EFFECT_STOP_POLICY::END == event.eStopPolicy ||
					!std::isfinite(local.vPosition.x) ||
					!std::isfinite(local.vPosition.y) ||
					!std::isfinite(local.vPosition.z) ||
					!std::isfinite(local.vRotationDegrees.x) ||
					!std::isfinite(local.vRotationDegrees.y) ||
					!std::isfinite(local.vRotationDegrees.z) ||
					!std::isfinite(local.vScale.x) ||
					!std::isfinite(local.vScale.y) ||
					!std::isfinite(local.vScale.z) ||
					local.vScale.x <= 0.f || local.vScale.y <= 0.f ||
					local.vScale.z <= 0.f ||
					(EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
						event.iEndMs <= event.iStartMs))
				{
					outStatus =
						"Admitted Effect cue ID, anchor, policy, or local transform is invalid.";
					return false;
				}
			}
		}
		else if (EFFECT_REFERENCE_KIND::NONE !=
			event.eEffectReferenceKind)
		{
			outStatus =
				"Only Animation EFFECT rows may carry an effect reference.";
			return false;
		}
	}

	return true;
}

bool_t Client::CAnimation_Tool::Events_AreEqual(
	const std::vector<ANIM_EVENT>& left,
	const std::vector<ANIM_EVENT>& right) const
{
	if (left.size() != right.size())
		return false;

	const auto hitsAreEqual = [](const HIT_PARAMS& a, const HIT_PARAMS& b)
	{
		return a.iRepeatCount == b.iRepeatCount &&
			a.iRepeatMs == b.iRepeatMs &&
			a.iFreezeMs == b.iFreezeMs &&
			a.iFreezeInMs == b.iFreezeInMs &&
			a.iFreezeOutMs == b.iFreezeOutMs &&
			a.iPushMs == b.iPushMs &&
			a.iPushRange == b.iPushRange &&
			a.iAreaType == b.iAreaType &&
			a.iAreaRange == b.iAreaRange &&
			a.iAreaAngle == b.iAreaAngle &&
			a.iAreaHeight == b.iAreaHeight &&
			a.iAreaOffsetX == b.iAreaOffsetX &&
			a.iAreaInner == b.iAreaInner &&
			a.iMaxTargets == b.iMaxTargets;
	};
	const auto effectTransformsAreEqual = [](
		const EFFECT_TRANSFORM_DESC& a,
		const EFFECT_TRANSFORM_DESC& b)
	{
		return a.vPosition.x == b.vPosition.x &&
			a.vPosition.y == b.vPosition.y &&
			a.vPosition.z == b.vPosition.z &&
			a.vRotationDegrees.x == b.vRotationDegrees.x &&
			a.vRotationDegrees.y == b.vRotationDegrees.y &&
			a.vRotationDegrees.z == b.vRotationDegrees.z &&
			a.vScale.x == b.vScale.x &&
			a.vScale.y == b.vScale.y &&
			a.vScale.z == b.vScale.z;
	};

	for (size_t index = 0; index < left.size(); ++index)
	{
		const ANIM_EVENT& a = left[index];
		const ANIM_EVENT& b = right[index];
		if (a.clipName != b.clipName ||
			a.eKind != b.eKind ||
			a.iStartMs != b.iStartMs ||
			a.iEndMs != b.iEndMs ||
			a.sPayload != b.sPayload ||
			a.eEffectReferenceKind != b.eEffectReferenceKind ||
			a.sAnchorSlotId != b.sAnchorSlotId ||
			a.eFollowPolicy != b.eFollowPolicy ||
			a.eStopPolicy != b.eStopPolicy ||
			!effectTransformsAreEqual(a.EffectLocalTransform,
				b.EffectLocalTransform) ||
			a.bImported != b.bImported ||
			!hitsAreEqual(a.hit, b.hit))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CAnimation_Tool::Load_EventsFromPath(
	const std::filesystem::path& path,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<ANIM_EVENT>& outEvents,
	int32_t& outSourceVersion,
	std::string& outStatus) const
{
	outEvents.clear();
	outSourceVersion = {};
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"rb") || nullptr == file)
	{
		outStatus = "No event file: " + path.string();
		return false;
	}

	int32_t lineNumber = 1;
	const auto fail = [&](
		const std::string& message) -> bool_t
	{
		fclose(file);
		outStatus = message + " [line " +
			std::to_string(lineNumber) + "]";
		return false;
	};

	char_t line[4096]{};
	if (nullptr == fgets(line, sizeof(line), file))
		return fail("Animation event file is empty.");

	const char_t* cursor = line;
	std::string magic;
	std::string versionToken;
	std::string owner;
	std::string countToken;
	int32_t version = {};
	int32_t declaredCount = {};
	if (!Read_Token(cursor, magic) ||
		!Read_Token(cursor, versionToken) ||
		!Read_Quoted(cursor, owner) ||
		!Read_Token(cursor, countToken) ||
		Has_RemainingToken(cursor) ||
		EVENT_FILE_MAGIC != magic ||
		!Parse_Integer(versionToken, version) ||
		!Parse_Integer(countToken, declaredCount) ||
		version < 1 || version > EVENT_FILE_VERSION ||
		declaredCount < 0 || declaredCount > MAX_EVENT_COUNT ||
		owner != m_AssetName)
	{
		return fail("Animation event header is invalid.");
	}

	std::vector<ANIM_EVENT> staged;
	staged.reserve(static_cast<size_t>(declaredCount));

	while (nullptr != fgets(line, sizeof(line), file))
	{
		++lineNumber;
		if (staged.size() >= static_cast<size_t>(declaredCount))
			return fail("Animation event row count exceeds the header.");

		const size_t lineLength = strlen(line);
		if (lineLength == sizeof(line) - 1u &&
			'\n' != line[lineLength - 1u] && !feof(file))
		{
			return fail("Animation event row is too long.");
		}

		cursor = line;
		std::string clipName;
		std::string kindName;
		if (!Read_Quoted(cursor, clipName) ||
			!Read_Token(cursor, kindName))
		{
			return fail("Malformed Animation event row.");
		}

		ANIM_EVENT event{};
		event.clipName = clipName;
		if ("HIT" == kindName)
			event.eKind = EVENT_KIND::HIT;
		else if ("CANCEL" == kindName)
			event.eKind = EVENT_KIND::CANCEL;
		else if ("SUPERARMOR" == kindName)
			event.eKind = EVENT_KIND::SUPERARMOR;
		else if ("INVULN" == kindName)
			event.eKind = EVENT_KIND::INVULN;
		else if ("MOVE" == kindName)
			event.eKind = EVENT_KIND::MOVE;
		else if ("SOUND" == kindName)
			event.eKind = EVENT_KIND::SOUND;
		else if ("EFFECT" == kindName)
		{
			event.eKind = EVENT_KIND::EFFECT;
			if (version < 4)
			{
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			}
		}
		else if ("SHAKE" == kindName)
			event.eKind = EVENT_KIND::SHAKE;
		else
			return fail("Unknown Animation event kind.");

		int32_t startFrame = {};
		int32_t endFrame = {};
		int32_t intervalFrame = {};
		bool_t hasStartFrame = false;
		bool_t hasEndFrame = false;
		bool_t hasStartMs = false;
		bool_t hasEndMs = false;
		bool_t hasEffectReference = false;

		if (1 == version)
		{
			std::string startToken;
			std::string endToken;
			if (EVENT_KIND::HIT != event.eKind ||
				!Read_Token(cursor, startToken) ||
				!Read_Token(cursor, endToken) ||
				Has_RemainingToken(cursor) ||
				!Parse_Integer(startToken, startFrame) ||
				!Parse_Integer(endToken, endFrame))
			{
				return fail("Invalid v1 Animation HIT row.");
			}
			hasStartFrame = true;
			hasEndFrame = true;
		}
		else
		{
			std::unordered_set<std::string> keys;
			while (Has_RemainingToken(cursor))
			{
				std::string key;
				std::string value;
				if (!Read_Pair(cursor, key, value) ||
					!keys.insert(key).second)
				{
					return fail(
						"Malformed or duplicate Animation event field.");
				}

				if ("payload" == key)
				{
					event.sPayload = value;
					continue;
				}
				if ("src" == key)
				{
					if ("orig" != value)
						return fail("Unknown Animation event source.");
					event.bImported = true;
					continue;
				}
				if ("effectref" == key)
				{
					if (version < 4 ||
						EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"effectref is only valid for v4/v5 EFFECT rows.");
					}
					if ("source" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
					}
					else if ("asset-id" == value || "asset" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
					}
					else
						return fail("Unknown Animation effect reference kind.");
					hasEffectReference = true;
					continue;
				}
				if ("anchor" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("anchor is only valid for v5 EFFECT rows.");
					event.sAnchorSlotId = value;
					continue;
				}
				if ("follow" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("follow is only valid for v5 EFFECT rows.");
					if ("follow" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
					else if ("snapshot" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
					else
						return fail("Unknown Effect follow policy.");
					continue;
				}
				if ("stop" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("stop is only valid for v5 EFFECT rows.");
					if ("natural" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					else if ("cue_end" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
					else
						return fail("Unknown Effect stop policy.");
					continue;
				}
				if (key == "px" || key == "py" || key == "pz" ||
					key == "rx" || key == "ry" || key == "rz" ||
					key == "sx" || key == "sy" || key == "sz")
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("Effect local transform is only valid in v5.");
					f32_t number = 0.f;
					if (!Parse_Float(value, number))
						return fail("Animation Effect transform number is invalid.");
					if ("px" == key) event.EffectLocalTransform.vPosition.x = number;
					else if ("py" == key) event.EffectLocalTransform.vPosition.y = number;
					else if ("pz" == key) event.EffectLocalTransform.vPosition.z = number;
					else if ("rx" == key) event.EffectLocalTransform.vRotationDegrees.x = number;
					else if ("ry" == key) event.EffectLocalTransform.vRotationDegrees.y = number;
					else if ("rz" == key) event.EffectLocalTransform.vRotationDegrees.z = number;
					else if ("sx" == key) event.EffectLocalTransform.vScale.x = number;
					else if ("sy" == key) event.EffectLocalTransform.vScale.y = number;
					else if ("sz" == key) event.EffectLocalTransform.vScale.z = number;
					continue;
				}

				int32_t number = {};
				if (!Parse_Integer(value, number))
					return fail("Animation event number is invalid.");

				if ("start" == key)
				{
					startFrame = number;
					hasStartFrame = true;
				}
				else if ("end" == key)
				{
					endFrame = number;
					hasEndFrame = true;
				}
				else if ("count" == key)
					event.hit.iRepeatCount = number;
				else if ("interval" == key)
					intervalFrame = number;
				else if ("startms" == key)
				{
					event.iStartMs = number;
					hasStartMs = true;
				}
				else if ("endms" == key)
				{
					event.iEndMs = number;
					hasEndMs = true;
				}
				else if ("rep" == key)
					event.hit.iRepeatCount = number;
				else if ("repms" == key)
					event.hit.iRepeatMs = number;
				else if ("fz" == key)
					event.hit.iFreezeMs = number;
				else if ("fzin" == key)
					event.hit.iFreezeInMs = number;
				else if ("fzout" == key)
					event.hit.iFreezeOutMs = number;
				else if ("push" == key)
					event.hit.iPushMs = number;
				else if ("pushr" == key)
					event.hit.iPushRange = number;
				else if ("area" == key)
					event.hit.iAreaType = number;
				else if ("ar" == key)
					event.hit.iAreaRange = number;
				else if ("aa" == key)
					event.hit.iAreaAngle = number;
				else if ("ah" == key)
					event.hit.iAreaHeight = number;
				else if ("ax" == key)
					event.hit.iAreaOffsetX = number;
				else if ("arem" == key)
					event.hit.iAreaInner = number;
				else if ("maxt" == key)
					event.hit.iMaxTargets = number;
				else
					return fail("Unknown Animation event field.");
			}
		}

		if (2 == version)
		{
			if (!hasStartFrame ||
				(Is_Window(event.eKind) && !hasEndFrame))
			{
				return fail("v2 Animation event time is missing.");
			}
			if (!hasEndFrame)
				endFrame = startFrame;
		}
		else if (version >= 3)
		{
			if (!hasStartMs ||
				(Is_Window(event.eKind) && !hasEndMs))
			{
				return fail("Animation event millisecond time is missing.");
			}
			if (!hasEndMs)
				event.iEndMs = event.iStartMs;
		}

		if (version < 3)
		{
			const f32_t tickRate =
				Get_ClipTickRate(pModel, event.clipName);
			if (2 == version &&
				intervalFrame > 0 &&
				event.hit.iRepeatCount <= 1)
			{
				event.hit.iRepeatCount =
					(endFrame - startFrame) / intervalFrame + 1;
			}
			if (event.hit.iRepeatCount > 1)
			{
				event.hit.iRepeatMs = intervalFrame > 0 ?
					Frame_To_Ms(intervalFrame, tickRate) :
					Frame_To_Ms(
						endFrame - startFrame,
						tickRate) /
						(event.hit.iRepeatCount - 1);
				endFrame = startFrame;
			}
			event.iStartMs = Frame_To_Ms(startFrame, tickRate);
			event.iEndMs = Frame_To_Ms(endFrame, tickRate);
		}

		if (event.hit.iRepeatCount < 1)
			event.hit.iRepeatCount = 1;
		if (!Is_Window(event.eKind) &&
			!(EVENT_KIND::EFFECT == event.eKind &&
				EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
					event.eEffectReferenceKind &&
				EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy))
			event.iEndMs = event.iStartMs;
		if (3 == version && EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			event.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
		if (version >= 4 &&
			EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			if (event.bImported)
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			else
				return fail("Authored v4/v5 EFFECT row requires effectref.");
		}
		if (EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
			(!hasEndMs || event.iEndMs <= event.iStartMs))
		{
			return fail("cue_end requires endms greater than startms.");
		}

		staged.push_back(std::move(event));
	}

	if (0 != ferror(file))
		return fail("Animation event file read failed.");
	if (0 != fclose(file))
	{
		outStatus = "Animation event file close failed.";
		return false;
	}
	file = nullptr;

	if (staged.size() != static_cast<size_t>(declaredCount))
	{
		outStatus = "Animation event row count does not match the header.";
		return false;
	}

	outEvents = std::move(staged);
	outSourceVersion = version;
	return true;
}

bool_t Client::CAnimation_Tool::Save_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	std::string validationStatus;
	if (!Validate_Events(pModel, m_Events, validationStatus))
	{
		m_Status = "Save rejected: " + validationStatus;
		return false;
	}

	const std::filesystem::path destination{ Get_EventFilePath() };
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Save failed to create authoring directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);
	if (removeError)
	{
		m_Status = "Save failed to clear stale temporary file: " +
			removeError.message();
		return false;
	}

	std::string transactionStatus;
	if (!Write_EventsToPath(
		temporary, m_Events, transactionStatus))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Save failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	if (!Load_EventsFromPath(
		temporary,
		pModel,
		staged,
		sourceVersion,
		transactionStatus) ||
		EVENT_FILE_VERSION != sourceVersion ||
		!Validate_Events(pModel, staged, transactionStatus) ||
		!Events_AreEqual(staged, m_Events))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		if (transactionStatus.empty())
		{
			transactionStatus =
				"Animation event round trip changed the document.";
		}
		m_Status = "Save validation failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	if (!MoveFileExW(
		temporary.c_str(),
		destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD errorCode = GetLastError();
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Atomic replace failed; previous file preserved. Win32 error " +
			std::to_string(errorCode) + ".";
		return false;
	}

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(m_Events.size()) +
		" event(s) atomically to " + destination.string();
	return true;
}

bool_t Client::CAnimation_Tool::Load_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	const std::filesystem::path path{ Get_EventFilePath() };
	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	std::string loadStatus;
	if (!Load_EventsFromPath(
		path, pModel, staged, sourceVersion, loadStatus) ||
		!Validate_Events(pModel, staged, loadStatus))
	{
		m_Status =
			"Load rejected; current document preserved: " + loadStatus;
		return false;
	}

	m_Events = std::move(staged);
	m_iSelectedEvent = -1;
	/* v3 already stores milliseconds and can be represented by v4 without moving
	any marker. Loading it must not create a false unsaved document that blocks a
	Character Select target change. Only v1/v2 frame conversion is dirty. */
	m_bDirty = sourceVersion < 3;
	m_Status = "Loaded " + std::to_string(m_Events.size()) +
		" event(s) from " + path.string();
	if (m_bDirty)
	{
		m_Status += " [converted from v" +
			std::to_string(sourceVersion) +
			" in memory; review and Save explicitly]";
	}
	else if (sourceVersion < EVENT_FILE_VERSION)
	{
		m_Status +=
			" [legacy source references preserved; the next authored Save writes v5]";
	}
	return true;
}

void Client::CAnimation_Tool::Render_ReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (m_bReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Animation Events?");
		m_bReloadConfirmationRequested = false;
	}

	if (!ImGui::BeginPopupModal(
		"Discard unsaved Animation Events?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::TextUnformatted(
		"Reload will replace the current unsaved Animation document.");
	if (ImGui::Button("Discard and Reload"))
	{
		if (Load_Events(pModel))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
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
		if (EVENT_KIND::EFFECT == evt.eKind)
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
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
