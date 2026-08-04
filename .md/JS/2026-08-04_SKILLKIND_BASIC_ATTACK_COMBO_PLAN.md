# skillKind 도입과 창술사 평타 콤보

작성자: JS · 2026-08-04 · 브랜치 `feature/skill-kind-basic-attack`

평타를 현재 스킬 스키마에 그대로 넣을 수 없다. `AGENTS.md`가 이유를 이미 적어뒀다.

> 평타·이동기·스탠스 전환은 쿨다운/히트/데미지가 없어 현재 스키마에 들어가지 않으며
> `skillKind` 도입 후에 추가한다.

이 문서는 `skillKind`를 도입하고 첫 소비자로 창술사 평타(34010) 4단 콤보를 닫는다.

## 0. 실측 — 설계가 여기서 결정됐다

### 0.1 단계 타이밍은 원작에서 전부 나온다

`Data/Animation/Reference/LanceMaster/LanceMaster.animnotify`의 클립 로컬 시각이다.

```text
단계  클립                      길이(ms)  히트(ms)  선입력창(ms)
 1    flm_att_identity1_1_01      1633      470     329 ~ 658
 2    flm_att_identity1_1_02      1367      356     330 ~ 554
 3    flm_att_identity1_1_03      1533      451     396 ~ 752
 4    flm_att_identity1_1_04      1567      500     없음
```

4단에 `[선콤]`이 없다는 것이 마지막 단계임을 데이터로 확정한다. 추측한 값이 없다.

### 0.2 `.skilltiming`에 34010이 없다

DB에 평타 쿨다운·데미지 레코드가 없다. **데미지는 우리가 정한다.** 이번 값 100은 자리값이다.

### 0.3 런타임 쿨다운은 이미 0을 견딘다

```text
MillisecondsToTicks(0) = (0 * 30 + 999) / 1000 = 0
cooldownEndTick = actionStartTick
검사 cooldown->second > actionStartTick  ->  같음  ->  통과
```

막는 것은 publisher의 `cooldownMs -eq 0` throw 하나뿐이다. Server 쿨다운 코드는 안 건드린다.

### 0.4 액션 중 입력 거부는 남겨야 한다

```cpp
// Server/Private/PlayerSkillSystem.cpp:36
PLAYER_ACTION_STATE::NONE != player.eAction
```

지우면 모든 스킬이 서로를 끊고, 다음 계약 테스트가 즉시 깨진다.

```cpp
tests.Require(!skills.Try_Start(player, useSkill, catalog, 10),
    "Reject duplicate skill command while action is active");
```

전역으로 풀지 않고 세 조건이 **전부** 참일 때만 예외를 연다.

```text
1. 들어온 스킬의 eSkillKind == COMBO
2. 진행 중인 액션이 같은 skillId
3. 경과 시각이 현재 단계의 선입력 창 안
```

### 0.5 `CLIP_CHAIN::sMode`가 이미 있고 소비되기를 기다린다

```cpp
struct CLIP_CHAIN
{
	int32_t iSkillId = {};
	int32_t iSeqIndex = {};
	/* COMBO advances a step per press, HOLD is a charge, ONESHOT/SEQUENCE
	run to the end. Only carried for now -- every chain runs to the end. */
	std::string sMode;
	std::vector<std::string> clips;
};
```

창술사 체인 110개가 이미 `SEQUENCE 82 / COMBO 14 / HOLD 6 / ONESHOT 8`로 나뉘어 파싱된다.
콤보 전용 우회 경로를 새로 만들지 않고 이 필드를 소비한다.

문제는 `Update_Chain`이 입력과 무관하게 다음 클립을 건다는 것이다.

```cpp
void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;
	++m_iChainStep;
	...
	Start_Clip(m_pChain->clips[m_iChainStep].c_str());
}
```

`sMode == "COMBO"`면 여기서 멈추고, 서버가 내려준 단계가 올릴 때만 진행한다.

**`sMode`는 자동 진행 여부만 정한다. 단계 번호의 정본은 서버다.** `.clipseq`의 mode는
클립 이름 패턴으로 추론한 값이라(`2026-07-31_..._SKILLTIMING_V2_EXTRACT_RESULT.md` 10.2절)
틀릴 수 있고, 실제로 각성기가 COMBO로 잘못 잡혀 손으로 고친 전례가 있다.

### 0.6 콤보 단계는 스냅샷에 실어야 한다

