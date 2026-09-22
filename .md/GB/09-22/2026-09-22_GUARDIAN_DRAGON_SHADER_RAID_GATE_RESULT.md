# Guardian S 용 재질과 Kouku 입장 준비 결과

## G01. S49290 모델 재질의 비정상 색

`SOURCE_CHARACTER`110/111은 원본 `sk_ddk_drr_01_head_st_mi_fx_dead` 및 neck의 Base PS `e56633f592e0154eb0794435cf3c2717`을 사용한다. 다섯 material texture와 LINEAR/SRGB 구분은 기존 원본 descriptor를 보존했다. V49400의106~108과 비교하면 S110/111에만 scene environment의 color/rotation 상수24/25 연결이 누락돼 있었다.

무지개 출력과 관련해 실제 수치 결함을 재현했다. 원본의 engine BRDF lookup `t5/s6`는 material texture가 아니며 현재 payload가 미복구다. 기존 생성기는 이 sample을0으로 두지만 뒤의 `1 / lookup.y`를 그대로 실행하므로 `0 * Inf`가 되어 최종 RT0 RGB가 NaN이었다. 해당 shader ID의 reciprocal만 분모0일 때0으로 처리한다. 유효한 lookup은 원본 계산을 유지하고, 없는 lookup은 환경반사0 기여가 된다. 정상재질에 임의 색 보정이나 텍스처 교체를 적용하지 않았다.

`Tools/VehiclePipeline/build_vehicle_source_material.py`와 Engine/Client의 `Shader_SourceCharacterBaseGroup084.hlsli`에 함께 반영했다. 두 설치 shader의 bytes는 동일하다. generator의 Base/Light/configure 재생성 검사가110/111 모두 exact다. 원본 HDR 푸른 rim/transcolor와 dissolve track은 보존했다.

WARP48건은2 program × 변경 전후 ×3 time ×4 lookup fixture다. lookup0인 변경 전6건은 NaN, 수정 후6건은 모든 기록 MRT finite다. 양수lookup36건은 원본 DXBC의 RT0/2/3/4/5와 최대차0이다. 이는 상수 texture fixture를 사용한 수식 검사이며 실제 화면색의 일치 판정은 아니다. 증거는 `out/GuardianDragonShader20260922/numeric-validated.json`, 생성기 exact 검사는 같은 경로의 작업 기록과 설치 hash receipt에 남겼다.

원본 `Engine.PBRPreintegratedGF` payload/생성식과 scene SH는 아직 확보되지 않았다. 원본 LUT를 복원했거나 원작 모든 간접광이 동일해졌다고 표시하지 않는다. Product shader 재컴파일·Debug 배포는 상위 통합 작업 결과를 따른다. Client 실행·UI 조작은 하지 않았다.

## G02. Guardian 선택 후 Kouku 입장 실패

기존 `client-session-55316.jsonl`과 `EffectFailure.user.log`에서 Server world5 승인은 성공했고 Client 필수 Effect 준비가52/57로 멈췄다. 실패5개는49000clip0/1,49100clip0,49260clip2,49150clip2이며 공통 오류는 `Source-owned AnimationTrail carrier target/history closure is invalid.`였다. class admission/Network 경로를 우회하지 않았다.

Guardian effect 담당자가 수정한 baked trail history clamp를 적용한 CPU executable로 현재 Debug의 동일57 target을 재구성했다. Guardian46 + World marker7 + Esther2 + monster1 + boss1이다. 실제 document-owned runtime projection, resource preparation, prevalidated staging과0.05초 seek 순회를57/57 통과했다. elements2234, ModelCues25, OwnerControls41이며 실패0이다. 문제의5문서도 모두 포함했다.

입력 목록은 `out/GuardianFollowup20260922/raid57-inputs.json`, 로그는 `raid57-result.log`, 집계/hash는 `out/GuardianDragonShader20260922/raid57-proof.json`이다. CPU prepare gate가 복구됐다는 증거이며 실제 Client/Server 전환·GPU renderer 준비 및 화면은 사용자 확인 대상이다.

