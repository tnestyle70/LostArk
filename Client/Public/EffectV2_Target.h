#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Engine)
class CGameObject;
class CModel;
NS_END

NS_BEGIN(Client)

class CNpc;
class CValtan;

enum class EFFECT_V2_TARGET_KIND : uint8_t
{
	NONE,
	NPC,
	VALTAN
};

/* The entity a v2 effect follows. pKey is identity only for runtime maps;
   liveness always comes from pOwner. */
struct EFFECT_V2_TARGET final
{
	EFFECT_V2_TARGET_KIND eKind = EFFECT_V2_TARGET_KIND::NONE;
	std::weak_ptr<Engine::CGameObject> pOwner;
	const Engine::CGameObject* pKey = nullptr;

	bool_t Is_Valid() const
	{
		return EFFECT_V2_TARGET_KIND::NONE != eKind && !pOwner.expired();
	}
	void Reset()
	{
		eKind = EFFECT_V2_TARGET_KIND::NONE;
		pOwner.reset();
		pKey = nullptr;
	}

	static EFFECT_V2_TARGET From_Npc(const std::shared_ptr<CNpc>& pNpc);
	static EFFECT_V2_TARGET From_Valtan(const std::shared_ptr<CValtan>& pValtan);
};

/* BoneRoot maps CModel::Get_BoneMatrix into world space (NPC: owner world;
   Valtan: body visual root x owner world). YawBasis is the owner world for
   PIVOT_ROTATION::TARGET_YAW on both kinds. */
struct EFFECT_V2_TARGET_VIEW final
{
	std::shared_ptr<Engine::CModel> pModel;
	float4x4_t BoneRoot{};
	float4x4_t YawBasis{};
};

NS_END
