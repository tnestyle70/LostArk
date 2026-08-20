# 2026-08-21 발탄 패턴 종합본 — 저작 기준본

## 0. 이 문서가 왜 필요한가

발탄 패턴을 저작하려는데 참고 자료가 네 갈래로 흩어져 있다. 패턴 하나 건드릴 때마다
네 군데를 다시 뒤지면 33개를 도는 동안 기준이 흔들린다. 이 문서는 그 넷을 패턴 단위로
합쳐 놓은 기준본이다. 여기까지가 저작 **입력**이고, 실제 저작은 이 문서를 보고 한다.

자료 넷과 각자 답하는 질문은 이렇다.

```text
A  선배 모작본        .md/GB/08-01/2026-08-01_LOSTARK_REFERENCE_TEAM_PORTFOLIO_VIDEO_ANALYSIS.md
                     같은 폴더 ..._RAW_TRANSCRIPT.md 가 교정 없는 원문
   답하는 질문        보스에 어떤 규칙과 시스템이 있는가
   안 답하는 것       숫자. 반경도 각도도 지속시간도 한 줄도 없다

B  원본 추출본        Data/Animation/Reference/Valtan/*.clipseq .skilltiming
                                                     .animevents .animnotify .clipmap
   답하는 질문        각 동작이 실제로 어떤 모양이고 몇 초인가
   안 답하는 것       언제 그 동작을 쓰는가. 선택 규칙은 원본에 없다

C  네 67개 관찰       Data/Animation/Authored/Valtan/Valtan.patternpreview.json
                     Data/Encounters/Valtan/ValtanDebugAudition.json
   답하는 질문        실제 판에서 어떤 순서로 몇 줄에 나왔는가
   안 답하는 것       판정 수치. 영상 관찰이라 반경을 잴 수 없다

D  현재 저작본        Data/Encounters/Valtan/ValtanEncounter.json
                     Data/Animation/Authored/Valtan/Valtan.patternbindings.json
   상태               33 패턴 / 129 스테이지. 규칙은 대체로 A·C 에서 왔고
                      판정 도형은 대부분 근거 없이 지어졌다
```

A 와 B 는 겹치지 않는다. **A 가 뼈대, B 가 살이다.** 그래서 "둘 중 뭘 따르나"가 아니라
"어느 쪽에서 뭘 가져오나"가 맞는 질문이고, 답은 아래 세 칸 모델이다.

```text
규칙 칸      언제 나오는가       근거 A + C     armorRequirement phaseRequirement
                                              triggerHealthBar selectionWeight
                                              minimum/maximumHealthBar
모양·시간 칸  몇 미터 몇 초인가    근거 B         hitShape hitLength hitAngleDegrees
                                              hitOuterRadius durationMs
                                              pushRangeM pushMs
동작 칸      어떤 clip 이 나오는가 근거 B         Valtan.patternbindings.json
```

A 와 B 와 C 가 서로 어긋나면 지어내지 않는다. 양쪽 근거를 나란히 적고 사용자가 고른다.
선례가 있다. 전멸기 이름은 `VALTAN_FLOOR_WIPE_130` 인데 `triggerHealthBar` 가 115 였고,
선배 문장은 130 이었다. `2026-08-20_VALTAN_ARMOR_SERVER_STATE_RESULT.md` 4.10.2 에
`사용자 결정에 따라 130으로 옮겼다` 로 남아 있다. 그 방식을 계속 쓴다.

---

## 1. 자료 A — 선배 모작본이 말한 발탄 규칙 전문

`..._VIDEO_ANALYSIS.md` 의 발탄 관련 문장 전부다. 줄 번호는 그 파일 기준이고 인용은 원문이다.
오른쪽은 지금 저장소에서 실측한 반영 상태다.

```text
 97  특수 패턴인 무력화 패턴은 보스의 본체 하단에 특수 무력화 게이지를
     시각적으로 나타내고 무력화 게이지가 소진되지 않는 한 행동이 중단되지 않는다
     → 미구현. VALTAN_MAGIC_ORB_STAGGER_76 에 STAGGER_WINDOW 스테이지 8000ms 는
       있지만 게이지 개념이 Server/Shared/Data 어디에도 없다

 99  부채꼴 모양의 스킬은 절두체 콜라이더를 구 콜라이더의 자식으로 넣어 구현했습니다
     → 우리는 다르게 갔다. 판정은 Engine/PhysX 비의존 Shared XZ primitive 를
       Server fixed tick 에서 평가한다. 구현 방식 차이일 뿐 결과는 같은 부채꼴이다

101  보스의 무력화 행동에 변수를 추가하여 공격이 중단되었을 때 공격을 다시
     반복하지 않고, 다음 행동으로 넘어가도록 조정했습니다
     → 미구현. 97 과 한 묶음이다

103  보스가 사망시 클리어 UI가 출력되며 잠시 후 마을 레벨의 서버 패킷을 보내
     레벨을 이동합니다
     → 이 문서 범위 밖. 패턴 저작이 아니다

111  보스 발탄은 다양한 파트 오브젝트를 가지고 있으며, 페이즈 변수와 갑옷
     변수로 어떠한 파츠 오브젝트 모델을 사용할 지 결정한다
     → 절반. 갑옷 변수는 됐다. iBrokenArmorMask 가 복제되고 CValtan 이 파츠를
       숨긴다. 페이즈 변수로 모델을 바꾸는 경로는 없다

112  1페이즈에서 발탄은 2개의 갑옷과 함께 돌진 패턴과 일반 패턴들을 반복한다
     → 반영. armorPlates 2장, VALTAN_DASH_CHARGE 가 ARMORED + PHASE_ONE

114  발탄의 어깨와 팔에 갑옷은 발탄이 피격시 패킷을 받을 때 받는 피해량을
     감소시키며 갑옷들은 돌진 패턴을 통해 제거할 수 있습니다
     → 반영. 판당 defense 50, ApplyPlayerHitDamage 가 성한 판의 합으로 감쇄

116  돌진 전에 3번 바닥을 치며 플레이어를 바라보고, 마지막으로 바라본 방향으로
     달려나갑니다
     → 절반. WINDUP 600ms 가 그 자리인데 durationMs 가 원본 clip 길이에서
       오지 않았다. 이 문서 6장 VALTAN_DASH_CHARGE 카드에서 대조한다

118  돌진 중에 발탄의 공격 콜라이더와 맵의 오브젝트 콜라이더가 부딪히게 되면
     돌진을 머추고 기절 상태에 빠지게 된다
     → 반영. ValtanChargeImpactActions.json 이 어느 스테이지가 돌진인지 소유하고
       receiver 접촉만으로 GROGGY 로 넘어간다

120  발탄은 기절 시 피격 함수 내에서 갑옷 내구도를 감소 시키고, 내구도가 0이하가
     되면 부위 파괴에 따른 행동을 하며 갑옷 버프를 감소 시키고 갑옷이 사라집니다
     → 반영. bPatternGroggy 동안만 내구도가 깎이고, 0 이 되면 PART_BREAK
       스테이지로 넘어간다. 한 창은 판을 최대 하나만 부순다

122  보스의 공격 콜라이더는 일반 몬스터와 다르게 개별적인 스킬 객체를 통해
     생성, 관리합니다
     → 우리는 다르게 갔다. 스테이지가 판정을 소유한다. 구조 차이다

124  하늘에서 도끼를 떨어뜨리는 패턴에서는 플레이어의 조작여부와 상관없이
     위치를 추적하는 스킬 객체를 생성하여 동기화했습니다
     → 미구현. VALTAN_HIGH_JUMP 는 있지만 추적 오브젝트가 없다

126  일정 시간이 지나거나 보스의 공격을 카운터 성공하거나 또는 보스의 갑옷
     파괴로 에스더 게이지를 채울 수 있으며 게이지를 소모해 공대장만이
     에스더 스킬을 사용할 수 있습니다
     → 3분의 1. CEstherSkillSystem 이 있고 게이지 1000, 초당 200 시간 충전만 한다.
       카운터 성공과 갑옷 파괴 충전 경로가 없다. 공대장 제한은 코드 주석이
       의도적으로 뺐다고 적어 두었다

128  발탄 레이드에서 에스더 스킬은 총 3가지이며 실리안, 바훈투르, 웨이의
     에스터 스킬 중 하나를 선택하여 사용할 수 있다
     → 3분의 1. Try_Consume 이 슬롯 1(실리안 NPC_59030)만 받고 나머지 둘은
       에셋 미쿠킹으로 UNSUPPORTED_SLOT 거부

132  실리안의 에스더 스킬은 강력한 데미지를 주어, 보스의 체력을 대량으로
     감소 시켜 보스를 마무리할 때 주로 사용한다
     → 소환은 된다. 피해 수치가 balance 정본에 있는지는 별도 확인 대상

138  발탄의 갑옷 여부나 보스 페이즈 변수에 따라 추후에 나올 패턴이 바뀔 수
     있도록 설계하여 반복된 패턴에 대한 지루함을 해소했습니다
     → 반영. armorRequirement / phaseRequirement 두 필드가 그것이다.
       다만 실제로 게이트가 걸린 패턴은 VALTAN_DASH_CHARGE 하나뿐이고
       나머지 32개는 전부 ANY 다. 즉 그릇만 있고 내용이 거의 비었다

140  발탄의 체력이 130줄 이하가 되면 발탄이 무적상태가 되고, 플레이어 모두를
     죽이는 스킬을 사용하는 전멸 패턴이 등장한다
     → 반영. VALTAN_FLOOR_WIPE_130 이 trigger 130, invulnerableWhileRunning true

142  전멸 패턴은 바훈투르의 에스더 스킬을 사용하여 공략할 수 있다. 해당 스킬은
     20초 동안 일정 데미지 및 경직을 무시하는 버프 효과를 부여한다
     → 미구현. 바훈투르 슬롯 자체가 거부 상태라 전멸기에 공략 수단이 없다.
       지금 130줄 전멸기는 맞으면 그냥 죽는다
```

**여기서 정정할 것이 하나 있다.** `2026-08-20_VALTAN_ARMOR_SERVER_STATE_RESULT.md` 479줄이
선배 설명으로 `체력이 109줄 이하가 되면 컷씬이 나오면서 2페이즈로 넘어갑니다` 를 인용해
`phaseTwoHpPercent` 를 50 에서 68 로 바꿨다. 그런데 **보관된 선배 원문 두 문서에 109 는
없다.** 130 만 있다. 다른 자리에서 들은 것을 옮겨 적었을 수 있지만, 지금 저장소 기준으로
109 페이즈 경계를 받치는 근거는 선배 문장이 아니라 다음 둘이다.

```text
encounter 저작   VALTAN_ARENA_BREAK_109 의 triggerHealthBar 가 109
네 67개 관찰      21번 `109줄 점프 착지와 추가 외벽 붕괴`
                 24번 대쉬 돌진이 109줄이고 그 뒤로 돌진이 한 번도 없다
```

이 둘이면 109 자체는 충분히 받쳐진다. 다만 **선배 근거로 기록된 것은 정정한다.**

---

## 2. 자료 B — 원본 추출본 읽는 법

### 2.1 파일별 역할

```text
.clipseq      동작(action) 이 어떤 clip 을 어떤 순서로 잇는지. mode 는 SEQUENCE / HOLD / COMBO
.clipmap      clip 이 어느 원본 스킬 소속인지
.animnotify   clip 길이와 그 안의 타격·저지·단계전환·이펙트 시점
.animevents   타격 창의 시작 ms 와 끝 ms
.skilltiming  재사용 / 사거리 / 접근 / 회전과 판정 도형 전체
```

### 2.2 skilltiming 필드 사전

`shape` 행의 필드는 원본 SkillEffect 값 그대로다. 단위는 cm 와 ms 이고 encounter 는 m 를
저장하므로 100 으로 나눈다.

```text
area    판정 종류.  0 판정없음 / 1 원·링 / 2 전방박스 / 3 부채꼴
ar      area=1 반경, area=2 길이, area=3 반경   (cm)
aa      area=2 폭, area=3 각도                  (cm 또는 도)
arem    내반경. 값이 있으면 링이다               (cm)
ax      시전자 전방 오프셋                      (cm)
maxt    최대 대상 수
push    밀치기 지속                            (ms)
pushr   밀치기 거리. 음수면 끌어당김            (cm)
fz      경직 지속                              (ms)
counter 저지 가능 표시
hittype 적용 방식 코드. 아래에서 따로 다룬다
pks     이 도형이 소유한 SkillEffect PK 목록
```

`hittype` 의 원본 의미는 확인되지 않았다. 대신 shape 행 474개를 전수로 세어
코드별 필드 서명을 측정했다. **이 서명이 사실이고, 코드 이름의 뜻은 추정이다.**

```text
ht=1  222행   fz 가 전부 정확히 325 (fzin 125 / fzout 200) 고정
              push 0%  tmpl_t 0%  fzcancel 0%   area=0 이 108행
              → 기본 템플릿 행. 저작된 값이 아니라 기본값이 채워진 자리로 보인다

ht=4   92행   fz 가 저작값 (400 45행 / 500 24행 / 300 20행)
              push 97%  pushr 97%  hitset 76%  fzcancel 100%  tmpl_t 0%
              → 즉시 타격. 밀치기와 경직을 그 자리에서 준다

ht=5  144행   fz 0%  tmpl_t 100%  fzcancel 100%  pushr 35%
              → 예고 장판. tmpl_t 가 장판이 떠 있는 시간으로 보인다

ht=6   16행   fz 0%  tmpl_t 100%  hitset 94%
              → ht=5 와 서명이 같은데 따로 있다. 420604 대쉬 돌진의 몸통 박스와
                420619 초강력 내려찍기에만 나온다. 이동하는 판정으로 추정한다
```

**그래서 `hittype` 으로 진짜 도형을 가르면 안 된다.** 420619·420623·420638 은 ht=4 가
하나도 없고 ht=5 에 도형이 다 들어 있다. 휠윈드 420633 은 shape 가 셋뿐인데 셋 다 ht=1
이고 그게 그 동작의 유일한 도형이다.

이 문서가 쓰는 판별 기준은 **크기**다.

```text
원본이 크기를 지정한 도형   area > 0 이고 0 < ar < 10000
아레나 전역 표식            ar = 10000 (100m). 도형 근거로 쓰지 않는다
```

`aa` 가 360 이면 부채꼴이 아니라 링이다. publisher 가 `CONE` 을 180도 이하로만 받으므로
(`Publish-GameplayBalance.ps1:1043`) 360도 부채꼴은 `RING` 으로, 180도를 넘는 부채꼴은
`CIRCLE` 이나 `RING` 으로 담고 반경은 원본 `ar`/`arem` 을 그대로 쓴다.

### 2.3 이번에 찾은 조인 — pks 가 clip 순간과 도형을 잇는다

`.animnotify` 의 `HIT` 행은 `asset="42060101"` 처럼 SkillEffect PK 를 들고 있고, 그 PK 가
`.skilltiming` 의 `pks=` 목록에 정확히 하나 들어 있다. 즉 **몇 밀리초에 어떤 도형이
터지는지가 원본에 그대로 적혀 있다.** 추측할 필요가 없다.

```text
pks 색인 크기         1089   중복 키 0
HIT notify 총 408 건  399 건 조인 성공 (97.8%)
미조인 7종            4004220 4004230 4004231 4004260 4004413 49000010 49000011
                     전부 400xxx 비레이드 세트이거나 그로기·부위파괴 범용 표식이라
                     skilltiming 에 도형 행 자체가 없다. 파서 구멍이 아니다
```

예를 들어 휘두르기는 이렇게 읽힌다.

```text
mesh_att_battle_1_01  3.00s
  1570~1770ms  HIT 42060101  →  skilltiming 420601 의 g=0 key=1 행
                                area=3 ar=400 aa=140 push=242 pushr=200 fz=400
                                = 부채꼴 반경 4.0m 각 140도, 밀치기 2.00m/242ms, 경직 400ms
  1900ms       단계전환
```

그래서 이 패턴의 WINDUP 은 0~1570ms, ACTIVE 는 1570~1770ms, 그 뒤가 다음 단계다.
지금 저작된 450 / 700 / 750ms 는 이 어디에서도 나오지 않는다.

### 2.4 단계전환 조건이 원본에 적혀 있다

`.animnotify` 의 `STAGE` 행 75개는 그 clip 이 끝날 때 **어떤 조건으로** 다음 단계로 가는지를
`src` 에 적어 둔다. 우리 encounter 는 지금 시계(durationMs)로만 넘어가는데, 원본은 조건부다.

```text
MonsterMoveNextStage                          55  무조건 넘어간다
MonsterMoveNextStageConditionStatusEffect      8  상태이상이 걸려 있어야
MonsterMoveNextStageConditionSkillEffectHit    5  그 스킬이 무언가를 맞혔어야
MonsterMoveNextStageConditionProbability       4  확률
MonsterMoveNextStageConditionChangeTarget      3  대상이 바뀌었어야
```

세 번째가 중요하다. 대쉬 돌진 `mesh_att_battle_4_01` 의 4100ms 전환이
`ConditionSkillEffectHit` 이다. **원본이 "돌진이 뭔가를 맞혔을 때만 다음 단계로 간다" 를
직접 적어 놓은 것**이고, 선배 문장 118 `돌진 중에 공격 콜라이더가 맵 오브젝트 콜라이더와
부딪히면 돌진을 멈추고 기절` 과 같은 말이다. 자료 A 와 B 가 독립적으로 같은 규칙을 가리킨다.

`COUNTER` 행 8개는 저지 가능 구간이다. 대쉬 돌진은 1900~3400ms 가 저지 창이고, 이것이
선배 문장 126 의 `보스의 공격을 카운터 성공하거나` 에 대응한다.

### 2.5 seq 는 고르지 않는다

`.clipseq` 는 한 action 에 seq 변형을 여러 개 담는데 발탄에서 seq 가 무슨 뜻인지
확인되지 않았다. 이 저장소가 seq 에 부여한 유일한 문서화된 의미는 플레이어 쪽
트라이포드 계약(`Tools/CharacterAnimationIntake/fill_projectiles.py:37`)이고 보스에
그대로 옮겨간다는 근거가 없다. 그래서 아래 카드는 **가장 낮은 seq** 를 싣고 변형 개수와
내용 동일 여부만 표시한다. 다른 seq 를 써야 하면 영상을 보고 사용자가 지정한다.

---

## 3. 체력줄 지도

`maximumHp 60000`, `maximumHealthBars 160` 이므로 한 줄은 375 HP 이고
`health bar = ceil(hp * 160 / 60000)` 이다.

```text
160줄  전투 시작            VALTAN_ENTRANCE_WHIRLWIND 가 introPatternId
159줄  VALTAN_ARMOR_BREAK_OPENING   오프닝 외벽 충돌 부위 파괴
130줄  VALTAN_FLOOR_WIPE_130        6방향 후 전방향 전멸  ← 무적 구간
109줄  VALTAN_ARENA_BREAK_109       아레나 붕괴
 ↑     phaseTwoHpPercent 68 → hp<=40800 → 이 줄 안쪽에서 iPhase 가 2 로 뒤집힌다
100줄  VALTAN_FOUR_PILLARS_105      중앙 착지와 4기둥 추적 원뿔
 84줄  VALTAN_ARENA_BREAK_84        외곽 바닥 붕괴
 73줄  VALTAN_MAGIC_ORB_STAGGER_76  남북·동서 마력구 폭발
 62줄  VALTAN_CENTER_GRAB_COUNTER_64 중앙 표적 폭발과 잡기 카운터
 30줄  VALTAN_ARENA_BREAK_33        두 번째 지형 절반 파괴
 14줄  VALTAN_GHOST_TRANSITION_15   포탈·최종 지면파괴와 망령화
```

네 67개 관찰이 적어 둔 체력 줄은 이렇다. 대본 기믹과 대조하는 데 쓴다.

```text
관찰  1  160줄   19  130줄   21  109줄   25  100줄   28  95줄(마커)
     32   84줄   37   73줄   41   62줄(마커)  48  30줄  49  29줄
     51   28줄   55   14줄   56   40줄   57  37줄  58  34줄
     59   31줄   60   30줄   64   19줄
```

56번 이후가 줄 수로 되돌아가는 것은 55번이 `14줄 포탈 진입부터 망령 전환 전체` 를 한 자리에
묶어 적었기 때문이다. 즉 55번은 관찰 순서상의 자리이지 그 시점의 체력이 아니다.

