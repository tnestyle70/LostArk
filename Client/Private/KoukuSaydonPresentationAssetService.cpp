#include "KoukuSaydonPresentationAssetService.h"
#include "KoukuSaydonAnimationBlend.h"
#include "KoukuSaydonCompositionDocument.h"

#include "ActorCatalog.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NetworkManager.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>
#include <functional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	using namespace Client;

	constexpr std::string_view KOUKU_ARCHETYPE =
		"BOSS_KAKULSAYDON_G1_KOUKU";
	constexpr std::string_view KOUKU_PRESENTATION =
		"boss.kakulsaydon.g1.kouku.client.v1";
	/* Every arena boss archetype shares this prefix and this client contract
	family; the Gate 1 Kouku keeps its exact model prototype tag. */
	constexpr std::string_view KOUKU_FAMILY_ARCHETYPE_PREFIX =
		"BOSS_KAKULSAYDON_";
	constexpr std::string_view KOUKU_FAMILY_PRESENTATION_PREFIX =
		"boss.kakulsaydon.";
	constexpr std::string_view KOUKU_FAMILY_PRESENTATION_SUFFIX =
		".client.v1";
	constexpr const wchar_t* KOUKU_MODEL_PROTOTYPE_PREFIX =
		L"Prototype_Component_Model_KoukuSaydon_";
	constexpr const wchar_t* KOUKU_WEAPON_PROTOTYPE_SUFFIX = L"_Weapon";
	/* Both Saydon bodies (MN_RPCT_05/06) socket their held weapon here. */
	constexpr const char_t* KOUKU_WEAPON_SOCKET_BONE = "b_wp_1";
	constexpr std::string_view BINDING_SCHEMA =
		"lostark.kouku-saydon-pattern-bindings";
	constexpr std::uint32_t BINDING_VERSION = 1u;
	constexpr std::uintmax_t MAX_BINDING_BYTES = 8u * 1024u * 1024u;
	constexpr const wchar_t* KOUKU_OBJECT_PROTOTYPE =
		L"Prototype_GameObject_KoukuSaydonPresentation";

	std::mutex g_KoukuAssetMutex;
	std::unordered_map<std::uint32_t, std::unordered_set<std::string>>
		g_ReadyByLevel;
	/* archetypeId -> actionId -> admitted clip row. Keyed per body because the
	same Product document serves every arena boss and each body owns only the
	rows whose clip exists on its rig. */
	std::unordered_map<std::string,
		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION>>
		g_ActionPresentationsByArchetype;
	using ATTACHMENT_GRIPS = std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>;
	std::unordered_map<std::string, ATTACHMENT_GRIPS> g_AttachmentGripsByArchetype;
	std::unordered_map<std::string, std::uint32_t> g_BindingSourceRevisions;
    // The same immutable source backs every model-filtered canonical cache.
    // Admission compares bytes once, then keeps the prepared clip rows in place.
    std::unordered_map<std::string, std::shared_ptr<const std::string>> g_CanonicalBindingSources;
    std::shared_ptr<const std::string> g_LastCanonicalBindingSource;
    struct BINDING_PREPARATION
    {
        std::uint32_t levelIndex = 0u, sourceRevision = 0u;
        std::shared_ptr<const std::string> source;
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft;
        std::vector<std::string> archetypes;
        std::size_t nextArchetype = 0u;
        bool reusesActive = false;
        decltype(g_ActionPresentationsByArchetype) actions;
        decltype(g_AttachmentGripsByArchetype) grips;
        decltype(g_BindingSourceRevisions) revisions;
        decltype(g_CanonicalBindingSources) sources;
    };
    std::unique_ptr<BINDING_PREPARATION> g_BindingPreparation;
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> g_PreparedDraft, g_AdmittedDraft;
    std::uint32_t g_AdmittedRunEpoch = 0u, g_AdmittedSourceRevision = 0u, g_PreparedAuthorizedEpoch = 0u;
    bool g_RunAdmissionFailed = false;
	std::string g_Status = "KoukuSaydon presentation has not been loaded.";

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		return std::all_of(names.begin(), names.end(),
			[&object](const std::string_view name)
			{
				return nullptr != object.Find(name);
			});
	}

	bool Has_BindingDocumentProperties(const DATA_JSON_VALUE& root)
	{
		const std::initializer_list<std::string_view> required =
			{"schema", "formatVersion", "bossArchetypeId", "sourceRevision", "bindings"};
		const std::initializer_list<std::string_view> optional =
			{"patterns", "lightResourceRevision", "folders", "bundles", "fearPresentations", "attachmentGrips", "targetedCombatVisuals"};
		if (!root.Is_Object()) return false;
		for (const auto key : required) if (!root.Find(key)) return false;
		for (const auto& [key, value] : root.Get_Object())
			if (std::find(required.begin(), required.end(), key) == required.end() &&
				std::find(optional.begin(), optional.end(), key) == optional.end()) return false;
		return true;
	}

	bool Is_StableToken(const std::string_view value)
	{
		if (value.empty() || value.size() > 255u || value == "." || value == "..")
			return false;
		return std::all_of(value.begin(), value.end(), [](const unsigned char c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
		});
	}

	bool Try_U32(
		const DATA_JSON_VALUE& value,
		const std::uint32_t maximum,
		std::uint32_t& out)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) || std::floor(number) != number)
		{
			return false;
		}
		out = static_cast<std::uint32_t>(number);
		return true;
	}

	bool Has_Clip(const Engine::CModel& model, const std::string_view clip)
	{
		for (std::uint32_t index = 0u; index < model.Get_NumAnimations(); ++index)
		{
			const char* name = model.Get_AnimationName(index);
			if (nullptr != name && clip == name)
				return true;
		}
		return false;
	}

    bool Try_GetClipDurationMs(const Engine::CModel& model, const std::string_view clip,
        double& outDurationMs)
    {
        for (uint32_t index = 0; index < model.Get_NumAnimations(); ++index)
        {
            const char* name = model.Get_AnimationName(index);
            if (!name || clip != name) continue;
            float cursor = 0.f, duration = 0.f;
            const float ticksPerSecond = model.Get_AnimationTickPerSecond(index);
            if (!model.Get_AnimationProgress(index, cursor, duration) ||
                !std::isfinite(duration) || duration <= 0.f ||
                !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f) return false;
            outDurationMs = double(duration) * 1000.0 / ticksPerSecond;
            return true;
        }
        return false;
    }

    bool Read_CanonicalBindingSource(std::shared_ptr<const std::string>& out, std::string& status)
    {
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path(L"Animation/Authored/KoukuSaydon") / L"KoukuSaydon.patternbindings.json");
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (path.empty() || error || !bytes || bytes > MAX_BINDING_BYTES)
        { status = "KoukuSaydon Product animation binding is missing or oversized."; return false; }
        const auto before = std::filesystem::last_write_time(path, error);
        if (error) { status = "KoukuSaydon Product animation binding timestamp is unavailable."; return false; }
        std::ifstream input(path, std::ios::binary);
        std::string text(static_cast<std::size_t>(bytes), '\0');
        input.read(text.data(), static_cast<std::streamsize>(bytes));
        if (!input || input.peek() != std::char_traits<char>::eof())
        { status = "KoukuSaydon Product animation binding changed during read."; return false; }
        const auto after = std::filesystem::last_write_time(path, error);
        if (error || before != after)
        { status = "KoukuSaydon Product animation binding changed during read."; return false; }
        // Equality, rather than timestamp/size alone, rejects edited contents even
        // when an authoring save preserves those two metadata fields.
        if (!g_LastCanonicalBindingSource || *g_LastCanonicalBindingSource != text)
            g_LastCanonicalBindingSource = std::make_shared<const std::string>(std::move(text));
        out = g_LastCanonicalBindingSource;
        return true;
    }

    std::vector<std::string> Ready_CanonicalBindingArchetypes(const std::uint32_t levelIndex)
    {
        std::vector<std::string> result;
        const auto ready = g_ReadyByLevel.find(levelIndex);
        if (ready != g_ReadyByLevel.end())
            for (const auto& archetype : ready->second)
                if (archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) result.push_back(archetype);
        std::sort(result.begin(), result.end());
        return result;
    }

    bool Have_PreparedCanonicalBindings(const std::uint32_t levelIndex,
        const std::uint32_t sourceRevision, const std::string& source)
    {
        const auto ready = g_ReadyByLevel.find(levelIndex);
        if (!sourceRevision || ready == g_ReadyByLevel.end()) return false;
        bool any = false;
        for (const auto& archetype : ready->second)
        {
            if (!archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) continue;
            const auto revision = g_BindingSourceRevisions.find(archetype);
            const auto bytes = g_CanonicalBindingSources.find(archetype);
            if (revision == g_BindingSourceRevisions.end() || revision->second != sourceRevision ||
                bytes == g_CanonicalBindingSources.end() || !bytes->second || *bytes->second != source ||
                !g_ActionPresentationsByArchetype.contains(archetype) || !g_AttachmentGripsByArchetype.contains(archetype))
                return false;
            any = true;
        }
        return any;
    }

	bool Load_PresentationBindings(
		const Engine::CModel& model,
		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION>& out,
		std::string& outStatus, std::uint32_t& outRevision, ATTACHMENT_GRIPS& outGrips,
		const std::uint32_t expectedRevision = 0u, const std::string* supplied = nullptr, const bool canonical = false,
        std::shared_ptr<const std::string>* outCanonicalSource = nullptr)
	{
        if (!supplied && !canonical && g_AdmittedDraft) supplied = &g_AdmittedDraft->PresentationJson;
        std::shared_ptr<const std::string> canonicalSource;
        std::string_view text;
        if (supplied)
        {
            if (supplied->empty() || supplied->size() > MAX_BINDING_BYTES)
            { outStatus = "Draft animation binding is empty or oversized."; return false; }
            text = *supplied;
        }
        else
        {
            if (!Read_CanonicalBindingSource(canonicalSource, outStatus)) return false;
            text = *canonicalSource;
        }

		DATA_JSON_VALUE root;
		std::string parseError;
		if (!CDataJson::Parse(text, root, parseError) || !Has_BindingDocumentProperties(root))
		{
			outStatus = "KoukuSaydon Product animation binding is malformed: " +
				parseError;
			return false;
		}

		const DATA_JSON_VALUE* schema = Required(
			root, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* version = Required(
			root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* archetype = Required(
			root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceRevision = Required(
			root, "sourceRevision", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* bindings = Required(
			root, "bindings", DATA_JSON_TYPE::ARRAY);
		// Animation and presentation readers consume the same Product document.
		// Its optional light pin must not reject all otherwise valid animations.
		const DATA_JSON_VALUE* lightRevision = root.Find("lightResourceRevision");
		std::uint32_t parsedVersion = 0u;
		std::uint32_t parsedRevision = 0u;
		std::uint32_t parsedLightRevision = 0u;
		if (nullptr == schema || schema->Get_String() != BINDING_SCHEMA ||
			nullptr == version ||
			!Try_U32(*version, BINDING_VERSION, parsedVersion) ||
			parsedVersion != BINDING_VERSION || nullptr == archetype ||
			archetype->Get_String() != KOUKU_ARCHETYPE ||
			nullptr == sourceRevision ||
			!Try_U32(*sourceRevision,
				(std::numeric_limits<std::uint32_t>::max)(), parsedRevision) ||
			0u == parsedRevision ||
			(expectedRevision != 0u && parsedRevision != expectedRevision) ||
			(nullptr != lightRevision &&
			 (!Try_U32(*lightRevision,
				(std::numeric_limits<std::uint32_t>::max)(), parsedLightRevision) ||
			  0u == parsedLightRevision)) || nullptr == bindings ||
			bindings->Get_Array().empty() ||
			bindings->Get_Array().size() > 16384u)
		{
			outStatus = "KoukuSaydon Product animation binding header is invalid.";
			return false;
		}

		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> staged;
		std::size_t skipped = 0u;
		std::unordered_set<std::string> duplicates;
		for (const DATA_JSON_VALUE& value : bindings->Get_Array())
		{
            const auto hasAnimationFields = [&value]()
            {
                const std::initializer_list<std::string_view> required = {"actionId", "occurrenceId", "clip", "startOffsetMs", "sourceStartMs", "playMs", "playRate", "endPolicy"};
                const std::initializer_list<std::string_view> optional = {"unblendedBoneContact", "animationRootVerticalScale", "blendInMs", "blendFromClip", "blendFromSourceMs", "holdAtWindowEnd", "sourceEndMs", "animationBlendWindows"};
                if (!value.Is_Object()) return false;
                for (auto name : required) if (!value.Find(name)) return false;
                for (const auto& [key, field] : value.Get_Object())
                    if (std::find(required.begin(), required.end(), key) == required.end() &&
                        std::find(optional.begin(), optional.end(), key) == optional.end()) return false;
                return true;
            };
            if (!hasAnimationFields())
			{
				outStatus = "KoukuSaydon Product animation row has unexpected fields.";
				++skipped; continue;
			}
			const DATA_JSON_VALUE* action = Required(
				value, "actionId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* occurrence = Required(
				value, "occurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* clip = Required(
				value, "clip", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* startOffset = Required(
				value, "startOffsetMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* sourceStart = Required(
				value, "sourceStartMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* playMs = Required(
				value, "playMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* playRate = Required(
				value, "playRate", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* endPolicy = Required(
				value, "endPolicy", DATA_JSON_TYPE::STRING);
			KOUKU_SAYDON_ACTION_PRESENTATION row;
			const DATA_JSON_VALUE* unblended = value.Find("unblendedBoneContact");
			const auto* verticalScale = value.Find("animationRootVerticalScale");
			const auto* holdAtEnd = value.Find("holdAtWindowEnd");
			const auto* sourceEnd = value.Find("sourceEndMs");
			if (nullptr == action || !Is_StableToken(action->Get_String()) ||
				nullptr == occurrence ||
				!Is_StableToken(occurrence->Get_String()) || nullptr == clip ||
				!Is_StableToken(clip->Get_String()) || nullptr == startOffset ||
				!Try_U32(*startOffset, 600000u, row.iStartOffsetMs) ||
				nullptr == sourceStart ||
				!Try_U32(*sourceStart, 600000u, row.iSourceStartMs) ||
				(sourceEnd && !Try_U32(*sourceEnd, 600000u, row.iSourceEndMs)) || nullptr == playMs ||
				!Try_U32(*playMs, 600000u, row.iPlayMs) || 0u == row.iPlayMs ||
				nullptr == playRate || !playRate->Is_Number() ||
				!std::isfinite(playRate->Get_Number()) ||
				playRate->Get_Number() < 0.1 || playRate->Get_Number() > 4.0 ||
				nullptr == endPolicy || (endPolicy->Get_String() != "EXACT" &&
                    endPolicy->Get_String() != "HOLD_LAST_POSE" && endPolicy->Get_String() != "LOOP_TO_WINDOW") ||
                (holdAtEnd && !holdAtEnd->Is_Boolean()) ||
				!Has_Clip(model, clip->Get_String()) || (unblended && !unblended->Is_Boolean()) ||
				(verticalScale && (!verticalScale->Is_Number() || !std::isfinite(verticalScale->Get_Number()) ||
					verticalScale->Get_Number() < 0.0 || verticalScale->Get_Number() > 1.0)))
			{
				outStatus = "KoukuSaydon Product animation row is invalid or unsupported.";
				++skipped; continue;
			}
			row.strActionId = action->Get_String();
			row.strOccurrenceId = occurrence->Get_String();
			row.strClip = clip->Get_String();
			row.fPlayRate = static_cast<f32_t>(playRate->Get_Number());
			row.fAnimationRootVerticalScale = verticalScale ? static_cast<f32_t>(verticalScale->Get_Number()) : 1.f;
			row.bUnblendedBoneContact = unblended && unblended->Get_Boolean();
            row.bLoopToWindow = endPolicy->Get_String() == "LOOP_TO_WINDOW";
            row.bHoldAtWindowEnd = (holdAtEnd && holdAtEnd->Get_Boolean()) || endPolicy->Get_String() == "HOLD_LAST_POSE";
            double nativeDurationMs = 0.0, sampledSourceMs = 0.0;
            if (!Try_GetClipDurationMs(model, row.strClip, nativeDurationMs) ||
                (endPolicy->Get_String() != "HOLD_LAST_POSE" && row.iSourceStartMs >= nativeDurationMs) ||
                !CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
                    row.iSourceStartMs, row.iSourceEndMs, 0.0, row.fPlayRate,
                    nativeDurationMs, row.bLoopToWindow, sampledSourceMs))
            {
                outStatus = "KoukuSaydon Product animation source range is invalid: " + row.strActionId;
                ++skipped; continue;
            }
            const auto* blendMs = value.Find("blendInMs");
            const auto* blendClip = value.Find("blendFromClip");
            const auto* blendSource = value.Find("blendFromSourceMs");
            if (blendMs || blendClip || blendSource)
            {
                if (!blendMs || !Try_U32(*blendMs, 1000u, row.iBlendInMs) || !row.iBlendInMs ||
                    row.iBlendInMs > row.iPlayMs || !blendClip || !blendClip->Is_String() ||
                    !Has_Clip(model, blendClip->Get_String()) || !blendSource || !blendSource->Is_Number() ||
                    !std::isfinite(blendSource->Get_Number()) || blendSource->Get_Number() < 0.0 ||
                    blendSource->Get_Number() > 600000.0)
                { outStatus = "KoukuSaydon Product animation transition is invalid."; ++skipped; continue; }
                double previousDurationMs = 0.0;
                if (!Try_GetClipDurationMs(model, blendClip->Get_String(), previousDurationMs))
                { outStatus = "KoukuSaydon Product animation transition clip is invalid."; ++skipped; continue; }
                row.strBlendFromClip = blendClip->Get_String();
                // Publisher samples the previous cropped range at its actual
                // transition boundary. Preserve the legacy native-end clamp when
                // an older Product row stores an uncapped previous source time.
                row.fBlendFromSourceMs = float((std::min)(previousDurationMs, blendSource->Get_Number()));
            }
            if (const auto* windows = value.Find("animationBlendWindows"))
            {
                if (!CKoukuSaydonAnimationBlend::Read_ProductWindows(*windows, row.AnimationBlendWindows, outStatus) ||
                    !CKoukuSaydonAnimationBlend::Validate_ModelWindows(model, row.AnimationBlendWindows, outStatus))
                { ++skipped; continue; }
            }
			const std::string actionId = row.strActionId;
			if (duplicates.contains(actionId) || !staged.emplace(actionId, std::move(row)).second)
			{
				staged.erase(actionId);
				duplicates.insert(actionId);
				outStatus = "KoukuSaydon Product animation actionId is duplicated.";
				++skipped; continue;
			}
		}
        ATTACHMENT_GRIPS stagedGrips;
        if (const auto* grips = root.Find("attachmentGrips"))
        {
            const auto* patterns = Required(root, "patterns", DATA_JSON_TYPE::ARRAY);
            if (!grips->Is_Array() || grips->Get_Array().size() > 4096u || !patterns)
            { outStatus = "KoukuSaydon attachmentGrips requires bounded rows and Product patterns."; return false; }
            for (const auto& grip : grips->Get_Array())
            {
                if (!Has_ExactProperties(grip, {"patternId", "attachmentSlot", "gripLocalOffset"}))
                { outStatus = "KoukuSaydon attachment grip fields are malformed."; return false; }
                const auto* id = Required(grip, "patternId", DATA_JSON_TYPE::STRING);
                const auto* slot = Required(grip, "attachmentSlot", DATA_JSON_TYPE::STRING);
                const auto* offset = Required(grip, "gripLocalOffset", DATA_JSON_TYPE::ARRAY);
                if (!id || !Is_StableToken(id->Get_String()) || !slot || slot->Get_String() != "BOSS_LEFT_HAND" ||
                    !offset || offset->Get_Array().size() != 3u ||
                    std::count_if(patterns->Get_Array().begin(), patterns->Get_Array().end(), [&](const auto& pattern)
                    { const auto* patternId = Required(pattern, "patternId", DATA_JSON_TYPE::STRING);
                      return patternId && patternId->Get_String() == id->Get_String(); }) != 1)
                { outStatus = "KoukuSaydon attachment grip has an invalid pattern, slot or offset."; return false; }
                float components[3]{};
                for (std::size_t i = 0; i < 3u; ++i)
                {
                    const auto& number = offset->Get_Array()[i];
                    if (!number.Is_Number() || !std::isfinite(number.Get_Number()) ||
                        std::abs(number.Get_Number()) > CPlayerHandGripTransform::MAX_GRIP_OFFSET_COMPONENT_M)
                    { outStatus = "KoukuSaydon gripLocalOffset must contain finite metre components within +/-10."; return false; }
                    components[i] = static_cast<float>(number.Get_Number());
                }
                const PLAYER_HAND_GRIP_LOCAL_OFFSET parsed{components[0], components[1], components[2]};
                if (!stagedGrips.emplace(id->Get_String(), parsed).second)
                { outStatus = "KoukuSaydon attachment grip patternId is duplicated."; return false; }
            }
        }
        outGrips = std::move(stagedGrips);
		out = std::move(staged);
		outRevision = parsedRevision;
        if (outCanonicalSource) *outCanonicalSource = std::move(canonicalSource);
		outStatus = "Loaded " + std::to_string(out.size()) +
			" KoukuSaydon Product animation action(s), skipped " + std::to_string(skipped) + " invalid row(s).";
		return true;
	}

	HRESULT Reject(const std::string_view reason)
	{
		g_Status = std::string(reason);
		OutputDebugStringA(("[KoukuSaydonPresentation] " + g_Status + "\n").c_str());
		return E_FAIL;
	}

	bool Is_FamilyPresentationId(const std::string_view presentationId)
	{
		return presentationId.size() >
				KOUKU_FAMILY_PRESENTATION_PREFIX.size() +
				KOUKU_FAMILY_PRESENTATION_SUFFIX.size() &&
			presentationId.starts_with(KOUKU_FAMILY_PRESENTATION_PREFIX) &&
			presentationId.ends_with(KOUKU_FAMILY_PRESENTATION_SUFFIX);
	}

	/* Archetype IDs are stable ASCII tokens, so the widening is a plain copy. */
	std::wstring Widen_Ascii(const std::string_view value)
	{
		std::wstring wide;
		wide.reserve(value.size());
		for (const char c : value)
			wide.push_back(static_cast<wchar_t>(static_cast<unsigned char>(c)));
		return wide;
	}
}

void Client::CKoukuSaydonPresentationAssetService::Begin_LevelLoad(
	const std::uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_KoukuAssetMutex };
	g_ReadyByLevel.erase(iLevelIndex);
	g_ActionPresentationsByArchetype.clear();
	g_AttachmentGripsByArchetype.clear();
	g_BindingSourceRevisions.clear();
    g_CanonicalBindingSources.clear(); g_LastCanonicalBindingSource.reset(); g_BindingPreparation.reset();
    g_PreparedDraft.reset(); g_AdmittedDraft.reset();
    g_AdmittedRunEpoch = g_AdmittedSourceRevision = g_PreparedAuthorizedEpoch = 0u; g_RunAdmissionFailed = false;
	g_Status = "KoukuSaydon presentation is waiting for Product admission.";
}

bool_t Client::CKoukuSaydonPresentationAssetService::Is_ArenaBossArchetype(
	const std::string_view archetypeId)
{
	if (!archetypeId.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX) ||
		!Is_StableToken(archetypeId))
	{
		return false;
	}
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	return nullptr != actor &&
		Is_FamilyPresentationId(actor->clientPresentationId);
}

std::wstring Client::CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag(
	const std::string_view archetypeId)
{
	if (archetypeId == KOUKU_ARCHETYPE)
		return L"Prototype_Component_Model_KoukuSaydon_MN_RPCZ_00";
	if (!Is_ArenaBossArchetype(archetypeId))
		return {};
	return KOUKU_MODEL_PROTOTYPE_PREFIX + Widen_Ascii(archetypeId);
}

std::wstring
Client::CKoukuSaydonPresentationAssetService::Get_WeaponModelPrototypeTag(
	const std::string_view archetypeId)
{
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	if (nullptr == actor || actor->weaponModel.empty() ||
		!Is_ArenaBossArchetype(archetypeId))
	{
		return {};
	}
	return Get_ModelPrototypeTag(archetypeId) + KOUKU_WEAPON_PROTOTYPE_SUFFIX;
}

const char_t*
Client::CKoukuSaydonPresentationAssetService::Get_WeaponSocketBone()
{
	return KOUKU_WEAPON_SOCKET_BONE;
}

const wchar_t*
Client::CKoukuSaydonPresentationAssetService::Get_GameObjectPrototypeTag()
{
	return KOUKU_OBJECT_PROTOTYPE;
}


HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_MazeHammerPrototype(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex)
{
	if (!pDevice || !pContext || iLevelIndex >= ETOUI(LEVEL::END)) return E_INVALIDARG;
	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	constexpr const char* key = "avatar.kouku-saydon.maze-hammer";
	if (ready.contains(key)) return S_FALSE;
	Engine::MODEL_ASSET_LOAD_DESC load;
	std::string status;
	if (!CActorCatalog::Build_ModelLoadDescription(
		"Effect/KoukuSaydon/WorldObjects/WhirlwindHammer/WhirlwindHammer.wmodel", load, status))
		return Reject("Card maze hammer material input failed: " + status);
	// The same world-object mesh is 56.016 cm long. The hand bone already
	// carries the class cm-to-m conversion; only the world's authored x2 remains.
	auto model = Engine::CModel::Create(pDevice, pContext, MODEL::NONANIM, load,
		XMMatrixScaling(2.f, 2.f, 2.f));
	if (!model || !model->Get_NumMeshes()) return Reject("Card maze hammer model failed.");
	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(KOUKU_MAZE_HAMMER_PROTOTYPE_TAG, std::move(model));
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("Card maze hammer prototype commit failed.");
	ready.insert(key);
	return S_OK;
}

HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_ClownBodyPrototype(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex)
{
	/* Polymorph 4134 is the player's MN_RPCZ_00-1 madness doll. Its
	100-bone body embeds its original and offline-tuned clips with variant material;
	the existing MN_RPCZ_00 boss remains a separate catalog presentation. */
	constexpr std::string_view CLOWN_BODY_ASSET =
		"Character/KoukuSaton/MN_RPCZ_00-1/MN_RPCZ_00-1.wmodel";
	constexpr f32_t CLOWN_BODY_PRE_SCALE = 0.017f * 0.709f;
	constexpr std::string_view CLOWN_READY_KEY = "avatar.kouku-saydon.clown";
	constexpr const char_t* CLOWN_IDLE_CLIP = "rpcz00p_idle_battle_1";
	constexpr const char_t* CLOWN_RUN_CLIP = "rpcz00p_run_battle_1";
	/* The hammer the doll swings is the Mario-1 monster's (WP_MN_RHKP_07). It
	is a submesh of REUP.wmodel carried by one Biped bone; the static cook
	beside REUP holds it in the monster's right-hand frame, in metres, and
	shares REUP's textures. The doll's skeleton chain bakes x100 into every
	bone, so a socketed metre-authored mesh receives 100 x 0.012053 per unit;
	in a Mario stage the doll is further scaled to 1.5 m (0.632). This
	pre-scale is 1 / (100 x 0.012053 x 0.632), so the hammer in the stage is
	the monster's 1.05 m, not shrunk with the doll. The cook is in the hand
	frame with the head along local +Y and the head's star faces along local
	X; on this doll's hand that put the head behind the character, so the
	pitch (about X) turns it round without moving the star faces. Sampled
	from the first key of rpcz00p_idle_battle_1, hand -Y after 180 points
	straight forward and hand +Z points mostly down, so pitch past 180 raises
	the head; the hand's Z is not vertical, so 40 degrees of pitch is 30
	degrees of elevation. 220 puts the head 30 degrees above forward in the
	idle stance. */
	constexpr std::string_view CLOWN_HAMMER_ASSET =
		"Character/Monster/MarioOriginal/REUP/WP_MN_RHKP_07_Static.wmodel";
	constexpr f32_t CLOWN_HAMMER_PRE_SCALE = 1.313f;
	constexpr f32_t CLOWN_HAMMER_PITCH_DEGREES = 220.f;
	constexpr f32_t CLOWN_HAMMER_YAW_DEGREES = 0.f;
	constexpr f32_t CLOWN_HAMMER_ROLL_DEGREES = 0.f;
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;

	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	if (ready.contains(std::string(CLOWN_READY_KEY)))
		return S_FALSE;
    Engine::MODEL_ASSET_LOAD_DESC bodyLoad;
    std::string materialStatus;
    if (!CActorCatalog::Build_ModelLoadDescription(CLOWN_BODY_ASSET, bodyLoad, materialStatus))
        return Reject("KoukuSaydon clown material input failed: " + materialStatus);
	/* CCharacter turns every playable body with the same -90 degree admission
	yaw (see CPlayableCharacterAssetService); the avatar follows that so it
	faces where the class body faced. */
	unique_ptr<Engine::CModel> body = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyLoad,
		XMMatrixScaling(CLOWN_BODY_PRE_SCALE, CLOWN_BODY_PRE_SCALE, CLOWN_BODY_PRE_SCALE) *
		XMMatrixRotationY(XMConvertToRadians(-90.f)));
	if (nullptr == body || 0u == body->Get_NumMeshes() ||
		0u == body->Get_SkeletonHash() || !body->Has_Animations())
	{
		return Reject("KoukuSaydon clown body has no usable animated geometry.");
	}
	if (!Has_Clip(*body, CLOWN_IDLE_CLIP) || !Has_Clip(*body, CLOWN_RUN_CLIP))
		return Reject("KoukuSaydon clown body is missing its idle/run clips.");
	/* Without the model the doll mimes its own swing clip. The socket bone is
	checked before the body is moved from. A class weapon is a static part on
	the static shader, so the hammer is admitted exactly that way; the rigged
	original would be refused as NONANIM and take the body down with it. */
	if (!body->Has_Bone(KOUKU_CLOWN_HAMMER_SOCKET_BONE))
		return Reject("KoukuSaydon clown body has no right-hand bone for the hammer.");
	const std::filesystem::path hammerPath =
		CRuntimeAssetRoot::Resolve(CLOWN_HAMMER_ASSET);
	if (hammerPath.empty())
		return Reject("KoukuSaydon clown hammer asset path is invalid.");
	unique_ptr<Engine::CModel> hammer = Engine::CModel::Create(
		pDevice, pContext, MODEL::NONANIM, hammerPath.string().c_str(),
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(CLOWN_HAMMER_PITCH_DEGREES),
			XMConvertToRadians(CLOWN_HAMMER_YAW_DEGREES),
			XMConvertToRadians(CLOWN_HAMMER_ROLL_DEGREES)) *
		XMMatrixScaling(CLOWN_HAMMER_PRE_SCALE,
			CLOWN_HAMMER_PRE_SCALE, CLOWN_HAMMER_PRE_SCALE));
	if (nullptr == hammer || 0u == hammer->Get_NumMeshes())
		return Reject("KoukuSaydon clown hammer static cook did not load.");
	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(KOUKU_CLOWN_BODY_PROTOTYPE_TAG, std::move(body));
	staged.emplace_back(KOUKU_CLOWN_HAMMER_PROTOTYPE_TAG, std::move(hammer));
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("KoukuSaydon clown body prototype commit failed.");
	ready.insert(std::string(CLOWN_READY_KEY));
	g_Status = "KoukuSaydon clown body admitted.";
	return S_OK;
}

HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) || !Is_ArenaBossArchetype(archetypeId))
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	if (ready.contains(std::string(archetypeId)))
		return S_FALSE;

	const bool_t isGateOneKouku = archetypeId == KOUKU_ARCHETYPE;
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	if (nullptr == actor ||
		(isGateOneKouku &&
		 (actor->clientPresentationId != KOUKU_PRESENTATION ||
		  !actor->weaponModel.empty() || 0.f != actor->weaponModelPreScale)) ||
		(!actor->weaponModel.empty() && actor->weaponModelPreScale <= 0.f))
	{
		return Reject("No exact embedded-body KoukuSaydon boss catalog row exists.");
	}
    Engine::MODEL_ASSET_LOAD_DESC bodyLoad;
    std::string materialStatus;
    if (!CActorCatalog::Build_ModelLoadDescription(actor->bodyModel, bodyLoad, materialStatus))
        return Reject("KoukuSaydon body material input failed: " + materialStatus);

	const f32_t scale = actor->bodyModelPreScale;
	unique_ptr<Engine::CModel> body = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyLoad,
		XMMatrixScaling(scale, scale, scale));
	if (nullptr == body || 0u == body->Get_NumMeshes() ||
		0u == body->Get_SkeletonHash() || !body->Has_Animations())
	{
		return Reject("KoukuSaydon embedded body has no usable animated geometry.");
	}
	if (!actor->animationSetId.empty() && actor->animationSetId != actor->bodyModel)
	{
		const auto donorPath = CRuntimeAssetRoot::Resolve(actor->animationSetId);
		if (donorPath.empty()) return Reject("KoukuSaydon animation donor path is invalid.");
		const auto donor = Engine::CModel::Create(pDevice, pContext, MODEL::ANIM,
			donorPath.string().c_str(), XMMatrixScaling(scale, scale, scale));
		// Attach validates the full skeleton identity and every duplicate clip before mutation.
		// Body geometry and its embedded gameplay clips remain on this same prototype.
		if (!donor || !donor->Has_Animations() || FAILED(body->Attach_AnimationSet(*donor)))
			return Reject("KoukuSaydon animation donor does not match its body: " + actor->animationSetId);
	}
	if (!Has_Clip(*body, actor->presentationClips.idle))
		return Reject("KoukuSaydon body is missing the catalog idle clip.");

	/* The weapon is a second animated model following the body's source clock
	and socket bone. Its own pre-transform converts the weapon's authored
	units; the socket bone matrix later adds the body's pre-transform. */
	unique_ptr<Engine::CModel> weapon;
	if (!actor->weaponModel.empty())
	{
		if (!body->Has_Bone(KOUKU_WEAPON_SOCKET_BONE))
			return Reject("KoukuSaydon body has no weapon socket bone for its catalog weapon.");
        Engine::MODEL_ASSET_LOAD_DESC weaponLoad;
        if (!CActorCatalog::Build_ModelLoadDescription(actor->weaponModel, weaponLoad, materialStatus))
            return Reject("KoukuSaydon weapon material input failed: " + materialStatus);
		/* The catalog rotation turns the weapon's authored axes onto the
		socket's before the scale; the scale is uniform, so the order only
		documents the intent. */
		const f32_t weaponScale = actor->weaponModelPreScale;
		const float3_t& weaponRotation = actor->weaponModelPreRotationDegrees;
		weapon = Engine::CModel::Create(
			pDevice, pContext, MODEL::ANIM, weaponLoad,
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(weaponRotation.x),
				XMConvertToRadians(weaponRotation.y),
				XMConvertToRadians(weaponRotation.z)) *
			XMMatrixScaling(weaponScale, weaponScale, weaponScale));
		if (nullptr == weapon || 0u == weapon->Get_NumMeshes())
			return Reject("KoukuSaydon weapon has no usable geometry.");
	}

	/* One Product binding document serves every arena boss; the loader keeps
	only the rows whose clip exists on this body, so a Saydon pattern never
	resolves on the Kouku rig and vice versa. */
	std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> bindings;
	std::string bindingStatus;
	std::uint32_t bindingRevision = 0u;
	ATTACHMENT_GRIPS grips;
    std::shared_ptr<const std::string> canonicalSource;
	if (!Load_PresentationBindings(*body, bindings, bindingStatus, bindingRevision, grips, 0u, nullptr, false, &canonicalSource))
	{
		// A missing action document must not remove the boss body and other tools.
		bindingStatus = "Animation bindings unavailable; boss body remains usable: " + bindingStatus;
		OutputDebugStringA(("[KoukuSaydonPresentation] " + bindingStatus + "\n").c_str());
	}
	bindingStatus = std::string(archetypeId) + ": " + bindingStatus;

	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(Get_ModelPrototypeTag(archetypeId), std::move(body));
	if (nullptr != weapon)
		staged.emplace_back(Get_WeaponModelPrototypeTag(archetypeId), std::move(weapon));
	/* One CNpc game-object prototype serves every arena boss of this level;
	it is committed with the first admitted archetype only. */
	if (ready.empty())
		staged.emplace_back(KOUKU_OBJECT_PROTOTYPE, CNpc::Create(pDevice, pContext));
	for (const auto& [tag, prototype] : staged)
	{
		if (tag.empty() || nullptr == prototype)
			return Reject("KoukuSaydon presentation prototype creation failed.");
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("KoukuSaydon presentation prototype commit failed.");

	g_ActionPresentationsByArchetype[std::string(archetypeId)] = std::move(bindings);
	g_AttachmentGripsByArchetype[std::string(archetypeId)] = std::move(grips);
	g_BindingSourceRevisions[std::string(archetypeId)] = bindingRevision;
    g_CanonicalBindingSources[std::string(archetypeId)] = std::move(canonicalSource);
	ready.insert(std::string(archetypeId));
	g_Status = std::move(bindingStatus);
	return S_OK;
}

