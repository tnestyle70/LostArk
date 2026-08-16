#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

NS_BEGIN(Client)

struct ANIMATION_PREVIEW_ASSET;
struct CHARACTER_ACTOR_ENTRY;
struct CHARACTER_SPEC;

enum class PLAYABLE_PREVIEW_OWNER_KIND : uint8_t
{
	CHARACTER,
	END
};

/* A validated join of the three existing presentation authorities used by
   Model View: selector asset, ActorCatalog resource inventory, and the exact
   CHARACTER_SPEC consumed by CCharacter::Ready_PartObjects. No part inventory
   is copied into a second preview-only format. */
struct PLAYABLE_CHARACTER_PREVIEW_COMPOSITION final
{
	const CHARACTER_SPEC* pSpec = nullptr;
	const CHARACTER_ACTOR_ENTRY* pActor = nullptr;
	PLAYABLE_PREVIEW_OWNER_KIND eOwnerKind =
		PLAYABLE_PREVIEW_OWNER_KIND::END;
	uint32_t iBodyPalettePartCount = 0u;
	uint32_t iBodySocketPartCount = 0u;
	LostArk::Shared::PLAYER_STANCE_ID eFallbackStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	uint32_t iFallbackVisibleWeaponPartCount = 0u;
};

class CPlayableCharacterPreviewContract final
{
public:
	static bool_t Stage(
		const ANIMATION_PREVIEW_ASSET& Asset,
		PLAYABLE_CHARACTER_PREVIEW_COMPOSITION& OutComposition);

	/* The explicit-source overload is the same validator used after catalog
	   lookup. It also makes failed joins testable without mutating global actor
	   catalogs; OutComposition is unchanged on failure. */
	static bool_t Stage(
		const ANIMATION_PREVIEW_ASSET& Asset,
		const CHARACTER_SPEC& Spec,
		const CHARACTER_ACTOR_ENTRY& Actor,
		PLAYABLE_CHARACTER_PREVIEW_COMPOSITION& OutComposition);
};

NS_END
