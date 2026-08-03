```cpp
// FILE: Shared/Public/Raid/ValtanContracts.h

#pragma once

#include "Gameplay/GameplayContracts.h"

#include <cstdint>

namespace LostArk::Shared
{
	enum class VALTAN_PHASE : std::uint8_t
	{
		INTRO,
		PHASE1_ARMORED,
		ARMOR_BREAK_WINDOW,
		PHASE1_UNARMORED,
		WIPE_AT_130,
		PHASE2,
		DEAD,
		END
	};

	enum class VALTAN_PATTERN_ID : std::uint32_t
	{
		NONE = 0,
		FRONT_SLAM = 3001,
		CHARGE = 3002,
		SPIN = 3003,
		WIPE = 3004
	};

	enum class SIDEREAL_SKILL_ID : std::uint8_t
	{
		THIRAIN,
		WEI,
		BALTHORR,
		END
	};

	struct BOSS_SNAPSHOT
	{
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iHp = 0;
		std::uint32_t iMaxHp = 0;
		std::uint8_t iHpBars = 0;
		VALTAN_PHASE ePhase = VALTAN_PHASE::END;
		std::uint32_t iArmorDurability = 0;
		std::uint32_t iMaxArmorDurability = 0;
		std::uint32_t iStagger = 0;
		std::uint32_t iMaxStagger = 0;
		VALTAN_PATTERN_ID ePatternId = VALTAN_PATTERN_ID::NONE;
		std::uint32_t iPatternStartTick = 0;
		PLAYER_ID iTargetPlayerId = INVALID_PLAYER_ID;
		float fLockedDirectionX = 0.f;
		float fLockedDirectionZ = 1.f;
		bool isInvulnerable = false;
		bool isCounterWindowOpen = false;
		std::uint32_t iSiderealGauge = 0;
	};

	struct S2C_BOSS_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		BOSS_SNAPSHOT Boss;
	};

	enum class BOSS_EVENT_TYPE : std::uint8_t
	{
		PATTERN_STARTED,
		CHARGE_DIRECTION_LOCKED,
		CHARGE_HIT_WALL,
		ARMOR_DAMAGED,
		ARMOR_BROKEN,
		STAGGER_CHANGED,
		STAGGERED,
		COUNTER_SUCCEEDED,
		WIPE_STARTED,
		SIDEREAL_ACTIVATED,
		BOSS_DIED,
		END
	};

	struct BOSS_EVENT
	{
		std::uint32_t iEventId = 0;
		std::uint32_t iServerTick = 0;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		BOSS_EVENT_TYPE eType = BOSS_EVENT_TYPE::END;
		VALTAN_PATTERN_ID ePatternId = VALTAN_PATTERN_ID::NONE;
		PLAYER_ID iActorPlayerId = INVALID_PLAYER_ID;
		std::uint32_t iValue = 0;
		float fDirectionX = 0.f;
		float fDirectionZ = 1.f;
	};

	struct S2C_BOSS_EVENT
	{
		BOSS_EVENT Event;
	};

	struct C2S_SIDEREAL
	{
		std::uint32_t iSequence = 0;
		SIDEREAL_SKILL_ID eSkillId = SIDEREAL_SKILL_ID::END;
	};
}
```

```cpp
// FILE: Shared/Public/Raid/ValtanMessages.h

#pragma once

#include "Raid/ValtanContracts.h"

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;
	bool Write_Message(CPacketWriter& writer, const C2S_SIDEREAL& message);
	bool Read_Message(CPacketReader& reader, C2S_SIDEREAL& message);
	bool Write_Message(CPacketWriter& writer, const S2C_BOSS_SNAPSHOT& message);
	bool Read_Message(CPacketReader& reader, S2C_BOSS_SNAPSHOT& message);
	bool Write_Message(CPacketWriter& writer, const S2C_BOSS_EVENT& message);
	bool Read_Message(CPacketReader& reader, S2C_BOSS_EVENT& message);
}
```

```cpp
// FILE: Shared/Public/Network/PacketType.h
// ADD ENUMERATORS AND Is_Known_Packet_Type() CASES

C2S_SIDEREAL,
S2C_BOSS_SNAPSHOT,
S2C_BOSS_EVENT,
```

