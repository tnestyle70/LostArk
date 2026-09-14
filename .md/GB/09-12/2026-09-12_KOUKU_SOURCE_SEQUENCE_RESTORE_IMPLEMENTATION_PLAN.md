# 쿠크 원본 시퀀스 저작 연결 구현 계획

## G06. 정상 팝업북·피날레를 기준으로 결합하는 수정 계획 — 코드·데이터·빌드 적용 완료

2026-09-12 조사 단계에서는 코드 수정 없이 이 G06을 작성했고, 이후 사용자의 **전부 반영하자** 요청으로 구현이 승인됐다. 적용 시작 HEAD는 `5168899d`, 작업 브랜치는 `codex/kouku-authored-finale-popup`이다. 아래 명세를 현재 정본에 병합하며 실제 적용·검증·남은 화면 확인은 대응 RESULT의 G08에 기록한다. 아래 G00~G05는 최초 원본 Matinee 연결 이력이다. 그중 G1의 41,488ms와 source Book/배경/배우 선택은 이 G06으로 대체한다. 다른 관문의 기존 변경은 이 계획의 대상이 아니다.

조사 시작 HEAD는 `a72058637567f21e9b4235ef214133fc70c84963`, 문서 정리 중 공유 작업공간 HEAD는 `aac4fbdd`로 바뀌었다. 브랜치는 `codex/map-character-render-performance`다. 다른 작업의 렌더링 성능 변경을 보존하고 실제 구현 시작 때 최신 diff를 다시 확인한다. 조사한 Sequence revision은 6, WorldSequence revision은 675다.

### G06-01. 선택할 정상 경로와 확인된 원인

**정상 피날레의 맵 동작 → 새 포탈·흡입 → 정상 팝업북의 맵·책·배우·카메라**를 한 Pattern에서 재생한다. 기존 WORLD/presentation/scene profile row를 소비하며 새로운 Parent/Bundle 런타임은 이 연출 수정에 필요하지 않다. 책과 커튼의 Transform·모션을 한 세트로 재사용하고, 표면 재질과 조명을 별도로 F1에 맞춘다.

| 현재 증상 | 실제 코드·데이터에서 확인한 원인 | 다음 변경 |
|---|---|---|
| 피날레의 문과 내부 요소가 움직이지 않음 | 현재 P4는 정상 P2의 `circusfinale` 23개 MAP track을 참조하지 않음 | P4 첫 WORLD에 기존 `kakulsaydon.g1.world.14` 사용 |
| 통합 책의 커튼·배경이 책 밖으로 나옴 | 정상 P1은 `evt2_book02`와 136개 저작 MAP 배치, P4는 `evt2_book01`과 source 배경 266개를 사용. 책 위치와 카메라도 다름 | 정상 책·136개 배치·카메라를 함께 채택. P4의 source WORLD 13개 참조 교체 |
| 정상 팝업북도 중간에 다른 맵처럼 바뀜 | `original_8T6_00~04` box가 4,507ms에 끝나 `Stop_All(...,true)`가 배치를 복구하고 standing arena를 표시 | MAP 모션 키는 보존하고 box 수명만 37,800ms로 연장 |
| 책이 긴 연출 중 사라지거나 닫힌 책이 보임 | `original_book`은 template 2,370ms, `motionEnd=STOP`. 긴 box만으로 OBJECT_RESOURCE 최종 pose가 유지되지 않음 | 동일 template을 참조하는 전용 HOLD instance 사용. 닫힌 기본 Deploy 책도 초기 숨김 |
| 책 쪽 세이튼 중복 | 기본 INTACT인 Deploy 5 위치 `[-0.2883,1.3253,737.629]`와 P4 `SaydonStage` 첫 좌표가 일치 | 정상 배우를 대여하는 경로로 통일하고 연출 Deploy 5·7 초기 숨김 |
| 펼쳐진 맵의 표면·밝기 차이 | 필요한 texture 파일은 존재하지만 일부 F1과 material variant가 다름. 책과 F1은 Z로 204.8m 떨어져 다른 local light/환경 영역을 소비 | 동일 mesh의 재질 6행 교체, 연출 수명의 F1 광원 사본과 전용 scene profile 적용 |

이미지의 모든 변형을 하나의 회전 부호 오류로 확정한 것은 아니다. 새로운 통합 경로가 기존 정상 저작을 사용하지 않는 사실과, 수명·재질·광원·중복 배우의 개별 원인을 확인한 것이다. 서버 F1 보스는 Z 약942.33에 있어 책의 배우와 약205m 떨어져 있다. 현재 중복을 서버 보스 때문이라고 기록하지 않는다. P1/P2/P4는 animationOccurrences가 비어 별도 CNpc preview actor를 만드는 경로도 아니다.

### G06-02. Sequence·WORLD·Camera 정본의 교체 내용

수정 파일은 다음 기존 정본이다. 새 C++ 파일은 없다.

- `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`: P4의 stage, WORLD, CAMERA/EFFECT presentation, scene profile 참조와 수명.
- `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`: 정상 book template을 쓰는 전용 HOLD instance 한 개.
- 같은 폴더의 `LV_LUT_MIDNIGHTC_ED.camerashots.json`: 기존 카메라를 복사한 Pattern 전용 shot.
- 같은 폴더의 `LV_LUT_MIDNIGHTC_ED.mapplacements`: G06-04의 material asset ID 6행.
- `Data/Rendering/Authored/RenderingProfiles.json`: G06-05의 팝업북 전용 환경.
- `Client/Public/Level_KakulSaydonArena.h`, 대응 CPP, `Client/Private/MainApp.cpp`: G06-05/06의 scoped 광원과 초기 Deploy 상태.
- `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`, `test_publish_rendering_profiles.py`: G06-07의 float32 왕복 검증.

통합 P4의 stable ID `KAKULSAYDON_G1_PATTERN_4`, `enterCombatOnFinish=true`, 기존 target boss 계약은 유지한다. `STAGE_1`/기존 action ID의 duration을 **58,810ms**로 설정한다. 정상 모션 속도는 바꾸지 않는다.

| 구간 | startMs | durationMs | 사용할 참조 |
|---|---:|---:|---|
| 피날레 맵 전체 | 0 | 21010 | `kakulsaydon.g1.world.14` → `circusfinale` |
| 피날레 고정 카메라 | 0 | 12258 | `1Stage.finale`의 Pattern 전용 사본 |
| 포탈·흡입 카메라 | 12258 | 8752 | 현재 source camera 1~5, 첫 600ms 전환 보정 |
| 정상 팝업북 맵 | 21010 | 37800 | `kakulsaydon.g1.world.7`~`.11` → `original_8T6_00`~`04` |
| 정상 팝업북 책 | 21010 | 37800 | 신규 `kakulsaydon.g1.world.33` → 아래 HOLD instance |
| 정상 팝업북 세이튼 | 21010 | 37800 | `kakulsaydon.g1.world.13` → `original_kouku` |
| 정상 팝업북 카메라 | 21010 | 37800 | `2Stage.book` 전체 track의 Pattern 전용 사본 |

피날레의 펼침은 11,900ms에 끝나고 카메라는 21,010ms 동안 정적이다. 뒤의 8,752ms에 포탈 카메라를 겹쳐도 맵 모션을 자르지 않는다. 대관람차의 후반 회전도 그대로 실행한다. 이 58.810초는 새 결합의 설계 시간이며 원본 게임 전체와 동일하다고 판정한 시간이 아니다.

새 WORLD row의 `sequenceInstanceId`는 `world.sequence.instance.kouku.gate1.authored.book`, `displayName`은 `팝업북_책_통합유지`다. 기존 book의 `templateId=sequence.LV_LUT_MIDNIGHTC_ED.book_open`, `position=[0,-1.41,737.28]`, `playbackSpeed=0.699999988`, binding `animated.prop / OBJECT_RESOURCE / world.object.kouku.popup.book`을 그대로 복사하고 `motionEnd`만 `HOLD`로 설정한다. 기존 trigger용 `original_book`은 보존한다. `nextWorldOrdinal`과 신규 occurrence ordinal은 실제 최신 문서에서 중복 없이 갱신한다.

독립 P1 `연출_팝업북`도 동일 유지 문제를 갖기 때문에 WORLD 7~11 box를 37,800ms로 연장하고 책 참조를 이 HOLD instance로 바꾼다. actor/camera 원본 키는 보존한다. 독립 P2 `연출_1관문 피날레`의 기존 맵 키와 카메라는 보존한다. 신규 효과의 전체 통합 확인 대상은 P4다.

새 카메라 ID는 `kouku.gate1.authored.finale`, `kouku.gate1.authored.book`, `kouku.gate1.authored.portal`을 사용한다. 기존 shot을 복사하되 `activation=PATTERN_ONLY`, `sequenceInstanceId=""`로 하여 AUTO shot이 별도의 WORLD를 시작하지 않게 한다. Sequence의 `presentationResources(kind=CAMERA, assetId=<shotId>)`와 실제 `presentationOccurrences`까지 연결한다. 원본 source camera 6~18은 P4에서 빼고 팝업북 카메라 하나로 대체한다. 공유 resource/shot/World 정의나 Resources 파일 자체를 삭제하지 않는다.

피날레 eye `[38.8578,4.65774,-75.0091]`, lookAt `[46.3628,4.30222,-81.6082]`, FOV 60도에서 source 첫 pose로 바로 바꾸면 eye 2.868m, 시선 4.383도, FOV 약17도의 차이가 난다. `authored.portal`의 local 0ms에 피날레 pose, 600ms와 4400ms에 source 첫 pose를 둔다. source camera1은 원래 첫 4400ms가 정적이므로 이후 카메라 움직임은 보존된다. camera2~5는 기존 시작 시각에 12258ms를 더한다. 첫 포탈은 14603ms에 생성되므로 600ms 전환과 겹치지 않는다.

구체 WORLD 8행, HOLD instance, 기존 책의 전체 카메라 키 사본은 `out/KoukuPopupFinale20260912/sequence-plan-snippets.json`에 미적용 자료로 보존했다. 이 자료 전체를 정본 JSON에 덮어쓰지 않고 위 항목만 최신 문서에 병합한다.

### G06-03. 새 이펙트·흡입·암전의 시간과 좌표

포탈 원점 `[72.857119,1.761627,-99.769365]`, 흡입 원점 `[72.376201,1.575498,-99.511553]`은 정상 피날레의 광장 바닥 placement 29 실제 WModel bounds 안에 있다. 닫힌 문판 placement 3은 이미 7150ms에 숨겨진다. 효과를 문판 중심이나 장식 gate 19로 옮기지 않는다. 제거할 source 배경 266개의 전체 키 Z 범위는 715.981~1182.710이라 이 앞쪽 광장 geometry를 소유하지 않는다.

현재 P4가 직접 쓰는 V1은 `source_matinee_0.1` 150요소, `source_matinee_0.2` 24요소, `effect.kouku.gate1.intro.festival.full.restore` 80요소다. 앞의 174요소는 actionCueAttachment가 꺼진 WORLD sourceTransformTrack이다. 후반 불꽃·trail도 이미 Z736~742의 정상 책 공간에 있다. festival도 MAP root `[13.99014,1.04008,735.37336]`를 사용한다. 새 source Book의 지하 위치와 정상 Book의 차이를 효과에 일괄 더하면 오배치가 생긴다. **이번 결합에서 효과 원점·scale·rotation과 원본 track 좌표는 보존하고 시간만 재배치한다.**

전반 포탈·흡입 등 source event의 시작 시각은 12258ms만큼 뒤로 옮긴다. 후반 source 시각은 다음 landmark 사이를 선형 대응한 팝업북 local 시각에 21010ms를 더한다. key 사이 효과의 모션 속도나 particle lifetime 전체를 늘리지 않고 각 activation의 시작점만 옮긴다.

| source ms | popup local ms |
|---:|---:|
| 8752 | 0 |
| 19741 | 9600 |
| 22244 | 12100 |
| 26919 | 17229 |
| 33583 | 26657 |
| 41488 | 37800 |

공유 원본 효과를 수정하지 않고 다음4개 전용 JSON을 만든다. V1은 effectAssetId, V2는 effectId를 신규 ID로 바꾸고 P4에서만 참조한다. 기존 recipe/native material/internal event ID는 보존한다.

| 읽을 원본 파일 | 추가할 전용 파일 |
|---|---|
| `Data/Effects/Authored/effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.1.effect.json` | `Data/Effects/Authored/effect.kouku.gate1.authored.portal-arrival.1.effect.json` |
| `Data/Effects/Authored/effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.2.effect.json` | `Data/Effects/Authored/effect.kouku.gate1.authored.portal-arrival.2.effect.json` |
| `Data/Effects/Authored/effect.kouku.gate1.intro.festival.full.restore.effect.json` | `Data/Effects/Authored/effect.kouku.gate1.authored.festival.effect.json` |
| `Data/Effects/V2/Authored/kouku.gate1.full.fade.black.effectv2.json` | `Data/Effects/V2/Authored/kouku.gate1.authored.fade.black.effectv2.json` |

`Data/Effects/EffectCatalog.json`에는 신규 V1 3개를 `DIRECT_AUTHORED_DOCUMENT`와 실제 저작 경로로 등록한다. V2는 기존 Authored 스캔으로 해석한다. 파일 생성이나 프로젝트 None 등록만으로 V1 lookup이 연결됐다고 처리하지 않는다.

새4개 EFFECT presentation occurrence는 통합 시각0ms부터58810ms까지 유지한다. source effect의 기존 MAP root/offset/rotation/scale은 대응 사본 row에 보존한다. `sourceNode`의 emitter/activation 경계(구분자 `|` 앞부분)가 같은 요소를 원본 activation 그룹으로 묶는다. split 문서0.1/0.2에 걸친 같은 activation도 같은 그룹의 기준 시각을 사용한다. sourceTransformTrack의 원래 origin `o=2.344761848449707`, 그룹의 최소 element delay를 `d_min`이라 하면 다음과 같다. 아래 T는 위 시간 대응을 초 단위로 계산한 함수다.

```text
s = o + d_min
delta = T(s) - d_min
element.detail.timing.startDelaySeconds_new = old_delay + delta
element.sourceTransformTrack.sourceTimeOriginSeconds_new = old_origin - delta
```

실제 origin field의 위치는 현재 요소의 sourceTransformTrack 구조를 유지한다. 이 규칙으로 원본 WORLD 이동 track은 `sampleTime + sourceTimeOrigin`에서 같은 좌표를 샘플하며, 그룹 내부 element 간 시간차와 lifetime을 보존한다. 음수 finite origin은 기존 Codec이 지원한다. sourcePresentation.sourceTimeSeconds는 provenance라 runtime 시계로 사용하지 않는다.

festival은 origin 속성이 없으므로 원본 발생 시작 `o=22.51650619506836`을 s 계산에만 쓰고 element delay만 바꾼다. 첫 축포의 새 시각은33.408970초, 마지막 두 묶음은55.606958/55.608716초다. 존재하지 않던 sourceTransformTrack/origin을 추가하지 않는다.

V2 fade는 `params.lifetime=58.810`으로 하고 `params.screenPost.intensityKeys[].timeSeconds`를 별도 변환한다. 첫0~1.9971559초 키는 피날레 reveal로 유지하고, 원본7.459초 이후 전환 키만 T로 옮긴다.20.2141868초에 완전 암전이 되고21.010초의 책 전환 뒤 다시 밝아진다. intensity 값과 첫 reveal의 곡선을 유지한다. 중간 암전과 최초 reveal을 한꺼번에12258ms 뒤로 미루지 않는다. 종료 key는58.810초/강도0이며, V1 emitter lifetime을 이 길이로 늘리지 않는다.

### G06-04. 맵 애니메이션을 유지하는 재질 적용

정상 popup의 37 WModel / 실제 40 material slot은 필요한 texture 88개가 모두 존재한다. 정상 책과 source 책의 WMSH 347,984bytes, WMAT 14,292bytes, WSKE 4,272bytes는 각각 동일하고 3개 named slot의 binding도 동일하다. 책 전체 재추출이나 shader 새 경로 추가를 이 문제의 해결책으로 두지 않는다.

`LV_LUT_MIDNIGHTC_ED.mapplacements`의 다음 stable placement 6행에서 **assetId만** 교체한다. 실제 세 쌍의 WMSH가 동일하고 catalog preScale/cull/render profile도 같아 기존 track·Transform을 유지할 수 있다. target은 이미 존재하는 F1 base asset이며 baked RNM variant가 아니다.

| placement ID | 기존 assetId | 사용할 F1 assetId |
|---|---|---|
| 43, 44 | `MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_A5EC96ACE437` | `MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_04DCDAA75173` |
| 45, 46 | `MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_C32AA0E5373E` | `MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_84D65232B541` |
| 114, 115 | `MAP_3A0FD723820D_BG_RAD_KOUKUSATON_DECO02B_SM_OVR_CC900DC1DE11` | `MAP_3A0FD723820D_BG_RAD_KOUKUSATON_DECO02B_SM` |

FLOOR08A의 F1 표면은 `bg_base_msk`/floor08b_mi이며 diffuse .7, specular .2, reflection .2, ambientreflection_15를 쓴다. popup의 floor08d_mi는 diffuse .5, specular 10, reflection .3, ambientreflection_13이다. FLOOR08도 F1의 `bg_seamless-specular_msk`/floor09b_mi와 다르다. 기존 MAP_PLACEMENT의 CModel→CMaterial 경로로 이 F1 family를 그대로 사용한다. BG8만 허용하는 World Object `mapMaterialBindings`로 강제 옮기지 않는다.

136개의 최종 pose를 Z+204.8로 옮겨 F1에 대응하면 위치≤1m/회전≤1도/scale축오차≤.01 기준으로 24개만 일치하고 112개는 일치하지 않는다. 따라서 전체 F1 placement로 교체하거나 F1 Transform을 복사하면 팝업북의 기존 움직임이 달라진다. 위 변경은 **표면 재사용**이며 배치 전체 동일성의 증거가 아니다. 이동하는 부품에는 F1의 고정 baked RNM을 복사하지 않는다. RNM0인 popup과 baked lighting이 있는 F1의 최종 간접광 차이는 사용자 화면 비교에서 별도로 판정한다.

### G06-05. F1 환경과 local light를 연출 수명에 연결

현재 책은 `[0,-1.41,737.28]`, F1 중심은 Z942.08이다. F1 주요 SPOT은 책에서 약205.75m 떨어지고 range는59.8m다. 같은 profile 이름만 연결해도 광원은 책에 도달하지 않는다. 또한 `CRenderingProfileService::Apply_CameraEnvironment`는 카메라 위치로 region을 고른다. popup region47 ambient는 F1 region48보다 25% 밝다.

RenderingProfiles에 `scene.kakulsaydon.g1.popup.v1`을 추가한다. 현재 `scene.kakulsaydon.g1.base.v1`을 복사해 qualityOverride, shadow, 환경 반사 입력과 scalar는 보존한다. light.diffuse/ambient에는 region `kouku.ps.environment.48`의 directionalColor/ambientColor를 사용한다. fog의 density/heightFalloff/topHeight/startDistance/maximumOpacity/color도 region48 값으로 교체하고, region fog의 inscatteringColor/lightDirection은 기존 base fog의 `sourceExponential` 안으로 대응시킨다. 나머지 fog 필드는 base를 유지한다. optional `environmentRegions` 필드를 생략해(명시적 빈 배열은 현재 reader가 거부함) 이 연출 동안 카메라가 region47 또는 바깥 영역에 들어가도 F1 환경이 유지되게 한다. 기존 G1 profile과 지역 조명 문서를 변경하지 않는다.

Sequence에는 `kakulsaydon.g1.sceneprofile.3` → 위 renderingProfileId를 추가한다. P4의 기본 profile occurrence는 0~21010ms, popup profile은 21010~58810ms로 나눈다. P1에도 popup profile을 0~37800ms 적용한다. profile 종료·Stop·실패의 복귀는 기존 sceneProfileOccurrences 소유권 경로를 유지한다.

local light는 Level이 **기존 CMapLightPresentationRuntime의 별도 문서 사본**을 연출 동안 소유한다. 새 MapLight renderer나 Effect point-light 경로를 만들지 않는다. 현재 문서는 formatVersion2,119개 광원이므로32개 사본을 추가한151개가 기존 MAX_LIGHT_COUNT512 이내다.

- 복사할32개: `light.LV_LUT_MIDNIGHTC_ED.1`, `light.kouku.source.sl05.151`~`.179`, `light.kouku.source.sl05.227`, `.228`.
- popup 동안 제외할14개: `light.kouku.source.ps.265`, `.267`, `.268`, `light.kouku.source.sl04.105`~`.110`, `.113`~`.117`.
- 사본은 source의 position에 `[0,0,-204.8]`만 더하고 ID에 `popup.` prefix를 붙인다. 원본 위치의32개도 사본 문서 안에 유지한다. kind, receiver, rotation, cone, range, color, brightness, falloff, staticShadowChannel을 보존한다. source는3 SPOT+29 POINT이며 SPOT을 POINT로 치환하지 않는다.