`iPhase` 는 지금 `phaseRequirement` 만 읽는다. 95줄 `2.5페이즈` 와 62줄 `3페이즈` 는 네
관찰에 마커로만 있고 encounter 에 페이즈 단계가 없다. 세 단계 페이즈가 필요하면 그건
`phaseRequirement` 값 집합을 늘리는 별도 계약이다.

---

## 4. 자료 D — 현재 저작 상태

```text
패턴        33개    대본 기믹 9 + intro 1 + 일반 반복 23
스테이지    129개
PATTERN 행  17 필드  (마지막 셋이 armorRequirement / phaseRequirement /
                     invulnerableWhileRunning)
receipt     패턴당 정확히 17 entry. 17번째 patterns[i].stages 가 스테이지 배열
            전체를 한 값으로 담는다. 스테이지 숫자 하나만 바꿔도 이 entry 를 통째로 간다
```

세 칸 기준으로 실측한 현재 상태다.

```text
규칙 칸       대체로 채워졌다. 다만 armor/phase 게이트가 걸린 패턴이
              VALTAN_DASH_CHARGE 하나뿐이라 선배 문장 138 의 의도가 거의 비어 있다
모양·시간 칸   비었다. 33개 중 현재 판정 도형이 원본 도형 목록에 있는 패턴이 0개다.
              밀치기만 원본에서 온 자리가 있다. 휘두르기 242ms/2.00m 와
              반격 535ms/4.90m 는 원본 값과 정확히 같다
동작 칸       clip 바인딩은 있는데 스테이지 durationMs 가 clip 길이와 무관하다.
              23개는 clip 보다 짧아 애니메이션이 잘리고 7개는 시간이 남는다.
              휘두르기는 3.00s 짜리 clip 을 WINDUP 450ms + ACTIVE 700ms 로 잘랐다
```

아래 5장과 6장이 패턴별 카드다. 카드는 위 네 자료를 그대로 합쳐 생성했고 지어낸 값이 없다.
---

## 5. 체력줄 대본 기믹 9개

### VALTAN_ARMOR_BREAK_OPENING — 오프닝 외벽 충돌 부위 파괴

**규칙 칸 — 언제 나오는가**

```text
등장     체력 159줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420627 420628 420654 420655
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WALL_CHARGE    WINDUP      1500ms  판정없음
GROGGY         GROGGY      5000ms  판정없음
RECOVERY       RECOVERY    1200ms  판정없음
PART_BREAK     PART_BREAK  1400ms  판정없음
                           9100ms  (합계)
```

원본이 말하는 것:

```text
420627 레이드 발탄_1번째 부위 파괴
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 HOLD  mesh_dmg_parts_start_1 -> mesh_dmg_parts_loop_1 -> mesh_dmg_parts_end_1 -> mesh_idle_battle_1   (seq 3종, 내용 동일)
     mesh_dmg_parts_start_1       1.50s
             0ms          전역표식 9개
          1400ms          단계전환 MonsterMoveNextStage
     mesh_dmg_parts_loop_1        1.33s
     mesh_dmg_parts_end_1         3.00s
          2850ms          전역표식 1개
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

420628 레이드 발탄_2번째 부위 파괴
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=2 HOLD  mesh_dmg_parts_start_1 -> mesh_dmg_parts_loop_1 -> mesh_dmg_parts_end_1 -> mesh_idle_battle_1   (seq 3종, 내용 동일)
     mesh_dmg_parts_start_1       1.50s
             0ms          전역표식 9개
          1400ms          단계전환 MonsterMoveNextStage
     mesh_dmg_parts_loop_1        1.33s
     mesh_dmg_parts_end_1         3.00s
          2850ms          전역표식 1개
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

420654 레이드 발탄_1페이즈 돌진 외벽 파괴 그로기
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_abn_groggy_1_start -> mesh_dmg_parts_start_1 -> mesh_dmg_parts_end_1 -> mesh_abn_groggy_1_end   (seq 3종, 내용 다름)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_dmg_parts_start_1       1.50s
             0ms          전역표식 9개
          1400ms          단계전환 MonsterMoveNextStage
     mesh_dmg_parts_end_1         3.00s
          2850ms          전역표식 1개
     mesh_abn_groggy_1_end        2.00s
             0ms          전역표식 1개

420655 레이드 발탄_오프닝 공격
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=2 SEQUENCE  mesh_att_battle_12_06 -> mesh_att_battle_18_03-1   (seq 3종, 내용 동일)
     mesh_att_battle_12_06        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_18_03-1      1.67s
             0ms          전역표식 1개
           400ms          전역표식 1개
           700ms~900   ms 타격 42062202  ht=5 원 반경 1.8m
           800ms~1000  ms 타격 42062203  ht=5 원 반경 2.8m  전방오프셋 2.90m
           800ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WALL_CHARGE    valtan.mechanic.armor-break-opening.charge     mesh_dmg_parts_start_1       1.50s
GROGGY         valtan.mechanic.armor-break-opening.groggy     mesh_dmg_parts_loop_1        1.33s
RECOVERY       valtan.mechanic.armor-break-opening.recovery   mesh_dmg_parts_end_1         3.00s
PART_BREAK     valtan.mechanic.armor-break-opening.part-break mesh_dmg_parts_end_1         3.00s
```

---

### VALTAN_FLOOR_WIPE_130 — 115줄 6방향 충격파 후 전방향 충격파

**규칙 칸 — 언제 나오는가**

```text
등장     체력 130줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=true
사거리   0.0~100.0m
원본동작 420630
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
19번  PRODUCT_DIRECT     hb=130  x1  115줄 6갈래와 원형 충격파
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1800ms  판정없음
FIRST_SMASH    ACTIVE       800ms  6방향 길이 14.0m 폭 4.4m               x1/0ms  밀치기 2.0m/242ms  넘어짐 2000ms  피해율 450
INTERVAL       WINDUP      2000ms  판정없음
SECOND_SMASH   ACTIVE       500ms  원 반경 100.0m                       x1/0ms  밀치기 2.0m/242ms  넘어짐 2000ms  피해율 100000
RECOVERY       RECOVERY    1500ms  판정없음
                           6600ms  (합계)
```

원본이 말하는 것:

```text
420630 레이드 발탄_1 페이즈 기믹 광역 즉사기
   재사용 50000ms  사거리 3.5m  접근 3.0m  회전 0도
   동작   seq=2 HOLD  mesh_att_battle_1_01 -> mesh_att_battle_1_02 -> mesh_att_battle_5_02_loop -> mesh_att_battle_5_02_end -> mesh_att_battle_5_04 -> mesh_att_battle_15_02 -> mesh_att_battle_15_05 -> mesh_att_battle_15_03 -> mesh_att_battle_15_04   (seq 3종, 내용 동일)
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_1_02         2.50s
          1000ms~2000  ms 저지가능 구간
          1800ms~2000  ms 타격 42060102  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1800ms~2400  ms 타격 42060103  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 4.25m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060104  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 -1.25m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060105  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 1.50m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060106  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 1.50m  밀치기 1.10m/0ms
          2400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_02_loop    0.83s
             0ms          전역표식 2개
          3000ms          전역표식 1개
          3900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_02_end     1.50s
           534ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_04         0.50s
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_02        1.00s
           900ms~1400  ms 저지가능 구간
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_05        1.43s
     mesh_att_battle_15_03        0.57s
           262ms          전역표식 4개
           450ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_04        1.00s
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.mechanic.floor-wipe-130.windup          mesh_att_battle_1_01         3.00s
FIRST_SMASH    valtan.mechanic.floor-wipe-130.first-smash     mesh_att_battle_1_02         2.50s
INTERVAL       valtan.mechanic.floor-wipe-130.interval        mesh_att_battle_5_02_loop    0.83s
SECOND_SMASH   valtan.mechanic.floor-wipe-130.second-smash    mesh_att_battle_5_02_end     1.50s
RECOVERY       valtan.mechanic.floor-wipe-130.recovery        mesh_att_battle_15_04        1.00s
```

---

### VALTAN_ARENA_BREAK_109 — 109줄 아레나 붕괴

**규칙 칸 — 언제 나오는가**

```text
등장     체력 109줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420629
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
21번  PRODUCT_DIRECT     hb=109  x1  109줄 점프 착지와 추가 외벽 붕괴
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
TAKEOFF        WINDUP       900ms  판정없음
DROP           WINDUP       700ms  판정없음
IMPACT         ACTIVE       400ms  원 반경 12.0m                        x1/0ms  넘어짐 1000ms  피해율 700
IMPACT_HOLD    ACTIVE      1100ms  판정없음
WIDE_REVEAL    ACTIVE      2300ms  판정없음
RECOVERY       RECOVERY     870ms  판정없음
                           6270ms  (합계)
```

원본이 말하는 것:

```text
420629 레이드 발탄_모든 지형물 파괴 연출
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 COMBO  mesh_att_battle_12_01 -> mesh_att_battle_12_02 -> mesh_att_battle_12_03   (seq 3종, 내용 동일)
     mesh_att_battle_12_01        1.20s
             0ms          전역표식 8개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_02        1.00s
             0ms          전역표식 1개
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_03        1.20s
           200ms          전역표식 2개
           230ms~430   ms 타격 42061901  ht=5 원 반경 3.0m
           300ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
TAKEOFF        valtan.mechanic.arena-break-109.takeoff        mesh_att_battle_12_01        1.20s
DROP           valtan.mechanic.arena-break-109.drop           mesh_att_battle_12_02        1.00s
IMPACT         valtan.mechanic.arena-break-109.impact         mesh_att_battle_12_03        1.20s
IMPACT_HOLD    valtan.mechanic.arena-break-109.impact-hold    mesh_att_battle_12_03        1.20s
WIDE_REVEAL    valtan.mechanic.arena-break-109.wide-reveal    mesh_att_battle_12_03        1.20s
RECOVERY       valtan.mechanic.arena-break-109.recovery       mesh_att_battle_12_03        1.20s
```

---

### VALTAN_FOUR_PILLARS_105 — 100줄 중앙 착지와 4기둥 추적 원뿔

**규칙 칸 — 언제 나오는가**

```text
등장     체력 100줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420610
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
25번  PRODUCT_CANDIDATE  hb=100  x1  100줄 중앙 착지와 2페이즈 전환
33번  PRODUCT_PARTIAL    hb=0    x1  비석 4개와 기 방출 넉백
49번  PRODUCT_PARTIAL    hb=29   x1  29줄 비석 4개와 기 방출
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
TAKEOFF        WINDUP      1600ms  판정없음
YELLOW_ZONE    ACTIVE       900ms  링 4.0~100.0m                      x1/0ms  밀치기 1.0m/150ms  넘어짐 1000ms  피해율 450
TARGET_CONE    ACTIVE       900ms  부채꼴 반경 30.0m 각 45도                x1/0ms  밀치기 1.0m/150ms  넘어짐 1000ms  피해율 450
RECOVERY       RECOVERY    1300ms  판정없음
                           4700ms  (합계)
```

원본이 말하는 것:

```text
420610 레이드 발탄_고공 점프 찍기
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=0 HOLD  mesh_idle_battle_1 -> mesh_att_battle_8_01_start -> mesh_att_battle_8_01_loop -> mesh_att_battle_8_01_end -> mesh_att_battle_8_01_loop -> mesh_att_battle_8_01_start -> mesh_att_battle_8_01_loop x2 -> mesh_att_battle_8_01_end x2 -> mesh_att_battle_8_01_loop x6 -> mesh_att_battle_8_01_end -> mesh_idle_battle_1 -> mesh_att_battle_8_01_start   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_8_01_start   1.93s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1750ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_loop    5.50s
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_end     3.20s
           240ms~440   ms 타격 42061008  ht=6 원 반경 3.5m
           250ms~450   ms 타격 42061031  ht=1 링 3.5~15.0m (360도)  경직 325ms  maxt=8
           250ms          전역표식 1개
          2000ms~2200  ms 타격 42061009  ht=5 링 3.5~15.0m (360도)  밀치기 1.00m/0ms
     mesh_att_battle_8_01_loop    5.50s
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_start   1.93s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1750ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_loop    5.50s   x2 반복
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_end     3.20s   x2 반복
           240ms~440   ms 타격 42061008  ht=6 원 반경 3.5m
           250ms~450   ms 타격 42061031  ht=1 링 3.5~15.0m (360도)  경직 325ms  maxt=8
           250ms          전역표식 1개
          2000ms~2200  ms 타격 42061009  ht=5 링 3.5~15.0m (360도)  밀치기 1.00m/0ms
     mesh_att_battle_8_01_loop    5.50s   x6 반복
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_end     3.20s
           240ms~440   ms 타격 42061008  ht=6 원 반경 3.5m
           250ms~450   ms 타격 42061031  ht=1 링 3.5~15.0m (360도)  경직 325ms  maxt=8
           250ms          전역표식 1개
          2000ms~2200  ms 타격 42061009  ht=5 링 3.5~15.0m (360도)  밀치기 1.00m/0ms
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_8_01_start   1.93s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1750ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
TAKEOFF        valtan.mechanic.four-pillars-105.takeoff       mesh_att_battle_8_01_start   1.93s
YELLOW_ZONE    valtan.mechanic.four-pillars-105.yellow-zone   mesh_att_battle_8_01_loop    5.50s
TARGET_CONE    valtan.mechanic.four-pillars-105.target-cone   mesh_att_battle_8_01_end     3.20s
RECOVERY       valtan.mechanic.four-pillars-105.recovery      mesh_att_battle_8_01_loop    5.50s
```

---

### VALTAN_ARENA_BREAK_84 — 84줄 외곽 바닥 붕괴

**규칙 칸 — 언제 나오는가**

```text
등장     체력 84줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420629
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
32번  PRODUCT_CANDIDATE  hb=84   x1  84줄 아레나 외곽 절반 붕괴
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       900ms  판정없음
IMPACT         ACTIVE       500ms  판정없음
RECOVERY       RECOVERY    1200ms  판정없음
                           2600ms  (합계)
```

원본이 말하는 것:

```text
420629 레이드 발탄_모든 지형물 파괴 연출
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 COMBO  mesh_att_battle_12_01 -> mesh_att_battle_12_02 -> mesh_att_battle_12_03   (seq 3종, 내용 동일)
     mesh_att_battle_12_01        1.20s
             0ms          전역표식 8개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_02        1.00s
             0ms          전역표식 1개
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_03        1.20s
           200ms          전역표식 2개
           230ms~430   ms 타격 42061901  ht=5 원 반경 3.0m
           300ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.mechanic.arena-floor-84.windup          (바인딩 없음)
IMPACT         valtan.mechanic.arena-floor-84.impact          (바인딩 없음)
RECOVERY       valtan.mechanic.arena-floor-84.recovery        (바인딩 없음)
```

---

### VALTAN_MAGIC_ORB_STAGGER_76 — 73줄 남북·동서 마력구 폭발

**규칙 칸 — 언제 나오는가**

```text
등장     체력 73줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420617 420618
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
37번  PRODUCT_DIRECT     hb=73   x1  73줄 양발과 남북동서 마력구 폭발
42번  PRODUCT_PARTIAL    hb=0    x1  손을 뻗어 마력구 확대와 폭발
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
SHIELD         WINDUP      1500ms  판정없음
STAGGER_WINDOW ACTIVE      8000ms  판정없음
RECOVERY       RECOVERY    1000ms  판정없음
                          10500ms  (합계)
```

원본이 말하는 것:

```text
420617 레이드 발탄_마력구 폭발
   재사용 50000ms  사거리 5.0m  접근 2.0m  회전 180도
   동작   seq=1 HOLD  mesh_att_battle_17_start -> mesh_att_battle_17_loop -> mesh_att_battle_17_end -> mesh_att_battle_17_loop -> mesh_att_battle_17_end   (seq 3종, 내용 다름)
     mesh_att_battle_17_start     2.00s
             0ms          전역표식 4개
          1100ms          전역표식 2개
          1200ms          전역표식 1개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_17_loop      1.00s
             0ms          전역표식 6개
           500ms~700   ms 타격 42061724  ht=4 원 반경 1.0m  전방오프셋 3.00m  밀치기 -0.40m/97ms  경직 300ms
          3500ms          전역표식 6개
          7000ms          전역표식 6개
          9900ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_17_end       3.00s
           500ms~700   ms 타격 42061724  ht=4 원 반경 1.0m  전방오프셋 3.00m  밀치기 -0.40m/97ms  경직 300ms
          1150ms          전역표식 1개
          2500ms          전역표식 2개
          2900ms          전역표식 1개
     mesh_att_battle_17_loop      1.00s
             0ms          전역표식 6개
           500ms~700   ms 타격 42061724  ht=4 원 반경 1.0m  전방오프셋 3.00m  밀치기 -0.40m/97ms  경직 300ms
          3500ms          전역표식 6개
          7000ms          전역표식 6개
          9900ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_17_end       3.00s
           500ms~700   ms 타격 42061724  ht=4 원 반경 1.0m  전방오프셋 3.00m  밀치기 -0.40m/97ms  경직 300ms
          1150ms          전역표식 1개
          2500ms          전역표식 2개
          2900ms          전역표식 1개

420618 레이드 발탄_마력구 폭발 (저지 성공)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=0 HOLD  mesh_abn_groggy_1_start -> mesh_abn_groggy_1_loop -> mesh_abn_groggy_1_end   (seq 3종, 내용 동일)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_abn_groggy_1_loop       0.80s
           600ms          단계전환 MonsterMoveNextStage
     mesh_abn_groggy_1_end        2.00s
             0ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
SHIELD         valtan.mechanic.magic-orb-stagger-76.shield    mesh_abn_groggy_1_start      1.83s
STAGGER_WINDOW valtan.mechanic.magic-orb-stagger-76.window    mesh_abn_groggy_1_loop       0.80s
RECOVERY       valtan.mechanic.magic-orb-stagger-76.recovery  mesh_abn_groggy_1_end        2.00s
```

---

### VALTAN_CENTER_GRAB_COUNTER_64 — 62줄 3페이즈 중앙 표적 폭발과 잡기 카운터

**규칙 칸 — 언제 나오는가**

```text
등장     체력 62줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420623 420631
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
CENTER         WINDUP      1400ms  판정없음
TARGET_EXPLOSION ACTIVE       800ms  원 반경 6.0m                         x1/0ms  밀치기 7.0m/150ms  넘어짐 1000ms  피해율 700
COUNTER_WINDOW WINDUP      2000ms  판정없음
FAILED_CHARGE  ACTIVE       650ms  전방박스 길이 18.0m 폭 6.0m              x1/0ms  밀치기 7.0m/150ms  넘어짐 1000ms  피해율 700
RECOVERY       RECOVERY    1200ms  판정없음
                           6050ms  (합계)
```

원본이 말하는 것:

```text
420623 레이드 발탄_돌진 잡기 후 사자후
   재사용 50000ms  사거리 7.0m  접근 2.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_21_01 -> mesh_att_battle_21_02 -> mesh_att_battle_21_03 -> mesh_att_battle_21_04 -> mesh_att_battle_13_02 -> mesh_att_battle_13_03 -> mesh_att_battle_13_04 -> mesh_att_battle_13_05-1 -> mesh_att_battle_21_02-1 -> mesh_att_battle_21_03-1 -> mesh_att_battle_21_04-1 -> mesh_att_battle_13_05-1 -> mesh_att_battle_21_04 -> mesh_att_battle_21_04-1   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_21_01        2.00s
          1400ms~1600  ms 타격 42062301  ht=5 부채꼴 반경 2.8m 각 140도
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_21_02        0.50s
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_21_03        0.93s
           300ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_21_04        2.00s
           650ms~850   ms 타격 42062304  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062305  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062361  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062362  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개
     mesh_att_battle_13_02        0.67s
           550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_13_03        1.00s
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_13_04        0.67s
           300ms~500   ms 타격 42062302  ht=5 전방박스 길이 6.5m 폭 2.0m  전방오프셋 0.50m
           550ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_13_05-1      3.00s
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개
     mesh_att_battle_21_02-1      0.50s
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_21_03-1      0.93s
           300ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_21_04-1      2.00s
           650ms~850   ms 타격 42062306  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062307  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062363  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062364  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개
     mesh_att_battle_13_05-1      3.00s
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개
     mesh_att_battle_21_04        2.00s
           650ms~850   ms 타격 42062304  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062305  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062361  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062362  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개
     mesh_att_battle_21_04-1      2.00s
           650ms~850   ms 타격 42062306  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062307  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062363  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062364  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개

420631 레이드 발탄_잡기 스킬 카운터 성공 및 저지 성공
   재사용 50000ms  사거리 8.0m  접근 3.5m  회전 0도
   동작   seq=3 HOLD  mesh_abn_groggy_1_start -> mesh_abn_groggy_1_loop -> mesh_abn_groggy_1_end   (seq 3종, 내용 동일)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_abn_groggy_1_loop       0.80s
           600ms          단계전환 MonsterMoveNextStage
     mesh_abn_groggy_1_end        2.00s
             0ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
CENTER         valtan.mechanic.center-grab-counter-64.center  mesh_att_battle_21_01        2.00s
TARGET_EXPLOSION valtan.mechanic.center-grab-counter-64.explosion mesh_att_battle_21_02        0.50s
COUNTER_WINDOW valtan.mechanic.center-grab-counter-64.counter mesh_att_battle_21_03        0.93s
FAILED_CHARGE  valtan.mechanic.center-grab-counter-64.failed-charge mesh_att_battle_21_04        2.00s
RECOVERY       valtan.mechanic.center-grab-counter-64.recovery mesh_att_battle_21_04-1      2.00s
```

