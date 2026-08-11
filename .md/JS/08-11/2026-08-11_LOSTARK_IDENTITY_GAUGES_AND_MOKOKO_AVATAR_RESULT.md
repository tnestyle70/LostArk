# 클래스별 아이덴티티 게이지 + 모코코 아바타 슬롯 — 결과

작성자: JS · 2026-08-11 · 브랜치 `feature/identity-gauges-and-mokoko-avatar`

세션 하나에 별개 작업 세 개가 들어갔다. 파일 상당수(특히 `Character.cpp`)가 겹쳐서 커밋은
하나로 묶었다.

1. 창술사 아바타 머리/방어구 슬롯 구조 (모코코 "기분 좋은" 세트)
2. 창술사·도화가·차원술사·워로드 identity gauge 4종 소모 방식
3. 워로드 풀배럴 캐넌(17240) 충전 애니메이션 재구성

## 1. 아바타 머리/방어구 슬롯

### 1.1 에셋 확보

`umodel_lostark_v7.exe`로 `PC_FT_AV_036` 패키지(모코코 아바타, "기분 좋은" 변형 `036-1`)를
추출했다. 헤드는 FT 전용 텍스처가 없고 `pc_wr_av_036-1_head_mi`(WR 그룹 텍스처)를 그대로
참조한다 — 모코코 탈이 얼굴을 다 가리니 원작이 성별별 헤드 텍스처를 안 만들고 재사용한
것으로 보인다. 창술사 221본 마스터 아마추어에 리바인딩해 FBX로 뽑고
`ModelAssetConverter.exe`로 쿠킹했다.

```text
Client/Bin/Resources/Character/LanceMaster/LanceMaster_Helmet_Mokoko.wmodel
Client/Bin/Resources/Character/LanceMaster/LanceMaster_Upper_Mokoko.wmodel
```

(`Client/Bin/Resources/`는 `.gitignore` 대상이라 이 두 파일은 커밋에 안 잡힌다. 팀장 관리
물리 폴더 규칙 그대로.)

### 1.2 슬롯 구조

기존 5파츠(Upper/Lower/Arm/Shoulder/Helmet) 장비는 그대로 두고, 그 위에 아바타 전용 슬롯
2개를 얹는 구조로 갔다 — "기본 장비를 아바타로 교체"가 아니라 "아바타 슬롯이 기본 장비를
가리는" 방식.

`EQUIPMENT_SLOT_KIND` 4종을 새로 만들었다(`CharacterSpec.h`).

```cpp
enum class EQUIPMENT_SLOT_KIND
{
	DEFAULT,        // 팔/상의/하의/어깨 등 일반 방어구
	DEFAULT_HELMET,  // 기본 머리 — AVATAR_HEAD가 있으면 가려짐
	AVATAR_HEAD,     // 존재하면 DEFAULT_HELMET만 가림
	AVATAR_ARMOR,    // 존재하면 DEFAULT_HELMET을 제외한 모든 DEFAULT를 가림
};
```

숨김 규칙은 `CCharacter::Ready_PartObjects`(`Character.cpp`) 한 곳에서 일반화해서 처리한다.
클래스 쪽(`Logic_LanceMaster.cpp`)은 슬롯 종류만 선언하면 되고 숨김 로직을 따로 안 짠다.

```cpp
bool_t hasAvatarHead = false, hasAvatarArmor = false;
// 1차 스캔으로 두 플래그 확정
...
const EQUIPMENT_SLOT_KIND eKind = m_pSpec->pEquipment[i].eSlotKind;
bool_t isHidden = m_pSpec->pEquipment[i].isHidden;
if (EQUIPMENT_SLOT_KIND::DEFAULT_HELMET == eKind && hasAvatarHead)
	isHidden = true;
else if (EQUIPMENT_SLOT_KIND::DEFAULT == eKind && hasAvatarArmor)
	isHidden = true;
```

