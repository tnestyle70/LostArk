# 창술사 V·Alt+V 원본 전체 복원 구현 계획

## G00. 원본 발생과 저작본 경계

현재 `codex/dimensionmaster-tool-round3`, 기준 HEAD `591012dbebf7eeab0b660baec42852b9396e77d4`의 다른 작업과 사용자 unified 저작본을 보존한다. V34610은 고유 source stage3개, Alt+V34630은4개다. 설치 `data3.lpk/XmlData/Action/LANCEMASTER.loa`의78개 활성 PlayParticleEffect typed payload가 모두 해독된다. V의 첫 LOD 활성 발생은53/83/31, Alt+V는146/66/77/27이다. Alt+V의 horse payload에 포함된 별도 FX_VT_PMSCK_00 reference array는 기본 sourceParticleSystem과 구분하며 임의로 동시재생하지 않는다.

각 clip에 새 `effect.lancemaster.skill.<id>.clip<n>.full.restore`를 만든다. 원본 sourceEnabled, ordered first LOD, stage local time, notify transform과 anchor, parameter override를 먼저 적용한다. 복원되지 않은 요소는 사유와 원본 ID를 별도 결과에 보존하고 실행 elements에서 제외한다. 삭제나 approximate material 허용을 복원 완료로 기록하지 않는다.

## G01. 현재 원본 입력과 재질 프로그램

보존 archive graph와 closure를 현재 설치 UPK의 동일 source object에 대조한다. 각 발생의 Required/TypeData와 실제 mesh material override, particle VF를 확정하고 MIC→parent→static parameter set→native shader map의 PS/VS를 회수한다. 원본 uniform 표현식의 순서, texture/sampler, UV/world/vertex color/dynamic parameter를 보존한다. 용 mesh와 검격이 어떤 입력을 필요로 하는지 원본 프로그램으로 판단한다.

`out/LanceMasterVARestore20260909`는 임시 추출·검증 입력을 소유한다. 최종 새 `Effect_LanceMasterVAMaterial.h`는 엄격한 source identity·resource·parameter 계약만 소유하고 `Shader_EffectLanceMasterVANative.hlsli`는 회수한 명령과 기존 runtime 좌표·depth adapter를 소유한다. native 번호는560~659 다음720~819이며 다른 작업에 할당된660~719를 사용하지 않는다. 전용 uniform은 `g_LanceVASourceMaterialParameters[32]`, `g_LanceVASourceMaterialTime`다.

## G02. 기존 렌더러 연결

기존 EffectDocumentRenderer/Playback/Tool와 CModel→CMaterial을 사용한다. 새 native profile의 header/HLSLI, full 문서와 필요한 Resources만 이 작업이 작성한다. shared Renderer/Codec/MaterialTemplate와 project/filter 등록은 통합 담당 root가 수행한다. include, profile lookup, parameter bind, shader dispatch와 필요한 vertex 입력을 실제 생성 결과로 전달한다. 새 profile 입력 실패는 해당 occurrence를 거절하고 기존 문서를 보존한다.

## G03. 검증과 실행 준비

변경 JSON parse, stable ID, 원본 활성/LOD와 발생 수, source module·parameter 보존, 실제 resource 존재, 선택 PS/VS와 texture sample closure, 독립 FXC 및 필요한 CPU 수치 검사를 수행한다. 통합 최소 Product 컴파일은 root가 수행한다. Client/UI 자율 실행·조작·캡처와 실행 중 Client/Server 종료는 하지 않는다. 빌드 성공과 사용자의 화면 판정은 RESULT에서 분리한다.

## G04. 말 소환과 실제 원본 애니메이션

사용자의 추가 요청에 따라 `SK_FLM_HOR_00.Mesh.SK_FLM_HOR_00_SK`를 소환 모델로 복원한다. Alt+V stage1/2/3의 PlaySkeletalMesh notify005/004/005는 각각 `_02/_03/_04` clip을 호출한다. 네 source material section을 같은 원본 skeleton과 세 clip으로 각각 cook하고, 기존 ModelCue가 네 CModel을 동시에 재생한다. 각 파일의 mesh/material index는0 하나이며 native 재질은 source Action의 네 explicit MIC array를 사용한다. mesh 기본 material과 Action override를 혼동하지 않는다.

