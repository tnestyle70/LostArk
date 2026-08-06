# 도화가 스킬 로스터 — 구현 결과

작성자: JS · 2026-08-07 · 브랜치 `feature/player-hold-skill`

사용자가 지정한 슬롯 구성으로 도화가 로스터를 9개에서 15개로 교체하고 모든 바인딩을
트라이포드 없는 기준 체인으로 맞췄다. 창술사·워로드와 같은 절차이며 이번에 새로 드러난 것은
HUD의 COMBO 가정 하나다.

## 1. 슬롯

| 키 | skillId | 이름 | 종류 | 쿨다운 | 마나 | 배율 |
|---|---|---|---|---|---|---|
| LMB | 31000 | 평타 | COMBO 4단 | — | 0 | 100 |
| Q | 31200 | 필법 : 먹물세례 | ACTIVE | 8s | 277 | 1788 |
| W | 31430 | 필법 : 흩뿌리기 | ACTIVE | 14s | 382 | 3267 |
| E | 31480 | 묵법 : 두루미나래 | ACTIVE | 24s | 521 | 6981 |
| R | 31210 | 필법 : 콩콩이 | **COMBO 4단** | 16s | 412 | 1301 |
| A | 31460 | 묵법 : 호접몽 | ACTIVE | 16s | 412 | 1139 |
| S | 31420 | 묵법 : 난치기 | ACTIVE | 24s | 521 | 983 |
| D | 31490 | 묵법 : 범가르기 | ACTIVE | 30s | 592 | 6029 |
| F | 31470 | 필법 : 한획긋기 | ACTIVE | 25s | 533 | 6975 |
| T | 31950 | 묵법 : 미르 새김 | ACTIVE | 72s | 903 | 100 |
| V | 31910 | 절기 : 몽유도원 | ACTIVE | 300s | 0 | 100 |
| ALT+V | 31930 | 몽중백화원 | ACTIVE | 300s | 0 | 100 |
| X | 31110 | 떠오르는 해 | ACTIVE | 1s | 0 | — |
| Z | 31050 | 저무는 달 | ACTIVE | 2s | 0 | — |
| SPACE | 31020 | 필법 : 흘리기 | ACTIVE | 7s | 0 | — |

빠진 것은 옹달샘(31230)·올려치기(31510)·해우물(31410)·진경산수(31900)·봉황(31920) 다섯이다.
`X`는 워로드에서 연 슬롯을 그대로 쓴다.

**콩콩이가 트라이포드에서 기준 체인으로 돌아왔다.** 기존 바인딩은 `sdm_sk_skykongkong_custom_8`
하나였는데 이건 seq=7 변형이다. 31210은 seq가 3·6·7 셋뿐이라 최저인 seq=3이 기준이고
`01 → 03 → 01 → 02` 4단 COMBO다. 사용자가 인게임에서 콤보임을 확인해 `skillKind`도 COMBO로 바꿨다.

## 2. 저무는 달과 떠오르는 해는 데미지가 없다

툴팁 매크로로 확인했다.

```text
31050 저무는 달   파티원에게 피해량 증가 버프 (24m)
31110 떠오르는 해 생명력 비율 최저 1명에게 최대체력 회복 (ValueD 2500 = 25%)
```

둘 다 `serverDamageProfileId`를 비우고 `hitTimeMs`/`maximumRange`를 0으로 뒀다.

**조화의 구슬 소모는 아직 모델에 없다.** 저무는 달 2개, 떠오르는 해 1개를 쓰지만 서버에
아이덴티티 자원이 없어 지금은 쿨다운만으로 게이트된다. 사용자 지시로 자원 구현은 뒤로 미루되
애니메이션은 나와야 하므로 바인딩에는 클립을 넣었다.

## 3. 각성기 배율이 원본에서 100이다

31910 몽유도원, 31930 몽중백화원, 31950 미르 새김 모두 툴팁이 지목한 셀의 `ValueA`가 100이고
행이 레벨 1 하나뿐이다. 이미 들어가 있던 31900 진경산수(319001)와 31920 봉황(319200)도 똑같이
100이었으므로 전례를 그대로 따랐다. 6975짜리 한획긋기 옆에서 낮아 보이지만 도화가가 서포터라
원본이 그렇다.

## 4. 타격 시각 대부분이 추정값이다

