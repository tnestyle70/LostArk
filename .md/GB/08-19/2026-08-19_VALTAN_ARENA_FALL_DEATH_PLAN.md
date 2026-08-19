# 2026-08-19 발탄 아레나 낙사 계획

바닥 붕괴 G1은 `.md/GB/08-16/2026-08-16_VALTAN_ARENA_FLOOR_COLLAPSE_RESULT.md`,
바닥 파편은 `.md/GB/08-18/2026-08-18_VALTAN_FLOOR_COLLAPSE_DEBRIS_RESULT.md`가 정본이다.
이 문서는 08-16 PLAN §5가 G2로 미뤄 둔 낙사를 닫는다.

## 1. 이번 G의 경계

무너진 바닥은 지금 통행 불가일 뿐이다. 그 위에 서 있던 플레이어는 아무 일도 겪지 않는다.
이번 작업은 **바닥이 사라진 자리에 남은 플레이어를 서버 권위로 떨어뜨려 죽이고, 기존 부활
경로로 되돌리는 것**까지다.

포함하지 않는 것:

- 보스와 몬스터의 낙사. Valtan은 무너진 바닥 위를 지나갈 수 있고 이번 범위가 아니다.
- 낙하 전용 애니메이션 clip 제작. 6 class 모두 낙하 clip이 없다(§2.5).
- 다른 Area의 낙사. `.navblockers`를 가진 Area는 Valtan 하나뿐이다.

## 2. 실측한 현재 상태

### 2.1 낙사 코드는 0줄이다

`FALLING`, `fallVolume`, `fallSequence` 전부 `Server/`, `Shared/`, `Client/`에서 grep 0건이다.

### 2.2 구멍의 범위는 이미 셀 단위로 저작돼 있다

`Data/Navigation/LV_LUT_HEARTRB_ED.navblockers`는 region 103개를 갖는다.

```text
벽   97개  activateWhenConditionTrue=0   서 있는 동안 셀을 막는다
바닥  6개  activateWhenConditionTrue=1   무너진 뒤에 셀을 막는다
```

바닥 6개의 실측 범위는 다음과 같다. grid `392x312`, cellSize `0.5`, origin `(-6, -165)` 기준이다.

```text
floor84.rail.7000000000000000001   347 cells   worldX[142.0,171.5] worldZ[-128.5,-106.5]
floor84.rail.7000000000000000005   325 cells   worldX[139.5,170.0] worldZ[-138.0,-116.0]
floor30.brick.7000000000000000002  365 cells   worldX[159.5,169.5] worldZ[-126.5,-110.0]
floor30.brick.7000000000000000003  427 cells   worldX[143.0,161.5] worldZ[-120.0,-109.0]
floor30.brick.7000000000000000006  360 cells   worldX[142.5,153.0] worldZ[-134.5,-118.5]
floor30.brick.7000000000000000007  425 cells   worldX[150.5,168.5] worldZ[-135.5,-124.5]
                                   ----------
                                   2249 cells  (84단계 672 + 30단계 1577)
```

### 2.3 필요한 질의가 이미 public API로 있다

`Server/Public/ServerNavigation.h:59`의 `Is_PointWalkableExact(float, float)`는 제품 호출자가
없고 계약 테스트만 쓴다. 다만 이 함수 하나로는 낙사를 판정할 수 없다. 벽 region이 막은 셀과
바닥 region이 막은 셀을 구분하지 못하기 때문이다.

### 2.4 서버 상태 계약

`PLAYER_ACTION_STATE`는 `Shared/Public/Network/PacketMessages.h:393`에서 이미 `std::uint8_t`이고
`NONE / SKILL / TRIGGER_MOVE / DEAD / END`다.

이동과 스킬 입력은 이미 `NONE != eAction`에서 닫힌다.

```text
Server/Private/GameRoom.cpp:1071         Handle_Move     NONE != eAction 이면 목표를 받지 않는다
Server/Private/PlayerSkillSystem.cpp:293 Try_Start       NONE != eAction 이면 false (COMBO 예외는 SKILL 한정)
Server/Private/GameRoom.cpp:3705         Update_Players  NONE != eAction 이면 이동 자체를 건너뛴다
```

타게팅과 광역 피격은 네 곳 모두 `!isCombatReady`를 본다.

```text
Server/Private/ValtanBrain.cpp:637   대상 획득
Server/Private/ValtanBrain.cpp:526   ApplyPatternHit  (DEAD 는 보지 않고 HP 와 isCombatReady 만 본다)
Server/Private/MonsterBrain.cpp:57   대상 획득
Server/Private/GameRoom.cpp:1735     Has_EngagedAuditionPlayer
```

Y축 이동 선례가 있다. `Server/Private/ServerTriggerSystem.cpp:88`의 `TRIGGER_MOVE`가
`fPositionY`를 포물선으로 움직이고, `Client/Private/ClientReplication.cpp:1259`가 그 Y를
그대로 캐릭터 transform에 적용한다.

