# 호랑이·말 애니메이션과 창술사 ALT V 통합 결과

## G00. 확인한 결함

사용자 첨부 화면의 말은 앞다리와 몸이 서로 다른 방향으로 꺾여 있었다. 실제 원본 패키지의 PSA와 설치 모델을 비교한 결과, 이전 PSA→glTF의 자식 본 quaternion conjugate 누락이 호랑이·말에 공통으로 남아 있었다. 단일 local rotation이나 FPS, 가중치 문제로 추정해 처리하지 않았다.

말 section0~3은 네 마리가 아니라 한 골격의 네 재질 조각이다. 기존 사용자 편집 clip2에서 section0만 `[90,-270,297]`, 나머지는 `[0,-90,0]`인 점도 조립을 깨는 별도 원인이었다. 기존 네 clip 문서는 그대로 보존하고 새 통합본만 한 동물의 공통 transform으로 맞췄다.

후속 사용자 관찰은 교정된 호랑이의 점프는 좋아졌지만 앞으로 뛴 뒤 뒤로 돌아온다는 것이었다.
원본 `b_root`의 수평 이동과 loop wrap을 실측해 두 Artist D cue에만 optional XZ root-motion
suppression을 적용했다(G06). 이 후속 소스는 focused compile·수치 검증을 통과했고 최신 통합
Debug Product 빌드는 2026-09-11 23:23:36 KST에 Engine/Shared/Server/Client 모두 PASS했다.
이번 수정이 포함된 화면 판정은 **USER_PENDING**이다.
사용자는 먼저 버그 수정 EXE를 직접 확인한 뒤 PR merge와 리소스 공유를 진행하기로 했다.
현재 PR merge/main sync와 외부 공유는 보류하며, 로컬 준비·CPU 검사와 배포 승인을 구분한다.

## G01. 원본 재추출과 기존 자산 교체

원본 `SK_SDM_TIG_00`과 `SK_FLM_HOR_00` 패키지에서 다시 추출했다. 공용 `Tools/ActorXAssetCooker/build_umodel_gltf_psa.py`의 이미 교정된 root 유지/child conjugate/mesh hierarchy/FLOAT weights 경로를 사용했다. 말의 PSA와 glTF joint는 같은 이름 집합을 갖지만 순서가 달라, 명시적인 `--allow-bone-order-remap`을 추가했다. default strict 검사와 skin JOINTS/inverseBind/hierarchy 순서는 유지하며 누락·중복 이름을 거부한다. 사용법과 기존 테스트도 갱신했다.

다음 Resources 5개를 기존 경로에 원자 교체했다. 이전 bytes는 `out/TigerHorseAnimation20260911/backup`에 보존했고 Git 추적·Drive 업로드는 하지 않았다.

- `Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel`: 37 source joints/38 cooked nodes/7 clips.
- `Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section0~3.wmodel`: 각각 151 source joints/152 cooked nodes/3 clips.

다섯 파일의 geometry/UV/weights/inverseBind(WMSH), skeleton/rest hierarchy(WSKL), 기존 material(WMAT)와 section table은 byte-identical로 유지했다. position/scale/time keys, duration과 30tick rate도 바꾸지 않았다. WANM의 교정된 animation quaternion만 교체했다. 이번 설치본에 잘못된 skin weights가 있었다고 주장하지 않는다.

PSA와 최종 cooked rotation 92,367표본을 대조한 최대 성분 오차는 호랑이 약 1.41380→1.68481e-6, 말 약 1.41408→4.37251e-6이었다. q와 -q의 동치를 제거한 값이며 각도 단위가 아니다. root 회전은 원본과 최대 1.11e-16로 같았다. 전체 원본 frame의 7,646,277 vertex 변환은 finite였다. 실제 Engine CModelDecoderRegistry로 설치 5파일/19clips/92,367rotation keys를 다시 읽어 통과했다. converter 기존·추가 13검사도 통과했다. 수치 일치가 사용자 화면의 visual fidelity 승인을 대신하지 않는다.

## G02. Artist D 두 호랑이와 반복

`effect.artist.skill.31490.full.restore`에 두 ModelCue를 반영했다. 왼쪽은 기존 `artist_31490_tig_source_projectile`, 오른쪽은 `_right` stable ID다.

| 입력 | 반영 값 |
|---|---|
| 위치 | 좌 `[-0.8,1,1.5]`, 우 `[0.8,1,1.5]` m |
| 회전 | 양쪽 `[0,-90,0]` degrees |
| 이동 속도 | 양쪽 `[0,0,3.25]` m/s, 기존 6.5의 절반 |
| animation | `sk_cloudtiger`, 원본 0.733333초를 반복 |
| cue 시작/길이 | 기존 1.14660001초 / 1.69230771초 유지 |
| loop/hold | `loop=true`, `holdLastFrame=false` |
| 수평 root motion | 두 cue 모두 `suppressHorizontalRootMotionBone="b_root"`; 원본 Y 점프 유지 |

