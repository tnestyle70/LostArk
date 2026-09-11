# 쿠크세이튼 패턴·이펙트·컷씬 full restore 전수조사

조사일: 2026-09-11. 기준은 `codex/kouku-full-material-lighting-restoration`의
HEAD `e26cd2b282293bd4cb433338bdbc6a7315dd3f84`와 현재 미커밋 변경이다.
git fetch 뒤 HEAD와 origin/main은 같은 commit이었다. 코드·게임 저작 데이터·Resources는
변경하지 않았으며 이 조사 문서와 목록만 추가했다. 실행 중 화면이나 원작 영상은 검증하지 않았다.

## G00. 조사 결론과 자료 범위

**원본 패턴 내부의 이펙트 정보는 실제로 있고, 그 정보를 추출할 수 있다.**
사용자가 원본을 보고 연결한 현재 애니메이션을 버릴 필요가 없다. 그 연결에 대응하는 원본
action을 찾아 발생 시각·ParticleSystem·emitter·재질·mesh·분포를 가져오는 것이 가능하다.
이번 조사에서 실제 action/notify/ParticleSystem 목록을 CSV로 추출했으며 단순한 파일명 검색이 아니다.

| 사용자가 알고 싶은 범위 | 이번 확인 결과 |
|---|---|
| 패턴 내부에 어떤 effect를 쓰는지 있는가 | 있음. action stage의 PlayParticleEffect가 실제 ParticleSystem 경로를 지정 |
| 언제 나타나고 얼마 동안 유지하는지 있는가 | 있음. notify local time/duration과 emitter delay/duration/loop/burst/Lifetime 정보. 일부 생략·상속·전이 의미는 추가 해독 필요 |
| 그 effect 내부의 구성도 뽑히는가 | 있음. source graph에서 emitter/LOD/module, mesh/material 참조와 분포를 읽을 수 있음 |
| 현재 엔진에서 원본 그대로 즉시 Play 가능한가 | 아직 별도 연결 필요. 추출된 원본 정보와 현재 renderer의 재질·입력·부착·시간 소비를 연결해야 함 |
| 빠진 정보 때문에 전부 수작업으로 돌아가야 하는가 | 그럴 필요 없음. 직접 source가 있는 부분은 복구하고 미해독 부분만 수작업/추가 해독 대상으로 분리 가능 |

**캐릭터 Q/W/E처럼 패턴별 full restore를 구성할 수 있다. 다만 쿠크의 원본 연결 단위는
애니메이션 클립 하나가 아니라 `profile + actionId + stage + notify occurrence`다.**
동일한 클립에 서로 다른 particle, 재질 변화, 사운드, 소환·판정이 붙는 경우를 보존해야 한다.

현재 원본 4개 profile에는 349개 action이 있다. 이는 짤패턴 349종을 뜻하지 않는다.
기본 상태, 성공/실패 분기, 반복 구간, 리허설·헬·싱글 변형과 주요 기믹이 함께 포함돼 있다.
현재 Composition revision 322는 28개 저장 패턴이고, 게시된 실행 대상은 24개 패턴·195 stage·7 bundle이다.
24개는 모두 `selectionMode=AUDITION_ONLY`, `selectionWeight=0`이므로 Server 권위의 수동
패턴 실행 연결과 자동 레이드 진행·랜덤 짤패턴 선택의 완성 상태를 구분한다.

전수조사의 분모는 이 PC에 보존된 추출본과 저장소 현재 데이터다. 원작의 모든 패치 버전이나
아직 추출되지 않은 Skill/SkillEffect/NPC 정의까지 확보했다는 뜻은 아니다.
원본 profile 이름만으로 관문을 확정하지 않았다. 현재 Gate1은 RPCT05 세이튼을 쓰며,
다른 profile의 action도 차용한다. 과거 09-05 조사에서 profile과 관문을 직접 묶은 부분은
현재 저작 관문과 구분해서 읽어야 한다.