H의 기존 `m_pMapLightAuthoringOverride` 다음에 `m_pCompositionMapLightPreview`와 활성 여부 `m_bCompositionMapLightPreviewActive`를 둔다. owner는 기존 `m_strCompositionWorldPreviewPattern`이고 수명은 Begin~Stop이다. CPP의 `Debug_BeginCompositionWorldPreview`는 실제 HOLD book instance를 포함하는 cue가 있는 경우에만 현재 authoring override 또는 runtime provider의 `Get_Document()`를 복사한다. 모든 source/exclude ID 존재·유일성, 새 ID 충돌과 finite 값을 검사해32사본/14비활성화를 stage한 뒤 `Replace_Authored`와 새 provider의 `Replace_Document`를 모두 통과시킨다. 이 준비는 기존 preview를 Stop하거나 Deploy/arena 상태를 commit하기 전에 한다. 실패 status는 기존 WORLD admission 실패 소비자에게 반환하며 이전 preview와 authoring override를 보존한다.

`Debug_SampleCompositionWorldPreview`는 실제 cue의 startMs와 `Get_InstanceElapsedSpanMs`로 HOLD book의 활성 구간을 판정한다. 하드코딩된21.010초가 아닌 편집한 occurrence 수명을 소비한다. 구간 전후 Seek에서는 provider 활성 여부만 바꾸고 재진입에 사용할 사본은 보유한다. Stop/실패/완료/Level 종료는 scoped 포인터와 flag를 해제한다. 외부 MapLight authoring override가 바뀌면 기존 연출을 Stop한 뒤 새 override를 선택하고 다음 Play에서 새 사본을 stage한다. 원본 provider의 문서나 파일을 덮어쓰지 않는다.

**광원 제출 시점도 함께 바꾼다.** 현재 Level `Update()`의 Submit_Frame은 MainApp WORLD Seek/Stop보다 먼저 실행된다. 그 자리에 두면 경계 프레임이 한 프레임 늦고 추가 제출은 이중 광원을 만든다. 기존 Level Update의 제출을 제거하고, Level의 public `Submit_MapLightFrame()`이 scoped/authoring/runtime 중 하나를 선택해 한 번 등록하도록 한다. `CMainApp::Render()`의 `Render.World` 직전, 최신 active Kouku Level에 이 함수를 한 번 호출한다. Debug 밖의 일반 재생도 동일 경로를 사용한다. 이미 등록한 provider에 `Clear()`를 호출하지 않고 owner 포인터만 reset한다. 등록된 provider를 unready로 바꾸면 Renderer의 Submit_FrameProviders에서 전체 프레임이 실패할 수 있기 때문이다. registration 실패는 기존 로그/status 경로로 드러내고 두 번째 provider를 중복 등록하지 않는다.

### G06-06. 책·세이튼·아레나 표시의 소유권

`Client/Private/Level_KakulSaydonArena.cpp::Initialize`의 기존 paper bridge 초기 숨김 batch에2개를 더한다. 현재 vector reserve도2개 늘리고, 기존 Set_States 실패 rollback을 그대로 사용한다.

```cpp
hiddenBridges.emplace_back(
    KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
hiddenBridges.emplace_back(
    KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
```

기존 `original_kouku`는 Deploy5를 Begin에서 대여하고 종료에 이전 상태를 복구한다. HOLD book도 기존 `world.object.kouku.popup.book` ID를 유지하므로 Deploy7 숨김 인식이 연결된다. P4에서 source SaydonBook/SaydonStage/SaydonFinale 참조를 제거하면 별도 중복 배우는 생성하지 않는다. WORLD STOP은 먼저 animation borrow를 해제한 다음 이전 Deploy 상태를 돌려준다. 사용자가 이후 바꾼 상태를 덮어쓰지 않는 기존 appliedState 비교를 유지한다.

`WorldSequencePlayer.cpp` 익명 namespace의 `Resolve_ObjectMotion` 함수에서 STOP/HOLD 자체를 전역 변경하지 않는다. MAP track의 최종 키 유지도 기존 Sample 경로를 사용한다. P1/P4 box 수명 변경으로 해결하고, 현재 standing arena 전환의 조건은 모션 template 길이가 아니라 살아 있는 MAP occurrence에 계속 종속시킨다. P4 종료 후 전투 진입은 기존 Server 승인·Complete Play 경계를 사용한다.

### G06-07. scene profile publish의 float32 저장 왕복 수정

`Publish-RenderingProfiles.ps1::Assert-FiniteFloatRange`는 입력을 double로 읽은 채 float32 하한을 double로 확장한 값과 비교한다. 그래서 9자리로 저장된 `0.100000001`을 C++에서는0.1f로 정상 읽으면서 publisher에서는 거절한다. `0.1`만 정밀 문자열로 되돌리는 수정은 하지 않는다.

이 함수는 JSON number 타입과 double finite를 먼저 확인하고, single 변환의 예외/overflow 및 single finite를 검사한 뒤 **변환한 single 값**을 single 하한·상한과 비교하도록 교체한다. `CRenderingProfileService.cpp::Read_Float`의 순서와 같게 한다. 현재 `Assert-FiniteRange`를 쓰는 base fog와 region fog의 runtime float 필드, blendTimeIn/Out도 이 함수로 변경한다. 특히 `heightFalloff=9.99999975e-05`도0.0001f와 같은 값이라 통과해야 한다. version/revision, 별도 double 원문 검사를 요구하는 bound/plane 검증은 무조건 바꾸지 않는다.

effectiveExposure/effectiveBloom 계산은 각 operand를 single로 읽은 뒤 product를 single로 변환해 runtime과 맞춘다. 기존 테스트 파일에0.1/0.0001/0.0312의9유효숫자 저장→Validate/Publish 왕복, 다음 범위 밖 float32, NaN/Infinity/overflow, 실패 시 기존 destination 보존을 추가한다. Rendering runtime revision이 authored와 같은지 검사하는 기존 테스트는 정상 Publish 후에 실행한다. 실제 실행 결과는 RESULT G08에 구분한다.

### G06-08. 구현 순서·검증·완료 경계

다음 구현은 ①WORLD·배우 수명과 정상 카메라 연결, ②효과 시간 재배치, ③재질6행과 scoped 환경/광원, ④publisher 경계와 게시 순서로 진행한다. JSON stage/validate가 끝나기 전에 runtime을 부분 교체하지 않는다. 기존 SourceSequence builder를 재실행해 현재 P4 저작을 덮어쓰지 않는다.

계획 자료의 baseline SHA256은 Sequence `196bd9edb43a14843c590d3623a7b417442851fac863c97a167f680a3bd45825`, WorldSequence `d0a50fd22687b03d73de308c3a502adbe39788fb49377dcc002ce38849ef9bdc`, CameraShots `fc674d10f87ce1c0bbfd7e2dd9f1a1403e0624c17ea885a399e68c3d5b084271`이다. 실제 구현 시 최신 문서를 읽고 의미 단위로 병합하며 이 SHA와 다르다는 이유로 사용자 변경을 되돌리지 않는다.

구현 후에만 아래 공식 검증·게시를 실행한다. 이것은 이번 조사에서 실행한 명령 목록이 아니다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Check
python -B -m unittest discover -s Tools/RenderingPipeline -p test_publish_rendering_profiles.py
git diff --check
```

Visual Studio 2022 Developer PowerShell에서는 아래 기존 Client 프로젝트 명령으로 필요한 변경을 빌드한다.

```powershell
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m:1 /nologo
```

변경한 C++는 기존 Client Debug x64 빌드로 컴파일한다. 새 H/CPP·shader 등록은 없다. `Client/Default/Client.vcxproj`의 기존 Effect None ItemGroup에 G06-03의4개 JSON을 `..\..\Data\Effects\...` 상대 경로로 추가하고, `Client.vcxproj.filters`에는 같은 Include와 `<Filter>96.DataFiles</Filter>`를 추가한다. 기존 항목과 필터는 재배치하지 않는다. 실제 데이터 변경과 함께 Area publisher의 runtime map 출력을 전달한다. 이번 계획에는 Resources 신규 물리 파일이 필요하지 않다. ZIP은 코드 적용·게시·빌드가 끝난 후 별도 배포 단계에서만 갱신하며 실제 ZIP 갱신 여부와 배포 경계는 RESULT G08에 기록한다.

CPU/문서 검증은 stable ID, WORLD23+136배치 참조, camera 키 유한성,3쌍 WMSH 동일성,6행 material 참조, 광원151행/3SPOT사본/중복 제출0,실패 시 기존 문서 보존을 확인한다. 시간 경계는 popup local2369/2370ms와 실재생 clip 종료 전후,4506/4507ms,통합21009/21010ms,58809/58810ms,역방향Seek와 중간Stop이다. 특히 box가 늘어나도 모션 속도가 느려지지 않고 마지막 pose를 유지해야 한다. 광원·profile·Deploy와 카메라가 같은 종료 프레임에 반환되는지 확인한다.

사용자가 직접 `KoukuSaydon → F1 Developer Tools → Sequencer Benchmark/Composition Sequencer → 1관문_통합_시퀀스`를 재생해 문 열림,포탈/흡입,암전,책 펼침,커튼과배경,세이튼1명,최종F1전환을 확인한다. 독립 `연출_팝업북`, `연출_1관문 피날레`도 보존 여부를 비교한다. 이 조사 결과는 화면 복원 PASS가 아니며 재질·조명과 마지막 전환의 시각 일치는 사용자 관찰로만 완료 처리한다.

읽기 전용 근거는 `out/KoukuPopupFinale20260912/sequence-transition-diagnosis.json`, `sequence-plan-snippets.json`, `material-candidate/PLAN_MATERIAL_LIGHTING_NOTES.md`, `region_light_comparison.json`, `f1_placement_correspondence.json`에 있다. 하위 메모의 새 MapLight rig API 제안은 G06-05의 기존 문서/provider 재사용 및 제출 시점 수정으로 대체한다.

## G00. 원본 시간축과 저장 경계

기존 `연출_팝업북`, `연출_1관문 피날레`, 사용자가 편집한 `2관문_진입컷씬`은 보존한다.
SCENE03A Matinee0의 41.487556초를 신규 `1관문_통합_시퀀스`의 시간축으로 사용한다.
포탈은 2.344762초, 흡입은 4.354424초에 시작하며 7.459291초의 암전 뒤
10.218567초부터 책이 펼쳐진다. 기존 두 패턴의 재생 시간을 단순 합산하지 않는다.

저장 정본은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`,
해당 Area의 `camerashots.json`, `worldsequences.json`과 기존 Effect 문서다.
원본은 `out/KoukuFireworks20260911`의 설치 UPK 추출과
`out/KoukuAllEffects20260912/organization.json`의 Matinee occurrence를 대조한다.
기존 JSON을 다시 읽고 변경 전 바이트를 비교한 후 신규 stable ID만 병합한다.

## G01. 원본 카메라와 화면 전환

`build_gate2_intro_composition.py`의 기존 UE3 좌표 변환·Hermite 표본화·카메라 축소를
Matinee와 InterpData별로 재사용한다. cut 경계를 보존하고 shot당 64key 이내로 나눈다.
UE3 수평 FOV는 16:9 수직 FOV로 변환한다. 180도는 투영 특이점이므로 기존 runtime의
0도와 180도는 runtime의 (1,179)도 범위 안인 1.1~178.9도로 유한 투영하고 원본과의 차이를 결과에 기록한다.
암전은 기존 V2 ScreenPost의 black overlay와 intensity key를 사용한다.
G1 통합과 G3 진입은 현재 아레나 기본 `scene.kakulsaydon.g1.base.v1`을 명시적으로 재사용한다.
이는 원본 환경 전체 복원이 아니며, G2에는 같은 프로필을 강제하지 않는다.

## G02. 월드 모델과 이펙트

책과 Saydon actor는 기존 CModel/WANM 경로로 원본 A/B slot blend를 표본화한다.
새로 생성하는 Resource 위치는 부모 작업과 조율한 뒤 설치하며 기존 파일을 덮어쓰지 않는다.
Saydon의 materialSourceModelAssetId와 modelPreScale은 BossCatalog의 동일 actor를 소비한다.
원본 cinematic drawScale과 전투 크기가 다르면 사용자가 요청한 전투 크기를 우선한다.

원본 Sequence Effect는 이미 설치된 최종 native leaf library를 소비하고 원본 Hermite
movement, 활성 구간과 origin을 보존한다. 지원되지 않은 skeletal attachment는 누락 이유를
기록하며 다른 위치로 임의 배치하지 않는다. runtime sourceTransformTrack 경로를 재사용한다.
Effect tail을 원본 Matinee 종료 시간으로 오인해 전투 진입을 늦추지 않는다.

## G03. 관문별 Sequence와 종료 계약

| stable Pattern ID | 표시명 | 원본 | 종료 |
|---|---|---|---|
| KAKULSAYDON_G1_PATTERN_4 | 1관문_통합_시퀀스 | SCENE03A Matinee0 | 41,488ms |
| KAKULSAYDON_G1_PATTERN_3 | 2관문_진입컷씬 | SCENE04A Matinee2 | 27,000ms |
| KAKULSAYDON_G1_PATTERN_5 | 2관문_클리어 | SCENE02A Matinee10 | 35,368ms |
| KAKULSAYDON_G1_PATTERN_6 | 2관문_카드미로 | SCENE04A Matinee1 | 11,950ms |
| KAKULSAYDON_G1_PATTERN_7 | 3관문_진입 | SCENE02A Matinee10의 SL05 도착 | 원본 도착 cut부터 종료까지 |

부모 작업과 합의한 optional boolean `enterCombatOnFinish`는 entry 4·3·7에만 true다.
Complete Play가 해당 gate의 entry 하나를 선택하며, 전투 boss 생성·player 이동·카메라 반환은
부모 작업의 기존 Server command 경계가 소유한다. clear와 maze는 명시적 연출 preview다.
카드미로 gameplay는 기존 Pattern28의 Server `CARD_MAZE_HIDE_NEXT/ENTER`와 구분한다.

## G04. 검증과 사용자 화면 확인

생성된 JSON parse, stable ID 참조, 원본 cut 시각과 카메라 유한성·key 개수,
Resource 실제 존재와 필요한 기존 Area publisher 구조검사를 실행한다.
현재 사용자 Visual Studio 빌드 중이므로 제품 빌드와 SDK/DLL/CSO 배포를 하지 않는다.
Client/UI 실행·조작·캡처와 최종 화면 판정은 사용자가 직접 수행한다.
새 Python 파일은 Tools 저작 경로이며 C++ 프로젝트 등록이 필요하지 않다.
신규 Effect JSON 15개는 Client 프로젝트/filters의 None, 96.DataFiles에 추가한다.
실행 중인 기존 exe는 새 metadata를 모르므로 Data와 프로젝트 변경은 out 후보로 먼저 검증하고,
호환되는 새 exe 전환 때 전체 baseline CAS를 확인한 뒤 설치한다.

## G05. 실제 WORLD 준비 오류 후속 수정

2026-09-12 실제 사용자 오류는 저작 world revision673과 runtime672의 신규18 instance 누락, G2 Table의 잘못된 WMSH tail 두 경계로 분리한다. Table은 4 submesh에서 원본 투명 슬롯 두 개를 제거하면서 4개의 bounds를 남겼다. 기존 Engine reader는 새 submeshCount2에 맞는 bounds80bytes만 소비하여 나머지80bytes를 거절한다.

visible_table_mesh는 bone10개를 그대로 보존하고 선택된 submesh1/2의 bounds만 남긴다. 이미 저작된 WANM, skeleton, material section은 그대로 유지한 교체 후보를 out/KoukuSequenceAdmission20260912에 작성한다. 실제 decoder로 후보 및 신규5개 시퀀스가 참조하는 모델을 검사하고, Data/Resources의 현재 bytes는 보존한다. runtime Area Publish와 기존 Table 교체는 실행 중인 사용자 Client/Server 종료 후 부모 작업이 소유한다. 이 검사는 CModel GPU 생성이나 화면 성공 판정이 아니다.

Level의 WORLD admission 실패는 missing instance, disabled, duration, speed, stable ID, duplicate를 구분하고 occurrence/instance ID와 실제 문서 revision 및 supplied snapshot/runtime 출처를 표시한다. WorldSequencePlayer는 기존 CModel Create 직후 같은 thread의 ModelDecoderRegistry report가 요청 meshPath와 일치할 때 실제 decoder 실패 이유를 덧붙인다. decode 성공 뒤 geometry/material 생성 실패는 별도로 표시하며 다른 모델의 오래된 report는 사용하지 않는다. 새 로더나 Engine public API는 만들지 않는다.


## G09. Rendering Benchmark Bloom 1.3과 게시 경계

사용자가 요청한 쿠크 레벨 Bloom의 정본은 scene.kakulsaydon.g1.base.v1의 qualityOverride다.
bloomIntensity를 1.3으로 변경한다. 팝업북 profile의 qualityOverride는 변경 전 기본 profile의
18개 값과 모두 동일한 복사본이므로 제거하여 기존 active Level 품질을 상속한다. 팝업북의
light/fog/environment 및 scene multiplier, 다른 Level과 전역 quality는 보존한다.
revision을 한 번 증가시키고 공식 RenderingProfiles publisher로 Validate와 Publish를 수행한다.

Save/Authored, Publish/Runtime, Reload/메모리의 실제 소유권과 오류 전달을 조사한다.
UI Publish가 stdout/stderr를 버리고 UI thread에서 동기 대기하는 구조는 확정 사항으로 기록하되,
사용자의 실제 실패 문구 없이 개별 실패 원인을 단정하지 않는다. 이 변경은 JSON과 게시이며
C++ 변경·Client/UI 조작은 하지 않는다. 검증은 두 JSON parse, 원본 변경 범위와 공식 게시,
기존 RenderingProfiles publisher 회귀로 한정한다.

## G10. 2026-09-13 MAP 중심·독립 포탈 시계와 연출 애니메이션

기준은 `kouku-pattern3-sequence` / `3fc23750761107fee9c6933df9f926d93165de05`다.
사용자가 저장한 Sequence revision19의 폭죽 추가와 포탈 조절을 out 원본에 보존한다.
실행 중 미저장 편집이 있으므로 Effect·Sequence·WorldSequence 데이터는 먼저 별도 후보로
검증한다. 사용자 Save 이후 최신 저장본과 해당 stable ID의 변경 필드만 합치며 외부 수정
freshness 검사를 제거하거나 현재 문서 전체를 이전 후보로 덮어쓰지 않는다.

`portal-arrival.1`의 공통 원점은 오망성의 실제 emitter 중심에서 약19.694m 떨어졌다.
이 문서의 오망성25행·흡입5행을 독립 Effect로 분리하고, 오망성의 source 중심을
회전 원점으로 사용한다. 원본 mesh normal, TypeData pre-rotation, source actor basis를
실측하여 수직 문 방향을 복구한다. 포탈 첫 발생을 local0초로 옮겨 Box Start가 출현 시각을
소유하게 한다. 같은 원본 문서의 쥐·금빛120행과 두 번째 문서의 금빛24행은 원래 시각과
좌표를 보존한다. 새 데이터만 기존 Effect catalog와 Client 96.DataFiles/filters에 등록한다.
새 C++ 파일이나 별도 Effect runtime을 만들지 않는다.

폭죽은 원본3발사점과 saved MAP root, 지면 높이, source attachment, 실제 particle 수명,
현재 카메라의 위치를 대조한다. Box24785ms, 방출 구간6.2385초와 tail을 포함한 실제 Playback 수명12.2385초를 구분한다.
위치와 원본 발사 높이를 보정하고 원본 재질·입자 및 금빛 연출에 전역 보정을 적용하지 않는다.
`KoukuSaydonActionWorkbench.cpp::Render_PresentationAnchor`에는 현재 플레이어 좌표와
MAP 원점까지 거리를 표시하고, F1 Move Player 완료 뒤 Use Player Position→Apply→Save
순서를 명확히 한다. 입력은 기존 편집 상태와 stage/commit 경로로 전달한다.

현재 P4 world.13은 `world.sequence.instance.original_kouku`의 실제12 animationTracks를
사용한다. `세이튼_1관문_연출` 이름의 placed-sequence alias를 기존 World Object 정본에
추가하고, Composition World 표시명도 맞춘다. 기존 Animation Clips 편집·삭제·Save가 같은
sequence instance를 소비하므로 별도 Action Pattern으로 animation을 복제하지 않는다.
WorldObjectTool의 목록에는 각 clip의 구간과 공유된 저장 대상을 표시한다. 사용자가 지우기
전에는12개 애니메이션을 모두 보존한다.

## G11. Parent fixedTimeline 게시 실패와 revision 일치

실제 Action 저장 revision420에 대해 게시 로그는 World publisher의 `fixedTimeline` unknown
field 거부와 전체 rollback을 기록하며 이전 Product382를 유지한다. Sequence19와 Action420의
revision은 서로 다른 owner다. `Publish-WorldGameplay.ps1::Get-EncounterProfiles`에서 Kouku
Parent의 optional boolean fixedTimeline을 기존 Gameplay publisher 계약과 같이 검증한다.
unknown field 거절, 잘못된 타입 거절, 저장·게시·Server-active revision 검사는 유지한다.
기존 projector focused test에서 실제 PowerShell reader를 사용해 true/false/생략을 승인하고
잘못된 타입·다른 encounter·unknown field를 거부한다.

검증은 현재 codec/CPU playback의 source transform·수명·되감기, 실제 shader/resource 연결,
JSON/XML parse, 사용자 변경 보존, 필요한 증분 Product Build와 동일 Area Publish/Check,
최종 Kouku owner의 Product/Map/World/Balance 게시로 수행한다. 실행 중 Client/Server는
종료하지 않는다. 미저장 편집 보존과 출력 잠금 해제 뒤에만 정본 교체·최종 링크를 수행하고,
사용자의 화면 조작·오망성/폭죽 시각 판정을 자동 성공으로 기록하지 않는다.


