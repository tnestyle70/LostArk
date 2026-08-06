# 발탄 전체 Action 튜닝·입장 보호·Revive 디테일 코드 계획서

- 작성일: 2026-08-06
- 대응 구현 계획서: `2026-08-06_VALTAN_COMPLETE_ACTION_TUNING_REVIVE_IMPLEMENTATION_PLAN.md`
- 문서 유형: 디테일 계획서
- 적용 원칙: 아래 선언과 함수 계약을 먼저 고정하고, 구현 완료 뒤 실제 반영 전문과 검증 증거를 같은 문서의 코드 정본 section에 동기화한다.

## G00. 데이터 계약

### `Data/Encounters/Valtan/ValtanEncounter.json`

pattern은 선택 정책을, stage는 실행 시간과 판정을 소유한다.

```json
{
  "patternId": "VALTAN_DASH_CHARGE",
  "displayName": "대쉬 돌진",
  "actionId": "valtan.attack.dash-charge",
  "sourceActionIds": [420604],
  "selectionMode": "NORMAL",
  "minimumHealthBar": 1,
  "maximumHealthBar": 160,
  "triggerHealthBar": 0,
  "triggerOrder": 0,
  "selectionWeight": 30,
  "maximumConsecutiveUses": 2,
  "minimumRange": 5.0,
  "maximumRange": 20.0,
  "stages": [
    {
      "stageId": "WINDUP",
      "actionId": "valtan.attack.dash-charge.windup",
      "stageKind": "WINDUP",
      "durationMs": 600,
      "hitShape": "NONE",
      "hitOuterRadius": 0.0,
      "hitInnerRadius": 0.0,
      "hitAngleDegrees": 0.0,
      "hitLength": 0.0,
      "hitHalfWidth": 0.0,
      "hitCount": 0,
      "hitIntervalMs": 0,
      "serverDamageProfileId": ""
    },
    {
      "stageId": "CHARGE",
      "actionId": "valtan.attack.dash-charge.active",
      "stageKind": "ACTIVE",
      "durationMs": 500,
      "hitShape": "BOX",
      "hitOuterRadius": 0.0,
      "hitInnerRadius": 0.0,
      "hitAngleDegrees": 0.0,
      "hitLength": 8.0,
      "hitHalfWidth": 2.5,
      "hitCount": 1,
      "hitIntervalMs": 0,
      "serverDamageProfileId": "damage.valtan.dash-charge"
    },
    {
      "stageId": "RECOVERY",
      "actionId": "valtan.attack.dash-charge.recovery",
      "stageKind": "RECOVERY",
      "durationMs": 900,
      "hitShape": "NONE",
      "hitOuterRadius": 0.0,
      "hitInnerRadius": 0.0,
      "hitAngleDegrees": 0.0,
      "hitLength": 0.0,
      "hitHalfWidth": 0.0,
      "hitCount": 0,
      "hitIntervalMs": 0,
      "serverDamageProfileId": ""
    }
  ]
}
```

불변식은 다음과 같다.

- `sourceActionIds`는 비어 있지 않고 pattern 내부에서 중복되지 않는다. 공용 Action은 서로 다른
  semantic pattern에서 재사용할 수 있다.
- `stages`는 1개 이상이며 index는 JSON 순서다.
- `NONE` stage는 모든 shape 수치, hit count, damage ID가 0/빈 값이다.
- damaging stage는 damage profile이 존재하고 마지막 hit가 `durationMs` 안에 들어온다.
- scripted pattern은 normal weight/repeat 필드가 0이고 trigger bar/order가 유효하다.

## G01. Server catalog H 계약

### `Server/Public/GameplayCatalog.h`

추가 enum과 구조체는 다음 책임을 가진다.

