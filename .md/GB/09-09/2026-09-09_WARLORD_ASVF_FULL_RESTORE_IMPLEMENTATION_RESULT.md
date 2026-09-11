# 워로드 A·S·F·V 전체 복원 구현 결과

2026-09-09. 기준 브랜치 codex/dimensionmaster-tool-round3, 시작 HEAD591012dbebf7eeab0b660baec42852b9396e77d4. 기존 dirty·unified·사용자 손튜닝·Resources 원본을 보존했다. Client/UI 실행·조작·캡처 및 시각 PASS는 하지 않았다.

## G00. 구현한 문서와 원본 발생

사용자 서면 확정은 A17090 사슬, S17040 파란 방패 세 개의 정면 발사다. S를 D로 재배정하지 않았다. V17170/F17140 번개도 함께 처리했다.

| 슬롯 | 최신 원본 활성 occurrence | 새 full restore | native 재질 | light | mesh | 실제 follow anchor |
|---|---:|---:|---:|---:|---:|---:|
| A17090 | 7 | 7 | 7 | 0 | 5 | 0 |
| S17040 | 35 | 32 | 31 | 1 | 9 | 0 |
| F17140 | 41 | 39 | 37 | 2 | 1 | 15 |
| V17170 | 38 | 37 | 29 | 8 | 7 | 14 |

새 데이터는 Data/Effects/Authored/effect.warlord.skill.<id>.full.restore.effect.json이다. 121개 활성 발생 중 115개를 유지한다. 원본 활성 notify의 첫 LOD를 따랐으며 모든 retained 행은 visible=true, sourceRecipe.enabled=true, authoringApproximate=false다. 새 파일만 생성하고 종전 스킬 binding/unified를 덮어쓰지 않았다.

최신 설치 data3.lpk에서 추출한 GUNLANCER.loa(2,389,113 bytes,SHA256 0a1a53334aac647f1c6b829ffab5c367132d4a809a095800d3573a7dec08611e)를 사용했다. raw serialized payload byte47의 enabled 값, payload SHA256, action/정확 stage/clip을 검증했다. A/S/F는 canonical stage000, V는 서로 다른 stage000/001/002와 source clip sequence를 연결한다. 같은 clip 이름·시간이 같은 다른 stage를 중복으로 합치지 않는다. 각 native emitter의 첫 LOD enabled와 최신 package serial hash를 보존했다.

## G01. 핵심 geometry와 셰이더

Client/Public/Effect_WarlordNativeMaterial.h는 73개 MIC+VF 프로그램의 native profile400~459/660~672, 이름 있는 texture/numeric/static-switch 계약과 실제 mesh carrier를 검증한다. Client/Bin/ShaderFiles/Shader_EffectWarlordNative.hlsli는 원본 PS의 RT0 출력까지 instruction/수학을 옮기고, 두 WPO VS의 식을 구현한다. native params32행과 texture최대9개를 기존 typed resource packet에 연결한다. renderer carrier별 macro는 dispatch에서 다른 carrier 호출만 제외하며 원본 식을 바꾸지 않는다.

A chain418은 원본 PS e1280265b9bee243ac5444160e9a57e3 / VS f2d41fff62e2df479277b98fe58d41e9를 사용한다. 기존 단일 atypical028 대체가 아니라 원본 diffuse·normal·grid·symbol·noise 등9개 texture expression을 회수했다. 8개 실제 sampled source SRV와 shader에서 읽지 않는9번째 원본 texture binding을 구분한다. native vertex COLOR와 bounds 중심을 사용하는 sin/cos WPO를 복구했다. A helix419는 별도 VS d17daa101dec2b4493fce2f510407f32의 uniform prefix를 따른다.

F electric446은 원본 PS21c77bef151c1642a4eaede1aab2279e / VS343a3e992efbf84683a44fb8904e9a82를 사용한다. 원본 COLOR.r·object radius·source time·DynamicParameter·named vector/scalar 식을 사용한다. 실제 native VS binding arrays는 shaderObject byte268에서 회수했다. source mesh native sphere radius106.800018cm, A source center(0,0,400.000031)cm를 사용한다.

S shield409/410은 원본 shield 두 층의 PSf3e012370cd62d4f9a62c9c2d507b775 /67e6424bbcd5dc4c8a0b530cb56e11d8를 각각 사용한다. raw notify025/026/027의 Par_N_WGL_Shield_02 세 발생은 중심·+31.997681도·-31.997681도 yaw다. 세 payload는 label07-1/2/3와 sourceTransformByteOffset389+40의 signed rotator[0,0,0]/[0,5825,0]/[0,-5825,0]만 달랐다. 이 검증한 triplet에 한해 추가 rotator를 적용한다. 기존 import decoder가 읽은 float rotation+28은 셋 다0이었다. disabled Shield04/05 발생은 새 active full에 넣지 않았다.

V electric671은 PS2d5b6cee31fb384a9d54f06bb62c7483와 실제 fm_d_rectan3cross_001 geometry를 사용한다. 핵심 전기 mesh 세 개를 포함해 V mesh7개를 유지했다.

## G02. 원본 소켓·기본값·light

설치 PC_WR_00.pc_wr_00_sk(physical UB1M781MZZESCJ69SSPFBO5.upk)의 socket58개를 headless UModel dump와 기존 socket parser로 회수했다. F EndControl은 b_weapon_rhand의 localX0.118626m, MidControl은 같은 bone의 localX0.852819m다. V b_effectroot는 실제 Warlord.wmodel bone215이며 parent b_root3, 원본 attack animation channel도 존재한다. b_effectroot/root 애니메이션을 identity로 대체하지 않는다. 기존 Tool의 selected-character CAnimationTargetService anchor provider를 사용하므로 Solo는 Warlord를 선택한 scene에서 실행한다. hidden model cue나 별도 animation runtime은 만들지 않았다. CharacterCatalog.json3343의 실제 bodyModel Character/Warlord/Warlord.wmodel은193clip이며 S bash, A hookchain, F thunderbolt, V guardian start/loop/attack6clip 모두216channel/30TPS로 존재한다. 핵심 character clip 누락0이며 불필요한 animation 복사본을 만들지 않았다.

