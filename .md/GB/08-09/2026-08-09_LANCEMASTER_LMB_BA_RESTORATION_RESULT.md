# 2026-08-09 창술사 LMB BA 수동 복원 후보 결과

## 1. 결론

창술사 `34010` BA1~4와 `34510` BA1~3의 Source 추출 및 v13 후보 데이터 준비를 마쳤다. Product에는 게시하지 않았다.

| 상태 축 | 현재 값 | 단계 수 |
|---|---|---:|
| 현재 복원 상태 | `SOURCE_EXTRACTED` | 7 |
| 수동 검증 gate | `MANUAL_VISUAL_PENDING` | 7 |
| 시각 승인 | `NOT_VISUAL_APPROVED` | 7 |
| Product 게시 | 미게시 | 7 |

후보 문서와 현재 worktree codec/playback 코어는 모두 v13 `transformInheritance` 계약이다. 현재 하네스에 Desktop 정본 Resource root를 명시하면 후보 7개가 Debug/Release에서 7/7 통과하고, 최신 self-root 하네스에서는 무환경 7/7을 통과한다. 기존 Product 101개도 Debug/Release에서 Load/Validate/Validate_Drawable 101/101을 통과했다. 다만 이 worktree 자체에는 `Client/Bin/Resources`가 없어 무환경 실행은 불가능하고 화면 승인도 별도이므로 상태는 `SOURCE_EXTRACTED / MANUAL_VISUAL_PENDING`으로 고정했다.

Product, Assembly, Component, skill binding, runtime catalog는 후보를 소비하지 않는다. runtime catalog의 `.restoration-candidate` ID는 0개다.

## 2. 흰색 초승달 RCA

사용자 캡처의 흰색 초승달은 Decal이나 Trail이 아니라 `fm_m_ring_001.wmodel` Mesh다. DDS 누락 fallback도 아니다.

연결된 리소스는 다음과 같다.

- Mesh: `fm_m_ring_001`
- Mask: `fx_m_trail_007`
- Base: `fx_h_atypical_01_1`
- Emissive: `fx_m_noise_003`
- Dissolve: `fx_m_noise_001`
- Noise: `fx_d_noise_030`

원본 scalar `emissive_tex_strength=5000`, power `2`는 실제 Source 수치다. 하지만 현재 grouped shader는 원본 UE3 graph가 아니며 standalone 변환 중 Particle color `[0.2, 0.375, 0.5]`, alpha `0.5`, 위치, 회전, rotation-rate를 잃었다. 결과는 대략 `generic shader × emissive 5000 × white identity tint`가 되어 HDR 순백으로 포화된다.

즉 위치 튜닝으로 고칠 문제가 아니고, missing DDS도 아니다. Source Particle 색과 Material graph semantics가 실행되기 전까지 Material 복원은 미완료다.

## 3. 34010 후보 결과

BA1~3 후보는 다음 세 요소를 가진다.

- visible `fm_m_ring_001` Mesh master 후보 1
- visible generic weapon-bone Trail audition 1
- invisible fallback-blocked Ribbon queue 1

BA4에는 위 세 요소와 별도로 impact Mesh 1 + Sprite 3 stack을 추가했다. Sprite 3개는 v13에서 impact Mesh의 최종 행렬을 상속하도록 선언했다.

AnimTrail의 Source exact 범위:

- BA별 notify start/duration
- `EFData_AnimNotify_Trails`
- `ParticleModuleTypeDataAnimTrail`
- `midcontrol`, tiling/tessellation source fields
- source material 및 alpha/dissolve/emissive/noise texture identity

AnimTrail의 미복원 범위:

- `b_weapon_rhand`는 프로젝트 추론 mapping이며 Source endpoint가 아님
- weapon tip/dual-edge offset
- 실제 two-edge width와 tangent geometry
- source shader와 dynamic parameter
- generic Trail의 width, point life, max point, sampling 의미

`WP_FLM_1_Battle`은 Ribbon event의 직접 근거이지 AnimTrail endpoint 근거가 아니다. receipt에서 둘을 분리했다.