`SNAPSHOT_PLAYER`는 `iSkillId`와 `iActionStartTick`만 보낸다. 클라가 단계를 자체 카운트하면
패킷 유실·재접속에서 서버와 어긋나 **잘못된 클립을 재생한다.**

### 0.7 꾹 누르기는 주기 전송으로만 가능하다

버퍼는 창 안에서 세워져야 하는데 클라는 창을 모른다(알면 서버 권위 침범). 누른 순간의 한 번은
창이 열리기 전이라 버려진다.

```text
t=0     누름 -> Try_Start -> 1단 시작
t=329   창 열림   <- 여기서 다시 보내야 버퍼가 선다
t=658   창 닫힘
t=1633  1단 종료 -> 버퍼 없으면 콤보 끝
```

가장 좁은 창이 2단의 224ms이므로 **100ms 주기**면 어떤 창에도 최소 2번 들어간다. 창을
계산하는 것이 아니라 전송률 제한이다. 연타와 꾹 누르기가 같은 경로가 된다.

## 1. 완료 조건

1. 좌클릭으로 34010이 제출되고 서버 승인 후 1단이 재생된다.
2. 연타하거나 누르고 있으면 2 → 3 → 4단으로 이어진다.
3. 창 밖 입력은 무시된다. 4단 종료 또는 버퍼 없음이면 콤보가 끝난다.
4. 콤보 중 다른 스킬(Q~ALT_V)은 기존대로 거부된다.
5. 기존 9스킬의 승인·쿨다운·데미지·체인 재생이 변하지 않는다.
6. 우클릭 이동이 좌클릭 평타와 섞이지 않는다.

## 2. 데이터

### 2.1 `Data/Balance/DamageProfiles.json`

배열 끝에 추가한다.

```json
    {
      "damageProfileId": "damage.player.34010",
      "amount": 100
    }
```

### 2.2 `Data/Balance/PlayerSkills.json`

`Assert-ExactProperties`가 속성 집합을 정확히 비교하므로 **기존 15행 전부**에
`skillKind`와 `comboStages`를 추가한다. 기존 행은 `"ACTIVE"`와 `[]`다.

```json
    {
      "skillId": 34120,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "Q",
      "displayName": "연환섬",
      "actionId": "lancemaster.skill.34120",
      "skillKind": "ACTIVE",
      "cooldownMs": 10000,
      "actionDurationMs": 2266,
      "hitTimeMs": 1295,
      "resourceCost": 15,
      "movementDistance": 0.0,
      "maximumRange": 7.3,
      "serverDamageProfileId": "damage.player.34120",
      "effectId": "",
      "comboStages": []
    },
```

평타 행을 배열 끝에 추가한다. 값은 0.1절 실측이다.

```json
    {
      "skillId": 34010,
      "characterClass": "LANCE_MASTER",
      "inputSlot": "LMB",
      "displayName": "긴 창_평타",
      "actionId": "lancemaster.skill.34010",
      "skillKind": "COMBO",
      "cooldownMs": 0,
      "actionDurationMs": 1633,
      "hitTimeMs": 470,
      "resourceCost": 0,
      "movementDistance": 0.0,
      "maximumRange": 3.5,
      "serverDamageProfileId": "damage.player.34010",
      "effectId": "",
      "comboStages": [
        { "actionDurationMs": 1633, "hitTimeMs": 470, "inputOpenMs": 329, "inputCloseMs": 658 },
        { "actionDurationMs": 1367, "hitTimeMs": 356, "inputOpenMs": 330, "inputCloseMs": 554 },
        { "actionDurationMs": 1533, "hitTimeMs": 451, "inputOpenMs": 396, "inputCloseMs": 752 },
        { "actionDurationMs": 1567, "hitTimeMs": 500, "inputOpenMs": 0,   "inputCloseMs": 0 }
      ]
    }
```

스킬 행의 `actionDurationMs`/`hitTimeMs`는 1단 값과 같게 둔다. `LMB`는 publisher의
`$playerSkillSlots`에 이미 있으므로 슬롯 목록은 안 바꾼다.

## 3. publisher

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`

### 3.1 속성 목록

```powershell
    Assert-ExactProperties $skill @(
		'skillId','characterClass','inputSlot','displayName','actionId','skillKind','cooldownMs','actionDurationMs',
        'hitTimeMs','resourceCost','movementDistance','maximumRange','serverDamageProfileId','effectId','comboStages') 'player skill'
