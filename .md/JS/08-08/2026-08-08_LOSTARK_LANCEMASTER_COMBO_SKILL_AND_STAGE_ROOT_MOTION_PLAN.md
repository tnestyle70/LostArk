# 공의연무·선풍참혼 COMBO 전환과 스테이지별 루트모션

작성자: JS · 2026-08-08 · 브랜치 `feature/lancemaster-skill-timing`에서 분기 예정

`2026-08-05_LOSTARK_LANCEMASTER_CLIP_ADVANCE_TIMING_PLAN.md` →
`2026-08-06_LOSTARK_PLAYER_ANIM_FRAMERATE_AND_CLIP_TIMING_RESULT.md`로 이어진 클립 전환
작업의 다음 단계다. 오늘 E 청룡출수(34100)를 캔슬 윈도우 기준으로 닫았고(커밋 `64a8c63`),
같은 요청의 R·A는 성격이 달라 이 문서로 분리한다.

## 0. 사용자가 확정한 계약

```text
COMBO = 다음 애니메이션 클립 재생에 재입력이 필요한 스킬
        한 번 눌러 끝까지 재생되면 COMBO가 아니라 일반 스킬이다
루트모션 = COMBO 여부와 무관하게 애니메이션에 이동량이 있으면 적용된다
```

두 번째 항목이 이번 작업의 범위를 넓힌다. `buildScript/extract_rootmotion.py:19-20`이
COMBO와 HOLD를 통째로 건너뛰기 때문에 **현재 여섯 클래스의 모든 COMBO 스킬에 루트모션이
없다.** 평타 34010/34510도 포함된다. R·A를 COMBO로 바꾸면 두 스킬의 이동(전진 6.53, 1.97)이
사라지므로, 스테이지별 루트모션을 먼저 닫지 않으면 이번 작업은 기능 후퇴가 된다.

목표 동작:

```text
R 공의연무 34160  1번 입력 -> riseup_01        재입력 -> riseup_02        종료
A 선풍참혼 34140  1번 입력 -> chestdestruction_01,02  재입력 -> chestdestruction_03,04
```

R은 스테이지당 클립 1개라 현재 구조에 그대로 맞는다. A만 스테이지당 클립 2개가 필요해
skillbindings 스키마 확장이 필요하다.

## 1. 실측 — 현재 COMBO 계약은 어디까지 완성돼 있나

### 1.1 Server는 이미 재입력을 판정한다

`Server/Private/PlayerSkillSystem.cpp:101-118` — 액션 중이라도 같은 skillId를 스테이지의
입력 창 안에서 누르면 버퍼에 표시한다. 승인이 아니라 버퍼이므로 `false`를 돌려주고, 한
윈도우가 두 스테이지를 진행시키지 못하게 `hasBufferedComboInput`으로 막는다.

`PlayerSkillSystem.cpp:62-77` `IsInsideComboWindow` — `[iInputOpenMs, iInputCloseMs]`
구간 판정. `iInputCloseMs == 0`이면 창이 닫힌 것으로 본다(마지막 스테이지).

`PlayerSkillSystem.cpp:362-401` 스테이지 진행:

```text
hasNextStage        = 버퍼된 입력 있고 다음 스테이지가 남았다
cancelsIntoNextStage = hasNextStage && hasAppliedSkillDamage
전이 조건           = cancelsIntoNextStage || 스테이지 duration 경과
전이 시            = iComboStage 증가, fActionElapsedSeconds = 0,
                     hasAppliedSkillDamage = false, iActionStartTick = serverTick
```

`fActionElapsedSeconds`가 스테이지마다 0으로 리셋된다(382행). **스테이지별 루트모션 커브를
스테이지 로컬 시간으로 그대로 샘플링할 수 있다는 뜻이며, 이번 설계의 전제다.**

`cancelsIntoNextStage`가 사용자가 말한 "캔슬"이다. 히트가 끝난 뒤 남은 클립을 잘라내고 다음
스테이지로 넘어간다. A에 별도 캔슬 메커니즘을 만들 필요가 없다.

