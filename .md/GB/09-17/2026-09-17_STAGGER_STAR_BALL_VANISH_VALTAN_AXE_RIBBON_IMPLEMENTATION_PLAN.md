# 세이튼 무력화 별·외곽 경계 V1, 공_튀기기 사라지기 교체, 발탄 도끼 리본 구현 계획서

## G00. 목표와 종료 증거

세 가지 요청을 한 변경 단위로 연결한다.

1. 세이튼 무력화(원본 Action 4219945) 이펙트를 원본 particle system의 움직임으로 역할을 확정하고
   `세이튼 / 무력화 | …` V1 문서 네 개(별 그리기·별 폭발, 외곽 경계, 둘의 합본, 레이저 생성·변화)로 만든다.
2. World Object `공_튀기기`에 `알비온_사라지기이펙트`를 넣으면 `b_root`가 없다고 거절되는 결함을 고치고,
   시작 위치와 MOTION_END에서 사라지기 이펙트가 재생되도록 저작 데이터를 교체한다.
3. 발탄 4연속 공격 Full Restore(stage008/009)의 도끼 trail/ribbon을 저주의식 지팡이 리본과 같은
   SpawnPerUnit 거리 생성 방식으로 다시 복원하고, 버러지 패턴의 왼손 sprite particle 세트를 도끼 socket에
   붙여 tick마다 생성·공전하게 하여 사용자가 두 표현을 비교할 수 있게 한다.

종료 증거는 변경 CPP/FX의 격리 컴파일, 변경·신규 JSON의 codec Load/Validate/Stage/Roundtrip과 15초 playback,
anchor·track 기하 probe 수치, `Validate-EffectSources.ps1`, `git diff --check`다. 화면 판정은 사용자가 한다.

## G01. 현재 실측

### 무력화 원본 (MN_RPCT_07 4219945, particle graph FX_MN_RPCT_05_L)

| stage | clip | notify | system | 역할 (모듈 근거) |
| --- | --- | --- | --- | --- |
| 2~4 | Att_Battle_6_04 2.267s | 0.609 / 0.624 / 2.037 | Sk_12 / Sk_12_1 / Sk_01_2 | 15m 수직 레이저 기둥 + 7m 상단 flare, 6m 지면 충격 링, 잔여 소형 히트·바닥 decal |
| 5 | Att_Battle_6_02 (particle 창 8.1s) | 0 | Sk_12_2 | 반경 8.25m 오망성 꼭짓점 카드 5장(8s) + 1~5초 꼭짓점 섬광 |
| 5 | | 0 | Sk_12_9 (Cast04) | LocationDirect tracer 5개가 0/1/2/3/4초에 V5→V1→V4→V2→V3→V5 변을 0.625초에 그리고 smoke_tail·ninjaflow가 따라감 |
| 5 | | 0 | Sk_12_6 / Sk_12_7 / Sk_12_8 (Cast~Cast03) | r9~10m 외곽 링과 중앙 충전, r7~8m 반짝임 분수, r3.4~5.9m 바닥 줄무늬 decal과 14m 광선 |
| 5 | | 4.1 | Sk_12_4 (Star02) | 완성된 별 mesh, emitter 1.75s burst → 절대 5.85s에 4초 페이드 인 |
| 5 | | 6.116 | Sk_12_3 (Star01) | 별 2회 섬광(6.1s, 7.1s), Paralyzation 7.7s |
| 6 | Att_Battle_6_03 | 0 | Sk_12_5 | 중앙 15m 폭발 + 별 섬광 0.5s + 카드 소멸 + 같은 10개 decal 재점등 |

설치된 `effect.kouku.source.fx_mn_rpct_05_l.par_l_rpct_05_sk_12_*_loc_int` 문서는 emitter delay와 burst를
`sourceRecipe`에 그대로 보존한다. `sk_12_9`만 문서가 없다(라이브러리 후보의 tracer가 `enginematerials.defaultparticle`이라
2026-09-12 설치가 중단됨). 현재 v2 `boss.kouku.disarm.*`는 반경 3.09m 오각형(원본 내접 오각형 3.15m)에 별을 그린다.