source Y-up/+X-forward를 owner Y-up/+Z-forward에 맞췄다. 잘못된 기존 upright basis에 화면에서 보인 30도만 추가하는 방법을 쓰지 않았다. 주변 particle 26행, material, delay와 전체 cue 길이는 보존했다. 기존 Artist generator는 현재 full 문서의 modelCues를 복사하므로 변경할 필요가 없었다.

기존 holdLastFrame=false는 loop가 아니었으며 clip보다 긴 cue를 거부하거나 마지막 pose에서 멈췄다. Authoring DTO/Codec/Renderer에 optional `loop=false`를 연결하고 loop+hold 동시 사용을 거부한다. Render와 bone anchor가 공유하는 Sample_ModelCuePose에서 animation local time만 clip 길이로 감싸며 velocity/revolution은 cue elapsed time을 계속 사용한다. backward seek도 같은 effect clock을 사용한다. prepared renderer 재사용에서도 loop를 끄고 clip보다 긴 cue로 바꾸는 잘못된 편집은 거부하고 기존 상태를 유지한다.

Effect Tool v1 Model/Summon detail의 `Loop Animation`/`Hold Last Frame`은 배타 토글이다. Local Position/Rotation과 함께 velocity/revolution도 편집해 같은 DTO에 저장한다.

최초 loop 구현 단계의 Codec Load/Save/reload, 잘못된 저장의 기존 파일 보존, loop 생략 시 이전 hold 의미가 통과했다. 실제 설치 CModel 두 clone과 production Sample_ModelCuePose를 사용한 15표본에서 반복 pose 최대 차이는 9.83477e-7, 두 호랑이 같은 시각의 차이는 0이었다. 서로 다른 위상의 pose 차이 0.317658로 실제 달리기 변화도 확인했다. +X→owner +Z, backward seek, 기존 hold 회귀, 실제 Artist 26 Elements+2 ModelCues의 Renderer resource Stage/재사용 timing 거부/재Stage도 통과했다. 이 검사는 loop 시간의 일치이며 원본 수평 root motion이 없는지를 검사한 결과는 아니었다. 후속 XZ suppression 검증은 G06을 따른다. draw와 사용자 화면을 검사한 결과는 아니다.

## G03. LanceMaster ALT V 하나의 full restore

새 `effect.lancemaster.skill.34630.full.restore`는 현재 사용자 편집된 clip1~4에서 378 Elements와 12 ModelCues를 모았다. clip1에서 사용자가 삭제한 6개를 복구하지 않았고 원본 네 파일은 byte 그대로 보존했다. `Tools/EffectPipeline/combine_lancemaster_altv_full_restore.py`가 별도 출력에 합치는 재현 도구다.

실제 캐릭터 WModel의 clip 시간에 따라 시작점은 `[0,2234,4434,5434]`ms다. 4개 animation 길이는 총 6801ms이며 emitter tail을 포함한 저장 sequence 길이는 12436ms다. source module 내부 시각/RNG/material/attachment는 유지하고 element/cue의 외부 startDelay만 옮겼다. 기존 camera 3shots와 살아 있는 localOnly 45 IDs를 같은 시간축에 합쳤다.

Catalog/ResourceTree와 Client 96.DataFiles의 Effect 및 Sequence None 항목에 등록했다. 첫 `flm_sk_super_squalllance_01`의 animevent 하나가 통합 effect를 NATURAL로 실행하며 뒤 세 clip에서는 중복 spawn하지 않는다. SourceBoneImportScaleNormalization의 기존 exact-ID 경로에도 새 full만 추가했다.

말의 12 cues는 각 stage별 네 재질 section으로 구성되고 모두 source +X를 owner +Z로 보내는 yaw -90을 사용한다. 기존 단일 문서의 ModelCue 최대 8 제한은 통합본을 막으므로 16으로 늘렸다. stable ID, path, clip, timing과 중복 검사는 유지한다.

정적 연결 검사에서 원본 네 파일 SHA 보존, 378 element의 외부 startDelay 외 의미 보존, 12 cues의 stage 시작과 공통 회전, 50 physical Resources 존재, camera 555,827bytes, 단일 Product event, Catalog/Tree 및 프로젝트 등록이 통과했다. ModelCue 부착 62행은 14 unique anchor에 연결되며 한 anchor ID에 서로 다른 owner/bone/socket이 겹치는 경우는 없었다.