### 1.2 Client는 스테이지 하나에 클립 하나만 안다

`Client/Private/Character.cpp:482-491`:

```cpp
bool_t CCharacter::Advance_ComboStage(const std::uint8_t comboStage)
{
	if (nullptr == m_pChain || 0u == comboStage)
		return false;
	const int32_t step = static_cast<int32_t>(comboStage) - 1;
	if (step >= static_cast<int32_t>(m_pChain->clips.size()))
		return false;
	m_iChainStep = step;
	return Start_Clip(m_pChain->clips[step]);
}
```

스테이지 N ↔ 클립 인덱스 N-1의 1:1 고정이다. `Update_Chain`(387-405행)은
`isServerStaged`면 즉시 반환해 마지막 포즈를 유지한다.

`Client/Private/AnimationSkillBindingDocument.cpp:402-410`이 이 계약을 강제한다.

```cpp
if (binding.Clips.empty() ||
	binding.Clips.size() > MAX_CLIPS_PER_BINDING ||
	(PLAYER_SKILL_KIND::COMBO == definition->eSkillKind &&
		binding.Clips.size() != definition->iComboStageCount))
```

### 1.3 루트모션은 스킬 단위 단일 커브다

`Server/Public/GameplayCatalog.h:27-49` `PLAYER_SKILL_DEFINITION::RootMotion`은
스킬당 하나다. `PlayerSkillSystem.cpp:240-253`이 `fActionElapsedSeconds`로 두 번
샘플링해 그 차이를 이번 틱 이동량으로 쓴다. `Server/Private/GameplayCatalog.cpp:381-399`의
`SKILLROOTMOTION` 행은 4필드(`태그, skillId, sampleCount, packed`)이고 스킬당 1회만
허용한다.

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1:755-791`이
`Data/Animation/RootMotion/*.rootmotion.json`을 읽어 이 행을 만든다. 샘플 `timeMs`가
`actionDurationMs`를 넘으면 거부한다(781행).

### 1.4 확정한 스테이지 수치

`.animnotify`에서 직접 뽑았다. 클립 전환 시점은 08-06 RESULT가 확정한 규칙
— **레이블이 없고 `win=NONE`이며 `t+d`가 클립 끝에 닿는 CANCEL 윈도우 중 가장 이른 것** — 을
그대로 적용한다. 입력 창은 `[선콤]`(COMBO_PRE)이 있으면 그것을, 없으면 `[선스]`(SKILL_PRE)를
쓴다.

R 공의연무 34160:

| 스테이지 | 클립 | 원본 길이 | playMs | 근거 |
|---|---|---|---|---|
| 1 | `flm_sk_riseup_01` | 2.6667 | 2300 | `t=2.3000 d=0.3666` → 2.6666 = 클립 끝 |
| 2 | `flm_sk_riseup_02` | 1.4000 | 없음 | 마지막 스테이지는 끝까지 재생 |

```text
stage 1  actionDurationMs=2300  hitTimeMs=500   inputOpenMs=1302  inputCloseMs=2300
stage 2  actionDurationMs=1400  hitTimeMs=250   inputOpenMs=0     inputCloseMs=0
skill    actionDurationMs=3700  (5667에서 감소)
```

`inputOpenMs=1302`는 `flm_sk_riseup_01`의 `[선스] t=1.3018`. `inputCloseMs`는 스테이지
duration으로 잘랐다 — `GameplayCatalog.cpp:360`이 `iInputCloseMs > iActionDurationMs`를
거부한다. `hitTimeMs`는 각 클립의 첫 `kind=HIT` 노티파이(0.5000 / 0.2500)다.

A 선풍참혼 34140:

| 스테이지 | 클립 | 원본 길이 | playMs | 근거 |
|---|---|---|---|---|
| 1 | `flm_sk_chestdestruction_01` | 0.7667 | 551 | `t=0.5507 d=0.2160` → 0.7667 = 클립 끝 |
| 1 | `flm_sk_chestdestruction_02` | 1.5667 | 없음 | 스테이지 마지막 클립 |
| 2 | `flm_sk_chestdestruction_03` | 1.3333 | 없음 | 자격 있는 윈도우 없음 |
| 2 | `flm_sk_chestdestruction_04` | 1.9333 | 없음 | 마지막 클립 |

```text
stage 1  actionDurationMs=2118  hitTimeMs=450   inputOpenMs=299   inputCloseMs=602
stage 2  actionDurationMs=3266  hitTimeMs=1633  inputOpenMs=0     inputCloseMs=0
skill    actionDurationMs=5384  (5600에서 감소)
```

`inputOpenMs/CloseMs`는 `flm_sk_chestdestruction_01`의 `[선콤] t=0.2990 d=0.3028`이다.
stage 1의 `hitTimeMs=450`이 이 창 안에 들어오므로 `cancelsIntoNextStage`가
`hasAppliedSkillDamage`를 만족한 뒤에만 전이한다는 서버 주석과 일치한다. stage 2의 1633은
`chestdestruction_03`(1333, HIT 없음) 뒤 `chestdestruction_04`의 첫 HIT `t=0.3000`이다.

## 2. G01 — skillbindings formatVersion 3: 스테이지별 클립 그룹

### 2.1 스키마

`clips` 배열의 원소로 **배열**을 허용한다. 원소가 배열이면 그 스테이지에서 순서대로 재생할
클립 묶음이다. 문자열과 객체 원소는 v2와 동일하게 클립 하나를 뜻한다.

```json
{
  "skillId": 34140,
  "clips": [
    [ { "clip": "flm_sk_chestdestruction_01", "playMs": 551 },
      "flm_sk_chestdestruction_02" ],
    [ "flm_sk_chestdestruction_03",
      "flm_sk_chestdestruction_04" ]
  ]
}
```

v2 문서는 전부 v3에서 그대로 유효하다. ACTIVE는 `clips` 전체가 한 스테이지의 체인이고,
COMBO는 `clips.length == comboStageCount`이며 각 원소가 그 스테이지의 클립 묶음이다. 여섯
클래스의 기존 평타 바인딩은 한 글자도 바꾸지 않는다.

### 2.2 `Client/Public/AnimationSkillBindingDocument.h`

```text
파일: Client/Public/AnimationSkillBindingDocument.h
작업: 교체
기준점: struct ANIMATION_SKILL_BINDING
위치: 기존 블록 전체 교체 (struct 시작부터 닫는 }; 까지)
필요한 이유: 스테이지당 여러 클립을 표현
연결되는 부분: Parse_Text, Validate, Serialize, CCharacter::Load_ClipChains
```

```cpp
	struct ANIMATION_SKILL_STAGE
	{
		std::vector<ANIMATION_SKILL_CLIP> Clips;

		bool operator==(const ANIMATION_SKILL_STAGE&) const = default;
	};

	struct ANIMATION_SKILL_BINDING
	{
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		std::vector<ANIMATION_SKILL_STAGE> Stages;
	};
```

`Clips`를 `Stages`로 바꾸면 ACTIVE 경로도 스테이지 1개를 거치게 되어 두 모델이 하나로
합쳐진다. 소비자는 `Animation_Tool.cpp`와 `Character.cpp` 두 곳뿐이라 전환 비용이 낮다.

### 2.3 `Client/Private/AnimationSkillBindingDocument.cpp`

세 지점을 고친다.

- `DOCUMENT_VERSION` 상수 `2` → `3`, `Serialize`의 `"formatVersion": 2` → `3`
- `Parse_Text`의 클립 원소 파싱에 `DATA_JSON_TYPE::ARRAY` 분기 추가. 배열 원소가 또 배열이면
  거부한다(중첩 1단계까지)
- `Validate`(402-410행) 교체

```cpp
		const std::size_t stageCount = binding.Stages.size();
		std::size_t totalClips = 0;
		for (const ANIMATION_SKILL_STAGE& stage : binding.Stages)
			totalClips += stage.Clips.size();
		const bool_t isStaged =
			PLAYER_SKILL_KIND::COMBO == definition->eSkillKind ||
			PLAYER_SKILL_KIND::HOLD == definition->eSkillKind;
		if (0u == stageCount || 0u == totalClips ||
			totalClips > MAX_CLIPS_PER_BINDING ||
			(isStaged && stageCount != definition->iComboStageCount) ||
			(!isStaged && 1u != stageCount))
		{
			outStatus =
				"COMBO clip count must match the Server-owned combo stage count.";
			return false;
		}
		for (const ANIMATION_SKILL_STAGE& stage : binding.Stages)
		{
			if (stage.Clips.empty())
			{
				outStatus = "A combo stage must bind at least one clip.";
				return false;
			}
			for (const ANIMATION_SKILL_CLIP& clip : stage.Clips)
			{
				/* 기존 411-424행의 클립 단위 검증을 그대로 옮긴다 */
			}
		}
