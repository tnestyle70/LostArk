# 각성기·쿠크 연출 후속 구현 계획

## G00. 현재 실측과 작업 경계

시작 HEAD는 `e26cd2b282293bd4cb433338bdbc6a7315dd3f84`, 작업 브랜치는
`codex/awakening-kouku-presentation-fixes`다. 기존 Composition, patternbindings,
Encounter, worldsequences, Warlord17240의 사용자 변경을 보존한다.
선행 상태는 09-09 LanceMaster V/AltV, Artist/Warlord full restore 및 09-10
Kouku Pattern Effect Anchor Fear Authoring PLAN/RESULT를 따른다.

도화가 ALT+V는 재사용 실패 원인을 분석하고 수정한다. 창술사 ALT+V와 V/T 용 크기,
워로드 ALT+V는 원본 추출 자료와 실제 소비 경로를 비교해 누락을 복구한다.
Client/UI는 실행하거나 조작하지 않으며 새 결과의 육안 판정은 사용자에게 남긴다.

## G01. 창술사·워로드 원본 카메라와 소환 모델

기존 `CEffectRecoveryCamera`가 읽는 sidecar에 원본 Matinee 카메라를 연결한다.
원본 track, notify 시작/종료와 clip 시간의 변환을 맞추며 두 번째 camera runtime을
추가하지 않는다. 소환 모델의 크기는 원본 geometry, cooked geometry, bone scale,
modelPreScale의 중복 여부를 먼저 실측한다. 증거 없는 임의 확대는 원본 복원으로 기록하지 않는다.
클래스별 effect/animation 데이터와 필요한 기존 importer만 변경한다.

## G02. 거미 얼굴의 화면 합성

현재 `CEffectV2Object::Submit_Presentation -> CPresentation_Manager ->
CRenderer::Render_ScreenPosts -> Render_Final` 흐름은 얼굴도 HDR에 넣고
암전 profile의 exposure를 적용한다. `SCREEN_POST_PARAMS`와 기존 overlay descriptor에
기본 false인 `displaySpace`를 추가하여 선택한 TexturedOverlay만 tone mapping 이후,
제품 UI 이전에 합성한다. texture의 sRGB/linear 해석과 화면 출력 색 공간을 구분한다.

V2 JSON reader/writer, Tool checkbox, 공식 Python 구조 검사와 실제 Renderer/HLSL
소비를 함께 연결한다. 기존 HDR overlay의 순서와 ping-pong target은 유지한다.
얼굴 leaf만 옵션을 켜고 암전 profile과 다른 효과는 보존한다. 새 C++ 파일은 없으며
기존 project/filter 등록을 사용한다. Deferred pass를 뒤에 추가하고 기존 index를 보존한다.

## G03. 공 분포와 휠윈드 이동

공 box의 사용자 변경 `3080 -> 1708ms`를 기준으로 기존 ball motion의 추가 지연을
줄인다. 현재 bounce motion에 기본 0인 평면 spawn extents를 연결하여 반복당 안정된
난수로 가로/세로 위치를 선택한다. 저장, publisher, runtime, Tool 편집을 함께 유지한다.

휠윈드는 현재 사용자가 늘린 회전 구간을 사용하고 기존 Server charge 계약으로
시작 시 player 방향을 정한다. 거미보다 낮은 평균 속도와 긴 거리로 이동하며
Server navigation/collision과 같은 snapshot presentation을 소비한다.

## G04. 패턴 트리와 대형 세이튼 등장 연출 설명

`CKoukuSaydonActionWorkbench::Render_PatternsAndResources`의 트리 표시 높이를
240에서480으로 늘린다. 대형 세이튼은 현재 순차 animation stage, bundle offset,
bundle presentation occurrence 소비를 조사하여 이펙트 선행 후 등장하는 저작 방법과
실제로 추가가 필요한 공백 처리 범위를 설명한다. 사용자가 묻는 설계 설명을 임의의
Gate 전체 진행 순서 생성으로 확대하지 않는다.

## G05. 도화가 반복과 차원술사 F·V·D 원본 입력

`Effect_PresentationService.cpp`는 원본 emitter의 내부 시간과 action occurrence 수명을
구분한다. Artist31930은 원본 500초 emitter clock을 바꾸지 않고 실제 6초 action cue를
`cue_end`로 종료한다. 새 action이 같은 owner/asset/occurrence를 다시 제출할 때 이미
지난 action의 CUE_END 예약만 해제한다. 다른 effect와 NATURAL tail은 그대로 둔다.

F2050230 현재43개와 D2050240 현재25개를 보존하면서, 지원 분기가 없어서 빠졌던
원본26개씩을 복구한다. 원본 MIC 이름뿐 아니라 static map, vertex factory, PS/VS,
상속 uniform과 texture를 대조해 같은 프로그램은 재사용하고 다른 프로그램은 기존
SD native 분기에 추가한다. material 식이 같아도 빠진 world position, source origin,
particle dynamic, scene/depth 입력이 있으면 실제 carrier까지 연결한다.

V2050520은 현재42개를 보존한다. 두 StartSize의 unbaked UniformRange가 양수·음수
구간을 단일 min/max로 합친 결함을 실제 export32의 네 벡터로 복구한다. Shader의
명령 문자열 차이를 결함으로 단정하지 않고 최종 RT0 수치에 영향을 주는지 검증한다.
F 가장자리와 D 메인 검격의 기존 native 연산 일치 결과와 구성 누락은 구별한다.