Source module/archetype/CDO의 raw Distribution을 값 단위로 덮어쓴다. source가 Distribution=0으로 명시한 raw struct에 CDO lookup[0,0,0,0]을 deep merge하던 결함을 수정했다. ParticleParameter는 실제 scalar/vector 종류와 각 axis ParamModes를 적용한다. 원본 native lookup table과 burst/order를 보존한다.

Orbit은 CDO의83byte OrbitOptions를 tagged bool3개로 다시 읽었다. offset/rotation/rotationRate 모두 spawn=true, update=false, emitterTime=false다. raw hex를 실행 입력으로 쓰거나 Orbit을 삭제하지 않고 기존 source runtime의 typed bool path에 연결했다.

Light11개는 EFTypeDataLight의 실제 PointLightComponent2종과 Engine/EFGame CDO를 회수했다. 기본 radius200cm/brightness10/white, 두 번째 radius485.606842cm/brightness15/R255G129B54, falloff2를 기존 typed point-light에 연결했다. 실제 Engine.u UClass SuperIndex도 PointLightComponent12462→LightComponent12313→ActorComponent1532로 검증했다. 원본 색 alpha0은 editor swatch이므로 기존 map-light adapter처럼 runtime alpha1로 전달한다. 원본 EF particle Size→radius tick 수학은 회수하지 못했으며 component의 radius·brightness 연결과 구분한다. bloomscale100 등 별도 원본 post/occlusion 속성을 현재 light 강도로 임의 승격하지 않았다.

## G03. 기존 screen-post 소비자 연결

기존 Engine의 IPresentationScreenPostMaterial::Bind와 PREPARED_MATERIAL consumer를 재사용한다. 새 Effect_NativeScreenPostMaterial.h/cpp는 queued frame의 SRV9개, parameter32행, source color/dynamic, 실제 scene camera projectionW와 source time을 immutable 값으로 소유한다. Bind가 원본 family uniform과 scene depth/color를 연결하고 Begin0만 수행하며 Engine이 ping-pong target과 quad draw를 소유한다.

Effect_Object.cpp는 native screen-post를 일반 zoom/film 값만 넘기는 경로로 보내지 않고 Build_NativeScreenPost가 준비한 typed callback을 제출한다. 알려진 native 계약 실패는 해당 presentation을 거절하고 generic으로 fallback하지 않는다. Shader_VtxEffectNativeScreenPost.hlsl은 DimV68/76, ALTV155/156, Warlord415/672, Artist876/894, Lance659를 명시 분기한다. Lance600 motionblur는 camera/primitive prefix와 v7.z 의미가 아직 닫히지 않아 지원 profile 목록과 dispatch에 넣지 않았다. scene color와 한 번만 alpha 합성한다. Warlord/Artist/LanceMaster의 일반 native refraction particle도 실제 scene capture를 요청하도록 소비자를 추가했다.

공용 Effect_DocumentRenderer core shader 사전 준비·resource→snapshot API, codec/native material/mesh/particle binding, 프로젝트 등록은 통합 root가 소유한다. 정확 삽입 코드는 TEMP/warlord-native-post-renderer.cpp.txt와 warlord-integration.md에 전달했다. 공용 변경의 최종 컴파일 결과는 root 통합 RESULT가 정본이다.

selected source VF의 dynamicparameter 여부를 descriptor bDynamicVertexFactory로 보존했다(73개 중42개). source Dynamic 모듈이 없을 때 원본 Null Dynamic stream의 all-one 값을 전달하는 공용 Playback 소비자도 root가 연결했다.

## G04. 새 full에서 실제 제외한6개

- S `fx_pc_wgl_02.par_n_wgl_shield_02.particlespriteemitter_14` — `SOLO_EXTERNAL_EMITTER_PARTICLE_STATE`; raw `action-17040/stage-000/notify-025`.
- S `fx_pc_wgl_02.par_n_wgl_shield_02.particlespriteemitter_14.event_source-event-014` — `SOLO_EXTERNAL_EMITTER_PARTICLE_STATE`; raw `action-17040/stage-000/notify-026`.
- S `fx_pc_wgl_02.par_n_wgl_shield_02.particlespriteemitter_14.event_source-event-015` — `SOLO_EXTERNAL_EMITTER_PARTICLE_STATE`; raw `action-17040/stage-000/notify-027`.
- F `fx_pc_wgl_07.par_s_wgl_guardianlightning_01.particlespriteemitter_33` — `SOLO_EXTERNAL_EMITTER_PARTICLE_STATE`; raw `action-17140/stage-000/notify-035`.
- F `fx_pc_wgl_07.par_s_wgl_guardianlightning_01.particlespriteemitter_30` — `SOLO_EXTERNAL_EMITTER_PARTICLE_STATE`; raw `action-17140/stage-000/notify-035`.
- V `fx_pc_wgl_00.par_d_gprotection_05.particlespriteemitter_17` — `NATIVE_DECAL_VF_AND_PROJECTOR_PASS_UNCLOSED`; raw `action-17170/stage-002/notify-003`.

S3개는 EFLocationEmitter(name aa)의 sibling particle 위치를, F2개는 LocationEmitter(name123b)의 sibling state를 필요로 한다. 모듈/CDO는 회수했지만 현재 Solo는 live sibling particle 위치·velocity·rotation provider가 없다. 핵심 shield 본체와 electric carrier는 유지했다.