```

ACTIVE에 `1u != stageCount`를 요구해 저작 실수로 ACTIVE에 스테이지를 나누는 것을 막는다.

### 2.4 `Client/Public/Character.h`, `Client/Private/Character.cpp`

`CLIP_CHAIN`이 스테이지를 갖는다.

```cpp
	struct CLIP_STAGE
	{
		std::vector<CLIP_STEP> clips;
	};

	struct CLIP_CHAIN
	{
		int32_t iSkillId = 0;
		bool_t isServerStaged = false;
		std::vector<CLIP_STAGE> stages;
	};
```

`m_iChainStep`(클립 인덱스) 옆에 `m_iChainStage`를 둔다. 세 함수가 바뀐다.

`Play_Skill` — `pPick->clips[0]` → `pPick->stages[0].clips[0]`, `m_iChainStage = 0`.

`Advance_ComboStage` — 스테이지 인덱스로 진입하고 그 스테이지의 첫 클립부터 재생한다.

```cpp
bool_t CCharacter::Advance_ComboStage(const std::uint8_t comboStage)
{
	if (nullptr == m_pChain || 0u == comboStage)
		return false;
	const int32_t stage = static_cast<int32_t>(comboStage) - 1;
	if (stage >= static_cast<int32_t>(m_pChain->stages.size()))
		return false;
	m_iChainStage = stage;
	m_iChainStep = 0;
	return Start_Clip(m_pChain->stages[stage].clips[0]);
}
```

`Update_Chain` — 스테이지 안에서는 스스로 전진하고, 스테이지의 마지막 클립에 도달하면
멈춰 서버의 다음 스테이지를 기다린다. **이것이 A의 "1번 입력에 클립 2개 연속"을 만드는
지점이다.**

```cpp
void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;

	const CLIP_STAGE& stage = m_pChain->stages[m_iChainStage];
	if (m_iChainStep + 1 < static_cast<int32_t>(stage.clips.size()))
	{
		++m_iChainStep;
		Start_Clip(stage.clips[m_iChainStep]);
		return;
	}

	/* A combo holds on its stage's last clip until the server confirms the next
	stage. Every other mode keeps running to the end by itself. */
	if (m_pChain->isServerStaged)
		return;

	if (m_iChainStage + 1 >= static_cast<int32_t>(m_pChain->stages.size()))
		return;

	++m_iChainStage;
	m_iChainStep = 0;
	Start_Clip(m_pChain->stages[m_iChainStage].clips[0]);
}
```

`Get_EffectPlaybackRate`(`Character.cpp:215-224`)도 `m_pChain->clips[m_iChainStep]` →
`m_pChain->stages[m_iChainStage].clips[m_iChainStep]`로 따라간다.

`Load_ClipChains`(133-156행)의 HOLD 루프 판정 `isHold && 1u == chain.clips.size() &&
3u == binding.Clips.size()`는 스테이지 기준으로 다시 쓴다 — HOLD 3스테이지의 2번째
스테이지 첫 클립이 루프다.

## 3. G02 — 스테이지별 루트모션

### 3.1 `buildScript/extract_rootmotion.py`

103-106행의 COMBO/HOLD skip을 제거하고, 스테이지가 있는 스킬은 스테이지마다 독립 커브를
만든다. 각 스테이지는 자기 시간 0에서 시작한다(서버가 `fActionElapsedSeconds`를 리셋하므로).
클립 묶음은 G01의 `clips` 원소 배열을 그대로 읽는다. `carry`는 스테이지 내부에서만 누적하고
스테이지 경계에서 0으로 되돌린다.

출력 문서는 `formatVersion` 1 → 2로 올리고 스테이지가 있는 스킬만 `stages`를 갖는다.

```json
{
  "skillId": 34140,
  "stages": [
    { "stageIndex": 0, "durationMs": 2118, "samples": [ ... ] },
    { "stageIndex": 1, "durationMs": 3266, "samples": [ ... ] }
  ]
}
```

ACTIVE 스킬은 기존 `durationMs`/`samples` 모양을 유지한다. 두 키는 상호 배타다.

### 3.2 `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`

755-791행 블록을 교체한다. `formatVersion` 1과 2를 모두 받고, `stages`가 있으면 각
스테이지 샘플을 `comboStages[stageIndex].actionDurationMs`와 대조한다. 새 bootstrap 행:

```text
SKILLSTAGEROOTMOTION <skillId> <stageIndex> <sampleCount> <packed>
```

기존 `SKILLROOTMOTION`은 ACTIVE 전용으로 남긴다. `$rows` 정렬과
`LOSTARK_GAMEPLAY_BOOTSTRAP` 버전 `3` → `4`.

### 3.3 `Server/Public/GameplayCatalog.h`

```text
파일: Server/Public/GameplayCatalog.h
작업: 추가
기준점: struct PLAYER_COMBO_STAGE의 std::uint32_t iInputCloseMs = 0;
위치: 그 선언 바로 아래, 닫는 }; 바로 위
추가할 대상: 멤버 변수
필요한 이유: 스테이지가 자기 이동 커브를 소유한다
연결되는 부분: CGameplayCatalog::Load의 SKILLSTAGEROOTMOTION 행, PlayerSkillSystem::Update
```

`PLAYER_ROOT_MOTION_SAMPLE`이 `PLAYER_COMBO_STAGE`보다 아래에 선언돼 있으므로 두 struct의
순서를 바꿔 `PLAYER_ROOT_MOTION_SAMPLE`을 먼저 둔다.

```cpp
		std::vector<PLAYER_ROOT_MOTION_SAMPLE> RootMotion;