**중요한 정정**: 처음엔 `hasAvatarArmor`가 헬멧도 가리게 짰다가 사용자가 잡아줬다 — 아바타
방어구는 헬멧을 제외한 나머지만 가리는 게 맞다. 헬멧 가시성은 오직 `AVATAR_HEAD`만 결정한다.

지금은 두 아바타 슬롯 다 정적으로 "항상 장착"이다. 실시간 착용/탈착 UI는 없다 — 그건 별도
수직 슬라이스.

### 1.3 배관

- `Data/Actors/CharacterCatalog.json`의 창술사 `equipmentModels`에 두 wmodel 경로를 append(교체
  아님, 기존 5개 뒤에 추가).
- `PlayableCharacterAssetService.cpp`의 `CHARACTER_PROTOTYPE_TAGS::Equipment` 배열을
  `std::array<..., 6>` → `std::array<..., 8>`로 늘리고 창술사 항목에 프로토타입 태그 2개 추가.
  배열 크기는 전 클래스 공용이라 다른 5클래스는 초기화 리스트가 그대로 유효하다(워로드가 이미
  6개 써서 6이었던 걸 8로).
- `Logic_LanceMaster.cpp`의 `Equipment[]`에 `Part_15_Avatar_Head` / `Part_15_Avatar_Armor` 추가.

## 2. Identity gauge 4종

### 2.1 기존 상태

`Data/Balance/PlayerProfiles.json`의 `maximumIdentity`/`identityRegenPerSecond`/
`identityDrainPerSecond`는 원래 6클래스 중 워로드만 실제 값(1000/40/120)이고 나머지는 전부 0.
서버 쪽 `Update_Identity`(`PlayerSkillSystem.cpp`)는 "대체 스탠스를 유지하는 동안 드레인,
안 하면 리젠, 0 되면 강제로 기본 스탠스 복귀"라는 워로드 전용 hold-drain 모델 하나만 알았다.

이번에 4클래스가 서로 다른 소모 방식을 요구하면서 방식이 4개로 늘었다. 전부
`maximumIdentity=100`으로 통일했다(사용자 지정, 워로드 1000→100도 리젠/드레인을 비례
축소해서 실제 타이밍은 그대로 보존).

| 클래스 | 방식 | 필드 |
|---|---|---|
| 워로드 | Hold-drain: 대체 스탠스 유지 중 초당 드레인, 0 되면 강제 복귀 | `identityDrainPerSecond=12` |
| 창술사 | Switch-cost: 스탠스 전환 순간 1회 소모, 부족하면 그냥 무료 전환 | `identityStanceSwitchCost=33` |
| 도화가 | Skill-cost: 스킬 단위 소모, 부족하면 스킬 자체가 발동 안 함 | 스킬별 `identityCost` |
| 차원술사 | Cyclic: 100 찍으면 즉시 0으로 감고 계속 리젠(시계 컨셉), 아무것도 소모 안 하고 wrap 자체가 소모 | `identityCyclic=1` |

### 2.2 창술사 — switch-cost

장창/단창 전환은 이미 있던 메커니즘(`skill->eSetsStance`가 확정되는 순간)에 훅을 걸었다.
`PlayerSkillSystem.cpp`의 combo-stage 종료 처리:

```cpp
if (PLAYER_STANCE_ID::NONE != skill->eSetsStance)
{
	if (skill->eSetsStance != player.eStance)
	{
		if (const PLAYER_RUNTIME_PROFILE* stanceProfile =
			catalog.Find_Player(player.eCharacterClass))
		{
			if (0u != stanceProfile->iIdentityStanceSwitchCost &&
				player.iCurrentIdentity >= stanceProfile->iIdentityStanceSwitchCost)
			{
				player.iCurrentIdentity -= stanceProfile->iIdentityStanceSwitchCost;
			}
		}
	}
	player.eStance = skill->eSetsStance;
}
```

부족하면 그냥 차감 없이 전환만 일어난다 — 실제 스탠스 전환 자체를 막지 않는다.

