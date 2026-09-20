# 발탄 연출·유령 표현 복구 결과

2026-09-20 현재 저장본 기준 결과다. 최종 Product 빌드와 사용자의 화면 판정은 통합 결과에서 별도로 기록한다. Client와 UI는 이 작업에서 실행하지 않았다.

## 제품 연출 연결

기존 Server pattern/entity/sequence/stage 식별자와 camera clock을 그대로 소비하고, 기존 `CWorldSequencePlayer`가 원본 actor·장비·FX의 생성, seek, 종료를 소유한다. 별도 모델 런타임이나 Client 전투 판정은 추가하지 않았다.

| 구간 | 확인한 원본 | 현재 소비자 |
|---|---|---|
| 입장 | `SCENE06A / Matinee 53`, 24.708초 | `VALTAN_ENTRANCE_CINEMATIC` 3개 stage, 원본 일반·colorless 발탄과 actor64 동시 재생 |
| 버러지들 | `SCENE06A / Matinee 54`, 6.374초 | `VALTAN_TRASH`의 STEP_05/06에서 원본 actor·camera·FX 재생 |
| 2페이즈 진입 | 원본 action420629 `Att_Battle_12_03 + 400ms / Event_02 → SCENE02A / Matinee 52`, actor74와 Camera20 | `VALTAN_ARENA_BREAK_109` 2600ms부터 5.5초 원본 assembly 재생 |
| 사망 | `SCENE04A / Matinee 24`, 23초, `MN_RPBF_02` | 실제 DEAD snapshot/despawn과 `VALTAN_GHOST_DEATH_AUDITION`에서 finite finale 재생, 완료까지 clear UI 대기 |
| 벽 파괴 | `SCENE06A / Matinee 55` 원본 `12_04/12_05` camera | `VALTAN_SIX_PIZZA_106` STEP_04/05의 실제 clip clock에 연결. 원본 WorldSequence roar는 별도 저장·MapTool 재생 경로를 유지 |

2페이즈는 기존 1600ms phase/wall authority와 그 밖의 gameplay field를 보존했다. 원본 컷씬의 마지막 1830ms를 위해 RECOVERY만 870→2700ms로 복구했다. 일반 이펙트 길이로 stage를 자동 연장하는 변경이 아니다. 원본 source assembly 전체를 기존 Server landing anchor로 한 번 평행 이동했다.

camera는 원본 up/roll과 director cut을 보존한다. Client와 publisher의 제한을 512 keys, 문서 1MiB/65,536 values/depth16, FOV 1..179로 일치시켰다. 벽 원본의 stage/Matinee 경계 156ms 차이는 source clock을 늘이지 않고 유지한다.

## Action Workbench 선택과 편집 경계

Valtan Action Workbench에서 다음 저장 pattern을 선택한다. 표시 이름은 현재 `Data/Valtan/Valtan.gameplay.json`의 실제 값이다.

| 확인할 연출 | Pattern ID | 표시 이름 |
|---|---|---|
| 입장 | `VALTAN_ENTRANCE_CINEMATIC` | 발탄 등장 컷신 |
| 벽 파괴 camera | `VALTAN_SIX_PIZZA_106` | 중앙이동 후 6방향 공격 후 피자 패턴 |
| 2페이즈 | `VALTAN_ARENA_BREAK_109` | 중앙 이동 후 2페이즈 컷씬 |
| 버러지들 | `VALTAN_TRASH` | 버러지 패턴 |
| 사망 | `VALTAN_GHOST_DEATH_AUDITION` | 3페이즈 발탄 사망 |
| 유령 부활 | `VALTAN_GHOST_RESPAWN_AUDITION` | 3페이즈 망령화 발탄 부활 |

기존 Stage/Animation 및 typed Details 편집은 Action Workbench가 소유한다. All Effects의 저장된 Pattern view는 이를 참조하며, `Animation Play`는 선택 clip 구간 preview, `Complete Play`는 Valtan Arena의 Server 승인 재생이다. 이 참조 화면에서 저장 composition을 별도로 생성하지 않는다.

Workbench의 정본은 `Valtan.gameplay.json`과 `Valtan.presentation.json`이며 `CBalanceTool::Get_ValtanStageDraft`가 admission된 tree를 읽는다. `Valtan.pattern.json`은 retired v1 참고 자료이고 그 문서의 Publish는 도구가 거절한다. 따라서 그 파일에 남은 RECOVERY=870을 현재 편집 값으로 읽지 않는다. 현재 gameplay 정본과 생성된 `ValtanEncounter.json`은 모두 RECOVERY=2700이다.