원본 349 action의 4,072 stage와 notify 30,980행을 전수 목록화했다. 직접 참조 고유
ParticleSystem 327개는 모두 기존 source graph에서 찾았다. RPCT/RPCZ 9패키지의 989개와
직접 참조 shared 19개를 합친 넓은 후보 목록은 1,008개다. 327개는 직접 소비 참조,
1,008개는 미참조·locale 변형까지 포함한 추출 목록이며 모두 runtime 복원됐다는 뜻은 아니다.
현재 Composition에서 직접 sourceAction 참조를 가진 것은 31/349개다. 이는 clip 또는 stage
차용 여부이며 31개 원본 패턴의 full restore 완료 수가 아니다.

별도로 현재 추출기가 명시 object reference를 얻지 못한 notify는221건이다
(PlayDecalEffect144, DefaultParticle60, PlayParticleEffect17). 이는 원본에 효과가 없다는
증거가 아니라 현재 decode 결과의 미해결 항목이다. 또한 SkillEffect가 만드는 projectile·소환체의
전체 gameplay 정의는 확보되지 않았다. `327/327 graph 경로 확인`을 전체 효과100% 복원으로
확대하지 않는다.

| 상세 자료 | 내용 |
|---|---|
| [현재 패턴 전수표](2026-09-11_KOUKU_PATTERN_RUNTIME_INVENTORY.md) | 저장 패턴, 실제 clip, Effect/WORLD/Logic, 게시·수동 실행 상태 |
| [원본 action·effect 전수표](2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY.md) | 원본 profile/action/stage/notify, particle graph 및 현재 연결 대조 |
| [컷씬 actor·재질 조사](2026-09-11_KOUKU_SEQUENCE_ACTOR_MATERIAL_AUDIT.md) | 독립 시퀀서의 두 연출, 모델·재질·clip·종료 경로 |
| [수명 실측 JSON](2026-09-11_KOUKU_FULL_RESTORE_LIFETIME_AUDIT.json) | 현재 캐릭터 105개 asset cue, Alt+V late burst, 쿠크 원본 독립 시간 사례 |
| [원본 action CSV](2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY_ACTIONS.csv) | 349행. 이름·clip·ParticleSystem·sound·현재 패턴 연결 |
| [원본 notify CSV](2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY_NOTIFIES.csv) | 30,980행. 발생 시각·수명·소스 위치·참조·부착 후보 |
| [원본 ParticleSystem CSV](2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY_PARTICLES.csv) | 1,008행. emitter·module·material·mesh 및 참조 action |
| [휠윈드 원본 추출 예시](2026-09-11_KOUKU_WHIRLWIND_SOURCE_EXTRACTION_EXAMPLE.md) | 실제 action의 clip·발생 시각·ParticleSystem 및 내부 emitter 정보 |

현재 연결을 관문별로 요약하면 다음과 같다. `G1`이라는 저장 ID 접두사나 `Gate1/` 폴더 대신
각 패턴의 실제 `gateId`를 사용했다.

| 현재 관문 | 저장 / 게시 | 현재 패턴과 표현 | 남은 주요 경계 |
|---|---:|---|---|
| 1관문 | 7 / 6 | 무력화 시작·성공, 진짜/가짜 세이튼 찾기, 댄스타임, 룰렛. 직접 Effect 45회 | 무력화 실패 P3은 빈 stage. 기존 V2 수동 조립과 원본 full 복원을 구분 |
| 2관문 | 19 / 16 | 쿠크·대형 세이튼 등장, 파1빨2, 조커찾기·성공, 거미카운터, 잡기, 피자, 레이저, 그로기, 팡파레, 휠윈드, 불뿜기, 카드미로연출. 직접 Effect 40회 | 대형 세이튼 거미 P16·레이저 P20·피자 P26은 빈 stage. 짤패턴 여러 개의 원본 Effect가 미연결 |
| 3관문 | 2 / 2 | 외곽불·갈고리 시각테스트와 갈고리만 확인용. Idle clip 반복 + WORLD 표현 | 본체 원본 공격 action/Effect 복원이 아니라 현재 테스트 구성. 마리오·쇼타임 등의 전체 encounter 연결과 구분 |
| 빙고 | 0 / 0 | 별도 Server Debug의 판·폭탄·망치 및 snapshot → V2 5group 표현 | Composition 보스 패턴 없음. 안전 판정·피해·무력화와 보스 animation을 통합한 원본 encounter는 미완성 |

