# Kouku 연출 Workbench 확장 구현 계획서

작성일: 2026-09-13. 기존 통합 구현은 `2026-09-13_ACTION_WORKBENCH_UNIFIED_ACTIONS_RESULT.md`를 따른다.
Gameplay bootstrap 33/34 시작 오류는 해당 RESULT G08에서 별도로 처리했다. 사용자는 실행을 재개했으며
현재 편집 프로세스를 유지한 채 본 연출 작업을 계속하도록 요청했다.

## G00. 현재 입력과 보존 경계

Sequence 정본은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`이다. 기존 창과 통합
Sequence는 모두 같은 `Resolve_SequencePath`를 사용한다. 현재 revision 25의 P4 `1관문_통합_시퀀스`에
firework와 기존 LIGHT .1 occurrence가 있고, 사용자가 추가한 MAP LIGHT .5 definition은 maplights에
있다. .5의 Sequence 연결은 저장본에서 발견되지 않았다. definition과 occurrence의 보존을 구분한다.

Boss 정본 revision 481의 P36 `세이튼_1관문연출`은 13개의 잘린 애니메이션 구간과 Logic 52 블렌딩 박스
5개를 소유한다. 기존 source window, 순서, 박스 시간과 stable ID를 유지한다. 사용자 첨부 이미지는
20,000ms에서 정지한 3D 장면의 중앙 방향 늘어짐과 검은 영역을 보여 주며, UI는 그 위에 정상 표시된다.
원본 fidelity 판정은 사용자가 한다.

현재 Client/Server EXE와 loaded authoring 정본을 외부에서 교체하거나 Reload하지 않는다. 코드와 신규
read-only reference를 구현하고 변경할 기존 저작 데이터는 현재 bytes를 보존한 후보로 `out`에 stage한다.
최종 적용 시 최신 저장본과 후보의 기준을 대조하며 사용자의 중간 편집을 덮어쓰지 않는다.

## G01. Pattern 자식 행과 애니메이션 연출 목록

`KoukuSaydonActionWorkbench.h/.cpp`의 기존 Pattern lane과 resource tree를 확장한다.
`Try_ExpandPatternDocument`가 해석한 child Animation / Logic / Effect 등 실제 연결을 Parent 아래에서
볼 수 있게 하고 선택은 정확한 child owner로 보낸다. 기존 runtime의 중첩 Pattern 해석을 재사용한다.

`Data/Animation/Reference/KoukuSaydon`에 별도 연출 catalog를 생성한다. 원본 Scene/Matinee/actor/track/key,
실제 설치 model/profile/clip, Source In/Out와 재생 속도를 함께 보존한다. Resources → Animation → 연출에
1관문, 2관문, 카드미로, 3관문 진입, 앵콜과 빙고 종료를 한글 묶음으로 표시하고 기존 Preview/Append에
연결한다. 원본 동시 A/B 트랙을 단순 순차 묶음과 동일하다고 기록하지 않는다. 누락 clip은 해당 항목의
오류로 표시하고 다른 정상 목록은 유지한다.

P36은 새 연출 목록으로 덮어쓰지 않고 Sequence에서 기존 연결을 재사용한다. 시작 위치는 해당 연출
생성 위치의 뒤쪽으로 두고 기존 `KOUKU_SAYDON_BOSS_MOTION`과 그 Preview/Product 소비자로 앞쪽 이동을
연결한다. 원본 b_root 이동과 수동 actor 이동을 중복 적용하지 않는다. 첫 walk의 사용자 구간 10277~15274ms와
현재 spawn yaw/ground Y를 유지하고 원본 actor track의 수평 이동거리 12.097183m를 뒤→spawn으로
재배치한다. 원본 높이 변화와 전체 Matinee timing을 동일하게 복원했다는 의미는 아니다.

Sequence에는 현재 P36의 13개 clip/Logic52 박스를 그대로 import하고 기존 P4를 executable Parent로
연결한다. 이는 가져온 시점의 편집본이며 서로 다른 Composition 문서의 자동 동기화 참조가 아니다.
기존 확장은 moving child를 거부하므로, 정확히 하나의 child·비반복·전체 구간·Parent 자체 animation이
없는 경우에만 child BossMotion을 Parent 시각으로 옮겨 실행 snapshot에 승계한다. 다중 moving owner,
repeat와 잘린 moving child는 오류를 유지한다. 원래 clip 순서·Source In/Out·play rate는 바꾸지 않는다.

## G02. Logic 박스 시간에 따른 클립 블렌딩

`KoukuSaydonCompositionDocument.h/.cpp`의 TRIGGER에 typed `ANIMATION_BLEND`를 추가한다. definition은
종류를 소유하고 occurrence의 start/duration이 전이 구간을 소유한다. 구간 안의 유일한 인접 clip
경계를 resolve하며 모호한 경계·잘못된 시간·겹치는 전이는 거부한다. 기존 Logic 52 ID와 박스 시간은
유지하고 종류 변경은 저장 계약을 지키는 후보 또는 기존 편집 명령으로 반영한다.

`Animation_Tool_KoukuPlayback.cpp`, `KoukuSaydonPresentationPlayer.cpp`와 필요한 asset service의 기존
`CModel::ANIMATION_TRANSITION_POSE` 소비자를 재사용한다. 두 clip의 실제 source window와 같은 전이
시간으로 Preview, Seek와 Product를 샘플한다. 기존 blendInMs로 박스 시작 시각을 대체하지 않는다.
`project_kouku_saydon_composition.py`도 같은 의미를 검증·투영한다. 새 gameplay 판정이나 packet은 없다.

## G03. Stop, F6와 라이트 제출

`KoukuSaydonActionWorkbench.cpp`의 일반/Sequence/Complete transport Stop은 `Request_PreviewPause`로
연결한다. Reset은 기존 `Stop_Preview` 정리 경로를 유지한다. 정지 중 clock, animation pose, world/effect
sample과 selection을 유지하고 Play는 현재 시각에서 이어간다.

`Level_KakulSaydonArena.cpp`, `KoukuSaydonPresentationPlayer.cpp`, `MainApp.cpp`에서 F6 free 상태는
카메라 트랙만 적용하지 않는다. 연출 재생은 계속하고 follow로 돌아오면 현재 시각의 카메라를 적용한다.
중간 F6 전환에서 현재 표시된 pose를 유지하며 override를 해제하도록 `Camera.h/.cpp`의 기존 owner
경계에 pose 보존 종료를 추가한다. Complete Play 취소/완료가 강제로 follow를 켜지 않게 한다.

`Presentation_Manager`의 실제 transient capacity 384를 같은 상수로 공개한다.
`MainApp_RenderingLighting.cpp`와 Kouku presentation light provider의 stale 64 제한을 수정하고
MapLightRuntime의 기존 8개 예약과 Engine GPU 총 400(기본 16 + transient 384)의 의미를 유지한다.
provider 순서와 light volume culling도 대조하며 이 한 불일치를 모든 카메라 flicker의 유일 원인으로
미리 단정하지 않는다.

MAP spotlight `light.LV_LUT_MIDNIGHTC_ED.5`는 팝업 book HOLD와 camera center의 X=0, Z=737.28을 사용한다.
기존 Y=9.317626과 색상·cone·intensity는 유지한다. 기존 .1 LIGHT row를 덮어쓰지 않고 .5를 구분해서
연결하는 데이터 후보를 만들고 기존 Map publisher로 배포한다.

## G04. 포탈 장면 수축과 차원술사 캡처

현재 Scene Profile은 조명·노출·fog·environment를 소유하며 화면 수축 소비자는 없다. 포탈은 기존
EffectPlayback의 source transform과 native material 경로다. 이 실제 소비자를 조사·확장하여
검은 배경 위 장면 이미지의 시간에 따른 축소·소멸을 연결한다. 원본 재질을 임의 변경하지 않고
기존 Screen Post에 `screen.scene-collapse.capture.v1` 저작 profile을 추가한다. 이 profile은 기존
`IPresentationScreenPostMaterial`/quad/ping-pong 경로와 frozen HDR/bloom snapshot을 사용하고
명시된 박스 수명에서 이미지 크기를 1→0으로 줄인다. native 원본 복원값과 프로젝트 보강을 구분한다. 현재 장면을 같은 renderer의
SceneHDR/SceneBloom snapshot으로 읽으며 별도 전역 renderer나 UI 화면 캡처를 만들지 않는다.

`Effect_DocumentRenderer_Staging.cpp`, MaterialHelpers/Rendering/Geometry와 필요한 public header 및
해당 native shader가 변경 범위다. 원본 material과 좌표/UV·시간 소비를 대조한 뒤 잘못된 왜곡과
누락된 수축을 구분한다. 끝·Seek·Stop에서 capture 수명과 검은 배경을 재현 가능한 시각으로 유지한다.

포탈 variant별 lead-in은 서로 다르다. 기존 `Effect_Tool_Editing.cpp`의
`Remove_CinematicLeadingDelay`가 보존하는 source clock 계약을 사용하고 단순히 모든 key에 17초를
일괄 빼지 않는다. 실제 Effect Tool 입력에서 지연 제거된 연출을 선택·재생할 수 있게 연결한다.

차원술사 cube는 기존 `Capture_StartingSceneTarget`의 frozen SceneHDR/SceneBloom SRV와 ALT178 texture
소비를 실제 asset에 연결한다. 6면 cubemap을 새로 렌더링하는 기술과 현재 view 이미지를 cube material에
사용하는 기술을 구분해 설명한다. 기존 candidate의 emitter/animated model 적용도 해당 owner와 source
baseline을 확인한 뒤 게시한다. 신규 또는 수정 resource는 기존 Resources 상대 ID 경계를 유지한다.

## G05. 등록, 검증과 최종 적용

신규 C++이 필요하면 현재 물리 폴더 기준으로 Client/Engine `.vcxproj`와 `.vcxproj.filters`에 필요한
항목만 등록한다. 기존 C++의 BOM/UTF-8 및 줄바꿈을 유지한다. 신규 reference JSON은 실제 loader가
소비하고 Client `96.DataFiles` None 항목으로 노출한다.

변경 기능의 parse/save/reload·실패 시 기존 문서 보존, authored source windows와 blend 경계 샘플,
정지/재개/F6 상태 전이, 실제 provider 경계와 shader compile을 확인한다. 현재 프로그램이 실행 중이므로
별도 컴파일·링크·후보 검증은 모두 `out`에서 수행한다. 검증 결과는 대응 RESULT에 기록한다.
새 코드 적용 시에는 정상 Product Build로 Engine SDK/Client와 변경 shader를 함께 배포하고 해당 domain
publisher를 실행한다. 이 단계 전에는 준비 상태와 현재 실행 제품을 구분한다. 최종 사용자 화면 확인과
미수행 검사를 PASS로 기록하지 않는다.

## G06. 원본 이동을 local Animation Play에도 연결

사용자가 Server Play 외의 툴 Play 이동도 연결하도록 명시했다. 기존 CModel의 named/current/transition
bone sample은 suppression 적용 후 행렬을 반환하므로 이를 이동 추출에 그대로 쓰지 않는다.
`Model.h/.cpp`에 suppression 전 root-local 변위를 실제 부모 basis와 모델 pre-transform으로 변환하는
읽기 전용 `Sample_AnimationRootTranslation`을 추가한다. 입력은 clip/ticks/root index/수직축/수직배율,
출력은 모델 좌표 변위이며 실패 시 출력과 live pose·cursor를 유지한다.

기존 suppression 설정은 현재 pose를 rest로 잡기 때문에 재생 중 그대로 켜면 원점이 오염된다.
immutable skeleton rest를 사용하는 설정과 기존 suppression 상태 capture/restore를 추가한다.
이 상태는 preview owner가 보유하고 release/실패에서 복원한다. 제품 Server animation의 억제 정책은
바꾸지 않는다.

신규 `Client/Public/KoukuSaydonPreviewRootMotion.h`와 `Client/Private/KoukuSaydonPreviewRootMotion.cpp`는
기존 Composition clip window와 실제 CModel에서 준비한 root 입력을 절대 시각으로 샘플한다. Source In,
Out, delay, rate, HOLD/LOOP, 반복 끝점 누적, stage 누적과 해당 stage yaw를 반영한다. 수동 BossMotion,
charge, teleport가 이동을 소유한 Pattern에서는 자동 이동을 중복 적용하지 않는다.

`Animation_Tool_KoukuPlayback.cpp`와 `KoukuSaydonPresentationPlayer.cpp`의 실제 preview Begin/Sample/
Reset·Release가 공통 helper를 소비한다. actor 위치와 Effect 기준점을 함께 이동시키며 Seek는 재생
이력과 무관한 절대 결과를 내고 Stop은 현재 pose, Reset/종료는 원래 preview 상태를 복원한다. 클립만
참고하는 모델 표시를 제품 이동 명령으로 바꾸지 않는다. 서버 packet/authority는 변경하지 않는다.

실제 설치 WModel의 백스텝, crop/속도/LOOP와 제자리 walk를 확인하고 actor/본의 중복 이동, suppression
복원, 실패 시 기존 preview 보존을 검사한다. helper H/CPP는 Client vcxproj/filters의 기존 Tool 계층에
등록하고 Engine public 변경은 최종 Product Build의 SDK 및 Client 소비자까지 확인한다.

## G07. 백스텝 후 감전빔의 원본 대조와 복구

사용자가 원본 스크린샷으로 현재 복구의 오류를 지적했다. 현재 18요소 문서는 Projectile 421990316의
비행 6요소를 세 방향으로 묶은 저작본이며 원본의 모든 후속 시스템을 포함한 복원본이 아니다.
고정 −30/0/+30도와 75m·50m/s는 기존 저작 선택값이므로 원본 실측값으로 표시하지 않는다. 현재 사용자
P46의 여러 stage와 변경한 연결을 이전 문서의 단일 stage로 덮어쓰지 않는다.

`Effect_Playback.cpp`의 ribbon 폭은 실제 portable carrier 분기를 통해 검증한다. 이 문서는
`Is_PortableAuthoredRibbonCarrier` 분기가 먼저 확률분포를 반영한 particle size를 소비하므로 뒤쪽의
deterministic-only helper를 현재 폭 결함으로 오판하지 않는다. 실제 CPU sample의 폭·점 간격·수명과
원본 분포를 대조하고, 잘못된 소비가 재현된 범위만 수정한다.

현재 생성기는 ribbon capacity를 60Hz 기준 32점으로 줄였지만 실제 portable carrier는 원본
SpawnPerUnit(30cm) particle stream을 연결한다. 기존 저작 속도 50m/s와 원본 수명 0.5초에서는 약
83점이 필요하다. 원본 500점 상한과 실제 전체 문서 예산을 대조해 이 잘못된 고정 60Hz 제한을
제거하고 실제 point count, 양 끝 거리·폭·head 시각을 검증한다.

원본 `par_v_rpct_elecarea_02_loc_int`의 검은 sprite와 붉은 번개 mesh/sprite, 관련 light는 현재 문서에서
제외되어 있다. serialized projectile/impact 연결과 발생 시각·위치를 확인한 뒤 기존 renderer/material로
포함하는 후보를 만든다. 확인 전에는 영상 한 장으로 impact 시각이나 새 trajectory를 발명하지 않는다.
Tools/EffectPipeline의 기존 생성기를 수정하여 원본 provenance, 좌표·폭·시간과 프로젝트 선택값을
구분하고 현재 loaded 문서는 직접 교체하지 않는다. native playback 수치·shader compile·Codec 검증은
자동으로 확인하고 최종 원본 외형 판정은 사용자에게 남긴다.

## G08. Box Set Group 시간 이동

사용자는 Box Set Group의 중앙 기준 회전은 동작하지만 위치 이동이 적용되지 않는다고 보고했다.
실제 코드에서는 group 선택으로 selected box 수가 늘어나면 Presentation box drag가 명시적으로
비활성화됐다. 여러 box를 선택한 그룹도 타임라인의 공통 delta로 이동하도록 연결하고 최초 시작
0 경계를 그룹 전체에 적용한다. 각 box의 상대 시각, 길이와 stable ID는 보존한다. 공간 translation은
이미 stage/Preview/save 경로가 연결되어 있으므로 확인된 시간 drag 차단을 수정한다.

## G09. 타임라인 겹침의 그룹별 행 분리

사용자는 시간상 겹친 박스들을 그룹 단위로 나누어 표시하는 의미라고 확답했다. 각 그룹의 전체
시간 구간으로 표시 행 묶음을 배정하고 같은 그룹의 박스들을 붙여서 표시한다. 내부적으로도 시간
구간이 겹치는 개별 박스는 클릭 가능한 하위 행을 가진다. 선택과 drag는 원래 stable ID에 연결하고
표시 행 때문에 기존 저작 시각이나 재생 블렌딩 정책을 바꾸지 않는다.

## G10. 화염포의 누락된 불꽃 층

사용자 첨부 화면에는 발광점과 회색 입자가 주로 남고 연속 불길이 부족하다. 현재 저작 ID는
`effect.kouku.gate3.backstep.flame`, `.full`, `.ring.flame`이며 공통 생성기는
`Tools/EffectPipeline/build_kouku_backstep_flame_groups.py`다. 실제 원본 emitter와 native material,
texture/subUV/particle color·alpha를 대조한다. 누락 또는 잘못된 소비가 재현된 부분만 수정하고
다른 정상 smoke·spark나 사용자 occurrence transform에 전역 색/배율을 강제하지 않는다.
현재 source를 기준으로 보존 후보를 만들고 기존 Codec/renderer/native particle sample을 검증한다.

## G11. 부채꼴 예고의 중심→바깥 fill

`effect.kouku.gate3.showtime.sector.warning.shot`의 native3602는 고정 외곽선과 inner scalar의
노란 내부 반경을 구분한다. 원본 시간은 caustic에 쓰이며 현재 inner는 고정 .66이다. 원본 시간 fill
근거가 없으면 inner 0→1 곡선을 프로젝트 저작으로 추가한다. 기존 sourceTransformTrack의
materialParameterTracks와 실제 native packet의 inner row0.z를 재사용한다. 외곽선을 두 번 그리는
element 복제는 필요하지 않다.

`Effect_ArtistMaterial.h`의 기존 Mesh admission을 실제 Kouku LocalDecal과 원본 재질 계약을
충족한 경우에만 확장한다. 기존 renderer의 Decal sample/bind 소비자는 유지하고 shader를 복제하지
않는다. `build_kouku_showtime_warning_groups.py`는 opt-in 후보 생성 경로에서 원본 요소·사용자
transform·시간을 보존하고 fill 곡선만 추가한다. 시작/중간/끝의 실제 native 반경과 source curve,
Codec 저장·재로드를 확인하며 최종 색·가시성은 사용자 판정으로 남긴다.

09-14 후속 요청은 같은 예고의 inner 곡선을 정본에 설치하고 사격 SpriteEmitter `_0`, `_17`,
`_19`의 세 stable element를 제거하는 것이다. 기존 부채꼴 Decal의 ID·반경11m·45도·색·위치·
1.5초 예고 수명은 유지한다. `--stage-sector-warning-only` 후보 생성 경로는 같은 입력에
재실행해도 결과가 같아야 한다. V1_ELEMENT도 기존 renderer의 표시 범위만 제한하므로 같은
SourceTransformTrack materialParameterTracks와 native packet 소비를 검증한다.

## G12. Effect Tool Play All의 불필요한 보스 모델 요구

실제 V1 Play All은 `Start_WorldPreviewFromBeginning → Try_PlayRecoveryEffect →
Select_SceneEffectTarget`을 사용한다. 현재 `effect.kouku.` 이름만으로 needsModel을 true로 만들어
순수 월드 포탈·독립 조합에도 Boss Composition 연결을 요구한다. Sequence에만 등록된 포탈은
이 검사에서 renderer staging 전에 거부된다. 별도 V1/V2 lifecycle 조사에서는 Update·Layer 추가·
렌더 제출 연결의 누락을 발견하지 않았다. 입력 소유권을 실제 넘길 때의 Stop과 구분한다.

`Client/Private/EffectAuthoringSequencer.cpp`의 needsModel은 기존 호출자가 수집한
`requiresSourceModel`과 문서의 명시 `SourceModelPreview`로만 결정한다. source 본·외부 모델
attachment가 필요한 경로는 유지하고, 순수 world/map은 기존 모델 선택 없는 player anchor 경로를
사용한다. 새 runtime/공통 public 필드/프로젝트 항목을 만들지 않는다. 실제 함수의 독립 포탈·
축포·화염과 본 부착/명시 source model·V2 진입 분기를 검사하고 제품 옵션으로 해당 TU를 컴파일한다.
기존 실행 중인 Client/Server와 미저장 데이터는 보존하며 EXE 반영은 종료 상태 확인 후 진행한다.


## G13. 2026-09-14 Play All과 Sequence의 일반 v15 문서 재생

사용자는 동일한 중심 앵커 포탈에서 Mesh/Sprite Play Family는 보이지만 Play All은 보이지 않는다고
보고했다. 실제 `Create_AuthoringOccurrence`와 Catalog의 직접 로드·worker staging·debug 등록/교체는
v15 전체 문서에 runtime projection을 강제한다. 대상 문서는 runtimeCarrier와 baked history가 모두
비어 있는 정상 v15이므로 `Create_DocumentOwnedRuntimeProjection`의 no admitted runtime carrier에서
거절된다. 앞선 G12의 모델 요구 해제는 이 다음 준비 단계까지 검사하지 못했다.

기존 `CEffectDocumentCodec`에 검증 후 사용하는 `Requires_DocumentOwnedRuntimeProjection` 판정을
추가한다. v15의 실제 carrier 또는 history가 있을 때만 기존 projection을 요구하며 일반 문서는
기존 Stage_Document로 재생한다. `Effect_Tool_Workspace.cpp`의 전체/선택 재생과
`Effect_Catalog.cpp`의 직접 로드·Product worker·Debug registration/replacement 및 이전 cache
일관성 검사를 같은 판정으로 연결한다. 실패 시 rollback, canonical identity와 malformed
carrier/orphan history 검증은 유지한다. 새 CPP와 project/filter 항목, 저작 JSON 변경은 없다.

현재 실제 centered/portal-arrival 문서의 이전 실패와 수정 후 staging/CPU emission, 실제 ribbon
carrier의 projection 유지, 잘못된 확장 거절을 집중 검증한다. 수정 TU는 현재 Product와 같은
toolset/SDK/문자 집합으로 컴파일한다. 실행 중인 Client/Server와 미저장 draft를 보존하며 정상
Product 링크는 출력 점유가 해제된 뒤 수행한다. 최종 화면 판정은 사용자가 한다.


## G14. 2026-09-14 고정 장면 수축과 탐색 방향에 따른 화면 차이

사용자가 제공한 세 이미지와 재현 조건은 RESULT에서 보존한다. 전체 재생의 포탈 생성은 사용자 확인을 받았다. 새 요청은 Sequence 19,819ms의 장면을 보존한 뒤 검정 배경 위의 사각형으로 수축하고, 같은 장면 캡처 경로를 차원술사 ALT_V에 적용하되 바깥은 현재 장면을 유지하는 것이다.

현재 scene-collapse는 occurrence 준비 중 마지막 완료 SceneHDR/Bloom을 복사하며 post의 시작 시각과 캡처 시각이 다르다. 변경은 실제 렌더 입력이 준비된 ScreenPost Bind에서 color/bloom을 한 번 복사하고 occurrence가 그 상태를 소유하도록 한다. 후처리 입력은 왜곡 resolve 뒤, HUD 앞의 장면이다. 매 프레임 생성되는 material packet은 같은 고정 캡처 상태를 참조하며 저장본 refresh는 같은 stable element와 타이밍일 때만 이를 보존한다. 실패한 GPU 복사는 기존 캡처를 부분 교체하지 않는다.

`Effect_NativeScreenPostMaterial.h/.cpp`와 기존 shader의 사각형 수축 경로에 바깥 검정 또는 live scene 합성, 실제 큐브 투영 크기를 연결한다. `Effect_AuthoringDocument.h`, Codec의 profile/상세 읽기·쓰기·검증, Tool의 profile 표시를 확장한다. cube profile은 실제 ModelCue stable ID와 그 시작 시각에서의 첫 pose 크기를 사용한다. 기존 두 ALT178 camera mesh만 숨기고 cube ModelCue와 나머지 이펙트는 보존한다. ModelCue animation cursor를 임의로 되감지 않는다. 새 C++ 파일과 project/filter 추가는 없다.

Sequence의 19,819ms는 현재 portal-suction.scene-collapse 박스 시작15,559ms에서4.260초다. 현재 카메라 .27은 이 시점 FOV 약167.394도이며 뒤에178.9도까지 커진다. 큰 화면 늘어짐을 Sprite lifetime만으로 설명하지 않는다. 실제 시간 진행과 역탐색의 캡처 재생성을 조사하여 필요한 수정만 추가하고, 전환 이후 고정 캡처가 현재 카메라 출력에 다시 덮이지 않도록 합성한다. 사용자 편집 중인 데이터는 최신 기준본과 field 단위로 대조하여 보존한다.

검증은 실제 Codec 저장 왕복과 잘못된 cue join 거절, 포탈 전환 전후 시간과 forward/reverse 상태, 설치된 cube 모델의 투영 수치, shader 컴파일 및 변경 TU 컴파일이다. UI는 실행하거나 자동 캡처하지 않는다. 제품 링크와 사용자 최종 화면 확인은 실행 상태와 함께 RESULT에서 구분한다.


### G14.1. Sequence의 캡처 경계 프레임과 요청 시각 보존

`KoukuSaydonPresentationPlayer.h/.cpp`의 `Resolve_PreviewCaptureClock`은 현재 재생 중인 단일
Pattern의 V1 ScreenPost 시작 시각을 occurrence 시작과 더하여 구한다. 숫자19,819를 코드에
고정하지 않는다. 아직 해당 stable element의 GPU 캡처가 없으면 사용자가 요청한 시각을 보존하고
실효 시각을 그 경계에 유지한다. 캡처 준비가 확인되면 원래 요청 시각으로 진행한다. 이미 종료된
post/occurrence와 다른 선택 element는 경계를 만들지 않는다. 사용자 Seek와 Stop은 보류 요청을
정리하고, 잘못된 occurrence admission은 기존 실패 상태를 소비한다.

`MainApp.cpp`는 단일 Pattern의 두 clock owner가 만든 최종 시각을 WORLD 샘플 전에 이 경계에
통과시킨다. Animation Tool이 clock owner이면 기존 `Seek_KoukuCompositionPreview`로 모델도
같은 실효 시각에 둔다. WORLD와 presentation/camera는 같은 시각을 받는다. 일반 서비스 Update가
이미 끝난 뒤 전달되는 V1 seek는 `Commit_WorldRootCaptureSample`로 해당 admitted handle만 즉시
확정한다. 기존 transform history 재생 경로를 사용하여4.260초를 고정 simulation step의4.25초나
이전 Effect frame의 시각으로 대체하지 않는다. WORLD/카메라 변경 전 Late_Update에서 생성한 렌더
등록을 캡처하지 않도록 첫 경계 프레임은 capture permission을 끈다. WORLD/presentation Sample이
완료된 다음 동일 경계 방문부터 허용하여 정상 Late_Update가 새 시각의 가시성·카메라를 소비하게
한다. GPU 캡처가 준비될 때까지 경계를 유지하며120 preview update를 넘으면 명시적으로 실패한다. 원래 시각으로 돌아갈 때는 같은 occurrence의
관찰 history와 고정 캡처를 유지한다. Server 제품 clock과 Bundle 전용 clock은 이 변경 대상이 아니다.

`Effect_Object.h/.cpp`는 renderer의 stable element별 `Has_CapturedScreenPost`를 전달하고,
`Effect_PresentationService.h/.cpp`는 현재 admitted world-root handle에서 그 상태를 조회한다.
대기 중 spawn은 준비 완료가 아니다. `Get_ScreenPostCaptureResult`는 준비 대기와 GPU 복사·할당
실패 HRESULT를 구분하여 실패한 캡처를 무한 대기하지 않게 한다. 전역 ScreenPost A/B가 꺼져 있으면
경계를 만들지 않고 대기 중 요청도 원래 시각으로 해제한다. shared GPU texture 자체는 renderer가
계속 소유한다. 새 파일이나 project/filter 항목은 필요하지 않다.

검증은 실제 resolver 함수를 추출한 CPU 검사에서 전진 경계 통과, 캡처 대기·완료, 후방1ms 두 번,
정지 상태의 정확한 경계, 경계 이후 직접 Seek, 종료된 구간, 선택 element 격리와 admission 실패를
비교한다. 이 검사는 GPU copy나 사용자가 보고한 두 이미지의 실제 교대 재현을 대신하지 않는다.
관련4 TU의 Product 옵션 컴파일과 `git diff --check`를 별도로 확인한다.


### G15 추가: 쇼타임 발사 섬광의 저FPS 표시 누락

`effect.kouku.gate3.showtime.gun.signature`의 실제 원본 총구 particle 수명은40~70ms다. 10FPS에서 Update100ms를60Hz fixed step6개로 진행하고 끝 상태만 그리면 그 사이의 birth/death가 모두 지나가 총구 render row가0이 된다. 실제 Codec+Playback에서60FPS의16.7ms에는 gunfire4+1개가 생성되지만10FPS/100ms와약6.7FPS/150ms에서는0개임을 재현했다.

`Effect_Playback.cpp/.h`의 기존 Update와 Update_WithTransformHistory에서 지정된 쇼타임 signature/signalshot 문서 및 source identity의75ms 이하 burst만 다룬다. 해당 particle이 아직 display frame 후보로 한 번도 제출되지 않았고 이번 update의 중간 fixed step에서만 살아 있다면, 중간에 평가한 양의alpha particle행을 최종 display frame에 한 번 보존한다. 동일요소의 후보가 이미 최종frame에 있으면 중복하지 않는다. 원본 lifetime, birth/death simulation, source clock, 다음update의live particle상태를 바꾸지 않는다. Seek는 정확한 지정시각을 유지하고 보존후보를 만들지 않는다.

중간평가는 해당shortburst요소만 기존 Rebuild_Frame의 particle 평가를 재사용한다. 다른요소의frame을 매 substep 다시평가하지 않는다. 문서의원본모듈과정본JSON은보존하고, 의도된 한displayupdate의표시보존임을 결과에서구분한다. native검사는60FPS동작불변,10/5FPS총구후보존재와다음update삭제,Seek/정지/다른문서격리를확인한다. GPU화면확인은사용자가한다.


보존 대상은 해당 원본 경로의 sprite, 1회 loop, 시각0의 단일 burst, 연속방출0, 최대64 birth와 실제75ms 이하 수명으로 한정한다. 한 update의 임시 후보는 최대256행이다. 문서 재stage·Reset은 후보와 제출기록을 초기화한다. 제출기록은 문서소유 element주소이며 저장ID가 아니고 source event/birth 기록도 아니다.

`EFFECT_EVALUATED_PARTICLE::fMaterialSampleTimeSeconds`는 일반 행에서-1, 보존 행에서 대표 substep 시각을 가진다. 양의alpha 최종행이 없을 때만 해당 element의 보이지 않는 terminal행을 대표행으로 교체하므로 한 sprite batch에 서로 다른 material clock이 섞이지 않는다. `Effect_DocumentRenderer_Particles.cpp`의 기존 sprite bind가 이 시각으로 common color와 native material time을 계산한다. 현재 총구 profile2439는 texture alpha×particle color alpha, glow2433는 radial falloff×particle color alpha이며 materialtime을 직접 참조하지 않는다. signalshot2422는 color/dynamic/depth를 소비한다. 그래도 대표행의 World·Color·Dynamic·NormalizedLife·SubUV·material clock을 같은 substep으로 유지한다. 실제 GPU 화면과 depth visibility는 사용자 확인 범위다.

### G16. 외곽불 World Object의 조명 독립 색상

사용자가 배경과 같은 불 색상을 요청한 D/E/F 세 객체만 기존 mapMaterialBindings의 optional `unlit`을 true로 지정한다. 이미 연결된 동일 BG diffuse·UV·원본 mask를 보존하고 움직이는 객체에 정적 RNM/lightmap을 복제하지 않는다. 생략값은 false다. WorldSequenceDocument 읽기·쓰기·동등성, publisher의 strict bool 검증, 기존 CModel surface override를 통해 이 선택을 실제 정적/animated shader까지 전달한다. unlit 표면은 diffuse RGB를 조명 곱 대신 기존 emission MRT에 기록하고 직접 diffuse/specular 기여를 0으로 만든다. 다른 객체·조명·spotlight·배치·동작을 변경하지 않는다.

`WorldSequenceDocument.h/.cpp`, `WorldSequencePlayer_Objects.cpp`, `ModelAssetData.h`, `MapAssetRenderUtils.cpp`, 기존 Map surface/static/animated/instanced shader와 `Publish-MapAuthoring.ps1`을 변경한다. 새 파일·프로젝트 등록은 없다. 실제 문서 저장 왕복과 잘못된 bool 거부, 세 fire slot/source/texture 대조, scoped WorldSequences publisher, 최소 TU 및 FXC 컴파일을 검사한다. 기존 Client/Server 실행은 유지하며 화면 판정은 사용자가 수행한다.


### G17. Composition 창의 반복 계산과 표시 중 로드

현재 사용자 profiler에서 Composition UI 구성은26프레임 평균83.346ms다. kind/version으로 제외할 Sound4,036행까지 먼저 텍스트 검색하고 고정9개 owner 이름을 반복 정규화하는 경로를 줄인다. 열린 본 선택 콤보에만 전체 이름 목록을 만들고 닫힌 상태는 기존 exact Find_BoneIndex로 검사한다. Toolbar·Timeline·Details·Patterns·Resources·PresentationResources와 검색 구간을 기존 Profiler로 계측한다. 기존 dirty·선택·저장·명령과 리소스 정본은 유지한다.

과거 camera 최초 실패 캐시는 유지되어 있으며 이번 profiler에 camera load 호출은 없다. 별도로 Created Resources의 V1_ELEMENT 이름을 그리면서 Catalog::Find가 파일 읽기를 재시작할 수 있었다. 표시에서는 펼친 metadata와 Find_Loaded만 소비하고 찾지 못하면 저장된 element ID를 표시한다. 이름이 확인된 경우만 Use Source Name을 제공하며 명시 Refresh/Locate Source 경로는 유지한다. 실제 입력의 검색 결과 일치, 반복 표시의 load API 호출0, 수정 TU 컴파일을 검사하고 전체 pane 시간 및 실제 FPS와 좁은 CPU 비교를 구분한다.