## G12. 맵 연출 예산·실제 배우 스포트라이트·쇼타임 연결

맵의 모든 LevelPlacement를 하나의 캐릭터 owner2048 mesh 예산에 합산하는 경계를 수정한다.
기존 포탈/금빛1995와 폭죽238은2233으로 owner한도를 넘지만 scene한도4096 안에 있다.
LevelOwner만 기존 scenehard 한도를 소비하고 Character/Boss owner 및 remote soft 한도는 보존한다.
실제 예산 함수로 동일 시퀀스 통과, 캐릭터 제한·scene 최대·pending·overflow 거절을 검증한다.

MapLight resource의 절대좌표는 MAP에서만 소비한다. BOSS/PLAYER/WORLD는 기존 광원 모양과
방향·range를 유지하며 실제 대상 높이의 평면을 비추도록 source ray 중심을 대상 원점에 맞춘다.
WORLD는 기존 world/occurrence stable ID와 같은 pivot resolver를 사용한다. P4의26525ms 광원은
전투 보스가 아닌 world.13의 연출 세이튼을 따른다. resource Preview도 선택한 관문/actor owner를
사용한다. parser/publisher/UI/runtime를 함께 연결하고 기존 MAP와 local actor light를 보존한다.

Showtime의 원본20종 문서와 carrier/texture, R-Hand·B_WP1/2와 실제 WORLD 총구 basis를
대조한다. 원본 bone용 자산은 유지하고 사용자의 WORLD 총에 맞는 독립 총구 occurrence를
연결한다. target.fixed의 BOSS+절대 MAP 좌표 중복을 제거하고 바닥/노란 표적의 실제 방향과
깊이를 검사한다. source ProjectileTrace의 지면 dust/impact를 비행탄으로 재해석하지 않는다.

팝업북은 현재136 MAP 배치·책의 source material과 누락 carrier를 대조하며, SCENE03A의
동적 광원 brightness/move/rotation 트랙을 기존 typed light 경로로 복원한다. POINT/SPOT/
DIRECTIONAL 타입을 보존하고 임의 point 치환이나 전체맵 shader 보정을 하지 않는다.
후보별 정확한 원본/현재 범위와 검증된 완료·미완료 경계는 RESULT에 기록한다.

## G13. 팝업북 StaticMeshActor 재질 곡선과 기존 native 입력

SCENE03A의 actor672·691·692·811·812·848는 기존136 MAP 배치에 없는 안개·바닥 평면·흰 섬광·빛기둥·부착 커튼이다.
`build_gate1_popup_carriers.py`는 실제 StaticMeshComponent와 연결된 Matinee group의
move/material parameter track을 읽고 기존 SourceTransformTrack의 시간축으로 투영한다.
WModel geometry와 source texture는 원본과 대조하며 신규 Resources만 설치한다.
다섯 native program3616~3620은 실제 source VS/PS·정적 파라미터와 기존 native table/dispatch를
소비한다. rendererShape만 보고 Cascade mesh particle로 승인하지 않고, 해당 source static
mesh 프로그램에만 명시적인 sourceTransformMesh 계약을 부여한다.

`EFFECT_SOURCE_TRANSFORM_TRACK::MaterialParameterTracks`는 선택적인 이름·SCALAR/VECTOR·
기존 Distribution을 소유한다. codec은 실제 프로그램의 parameter table로 이름과 타입을
검증하고 renderer staging은 그 table의 row/lane을 보관한다. MaterialBinding은 기존
Frame sample time에 SourceTimeOrigin을 더해32행 native packet 중 해당 lane 또는RGB만
갱신한다. vector의W, 다른 파라미터, 곡선 없는 기존 문서의 packet은 유지한다.
알 수 없는 이름·중복·타입 불일치·비유한 값은 저장/준비 단계에서 거절한다.
별도 애니메이션 시계나 Particle 위장을 만들지 않는다.

white_t의 op/color와 shine의32.noisestr/31.fresnal_power 네 곡선을 기존 이동과 같은
P4 start12258ms/duration46552ms에 배치한다. 평면 actor691·692의 실제 슬롯은
EngineMaterials.DefaultMaterial이다. cooked graph는 삭제됐지만 main shader cache의
정확한 material GUID/static set/repeated set과 LocalVF BasePass VS/PS를 확인했다.
추출기는 기존 count0 고정 가정 대신 실제 global shader 참조3개를 소비하고 VF를 읽는다.
별도 radial-blur shader가 캐시에 참조됐다는 이유로 화면 blur 실행을 추가하지 않는다.
두 평면의 source bUseQuatInterpolation은 선택적인 node bool로 보존하며 기존 source
Euler endpoint를 quaternion으로 변환한 shortest-arc slerp를 같은 transform 경로에 연결한다.
flag가 없는 기존 문서는 기존 계산과 직렬화를 유지한다. 누락의 물리·시간·원본 근거와
최종 복구 범위는 RESULT에 구분한다.

커튼848은 actor819→camera4→curtain848의 원본 부모3개를 같은 node chain으로 소비한다.
camera4는 이번 구간의 활성 시점 카메라가 아닌 transform carrier다. 원본 director 카메라와
가림 전 frustum 대조에서 P4 약22.71~26.99초의 기여 후보를 확인했으며 source material
curtain01c_mi의 실제 정점 alpha·tangent view/up·masked discard를 native3620에 연결한다.
기존 MAP 커튼8개와 이미 등록된 child356은 새로 생성하지 않는다.


## G14. Effect Box Preview의 시작 대기 제거

Box Detail의 명시적인 Effect Preview는 현재 커서의 정지 geometry preview 대신 해당 박스의
`iStartMs`에서 기존 Pattern preview를 재생한다. 이펙트의 local age는 0으로 시작하면서
BOSS 본 애니메이션과 WORLD sequence는 같은 절대 Pattern 시각을 소비한다. 일반 TRS
드래그의 geometry preview는 현재 커서와 일시정지 상태를 유지한다. Box Detail의 미적용
시간·배치 값은 기존 검증된 임시 override로 전달하며 저장 파일과 다른 occurrence는 바꾸지 않는다.

호출자는 `Queue_PresentationPreview`의 EFFECT occurrence 분기이고 소비자는 기존
`Request_PatternPreview → MainApp → CKoukuSaydonPresentationPlayer::Sample`이다.
저장 포탈의 내부 첫 발생은 이미 0초이며 14603ms는 timeline 박스의 시작값이다. 해당 값은
사용자가 타임라인에서 조절한다. 실제 Workbench 요청·검증·임시 문서와 source 보존을
CPU probe로 확인하고 변경한 Workbench를 격리 컴파일한다. 실행 중 Client/Server와
미저장 저작 문서는 유지하며 최종 화면은 사용자가 확인한다.

Effect Tool의 Current Effect에는 공통 앞 대기와 `Remove Leading Delay`를 표시한다.
지원 범위는 모든 요소가 독립 SourceTransformTrack을 가지며 model cue, source actor,
본 부착, transform inheritance, baked history, enabled SourcePresentation을 쓰지 않는
맵 연출 문서다. 기존 Detail 미적용 값은 먼저 Apply하도록 보호하며 문서 전체 candidate를
검증한 뒤 기존 `Try_CommitDocument`로 한 번 교체한다. 모든 요소의 StartDelay에서 같은
최솟값을 빼고 각 SourceTimeOrigin에 더한다. 상대 발생 간격, native emitter delay, 수명,
원본 transform/alpha/material 곡선은 보존한다. Save Changes만 실제 파일을 쓴다.

현재 Sequence revision22의 centered 박스는 사용자가 이미 start0/duration36541로 저장했다.
별도 portal context의17.134767초 내부 대기 제거 후보를 out에 준비하고 원본 SHA 및
2개 필드만의 변경 영수증을 남긴다. 실행 중 저작 문서에는 외부 덮어쓰기를 하지 않는다.

## G10. 통합 암전의 A coverage 입력 교체 (2026-09-13)

사용자 요청은 포탈을 제외하고 재생 시 암전이 표시되도록 수정하는 것이다. C++/HLSL, 카메라 FOV/위치, 포탈, 다른 컷신, 기존 암전 intensity/time 키는 변경하지 않는다.

현재 CEffectV2Object::Submit_Presentation은 PRESENTATION_SCREEN_OVERLAY_DESC 기본 A coverage를 사용한다. 원래 base DDS는 DXT1(BC1)이며 CPresentation_Manager의 HasOverlayCoverageChannel A 분기는 BC1을 허용하지 않으므로 Add_ScreenOverlay가 거절한다. 픽셀을 RGBA로 디코딩해서 흰색인지 확인하는 것만으로 runtime format admission을 통과했다고 판단하면 안 된다.

기존 Resources/Map/LV_BER_BERNCASTLE/SourceMaterials/50c22a2692ed_flat_white.dds는 DXGI 28 RGBA8, 2x2와 1x1 mip, 모든 채널 255이다. 원본을 바꾸지 않고 아래 암전 전용 경로에 동일 bytes로 복사한다.

- 추가 물리 파일: Client/Bin/Resources/Effect/KoukuSaydon/Textures/Cutscene/fade_opaque_white.dds (168 bytes, Git 제외 runtime 입력).
- 수정 정본: Data/Effects/V2/Authored/kouku.gate1.authored.fade.black.effectv2.json의 slots.base 한 필드.
- 정확한 교체 행: `"base": "Effect/KoukuSaydon/Textures/Cutscene/fade_opaque_white.dds",`
- 기존 DXT1 공용 텍스처, 원래 timing 변경 16개 및 params 전체는 보존한다. 기존 JSON은 프로젝트에 등록되어 있으므로 신규 project/filter 등록과 C++/FX 빌드는 없다.

검증은 변경 전후 JSON의 base 외 모든 값 동일, RGBA8/A=255 및 runtime 허용 목록 일치, 해당 Effect V2 authoring validator, git diff --check로 수행한다. 기존 실행 파일에서 Stop 후 새 Play로 leaf snapshot과 객체를 다시 만들면 저작 JSON을 읽는다. 0초/20.7초의 검정 및 23.2초의 복귀, 실제 화면 합성은 사용자가 확인한다. 다른 PC에는 새 DDS 상대 경로를 함께 전달한다.

## G12. 2관문 진입·3관문 진입 전체·빙고 최종 엔딩 제작 계획 (2026-09-13)

### G12-00. 이번 요청과 완료 목표

사용자 확인으로 대상은 아래 세 개뿐이다. **이번 차례는 계획 작성이며 제품 코드·JSON·리소스 변경과 빌드는 하지 않는다.** 앞서 준비한 G11 포탈/흡입 카메라 후보는 Client 실행 때문에 설치하지 않았고, 이번 작업과 분리해 보류한다. G11 후보가 보관된 out/PortalFix20260913의 옛 PLAN을 현재 문서 위에 복사하지 않는다.

| 사용자 대상 | 사용할 Pattern | 이번 제작 방향 |
|---|---|---|
| 2관문_진입 | 기존 `KAKULSAYDON_G1_PATTERN_3` / 현재 표시명 `2관문_진입컷씬` | 이미 있는 배우·책·카드 무대·카메라를 원본/영상과 대조해 보완 |
| 3관문_진입 | 기존 `KAKULSAYDON_G1_PATTERN_7` | 사용자가 요청한 **출발·비행부터 도착까지 전체**로 확장. 현재 18,658ms의 도착 부분만 재생하는 구성을 대체 |
| 빙고_최종엔딩씬 | 신규 독립 Sequence Pattern | 빙고판에서 쓰러짐 → 두 배우 연기 → 배우 교체 → 마지막 숨김까지 새로 연결 |

P1 `연출_팝업북`, P2 `연출_1관문 피날레`, P4 `1관문_통합_시퀀스`는 제작 방식의 참고이며 수정 대상이 아니다. P5 `2관문_클리어`는 3관문 진입 전체 원본의 기존 연결을 참고하는 읽기 전용 재료다. P6 카드미로, 앵콜/가짜 클리어도 이번 대상이 아니다. 영상의 1관문 종료 부분을 별도 네 번째 Pattern으로 만들지 않는다.

완료 목표는 **각 항목 하나를 선택하면 카메라·배우·맵 동작이 함께 재생되고 기존 Box Detail / Object Tool / Camera 편집기로 수정·저장할 수 있는 상태**다. 목록에 이름만 추가하거나 전체 타임라인을 안내문으로 대체하는 것으로 끝내지 않는다.

### G12-01. 담당 경계 — 팀장 작업과 분리

이번 담당: 원본 데이터 선택, 카메라 컷·경로·roll/FOV, 배우/소품 animation·transform·visibility, 필요한 맵 배치, Sequence 연결, 저장·재로드·중단 복구.

팀장 담당으로 제외: 전등/스포트라이트, 밝기·환경광·안개·SSAO·Bloom, 암전 곡선/합성, 재질 shader·텍스처·RNM 복구. 기존 LIGHT/암전/재질 문서의 내부 값은 변경하거나 제거하지 않는다. 단, P7의 앞부분 16,710ms 추가에 따른 **기존 occurrence 박스 시각 이동만** G12-06처럼 수행해 같은 원본 시각을 유지한다. 이는 효과 내부의 복구/튜닝이 아니다. 새 배우의 모델에 이미 있는 재질·텍스처 참조는 보존하되 새 재질 복구를 수행한 것으로 기록하지 않는다. 새 배치와 파생 모델에 필요한 기존 재질 참조의 보존은 재질 계산의 수정과 구분한다.

그 외 연기·카드·소멸·비행 터널 FX는 이미 연결된 호환 리소스가 있으면 기존 시각/공간 관계를 유지한다. 신규 FX shader/재질 복구는 이번 1차 구현의 선행 조건으로 추가하지 않는다. 특히 3관문 보라색 터널은 **움직이는 맵 벽이 아니라 원본 particle mesh 효과**다. 안 보인다고 거대한 임시 불투명 맵이나 임의 암전으로 가리지 않는다. 필요한 원본 ID·시각·좌표는 팀장 인계 목록으로 남긴다.

실제 전투 승리 시 자동 재생, 보상/결과 UI, 플레이어 워프/동기화와 Server death lifecycle 변경은 이번 툴 저작 목표 밖이다. 엔딩 배우는 컷신용 WORLD이며 죽은 전투 보스를 HP 1로 살리거나 dead guard를 해제해 재생하지 않는다.

### G12-02. 직접 읽은 영상과 원본 시간표

아래 영상은 각각 24개 시점, 총 72개 프레임을 오프라인으로 열람했다. 표의 영상 구간은 장면 구분용 근사값이다. Client 화면을 실행·캡처한 결과나 구현 후 visual PASS가 아니다.

| 영상 | 실제 파일 길이 | 원본 대응 및 저작 시간 |
|---|---:|---|
| [2관문 컷신 .mp4](<C:/Users/USER/OneDrive/바탕 화면/2관문 컷신 .mp4>) | 27.3333초 / 30fps | SCENE04A / Matinee2, 27,000ms. 1관문 종료 후 2관문 진입 연출 |
| [3관문 컷신 .mp4](<C:/Users/USER/OneDrive/바탕 화면/3관문 컷신 .mp4>) | 36.4666초 / 30fps | SCENE02A / Matinee10, 35,368ms 전체. P7의 source start=16,710ms를 0ms로 확장 |
| [진짜 마무리 .mp4](<C:/Users/USER/OneDrive/바탕 화면/진짜 마무리 .mp4>) | 47.3333초 / 30fps | 빙고 공간 SCENE01B / Matinee0, 49,083ms를 기본 소스로 사용. SCENE01C 공간 변형과 혼합하지 않음 |

영상 3개는 모두 2160×1440 녹화 컨테이너이며 플레이어 UI/여백을 포함한다. 이 비율을 실제 게임 viewport aspect나 원본 카메라 aspect로 사용하지 않는다. 영상 제목 자막, 재생 바, 마우스, 스킵 투표 숫자는 재현 대상이 아니다.

| 대상 | 영상에서 관찰한 주요 구간 | 먼저 맞출 기하·동작 |
|---|---|---|
| 2관문 | 약 0~2초 기존 무대 전경, 2~11초 큰 세이튼/작은 쿠크, 13~16초 전환·책, 16~20초 카드 무대 펼침, 20~27초 작은 쿠크/카드 분출 | 배우 위치·손/책 접촉, 책과 무대 중심, 펼침 방향, 근접/전경 카메라 전환 |
| 3관문 | 약 0~6초 카드 무대 출발, 7~15초 비행, 17~19초 천막 도착, 20~31초 배우 연기, 31~36초 전경·커튼·게임 화면 복귀 | 출발 배우 두 개의 상대 pose, 비행 경로, 천막 도착 위치, 커튼/바닥/배우 구도 |
| 빙고 엔딩 | 약 0~8초 쓰러짐, 10~15초 얼굴, 16~21초 누운 배우와 작은 쿠크, 22~31초 일어나기/잡기, 31~38초 합쳐진 형태, 39~47초 소멸과 빈 바닥 | 바닥 접촉, 무기/손 위치, 작은 쿠크 크기/부착, 배우 교체와 숨김 시각 |

원본 시각을 우선한다. 녹화 길이를 원본 길이로 나눈 비율로 전체 키/속도를 압축하지 않는다. 복수의 대응 장면으로 앞뒤 편집·슬로모·시작점 차이를 비교하고, 사용자 편집이 필요할 때만 원본→저작 시간 대응표를 별도로 적용한다. 빙고 엔딩은 원본의 첫 camera preroll -133ms를 0ms 상태로 평가하고 음수를 unsigned key로 저장하지 않는다.

### G12-03. 현재 저장값과 재사용 자산

조사 checkout: `C:/Users/USER/source/졸업팀폴/LostArk`, branch `codex/kouku-g1-fade-visible`, HEAD `461224f9`. 2026-09-13 현재 Sequence revision 14 / 7 Pattern, World revision 1736 / 315 object / 201 template / 257 instance, Camera revision 81 / 86 shot이다. 과거 RESULT의 revision 7 등으로 되돌리지 않는다. 구현 직전에 다시 읽어 baseline을 잡는다.

- P3: WORLD 17개, CAMERA 5개, EFFECT 3개. 배우/소품 Saydon·Kouku·Table·Book·HandBook 5개, chair 4개, candle 2개, card eruption 6개가 있다. `Client/Bin/Resources/Map/KakulSaydon/Gate2Intro/`의 기존 모델과 WORLD 정의부터 재사용한다. 촛대 **메시**와 그 위 **조명**은 별개다. 촛대 배치·동작은 이번 범위, 광원 수정은 제외한다.
- P5: SCENE02A Matinee10 전체의 LargeSaydon·Kouku·SaydonArrival WORLD 3개, CAMERA 18개가 이미 연결돼 있다. P7은 SaydonArrival WORLD 1개, CAMERA 8개와 뒤쪽 18,658ms만 가진다. P5의 원본 연결을 대조해 P7 전체 구성의 후보를 만들되 P5 공용 row를 수정하지 않는다. 수정이 필요한 공유 정의만 P7 전용 stable ID로 분리한다.
- 빙고 바닥/폭탄/망치 리소스는 있지만, 최종 엔딩 전용 Pattern·배우·Camera 구성은 없다. 전투 망치·폭탄을 원본에 없다는 이유 없이 엔딩에 함께 재생하지 않는다.
- 기존 원본 조사: [앵콜/마지막 계획 G06~G12](../09-10/2026-09-10_KOUKU_ENCORE_FINAL_CUTSCENES_PLAN.md), [동일 작업 결과](../09-10/2026-09-10_KOUKU_ENCORE_FINAL_CUTSCENES_RESULT.md), [원본 시각 감사](../09-11/2026-09-11_KOUKU_CUTSCENE_SOURCE_VISUAL_AUDIT_RESULT.md). 앵콜·Server·조명까지 포함한 옛 전체 계획은 이번 승인 범위가 아니며 G12의 좁은 경계를 우선한다.

기존 09-10 문서의 “pose가 up vector를 담지 못한다”는 설명은 현재 코드에 그대로 적용되지 않는다. 현재 CameraShot key/pose와 Apply_PresentationPoseWithUp 경로가 있으므로 roll 보존에 새 카메라 런타임을 만들지 않는다. 120도 초과 Capture_ViewPose 수정은 별도 G11 미반영 후보이며 이번 계획에 적용 완료로 집계하지 않는다.

### G12-04. 1단계 — 세 컷신의 원본 연결표 확정

입력은 원본 UPK/추출 JSON/현재 저작값/영상이다. 원본 파일은 읽기 전용으로 보존한다. 세트마다 `package → Matinee → Group → Actor → model/clip → parent → 시간 → 현재 resource/instance`를 연결한 표를 만든다.

기본 원본은 SCENE04A Matinee2 / SCENE02A Matinee10 전체 / SCENE01B Matinee0이다. 빙고 전 이동인 SCENE07A Matinee23(23,333ms)과 최종 엔딩을 혼동하지 않는다. SCENE01C는 Z≈935의 다른 공간 변형, SCENE01B는 Z≈1145 빙고 공간이다. 원작 trigger event/실제 재생 wall time/Matinee InterpLength는 별도 열로 둔다.