총 직접 Effect box는 85개, 고유 asset ID는 28개다. 직접 box가 0이어도 WORLD 소품,
FEAR 결과, NPC clip binding, 카드 상태·미로·빙고 전용 표현은 별도로 존재할 수 있다.
현재 복원률을 `28 / 원본 particle 수`처럼 계산하면 안 된다. 두 값은 집계 단위가 다르다.

## G01. 원본에서 패턴과 이펙트가 묶이는 방식

```text
Action LOA: profile + actionId
  └─ stage (분기·반복·전이 정보를 가진 액션 구간)
      ├─ Anim notify → clip 이름·시작·길이
      ├─ PlayParticleEffect notify → ParticleSystem·발생 시각·유지·부착
      │   └─ emitter → LOD·module·분포·mesh·material
      │       └─ MIC/parent/static switch → shader·texture·sampler·uniform
      ├─ PawnMaterialParam / HidePawn / TrailGhostEffect
      ├─ AKEvent / ViewShake / 카메라·조명
      └─ Effect / Trigger / 소환·상태·조건: 별도 gameplay 의미
```

`Effect`라는 원본 notify는 숫자 skill-effect ID일 수 있다. 이를 모두 화면의 particle로 세면
패턴별 이펙트 목록이 틀어진다. ParticleSystem도 실제 입자 한 개가 아니고 여러 emitter의 묶음이다.
같은 particle 이름을 여러 번 호출하면 각 발생 시각·부착 대상·배율을 따로 보존해야 한다.

현재 모델에 clip이 있다는 사실은 그 액션의 particle notify도 자동 재생된다는 뜻이 아니다.
원본 action reference는 참고 정본이고, 제품은 Composition의 animation stage와 별도로 저장된
presentation occurrence를 소비한다. 현재 연결은 상세 전수표에서 직접 참조와 clip 공유 후보를
구분했다. action ID가 다르다는 이유만으로 새 애니메이션을 만들 필요도 없다.

이름도 복원 기준으로 단독 사용하면 안 된다. 현재 `쿠크_팡파레` P23이 차용한 4219714의
원본 이름은 `쿠크_나팔_액션_피자`이고, 현재 `쿠크_피자` P25는 4219769의
`쿠크_대폭발_반시계_시계_반시계`를 차용했다. 현재 저작을 잘못됐다고 판정한 것이 아니라,
표시 이름과 원본 action ID·기믹 의미가 1:1이 아니라는 직접 사례다. 기존 사용자 조립본의
기능을 보존하면서 원본 대응을 구분해야 한다.

### 발탄에서도 같은 정보가 있는지 대표 대조

발탄도 원본 action의 이펙트 발생 정보가 보존돼 있다.
[Valtan.actionbindings.json](C:/Users/user/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.actionbindings.json:33)의
`MN_RPBF_00 / 420601 / 레이드 발탄_휘두르기 / stage0 / Att_Battle_1_01`에는 다음 정보가 있다.

| stage-local 시간 | 원본 notify | 대상 |
|---:|---|---|
| 1.0초 | PlayParticleEffect | FX_MN_RPBF_00_S.Par_S_RPBF_Atk_02_1 |
| 1.49초, 0.28초 유지 | Trails | FX_BS_01.Trail.Par_O_BOEH_Trail_01_1 |
| 1.49초, 0.5초 유지 | PlayParticleEffect | FX_CM_02.Light.Par_MP_Light_05_L |
| 1.55초 | PlayParticleEffect | FX_CM_00.Dust.Par_D_Dust_002_pr |

발탄의 현재 일부 carrier는 수동 조립도 포함한다. 원본 정보가 없어서 반드시 수작업만 해야
한다는 결론과는 다르다. 발탄은 이번에 이 보존 notify 추출본을 직접 확인했으며, 과거 receipt에
적힌 일부 외부 원본 경로는 현재 존재하지 않아 emitter 전체 확보까지 확인했다고 쓰지 않는다.
쿠크는 이번에 현 PC의 내부 particle graph까지 직접 찾고 목록화했다는 차이가 있다.

## G02. 시퀀서의 세이튼과 컷씬 종료

독립 Composition Sequencer의 실제 두 항목은 `연출_팝업북` 37,800ms와
`연출_1관문 피날레` 21,010ms다. 둘 다 독립 Animation lane은 0개이며
WORLD occurrence가 모델과 애니메이션 재생을 소유한다.