### 2.5 낙하 애니메이션은 없다

`Client/Public/CharacterSpec.h:14`의 `CHARACTER_ANIM`은 `IDLE, RUN, HIT, DEAD` 넷뿐이고
6 class의 `AnimationClips`도 정확히 네 개씩이다. `Data/Animation/Reference/<Class>/<Class>.clipmap`
전수 검색에서 fall/land/drop 계열 clip이 0건이다.

### 2.6 부활은 제자리다

`Server/Private/GameRoom.cpp:1185` `Handle_RevivePlayer`는 HP, 자원, action, stance, cooldown을
복구하지만 **위치를 옮기지 않는다.** 구멍에서 죽으면 구멍에서 부활해 즉시 다시 떨어진다.

### 2.7 프로토콜 버전은 23이다

`Shared/Public/Network/PacketType.h:8`이 `NETWORK_PROTOCOL_VERSION = 23`이다.
08-16 PLAN §5-6이 적은 `21 -> 22`는 그 뒤 인벤토리 작업이 올린 값을 반영하지 않은 낡은 숫자다.

## 3. 08-16 PLAN §5와 달라지는 점

| 08-16 PLAN | 이번 계획 | 이유 |
|---|---|---|
| `Gameplay.world.json`에 `fallVolume` kind 추가 | 추가하지 않는다 | 구멍 범위 2249 cell이 이미 `.navblockers`에 있다. shape를 따로 저작하면 정본이 둘이 되고 반드시 어긋난다 |
| `PLAYER_SNAPSHOT`에 `fallSequence`, `fallStartTick`, 시작 위치, 초기 속도, 중력, `deathTick`, `fallVolumeId` 추가 | 새 필드 0개 | Client가 필요한 것은 `eAction`, 기존 `iActionStartTick`, 기존 `fPositionY` 셋뿐이다. 나머지는 전부 서버 내부 상태다 |
| collapse commit transaction 안에서 발밑 스캔 + 이후 swept segment 별도 처리 | `Update_Players` 매 tick 1회 검사 | 서 있다 무너지는 경우, 밀려 들어가는 경우, 스킬 이동으로 들어가는 경우가 한 코드로 닫힌다. 1 tick(33 ms) 늦지만 폭 7~14 m 고리를 33 ms에 통과하는 이동은 없다 |
| 프로토콜 `21 -> 22` | 올리지 않는다 (`23` 유지) | 현재 값은 23이고 `PLAYER_SNAPSHOT` 레이아웃이 바뀌지 않는다. 08-18 RESULT §8.3이 정한 "레이아웃 변경일 때만 올린다"는 판례를 따른다. 팀은 각자 자기 PC의 `127.0.0.1:7777`에 서버를 띄우므로 서로 다른 버전이 만나는 조합 자체가 나오지 않는다 |

## 4. G1 — Shared 계약

### 4.1 목표와 종료 증거

`PLAYER_ACTION_STATE`에 `FALLING`을 더하고, 그 상태의 snapshot이 유효한 조건을 못 박는다.
종료 증거는 `NetworkProtocolHarness` `failures : 0`이며 새 round-trip 3건이 포함된다.

### 4.2 `Shared/Public/Network/PacketType.h` — 바꾸지 않는다

`NETWORK_PROTOCOL_VERSION`은 `23` 그대로 둔다.

`C2S_ENTER_WORLD`의 wire는 바뀌지 않고 `PLAYER_SNAPSHOT`의 필드도 하나도 늘지 않는다.
`PLAYER_ACTION_STATE`에 값을 하나 더하는 것은 `std::uint8_t` 한 칸 안의 일이라 레이아웃
변경이 아니다. 08-18 RESULT §8.3이 `BREAK_EVERY_WALL`을 두고 정한 기준이 그대로 적용된다.

버전을 올리지 않을 때 남는 실패 모드는 하나뿐이다. 같은 PC에서 Server와 Client 중 한쪽만
다시 빌드하면, 접속은 되지만 첫 낙사 snapshot에서 구버전 쪽 reader가 `rawAction >= END`로
그 프레임을 버린다. 팀은 각자 자기 PC의 `127.0.0.1:7777`에 서버를 띄우고
`Framework.slnLaunch`의 `Server + Client` profile이 둘을 함께 빌드하므로, 이 조합은 반쪽
빌드를 했을 때만 나온다. 따라서 PR 설명에 **Shared 변경이므로 Shared -> Server -> Client를
모두 다시 빌드해야 한다**고 적는 것으로 닫는다.

### 4.3 `Shared/Public/Network/PacketMessages.h`