실제 UPK의 Kismet 연결도 읽어 원본 선택을 확인했다. SCENE01B(`B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8FD.upk`)는 `37081_342` RemoteEvent(export 213) → Matinee(export 32) → Data(export 45)이며 Completed는 같은 이벤트의 EndRemoteEvent(export 31)로 연결된다. SCENE01C(`…8FK.upk`)는 `37081_341` RemoteEvent(216) → Matinee(31) → Data(44) → Completed/EndRemoteEvent(30)다. 두 Data 모두 23 group / InterpLength 49.083335876초지만 공간이 다르다. 이 연결만으로 원작 난이도별 모든 분기까지 검증했다고 확대하지 않는다. TriggerMapData에 기록된 52.13291초는 이 Matinee 길이와 구분하며, 엔딩 박스 길이로 그대로 복사하지 않는다.

연결표에는 최소 다음을 기록한다.

- 활성 Director cut, 첫 상태, 부모·hardAttach, MoveTrack 보간·tangent, FOV·roll.
- 배우별 clip·sourceStartOffset·sourceEndOffset·playRate·loop/reverse·A/B/C 슬롯·weight·bone/parent.
- 시작 시 숨김, SHOW/HIDE 전환과 최종 표시 상태. actor가 숨겨져 있어도 scene 시간에 맞는 pose를 계산한다.
- 고정 배경과 움직이는 props의 원본 Transform, streaming/visibility 범위. 명시 binding이 없는 group은 같은 이름의 배우를 임의 복제하지 않는다.
- 팀장 담당 light/fade/material/FX 필요 항목은 **정보만** 추출한다. 자동 생성/게시 대상에 넣지 않는다.

기존 빌더의 A/B 처리만으로 C-slot을 무시하지 않는다. 최종 엔딩의 AnimTree 슬롯 순서/mask와 필요한 control을 원본에서 확인한 뒤 오프라인 bake 규칙에 반영한다. 원본에서 해석되지 않은 채널을 다른 clip으로 대체해 “원본 완료”로 승인하지 않는다. 해당 배우의 입력 해석을 먼저 완료한 뒤 후보 bake를 진행한다.

### G12-05. 2단계 — 2관문 진입부터 보완

P3의 27,000ms와 stable ID는 유지하고 표시명만 `2관문_진입컷씬 → 2관문_진입`으로 맞춘다. 기존 17 WORLD를 지우고 다시 만들지 않고 원본 배우·소품별로 모델/clip/placement 차이를 검사한다. 첫 전경, 큰 세이튼 근접, 작은 쿠크의 책·손, 카드 무대 펼침, 카드 분출의 다섯 확인 구간을 기준으로 한다.

원본 Transform이 있으면 축 변환과 부모 합성으로 먼저 배치한다. 원본에는 없는 현재 수동 세트의 위치 보정은 아래 G12-08의 공통 offset으로 적용한다. 책을 독립적으로 옮긴 뒤 손·카메라를 감으로 각각 옮기는 방식으로 시작하지 않는다.

책 모션 종료와 WORLD 표시 종료는 분리한다. 모션의 마지막 pose를 유지해야 하는 구간은 HOLD/표시 수명을 맞춰 중간에 책이나 배경이 초기 상태로 복구되지 않게 한다. 카드 WORLD 6개는 현재 21,040~26,960ms window 및 기존 bake 표현을 먼저 유지하고, 원본 particle 재질 복구는 팀장 영역으로 남긴다.

### G12-06. 3단계 — 3관문 진입을 전체 영상 구간으로 확장

P7의 원본 window를 `[16,710, 35,368]ms`에서 `[0, 35,368]ms`로 바꾼다. 출발 무대 → 두 배우 이동/비행 → 천막 도착 → 연기 → 전경/커튼 → 기존 관문 입장 handoff가 하나의 `3관문_진입`에서 이어지게 한다.

P5의 전체 35,368ms 카메라 shot 18개와 배우 3개를 참조 기준으로 삼는다. 원작 Director 컷은 13개이며 현재 18개는 키 제한에 따른 segment 분할을 포함한다. 배우 donor는 `kakulsaydon.g1.world.28` LargeSaydon / `.29` Kouku / `.30` SaydonArrival이다. 기존 P7 도착 키에 16,710ms를 더하는 것만으로 작업을 끝내지 않는다. 앞부분 배우·부모·세트/visibility도 함께 연결하고 전체 시각에서 다시 샘플한다. 도착 부분을 공유하는 P5 리소스를 수정해야 하면 P7 전용 복사본만 수정한다.

기존 P7 도착 구간의 EFFECT/profile 박스도 아래처럼 함께 이동한다. 내부 effect JSON, 광원/RenderingProfile 문서, 곡선·색·강도·기간은 그대로 둔다. 앞 구간에 새 암전·profile을 만들지 않는다. 이 이동을 누락하면 기존 도착 효과가 출발 구간에 잘못 나온다.

| 기존 P7 occurrence | 기존 start/duration(ms) | 전체 구성 start/duration(ms) |
|---|---|---|
| `.presentation.9` 기존 fade | 0 / 18,658 | 16,710 / 18,658 |
| `.presentation.10` arrival.1 | 1,139 / 17,519 | 17,849 / 17,519 |
| `.presentation.11` arrival.2 | 1,139 / 17,519 | 17,849 / 17,519 |
| `.sceneprofile.1` 기존 도착 profile | 0 / 18,658 | 16,710 / 18,658 |

앞부분의 source FX를 추가로 연결할 경우 위 도착 FX와 같은 source occurrence를 중복 생성하지 않는다. 팀장에게는 이 네 박스의 시간 이동표와 앞부분에서 별도 복구가 필요한 시각만 전달한다. 팀장 소유 leaf/profile 내용 변경은 수행하지 않는다.

출발/도착 세트가 서로 다른 원본 공간인 점을 유지한다. 현재 P5/P7에는 별도 정적 배경 WORLD track이 없으므로 도착 세트 SL05의 텐트·커튼·벽 및 현재 placement/motion 연결부터 대조한다. 카메라·배우·맵의 원본 anchor를 같은 방식으로 평가하고, 무대 두 개를 무조건 동일 위치로 겹치지 않는다. 기존 standing map을 통째로 지우거나 맵 전체를 숨기는 전역 플래그로 FX 빈 구간을 메우지 않는다.

P7의 `enterCombatOnFinish=true`는 기존 입장 계약으로 유지한다. 같은 Gate3에 entry=true Pattern을 또 만들지 않는다. 종료 후 실제 전투 승인/이동은 현재 CompleteSequence의 typed 경계를 사용하며 새 보상/전투 로직을 추가하지 않는다.

### G12-07. 4단계 — 빙고 최종 엔딩 신규 구성

새 표시명은 `빙고_최종엔딩씬`. 현재 nextPatternOrdinal은 8이므로 충돌이 없다면 `KAKULSAYDON_G1_PATTERN_8`이 후보지만, 설치 시 최신 counter를 다시 읽어 발급한다. 기존 저장 ID나 actor index를 강제로 재사용하지 않는다. `gateId=GATE3`, `enterCombatOnFinish=false`, 독립 관찰 Sequence로 만든다. 끝났다고 보상/clear state/새 전투를 생성하지 않는다.

배우는 원본 역할별로 나눈다: 쓰러짐·아픔을 연기하는 세이튼_1, 독립 쿠크, 별도 wp2, 후반 쿠크세이튼, 세이튼에 부착된 작은 쿠크2. 같은 모델을 사용해도 서로 다른 원본 actor다. 전투용 기본 모델을 덮어쓰지 않고 컷신 전용 파생 모델/clip을 기존 CModel 경로에서 사용한다.

| 원본 Matinee 시각 | 필수 상태 |
|---:|---|
| 0ms | 세이튼_1 표시·쓰러짐 시작. 후반 쿠크세이튼/쿠크2는 숨김 상태에서 clock 평가 |
| 28,542ms | 작은 쿠크2 표시. 원본 parent와 약 0.3 scale 유지 |
| 30,933ms | 세이튼_1/쿠크2 숨김, 후반 쿠크세이튼 표시, cam07 전환을 같은 시각에 평가 |
| 41,067ms | 후반 배우 숨김. 컷신용 visibility만 변경하며 Server entity를 직접 despawn하지 않음 |
| 49,083ms | 원본 timeline 종료. 이번 단계에서는 기존 카메라/배치 복구만 처리하고 결과 UI·암전 handoff는 추가하지 않음 |

후반 배우는 25,417ms에 시작된 clip의 진행 상태로 30,933ms에 등장해야 한다. SHOW 시점에 clip을 0초부터 시작시키지 않는다. 별도 wp2가 요구하는 `idle_normal_1`과 현재 무기 clip을 동일시하지 않는다. 원본 동일 rig의 clip을 확보하거나 실제 정적 pose임을 입증한 경우에만 transform-only 표현을 선택한다. 이름을 바꾸거나 clip 0 fallback을 쓰지 않는다.

원본 Director는 10개 컷이다. 원본 시간 0 / 4,933 / 9,567 / 12,967 / 18,800 / 21,800 / 25,000 / 26,875 / 30,933 / 37,000ms를 보존한다. 카메라 shot 내부 blendIn/out은 컷 경계에 임의 gameplay-camera 복귀를 끼워 넣지 않도록 구성한다. 표시 UI상 shot은 내부 key 제한 때문에 더 나뉠 수 있지만, 추가된 segment를 원작의 새 컷으로 취급하지 않는다.

### G12-08. 자동 배치와 수동 배치의 경계

**원본 좌표·곡선·모션이 있으면 자동으로 먼저 만든다. 수동 편집은 자동 결과의 보정층이다.** 카메라 화면을 보며 모든 X/Z를 다시 추측하는 작업으로 시작하지 않는다.

| 자동으로 가져올 것 | 그다음 사용자와 수동 조정할 것 |
|---|---|
| Actor/StaticMesh 초기 Transform, 부모 합성, Move/Scale/Rotation 키 | 현재 프로젝트 세트와 원본 세트의 공통 위치·방향 차이 |
| Director cut, source camera eye/lookAt/up/FOV | 실제 viewport에서 배우가 잘리는 구도, 손/소품 접촉, 가까운 클리핑 |
| animation clip·offset·rate·visibility | 데이터 변환 오차를 해결한 뒤 필요한 연출상의 작은 보정 |
| 원본 배경 배치 및 표시 window | 원본에 없거나 확보되지 않은 앞/옆/뒤 배경의 최소 추가 배치 |

좌표는 기존 UE3→프로젝트 변환을 정확히 한 번 적용한다. 같은 연출 세트를 재배치할 때 공통 rigid transform `p' = R·p + t`를 배우·카메라 eye/lookAt·정적 props 모두에 적용하고 up은 `R·up`만 적용한다. parent-local key까지 다시 world offset을 중복 적용하지 않는다. 사용자가 개별 소품만 바꾸길 원할 때에만 해당 소품의 별도 보정을 둔다.

모델 geometry/골격 preScale(cm→m), Object 공통 scale, key scaleMultiplier는 별개다. 크기 보정으로 이동거리와 오프셋까지 함께 늘리지 않는다. 이동 끝의 scale key가 1로 남아 처음만 커졌다가 작아지는 문제를 다시 만들지 않는다.

카메라 뒤·무대 외곽이 드러나면 먼저 원본 parent, FOV 의미, 배경 lifetime과 누락 placement를 확인한다. 재질 양면화·암전·조명 확대는 이번 담당의 해결책으로 사용하지 않는다. 원본에 없는 보충 배치는 따로 표기하고 사용자가 화면으로 결정한다.

### G12-09. 변환·저장할 파일과 안전한 적용

실제 루트는 `C:/Users/USER/source/졸업팀폴/LostArk/`이다.

| 기존 파일/폴더 | 계획된 작업 |
|---|---|
| `Tools/KoukuSaydonPipeline/build_source_sequences.py` | 필요한 source sampling을 재사용하되 이번 대상만 후보 생성하도록 범위 제한. visibility와 final A/B/C 입력 해석을 추가. 기존 CONFIGS 전체 실행 금지 |
| `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py` | 원본 camera/parent/curve/clip 처리 재사용. 필요한 수정만 추가하고 기존 P3 결과와 비교 |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | P3/P7의 해당 WORLD/CAMERA 연결 보완, 신규 엔딩 추가. P1/P2/P4/P5/P6와 비대상 resource 보존 |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | 신규 전용 actor/prop/motion, 원본 visibility 및 수명. 공용 정의 수정은 사용처를 확인한 뒤 분리 |
| 같은 폴더의 `LV_LUT_MIDNIGHTC_ED.camerashots.json` | P7 전체와 엔딩 shot, P3의 필요한 보정만 저장 |
| 같은 폴더의 `LV_LUT_MIDNIGHTC_ED.mapplacements` 및 필요한 catalog/Deploy 정본 | 원본에서 확인된 누락 배치만 추가. 팀장 재질 asset 행과 기존 배치값을 덮어쓰지 않음 |
| `Client/Bin/Resources/Map/KakulSaydon/SourceSequences/` | 새 컷신 전용 파생 WModel/animation. 원본 actor/rig/clip 및 생성 설정을 추적하고 기존 파일 보존 |
| `Client/Default/Client.vcxproj` / `.vcxproj.filters` | 새 Git 관리 JSON이 실제 추가될 때에만 `96.DataFiles`의 None 항목 등록. 새 C++/FX 파일은 1차 계획에 없음 |

기존 builder의 P4 설정은 아직 41,488ms이고 현재 저작 P4는 58,810ms다. `build_source_sequences.py --install` 전체 재실행, 기존 merge 충돌 검사 제거, 기본 생성물로 사용자 저장값 덮어쓰기는 금지한다. 기존 destination의 clip 이름/개수만 맞는다고 최신 bake로 인정하지 않는다. 이번 대상의 입력/설정과 결과를 대조하고 별도 candidate에서 검증한다.

후보는 source snapshot → parse/validate → 생성 → 전체 참조 검증 → 사용자 Save/종료 → baseline 일치 확인 → 백업/적용 순서로 처리한다. 원본·candidate·수동 보정값을 구분해 외부 Save가 변경을 덮어쓰지 않도록 한다. 적용 전에 원본이 달라졌으면 설치를 중단하고 그 값을 기준으로 재병합한다.

현재 World JSON은 14,387,090 bytes, 한도 16MiB까지 약 2.39MB 여유다. Camera는 1,336,647 bytes, 한도 2MiB / 128 shot / shot당 64 key다. World template당 32 track / 256 key, 전체 256 template 한도를 현재 reader로 검사한다. 불필요한 동일 정적 키를 중복 생성하지 말고 기존 잘 맞는 정의를 재사용한다. 한도를 넘으면 새 C++ 상한 증가부터 하지 않고 이번 후보의 샘플링·분할·공유를 조정한다. 다른 팀의 기존 row를 지워 공간을 확보하지 않는다.

이 단계는 저작 데이터와 오프라인 모델 준비를 우선하므로 기본적으로 C++/셰이더 재빌드를 요구하지 않는다. 실제 runtime에 없는 실행 의미가 필요한 것이 확인될 때만 호출자/실패 소비자를 좁혀 별도 변경을 제안한다. 기존 A/B/C를 오프라인으로 해석할 문제를 새 런타임 animation engine으로 우회하지 않는다.

### G12-10. 사용자가 편집하는 위치

`KoukuSaydon → F1 Developer Tools → Sequencer Benchmark → Composition Sequencer`에서 해당 Gate와 Sequence를 선택한다. P3/P7는 기존 입장 경로, 신규 엔딩은 독립 관찰 경로다. 전체 카메라와 WORLD가 같은 타임라인에 나타나야 한다.

1. 전체 구간: `Play Sequence`, `Pause`, 시간축 Seek로 본다.
2. 박스 시간·위치: 해당 WORLD/CAMERA box를 선택해 `Box Detail`을 편집한다.
3. 배우/맵 모션: WORLD의 `Edit This Motion` / `Edit Object`로 Object Detail·Object Sequencer를 연다.
4. 카메라 키: CAMERA의 `Open Composition Camera` → key 편집 → `Save Camera`를 사용한다. `Set Camera Pos / Capture view`는 기존 track을 정지 pose로 교체하므로 전체 원본 궤적을 보존할 작업에서 누르지 않는다.
5. Composition `Save`, Object/Map 저장, Camera 저장은 각 파일의 별도 저장이다. Sequence workspace에서 `Publish All Patterns`는 지원하지 않으며 전투 Pattern 게시와 혼동하지 않는다.

새 배치가 포함되면 Map publisher의 Area scope로 catalog/placement/World/Camera를 같은 검증 범위에서 게시한다. Object-only WorldSequences scope는 카메라/새 맵 배치를 게시하지 않는다. runtime DataFiles를 직접 편집하지 않는다. 게시 시 팀장 문서가 자동 재작성되지 않는지 diff를 확인하고, 저장/게시가 진행 중이면 먼저 완료된 뒤 baseline을 잡는다.

### G12-11. 진행 순서와 확인 기준

**순서: 원본 연결표 → P3 보완 → P7 전체 구성 → 빙고 엔딩 → 사용자 수동 배치 보정.** 세 개를 한 번에 전부 새로 생성하지 않는다. 각 Sequence의 후보를 따로 검사하고 넘어간다.

자동 검사:

- 수정 JSON parse 및 해당 Camera/World reader/publisher 통과, stable ID/모델/clip/placement 참조 누락 없음.
- 카메라 key 시간 증가·finite pose·up/roll·FOV, hard cut과 연속 segment 구분, 원본 cut 경계 보존.
- source time/clip trim/reverse/loop/슬롯 평가와 root/parent 중복 변환 없음. SHOW 시점 이전 animation 진행 상태 유지.
- 배우 교체 30,933ms와 최종 숨김 41,067ms의 직전/해당/직후 상태, 0ms preroll·Stop·재시작·역방향 Seek.
- 공용 배우/배치의 baseline 복구와 종료 수명, source model/기존 P1/P2/P4/P5/P6 불변.
- 조명/암전/재질/RenderingProfile 원본 bytes 보존, 후보 크기·키 개수 한도, `git diff --check`.

사용자 확인:

- P3: 큰/작은 배우·책 위치, 손 접촉, 카드 무대 펼침과 마지막까지 유지.
- P7: 출발·비행이 실제 포함되는지, 원본 천막 도착과 전경/커튼이 이어지는지, 기존 입장 완료 흐름이 유지되는지.
- 엔딩: 바닥에 눕는 자세, 무기·작은 쿠크 부착, 배우 교체에 중복/깜빡임이 없는지, 숨김/빈 바닥과 카메라 복귀.
- 모든 장면: 작업 화면의 프레이밍, 외곽 배경 노출, 보정할 세트 위치를 사용자 판단으로 확정.

전등·암전·재질·FX shader 미복구로 인한 명암/분위기 차이는 이번 기하·동작 구현의 미완료와 구분한다. 그렇다고 배우가 겹치거나 카메라가 다른 위치를 보는 문제를 팀장 담당으로 넘기지 않는다.

최종 인계는 세 Sequence 이름/stable ID, 변경 정본, 새 Resources 상대 ID와 물리 위치, 자동 검사 결과, 사용자 수동 보정값, 팀장 후속 목록으로 한다. 빌드·실제 Client 재생·사용자 시각 승인은 수행한 경우에만 RESULT에 기록한다.

---

## G13. 2026-09-13 녹화 비교 후속 계획 — 2관문·3관문 진입의 카메라와 맵 애니메이션

### G13-01. 이번에 할 일과 현재 결론

**2관문은 빠진 무대 소품의 연결·동작을 먼저 보완하고, 두 관문 모두 원작과 다른 카메라 구도를 장면별로 바로잡는다.** 기존 G12 전체를 다시 생성하는 작업이 아니다. 현재의 P3 `2관문_진입`, P7 `3관문_진입`만 대상으로 한다. 빙고 최종 엔딩, 배우 동작 재제작, 조명, 암전, 재질 복구, 비행 터널·카드·불꽃 이펙트는 이번 수정 대상에서 제외한다.

이 절은 **조사와 수정 계획**이다. 이번 조사에서 C++·연출 JSON·Resources는 수정하지 않았고 Client를 실행하지 않았다. 첨부 녹화의 화면 차이는 확인했지만 수정 결과에 대한 사용자 시각 승인은 아직 없다.

| 우선순위 | 확인한 문제 | 수정 방향 |
|---|---|---|
| 1 | 2관문 책 무대가 펼쳐지는 장면의 주변 구조·촛대 구성이 원작과 다름. 현재 P3에는 별도 backdrop 165개를 연결한 결과가 없음 | 원본 부모에 붙은 소품을 기존 추출 경로로 연결하고, 기존 의자·촛대의 위치·표시·부모 동작도 함께 점검 |
| 2 | 2관문 쿠크 클로즈업에서 원작은 얼굴, 프레임워크는 등이 보임 | 같은 원본 시각에서 camera eye/lookAt/up, 배우 방향·동작 시각을 대조한 뒤 틀린 변환만 수정. 임의 180도 회전부터 하지 않음 |
| 3 | 3관문 시작과 손 위 쿠크/세이튼 클로즈업의 구도가 다름. 몸통·장식에 얼굴이 가림 | c1 계열과 c4_1 카메라의 부모 합성·시선·FOV·배우와의 상대 위치를 우선 수정 |
| 4 | 3관문 도착과 후퇴 장면에서 바닥·커튼·전경이 원작과 다른 비율로 화면을 차지함 | 카메라와 현재 세트의 기준 위치를 대조하고, 확인된 배치·표시 오류만 보정 |

