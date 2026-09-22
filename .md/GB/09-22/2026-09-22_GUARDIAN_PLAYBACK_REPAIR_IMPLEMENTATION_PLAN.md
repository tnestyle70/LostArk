# Guardian 재생·화신화·ALT V 수정 계획

## G00. 현재 기준과 변경 경계

2026-09-22 사용자가 첨부한 10개 이미지는 원작/현재 표시 참고다. 사용자 요청은 Monster 저작 목록과 피해 전용 collider, Gate3 오라/10초 입장, Guardian 이펙트/재생 복원이다. Monster와 Gate3, particle/native material은 별도 대응 PLAN/RESULT에서 담당하고 이 문서는 Guardian 재생 제어를 소유한다.

현재 브랜치는 `GB/collider-pattern-bug-fix`다. 작업 시작 후 다른 작업이 기존 변경을 commit하여 현재 기준은 `48ce17245398627e644b701c906757ba1e9ae603`이다. 기존 변경을 되돌리지 않고 새 기능 hunk만 추가한다.

## G01. Server 화신화

`Server/Private/PlayerSkillSystem.cpp`의 `Try_StartInternal`은 Ember profile이 있는 class의 변신을 최대 identity에서만 허용한다. `Commit_StanceChange`는 socket/orb를 채우지만 identity는 채우지 않는다. 게이지 진입 조건을 제거하고 실제 변신 commit에서 identity를 최대치로 초기화한다. 기존 Server 승인, action/cooldown/stance 검사 및 15초 지속 후 해제를 유지한다. 3초 cooldown은 창술사와 같은 기존 양방향 공통 제한이다.

기존 `ServerGameplayContractTests_PlayerActions.cpp`의 가득 찬 게이지 전용 계약을 빈 게이지에서도 승인·실제 변신 이후 full gauge·15초 해제 계약으로 갱신한다.

## G02. Full Restore Play All

`Effect_Tool_CatalogPreview.cpp`의 saved document load/preview 실패 상태가 UI까지 전달되는지 확인한다. COMBO는 실제 animevents와 skillbindings의 stage를 사용하며, 추측 stage fallback을 넣지 않는다. `Effect_Tool_ResourceBrowser.cpp`는 비활성 Play All의 이유와 현재 재생 실패를 사용자가 보는 목록에서 표시한다. source 문서 입장 검사는 유지한다.

## G03. ALT V

`EffectRecoveryCamera`, Sequencer camera와 effect 문서의 실제 basis를 비교한다. 원본 `guardian_camera_projection.py`의 CM02 카메라가 action의 앞 3800ms만 연결된 상태이므로 후속 source camera graph·action notify를 조사하고 같은 저장 카메라 경로에 연결한다. opaque backdrop의 `compositionLayer=sceneBackdrop`는 기존 다른 ALT V의 소비 계약을 재사용한다. 모델 cue와 gameplay root의 90도 차이는 해당 occurrence에 한해 원본/설치 model basis를 측정하여 수정한다.

새 C++ 파일을 추가하지 않으므로 프로젝트/filter 등록은 필요 없다. JSON 후보는 latest disk baseline을 보존하며 필요한 필드만 병합한다. 실행 중 저작 여부에 따른 최종 적용 정책은 AGENTS의 편집 중 데이터 반영 절차를 따른다.

## G04. 검증

변경 JSON parse와 기존 camera projection 테스트, 관련 Server contract, 정상 증분 Product Debug Build 및 `git diff --check`를 수행한다. Client/UI는 자율 실행하지 않는다. 자동 수치 결과와 사용자의 아레나 최종 화면 확인은 RESULT에 분리한다.


## G05. 사용자 지정 일반 S의 용머리 presentation

사용자의 최종 지정은 화신화가 아니라 일반 상태 S에 용머리와 원형 이펙트를 표시하는 것이다. HUMAN S의 gameplay stable skillId49220, inputSlot·requiredStance·피해·쿨타임·범위·Server 권위는 유지하고 source49290 DragonicResonance의 두 body clip과 모델·Effect를 presentation으로 연결한다. DRAGON S49230의 기존 source49230 불구슬·줄기는 유지한다. source49280/49290 원본 상태 짝으로 슬롯을 다시 지정하지 않는다.