최신 실제 Codec으로 통합 378 Elements/12 ModelCues의 Load/Save와 실제 Renderer resource Stage가 통과했다. 17개 cue의 상한 거부도 확인했다. 별도 실제 CEffectRecoveryCamera parser/sampler에서 기존 sidecar와 합친 3shots를 4333개 시점의 회전·이동 root에서 대조해 eye/look/up/FOV 차이가 1e-4 이내였다. clip offset 중복 없이 적용됐으며 camera overlap 거부도 확인했다.

최신 DTO/Codec/Playback으로 전체 12.436초를 60Hz·749프레임 평가했다. 378 Elements/4082 source modules에서 171,429 particle rows, peak 538, evaluated field hash `bcf35975c3e19ea3`를 얻었다. 인공 moving root/anchor를 사용한 CPU 회귀 검사이며 실제 본 부착·GPU draw·게임 FPS의 결과로 사용하지 않는다.

## G04. 거대 창과 부착 편집

거대 창 후보는 `fm_x_flm_gdr_01` MeshParticle이며 네 clip에 관련 occurrence가 다섯 개 있다. Model/Summon 목록의 말과 다른 요소다. 원본은 Midcontrol→`b_weapon_rhand`와 socket offset을 사용한다. Play All과 Play Saved Effect는 현재 같은 recovery/Sequencer/document factory를 소비하므로 별도 거대 창 spawn 경로를 추가하지 않았다. 새 통합 문서는 네 clip animation과 effect의 시작 시간을 함께 연결한다.

기존 Tool은 source MeshParticle의 Follow Attachment를 숨겨 source socket 보정값을 편집할 수 없었다. 원본 source-contract가 아닌 해당 요소의 기존 detail draft/commit 경로를 열어 `Socket Offset (Meters)`, `Socket Rotation (Degrees)`, Bone/Owner yaw를 편집할 수 있게 했다. 수정한 요소는 자신의 stable ElementId를 runtimeAnchorSlotId로 사용해 같은 Midcontrol을 공유하던 다른 요소와 분리한다. 원본 0.4m socket offset 등은 근거 없이 덮어쓰지 않았다.

원하는 거대 창의 최종 위치가 손의 무기 부착인지 앞 바닥 고정인지 사용자에게 확인 중이다. 화면상 원하는 수치가 확정되지 않았으므로 임의 위치·회전을 반영 완료로 기록하지 않는다.

## G05. 빌드와 사용자 확인 경계

최초 재추출·loop·ALT V 통합 단계의 Codec/Renderer와 converter/모델 decode focused compile 및 위 actual ModelCue·camera 수치 검사를 완료했다. 해당 Product Debug 빌드가 2026-09-11 21:15 KST 종료 0으로 완료됐다. Engine→Shared→Server→Client 모두 PASS이며 compile/link error 0, Client EXE와 Engine DLL·CSO·runtime dependency 배포까지 완료됐다. ModelCue 상한 16 변경 뒤 Codec.cpp도 21:13:59에 다시 컴파일되어 해당 EXE에 포함됐다. 기존 warning들은 유지했고 warning-free나 Release 빌드 완료로 기록하지 않는다. 이 receipt는 뒤에 추가한 G06 suppression과 Workbench assertion 수정까지 포함한 증거가 아니다.

G06 suppression과 Workbench 수정까지 포함한 최신 Debug Product 빌드는 2026-09-11
23:23:36 KST에 Engine/Shared/Server/Client 모두 PASS했다. 증거는
`out/BuildPipeline/runs/20260911T142336444Z-debug-product.json`이며 총 867183ms,
`missingRuntimeInputs=[]`다. Client.exe는 23:23:36 KST의 54,017,536 bytes 산출물이다.
사용자 화면은 USER_PENDING이고 PR merge/main sync·외부 공유 보류는 유지한다.

해당 단계의 `git diff --check`와 변경 프로젝트 두 XML parse도 통과했다. 제품 빌드 증거는 `out/BuildPipeline/runs/20260911T121509244Z-debug-product.json`과 `out/LoadingFreeze20260911/product_debug_final.log`다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았다. 당시 종료 시 Server/Client 정지 기록과 현재 프로세스 상태를 혼동하지 않는다. 최신 통합 빌드 완료를 안내했으며 Visual Studio Debug x64의 `Server + Client` profile에서 사용자가 Ctrl+F5로 실행한다.

사용자 확인 경로는 F1 → Effect Tool v1 → Artist D full restore 또는 `LanceMaster ALT V Full / Animation + Horse + Camera` → Play All이다. 캐릭터 ALT V에서도 하나의 통합 문서를 실행한다. 첫 이미지 거대 창은 Mesh Particle 목록에서 찾고 해당 Element Follow Attachment를 사용한다. 최종 말 달리기·호랑이 정면 배치·거대 창의 화면 결과는 사용자의 관찰이 필요하다.