```

### 3.4 `Server/Private/GameplayCatalog.cpp`

381행 `SKILLROOTMOTION` 분기 뒤에 `SKILLSTAGEROOTMOTION` 분기를 추가한다. 5필드이고
`stageIndex`가 소유 스킬의 `ComboStages` 범위 안이어야 하며 같은 스테이지에 두 번 오면
거부한다. 샘플 언팩은 기존 400-430행 로직을 공유 헬퍼로 뽑아 재사용한다.

### 3.5 `Server/Private/PlayerSkillSystem.cpp`

240행 `if (!skill->RootMotion.empty())`를 스테이지 우선으로 바꾼다.

```cpp
	const std::vector<PLAYER_ROOT_MOTION_SAMPLE>& rootMotion =
		hasStage ? skill->ComboStages[stageIndex].RootMotion : skill->RootMotion;
	if (!rootMotion.empty())
```

`hasStage`는 227-229행에서 이미 계산돼 있다. 아래 두 `Sample_RootMotion` 호출의 인자를
`rootMotion`으로 바꾸면 끝이다. 스테이지 클록이 이미 리셋되므로 시간 변환은 없다.

## 4. G03 — 밸런스 전환

### 4.1 `Data/Balance/PlayerSkills.json`

34160과 34140의 `skillKind`를 `ACTIVE` → `COMBO`로 바꾸고 §1.4 표의 `comboStages`를 채운다.
`actionDurationMs`는 스테이지 합(3700 / 5384)으로 맞춘다.

### 4.2 `Data/Animation/Authored/LanceMaster/LanceMaster.skillbindings.json`

34160은 `riseup_custom_6`을 버리고 2스테이지로, 34140은 §2.1 모양으로 바꾼다.

### 4.3 provenance receipt

`skill:34160`, `skill:34140`의 `skillKind`, `comboStages.length`, `actionDurationMs`를
동기화한다. 스테이지 필드는 receipt가 개별 항목으로 추적하지 않으므로
`comboStages.length`만 갱신하면 되는지 publish 단계에서 확인한다.

### 4.4 루트모션 재굽기

```powershell
& $blenderPython buildScript\extract_rootmotion.py `
    Client\Bin\Resources\Character\LanceMaster\LanceMaster.wmodel `
    Data\Animation\Authored\LanceMaster\LanceMaster.skillbindings.json `
    LanceMaster Data\Balance\PlayerSkills.json <scratch>\LanceMaster.rootmotion.json
```

