# Valtan 109 Arena Break Full Wall Collapse Plan

## 목표

발탄 2페이즈 진입 컷신(`VALTAN_ARENA_BREAK_109` / `IMPACT`)에서 아레나 외곽 링 30개만
날아가고, 아레나 지형 위에 아직 서 있는 내부 벽들은 그대로 남는다. 이 컷신이 외곽 링과
함께 남아 있는 모든 내부 벽까지 한 번에 무너뜨리도록 기존 제품 경로를 연결한다.

## 실측한 현재 상태

`Server/Bin/DataFiles/World/VALTAN_ARENA.worlddestructionbootstrap`과
`Data/Encounters/Valtan/ValtanWorldEvents.json`을 직접 읽어 확인한 값이다.

```text
파괴 group 105개
  outerwall109  30   STAGE_ENTER(109/IMPACT) enabled + DASH_CHARGE impact
  wall          57   COLLIDER_CONTACT만 enabled
                     STAGE_ENTER(109/IMPACT) 57개가 enabled=false 로 존재
  wall159       10   COLLIDER_CONTACT + 오프닝 charge + DASH_CHARGE impact
                     STAGE_ENTER(109/IMPACT) 10개가 enabled=false 로 존재
  entrance       2   첫 등장 sweep 전용
  floor30/84     6   84/30 바 지형 붕괴 전용
```

문서 내 유일한 `false` 값 67개가 정확히 이 휴면 STAGE 바인딩이며, 각각 서로 다른 group을
가리킨다. `Publish-ValtanWorldDestruction.ps1:53` 주석도 "Interior groups stay authored but
dormant until the earlier pattern that actually destroys them is identified"로 같은 사실을
기록하고 있다. 즉 이번 작업은 새 데이터를 만드는 것이 아니라 이미 저작된 휴면 계약을 켜는
것이다.

내부 벽의 위치도 확인했다. 아레나 중심은 외곽 링 30개의 평균인 `x=156.030 z=-122.060`,
링 반지름은 정확히 `16.10`이고 바닥 `y=23.04`다.

```text
wall      57 group / 65 placement   반지름 4.57 ~ 14.09   y 23.04
wall159   10 group / 10 placement   반지름 9.43 ~ 12.96   y 23.04
entrance   2 group /  2 placement   반지름 12.97 ~ 14.41  y 23.02~23.04
floor      6 group /  6 placement   반지름 0.26          y 23.24
```

## 범위 결정

- 포함: `wall` 57 + `wall159` 10 = 67 group. 아레나 바닥 위에 서 있는 벽이고, 휴면
  STAGE 바인딩이 이미 저작되어 있다.
- 제외: `floor30`/`floor84` 6 group. 벽이 아니라 바닥이며 84/30 바 지형 붕괴 패턴이
  소유한다. 109에서 같이 없애면 아레나 발판을 한 체력 단계 일찍 잃는다.
- 제외: `entrance` 2 group. 첫 등장 sweep이 전투 시작 전에 부수며 휴면 STAGE 바인딩이
  저작되어 있지 않다. 없는 계약을 추측으로 만들지 않는다.

## 발견한 두 개의 실제 상한

데이터만 켜면 되는 작업이 아니었다. 97개를 한 tick에 처리할 때 두 곳에서 fail-close된다.

1. `Shared/Public/Network/PacketMessages.h`의 `MAX_WORLD_DESTRUCTION_EVENTS = 64`.
   `GameRoom.cpp:8536`이 `eventCount > MAX_WORLD_DESTRUCTION_EVENTS`이면 false를 반환하므로
   97개 batch는 `Apply_WorldDestructionStageEntry` 자체가 실패한다. 형제 상수
   `MAX_WORLD_DESTRUCTION_GROUPS`/`MAX_WORLD_DESTRUCTION_CHANGED_STATES`는 이미 128이다.
2. `Client/Public/WorldDestructionDebrisPresentationRuntime.h`의
   `MAX_ACTIVE_ACTORS = OUTER_RING_EMITTERS(30) * ACTORS_PER_EMITTER(12) = 360`.
   97 emitter × 12 조각 = 1,164 actor가 필요하므로 예산이 잘려 2/3의 벽이 잔해 없이
   사라진다. debris recipe 8종은 모두 정확히 12조각이라 조각 수 자체는 문제가 없다.