```

### 3.2 kind별 검증

기존 조건식에서 `[uint32]$skill.cooldownMs -eq 0` 항을 제거하고 아래를 `Assert-StableId`
호출들 뒤에 넣는다.

```powershell
	$skillKind = [string]$skill.skillKind
	if ($skillKind -notin @('ACTIVE','COMBO')) {
		throw "Unknown skillKind: $($skill.skillId) $skillKind"
	}
	$stages = @($skill.comboStages)
	if ($skillKind -eq 'ACTIVE') {
		if ($stages.Count -ne 0) {
			throw "ACTIVE skill must not carry comboStages: $($skill.skillId)"
		}
		if ([uint32]$skill.cooldownMs -eq 0) {
			throw "ACTIVE skill needs a cooldown: $($skill.skillId)"
		}
	}
	else {
		if ($stages.Count -lt 2 -or $stages.Count -gt 8) {
			throw "COMBO skill needs 2..8 stages: $($skill.skillId)"
		}
		for ($i = 0; $i -lt $stages.Count; $i++) {
			$stage = $stages[$i]
			Assert-ExactProperties $stage @(
				'actionDurationMs','hitTimeMs','inputOpenMs','inputCloseMs') 'combo stage'
			if ([uint32]$stage.actionDurationMs -eq 0 -or
				[uint32]$stage.hitTimeMs -gt [uint32]$stage.actionDurationMs) {
				throw "Combo stage timing is invalid: $($skill.skillId) stage $i"
			}
			if ($i -eq $stages.Count - 1) {
				if ([uint32]$stage.inputOpenMs -ne 0 -or [uint32]$stage.inputCloseMs -ne 0) {
					throw "Final combo stage must not open an input window: $($skill.skillId)"
				}
			}
			elseif ([uint32]$stage.inputOpenMs -ge [uint32]$stage.inputCloseMs -or
				[uint32]$stage.inputCloseMs -gt [uint32]$stage.actionDurationMs) {
				throw "Combo input window is invalid: $($skill.skillId) stage $i"
			}
		}
	}
```

### 3.3 bootstrap 출력

SKILL 행에 kind를 붙여 13필드로 만들고 단계는 별도 행으로 낸다. `effectId`는 계속 싣지 않는다.

```powershell
    $skillRows.Add((@(
        'SKILL', $id, $skill.characterClass, $skill.inputSlot, $skill.actionId,
        [uint32]$skill.cooldownMs, [uint32]$skill.actionDurationMs, [uint32]$skill.hitTimeMs,
        [uint32]$skill.resourceCost,
        (Format-InvariantFloat $skill.movementDistance "skill $id movementDistance"),
        (Format-InvariantFloat $skill.maximumRange "skill $id maximumRange"),
        $skill.serverDamageProfileId,
        $skillKind) -join "`t"))
    for ($i = 0; $i -lt $stages.Count; $i++) {
        $stage = $stages[$i]
        $skillRows.Add((@(
            'SKILLSTAGE', $id, $i,
            [uint32]$stage.actionDurationMs, [uint32]$stage.hitTimeMs,
            [uint32]$stage.inputOpenMs, [uint32]$stage.inputCloseMs) -join "`t"))
    }
```

## 4. Shared

### 4.1 `Shared/Public/Network/PacketType.h`

```cpp
	enum class PLAYER_SKILL_KIND : std::uint8_t
	{
		ACTIVE = 0,
		COMBO = 1,
		END
	};
```

### 4.2 `Shared/Public/Network/PacketMessages.h`

`SNAPSHOT_PLAYER`의 `Cooldowns` 바로 앞에 넣는다. 기존 필드 순서는 바꾸지 않는다.

```cpp
		std::uint32_t iCurrentResource = 0;
		std::uint32_t iMaximumResource = 1;
		std::uint8_t iComboStage = 0;
		std::vector<SKILL_COOLDOWN_SNAPSHOT> Cooldowns;
```

### 4.3 `Shared/Private/Network/PacketMessages.cpp`

Writer에서 `Write_U32(player.iMaximumResource);` 다음 줄:

```cpp
	writer.Write_U8(player.iComboStage);
```

Reader에서 `!reader.Read_U32(player.iMaximumResource) ||` 다음 줄:

```cpp
		!reader.Read_U8(player.iComboStage) ||
```

Validate의 player 루프에 범위 검사를 넣는다. 8은 3.2절 상한과 같다.

```cpp
		if (player.iComboStage > 8u)
			return false;
