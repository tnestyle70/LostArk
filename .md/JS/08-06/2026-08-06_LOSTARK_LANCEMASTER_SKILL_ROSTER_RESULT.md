# 창술사 스킬 로스터 교체 — 구현 결과

작성자: JS · 2026-08-06 · 브랜치 `feature/player-hold-skill`

사용자가 지정한 슬롯 구성으로 창술사 스킬 로스터를 교체하고, 모든 바인딩을 트라이포드 없는
기준 체인으로 맞췄다. 코드 변경은 없고 `Data` 원본 5개만 바뀐다.

## 1. 트라이포드 없는 기준이 무엇인가

`Data/Animation/Reference/LanceMaster/LanceMaster.clipseq`의 `seq=` 그룹이 트라이포드 변형
단위다(07-31 추출 문서 §9의 `CEFActionObject → 그룹 → CEFActionStage` 구조). 그래서 기준
체인은 **스킬별 최저 seq 그룹**이다. 그룹 번호가 0에서 시작하지 않는 스킬이 있으므로
(34620은 2부터, 34650은 2부터) `seq=0`을 고정 조건으로 쓰면 안 된다. 런타임 `Play_Skill`이
이미 같은 최저 체인 폴백을 쓴다.

**이름의 `custom_`은 트라이포드 표식이 아니다.** `flm_sk_crescentsweep_custom_5`와
`flm_sk_stab_custom_3_01`은 기준 체인의 일부다. 판별은 이름이 아니라 seq 그룹으로 한다.

## 2. 슬롯 구성

| 키 | 긴 창 | 짧은 창 | 스탠스 무관 |
|---|---|---|---|
| LMB | 34010 평타 | 34510 평타 | |
| Q | **34040 이연격** | 34540 나선창 | |
| W | 34090 철량추 | 34550 사두룡격 | |
| E | **34100 청룡출수** | 34560 굉열파 | |
| R | **34160 공의연무** | 34570 유성강천 | |
| A | **34140 선풍참혼** | 34580 절룡세 | |
| S | 34120 연환섬 | 34590 적룡포 | |
| D | 34110 반월섬 | — | |
| F | 34150 맹룡열파 | — | |
| Z | 34000 전환 | 34500 전환 | |
| SPACE | 34020 탄영 | 34520 돌파 | |
| T | | | **34650 적룡필살** |
| V | | | **34610 적룡질풍격** |
| ALT+V | | | **34630 마룡합일섬** |

굵은 글씨 7개가 신규다. 빠진 7개는 일섬각(34080)·회선창(34070)·풍진격(34050)·청룡진(34170)·
맹룡난무(34640)·은하유성탄(34600)·은하비섬창(34620)이다.

T/V/ALT_V 세 개가 스탠스 무관인 근거는 `.clipseq` 이름 접두사다(08-03 계획서 §2). 적룡필살
`초각성스킬_`, 적룡질풍격 `연가창식:`, 마룡합일섬 `초각성기_연가창식:` 전부 `긴 창_`/`짧은 창_`
접두사가 없다. ID 범위로 판별하면 틀린다.

기존 바인딩 둘도 기준 체인에서 벗어나 있어 고쳤다.

| skillId | 기존 | 기준 체인 |
|---|---|---|
| 34110 반월섬 | `crescentsweep` | `crescentsweep` + `crescentsweep_custom_5` |
| 34580 절룡세 | `counterattack_02` | `counterattack_01` + `counterattack_02` |

34590 적룡포는 seq=0이 `start,loop,end,end`로 end가 두 번이지만 HOLD 계약이 스테이지 3이라
현재 3개를 유지한다.

## 3. 수치는 전부 원본에서 뽑았다

추측값 없다. receipt가 쓰던 좌표 규약을 그대로 따랐다.

