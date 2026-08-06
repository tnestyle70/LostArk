# 차원술사 스킬 로스터 — 구현 결과

작성자: JS · 2026-08-07 · 브랜치 `feature/player-hold-skill`

사용자가 지정한 슬롯 구성으로 차원술사 로스터를 11개에서 13개로 교체했다. 창술사·워로드·도화가와
같은 절차이며 이번에 새로 드러난 것은 Effect 문서와 퀵슬롯의 수명이 다르다는 사실이다.

## 1. 슬롯

| 키 | skillId | 이름 | 종류 | 쿨다운 | 마나 | 배율 |
|---|---|---|---|---|---|---|
| LMB | 2050010 | 기본 공격 | COMBO 4단 | — | 0 | 100 |
| Q | **2050100** | 일침 | ACTIVE | 14s | 382 | 740 |
| W | **2050120** | 분절 | ACTIVE | 18s | 441 | 300 |
| E | **2050160** | 건너 찌르기 | ACTIVE | 28s | 569 | 1418 |
| R | **2050180** | 너머 베기 | ACTIVE | 40s | 698 | 1368 |
| A | 2050210 | 분광 | ACTIVE | 55s | 839 | 1788 |
| S | 2050220 | 일점 관통 | ACTIVE | 40s | 698 | 3056 |
| D | 2050240 | 경계 돌파 | ACTIVE | 50s | 794 | 4722 |
| F | **2050230** | 시간 분쇄 | ACTIVE | 55s | 839 | 2459 |
| T | 2050500 | 업의 경계 | ACTIVE | 120s | 938 | 100 |
| V | **2050520** | 시간의 굴레 | ACTIVE | 300s | 0 | 100 |
| ALT+V | 2050540 | 무간의 옥 | ACTIVE | 300s | 0 | 100 |
| SPACE | **2050020** | 공간 도약 | ACTIVE | 8s | 0 | — |

빠진 것은 예고(2050110)·공간 베기(2050150)·진공(2050190)·공간 절단(2050200)·일념(2050510)
다섯이다. `Z`와 `X`는 쓰지 않는다. 차원 시계와 공간 조작(2050130~2050133) 계열은 이번 범위 밖이다.

## 2. SPACE는 DB로 확정했다

clipseq에는 2050020이 "회피기"로 적혀 있어 이동기인지 회피기인지 애매했다. `EFTable_PC` PK 611
Specialist_Male의 `MoveSkillId1 = 2050020`이 정답이고, 클립도 `moving_normal_1_04, jump_04`다.
창술사 탄영(34020)·워로드 돌격(17020)과 같은 자리다. 공간 조작(2050130)은 별개 스킬이다.

## 3. mode= 를 인게임 동작으로 덮었다

`.clipseq`의 `mode=`가 W와 E를 COMBO로 잡았지만 사용자 확인 결과 둘 다 한 번 누르면 끝까지 가는
ACTIVE다. 클립 길이가 설명과 정확히 맞는다.

```text
분절        _01 400ms 칼 던지기 → _02 300ms 점프 → _03 1567ms 베면서 낙하
건너 찌르기  _01 633ms 진입 → _02/_03/_04 각 1500ms 찌르기 3번 → _05 767ms 마무리
```

**건너 찌르기의 기준 체인은 11클립인데 뒤 6개가 `overslash_02` 반복이다.** 그대로 쓰면 찌르기가
아홉 번이 되므로 서로 다른 앞 5클립만 넣었다. 원본 체인 그대로가 아니라 인게임 동작에 맞춘
유일한 지점이다.

`mode=`가 추론값이라는 것은 07-31 추출 문서 §10.2에 적혀 있고 워로드에서도 같은 판단을 했다.

## 4. 기존 PROJECT_TUNED 둘에 근거가 붙었다

툴팁 매크로가 셀을 직접 지목한다.

```text
tip.desc.skill_2050220  <$MACRO magic_ch @1:20502210 @2:612/>
tip.desc.skill_2050180  <$MACRO physic_ch @1:20501810/…11/…12 @2:612/>
```

- **일점 관통 3056** — `20502210/s10`. 기존에 손으로 넣은 값과 정확히 일치해
  `PROJECT_TUNED` → `OFFICIAL_EXTRACTED`로 승격했다.