skillbindings의 현재 schema는 skillId와 clips만 허용하므로 새 sourceSkillId 필드를 넣지 않는다. 두 clipOccurrenceId와 `effect.guardianknight.skill.49220.source.49290` Effect asset/sourcePresentation 및 결과표에 원본 provenance를 남긴다. 실제 설치 body clip은 30Hz에서 26/68 ticks다. 기존 Server hit 300ms에 두 번째 clip의 용머리가 시작하고 action 2000ms 안에 마치도록 각각 playRate 2.888889/1.333333을 사용한다. body와 FX는 기존 공통 playbackRate 경로를 소비하고 source particle/component 시간은 그대로 보존한다. 이것은 사용자 지정 presentation의 시간 배율이며 원본 1배 재생속도 복원으로 기록하지 않는다. 최신 저장본의 stable49220 row만 병합한다.

사용자가 추가로 지정한 일반 S의 지면 조준은 차원술사 T의 기존 `PlayerSkillTargeting` 데이터와 `CPlayerController -> IPlayerCommandSink -> Server` 경로를 사용한다. S를 누르면 조준, LMB로 확정하며 이동 가능 지점과 4.5m 사거리를 Server가 재검증한다. 범위 표시는 지름 9m, 지점 표시는 현재 49220 HitShapes의 피해 반경 1.3m와 같은 지름 2.6m다. 신규 두 Effect cue는 `anchor=skill_target`, `follow=snapshot`으로 선택한 지점을 사용하고 head 내부 본/원본 부착은 보존한다. 다른 스킬/49230과 기존 피해, Ember, 취소창, root motion을 변경하지 않는다. 공식 publisher로 `SKILLTARGET 49220 GROUND_POINT 4.5 1`을 생성하며 생성물을 직접 편집하지 않는다.

## G06. ALT V 초반 용 모델 그림자

원본 action49420 notify024의 EF skeletal component는 generic CDO chain에서 CastShadow와 dynamic shadow를 true로 상속하고 notify의 PlaySkeletalMeshActor struct에는 별도 shadow override가 없다. 이는 확인한 데이터 근거이며 원본 native 생성 코드 전체의 복원으로 주장하지 않는다. 별도 projectile494200 DragonDecal은 5.19초 이후의 바닥 이펙트로 원본대로 따로 복원한다.

기존 Effect ModelCue에 optional `castsShadow`(누락 시 false)를 추가한다. codec parse/save/validation과 prepared resource 비교에 연결하고 원본49420 용의 5개 material-part cue만 true로 설정한다. CEffectObject는 기존 SHADOW group에 제출하고 CEffectDocumentRenderer는 surface와 동일 Sample_ModelCuePose, source material parameter tracks, CModel bone/material을 사용해 깊이를 그린다. 기존 Shader_VtxAnimMeshBinary의15개 pass(0..14) 순서를 유지하면서 전용 shadow pass15를 끝에 추가한다. 이 pass는 동일 source coverage/dead dissolve 계산을 소비하며 기존 Character shadow pass1은 변경하지 않는다. 신규 C++ 파일은 없으며 프로젝트 등록도 필요 없다. codec roundtrip/old default/invalid input, 실제 pose 재사용과 변경 부분 컴파일, pass 순서와 shader compile을 확인하고 최종 화면 판정은 사용자가 한다.

## G04. 일반 S 용머리 원본 재질

source49290의 SK_DDK_DRR_01 head/neck WModel은 기존 CModel model cue로 재생한다. 두 override 재질 *_st_mi_fx_dead의 정확한 GPU-skin BasePass/DirectionalLight shader와 texture8개를 추출하여 SourceCharacter native110/111로 추가한다. 원본 geometry slot *_st_mi와 notify override *_st_mi_fx_dead의 대응을 명시하고 texture/object provenance를 보존한다. 기존 shader function과 material family는 변경하지 않으며 새 프로그램은 기존 CMaterial 소비 경로를 사용한다.