`Is_HoldingGaugedStance`도 `identityDrainPerSecond` 존재 여부로 게이트했다
(`0u != profile.iIdentityDrainPerSecond && ...`). 창술사는 드레인이 0이라 이 함수가 항상
false를 반환하고, `Update_Identity`는 hold-drain 분기를 절대 안 타고 순수 리젠만 한다 —
그래서 단창 스탠스를 오래 유지해도 강제로 장창으로 안 돌아간다.

### 2.3 도화가 — skill-cost

`PLAYER_SKILL_DEFINITION`에 `iIdentityCost`를 신설(`resourceCost`와 완전히 같은 자리에서 같은
방식으로 작동). `Try_Start`의 게이트/차감에 나란히 추가.

```cpp
if ((cooldown ...) || player.iCurrentResource < skill->iResourceCost ||
	player.iCurrentIdentity < skill->iIdentityCost)
	return false;
...
player.iCurrentResource -= skill->iResourceCost;
player.iCurrentIdentity -= skill->iIdentityCost;
```

이미 있던 도화가 Z(31050 "저무는 달")/X(31110 "떠오르는 해") 스킬을 그대로 썼다 — 원본
`clipmap`에 이 둘이 정확히 문/썬 페어링으로 skill=17240... 아니, skill=31050/31110으로 잡혀
있고 둘 다 지금까지 `resourceCost=0`인 빈 스텁이었다. 여기에 소모 게이트만 새로 연결했다.

```text
Z(31050, 달, 딜 버프 예정) = 2구슬 = identityCost 66
X(31110, 해, 파티 회복 예정) = 1구슬 = identityCost 33
```

버프/회복 자체는 손 안 댔다 — 여전히 빈 스텁. `구슬 1개 = maximumIdentity/3 ≈ 33`으로 잡았다
(정확히 3등분은 안 되지만 사용자가 "약 33.3%"로 확인).

### 2.4 차원술사 — cyclic

새 필드 `iIdentityCyclic`(0/1). `Update_Identity`의 리젠 분기를 캡 방식에서 랩어라운드
방식으로 바꿨다.

```cpp
else if (isCyclic)
{
	++player.iCurrentIdentity;
	if (player.iCurrentIdentity >= player.iMaximumIdentity)
		player.iCurrentIdentity = 0u;
}
```

바깥쪽 "이미 꽉 찼으면 정지" 게이트(`!isHolding && player.iCurrentIdentity >= maximum`)도
`!isCyclic` 조건을 추가해서 cyclic 클래스는 절대 안 멈춘다. 100 도달 시 효과(전체 스킬 쿨감)는
사용자가 명시적으로 보류 — 지금은 그냥 무한 순환만 한다.

### 2.5 검증 규칙 3중 동기화

`maximumIdentity != 0`인데 아무것도 소모하지 않는 클래스는 막아야 한다는 원칙 자체는 기존에
있었지만("A gauge that never drains would hold a stance open forever"), 소모 방식이 4개로
늘면서 "아무것도 소모 안 함"의 판정 기준이 훨씬 복잡해졌다. 이 판정을 **세 군데**에서 똑같이
유지해야 한다.

1. `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` — 스킬 문서까지 읽은 뒤 post-check로
   판정(플레이어 루프 안에서는 스킬 정보가 아직 없어서 못 함).
2. `Server/Private/GameplayCatalog.cpp` — 같은 이유로 스킬 로드 이후 post-check.
3. 둘 다 "drain != 0 OR switchCost != 0 OR cyclic != 0 OR 그 클래스 스킬 중 identityCost != 0인
   게 있다" 넷 중 하나.

