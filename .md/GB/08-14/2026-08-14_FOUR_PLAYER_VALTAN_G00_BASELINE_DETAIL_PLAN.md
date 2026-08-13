# 2026-08-14 4인 발탄 G00 기준선 디테일 계획

## G00-00. 목표와 종료 증거

G00은 현재 blocking socket transport를 유지한 채 다음 계약을 먼저 실행 가능한 기준선으로 만든다.

```text
Client 1~4 C2S_ENTER_WORLD
  -> 각 Client S2C_ENTER_ACCEPTED
  -> 각 Client가 playerCount=4 snapshot 관측

Client 5 C2S_ENTER_WORLD
  -> S2C_ENTER_REJECTED(VALTAN_ARENA, ROOM_FULL)
  -> connection close

Client 2 disconnect
  -> 나머지 Client가 playerCount=3 snapshot 관측
  -> replacement Client accepted
  -> 다시 playerCount=4 수렴

전원 disconnect
  -> trigger/spawn group/boss/monster/damage reset
  -> tick/PlayerId/NetEntityId/pending transfer는 단조 증가 또는 보존
  -> 두 번째 세대 네 명 accepted
```

종료 증거는 protocol codec harness, Server in-process contract, 실제 TCP live harness, World publisher,
Debug/Release build다. 발탄 HP bar의 화면 위치와 미감은 사용자의 수동 판정 전까지 PASS로 기록하지 않는다.

## G00-01. 변경 파일과 존재 이유

### Shared protocol

- `Shared/Public/Network/PacketType.h`: protocol version과 known packet type을 소유한다.
- `Shared/Public/Network/PacketMessages.h`: typed enter rejection enum/message/codec 선언을 소유한다.
- `Shared/Private/Network/PacketMessages.cpp`: rejection wire encoding과 transactional decoding을 소유한다.

### Server room

- `Server/Public/GameRoom.h`: admission rejection, replayable-room reset private 경계를 선언한다.
- `Server/Private/GameRoom.cpp`: 네 spawn admission, rejection send, empty reset을 room tick single writer에서 수행한다.
- `Server/Private/ServerGameplayContractTests.cpp`: map/allocator 불변식과 Valtan pristine reset을 실행형으로 검증한다.

### Client consumer와 HUD

- `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`: receive frame을 main-thread pending rejection으로 변환한다.
- `Client/Public/Level_Lobby.h`, `Client/Private/Level_Lobby.cpp`: pending rejection을 entry state와 대조하고 Lobby에 이유를 표시한다.
- `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`: boss snapshot을 단일 bar와 `current / max`로 표시한다.

### Publisher, harness, build

- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`: Valtan enabled player spawn이 정확히 네 개인지 publish 전에 검증한다.
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`: rejection codec 정상/실패/불변성 계약을 검증한다.
- `Tools/ValtanFourPlayerHarness/*`: 실제 TCP client 다섯 개와 두 세대를 구동하는 새 실행형 harness다.
- `Tools/Network/Run-ValtanFourPlayerHarness.ps1`: port 소유권, Server 실행, timeout, finally cleanup을 소유한다.
- `Framework.sln`: 새 C++ harness를 solution build graph에 등록한다.
- `Tools/Build/Invoke-BuildAndRegression.ps1`: Debug/Release 정본 회귀에서 live harness를 실행한다.
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`: protocol 18, generalized reset, Client rejection 소비, 단일 boss bar 정적 계약을 추적한다.

## G00-02. H 계약

### `ENTER_WORLD_REJECTION_REASON`

`ROOM_FULL`은 완전히 decode되고 world/class/nickname 검증까지 통과한 요청이 room의 enabled spawn을
더 이상 확보할 수 없음을 뜻한다. malformed frame과 protocol mismatch는 typed rejection이 아니라 기존처럼
연결을 fail-close한다. `END`는 저장하거나 wire로 보내는 정상값이 아닌 sentinel이다.

### `S2C_ENTER_REJECTED`

- `iProtocolVersion`: Client와 Server가 동일한 wire contract를 쓰는지 검증한다.
- `eWorldId`: 어느 pending world 요청이 거절됐는지 식별한다.
- `eReason`: 현재 G00에서는 `ROOM_FULL`만 정상이다.

Reader는 모든 field와 enum을 검증한 local `decoded`를 만든 뒤에만 output message를 교체한다.
truncated/unknown input은 호출자가 가진 이전 message를 변경하지 않는다.

### `CGameRoom`

새 private 함수의 책임은 다음과 같다.

- `Is_PlayerAdmissionFull`: authoring의 enabled player spawn 중 현재 player가 사용하지 않는 stable placement가 있는지만 판정한다.
- `Send_EnterRejected`: Shared codec으로 rejection frame을 만들고 해당 session에 전송한다.
- `Reset_ReplayableArenaWhenEmpty`: Character Select/Valtan의 마지막 player가 사라졌을 때 replayable state만 초기화한다.

`m_iServerTick`, `m_iNextPlayerId`, `m_iNextNetEntityId`, `m_PendingWorldTransfers`는 reset 대상이 아니다.
stale identity 재사용과 같은 tick에 이미 확정된 world transfer 유실을 막기 위한 불변식이다.

### `CNetworkManager`

`m_hasPendingEnterRejected`와 `m_PendingEnterRejected`는 receive worker가 아니라 main thread에서만 변경·소비되는
connection-scoped 상태다. connect 성공과 close 양쪽에서 accepted/rejected pending state를 대칭으로 지운다.

### `CLevel_Lobby`

`Consume_EnterRejected`는 `Consume_EnterAccepted`와 disconnect 확인보다 먼저 호출한다. Server가 rejection을
보낸 직후 socket을 닫아도 구체적인 `4/4` 이유가 generic disconnect 상태에 덮이지 않게 하는 순서다.

### `CMainApp`

`RenderBossHealthBar`는 ImGui frame 안에서 foreground draw list에 bar background/fill/border만 추가한다.
`RenderCombatHUDText`는 ImGui EndFrame 뒤 Font Manager로 `current / max` 숫자만 그린다. boss name, phase,
health-bar count는 그리지 않는다.

## G00-03. CPP 함수별 한 줄 책임

- `Write_Message(S2C_ENTER_REJECTED)`: protocol/world/reason을 검증하고 5-byte payload를 기록한다.
- `Read_Message(S2C_ENTER_REJECTED)`: payload를 local 값으로 읽고 완전 검증 뒤 output을 commit한다.
- `CGameRoom::Join`: 일반 invalid request와 정상 full admission을 분리하고 성공 전에 ID/map을 변경하지 않는다.
- `CGameRoom::Send_EnterRejected`: current room ID와 reason을 wire frame으로 보낸다.
- `CGameRoom::Reset_ReplayableArenaWhenEmpty`: trigger, spawn runtime, world entity, tick damage를 새 generation으로 만든다.
- `CNetworkManager::Handle_Frame`: rejection frame을 typed main-thread pending state로 stage한다.
- `CNetworkManager::Try_Consume_EnterRejected`: pending rejection을 Lobby에 한 번만 전달한다.
- `CLevel_Lobby::Consume_EnterRejected`: pending entry와 world/reason을 대조하고 connection을 정리한 뒤 상태를 보존한다.
- `CMainApp::RenderBossHealthBar`: boss HP ratio를 viewport-scaled 단일 bar로 그린다.
- `Run_FourPlayerCohort`: 실제 socket cohort의 입장/full/교체/2세대 순서를 검증한다.
- `Run-ValtanFourPlayerHarness.ps1`: 소유한 Server와 harness만 시작·종료하고 7777 residue를 거부한다.

## G00-04. 실제 호출 흐름

### 다섯 번째 입장 거절

```text
CServerApp::On_SessionFrame
  -> C2S_ENTER_WORLD codec 검증
  -> ROOM_COMMAND::ENTER_WORLD enqueue
  -> CGameRoom::Join
  -> Is_PlayerAdmissionFull
  -> Send_EnterRejected
  -> CClientSession::Send_Frame
  -> Request_Close
  -> Client receive worker/parser
  -> CNetworkManager::Handle_Frame
  -> CLevel_Lobby::Consume_EnterRejected
  -> Cancel_PendingEntry
  -> Lobby status = Valtan raid is full (4/4)
```

full 분기에서는 PlayerId/NetEntityId allocator와 session/player/entity map을 전혀 바꾸지 않는다.

### 마지막 player 퇴장

```text
CGameRoom::Leave
  -> trigger player membership 제거
  -> player/session/entity map 제거
  -> despawn broadcast
  -> Reset_ReplayableArenaWhenEmpty
     -> CServerTriggerSystem::Initialize
     -> CSpawnGroupRuntime::Initialize
     -> Initialize_WorldEntities
     -> m_TickDamageEvents.clear
```

reset 중 어느 단계라도 실패하면 `m_isReady=false`로 바꾸고 partial-ready room을 계속 사용하지 않는다.

### 단일 boss HP 표시

```text
Server world snapshot boss HP
  -> CClientReplication
  -> CCombatHUDViewModel::Get_Boss
  -> CMainApp::RenderBossHealthBar (bar)
  -> ImGuiLayer::EndFrame
  -> CMainApp::RenderCombatHUDText (current / max)
```

## G00-05. 사용자가 main에 직접 작성할 순서

1. Shared packet type/message/codec와 NetworkProtocolHarness를 먼저 반영하고 protocol 18을 빌드한다.
2. World publisher의 Valtan four-spawn gate를 반영하고 `-Mode Validate`를 실행한다.
3. GameRoom H/CPP와 Server contract test를 반영해 Debug/Release `--contract-test`를 실행한다.
4. NetworkManager H/CPP를 반영해 typed rejection pending state를 만든다.
5. Level_Lobby H/CPP를 반영해 rejection-first 소비 순서를 연결한다.
6. MainApp H/CPP를 반영해 single boss bar와 current/max text만 남긴다.
7. 새 ValtanFourPlayerHarness project/filter/CPP와 orchestration script를 추가한다.
8. `Framework.sln`, build regression, ProjectAudit contract를 갱신한다.
9. Debug/Release live harness를 실행하고 listener residue 0을 확인한다.
10. Client를 직접 실행해 Valtan 상단 HP bar를 육안 확인한다.

## G00-06. build와 harness

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate

msbuild Shared/Default/Shared.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
msbuild Server/Default/Server.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
Server/Bin/Debug/Server.exe --contract-test
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
powershell -ExecutionPolicy Bypass -File Tools/Network/Run-ValtanFourPlayerHarness.ps1 -Configuration Debug

msbuild Shared/Default/Shared.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
msbuild Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/Release/NetworkProtocolHarness.exe
msbuild Server/Default/Server.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
Server/Bin/Release/Server.exe --contract-test
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
powershell -ExecutionPolicy Bypass -File Tools/Network/Run-ValtanFourPlayerHarness.ps1 -Configuration Release

git diff --check
```

## G00-07. 적용 후 전체 코드 정본

아래 전문은 이 G00 worktree의 적용 후 파일을 기계적으로 복제한 코드 정본이다. 각 section의 경로가
main에서 대조·교체할 대상이며 `...` 또는 의사 코드는 사용하지 않는다.

### G00-07-01. `Shared/Public/Network/PacketType.h` full code

```cpp
#pragma once

#include <cstddef>
#include <cstdint>

namespace LostArk::Shared
{
	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 18;

	enum class WORLD_ID : std::uint16_t
	{
		BERN = 1,
		VALTAN_ARENA = 2,
		TRAINING_GROUND = 3,
		CHARACTER_SELECT_ARENA = 4,
		END
	};

	[[nodiscard]]
	constexpr bool Is_Known_World_Id(const WORLD_ID worldId)
	{
		return WORLD_ID::BERN == worldId ||
			WORLD_ID::VALTAN_ARENA == worldId ||
			WORLD_ID::TRAINING_GROUND == worldId ||
			WORLD_ID::CHARACTER_SELECT_ARENA == worldId;
	}

	enum class CHARACTER_CLASS_ID : std::uint8_t
	{
		LANCE_MASTER = 0,
		GUNSLINGER = 1,
		SLAYER = 2,
		ARTIST = 3,
		DESTROYER = 4,
		DIMENSIONMASTER = 5,
		WARLORD = 6,
		END
	};

	[[nodiscard]]
	constexpr bool Is_Known_Character_Class(const CHARACTER_CLASS_ID characterClass)
	{
		return static_cast<std::uint8_t>(characterClass) <
			static_cast<std::uint8_t>(CHARACTER_CLASS_ID::END);
	}

	enum class PLAYER_SKILL_KIND : std::uint8_t
	{
		ACTIVE = 0,
		COMBO = 1,
		HOLD = 2,
		// Two stages whose advance is a hit taken, not a press: the first stage
		// guards and the second is the counter it buys.
		COUNTER = 3,
		END
	};

	// A protocol value may be reserved before its runtime bundle exists. Only
	// classes accepted here may enter a world on the current build.
	[[nodiscard]]
	constexpr bool Is_Supported_Playable_Character_Class(
		const CHARACTER_CLASS_ID characterClass)
	{
		return CHARACTER_CLASS_ID::LANCE_MASTER == characterClass ||
			CHARACTER_CLASS_ID::GUNSLINGER == characterClass ||
			CHARACTER_CLASS_ID::SLAYER == characterClass ||
			CHARACTER_CLASS_ID::ARTIST == characterClass ||
			CHARACTER_CLASS_ID::DIMENSIONMASTER == characterClass ||
			CHARACTER_CLASS_ID::WARLORD == characterClass;
	}

	enum class PACKET_TYPE : std::uint16_t
	{
		INVALID,

		C2S_ENTER_WORLD,
		S2C_ENTER_ACCEPTED,
		S2C_ENTER_REJECTED,
		S2C_PLAYER_SPAWNED,
		S2C_WORLD_ENTITY_SPAWNED,
		C2S_SPAWN_WORLD_ENTITY,
		S2C_WORLD_ENTITY_SPAWN_RESULT,

		C2S_MOVE,
		C2S_USE_SKILL,
		C2S_RELEASE_SKILL,
		C2S_UPDATE_SKILL_AIM,
		C2S_REVIVE_PLAYER,
		C2S_CHANGE_CHARACTER_CLASS,
		S2C_CHARACTER_CLASS_CHANGE_RESULT,
		S2C_WORLD_SNAPSHOT,

		C2S_CHAT,
		S2C_CHAT,

		S2C_PLAYER_DESPAWNED,
		S2C_WORLD_ENTITY_DESPAWNED
	};

	//TCP는 메시지 경계를 보존하지 않기 때문에, payload앞에 header를 둔다.
	//패킷별로 경계를 구분해야, packet을 하나의 의미 단위로 읽을 수 있다.
	// Header는 uint32 전체 Frame 크기 4바이트 + uint16 PacketType 2바이트 = 6바이트다.
	inline constexpr std::size_t PACKET_HEADER_BYTES = 6;
	// Header를 포함한 Frame 하나의 최대 크기는 64 KiB다.
	inline constexpr std::uint32_t MAX_PACKET_BYTES =
		64u * 1024u;
	// Parser 누적 버퍼는 최대 Frame 4개 분량만 허용해 비정상 입력의 무제한 증가를 막는다.
	inline constexpr std::size_t MAX_BUFFERED_PACKET_BYTES =
		static_cast<std::size_t>(MAX_PACKET_BYTES) * 4u;
	//알려진 패킷인지를 검사하는 함수, client에서 정해진 규칙대로 보낸 packet인지를 검사
	[[nodiscard]]
	constexpr bool Is_Known_Packet_Type(
		PACKET_TYPE packetType)
	{
		switch (packetType)
		{
		case PACKET_TYPE::C2S_ENTER_WORLD:
		case PACKET_TYPE::S2C_ENTER_ACCEPTED:
		case PACKET_TYPE::S2C_ENTER_REJECTED:
		case PACKET_TYPE::S2C_PLAYER_SPAWNED:
		case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED:
		case PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY:
		case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT:
		case PACKET_TYPE::C2S_MOVE:
		case PACKET_TYPE::C2S_USE_SKILL:
		case PACKET_TYPE::C2S_RELEASE_SKILL:
		case PACKET_TYPE::C2S_UPDATE_SKILL_AIM:
		case PACKET_TYPE::C2S_REVIVE_PLAYER:
		case PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS:
		case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
		case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
		case PACKET_TYPE::C2S_CHAT:
		case PACKET_TYPE::S2C_CHAT:
		case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
		case PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED:
			return true;
		default:
			return  false;
		}
	}

	//inline constexpr인 이유
	//inline : 이 헤더를 여러 .cpp가 include해도 동일한 변수 정의로 취급한다.
	//constexpr : 컴파일 타임 시간 상수
	inline constexpr std::size_t MAX_NICKNAME_BYTES = 32;
}
```

### G00-07-02. `Shared/Public/Network/PacketMessages.h` full code

```cpp
#pragma once

#include "Network/PacketType.h"
#include "NetworkIds.h"

#include <string>
#include <vector>
//character의 class와 nickname용 packet
namespace LostArk::Shared
{
	//harness가 직접 write u8, write string을 호출하기 때문에, client와 server가 같은 함수를 쓰도록,
	//shared로 옮긴다.

	class CPacketReader;
	class CPacketWriter;

	//Enter World
	struct C2S_ENTER_WORLD
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::BERN;

		CHARACTER_CLASS_ID eCharacterClass =
			CHARACTER_CLASS_ID::END;

		std::string strNickName;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_ENTER_WORLD& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_ENTER_WORLD& message);

	//Enter Accepted
	struct S2C_ENTER_ACCEPTED
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::BERN;

		PLAYER_ID iPlayerId =
			INVALID_PLAYER_ID;

		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_ENTER_ACCEPTED& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_ENTER_ACCEPTED& message);

	enum class ENTER_WORLD_REJECTION_REASON : std::uint8_t
	{
		ROOM_FULL,
		END
	};

	struct S2C_ENTER_REJECTED
	{
		std::uint16_t iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		WORLD_ID eWorldId = WORLD_ID::END;
		ENTER_WORLD_REJECTION_REASON eReason =
			ENTER_WORLD_REJECTION_REASON::END;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_ENTER_REJECTED& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_ENTER_REJECTED& message);

	//Player Spawn
	struct S2C_PLAYER_SPAWNED
	{
		PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
		std::string strNickName;
		//서버 기준 최초 생성 위치
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		//서버 기준 Y축 회전 각도
		float fYawDegrees = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PLAYER_SPAWNED& spawned);

	bool Read_Message(
		CPacketReader& reader,
		S2C_PLAYER_SPAWNED& spawned);

	inline constexpr std::size_t MAX_STABLE_NETWORK_ID_BYTES = 128;
	inline constexpr std::size_t MAX_WORLD_SNAPSHOT_ENTITIES = 256;

	struct C2S_SPAWN_WORLD_ENTITY
	{
		std::string strPlacementId;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_SPAWN_WORLD_ENTITY& message);
	bool Read_Message(
		CPacketReader& reader,
		C2S_SPAWN_WORLD_ENTITY& message);

	enum class WORLD_ENTITY_SPAWN_RESULT : std::uint8_t
	{
		SPAWNED,
		ALREADY_EXISTS,
		ACTIVATED,
		REJECTED,
		END
	};

	struct S2C_WORLD_ENTITY_SPAWN_RESULT
	{
		std::string strPlacementId;
		WORLD_ENTITY_SPAWN_RESULT eResult =
			WORLD_ENTITY_SPAWN_RESULT::REJECTED;
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_SPAWN_RESULT& message);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_SPAWN_RESULT& message);

	enum class WORLD_ENTITY_KIND : std::uint8_t
	{
		NPC,
		BOSS,
		MONSTER,
		END
	};

	enum class WORLD_ENTITY_ACTION : std::uint8_t
	{
		IDLE,
		CHASE,
		PATTERN_WINDUP,
		PATTERN_ACTIVE,
		PATTERN_RECOVERY,
		DEAD,
		END
	};

	struct S2C_WORLD_ENTITY_SPAWNED
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_KIND eKind = WORLD_ENTITY_KIND::END;
		std::string strArchetypeId;
		std::string strEncounterId;
		std::string strPlacementId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		// Server-authoritative XZ combat body radius. NPC presentations have no
		// combat body and therefore publish zero.
		float fCollisionRadius = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_SPAWNED& spawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_SPAWNED& spawned);

	struct S2C_WORLD_ENTITY_DESPAWNED
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_ENTITY_DESPAWNED& despawned);
	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_ENTITY_DESPAWNED& despawned);

	//server와 client가 player제거으 byte 순서를 똑같이 사용하기 위해서이다.
	//H 계약 : NetEntityId와 제거 이유 enun을 값으로 선언하고, writer/reaeder/
	//overload를 공개한다.
	//cpp 흐름 : 모든 입력 검증 -> ID U32 reasonU8 기록.
	//읽을 때는 지역 변수로 전부 복원하고 검증이 끝난 뒤 출력 구조체에 한 번만 commit한다.
	enum class PLAYER_DESPAWN_REASON : std::uint8_t
	{
		DISCONNECTED,
		LEFT_ROOM,
		KICKED,
		LEVEL_CHANGED,
		END
	};
	//player entity id와 reason에 대한 정보를 enum으로 들고있다.
	struct S2C_PLAYER_DESPAWNED
	{
		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;

		PLAYER_DESPAWN_REASON eReason =
			PLAYER_DESPAWN_REASON::END;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PLAYER_DESPAWNED& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_PLAYER_DESPAWNED& message);

	//플레이어 이동과 스냅샷을 server에게 전달하고 동기화 시키는 것까지 구현
	inline constexpr std::size_t
		MAX_WORLD_SNAPSHOT_PLAYERS = 32;
	// Lance Master alone authors nine ACTIVE skills and can hold all nine on
	// cooldown at once, so eight silently dropped one tile from the HUD.
	inline constexpr std::size_t MAX_PLAYER_COOLDOWNS = 16;
	// One tick applies at most one player hit and one boss hit per actor, so this
	// bounds a 30 Hz frame rather than a fight.
	inline constexpr std::size_t MAX_DAMAGE_EVENTS = 64;
	// Matches the publisher's 2..8 comboStages bound.
	inline constexpr std::uint8_t MAX_COMBO_STAGES = 8;
	using SKILL_ID = std::uint32_t;
	inline constexpr SKILL_ID INVALID_SKILL_ID = 0;
	//이게 플레이어의 스킬을 의미하는 건가?
	enum class PLAYER_LOCOMOTION_STATE : std::uint8_t
	{
		IDLE,
		MOVING,
		END
	};
	//client->server move
	struct C2S_MOVE
	{
		std::uint32_t iClientSequence = 0;

		float fGoalX = 0.f;
		float fGoalZ = 0.f;
	};
	//근데 read가 const가 붙어야 하는 거 아닌가?
	bool Write_Message(
		CPacketWriter& writer,
		const C2S_MOVE& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_MOVE& message);

	// Client intent contains no player or entity ID. The server resolves the
	// actor from the authenticated session that owns this command.
	struct C2S_USE_SKILL
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_USE_SKILL& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_USE_SKILL& message);

	// A HOLD skill leaves its loop when the player lets the key go. The server
	// still owns when the action ends; this only reports the input edge.
	struct C2S_RELEASE_SKILL
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
	};

	// Development Balance Tool intent. The authenticated session identifies the
	// player; no position or HP is trusted from the client.
	struct C2S_REVIVE_PLAYER
	{
		std::uint32_t iClientSequence = 0;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_RELEASE_SKILL& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_RELEASE_SKILL& message);

	// A HOLD skill may keep turning toward the cursor while it charges. The
	// server only honors this during the charge stages; the firing stage keeps
	// the last direction it was given.
	struct C2S_UPDATE_SKILL_AIM
	{
		std::uint32_t iClientSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		float fAimX = 0.f;
		float fAimZ = 0.f;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_UPDATE_SKILL_AIM& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_UPDATE_SKILL_AIM& message);

	bool Write_Message(
		CPacketWriter& writer,
		const C2S_REVIVE_PLAYER& message);

	bool Read_Message(
		CPacketReader& reader,
		C2S_REVIVE_PLAYER& message);

	struct C2S_CHANGE_CHARACTER_CLASS
	{
		std::uint32_t iClientSequence = 0;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
	};

	enum class CHARACTER_CLASS_CHANGE_RESULT : std::uint8_t
	{
		ACCEPTED,
		REJECTED_WRONG_WORLD,
		REJECTED_STALE_SEQUENCE,
		REJECTED_UNSUPPORTED_CLASS,
		REJECTED_SAME_CLASS,
		REJECTED_STATE,
		END
	};

	struct S2C_CHARACTER_CLASS_CHANGE_RESULT
	{
		std::uint32_t iClientSequence = 0;
		CHARACTER_CLASS_CHANGE_RESULT eResult =
			CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
		CHARACTER_CLASS_ID eRequestedClass = CHARACTER_CLASS_ID::END;
		CHARACTER_CLASS_ID eActiveClass = CHARACTER_CLASS_ID::END;
	};

	bool Write_Message(CPacketWriter& writer,
		const C2S_CHANGE_CHARACTER_CLASS& message);
	bool Read_Message(CPacketReader& reader,
		C2S_CHANGE_CHARACTER_CLASS& message);
	bool Write_Message(CPacketWriter& writer,
		const S2C_CHARACTER_CLASS_CHANGE_RESULT& message);
	bool Read_Message(CPacketReader& reader,
		S2C_CHARACTER_CLASS_CHANGE_RESULT& message);

	enum class PLAYER_ACTION_STATE : std::uint8_t
	{
		NONE,
		SKILL,
		TRIGGER_MOVE,
		DEAD,
		END
	};

	enum class PLAYER_STANCE_ID : std::uint8_t
	{
		NONE,
		LANCE_MASTER_LONG_SPEAR,
		LANCE_MASTER_SHORT_SPEAR,
		WARLORD_NORMAL,
		WARLORD_DEFENSE,
		END
	};

	struct SKILL_COOLDOWN_SNAPSHOT
	{
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		std::uint32_t iCooldownEndTick = 0;
	};

	//player
	struct PLAYER_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId =
			INVALID_NET_ENTITY_ID;
		CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;

		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;

		PLAYER_LOCOMOTION_STATE eLocomotionState =
			PLAYER_LOCOMOTION_STATE::IDLE;

		PLAYER_ACTION_STATE eAction = PLAYER_ACTION_STATE::NONE;
		PLAYER_STANCE_ID eStance = PLAYER_STANCE_ID::NONE;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint32_t iCurrentResource = 0;
		std::uint32_t iMaximumResource = 1;
		// The class identity gauge. A maximum of 0 says the class has none, and
		// the HUD then has nothing to draw.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		bool isCombatReady = true;
		// 0 outside a staged action, 1-based stage index while one runs: combo
		// stages, and start/loop/end for a HOLD skill. The server owns it; the
		// client must not count stages itself.
		std::uint8_t iComboStage = 0;
		std::vector<SKILL_COOLDOWN_SNAPSHOT> Cooldowns;
	};

	struct WORLD_ENTITY_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		WORLD_ENTITY_ACTION eAction = WORLD_ENTITY_ACTION::END;
		std::string strPatternId;
		std::string strActionId;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStageIndex = 0;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		std::uint32_t iActionStartTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		std::uint8_t iPhase = 1;
	};
	// One resolved hit. HP in the snapshots above is a level, so a client that
	// only sees levels cannot tell 500 damage from two 250s inside one tick, and
	// cannot place a number where the hit landed. The server already computes this
	// value to subtract it; this carries the same number rather than letting the
	// client re-derive one it has no authority for. This is the snapshot's only
	// edge-triggered payload: a missed event never desyncs HP.
	struct DAMAGE_EVENT
	{
		// Whoever took the damage: a player or a world entity, both of which live
		// in the same NET_ENTITY_ID space.
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iAmount = 0;
		// Where to anchor the number, in world units. Taken from the target at the
		// moment of the hit so a number does not follow the target afterwards.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		// True when a player dealt it. Presentation styles incoming and outgoing
		// damage differently, and only the server knows which is which.
		bool isOutgoing = false;
	};

	//player snapshot을 vector 구조체로 들고, servertick을 들고있다?
	struct S2C_WORLD_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		WORLD_ID eWorldId = WORLD_ID::BERN;
		std::vector<PLAYER_SNAPSHOT> Players;
		std::vector<WORLD_ENTITY_SNAPSHOT> Entities;
		std::vector<DAMAGE_EVENT> DamageEvents;
	};

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_WORLD_SNAPSHOT& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_WORLD_SNAPSHOT& message);
}
```

### G00-07-03. `Shared/Private/Network/PacketMessages.cpp` full code

```cpp
#include "Network/PacketMessages.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

//writer가 일부만 기록한 뒤 실패하지 않도록 snapshot의 모든 state를 먼저 검증한다.
namespace
{
    //유효한 애니메이션인지 검증
    bool Is_Valid_Locomotion(
        LostArk::Shared::PLAYER_LOCOMOTION_STATE state)
    {
        return static_cast<std::uint8_t>(state) <
            static_cast<std::uint8_t>(
                LostArk::Shared::PLAYER_LOCOMOTION_STATE::END);
    }

	bool Is_Valid_PlayerAction(
		const LostArk::Shared::PLAYER_ACTION_STATE state)
	{
		return static_cast<std::uint8_t>(state) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_ACTION_STATE::END);
	}

	bool Is_Valid_Stance(
		const LostArk::Shared::PLAYER_STANCE_ID stance)
	{
		return static_cast<std::uint8_t>(stance) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_STANCE_ID::END);
	}

	bool Is_Valid_Cooldowns(
		const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>& cooldowns)
	{
		if (cooldowns.size() > LostArk::Shared::MAX_PLAYER_COOLDOWNS)
			return false;
		for (std::size_t i = 0; i < cooldowns.size(); ++i)
		{
			if (cooldowns[i].iSkillId == LostArk::Shared::INVALID_SKILL_ID ||
				0 == cooldowns[i].iCooldownEndTick)
			{
				return false;
			}
			for (std::size_t j = i + 1; j < cooldowns.size(); ++j)
			{
				if (cooldowns[i].iSkillId == cooldowns[j].iSkillId)
					return false;
			}
		}
		return true;
	}
    //유효한 플레이어 스냅샷인지 검증 - netentityid, position x y z, locomotion state
    bool Is_Valid_PlayerSnapshot(
        const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
    {
        return
            snapshot.iNetEntityId != LostArk::Shared::INVALID_NET_ENTITY_ID &&
			LostArk::Shared::Is_Supported_Playable_Character_Class(
				snapshot.eCharacterClass) &&
            std::isfinite(snapshot.fPositionX) &&
            std::isfinite(snapshot.fPositionY) &&
            std::isfinite(snapshot.fPositionZ) &&
            std::isfinite(snapshot.fYawDegrees) &&
            Is_Valid_Locomotion(snapshot.eLocomotionState) &&
			Is_Valid_PlayerAction(snapshot.eAction) &&
			Is_Valid_Stance(snapshot.eStance) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iMaximumResource &&
			snapshot.iCurrentResource <= snapshot.iMaximumResource &&
			snapshot.iCurrentIdentity <= snapshot.iMaximumIdentity &&
			Is_Valid_Cooldowns(snapshot.Cooldowns) &&
			snapshot.iComboStage <= LostArk::Shared::MAX_COMBO_STAGES &&
			(0 == snapshot.iComboStage ||
				LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction) &&
			((LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction &&
				snapshot.iSkillId != LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 (LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 ((LostArk::Shared::PLAYER_ACTION_STATE::SKILL != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != snapshot.eAction) &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID));
    }

	bool Is_Valid_StableId(const std::string& value, const bool allowEmpty)
	{
		return (allowEmpty && value.empty()) ||
			(!value.empty() && value.size() <=
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES &&
				std::all_of(value.begin(), value.end(), [](const unsigned char character)
				{
					return 0 != std::isalnum(character) || character == '_' ||
						character == '-' || character == '.';
				}));
	}

	bool Is_Valid_WorldEntityKind(
		const LostArk::Shared::WORLD_ENTITY_KIND kind)
	{
		return static_cast<std::uint8_t>(kind) <
			static_cast<std::uint8_t>(LostArk::Shared::WORLD_ENTITY_KIND::END);
	}

	bool Is_Valid_WorldEntityAction(
		const LostArk::Shared::WORLD_ENTITY_ACTION action)
	{
		return static_cast<std::uint8_t>(action) <
			static_cast<std::uint8_t>(LostArk::Shared::WORLD_ENTITY_ACTION::END);
	}

	bool Is_Valid_WorldEntitySnapshot(
		const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot)
	{
		return snapshot.iNetEntityId != LostArk::Shared::INVALID_NET_ENTITY_ID &&
			Is_Valid_WorldEntityAction(snapshot.eAction) &&
			Is_Valid_StableId(snapshot.strPatternId, true) &&
			Is_Valid_StableId(snapshot.strActionId, true) &&
			std::isfinite(snapshot.fPositionX) &&
			std::isfinite(snapshot.fPositionY) &&
			std::isfinite(snapshot.fPositionZ) &&
			std::isfinite(snapshot.fYawDegrees) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iPhase;
	}

	// A zero amount is not a hit, so it must not reach presentation as one; the
	// server clamps every resolved hit to at least 1.
	bool Is_Valid_DamageEvent(
		const LostArk::Shared::DAMAGE_EVENT& damage)
	{
		return
			damage.iTargetNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			0 != damage.iAmount &&
			std::isfinite(damage.fPositionX) &&
			std::isfinite(damage.fPositionY) &&
			std::isfinite(damage.fPositionZ);
	}
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_ENTER_WORLD& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId))
	{
		return false;
	}

    //character class write
    const std::uint8_t rawCharacterClass =
        static_cast<std::uint8_t>(
            message.eCharacterClass);

    if (rawCharacterClass >= static_cast<std::uint8_t>(
        CHARACTER_CLASS_ID::END))
        return false;

    //message의 nickname 검증
    if (message.strNickName.empty())
        return false;

    if (message.strNickName.size() >
        MAX_NICKNAME_BYTES)
        return false;

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));

	//character class write
	writer.Write_U8(rawCharacterClass);

    //nickname write
    return writer.Write_String(
        message.strNickName,
        MAX_NICKNAME_BYTES);
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_ENTER_WORLD& message)
{
	std::uint16_t protocolVersion = {};
	std::uint16_t rawWorldId = {};
	std::uint8_t rawCharacterClass = {};
	std::string nickName;

	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)))
	{
		return false;
	}

    //character class 예외처리
    if (!reader.Read_U8(rawCharacterClass))
        return false;

    if (rawCharacterClass >= static_cast<std::uint8_t>(
        CHARACTER_CLASS_ID::END))
        return false;

    //nickname read
    if (!reader.Read_String(
        nickName,
        MAX_NICKNAME_BYTES))
        return false;

    if (nickName.empty())
        return false;

	C2S_ENTER_WORLD decoded{};
	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);

    decoded.eCharacterClass =
        static_cast<CHARACTER_CLASS_ID>(
            rawCharacterClass);

    decoded.strNickName = std::move(nickName);

    //const라서 대입이 안 되는 상황?
    message = std::move(decoded);

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer,
    const S2C_ENTER_ACCEPTED& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId))
	{
		return false;
	}

    //playerid가 유효한지 검사
    if (message.iPlayerId == INVALID_PLAYER_ID)
        return false;

    //netid가 유효한지 검사
    if (message.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));

	//playerid, netentityid를 u32로 기록
    writer.Write_U32(message.iPlayerId);
    writer.Write_U32(message.iNetEntityId);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader,
   S2C_ENTER_ACCEPTED& message)
{
	std::uint16_t protocolVersion = {};
	std::uint16_t rawWorldId = {};
	PLAYER_ID playerId =
        INVALID_PLAYER_ID;

    NET_ENTITY_ID netEntityId =
        INVALID_NET_ENTITY_ID;

	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)))
	{
		return false;
	}

	if (!reader.Read_U32(playerId))
        return false;
    if (!reader.Read_U32(netEntityId))
        return false;

    if (playerId == 0 || netEntityId == 0)
        return false;

	S2C_ENTER_ACCEPTED decoded {};

	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.iPlayerId = playerId;
    decoded.iNetEntityId = netEntityId;

    message = decoded;

    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_ENTER_REJECTED& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId) ||
		ENTER_WORLD_REJECTION_REASON::ROOM_FULL != message.eReason)
	{
		return false;
	}

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U8(static_cast<std::uint8_t>(message.eReason));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_ENTER_REJECTED& message)
{
	std::uint16_t protocolVersion = 0;
	std::uint16_t rawWorldId = 0;
	std::uint8_t rawReason = 0;
	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		!reader.Read_U8(rawReason) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)) ||
		static_cast<std::uint8_t>(ENTER_WORLD_REJECTION_REASON::ROOM_FULL) !=
			rawReason)
	{
		return false;
	}

	S2C_ENTER_REJECTED decoded{};
	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.eReason = static_cast<ENTER_WORLD_REJECTION_REASON>(rawReason);
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer,
    const S2C_PLAYER_SPAWNED& spawned)
{
    //playerid가 유효한지 검사
    if (spawned.iPlayerId == INVALID_PLAYER_ID)
        return false;

    //netid가 유효한지 검사
    if (spawned.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    //character class가 end인지 검사
    const std::uint8_t rawCharacterClass =
        static_cast<std::uint8_t>(spawned.eCharacterClass);

    if (rawCharacterClass >=
        static_cast<std::uint8_t>(CHARACTER_CLASS_ID::END))
        return false;

    //Nickname이 비어있는지 검사
    if (spawned.strNickName.empty() ||
        spawned.strNickName.length() > MAX_NICKNAME_BYTES)
        return false;

    //position X/Y/Z가 finite인지 검사
    if (!std::isfinite(spawned.fPositionX) ||
        !std::isfinite(spawned.fPositionY) ||
        !std::isfinite(spawned.fPositionZ))
        return false;

    //yawDegrees가 finite인지 검사
    if (!std::isfinite(spawned.fYawDegrees))
        return false;

    //playerid, net entity, character class, nickname
    //position x y z, yawdegrees를 u32로 기록
    writer.Write_U32(spawned.iPlayerId);
    writer.Write_U32(spawned.iNetEntityId);

    writer.Write_U8(rawCharacterClass);

    if (!writer.Write_String(
        spawned.strNickName,
        MAX_NICKNAME_BYTES))
    {
        return false;
    }

    writer.Write_F32(spawned.fPositionX);
    writer.Write_F32(spawned.fPositionY);
    writer.Write_F32(spawned.fPositionZ);
    writer.Write_F32(spawned.fYawDegrees);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader,
    S2C_PLAYER_SPAWNED& spawned)
{
    PLAYER_ID iPlayerId =
        INVALID_PLAYER_ID;

    NET_ENTITY_ID iNetEntityId =
        INVALID_NET_ENTITY_ID;

    std::uint8_t rawCharacterClass = {};
    std::string nickName;

    float positionX = 0.f;
    float positionY = 0.f;
    float positionZ = 0.f;
    float yawDegrees = 0.f;

    if (!reader.Read_U32(iPlayerId))
        return false;

    if (!reader.Read_U32(iNetEntityId))
        return false;

    if (!reader.Read_U8(rawCharacterClass))
        return false;

    if (!reader.Read_String(
        nickName,
        MAX_NICKNAME_BYTES))
    {
        return false;
    }

    if (!reader.Read_F32(positionX))
        return false;

    if (!reader.Read_F32(positionY))
        return false;

    if (!reader.Read_F32(positionZ))
        return false;

    if (!reader.Read_F32(yawDegrees))
        return false;

    if (iPlayerId == INVALID_PLAYER_ID)
        return false;

    if (iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    if (rawCharacterClass >=
        static_cast<std::uint8_t>(
            CHARACTER_CLASS_ID::END))
    {
        return false;
    }

    if (nickName.empty())
        return false;

    if (!std::isfinite(positionX) ||
        !std::isfinite(positionY) ||
        !std::isfinite(positionZ) ||
        !std::isfinite(yawDegrees))
    {
        return false;
    }



    S2C_PLAYER_SPAWNED decoded{};

    decoded.iPlayerId = iPlayerId;
    decoded.iNetEntityId = iNetEntityId;

    decoded.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(
        rawCharacterClass);

    decoded.strNickName =
        std::move(nickName);

    decoded.fPositionX = positionX;
    decoded.fPositionY = positionY;
    decoded.fPositionZ = positionZ;
    decoded.fYawDegrees = yawDegrees;

    spawned = std::move(decoded);

    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	if (spawned.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(spawned.eKind) ||
		!Is_Valid_StableId(spawned.strArchetypeId, false) ||
		!Is_Valid_StableId(spawned.strEncounterId, true) ||
		!Is_Valid_StableId(spawned.strPlacementId, true) ||
		!std::isfinite(spawned.fPositionX) ||
		!std::isfinite(spawned.fPositionY) ||
		!std::isfinite(spawned.fPositionZ) ||
		!std::isfinite(spawned.fYawDegrees) ||
		!std::isfinite(spawned.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC == spawned.eKind &&
			0.f != spawned.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC != spawned.eKind &&
			spawned.fCollisionRadius <= 0.f))
	{
		return false;
	}
	writer.Write_U32(spawned.iNetEntityId);
	writer.Write_U8(static_cast<std::uint8_t>(spawned.eKind));
	if (!writer.Write_String(
		spawned.strArchetypeId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
		spawned.strEncounterId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
		spawned.strPlacementId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_F32(spawned.fPositionX);
	writer.Write_F32(spawned.fPositionY);
	writer.Write_F32(spawned.fPositionZ);
	writer.Write_F32(spawned.fYawDegrees);
	writer.Write_F32(spawned.fCollisionRadius);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	S2C_WORLD_ENTITY_SPAWNED decoded{};
	std::uint8_t rawKind = 0;
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		!reader.Read_U8(rawKind) ||
		!reader.Read_String(
			decoded.strArchetypeId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strEncounterId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strPlacementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_F32(decoded.fPositionX) ||
		!reader.Read_F32(decoded.fPositionY) ||
		!reader.Read_F32(decoded.fPositionZ) ||
		!reader.Read_F32(decoded.fYawDegrees) ||
		!reader.Read_F32(decoded.fCollisionRadius))
	{
		return false;
	}
	decoded.eKind = static_cast<WORLD_ENTITY_KIND>(rawKind);
	if (decoded.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(decoded.eKind) ||
		!Is_Valid_StableId(decoded.strArchetypeId, false) ||
		!Is_Valid_StableId(decoded.strEncounterId, true) ||
		!Is_Valid_StableId(decoded.strPlacementId, true) ||
		!std::isfinite(decoded.fPositionX) ||
		!std::isfinite(decoded.fPositionY) ||
		!std::isfinite(decoded.fPositionZ) ||
		!std::isfinite(decoded.fYawDegrees) ||
		!std::isfinite(decoded.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC == decoded.eKind &&
			0.f != decoded.fCollisionRadius) ||
		(WORLD_ENTITY_KIND::NPC != decoded.eKind &&
			decoded.fCollisionRadius <= 0.f))
	{
		return false;
	}
	spawned = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	if (INVALID_NET_ENTITY_ID == despawned.iNetEntityId)
		return false;

	writer.Write_U32(despawned.iNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	S2C_WORLD_ENTITY_DESPAWNED decoded{};
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		INVALID_NET_ENTITY_ID == decoded.iNetEntityId)
	{
		return false;
	}

	despawned = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_SPAWN_WORLD_ENTITY& message)
{
	return Is_Valid_StableId(message.strPlacementId, false) &&
		writer.Write_String(
			message.strPlacementId,
			MAX_STABLE_NETWORK_ID_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_SPAWN_WORLD_ENTITY& message)
{
	std::string placementId;
	if (!reader.Read_String(placementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!Is_Valid_StableId(placementId, false))
	{
		return false;
	}

	C2S_SPAWN_WORLD_ENTITY decoded{};
	decoded.strPlacementId = std::move(placementId);
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	const std::uint8_t rawResult =
		static_cast<std::uint8_t>(message.eResult);
	const bool hasEntity =
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED == message.eResult ||
		WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS == message.eResult;
	if (!Is_Valid_StableId(message.strPlacementId, false) ||
		rawResult >= static_cast<std::uint8_t>(
			WORLD_ENTITY_SPAWN_RESULT::END) ||
		(hasEntity && INVALID_NET_ENTITY_ID == message.iNetEntityId) ||
		(!hasEntity && INVALID_NET_ENTITY_ID != message.iNetEntityId))
	{
		return false;
	}
	if (!writer.Write_String(
		message.strPlacementId,
		MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U8(rawResult);
	writer.Write_U32(message.iNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	S2C_WORLD_ENTITY_SPAWN_RESULT decoded{};
	std::uint8_t rawResult = 0;
	if (!reader.Read_String(
		decoded.strPlacementId,
		MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U8(rawResult) ||
		!reader.Read_U32(decoded.iNetEntityId))
	{
		return false;
	}
	decoded.eResult = static_cast<WORLD_ENTITY_SPAWN_RESULT>(rawResult);
	const bool hasEntity =
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED == decoded.eResult ||
		WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS == decoded.eResult;
	if (!Is_Valid_StableId(decoded.strPlacementId, false) ||
		rawResult >= static_cast<std::uint8_t>(
			WORLD_ENTITY_SPAWN_RESULT::END) ||
		(hasEntity && INVALID_NET_ENTITY_ID == decoded.iNetEntityId) ||
		(!hasEntity && INVALID_NET_ENTITY_ID != decoded.iNetEntityId))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_PLAYER_DESPAWNED& message)
{
    //검증
    if (message.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    const std::uint8_t rawReason =
        static_cast<std::uint8_t>(message.eReason);

    if (rawReason >= static_cast<std::uint8_t>(
        PLAYER_DESPAWN_REASON::END))
    {
        return false;
    }

    //정보 쓰기
    writer.Write_U32(message.iNetEntityId);
    writer.Write_U8(rawReason);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_PLAYER_DESPAWNED& message)
{
    NET_ENTITY_ID netEntityId = INVALID_NET_ENTITY_ID;
    std::uint8_t rawReason = {};

    if (!reader.Read_U32(netEntityId))
        return false;

    if (!reader.Read_U8(rawReason))
        return false;

    if (netEntityId == INVALID_NET_ENTITY_ID)
        return false;

    if (rawReason >= static_cast<std::uint8_t>(
        PLAYER_DESPAWN_REASON::END))
        return false;

    S2C_PLAYER_DESPAWNED decoded{};

    decoded.iNetEntityId = netEntityId;
    decoded.eReason = static_cast<PLAYER_DESPAWN_REASON>(
        rawReason);

    message = decoded;

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_MOVE& message)
{
    if (0 == message.iClientSequence ||
        !std::isfinite(message.fGoalX) ||
        !std::isfinite(message.fGoalZ))
    {
        return false;
    }

    writer.Write_U32(message.iClientSequence);
    writer.Write_F32(message.fGoalX);
    writer.Write_F32(message.fGoalZ);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_MOVE& message)
{
    std::uint32_t clientSequence = 0;
    float fGoalX = 0.f;
    float fGoalZ = 0.f;

    if (!reader.Read_U32(clientSequence) ||
        !reader.Read_F32(fGoalX) ||
        !reader.Read_F32(fGoalZ))
    {
        return false;
    }

    if (0 == clientSequence ||
        !std::isfinite(fGoalX) ||
        !std::isfinite(fGoalZ))
    {
        return false;
    }

    C2S_MOVE decoded{};
    decoded.iClientSequence = clientSequence;
    decoded.fGoalX = fGoalX;
    decoded.fGoalZ = fGoalZ;

    message = decoded;
    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_USE_SKILL& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_SKILL& message)
{
	C2S_USE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_RELEASE_SKILL& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId)
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_REVIVE_PLAYER& message)
{
	if (0u == message.iClientSequence)
		return false;
	writer.Write_U32(message.iClientSequence);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_RELEASE_SKILL& message)
{
	C2S_RELEASE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId)
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_UPDATE_SKILL_AIM& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_UPDATE_SKILL_AIM& message)
{
	C2S_UPDATE_SKILL_AIM decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_REVIVE_PLAYER& message)
{
	C2S_REVIVE_PLAYER decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		0u == decoded.iClientSequence)
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_CHANGE_CHARACTER_CLASS& message)
{
	if (0u == message.iClientSequence ||
		!Is_Known_Character_Class(message.eCharacterClass))
	{
		return false;
	}
	writer.Write_U32(message.iClientSequence);
	writer.Write_U8(static_cast<std::uint8_t>(message.eCharacterClass));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_CHANGE_CHARACTER_CLASS& message)
{
	C2S_CHANGE_CHARACTER_CLASS decoded{};
	std::uint8_t rawClass = 0;
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U8(rawClass))
	{
		return false;
	}
	decoded.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(rawClass);
	if (0u == decoded.iClientSequence ||
		!Is_Known_Character_Class(decoded.eCharacterClass))
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	if (0u == message.iClientSequence ||
		static_cast<std::uint8_t>(message.eResult) >=
			static_cast<std::uint8_t>(CHARACTER_CLASS_CHANGE_RESULT::END) ||
		!Is_Known_Character_Class(message.eRequestedClass) ||
		!Is_Supported_Playable_Character_Class(message.eActiveClass) ||
		((CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == message.eResult ||
			CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS == message.eResult) &&
			message.eRequestedClass != message.eActiveClass))
	{
		return false;
	}
	writer.Write_U32(message.iClientSequence);
	writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
	writer.Write_U8(static_cast<std::uint8_t>(message.eRequestedClass));
	writer.Write_U8(static_cast<std::uint8_t>(message.eActiveClass));
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	S2C_CHARACTER_CLASS_CHANGE_RESULT decoded{};
	std::uint8_t rawResult = 0;
	std::uint8_t rawRequested = 0;
	std::uint8_t rawActive = 0;
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U8(rawResult) ||
		!reader.Read_U8(rawRequested) ||
		!reader.Read_U8(rawActive))
	{
		return false;
	}
	decoded.eResult = static_cast<CHARACTER_CLASS_CHANGE_RESULT>(rawResult);
	decoded.eRequestedClass = static_cast<CHARACTER_CLASS_ID>(rawRequested);
	decoded.eActiveClass = static_cast<CHARACTER_CLASS_ID>(rawActive);
	if (0u == decoded.iClientSequence ||
		rawResult >= static_cast<std::uint8_t>(CHARACTER_CLASS_CHANGE_RESULT::END) ||
		!Is_Known_Character_Class(decoded.eRequestedClass) ||
		!Is_Supported_Playable_Character_Class(decoded.eActiveClass) ||
		((CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == decoded.eResult ||
			CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS == decoded.eResult) &&
			decoded.eRequestedClass != decoded.eActiveClass))
	{
		return false;
	}
	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_WORLD_SNAPSHOT& message)
{
    //world의 snapshot write, servertick과 player 정보
	if (0 == message.iServerTick ||
		!Is_Known_World_Id(message.eWorldId) ||
        message.Players.empty() ||
        message.Players.size() >
        MAX_WORLD_SNAPSHOT_PLAYERS ||
		message.Entities.size() > MAX_WORLD_SNAPSHOT_ENTITIES ||
		message.DamageEvents.size() > MAX_DAMAGE_EVENTS)
    {
        return false;
    }
    //유효하지 않은 플레이어 스냅샷 검사
    for (const PLAYER_SNAPSHOT& player : message.Players)
    {
        if (!Is_Valid_PlayerSnapshot(player))
            return false;
    }
	for (const WORLD_ENTITY_SNAPSHOT& entity : message.Entities)
	{
		if (!Is_Valid_WorldEntitySnapshot(entity))
			return false;
	}
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		if (!Is_Valid_DamageEvent(damage))
			return false;
	}
    //server tick과 player size 넣기
	writer.Write_U32(message.iServerTick);
	writer.Write_U16(
		static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U16(
        static_cast<std::uint16_t>(
            message.Players.size()));
	writer.Write_U16(
		static_cast<std::uint16_t>(message.Entities.size()));
	// U8 is enough: MAX_DAMAGE_EVENTS bounds one tick, far under 255.
	writer.Write_U8(
		static_cast<std::uint8_t>(message.DamageEvents.size()));

    for (const PLAYER_SNAPSHOT& player : message.Players)
    {
        writer.Write_U32(player.iNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(player.eCharacterClass));
        writer.Write_F32(player.fPositionX);
        writer.Write_F32(player.fPositionY);
        writer.Write_F32(player.fPositionZ);
        writer.Write_F32(player.fYawDegrees);
        writer.Write_U8(
            static_cast<std::uint8_t>(
                player.eLocomotionState));
		writer.Write_U8(static_cast<std::uint8_t>(player.eAction));
		writer.Write_U8(static_cast<std::uint8_t>(player.eStance));
		writer.Write_U32(player.iSkillId);
		writer.Write_U32(player.iActionStartTick);
		writer.Write_U32(player.iCurrentHp);
		writer.Write_U32(player.iMaximumHp);
		writer.Write_U32(player.iCurrentResource);
		writer.Write_U32(player.iMaximumResource);
		writer.Write_U32(player.iCurrentIdentity);
		writer.Write_U32(player.iMaximumIdentity);
		writer.Write_U8(player.isCombatReady ? 1u : 0u);
		writer.Write_U8(player.iComboStage);
		writer.Write_U8(static_cast<std::uint8_t>(player.Cooldowns.size()));
		for (const SKILL_COOLDOWN_SNAPSHOT& cooldown : player.Cooldowns)
		{
			writer.Write_U32(cooldown.iSkillId);
			writer.Write_U32(cooldown.iCooldownEndTick);
		}
    }
	for (const WORLD_ENTITY_SNAPSHOT& entity : message.Entities)
	{
		writer.Write_U32(entity.iNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(entity.eAction));
		if (!writer.Write_String(
			entity.strPatternId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		if (!writer.Write_String(
			entity.strActionId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		writer.Write_F32(entity.fPositionX);
		writer.Write_F32(entity.fPositionY);
		writer.Write_F32(entity.fPositionZ);
		writer.Write_F32(entity.fYawDegrees);
		writer.Write_U32(entity.iActionStartTick);
		writer.Write_U32(entity.iPatternSequence);
		writer.Write_U32(entity.iPatternStageIndex);
		writer.Write_U32(entity.iCurrentHp);
		writer.Write_U32(entity.iMaximumHp);
		writer.Write_U8(entity.iPhase);
	}
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		writer.Write_U32(damage.iTargetNetEntityId);
		writer.Write_U32(damage.iAmount);
		writer.Write_F32(damage.fPositionX);
		writer.Write_F32(damage.fPositionY);
		writer.Write_F32(damage.fPositionZ);
		writer.Write_U8(damage.isOutgoing ? 1u : 0u);
	}

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_WORLD_SNAPSHOT& message)
{
	std::uint32_t serverTick = 0;
	std::uint16_t rawWorldId = 0;
	std::uint16_t playerCount = 0;
	std::uint16_t entityCount = 0;
	std::uint8_t damageEventCount = 0;

	if (!reader.Read_U32(serverTick) ||
		!reader.Read_U16(rawWorldId) ||
		!reader.Read_U16(playerCount) ||
		!reader.Read_U16(entityCount) ||
		!reader.Read_U8(damageEventCount))
    {
        return false;
    }

	if (0 == serverTick ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)) ||
        0 == playerCount ||
        playerCount > MAX_WORLD_SNAPSHOT_PLAYERS ||
		entityCount > MAX_WORLD_SNAPSHOT_ENTITIES ||
		damageEventCount > MAX_DAMAGE_EVENTS)
    {
        return false;
    }

	S2C_WORLD_SNAPSHOT decoded{};
	decoded.iServerTick = serverTick;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
    decoded.Players.reserve(playerCount);
	decoded.Entities.reserve(entityCount);
	decoded.DamageEvents.reserve(damageEventCount);

    for (std::uint16_t i = 0; i < playerCount; ++i)
    {
        PLAYER_SNAPSHOT player{};
		std::uint8_t rawCharacterClass = 0;
        std::uint8_t rawLocomotion = 0;
		std::uint8_t rawAction = 0;
		std::uint8_t rawStance = 0;
		std::uint8_t rawCombatReady = 0;
		std::uint8_t cooldownCount = 0;

        if (!reader.Read_U32(player.iNetEntityId) ||
			!reader.Read_U8(rawCharacterClass) ||
            !reader.Read_F32(player.fPositionX) ||
            !reader.Read_F32(player.fPositionY) ||
            !reader.Read_F32(player.fPositionZ) ||
            !reader.Read_F32(player.fYawDegrees) ||
            !reader.Read_U8(rawLocomotion) ||
			!reader.Read_U8(rawAction) ||
			!reader.Read_U8(rawStance) ||
			!reader.Read_U32(player.iSkillId) ||
			!reader.Read_U32(player.iActionStartTick) ||
			!reader.Read_U32(player.iCurrentHp) ||
			!reader.Read_U32(player.iMaximumHp) ||
			!reader.Read_U32(player.iCurrentResource) ||
			!reader.Read_U32(player.iMaximumResource) ||
			!reader.Read_U32(player.iCurrentIdentity) ||
			!reader.Read_U32(player.iMaximumIdentity) ||
			!reader.Read_U8(rawCombatReady) ||
			rawCombatReady > 1u ||
			!reader.Read_U8(player.iComboStage) ||
			player.iComboStage > MAX_COMBO_STAGES ||
			!reader.Read_U8(cooldownCount) ||
			cooldownCount > MAX_PLAYER_COOLDOWNS)
        {
            return false;
        }

        player.eLocomotionState =
            static_cast<PLAYER_LOCOMOTION_STATE>(
                rawLocomotion);
		player.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(rawCharacterClass);
		player.eAction = static_cast<PLAYER_ACTION_STATE>(rawAction);
		player.eStance = static_cast<PLAYER_STANCE_ID>(rawStance);
		player.isCombatReady = 0u != rawCombatReady;
		player.Cooldowns.reserve(cooldownCount);
		for (std::uint8_t cooldownIndex = 0;
			cooldownIndex < cooldownCount;
			++cooldownIndex)
		{
			SKILL_COOLDOWN_SNAPSHOT cooldown{};
			if (!reader.Read_U32(cooldown.iSkillId) ||
				!reader.Read_U32(cooldown.iCooldownEndTick))
			{
				return false;
			}
			player.Cooldowns.push_back(cooldown);
		}

        if (!Is_Valid_PlayerSnapshot(player))
            return false;

        decoded.Players.push_back(player);
    }
	for (std::uint16_t i = 0; i < entityCount; ++i)
	{
		WORLD_ENTITY_SNAPSHOT entity{};
		std::uint8_t rawAction = 0;
		if (!reader.Read_U32(entity.iNetEntityId) ||
			!reader.Read_U8(rawAction) ||
			!reader.Read_String(
				entity.strPatternId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_String(
				entity.strActionId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_F32(entity.fPositionX) ||
			!reader.Read_F32(entity.fPositionY) ||
			!reader.Read_F32(entity.fPositionZ) ||
			!reader.Read_F32(entity.fYawDegrees) ||
			!reader.Read_U32(entity.iActionStartTick) ||
			!reader.Read_U32(entity.iPatternSequence) ||
			!reader.Read_U32(entity.iPatternStageIndex) ||
			!reader.Read_U32(entity.iCurrentHp) ||
			!reader.Read_U32(entity.iMaximumHp) ||
			!reader.Read_U8(entity.iPhase))
		{
			return false;
		}
		entity.eAction = static_cast<WORLD_ENTITY_ACTION>(rawAction);
		if (!Is_Valid_WorldEntitySnapshot(entity))
			return false;
		decoded.Entities.push_back(std::move(entity));
	}
	for (std::uint8_t i = 0; i < damageEventCount; ++i)
	{
		DAMAGE_EVENT damage{};
		std::uint8_t rawOutgoing = 0;
		if (!reader.Read_U32(damage.iTargetNetEntityId) ||
			!reader.Read_U32(damage.iAmount) ||
			!reader.Read_F32(damage.fPositionX) ||
			!reader.Read_F32(damage.fPositionY) ||
			!reader.Read_F32(damage.fPositionZ) ||
			!reader.Read_U8(rawOutgoing) ||
			rawOutgoing > 1u)
		{
			return false;
		}
		damage.isOutgoing = 0u != rawOutgoing;
		if (!Is_Valid_DamageEvent(damage))
			return false;
		decoded.DamageEvents.push_back(damage);
	}

    message = std::move(decoded);
    return true;
}
```

### G00-07-04. `Server/Public/GameRoom.h` full code

```cpp
#pragma once

#include "RoomCommand.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "ValtanBrain.h"

#include <cstddef>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	class CClientSession;

	class CGameRoom final
	{
		friend int Run_ServerGameplayContractTests();
	public:
		explicit CGameRoom(LostArk::Shared::WORLD_ID worldId);

		bool Enqueue(ROOM_COMMAND command);
		void Tick(float fixedDeltaSeconds);
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);

		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		[[nodiscard]] bool Is_Ready() const { return m_isReady; }
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	private:
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld);
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		/* 1 unless the player is standing in the stance its identity gauge pays
		for, which is the only thing that changes how fast anyone walks. */
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		void Update_Players(float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalog m_GameplayCatalog;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CMonsterBrain m_MonsterBrain;
		CValtanBrain m_ValtanBrain;
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		/* One tick's resolved hits. Cleared at the top of every simulation phase
		and consumed by Broadcast_WorldSnapshot, so an event can only ever ride
		the snapshot of the tick that produced it. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		std::string m_strStatus;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
	};
}
```

### G00-07-05. `Server/Private/GameRoom.cpp` full code

```cpp
#include "GameRoom.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
	using namespace LostArk::Shared;
	using namespace LostArk::Server;

	constexpr float MAX_ABS_MOVE_GOAL = 10000.f;
	constexpr float MOVE_STOP_DISTANCE = 0.05f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;
	constexpr float PLAYER_TURN_DEGREES_PER_SECOND = 540.f;
	constexpr float DIRECT_BEARING_DISTANCE = 1.5f;

	float Wrap_Degrees(float degrees)
	{
		while (degrees > 180.f)
			degrees -= 360.f;
		while (degrees < -180.f)
			degrees += 360.f;
		return degrees;
	}
#ifdef _DEBUG
	constexpr std::uint32_t CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS = 90u;

	constexpr std::uint32_t Add_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick,
		const std::uint32_t elapsedTicks)
	{
		constexpr std::uint64_t SERVER_TICK_CARDINALITY =
			(static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()));
		return static_cast<std::uint32_t>(
			((static_cast<std::uint64_t>(startTick - 1u) + elapsedTicks) %
				SERVER_TICK_CARDINALITY) + 1u);
	}

	static_assert(91u == Add_ServerTicksSkippingReservedZero(1u, 90u));
	static_assert(90u == Add_ServerTicksSkippingReservedZero(
		(std::numeric_limits<std::uint32_t>::max)(), 90u));
	static_assert(1u == Add_ServerTicksSkippingReservedZero(
		(std::numeric_limits<std::uint32_t>::max)() - 89u, 90u));
#endif

	bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message)
	{
		return message.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			Is_Known_World_Id(message.eWorldId) &&
			Is_Supported_Playable_Character_Class(message.eCharacterClass) &&
			!message.strNickName.empty() &&
			message.strNickName.size() <= MAX_NICKNAME_BYTES;
	}

	bool Is_NewerSequence(
		const std::uint32_t candidate,
		const std::uint32_t previous)
	{
		return 0u != candidate &&
			static_cast<std::int32_t>(candidate - previous) > 0;
	}

	void Smooth_MovePath(
		const CServerNavigation& navigation,
		const float startX,
		const float startZ,
		const float goalX,
		const float goalZ,
		std::vector<SERVER_NAV_POINT>& path)
	{
		if (path.empty())
			return;
		SERVER_NAV_POINT exactGoal{};
		if (navigation.Sample_Position(goalX, goalZ, exactGoal) &&
			navigation.Has_LineOfSight(
				path.back().x, path.back().z, goalX, goalZ))
		{
			path.back() = exactGoal;
		}
		std::vector<SERVER_NAV_POINT> smoothed;
		smoothed.reserve(path.size());
		float fromX = startX;
		float fromZ = startZ;
		std::size_t index = 0;
		while (index < path.size())
		{
			std::size_t visible = index;
			for (std::size_t candidate = path.size();
				candidate > index + 1; --candidate)
			{
				const SERVER_NAV_POINT& point = path[candidate - 1];
				if (navigation.Has_LineOfSight(fromX, fromZ, point.x, point.z))
				{
					visible = candidate - 1;
					break;
				}
			}
			smoothed.push_back(path[visible]);
			fromX = path[visible].x;
			fromZ = path[visible].z;
			index = visible + 1;
		}
		path = std::move(smoothed);
	}

	WORLD_ENTITY_KIND To_NetworkKind(const WORLD_BOOTSTRAP_KIND kind)
	{
		switch (kind)
		{
		case WORLD_BOOTSTRAP_KIND::NPC: return WORLD_ENTITY_KIND::NPC;
		case WORLD_BOOTSTRAP_KIND::BOSS: return WORLD_ENTITY_KIND::BOSS;
		case WORLD_BOOTSTRAP_KIND::MONSTER: return WORLD_ENTITY_KIND::MONSTER;
		default: return WORLD_ENTITY_KIND::END;
		}
	}

	WORLD_ENTITY_ACTION To_NetworkAction(const SERVER_ENTITY_ACTION action)
	{
		switch (action)
		{
		case SERVER_ENTITY_ACTION::IDLE: return WORLD_ENTITY_ACTION::IDLE;
		case SERVER_ENTITY_ACTION::CHASE: return WORLD_ENTITY_ACTION::CHASE;
		case SERVER_ENTITY_ACTION::PATTERN_WINDUP: return WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		case SERVER_ENTITY_ACTION::PATTERN_ACTIVE: return WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		case SERVER_ENTITY_ACTION::PATTERN_RECOVERY: return WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		case SERVER_ENTITY_ACTION::DEAD: return WORLD_ENTITY_ACTION::DEAD;
		default: return WORLD_ENTITY_ACTION::END;
		}
	}
}

LostArk::Server::CGameRoom::CGameRoom(
	const LostArk::Shared::WORLD_ID worldId)
	: m_eWorldId(worldId)
{
	if (!LostArk::Shared::Is_Known_World_Id(worldId))
	{
		m_strStatus = "Unknown room world ID";
		return;
	}
	if (!m_WorldBootstrap.Load(worldId))
	{
		m_strStatus = m_WorldBootstrap.Get_Status();
		return;
	}
	if (!m_GameplayCatalog.Load())
	{
		m_strStatus = m_GameplayCatalog.Get_Status();
		return;
	}
	if (!m_SpawnGroupBootstrap.Load(worldId))
	{
		m_strStatus = m_SpawnGroupBootstrap.Get_Status();
		return;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, m_strStatus))
		return;
	if ((LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
		return;
	}
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}
	if (!m_ServerCollisionSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (placement.isEnabled &&
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
			!m_ServerCollisionSystem.Is_PlayerSpawnClear(placement))
		{
			m_strStatus = "Player spawn overlaps a collision box: " +
				placement.strPlacementId;
			return;
		}
	}
	if (!Initialize_WorldEntities())
		return;
	if (nullptr == Find_AvailablePlayerSpawn())
	{
		m_strStatus = "World bootstrap has no enabled player spawn";
		return;
	}

	m_isReady = true;
	m_strStatus = m_WorldBootstrap.Get_Status();
}

bool LostArk::Server::CGameRoom::Enqueue(ROOM_COMMAND command)
{
	if (!m_isReady || command.iSessionId == INVALID_SESSION_ID)
		return false;
	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession ||
			command.pSession->Get_SessionId() != command.iSessionId))
	{
		return false;
	}

	std::scoped_lock lock{ m_CommandMutex };
	m_InboundCommands.push_back(std::move(command));
	return true;
}

bool LostArk::Server::CGameRoom::Try_DequeueWorldTransfer(
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	if (m_PendingWorldTransfers.empty())
		return false;
	outTransfer = std::move(m_PendingWorldTransfers.front());
	m_PendingWorldTransfers.pop_front();
	return true;
}

void LostArk::Server::CGameRoom::Tick(const float fixedDeltaSeconds)
{
	if (!m_isReady)
		return;

	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{ m_CommandMutex };
		commands.swap(m_InboundCommands);
	}

	for (ROOM_COMMAND& command : commands)
	{
		switch (command.eType)
		{
		case ROOM_COMMAND_TYPE::REGISTER_SESSION:
			Handle_Register(command.pSession);
			break;
		case ROOM_COMMAND_TYPE::ENTER_WORLD:
			Join(command.iSessionId, command.EnterWorld);
			break;
		case ROOM_COMMAND_TYPE::MOVE:
			Handle_Move(command.iSessionId, command.Move);
			break;
		case ROOM_COMMAND_TYPE::USE_SKILL:
			Handle_UseSkill(command.iSessionId, command.UseSkill);
			break;
		case ROOM_COMMAND_TYPE::RELEASE_SKILL:
			Handle_ReleaseSkill(command.iSessionId, command.ReleaseSkill);
			break;
		case ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM:
			Handle_UpdateSkillAim(command.iSessionId, command.UpdateSkillAim);
			break;
		case ROOM_COMMAND_TYPE::REVIVE_PLAYER:
			Handle_RevivePlayer(command.iSessionId, command.RevivePlayer);
			break;
		case ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS:
			Handle_ChangeCharacterClass(
				command.iSessionId, command.ChangeCharacterClass);
			break;
		case ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY:
			Handle_SpawnWorldEntity(
				command.iSessionId,
				command.SpawnWorldEntity);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			Leave(command.iSessionId, command.eLeaveReason);
			break;
		}
	}

	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
		return;

	m_TickDamageEvents.clear();
	Update_Players(fixedDeltaSeconds);
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	m_ServerTriggerSystem.Evaluate_Entries(
		m_Players,
		updateTick,
		transfers,
		[this](const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
				return m_SpawnGroupRuntime.Activate(targetId);
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
				return Activate_Encounter(targetId);
			return false;
		});
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		Leave(
			transfer.iSessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
		m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	m_SpawnGroupRuntime.Update(
		fixedDeltaSeconds,
		m_SpawnGroupBootstrap,
		[this](const std::string& spawnGroupId)
		{
			return Count_SpawnGroupEntities(spawnGroupId);
		},
		[this](const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			const std::uint32_t ordinal)
		{
			return Spawn_Monster(
				spawnGroupId, entry, anchor, profile, ordinal);
		});
	Update_WorldEntities(fixedDeltaSeconds);
	++m_iServerTick;
	if (0u == m_iServerTick)
		m_iServerTick = 1u;
	if (!m_Players.empty())
		Broadcast_WorldSnapshot();
}

void LostArk::Server::CGameRoom::Handle_Register(
	const std::shared_ptr<CClientSession>& session)
{
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
		return;
	m_Sessions.insert_or_assign(session->Get_SessionId(), session);
}

bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld)
{
	using namespace LostArk::Shared;

	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
			session->Request_Close();
		return false;
	}
	if (Is_PlayerAdmissionFull())
	{
		Send_EnterRejected(
			session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL);
		session->Request_Close();
		return false;
	}
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = Find_AvailablePlayerSpawn();
	if (nullptr == spawn)
	{
		session->Request_Close();
		return false;
	}

	SERVER_PLAYER player{};
	player.iSessionId = sessionId;
	player.iPlayerId = m_iNextPlayerId;
	player.iNetEntityId = m_iNextNetEntityId;
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	player.strSpawnPlacementId = spawn->strPlacementId;
	player.fPositionX = spawn->fPositionX;
	player.fPositionY = spawn->fPositionY;
	player.fPositionZ = spawn->fPositionZ;
	player.fYawDegrees = spawn->fYawDegrees;
	const PLAYER_RUNTIME_PROFILE* playerProfile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == playerProfile)
	{
		session->Request_Close();
		return false;
	}
	player.eStance = playerProfile->eDefaultStance;
	player.iCurrentHp = playerProfile->iMaximumHp;
	player.iMaximumHp = playerProfile->iMaximumHp;
	player.iCurrentResource = playerProfile->iMaximumResource;
	player.iMaximumResource = playerProfile->iMaximumResource;
	player.fMoveSpeed = playerProfile->fMoveSpeed;
	// A gauge starts full: the stance it pays for is available on entry, and a
	// class without one keeps both at 0 so nothing ever drains.
	player.iMaximumIdentity = playerProfile->iMaximumIdentity;
	player.iCurrentIdentity = playerProfile->iMaximumIdentity;
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;
	if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(
			player.fPositionX, player.fPositionZ, projected))
		{
			session->Request_Close();
			return false;
		}
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}

	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	m_Players.emplace(player.iPlayerId, player);
	m_PlayerIdBySessionId.emplace(sessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	session->Bind_PlayerId(player.iPlayerId);

	if (!Send_Accepted(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (!Send_WorldEntitySpawned(session, entity))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
	for (const auto& [existingPlayerId, existingPlayer] : m_Players)
	{
		(void)existingPlayerId;
		if (existingPlayer.iSessionId == sessionId)
			continue;
		if (!Send_Spawned(session, existingPlayer))
		{
			Rollback_Join(sessionId);
			session->Request_Close();
			return false;
		}
	}
	if (!Send_Spawned(session, player))
	{
		Rollback_Join(sessionId);
		session->Request_Close();
		return false;
	}
	Broadcast_Spawned(player, sessionId);

	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", PlayerId=" << player.iPlayerId
		<< ", Spawn=" << player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;

	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
	{
		m_Sessions.erase(sessionId);
		return;
	}
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter == m_Players.end())
	{
		m_PlayerIdBySessionId.erase(sessionPlayerIter);
		m_Sessions.erase(sessionId);
		return;
	}

	const NET_ENTITY_ID netEntityId = playerIter->second.iNetEntityId;
	m_ServerTriggerSystem.Remove_Player(playerId);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	Broadcast_Despawned(netEntityId, reason);

	std::cout << "Player left. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", RoomPlayers=" << m_Players.size() << '\n';

	if (!Reset_ReplayableArenaWhenEmpty())
	{
		std::cerr << "Replayable arena reset failed. World="
			<< static_cast<unsigned>(m_eWorldId) << ", Status="
			<< m_strStatus << '\n';
	}
}

void LostArk::Server::CGameRoom::Handle_Move(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_MOVE& move)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Is_NewerSequence(move.iClientSequence, player.iLastMoveSequence) ||
		!std::isfinite(move.fGoalX) || !std::isfinite(move.fGoalZ) ||
		std::abs(move.fGoalX) > MAX_ABS_MOVE_GOAL ||
		std::abs(move.fGoalZ) > MAX_ABS_MOVE_GOAL)
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}

	player.iLastMoveSequence = move.iClientSequence;
	if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp)
	{
		return;
	}
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	if (m_ServerNavigation.Is_Loaded())
	{
		if (!m_ServerNavigation.Find_Path(
			player.fPositionX,
			player.fPositionZ,
			move.fGoalX,
			move.fGoalZ,
			player.MovePath))
		{
			player.hasMoveGoal = false;
			return;
		}
		Smooth_MovePath(
			m_ServerNavigation,
			player.fPositionX,
			player.fPositionZ,
			move.fGoalX,
			move.fGoalZ,
			player.MovePath);
		const SERVER_NAV_POINT& goal = player.MovePath.back();
		player.fMoveGoalX = goal.x;
		player.fMoveGoalZ = goal.z;
	}
	else
	{
		player.fMoveGoalX = move.fGoalX;
		player.fMoveGoalZ = move.fGoalZ;
	}
	player.hasMoveGoal = true;
	player.isCombatReady = true;
}

void LostArk::Server::CGameRoom::Handle_UseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SKILL& useSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	const std::uint32_t actionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
#ifdef _DEBUG
	/* Character Select Server Arena is the presentation audition room.  Keep
	its retries Server-authoritative with a fixed three-second audition cooldown
	and full resources; action-running, sequence, class, aim and snapshot gates
	remain in CPlayerSkillSystem::Try_Start.  Release rooms retain authored
	balance. */
	if (LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId)
	{
		playerIter->second.iCurrentResource =
			playerIter->second.iMaximumResource;
		playerIter->second.iResourceAccumulator = 0u;
	}
#endif
	// A valid but currently unavailable skill is rejected as gameplay state;
	// malformed payloads are already closed at the ServerApp packet boundary.
	if (m_PlayerSkillSystem.Try_Start(
		playerIter->second,
		useSkill,
		m_GameplayCatalog,
		actionStartTick))
	{
#ifdef _DEBUG
		if (LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId)
		{
			playerIter->second.CooldownEndTickBySkillId.insert_or_assign(
				useSkill.iSkillId,
				Add_ServerTicksSkippingReservedZero(
					actionStartTick,
					CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS));
		}
#endif
		playerIter->second.isCombatReady = true;
	}
}

void LostArk::Server::CGameRoom::Handle_RevivePlayer(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer)
{
	using namespace LostArk::Shared;
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		sessionIter == m_PlayerIdBySessionId.end())
	{
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Is_NewerSequence(
		revivePlayer.iClientSequence, player.iLastReviveSequence))
	{
		return;
	}
	player.iLastReviveSequence = revivePlayer.iClientSequence;
	if (0u != player.iCurrentHp || PLAYER_ACTION_STATE::DEAD != player.eAction)
		return;

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return;
	player.iCurrentHp = player.iMaximumHp;
	player.iCurrentResource = player.iMaximumResource;
	player.iResourceAccumulator = 0u;
	player.iCurrentIdentity = player.iMaximumIdentity;
	player.iIdentityAccumulator = 0u;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.eStance = profile->eDefaultStance;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.fSkillAimDirectionX = 0.f;
	player.fSkillAimDirectionZ = 1.f;
	player.hasAppliedSkillDamage = false;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.CooldownEndTickBySkillId.clear();
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.TriggerMove = {};
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
}

void LostArk::Server::CGameRoom::Handle_ReleaseSkill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	m_PlayerSkillSystem.Release(
		playerIter->second,
		releaseSkill,
		m_GameplayCatalog);
}

void LostArk::Server::CGameRoom::Handle_UpdateSkillAim(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
	{
		if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
			session->Request_Close();
		return;
	}
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	m_PlayerSkillSystem.Update_Aim(
		playerIter->second,
		updateSkillAim,
		m_GameplayCatalog);
}

LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT
LostArk::Server::CGameRoom::Apply_CharacterClassChange(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD;
	if (!Is_NewerSequence(
		request.iClientSequence, player.iLastClassChangeSequence))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE;
	}
	if (!Is_Supported_Playable_Character_Class(request.eCharacterClass) ||
		nullptr == m_GameplayCatalog.Find_Player(request.eCharacterClass))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_UNSUPPORTED_CLASS;
	}
	if (request.eCharacterClass == player.eCharacterClass)
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_SAME_CLASS;

	const bool isDead = 0u == player.iCurrentHp &&
		PLAYER_ACTION_STATE::DEAD == player.eAction;
	if ((0u == player.iCurrentHp) !=
		(PLAYER_ACTION_STATE::DEAD == player.eAction))
	{
		return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
	}

	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(request.eCharacterClass);
	SERVER_PLAYER staged = player;
	if (isDead)
	{
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			Find_Placement(player.strSpawnPlacementId);
		if (nullptr == spawn || !spawn->isEnabled ||
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn->eKind)
		{
			return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
		}
		staged.fPositionX = spawn->fPositionX;
		staged.fPositionY = spawn->fPositionY;
		staged.fPositionZ = spawn->fPositionZ;
		staged.fYawDegrees = spawn->fYawDegrees;
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT projected{};
			if (!m_ServerNavigation.Project_Point(
				staged.fPositionX, staged.fPositionZ, projected))
			{
				return CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STATE;
			}
			staged.fPositionX = projected.x;
			staged.fPositionY = projected.y;
			staged.fPositionZ = projected.z;
		}
	}

	staged.eCharacterClass = request.eCharacterClass;
	staged.iLastClassChangeSequence = request.iClientSequence;
	staged.fMoveGoalX = 0.f;
	staged.fMoveGoalZ = 0.f;
	staged.fMoveSpeed = profile->fMoveSpeed;
	staged.hasMoveGoal = false;
	staged.MovePath.clear();
	staged.iMovePathIndex = 0u;
	staged.iCurrentHp = profile->iMaximumHp;
	staged.iMaximumHp = profile->iMaximumHp;
	staged.iCurrentResource = profile->iMaximumResource;
	staged.iMaximumResource = profile->iMaximumResource;
	staged.iResourceAccumulator = 0u;
	staged.eAction = PLAYER_ACTION_STATE::NONE;
	staged.eStance = profile->eDefaultStance;
	staged.iCurrentSkillId = INVALID_SKILL_ID;
	staged.iActionStartTick = 0u;
	staged.TriggerMove = {};
	staged.fActionElapsedSeconds = 0.f;
	staged.fSkillAimDirectionX = 0.f;
	staged.fSkillAimDirectionZ = 1.f;
	staged.hasAppliedSkillDamage = false;
	staged.iComboStage = 0u;
	staged.hasBufferedComboInput = false;
	staged.hasReleasedHold = false;
	staged.CooldownEndTickBySkillId.clear();
	staged.isCombatReady = true;

	player = std::move(staged);
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED;
}

void LostArk::Server::CGameRoom::Handle_ChangeCharacterClass(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	const CHARACTER_CLASS_CHANGE_RESULT result =
		Apply_CharacterClassChange(player, request);
	if (!Send_CharacterClassChangeResult(
		session, request, result, player.eCharacterClass))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_SpawnWorldEntity(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId ||
		!m_PlayerIdBySessionId.contains(sessionId) || nullptr == session)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* placement =
		Find_Placement(request.strPlacementId);
	if (nullptr == placement)
	{
		const auto group = std::find_if(
			m_SpawnGroupBootstrap.Get_Groups().begin(),
			m_SpawnGroupBootstrap.Get_Groups().end(),
			[&request](const SPAWN_GROUP_DEFINITION& definition)
			{
				return definition.strSpawnGroupId == request.strPlacementId;
			});
		if (m_SpawnGroupBootstrap.Get_Groups().end() == group)
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}
		if (!m_SpawnGroupRuntime.Is_ActiveOrCompleted(request.strPlacementId) &&
			!m_SpawnGroupRuntime.Activate_Immediate(
				request.strPlacementId,
				m_SpawnGroupBootstrap,
				[this](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				}))
		{
			Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::REJECTED,
				INVALID_NET_ENTITY_ID);
			return;
		}

		if (!Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::ACTIVATED,
			INVALID_NET_ENTITY_ID))
		{
			session->Request_Close();
		}
		return;
	}

	if (placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		placement->strArchetypeId != "BOSS_VALTAN")
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}
	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == request.strPlacementId;
		});
	if (m_WorldEntities.end() != existing)
	{
		if (!Send_WorldEntitySpawned(session, *existing) ||
			!Send_WorldEntitySpawnResult(
				session,
				request.strPlacementId,
				WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS,
				existing->iNetEntityId))
		{
			session->Request_Close();
		}
		return;
	}
	if (m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
	{
		Send_WorldEntitySpawnResult(
			session,
			request.strPlacementId,
			WORLD_ENTITY_SPAWN_RESULT::REJECTED,
			INVALID_NET_ENTITY_ID);
		return;
	}

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	if (!Send_WorldEntitySpawnResult(
		session,
		request.strPlacementId,
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED,
		m_WorldEntities.back().iNetEntityId))
	{
		session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_Accepted(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_ENTER_ACCEPTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_EnterRejected(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_REJECTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Spawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_SPAWNED message{};
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.eCharacterClass = player.eCharacterClass;
	message.strNickName = player.strNickName;
	message.fPositionX = player.fPositionX;
	message.fPositionY = player.fPositionY;
	message.fPositionZ = player.fPositionZ;
	message.fYawDegrees = player.fYawDegrees;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_WORLD_ENTITY& entity)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED message{};
	message.iNetEntityId = entity.iNetEntityId;
	message.eKind = To_NetworkKind(entity.eKind);
	message.strArchetypeId = entity.strArchetypeId;
	message.strEncounterId = entity.strEncounterId;
	message.strPlacementId = entity.strPlacementId;
	message.fPositionX = entity.fPositionX;
	message.fPositionY = entity.fPositionY;
	message.fPositionZ = entity.fPositionZ;
	message.fYawDegrees = entity.fYawDegrees;
	message.fCollisionRadius = entity.fCollisionRadius;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Despawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_Spawned(
	const SERVER_PLAYER& player,
	const SESSION_ID exceptSessionId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		if (sessionId == exceptSessionId)
			continue;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Spawned(session, player))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_Despawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Despawned(session, netEntityId, reason))
			session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawnResult(
	const std::shared_ptr<CClientSession>& session,
	const std::string& placementId,
	const LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWN_RESULT message{};
	message.strPlacementId = placementId;
	message.eResult = result;
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CharacterClassChangeResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
	const LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
	const LostArk::Shared::CHARACTER_CLASS_ID activeClass)
{
	using namespace LostArk::Shared;
	S2C_CHARACTER_CLASS_CHANGE_RESULT message{};
	message.iClientSequence = request.iClientSequence;
	message.eResult = result;
	message.eRequestedClass = request.eCharacterClass;
	message.eActiveClass = activeClass;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT,
			writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntitySpawned(
	const SERVER_WORLD_ENTITY& entity)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldEntitySpawned(session, entity))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntityDespawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session &&
			!Send_WorldEntityDespawned(session, netEntityId))
		{
			session->Request_Close();
		}
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldSnapshot()
{
	using namespace LostArk::Shared;
	S2C_WORLD_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.eWorldId = m_eWorldId;
	message.Players.reserve(m_Players.size());
	message.Entities.reserve(m_WorldEntities.size());
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		PLAYER_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = player.iNetEntityId;
		snapshot.eCharacterClass = player.eCharacterClass;
		snapshot.fPositionX = player.fPositionX;
		snapshot.fPositionY = player.fPositionY;
		snapshot.fPositionZ = player.fPositionZ;
		snapshot.fYawDegrees = player.fYawDegrees;
		snapshot.eLocomotionState =
			(player.hasMoveGoal || player.TriggerMove.isActive) ?
			PLAYER_LOCOMOTION_STATE::MOVING : PLAYER_LOCOMOTION_STATE::IDLE;
		snapshot.eAction = player.eAction;
		snapshot.eStance = player.eStance;
		snapshot.iSkillId = player.iCurrentSkillId;
		snapshot.iActionStartTick = player.iActionStartTick;
		snapshot.iCurrentHp = player.iCurrentHp;
		snapshot.iMaximumHp = player.iMaximumHp;
		snapshot.iCurrentResource = player.iCurrentResource;
		snapshot.iMaximumResource = player.iMaximumResource;
		snapshot.iCurrentIdentity = player.iCurrentIdentity;
		snapshot.iMaximumIdentity = player.iMaximumIdentity;
		snapshot.isCombatReady = player.isCombatReady;
		snapshot.iComboStage = player.iComboStage;
		/* Collect, sort, then truncate: cutting during unordered_map iteration
		made the surviving cooldowns depend on hash order. Signed difference keeps
		ordering across a wrapped tick counter. */
		for (const auto& [skillId, cooldownEndTick] :
			player.CooldownEndTickBySkillId)
		{
			if (static_cast<std::int32_t>(cooldownEndTick - m_iServerTick) > 0)
				snapshot.Cooldowns.push_back({ skillId, cooldownEndTick });
		}
		std::sort(snapshot.Cooldowns.begin(), snapshot.Cooldowns.end(),
			[](const SKILL_COOLDOWN_SNAPSHOT& left,
				const SKILL_COOLDOWN_SNAPSHOT& right)
			{
				return left.iSkillId < right.iSkillId;
			});
		if (snapshot.Cooldowns.size() > MAX_PLAYER_COOLDOWNS)
			snapshot.Cooldowns.resize(MAX_PLAYER_COOLDOWNS);
		message.Players.push_back(snapshot);
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		WORLD_ENTITY_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = entity.iNetEntityId;
		snapshot.eAction = To_NetworkAction(entity.eAction);
		snapshot.strPatternId = entity.strPatternId;
		if (entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY)
		{
			snapshot.strActionId = entity.strActionId;
		}
		snapshot.fPositionX = entity.fPositionX;
		snapshot.fPositionY = entity.fPositionY;
		snapshot.fPositionZ = entity.fPositionZ;
		snapshot.fYawDegrees = entity.fYawDegrees;
		snapshot.iActionStartTick = entity.iActionStartTick;
		snapshot.iPatternSequence = entity.iPatternSequence;
		snapshot.iPatternStageIndex = entity.iPatternStageIndex;
		snapshot.iCurrentHp = entity.iCurrentHp;
		snapshot.iMaximumHp = entity.iMaximumHp;
		snapshot.iPhase = entity.iPhase;
		message.Entities.push_back(std::move(snapshot));
	}
	message.DamageEvents = m_TickDamageEvents;

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
}

std::shared_ptr<LostArk::Server::CClientSession>
LostArk::Server::CGameRoom::Find_Session(const SESSION_ID sessionId) const
{
	const auto iter = m_Sessions.find(sessionId);
	return iter == m_Sessions.end() ? nullptr : iter->second.lock();
}

void LostArk::Server::CGameRoom::Rollback_Join(const SESSION_ID sessionId)
{
	using namespace LostArk::Shared;
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter != m_Players.end())
	{
		m_PlayerIdByEntityId.erase(playerIter->second.iNetEntityId);
		m_Players.erase(playerIter);
	}
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
}

bool LostArk::Server::CGameRoom::Is_PlayerAdmissionFull() const
{
	return nullptr == Find_AvailablePlayerSpawn();
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_AvailablePlayerSpawn() const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN)
		{
			continue;
		}
		const bool isUsed = std::any_of(
			m_Players.begin(), m_Players.end(),
			[&placement](const auto& playerEntry)
			{
				return playerEntry.second.strSpawnPlacementId ==
					placement.strPlacementId;
			});
		if (!isUsed)
			return &placement;
	}
	return nullptr;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_Placement(
	const std::string& placementId) const
{
	const auto& placements = m_WorldBootstrap.Get_Placements();
	const auto iter = std::find_if(
		placements.begin(),
		placements.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == placementId;
		});
	return placements.end() != iter ? &*iter : nullptr;
}

bool LostArk::Server::CGameRoom::Build_WorldEntity(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	SERVER_WORLD_ENTITY& outEntity)
{
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::END == placement.eKind)
	{
		m_strStatus = "World entity placement is invalid";
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = netEntityId;
	staged.strPlacementId = placement.strPlacementId;
	staged.strArchetypeId = placement.strArchetypeId;
	staged.strEncounterId = placement.strEncounterId;
	staged.eKind = placement.eKind;
	staged.fPositionX = placement.fPositionX;
	staged.fPositionY = placement.fPositionY;
	staged.fPositionZ = placement.fPositionZ;
	staged.fYawDegrees = placement.fYawDegrees;
	if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)
	{
		const BOSS_RUNTIME_PROFILE* profile =
			m_GameplayCatalog.Find_Boss(staged.strArchetypeId);
		const auto* patterns =
			m_GameplayCatalog.Find_BossPatterns(staged.strEncounterId);
		if (nullptr == profile ||
			profile->strEncounterId != staged.strEncounterId ||
			nullptr == patterns || patterns->empty())
		{
			m_strStatus = "Boss gameplay profile or damage profile is missing";
			return false;
		}
		staged.iCurrentHp = profile->iMaximumHp;
		staged.iMaximumHp = profile->iMaximumHp;
		staged.iMaximumHealthBars = profile->iMaximumHealthBars;
		staged.iLastEvaluatedHealthBar = profile->iMaximumHealthBars;
		staged.fCollisionRadius = profile->fCollisionRadius;
		staged.fEngageDistance = profile->fEngageDistance;
		staged.fMoveSpeed = profile->fMoveSpeed;
		staged.iPhaseTwoHpPercent = profile->iPhaseTwoHpPercent;
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT projected{};
			if (!m_ServerNavigation.Project_Point(
				staged.fPositionX,
				staged.fPositionZ,
				projected))
			{
				m_strStatus = "Boss placement is outside server navigation";
				return false;
			}
			staged.fPositionX = projected.x;
			staged.fPositionY = projected.y;
			staged.fPositionZ = projected.z;
		}
	}
	outEntity = std::move(staged);
	return true;
}

bool LostArk::Server::CGameRoom::Initialize_WorldEntities()
{
	m_WorldEntities.clear();
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::COLLISION_BOX)
		{
			continue;
		}
		if (m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		{
			m_strStatus = "World entity ID space exhausted";
			return false;
		}
		SERVER_WORLD_ENTITY entity{};
		if (!Build_WorldEntity(placement, m_iNextNetEntityId, entity))
			return false;
		++m_iNextNetEntityId;
		m_WorldEntities.push_back(std::move(entity));
	}
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ReplayableArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if ((WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId &&
		WORLD_ID::VALTAN_ARENA != m_eWorldId) || !m_Players.empty())
		return true;

	std::string resetStatus;
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		m_isReady = false;
		return false;
	}
	if (!Initialize_WorldEntities())
	{
		m_isReady = false;
		return false;
	}
	m_TickDamageEvents.clear();
	m_strStatus = "Replayable arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Activate_Encounter(
	const std::string& placementId)
{
	const WORLD_BOOTSTRAP_PLACEMENT* placement = Find_Placement(placementId);
	if (nullptr == placement ||
		placement->isEnabled ||
		WORLD_BOOTSTRAP_KIND::BOSS != placement->eKind ||
		m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	const auto existing = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&placementId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.strPlacementId == placementId;
		});
	if (m_WorldEntities.end() != existing)
		return false;

	SERVER_WORLD_ENTITY staged{};
	if (!Build_WorldEntity(*placement, m_iNextNetEntityId, staged))
		return false;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

bool LostArk::Server::CGameRoom::Spawn_Monster(
	const std::string& spawnGroupId,
	const SPAWN_GROUP_ENTRY& entry,
	const SPAWN_GROUP_ANCHOR& anchor,
	const MONSTER_RUNTIME_PROFILE& profile,
	const std::uint32_t ordinal)
{
	if (spawnGroupId.empty() ||
		entry.strArchetypeId != profile.strArchetypeId ||
		m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	SERVER_NAV_POINT projected{
		anchor.fPositionX, anchor.fPositionY, anchor.fPositionZ };
	if (m_ServerNavigation.Is_Loaded() &&
		!m_ServerNavigation.Project_Point(
			anchor.fPositionX, anchor.fPositionZ, projected))
	{
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = m_iNextNetEntityId;
	staged.strPlacementId = spawnGroupId + "." +
		std::to_string(staged.iNetEntityId) + "." + std::to_string(ordinal);
	staged.strArchetypeId = profile.strArchetypeId;
	staged.strSpawnGroupId = spawnGroupId;
	staged.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
	staged.eAction = SERVER_ENTITY_ACTION::IDLE;
	staged.fPositionX = projected.x;
	staged.fPositionY = projected.y;
	staged.fPositionZ = projected.z;
	staged.fYawDegrees = anchor.fYawDegrees;
	staged.iCurrentHp = profile.iMaxHp;
	staged.iMaximumHp = profile.iMaxHp;
	staged.iAttackPower = profile.iAttackPower;
	staged.iDefense = profile.iDefense;
	staged.fCollisionRadius = profile.fCollisionRadius;
	staged.fEngageDistance = profile.fEngageRange;
	staged.fMoveSpeed = profile.fMoveSpeed;
	staged.fAttackRange = profile.fAttackRange;
	staged.iPatternTelegraphMs = profile.iAttackWindupMs;
	staged.iPatternActiveMs = profile.iAttackActiveMs;
	staged.iPatternRecoveryMs = profile.iAttackRecoveryMs;
	staged.iDeadDespawnMs = profile.iDeadDespawnMs;

	++m_iNextNetEntityId;
	m_WorldEntities.push_back(std::move(staged));
	Broadcast_WorldEntitySpawned(m_WorldEntities.back());
	return true;
}

std::uint32_t LostArk::Server::CGameRoom::Count_SpawnGroupEntities(
	const std::string& spawnGroupId) const
{
	return static_cast<std::uint32_t>(std::count_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&spawnGroupId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER &&
				entity.strSpawnGroupId == spawnGroupId;
		}));
}

float LostArk::Server::CGameRoom::Resolve_StanceMoveSpeedScale(
	const SERVER_PLAYER& player) const
{
	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile ||
		!CPlayerSkillSystem::Is_HoldingGaugedStance(player, *profile))
	{
		return 1.f;
	}
	return profile->fDefenseStanceMoveSpeedScale;
}

void LostArk::Server::CGameRoom::Update_Players(const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			continue;
		}
		m_PlayerSkillSystem.Update(
			player,
			m_WorldEntities,
			m_GameplayCatalog,
			m_ServerNavigation.Is_Loaded() ? &m_ServerNavigation : nullptr,
			fixedDeltaSeconds,
			updateTick,
			m_TickDamageEvents);
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
			continue;
		if (!player.hasMoveGoal)
			continue;
		float targetX = player.fMoveGoalX;
		float targetY = player.fPositionY;
		float targetZ = player.fMoveGoalZ;
		if (player.iMovePathIndex < player.MovePath.size())
		{
			const SERVER_NAV_POINT& pathPoint =
				player.MovePath[player.iMovePathIndex];
			targetX = pathPoint.x;
			targetY = pathPoint.y;
			targetZ = pathPoint.z;
		}
		const float deltaX = targetX - player.fPositionX;
		const float deltaZ = targetZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const bool reachedPathPoint = distance <= MOVE_STOP_DISTANCE;
		float proposedX = targetX;
		float proposedY = targetY;
		float proposedZ = targetZ;
		if (!reachedPathPoint)
		{
			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float yawDifference =
				Wrap_Degrees(desiredYaw - player.fYawDegrees);
			const float maxYawStep =
				PLAYER_TURN_DEGREES_PER_SECOND * fixedDeltaSeconds;
			if (distance <= DIRECT_BEARING_DISTANCE ||
				std::abs(yawDifference) <= maxYawStep)
			{
				player.fYawDegrees = desiredYaw;
			}
			else
			{
				player.fYawDegrees = Wrap_Degrees(player.fYawDegrees +
					(yawDifference > 0.f ? maxYawStep : -maxYawStep));
			}
			const float moveDistance = (std::min)(
				player.fMoveSpeed * Resolve_StanceMoveSpeedScale(player) *
					fixedDeltaSeconds,
				distance);
			const float moveRatio = moveDistance / distance;
			float stepX = deltaX * moveRatio;
			float stepZ = deltaZ * moveRatio;
			if (player.fYawDegrees != desiredYaw)
			{
				const float headingRadians =
					player.fYawDegrees / RADIANS_TO_DEGREES;
				const float headingStepX =
					std::sin(headingRadians) * moveDistance;
				const float headingStepZ =
					std::cos(headingRadians) * moveDistance;
				SERVER_NAV_POINT walkable{};
				if (!m_ServerNavigation.Is_Loaded() ||
					m_ServerNavigation.Sample_Position(
						player.fPositionX + headingStepX,
						player.fPositionZ + headingStepZ,
						walkable))
				{
					stepX = headingStepX;
					stepZ = headingStepZ;
				}
				else
				{
					player.fYawDegrees = desiredYaw;
				}
			}
			proposedX = player.fPositionX + stepX;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + stepZ;
		}

		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(
			player,
			proposedX,
			proposedY,
			proposedZ,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked))
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (wasBlocked)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		if (reachedPathPoint)
		{
			if (player.iMovePathIndex < player.MovePath.size())
				++player.iMovePathIndex;
			if (player.iMovePathIndex >= player.MovePath.size())
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0;
			}
			continue;
		}
	}
}

void LostArk::Server::CGameRoom::Update_WorldEntities(
	const float fixedDeltaSeconds)
{
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
			m_ServerNavigation.Is_Loaded())
		{
			m_ValtanBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
		}
		else if (entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER &&
			m_ServerNavigation.Is_Loaded())
		{
			m_MonsterBrain.Update(
				entity,
				m_Players,
				m_GameplayCatalog,
				m_ServerNavigation,
				fixedDeltaSeconds,
				updateTick,
				m_TickDamageEvents);
		}
	}

	for (auto iter = m_WorldEntities.begin(); iter != m_WorldEntities.end();)
	{
		const bool shouldDespawn =
			WORLD_BOOTSTRAP_KIND::MONSTER == iter->eKind &&
			SERVER_ENTITY_ACTION::DEAD == iter->eAction &&
			iter->fActionElapsedSeconds * 1000.f >=
				static_cast<float>(iter->iDeadDespawnMs);
		if (!shouldDespawn)
		{
			++iter;
			continue;
		}
		Broadcast_WorldEntityDespawned(iter->iNetEntityId);
		iter = m_WorldEntities.erase(iter);
	}
}
```

### G00-07-06. `Server/Private/ServerGameplayContractTests.cpp` full code

```cpp
#include "ServerGameplayContractTests.h"

#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

namespace
{
	struct TESTS
	{
		void Require(const bool condition, const char* name)
		{
			std::cout << (condition ? "[PASS] " : "[FAILURE] ") << name << '\n';
			if (!condition)
				++failures;
		}
		int failures = 0;
	};

	struct QUICK_SKILL_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		const char* inputSlot;
	};

	constexpr std::array QUICK_SKILLS
	{
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34040, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34540, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34090, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34550, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34100, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34560, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34160, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34570, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34580, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34120, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34590, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34110, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34150, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34000, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34500, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34520, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34650, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34610, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34630, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38020, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38050, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38120, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38200, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38140, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38180, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38210, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38260, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38290, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38250, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38320, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45050, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45620, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45300, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45070, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45190, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45600, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45810, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45820, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31200, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31430, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31480, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31210, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31460, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31420, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31490, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31470, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31950, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31110, "X" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31050, "Z" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31910, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31930, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050100, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050120, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050160, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050180, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050210, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050220, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050240, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050230, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050500, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050520, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050540, "ALT_V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050020, "SPACE" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17030, "Q" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17060, "W" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17080, "E" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17110, "R" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17090, "A" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17040, "S" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17100, "D" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17140, "F" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17240, "T" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17820, "X" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17170, "V" },
		QUICK_SKILL_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17250, "ALT_V" }
	};

	struct BASIC_ATTACK_CONTRACT final
	{
		LostArk::Shared::CHARACTER_CLASS_ID characterClass;
		LostArk::Shared::SKILL_ID skillId;
		std::size_t stageCount;
	};

	constexpr std::array BASIC_ATTACKS
	{
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD, 17000, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34010, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER, 34510, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER, 38000, 3 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER, 45000, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, 31000, 4 },
		BASIC_ATTACK_CONTRACT{
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER, 2050010, 4 }
	};
}

int LostArk::Server::Run_ServerGameplayContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests{};
	CGameplayCatalog catalog;
	tests.Require(catalog.Load(), "Load gameplay balance bootstrap");
	{
		using namespace LostArk::Shared::CombatCollision;

		const CIRCLE_XZ circle{ 0.f, 0.f, 2.f };
		const BODY_CIRCLE_XZ tangentCircle{ 3.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ missedCircle{ 3.001f, 0.f, 1.f };
		tests.Require(
			Circles_Overlap(circle, tangentCircle) &&
			!Circles_Overlap(circle, missedCircle),
			"Treat circle tangency as contact and reject a separated circle");

		const BODY_CIRCLE_XZ innerRingTangent{ 2.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ insideRingHole{ 1.9f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outerRingTangent{ 6.f, 0.f, 1.f };
		const BODY_CIRCLE_XZ outsideRing{ 6.001f, 0.f, 1.f };
		tests.Require(
			Circle_IntersectsRing(innerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(insideRingHole, 0.f, 0.f, 3.f, 5.f) &&
			Circle_IntersectsRing(outerRingTangent, 0.f, 0.f, 3.f, 5.f) &&
			!Circle_IntersectsRing(outsideRing, 0.f, 0.f, 3.f, 5.f),
			"Respect both inclusive ring boundaries and reject both misses");

		const BODY_CIRCLE_XZ rotatedShapeHit{ 2.f, 2.f, 0.25f };
		const BODY_CIRCLE_XZ rotatedShapeMiss{ 0.f, 2.5f, 0.25f };
		tests.Require(
			Circle_IntersectsForwardBox(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsForwardBox(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a forward box in its rotated basis");
		tests.Require(
			Circle_IntersectsCone(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f) &&
			!Circle_IntersectsCone(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 5.f, 60.f),
			"Evaluate a cone in its rotated basis");
		tests.Require(
			Circle_IntersectsCross(
				rotatedShapeHit, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f) &&
			!Circle_IntersectsCross(
				rotatedShapeMiss, 0.f, 0.f, 1.f, 1.f, 4.f, 0.5f),
			"Evaluate a cross in its rotated basis");
	}
	{
		CServerNavigation navigation;
		const bool navigationLoaded =
			navigation.Load("LV_LOBBY_CLASSSELECT_SL00");
		SERVER_WORLD_ENTITY monster{};
		monster.iNetEntityId = 700u;
		monster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		monster.iCurrentHp = 100u;
		monster.iMaximumHp = 100u;
		monster.fCollisionRadius = 0.6f;
		monster.fAttackRange = 1.f;
		monster.fEngageDistance = 8.f;
		monster.fMoveSpeed = 2.f;
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER protectedPlayer{};
		protectedPlayer.iPlayerId = 701u;
		protectedPlayer.iNetEntityId = 702u;
		protectedPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		protectedPlayer.iCurrentHp = 100u;
		protectedPlayer.iMaximumHp = 100u;
		protectedPlayer.fPositionX = 1.f;
		protectedPlayer.isCombatReady = false;
		players.emplace(protectedPlayer.iPlayerId, protectedPlayer);
		std::vector<DAMAGE_EVENT> damageEvents;
		CMonsterBrain monsterBrain;
		monsterBrain.Update(
			monster, players, catalog, navigation, 1.f / 30.f, 1u, damageEvents);
		const bool ignoredProtectedPlayer =
			INVALID_NET_ENTITY_ID == monster.iTargetEntityId &&
			SERVER_ENTITY_ACTION::IDLE == monster.eAction;
		players.begin()->second.isCombatReady = true;
		monsterBrain.Update(
			monster, players, catalog, navigation, 1.f / 30.f, 2u, damageEvents);
		tests.Require(
			navigationLoaded && ignoredProtectedPlayer &&
			SERVER_ENTITY_ACTION::PATTERN_WINDUP == monster.eAction &&
			players.begin()->second.iNetEntityId == monster.iTargetEntityId,
			"Ignore protected players and acquire the same player after combat admission");
	}
	{
		CGameRoom room{ WORLD_ID::CHARACTER_SELECT_ARENA };
		tests.Require(room.Is_Ready(),
			"Initialize Character Select room for class changes");
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			room.Find_AvailablePlayerSpawn();
		tests.Require(nullptr != spawn,
			"Resolve Character Select class-change respawn placement");
		if (room.Is_Ready() && nullptr != spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = 11u;
			player.iPlayerId = 12u;
			player.iNetEntityId = 112u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "ClassSwitch";
			player.strSpawnPlacementId = spawn->strPlacementId;
			player.fPositionX = 17.f;
			player.fPositionY = 3.f;
			player.fPositionZ = -9.f;
			player.fYawDegrees = 33.f;
			player.iCurrentHp = 10u;
			player.iMaximumHp = 100u;
			player.iCurrentResource = 2u;
			player.iMaximumResource = 10u;
			player.eAction = PLAYER_ACTION_STATE::SKILL;
			player.iCurrentSkillId = 34120u;
			player.iActionStartTick = 9u;
			player.iLastMoveSequence = 7u;
			player.iLastSkillSequence = 8u;
			player.hasMoveGoal = true;
			player.CooldownEndTickBySkillId.emplace(34120u, 100u);

			C2S_CHANGE_CHARACTER_CLASS request{};
			request.iClientSequence = 1u;
			request.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass &&
				17.f == player.fPositionX && -9.f == player.fPositionZ &&
				12u == player.iPlayerId && 112u == player.iNetEntityId &&
				7u == player.iLastMoveSequence &&
				8u == player.iLastSkillSequence &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				INVALID_SKILL_ID == player.iCurrentSkillId &&
				!player.hasMoveGoal && player.CooldownEndTickBySkillId.empty() &&
				player.iCurrentHp == player.iMaximumHp &&
				player.iCurrentResource == player.iMaximumResource,
				"Change class during action, preserve identity/position/sequences, and reset state");

			C2S_USE_SKILL oldClassSkill{};
			oldClassSkill.iClientSequence = 9u;
			oldClassSkill.iSkillId = 34120u;
			oldClassSkill.fAimX = 1.f;
			oldClassSkill.fAimZ = 0.f;
			C2S_USE_SKILL newClassSkill = oldClassSkill;
			newClassSkill.iSkillId = 31200u;
			tests.Require(
				!room.m_PlayerSkillSystem.Try_Start(
					player, oldClassSkill, room.m_GameplayCatalog, 10u) &&
				room.m_PlayerSkillSystem.Try_Start(
					player, newClassSkill, room.m_GameplayCatalog, 10u) &&
				31200u == player.iCurrentSkillId &&
				9u == player.iLastSkillSequence,
				"Reject old-class skill and approve new-class skill after class change");

			const SERVER_PLAYER accepted = player;
			tests.Require(
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_STALE_SEQUENCE ==
					room.Apply_CharacterClassChange(player, request) &&
				accepted.eCharacterClass == player.eCharacterClass &&
				accepted.iCurrentHp == player.iCurrentHp,
				"Reject stale class change without mutating player");

			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.fPositionX = 999.f;
			player.fPositionY = 999.f;
			player.fPositionZ = 999.f;
			request.iClientSequence = 2u;
			request.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			SERVER_NAV_POINT projected{};
			const bool projectedSpawn = room.m_ServerNavigation.Project_Point(
				spawn->fPositionX, spawn->fPositionZ, projected);
			tests.Require(projectedSpawn &&
				CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED ==
					room.Apply_CharacterClassChange(player, request) &&
				CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
				projected.x == player.fPositionX &&
				projected.y == player.fPositionY &&
				projected.z == player.fPositionZ &&
				PLAYER_ACTION_STATE::NONE == player.eAction &&
				0u != player.iCurrentHp,
				"Change dead player class and respawn at projected original spawn");

			CGameRoom bernRoom{ WORLD_ID::BERN };
			const SERVER_PLAYER beforeWrongWorld = player;
			request.iClientSequence = 3u;
			request.eCharacterClass = CHARACTER_CLASS_ID::SLAYER;
			tests.Require(bernRoom.Is_Ready() &&
				CHARACTER_CLASS_CHANGE_RESULT::REJECTED_WRONG_WORLD ==
					bernRoom.Apply_CharacterClassChange(player, request) &&
				beforeWrongWorld.eCharacterClass == player.eCharacterClass &&
				beforeWrongWorld.iCurrentHp == player.iCurrentHp,
				"Reject class change outside Character Select without mutation");
		}
	}
	for (const QUICK_SKILL_CONTRACT& contract : QUICK_SKILLS)
	{
		const PLAYER_SKILL_DEFINITION* skill =
			catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != skill &&
			skill->eCharacterClass == contract.characterClass &&
			skill->strInputSlot == contract.inputSlot,
			"Resolve playable skill binding");
		tests.Require(
			nullptr != skill &&
			(skill->strDamageProfileId.empty() ?
				0u == catalog.Find_DamageRatePercent(skill->strDamageProfileId) :
				0u != catalog.Find_DamageRatePercent(skill->strDamageProfileId)),
			"Resolve playable skill damage policy");

		SERVER_PLAYER quickPlayer{};
		quickPlayer.eCharacterClass = contract.characterClass;
		quickPlayer.eStance = nullptr != skill ?
			skill->eRequiredStance : PLAYER_STANCE_ID::NONE;
		quickPlayer.iCurrentHp = 1;
		quickPlayer.iMaximumHp = 1;
		/* Official CostMp runs 206..938 at the reference level, so the test pool
		matches the published class pool rather than the old 100. */
		quickPlayer.iCurrentResource = 1000;
		quickPlayer.iMaximumResource = 1000;
		/* Same idea for identityCost (Artist's moon/sun orbs): a class without
		a gauge finds nullptr and stays at 0, which is correct there too. */
		const PLAYER_RUNTIME_PROFILE* quickIdentityProfile =
			catalog.Find_Player(contract.characterClass);
		quickPlayer.iMaximumIdentity = nullptr != quickIdentityProfile ?
			quickIdentityProfile->iMaximumIdentity : 0u;
		quickPlayer.iCurrentIdentity = quickPlayer.iMaximumIdentity;
		C2S_USE_SKILL quickCommand{};
		quickCommand.iClientSequence = 1;
		quickCommand.iSkillId = contract.skillId;
		quickCommand.fAimX = 1.f;
		quickCommand.fAimZ = 0.f;
		CPlayerSkillSystem quickSkillSystem;
		tests.Require(
			quickSkillSystem.Try_Start(
				quickPlayer,
				quickCommand,
				catalog,
				1),
			"Approve playable skill command");
	}
	for (const BASIC_ATTACK_CONTRACT& contract : BASIC_ATTACKS)
	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(contract.skillId);
		tests.Require(
			nullptr != combo &&
			combo->eCharacterClass == contract.characterClass &&
			combo->strInputSlot == "LMB" &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			combo->ComboStages.size() == contract.stageCount &&
			0u == combo->ComboStages.back().iInputCloseMs,
			"Resolve playable basic attack combo");
		tests.Require(
			nullptr != combo &&
			0u != catalog.Find_DamageRatePercent(combo->strDamageProfileId),
			"Resolve playable basic attack damage rate");
		if (nullptr == combo || combo->ComboStages.size() < 2u)
			continue;

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = contract.characterClass;
		comboPlayer.eStance = combo->eRequiredStance;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = contract.skillId;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		CPlayerSkillSystem comboSystem;
		tests.Require(
			comboSystem.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve playable basic attack first stage");

		const PLAYER_COMBO_STAGE& firstStage = combo->ComboStages.front();
		comboPlayer.fActionElapsedSeconds =
			static_cast<float>(firstStage.iInputOpenMs +
				firstStage.iInputCloseMs) * 0.0005f;
		press.iClientSequence = 2;
		comboSystem.Try_Start(comboPlayer, press, catalog, 11);
		tests.Require(
			comboPlayer.hasBufferedComboInput,
			"Buffer playable basic attack inside its input window");

		std::vector<SERVER_WORLD_ENTITY> noTargets;
		std::vector<DAMAGE_EVENT> noDamageEvents;
		for (std::uint32_t tick = 12;
			tick < 132 && comboPlayer.iComboStage < 2u;
			++tick)
		{
			comboSystem.Update(
				comboPlayer,
				noTargets,
				catalog,
				nullptr,
				1.f / 30.f,
				tick,
				noDamageEvents);
		}
		tests.Require(
			2u == comboPlayer.iComboStage,
			"Advance playable basic attack from Server-owned combo window");

		SERVER_PLAYER wrongClassPlayer{};
		wrongClassPlayer.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER == contract.characterClass ?
			CHARACTER_CLASS_ID::GUNSLINGER :
			CHARACTER_CLASS_ID::LANCE_MASTER;
		wrongClassPlayer.iCurrentHp = 1000;
		wrongClassPlayer.iMaximumHp = 1000;
		wrongClassPlayer.iCurrentResource = 1000;
		wrongClassPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem wrongClassSystem;
		tests.Require(
			!wrongClassSystem.Try_Start(
				wrongClassPlayer, press, catalog, 10),
			"Reject another class's basic attack");
	}
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER),
		"Resolve LanceMaster player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::GUNSLINGER),
		"Resolve Gunslinger player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::SLAYER),
		"Resolve Slayer player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::ARTIST),
		"Resolve Artist player profile");
	tests.Require(nullptr != catalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER),
		"Resolve DimensionMaster player profile");
	tests.Require(nullptr == catalog.Find_Player(CHARACTER_CLASS_ID::DESTROYER),
		"Reject unsupported Destroyer player profile");
	tests.Require(361u == catalog.Find_DamageRatePercent("damage.player.34120"),
		"Resolve player damage rate");
	tests.Require(361u == CGameplayCatalog::Resolve_Damage(100u, 361u),
		"Resolve damage as attack power times rate");
	tests.Require(100u == CGameplayCatalog::Resolve_Damage(100u, 100u),
		"Resolve basic attack rate as exactly one attack power");
	tests.Require(0u == CGameplayCatalog::Resolve_Damage(0u, 361u),
		"Resolve zero attack power as no damage");
	tests.Require(1u == CGameplayCatalog::Resolve_Damage(1u, 1u),
		"Clamp a connected hit to at least one damage");
	tests.Require(170u == CGameplayCatalog::Apply_Defense(350u, 105u),
		"Apply the documented project defense curve");
	tests.Require(1u == CGameplayCatalog::Apply_Defense(1u, 100000u),
		"Clamp a mitigated connected hit to at least one damage");

	CServerNavigation navigation;
	CWorldBootstrap world;
	tests.Require(world.Load(WORLD_ID::VALTAN_ARENA) &&
		world.Get_AreaId() == "LV_LUT_HEARTRB_ED",
		"Preserve world area ID across placement parsing");
	tests.Require(
		4u == static_cast<std::size_t>(std::count_if(
			world.Get_Placements().begin(),
			world.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})),
		"Load exactly four enabled Valtan player spawns");
	tests.Require(navigation.Load("LV_LUT_HEARTRB_ED"),
		"Load Valtan server navigation");
	std::vector<SERVER_NAV_POINT> path;
	tests.Require(navigation.Find_Path(152.f, -137.f, 151.f, -122.f, path) &&
		!path.empty(), "Find authoritative navigation path");
	SERVER_NAV_POINT rejected{};
	tests.Require(!navigation.Project_Point(10000.f, 10000.f, rejected),
		"Reject navigation point outside projection radius");

	CWorldBootstrap bernWorld;
	const bool bernLoaded = bernWorld.Load(WORLD_ID::BERN);
	const auto& bernPlacements = bernWorld.Get_Placements();
	const auto bernNpc = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == "npc.bern.beda.guide";
		});
	const auto bernCollision = std::find_if(
		bernPlacements.begin(), bernPlacements.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"collision.bern.editor-proof";
		});
	tests.Require(
		bernLoaded && bernPlacements.size() == 7u &&
		4u == static_cast<size_t>(std::count_if(
			bernPlacements.begin(), bernPlacements.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.isEnabled;
			})) &&
		bernNpc != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::NPC == bernNpc->eKind &&
		bernNpc->strArchetypeId == "NPC_BEDA" &&
		bernCollision != bernPlacements.end() &&
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == bernCollision->eKind,
		"Load Bern spawns, NPC_BEDA, trigger, and collision box");
	CServerCollisionSystem bernCollisionSystem;
	std::string bernCollisionStatus;
	tests.Require(
		bernCollisionSystem.Initialize(bernPlacements, bernCollisionStatus) &&
		1u == bernCollisionSystem.Get_CollisionBoxCount() &&
		std::all_of(
			bernPlacements.begin(), bernPlacements.end(),
			[&bernCollisionSystem](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != placement.eKind ||
					bernCollisionSystem.Is_PlayerSpawnClear(placement);
			}),
		"Stage Bern collision box without overlapping player spawns");
	SERVER_PLAYER collisionPlayer{};
	collisionPlayer.fPositionX = 138.f;
	collisionPlayer.fPositionY = 42.7f;
	collisionPlayer.fPositionZ = -65.3f;
	float resolvedX = 0.f;
	float resolvedY = 0.f;
	float resolvedZ = 0.f;
	bool wasBlocked = false;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-65.3f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		wasBlocked && resolvedX < 139.851f && resolvedX > 138.f,
		"Stop a fast player sweep before the Bern collision box");
	collisionPlayer.fPositionZ = -60.f;
	tests.Require(
		bernCollisionSystem.Resolve_PlayerMove(
			collisionPlayer,
			143.f,
			42.7f,
			-60.f,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked) &&
		!wasBlocked && std::abs(resolvedX - 143.f) < 0.001f,
		"Preserve movement that passes outside the collision box");

	CWorldBootstrap trainingWorld;
	CServerNavigation trainingNavigation;
	tests.Require(trainingWorld.Load(WORLD_ID::TRAINING_GROUND) &&
		trainingWorld.Get_AreaId() == "LV_DEV_TRAINING_GROUND" &&
		std::all_of(
			trainingWorld.Get_Placements().begin(),
			trainingWorld.Get_Placements().end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty();
			}),
		"Load class-neutral training player spawns");
	tests.Require(trainingNavigation.Load("LV_DEV_TRAINING_GROUND"),
		"Load training server navigation");
	SERVER_NAV_POINT trainingPoint{};
	tests.Require(trainingNavigation.Project_Point(0.f, -4.f, trainingPoint),
		"Project training spawn to walkable cell");
	tests.Require(!trainingNavigation.Project_Point(16.01f, 0.f, trainingPoint),
		"Reject training point beyond arena navigation bounds");

	CWorldBootstrap characterSelectWorld;
	CServerNavigation characterSelectNavigation;
	const bool characterSelectWorldLoaded =
		characterSelectWorld.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
	const auto& characterSelectSpawns =
		characterSelectWorld.Get_Placements();
	const auto lazyValtan = std::find_if(
		characterSelectSpawns.begin(),
		characterSelectSpawns.end(),
		[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId ==
				"boss.valtan.character-select.lazy";
		});
	tests.Require(
		characterSelectWorldLoaded &&
		characterSelectWorld.Get_AreaId() ==
			"LV_LOBBY_CLASSSELECT_SL00" &&
		characterSelectSpawns.size() == 5 &&
		4u == static_cast<size_t>(std::count_if(
			characterSelectSpawns.begin(),
			characterSelectSpawns.end(),
			[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
			{
				return WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
					placement.strArchetypeId.empty() &&
					placement.isEnabled;
			})),
		"Load class-neutral Character Select arena player spawns");
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		!lazyValtan->isEnabled &&
		lazyValtan->eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
		lazyValtan->strArchetypeId == "BOSS_VALTAN" &&
		lazyValtan->strEncounterId == "ENCOUNTER_VALTAN",
		"Load disabled Character Select Valtan lazy template");
	tests.Require(
		characterSelectNavigation.Load("LV_LOBBY_CLASSSELECT_SL00"),
		"Load Character Select arena server navigation");
	bool characterSelectSpawnsOnNavigation =
		characterSelectWorldLoaded && characterSelectSpawns.size() == 5;
	SERVER_NAV_POINT characterSelectPoint{};
	for (const WORLD_BOOTSTRAP_PLACEMENT& spawn : characterSelectSpawns)
	{
		if (WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn.eKind)
			continue;
		SERVER_NAV_POINT projected{};
		characterSelectSpawnsOnNavigation =
			characterSelectSpawnsOnNavigation &&
			characterSelectNavigation.Project_Point(
				spawn.fPositionX,
				spawn.fPositionZ,
				projected) &&
			std::abs(projected.y - spawn.fPositionY) <= 0.25f;
	}
	tests.Require(
		characterSelectSpawnsOnNavigation,
		"Project all Character Select spawns to baked navigation");
	SERVER_NAV_POINT lazyValtanPoint{};
	tests.Require(
		characterSelectSpawns.end() != lazyValtan &&
		characterSelectNavigation.Project_Point(
			lazyValtan->fPositionX,
			lazyValtan->fPositionZ,
			lazyValtanPoint) &&
		std::abs(lazyValtanPoint.y - lazyValtan->fPositionY) <= 0.25f,
		"Project disabled Character Select Valtan template to navigation");
	if (!characterSelectSpawns.empty())
	{
		characterSelectNavigation.Project_Point(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectPoint);
	}
	std::vector<SERVER_NAV_POINT> characterSelectPath;
	tests.Require(
		characterSelectSpawns.size() >= 2 &&
		characterSelectNavigation.Find_Path(
			characterSelectSpawns.front().fPositionX,
			characterSelectSpawns.front().fPositionZ,
			characterSelectSpawns[1].fPositionX,
			characterSelectSpawns[1].fPositionZ,
			characterSelectPath) &&
		characterSelectPath.size() >= 2 &&
		std::adjacent_find(
			characterSelectPath.begin(),
			characterSelectPath.end(),
			[](const SERVER_NAV_POINT& left, const SERVER_NAV_POINT& right)
			{
				return std::abs(left.y - right.y) > 0.6f;
			}) == characterSelectPath.end(),
		"Find Character Select arena navigation path");
	SERVER_NAV_POINT characterSelectOutside{};
	tests.Require(
		!characterSelectNavigation.Project_Point(
			-787.6f,
			197.5f,
			characterSelectOutside),
		"Reject point beyond Character Select arena navigation bounds");

	SERVER_PLAYER arenaSkillPlayer{};
	arenaSkillPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	arenaSkillPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	arenaSkillPlayer.iCurrentHp = 1000;
	arenaSkillPlayer.iMaximumHp = 1000;
	arenaSkillPlayer.iCurrentResource = 1000;
	arenaSkillPlayer.iMaximumResource = 1000;
	arenaSkillPlayer.fPositionX = characterSelectPoint.x;
	arenaSkillPlayer.fPositionY = characterSelectPoint.y;
	arenaSkillPlayer.fPositionZ = characterSelectPoint.z;
	C2S_USE_SKILL arenaSkillCommand{};
	arenaSkillCommand.iClientSequence = 1;
	arenaSkillCommand.iSkillId = 34120;
	arenaSkillCommand.fAimX = characterSelectPoint.x + 3.f;
	arenaSkillCommand.fAimZ = characterSelectPoint.z;
	CPlayerSkillSystem arenaSkillSystem;
	std::vector<SERVER_WORLD_ENTITY> arenaEntities;
	tests.Require(
		arenaSkillSystem.Try_Start(
			arenaSkillPlayer,
			arenaSkillCommand,
			catalog,
			10) &&
		PLAYER_ACTION_STATE::SKILL == arenaSkillPlayer.eAction &&
		34120u == arenaSkillPlayer.iCurrentSkillId &&
		10u == arenaSkillPlayer.iActionStartTick,
		"Start Character Select arena skill action");
	std::vector<DAMAGE_EVENT> arenaDamageEvents;
	arenaSkillSystem.Update(
		arenaSkillPlayer,
		arenaEntities,
		catalog,
		&characterSelectNavigation,
		1.f / 30.f,
		11,
		arenaDamageEvents);
	SERVER_NAV_POINT arenaSkillPoint{};
	tests.Require(
		characterSelectNavigation.Project_Point(
			arenaSkillPlayer.fPositionX,
			arenaSkillPlayer.fPositionZ,
			arenaSkillPoint),
		"Keep Character Select skill action position on baked navigation");

	SERVER_PLAYER player{};
	player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
	player.iCurrentResource = 1000;
	player.iMaximumResource = 1000;
	player.fPositionX = 151.f;
	player.fPositionY = 22.97f;
	player.fPositionZ = -129.f;
	SERVER_WORLD_ENTITY boss{};
	boss.iNetEntityId = 900u;
	boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	boss.eAction = SERVER_ENTITY_ACTION::IDLE;
	boss.strArchetypeId = "BOSS_VALTAN";
	boss.iCurrentHp = 10000;
	boss.iMaximumHp = 10000;
	boss.fPositionX = 151.f;
	boss.fPositionY = 22.97f;
	boss.fPositionZ = -122.f;
	boss.fCollisionRadius = 3.f;
	std::vector<SERVER_WORLD_ENTITY> entities{ boss };
	C2S_USE_SKILL useSkill{};
	useSkill.iClientSequence = 1;
	useSkill.iSkillId = 34120;
	useSkill.fAimX = boss.fPositionX;
	useSkill.fAimZ = boss.fPositionZ;
	CPlayerSkillSystem skills;
	tests.Require(skills.Try_Start(player, useSkill, catalog, 10),
		"Approve valid skill command");
	tests.Require(!skills.Try_Start(player, useSkill, catalog, 10),
		"Reject duplicate skill command while action is active");
	std::vector<DAMAGE_EVENT> damageEvents;
	for (std::uint32_t tick = 11; tick < 70; ++tick)
		skills.Update(player, entities, catalog, &navigation, 1.f / 30.f, tick,
			damageEvents);
	/* 34120 is official rate 361 at attack power 100. */
	tests.Require(9639u == entities[0].iCurrentHp,
		"Apply server-authoritative player damage once");
	tests.Require(
		1u == damageEvents.size() &&
		361u == damageEvents[0].iAmount &&
		damageEvents[0].isOutgoing &&
		entities[0].iNetEntityId == damageEvents[0].iTargetNetEntityId,
		"Emit one outgoing damage event for the resolved hit");
	C2S_USE_SKILL cooldownAttempt = useSkill;
	cooldownAttempt.iClientSequence = 2;
	tests.Require(!skills.Try_Start(player, cooldownAttempt, catalog, 70),
		"Reject skill during authoritative cooldown");

	{
		const PLAYER_SKILL_DEFINITION* combo = catalog.Find_Skill(34010);
		tests.Require(
			nullptr != combo &&
			PLAYER_SKILL_KIND::COMBO == combo->eSkillKind &&
			4u == combo->ComboStages.size() &&
			0u == combo->ComboStages[3].iInputCloseMs,
			"Resolve LanceMaster basic attack combo stages");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 100;
		comboPlayer.iMaximumResource = 100;
		std::vector<SERVER_WORLD_ENTITY> comboEntities;
		std::vector<DAMAGE_EVENT> comboDamageEvents;
		CPlayerSkillSystem comboSkills;

		C2S_USE_SKILL press{};
		press.iClientSequence = 1;
		press.iSkillId = 34010;
		press.fAimX = 1.f;
		press.fAimZ = 0.f;
		tests.Require(
			comboSkills.Try_Start(comboPlayer, press, catalog, 10) &&
			1u == comboPlayer.iComboStage,
			"Approve basic attack first stage");

		// 329ms is where stage one opens; 100ms is deliberately before it.
		comboPlayer.fActionElapsedSeconds = 0.1f;
		press.iClientSequence = 2;
		comboSkills.Try_Start(comboPlayer, press, catalog, 12);
		tests.Require(!comboPlayer.hasBufferedComboInput,
			"Reject combo input before the window opens");

		comboPlayer.fActionElapsedSeconds = 0.4f;
		press.iClientSequence = 3;
		comboSkills.Try_Start(comboPlayer, press, catalog, 14);
		tests.Require(comboPlayer.hasBufferedComboInput,
			"Buffer combo input inside the window");

		press.iClientSequence = 4;
		comboSkills.Try_Start(comboPlayer, press, catalog, 15);
		tests.Require(1u == comboPlayer.iComboStage,
			"Ignore a second press inside the same window");

		C2S_USE_SKILL other{};
		other.iClientSequence = 5;
		other.iSkillId = 34120;
		other.fAimX = 1.f;
		other.fAimZ = 0.f;
		tests.Require(
			!comboSkills.Try_Start(comboPlayer, other, catalog, 16) &&
			34010u == comboPlayer.iCurrentSkillId,
			"Reject a different skill during a combo");

		/* Stage one is 1633 ms long but its hit lands at 470 ms, so a buffered
		press has to cut in there rather than waiting out the clip. 20 ticks is
		about 667 ms: past the hit, nowhere near the full duration. */
		for (std::uint32_t tick = 17; tick < 37; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Cancel into the next combo stage once the hit has landed");

		/* Nothing is buffered now, so stage two has to run its whole 1367 ms
		instead of cutting at its hit. */
		for (std::uint32_t tick = 37; tick < 57; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			2u == comboPlayer.iComboStage &&
			PLAYER_ACTION_STATE::SKILL == comboPlayer.eAction,
			"Hold the stage past its hit when no press was buffered");

		for (std::uint32_t tick = 57; tick < 120; ++tick)
			comboSkills.Update(comboPlayer, comboEntities, catalog, nullptr,
				1.f / 30.f, tick, comboDamageEvents);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == comboPlayer.eAction &&
			0u == comboPlayer.iComboStage,
			"End the combo when no press was buffered");
	}

	std::map<PLAYER_ID, SERVER_PLAYER> players;
	SERVER_PLAYER target{};
	target.iPlayerId = 1;
	target.iNetEntityId = 100;
	target.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
	target.iCurrentHp = 1000;
	target.iMaximumHp = 1000;
	target.fPositionX = 151.f;
	target.fPositionY = 22.97f;
	target.fPositionZ = -128.f;
	target.isCombatReady = false;
	players.emplace(target.iPlayerId, target);
	SERVER_WORLD_ENTITY valtan{};
	valtan.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
	valtan.eAction = SERVER_ENTITY_ACTION::IDLE;
	/* The brain resolves damage through the boss's own catalog profile, so the
	test entity carries the archetype the room would have stamped on it. */
	valtan.strArchetypeId = "BOSS_VALTAN";
	valtan.strEncounterId = "ENCOUNTER_VALTAN";
	valtan.iCurrentHp = 48750;
	valtan.iMaximumHp = 60000;
	valtan.iMaximumHealthBars = 160;
	valtan.iLastEvaluatedHealthBar = 131;
	valtan.iPhaseTwoHpPercent = 50;
	valtan.iPhase = 1;
	valtan.fPositionX = 151.f;
	valtan.fPositionY = 22.97f;
	valtan.fPositionZ = -122.f;
	valtan.fEngageDistance = 35.f;
	valtan.fMoveSpeed = 3.f;
	CValtanBrain brain;
	std::vector<DAMAGE_EVENT> valtanDamageEvents;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 99,
		valtanDamageEvents);
	tests.Require(
		SERVER_ENTITY_ACTION::IDLE == valtan.eAction &&
		1000u == players.begin()->second.iCurrentHp &&
		valtanDamageEvents.empty(),
		"Protect Valtan entrant until first accepted gameplay intent");
	players.begin()->second.isCombatReady = true;
	for (std::uint32_t tick = 100; tick < 140 && valtanDamageEvents.empty(); ++tick)
		brain.Update(valtan, players, catalog, navigation, 0.1f, tick,
			valtanDamageEvents);
	tests.Require(781u == players.begin()->second.iCurrentHp,
		"Apply the queued 130-bar Valtan circle hit once");
	tests.Require(
		1u == valtanDamageEvents.size() &&
		219u == valtanDamageEvents[0].iAmount &&
		!valtanDamageEvents[0].isOutgoing &&
		players.begin()->second.iNetEntityId ==
			valtanDamageEvents[0].iTargetNetEntityId,
		"Emit one incoming damage event for the 130-bar boss hit");
	tests.Require(
		"VALTAN_FLOOR_WIPE_130" == valtan.strPatternId &&
		valtan.PendingPatternIds.empty() &&
		1u == valtan.TriggeredPatternIds.size() &&
		1u == valtan.iPatternSequence &&
		1u == valtan.iPatternStageIndex,
		"Queue and advance the staged 130-bar scripted mechanic");
	valtan.iCurrentHp = 30000;
	brain.Update(valtan, players, catalog, navigation, 0.1f, 141,
		valtanDamageEvents);
	tests.Require(2u == valtan.iPhase, "Advance Valtan phase from server HP");

	{
		SERVER_PLAYER meleePlayer{};
		meleePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		meleePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		meleePlayer.iCurrentHp = 1000;
		meleePlayer.iMaximumHp = 1000;
		meleePlayer.iCurrentResource = 1000;
		meleePlayer.iMaximumResource = 1000;
		meleePlayer.fPositionX = 0.f;
		meleePlayer.fPositionZ = 0.f;
		SERVER_WORLD_ENTITY meleeBoss{};
		meleeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		meleeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		meleeBoss.strArchetypeId = "BOSS_VALTAN";
		meleeBoss.iCurrentHp = 10000;
		meleeBoss.iMaximumHp = 10000;
		meleeBoss.fPositionX = 0.f;
		/* 34090 reaches 2.8 on its own; 3.5 is inside reach only because the
		boss's 3.0 collision radius extends the centre-to-centre test. */
		meleeBoss.fPositionZ = 3.5f;
		std::vector<SERVER_WORLD_ENTITY> meleeEntities{ meleeBoss };
		C2S_USE_SKILL melee{};
		melee.iClientSequence = 1;
		melee.iSkillId = 34090;
		melee.fAimX = 0.f;
		melee.fAimZ = 3.5f;
		CPlayerSkillSystem meleeSkills;
		std::vector<DAMAGE_EVENT> meleeDamageEvents;
		tests.Require(meleeSkills.Try_Start(meleePlayer, melee, catalog, 10),
			"Approve melee skill command");
		for (std::uint32_t tick = 11; tick < 60; ++tick)
		{
			meleeSkills.Update(
				meleePlayer, meleeEntities, catalog, nullptr, 1.f / 30.f, tick,
				meleeDamageEvents);
		}
		tests.Require(10000u - 1050u == meleeEntities[0].iCurrentHp,
			"Reach the boss through its collision radius");
	}

	{
		namespace fs = std::filesystem;
		const fs::path triggerRoot =
			fs::temp_directory_path() / L"LostArkWorldTriggerContractTest";
		std::error_code prepareError;
		fs::remove_all(triggerRoot, prepareError);
		fs::create_directories(triggerRoot / L"World");
		const fs::path bootstrapPath =
			triggerRoot / L"World" / L"VALTAN_ARENA.worldbootstrap";
		const auto writeTriggerBootstrap =
			[&bootstrapPath](const float durationSeconds)
			{
				std::ofstream bootstrap(bootstrapPath, std::ios::binary);
				bootstrap <<
					"LOSTARK_WORLD_BOOTSTRAP\t6\tVALTAN_ARENA"
					"\tLV_LUT_HEARTRB_ED\t3\t3\n"
					"player.spawn.contract\tplayerSpawn\t-\t-\t0\t0\t0\t0\t1\n"
					"trigger.contract.jump\ttriggerBox\t-\t-\t0\t0\t0\t0\t1"
					"\t2\t2\t2\t0\t1\tmovePlayer\t5\t10\t0\t0\t"
					<< durationSeconds << "\t4\n"
					"collision.contract.wall\tcollisionBox\t-\t-\t4\t1\t0\t0\t1"
					"\t0.5\t1\t2\n";
			};
		writeTriggerBootstrap(1.f);

		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", triggerRoot.c_str());
		CWorldBootstrap triggerBootstrap;
		const bool loadedTriggerBootstrap = triggerBootstrap.Load(
			WORLD_ID::VALTAN_ARENA);
		tests.Require(
			loadedTriggerBootstrap &&
			3u == triggerBootstrap.Get_Placements().size() &&
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ==
				triggerBootstrap.Get_Placements()[1].eKind &&
			WORLD_BOOTSTRAP_KIND::COLLISION_BOX ==
				triggerBootstrap.Get_Placements()[2].eKind &&
			1u == triggerBootstrap.Get_Placements()[1].TriggerActions.size(),
			"Parse trigger and collision box from world bootstrap v6");

		writeTriggerBootstrap(-1.f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			3u == triggerBootstrap.Get_Placements().size(),
			"Reject invalid trigger bootstrap without replacing committed world");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(triggerRoot, cleanupError);
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.jump";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = false;
		WORLD_TRIGGER_ACTION move{};
		move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		move.fTargetX = 10.f;
		move.fTargetY = 0.f;
		move.fTargetZ = 0.f;
		move.fDurationSeconds = 1.f;
		move.fArcHeight = 4.f;
		trigger.TriggerActions.push_back(move);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus) &&
			1u == triggerSystem.Get_TriggerCount(),
			"Initialize enabled movePlayer trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> triggerPlayers;
		SERVER_PLAYER triggerPlayer{};
		triggerPlayer.iPlayerId = 1;
		triggerPlayer.fPositionX = 2.4f;
		triggerPlayer.iCurrentHp = 100;
		triggerPlayer.iMaximumHp = 100;
		triggerPlayers.emplace(1, triggerPlayer);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		triggerSystem.Evaluate_Entries(triggerPlayers, 10, transfers, {});
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			triggerPlayers.begin()->second.TriggerMove.isActive &&
			10u == triggerPlayers.begin()->second.iActionStartTick,
			"Fire trigger on OBB entry");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 6.2f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY - 4.f) < 0.001f,
			"Advance movePlayer with authored parabolic arc");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 10.f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY) < 0.001f &&
			PLAYER_ACTION_STATE::NONE == triggerPlayers.begin()->second.eAction &&
			!triggerPlayers.begin()->second.TriggerMove.isActive,
			"Complete movePlayer at exact authored destination");
		triggerSystem.Evaluate_Entries(triggerPlayers, 11, transfers, {});
		triggerPlayers.begin()->second.fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(triggerPlayers, 12, transfers, {});
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			12u == triggerPlayers.begin()->second.iActionStartTick,
			"Rearm non-once trigger after player exits");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.change-level";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION changeLevel{};
		changeLevel.eKind = WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL;
		changeLevel.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
		trigger.TriggerActions.push_back(changeLevel);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled changeLevel trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iSessionId = 7;
		player.iPlayerId = 3;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.strNickName = "TriggerTransfer";
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		triggerSystem.Evaluate_Entries(players, 20, transfers, {});
		tests.Require(
			1u == transfers.size() &&
			7u == transfers.front().iSessionId &&
			WORLD_ID::VALTAN_ARENA == transfers.front().eTargetWorldId &&
			CHARACTER_CLASS_ID::LANCE_MASTER ==
				transfers.front().eCharacterClass &&
			"TriggerTransfer" == transfers.front().strNickName,
			"Emit one typed Server world transfer request on OBB entry");
		triggerSystem.Evaluate_Entries(players, 21, transfers, {});
			tests.Require(
			transfers.empty(),
			"Do not repeat a triggerOnce world transfer while occupied");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-spawn-group";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateSpawnGroup trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 4;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			30,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind ||
					"spawn.valtan.stage01" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateSpawnGroup target on OBB entry");
		triggerSystem.Evaluate_Entries(
			players,
			31,
			transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount,
			"Do not repeat a triggerOnce spawn-group activation while occupied");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-encounter";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER;
		activate.strTargetId = "boss.valtan.center";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateEncounter trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 5;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			40,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind ||
					"boss.valtan.center" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			});
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateEncounter target on OBB entry");
	}

	{
		/* A bootstrap whose skill cost exceeds every class pool must fail load:
		the publisher enforces the same bound, so acceptance here would mean the
		two sides disagree about the same document. */
		namespace fs = std::filesystem;
		const fs::path overCostRoot =
			fs::temp_directory_path() / L"LostArkBalanceContractTest";
		std::error_code prepareError;
		fs::remove_all(overCostRoot, prepareError);
		fs::create_directories(overCostRoot / L"Gameplay");
		{
			std::ofstream bootstrap(
				overCostRoot / L"Gameplay" / L"Gameplay.bootstrap",
				std::ios::binary);
			bootstrap <<
				"LOSTARK_GAMEPLAY_BOOTSTRAP\t4\t6\n"
				"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
				"DAMAGE\tdamage.player.34120\t361\n"
				"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\n"
				"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
				"SKILL\t34120\tLANCE_MASTER\tQ\tlancemaster.skill.34120\t10000\t2266"
				"\t1510\t2000\t0\t0\t8\tdamage.player.34120\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
		}
		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		CGameplayCatalog rollbackCatalog;
		tests.Require(rollbackCatalog.Load(),
			"Stage a valid gameplay catalog before rollback test");
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", overCostRoot.c_str());
		CGameplayCatalog overCostCatalog;
		tests.Require(!overCostCatalog.Load(),
			"Reject bootstrap skill cost above every class pool");
		tests.Require(
			!rollbackCatalog.Load() &&
			nullptr != rollbackCatalog.Find_Skill(34010) &&
			nullptr != rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN") &&
			31u == rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN")->size(),
			"Preserve the committed catalog after a corrupt replacement fails");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(overCostRoot, cleanupError);
	}

	{
		/* A skill with no damage profile never resolves a hit, so the hit time and
		reach that only describe that hit must be zero. Accepting a reach here would
		let a movement skill silently keep a damage window. */
		namespace fs = std::filesystem;
		const fs::path noDamageRoot =
			fs::temp_directory_path() / L"LostArkNoDamageContractTest";
		const auto loadWithMovementSkill =
			[&noDamageRoot](const char* hitTimeMs, const char* maximumRange)
		{
			std::error_code prepareError;
			fs::remove_all(noDamageRoot, prepareError);
			fs::create_directories(noDamageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					noDamageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t4\t6\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34120\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020"
					"\t8000\t900\t" << hitTimeMs << "\t242\t0\t6\t" << maximumRange <<
					"\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n";
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", noDamageRoot.c_str());
			CGameplayCatalog catalog;
			const bool loaded = catalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithMovementSkill("0", "0"),
			"Accept a skill that carries no damage profile");
		tests.Require(!loadWithMovementSkill("0", "3"),
			"Reject a damageless skill that still claims reach");
		tests.Require(!loadWithMovementSkill("400", "0"),
			"Reject a damageless skill that still claims a hit time");
		std::error_code noDamageCleanupError;
		fs::remove_all(noDamageRoot, noDamageCleanupError);
	}

	{
		/* A staged skill carries movement per stage, because a stage advance
		resets the action clock the curve is sampled on. */
		const PLAYER_SKILL_DEFINITION* basicAttack = catalog.Find_Skill(34010);
		bool everyStageCurveFits = nullptr != basicAttack &&
			!basicAttack->ComboStages.empty() &&
			basicAttack->RootMotion.empty();
		bool anyStageMoves = false;
		if (nullptr != basicAttack)
		{
			for (const PLAYER_COMBO_STAGE& stage : basicAttack->ComboStages)
			{
				if (stage.RootMotion.empty())
					continue;
				anyStageMoves = true;
				everyStageCurveFits = everyStageCurveFits &&
					stage.RootMotion.size() >= 2u &&
					stage.RootMotion.back().iTimeMs <= stage.iActionDurationMs;
			}
		}
		tests.Require(everyStageCurveFits && anyStageMoves,
			"Resolve per-stage root motion inside each combo stage duration");

		namespace fs = std::filesystem;
		const fs::path stageRoot =
			fs::temp_directory_path() / L"LostArkStageRootMotionContractTest";
		std::error_code stagePrepareError;
		const auto loadWithStageRow = [&](const char* stageIndex)
		{
			fs::remove_all(stageRoot, stagePrepareError);
			fs::create_directories(stageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					stageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t4\t8\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\t50\n"
					"DAMAGE\tdamage.player.34010\t100\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t1\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\tdamage.player.34010\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34010\tLANCE_MASTER\tLMB\tlancemaster.skill.34010"
					"\t0\t1633\t470\t0\t0\t0\t3\tdamage.player.34010\tCOMBO"
					"\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLSTAGE\t34010\t0\t1633\t470\t329\t658\n"
					"SKILLSTAGEROOTMOTION\t34010\t" << stageIndex <<
					"\t2\t0:0:0,1600:1.5:0\n";
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", stageRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithStageRow("0"),
			"Accept a root motion row that names an existing combo stage");
		tests.Require(!loadWithStageRow("1"),
			"Reject a root motion row past the last combo stage");
		std::error_code stageCleanupError;
		fs::remove_all(stageRoot, stageCleanupError);
	}

	{
		/* 절룡세 guards, and a hit taken inside that window is what buys the
		counter: no press advances it and the guard itself lands nothing. */
		SERVER_PLAYER counterPlayer{};
		counterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		counterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		counterPlayer.iCurrentHp = 1000;
		counterPlayer.iMaximumHp = 1000;
		counterPlayer.iCurrentResource = 1000;
		counterPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem counterSkills;

		C2S_USE_SKILL counterCommand{};
		counterCommand.iClientSequence = 1;
		counterCommand.iSkillId = 34580;
		counterCommand.fAimX = 1.f;
		counterCommand.fAimZ = 0.f;
		tests.Require(
			counterSkills.Try_Start(counterPlayer, counterCommand, catalog, 10) &&
			1u == counterPlayer.iComboStage,
			"Approve the counter guard stage");

		std::vector<SERVER_WORLD_ENTITY> counterEntities;
		std::vector<DAMAGE_EVENT> counterDamageEvents;
		counterSkills.Update(counterPlayer, counterEntities, catalog, nullptr,
			1.f / 30.f, 11, counterDamageEvents);
		tests.Require(
			1u == counterPlayer.iComboStage && counterDamageEvents.empty(),
			"Hold the guard stage and land no damage while it runs");

		const std::uint32_t hpBeforeCounter = counterPlayer.iCurrentHp;
		tests.Require(
			CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 12) &&
			2u == counterPlayer.iComboStage &&
			hpBeforeCounter == counterPlayer.iCurrentHp &&
			0.f == counterPlayer.fActionElapsedSeconds,
			"Absorb the hit inside the guard window and promote to the counter");
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 13),
			"Do not counter twice from one guard");

		SERVER_PLAYER lateCounter{};
		lateCounter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		lateCounter.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		lateCounter.iCurrentHp = 1000;
		lateCounter.iMaximumHp = 1000;
		lateCounter.iCurrentResource = 1000;
		lateCounter.iMaximumResource = 1000;
		CPlayerSkillSystem lateSkills;
		C2S_USE_SKILL lateCommand = counterCommand;
		lateSkills.Try_Start(lateCounter, lateCommand, catalog, 10);
		lateCounter.fActionElapsedSeconds = 1.5f;
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(lateCounter, catalog, 20) &&
			1u == lateCounter.iComboStage,
			"Reject a hit that lands after the guard window closed");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem comboSkills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1;
		basicAttack.iSkillId = 34010;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		comboSkills.Try_Start(comboPlayer, basicAttack, catalog, 10);
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(comboPlayer, catalog, 11),
			"Never counter out of a skill that is not a COUNTER");
	}

	{
		SERVER_PLAYER stancePlayer{};
		stancePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		stancePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		stancePlayer.iCurrentHp = 1000;
		stancePlayer.iMaximumHp = 1000;
		stancePlayer.iCurrentResource = 1000;
		stancePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem stanceSkills;

		C2S_USE_SKILL shortOnlySkill{};
		shortOnlySkill.iClientSequence = 1;
		shortOnlySkill.iSkillId = 34540;
		shortOnlySkill.fAimX = 1.f;
		shortOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, shortOnlySkill, catalog, 10),
			"Reject a short spear skill while in the long spear stance");

		C2S_USE_SKILL switchToShort{};
		switchToShort.iClientSequence = 2;
		switchToShort.iSkillId = 34000;
		switchToShort.fAimX = 1.f;
		switchToShort.fAimZ = 0.f;
		tests.Require(stanceSkills.Try_Start(stancePlayer, switchToShort, catalog, 10),
			"Approve the long to short spear stance transition");
		std::vector<SERVER_WORLD_ENTITY> stanceEntities;
		std::vector<DAMAGE_EVENT> stanceDamageEvents;
		for (std::uint32_t tick = 11; tick < 40; ++tick)
		{
			stanceSkills.Update(stancePlayer, stanceEntities, catalog, nullptr,
				1.f / 30.f, tick, stanceDamageEvents);
		}
		tests.Require(
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == stancePlayer.eStance &&
			PLAYER_ACTION_STATE::NONE == stancePlayer.eAction,
			"Flip to the short spear stance once the transition action completes");

		C2S_USE_SKILL longOnlySkill{};
		longOnlySkill.iClientSequence = 3;
		longOnlySkill.iSkillId = 34120;
		longOnlySkill.fAimX = 1.f;
		longOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, longOnlySkill, catalog, 40),
			"Reject a long spear skill after switching to the short spear stance");

		C2S_USE_SKILL shortSkillNow = shortOnlySkill;
		shortSkillNow.iClientSequence = 4;
		tests.Require(stanceSkills.Try_Start(stancePlayer, shortSkillNow, catalog, 40),
			"Approve a short spear skill after switching to the short spear stance");
	}

	{
		SERVER_PLAYER holdPlayer{};
		holdPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		holdPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		holdPlayer.iCurrentHp = 1000;
		holdPlayer.iMaximumHp = 1000;
		holdPlayer.iCurrentResource = 1000;
		holdPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem holdSkills;

		C2S_USE_SKILL chargeStart{};
		chargeStart.iClientSequence = 1;
		chargeStart.iSkillId = 34590;
		chargeStart.fAimX = 1.f;
		chargeStart.fAimZ = 0.f;
		tests.Require(holdSkills.Try_Start(holdPlayer, chargeStart, catalog, 10),
			"Approve the short spear hold skill");

		C2S_UPDATE_SKILL_AIM turnedAim{};
		turnedAim.iClientSequence = 2;
		turnedAim.iSkillId = 34590;
		turnedAim.fAimX = 0.f;
		turnedAim.fAimZ = -1.f;
		const float chargeYaw = holdPlayer.fYawDegrees;
		holdSkills.Update_Aim(holdPlayer, turnedAim, catalog);
		tests.Require(
			holdPlayer.fYawDegrees != chargeYaw &&
			holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Turn a charging hold skill toward a new aim");

		C2S_UPDATE_SKILL_AIM wrongSkillAim = turnedAim;
		wrongSkillAim.iClientSequence = 3;
		wrongSkillAim.iSkillId = 34540;
		wrongSkillAim.fAimX = 1.f;
		wrongSkillAim.fAimZ = 1.f;
		holdSkills.Update_Aim(holdPlayer, wrongSkillAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Ignore an aim update naming a skill that is not running");

		holdPlayer.hasReleasedHold = true;
		C2S_UPDATE_SKILL_AIM releasedAim = turnedAim;
		releasedAim.iClientSequence = 4;
		releasedAim.fAimX = 1.f;
		releasedAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, releasedAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim once the hold key is released");

		holdPlayer.hasReleasedHold = false;
		holdPlayer.iComboStage = 3u;
		C2S_UPDATE_SKILL_AIM firingAim = turnedAim;
		firingAim.iClientSequence = 5;
		firingAim.fAimX = 1.f;
		firingAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, firingAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim through the firing stage");

		SERVER_PLAYER activePlayer{};
		activePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		activePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		activePlayer.iCurrentHp = 1000;
		activePlayer.iMaximumHp = 1000;
		activePlayer.iCurrentResource = 1000;
		activePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem activeSkills;
		C2S_USE_SKILL activeStart{};
		activeStart.iClientSequence = 1;
		activeStart.iSkillId = 34540;
		activeStart.fAimX = 1.f;
		activeStart.fAimZ = 0.f;
		tests.Require(activeSkills.Try_Start(activePlayer, activeStart, catalog, 10),
			"Approve a non-hold short spear skill for the aim guard");
		const float activeAimX = activePlayer.fSkillAimDirectionX;
		C2S_UPDATE_SKILL_AIM activeAim{};
		activeAim.iClientSequence = 2;
		activeAim.iSkillId = 34540;
		activeAim.fAimX = 0.f;
		activeAim.fAimZ = -1.f;
		activeSkills.Update_Aim(activePlayer, activeAim, catalog);
		tests.Require(activeAimX == activePlayer.fSkillAimDirectionX,
			"Ignore an aim update on a skill that is not a HOLD");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		const bool loaded =
			spawnBootstrap.Load(WORLD_ID::CHARACTER_SELECT_ARENA);
		const auto& groups = spawnBootstrap.Get_Groups();
		const auto monsterGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		const auto minibossGroup = std::find_if(
			groups.begin(), groups.end(),
			[](const SPAWN_GROUP_DEFINITION& group)
			{
				return group.strSpawnGroupId ==
					"spawn.character-select.miniboss";
			});
		const SPAWN_GROUP_ANCHOR* monsterAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.monster");
		const SPAWN_GROUP_ANCHOR* minibossAnchor = spawnBootstrap.Find_Anchor(
			"anchor.character-select.miniboss");
		const MONSTER_RUNTIME_PROFILE* monsterProfile =
			spawnBootstrap.Find_Profile("MONSTER_VALTAN_PADD_01");
		const MONSTER_RUNTIME_PROFILE* minibossProfile =
			spawnBootstrap.Find_Profile("MINIBOSS_LUGARU");
		tests.Require(
			loaded && 1u == spawnBootstrap.Get_Revision() && 2u == groups.size() &&
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			nullptr != monsterAnchor && nullptr != minibossAnchor &&
			nullptr != monsterProfile && nullptr != minibossProfile,
			"Load two Character Select spawn groups, anchors, and profiles");

		const auto hasImmediateEntry = [](
			const SPAWN_GROUP_DEFINITION& group,
			const char* archetypeId,
			const char* anchorId)
		{
			return group.strRequiredCompletedGroupId.empty() &&
				1u == group.iMaxAlive && 1u == group.Waves.size() &&
				0u == group.Waves[0].iStartDelayMs &&
				1u == group.Waves[0].Entries.size() &&
				group.Waves[0].Entries[0].strArchetypeId == archetypeId &&
				1u == group.Waves[0].Entries[0].iCount &&
				group.Waves[0].Entries[0].strAnchorId == anchorId &&
				0u == group.Waves[0].Entries[0].iInitialDelayMs &&
				0u == group.Waves[0].Entries[0].iSpawnIntervalMs;
		};
		tests.Require(
			groups.end() != monsterGroup && groups.end() != minibossGroup &&
			hasImmediateEntry(
				*monsterGroup,
				"MONSTER_VALTAN_PADD_01",
				"anchor.character-select.monster") &&
			hasImmediateEntry(
				*minibossGroup,
				"MINIBOSS_LUGARU",
				"anchor.character-select.miniboss"),
			"Keep Character Select audition groups single-wave and zero-delay");

		CSpawnGroupRuntime immediateRuntime;
		std::string immediateStatus;
		std::uint32_t immediateSpawnCount = 0u;
		const bool immediateInitialized =
			immediateRuntime.Initialize(spawnBootstrap, immediateStatus);
		const bool failedImmediatePreservedDormant = immediateInitialized &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				[](const std::string&, const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&, std::uint32_t)
				{
					return false;
				}) &&
			!immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster");
		const auto countImmediateSpawn = [&immediateSpawnCount](
			const std::string&, const SPAWN_GROUP_ENTRY&,
			const SPAWN_GROUP_ANCHOR&, const MONSTER_RUNTIME_PROFILE&,
			std::uint32_t)
			{
				++immediateSpawnCount;
				return true;
			};
		tests.Require(
			failedImmediatePreservedDormant &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn) &&
			immediateRuntime.Activate_Immediate(
				"spawn.character-select.miniboss",
				spawnBootstrap,
				countImmediateSpawn) &&
			2u == immediateSpawnCount &&
			immediateRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!immediateRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				spawnBootstrap,
				countImmediateSpawn),
			"Commit immediate audition activation only after its spawn callback succeeds");

		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		const bool initialized =
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus);
		tests.Require(
			initialized &&
			spawnRuntime.Activate("spawn.character-select.monster") &&
			spawnRuntime.Activate("spawn.character-select.miniboss") &&
			spawnRuntime.Is_ActiveOrCompleted(
				"spawn.character-select.monster") &&
			!spawnRuntime.Activate("spawn.character-select.monster"),
			"Activate both Character Select audition groups");
		std::array<std::uint32_t, 2> scheduledByGroup{};
		bool callbackContractValid = true;
		spawnRuntime.Update(
			1.f / 30.f,
			spawnBootstrap,
			[](const std::string&) { return 0u; },
			[&scheduledByGroup, &callbackContractValid](
				const std::string& spawnGroupId,
				const SPAWN_GROUP_ENTRY& entry,
				const SPAWN_GROUP_ANCHOR& anchor,
				const MONSTER_RUNTIME_PROFILE& profile,
				const std::uint32_t ordinal)
			{
				if (spawnGroupId == "spawn.character-select.monster")
				{
					++scheduledByGroup[0];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MONSTER_VALTAN_PADD_01" &&
						anchor.strAnchorId == "anchor.character-select.monster" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else if (spawnGroupId == "spawn.character-select.miniboss")
				{
					++scheduledByGroup[1];
					callbackContractValid = callbackContractValid &&
						entry.strArchetypeId == "MINIBOSS_LUGARU" &&
						anchor.strAnchorId == "anchor.character-select.miniboss" &&
						profile.strArchetypeId == entry.strArchetypeId &&
						0u == ordinal;
				}
				else
				{
					callbackContractValid = false;
				}
				return true;
			});
		tests.Require(
			callbackContractValid && 1u == scheduledByGroup[0] &&
			1u == scheduledByGroup[1] &&
			2u == scheduledByGroup[0] + scheduledByGroup[1],
			"Schedule exactly two Character Select callbacks in the first update");

		CGameRoom resetRoom{ WORLD_ID::CHARACTER_SELECT_ARENA };
		SERVER_PLAYER resetPlayer{};
		resetPlayer.iSessionId = 501u;
		resetPlayer.iPlayerId = 502u;
		resetPlayer.iNetEntityId = 503u;
		resetPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		resetPlayer.strNickName = "ResetFixture";
		resetPlayer.iCurrentHp = 100u;
		resetPlayer.iMaximumHp = 100u;
		resetPlayer.isCombatReady = true;
		resetRoom.m_Players.emplace(resetPlayer.iPlayerId, resetPlayer);
		resetRoom.m_PlayerIdBySessionId.emplace(
			resetPlayer.iSessionId, resetPlayer.iPlayerId);
		resetRoom.m_PlayerIdByEntityId.emplace(
			resetPlayer.iNetEntityId, resetPlayer.iPlayerId);
		const bool resetGroupActivated =
			resetRoom.m_SpawnGroupRuntime.Activate_Immediate(
				"spawn.character-select.monster",
				resetRoom.m_SpawnGroupBootstrap,
				[&resetRoom](const std::string& spawnGroupId,
					const SPAWN_GROUP_ENTRY& entry,
					const SPAWN_GROUP_ANCHOR& anchor,
					const MONSTER_RUNTIME_PROFILE& profile,
					const std::uint32_t ordinal)
				{
					return resetRoom.Spawn_Monster(
						spawnGroupId, entry, anchor, profile, ordinal);
				});
		const bool spawnedBeforeLeave = std::any_of(
			resetRoom.m_WorldEntities.begin(),
			resetRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return entity.strSpawnGroupId ==
					"spawn.character-select.monster";
			});
		resetRoom.Leave(
			resetPlayer.iSessionId,
			PLAYER_DESPAWN_REASON::DISCONNECTED);
		tests.Require(
			resetGroupActivated && spawnedBeforeLeave &&
			resetRoom.m_Players.empty() &&
			resetRoom.m_WorldEntities.empty() &&
			resetRoom.m_SpawnGroupRuntime.Activate(
				"spawn.character-select.monster"),
			"Reset Character Select dynamic entities and spawn groups after the room becomes empty");
	}

	{
		CGameRoom raidRoom{ WORLD_ID::VALTAN_ARENA };
		const auto& placements = raidRoom.m_WorldBootstrap.Get_Placements();
		std::vector<const WORLD_BOOTSTRAP_PLACEMENT*> playerSpawns;
		for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
		{
			if (placement.isEnabled &&
				WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind)
			{
				playerSpawns.push_back(&placement);
			}
		}

		auto addPlayer = [&raidRoom](
			const SESSION_ID sessionId,
			const PLAYER_ID playerId,
			const NET_ENTITY_ID entityId,
			const WORLD_BOOTSTRAP_PLACEMENT& spawn)
		{
			SERVER_PLAYER player{};
			player.iSessionId = sessionId;
			player.iPlayerId = playerId;
			player.iNetEntityId = entityId;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "RaidFixture" + std::to_string(playerId);
			player.strSpawnPlacementId = spawn.strPlacementId;
			player.fPositionX = spawn.fPositionX;
			player.fPositionY = spawn.fPositionY;
			player.fPositionZ = spawn.fPositionZ;
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.isCombatReady = true;
			raidRoom.m_Players.emplace(playerId, player);
			raidRoom.m_PlayerIdBySessionId.emplace(sessionId, playerId);
			raidRoom.m_PlayerIdByEntityId.emplace(entityId, playerId);
		};

		if (raidRoom.Is_Ready() && 4u == playerSpawns.size())
		{
			for (std::size_t index = 0; index < playerSpawns.size(); ++index)
			{
				addPlayer(
					1001u + index,
					static_cast<PLAYER_ID>(2001u + index),
					static_cast<NET_ENTITY_ID>(3001u + index),
					*playerSpawns[index]);
			}
		}
		const PLAYER_ID allocatorBeforeFull = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID entityAllocatorBeforeFull = raidRoom.m_iNextNetEntityId;
		const std::size_t playersBeforeFull = raidRoom.m_Players.size();
		const std::size_t sessionOwnersBeforeFull =
			raidRoom.m_PlayerIdBySessionId.size();
		const std::size_t entityOwnersBeforeFull =
			raidRoom.m_PlayerIdByEntityId.size();
		tests.Require(
			raidRoom.Is_Ready() && 4u == playerSpawns.size() &&
			raidRoom.Is_PlayerAdmissionFull() &&
			nullptr == raidRoom.Find_AvailablePlayerSpawn() &&
			allocatorBeforeFull == raidRoom.m_iNextPlayerId &&
			entityAllocatorBeforeFull == raidRoom.m_iNextNetEntityId &&
			playersBeforeFull == raidRoom.m_Players.size() &&
			sessionOwnersBeforeFull == raidRoom.m_PlayerIdBySessionId.size() &&
			entityOwnersBeforeFull == raidRoom.m_PlayerIdByEntityId.size(),
			"Reject a fifth Valtan admission as room full without mutating room ownership or allocators");

		const std::string releasedSpawnId =
			playerSpawns.size() < 2u ? std::string{} :
			playerSpawns[1]->strPlacementId;
		raidRoom.Leave(1002u, PLAYER_DESPAWN_REASON::DISCONNECTED);
		const WORLD_BOOTSTRAP_PLACEMENT* releasedSpawn =
			raidRoom.Find_AvailablePlayerSpawn();
		const bool releasedSlotAvailable = nullptr != releasedSpawn &&
			releasedSpawn->strPlacementId == releasedSpawnId;
		if (releasedSlotAvailable)
		{
			addPlayer(1010u, 2010u, 3010u, *releasedSpawn);
		}
		tests.Require(
			releasedSlotAvailable && raidRoom.Is_PlayerAdmissionFull() &&
			4u == raidRoom.m_Players.size(),
			"Release a disconnected Valtan slot and admit a replacement into the same stable spawn");

		const WORLD_BOOTSTRAP_PLACEMENT* bossTrigger =
			raidRoom.Find_Placement("Stage_Boss");
		if (nullptr != bossTrigger && !raidRoom.m_Players.empty())
		{
			SERVER_PLAYER& triggerPlayer = raidRoom.m_Players.begin()->second;
			triggerPlayer.fPositionX = bossTrigger->fPositionX;
			triggerPlayer.fPositionY = bossTrigger->fPositionY -
				LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
			triggerPlayer.fPositionZ = bossTrigger->fPositionZ;
		}
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::uint32_t encounterActivationCount = 0u;
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			700u,
			transfers,
			[&raidRoom, &encounterActivationCount](
				const WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
					return false;
				++encounterActivationCount;
				return raidRoom.Activate_Encounter(targetId);
			});
		auto bossBeforeReset = std::find_if(
			raidRoom.m_WorldEntities.begin(),
			raidRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return "boss.valtan.center" == entity.strPlacementId;
			});
		const bool bossActivatedBeforeReset =
			1u == encounterActivationCount &&
			raidRoom.m_WorldEntities.end() != bossBeforeReset;
		if (raidRoom.m_WorldEntities.end() != bossBeforeReset)
		{
			bossBeforeReset->iCurrentHp = 1u;
			bossBeforeReset->iPhase = 2u;
			bossBeforeReset->eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
			bossBeforeReset->strPatternId = "reset.fixture.pattern";
		}
		const bool spawnGroupActivatedBeforeReset =
			raidRoom.m_SpawnGroupRuntime.Activate("spawn.valtan.stage01");
		SERVER_WORLD_ENTITY dynamicMonster{};
		dynamicMonster.iNetEntityId = 9001u;
		dynamicMonster.eKind = WORLD_BOOTSTRAP_KIND::MONSTER;
		dynamicMonster.strPlacementId = "reset.fixture.monster";
		dynamicMonster.strSpawnGroupId = "spawn.valtan.stage01";
		raidRoom.m_WorldEntities.push_back(std::move(dynamicMonster));
		DAMAGE_EVENT damageEvent{};
		damageEvent.iTargetNetEntityId = 3001u;
		damageEvent.iAmount = 1u;
		raidRoom.m_TickDamageEvents.push_back(damageEvent);
		raidRoom.m_iServerTick = 777u;
		SERVER_WORLD_TRANSFER_REQUEST pendingTransfer{};
		pendingTransfer.iSessionId = 8080u;
		pendingTransfer.eTargetWorldId = WORLD_ID::BERN;
		pendingTransfer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		pendingTransfer.strNickName = "PendingTransfer";
		raidRoom.m_PendingWorldTransfers.push_back(pendingTransfer);
		const PLAYER_ID playerAllocatorBeforeReset = raidRoom.m_iNextPlayerId;
		const NET_ENTITY_ID netAllocatorBeforeReset = raidRoom.m_iNextNetEntityId;

		std::vector<SESSION_ID> activeSessions;
		for (const auto& [sessionId, playerId] : raidRoom.m_PlayerIdBySessionId)
		{
			(void)playerId;
			activeSessions.push_back(sessionId);
		}
		for (const SESSION_ID sessionId : activeSessions)
		{
			raidRoom.Leave(sessionId, PLAYER_DESPAWN_REASON::DISCONNECTED);
		}
		const bool emptyResetPreservedMonotonicState =
			raidRoom.Is_Ready() && raidRoom.m_Players.empty() &&
			raidRoom.m_PlayerIdBySessionId.empty() &&
			raidRoom.m_PlayerIdByEntityId.empty() &&
			raidRoom.m_WorldEntities.empty() &&
			raidRoom.m_TickDamageEvents.empty() &&
			777u == raidRoom.m_iServerTick &&
			playerAllocatorBeforeReset == raidRoom.m_iNextPlayerId &&
			netAllocatorBeforeReset == raidRoom.m_iNextNetEntityId &&
			1u == raidRoom.m_PendingWorldTransfers.size() &&
			8080u == raidRoom.m_PendingWorldTransfers.front().iSessionId;
		const bool spawnGroupReactivated =
			raidRoom.m_SpawnGroupRuntime.Activate("spawn.valtan.stage01");

		if (!playerSpawns.empty() && nullptr != bossTrigger)
		{
			addPlayer(1020u, 2020u, 3020u, *playerSpawns.front());
			SERVER_PLAYER& nextGenerationPlayer =
				raidRoom.m_Players.find(2020u)->second;
			nextGenerationPlayer.fPositionX = bossTrigger->fPositionX;
			nextGenerationPlayer.fPositionY = bossTrigger->fPositionY -
				LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
			nextGenerationPlayer.fPositionZ = bossTrigger->fPositionZ;
		}
		encounterActivationCount = 0u;
		raidRoom.m_ServerTriggerSystem.Evaluate_Entries(
			raidRoom.m_Players,
			778u,
			transfers,
			[&raidRoom, &encounterActivationCount](
				const WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
					return false;
				++encounterActivationCount;
				return raidRoom.Activate_Encounter(targetId);
			});
		const auto bossAfterReset = std::find_if(
			raidRoom.m_WorldEntities.begin(),
			raidRoom.m_WorldEntities.end(),
			[](const SERVER_WORLD_ENTITY& entity)
			{
				return "boss.valtan.center" == entity.strPlacementId;
			});
		const bool encounterRestarted =
			1u == encounterActivationCount &&
			raidRoom.m_WorldEntities.end() != bossAfterReset &&
			bossAfterReset->iCurrentHp == bossAfterReset->iMaximumHp &&
			1u == bossAfterReset->iPhase &&
			SERVER_ENTITY_ACTION::IDLE == bossAfterReset->eAction &&
			bossAfterReset->strPatternId.empty();
		tests.Require(
			bossActivatedBeforeReset && spawnGroupActivatedBeforeReset &&
			emptyResetPreservedMonotonicState && spawnGroupReactivated &&
			encounterRestarted,
			"Reset Valtan boss, triggers, spawn groups, dynamic entities, and damage after the last disconnect while preserving IDs, tick, and transfers");
	}

	{
		CSpawnGroupBootstrap spawnBootstrap;
		tests.Require(
			spawnBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			3u == spawnBootstrap.Get_Groups().size(),
			"Load three authored Valtan spawn groups");
		CSpawnGroupRuntime spawnRuntime;
		std::string spawnStatus;
		tests.Require(
			spawnRuntime.Initialize(spawnBootstrap, spawnStatus),
			"Initialize Valtan spawn group runtime");
		tests.Require(
			!spawnRuntime.Activate("spawn.valtan.stage02.miniboss"),
			"Reject miniboss group before Stage 1 completion");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage01"),
			"Activate Stage 1 spawn group exactly once");
		std::uint32_t scheduledMonsterCount = 0;
		for (std::uint32_t step = 0; step < 64u &&
			!spawnRuntime.Is_Completed("spawn.valtan.stage01"); ++step)
		{
			spawnRuntime.Update(
				1.f,
				spawnBootstrap,
				[](const std::string&) { return 0u; },
				[&scheduledMonsterCount](const std::string&,
					const SPAWN_GROUP_ENTRY&,
					const SPAWN_GROUP_ANCHOR&,
					const MONSTER_RUNTIME_PROFILE&,
					const std::uint32_t)
				{
					++scheduledMonsterCount;
					return true;
				});
		}
		tests.Require(
			15u == scheduledMonsterCount &&
			spawnRuntime.Is_Completed("spawn.valtan.stage01"),
			"Schedule all Stage 1 waves and complete after all entities clear");
		tests.Require(
			spawnRuntime.Activate("spawn.valtan.stage02.miniboss"),
			"Unlock miniboss group after Stage 1 completion");
	}

	std::cout << "failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
```

### G00-07-07. `Client/Public/NetworkManager.h` full code

```cpp
#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "ClientReplicationEvent.h"

#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketStreamParser.h"

//race�� �����ϱ� ���ؼ� atomic header�� �߰�
#include <atomic>
#include <deque>
//���� ���� race�� ���� ���ؼ� mutex ���� �� ���
#include <mutex>
#include <thread>
#include <cstdint>
#include <span>
#include <string_view>


class CNetworkManager final
{
public:
	CNetworkManager() = default;
	CNetworkManager(const CNetworkManager&) = delete;
	CNetworkManager& operator=(const CNetworkManager&) = delete;

private:
	~CNetworkManager() = default;

public:
	static CNetworkManager& Get();
	static constexpr std::uint16_t DEFAULT_SERVER_PORT = 7777;
	static std::string Resolve_ServerHost();
	static std::string Resolve_MapEditorServerHost();

	bool Initialize();
	void Shutdown();
	void Update();

	bool Connect_To_Server(
		std::string_view host,
		std::uint16_t port);
	bool Connect_To_Server(std::uint16_t port)
	{
		return Connect_To_Server("192.168.200.103", port);
	}

	bool Send_EnterWorld(
		LostArk::Shared::WORLD_ID worldId,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickName);
	//playercontroller�� ��ǥ XZ�� �����ϴ� public ���
	bool Send_MoveGoal(
		std::uint32_t clientSequence,
		float goalX,
		float goalZ);
	bool Send_UseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);
	bool Send_ReleaseSkill(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId);
	bool Send_SkillAim(
		std::uint32_t clientSequence,
		LostArk::Shared::SKILL_ID skillId,
		float aimX,
		float aimZ);
	bool Send_RevivePlayer(std::uint32_t clientSequence);
	bool Send_ChangeCharacterClass(
		std::uint32_t clientSequence,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool Send_SpawnWorldEntity(std::string_view placementId);

	bool Try_Consume_EnterAccepted(
		LostArk::Shared::S2C_ENTER_ACCEPTED& message);
	bool Try_Consume_EnterRejected(
		LostArk::Shared::S2C_ENTER_REJECTED& message);
	bool Try_Consume_WorldEntitySpawnResult(
		LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT& message);
	bool Try_Consume_CharacterClassChangeResult(
		LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT& message);

	bool Try_Consume_ReplicationEvent(
		Client::CLIENT_REPLICATION_EVENT& event);

	void Close_ServerConnection();

	[[nodiscard]] bool Is_Connected() const;
	[[nodiscard]] int Get_LastErrorCode() const;
	[[nodiscard]] LostArk::Shared::PLAYER_ID Get_LocalPlayerId() const;
	[[nodiscard]] LostArk::Shared::NET_ENTITY_ID Get_LocalEntityId() const;
	[[nodiscard]] LostArk::Shared::CHARACTER_CLASS_ID
		Get_LocalCharacterClass() const;
	[[nodiscard]] bool Try_Get_LocalSpawn(
		LostArk::Shared::S2C_PLAYER_SPAWNED& outSpawn) const;


private:
	bool Send_All(std::span<const std::uint8_t> bytes);
	//���� worker �ϳ��� 4096-byte ���� ���۷� Server�� TCP byte stream�� �д´�.
	void Receive_Loop(SOCKET serverSocket);
	void Handle_Frame(const LostArk::Shared::PACKET_FRAME& frame);

private:
	SOCKET m_hServerSocket = INVALID_SOCKET;
	//main thread�� Receive worker�� ���� �ڵ带 �Բ� �а� ���Ƿ� atomic���� ��ȣ�Ѵ�.
	std::atomic<int> m_iLastErrorCode{ 0 };
	bool m_isWinSocketInitialized = false;

	std::thread m_ReceiveThread;

	std::atomic_bool m_isReceiveRunning{ false };

	LostArk::Shared::CPacketStreamParser m_StreamParser;

	std::mutex m_InboundMutex;
	std::deque<LostArk::Shared::PACKET_FRAME> m_InboundFrames;

	//Handle Frame�� �Һ��� ��� main thread�̴�.
	std::deque<Client::CLIENT_REPLICATION_EVENT> m_ReplicationEvents;
	std::deque<LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT>
		m_WorldEntitySpawnResults;
	std::deque<LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT>
		m_CharacterClassChangeResults;

	bool m_hasPendingEnterAccepted = false;

	LostArk::Shared::S2C_ENTER_ACCEPTED m_PendingEnterAccepted{};
	bool m_hasPendingEnterRejected = false;
	LostArk::Shared::S2C_ENTER_REJECTED m_PendingEnterRejected{};

	LostArk::Shared::PLAYER_ID m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;

	LostArk::Shared::NET_ENTITY_ID m_iLocalNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	LostArk::Shared::WORLD_ID m_eWorldId =
		LostArk::Shared::WORLD_ID::END;
	LostArk::Shared::CHARACTER_CLASS_ID m_eLocalCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool m_hasLocalSpawn = false;
	LostArk::Shared::S2C_PLAYER_SPAWNED m_LocalSpawn = {};

};
```

### G00-07-08. `Client/Private/NetworkManager.cpp` full code

```cpp
#include "NetworkManager.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <string>
#include <vector>

#include <array>
#include <cmath>
#include <utility>

//Socket worker thread�� client main thread�� �и��ϱ� ���ؼ� �����Ѵ�.
//workter thread -> byte ���Ű� frame ������ ����
//main thread -> frame �ؼ��� replication event ����

CNetworkManager& CNetworkManager::Get()
{
	static CNetworkManager instance;
	return instance;
}

std::string CNetworkManager::Resolve_ServerHost()
{
	constexpr char DEFAULT_SERVER_HOST[] = "192.168.200.103";
	constexpr char SERVER_HOST_ENVIRONMENT[] = "LOSTARK_SERVER_HOST";
	char configuredHost[64]{};
	const DWORD configuredLength = ::GetEnvironmentVariableA(
		SERVER_HOST_ENVIRONMENT,
		configuredHost,
		static_cast<DWORD>(std::size(configuredHost)));
	if (0 == configuredLength ||
		configuredLength >= std::size(configuredHost) ||
		"0.0.0.0" == std::string_view{ configuredHost })
	{
		return DEFAULT_SERVER_HOST;
	}
	return configuredHost;
}

std::string CNetworkManager::Resolve_MapEditorServerHost()
{
	constexpr char MAP_EDITOR_HOST_ENVIRONMENT[] =
		"LOSTARK_MAPEDITOR_SERVER_HOST";
	char configuredHost[64]{};
	const DWORD configuredLength = ::GetEnvironmentVariableA(
		MAP_EDITOR_HOST_ENVIRONMENT,
		configuredHost,
		static_cast<DWORD>(std::size(configuredHost)));
	if (0 == configuredLength ||
		configuredLength >= std::size(configuredHost) ||
		"0.0.0.0" == std::string_view{ configuredHost })
	{
		return Resolve_ServerHost();
	}
	return configuredHost;
}

bool CNetworkManager::Initialize()
{
	if (m_isWinSocketInitialized)
		return true;

	WSADATA winSockData{};
	const int result = ::WSAStartup(MAKEWORD(2, 2), &winSockData);
	if (0 != result)
	{
		m_iLastErrorCode = result;
		return false;
	}

	const bool isVersionSupported =
		2 == LOBYTE(winSockData.wVersion) &&
		2 == HIBYTE(winSockData.wVersion);

	if (!isVersionSupported)
	{
		m_iLastErrorCode = WSAVERNOTSUPPORTED;
		::WSACleanup();
		return false;
	}

	m_isWinSocketInitialized = true;
	m_iLastErrorCode = 0;
	return true;
}

void CNetworkManager::Shutdown()
{
	Close_ServerConnection();

	if (!m_isWinSocketInitialized)
		return;

	::WSACleanup();
	m_isWinSocketInitialized = false;
}
//�� �����Ӹ��� main thread���� ȣ��
void CNetworkManager::Update()
{
	//Inbound mutex ��� -> Worker�� ���� raw frame queue�� ���� queue�� swap
	//mutex ���� -> frame�� ���� ������� handle_frame�� ����

	std::deque<LostArk::Shared::PACKET_FRAME> receivedFrames;
	// Worker�� �ϼ��� Frame�� Main Thread�� �Ű� Packet �޽�����
	// Replication Event�� �����Ѵ�. Engine ��ü�� ���⼭ ���� �������� �ʴ´�.
	{
		std::scoped_lock lock
		{
			m_InboundMutex
		};
		//swap�� ���ؼ� frame�� �ؼ��ϴ� ���� network workter�� ���
		//�� frame�� ���� �� �ִ�.
		receivedFrames.swap(m_InboundFrames);
	}

	for (const auto& frame : receivedFrames)
	{
		Handle_Frame(frame);
	}
}

bool CNetworkManager::Connect_To_Server(
	const std::string_view host,
	const std::uint16_t port)
{
	if (!m_isWinSocketInitialized)
	{
		m_iLastErrorCode = WSANOTINITIALISED;
		return false;
	}

	if (Is_Connected())
		return true;
	if (host.empty() || host.size() > 63u || 0u == port)
	{
		m_iLastErrorCode = WSAEINVAL;
		return false;
	}

	// ��밡 ���� ������ ������ Receive Thread�� ����� joinable ������ �� �ִ�.
	// �� Socket�� Thread�� ����� ���� ���� ���� �ڿ��� ������ ȸ���Ѵ�.
	if (INVALID_SOCKET != m_hServerSocket ||
		m_ReceiveThread.joinable())
	{
		Close_ServerConnection();
	}

	m_hServerSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_hServerSocket)
	{
		m_iLastErrorCode = ::WSAGetLastError();
		return false;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	const std::string hostText{ host };
	if ("localhost" == hostText)
	{
		serverAddress.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
	}
	else if (1 != ::InetPtonA(
		AF_INET,
		hostText.c_str(),
		&serverAddress.sin_addr))
	{
		m_iLastErrorCode = WSAEINVAL;
		Close_ServerConnection();
		return false;
	}
	serverAddress.sin_port = ::htons(port);

	u_long nonBlocking = 1;
	if (SOCKET_ERROR == ::ioctlsocket(
		m_hServerSocket,
		FIONBIO,
		&nonBlocking))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		Close_ServerConnection();
		return false;
	}

	const int connectResult = ::connect(
		m_hServerSocket,
		reinterpret_cast<const sockaddr*>(&serverAddress),
		sizeof(serverAddress));
	if (SOCKET_ERROR == connectResult)
	{
		const int connectError = ::WSAGetLastError();
		if (WSAEWOULDBLOCK != connectError)
		{
			m_iLastErrorCode = connectError;
			Close_ServerConnection();
			return false;
		}

		fd_set writableSockets;
		FD_ZERO(&writableSockets);
		FD_SET(m_hServerSocket, &writableSockets);
		fd_set errorSockets;
		FD_ZERO(&errorSockets);
		FD_SET(m_hServerSocket, &errorSockets);
		timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 500000;
		const int selectResult = ::select(
			0,
			nullptr,
			&writableSockets,
			&errorSockets,
			&timeout);
		if (selectResult <= 0)
		{
			m_iLastErrorCode = 0 == selectResult ?
				WSAETIMEDOUT : ::WSAGetLastError();
			Close_ServerConnection();
			return false;
		}

		int socketError = 0;
		int socketErrorSize = sizeof(socketError);
		if (SOCKET_ERROR == ::getsockopt(
			m_hServerSocket,
			SOL_SOCKET,
			SO_ERROR,
			reinterpret_cast<char*>(&socketError),
			&socketErrorSize) ||
			0 != socketError)
		{
			m_iLastErrorCode = 0 != socketError ?
				socketError : ::WSAGetLastError();
			Close_ServerConnection();
			return false;
		}
	}

	nonBlocking = 0;
	if (SOCKET_ERROR == ::ioctlsocket(
		m_hServerSocket,
		FIONBIO,
		&nonBlocking))
	{
		m_iLastErrorCode = ::WSAGetLastError();
		Close_ServerConnection();
		return false;
	}

	m_StreamParser.Reset();
	m_ReplicationEvents.clear();
	m_WorldEntitySpawnResults.clear();
	m_CharacterClassChangeResults.clear();
	m_hasPendingEnterAccepted = false;
	m_PendingEnterAccepted = {};
	m_hasPendingEnterRejected = false;
	m_PendingEnterRejected = {};
	m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eWorldId = LostArk::Shared::WORLD_ID::END;
	m_eLocalCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_hasLocalSpawn = false;
	m_LocalSpawn = {};
	m_iLastErrorCode.store(0);
	m_isReceiveRunning.store(true);
	m_ReceiveThread = std::thread(
		&CNetworkManager::Receive_Loop,
		this,
		m_hServerSocket);
	return true;
}
bool CNetworkManager::Send_EnterWorld(
	LostArk::Shared::WORLD_ID worldId,
	LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	std::string_view nickName)
{
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_ENTER_WORLD message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = worldId;
	message.eCharacterClass = characterClass;
	message.strNickName = std::string{ nickName };

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_ENTER_WORLD,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		return false;
	}

	if (!Send_All(frameBytes))
		return false;

	m_eLocalCharacterClass = characterClass;
	return true;
}

bool CNetworkManager::Send_MoveGoal(std::uint32_t clientSequence, float goalX, float goalZ)
{
	//���� ���� �˻� -> C2S_MOVE �� ����ü ���� -> sequence�� goal XZ ����
	//packetwriter�� payload ����ȭ -> C2S_MOVE frame ���� -> send_all
	//client sequence�� animation�� ������ ��� �ִ� �ǰ�? �ִϸ��̼� 1 2 3 4 ������ ������ ��� �ִ�?
	//�� �ִϸ��̼ǿ� ���� �κ��� ��� ó���ؾ� �ұ�?
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_MOVE message{};
	message.iClientSequence = clientSequence;
	message.fGoalX = goalX;
	message.fGoalZ = goalZ;

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_MOVE,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		return false;
	}

	return Send_All(frameBytes);
}

bool CNetworkManager::Send_UseSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_USE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ReleaseSkill(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_RELEASE_SKILL message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_RELEASE_SKILL,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_SkillAim(
	const std::uint32_t clientSequence,
	const LostArk::Shared::SKILL_ID skillId,
	const float aimX,
	const float aimZ)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_UPDATE_SKILL_AIM message{};
	message.iClientSequence = clientSequence;
	message.iSkillId = skillId;
	message.fAimX = aimX;
	message.fAimZ = aimZ;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_UPDATE_SKILL_AIM,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_RevivePlayer(
	const std::uint32_t clientSequence)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_REVIVE_PLAYER message{};
	message.iClientSequence = clientSequence;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_REVIVE_PLAYER,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_ChangeCharacterClass(
	const std::uint32_t clientSequence,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;
	C2S_CHANGE_CHARACTER_CLASS message{};
	message.iClientSequence = clientSequence;
	message.eCharacterClass = characterClass;
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;
	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Send_SpawnWorldEntity(
	const std::string_view placementId)
{
	using namespace LostArk::Shared;
	if (!Is_Connected())
		return false;

	C2S_SPAWN_WORLD_ENTITY message{};
	message.strPlacementId = std::string{ placementId };
	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	return Build_Packet_Frame(
		PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY,
		payloadWriter.Get_Buffer(),
		frameBytes) && Send_All(frameBytes);
}

bool CNetworkManager::Try_Consume_EnterAccepted(LostArk::Shared::S2C_ENTER_ACCEPTED& message)
{
	// ���� �ϳ��� �� ���� �Һ��Ͽ� Lobby�� ���� �������� Level�� �ߺ� ��ȯ���� �ʰ� �Ѵ�.
	if (!m_hasPendingEnterAccepted)
		return false;

	message = m_PendingEnterAccepted;

	m_hasPendingEnterAccepted = false;

	return true;
}

bool CNetworkManager::Try_Consume_EnterRejected(
	LostArk::Shared::S2C_ENTER_REJECTED& message)
{
	if (!m_hasPendingEnterRejected)
		return false;

	message = m_PendingEnterRejected;
	m_hasPendingEnterRejected = false;
	return true;
}

bool CNetworkManager::Try_Consume_WorldEntitySpawnResult(
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	if (m_WorldEntitySpawnResults.empty())
		return false;
	message = std::move(m_WorldEntitySpawnResults.front());
	m_WorldEntitySpawnResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_CharacterClassChangeResult(
	LostArk::Shared::S2C_CHARACTER_CLASS_CHANGE_RESULT& message)
{
	if (m_CharacterClassChangeResults.empty())
		return false;
	message = std::move(m_CharacterClassChangeResults.front());
	m_CharacterClassChangeResults.pop_front();
	return true;
}

bool CNetworkManager::Try_Consume_ReplicationEvent(Client::CLIENT_REPLICATION_EVENT& event)
{
	if (m_ReplicationEvents.empty())
	{
		return false;
	}

	event = std::move(m_ReplicationEvents.front());
	m_ReplicationEvents.pop_front();
	return true;
}

void CNetworkManager::Close_ServerConnection()
{
	m_isReceiveRunning.store(false);
	const SOCKET socketToClose = m_hServerSocket;
	m_hServerSocket = INVALID_SOCKET;

	if (INVALID_SOCKET != socketToClose)
	{
		::shutdown(socketToClose, SD_BOTH);
		::closesocket(socketToClose);
	}

	if (m_ReceiveThread.joinable())
		m_ReceiveThread.join();

	{
		std::scoped_lock lock{ m_InboundMutex };
		m_InboundFrames.clear();
	}

	m_StreamParser.Reset();
	m_ReplicationEvents.clear();
	m_WorldEntitySpawnResults.clear();
	m_CharacterClassChangeResults.clear();
	m_hasPendingEnterAccepted = false;
	m_PendingEnterAccepted = {};
	m_hasPendingEnterRejected = false;
	m_PendingEnterRejected = {};
	m_iLocalPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
	m_iLocalNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_eWorldId = LostArk::Shared::WORLD_ID::END;
	m_eLocalCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	m_hasLocalSpawn = false;
	m_LocalSpawn = {};
}

bool CNetworkManager::Is_Connected() const
{
	return
		INVALID_SOCKET != m_hServerSocket &&
		m_isReceiveRunning.load();
}

int CNetworkManager::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

LostArk::Shared::PLAYER_ID CNetworkManager::Get_LocalPlayerId() const
{
	return m_iLocalPlayerId;
}

LostArk::Shared::NET_ENTITY_ID CNetworkManager::Get_LocalEntityId() const
{
	return m_iLocalNetEntityId;
}

LostArk::Shared::CHARACTER_CLASS_ID
CNetworkManager::Get_LocalCharacterClass() const
{
	return m_eLocalCharacterClass;
}

bool CNetworkManager::Try_Get_LocalSpawn(
	LostArk::Shared::S2C_PLAYER_SPAWNED& outSpawn) const
{
	if (!m_hasLocalSpawn)
		return false;

	outSpawn = m_LocalSpawn;
	return true;
}

void CNetworkManager::Receive_Loop(const SOCKET serverSocket)
{
	//recv()�� server�� ���� ����Ʈ�� �޴´�
	//���� ����Ʈ�� PacketStreamParser�� �߰��Ѵ�.
	//parser���� �ϼ��� �������� �����Ѹ�ŭ ������.
	//�ϼ��� �������� inbound queue�� �ִ´�.
	using namespace LostArk::Shared;

	std::array<std::uint8_t, 4096> receiveBuffer{};

	// Main Thread�� Connect/Close�� ���� �ٲٰ� Receive Worker�� �ݺ� �������� �д´�.
	while (m_isReceiveRunning.load())
	{
		//serversocket�� �ִ� data recv�� �б�
		const int receiveByteCount = ::recv(
			serverSocket,
			reinterpret_cast<char*>(
				receiveBuffer.data()),
			static_cast<int>(
				receiveBuffer.size()),
			0);
		//ByteCount�� ���ؼ� ���� �� ���� ���� �Ǵ�

		//��밡 ���������� ������ �����޴�.
		if (0 == receiveByteCount)
			break;

		//socket I/O ���� �Ǵ� shutdown���� recv�� �����ƴ�.
		if (SOCKET_ERROR == receiveByteCount)
		{
			const int errorCode = ::WSAGetLastError();

			//����ڰ� ������ ����� ������ ���� ��� ������ ��� X
			if (m_isReceiveRunning.load())
				m_iLastErrorCode.store(errorCode);

			break;
		}

		// recv ����� Header�� Payload ��踦 �������� �ʴ� TCP ����Ʈ �����̴�.
		// Parser�� ���� recv ������ �����Ͽ� �ϼ��� Frame���� �����Ѵ�.
		const std::span<const std::uint8_t> receiveBytes
		{
			receiveBuffer.data(), static_cast<std::size_t>(receiveByteCount)
		};
		//TCP���� ���� ����Ʈ�� Parser�� ���� ���ۿ� ���δ�.
		if (!m_StreamParser.Append(receiveBytes))
		{
			m_iLastErrorCode.store(WSAEMSGSIZE);
			break;
		}
		//�̹� recv�� �ϼ��� �������� ���� �� ������ �� �ִ�. ;; ���� ���� ���鼭 �ľ�
		for (;;)
		{
			// PACKET_FRAME�� Header���� ������ PacketType�� Payload�� ���� �ǹ� ������.
			PACKET_FRAME frame{};

			const PACKET_PARSE_RESULT parseResult = m_StreamParser.Try_Pop(frame);

			if (PACKET_PARSE_RESULT::NEED_MORE_DATA == parseResult)
			{
				//���� ������ �ϳ��� �ϼ����� �ʾұ� ������, ���� ����� ��ٸ���.
				break;
			}
			if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
			{
				//�߸��� ũ�� �Ǵ� ��Ŷ Ÿ���� �߰߉Ѵ�.
				m_iLastErrorCode.store(WSAEPROTONOSUPPORT);

				m_isReceiveRunning.store(false);
				return;
			}
			//FRAME_READY�� ��쿡�� main thread ���� ť�� �ִ´�.
			{
				std::scoped_lock lock{
				   m_InboundMutex
				};

				m_InboundFrames.push_back(
					std::move(frame));
			}
		}
	}
	m_isReceiveRunning.store(false);
}

void CNetworkManager::Handle_Frame(const LostArk::Shared::PACKET_FRAME & frame)
{
	using namespace LostArk::Shared;

	//frame�� payload ������ �д´�. packet - ������ ��� header�� payload - class,strName �̷��� 2���� ������
	CPacketReader reader{ frame.Payload };

	switch (frame.ePacketType)
	{
	//Server Enter
	case PACKET_TYPE::S2C_ENTER_ACCEPTED:
	{
		S2C_ENTER_ACCEPTED accepted{};

		if (!Read_Message(reader, accepted) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}

		m_iLocalPlayerId = accepted.iPlayerId;
		m_iLocalNetEntityId = accepted.iNetEntityId;
		m_eWorldId = accepted.eWorldId;
		m_hasLocalSpawn = false;
		m_LocalSpawn = {};
		m_hasPendingEnterAccepted = true;
		m_PendingEnterAccepted = accepted;
		break;
	}
	case PACKET_TYPE::S2C_ENTER_REJECTED:
	{
		S2C_ENTER_REJECTED rejected{};
		if (!Read_Message(reader, rejected) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		m_hasPendingEnterRejected = true;
		m_PendingEnterRejected = rejected;
		break;
	}
	//Player Spawn
	case PACKET_TYPE::S2C_PLAYER_SPAWNED:
	{
		S2C_PLAYER_SPAWNED spawned{};

		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		//Client Replication Event ����
		if (spawned.iPlayerId == m_iLocalPlayerId &&
			spawned.iNetEntityId == m_iLocalNetEntityId &&
			std::isfinite(spawned.fPositionX) &&
			std::isfinite(spawned.fPositionY) &&
			std::isfinite(spawned.fPositionZ) &&
			std::isfinite(spawned.fYawDegrees))
		{
			m_LocalSpawn = spawned;
			m_hasLocalSpawn = true;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED;
		event.PlayerSpawned = std::move(spawned);
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED:
	{
		S2C_WORLD_ENTITY_SPAWNED spawned{};
		if (!Read_Message(reader, spawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_SPAWNED;
		event.WorldEntitySpawned = std::move(spawned);
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED:
	{
		S2C_WORLD_ENTITY_DESPAWNED despawned{};
		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_DESPAWNED;
		event.WorldEntityDespawned = despawned;
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	case PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT:
	{
		S2C_WORLD_ENTITY_SPAWN_RESULT result{};
		if (!Read_Message(reader, result) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		m_WorldEntitySpawnResults.push_back(std::move(result));
		break;
	}
	case PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT:
	{
		S2C_CHARACTER_CLASS_CHANGE_RESULT result{};
		if (!Read_Message(reader, result) || 0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED == result.eResult)
		{
			m_eLocalCharacterClass = result.eActiveClass;
			if (m_hasLocalSpawn)
				m_LocalSpawn.eCharacterClass = result.eActiveClass;
		}
		m_CharacterClassChangeResults.push_back(std::move(result));
		break;
	}
	//snapshot
	case PACKET_TYPE::S2C_WORLD_SNAPSHOT:
	{
		//world�� snapshot�� ���� ����ü ����
		S2C_WORLD_SNAPSHOT snapshot{};

		if (!Read_Message(reader, snapshot) ||
			0 != reader.Get_RemainingSize() ||
			snapshot.eWorldId != m_eWorldId)
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}

		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType =
			Client::CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT;
		event.WorldSnapshot = std::move(snapshot);
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	//player despawn
	case PACKET_TYPE::S2C_PLAYER_DESPAWNED:
	{
		S2C_PLAYER_DESPAWNED despawned{};

		if (!Read_Message(reader, despawned) ||
			0 != reader.Get_RemainingSize())
		{
			m_iLastErrorCode.store(WSAEINVAL);
			return;
		}
		if (despawned.iNetEntityId == m_iLocalNetEntityId)
		{
			m_hasLocalSpawn = false;
			m_LocalSpawn = {};
		}
		Client::CLIENT_REPLICATION_EVENT event{};
		event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED;
		event.PlayerDespawned = despawned;
		m_ReplicationEvents.push_back(std::move(event));
		break;
	}
	default:
		break;
	}
}

bool CNetworkManager::Send_All(
	std::span<const std::uint8_t> bytes)
{
	if (!Is_Connected())
		return false;

	std::size_t sentByteCount = 0;

	while (sentByteCount < bytes.size())
	{
		const int result = ::send(
			m_hServerSocket,
			reinterpret_cast<const char*>(
				bytes.data() + sentByteCount),
			static_cast<int>(bytes.size() - sentByteCount),
			0);

		if (SOCKET_ERROR == result)
		{
			m_iLastErrorCode.store(::WSAGetLastError());
			return false;
		}

		if (0 == result)
			return false;

		sentByteCount += static_cast<std::size_t>(result);
	}

	return true;
}
```

### G00-07-09. `Client/Public/Level_Lobby.h` full code

```cpp
#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "Network/PacketType.h"

#include <chrono>

NS_BEGIN(Client)

class CLevel_Lobby final : public CLevel
{
private:
	enum class ENTRY_STATE
	{
		IDLE,
		WAITING_FOR_APPROVAL
	};

private:
	CLevel_Lobby(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Lobby();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	bool_t Begin_StageRequest(const LOBBY_COMMAND& command);
	bool_t Begin_NetworkEntry(
		LostArk::Shared::WORLD_ID eWorldId,
		LEVEL eTargetLevel,
		LOBBY_COMMAND_PURPOSE purpose);
	bool_t Resolve_Stage(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_PURPOSE purpose,
		LostArk::Shared::WORLD_ID& outWorldId,
		LEVEL& outTargetLevel) const;
	void Consume_EnterRejected();
	void Consume_EnterAccepted();
	void Cancel_PendingEntry(const string& reason);
	void Render_StagePanel();

private:
	ENTRY_STATE m_eEntryState = ENTRY_STATE::IDLE;
	LostArk::Shared::WORLD_ID m_ePendingWorldId =
		LostArk::Shared::WORLD_ID::END;
	LEVEL m_ePendingLevel = LEVEL::END;
	LOBBY_COMMAND_PURPOSE m_ePendingPurpose =
		LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	std::chrono::steady_clock::time_point m_ApprovalDeadline{};
	string m_strStatus =
		"Choose a stage directly or open Character Select to change class.";

public:
	static unique_ptr<CLevel_Lobby> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### G00-07-10. `Client/Private/Level_Lobby.cpp` full code

```cpp
#include "imgui.h"

#include "Level_Lobby.h"

#include "CharacterSelectionState.h"
#include "LevelTransitionService.h"
#include "NetworkManager.h"

#ifdef _DEBUG
#include "MapEditorWorkspaceService.h"
#endif

namespace
{
	constexpr char_t PLAYER_NICKNAME[] = "Player";
	constexpr LostArk::Shared::CHARACTER_CLASS_ID DEFAULT_ENTRY_CLASS =
		LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER;

	string Describe_ServerEndpoint()
	{
		return CNetworkManager::Resolve_ServerHost() + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT);
	}

#ifdef _DEBUG
	string Describe_MapEditorServerEndpoint()
	{
		return CNetworkManager::Resolve_MapEditorServerHost() + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT);
	}
#endif

	const char_t* Get_CharacterClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "Dimension Master";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		default:
			return "Not selected";
		}
	}

	bool_t Resolve_EntryCharacterClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass,
		bool_t& outUsedDefault)
	{
		outUsedDefault = false;
		if (CCharacterSelectionState::Try_Get_SelectedClass(
			outCharacterClass))
		{
			return true;
		}

		if (!CCharacterSelectionState::Select(DEFAULT_ENTRY_CLASS))
		{
			outCharacterClass =
				LostArk::Shared::CHARACTER_CLASS_ID::END;
			return false;
		}

		outCharacterClass = DEFAULT_ENTRY_CLASS;
		outUsedDefault = true;
		return true;
	}
}

CLevel_Lobby::CLevel_Lobby(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Lobby::~CLevel_Lobby()
{
}

HRESULT CLevel_Lobby::Initialize()
{
	return __super::Initialize();
}

void CLevel_Lobby::Update(const f32_t fTimeDelta)
{
	LOBBY_COMMAND command{};
	if (CLobbyCommandService::Try_Consume(command))
		Begin_StageRequest(command);

	Consume_EnterRejected();
	Consume_EnterAccepted();

	if (ENTRY_STATE::WAITING_FOR_APPROVAL == m_eEntryState)
	{
		if (!CNetworkManager::Get().Is_Connected())
		{
			Cancel_PendingEntry(
				"Server disconnected before approving entry. Lobby remains active.");
		}
		else if (std::chrono::steady_clock::now() >= m_ApprovalDeadline)
		{
			Cancel_PendingEntry(
				"Server entry approval timed out after 5 seconds. Lobby remains active.");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Lobby::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	Render_StagePanel();
	return S_OK;
}

void CLevel_Lobby::Consume_EnterRejected()
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED rejected{};
	if (!CNetworkManager::Get().Try_Consume_EnterRejected(rejected))
		return;

	if (ENTRY_STATE::WAITING_FOR_APPROVAL != m_eEntryState ||
		NETWORK_PROTOCOL_VERSION != rejected.iProtocolVersion ||
		rejected.eWorldId != m_ePendingWorldId ||
		ENTER_WORLD_REJECTION_REASON::ROOM_FULL != rejected.eReason)
	{
		Cancel_PendingEntry("Server returned an invalid world-entry rejection.");
		return;
	}

	if (WORLD_ID::VALTAN_ARENA == rejected.eWorldId)
	{
		Cancel_PendingEntry(
			"Valtan raid is full (4/4). Lobby remains active.");
		return;
	}

	Cancel_PendingEntry("The selected world is full. Lobby remains active.");
}

bool_t CLevel_Lobby::Begin_StageRequest(const LOBBY_COMMAND& command)
{
	if (ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus = "Another entry or level transition is already pending.";
		return false;
	}

	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	LEVEL targetLevel = LEVEL::END;
	if (!Resolve_Stage(
		command.eStage,
		command.ePurpose,
		worldId,
		targetLevel))
	{
		m_strStatus = "The selected stage is not registered.";
		return false;
	}

	return Begin_NetworkEntry(worldId, targetLevel, command.ePurpose);
}

bool_t CLevel_Lobby::Begin_NetworkEntry(
	const LostArk::Shared::WORLD_ID eWorldId,
	const LEVEL eTargetLevel,
	const LOBBY_COMMAND_PURPOSE purpose)
{
	if (LOBBY_COMMAND_PURPOSE::END == purpose ||
		(LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose &&
			LEVEL::DEVELOPMENT != eTargetLevel))
	{
		m_strStatus = "The entry purpose is not valid for the selected stage.";
		return false;
	}
#ifndef _DEBUG
	if (LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose)
	{
		m_strStatus = "Map Editor workspace is available only in Debug.";
		return false;
	}
#endif

	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool_t usedDefaultClass = false;
	if (!Resolve_EntryCharacterClass(characterClass, usedDefaultClass))
	{
		m_strStatus = "The default entry class could not be committed.";
		return false;
	}

	CNetworkManager& networkManager = CNetworkManager::Get();
	networkManager.Close_ServerConnection();
	const string serverHost =
		LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose ?
		CNetworkManager::Resolve_MapEditorServerHost() :
		CNetworkManager::Resolve_ServerHost();
	if (!networkManager.Connect_To_Server(
		serverHost,
		CNetworkManager::DEFAULT_SERVER_PORT))
	{
		m_strStatus = "Server connection failed for " +
			serverHost + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT) + " (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		return false;
	}

	if (!networkManager.Send_EnterWorld(
		eWorldId,
		characterClass,
		PLAYER_NICKNAME))
	{
		m_strStatus = "C2S_ENTER_WORLD send failed (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		networkManager.Close_ServerConnection();
		return false;
	}

	m_eEntryState = ENTRY_STATE::WAITING_FOR_APPROVAL;
	m_ePendingWorldId = eWorldId;
	m_ePendingLevel = eTargetLevel;
	m_ePendingPurpose = purpose;
	m_ApprovalDeadline =
		std::chrono::steady_clock::now() + std::chrono::seconds(5);
	m_strStatus = usedDefaultClass ?
		"No class was selected. Lance Master was committed and entry approval is pending." :
		"C2S_ENTER_WORLD sent. Waiting for server approval.";
	return true;
}

bool_t CLevel_Lobby::Resolve_Stage(
	const LOBBY_STAGE eStage,
	const LOBBY_COMMAND_PURPOSE purpose,
	LostArk::Shared::WORLD_ID& outWorldId,
	LEVEL& outTargetLevel) const
{
	using LostArk::Shared::WORLD_ID;
	outWorldId = WORLD_ID::END;
	outTargetLevel = LEVEL::END;

	switch (eStage)
	{
	case LOBBY_STAGE::CHARACTER_SELECT:
		if (LOBBY_COMMAND_PURPOSE::GAMEPLAY != purpose)
			return false;
		outWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
		outTargetLevel = LEVEL::CHARACTER_SELECT;
		return true;
	case LOBBY_STAGE::TEST:
		if (LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose)
		{
			outWorldId = WORLD_ID::TRAINING_GROUND;
			outTargetLevel = LEVEL::DEVELOPMENT;
		}
		else
		{
			outWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
			outTargetLevel = LEVEL::CHARACTER_SELECT;
		}
		return true;
	case LOBBY_STAGE::VALTAN:
		outWorldId = WORLD_ID::VALTAN_ARENA;
		outTargetLevel = LEVEL::VALTAN_ARENA;
		return true;
	case LOBBY_STAGE::BERN:
		outWorldId = WORLD_ID::BERN;
		outTargetLevel = LEVEL::BERN;
		return true;
	default:
		return false;
	}
}

void CLevel_Lobby::Consume_EnterAccepted()
{
	LostArk::Shared::S2C_ENTER_ACCEPTED accepted{};
	CNetworkManager& networkManager = CNetworkManager::Get();
	if (!networkManager.Try_Consume_EnterAccepted(accepted))
		return;

	if (ENTRY_STATE::WAITING_FOR_APPROVAL != m_eEntryState)
	{
		Cancel_PendingEntry("Unexpected server approval was rejected.");
		return;
	}

	if (accepted.iProtocolVersion != LostArk::Shared::NETWORK_PROTOCOL_VERSION ||
		accepted.eWorldId != m_ePendingWorldId ||
		accepted.iPlayerId == LostArk::Shared::INVALID_PLAYER_ID ||
		accepted.iNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		Cancel_PendingEntry("Server returned an invalid world approval.");
		return;
	}

	const LEVEL approvedLevel = m_ePendingLevel;
#ifdef _DEBUG
	const bool_t opensMapEditorWorkspace =
		LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE ==
		m_ePendingPurpose;
	if (opensMapEditorWorkspace)
		CMapEditorWorkspaceService::Request();
#endif
	if (!CLevelTransitionService::Request_Load(
		approvedLevel,
		"lobby.enter-accepted"))
	{
#ifdef _DEBUG
		if (opensMapEditorWorkspace)
			CMapEditorWorkspaceService::Cancel();
#endif
		Cancel_PendingEntry(CLevelTransitionService::Get_Status());
		return;
	}

#ifdef _DEBUG
	if (opensMapEditorWorkspace)
		networkManager.Close_ServerConnection();
#endif

	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ePendingPurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	m_ApprovalDeadline = {};
	m_strStatus = "Server approved the world. Loading the stage.";
}

void CLevel_Lobby::Cancel_PendingEntry(const string& reason)
{
	CNetworkManager::Get().Close_ServerConnection();
	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ePendingPurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	m_ApprovalDeadline = {};
	m_strStatus = reason;
}

void CLevel_Lobby::Render_StagePanel()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport)
	{
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 24.f, viewport->WorkPos.y + 24.f),
			ImGuiCond_Always);
	}

	if (!ImGui::Begin(
		"LostArk Lobby",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	LostArk::Shared::CHARACTER_CLASS_ID selectedClass =
		DEFAULT_ENTRY_CLASS;
	const bool_t hasExplicitSelection =
		CCharacterSelectionState::Try_Get_SelectedClass(selectedClass);
	ImGui::Text(
		"Entry character: %s%s",
		Get_CharacterClassName(selectedClass),
		hasExplicitSelection ? "" : " (default)");
	const string serverEndpoint = Describe_ServerEndpoint();
	ImGui::TextDisabled("Server: %s", serverEndpoint.c_str());
#ifdef _DEBUG
	const string mapEditorEndpoint = Describe_MapEditorServerEndpoint();
	if (mapEditorEndpoint != serverEndpoint)
	{
		ImGui::TextDisabled(
			"Test (Map Editor): %s", mapEditorEndpoint.c_str());
	}
#endif
	ImGui::Separator();

	const bool_t isBusy = ENTRY_STATE::IDLE != m_eEntryState ||
		CLevelTransitionService::Is_Pending();
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Test"))
	{
#ifdef _DEBUG
		CLobbyCommandService::Request(
			LOBBY_STAGE::TEST,
			LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE);
#else
		CLobbyCommandService::Request(LOBBY_STAGE::TEST);
#endif
	}
	ImGui::SameLine();
	if (ImGui::Button("Character Select"))
		CLobbyCommandService::Request(LOBBY_STAGE::CHARACTER_SELECT);
	ImGui::SameLine();
	if (ImGui::Button("Valtan"))
		CLobbyCommandService::Request(LOBBY_STAGE::VALTAN);
	ImGui::SameLine();
	if (ImGui::Button("Bern"))
		CLobbyCommandService::Request(LOBBY_STAGE::BERN);
	ImGui::EndDisabled();

	if (!hasExplicitSelection)
	{
		ImGui::TextDisabled(
			"Direct entry commits Lance Master. Character Select changes it.");
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}

unique_ptr<CLevel_Lobby> CLevel_Lobby::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_Lobby>(
		new CLevel_Lobby(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
```

### G00-07-11. `Client/Public/MainApp.h` full code

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"
#include "Network/PacketMessages.h"
#include "RenderingProfileService.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;
class CAnimation_Tool;
class CHUDLayoutTool;
class CHUDRuntimeView;
class CBalanceTool;
class CCharacterPreviewPanel;
class CSkillWindowView;
class CChatWindowView;
class CPartyWindowView;


class CMainApp final
{
#ifdef _DEBUG
private:
	enum class DEBUG_TOOL
	{
		NONE,
		MAP,
		ANIMATION,
		EFFECT,
		RENDERING,
		UI,
		BALANCE
	};
#endif

private:
	CMainApp();

public:
	~CMainApp();

public:
	HRESULT Initialize();
	void Update(f32_t fTimeDelta);
	HRESULT Render();

private:
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Ready_Prototype_For_LoadingChrome();
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	void Apply_LevelRequest();
	HRESULT ReadyImGuiRuntime();
	void RenderCombatHUD();
	void RenderBossHealthBar();
	/* LanceMaster's 3-segment identity meter -- drawn procedurally (matching the real
	LanceMasterProgress.as formula: target.rotation = maxDegree * value/100, cascading through
	3 segments) rather than from extracted art, since the real asset's moving "target" piece is
	a vector shape this pipeline can't crop as an image (see .../HudGfx_Extracted notes). */
	void RenderLanceMasterIdentityGauge();
	void RenderSkillIcons();
	void RenderSkillCooldowns();
	/* Experimental replacement for RenderSkillIcons/RenderSkillCooldowns, built from the real
	extracted QuickSlot.gfx Scaleform asset (icon frame, cooldown sweep, on-use flash) instead
	of hand-placed layers -- see .../HudGfx_Extracted. */
	void RenderQuickSlot();
	void RenderCombatHUDText();

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool);
	void UpdateDebugToolShortcut();
	void RenderDeveloperTools();
	void RenderRenderingWorkbench();
	void RenderProfilerOverlay();
	void RenderProfilerSettings();
#endif

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	CRenderingProfileService m_RenderingProfiles;
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	/* Not _DEBUG-gated: the runtime HUD art must render in Release too. */
	unique_ptr<CHUDRuntimeView> m_pHUDRuntimeView = { nullptr };
	/* Edge-detects the local player's stance so RenderCombatHUD only calls
	CHUDRuntimeView::Play_KeyframeAnimation on an actual change (or the first frame a stance is
	known at all), instead of re-triggering the icon's animation every frame. NONE never matches a
	real stance, so the very first Render sees an edge and plays the arrival pose. */
	LostArk::Shared::PLAYER_STANCE_ID m_ePreviousHudStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	/* RenderQuickSlot edge-detects "skill just used" per Q..F slot as a ready-to-not-ready
	transition. iCooldownEndTick itself can't be compared directly across frames: for a ready
	skill CombatHUDViewModel defaults it to the current (ever-increasing) serverTick rather than
	a fixed sentinel, so a raw "did it grow" check fires every single frame. Index order matches
	RenderQuickSlot's own INPUT_SLOTS. */
	bool_t m_bPreviousQuickSlotReady[8] = { true, true, true, true, true, true, true, true };
	/* RenderLanceMasterIdentityGauge edge-detects each of the 3 identity segments reaching 100 to
	trigger the real extracted gauge0/1/2 highLightMc "burn" flourish (Lance_Id_GaugeBurn0/1/2)
	exactly once per fill, not every frame it stays full. */
	bool_t m_bLanceGaugeSegmentWasFull[3] = { false, false, false };
	/* The Lobby's animated title-screen backdrop (Data/UI/Lobby/Lobby_Layout.json), drawn
	behind everything else instead of a flat clear color. Release-safe, like the HUD view. */
	unique_ptr<CHUDRuntimeView> m_pLobbyBackgroundView = { nullptr };
	/* Not _DEBUG-gated: K opens the skill window during real gameplay, in Release too. */
	unique_ptr<CSkillWindowView> m_pSkillWindowView = { nullptr };
	bool_t m_bKDown = false;
	/* Not _DEBUG-gated: Enter opens the chat input during real gameplay, in Release too. */
	unique_ptr<CChatWindowView> m_pChatWindowView = { nullptr };
	bool_t m_bEnterDown = false;
	bool_t m_bEscapeDown = false;
	/* Not _DEBUG-gated: the party roster overlay draws in Release too, same as the rest of the
	combat HUD. UI-only placeholder roster until a party Shared protocol exists. */
	unique_ptr<CPartyWindowView> m_pPartyWindowView = { nullptr };

#ifdef _DEBUG
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	unique_ptr<CBalanceTool> m_pBalanceTool = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	bool_t m_bRenderQualityDraftInitialized = false;
	DEBUG_TOOL m_eActiveDebugTool = DEBUG_TOOL::NONE;
	RENDER_QUALITY_SETTINGS m_RenderQualityDraft = {};
	SCENE_RENDERING_PROFILE m_SceneRenderingDraft = {};
	string m_strRenderingDraftProfileId;
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strRenderingStatus =
		"Rendering profiles are loaded from the published runtime catalog.";
	string m_strProfilerCaptureStatus;
#endif

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END
```

### G00-07-12. `Client/Private/MainApp.cpp` full code

```cpp
#include "imgui.h"

#include "MainApp.h"

#include "ChatWindowView.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "ImGuiLayer.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Loading.h"
#include "LobbyCommandService.h"
#include "NetworkManager.h"
#include "PartyWindowView.h"
#include "PlayerSkillCatalog.h"
#include "Profiler.h"
#include "Presentation_Manager.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SkillWindowView.h"
#include "UI_Sprite.h"

#ifdef _DEBUG
#include "Animation_Tool.h"
#include "BalanceTool.h"
#include "CharacterPreviewPanel.h"
#include "Effect_Tool.h"
#include "HUDLayoutTool.h"
#include "MapEditorWorkspaceService.h"
#include "MapTool.h"
#include "NetworkPlayerCommandSink.h"
#include "ProfilerCaptureIO.h"
#endif

#include <algorithm>
#include <cmath>
#include <fstream>

namespace
{
	/* Not _DEBUG-gated: the K (skill window) toggle below needs this in Release too, not just
	the _DEBUG-only map tool focus check further down. */
	bool_t IsWindowOwnedByCurrentProcess(HWND hWnd)
	{
		if (nullptr == hWnd)
			return false;

		DWORD processId = {};
		return 0 != GetWindowThreadProcessId(hWnd, &processId) &&
			GetCurrentProcessId() == processId;
	}

#ifdef _DEBUG

	const char_t* GetHUDLayoutClassId(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Yinyangshi";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		default:
			return "Default";
		}
	}
#endif

	/* HUD_Layout.json's "ownerClass" strings (no spaces) must match the schema/tool names. */
	const string GetHUDOwnerClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Yinyangshi";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		default:
			return "";
		}
	}
}

CMainApp::CMainApp()
{
}

CMainApp::~CMainApp()
{
	Free();
}

HRESULT CMainApp::Initialize()
{
	/* CreateWICTextureFromFile (used by the HUD runtime view for non-DDS art) needs COM on the
	calling thread. The main thread never initializes it otherwise. */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	ENGINE_DESC engineDesc{};
	engineDesc.hInstance = g_hInst;
	engineDesc.hWnd = g_hWnd;
	engineDesc.eWinMode = WINMODE::WIN;
	engineDesc.iNumLevels = ETOUI(LEVEL::END);
	engineDesc.iWinSizeX = g_iWinSizeX;
	engineDesc.iWinSizeY = g_iWinSizeY;

	if (FAILED(CGameInstance::Get().Initialize_Engine(
		engineDesc,
		m_pDevice,
		m_pContext)))
	{
		return E_FAIL;
	}
	string renderingProfileStatus;
	if (!m_RenderingProfiles.Load_Runtime(renderingProfileStatus))
	{
		OutputDebugStringA((
			"[MainApp] Rendering profile initialization failed: " +
			renderingProfileStatus + "\n").c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, renderingProfileStatus.c_str(),
			"Rendering Profile Load Failed", MB_OK | MB_ICONERROR);
#endif
		return E_FAIL;
	}

	if (!CNetworkManager::Get().Initialize())
		return E_FAIL;
	if (FAILED(ReadyImGuiRuntime()))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(ReadyDebugTools()))
		return E_FAIL;
#endif

	if (FAILED(Ready_Fonts()) ||
		FAILED(Ready_Prototype_For_Static()))
	{
		return E_FAIL;
	}

	std::string effectCatalogStatus;
	if (!CEffectCatalog::Load(effectCatalogStatus))
	{
		const std::string diagnostic =
			"[MainApp] Effect Catalog initialization failed: " +
			effectCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, effectCatalogStatus.c_str(),
			"Effect Catalog Load Failed", MB_OK | MB_ICONERROR);
#endif
		return E_FAIL;
	}

	m_pHUDRuntimeView = std::make_unique<CHUDRuntimeView>(m_pDevice, m_pContext);
	m_pLobbyBackgroundView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/Lobby/Lobby_Layout.json",
		CHUDRuntimeView::DRAW_TARGET::BACKGROUND);
	m_pSkillWindowView = std::make_unique<CSkillWindowView>(m_pDevice, m_pContext);
	m_pChatWindowView = std::make_unique<CChatWindowView>(m_pDevice);
	m_pPartyWindowView = std::make_unique<CPartyWindowView>(m_pDevice);

	if (FAILED(Start_Level(LEVEL::LOBBY)))
		return E_FAIL;

	return S_OK;
}

void CMainApp::Update(const f32_t fTimeDelta)
{
#ifdef _DEBUG
	UpdateDebugToolShortcut();
#endif

	/* Not _DEBUG-gated: K is a normal gameplay keybind (the skill window), not one of the
	F1/F6 tool-switch keys AGENTS.md reserves. Skip it while ImGui already owns text input,
	so typing in the rune search box (once that becomes real) cannot also toggle the window. */
	if (nullptr != m_pSkillWindowView && !ImGui::GetIO().WantTextInput)
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t kDown = windowFocused &&
			0 != (GetAsyncKeyState(0x4B /* VK_K */) & 0x8000);
		if (kDown && !m_bKDown)
			m_pSkillWindowView->Toggle();
		m_bKDown = kDown;
	}

	/* Enter opens the chat input the same way K toggles the skill window: only while nothing
	else already owns text input, so it cannot hijack an unrelated focused field. Once open,
	ImGui::GetIO().WantTextInput is true for as long as the InputText keeps focus, which both
	naturally blocks this same re-open check and (via the keyboardCaptured/SetInputBlocked
	logic below) blocks gameplay key polling while typing -- no separate plumbing needed for
	that part. Escape closes it and is checked outside the WantTextInput guard, since that is
	exactly the state Escape needs to fire in. */
	if (nullptr != m_pChatWindowView && !ImGui::GetIO().WantTextInput)
	{
		/* Same level restriction as the chat window's own Render() gate -- Enter should not open
		an input box that would render invisible outside Bern/Valtan. */
		const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
		const bool_t chatLevelAllowed =
			ETOUI(LEVEL::BERN) == chatLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatLevel;
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t enterDown = chatLevelAllowed && windowFocused &&
			0 != (GetAsyncKeyState(VK_RETURN) & 0x8000);
		if (enterDown && !m_bEnterDown && !m_pChatWindowView->Is_Open())
			m_pChatWindowView->Open_Input();
		m_bEnterDown = enterDown;
	}
	if (nullptr != m_pChatWindowView && m_pChatWindowView->Is_Open())
	{
		const bool_t escapeDown =
			0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		if (escapeDown && !m_bEscapeDown)
			m_pChatWindowView->Close_Input();
		m_bEscapeDown = escapeDown;
	}

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->BeginFrame();

#ifdef _DEBUG
	const bool_t mapToolOpen = m_bDeveloperToolsVisible &&
		nullptr != m_pMapTool && m_pMapTool->IsOpen();
	const HWND foregroundWindow = GetForegroundWindow();
	const bool_t externalToolFocused = mapToolOpen &&
		nullptr != foregroundWindow &&
		foregroundWindow != g_hWnd &&
		IsWindowOwnedByCurrentProcess(foregroundWindow);
	const bool_t worldLeftMouseConsumed =
		nullptr != m_pMapTool && m_pMapTool->ConsumesWorldLeftMouse();
#else
	constexpr bool_t externalToolFocused = false;
	constexpr bool_t worldLeftMouseConsumed = false;
#endif

	const bool_t keyboardCaptured = nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureKeyboard() || externalToolFocused);
	const bool_t mouseCaptured = nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureMouse() || externalToolFocused);
	CGameInstance::Get().SetInputBlocked(keyboardCaptured, mouseCaptured);
	CGameInstance::Get().SetMouseButtonBlocked(
		DIM::LB,
		worldLeftMouseConsumed);

	CNetworkManager::Get().Update();
	CGameInstance::Get().Update_Engine(fTimeDelta);
	CEffectPresentationService::Commit_PendingSpawns();
	CEffectPresentationService::Synchronize_FollowAnchors();
	CEffectPresentationService::Update(fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
	if (nullptr != m_pEffectTool)
		m_pEffectTool->Update(fTimeDelta);
#endif

	// 현재 Level의 Update가 끝난 뒤에만 기존 Level을 파괴한다.
	Apply_LevelRequest();
}

HRESULT CMainApp::Render()
{
	float4_t clearColor = { 0.008f, 0.012f, 0.025f, 1.f };
	if (FAILED(CGameInstance::Get().Render_Begin(&clearColor)))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return E_FAIL;
	}

	if (FAILED(CGameInstance::Get().Render()))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return E_FAIL;
	}

	if (nullptr != m_pImGuiLayer)
	{
		if (nullptr != m_pLobbyBackgroundView &&
			ETOUI(LEVEL::LOBBY) == CGameInstance::Get().Get_CurrentLevelID())
		{
			m_pLobbyBackgroundView->Render("", 0);
		}
	#ifdef _DEBUG
		const HUD_PLAYER_STATE& hudPlayer =
			CCombatHUDViewModel::Get().Get_Player();
		const uint32_t hudLevel =
			CGameInstance::Get().Get_CurrentLevelID();
		const bool_t supportsAuthoredHUD =
			ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel ||
			ETOUI(LEVEL::DEVELOPMENT) == hudLevel ||
			ETOUI(LEVEL::BERN) == hudLevel ||
			ETOUI(LEVEL::VALTAN_ARENA) == hudLevel;
		/* Same reason RenderCombatHUD skips m_pHUDRuntimeView while the Skill Window is open --
		this is a second, independent path that draws the same class emblem/bars and was not
		gated on that the first time, so it kept bleeding through underneath. */
		const bool_t skillWindowOpenForPreview =
			nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
		if (nullptr != m_pHUDLayoutTool && hudPlayer.isValid &&
			supportsAuthoredHUD && !skillWindowOpenForPreview)
		{
			m_pHUDLayoutTool->Render_RuntimePreview(
				GetHUDLayoutClassId(hudPlayer.eCharacterClass));
		}
	#endif
		RenderCombatHUD();
		RenderBossHealthBar();
		/* RenderQuickSlot only draws the extracted QuickSlot.gfx on-use flash overlay -- it does
		not draw icon art, cooldown sweep, or keybind text for any class, so it is additive on top
		of the existing icon/cooldown rendering below, not a replacement for it. Disabling these
		two calls previously took every class's skill icons off screen, not just LanceMaster's. */
		RenderSkillIcons();
		RenderSkillCooldowns();
		RenderQuickSlot();
		if (nullptr != m_pChatWindowView)
		{
			/* Only in actual in-game play (Bern/Valtan), not Character Select -- more levels join
			this list as real in-game stages are added. */
			const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
			if (ETOUI(LEVEL::BERN) == chatLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatLevel)
				m_pChatWindowView->Render();
		}
		if (nullptr != m_pPartyWindowView)
		{
			/* Same level set as the chat window. UI-only placeholder roster for now (no party
			Shared protocol to gate on actual invite-accepted state yet). */
			const uint32_t partyLevel = CGameInstance::Get().Get_CurrentLevelID();
			if (ETOUI(LEVEL::BERN) == partyLevel || ETOUI(LEVEL::VALTAN_ARENA) == partyLevel)
			{
				m_pPartyWindowView->Render();
			}
		}
#ifdef _DEBUG
		if (m_bDeveloperToolsVisible)
		{
			RenderDeveloperTools();
			switch (m_eActiveDebugTool)
			{
			case DEBUG_TOOL::MAP:
				if (nullptr != m_pMapTool)
					m_pMapTool->Render();
				break;
			case DEBUG_TOOL::ANIMATION:
				if (nullptr != m_pAnimationTool)
					m_pAnimationTool->Render();
				break;
			case DEBUG_TOOL::EFFECT:
				if (nullptr != m_pEffectTool)
					m_pEffectTool->Render();
				break;
			case DEBUG_TOOL::RENDERING:
				RenderRenderingWorkbench();
				break;
			case DEBUG_TOOL::UI:
				/* Skill Window's slots (tripod plate, node glows, ...) are placed and dragged
				right in this same canvas, exactly like Combat HUD/Screen UI/Loading Screen --
				CHUDLayoutTool::Render_Canvas already draws and hit-tests m_Slots generically
				regardless of which document tab is active, so no separate preview window is
				needed (an earlier version of this code opened one; it only ended up floating
				over this window and blocking it instead of helping). */
				if (nullptr != m_pHUDLayoutTool)
					m_pHUDLayoutTool->Render();
				break;
			case DEBUG_TOOL::BALANCE:
				if (nullptr != m_pBalanceTool)
					m_pBalanceTool->Render();
				break;
			default:
				break;
			}

			if (m_bProfilerVisible)
			{
				RenderProfilerOverlay();
				RenderProfilerSettings();
			}
		}
#endif
		m_pImGuiLayer->EndFrame();
	}
	RenderCombatHUDText();

	return CGameInstance::Get().Render_End();
}

void CMainApp::RenderCombatHUD()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player =
		CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp ||
		0u == player.iMaximumResource)
	{
		return;
	}

	/* The Combat HUD draws to the always-on-top foreground layer, so it would otherwise show
	through around/behind the Skill Window (which does not necessarily cover every pixel of the
	viewport) instead of being hidden by it like a real full-screen menu hides the HUD. */
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();

	if (!skillWindowOpen && nullptr != m_pHUDRuntimeView)
	{
		/* Base state only for now -- no gauge/resource-driven stage switching yet. */
		const string strOwnerClass = GetHUDOwnerClassName(player.eCharacterClass);

		/* LanceMaster's identity icon is a keyframe-animated Scaleform extraction, not a static
		layer stack -- it has to be told to play, and only on an actual stance edge (the source
		asset's own stanceMc.gotoAndPlay("focus"/"wild") trigger, see LanceMasterSkinFrame.as).
		Every other class's identity art still comes from Slot.Layers and needs nothing here. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass &&
			player.eStance != m_ePreviousHudStance)
		{
			/* spear01 (used by the "wild" label frame) is visually the straight short spear;
			spear02 (used by "focus") is the curved glaive blade -- opposite of what the asset's
			own "spear01/spear02" filenames suggest, confirmed by actually opening both crops. */
			const char_t* pLabel =
				LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == player.eStance ?
					"wild" : "focus";
			m_pHUDRuntimeView->Play_KeyframeAnimation("Lance_Id_Stance", pLabel);
		}
		m_ePreviousHudStance = player.eStance;

		m_pHUDRuntimeView->Render(strOwnerClass, 0);
	}

	/* Disabled for now -- every attempt at the 3-segment gauge visual (procedural arcs, then the
	real track art) has landed wrong (bad radius/position, missing tint) and needs a proper
	in-game reference to get right rather than another guess. RenderLanceMasterIdentityGauge stays
	defined, unchanged, for whenever that reference is available. */
	// if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass)
	//	RenderLanceMasterIdentityGauge();

	if (nullptr != m_pSkillWindowView)
		m_pSkillWindowView->Render(player.eCharacterClass);
}

void CMainApp::RenderLanceMasterIdentityGauge()
{
	/* Same real formula as ark.ui.identityLanceMaster.LanceMasterProgress::updateProgress():
	each of the 3 segments independently tracks 0..100, and only fills once every segment
	before it is already full (LanceMasterStance.as's bubbleEffect cascade). Degrees come from
	the real gauge0/1/2.maxDegree constants (-80/-82/-76); sign only decided sweep direction in
	Flash's own rotation convention, so the sweep magnitude is what is real about it. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumIdentity || nullptr == m_pHUDRuntimeView)
		return;

	constexpr f32_t SEGMENT_MAX = 100.f;
	const f32_t fSegmentScale = player.iMaximumIdentity / (SEGMENT_MAX * 3.f);
	f32_t fRemaining = static_cast<f32_t>(player.iCurrentIdentity) / fSegmentScale;
	f32_t fSegmentValue[3];
	for (int32_t i = 0; i < 3; ++i)
	{
		fSegmentValue[i] = (std::min)(SEGMENT_MAX, (std::max)(0.f, fRemaining));
		fRemaining -= SEGMENT_MAX;
	}

	/* No procedural track/fill arc here anymore -- a prior pass drew one using each track piece's
	own long-axis pixel length (44.25/48) as an ImGui PathArcTo *radius*, which is wrong: those
	pieces are thin curved strips (their real footprint is 12.75x44.25 / 48x12 reference px, drawn
	as-is by the real Lance_Id_GaugeTrack0/1/2 slots below), not a description of some much larger
	circle. Using that length as a radius instead drew a circle nearly as wide as the whole icon --
	the "two huge curves" this replaced. The AS3-driven fill ("target", the piece that actually
	moves as value climbs 0->100) is confirmed vector-only, no extractable bitmap and no baked
	track/fill *color* data either, so it is not redrawn here at all rather than guessed again;
	real reference footage is needed to fill this back in correctly. */
	for (int32_t i = 0; i < 3; ++i)
	{
		/* Real extracted gauge0/1/2 highLightMc flourish (Data/.../Gauge0/1/2Burn.json, baked from
		lancemaster_identity.xml sprite386/438/368) -- the orange/yellow "this segment just filled"
		burn, not an invented effect. Triggered once on the empty->full edge, same pattern as
		RenderQuickSlot's on-use flash. */
		const bool_t bIsFull = fSegmentValue[i] >= SEGMENT_MAX;
		if (bIsFull && !m_bLanceGaugeSegmentWasFull[i])
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Lance_Id_GaugeBurn") + std::to_string(i), "burn");
		}
		m_bLanceGaugeSegmentWasFull[i] = bIsFull;
	}
}

void CMainApp::RenderSkillCooldowns()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
		return;

	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	if (skillWindowOpen || nullptr == m_pHUDRuntimeView)
		return;

	/* Matches the fixed server tick rate other Client files already redeclare locally
	(CombatHUDViewModel.cpp, Character.cpp) rather than exposing a Shared constant for it. */
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	constexpr f32_t PI = 3.14159265f;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	for (const HUD_SKILL_STATE& Skill : player.Skills)
	{
		if (Skill.strInputSlot.empty() || Skill.Is_Ready(player.iServerTick))
			continue;

		const uint32_t remainingTicks = Skill.iCooldownEndTick > player.iServerTick ?
			Skill.iCooldownEndTick - player.iServerTick : 0u;
		if (0u == remainingTicks)
			continue;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect("Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
			continue;

		const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
		const f32_t fTotalSeconds = Skill.iCooldownDurationTicks > 0u ?
			static_cast<f32_t>(Skill.iCooldownDurationTicks) / SERVER_TICK_HZ : fRemainingSeconds;
		const f32_t fFraction = fTotalSeconds > 0.f ?
			(std::min)(1.f, (std::max)(0.f, fRemainingSeconds / fTotalSeconds)) : 0.f;

		const ImVec2 vTopLeft(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
		const ImVec2 vBotRight(
			vTopLeft.x + fWidth * fScaleX,
			vTopLeft.y + fHeight * fScaleY);
		const ImVec2 vCenter(
			(vTopLeft.x + vBotRight.x) * 0.5f,
			(vTopLeft.y + vBotRight.y) * 0.5f);
		const f32_t fHalfW = (vBotRight.x - vTopLeft.x) * 0.5f;
		const f32_t fHalfH = (vBotRight.y - vTopLeft.y) * 0.5f;
		/* Sized past the slot's corners and clipped to its rect below, so the visible edge of
		the pie traces the square's own border instead of an inscribed circle -- a plain
		circular-sector fill would leave the corners uncovered while mostly full. */
		const f32_t fRadius = sqrtf(fHalfW * fHalfW + fHalfH * fHalfH) + 2.f;

		/* Sweeps clockwise from 12 o'clock as the *remaining* cooldown, shrinking back to
		nothing as it expires -- the icon starts fully covered right after use and is revealed
		clockwise, matching the reference cooldown swipe. */
		const f32_t fStartAngle = -PI * 0.5f;
		const f32_t fEndAngle = fStartAngle + fFraction * 2.f * PI;

		pDrawList->PushClipRect(vTopLeft, vBotRight, true);
		pDrawList->PathClear();
		pDrawList->PathLineTo(vCenter);
		pDrawList->PathArcTo(vCenter, fRadius, fStartAngle, fEndAngle, 32);
		pDrawList->PathFillConvex(IM_COL32(0, 0, 0, 150));
		pDrawList->PopClipRect();

		const int32_t iDisplaySeconds = static_cast<int32_t>(ceilf(fRemainingSeconds));
		const string strCooldownLabel = std::to_string(iDisplaySeconds) + "s";

		ImFont* pFont = ImGui::GetFont();
		const f32_t fFontSize = fHeight * fScaleY * 0.34f;
		const ImVec2 vTextSize =
			pFont->CalcTextSizeA(fFontSize, FLT_MAX, 0.f, strCooldownLabel.c_str());
		const ImVec2 vTextPos(
			vCenter.x - vTextSize.x * 0.5f,
			vCenter.y - vTextSize.y * 0.5f);

		pDrawList->AddText(pFont, fFontSize, ImVec2(vTextPos.x + 1.f, vTextPos.y + 1.f),
			IM_COL32(0, 0, 0, 220), strCooldownLabel.c_str());
		pDrawList->AddText(pFont, fFontSize, vTextPos,
			IM_COL32(255, 255, 255, 255), strCooldownLabel.c_str());
	}
}

void CMainApp::RenderBossHealthBar()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;

	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	const float uiScale = (std::min)(scaleX, scaleY);
	const float healthRatio = (std::clamp)(
		static_cast<float>(boss.iCurrentHp) /
		static_cast<float>(boss.iMaximumHp), 0.f, 1.f);
	const ImVec2 barMin{
		pViewport->WorkPos.x + 360.f * scaleX,
		pViewport->WorkPos.y + 30.f * scaleY };
	const ImVec2 barMax{
		pViewport->WorkPos.x + 920.f * scaleX,
		pViewport->WorkPos.y + 50.f * scaleY };
	const float fillRight = barMin.x +
		(barMax.x - barMin.x) * healthRatio;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);
	pDrawList->AddRectFilled(
		barMin, barMax, IM_COL32(24, 24, 28, 230), 3.f * uiScale);
	if (fillRight > barMin.x)
	{
		pDrawList->AddRectFilled(
			barMin,
			ImVec2(fillRight, barMax.y),
			IM_COL32(176, 34, 40, 255),
			3.f * uiScale);
	}
	pDrawList->AddRect(
		barMin, barMax, IM_COL32(224, 208, 176, 255),
		3.f * uiScale, 0, (std::max)(1.f, uiScale));
}

void CMainApp::RenderSkillIcons()
{
	/* Which skill icon belongs in Skill_Q.."Skill_F" is content, not layout: it depends on the
	live (class, stance) pair via CPlayerSkillCatalog::Find_BySlot, the same source of truth the
	input controller already resolves quick slots from. HUD_Layout.json only owns the shared
	frame's position/size (ownerClass null "Skill_Q".."Skill_F"); it must not carry a second,
	class-hardcoded copy of "which icon" that can drift out of sync with PlayerSkills.json. This
	replaces the LanceMaster-only stance special-case with one path for every class -- Find_BySlot
	already resolves stance-gated skills correctly and ignores the stance argument for classes
	whose skills have no requiredStance. */
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid)
		return;

	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	if (skillWindowOpen || nullptr == m_pHUDRuntimeView)
		return;

	struct SKILL_ICON_ENTRY { LostArk::Shared::SKILL_ID iSkillId; const char* pIconPath; };
	constexpr SKILL_ICON_ENTRY SKILL_ICON_TABLE[] =
	{
		/* LanceMaster -- Long Spear */
		{ 34040, "UI/Skill/LanceMaster/34040_DoubleStrike.png" },
		{ 34090, "UI/Skill/LanceMaster/34090_ThornJab.png" },
		{ 34100, "UI/Skill/LanceMaster/34100_BlueDragonsClaw.png" },
		{ 34160, "UI/Skill/LanceMaster/34160_SpearDive.png" },
		{ 34140, "UI/Skill/LanceMaster/34140_SoulCutter.png" },
		{ 34120, "UI/Skill/LanceMaster/34120_ChainSlash.png" },
		{ 34110, "UI/Skill/LanceMaster/34110_HalfMoonSlash.png" },
		{ 34150, "UI/Skill/LanceMaster/34150_RagingDragonSlash.png" },
		/* LanceMaster -- Short Spear (no D/F skill in that stance) */
		{ 34540, "UI/Skill/LanceMaster/34540_SpiralingSpear.png" },
		{ 34550, "UI/Skill/LanceMaster/34550_4HeadedDragon.png" },
		{ 34560, "UI/Skill/LanceMaster/34560_ThrustOfDestruction.png" },
		{ 34570, "UI/Skill/LanceMaster/34570_StarfallPounce.png" },
		{ 34580, "UI/Skill/LanceMaster/34580_DragonscaleDefense.png" },
		{ 34590, "UI/Skill/LanceMaster/34590_RedDragonsHorn.png" },
		/* Warlord */
		{ 17030, "UI/Skill/Warlord/17030_SharpSpear.png" },
		{ 17060, "UI/Skill/Warlord/17060_FireBullet.png" },
		{ 17080, "UI/Skill/Warlord/17080_DashUpperFire.png" },
		{ 17110, "UI/Skill/Warlord/17110_LeapAttack.png" },
		{ 17090, "UI/Skill/Warlord/17090_HookChain.png" },
		{ 17040, "UI/Skill/Warlord/17040_Bash.png" },
		{ 17100, "UI/Skill/Warlord/17100_ShieldShock.png" },
		{ 17140, "UI/Skill/Warlord/17140_GuardiansLightning.png" },
		/* Artist */
		{ 31200, "UI/Skill/Artist/31200_InkShower.png" },
		{ 31430, "UI/Skill/Artist/31430_Scatter.png" },
		{ 31480, "UI/Skill/Artist/31480_CraneWings.png" },
		{ 31210, "UI/Skill/Artist/31210_Kongkongi.png" },
		{ 31460, "UI/Skill/Artist/31460_ButterflyDream.png" },
		{ 31420, "UI/Skill/Artist/31420_OrchidStrike.png" },
		{ 31490, "UI/Skill/Artist/31490_TigerSlash.png" },
		{ 31470, "UI/Skill/Artist/31470_OneStroke.png" },
		/* DimensionMaster */
		{ 2050100, "UI/Skill/DimensionMaster/2050100_OneNeedle.png" },
		{ 2050120, "UI/Skill/DimensionMaster/2050120_Fragment.png" },
		{ 2050160, "UI/Skill/DimensionMaster/2050160_CrossThrust.png" },
		{ 2050180, "UI/Skill/DimensionMaster/2050180_BeyondSlash.png" },
		{ 2050210, "UI/Skill/DimensionMaster/2050210_LightSplit.png" },
		{ 2050220, "UI/Skill/DimensionMaster/2050220_PointPierce.png" },
		{ 2050240, "UI/Skill/DimensionMaster/2050240_BoundaryBreak.png" },
		{ 2050230, "UI/Skill/DimensionMaster/2050230_TimeShatter.png" },
	};

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F" };

	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	ID3D11ShaderResourceView* pEmptySlotSRV =
		m_pHUDRuntimeView->Load_Texture("UI/HUD/Common/Empty Slot.png");

	for (const char* pInputSlot : INPUT_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(
			string("Skill_") + pInputSlot, fX, fY, fWidth, fHeight))
		{
			continue;
		}

		const ImVec2 vTopLeft(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
		const ImVec2 vBotRight(
			vTopLeft.x + fWidth * fScaleX,
			vTopLeft.y + fHeight * fScaleY);

		const char* pIconPath = nullptr;
		if (const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
			player.eCharacterClass, pInputSlot, player.eStance))
		{
			for (const SKILL_ICON_ENTRY& Entry : SKILL_ICON_TABLE)
			{
				if (Entry.iSkillId == pSkill->iSkillId)
				{
					pIconPath = Entry.pIconPath;
					break;
				}
			}
		}

		/* Icon first, then the shared frame's border/tab back on top -- the shared "Skill_Q"
		slot already drew that same border underneath before this runs, so redrawing it here
		is what keeps it above the icon instead of the icon covering it. */
		if (nullptr != pIconPath)
		{
			if (ID3D11ShaderResourceView* pIconSRV = m_pHUDRuntimeView->Load_Texture(pIconPath))
				pDrawList->AddImage(pIconSRV, vTopLeft, vBotRight);
		}
		if (nullptr != pEmptySlotSRV)
			pDrawList->AddImage(pEmptySlotSRV, vTopLeft, vBotRight);
	}
}

void CMainApp::RenderQuickSlot()
{
	/* Icon art, slot frame, keybind label, and cooldown sweep aren't extracted from QuickSlot.gfx
	yet, so this only draws the real on-use flash -- RenderSkillIcons/RenderSkillCooldowns (called
	alongside this, not instead of it) still own everything else. */

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || nullptr == m_pHUDRuntimeView)
		return;

	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	if (skillWindowOpen)
		return;

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F" };

	uint32_t iSlotIndex = 0;
	for (const char* pInputSlot : INPUT_SLOTS)
	{
		bool_t bIsReady = true;
		for (const HUD_SKILL_STATE& Skill : player.Skills)
		{
			if (Skill.strInputSlot == pInputSlot)
			{
				bIsReady = Skill.Is_Ready(player.iServerTick);
				break;
			}
		}

		/* Ready-to-not-ready is the only reliable "used just now" signal -- see the member
		comment on m_bPreviousQuickSlotReady for why comparing raw iCooldownEndTick doesn't work. */
		if (m_bPreviousQuickSlotReady[iSlotIndex] && !bIsReady)
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Skill_") + pInputSlot + "_Flash", "flash");
		}
		m_bPreviousQuickSlotReady[iSlotIndex] = bIsReady;
		++iSlotIndex;
	}
}

void CMainApp::RenderCombatHUDText()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;
	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	const float scaleX = viewportSize.x / 1280.f;
	const float scaleY = viewportSize.y / 720.f;
	const float textScale = (std::min)(scaleX, scaleY);
	const auto position = [scaleX, scaleY](const float x, const float y)
	{
		return float2_t(x * scaleX, y * scaleY);
	};
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (player.isValid && player.iMaximumHp > 0u && player.iMaximumResource > 0u)
	{
		const wstring hp = L"HP  " + std::to_wstring(player.iCurrentHp) +
			L" / " + std::to_wstring(player.iMaximumHp);
		const wstring mana = L"MANA  " + std::to_wstring(player.iCurrentResource) +
			L" / " + std::to_wstring(player.iMaximumResource);
		/* Positions/size follow the same 0.75 anchor-scale (around 673.675, 747.092) and -12
		vertical shift applied to the whole bottom HUD in HUD_Layout.json -- these two labels
		are drawn here in C++, not from that JSON, so they need the same transform by hand or
		they drift off the now-smaller HP/mana bars. */
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), hp.c_str(),
			position(504.419f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), mana.c_str(),
			position(835.169f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
	}
	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp)
	{
		return;
	}

	const wstring hp = std::to_wstring(boss.iCurrentHp) + L" / " +
		std::to_wstring(boss.iMaximumHp);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), hp.c_str(),
		position(640.f, 58.f), Colors::White, 0.f,
		float2_t(0.5f, 0.5f), 0.42f * textScale);
}

HRESULT CMainApp::Ready_Fonts()
{
	const filesystem::path fontPath =
		CRuntimeAssetRoot::Resolve_Font(L"161ex.spritefont");
	if (fontPath.empty() || FAILED(CGameInstance::Get().Add_Font(
		TEXT("Font_Default"),
		fontPath.c_str())))
	{
		return E_FAIL;
	}

	/* LostArk's own source fonts (see SourceData/LPK/font/Binaries/Fonts/FontMap.xml),
	converted to DirectXTK .spritefont via MakeSpriteFont. Tag names mirror the
	original $-prefixed FontMap keys. */
	struct SOURCE_FONT { const tchar_t* strTag; const wchar_t* strFile; };
	constexpr SOURCE_FONT sourceFonts[] =
	{
		{ TEXT("Font_YG760"), L"YG760.spritefont" },
		{ TEXT("Font_YG330"), L"YG330.spritefont" },
		{ TEXT("Font_YoonGasiIIM"), L"YoonGasiIIM.spritefont" },
		{ TEXT("Font_EventDamage"), L"BMKkubulim.spritefont" },
	};

	for (const SOURCE_FONT& sourceFont : sourceFonts)
	{
		const filesystem::path sourceFontPath =
			CRuntimeAssetRoot::Resolve_Font(sourceFont.strFile);
		if (sourceFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(
			sourceFont.strTag,
			sourceFontPath.c_str())))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMainApp::Ready_Prototype_For_Static()
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxTex"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl"),
			VTXTEX::Elements,
			VTXTEX::iNumElements))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::STATIC),
			TEXT("Prototype_Component_VIBuffer_Rect"),
			CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		TEXT("Prototype_GameObject_EffectObject"),
		CEffectObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	/* For.Prototype_GameObject_UI_Sprite */
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Sprite"),
		CUI_Sprite::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	/* Every texture the loading-screen JSON references gets its own Texture prototype up
	front, keyed by its Resources-relative path -- CLevel_Loading::Ready_Layer_Chrome() Clones
	Prototype_GameObject_UI_Sprite once per slot and looks the texture prototype up by that
	same path string. */
	return Ready_Prototype_For_LoadingChrome();
}

HRESULT CMainApp::Ready_Prototype_For_LoadingChrome()
{
	const filesystem::path layoutPath =
		CProjectDataRoot::Resolve(L"UI/Loading/LoadingLayout.json");

	ifstream stream(layoutPath);
	if (!stream.is_open())
		return S_OK;

	const string text(
		(istreambuf_iterator<char>(stream)),
		istreambuf_iterator<char>());

	DATA_JSON_VALUE root;
	string error;
	if (!CDataJson::Parse(text, root, error))
		return S_OK;

	const DATA_JSON_VALUE* pSlots = root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Array())
		return S_OK;

	vector<wstring_t> registeredPaths;
	for (const DATA_JSON_VALUE& slot : pSlots->Get_Array())
	{
		const DATA_JSON_VALUE* pLayers = slot.Find("layers");
		if (nullptr == pLayers || !pLayers->Is_Array())
			continue;

		for (const DATA_JSON_VALUE& layer : pLayers->Get_Array())
		{
			const DATA_JSON_VALUE* pPath = layer.Find("path");
			if (nullptr == pPath || !pPath->Is_String() || pPath->Get_String().empty())
				continue;

			/* Loading chrome paths are plain ASCII filenames, so a naive widen is safe here. */
			const string& narrowPath = pPath->Get_String();
			const wstring_t widePath(narrowPath.begin(), narrowPath.end());

			if (registeredPaths.end() != find(registeredPaths.begin(), registeredPaths.end(), widePath))
				continue;
			registeredPaths.push_back(widePath);

			const filesystem::path resolvedPath = CRuntimeAssetRoot::Resolve(widePath);
			if (resolvedPath.empty())
				continue;

			if (FAILED(CGameInstance::Get().Add_Prototype(
				ETOUI(LEVEL::STATIC), widePath,
				CTexture::Create(m_pDevice, m_pContext, resolvedPath.c_str(), 1))))
			{
				return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CMainApp::Start_Level(
	const LEVEL eTargetLevel,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	const CLIENT_LEVEL_DESCRIPTOR* pTarget =
		CLevelRegistry::Find(eTargetLevel);
	if (nullptr == pTarget || nullptr == pTarget->pRenderingProfileId ||
		!m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId) ||
		!m_RenderingProfiles.Has_Profile(
			CRenderingProfileService::LOADING_PROFILE_ID))
	{
		return E_INVALIDARG;
	}

	unique_ptr<CLevel_Loading> loading =
		CLevel_Loading::Create(
			m_pDevice,
			m_pContext,
			eTargetLevel,
			lobbyCommandToken);
	if (nullptr == loading)
		return E_FAIL;

	const string previousProfileId =
		m_RenderingProfiles.Get_ActiveProfileId();
	string status;
	if (!m_RenderingProfiles.Activate_Profile(
		CRenderingProfileService::LOADING_PROFILE_ID, status))
	{
		return E_FAIL;
	}
	const HRESULT hChange = CGameInstance::Get().Change_Level(
		ETOUI(LEVEL::LOADING),
		move(loading));
	if (FAILED(hChange))
	{
		if (!previousProfileId.empty())
		{
			string rollbackStatus;
			if (!m_RenderingProfiles.Activate_Profile(
				previousProfileId, rollbackStatus))
			{
				OutputDebugStringA((
					"[MainApp] Loading profile rollback failed: " +
					rollbackStatus + "\n").c_str());
			}
		}
		return hChange;
	}
	return S_OK;
}

void CMainApp::Apply_LevelRequest()
{
	LEVEL_TRANSITION_REQUEST request{};
	if (!CLevelTransitionService::Try_Consume(request))
		return;
	const uint32_t iPreviousLevel =
		CGameInstance::Get().Get_CurrentLevelID();

	if (LEVEL_TRANSITION_PHASE::LOAD == request.ePhase)
	{
		const HRESULT result = Start_Level(
			request.eTargetLevel,
			request.iLobbyCommandToken);
		if (FAILED(result))
		{
			if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
			{
				CLobbyCommandService::Cancel(
					request.iLobbyCommandToken,
					"target level loading could not start");
			}
			CLevelTransitionService::Report_LoadFailure(result);
		}
		else
		{
			CEffectPresentationService::Clear_Level(iPreviousLevel);
		}
		return;
	}

	const CLIENT_LEVEL_DESCRIPTOR* pTarget =
		CLevelRegistry::Find(request.eTargetLevel);
	const bool_t hasTargetProfile = nullptr != pTarget &&
		nullptr != pTarget->pRenderingProfileId &&
		m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId);
	unique_ptr<CLevel> nextLevel = hasTargetProfile ?
		CLevelRegistry::Create_Level(
			request.eTargetLevel,
			m_pDevice,
			m_pContext) : nullptr;
	const string previousProfileId =
		m_RenderingProfiles.Get_ActiveProfileId();
	string profileStatus;
	const bool_t profileActivated = nullptr != nextLevel &&
		m_RenderingProfiles.Activate_Profile(
			pTarget->pRenderingProfileId, profileStatus);
	if (nullptr != nextLevel && !profileActivated)
	{
		OutputDebugStringA((
			"[MainApp] Target rendering profile activation failed: " +
			profileStatus + "\n").c_str());
	}
	if (profileActivated && SUCCEEDED(CGameInstance::Get().Change_Level(
		ETOUI(request.eTargetLevel), move(nextLevel))))
	{
		CEffectPresentationService::Clear_Level(iPreviousLevel);
		return;
	}
	if (profileActivated && !previousProfileId.empty())
	{
		string rollbackStatus;
		if (!m_RenderingProfiles.Activate_Profile(
			previousProfileId, rollbackStatus))
		{
			OutputDebugStringA((
				"[MainApp] Rendering profile rollback failed after level activation failure: " +
				rollbackStatus + "\n").c_str());
		}
	}

	if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
	{
		CLobbyCommandService::Cancel(
			request.iLobbyCommandToken,
			"target level activation failed");
	}
	CGameInstance::Get().Clear_Resources(ETOUI(request.eTargetLevel));
	CNetworkManager::Get().Close_ServerConnection();
	CLevelTransitionService::Report_LoadFailure(E_FAIL);
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"main-app.activation-failure"))
	{
		OutputDebugStringA(
			"[MainApp] Failed to stage Lobby recovery after activation failure.\n");
	}
}

HRESULT CMainApp::ReadyImGuiRuntime()
{
	m_pImGuiLayer = make_unique<Engine::CImGuiLayer>();
	if (!m_pImGuiLayer->Initialize(
		g_hWnd,
		m_pDevice.Get(),
		m_pContext.Get()))
	{
		return E_FAIL;
	}
	return S_OK;
}

#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Reset_History();
		pProfiler->Set_Enabled(false);
	}
	m_bProfilerVisible = false;
	m_pHUDLayoutTool =
		make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
	return S_OK;
}

HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	if (nullptr != m_pMapTool && DEBUG_TOOL::MAP != eTool)
		m_pMapTool->SetOpen(false);

	switch (eTool)
	{
	case DEBUG_TOOL::MAP:
		if (nullptr == m_pMapTool)
		{
			auto mapTool = make_unique<CMapTool>();
			if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
				return E_FAIL;
			m_pMapTool = move(mapTool);
		}
		m_pMapTool->SetOpen(true);
		break;
	case DEBUG_TOOL::ANIMATION:
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>();
		if (nullptr == m_pAnimationTool)
			m_pAnimationTool = make_unique<CAnimation_Tool>(
				m_pCharacterPreviewPanel);
		break;
	case DEBUG_TOOL::EFFECT:
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>();
		if (nullptr == m_pEffectTool)
			m_pEffectTool =
				make_unique<CEffect_Tool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel);
		break;
	case DEBUG_TOOL::RENDERING:
		if (!m_bRenderQualityDraftInitialized)
		{
			const SCENE_RENDERING_PROFILE* pProfile =
				m_RenderingProfiles.Get_ActiveProfile();
			if (nullptr == pProfile)
				return E_FAIL;
			m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
			m_SceneRenderingDraft = *pProfile;
			m_strRenderingDraftProfileId = pProfile->strProfileId;
			m_bRenderQualityDraftInitialized = true;
		}
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
	case DEBUG_TOOL::BALANCE:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>(
				make_shared<CNetworkPlayerCommandSink>());
		break;
	default:
		return E_INVALIDARG;
	}

	m_eActiveDebugTool = eTool;
	return S_OK;
}

void CMainApp::RenderDeveloperTools()
{
	if (!ImGui::Begin(
		"LostArk Developer Tools",
		&m_bDeveloperToolsVisible,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	const uint32_t currentLevelId =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isMapEditorWorkspace =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelId &&
		CMapEditorWorkspaceService::Is_Active();
	ImGui::Text("Current level id: %u", currentLevelId);
	ImGui::TextDisabled(isMapEditorWorkspace ?
		"Map Editor is active. Open Map Tool to author the selected Area." :
		"F1 only toggles tools. Enter Map Editor through Lobby Test.");
	ImGui::SeparatorText("Tools");

	const auto toolButton = [this](
		const char_t* pLabel,
		const DEBUG_TOOL eTool,
		const bool_t isEnabled)
	{
		ImGui::BeginDisabled(!isEnabled);
		if (ImGui::Button(pLabel))
		{
			m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
				"Tool opened." : "Tool initialization failed.";
		}
		ImGui::EndDisabled();
	};

	toolButton("Map Tool", DEBUG_TOOL::MAP, isMapEditorWorkspace);
	ImGui::SameLine();
	toolButton(
		"Animation Tool",
		DEBUG_TOOL::ANIMATION,
		true);
	toolButton("Effect Tool", DEBUG_TOOL::EFFECT, true);
	ImGui::SameLine();
	toolButton("Rendering Workbench", DEBUG_TOOL::RENDERING, true);
	ImGui::SameLine();
	toolButton("HUD Layout Tool", DEBUG_TOOL::UI, true);
	ImGui::SameLine();
	toolButton("Balance Tool", DEBUG_TOOL::BALANCE, true);
	ImGui::TextWrapped("%s", m_strToolStatus.c_str());

	ImGui::SeparatorText("Diagnostics");
	const ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("FPS: %.1f  |  Frame: %.2f ms",
		io.Framerate,
		io.DeltaTime > 0.f ? io.DeltaTime * 1000.f : 0.f);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"FPS is ImGui's rolling average; Frame is the latest frame time.");
	bool_t profilerVisible = m_bProfilerVisible;
	if (ImGui::Checkbox("Profiler", &profilerVisible))
	{
		m_bProfilerVisible = profilerVisible;
		if (Engine::CProfiler* pProfiler =
			CGameInstance::Get().Get_Profiler())
		{
			if (m_bProfilerVisible)
				pProfiler->Reset_History();
			pProfiler->Set_Enabled(m_bProfilerVisible);
		}
	}
	ImGui::TextDisabled("F1: Developer Tools  |  F6: Follow/Free Camera");
	ImGui::End();
}

void CMainApp::RenderRenderingWorkbench()
{
	const SCENE_RENDERING_PROFILE* pActiveProfile =
		m_RenderingProfiles.Get_ActiveProfile();
	if (nullptr == pActiveProfile)
		return;
	if (!m_bRenderQualityDraftInitialized ||
		m_strRenderingDraftProfileId != pActiveProfile->strProfileId)
	{
		m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
		m_SceneRenderingDraft = *pActiveProfile;
		m_strRenderingDraftProfileId = pActiveProfile->strProfileId;
		m_bRenderQualityDraftInitialized = true;
	}

	if (!ImGui::Begin(
		"Rendering Workbench",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	ImGui::Text("Pipeline: legacy_deferred_v1");
	ImGui::Text("Scene profile: %s", m_strRenderingDraftProfileId.c_str());
	ImGui::Text("Viewport: %.0f x %.0f", viewportSize.x, viewportSize.y);
	ImGui::TextDisabled(
		"FP16 Light -> SceneHDR -> Screen Post -> half-res Bloom -> Hable/FXAA -> UI");
	ImGui::TextDisabled(
		"Global technical settings and scene artistic multipliers are stored separately.");

	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	ImGui::SeparatorText("Effect Presentation");
	bool_t bEffectLights = Presentation.Are_TransientLightsEnabled();
	if (ImGui::Checkbox("Typed Effect Lights", &bEffectLights))
		Presentation.Set_TransientLightsEnabled(bEffectLights);
	ImGui::SameLine();
	bool_t bEffectPosts = Presentation.Are_ScreenPostsEnabled();
	if (ImGui::Checkbox("Typed Effect Screen Posts", &bEffectPosts))
		Presentation.Set_ScreenPostsEnabled(bEffectPosts);
	ImGui::Text("Last submitted: Light %u | Screen Post %u",
		Presentation.Get_LastTransientLightCount(),
		Presentation.Get_LastScreenPostCount());
	ImGui::TextDisabled(
		"Effect Base/Mask/Dissolve/Distortion/Emissive enter SceneHDR before these posts and Bloom.");

	const auto applyGlobal = [this]()
	{
		m_RenderingProfiles.Apply_GlobalQuality(
			m_RenderQualityDraft, m_strRenderingStatus);
		m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
	};
	const auto applyScene = [this]()
	{
		m_RenderingProfiles.Apply_ActiveProfile(
			m_SceneRenderingDraft, m_strRenderingStatus);
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Get_ActiveProfile())
		{
			m_SceneRenderingDraft = *pProfile;
		}
	};

	bool_t globalChanged = false;
	ImGui::SeparatorText("Global Technical Quality");
	globalChanged |= ImGui::Checkbox(
		"Enabled##SSAO", &m_RenderQualityDraft.bSSAOEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bSSAOEnabled);
	globalChanged |= ImGui::DragFloat(
		"SSAO Radius", &m_RenderQualityDraft.fSSAORadius,
		0.01f, 0.01f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Bias", &m_RenderQualityDraft.fSSAOBias,
		0.001f, 0.f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Intensity", &m_RenderQualityDraft.fSSAOIntensity,
		0.01f, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Power", &m_RenderQualityDraft.fSSAOPower,
		0.01f, 0.1f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Distance Fade", &m_RenderQualityDraft.fSSAODistanceFade,
		0.25f, 1.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"SSAO darkens ambient lighting only; direct light, emissive, and Bloom remain independent.");

	ImGui::SeparatorText("Bloom / Tone / Anti-Aliasing");
	globalChanged |= ImGui::Checkbox(
		"Enabled##Bloom", &m_RenderQualityDraft.bBloomEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bBloomEnabled);
	globalChanged |= ImGui::DragFloat(
		"Threshold", &m_RenderQualityDraft.fBloomThreshold,
		0.01f, 0.f, 64.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Soft Knee", &m_RenderQualityDraft.fBloomSoftKnee,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Base Bloom Intensity", &m_RenderQualityDraft.fBloomIntensity,
		0.01f, 0.f, 16.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Scatter", &m_RenderQualityDraft.fBloomScatter,
		0.01f, 0.25f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Bloom spreads pixels already above Threshold; it does not replace lighting or GI.");

	globalChanged |= ImGui::DragFloat(
		"Base Exposure", &m_RenderQualityDraft.fExposure,
		0.01f, 0.01f, 32.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Hable White Point", &m_RenderQualityDraft.fWhitePoint,
		0.05f, 1.f, 64.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Display Gamma", &m_RenderQualityDraft.fGamma,
		0.005f, 1.f, 3.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

	globalChanged |= ImGui::Checkbox(
		"FXAA Enabled", &m_RenderQualityDraft.bFXAAEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bFXAAEnabled);
	globalChanged |= ImGui::DragFloat(
		"FXAA Blend", &m_RenderQualityDraft.fFXAASubpixel,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"FXAA Edge Threshold", &m_RenderQualityDraft.fFXAAEdgeThreshold,
		0.001f, 0.0312f, 0.333f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"FXAA Edge Threshold Min", &m_RenderQualityDraft.fFXAAEdgeThresholdMin,
		0.001f, 0.0156f, 0.0833f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled("FXAA is evaluated before display-space UI, so HUD text stays sharp.");

	if (globalChanged)
	{
		m_RenderQualityDraft.fSSAOBias = (std::min)(
			m_RenderQualityDraft.fSSAOBias,
			(std::max)(0.f, m_RenderQualityDraft.fSSAORadius - 0.0001f));
		m_RenderQualityDraft.fSSAODistanceFade = (std::max)(
			m_RenderQualityDraft.fSSAODistanceFade,
			m_RenderQualityDraft.fSSAORadius);
		applyGlobal();
	}

	bool_t sceneChanged = false;
	ImGui::SeparatorText("Active Scene Artistic Profile");
	sceneChanged |= ImGui::DragFloat3(
		"Light Direction", &m_SceneRenderingDraft.Light.vDirection.x,
		0.01f, -8.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat3(
		"Diffuse RGB", &m_SceneRenderingDraft.Light.vDiffuse.x,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat3(
		"Ambient RGB", &m_SceneRenderingDraft.Light.vAmbient.x,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat3(
		"Specular RGB", &m_SceneRenderingDraft.Light.vSpecular.x,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Exposure Multiplier", &m_SceneRenderingDraft.fExposureMultiplier,
		0.005f, 0.1f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Bloom Intensity Multiplier",
		&m_SceneRenderingDraft.fBloomIntensityMultiplier,
		0.005f, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::Checkbox(
		"Directional Shadow Enabled",
		&m_SceneRenderingDraft.ShadowSettings.bEnabled);
	ImGui::BeginDisabled(!m_SceneRenderingDraft.ShadowSettings.bEnabled);
	sceneChanged |= ImGui::DragFloat3(
		"Shadow Focus", &m_SceneRenderingDraft.vShadowFocus.x,
		0.1f, -100000.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Light Distance", &m_SceneRenderingDraft.fShadowDistance,
		0.1f, 0.1f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Coverage Width",
		&m_SceneRenderingDraft.ShadowSettings.fOrthographicWidth,
		0.1f, 0.1f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Coverage Height",
		&m_SceneRenderingDraft.ShadowSettings.fOrthographicHeight,
		0.1f, 0.1f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Near", &m_SceneRenderingDraft.ShadowSettings.fNear,
		0.01f, 0.0001f, 100000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Far", &m_SceneRenderingDraft.ShadowSettings.fFar,
		0.1f, 0.0001f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Depth Bias", &m_SceneRenderingDraft.ShadowSettings.fDepthBias,
		0.00005f, 0.f, 0.05f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Normal Bias", &m_SceneRenderingDraft.ShadowSettings.fNormalBias,
		0.001f, 0.f, 10.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Strength", &m_SceneRenderingDraft.ShadowSettings.fStrength,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Shadow uses a fixed 2048 depth map with 3x3 PCF; light eye is derived from focus and scene direction.");
	if (sceneChanged)
	{
		m_SceneRenderingDraft.Light.vDirection.w = 0.f;
		m_SceneRenderingDraft.Light.vDiffuse.w = 1.f;
		m_SceneRenderingDraft.Light.vAmbient.w = 1.f;
		m_SceneRenderingDraft.Light.vSpecular.w = 1.f;
		m_SceneRenderingDraft.ShadowSettings.fFar = (std::max)(
			m_SceneRenderingDraft.ShadowSettings.fFar,
			m_SceneRenderingDraft.ShadowSettings.fNear + 0.0001f);
		applyScene();
	}
	ImGui::TextDisabled(
		"Effective Exposure/Bloom = global base x active scene multiplier (never cumulative).");

	ImGui::SeparatorText("Global A/B Actions");
	if (ImGui::Button("Reset Global Legacy Defaults"))
	{
		m_RenderQualityDraft = {};
		m_RenderQualityDraft.bSSAOEnabled = false;
		applyGlobal();
	}
	ImGui::SameLine();
	if (ImGui::Button("Global Reference A/B Start"))
	{
		m_RenderQualityDraft = {};
		m_RenderQualityDraft.fBloomThreshold = 1.4f;
		m_RenderQualityDraft.fBloomSoftKnee = 0.45f;
		m_RenderQualityDraft.fBloomIntensity = 0.2f;
		m_RenderQualityDraft.fBloomScatter = 1.f;
		m_RenderQualityDraft.fExposure = 1.2f;
		m_RenderQualityDraft.bFXAAEnabled = true;
		applyGlobal();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Active"))
	{
		m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Get_ActiveProfile())
		{
			m_SceneRenderingDraft = *pProfile;
			m_strRenderingDraftProfileId = pProfile->strProfileId;
		}
		m_strRenderingStatus = "Drafts reloaded from the active profile service.";
	}

	ImGui::SeparatorText("Authoring Pipeline");
	if (ImGui::Button("Save Authored"))
		m_RenderingProfiles.Save_Authored(m_strRenderingStatus);
	ImGui::SameLine();
	if (ImGui::Button("Publish Runtime"))
		m_RenderingProfiles.Publish_Runtime(m_strRenderingStatus);
	ImGui::SameLine();
	if (ImGui::Button("Reload Runtime"))
	{
		if (m_RenderingProfiles.Reload_Runtime(m_strRenderingStatus))
		{
			m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
			if (const SCENE_RENDERING_PROFILE* pProfile =
				m_RenderingProfiles.Get_ActiveProfile())
			{
				m_SceneRenderingDraft = *pProfile;
				m_strRenderingDraftProfileId = pProfile->strProfileId;
			}
		}
	}
	ImGui::TextWrapped("%s", m_strRenderingStatus.c_str());
	ImGui::TextDisabled(
		"Save changes Authored only; Publish validates/promotes Runtime; Reload commits atomically.");
	ImGui::End();
}

void CMainApp::RenderProfilerOverlay()
{
	if (!m_bProfilerVisible)
		return;

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (nullptr == pProfiler)
		return;

	Engine::FProfilerLiveStats stats{};
	const bool_t hasStats = pProfiler->Get_LiveStats(stats);
	const ImGuiIO& io = ImGui::GetIO();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(viewport->WorkPos.x + 10.f, viewport->WorkPos.y + 30.f),
		ImGuiCond_Always);
	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;
	if (ImGui::Begin("##LostArkPerfOverlay", nullptr, flags))
	{
		ImGui::Text("FPS: %.1f  (%.3f ms)",
			io.Framerate,
			io.Framerate > 0.f ? 1000.f / io.Framerate : 0.f);
		if (hasStats)
		{
			ImGui::Text("CPU: %.3f ms", stats.CpuFrameMs);
			ImGui::Text("GPU: %s",
				stats.GpuValid ? "available" : "warming up");
			if (stats.GpuValid)
				ImGui::Text("GPU time: %.3f ms", stats.GpuFrameMs);
		}
	}
	ImGui::End();
}

void CMainApp::RenderProfilerSettings()
{
	if (!m_bProfilerVisible)
		return;

	if (!ImGui::Begin(
		"LostArk Profiler Details",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (ImGui::Button("Reset profiler history") && nullptr != pProfiler)
		pProfiler->Reset_History();
	ImGui::SameLine();
	if (ImGui::Button("Save profiler JSON"))
	{
		if (nullptr == pProfiler)
		{
			m_strProfilerCaptureStatus = "Profiler is not available.";
		}
		else
		{
			const Engine::FProfilerCaptureSnapshot snapshot =
				pProfiler->Snapshot();
			const uint64_t frameNumber = snapshot.Frames.empty() ?
				0u : snapshot.Frames.back().FrameNumber;
			const filesystem::path outputPath =
				CProfilerCaptureIO::Make_DefaultPath(frameNumber);
			string error;
			m_strProfilerCaptureStatus = CProfilerCaptureIO::Save_Json(
				snapshot,
				outputPath,
				&error) ? "Saved: " + outputPath.string() : error;
		}
	}
	if (!m_strProfilerCaptureStatus.empty())
		ImGui::TextWrapped("%s", m_strProfilerCaptureStatus.c_str());
	ImGui::End();
}

void CMainApp::UpdateDebugToolShortcut()
{
	const bool_t windowFocused =
		IsWindowOwnedByCurrentProcess(GetForegroundWindow());
	const bool_t f1Down = windowFocused &&
		0 != (GetAsyncKeyState(VK_F1) & 0x8000);
	if (f1Down && !m_bF1Down)
		m_bDeveloperToolsVisible = !m_bDeveloperToolsVisible;
	m_bF1Down = f1Down;
}
#endif

unique_ptr<CMainApp> CMainApp::Create()
{
	auto instance = unique_ptr<CMainApp>(new CMainApp());
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}

void CMainApp::Free()
{
	CEffectPresentationService::Release_PreparedResources();
	CEffectCatalog::Clear();
	CNetworkManager::Get().Shutdown();
	CGameInstance::Get().SetInputBlocked(false, false);

#ifdef _DEBUG
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		pProfiler->Set_Enabled(false);
	m_pAnimationTool.reset();
	m_pEffectTool.reset();
	if (nullptr != m_pCharacterPreviewPanel)
		m_pCharacterPreviewPanel->Release(true);
	m_pCharacterPreviewPanel.reset();
	m_pHUDLayoutTool.reset();
	m_pBalanceTool.reset();
	m_pMapTool.reset();
#endif

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->Shutdown();
	m_pImGuiLayer.reset();
	CGameInstance::Get().Release_Engine();
}
```

### G00-07-13. `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` full code

```cpp
#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketStreamParser.h"
#include "Network/PacketWriter.h"

#include <cstdint>
//이 manip -> 콘솔 출력 형식을 제어한다.
//std::hex        // 이후 숫자를 16진수로 출력
//std::setw(2)    // 다음 값의 최소 폭을 2칸으로 설정
//std::setfill('0') // 빈 칸을 0으로 채움
//std::dec        // 다시 10진수 출력
#include <iomanip>
#include <iostream>
#include <limits>
#include <span>
#include <string>
#include <vector>

using namespace LostArk::Shared;

//ananimous namespace 왜?
namespace
{
	struct TEST_RUNNER
	{
		void Require(
			bool condition,
			const char* testName)
		{
			if (condition)
			{
				std::cout
					<< "[PASS]"
					<< testName
					<< '\n';
			}
			else
			{
				++iFailureCount;

				std::cout
					<< "[FAILURE]"
					<< testName
					<< '\n';
			}
		}

		std::size_t iFailureCount = {};
	};


	//받은 packet을 읽어서 콘솔창에 출력한다.
	void Print_Bytes(
		const char* label,
		const std::vector<std::uint8_t>& bytes)
	{
		std::cout << label;

		for (const std::uint8_t value : bytes)
		{
			std::cout
				<< std::hex
				<< std::setw(2)
				<< std::setfill('0')
				<< static_cast<int>(value)
				<< ' ';
		}

		std::cout << std::dec << '\n';
	}

	bool Build_EnterWorldPayload(
		const C2S_ENTER_WORLD& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();

		return true;
	}

	bool Build_EnterAcceptedPayload(
		const S2C_ENTER_ACCEPTED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();

		return true;
	}
	//플레이어 스폰
	bool Build_EnterRejectedPayload(
		const S2C_ENTER_REJECTED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}

	bool Build_PlayerSpawnedPayload(
		const S2C_PLAYER_SPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();

		return true;
	}
	bool Build_WorldEntitySpawnedPayload(
		const S2C_WORLD_ENTITY_SPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	bool Build_WorldEntityDespawnedPayload(
		const S2C_WORLD_ENTITY_DESPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	//플레이어 디스폰
	bool Build_PlayerDespawnedPayload(
		const S2C_PLAYER_DESPAWNED& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();
		return true;
	}

	//플레이어 move
	bool Build_MovePayload(
		const C2S_MOVE& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();
		return true;
	}

	bool Build_UseSkillPayload(
		const C2S_USE_SKILL& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		payload = writer.Get_Buffer();
		return true;
	}
	//플레이어 payload
	bool Build_WorldSnapshotPayload(
		const S2C_WORLD_SNAPSHOT& message,
		std::vector<std::uint8_t>& payload)
	{
		CPacketWriter writer;

		if (!Write_Message(writer, message))
			return false;

		payload = writer.Get_Buffer();
		return true;
	}

	//월드 진입에 대한 테스트
	void Test_EnterWorldRoundTrip(
		TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD source{};
		source.eWorldId = WORLD_ID::TRAINING_GROUND;

		source.eCharacterClass =
			CHARACTER_CLASS_ID::DIMENSIONMASTER;

		source.strNickName = "건보";

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_EnterWorldPayload(
				source,
				payload),
				"Writer Enter World");

		Print_Bytes(
			"Payload bytes : ",
			payload);

		CPacketReader reader{ payload };
		//character class와 nickname string
		C2S_ENTER_WORLD decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Enter World");

		testRunner.Require(
			decoded.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			decoded.eWorldId == WORLD_ID::TRAINING_GROUND,
			"Enter World Contract Round Trip");

		testRunner.Require(
			decoded.eCharacterClass ==
			source.eCharacterClass,
			"Character Class Round Trip");

		testRunner.Require(
			decoded.strNickName ==
			source.strNickName,
			"NickName Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Payload");
	}

	void Test_PlayableCharacterRoster(TEST_RUNNER& testRunner)
	{
		testRunner.Require(
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::LANCE_MASTER) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::GUNSLINGER) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::SLAYER) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::ARTIST) &&
			Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::DIMENSIONMASTER),
			"Accept Six Playable Character Classes");

		testRunner.Require(
			!Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::DESTROYER) &&
			!Is_Supported_Playable_Character_Class(
				CHARACTER_CLASS_ID::END),
			"Reject Reserved Character Classes");
	}
	//유효한 입력에 대한 테스트
	void Test_EnterAcceptedRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_ENTER_ACCEPTED source{};

		source.iPlayerId = 1;
		source.iNetEntityId = 100;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_EnterAcceptedPayload(
				source,
				payload),
			"Writer Enter Accepted");

		Print_Bytes(
			"Accepted payload bytes : ",
			payload);

		testRunner.Require(
			12 == payload.size(),
			"Enter Accepted Payload Size");

		CPacketReader reader{ payload };

		S2C_ENTER_ACCEPTED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Enter Accepted");

		testRunner.Require(
			source.iProtocolVersion == decoded.iProtocolVersion &&
			source.eWorldId == decoded.eWorldId,
			"Accepted Contract Round Trip");

		testRunner.Require(
			source.iPlayerId ==
			decoded.iPlayerId,
			"Player ID Round Trip");

		testRunner.Require(
			source.iNetEntityId ==
			decoded.iNetEntityId,
			"Net Entity ID Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Accepted Payload");
	}
	//플레이어 위치
	void Test_EnterRejectedRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_ENTER_REJECTED source{};
		source.eWorldId = WORLD_ID::VALTAN_ARENA;
		source.eReason = ENTER_WORLD_REJECTION_REASON::ROOM_FULL;
		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_EnterRejectedPayload(source, payload) && 5u == payload.size(),
			"Writer Enter Rejected Room Full");

		CPacketReader reader{ payload };
		S2C_ENTER_REJECTED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			decoded.eWorldId == WORLD_ID::VALTAN_ARENA &&
			decoded.eReason == ENTER_WORLD_REJECTION_REASON::ROOM_FULL &&
			0u == reader.Get_RemainingSize(),
			"Enter Rejected Room Full Round Trip");
		testRunner.Require(
			Is_Known_Packet_Type(PACKET_TYPE::S2C_ENTER_REJECTED),
			"Register Enter Rejected Packet Type");

		S2C_ENTER_REJECTED invalid = source;
		invalid.eReason = ENTER_WORLD_REJECTION_REASON::END;
		CPacketWriter invalidReasonWriter;
		testRunner.Require(
			!Write_Message(invalidReasonWriter, invalid),
			"Reject Unknown Enter Rejection Reason From Writer");
		invalid = source;
		invalid.eWorldId = WORLD_ID::END;
		CPacketWriter invalidWorldWriter;
		testRunner.Require(
			!Write_Message(invalidWorldWriter, invalid),
			"Reject Unknown Enter Rejection World From Writer");
		invalid = source;
		invalid.iProtocolVersion = NETWORK_PROTOCOL_VERSION + 1u;
		CPacketWriter invalidProtocolWriter;
		testRunner.Require(
			!Write_Message(invalidProtocolWriter, invalid),
			"Reject Enter Rejection Protocol Mismatch From Writer");

		CPacketWriter unknownReasonPayload;
		unknownReasonPayload.Write_U16(NETWORK_PROTOCOL_VERSION);
		unknownReasonPayload.Write_U16(
			static_cast<std::uint16_t>(WORLD_ID::VALTAN_ARENA));
		unknownReasonPayload.Write_U8(
			static_cast<std::uint8_t>(ENTER_WORLD_REJECTION_REASON::END));
		CPacketReader unknownReasonReader{ unknownReasonPayload.Get_Buffer() };
		S2C_ENTER_REJECTED unchanged{};
		unchanged.iProtocolVersion = 77u;
		unchanged.eWorldId = WORLD_ID::BERN;
		unchanged.eReason = ENTER_WORLD_REJECTION_REASON::ROOM_FULL;
		testRunner.Require(
			!Read_Message(unknownReasonReader, unchanged) &&
			77u == unchanged.iProtocolVersion &&
			WORLD_ID::BERN == unchanged.eWorldId &&
			ENTER_WORLD_REJECTION_REASON::ROOM_FULL == unchanged.eReason,
			"Reject Unknown Enter Rejection Reason Without Mutation");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			77u == unchanged.iProtocolVersion &&
			WORLD_ID::BERN == unchanged.eWorldId &&
			ENTER_WORLD_REJECTION_REASON::ROOM_FULL == unchanged.eReason,
			"Reject Truncated Enter Rejection Without Mutation");
	}

	void Test_F32RoundTrip(
		TEST_RUNNER& testRunner)
	{
		CPacketWriter writer;

		writer.Write_F32(1.f);

		const std::vector<std::uint8_t> expectedBytes
		{
			0x00, 0x00, 0x80, 0x3F
		};

		testRunner.Require(
			writer.Get_Buffer() == expectedBytes,
			"F32 IEEE754 Bytes");

		CPacketReader reader
		{
			writer.Get_Buffer()
		};

		float decoded = 0.f;

		testRunner.Require(
			reader.Read_F32(decoded),
			"Read F32");

		testRunner.Require(
			decoded == 1.f,
			"F32 Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire F32");

		const std::vector<std::uint8_t> truncatedBytes
		{
			0x00, 0x00, 0x80
		};

		CPacketReader truncatedReader
		{
			truncatedBytes
		};

		float unchanged = 77.f;

		testRunner.Require(
			!truncatedReader.Read_F32(unchanged),
			"Reject Truncated F32");

		testRunner.Require(
			unchanged == 77.f,
			"Failed F32 Does Not Mutate");
	}
	//플레이어 스폰
	void Test_PlayerSpawnedRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_PLAYER_SPAWNED source{};

		source.iPlayerId = 1;
		source.iNetEntityId = 100;
		source.eCharacterClass =
			CHARACTER_CLASS_ID::DIMENSIONMASTER;
		source.strNickName = "건보";
		source.fPositionX = 1.f;
		source.fPositionY = 2.f;
		source.fPositionZ = 3.f;
		source.fYawDegrees = 90.f;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_PlayerSpawnedPayload(
				source,
				payload),
			"Writer Player Spawned");

		Print_Bytes(
			"Player Spawned payload bytes : ",
			payload);

		const std::vector<std::uint8_t> expectedPayload
		{
			0x01, 0x00, 0x00, 0x00,
			0x64, 0x00, 0x00, 0x00,
			0x05,
			0x06, 0x00,
			0xEA, 0xB1, 0xB4,
			0xEB, 0xB3, 0xB4,
			0x00, 0x00, 0x80, 0x3F,
			0x00, 0x00, 0x00, 0x40,
			0x00, 0x00, 0x40, 0x40,
			0x00, 0x00, 0xB4, 0x42
		};

		testRunner.Require(
			payload.size() == 33,
			"Player Spawned Payload Size");

		testRunner.Require(
			payload == expectedPayload,
			"Player Spawned Payload Layout");

		CPacketReader reader{ payload };
		S2C_PLAYER_SPAWNED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Player Spawned");

		testRunner.Require(
			decoded.iPlayerId == source.iPlayerId &&
			decoded.iNetEntityId == source.iNetEntityId,
			"Spawned IDs Round Trip");

		testRunner.Require(
			decoded.eCharacterClass == source.eCharacterClass &&
			decoded.strNickName == source.strNickName,
			"Spawned Identity Round Trip");

		testRunner.Require(
			decoded.fPositionX == source.fPositionX &&
			decoded.fPositionY == source.fPositionY &&
			decoded.fPositionZ == source.fPositionZ &&
			decoded.fYawDegrees == source.fYawDegrees,
			"Spawned Transform Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Spawned Payload");
	}
	//유효하지 않은 플레이어 스폰
	void Test_InvalidPlayerSpawnedPayloads(
		TEST_RUNNER& testRunner)
	{
		S2C_PLAYER_SPAWNED valid{};

		valid.iPlayerId = 1;
		valid.iNetEntityId = 100;
		valid.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;
		valid.strNickName = "건보";
		valid.fPositionX = 1.f;
		valid.fPositionY = 2.f;
		valid.fPositionZ = 3.f;
		valid.fYawDegrees = 90.f;

		S2C_PLAYER_SPAWNED invalidId = valid;
		invalidId.iPlayerId = INVALID_PLAYER_ID;

		CPacketWriter invalidIdWriter;

		testRunner.Require(
			!Write_Message(invalidIdWriter, invalidId),
			"Reject Spawned Zero Player ID");

		S2C_PLAYER_SPAWNED invalidClass = valid;
		invalidClass.eCharacterClass =
			static_cast<CHARACTER_CLASS_ID>(0xFF);

		CPacketWriter invalidClassWriter;

		testRunner.Require(
			!Write_Message(invalidClassWriter, invalidClass),
			"Reject Spawned Invalid Class");

		S2C_PLAYER_SPAWNED invalidPosition = valid;
		invalidPosition.fPositionX =
			std::numeric_limits<float>::infinity();

		CPacketWriter invalidPositionWriter;

		testRunner.Require(
			!Write_Message(
				invalidPositionWriter,
				invalidPosition),
			"Reject Spawned Infinite Position");

		std::vector<std::uint8_t> truncatedPayload;

		const bool builtPayload =
			Build_PlayerSpawnedPayload(
				valid,
				truncatedPayload);

		testRunner.Require(
			builtPayload,
			"Build Spawned Truncation Source");

		if (!builtPayload)
			return;

		truncatedPayload.pop_back();

		CPacketReader truncatedReader
		{
			truncatedPayload
		};

		S2C_PLAYER_SPAWNED unchanged{};

		unchanged.iPlayerId = 77;
		unchanged.iNetEntityId = 88;
		unchanged.eCharacterClass =
			CHARACTER_CLASS_ID::ARTIST;
		unchanged.strNickName = "keep";
		unchanged.fPositionX = 10.f;
		unchanged.fPositionY = 20.f;
		unchanged.fPositionZ = 30.f;
		unchanged.fYawDegrees = 40.f;

		testRunner.Require(
			!Read_Message(
				truncatedReader,
				unchanged),
			"Reject Truncated Player Spawned");

		testRunner.Require(
			unchanged.iPlayerId == 77 &&
			unchanged.iNetEntityId == 88 &&
			unchanged.eCharacterClass ==
				CHARACTER_CLASS_ID::ARTIST &&
			unchanged.strNickName == "keep" &&
			unchanged.fPositionX == 10.f &&
			unchanged.fPositionY == 20.f &&
			unchanged.fPositionZ == 30.f &&
			unchanged.fYawDegrees == 40.f,
			"Failed Spawn Does Not Mutate Message");
	}

	void Test_WorldEntitySpawnedRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_WORLD_ENTITY_SPAWNED source{};
		source.iNetEntityId = 900;
		source.eKind = WORLD_ENTITY_KIND::BOSS;
		source.strArchetypeId = "BOSS_VALTAN";
		source.strEncounterId = "ENCOUNTER_VALTAN";
		source.strPlacementId = "boss_valtan_1";
		source.fPositionX = 151.25f;
		source.fPositionY = 22.97f;
		source.fPositionZ = -121.75f;
		source.fYawDegrees = 225.f;
		source.fCollisionRadius = 3.f;

		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_WorldEntitySpawnedPayload(source, payload),
			"Writer World Entity Spawned");
		CPacketReader reader{ payload };
		S2C_WORLD_ENTITY_SPAWNED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded),
			"Read World Entity Spawned");
		testRunner.Require(
			decoded.iNetEntityId == source.iNetEntityId &&
			decoded.eKind == source.eKind &&
			decoded.strArchetypeId == source.strArchetypeId &&
			decoded.strEncounterId == source.strEncounterId &&
			decoded.strPlacementId == source.strPlacementId &&
			decoded.fPositionX == source.fPositionX &&
			decoded.fYawDegrees == source.fYawDegrees &&
			decoded.fCollisionRadius == source.fCollisionRadius,
			"World Entity Spawned Round Trip");
		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire World Entity Spawn");

		S2C_WORLD_ENTITY_SPAWNED invalid = source;
		invalid.strArchetypeId = "../BOSS_VALTAN";
		CPacketWriter invalidWriter;
		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject Unstable World Archetype ID");
		invalid = source;
		invalid.strPlacementId = "../boss_valtan_1";
		CPacketWriter invalidPlacementWriter;
		testRunner.Require(
			!Write_Message(invalidPlacementWriter, invalid),
			"Reject Unstable World Placement ID");
		invalid = source;
		invalid.strPlacementId.clear();
		CPacketWriter emptyPlacementWriter;
		testRunner.Require(
			Write_Message(emptyPlacementWriter, invalid),
			"Allow Dynamic Spawn Without Placement ID");
		invalid = source;
		invalid.fCollisionRadius = 0.f;
		CPacketWriter zeroRadiusWriter;
		testRunner.Require(
			!Write_Message(zeroRadiusWriter, invalid),
			"Reject Combat Entity Without Collision Radius");
		invalid = source;
		invalid.eKind = WORLD_ENTITY_KIND::NPC;
		CPacketWriter npcRadiusWriter;
		testRunner.Require(
			!Write_Message(npcRadiusWriter, invalid),
			"Reject NPC With Combat Collision Radius");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		S2C_WORLD_ENTITY_SPAWNED unchanged{};
		unchanged.iNetEntityId = 77u;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			unchanged.iNetEntityId == 77u,
			"Reject Truncated World Entity Collision Radius Without Mutation");
	}

	void Test_WorldEntityDespawnedRoundTrip(TEST_RUNNER& testRunner)
	{
		S2C_WORLD_ENTITY_DESPAWNED source{};
		source.iNetEntityId = 901u;
		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_WorldEntityDespawnedPayload(source, payload),
			"Writer World Entity Despawned");
		testRunner.Require(
			payload == std::vector<std::uint8_t>{ 0x85, 0x03, 0x00, 0x00 },
			"World Entity Despawned Payload Layout");

		CPacketReader reader{ payload };
		S2C_WORLD_ENTITY_DESPAWNED decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iNetEntityId == source.iNetEntityId &&
			0u == reader.Get_RemainingSize(),
			"World Entity Despawned Round Trip");

		S2C_WORLD_ENTITY_DESPAWNED invalid{};
		CPacketWriter invalidWriter;
		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject World Entity Despawned Zero Entity ID");
	}
	//유효하지 않은 입력에 대한 테스트
	void Test_InvalidEnterAcceptedPayloads(
		TEST_RUNNER& testRunner)
	{
		S2C_ENTER_ACCEPTED invalidPlayer{};

		invalidPlayer.iPlayerId =
			INVALID_PLAYER_ID;

		invalidPlayer.iNetEntityId = 100;

		CPacketWriter invalidPlayerWriter;

		testRunner.Require(
			!Write_Message(
				invalidPlayerWriter,
				invalidPlayer),
			"Reject Zero Player ID From Writer");

		S2C_ENTER_ACCEPTED invalidEntity{};

		invalidEntity.iPlayerId = 1;

		invalidEntity.iNetEntityId =
			INVALID_NET_ENTITY_ID;

		CPacketWriter invalidEntityWriter;

		testRunner.Require(
			!Write_Message(
				invalidEntityWriter,
				invalidEntity),
			"Reject Zero Net Entity ID From Writer");

		CPacketWriter zeroPlayerPayloadWriter;
		zeroPlayerPayloadWriter.Write_U16(NETWORK_PROTOCOL_VERSION);
		zeroPlayerPayloadWriter.Write_U16(
			static_cast<std::uint16_t>(WORLD_ID::BERN));

		zeroPlayerPayloadWriter.Write_U32(
			INVALID_PLAYER_ID);

		zeroPlayerPayloadWriter.Write_U32(100);

		CPacketReader zeroPlayerReader
		{
			zeroPlayerPayloadWriter.Get_Buffer()
		};

		S2C_ENTER_ACCEPTED decoded{};

		testRunner.Require(
			!Read_Message(
				zeroPlayerReader,
				decoded),
			"Reject Zero Player ID From Reader");

		CPacketWriter zeroEntityPayloadWriter;
		zeroEntityPayloadWriter.Write_U16(NETWORK_PROTOCOL_VERSION);
		zeroEntityPayloadWriter.Write_U16(
			static_cast<std::uint16_t>(WORLD_ID::BERN));

		zeroEntityPayloadWriter.Write_U32(1);

		zeroEntityPayloadWriter.Write_U32(
			INVALID_NET_ENTITY_ID);

		CPacketReader zeroEntityReader
		{
			zeroEntityPayloadWriter.Get_Buffer()
		};

		testRunner.Require(
			!Read_Message(
				zeroEntityReader,
				decoded),
			"Reject Zero Net Entity ID From Reader");

		const std::vector<std::uint8_t>
			truncatedAccepted
		{
			0x06, 0x00,
			0x01, 0x00,
			0x01, 0x00, 0x00, 0x00,
			0x64, 0x00, 0x00
		};

		CPacketReader truncatedReader
		{
			truncatedAccepted
		};

		S2C_ENTER_ACCEPTED unchanged{};

		unchanged.iPlayerId = 77;
		unchanged.iNetEntityId = 88;

		testRunner.Require(
			!Read_Message(
				truncatedReader,
				unchanged),
			"Reject Truncated Enter Accepted");

		testRunner.Require(
			77 == unchanged.iPlayerId &&
			88 == unchanged.iNetEntityId,
			"Failed Accepted Does Not Mutate Message");
	}

	void Test_InvalidPayloads(
		TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD tooLong{};

		tooLong.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;
		// max nick name bytes + 1에 'A'가 의미하는 거는 뭐임?
		tooLong.strNickName =
			std::string(
				MAX_NICKNAME_BYTES + 1,
				'A');

		CPacketWriter longWriter;

		testRunner.Require(
			!Write_Message(
				longWriter,
				tooLong),
			"Reject Long Nickname");

		C2S_ENTER_WORLD invalidProtocol = tooLong;
		invalidProtocol.strNickName = "valid";
		invalidProtocol.iProtocolVersion =
			NETWORK_PROTOCOL_VERSION + 1;
		CPacketWriter protocolWriter;
		testRunner.Require(
			!Write_Message(protocolWriter, invalidProtocol),
			"Reject Unsupported Protocol Version");

		C2S_ENTER_WORLD invalidWorld = invalidProtocol;
		invalidWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		invalidWorld.eWorldId = WORLD_ID::END;
		CPacketWriter worldWriter;
		testRunner.Require(
			!Write_Message(worldWriter, invalidWorld),
			"Reject Unknown World ID");

		const std::vector<std::uint8_t> invalidClass
		{
			0x06, 0x00,
			0x01, 0x00,
			0xFF, 0x00, 0x00
		};

		CPacketReader invalidClassReader
		{
			invalidClass
		};

		C2S_ENTER_WORLD decoded{};

		testRunner.Require(
			!Read_Message(
				invalidClassReader,
				decoded),
			"Reject Invalid Character Class");
		//이 truncated string이 의미하는 게 뭐지?
		const std::vector<std::uint8_t> truncatedString
		{
			0x06, 0x00,
			0x01, 0x00,
			0x00, 0x06, 0x00, 0xEA
		};

		CPacketReader truncatedReader
		{
			truncatedString
		};


		testRunner.Require(
			!Read_Message(
				truncatedReader,
				decoded),
			"Reject Truncated Nickname");

		const std::vector<std::uint8_t>
			threeBytes
		{
			0x01,
			0x02,
			0x03
		};
		CPacketReader u32Reader{ threeBytes };
		std::uint32_t value = {};

		testRunner.Require(
			!u32Reader.Read_U32(value),
			"Reject Truncated U32");

		testRunner.Require(
			3 == u32Reader.Get_RemainingSize(),
			"Failed U32 Does Not Consume");
	}
	void Test_StreamFraming(TEST_RUNNER& testRunner)
	{
		C2S_ENTER_WORLD source{};

		source.eCharacterClass =
			CHARACTER_CLASS_ID::LANCE_MASTER;

		source.strNickName = "건보";

		std::vector<std::uint8_t> payload;
		//한 프레임에 필요한 payload를 구현한다는 것 맞나?
		testRunner.Require(
			Build_EnterWorldPayload(
				source,
				payload),
			"Build Payload For Frame");

		std::vector<std::uint8_t> frameBytes;

		testRunner.Require(
			Build_Packet_Frame(
				PACKET_TYPE::C2S_ENTER_WORLD,
				payload,
				frameBytes),
			"Build Packet Frame");
		Print_Bytes(
			"Frame bytes: ",
			frameBytes);

		CPacketStreamParser splitParser;

		bool needMoreDataWasCorrect = true;

		for (std::size_t i = 0;
			i + 1 < frameBytes.size();
			++i)
		{
			const std::span<const std::uint8_t>
				oneByte
			{
				frameBytes.data() + i,
				1
			};

			needMoreDataWasCorrect &=
				splitParser.Append(oneByte);

			PACKET_FRAME frame{};

			needMoreDataWasCorrect &=
				PACKET_PARSE_RESULT::
				NEED_MORE_DATA ==
				splitParser.Try_Pop(frame);
		}

		testRunner.Require(
			needMoreDataWasCorrect,
			"Wait For Split Frame");

		testRunner.Require(
			splitParser.Append(
				std::span<const std::uint8_t>
		{
			frameBytes.data() +
				frameBytes.size() - 1,
				1
		}),
			"Append Last Frame Byte");

		PACKET_FRAME splitFrame{};

		testRunner.Require(
			PACKET_PARSE_RESULT::FRAME_READY ==
			splitParser.Try_Pop(splitFrame),
			"Pop Split Frame");

		testRunner.Require(
			splitFrame.ePacketType ==
			PACKET_TYPE::C2S_ENTER_WORLD,
			"Split Frame Packet Type");

		testRunner.Require(
			splitFrame.Payload == payload,
			"Split Frame Payload");

		std::vector<std::uint8_t> combined;

		combined.insert(
			combined.end(),
			frameBytes.begin(),
			frameBytes.end());

		combined.insert(
			combined.end(),
			frameBytes.begin(),
			frameBytes.end());

		CPacketStreamParser combinedParser;

		testRunner.Require(
			combinedParser.Append(combined),
			"Append Combined Frames");

		PACKET_FRAME first{};
		PACKET_FRAME second{};
		PACKET_FRAME none{};

		testRunner.Require(
			PACKET_PARSE_RESULT::FRAME_READY ==
			combinedParser.Try_Pop(first),
			"Pop First Combined Frame");

		testRunner.Require(
			PACKET_PARSE_RESULT::FRAME_READY ==
			combinedParser.Try_Pop(second),
			"Pop Second Combined Frame");

		testRunner.Require(
			PACKET_PARSE_RESULT::NEED_MORE_DATA ==
			combinedParser.Try_Pop(none),
			"No Third Combined Frame");

		CPacketWriter invalidHeaderWriter;

		invalidHeaderWriter.Write_U32(
			static_cast<std::uint32_t>(
				PACKET_HEADER_BYTES));

		invalidHeaderWriter.Write_U16(
			0xFFFF);

		CPacketStreamParser invalidParser;

		invalidParser.Append(
			invalidHeaderWriter.Get_Buffer());

		PACKET_FRAME invalidFrame{};

		testRunner.Require(
			PACKET_PARSE_RESULT::INVALID_FRAME ==
			invalidParser.Try_Pop(
				invalidFrame),
			"Reject Unknown Packet Type");
	}

	//플레이어 디스폰
	void Test_PlayerDespawnedRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_PLAYER_DESPAWNED source{};
		source.iNetEntityId = 100;
		source.eReason =
			PLAYER_DESPAWN_REASON::DISCONNECTED;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_PlayerDespawnedPayload(source, payload),
			"Writer Player Despawned");

		const std::vector<std::uint8_t> expected
		{
			0x64, 0x00, 0x00, 0x00,
			0x00
		};

		testRunner.Require(
			payload == expected,
			"Player Despawned Payload Layout");

		CPacketReader reader{ payload };
		S2C_PLAYER_DESPAWNED decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Player Despawned");

		testRunner.Require(
			decoded.iNetEntityId == source.iNetEntityId &&
			decoded.eReason == source.eReason,
			"Player Despawned Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Despawned Payload");

		S2C_PLAYER_DESPAWNED invalidId = source;
		invalidId.iNetEntityId = INVALID_NET_ENTITY_ID;
		CPacketWriter invalidIdWriter;

		testRunner.Require(
			!Write_Message(invalidIdWriter, invalidId),
			"Reject Despawned Zero Entity ID");

		S2C_PLAYER_DESPAWNED invalidReason = source;
		invalidReason.eReason =
			PLAYER_DESPAWN_REASON::END;
		CPacketWriter invalidReasonWriter;

		testRunner.Require(
			!Write_Message(invalidReasonWriter, invalidReason),
			"Reject Invalid Despawn Reason");

		std::vector<std::uint8_t> truncated = payload;
		truncated.pop_back();

		CPacketReader truncatedReader{ truncated };
		S2C_PLAYER_DESPAWNED unchanged{};
		unchanged.iNetEntityId = 777;
		unchanged.eReason =
			PLAYER_DESPAWN_REASON::KICKED;

		testRunner.Require(
			!Read_Message(truncatedReader, unchanged),
			"Reject Truncated Player Despawned");

		testRunner.Require(
			unchanged.iNetEntityId == 777 &&
			unchanged.eReason ==
			PLAYER_DESPAWN_REASON::KICKED,
			"Failed Despawn Does Not Mutate");
	}

	void Test_MoveRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_MOVE source{};
		source.iClientSequence = 7;
		source.fGoalX = 10.f;
		source.fGoalZ = -5.f;

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_MovePayload(source, payload),
			"Writer Move Goal");

		const std::vector<std::uint8_t> expected
		{
			0x07, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x20, 0x41,
			0x00, 0x00, 0xA0, 0xC0
		};

		testRunner.Require(
			payload == expected,
			"Move Goal Payload Layout");

		CPacketReader reader{ payload };
		C2S_MOVE decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Move Goal");

		testRunner.Require(
			decoded.iClientSequence == 7 &&
			decoded.fGoalX == 10.f &&
			decoded.fGoalZ == -5.f,
			"Move Goal Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire Move Goal");

		C2S_MOVE invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter sequenceWriter;

		testRunner.Require(
			!Write_Message(sequenceWriter, invalidSequence),
			"Reject Zero Move Sequence");

		C2S_MOVE invalidGoal = source;
		invalidGoal.fGoalX =
			std::numeric_limits<float>::infinity();
		CPacketWriter goalWriter;

		testRunner.Require(
			!Write_Message(goalWriter, invalidGoal),
			"Reject Infinite Move Goal");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_MOVE unchanged{};
		unchanged.iClientSequence = 99;
		unchanged.fGoalX = 77.f;
		unchanged.fGoalZ = 88.f;

		testRunner.Require(
			!Read_Message(truncatedReader, unchanged),
			"Reject Truncated Move Goal");

		testRunner.Require(
			unchanged.iClientSequence == 99 &&
			unchanged.fGoalX == 77.f &&
			unchanged.fGoalZ == 88.f,
			"Failed Move Does Not Mutate");
	}

	void Test_UseSkillRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_USE_SKILL source{};
		source.iClientSequence = 9;
		source.iSkillId = 34060;
		source.fAimX = 151.f;
		source.fAimZ = -122.f;

		std::vector<std::uint8_t> payload;
		testRunner.Require(
			Build_UseSkillPayload(source, payload),
			"Writer Use Skill");
		testRunner.Require(
			16 == payload.size(),
			"Use Skill Payload Size");

		CPacketReader reader{ payload };
		C2S_USE_SKILL decoded{};
		testRunner.Require(
			Read_Message(reader, decoded),
			"Read Use Skill");
		testRunner.Require(
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
			decoded.fAimX == source.fAimX &&
			decoded.fAimZ == source.fAimZ &&
			0 == reader.Get_RemainingSize(),
			"Use Skill Round Trip");

		C2S_USE_SKILL invalid = source;
		invalid.iSkillId = INVALID_SKILL_ID;
		CPacketWriter invalidWriter;
		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject Invalid Skill ID");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_USE_SKILL unchanged{};
		unchanged.iClientSequence = 77;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			77 == unchanged.iClientSequence,
			"Reject Truncated Skill Without Mutation");
	}

	void Test_ReleaseSkillRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_RELEASE_SKILL source{};
		source.iClientSequence = 12;
		source.iSkillId = 34590;

		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source),
			"Writer Release Skill");
		std::vector<std::uint8_t> payload = writer.Get_Buffer();
		testRunner.Require(
			8 == payload.size(),
			"Release Skill Payload Size");

		CPacketReader reader{ payload };
		C2S_RELEASE_SKILL decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
			0 == reader.Get_RemainingSize(),
			"Release Skill Round Trip");

		C2S_RELEASE_SKILL invalidSkill = source;
		invalidSkill.iSkillId = INVALID_SKILL_ID;
		CPacketWriter invalidSkillWriter;
		testRunner.Require(
			!Write_Message(invalidSkillWriter, invalidSkill),
			"Reject Release Without Skill ID");

		C2S_RELEASE_SKILL invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter invalidSequenceWriter;
		testRunner.Require(
			!Write_Message(invalidSequenceWriter, invalidSequence),
			"Reject Release Without Sequence");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_RELEASE_SKILL unchanged{};
		unchanged.iClientSequence = 55;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			55 == unchanged.iClientSequence,
			"Failed Release Does Not Mutate");
	}

	void Test_UpdateSkillAimRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_UPDATE_SKILL_AIM source{};
		source.iClientSequence = 14;
		source.iSkillId = 34590;
		source.fAimX = 148.5f;
		source.fAimZ = -119.25f;

		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source),
			"Writer Update Skill Aim");
		std::vector<std::uint8_t> payload = writer.Get_Buffer();
		testRunner.Require(
			16 == payload.size(),
			"Update Skill Aim Payload Size");

		CPacketReader reader{ payload };
		C2S_UPDATE_SKILL_AIM decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			decoded.iSkillId == source.iSkillId &&
			decoded.fAimX == source.fAimX &&
			decoded.fAimZ == source.fAimZ &&
			0 == reader.Get_RemainingSize(),
			"Update Skill Aim Round Trip");

		C2S_UPDATE_SKILL_AIM invalidSkill = source;
		invalidSkill.iSkillId = INVALID_SKILL_ID;
		CPacketWriter invalidSkillWriter;
		testRunner.Require(
			!Write_Message(invalidSkillWriter, invalidSkill),
			"Reject Aim Update Without Skill ID");

		C2S_UPDATE_SKILL_AIM invalidSequence = source;
		invalidSequence.iClientSequence = 0;
		CPacketWriter invalidSequenceWriter;
		testRunner.Require(
			!Write_Message(invalidSequenceWriter, invalidSequence),
			"Reject Aim Update Without Sequence");

		C2S_UPDATE_SKILL_AIM invalidAim = source;
		invalidAim.fAimX = std::numeric_limits<float>::quiet_NaN();
		CPacketWriter invalidAimWriter;
		testRunner.Require(
			!Write_Message(invalidAimWriter, invalidAim),
			"Reject Aim Update With Non-Finite Aim");

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		C2S_UPDATE_SKILL_AIM unchanged{};
		unchanged.iClientSequence = 66;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			66 == unchanged.iClientSequence,
			"Failed Aim Update Does Not Mutate");
	}

	void Test_RevivePlayerRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_REVIVE_PLAYER source{};
		source.iClientSequence = 17u;
		CPacketWriter writer;
		testRunner.Require(
			Write_Message(writer, source) && 4u == writer.Get_Buffer().size(),
			"Writer Revive Player");
		CPacketReader reader{ writer.Get_Buffer() };
		C2S_REVIVE_PLAYER decoded{};
		testRunner.Require(
			Read_Message(reader, decoded) &&
			decoded.iClientSequence == source.iClientSequence &&
			0u == reader.Get_RemainingSize(),
			"Revive Player Round Trip");
		C2S_REVIVE_PLAYER invalid{};
		CPacketWriter invalidWriter;
		testRunner.Require(!Write_Message(invalidWriter, invalid),
			"Reject Zero Revive Sequence");
		std::vector<std::uint8_t> truncated = writer.Get_Buffer();
		truncated.pop_back();
		CPacketReader truncatedReader{ truncated };
		C2S_REVIVE_PLAYER unchanged{};
		unchanged.iClientSequence = 99u;
		testRunner.Require(
			!Read_Message(truncatedReader, unchanged) &&
			99u == unchanged.iClientSequence,
			"Reject Truncated Revive Without Mutation");
	}

	void Test_CharacterClassChangeRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_CHANGE_CHARACTER_CLASS request{};
		request.iClientSequence = 41u;
		request.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		CPacketWriter requestWriter;
		testRunner.Require(Write_Message(requestWriter, request),
			"Writer Character Class Change");
		CPacketReader requestReader{ requestWriter.Get_Buffer() };
		C2S_CHANGE_CHARACTER_CLASS decodedRequest{};
		testRunner.Require(Read_Message(requestReader, decodedRequest) &&
			0u == requestReader.Get_RemainingSize() &&
			request.iClientSequence == decodedRequest.iClientSequence &&
			request.eCharacterClass == decodedRequest.eCharacterClass,
			"Character Class Change Round Trip");

		S2C_CHARACTER_CLASS_CHANGE_RESULT result{};
		result.iClientSequence = request.iClientSequence;
		result.eResult = CHARACTER_CLASS_CHANGE_RESULT::ACCEPTED;
		result.eRequestedClass = request.eCharacterClass;
		result.eActiveClass = request.eCharacterClass;
		CPacketWriter resultWriter;
		testRunner.Require(Write_Message(resultWriter, result),
			"Writer Character Class Change Result");
		CPacketReader resultReader{ resultWriter.Get_Buffer() };
		S2C_CHARACTER_CLASS_CHANGE_RESULT decodedResult{};
		testRunner.Require(Read_Message(resultReader, decodedResult) &&
			0u == resultReader.Get_RemainingSize() &&
			result.iClientSequence == decodedResult.iClientSequence &&
			result.eResult == decodedResult.eResult &&
			result.eActiveClass == decodedResult.eActiveClass,
			"Character Class Change Result Round Trip");

		result.eActiveClass = CHARACTER_CLASS_ID::WARLORD;
		CPacketWriter inconsistentWriter;
		testRunner.Require(!Write_Message(inconsistentWriter, result),
			"Reject Accepted Class Change With Different Active Class");
		request.iClientSequence = 0u;
		CPacketWriter staleWriter;
		testRunner.Require(!Write_Message(staleWriter, request),
			"Reject Zero Class Change Sequence");
	}

	void Test_WorldSnapshotRoundTrip(
		TEST_RUNNER& testRunner)
	{
		S2C_WORLD_SNAPSHOT source{};
		source.iServerTick = 30;

		PLAYER_SNAPSHOT first{};
		first.iNetEntityId = 100;
		first.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		first.fPositionX = 1.f;
		first.fPositionY = 0.f;
		first.fPositionZ = 2.f;
		first.fYawDegrees = 90.f;
		first.eLocomotionState =
			PLAYER_LOCOMOTION_STATE::MOVING;
		first.eAction = PLAYER_ACTION_STATE::SKILL;
		first.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		first.iSkillId = 34060;
		first.iActionStartTick = 25;
		first.iCurrentHp = 875;
		first.iMaximumHp = 1000;
		first.iCurrentResource = 80;
		first.iMaximumResource = 100;
		first.iComboStage = 3;
		first.Cooldowns.push_back({ 34060, 330 });

		PLAYER_SNAPSHOT second{};
		second.iNetEntityId = 101;
		second.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		second.fPositionX = 3.f;
		second.fPositionY = 0.f;
		second.fPositionZ = 4.f;
		second.fYawDegrees = 180.f;
		second.eLocomotionState =
			PLAYER_LOCOMOTION_STATE::MOVING;
		second.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
		second.iActionStartTick = 29;
		second.isCombatReady = false;

		source.Players.push_back(first);
		source.Players.push_back(second);

		WORLD_ENTITY_SNAPSHOT entity{};
		entity.iNetEntityId = 900;
		entity.eAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		entity.strPatternId = "VALTAN_SWING";
		entity.strActionId = "valtan.attack.swing.active";
		entity.iPatternSequence = 7u;
		entity.iPatternStageIndex = 1u;
		entity.fPositionX = 150.f;
		entity.fPositionY = 22.97f;
		entity.fPositionZ = -122.f;
		entity.fYawDegrees = 225.f;
		entity.iActionStartTick = 20;
		entity.iCurrentHp = 9500;
		entity.iMaximumHp = 10000;
		entity.iPhase = 1;
		source.Entities.push_back(entity);

		std::vector<std::uint8_t> payload;

		testRunner.Require(
			Build_WorldSnapshotPayload(source, payload),
			"Writer World Snapshot");

		constexpr std::size_t snapshotHeaderBytes =
			4 + 2 + 2 + 2 + 1;
		constexpr std::size_t playerFixedBytes =
			4 + 1 + (4 * 4) + 1 + 1 + 1 + (4 * 8) + 1 + 1 + 1;
		constexpr std::size_t cooldownBytes = 4 + 4;
		const std::size_t entityBytes =
			4 + 1 + 2 + entity.strPatternId.size() + 2 +
			entity.strActionId.size() + (4 * 4) + (4 * 5) + 1;
		const std::size_t expectedPayloadBytes =
			snapshotHeaderBytes +
			(playerFixedBytes * 2) +
			cooldownBytes +
			entityBytes;

		testRunner.Require(
			expectedPayloadBytes == payload.size(),
			"World Snapshot Payload Size");

		CPacketReader reader{ payload };
		S2C_WORLD_SNAPSHOT decoded{};

		testRunner.Require(
			Read_Message(reader, decoded),
			"Read World Snapshot");

		testRunner.Require(
			decoded.iServerTick == 30 &&
			decoded.eWorldId == WORLD_ID::BERN &&
			decoded.Players.size() == 2 &&
			decoded.Entities.size() == 1,
			"World Snapshot Header Round Trip");
		if (2u != decoded.Players.size() || 1u != decoded.Entities.size())
			return;

		testRunner.Require(
			decoded.Players[0].iNetEntityId == 100 &&
			decoded.Players[0].eCharacterClass == CHARACTER_CLASS_ID::LANCE_MASTER &&
			decoded.Players[0].fPositionX == 1.f &&
			decoded.Players[0].fPositionZ == 2.f &&
			decoded.Players[0].eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::MOVING &&
			decoded.Players[0].eAction == PLAYER_ACTION_STATE::SKILL &&
			decoded.Players[0].eStance ==
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR &&
			decoded.Players[0].iSkillId == 34060 &&
			decoded.Players[0].iActionStartTick == 25 &&
			decoded.Players[0].iCurrentHp == 875 &&
			decoded.Players[0].Cooldowns.size() == 1 &&
			decoded.Players[0].Cooldowns[0].iCooldownEndTick == 330 &&
			decoded.Players[0].iComboStage == 3 &&
			decoded.Players[1].iComboStage == 0 &&
			decoded.Players[1].iNetEntityId == 101 &&
			decoded.Players[1].fPositionX == 3.f &&
			decoded.Players[1].fPositionZ == 4.f &&
			decoded.Players[1].eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::MOVING &&
			decoded.Players[1].eAction ==
			PLAYER_ACTION_STATE::TRIGGER_MOVE &&
			decoded.Players[1].iActionStartTick == 29 &&
			!decoded.Players[1].isCombatReady,
			"World Snapshot Players Round Trip");

		testRunner.Require(
			decoded.Entities[0].iNetEntityId == 900 &&
			decoded.Entities[0].eAction == WORLD_ENTITY_ACTION::PATTERN_ACTIVE &&
			decoded.Entities[0].strPatternId == entity.strPatternId &&
			decoded.Entities[0].strActionId == entity.strActionId &&
			decoded.Entities[0].iPatternSequence == 7u &&
			decoded.Entities[0].iPatternStageIndex == 1u &&
			decoded.Entities[0].fPositionX == 150.f &&
			decoded.Entities[0].iCurrentHp == 9500 &&
			decoded.Entities[0].iMaximumHp == 10000 &&
			decoded.Entities[0].iPhase == 1,
			"World Snapshot Entities Round Trip");

		testRunner.Require(
			0 == reader.Get_RemainingSize(),
			"Consume Entire World Snapshot");

		S2C_WORLD_SNAPSHOT empty{};
		empty.iServerTick = 1;
		CPacketWriter emptyWriter;

		testRunner.Require(
			!Write_Message(emptyWriter, empty),
			"Reject Empty World Snapshot");

		S2C_WORLD_SNAPSHOT invalid = source;
		invalid.Players[0].eLocomotionState =
			PLAYER_LOCOMOTION_STATE::END;
		CPacketWriter invalidWriter;

		testRunner.Require(
			!Write_Message(invalidWriter, invalid),
			"Reject Invalid Locomotion State");

		{
			S2C_WORLD_SNAPSHOT overflow = source;
			overflow.Players[0].iComboStage = 9;
			CPacketWriter overflowWriter;
			testRunner.Require(
				!Write_Message(overflowWriter, overflow),
				"Reject Combo Stage Above Maximum");

			S2C_WORLD_SNAPSHOT idleCombo = source;
			idleCombo.Players[1].iComboStage = 1;
			CPacketWriter idleComboWriter;
			testRunner.Require(
				!Write_Message(idleComboWriter, idleCombo),
				"Reject Combo Stage Without Skill Action");

			S2C_WORLD_SNAPSHOT triggerWithoutTick = source;
			triggerWithoutTick.Players[1].iActionStartTick = 0;
			CPacketWriter triggerWithoutTickWriter;
			testRunner.Require(
				!Write_Message(triggerWithoutTickWriter, triggerWithoutTick),
				"Reject Trigger Move Without Action Tick");
		}

		payload.pop_back();
		CPacketReader truncatedReader{ payload };
		S2C_WORLD_SNAPSHOT unchanged{};
		unchanged.iServerTick = 777;

		testRunner.Require(
			!Read_Message(truncatedReader, unchanged),
			"Reject Truncated World Snapshot");

		testRunner.Require(
			unchanged.iServerTick == 777 &&
			unchanged.Players.empty(),
			"Failed Snapshot Does Not Mutate");
	}

	void Test_WorldEntitySpawnCommandRoundTrip(TEST_RUNNER& testRunner)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		request.strPlacementId = "boss.valtan.character-select.lazy";
		CPacketWriter requestWriter;
		testRunner.Require(
			Write_Message(requestWriter, request),
			"Writer World Entity Spawn Request");
		CPacketReader requestReader{ requestWriter.Get_Buffer() };
		C2S_SPAWN_WORLD_ENTITY decodedRequest{};
		testRunner.Require(
			Read_Message(requestReader, decodedRequest) &&
			decodedRequest.strPlacementId == request.strPlacementId &&
			0u == requestReader.Get_RemainingSize(),
			"World Entity Spawn Request Round Trip");

		C2S_SPAWN_WORLD_ENTITY invalidRequest{};
		CPacketWriter invalidRequestWriter;
		testRunner.Require(
			!Write_Message(invalidRequestWriter, invalidRequest),
			"Reject Empty World Entity Spawn Placement");
		invalidRequest.strPlacementId = "boss/valtan";
		testRunner.Require(
			!Write_Message(invalidRequestWriter, invalidRequest),
			"Reject Invalid World Entity Spawn Placement");
		invalidRequest.strPlacementId.assign(
			MAX_STABLE_NETWORK_ID_BYTES + 1u,
			'a');
		testRunner.Require(
			!Write_Message(invalidRequestWriter, invalidRequest),
			"Reject Oversize World Entity Spawn Placement");

		S2C_WORLD_ENTITY_SPAWN_RESULT result{};
		result.strPlacementId = request.strPlacementId;
		result.eResult = WORLD_ENTITY_SPAWN_RESULT::SPAWNED;
		result.iNetEntityId = 900u;
		CPacketWriter resultWriter;
		testRunner.Require(
			Write_Message(resultWriter, result),
			"Writer World Entity Spawn Result");
		CPacketReader resultReader{ resultWriter.Get_Buffer() };
		S2C_WORLD_ENTITY_SPAWN_RESULT decodedResult{};
		testRunner.Require(
			Read_Message(resultReader, decodedResult) &&
			decodedResult.eResult == WORLD_ENTITY_SPAWN_RESULT::SPAWNED &&
			decodedResult.iNetEntityId == 900u &&
			0u == resultReader.Get_RemainingSize(),
			"World Entity Spawn Result Round Trip");

		result.eResult = WORLD_ENTITY_SPAWN_RESULT::REJECTED;
		testRunner.Require(
			!Write_Message(resultWriter, result),
			"Reject Spawn Rejection With Entity ID");
		result.iNetEntityId = INVALID_NET_ENTITY_ID;
		CPacketWriter rejectionWriter;
		testRunner.Require(
			Write_Message(rejectionWriter, result),
			"Accept Explicit World Entity Spawn Rejection");

		result.eResult = WORLD_ENTITY_SPAWN_RESULT::ACTIVATED;
		CPacketWriter activationWriter;
		testRunner.Require(
			Write_Message(activationWriter, result),
			"Accept Spawn Group Activation Without Entity ID");
		CPacketReader activationReader{ activationWriter.Get_Buffer() };
		S2C_WORLD_ENTITY_SPAWN_RESULT activated{};
		testRunner.Require(
			Read_Message(activationReader, activated) &&
			activated.eResult == WORLD_ENTITY_SPAWN_RESULT::ACTIVATED &&
			activated.iNetEntityId == INVALID_NET_ENTITY_ID &&
			0u == activationReader.Get_RemainingSize(),
			"Spawn Group Activation Result Round Trip");
	}
}

int main()
{
	TEST_RUNNER testRunner{};

	Test_EnterWorldRoundTrip(testRunner);
	Test_PlayableCharacterRoster(testRunner);
	Test_InvalidPayloads(testRunner);

	Test_EnterAcceptedRoundTrip(testRunner);
	Test_InvalidEnterAcceptedPayloads(testRunner);
	Test_EnterRejectedRoundTrip(testRunner);

	Test_F32RoundTrip(testRunner);
	Test_PlayerSpawnedRoundTrip(testRunner);
	Test_InvalidPlayerSpawnedPayloads(testRunner);
	Test_WorldEntitySpawnedRoundTrip(testRunner);
	Test_WorldEntityDespawnedRoundTrip(testRunner);
	Test_PlayerDespawnedRoundTrip(testRunner);
	//Move, Snapshot roundtrip
	Test_MoveRoundTrip(testRunner);
	Test_UseSkillRoundTrip(testRunner);
	Test_ReleaseSkillRoundTrip(testRunner);
	Test_UpdateSkillAimRoundTrip(testRunner);
	Test_RevivePlayerRoundTrip(testRunner);
	Test_CharacterClassChangeRoundTrip(testRunner);
	Test_WorldEntitySpawnCommandRoundTrip(testRunner);
	Test_WorldSnapshotRoundTrip(testRunner);

	Test_StreamFraming(testRunner);

	std::cout
		<< "failures : "
		<< testRunner.iFailureCount
		<< '\n';

	return 0 == testRunner.iFailureCount ? 0 : 1;
}
```

### G00-07-14. `Tools/ValtanFourPlayerHarness/Default/ValtanFourPlayerHarness.vcxproj` full code

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}</ProjectGuid>
    <RootNamespace>ValtanFourPlayerHarness</RootNamespace>
    <ProjectName>ValtanFourPlayerHarness</ProjectName>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)..\Bin\$(Configuration)\</OutDir>
    <IntDir>$(ProjectDir)..\Intermediate\$(Platform)\$(Configuration)\</IntDir>
    <TargetName>ValtanFourPlayerHarness</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ProjectReference Include="..\..\..\Shared\Default\Shared.vcxproj">
      <Project>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</Project>
    </ProjectReference>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\ValtanFourPlayerHarness.cpp" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
</Project>
```

### G00-07-15. `Tools/ValtanFourPlayerHarness/Default/ValtanFourPlayerHarness.vcxproj.filters` full code

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Private">
      <UniqueIdentifier>{D3022BBD-0CF6-471A-BB04-50E5889CC9A8}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\ValtanFourPlayerHarness.cpp">
      <Filter>Private</Filter>
    </ClCompile>
  </ItemGroup>
</Project>
```

### G00-07-16. `Tools/ValtanFourPlayerHarness/Private/ValtanFourPlayerHarness.cpp` full code

```cpp
#include "Network/PacketFrame.h"
#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketStreamParser.h"
#include "Network/PacketWriter.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace
{
	using namespace LostArk::Shared;

	struct HARNESS_OPTIONS final
	{
		std::string strHost = "127.0.0.1";
		std::uint16_t iPort = 7777u;
		std::uint32_t iTimeoutMilliseconds = 10000u;
	};

	bool Parse_Unsigned(
		const std::string_view text,
		std::uint32_t& value)
	{
		std::uint32_t parsed = 0;
		const auto result = std::from_chars(
			text.data(), text.data() + text.size(), parsed);
		if (result.ec != std::errc{} ||
			result.ptr != text.data() + text.size())
		{
			return false;
		}
		value = parsed;
		return true;
	}

	bool Parse_Options(
		const int argumentCount,
		char** arguments,
		HARNESS_OPTIONS& options,
		std::string& error)
	{
		bool hasHost = false;
		bool hasPort = false;
		bool hasTimeout = false;
		for (int index = 1; index < argumentCount; ++index)
		{
			const std::string_view argument(arguments[index]);
			if ("--host" == argument && !hasHost && index + 1 < argumentCount)
			{
				options.strHost = arguments[++index];
				hasHost = true;
				continue;
			}
			if ("--port" == argument && !hasPort && index + 1 < argumentCount)
			{
				std::uint32_t port = 0;
				if (!Parse_Unsigned(arguments[++index], port) ||
					0u == port || port > 65535u)
				{
					error = "--port must be an integer from 1 to 65535";
					return false;
				}
				options.iPort = static_cast<std::uint16_t>(port);
				hasPort = true;
				continue;
			}
			if ("--timeout-ms" == argument && !hasTimeout &&
				index + 1 < argumentCount)
			{
				std::uint32_t timeout = 0;
				if (!Parse_Unsigned(arguments[++index], timeout) ||
					timeout < 1000u || timeout > 30000u)
				{
					error = "--timeout-ms must be an integer from 1000 to 30000";
					return false;
				}
				options.iTimeoutMilliseconds = timeout;
				hasTimeout = true;
				continue;
			}

			error = "Usage: ValtanFourPlayerHarness [--host IPv4] "
				"[--port 1..65535] [--timeout-ms 1000..30000]";
			return false;
		}

		IN_ADDR address{};
		if (options.strHost.empty() ||
			1 != ::InetPtonA(AF_INET, options.strHost.c_str(), &address))
		{
			error = "--host must be one IPv4 address";
			return false;
		}
		return true;
	}

	class CWinsockScope final
	{
	public:
		CWinsockScope()
		{
			WSADATA data{};
			m_isReady = 0 == ::WSAStartup(MAKEWORD(2, 2), &data) &&
				LOBYTE(data.wVersion) == 2 && HIBYTE(data.wVersion) == 2;
		}

		~CWinsockScope()
		{
			if (m_isReady)
				::WSACleanup();
		}

		[[nodiscard]] bool Is_Ready() const
		{
			return m_isReady;
		}

	private:
		bool m_isReady = false;
	};

	class CTestClient final
	{
	public:
		explicit CTestClient(std::string label)
			: m_strLabel(std::move(label))
		{}

		~CTestClient()
		{
			Close();
		}

		CTestClient(const CTestClient&) = delete;
		CTestClient& operator=(const CTestClient&) = delete;

		bool Connect_AndEnter(
			const HARNESS_OPTIONS& options,
			const CHARACTER_CLASS_ID characterClass,
			std::string& error)
		{
			Close();
			m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == m_hSocket)
				return Set_SocketError("socket", error);

			const DWORD socketTimeout = 2000u;
			::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDTIMEO,
				reinterpret_cast<const char*>(&socketTimeout),
				static_cast<int>(sizeof(socketTimeout)));
			::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO,
				reinterpret_cast<const char*>(&socketTimeout),
				static_cast<int>(sizeof(socketTimeout)));

			sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_port = ::htons(options.iPort);
			if (1 != ::InetPtonA(
				AF_INET, options.strHost.c_str(), &address.sin_addr))
			{
				error = m_strLabel + ": invalid IPv4 address";
				Close();
				return false;
			}
			if (SOCKET_ERROR == ::connect(
				m_hSocket,
				reinterpret_cast<const sockaddr*>(&address),
				static_cast<int>(sizeof(address))))
			{
				return Set_SocketError("connect", error);
			}

			C2S_ENTER_WORLD enter{};
			enter.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
			enter.eWorldId = WORLD_ID::VALTAN_ARENA;
			enter.eCharacterClass = characterClass;
			enter.strNickName = m_strLabel;
			CPacketWriter payloadWriter;
			std::vector<std::uint8_t> frameBytes;
			if (!Write_Message(payloadWriter, enter) ||
				!Build_Packet_Frame(
					PACKET_TYPE::C2S_ENTER_WORLD,
					payloadWriter.Get_Buffer(),
					frameBytes) ||
				!Send_All(frameBytes, error))
			{
				Close();
				return false;
			}
			return true;
		}

		bool Poll(std::string& error)
		{
			if (INVALID_SOCKET == m_hSocket)
			{
				error = m_strLabel + ": socket is not open";
				return false;
			}

			fd_set readable{};
			FD_ZERO(&readable);
			FD_SET(m_hSocket, &readable);
			timeval noWait{};
			const int selected = ::select(0, &readable, nullptr, nullptr, &noWait);
			if (SOCKET_ERROR == selected)
				return Set_SocketError("select", error);
			if (0 == selected)
				return true;

			std::array<std::uint8_t, 8192> receiveBuffer{};
			const int received = ::recv(
				m_hSocket,
				reinterpret_cast<char*>(receiveBuffer.data()),
				static_cast<int>(receiveBuffer.size()),
				0);
			if (0 == received)
			{
				if (m_hasEnterRejected)
					return true;
				error = m_strLabel + ": server closed the connection";
				return false;
			}
			if (SOCKET_ERROR == received)
				return Set_SocketError("recv", error);

			if (!m_StreamParser.Append(std::span<const std::uint8_t>(
				receiveBuffer.data(), static_cast<std::size_t>(received))))
			{
				error = m_strLabel + ": receive buffer exceeded protocol bound";
				return false;
			}
			for (;;)
			{
				PACKET_FRAME frame{};
				const PACKET_PARSE_RESULT result = m_StreamParser.Try_Pop(frame);
				if (PACKET_PARSE_RESULT::NEED_MORE_DATA == result)
					break;
				if (PACKET_PARSE_RESULT::INVALID_FRAME == result)
				{
					error = m_strLabel + ": received an invalid packet frame";
					return false;
				}
				if (!Handle_Frame(frame, error))
					return false;
			}
			return true;
		}

		void Close()
		{
			if (INVALID_SOCKET == m_hSocket)
				return;
			::shutdown(m_hSocket, SD_BOTH);
			::closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}

		[[nodiscard]] bool Is_Accepted() const
		{
			return m_hasEnterAccepted;
		}

		[[nodiscard]] bool Is_RoomFullRejected() const
		{
			return m_hasEnterRejected &&
				WORLD_ID::VALTAN_ARENA == m_EnterRejected.eWorldId &&
				ENTER_WORLD_REJECTION_REASON::ROOM_FULL ==
					m_EnterRejected.eReason;
		}

		[[nodiscard]] bool Has_PlayerCount(const std::size_t expected) const
		{
			return m_hasWorldSnapshot && m_iSnapshotPlayerCount == expected &&
				m_iSnapshotUniqueEntityCount == expected;
		}

		[[nodiscard]] PLAYER_ID Get_PlayerId() const
		{
			return m_EnterAccepted.iPlayerId;
		}

		[[nodiscard]] NET_ENTITY_ID Get_NetEntityId() const
		{
			return m_EnterAccepted.iNetEntityId;
		}

		[[nodiscard]] const std::string& Get_Label() const
		{
			return m_strLabel;
		}

	private:
		bool Send_All(
			const std::span<const std::uint8_t> bytes,
			std::string& error)
		{
			std::size_t sent = 0;
			while (sent < bytes.size())
			{
				const int result = ::send(
					m_hSocket,
					reinterpret_cast<const char*>(bytes.data() + sent),
					static_cast<int>(bytes.size() - sent),
					0);
				if (SOCKET_ERROR == result)
					return Set_SocketError("send", error);
				if (0 == result)
				{
					error = m_strLabel + ": send returned zero bytes";
					return false;
				}
				sent += static_cast<std::size_t>(result);
			}
			return true;
		}

		bool Handle_Frame(
			const PACKET_FRAME& frame,
			std::string& error)
		{
			CPacketReader reader{ frame.Payload };
			if (PACKET_TYPE::S2C_ENTER_ACCEPTED == frame.ePacketType)
			{
				S2C_ENTER_ACCEPTED accepted{};
				if (m_hasEnterAccepted || !Read_Message(reader, accepted) ||
					0u != reader.Get_RemainingSize() ||
					WORLD_ID::VALTAN_ARENA != accepted.eWorldId)
				{
					error = m_strLabel + ": invalid or duplicate enter acceptance";
					return false;
				}
				m_EnterAccepted = accepted;
				m_hasEnterAccepted = true;
				return true;
			}
			if (PACKET_TYPE::S2C_ENTER_REJECTED == frame.ePacketType)
			{
				S2C_ENTER_REJECTED rejected{};
				if (m_hasEnterRejected || !Read_Message(reader, rejected) ||
					0u != reader.Get_RemainingSize())
				{
					error = m_strLabel + ": invalid or duplicate enter rejection";
					return false;
				}
				m_EnterRejected = rejected;
				m_hasEnterRejected = true;
				return true;
			}
			if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == frame.ePacketType)
			{
				S2C_WORLD_SNAPSHOT snapshot{};
				if (!Read_Message(reader, snapshot) ||
					0u != reader.Get_RemainingSize() ||
					WORLD_ID::VALTAN_ARENA != snapshot.eWorldId)
				{
					error = m_strLabel + ": invalid Valtan world snapshot";
					return false;
				}
				std::set<NET_ENTITY_ID> entityIds;
				for (const PLAYER_SNAPSHOT& player : snapshot.Players)
					entityIds.insert(player.iNetEntityId);
				m_iSnapshotPlayerCount = snapshot.Players.size();
				m_iSnapshotUniqueEntityCount = entityIds.size();
				m_hasWorldSnapshot = true;
				return true;
			}

			// Spawn/despawn and boss packets are intentionally consumed by the
			// production stream parser but are not cohort admission assertions.
			return true;
		}

		bool Set_SocketError(
			const char* operation,
			std::string& error)
		{
			const int code = ::WSAGetLastError();
			error = m_strLabel + ": " + operation + " failed, WSA=" +
				std::to_string(code);
			Close();
			return false;
		}

	private:
		std::string m_strLabel;
		SOCKET m_hSocket = INVALID_SOCKET;
		CPacketStreamParser m_StreamParser;
		S2C_ENTER_ACCEPTED m_EnterAccepted{};
		S2C_ENTER_REJECTED m_EnterRejected{};
		std::size_t m_iSnapshotPlayerCount = 0;
		std::size_t m_iSnapshotUniqueEntityCount = 0;
		bool m_hasEnterAccepted = false;
		bool m_hasEnterRejected = false;
		bool m_hasWorldSnapshot = false;
	};

	template<typename PREDICATE>
	bool Pump_Until(
		const std::span<CTestClient*> clients,
		const std::chrono::milliseconds timeout,
		PREDICATE predicate,
		const std::string_view stage,
		std::string& error)
	{
		const auto deadline = std::chrono::steady_clock::now() + timeout;
		while (std::chrono::steady_clock::now() < deadline)
		{
			if (predicate())
				return true;
			for (CTestClient* client : clients)
			{
				if (nullptr != client && !client->Poll(error))
				{
					error = std::string(stage) + ": " + error;
					return false;
				}
			}
			if (predicate())
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		error = std::string(stage) + ": timed out after " +
			std::to_string(timeout.count()) + " ms";
		return false;
	}

	bool All_AcceptedWithSnapshotCount(
		const std::span<CTestClient*> clients,
		const std::size_t expectedCount)
	{
		std::set<PLAYER_ID> playerIds;
		std::set<NET_ENTITY_ID> entityIds;
		for (const CTestClient* client : clients)
		{
			if (nullptr == client || !client->Is_Accepted() ||
				!client->Has_PlayerCount(expectedCount))
			{
				return false;
			}
			playerIds.insert(client->Get_PlayerId());
			entityIds.insert(client->Get_NetEntityId());
		}
		return playerIds.size() == clients.size() &&
			entityIds.size() == clients.size();
	}

	std::unique_ptr<CTestClient> Connect_Client(
		const HARNESS_OPTIONS& options,
		const std::string& label,
		const CHARACTER_CLASS_ID characterClass,
		std::string& error)
	{
		auto client = std::make_unique<CTestClient>(label);
		if (!client->Connect_AndEnter(options, characterClass, error))
			return {};
		return client;
	}

	bool Run_FourPlayerCohort(
		const HARNESS_OPTIONS& options,
		std::string& error)
	{
		const auto timeout =
			std::chrono::milliseconds(options.iTimeoutMilliseconds);
		constexpr std::array CHARACTER_CLASSES
		{
			CHARACTER_CLASS_ID::LANCE_MASTER,
			CHARACTER_CLASS_ID::GUNSLINGER,
			CHARACTER_CLASS_ID::SLAYER,
			CHARACTER_CLASS_ID::ARTIST,
			CHARACTER_CLASS_ID::DIMENSIONMASTER,
			CHARACTER_CLASS_ID::WARLORD
		};

		std::vector<std::unique_ptr<CTestClient>> firstGeneration;
		std::vector<CTestClient*> firstActive;
		for (std::size_t index = 0; index < 4u; ++index)
		{
			auto client = Connect_Client(
				options,
				"G1-Player-" + std::to_string(index + 1u),
				CHARACTER_CLASSES[index],
				error);
			if (nullptr == client)
				return false;
			firstActive.push_back(client.get());
			firstGeneration.push_back(std::move(client));
		}
		if (!Pump_Until(firstActive, timeout,
			[&firstActive]()
			{
				return All_AcceptedWithSnapshotCount(firstActive, 4u);
			},
			"first generation reaches four-player snapshot", error))
		{
			return false;
		}
		std::cout << "[PASS] four initial clients accepted and converged at 4/4\n";

		auto overflow = Connect_Client(
			options,
			"G1-Overflow-5",
			CHARACTER_CLASSES[4],
			error);
		if (nullptr == overflow)
			return false;
		std::array overflowSpan{ overflow.get() };
		if (!Pump_Until(overflowSpan, timeout,
			[&overflow]() { return overflow->Is_RoomFullRejected(); },
			"fifth client receives typed ROOM_FULL", error))
		{
			return false;
		}
		std::cout << "[PASS] fifth client received S2C_ENTER_REJECTED ROOM_FULL\n";
		overflow->Close();

		firstGeneration[1]->Close();
		firstActive.erase(firstActive.begin() + 1);
		if (!Pump_Until(firstActive, timeout,
			[&firstActive]()
			{
				return std::all_of(
					firstActive.begin(), firstActive.end(),
					[](const CTestClient* client)
					{
						return nullptr != client && client->Has_PlayerCount(3u);
					});
			},
			"remaining clients observe disconnect", error))
		{
			return false;
		}

		auto replacement = Connect_Client(
			options,
			"G1-Replacement-5",
			CHARACTER_CLASSES[5],
			error);
		if (nullptr == replacement)
			return false;
		firstActive.push_back(replacement.get());
		if (!Pump_Until(firstActive, timeout,
			[&firstActive]()
			{
				return All_AcceptedWithSnapshotCount(firstActive, 4u);
			},
			"replacement restores four-player snapshot", error))
		{
			return false;
		}
		std::cout << "[PASS] disconnect and replacement reconverged at 4/4\n";

		for (CTestClient* client : firstActive)
			client->Close();
		replacement->Close();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::vector<std::unique_ptr<CTestClient>> secondGeneration;
		std::vector<CTestClient*> secondActive;
		for (std::size_t index = 0; index < 4u; ++index)
		{
			auto client = Connect_Client(
				options,
				"G2-Player-" + std::to_string(index + 1u),
				CHARACTER_CLASSES[(index + 2u) % CHARACTER_CLASSES.size()],
				error);
			if (nullptr == client)
				return false;
			secondActive.push_back(client.get());
			secondGeneration.push_back(std::move(client));
		}
		if (!Pump_Until(secondActive, timeout,
			[&secondActive]()
			{
				return All_AcceptedWithSnapshotCount(secondActive, 4u);
			},
			"second generation reaches four-player snapshot", error))
		{
			return false;
		}
		std::cout << "[PASS] empty-room reset admitted a second 4/4 generation\n";
		for (CTestClient* client : secondActive)
			client->Close();
		return true;
	}
}

int main(const int argumentCount, char** arguments)
{
	HARNESS_OPTIONS options{};
	std::string error;
	if (!Parse_Options(argumentCount, arguments, options, error))
	{
		std::cerr << "[FAILURE] " << error << '\n';
		return 2;
	}

	const CWinsockScope winsock;
	if (!winsock.Is_Ready())
	{
		std::cerr << "[FAILURE] WSAStartup 2.2 failed\n";
		return 1;
	}

	if (!Run_FourPlayerCohort(options, error))
	{
		std::cerr << "[FAILURE] " << error << '\n';
		return 1;
	}

	std::cout << "failures : 0\n";
	return 0;
}
```

### G00-07-17. `Tools/Network/Run-ValtanFourPlayerHarness.ps1` full code

```powershell
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateRange(1000, 30000)]
    [int]$HarnessTimeoutMilliseconds = 10000,
    [ValidateRange(1000, 15000)]
    [int]$ServerStartupTimeoutMilliseconds = 10000
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$harnessExe = Join-Path $repoRoot `
    "Tools\ValtanFourPlayerHarness\Bin\$Configuration\ValtanFourPlayerHarness.exe"
$logRoot = [IO.Path]::GetFullPath(
    (Join-Path $repoRoot ".codex_tmp\valtan-four-player-harness\$Configuration"))
$generatedRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot '.codex_tmp'))

if (-not $logRoot.StartsWith(
        $generatedRoot.TrimEnd('\') + '\',
        [StringComparison]::OrdinalIgnoreCase)) {
    throw "Harness log path escaped the generated root: $logRoot"
}
if (-not (Test-Path -LiteralPath $serverExe -PathType Leaf)) {
    throw "Server executable is missing: $serverExe"
}
if (-not (Test-Path -LiteralPath $harnessExe -PathType Leaf)) {
    throw "Valtan four-player harness executable is missing: $harnessExe"
}

function Get-HarnessPortListeners {
    return @(Get-NetTCPConnection -LocalPort 7777 -State Listen `
        -ErrorAction SilentlyContinue)
}

function Write-CapturedLog {
    param(
        [string]$Path,
        [switch]$AsError
    )
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return
    }
    foreach ($line in Get-Content -LiteralPath $Path) {
        if ($AsError) {
            [Console]::Error.WriteLine($line)
        }
        else {
            [Console]::Out.WriteLine($line)
        }
    }
}

$preexisting = Get-HarnessPortListeners
if ($preexisting.Count -ne 0) {
    $owners = @($preexisting | ForEach-Object { $_.OwningProcess } |
        Sort-Object -Unique)
    throw "TCP 7777 is already listening; refusing to touch PID(s): $($owners -join ', ')"
}

New-Item -ItemType Directory -Path $logRoot -Force | Out-Null
$runToken = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfff')
$serverStdout = Join-Path $logRoot "$runToken.server.stdout.log"
$serverStderr = Join-Path $logRoot "$runToken.server.stderr.log"
$harnessStdout = Join-Path $logRoot "$runToken.harness.stdout.log"
$harnessStderr = Join-Path $logRoot "$runToken.harness.stderr.log"
$serverProcess = $null
$harnessProcess = $null
$serverLifetimeMilliseconds = [Math]::Min(
    60000,
    [Math]::Max(
        15000,
        $ServerStartupTimeoutMilliseconds + $HarnessTimeoutMilliseconds + 10000))

try {
    $serverProcess = Start-Process `
        -FilePath $serverExe `
        -ArgumentList @(
            '--bind-address', '127.0.0.1',
            '--smoke-timeout-ms', $serverLifetimeMilliseconds) `
        -WorkingDirectory (Split-Path -Parent $serverExe) `
        -WindowStyle Hidden `
        -RedirectStandardOutput $serverStdout `
        -RedirectStandardError $serverStderr `
        -PassThru

    $startupDeadline = [DateTime]::UtcNow.AddMilliseconds(
        $ServerStartupTimeoutMilliseconds)
    $listenerReady = $false
    while ([DateTime]::UtcNow -lt $startupDeadline) {
        $serverProcess.Refresh()
        if ($serverProcess.HasExited) {
            throw "Owned Server exited during startup with code $($serverProcess.ExitCode)."
        }

        $listeners = Get-HarnessPortListeners
        if ($listeners.Count -ne 0) {
            $foreign = @($listeners | Where-Object {
                $_.OwningProcess -ne $serverProcess.Id
            })
            if ($foreign.Count -ne 0) {
                throw 'TCP 7777 ownership changed during harness startup.'
            }
            $owned = @($listeners | Where-Object {
                $_.OwningProcess -eq $serverProcess.Id -and
                $_.LocalAddress -eq '127.0.0.1'
            })
            if ($owned.Count -eq 1) {
                $listenerReady = $true
                break
            }
        }
        Start-Sleep -Milliseconds 25
    }
    if (-not $listenerReady) {
        throw "Owned Server did not listen on 127.0.0.1:7777 within $ServerStartupTimeoutMilliseconds ms."
    }

    $harnessStartInfo = [Diagnostics.ProcessStartInfo]::new()
    $harnessStartInfo.FileName = $harnessExe
    $harnessStartInfo.WorkingDirectory = Split-Path -Parent $harnessExe
    $harnessStartInfo.UseShellExecute = $false
    $harnessStartInfo.CreateNoWindow = $true
    $harnessStartInfo.RedirectStandardOutput = $true
    $harnessStartInfo.RedirectStandardError = $true
    $harnessStartInfo.Arguments =
        "--host 127.0.0.1 --port 7777 --timeout-ms $HarnessTimeoutMilliseconds"
    $harnessProcess = [Diagnostics.Process]::new()
    $harnessProcess.StartInfo = $harnessStartInfo
    if (-not $harnessProcess.Start()) {
        throw 'Valtan four-player harness failed to start.'
    }
    $harnessStdoutTask = $harnessProcess.StandardOutput.ReadToEndAsync()
    $harnessStderrTask = $harnessProcess.StandardError.ReadToEndAsync()
    if (-not $harnessProcess.WaitForExit($HarnessTimeoutMilliseconds + 5000)) {
        $harnessProcess.Kill($true)
        throw 'Valtan four-player harness exceeded its external timeout.'
    }
    $harnessProcess.WaitForExit()
    $harnessExitCode = $harnessProcess.ExitCode
    Set-Content -LiteralPath $harnessStdout `
        -Value $harnessStdoutTask.GetAwaiter().GetResult() `
        -Encoding UTF8
    Set-Content -LiteralPath $harnessStderr `
        -Value $harnessStderrTask.GetAwaiter().GetResult() `
        -Encoding UTF8
    Write-CapturedLog -Path $harnessStdout
    Write-CapturedLog -Path $harnessStderr -AsError
    if (0 -ne $harnessExitCode) {
        throw "Valtan four-player harness failed with code $harnessExitCode."
    }

    Write-Host "Valtan four-player live harness passed: $Configuration"
}
catch {
    Write-CapturedLog -Path $harnessStdout
    Write-CapturedLog -Path $harnessStderr -AsError
    Write-CapturedLog -Path $serverStdout
    Write-CapturedLog -Path $serverStderr -AsError
    throw
}
finally {
    if ($null -ne $harnessProcess) {
        $harnessProcess.Refresh()
        if (-not $harnessProcess.HasExited) {
            $harnessProcess.Kill($true)
            $null = $harnessProcess.WaitForExit(5000)
        }
        $harnessProcess.Dispose()
    }
    if ($null -ne $serverProcess) {
        $serverProcess.Refresh()
        if (-not $serverProcess.HasExited) {
            Stop-Process -Id $serverProcess.Id -ErrorAction SilentlyContinue
            $null = $serverProcess.WaitForExit(5000)
        }
    }

    $cleanupDeadline = [DateTime]::UtcNow.AddSeconds(5)
    do {
        $residue = Get-HarnessPortListeners
        if ($residue.Count -eq 0) {
            break
        }
        Start-Sleep -Milliseconds 50
    } while ([DateTime]::UtcNow -lt $cleanupDeadline)

    if ($residue.Count -ne 0) {
        $owners = @($residue | ForEach-Object { $_.OwningProcess } |
            Sort-Object -Unique)
        throw "TCP 7777 listener residue remains after owned-process cleanup: $($owners -join ', ')"
    }
}
```

### G00-07-18. `Tools/WorldPipeline/Publish-WorldGameplay.ps1` full code

```powershell
[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles/World',
	[ValidateRange(0, 12)]
	[int]$FailureAfterPromote = 0
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'

function Read-ProjectJson {
    param([string]$RelativePath)
    $path = [IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
    if (-not [IO.File]::Exists($path)) {
        throw "Required JSON document is missing: $RelativePath"
    }
	return Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Assert-ExactProperties {
    param(
        [object]$Value,
        [string[]]$Expected,
        [string]$Context
    )
    $actual = @($Value.PSObject.Properties.Name | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    if (($actual -join "`n") -ne ($expectedSorted -join "`n")) {
        throw "$Context has missing or unknown fields. expected=[$($expectedSorted -join ',')] actual=[$($actual -join ',')]"
    }
}

function Assert-StableId {
    param([string]$Value, [string]$Context, [switch]$AllowEmpty)
    if ($AllowEmpty -and [string]::IsNullOrEmpty($Value)) {
        return
    }
    if ($Value -notmatch $stableIdPattern) {
        throw "$Context is not a stable ID: '$Value'"
    }
}

function Assert-JsonInteger {
    param([object]$Value, [string]$Context, [long]$Minimum = 0, [long]$Maximum = [long]::MaxValue)
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64])) {
        throw "$Context must be a JSON integer."
    }
    $number = [long]$Value
    if ($number -lt $Minimum -or $number -gt $Maximum) {
        throw "$Context integer is out of range: $number"
    }
}

function Assert-JsonNumber {
    param([object]$Value, [string]$Context)
    if (($Value -isnot [int]) -and ($Value -isnot [long]) -and
        ($Value -isnot [uint32]) -and ($Value -isnot [uint64]) -and
        ($Value -isnot [double]) -and ($Value -isnot [decimal])) {
        throw "$Context must be a JSON number."
    }
    $number = [double]$Value
    if ([double]::IsNaN($number) -or [double]::IsInfinity($number)) {
        throw "$Context must be finite."
    }
}

function Assert-JsonString {
    param([object]$Value, [string]$Context, [switch]$AllowNull)
    if ($AllowNull -and $null -eq $Value) { return }
    if ($Value -isnot [string]) { throw "$Context must be a JSON string." }
}

function Get-ActorIds {
    $characterCatalog = Read-ProjectJson 'Data/Actors/CharacterCatalog.json'
    $bossCatalog = Read-ProjectJson 'Data/Actors/BossCatalog.json'
    $npcCatalog = Read-ProjectJson 'Data/Actors/NpcCatalog.json'
	$monsterCatalog = Read-ProjectJson 'Data/Actors/MonsterCatalog.json'

    $idsByKind = @{
        playerSpawn = @($characterCatalog.characters |
            Where-Object runtimeStatus -eq 'supported' |
            ForEach-Object archetypeId)
        boss = @($bossCatalog.bosses | ForEach-Object archetypeId)
        npc = @($npcCatalog.npcs |
			Where-Object runtimeStatus -eq 'supported' |
			ForEach-Object archetypeId)
		monster = @($monsterCatalog.monsters |
			Where-Object runtimeStatus -eq 'supported' |
			ForEach-Object archetypeId)
    }
    return $idsByKind
}

function Format-InvariantFloat {
    param([double]$Value)
    if ([double]::IsNaN($Value) -or [double]::IsInfinity($Value) -or [math]::Abs($Value) -gt 100000.0) {
        throw "World coordinate is invalid: $Value"
    }
    return $Value.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
}

function Get-EncounterProfiles {
    $documents = @(
        (Read-ProjectJson 'Data/Encounters/Valtan/ValtanEncounter.json')
    )
    $profiles = @{}
    foreach ($document in $documents) {
        Assert-ExactProperties $document @(
            'schema','formatVersion','encounterId','bossArchetypeId',
            'authority','fixedTickHz','states','patterns') 'encounter profile'
		Assert-JsonInteger $document.formatVersion "$($document.encounterId) formatVersion" 3 3
		Assert-JsonInteger $document.fixedTickHz "$($document.encounterId) fixedTickHz" 30 30
        Assert-StableId $document.encounterId 'encounterId'
        Assert-StableId $document.bossArchetypeId 'bossArchetypeId'
        if ($document.schema -ne 'lostark.encounter-profile' -or
			$document.formatVersion -ne 3 -or
            $document.authority -ne 'server' -or
            $document.fixedTickHz -ne 30) {
            throw "Encounter header is invalid: $($document.encounterId)"
        }
        if ($profiles.ContainsKey([string]$document.encounterId)) {
            throw "Duplicate encounter ID: $($document.encounterId)"
        }

        $stateIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($state in @($document.states)) {
            Assert-ExactProperties $state @('id','actionId','next') "$($document.encounterId) state"
			Assert-JsonString $state.id "$($document.encounterId) state id"
			Assert-JsonString $state.actionId "$($document.encounterId) state actionId"
			Assert-JsonString $state.next "$($document.encounterId) state next" -AllowNull
            Assert-StableId $state.id "$($document.encounterId) state id"
            Assert-StableId $state.actionId "$($document.encounterId) state actionId"
            Assert-StableId $state.next "$($document.encounterId) state next" -AllowEmpty
            if (-not $stateIds.Add([string]$state.id)) {
                throw "Duplicate encounter state ID: $($state.id)"
            }
        }
        if ($stateIds.Count -eq 0) {
            throw "Encounter requires at least one state: $($document.encounterId)"
        }

        $patterns = @($document.patterns)
        if ($patterns.Count -eq 0) {
            throw "Encounter requires at least one pattern: $($document.encounterId)"
        }
		$patternIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		foreach ($pattern in $patterns) {
			Assert-ExactProperties $pattern @(
				'patternId','displayName','actionId','sourceActionIds','selectionMode',
				'minimumHealthBar','maximumHealthBar','triggerHealthBar','triggerOrder',
				'selectionWeight','maximumConsecutiveUses','minimumRange','maximumRange',
				'stages') "$($document.encounterId) pattern"
			Assert-JsonNumber $pattern.minimumRange "$($document.encounterId) minimumRange"
			Assert-JsonNumber $pattern.maximumRange "$($document.encounterId) maximumRange"
			Assert-StableId $pattern.patternId "$($document.encounterId) patternId"
			Assert-StableId $pattern.actionId "$($document.encounterId) actionId"
			if (-not $patternIds.Add([string]$pattern.patternId)) {
				throw "Duplicate encounter pattern ID: $($pattern.patternId)"
			}
			if ([double]$pattern.minimumRange -lt 0.0 -or
				[double]$pattern.maximumRange -le [double]$pattern.minimumRange -or
				@($pattern.sourceActionIds).Count -eq 0 -or
				@($pattern.stages).Count -eq 0) {
				throw "Encounter pattern timing or range is invalid: $($pattern.patternId)"
			}
			foreach ($stage in @($pattern.stages)) {
				Assert-StableId $stage.stageId "$($document.encounterId) stageId"
				Assert-StableId $stage.actionId "$($document.encounterId) stage actionId"
				Assert-JsonInteger $stage.durationMs "$($document.encounterId) stage durationMs" 1 ([uint32]::MaxValue)
			}
		}
        $profiles[[string]$document.encounterId] = $document
    }
    return $profiles
}

function Get-MonsterProfiles {
    $document = Read-ProjectJson 'Data/Balance/MonsterProfiles.json'
    Assert-ExactProperties $document @('schema','formatVersion','basis','profiles') 'monster profiles'
    if ($document.schema -ne 'lostark.monster-profiles' -or
        $document.formatVersion -ne 1 -or $document.basis -ne 'PROJECT_TUNED') {
        throw 'Monster profile header is invalid.'
    }
    $profiles = @{}
    foreach ($profile in @($document.profiles)) {
        Assert-ExactProperties $profile @(
            'archetypeId','maxHp','attackPower','defense','collisionRadius',
            'engageRange','moveSpeed','attackRange','attackWindupMs',
            'attackActiveMs','attackRecoveryMs','deadDespawnMs') 'monster profile'
        Assert-StableId $profile.archetypeId 'monster profile archetypeId'
        Assert-JsonInteger $profile.maxHp "$($profile.archetypeId) maxHp" 1 2000000000
        Assert-JsonInteger $profile.attackPower "$($profile.archetypeId) attackPower" 1 2000000000
        Assert-JsonInteger $profile.defense "$($profile.archetypeId) defense" 0 2000000000
        foreach ($field in @('collisionRadius','engageRange','moveSpeed','attackRange')) {
            Assert-JsonNumber $profile.$field "$($profile.archetypeId) $field"
            if ([double]$profile.$field -le 0.0 -or [double]$profile.$field -gt 1000.0) {
                throw "Monster profile $field is out of range: $($profile.archetypeId)"
            }
        }
        foreach ($field in @('attackWindupMs','attackActiveMs','attackRecoveryMs','deadDespawnMs')) {
            Assert-JsonInteger $profile.$field "$($profile.archetypeId) $field" 1 600000
        }
        if ($profiles.ContainsKey([string]$profile.archetypeId)) {
            throw "Duplicate monster profile: $($profile.archetypeId)"
        }
        $profiles[[string]$profile.archetypeId] = $profile
    }
    return $profiles
}

function Convert-SpawnGroupsDocument {
    param(
        [string]$AreaId,
        [string]$WorldId,
        [hashtable]$ActorIds,
        [hashtable]$MonsterProfiles
    )

    $relativePath = "Data/Worlds/$AreaId/SpawnGroups.world.json"
    $absolutePath = [IO.Path]::GetFullPath((Join-Path $repoRoot $relativePath))
    if (-not [IO.File]::Exists($absolutePath)) {
        return [ordered]@{
            WorldId = $WorldId; AreaId = $AreaId; Revision = 1
            Lines = [Collections.Generic.List[string]]::new()
            GroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
            Count = 0; IsPresent = $false
        }
    }
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @(
        'schema','formatVersion','areaId','revision','anchors','spawnGroups') $relativePath
    if ($document.schema -ne 'lostark.world-spawn-groups' -or
        $document.formatVersion -ne 1 -or $document.areaId -ne $AreaId) {
        throw "Spawn group header is invalid: $relativePath"
    }
    Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if (@($document.anchors).Count -gt 128 -or @($document.spawnGroups).Count -gt 32) {
        throw "Spawn group document exceeds its limits: $relativePath"
    }

    $anchorIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $anchorRows = [Collections.Generic.List[string]]::new()
    foreach ($anchor in @($document.anchors)) {
        Assert-ExactProperties $anchor @('anchorId','position','yawDegrees') "$relativePath anchor"
        Assert-StableId $anchor.anchorId "$relativePath anchorId"
        if (-not $anchorIds.Add([string]$anchor.anchorId)) {
            throw "Duplicate spawn anchor ID: $($anchor.anchorId)"
        }
        if (@($anchor.position).Count -ne 3) { throw "Spawn anchor requires three coordinates: $($anchor.anchorId)" }
        for ($i = 0; $i -lt 3; $i++) { Assert-JsonNumber $anchor.position[$i] "$relativePath anchor position[$i]" }
        Assert-JsonNumber $anchor.yawDegrees "$relativePath anchor yawDegrees"
        $anchorRows.Add((@('ANCHOR',[string]$anchor.anchorId,
            (Format-InvariantFloat $anchor.position[0]),
            (Format-InvariantFloat $anchor.position[1]),
            (Format-InvariantFloat $anchor.position[2]),
            (Format-InvariantFloat $anchor.yawDegrees)) -join "`t"))
    }

    $groupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.spawnGroups)) {
        Assert-StableId $group.spawnGroupId "$relativePath spawnGroupId"
        if (-not $groupIds.Add([string]$group.spawnGroupId)) {
            throw "Duplicate spawn group ID: $($group.spawnGroupId)"
        }
    }
    $groupRows = [Collections.Generic.List[string]]::new()
    $usedArchetypes = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($group in @($document.spawnGroups)) {
        Assert-ExactProperties $group @(
            'spawnGroupId','requiredCompletedGroupId','maxAlive','repeatPolicy',
            'completionPolicy','waves') "$relativePath group"
        Assert-JsonString $group.requiredCompletedGroupId "$relativePath prerequisite" -AllowNull
        if ($null -ne $group.requiredCompletedGroupId) {
            Assert-StableId $group.requiredCompletedGroupId "$relativePath prerequisite"
            if (-not $groupIds.Contains([string]$group.requiredCompletedGroupId)) {
                throw "Unknown spawn group prerequisite: $($group.requiredCompletedGroupId)"
            }
        }
        Assert-JsonInteger $group.maxAlive "$relativePath maxAlive" 1 64
        if ($group.repeatPolicy -ne 'ONCE' -or $group.completionPolicy -ne 'ALL_WAVES_CLEARED') {
            throw "Unsupported spawn group policy: $($group.spawnGroupId)"
        }
        $waves = @($group.waves)
        if ($waves.Count -lt 1 -or $waves.Count -gt 16) { throw "Spawn group wave count is invalid: $($group.spawnGroupId)" }
        $prerequisite = if ($null -eq $group.requiredCompletedGroupId) { '-' } else { [string]$group.requiredCompletedGroupId }
        $groupRows.Add((@('GROUP',[string]$group.spawnGroupId,$prerequisite,[string][uint32]$group.maxAlive,[string]$waves.Count) -join "`t"))
        $waveIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $totalCount = 0
        for ($waveIndex = 0; $waveIndex -lt $waves.Count; $waveIndex++) {
            $wave = $waves[$waveIndex]
            Assert-ExactProperties $wave @('waveId','startDelayMs','nextWavePolicy','entries') "$relativePath wave"
            Assert-StableId $wave.waveId "$relativePath waveId"
            if (-not $waveIds.Add([string]$wave.waveId)) { throw "Duplicate wave ID in group $($group.spawnGroupId): $($wave.waveId)" }
            Assert-JsonInteger $wave.startDelayMs "$relativePath startDelayMs" 0 600000
            if ($wave.nextWavePolicy -ne 'ALL_DEAD') { throw "Unsupported next wave policy: $($wave.waveId)" }
            $entries = @($wave.entries)
            if ($entries.Count -lt 1 -or $entries.Count -gt 16) { throw "Spawn wave entry count is invalid: $($wave.waveId)" }
            $groupRows.Add((@('WAVE',[string]$group.spawnGroupId,[string]$wave.waveId,
                [string]$waveIndex,[string][uint32]$wave.startDelayMs,[string]$entries.Count) -join "`t"))
            for ($entryIndex = 0; $entryIndex -lt $entries.Count; $entryIndex++) {
                $entry = $entries[$entryIndex]
                Assert-ExactProperties $entry @('archetypeId','count','anchorId','initialDelayMs','spawnIntervalMs') "$relativePath entry"
                Assert-StableId $entry.archetypeId "$relativePath entry archetypeId"
                Assert-StableId $entry.anchorId "$relativePath entry anchorId"
                if ($entry.archetypeId -notin @($ActorIds.monster) -or -not $MonsterProfiles.ContainsKey([string]$entry.archetypeId)) {
                    throw "Spawn entry references an unsupported monster archetype: $($entry.archetypeId)"
                }
                if (-not $anchorIds.Contains([string]$entry.anchorId)) { throw "Spawn entry references an unknown anchor: $($entry.anchorId)" }
                Assert-JsonInteger $entry.count "$relativePath entry count" 1 1000
                Assert-JsonInteger $entry.initialDelayMs "$relativePath initialDelayMs" 0 600000
                Assert-JsonInteger $entry.spawnIntervalMs "$relativePath spawnIntervalMs" 0 600000
                $totalCount += [uint32]$entry.count
                if ($totalCount -gt 1000) { throw "Spawn group exceeds total spawn limit: $($group.spawnGroupId)" }
                [void]$usedArchetypes.Add([string]$entry.archetypeId)
                $groupRows.Add((@('ENTRY',[string]$group.spawnGroupId,[string]$wave.waveId,
                    [string]$entryIndex,[string]$entry.archetypeId,[string][uint32]$entry.count,
                    [string]$entry.anchorId,[string][uint32]$entry.initialDelayMs,
                    [string][uint32]$entry.spawnIntervalMs) -join "`t"))
            }
        }
    }

    foreach ($group in @($document.spawnGroups)) {
        $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        $cursor = $group
        while ($null -ne $cursor.requiredCompletedGroupId) {
            if (-not $seen.Add([string]$cursor.spawnGroupId)) { throw "Spawn group prerequisite cycle: $($group.spawnGroupId)" }
            $nextId = [string]$cursor.requiredCompletedGroupId
            $cursor = @($document.spawnGroups | Where-Object spawnGroupId -eq $nextId)[0]
        }
    }

    $profileRows = [Collections.Generic.List[string]]::new()
    foreach ($archetypeId in @($usedArchetypes | Sort-Object)) {
        $profile = $MonsterProfiles[$archetypeId]
        $profileRows.Add((@('PROFILE',$archetypeId,[string][uint32]$profile.maxHp,
            [string][uint32]$profile.attackPower,[string][uint32]$profile.defense,
            (Format-InvariantFloat $profile.collisionRadius),
            (Format-InvariantFloat $profile.engageRange),
            (Format-InvariantFloat $profile.moveSpeed),
            (Format-InvariantFloat $profile.attackRange),
            [string][uint32]$profile.attackWindupMs,[string][uint32]$profile.attackActiveMs,
            [string][uint32]$profile.attackRecoveryMs,[string][uint32]$profile.deadDespawnMs) -join "`t"))
    }
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add("LOSTARK_SPAWN_GROUP_BOOTSTRAP`t1`t$WorldId`t$AreaId`t$($document.revision)`t$($anchorRows.Count)`t$($groupIds.Count)`t$($profileRows.Count)")
    foreach ($row in @($profileRows | Sort-Object)) { $lines.Add($row) }
    foreach ($row in @($anchorRows | Sort-Object)) { $lines.Add($row) }
    foreach ($row in $groupRows) { $lines.Add($row) }
    return [ordered]@{
        WorldId = $WorldId; AreaId = $AreaId; Revision = [uint32]$document.revision
        Lines = $lines; GroupIds = $groupIds; Count = $groupIds.Count; IsPresent = $true
    }
}

function Convert-WorldDocument {
    param(
        [string]$AreaId,
        [string]$WorldId,
        [hashtable]$ActorIds,
        [hashtable]$EncounterProfiles,
        [Collections.Generic.HashSet[string]]$SpawnGroupIds
    )

    $relativePath = "Data/Worlds/$AreaId/Gameplay.world.json"
    $document = Read-ProjectJson $relativePath
    Assert-ExactProperties $document @('schema','formatVersion','areaId','revision','placements') $relativePath
	Assert-JsonString $document.schema "$relativePath schema"
	Assert-JsonInteger $document.formatVersion "$relativePath formatVersion" 5 5
	Assert-JsonString $document.areaId "$relativePath areaId"
	Assert-JsonInteger $document.revision "$relativePath revision" 1 ([uint32]::MaxValue)
    if ($document.schema -ne 'lostark.world-gameplay' -or
		$document.formatVersion -ne 5 -or
        $document.areaId -ne $AreaId -or
        $document.revision -lt 1) {
        throw "World gameplay header is invalid: $relativePath"
    }

    $ids = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$npcPresentationEntries = [Collections.Generic.List[object]]::new()
	$placementIndex = @{}
	foreach ($placement in @($document.placements)) {
		if ($null -ne $placement.placementId) {
			$placementIndex[[string]$placement.placementId] = $placement
		}
	}
	$enabledPlayerSpawnCount = 0
    $rows = [Collections.Generic.List[string]]::new()
    foreach ($placement in @($document.placements)) {
		Assert-JsonString $placement.placementId "$relativePath placementId"
		Assert-JsonString $placement.kind "$relativePath kind"
		if ($placement.enabled -isnot [bool]) {
			throw "$relativePath enabled must be a JSON Boolean: $($placement.placementId)"
		}
        Assert-StableId $placement.placementId "$relativePath placementId"
        if (-not $ids.Add([string]$placement.placementId)) {
            throw "Duplicate world placement ID: $($placement.placementId)"
        }
		if ($placement.kind -notin @('playerSpawn','npc','boss','triggerBox','collisionBox')) {
			throw "Unknown world placement kind: $($placement.kind)"
		}
        if (@($placement.position).Count -ne 3) {
            throw "Placement position must contain exactly three numbers: $($placement.placementId)"
        }
		for ($coordinateIndex = 0; $coordinateIndex -lt 3; $coordinateIndex++) {
			Assert-JsonNumber $placement.position[$coordinateIndex] `
				"$relativePath position[$coordinateIndex]"
		}
		Assert-JsonNumber $placement.yawDegrees "$relativePath yawDegrees"
        $enabled = if ($placement.enabled) { 1 } else { 0 }
		$commonFields = @(
			$placement.placementId,
			$placement.kind,
			'-',
			'-',
			(Format-InvariantFloat $placement.position[0]),
			(Format-InvariantFloat $placement.position[1]),
			(Format-InvariantFloat $placement.position[2]),
			(Format-InvariantFloat $placement.yawDegrees),
			$enabled)
		if ($placement.kind -eq 'collisionBox') {
			Assert-ExactProperties $placement @(
				'placementId','kind','position','yawDegrees','enabled',
				'halfExtents') "$relativePath collisionBox"
			if (@($placement.halfExtents).Count -ne 3) {
				throw "Collision Box halfExtents must contain exactly three numbers: $($placement.placementId)"
			}
			$collisionFields = @()
			for ($extentIndex = 0; $extentIndex -lt 3; $extentIndex++) {
				Assert-JsonNumber $placement.halfExtents[$extentIndex] "$relativePath halfExtents[$extentIndex]"
				$extent = [double]$placement.halfExtents[$extentIndex]
				if ($extent -le 0.0 -or $extent -gt 1000.0) {
					throw "Collision Box half extent is out of range: $($placement.placementId)"
				}
				$collisionFields += Format-InvariantFloat $extent
			}
			$rows.Add((($commonFields + $collisionFields) -join "`t"))
			continue
		}
		if ($placement.kind -eq 'triggerBox') {
			Assert-ExactProperties $placement @(
				'placementId','kind','position','yawDegrees','enabled',
				'halfExtents','triggerOnce','events') "$relativePath triggerBox"
			if (@($placement.halfExtents).Count -ne 3) {
				throw "Trigger halfExtents must contain exactly three numbers: $($placement.placementId)"
			}
			for ($extentIndex = 0; $extentIndex -lt 3; $extentIndex++) {
				Assert-JsonNumber $placement.halfExtents[$extentIndex] "$relativePath halfExtents[$extentIndex]"
				$extent = [double]$placement.halfExtents[$extentIndex]
				if ($extent -le 0.0 -or $extent -gt 1000.0) {
					throw "Trigger half extent is out of range: $($placement.placementId)"
				}
			}
			if ($placement.triggerOnce -isnot [bool]) {
				throw "Trigger triggerOnce must be a JSON Boolean: $($placement.placementId)"
			}
			$events = @($placement.events)
			if ($events.Count -gt 1 -or ($placement.enabled -and $events.Count -ne 1)) {
				throw "Trigger Box requires exactly one event when enabled: $($placement.placementId)"
			}
			$triggerFields = @(
				(Format-InvariantFloat $placement.halfExtents[0]),
				(Format-InvariantFloat $placement.halfExtents[1]),
				(Format-InvariantFloat $placement.halfExtents[2]),
				$(if ($placement.triggerOnce) { '1' } else { '0' }),
				[string]$events.Count)
			foreach ($event in $events) {
				if ($event.type -eq 'movePlayer') {
					Assert-ExactProperties $event @(
						'type','targetPosition','durationSeconds','arcHeight') "$relativePath movePlayer event"
					if (@($event.targetPosition).Count -ne 3) {
						throw "movePlayer target requires three coordinates: $($placement.placementId)"
					}
					for ($targetIndex = 0; $targetIndex -lt 3; $targetIndex++) {
						Assert-JsonNumber $event.targetPosition[$targetIndex] "$relativePath targetPosition[$targetIndex]"
					}
					Assert-JsonNumber $event.durationSeconds "$relativePath durationSeconds"
					Assert-JsonNumber $event.arcHeight "$relativePath arcHeight"
					$duration = [double]$event.durationSeconds
					$arcHeight = [double]$event.arcHeight
					if ($duration -lt 0.05 -or $duration -gt 10.0 -or
						$arcHeight -lt 0.0 -or $arcHeight -gt 1000.0) {
						throw "movePlayer timing or arc is out of range: $($placement.placementId)"
					}
					$triggerFields += @(
						'movePlayer', '5',
						(Format-InvariantFloat $event.targetPosition[0]),
						(Format-InvariantFloat $event.targetPosition[1]),
						(Format-InvariantFloat $event.targetPosition[2]),
						(Format-InvariantFloat $duration),
						(Format-InvariantFloat $arcHeight))
				}
				elseif ($event.type -eq 'changeLevel') {
					Assert-ExactProperties $event @('type','targetWorldId') "$relativePath changeLevel event"
					Assert-JsonString $event.targetWorldId "$relativePath changeLevel targetWorldId"
					if ($event.targetWorldId -notin @('BERN','VALTAN_ARENA') -or
						[string]$event.targetWorldId -eq $WorldId) {
						throw "changeLevel target is unknown or equals the source world: $($placement.placementId)"
					}
					if ($WorldId -notin @('BERN','VALTAN_ARENA')) {
						throw "changeLevel is only supported between Bern and Valtan Arena."
					}
					$triggerFields += @('changeLevel', '1', [string]$event.targetWorldId)
				}
				elseif ($event.type -eq 'activateSpawnGroup') {
					Assert-ExactProperties $event @('type','spawnGroupId') "$relativePath activateSpawnGroup event"
					Assert-StableId $event.spawnGroupId "$relativePath spawnGroupId"
					if ($null -eq $SpawnGroupIds -or -not $SpawnGroupIds.Contains([string]$event.spawnGroupId)) {
						throw "Trigger references an unknown spawn group: $($event.spawnGroupId)"
					}
					$triggerFields += @('activateSpawnGroup', '1', [string]$event.spawnGroupId)
				}
				elseif ($event.type -eq 'activateEncounter') {
					Assert-ExactProperties $event @('type','targetPlacementId') "$relativePath activateEncounter event"
					Assert-StableId $event.targetPlacementId "$relativePath targetPlacementId"
					$target = $placementIndex[[string]$event.targetPlacementId]
					if ($null -eq $target -or $target.kind -ne 'boss' -or $target.enabled) {
						throw "Encounter trigger target must be a disabled boss placement: $($event.targetPlacementId)"
					}
					$triggerFields += @('activateEncounter', '1', [string]$event.targetPlacementId)
				}
				else {
					throw "Unsupported product trigger event: $($event.type)"
				}
			}
			$rows.Add((($commonFields + $triggerFields) -join "`t"))
			continue
		}

		if ($placement.kind -eq 'npc') {
			Assert-ExactProperties $placement @(
				'placementId','kind','archetypeId','encounterId',
				'idleClip','position','yawDegrees','enabled') "$relativePath placement"
			if ($null -ne $placement.idleClip) {
				Assert-JsonString $placement.idleClip "$relativePath idleClip"
				$clip = [string]$placement.idleClip
				if ($clip -notmatch '^[A-Za-z0-9_~.-]{1,64}$') {
					throw "NPC idleClip is not a stable clip name: $($placement.placementId)"
				}
				$npcPresentationEntries.Add(@{
					placementId = [string]$placement.placementId
					idleClip = $clip
				})
			}
		}
		else {
			Assert-ExactProperties $placement @(
				'placementId','kind','archetypeId','encounterId',
				'position','yawDegrees','enabled') "$relativePath placement"
		}
		Assert-JsonString $placement.archetypeId "$relativePath archetypeId" -AllowNull
		Assert-JsonString $placement.encounterId "$relativePath encounterId" -AllowNull
		if ($placement.kind -eq 'playerSpawn') {
			if ($null -ne $placement.archetypeId -and
				-not [string]::IsNullOrEmpty([string]$placement.archetypeId)) {
				throw "Player spawn must not own a character archetype: $($placement.placementId)"
			}
			if ($null -ne $placement.encounterId -and
				-not [string]::IsNullOrEmpty([string]$placement.encounterId)) {
				throw "Player spawn must not own an encounter: $($placement.placementId)"
			}
			if ($placement.enabled) {
				++$enabledPlayerSpawnCount
			}
		}
		else {
			Assert-StableId $placement.archetypeId "$relativePath archetypeId"
		}
		Assert-StableId $placement.encounterId "$relativePath encounterId" -AllowEmpty
		if ($placement.kind -ne 'playerSpawn' -and
			$placement.archetypeId -notin @($ActorIds[$placement.kind])) {
			throw "Archetype '$($placement.archetypeId)' is not available for kind '$($placement.kind)'."
		}
		$encounterId = if ($null -eq $placement.encounterId) { '' } else { [string]$placement.encounterId }
		if ($encounterId -and -not $EncounterProfiles.ContainsKey($encounterId)) {
			throw "Placement references an unknown encounter: $encounterId"
		}
        $serializedEncounterId = if ($encounterId) { $encounterId } else { '-' }
        if ($placement.kind -eq 'boss') {
            if (-not $encounterId) {
                throw "Boss placement requires an encounter ID: $($placement.placementId)"
            }
            $profile = $EncounterProfiles[$encounterId]
            if ($profile.bossArchetypeId -ne $placement.archetypeId) {
                throw "Boss placement archetype does not match encounter '$encounterId'."
            }
        }
        $serializedArchetypeId = if ($placement.kind -eq 'playerSpawn') { '-' } else { [string]$placement.archetypeId }
		$commonFields[2] = $serializedArchetypeId
		$commonFields[3] = $serializedEncounterId
		$rowFields = $commonFields
        $rows.Add(($rowFields -join "`t"))
    }
	if ($WorldId -eq 'VALTAN_ARENA' -and $enabledPlayerSpawnCount -ne 4) {
		throw "Valtan Arena requires exactly four enabled player spawns; got $enabledPlayerSpawnCount."
	}

    $sortedRows = @($rows | Sort-Object)
    $lines = [Collections.Generic.List[string]]::new()
	$lines.Add("LOSTARK_WORLD_BOOTSTRAP`t6`t$WorldId`t$AreaId`t$($document.revision)`t$($sortedRows.Count)")
    foreach ($row in $sortedRows) {
        $lines.Add($row)
    }
    return [ordered]@{
        WorldId = $WorldId
        AreaId = $AreaId
        Lines = $lines
        Count = $sortedRows.Count
        NpcPresentation = $npcPresentationEntries
    }
}

$actorIds = Get-ActorIds
$encounterProfiles = Get-EncounterProfiles
$monsterProfiles = Get-MonsterProfiles
$spawnDocuments = @(
    (Convert-SpawnGroupsDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -MonsterProfiles $monsterProfiles),
    (Convert-SpawnGroupsDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -MonsterProfiles $monsterProfiles)
)
$spawnByWorld = @{}
foreach ($spawn in $spawnDocuments) { $spawnByWorld[$spawn.WorldId] = $spawn }
$worlds = @(
    (Convert-WorldDocument -AreaId 'LV_BER_BERNCASTLE' -WorldId 'BERN' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.BERN.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_LUT_HEARTRB_ED' -WorldId 'VALTAN_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.VALTAN_ARENA.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_DEV_TRAINING_GROUND' -WorldId 'TRAINING_GROUND' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.TRAINING_GROUND.GroupIds),
    (Convert-WorldDocument -AreaId 'LV_LOBBY_CLASSSELECT_SL00' -WorldId 'CHARACTER_SELECT_ARENA' -ActorIds $actorIds -EncounterProfiles $encounterProfiles -SpawnGroupIds $spawnByWorld.CHARACTER_SELECT_ARENA.GroupIds)
)

if ($Mode -eq 'Publish') {
    $resolvedOutputRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
    [IO.Directory]::CreateDirectory($resolvedOutputRoot) | Out-Null
	$transactionId = [Guid]::NewGuid().ToString('N')
	$stagingRoot = Join-Path $resolvedOutputRoot ".staging.$transactionId"
	$promotions = [Collections.Generic.List[object]]::new()
	[IO.Directory]::CreateDirectory($stagingRoot) | Out-Null
	try {
		foreach ($world in $worlds) {
			$staged = Join-Path $stagingRoot "$($world.WorldId).worldbootstrap"
			[IO.File]::WriteAllLines(
				$staged,
				$world.Lines,
				[Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $world
				Staged = $staged
				Destination = Join-Path $resolvedOutputRoot "$($world.WorldId).worldbootstrap"
				Rollback = Join-Path $resolvedOutputRoot ".$($world.WorldId).rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}
		foreach ($spawn in @($spawnDocuments | Where-Object IsPresent)) {
			$staged = Join-Path $stagingRoot "$($spawn.WorldId).spawngroupsbootstrap"
			[IO.File]::WriteAllLines(
				$staged,
				$spawn.Lines,
				[Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $spawn
				Staged = $staged
				Destination = Join-Path $resolvedOutputRoot "$($spawn.WorldId).spawngroupsbootstrap"
				Rollback = Join-Path $resolvedOutputRoot ".$($spawn.WorldId).spawngroups.rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}
		$clientWorldRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Client/Bin/DataFiles/World'))
		[IO.Directory]::CreateDirectory($clientWorldRoot) | Out-Null
		foreach ($world in $worlds) {
			$staged = Join-Path $stagingRoot "$($world.WorldId).npcpresentation.json"
			$jsonLines = [Collections.Generic.List[string]]::new()
			$jsonLines.Add('{')
			$jsonLines.Add('  "schema": "lostark.npc-placement-presentation",')
			$jsonLines.Add('  "formatVersion": 1,')
			$jsonLines.Add("  `"worldId`": `"$($world.WorldId)`",")
			$entries = @($world.NpcPresentation | Sort-Object { [string]$_.placementId })
			if ($entries.Count -eq 0) {
				$jsonLines.Add('  "entries": []')
			}
			else {
				$jsonLines.Add('  "entries": [')
				for ($entryIndex = 0; $entryIndex -lt $entries.Count; $entryIndex++) {
					$suffix = if ($entryIndex -lt $entries.Count - 1) { ',' } else { '' }
					$jsonLines.Add("    { `"placementId`": `"$($entries[$entryIndex].placementId)`", `"idleClip`": `"$($entries[$entryIndex].idleClip)`" }$suffix")
				}
				$jsonLines.Add('  ]')
			}
			$jsonLines.Add('}')
			[IO.File]::WriteAllLines($staged, $jsonLines, [Text.UTF8Encoding]::new($false))
			$promotions.Add([ordered]@{
				World = $world
				Staged = $staged
				Destination = Join-Path $clientWorldRoot "$($world.WorldId).npcpresentation.json"
				Rollback = Join-Path $clientWorldRoot ".$($world.WorldId).npcpresentation.rollback.$transactionId"
				HadPrevious = $false
				Promoted = $false
			})
		}

		$promotedCount = 0
		foreach ($promotion in $promotions) {
			if ([IO.File]::Exists($promotion.Destination)) {
				[IO.File]::Move($promotion.Destination, $promotion.Rollback)
				$promotion.HadPrevious = $true
			}
			[IO.File]::Move($promotion.Staged, $promotion.Destination)
			$promotion.Promoted = $true
			$promotedCount++
			if ($FailureAfterPromote -eq $promotedCount) {
				throw "Injected world publish failure after promotion $promotedCount."
			}
		}

		foreach ($promotion in $promotions) {
			if ([IO.File]::Exists($promotion.Rollback)) {
				[IO.File]::Delete($promotion.Rollback)
			}
			Write-Output "Published $($promotion.World.WorldId): $($promotion.World.Count) placements -> $($promotion.Destination)"
		}
	}
	catch {
		$publishFailure = $_
		$rollbackFailures = [Collections.Generic.List[string]]::new()
		for ($index = $promotions.Count - 1; $index -ge 0; --$index) {
			$promotion = $promotions[$index]
			try {
				if ($promotion.Promoted -and [IO.File]::Exists($promotion.Destination)) {
					[IO.File]::Delete($promotion.Destination)
				}
				if ($promotion.HadPrevious) {
					if (-not [IO.File]::Exists($promotion.Rollback)) {
						throw "Rollback backup is missing: $($promotion.Rollback)"
					}
					[IO.File]::Move($promotion.Rollback, $promotion.Destination)
				}
			}
			catch {
				$rollbackFailures.Add("$($promotion.Destination): $($_.Exception.Message)")
			}
		}
		if ($rollbackFailures.Count -ne 0) {
			$preserved = @($promotions | Where-Object { [IO.File]::Exists($_.Rollback) } |
				ForEach-Object Rollback)
			throw "World publish failed: $($publishFailure.Exception.Message) Rollback recovery was incomplete. Preserved backups=[$($preserved -join ',')]. Failures=[$($rollbackFailures -join '; ')]"
		}
		throw $publishFailure
	}
	finally {
		if ([IO.Directory]::Exists($stagingRoot)) {
			Remove-Item -LiteralPath $stagingRoot -Recurse -Force
		}
	}
}
else {
	foreach ($world in $worlds) {
		Write-Output "Validated $($world.WorldId): $($world.Count) placements"
	}
	foreach ($spawn in @($spawnDocuments | Where-Object IsPresent)) {
		Write-Output "Validated $($spawn.WorldId): $($spawn.Count) spawn groups"
	}
}
```

### G00-07-19. `Tools/Build/Invoke-BuildAndRegression.ps1` full code

```powershell
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipBuild,
    [string]$ResourceRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$reportParent = [IO.Path]::GetFullPath(
    (Join-Path $repoRoot '.codex_tmp\regression'))
$reportRoot = [IO.Path]::GetFullPath(
    (Join-Path $reportParent $Configuration))
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$valtanHarnessExe = Join-Path $repoRoot `
    "Tools\ValtanFourPlayerHarness\Bin\$Configuration\ValtanFourPlayerHarness.exe"
$runtimeResourceRoot = if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    Join-Path $repoRoot 'Client\Bin\Resources'
}
else {
    [IO.Path]::GetFullPath($ResourceRoot)
}

function Resolve-MSBuild {
    $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    $candidate =
        'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        return $candidate
    }
    throw 'MSBuild.exe was not found.'
}

function Invoke-MSBuildProject {
    param(
        [string]$MSBuild,
        [string]$Project
    )

    & $MSBuild $Project /m /nodeReuse:false /t:Build `
        "/p:Configuration=$Configuration" /p:Platform=x64 /v:minimal
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed: $Project"
    }
}

function Assert-RuntimeLayout {
    $required = @(
        $clientExe,
        $serverExe,
        $valtanHarnessExe,
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_VtxTex.hlsl'),
        (Join-Path $runtimeResourceRoot 'Fonts')
    )
    $missing = @($required | Where-Object {
        -not (Test-Path -LiteralPath $_)
    })
    if ($missing.Count -ne 0) {
        throw "Runtime layout is incomplete: $($missing -join ', ')"
    }
}

Push-Location $repoRoot
try {
    if (-not $reportRoot.StartsWith(
            $reportParent.TrimEnd('\') + '\',
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Regression report path escaped its generated root: $reportRoot"
    }
    if ([IO.Directory]::Exists($reportRoot)) {
        Remove-Item -LiteralPath $reportRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Path $reportRoot -Force | Out-Null

    if (-not $SkipBuild) {
        $msbuild = Resolve-MSBuild
        Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
        & cmd /c ".\UpdateLib.bat $Configuration"
        if ($LASTEXITCODE -ne 0) {
            throw 'UpdateLib.bat failed.'
        }
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ValtanFourPlayerHarness\Default\ValtanFourPlayerHarness.vcxproj'
        Invoke-MSBuildProject $msbuild `
            'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
    }

    Assert-RuntimeLayout

    $protocolHarness = Join-Path $repoRoot `
        "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
    & $protocolHarness
    if ($LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }

    $frontendHarness = Join-Path $repoRoot `
        "Tools\ClientFrontendHarness\Bin\$Configuration\ClientFrontendHarness.exe"
    $previousResourceRoot = [Environment]::GetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', 'Process')
    try {
        [Environment]::SetEnvironmentVariable(
            'LOSTARK_RESOURCE_ROOT',
            $runtimeResourceRoot,
            'Process')
        & $frontendHarness
        if ($LASTEXITCODE -ne 0) {
            throw 'ClientFrontendHarness failed.'
        }

		$effectCatalog = Join-Path $repoRoot `
			'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
		& $frontendHarness --effect-reconstructed-gpu-material $effectCatalog
		if ($LASTEXITCODE -ne 0) {
			throw 'Artist 31470 WARP first-draw harness failed.'
		}
    }
    finally {
        [Environment]::SetEnvironmentVariable(
            'LOSTARK_RESOURCE_ROOT', $previousResourceRoot, 'Process')
    }

    & $serverExe --contract-test
    if ($LASTEXITCODE -ne 0) {
        throw 'Server gameplay contract tests failed.'
    }

    & (Join-Path $repoRoot `
        'Tools\Network\Run-ValtanFourPlayerHarness.ps1') `
        -Configuration $Configuration

    & (Join-Path $repoRoot `
        'Tools\ProjectAudit\Invoke-ProjectAudit.ps1') `
        -ReportPath (Join-Path $reportRoot 'ProjectAudit.json') `
        -ResourceRoot $runtimeResourceRoot
    if ($LASTEXITCODE -ne 0) {
        throw 'Project audit failed.'
    }

    Write-Host "Regression completed: $Configuration"
    Write-Host 'Runtime level validation uses Framework.slnLaunch (Server + Client).'
}
finally {
    Pop-Location
}
```

### G00-07-20. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` full code

```powershell
[CmdletBinding()]
param(
    [string]$ReportPath = '.codex_tmp/ProjectAudit.json',
    [string]$ResourceRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Get-Location).Path)
$resourceRoot = if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    Join-Path $repoRoot 'Client\Bin\Resources'
}
else {
    [IO.Path]::GetFullPath($ResourceRoot)
}
$checks = [Collections.Generic.List[object]]::new()
$failures = [Collections.Generic.List[string]]::new()

function Add-Check {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Detail
    )

    $checks.Add([ordered]@{
        name = $Name
        passed = $Passed
        detail = $Detail
    })
    if (-not $Passed) {
        $failures.Add("$Name`: $Detail")
    }
}

function Read-Json {
    param([string]$Path)
	return Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Test-JsonNumber {
	param([object]$Value)
	if ($null -eq $Value -or $Value -is [bool] -or
		$Value -isnot [byte] -and $Value -isnot [sbyte] -and
		$Value -isnot [int16] -and $Value -isnot [uint16] -and
		$Value -isnot [int32] -and $Value -isnot [uint32] -and
		$Value -isnot [int64] -and $Value -isnot [uint64] -and
		$Value -isnot [single] -and $Value -isnot [double] -and
		$Value -isnot [decimal]) { return $false }
	$number = [double]$Value
	return -not [double]::IsNaN($number) -and -not [double]::IsInfinity($number)
}

function Test-Uint64String {
	param([object]$Value)
	$parsed = [uint64]0
	return $Value -is [string] -and $Value -match '^[1-9][0-9]{0,19}$' -and
		[uint64]::TryParse($Value, [Globalization.NumberStyles]::None,
			[Globalization.CultureInfo]::InvariantCulture, [ref]$parsed) -and $parsed -ne 0
}

function Get-ProjectItems {
    param([string]$ProjectPath)

    [xml]$project = Get-Content -LiteralPath $ProjectPath
    $manager = [Xml.XmlNamespaceManager]::new($project.NameTable)
    $manager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
    return @($project.SelectNodes('//m:ClInclude|//m:ClCompile|//m:FxCompile|//m:None', $manager))
}

function Get-WModelTextureReferences {
    param([string]$Path)

    $decoded = [Text.Encoding]::Unicode.GetString(
        [IO.File]::ReadAllBytes($Path))
    return @([regex]::Matches(
        $decoded,
        '(?:Resource|Resources)[\\/][^\x00\r\n]{1,259}?\.(?:dds|png|tga|jpg|jpeg|bmp)',
        [Text.RegularExpressions.RegexOptions]::IgnoreCase) |
        ForEach-Object { $_.Value })
}

try {
    $uiDocuments = @(
        'Data\UI\HUD\HUD_Layout.json',
        'Data\UI\ScreenUI\ScreenUI.json')
    $totalSlots = 0
    $uiValid = $true
    foreach ($uiPath in $uiDocuments) {
        $ui = Read-Json $uiPath
        $slotIds = @($ui.slots | ForEach-Object id)
        $duplicates = @($slotIds | Group-Object | Where-Object Count -gt 1)
        $badPaths = @($ui.slots | ForEach-Object {
            $_.layers.path
            $_.layers.hoverPath
            $_.shine.texture
            $_.animation.frames
        } | Where-Object { $_ -and $_ -notlike 'UI/*' })
        $uiValid = $uiValid -and $ui.schema -eq 'lostark.ui-layout' -and
            $ui.formatVersion -eq 1 -and $duplicates.Count -eq 0 -and
            $badPaths.Count -eq 0 -and @($ui.slots).Count -gt 0
        $totalSlots += @($ui.slots).Count
    }
    Add-Check 'ui.json-contract' $uiValid "documents=$($uiDocuments.Count) slots=$totalSlots"

    $mapCatalog = Read-Json 'Data\Maps\MapCatalog.json'
    $mapRoot = 'Client\Bin\DataFiles\Map'
    $missingMapFiles = [Collections.Generic.List[string]]::new()
    $invalidMapPaths = [Collections.Generic.List[string]]::new()
    foreach ($area in $mapCatalog.areas) {
        foreach ($requiredProperty in @('sourceCatalog', 'sourcePlacements', 'catalog')) {
            if ($area.PSObject.Properties.Name -notcontains $requiredProperty) {
                $invalidMapPaths.Add("$($area.id):missing-$requiredProperty")
            }
        }
        foreach ($property in @(
            'sourceCatalog', 'sourcePlacements',
            'sourceLights',
            'sourceDeployCatalog', 'sourceDeployPlacements',
            'catalog', 'placements', 'lights', 'deployCatalog', 'deployPlacements',
            'navigationSource', 'navigationPaint', 'navigationBlockers',
            'navigationRuntime',
            'gameplayDocument')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not (Test-Path -LiteralPath $area.$property) -and
                -not ($area.id -eq 'LV_BER_BERNCASTLE' -and
                    $property -in @('navigationSource', 'navigationPaint'))) {
                $missingMapFiles.Add($area.$property)
            }
        }
        foreach ($property in @(
            'sourceCatalog', 'sourcePlacements',
            'sourceLights',
            'sourceDeployCatalog', 'sourceDeployPlacements')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not ([string]$area.$property).StartsWith('Data/Maps/', [StringComparison]::Ordinal)) {
                $invalidMapPaths.Add("$($area.id):$property")
            }
        }
        foreach ($property in @('catalog', 'placements', 'lights', 'deployCatalog', 'deployPlacements')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not ([string]$area.$property).StartsWith('Client/Bin/DataFiles/Map/', [StringComparison]::Ordinal)) {
                $invalidMapPaths.Add("$($area.id):$property")
            }
        }
        foreach ($property in @('navigationSource', 'navigationPaint', 'navigationBlockers')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not ([string]$area.$property).StartsWith('Data/Navigation/', [StringComparison]::Ordinal)) {
                $invalidMapPaths.Add("$($area.id):$property")
            }
        }
        if ($area.PSObject.Properties.Name -contains 'navigationRuntime' -and
            -not ([string]$area.navigationRuntime).StartsWith(
                'Client/Bin/DataFiles/Navigation/', [StringComparison]::Ordinal)) {
            $invalidMapPaths.Add("$($area.id):navigationRuntime")
        }
    }
    $mapDataNames = @(Get-ChildItem -LiteralPath $mapRoot -File | ForEach-Object Name)
    $legacyMapFiles = @($mapDataNames | Where-Object { $_ -like 'BG_RAD_VALTAN_A*' })
	Add-Check 'maps.catalog' ($mapCatalog.schema -eq 'lostark.map-catalog' -and
		$missingMapFiles.Count -eq 0 -and $invalidMapPaths.Count -eq 0 -and
		$legacyMapFiles.Count -eq 0) "missing=$($missingMapFiles.Count) invalidPaths=$($invalidMapPaths.Count) legacy=$($legacyMapFiles.Count)"

	$valtanCatalogEntries = @($mapCatalog.areas | Where-Object {
		$_.id -ceq 'LV_LUT_HEARTRB_ED'
	})
	$valtanLightErrors = [Collections.Generic.List[string]]::new()
	$valtanLightCount = 0
	$valtanLightHashesMatch = $false
	if ($valtanCatalogEntries.Count -ne 1) {
		$valtanLightErrors.Add('catalog-entry-count')
	}
	else {
		$valtanCatalogEntry = $valtanCatalogEntries[0]
		$expectedSourceLightPath =
			'Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.maplights.json'
		$expectedRuntimeLightPath =
			'Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.maplights.json'
		if ($valtanCatalogEntry.sourceLights -cne $expectedSourceLightPath -or
			$valtanCatalogEntry.lights -cne $expectedRuntimeLightPath) {
			$valtanLightErrors.Add('catalog-paths')
		}
		elseif (-not (Test-Path -LiteralPath $expectedSourceLightPath -PathType Leaf) -or
			-not (Test-Path -LiteralPath $expectedRuntimeLightPath -PathType Leaf)) {
			$valtanLightErrors.Add('missing-source-or-runtime')
		}
		else {
			$valtanLightHashesMatch =
				(Get-FileHash -LiteralPath $expectedSourceLightPath -Algorithm SHA256).Hash -ceq
				(Get-FileHash -LiteralPath $expectedRuntimeLightPath -Algorithm SHA256).Hash
			if (-not $valtanLightHashesMatch) {
				$valtanLightErrors.Add('source-runtime-sha')
			}

			$lightDocument = Read-Json $expectedSourceLightPath
			$rootFields = @('schema', 'formatVersion', 'areaId', 'provenance', 'lights')
			$rootHasExactFields = @($lightDocument.PSObject.Properties).Count -eq $rootFields.Count -and
				@($rootFields | Where-Object {
					$lightDocument.PSObject.Properties.Name -cnotcontains $_
				}).Count -eq 0
			if (-not $rootHasExactFields -or
				$lightDocument.schema -cne 'lostark.map-light-presentation' -or
				-not (Test-JsonNumber $lightDocument.formatVersion) -or
				[double]$lightDocument.formatVersion -ne 1.0 -or
				$lightDocument.areaId -cne 'LV_LUT_HEARTRB_ED' -or
				$lightDocument.provenance -cne `
				'SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED' -or
				$lightDocument.lights -isnot [System.Array]) {
				$valtanLightErrors.Add('root-contract')
			}

			$expectedTowerLights = @{
				'light.valtan.tower.pointlight_102_lc' = @{
					Source = 'SL04:export:733:pointlight_102_lc'
					Position = @(203.460606, 24.734033, -127.571465)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_104_lc' = @{
					Source = 'SL04:export:735:pointlight_104_lc'
					Position = @(187.751602, 24.734033, -158.425654)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_21_lc' = @{
					Source = 'SL04:export:750:pointlight_21_lc'
					Position = @(161.835400, 24.734033, -168.356855)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_106_lc' = @{
					Source = 'SL04:export:737:pointlight_106_lc'
					Position = @(135.544980, 24.734033, -159.737500)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_11_lc' = @{
					Source = 'SL04:export:739:pointlight_11_lc'
					Position = @(191.995703, 24.734033, -99.211338)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_64_lc' = @{
					Source = 'SL04:export:768:pointlight_64_lc'
					Position = @(188.291309, 14.172728, -123.240234)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_5_lc' = @{
					Source = 'SL04:export:767:pointlight_5_lc'
					Position = @(196.451973, -1.394727, -127.035703)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_103_lc' = @{
					Source = 'SL04:export:734:pointlight_103_lc'
					Position = @(198.301699, -16.068151, -123.891680)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 10.24; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_98_lc' = @{
					Source = 'SL04:export:780:pointlight_98_lc'
					Position = @(187.936191, -35.772830, -122.317480)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 3.0
				}
				'light.valtan.tower.pointlight_24_lc' = @{
					Source = 'SL04:export:753:pointlight_24_lc'
					Position = @(176.511172, 10.832922, -143.720859)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_25_lc' = @{
					Source = 'SL04:export:754:pointlight_25_lc'
					Position = @(173.250137, 10.832922, -145.604990)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_27_lc' = @{
					Source = 'SL04:export:756:pointlight_27_lc'
					Position = @(180.038984, -1.394727, -150.643105)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_28_lc' = @{
					Source = 'SL04:export:757:pointlight_28_lc'
					Position = @(183.363691, -21.762295, -150.889150)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_29_lc' = @{
					Source = 'SL04:export:758:pointlight_29_lc'
					Position = @(136.400566, 13.365049, -151.214629)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_68_lc' = @{
					Source = 'SL04:export:770:pointlight_68_lc'
					Position = @(131.826855, -1.394727, -159.343447)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_105_lc' = @{
					Source = 'SL04:export:736:pointlight_105_lc'
					Position = @(137.638096, -1.394727, -151.278281)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_22_lc' = @{
					Source = 'SL04:export:751:pointlight_22_lc'
					Position = @(137.706982, -21.762295, -151.423994)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_30_lc' = @{
					Source = 'SL04:export:760:pointlight_30_lc'
					Position = @(182.656191, 14.172728, -101.204268)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_15_lc' = @{
					Source = 'SL04:export:743:pointlight_15_lc'
					Position = @(184.628574, -1.394727, -100.711621)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_26_lc' = @{
					Source = 'SL04:export:755:pointlight_26_lc'
					Position = @(158.904902, 13.365049, -153.888477)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 9.0; Falloff = 2.0; Brightness = 6.0
				}
				'light.valtan.tower.pointlight_0_lc' = @{
					Source = 'SL04:export:732:pointlight_0_lc'
					Position = @(160.520391, -1.394727, -161.196641)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
				'light.valtan.tower.pointlight_20_lc' = @{
					Source = 'SL04:export:749:pointlight_20_lc'
					Position = @(162.287881, -21.762295, -160.083994)
					Color = @(1.0, (37.0 / 255.0), 0.0, 1.0)
					Radius = 20.48; Falloff = 2.0; Brightness = 2.5
				}
			}
			$lightFields = @('lightId', 'sourceLevel', 'sourceObjectId', 'position',
				'radiusMeters', 'falloffExponent', 'color', 'brightness')
			$seenLightIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			$seenSourceIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
			$valtanLights = @($lightDocument.lights)
			$valtanLightCount = $valtanLights.Count
			foreach ($light in $valtanLights) {
				$lightId = [string]$light.lightId
				$expectedLight = $expectedTowerLights[$lightId]
				$position = @($light.position)
				$color = @($light.color)
				$lightHasExactFields = @($light.PSObject.Properties).Count -eq $lightFields.Count -and
					@($lightFields | Where-Object {
						$light.PSObject.Properties.Name -cnotcontains $_
					}).Count -eq 0
				$lightValid = $lightHasExactFields -and $null -ne $expectedLight -and
					$seenLightIds.Add($lightId) -and
					$seenSourceIds.Add([string]$light.sourceObjectId) -and
					$light.sourceLevel -ceq 'LV_LUT_HEARTRB_ED_SL04' -and
					$light.sourceObjectId -ceq $expectedLight.Source -and
					$light.position -is [System.Array] -and $position.Count -eq 3 -and
					$light.color -is [System.Array] -and $color.Count -eq 4 -and
					(Test-JsonNumber $light.radiusMeters) -and
					[math]::Abs([double]$light.radiusMeters - [double]$expectedLight.Radius) -le 0.000000001 -and
					(Test-JsonNumber $light.falloffExponent) -and
					[math]::Abs([double]$light.falloffExponent - [double]$expectedLight.Falloff) -le 0.000000001 -and
					(Test-JsonNumber $light.brightness) -and
					[math]::Abs([double]$light.brightness - [double]$expectedLight.Brightness) -le 0.000000001
				for ($axis = 0; $lightValid -and $axis -lt 3; ++$axis) {
					$lightValid = (Test-JsonNumber $position[$axis]) -and
						[math]::Abs([double]$position[$axis] -
							[double]$expectedLight.Position[$axis]) -le 0.000000001
				}
				for ($channel = 0; $lightValid -and $channel -lt 4; ++$channel) {
					$lightValid = (Test-JsonNumber $color[$channel]) -and
						[math]::Abs([double]$color[$channel] -
							[double]$expectedLight.Color[$channel]) -le 0.000000001
				}
				if (-not $lightValid) {
					$valtanLightErrors.Add("light:$lightId")
				}
			}
			if ($valtanLightCount -ne $expectedTowerLights.Count -or
				$seenLightIds.Count -ne $expectedTowerLights.Count -or
				$seenSourceIds.Count -ne $expectedTowerLights.Count) {
				$valtanLightErrors.Add('light-count-or-identity')
			}
		}
	}
	$mapLightDocumentSource = Get-Content 'Client\Private\MapLightDocument.cpp' -Raw
	$mapLightRuntimeSource = Get-Content 'Client\Private\MapLightPresentationRuntime.cpp' -Raw
	$valtanLevelSource = Get-Content 'Client\Private\Level_ValtanArena.cpp' -Raw
	$mapToolLightSource = Get-Content 'Client\Private\MapTool.cpp' -Raw
	$mapPublisherSource = Get-Content 'Tools\MapPipeline\Publish-MapAuthoring.ps1' -Raw
	$mapLightSeamsValid =
		$mapLightDocumentSource -match 'lostark\.map-light-presentation' -and
		$mapLightRuntimeSource -match 'Try_BuildEffectPointLightDesc' -and
		$mapLightRuntimeSource -match 'Add_TransientLight' -and
		$valtanLevelSource -match 'Load_Runtime\(pEntry->pMapAreaId\)' -and
		$mapToolLightSource -match 'descriptor\.sourceLights' -and
		$mapToolLightSource -match 'stagedMapLightPresentation->Load' -and
		$mapPublisherSource -match 'function Add-MapLightPublishFile' -and
		$mapPublisherSource -match 'Add-MapLightPublishFile \$files'
	if (-not $mapLightSeamsValid) {
		$valtanLightErrors.Add('publisher-or-runtime-seam')
	}
	Add-Check 'maps.valtan-source-light-presentation' `
		($valtanLightErrors.Count -eq 0) `
		"lights=$valtanLightCount shaMatch=$valtanLightHashesMatch errors=$($valtanLightErrors.Count)"

	$valtanSourcePlacements =
		'Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.mapplacements'
	$valtanRuntimePlacements =
		'Client\Bin\DataFiles\Map\LV_LUT_HEARTRB_ED.mapplacements'
	$sourcePlacementText = Get-Content -LiteralPath $valtanSourcePlacements -Raw -Encoding UTF8
	$runtimePlacementText = Get-Content -LiteralPath $valtanRuntimePlacements -Raw -Encoding UTF8
	$sourcePlacementLines = @($sourcePlacementText -split "`r?`n" |
		Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
	$runtimePlacementLines = @($runtimePlacementText -split "`r?`n" |
		Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
	$sourcePlacementHeader = if ($sourcePlacementLines.Count -gt 0) {
		$sourcePlacementLines[0]
	} else { '' }
	$runtimePlacementHeader = if ($runtimePlacementLines.Count -gt 0) {
		$runtimePlacementLines[0]
	} else { '' }
	$sourcePlacementIds = [Collections.Generic.HashSet[string]]::new(
		[StringComparer]::Ordinal)
	$runtimePlacementIds = [Collections.Generic.HashSet[string]]::new(
		[StringComparer]::Ordinal)
	$sourcePlacementRowsValid = $true
	$runtimePlacementRowsValid = $true
	foreach ($placementLine in @($sourcePlacementLines | Select-Object -Skip 1)) {
		$placementMatch = [regex]::Match(
			$placementLine, '^(?<Id>[0-9]+)\s+')
		if (-not $placementMatch.Success -or
			-not $sourcePlacementIds.Add($placementMatch.Groups['Id'].Value)) {
			$sourcePlacementRowsValid = $false
		}
	}
	foreach ($placementLine in @($runtimePlacementLines | Select-Object -Skip 1)) {
		$placementMatch = [regex]::Match(
			$placementLine, '^(?<Id>[0-9]+)\s+')
		if (-not $placementMatch.Success -or
			-not $runtimePlacementIds.Add($placementMatch.Groups['Id'].Value)) {
			$runtimePlacementRowsValid = $false
		}
	}
	$placementFilesMatch =
		(Get-FileHash -LiteralPath $valtanSourcePlacements -Algorithm SHA256).Hash -ceq
		(Get-FileHash -LiteralPath $valtanRuntimePlacements -Algorithm SHA256).Hash
	$sourceTowerRepresentatives = @(
		@{ Id = '16607808932384341195'; Export = '2842' },
		@{ Id = '17340363601769171092'; Export = '3990' },
		@{ Id = '12792519784808763156'; Export = '4281' },
		@{ Id = '15813727945424250256'; Export = '4401' },
		@{ Id = '12059080955363256336'; Export = '4544' })
	$missingTowerRepresentatives = [Collections.Generic.List[string]]::new()
	foreach ($representative in $sourceTowerRepresentatives) {
		$representativePattern = '(?m)^' + [regex]::Escape($representative.Id) +
			'\s+"LV_LUT_HEARTRB_ED_SL04:export:' + [regex]::Escape($representative.Export) +
			'"\s+"LV_LUT_HEARTRB_ED_SL04"\s+"component"\s+' +
			'"MAP_70B41719DECE_BG_ATM_LOGHILL_CONSTRUCTPROP04_SM_OLD_02"\s+'
		if ($sourcePlacementText -notmatch $representativePattern -or
			$runtimePlacementText -notmatch $representativePattern) {
			$missingTowerRepresentatives.Add($representative.Id)
		}
	}
	$badTowerOverlayPattern = '(?m)^710[0-9]+\s+'
	$badTowerOverlayCount =
		[regex]::Matches($sourcePlacementText, $badTowerOverlayPattern).Count +
		[regex]::Matches($runtimePlacementText, $badTowerOverlayPattern).Count
	$badTowerProvenanceCount =
		[regex]::Matches($sourcePlacementText, 'PROJECT_AUTHORED_RIM').Count +
		[regex]::Matches($runtimePlacementText, 'PROJECT_AUTHORED_RIM').Count
	$valtanPlacementDocumentsValid =
		$sourcePlacementHeader -ceq 'LOSTARK_MAP_PLACEMENTS 2 "LV_LUT_HEARTRB_ED" 13192' -and
		$runtimePlacementHeader -ceq 'LOSTARK_MAP_PLACEMENTS 2 "LV_LUT_HEARTRB_ED" 13192' -and
		$sourcePlacementLines.Count -eq 13193 -and
		$runtimePlacementLines.Count -eq 13193 -and
		$sourcePlacementIds.Count -eq 13192 -and
		$runtimePlacementIds.Count -eq 13192 -and
		$sourcePlacementRowsValid -and $runtimePlacementRowsValid -and
		$placementFilesMatch -and
		@(Compare-Object @($sourcePlacementIds) @($runtimePlacementIds) -CaseSensitive).Count -eq 0 -and
		$valtanCatalogEntries.Count -eq 1 -and
		[int]$valtanCatalogEntries[0].placementCount -eq 13192 -and
		$badTowerProvenanceCount -eq 0 -and $badTowerOverlayCount -eq 0 -and
		$missingTowerRepresentatives.Count -eq 0
	Add-Check 'maps.valtan-tower-source-geometry' `
		$valtanPlacementDocumentsValid `
		"placements=$($sourcePlacementIds.Count)/13192 bytesMatch=$placementFilesMatch representatives=$($sourceTowerRepresentatives.Count - $missingTowerRepresentatives.Count)/$($sourceTowerRepresentatives.Count) badRim=$badTowerProvenanceCount bad710=$badTowerOverlayCount"
	$editorAreas = @{}
	foreach ($area in $mapCatalog.areas) { $editorAreas[[string]$area.id] = $area }
	$characterSelectEditor = $editorAreas['LV_LOBBY_CLASSSELECT_SL00']
	$bernEditor = $editorAreas['LV_BER_BERNCASTLE']
	$valtanEditor = $editorAreas['LV_LUT_HEARTRB_ED']
	$trainingEditor = $editorAreas['LV_SHS_RCARENA_D']
	$mapToolSource = Get-Content 'Client\Private\MapTool.cpp' -Raw
	$loaderSource = Get-Content 'Client\Private\Loader.cpp' -Raw
	$mainAppSource = Get-Content 'Client\Private\MainApp.cpp' -Raw
	$lobbySource = Get-Content 'Client\Private\Level_Lobby.cpp' -Raw
	$editorPoliciesValid =
		$null -ne $characterSelectEditor -and
		$characterSelectEditor.navigationSource -eq 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource' -and
		$characterSelectEditor.navigationPaint -eq 'Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint' -and
		$characterSelectEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$null -ne $bernEditor -and
		$bernEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$bernEditor.navigationSource -eq 'Data/Navigation/LV_BER_BERNCASTLE.navsource' -and
		$bernEditor.navigationPaint -eq 'Data/Navigation/LV_BER_BERNCASTLE.navpaint' -and
		$null -ne $valtanEditor -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationSource' -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationPaint' -and
		$valtanEditor.PSObject.Properties.Name -contains 'navigationBlockers' -and
		$valtanEditor.PSObject.Properties.Name -contains 'gameplayDocument' -and
		$null -ne $trainingEditor -and
		$trainingEditor.PSObject.Properties.Name -notcontains 'navigationSource' -and
		$trainingEditor.PSObject.Properties.Name -notcontains 'gameplayDocument' -and
		$mapToolSource -match 'Load_Source\(' -and
		$mapToolSource -match 'if \(descriptor\.areaId == "LV_LOBBY_CLASSSELECT_SL00" \|\|[\s\S]{0,160}descriptor\.areaId == "LV_BER_BERNCASTLE"' -and
		$mapToolSource -match 'allowNavigationBootstrap\s*=\s*descriptor\.areaId == "LV_BER_BERNCASTLE"' -and
		$mapToolSource -notmatch '\.Export_Runtime\(' -and
		$mapToolSource -match 'MapEditorWorkspaceService::Is_Active' -and
		$loaderSource -match 'Ready_MapAuthoringCore' -and
		$loaderSource -match 'MapEditorWorkspaceService::Is_Requested' -and
		$mainAppSource -notmatch 'debug\.map-editor' -and
		$lobbySource -match 'MapEditorWorkspaceService::Request\(' -and
		$lobbySource -match 'LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE'
	Add-Check 'maps.editor-workspace-policy' $editorPoliciesValid 'Lobby Test owns editor entry; F1 only toggles tools; four exact editor Areas; Data-only save'
	$mapLoadScopeHeader = Get-Content 'Client\Public\MapLoadScope.h' -Raw
	$mapPlacementRuntimeSource = Get-Content 'Client\Private\MapPlacementRuntime.cpp' -Raw
	$levelRegistryMapScopeSource = Get-Content 'Client\Private\LevelRegistry.cpp' -Raw
	$productEditorVisualScopeValid =
		$mapLoadScopeHeader -match 'std::string excludedAssetGroupId' -and
		$mapPlacementRuntimeSource -match 'left\.excludedAssetGroupId == right\.excludedAssetGroupId' -and
		$mapPlacementRuntimeSource -match 'pAsset->groupId == loadScope\.excludedAssetGroupId' -and
		$levelRegistryMapScopeSource -match 'MakeFullMapScope\("landscape"\)' -and
		$levelRegistryMapScopeSource -match '"LV_LUT_HEARTRB_ED",\s*"scene\.valtan\.cool-low-key\.v1",\s*MakeFullMapScope\(\)' -and
		$mapToolSource -match 'IsBernLandscapePlacement' -and
		$mapToolSource -match 'Show Bern Landscape'
	Add-Check 'maps.product-editor-visual-scope' $productEditorVisualScopeValid 'Bern full published map excluding quarantined landscape; Valtan full published map; MapTool reversible Bern preview'

	$physicsSdkRoot = Join-Path $repoRoot 'Engine\ThirdPartyLib\PhysX'
	$physicsSdkRequired = @(
		'Inc\PxPhysicsAPI.h',
		'Lib\Debug\PhysX_64.lib',
		'Lib\Debug\PhysXCommon_64.lib',
		'Lib\Debug\PhysXFoundation_64.lib',
		'Lib\Debug\PhysXExtensions_static_64.lib',
		'Lib\Debug\PhysXPvdSDK_static_64.lib',
		'Lib\Release\PhysX_64.lib',
		'Lib\Release\PhysXCommon_64.lib',
		'Lib\Release\PhysXFoundation_64.lib',
		'Lib\Release\PhysXExtensions_static_64.lib',
		'Lib\Release\PhysXPvdSDK_static_64.lib',
		'Bin\Debug\PhysX_64.dll',
		'Bin\Debug\PhysXCommon_64.dll',
		'Bin\Debug\PhysXFoundation_64.dll',
		'Bin\Release\PhysX_64.dll',
		'Bin\Release\PhysXCommon_64.dll',
		'Bin\Release\PhysXFoundation_64.dll',
		'LICENSE.md',
		'NOTICE.md')
	$missingPhysicsSdk = @($physicsSdkRequired | Where-Object {
		-not (Test-Path -LiteralPath (Join-Path $physicsSdkRoot $_))
	})
	Add-Check 'physics.physx-sdk-layout' ($missingPhysicsSdk.Count -eq 0) `
		"missing=$($missingPhysicsSdk -join ',')"
	$clientRuntimeProjectSource = Get-Content 'Client\Default\Client.vcxproj' -Raw
	$updateLibSource = Get-Content 'UpdateLib.bat' -Raw
	$physxRuntimeNames = @(
		'PhysX_64.dll',
		'PhysXCommon_64.dll',
		'PhysXFoundation_64.dll')
	$physxRuntimeDeploymentValid =
		$clientRuntimeProjectSource -match 'DeployClientRuntimeDependencies' -and
		$clientRuntimeProjectSource -match 'PhysXRuntimeRoot'
	foreach ($runtimeName in $physxRuntimeNames) {
		$physxRuntimeDeploymentValid = $physxRuntimeDeploymentValid -and
			$clientRuntimeProjectSource.Contains($runtimeName) -and
			$updateLibSource.Contains($runtimeName)
	}
	Add-Check 'physics.client-runtime-deployment' $physxRuntimeDeploymentValid `
		'Direct Client builds and UpdateLib both deploy all three configuration-matched PhysX runtimes'

	$physicsManagerHeader = Get-Content 'Engine\Public\Physics_Manager.h' -Raw
	$physicsManagerSource = Get-Content 'Engine\Private\Physics_Manager.cpp' -Raw
	$rigidBodyHeader = Get-Content 'Engine\Public\RigidBody.h' -Raw
	$gameInstanceSource = Get-Content 'Engine\Private\GameInstance.cpp' -Raw
	$destructionRuntimeSource = Get-Content 'Client\Private\DestructionSimulationRuntime.cpp' -Raw
	$destructionControllerSource = Get-Content 'Client\Private\DestructionSimulationController.cpp' -Raw
	$deployPropObjectSource = Get-Content 'Client\Private\DeployPropObject.cpp' -Raw
	$physicsContractValid =
		$physicsManagerHeader -match 'PHYSICS_ACTOR_HANDLE' -and
		$physicsManagerHeader -match 'Simulate_DebugSteps\(uint32_t' -and
		$physicsManagerHeader -notmatch '#include\s*[<\"]Px' -and
		$physicsManagerSource -match 'simulate\(CPhysics_Manager::FIXED_TIMESTEP\)' -and
		$physicsManagerSource -match 'fetchResults\(true\)' -and
		$rigidBodyHeader -match 'Create_Runtime' -and
		$gameInstanceSource -match 'm_pPhysics_Manager->Update\(fTimeDelta\)' -and
		$gameInstanceSource -match 'Post_Physics_Update\(fTimeDelta\)' -and
		$destructionRuntimeSource -match 'Begin_PhysicsPreview' -and
		$destructionRuntimeSource -match 'Simulate_DebugSteps' -and
		$destructionControllerSource -match 'Advance_Timeline\(FIXED_DELTA_SECONDS' -and
		$destructionControllerSource -match 'Simulate_PhysicsSteps\(1u' -and
		$destructionControllerSource -match 'Post_Physics_Update\(status\)'
	Add-Check 'physics.fixed-step-destruction-preview' $physicsContractValid `
		'handle facade; no public PhysX include; one paused 1/60 clock; Deploy pose pull'

	$simulationPath = 'Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.destructionsimulation.json'
	$worldEventsPath = 'Data\Encounters\Valtan\ValtanWorldEvents.json'
	$simulation = Read-Json $simulationPath
	$worldEvents = Read-Json $worldEventsPath
	$profiles = @($simulation.profiles)
	$simulationErrors = [Collections.Generic.List[string]]::new()
	$profileIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$staticWallAssets = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	@('DEPLOY_ITR_02306', 'DEPLOY_ITR_02307', 'DEPLOY_ITR_02308', 'DEPLOY_ITR_02309',
		'DEPLOY_ITR_02310', 'DEPLOY_ITR_02311', 'DEPLOY_ITR_02315', 'DEPLOY_ITR_02316') |
		ForEach-Object { [void]$staticWallAssets.Add($_) }
	$deployPlacementLines = @(Get-Content 'Data\Maps\Authoring\LV_LUT_HEARTRB_ED\LV_LUT_HEARTRB_ED.deployplacements')
	$eligibleWallPlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$deployPlacementPositions = @{}
	foreach ($deployPlacementLine in @($deployPlacementLines | Select-Object -Skip 1)) {
		if ($deployPlacementLine -match '^(?<id>\d+)\s+\d+\s+\d+\s+"[^"]+"\s+"(?<asset>[^"]+)"\s+(?<x>[-+0-9.eE]+)\s+[-+0-9.eE]+\s+(?<z>[-+0-9.eE]+)\s') {
			$placementId = $Matches['id']
			$deployPlacementPositions[$placementId] = @([double]$Matches['x'], [double]$Matches['z'])
			if ($staticWallAssets.Contains($Matches['asset'])) {
				[void]$eligibleWallPlacementIds.Add($placementId)
			}
		}
	}
	$allProjectedPlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
	$inwardEmitterCount = 0
	$arenaCenterX = 156.131
	$arenaCenterZ = -122.385
	$inwardVertical = 0.12
	$inwardPlanarMagnitude = [math]::Sqrt(1.0 - $inwardVertical * $inwardVertical)
	foreach ($profile in $profiles) {
		$elements = @($profile.elements)
		if ($profile.profileId -isnot [string] -or -not $profileIds.Add($profile.profileId) -or
			$profile.groupId -isnot [string] -or $profile.previewGroundEnabled -isnot [bool] -or
			-not (Test-JsonNumber $profile.durationSeconds) -or $elements.Count -lt 1 -or $elements.Count -gt 256) {
			$simulationErrors.Add("profile:$($profile.profileId)")
			continue
		}
		$projectedPlacementIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
		foreach ($element in $elements) {
			$elementFields = @('elementId', 'sourceRuntimePlacementId', 'suppressionAliasPlacementIds',
				'spawnOffset', 'direction', 'speedMetersPerSecond', 'gravityScale', 'lifetimeSeconds', 'trigger')
			$hasExactElementFields = @($element.PSObject.Properties).Count -eq $elementFields.Count -and
				@($elementFields | Where-Object { $element.PSObject.Properties.Name -cnotcontains $_ }).Count -eq 0
			$aliases = @($element.suppressionAliasPlacementIds)
			$aliasesAreArray = $element.suppressionAliasPlacementIds -is [System.Array]
			$direction = @($element.direction)
			$lengthSquared = if ($direction.Count -eq 3 -and @($direction | Where-Object { -not (Test-JsonNumber $_) }).Count -eq 0) {
				[double]$direction[0] * [double]$direction[0] + [double]$direction[1] * [double]$direction[1] + [double]$direction[2] * [double]$direction[2]
			} else { 0.0 }
			if (-not $hasExactElementFields -or -not $aliasesAreArray -or
				$element.elementId -isnot [string] -or -not (Test-Uint64String $element.sourceRuntimePlacementId) -or
				-not $projectedPlacementIds.Add($element.sourceRuntimePlacementId) -or
				[math]::Abs([math]::Sqrt($lengthSquared) - 1.0) -gt 0.001 -or
				-not (Test-JsonNumber $element.speedMetersPerSecond) -or
				[double]$element.speedMetersPerSecond -lt 0.0 -or [double]$element.speedMetersPerSecond -gt 250.0 -or
				-not (Test-JsonNumber $element.gravityScale) -or
				[double]$element.gravityScale -lt 0.0 -or [double]$element.gravityScale -gt 10.0 -or
				-not (Test-JsonNumber $element.lifetimeSeconds) -or
				[double]$element.lifetimeSeconds -le 0.0 -or [double]$element.lifetimeSeconds -gt 60.0) {
				$simulationErrors.Add("element:$($element.elementId)")
			}
			if ([double]$element.speedMetersPerSecond -ne 5.0 -or
				[double]$element.gravityScale -ne 2.0 -or [double]$element.lifetimeSeconds -ne 4.0) {
				$simulationErrors.Add("collapse-policy:$($element.elementId)")
			}
			$sourcePlacementId = [string]$element.sourceRuntimePlacementId
			if (-not $allProjectedPlacementIds.Add($sourcePlacementId)) {
				$simulationErrors.Add("duplicate-placement:$sourcePlacementId")
			}
			$directionIsNumeric = $direction.Count -eq 3 -and
				@($direction | Where-Object { -not (Test-JsonNumber $_) }).Count -eq 0
			if ($deployPlacementPositions.ContainsKey($sourcePlacementId) -and $directionIsNumeric) {
				$position = $deployPlacementPositions[$sourcePlacementId]
				$deltaX = $arenaCenterX - [double]$position[0]
				$deltaZ = $arenaCenterZ - [double]$position[1]
				$planarLength = [math]::Sqrt($deltaX * $deltaX + $deltaZ * $deltaZ)
				if ($planarLength -le 0.001) {
					$simulationErrors.Add("not-inward:$sourcePlacementId")
				} else {
					$expectedDirectionX = $deltaX / $planarLength * $inwardPlanarMagnitude
					$expectedDirectionZ = $deltaZ / $planarLength * $inwardPlanarMagnitude
					if ([math]::Abs([double]$direction[0] - $expectedDirectionX) -gt 0.00001 -or
						[math]::Abs([double]$direction[1] - $inwardVertical) -gt 0.00001 -or
						[math]::Abs([double]$direction[2] - $expectedDirectionZ) -gt 0.00001) {
						$simulationErrors.Add("not-inward:$sourcePlacementId")
					} else {
						$inwardEmitterCount++
					}
				}
			} else {
				$simulationErrors.Add("missing-placement:$sourcePlacementId")
			}
			foreach ($aliasPlacementId in $aliases) {
				if (-not (Test-Uint64String $aliasPlacementId) -or
					-not $projectedPlacementIds.Add([string]$aliasPlacementId)) {
					$simulationErrors.Add("alias:$($element.elementId):$aliasPlacementId")
				}
				if (-not $allProjectedPlacementIds.Add([string]$aliasPlacementId)) {
					$simulationErrors.Add("duplicate-placement:$aliasPlacementId")
				}
			}
		}
		$groups = @($worldEvents.groups | Where-Object { $_.groupId -ceq $profile.groupId })
		$members = if ($groups.Count -eq 1) { @($groups[0].memberPlacementIds | ForEach-Object { [string]$_ }) } else { @() }
		if ($groups.Count -ne 1 -or $members.Count -ne $projectedPlacementIds.Count -or
			@(Compare-Object @($projectedPlacementIds | ForEach-Object { $_ }) $members -CaseSensitive).Count -ne 0) {
			$simulationErrors.Add("group:$($profile.groupId)")
		}
	}
	$selectedPlacementId = '17150846598057876717'
	$selectedSuppressionAliasId = '10426387515393336411'
	# Each co-located DEPLOY_ITR_02316 wall contributes exactly one debris emitter
	# plus one suppress-only alias. Adding a wall adds a pair here, never a lone alias.
	$selectedEmitterPairs = @(
		@{ Source = $selectedPlacementId; Alias = $selectedSuppressionAliasId },
		@{ Source = '9863801195242004116'; Alias = '12598937882346321836' })
	$selectedProfile = @($profiles | Where-Object {
		$_.profileId -ceq "destroyable.group.valtan.deploy.$selectedPlacementId.preview" -and
		$_.groupId -ceq "destroyable.group.valtan.deploy.$selectedPlacementId"
	})
	$expectedGroupMembers = [System.Collections.Generic.List[string]]::new()
	$matchedDeployRows = 0
	foreach ($emitterPair in $selectedEmitterPairs) {
		foreach ($pairPlacementId in @($emitterPair.Source, $emitterPair.Alias)) {
			$expectedGroupMembers.Add($pairPlacementId)
			$matchedDeployRows += @($deployPlacementLines | Where-Object {
				$_ -match ("^" + $pairPlacementId + '\s+\d+\s+\d+\s+"[^"]+"\s+"DEPLOY_ITR_02316"\s')
			}).Count
		}
	}
	$selectedElements = if ($selectedProfile.Count -eq 1) { @($selectedProfile[0].elements) } else { @() }
	$selectedEmittersValid = $selectedElements.Count -eq $selectedEmitterPairs.Count
	for ($pairIndex = 0; $selectedEmittersValid -and $pairIndex -lt $selectedEmitterPairs.Count; $pairIndex++) {
		$pairElement = $selectedElements[$pairIndex]
		$pairExpected = $selectedEmitterPairs[$pairIndex]
		if ($pairElement.sourceRuntimePlacementId -cne $pairExpected.Source -or
			@($pairElement.suppressionAliasPlacementIds).Count -ne 1 -or
			$pairElement.suppressionAliasPlacementIds[0] -cne $pairExpected.Alias) {
			$selectedEmittersValid = $false
		}
	}
	$selectedGroup = @($worldEvents.groups | Where-Object {
		$_.groupId -ceq "destroyable.group.valtan.deploy.$selectedPlacementId"
	})
	$selectedGroupMembers = if ($selectedGroup.Count -eq 1) {
		@($selectedGroup[0].memberPlacementIds | ForEach-Object { [string]$_ })
	} else { @() }
	$collapseGroupId = 'destroyable.group.valtan.deploy.11047903315509031966'
	$collapseExpectedMembers = @(
		'11047903315509031966',
		'10619331645164562008',
		'15221142224278623810',
		'10758879797263276529',
		'12385961325933108240',
		'15719065619666776634',
		'16891123250307634887',
		'17769051189408857662',
		'18064647264029106986',
		'18095941116451824308'
	)
	$collapseGroup = @($worldEvents.groups | Where-Object { $_.groupId -ceq $collapseGroupId })
	$collapseProfile = @($profiles | Where-Object { $_.groupId -ceq $collapseGroupId })
	$collapseGroupMembers = if ($collapseGroup.Count -eq 1) {
		@($collapseGroup[0].memberPlacementIds | ForEach-Object { [string]$_ })
	} else { @() }
	$collapseElements = if ($collapseProfile.Count -eq 1) { @($collapseProfile[0].elements) } else { @() }
	$collapseEmitterIds = @($collapseElements | ForEach-Object { [string]$_.sourceRuntimePlacementId })
	$collapseGroupValid =
		$collapseGroup.Count -eq 1 -and $collapseProfile.Count -eq 1 -and
		$collapseGroupMembers.Count -eq $collapseExpectedMembers.Count -and
		$collapseElements.Count -eq $collapseExpectedMembers.Count -and
		@(Compare-Object $collapseExpectedMembers $collapseGroupMembers -CaseSensitive).Count -eq 0 -and
		@(Compare-Object $collapseExpectedMembers $collapseEmitterIds -CaseSensitive).Count -eq 0 -and
		@($collapseElements | Where-Object { @($_.suppressionAliasPlacementIds).Count -ne 0 }).Count -eq 0
	$singleClickGroupId = 'destroyable.group.valtan.deploy.15495684800954131707'
	$singleClickExpectedMembers = @(
		'15495684800954131707',
		'14210737191027038638',
		'14345577340107998055',
		'14439275443581760904',
		'18100758183075339831',
		'15423695113667715748',
		'12804542616476082944',
		'9671293252162296055',
		'15358675764127682739',
		'13290943434399830027',
		'12819104299115621849',
		'17359606121069777624',
		'14520149530921636720'
	)
	$singleClickGroup = @($worldEvents.groups | Where-Object { $_.groupId -ceq $singleClickGroupId })
	$singleClickProfile = @($profiles | Where-Object { $_.groupId -ceq $singleClickGroupId })
	$singleClickGroupMembers = if ($singleClickGroup.Count -eq 1) {
		@($singleClickGroup[0].memberPlacementIds | ForEach-Object { [string]$_ })
	} else { @() }
	$singleClickElements = if ($singleClickProfile.Count -eq 1) { @($singleClickProfile[0].elements) } else { @() }
	$singleClickEmitterIds = @($singleClickElements | ForEach-Object { [string]$_.sourceRuntimePlacementId })
	$singleClickGroupValid =
		$singleClickGroup.Count -eq 1 -and $singleClickProfile.Count -eq 1 -and
		$singleClickGroupMembers.Count -eq $singleClickExpectedMembers.Count -and
		$singleClickElements.Count -eq $singleClickExpectedMembers.Count -and
		@(Compare-Object $singleClickExpectedMembers $singleClickGroupMembers -CaseSensitive).Count -eq 0 -and
		@(Compare-Object $singleClickExpectedMembers $singleClickEmitterIds -CaseSensitive).Count -eq 0 -and
		@($singleClickElements | Where-Object { @($_.suppressionAliasPlacementIds).Count -ne 0 }).Count -eq 0
	$frontCollapseGroupId = 'destroyable.group.valtan.deploy.12252498194881956917'
	$frontCollapseExpectedMembers = @(
		'12252498194881956917',
		'17404480870746281119',
		'13154681496987253568',
		'15393154990485032341',
		'17788407538468874008',
		'10912388091405377926',
		'18362175117092037953',
		'13837284136111935898',
		'15148418014021580024',
		'10435996578153793381',
		'15736179535184396369',
		'17296963925068961735'
	)
	$frontCollapseGroup = @($worldEvents.groups | Where-Object { $_.groupId -ceq $frontCollapseGroupId })
	$frontCollapseProfile = @($profiles | Where-Object { $_.groupId -ceq $frontCollapseGroupId })
	$frontCollapseGroupMembers = if ($frontCollapseGroup.Count -eq 1) {
		@($frontCollapseGroup[0].memberPlacementIds | ForEach-Object { [string]$_ })
	} else { @() }
	$frontCollapseElements = if ($frontCollapseProfile.Count -eq 1) { @($frontCollapseProfile[0].elements) } else { @() }
	$frontCollapseEmitterIds = @($frontCollapseElements | ForEach-Object { [string]$_.sourceRuntimePlacementId })
	$frontCollapseGroupValid =
		$frontCollapseGroup.Count -eq 1 -and $frontCollapseProfile.Count -eq 1 -and
		$frontCollapseGroupMembers.Count -eq $frontCollapseExpectedMembers.Count -and
		$frontCollapseElements.Count -eq $frontCollapseExpectedMembers.Count -and
		@(Compare-Object $frontCollapseExpectedMembers $frontCollapseGroupMembers -CaseSensitive).Count -eq 0 -and
		@(Compare-Object $frontCollapseExpectedMembers $frontCollapseEmitterIds -CaseSensitive).Count -eq 0 -and
		@($frontCollapseElements | Where-Object { @($_.suppressionAliasPlacementIds).Count -ne 0 }).Count -eq 0
	$simulationAuthoringValid =
		$simulation.schema -ceq 'lostark.destruction-simulation' -and (Test-JsonNumber $simulation.formatVersion) -and
		[double]$simulation.formatVersion -eq 2.0 -and $simulation.areaId -ceq 'LV_LUT_HEARTRB_ED' -and
		$profiles.Count -ge 1 -and $profiles.Count -le 128 -and $simulationErrors.Count -eq 0 -and
		$eligibleWallPlacementIds.Count -eq 77 -and $allProjectedPlacementIds.Count -eq 77 -and
		@(Compare-Object @($eligibleWallPlacementIds) @($allProjectedPlacementIds) -CaseSensitive).Count -eq 0 -and
		$inwardEmitterCount -eq 69 -and
		@($worldEvents.bindings | Where-Object { $_.enabled -isnot [bool] -or $_.enabled }).Count -eq 0 -and
		$selectedProfile.Count -eq 1 -and $selectedEmittersValid -and
		$selectedGroup.Count -eq 1 -and
		$selectedGroupMembers.Count -eq $expectedGroupMembers.Count -and
		@(Compare-Object @($expectedGroupMembers) $selectedGroupMembers -CaseSensitive).Count -eq 0 -and
		$matchedDeployRows -eq $expectedGroupMembers.Count -and $collapseGroupValid -and
		$singleClickGroupValid -and $frontCollapseGroupValid
	Add-Check 'maps.valtan-destruction-simulation-authoring' `
		$simulationAuthoringValid `
		"profiles=$($profiles.Count) errors=$($simulationErrors.Count) inward=$inwardEmitterCount/69 coverage=$($allProjectedPlacementIds.Count)/$($eligibleWallPlacementIds.Count) emitters=$($selectedElements.Count) groupMembers=$($selectedGroupMembers.Count) collapse=$($collapseElements.Count)/$($collapseGroupMembers.Count) singleClick=$($singleClickElements.Count)/$($singleClickGroupMembers.Count) frontCollapse=$($frontCollapseElements.Count)/$($frontCollapseGroupMembers.Count) deployRows=$matchedDeployRows"

	# One exact 12-shard recipe per source Deploy wall asset. Runtime admission keys
	# on sourceDeployAssetId, so every cooked asset must stay 1:1 with its receipt.
	# Order must match Get_ProjectAuthoredDebrisModelSpecs so the runtime table and
	# the receipts can be compared 1:1. Every STATIC Valtan wall asset is cooked, so
	# no destructible placement falls back to the generic stone proxies.
	$debrisRecipes = @(
		@{ Asset = 'DEPLOY_ITR_02316'; Triangles = 17731L },
		@{ Asset = 'DEPLOY_ITR_02315'; Triangles = 8867L },
		@{ Asset = 'DEPLOY_ITR_02306'; Triangles = 19833L },
		@{ Asset = 'DEPLOY_ITR_02307'; Triangles = 11397L },
		@{ Asset = 'DEPLOY_ITR_02308'; Triangles = 24985L },
		@{ Asset = 'DEPLOY_ITR_02309'; Triangles = 29832L },
		@{ Asset = 'DEPLOY_ITR_02310'; Triangles = 25747L },
		@{ Asset = 'DEPLOY_ITR_02311'; Triangles = 23774L })
	$recipeValid = $true
	$recipePieces = [System.Collections.Generic.List[object]]::new()
	$triangleTotal = 0L
	foreach ($recipeEntry in $debrisRecipes) {
		$recipe = Read-Json "Data\Maps\Authoring\LV_LUT_HEARTRB_ED\$($recipeEntry.Asset).debrisrecipe.json"
		$pieces = @($recipe.pieces)
		$recipeValid = $recipeValid -and $recipe.schema -ceq 'lostark.deploy-wall-debris-recipe' -and
			(Test-JsonNumber $recipe.formatVersion) -and [double]$recipe.formatVersion -eq 1.0 -and
			$recipe.provenance -ceq 'PROJECT_AUTHORED' -and $recipe.sourceAssetId -ceq $recipeEntry.Asset -and
			[long]$recipe.partition.sourceTriangleCount -eq $recipeEntry.Triangles -and
			$recipe.partition.coverage -ceq 'EVERY_SOURCE_TRIANGLE_EXACTLY_ONCE' -and $pieces.Count -eq 12
		$sourceModelPath = Join-Path $repoRoot ([string]$recipe.sourceFracturedWModel)
		$recipeValid = $recipeValid -and (Test-Path -LiteralPath $sourceModelPath) -and
			(Get-FileHash -LiteralPath $sourceModelPath -Algorithm SHA256).Hash.ToLowerInvariant() -ceq $recipe.sourceSha256
		$assetTriangles = 0L
		for ($pieceIndex = 0; $recipeValid -and $pieceIndex -lt 12; ++$pieceIndex) {
			$piece = $pieces[$pieceIndex]
			$suffix = $pieceIndex.ToString('00', [Globalization.CultureInfo]::InvariantCulture)
			$piecePath = Join-Path $resourceRoot ([string]$piece.assetId)
			$units = @($piece.pivotWModelUnits)
			$meters = @($piece.pivotMetersAtScale1)
			$recipeValid = $piece.pieceId -ceq "piece.$suffix" -and
				$piece.assetId -ceq "Deploy/LV_LUT_HEARTRB_ED/$($recipeEntry.Asset)/fractured/$($recipeEntry.Asset)_CHUNK_$suffix.wmodel" -and
				$piece.sha256 -is [string] -and $piece.sha256 -match '^[0-9a-f]{64}$' -and
				(Test-Path -LiteralPath $piecePath) -and
				(Get-FileHash -LiteralPath $piecePath -Algorithm SHA256).Hash.ToLowerInvariant() -ceq $piece.sha256 -and
				(Test-JsonNumber $piece.triangleCount) -and (Test-JsonNumber $piece.vertexCount) -and
				(Test-JsonNumber $piece.indexCount) -and [long]$piece.indexCount -eq 3L * [long]$piece.triangleCount -and
				$units.Count -eq 3 -and $meters.Count -eq 3
			for ($axis = 0; $recipeValid -and $axis -lt 3; ++$axis) {
				$recipeValid = (Test-JsonNumber $units[$axis]) -and (Test-JsonNumber $meters[$axis]) -and
					[math]::Abs([double]$meters[$axis] - [double]$units[$axis] * 0.01) -le 0.00000001
			}
			$assetTriangles += [long]$piece.triangleCount
			$recipePieces.Add(@{ Piece = $piece; Asset = $recipeEntry.Asset })
		}
		$recipeValid = $recipeValid -and $assetTriangles -eq $recipeEntry.Triangles
		$triangleTotal += $assetTriangles
	}
	Add-Check 'maps.valtan-deploy-wall-debris-recipe' $recipeValid `
		"recipes=$($debrisRecipes.Count) pieces=$($recipePieces.Count) triangles=$triangleTotal"

	$runtimeCode = [regex]::Replace($destructionRuntimeSource, '(?s)/\*.*?\*/', '')
	$runtimeCode = [regex]::Replace($runtimeCode, '(?m)//.*$', '')
	$specFunction = [regex]::Match($runtimeCode, '(?s)Get_ProjectAuthoredDebrisModelSpecs\(\)\s*\{.*?specs\s*=\s*\{(?<Body>.*?)\};\s*return\s+specs\s*;')
	$numberPattern = '[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?'
	$specPattern = '\{\s*L"(?<Tag>[^"]+)"\s*,\s*"(?<Asset>[^"]+)"\s*,\s*(?<Scale>' + $numberPattern +
		')[fF]?\s*,\s*"(?<Source>[^"]+)"\s*,\s*\{\s*(?<X>' + $numberPattern + ')[fF]?\s*,\s*(?<Y>' +
		$numberPattern + ')[fF]?\s*,\s*(?<Z>' + $numberPattern + ')[fF]?\s*\}\s*\}'
	$runtimeSpecs = if ($specFunction.Success) { @([regex]::Matches($specFunction.Groups['Body'].Value, $specPattern)) } else { @() }
	$runtimeValid = $runtimeSpecs.Count -eq $recipePieces.Count -and
		$runtimeCode -match 'const\s+auto&\s+modelSpecs\s*=\s*Get_ProjectAuthoredDebrisModelSpecs\(\)\s*;' -and
		$runtimeCode -match 'spec\.sourceDeployAssetId\s*==\s*runtime\.sourceDeployAssetId'
	for ($specIndex = 0; $runtimeValid -and $specIndex -lt $recipePieces.Count; ++$specIndex) {
		$suffix = ($specIndex % 12).ToString('00', [Globalization.CultureInfo]::InvariantCulture)
		$spec = $runtimeSpecs[$specIndex]
		$expected = $recipePieces[$specIndex]
		$piece = $expected.Piece
		$assetTag = ([string]$expected.Asset).Substring(([string]$expected.Asset).Length - 5)
		$runtimeValid = $spec.Groups['Tag'].Value -ceq "Prototype_Component_Model_DestructionWall_${assetTag}_Chunk$suffix" -and
			$spec.Groups['Asset'].Value -ceq $piece.assetId -and $spec.Groups['Source'].Value -ceq $expected.Asset -and
			[single][double]$spec.Groups['Scale'].Value -eq [single]1.0
		$axes = @('X', 'Y', 'Z')
		for ($axis = 0; $runtimeValid -and $axis -lt 3; ++$axis) {
			$runtimeValid = [single][double]$spec.Groups[$axes[$axis]].Value -eq [single][double]$piece.pivotMetersAtScale1[$axis]
		}
	}
	Add-Check 'maps.valtan-deploy-wall-debris-runtime-registration' $runtimeValid "activeSpecs=$($runtimeSpecs.Count)"

	$aliasRestoreCalls = @([regex]::Matches(
		$runtimeCode, 'Restore_SuppressionAliases\(runtime')).Count
	$sourceSuppressionPredicates = @([regex]::Matches(
		$deployPropObjectSource,
		'm_bPhysicsPreviewActive\s*&&\s*m_bDebrisPreviewActive\s*&&\s*m_bDebrisSuppressSource')).Count
	$aliasLifecycleValid =
		$runtimeCode -match 'aliasObject->Is_StaticDeployModel\(\)' -and
		$runtimeCode -match 'alias\.ePreviousState\s*=\s*alias\.pObject->Get_State\(\)' -and
		$runtimeCode -match 'alias\.pObject->Set_State\(DEPLOY_PROP_STATE::DESPAWNED\)' -and
		$runtimeCode -match 'alias->pObject->Set_State\(alias->ePreviousState\)' -and
		$runtimeCode -match 'if\s*\(!Destroy_Actors\(&restoreStatus\)\)' -and
		$runtimeCode -match 'if\s*\(!Destroy_Actors\(&outStatus\)\)' -and
		$runtimeCode -match 'runtime\.eState\s*=\s*DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED' -and
		$aliasRestoreCalls -ge 3 -and $sourceSuppressionPredicates -eq 3
	Add-Check 'maps.valtan-destruction-suppression-alias-lifecycle' `
		$aliasLifecycleValid `
		"restoreCalls=$aliasRestoreCalls sourcePredicates=$sourceSuppressionPredicates"

	$worldPublisherSource = Get-Content 'Tools\WorldPipeline\Publish-WorldGameplay.ps1' -Raw
	$destroyableProductGateClosed =
		$worldPublisherSource -match "placement\.kind -notin @\('playerSpawn','npc','boss','triggerBox','collisionBox'\)" -and
		$worldPublisherSource -notmatch "placement\.kind -eq 'destroyable'"
	Add-Check 'world.destroyable-product-gate' $destroyableProductGateClosed `
		'MapTool physics authoring is available; product destroyable admission remains fail-closed'
	$singleAreaContractErrors = [Collections.Generic.List[string]]::new()
	$exclusiveRuntimeAreaIds = @(
		'LV_LOBBY_CLASSSELECT_SL00',
		'LV_SHS_RCARENA_D'
	)
	$exclusiveRuntimeAreas = @($mapCatalog.areas | Where-Object {
		$_.id -in $exclusiveRuntimeAreaIds
	})
	if ($exclusiveRuntimeAreas.Count -ne $exclusiveRuntimeAreaIds.Count) {
		$singleAreaContractErrors.Add('missing extracted single-area contract')
	}
	foreach ($area in $exclusiveRuntimeAreas) {
		$catalogLines = @(Get-Content -LiteralPath $area.catalog -Encoding utf8)
		$catalogHeaderMatch = if ($catalogLines.Count -gt 0) {
			[regex]::Match($catalogLines[0], '^LOSTARK_MAP_ASSET_CATALOG\s+\d+\s+"(?<Area>[^"]+)"\s+(?<Count>\d+)$')
		} else { $null }
		if ($null -eq $catalogHeaderMatch -or -not $catalogHeaderMatch.Success -or
			$catalogHeaderMatch.Groups['Area'].Value -ne $area.id -or
			($area.PSObject.Properties.Name -contains 'assetCount' -and
				[int]$catalogHeaderMatch.Groups['Count'].Value -ne [int]$area.assetCount)) {
			$singleAreaContractErrors.Add("$($area.id): catalog header/count")
			continue
		}

		$runtimePrefix = [string]$area.runtimeAssetRoot
		$runtimePrefix = $runtimePrefix.Replace('\', '/').TrimEnd('/')
		$modelPaths = [Collections.Generic.List[string]]::new()
		$catalogAssetKeys = [Collections.Generic.List[string]]::new()
		foreach ($line in @($catalogLines | Select-Object -Skip 1)) {
			if ($line -notmatch '^"(?<Asset>[^"]+)"\s+"[^"]+"\s+"(?<Model>[^"]+)"') {
				$singleAreaContractErrors.Add("$($area.id): malformed asset row")
				continue
			}
			$assetId = $Matches.Asset
			$modelPath = $Matches.Model.Replace('\', '/')
			$modelPaths.Add($modelPath)
			if (-not $modelPath.StartsWith($runtimePrefix + '/', [StringComparison]::OrdinalIgnoreCase)) {
				$singleAreaContractErrors.Add("$($area.id): model outside runtimeAssetRoot")
			} else {
				$relativeModel = $modelPath.Substring($runtimePrefix.Length + 1)
				$catalogAssetKeys.Add("$assetId|$relativeModel")
			}
		}

		if ($area.PSObject.Properties.Name -contains 'placements') {
			$placementHeader = Get-Content -LiteralPath $area.placements -Encoding utf8 -TotalCount 1
			$placementHeaderMatch = [regex]::Match($placementHeader, '^LOSTARK_MAP_PLACEMENTS\s+\d+\s+"(?<Area>[^"]+)"\s+(?<Count>\d+)$')
			if (-not $placementHeaderMatch.Success -or
				$placementHeaderMatch.Groups['Area'].Value -ne $area.id -or
				($area.PSObject.Properties.Name -contains 'placementCount' -and
					[int]$placementHeaderMatch.Groups['Count'].Value -ne [int]$area.placementCount)) {
				$singleAreaContractErrors.Add("$($area.id): placement header/count")
			}
		}

		$runtimeDirectory = Join-Path $resourceRoot $runtimePrefix.Replace('/', '\')
		if (Test-Path -LiteralPath $runtimeDirectory -PathType Container) {
			foreach ($modelPath in $modelPaths) {
				$modelFile = Join-Path $resourceRoot $modelPath.Replace('/', '\')
				if (-not (Test-Path -LiteralPath $modelFile -PathType Leaf)) {
					$singleAreaContractErrors.Add("$($area.id): missing runtime model")
					continue
				}

				$textureReferences = @(Get-WModelTextureReferences $modelFile)
				if ($textureReferences.Count -eq 0) {
					$singleAreaContractErrors.Add("$($area.id): model has no material texture")
					continue
				}
				foreach ($textureReference in $textureReferences) {
					if ($textureReference.Contains('\') -or
						$textureReference.Contains(':') -or
						-not $textureReference.StartsWith('Resource/', [StringComparison]::Ordinal)) {
						$singleAreaContractErrors.Add("$($area.id): invalid material texture path")
						continue
					}

					$relativeTexture = $textureReference.Substring('Resource/'.Length)
					$invalidSegments = @($relativeTexture.Split('/') | Where-Object {
						[string]::IsNullOrWhiteSpace($_) -or $_ -eq '.' -or $_ -eq '..'
					})
					if ($invalidSegments.Count -gt 0 -or
						-not $relativeTexture.StartsWith($runtimePrefix + '/', [StringComparison]::OrdinalIgnoreCase)) {
						$singleAreaContractErrors.Add("$($area.id): material outside runtimeAssetRoot")
						continue
					}

					try {
						$textureFile = [IO.Path]::GetFullPath((Join-Path $resourceRoot $relativeTexture.Replace('/', '\')))
					} catch {
						$singleAreaContractErrors.Add("$($area.id): invalid material texture path")
						continue
					}
					$resourcePrefix = $resourceRoot + [IO.Path]::DirectorySeparatorChar
					if (-not $textureFile.StartsWith($resourcePrefix, [StringComparison]::OrdinalIgnoreCase) -or
						-not (Test-Path -LiteralPath $textureFile -PathType Leaf)) {
						$singleAreaContractErrors.Add("$($area.id): unresolved material texture")
					}
				}
			}
			$runtimeManifestPath = Join-Path $runtimeDirectory 'map_asset_runtime_manifest.json'
			if (Test-Path -LiteralPath $runtimeManifestPath -PathType Leaf) {
				$runtimeManifest = Read-Json $runtimeManifestPath
				$runtimeManifestKeys = @($runtimeManifest.assets | ForEach-Object {
					"$($_.assetId)|$(([string]$_.model).Replace('\', '/'))"
				} | Sort-Object)
				$catalogKeys = @($catalogAssetKeys | Sort-Object)
				if ($runtimeManifest.areaId -ne $area.id -or
					[int]$runtimeManifest.assetCount -ne $modelPaths.Count -or
					@($runtimeManifest.assets).Count -ne $modelPaths.Count -or
					($runtimeManifestKeys -join "`n") -ne ($catalogKeys -join "`n")) {
					$singleAreaContractErrors.Add("$($area.id): runtime manifest set/count")
				}
			} else {
				$singleAreaContractErrors.Add("$($area.id): missing runtime manifest")
			}
		} else {
			$singleAreaContractErrors.Add("$($area.id): missing runtime directory")
		}
	}
	Add-Check 'maps.extracted-area-runtime-roots' ($singleAreaContractErrors.Count -eq 0) "errors=$($singleAreaContractErrors.Count)"
	$trainingArea = @($mapCatalog.areas | Where-Object id -eq 'LV_DEV_TRAINING_GROUND')
	$trainingAssetRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapassets') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapassets' | Select-Object -Skip 1).Count
	} else { 0 }
	$trainingPlacementRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapplacements') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapplacements' | Select-Object -Skip 1).Count
	} else { 0 }
	$trainingWorld = Read-Json 'Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json'
	$invalidTrainingSpawns = @($trainingWorld.placements | Where-Object {
		$_.kind -ne 'playerSpawn' -or $null -ne $_.archetypeId })
	Add-Check 'maps.training-area-contract' (
		$trainingArea.Count -eq 1 -and
		$trainingArea[0].assetCount -eq 10 -and
		$trainingArea[0].placementCount -eq 18 -and
		$trainingAssetRows -eq 10 -and
		$trainingPlacementRows -eq 18 -and
		@($trainingWorld.placements).Count -eq 4 -and
		$invalidTrainingSpawns.Count -eq 0 -and
		(Test-Path -LiteralPath 'Client\Bin\DataFiles\Navigation\LV_DEV_TRAINING_GROUND.navgrid')) "assets=$trainingAssetRows placements=$trainingPlacementRows spawns=$(@($trainingWorld.placements).Count)"
	$characterSelectArea = @($mapCatalog.areas |
		Where-Object id -eq 'LV_LOBBY_CLASSSELECT_SL00')
	$characterSelectAssetRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets' | Select-Object -Skip 1).Count
	} else { 0 }
	$characterSelectPlacementRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapplacements' | Select-Object -Skip 1).Count
	} else { 0 }
	$characterSelectRuntimeRoot = if ($characterSelectArea.Count -eq 1) {
		[string]$characterSelectArea[0].runtimeAssetRoot
	} else { '' }
	$characterSelectManifestPath = if ([string]::IsNullOrWhiteSpace($characterSelectRuntimeRoot)) {
		''
	} else {
		Join-Path $resourceRoot `
			(Join-Path $characterSelectRuntimeRoot 'map_asset_runtime_manifest.json')
	}
	$characterSelectManifest = if (-not [string]::IsNullOrWhiteSpace($characterSelectManifestPath) -and
		(Test-Path -LiteralPath $characterSelectManifestPath)) {
		Read-Json $characterSelectManifestPath
	} else { $null }
	Add-Check 'maps.character-select-area-contract' (
		$characterSelectArea.Count -eq 1 -and
		$characterSelectArea[0].kind -eq 'product' -and
		$characterSelectArea[0].catalogType -eq 'single' -and
		$characterSelectArea[0].assetCount -eq 55 -and
		$characterSelectArea[0].placementCount -eq 803 -and
		$characterSelectAssetRows -eq 55 -and
		$characterSelectPlacementRows -eq 803 -and
		$null -ne $characterSelectManifest -and
		$characterSelectManifest.areaId -eq 'LV_LOBBY_CLASSSELECT_SL00' -and
		$characterSelectManifest.assetCount -eq 55 -and
		@($characterSelectManifest.assets).Count -eq 55) "assets=$characterSelectAssetRows placements=$characterSelectPlacementRows manifest=$($characterSelectManifest.assetCount)"
	Add-Check 'resource.no-lol-annie' (
		-not (Test-Path -LiteralPath (Join-Path $resourceRoot 'Map\LoL\Annie'))) 'legacy Annie resources are quarantined outside the repository'

	$mapPublishPassed = $false
	$mapPublishDetail = ''
	$mapFixtureRoot = Join-Path $repoRoot ".codex_tmp\MapPublishFixture-$PID"
	try {
		$mapFixtureAuthoring = Join-Path $mapFixtureRoot 'Data\Maps\Authoring\FIXTURE'
		$mapFixtureImported = Join-Path $mapFixtureRoot 'Data\Maps\Imported\FIXTURE'
		$mapFixtureRuntime = Join-Path $mapFixtureRoot 'Client\Bin\DataFiles\Map'
		[IO.Directory]::CreateDirectory($mapFixtureAuthoring) | Out-Null
		[IO.Directory]::CreateDirectory($mapFixtureImported) | Out-Null
		[IO.Directory]::CreateDirectory($mapFixtureRuntime) | Out-Null
		$fixtureUtf8 = [Text.UTF8Encoding]::new($false)
		$fixtureMapCatalog = [ordered]@{
			schema = 'lostark.map-catalog'
			formatVersion = 1
			areas = @([ordered]@{
				id = 'FIXTURE'
				kind = 'development'
				catalogType = 'shard-set'
				sourceCatalog = 'Data/Maps/Imported/FIXTURE/FIXTURE.mapset'
				sourcePlacements = 'Data/Maps/Authoring/FIXTURE/FIXTURE.mapplacements'
				catalog = 'Client/Bin/DataFiles/Map/FIXTURE.mapset'
			})
		}
		[IO.File]::WriteAllText(
			(Join-Path $mapFixtureRoot 'Data\Maps\MapCatalog.json'),
			($fixtureMapCatalog | ConvertTo-Json -Depth 8),
			$fixtureUtf8)
		$rowA = '1 "source:a" "A" "editor" "ASSET_A" 0 0 0 0 0 0 1 1 1 1 1'
		$rowB = '2 "source:b" "B" "editor" "ASSET_B" 0 0 0 0 0 0 1 1 1 1 1'
		$rowNew = '3 "source:new" "A" "editor" "ASSET_A" 1 0 0 0 0 0 1 1 1 1 1'
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'FIXTURE.mapset'), @(
			'LOSTARK_MAP_SHARD_SET 1 "FIXTURE" 2',
			'"A" "A.mapassets" "A.mapplacements" 1 1',
			'"B" "B.mapassets" "B.mapplacements" 1 1'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'A.mapassets'), @(
			'LOSTARK_MAP_ASSET_CATALOG 4 "FIXTURE" 1', '"ASSET_A" placeholder'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'B.mapassets'), @(
			'LOSTARK_MAP_ASSET_CATALOG 4 "FIXTURE" 1', '"ASSET_B" placeholder'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'A.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1', $rowA), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'B.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1', $rowB), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowNew), $fixtureUtf8)

		& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 `
			-AreaId FIXTURE `
			-ProjectRoot $mapFixtureRoot | Out-Null
		$publishedMapSet = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'FIXTURE.mapset'))
		$publishedShardA = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'A.mapplacements'))
		$publishedShardB = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'B.mapplacements'))
		$staleMapPublish = @(Get-ChildItem -LiteralPath $mapFixtureRuntime -Force |
			Where-Object Name -Match 'staging|rollback')
		$baselineRuntimeHashes = @{}
		foreach ($runtimeName in @('A.mapplacements', 'B.mapplacements', 'FIXTURE.mapset')) {
			$baselineRuntimeHashes[$runtimeName] =
				(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime $runtimeName)).Hash
		}
		$rowChanged = '3 "source:new" "A" "editor" "ASSET_A" 2 0 0 0 0 0 1 1 1 1 1'
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowChanged), $fixtureUtf8)
		$rollbackRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 `
				-AreaId FIXTURE `
				-ProjectRoot $mapFixtureRoot `
				-FailureAfterPromote 1 | Out-Null
		}
		catch { $rollbackRejected = $true }
		$rollbackPreserved = $true
		foreach ($runtimeName in $baselineRuntimeHashes.Keys) {
			if ($baselineRuntimeHashes[$runtimeName] -ne
				(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime $runtimeName)).Hash) {
				$rollbackPreserved = $false
			}
		}
		$rollbackStale = @(Get-ChildItem -LiteralPath $mapFixtureRuntime -Force |
			Where-Object Name -Match 'staging|rollback')
		$baselineRuntimeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime 'A.mapplacements')).Hash
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, ($rowNew + ' EXTRA')), $fixtureUtf8)
		$malformedRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 -AreaId FIXTURE -ProjectRoot $mapFixtureRoot | Out-Null
		}
		catch { $malformedRejected = $true }
		$malformedPreservedRuntime = $baselineRuntimeHash -eq
			(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime 'A.mapplacements')).Hash

		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowNew), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureImported 'FIXTURE.mapset'), @(
			'LOSTARK_MAP_SHARD_SET 1 "FIXTURE" 2',
			'"A" "..\outside.mapassets" "A.mapplacements" 1 2',
			'"B" "B.mapassets" "B.mapplacements" 1 1'), $fixtureUtf8)
		$escapeRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 -AreaId FIXTURE -ProjectRoot $mapFixtureRoot | Out-Null
		}
		catch { $escapeRejected = $true }
		$mapPublishPassed =
			$publishedMapSet -match '"A" "A\.mapassets" "A\.mapplacements" 1 2' -and
			$publishedMapSet -match '"B" "B\.mapassets" "B\.mapplacements" 1 1' -and
			$publishedShardA -match 'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 2' -and
			$publishedShardB -match 'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1' -and
			$staleMapPublish.Count -eq 0 -and
			$rollbackRejected -and $rollbackPreserved -and
			$rollbackStale.Count -eq 0 -and
			$malformedRejected -and $malformedPreservedRuntime -and
			$escapeRejected
		$mapPublishDetail = "stale=$($staleMapPublish.Count) rollbackRejected=$rollbackRejected rollbackPreserved=$rollbackPreserved malformedRejected=$malformedRejected escapeRejected=$escapeRejected"
	}
	catch {
		$mapPublishDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($mapFixtureRoot)) {
			Remove-Item -LiteralPath $mapFixtureRoot -Recurse -Force
		}
	}
	Add-Check 'maps.sharded-authoring-publish' $mapPublishPassed $mapPublishDetail

	$agentsGuide = Get-Content -LiteralPath 'AGENTS.md' -Raw -Encoding utf8
	$claudeGuide = Get-Content -LiteralPath 'CLAUDE.md' -Raw -Encoding utf8
	$teamReadme = Get-Content -LiteralPath '.md\TEAM\README.md' -Raw -Encoding utf8
	$teamGameplayHandbook = Get-Content -LiteralPath '.md\TEAM\TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md' -Raw -Encoding utf8
	$verticalSliceContractMarker =
		'team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions'
	Add-Check 'team.vertical-slice-ownership' (
		$agentsGuide.Contains($verticalSliceContractMarker) -and
		$claudeGuide.Contains($verticalSliceContractMarker) -and
		$teamReadme.Contains($verticalSliceContractMarker) -and
		$teamGameplayHandbook.Contains($verticalSliceContractMarker)) 'team roles define authority interfaces while feature owners may implement required Data/Shared/Server/Client/harness slices'

    $clientItems = Get-ProjectItems 'Client\Default\Client.vcxproj'
    $engineItems = Get-ProjectItems 'Engine\Default\Engine.vcxproj'
    $serverItems = Get-ProjectItems 'Server\Default\Server.vcxproj'
    $missingProjectItems = [Collections.Generic.List[string]]::new()
    foreach ($pair in @(
        @('Client\Default', $clientItems),
        @('Engine\Default', $engineItems),
        @('Server\Default', $serverItems))) {
        $projectDirectory = [IO.Path]::GetFullPath($pair[0])
        foreach ($item in $pair[1]) {
            $fullName = [IO.Path]::GetFullPath((Join-Path $projectDirectory $item.Include))
            if (-not (Test-Path -LiteralPath $fullName)) {
                $missingProjectItems.Add($fullName)
            }
        }
    }
    Add-Check 'projects.registered-files-exist' ($missingProjectItems.Count -eq 0) "missing=$($missingProjectItems.Count)"

    [xml]$clientFilters = Get-Content -LiteralPath 'Client\Default\Client.vcxproj.filters'
    [xml]$engineFilters = Get-Content -LiteralPath 'Engine\Default\Engine.vcxproj.filters'
    [xml]$serverFilters = Get-Content -LiteralPath 'Server\Default\Server.vcxproj.filters'
    Add-Check 'projects.filters-xml' ($null -ne $clientFilters.Project -and $null -ne $engineFilters.Project -and $null -ne $serverFilters.Project) 'XML parsed'

	$expectedDataIncludes = @(& git ls-files --cached --others --exclude-standard -- Data |
		Where-Object { Test-Path -LiteralPath $_ } |
		ForEach-Object { '..\..\' + $_.Replace('/', '\') } | Sort-Object)
	$clientDataItems = @($clientItems | Where-Object Include -Like '..\..\Data\*')
	$actualDataIncludes = @($clientDataItems | ForEach-Object Include | Sort-Object)
	$filterManager = [Xml.XmlNamespaceManager]::new($clientFilters.NameTable)
	$filterManager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
	$clientDataFilterItems = @($clientFilters.SelectNodes('//m:None', $filterManager) |
		Where-Object Include -Like '..\..\Data\*')
	$invalidDataFilters = @($clientDataFilterItems | Where-Object {
		$null -eq $_.Filter -or -not $_.Filter.StartsWith('96.DataFiles') })
	Add-Check 'projects.data-source-visibility' (
		$clientDataItems.Count -eq $expectedDataIncludes.Count -and
		($actualDataIncludes -join "`n") -eq ($expectedDataIncludes -join "`n") -and
		@($clientDataItems | Where-Object LocalName -ne 'None').Count -eq 0 -and
		$clientDataFilterItems.Count -eq $expectedDataIncludes.Count -and
		$invalidDataFilters.Count -eq 0) "expected=$($expectedDataIncludes.Count) project=$($clientDataItems.Count) filters=$($clientDataFilterItems.Count)"

    $activeSourceRoots = @('Client\Private', 'Client\Public', 'Engine\Private', 'Engine\Public')
    $activeFiles = @(Get-ChildItem -Path $activeSourceRoots -Recurse -File |
        Where-Object Extension -in @('.cpp', '.h'))
    $clientSourceFiles = @(Get-ChildItem -Path @('Client\Private', 'Client\Public') -Recurse -File |
        Where-Object Extension -in @('.cpp', '.h'))
    $forbiddenSymbols = 'CCookedModel|CBinaryAssetObject|\bCMonster\b|CVIBuffer_(Cube|Terrain|Instance_Point|Instance_Rect)|LEVEL_(ASSET_TEST|TEST_LEVEL2|BAREN|GAMEPLAY|MAINMENU|LOGO|CHAOS)'
    $legacyHits = @($activeFiles | Select-String -Pattern $forbiddenSymbols -CaseSensitive)
    Add-Check 'source.legacy-symbols' ($legacyHits.Count -eq 0) "hits=$($legacyHits.Count)"
	Add-Check 'source.no-course-sample-root' (
		-not (Test-Path -LiteralPath 'astar') -and
		-not (Test-Path -LiteralPath '99.수업') -and
		-not (Test-Path -LiteralPath '_work\resource-layout-backup\legacy-map-packs\dev-lol-annie')) 'course and legacy smoke samples are quarantined outside the repository'

	$effectDocumentHeader = Get-Content -LiteralPath 'Client\Public\Effect_AuthoringDocument.h' -Raw
	$effectToolHeader = Get-Content -LiteralPath 'Client\Public\Effect_Tool.h' -Raw
	$effectToolSource = Get-Content -LiteralPath 'Client\Private\Effect_Tool.cpp' -Raw
	$effectMeshPreviewShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl' -Raw
	$effectRectPreviewShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl' -Raw
	$mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
	$clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw
	$engineImGuiSource = Get-Content -LiteralPath 'Engine\Private\ImGuiLayer.cpp' -Raw
	$clientProjectSource = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$removedEffectPaths = @(
		'Client\Public\Effect_Types.h',
		'Client\Public\Effect_AssetIO.h',
		'Client\Private\Effect_AssetIO.cpp',
		'Client\Public\Effect_ParticleSimulator.h',
		'Client\Private\Effect_ParticleSimulator.cpp',
		'Client\Public\Effect_Runtime.h',
		'Client\Private\Effect_Runtime.cpp',
		'Client\Public\Effect_ResourceCatalog.h',
		'Client\Private\Effect_ResourceCatalog.cpp',
		'Client\Public\Effect_Preview.h',
		'Client\Private\Effect_Preview.cpp')
	$removedEffectPathHits = @($removedEffectPaths |
		Where-Object { Test-Path -LiteralPath $_ })
	$authoredEffectFiles = @(Get-ChildItem -LiteralPath 'Data\Effects\Authored' -Recurse -File -ErrorAction SilentlyContinue)
	$unexpectedAuthoredEffectFiles = @($authoredEffectFiles |
		Where-Object { $_.Name -notmatch '^[A-Za-z0-9_.-]+\.effect\.json$' })
	$effectIntakeFiles = @(Get-ChildItem -LiteralPath 'Tools\EffectResourceIntake' -Recurse -File -ErrorAction SilentlyContinue)
	$effectShaderFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\ShaderFiles' -File -Filter 'Shader_Effect*' -ErrorAction SilentlyContinue)
	$legacyEffectSymbolHits = @($clientSourceFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
	$legacyEffectProjectHits = @($clientProjectSource | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)')
	$legacyEffectEntry =
		$clientEntrySource -match 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC|--effect-' -or
		$engineImGuiSource -match '--effect-'
	$effectPassOrderPattern =
		'pass\s+OpaqueBackDepthWrite[\s\S]*pass\s+AlphaTwoSidedDepthRead[\s\S]*pass\s+AdditiveTwoSidedDepthRead'
	$effectG6DetailPreviewShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*12u' -and
		$effectDocumentHeader -match 'struct EFFECT_SOURCE_MATERIAL_DESC[\s\S]*DynamicParameterSemantics[\s\S]*strSubUVMode' -and
		$effectDocumentHeader -match 'struct EFFECT_PARTICLE_SYSTEM_DESC[\s\S]*fUniformScaleMultiplier[\s\S]*fDirectionYawDegrees[\s\S]*fInitialSpeedMultiplier' -and
		$effectDocumentHeader -match 'EFFECT_AUTHORING_MIN_SUPPORTED_VERSION\s*=\s*3u' -and
		$effectDocumentHeader -match 'struct EFFECT_ELEMENT_DESC[\s\S]*ResourceBindings[\s\S]*Material[\s\S]*Detail' -and
		$effectDocumentHeader -match 'struct EFFECT_DETAIL_DESC[\s\S]*Transform[\s\S]*Color[\s\S]*UV[\s\S]*Timing[\s\S]*Mesh[\s\S]*Sprite[\s\S]*Decal[\s\S]*LinearLerp[\s\S]*Particle[\s\S]*Trail[\s\S]*AfterImage' -and
		$effectDocumentHeader -notmatch 'filesystem|DataJson|Parse_EffectDocument|Serialize_EffectDocument' -and
		$effectToolHeader -match 'weak_ptr<CEffectObject>\s+m_pWorldPreviewObject' -and
		$effectToolHeader -match 'Render_EffectDetailWindow' -and
		$effectToolHeader -match 'Render_ModelViewWindow' -and
		$effectToolSource -match 'Try_CommitDocument' -and
		$effectToolSource -match 'Stage_WorldPreview' -and
		$effectToolSource -match 'Render_ResourceGrid' -and
		$effectToolSource -match 'Render_SourceRecipeDetail' -and
		$effectToolSource -match 'duplicate classes execute in source order'
	$effectG6DetailPreviewShape =
		$effectG6DetailPreviewShape -and
		$mainAppSource -match 'make_unique<CEffect_Tool>\(\s*m_pDevice,\s*m_pContext,\s*m_pCharacterPreviewPanel\s*\)' -and
		$mainAppSource -match 'Prototype_GameObject_EffectObject' -and
		$effectMeshPreviewShader -match $effectPassOrderPattern -and
		$effectRectPreviewShader -match $effectPassOrderPattern -and
		(Test-Path -LiteralPath 'Client\Private\Effect_Playback.cpp') -and
		(Test-Path -LiteralPath 'Client\Private\Effect_DocumentRenderer.cpp') -and
		(Test-Path -LiteralPath 'Client\Private\Effect_PresentationService.cpp') -and
		(Test-Path -LiteralPath 'Tools\EffectPipeline\Publish-Effects.ps1') -and
		$clientProjectSource -match 'Shader_VtxEffectMeshPreview\.hlsl' -and
		$clientProjectSource -match 'Shader_VtxEffectRectPreview\.hlsl' -and
		$clientProjectSource -match 'Shader_VtxEffectParticle\.hlsl' -and
		$clientProjectSource -match 'Effect_DocumentRenderer\.cpp' -and
		-not (Test-Path -LiteralPath 'Client\Private\Effect_AuthoringDocument.cpp')
	Add-Check 'effect.g09-authoring-world-runtime-boundary' (
		$removedEffectPathHits.Count -eq 0 -and
		$unexpectedAuthoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -le 1 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG6DetailPreviewShape) "paths=$($removedEffectPathHits.Count) authoredUnexpected=$($unexpectedAuthoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry detailPreview=$effectG6DetailPreviewShape"
	$effectFinalAuditPassed = $false
	$effectFinalAuditDetail = ''
	try {
		$effectFinalAuditDetail = (& .\Tools\ProjectAudit\Test-EffectToolFinal.ps1 `
			-ResourceRoot $resourceRoot 2>&1) -join ' '
		$effectFinalAuditPassed = $true
	}
	catch {
		$effectFinalAuditDetail = $_.Exception.Message
	}
	Add-Check 'effect.g09-cross-document-contract' $effectFinalAuditPassed $effectFinalAuditDetail
	$artistSourceContractPassed = $false
	$artistSourceContractDetail = ''
	try {
		$artistSourceContractDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470SourceContract.ps1' `
			2>&1 | Out-String).Trim()
		$artistSourceContractPassed =
			$artistSourceContractDetail -match
			'PASS: Artist F 31470 Source Contract cues=7 elements=35'
	}
	catch {
		$artistSourceContractDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-source-contract' `
		$artistSourceContractPassed `
		$artistSourceContractDetail
	$artistSourceExecutionPassed = $false
	$artistSourceExecutionDetail = ''
	try {
		$savedErrorActionPreference = $ErrorActionPreference
		try {
			$ErrorActionPreference = 'Continue'
			$artistSourceExecutionDetail = (& `
				'.\Tools\ProjectAudit\Test-Artist31470SourceExecutionSemantics.ps1' `
				*>&1 | Out-String -Width 4096).Trim()
			$artistSourceExecutionInvocationPassed = $?
		}
		finally {
			$ErrorActionPreference = $savedErrorActionPreference
		}
		$artistSourceExecutionPassed =
			$artistSourceExecutionInvocationPassed -and
			$artistSourceExecutionDetail -match
			'PASS: Artist F 31470 Source execution mode=shallow modules=399 ready=370 blocked=29'
	}
	catch {
		$artistSourceExecutionDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-source-execution-semantics' `
		$artistSourceExecutionPassed `
		$artistSourceExecutionDetail
	$artistCustomHandlerOraclePassed = $false
	$artistCustomHandlerOracleDetail = ''
	try {
		$savedErrorActionPreference = $ErrorActionPreference
		try {
			$ErrorActionPreference = 'Continue'
			$artistCustomHandlerOracleDetail = (& `
				'.\Tools\ProjectAudit\Test-Artist31470CustomHandlerOracle.ps1' `
				*>&1 | Out-String -Width 4096).Trim()
			$artistCustomHandlerOracleInvocationPassed = $?
		}
		finally {
			$ErrorActionPreference = $savedErrorActionPreference
		}
		$artistCustomHandlerOraclePassed =
			$artistCustomHandlerOracleInvocationPassed -and
			$artistCustomHandlerOracleDetail -match
			'PASS: Artist F 31470 custom handler oracle mode=shallow ready=370 blocked=29 distributionReady=626 distributionBlocked=3 outputOracles=0 ownerless=0 product=false'
	}
	catch {
		$artistCustomHandlerOracleDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-custom-handler-oracle' `
		$artistCustomHandlerOraclePassed `
		$artistCustomHandlerOracleDetail
	$artistSourceOracleAcquisitionPassed = $false
	$artistSourceOracleAcquisitionDetail = ''
	try {
		$artistSourceOracleAcquisitionDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470SourceOracleAcquisition.ps1' `
			| Out-String).Trim()
		$artistSourceOracleAcquisitionPassed =
			$artistSourceOracleAcquisitionDetail -match
			'PASS: Artist F 31470 Source oracle acquisition mode=shallow classes=15 families=7 blocked=29 providers=0 pilots=0 vss=permission-unchecked nextStage=NO-GO product=false'
	}
	catch {
		$artistSourceOracleAcquisitionDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-source-oracle-acquisition' `
		$artistSourceOracleAcquisitionPassed `
		$artistSourceOracleAcquisitionDetail
	$artistReconstructedSourceCapabilityPassed = $false
	$artistReconstructedSourceCapabilityDetail = ''
	try {
		$artistReconstructedSourceCapabilityDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedSourceCapability.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedSourceCapabilityPassed =
			$artistReconstructedSourceCapabilityDetail -match
			'PASS: Artist F 31470 reconstructed Source capability mode=shallow families=7 occurrences=29 properties=148 distributions=65 samples=87 unknown=0 ownerless=0 genericFallback=0 sourceExact=0 execution=false product=false'
	}
	catch {
		$artistReconstructedSourceCapabilityDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-source-capability' `
		$artistReconstructedSourceCapabilityPassed `
		$artistReconstructedSourceCapabilityDetail
	$artistMaterialContractPassed = $false
	$artistMaterialContractDetail = ''
	try {
		$artistMaterialContractDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialEvidenceContract.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialContractPassed =
			$artistMaterialContractDetail -match
			'PASS: Artist F 31470 Material evidence mode=shallow recipes=27 occurrences=34'
	}
	catch {
		$artistMaterialContractDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-evidence-contract' `
		$artistMaterialContractPassed `
		$artistMaterialContractDetail
	$artistShaderCachePassed = $false
	$artistShaderCacheDetail = ''
	try {
		$artistShaderCacheDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ShaderCacheOracle.ps1' `
			2>&1 | Out-String).Trim()
		$artistShaderCachePassed =
			$artistShaderCacheDetail -match
			'PASS: Artist F 31470 ShaderCache mode=shallow material=23 recipe=27 mic=25/24'
	}
	catch {
		$artistShaderCacheDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-shader-cache-oracle' `
		$artistShaderCachePassed `
		$artistShaderCacheDetail
	$artistMaterialNativeResourcePassed = $false
	$artistMaterialNativeResourceDetail = ''
	try {
		$artistMaterialNativeResourceDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialNativeResource.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialNativeResourcePassed =
			$artistMaterialNativeResourceDetail -match
			'PASS: Artist F 31470 Material native resource mode=shallow families=23 textures=64/55 exact-bound=17 artist-dds=34/55 lookups=4/7 effective=3 replaced=4 identity-override=1 main-dissolve=2 lookup=0 delta=0'
	}
	catch {
		$artistMaterialNativeResourceDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-native-resource' `
		$artistMaterialNativeResourcePassed `
		$artistMaterialNativeResourceDetail
	$artistMainShaderMapIdentityPassed = $false
	$artistMainShaderMapIdentityDetail = ''
	try {
		$artistMainShaderMapIdentityDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MainShaderMapIdentity.ps1' `
			2>&1 | Out-String).Trim()
		$artistMainShaderMapIdentityPassed =
			$artistMainShaderMapIdentityDetail -match
			'PASS: Artist F 31470 main ShaderMap identity mode=shallow families=2 occurrences=3 key=FStaticParameterSet platform=4 map-end=25/25 join=0 dxbc=0 hlsl=false visual=false'
	}
	catch {
		$artistMainShaderMapIdentityDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-main-shader-map-identity' `
		$artistMainShaderMapIdentityPassed `
		$artistMainShaderMapIdentityDetail
	$artistMainOriginalDxbcPassed = $false
	$artistMainOriginalDxbcDetail = ''
	try {
		$artistMainOriginalDxbcDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MainOriginalDxbcReplay.ps1' `
			2>&1 | Out-String).Trim()
		$artistMainOriginalDxbcPassed =
			$artistMainOriginalDxbcDetail -match
			'PASS: Artist F 31470 main original DXBC mode=shallow shaders=2 cases=21 candidate=true occurrence=false hlsl=false visual=false'
	}
	catch {
		$artistMainOriginalDxbcDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-main-original-dxbc-replay' `
		$artistMainOriginalDxbcPassed `
		$artistMainOriginalDxbcDetail
	$artistMainRuntimeSourceReplayPassed = $false
	$artistMainRuntimeSourceReplayDetail = ''
	try {
		$artistMainRuntimeSourceReplayDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MainRuntimeSourceReplay.ps1' `
			2>&1 | Out-String).Trim()
		$artistMainRuntimeSourceReplayPassed =
			$artistMainRuntimeSourceReplayDetail -match
			'Artist 31470 main runtime source replay audit PASS: mode=shallow product=false visual=false'
	}
	catch {
		$artistMainRuntimeSourceReplayDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-main-runtime-source-replay' `
		$artistMainRuntimeSourceReplayPassed `
		$artistMainRuntimeSourceReplayDetail
	$artistWarpEngineStruct = Get-Content -LiteralPath `
		'.\Engine\Public\Engine_Struct.h' -Raw
	$artistWarpEngineMacro = Get-Content -LiteralPath `
		'.\Engine\Public\Engine_Macro.h' -Raw
	$artistWarpGraphicDevice = Get-Content -LiteralPath `
		'.\Engine\Private\Graphic_Device.cpp' -Raw
	$artistWarpGameInstance = Get-Content -LiteralPath `
		'.\Engine\Private\GameInstance.cpp' -Raw
	$artistWarpHarness = Get-Content -LiteralPath `
		'.\Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp' -Raw
	$artistWarpMainApp = Get-Content -LiteralPath `
		'.\Client\Private\MainApp.cpp' -Raw
	$artistWarpRegression = Get-Content -LiteralPath `
		'.\Tools\Build\Invoke-BuildAndRegression.ps1' -Raw
	Add-Check 'effect.artist-31470-main-warp-first-draw-contract' (
		$artistWarpEngineStruct -match
			'D3D_DRIVER_TYPE\s+eDriverType\s*=\s*D3D_DRIVER_TYPE_HARDWARE' -and
		$artistWarpEngineStruct -match
			'bool_t\s+bNonInteractiveErrors\s*=\s*false' -and
		$artistWarpEngineMacro -match
			'MSG_BOX\(_message\)[\s\S]{0,100}Show_EngineMessage' -and
		$artistWarpGraphicDevice -match
			'eDriverType\s*!=\s*D3D_DRIVER_TYPE_HARDWARE[\s\S]{0,100}eDriverType\s*!=\s*D3D_DRIVER_TYPE_WARP' -and
		$artistWarpGraphicDevice -match
			'D3D11CreateDevice\(nullptr,\s*eDriverType' -and
		$artistWarpGraphicDevice -notmatch
			'MSG_BOX\("Failed to Created : CGraphic_Device"\)' -and
		$artistWarpGraphicDevice -match
			'if \(nullptr != m_pDeviceContext\)[\s\S]{0,100}ClearState\(\)' -and
		$artistWarpGameInstance -match
			'const auto FailInitialization[\s\S]{0,300}Release_Engine\(\)[\s\S]{0,200}pOutDevice\.Reset\(\)[\s\S]{0,100}pOutContext\.Reset\(\)' -and
		$artistWarpGameInstance -match
			'if \(nullptr != m_pGraphic_Device\)[\s\S]{0,100}m_pGraphic_Device->Shutdown\(\)' -and
		$artistWarpGameInstance -match
			'm_pShadow\.reset\(\)[\s\S]{0,100}m_pPicking\.reset\(\)[\s\S]{0,100}m_pTarget_Manager\.reset\(\)' -and
		$artistWarpGameInstance -match
			'Set_NonInteractiveErrorMode\(EngineDesc\.bNonInteractiveErrors\)' -and
		$artistWarpGameInstance -match
			'if \(Is_NonInteractiveErrorMode\(\)\)[\s\S]{0,200}OutputDebugStringW[\s\S]{0,200}return IDOK[\s\S]{0,200}MessageBoxW' -and
		$artistWarpMainApp -notmatch
			'D3D_DRIVER_TYPE_WARP' -and
		$artistWarpMainApp -notmatch
			'bNonInteractiveErrors\s*=\s*true' -and
		$artistWarpHarness -match
			'Desc\.eDriverType\s*=\s*D3D_DRIVER_TYPE_WARP' -and
		$artistWarpHarness -match
			'Desc\.bNonInteractiveErrors\s*=\s*true' -and
		$artistWarpHarness -match
			'SCOPED_ENGINE_ERROR_MODE NonInteractiveErrors\(true\)[\s\S]{0,1000}Test_Artist31470CatalogRenderResourceAuthority' -and
		$artistWarpHarness -match
			'AdapterDesc\.VendorId\s*!=\s*0x1414u' -and
		$artistWarpHarness -match
			'AdapterDesc\.DeviceId\s*!=\s*0x008cu' -and
		$artistWarpHarness -match
			'Resolve_ClientWorkingDirectory\(\)' -and
		$artistWarpRegression -match
			'--effect-reconstructed-gpu-material\s+\$effectCatalog'
	) 'production defaults to interactive hardware; initialization rolls back safely; the D/R first-draw harness pins noninteractive Microsoft WARP and Client/Default'
	$artistNativeBaseTexturesPassed = $false
	$artistNativeBaseTexturesDetail = ''
	try {
		$artistNativeBaseTexturesDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470NativeBaseTextures.ps1' `
			2>&1 | Out-String).Trim()
		$artistNativeBaseTexturesPassed =
			$artistNativeBaseTexturesDetail -match
			'PASS: Artist F 31470 native base textures mode=shallow packages=2 assets=4 fresh-dds=4 active-effective=0 runtime-binding=0 product=0'
	}
	catch {
		$artistNativeBaseTexturesDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-native-base-textures' `
		$artistNativeBaseTexturesPassed `
		$artistNativeBaseTexturesDetail
	$artistMaterialRuntimePassed = $false
	$artistMaterialRuntimeDetail = ''
	try {
		$artistMaterialRuntimeDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialRuntimeOracle.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialRuntimePassed =
			$artistMaterialRuntimeDetail -match
			'PASS: Artist F 31470 Material runtime mode=shallow family=23 recipe=27 occurrence=34'
	}
	catch {
		$artistMaterialRuntimeDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-runtime-oracle' `
		$artistMaterialRuntimePassed `
		$artistMaterialRuntimeDetail
	$artistMaterialPolicyPassed = $false
	$artistMaterialPolicyDetail = ''
	try {
		$artistMaterialPolicyDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialReconstructedPolicy.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialPolicyPassed =
			$artistMaterialPolicyDetail -match
			'PASS: Artist F 31470 Material reconstructed policy mode=shallow rows=89\+94\+72/255'
	}
	catch {
		$artistMaterialPolicyDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-reconstructed-policy' `
		$artistMaterialPolicyPassed `
		$artistMaterialPolicyDetail
	$artistMaterialTextureBindingPassed = $false
	$artistMaterialTextureBindingDetail = ''
	try {
		$artistMaterialTextureBindingDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialTextureRuntimeBinding.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialTextureBindingPassed =
			$artistMaterialTextureBindingDetail -match
			'PASS: Artist F 31470 Material texture runtime binding mode=shallow rows=68\+4/72 unique=44\+4/48 proposals=4 product=false'
	}
	catch {
		$artistMaterialTextureBindingDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-texture-runtime-binding' `
		$artistMaterialTextureBindingPassed `
		$artistMaterialTextureBindingDetail
	$artistMaterialRenderResourceApprovalPassed = $false
	$artistMaterialRenderResourceApprovalDetail = ''
	try {
		$artistMaterialRenderResourceApprovalDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470MaterialRenderResourceBindingApproval.ps1' `
			2>&1 | Out-String).Trim()
		$artistMaterialRenderResourceApprovalPassed =
			$artistMaterialRenderResourceApprovalDetail -match
			'PASS: Artist F 31470 Material render-resource approval recipes=27 renderer=57 ambiguous=3 descriptors=27\+18\+1/46 autocrlf=20/20\+check bytes=376183 CR=0 BOM=false'
	}
	catch {
		$artistMaterialRenderResourceApprovalDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-material-render-resource-binding-approval' `
		$artistMaterialRenderResourceApprovalPassed `
		$artistMaterialRenderResourceApprovalDetail
	$artistReconstructedRenderResourceAuthorityPassed = $false
	$artistReconstructedRenderResourceAuthorityDetail = ''
	try {
		$artistReconstructedRenderResourceAuthorityDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedRenderResourceAuthority.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedRenderResourceAuthorityPassed =
			$artistReconstructedRenderResourceAuthorityDetail -match
			'PASS: Artist F 31470 reconstructed render-resource authority resources=48 bindings=72 formats=35\+8\+4\+1 srv=58\+9\+4\+1 colors=67\+5 recipes=27 renderer=57 ambiguous=3 descriptors=27\+18\+1/46 publisher=10/16/25/tool3 bridge=13/21/26/tool3 autocrlf=31/31\+check bytes=746788 CR=0 BOM=false'
	}
	catch {
		$artistReconstructedRenderResourceAuthorityDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-render-resource-authority' `
		$artistReconstructedRenderResourceAuthorityPassed `
		$artistReconstructedRenderResourceAuthorityDetail
	$artistExactDdsDeploymentPassed = $false
	$artistExactDdsDeploymentDetail = ''
	try {
		$artistExactDdsDeploymentDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ExactDdsRuntimeDeployment.ps1' `
			2>&1 | Out-String).Trim()
		$artistExactDdsDeploymentPassed =
			$artistExactDdsDeploymentDetail -match
			'PASS: Artist F 31470 exact DDS runtime deployment mode=shallow assets=4/4 sourceExactMaterial=false r4=false product=false'
	}
	catch {
		$artistExactDdsDeploymentDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-exact-dds-runtime-deployment' `
		$artistExactDdsDeploymentPassed `
		$artistExactDdsDeploymentDetail
	$artistGeometryContractPassed = $false
	$artistGeometryContractDetail = ''
	try {
		& '.\Tools\ProjectAudit\Test-Artist31470WModelGeometryContract.ps1' `
			-Configuration Debug
		$artistGeometryContractPassed = 0 -eq $LASTEXITCODE
		$artistGeometryContractDetail = if ($artistGeometryContractPassed) {
			'focused Debug cooker/EOL/decoder contract PASS'
		}
		else {
			"focused Debug geometry audit exit=$LASTEXITCODE"
		}
	}
	catch {
		$artistGeometryContractDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-wmodel-geometry-contract' `
		$artistGeometryContractPassed `
		$artistGeometryContractDetail
	$artistGeometryBindingPassed = $false
	$artistGeometryBindingDetail = ''
	try {
		$artistGeometryBindingDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470GeometryResourceBinding.ps1' `
			2>&1 | Out-String).Trim()
		$artistGeometryBindingPassed =
			$artistGeometryBindingDetail -match
			'PASS: Artist F 31470 GeometryBinding mode=shallow carriers=7'
	}
	catch {
		$artistGeometryBindingDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-geometry-resource-binding' `
		$artistGeometryBindingPassed `
		$artistGeometryBindingDetail
	$effectCascadeCompilerPassed = $false
	$effectCascadeCompilerDetail = ''
	try {
		$effectCascadeCompilerDetail = (& `
			'.\Tools\ProjectAudit\Test-EffectCascadeCompiler.ps1' `
			2>&1 | Out-String).Trim()
		$effectCascadeCompilerPassed =
			$effectCascadeCompilerDetail -match
			'PASS: non-executable Cascade source-inspection IR'
	}
	catch {
		$effectCascadeCompilerDetail = $_.Exception.Message
	}
	Add-Check 'effect.typed-cascade-compiler' `
		$effectCascadeCompilerPassed `
		$effectCascadeCompilerDetail
	$effectDerivedPublisherPassed = $false
	$effectDerivedPublisherDetail = ''
	try {
		$effectDerivedPublisherDetail = (& `
			'.\Tools\ProjectAudit\Test-EffectDerivedArtifactPublisher.ps1' `
			2>&1 | Out-String).Trim()
		$effectDerivedPublisherPassed =
			$effectDerivedPublisherDetail -match
			'PASS: derived Effect artifact publisher schema tests=28 reserved-reconstructed-id=true current-tools=3 duplicate-json-keys=true duplicate-json-walk=true clean-checkout-lf=true reconstructed-source-id=true authenticated-blocker-union=true bridge=13/21/26/tool3 catalog=102/555 sourceExact=false runtime=false execute=false submit=false render=false product=false rollback=true'
	}
	catch {
		$effectDerivedPublisherDetail = $_.Exception.Message
	}
	Add-Check 'effect.derived-artifact-publisher' `
		$effectDerivedPublisherPassed `
		$effectDerivedPublisherDetail
	$legacyProductProjectionPassed = $false
	$legacyProductProjectionDetail = ''
	try {
		$legacyProductProjectionDetail = (& python -B `
			'.\Tools\EffectPipeline\verify_legacy_product_cue_projection.py' `
			2>&1 | Out-String).Trim()
		$legacyProductProjectionExitCode = $LASTEXITCODE
		$legacyProductProjectionPassed =
			$legacyProductProjectionExitCode -eq 0 -and
			$legacyProductProjectionDetail -match
			'PASS: source=18d2b48920b2a327ac59b572960325d352e77a6f cues=101/101 effects=101/101 components=555/555 old101Delta=0 nonTargetDelta=0'
	}
	catch {
		$legacyProductProjectionDetail = $_.Exception.Message
	}
	Add-Check 'effect.legacy-product-cue-projection' `
		$legacyProductProjectionPassed `
		$legacyProductProjectionDetail
	$effectRuntimeAuthorityPassed = $false
	$effectRuntimeAuthorityDetail = ''
	try {
		$effectRuntimeAuthorityDetail = (& `
			'.\Tools\ProjectAudit\Test-EffectRuntimeAuthority.ps1' `
			2>&1 | Out-String).Trim()
		$effectRuntimeAuthorityPassed =
			$effectRuntimeAuthorityDetail -match
			'PASS: format3 immutable compiled authority'
	}
	catch {
		$effectRuntimeAuthorityDetail = $_.Exception.Message
	}
	Add-Check 'effect.runtime-compiled-authority' `
		$effectRuntimeAuthorityPassed `
		$effectRuntimeAuthorityDetail
	$artistTypedExecutionPlanPassed = $false
	$artistTypedExecutionPlanDetail = ''
	try {
		$artistTypedExecutionPlanDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470TypedExecutionPlan.ps1' `
			2>&1 | Out-String).Trim()
		$artistTypedExecutionPlanPassed =
			$artistTypedExecutionPlanDetail -match
			'PASS: Artist 31470 typed execution plan schedules=7 emitters=35 modules=399 distributions=629 rng=v1 fixedHz=60 Product=false'
	}
	catch {
		$artistTypedExecutionPlanDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-typed-execution-plan' `
		$artistTypedExecutionPlanPassed `
		$artistTypedExecutionPlanDetail
	$artistReconstructedPolicyPassed = $false
	$artistReconstructedPolicyDetail = ''
	try {
		$artistReconstructedPolicyDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedApprovalPolicy.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedPolicyPassed =
			$artistReconstructedPolicyDetail -match
			'PASS: Artist F 31470 reconstructed approval policy tests=41 source=29 material=255 sampler=72 arithmetic=23 geometry=7 sourceExact=false execution=false product=false'
	}
	catch {
		$artistReconstructedPolicyDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-approval-policy' `
		$artistReconstructedPolicyPassed `
		$artistReconstructedPolicyDetail
	$artistReconstructedRuntimeProgramPassed = $false
	$artistReconstructedRuntimeProgramDetail = ''
	try {
		$artistReconstructedRuntimeProgramDetail = (& `
			'.\Tools\ProjectAudit\Test-Artist31470ReconstructedRuntimeProgram.ps1' `
			2>&1 | Out-String).Trim()
		$artistReconstructedRuntimeProgramPassed =
			$artistReconstructedRuntimeProgramDetail -match
			'PASS: Artist F 31470 reconstructed runtime program tests=12 emitters=35 schedules=7 modules=399 properties=1434 leaves=1572 distributions=629 material=23/27/34/255 textures=68/72\+57 geometry=7/13 sourceExact=0 runtime=false product=false'
	}
	catch {
		$artistReconstructedRuntimeProgramDetail = $_.Exception.Message
	}
	Add-Check 'effect.artist-31470-reconstructed-runtime-program' `
		$artistReconstructedRuntimeProgramPassed `
		$artistReconstructedRuntimeProgramDetail
	$effectComponentAuditPassed = $false
	$effectComponentAuditDetail = ''
	$effectSkillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
	$dimensionMasterEffectSkills = @($effectSkillDocument.skills | Where-Object {
		$_.characterClass -eq 'DIMENSIONMASTER' -and
		$_.inputSlot -ne 'SPACE'
	})
	$missingDimensionMasterEffectMappings = @($dimensionMasterEffectSkills |
		Where-Object { [string]::IsNullOrWhiteSpace([string]$_.effectId) })
	try {
		$effectComponentAuditDetail = (& python `
			'.\Tools\LevelPlacementExtractor\build_effect_components.py' `
			'--all-product-classes' `
			'--receipt-root' `
			'.\Data\Effects\AuthoredCorrections\Generated\ComponentBuild' `
			'--verify-existing' 2>&1 | Out-String).Trim()
		if ($LASTEXITCODE -ne 0) {
			throw "WFX verifier exited with code $LASTEXITCODE`: $effectComponentAuditDetail"
		}
		$effectComponentAuditResult = $effectComponentAuditDetail |
			ConvertFrom-Json
		$allProductClassComponentsValid = $true
		foreach ($className in @('DimensionMaster','LanceMaster','Artist','Warlord')) {
			$classAudit = $effectComponentAuditResult.classes.$className
			if ($null -eq $classAudit -or
				-not [bool]$classAudit.compileIdentityComplete -or
				[int]$classAudit.effectCount -le 0 -or
				[int]$classAudit.componentCount -le 0 -or
				[int]$classAudit.emitterCount -le 0) {
				$allProductClassComponentsValid = $false
			}
		}
		$effectComponentAuditPassed =
			$missingDimensionMasterEffectMappings.Count -eq 0 -and
			[bool]$effectComponentAuditResult.compileIdentityComplete -and
			[int]$effectComponentAuditResult.effectCount -gt 0 -and
			[int]$effectComponentAuditResult.componentCount -gt 0 -and
			[int]$effectComponentAuditResult.emitterCount -gt 0 -and
			$allProductClassComponentsValid
	}
	catch {
		$effectComponentAuditDetail = $_.Exception.Message
	}
	$effectComponentAuditDetail =
		"incrementalCompile=true missingDimensionMasterMappings=$($missingDimensionMasterEffectMappings.Count) $effectComponentAuditDetail"
	Add-Check 'effect.wfx-component-assembly' `
		$effectComponentAuditPassed $effectComponentAuditDetail

	$representativeMaterializationPassed = $false
	$representativeMaterializationDetail = ''
	try {
		$representativeMaterializationJson = (& python `
			'.\Tools\EffectPipeline\materialize_representative_authored_baselines.py' `
			2>&1 | Out-String).Trim()
		if ($LASTEXITCODE -ne 0) {
			throw "Representative materializer exited with code $LASTEXITCODE."
		}
		$representativeMaterialization =
			$representativeMaterializationJson | ConvertFrom-Json
		$representativeSkills = @($representativeMaterialization.skills)
		$expectedRepresentatives = @(
			@{ characterClass = 'DIMENSIONMASTER'; skillId = 2050210; targetCount = 1 },
			@{ characterClass = 'LANCE_MASTER'; skillId = 34010; targetCount = 4 },
			@{ characterClass = 'ARTIST'; skillId = 31000; targetCount = 4 },
			@{ characterClass = 'WARLORD'; skillId = 17000; targetCount = 3 }
		)
		$representativeRowsValid = $representativeSkills.Count -eq 4
		foreach ($expected in $expectedRepresentatives) {
			$matches = @($representativeSkills | Where-Object {
				$_.characterClass -eq $expected.characterClass -and
				[int]$_.skillId -eq [int]$expected.skillId
			})
			if ($matches.Count -ne 1 -or
				[int]$matches[0].targetCount -ne [int]$expected.targetCount -or
				[int]$matches[0].materializedTargetCount -ne 0 -or
				$matches[0].status -notin @('preserveExisting', 'blocked')) {
				$representativeRowsValid = $false
			}
		}
		$preservedRepresentatives = @($representativeSkills | Where-Object {
			$_.status -eq 'preserveExisting'
		})
		$blockedRepresentatives = @($representativeSkills | Where-Object {
			$_.status -eq 'blocked'
		})
		$preservedProductGatesValid = @($preservedRepresentatives | Where-Object {
			$productGates = @($_.productGates)
			$productGates.Count -ne [int]$_.targetCount -or
			@($productGates | Where-Object { $_.status -ne 'passed' }).Count -ne 0
		}).Count -eq 0
		$blockedRowsValid = @($blockedRepresentatives | Where-Object {
			@($_.blockers).Count -eq 0 -or
			@($_.productGates).Count -ne 0 -or
			$null -eq $_.sourceDiagnostics
		}).Count -eq 0
		$dimensionMasterRepresentative = @($representativeSkills | Where-Object {
			$_.characterClass -eq 'DIMENSIONMASTER' -and
			[int]$_.skillId -eq 2050210
		})
		$dimensionMasterGate = @()
		if ($dimensionMasterRepresentative.Count -eq 1) {
			$dimensionMasterGate =
				@($dimensionMasterRepresentative[0].productGates)
		}
		$dimensionMasterBaselineValid =
			$dimensionMasterRepresentative.Count -eq 1 -and
			$dimensionMasterRepresentative[0].status -eq 'preserveExisting' -and
			$dimensionMasterGate.Count -eq 1 -and
			$dimensionMasterGate[0].status -eq 'passed' -and
			[int]$dimensionMasterGate[0].elementCount -eq 24 -and
			[int]$dimensionMasterGate[0].occurrenceCount -eq 4 -and
			[int]$dimensionMasterGate[0].kindCounts.mesh -eq 20 -and
			[int]$dimensionMasterGate[0].kindCounts.sprite -eq 4 -and
			[int]$dimensionMasterGate[0].kindCounts.particle -eq 0 -and
			[double]$dimensionMasterGate[0].spriteBillboardRollDegrees -eq -90.0 -and
			[bool]$dimensionMasterGate[0].catalogRegistered -and
			[bool]$dimensionMasterGate[0].exactCueRegistered
		$summary = $representativeMaterialization.summary
		$representativeMaterializationPassed =
			$representativeMaterialization.schema -eq
				'lostark.effect-authored-materialization-status' -and
			[int]$representativeMaterialization.version -eq 1 -and
			$representativeMaterialization.setId -eq
				'representative-four.authored-baselines' -and
			[int]$summary.skillCount -eq 4 -and
			[int]$summary.readyCount -eq 0 -and
			[int]$summary.pendingOutputCount -eq 0 -and
			[int]$summary.preservedCount + [int]$summary.blockedCount -eq 4 -and
			$representativeRowsValid -and
			$preservedProductGatesValid -and
			$blockedRowsValid -and
			$dimensionMasterBaselineValid
		$representativeMaterializationDetail =
			"preserved=$($summary.preservedCount) blocked=$($summary.blockedCount) ready=$($summary.readyCount) pending=$($summary.pendingOutputCount) dmBaseline=$dimensionMasterBaselineValid"
	}
	catch {
		$representativeMaterializationDetail = $_.Exception.Message
	}
	Add-Check 'effect.representative-authored-readiness' `
		$representativeMaterializationPassed `
		$representativeMaterializationDetail

	$fourClassAuthoredRolloutPassed = $false
	$fourClassAuthoredRolloutDetail = ''
	try {
		$fourClassAuthoredRolloutDetail = (& `
			'.\Tools\ProjectAudit\Test-FourClassAuthoredRollout.ps1' `
			2>&1 | Out-String).Trim()
		$fourClassAuthoredRolloutPassed =
			$fourClassAuthoredRolloutDetail -match
			'PASS: Four-class Authored rollout skills=51 stages=74 clips=113 effectBearing=73 silent=1 visualClips=102 derived=48 retained=53 targets=101 cues=101 Particle=0 components=[1-9][0-9]* emitters=[1-9][0-9]*'
	}
	catch {
		$fourClassAuthoredRolloutDetail = $_.Exception.Message
	}
	Add-Check 'effect.four-class-authored-clip-product-exact101' `
		$fourClassAuthoredRolloutPassed `
		$fourClassAuthoredRolloutDetail

    $wrapperHits = @($activeFiles | Select-String -Pattern 'Resources[\\/]LostArk')
    Add-Check 'source.resource-wrapper' ($wrapperHits.Count -eq 0) "hits=$($wrapperHits.Count)"

    $legacyLaunchHits = @($clientSourceFiles | Select-String -Pattern 'CLIENT_SCENARIO|CLIENT_ENTRY_MODE|LOCAL_PREVIEW|CClientLaunchOptions|CLevelCatalog|COfflinePlayerPreview|--smoke|--scenario')
    Add-Check 'source.no-client-runtime-harness' (
        $legacyLaunchHits.Count -eq 0) "legacyRuntimeHits=$($legacyLaunchHits.Count)"

    $readyGaraHits = @($activeFiles | Select-String -Pattern 'Ready_Gara')
    Add-Check 'source.runtime-authoring' ($readyGaraHits.Count -eq 0) "Ready_Gara hits=$($readyGaraHits.Count)"

    $activeCfgFiles = @(Get-ChildItem -Path @('Data', 'Client\Private', 'Client\Public') -Recurse -File -Force |
        Where-Object { $_.Extension.ToLowerInvariant() -eq '.cfg' })
    $cfgReaderHits = @($activeFiles | Select-String -Pattern '\.cfg\b')
    Add-Check 'data.json-only' ($activeCfgFiles.Count -eq 0 -and $cfgReaderHits.Count -eq 0) "cfgFiles=$($activeCfgFiles.Count) readers=$($cfgReaderHits.Count)"

    $animationFiles = @(Get-ChildItem -LiteralPath 'Data\Animation' -Recurse -File -Force)
    $animationLegacyFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\DataFiles\Anim' -File -Force -ErrorAction SilentlyContinue)
    $animationLegacyHits = @($activeFiles | Select-String -Pattern 'DataFiles[\\/]Anim')
    Add-Check 'animation.data-boundary' ($animationFiles.Count -gt 0 -and $animationLegacyFiles.Count -eq 0 -and $animationLegacyHits.Count -eq 0) "data=$($animationFiles.Count) legacyFiles=$($animationLegacyFiles.Count) legacyRefs=$($animationLegacyHits.Count)"

    $functionKeyHits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F([2-5]|[7-9]|1[0-2])\b')
    $f1Hits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F1\b')
    $f6Hits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F6\b')
	$cameraGameplayGateHits = @($clientSourceFiles |
		Select-String -Pattern 'Set_GameplayInputEnabled|m_isGameplayInputEnabled')
    Add-Check 'input.official-function-keys' (
		$functionKeyHits.Count -eq 0 -and
		$f1Hits.Count -eq 1 -and
		$f6Hits.Count -eq 1) "forbidden=$($functionKeyHits.Count) f1=$($f1Hits.Count) f6=$($f6Hits.Count)"
	Add-Check 'input.camera-mode-independent-gameplay' (
		$cameraGameplayGateHits.Count -eq 0) "cameraGameplayGates=$($cameraGameplayGateHits.Count)"

	$lanceLogicSource = Get-Content -LiteralPath 'Client\Private\Logic_LanceMaster.cpp' -Raw
	$characterSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	$modelSource = Get-Content -LiteralPath 'Engine\Private\Model.cpp' -Raw
	$boneSource = Get-Content -LiteralPath 'Engine\Private\Bone.cpp' -Raw
	$obsoletePrRuntimeFiles = @(
		'Client\Private\Level_Test2.cpp',
		'Client\Private\SkillData.cpp',
		'Client\Public\SkillData.h')
	$presentObsoletePrRuntimeFiles = @($obsoletePrRuntimeFiles |
		Where-Object { Test-Path -LiteralPath $_ })
	Add-Check 'integration.pr34-pr35-reconciliation' (
		$presentObsoletePrRuntimeFiles.Count -eq 0 -and
		$lanceLogicSource -notmatch 'Get_DIKeyState|DIK_|Play_Skill' -and
		$characterSource -match 'CLIP_BLEND_SECONDS' -and
		$modelSource -match 'Begin_AnimBlend' -and
		$boneSource -match 'Blend_TransformationMatrix') "obsolete=$($presentObsoletePrRuntimeFiles -join ',') localInput=$($lanceLogicSource -match 'Get_DIKeyState|DIK_|Play_Skill') crossfade=$($characterSource -match 'CLIP_BLEND_SECONDS')"

    $changeLevelHits = @($clientSourceFiles | Select-String -Pattern 'Change_Level\(')
    $unexpectedChangeLevelHits = @($changeLevelHits | Where-Object {
        $_.Path -notlike '*Client\Private\MainApp.cpp' })
    Add-Check 'levels.transition-boundary' (
        $changeLevelHits.Count -eq 2 -and
        $unexpectedChangeLevelHits.Count -eq 0) "calls=$($changeLevelHits.Count) unexpected=$($unexpectedChangeLevelHits.Count)"

    $mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
    $mainAppHeader = Get-Content -LiteralPath 'Client\Public\MainApp.h' -Raw
    $lobbySource = Get-Content -LiteralPath 'Client\Private\Level_Lobby.cpp' -Raw
    $characterSelectSource = Get-Content -LiteralPath 'Client\Private\Level_CharacterSelect.cpp' -Raw
    $levelRegistrySource = Get-Content -LiteralPath 'Client\Private\LevelRegistry.cpp' -Raw
	$renderingProfileServiceSource = Get-Content -LiteralPath `
		'Client\Private\RenderingProfileService.cpp' -Raw
    $loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
    $replicationHeader = Get-Content -LiteralPath 'Client\Public\ClientReplication.h' -Raw
    $replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
    $bernLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Bern.cpp' -Raw
    $valtanLevelSource = Get-Content -LiteralPath 'Client\Private\Level_ValtanArena.cpp' -Raw
    $developmentLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Development.cpp' -Raw
    $clientProjectText = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
    $clientFilterText = Get-Content -LiteralPath 'Client\Default\Client.vcxproj.filters' -Raw
    $clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw

    Add-Check 'levels.lobby-start' (
        $mainAppSource -match 'Start_Level\(LEVEL::LOBBY\)') 'MainApp starts Lobby'
	$characterSelectLoaderFunction = [regex]::Match(
		$loaderSource,
		'HRESULT CLoader::Ready_For_CharacterSelect\(\)[\s\S]*?(?=HRESULT CLoader::Ready_For_Bern\(\))').Value
	$lobbyCommandHeaderSource = Get-Content -LiteralPath 'Client\Public\LobbyCommandService.h' -Raw
	$lobbyCommandSource = Get-Content -LiteralPath 'Client\Private\LobbyCommandService.cpp' -Raw
	$transitionHeaderSource = Get-Content -LiteralPath 'Client\Public\LevelTransitionService.h' -Raw
	$loadingSource = Get-Content -LiteralPath 'Client\Private\Level_Loading.cpp' -Raw
	$frontendHarnessSource = Get-Content -LiteralPath 'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp' -Raw
	$frontendHarnessProject = Get-Content -LiteralPath 'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj' -Raw
	$buildRegressionSource = Get-Content -LiteralPath 'Tools\Build\Invoke-BuildAndRegression.ps1' -Raw
	$characterSelectionStateSource = Get-Content -LiteralPath 'Client\Private\CharacterSelectionState.cpp' -Raw
	$packetTypeSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketType.h' -Raw
	$packetMessagesSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketMessages.h' -Raw
	$gameRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
	$networkManagerSource = Get-Content -LiteralPath 'Client\Private\NetworkManager.cpp' -Raw
	$playerControllerSourceForClassSwitch = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
    Add-Check 'levels.character-select-contract' (
        $levelRegistrySource -match 'LEVEL::CHARACTER_SELECT' -and
		$levelRegistrySource -match 'LV_LOBBY_CLASSSELECT_SL00' -and
		$levelRegistrySource -match '\{ true, true, -792\.f, 158\.f, -750\.f, 218\.f \}' -and
		$levelRegistrySource -match 'Ready_For_CharacterSelect' -and
		$characterSelectLoaderFunction -match 'Ready_MapArea\(' -and
		$characterSelectLoaderFunction -notmatch 'Ready_Camera_Prototype\(' -and
		$characterSelectLoaderFunction -match 'Ready_Character_Rendering\(' -and
		$characterSelectLoaderFunction -notmatch 'Ready_AnimationPreviewModels\(' -and
		$characterSelectLoaderFunction -match 'CCharacterSelectionState::Try_Get_SelectedClass' -and
		$characterSelectLoaderFunction -match 'const std::array characterClasses = \{ initialClass \}' -and
		$characterSelectSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$characterSelectSource -match 'm_MapRuntime\.Load_Area' -and
		$levelRegistrySource -match 'scene\.character-select\.warm-high-key\.v1' -and
		$mainAppSource -match 'm_RenderingProfiles\.Activate_Profile' -and
		$characterSelectSource -match 'CCharacterSelectionState::Select' -and
		$characterSelectSource -match 'MODE::CONNECTING' -and
		$characterSelectSource -notmatch 'MODE::PREVIEW|Stage_Preview|Select_Preview|Ready_Preview' -and
		$characterSelectionStateSource -notmatch 'TestEntryMode|TEST_ENTRY_MODE' -and
		$characterSelectSource -notmatch 'Request_ServerArena\(' -and
		$characterSelectSource -notmatch 'Connect_To_Server\(' -and
		$characterSelectSource -notmatch 'Send_EnterWorld\(' -and
		$characterSelectSource -notmatch 'Try_Consume_EnterAccepted' -and
		$characterSelectSource -match 'CONNECTION_TIMEOUT' -and
		$characterSelectSource -notmatch 'Return_ToPreview\(' -and
		$characterSelectSource -match 'character-select\.server-disconnect' -and
		$lobbyCommandHeaderSource -match 'LOBBY_COMMAND_PURPOSE' -and
		$lobbyCommandHeaderSource -match 'MAP_EDITOR_WORKSPACE' -and
		$characterSelectSource -match 'character-select\.enter-bern' -and
		$characterSelectSource -match 'character-select\.enter-valtan' -and
		$characterSelectSource -notmatch 'ImGui::RadioButton\("Preview"|Server Play \(Lobby-approved\)' -and
		$characterSelectSource -match 'Request_ClassChange' -and
		$characterSelectSource -match 'Request_ChangeCharacterClass' -and
		$characterSelectSource -match 'Try_Consume_CharacterClassChangeResult' -and
		$characterSelectSource -match 'm_iPendingClassChangeSequence' -and
		$characterSelectSource -notmatch 'ImGui::BeginDisabled\(true\)' -and
		$characterSelectSource -match 'ARENA_SPAWN_OPTIONS' -and
		$characterSelectSource -match 'spawn\.character-select\.monster' -and
		$characterSelectSource -match 'spawn\.character-select\.miniboss' -and
		$characterSelectSource -match 'boss\.valtan\.character-select\.lazy' -and
		$characterSelectSource -match 'ImGui::Button\("Spawn Selected"\)' -and
		$characterSelectSource -match 'Show Combat Colliders' -and
		$characterSelectSource -notmatch 'ImGui::Button\("Enter Test"\)' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Bern"\)' -and
		$characterSelectSource -match 'ImGui::Button\("Enter Valtan Map"\)' -and
		$characterSelectSource -match 'CClientReplication' -and
		$characterSelectSource -match 'CNetworkPlayerCommandSink' -and
		$characterSelectSource -match 'CNetworkWorldEntityCommandSink' -and
		$characterSelectSource -match 'm_PlayerController\.Update\(' -and
		$characterSelectSource -match 'm_pCamera->Is_FollowEnabled\(\)' -and
		$characterSelectSource -notmatch 'Ready_ServerGameplayCamera\(\)' -and
		$characterSelectSource -match 'Render_SelectionPanel\(\);' -and
		$lobbySource -match 'DEFAULT_ENTRY_CLASS' -and
		$lobbySource -match 'Resolve_EntryCharacterClass' -and
		$lobbySource -match 'Send_EnterWorld\(' -and
		$lobbySource -match 'case LOBBY_STAGE::CHARACTER_SELECT:[\s\S]{0,180}WORLD_ID::CHARACTER_SELECT_ARENA' -and
		$lobbySource -notmatch 'Stage_TestEntryMode|Clear_TestEntryMode' -and
		$lobbySource -match 'accepted\.iProtocolVersion' -and
		$lobbySource -match 'accepted\.iPlayerId' -and
		$lobbySource -match 'accepted\.iNetEntityId' -and
		$frontendHarnessProject -match 'CharacterSelectionState\.cpp' -and
		$frontendHarnessProject -match 'NetObjectRegistry\.cpp' -and
		$frontendHarnessSource -match 'Test_CharacterSelectAuthorizedSelection' -and
		$frontendHarnessSource -match 'Test_NetObjectRegistryClassReplacement' -and
		$packetTypeSource -match 'NETWORK_PROTOCOL_VERSION = 18' -and
		$packetTypeSource -match 'S2C_ENTER_REJECTED' -and
		$packetTypeSource -match 'C2S_CHANGE_CHARACTER_CLASS' -and
		$packetMessagesSource -match 'PLAYER_SNAPSHOT[\s\S]{0,180}eCharacterClass' -and
		$gameRoomSource -match 'Apply_CharacterClassChange' -and
		$gameRoomSource -match 'snapshot\.eCharacterClass = player\.eCharacterClass' -and
		$networkManagerSource -match 'S2C_CHARACTER_CLASS_CHANGE_RESULT' -and
		$networkManagerSource -match 'Try_Consume_EnterRejected' -and
		$networkManagerSource -match 'case PACKET_TYPE::S2C_ENTER_REJECTED' -and
		$lobbySource -match 'Consume_EnterRejected\(\);' -and
		$lobbySource -match 'Try_Consume_EnterRejected' -and
		$lobbySource -match 'Valtan raid is full \(4/4\)' -and
		$replicationSource -match 'Replace_CharacterClass' -and
		$replicationSource -match 'RECOVERED_FAILURE' -and
		$playerControllerSourceForClassSwitch -match 'Rebind_LocalCharacter' -and
        $lobbySource -match '"Test"' -and
		$lobbySource -match '"Character Select"' -and
		$lobbySource -match '"Valtan"' -and
		$lobbySource -match '"Bern"') 'Character Select requires Lobby Server approval, changes class through a typed Server command, provides Server-only monster/miniboss/Valtan spawn controls, and transactionally replaces replicated presentation'
	Add-Check 'levels.character-select-camera-framing' (
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_SIDE = 0\.4f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_HEIGHT = 7\.5f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_DISTANCE = 4\.5f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_LOOK_HEIGHT = 1\.05f' -and
		$characterSelectSource -match 'CHARACTER_SELECT_CAMERA_FOV_Y = 45\.f' -and
		$characterSelectSource -match 'desc\.vLookOffset = lookOffset' -and
		$characterSelectSource -match 'Bind_CameraTarget\([\s\S]{0,100}CharacterSelectCameraPositionOffset\(\)' -and
		$characterSelectSource -notmatch 'PREVIEW_POSITION|PREVIEW_CAMERA_HEIGHT|SERVER_CAMERA_HEIGHT') 'Character Select Server Arena uses one initial framing preset and rebinds the replicated local target'
	$renderingProfiles = Read-Json `
		'Data\Rendering\Authored\RenderingProfiles.json'
	$renderingProfileIds = @($renderingProfiles.profiles | ForEach-Object {
		[string]$_.profileId
	})
	$unsupportedSceneFields = @($renderingProfiles.profiles |
		ForEach-Object { $_.PSObject.Properties.Name } |
		Where-Object { $_ -notin @(
			'profileId', 'exposureMultiplier',
			'bloomIntensityMultiplier', 'light', 'shadow') })
	Add-Check 'rendering.scene-profile-runtime-contract' (
		$renderingProfiles.schema -eq 'lostark.rendering-profiles' -and
		[int]$renderingProfiles.formatVersion -eq 1 -and
		$renderingProfileIds -contains 'scene.character-select.warm-high-key.v1' -and
		$renderingProfileIds -contains 'scene.valtan.cool-low-key.v1' -and
		$unsupportedSceneFields.Count -eq 0 -and
		$mainAppSource -match 'pRenderingProfileId' -and
		$mainAppSource -match 'Save Authored' -and
		$mainAppSource -match 'Publish Runtime' -and
		$mainAppSource -match 'Reload Runtime' -and
		$renderingProfileServiceSource -match 'OutEffective = GlobalQuality' -and
		$renderingProfileServiceSource -match 'fExposure \* Profile\.fExposureMultiplier' -and
		(Test-Path -LiteralPath 'Tools\RenderingPipeline\Publish-RenderingProfiles.ps1') -and
		(Test-Path -LiteralPath 'Tools\ProjectAudit\Test-RenderingProfiles.ps1')) `
		'descriptor-owned scene profiles use one renderer/light path with authored publish and non-cumulative multipliers'
	$renderingProfileAuditPassed = $false
	$renderingProfileAuditDetail = ''
	try {
		$renderingProfileAuditDetail = (& .\Tools\ProjectAudit\Test-RenderingProfiles.ps1 `
			-RepoRoot (Get-Location).Path 2>&1) -join ' '
		$renderingProfileAuditPassed = $true
	}
	catch {
		$renderingProfileAuditDetail = $_.Exception.Message
	}
	Add-Check 'rendering.profile-parser-contract' `
		$renderingProfileAuditPassed $renderingProfileAuditDetail
	$renderQualityAuditPassed = $false
	$renderQualityAuditDetail = ''
	try {
		$renderQualityAuditDetail = (& .\Tools\ProjectAudit\Test-RenderQualityWorkbench.ps1 `
			-RepoRoot (Get-Location).Path 2>&1) -join ' '
		$renderQualityAuditPassed = $true
	}
	catch {
		$renderQualityAuditDetail = $_.Exception.Message
	}
	Add-Check 'rendering.quality-workbench-contract' `
		$renderQualityAuditPassed $renderQualityAuditDetail
	$pointLightFalloffAuditPassed = $false
	$pointLightFalloffAuditDetail = ''
	try {
		$pointLightFalloffAuditDetail = (& .\Tools\ProjectAudit\Test-PointLightFalloff.ps1 `
			-RepoRoot (Get-Location).Path 2>&1) -join ' '
		$pointLightFalloffAuditPassed = $true
	}
	catch {
		$pointLightFalloffAuditDetail = $_.Exception.Message
	}
	if ([string]::IsNullOrWhiteSpace($pointLightFalloffAuditDetail)) {
		$pointLightFalloffAuditPassed = $false
		$pointLightFalloffAuditDetail =
			'Focused PointLight falloff audit returned no evidence detail.'
	}
	Add-Check 'rendering.point-light-falloff-contract' `
		$pointLightFalloffAuditPassed $pointLightFalloffAuditDetail
	Add-Check 'levels.loading-progress-overlay' (
		$loadingSource -match 'CLoader::Get_ActiveStatus\(\)' -and
		$loadingSource -match '"Loading progress"' -and
		$loadingSource -match 'viewport->WorkPos\.y \+ 16\.f' -and
		$loaderSource -match 'Character %zu/%zu \| %s models %zu/%zu \| %s' -and
		$loaderSource -match 'completedModelCount' -and
		$loaderSource -match 'totalModelCount') 'Loading renders top-screen character class/model counts and the active binary file'
	Add-Check 'levels.character-select-handoff-ticket' (
		$lobbyCommandHeaderSource -match 'LOBBY_COMMAND_TOKEN' -and
		$lobbyCommandHeaderSource -match 'Cancel\(' -and
		$lobbyCommandSource -match 'g_iNextToken' -and
		$lobbyCommandSource -match 'g_PendingCommand->iToken != token' -and
		$transitionHeaderSource -match 'iLobbyCommandToken' -and
		$loadingSource -match 'Request_Activation\([\s\S]{0,160}m_iLobbyCommandToken' -and
		$loadingSource -match 'Cancel_LobbyCommand\("target level loading failed"\)' -and
		$mainAppSource -match 'target level loading could not start' -and
		$mainAppSource -match 'target level activation failed' -and
		$frontendHarnessSource -match 'Exact Cancellation Leaves No Stale Command' -and
		$frontendHarnessSource -match 'Stale Failure Cannot Cancel New Handoff' -and
		$frontendHarnessProject -match 'Client\\Private\\LobbyCommandService\.cpp' -and
		$buildRegressionSource -match 'ClientFrontendHarness') 'token service has an executable unit harness and integration failure hooks are statically admitted'
    Add-Check 'levels.release-imgui-entry' (
        $mainAppSource -match 'ReadyImGuiRuntime\(\)' -and
        $lobbySource -match 'Render_StagePanel\(\);' -and
        $lobbySource -notmatch '#ifdef _DEBUG[\s\S]{0,80}Render_StagePanel' -and
		$characterSelectSource -match 'Render_SelectionPanel\(\);' -and
		$characterSelectSource -notmatch '#ifdef _DEBUG[\s\S]{0,80}Render_SelectionPanel' -and
        $clientEntrySource -match 'CImGuiLayer::HandleWindowMessage' -and
        $clientEntrySource -notmatch '#ifdef _DEBUG\s*\r?\n\s*if \(CImGuiLayer::HandleWindowMessage') 'Lobby and Character Select ImGui are available in Release'

    $deletedRuntimeFiles = @(
        'Client\Public\ClientLaunchOptions.h',
        'Client\Private\ClientLaunchOptions.cpp',
        'Client\Public\LevelCatalog.h',
        'Client\Private\LevelCatalog.cpp',
        'Client\Public\OfflinePlayerPreview.h',
        'Client\Private\OfflinePlayerPreview.cpp',
        'Data\Levels\LevelCatalog.json',
        'Tools\Build\Invoke-OfflineClientSmoke.ps1',
        'Tools\Build\Invoke-NetworkEndpointSmoke.ps1')
    $presentDeletedRuntimeFiles = @($deletedRuntimeFiles | Where-Object {
        Test-Path -LiteralPath $_ })
    Add-Check 'levels.legacy-runtime-removed' (
        $presentDeletedRuntimeFiles.Count -eq 0 -and
        $mainAppHeader -notmatch 'SmokeHarness|OfflinePreview|CLIENT_SCENARIO' -and
        $mainAppSource -notmatch 'SmokeHarness|OfflinePreview|CLIENT_SCENARIO' -and
        $replicationHeader -notmatch 'LOCAL_PREVIEW|LocalPreview' -and
        $replicationSource -notmatch 'LOCAL_PREVIEW|LocalPreview' -and
        $clientProjectText -notmatch 'ClientLaunchOptions|LevelCatalog|OfflinePlayerPreview|SceneTransitionService' -and
        $clientFilterText -notmatch 'ClientLaunchOptions|LevelCatalog|OfflinePlayerPreview|SceneTransitionService') "present=$($presentDeletedRuntimeFiles -join ',')"

    $networkManagerSource = Get-Content -LiteralPath 'Client\Private\NetworkManager.cpp' -Raw
	$networkManagerHeader = Get-Content -LiteralPath 'Client\Public\NetworkManager.h' -Raw
    $serverMainSource = Get-Content -LiteralPath 'Server\Private\Main.cpp' -Raw
	$serverAppHeader = Get-Content -LiteralPath 'Server\Public\ServerApp.h' -Raw
	$tcpListenerHeader = Get-Content -LiteralPath 'Server\Public\TcpListener.h' -Raw
    $tcpListenerSource = Get-Content -LiteralPath 'Server\Private\TcpListener.cpp' -Raw
	$serverProjectText = Get-Content -LiteralPath 'Server\Default\Server.vcxproj' -Raw
	$teamLanEndpoint = Read-Json 'Tools\Network\TeamLanEndpoint.json'
	$teamLanSyncSource = Get-Content -LiteralPath `
		'Tools\Network\Sync-TeamLanEndpoint.ps1' -Raw
	$agentsSource = Get-Content -LiteralPath 'AGENTS.md' -Raw
	$teamLanHostPattern = [regex]::Escape(
		[string]$teamLanEndpoint.serverHost)
	$teamLanBindPattern = [regex]::Escape(
		[string]$teamLanEndpoint.serverBindAddress)
	$teamLanActiveThrough = [DateTimeOffset]::Parse(
		[string]$teamLanEndpoint.activeThroughKst,
		[Globalization.CultureInfo]::InvariantCulture)
	Add-Check 'network.team-lan-session-sync' (
		$teamLanEndpoint.schema -eq 'lostark.team-lan-endpoint' -and
		[int]$teamLanEndpoint.version -eq 1 -and
		[int]$teamLanEndpoint.port -eq 7777 -and
		[string]$teamLanEndpoint.serverBindAddress -eq '0.0.0.0' -and
		[DateTimeOffset]::Now -le $teamLanActiveThrough -and
		$teamLanSyncSource -match 'Set-ProjectUserProperty' -and
		$teamLanSyncSource -match 'Sync-HostFirewall' -and
		$teamLanSyncSource -match "ValidateSet\('Auto', 'Server', 'Client'\)" -and
		$teamLanSyncSource -match 'Machine role: \$effectiveRole' -and
		$teamLanSyncSource -match 'activeThroughKst' -and
		$agentsSource -match 'Sync-TeamLanEndpoint\.ps1' -and
		$agentsSource -match '2026-08-20 23:59 KST') `
		"host=$($teamLanEndpoint.serverHost), activeThrough=$($teamLanActiveThrough.ToString('o'))"
	Add-Check 'network.server-authorized-entry' (
        $networkManagerSource -match 'Connect_To_Server\(' -and
        $networkManagerSource -match 'InetPtonA' -and
		$networkManagerSource -match (
			'DEFAULT_SERVER_HOST\[\] = "' + $teamLanHostPattern + '"') -and
		$networkManagerHeader -match (
			'Connect_To_Server\("' + $teamLanHostPattern + '", port\)') -and
        $serverMainSource -match '--bind-address' -and
		$serverMainSource -match (
			'bindAddress = "' + $teamLanBindPattern + '"') -and
		$serverAppHeader -match (
			'bindAddress = "' + $teamLanBindPattern + '"') -and
		$tcpListenerHeader -match (
			'Open\("' + $teamLanBindPattern + '", port\)') -and
		$serverProjectText -match (
			'<LocalDebuggerCommandArguments>--bind-address ' +
			$teamLanBindPattern + '</LocalDebuggerCommandArguments>') -and
		$clientProjectText -match (
			'<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=' +
			$teamLanHostPattern + '</LocalDebuggerEnvironment>') -and
        $tcpListenerSource -match 'INADDR_ANY' -and
        $tcpListenerSource -match 'INADDR_LOOPBACK' -and
		$networkManagerSource -match 'LOSTARK_SERVER_HOST' -and
		$lobbySource -match 'CNetworkManager::Resolve_ServerHost' -and
		$lobbySource -match 'CNetworkManager::DEFAULT_SERVER_PORT' -and
		$lobbySource -notmatch '192\.168\.' -and
        $lobbySource -match 'Send_EnterWorld\(' -and
        $lobbySource -match 'Try_Consume_EnterAccepted' -and
        $lobbySource -match 'seconds\(5\)' -and
		$bernLevelSource -match 'network\.connection-lost' -and
		$valtanLevelSource -match 'network\.connection-lost' -and
		$developmentLevelSource -match 'network\.connection-lost' -and
		$characterSelectSource -match 'character-select\.server-disconnect' -and
		$characterSelectSource -match 'RETURNING_TO_LOBBY') 'Server approval is mandatory; Character Select consumes the Lobby-approved socket and every product world returns to Lobby on disconnect'

    $playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
    Add-Check 'player.command-sink-boundary' ($playerControllerSource -notmatch 'NetworkManager' -and $playerControllerSource -match 'IPlayerCommandSink') 'PlayerController depends on command sink'
	Add-Check 'player.free-camera-command-gate' (
		$playerControllerSource -match 'Update\(const bool_t gameplayCommandsEnabled\)' -and
		$playerControllerSource -match 'gameplayCommandsEnabled && isRightMouseDown' -and
		$playerControllerSource -match 'suppressKeyboard \|\| !gameplayCommandsEnabled' -and
		$bernLevelSource -match 'm_pCamera->Is_FollowEnabled\(\)' -and
		$valtanLevelSource -match 'm_pCamera->Is_FollowEnabled\(\)' -and
		$developmentLevelSource -match 'camera->Is_FollowEnabled\(\)' -and
		$characterSelectSource -match 'm_pCamera->Is_FollowEnabled\(\)') `
		'free camera synchronizes physical edges but blocks move and skill command submission in every gameplay level'
	$cameraFreeSource = Get-Content -LiteralPath 'Client\Private\Camera_Free.cpp' -Raw
	$cameraBaseSource = Get-Content -LiteralPath 'Engine\Private\Camera.cpp' -Raw
	$gameInstanceSource = Get-Content -LiteralPath 'Engine\Private\GameInstance.cpp' -Raw
	Add-Check 'camera.follow-same-frame-transform' (
		$cameraFreeSource -notmatch '(?s)Priority_Update\(f32_t fTimeDelta\).*?Update_FollowCamera\(fTimeDelta\).*?void CCamera_Free::Update\(' -and
		$cameraFreeSource -match '(?s)Late_Update\(f32_t fTimeDelta\).*?Update_FollowCamera\(fTimeDelta\).*?Update_PipeLine' -and
		$cameraFreeSource -match '(?s)Set_FollowEnabled\(bool_t isEnabled\).*?Update_FollowCamera\(0\.f\).*?Update_PipeLine' -and
		$cameraBaseSource -match 'Refresh_CameraState\(\)' -and
		$gameInstanceSource -match '(?s)Refresh_CameraState\(\).*?m_pPipeLine->Update\(\).*?m_pFrustum->Update_InWorldSpace\(\)' -and
		$networkManagerSource -match 'Try_Get_LocalSpawn' -and
		$networkManagerSource -match 'spawned\.iPlayerId == m_iLocalPlayerId' -and
		([regex]::Matches($networkManagerSource, 'm_hasLocalSpawn = false').Count -ge 4) -and
		$bernLevelSource -match 'Try_Get_LocalSpawn' -and
		$bernLevelSource -match 'cameraDesc\.fFollowResponse = 0\.f') `
		'follow camera commits the updated character transform and camera-derived state in the same frame; Bern starts from the approved local spawn when available'

    $characterLogicFiles = @(Get-ChildItem -LiteralPath 'Client\Private' -Filter 'Logic_*.cpp' -File)
    $characterLogicBoundaryHits = @($characterLogicFiles | Select-String -Pattern 'Get_DIKey|Get_DIMouse|NetworkManager|Play_Skill\(')
    $characterSpecSource = Get-Content -LiteralPath 'Client\Public\CharacterSpec.h' -Raw
    Add-Check 'character.presentation-boundary' (
        $characterLogicBoundaryHits.Count -eq 0 -and
        $characterSpecSource -match 'Update_Presentation') "forbiddenCalls=$($characterLogicBoundaryHits.Count)"

    $materialReaderSource = Get-Content -LiteralPath 'Engine\Private\BinaryAsset\Winters\WMaterialReader.cpp' -Raw
    $modelSource = Get-Content -LiteralPath 'Engine\Private\Model.cpp' -Raw
    Add-Check 'model.resource-confinement' (
        $materialReaderSource -match 'IsBelowRoot' -and
        $materialReaderSource -notmatch 'Resources/LostArk' -and
        $materialReaderSource -notmatch 'filesystem::exists\(normalizedPath\)' -and
        $modelSource -notmatch 'L"LostArk"') 'material textures remain below the flat Resources root'
	$loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$loaderFactoryFiles = @(
		'Engine\Private\Shader.cpp',
		'Engine\Private\Material.cpp',
		'Engine\Private\Model.cpp',
		'Engine\Private\Navigation.cpp',
		'Client\Private\Camera_Free.cpp',
		'Client\Private\Character.cpp',
		'Client\Private\Part_Body.cpp',
		'Client\Private\Part_Equipment.cpp',
		'Client\Private\Valtan.cpp',
		'Client\Private\Body_Valtan.cpp')
	$loaderFactoryModalHits = @($loaderFactoryFiles |
		ForEach-Object { Select-String -LiteralPath $_ -Pattern 'MSG_BOX|MessageBox' })
    Add-Check 'loader.no-worker-modal' (
		$loaderFactoryModalHits.Count -eq 0) "factoryModalHits=$($loaderFactoryModalHits.Count)"
	Add-Check 'loader.no-thread-termination' (
		$loaderSource -notmatch 'TerminateThread' -and
		$loaderSource -match 'TerminateProcess\(GetCurrentProcess\(\), ERROR_TIMEOUT\)') 'cooperative cancellation escalates to bounded process fail-fast, never thread termination'

    $regressionHarnessSource = Get-Content -LiteralPath 'Tools\Build\Invoke-BuildAndRegression.ps1' -Raw
	Add-Check 'harness.runtime-boundary' (
		$regressionHarnessSource -match 'Assert-RuntimeLayout' -and
		$regressionHarnessSource -match 'NetworkProtocolHarness' -and
		$regressionHarnessSource -match '--contract-test' -and
		$regressionHarnessSource -notmatch 'Invoke-ClientSmoke|--smoke|--scenario') 'automation verifies protocol and server contracts without embedding a Client runtime harness'
	Add-Check 'harness.server-gameplay-contract' (
		$regressionHarnessSource -match '--contract-test' -and
		(Test-Path -LiteralPath 'Server\Private\ServerGameplayContractTests.cpp')) 'regression runs server skill, damage, boss, and navigation contracts'

	$solutionLaunch = Read-Json 'Framework.slnLaunch'
	$serverClientProfiles = @($solutionLaunch | Where-Object {
		$_.Name -eq 'Server + Client' })
	$launchProjects = if ($serverClientProfiles.Count -eq 1) {
		@($serverClientProfiles[0].Projects)
	}
	else {
		@()
	}
	Add-Check 'harness.server-client-launch' (
		$launchProjects.Count -eq 2 -and
		$launchProjects[0].Path -eq 'Server\Default\Server.vcxproj' -and
		$launchProjects[0].Action -eq 'Start' -and
		$launchProjects[1].Path -eq 'Client\Default\Client.vcxproj' -and
		$launchProjects[1].Action -eq 'Start') 'Visual Studio launches the real Server before the real Client'

    $characterCatalog = Read-Json 'Data\Actors\CharacterCatalog.json'
    $bossCatalog = Read-Json 'Data\Actors\BossCatalog.json'
    $missingActorAssets = [Collections.Generic.List[string]]::new()
    foreach ($character in @($characterCatalog.characters | Where-Object runtimeStatus -eq 'supported')) {
		foreach ($assetPath in @($character.bodyModel) + @($character.weaponModels) + @($character.equipmentModels)) {
			if ($assetPath -and -not (Test-Path -LiteralPath (Join-Path $resourceRoot $assetPath))) {
				$missingActorAssets.Add($assetPath)
			}
        }
    }
    foreach ($boss in @($bossCatalog.bosses)) {
        foreach ($assetPath in @($boss.bodyModel, $boss.weaponModel)) {
            if ($assetPath -and -not (Test-Path -LiteralPath (Join-Path $resourceRoot $assetPath))) {
                $missingActorAssets.Add($assetPath)
            }
        }
    }
    Add-Check 'actors.catalog-assets' ($missingActorAssets.Count -eq 0) "missing=$($missingActorAssets -join ',')"
	$lobbySource = Get-Content -LiteralPath 'Client\Private\Level_Lobby.cpp' -Raw
	$characterSelectionStateSource = Get-Content -LiteralPath 'Client\Private\CharacterSelectionState.cpp' -Raw
	$playableRoster = @($characterCatalog.characters | ForEach-Object networkClassId) -join ','
	$rosterStatus = @($characterCatalog.characters | ForEach-Object runtimeStatus) -join ','
	Add-Check 'actors.playable-roster' (
		$playableRoster -eq 'LANCE_MASTER,GUNSLINGER,SLAYER,ARTIST,DIMENSIONMASTER,WARLORD' -and
		$rosterStatus -eq 'supported,supported,supported,supported,supported,supported' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::LANCE_MASTER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::GUNSLINGER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::SLAYER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::ARTIST' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::WARLORD' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::DIMENSIONMASTER' -and
		$lobbySource -notmatch 'CHARACTER_CLASS_ID::DESTROYER' -and
		$characterSelectionStateSource -match 'Is_Supported_Playable_Character_Class') "roster=$playableRoster status=$rosterStatus"
	$dimensionmasterActor = @($characterCatalog.characters | Where-Object networkClassId -eq 'DIMENSIONMASTER')
	$dimensionmasterAnimationContracts = @(
		[pscustomobject]@{ Path = 'Data\Animation\Authored\DimensionMaster\DimensionMaster.animevents'; Header = 'LOSTARK_ANIM_EVENTS 5 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.animnotify'; Header = 'LOSTARK_ANIM_NOTIFY 1 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.clipmap'; Header = 'LOSTARK_CLIP_MAP 1 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.clipseq'; Header = 'LOSTARK_CLIP_SEQ 2 "DimensionMaster"' },
		[pscustomobject]@{ Path = 'Data\Animation\Reference\DimensionMaster\DimensionMaster.skilltiming'; Header = 'LOSTARK_SKILL_TIMING 2 "DimensionMaster"' })
	$invalidDimensionMasterAnimationDocuments = @(
		foreach ($contract in $dimensionmasterAnimationContracts) {
			if (-not (Test-Path -LiteralPath $contract.Path -PathType Leaf)) {
				$contract.Path
				continue
			}
			if ((Get-Content -LiteralPath $contract.Path -TotalCount 1) -cnotmatch (
				'^' + [regex]::Escape($contract.Header) + ' \d+$')) {
				$contract.Path
			}
		})
	Add-Check 'actors.dimensionmaster-runtime-animation' (
		$dimensionmasterActor.Count -eq 1 -and
		[int]$characterCatalog.formatVersion -eq 2 -and
		$dimensionmasterActor[0].bodyModel -eq 'Character/DimensionMaster/DimensionMaster_Character.wmodel' -and
		@($dimensionmasterActor[0].equipmentModels).Count -eq 0 -and
		(@($dimensionmasterActor[0].weaponModels) -join ',') -eq (
			'Character/WP_WSWP_M_06/WP_WSWP_M_06L.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06S.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06P.wmodel,' +
			'Character/WP_WSWP_M_06/WP_WSWP_M_06E.wmodel') -and
		(Test-Path -LiteralPath (Join-Path $resourceRoot 'Character\DimensionMaster\DimensionMaster_Character.wmodel') -PathType Leaf) -and
		$invalidDimensionMasterAnimationDocuments.Count -eq 0) "combined body, four socketed weapon assets and owner-matched Animation Tool documents exist; invalid=$($invalidDimensionMasterAnimationDocuments -join ',')"
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$valtanAssetServiceSource = Get-Content -LiteralPath 'Client\Private\ValtanPresentationAssetService.cpp' -Raw
	$npcAssetServiceSource = Get-Content -LiteralPath 'Client\Private\NpcPresentationAssetService.cpp' -Raw
	$dimensionmasterLogicSource = Get-Content -LiteralPath 'Client\Private\Logic_DimensionMaster.cpp' -Raw
	Add-Check 'actors.dimensionmaster-four-part-weapon' (
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_L' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_S' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_P' -and
		$playableAssetServiceSource -match 'Prototype_Component_Model_DimensionMaster_Weapon_E' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_1' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_2' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_3' -and
		$dimensionmasterLogicSource -match 'b_wp_swm_m_4_02' -and
		$dimensionmasterLogicSource -match '180\.f') 'DimensionMaster L/S/P/E prototype tags and exact battle sockets are connected'
	Add-Check 'actors.dimensionmaster-actorx-scale' (
		$playableAssetServiceSource -match 'CHARACTER_CLASS_ID::DIMENSIONMASTER == characterClass[\s\S]{0,80}0\.01f : 0\.0001f') 'ActorX DimensionMaster uses 0.01 while legacy character packages retain 0.0001'
	$hudViewModelHeader = Get-Content -LiteralPath 'Client\Public\CombatHUDViewModel.h' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	$characterSelectSource = Get-Content -LiteralPath 'Client\Private\Level_CharacterSelect.cpp' -Raw
	Add-Check 'hud.selected-class-boundary' (
		$hudViewModelHeader -match 'HUD_PLAYER_STATE[\s\S]{0,300}eCharacterClass' -and
		$hudViewModelHeader -match 'Apply_LocalPlayer' -and
		$hudViewModelSource -match 'definition\.eCharacterClass != characterClass' -and
		$hudViewModelSource -match 'Balance/PlayerProfiles\.json' -and
		$replicationSource -match 'Apply_LocalPlayer\([\s\S]{0,160}localRecord->eCharacterClass' -and
		$replicationSource -match 'Replace_CharacterClass' -and
		$mainAppSource -match 'LEVEL::CHARACTER_SELECT' -and
		$mainAppSource -match 'RenderCombatHUD\(\);') 'runtime HUD follows the Server snapshot class while filtering skill definitions by class'
	$playerSkillDocument = Read-Json 'Data\Balance\PlayerSkills.json'
	$missingQuickSlots = [Collections.Generic.List[string]]::new()
	$classQuickSlotContracts = [ordered]@{
		'LANCE_MASTER' = @('Q','W','E','R','A','S','D','F','Z','SPACE','T','V','ALT_V','LMB')
		'GUNSLINGER' = @('Q','W','E','R','A','S','D','F','T','V','ALT_V','LMB')
		'SLAYER' = @('Q','W','E','R','A','S','D','F','V','ALT_V','LMB')
		'ARTIST' = @('Q','W','E','R','A','S','D','F','T','X','Z','V','ALT_V','SPACE','LMB')
		'DIMENSIONMASTER' = @('Q','W','E','R','A','S','D','F','T','V','ALT_V','SPACE','LMB')
		'WARLORD' = @('Q','W','E','R','A','S','D','F','T','X','Z','V','ALT_V','SPACE','LMB')
	}
	foreach ($className in $classQuickSlotContracts.Keys) {
		foreach ($slotName in $classQuickSlotContracts[$className]) {
			$bindings = @($playerSkillDocument.skills | Where-Object {
				$_.characterClass -eq $className -and $_.inputSlot -eq $slotName
			})
			$distinctStances = @($bindings | Select-Object -ExpandProperty requiredStance -Unique)
			if ($bindings.Count -eq 0 -or $bindings.Count -ne $distinctStances.Count) {
				$missingQuickSlots.Add("${className}:$slotName")
			}
		}
	}
	$dimensionmasterSkillRows = @($playerSkillDocument.skills |
		Where-Object characterClass -eq 'DIMENSIONMASTER')
	Add-Check 'gameplay.playable-qw-contract' (
		$missingQuickSlots.Count -eq 0 -and
		$dimensionmasterSkillRows.Count -eq 13) "missing=$($missingQuickSlots -join ',') dimensionmasterRows=$($dimensionmasterSkillRows.Count)"

	$skillBindingOwners = [ordered]@{
		'LANCE_MASTER' = 'LanceMaster'
		'GUNSLINGER' = 'GunSlinger'
		'SLAYER' = 'Slayer'
		'ARTIST' = 'Artist'
		'DIMENSIONMASTER' = 'DimensionMaster'
		'WARLORD' = 'Warlord'
	}
	$quickSkillAnimationErrors = [Collections.Generic.List[string]]::new()
	$totalAuthoredBindings = 0
	foreach ($className in $skillBindingOwners.Keys) {
		$assetName = $skillBindingOwners[$className]
		$bindingPath = "Data\Animation\Authored\$assetName\$assetName.skillbindings.json"
		if (-not (Test-Path -LiteralPath $bindingPath -PathType Leaf)) {
			$quickSkillAnimationErrors.Add("${className}:missing authored binding document")
			continue
		}
		try {
			$bindingDocument = Read-Json $bindingPath
		}
		catch {
			$quickSkillAnimationErrors.Add("${className}:malformed authored binding document")
			continue
		}
		if ($bindingDocument.schema -ne 'lostark.animation-skill-bindings' -or
			[int]$bindingDocument.formatVersion -ne 3 -or
			$bindingDocument.animationAssetId -ne $assetName -or
			$bindingDocument.characterClass -ne $className) {
			$quickSkillAnimationErrors.Add("${className}:owner/schema mismatch")
			continue
		}
		$classSkills = @($playerSkillDocument.skills |
			Where-Object characterClass -eq $className)
		$bindings = @($bindingDocument.bindings)
		$totalAuthoredBindings += $bindings.Count
		if ($bindings.Count -ne $classSkills.Count) {
			$quickSkillAnimationErrors.Add("${className}:binding count $($bindings.Count)/$($classSkills.Count)")
		}
		foreach ($binding in $bindings) {
			if ($null -ne $binding.inputSlot -or $null -ne $binding.mode) {
				$quickSkillAnimationErrors.Add("${className}:binding duplicates gameplay authority")
			}
			$skillRows = @($classSkills | Where-Object skillId -eq $binding.skillId)
			$elements = @($binding.clips)
			# A flat clips array is one stage; a nested one is a stage per element.
			# Mixing the shapes leaves the stage count ambiguous.
			$nestedCount = @($elements | Where-Object {
				$_ -is [Array] -or $_ -is [Collections.IEnumerable] -and
					$_ -isnot [string] -and $_ -isnot [Management.Automation.PSCustomObject] }).Count
			$isNested = $nestedCount -eq $elements.Count -and $elements.Count -gt 0
			if ($nestedCount -ne 0 -and -not $isNested) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) mixed stage shape")
				continue
			}
			# Built by explicit append: returning a nested array from if/else lets
			# the output stream unroll it back into a flat one.
			$stages = @()
			if ($isNested) {
				foreach ($element in $elements) { $stages += , @($element) }
			}
			else { $stages += , @($elements) }
			$clips = @($stages | ForEach-Object { $_ })
			$clipNames = @($clips | ForEach-Object {
				if ($_ -is [string]) { $_ } else { [string]$_.clip } })
			$invalidClipRows = @($clips | Where-Object {
				$_ -isnot [string] -and (
					$null -eq $_.clip -or
					($null -eq $_.playMs -and $null -eq $_.playRate) -or
					($null -ne $_.playMs -and
						([int]$_.playMs -lt 1 -or [int]$_.playMs -gt 60000)) -or
					($null -ne $_.playRate -and
						([double]$_.playRate -lt 0.05 -or [double]$_.playRate -gt 16))) })
			if ($skillRows.Count -ne 1 -or $clips.Count -lt 1 -or $clips.Count -gt 16 -or
				@($stages | Where-Object { @($_).Count -lt 1 }).Count -ne 0 -or
				$invalidClipRows.Count -ne 0 -or
				@($clipNames | Where-Object { $_ -notmatch '^[A-Za-z0-9_.-]{1,255}$' }).Count -ne 0) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) invalid row")
				continue
			}
			$isStaged = $skillRows[0].skillKind -in @('COMBO', 'HOLD', 'COUNTER')
			if ($isStaged -and
				$stages.Count -ne @($skillRows[0].comboStages).Count) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) combo stage count")
			}
			if (-not $isStaged -and $stages.Count -ne 1) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) active must be one stage")
			}
			if ($skillRows[0].skillKind -eq 'HOLD' -and $stages.Count -ne 3) {
				$quickSkillAnimationErrors.Add("${className}:$($binding.skillId) hold needs start/loop/end")
			}
		}
		foreach ($skill in $classSkills) {
			if (@($bindings | Where-Object skillId -eq $skill.skillId).Count -ne 1) {
				$quickSkillAnimationErrors.Add("${className}:$($skill.skillId) missing/duplicate")
			}
		}
	}
	$characterRuntimeSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	$animationToolSource = Get-Content -LiteralPath 'Client\Private\Animation_Tool.cpp' -Raw
	$animationToolHeader = Get-Content -LiteralPath 'Client\Public\Animation_Tool.h' -Raw
	Add-Check 'gameplay.playable-skill-animation-authoring-contract' (
		$quickSkillAnimationErrors.Count -eq 0 -and
		$totalAuthoredBindings -eq @($playerSkillDocument.skills).Count -and
		$characterRuntimeSource -match 'CAnimationSkillBindingDocument::Load' -and
		$characterRuntimeSource -notmatch 'Animation/Reference|\.clipseq|\.clipmap' -and
		$characterRuntimeSource -match 'm_PendingChains' -and
		$characterRuntimeSource -match 'Commit_PendingClipChains' -and
		$characterRuntimeSource -match 'Character skill presentation unavailable' -and
		$animationToolSource -match 'Create_SkillBindingDraft' -and
		$animationToolSource -match 'CPlayerSkillCatalog::Get_Skills' -and
		$animationToolSource -match 'CAnimationSkillBindingDocument::Save_Atomic' -and
		$animationToolHeader -match 'm_bSkillBindingDirty' -and
		$animationToolHeader -match 'm_bDirty') "errors=$($quickSkillAnimationErrors -join ',') authored=$totalAuthoredBindings skills=$(@($playerSkillDocument.skills).Count)"
	$actorCatalogSource = Get-Content -LiteralPath 'Client\Private\ActorCatalog.cpp' -Raw
	$actorLoaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
	$hardcodedActorModelHits = @(($actorLoaderSource + $playableAssetServiceSource) |
		Select-String -AllMatches -Pattern 'Character/[A-Za-z0-9_./-]+\.wmodel')
	Add-Check 'actors.runtime-catalog-boundary' (
		$actorCatalogSource -match 'Actors/CharacterCatalog\.json' -and
		$actorCatalogSource -match 'Actors/BossCatalog\.json' -and
		$actorCatalogSource -match 'Actors/NpcCatalog\.json' -and
		$playableAssetServiceSource -match 'CActorCatalog::Find_Character' -and
		$valtanAssetServiceSource -match 'CActorCatalog::Find_Boss' -and
		$valtanAssetServiceSource -match 'Add_Prototypes' -and
		$npcAssetServiceSource -match 'CActorCatalog::Find_Npc' -and
		$npcAssetServiceSource -match 'Add_Prototypes' -and
		$replicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
		$replicationSource -match 'CActorCatalog::Find_Npc\(spawned\.strArchetypeId\)' -and
		$hardcodedActorModelHits.Count -eq 0) "hardcodedModelPaths=$($hardcodedActorModelHits.Count)"
	Add-Check 'actors.selected-first-on-demand-load' (
		$actorLoaderSource -match 'Get_LocalCharacterClass\(\)' -and
		$actorLoaderSource -match 'CPlayableCharacterAssetService::Begin_LevelLoad' -and
		$actorLoaderSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$replicationSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$playableAssetServiceSource -match 'g_ReadyClassesByLevel') 'loader admits selected class; replication admits a remote class once'

    $worldValidationPassed = $true
    $worldValidationDetail = ''
    try {
        $worldValidationDetail = (& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 -Mode Validate 2>&1) -join ' '
    }
    catch {
        $worldValidationPassed = $false
        $worldValidationDetail = $_.Exception.Message
    }
    Add-Check 'world.publish-contract' $worldValidationPassed $worldValidationDetail
	$worldAuthoringDocuments = @(
		'Data\Worlds\LV_BER_BERNCASTLE\Gameplay.world.json',
		'Data\Worlds\LV_LUT_HEARTRB_ED\Gameplay.world.json',
		'Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json',
		'Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\Gameplay.world.json')
	$invalidWorldAuthoring = @($worldAuthoringDocuments | Where-Object {
		$document = Read-Json $_
		$document.schema -ne 'lostark.world-gameplay' -or [int]$document.formatVersion -ne 4
	})
	Add-Check 'world.authoring-format-v4' ($invalidWorldAuthoring.Count -eq 0) `
		"invalid=$($invalidWorldAuthoring -join ',')"

	$bernWorldDocument = Read-Json 'Data\Worlds\LV_BER_BERNCASTLE\Gameplay.world.json'
	$bernChangeLevelTriggers = @($bernWorldDocument.placements | Where-Object {
		$_.kind -eq 'triggerBox' -and
		$_.enabled -eq $true -and
		@($_.events).Count -eq 1 -and
		$_.events[0].type -eq 'changeLevel' -and
		$_.events[0].targetWorldId -eq 'VALTAN_ARENA'
	})
	$bernLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Bern.cpp' -Raw
	$loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	Add-Check 'world.debug-change-level-trigger-presentation' (
		$bernChangeLevelTriggers.Count -eq 1 -and
		$bernChangeLevelTriggers[0].placementId -eq 'trigger.bern.to-valtan' -and
		$loaderSource -match 'LEVEL::DEVELOPMENT\)[\s\S]*LEVEL::BERN\)[\s\S]*Prototype_GameObject_TriggerBox' -and
		$bernLevelSource -match 'Ready_DebugLevelChangeTriggers' -and
		$bernLevelSource -match 'WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL' -and
		$bernLevelSource -match 'Layer_DebugWorldGameplay' -and
		$bernLevelSource -match 'rollback\(\)') `
		"bernChangeLevelTriggers=$($bernChangeLevelTriggers.Count)"

	$gameplayValidationPassed = $true
	$gameplayValidationDetail = ''
	try {
		$gameplayValidationDetail = (& .\Tools\GameplayPipeline\Publish-GameplayBalance.ps1 -Mode Validate 2>&1) -join ' '
	}
	catch {
		$gameplayValidationPassed = $false
		$gameplayValidationDetail = $_.Exception.Message
	}
	Add-Check 'gameplay.balance-publish-contract' $gameplayValidationPassed $gameplayValidationDetail
	$runtimeSetRollbackPassed = $false
	$runtimeSetRollbackDetail = ''
	$runtimeSetFixtureRelative = ".codex_tmp/ProjectAuditBalanceRuntimeSet-$PID"
	$runtimeSetFixturePath = Join-Path $repoRoot $runtimeSetFixtureRelative
	try {
		$runtimeSetFiles = @(
			'Gameplay\Gameplay.bootstrap',
			'World\BERN.worldbootstrap',
			'World\VALTAN_ARENA.worldbootstrap',
			'World\TRAINING_GROUND.worldbootstrap',
			'World\CHARACTER_SELECT_ARENA.worldbootstrap')
		$baselineText = @{}
		foreach ($relative in $runtimeSetFiles) {
			$path = Join-Path $runtimeSetFixturePath $relative
			[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($path)) | Out-Null
			$sentinel = "old-$($relative.Replace('\', '-'))"
			[IO.File]::WriteAllText($path, $sentinel)
			$baselineText[$relative] = $sentinel
		}
		$runtimeSetRollbackPassed = $true
		foreach ($failurePoint in @(3, 5)) {
			try {
				& .\Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1 `
					-Mode Publish -OutputRoot $runtimeSetFixtureRelative `
					-FailureAfterPromote $failurePoint | Out-Null
				throw 'Balance runtime set failure injection unexpectedly succeeded.'
			}
			catch {
				if ($_.Exception.Message -eq 'Balance runtime set failure injection unexpectedly succeeded.') { throw }
			}
			foreach ($relative in $runtimeSetFiles) {
				$path = Join-Path $runtimeSetFixturePath $relative
				if ([IO.File]::ReadAllText($path) -cne $baselineText[$relative]) {
					$runtimeSetRollbackPassed = $false
				}
			}
		}
		& .\Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1 `
			-Mode Publish -OutputRoot $runtimeSetFixtureRelative | Out-Null
		foreach ($relative in $runtimeSetFiles) {
			$path = Join-Path $runtimeSetFixturePath $relative
			if (-not [IO.File]::Exists($path) -or
				[IO.File]::ReadAllText($path) -ceq $baselineText[$relative]) {
				$runtimeSetRollbackPassed = $false
			}
		}
		$runtimeSetLeftovers = @(Get-ChildItem -LiteralPath $runtimeSetFixturePath `
			-Recurse -Force | Where-Object Name -Match 'balance-runtime-set|rollback')
		$runtimeSetRollbackPassed = $runtimeSetRollbackPassed -and
			$runtimeSetLeftovers.Count -eq 0
		$runtimeSetRollbackDetail = "leftovers=$($runtimeSetLeftovers.Count)"
	}
	catch {
		$runtimeSetRollbackDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($runtimeSetFixturePath)) {
			Remove-Item -LiteralPath $runtimeSetFixturePath -Recurse -Force
		}
	}
	Add-Check 'gameplay.balance-runtime-set-rollback' `
		$runtimeSetRollbackPassed $runtimeSetRollbackDetail
	$provenanceReceipt = Read-Json 'Data\Balance\Reference\Official\2026-08-05.balance-provenance.receipt.json'
	Add-Check 'gameplay.balance-field-provenance' (
		$provenanceReceipt.schema -eq 'lostark.balance-provenance-receipt' -and
		[int]$provenanceReceipt.referenceSkillLevel -eq 10 -and
		[int]$provenanceReceipt.coverage.skillDefinitionCount -eq @($playerSkillDocument.skills).Count -and
		[int]$provenanceReceipt.coverage.fieldEntryCount -eq @($provenanceReceipt.entries).Count -and
		(Test-Path -LiteralPath 'Tools\GameplayPipeline\Export-OfficialBalanceReceipt.py') -and
		(Test-Path -LiteralPath 'Tools\GameplayPipeline\Update-BalanceProvenanceReceipt.ps1')) `
		"skills=$($provenanceReceipt.coverage.skillDefinitionCount) fields=$(@($provenanceReceipt.entries).Count)"

	$navigationValidationPassed = $true
	$navigationValidationDetail = ''
	$legacyNavigationFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\DataFiles\Navigation' `
		-File -Filter 'ValtanArena.*' -ErrorAction SilentlyContinue)
	$valtanNavigationAuthoring = @(
		'Data\Navigation\LV_LUT_HEARTRB_ED.navsource',
		'Data\Navigation\LV_LUT_HEARTRB_ED.navpaint',
		'Data\Navigation\LV_LUT_HEARTRB_ED.navblockers')
	$missingValtanNavigationAuthoring = @($valtanNavigationAuthoring | Where-Object {
		-not (Test-Path -LiteralPath $_ -PathType Leaf)
	})
	Add-Check 'navigation.data-root-contract' (
		$legacyNavigationFiles.Count -eq 0 -and
		$missingValtanNavigationAuthoring.Count -eq 0 -and
		(Test-Path -LiteralPath 'Client\Bin\DataFiles\Navigation\LV_LUT_HEARTRB_ED.navgrid' -PathType Leaf)) `
		"legacy=$($legacyNavigationFiles.Count) missingAuthoring=$($missingValtanNavigationAuthoring.Count)"
	try {
		$navigationValidationDetail = (& .\Tools\NavigationPipeline\Publish-ServerNavigation.ps1 -Mode Validate 2>&1) -join ' '
	}
	catch {
		$navigationValidationPassed = $false
		$navigationValidationDetail = $_.Exception.Message
	}
	Add-Check 'navigation.server-publish-contract' $navigationValidationPassed $navigationValidationDetail

	$worldRollbackPassed = $false
	$worldRollbackDetail = ''
	$worldFixtureRelative = ".codex_tmp/ProjectAuditWorldPublish-$PID"
	$worldFixturePath = Join-Path $repoRoot $worldFixtureRelative
	try {
		[IO.Directory]::CreateDirectory($worldFixturePath) | Out-Null
		$bernFixture = Join-Path $worldFixturePath 'BERN.worldbootstrap'
		$valtanFixture = Join-Path $worldFixturePath 'VALTAN_ARENA.worldbootstrap'
		$trainingFixture = Join-Path $worldFixturePath 'TRAINING_GROUND.worldbootstrap'
		$characterSelectFixture = Join-Path $worldFixturePath 'CHARACTER_SELECT_ARENA.worldbootstrap'
		[IO.File]::WriteAllText($bernFixture, 'original-bern')
		[IO.File]::WriteAllText($valtanFixture, 'original-valtan')
		[IO.File]::WriteAllText($trainingFixture, 'original-training')
		[IO.File]::WriteAllText($characterSelectFixture, 'original-character-select')
		foreach ($failurePoint in @(1, 4)) {
			try {
				& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 `
					-Mode Publish `
					-OutputRoot $worldFixtureRelative `
					-FailureAfterPromote $failurePoint | Out-Null
				throw 'Failure injection unexpectedly succeeded.'
			}
			catch {
				if ($_.Exception.Message -eq 'Failure injection unexpectedly succeeded.') {
					throw
				}
			}
		}
		$staleTransactionFiles = @(Get-ChildItem -LiteralPath $worldFixturePath -Force |
			Where-Object Name -Match '^\.staging\.|\.rollback\.')
		$worldRollbackPassed =
			[IO.File]::ReadAllText($bernFixture) -eq 'original-bern' -and
			[IO.File]::ReadAllText($valtanFixture) -eq 'original-valtan' -and
			[IO.File]::ReadAllText($trainingFixture) -eq 'original-training' -and
			[IO.File]::ReadAllText($characterSelectFixture) -eq 'original-character-select' -and
			$staleTransactionFiles.Count -eq 0
		$worldRollbackDetail = "stale=$($staleTransactionFiles.Count)"
	}
	catch {
		$worldRollbackDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($worldFixturePath)) {
			Remove-Item -LiteralPath $worldFixturePath -Recurse -Force
		}
	}
	Add-Check 'world.publish-generation-rollback' $worldRollbackPassed $worldRollbackDetail

    $monsterContractFiles = @(
        'Data\Actors\MonsterCatalog.json',
        'Data\Balance\MonsterProfiles.json',
        'Data\Worlds\LV_LUT_HEARTRB_ED\SpawnGroups.world.json',
		'Data\Worlds\LV_LOBBY_CLASSSELECT_SL00\SpawnGroups.world.json',
		'Shared\Public\Gameplay\CombatCollisionContract.h',
		'Shared\Private\Gameplay\CombatCollisionContract.cpp',
        'Server\Public\SpawnGroupRuntime.h',
        'Server\Private\SpawnGroupRuntime.cpp',
        'Server\Public\MonsterBrain.h',
        'Server\Private\MonsterBrain.cpp',
        'Client\Public\MonsterPresentationAssetService.h',
        'Client\Private\MonsterPresentationAssetService.cpp')
    $missingMonsterContractFiles = @($monsterContractFiles |
        Where-Object { -not (Test-Path -LiteralPath $_) })
    $legacyMonsterRuntimeHits = @(
        Select-String -LiteralPath @(
            'Server\Private\GameRoom.cpp',
            'Client\Private\ClientReplication.cpp',
            'Client\Private\MonsterPresentationAssetService.cpp') `
            -Pattern '#include\s+"Monster\.h"|CMonster::Create|Logic_Monster')
    $spawnGroupPublisherSource = Get-Content -LiteralPath 'Tools\WorldPipeline\Publish-WorldGameplay.ps1' -Raw
	$spawnGroupRuntimeSource = Get-Content -LiteralPath 'Server\Private\SpawnGroupRuntime.cpp' -Raw
    $staleWorldPublishFiles = @(Get-ChildItem -LiteralPath 'Server\Bin\DataFiles\World' -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '\.(tmp|rollback)\.' })
    Add-Check 'world.monster-spawn-group-contract' (
        $missingMonsterContractFiles.Count -eq 0 -and
        $legacyMonsterRuntimeHits.Count -eq 0 -and
        $spawnGroupPublisherSource -match 'SpawnGroups\.world\.json' -and
		$spawnGroupPublisherSource -match 'spawngroupsbootstrap' -and
		$gameRoomSource -match 'WORLD_ENTITY_SPAWN_RESULT::ACTIVATED' -and
		$gameRoomSource -match 'Reset_ReplayableArenaWhenEmpty' -and
		$spawnGroupRuntimeSource -match 'Activate_Immediate' -and
		$replicationSource -match 'Set_CombatColliderDebugVisible') "missing=$($missingMonsterContractFiles -join ',') legacyRuntimeHits=$($legacyMonsterRuntimeHits.Count)"
    Add-Check 'world.publish-cleanup' ($staleWorldPublishFiles.Count -eq 0) "stale=$($staleWorldPublishFiles.Name -join ',')"

    $serverRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
    $serverProjectSource = Get-Content -LiteralPath 'Server\Default\Server.vcxproj' -Raw
	$mapToolSource = Get-Content -LiteralPath 'Client\Private\MapTool.cpp' -Raw
	Add-Check 'world.player-spawn-position-authoring' (
		$mapToolSource -match 'Player Spawn Position Offset' -and
		$mapToolSource -match 'Apply Delta To Spawn Position' -and
		$mapToolSource -match 'staged\.position\.x \+= m_WorldPlacementPositionDelta\.x' -and
		$serverRoomSource -match 'player\.fPositionX = spawn->fPositionX' -and
		$serverRoomSource -match 'player\.fPositionY = spawn->fPositionY' -and
		$serverRoomSource -match 'player\.fPositionZ = spawn->fPositionZ') 'MapTool resolves authored position delta and Server consumes the saved spawn transform'
	Add-Check 'server.world-bootstrap-boundary' (
		$serverRoomSource -match 'Find_AvailablePlayerSpawn' -and
		$serverRoomSource -match 'Update_WorldEntities' -and
		$serverRoomSource -match 'Find_BossPatterns' -and
		$serverRoomSource -notmatch 'placement\.iPatternTelegraphMs' -and
        $serverRoomSource -notmatch 'fActionElapsedSeconds >= 0\.8f' -and
        $serverRoomSource -notmatch 'm_Players\.size\(\)\) \* 2\.f' -and
        $serverProjectSource -match 'PublishWorldGameplay') 'MapTool world documents publish before Server compile'

    $packetMessagesSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketMessages.h' -Raw
    $clientReplicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
    Add-Check 'server.world-entity-replication' (
        $packetMessagesSource -match 'S2C_WORLD_ENTITY_SPAWNED' -and
        $packetMessagesSource -match 'WORLD_ENTITY_ACTION' -and
		$clientReplicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
        $serverRoomSource -match 'Broadcast_WorldSnapshot' -and
        $serverRoomSource -match 'Update_WorldEntities') 'server owns world entity state; client consumes presentation snapshots'

	$playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	Add-Check 'gameplay.command-server-truth' (
		$packetMessagesSource -match 'C2S_USE_SKILL' -and
		$packetMessagesSource -match 'PLAYER_ACTION_STATE' -and
		$serverRoomSource -match 'Handle_UseSkill' -and
		$playerControllerSource -match 'Request_UseSkill' -and
		$playerControllerSource -notmatch 'Play_Skill') 'input emits intent and presentation consumes approved snapshot action'
	Add-Check 'debug.character-select-skill-audition' (
		$serverRoomSource -match 'CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS\s*=\s*90u' -and
		$serverRoomSource -match 'Add_ServerTicksSkippingReservedZero[\s\S]*?SERVER_TICK_CARDINALITY[\s\S]*?startTick - 1u[\s\S]*?elapsedTicks[\s\S]*?\+ 1u' -and
		$serverRoomSource -match 'static_assert\(90u\s*==\s*Add_ServerTicksSkippingReservedZero\([\s\S]*?numeric_limits<std::uint32_t>::max[\s\S]*?90u\)\)' -and
		$serverRoomSource -match 'static_assert\(1u\s*==\s*Add_ServerTicksSkippingReservedZero\([\s\S]*?numeric_limits<std::uint32_t>::max[\s\S]*?- 89u[\s\S]*?90u\)\)' -and
		$serverRoomSource -match '#ifdef _DEBUG[\s\S]*?WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId[\s\S]*?iCurrentResource\s*=\s*[\s\S]*?iMaximumResource[\s\S]*?#endif[\s\S]*?m_PlayerSkillSystem\.Try_Start[\s\S]*?CooldownEndTickBySkillId\.insert_or_assign\([\s\S]*?Add_ServerTicksSkippingReservedZero\([\s\S]*?CHARACTER_SELECT_AUDITION_COOLDOWN_TICKS' -and
		$serverRoomSource -notmatch 'CooldownEndTickBySkillId\.erase\(useSkill\.iSkillId\)' -and
		$serverRoomSource -notmatch 'command\.iSkillId\s*==\s*2050210') `
		'Debug Character Select Server Arena uses a Server-authoritative three-second audition cooldown with full resources'
	$skillCatalogSource = Get-Content -LiteralPath 'Client\Private\PlayerSkillCatalog.cpp' -Raw
	Add-Check 'gameplay.skill-binding-is-data' (
		$playerControllerSource -match 'CPlayerSkillCatalog::Find_BySlot' -and
		$playerControllerSource -notmatch '\b3\d{4}\b' -and
		$skillCatalogSource -match 'Balance/PlayerSkills\.json') 'quick slots resolve through balance data instead of skill IDs hardcoded per class'
	Add-Check 'ui.combat-viewmodel-boundary' (
		$skillCatalogSource -match 'Balance/PlayerSkills\.json' -and
		$skillCatalogSource -match 'Balance/DamageProfiles\.json' -and
		$hudViewModelSource -match 'CPlayerSkillCatalog::Get_Skills' -and
		$clientReplicationSource -match 'Apply_LocalPlayer' -and
		$clientReplicationSource -match 'Apply_Boss' -and
		$clientReplicationSource -match 'Apply_DamageEvents') 'HUD consumes server snapshot plus validated balance definitions without packets'
	$clientProjectSource = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$balanceToolSource = Get-Content -LiteralPath 'Client\Private\BalanceTool.cpp' -Raw
	$balanceRuntimePublisherSource = Get-Content -LiteralPath 'Tools\GameplayPipeline\Publish-BalanceRuntimeSet.ps1' -Raw
	$gameplayCatalogSource = Get-Content -LiteralPath 'Server\Private\GameplayCatalog.cpp' -Raw
	$valtanBrainSource = Get-Content -LiteralPath 'Server\Private\ValtanBrain.cpp' -Raw
	Add-Check 'debug.balance-tool-contract' (
		$clientProjectSource -match 'BalanceTool\.cpp' -and
		$balanceToolSource -match 'Publish-BalanceRuntimeSet\.ps1' -and
		$balanceToolSource -match 'Update-BalanceProvenanceReceipt\.ps1' -and
		$balanceRuntimePublisherSource -match 'Publish-GameplayBalance\.ps1' -and
		$balanceRuntimePublisherSource -match 'Publish-WorldGameplay\.ps1' -and
		$balanceRuntimePublisherSource -match 'FailureAfterPromote' -and
		$balanceToolSource -match 'CREATE_NO_WINDOW' -and
		$balanceToolSource -match 'TerminateProcess') `
		'F1 Balance Tool saves authoring, synchronizes provenance, validates/publishes, and bounds its owned subprocess'
	Add-Check 'gameplay.defense-consumer' (
		$gameplayCatalogSource -match 'Apply_Defense' -and
		$valtanBrainSource -match 'playerProfile->iDefense') `
		'player defense is consumed by the centralized incoming damage curve'
	$bossProfileDocument = Read-Json 'Data\Balance\BossProfiles.json'
	$valtanEncounterDocument = Read-Json 'Data\Encounters\Valtan\ValtanEncounter.json'
	$valtanPatternIds = @($valtanEncounterDocument.patterns | ForEach-Object { [string]$_.patternId })
	Add-Check 'gameplay.valtan-health-bar-pattern-contract' (
		[int]$bossProfileDocument.formatVersion -eq 3 -and
		[int]$bossProfileDocument.bosses[0].maximumHealthBars -eq 160 -and
		[int]$valtanEncounterDocument.formatVersion -eq 3 -and
		$valtanPatternIds.Count -eq 31 -and
		$valtanPatternIds -contains 'VALTAN_FLOOR_WIPE_130' -and
		$valtanPatternIds -contains 'VALTAN_FOUR_PILLARS_105' -and
		$valtanPatternIds -contains 'VALTAN_ARENA_BREAK_80' -and
		$valtanPatternIds -contains 'VALTAN_MAGIC_ORB_STAGGER_76' -and
		$valtanPatternIds -contains 'VALTAN_CENTER_GRAB_COUNTER_64' -and
		$valtanPatternIds -contains 'VALTAN_ARENA_BREAK_33' -and
		$valtanPatternIds -contains 'VALTAN_GHOST_TRANSITION_15' -and
		$gameplayCatalogSource -match 'PATTERNSTAGE' -and
		$valtanBrainSource -match 'QueueCrossedHealthBarPatterns' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::RING' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::CONE' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::BOX' -and
		$valtanBrainSource -match 'BOSS_PATTERN_HIT_SHAPE::CROSS') `
		'Valtan 160-bar authoring, 31 staged patterns, ordered thresholds, and Server collider hits share one runtime path'
	$serverPlayerSource = Get-Content -LiteralPath 'Server\Public\ServerPlayer.h' -Raw
	$gameRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
	Add-Check 'gameplay.valtan-entry-protection-revive-contract' (
		$serverPlayerSource -match 'isCombatReady' -and
		$gameRoomSource -match 'Handle_RevivePlayer' -and
		$gameRoomSource -match 'WORLD_ID::VALTAN_ARENA' -and
		$packetMessagesSource -match 'C2S_REVIVE_PLAYER' -and
		$balanceToolSource -match 'Revive at death position') `
		'Valtan entry protection and same-position revive remain Server-authoritative and Balance Tool-addressable'
	Add-Check 'ui.combat-font-hud-contract' (
		$mainAppSource -match 'RenderCombatHUDText' -and
		$mainAppSource -match 'RenderBossHealthBar' -and
		$mainAppSource -match 'AddRectFilled' -and
		$mainAppSource -match 'Font_YG330' -and
		$mainAppSource -notmatch 'const wstring name = Utf8ToWide\(boss\.strDisplayName\)' -and
		$mainAppSource -notmatch 'const wstring bars = std::to_wstring\(currentHealthBar\)' -and
		$mainAppSource -notmatch 'ImGui::ProgressBar' -and
		$mainAppSource -notmatch '##RuntimeCombatHUD') `
		'player HP/Mana and one Valtan health bar with current/max text render after the authored HUD'

    $report = [ordered]@{
        schema = 'lostark.project-audit-report'
        formatVersion = 1
        generatedUtc = [DateTime]::UtcNow.ToString('o')
        passed = $failures.Count -eq 0
        checks = $checks
        failures = $failures
    }
    $resolvedReport = [IO.Path]::GetFullPath($ReportPath)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($resolvedReport)) | Out-Null
    [IO.File]::WriteAllText(
        $resolvedReport,
        ($report | ConvertTo-Json -Depth 8) + [Environment]::NewLine,
        [Text.UTF8Encoding]::new($false))

    if ($failures.Count -gt 0) {
        throw "Project audit failed ($($failures.Count)): $($failures -join '; ')"
    }
    Write-Output "Project audit passed: $($checks.Count) checks. Report: $resolvedReport"
}
catch {
    Write-Error ("{0}`n{1}" -f $_.Exception.Message, $_.ScriptStackTrace)
    exit 1
}
```

### G00-07-21. `Framework.sln` full code

```text

Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.10.35122.118
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Client", "Client\Default\Client.vcxproj", "{1A5672C6-54B4-4779-99E7-84EF85171388}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Engine", "Engine\Default\Engine.vcxproj", "{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Shared", "Shared\Default\Shared.vcxproj", "{F4CCF815-6D51-412F-A76E-84D2F1D05571}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "NetworkProtocolHarness", "Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj", "{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "ValtanFourPlayerHarness", "Tools\ValtanFourPlayerHarness\Default\ValtanFourPlayerHarness.vcxproj", "{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "ClientFrontendHarness", "Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj", "{78406C04-3D55-4F36-B6D1-B5180A48F521}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Server", "Server\Default\Server.vcxproj", "{053788E7-C377-4811-8AFE-BC23D9BE4AE7}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "PhysicsContractHarness", "Tools\PhysicsContractHarness\Default\PhysicsContractHarness.vcxproj", "{AB5658F8-984D-4826-8F05-5044E1B12BAF}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "WModelGeometryContractHarness", "Tools\WModelGeometryContractHarness\Default\WModelGeometryContractHarness.vcxproj", "{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}"
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "PointLightFalloffContractHarness", "Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj", "{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Debug|x86 = Debug|x86
		Release|x64 = Release|x64
		Release|x86 = Release|x86
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x64.ActiveCfg = Debug|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x64.Build.0 = Debug|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x86.ActiveCfg = Debug|Win32
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Debug|x86.Build.0 = Debug|Win32
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x64.ActiveCfg = Release|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x64.Build.0 = Release|x64
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x86.ActiveCfg = Release|Win32
		{1A5672C6-54B4-4779-99E7-84EF85171388}.Release|x86.Build.0 = Release|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x64.ActiveCfg = Debug|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x64.Build.0 = Debug|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x86.ActiveCfg = Debug|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Debug|x86.Build.0 = Debug|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x64.ActiveCfg = Release|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x64.Build.0 = Release|x64
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x86.ActiveCfg = Release|Win32
		{922BAEFB-8A3C-4F15-9F51-BF7C7ECC1CD7}.Release|x86.Build.0 = Release|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x64.ActiveCfg = Debug|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x64.Build.0 = Debug|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x86.ActiveCfg = Debug|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Debug|x86.Build.0 = Debug|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x64.ActiveCfg = Release|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x64.Build.0 = Release|x64
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x86.ActiveCfg = Release|Win32
		{F4CCF815-6D51-412F-A76E-84D2F1D05571}.Release|x86.Build.0 = Release|Win32
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Debug|x64.ActiveCfg = Debug|x64
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Debug|x64.Build.0 = Debug|x64
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Debug|x86.ActiveCfg = Debug|Win32
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Debug|x86.Build.0 = Debug|Win32
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Release|x64.ActiveCfg = Release|x64
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Release|x64.Build.0 = Release|x64
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Release|x86.ActiveCfg = Release|Win32
		{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}.Release|x86.Build.0 = Release|Win32
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Debug|x64.ActiveCfg = Debug|x64
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Debug|x64.Build.0 = Debug|x64
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Debug|x86.ActiveCfg = Debug|Win32
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Debug|x86.Build.0 = Debug|Win32
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Release|x64.ActiveCfg = Release|x64
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Release|x64.Build.0 = Release|x64
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Release|x86.ActiveCfg = Release|Win32
		{B29B7E13-D49A-43E4-9723-4E9ABBF0C2F1}.Release|x86.Build.0 = Release|Win32
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Debug|x64.ActiveCfg = Debug|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Debug|x64.Build.0 = Debug|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Debug|x86.ActiveCfg = Debug|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Release|x64.ActiveCfg = Release|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Release|x64.Build.0 = Release|x64
		{78406C04-3D55-4F36-B6D1-B5180A48F521}.Release|x86.ActiveCfg = Release|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x64.ActiveCfg = Debug|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x64.Build.0 = Debug|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x86.ActiveCfg = Debug|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Debug|x86.Build.0 = Debug|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x64.ActiveCfg = Release|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x64.Build.0 = Release|x64
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x86.ActiveCfg = Release|Win32
		{053788E7-C377-4811-8AFE-BC23D9BE4AE7}.Release|x86.Build.0 = Release|Win32
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Debug|x64.ActiveCfg = Debug|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Debug|x64.Build.0 = Debug|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Debug|x86.ActiveCfg = Debug|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Release|x64.ActiveCfg = Release|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Release|x64.Build.0 = Release|x64
		{AB5658F8-984D-4826-8F05-5044E1B12BAF}.Release|x86.ActiveCfg = Release|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Debug|x64.ActiveCfg = Debug|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Debug|x64.Build.0 = Debug|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Debug|x86.ActiveCfg = Debug|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Release|x64.ActiveCfg = Release|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Release|x64.Build.0 = Release|x64
		{9B42E292-78B8-4B77-A65F-1CD55AB5D1D0}.Release|x86.ActiveCfg = Release|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Debug|x64.ActiveCfg = Debug|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Debug|x64.Build.0 = Debug|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Debug|x86.ActiveCfg = Debug|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Release|x64.ActiveCfg = Release|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Release|x64.Build.0 = Release|x64
		{6D0C24DF-FC4B-4BA1-9E20-F38903BA14AE}.Release|x86.ActiveCfg = Release|x64
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {D406B777-1065-4D58-B716-7FC29A94B7C2}
	EndGlobalSection
EndGlobal
```