근거는 `out/TigerHorseAnimation20260911/result_section.md`, `validation.json`, `decode_probe.log`, `artist_d/`, `out/LanceAltVCombined20260911/composition_verification.json`과 `Effects/combine_receipt.json`에 있다. 선행 로딩·Profiler 결과는 같은 날짜의 `2026-09-11_LOADING_FREEZE_AND_NAMED_PROFILER_IMPLEMENTATION_RESULT.md`를 따른다.

## G06. 호랑이 반복 시 수평 되돌림과 root-motion suppression

설치 `sk_cloudtiger`는 23 source samples/30Hz, duration `0.7333333969`초다. `b_root`의 원본 X는
0에서 `-0.6135301971m`로 물러난 뒤 `+3.4171255493m`까지 이동하고, Y는
`0..0.9700125885m`, Z는 0이다. PSA와 설치 WModel의 root position 최대 차이는
`1.02014e-6m`였다. 원본 수평 locomotion과 authored `+Z 3.25m/s`가 함께 적용되다가 loop의
local time이 되감기면서 약 3.417m의 원본 이동이 사라졌다. quaternion 교정이나 30Hz retime이
다시 잘못된 것으로 처리하지 않았다.

`Effect_AuthoringDocument.h`의 ModelCue에 optional `strSuppressHorizontalRootMotionBone`을
추가하고 Codec이 `suppressHorizontalRootMotionBone`을 읽고 저장한다. 생략/빈 값은 기존 root
motion 의미를 유지한다. 이름은 길이·문자와 실제 Stage의 bone 존재 여부를 검증한다.
Renderer는 아직 pose를 재생하지 않은 fresh prototype에서 기존
`CModel::Enable_RootMotionSuppression(name, 1)`을 사용하고 clone이 같은 설정을 상속한다.
source Y를 유지하고 source X/Z를 rest translation으로 고정한다. 이 옵션은 shared prototype
cache key와 prepared resource signature에도 포함돼, 옵션만 바꿔도 이전 준비 결과가 재사용되지
않는다. 잘못된 bone은 Stage를 실패시키며 이전 정상 설정을 보존한다.

두 Artist D tiger cue에만 `"suppressHorizontalRootMotionBone":"b_root"`를 저장했다.
주변 26 Elements, material, cue ID/시작/길이, loop, 배치와 `3.25m/s` authored velocity는 보존했다.
이 후속 보정 때문에 모델 바이너리를 다시 굽거나 CModel root-motion API/다른 ModelCue의 기본
의미를 변경하지 않았다. Render와 bone anchor는 기존 동일 sample/rewind 계산을 계속 소비한다.

`out/TigerRootMotion20260911/run.log`의 실제 CModel 두 clone·3,017 pose 표본 결과는 다음과 같다.

| 검사 | 수치 |
|---|---:|
| 보정 root X/Z 잔차 | 0 |
| 원본 같은 위상 대비 Y 차이 | 0 |
| 보존된 점프 최대 Y | 0.969994m |
| 두 호랑이 같은 시각 pose 차이 | 0 |
| 표본 간 최소 전진량 | +0.00549889m |
| backward seek pose 차이 | 0 |
| 한 loop 뒤 같은 위상 pose 차이 | 1.78814e-7 |
| 보정 전 wrap의 source X 급변 | -3.41725m |

실제 Codec Load/Save/reload, 옵션 생략, 잘못된 이름 저장 시 기존 파일 보존, actual Renderer
Stage와 missing-bone 거부, 이전 설정 재Stage, suppression 재Stage를 통과했다. 최종 두 경우는
옵션만 변경할 때의 cache invalidation을 검증한다. 변경 Codec/Renderer와 probe 의존성은 out에만
focused compile/link했으며 기존 C4828/C4805 경고가 남았다. WARP는 CModel/Renderer resource
admission 용도로만 사용했고 Client/UI/window/swapchain/draw를 실행하지 않았다.
추가 증거는 같은 폴더의 `source_and_data_checks.json`, `result.md`다.

사용자는 완료된 최신 빌드의 Artist D/Play All에서 두 호랑이의 Y 점프를 유지하면서 앞으로 계속
이동하고 loop 경계에서 뒤로 돌아가지 않는지 확인한다. F1 Action Workbench 첫 열기의 별도
배열 assertion 수정과 확인 경계는
[ImGui 결과 G07](2026-09-11_IMGUI_PROFILING_AND_TOOL_OPTIMIZATION_RESULT.md#g07-action-workbench-첫-열기-배열-범위-수정)을 따른다.