여기서 실제로 빌드가 통째로 깨지는 버그를 냈었다 — GameplayCatalog.cpp의 PLAYER 행 인라인
검증에 옛날 3방식 기준의 "절대 소모 안 함" 체크가 그대로 남아 있어서, 도화가(스킬-cost만
있고 드레인/스위치는 0)의 PLAYER 행이 파싱 단계에서 바로 거부당했다. `catalog.Load()`가
실패하면서 contract test 290개가 연쇄로 죽었다. 원인은 이 인라인 체크가 스킬 데이터를 아직
안 읽은 시점에 있었다는 것 — PS1 스크립트는 이미 스킬 로드 후로 옮겨놨는데 C++는 옮기는 걸
빠뜨렸다. 스킬 로드 완료 후의 post-check로 옮겨서 해결했다(`GameplayCatalog.cpp`
`Load()` 끝부분, `largestIdentityPool` 계산 옆).

### 2.6 부트스트랩 포맷 변경

`PLAYER` 행 필드 13개→15개(`identityStanceSwitchCost`, `identityCyclic` 추가),
`SKILL` 행 필드 15개→16개(`identityCost` 추가). `GameplayCatalog.cpp`의 필드 인덱스와
`ServerGameplayContractTests.cpp`의 하드코딩된 fixture 문자열(`PLAYER\t...`, `SKILL\t...`)
전부 같이 고쳤다 — 안 고치면 필드 개수 불일치로 그 fixture들만 조용히 파싱 실패한다.

`Data/Balance/PlayerSkills.json`의 스킬 90개 전부에 `identityCost` 필드를 추가해야 했다(스키마가
`Assert-ExactProperties`로 필드 집합을 엄격히 강제). 손으로 90개를 고치는 대신 Python
스크립트로 일괄 삽입했다(`resourceCost` 바로 뒤, 도화가 2개만 실값 나머지 0).

`Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`도 같이 늘었다.
publisher가 모든 balance JSON 필드를 receipt와 1:1로 대조하기 때문에, 새 필드마다 receipt
entry가 없으면 "coverage count mismatch"로 즉시 거부된다. `identityCost` 90개 + `identityCyclic`
6개 + 바뀐 기존 값(창술사/도화가/차원술사/워로드 identity 필드들) receipt entry를 같이 넣었다.
90개짜리는 이것도 스크립트로 처리했다 — receipt의 기존 `PROJECT_TUNED` 블록 텍스트를 템플릿
삼아 정확히 같은 포맷으로 삽입(파일 전체를 JSON 파서로 재직렬화하면 PowerShell
`ConvertTo-Json`의 콜론 뒤 2칸 공백 스타일이 전부 깨져서 diff가 터지므로, 원문 텍스트 삽입
방식을 썼다).

## 3. 워로드 풀배럴 캐넌(17240) 충전 애니메이션

### 3.1 문제

원작은 차지 스킬인데, 프로젝트 데이터는 HOLD 3스테이지(charge, charge, attack)로 charge
스테이지가 하나뿐이었다. 사용자가 원하는 건 charge가 2번(clip 2→3→4로 3단계 파워) 진행되는
것.

서버 규칙상 HOLD는 스테이지가 **정확히 3개**여야 한다(`ProjectAudit`의
`gameplay.playable-skill-animation-authoring-contract`가 "hold needs start/loop/end"로
강제, publisher도 `Hold stage timing is invalid`로 별도 체크). 그래서 스테이지를 4개로
늘리는 대신, **가운데 loop 스테이지 하나 안에 클립 3개를 체인**으로 묶었다.

```json
"clips": [
  [ "wgl_sk_eternalcyclone_01" ],
  [
    { "clip": "wgl_sk_eternalcyclone_02", "playMs": 300 },
    { "clip": "wgl_sk_eternalcyclone_03", "playMs": 300 },
    "wgl_sk_eternalcyclone_04"
  ],
  [ "wgl_sk_eternalcyclone_07" ]
]
```

`playMs`는 클립 길이(`.animnotify`의 `len=1.0000`, 1000ms)가 아니라 `d=0.3000`(30fps 기준
9프레임)짜리 CANCEL 윈도우("회전동기화"/"진격태세", `win=OTHER`)를 기준으로 잡았다 — 사용자가
"9에서 넘어가야 원작이랑 비슷할 것"이라고 짚은 지점과 일치했다.