```text
파일: Shared/Public/Network/PacketMessages.h
작업: 교체
기준점: 393번 줄 enum class PLAYER_ACTION_STATE 전체
위치: DEAD 와 END 사이에 FALLING 삽입
필요한 이유: 낙하 중임을 Client 에 알리는 유일한 값
연결되는 부분: Is_Valid_PlayerAction, Is_Valid_PlayerSnapshot, CGameRoom::Update_PlayerFall, CCharacter
```

`FALLING`을 **`DEAD` 뒤에** 넣는다. 앞에 넣으면 `DEAD`가 3에서 4로 바뀌어 기존 wire 값이 전부
어긋난다.

```cpp
	enum class PLAYER_ACTION_STATE : std::uint8_t
	{
		NONE,
		SKILL,
		TRIGGER_MOVE,
		DEAD,
		/* The authored ground under the player was removed by a collapse. The
		server owns the descent and the death tick; the snapshot carries only
		this state and the position it already sends, so no field is added.
		Appended after DEAD so every wire value that exists today keeps its
		number. */
		FALLING,
		END
	};
```

### 4.4 `Shared/Private/Network/PacketMessages.cpp`

```text
파일: Shared/Private/Network/PacketMessages.cpp
작업: 교체
기준점: Is_Valid_PlayerSnapshot 의 마지막 조건 블록
        (SKILL == snapshot.eAction 로 시작해 함수의 세미콜론으로 끝나는 부분)
필요한 이유: FALLING 은 skillId 를 갖지 않고 반드시 시작 tick 을 갖는다.
             그래야 late join 이 낙하 경과를 seek 할 수 있다
연결되는 부분: NetworkProtocolHarness, CClientReplication
```

기존 마지막 세 갈래를 네 갈래로 바꾼다.

```cpp
			((LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction &&
				snapshot.iSkillId != LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 (LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 /* A fall is timed: the client seeks the descent from this tick when
			 it joins late, so a FALLING snapshot without one is malformed. */
			 (LostArk::Shared::PLAYER_ACTION_STATE::FALLING == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 ((LostArk::Shared::PLAYER_ACTION_STATE::SKILL != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::FALLING != snapshot.eAction) &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID));
```

### 4.5 `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`

기존 player snapshot round-trip 테스트 옆에 3건을 더한다.

```text
Falling Player Snapshot Round Trip              FALLING + skillId 무효 + actionStartTick 유효 → 왕복 일치
Reject A Falling Snapshot That Carries A Skill  FALLING + skillId 유효 → 거부
Reject A Falling Snapshot Without A Start Tick  FALLING + actionStartTick 0 → 거부
```

## 5. G2 — 구멍 판정 기반

### 5.1 문제

`Is_PointWalkableExact`는 "지금 못 걷는다"만 답한다. 벽 안인지 구멍 위인지 구분하지 못한다.
넉백으로 벽에 박힌 플레이어가 낙사하면 안 되므로 구분이 필요하다.

지금 바닥/벽 구분은 `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1:60`의
`destroyable.group.valtan.floor` groupId 접두사 규칙 하나뿐이다. 런타임에는 어디에도 없다.

### 5.2 선택한 방법

**destruction bootstrap의 mutation 행에 `removesGround` 플래그를 명시**하고, Server가 그
조건 ID 집합을 navigation에 넘긴다. navigation은 셀별 void count를 기존 block count와 같은
방식으로 유지한다.

`.navblockers` 포맷을 바꾸지 않는 이유는 그 파일을 Server 파서, Client
`NavRuntimeBlockerDocument`(read/write), MapTool UI, publisher 넷이 함께 쓰기 때문이다.
바닥/벽 판단은 이미 Valtan destruction publisher가 소유하고 있으므로 그 계약에 싣는 편이
변경 표면이 작다.

### 5.3 `Server/Public/WorldDestructionRuntime.h`

```text
파일: Server/Public/WorldDestructionRuntime.h
작업: 추가
기준점: struct WORLD_DESTRUCTION_MUTATION_DESCRIPTOR 의 strNavigationStateId 멤버
위치: strNavigationStateId 바로 아래, 닫는 }; 바로 위
필요한 이유: 이 mutation 이 장애물을 없애는지 발밑을 없애는지가 낙사 판정의 유일한 근거다
연결되는 부분: CWorldDestructionBootstrap 파서, CGameRoom::Initialize
```

```cpp
	struct WORLD_DESTRUCTION_MUTATION_DESCRIPTOR final
	{
		std::string strMutationId;
		std::string strGroupId;
		WORLD_DESTRUCTION_STATE eFinalState =
			WORLD_DESTRUCTION_STATE::FRACTURED;
		std::uint32_t iBreakingDurationTicks = 0u;
		std::string strCollisionStateId;
		std::string strNavigationStateId;
		/* True when applying this mutation removes the ground the players stand
		on instead of an obstacle beside them. Only a mutation that owns a
		navigation condition may set it, because the hole is that condition's
		cells. */
		bool bRemovesGround = false;
	};
```

### 5.4 `Server/Private/WorldDestructionBootstrap.cpp`