V decal의 native shader 자체는 회수했다(localDecalVF PS6861d20b5b3dcb4da749ef0527cd7c61/VS5d79421dc8571c45aa49790f50274f51). 원본 typed near/far=-250/+250cm와 PS projector depth, receiver fade, sky/ambient prefix를 기존 projector+per-particle clock에 정확히 연결하지 못했다. 종전 imported depth0.25m는 원본과 다르다. 이를 sprite로 대체하지 않고 새 full의 elements에서 제외했다. 원본 unified/Resources/source payload 및 shader evidence는 삭제하지 않았다.

## G05. 검증·실행 경계

- JSON parse, stable ID 중복 없음, descriptor texture/parameter/switch명, Sources/Resources 파일 존재와 runtime bone 이름 검증 완료.
- 실제 native geometry cook에서 legacy P/N/T.xyz/UV0/index bytes를 보존하고 원본 P/UV의 모든 matching 후보가 native COLOR/W에 동의함을 확인했다. native COLOR42종(chain)/62종(electric),255/124vertex.
- standalone FXC WarlordPS ps_5_0, VS vs_5_0 성공. PS에는FXC X4000 경고230개가 있으며 VS는경고0개다. 사용하지 않는 scalar uniform yzw의 0 나눗셈은 x-only 평가로 제거했다. native post fx_5_0도 성공; 포함하는 기존 ALTV/Artist source 경고는 별도 로그에 남아 있다. Lance659 연결 후 전체 post fx_5_0도 성공(X4717 1개/X3078 6개/X4008 32개/X4000 5개)했고 compiled effect dump에서 SRV0~8, 다섯 family parameter32행 및 source color/dynamic/projectionW 변수가 모두 존재함을 확인했다. 경고0이나 visual PASS로 기록하지 않는다.
- 실제 codec의 S32/A7/V37 모두 load/save/Solo76은 root probe로 통과했다. raw null CDO, light 상세0, OrbitOptions raw hex 결함은 원본 입력 재회수로 수정했다. F의 active SizeMultiplyVelocity 모듈은 첫 검증에서 strict executor 미지원을 확인했고, 실제 native UClass/CDO와 Epic UE3 원문 Spawn/Update의 속력 배율·축별mask·caps를 대조해 root 공용 구현에 전달했다. F는 최신 공용 Playback/Codec을 독립 임시 probe로 컴파일하여 Load·Stage·Solo 및 시간 sweep를 통과했다. 실제 F39개260step peak155, 해당 SizeMultiplyVelocity Solo224step peak50이며 전프레임 finite다. 원본 단위 속력 배율·Y축 제외·양수 min/max cap·속력0·LifeMultiplier 순차 적용5개 수치 사례를 각각 direct Spawn/footer, BaseSize 보존,45회 public Update, public Seek로 검사해266검사/실패0을 확인했다. 임시 private access는 상태 관측과 직접 Spawn 호출만 허용했고 제품 인터페이스를 추가하지 않았다. follow anchor2개에는 CPU 검사용 identity를 주입했으며 source bone 시각 검증으로 취급하지 않는다. root의 후속 actualCodec 검증으로 F Save_Atomic까지 포함해 워로드115개 전체 Load·Save·Solo PASS를 확인했다. root가 현재 Effect_DocumentRenderer.cpp와 새 Effect_NativeScreenPostMaterial.cpp focused C++ compile 성공을 확인했다.
- 최신 공용 SubUV 실행 변경 후 같은 독립 probe를 재컴파일했다. F 266검사 회귀를 포함해 총 976검사/실패0이다. 원본 native Artist470/825/827, Lance604/651, Warlord406/416의 7개 occurrence를 source module 수정 없이 각각 Load·Stage하고, 935 live frame/13,119 particle sample 및 21개 public Seek 대조를 확인했다. 32개 seed의 출생 무작위 프레임, Changes=0의 평생 유지와 추가 RNG 미소비, 원본 상대수명 간격0.198/0.495의 equality 유지·strict greater 교체, sprite 원본 image flip을 포함한 UV 셀 범위와 mesh UV를 검사했다. 실행한 Playback SHA256은45e4955df56ce1d1b1384e85720c24144e9dc98fe878aba10835f0904d78920b이고 기록은 out/WarlordASVFRestore20260909/size_velocity_subuv_result.txt다. 화면과 GPU pixel 결과는 검증하지 않았다.
- Effect_Object.cpp와 native post shader의 git diff --check 완료. 새 UTF-8파일과4JSON을 생성했으며 Client/UI/서버 아레나/카메라를 실행하지 않았다. 원본 deferred masked depth-write, full engine fog/scene sky, EF light tick 수학의 시각 동등성을 주장하지 않는다.

## G06. Resources 인계

물리 루트는 Client/Bin/Resources다. Git에 binary를추적하지 않았다. 기존 파일을 덮어쓰지 않고 아래 새12개만 설치했다. Drive 전달 준비는 로컬 설치/상대 ID 기록까지이며 Drive 업로드는 하지 않았다.

- `Effect/Warlord/FullRestore/Meshes/fm_d_berchain_06.wmodel`
- `Effect/Warlord/FullRestore/Meshes/fm_d_electric_05_vertexcolor.wmodel`

