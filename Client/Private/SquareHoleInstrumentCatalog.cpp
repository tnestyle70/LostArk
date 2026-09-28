#include <WinSock2.h>
#include "SquareHoleInstrumentCatalog.h"

#include "ActorCatalog.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <mutex>
#include <vector>

namespace
{
	struct ENTRY
	{
		std::string strCharacterAssetId;
		Client::SQUAREHOLE_INSTRUMENT Instrument;
	};

	std::mutex g_Mutex;
	bool g_isLoaded = false;
	std::vector<ENTRY> g_Entries;

	bool Read_Float3(const DATA_JSON_VALUE& row, const char* pKey, const float limit, float3_t& out)
	{
		const DATA_JSON_VALUE* pValue = row.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array() || 3u != pValue->Get_Array().size())
			return false;
		float values[3]{};
		for (size_t i = 0u; i < 3u; ++i)
		{
			const DATA_JSON_VALUE& item = pValue->Get_Array()[i];
			if (!item.Is_Number() || !std::isfinite(item.Get_Number()) || std::abs(item.Get_Number()) > limit)
				return false;
			values[i] = static_cast<float>(item.Get_Number());
		}
		out = float3_t(values[0], values[1], values[2]);
		return true;
	}

	/* Called under g_Mutex. Any invalid row rejects the whole document, so a typo can never
	leave one class with a half-read offset. */
	void Load_Locked()
	{
		g_isLoaded = true;
		g_Entries.clear();
		const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Actors/SquareHoleInstruments.json"));
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			OutputDebugStringA("[SquareHoleInstruments] document is missing; no instrument is shown.\n");
			return;
		}
		const std::string text{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE root;
		std::string error;
		const DATA_JSON_VALUE* pSchema = nullptr;
		const DATA_JSON_VALUE* pVersion = nullptr;
		const DATA_JSON_VALUE* pRows = nullptr;
		if (!CDataJson::Parse(text, root, error) || !root.Is_Object() ||
			nullptr == (pSchema = root.Find("schema")) || !pSchema->Is_String() ||
			"lostark.squarehole-instruments" != pSchema->Get_String() ||
			nullptr == (pVersion = root.Find("formatVersion")) || !pVersion->Is_Number() ||
			1.0 != pVersion->Get_Number() ||
			nullptr == (pRows = root.Find("instruments")) || !pRows->Is_Array())
		{
			OutputDebugStringA("[SquareHoleInstruments] document header is invalid; no instrument is shown.\n");
			return;
		}
		std::vector<ENTRY> staged;
		for (const DATA_JSON_VALUE& row : pRows->Get_Array())
		{
			const DATA_JSON_VALUE* pCharacter = row.Is_Object() ? row.Find("characterAssetId") : nullptr;
			const DATA_JSON_VALUE* pModel = row.Is_Object() ? row.Find("modelAssetId") : nullptr;
			const DATA_JSON_VALUE* pBone = row.Is_Object() ? row.Find("socketBone") : nullptr;
			ENTRY entry;
			if (nullptr == pCharacter || !pCharacter->Is_String() || pCharacter->Get_String().empty() ||
				nullptr == pModel || !pModel->Is_String() || pModel->Get_String().empty() ||
				nullptr == pBone || !pBone->Is_String() || pBone->Get_String().empty() ||
				!Read_Float3(row, "positionMeters", 10.f, entry.Instrument.vPositionMeters) ||
				!Read_Float3(row, "rotationDegrees", 360.f, entry.Instrument.vRotationDegrees))
			{
				OutputDebugStringA("[SquareHoleInstruments] an instrument row is invalid; no instrument is shown.\n");
				return;
			}
			entry.strCharacterAssetId = pCharacter->Get_String();
			entry.Instrument.strModelAssetId = pModel->Get_String();
			entry.Instrument.strSocketBone = pBone->Get_String();
			for (const ENTRY& other : staged)
				if (other.strCharacterAssetId == entry.strCharacterAssetId)
				{
					OutputDebugStringA("[SquareHoleInstruments] a character is listed twice; no instrument is shown.\n");
					return;
				}
			staged.push_back(std::move(entry));
		}
		g_Entries = std::move(staged);
	}
}

std::optional<Client::SQUAREHOLE_INSTRUMENT> Client::CSquareHoleInstrumentCatalog::Find(
	const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass)
{
	const CHARACTER_ACTOR_ENTRY* pActor = CActorCatalog::Find_Character(eCharacterClass);
	if (nullptr == pActor)
		return std::nullopt;
	std::scoped_lock lock{ g_Mutex };
	if (!g_isLoaded)
		Load_Locked();
	for (const ENTRY& entry : g_Entries)
		if (entry.strCharacterAssetId == pActor->assetId)
			return entry.Instrument;
	return std::nullopt;
}

std::wstring Client::CSquareHoleInstrumentCatalog::Get_ModelTag(
	const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass)
{
	return L"Prototype_Component_Model_SquareHoleInstrument_" +
		std::to_wstring(static_cast<uint32_t>(eCharacterClass));
}
