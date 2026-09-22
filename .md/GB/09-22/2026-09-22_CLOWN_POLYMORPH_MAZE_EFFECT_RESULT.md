# Clown POLYMORPH Q/W/E와 카드미로 Q/LMB 이펙트 후보

## 완료 상태와 반영 경계

2026-09-22 후속 요청의 실제 반영 승인으로 Clown `POLYMORPH` Q/W/E와 캐릭터 `MAZE` Q/LMB 연결 Data 및 Resources 9개를 최신 저장본에 설치했다. 통합 revision 2194의 Kouku domain publish도 PASS했다. 아래 후보 작성·검증 기록은 설치 전 이력이다. Product 출력은 실행 중 Client/Server 때문에 최종 교체 대기이며, Client 실행·조작·Reload는 수행하지 않았다. CPU codec/playback 검증과 GPU 표시·사용자 화면 판정을 구분한다. 최종 설치·게시·빌드 상태는 [통합 결과](2026-09-22_KOUKU_GATE1_PATTERN_RESTORATION_RESULT.md)를 따른다.

후보 정본은 `out/ClownInteraction20260922/candidate-manifest.json`이다. `candidate/Data`에는 효과 문서 5개와 Clown interactionbinding 문서가 있다. `baseline`에는 후보 작성 시 읽은 기존 저장본을 보관했다. manifest는 기존 문서의 SHA-256과 element stable ID append, `(mode, clip)`별 binding field 변경, catalog/tree append, 신규 Resources 9개의 hash를 분리한다. 승인 후 최신 저장본을 다시 읽어 필요한 필드만 병합해야 하며 후보 전체 파일로 기존 저작본을 덮어쓰면 안 된다.

## 효과와 애니메이션 연결

| 슬롯 | 후보 asset ID | 구성과 시작 시점 |
|---|---|---|
| Clown Q | `effect.kouku.clown.polymorph.q` | 기존 폭탄 모델 투척 1 element와 기존 native 폭발 10 elements. 원본 Action 43340의 Projectile 413401 발생 0.798826초에 투척을 시작한다. |
| Clown W | `effect.kouku.clown.polymorph.w` | 원본 `sk_02` 5 elements를 0.097010초부터 `WP_1 → b_wp_1`에 부착한다. 원본 `sk_02_1_loc_int` 19 elements는 0.590822초, 기존 서커스 공 1 element는 0.600309초에 시작한다. |
| Clown E | `effect.kouku.clown.polymorph.e` | 원본 `sk_04` 흰색 효과 12 elements를 0.471154초부터 `Bip001-R-Finger11 → bip001-r-finger11`에 부착한다. 선물상자는 0.470165초부터 원본 `respawn_1` 재생 후 `idle_normal_1`로 이어져 총 10초 유지한다. |
| MAZE Q | `effect.kouku.cardmaze.q` | 요청한 `fx_mn_rpcz_00_g.par_g_rpcz_00_jump_loc_int` 10 elements를 기존 빈 저작 문서에 추가한다. 기존 게임 Q 타격 시점 1.0초에 맞춘다. |
| MAZE LMB | `effect.kouku.cardmaze.lmb` | 요청한 `effect.kouku.mario.clown.hammer.end` 2 elements를 기존 빈 저작 문서에 추가한다. 기존 게임 LMB 타격 시점 0.4초에 맞춘다. |

Q의 투척 궤적, 재사용 폭탄과 폭발 조합은 `PROJECT_TUNED`다. 원본 Q 전용 파티클을 완전히 복원했다는 의미가 아니다. 투척은 초기 높이 1.1m, 전방 0.3m에서 수직 속도 1.7m/s·전방 속도 5m/s·중력 −10m/s²로 0.65초 진행한다. 기존 폭발은 그 종료 지점에 배치한다. W 공의 기존 7m 낙하 곡선은 이 occurrence에서만 1/14로 줄였으며 native 재질과 mesh를 재사용한다. 측정된 공 중심 높이는 0.671429~1.4m다. 원본 library 문서는 수정하지 않았다.

MAZE LMB는 기존 NPC의 별도 +90도 yaw를 포함한 결과를 이 occurrence의 snapshot root yaw 0도로 옮겼다. MAZE Q의 원본 X-forward basis는 snapshot root −90도로 변환한다. 다른 효과 전체에 회전이나 배율을 적용하지 않았다. 두 MAZE 효과의 실제 화면 방향·크기는 사용자 확인 대상이다.