```cpp
enum class BOSS_PATTERN_STAGE_KIND
{
	WINDUP,
	ACTIVE,
	RECOVERY
};

enum class BOSS_PATTERN_HIT_SHAPE
{
	NONE,
	CIRCLE,
	RING,
	CONE,
	BOX,
	CROSS
};

struct BOSS_PATTERN_STAGE_DEFINITION
{
	std::string strStageId;
	std::string strActionId;
	std::string strDamageProfileId;
	BOSS_PATTERN_STAGE_KIND eStageKind = BOSS_PATTERN_STAGE_KIND::WINDUP;
	BOSS_PATTERN_HIT_SHAPE eHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
	std::uint32_t iDurationMs = 0;
	float fHitOuterRadius = 0.f;
	float fHitInnerRadius = 0.f;
	float fHitAngleDegrees = 0.f;
	float fHitLength = 0.f;
	float fHitHalfWidth = 0.f;
	std::uint32_t iHitCount = 0;
	std::uint32_t iHitIntervalMs = 0;
};
```

`BOSS_PATTERN_DEFINITION`은 stage별 수치를 직접 복제하지 않고
`std::vector<BOSS_PATTERN_STAGE_DEFINITION> Stages`를 소유한다.

## G02. Server runtime H 계약

### `Server/Public/ServerWorldEntity.h`

추가·교체 상태는 다음과 같다.

```cpp
std::uint32_t iPatternSequence = 0;
std::uint32_t iPatternStageIndex = 0;
float fActionElapsedSeconds = 0.f;
BOSS_PATTERN_HIT_SHAPE ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
float fPatternHitOuterRadius = 0.f;
float fPatternHitInnerRadius = 0.f;
float fPatternHitAngleDegrees = 0.f;
float fPatternHitLength = 0.f;
float fPatternHitHalfWidth = 0.f;
std::uint32_t iPatternHitCount = 0;
std::uint32_t iPatternHitIntervalMs = 0;
std::uint32_t iAppliedPatternHitCount = 0;
```

`iPatternSequence`는 새 pattern을 시작할 때 증가한다. stage 전환은 sequence를 바꾸지 않고
`iPatternStageIndex`와 `iActionStartTick`만 바꾼다.

### `Server/Private/ValtanBrain.cpp`

변경 함수 책임은 다음과 같다.

```text
SelectNormalPattern
  현재 줄·거리·repeat를 통과한 후보에서 sequence가 섞인 결정적 ticket으로 하나를 고른다.

BeginPattern
  pattern identity와 sequence를 commit하고 stage 0을 시작한다.

EnterPatternStage
  stage kind를 network action으로 번역하고 hit 수치와 action start tick을 원자적으로 교체한다.

ContainsPatternHit
  boss-local XZ 좌표로 CIRCLE/RING/CONE/BOX/CROSS 포함 여부를 계산한다.

ApplyPatternHit
  combat-ready인 살아 있는 player만 검사하고 Server damage event를 만든다.

AdvancePatternStage
  다음 stage가 있으면 전환하고 끝이면 IDLE로 돌려 다음 tick selection을 허용한다.
```

## G03. Shared protocol 계약

### `Shared/Public/Network/PacketMessages.h`

```cpp
struct C2S_REVIVE_PLAYER
{
	std::uint32_t iClientSequence = 0;
};

struct PLAYER_SNAPSHOT
{
	// existing fields
	bool isCombatReady = true;
};

struct WORLD_ENTITY_SNAPSHOT
{
	// existing fields
	std::string strPatternId;
	std::uint32_t iPatternSequence = 0;
	std::uint32_t iPatternStageIndex = 0;
};
```

writer와 reader는 bool을 U8 `0/1`로 검증한다. pattern ID는 기존 stable network ID 제한을 사용한다.

## G04. 입장 보호와 Revive 코드 계약

### `Server/Public/ServerPlayer.h`

```cpp
bool isCombatReady = true;
std::uint32_t iLastReviveSequence = 0;
```

### `Server/Private/GameRoom.cpp`