먼저 스크래치에 뽑아 기존 문서와 대조한다. **34160·34140·34010 세 스킬만 달라져야 한다**
(34010은 이번 수정으로 처음 루트모션을 얻는다). 나머지 15개가 그대로면 입력과 도구가
일치한다는 증거다. 이번 변경은 여섯 클래스 전부에 적용되므로 나머지 다섯 클래스도 같은
방식으로 재굽고 각자의 평타가 루트모션을 얻는지 확인한다.

## 5. G04 — 도구·검증 경로

- `Client/Private/Animation_Tool.cpp:908-916` — `binding.Clips.assign(clipCount, ...)`가
  스테이지 모델로 바뀐다. COMBO는 스테이지 수만큼 그룹을 만들고 각 그룹에 클립을 추가·삭제하는
  UI가 필요하다. 1190-1201행의 클립 수 고정 안내도 스테이지 단위로 다시 쓴다.
- `Tools/ClientFrontendHarness` — v3 픽스처와 실패 케이스를 추가한다.
  스테이지 수 불일치, 빈 스테이지, ACTIVE에 2스테이지, 중첩 배열 거부.
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` —
  `gameplay.playable-skill-animation-authoring-contract`가 v3 스테이지 모양을 알아야 한다.
- `Server.exe --contract-test` — 스테이지 루트모션 행의 정상·중복·범위 밖 stageIndex 케이스.

## 6. 종료 조건

```text
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug 전 구간 통과
  (NoDefaultCurrentDirectoryInExePath 비우고 Blender python을 PATH 앞에 둘 것)