**도화가는 `.animnotify`에 HIT 노티파이가 전체에서 2개뿐이고 둘 다 `t=0`이다**
(`sdm_sk_fourgentlemen`). 그래서 `.skilltiming`에 행이 있는 넷만 실측이다.

| 실측 | 콩콩이 1445 · 한획긋기 1657 · 범가르기 1654 · 몽중백화원 1426 |
|---|---|
| 추정 | 먹물세례 · 흩뿌리기 · 두루미나래 · 호접몽 · 난치기 · 미르새김 · 몽유도원 |

추정분은 체인 길이의 85% 지점으로 잡았다. 기존 도화가 행들도 같은 상태였고(31230과 31510이
둘 다 1445, 31410과 31470이 둘 다 1657로 관련 스킬 값을 복사한 흔적) receipt에는 전부
`PROJECT_TUNED`의 `animation-reference-hit-timing-v1`로 남겼다.

사거리는 미르 새김 6m와 몽중백화원 10m가 툴팁 명시값이고, `MaxRange`가 0인 나머지는
project override다.

## 5. 걸린 것 — HUD가 COMBO를 통째로 걸렀다

인게임에서 **R을 누르면 콩콩이가 나가는데 좌하단 퀵슬롯에는 안 보였다.**
`CombatHUDViewModel.cpp`의 필터가 원인이다.

```cpp
/* A combo has no cooldown to count down, so it takes no quick-slot tile. */
if (PLAYER_SKILL_KIND::COMBO == definition.eSkillKind)
    continue;
```

이 규칙이 만들어질 때 COMBO는 **LMB 평타뿐**이었다. 평타는 쿨다운 0에 항상 쓸 수 있으니 타일이
필요 없고, 그래서 "콤보 = 타일 없음"이 성립했다. 콩콩이가 쿨다운 16초짜리 퀵슬롯 COMBO가 되면서
가정이 깨졌다.

판정 기준을 kind가 아니라 슬롯으로 바꿨다. 타일이 없어야 하는 이유는 콤보라서가 아니라 평타
슬롯이라서다.

```cpp
if ("LMB" == definition.strInputSlot)
    continue;
```

**입력은 되는데 HUD만 안 보인 이유**도 같다. `CPlayerController`는 `inputSlot`으로 직접 조회하므로
R이 정상 제출됐고, HUD만 별도 필터를 갖고 있었다.

`PLAYER_SKILL_KIND::COMBO` 참조를 전부 훑어 같은 가정이 더 있는지 확인했다. 나머지는 전부
타당하다 — 서버의 콤보 윈도우 연속 입력(`PlayerSkillSystem.cpp:104`), 스테이지별
duration/hitTime(`:228`), 클라이언트의 서버 확정 단계 재생(`Character.cpp:146`)은 콩콩이에도
그대로 맞는다. 쿨다운은 `Try_Start`가 kind와 무관하게 걸어(`:153`) 16초도 정상 동작한다.

## 6. 검증

```text
Engine / Shared / NetworkProtocolHarness / Server / Client   빌드 성공
NetworkProtocolHarness        failures : 0
ClientFrontendHarness         failures : 0
Server.exe --contract-test    failures : 0
Gameplay balance Validate     6 profiles / 123 skills / 76 damage profiles
바인딩                        15개, 클립 24개 전부 모델에 존재, COMBO 클립 수 계약 일치
git diff --check              통과
```

`ProjectAudit`의 `projects.data-source-visibility`는 224 vs 222로 기존 실패가 남아 있다
(`Data/UI/SkillWindow/`의 두 파일). 이번 작업은 `Data` 파일을 새로 만들지 않았다.

**인게임 확인(사용자): 스킬 애니메이션이 정상 재생되고, HUD 수정 후 R 타일도 보인다.**

## 7. 남은 것

- 조화의 구슬(아이덴티티 자원) 미구현. 저무는 달·떠오르는 해가 쿨다운으로만 게이트된다.
- 콩콩이 2단 클립(`sdm_sk_skykongkong_03`)에 선콤 윈도우가 없어 300~700ms는 추정이다.
  1·3단은 `sdm_sk_skykongkong_01`의 실측 309~647ms를 쓴다.
- 루트 모션이 없다. 흘리기(31020)는 이동기인데 제자리에서 재생된다.
- 추정 hitTimeMs 일곱 개는 인게임에서 타격 체감과 맞는지 확인이 남았다.