bool_t Client::CKoukuSaydonPresentationAssetService::Try_Resolve_Action(
	const std::string_view archetypeId,
	const std::string_view actionId,
	KOUKU_SAYDON_ACTION_PRESENTATION& outPresentation, const std::uint32_t expectedSourceRevision)
{
	std::scoped_lock lock{ g_KoukuAssetMutex };
	if (g_RunAdmissionFailed) return false;
	if (expectedSourceRevision && g_BindingSourceRevisions[std::string(archetypeId)] != expectedSourceRevision) return false;
	const auto owner = g_ActionPresentationsByArchetype.find(std::string(archetypeId));
	if (owner == g_ActionPresentationsByArchetype.end())
		return false;
	const auto found = owner->second.find(std::string(actionId));
	if (found == owner->second.end())
		return false;
	outPresentation = found->second;
	return true;
}

bool_t Client::CKoukuSaydonPresentationAssetService::Try_Resolve_AttachmentGrip(
    const std::string_view archetypeId, const std::string_view patternId,
    const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
    PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset, const std::uint32_t expectedSourceRevision)
{
    if (slot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND) return false;
    std::scoped_lock lock{ g_KoukuAssetMutex };
    if (g_RunAdmissionFailed) return false;
    const auto revision = g_BindingSourceRevisions.find(std::string(archetypeId));
    if (revision == g_BindingSourceRevisions.end() || !revision->second ||
        (expectedSourceRevision && revision->second != expectedSourceRevision)) return false;
    const auto owner = g_AttachmentGripsByArchetype.find(std::string(archetypeId));
    if (owner == g_AttachmentGripsByArchetype.end()) return false;
    const auto found = owner->second.find(std::string(patternId));
    if (found == owner->second.end()) return false;
    outOffset = found->second;
    return true;
}

