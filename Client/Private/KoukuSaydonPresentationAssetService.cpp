#include "KoukuSaydonPresentationAssetService.h"

#include "ActorCatalog.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>
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
			{"patterns", "lightResourceRevision", "folders", "bundles", "fearPresentations", "attachmentGrips"};
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

	bool Load_PresentationBindings(
		const Engine::CModel& model,
		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION>& out,
		std::string& outStatus, std::uint32_t& outRevision, ATTACHMENT_GRIPS& outGrips,
		const std::uint32_t expectedRevision = 0u)
	{
		const std::filesystem::path path = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Animation/Authored/KoukuSaydon") /
			L"KoukuSaydon.patternbindings.json");
		std::error_code fileError;
		const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
		if (path.empty() || fileError || fileBytes > MAX_BINDING_BYTES)
		{
			outStatus = "KoukuSaydon Product animation binding is missing or oversized.";
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (!input || input.bad() || text.size() != fileBytes)
		{
			outStatus = "KoukuSaydon Product animation binding could not be read.";
			return false;
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
                const std::initializer_list<std::string_view> optional = {"unblendedBoneContact", "animationRootVerticalScale", "blendInMs", "blendFromClip", "blendFromSourceMs", "holdAtWindowEnd"};
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
			std::uint32_t parsedStartOffset = 0u;
			std::uint32_t parsedSourceStart = 0u;
			if (nullptr == action || !Is_StableToken(action->Get_String()) ||
				nullptr == occurrence ||
				!Is_StableToken(occurrence->Get_String()) || nullptr == clip ||
				!Is_StableToken(clip->Get_String()) || nullptr == startOffset ||
				!Try_U32(*startOffset, 600000u, parsedStartOffset) ||
				0u != parsedStartOffset || nullptr == sourceStart ||
				!Try_U32(*sourceStart, 600000u, parsedSourceStart) ||
				0u != parsedSourceStart || nullptr == playMs ||
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
                row.strBlendFromClip = blendClip->Get_String();
                row.fBlendFromSourceMs = float(blendSource->Get_Number());
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
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;

	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	if (ready.contains(std::string(CLOWN_READY_KEY)))
		return S_FALSE;
	const std::filesystem::path bodyPath = CRuntimeAssetRoot::Resolve(CLOWN_BODY_ASSET);
	if (bodyPath.empty())
		return Reject("KoukuSaydon clown body asset path is invalid.");
	/* CCharacter turns every playable body with the same -90 degree admission
	yaw (see CPlayableCharacterAssetService); the avatar follows that so it
	faces where the class body faced. */
	unique_ptr<Engine::CModel> body = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyPath.string().c_str(),
		XMMatrixScaling(CLOWN_BODY_PRE_SCALE, CLOWN_BODY_PRE_SCALE, CLOWN_BODY_PRE_SCALE) *
		XMMatrixRotationY(XMConvertToRadians(-90.f)));
	if (nullptr == body || 0u == body->Get_NumMeshes() ||
		0u == body->Get_SkeletonHash() || !body->Has_Animations())
	{
		return Reject("KoukuSaydon clown body has no usable animated geometry.");
	}
	if (!Has_Clip(*body, CLOWN_IDLE_CLIP) || !Has_Clip(*body, CLOWN_RUN_CLIP))
		return Reject("KoukuSaydon clown body is missing its idle/run clips.");
	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(KOUKU_CLOWN_BODY_PROTOTYPE_TAG, std::move(body));
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
	const std::filesystem::path bodyPath =
		CRuntimeAssetRoot::Resolve(actor->bodyModel);
	if (bodyPath.empty())
		return Reject("KoukuSaydon body asset path is invalid.");

	const f32_t scale = actor->bodyModelPreScale;
	unique_ptr<Engine::CModel> body = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyPath.string().c_str(),
		XMMatrixScaling(scale, scale, scale));
	if (nullptr == body || 0u == body->Get_NumMeshes() ||
		0u == body->Get_SkeletonHash() || !body->Has_Animations())
	{
		return Reject("KoukuSaydon embedded body has no usable animated geometry.");
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
		const std::filesystem::path weaponPath =
			CRuntimeAssetRoot::Resolve(actor->weaponModel);
		if (weaponPath.empty())
			return Reject("KoukuSaydon weapon asset path is invalid.");
		/* The catalog rotation turns the weapon's authored axes onto the
		socket's before the scale; the scale is uniform, so the order only
		documents the intent. */
		const f32_t weaponScale = actor->weaponModelPreScale;
		const float3_t& weaponRotation = actor->weaponModelPreRotationDegrees;
		weapon = Engine::CModel::Create(
			pDevice, pContext, MODEL::ANIM, weaponPath.string().c_str(),
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
	if (!Load_PresentationBindings(*body, bindings, bindingStatus, bindingRevision, grips))
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
    if (!expectedSourceRevision) { status = "Expected Product source revision is missing."; return false; }
    std::scoped_lock lock{ g_KoukuAssetMutex };
    auto staged = g_ActionPresentationsByArchetype;
    auto stagedGrips = g_AttachmentGripsByArchetype;
    auto revisions = g_BindingSourceRevisions;
    const auto ready = g_ReadyByLevel.find(levelIndex);
    if (ready != g_ReadyByLevel.end())
        for (const auto& archetype : ready->second)
        {
            if (!archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) continue;
            const auto model = std::dynamic_pointer_cast<Engine::CModel>(
                CGameInstance::Get().Clone_Prototype(levelIndex, Get_ModelPrototypeTag(archetype)));
            std::uint32_t revision = 0u;
            std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> rows;
            ATTACHMENT_GRIPS grips;
            if (!model || !Load_PresentationBindings(*model, rows, status, revision, grips, expectedSourceRevision))
            { g_Status = "Product animation reload preserved the previous cache: " + status; return false; }
            staged[archetype] = std::move(rows);
            stagedGrips[archetype] = std::move(grips);
            revisions[archetype] = revision;
        }
    g_ActionPresentationsByArchetype = std::move(staged);
    g_AttachmentGripsByArchetype = std::move(stagedGrips);
    g_BindingSourceRevisions = std::move(revisions);
    status = g_Status = "Product animation bindings admitted for source revision " + std::to_string(expectedSourceRevision);
    return true;
}
