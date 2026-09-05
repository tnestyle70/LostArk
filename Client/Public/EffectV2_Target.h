#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Engine)
class CGameObject;
class CModel;
NS_END

NS_BEGIN(Client)

class CNpc;
class CPart_Body;
class CValtan;

enum class EFFECT_V2_TARGET_KIND : uint8_t
{
	NONE,
	NPC,
	VALTAN,
	PREVIEW_BODY
};

/* The entity a v2 effect follows. pKey is identity only for runtime maps;
   liveness always comes from pOwner. strArchetypeId is the binding owner for
   PREVIEW_BODY only. */
struct EFFECT_V2_TARGET final
{
	EFFECT_V2_TARGET_KIND eKind = EFFECT_V2_TARGET_KIND::NONE;
	std::weak_ptr<Engine::CGameObject> pOwner;
	const Engine::CGameObject* pKey = nullptr;
	std::string strArchetypeId;

	bool_t Is_Valid() const
	{
		return EFFECT_V2_TARGET_KIND::NONE != eKind && !pOwner.expired();
	}
	void Reset()
	{
		eKind = EFFECT_V2_TARGET_KIND::NONE;
		pOwner.reset();
		pKey = nullptr;
		strArchetypeId.clear();
	}

	static EFFECT_V2_TARGET From_Npc(const std::shared_ptr<CNpc>& pNpc);
	static EFFECT_V2_TARGET From_Valtan(const std::shared_ptr<CValtan>& pValtan);
	static EFFECT_V2_TARGET From_PreviewBody(
		const std::shared_ptr<CPart_Body>& pBody,
		std::string strArchetypeId);
};

/* BoneRoot maps CModel::Get_BoneMatrix into world space (NPC: owner world;
   Valtan: body visual root x owner world; preview body: body transform).
   YawBasis is the owner world for PIVOT_ROTATION::TARGET_YAW on every kind. */
struct EFFECT_V2_TARGET_VIEW final
{
	std::shared_ptr<Engine::CModel> pModel;
	float4x4_t BoneRoot{};
	float4x4_t YawBasis{};
	bool_t bHasPortalRushRoute = false;
	float4x4_t PortalRushStart{};
	float4x4_t PortalRushEnd{};
};

NS_END