const std::string&
Client::CKoukuSaydonPresentationAssetService::Get_Status()
{
	return g_Status;
}


bool_t Client::CKoukuSaydonPresentationAssetService::Reload_ProductBindings(
    std::uint32_t levelIndex, std::uint32_t expectedSourceRevision, std::string& status)
{
    return Admit_RunProduct(levelIndex, expectedSourceRevision, {}, 0u, status);
}

std::shared_ptr<const Client::KOUKU_SAYDON_DRAFT_PRODUCT>
Client::CKoukuSaydonPresentationAssetService::Prepare_DraftProduct(
    const std::string& presentationJson, const std::string& encounterJson, const std::string& gameplayRows,
    const std::uint32_t sourceRevision, std::string& status)
{
    auto candidate = std::make_shared<KOUKU_SAYDON_DRAFT_PRODUCT>();
    candidate->PresentationJson = presentationJson; candidate->EncounterJson = encounterJson;
    candidate->iSourceRevision = sourceRevision;
    if (!sourceRevision || !CNetworkManager::Compute_KoukuDraftRowsRevision(gameplayRows, candidate->RowsRevision) ||
        !CKoukuSaydonPresentationPlayer::Validate_DraftProductJson(presentationJson, sourceRevision, status))
    { if (status.empty()) status = "Draft gameplay/presentation identity is invalid."; return {}; }
    DATA_JSON_VALUE root; std::string error;
    if (encounterJson.empty() || encounterJson.size() > 16u * 1024u * 1024u || !CDataJson::Parse(encounterJson, root, error))
    { status = "Draft encounter is invalid: " + error; return {}; }
    const auto* patterns = Required(root, "patterns", DATA_JSON_TYPE::ARRAY);
    if (!patterns || patterns->Get_Array().empty() || patterns->Get_Array().size() > 4096u)
    { status = "Draft encounter has no bounded pattern closure."; return {}; }
    std::vector<std::string> identities;
    for (const auto& pattern : patterns->Get_Array())
    {
        const auto* identity = Required(pattern, "patternId", DATA_JSON_TYPE::STRING);
        if (!identity) { status = "Draft encounter pattern identity is missing."; return {}; }
        identities.push_back(identity->Get_String());
    }
    KOUKU_SAYDON_PLAY_RESOURCES resources;
    if (!Collect_CompletePlayResources(identities, {}, sourceRevision, resources, status, candidate)) return {};
    status = "Immutable draft Product parsed; canonical files are unchanged.";
    return candidate;
}