Texture 세부 설치 기록: out/WarlordASVFRestore20260909/texture_installation.json.
- `Effect/Warlord/Textures/FX_TEX_HIGH_00/fx_a_cloud_026.dds`
- `Effect/Warlord/Textures/FX_TEX_01/fx_c_cloud_018.dds`
- `Effect/Warlord/Textures/FX_TEX_00/fx_a_environ_003.dds`
- `Effect/Warlord/Textures/FX_MASTERMATERIAL/fx_e_normal.dds`
- `Effect/Warlord/Textures/FX_TEX_01/fx_c_noise_002.dds`
- `Effect/Warlord/Textures/EFMASTER_MATERIAL_PROLOGUE/fx_a_noise_018.dds`
- `Effect/Warlord/Textures/FX_TEX_HIGH_00/fx_c_cloud_013_cl.dds`
- `Effect/Warlord/Textures/FX_TEX_04/fx_j_ylinestream_01.dds`
- `Effect/Warlord/Textures/FX_TEX_00/fx_a_atypical_020.dds`
- `Effect/Warlord/Textures/FX_TEX_01/fx_c_line_001.dds`

## G07. 재현 자료

Tools/EffectPipeline/build_warlord_asvf_full_restore.py는 out/WarlordASVFRestore20260909의 회수 결과와 보존 Imported/Warlord/CurrentCombat을 읽어 새4문서만 생성한다. 설치판 raw package·shader·CDO·module·geometry·socket evidence는 해당 out폴더와 TEMP/lostark-20260909-audit에 있다. 이 추출 binary나 intermediate를 기능소스 커밋에 포함하지 않는다.


## G08. 2026-09-10 전체 슬롯 확대와 승인된 F 보존

사용자가 F 낙뢰 결과를 승인했다. F full39행은 수정하지 않았고, 기존 WarlordNative400~459/660~672의73개 함수 본문을 확장 전 파일과 문자열로 대조해 모두 동일함을 확인했다. A/S full도 보존했다. 신규 native196개를1000~1199 및2000~2004의 명시 ID로 추가했다. 전체269개 descriptor이며 새 shader는 EFFECT_WARLORD_GROUP960/1024/1088/1152/1984로 나누었다. 기존 group0과 새 그룹의 C++ family dispatch·FX 진입 파일은 root가 소유한다.

현재 전체27 full 문서718행이 JSON parse, stable ID 중복 검사와 모든 Resources 실물 존재 검사를 통과했다. 이 수는 V의 통합 검토본과 clip별 실행본에 중복된 원본 발생을 포함한 문서 행 수다. 원본 발생718개 또는 한 번에718개가 생성된다는 뜻이 아니다. 기존4문서에서23문서를 추가했고, V 통합본의 방패 배치를 변경했다. LMB/Q/W/E/R/A/S/D/F/T/X/V/ALT_V를 포함한다.

| 스킬 | full 문서와 연결 clip | 현재 행 수 |
|---|---|---:|
| LMB17000 | ba0/ba1/ba2 → att_battle_1_01/02/03 |9 /9 /13|
| Q17030 | full → pierceingspear |18|
| W17060 | full → firebullet |14|
| E17080 | clip1/2 → dashupperfire_01/02 |4 /20|
| A17090 / S17040 / F17140 | 승인 전 기존 full 보존 |7 /32 /39|
| D17100 | full → shieldstamp canonical stage000 |4|
| R17110 | clip2/3 → leap_jump / leap_attack_01 |8 /7|
| T17240 | ba0/ba1/clip3/clip4/ba2 → eternalcyclone_01/02/03/04/07 |34 /8 /12 /12 /49|
| X17820 | clip1/2/5 → shieldofbattlefield_01/02/03 |3 /3 /11|
| V17170 | 통합 full / clip1/2/3 start·loop·attack |43 /10 /21 /18|
| ALT_V17250 | clip1/2 → super_guardianofprotection_01/02 |167 /143|

T의 현행 skillbindings 두 번째 콤보에는 _02를300ms, _03를300ms, _04를 끝까지 재생하는 chain이 있다. 보존 import receipt가 _01/_02/_07만 선택해 _03/_04의 원본 활성40행을 빠뜨렸으므로 실제 raw stage002/003을 추가 회수했다. 그중 기존 native material+VF가 닫힌24행을 clip3/4에 연결했다. D는 같은 clip을 쓰는 stage000/001/002를 합치지 않았으며, X는 현재 bound _01/_02/_03(raw stage000/001/004)만 사용한다. 정본 animevents의 실제 게임 재생 연결은 root 통합 변경으로 별도 검증한다.

## G09. V와 ALT_V의 실제 방패 모델 배치

실제 ALT_V 원본 방패는 `fx_sm_00.sk_wgl_gdd_01`이다. 설치 legacy1016정점/1224triangle과 현재 원본 P/UV를 대조하고 native COLOR/TANGENT.W를 회수한 `Effect/Warlord/FullRestore/Meshes/sk_wgl_gdd_01.wmodel`을 연결했다. source MIC `fx_m_mi_w_00.mi.fx_w_wg_gdd_01_01_ma`의 native1124는 원본 diffuse/normal/spec/emissive4texture와 가짜 광원 식을 사용한다. 원본 PBR01/02 두 재질의 실제 lighting prefix는 아직 미연결이므로 이 masked 원본 변형을 PBR 복원이라고 기록하지 않는다.

1124의 source PS b2c15f2519e18241a312c198882af321는 cb0[0].xyz를 RGB multiplier, .w를 dither opacity로 읽는다. .x만1로 공급하면 모든 픽셀을 discard하는 것을 instruction에서 확인하여 정확한 neutral(1,1,1,1)을 공급했다. 실제 material uniform1~14는 회수된 이름 있는 native bindings가 채운다. 원본 masked RT0.w=0은 alpha blend용 값이 아니므로 source discard 뒤 새 masked1013/1124/1143의 carrier alpha를1로 공급했다.1143도 같은 neutral prefix를 사용한다. 기존73함수의 계산은 바꾸지 않았다.

