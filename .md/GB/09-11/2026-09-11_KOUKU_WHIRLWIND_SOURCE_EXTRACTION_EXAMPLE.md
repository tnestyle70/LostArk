# 2026-09-11 쿠크 휠윈드 원본 Effect 추출 실례

**원본 휠윈드 Action에는 어떤 Effect를 언제 재생하는지 정보가 있고, 연결된 ParticleSystem에는 emitter·mesh·material·수명·분포 정보까지 실제로 있다. 기존 원본 추출 파일을 다시 읽어서 확인했다.**

이 패턴은 이름이나 화면만 보며 처음부터 전부 손으로 만들어야 하는 상태가 아니다. 원본 데이터를 추출해 복원의 출발점으로 삼을 수 있다. 다만 추출된 정보를 현재 renderer에 변환·연결하고 재질과 종료 동작을 맞추는 구현은 별도 작업이다.

앞서 만든 ACTIONS/NOTIFIES/PARTICLES CSV가 보유 corpus의 전수 추출 목록이고, 이 문서는 그중 **4219708 휠윈드 하나의 실제 추출 내용**을 펼친 예시다. 새로운 Engine용 full.restore 파일, publisher 출력 또는 Resources는 생성·변경하지 않았다.

## 다시 읽은 원본

| 종류 | 파일 / identity |
|---|---|
| Action 추출본 | C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCZ_00.action-effects.json |
| 원본 Action | profile MN_RPCZ_00, action4219708, 이름 **쿠크_뿅망치_액션_휠윈드** |
| 원본 LOA | 같은 추출 root의 CanonicalSource/Sound/Lpk/EFGame_Extra/ClientData/XmlData/Action/MN_RPCZ_00.loa |
| Particle graph | 같은 추출 root의 CanonicalSource/Effect/Graphs/LV_LUT_MIDNIGHTC_ED.particle-graph.json/FX_MN_RPCZ_00_G.particle-graph.json |
| 회전 Effect | FX_MN_RPCZ_00_G.par_g_rpcz_00_hammer_b_loc_int / **FX_MN_RPCZ_00_G:export:9295** |
| runtime clip 대조 | Data/Animation/Reference/KoukuSaydon/MN_RPCZ_00.actionreference.json의 동일 action |

원본 action에는 **10stage,22개 Particle notify,6개 고유 ParticleSystem**이 직접 들어 있다. stage0~4와5~9에 같은 clip·particle 구성이 저장돼 있다. 원본 stage 전이·분기 의미는 아직 별도 해독 대상이므로 이것만 보고 “반드시 두 번 연속 실행되는 패턴”이라고 단정하지 않는다.

## 실제 clip과 Effect 시간

모든 시간은 해당 stage 시작 기준 초다. duration0은 원본에0으로 저장된 값이며, 0초 동안만 보이거나 즉시 제거하라는 의미로 해석하지 않았다. WP_1/B_root는 notify payload의 serialized label에서 직접 확인한 원본 부착 이름이다. 현재 runtime bone과의 변환은 별도다.

아래 표의 Effect 약칭은 바로 다음 표에서 원본 전체 이름과 연결한다.

| stage | source clip / 선언 길이 | Particle 시작 t / duration / 부착 이름 |
|---|---|---|
| 0 / 5 | Att_Battle_2_01 / 2.5s | Hammer:1.698705/0/WP_1, WeaponSpawn:1.745535/0 |
| 1 / 6 | Att_Battle_2_04 / 1.5s | Hammer:0/1.5/WP_1 |
| 2 / 7 | Att_Battle_2_05 / **0.266667s** | Hammer:0/**4**/WP_1, HammerB:0/**4**/B_root, UnderDust:0/0, UnderDust:1.292884/0, JumpDust:2.657340/0 |
| 3 / 8 | Att_Battle_2_06 / 0.9s | Hammer:0/0.9/WP_1 |
| 4 / 9 | Att_Battle_2_09 / 1s | Hammer:0/0.75/WP_1, HammerDespawn:0.754545/0 |

| 약칭 | 원본 ParticleSystem 전체 이름 |
|---|---|
| Hammer | FX_MN_RPCZ_00_G.Par_G_RPCZ_00_WP_Hammer_01 |
| WeaponSpawn | fx_mn_rpcz_00_g.Par_G_RPCZ_00_Weapon_Spawn_LOC_INT |
| HammerB | fx_mn_rpcz_00_g.Par_G_RPCZ_00_Hammer_B_LOC_INT |
| UnderDust | FX_CM_00.Dust.Par_E_UnderDustLoop_02 |
| JumpDust | FX_MN_RPCZ_00_G.Par_G_RPCZ_00_Jump_dust |
| HammerDespawn | fx_mn_rpcz_00_g.Par_G_RPCZ_00_WP_Hammer_Despawn_01_LOC_INT |