bool Client::CKoukuSaydonPresentationAssetService::Stage_DraftProduct(
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft, std::string& status)
{
    if (!draft || !draft->iSourceRevision || !draft->RowsRevision.Is_Valid())
    { status = "Draft Product is not prepared."; return false; }
    std::scoped_lock lock{g_KoukuAssetMutex};
    g_PreparedDraft = std::move(draft); g_PreparedAuthorizedEpoch = 0u;
    status = "Draft Product staged for exact Server hash admission."; return true;
}

bool Client::CKoukuSaydonPresentationAssetService::Authorize_DraftProduct(
    const LostArk::Shared::GameplayDataRevision& rowsRevision, const std::uint32_t runEpoch, std::string& status)
{
    std::scoped_lock lock{g_KoukuAssetMutex};
    if (g_AdmittedDraft && rowsRevision == g_AdmittedDraft->RowsRevision && runEpoch == g_AdmittedRunEpoch) return true;
    if (!rowsRevision.Is_Valid() || !runEpoch || runEpoch <= g_AdmittedRunEpoch ||
        !g_PreparedDraft || rowsRevision != g_PreparedDraft->RowsRevision)
    { status = "Own accepted draft hash/epoch does not match the staged Product."; return false; }
    g_PreparedAuthorizedEpoch = runEpoch; status.clear(); return true;
}