```cpp
// FILE: Shared/Private/Raid/ValtanMessages.cpp

#include "Raid/ValtanMessages.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <cmath>

namespace
{
	bool Is_Valid(const LostArk::Shared::BOSS_SNAPSHOT& boss)
	{
		using namespace LostArk::Shared;
		return boss.iBossNetEntityId != INVALID_NET_ENTITY_ID &&
			boss.iMaxHp > 0 && boss.iHp <= boss.iMaxHp && boss.iHpBars > 0 &&
			static_cast<std::uint8_t>(boss.ePhase) < static_cast<std::uint8_t>(VALTAN_PHASE::END) &&
			boss.iArmorDurability <= boss.iMaxArmorDurability &&
			boss.iStagger <= boss.iMaxStagger &&
			std::isfinite(boss.fLockedDirectionX) && std::isfinite(boss.fLockedDirectionZ) &&
			boss.iSiderealGauge <= 1000;
	}

	void Write_Boss(LostArk::Shared::CPacketWriter& writer,
		const LostArk::Shared::BOSS_SNAPSHOT& boss)
	{
		writer.Write_U32(boss.iBossNetEntityId); writer.Write_U32(boss.iHp);
		writer.Write_U32(boss.iMaxHp); writer.Write_U8(boss.iHpBars);
		writer.Write_U8(static_cast<std::uint8_t>(boss.ePhase));
		writer.Write_U32(boss.iArmorDurability); writer.Write_U32(boss.iMaxArmorDurability);
		writer.Write_U32(boss.iStagger); writer.Write_U32(boss.iMaxStagger);
		writer.Write_U32(static_cast<std::uint32_t>(boss.ePatternId));
		writer.Write_U32(boss.iPatternStartTick); writer.Write_U32(boss.iTargetPlayerId);
		writer.Write_F32(boss.fLockedDirectionX); writer.Write_F32(boss.fLockedDirectionZ);
		writer.Write_U8(boss.isInvulnerable ? 1u : 0u);
		writer.Write_U8(boss.isCounterWindowOpen ? 1u : 0u);
		writer.Write_U32(boss.iSiderealGauge);
	}
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_SIDEREAL& message)
{
	if (0 == message.iSequence ||
		static_cast<std::uint8_t>(message.eSkillId) >= static_cast<std::uint8_t>(SIDEREAL_SKILL_ID::END)) return false;
	writer.Write_U32(message.iSequence); writer.Write_U8(static_cast<std::uint8_t>(message.eSkillId)); return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_SIDEREAL& message)
{
	C2S_SIDEREAL decoded{}; std::uint8_t skill = 0;
	if (!reader.Read_U32(decoded.iSequence) || !reader.Read_U8(skill) || 0 == decoded.iSequence ||
		skill >= static_cast<std::uint8_t>(SIDEREAL_SKILL_ID::END)) return false;
	decoded.eSkillId = static_cast<SIDEREAL_SKILL_ID>(skill); message = decoded; return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_BOSS_SNAPSHOT& message)
{
	if (0 == message.iServerTick || !Is_Valid(message.Boss)) return false;
	writer.Write_U32(message.iServerTick); Write_Boss(writer, message.Boss); return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_BOSS_SNAPSHOT& message)
{
	S2C_BOSS_SNAPSHOT value{}; std::uint8_t phase = 0, invulnerable = 0, counter = 0; std::uint32_t pattern = 0;
	if (!reader.Read_U32(value.iServerTick) || !reader.Read_U32(value.Boss.iBossNetEntityId) ||
		!reader.Read_U32(value.Boss.iHp) || !reader.Read_U32(value.Boss.iMaxHp) ||
		!reader.Read_U8(value.Boss.iHpBars) || !reader.Read_U8(phase) ||
		!reader.Read_U32(value.Boss.iArmorDurability) || !reader.Read_U32(value.Boss.iMaxArmorDurability) ||
		!reader.Read_U32(value.Boss.iStagger) || !reader.Read_U32(value.Boss.iMaxStagger) ||
		!reader.Read_U32(pattern) || !reader.Read_U32(value.Boss.iPatternStartTick) ||
		!reader.Read_U32(value.Boss.iTargetPlayerId) || !reader.Read_F32(value.Boss.fLockedDirectionX) ||
		!reader.Read_F32(value.Boss.fLockedDirectionZ) || !reader.Read_U8(invulnerable) ||
		!reader.Read_U8(counter) || !reader.Read_U32(value.Boss.iSiderealGauge) ||
		invulnerable > 1u || counter > 1u) return false;
	value.Boss.ePhase = static_cast<VALTAN_PHASE>(phase);
	value.Boss.ePatternId = static_cast<VALTAN_PATTERN_ID>(pattern);
	value.Boss.isInvulnerable = 1u == invulnerable;
	value.Boss.isCounterWindowOpen = 1u == counter;
	if (0 == value.iServerTick || !Is_Valid(value.Boss)) return false;
	message = value; return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_BOSS_EVENT& message)
{
	const BOSS_EVENT& event = message.Event;
	if (0 == event.iEventId || 0 == event.iServerTick ||
		event.iBossNetEntityId == INVALID_NET_ENTITY_ID ||
		static_cast<std::uint8_t>(event.eType) >= static_cast<std::uint8_t>(BOSS_EVENT_TYPE::END) ||
		!std::isfinite(event.fDirectionX) || !std::isfinite(event.fDirectionZ)) return false;
	writer.Write_U32(event.iEventId); writer.Write_U32(event.iServerTick);
	writer.Write_U32(event.iBossNetEntityId); writer.Write_U8(static_cast<std::uint8_t>(event.eType));
	writer.Write_U32(static_cast<std::uint32_t>(event.ePatternId));
	writer.Write_U32(event.iActorPlayerId); writer.Write_U32(event.iValue);
	writer.Write_F32(event.fDirectionX); writer.Write_F32(event.fDirectionZ); return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_BOSS_EVENT& message)
{
	BOSS_EVENT event{}; std::uint8_t type = 0; std::uint32_t pattern = 0;
	if (!reader.Read_U32(event.iEventId) || !reader.Read_U32(event.iServerTick) ||
		!reader.Read_U32(event.iBossNetEntityId) || !reader.Read_U8(type) ||
		!reader.Read_U32(pattern) || !reader.Read_U32(event.iActorPlayerId) ||
		!reader.Read_U32(event.iValue) || !reader.Read_F32(event.fDirectionX) ||
		!reader.Read_F32(event.fDirectionZ)) return false;
	event.eType = static_cast<BOSS_EVENT_TYPE>(type);
	event.ePatternId = static_cast<VALTAN_PATTERN_ID>(pattern);
	S2C_BOSS_EVENT decoded{}; decoded.Event = event;
	CPacketWriter validator; if (!Write_Message(validator, decoded)) return false;
	message = decoded; return true;
}
```

