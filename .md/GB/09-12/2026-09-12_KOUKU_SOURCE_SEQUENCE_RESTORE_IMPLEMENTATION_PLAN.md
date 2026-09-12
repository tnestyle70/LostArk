# 쿠크 원본 시퀀스 저작 연결 구현 계획

## G06. 정상 팝업북·피날레를 기준으로 결합하는 수정 계획 — 미적용

2026-09-12 사용자가 **코드를 수정하지 말고 원인 조사와 수정 계획까지만 진행**하도록 범위를 변경했다. 이 G06은 조사 결과와 다음 구현의 명세다. 이 후속 조사에서 제품 코드·Data·Resources·runtime·ZIP은 수정하지 않았고 publisher·빌드·Client 실행도 하지 않았다. 아래 G00~G05는 최초 원본 Matinee 연결 이력이다. 그중 G1의 41,488ms, source Book/배경/배우 선택은 다음 구현에서 이 G06으로 대체한다. 다른 관문의 기존 변경은 이 계획의 대상이 아니다.

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

RenderingProfiles에 `scene.kakulsaydon.g1.popup.v1`을 추가한다. 현재 `scene.kakulsaydon.g1.base.v1`을 복사해 qualityOverride, shadow, 환경 반사 입력과 scalar는 보존한다. light.diffuse/ambient에는 region `kouku.ps.environment.48`의 directionalColor/ambientColor를 사용한다. fog의 density/heightFalloff/topHeight/startDistance/maximumOpacity/color도 region48 값으로 교체하고, region fog의 inscatteringColor/lightDirection은 기존 base fog의 `sourceExponential` 안으로 대응시킨다. 나머지 fog 필드는 base를 유지한다. `environmentRegions=[]`로 두어 이 연출 동안 카메라가 region47 또는 바깥 영역에 들어가도 F1 환경이 유지되게 한다. 기존 G1 profile과 지역 조명 문서를 변경하지 않는다.

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

effectiveExposure/effectiveBloom 계산은 각 operand를 single로 읽은 뒤 product를 single로 변환해 runtime과 맞춘다. 기존 테스트 파일에0.1/0.0001/0.0312의9유효숫자 저장→Validate/Publish 왕복, 다음 범위 밖 float32, NaN/Infinity/overflow, 실패 시 기존 destination 보존을 추가한다. Rendering runtime revision이 authored와 같은지 검사하는 기존 테스트는 정상 Publish 후에 실행한다. 이번 조사에서는 이 변경과 Publish를 실행하지 않았다.

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

변경한 C++는 기존 Client Debug x64 빌드로 컴파일한다. 새 H/CPP·shader 등록은 없다. `Client/Default/Client.vcxproj`의 기존 Effect None ItemGroup에 G06-03의4개 JSON을 `..\..\Data\Effects\...` 상대 경로로 추가하고, `Client.vcxproj.filters`에는 같은 Include와 `<Filter>96.DataFiles</Filter>`를 추가한다. 기존 항목과 필터는 재배치하지 않는다. 실제 데이터 변경과 함께 Area publisher의 runtime map 출력을 전달한다. 이번 계획에는 Resources 신규 물리 파일이 필요하지 않다. ZIP은 코드 적용·게시·빌드가 끝난 후 별도 배포 단계에서만 갱신하며 현재 ZIP에는 이 미적용 계획이 반영되지 않았다.

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