헤더 version `1` -> `2`, mutation 행 필드 7개 -> 8개.

```text
파일: Server/Private/WorldDestructionBootstrap.cpp
작업: 교체 (2곳)
기준점 1: 199번 줄 !ParseNumber(header[1], version) || 1u != version
기준점 2: 299번 줄 if (7u != fields.size() || "M" != fields[0] || 로 시작하는 검증 블록
필요한 이유: 새 8번째 필드를 읽고, v1 문서는 추측 변환 없이 거부한다
연결되는 부분: Publish-ValtanWorldDestruction.ps1, WorldDestructionBootstrapContractTests.cpp
```

헤더 검증:

```cpp
		!ParseNumber(header[1], version) || 2u != version ||
```

mutation 행 검증 블록 전체 교체:

```cpp
		const std::vector<std::string_view> fields = SplitTabs(line);
		WORLD_DESTRUCTION_MUTATION_DESCRIPTOR mutation;
		std::uint32_t removesGround = 0u;
		if (8u != fields.size() || "M" != fields[0] ||
			!Is_StableId(fields[1]) || !Is_StableId(fields[2]) ||
			!ParseState(fields[3], mutation.eFinalState, false) ||
			!ParseNumber(fields[4], mutation.iBreakingDurationTicks) ||
			mutation.iBreakingDurationTicks >
				static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()) ||
			("-" != fields[5] && !Is_StableId(fields[5])) ||
			("-" != fields[6] && !Is_StableId(fields[6])) ||
			!ParseNumber(fields[7], removesGround) || 1u < removesGround ||
			/* A hole is the navigation condition's cells. A mutation that claims
			to remove ground without owning one has nothing to open. */
			(0u != removesGround && "-" == fields[6]))
		{
			m_strStatus = "World destruction mutation row is invalid";
			return false;
		}
```

기존 `if ("-" != fields[6]) mutation.strNavigationStateId = fields[6];` 바로 아래에 한 줄:

```cpp
		mutation.bRemovesGround = 0u != removesGround;
```

### 5.5 `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`

헤더 `1` -> `2`, mutation 행 끝에 필드 하나.

값의 근거는 groupId 접두사가 아니라 **이미 저작돼 있는 `navPolarity`**다.
`Data/Encounters/Valtan/ValtanWorldEvents.json`의 group마다 `BLOCK_WHILE_INTACT`(벽 99개)
또는 `BLOCK_WHILE_FRACTURED`(바닥 6개) 중 하나를 갖고, publisher가 이미
`Publish-ValtanWorldDestruction.ps1:900`에서 이 값으로 navigation region의 polarity를
검증한다. 후자는 "부서진 뒤에 셀이 막힌다" 즉 발밑이 사라진다는 뜻이므로 그대로 쓴다.

```powershell
                RemovesGround = if (
                    $group.navPolarity -ceq 'BLOCK_WHILE_FRACTURED') { 1 } else { 0 }
```

검증은 두 자리로 나눈다. compile 함수에는 "removesGround=1이면 navigationStateId를 반드시
갖는다"는 per-mutation 불변식만 둔다. 절대 개수 6은 `Invoke-ContractTests`가 실제 정본
문서를 compile한 결과(`$canonical.RemovedGroundMutationCount`)에 대해 검사한다. compile
함수에 절대 개수를 넣으면 ContractTest의 합성 fixture가 통과할 수 없다.

### 5.6 `Server/Public/ServerNavigation.h`

```text
파일: Server/Public/ServerNavigation.h
작업: 추가 3곳
기준점 1: struct SERVER_NAVIGATION_CONDITION_STAGE 의 BlockCounts 멤버
기준점 2: public 구역의 Is_PointWalkableExact 선언
기준점 3: private 구역의 m_BlockCounts 멤버
필요한 이유: 구멍 셀 수를 막힌 셀 수와 같은 방식으로 유지해 질의를 O(1)로 만든다
연결되는 부분: CGameRoom::Initialize, CGameRoom::Update_PlayerFall
```

`#include <set>`를 헤더 include 목록에 더한다.

```cpp
	struct SERVER_NAVIGATION_CONDITION_STAGE final
	{
		std::uint64_t iBaseRevision = 0u;
		std::uint64_t iNextRevision = 0u;
		std::map<std::string, bool> ConditionValues;
		std::vector<std::uint16_t> BlockCounts;
		/* Parallel to BlockCounts and staged in the same transaction: how many
		active regions removed the ground under this cell, as opposed to putting
		an obstacle on it. */
		std::vector<std::uint16_t> VoidCounts;
		bool bChanged = false;
	};
```

public 선언 두 개를 `Is_PointWalkableExact` 바로 아래에 더한다.