| 필드 | 출처 | basis |
|---|---|---|
| `cooldownMs` / `resourceCost` | `EFTable_Skill.Skill` PK=skillId, SK=참조레벨, `Cooltime`/`CostMp` | OFFICIAL_EXTRACTED |
| `maximumRange` | 같은 행 `MaxRange`. 0이 아니면 `/100`, 0이면 project override | OFFICIAL_DERIVED / PROJECT_TUNED |
| `damageRatePercent` | `EFTable_SkillEffect.SkillEffect` PK=skillId×10+변형, `ValueA` | OFFICIAL_EXTRACTED |
| `actionDurationMs` | `.animnotify` `len=` 클립 길이 합, `playRate` 반영 | PROJECT_TUNED |
| `hitTimeMs` | 체인 첫 `kind=HIT` 노티파이 시각 + 선행 클립 길이 | PROJECT_TUNED |

참조 레벨은 receipt의 `referenceSkillLevel: 10`이다. 각성기 3종(34610/34630/34650)은
`Skill` 행이 SecondaryKey 1에만 있어 1을 썼다.

먼저 같은 쿼리로 기존 값이 재현되는지 확인했다. 34120=361, 34090=1050, 34150=3839,
34110=499, 34080=261, 34070=459, 34120의 10000/276/800→8.0이 전부 저장소 값과 일치했다.

신규 7개 결과:

| skillId | cd | mp | range | dur | hit | rate |
|---|---|---|---|---|---|---|
| 34040 이연격 | 6000 | 206 | 3.0 | 1500 | 300 | 301 |
| 34100 청룡출수 | 20000 | 410 | 3.5 | 4333 | 300 | 298 |
| 34140 선풍참혼 | 15000 | 347 | 8.0 | 5600 | 450 | 439 |
| 34160 공의연무 | 16000 | 361 | 3.0 | 5667 | 500 | 344 |
| 34610 적룡질풍격 | 300000 | 0 | 8.0 | 6434 | 900 | 2639 |
| 34630 마룡합일섬 | 300000 | 0 | 6.0 | 6800 | 1445 | 67652 |
| 34650 적룡필살 | 55000 | 734 | 30.0 | 2433 | 1240 | 4541 |

### 3.1 적룡필살만 변형 선택 규칙을 벗어났다

다른 스킬의 규칙은 "sentinel이 아닌 가장 낮은 변형"이다(34600이 `346000`의 자기참조를 건너뛰고
`346001`을 쓴 전례). 34650에 그대로 적용하면 `346501 = 454`가 된다.

그런데 34650의 효과 행 중 `HitTypeTimeMin`을 가진 것은 `346504` 하나뿐이고 그 값 1240이
`.skilltiming`의 34650 base 행 `hits="1.24-1.305"`와 정확히 일치한다. 실제 타격 판정을 가진
행이 346504라는 뜻이다. 454는 50초 쿨다운 34640 맹룡난무(1530)보다 낮아 앞뒤도 안 맞는다.
그래서 `346504 = 4541`을 썼다.

**규칙보다 증거를 택한 것이므로 되돌릴 여지가 있다.**

### 3.2 로컬 DB 해시가 receipt와 다르다

receipt는 `EFTable_Skill.db`를 `8691edbd…`로 기록하는데 `C:\Users\95jus\Downloads\SourceData`의
파일은 `303bf047…`다. `EFTable_SkillEffect.db`도 마찬가지다. 값이 전부 재현되므로 내용은 같은
빌드로 보이지만 파일 자체는 receipt를 만들 때 쓴 사본이 아니다.

publisher는 receipt 내부 일관성만 검사하고(`Publish-GameplayBalance.ps1:172`) 실제 파일을
해싱하지 않으므로 새 항목에도 receipt가 기록한 해시를 그대로 썼다. **확인하지 않은 파일을
근거로 적은 셈이니 팀장이 원본 사본을 맞출 때 같이 정리해야 한다.**

## 4. 루트 모션 재굽기

`buildScript/extract_rootmotion.py`를 Blender 5.0 번들 인터프리터로 실행해 문서를 재생성했다.
바인딩과 `PlayerSkills.json`을 입력으로 받으므로 새 로스터가 그대로 반영된다.