검은 배경을 전부 같은 원인으로 보지 않는다. 2관문 도입부는 원래 전투 무대의 표시/전환을 확인해야 하지만, 3관문 비행 중 보라색 터널은 원본상 이펙트다. 후자는 맵 애니메이션을 추가해서 해결할 항목이 아니다.

### G13-02. 비교 자료와 시간 읽는 방법

영상 물리 폴더: `C:/Users/USER/OneDrive/바탕 화면/`.

| 첨부 파일 | 구분 | 측정 길이 | 인코딩 크기 / FPS |
|---|---|---:|---|
| `2관문 컷신 .mp4` | 실제 게임 참고 | 27.33초 | 2160×1440 / 30 |
| `2관문 진입(프레임워크1).mp4` | 현재 작업본 | 29.83초 | 1282×772 / 30 |
| `3관문 컷신 .mp4` | 실제 게임 참고 | 36.47초 | 2160×1440 / 30 |
| `3관문 진입(프레임워크1).mp4` | 현재 작업본 | 36.37초 | 1282×772 / 30 |

영상의 앞뒤 gameplay, 창 제목·HUD·검은 여백이 서로 다르다. 특히 2관문 참고 영상 끝에는 다음 장면이 포함된다. 따라서 길이 차이를 곧바로 재생 속도 오류로 보거나 영상의 같은 초를 동일 애니메이션 프레임으로 비교하지 않는다. 인코딩 해상도 비율도 실제 카메라 viewport 비율이 아니다.

오프라인으로 전체 구간의 2초 간격 접촉 시트를 열람하고, 주요 장면을 별도 프레임으로 대조했다. 아래 비교판은 **장면별 차이 설명용**이며 정확히 동기화한 동일 포즈 비교는 아니다. 다음 제작 단계에서 첫 컷·책 펼침·클로즈업·도착·커튼 후퇴를 기준으로 각 녹화 시각을 원본 Matinee 시각에 맞춘다. 자동 scene-change 검출값은 후보일 뿐 Director cut 정답으로 쓰지 않는다.

- [2관문 주요 장면 비교판](C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutsceneCompare20260913/gate2_comparison.jpg)
- [3관문 주요 장면 비교판](C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutsceneCompare20260913/gate3_comparison.jpg)
- [영상 메타데이터](C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutsceneCompare20260913/video_metadata.json)
- [현재 Composition/World/Camera 연결 조사](C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutsceneCompare20260913/current_data.json)

위 비교판과 조사 JSON은 이 PC의 분석 산출물이며 runtime 입력이나 팀 공유 Resources가 아니다.

### G13-03. 원본 데이터와 현재 저장 상태

2관문 원본은 바탕 화면의 `쿠크1관문_연출_원본_20260913/01_컷신별_타임라인/1관문클리어_221_SCENE04A_matinee2_전체.json`에서 확인했다. 이름은 1관문 클리어지만 이번 P3의 2관문 진입 연출에 대응한다. 3관문은 실제 저장소의 `out/KoukuSourceSequenceRestore20260912/LV_LUT_MIDNIGHTC_ED_SCENE02A.json` 추출 데이터와 설치된 원본 UPK를 확인했다. 바탕 화면의 1관문 폴더에 3관문 원본까지 모두 있다고 가정하지 않는다.

| 대상 | 원본 | 현재 저작 상태 | 해석 |
|---|---|---|---|
| P3 `KAKULSAYDON_G1_PATTERN_3` | SCENE04A, Matinee 329 / Data 394, 27,000ms | CAMERA 7개, WORLD 17개 | 원본 Director 5컷을 키 제한에 맞춰 나눈 것. 7컷이므로 원작보다 컷이 많다는 결론은 틀림 |
| P7 `KAKULSAYDON_G1_PATTERN_7` | SCENE02A, Matinee 63 / Data 117, 35,368ms | CAMERA 18개, WORLD 3개 | 현재는 출발·비행까지 이미 들어 있음. 예전의 뒤쪽 18.7초 버전으로 되돌리면 안 됨 |

조사 시점 revision은 Composition 17, Camera 84, World 1736이다. 이후 사용자 Save가 있으면 이 숫자와 원본 파일을 다시 읽고 후보를 만든다.

원본에서 활성화된 Director만 사용한다. P3는 cam1 → 2.10초 cam2 → 13.95초 cam3 → 19.49초 cam4 → 23.95초 cam6다. P7는 13개 원본 컷이며 c6 등 긴 구간이 나뉘어 현재 18개 CAMERA가 됐다. 비활성 Director의 c5 등을 누락 컷으로 오인해 추가하지 않는다.

기존 `build_gate2_intro_composition.py`에는 이미 수평 FOV를 16:9 기준 수직 FOV로 바꾸는 처리와 camera parent/roll 처리가 있다. 따라서 “FOV 변환이 없어서 틀렸다”는 진단은 하지 않는다. 원본에는 `FOVAngle`과 `AbsoluteFOVAngle`이 함께 있는 카메라도 있어, 활성 트랙의 의미와 실제 소비값을 대조할 필요는 있다. 이 차이가 현재 오류의 확정 원인이라는 뜻은 아니다.

### G13-04. 2관문 장면별 수정

아래 시간은 녹화 시각이 아니라 **P3 시퀀스 시각**이다.

| 시각 / 카메라 | 원작에서 보여야 하는 것 | 현재 관찰 / 확인 | 작업 내용 |
|---|---|---|---|
| 0–2.10초 / camera.1 | 기존 무대 위 배우와 주변 공간을 소개 | 작업 녹화 도입에서 빈 검은 공간과 매달린 소품이 보임 | 시퀀스 시작 전후 현재 전투 맵의 표시 상태, 카메라 기준 위치, 무대 전환 시각부터 확인. 이 구간을 165개 책 소품만으로 해결한다고 단정하지 않음 |
| 2.10–13.95초 / camera.2 | 세이튼·쿠크·책을 한 동작으로 연결 | 큰 흐름은 있지만 책과 배우의 방향·화면 점유가 다름 | 동일 동작 시각에서 손·책·얼굴의 상대 좌표와 카메라를 대조. 배우가 잘못 향한 경우 카메라 전체 이동으로 숨기지 않음 |
| 13.95–19.49초 / camera.3 | 책과 테이블이 펼쳐지고 주변 구조가 따라 움직임 | 펼침 자체는 보임. 원본의 주변 구조와 촛대 구성은 부족하거나 위치가 다름 | 기존 backdrop builder의 165개 부모 부착 소품을 후보에 연결. 이미 존재하는 book/table/chair/candle WORLD도 부모·Transform·visibility를 확인 |
| 19.49–23.95초 / camera.4–6 | 쿠크 얼굴 클로즈업에서 무대를 드러내는 후퇴 | 비교 장면에서 쿠크 등이 보이고 후퇴 구도의 크기도 다름 | 원본 cam4 하나의 연속 궤적으로 평가. 배우 시각/방향과 카메라 부모 변환을 비교한 뒤 eye/lookAt/up/FOV를 보정. 21.714·22.818초의 내부 분할에서 새 컷·카메라 복귀가 생기지 않게 함 |
| 23.95–27.00초 / camera.7 | 카드 무대와 배우의 마무리 | 무대는 나타나지만 주변 세트와 카드 양이 다름 | 맵 소품은 마지막 pose까지 유지. 카드는 FX·배우 항목과 구분하고 이번에는 양을 늘리지 않음. 종료 뒤 gameplay 복귀를 컷신 중 누락으로 오인하지 않음 |

165는 아무 소품이나 추가할 개수가 아니다. 원본의 static mesh actor 172개 중 직접 Matinee에 연결된 7개와 별도로, 부모를 통해 따라가는 165개의 원본 identity를 대상으로 한다. 현재 P3 WORLD 17개에는 의자 4개·촛대 2개 정의가 이미 있다. **촛대가 영상에서 안 보인다고 촛대 데이터 자체가 없다고 판단해 중복 생성하지 않는다.** 원본 부모·자식과 현재 stable ID를 대응시켜 중복과 누락을 동시에 검사한다.

기존 `build_gate2_intro_backdrops.py`를 재사용하되 source parent의 첫 키 기준 변환, Base/RelativeLocation/Rotation, cm→m를 각각 한 번만 적용한다. 의자·기둥 등이 책과 따로 떠 있지 않은지, 펼침 종료 뒤 제자리로 돌아오지 않는지 확인한다. 프로그램 파형으로 재구성한 순환 흔들림은 원본 키 직접 복사와 구분하고 위상·진폭을 검증한다.

### G13-05. 3관문 장면별 수정

아래 시간은 **P7 시퀀스 시각**이다.

| 시각 / 원본 카메라 | 원작에서 보여야 하는 것 | 현재 관찰 / 확인 | 작업 내용 |
|---|---|---|---|
| 0–6.599초 / c1 계열 | 작은 쿠크를 중심으로 시작한 뒤 출발 액션으로 연결 | 첫 구간에서 큰 세이튼 몸이 좌측을 차지하고 작은 쿠크가 멀리 보이는 등 주인공 구도가 다름 | camera.1–6의 카메라 선택·부모 좌표·시선·FOV를 배우 동작 시각과 함께 대조. 원작의 얼굴/전신 전환을 보존 |
| 6.599–16.710초 / c2, c3, c3_a1 | 출발·비행하는 배우를 카메라가 따라감 | 비행 동작은 있지만 배경이 검음그리고쿠크와세이튼이둘다전체적으로보여야하는데그게아니라쿠크만너무확대된듯하게보임                | camera.7–10의 추적 위치·거리·회전 연속성을 수정 대상에 포함. 보라색 터널은 별도 FX 담당 목록으로만 인계 |
| 16.710–22.070초 / c4, c4_a1 | 천막 도착, 바닥·배우·관중 방향을 읽을 수 있는 구도 | 도착 장면은 있으나 커튼 장식과 전경이 차지하는 비율이 다름 | camera.11–12의 hard-attach 부모와 실제 세트 기준 좌표를 대조. 확인된 세트 위치·표시 오류만 보정. 원작과 다른 밝기·불 재질은 이번에 수정하지 않음 |
| 22.070–26.420초 / c4_1 | 손 위 쿠크와 세이튼 얼굴을 가까이 잡음 | 비교 프레임에서는 얼굴 대신 손·몸통·장식이 크게 보임 | camera.13의 eye/lookAt/up을 원본과 현재 배우 좌표에 대조. 가장 먼저 교정할 근접 구간. 단순 FOV 확대만으로 얼굴을 억지로 넣지 않음 |
| 26.420–35.368초 / c6 | 넓은 무대에서 커튼 바깥으로 빠져나감 | 후퇴 자체는 이어지지만 커튼·바닥·무대 점유 비율이 다름 | camera.14–18을 원본 c6의 연속 이동으로 검증. c6_p 부모와 끝 위치를 보존하며 분할 경계의 속도/방향 튐을 제거. 종료 복귀 시각은 원본 동작 종료와 분리 |

SCENE02A 추출에서 직접 열거되는 static mesh actor는 0개다. 따라서 3관문에 2관문처럼 수백 개의 새 맵 애니메이션이 빠졌다고 주장할 근거는 없다. 현재 세트의 배치·표시, 카메라 부착 부모, 배우와 카메라의 좌표 대응을 먼저 확인한다. 별도 맵 sublevel이나 다른 자산의 움직임이 실제 연결된 것이 확인될 때만 해당 원본을 추가 대상으로 삼는다.

P7 WORLD 3개는 기존 P5와 공유하는 `.world.28`, `.world.29`, `.world.30`이다. P7 얼굴 구도를 고치려고 공용 배우 방향·크기를 일괄 변경해 P5를 망가뜨리지 않는다. 카메라가 원본과 맞는데 배우 자체의 bake/포즈가 다르다면 그 증거를 남기고 배우 수정 범위를 별도로 제안한다. 이번 단계에서 배우 전체를 재생성하지 않는다.

### G13-06. 실제 수정 순서와 파일 범위

1. **같은 시각과 좌표로 맞춰 비교한다.** 녹화별 컷신 시작/끝과 active viewport를 맞추고, 원본 Director 시각의 actor/parent/camera 값 → 생성된 JSON → runtime에서 평가한 값의 연결을 검사한다. 스냅샷의 전역 시간과 box-local 시간, 첫 키 기준 상대 이동, 부모 변환, 모델 preScale을 구분한다. 소스가 맞고 runtime 소비가 다를 때만 소비 경로를 수정 대상으로 좁힌다.
2. **2관문 무대 후보를 먼저 만든다.** 기존 17개 WORLD와 배우 모델은 보존하고 165개 연결의 누락을 보완한다. 정적 배치와 실제 움직이는 부모 부착 소품을 구분한다. 원본 키가 있는 동작은 원본대로, 원본 정보가 없는 보충 배치는 수동 보정 목록으로 분리한다.
3. **두 관문의 카메라를 고친다.** P3 cam4 얼굴→후퇴, P7 c1 도입과 c4_1 얼굴 구도를 우선한다. 그다음 도착/퇴장 구도를 조정한다. 원본 cut 시각은 유지하고 단순 key 분할을 새 연출 컷으로 바꾸지 않는다. 원본 좌표 변환 오류를 수정한 뒤에도 현재 세트의 배치 차이가 남을 때만 전용 보정값을 적용한다.
4. **저장·재로드 가능한 후보만 적용한다.** 생성 결과와 기존 사용자 값을 비교하고, 외부 Save가 없었는지 baseline을 다시 확인한다. P3/P7만 교체하며 P4 팝업북, P5 공유 배우, 빙고 엔딩과 팀장 담당 데이터는 보존한다.
5. **사용자가 같은 경로에서 확인한다.** 전체 재생, 문제 구간 Seek, Stop/Restore, 재시작을 확인한 뒤 수동 배치 보정값을 확정한다. 이번 계획 단계에서는 재생 결과를 PASS로 기록하지 않는다.

| 파일 | 이번 구현 단계의 변경 범위 |
|---|---|
| `Tools/KoukuSaydonPipeline/build_gate2_intro_backdrops.py` | 원본 부착 소품 후보·부모/키/크기 검증. 기존 source matching과 material 연결 재사용 |
| `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py` / `build_gate_cutscenes_g12.py` | 확인된 camera/parent 변환 오류만 수정. P3/P7 camera/map-only 후보 생성 경로 사용 |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | P3의 누락 WORLD 연결, 필요한 P3/P7 CAMERA 연결만 변경 |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | P3 소품 motion/visibility/수명. 공용 배우·타 컷신 유지 |
| 같은 폴더의 `.camerashots.json` | `kouku.gate2.intro.camera.*`, `kouku.gate3.intro.camera.*` 중 확인된 수정만 적용 |
| 같은 폴더의 `.mapplacements` / catalog | 원본과 현재 세트 차이로 입증된 누락 배치만. 무조건 새 맵을 다시 배치하지 않음 |

전체 builder의 `--install`을 재실행하면 배우 bake·암전·이펙트까지 섞일 수 있으므로 사용하지 않는다. 기존 SourceSequences 모델과 Resources 상대 asset ID를 우선 재사용한다. 새로운 물리 asset이 필요하면 실제 상대 ID·폴더·원본 identity를 결과에 따로 기록한다. 이번 계획에서 새 C++ 파일·셰이더·프로젝트 등록은 제안하지 않는다.

### G13-07. 파일 크기 때문에 다시 막히지 않게 할 것

현재 World 문서는 14,387,090 bytes로 reader 한도 16MiB에 가깝다. 이전 G12 결과의 backdrop 후보 추정치는 약 3.5MB이므로 그대로 더하는 방식은 안전하지 않다. 그 추정 후보를 이번 조사에서 다시 생성한 것은 아니다.

같은 현재 JSON을 값 변경 없이 compact 직렬화했을 때 메모리상 5,302,425 bytes였다. 그러나 `WorldSequenceDocument.cpp`의 실제 Save는 들여쓰기를 넣어 다시 출력한다. **설치할 때만 minify해서 통과시키는 것은 해결책이 아니다.** 사용자가 Save하면 재확장될 수 있다.

우선 **이번에 추가하는 P3 backdrop 후보만** 동일 정적 키를 줄이고 부모별 그룹·분할을 최적화한 후, 기존 데이터와 합쳐 툴 Save 형식으로도 16MiB 아래인지 검사한다. 기존 World 문서 전체나 다른 컷신의 키를 최적화 대상으로 삼지 않는다. curve 허용 오차를 몰래 늘리거나 다른 컷신을 삭제해 크기를 맞추지 않는다. 이 방식으로 충분하지 않으면 compact 저장을 툴 Save까지 일관되게 지원하는 별도 최소 C++ 변경이 필요하다고 보고한다. 그 경우에만 해당 함수의 구현 계획과 C++ 증분 빌드를 추가하며, 근거 없이 reader 한도만 올리지 않는다.

따라서 카메라·배치 데이터 수정 자체는 C++/셰이더 재빌드 대상이 아니다. 저장기나 runtime 평가의 실제 오류가 확인된 경우에만 C++ 빌드가 추가된다. 리소스 전체 복사·셰이더 재컴파일을 이 작업의 기본 절차로 삼지 않는다.

### G13-08. 검증과 완료 기준

자동 검증은 다음 구현 때 수행한다. 이번 조사에서 실행한 것은 영상 메타데이터/프레임 비교, source JSON·현재 저작 연결·기존 코드 읽기뿐이다.

- P3/P7 원본 활성 Director cut과 전체 길이 보존. camera key 시간 증가, finite eye/lookAt/up/FOV, 64-key 제한과 연속 segment 경계 검사. 현재 generator의 위치 5mm·회전 0.1도·FOV 0.05도 축약 기준을 무단 완화하지 않는다.
- 165개 대상 원본 identity가 후보의 부모/mesh/stable ID에 연결되며 기존 직접 연결 7개와 중복되지 않는지 검사. 원본 key 시각과 경계 직전/직후를 평가해 순간이동·중복 변환·동작 재시작이 없는지 확인한다.
- 실제 툴 Save 형식의 크기, reader 한도, World track/key/template 제한, Camera shot 제한을 검사. Save→Reload 후 값·참조·크기가 유효해야 한다.
- Map/Camera/World는 `Publish-MapAuthoring.ps1`의 해당 Area `Validate`와 적용 후 `Check`로 검사한다. 카메라와 새 맵 배치를 Object-only WorldSequences 게시로 대신하지 않는다. runtime DataFiles를 직접 편집하지 않는다.
- Composition은 `KoukuSaydonCompositionDocument.cpp`의 `KoukuSaydonSequenceComposition.json` reader가 요구하는 schema/identity와 WORLD/CAMERA 참조, occurrence ID 유일성·구간을 검사하는 후보 검증을 생성기에 보강한다. 기존 G12 후보 assertion만으로 schema 전체를 검증했다고 하지 않는다. 범용 `Publish-Compositions.ps1`은 별도 `KoukuSaydonArena.sequencer.json` 경로를 소비하므로 이번 파일 검증의 대체 수단이 아니다. 전투 패턴 Publish와 Server 반영은 하지 않는다.
- 변경한 생성기 focused test, JSON parse, `git diff --check`. P3/P7 이외 연출 및 조명·암전·재질·FX 문서 보존을 비교한다. C++ 변경이 없으면 불필요한 전체 빌드 없이 데이터 검증 결과를 기록한다.

사용자 화면 확인은 `F1 → Sequencer Benchmark → Composition Sequences`에서 `2관문_진입` 또는 `3관문_진입`을 선택하고 `Composition Sequencer`로 재생한다. 최종 수정본의 실제 UI 명칭도 인계 시 확인한다.

| 대상 | 사용자 확인 기준 |
|---|---|
| 2관문 | 도입 무대가 갑자기 빈 공간으로 바뀌지 않음. 책·테이블·의자·기둥·촛대가 같은 세트로 펼쳐지고 마지막까지 유지. 얼굴 클로즈업과 후퇴가 원작의 장면 의도와 일치 |
| 3관문 | 시작 쿠크 구도, 출발·비행 추적, 도착 바닥/배우, 얼굴 클로즈업, 커튼 후퇴가 이어짐. 몸통/장식 때문에 주인공 얼굴이 의도치 않게 가려지지 않음 |
| 공통 | 0초부터 재생·중간 Seek·Stop/Restore·재시작 후 동일 결과. 수정값 Save/Reload 유지. 공용 P5와 팝업북 등 다른 연출에 부작용 없음 |

비행 터널, 카드 양, 불빛, 암전, 재질의 차이는 별도 후속 목록에 남긴다. 이 항목들이 달라도 이번 카메라/기하 검증은 따로 진행할 수 있지만, 그것만으로 원작 전체 연출이 완성됐다고 보고하지 않는다.

### G13-09. 추가 설명 — 무엇을 갖춰야 원작에 맞췄다고 할 수 있는가

이번 추가 요청은 **2관문·3관문 진입의 카메라와 맵 연출을 원작에 맞추기 위한 설명·검증 절차 보강**이다. 구현이나 작업 범위 확대가 아니다. 아래 기준은 앞으로 적용할 제안 기준이며, 현재 데이터가 이 기준을 통과했다는 뜻이 아니다.

“원본 데이터를 넣었다”, “빌드가 됐다”, “주요 화면 한 장이 비슷하다”만으로 완료하지 않는다. 다음 세 층을 각각 확인한다.