사용자가 지정한 배치는 source occurrence를 그대로 복제했다는 주장과 분리했다. V는 반지름4.5m에 방패5개를72도 간격으로 배치하고, 캐릭터 중심에는 원본 protectshield material2001과 fm_m_sphere_004로 보호막 하나를 추가했다. ALT_V는 외곽 반지름6m의6개와 캐릭터 주변 반지름1.4m의6개를 각각60도 간격으로 배치한다. 각 shield element는 deterministic burst1개만 생성하며 원본 ALT_V의 추가6burst shield occurrence는 중복을 막기 위해 교체했다. clip1/2에는 각각12개의 별도 shield element가 있다. 사용자가 요청한 수·위치·지속 시간은 project-authored이다.

V 통합 full은 Tool 검토용으로 유지한다. gameplay clock 중복을 막기 위해 raw stage0/1/2의 offset0/1.1667/2.5초를 빼서 세 clip 문서를 별도로 만든다. 방패와 개인 보호막은 loop·attack 문서에만 배치한다. source PlaySkeletalMesh notify가 None/None인 빈 항목을 모델 복원 근거로 사용하지 않았다. fm_d_wgl_gprotection_05는6개 직육면체가 이미 합쳐진 geometry이므로 이를 단일 방패로5번 또는12번 겹쳐 놓지 않았다.

## G10. 실제 제외와 자동 검증 경계

추가 문서에서 source native material/VF 미연결76행, live sibling particle 의존6행, typed source parameter ordinal2 미지원5행, particle collision module 미지원3행, 실제 owner socket FX_Weapon_01 미해결1행을 제외했다. 정확한 source element/notify와 사유는 같은 폴더의 `2026-09-09_WARLORD_ASVF_FULL_RESTORE_EXCLUSIONS.json`에 보존한다. 원본 Import와 Resources는 제거하지 않았다. inactive source notify와 다른 class clip은 full에 자동 활성화하지 않는다.

root의 첫 actualCodec Load/Save/Solo 검사에서 W/E/T의 동일 collision occurrence3개와 authored shield attachment의 socketLocalTransform 누락을 확인했다. collision은 해당3행만 제외했고, attachment는 원본 전체 구조를 유지한 채 enabled/follow=false로 수정했다. 이는 화면 Solo나 pixel 검증이 아니다. 최신27문서의 최종 actualCodec 재검증 및 통합 ClCompile/FxCompile 결과는 root의 대응 통합 RESULT를 따른다. 실행하지 않은 검사나 사용자 화면은 PASS로 기록하지 않는다.

후속 actualCodec에서 T clip3/4의 빈 module class와 ALT_V clip1의 vectorfield asset literal 누락을 확인했다. 새 T source export가 가진 실제 className/objectPath를 imported metadata보다 우선하도록 고쳤고, ALT_V가 참조한 정확한 원본 vectorfield를 기존 portable asset 계약으로 배치·연결했다. 해당 요소를 삭제하거나 module 검증을 완화하지 않았으며 행 수는27문서718행이다. 통합 Client ClCompile/FxCompile은 root가 성공을 확인했다. 현재 통합 shader 로그의 Warlord X4000은 sampler0~7과 Shade dispatch에만 발생한다. sampler의 모든 주소/LOD 분기는 반환하며 dispatch 출력·color·additive, 양쪽 carrier input과 native source/register 배열도 초기화되어 있어 읽기 검토에서 실제 미초기화 경로를 발견하지 못했다. 기존 sampler/control-flow 형식에서 전파된 FXC 경고로 판단하며 경고0이나 화면 PASS를 뜻하지 않는다.

ALT_V의 원본 `par_w_wgl_supergprotection_sdenergy.particlespriteemitter_36.particlelodlevel_48`은 동일 cameraoffset_4 export(import index -702)를 modules 목록에 두 번 저장한다. 기존 import stableId를 두 번 복사하여 codec이 ID 중복을 거절한 원인을 원본 목록과 대조했다. 두 모듈 적용과 순서는 유지하고, 두 번째 참조만 source LOD/object/reference occurrence로 만든 hash ID를 사용한다. 전27문서의 모든 module ID 유일성·문자열 길이·class/path 존재·literal/distribution 개수는 실제 codec 제한으로 다시 검사했다. T clip3/4는 수정 후 actualCodec에서 각각12행 Load/Save/Solo를 통과했다.

최종 수정본의 root actualCodec 로그를 직접 읽었다. `out/ThreeClassFullRestore20260910/codec-final.log`의 기존24문서527행, `codec-final-fixed.log`의 T2문서24행, `codec-final-alt.log`의 ALT_V clip1 167행이 각각 FILE_PASS다. 전체27문서718행이 Load/Save/Solo admission을 통과했고 save_failures/예상 밖 실패0, hidden0이다. 각 단계 사이에는 실패한 문서만 위와 같이 수정했으며 최종 성공 로그를 합산한 결과다. GPU 화면과 사용자 Solo 판정은 계속 미실시다.

직접 확인한 검사는27JSON parse/stable ID/실물 존재, 기존73 native 함수 본문 보존, 기존 geometry의 P/N/T.xyz/UV/index bytes 보존과 원본 native color/W 회수, generator 실행, 변경 파일 diff whitespace 검사다. `out/WarlordAllRestore20260910/data_validation.json`에 현재 F 파일SHA256와 문서별 count를 보존했다. Client/UI 실행·조작·캡처·최종 시각 판정은 하지 않았다.

## G11. 추가 Resources와 재현

새 geometry35개를 `Client/Bin/Resources/Effect/Warlord/FullRestore/Meshes`에 설치했다. 정확한35개 상대 ID는 `out/WarlordAllRestore20260910/geometry_installation.json`에서 기존 chain/electric2개를 제외한 목록이다. source P/UV가 같은 seam은 native normal과 tangent까지 대조해 하나의 COLOR/W로 확정했다. ambiguous source 입력을 임의 첫 vertex로 선택하지 않았다. 완성된 binary는 Git에 추적하지 않는다.

