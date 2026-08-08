# 공의연무·선풍참혼 COMBO 전환과 스테이지별 루트모션 — 결과

작성자: JS · 2026-08-08 · 브랜치 `feature/lancemaster-skill-timing`

대응 PLAN: `2026-08-08_LOSTARK_LANCEMASTER_COMBO_SKILL_AND_STAGE_ROOT_MOTION_PLAN.md`

PLAN의 G01~G03을 구현하고 실기 확인까지 마쳤다. 확인 과정에서 스킬 네 개가 추가로 나왔고
PLAN과 다른 결정을 세 번 내렸다. 이 문서는 최종 상태와 그 판단 근거를 남긴다.

## 1. 구현 완료

### 1.1 새 계약 세 개

**skillbindings formatVersion 3 — 스테이지별 클립 그룹**

`clips` 원소로 배열을 허용한다. 원소가 배열이면 그 스테이지에서 순서대로 재생할 클립 묶음이다.

```json
"clips": [
  [ { "clip": "flm_sk_chestdestruction_01", "playMs": 551 },
    "flm_sk_chestdestruction_02" ],
  [ { "clip": "flm_sk_chestdestruction_03", "playMs": 100 },
    "flm_sk_chestdestruction_04" ]
]
```

`ANIMATION_SKILL_BINDING::Clips`가 `Stages`로 바뀌었고 `CCharacter::CLIP_CHAIN`도 스테이지를
갖는다. **한 입력에 클립 여러 개가 이어지는 지점은 `Update_Chain`이다** — 스테이지 안에서는
스스로 전진하고 스테이지의 마지막 클립에서 멈춰 서버의 다음 스테이지를 기다린다.

**rootmotion formatVersion 2 — 스테이지별 커브**

`extract_rootmotion.py`가 COMBO/HOLD를 건너뛰던 것을 걷어냈다. 스테이지 스킬은 `stages[]`를
갖고 이동이 없는 스테이지는 생략한다. `Publish-GameplayBalance.ps1`이 각 스테이지를 자기
`comboStages[i].actionDurationMs`와 대조해 `SKILLSTAGEROOTMOTION` 행을 만들며 gameplay
bootstrap이 버전 4가 됐다. `PLAYER_COMBO_STAGE`가 `RootMotion`을 소유하고
`PlayerSkillSystem`이 `hasStage`일 때 그 커브를 읽는다.

전제는 서버가 스테이지 전이에서 `fActionElapsedSeconds`를 0으로 되돌린다는 것이다
(`PlayerSkillSystem.cpp:382`). 그래서 스테이지 커브를 시간 변환 없이 그대로 샘플링한다.

**`PLAYER_SKILL_KIND::COUNTER` — 피격이 전이 조건인 2스테이지 스킬**

```text
COMBO    hasBufferedComboInput  (재입력)
HOLD     시간 경과 / 릴리즈
COUNTER  가드 창 안의 피격
```

`CPlayerSkillSystem::Try_Counter()`가 트리거 전부다. 플레이어에게 들어오는 모든 데미지가
적용 전에 이걸 먼저 묻고, true면 호출자가 데미지를 건너뛴다. 가드가 그 피격을 흡수한 것이다.

```text
MonsterBrain.cpp:137   사거리 안 && !Try_Counter(...)      → 그때만 데미지 적용
ValtanBrain.cpp:357    패턴 히트 범위 안 && Try_Counter(...) → continue
```

`player + catalog`만 읽으므로 static이다. 브레인이 스킬 상태를 소유하지 않게 하는 경계다.
가드 스테이지는 데미지를 내지 않고(`counterWithoutDamage`, HOLD의 같은 패턴), COUNTER는
시간이나 입력으로 스테이지가 넘어가지 않는다(`hasNextStage && !isCounter`).

### 1.2 최종 스킬 값