```cpp
		/* Declares which conditions open a hole rather than clear an obstacle.
		Called once at room admission from the world destruction descriptor that
		authored them; every id must already exist and every region that uses it
		must block when the condition becomes true, or the room fails to load. */
		bool Set_VoidConditions(
			const std::set<std::string>& conditionIds,
			std::string& outStatus);
		bool Is_PointInVoidRegion(float x, float z) const;
```

private에 헬퍼와 상태를 더한다.

```cpp
		bool Is_CellVoid(std::uint32_t index) const;
```

```cpp
		std::vector<std::uint16_t> m_BlockCounts;
		std::set<std::string> m_VoidConditionIds;
		std::vector<std::uint16_t> m_VoidCounts;
```

### 5.7 `Server/Private/ServerNavigation.cpp`

`Rebuild_InitialRuntimeBlockers`는 void count도 함께 세운다.

```cpp
void LostArk::Server::CServerNavigation::Rebuild_InitialRuntimeBlockers() noexcept
{
	for (auto& [conditionId, value] : m_ConditionValues)
	{
		(void)conditionId;
		value = false;
	}
	m_BlockCounts.assign(m_Walkable.size(), 0u);
	m_VoidCounts.assign(m_Walkable.size(), 0u);
	for (const RUNTIME_BLOCKER_REGION& region : m_RuntimeBlockerRegions)
	{
		const bool conditionValue = m_ConditionValues[region.strConditionId];
		if (conditionValue != region.bActivateWhenConditionTrue)
			continue;
		const bool isVoid = m_VoidConditionIds.contains(region.strConditionId);
		for (const std::uint32_t cellIndex : region.CellIndices)
		{
			++m_BlockCounts[cellIndex];
			if (isVoid)
				++m_VoidCounts[cellIndex];
		}
	}
	m_iRevision = 1u;
}
```

`Prepare_ConditionChanges`는 `outStage.VoidCounts = m_VoidCounts;`를 `BlockCounts` 옆에서
복사하고, 셀 루프에서 `isVoid`일 때만 void count를 같은 방향으로 증감한다. overflow와
underflow 검사는 block count와 동일하게 둔다.

`Commit_ConditionChanges`는 `m_VoidCounts = std::move(stage.VoidCounts);`를 더한다.

새 함수 세 개:

```cpp
/* Ties the destruction descriptor's ground-removal claim to the navigation data
that owns the cells. It is the only place the two documents are cross-checked,
and it fails the room instead of leaving a hole nobody can fall into. */
bool LostArk::Server::CServerNavigation::Set_VoidConditions(
	const std::set<std::string>& conditionIds,
	std::string& outStatus)
{
	if (!Is_Loaded())
	{
		outStatus = "Server navigation is not loaded";
		return false;
	}
	for (const std::string& conditionId : conditionIds)
	{
		if (!m_ConditionValues.contains(conditionId))
		{
			outStatus = "Unknown navigation void condition: " + conditionId;
			return false;
		}
		bool hasRegion = false;
		for (const RUNTIME_BLOCKER_REGION& region : m_RuntimeBlockerRegions)
		{
			if (region.strConditionId != conditionId)
				continue;
			hasRegion = true;
			/* A hole appears when the condition becomes true. A region with the
			opposite polarity is an obstacle that the same condition clears, and
			calling it a hole would drop players into standing geometry. */
			if (!region.bActivateWhenConditionTrue)
			{
				outStatus = "Navigation void condition has obstacle polarity: " +
					conditionId;
				return false;
			}
		}
		if (!hasRegion)
		{
			outStatus = "Navigation void condition owns no region: " + conditionId;
			return false;
		}
	}
	m_VoidConditionIds = conditionIds;
	Rebuild_InitialRuntimeBlockers();
	outStatus = "Navigation void conditions accepted: " +
		std::to_string(m_VoidConditionIds.size());
	return true;
}

bool LostArk::Server::CServerNavigation::Is_CellVoid(
	const std::uint32_t index) const
{
	return index < m_VoidCounts.size() && 0u != m_VoidCounts[index];
}

bool LostArk::Server::CServerNavigation::Is_PointInVoidRegion(
	const float x,
	const float z) const
{
	std::uint32_t cellIndex = 0u;
	if (!Resolve_Cell(x, z, cellIndex))
		return false;
	return Is_CellVoid(cellIndex);
}
```

`Set_VoidConditions`가 `Rebuild_InitialRuntimeBlockers`를 부르므로 **반드시 조건 변경이
일어나기 전, 방 admission 시점에만** 호출한다.

### 5.8 `Server/Private/GameRoom.cpp` — admission 연결

```text
파일: Server/Private/GameRoom.cpp
작업: 교체
기준점: Initialize 안의 for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation : ...) 검증 루프
위치: 기존 루프 전체를 교체하고 루프 뒤에 Set_VoidConditions 호출을 붙인다
필요한 이유: 이미 mutation 의 navigation 조건 존재를 검사하는 자리다. 같은 루프에서 void 조건을 모은다
```