runtime clip은 기존 Reference에서 rpcz00_att_battle_2_01, rpcz00_att_battle_2_04, rpcz00_att_battle_2_05, rpcz00_att_battle_2_06, rpcz00_att_battle_2_09로 대응돼 있다.

**stage2/7에서 clip 선언 길이는0.266667초인데 회전 Effect notify는4초다.** 먼지 notify도1.292884초와2.657340초에 있다. clip 끝에 맞춰 모든 Effect를 자르는 방식은 이 원본 데이터를 보존하지 못한다. 정확한 복원에는 stage 시간, clip 반복/hold 정책, Effect duration과 종료 정책을 따로 연결해야 한다. Reference의 loop=false만으로 원본의 장시간 stage 실행 정책을 확정해서는 안 된다.

## HammerB 안의 8개 emitter

ParticleSystem의 emitters 배열 저장 순서대로 읽었다. 각 emitter의 첫 LOD 배열 항목을 따라 Required/Lifetime/Spawn/TypeData를 확인한 값이다. 수명 범위는 원본 lookup table의 최소·최대 표본이며, emitter 자체 duration과 다른 값이다. 미기록 property를 false·0·default로 채우지 않았다.

| 순서 / emitter object | 원본 material reference | 원본 mesh reference | emitter duration / loops | 입자 lifetime 표본 | Spawn rate 상수 표본 |
|---|---|---|---|---|---|
| 0 particlespriteemitter_17, 이름 flame | fx_m_mi_00.fx_mi.fx_d_me_flow_02_23_tr | fx_sm_00.fm_a_sprial_003 | 4.5s / 1 | 0.4~0.5s | 5 |
| 1 particlespriteemitter_9 | fx_m_mi_m_00.fx_mi.fx_m_pa_missiletrail_01_10_tr | fx_sm_01.fm_m_trail_002 | 4.5s / 1 | 0.4~0.5s | 5 |
| 2 particlespriteemitter_10 | fx_m_mi_m_00.fx_mi.fx_m_pa_missiletrail_01_10_tr | fx_sm_01.fm_m_trail_002 | 4.5s / 1 | 0.4~0.5s | 5 |
| 3 particlespriteemitter_6 | fx_m_mi_k_00.fx_mi.fx_k_pa_makeflow_02_15_tr | 별도 Mesh TypeData 없음 | 4.5s / 1 | 0.5s | 3 |
| 4 particlespriteemitter_13, 이름 smoke_tail | fx_m_mi_02.fx_mi.fx_k_pa_turbpa_06_tr | 별도 Mesh TypeData 없음 | 4.5s / 1 | 0.4~0.5s | 표본 미기록 |
| 5 particlespriteemitter_3 | fx_m_mi_01.fx_mi.fx_g_pa_symbol_01_1_tr | 별도 Mesh TypeData 없음 | 3.5s / 1 | 0.5~0.6s | 50 |
| 6 particlespriteemitter_8 | fx_m_mi_00.fx_mi.fx_a_pa_db_01_1_ad | 별도 Mesh TypeData 없음 | 4s / 1 | 0.6~0.8s | 50 |
| 7 particlespriteemitter_11 | fx_m_mi_02.fx_mi.fx_j_rgbsplit_01_1_ad | 별도 Mesh TypeData 없음 | 4s / 1 | 0.5~0.6s | 30 |

첫 emitter는 이름 flame, 회전 형상 mesh fm_a_sprial_003, material fx_d_me_flow_02_23_tr를 실제로 참조한다. 뒤의 두 emitter는 fm_m_trail_002 mesh를 사용한다. 즉 “회전 Effect가 있었다”는 이름만 남은 것이 아니라 **어떤 형상을 어떤 재질로 얼마나 생성할지**가 저장돼 있다. 이 구조만으로 최종 색·밝기·투명도가 원본 화면과 같다고 판정하지는 않는다.

두 mesh의 현재 물리 파일도 존재한다. 존재만 확인했으며 로드·렌더링하거나 수정하지 않았다.