```cpp
// FILE: Server/Public/ServerValtan.h

#pragma once

#include "Raid/ValtanContracts.h"

namespace LostArk::Server
{
	struct SERVER_VALTAN
	{
		LostArk::Shared::BOSS_SNAPSHOT Snapshot;
		std::uint32_t iNextPatternTick = 0;
		std::uint32_t iPatternEndTick = 0;
		std::uint32_t iLastSiderealSequence = 0;
		bool hasEmittedArmorBroken = false;
		bool hasEmittedWipe = false;
		bool hasEmittedDeath = false;
	};
}
```

```cpp
// FILE: Server/Public/EncounterContext.h

#pragma once

#include "Raid/ValtanContracts.h"

namespace LostArk::Server
{
	class IEncounterContext
	{
	public:
		virtual ~IEncounterContext() = default;
		virtual std::uint32_t Get_ServerTick() const = 0;
		virtual LostArk::Shared::PLAYER_ID Select_LivingTarget() const = 0;
		virtual bool Emit_BossEvent(const LostArk::Shared::BOSS_EVENT& event) = 0;
	};
}
```

```cpp
// FILE: Server/Public/ValtanEncounter.h

#pragma once

#include "EncounterContext.h"
#include "ServerValtan.h"

namespace LostArk::Server
{
	class CValtanEncounter final
	{
	public:
		void Initialize(SERVER_VALTAN& boss, LostArk::Shared::NET_ENTITY_ID entityId,
			std::uint32_t serverTick) const;
		void Tick(SERVER_VALTAN& boss, IEncounterContext& context) const;
		bool Apply_Damage(SERVER_VALTAN& boss, IEncounterContext& context,
			LostArk::Shared::PLAYER_ID sourcePlayerId, std::uint32_t damage,
			std::uint32_t armorDamage, std::uint32_t staggerDamage) const;
		bool Request_Sidereal(SERVER_VALTAN& boss, IEncounterContext& context,
			LostArk::Shared::PLAYER_ID requesterPlayerId, bool isRaidLeader,
			const LostArk::Shared::C2S_SIDEREAL& command) const;
	};
}
```