```cpp
		std::set<std::string> voidConditionIds;
		for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
			m_WorldDestructionBootstrap.Get_DescriptorGraph().Mutations)
		{
			if ((!mutation.strCollisionStateId.empty() &&
				 !m_ServerCollisionSystem.Has_CollisionStateTarget(
					 mutation.strCollisionStateId)) ||
				(!mutation.strNavigationStateId.empty() &&
				 !m_ServerNavigation.Has_Condition(
					 mutation.strNavigationStateId)))
			{
				m_strStatus = "World destruction dynamic state reference is unknown: " +
					mutation.strMutationId;
				return;
			}
			if (mutation.bRemovesGround)
				voidConditionIds.insert(mutation.strNavigationStateId);
		}
		if (!m_ServerNavigation.Set_VoidConditions(
			voidConditionIds, m_strStatus))
		{
			return;
		}
```

`#include <set>`가 필요하면 더한다.

## 6. G3 — 낙사 상태 기계

### 6.1 `Server/Public/ServerPlayer.h`

```text
파일: Server/Public/ServerPlayer.h
작업: 추가
기준점: SERVER_PLAYER 의 iActionStartTick 멤버
위치: iActionStartTick 바로 아래, SERVER_TRIGGER_MOVE TriggerMove 바로 위
필요한 이유: 낙하 속도와 사망 예정 tick 은 서버만 알면 되고 wire 에 실리지 않는다
연결되는 부분: CGameRoom::Update_PlayerFall, Handle_RevivePlayer, Reset_ValtanAuditionState
```

```cpp
		/* Live only while eAction is FALLING. The velocity integrates downward
		from zero at the tick the ground disappeared, and the death tick is the
		deadline that same tick scheduled. Neither is replicated: the client
		reads the descent from the position the snapshot already carries. */
		float fFallVelocityY = 0.f;
		std::uint32_t iFallDeathTick = 0u;
```

### 6.2 `Server/Private/GameRoom.cpp` — 상수

익명 namespace의 `Wrap_Degrees` 바로 아래, **`#ifdef _DEBUG` 앞**에 둔다. 낙사는 제품
동작이므로 Debug 전용 구역에 넣으면 Release에서 컴파일되지 않는다. 같은 이유로 사망 tick도
Debug 전용인 `Add_ServerTicksSkippingReservedZero` 대신 그 자리에서 직접 계산한다.

```cpp
	/* An arena floor collapse drops the player for one and a half seconds and
	then kills. The room steps at a fixed 30 Hz, so the deadline is 45 ticks
	after the fall begins. Gravity is the plain metric constant: the descent only
	presents a death the collapse already decided. */
	constexpr float FALL_GRAVITY_METERS_PER_SECOND_SQUARED = 9.8f;
	constexpr std::uint32_t FALL_DEATH_TICKS = 45u;
```

### 6.3 `Server/Public/GameRoom.h` — 선언

```text
파일: Server/Public/GameRoom.h
작업: 추가
기준점: private 구역의 void Update_Players(float fixedDeltaSeconds); 선언
위치: 바로 위
정의 위치: Server/Private/GameRoom.cpp 의 Update_Players 정의 바로 위
```

```cpp
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
```

### 6.4 `Server/Private/GameRoom.cpp` — `Update_PlayerFall`

한 줄 책임: 한 플레이어의 낙하 생애 주기 전체를 한 tick 안에서 소유한다. 발밑이 사라졌으면
낙하를 시작하고, 진행 중이면 적분하고, 예정 tick이 되면 기존 사망 상태로 바꾼다.
`true`를 돌려주면 그 tick에는 trigger motion, 스킬, 이동이 전혀 돌지 않는다.

```cpp
bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.fFallVelocityY -=
			FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
		player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
		/* Signed difference so a wrapped tick counter keeps ordering, the same
		rule the cooldown deadlines use. */
		const std::int32_t sinceDeadline = static_cast<std::int32_t>(
			updateTick - player.iFallDeathTick);
		if (!std::isfinite(player.fPositionY) || sinceDeadline >= 0)
		{
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
			player.fFallVelocityY = 0.f;
			player.iFallDeathTick = 0u;
		}
		return true;
	}
	if (!m_ServerNavigation.Is_Loaded() ||
		0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		!m_ServerNavigation.Is_PointInVoidRegion(
			player.fPositionX, player.fPositionZ))
	{
		return false;
	}

	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
	const std::uint32_t deadline = player.iActionStartTick + FALL_DEATH_TICKS;
	player.iFallDeathTick = 0u == deadline ? 1u : deadline;
	player.fFallVelocityY = 0.f;
	/* Everything the fall interrupts is cleared here instead of inside each
	system, so no half-finished action can resume when the body lands dead. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.Projectiles.clear();
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	/* Every boss and monster gate already refuses a player that is not combat
	ready, so this one flag removes the falling body from acquisition and from
	area damage without editing four separate target filters. */
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return true;
}
```