```

## 5. Server

### 5.1 `Server/Public/GameplayCatalog.h`

```cpp
	struct PLAYER_COMBO_STAGE final
	{
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		std::uint32_t iInputOpenMs = 0;
		std::uint32_t iInputCloseMs = 0;
	};
```

`PLAYER_SKILL_DEFINITION`의 `fMaximumRange` 다음:

```cpp
		LostArk::Shared::PLAYER_SKILL_KIND eSkillKind =
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
		std::vector<PLAYER_COMBO_STAGE> ComboStages;
```

### 5.2 `Server/Private/GameplayCatalog.cpp`

익명 네임스페이스에 파서를 추가한다. 모르는 값은 거부한다.

```cpp
	bool ParseSkillKind(
		const std::string& value,
		LostArk::Shared::PLAYER_SKILL_KIND& output)
	{
		using LostArk::Shared::PLAYER_SKILL_KIND;
		if ("ACTIVE" == value)
			output = PLAYER_SKILL_KIND::ACTIVE;
		else if ("COMBO" == value)
			output = PLAYER_SKILL_KIND::COMBO;
		else
			return false;
		return true;
	}
```

SKILL 분기의 필드 수를 13으로 바꾸고 마지막 조건 뒤에 kind 파싱을 잇는다.

```cpp
			if (13u != fields.size() ||
```

```cpp
				!IsStableId(fields[11]) ||
				!ParseSkillKind(fields[12], skill.eSkillKind))
```

SKILL 분기 뒤에 단계 분기를 추가한다. `m_Skills`는
`unordered_map<SKILL_ID, PLAYER_SKILL_DEFINITION>`이므로 `find`가 유효하다. 단계는 소유
스킬 뒤에 순서대로 와야 한다.

```cpp
		else if (!fields.empty() && "SKILLSTAGE" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			PLAYER_COMBO_STAGE stage{};
			if (7u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], stage.iActionDurationMs) ||
				!ParseNumber(fields[4], stage.iHitTimeMs) ||
				!ParseNumber(fields[5], stage.iInputOpenMs) ||
				!ParseNumber(fields[6], stage.iInputCloseMs))
			{
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				LostArk::Shared::PLAYER_SKILL_KIND::COMBO !=
					owner->second.eSkillKind ||
				stageIndex != owner->second.ComboStages.size())
			{
				return false;
			}
			owner->second.ComboStages.push_back(stage);
		}
```

### 5.3 `Server/Public/ServerPlayer.h`

```cpp
		// 1-based while a combo action runs, 0 otherwise.
		std::uint8_t iComboStage = 0;
		bool hasBufferedComboInput = false;
```

### 5.4 `Server/Private/PlayerSkillSystem.cpp` — `Try_Start`

익명 네임스페이스에 창 판정을 추가한다.

```cpp
	bool IsInsideComboWindow(
		const LostArk::Server::PLAYER_SKILL_DEFINITION& skill,
		const LostArk::Server::SERVER_PLAYER& player)
	{
		if (0u == player.iComboStage ||
			player.iComboStage > skill.ComboStages.size())
		{
			return false;
		}
		const auto& stage = skill.ComboStages[player.iComboStage - 1u];
		if (0u == stage.iInputCloseMs)
			return false;
		const float elapsedMs = player.fActionElapsedSeconds * 1000.f;
		return elapsedMs >= static_cast<float>(stage.iInputOpenMs) &&
			elapsedMs <= static_cast<float>(stage.iInputCloseMs);
	}
```

기존 첫 조건식에서 `PLAYER_ACTION_STATE::NONE != player.eAction`을 뺀다.

```cpp
	const PLAYER_SKILL_DEFINITION* skill = catalog.Find_Skill(command.iSkillId);
	if (!IsNewerSequence(command.iClientSequence, player.iLastSkillSequence) ||
		nullptr == skill || skill->eCharacterClass != player.eCharacterClass ||
		0u == player.iCurrentHp ||
		!std::isfinite(command.fAimX) || !std::isfinite(command.fAimZ))
	{
		return false;
	}