| 확인 층 | 확인할 것 | 이 단계만 통과해서는 알 수 없는 것 |
|---|---|---|
| 원본 의미 | 활성 컷과 시간, 카메라·소품의 부모 관계, 좌표·회전·스케일·표시 키를 빠짐없이 해석 | 게임에서 실제로 같은 값이 쓰이는지 |
| 실제 재생 | 생성/저장/로드된 값이 같은 시퀀스 시각에 평가되고 다른 카메라나 시스템이 덮어쓰지 않는지 | 원작 배우/세트 차이까지 포함한 최종 화면 구도 |
| 원작 화면 대조 | 같은 동작 시점에서 인물 크기·방향, 소품 위치, 이동·전환이 일치하는지 | 제외한 조명·재질·이펙트까지의 완전한 화면 일치 |

완료 표현은 **“P3/P7 카메라·맵 연출이 명시한 검증 기준을 통과했고 사용자가 확인했다”**로 한정한다. 유한한 샘플과 압축된 30fps 참고 녹화로 모든 순간의 오차 0을 증명할 수는 없다. 아래 허용 오차를 충족하는 것과 수학적·픽셀 단위의 완전 동일은 구분한다. 미확인 장면을 제외하고 전체가 완벽하다고 보고하지 않는다.

### G13-10. 기준 시각과 비교 화면부터 고정한다

1. 기준 연출은 기존 G13-03의 SCENE04A/P3와 SCENE02A/P7로 고정한다. 원본 export/group/track → 현재 resource/occurrence/shot ID를 대응시킨다. 다른 Matinee의 비활성 Director, 비슷한 이름의 카메라를 섞지 않는다.
2. 각 녹화의 실제 영상 영역과 앞뒤 잘린 구간을 기록한다. 창 제목·유튜브 UI·HUD·letterbox는 비교 대상에서 분리하지만, 화면의 불일치를 숨기기 위한 임의 크롭은 하지 않는다. 실제 viewport 비율을 맞추고 종횡비를 찌그러뜨려 비교하지 않는다.
3. 첫 컷, 책이 펴지는 순간, 얼굴 클로즈업 진입, 천막 도착, 후퇴 시작처럼 식별 가능한 사건을 이용해 `원본 시각 ↔ 참고 녹화 시각 ↔ 작업 녹화 시각`을 대응시킨다. 영상 전체 길이를 나눠 일괄 배속 보정하지 않는다. 편집·중복 프레임·누락 구간이 있으면 해당 구간을 따로 표시한다.
4. 데이터 시간은 원본 실수 초와 프로젝트 ms를 모두 남긴다. 화면 비교는 녹화 프레임의 PTS를 사용한다. 30fps 한 프레임은 약 33.3ms이므로 녹화만으로 1ms 정확도를 주장하지 않는다. 명확한 사건에서도 한 프레임 이상 대응이 불확실하면 시간 불확실부터 해소하고 구도 오차를 측정한다.
5. 원본 데이터와 참고 영상의 사건 순서·배치가 일치하지 않는다면 다른 버전/연출 또는 잘린 영상인지 확인한다. 양쪽을 동시에 정답으로 놓고 임의 보정하지 않는다. 같은 원본 버전임을 확인할 수 없으면 그 장면의 기준 선택을 사용자에게 요청하고 해당 항목을 미확인으로 유지한다.

카메라 검증에는 맵의 기준점과 배우를 구분한다. 무대 모서리·책 힌지·촛대 밑면 등 고정된 지점을 먼저 맞추고, 얼굴·손처럼 동작하는 지점은 같은 clip 시각과 pose인지 확인한 뒤 쓴다. 배우 자세가 다른데 얼굴 위치만 따라 카메라를 움직이면 다른 소품의 구도가 틀어질 수 있다.

### G13-11. 원본 → 후보 → 저장본 → 실제 재생을 한 줄로 추적한다

각 문제 장면에 다음 증거를 묶는다. 문서에 값이 존재하는 것과 runtime에 반영된 것은 별개다.

| 단계 | 남길 정보 | 확인 목적 |
|---|---|---|
| 원본 | package/export/group/track, 활성 상태, 원본 시각, 부모·부착 bone, 원시 키와 보간 방식 | 다른 트랙 선택·빠진 부모·상대/절대 좌표 오해 방지 |
| 후보 | 대응 stable ID, box 시작/속도, 계산한 local 시각, world transform, camera eye/forward/up/FOV | 시간·단위·좌표 변환이 어디서 달라졌는지 찾기 |
| 저장/로드 | 실제 소비 파일 경로·revision, 로드된 해당 값, instance/shot ID | 다른 파일 편집·오래된 게시본·Save 시 손실 구분 |
| 최종 재생 | 시퀀스 clock, 활성 카메라 소유자와 shot, 최종 view/projection, 소품 최종 transform/visible 상태 | gameplay 카메라·부모 갱신·visibility 등이 뒤에서 덮어쓰는지 확인 |

기존 로그로 마지막 정보를 얻을 수 없다면 필요한 필드만 좁혀 진단 출력을 추가하는 구현 계획을 별도로 작성한다. 이 설명서를 쓰면서 진단 C++이나 두 번째 카메라 런타임을 생성하지 않는다. 로그가 없는데 최종 재생 값까지 검증했다고 기록하지 않는다. 사용자가 직접 Client를 조작하고, 에이전트는 허용된 로그와 사용자가 제공한 녹화를 분석한다.

생성기 함수를 그대로 다시 호출해 “기대값”과 “실제값”을 만들면 같은 오류가 양쪽에 남을 수 있다. 원시 키의 경계값, 독립적인 부모/좌표 계산과 정적인 맵 기준점의 화면 투영을 교차 확인한다. `world_pose`·`make_cameras`의 결과끼리만 같다고 원작 의미가 검증됐다고 하지 않는다. 키 사이 보간, 원본 tangent/constant 동작과 회전 경로도 대조한다.

### G13-12. 카메라를 맞추는 순서 — 줌 하나로 모든 문제를 덮지 않는다

검사 순서는 **시각·활성 shot → 좌표/부모 → 회전과 시선 → 투영 → 보간과 이동 속도 → 현재 세트에 필요한 보정**이다.

- **시각·소유자:** Director cut과 Composition box-local clock을 먼저 맞춘다. 하나의 장면을 key 제한 때문에 나눈 경계에서 gameplay 카메라로 돌아가거나 blend가 다시 시작되지 않아야 한다. 실제 원본 hard cut의 위치 변화는 오류가 아니다.
- **좌표·부모:** actor 배치, 부모 이동, 부착 bone, 첫 키 기준 상대 이동과 UE→프로젝트 단위 변환을 확인한다. 원본 소품과 카메라를 같은 좌표계로 비교한다. 세트를 통째로 옮겨야 한다면 공통 rigid transform을 한 번 적용하고, camera eye와 lookAt에는 이동·회전, up에는 회전만 적용한다.
- **방향·시선:** eye가 맞아도 forward/up이나 부모 회전이 틀리면 몸통·등이 보인다. roll을 빼거나 lookAt을 임의 고정점으로 바꾸지 않는다. 회전각 표기가 0/360도에서 바뀌는 것과 실제 자세가 도는 것을 구분한다.
- **투영:** 원본 FOV 트랙의 의미, 현재 생성기의 수평→수직 변환, 실제 viewport aspect와 최종 projection을 한 번씩 확인한다. 시선은 맞지만 화면 점유율이 다를 때에만 거리와 FOV를 분리해 조정한다. near clipping으로 모델이 잘리는 경우와 프레임 밖으로 나가는 경우도 구분한다.
- **움직임:** 시작/끝뿐 아니라 구간 내부의 eye·시선·FOV 곡선을 비교한다. 가속·감속과 빠른 후퇴의 타이밍, 카메라 흔들림이 원본 curve인지 별도 트랙인지 확인한다. 원본에 없는 부드러운 보간을 임의로 넣지 않는다.

**3관문 비행 구간(camera.7–10)은 필수 보완 대상이다.** 사용자가 추가한 “쿠크만 확대돼 보인다”는 관찰을 반영해, 참고 영상에서 쿠크와 세이튼이 함께 잡히는 시점에는 두 배우의 위치·화면 점유율·잘림을 각각 검사한다. 모든 비행 컷을 무조건 전신 구도로 바꾸는 것이 아니라, 대응 원작 컷이 보여 주는 두 인물의 범위를 맞춘다. 비행 터널 FX가 없어도 이 인물 구도 검증은 수행한다.

데이터가 원본과 일치하는데 화면이 다르면 배우/세트 기준 위치·모델 preScale·실제 pose를 먼저 대조한다. 모델이 다르게 구워진 문제를 카메라 보정만으로 숨기지 않는다. 배우 수정이 꼭 필요하면 그 구도 항목은 미완료로 남기고 범위 승인을 받는다.

### G13-13. 맵 소품을 맞추는 순서 — 개수뿐 아니라 붙는 곳과 움직임을 확인한다

2관문 165개 후보는 source actor 하나마다 대응 관계를 남긴다. 여러 source actor를 한 runtime 그룹으로 묶더라도 각각의 원본 identity와 transform을 검증할 수 있어야 한다. 전체 WORLD 숫자가 늘었다는 이유만으로 연결 완료로 판정하지 않는다.

1. **구성:** 원본 mesh identity, 부모, 현재 stable ID, 배치 여부를 대응한다. 기존 직접 연결 소품과 중복 생성하지 않는다. 참조가 풀리지 않는 mesh는 비슷한 다른 mesh로 자동 대체하지 않는다.
2. **배치:** 책 힌지와 테이블 모서리, 의자·촛대 밑면 등으로 원본의 pivot·상대 거리·방향·크기를 확인한다. 모델 geometry preScale과 placement/key scale을 구분하고 local offset에 scale을 두 번 적용하지 않는다.
3. **동작:** 각 소품의 시작/중간/끝 pose와 부모 부착을 확인한다. 먼저 부모 원본 곡선, 다음 자식 local 변환 순으로 검사해 책이 펴질 때 기둥·장식만 공중에 남지 않도록 한다. 동일 key 사이에도 회전·가감속이 원본과 다르면 보간을 교정한다.
4. **표시/수명:** 생성·표시·숨김·motion 종료를 별개로 검사한다. 지연된 소품은 자신의 local clock으로 진행하되 전체 컷신과 동기화돼야 한다. 도중 재생성, 종료 시 시작 위치 복귀, 마지막 포즈 전에 사라짐은 허용하지 않는다. 원본의 의도된 숨김은 그대로 보존한다.
5. **재현:** 0초부터 연속 재생했을 때와 같은 시각으로 Seek했을 때 상태가 일치해야 한다. pause/resume, 역방향 Seek, Stop/Restore, 두 번째 재생에서도 누적 회전·중복 소품이 없어야 한다.

3관문은 현재 SCENE02A에 직접 static mesh actor가 없다는 조사만으로 맵 전체를 확인했다고 하지 않는다. 도착·퇴장 화면에 등장하는 현재 세트의 source sublevel/배치·표시 관계를 확인한다. 원본 움직임의 근거가 발견되면 대응시키되, 참고 영상의 카메라 이동으로 생긴 화면상 움직임을 소품 애니메이션으로 오인하지 않는다. 원본이 확보되지 않은 항목은 “수동 재현/사용자 승인”으로 분리하고 “원본 데이터 복원”이라고 부르지 않는다.

### G13-14. 수치·화면 검증 기준과 장면별 확인표

아래 수치는 **현재 값의 측정 결과가 아니라 작업용 제안 허용 오차**다. 기존 카메라 축약 기준보다 느슨하게 만들지 않고, 화면상의 명백한 결함은 수치를 통과해도 수정한다.

수치 검사는 두 갈래로 나눈다. **A: 원본 → 보정 전 후보**에서는 원본 변환의 정확성을 검사한다. **B: 승인 보정을 적용한 기대값 → 저장본·최종 재생**에서는 의도한 보정이 정확히 반영되는지 검사한다. 공통 세트 이동·회전은 동일 기준계로 환산한 뒤 비교한다. 아래 오차는 각 갈래의 기대값에 대해 적용하며, 의도적으로 변경한 위치·방향·FOV의 원본 대비 차이는 오차에 숨기지 않고 별도 기록한다. B가 통과해도 그 항목을 원본 수치 그대로의 복원이라고 부르지 않는다. 원작 화면 일치는 별도 화면 비교와 사용자 확인을 통과해야 하며, 원본 기준 궤적 변경의 허용 여부도 사용자가 결정한다.

| 검사 | 제안 기준 | 적용 주의 |
|---|---|---|
| 원본 cut/표시 사건의 데이터 시간 | 원본 초→정수 ms 변환 오차 1ms 이내, 사건 순서 동일 | 녹화 비교의 시간 정밀도와 혼동하지 않음 |
| 카메라 위치·방향·FOV | 각 검사 갈래의 같은 시각·좌표계 기대값 대비 위치 5mm, 실제 orientation 0.1도, FOV 0.05도 이내 | 보정 전 후보와 승인 보정 후 최종 평가를 구분. raw Euler 각 차이를 그대로 쓰지 않음 |
| 맵 소품 transform | 각 검사 갈래의 같은 시각 기대값 대비 위치 1mm, orientation 0.05도, 각 scale 성분 상대 오차 0.1% 이내 | scale이 0 근처이면 절대 오차와 원본 의미를 따로 기록. 최종 geometry/pivot 차이는 별도 검사 |
| 화면의 고정 기준점 | active viewport에 정규화한 X/Y 오차 각각 1% 이내를 1차 목표로 사용 | 프레임별 임의 이동/확대 정렬 금지. 가려진 점을 추측해 채우지 않음 |
| 주인공 화면 점유율 | 대응 pose의 너비/높이를 viewport 비율로 비교해 차이 각각 2%p 이내를 1차 목표로 사용 | 배우 silhouette/pose가 다르면 카메라 오차로 확정하지 않음. 원작에 없는 잘림·얼굴 가림은 별도 실패 |
| 누락·중복·재시작 | 대상 identity 누락/의도치 않은 중복/중도 원점 복귀 0건 | 원본의 의도된 표시 window와 카메라 cut은 실패에서 제외 |

데이터 비교는 모든 원본 key와 cut/표시/box 분할 경계의 직전·해당·직후를 포함한다. 구간 내부는 최소 60Hz 표본과 key 중간값을 검사하고, 빠른 이동·회전과 보간 오차가 큰 구간은 표본을 추가한다. 표본 검사가 연속 시간 전체의 완전 동일을 증명하지는 않는다. 위치·각도 평균만 남기지 말고 최댓값과 발생 시각/ID를 기록한다. 원본이 의도한 속도 변화와 달리 내부 분할에서만 이동이 끊기면 허용 오차 평균이 작아도 실패다.

화면 비교는 대표 프레임뿐 아니라 전체 재생을 함께 본다. 첨부된 참고·사용자 녹화에 대해 같은 시각의 나란히 보기/겹쳐 보기를 만들 수 있으나, 조명·FX 차이가 큰 전체 이미지의 픽셀 점수를 카메라 정답으로 사용하지 않는다. 사용자만 Client를 직접 실행·녹화하고 최종 시각 판정을 한다. 데이터-only 검사는 원본 화면이 가려진 장면의 시각 검증을 대신하지 않는다.

| 필수 장면 | 통과시켜야 할 내용 |
|---|---|
| P3 0–2.10초 | 원작과 같은 무대/배우 소개. 의도하지 않은 빈 공간 노출 없음 |
| P3 2.10–13.95초 | 배우·손·책의 상대 구도와 camera 진행. 동작 시각 대응 |
| P3 13.95–19.49초 | 책·무대 펼침, 소품 부착, 기둥·의자·촛대 배치/표시 |
| P3 19.49–23.95초 | 쿠크 얼굴 방향·크기와 연속 후퇴. 내부 분할에서 튐 없음 |
| P3 23.95–27.00초 | 무대/소품의 마지막 pose·표시와 올바른 카메라 종료 |
| P7 0–6.599초 | 작은 쿠크 도입부터 출발까지 각 컷의 주인공 구도 |
| P7 6.599–16.710초 | 원작 해당 컷에서 쿠크·세이튼 동시 구도, 추적 거리/줌/회전 |
| P7 16.710–22.070초 | 도착한 무대의 바닥·배우·커튼·전경 배치와 카메라 |
| P7 22.070–26.420초 | 손 위 쿠크와 세이튼 얼굴이 의도된 위치/크기로 보임 |
| P7 26.420–35.368초 | 넓은 무대→커튼 후퇴의 연속성과 종료 상태 |

이 표는 장면별 최소 목록이다. P3 원본 5컷과 P7 원본 13컷 모두에 세부 판정 행을 두고, 나뉜 CAMERA segment와 소품 identity의 검사 결과를 연결한다. 일부 좋은 프레임만 골라 전체 PASS로 처리하지 않는다.

### G13-15. 차이가 남았을 때의 처리와 최종 인계

문제마다 `sequence/원본 컷/시각 → 기대 화면·값 → 현재 화면·값 → 원인 층 → 수정 대상 → 재검사 결과`를 기록한다. 상태는 **미검사 / 실패 / 자동검사 통과·사용자 확인 대기 / 사용자 확인 완료 / 원본 또는 범위 제약**으로 구분한다. 제약을 PASS로 합산하지 않는다.

| 비교 결과 | 처리 |
|---|---|
| 원본과 후보가 다름 | 추출 트랙, 시간, parent, 좌표·보간의 최초 불일치를 고치고 후보 재생성 |
| 후보는 맞지만 저장/로드 후 다름 | 실제 소비 경로, Save writer, 게시본·revision을 수정. 카메라 좌표를 재보정하지 않음 |
| 로드된 값은 맞지만 최종 재생이 다름 | 카메라 소유권·blend·clock·부모 갱신·표시 덮어쓰기를 조사하고 필요한 구현 범위를 명시 |
| 실제 카메라는 맞지만 원작 구도가 다름 | viewport 투영, 세트 배치, 배우 geometry/pose와 원본 버전 차이 확인. 원인에 맞는 항목만 수정 |
| 원본 또는 배우/FX 작업 없이는 확인·수정 불가 | 해당 장면을 미완료/제약으로 남기고 정확한 입력 또는 범위 승인을 요청. 제외 항목을 몰래 수정하거나 임의 보정으로 완료하지 않음 |

수정은 한 원인 단위로 한다. 예를 들어 P7 비행 구도에서는 시간·부모를 확인한 뒤 거리/FOV를 분리해 보정하고, 가까운 얼굴 장면과 종료 장면에 부작용이 없는지 같은 회차에 확인한다. 다음 회차에 같은 값이 덮어써지지 않도록 원본 생성값과 승인된 수동 보정의 대상 ID·값·사유를 남긴다. 공용 P5 배우를 바꾸는 방식은 사용하지 않는다.

최종 인계에는 컷별 최대 수치 오차와 발생 시각, 누락/중복 검사, 사용자 녹화 기준 장면 판정, Save→Reload 후 유지 여부, 비대상 연출 회귀, 남은 제약을 포함한다. G13-07의 크기·Save 문제와 G13-08의 실제 reader/publisher 검사도 계속 필수다. **이 모든 단계가 닫혀야 카메라·맵 연출 검증 완료이며, 그 전에는 “설명서대로 하면 완벽하다”라고 약속하지 않는다.**

## G15. 2관문 진입 카메라·책/테이블·무대 소품 복구 및 편집 인계 (2026-09-14)

### G15-00. 이번 실행 범위와 문서 상태

이번 사용자 요청의 실행 정본은 **G15만**이다. 앞의 G12/G13은 조사 이력과 재사용 근거이며 그곳의 3관문·빙고·조명 작업을 함께 실행하지 않는다. 기존 계획서를 유지하기 위해 이 절에 후속 범위를 추가했다.

목표: 사용자가 `Lobby → KoukuSaydon → F1 → Action Workbench → Composition Actions → Sequence → Gate 2 → 2관문_진입컷씬` 하나를 선택하면 27초 동안 카메라와 책·도박판·주변 무대가 함께 재생되고, 각 소품을 기존 편집기로 수정·저장한 뒤 다시 열어도 같은 결과가 유지된다. 팀장에게는 이 단일 진입점과 소품별 편집/저장 위치를 전달한다.

이 절은 **확인된 연결과 실패 이력을 바탕으로 한 조사·조건부 수정 실행계획**이다. 원인이 확정되지 않은 좌표나 C++ 정답 패치를 만들어 넣지 않는다. 이번 문서 작성에서 제품 데이터·C++·WModel을 수정하거나 빌드하지 않았다. 구현자는 아래 비교로 최초 불일치를 확정하고, 수정 직전에 이 절의 해당 파일 항목에 실제 교체 블록을 기록한다. H/CPP가 필요해지면 저장소 계획 규칙에 따른 전체 반영 코드와 호출자·실패 소비자를 먼저 보강한다. 이 문서 자체를 복원 완료 증거로 사용하지 않는다.

포함: P3의 카메라 시간·구도·이동·전환, Book/Table/HandBook과 의자·촛대 등 소품의 모델 동작·TRS·표시, 원본 근거가 있는 주변 무대 구성, 기존 편집/저장/재로드 연결.

제외: P1/P2/P4 팝업북·피날레, P5/P7 및 3관문, 빙고, 카드미로, 마리오, 전투 로직·보상·서버 자동 컷신 트리거, 새로운 플레이어 연출. 전등·암전·안개·환경광·SSAO·Bloom·재질·셰이더·카드/연기/빛 링 FX 복구는 팀장 담당으로 변경하지 않는다. 촛대의 메시 배치와 광원은 구분한다. 기존 재질/텍스처 연결 보존은 필수지만 재질 계산을 고치는 작업으로 확장하지 않는다.