LanceMaster, GunSlinger, Slayer, Artist, DimensionMaster, Warlord, GuardianKnight의 기존 MAZE 바인딩은 이미 Q/LMB 두 공유 asset ID를 참조한다. 이 7개 파일은 변경하지 않아도 추가 element를 소비한다. Clown의 legacy MAZE 1-slot 후보는 현재 설치 모델에 존재하는 hammer clip을 그대로 써 Q/LMB 2-slot으로 확장한다. POLYMORPH Q/W/E는 기존 clip과 playRate를 보존하고 `effectAssetId`만 추가한다.

실제 소비자는 `CCharacter::Load_InteractionAnimationBindings → EffectCatalog load target → Server interaction snapshot → Play_InteractionAnimation의 effect spawn`이다. CharacterModelWorkbench도 같은 `effectAssetId`를 0초 cue로 읽고 effect 내부 시작 지연을 소비한다. Workbench의 두 MAZE 항목이 모두 `LMB / Q`로 보이던 라벨은 index 0 `Q`, index 1 `LMB`로 구분했다.

## 원본 선물상자와 native 재질

원본 설치 경로 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame`에서 Action `MN_RPCZ_00-1.loa`, SkillEffect 413406, Npc 480722를 대조했다. E는 원본 모델 `MN_PPPP_00`을 10,000ms 소환한다. UModel의 원본 glTF와 PSA를 기존 `build_umodel_gltf_psa.py → ModelAssetConverter → retime_wmodel_ticks.py` 경로로 변환했다.

선물상자는 원본 10 joints, cooked root를 포함해 11 bones, 1,291 vertices, 20 clips이다. cooked cm 좌표는 model cue에서 preScale 0.01을 한 번 적용한다. 실제 가중치가 있는 5개 bone의 inverse-bind × bind-world 오차 최댓값은 `5.814291e-5`로 `1e-3` 이하다. 가중치가 없는 effectname/cameratarget helper의 offset을 피부 bind-pose 검증으로 오인하지 않았다. 근거는 `gift-stage/gift.report.json`, `gift-model-evidence.json`이다.

재질 `mn_pppp_00.mat.mn_pppp_00_mi`의 parent는 `efbasematerial_prologue.ch.monster.base.monster_base_msk`다. native selected map key는 `fd6df5a0ab9a8c760ad61c08c3fdf6d2cc76daa03ad2ce52c053427504c74dc0`, Base pixel shader는 `d29e975c521d17458747d3450bbf5ed5`, Light는 `6d53b407f618b947baf08379872cb1a7`이다. 기존 `build_vehicle_source_material.py`로 `source.character.monster-fd6df5a0ab9a.v1`, program 109를 생성했다. 독립된 모델 렌더링 경로는 추가하지 않았다.

program 109는 기존 마지막 CSO cohort 84~108을 84~109로 확장해 연결했다. `SourceCharacterMaterialParameters.h`, Engine Model/Shader admission, native_shader_dispatch 범위를 함께 갱신했다. Base/Light generated leaf와 dispatcher는 Engine 정본과 Client mirror에 반영했다. Light109의 UV/light/view/source-position varying은 2/3/5/6이므로 `Shader_SourceCharacterMaterial.hlsli`의 해당 LIGHT_PASS 분기에 109만 추가했다. generic varying 기본값을 그대로 사용하지 않는다.

실제 기존 shader wrapper는 `Engine/Bin/ShaderFiles/Shader_Deferred_SourceGroup084.hlsl`, `Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary_SourceGroup084.hlsl`, `Client/Bin/ShaderFiles/Shader_VtxMeshBinary_SourceGroup084.hlsl`의 3개다. 모두 기존 프로젝트 등록을 사용한다. root 통합 담당의 세 wrapper scratch fxc `fx_5_0` 컴파일과 Engine Model/Shader 2 TU 컴파일에 모두 성공했다. fxc error 0건이며 기존 source의 X4000 potential uninitialized 경고는 남아 있다. 근거는 `out/ClownInteraction20260922/shader-compile`과 통합 RESULT다. 사용자 요청에 따라 Product 링크와 배포는 대기한다.

## 검증 결과

`out/ClownInteraction20260922/native_probe.exe`는 기존 Product codec/playback object를 재사용하고, 실제 family 검증 소비자인 `WorldSequenceDocument.cpp` 1 TU를 현재 program 109 header로 다시 컴파일했다. 후보 Resource overlay 93개 파일은 out 안에만 두었고 runtime Resources는 교체하지 않았다. 후보 5개 모두 `Stage_ProductLoadTarget → Stage_Document → 60Hz Playback`에 성공했다.

| 문서 | elements | 총 duration | peak particles | 마지막 visible particle |
|---|---:|---:|---:|---:|
| MAZE LMB | 2 | 1.8초 | 2 | 0.783333초 |
| MAZE Q | 10 | 3.5초 | 56 | 2.416667초 |
| Clown E | 12 + model cues 2 | 10.470165초 | 39 | 1.6초 |
| Clown Q | 11 | 6.448826초 | 39 | 2.533333초 |
| Clown W | 25 | 4.600309초 | 129 | 2.616667초 |

11,254 particle 샘플에서 transform/alpha finite 및 capacity 검사 실패 0건이다. 이 수치는 모델의 GPU 표시나 pixel fidelity 결과가 아니다. Q 폭탄은 0.8초 샘플에서 `(0,1.12556,0.383333)`, 1.433333초 샘플에서 `(0,0.038333,3.549999)`에 있다.

W/E 부착 검증은 synthetic anchor가 아니다. 설치 Clown WModel의 해당 실제 clip을 60Hz로 계산해 `b_wp_1` 91 samples, `bip001-r-finger11` 61 samples를 공급했다. CModel과 같은 preScale `0.017×0.709 = 0.012053`, yaw −90도를 적용했으며 두 bone basis 길이는 약 1.20530이다. 첫 bone 위치는 W `(0.269901,1.086244,-0.070457)`, E `(0.200396,1.083790,-0.041256)`이다. 근거는 `anchor-evidence.json`과 `anchor_samples.h`다. clip이 끝난 뒤 probe는 마지막 실제 pose를 유지하므로 이후 locomotion 전환 시각까지 검증한 것은 아니다.

`test_source_character_program_groups.py` 3개 테스트와 해당 source 변경의 `git diff --check`에 성공했다. 상한을 넘어선 미등록 fixture는 stale 100에서 110으로 교정했다. 후보 JSON 파싱과 참조 리소스 93개 존재 검사도 성공했다. 전체 빌드와 사용자 화면 검증은 수행 완료로 기록하지 않는다.

## Resources 전달 목록

설치 전 후보 루트는 `out/ClownInteraction20260922/candidate/Resources`다. 대상은 아래 Resources-relative 경로 9개이며 SHA-256은 manifest에 있다. W 공 및 source 효과의 기존 리소스는 다시 배포하지 않는다.

| asset ID | bytes |
|---|---:|
| `Character/KoukuSaton/MN_PPPP_00/MN_PPPP_00.wmodel` | 516484 |
| `Character/KoukuSaton/MN_PPPP_00/textures/mn_pppp_00_cm.dds` | 131200 |
| `Character/KoukuSaton/MN_PPPP_00/textures/mn_pppp_00_d.dds` | 262272 |
| `Character/KoukuSaton/MN_PPPP_00/textures/mn_pppp_00_n.dds` | 262272 |
| `Character/KoukuSaton/MN_PPPP_00/textures/statefx_default.dds` | 524416 |
| `Effect/KoukuSaydon/Clown/Bomb/Bomb.wmodel` | 45684 |
| `Effect/KoukuSaydon/Clown/Bomb/textures/mn_rhcn_01_d_loc_int.dds` | 524416 |
| `Effect/KoukuSaydon/Clown/Bomb/textures/mn_rhcn_01_n.dds` | 1048704 |
| `Effect/KoukuSaydon/Clown/Bomb/textures/mn_rhcn_01_s.dds` | 262272 |

Q의 Bomb 4개 파일은 현재 설치된 MarioProps/Bomb의 동일 bytes 복제다. Effect element resource 계약은 `Map/`를 허용하지 않으므로 `Effect/` 아래에 후보를 만들었으며 계약을 느슨하게 바꾸지 않았다. Data manifest와 모델/texture 후보는 아직 설치되지 않았다. 사용자가 저장본 기준 반영을 승인하면 최신 디스크 stable ID 병합, hash 재확인, backup·원자적 교체·실패 시 자기 변경 rollback 후 domain publish를 수행하고 실행 중 메모리 Reload 여부는 사용자에게 맡긴다.