```

이어서 콤보 분기를 넣는다. **이미 버퍼가 차 있으면 무시한다** — 같은 창 안의 두 번째
입력이 버퍼를 덮어써 단계를 건너뛰는 것을 막는다.

```cpp
	const bool isComboContinuation =
		LostArk::Shared::PLAYER_SKILL_KIND::COMBO == skill->eSkillKind &&
		PLAYER_ACTION_STATE::SKILL == player.eAction &&
		player.iCurrentSkillId == command.iSkillId &&
		IsInsideComboWindow(*skill, player);

	if (PLAYER_ACTION_STATE::NONE != player.eAction)
	{
		if (!isComboContinuation)
			return false;
		if (!player.hasBufferedComboInput)
		{
			player.iLastSkillSequence = command.iClientSequence;
			player.hasBufferedComboInput = true;
		}
		return false;
	}
```

**버퍼링은 `false`를 반환한다.** `GameRoom::Handle_UseSkill`이 반환값을 쓰지 않으므로 서버
동작에는 차이가 없고, 클라의 `Request_UseSkill`이 true일 때만 시퀀스를 올리므로 버퍼링을
승인으로 착각해 시퀀스가 어긋나는 것을 막는다.

승인 경로 끝(`player.fYawDegrees = ...` 다음)에 단계 초기화를 넣는다.

```cpp
	player.iComboStage =
		LostArk::Shared::PLAYER_SKILL_KIND::COMBO == skill->eSkillKind ? 1u : 0u;
	player.hasBufferedComboInput = false;
```

### 5.5 `Update` — 단계별 길이·히트와 진행

`const PLAYER_SKILL_DEFINITION* skill = ...` 다음에 현재 단계 값을 고른다.

```cpp
	const std::size_t stageIndex =
		0u == player.iComboStage ? 0u : player.iComboStage - 1u;
	const bool hasStage =
		LostArk::Shared::PLAYER_SKILL_KIND::COMBO == skill->eSkillKind &&
		stageIndex < skill->ComboStages.size();
	const std::uint32_t durationMs = hasStage ?
		skill->ComboStages[stageIndex].iActionDurationMs :
		skill->iActionDurationMs;
	const std::uint32_t hitMs = hasStage ?
		skill->ComboStages[stageIndex].iHitTimeMs :
		skill->iHitTimeMs;
```

기존 두 줄을 이 값으로 바꾼다.

```cpp
	const float durationSeconds =
		static_cast<float>(durationMs) * MILLISECONDS_TO_SECONDS;
```

```cpp
	const float hitSeconds =
		static_cast<float>(hitMs) * MILLISECONDS_TO_SECONDS;
```

종료 블록을 교체한다. 원본은 다음과 같다.

```cpp
	if (player.fActionElapsedSeconds >= durationSeconds)
	{
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0;
		player.fActionElapsedSeconds = 0.f;
		player.hasAppliedSkillDamage = false;
	}
```

교체본:

```cpp
	if (player.fActionElapsedSeconds >= durationSeconds)
	{
		const bool hasNextStage = hasStage &&
			player.hasBufferedComboInput &&
			static_cast<std::size_t>(player.iComboStage) <
				skill->ComboStages.size();
		if (hasNextStage)
		{
			++player.iComboStage;
			player.hasBufferedComboInput = false;
			player.fActionElapsedSeconds = 0.f;
			player.hasAppliedSkillDamage = false;
			player.iActionStartTick = 0u == serverTick ? 1u : serverTick;
		}
		else
		{
			player.eAction = PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.iActionStartTick = 0;
			player.fActionElapsedSeconds = 0.f;
			player.hasAppliedSkillDamage = false;
			player.iComboStage = 0;
			player.hasBufferedComboInput = false;
		}
	}
```

`iActionStartTick`을 갱신하므로 클라의 액션 시작 에지 판정이 단계마다 걸린다.

### 5.6 스냅샷 채우기

`GameRoom`이 `SNAPSHOT_PLAYER`를 만드는 자리에 한 줄 추가한다.

```cpp
	snapshotPlayer.iComboStage = player.iComboStage;
```

## 6. Client

### 6.1 `Client/Public/PlayerSkillCatalog.h`

```cpp
		std::uint32_t iCooldownMs = 0;
		std::uint32_t iDamage = 0;
		LostArk::Shared::PLAYER_SKILL_KIND eSkillKind =
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
```

`PlayerSkillCatalog.cpp`의 파서가 `skillKind` 문자열을 읽고, 모르는 값이면 그 행을 버린다.
단계 배열은 **읽지 않는다** — 클라는 서버가 준 단계 번호만 쓰고 타이밍을 판정하지 않는다.

### 6.2 `Client/Public/PlayerController.h`

```cpp
	bool_t m_wasLeftMouseDown = false;
	// Held fire re-sends on an interval because the combo buffer must land
	// inside a window the client is not allowed to know.
	f32_t m_fBasicAttackResendTimer = 0.f;