팝업북의 세이튼은 Deploy placement5의 `DEPLOY_BOSS_MN_RPCT_00`이고,
전투 Gate1의 세이튼은 CNpc의 `MN_RPCT_05`다. **서로 다른 actor/model 경로라는 기억은 맞다.**
다만 최신 소스에는 컷씬 Deploy 경로에도 BossCatalog의 native material override를 전달하는
수정이 이미 들어 있다. 따라서 현재 소스 기준으로 컷씬 세이튼이 반드시 미복원 재질이라고
단정할 수 없다. 실제 실행 EXE와 화면 확인은 별도다.

현재 두 WModel은 각각 249 clip을 갖고, 팝업북이 요구하는 12 animation track의 clip도
전투 모델05에 존재한다. 재사용 가능성이 높은 근거지만 node 이름·일부 key 값까지 완전히
같은 바이너리는 아니다. 두 모델의 embedded animation eventCount는 모두0이어서 해당
clip을 재생하는 것만으로 원본 Action의 Effect가 자동 생성되지 않는다.

또한 팝업북 Deploy actor는 현재 z=737.629, Gate1 Server 세이튼 배치는 z=942.330017로
약204.7m 떨어져 있고 Server 배치는 disabled template이다. 기존 팝업북 trigger의
`2-1Stage_Move`와 현재 F1 Gate1 admission을 같은 진입 사건이라고 단정할 근거도 없다.
사용자가 원하는 진입 경로로 연결하는 작업에는 actor admission·위치·카메라 소유권까지 포함된다.

현재 종료는 컷씬 prop을 정리하는 구조이며 해당 객체를 전투 actor로 인계하는 구현은 없다.
향후 목표는 다음 두 계약으로 나누는 것이 정확하다.

1. 컷씬과 전투가 같은 재질 복원 규칙을 소비하도록 한다. 정확한 원본 재질·slot이 다르면
   모델 파일을 같게 만드는 대신 각각의 올바른 descriptor를 같은 CModel/CMaterial에서 사용한다.
2. 진입 연출이 끝나는 시점에 이미 준비된 Server 전투 actor의 표현을 이어 준다. 컷씬 마지막
   pose/위치와 전투 actor의 Server-approved pose, visibility, 카메라 복귀를 명시적으로 맞춘다.
   Deploy 객체를 그대로 전투 객체로 바꾸거나 Client에서 전투 entity를 임의 생성하지 않는다.

실제 전투 actor를 컷씬 표현 대상으로 잠시 사용할 경우에는 그 actor의 action/animation·visibility
소유권과 종료/취소 시 복귀를 기존 sequence 경로에 연결해야 한다. 단순 modelAssetId 교체만으로
RPCZ/RPCT clip 이름·뼈·weapon·root 변환·원본 시간 대응이 성립하지 않는다.

## G03. 애니메이션 수명 때문에 이펙트가 빠졌다는 가설

**일부 누락을 설명하는 유효한 가설이다. 그러나 전체 원인을 애니메이션 길이로 묶을 수는 없다.**
현재 데이터에서 다음 세 종류를 직접 구분할 수 있다.

| 시간 | 소유자 | 잘못 맞추면 생기는 일 |
|---|---|---|
| clip의 원본 재생 시간 | animation clip, source slice, playRate | slice 밖 notify 미발생, 반복·전환 누락 |
| action/stage의 실제 유지 시간 | Server stage와 presentation timeline | 짧은 clip의 loop/hold, 판정 창·이펙트 유지 누락 |
| emitter 방출 시간 | source Required duration·delay·loop·burst | 늦은 burst가 생성되지 않음 |
| 이미 생성된 입자의 잔여 수명 | particle Lifetime, ribbon/trail tail | 방출 중단 때 잔상까지 갑자기 사라짐 |
| occurrence 정리 시각 | NATURAL / CUE_END / 외부 시퀀서 box / owner | 패턴 종료·재시작·퇴장 때 정리 범위가 달라짐 |

### 현재 캐릭터 연결의 실측

`Data/Animation/Authored/{Artist,DimensionMaster,Warlord,LanceMaster}/*.animevents`에서
실제 `effectref=asset` EFFECT 행을 셌다. 참고 원본 notify나 단순 파일 개수를 세지 않았다.