`comboStages[1].actionDurationMs`도 3000→900(300×3)으로 같이 줄였다 — 전체 충전 시간을
원작에 가깝게 만드는 결정이라 사용자 확인 후 반영. 스킬 top-level `actionDurationMs`는
HOLD 스테이지 합과 같아야 한다는 publisher 규칙(`Hold stage durations must sum to
actionDurationMs`) 때문에 1000+900+1100=3000으로 다시 계산했다.

### 3.2 C++ 버그 — loop 플래그가 스테이지 전체에 걸림

처음 반영 후 "안 바뀌었다"는 피드백을 받았다. 원인은 `Character.cpp`의
`isHoldLoop` 계산이 **스테이지 안 클립 전부**에 `true`를 매겼다는 것 — 원래 loop 스테이지가
클립 1개뿐이던 시절 코드라 문제가 없었는데, 3개로 늘리면서 앞쪽 두 클립(02, 03)도 자기
자신을 반복 재생하게 됐다. `playMs`(=클립 자체 길이 1000ms)와 클립이 매 프레임 랩어라운드하는
타이밍이 겹치면서, `Is_ClipFinished()`의 `fPosition >= fLimit` 판정이 랩 직전에 매번 새로
리셋되는 위치를 보게 되는 레이스가 생겨 사실상 02에서 영원히 멈춰 있었다.

수정: loop 플래그는 스테이지의 **마지막 클립에만** 준다.

```cpp
for (std::size_t clipIndex = 0; clipIndex < stage.Clips.size(); ++clipIndex)
{
	const ANIMATION_SKILL_CLIP& clip = stage.Clips[clipIndex];
	const bool_t isHoldLoop =
		isHold && 1u == chain.stages.size() &&
		3u == binding.Stages.size() &&
		clipIndex + 1u == stage.Clips.size();
	...
}
```

기존 1클립 스테이지는 `clipIndex+1==size`가 항상 참이라 동작 그대로다.

## 4. 검증

```text
Server 빌드                        성공 (여러 차례, 최종본 성공)
Client 빌드                        성공
Server --contract-test             failures : 0 (identity 버그 수정 전 290개 실패 → 수정 후 0)
Publish-GameplayBalance.ps1        Publish 성공 (여러 차례, 최종본 성공)
Invoke-ProjectAudit.ps1            신규 실패 없음. 기존 9개(이펙트 파이프라인, Artist/31210
                                    manifest mismatch 등)는 이번 세션 시작 전부터 있던 것과 동일
Server+Client 로컬 루프백(127.0.0.1) 실행  실제 확인 — 아바타 표시, 창술사 스탠스 전환,
                                    풀배럴 캐넌 02→03→04→공격 전환 전부 사용자가 눈으로 확인
```

## 5. 다음에 할 것

- 도화가 Z/X(달/해) 실제 딜 버프·파티 회복 효과 — 지금은 구슬 소모만 되고 효과는 빈 스텁.
- 도화가/차원술사도 HUD에서 identity gauge가 실제로 보이는지는 UI 담당자 작업 — 서버는
  `iCurrentIdentity`/`iMaximumIdentity`를 클래스 무관하게 그대로 넘기니 추가 작업 불필요.
- 차원술사 identity 100 도달 시 효과(전체 스킬 쿨다운 감소)는 사용자가 명시적으로 보류.
- 아바타 슬롯은 정적 장착만 있고 실시간 착용/탈착 커맨드는 없음 — 필요해지면 별도 슬라이스.
- 워로드 풀배럴 캐넌의 놓친 클립(05, 06)은 이번에 안 썼다 — clipseq의 seq=4(01→06 SEQUENCE)가
  다른 변형(즉시 발사?)일 가능성이 있는데 확인 안 함.
