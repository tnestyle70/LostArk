#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

struct KOUKU_SAYDON_ACTION_PRESENTATION final
{
	std::string strActionId;
	std::string strOccurrenceId;
	std::string strClip;
	std::uint32_t iPlayMs = 0u;
	f32_t fPlayRate = 1.f;
};

/* Loads the embedded-body presentation of every KoukuSaydon arena boss
(BOSS_KAKULSAYDON_* rows whose client contract is boss.kakulsaydon.*), with an
optional rest-pose weapon socketed on the body rig. Product animation bindings
are admitted for the Gate 1 Kouku only. It deliberately does not share
Valtan's armour prototype or joined presentation graph. */
/* The colourless Saydon body a player wears while PLAYER_MADNESS_FORM::CLOWN.
Shared with the clown CHARACTER_SPEC so the spec and the admission agree. */
inline constexpr const wchar_t* KOUKU_CLOWN_BODY_PROTOTYPE_TAG =
	L"Prototype_Component_Model_KoukuSaydonClown";

class CKoukuSaydonPresentationAssetService final
{
public:
	static void Begin_LevelLoad(std::uint32_t iLevelIndex);
	/* Admits the clown avatar body (MN_RPCT_03) once per level with the same
	Saydon admission scale and verifies its idle/run clips. S_FALSE when it
	is already ready; the failure reason lands in Get_Status(). */
	static HRESULT Ensure_ClownBodyPrototype(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::uint32_t iLevelIndex);
	/* True for a catalog row this service owns: the archetype prefix and the
	client presentation contract both name the KoukuSaydon family. */
	static bool_t Is_ArenaBossArchetype(std::string_view archetypeId);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::uint32_t iLevelIndex,
		std::string_view archetypeId);
	static std::wstring Get_ModelPrototypeTag(std::string_view archetypeId);
	/* Empty when the catalog row declares no weapon. */
	static std::wstring Get_WeaponModelPrototypeTag(std::string_view archetypeId);
	static const char_t* Get_WeaponSocketBone();
	static const wchar_t* Get_GameObjectPrototypeTag();
	/* Product action bindings admitted for one arena boss body. Every arena
	boss loads the same binding document and keeps only the rows whose clip
	its own body owns, so a Saydon pattern resolves on a Saydon body only. */
	static bool_t Try_Resolve_Action(
		std::string_view archetypeId,
		std::string_view actionId,
		KOUKU_SAYDON_ACTION_PRESENTATION& outPresentation);
	static const std::string& Get_Status();
};

NS_END