## G06. 각성기 원본 배경의 합성 순서

원본 배경용 opaque static mesh에 선택적인 `compositionLayer: sceneBackdrop`을
저장한다. 기존 MESH와 `OPAQUE_BACK_DEPTH_WRITE` 조합만 허용한다. 기존 WORLD_MARK의
NONLIGHT 제출 단계를 재사용하여 캐릭터의 depth를 보존하면서 일반 투명 이펙트보다
먼저 그린다. 활성 프레임의 local 제출 요소가 실제 배경을 포함할 때만 Renderer에
환경 교체를 요청하고, Client의 MapAssetObject와 MapStaticBatchObject는 그 프레임의 맵·sky 및 map shadow
draw를 생략한다. 기존 scene/profile, placement visibility와 캐릭터 렌더 경로는 유지한다.
프레임 성공·실패 종료에서 요청을 초기화한다. 이는 배경이 활성인 동안의 전체 맵
교체이며 부분 화면 stencil mask가 아니다. 원본에 있는 배경 geometry를 재사용한다.

Artist 2개, Lance 각 clip의 opaque 19개, Warlord opaque 44개 section을 대상으로
한다. Lance alpha 2개씩과 Warlord alpha 4개는 기존 합성을 유지한다. 배경과 camera
particle의 `localOnlyElementIds`를 sidecar까지 연결하여 다른 플레이어의 연출이
로컬 환경을 교체하지 않도록 한다. Tool 저장·재로드와 Solo isolation도 같은 계약이다.

## G07. 차원술사 평타 입력과 원본 효과

원본 첫 clip 자체의 두 찌르기 notify 0.2/0.4초를 유지한다. 제품 콤보는 첫 clip,
BA3, BA4 세 단계로 연결하고 Server comboStage, 각 단계 root motion, authored
skillbindings와 입력 창을 함께 맞춘다. 첫 동작 1.4초와 후속 동작의 실제 길이를
보존하면서 늦은 다음 입력이 유실되지 않도록 검사한다. 두 번 찌르기를 위해 첫
clip 뒤에 별도 BA2를 추가하지 않는다. 원본 BA2 자료는 비교 후보로 보존한다.

원본 75개 중 현재 59개를 기준으로 누락 16개를 복구한다. source MIC, PS/VS와
vertex factory를 대조하여 미연결 sprite와 근사 분기의 정확한 프로그램을 연결한다.
원본 StartSize의 네 벡터를 잃은 UniformRange, particle 입력, native alpha와
좌표·크기 변환을 실제 소비자까지 복구한다. F/D와 동일한 프로그램은 재사용한다.

## G08. 쿠크 Reverse Sector와 레이저 밀림

Reverse Sector는 같은 원 또는 타원 안에서 기존 sector를 제외한 영역을 뜻한다.
저작 collider 선택·저장, publisher, Server 판정, Client debug 표시를 함께 연결한다.
피자 safe wedge, 보완각 경계와 player body radius의 교차를 기존 Shared XZ 계약에
맞춰 검증한다. sector를 단순히 180도 회전한 모양으로 처리하지 않는다.

Collider의 X/Z 크기를 독립적으로 허용하고 `radiusXM/radiusZM`을 optional 쌍으로
투영한다. 각도는 축척 전 단위 원의 각이며 X=right, Z=forward다. 플레이어 몸체는
실제 metre 단위 원을 유지하고, 두 방사 선분과 타원 arc의 최단거리로 판정한다.
정규화 공간에서 body 반경을 임의 확대하지 않는다. Shared의 기존 원형 Cone은
유지하며 새 타원 함수의 equal-radius 결과와 호환성을 검사한다. World Track의
균일 축척 계약은 유지하고 Collider 자체 X/Z 축척만 독립화한다.

단순 데미지 영역은 Box Detail에서 직접 선택한다. 기존 `Set_ColliderTriggerDamage`의
원자적 ENTER_AREA/RESULT 연결을 재사용하여 피해(% 최대 HP), 재진입·밀림 종료 후 반복,
밀림 거리/시간/방향을 편집한다. 동일한 설정만 stable definition을 재사용하고 다른
설정은 해당 occurrence의 연결만 바꾼다. 조건 분기와 성공·실패를 가진 기존 Logic은
보존하며 단순 피해를 위해 사용자가 이름 붙은 Logic을 별도로 만들 필요를 없앤다.

쿠크 레이저의 기존 collider occurrence에 휠윈드와 같은 Server damage/push 경로를
연결한다. 밀림 방향은 boss에서 player로 향하는 벡터 대신 해당 시점 쿠크의 forward를
사용한다. 밀림 동안의 navigation/collision, 지속 접촉 재판정과 snapshot 소비를 보존한다.

## G09. 검증과 인계

변경 JSON/XML parse, 관련 기존 focused 검사, 필요한 Debug 컴파일과
`git diff --check`를 실행한다. Client/Server 종료 확인 뒤 마지막 사용자 Save를
다시 읽고 해당 domain만 게시한 뒤 제품 링크·배포한다. 빌드 성공과 사용자 화면
확인을 분리하여 RESULT에 기록한다. 대규모 사용자 dirty 변경을 자동 commit하지 않는다.