세이튼/쿠크 배우의 골격·클립·AnimTree 재제작은 이번 기본 범위에 넣지 않는다. 배우 포즈가 카메라 불일치의 원인임이 확인되면 해당 장면의 증거와 필요한 좁은 수정 범위를 보고해 별도 승인을 받는다. 정상 카메라를 뒤집어 배우 문제를 숨기지 않는다. 독립적으로 가능한 책·테이블·배경 작업은 계속한다.

### G15-01. 현재 파일과 사실의 등급

실제 작업 저장소는 `C:/Users/USER/source/졸업팀폴/LostArk`다. `C:/Users/USER/.codex/worktrees/7395/LostArk`의 오래된 제품 소스로 현재 동작을 판단하지 않는다. 2026-09-14 재확인 시 실제 저장소는 `feature/kouku-cardmaze-visual-fix`이며 World 문서와 여러 C++/runtime 파일에 다른 작업의 미커밋 변경이 있다. 자동 checkout/stash/reset/stage/commit하지 않는다. 수정 전 최신 파일과 사용자의 툴 draft를 보존하고, 작업 브랜치 충돌은 사용자와 조정한다. 이번 `git fetch`는 FETCH_HEAD 권한 문제로 실패했으므로 원격 최신 상태 확인 완료로 적지 않는다.

| 사실 등급 | 확인 내용 | 해석 제한 |
|---|---|---|
| 현재 JSON 확인 | P3 stable ID `KAKULSAYDON_G1_PATTERN_3`, 표시명 `2관문_진입컷씬`, Gate `GATE2`, 길이 27,000ms, WORLD 17개 | ID의 G1 문자열을 G2로 바꾸지 않는다. 표시명도 이번에 불필요하게 바꾸지 않는다 |
| 현재 JSON 확인 | Table → `world.object.kouku.gate2.intro.table`, Motion instance → `world.sequence.instance.kouku.gate2.intro.table`, template → `sequence.kouku.gate2.intro.table` | 존재만으로 전체 동작 정확성을 증명하지 않는다 |
| 현재 연결/생성기 확인 | Table의 clip은 `gate2_intro_27s`, 시작 0ms, rate 1, loop false, holdLastFrame true. `bake_actor`는 30Hz 811개 표본으로 27초 클립을 만드는 경로 | 현재 설치 바이너리의 골격/샘플 정확성은 별도 대조한다 |
| 원본 추출 JSON 확인 | table 그룹 500의 활성 anim track: `evt2_table_open01`을 11.0초 reverse, 15.659999847초 forward, rate 1, non-loop | 원본 시각이다. 녹화 시작 시각과 혼동하지 않는다 |
| 첨부 영상 관찰 | 현재 녹화에도 초록색 테이블이 펼쳐진다. 무대/인물 구도와 주변 구성은 참고 영상과 다르다 | 이번 재확인은 접촉 시트 관찰이며 프레임 정합 후 전 구간 일치 판정이 아니다 |
| 결과 기록 및 현재 검색 | 165개 backdrop 후보를 설치했다가 수직 바닥 띠·공중 카드 문제로 제거한 이력. 현재 World에서 `kouku.gate2.intro.backdrop` 연결을 찾지 못함 | 165를 맞추기 위한 임의 소품 추가 금지. 다른 ID/배치 경로 존재 여부도 원본 actor별로 대조 |
| 과거 결과의 주장 | 쿠크 등 노출은 배우 bake가 원인이라는 기록, parent 회전/ignore-base가 backdrop 의심 지점이라는 기록 | 기록만으로 현재 확정 원인으로 승격하지 않는다. 같은 시각의 실제 pose/행렬로 재검증 |

현재 Composition CAMERA는 5개이며 시작/길이는 `(0,2100), (2100,11850), (13950,5540), (19490,4460), (23950,3050)`ms다. 과거 G13의 7개 CAMERA와 다르다. 생성기의 키 제한 분할 결과와 현재 설치 샷/박스 연결을 다시 읽고 **현재 5개인 이유를 먼저 확인**한다. 5개/7개라는 개수 자체로 누락이나 정상 여부를 판정하지 않는다. 이번에 확인한 `make_cameras`는 64키 제한 분할과 위치 0.005m·방향 0.1도·FOV 0.02도 기준으로 축약한다. 앞 절의 FOV 0.05도 표보다 현재 더 엄격한 기준을 느슨하게 만들지 않는다.

원본 모델/패키지 이름은 사용자 전달 Claude 조사에서 다음과 같이 제시됐다. 이를 추출 manifest/원본 참조로 다시 연결하며 이름 유사 검색만으로 대체하지 않는다.

- SkeletalMesh: `cine_prob_09_s2_2012.mesh.bg_rad_koukusaton_table`
- AnimSet: `cine_prob_09_s2_2012.ani.bg_rad_koukusaton_table_evt2_ani`
- Clip: `evt2_table_open01`
- Package: `CINE_PROB_09_S2_2012`, 물리 파일 `CIHQ2CV9O52N0R2NGE2NE07E.upk`
- 패키지 45 frames / 1.5333초, PSA 47 frames / 30fps라는 전달값은 서로 다른 시간 표기일 수 있다. 단순히 프레임 개수로 길이를 덮어쓰지 말고 converter의 duration/ticks 및 표본 끝점 처리와 함께 읽는다.

### G15-02. 실제 수정 지점과 저장 소유권

아래 경로는 모두 실제 저장소 기준이다. 기존 생성기를 **통째로 재실행**하는 허가 목록이 아니다.

| 절대 경로 | 역할 / 이번 수정 경계 |
|---|---|
| `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py` | `anim_at` 원본 clip 시간 선택, `bake_actor` 골격 표본, `world_pose` 부모/이동 평가, `make_cameras` 활성 Director/샷 생성. 최초 불일치가 입증된 함수만 수정 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KoukuSaydonPipeline/build_gate2_intro_backdrops.py` | 원본 actor/부모에 대응한 무대 소품 후보. 되돌린 165개를 그대로 재설치하지 않음 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py` | 후보 생성/병합 및 `tool_document` 직렬화 재사용. 3관문/빙고를 함께 설치하지 않음 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | P3만의 WORLD/CAMERA 박스와 참조 등록. Sequence Save의 정본 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | Object Resource, template, instance, TRS/visibility/animationTracks. Object Tool Save의 정본 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json` | P3 카메라 eye/lookAt/up/FOV/키. Save Camera 정본 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/Map/KakulSaydon/Gate2Intro/Table/Table.wmodel` | Table의 geometry/골격/구운 clip. Object/Sequence Save로 내부 골격 클립을 다시 굽지 않음 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/WorldObjectTool.cpp` | `Save_Source`가 검증된 World와 필요한 linked 참조를 저장하고 `Start_Publish`가 WorldSequences scope 적용. 저장 충돌 우회 금지 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp` | `Save`, `Request_PatternPreview`, 카메라 편집의 `Save Camera`. 기존 UI 재사용; 설명만 나오는 대체 창을 만들지 않음 |
| `C:/Users/USER/source/졸업팀폴/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1` | Area 또는 WorldSequences scope로 생성/검증. runtime DataFiles 직접 편집 금지 |

정적 배치가 실제로 빠졌을 때에만 해당 Area `.mapplacements`와 Imported catalog의 정확한 원본 actor/asset 행을 추가한다. 같은 소품을 MAP 배치와 OBJECT_RESOURCE 양쪽에 만들어 두 번 그리지 않는다. 새 모델을 만들면 CModel→CMaterial 경로, 기존 material binding 및 모델 상대 DDS 경로를 유지한다. 모델 전체를 합친 하나의 mesh로 만들어 개별 소품 편집을 잃지 않는다.

현재 Table은 OBJECT_RESOURCE다. `Map Tool → Animated Props → Save Animated Props`를 이 Table의 저장 버튼으로 안내하지 않는다. 그 버튼은 별도 Deploy placement 저장 경로다. 모델 내부 접힘 동작, World 이동 키, Composition 박스 시계를 각각 구분한다. Object Save는 경우에 따라 linked 참조도 갱신하므로 완전히 서로 무관한 저장이라고 설명하지 않으며, Save 성공과 뒤의 적용 성공을 따로 확인한다.

### G15-03. 먼저 고정할 비교 입력과 시계

원본 영상: `C:/Users/USER/OneDrive/바탕 화면/2관문 컷신 .mp4`

현재 녹화: `C:/Users/USER/OneDrive/바탕 화면/2관문 진입(프레임워크1).mp4`

확인된 원본 추출 JSON: `C:/Users/USER/OneDrive/바탕 화면/쿠크1관문_연출_원본_20260913/01_컷신별_타임라인/1관문클리어_221_SCENE04A_matinee2_전체.json`

이름은 1관문 클리어지만 이번 2관문 진입 P3에 대응하는 SCENE04A Matinee2다. `tmp/gate2/pkg` 등 다른 세션의 scratch는 존재와 입력 identity를 다시 확인하고 정본으로 가정하지 않는다.

1. 첨부 영상만 오프라인으로 읽어 실제 게임 viewport를 고정한다. 창 테두리/유튜브 바/HUD를 aspect 기준으로 쓰지 않는다. 프레임마다 임의 crop/zoom으로 구도를 맞추지 않는다.
2. 첫 카메라 전환, 책/테이블 등장, 펼침 시작, 후퇴, 마지막 컷의 복수 사건으로 `참고 녹화 PTS → Matinee 시간`, `작업 녹화 PTS → Sequence 시간`을 따로 만든다. 기록 길이 비율로 27초 전체를 늘이거나 0.5초 지연을 임의 적용하지 않는다.
3. 각 문제는 `원본 actor/group/track → 변환 후보 → 저장 JSON/WModel → 로드 후 값 → 재생 시각의 최종 값 → 사용자 영상` 순서로 추적한다. 같은 생성기 함수를 두 번 호출한 0오차만으로 원본 의미가 정확하다고 결론 내리지 않는다.
4. 현재 5개 샷의 key 개수/구간/내용을 과거 7개 분할과 대조한다. 과거 로그의 0오차/최대 65cm를 이번 설치본의 측정값으로 재사용하지 않는다.

기존 read-only 파서/모델 검사/후보 스크립트를 우선 사용한다. 새로운 범용 oracle, 전용 런타임, 대규모 하네스 프로젝트를 자동으로 추가하지 않는다. 제품 UI를 자동 실행/조작/녹화하지 않는다. 최종 runtime 측정에 사용자 실행이 필요하면 정확한 구간과 기존 로그 수집 방법을 전달한다.

### G15-04. Book/Table의 기존 동작 검증과 조건부 재생성

이 작업의 시작은 원본 clip을 또 Append하는 것이 아니다. 현재 27초 baked clip이 원본을 올바르게 표현하는지 검사한다.

입력: 원본 table 그룹의 이동/회전/scale/visibility/anim control, 원본 모델/클립과 설치 WModel, World template 및 instance, Composition `.world.3` 박스.

검사 순서:

1. 원본의 11.0초 역재생과 15.66초 정재생, trim/rate/non-loop/weight/root 옵션을 읽는다. 첫 anim key 이전 pose, 역재생 끝의 hold, 두 키 사이 pose를 확인한다. 첫 키를 무조건 0초 시작으로 바꾸지 않는다.
2. `anim_at` → `bake_actor` → 설치 clip의 같은 source 시간에서 힌지 뼈/테이블 모서리/밑면의 위치·방향을 대조한다. clip이 811개 표본이라는 이유만으로 15.66초 같은 off-grid 사건이 정확히 처리됐다고 하지 않는다. 30Hz bake 오차와 ms 경계의 원본 의미를 따로 기록한다.
3. WModel의 bind/inverse-bind, geometry basis/preScale와 World TRS를 연결해 루트 이동/회전/scale이 중복 적용되는지 확인한다. 뼈 애니메이션과 물체 전체 이동은 별도다.
4. 원본 `cim_constant` 이동은 의도한 순간이동으로 유지한다. 두 키 사이 임의 Linear/SmoothStep 이동으로 책이 무대를 가로질러 날아가지 않게 한다. 현재 샘플/직전 ms 키 방식이 최종 evaluator에서 어떻게 보이는지 검사한다.
5. `holdLastFrame`과 WORLD 표시 종료를 구분한다. 필요한 끝 pose가 27초 이전에 초기화되지 않으며, 원본 의도된 HIDE는 유지되어야 한다. 순방향 재생과 Seek 결과가 같은지 검사한다.

결정: 설치 clip/변환이 맞으면 재쿠킹 없이 보존한다. 골격 샘플이 틀리면 해당 소품의 bake만 고쳐 별도 candidate에 만들고 검증 후 교체한다. World 키/시계만 틀리면 JSON만 고친다. 임의 시작/끝 scale 보정으로 중간에 작아지는 현상을 만들지 않는다. Book/HandBook도 원본 연결을 각각 확인하고 Table과 같은 모델/시간으로 간주하지 않는다.

새 raw clip 재쿠킹은 기존 baked 경로로 표현할 수 없는 실제 저작 요구가 확인될 때만 별도 제안한다. Table/Book을 통째로 재생성하는 명령이 Saydon/Kouku·재질·FX도 덮어쓰면 사용하지 않는다.

### G15-05. 되돌린 주변 무대 소품의 원인별 복구

근거: `C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-13/2026-09-13_KOUKU_G12_CUTSCENES_CAMERA_MAP_RESULT.md`의 G13-R3~R6. 이 기록에는 개수 설명의 산술 불일치도 있으므로 문장의 숫자를 그대로 조립 목표로 쓰지 말고 source actor identity의 실제 집합으로 재계산한다.

원본 actor 하나마다 모델·parent/base·relative TRS·초기 world TRS·active Move/Scale/Toggle·현재 resource/placement를 연결한다. 새 안정 ID는 기존 발급/reader 규칙을 사용하고 포인터나 vector index로 발급하지 않는다. 원본 export 번호는 provenance로 보존하되 런타임 임시 인덱스를 저장 ID로 쓰지 않는다.

먼저 floor16 받침 한 조각, 기둥 한 개, 매달린 카드 한 개를 골라 원본 공간에서 점 3개와 법선/전방, 부모 회전 전후 값을 검사한다. `Base`, `bHardAttach`, `RelativeLocation/Rotation`, `bIgnoreBaseRotation`, initial-relative Move, drawscale/drawscale3d의 의미를 확인한다. 사진만 보고 일괄 90도 눕히거나 부모 회전을 무조건 제거하지 않는다.

대표 소품의 변환을 확인한 뒤 **같은 원본 부착 규칙을 가진 묶음**에 적용한다. 이미 있는 의자 4개·촛대 2개 및 Book/Table과 중복되지 않도록 현재 17개 WORLD와 원본 actor로 join한다. 정적인 구조물도 시퀀스에서 표시/수명이 필요하면 기존 WORLD 소품 경로를 사용하고, 화면에 움직여 보인다는 이유만으로 모두 골격 애니메이션을 만들지 않는다.

매달린 카드의 프로그램 흔들림은 원본 키 복사와 구분한다. 과거 후보의 zero-phase sine를 원본 위상으로 선언하지 않는다. 원본 seed/phase/초기 조건이 복구되지 않는 경우 실제 알려진 진폭·주기와 남는 위상 제약을 기록하고, 사용자의 수동 조정으로 바꾼 값은 원본 복원값과 분리한다.

목표 증거: 책 밑 받침이 수직 띠로 서지 않음, 기둥/의자/촛대의 접촉점이 분리되지 않음, 카드가 잘못된 위치에 줄지어 떠 있지 않음, 마지막 포즈에서 임의 초기화 없음. 수치와 사용자 영상 확인을 별도로 남긴다. 원본에 없는 보충 배치는 자동 설치하지 않고 필요한 위치/이유를 사용자에게 제시한다.

### G15-06. 카메라와 세트를 같은 기준으로 수정

활성 Director의 0/2.10/13.95/19.49/23.95초와 총 27초를 우선 보존한다. 내부 64키 분할은 새 연출 컷이 아니다. 새 ID가 필요하면 다른 샷을 덮지 않는 현재 등록 방식으로 발급한다.

| Sequence 구간 | 검사 대상 | 원인 분리 |
|---|---|---|
| 0–2.10초 | 도입 무대와 배우의 실제 위치, 표시 범위, 첫 camera pose | 무대 객체 미생성과 조명 부족을 분리. 새 검은/불투명 배경으로 가리지 않음 |
| 2.10–13.95초 | 큰/작은 배우, 손과 책의 상대 구도 | actor pose가 틀리면 정상 camera를 임의 회전하지 않음 |
| 13.95–19.49초 | 책·테이블 펼침과 주변 세트 | 안개로 가려지는 차이는 FX 담당으로 구분하되 geometry/시계 오류는 검사 |
| 19.49–23.95초 | 쿠크 클로즈업과 빠른 후퇴 | 원본 cam4 전체 곡선, 실제 재생 clock 및 내부 segment 연속성 |
| 23.95–27초 | 마무리 구도와 소품 마지막 pose | 카드 FX 양과 소품 누락을 분리하고 끝나기 전 카메라 복귀 방지 |

수정 순서: 활성 트랙/원본 시간 → parent/기준 좌표 → eye/forward/up/roll → FOV와 실제 viewport → 구간 내부 곡선 → 최종 runtime camera 소유권/블렌드. 수평→수직 FOV 변환은 기존 생성기에 이미 있으므로 중복 추가하지 않는다. 모델 silhouette 차이와 near clipping, 프레임 밖 잘림을 구분한다.

원본 공간과 프로젝트 공간을 옮겨야 하면 관련 세트와 camera에 공통 rigid transform을 한 번만 적용한다. eye/lookAt은 회전+이동, up은 회전만 적용한다. 기준 배우 위치를 제외 범위라며 고정해 둔 채 카메라만 억지 이동하는 대신 필요한 좌표 보정 범위를 제시한다.

활성 원본 데이터와 참고 영상이 실제로 다른 버전이면 두 결과를 보여 주고 해당 컷의 기준 선택을 사용자에게 요청한다. 비활성 트랙을 몰래 활성화하거나 영상에 맞춘 수동 키를 원본 추출이라고 기록하지 않는다.

### G15-07. 팀장에게 보일 단일 항목과 편집 동선

기존 `2관문_진입컷씬` P3를 유지한다. 새로운 별도 Demo/Benchmark 목록을 만들지 않는다. 하나의 Play로 모든 대상 WORLD/CAMERA가 같이 재생되어야 한다. 전체를 하나의 WModel이나 하나의 편집 불가 안내문으로 합치지 않는다.

팀장용 목록은 기존 UI가 제공하는 표시명/그룹으로 `책`, `도박판`, `의자`, `촛대`, `무대 받침`, `매달린 카드`, `카메라`를 알아보기 쉽게 한다. 표시명은 바꿀 수 있지만 stable ID와 기존 참조는 유지한다. 165개를 개별 클릭해야 전체가 보이는 방식으로 끝내지 않는다. 그룹을 쓰더라도 개별 target의 원본 identity·Transform 편집이 가능해야 한다. UI 기능이 부족하면 새 창을 먼저 만들지 말고 기존 Box Detail → Edit This Motion/Edit Object 호출과 multi-target 편집 지원부터 확인한다.

사용자 편집 위치:

1. 전체 재생/박스 시각: Action Workbench의 P3 Sequence 및 Box Detail.
2. 소품 전체 위치·회전·이동 키·재생 clip: WORLD 박스에서 기존 Edit This Motion/Edit Object로 World Object Tool의 해당 Motion을 연다. Object Resources에서 직접 선택해도 동일 source여야 한다.
3. 카메라: CAMERA 박스의 Open Composition Camera로 키를 편집하고 Save Camera. `Set Camera Pos / Capture view`는 원본 track을 정지 pose로 바꾸므로 원본 궤적 편집에서 사용하지 않는다.
4. 모델 내부의 접힘/펼침: 현재 baked WModel의 영역. Object UI에 없는 골격 키 편집을 가능하다고 안내하지 않는다. bake 변경은 구현자가 처리한다.

현재 코드에서 버튼/연결이 있는지 확인하고, 런타임에서 활성 여부는 사용자가 확인한다. 문서에 메뉴 이름을 적었다는 이유로 실행 확인 완료로 처리하지 않는다.

### G15-08. 안전한 설치·저장·게시

먼저 원본과 현재 정본을 읽어 후보를 별도 out 폴더에 만든다. `build_source_sequences.py`나 G12 전체 `--install`을 실행해 정상 컷신을 다시 생성하지 않는다. 현재 대상 파일 baseline과 draft 보존을 확인한 뒤에만 설치한다. 사용자가 툴을 열어둔 상태라면 Save/종료 여부를 묻고, 외부 수정 중 다시 Save해서 후보를 덮어쓰지 않게 안내한다.

설치 직전 대상 원문이 달라졌으면 중단하고 최신본으로 병합한다. 새 candidate의 검증을 모두 통과하기 전 정본/모델을 부분 교체하지 않는다. 여러 파일 적용의 중간 실패에는 이번 작업으로 바뀐 대상만 복구하고, 다른 사람의 이후 Save나 미커밋 변경은 덮어쓰지 않는다. 복구가 안전하지 않으면 남은 recovery 파일과 충돌 대상을 보고한다.