bool Client::CKoukuSaydonPresentationAssetService::Prepare_ProductBindings(
    const std::uint32_t levelIndex, const std::uint32_t sourceRevision, bool& ready, std::string& status,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft)
{
    ready = false;
    std::scoped_lock lock{g_KoukuAssetMutex};
    const auto archetypes = Ready_CanonicalBindingArchetypes(levelIndex);
    if (!sourceRevision || archetypes.empty())
    { status = "Animation preparation requires an exact revision and prepared boss models."; return false; }
    if (draft && (draft->iSourceRevision != sourceRevision || !draft->RowsRevision.Is_Valid()))
    { status = "Draft animation preparation has no exact source/hash identity."; return false; }
    std::shared_ptr<const std::string> source;
    if (draft) source = std::shared_ptr<const std::string>(draft, &draft->PresentationJson);
    else if (!Read_CanonicalBindingSource(source, status)) return false;
    if (!g_BindingPreparation || g_BindingPreparation->levelIndex != levelIndex ||
        g_BindingPreparation->sourceRevision != sourceRevision || g_BindingPreparation->archetypes != archetypes ||
        g_BindingPreparation->draft != draft)
    {
        auto staged = std::make_unique<BINDING_PREPARATION>();
        staged->levelIndex = levelIndex; staged->sourceRevision = sourceRevision;
        staged->source = source; staged->draft = draft; staged->archetypes = archetypes;
        staged->reusesActive = !draft && Have_PreparedCanonicalBindings(levelIndex, sourceRevision, *source);
        if (staged->reusesActive) staged->nextArchetype = archetypes.size();
        else
        {
            staged->actions = g_ActionPresentationsByArchetype; staged->grips = g_AttachmentGripsByArchetype;
            staged->revisions = g_BindingSourceRevisions; staged->sources = g_CanonicalBindingSources;
        }
        g_BindingPreparation = std::move(staged);
    }
    auto& staged = *g_BindingPreparation;
    if (!draft && *source != *staged.source)
    { status = "Canonical animation source changed during preparation. Prepare the saved Product again."; return false; }
    if (staged.nextArchetype < staged.archetypes.size())
    {
        // Reusing a GPU model does not make its old revision's clip cache ready.
        // Validate one model per PREPARING frame, without touching the live run.
        const auto& archetype = staged.archetypes[staged.nextArchetype];
        const auto model = std::dynamic_pointer_cast<Engine::CModel>(CGameInstance::Get().Clone_Prototype(levelIndex, Get_ModelPrototypeTag(archetype)));
        std::uint32_t revision = 0u;
        std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> rows;
        ATTACHMENT_GRIPS grips;
        if (!model || !Load_PresentationBindings(*model, rows, status, revision, grips, sourceRevision, staged.source.get(), true))
        { status = "Animation preparation preserved the active cache: " + status; return false; }
        staged.actions[archetype] = std::move(rows); staged.grips[archetype] = std::move(grips);
        staged.revisions[archetype] = revision;
        staged.sources[archetype] = draft ? std::shared_ptr<const std::string>{} : staged.source;
        ++staged.nextArchetype;
        status = "Preparing animation bindings " + std::to_string(staged.nextArchetype) + "/" + std::to_string(staged.archetypes.size());
        return true;
    }
    ready = true;
    status = "Animation bindings are staged for the exact selected Product.";
    return true;
}