- Client/Bin/Resources/Effect/KoukuSaydon/Meshes/fx_sm_00/fm_a_sprial_003.wmodel
- Client/Bin/Resources/Effect/KoukuSaydon/Meshes/fx_sm_01/fm_m_trail_002.wmodel

## 단순 이미지 목록보다 더 많은 분포가 있다

첫 flame emitter에는 Required, Lifetime, Size_Seeded, Velocity, ParameterDynamic, Location, VelocityOverLifetime, MeshRotationRate, Color, ColorScaleOverLife, MeshRotation, TypeDataMesh, Spawn module이 연결돼 있다.

다음은 직접 읽은 작은 발췌다. lookup table은 원본 형식의 숫자 표본을 그대로 기록했으며, packed table의 헤더·기본값을 무시하고 현재 엔진의 vector로 대입해도 된다는 뜻이 아니다.

| source object / property | 실제 값 |
|---|---|
| FX_MN_RPCZ_00_G:export:5355 / emitterduration | 4.5 |
| FX_MN_RPCZ_00_G:export:3485 / lifetime | op2, numElements2, chunkSize2, lookup=[0.4000000059604645,0.5,0.4000000059604645,0.5,0.4000000059604645,0.5] |
| FX_MN_RPCZ_00_G:export:8303 / startvelocity | op2, chunkSize6, lookup=[0,20,0,0,33,0,0,20,0,0,33,0,0,20] |
| FX_MN_RPCZ_00_G:export:4531 / startrotationrate | op1, chunkSize3, lookup=[0,1,0,0,1,0,0,1] |
| FX_MN_RPCZ_00_G:export:4778 / dynamicparams | distortion[0-x], pan[1-2], uv.y_controll[1-x], uvdistort[0-x]와 각각의 시간 분포 |

uv.y_controll[1-x]에는0.5~1.0으로 변하는 lookup table이 있고, pan[1-2]에도0.3~1.7의 표본과 시간축 값이 남아 있다. 단순 DDS 한 장 또는 clip 이름만 얻은 상황이 아니다.

smoke_tail에는 cylinder 기반 spawn 위치, efparticlemodulevortex, bKillOnDeactivate=true도 연결돼 있다. 다른 emitter의 killOnDeactivate는 해당 Required object에 미기록이므로 false로 확정하지 않았다. 이 구성은 이미지·mesh만 골라 일반 sprite 하나로 바꿀 때 놓칠 수 있는 생성 위치·회전·종료 정보다. 현재 renderer의 해당 module 지원, unit conversion, random-seed 일치는 이번에 검사하지 않았다.

## 현재 확인과 남은 복원

| 단계 | 이번 확인 |
|---|---|
| action→clip→particle identity와 시간 추출 | 원본에서 직접 확인. 전수 CSV에도 포함 |
| particle→emitter→module→mesh/material 참조 | HammerB의8개 emitter를 실제 graph에서 확인 |
| mesh 물리 입력 | 위2개 wmodel 존재. native material 렌더링 성공의 증거는 아님 |
| 원본 material shader·texture closure | material reference까지 확인. parent/shader expression/parameter/각 DDS 및 sampling·blend 복원은 이 사례에서 미검증 |
| 종료·분기·별도 gameplay | duration0, stage 전이, weapon 유지/교체·제거, 피해·소환체 정책은 추가 연결 필요 |
| Engine full.restore 생성·현재 P24 연결 | 미실행. 이번 산출물은 조사 문서 |
| 인게임 최종 재질·색·형상 | 사용자 전용 관찰 영역, 미실행 |

이 패턴은 원본 정보를 추출해 복원할 수 있는 구체적인 사례다. 다음 구현은 이 원본 묶음을 기존 full.restore 경로로 변환하고 현재 휠윈드의 animation/stage/부착·종료 정책에 연결하는 작업이다. source 정보가 없는 다른 notify나 child object까지 이 사례 하나로 해결됐다고 일반화하지 않는다.

실행한 검증은 원본 Action/graph/reference JSON 재읽기, action4219708·ParticleSystem exact identity 확인, emitter8개와 첫 LOD 참조 접근, 현재 mesh2개 존재 확인이다. 제품 데이터·C++·Resources 변경이 없으므로 build/publisher는 실행하지 않았다.

전체 목록은 [쿠크 원본 패턴·Effect 전수 조사](2026-09-11_KOUKU_SOURCE_PATTERN_EFFECT_INVENTORY.md)의 CSV들을 기준으로 본다.