새 texture9개를 `Effect/Warlord/Textures` 아래에 설치했다: FX_TEX_00/fx_b_cloud_019.dds, fx_a_atypical_048_cl.dds, fx_a_noise_006.dds, fx_a_cloud_003.dds, fx_a_cloud_031_cl.dds; FX_TEX_01/fx_c_trail_005_cl.dds; FX_TEX_02/fx_d_fire_003.dds; FX_TEX_04/fx_j_bigsmokes_m_01.dds, fx_j_checkerboard_01.dds. UModel headless export 결과 DDS header와 실제 file 존재를 확인했다. 설치·정확 ID 기록까지 완료했으며 Drive 업로드는 하지 않았다.

ALT_V clip1의 원본 `fx_cm_05.vectorfield.fx_n_vector_field1`은 실제10×10×10 벡터 필드다. 같은 원본을 읽은 Artist 추출 결과의 raw/source export 근거와 binary SHA256를 대조해 `Effect/Warlord/VectorFields/fx_cm_05.fx_n_vector_field1.wvectorfield`에 배치하고 source module의 `vectorfield.assetid`로 연결했다. material이나 단순 force vector로 대체하지 않았다. 정확한 source package/export와 파일 기록은 `out/WarlordAllRestore20260910/vector_field_receipt.json`에 있다.

현재 generator는 `--all-skills --evidence-root out/WarlordAllRestore20260910`으로 일반 full들을 생성한 뒤 `--compose-shields-only`로 방패 및 V clip별 문서를 생성한다. 이전 default invocation은 A/S/F/V4개 원본 생성 경로를 유지한다. 원본 package·shader cache·module·geometry와 extra T raw stage의 회수 자료는 해당 out폴더에 보존한다. 현재 package에서 shader cache layout을 다시 읽었으며 이전 저장 layout의 code-section offset을 강제로 허용하지 않았다.

## G12. 사용자 재검토 후 방패와 황색 번개 수정

사용자는 현재 EXE에서 V/Alt V 방패와 V의 F형 번개가 보이지 않는다고 보고했다. 방패의 native1122/1123은 local vertex factory의 원본 material uniform `meshemitterdynamicparameter`를 써야 하지만 새 PBR adapter가 Dynamic 모듈 없는 particle stream0을 넘겼다. 원본 dissolve 식이 모든 fragment를 버리는 원인을 확인하고 `Shader_EffectWarlordNativeGroup1088.hlsli`의 base/light 네 입력을1122 parameter12,1123 parameter16으로 복구했다. 원본 cutoff·dead와 기존78개 방패/장식 행의 위치·크기·수명은 유지했다.

기존 standalone PS probe를 재사용했다.1122 full PS는 수정 전0/4096에서 수정 후4096/4096 비영 RGB로 바뀌었고 최대RGB는5.859292다.1122/1123의 원본 mask gate도0→4096이며 material uniform을0으로 둔 음성 대조는 계속0이다. 수정 후27case에서 nonfinite는0이었다.1123은 probe의7texture 입력 한계 때문에8texture 전체 radiance가 아닌 실제 mask14instruction만 검사했다. 근거는 `out/WarlordGuardianVisible20260910/handoff.md`, `before-results.csv`, `after-results.csv`다.

V의 F17140 native4464wave는 이미 연결되어 있었다. 이전 patch가 원본 F의 HDR startColor와 ColorScaleOverLife를 낮은 상수로 교체한 차이를 확인했다. 현재도 그 이전 patch 값과 정확히 일치하는 두 분포만 F 원본으로 되돌렸다. 총16distribution을 교정했고 기존 wave 시각·반경·position·seed·alpha·material과 다른 Element는 모두 보존했다. F 원문 SHA는 변하지 않았다. 이 변경은 실제 F 입력의 복구이며 V 미표시의 모든 원인을 화면으로 검증했다는 뜻은 아니다.

Alt V clip1의 native1166 electric005 번개6개는 기존 발생 시각과 shape 그대로 독립 seed 복제6개를 추가했다. 번개 occurrence6→12, 명시 burst24→48이며 기존179Element는 모든 field가 동일하다. 새 전체 행 수는185다. 새 copy의 sourceNode는 `authored-copy:<원본 ID>`를 사용한다. 수정 스크립트 재실행은 모든 대상 JSON을 바이트 단위로 유지했다.

| 항목 | 상태 |
|---|---|
| V full/clip2/clip3 행 수 |52/29/24 유지 |
| Alt V clip1/clip2 행 수 |185/155 |
| 변경 JSON parse·보존 대조·스크립트 멱등성·diff check |PASS |
| 방패 PS 수치 검증 |위에 명시한 범위 PASS |
| 변경4문서 실제 codec Load/Save/Solo |290Element,290Solo, save_failures0·예상 밖 실패0 PASS (`out/WarlordGuardianVisible20260910/codec-result.log`) |
| 최종 Product shader |Debug Product compile/link·배포 PASS. 새 Warlord1088 mesh/particle CSO 생성 및 본체9모델/14slot의 새 제품 DLL/CSO 바인딩 failures0 |
| 사용자 새 실행파일에서 방패·번개 재검토 |미실행 |

데이터 변경 근거는 `out/ArtistWarlordVisualFollowup20260910/Warlord/v-altv-lightning-resume.json`과 `lightning-resume-verification.json`, 직전 사용자 파일은 `lightning_resume_before/`에 있다.