### 6.5 `Server/Private/GameRoom.cpp` — `Update_Players` 진입

```text
파일: Server/Private/GameRoom.cpp
작업: 추가
기준점: Update_Players 안의 for 루프 첫 문장 (void)playerId;
위치: 바로 아래, m_ServerTriggerSystem.Update_PlayerMotion 호출 바로 위
```

```cpp
		if (Update_PlayerFall(player, fixedDeltaSeconds, updateTick))
			continue;
```

### 6.6 `Server/Private/PlayerSkillSystem.cpp`

```text
파일: Server/Private/PlayerSkillSystem.cpp
작업: 추가
기준점: Update 의 DEAD 블록(693~701줄)의 닫는 } 바로 아래
필요한 이유: Update 는 Update_Players 에서 조건 없이 불린다.
             낙하 중에 자원 재생과 투사체 전진이 계속되면 안 된다
```

```cpp
	/* A falling player is still alive, so the DEAD guard above does not catch
	them. The room owns the descent this tick; nothing here may advance. */
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
		return;
```

`Update_PlayerFall`이 `true`일 때 `Update_Players`가 `continue`하므로 이 반환은 도달하지
않는 것이 정상이다. 그럼에도 두는 이유는 이 시스템이 다른 호출자에게 열려 있고, 낙하 중
플레이어를 안전한 상태로 취급한다는 계약을 이 파일 안에서 스스로 지키기 위해서다.

### 6.7 `Server/Private/GameRoom.cpp` — 부활 위치 보정

```text
파일: Server/Private/GameRoom.cpp
작업: 추가
기준점: Handle_RevivePlayer 의 const PLAYER_RUNTIME_PROFILE* profile = ... nullptr 검사 블록 바로 아래
위치: player.iCurrentHp = player.iMaximumHp; 바로 위
필요한 이유: 구멍에서 제자리 부활하면 다음 tick 에 다시 떨어진다
```

```cpp
	/* A fall kills the player over a hole. Reviving in place would drop them
	again on the next tick, so a revive whose current cell is no longer walkable
	returns to the spawn this player entered from. Every other death still
	revives exactly where it happened. */
	if (m_ServerNavigation.Is_Loaded() &&
		!m_ServerNavigation.Is_PointWalkableExact(
			player.fPositionX, player.fPositionZ))
	{
		const WORLD_BOOTSTRAP_PLACEMENT* spawn =
			Find_Placement(player.strSpawnPlacementId);
		SERVER_NAV_POINT projected{};
		if (nullptr == spawn || !spawn->isEnabled ||
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != spawn->eKind ||
			!m_ServerNavigation.Project_Point(
				spawn->fPositionX, spawn->fPositionZ, projected))
		{
			return;
		}
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
		player.fYawDegrees = spawn->fYawDegrees;
	}
```

같은 함수의 상태 초기화 목록에 두 줄을 더한다.

```cpp
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
```

### 6.8 `Server/Private/GameRoom.cpp` — audition reset

```text
파일: Server/Private/GameRoom.cpp
작업: 추가
기준점: Reset_ValtanAuditionState 안의 Invalidate_DynamicNavigationPaths(); 호출
위치: 바로 위
필요한 이유: 낙하 중에 Reset 을 누르면 바닥은 돌아오지만 플레이어는 계속 떨어진다
```

```cpp
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::FALLING != player.eAction)
			continue;
		player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0u;
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;
		/* The reset already put the floor back, so the same XZ projects onto
		solid ground again. */
		SERVER_NAV_POINT ground{};
		if (m_ServerNavigation.Is_Loaded() &&
			m_ServerNavigation.Project_Point(
				player.fPositionX, player.fPositionZ, ground))
		{
			player.fPositionX = ground.x;
			player.fPositionY = ground.y;
			player.fPositionZ = ground.z;
		}
	}
```

### 6.9 `Server/Private/ServerGameplayContractTests.cpp`

```text
Drop a player into a falling state on the tick after the stage-A floor sector collapses
Kill a falling player exactly at the authored death tick and leave the revive path intact
Keep Valtan from acquiring or damaging a player while that player is falling
Revive a fall death at the entered player spawn instead of inside the hole
Leave a player on the intact core untouched when the stage-B ring collapses
```

`Server/Private/WorldDestructionBootstrapContractTests.cpp`에 더한다.

```text
Accept a v2 bootstrap whose six floor mutations declare removed ground
Reject a v1 bootstrap instead of guessing the ground flag
Reject a mutation that claims removed ground without a navigation condition
```

## 7. G4 — Client 표현

### 7.1 `Client/Private/Character.cpp`