Ribbon은 BA마다 typed Trail queue 요소를 만들었지만 visible은 끈 상태다. source material `fx_m_pa_ribbonmaster_01_9_tr`이 `YGI3SB3OBJ3O1MGUMP6QMP8B5.upk`와 `ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk` 두 후보로 해석되며 exact package/DDS join이 닫히지 않았다. 두 후보는 같은 alias가 아니다. YGI는 `fx_b_trail_004`/emissive 300 계열이고 ZHJ4는 `fx_bg_lightbeam_falloff_03`/emissive 44 계열이라 화면 의미가 크게 다르다. generic Base alias를 붙이지 않고 Imported의 `effect.ue3.fallback-blocked.v1`을 보존했다.

현재 generic Trail renderer는 단일 body bone 위치를 샘플링하는 one-centerline polyline이다. 실제 AnimTrail/TypeDataRibbon runtime 복원 완료가 아니다.

## 4. 34510 후보 결과

BA1~3 각각 Mesh 3 + Sprite 3을 준비했다.

- terminal master: emitter 9 `fm_d_plane_003`
- companion: emitter 8/12 Mesh, emitter 7/2/11 Sprite
- 모든 visible 요소의 stage-local start: `0.08 s`
- 모든 companion: 같은 group/root attachment에서 master final matrix 상속 선언
- Source 상대 Transform: receipt evidence only
- emitter 2의 random distribution 및 `+0.1 s`: 실행하지 않음
- textureless emitter 0/1: cross-emitter DDS proxy 없이 blocked
- `FX_CM_02.Light.Par_MP_Light_01`: 현재 slice 밖의 미복원 Source carrier

Scale, 음수/비균일 Scale, pivot, rotation-rate, motion/orbit, multiplicity, Light runtime, source shader graph는 복원 완료가 아니다.

## 5. 전수 renderer 분류 반영

추가 Source 감사 결과를 다음처럼 반영했다.

| 축 | Imported canonical/event | 현재 Product |
|---|---:|---:|
| meshParticle | 1187 | standalone Mesh proxy 872 |
| spriteParticle | 3564 | Sprite proxy 1231 |
| Ribbon | 95 | 잘못 Sprite로 평탄화 3, Trail 0 |
| Decal | 79 | Decal 46 |
| Light | 218 | 0 |
| Screen Post | 89 | 0 |
| animation Trail notify | TrailGhost 72 + Trails 8 | Trail 0 |

현재 2168 Product 요소는 Mesh 888, Sprite 1234, Decal 46이며 Particle/Trail/Light/Post는 0이다. 물리 에셋 누락과 visible Base 누락은 모두 0이므로 문제는 asset missing이 아니라 renderer semantics와 평탄화다.

고위험 Trail 후속 대상은 DimensionMaster A `2050210`, LanceMaster `34010/34570`, Artist `31210/31420/31460`, Warlord `17820`이다. 이 작업에서는 34010 Source 추출과 fail-closed 후보까지만 다뤘으며 다른 Product를 완료 처리하지 않았다.

## 6. 구현 산출물

- `Tools/EffectPipeline/seed_lancemaster_lmb_ba_candidates.py`
- `Tools/EffectPipeline/test_seed_lancemaster_lmb_ba_candidates.py`
- `Data/Effects/Authored/effect.lancemaster.skill.34010.ba1~4.restoration-candidate.effect.json`
- `Data/Effects/Authored/effect.lancemaster.skill.34510.ba1~3.restoration-candidate.effect.json`
- `Data/Effects/AuthoredCorrections/Generated/TrackB/LanceMasterLmbCandidates/*.candidate-receipt.json`
- `Data/Effects/AuthoredCorrections/Generated/TrackB/lancemaster-artist.decal-inventory.json`

생성기는 기존 후보 7개의 raw SHA를 전부 먼저 확인한다. 하나라도 사용자 저장 또는 포맷 변경이 있으면 후보와 metadata 전체 갱신을 거부한다. visible 요소는 WModel/Base를 검사하고, hidden no-Base 요소는 fallback-blocked profile만 허용한다.