### 공_튀기기 결함

`Client/Private/WorldSequencePlayer_Objects.cpp`의 `Prepare_ObjectResources`(V1 preflight)와
`Sample_ObjectEffectAttachments`(fixed-step provider)는 V1 문서의 follow attachment 본을 Object 모델에서 찾고,
없으면 `World Object V1 source bone is unavailable: b_root`로 실패한다. `world.object.kouku.ball`은 정적
`fm_d_rhcn_00.wmodel`이라 본이 없고, 보스 경로(`KoukuSaydonPresentationPlayer`)는 rig에 `b_root`가 있어 통과한다.
owner 경로 `Effect_PresentationService::Resolve_SourceAnchors`는 없는 본을 건너뛴다. 단, `Effect_Playback::Collect_TransformHistorySample`은
follow slot이 `SourceAnchorWorlds`에 없으면 오류이므로 slot을 비우는 대신 object pivot(identity bone)으로 채워야 한다.

### 발탄 stage008/009

stage008: baked AnimationTrail 3개(1.667/2.221/3.008s, span .2667/.2/.2s, native2379). stage009: baked 1개(1.624s, .5333s) +
`Par_O_RPBF_Trail_01`(1.64s) = ribbon emitter_21(cascadeRibbonV1, StartControl/b_wp_r_01, spawn rate 50/s, SpawnPerUnit 없음, native2410)과
sprite emitter_20/22(SpawnPerUnit 50cm). 저주의식 리본은 SpawnPerUnit(75cm)·rate 0·bspawninitialparticle이다.
native2410은 `v2.x`를 0..1 길이 gradient, `v2.zw`를 tiled panner로 읽으므로 2379와 같은 `(coverageUV, distanceUV)` packing이
필요하지만 `Render_Trails` coverage 목록과 `Resolve_NativeTrailUV`에 없다. 버러지 420612 stage004의
`par_n_rpbf_cast_exp_01` 13개 중 sprite 9개가 `FX_L_Hand_01/bip001-l-hand`에 붙어 있다.

## G02. 변경 파일

| 파일 | 변경 |
| --- | --- |
| `Client/Private/WorldSequencePlayer_Objects.cpp` | 없는 follow 본은 identity bone으로 slot을 채워 object pivot에 부착, preflight는 거절 대신 debug note |
| `Client/Private/Effect_DocumentRenderer_Particles.cpp` | coverage 변환 profile 목록에 2410 추가 |
| `Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl` | `Resolve_NativeTrailUV` 2410 → `(coverageUV, distanceUV)` |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | `ball_bounce` effectTracks 교체, revision 2075→2076, `-Scope WorldSequences` publish |
| `Tools/EffectPipeline/build_saydon_stagger_star_boundary.py` (신규) | 무력화 V1 4문서 생성·등록 |
| `Tools/EffectPipeline/build_valtan_four_slash_axe_ribbon.py` (신규) | stage008/009에 리본·worms 요소 추가 |
| `Data/Effects/Authored/effect.kouku.gate1.stagger.{star,boundary,star.boundary,laser}.group.effect.json` (신규) | 카탈로그·트리·vcxproj 등록 |
| `Data/Effects/Authored/effect.valtan.action.420609.stage00{8,9}.full.restore.effect.json` | 요소 추가만, baked history 불변 |

## G03. 데이터·호출 흐름

### 무력화 V1 문서 구성

- 기본 문서는 `renamed(leaf('12_2_loc_int'))`로 만들고 `sourceModelPreview`는 PATTERN_1의 stage 2~6(6_02×4, 6_03) 또는
  stage 1(6_04)만 0 기준으로 재배치한다.