---

### VALTAN_ARENA_BREAK_33 — 30줄 두 번째 지형 절반 파괴

**규칙 칸 — 언제 나오는가**

```text
등장     체력 30줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420629
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
48번  PRODUCT_CANDIDATE  hb=30   x1  30줄 중앙 착지와 지반 절반 삭제
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
CUTSCENE       WINDUP      2500ms  판정없음
LANDING        ACTIVE       900ms  원 반경 12.0m                        x1/0ms  넘어짐 1000ms  피해율 900
SPIN           ACTIVE      1200ms  원 반경 10.0m                        x3/350ms  넘어짐 1000ms  피해율 900
RECOVERY       RECOVERY    1500ms  판정없음
                           6100ms  (합계)
```

원본이 말하는 것:

```text
420629 레이드 발탄_모든 지형물 파괴 연출
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 COMBO  mesh_att_battle_12_01 -> mesh_att_battle_12_02 -> mesh_att_battle_12_03   (seq 3종, 내용 동일)
     mesh_att_battle_12_01        1.20s
             0ms          전역표식 8개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_02        1.00s
             0ms          전역표식 1개
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_03        1.20s
           200ms          전역표식 2개
           230ms~430   ms 타격 42061901  ht=5 원 반경 3.0m
           300ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
CUTSCENE       valtan.mechanic.arena-break-33.cutscene        mesh_att_battle_12_01        1.20s
LANDING        valtan.mechanic.arena-break-33.landing         mesh_att_battle_12_02        1.00s
SPIN           valtan.mechanic.arena-break-33.spin            mesh_att_battle_12_02        1.00s
RECOVERY       valtan.mechanic.arena-break-33.recovery        mesh_att_battle_12_03        1.20s
```

---

### VALTAN_GHOST_TRANSITION_15 — 14줄 포탈·최종 지면파괴와 망령화

**규칙 칸 — 언제 나오는가**

```text
등장     체력 14줄 대본 기믹, triggerOrder 1, 선택가중치 0(비선택)
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420616 420624 420625 420626 420634 420651 420652 420653 420658 420659 420665
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
55번  PRODUCT_PARTIAL    hb=14   x1  14줄 포탈 진입부터 망령 전환 전체
56번  PRODUCT_PARTIAL    hb=40   x1  연한 파란 망령화와 40줄 회복
58번  PRODUCT_PARTIAL    hb=34   x1  34줄 포탈과 저지 가능한 분신
64번  PRODUCT_PARTIAL    hb=19   x1  19~10줄 다중 포탈 분신 돌진
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
PORTAL_OPEN    WINDUP      1180ms  판정없음
GHOST_APPEAR   WINDUP       620ms  판정없음
FOUR_DIRECTIONS ACTIVE      1800ms  십자 길이 20.0m 폭 6.0m                x4/400ms  밀치기 5.0m/150ms  넘어짐 1000ms  피해율 900
TRACKING_EXPLOSIONS ACTIVE      2000ms  원 반경 6.0m                         x4/450ms  밀치기 5.0m/150ms  넘어짐 1000ms  피해율 900
INNER          ACTIVE       700ms  원 반경 7.0m                         x1/0ms  밀치기 5.0m/150ms  넘어짐 1000ms  피해율 900
OUTER          ACTIVE       700ms  링 7.0~18.0m                       x1/0ms  밀치기 5.0m/150ms  넘어짐 1000ms  피해율 900
GHOST          RECOVERY    2500ms  판정없음
                           9500ms  (합계)
```

원본이 말하는 것:

```text
420616 레이드 발탄_지면파괴 사자후 콤보
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 HOLD  mesh_idle_battle_1 x2 -> mesh_att_battle_16_01 -> mesh_att_battle_16_02 -> mesh_att_battle_16_03 -> mesh_att_battle_16_04 -> mesh_att_battle_16_05 -> mesh_att_battle_16_06 -> mesh_att_battle_16_08 -> mesh_att_battle_16_09 -> mesh_att_battle_16_10 -> mesh_att_battle_20_03 -> mesh_att_battle_20_02 -> mesh_att_battle_20_04 -> mesh_evt1_att_battle_5_01_start -> mesh_evt1_att_battle_5_01_loop -> mesh_evt1_att_battle_5_01_end -> mesh_att_battle_11_01 -> mesh_att_battle_16_01 -> mesh_att_battle_16_02 -> mesh_att_battle_16_03 -> mesh_idle_battle_1 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04 -> mesh_evt1_att_battle_5_01_end   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s   x2 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_16_01        1.83s
             0ms          전역표식 3개
           200ms          전역표식 1개
          1700ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_02        1.00s
           200ms          전역표식 1개
           800ms          전역표식 1개
          1500ms          전역표식 2개
          1700ms          전역표식 2개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_03        0.67s
           200ms~400   ms 타격 42061603  ht=5 원 반경 2.5m
           550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_04        1.00s
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_05        1.00s
           450ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_06        1.00s
           500ms          전역표식 1개
          2900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_08        1.00s
           300ms          전역표식 3개
           880ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_16_09        1.00s
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_10        2.23s
             0ms          전역표식 1개
           400ms~600   ms 타격 42061605  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061651  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061662  ht=1 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m  경직 325ms
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          1800ms          전역표식 1개
          1900ms          전역표식 2개
          2130ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_start 1.40s
             0ms          전역표식 1개
           400ms          전역표식 1개
          1300ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_loop 0.90s
          2900ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_end 2.63s
             0ms          전역표식 3개
           756ms~957   ms 타격 42060801  ht=4 원 반경 5.0m  경직 500ms
           756ms~957   ms 타격 42060802  ht=4 원 반경 10.0m  경직 500ms
           756ms~957   ms 타격 42060803  ht=4 원 반경 15.0m  경직 500ms
           756ms~957   ms 타격 42060804  ht=1 원 반경 20.0m  경직 325ms
          2500ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_11_01        1.80s
           600ms          전역표식 4개
          1300ms          전역표식 4개
     mesh_att_battle_16_01        1.83s
             0ms          전역표식 3개
           200ms          전역표식 1개
          1700ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_02        1.00s
           200ms          전역표식 1개
           800ms          전역표식 1개
          1500ms          전역표식 2개
          1700ms          전역표식 2개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_03        0.67s
           200ms~400   ms 타격 42061603  ht=5 원 반경 2.5m
           550ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_end 2.63s
             0ms          전역표식 3개
           756ms~957   ms 타격 42060801  ht=4 원 반경 5.0m  경직 500ms
           756ms~957   ms 타격 42060802  ht=4 원 반경 10.0m  경직 500ms
           756ms~957   ms 타격 42060803  ht=4 원 반경 15.0m  경직 500ms
           756ms~957   ms 타격 42060804  ht=1 원 반경 20.0m  경직 325ms
          2500ms          단계전환 MonsterMoveNextStageConditionProbability

420624 레이드 발탄_하얗게 불사르고 망령화 (마수 군단장 레이드 존 전용)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 HOLD  mesh_att_battle_18_01 -> mesh_att_battle_18_02 -> mesh_att_battle_1_01 -> mesh_idle_battle_1 x2 -> mesh_att_battle_18_03-1 -> mesh_att_battle_18_03-2 -> mesh_att_battle_19_01 -> mesh_att_battle_19_02 -> mesh_att_battle_19_03 -> mesh_att_battle_19_04 -> mesh_att_battle_5_01_loop -> mesh_att_battle_5_01_end -> mesh_att_battle_19_05 -> mesh_att_battle_19_06 x2 -> mesh_att_battle_5_01_end   (seq 3종, 내용 다름)
     mesh_att_battle_18_01        2.00s
             0ms          전역표식 6개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_18_02        0.50s
           100ms~300   ms 타격 42062201  ht=4 전방박스 길이 5.0m 폭 2.5m  밀치기 3.00m/242ms  경직 500ms
           200ms          전역표식 1개
           350ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_idle_battle_1           2.33s   x2 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_18_03-1      1.67s
             0ms          전역표식 1개
           400ms          전역표식 1개
           700ms~900   ms 타격 42062202  ht=5 원 반경 1.8m
           800ms~1000  ms 타격 42062203  ht=5 원 반경 2.8m  전방오프셋 2.90m
           800ms          전역표식 1개
     mesh_att_battle_18_03-2      1.67s
           400ms          전역표식 1개
           500ms~1100  ms 타격 42062259  ht=6 전방박스 길이 10.0m 폭 2.5m  전방오프셋 -8.50m
           500ms~700   ms 타격 42062204  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -8.00m
           700ms~900   ms 타격 42062205  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -5.00m
           900ms~1100  ms 타격 42062206  ht=6 전방박스 길이 3.0m 폭 2.5m  전방오프셋 -2.00m
          1100ms          전역표식 1개
     mesh_att_battle_19_01        5.00s
          1200ms~1400  ms 타격 42062402  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          1200ms~1400  ms 타격 42062403  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062404  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062405  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062406  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062407  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062408  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062409  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_02        1.50s
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_03        0.67s
           200ms~300   ms 타격 42062415  ht=5 원 반경 1.2m  전방오프셋 1.20m  밀치기 0.50m/0ms
           400ms~500   ms 타격 42062416  ht=5 원 반경 1.4m  전방오프셋 1.20m  밀치기 0.60m/0ms
           600ms          전역표식 1개
           800ms~900   ms 타격 42062417  ht=5 원 반경 1.6m  전방오프셋 1.20m  밀치기 0.70m/0ms
          1000ms~1100  ms 타격 42062418  ht=5 원 반경 1.8m  전방오프셋 1.20m  밀치기 0.80m/0ms
          1000ms          전역표식 1개
          1200ms          전역표식 2개
          1400ms~1500  ms 타격 42062419  ht=5 원 반경 2.0m  전방오프셋 1.20m  밀치기 0.90m/0ms
          1600ms~1700  ms 타격 42062420  ht=5 원 반경 2.2m  전방오프셋 1.20m  밀치기 1.00m/0ms
          1800ms          전역표식 1개
          2000ms~2100  ms 타격 42062421  ht=5 원 반경 2.4m  전방오프셋 1.20m  밀치기 1.10m/0ms
          2200ms~2300  ms 타격 42062422  ht=5 원 반경 2.6m  전방오프셋 1.20m  밀치기 1.20m/0ms
          2200ms          전역표식 1개
          2400ms          전역표식 2개
          2600ms~2700  ms 타격 42062423  ht=5 원 반경 2.8m  전방오프셋 1.20m  밀치기 1.30m/0ms
          2800ms~2900  ms 타격 42062424  ht=5 원 반경 3.0m  전방오프셋 1.20m  밀치기 1.40m/0ms
          3200ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_04        2.60s
           600ms          전역표식 2개
          1000ms~1200  ms 타격 42062425  ht=5 원 반경 4.0m  전방오프셋 1.60m
          1500ms          전역표식 1개
          2496ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_loop    4.00s
          3870ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_end     4.43s
           900ms          전역표식 2개
          1600ms          전역표식 1개
          1800ms          전역표식 1개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_05        2.33s
             0ms          전역표식 2개
           800ms          전역표식 2개
          2220ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_06        0.33s   x2 반복
             0ms          전역표식 1개
          2500ms          전역표식 1개
          2800ms          전역표식 1개
     mesh_att_battle_5_01_end     4.43s
           900ms          전역표식 2개
          1600ms          전역표식 1개
          1800ms          전역표식 1개
          2000ms          단계전환 MonsterMoveNextStage

420625 레이드 발탄_망령화 사망
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=3 HOLD  mesh_abn_groggy_1_start -> mesh_att_battle_5_01_loop -> mesh_att_battle_5_01_end -> mesh_abn_groggy_1_loop -> mesh_att_battle_5_03   (seq 4종, 내용 다름)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_att_battle_5_01_loop    4.00s
          3870ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_end     4.43s
           900ms          전역표식 2개
          1600ms          전역표식 1개
          1800ms          전역표식 1개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_abn_groggy_1_loop       0.80s
           600ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_03         1.00s

420626 레이드 발탄_망령화 분신 사망
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_abn_groggy_1_start   (seq 3종, 내용 동일)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개

420634 레이드 발탄_망령화 분신 스폰 대쉬 돌진
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_4_02   (seq 10종, 내용 동일)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_4_02         2.00s
             0ms~1200  ms 저지가능 구간
             0ms~200   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
             0ms~200   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           200ms~400   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           200ms~400   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           400ms~600   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           400ms~600   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           600ms~800   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           600ms~800   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           800ms~1000  ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           800ms~1000  ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          1900ms          전역표식 2개

420651 레이드 발탄_3페이즈 전멸기
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=2 HOLD  mesh_att_battle_5_01_start -> mesh_att_battle_5_01_loop -> mesh_att_battle_5_01_end   (seq 3종, 내용 다름)
     mesh_att_battle_5_01_start   1.40s
          1200ms~1300  ms 타격 42060305  ht=4 원 반경 3.5m  밀치기 -0.90m/97ms  경직 500ms
          1200ms~1300  ms 타격 42060306  ht=4 링 3.5~9.5m (360도)  밀치기 -1.60m/97ms  경직 450ms
          1200ms~1300  ms 타격 42060307  ht=4 링 9.5~15.0m (360도)  밀치기 -2.40m/97ms  경직 400ms
          1200ms~1300  ms 타격 42060312  ht=1 원 반경 10.0m  경직 325ms  maxt=3
          1300ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_loop    4.00s
          3870ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_end     4.43s
           900ms          전역표식 2개
          1600ms          전역표식 1개
          1800ms          전역표식 1개
          2000ms          단계전환 MonsterMoveNextStage

420652 레이드 발탄_망령화 발탄 분신 대기 스킬 (저지)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=2 SEQUENCE  mesh_idle_battle_1 -> mesh_evt2_idle_action_01 -> mesh_evt2_idle_action_02   (seq 3종, 내용 동일)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_evt2_idle_action_01     4.67s
     mesh_evt2_idle_action_02     4.00s

420653 레이드 발탄_망령화 발탄 분신 대기 스킬 (저지 완료)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=2 SEQUENCE  mesh_att_battle_21_01 -> mesh_idle_battle_1   (seq 3종, 내용 동일)
     mesh_att_battle_21_01        2.00s
          1400ms~1600  ms 타격 42062301  ht=5 부채꼴 반경 2.8m 각 140도
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

420658 레이드 발탄_마지막 지면파괴 사자후 콤보
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 HOLD  mesh_idle_battle_1 x2 -> mesh_att_battle_16_01 -> mesh_att_battle_16_02 -> mesh_att_battle_16_03 -> mesh_att_battle_16_04 -> mesh_att_battle_16_05 -> mesh_att_battle_16_06 -> mesh_att_battle_16_08 -> mesh_att_battle_16_09 -> mesh_att_battle_16_10 -> mesh_att_battle_20_03 -> mesh_att_battle_20_02 -> mesh_att_battle_20_04 -> mesh_evt1_att_battle_5_01_start -> mesh_evt1_att_battle_5_01_loop -> mesh_evt1_att_battle_5_01_end -> mesh_att_battle_11_01 -> mesh_att_battle_16_01 -> mesh_att_battle_16_02 -> mesh_att_battle_16_03 -> mesh_idle_battle_1 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04 -> mesh_evt1_att_battle_5_01_end   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s   x2 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_16_01        1.83s
             0ms          전역표식 3개
           200ms          전역표식 1개
          1700ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_02        1.00s
           200ms          전역표식 1개
           800ms          전역표식 1개
          1500ms          전역표식 2개
          1700ms          전역표식 2개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_03        0.67s
           200ms~400   ms 타격 42061603  ht=5 원 반경 2.5m
           550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_04        1.00s
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_05        1.00s
           450ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_06        1.00s
           500ms          전역표식 1개
          2900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_08        1.00s
           300ms          전역표식 3개
           880ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_16_09        1.00s
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_10        2.23s
             0ms          전역표식 1개
           400ms~600   ms 타격 42061605  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061651  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061662  ht=1 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m  경직 325ms
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          1800ms          전역표식 1개
          1900ms          전역표식 2개
          2130ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_start 1.40s
             0ms          전역표식 1개
           400ms          전역표식 1개
          1300ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_loop 0.90s
          2900ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_end 2.63s
             0ms          전역표식 3개
           756ms~957   ms 타격 42060801  ht=4 원 반경 5.0m  경직 500ms
           756ms~957   ms 타격 42060802  ht=4 원 반경 10.0m  경직 500ms
           756ms~957   ms 타격 42060803  ht=4 원 반경 15.0m  경직 500ms
           756ms~957   ms 타격 42060804  ht=1 원 반경 20.0m  경직 325ms
          2500ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_11_01        1.80s
           600ms          전역표식 4개
          1300ms          전역표식 4개
     mesh_att_battle_16_01        1.83s
             0ms          전역표식 3개
           200ms          전역표식 1개
          1700ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_02        1.00s
           200ms          전역표식 1개
           800ms          전역표식 1개
          1500ms          전역표식 2개
          1700ms          전역표식 2개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_03        0.67s
           200ms~400   ms 타격 42061603  ht=5 원 반경 2.5m
           550ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_end 2.63s
             0ms          전역표식 3개
           756ms~957   ms 타격 42060801  ht=4 원 반경 5.0m  경직 500ms
           756ms~957   ms 타격 42060802  ht=4 원 반경 10.0m  경직 500ms
           756ms~957   ms 타격 42060803  ht=4 원 반경 15.0m  경직 500ms
           756ms~957   ms 타격 42060804  ht=1 원 반경 20.0m  경직 325ms
          2500ms          단계전환 MonsterMoveNextStageConditionProbability

420659 레이드 발탄_지면파괴 사자후 콤보 마무리 공격
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_att_battle_16_08 -> mesh_att_battle_16_09 -> mesh_att_battle_16_10 -> mesh_idle_battle_1   (seq 3종, 내용 동일)
     mesh_att_battle_16_08        1.00s
           300ms          전역표식 3개
           880ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_16_09        1.00s
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_10        2.23s
             0ms          전역표식 1개
           400ms~600   ms 타격 42061605  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061651  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061662  ht=1 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m  경직 325ms
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          1800ms          전역표식 1개
          1900ms          전역표식 2개
          2130ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

420665 레이드 발탄_하얗게 불사르고 망령화_노멀 버전 (마수 군단장 레이드 존 전용)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 HOLD  mesh_att_battle_18_01 -> mesh_att_battle_18_02 -> mesh_att_battle_1_01 -> mesh_idle_battle_1 x2 -> mesh_att_battle_18_03-1 -> mesh_att_battle_18_03-2 -> mesh_att_battle_19_01 -> mesh_att_battle_19_02 -> mesh_att_battle_19_03 -> mesh_att_battle_19_04 -> mesh_att_battle_5_01_loop -> mesh_att_battle_5_01_end -> mesh_att_battle_19_05 -> mesh_att_battle_19_06 x2   (seq 3종, 내용 동일)
     mesh_att_battle_18_01        2.00s
             0ms          전역표식 6개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_18_02        0.50s
           100ms~300   ms 타격 42062201  ht=4 전방박스 길이 5.0m 폭 2.5m  밀치기 3.00m/242ms  경직 500ms
           200ms          전역표식 1개
           350ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_idle_battle_1           2.33s   x2 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_18_03-1      1.67s
             0ms          전역표식 1개
           400ms          전역표식 1개
           700ms~900   ms 타격 42062202  ht=5 원 반경 1.8m
           800ms~1000  ms 타격 42062203  ht=5 원 반경 2.8m  전방오프셋 2.90m
           800ms          전역표식 1개
     mesh_att_battle_18_03-2      1.67s
           400ms          전역표식 1개
           500ms~1100  ms 타격 42062259  ht=6 전방박스 길이 10.0m 폭 2.5m  전방오프셋 -8.50m
           500ms~700   ms 타격 42062204  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -8.00m
           700ms~900   ms 타격 42062205  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -5.00m
           900ms~1100  ms 타격 42062206  ht=6 전방박스 길이 3.0m 폭 2.5m  전방오프셋 -2.00m
          1100ms          전역표식 1개
     mesh_att_battle_19_01        5.00s
          1200ms~1400  ms 타격 42062402  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          1200ms~1400  ms 타격 42062403  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062404  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062405  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062406  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062407  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062408  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062409  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_02        1.50s
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_03        0.67s
           200ms~300   ms 타격 42062415  ht=5 원 반경 1.2m  전방오프셋 1.20m  밀치기 0.50m/0ms
           400ms~500   ms 타격 42062416  ht=5 원 반경 1.4m  전방오프셋 1.20m  밀치기 0.60m/0ms
           600ms          전역표식 1개
           800ms~900   ms 타격 42062417  ht=5 원 반경 1.6m  전방오프셋 1.20m  밀치기 0.70m/0ms
          1000ms~1100  ms 타격 42062418  ht=5 원 반경 1.8m  전방오프셋 1.20m  밀치기 0.80m/0ms
          1000ms          전역표식 1개
          1200ms          전역표식 2개
          1400ms~1500  ms 타격 42062419  ht=5 원 반경 2.0m  전방오프셋 1.20m  밀치기 0.90m/0ms
          1600ms~1700  ms 타격 42062420  ht=5 원 반경 2.2m  전방오프셋 1.20m  밀치기 1.00m/0ms
          1800ms          전역표식 1개
          2000ms~2100  ms 타격 42062421  ht=5 원 반경 2.4m  전방오프셋 1.20m  밀치기 1.10m/0ms
          2200ms~2300  ms 타격 42062422  ht=5 원 반경 2.6m  전방오프셋 1.20m  밀치기 1.20m/0ms
          2200ms          전역표식 1개
          2400ms          전역표식 2개
          2600ms~2700  ms 타격 42062423  ht=5 원 반경 2.8m  전방오프셋 1.20m  밀치기 1.30m/0ms
          2800ms~2900  ms 타격 42062424  ht=5 원 반경 3.0m  전방오프셋 1.20m  밀치기 1.40m/0ms
          3200ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_04        2.60s
           600ms          전역표식 2개
          1000ms~1200  ms 타격 42062425  ht=5 원 반경 4.0m  전방오프셋 1.60m
          1500ms          전역표식 1개
          2496ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_loop    4.00s
          3870ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_end     4.43s
           900ms          전역표식 2개
          1600ms          전역표식 1개
          1800ms          전역표식 1개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_05        2.33s
             0ms          전역표식 2개
           800ms          전역표식 2개
          2220ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_06        0.33s   x2 반복
             0ms          전역표식 1개
          2500ms          전역표식 1개
          2800ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
PORTAL_OPEN    valtan.mechanic.ghost-transition-15.portal-open mesh_abn_groggy_1_start      1.83s
GHOST_APPEAR   valtan.mechanic.ghost-transition-15.ghost-appear mesh_abn_groggy_1_loop       0.80s
FOUR_DIRECTIONS valtan.mechanic.ghost-transition-15.four-directions mesh_att_battle_5_01_loop    4.00s
TRACKING_EXPLOSIONS valtan.mechanic.ghost-transition-15.tracking   mesh_att_battle_5_01_end     4.43s
INNER          valtan.mechanic.ghost-transition-15.inner      mesh_abn_groggy_1_loop       0.80s
OUTER          valtan.mechanic.ghost-transition-15.outer      mesh_att_battle_5_03         1.00s
GHOST          valtan.mechanic.ghost-transition-15.ghost      mesh_abn_groggy_1_loop       0.80s
```