18개가 곡선을 갖는다. COMBO·HOLD는 스테이지별 시계를 써서 스크립트가 건너뛴다(평타 2종,
적룡포). 반월섬 1250→2667, 절룡세 1300→3300으로 새 체인 길이에 맞춰졌다.

### 4.1 공의연무 두 가지

**앞으로 6.527m 간다.** 체인 세 클립이 각각 2.50 / 1.58 / 2.45를 더한 결과다. 원본이 그렇게
저작돼 있고 기준 체인이므로 그대로 뒀다.

**`up`이 -1.986으로 끝난다.** `riseup_02`와 `riseup_custom_6`이 각각 공중에서 시작해 1m씩
내려오는 클립인데, 베이커가 클립마다 자기 0프레임을 기준으로 삼아 시작 높이를 잃고 하강분만
누적한다. 다만 publisher가 `SKILLROOTMOTION` 행에 `timeMs:forward:lateral`만 담고 `up`은
넣지 않으므로(`Publish-GameplayBalance.ps1:613`) 서버 판정에는 영향이 없다. 저작 문서에만
남는 값이다.

## 5. 걸린 것

**ProjectAudit이 바인딩과 `PlayerSkills.json`을 1:1로 강제한다.** `Invoke-ProjectAudit.ps1:1066`이
개수를, `:1100`이 스킬별 존재를 검사한다. "바인딩부터" 따로 할 수 없고 슬롯 정의·damage
프로파일·receipt까지 한 변경 단위여야 한다.

**receipt를 끝에 append했더니 diff가 7148줄이 됐다.** 중간에서 133개를 빼고 뒤에 133개를
붙이면 이후 전 항목이 밀려 무관한 스킬까지 diff에 들어온다. 은퇴하는 슬롯 자리에 새 스킬을
그대로 치환하도록 바꿔 874줄로 줄였다. 대응은 슬롯 기준으로 뒀다(34600→34610, 34620→34630,
34640→34650은 V/ALT_V/T가 그대로 이어진다).

**`git checkout --`와 `git stash pop`이 파일을 CRLF로 되돌린다.** `core.autocrlf` 때문이다.
receipt 텍스트 수술이 `\n` 기준이라 한 번 깨졌다. 두 번 다 LF로 되돌려 놓았고 최종 5개 파일
전부 LF다.

**`movementDistance`는 `0`이 아니라 `0.0`이어야 한다.** publisher가 `ConvertTo-Json`으로
비교해서 정수 `0`은 `0`, 실수 `0.0`은 `0.0`이 되어 불일치로 막힌다.

## 6. 검증

- `Publish-GameplayBalance.ps1 -Mode Validate` / `-Mode Publish` 성공 — 5 player profiles,
  91 skills, 63 damage profiles, 1 bosses
- 바인딩 23개 전부 기준 체인과 일치 확인(34590 HOLD는 계약대로 3스테이지)
- 바인딩된 클립 54개가 전부 `LanceMaster.wmodel`에 존재
- `ProjectAudit`: `projects.data-source-visibility` 실패(218 vs 216) 하나. 변경분을 stash하고
  다시 돌려 같은 실패를 확인했다 — SkillWindow 2개 미등록으로 기존 것이다
- `git diff --check` 통과, 5개 파일 LF 유지

**인게임 실행 확인은 아직 안 했다.**

## 7. 남은 것

- 신규 7개는 `playRate` 조정이 없어 클립 원본 길이 그대로다. 선풍참혼 5600ms, 공의연무 5667ms,
  마룡합일섬 6800ms가 길다. 인게임에서 보고 `playMs`/`playRate`로 줄인다.
- 적룡필살 damage 근거 4541 대 454 확정.
- 공의연무 전진 6.5m가 과한지 인게임 확인.
- `.skilltiming` base 행이 없는 34040·34140·34160·34610은 `hitTimeMs`를 `.animnotify` HIT
  노티파이로 채웠다. 다단 히트 `hits[]`는 여전히 미착수라 첫 타만 반영된다.