- `append_group`으로 시스템별 시작 오프셋만 더한다: 카드 0, 별 선 0, 별 완성 4.1, 별 섬광 6.116, 폭발 8.1, 외곽 세 시스템 0,
  레이저 0.609/0.624/2.037.
- 별 선: sk_12_9 후보의 smoke_tail(21~25)·ninjaflow(16~20) 요소를 tracer 곡선(UE3 cm 11 표본, 0.625s)의
  `sourceTransformTrack` positionKeys로 옮기고 `localSpace=false`로 두어 태어난 입자가 변을 따라 남게 한다.
  `efparticlemodulelocationemitter` 모듈은 제거한다. ninjaflow 재질은 `flame.wave.decal`의 admitted sprite(2560)에서,
  smoke_tail의 turbpa_06(3008)은 ribbon-shape 등록이라 sprite에 admission되지 않으므로 레이저 strike의 admitted
  `fx_m_pa_smoke_01_8_tr`(2992)를 PROJECT_AUTHORED 대체로 쓴다(표시명 `smoke*`).
- 별 위치 폭발(PROJECT_AUTHORED): sk_12_5 핵심 7 emitter(53/51/49/52/54/55/60)를 그리기 순서 V5,V1,V4,V2,V3의 꼭짓점에
  8.1s부터 0.12s 간격, `sourceScale.size 0.35`, `count 0.6`으로 반복한다.
- 등록: 카탈로그 행, 트리 `V1` 참조(부모 `kouku.category.cdef6f5c47c6e9a619ed` 세이튼_무력화 시작), Client.vcxproj/.filters
  `None` 항목. Composition presentationResources는 사용자가 Workbench에서 저장 중이므로 자동 추가하지 않고 Append로 만든다.

### 공_튀기기 저작

`effect.ball_bounce.smoke`(GROUP, MOTION_END)와 `effect.1/2`(follow)를 제거하고
`effect.ball_bounce.disappear_start`(TIME 0, followObject false)와 `effect.ball_bounce.disappear_end`(MOTION_END, followObject false,
offset `[0,-2.3,-1.35]/1.5` = 이전 연기와 같은 월드 오프셋)를 둔다. V1 provider는 Object 저작 scale 1.5를 pivot에 유지한다.

### 발탄 리본·worms

- 리본: emitter_21 요소 복제, 시작 = 원본 Trails notify, emission = baked span, life = span + 0.8(클립 끝에서 절단),
  spawn rate 0, SpawnPerUnit(unitscalar 50, emitter_20/22와 동일), `bspawninitialparticle true`, 재질·carrier 유지.
- worms: 420612 stage004 sprite 9개를 `StartControl/b_wp_r_01`(stage009 socket 변환 복사)에 붙이고 rate 60/s(1 tick당 1),
  burst 유지, `localSpace=false`, 40cm 반지름·1.5회전/s 원형 `sourceTransformTrack`(30Hz 키)로 socket 주위를 공전한다.
- stage008 세 슬래시 창마다 리본 1 + worms 9, stage009 창 1개 → stage008 +30, stage009 +10 요소.

## G04. 검증

1. `WorldSequencePlayer_Objects.cpp`, `Effect_DocumentRenderer_Particles.cpp` 격리 컴파일, `Shader_VtxEffectTrail.hlsl` FXC fx_5_0 →
   `out/StaggerBallValtan20260917/compile-*.log`.
2. 신규·변경 문서: `codec_probe.exe`(Load/Validate_Drawable/Roundtrip/Stage), `registration_duration_probe.exe`(15초 playback),
   `geometry_probe.exe track`(별 선 입자의 변 방향 분포)과 `anchor`(합성 StartControl 이동에서 리본 point·worms 입자 수).
3. `Tools/EffectPipeline/Validate-EffectSources.ps1`, `Publish-MapAuthoring.ps1 -Scope WorldSequences`, `git diff --check`.
4. Client.exe·Server.exe·VS가 실행 중이므로 제품 링크는 사용자가 Client 종료 후 Product Build로 수행한다.