bool Client::CKoukuSaydonPresentationAssetService::Admit_RunProduct(
    const std::uint32_t levelIndex, const std::uint32_t sourceRevision,
    const LostArk::Shared::GameplayDataRevision& rowsRevision, const std::uint32_t runEpoch, std::string& status)
{
    std::scoped_lock lock{g_KoukuAssetMutex};
    const auto fail = [&](const std::string& reason) { g_RunAdmissionFailed = true; status = reason; return false; };
    if (!sourceRevision) return fail("Admitted Product source revision is missing.");
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> candidate;
    if (rowsRevision.Is_Valid())
    {
        if (g_AdmittedDraft && runEpoch == g_AdmittedRunEpoch && rowsRevision == g_AdmittedDraft->RowsRevision &&
            sourceRevision == g_AdmittedDraft->iSourceRevision) candidate = g_AdmittedDraft;
        else if (g_PreparedDraft && runEpoch == g_PreparedAuthorizedEpoch && runEpoch > g_AdmittedRunEpoch &&
            rowsRevision == g_PreparedDraft->RowsRevision && sourceRevision == g_PreparedDraft->iSourceRevision) candidate = g_PreparedDraft;
        else return fail("Admitted draft hash/source is unavailable locally; previous caches are preserved.");
        if (!runEpoch) return fail("Draft admission requires the Server run epoch.");
    }
    const auto commitAdmission = [&]() {
        g_AdmittedDraft = candidate;
        g_AdmittedRunEpoch = runEpoch; g_AdmittedSourceRevision = sourceRevision; g_RunAdmissionFailed = false;
        status = g_Status = "Product animation bindings admitted for the exact run.";
        return true;
    };
    if (g_BindingPreparation && g_BindingPreparation->levelIndex == levelIndex &&
        g_BindingPreparation->sourceRevision == sourceRevision && g_BindingPreparation->draft == candidate)
    {
        auto& prepared = *g_BindingPreparation;
        if (prepared.nextArchetype != prepared.archetypes.size() || Ready_CanonicalBindingArchetypes(levelIndex) != prepared.archetypes)
            return fail("Animation Product reached admission before its preparation barrier completed.");
        // Draft bytes are owned by the exact immutable pointer whose hash/epoch
        // was authorized above. Canonical files also need a final freshness read.
        auto source = prepared.source;
        if (!candidate)
        {
            if (!Read_CanonicalBindingSource(source, status)) return fail(status);
            if (*source != *prepared.source)
                return fail("Canonical animation source changed after READY; the previous cache is preserved. Prepare the saved Product again.");
        }
        if (prepared.reusesActive)
        {
            if (!Have_PreparedCanonicalBindings(levelIndex, sourceRevision, *source))
                return fail("Prepared canonical animation cache changed before admission; the previous cache is preserved.");
        }
        else
        {
            g_ActionPresentationsByArchetype = std::move(prepared.actions);
            g_AttachmentGripsByArchetype = std::move(prepared.grips);
            g_BindingSourceRevisions = std::move(prepared.revisions);
            g_CanonicalBindingSources = std::move(prepared.sources);
        }
        g_BindingPreparation.reset();
        return commitAdmission();
    }
    const bool retained = !g_RunAdmissionFailed && sourceRevision == g_AdmittedSourceRevision && candidate == g_AdmittedDraft &&
        (!candidate || runEpoch == g_AdmittedRunEpoch);
    // Later occurrences keep the immutable admitted pin; they must not copy every
    // archetype's validated rows just to update the room occurrence epoch.
    if (retained) return commitAdmission();
    if (!candidate && !g_AdmittedDraft)
    {
        std::shared_ptr<const std::string> canonicalSource;
        if (!Read_CanonicalBindingSource(canonicalSource, status)) return fail(status);
        if (Have_PreparedCanonicalBindings(levelIndex, sourceRevision, *canonicalSource))
            return commitAdmission();
    }
    // Standalone/late-observer admission without a Complete Play barrier keeps
    // the existing recovery path. A prepared raid never reparses at first combat.
    auto staged = g_ActionPresentationsByArchetype;
    auto stagedGrips = g_AttachmentGripsByArchetype;
    auto revisions = g_BindingSourceRevisions;
    auto sources = g_CanonicalBindingSources;
    const auto ready = g_ReadyByLevel.find(levelIndex);
    if (ready != g_ReadyByLevel.end())
        for (const auto& archetype : ready->second)
        {
            if (!archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) continue;
            const auto model = std::dynamic_pointer_cast<Engine::CModel>(CGameInstance::Get().Clone_Prototype(levelIndex, Get_ModelPrototypeTag(archetype)));
            std::uint32_t revision = 0u; std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> rows; ATTACHMENT_GRIPS grips;
            std::shared_ptr<const std::string> canonicalSource;
            if (!model || !Load_PresentationBindings(*model, rows, status, revision, grips, sourceRevision,
                candidate ? &candidate->PresentationJson : nullptr, !candidate, &canonicalSource)) return fail("Product animation admission preserved the previous cache: " + status);
            staged[archetype] = std::move(rows); stagedGrips[archetype] = std::move(grips); revisions[archetype] = revision;
            sources[archetype] = std::move(canonicalSource);
        }
    g_ActionPresentationsByArchetype = std::move(staged); g_AttachmentGripsByArchetype = std::move(stagedGrips);
    g_BindingSourceRevisions = std::move(revisions); g_CanonicalBindingSources = std::move(sources);
    return commitAdmission();
}

bool Client::CKoukuSaydonPresentationAssetService::Matches_AdmittedRun(
    const std::uint32_t sourceRevision, const LostArk::Shared::GameplayDataRevision& rowsRevision, const std::uint32_t runEpoch)
{
    std::scoped_lock lock{g_KoukuAssetMutex};
    return !g_RunAdmissionFailed && sourceRevision == g_AdmittedSourceRevision && runEpoch == g_AdmittedRunEpoch &&
        rowsRevision == (g_AdmittedDraft ? g_AdmittedDraft->RowsRevision : LostArk::Shared::GameplayDataRevision{});
}

std::shared_ptr<const Client::KOUKU_SAYDON_DRAFT_PRODUCT>
Client::CKoukuSaydonPresentationAssetService::Get_AdmittedDraftProduct()
{
    std::scoped_lock lock{g_KoukuAssetMutex}; return g_AdmittedDraft;
}