## 파일 목록

| 구분 | 경로 | 역할 |
|---|---|---|
| 수정 | `Data/Encounters/Valtan/ValtanWorldEvents.json` | 휴면 STAGE 바인딩 67개 활성화 |
| 수정 | `Data/Valtan/Valtan.worldeventsets.json` | 109 event set에 member 67개 추가 |
| 수정 | `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | member ID 계열 일반화, nav closure 완화 |
| 수정 | `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1` | 내부 벽 계약과 음성 테스트 교체 |
| 수정 | `Shared/Public/Network/PacketMessages.h` | 파괴 live-event 상한 64 → 128 |
| 수정 | `Shared/Public/Network/PacketType.h` | protocol 39 → 40 |
| 수정 | `Client/Public/WorldDestructionDebrisPresentationRuntime.h` | 잔해 예산 360 → 1,164 actor |
| 수정 | `Client/Private/Level_ValtanArena.cpp` | Debug 진단 반전과 안내 문구 |
| 수정 | `Server/Private/WorldDestructionBootstrapContractTests.cpp` | 30/60 → 97/135 계약 |
| 수정 | `Server/Private/ServerGameplayContractTests.cpp` | live-event batch 계약 |
| 수정 | `CLAUDE.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | protocol v40 반영 |
| 생성물 | `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction*.json` | publisher 재생성 |
| 생성물 | `Server/Bin/DataFiles/World/VALTAN_ARENA.worlddestructionbootstrap` | publisher 재생성(Git 제외) |

새 C++ 파일이 없으므로 `.vcxproj` / `.vcxproj.filters` 등록 변경은 없다.

## 교체 블록

### 1. `Data/Encounters/Valtan/ValtanWorldEvents.json`

문서 전체에서 `"enabled":  false` 67줄을 `"enabled":  true`로 바꾼다. 이 값들이 문서의
유일한 `false`이고 전부 `VALTAN_ARENA_BREAK_109 / IMPACT / STAGE_ENTER`,
`offsetMs 0`, `receiverCollisionId ""`이므로 다른 계약을 건드리지 않는다.

### 2. `Data/Valtan/Valtan.worldeventsets.json`

`worldeventset.valtan.arena-break-109.outer-wall`의 `members`에 67개를 이어 붙인다.
set ID는 저장 계약이므로 이름을 바꾸지 않는다. 각 member는 원본 flat 바인딩에서 그대로
투영한다.

```json
{
  "memberId": "worldeventmember.valtan.wall.10435996578153793381",
  "bindingId": "binding.valtan.wall.10435996578153793381.preview",
  "groupId": "destroyable.group.valtan.wall.10435996578153793381",
  "mutationId": "mutation.valtan.wall.10435996578153793381.despawn",
  "offsetMs": 0,
  "receiverCollisionId": "",
  "enabled": true
}
```

### 3. `Tools/ValtanPipeline/valtan_tuning_pipeline.py`

`_world_member_id`는 `outerwall109.` 접두사만 받아 다른 계열에서 예외를 던진다. 세 계열을
받도록 바꾼다. `wall159.`를 `wall.`보다 먼저 두는 것은 가독성 때문이고, 두 접두사가 같은
group ID에 동시에 걸리는 경우는 없다.

```python
WORLD_MEMBER_GROUP_PREFIXES = (
    "destroyable.group.valtan.outerwall109.",
    "destroyable.group.valtan.wall159.",
    "destroyable.group.valtan.wall.",
)
_WORLD_GROUP_NAMESPACE = "destroyable.group.valtan."


def _world_member_id(group_id: str) -> str:
    for marker in WORLD_MEMBER_GROUP_PREFIXES:
        if group_id.startswith(marker):
            family = marker[len(_WORLD_GROUP_NAMESPACE) : -1]
            return f"worldeventmember.valtan.{family}." + group_id[len(marker) :]
    raise PipelineError(f"109 managed group has unexpected ID: {group_id}")
```