원본 cinematic actor와 보조 무기, emitter transform 및 FX 시간 구간은 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json`의 별도 WorldSequence 문서에 있다. 이 assembly의 Object/Sequence 편집은 기존 World/Map 도구 경로이며 Action Workbench의 Stage row 자체가 각 actor slot/FX track을 소유하는 구조로 바뀐 것은 아니다. camera cue는 기존 camera 문서/도구 경계를 사용한다.

## 모델·이펙트 데이터

일반·유령 cinematic AnimSet은 각각 기존 146 clips와 원본 section을 보존하고 입장·버러지들·사망·2페이즈 4개 clip을 추가한다. A/B blend, reverse walk, spine1 이하 upper-body mask, H_Up parent-bone control은 원본 자료로 bake했다.

입장 actor64는 원본 body·weapon과 두 손 FX를 사용한다. body와 무기의 두 visibility 구간을 서로 다른 기존 WorldSequence instance 3개로 저장하여 single binding 계약을 보존했다. 입장 companion들은 동일한 24.708초 clock과 anchor로 prewarm/play/seek한다. per-effect slot에서 실제 binding resource와 model을 조회하고 다음 frame의 motion owner를 사용하는 소비자 수정도 반영했다.

원본 누락 ParticleSystem 33종은 재질 143개 중 기존 89개를 재사용하고 native 3840..3893의 54개를 설치했다. 2페이즈 8종/71 emitters/48 materials에는 native 3894..3911의 18개를 추가했다. 원본 local decal, HDR SceneColor bias와 distortion pass를 각각 해당 carrier에서 복구했으며 shader 근사식이나 광역 prefix 허용은 추가하지 않았다.

7개 이동 emitter 후보는 기존 `sourceTransformTrack`의 원본 Hermite key/tangent/parent/frame을 보존했다. 입장 눈의 단일 vector parameter는 원본 constant `(0,5,12)`로 복구했다. 사망 actor35의 alpha 및 alpha_1은 각각 원본 곡선을 유지하고 소비 횟수대로 곱한다. 선택적 `alphaScaleFactors`는 최대 16개이며, 같은 factor의 중복을 허용하고 미지정 값은 1이다. 기존 `alphaScaleKeys` 읽기·쓰기는 유지했다.

사망 마지막 emitter의 오래된 Detail lifetime `[0,0]`은 원본 seeded distribution의 실제 constant range `[6,8]`로 교정했다. 원본 SourceRecipe는 변경하지 않았다. 원본 EmitterLoops=0인 FX는 저장된 ON/OFF 구간 동안 유지되도록 기존 bounded source loop clock을 연결했다.

## 최종 FX 연결 수와 남은 원본 항목

현재 WorldSequence에는 실제 admission을 통과한 52개 asset의 FX 89개 시간 구간이 연결되어 있다: 입장16, 버러지들8, roar16, 사망39, 2페이즈10. 이 수치는 저장된 source-preview WorldSequence 전체이며, roar16개를 별도의 자동 전투 연출 소비가 완료된 것으로 세지 않는다.

아래 원본 5개 시스템의 7개 시간 구간은 실제 codec/Stage admission 실패로 보류했다. 원본 recipe와 native material 설치본은 보존하고 WorldSequence 연결만 제거하여 나머지 연출 prewarm을 막지 않게 했다. 사용자 마무리 지시에 따라 신규 module consumer나 admission 범위 확장은 진행하지 않았다.

| 원본 구간 | ParticleSystem | 거절 근거 |
|---|---|---|
| Matinee53 actor75, 2260..8309ms | `fx_npc_m_00.par_m_ghostmeteor_01` | locationemitter family/cardinality 계약 |
| Matinee53 actor76, 4439..10593ms | `fx_npc_m_00.par_m_ghostmeteor_loop_02` | Detail lifetime 하한 0이 현재 양수 범위 계약에 맞지 않음 |
| Matinee53 actor77, 0..678ms 및 678..15846ms | `fx_npc_m_00.par_m_gravityarea_01_1` | `particlemodulemeshmaterial` 미지원 |
| Matinee55 actor92, 3515..7003ms | `scene_a.fx.par_j_arktrail_02_cine` | Detail lifetime `[0,0]`이 현재 양수 범위 계약에 맞지 않음 |
| Matinee55 actor98, 3810..4889ms 및 4889..7003ms | `bfx_low_03.lightning.par_c_lightning_001` | `particlemodulesubuvdirect` 미지원 |

위 Matinee는 모두 `SCENE06A` 원본이다. lifetime 0의 원본 의미를 임의 양수로 대체하지 않았다. 사망의 `[6,8]` 교정은 원본 seeded lookup으로 정확한 값이 확인된 별도 항목이다.

## 유령·All Effects·수명 수정

- normal/ghost body·weapon·armor assembly를 transactionally 교체하고, 실패하면 기존 presentation을 유지한다. ghost Respawn1 cue와 원본 model-specific particle override를 연결했다.
- All Effects Full Restore는 저장 cue asset ID와 실제 animation clip을 정확히 연결한다. 입장 3 stages와 사망 1 stage를 포함한 실제 authored pattern/animation row를 기존 typed audition 경로에서 재생한다.
- 3연속 삼각형 반경을 9→13.5m로 변경했다. 지름은 18→27m이며 기존 이동 시간 1.3초를 유지하도록 edge distance/speed를 함께 맞췄다.
- V2 stage cue의 `stopWithClip=false` child는 자기 경과 시간과 이전 stage의 종료 기준을 보존하며 자연 종료한다. `true`, 명시 중지, owner 해제는 기존 정리 계약을 따른다. V1 NATURAL와 이미 시작한 one-shot sound도 일반 stage 전환에서 유지된다. Valtan의 미래 start cue를 이전 stage 밖에서 새로 시작하는 기능까지 추가했다고 주장하지 않는다.

## 실행한 검증

- 실제 CModel Attach와 원본 cinematic pose 검사: 초기 3 clips 208,256 finite matrix checks, phase2 21,248 checks PASS. actor64는 743 frames/59,440 checks, 원본 basis 최대 오차 7.15e-7 PASS.
- 실제 Effect codec/Drawable/Serialize-Parse/Stage: 9개 curve 후보 중 위의 미지원 actor98 한 개를 제외한 8개 PASS. 실제 source transform 772 samples×16 components, 최대 오차 2.0116e-5 PASS. 사망 9 emitters/108 alpha samples 및 bounded factor 거절 검사 PASS. `out/ValtanCurveProbe20260920/curve-probe.log`.
- 최종 연결 asset 전체에 실제 Load/Drawable/Stage admission을 실행했다. 조사 시 58개 중 6개 문서가 거절되어 위 5개 원본 시스템의 연결을 격리했다. 최신 52개 전체 재검사 PASS, failures=0: `out/ValtanCurveProbe20260920/world-admission-final.log`. 실행 환경은 실제 `LOSTARK_RESOURCE_ROOT=Client/Bin/Resources`와 Debug Engine DLL을 사용했다.
- Client 변경 TU와 alpha codec/playback, 최종 Level 및 WorldSequence Objects TU의 격리 Debug compile PASS. Native Particle/Mesh 3840/3904, Decal 및 관련 distortion shader의 실제 fxc PASS.
- 최신 빌드된 하네스 `--valtan-presentation-contract` PASS: `out/RaidRepair20260920/valtan-presentation-contract.log`.
- 전체 Gameplay Validate 65 patterns/280 stages/52 auditions, V2 Publish, Valtan shadow/composition 동기화·검증, ghost primary 10개와 Full Restore metadata 13개 검사 PASS. 최종 통합 Gameplay Publish/Product 빌드는 루트 작업에서 별도 기록한다.
- WorldSequence는 최신 디스크 stable ID 병합, 교체 전 hash 확인, backup 및 atomic 교체를 사용했다. 설치·게시가 실행 중 메모리 draft나 Server를 자동 갱신했다는 의미는 아니다.
- 최종 WorldSequence Publish PASS: SHA256 `11359bf34d02b4c0843702cd6a284f7f673e18091c37fb68fe2e170a3e865a80`, `out/ValtanRestoration20260920/final-complete-world-publish.log`.

원본 소스·recipe·재질 설치와 CPU 수치 검증은 완료된 부분에 대한 증거다. 실제 밝기, camera 전환, bone 부착과 화면 표시 품질의 최종 판정은 사용자가 실행하여 확인한다.