---

## 6. 일반 반복 패턴 24개

### VALTAN_DASH_CHARGE — 대쉬 돌진

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 30, 체력밴드 1~160줄, 연속 최대 2회
조건     갑옷=ARMORED  페이즈=PHASE_ONE  무적=false
사거리   5.0~20.0m
원본동작 420604
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
 2번  PRODUCT_CANDIDATE  hb=0    x1  양손 지면 3회 후 대쉬 돌진
 5번  PRODUCT_CANDIDATE  hb=0    x1  지면 3회 후 재돌진
10번  PRODUCT_CANDIDATE  hb=0    x1  지면 3회 후 플레이어 돌진
18번  PRODUCT_CANDIDATE  hb=0    x1  지면 3회 후 돌진 반복
24번  PRODUCT_CANDIDATE  hb=0    x1  지면 3회 후 돌진 반복
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       600ms  판정없음
CHARGE         ACTIVE       500ms  전방박스 길이 10.0m 폭 5.0m              x1/0ms  밀치기 2.0m/150ms  넘어짐 1000ms  피해율 400
GROGGY         GROGGY      5000ms  판정없음
RECOVERY       RECOVERY     900ms  판정없음
PART_BREAK     PART_BREAK  1400ms  판정없음
                           8400ms  (합계)
```

원본이 말하는 것:

```text
420604 레이드 발탄_대쉬 돌진
   재사용 5000ms  사거리 10.0m  접근 5.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 x3 -> mesh_att_battle_4_01 x10 -> mesh_att_battle_4_02 x2 -> mesh_att_battle_4_01 x2 -> mesh_att_battle_4_02 -> mesh_att_battle_4_01 -> mesh_att_battle_4_02 -> mesh_att_battle_4_01 -> mesh_att_battle_4_02 -> mesh_att_battle_4_01 x6   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s   x3 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_4_01         4.73s   x10 반복
          1900ms~3400  ms 저지가능 구간
          2450ms~2650  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2450ms~2650  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2650ms~2850  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2650ms~2850  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2850ms~3050  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2850ms~3050  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3050ms~3250  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3050ms~3250  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3250ms~3450  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3250ms~3450  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          4100ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          4600ms          전역표식 2개
     mesh_att_battle_4_02         2.00s   x2 반복
             0ms~1200  ms 저지가능 구간
             0ms~200   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
             0ms~200   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           200ms~400   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           200ms~400   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           400ms~600   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           400ms~600   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           600ms~800   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           600ms~800   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           800ms~1000  ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           800ms~1000  ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          1900ms          전역표식 2개
     mesh_att_battle_4_01         4.73s   x2 반복
          1900ms~3400  ms 저지가능 구간
          2450ms~2650  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2450ms~2650  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2650ms~2850  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2650ms~2850  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2850ms~3050  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2850ms~3050  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3050ms~3250  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3050ms~3250  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3250ms~3450  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3250ms~3450  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          4100ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          4600ms          전역표식 2개
     mesh_att_battle_4_02         2.00s
             0ms~1200  ms 저지가능 구간
             0ms~200   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
             0ms~200   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           200ms~400   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           200ms~400   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           400ms~600   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           400ms~600   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           600ms~800   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           600ms~800   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           800ms~1000  ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           800ms~1000  ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          1900ms          전역표식 2개
     mesh_att_battle_4_01         4.73s
          1900ms~3400  ms 저지가능 구간
          2450ms~2650  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2450ms~2650  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2650ms~2850  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2650ms~2850  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2850ms~3050  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2850ms~3050  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3050ms~3250  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3050ms~3250  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3250ms~3450  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3250ms~3450  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          4100ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          4600ms          전역표식 2개
     mesh_att_battle_4_02         2.00s
             0ms~1200  ms 저지가능 구간
             0ms~200   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
             0ms~200   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           200ms~400   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           200ms~400   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           400ms~600   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           400ms~600   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           600ms~800   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           600ms~800   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           800ms~1000  ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           800ms~1000  ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          1900ms          전역표식 2개
     mesh_att_battle_4_01         4.73s
          1900ms~3400  ms 저지가능 구간
          2450ms~2650  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2450ms~2650  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2650ms~2850  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2650ms~2850  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2850ms~3050  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2850ms~3050  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3050ms~3250  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3050ms~3250  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3250ms~3450  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3250ms~3450  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          4100ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          4600ms          전역표식 2개
     mesh_att_battle_4_02         2.00s
             0ms~1200  ms 저지가능 구간
             0ms~200   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
             0ms~200   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           200ms~400   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           200ms~400   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           400ms~600   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           400ms~600   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           600ms~800   ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           600ms~800   ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
           800ms~1000  ms 타격 42060402  ht=6 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
           800ms~1000  ms 타격 42060452  ht=1 전방박스 길이 4.5m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          1900ms          전역표식 2개
     mesh_att_battle_4_01         4.73s   x6 반복
          1900ms~3400  ms 저지가능 구간
          2450ms~2650  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2450ms~2650  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2650ms~2850  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2650ms~2850  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          2850ms~3050  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          2850ms~3050  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3050ms~3250  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3050ms~3250  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          3250ms~3450  ms 타격 42060401  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  밀치기 2.00m/0ms
          3250ms~3450  ms 타격 42060451  ht=1 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -0.25m  경직 325ms
          4100ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
          4600ms          전역표식 2개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.dash-charge.windup               mesh_att_battle_4_01         4.73s
CHARGE         valtan.attack.dash-charge.active               mesh_att_battle_4_01         4.73s
GROGGY         valtan.attack.dash-charge.groggy               mesh_dmg_parts_loop_1        1.33s
RECOVERY       valtan.attack.dash-charge.recovery             mesh_att_battle_4_02         2.00s
PART_BREAK     valtan.attack.dash-charge.part-break           mesh_dmg_parts_end_1         3.00s
```

---

### VALTAN_WHIRLWIND — 휠윈드

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 20, 체력밴드 1~160줄, 연속 최대 2회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~12.0m
원본동작 420633
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
 4번  PRODUCT_DIRECT     hb=0    x1  회오리 공격과 주변 벽 파괴
14번  PRODUCT_DIRECT     hb=0    x1  플레이어 추적 휠윈드
30번  PRODUCT_DIRECT     hb=0    x1  플레이어 추적 휠윈드 반복
52번  PRODUCT_DIRECT     hb=0    x1  제자리 회오리
63번  PRODUCT_DIRECT     hb=0    x1  제자리 회오리 반복
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1200ms  판정없음
SPIN           ACTIVE      1500ms  원 반경 10.0m                        x1/0ms  피해율 300
SPIN_2         ACTIVE       700ms  원 반경 10.0m                        x1/0ms  피해율 300
SPIN_3         ACTIVE       650ms  원 반경 10.0m                        x1/0ms  피해율 300
RECOVERY       RECOVERY    1000ms  판정없음
                           5050ms  (합계)
```

원본이 말하는 것:

```text
420633 레이드 발탄_휠윈드
   재사용 5000ms  사거리 3.0m  접근 1.5m  회전 0도
   동작   seq=2 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_20_02 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.whirlwind.windup                 mesh_att_battle_20_02        1.33s
SPIN           valtan.attack.whirlwind.active                 mesh_att_battle_20_03        0.53s
SPIN_2         valtan.attack.whirlwind.active-2               (바인딩 없음)
SPIN_3         valtan.attack.whirlwind.active-3               (바인딩 없음)
RECOVERY       valtan.attack.whirlwind.recovery               mesh_att_battle_20_04        1.47s
```

---

### VALTAN_FRONT_BACK_FRONT — 앞뒤앞 내려찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 20, 체력밴드 1~160줄, 연속 최대 2회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~10.0m
원본동작 420637 420666
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
39번  PRODUCT_DIRECT     hb=0    x1  도끼 앞뒤앞 내려찍기
54번  PRODUCT_DIRECT     hb=0    x1  앞뒤앞 내려찍기 매 타격 충격파
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       700ms  판정없음
SMASHES        ACTIVE      1200ms  십자 길이 9.0m 폭 4.0m                 x3/350ms  피해율 180
RECOVERY       RECOVERY    1000ms  판정없음
                           2900ms  (합계)
```

원본이 말하는 것:

```text
420637 레이드 발탄_앞뒤앞 내려찍기
   재사용 5000ms  사거리 5.0m  접근 3.0m  회전 180도
   동작   seq=1 COMBO  mesh_att_battle_19_01 -> mesh_att_battle_19_06 -> mesh_att_battle_2_03   (seq 4종, 내용 다름)
     mesh_att_battle_19_01        5.00s
          1200ms~1400  ms 타격 42062402  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          1200ms~1400  ms 타격 42062403  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062404  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062405  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062406  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062407  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062408  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062409  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_06        0.33s
             0ms          전역표식 1개
          2500ms          전역표식 1개
          2800ms          전역표식 1개
     mesh_att_battle_2_03         2.10s
             0ms~1200  ms 저지가능 구간
          1200ms~1400  ms 타격 42066104  ht=4 원 반경 3.2m  전방오프셋 2.75m  밀치기 2.75m/242ms  경직 400ms
          1300ms~1500  ms 타격 42060264  ht=5 원 반경 3.2m  전방오프셋 2.75m
          1300ms          전역표식 10개

420666 레이드 발탄_앞뒤앞 내려찍기_노멀 버전
   재사용 5000ms  사거리 5.0m  접근 3.0m  회전 180도
   동작   seq=1 COMBO  mesh_att_battle_19_01 -> mesh_att_battle_19_06 -> mesh_att_battle_2_03   (seq 4종, 내용 다름)
     mesh_att_battle_19_01        5.00s
          1200ms~1400  ms 타격 42062402  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          1200ms~1400  ms 타격 42062403  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062404  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          2200ms~2400  ms 타격 42062405  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062406  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          3200ms~3400  ms 타격 42062407  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062408  ht=5 부채꼴 반경 10.0m 각 80도  밀치기 1.00m/0ms
          4200ms~4400  ms 타격 42062409  ht=5 원 반경 1.8m  전방오프셋 2.75m  밀치기 1.00m/0ms
          4900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_06        0.33s
             0ms          전역표식 1개
          2500ms          전역표식 1개
          2800ms          전역표식 1개
     mesh_att_battle_2_03         2.10s
             0ms~1200  ms 저지가능 구간
          1200ms~1400  ms 타격 42066104  ht=4 원 반경 3.2m  전방오프셋 2.75m  밀치기 2.75m/242ms  경직 400ms
          1300ms~1500  ms 타격 42060264  ht=5 원 반경 3.2m  전방오프셋 2.75m
          1300ms          전역표식 10개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.front-back-front.windup          mesh_att_battle_19_01        5.00s
SMASHES        valtan.attack.front-back-front.active          mesh_att_battle_19_06        0.33s
RECOVERY       valtan.attack.front-back-front.recovery        mesh_att_battle_2_03         2.10s
```

---

### VALTAN_SWING — 휘두르기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 18, 체력밴드 1~160줄, 연속 최대 2회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~9.0m
원본동작 420601 420660
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       450ms  판정없음
SWEEP          ACTIVE       700ms  부채꼴 반경 8.0m 각 120도                x2/300ms  밀치기 2.0m/242ms  넘어짐 2000ms  피해율 220
RECOVERY       RECOVERY     750ms  판정없음
                           1900ms  (합계)
```

원본이 말하는 것:

```text
420601 레이드 발탄_휘두르기
   재사용 5000ms  사거리 3.5m  접근 3.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_att_battle_1_01 -> mesh_att_battle_1_02   (seq 5종, 내용 다름)
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_1_02         2.50s
          1000ms~2000  ms 저지가능 구간
          1800ms~2000  ms 타격 42060102  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1800ms~2400  ms 타격 42060103  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 4.25m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060104  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 -1.25m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060105  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 1.50m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060106  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 1.50m  밀치기 1.10m/0ms
          2400ms          단계전환 MonsterMoveNextStage

420660 레이드 발탄_휘두르기_노멀 버전
   재사용 5000ms  사거리 3.5m  접근 3.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_att_battle_1_01 -> mesh_att_battle_1_02   (seq 3종, 내용 다름)
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_1_02         2.50s
          1000ms~2000  ms 저지가능 구간
          1800ms~2000  ms 타격 42060102  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1800ms~2400  ms 타격 42060103  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 4.25m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060104  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 -1.25m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060105  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 1.50m  밀치기 1.10m/0ms
          1800ms~2400  ms 타격 42060106  ht=5 전방박스 길이 6.0m 폭 1.5m  전방오프셋 1.50m  밀치기 1.10m/0ms
          2400ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.swing.windup                     mesh_att_battle_1_01         3.00s
SWEEP          valtan.attack.swing.active                     mesh_att_battle_1_01         3.00s
RECOVERY       valtan.attack.swing.recovery                   mesh_att_battle_1_02         2.50s
```

---

### VALTAN_DOWN_SMASH — 내려찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 18, 체력밴드 1~160줄, 연속 최대 2회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~10.0m
원본동작 420602 420661
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
66번  PRODUCT_PARTIAL    hb=0    x1  마지막 위치 내려찍기와 포탈 분신
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       650ms  판정없음
IMPACT         ACTIVE       500ms  십자 길이 10.0m 폭 3.6m                x1/0ms  밀치기 2.8m/242ms  넘어짐 2000ms  피해율 350
RECOVERY       RECOVERY     850ms  판정없음
                           2000ms  (합계)
```

원본이 말하는 것:

```text
420602 레이드 발탄_내려찍기
   재사용 20000ms  사거리 3.5m  접근 2.5m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_2_01 -> mesh_att_battle_2_02 x2 -> mesh_att_battle_2_03 x2 -> mesh_att_battle_2_01 -> mesh_att_battle_2_02 x2   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_2_01         3.00s
          1600ms~1800  ms 타격 42060201  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms~1800  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
          1600ms~1800  ms 타격 42060261  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms          전역표식 3개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_2_02         1.20s   x2 반복
           950ms~1150  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
           950ms~1150  ms 타격 42060202  ht=5 원 반경 2.0m  전방오프셋 2.75m  밀치기 0.25m/0ms
           950ms~1150  ms 타격 42060262  ht=5 원 반경 2.0m  전방오프셋 2.75m
           950ms          전역표식 4개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_2_03         2.10s   x2 반복
             0ms~1200  ms 저지가능 구간
          1200ms~1400  ms 타격 42066104  ht=4 원 반경 3.2m  전방오프셋 2.75m  밀치기 2.75m/242ms  경직 400ms
          1300ms~1500  ms 타격 42060264  ht=5 원 반경 3.2m  전방오프셋 2.75m
          1300ms          전역표식 10개
     mesh_att_battle_2_01         3.00s
          1600ms~1800  ms 타격 42060201  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms~1800  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
          1600ms~1800  ms 타격 42060261  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms          전역표식 3개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_2_02         1.20s   x2 반복
           950ms~1150  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
           950ms~1150  ms 타격 42060202  ht=5 원 반경 2.0m  전방오프셋 2.75m  밀치기 0.25m/0ms
           950ms~1150  ms 타격 42060262  ht=5 원 반경 2.0m  전방오프셋 2.75m
           950ms          전역표식 4개
          1100ms          단계전환 MonsterMoveNextStage