| ID | 슬롯 | kind | dur | hit | 스테이지 `[dur/hit/open-close]` |
|---|---|---|---|---|---|
| 34100 | E 청룡출수 | ACTIVE | 3034 | 300 | — |
| 34160 | R 공의연무 | COMBO | 2300 | 500 | `[2300/500/1302-2300]` `[1400/250/0-0]` |
| 34140 | A 선풍참혼 | COMBO | 2118 | 450 | `[2118/450/299-2118]` `[2033/400/0-0]` |
| 34110 | D 반월섬 | ACTIVE | 1250 | 200 | — |
| 34540 | Q 나선창 | ACTIVE | 609 | 304 | — |
| 34580 | A 절룡세 | COUNTER | 2000 | 0 | `[2000/0/0-1200]` `[1300/350/0-0]` |

클립 컷은 08-06 RESULT가 확정한 규칙 — **레이블 없고 `win=NONE`이며 `t+d`가 클립 끝에 닿는
CANCEL 윈도우 중 가장 이른 것** — 을 그대로 썼다. 입력 창은 `[선콤]`(COMBO_PRE)이 있으면
그것을, 없으면 `[선스]`(SKILL_PRE)를 쓴다.

`LanceMaster.rootmotion.json`은 v2, 21 스킬 중 6개가 스테이지 커브를 갖는다.

## 2. PLAN과 달라진 결정 세 개

### 2.1 v2 문서 호환 → 형태 명시 (G01)

PLAN은 "v2 문서는 전부 v3에서 그대로 유효하다"고 적었다. **틀렸다.** `Parse_Text`는 skillKind를
모르기 때문에 평탄한 `clips`를 COMBO에서 만나면 "스테이지 N개×클립 1개"인지 "스테이지 1개×클립
N개"인지 판단할 수 없다.

암묵적 추론을 넣는 대신 형태로 못박았다. **평탄 배열 = 스테이지 1개, 중첩 배열 = 스테이지 N개**,
한 바인딩에 두 형태를 섞으면 거부. 그래서 여섯 클래스의 COMBO/HOLD 바인딩 10개(35 스테이지)를
중첩 형태로 옮겼다. 저장 계약을 파서가 추론하지 않게 하는 편이 맞다.

### 2.2 반월섬 — 레퍼런스 해석이 실제와 달랐다

`.clipseq`의 최저 `seq=0`이 기준 체인이라는 규칙으로 읽으면 반월섬은 2클립이다.

```text
seq=0  clips="flm_sk_crescentsweep, flm_sk_crescentsweep_custom_5"
```

`_custom_5`가 여섯 시퀀스 전부에 공통이고, 기준 34110의 유일한 히트가 `1.857-1.955`로
`crescentsweep`(1.5s) 밖에 있어 2클립이어야 한다고 판단했다. **인게임에서는 휘두르는 동작이
두 번 보였고 원작 기준은 1클립이다.** 사용자 확인이 데이터 해석을 이겼다.

`[[lostark-clipseq-seq-is-tripod-variant]]` 규칙이 이 스킬에는 맞지 않는다. 다른 스킬에도 같은
함정이 있을 수 있으므로 `.clipseq`만으로 클립 수를 확정하지 않는다.

### 2.3 절룡세 — 기존 kind 재사용 → COUNTER 신설

COMBO에 얹을 수도 있었지만 COMBO의 정의가 "재입력으로 다음 클립"이고 절룡세는 재입력이 아니다.
kind를 새로 만들어 전이 조건을 타입으로 드러냈다. `hasNextStage && !isCounter` 한 줄로
"시간·입력으로는 절대 안 넘어간다"가 계약이 된다.