한도를 설치 직전에 실제 reader로 재확인한다. 과거 World 16MiB/32 target/256 key, Camera 2MiB/128 shot/64 key는 역사적 기준이며 현재 코드와 대조한다. `tool_document`의 툴 호환 직렬화와 Save→Load 왕복을 검사한다. 설치 때만 minify하거나 비대상 키 삭제/상한 증대로 실패를 숨기지 않는다. 동일 World instance에서 같은 Object Resource target을 중복 바인딩하지 않는 기존 규칙도 유지한다.

저장 순서: 소품 Motion Save 및 적용 결과 확인 → Save Camera → 현재 Composition draft와 외부 변경을 재조정 → Sequence Save → 사용자 재로드/재생. Object Save가 linked 문서를 바꿨을 때 freshness 검사를 해제하지 않는다. 시퀀스 Save는 독립 저작 저장이며 전투 Pattern Publish를 대신하지 않는다.

World만 변경하면 기존 Object Save의 WorldSequences 게시를 확인한다. 카메라/배치도 바뀌면 아래 Area 게시를 사용한다. 같은 Area에서 진행 중인 카드미로·조명 작업의 draft/저장값을 먼저 조정하고 대상 밖 authoring은 건드리지 않는다. Area Publish는 다른 저장된 layer도 반영할 수 있으므로 출력 diff를 확인하고, 예상하지 않은 타 작업 반영이 있으면 조정 없이 덮어쓰지 않는다.

실제 저장소 루트에서 사용하는 명령(구현자가 후보 적용 뒤 실행):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Check
```

기존 잘못된 ID 하나로 전체 MapTool이 비는 회귀를 막도록 후보의 모든 참조와 실제 reader 허용 ID를 검사한다. validator를 삭제하거나 잘못된 바인딩을 아무 객체로 대체하지 않는다. Sequence 문서는 해당 `CKoukuSaydonCompositionDocument`의 Sequence 경로로 검증한다. 다른 형식의 범용 Composition publisher나 전투 Publish를 대신 실행하지 않는다.

### G15-09. 최소 검증과 완료 판정

구조 검사: 변경 JSON parse, stable ID/clip/asset/target 참조, 유한 TRS/정규화 quaternion, 올바른 시간 순서, P3 CAMERA 빈틈/의도치 않은 겹침 없음, source actor 누락/중복 대조, 실제 저장기 왕복, 비대상 P1/P2/P4/P5/P7/빙고 및 팀장 source 보존, 변경 파일 `git diff --check`.

동작 검사: 0ms; 모든 원본 anim/Move/Toggle/camera/box 경계의 직전·해당·직후; 펼침 중간과 완료; cam4 빠른 후퇴 내부; 27초 직전. 테이블 11.0/15.66초를 포함한다. 카메라 곡선은 기존 생성기의 16ms 간격 표본과 원본 key를 사용하고, 골격 bake는 30Hz 표본 사이 보간까지 대조한다. 최대 오차와 발생 ID/시각을 남기고 평균만 보고하지 않는다. 과거 G13 제안 허용치와 현재 생성기의 더 엄격한 값을 혼동하지 않는다. 표본 검사는 연속시간 완전 동일의 증명이 아니다.

사용자 확인: 처음부터 연속 Play, Pause/Resume, 같은 시각 Seek, 역방향 Seek, Stop/Reset/Restore의 해당 UI, 두 번째 Play, Save 후 재로드, Client 재실행 후 같은 메뉴에서 P3 재생. 각 경우 책·테이블 포즈/표시와 소품 수가 유지되고 중복 생성/누적 회전/중도 초기화가 없어야 한다. Seek 기능이 지원되지 않는 경계는 정상으로 가장하지 말고 제한/필요 수정으로 구분한다.

정본 데이터/모델만 바뀌면 C++/셰이더 전체 빌드를 요구하지 않는다. 실제 C++ 수정이 필요한 경우에만 저장소의 현재 Product 증분 Build를 사용한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

현재 계획은 신규 C++/HLSL 파일·프로젝트 등록을 제안하지 않는다. 필요가 입증되어 추가할 때는 실제 `.vcxproj`/`.filters` 등록과 영향받는 컴파일을 같은 변경에 포함한다. Rebuild/Clean/OBJ/PCH/tlog 삭제, CSO 타임스탬프 조작, 전체 FxCompile 강제 해제는 하지 않는다. F5/Ctrl+F5 모두 VS 설정에 따라 Build하므로 무빌드라고 안내하지 않는다.

Client/UI는 사용자가 실행한다. 에이전트는 직접 실행/조작/화면 캡처하거나 visual PASS를 기록하지 않는다. 기존 자동 검사를 수정 데이터와 함수 범위로 실행하며 광역 하네스 자동 추가/반복은 하지 않는다.

### G15-10. Claude 결과물과 팀장 인계

구현 결과는 기존 `C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/09-13/2026-09-13_KOUKU_G12_CUTSCENES_CAMERA_MAP_RESULT.md`에 G15 결과로 추가하고 역사적 기록을 지우지 않는다. 결과는 다음을 구분한다.

- 최초 불일치: 해당 원본 actor/track, 현재 resource/clip/camera, 시각, 기대값과 실측값, 실제 고친 파일/함수.
- 변경한 카메라/소품: 원본 직접 복원, 변환 오류 수정, 사용자 승인 수동 보정, 범위 밖 제약을 각각 표시.
- 코드·데이터 적용, 자동 검사, 사용자 실행/화면 확인의 상태를 따로 기록. 실행하지 않은 검사는 PASS로 적지 않음.
- 모델이 이미 정확해 유지한 것과 실제 재쿠킹한 것을 구분. 설치된 clip 이름만으로 복원 완료 선언 금지.
- 팀장용 한 장 안내: 위 단일 P3 메뉴, 소품별 표시명/stable ID/편집 버튼/Save 위치, 대표 확인 시각, 재로드 방법, 현재 EXE/작업 디렉터리.
- Git/DataFiles 전달 범위와 Drive Resources 전달을 구분. Table의 상대 ID `Map/KakulSaydon/Gate2Intro/Table/Table.wmodel`과 실제 물리 경로, 함께 필요한 DDS/모델의 실제 목록, 전달 준비 여부를 기록. Resources를 force-add하거나 전체 pack/hash manifest를 새 완료 조건으로 만들지 않음.
- 배우 bake·전등·안개·암전·재질·FX가 남긴 화면 차이는 시각과 원본 ID/근거를 붙여 팀장에게 인계. 그 차이를 포함해 전체가 원작과 완전히 같다고 보증하지 않음.

완료는 이름 추가나 애니메이션 연결 존재가 아니라, **P3 하나에서 대상 소품과 카메라가 맞게 재생되고 개별 편집·저장·재로드가 유지된 것을 사용자와 확인한 상태**다. 사용자 실행 전에는 `적용·자동 검사 완료 / 사용자 확인 대기`까지만 보고한다.

### G15-11. 측정으로 확정한 원인과 적용 교체 블록 (2026-09-14 Claude 실행)

G15-00~10의 비교를 실제로 수행한 결과다. 원인이 코드·데이터로 확정된 두 가지만 아래 블록으로 적용하고, 확정하지 못한 항목은 G15-11-4에 남긴다.

#### G15-11-1. 확정 1 — P3 카메라 박스가 병합으로 5개로 되돌아감

- 원본 Director 활성 컷: cam1 −1.01초, cam2 2.10, cam3 13.95, cam4 19.49, cam6 23.95. 카메라 액터에 부모 부착 없음.
- `LV_LUT_MIDNIGHTC_ED.camerashots.json` revision 84의 `kouku.gate2.intro.camera.1~7`은 `make_cameras` 재생성과 키 수·시각·eye·lookAt·FOV가 모두 0.0 차이다(같은 생성기 비교이므로 원본 의미 증명은 아님). 구간은 1 [0,2100], 2 [2100,13950], 3 [13950,19490], 4 [19490,21714], 5 [21714,22818], 6 [22818,23950], 7 [23950,27000]이다. cam4는 64키 제한으로 4·5·6 세 샷으로 나뉜다.
- `git show 31936830`(G13)의 Composition P3는 박스 7개였고, 병합 `400439b2 Merge main into Kouku branch and preserve authored sequences`에서 Composition만 이전 5박스(revision 41)로 돌아갔다. camerashots는 7분할 그대로다.
- 결과: 현재 박스 4(19490, 4460ms)는 샷 4의 2224ms 뒤 마지막 키를 붙잡고, 박스 5(23950, 3050ms)는 cam6가 아니라 cam4 중간 샷 5를 재생한다. 샷 6·7은 쓰이지 않는다. 투영 계산으로 박스 5 구간은 촛대 두 개가 화면 안에 들어오는 넓은 후퇴 구도이고, 샷 7(cam6)은 테이블 위 근접 구도다.

교체(`Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`, C++ 저장 형식 CRLF 유지, 텍스트 앵커 교체):

```text
presentationResources
  presentation.kouku.gate2.intro.camera.4  durationMs 4460 -> 2224 (displayName 유지 "... / cam4")
  presentation.kouku.gate2.intro.camera.5  durationMs 3050 -> 1104, displayName "... / cam6" -> "... / cam4"
  + presentation.kouku.gate2.intro.camera.6  CAMERA assetId kouku.gate2.intro.camera.6 durationMs 1132 "... / cam4"
  + presentation.kouku.gate2.intro.camera.7  CAMERA assetId kouku.gate2.intro.camera.7 durationMs 3050 "... / cam6"
patterns[KAKULSAYDON_G1_PATTERN_3].presentationOccurrences
  .presentation.4  startMs 19490 durationMs 4460 -> 2224
  .presentation.5  startMs 23950 -> 21714, durationMs 3050 -> 1104
  + .presentation.9   resourceId camera.6 startMs 22818 durationMs 1132 (다른 필드는 .presentation.5와 동일)
  + .presentation.10  resourceId camera.7 startMs 23950 durationMs 3050
  nextPresentationOccurrenceOrdinal 9 -> 11
revision +1
```

같은 병합에서 P7(3관문_진입 전체 35,368ms)과 G12 빙고 엔딩도 이전 상태로 돌아간 사실을 확인했다. G15 범위 밖이므로 수정하지 않고 결과에 보고한다.

#### G15-11-2. 확정 2 — 무대 소품 165개 되돌림의 원인은 부모 Matinee 트랙 누락

- 소품 165개 모두 `bHardAttach`이고, 부모 저장 Transform × `RelativeLocation/RelativeRotation`이 자식 저장 월드 Transform과 최대 0.013cm·0.039°로 일치한다. 즉 부모-상대 합성 규칙 자체는 맞다.
- 회전 변환은 맵 importer `convert_rotation`과 backdrop의 `BASIS @ R @ BASIS.T`가 무작위 200개 rotator에서 8.9e-16 차이로 같다.
- 원인: `build_gate_cutscenes_g12.install_backdrops`와 `build_gate2_intro_composition.build`가 부모를 `world_pose(rows, None, actor, t)`로 샘플한다. group이 None이면 `active_tracks`가 빈 목록이라 부모 더미의 Move 트랙이 전혀 적용되지 않았다. G13-R3의 "부모 더미가 움직이지 않음"은 이 호출 결과였고 원본 데이터와 다르다.
- 원본 부모 그룹(모두 활성 Move 트랙, `imf_relativetoinitial`):
  - `d1~d8`(cameraactor_3~10, 카드 80장): 0.62초에 약 92m 위로 올라가 23.0~25.5초에 원래 자리로 내려온다. 원본 영상 22~26초의 카드 비와 시각이 맞다.
  - `기둥1~5`(cameraactor_11~15, floor16 판 25장): 11.2~16.5초에 접혔다 펴지고, 23.5초에 판 25장이 Y −107.8~−101.7로 책 페이지(−108.5)와 테이블 윗면(−101.4) 사이, 테이블 가장자리(X −309~−289, Z 439~459)를 두른다. 원본 23~24초 테이블 아래 받침 구조와 같은 자리다.
  - `데스크기둥`(cameraactor_1, floor16 판 25장): 0.88초에 멀리 이동했다가 24.56~25.4초에 테이블 자리로 돌아온다.
  - `tabetcdum`(cameraactor_37, 35장): 세이튼 `bip001-l-hand` 뼈 부착. 이 경로는 bake된 세이튼 손 포즈에 의존한다.
- floor16a~d 메쉬는 두께 0의 YZ 평면(아래로 약 11m)이고 DECO19 촛대도 두께 0 판이다. 부모 트랙이 빠진 저장 자세에서는 판이 세워진 채 테이블 주변에 보였고, 카드는 저장 위치(테이블 위 12m)에 줄지어 떠 있었다. G13-R6 화면과 같다.

교체 블록:

`Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py` — `group_actor` 뒤에 추가하고 `build()`의 호출을 교체한다.

```python
def parent_pose_sampler(rows, matinee=329, interp_data=394):
    """Pose of a set-piece parent including its own Matinee group.

    world_pose(rows, None, ...) skips every Move track, which left the SCENE04A
    camera dummies (pillars, desk legs, cards) at their editor placement."""
    groups = rows[interp_data]["p"]["interpgroups"]
    def sample(actor, seconds):
        group = next((g for g in groups if actor in group_actor(rows, g, matinee)), None)
        return world_pose(rows, group, actor, seconds, matinee, interp_data)
    return sample
```

```python
# build()
backdrops=build_backdrops(rows,imports,parent_pose_sampler(rows),exclude_bone_attached=True)
```

`Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py` — `install_backdrops`:

```python
result = bd.build_backdrops(rows, cache['imports'], base.parent_pose_sampler(rows), exclude_bone_attached=True)
```

`Tools/KoukuSaydonPipeline/build_gate2_intro_backdrops.py` — `build_backdrops(rows, imports, pose_sampler, exclude_bone_attached=False)`:
- 165개 확인 뒤, `exclude_bone_attached`이면 부모에 `basebonename`이 있는 자식을 제외하고 receipt에 `excludedBoneAttached`(actor·부모·뼈·제외 사유)를 남긴다.
- 부모 그룹 이름을 표시명에 넣는다: `d1~d8` → "매달린 카드 dN", `기둥N` → "테이블 받침 기둥N", `데스크기둥` → "책상 다리 데스크기둥". stable ID 규칙은 기존 그대로다.

설치(`--backdrops --install` 모드만, 전체 생성기 `--install` 금지):
- World 문서는 G13 전체 writer가 현재 문서의 `mapMaterialBindings.unlit`에서 동등성 검사를 통과하지 못하므로 전체 재직렬화하지 않는다. 기존 바이트를 그대로 두고 세 배열 끝에 `tool_object_resource/tool_template/tool_instance` 형식의 새 행만 끼워 넣는다. 재파싱 결과가 "기존 문서 + 새 행"과 float32로 같고, 새 행 앞뒤 기존 바이트가 변하지 않았음을 확인한 뒤 쓴다. revision +1.
- Composition은 G15-11-1과 같은 텍스트 앵커 방식으로 `worlds` 등록과 P3 `worldOccurrences`(0~27000ms)만 추가한다.
- 설치 직전 두 파일 바이트가 읽은 baseline과 같지 않으면 중단한다.
- 이후 `Publish-MapAuthoring.ps1 -Scope Area` Validate → Publish → Check.

추가 교체(후보 측정 후, 2026-09-14): 부모 트랙을 적용하자 카드 부모 8개가 묶음 전체 키 합집합 기준으로 두 구간씩 나뉘어 템플릿이 22개가 됐다. 현재 문서 템플릿 237개에 더하면 한도 256을 넘는다. 트랙별 키는 최대 185개라 한 구간에 들어가므로, `build_backdrops`의 구간 분할을 "모든 트랙이 256키 이하이면 한 구간"으로 바꾼다(초과할 때만 기존 합집합 분할). 결과는 부모당 템플릿 1개, 모든 박스 0~27000ms, 이음매 없음이다.

```python
            indices = sorted(set(i for actor in chunk for i in sampled[actor][4]))
            # The 256-key bound is per track. Keep one window when every track fits;
            # splitting on the union adds seams and templates the document cannot spare.
            if all(len(sampled[actor][4]) <= 256 for actor in chunk):
                segment_lists = [indices]
            else:
                segment_lists = [indices[start:start + 256] for start in range(0, len(indices) - 1, 255)]
            for segment, segment_indices in enumerate(segment_lists):
                if segment_indices[-1] != indices[-1] and len(segment_indices) < 256:
                    raise AssertionError("incomplete backdrop segment")
```

#### G15-11-3. 확인했고 바꾸지 않는 것

- Table: 설치된 `gate2_intro_27s`의 움직임 구간이 11.27~12.533초(역재생 닫힘, 11.0 + 46/30초)와 15.667~16.967초(정재생 펼침)이고, 그 사이와 17초 이후는 정지 포즈를 유지한다. 원본 키(11.0 reverse, 15.66 forward)와 PSA 47프레임 길이에 맞는다. 15.66초는 30Hz bake로 7ms 늦다. 원본 비bake 모델(`out/KoukuGate2Restore20260911`)이 이 PC에 없어 뼈 단위 정확도 비교는 하지 못했다.
- Book: 15.20~17.20초 펼침(원본 15.19초 시작), 이후 유지.
- 촛대 두 개: visible 키 전 구간 true, scale 1, 넓은 후퇴 구도에서 화면 안·카메라를 향함(`|n·v|` 0.85~0.91), 재질 family `bg-source-opaque-masked`. 녹화에서 안 보인 원인은 데이터 배치가 아니라 재질·렌더 쪽 후보이며 확정하지 않았다(팀장 영역).

#### G15-11-4. 확정하지 못해 적용하지 않는 것

- 손에 든 책(5~11초): 원본은 쟁반처럼 눕고 위에 미니 세트가 있으며, 현재는 세워져 있다. 저장 키는 원본 Move 트랙 평가와 최대 2.4°로 같고, bake는 AnimTree `evt2_animblending_mix_scale`의 `dummy001` SkelControl(`scale` bone scale 0·강도 0.99, `b_up` +100·강도 1→0.5)을 이미 적용한다. 차이의 원인은 찾지 못했다.
- 손 부착 미니 세트 35장(`tabetcdum`): 부모가 bake된 세이튼 손에 붙는데, 6.8~9.8초 부모 위쪽 축이 수직에서 약 80° 기울어 원본 화면과 다르게 옆으로 누운 세트가 된다. 세이튼 bake 문제일 가능성이 있어 설치하지 않는다.
- 0~2.1초 검은 무대, 쿠크 클로즈업의 등 노출: G13-R4 기록(조명/씬 프로필, 배우 bake) 이후 새 증거를 만들지 않았다.

## G16. 완성 시퀀스에 2관문 무대 14 WORLD만 선택 반영

2026-09-14 PR #384 뒤 사용자는 G15의 무대 소품130개/14 WORLD를 반영하고 PR merge까지 진행하도록 승인했다. 기준은 main `6307ad2a`의 Sequence revision58, World revision1846이다. `c4dc51b0`에서 부모 Matinee 이동이 반영된 backdrop resource130/template14/instance14와 WORLD 등록·발생14개만 선택한다. 우리 기존 행과 P3 CAMERA5개, 다른 패턴·타이밍·재질·튜닝은 보존한다. 손 부착 미니 세트35개는 추가하지 않는다.

World의 세 배열과 Composition worlds/P3 worldOccurrences 끝에 donor 원문 행만 삽입하며 전체 JSON을 재직렬화하지 않는다. revision은 각각+1, P3 nextWorldOccurrenceOrdinal은32로 갱신한다. 기존 행의 의미·원문 보존, donor 행 동등성, ID 충돌0, 참조 모델 존재, template251/256 및 문서16MiB 한도를 검사한다. 설치 직전 원문 일치 검사를 수행하고 두 authoring 파일을 갱신한다.

`Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences`의 Validate → Publish → Check로 해당 runtime 문서만 게시한다. 변경 JSON parse·`git diff --check`를 확인하고 feature branch/PR로 main에 병합한 뒤 pull한다. C++·셰이더·Resources 변경이 없어 재빌드는 하지 않으며 Client/UI 실행과 화면 판정은 사용자에게 남긴다. 실제 결과는 기존 카메라·맵 RESULT의 G16에 기록한다.

## G17. `피날레_맵`의 누락된 벽 움직임만 선택 반영

2026-09-14 사용자가 확정한 대상은 Sequence Composition의 `피날레_맵` → `world.sequence.instance.circusfinale`이다. main `904303a9`에는 수정된 MAP 3·419 배치가 있으나, PR #384의 World 문서 충돌에서 이전 애니메이션이 유지됐다. W-R5의 원본 수정 commit `fb9a8bb9`에서 `sequence.LV_LUT_MIDNIGHTC_ED.circus_finale`의 `obj01` 키를 가져오고, 뒤판용 `obj24` 트랙과 해당 instance의 `obj24 → MAP_PLACEMENT 419` 바인딩을 추가한다. 전체 21010ms, 다른 22개 트랙과 기존 바인딩, 다른 모든 World 행 및 Composition의 타이밍·이펙트·UV는 그대로 둔다. 별도 `Stage1_wall` 시퀀스는 수정하지 않는다.

기존 원문에서 해당 두 트랙·바인딩과 revision(+1)만 치환·추가한다. 설치 직전 원문 일치, 나머지 행의 바이트 일치, donor 동등성 및 참조 배치를 확인한다. WorldSequences Validate → Publish → Check와 JSON parse·`git diff --check` 후 PR merge/pull한다. C++·셰이더 빌드는 필요 없다. 결과와 사용자 화면 확인 경계는 기존 `2026-09-14_KOUKU_STAGE1_WALL_ORIGINAL_RESTORE_RESULT.md` W-R6에 기록한다.