```cpp
// FILE: Server/Private/ValtanEncounter.cpp

#include "ValtanEncounter.h"

#include <algorithm>

void LostArk::Server::CValtanEncounter::Initialize(
	SERVER_VALTAN& boss, LostArk::Shared::NET_ENTITY_ID entityId,
	std::uint32_t serverTick) const
{
	boss = {};
	boss.Snapshot.iBossNetEntityId = entityId;
	boss.Snapshot.iHp = 10000; boss.Snapshot.iMaxHp = 10000;
	boss.Snapshot.iHpBars = 160;
	boss.Snapshot.ePhase = LostArk::Shared::VALTAN_PHASE::INTRO;
	boss.Snapshot.iArmorDurability = 2000; boss.Snapshot.iMaxArmorDurability = 2000;
	boss.Snapshot.iMaxStagger = 1000;
	boss.Snapshot.isInvulnerable = true;
	boss.iNextPatternTick = serverTick + 60;
}

void LostArk::Server::CValtanEncounter::Tick(
	SERVER_VALTAN& boss, IEncounterContext& context) const
{
	using namespace LostArk::Shared;
	auto& state = boss.Snapshot;
	const std::uint32_t tick = context.Get_ServerTick();
	if (0 == state.iHp)
	{
		state.ePhase = VALTAN_PHASE::DEAD; state.ePatternId = VALTAN_PATTERN_ID::NONE;
		state.isInvulnerable = true;
		if (!boss.hasEmittedDeath)
		{
			boss.hasEmittedDeath = true;
			BOSS_EVENT event{}; event.eType = BOSS_EVENT_TYPE::BOSS_DIED;
			event.iBossNetEntityId = state.iBossNetEntityId; context.Emit_BossEvent(event);
		}
		return;
	}

	if (state.ePhase == VALTAN_PHASE::INTRO && tick >= boss.iNextPatternTick)
	{
		state.ePhase = VALTAN_PHASE::PHASE1_ARMORED;
		state.isInvulnerable = false;
		boss.iNextPatternTick = tick + 90;
	}
	if (0 == state.iArmorDurability && !boss.hasEmittedArmorBroken)
	{
		boss.hasEmittedArmorBroken = true;
		state.ePhase = VALTAN_PHASE::PHASE1_UNARMORED;
		BOSS_EVENT event{}; event.eType = BOSS_EVENT_TYPE::ARMOR_BROKEN;
		event.iBossNetEntityId = state.iBossNetEntityId; context.Emit_BossEvent(event);
	}
	if (state.iHp <= 5000 && state.ePhase < VALTAN_PHASE::PHASE2)
		state.ePhase = VALTAN_PHASE::PHASE2;
	if (state.iHp <= 1300 && !boss.hasEmittedWipe)
	{
		boss.hasEmittedWipe = true; state.ePhase = VALTAN_PHASE::WIPE_AT_130;
		state.ePatternId = VALTAN_PATTERN_ID::WIPE; state.iPatternStartTick = tick;
		BOSS_EVENT event{}; event.eType = BOSS_EVENT_TYPE::WIPE_STARTED;
		event.iBossNetEntityId = state.iBossNetEntityId;
		event.ePatternId = VALTAN_PATTERN_ID::WIPE; context.Emit_BossEvent(event);
	}

	if (tick >= boss.iPatternEndTick) state.ePatternId = VALTAN_PATTERN_ID::NONE;
	if (state.ePhase != VALTAN_PHASE::WIPE_AT_130 && tick >= boss.iNextPatternTick)
	{
		state.ePatternId = state.ePhase == VALTAN_PHASE::PHASE2 ?
			VALTAN_PATTERN_ID::SPIN : VALTAN_PATTERN_ID::CHARGE;
		state.iPatternStartTick = tick; state.iTargetPlayerId = context.Select_LivingTarget();
		boss.iPatternEndTick = tick + 45; boss.iNextPatternTick = tick + 120;
		BOSS_EVENT event{}; event.eType = BOSS_EVENT_TYPE::PATTERN_STARTED;
		event.iBossNetEntityId = state.iBossNetEntityId;
		event.ePatternId = state.ePatternId; context.Emit_BossEvent(event);
	}
}

bool LostArk::Server::CValtanEncounter::Apply_Damage(
	SERVER_VALTAN& boss, IEncounterContext& context,
	LostArk::Shared::PLAYER_ID sourcePlayerId, std::uint32_t damage,
	std::uint32_t armorDamage, std::uint32_t staggerDamage) const
{
	using namespace LostArk::Shared;
	auto& state = boss.Snapshot;
	if (state.isInvulnerable || 0 == state.iHp || sourcePlayerId == INVALID_PLAYER_ID) return false;
	state.iHp -= (std::min)(state.iHp, damage);
	state.iArmorDurability -= (std::min)(state.iArmorDurability, armorDamage);
	state.iStagger = (std::min)(state.iMaxStagger, state.iStagger + staggerDamage);
	BOSS_EVENT armor{}; armor.eType = BOSS_EVENT_TYPE::ARMOR_DAMAGED;
	armor.iBossNetEntityId = state.iBossNetEntityId; armor.iActorPlayerId = sourcePlayerId;
	armor.iValue = state.iArmorDurability; context.Emit_BossEvent(armor);
	BOSS_EVENT stagger{}; stagger.eType = BOSS_EVENT_TYPE::STAGGER_CHANGED;
	stagger.iBossNetEntityId = state.iBossNetEntityId; stagger.iActorPlayerId = sourcePlayerId;
	stagger.iValue = state.iStagger; context.Emit_BossEvent(stagger);
	if (state.iStagger == state.iMaxStagger)
	{
		BOSS_EVENT event{}; event.eType = BOSS_EVENT_TYPE::STAGGERED;
		event.iBossNetEntityId = state.iBossNetEntityId; context.Emit_BossEvent(event);
		state.iStagger = 0;
	}
	return true;
}

bool LostArk::Server::CValtanEncounter::Request_Sidereal(
	SERVER_VALTAN& boss, IEncounterContext& context,
	LostArk::Shared::PLAYER_ID requesterPlayerId, bool isRaidLeader,
	const LostArk::Shared::C2S_SIDEREAL& command) const
{
	using namespace LostArk::Shared;
	if (!isRaidLeader || 0 == command.iSequence || command.iSequence <= boss.iLastSiderealSequence ||
		boss.Snapshot.iSiderealGauge < 1000) return false;
	boss.iLastSiderealSequence = command.iSequence;
	boss.Snapshot.iSiderealGauge = 0;
	BOSS_EVENT event{}; event.eType = BOSS_EVENT_TYPE::SIDEREAL_ACTIVATED;
	event.iBossNetEntityId = boss.Snapshot.iBossNetEntityId;
	event.iActorPlayerId = requesterPlayerId;
	event.iValue = static_cast<std::uint32_t>(command.eSkillId);
	return context.Emit_BossEvent(event);
}
```