## 3. 자동 검증 (실행함)

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug` 전 구간, 매 단계 실행.

```text
빌드 6종 (Engine → UpdateLib → Shared → 하네스 2종 → Server → Client)  성공
NetworkProtocolHarness   failures : 0
ClientFrontendHarness    failures : 0
Server.exe --contract-test  failures : 0
Gameplay balance Validate + Publish  131 skills
ProjectAudit  77 checks
```

새로 추가한 검증:

- ClientFrontendHarness 4개 — 형태 혼합 거부, 빈 스테이지 거부, ACTIVE 다중 스테이지 거부,
  평탄/중첩이 각각 몇 스테이지로 파싱되는지
- contract 3개 — 스테이지 커브가 자기 스테이지 duration 안에 있는지, 유효 stageIndex를 받는지,
  마지막 스테이지를 넘는 행을 거부하는지
- contract 6개 — 가드 승인, 가드 중 무데미지, 창 안 피격 흡수 후 스테이지 2 승격, 두 번 카운터
  거부, 창 닫힌 뒤 피격 거부, 다른 kind에서 카운터 안 됨

루트모션 재굽기는 매번 스크래치에 먼저 뽑아 기존 문서와 대조했다. **ACTIVE 항목이 바이트 단위로
동일하게 재현되는지**가 도구·입력이 커밋된 데이터와 같다는 증거다.

## 4. 수동 검증 (사용자 확인)

Server + Client 세션에서 확인.

```text
공의연무  1번 입력에 riseup_01, 재입력 창에서 다시 눌러야 riseup_02      확인
선풍참혼  재입력 창 확대 후 잘 먹힘, 3번 클립 컷으로 대기 시간 해소       확인
반월섬    1클립                                                        확인
나선창    1클립                                                        확인
절룡세    가드만 하다가 몬스터 피격 시 반격, HP 안 깎임                  확인
```

## 5. 남은 경계

- **`extract_rootmotion.py`가 저장소 밖이다.** `C:\Users\95jus\Desktop\buildScript\`에 있고
  이번에 크게 바뀌었다(COMBO/HOLD skip 제거, 스테이지 커브, formatVersion 2, COUNTER 인식).
  다른 팀원이 재굽기를 하면 v1 문서를 만들어 publisher가 거부한다. `Tools/`로 옮길지 미결.
- **Slayer·GunSlinger는 rootmotion 문서가 없다.** 스크립트를 돌리면 11개·12개 바인딩이 이동을
  갖는데 커밋된 문서가 없어 두 클래스는 스킬 이동이 전혀 없다. 이번 범위 밖.
- **평타 6종에 처음 이동이 생겼다.** 지금까지 0이었으므로 체감이 크다. 창술사 평타는
  스테이지별 전진 0.121 / 0.634 / 0.675 / 0.285이며, 나머지 클래스는 실기 확인이 남았다.
- **절룡세 가드 창 1200ms는 추론이다.** `counterattack_01`의 `입력캔슬 ANY_CANCEL`이 `t=1.2`에서
  열리는 것을 회수 시작으로 읽었다. 원작에 "가드 활성" 노티파이가 없다.
- **반월섬 `hitTimeMs 200`이 원작 1857과 크게 다르다.** 배속 1.2를 반영하면 1548이 맞는 값이나
  1클립(1250ms)으로 줄인 뒤에는 그 안에 들어가지 않는다. 별도 판단 필요.
- 우클릭 홀드 이동은 `feature/player-hold-move`의 `375f1b4`로 분리돼 있다.

## 6. 커밋

```text
b8f51a0  docs: plan the LanceMaster combo skills and per-stage root motion
f93890e  feat(animation): give a skill binding one clip group per combo stage
dca7a14  feat(gameplay): give every combo stage its own root motion curve
52de508  feat(balance): make 공의연무 and 선풍참혼 the combo skills they are
c33640d  fix(balance): widen the 선풍참혼 window and cut 반월섬 to its base clip
748a6c6  feat(gameplay): counter 절룡세 on a hit taken, and settle the LanceMaster clip cuts
```

E 청룡출수는 이 브랜치의 선행 커밋 `64a8c63`이다.