protocol harness / frontend harness / contract test failures : 0
Gameplay balance Validate + Publish 성공
ProjectAudit 통과
Server 재기동 후 실기 확인:
  R 1번 입력에 riseup_01만, 재입력 창에서 다시 눌러야 riseup_02로 넘어가는가
  R 재입력을 안 하면 riseup_01에서 끝나는가
  A 1번 입력에 chestdestruction_01,02가 연속 재생되는가
  A 재입력에 03,04로 넘어가고 02의 남은 구간이 잘리는가
  R·A·평타가 실제로 전진하는가 (스테이지 루트모션 회귀 확인)
  나머지 다섯 클래스 평타의 이동이 새로 생겼는지, 과하지 않은지
```

## 7. 미결

- **평타의 스테이지 루트모션이 기존 감각을 바꾼다.** 지금까지 이동이 0이었으므로 커브가
  붙는 순간 여섯 클래스의 평타가 전부 전진하기 시작한다. 원작이 그렇다는 것이 사용자 확인
  사항이지만, 실기에서 과하면 스테이지 커브만 별도 조정이 필요할 수 있다.
- `movementDistance` 필드(`PlayerSkillSystem.cpp:255-259`)는 루트모션이 없을 때만 쓰는
  대체 경로다. R·A는 현재 0이라 충돌하지 않지만, 다른 스킬에서 두 값이 함께 설정된 경우가
  있는지 확인이 필요하다.
- HOLD 34590 적룡포도 스테이지 스킬이라 이번 스테이지 루트모션의 영향을 받는다. 홀딩 입력
  계약은 `2026-08-06_LOSTARK_HOLD_SKILL_CONTRACT_RESULT.md` 범위이며 이번 문서에서 동작을
  바꾸지 않는다.