## G03. 별도 세션으로 이관한 의상 재질 작업

사용자의 범위 변경에 따라 CharacterCatalog root equipment override와 EquipmentPresentationService consumer의 미완성 작업은 동결했다. 두 파일 diff가 보존된 patch와 완전히 같은지 확인한 뒤 자기 변경만 역적용했다. 역적용 시점의 두 파일 git diff는0이었다. 이후 별도 재질 세션이 같은 파일의 구현을 인계했으므로 그 세션의 후속 변경과 이 미완성 hunk를 구분한다.

`out/CharacterMaterials20260922/material-consumer-wip.patch`, `before/`, `frozen-live/`, `frozen-material-consumer-rollback.json`을 보존했다. resource/catalog 변경은 하지 않았다. 이관한 inventory와 lineage는 조사 자료이며 재질 복원 완료 증거가 아니다.

## G04. 화신화 A49210 창 방향 후보와 누락 조사

라이브 authored 문서는 교체하지 않았다. 후보는 `out/GuardianDragonShader20260922/ZA/candidate/`이며 latest disk 기준 hash와 stable ID별 변경값은 `field-patch.json`에 기록했다. 현재 원본 `notify-014`의 창 `fm_a_plan_001`1개와 `fx_j_helixline_2`3개에만 `detail.transform.rotationDegrees.y += 90`를 적용한다. 이것은 사용자 요청의 PROJECT_TUNED 시계 방향90도이며 원본 복원값이라고 표시하지 않는다.

원본 TypeData pitch90, MeshRotation의 turn 값, root snapshot basis -90, notify 위치는 보존했다. bone-follow notify001, gauntlet, 바닥 decal, sprite, 충격sphere/cone는 바꾸지 않았다. actual GuardianKnight.wmodel의 CModel preScale.0001과 실제 bone/clip을 사용한 후보 검증은53elements를 통과했다. 첫 재생표본에서 선택4mesh는 같은 notify origin[0,.8,1]을 중심으로 수평방향이90도 바뀌고 축별 크기·높이가 보존됐으며 나머지6mesh는 출력matrix값이 동일했다. 별도 actual document-owned projection/preparation/staging도 통과했다. `transform-proof.json`, `candidate-bone.log`, `projected-candidate.log`가 증거다.

누락으로 지목된 검격의 최초 실제 정점 높이를 실측했다.3998 planeedge는y=.8,4110 창plane은y=.307~1.293이다. helix 중2개는첫표본 정점의6~7%만 바닥아래이므로 이 자료만으로 전체 검격 누락을 높이 문제라고 판정할 수 없다.

원본 native3998/4100/4106/4110/4112의 실제 설치DDS와 실제 CPU particle color/dynamic을 넣은245 UV표본은 모두finite이며 RGB가 존재했다.3998/4110/4112의 수명별1323표본도finite였다.3998 alpha는0~1로 나타났고4110/4112는additive이므로 alpha0만으로 누락판정하지 않는다. RGB 최대치는 `lifetime-summary.json`에 기록했다. 회전 후보의 방향·화면 겹침과 사용자가 지목한 일부 검격 미표시는 사용자 화면확인이 남아 있으며, 이 수치검사를 전체화면완료로 대신하지 않는다.

## G05. 일반 S 즉시 시전 targeting 공식 후보 검증

`out/GuardianFollowup20260922/candidate`를 InputOverlayRoot로 공식 `Publish-GameplayBalance.ps1 -Mode Validate`가 exit0으로 통과했다. 이어 OutputRoot를 `out/GuardianDragonShader20260922/Targeting/isolated-publish`로 고정한 Publish도 exit0으로 통과했다. publisher의 explicit overlay 경로는 Valtan canonical projection을 수행하지 않고 bootstrap·generation manifest를 지정한 output에만 저장한다. admission lock은out하위다.

