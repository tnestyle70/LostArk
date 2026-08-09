# 도화가 스킬 타이밍과 콤보 방향 — 결과

작성자: JS · 2026-08-09 · 브랜치 `feature/artist-skill-timing`

사용자가 인게임에서 본 세 가지(평타 막타 지연, 흘리기 2회 재생, 콩콩이 점프 횟수)를 고쳤다.
고치는 과정에서 콤보 방향 버그와 도화가 타격 시각의 실측 근거가 추가로 나왔다.
로스터는 08-07 RESULT가 확정한 그대로이며 이번에는 타이밍과 표현만 다룬다.

## 1. 도화가는 타격을 이펙트에 싣는다

08-07 RESULT는 도화가 `hitTimeMs` 일곱 개를 "HIT 노티파이가 2개뿐이라 추정"으로 남겼다.
그 전제가 반쪽이었다. `kind=HIT`은 전부 `src=ParticleHit`이고, 도화가는 그 이벤트를 거의 쓰지 않는 대신
이펙트 재생 자체가 타격 순간을 들고 있다.

```text
LanceMaster HIT 173   Warlord HIT 34   DimensionMaster HIT 19   Artist HIT 2
```

판별자는 **`d=0.0000`인 EFFECT 노티파이**다. `d>0`은 무기 궤적처럼 지속되는 것이고 `d=0`은 순간 버스트다.
콩콩이가 둘로 정확히 갈린다.

```text
sdm_sk_skykongkong_01  Weapon_01/04/06  t=0.0000 d=0.6333   휘두르는 궤적
                       Skl_01/04/06     t=0.6004 d=0.0000   내려찍는 순간
```

이름으로도 확인된다. 한획긋기는 `Par_V_SDM_OneStroke_hit_01/04/07/47`이 `t=1.4506`에 동시에 터지고
충격 왜곡 `Par_ConvateDisOL_FSM_PushingHit_01`이 `t=1.4520`에 붙는다. `kind=HIT` 행으로 세면 0개인 스킬이다.

이 규칙으로 여덟 개를 실측값으로 교체했다. 전부 기존 값보다 이르다.

| skillId | 슬롯 | 이전 | 이후 | 근거 |
|---|---|---|---|---|
| 31200 먹물세례 | Q | 1077 | 625 | inkpaddle d=0 버스트 |
| 31430 흩뿌리기 | W | 1190 | 562 | inkshot `InkShot_Skl_01` |
| 31480 두루미나래 | E | 1502 | 997 | flyinheaven d=0 버스트 |
| 31460 호접몽 | A | 1700 | 540 | butterflydream 첫 버스트 |
| 31420 난치기 | S | 1275 | 432 | fourgentlemen d=0 버스트 |
| 31490 범가르기 | D | 1654 | 1000 | cloudtiger d=0 버스트 |
| 31470 한획긋기 | F | 1657 | 1451 | onestroke `OneStroke_hit_*` |
| 31910 몽유도원 | V | 5213 | 3000 | pungnyudo 첫 버스트 |

`31470`과 `31490`은 `.skilltiming` 실측이었는데도 바뀌었다. DB 시각이 시전 전체 기준이라 클립 로컬과
다르다는 07-31·08-07의 결론이 여기서도 그대로 나타난다.

**31950 미르 새김은 건드리지 않았다.** `t=0` 버스트뿐이라 시전 이펙트지 타격점이 아니다. 추정 1700이 남는다.

## 2. 평타 막타는 입력 창이 잘못 뽑혀 있었다

`sdm_att_battle_1_01`에는 `[선콤]`(COMBO_PRE)이 없다. 08-06이 정한 규칙은 "COMBO_PRE가 있으면 그것,
없으면 `[선스]`(SKILL_PRE)"인데 저장된 값은 클립 끝의 레이블 없는 `win=NONE` 창이었다. 그건 입력 창이
아니라 클립 컷 마커다.

```text
SKILL_PRE  t=0.1000 d=0.8200  →  100-920      (규칙이 지목하는 값)
win=NONE   t=1.1500 d=0.1833  →  1150-1333    (저장돼 있던 값, 클립 컷 마커)
```

1·2단은 COMBO_PRE로 제대로 뽑혀 있어 3단만 어긋난 상태였다. `100/920`으로 고쳤다.

## 3. 흘리기는 같은 대시의 두 변형이었다

`.clipseq`가 `mode=SEQUENCE clips="sdm_sk_moving_normal_1,sdm_sk_spacing"`으로 2클립이라고 적어놨지만
둘 다 `len=1.5000`에 `SUPERARMOR 0-0.82`, `[선스]/[선이] 0.2-0.82`로 노티파이 구조가 같다. 사운드만
`Moving1` / `Moving2`로 갈린다. 이어지는 2파트가 아니라 변형 두 개다.

`sdm_sk_spacing` 하나로 줄이고 `actionDurationMs`를 3000에서 1500으로 내렸다. 08-08 반월섬과 같은 함정이며
`.clipseq`만으로 클립 수를 확정하지 않는다는 규칙이 다시 확인됐다.

## 4. 콩콩이는 3스테이지다

체인은 `01 → 03 → 01 → 02` 4클립인데 찍는 이펙트를 가진 것은 `01`, `01`, `02` 셋뿐이다.
`_03`은 노티파이가 하나도 없는 연결 동작이라 자기 press를 가질 수 없다. 사용자가 원작에서 3회라고
확인해준 것과 일치한다. v3 중첩 배열로 `_03`을 1찍 뒤에 묶었다.