| 클래스 | asset cue | NATURAL | CUE_END |
|---|---:|---:|---:|
| Artist | 18 | 17 | 1 |
| DimensionMaster | 15 | 15 | 0 |
| Warlord | 26 | 26 | 0 |
| LanceMaster | 46 | 46 | 0 |
| 합계 | 105 | 104 | 1 |

NATURAL은 이미 생성된 effect 자체의 종료를 소비한다. 캐릭터 clip이 끝났다는 이유만으로 모두 종료하지 않는다.
유일한 CUE_END는 Artist31930의 6,000ms다. 이는 500초짜리 원본 emitter 설정 때문에
빈 effect와 예약량이 오래 남아 재사용을 막던 문제를 제한한 명시적 action 수명이다.
이 경계를 모든 스킬에 복사해서는 안 된다.

현재 `CCharacter::Update_EffectCues`는 실제 stage/clip/sourceStartMs/playMs/playRate로 cue의
발생 가능 구간을 계산한다. 잘못된 clip 연결·너무 짧은 source slice는 발생 자체를 막을 수 있다.
카메라는 현재 action인지를 추가 확인하므로 particle의 NATURAL 수명과 카메라 복귀도 구분한다.
이미 prewarm 대기열에 들어간 일반 pending spawn 전체가 action 전환마다 취소되는 것도 아니다.
아직 발생하지 않은 cue의 유효 clip 구간, pending 준비, 실제 생성된 effect의 종료는 서로 다른 단계다.

근거: `Client/Private/Character.cpp:465`, `Client/Private/ActionPresentationTimeline.cpp:220`,
`Client/Private/Effect_PresentationService.cpp:4895`, `:4941`, `:5062`.

### 현재 남아 있는 구체적인 방출 시간 사례

차원술사 `effect.dimensionmaster.skill.2050540.full.restore`의 보이는 원본 recipe 중
12개 element에 방출 가능 구간 뒤의 burst 행 36개가 있다. 8개 element는 모든 burst가
구간 뒤에 있고, 나머지 4개는 앞쪽 burst와 늦은 burst가 함께 있다.

- source `emitterDurationSeconds=0`, `emitterLoopCount=1`.
- fallback `detail.timing.lifeTimeSeconds=0.100000001`.
- 실제 늦은 burst는 0.15~0.9초에 위치한다.
- `par_m_swp_tw_exp_01.particlespriteemitter_40`은 0.3초 단발이라 현재 방출 구간에 도달하지 못한다.
- `particlespriteemitter_7/9/10`은 0초 burst는 가능하지만 0.15~0.9초 후속 burst가 남는다.

이는 현재 JSON과 `Effect_Playback.cpp:3888`의 방출 분기를 대조한 정적 증거다.
이번 조사에서 GPU 발생이나 화면은 실행하지 않았다. 클립을 늘리는 조치보다 source의 누락된
Required/CDO·archetype duration을 회수하고 실제 burst/loop와 대조하는 일이 먼저다.
기존 importer도 `max(EmitterDuration, notify duration, 0.1)`을 `APPROXIMATION`으로 기록한다
(`Tools/LevelPlacementExtractor/build_imported_effect_documents.py:663`).

### 쿠크 원본이 보여 주는 독립 시간

`MN_RPCZ_00 / action42197100 / stage1`에서 `Att_Battle_14_02`의 원본 길이는 0.666667초다.
같은 구간의 `Par_U_RPCZ_EnergyBalloon_01_LOC_INT`와 PawnMaterialParam은 2.6초이고,
Paralyzation notify는 2.4초에도 있다. 즉 **모든 발생·유지를 clip 길이로 자르면 원본 데이터와
맞지 않는다.** 정확한 원작 stage 전이·loop/hold 정책은 원본 전이값 해독이 미완료라 확정하지 않았다.

현재 쿠크는 `KoukuSaydonPresentationPlayer.cpp:1057`의 box `[start, start+duration)` 밖에서
effect를 정리하고 `:1295`에서 V1 Stop_WorldRoot/V2 Stop_Group을 호출한다.
particle leaf/group은 내부 emitter 시간을 box 길이로 늘리지 않도록 `fDurationSeconds=-1`로
보존하는 코드가 이미 있다(`:1186`, `:1199`). 다만 box 자체가 짧으면 마지막 tail은 잘릴 수 있다.
각 패턴의 원본 발생 끝·tail·중단 정책과 box 끝을 대조해야 실제 결함을 확정할 수 있다.