420661 레이드 발탄_내려찍기_노멀 버전
   재사용 20000ms  사거리 3.5m  접근 2.5m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_2_01 -> mesh_att_battle_2_02 x2 -> mesh_att_battle_2_03 x2 -> mesh_att_battle_2_01 -> mesh_att_battle_2_02 x2   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_2_01         3.00s
          1600ms~1800  ms 타격 42060201  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms~1800  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
          1600ms~1800  ms 타격 42060261  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms          전역표식 3개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_2_02         1.20s   x2 반복
           950ms~1150  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
           950ms~1150  ms 타격 42060202  ht=5 원 반경 2.0m  전방오프셋 2.75m  밀치기 0.25m/0ms
           950ms~1150  ms 타격 42060262  ht=5 원 반경 2.0m  전방오프셋 2.75m
           950ms          전역표식 4개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_2_03         2.10s   x2 반복
             0ms~1200  ms 저지가능 구간
          1200ms~1400  ms 타격 42066104  ht=4 원 반경 3.2m  전방오프셋 2.75m  밀치기 2.75m/242ms  경직 400ms
          1300ms~1500  ms 타격 42060264  ht=5 원 반경 3.2m  전방오프셋 2.75m
          1300ms          전역표식 10개
     mesh_att_battle_2_01         3.00s
          1600ms~1800  ms 타격 42060201  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms~1800  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
          1600ms~1800  ms 타격 42060261  ht=5 원 반경 1.5m  전방오프셋 2.75m
          1600ms          전역표식 3개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_2_02         1.20s   x2 반복
           950ms~1150  ms 타격 42060218  ht=1 원 반경 5.0m  전방오프셋 1.40m  경직 325ms  maxt=1
           950ms~1150  ms 타격 42060202  ht=5 원 반경 2.0m  전방오프셋 2.75m  밀치기 0.25m/0ms
           950ms~1150  ms 타격 42060262  ht=5 원 반경 2.0m  전방오프셋 2.75m
           950ms          전역표식 4개
          1100ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.down-smash.windup                mesh_att_battle_2_01         3.00s
IMPACT         valtan.attack.down-smash.active                mesh_att_battle_2_03         2.10s
RECOVERY       valtan.attack.down-smash.recovery              mesh_att_battle_2_03         2.10s
```

---

### VALTAN_JUMP_SPIN — 점프 찍기 후 휠윈드

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 18, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   2.0~18.0m
원본동작 420621 420663
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
JUMP           WINDUP       950ms  판정없음
LAND           ACTIVE       500ms  십자 길이 9.0m 폭 3.6m                 x1/0ms  밀치기 3.0m/242ms  넘어짐 2000ms  피해율 300
SPIN           ACTIVE      1400ms  원 반경 10.0m                        x3/420ms  밀치기 3.0m/242ms  넘어짐 2000ms  피해율 300
RECOVERY       RECOVERY    1000ms  판정없음
                           3850ms  (합계)
```

원본이 말하는 것:

```text
420621 레이드 발탄_점프 찍기 후 휠윈드
   재사용 5000ms  사거리 8.0m  접근 5.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_20_01 -> mesh_att_battle_20_02 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04 x2 -> mesh_att_battle_20_02 -> mesh_att_battle_20_01 -> mesh_att_battle_20_03 x2 -> mesh_att_battle_20_04 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_01        2.50s
           500ms~1200  ms 저지가능 구간
          1250ms~1450  ms 타격 42062101  ht=5 원 반경 2.5m  전방오프셋 2.75m
          1250ms~2050  ms 타격 42062102  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 5.00m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062103  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 -0.50m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062104  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062105  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~1450  ms 타격 42062151  ht=5 원 반경 2.5m  전방오프셋 2.75m
          2000ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s   x2 반복
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_01        2.50s
           500ms~1200  ms 저지가능 구간
          1250ms~1450  ms 타격 42062101  ht=5 원 반경 2.5m  전방오프셋 2.75m
          1250ms~2050  ms 타격 42062102  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 5.00m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062103  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 -0.50m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062104  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062105  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~1450  ms 타격 42062151  ht=5 원 반경 2.5m  전방오프셋 2.75m
          2000ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s   x2 반복
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage

420663 레이드 발탄_점프 찍기 후 휠윈드_노멀 버전
   재사용 5000ms  사거리 8.0m  접근 5.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_20_01 -> mesh_att_battle_20_02 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04 x2 -> mesh_att_battle_20_02 -> mesh_att_battle_20_01 -> mesh_att_battle_20_03   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_01        2.50s
           500ms~1200  ms 저지가능 구간
          1250ms~1450  ms 타격 42062101  ht=5 원 반경 2.5m  전방오프셋 2.75m
          1250ms~2050  ms 타격 42062102  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 5.00m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062103  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 -0.50m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062104  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062105  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~1450  ms 타격 42062151  ht=5 원 반경 2.5m  전방오프셋 2.75m
          2000ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s   x2 반복
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_01        2.50s
           500ms~1200  ms 저지가능 구간
          1250ms~1450  ms 타격 42062101  ht=5 원 반경 2.5m  전방오프셋 2.75m
          1250ms~2050  ms 타격 42062102  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 5.00m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062103  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 -0.50m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062104  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~2050  ms 타격 42062105  ht=5 전방박스 길이 8.2m 폭 1.7m  전방오프셋 2.25m  밀치기 1.10m/0ms
          1250ms~1450  ms 타격 42062151  ht=5 원 반경 2.5m  전방오프셋 2.75m
          2000ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
JUMP           valtan.attack.jump-spin.jump                   mesh_att_battle_20_01        2.50s
LAND           valtan.attack.jump-spin.land                   mesh_att_battle_20_02        1.33s
SPIN           valtan.attack.jump-spin.spin                   mesh_att_battle_20_03        0.53s
RECOVERY       valtan.attack.jump-spin.recovery               mesh_att_battle_20_04        1.47s
```

---

### VALTAN_EARTHQUAKE_SMASH — 지진 찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 16, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~14.0m
원본동작 420605 420662
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
13번  PRODUCT_DIRECT     hb=0    x1  랜덤 위치 지진 찍기와 돌 상승
15번  PRODUCT_CANDIDATE  hb=0    x1  기 모아 내려찍기와 동서남북 돌뿌리
22번  PRODUCT_CANDIDATE  hb=0    x1  기 모아 내려찍기와 돌뿌리 반복
23번  PRODUCT_CANDIDATE  hb=0    x1  기 모아 내려찍기와 돌뿌리 반복
29번  PRODUCT_CANDIDATE  hb=0    x1  기 모아 내려찍기와 돌뿌리
31번  PRODUCT_CANDIDATE  hb=0    x1  기 모아 내려찍기와 돌뿌리
45번  PRODUCT_CANDIDATE  hb=0    x1  기 모아 내려찍기와 돌뿌리
59번  PRODUCT_PARTIAL    hb=31   x1  동서남북 내려찍기와 포탈 분신 돌진
60번  PRODUCT_PARTIAL    hb=30   x1  돌뿌리와 분신 플레이어 내려찍기
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       900ms  판정없음
IMPACT         ACTIVE       450ms  원 반경 7.0m                         x1/0ms  넘어짐 500ms  피해율 280
DELAYED_QUAKE  ACTIVE      1200ms  링 7.0~14.0m                       x1/0ms  넘어짐 500ms  피해율 280
RECOVERY       RECOVERY     900ms  판정없음
                           3450ms  (합계)