```text
Join
  Valtan Arena이면 isCombatReady=false, 다른 world는 true로 생성한다.

Handle_Move
  payload와 sequence가 유효하고 player가 살아 있으면 보호를 해제한 뒤 이동을 stage한다.

Handle_UseSkill
  Try_Start가 성공한 경우 보호를 해제한다.

Handle_RevivePlayer
  authenticated session -> dead player -> newer sequence -> Valtan Arena 순으로 검증한다.
  위치와 yaw는 유지하고 HP/resource/stance/action/path/cooldown을 복원한다.
  isCombatReady=false로 두어 로딩과 동일한 안전 경계를 재사용한다.
```

## G05. Client 명령과 Balance Tool 계약

### `Client/Public/PlayerCommandSink.h`

```cpp
virtual bool Request_RevivePlayer(std::uint32_t clientSequence) = 0;
```

### `Client/Public/BalanceTool.h`

Balance Tool은 socket을 소유하지 않고 주입된 `shared_ptr<IPlayerCommandSink>`만 보존한다.

```cpp
explicit CBalanceTool(std::shared_ptr<IPlayerCommandSink> commandSink);

std::shared_ptr<IPlayerCommandSink> m_commandSink;
std::uint32_t m_reviveSequence = 0;
```

`RenderLiveVerification`은 non-const로 바꾸고 player HP가 0일 때만 Revive 버튼을 활성화한다.
요청 성공은 다음 snapshot을 기다리며 Client HP를 직접 수정하지 않는다.

## G06. 실제 반영 정본

계획에서 정의한 public 구조와 함수는 다음 파일에 최종 반영됐다.

| 계약 | 실제 정본 |
|---|---|
| pattern/stage schema | `Data/Encounters/Valtan/ValtanEncounter.json` formatVersion 3 |
| damage rate | `Data/Balance/DamageProfiles.json` |
| publish row | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`의 `PATTERN`, `PATTERNSTAGE` |
| staged catalog | `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` |
| stage 실행/collider | `Server/Public/ServerWorldEntity.h`, `Server/Private/ValtanBrain.cpp` |
| combat-ready/revive | `Server/Public/ServerPlayer.h`, `Server/Private/GameRoom.cpp` |
| protocol | `Shared/Public/Network/PacketType.h`, `PacketMessages.h/.cpp` |
| typed Client command | `Client/Public/PlayerCommandSink.h`, `NetworkPlayerCommandSink.h/.cpp`, `NetworkManager.h/.cpp` |
| tuning UI | `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` |
| live snapshot | `Client/Public/CombatHUDViewModel.h`, `Client/Private/CombatHUDViewModel.cpp` |

최종 Server stage 진입 함수의 데이터 복사 경계는 다음과 같다.

```cpp
void EnterPatternStage(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_DEFINITION& stage,
	std::uint32_t stageIndex,
	std::uint32_t serverTick);
```

이 함수만 stage의 action, duration, shape, geometry, hit 수치, damage profile,
`iPatternStageIndex`, `iActionStartTick`을 현재 runtime entity에 commit한다. stage 종료는
`iPatternStageDurationMs`와 Server fixed tick elapsed만으로 결정하며 Client animation 종료 신호를 받지 않는다.

Revive 최종 실행 경계는 다음과 같다.

```cpp
void CGameRoom::Handle_RevivePlayer(
	SESSION_ID sessionId,
	const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
```

이 함수는 Valtan room, authenticated session, newer sequence, dead state를 확인하고 위치/yaw는
그대로 둔 채 HP/resource/stance/action/path/cooldown을 복원한다. 마지막에 `isCombatReady=false`를
적용하므로 Revive 직후에도 첫 유효 이동/스킬 전에는 발탄이 플레이어를 target/hit하지 않는다.

## G07. 정본 검증 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Publish
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
Server/Bin/Debug/Server.exe --contract-test
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```