```text
파일: Client/Private/Character.cpp
작업: 추가
기준점: else if (PLAYER_ACTION_STATE::DEAD == action) 분기
위치: 그 분기 바로 위
필요한 이유: 낙하 clip 이 없으므로 피격 경직 루프로 대신하고, 실제 하강은 복제된 Y 가 그린다
연결되는 부분: CClientReplication 이 넘기는 snapshot 의 eAction
```

```cpp
	else if (PLAYER_ACTION_STATE::FALLING == action)
	{
		if (INVALID_SKILL_ID != skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action)
			return true;
		m_pChain = nullptr;
		m_iChainStage = 0;
		m_iChainStep = 0;
		m_fActionPresentationSeconds = 0.f;
		Commit_PendingClipChains();
		/* No class owns a falling clip, so the damaged-idle loop plays while the
		server drives the body down. The descent itself is the replicated Y, not
		an animation. */
		Set_Animation(CHARACTER_ANIM::HIT, true);
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
	}
```

### 7.2 Client 하네스 테스트는 넣지 않는다

`ClientFrontendHarness`는 `Client/Private/Character.cpp`를 컴파일하지 않는다. 이 하네스에
`PLAYER_ACTION_STATE`를 쓰는 코드도 0줄이다. 테스트를 넣으려면
`ClientFrontendHarness.vcxproj`에 source를 추가해야 하는데, `.vcxproj`와 `.filters`는
팀장이 병합할 때 충돌하는 파일이라 이번 변경에서 건드리지 않는다.

따라서 Client의 `FALLING` 분기 검증은 **빌드 성공 + 사용자의 실제 화면 확인**이다.
§8.2 절차의 4번과 7번이 그 확인이다.

## 8. 검증 계획

### 8.1 자동

```text
1. Publish-ValtanWorldDestruction.ps1  -Mode Validate / -Mode ContractTest / Publish
   기대: mutations=105, removesGround=1 이 정확히 6
2. Shared + NetworkProtocolHarness x64 Debug 빌드/실행, failures : 0
3. Server x64 Debug 빌드, Server.exe --contract-test failures : 0
4. Client x64 Debug 빌드, ClientFrontendHarness 신규 실패 0건
   (Effect 계열 기존 실패는 main 이 열어 둔 별건이다)
   Client FALLING 분기 자체는 하네스 대상이 아니다 (§7.2)
5. 변경 JSON parse, git diff --check, 편집한 C++ 파일 괄호 수지 +0
```

`Publish-ServerNavigation.ps1`은 이번 변경에 포함되지 않는다. `.navblockers` 포맷을 바꾸지
않기 때문이다. 회귀 확인용으로 `-Mode Validate`만 돌린다.

### 8.2 사용자 수동 (에이전트가 대신 판정하지 않음)

`Play Selected (Keep Broken)`은 플레이어를 순간이동시키지 않는다(`isOneClickPlay`에 ARM/CROSS가
빠져 있다). 따라서 원하는 자리에 서 있다가 무너뜨릴 수 있다.

```text
1. Server + Client 실행, Lobby -> Valtan
2. Reset + Break Every Wall (Keep Floor)    벽만 사라지고 바닥 6개는 그대로
3. 우클릭으로 바깥 테두리 링 위로 이동       (반경 13~16 m, 84 단계 자리)
4. 체력바 84 선택 -> Play Selected (Keep Broken)
   기대: 발밑 링이 사라지고 곧바로 아래로 떨어진다. 1.5초 뒤 사망 자세로 바뀐다
5. 부활                                      기대: 구멍이 아니라 진입 spawn 위치에서 살아난다
6. 다시 벽돌 링 위로 이동 (반경 7~14 m)
7. 체력바 30 선택 -> Play Selected (Keep Broken)   기대: 4번과 같다
8. 중앙 코어에 선 채로 84/30 재생             기대: 떨어지지 않는다
9. 낙하 도중 Reset Arena State                기대: 바닥이 돌아오고 그 자리에 다시 선다
```

## 9. 남는 불확실성

- **낙하 1.5초와 즉사는 원작 근거가 없는 결정이다.** 발탄 아레나 낙사가 즉사인지 큰 피해인지
  원본 데이터로 확인하지 않았다. 08-18 RESULT가 84 패턴 타이밍을 두고 적은 것과 같은 종류의
  미확인 항목이며, 근거가 나오면 `FALL_DEATH_TICKS` 하나로 교정할 수 있다.
- **낙하 clip이 없어 피격 경직 루프로 대신한다.** 화면에서 어색하면 clip 추출이 선행되는
  별도 작업이 된다.
- **보스와 몬스터는 떨어지지 않는다.** Valtan은 무너진 바닥 위를 계속 지나갈 수 있다.
- **프로토콜 버전은 23 그대로다.** 레이아웃이 바뀌지 않아 올리지 않지만, Shared를 건드리므로
  Shared -> Server -> Client를 모두 다시 빌드해야 한다. PR 설명에 반드시 적는다. 한쪽만 빌드하면
  낙사 순간 그 프레임이 버려진다.