### 수명 외에 확인된 누락 원인

최근 RESULT와 현재 코드에는 shader 입력/재질 프로그램 미연결, UniformRange 축소,
카메라·static/model notify 누락, SourceScale·bone basis 중복, SceneColor 복사 시점,
animevents 헤더 행 수 불일치 등 서로 다른 원인이 있다. 이들은 clip 길이를 늘려 해결되지 않는다.
full이라는 파일명도 모든 원본 occurrence가 연결됐거나 화면 검증이 끝났다는 표식이 아니다.

## G04. 짤패턴 full restore에 적용할 복원 방식

원본 일반기·이동 후보군은 다음과 같다. profile 기준의 원본 그룹이며 실제 관문 사용 여부는
현재 gateId 또는 추가 gameplay 근거로 확인한다. 조사 편의 NORMAL 분류는 총144 action이고,
그 안에도 이동·조롱·변형이 있으므로 독립 짤패턴144종이라는 뜻은 아니다.

| 원본 actor | 일반기·이동 후보 | 현재 sourceAction 차용 |
|---|---|---|
| 쿠크 RPCZ00 | 바주카12, 뿅망치8, 나팔7, 저글링·융단폭격·저주받은 인형·슈퍼 바주카·훌라후프10 | 각각 1/1/1/1 action. 원본 피자4219714는 별도 기믹 분류 |
| 세이튼 RPCT05 | 불뿜기·종이비둘기·서커스공·아드레날린 등 쇼/무기18, 화염·근접·이동26 | 해당 일반기 그룹의 직접 sourceAction 차용0 |
| 대형 세이튼 RPCT06 | 불뿜기·바람불기·3연타·공포·기모아 내려찍기·종이비둘기12 | 불뿜기4221809·공포4221813만 차용 |
| 합체 RPCT07 | 재사용 불·비둘기·랜덤박스·메두사·레이저 등24, 무기뇌격·공·불길·표식·폭탄14, 이동·조롱13 | 해당 일반기 그룹의 직접 sourceAction 차용0 |

각 그룹의 모든 action ID·실제 이름·clip·직접 ParticleSystem은 원본 전수표/CSV에 있다.
빙고는 원본 오프닝4219905·폭탄소환4219984를 직접 특정했고, 원기옥4219927·인터셉트
테스트4219987은 조사 후보로 구분했다. 일반기를 임의로 빙고 전용 패턴으로 배정하지 않았다.

현재 이미 선택·재생할 수 있는 짤패턴부터 원본 표현을 붙이면 비교가 쉽다.

| 우선 대상 | 현재 상태 | 직접 대응 원본 |
|---|---|---|
| P24 쿠크 휠윈드 | clip15·망치 WORLD1, 직접 Effect0 | action4219708: particle notify22회/고유 system6. Hammer_B·먼지·무기 출현/유지/퇴장 |
| P23 쿠크 팡파레 | clip4·나팔 WORLD1, 직접 Effect0 | action4219714: particle notify18회/고유 system4. Trumpet_C·무기 출현/유지/퇴장. 원본 이름은 나팔 피자 |
| P21 쿠크 레이저 | clip3·대포 WORLD1, 직접 Effect0 | action4219740: particle notify17회/고유 system4. Bazooka_Fire·Jump_dust·무기 유지/퇴장. 원본 이름은 바주카 액션1 |
| P27 대형 세이튼 불뿜기 | clip4, 직접 Effect/WORLD/Logic0 | action4221809: particle notify14회/고유 system7. FireCast·FireBreath·FireDc·광원·ZoomBlur |

이 표의 notify 수는 원본 action 전체다. 현재 선택한 stage만의 분모나 동시에 생성되는 입자
수는 아니다. 무기 particle을 현재 WORLD 소품과 함께 복구할 때에는 동일 무기가 두 번
표시되지 않도록 어떤 표현이 원본 무기를 소유할지 정한다. 남아 있는 MN_RPCT06 legacy
effectv2 clip binding2개도 Composition 효과와 중복 실행되는지 함께 대조한다.

