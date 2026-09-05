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
	constexpr std::string_view BINDING_SCHEMA =
		"lostark.kouku-saydon-pattern-bindings";
	constexpr std::uint32_t BINDING_VERSION = 1u;
	constexpr std::uintmax_t MAX_BINDING_BYTES = 8u * 1024u * 1024u;
	constexpr const wchar_t* KOUKU_OBJECT_PROTOTYPE =
		L"Prototype_GameObject_KoukuSaydonPresentation";

	std::mutex g_KoukuAssetMutex;
	std::unordered_map<std::uint32_t, std::unordered_set<std::string>>
		g_ReadyByLevel;
	std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION>
		g_ActionPresentations;
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
		std::string& outStatus)
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
		if (!CDataJson::Parse(text, root, parseError) ||
			!Has_ExactProperties(root,
				{ "schema", "formatVersion", "bossArchetypeId",
				  "sourceRevision", "bindings" }))
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
		std::uint32_t parsedVersion = 0u;
		std::uint32_t parsedRevision = 0u;
		if (nullptr == schema || schema->Get_String() != BINDING_SCHEMA ||
			nullptr == version ||
			!Try_U32(*version, BINDING_VERSION, parsedVersion) ||
			parsedVersion != BINDING_VERSION || nullptr == archetype ||
			archetype->Get_String() != KOUKU_ARCHETYPE ||
			nullptr == sourceRevision ||
			!Try_U32(*sourceRevision,
				(std::numeric_limits<std::uint32_t>::max)(), parsedRevision) ||
			0u == parsedRevision || nullptr == bindings ||
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
			if (!Has_ExactProperties(value,
					{ "actionId", "occurrenceId", "clip", "startOffsetMs",
					  "sourceStartMs", "playMs", "playRate", "endPolicy" }))
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
				nullptr == endPolicy || endPolicy->Get_String() != "EXACT" ||
				!Has_Clip(model, clip->Get_String()))
			{
				outStatus = "KoukuSaydon Product animation row is invalid or unsupported.";
				++skipped; continue;
			}
			row.strActionId = action->Get_String();
			row.strOccurrenceId = occurrence->Get_String();
			row.strClip = clip->Get_String();
			row.fPlayRate = static_cast<f32_t>(playRate->Get_Number());
			const std::string actionId = row.strActionId;
			if (duplicates.contains(actionId) || !staged.emplace(actionId, std::move(row)).second)
			{
				staged.erase(actionId);
				duplicates.insert(actionId);
				outStatus = "KoukuSaydon Product animation actionId is duplicated.";
				++skipped; continue;
			}
		}
		out = std::move(staged);
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
}

void Client::CKoukuSaydonPresentationAssetService::Begin_LevelLoad(
	const std::uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_KoukuAssetMutex };
	g_ReadyByLevel.erase(iLevelIndex);
	g_ActionPresentations.clear();
	g_Status = "KoukuSaydon presentation is waiting for Product admission.";
}

std::wstring Client::CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag(
	const std::string_view archetypeId)
{
	if (archetypeId != KOUKU_ARCHETYPE)
		return {};
	return L"Prototype_Component_Model_KoukuSaydon_MN_RPCZ_00";
}

const wchar_t*
Client::CKoukuSaydonPresentationAssetService::Get_GameObjectPrototypeTag()
{
	return KOUKU_OBJECT_PROTOTYPE;
}

HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) || archetypeId != KOUKU_ARCHETYPE)
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	if (ready.contains(std::string(archetypeId)))
		return S_FALSE;

	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	if (nullptr == actor || actor->clientPresentationId != KOUKU_PRESENTATION ||
		!actor->weaponModel.empty() || 0.f != actor->weaponModelPreScale)
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

	std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> bindings;
	std::string bindingStatus;
	if (!Load_PresentationBindings(*body, bindings, bindingStatus))
	{
		// A missing action document must not remove the boss body and other tools.
		bindingStatus = "Animation bindings unavailable; boss body remains usable: " + bindingStatus;
		OutputDebugStringA(("[KoukuSaydonPresentation] " + bindingStatus + "\n").c_str());
	}

	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(Get_ModelPrototypeTag(archetypeId), std::move(body));
	staged.emplace_back(KOUKU_OBJECT_PROTOTYPE, CNpc::Create(pDevice, pContext));
	for (const auto& [tag, prototype] : staged)
	{
		(void)tag;
		if (nullptr == prototype)
			return Reject("KoukuSaydon presentation prototype creation failed.");
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("KoukuSaydon presentation prototype commit failed.");

	g_ActionPresentations = std::move(bindings);
	ready.insert(std::string(archetypeId));
	g_Status = std::move(bindingStatus);
	return S_OK;
}

bool_t Client::CKoukuSaydonPresentationAssetService::Try_Resolve_Action(
	const std::string_view actionId,
	KOUKU_SAYDON_ACTION_PRESENTATION& outPresentation)
{
	std::scoped_lock lock{ g_KoukuAssetMutex };
	const auto found = g_ActionPresentations.find(std::string(actionId));
	if (found == g_ActionPresentations.end())
		return false;
	outPresentation = found->second;
	return true;
}

const std::string&
Client::CKoukuSaydonPresentationAssetService::Get_Status()
{
	return g_Status;
}