```json
"clips": [
  [ "sdm_sk_skykongkong_01", "sdm_sk_skykongkong_03" ],
  [ "sdm_sk_skykongkong_01" ],
  [ "sdm_sk_skykongkong_02" ]
]
```

| 스테이지 | dur | hit | 입력 창 |
|---|---|---|---|
| `[01, 03]` | 2934 | 600 | 300-2934 |
| `[01]` | 1867 | 600 | 300-1867 |
| `[02]` | 533 | 533 | — |

`hitTimeMs`가 처음에는 `.skilltiming`의 1445였다. 서버가 `hasAppliedSkillDamage` 이후에만 남은 클립을
잘라내므로(`PlayerSkillSystem.cpp`의 `cancelsIntoNextStage`) 찍는 게 보이고 나서 850ms를 더 기다려야
다음 점프가 나갔다. §1의 규칙으로 600으로 내려 해소했다.

입력 창 `300-2934`는 튜닝값이다. 콩콩이 클립 넷 다 InputTiming 노티파이가 없어 원본 근거가 없다.
버스트보다 이르게 열어 미리 눌러두면 찍는 순간 바로 전이되게 잡았다.

## 5. 콤보 재입력이 조준을 버리고 있었다

`Try_Start`는 액션 중 재입력을 받으면 버퍼 플래그만 세우고 return한다. 조준 방향을 계산해 `fYawDegrees`와
`fSkillAimDirection`에 넣는 코드는 그 아래 신규 시전 경로에만 있었다. 그래서 2단 이후가 1단에서 정한
방향으로만 나갔다.

```text
SERVER_PLAYER          fBufferedComboAimX / Z 추가
Try_Start 버퍼 분기     그 press의 조준 방향을 계산해 저장
Update 스테이지 전이     fSkillAimDirection과 fYawDegrees를 버퍼된 값으로 갱신
```

전이 시점에 적용하는 이유는 클립 중간에 도는 게 아니라 다음 스테이지가 시작할 때 그쪽을 보게 하기
위해서다. 루트 모션도 같은 방향 벡터를 쓰므로 실제 이동까지 따라간다. HOLD는 자기 시계로 넘어가므로
`hasBufferedComboInput`이 false여서 기존 방향을 유지한다.

방향 계산이 두 곳에서 필요해져 `ResolveAimDirection`으로 뽑았다. 신규 시전 경로도 같은 함수를 쓰며
동작은 그대로다.

## 6. 루트 모션 재굽기

`extract_rootmotion.py`를 Blender 5.0 번들 인터프리터로 실행해 도화가 문서를 재생성했다.
스크래치에 먼저 뽑아 커밋본과 대조했고 **31210과 31020만 바뀌고 나머지 12개 ACTIVE 항목이 바이트 단위로
동일하게 재현**됐다. 도구와 입력이 커밋된 데이터와 같다는 증거다.

31210은 스테이지 4개에서 3개로, 31020은 9.04m에서 4.52m로 줄었다.

## 7. 검증 (실행함)

```text
Server 빌드                          성공
NetworkProtocolHarness               failures : 0
Server.exe --contract-test           failures : 0
Gameplay balance Validate / Publish  6 profiles / 130 skills / 108 damage profiles
루트 모션 문서                        샘플 순서·duration 초과 검사 통과
git diff --check                     통과
```

`ClientFrontendHarness`는 `failures : 1`이다. `Effect Runtime Invalid Catalog Preserves Committed Assembly
State`가 롤백 후 남아 있어야 할 id로 `effect.dimensionmaster.skill.2050500`을 하드코딩하는데 PR #73이
publish한 카탈로그에는 `effect.dimensionmaster.skill.2050500.authored-baseline`만 있다.
`ClientFrontendHarness.cpp:3613`의 단언이 데이터를 못 따라간 것이며 이번 변경과 무관하다.

`ProjectAudit`은 4건 실패다. `effect.wfx-component-assembly`,
`effect.representative-authored-readiness`, `effect.four-class-authored-clip-product-exact101`,
`rendering.profile-parser-contract`이며 메시지가 전부 `Python `으로 끝난다. 이 PC의 `python`이
Store 스텁이라 실행되지 않는다. 변경 전 baseline과 동일하고
`gameplay.playable-skill-animation-authoring-contract`는 통과한다.

**인게임 확인(사용자): 평타 막타 즉시 반응, 흘리기 1회, 콩콩이 3회에 마우스 방향 추적까지 정상.**
타격 시각 여덟 개 적용 후 재확인도 완료.

## 8. 남은 것

- **다단 히트 미구현.** 호접몽 540/1150, 몽유도원 3000/3200/3400이 다단인데 `hitTimeMs`가 하나뿐이라
  첫 타만 들어간다. 창술사·차원술사·워로드에도 같은 제약이 있다.
- **31950 미르 새김 `hitTimeMs` 1700은 추정.** `t=0` 버스트뿐이라 근거가 없다.
- **도화가 바인딩에 `playMs`가 없다.** 평타 `_01`(1150)·`_04`(1050), 흘리기(800)는 근거를 찾아뒀고
  아직 걸지 않았다.
- **조화의 구슬 미구현.** 저무는 달·떠오르는 해가 쿨다운으로만 게이트된다.
- `d=0.0000` EFFECT 판별자는 도화가에서만 검증했다. 다른 클래스의 추정 `hitTimeMs`에도 적용 가능해 보이나
  확인하지 않았다.