- **너머 베기 1368** — 3타 스킬이고 셀 값이 1368/1368/1825다. 서버가 타격을 한 번만 적용하므로
  첫 타를 쓴다. 다단 히트 미구현이라 실제 총합 4561보다 낮게 들어간다.

업의 경계(2050500)는 툴팁이 `(공격력 × ValueF/10000 + (ValueA+ValueB)/2) × 12` 공식을 쓴다.
우리 damage 모델과 형태가 달라 기존 값 100을 유지하고 `20505010/s1`의 `ValueA`를 근거로 달았다.

**`skillId × 10` 규약이 성립하지 않는 경우가 또 나왔다.** 워로드 풀배럴 캐넌과 같은 패턴이니
효과 행이 비었거나 값이 이상하면 툴팁 매크로를 먼저 본다.

## 5. 걸린 것 — effectId를 통째로 지웠다

로스터를 갈면서 차원술사 행을 전부 새로 쓸 때 `effectId`를 빈 문자열로 넣었다.
**차원술사는 여섯 클래스 중 유일하게 `effectId`가 채워져 있던 클래스**였고, 그래서 하네스의
`All Effects Joins Every PlayerSkills EffectId To Valid Authored Document`가
`mappedCount != 0`에서 실패했다.

로스터에 남는 여섯(2050010/2050210/2050220/2050240/2050500/2050540)의 `effectId`를
`PlayerSkills.json`과 receipt 양쪽에 복구했다. 저작 문서가 실재하는지 파일 존재로 확인한 뒤에
넣었다.

## 6. Effect 계약 조정 — 저작 문서는 슬롯보다 오래 산다

`Tools/ProjectAudit/Test-EffectToolFinal.ps1`이 차원술사 스킬 11개를 고정 목록으로 검사한다.
각 ID마다 `PlayerSkills.json` 행 존재와 `effectId` 연결을 요구하는데, 빠진 다섯도 저작 Effect
문서가 실재해 로스터 축소가 이 계약을 깨뜨렸다. 사용자 컨펌을 받고 조정했다.

**저작 문서·`EffectCatalog.json`·`DimensionMaster.animevents`는 하나도 건드리지 않았다.**
documents 11, resources 4, cues 14 그대로다. 대신 `$boundSkillIds`를 따로 두어 `PlayerSkills`
조인만 현재 슬롯에 걸린 여섯으로 좁혔다.

```powershell
# 저작된 Effect 문서는 그것이 쓰이던 슬롯보다 오래 남는다. 문서·카탈로그·애니메이션 큐는
# 여전히 열한 개를 전부 덮고, PlayerSkills 조인만 지금 시전 가능한 스킬로 제한한다.
$boundSkillIds = @(2050010, 2050210, 2050220, 2050240, 2050500, 2050540)
```

빠진 다섯의 Effect 문서는 고아로 남는다. 나중에 그 스킬을 다시 슬롯에 올리면 그대로 이어진다.

## 7. 검증

```text
Engine / Shared / NetworkProtocolHarness / Server / Client   빌드 성공
NetworkProtocolHarness        failures : 0
ClientFrontendHarness         failures : 0
Server.exe --contract-test    failures : 0
Effect Tool bundle            PASS (documents 11, resources 4, cues 14)
Gameplay balance Validate     6 profiles / 125 skills / 77 damage profiles
바인딩                        13개, 클립 24개 전부 모델에 존재
git diff --check              통과
```

`ProjectAudit`의 `projects.data-source-visibility`는 224 vs 222로 기존 실패가 남아 있다
(`Data/UI/SkillWindow/`의 두 파일). 이번 작업은 `Data` 파일을 새로 만들지 않았다.

**인게임 확인(사용자): 정상 동작한다.**

## 8. 남은 것

- 다단 히트 미구현. 너머 베기 3타 중 1타만 적용된다.
- 루트 모션이 없다. 공간 도약(2050020)·분절(2050120)·건너 찌르기(2050160)가 이동을 포함하는데
  제자리에서 재생된다.
- 건너 찌르기의 반복 꼬리 여섯 클립을 버렸다. 트라이포드나 지속 시간 확장과 관련된 것이라면
  나중에 다시 봐야 한다.
- 아이덴티티(차원 시계, 공간 조작 2050130~2050133)는 슬롯에 없다.
- 빠진 다섯의 Effect 문서가 고아 상태다.