```cpp
// FILE: Client/Public/RaidHUDViewModel.h

#pragma once

#include "Raid/ValtanContracts.h"

namespace Client
{
	class CRaidHUDViewModel final
	{
	public:
		bool Apply(const LostArk::Shared::S2C_BOSS_SNAPSHOT& snapshot);
		bool Apply(const LostArk::Shared::S2C_BOSS_EVENT& event);
		void Reset();
		const LostArk::Shared::BOSS_SNAPSHOT* Get_Boss() const;
	private:
		LostArk::Shared::BOSS_SNAPSHOT m_Boss;
		std::uint32_t m_iLastServerTick = 0;
		std::uint32_t m_iLastEventId = 0;
		bool m_hasBoss = false;
	};
}
```

```cpp
// FILE: Client/Private/RaidHUDViewModel.cpp

#include "RaidHUDViewModel.h"

bool Client::CRaidHUDViewModel::Apply(
	const LostArk::Shared::S2C_BOSS_SNAPSHOT& snapshot)
{
	if (0 == snapshot.iServerTick || snapshot.iServerTick <= m_iLastServerTick)
		return false;
	m_Boss = snapshot.Boss;
	m_iLastServerTick = snapshot.iServerTick;
	m_hasBoss = true;
	return true;
}

bool Client::CRaidHUDViewModel::Apply(
	const LostArk::Shared::S2C_BOSS_EVENT& event)
{
	if (0 == event.Event.iEventId || event.Event.iEventId <= m_iLastEventId)
		return false;
	m_iLastEventId = event.Event.iEventId;
	return true;
}

void Client::CRaidHUDViewModel::Reset()
{
	m_Boss = {};
	m_iLastServerTick = 0;
	m_iLastEventId = 0;
	m_hasBoss = false;
}

const LostArk::Shared::BOSS_SNAPSHOT*
Client::CRaidHUDViewModel::Get_Boss() const
{
	return m_hasBoss ? &m_Boss : nullptr;
}
```