사용자가 선택하는 단위는 `관문 / 패턴명 / 변형`으로 두고, 그 안에 action·stage·clip과
모든 effect occurrence가 펼쳐지게 하는 방식이 적합하다. 캐릭터의 `Q/W/E` 자리에 안정적인
pattern ID가 들어가며, 기존 Kouku Action Workbench와 Composition Sequencer를 계속 사용한다.

| 순서 | 기존 경로에 연결할 내용 | 해당 단위에서 확인할 결과 |
|---|---|---|
| G1 | 현재 authored pattern과 원본 profile/action/분기를 확정 | 이름이 비슷한 clip이나 다른 관문의 action을 잘못 합치지 않음 |
| G2 | 원본 notify별 particle/system/emitter와 실제 CDO·MIC·mesh·texture 회수 | source occurrence마다 연결·의도적 제외·미해독 사유가 남음 |
| G3 | 기존 V1 source full restore 문서와 CModel model cue에 재질·운동·부착 연결 | 이미 복원한 프로그램/renderer 재사용, 새 계산·입력만 보강 |
| G4 | 기존 Composition의 V1_EFFECT/V1_ELEMENT resource와 animation/WORLD/SOUND/CAMERA lane 연결 | 같은 action clock에서 spawn·seek·repeat·stop과 tail이 맞음 |
| G5 | 패턴의 gameplay 필요 부분을 현재 Shared/Server Logic과 publisher로 연결 | Server 승인·판정·상태와 Client 표현이 같은 실행을 소비 |
| G6 | 최소 컴파일·해당 데이터 검증 후 사용자 재생 | 각 패턴 Play, 완료 직후 재생, 중단·실패 시 기존 actor/카메라 복귀 |

G1~G6은 이번에 구현한 기능이 아니라 전수조사에 따른 작업 순서다. 별도 importer/runtime/validator를
먼저 만드는 작업을 완료 조건으로 두지 않는다. 기존 V2 수동 조립본은 사용자가 편집한 비교본으로
보존하고, 원본 full restore와 이름·ID로 구분한다. 실제 지원하지 않는 branch/notify를 조용히
버리거나 원본과 동일하다고 표시하지 않는다.

현재 V1_ELEMENT 재생은 ModelCue 렌더를 제외한다. 원본 소환 모델이 필요한 복구본은
V1_EFFECT 전체 또는 기존 WORLD 모델 표현으로 연결하고, V1_ELEMENT는 선택 particle 등
개별 요소 재생에 사용한다. 동일 full 문서를 두 경로로 중복 생성하지 않는다.

원본에 별도 projectile/object가 필요한 동작은 animation-only effect로 축약하지 않는다.
원본 projectile gameplay 정의가 없으면 확보한 시각·시간 자료와 프로젝트가 저작해야 할
Server 값의 경계를 명시한다. 복원할 pattern 하나의 실제 consumer까지 연결한 뒤 다음 패턴으로
확대한다. 여러 스킬·패턴이 쓰는 동일 MIC/program은 공유하되 occurrence는 합치지 않는다.

## G05. 수행한 검증과 남은 경계

이번 작업은 조사와 문서 작성이다. Client/Server 실행, UI 조작, 캡처, 빌드, publisher 실행,
재질·이펙트·시퀀스 데이터 변경은 수행하지 않았다. 지난 RESULT의 build/PASS를 이번 검증으로
옮겨 기록하지 않았다. 현 dirty worktree의 코드는 최신 실행 EXE와 같다는 보장이 없으므로
실행 중 모습은 이 정적 조사만으로 판정하지 않는다.

LAN 동기화는 `server-host`, 방화벽 TCP7777 LocalSubnet ready,
endpoint `192.168.0.14:7777` not-listening이었다. 사용자 실행 대상은 Visual Studio
`Server + Client` profile의 Ctrl+F5다. 이번 요청은 우선 조사이므로 실행을 요구하지 않는다.

원본 조사·현재 연결표·수명 정적 대조를 완료한 것과 full restore 구현·사용자 시각 확인은 별개다.
후자는 아직 이 작업에서 수행하지 않았다.