최종 빌드 근거는 `out/BuildPipeline/runs/20260910T063843956Z-debug-product.json`이다. 새 제품의 본체 재질 회귀 검사는 `out/DimensionMasterMaterialResume20260910/final-warlord-activation.json`, DLL/CSO 실물 일치는 같은 폴더의 `final-probe-inputs.json`에 있다.

## G13. 워로드 Q full source follow 중복 축소 교정

2026-09-10 사용자는 Q full을 Play해도 잘 보이지 않고, 이후 무언가는 보이지만 BA만큼 깔끔하지 않다고 보고했다. 현재 Q17030→`wgl_sk_pierceingspear`→`effect.warlord.skill.17030.full.restore` 연결과18행은 정상이다. 원본 Particle Color는 붉은 HDR 값이며 예를 들어30/5/1이다. 실제 Dynamic 모듈이 있는 Q mesh1000/1005/1012는 앞선 방패의 빈 Dynamic stream 문제와 다르다. shader 수식과 원본 색을 임의 교체하지 않았다.

실제 Playback119행을 현재 native PS12개와 실제 source DDS에 공급한357개 WARP 수치 case에서 nonfinite0이었다. 핵심 mesh1000/1005/1012의 alpha와 RGB가 양수여서 shader 전체0 가설을 배제했다. 원본 RT0 alpha0을 쓰는 additive 발생은 RGB로 판정했다. 원본 screen-post와 light는 이 PS 대상이 아니다. GPU 결과는 숫자만 읽었고 이미지나 스크린샷을 생성하지 않았다.

원인은 source follow의 이중 단위 변환이다. 제품 `PlayableCharacterAssetService`의 Warlord admission0.0001×yaw-90과 rig root100이 실제 named bone combined basis0.01을 만든다. `b_weapon_rhand`, `b_effectworldzero`, `bip001-spine2`의 translation은 이미 m단위다. 일반 source follow가 이0.01을 이미 m단위인 Q particle 크기에 다시 적용해 핵심 효과가 작아졌다. 같은 실제 모델·pose·현재 CSO·`CEffectDocumentRenderer::Render`를 쓰고3×3의 import basis만 제거하면 효과 출력이 회복됐다.

| 같은256×256 WARP 수치 장면 | 수정 전 | 수정 후 |
|---|---:|---:|
| 전체28sample 최대 비영 RGB pixel |217|2456|
| 전체28sample 최대 RGB |0.122042|413.648|
|0.6667초 비영 RGB pixel |217|1649|
|0.6667초 최대 RGB |0.119533|81.4834|
|0.6667초 mesh 제출 수 |4/6|6/6|
| nonfinite |0|0|

합성 camera/depth와 실제 effect geometry를 사용하는 수치 비교다. live scene, tone mapping, BA와의 최종 실루엣·색 일치를 대신 판정하지 않는다.

`Effect_PresentationService.cpp`의 typed source-bone scale manifest가 Q full 한 asset의 prototypeAdmissionScale0.0001, rigRootScale100, combinedAnchorScale0.01, reciprocal100을 소유한다. 합성값과 기존0.01 helper의 호환은 static_assert로 검사한다. 새 `Requires_SourceBoneImportScaleNormalization` 선택을 Product source-anchor 수집과 Tool 현재 pose/과거 pose 수집이 함께 소비한다. 기존 `Build_SourceBoneAnchorWorld`의 엄격한0.01·직교·affine 검사와 translation 보존은 그대로 재사용한다. 다른 Warlord 슬롯·Artist·미등록 asset은 새 선택에서 제외되며 기존 Artist31470 전용 경로는 바꾸지 않았다.

변경 파일은 `Client/Public/Effect_PresentationService.h`, `Client/Private/Effect_PresentationService.cpp`, `Client/Private/Effect_Tool.cpp`와 대응 PLAN/RESULT다. 기존 인코딩과 줄끝을 유지했다. Q JSON18행, source resource ID, shader 수식, timing과 cue 연결은 보존했다. 신규 C++/shader 파일과 project/filter 등록은 없다.

검증 증거는 `out/WarlordQRestore20260910`에 있다. `draw-product-root.log`는 수정 전, `draw-actual-fix.log`는 실제 수정된 Service 객체의 helper를 연결한 현재 CModel/Playback/Renderer 숫자 결과다. `frames.csv`, `ps_results.csv`, `ps-probe.log`는119입력/357case다. 최종 변경 Service와 Tool 번역 단위 최소 컴파일 및 `git diff --check`는 성공했다. JSON/XML은 수정하지 않았다.

`anchor-debug.log`는 실제 제품 pretransform으로 로드한 bind pose와 Q0.6167초 pose에서 wgl·손·worldzero·spine의8개 matrix를 검사한다. 축0.01→1, translation 정확 보존, owner scale2 보존, 잘못된0.0001/1/비균등 입력24개 거절, Q 외 asset3개 제외를 확인했다. 이 독립 probe는 현재 source helper 본문을 그대로 컴파일했다. 별도 actual renderer 검증은 실제 Service 번역 단위를 연결했다.

Release 최소 probe의 컴파일은 가능했으나 기존 `Engine/Bin/Release/Engine.lib`가 현재 `CModel::Create(..., bool)` 심볼을 제공하지 않아 link 실패했다. 오래된 Release ABI를 임의 우회하거나 전체 Release 제품을 빌드하지 않았다. Release actual model 검증은 미완료다. Debug Product 전체 빌드·배포는 통합 root가 소유하며 이 절의 개별 TU 검사와 구분한다. 실행 중인 Client/UI 조작·캡처·EXE 교체와 사용자 최종 화면 판정은 수행하지 않았다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.

## G14. 2026-09-11 Alt V의 F 방식 황금 낙뢰 연결