```cpp
// FILE: Client/Public/Valtan.h
// ADD NETWORK PRESENTATION API/MEMBERS; NETWORK MODE BYPASSES LOCAL NavPathFollower

void Apply_BossSnapshot(const LostArk::Shared::BOSS_SNAPSHOT& snapshot);
void Apply_BossEvent(const LostArk::Shared::BOSS_EVENT& event);

bool_t m_hasNetworkState = false;
LostArk::Shared::BOSS_SNAPSHOT m_NetworkSnapshot;
std::uint32_t m_iLastBossEventId = 0;
```

```cpp
// FILE: Client/Private/Valtan.cpp
// ADD AT START OF Update()

if (m_hasNetworkState)
{
	if (m_NetworkSnapshot.ePhase == LostArk::Shared::VALTAN_PHASE::DEAD)
		Set_ChaseState(false);
	__super::Update(fTimeDelta);
	return;
}
```

```cpp
// FILE: Client/Private/Valtan.cpp
// APPEND

void CValtan::Apply_BossSnapshot(const LostArk::Shared::BOSS_SNAPSHOT& snapshot)
{
	m_hasNetworkState = true;
	m_NetworkSnapshot = snapshot;
}

void CValtan::Apply_BossEvent(const LostArk::Shared::BOSS_EVENT& event)
{
	if (event.iEventId <= m_iLastBossEventId) return;
	m_iLastBossEventId = event.iEventId;
	if (event.eType == LostArk::Shared::BOSS_EVENT_TYPE::PATTERN_STARTED)
	{
		if (event.ePatternId == LostArk::Shared::VALTAN_PATTERN_ID::CHARGE)
			m_pBodyModelCom->Set_Animation("charge", false);
		else if (event.ePatternId == LostArk::Shared::VALTAN_PATTERN_ID::SPIN)
			m_pBodyModelCom->Set_Animation("spin", false);
	}
	else if (event.eType == LostArk::Shared::BOSS_EVENT_TYPE::STAGGERED)
		m_pBodyModelCom->Set_Animation("stagger", false);
	else if (event.eType == LostArk::Shared::BOSS_EVENT_TYPE::BOSS_DIED)
		m_pBodyModelCom->Set_Animation("death", false);
}
```

```cpp
// FILE: Client/Public/ClientReplicationEvent.h AND NetworkManager.cpp

BOSS_SNAPSHOT,
BOSS_EVENT,
LostArk::Shared::S2C_BOSS_SNAPSHOT BossSnapshot;
LostArk::Shared::S2C_BOSS_EVENT BossEvent;

case PACKET_TYPE::S2C_BOSS_SNAPSHOT:
{
	S2C_BOSS_SNAPSHOT message{};
	if (!Read_Message(reader, message) || 0 != reader.Get_RemainingSize())
	{ m_iLastErrorCode.store(WSAEINVAL); return; }
	Client::CLIENT_REPLICATION_EVENT event{};
	event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::BOSS_SNAPSHOT;
	event.BossSnapshot = message;
	m_ReplicationEvents.push_back(std::move(event));
	break;
}

case PACKET_TYPE::S2C_BOSS_EVENT:
{
	S2C_BOSS_EVENT message{};
	if (!Read_Message(reader, message) || 0 != reader.Get_RemainingSize())
	{ m_iLastErrorCode.store(WSAEINVAL); return; }
	Client::CLIENT_REPLICATION_EVENT event{};
	event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::BOSS_EVENT;
	event.BossEvent = message;
	m_ReplicationEvents.push_back(std::move(event));
	break;
}
```

```cpp
// FILE: Client/Public/ClientReplication.h

#include "RaidHUDViewModel.h"

void Set_Valtan(const std::shared_ptr<CValtan>& valtan);
bool Apply_BossSnapshot(const LostArk::Shared::S2C_BOSS_SNAPSHOT& snapshot);
bool Apply_BossEvent(const LostArk::Shared::S2C_BOSS_EVENT& event);

std::weak_ptr<CValtan> m_pValtan;
CRaidHUDViewModel m_RaidHUDViewModel;
```