격리 bootstrap은version34,40242rows다.49220/49230 SKILL 정의는 모두 남고 targeting delta는 `SKILLTARGET 49220 GROUND_POINT 4.5 1` 한 줄 제거뿐이다. `SKILLTARGET 49230 GROUND_POINT 4 1`은 유지된다. Client/Server의 native skill 구조체 기본 target intent가AIM_POINT이고 SKILLTARGET가명시된GROUND_POINT만 덮어쓰는 것을 코드로 확인했다.

공식 검사 결과는8profiles,313skill rows,125damage profiles,65boss patterns/280stages와 hit-shape92/92다. 후보targeting SHA256은`bda7e3f1d7853a9e1d8aaf64d39da93d6887ce874ec7646549e63e4b76b49da3`, 격리bootstrap SHA256은`b63e0791afd73e918d3a3334c632b775eff2e48d77fc498899229c3dc9156c7f`다.

live Data/Balance와 Server/Bin/DataFiles/Gameplay의기존25파일은 전후hash가같다. liveData교체·runtime publish·Server재시작은하지않았다. 명령과로그·assertion은 `out/GuardianDragonShader20260922/Targeting/README.md`, `validate.log`, `publish.log`, `proof.json`에보존했다.

## G06. 통합 Gameplay 게시 중 Kouku Product stale 진단

통합 담당자의 이후 live Gameplay 게시가 Kouku Product freshness 검사에서 중단됐다. 앞선 후보 검증 당시에는 source revision2195였고, 이후 사용자 저장본이2197로 바뀌었지만 generated Encounter와 patternbindings는2195였다. 이를 검증 우회나 저장본 되돌리기로 처리하지 않았다.

공식 projector의 `prepare_publication`과 `projected_outputs`를 freshness session 안에서 실행해 최신 원본으로 두 후보를 생성했다. 이 진단은 `out`에만 썼고 source·Product·runtime을 변경하지 않았다. 입력15개 freshness 재검사는 통과했다. 최신 원본은 쿠크 나팔액션105의3stage와 저글링액션106의2stage를 추가했고, 다른 저장 차이는 기본값 명시/숫자 표기와 false 필드 생략이다.

기존94개 Product pattern은 모두 보존되고 준비가 끝난 두 신규 pattern이 추가돼96개가 됐다. 공식 projector는 DRAFT를 legacy 편집 필드로 보고 실행 가능한 closure를 private copy에서 PRODUCT로 만들며, source 자체는 보존한다. 이전94개의 animation binding은 정확히 같고5개만 추가됐다. 전체 projected stage는618→623이며 저작 stage summary와 집계 기준이 다르다. raidGates와9bundles는 정확히 같다. playAllPatternIds에는105/106이 끝에 추가된다.

기존103 card-rain의 content hash 기반 clientVisualId 한 개가 저장된 명시적 기본값 때문에 바뀌며 Encounter 참조와 presentation catalog가 함께 갱신된다. 이 항목을 재매핑하면 기존94개의 Encounter 행동은 정확히 같다. `boneTarget=BODY`, `brightnessMultiplier=1`, `fitEffectToDuration=false`를 런타임 기본값으로 정규화하면 기존94 presentation과 card-rain payload도 정확히 같다. 기본값과 optional parsing은 `KoukuSaydonCompositionDocument.h` 및 `KoukuSaydonPresentationPlayer.cpp`에서 확인했다.

후보·정확한 semantic diff·hash·behavior assertion은 `out/GuardianDragonShader20260922/Targeting/kouku-latest-diagnostic/`에 있다. 공식 `project_kouku_saydon_composition.py --mode publish`는 `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`과 `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json` 두 파생 파일만 교체한다. 기존 freshness·백업·원자적 promotion·실패 rollback을 유지한 명령을 통합 담당자에게 전달했다. 실제 공식 Kouku publish와 후속 Gameplay publish 완료 여부는 상위 통합 결과를 따른다.