```

### 6.3 `Client/Private/PlayerController.cpp`

`Poll_SkillSlots`의 키보드 루프가 끝난 뒤, `outSkillId`가 아직 비었을 때만 좌클릭을 본다.
제출 블록은 기존 것 하나를 그대로 쓴다.

```cpp
	if (LostArk::Shared::INVALID_SKILL_ID != outSkillId || nullptr == pSpec)
		return;

	const bool_t isLeftMouseDown =
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::LB) & 0x80);
	if (!isLeftMouseDown)
	{
		m_wasLeftMouseDown = false;
		m_fBasicAttackResendTimer = 0.f;
		return;
	}

	const bool_t isFirstPress = !m_wasLeftMouseDown;
	m_wasLeftMouseDown = true;
	if (!isFirstPress && m_fBasicAttackResendTimer > 0.f)
		return;
	m_fBasicAttackResendTimer = BASIC_ATTACK_RESEND_SECONDS;

	const PLAYER_SKILL_DEFINITION* basic =
		CPlayerSkillCatalog::Find_BySlot(pSpec->eCharacterClass, "LMB");
	if (nullptr != basic)
		outSkillId = basic->iSkillId;
```

`CHARACTER_SPEC::eCharacterClass`가 실제 필드명이며, 헤더 주석이 이 용도를 명시한다 —
"Input binding resolves quick slots through this."

타이머는 `Update`에서 줄인다. `Poll_SkillSlots`는 `fTimeDelta`를 받지 않으므로 호출 전에
감산한다.

```cpp
	if (m_fBasicAttackResendTimer > 0.f)
		m_fBasicAttackResendTimer -= fTimeDelta;
```

상수는 파일 상단 익명 네임스페이스에 둔다.

```cpp
	constexpr f32_t BASIC_ATTACK_RESEND_SECONDS = 0.1f;
```

### 6.4 `Client/Private/Character.cpp` — 자동 진행 차단

```cpp
void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;
	if ("COMBO" == m_pChain->sMode)
		return;
	++m_iChainStep;
	...
}
```

### 6.5 단계 소비

`Apply_NetworkAction`에 단계를 넘긴다. 현재 시그니처는 다음과 같다.

```cpp
bool_t CCharacter::Apply_NetworkAction(
	const LostArk::Shared::PLAYER_ACTION_STATE action,
	const LostArk::Shared::SKILL_ID skillId,
	const std::uint32_t actionStartTick)
```

네 번째 인자를 추가한다.

```cpp
bool_t CCharacter::Apply_NetworkAction(
	const LostArk::Shared::PLAYER_ACTION_STATE action,
	const LostArk::Shared::SKILL_ID skillId,
	const std::uint32_t actionStartTick,
	const std::uint8_t comboStage)
```

SKILL 분기의 `Play_Skill` 호출을 단계에 따라 나눈다.

```cpp
		if (m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick)
		{
			return true;
		}
		if (0u == comboStage)
		{
			if (!Play_Skill(static_cast<int32_t>(skillId)))
				return false;
		}
		else if (1u == comboStage)
		{
			if (!Play_Skill(static_cast<int32_t>(skillId)))
				return false;
		}
		else
		{
			if (!Advance_ComboStage(comboStage))
				return false;
		}
		m_iLastNetworkActionStartTick = actionStartTick;