```

원본이 말하는 것:

```text
420605 레이드 발탄_지진 찍기
   재사용 10000ms  사거리 10.0m  접근 1.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_7_01 -> mesh_att_battle_7_03 -> mesh_att_battle_7_02 -> mesh_att_battle_7_03 -> mesh_att_battle_7_01 -> mesh_att_battle_7_03 -> mesh_turn_r_1 -> mesh_idle_battle_1 -> mesh_att_battle_7_01 -> mesh_turn_r_1   (seq 5종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_7_01         6.07s
          3160ms          전역표식 2개
          3162ms~3362  ms 타격 42060501  ht=5 부채꼴 반경 12.0m 각 60도
          3162ms          전역표식 1개
          3600ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_att_battle_7_02         1.20s
           796ms~996   ms 타격 42060502  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060552  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060555  ht=5 원 반경 1.5m  전방오프셋 3.00m
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_att_battle_7_01         6.07s
          3160ms          전역표식 2개
          3162ms~3362  ms 타격 42060501  ht=5 부채꼴 반경 12.0m 각 60도
          3162ms          전역표식 1개
          3600ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_turn_r_1                1.33s
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_7_01         6.07s
          3160ms          전역표식 2개
          3162ms~3362  ms 타격 42060501  ht=5 부채꼴 반경 12.0m 각 60도
          3162ms          전역표식 1개
          3600ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_turn_r_1                1.33s

420662 레이드 발탄_지진 찍기_노멀 버전
   재사용 10000ms  사거리 10.0m  접근 1.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_7_01 -> mesh_att_battle_7_03 -> mesh_att_battle_7_02 -> mesh_att_battle_7_03 -> mesh_att_battle_7_01 -> mesh_att_battle_7_03 -> mesh_turn_r_1 -> mesh_idle_battle_1 -> mesh_att_battle_7_01 -> mesh_turn_r_1   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_7_01         6.07s
          3160ms          전역표식 2개
          3162ms~3362  ms 타격 42060501  ht=5 부채꼴 반경 12.0m 각 60도
          3162ms          전역표식 1개
          3600ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_att_battle_7_02         1.20s
           796ms~996   ms 타격 42060502  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060552  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060555  ht=5 원 반경 1.5m  전방오프셋 3.00m
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_att_battle_7_01         6.07s
          3160ms          전역표식 2개
          3162ms~3362  ms 타격 42060501  ht=5 부채꼴 반경 12.0m 각 60도
          3162ms          전역표식 1개
          3600ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_turn_r_1                1.33s
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_7_01         6.07s
          3160ms          전역표식 2개
          3162ms~3362  ms 타격 42060501  ht=5 부채꼴 반경 12.0m 각 60도
          3162ms          전역표식 1개
          3600ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_turn_r_1                1.33s

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.earthquake-smash.windup          mesh_att_battle_7_01         6.07s
IMPACT         valtan.attack.earthquake-smash.impact          mesh_att_battle_7_03         2.00s
DELAYED_QUAKE  valtan.attack.earthquake-smash.delayed         mesh_att_battle_7_01         6.07s
RECOVERY       valtan.attack.earthquake-smash.recovery        mesh_att_battle_7_03         2.00s
```

---

### VALTAN_HIGH_JUMP — 고공 점프 찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 14, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   2.0~18.0m
원본동작 420610
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
 3번  PRODUCT_CANDIDATE  hb=0    x1  무작위 위치 점프 낙하
 8번  PRODUCT_PARTIAL    hb=0    x1  고공 점프 후 플레이어 위치 도끼 투척
 9번  PRODUCT_CANDIDATE  hb=0    x1  중앙 낙하 충격파
11번  PRODUCT_PARTIAL    hb=0    x1  두 번째 고공 도끼 투척
12번  PRODUCT_CANDIDATE  hb=0    x1  두 번째 중앙 낙하 충격파
40번  PRODUCT_DIRECT     hb=0    x2  플레이어 위치 점프 내려찍기 2회
43번  PRODUCT_DIRECT     hb=0    x2  플레이어 위치 점프 내려찍기 2회
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
TAKEOFF        WINDUP      1000ms  판정없음
AIRBORNE       WINDUP      1200ms  판정없음
LAND           ACTIVE       700ms  원 반경 10.0m                        x1/0ms  밀치기 1.0m/150ms  넘어짐 1000ms  피해율 500
RECOVERY       RECOVERY    1100ms  판정없음
                           4000ms  (합계)
```

원본이 말하는 것:

```text
420610 레이드 발탄_고공 점프 찍기
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=0 HOLD  mesh_idle_battle_1 -> mesh_att_battle_8_01_start -> mesh_att_battle_8_01_loop -> mesh_att_battle_8_01_end -> mesh_att_battle_8_01_loop -> mesh_att_battle_8_01_start -> mesh_att_battle_8_01_loop x2 -> mesh_att_battle_8_01_end x2 -> mesh_att_battle_8_01_loop x6 -> mesh_att_battle_8_01_end -> mesh_idle_battle_1 -> mesh_att_battle_8_01_start   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_8_01_start   1.93s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1750ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_loop    5.50s
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_end     3.20s
           240ms~440   ms 타격 42061008  ht=6 원 반경 3.5m
           250ms~450   ms 타격 42061031  ht=1 링 3.5~15.0m (360도)  경직 325ms  maxt=8
           250ms          전역표식 1개
          2000ms~2200  ms 타격 42061009  ht=5 링 3.5~15.0m (360도)  밀치기 1.00m/0ms
     mesh_att_battle_8_01_loop    5.50s
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_start   1.93s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1750ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_loop    5.50s   x2 반복
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_end     3.20s   x2 반복
           240ms~440   ms 타격 42061008  ht=6 원 반경 3.5m
           250ms~450   ms 타격 42061031  ht=1 링 3.5~15.0m (360도)  경직 325ms  maxt=8
           250ms          전역표식 1개
          2000ms~2200  ms 타격 42061009  ht=5 링 3.5~15.0m (360도)  밀치기 1.00m/0ms
     mesh_att_battle_8_01_loop    5.50s   x6 반복
             0ms          전역표식 2개
          3200ms          전역표식 1개
          3800ms          전역표식 6개
          4400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_8_01_end     3.20s
           240ms~440   ms 타격 42061008  ht=6 원 반경 3.5m
           250ms~450   ms 타격 42061031  ht=1 링 3.5~15.0m (360도)  경직 325ms  maxt=8
           250ms          전역표식 1개
          2000ms~2200  ms 타격 42061009  ht=5 링 3.5~15.0m (360도)  밀치기 1.00m/0ms
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_8_01_start   1.93s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1750ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
TAKEOFF        valtan.attack.high-jump.takeoff                mesh_att_battle_8_01_start   1.93s
AIRBORNE       valtan.attack.high-jump.airborne               mesh_att_battle_8_01_loop    5.50s
LAND           valtan.attack.high-jump.land                   mesh_att_battle_8_01_end     3.20s
RECOVERY       valtan.attack.high-jump.recovery               mesh_att_battle_8_01_end     3.20s
```

---

### VALTAN_FIST_IN_OUT — 두 손 내려찍기 안밖 폭발

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 14, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~16.0m
원본동작 420638
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
51번  PRODUCT_DIRECT     hb=28   x1  28줄 양손 지면 내려찍기와 외곽 파동
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1100ms  판정없음
INNER          ACTIVE       600ms  원 반경 7.0m                         x1/0ms  피해율 450
OUTER          ACTIVE       800ms  링 7.0~16.0m                       x1/0ms  피해율 450
RECOVERY       RECOVERY    1000ms  판정없음
                           3500ms  (합계)
```

원본이 말하는 것:

```text
420638 레이드 발탄_두손 내려찍어 지면 폭발
   재사용 10000ms  사거리 10.0m  접근 0.0m  회전 0도
   동작   seq=2 COMBO  mesh_att_battle_19_02 -> mesh_att_battle_19_04 -> mesh_att_battle_1_01   (seq 3종, 내용 동일)
     mesh_att_battle_19_02        1.50s
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_19_04        2.60s
           600ms          전역표식 2개
          1000ms~1200  ms 타격 42062425  ht=5 원 반경 4.0m  전방오프셋 1.60m
          1500ms          전역표식 1개
          2496ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.fist-in-out.windup               mesh_att_battle_19_02        1.50s
INNER          valtan.attack.fist-in-out.inner                mesh_att_battle_19_04        2.60s
OUTER          valtan.attack.fist-in-out.outer                mesh_att_battle_19_04        2.60s
RECOVERY       valtan.attack.fist-in-out.recovery             mesh_att_battle_1_01         3.00s
```

---

### VALTAN_FOUR_SLASH — 4연속 베기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 12, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~10.0m
원본동작 420609
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
 6번  PRODUCT_DIRECT     hb=0    x1  3회 휘두르기와 회전 충격파
17번  PRODUCT_DIRECT     hb=0    x1  4연속 베기 반복
20번  PRODUCT_DIRECT     hb=0    x1  4연속 베기 반복
38번  PRODUCT_DIRECT     hb=0    x1  4연속 베기 반복
44번  PRODUCT_DIRECT     hb=0    x1  4연속 베기 반복
53번  PRODUCT_DIRECT     hb=0    x1  4연속 베기 반복
62번  PRODUCT_DIRECT     hb=0    x1  4연속 베기 반복
67번  PRODUCT_PARTIAL    hb=0    x1  4연속 베기와 포탈 분신 돌진
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       500ms  판정없음
SLASHES        ACTIVE      1500ms  부채꼴 반경 9.0m 각 110도                x4/350ms  밀치기 2.0m/150ms  넘어짐 1000ms  피해율 160
RECOVERY       RECOVERY     850ms  판정없음
                           2850ms  (합계)
```

원본이 말하는 것:

```text
420609 레이드 발탄_4연속 베기
   재사용 10000ms  사거리 4.5m  접근 3.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_att_battle_10_01 -> mesh_att_battle_10_02 -> mesh_idle_battle_1 -> mesh_att_battle_10_02   (seq 3종, 내용 다름)
     mesh_att_battle_10_01        3.50s
          1791ms~1992  ms 타격 42060901  ht=4 부채꼴 반경 4.5m 각 100도  밀치기 1.20m/97ms  경직 400ms
          1791ms~1992  ms 타격 42060951  ht=4 부채꼴 반경 4.5m 각 100도  밀치기 1.20m/97ms  경직 400ms
          2300ms~2500  ms 타격 42060902  ht=4 부채꼴 반경 4.5m 각 100도  밀치기 1.20m/97ms  경직 400ms
          2300ms~2500  ms 타격 42060952  ht=4 부채꼴 반경 4.5m 각 100도  밀치기 1.20m/97ms  경직 400ms
          3138ms~3338  ms 타격 42060903  ht=5 원 반경 1.5m  전방오프셋 3.00m  밀치기 2.00m/0ms
          3138ms~3338  ms 타격 42060953  ht=5 원 반경 1.5m  전방오프셋 3.00m  밀치기 2.00m/0ms
          3420ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_10_02        3.17s
           600ms~800   ms 타격 42060904  ht=5 링 4.0~9.0m (360도)  전방오프셋 3.00m
          1800ms~2000  ms 타격 42060905  ht=5 원 반경 4.0m  밀치기 -1.00m/0ms
          1800ms~2000  ms 타격 42060955  ht=5 원 반경 4.0m  밀치기 -1.00m/0ms
          1800ms~2000  ms 타격 42060910  ht=4 원 반경 2.5m  밀치기 0.40m/97ms  경직 300ms
          1800ms~2000  ms 타격 42060911  ht=1 원 반경 2.5m  경직 325ms
          3000ms~3167  ms 타격 42060906  ht=5 링 4.0~6.0m (360도)  밀치기 0.50m/0ms
          3070ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_10_02        3.17s
           600ms~800   ms 타격 42060904  ht=5 링 4.0~9.0m (360도)  전방오프셋 3.00m
          1800ms~2000  ms 타격 42060905  ht=5 원 반경 4.0m  밀치기 -1.00m/0ms
          1800ms~2000  ms 타격 42060955  ht=5 원 반경 4.0m  밀치기 -1.00m/0ms
          1800ms~2000  ms 타격 42060910  ht=4 원 반경 2.5m  밀치기 0.40m/97ms  경직 300ms
          1800ms~2000  ms 타격 42060911  ht=1 원 반경 2.5m  경직 325ms
          3000ms~3167  ms 타격 42060906  ht=5 링 4.0~6.0m (360도)  밀치기 0.50m/0ms
          3070ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.four-slash.windup                mesh_att_battle_10_01        3.50s
SLASHES        valtan.attack.four-slash.active                mesh_att_battle_10_02        3.17s
RECOVERY       valtan.attack.four-slash.recovery              mesh_att_battle_10_02        3.17s
```

---

### VALTAN_GROUND_WAVE_SMASH — 지진파 내려찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 12, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~16.0m
원본동작 420615
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
36번  PRODUCT_DIRECT     hb=0    x1  7방향 지면 에너지파
47번  PRODUCT_DIRECT     hb=0    x1  7방향 지면 에너지파 반복
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       850ms  판정없음
WAVE           ACTIVE      1000ms  부채꼴 반경 15.0m 각 75도                x3/300ms  밀치기 0.4m/97ms  넘어짐 2000ms  피해율 300
RECOVERY       RECOVERY    1000ms  판정없음
                           2850ms  (합계)
```

원본이 말하는 것:

```text
420615 레이드 발탄_지진파 내려찍기
   재사용 50000ms  사거리 4.5m  접근 2.5m  회전 180도
   동작   seq=3 COMBO  mesh_att_battle_1_01 -> mesh_att_battle_15_01 -> mesh_att_battle_15_02 -> mesh_att_battle_15_03 -> mesh_att_battle_15_04 -> mesh_att_battle_15_05 -> mesh_att_battle_15_03 -> mesh_att_battle_15_04 x3 -> mesh_att_battle_15_01 -> mesh_att_battle_15_02 -> mesh_att_battle_15_03 x2 -> mesh_att_battle_15_04 x2 -> mesh_att_battle_15_05 -> mesh_att_battle_15_04 x2   (seq 3종, 내용 다름)
     mesh_att_battle_1_01         3.00s
          1570ms~1770  ms 타격 42060101  ht=4 부채꼴 반경 4.0m 각 140도  밀치기 2.00m/242ms  경직 400ms
          1570ms          전역표식 3개
          1900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_15_01        1.50s
           400ms~600   ms 타격 42061500  ht=1 원 반경 3.5m  경직 325ms
           400ms~600   ms 타격 42061530  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
           400ms~600   ms 타격 42061531  ht=1 원 반경 1.8m  경직 325ms
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_02        1.00s
           900ms~1400  ms 저지가능 구간
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_03        0.57s
           262ms          전역표식 4개
           450ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_04        1.00s
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_05        1.43s
     mesh_att_battle_15_03        0.57s
           262ms          전역표식 4개
           450ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_04        1.00s   x3 반복
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_01        1.50s
           400ms~600   ms 타격 42061500  ht=1 원 반경 3.5m  경직 325ms
           400ms~600   ms 타격 42061530  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
           400ms~600   ms 타격 42061531  ht=1 원 반경 1.8m  경직 325ms
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_02        1.00s
           900ms~1400  ms 저지가능 구간
          1400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_03        0.57s   x2 반복
           262ms          전역표식 4개
           450ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_04        1.00s   x2 반복
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_15_05        1.43s
     mesh_att_battle_15_04        1.00s   x2 반복
             0ms          전역표식 3개
           400ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.ground-wave-smash.windup         mesh_att_battle_1_01         3.00s
WAVE           valtan.attack.ground-wave-smash.active         mesh_att_battle_15_01        1.50s
RECOVERY       valtan.attack.ground-wave-smash.recovery       mesh_att_battle_15_05        1.43s
```

---

### VALTAN_STOMP — 발구르기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 10, 체력밴드 1~160줄, 연속 최대 2회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~9.0m
원본동작 420611
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       550ms  판정없음
IMPACT         ACTIVE       450ms  원 반경 7.0m                         x1/0ms  피해율 250
RECOVERY       RECOVERY     700ms  판정없음
                           1700ms  (합계)
```

원본이 말하는 것:

```text
420611 레이드 발탄_발구르기
   재사용 50000ms  사거리 5.0m  접근 0.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 x3 -> mesh_att_battle_11_01 x2 -> mesh_idle_battle_1 x2 -> mesh_att_battle_11_01 x4 -> mesh_idle_battle_1 -> mesh_att_battle_11_01 x2 -> mesh_idle_battle_1   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s   x3 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_11_01        1.80s   x2 반복
           600ms          전역표식 4개
          1300ms          전역표식 4개
     mesh_idle_battle_1           2.33s   x2 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_11_01        1.80s   x4 반복
           600ms          전역표식 4개
          1300ms          전역표식 4개
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_11_01        1.80s   x2 반복
           600ms          전역표식 4개
          1300ms          전역표식 4개
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.stomp.windup                     mesh_att_battle_11_01        1.80s
IMPACT         valtan.attack.stomp.active                     mesh_att_battle_11_01        1.80s
RECOVERY       valtan.attack.stomp.recovery                   mesh_att_battle_11_01        1.80s
```

---

### VALTAN_BACKSTEP_ATTACK — 공격하면서 뒤로 빠지기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 10, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~10.0m
원본동작 420635 420664
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP       450ms  판정없음
SWEEP          ACTIVE       700ms  부채꼴 반경 9.0m 각 120도                x1/0ms  피해율 220
RECOVERY       RECOVERY     700ms  판정없음
                           1850ms  (합계)
```

원본이 말하는 것:

```text
420635 레이드 발탄_공격하면서 뒤로 빠지기
   재사용 20000ms  사거리 4.5m  접근 2.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_20_03 -> mesh_att_battle_20_02 -> mesh_att_battle_20_04 -> mesh_turn_r_1 -> mesh_att_battle_7_02 -> mesh_turn_r_1 -> mesh_att_battle_7_03 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_turn_r_1                1.33s
     mesh_att_battle_7_02         1.20s
           796ms~996   ms 타격 42060502  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060552  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060555  ht=5 원 반경 1.5m  전방오프셋 3.00m
          1100ms          단계전환 MonsterMoveNextStage
     mesh_turn_r_1                1.33s
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage

420664 레이드 발탄_공격하면서 뒤로 빠지기_노멀 버전
   재사용 20000ms  사거리 4.5m  접근 2.0m  회전 180도
   동작   seq=0 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_20_03 -> mesh_att_battle_20_02 -> mesh_att_battle_20_04 -> mesh_turn_r_1 -> mesh_att_battle_7_02 -> mesh_turn_r_1 -> mesh_att_battle_7_03   (seq 3종, 내용 동일)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_turn_r_1                1.33s
     mesh_att_battle_7_02         1.20s
           796ms~996   ms 타격 42060502  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060552  ht=5 부채꼴 반경 12.0m 각 75도
           796ms~996   ms 타격 42060555  ht=5 원 반경 1.5m  전방오프셋 3.00m
          1100ms          단계전환 MonsterMoveNextStage
     mesh_turn_r_1                1.33s
     mesh_att_battle_7_03         2.00s
           796ms~996   ms 타격 42060503  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060553  ht=5 부채꼴 반경 12.0m 각 90도
           796ms~996   ms 타격 42060556  ht=5 원 반경 1.5m  전방오프셋 3.00m

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.backstep.windup                  mesh_att_battle_20_03        0.53s
SWEEP          valtan.attack.backstep.active                  mesh_att_battle_20_02        1.33s
RECOVERY       valtan.attack.backstep.recovery                mesh_att_battle_7_03         2.00s
```

---

### VALTAN_RED_BLADE_WAVE — 붉은 검기 날리기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 9, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   4.0~22.0m
원본동작 420636
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
27번  PRODUCT_PARTIAL    hb=0    x1  붉은 도끼와 비석 2개씩 폭파
34번  PRODUCT_PARTIAL    hb=0    x1  붉은 도끼와 비석 폭파
50번  PRODUCT_PARTIAL    hb=0    x1  붉은 도끼와 비석 폭파 반복
57번  PRODUCT_CANDIDATE  hb=37   x1  37줄 무작위 위치 충격파
61번  PRODUCT_PARTIAL    hb=0    x1  무작위 충격파와 포탈 분신 돌진
65번  PRODUCT_CANDIDATE  hb=0    x1  무작위 위치 충격파 반복
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1000ms  판정없음
PROJECTILE     ACTIVE       900ms  전방박스 길이 22.0m 폭 4.0m              x1/0ms  피해율 350
RECOVERY       RECOVERY     900ms  판정없음
                           2800ms  (합계)
```

원본이 말하는 것:

```text
420636 레이드 발탄_붉은 검기 날리기
   재사용 30000ms  사거리 10.0m  접근 0.0m  회전 180도
   동작   seq=2 SEQUENCE  mesh_att_battle_9_01_start -> mesh_att_battle_12_10 -> mesh_att_battle_12_11   (seq 3종, 내용 동일)
     mesh_att_battle_9_01_start   1.70s
             0ms          전역표식 4개
          1300ms          전역표식 1개
     mesh_att_battle_12_10        1.00s
          2900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_11        1.30s
           150ms~350   ms 타격 42061914  ht=6 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  밀치기 1.00m/0ms
           150ms~350   ms 타격 42061953  ht=6 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  밀치기 1.00m/0ms
           150ms~350   ms 타격 42061962  ht=1 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  경직 325ms
           700ms          전역표식 1개
          1150ms          전역표식 4개
          1190ms          단계전환 MonsterMoveNextStageConditionStatusEffect

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.red-blade-wave.windup            mesh_att_battle_9_01_start   1.70s
PROJECTILE     valtan.attack.red-blade-wave.active            mesh_att_battle_12_10        1.00s
RECOVERY       valtan.attack.red-blade-wave.recovery          mesh_att_battle_12_11        1.30s
```

---

### VALTAN_MAGIC_CHOICE — 마력기운 양자택일

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 8, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~18.0m
원본동작 420608
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
26번  PRODUCT_PARTIAL    hb=0    x1  비석 4개와 방향 에너지파
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1200ms  판정없음
INNER          ACTIVE       650ms  원 반경 7.0m                         x1/0ms  피해율 350
OUTER          ACTIVE       650ms  링 7.0~17.0m                       x1/0ms  피해율 350
RECOVERY       RECOVERY     900ms  판정없음
                           3400ms  (합계)
```

원본이 말하는 것:

```text
420608 레이드 발탄_마력기운 발산 후 양자택일
   재사용 50000ms  사거리 6.5m  접근 2.0m  회전 180도
   동작   seq=0 HOLD  mesh_evt1_att_battle_5_01_end -> mesh_att_battle_5_02_start -> mesh_att_battle_5_02_loop -> mesh_att_battle_5_02_end -> mesh_att_battle_5_02_start -> mesh_att_battle_5_02_loop -> mesh_att_battle_5_02_end -> mesh_att_battle_5_02_loop x2   (seq 3종, 내용 다름)
     mesh_evt1_att_battle_5_01_end 2.63s
             0ms          전역표식 3개
           756ms~957   ms 타격 42060801  ht=4 원 반경 5.0m  경직 500ms
           756ms~957   ms 타격 42060802  ht=4 원 반경 10.0m  경직 500ms
           756ms~957   ms 타격 42060803  ht=4 원 반경 15.0m  경직 500ms
           756ms~957   ms 타격 42060804  ht=1 원 반경 20.0m  경직 325ms
          2500ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_5_02_start   1.40s
          1300ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_5_02_loop    0.83s
             0ms          전역표식 2개
          3000ms          전역표식 1개
          3900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_02_end     1.50s
           534ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_02_start   1.40s
          1300ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_5_02_loop    0.83s
             0ms          전역표식 2개
          3000ms          전역표식 1개
          3900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_02_end     1.50s
           534ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_02_loop    0.83s   x2 반복
             0ms          전역표식 2개
          3000ms          전역표식 1개
          3900ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.magic-choice.windup              mesh_att_battle_5_02_start   1.40s
INNER          valtan.attack.magic-choice.inner               mesh_att_battle_5_02_loop    0.83s
OUTER          valtan.attack.magic-choice.outer               mesh_att_battle_5_02_loop    0.83s
RECOVERY       valtan.attack.magic-choice.recovery            mesh_att_battle_5_02_end     1.50s
```

---

### VALTAN_PORTAL_RUSH — 워프 돌진 콤보

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 8, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   4.0~22.0m
원본동작 420622
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
PORTAL         WINDUP      1000ms  판정없음
RUSHES         ACTIVE      2000ms  전방박스 길이 20.0m 폭 5.0m              x3/600ms  밀치기 3.0m/242ms  넘어짐 2000ms  피해율 450
FINISH         ACTIVE       600ms  원 반경 6.0m                         x1/0ms  밀치기 3.0m/242ms  넘어짐 2000ms  피해율 450
RECOVERY       RECOVERY    1100ms  판정없음
                           4700ms  (합계)
```

원본이 말하는 것:

```text
420622 레이드 발탄_워프 돌진 콤보
   재사용 50000ms  사거리 4.5m  접근 2.5m  회전 180도
   동작   seq=1 SEQUENCE  mesh_att_battle_18_01 -> mesh_att_battle_18_02 -> mesh_idle_battle_1 x3 -> mesh_att_battle_18_03-1 -> mesh_att_battle_18_03-2 -> mesh_idle_battle_1   (seq 3종, 내용 다름)
     mesh_att_battle_18_01        2.00s
             0ms          전역표식 6개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_18_02        0.50s
           100ms~300   ms 타격 42062201  ht=4 전방박스 길이 5.0m 폭 2.5m  밀치기 3.00m/242ms  경직 500ms
           200ms          전역표식 1개
           350ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_idle_battle_1           2.33s   x3 반복
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_18_03-1      1.67s
             0ms          전역표식 1개
           400ms          전역표식 1개
           700ms~900   ms 타격 42062202  ht=5 원 반경 1.8m
           800ms~1000  ms 타격 42062203  ht=5 원 반경 2.8m  전방오프셋 2.90m
           800ms          전역표식 1개
     mesh_att_battle_18_03-2      1.67s
           400ms          전역표식 1개
           500ms~1100  ms 타격 42062259  ht=6 전방박스 길이 10.0m 폭 2.5m  전방오프셋 -8.50m
           500ms~700   ms 타격 42062204  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -8.00m
           700ms~900   ms 타격 42062205  ht=6 전방박스 길이 4.0m 폭 2.5m  전방오프셋 -5.00m
           900ms~1100  ms 타격 42062206  ht=6 전방박스 길이 3.0m 폭 2.5m  전방오프셋 -2.00m
          1100ms          전역표식 1개
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

```

**동작 칸 — 어떤 clip이 나오는가**

```text
PORTAL         valtan.attack.portal-rush.portal               mesh_att_battle_18_01        2.00s
RUSHES         valtan.attack.portal-rush.rushes               mesh_att_battle_18_02        0.50s
FINISH         valtan.attack.portal-rush.finish               mesh_att_battle_18_03-1      1.67s
RECOVERY       valtan.attack.portal-rush.recovery             mesh_att_battle_18_03-2      1.67s
```

---

### VALTAN_LEDGE_ROAR — 낙사 사자후

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 8, 체력밴드 1~90줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~18.0m
원본동작 420639
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1300ms  판정없음
ROAR           ACTIVE       750ms  부채꼴 반경 17.0m 각 120도               x1/0ms  피해율 400
RECOVERY       RECOVERY    1000ms  판정없음
                           3050ms  (합계)
```

원본이 말하는 것:

```text
420639 레이드 발탄_낙사 사자후
   재사용 50000ms  사거리 10.0m  접근 0.0m  회전 0도
   동작   seq=2 HOLD  mesh_evt1_att_battle_5_01_start -> mesh_evt1_att_battle_5_01_end -> mesh_evt1_att_battle_5_01_loop   (seq 3종, 내용 동일)
     mesh_evt1_att_battle_5_01_start 1.40s
             0ms          전역표식 1개
           400ms          전역표식 1개
          1300ms          단계전환 MonsterMoveNextStage
     mesh_evt1_att_battle_5_01_end 2.63s
             0ms          전역표식 3개
           756ms~957   ms 타격 42060801  ht=4 원 반경 5.0m  경직 500ms
           756ms~957   ms 타격 42060802  ht=4 원 반경 10.0m  경직 500ms
           756ms~957   ms 타격 42060803  ht=4 원 반경 15.0m  경직 500ms
           756ms~957   ms 타격 42060804  ht=1 원 반경 20.0m  경직 325ms
          2500ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_evt1_att_battle_5_01_loop 0.90s
          2900ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.ledge-roar.windup                mesh_evt1_att_battle_5_01_start 1.40s
ROAR           valtan.attack.ledge-roar.active                mesh_evt1_att_battle_5_01_loop 0.90s
RECOVERY       valtan.attack.ledge-roar.recovery              mesh_evt1_att_battle_5_01_end 2.63s
```

---

### VALTAN_IMPRISON_ROAR — 감금 사자후

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 6, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~18.0m
원본동작 420603
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1100ms  판정없음
ROAR           ACTIVE       700ms  원 반경 16.0m                        x1/0ms  밀치기 -2.4m/97ms  넘어짐 2000ms  피해율 220
RECOVERY       RECOVERY    1000ms  판정없음
                           2800ms  (합계)
```

원본이 말하는 것:

```text
420603 레이드 발탄_감금 사자후
   재사용 50000ms  사거리 8.0m  접근 1.0m  회전 180도
   동작   seq=0 HOLD  mesh_idle_battle_1 -> mesh_att_battle_5_01_start -> mesh_att_battle_5_01_loop -> mesh_att_battle_5_01_end -> mesh_att_battle_5_01_start   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_5_01_start   1.40s
          1200ms~1300  ms 타격 42060305  ht=4 원 반경 3.5m  밀치기 -0.90m/97ms  경직 500ms
          1200ms~1300  ms 타격 42060306  ht=4 링 3.5~9.5m (360도)  밀치기 -1.60m/97ms  경직 450ms
          1200ms~1300  ms 타격 42060307  ht=4 링 9.5~15.0m (360도)  밀치기 -2.40m/97ms  경직 400ms
          1200ms~1300  ms 타격 42060312  ht=1 원 반경 10.0m  경직 325ms  maxt=3
          1300ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_loop    4.00s
          3870ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_end     4.43s
           900ms          전역표식 2개
          1600ms          전역표식 1개
          1800ms          전역표식 1개
          2000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_5_01_start   1.40s
          1200ms~1300  ms 타격 42060305  ht=4 원 반경 3.5m  밀치기 -0.90m/97ms  경직 500ms
          1200ms~1300  ms 타격 42060306  ht=4 링 3.5~9.5m (360도)  밀치기 -1.60m/97ms  경직 450ms
          1200ms~1300  ms 타격 42060307  ht=4 링 9.5~15.0m (360도)  밀치기 -2.40m/97ms  경직 400ms
          1200ms~1300  ms 타격 42060312  ht=1 원 반경 10.0m  경직 325ms  maxt=3
          1300ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.imprison-roar.windup             mesh_att_battle_5_01_start   1.40s
ROAR           valtan.attack.imprison-roar.active             mesh_att_battle_5_01_loop    4.00s
RECOVERY       valtan.attack.imprison-roar.recovery           mesh_att_battle_5_01_end     4.43s
```

---

### VALTAN_PARRY — 큰 베기 반격

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 6, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~12.0m
원본동작 420606 420607
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
STANCE         WINDUP      2000ms  판정없음
COUNTER_SLASH  ACTIVE       700ms  원 반경 12.0m                        x1/0ms  밀치기 4.9m/535ms  넘어짐 2000ms  피해율 600
RECOVERY       RECOVERY    1000ms  판정없음
                           3700ms  (합계)
```

원본이 말하는 것:

```text
420606 레이드 발탄_큰 베기 (반격기)
   재사용 50000ms  사거리 4.0m  접근 2.5m  회전 180도
   동작   seq=0 HOLD  mesh_att_battle_9_01_start -> mesh_att_battle_9_01_loop -> mesh_att_battle_9_01_end   (seq 3종, 내용 동일)
     mesh_att_battle_9_01_start   1.70s
             0ms          전역표식 4개
          1300ms          전역표식 1개
     mesh_att_battle_9_01_loop    1.50s
          1360ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_9_01_end     1.07s
             0ms          전역표식 1개
           140ms~340   ms 타격 42060601  ht=4 부채꼴 반경 4.5m 각 220도  전방오프셋 0.50m  밀치기 4.90m/535ms  경직 400ms
          1000ms          전역표식 1개

420607 레이드 발탄_큰 베기 (반격 성공)
   재사용 50000ms  사거리 4.0m  접근 2.5m  회전 0도
   동작   seq=3 SEQUENCE  mesh_att_battle_9_01_end-2   (seq 3종, 내용 동일)
     mesh_att_battle_9_01_end-2   1.60s
           300ms~500   ms 타격 42060602  ht=5 원 반경 6.0m  밀치기 2.00m/0ms
          1500ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
STANCE         valtan.reactive.parry.stance                   mesh_att_battle_9_01_start   1.70s
COUNTER_SLASH  valtan.reactive.parry.slash                    mesh_att_battle_9_01_loop    1.50s
RECOVERY       valtan.reactive.parry.recovery                 mesh_att_battle_9_01_end     1.07s
```

---

### VALTAN_BIND_CHARGE_SMASH — 구속 돌진 잡기 후 내려찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 6, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   3.0~18.0m
원본동작 420612 420613 420614
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
LOCK           WINDUP      1100ms  판정없음
CHARGE         ACTIVE       700ms  전방박스 길이 14.0m 폭 5.0m              x1/0ms  밀치기 1.0m/150ms  넘어짐 1000ms  피해율 500
SMASH          ACTIVE       800ms  원 반경 8.0m                         x1/0ms  밀치기 1.0m/150ms  넘어짐 1000ms  피해율 500
RECOVERY       RECOVERY    1200ms  판정없음
                           3800ms  (합계)
```

원본이 말하는 것:

```text
420612 레이드 발탄_구속시켜 돌진 잡기 후 내려찍기 시작 (군단장 스킬)
   재사용 50000ms  사거리 8.0m  접근 0.5m  회전 180도
   동작   seq=2 SEQUENCE  mesh_att_battle_12_01 -> mesh_att_battle_13_01 -> mesh_idle_battle_1 -> mesh_att_battle_13_03 -> mesh_att_battle_13_02-1 -> mesh_att_battle_12_03 -> mesh_att_battle_12_02   (seq 3종, 내용 동일)
     mesh_att_battle_12_01        1.20s
             0ms          전역표식 8개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_13_01        2.33s
          1450ms          전역표식 1개
          1700ms          전역표식 2개
          2100ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_13_03        1.00s
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_13_02-1      4.10s
          4000ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_03        1.20s
           200ms          전역표식 2개
           230ms~430   ms 타격 42061901  ht=5 원 반경 3.0m
           300ms          전역표식 1개
     mesh_att_battle_12_02        1.00s
             0ms          전역표식 1개
           900ms          단계전환 MonsterMoveNextStage

420613 레이드 발탄_구속시켜 돌진 잡기 후 내려찍기 종료 (군단장 스킬)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=2 SEQUENCE  mesh_att_battle_13_03 x2 -> mesh_att_battle_13_04 -> mesh_att_battle_13_05-1 -> mesh_att_battle_13_03 x8 -> mesh_att_battle_13_04 x7 -> mesh_att_battle_13_05-1 -> mesh_att_battle_13_03 x9 -> mesh_att_battle_13_04 -> mesh_att_battle_13_03   (seq 4종, 내용 다름)
     mesh_att_battle_13_03        1.00s   x2 반복
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_13_04        0.67s
           300ms~500   ms 타격 42062302  ht=5 전방박스 길이 6.5m 폭 2.0m  전방오프셋 0.50m
           550ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_13_05-1      3.00s
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개
     mesh_att_battle_13_03        1.00s   x8 반복
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_13_04        0.67s   x7 반복
           300ms~500   ms 타격 42062302  ht=5 전방박스 길이 6.5m 폭 2.0m  전방오프셋 0.50m
           550ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_13_05-1      3.00s
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개
     mesh_att_battle_13_03        1.00s   x9 반복
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_13_04        0.67s
           300ms~500   ms 타격 42062302  ht=5 전방박스 길이 6.5m 폭 2.0m  전방오프셋 0.50m
           550ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_13_03        1.00s
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget

420614 레이드 발탄_구속시켜 돌진 잡기 후 내려찍기 (예외사항 처리)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_att_battle_13_05-1 x3   (seq 3종, 내용 다름)
     mesh_att_battle_13_05-1      3.00s   x3 반복
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
LOCK           valtan.attack.bind-charge-smash.lock           mesh_att_battle_12_01        1.20s
CHARGE         valtan.attack.bind-charge-smash.charge         mesh_att_battle_13_01        2.33s
SMASH          valtan.attack.bind-charge-smash.smash          mesh_att_battle_13_03        1.00s
RECOVERY       valtan.attack.bind-charge-smash.recovery       mesh_att_battle_12_02        1.00s
```

---

### VALTAN_CHARGE_GRAB_ROAR — 돌진 잡기 후 사자후

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 6, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   3.0~18.0m
원본동작 420623 420631 420632
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
COUNTER_WINDOW WINDUP      1800ms  판정없음
CHARGE         ACTIVE       650ms  전방박스 길이 16.0m 폭 6.0m              x1/0ms  밀치기 7.0m/150ms  넘어짐 1000ms  피해율 500
ROAR           ACTIVE       800ms  부채꼴 반경 14.0m 각 100도               x1/0ms  밀치기 7.0m/150ms  넘어짐 1000ms  피해율 500
RECOVERY       RECOVERY    1200ms  판정없음
                           4450ms  (합계)
```

원본이 말하는 것:

```text
420623 레이드 발탄_돌진 잡기 후 사자후
   재사용 50000ms  사거리 7.0m  접근 2.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_21_01 -> mesh_att_battle_21_02 -> mesh_att_battle_21_03 -> mesh_att_battle_21_04 -> mesh_att_battle_13_02 -> mesh_att_battle_13_03 -> mesh_att_battle_13_04 -> mesh_att_battle_13_05-1 -> mesh_att_battle_21_02-1 -> mesh_att_battle_21_03-1 -> mesh_att_battle_21_04-1 -> mesh_att_battle_13_05-1 -> mesh_att_battle_21_04 -> mesh_att_battle_21_04-1   (seq 4종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_21_01        2.00s
          1400ms~1600  ms 타격 42062301  ht=5 부채꼴 반경 2.8m 각 140도
          1700ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_21_02        0.50s
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_21_03        0.93s
           300ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_21_04        2.00s
           650ms~850   ms 타격 42062304  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062305  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062361  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062362  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개
     mesh_att_battle_13_02        0.67s
           550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_13_03        1.00s
             0ms          전역표식 5개
           200ms          전역표식 1개
          1000ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_13_04        0.67s
           300ms~500   ms 타격 42062302  ht=5 전방박스 길이 6.5m 폭 2.0m  전방오프셋 0.50m
           550ms          단계전환 MonsterMoveNextStageConditionSkillEffectHit
     mesh_att_battle_13_05-1      3.00s
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개
     mesh_att_battle_21_02-1      0.50s
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_21_03-1      0.93s
           300ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_21_04-1      2.00s
           650ms~850   ms 타격 42062306  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062307  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062363  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062364  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개
     mesh_att_battle_13_05-1      3.00s
             0ms~200   ms 저지가능 구간
             0ms          전역표식 1개
          1500ms~1700  ms 타격 42061207  ht=5 원 반경 4.5m  전방오프셋 1.70m  밀치기 1.00m/0ms
          1500ms~1700  ms 타격 42061208  ht=5 원 반경 4.5m  전방오프셋 1.70m
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          2200ms          전역표식 8개
          2800ms          전역표식 1개
     mesh_att_battle_21_04        2.00s
           650ms~850   ms 타격 42062304  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062305  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062361  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062362  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개
     mesh_att_battle_21_04-1      2.00s
           650ms~850   ms 타격 42062306  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062307  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062363  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062364  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개

420631 레이드 발탄_잡기 스킬 카운터 성공 및 저지 성공
   재사용 50000ms  사거리 8.0m  접근 3.5m  회전 0도
   동작   seq=3 HOLD  mesh_abn_groggy_1_start -> mesh_abn_groggy_1_loop -> mesh_abn_groggy_1_end   (seq 3종, 내용 동일)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_abn_groggy_1_loop       0.80s
           600ms          단계전환 MonsterMoveNextStage
     mesh_abn_groggy_1_end        2.00s
             0ms          전역표식 1개

420632 레이드 발탄_돌진 잡기 후 사자후 (예외사항 처리)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=2 COMBO  mesh_att_battle_21_02 -> mesh_att_battle_21_03 -> mesh_att_battle_21_04   (seq 3종, 내용 다름)
     mesh_att_battle_21_02        0.50s
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_21_03        0.93s
           300ms          전역표식 1개
          2400ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_21_04        2.00s
           650ms~850   ms 타격 42062304  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062305  ht=5 부채꼴 반경 30.0m 각 135도  밀치기 7.00m/0ms
           650ms~850   ms 타격 42062361  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms~850   ms 타격 42062362  ht=1 부채꼴 반경 30.0m 각 135도  경직 325ms
           650ms          전역표식 1개
          1799ms          전역표식 1개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
COUNTER_WINDOW valtan.attack.charge-grab-roar.counter         mesh_att_battle_21_01        2.00s
CHARGE         valtan.attack.charge-grab-roar.charge          mesh_att_battle_21_02        0.50s
ROAR           valtan.attack.charge-grab-roar.roar            mesh_att_battle_21_03        0.93s
RECOVERY       valtan.attack.charge-grab-roar.recovery        mesh_att_battle_21_04-1      2.00s
```

---

### VALTAN_SUPER_SMASH — 초강력 내려찍기 콤보

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 5, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~15.0m
원본동작 420619 420620 420656 420657
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
16번  PRODUCT_CANDIDATE  hb=0    x1  플레이어 위치 기반 4회 내려찍기
35번  PRODUCT_CANDIDATE  hb=0    x1  플레이어 위치 기반 4회 내려찍기
46번  PRODUCT_CANDIDATE  hb=0    x1  플레이어 위치 기반 4회 내려찍기
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1600ms  판정없음
IMPACTS        ACTIVE      1800ms  원 반경 13.0m                        x3/550ms  밀치기 5.0m/150ms  넘어짐 1000ms  피해율 700
RECOVERY       RECOVERY    1400ms  판정없음
                           4800ms  (합계)
```

원본이 말하는 것:

```text
420619 레이드 발탄_초강력 내려찍기 콤보 시작 (군단장 스킬)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 COMBO  mesh_att_battle_12_01 -> mesh_att_battle_12_02 -> mesh_att_battle_12_03 -> mesh_att_battle_12_04 -> mesh_att_battle_12_05 -> mesh_att_battle_12_04   (seq 3종, 내용 동일)
     mesh_att_battle_12_01        1.20s
             0ms          전역표식 8개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_02        1.00s
             0ms          전역표식 1개
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_03        1.20s
           200ms          전역표식 2개
           230ms~430   ms 타격 42061901  ht=5 원 반경 3.0m
           300ms          전역표식 1개
     mesh_att_battle_12_04        2.80s
           900ms          전역표식 7개
          2000ms          전역표식 2개
          2700ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_05        5.00s
           400ms          전역표식 1개
     mesh_att_battle_12_04        2.80s
           900ms          전역표식 7개
          2000ms          전역표식 2개
          2700ms          단계전환 MonsterMoveNextStage

420620 레이드 발탄_초강력 내려찍기 콤보 종료 (군단장 스킬) - 백업
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_att_battle_12_06 -> mesh_att_battle_12_07 -> mesh_att_battle_12_08 -> mesh_att_battle_12_09 -> mesh_att_battle_12_10 -> mesh_att_battle_12_11 -> mesh_att_battle_12_06   (seq 3종, 내용 동일)
     mesh_att_battle_12_06        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_07        1.20s
           250ms~450   ms 타격 42061913  ht=5 원 반경 1.4m
           250ms~450   ms 타격 42061952  ht=5 원 반경 1.5m
           250ms          전역표식 4개
           800ms          전역표식 2개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_08        1.00s
           100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_09        1.20s
             0ms          전역표식 2개
           206ms          전역표식 1개
          1100ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_12_10        1.00s
          2900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_11        1.30s
           150ms~350   ms 타격 42061914  ht=6 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  밀치기 1.00m/0ms
           150ms~350   ms 타격 42061953  ht=6 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  밀치기 1.00m/0ms
           150ms~350   ms 타격 42061962  ht=1 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  경직 325ms
           700ms          전역표식 1개
          1150ms          전역표식 4개
          1190ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_12_06        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage

420656 레이드 발탄_초강력 내려찍기 콤보 종료 - 1 (군단장 스킬)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_att_battle_12_06 -> mesh_att_battle_12_07 -> mesh_att_battle_12_08 -> mesh_att_battle_12_09 -> mesh_att_battle_12_06 -> mesh_idle_battle_1   (seq 4종, 내용 다름)
     mesh_att_battle_12_06        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_07        1.20s
           250ms~450   ms 타격 42061913  ht=5 원 반경 1.4m
           250ms~450   ms 타격 42061952  ht=5 원 반경 1.5m
           250ms          전역표식 4개
           800ms          전역표식 2개
          1100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_08        1.00s
           100ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_09        1.20s
             0ms          전역표식 2개
           206ms          전역표식 1개
          1100ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_12_06        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

420657 레이드 발탄_초강력 내려찍기 콤보 종료 - 2 (군단장 스킬)
   재사용 50000ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_att_battle_9_01_start -> mesh_att_battle_12_10 -> mesh_att_battle_12_11 -> mesh_idle_battle_1 -> mesh_att_battle_16_10 -> mesh_att_battle_16_08 -> mesh_att_battle_16_09 -> mesh_idle_battle_1   (seq 4종, 내용 다름)
     mesh_att_battle_9_01_start   1.70s
             0ms          전역표식 4개
          1300ms          전역표식 1개
     mesh_att_battle_12_10        1.00s
          2900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_12_11        1.30s
           150ms~350   ms 타격 42061914  ht=6 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  밀치기 1.00m/0ms
           150ms~350   ms 타격 42061953  ht=6 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  밀치기 1.00m/0ms
           150ms~350   ms 타격 42061962  ht=1 부채꼴 반경 16.0m 각 80도  전방오프셋 -1.00m  경직 325ms
           700ms          전역표식 1개
          1150ms          전역표식 4개
          1190ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_16_10        2.23s
             0ms          전역표식 1개
           400ms~600   ms 타격 42061605  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061651  ht=6 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m
           400ms~600   ms 타격 42061662  ht=1 부채꼴 반경 16.0m 각 100도  전방오프셋 -1.00m  경직 325ms
          1500ms          전역표식 1개
          1700ms          전역표식 1개
          1800ms          전역표식 1개
          1900ms          전역표식 2개
          2130ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_16_08        1.00s
           300ms          전역표식 3개
           880ms          단계전환 MonsterMoveNextStageConditionChangeTarget
     mesh_att_battle_16_09        1.00s
          4900ms          단계전환 MonsterMoveNextStage
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.attack.super-smash.windup               mesh_att_battle_12_01        1.20s
IMPACTS        valtan.attack.super-smash.active               mesh_att_battle_12_02        1.00s
RECOVERY       valtan.attack.super-smash.recovery             mesh_att_battle_12_05        5.00s
```

---

### VALTAN_TRIPLE_COUNTER — 연속 카운터 내려찍기

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 4, 체력밴드 30~90줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~18.0m
원본동작 420640 420641 420642 420643 420644 420645 420646 420647
```

네 67개 관찰에 배정된 자리가 **없다**. 지금 등장 근거는 encounter 저작뿐이다.

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
COUNTER_1      WINDUP      1800ms  판정없음
FAIL_1         ACTIVE       600ms  원 반경 18.0m                        x1/0ms  넘어짐 2000ms  피해율 500
COUNTER_2      WINDUP      1600ms  판정없음
FAIL_2         ACTIVE       600ms  원 반경 18.0m                        x1/0ms  넘어짐 2000ms  피해율 500
COUNTER_3      WINDUP      1400ms  판정없음
FAIL_3         ACTIVE       600ms  원 반경 100.0m                       x1/0ms  넘어짐 2000ms  피해율 100000
RECOVERY       RECOVERY    1200ms  판정없음
                           7800ms  (합계)
```

원본이 말하는 것:

```text
420640 레이드 발탄_연속 카운터 내려찍기 (마지막 그로기)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=0 HOLD  mesh_abn_groggy_1_start -> mesh_abn_groggy_1_loop -> mesh_abn_groggy_1_end   (seq 3종, 내용 동일)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_abn_groggy_1_loop       0.80s
           600ms          단계전환 MonsterMoveNextStage
     mesh_abn_groggy_1_end        2.00s
             0ms          전역표식 1개

420641 레이드 발탄_연속 카운터 내려찍기 (중간 그로기)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=2 HOLD  mesh_abn_groggy_1_start -> mesh_abn_groggy_1_loop -> mesh_abn_groggy_1_end   (seq 3종, 내용 동일)
     mesh_abn_groggy_1_start      1.83s
             0ms          전역표식 11개
     mesh_abn_groggy_1_loop       0.80s
           600ms          단계전환 MonsterMoveNextStage
     mesh_abn_groggy_1_end        2.00s
             0ms          전역표식 1개

420642 레이드 발탄_연속 카운터 내려찍기 (첫 공격)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 180도
   동작   seq=1 SEQUENCE  mesh_att_battle_14_01 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_02 x4 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 x2 -> mesh_att_battle_14_02   (seq 3종, 내용 동일)
     mesh_att_battle_14_01        2.00s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x4 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s   x2 반복
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage

420643 레이드 발탄_연속 카운터 내려찍기 (중간 공격)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_att_battle_14_02 x3 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_02 x4 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 x2 -> mesh_att_battle_14_02 x3   (seq 3종, 내용 동일)
     mesh_att_battle_14_02        1.00s   x3 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x4 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s   x2 반복
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x3 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage

420644 레이드 발탄_연속 카운터 내려찍기 (마지막 공격)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_att_battle_14_02 x3 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 x4 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 x2 -> mesh_att_battle_14_02 -> mesh_att_battle_14_04-2 -> mesh_att_battle_14_02   (seq 3종, 내용 동일)
     mesh_att_battle_14_02        1.00s   x3 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x4 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s   x2 반복
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-2      1.00s
             0ms          전역표식 4개
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage

420645 레이드 발탄_연속 카운터 내려찍기 (그로기 후 중간 공격)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_02 x4 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 x2 -> mesh_att_battle_14_02 x3   (seq 3종, 내용 동일)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x4 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s   x2 반복
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x3 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage

420646 레이드 발탄_연속 카운터 내려찍기 (그로기 후 마지막 공격)
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_02 x4 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 x2 -> mesh_att_battle_14_03 x2 -> mesh_att_battle_14_02 x2   (seq 3종, 내용 동일)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x4 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s   x2 반복
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s   x2 반복
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage

420647 레이드 발탄_연속 카운터 내려찍기_백업용
   재사용 0ms  사거리 100.0m  접근 0.0m  회전 0도
   동작   seq=1 SEQUENCE  mesh_att_battle_14_01 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_02 -> mesh_att_battle_14_03 -> mesh_att_battle_14_04-1 -> mesh_att_battle_14_03 -> mesh_att_battle_14_02 -> mesh_att_battle_14_04-2   (seq 3종, 내용 동일)
     mesh_att_battle_14_01        2.00s
             0ms          전역표식 3개
          1600ms          전역표식 2개
          1900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-1      1.00s
           900ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_03        1.67s
           100ms~600   ms 저지가능 구간
          1000ms          전역표식 1개
          1550ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_02        1.00s
             0ms          전역표식 2개
           200ms          전역표식 1개
           400ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_14_04-2      1.00s
             0ms          전역표식 4개

```

**동작 칸 — 어떤 clip이 나오는가**

```text
COUNTER_1      valtan.reactive.triple-counter.first           mesh_abn_groggy_1_start      1.83s
FAIL_1         valtan.reactive.triple-counter.first-fail      mesh_abn_groggy_1_loop       0.80s
COUNTER_2      valtan.reactive.triple-counter.second          mesh_abn_groggy_1_loop       0.80s
FAIL_2         valtan.reactive.triple-counter.second-fail     mesh_abn_groggy_1_loop       0.80s
COUNTER_3      valtan.reactive.triple-counter.third           mesh_abn_groggy_1_loop       0.80s
FAIL_3         valtan.reactive.triple-counter.third-fail      mesh_abn_groggy_1_loop       0.80s
RECOVERY       valtan.reactive.triple-counter.recovery        mesh_abn_groggy_1_end        2.00s
```

---

### VALTAN_ENTRANCE_WHIRLWIND — 첫 등장 회전 돌진

**규칙 칸 — 언제 나오는가**

```text
등장     일반 선택, 가중치 1, 체력밴드 1~160줄, 연속 최대 1회
조건     갑옷=ANY  페이즈=ANY  무적=false
사거리   0.0~100.0m
원본동작 420633
```

네 67개 관찰에서 이 패턴이 맡은 자리:

```text
 1번  PRODUCT_CANDIDATE  hb=160  x1  160줄 첫 등장 회오리
```

**모양·시간 칸 — 몇 미터 몇 초인가**

현재 저작:

```text
WINDUP         WINDUP      1070ms  판정없음
SWEEP          ACTIVE      1930ms  원 반경 7.0m                         x2/700ms  피해율 300
RECOVERY       RECOVERY    3160ms  판정없음
                           6160ms  (합계)
```

원본이 말하는 것:

```text
420633 레이드 발탄_휠윈드
   재사용 5000ms  사거리 3.0m  접근 1.5m  회전 0도
   동작   seq=2 SEQUENCE  mesh_idle_battle_1 -> mesh_att_battle_20_02 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04 -> mesh_att_battle_20_03 -> mesh_att_battle_20_04   (seq 3종, 내용 다름)
     mesh_idle_battle_1           2.33s
             0ms          전역표식 1개
           800ms          전역표식 1개
          1600ms          전역표식 1개
          2400ms          전역표식 1개
          3200ms          전역표식 1개
          4000ms          전역표식 1개
          4800ms          전역표식 1개
          6900ms          단계전환 MonsterMoveNextStageConditionProbability
     mesh_att_battle_20_02        1.33s
          1200ms          단계전환 MonsterMoveNextStageConditionStatusEffect
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_03        0.53s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062156  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
          1500ms          단계전환 MonsterMoveNextStage
     mesh_att_battle_20_04        1.47s
             0ms~200   ms 타격 42062106  ht=4 원 반경 3.2m  밀치기 3.00m/242ms  경직 500ms
             0ms~200   ms 타격 42062107  ht=4 원 반경 1.8m  밀치기 0.40m/97ms  경직 300ms
             0ms~200   ms 타격 42062108  ht=1 원 반경 1.8m  경직 325ms
           700ms          전역표식 1개
          1350ms          단계전환 MonsterMoveNextStage

```

**동작 칸 — 어떤 clip이 나오는가**

```text
WINDUP         valtan.mechanic.entrance-whirlwind.windup      mesh_att_battle_20_02        1.33s
SWEEP          valtan.mechanic.entrance-whirlwind.sweep       mesh_att_battle_20_03        0.53s
RECOVERY       valtan.mechanic.entrance-whirlwind.recovery    mesh_att_battle_20_04        1.47s
```

---
## 7. 대조 요약 — 어디가 어긋나 있고, 어디는 원본이 침묵하는가

카드를 다 읽지 않고도 규모를 보라고 만든 표다. 두 가지를 먼저 밝힌다.

첫째, `원본 도형`은 그 동작의 `.skilltiming` shape 행 중 크기가 지정된 것
(`area>0` 이고 `ar<10000`) 전부다. 100m 짜리는 아레나 전역 표식이라 뺐다.
둘째, 그중 **어느 것이 그 패턴의 대표 판정인지는 이 표가 정하지 않는다.**
원본은 한 동작에 여러 도형을 담고 clip 의 어느 순간에 어느 것이 터지는지로
구분한다. 그 순간별 대응은 아래 5·6장 카드의 clip 타임라인에 있다.

```text
패턴                             현재 저작 첫 ACTIVE             원본이 준 도형
----------------------------------------------------------------------------------------------------
VALTAN_SWING                   부채꼴 반경 8.0m 각 120도         부채꼴 반경 4.0m 각 140도 / 원 반경 1.5m / 전방박스 길이 6.0m 폭 1.5m
VALTAN_DOWN_SMASH              십자 길이 10.0m 폭 3.6m         원 반경 1.5m / 원 반경 2.0m / 원 반경 2.3m  ...외 2종
VALTAN_IMPRISON_ROAR           원 반경 16.0m                 원 반경 10.0m / 원 반경 15.0m / 원 반경 7.5m  ...외 3종
VALTAN_DASH_CHARGE             전방박스 길이 10.0m 폭 5.0m       전방박스 길이 4.0m 폭 2.5m / 전방박스 길이 4.5m 폭 2.5m / 전방박스 길이 11.8m 폭 2.5m  ...외 1종
VALTAN_EARTHQUAKE_SMASH        원 반경 7.0m                  부채꼴 반경 12.0m 각 60도 / 부채꼴 반경 12.0m 각 75도 / 부채꼴 반경 12.0m 각 90도  ...외 1종
VALTAN_PARRY                   원 반경 12.0m                 부채꼴 반경 4.5m 각 220도 / 원 반경 6.0m
VALTAN_MAGIC_CHOICE            원 반경 7.0m                  원 반경 5.0m / 원 반경 10.0m / 원 반경 15.0m  ...외 3종
VALTAN_FOUR_SLASH              부채꼴 반경 9.0m 각 110도         부채꼴 반경 4.5m 각 100도 / 원 반경 1.5m / 링 4.0~9.0m (360도)  ...외 4종
VALTAN_HIGH_JUMP               원 반경 10.0m                 원 반경 1.8m / 원 반경 3.5m / 링 3.5~15.0m (360도)  ...외 2종
VALTAN_STOMP                   원 반경 7.0m                  원 반경 1.5m / 원 반경 30.0m / 링 1.5~3.0m (360도)  ...외 1종
VALTAN_BIND_CHARGE_SMASH       전방박스 길이 14.0m 폭 5.0m       부채꼴 반경 9.0m 각 135도 / 전방박스 길이 5.0m 폭 2.0m / 전방박스 길이 6.0m 폭 2.0m  ...외 15종
VALTAN_GROUND_WAVE_SMASH       부채꼴 반경 15.0m 각 75도         원 반경 3.5m / 원 반경 2.0m / 원 반경 4.0m  ...외 4종
VALTAN_SUPER_SMASH             원 반경 13.0m                 원 반경 3.0m / 원 반경 2.5m / 부채꼴 반경 15.0m 각 35도  ...외 4종
VALTAN_JUMP_SPIN               십자 길이 9.0m 폭 3.6m          원 반경 2.5m / 전방박스 길이 8.2m 폭 1.7m / 원 반경 3.2m  ...외 1종
VALTAN_PORTAL_RUSH             전방박스 길이 20.0m 폭 5.0m       전방박스 길이 5.0m 폭 2.5m / 원 반경 1.8m / 원 반경 2.8m  ...외 5종
VALTAN_CHARGE_GRAB_ROAR        전방박스 길이 16.0m 폭 6.0m       부채꼴 반경 2.8m 각 140도 / 전방박스 길이 6.5m 폭 2.0m / 부채꼴 반경 30.0m 각 135도  ...외 3종
VALTAN_WHIRLWIND               원 반경 10.0m                 원 반경 1.8m
VALTAN_BACKSTEP_ATTACK         부채꼴 반경 9.0m 각 120도         없음 — 원본이 크기를 안 준다
VALTAN_RED_BLADE_WAVE          전방박스 길이 22.0m 폭 4.0m       없음 — 원본이 크기를 안 준다
VALTAN_FRONT_BACK_FRONT        십자 길이 9.0m 폭 4.0m          부채꼴 반경 10.0m 각 80도 / 원 반경 1.8m / 원 반경 3.5m
VALTAN_FIST_IN_OUT             원 반경 7.0m                  원 반경 4.0m / 부채꼴 반경 10.0m 각 80도 / 링 4.5~10.0m (360도)  ...외 1종
VALTAN_LEDGE_ROAR              부채꼴 반경 17.0m 각 120도        없음 — 원본이 크기를 안 준다
VALTAN_TRIPLE_COUNTER          원 반경 18.0m                 원 반경 2.0m / 원 반경 0.3m
VALTAN_ARMOR_BREAK_OPENING     (판정 스테이지 없음)               원 반경 1.8m / 원 반경 2.8m
VALTAN_FLOOR_WIPE_130          6방향 길이 14.0m 폭 4.4m        원 반경 2.5m / 부채꼴 반경 20.0m 각 35도 / 원 반경 1.0m
VALTAN_FOUR_PILLARS_105        링 4.0~100.0m               원 반경 1.8m / 원 반경 3.5m / 링 3.5~15.0m (360도)  ...외 2종
VALTAN_ENTRANCE_WHIRLWIND      원 반경 7.0m                  원 반경 1.8m
VALTAN_ARENA_BREAK_109         원 반경 12.0m                 없음 — 원본이 크기를 안 준다
VALTAN_ARENA_BREAK_84          (판정 스테이지 없음)               없음 — 원본이 크기를 안 준다
VALTAN_MAGIC_ORB_STAGGER_76    (판정 스테이지 없음)               부채꼴 반경 10.0m 각 25도 / 원 반경 1.0m / 전방박스 길이 2.5m 폭 1.5m  ...외 4종
VALTAN_CENTER_GRAB_COUNTER_64  원 반경 6.0m                  부채꼴 반경 2.8m 각 140도 / 전방박스 길이 6.5m 폭 2.0m / 부채꼴 반경 30.0m 각 135도  ...외 1종
VALTAN_ARENA_BREAK_33          원 반경 12.0m                 없음 — 원본이 크기를 안 준다
VALTAN_GHOST_TRANSITION_15     십자 길이 20.0m 폭 6.0m         원 반경 2.5m / 부채꼴 반경 16.0m 각 100도 / 부채꼴 반경 15.0m 각 176도  ...외 23종
```

```text
원본 도형 목록에 현재 값이 있는 패턴    0개  -
원본 도형이 있는데 값이 다른 패턴      27개
원본이 크기를 아예 안 준 패턴           6개
    VALTAN_BACKSTEP_ATTACK
    VALTAN_RED_BLADE_WAVE
    VALTAN_LEDGE_ROAR
    VALTAN_ARENA_BREAK_109
    VALTAN_ARENA_BREAK_84
    VALTAN_ARENA_BREAK_33
```

마지막 묶음이 중요하다. 이 패턴들은 **원본이 답을 갖고 있지 않다.** 지어내지 말고
선배 문장·네 관찰·플레이 감각 중 무엇을 근거로 삼을지 네가 정해야 하는 자리다.

### 7.1 스테이지 시간

패턴이 실제로 바인딩한 clip 길이 합과 저작 `durationMs` 합을 비교했다.
1.00x 여야 애니메이션이 잘리거나 남지 않는다.

```text
패턴                                   저작 합     clip 합       비율
--------------------------------------------------------------
VALTAN_SWING                       1900ms     5500ms    0.35x
VALTAN_DOWN_SMASH                  2000ms     5100ms    0.39x
VALTAN_IMPRISON_ROAR               2800ms     9833ms    0.28x
VALTAN_DASH_CHARGE                 8400ms    11067ms    0.76x
VALTAN_EARTHQUAKE_SMASH            3450ms     8067ms    0.43x
VALTAN_PARRY                       3700ms     4267ms    0.87x
VALTAN_MAGIC_CHOICE                3400ms     3733ms    0.91x
VALTAN_FOUR_SLASH                  2850ms     6667ms    0.43x
VALTAN_HIGH_JUMP                   4000ms    10633ms    0.38x
VALTAN_STOMP                       1700ms     1800ms    0.94x
VALTAN_BIND_CHARGE_SMASH           3800ms     5533ms    0.69x
VALTAN_GROUND_WAVE_SMASH           2850ms     5933ms    0.48x
VALTAN_SUPER_SMASH                 4800ms     7200ms    0.67x
VALTAN_JUMP_SPIN                   3850ms     5833ms    0.66x
VALTAN_PORTAL_RUSH                 4700ms     5833ms    0.81x
VALTAN_CHARGE_GRAB_ROAR            4450ms     5433ms    0.82x
VALTAN_WHIRLWIND                   5050ms     3333ms    1.52x
VALTAN_BACKSTEP_ATTACK             1850ms     3867ms    0.48x
VALTAN_RED_BLADE_WAVE              2800ms     4000ms    0.70x
VALTAN_FRONT_BACK_FRONT            2900ms     7433ms    0.39x
VALTAN_FIST_IN_OUT                 3500ms     7100ms    0.49x
VALTAN_LEDGE_ROAR                  3050ms     4933ms    0.62x
VALTAN_TRIPLE_COUNTER              7800ms     4633ms    1.68x
VALTAN_ARMOR_BREAK_OPENING         9100ms     5833ms    1.56x
VALTAN_FLOOR_WIPE_130              6600ms     8833ms    0.75x
VALTAN_FOUR_PILLARS_105            4700ms    10633ms    0.44x
VALTAN_ENTRANCE_WHIRLWIND          6160ms     3333ms    1.85x
VALTAN_ARENA_BREAK_109             6270ms     3400ms    1.84x
VALTAN_ARENA_BREAK_84              2600ms        0ms        -
VALTAN_MAGIC_ORB_STAGGER_76       10500ms     4633ms    2.27x
VALTAN_CENTER_GRAB_COUNTER_64      6050ms     7433ms    0.81x
VALTAN_ARENA_BREAK_33              6100ms     3400ms    1.79x
VALTAN_GHOST_TRANSITION_15         9500ms    12067ms    0.79x
```

```text
애니가 잘린다 <0.9x            23개
시간이 남는다 >1.1x             7개
거의 맞음 0.9~1.1x            2개
바인딩 없음                    1개
```
---

## 8. 네가 정해야 하는 것

이 여섯 개는 자료가 답을 안 주거나 자료끼리 어긋나는 자리다. 지어내지 않고 남겨 둔다.

### 8.1 척도 — 결정 불필요, 선례가 있다

원본 cm 를 100 으로 나눠 그대로 쓰면 된다. 플레이어 쪽이 이미 그렇게 하고 있다.

```text
원본 34010 긴 창 평타     부채꼴 180cm 230도  밀치기 35cm
Data/Animation/HitShapes/LanceMaster.hitshapes.json
                          areaType 3  range 1.8  angle 230  pushRange 0.35
```

발탄 휘두르기 4.0m 는 이 평타 1.8m 의 2.2배다. 보스 크기로 납득이 간다.
`BOSS_VALTAN` 의 `collisionRadius` 가 3.0m 이므로 몸에서 1m 나가는 부채꼴이 된다.
지금 저작된 8.0m 는 몸에서 5m 나간다.

### 8.2 180도 넘는 부채꼴 두 개

encounter publisher 는 `CONE` 을 180도 이하로만 받는다
(`Tools/GameplayPipeline/Publish-GameplayBalance.ps1:1043`). 그런데 플레이어 hitshape
쪽은 230도와 360도를 그대로 저장한다. 즉 **encounter 스키마가 플레이어 스키마보다
표현력이 낮다.**

크기가 지정된 원본 부채꼴 81개를 세어 보면 이 규칙이 실제로 막는 것은 둘뿐이다.

```text
180 이하        51개   그대로 CONE 으로 들어간다
360도           28개   전부 arem 이 있는 링이다. RING 이 그대로 담는다
180 초과 360 미만 2개
   420606 큰 베기 (반격기)        반경 4.5m 각 220도   ← VALTAN_PARRY
   420623 돌진 잡기 후 사자후      반경 2.8m 각 225도   ← VALTAN_CHARGE_GRAB_ROAR
```

두 갈래다.

```text
가  CIRCLE 반경 4.5m / 2.8m 로 근사한다
    뒤쪽 140도와 135도가 판정에 더 들어간다. 저작 한 줄로 끝난다
나  publisher 의 CONE 상한을 360 으로 올리고 Server 판정도 맞춘다
    원본대로 들어가지만 publisher / Shared XZ primitive / 계약 테스트를 함께 손댄다
```

### 8.3 원본이 크기를 아예 안 준 6개 패턴

```text
VALTAN_BACKSTEP_ATTACK    420635 shapes=0
VALTAN_RED_BLADE_WAVE     420636 shapes=1 인데 area=0
VALTAN_LEDGE_ROAR         420639 shapes=0
VALTAN_ARENA_BREAK_109    420629
VALTAN_ARENA_BREAK_84
VALTAN_ARENA_BREAK_33
```

앞의 셋은 원본에 판정 도형이 실제로 없다. 뒤의 셋은 지형 파괴 연출이라 원래 타격
스킬이 아닐 가능성이 크다. 그렇다면 **판정을 아예 빼는 것**도 답이다. 지금은
`ARENA_BREAK_109` 와 `ARENA_BREAK_33` 이 원 반경 12m 판정을 들고 있다.

### 8.4 스테이지 시간을 clip 길이에 맞출 것인가

7.1 표대로 지금은 33개 중 23개가 clip 보다 짧다. 원본 clip 길이에 맞추면 대부분의
패턴이 2~3배 길어진다. 휘두르기는 1.9초에서 5.5초가 된다.

```text
가  clip 에 맞춘다      애니메이션이 안 잘린다. 전투 템포가 크게 느려진다
나  지금 템포를 지킨다   빠른데 애니메이션이 중간에 끊긴다
다  타격 순간만 맞춘다   원본 HIT notify 시각(휘두르기 1570ms)에 ACTIVE 가 오도록
                       WINDUP 을 늘리고, RECOVERY 는 짧게 잘라 템포를 지킨다
```

`다` 가 절충이지만 그래도 휘두르기 WINDUP 이 450ms 에서 1570ms 가 된다.

### 8.5 109줄 페이즈 경계의 근거 재기록

1장 끝에 적은 대로 `2026-08-20_..._ARMOR_SERVER_STATE_RESULT.md` 479줄의 선배 인용이
보관 원문에 없다. `phaseTwoHpPercent 68` 값 자체는 encounter 트리거와 네 관찰이
받쳐 주므로 **값은 그대로 두고 근거 문장만 고치면 된다.** 고칠지 여부는 네 결정이다.

### 8.6 선배 규칙 중 미구현 5개를 이번 범위에 넣을 것인가

```text
무력화 게이지            선배 97·101   Server 상태 + Shared 복제 + HUD 가 필요하다
에스더 카운터·갑옷 충전   선배 126      지금은 시간 충전만 있다
에스더 3종 로스터        선배 128      바훈투르·웨이 에셋 미쿠킹으로 슬롯 거부 중
바훈투르 20초 무시 버프   선배 142      없어서 130줄 전멸기에 공략 수단이 없다
하늘 도끼 추적 오브젝트   선배 124      VALTAN_HIGH_JUMP 에 추적 객체가 없다
```

전부 패턴 저작이 아니라 시스템 작업이다. 패턴 33개를 먼저 돌 것인지, 이 중 하나를
먼저 닫을 것인지가 갈린다.

---

## 9. 저작 사이클 — 패턴 하나를 끝낼 때 실제로 도는 것

```text
1  이 문서의 해당 카드를 읽는다
2  Data/Encounters/Valtan/ValtanEncounter.json 의 그 패턴만 고친다
3  필요하면 Data/Animation/Authored/Valtan/Valtan.patternbindings.json
   Data/Balance/DamageProfiles.json 을 같이 고친다
4  powershell -File Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1
   receipt 를 동기화한다. 패턴 하나당 17 entry 이고 17번째 patterns[i].stages 가
   스테이지 배열 전체를 한 값으로 담으므로 손으로 고치지 않는다
5  powershell -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
6  powershell -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
7  Server\Bin\Debug\Server.exe --contract-test        failures : 0
```

6번을 빼면 안 된다. 계약 테스트 일부가 합성 픽스처가 아니라 실제 published
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` 을 읽는다
(`ServerGameplayContractTests.cpp:2438` 의 갑옷 판 로드 계약이 그렇다).

`PATTERN` 행에 **필드를 새로 추가할 때만** 아래가 추가로 필요하다. 값만 바꾸면 해당 없다.

```text
Tools/GameplayPipeline/Publish-GameplayBalance.ps1     행 발행과 검증
Tools/WorldPipeline/Publish-WorldGameplay.ps1          같은 문서 검증
Client/Private/EncounterPatternReference.cpp           Effect cue join
Client/Private/BalanceTool.cpp / .h                    읽기·검증·저장 왕복
Server/Private/ServerGameplayContractTests.cpp         PATTERN 행을 직접 찍는
                                                       합성 픽스처 4개의 행 폭
```

이 세션 시작 시점 기준선은 녹색이었다.

```text
Publish-GameplayBalance -Mode Validate
  6 player profiles, 136 skills, 108 damage profiles, 1 boss, 2 armour plates,
  33 boss patterns, 129 pattern stages, 67 Valtan Debug audition occurrences
Server\Bin\Debug\Server.exe --contract-test   failures : 0
```

---

## 10. 이 문서를 다시 만드는 법

5·6·7장은 손으로 쓰지 않고 저장소에서 생성했다. 데이터가 바뀌면 다시 돌린다.
생성 스크립트는 세션 scratchpad 에 있고 입력은 전부 Git 추적 파일이다.

```text
입력  Data/Encounters/Valtan/ValtanEncounter.json
      Data/Encounters/Valtan/ValtanDebugAudition.json
      Data/Animation/Authored/Valtan/Valtan.patternpreview.json
      Data/Animation/Authored/Valtan/Valtan.patternbindings.json
      Data/Balance/DamageProfiles.json
      Data/Animation/Reference/Valtan/Valtan.{clipseq,skilltiming,animnotify,animevents}
```

카드에 들어간 값 중 지어낸 것은 없다. 셋만 추정이라고 명시했다.

```text
hittype 코드의 이름 뜻      필드 서명은 실측, 뜻은 추정 (2.2)
seq 의 의미                발탄에서 확인되지 않음. 가장 낮은 seq 를 실었다 (2.5)
어느 도형이 대표 판정인가    표는 정하지 않는다. clip 타임라인이 순간별로 보여준다 (7)
```
