#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <optional>
#include <string>

NS_BEGIN(Client)

/* The instrument a class plays while the Server holds SQUAREHOLE_SONG. Retail equips one
IT_<class>_<name>_00 item for the whole clip and unequips it on the musicend stage; the socket it
rides is the body mesh's `sc_prop3_01`, which names the bone `bip001-prop3` and adds no offset.
The offsets below start at zero and are tuned in Data/Actors/SquareHoleInstruments.json, which is
read once per Client start. */
struct SQUAREHOLE_INSTRUMENT
{
	std::string strModelAssetId;
	std::string strSocketBone;
	float3_t vPositionMeters{};
	float3_t vRotationDegrees{};
};

class CSquareHoleInstrumentCatalog final
{
public:
	/* The part key the Character files the instrument under; its visibility is owned by the
	song action, not by the weapon or identity rules the other equipment follows. */
	static constexpr const tchar_t* PART_TAG = TEXT("Part_95_SquareHoleInstrument");

	/* nullopt when the class has no entry or the document is unreadable. The document is parsed
	on first use and never again, so a failure is reported once and leaves every class without
	an instrument instead of failing a Character. */
	static std::optional<SQUAREHOLE_INSTRUMENT> Find(LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass);

	/* Prototype tag of the class's instrument model in a level's prototype registry. */
	static std::wstring Get_ModelTag(LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass);
};

NS_END