PSA의 bone 순서와 glTF의 bone 순서는 source bone 이름의 전단사로 연결하고 mesh의 실제 hierarchy를 보존한다. source child CEFAN_Particle6개와15개 attachment anchor를 parent ModelCue section0에 연결한다. source call ID의 `/`는 runtime stable ID에서 `.`로 변환하며 원본 ID는 receipt에 보존한다. 말 소환 own-transform에 아직 분해되지 않은 원본 필드는 프로젝트 root identity와 source-axis yaw−90 적용으로 명시하며, raw byte를 해석한 값으로 주장하지 않는다.

## G05. 카메라 부착 파티클과 PlayStaticMesh

카메라 이동·회전·FOV·shake 및 sequencer 확장은 사용자 최신 범위에서 제외한다. PlayCameraParticleEffect의 source offset110cm, identity 회전·스케일과 stage local time은 기존 camera_view attachment로 전달한다. PlayStaticMesh10회는 서로 다른5개 원본 mesh의21개 material section을42개의 일반 Mesh Element로 연결한다. 각 section은 원본 위치·normal·UV·vertex color·index를 기존 typed CModel cook으로 보존하며 source material에 맞는 native PS를 연결한다. 원본 EffectRoot/flags와 runtime snapshot-root 해석을 구분해 기록한다.

## G06. CDO와 단독 재생 경계

원본 Engine/EFGame/Core의198개 CDO로72개 도달 class의 module/distribution 기본값을 복구한다. RawDistribution이 명시적으로 직렬화되면 그 값은 통째로 보존하여 source reference와 CDO lookup table을 섞지 않는다. OrbitOptions의83-byte native bool payload는 spawn/update/emitter-time 세 field로 해독한다. 미지원 native engine prefix·ribbon history·decal VF와 sibling event provider가 필요한 항목은 full elements 밖의 원본 occurrence 목록에 분류한다. 사용자 단독 재생 요구 때문에 sibling-only 요소를 제거해도 그 source 자료와 복원 후보를 삭제하지 않는다.

## G07. 2026-09-10 전체 스킬 확대

사용자가 V clip2 full restore 화면을 승인했고, 이를 기준으로 두 stance의 LMB부터 현재 `LanceMaster.skillbindings.json`의 모든 source Action을 확대한다. V/Alt+V7개 문서와 기존 native 번호는 보존한다. `prepare_lancemaster_all_full_restore.py`는 정확한 clip 이름과 Action 순서로 base stage를 선택하고 같은 clip을 쓰는 tripod variant를 합치지 않는다. stage의 원본 enabled, first LOD, CDO, typed parameter와 attachment를 회수한다. 원본에 particle call이 없는 stage는 명시적으로 결과에 남긴다.

신규 재질은 기존 `Effect_LanceMasterVAMaterial.h`와 전용 HLSLI를 확장하고 번호1200~1599를 사용한다. 기존 프로그램의 함수 본문과 승인된 V2 문서는 바꾸지 않는다. 필요한 Resources는 같은 `Effect/LanceMaster` 경로 아래 설치하며 binary를 Git에 추가하지 않는다. source occurrence와 primitive/VF가 독립 Solo 소비자에 연결되지 않는 항목은 정확한 이유와 원본 ID를 남겨 실행 문서에서 제외한다.

Alt+V 말의4section/3clip 원본은 이미 CModel load 검증이 있다. 현재 문서 load를 막는 SubUVMovie 지원은 공용 runtime 담당이 복구하고, 이 작업은 실제 세 말 clip과 ModelCue12개의 연결·리소스 입력을 보존한다. 전체 새 문서는 실제 Codec load/save/Solo로 검증하고 공용 Catalog/ResourceTree 및 animevents의 실제 소비자는 통합 담당이 연결한다. Client/UI 실행과 화면 캡처는 수행하지 않는다.
## G18. 2026-09-10 사용자 검격 교체와 T 돌진 재검토