```

`Advance_ComboStage`는 현재 체인의 n번째 클립으로 이동한다. 체인이 없거나 범위를 넘으면
실패를 반환해 호출부가 기존 상태를 유지한다.

```cpp
bool_t CCharacter::Advance_ComboStage(const std::uint8_t comboStage)
{
	if (nullptr == m_pChain || 0u == comboStage)
		return false;
	const int32_t step = static_cast<int32_t>(comboStage) - 1;
	if (step >= static_cast<int32_t>(m_pChain->clips.size()))
		return false;
	m_iChainStep = step;
	return Start_Clip(m_pChain->clips[step].c_str());
}
```

`Start_Clip`은 `bool_t`를 반환하며 내부에서 `Set_Animation` 실패를 그대로 전파하고
성공 시 트랙을 0으로 되감는다. 그래서 위 반환이 그대로 성립한다.

`CClientReplication`이 `Apply_NetworkAction`을 부르는 자리에 `player.iComboStage`를 넘긴다.

### 6.6 HUD

`CCombatHUDViewModel`은 `LMB` 슬롯을 쿨다운 표시에서 제외한다. 쿨다운이 0이라 표시할 것이
없다.

## 7. 하네스

### 7.1 `Tools/NetworkProtocolHarness`

```text
iComboStage 왕복 (0, 3)
iComboStage = 9 인 스냅샷 거부
```

### 7.2 `Server/Private/ServerGameplayContractTests.cpp`

기존 "Reject duplicate skill command while action is active"는 **그대로 통과해야 한다.**

```text
콤보 1단 승인 -> iComboStage == 1
창 이전(예: 100ms) 입력 -> 버퍼 안 섬
창 안(예: 400ms) 입력 -> 버퍼 섬
같은 창 두 번째 입력 -> 버퍼 그대로 (단계 건너뛰기 없음)
단계 종료 -> iComboStage == 2, iActionStartTick 갱신
콤보 중 34120 -> 거부
버퍼 없이 단계 종료 -> eAction NONE, iComboStage 0
4단 종료 -> 콤보 종료
```

## 8. 검증 순서

```text
Publish-GameplayBalance Validate + 실패 픽스처
  (kind 미상 / ACTIVE에 stages / 창 역전 / 마지막 단계에 창)
Engine -> UpdateLib -> Shared -> NetworkProtocolHarness 실행
Server 빌드 + --contract-test
Client 빌드
Server+Client 실행:
  좌클릭 연타 -> 4단, 꾹 누르기 -> 4단
  창 밖 입력 무시, 콤보 중 Q 거부
  기존 9스킬 동작 불변
ProjectAudit
```

## 9. 팀 문서 갱신

- `AGENTS.md`의 "평타·이동기·스탠스 전환은 ... `skillKind` 도입 후에 추가한다"에서 평타를
  뺀다. 이동기·스탠스는 남는다.
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` 3절 입력 표에 좌클릭 행을 추가하고,
  6절 밸런스 표의 `PlayerSkills.json` 설명에 `skillKind`/`comboStages`를 넣는다.

## 9.1 선입력 취소 — 첫 실행 확인 뒤 추가했다

처음 구현은 단계 클립이 `actionDurationMs`를 다 채워야 넘어갔다(1단 1633ms). 실행해 보니
원작보다 확연히 느렸다.

데이터가 답을 준다. 세 단계 모두 **히트가 자기 입력 창 안에 있다.**

```text
단계  히트   창           클립 길이
 1    470    329 ~ 658    1633
 2    356    330 ~ 554    1367
 3    451    396 ~ 752    1533
```

그래서 규칙을 하나 더 두었다.

> 버퍼가 찼고 히트가 지나갔으면 남은 클립을 끊고 다음 단계로 넘어간다.

전환 시점이 클립 길이에서 히트 시각으로 바뀐다(1633 → 470, 약 3.5배). 히트 이후에만
끊으므로 **데미지는 하나도 잃지 않는다.** 새 데이터 필드도 필요 없다.

버퍼가 없으면 기존대로 클립을 끝까지 재생한다. 연타를 멈추면 마지막 동작이 잘리지 않는다.
서버 `Update`의 종료 판정 한 곳만 바뀌었고 Data/Shared/Client 계약은 그대로다.

여전히 느리다면 다음 후보는 창 열림(329ms) 기준인데, 1단 히트(470ms)를 지나기 전에 끊겨
데미지가 사라진다. 그 경우 히트 시각 재조정이나 마지막 단계 집중 데미지 중 하나를 먼저
정해야 한다.

## 10. 이번 범위 밖

- 이동기와 스탠스 전환. `skillKind`는 열지만 두 kind는 별도 슬라이스다.
- HOLD 모드. `Update_Chain`의 같은 자리에서 나중에 확장한다.
- 루트 모션. 서버가 `movementDistance`로 위치를 확정하는 현재 계약과 충돌하며 예측/확정
  분리 설계가 선행이다(`2026-08-03_LOSTARK_ROOT_MOTION_ANIM_LOAD_NOTES.md` 6절).
- 짧은 창 평타(34510). 스탠스 전환이 없어 진입 경로가 없다.
- 평타 데미지 튜닝. 100은 자리값이다.