사용자는 Alt+V 번개가 전혀 보이지 않으며 F의 번개를 그대로 사용해 개수를 늘리고
황금색으로 표시해 달라고 요청했다. 현재 정본을 대조하면 F native446는 V17170에만
연결되어 있고 Alt V17250에는0개였다. 이전 Alt V 보강은 F와 다른 native1166 sprite의
b_effectroot FOLLOW6개를12개로 복제한 것이다. F와 같은 번개를 재생하라는 요청이
실제 Alt V 입력에 연결되지 않은 차이를 확인했다. 기존1166 자체의 화면 미발생 원인을
사용자 화면 없이 확정한 것은 아니다.

실제 Alt V clip1/clip2 두 문서에 F의 native446 mesh 낙뢰18개를 추가했다.
clip1은218→230행, clip2는180→186행이고 기존398개는 모든 field를 보존했다.
0.6473/0.8472/1.0472/1.247/1.447/1.647초의 clip1 6회, 0.172/0.526/0.876초의
clip2 3회에 각각 안쪽200~220cm와 바깥360~400cm 두 반경으로 재생한다.
독립 seed18개, F와 같은 발생당 burst4개로 총72개 명시 낙뢰를 추가했다.
기존 Alt V sprite48개는 삭제·재색칠·일괄 확대하지 않았다.

F의 root snapshot과 source yaw-90, vertexcolor electric mesh, native446 material,
WPO/Dynamic/dissolve, emitter·particle lifetime와 grayscale ColorScaleOverLife를
그대로 복제했다. startcolor만 원본 peak5를 유지한 황금 HDR [5,3.6,0.4]로 저작했다.
파란 source color에 황색 multiply를 곱해 어둡게 만드는 경로를 사용하지 않는다.
이 황금색과 추가 파동은 사용자가 요청한 PROJECT_TUNED이며 원본 발생 복원과 구분한다.

수정 재현 명령은 다음과 같다.

```powershell
python -X utf8 Tools/EffectPipeline/patch_warlord_v_guardian_lightning.py --altv-f-gold-only --output-root out/WarlordAltVGold20260911
```

기존 patch 스크립트에 위 선택을 추가했고 CAS 저장 및 동일 ID 재실행 보존을 유지했다.
F 원문 SHA256는 `04d6dc2e93450e02b84f3053008ec32804a7173573934e7e14176bdbbeda28f1`이다.
F·V 문서, camera sidecar, skillbindings와 animevents는 변경하지 않았다.
현재 Alt V 두 asset의 Product animevent가 이미 연결되어 Data 정본을 직접 읽으므로
별도 publish/새 catalog 항목/새 runtime 경로는 없다.

| 자동 검사 | 실제 결과 |
|---|---|
| 변경 JSON parse·stable ID·seed·burst·F의 material/geometry/attachment 대조 |PASS |
| 기존 Alt V398개 모든 field와 문서 상위 값 보존 |PASS |
| 패치 재실행 두 JSON과 F bytes 동일 |PASS |
| 공식 문서 검사 함수의 color-space/native sprite/module override/attachment 검사 |변경 두 문서416행 PASS |
| 공식 Resources closure 함수 |270개/80,005,436bytes PASS |
| Python 구문 컴파일·git diff --check |PASS |
| 전체 Validate-EffectSources |실패. 이번에 수정하지 않은 Artist unified31000 BA1~4와31050 clip2의 runtime consumer5개 누락 보고 |
| 실제 Product codec/stage/playback |두 문서 230/186개 admission PASS, 추가 황금 18개 모두 실제 CPU particle 생성·양수 alpha·HDR 비율·수명 범위 PASS |
| 제품 최소 컴파일·배포 |통합 root의 최종 빌드 기록 참조 |
| 사용자 새 Alt V 입력과 최종 황금색·크기·개수 확인 |미실행 |

변경 전 bytes와 적용 기록은 `out/WarlordAltVGold20260911/lightning_resume_before/`,
`altv-f-native-gold.json`에 있으며 보존/멱등성과 공식 scoped 검사 결과는 같은 폴더의
`verification.json`, `official-scoped-validation.json`이다. 기존 CPU 경로로 검사한
`cpu_probe_result.json`, `cpu_focused_checks.json`도 같은 폴더에 있다. 신규 제품 하네스는
추가하지 않았다. 명시 burst는 72개이며 F의 continuous rate도 보존했으므로,
각 요소의 관측 peak 5 particle을 총 고유 발생 개수로 해석하지 않는다.

새 Resources는 없다. 기존 `Effect/Warlord/FullRestore/Meshes/fm_d_electric_05_vertexcolor.wmodel`,
`Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_049.dds`,
`Effect/Warlord/Textures/FX_TEX_04/fx_i_thunder_02_ycl.dds`를 사용한다.
C++/shader/project/filter/XML 변경과 Resources Git 추가는 없다.
Client/UI 실행·조작·캡처는 하지 않았다. 사용자 경로는 Server + Client profile Ctrl+F5 →
Lobby → Character Select → Warlord → F 비교 후 Alt+V다.

## G15. 2026-09-11 Alt+V 최종 통합 빌드

황금 F-native 번개18개 묶음을 포함한 최신 워크트리에서 Debug Product의
Engine/Shared/Server/Client compile·link·deploy가 모두 PASS다.
`out/BuildPipeline/runs/20260911T073907619Z-debug-product.json`, Client.exe16:39:07 KST.
두 Alt+V 문서의 JSON parse와 실제 Resources 참조, 최종 `git diff --check`를 확인했다.
서버/Client는 마지막 확인에서 종료 상태이며, 사용자가 Server + Client profile을 실행하고
Character Select의 Warlord F와 Alt+V를 비교한다. 황금색/개수/타이밍의 시각 확인은 USER_PENDING이다.