```cpp
// FILE: Client/Private/ClientReplication.cpp

case CLIENT_REPLICATION_EVENT_TYPE::BOSS_SNAPSHOT:
	allSucceeded = Apply_BossSnapshot(event.BossSnapshot) && allSucceeded;
	break;
case CLIENT_REPLICATION_EVENT_TYPE::BOSS_EVENT:
	allSucceeded = Apply_BossEvent(event.BossEvent) && allSucceeded;
	break;

void Client::CClientReplication::Set_Valtan(const std::shared_ptr<CValtan>& valtan)
{
	m_pValtan = valtan;
}

bool Client::CClientReplication::Apply_BossSnapshot(
	const LostArk::Shared::S2C_BOSS_SNAPSHOT& snapshot)
{
	if (!m_RaidHUDViewModel.Apply(snapshot)) return false;
	const auto valtan = m_pValtan.lock();
	if (nullptr == valtan) return false;
	valtan->Apply_BossSnapshot(snapshot.Boss);
	return true;
}

bool Client::CClientReplication::Apply_BossEvent(
	const LostArk::Shared::S2C_BOSS_EVENT& event)
{
	if (!m_RaidHUDViewModel.Apply(event)) return false;
	const auto valtan = m_pValtan.lock();
	if (nullptr == valtan) return false;
	valtan->Apply_BossEvent(event.Event);
	return true;
}

// ADD TO Reset_World()
m_pValtan.reset();
m_RaidHUDViewModel.Reset();
```

```cpp
// FILE: Server/Public/GameRoom.h
// ADD IEncounterContext, MEMBERS, BROADCASTS

#include "EncounterContext.h"
#include "ServerValtan.h"
#include "ValtanEncounter.h"

SERVER_VALTAN m_Valtan;
CValtanEncounter m_ValtanEncounter;
std::uint32_t m_iNextBossEventId = 1;

bool Broadcast_BossSnapshot();
bool Emit_BossEvent(const LostArk::Shared::BOSS_EVENT& event) override;
LostArk::Shared::PLAYER_ID Select_LivingTarget() const override;
void Begin_ValtanEncounter();
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// ADD TO 30 HZ Tick()

m_ValtanEncounter.Tick(m_Valtan, *this);
Broadcast_BossSnapshot();
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// CALL WHEN A ZONE_TRANSITION_COMMIT_EVENT TARGETS VALTAN_ARENA

void LostArk::Server::CGameRoom::Begin_ValtanEncounter()
{
	if (m_Valtan.Snapshot.iBossNetEntityId !=
		LostArk::Shared::INVALID_NET_ENTITY_ID) return;
	if (m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		++m_iNextNetEntityId;
	m_ValtanEncounter.Initialize(
		m_Valtan,
		m_iNextNetEntityId++,
		m_iServerTick);
}
```

```cpp
// FILE: Server/Private/GameRoom.cpp

bool LostArk::Server::CGameRoom::Broadcast_BossSnapshot()
{
	using namespace LostArk::Shared;
	if (m_Valtan.Snapshot.iBossNetEntityId == INVALID_NET_ENTITY_ID) return true;
	S2C_BOSS_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.Boss = m_Valtan.Snapshot;
	CPacketWriter writer; if (!Write_Message(writer, message)) return false;
	bool success = true;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const auto session = Find_Session(sessionId);
		if (nullptr == session || !session->Send_Frame(
			PACKET_TYPE::S2C_BOSS_SNAPSHOT, writer.Get_Buffer())) success = false;
	}
	return success;
}

bool LostArk::Server::CGameRoom::Emit_BossEvent(
	const LostArk::Shared::BOSS_EVENT& source)
{
	using namespace LostArk::Shared;
	if (0 == m_iNextBossEventId) ++m_iNextBossEventId;
	S2C_BOSS_EVENT message{};
	message.Event = source;
	message.Event.iEventId = m_iNextBossEventId++;
	message.Event.iServerTick = m_iServerTick;
	if (message.Event.iBossNetEntityId == INVALID_NET_ENTITY_ID)
		message.Event.iBossNetEntityId = m_Valtan.Snapshot.iBossNetEntityId;
	CPacketWriter writer; if (!Write_Message(writer, message)) return false;
	bool success = true;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const auto session = Find_Session(sessionId);
		if (nullptr == session || !session->Send_Frame(
			PACKET_TYPE::S2C_BOSS_EVENT, writer.Get_Buffer())) success = false;
	}
	return success;
}

LostArk::Shared::PLAYER_ID
LostArk::Server::CGameRoom::Select_LivingTarget() const
{
	for (const auto& [playerId, player] : m_Players)
		if (player.iHp > 0) return playerId;
	return LostArk::Shared::INVALID_PLAYER_ID;
}
```

