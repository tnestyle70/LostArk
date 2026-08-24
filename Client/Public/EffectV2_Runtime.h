#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CNpc;

class CEffectV2Runtime final
{
public:
	static void Notify_NpcClip(
		const std::shared_ptr<CNpc>& pNpc,
		const char_t* pClipName);
	static void Tick_Npc(
		const std::shared_ptr<CNpc>& pNpc,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Prewarm_Archetype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strArchetypeId);
	static void Set_Ignored(const std::shared_ptr<CNpc>& pNpc, bool_t bIgnored);
	static void Invalidate_Caches();
	static const std::string& Last_Error();
};

NS_END