## 7. 자동 검증

| 검증 | 결과 |
|---|---|
| Python compile | PASS |
| Candidate generator unit test | PASS, 11 tests |
| Generator `--check` | PASS, 15 outputs |
| v12→v13 / Ribbon migration overwrite guard | PASS |
| visible WModel/Base 및 hidden fallback-blocked 정책 | PASS |
| Product runtime catalog candidate ID | PASS, 0 |
| Candidate JSON format | PASS, 7개 모두 version 13 |
| 현재 worktree v13 codec로 후보 load | PASS, Debug/Release 각각 7/7; Desktop 정본 Resource root 명시 |
| 현재 worktree 하네스 무환경 후보 load | EXPECTED FAIL, Debug/Release 각각 0/7; 로컬 Resources 부재와 self-root 미재베이스 |
| 최신 Desktop self-root harness로 후보 load | PASS, 수동 환경 변수 없이 7/7 |
| 최신 Desktop `--effect-products-fast` | PASS, Debug/Release 각각 101/101 Load/Validate/Validate_Drawable, failures 0 |
| 최신 Desktop 전역 ProjectAudit | PASS, 89 checks |
| v13 codec/playback 선택 컴파일 | PASS, Debug/Release |
| 이 격리 worktree의 전역 `Invoke-ProjectAudit.ps1` | FAIL(10), 미재베이스 Resources/A authority/G09/publisher 경계 |

처음 단독 실행에서 보였던 resource/material invalid 오류는 codec 회귀가 아니었다. `ClientFrontendHarness --effect-document` 모드가 `LOSTARK_RESOURCE_ROOT`를 초기화하지 않아 `Tools/ClientFrontendHarness/Bin/.../Resources`를 잘못 찾은 harness 문제였다. 최신 Desktop self-root 수정본에서는 수동 환경 변수 없이 후보 7개와 기존 Product 101개 회귀가 통과한다. 이 worktree에는 다른 dirty harness 변경을 보호하기 위해 해당 소스 파일을 재베이스하지 않았으므로, 로컬 standalone harness 검증은 명시적 Resource root를 사용한다.

최신 Desktop/source 정본의 전역 ProjectAudit은 89 checks PASS로 닫혔다. 다만 이 격리 worktree에서 재실행한 결과는 FAIL(10)이다. 주요 원인은 `Client/Bin/Resources` 부재로 인한 map/actor/Decal asset 검사 실패, source 세션 소유 `DimensionMaster.ba-r-master-carrier.materialization.json` 부재, 아직 재베이스하지 않은 G09 workbench/cross-document 경계, 그에 따른 FourClass publisher 테스트 오류다. Track B 후보 단위 검증과는 분리된 미재베이스 실패이며 해당 전역 파일을 임의로 보충하지 않았다.

또한 로컬 worklist는 아직 이전 상태 집계 `95/5/1`을 담고 있고 최신 Desktop 정본은 `95/6/0`이므로, receipt의 `globalSummary`는 로컬 snapshot provenance일 뿐 Track B 승인 수치가 아니다. 이 전역 worklist 파일은 source 세션 소유라 본 작업에서 덮어쓰지 않았다.

## 8. 수동 검증과 다음 단계

수동 이미지 판정과 Character Select 검증은 수행하지 않았다.

다음 단계는 다음과 같다.

1. Track B 후보/receipt 15산출물을 Resources와 최신 workbench가 있는 Desktop 정본에 수술식으로 통합한다. 전역 rollout/worklist/A 파일은 source 정본을 유지한다.
2. 사용자가 Solo Element -> Solo Group/Trail -> Character Select 순서로 위치, 회전, 크기, 겹침을 조정한다.
3. 저장된 후보를 다시 Load/Validate한 뒤 수동 조립 상태를 기록한다.
4. Ribbon physical package/DDS/shader와 실제 AnimTrail/Ribbon endpoint runtime을 닫는다.
5. 사용자 승인 뒤에만 `VISUAL_APPROVED`와 Product 게시를 수행한다.