bool Client::CKoukuSaydonPresentationAssetService::Collect_CompletePlayResources(
    const std::vector<std::string>& patternIds, const std::vector<std::string>& bundleIds,
    const std::uint32_t sourceRevision, KOUKU_SAYDON_PLAY_RESOURCES& output, std::string& status,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft)
{
    try
    {
        if (!sourceRevision || (patternIds.empty() && bundleIds.empty()))
            throw std::runtime_error("Complete Play requires a pinned, nonempty selection.");
        const auto text = [](const DATA_JSON_VALUE& row, const char* key) -> const std::string& {
            const auto* value = Required(row, key, DATA_JSON_TYPE::STRING);
            if (!value) throw std::runtime_error(std::string("Missing dependency identity: ") + key);
            return value->Get_String();
        };
        const auto array = [](const DATA_JSON_VALUE& row, const char* key) -> const auto& {
            const auto* value = Required(row, key, DATA_JSON_TYPE::ARRAY);
            if (!value || value->Get_Array().size() > 16384u)
                throw std::runtime_error(std::string("Invalid dependency array: ") + key);
            return value->Get_Array();
        };
        const auto read = [&](const wchar_t* relative, const char* schema, uint32_t version) {
            std::string data;
            const auto path = CProjectDataRoot::Resolve(relative);
            if (draft)
            {
                if (draft->iSourceRevision != sourceRevision || !draft->RowsRevision.Is_Valid())
                    throw std::runtime_error("Draft dependency identity does not match its selected run.");
                data = std::string_view(schema) == "lostark.encounter-profile" ? draft->EncounterJson : draft->PresentationJson;
                if (data.empty() || data.size() > 16u * 1024u * 1024u)
                    throw std::runtime_error("Draft dependency document is empty or oversized.");
            }
            else
            {
                std::error_code error;
                const auto bytes = std::filesystem::file_size(path, error);
                if (error || !bytes || bytes > 64u * 1024u * 1024u)
                    throw std::runtime_error("Missing/oversized Complete Play dependency document: " + path.string());
                std::ifstream input(path, std::ios::binary);
                data.resize(size_t(bytes)); input.read(data.data(), std::streamsize(bytes));
                if (!input || input.peek() != std::char_traits<char>::eof())
                    throw std::runtime_error("Complete Play dependency document changed during read: " + path.string());
            }
            DATA_JSON_VALUE root; DATA_JSON_PARSE_LIMITS limits; std::string errorText;
            limits.iMaximumBytes = 64u * 1024u * 1024u; limits.iMaximumValues = 4'000'000u;
            uint32_t revision = 0u, parsedVersion = 0u;
            if (!CDataJson::Parse(data, root, errorText, limits) || text(root, "schema") != schema ||
                !root.Find("sourceRevision") || !Try_U32(*root.Find("sourceRevision"), UINT32_MAX, revision) ||
                revision != sourceRevision || !root.Find("formatVersion") ||
                !Try_U32(*root.Find("formatVersion"), version, parsedVersion) || parsedVersion != version)
                throw std::runtime_error("Complete Play dependency document revision/header mismatch: " + path.string() + "; " + errorText);
            return root;
        };
        const auto encounter = read(L"Encounters/KoukuSaydon/KoukuSaydonEncounter.json", "lostark.encounter-profile", 4u);
        const auto presentation = read(L"Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json", "lostark.kouku-saydon-pattern-bindings", 1u);
        std::map<std::string, const DATA_JSON_VALUE*> patterns, visuals, fears, presentationPatterns, bundles;
        const auto index = [&](const DATA_JSON_VALUE& root, const char* field, const char* key, auto& target) {
            if (!root.Find(field)) return;
            for (const auto& row : array(root, field))
                if (!target.emplace(text(row, key), &row).second)
                    throw std::runtime_error("Duplicate Complete Play dependency: " + text(row, key));
        };
        index(encounter, "patterns", "patternId", patterns);
        index(encounter, "bundles", "bundleId", bundles);
        index(presentation, "patterns", "patternId", presentationPatterns);
        index(presentation, "targetedCombatVisuals", "clientVisualId", visuals);
        index(presentation, "fearPresentations", "presentationId", fears);
        std::set<std::string> pending(patternIds.begin(), patternIds.end()), visited, v1, worlds, actors;
        std::set<std::pair<std::string, std::string>> v2;
        std::set<std::string> selectedVisuals, selectedFears;
        const auto addEffect = [&](const DATA_JSON_VALUE& row) {
            if (text(row, "kind") != "EFFECT") return;
            const auto& id = text(row, "assetId"); const auto& kind = text(row, "resourceKind");
            if (id.empty()) throw std::runtime_error("Empty Complete Play Effect ID.");
            if (kind == "V1_EFFECT" || kind == "V1_ELEMENT") v1.insert(id);
            else if (kind == "GROUP" || kind == "LEAF") v2.emplace(kind, id);
            else throw std::runtime_error("Unsupported Complete Play Effect kind: " + kind);
        };
        for (const auto& id : bundleIds)
        {
            const auto found = bundles.find(id);
            if (found == bundles.end()) throw std::runtime_error("Missing Complete Play Bundle: " + id);
            for (const auto& member : array(*found->second, "members")) pending.insert(text(member, "patternId"));
        }
        // Traverse only typed Product identity fields. This includes all branches
        // and every future row, without filtering against stage time or duration.
        std::function<void(const DATA_JSON_VALUE&)> dependencies;
        dependencies = [&](const DATA_JSON_VALUE& value) {
            if (value.Is_Array()) { for (const auto& child : value.Get_Array()) dependencies(child); return; }
            if (!value.Is_Object()) return;
            for (const auto& [key, child] : value.Get_Object())
            {
                if (key == "patternId" || key == "clonePatternId")
                { if (child.Is_String() && !child.Get_String().empty()) pending.insert(child.Get_String()); }
                else if (key == "patternIds" || key == "directionPatternIds")
                {
                    if (!child.Is_Array()) throw std::runtime_error("Invalid child pattern dependency array.");
                    for (const auto& id : child.Get_Array())
                    { if (!id.Is_String() || id.Get_String().empty()) throw std::runtime_error("Invalid child pattern identity."); pending.insert(id.Get_String()); }
                }
                else if (key == "sequenceInstanceId" || key == "worldSequenceInstanceId" ||
                    key == "targetWorldInstanceId" || key == "motionInstanceId")
                { if (child.Is_String() && !child.Get_String().empty()) worlds.insert(child.Get_String()); }
                else if (key == "fixedVisualId" || key == "trackingVisualId" || key == "clientVisualId" || key == "selectedEffectVisualId")
                { if (child.Is_String() && !child.Get_String().empty()) selectedVisuals.insert(child.Get_String()); }
                else if (key == "visualIds")
                {
                    if (!child.Is_Array()) throw std::runtime_error("Invalid targeted visual dependencies.");
                    for (const auto& id : child.Get_Array())
                    { if (!id.Is_String()) throw std::runtime_error("Invalid targeted visual identity."); selectedVisuals.insert(id.Get_String()); }
                }
                else if (key == "presentationId")
                { if (child.Is_String() && !child.Get_String().empty()) selectedFears.insert(child.Get_String()); }
                dependencies(child);
            }
        };
        while (!pending.empty())
        {
            const auto id = *pending.begin(); pending.erase(pending.begin());
            if (!visited.insert(id).second) continue;
            if (visited.size() > 4096u) throw std::runtime_error("Complete Play pattern closure exceeds its bounded capacity.");
            const auto source = patterns.find(id), visual = presentationPatterns.find(id);
            if (source == patterns.end() || visual == presentationPatterns.end())
                throw std::runtime_error("Missing published Complete Play child pattern: " + id);
            dependencies(*source->second);
            const auto actor = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(text(*source->second, "targetBossPlacementId"));
            if (actor.empty()) throw std::runtime_error("Missing Complete Play boss identity: " + id);
            actors.emplace(actor);
            for (const auto& row : array(*visual->second, "presentationOccurrences"))
            { addEffect(row); dependencies(row); }
        }
        for (const auto& id : selectedVisuals)
        {
            const auto found = visuals.find(id);
            if (found == visuals.end()) throw std::runtime_error("Missing Complete Play combat visual: " + id);
            for (const auto& row : array(*found->second, "resources")) addEffect(row);
            if (const auto* contact = found->second->Find("contactEffectAssetId"); contact && contact->Is_String() && !contact->Get_String().empty())
                v1.insert(contact->Get_String());
        }
        for (const auto& id : selectedFears)
        {
            const auto found = fears.find(id);
            if (found == fears.end()) throw std::runtime_error("Missing Complete Play Fear presentation: " + id);
            if (const auto* effect = found->second->Find("effectResource"); effect && !effect->Is_Null()) addEffect(*effect);
        }
        // The same shared state presentations used by Release prewarm are selected
        // by authoritative card/ball state rather than a named occurrence.
        for (const char* symbol : {"heart", "spade", "clober", "dia"})
            for (const char* color : {"red", "black"}) v2.emplace("GROUP", std::string("boss.kouku.card.") + symbol + "." + color);
        for (const char* color : {"red", "blue", "yellow"}) v2.emplace("LEAF", std::string("boss.kouku.ball.smoke.") + color + "_1");
        KOUKU_SAYDON_PLAY_RESOURCES staged;
        staged.PatternIds.assign(visited.begin(), visited.end()); staged.V1EffectIds.assign(v1.begin(), v1.end());
        staged.V2Effects.assign(v2.begin(), v2.end()); staged.WorldInstanceIds.assign(worlds.begin(), worlds.end());
        staged.BossArchetypeIds.assign(actors.begin(), actors.end());
        output = std::move(staged);
        status = "Complete Play dependency closure collected.";
        return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