```cpp
// FILE: Server/Public/RoomCommand.h AND ServerApp.cpp

SIDEREAL,
LostArk::Shared::C2S_SIDEREAL Sidereal;

case PACKET_TYPE::C2S_SIDEREAL:
{
	C2S_SIDEREAL commandMessage{};
	if (!Read_Message(reader, commandMessage) || 0 != reader.Get_RemainingSize())
	{ Request_SessionClose(sessionId); return; }
	command.eType = ROOM_COMMAND_TYPE::SIDEREAL;
	command.Sidereal = commandMessage;
	break;
}
```

```cpp
// FILE: Server/Private/GameRoom.cpp

case ROOM_COMMAND_TYPE::SIDEREAL:
{
	const auto playerId = m_PlayerIdBySessionId.find(command.iSessionId);
	if (playerId == m_PlayerIdBySessionId.end()) break;
	const bool isRaidLeader = m_PartySystem.Is_Leader(playerId->second);
	m_ValtanEncounter.Request_Sidereal(
		m_Valtan, *this, playerId->second, isRaidLeader, command.Sidereal);
	break;
}
```

```xml
<!-- PROJECT ITEMS -->
<ClInclude Include="..\Public\Raid\ValtanContracts.h" />
<ClInclude Include="..\Public\Raid\ValtanMessages.h" />
<ClCompile Include="..\Private\Raid\ValtanMessages.cpp" />
<ClInclude Include="..\Public\ServerValtan.h" />
<ClInclude Include="..\Public\EncounterContext.h" />
<ClInclude Include="..\Public\ValtanEncounter.h" />
<ClCompile Include="..\Private\ValtanEncounter.cpp" />
<ClInclude Include="..\Public\RaidHUDViewModel.h" />
<ClCompile Include="..\Private\RaidHUDViewModel.cpp" />
```

```xml
<!-- FILE: Shared/Default/Shared.vcxproj.filters -->
<Filter Include="Public\Raid"><UniqueIdentifier>{D83FC61E-F9E8-452B-B616-DB8A4B29F89E}</UniqueIdentifier></Filter>
<Filter Include="Private\Raid"><UniqueIdentifier>{390E56F6-C29D-48A9-938A-25FFCF941027}</UniqueIdentifier></Filter>
<ClInclude Include="..\Public\Raid\ValtanContracts.h"><Filter>Public\Raid</Filter></ClInclude>
<ClInclude Include="..\Public\Raid\ValtanMessages.h"><Filter>Public\Raid</Filter></ClInclude>
<ClCompile Include="..\Private\Raid\ValtanMessages.cpp"><Filter>Private\Raid</Filter></ClCompile>

<!-- FILE: Server/Default/Server.vcxproj.filters -->
<ClInclude Include="..\Public\ServerValtan.h"><Filter>Public</Filter></ClInclude>
<ClInclude Include="..\Public\EncounterContext.h"><Filter>Public</Filter></ClInclude>
<ClInclude Include="..\Public\ValtanEncounter.h"><Filter>Public</Filter></ClInclude>
<ClCompile Include="..\Private\ValtanEncounter.cpp"><Filter>Private</Filter></ClCompile>

<!-- FILE: Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\RaidHUDViewModel.h"><Filter>04. Network</Filter></ClInclude>
<ClCompile Include="..\Private\RaidHUDViewModel.cpp"><Filter>04. Network</Filter></ClCompile>
```

```text
two clients have identical boss hp/hpBars/phase/armor/stagger/pattern/tick
ArmorBroken eventId emitted once
Staggered eventId emitted once per full stagger cycle
PatternStarted precedes Effect/Camera/Sound consumers
non-leader Sidereal command rejected without event or gauge mutation
leader Sidereal command consumes gauge and emits once
WipeStarted emitted once at 130-line threshold
BossDied emitted once and prevents later pattern selection
clear/reward/return transition begins only after BossDied server truth
```