`validate_world_event_sets`의 group closure 검사는 `navigationRegionIds`가 비면 실패한다.
내부 벽 57개 중 2개(`wall.15675921917269125843`, `wall.13221745590928612509`)는 nav region이
없다. 이는 손상이 아니라 `Publish-ValtanWorldDestruction.ps1:951`이 문서화한 정상 상태다
("A wall with a cliff behind it authors none, because removing it must not make the drop
walkable"). placement가 없는 것만 치명으로 남긴다.

```python
            placements = group.get("memberPlacementIds")
            nav_regions = group.get("navigationRegionIds")
            # A member with no placement has nothing to remove, so that stays
            # fatal. An empty navigation closure does not: the destruction
            # publisher authors none for a wall with a cliff behind it, because
            # breaking that wall must not turn the drop into walkable floor.
            if not isinstance(placements, list) or not placements or not isinstance(nav_regions, list):
                raise PipelineError(f"group closure is incomplete for {member_id}")
```

### 4. `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`

상수 블록에 내부 벽 계약을 추가하고, `$interior109BindingCount -ne 0` 게이트를 실제 형태
검사로 바꾼다. 접두사를 명시적으로 나열하는 것이 floor/entrance가 "outer가 아니다"라는
이유만으로 편승하는 것을 막는다.

```powershell
$interiorGroupIdPrefixes = @(
    'destroyable.group.valtan.wall.',
    'destroyable.group.valtan.wall159.')
$expectedInterior109BindingCount = 67
```

```powershell
        if ($interior109BindingCount -ne $expectedInterior109BindingCount -or
            $interior109GroupIds.Count -ne $expectedInterior109BindingCount) {
            throw "The 109 collapse must also reach exactly $expectedInterior109BindingCount interior wall groups, not $($interior109GroupIds.Count)."
        }
```

음성 계약 테스트 `interior group rejoining the 109 batch`는 이제 정상 입력이므로,
새 경계를 지키는 두 개로 교체한다.

- `interior wall dropping out of the 109 batch`: 내부 벽 하나를 disable하면 거부
- `floor sector riding the 109 collapse`: floor 바인딩을 109로 옮기면 거부

### 5. `Shared/Public/Network/PacketMessages.h` / `PacketType.h`

```cpp
	// The 109 collapse breaks the outer ring and every interior wall on one
	// edge, so this has to reach the whole graph the way its two siblings do.
	inline constexpr std::size_t MAX_WORLD_DESTRUCTION_EVENTS = 128;
```

```cpp
	/* 40 raises the world destruction live-event bound so the 109 collapse
	can carry the outer ring and every interior wall in one delta. 39 adds
	bounded Debug Valtan pattern-flow authoring playback. 38 adds typed
	boss-owned player attachments. */
	inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 40;
```

### 6. `Client/Public/WorldDestructionDebrisPresentationRuntime.h`

예산의 불변식("모든 벽이 온전한 recipe 하나를 갖는다")을 유지한 채 batch 크기를 따라간다.

```cpp
	static constexpr uint32_t OUTER_RING_EMITTERS = 30u;
	static constexpr uint32_t INTERIOR_WALL_EMITTERS = 67u;
	static constexpr uint32_t ARENA_COLLAPSE_EMITTERS =
		OUTER_RING_EMITTERS + INTERIOR_WALL_EMITTERS;
	static constexpr uint32_t MAX_ACTIVE_ACTORS =
		ARENA_COLLAPSE_EMITTERS * ACTORS_PER_EMITTER;
```

## 실패 경계

- 잘못된 개수나 계열은 publish 전에 거부한다. floor/entrance가 109 batch에 들어오면 throw.
- 이미 부서진 벽에 109 stage가 도달하면 파괴는 단방향이므로 `NO_CHANGE`로 답하고 그대로 둔다.
- Server는 clip/model을 모르고 Client는 파괴 판정을 만들지 않는다. Client는 group 상태
  전이를 보고 잔해만 재생한다.
- 화면의 최종 자연스러움과 1,164 actor의 체감 성능은 사용자가 `Lobby -> Valtan`에서 직접
  판정한다.

## 검증

- `valtan_tuning_pipeline validate`
- `Publish-ValtanWorldDestruction.ps1` `-Mode Validate` / `-Mode ContractTest` / `-Mode Publish`
- `Tools.ValtanPipeline.test_valtan_pattern_master_v2` 외 관련 python 계약
- `git diff --check`, 변경 JSON parse
- 사용자 빌드 후 `NetworkProtocolHarness`, `Server.exe --contract-test`, Client Debug/Release
- 사용자 실행: `Lobby -> Valtan`에서 109 컷신에 남은 벽이 함께 날아가는지 육안 확인