사용자가 승인한 A 첫 반원은 34140 ba1 clip1의 `par_o_flm_chestdestruction_01_1` 네 Mesh Element다. 긴 창 BA/Q/W/E/R의 지적된 trail occurrence를 이 네 요소의 동일 geometry·native 재질·입자 모듈로 교체한다. 대상 스킬의 원래 발생 시각을 유지하고, 분리된 광원·피격·바닥 먼지와 사용자 다른 값은 보존한다. E 첫 clip의 shoulder 충격은 검격과 구분하여 보존하고 같은 공격 시점에 반원을 추가한다. 복제본은 기존 `authored-copy:<originElementId>`를 사용하여 원본 난수와 native source identity를 보존한다. 이는 사용자 요청에 따른 프로젝트 연출 교체이며 해당 스킬 원본 검격의 source exact 복원으로 기록하지 않는다.

S는 원본 base Action의 네 번 발생(clip1 0.463초, clip2 0.531초, clip3 0.433/0.803초)을 각각 같은 반원으로 연결한다. 현재 skillbindings의 playMs/playRate와 원본 clip local clock을 유지하여 전체 스킬 시계에 임의 균등 배치하지 않는다. V는 승인된 clip2를 보존하고 clip1의 spear_00 11개 요소를 마지막 clip의 0.4초 돌진 시작에 추가한다. native 재질·shape·상대 emitter delay는 그대로 두고 새 stable ID와 저작 provenance만 부여한다.

V/T의 실제 이동은 `LanceMaster.rootmotion.json -> Publish-GameplayBalance -> Gameplay.bootstrap SKILLROOTMOTION -> CGameplayCatalog -> CPlayerSkillSystem`을 조사한다. 현재 V 194/T 74개 이동 샘플이 저작본과 게시본에 모두 있고 Server는 movementDistance보다 이를 우선 소비한다. 실제 CModel 30fps duration과 clip chain은 V 6.4333초, T 2.4333초로 같은 시계를 사용한다. 무이동 원인을 movementDistance=0으로 단정하거나 Client Transform 이동을 추가하지 않는다. 좁은 Server 입력 검증과 root motion 적용 결과를 확인한 뒤 필요한 변경만 한다.

T 34650은 clip1 원본 30/30, clip2 74/76 요소가 연결되어 있고 제외 2개는 native decal VF다. 첨부 원작의 용머리·붉은 돌진 연기·번개·검은 바닥 파편을 각각 source occurrence, 실제 bone anchor, 재질과 입자 출력으로 대조한다. 원본 후보가 이미 연결되어 있다는 사실을 실제 출력 완료의 근거로 사용하지 않는다. 제외 decal 또는 다른 base/variant Action의 역할을 확인하고 필요한 최소 리소스/문서 교정을 이어간다.

변경 전 최신 원문·SHA와 변환 receipt는 `out/LanceMasterSlashFollowup20260910`에 보존한다. 기존 full 문서의 ID와 Catalog 연결을 유지하므로 새 Catalog/ResourceTree 항목은 만들지 않는다. 제품 C++/FX/Server 빌드는 root가 직렬 실행하며 Client/UI 조작과 캡처는 하지 않는다. 자동 숫자 검증과 사용자의 실제 화면 판정은 구분한다.


T 추가 조사에서 base stage1 notify014는 clip local 0.5초부터1.2초의 PlaySkeletalMesh이며 기존 PPE 전용 generator의 범위 밖이라 빠졌음을 확정했다. `SK_FLM_GDR_01.Mesh.SK_FLM_PMSHB_00_SK`, source animation set `SK_FLM_GDR_01.Ani.SK_FLM_PMSHB_00_Ani`, source material `FX_M_MI_T_00.FX_MI.FX_T_Me_Master_02_01_Sk_Dt_Tr`와 child `Par_T_FLM_DragonCleave_01_Cast_01`를 기존 ModelCue·native source material·bone attachment로 복구한다. 현재 native653/654의 static 무기소품은 돌진 용 본체와 다르다. T 전용 모델·재질·clip2 문서의 추가는 artist 담당이 수행하고 현재74개 요소를 보존한다. 새 native 번호가 필요하면1360~1399를 사용하며 source clip `SK_DragonCleave_03`의0.5초 cue 시각은 바꾸지 않는다.
