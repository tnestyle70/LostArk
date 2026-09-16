# 2관문 진입 — 전체 재생 복구와 원본 맵 애니메이션 수정 계획

## M01. 이번 작업과 이전 계획의 관계

사용자 최신 요청에 따라 1관문 벽 카메라와 분리한 **2관문 맵 애니메이션 전용 실행계획**이다. 이전 `2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_IMPLEMENTATION_PLAN.md`의 G15는 역사적 근거로 읽되 이번 실행 범위는 이 문서를 우선한다. 예전 17 WORLD/배경 미설치 상태로 돌아가 작업하지 않는다. 1관문 작업은 별도 WALL_CAMERA 계획을 따른다.

목표: F1 → Action Workbench → Composition Actions → Sequence → Gate2 → `2관문_진입컷씬`의 전체 Play가 실제로 진행되고, 저장된 원래 세트 위치에서 책·테이블·받침·카드 등 원본 맵 소품 동작이 함께 재생되며 개별 수정/Save/Reload가 유지된다.

**Object 탭 Preview at Character 성공은 이 목표의 완료가 아니다.** 이번 문서는 제품 수정 명령의 실행 순서이며, 아직 원인이 확인되지 않은0ms 정지의 정답 패치를 만들어 넣지 않는다. 구현자는 최초 실패를 확인하고 해당 파일/함수의 실제 교체 블록을 계획에 보강한 뒤 최소 수정한다. 새 H/CPP가 필요하면 저장소 규칙의 전문과 프로젝트 등록을 함께 명시한다.

포함: P3 전체 재생에 필요한 실패 경로 교정, 기존 소품 원본 동작/부모/시계 검증과 필요한 수정, Book/Table/HandBook과 주변 소품의 위치·회전·scale·visibility·clip, 저장·재로드·반복 재생.

제외: 1관문·3관문·빙고·카드미로·마리오, 새 전투/서버 흐름, 세이튼/쿠크 배우 골격 재제작, 전등·암전·안개·환경광·재질·셰이더·새FX. 촛대 메시와 광원은 구분한다. 카메라 연출 자체의 재설계는 제외하지만, 아래처럼 확인된 P3 샷/박스 연결 불일치는 맵 동작 관찰을 위한 기존 카메라 연결 복구로 좁게 처리한다. 원본 카메라 궤적을 임의 조정하지 않는다.

## M02. 최신 상태 — 이미 된 일과 아직 안 된 일

실제 루트 `C:/Users/USER/source/졸업팀폴/LostArk`. 작성 시 branch `feature/kouku-cardmaze-visual-fix`; 미커밋 `Level_KakulSaydonArena_WorldObjects.cpp`, `MainApp.cpp`, `Level_KakulSaydonArena.h`, RESULT와 Framework.sln 변경을 보존한다. 자동 reset/checkout/stash/stage/commit하지 않는다.

| 상태 | 현재 확인 내용 | 다음 행동 |
|---|---|---|
| 현재 JSON | P3 `KAKULSAYDON_G1_PATTERN_3`, Gate2, 27000ms, WORLD31개 | 기존17+추가14 WORLD를 보존하고 identity로 대조 |
| 기존 적용 기록 | 부모 Matinee를 반영한 backdrop Object130개, template14/instance14 설치. 카드80+받침/다리50 | 옛165개 후보 재설치/중복추가 금지. 현재 그룹/모델별 확인 |
| 제외된 대상 | 손 뼈에 붙는 tabetcdum35개가 제외됐다는 기록 | 복구 완료 개수에 합산하지 말고 원본 parent/bone·현재 손 pose부터 확인 |
| 미커밋 코드 | Preview at Character의 baseline을0인 instance.position 대신 첫 키로 잡는 선택 인자/호출 수정 | 삭제하거나 다시 덮지 말고 실제 transform 의미와 회귀 점검 |
| 검증 수준 | 기존 cl /Zs 구문 검사 기록, 제품 링크·화면은 미확인 | 구문검사 성공을 빌드/실행 성공으로 표시하지 않음 |
| 사용자/RESULT 보고 | 전체 Sequence Play가0ms에 머무름 | **M03을 첫 실행 단계로 처리**. 별개라고 남겨둔 채 완료하지 않음 |
| 현재 CAMERA | Composition5박스, 실제 shots7개 분할 | M06의 참조/길이 불일치를 확인하고 좁게 연결 복구 |

부모 누락 원인은 과거 결과에서 `world_pose(rows,None,actor,t)`가 부모 자신의 Matinee Move를 누락한 것으로 설명한다. 현재 `parent_pose_sampler(rows)`를 사용하는 설치본인지 실제 생성기와 각 group/instance를 확인한다. 과거 PASS를 새 작업의 실측으로 재사용하지 않는다.

현재 Preview at Character 코드는 `position==0 && emissions.empty()`를 절대좌표 판별 조건으로 쓴다. 이는 모든0위치 객체가 절대좌표라는 schema 증명이 아니다. 첫 키 숨김/원거리 치움/지연, 여러 target 평균, 비단위 instance TRS, 일반0위치 모션을 확인한다. 로컬 미리보기용 보정을 저장 World 키나 전체 컷신에 적용하지 않는다. 53개 영향이라는 전달값은 실행 시 다시 열거하며, 마리오 등 다른 대상에 전역 보정을 확대하지 않는다.

## M03. 먼저 전체 Sequence Play의0ms 정지를 진단·교정

현재 확인된 호출 경계:

`CKoukuSaydonActionWorkbench::Request_PatternPreview → Queue_PatternDocumentPreview → Consume_PatternPreviewRequest(MainApp) → CKoukuSaydonPresentationPlayer::Begin_Preview/Begin_BundlePreview → Begin_KoukuWorldPreview → Notify_SequencePreviewAdmission → 재생 clock/Update`

P3는 주로 WORLD/CAMERA/EFFECT로 구성되므로 실제 hasAnimation 분기와 별도 clock 소유자를 현재 코드로 확인한다. 다른 Pattern의 동작 경로를 추정해 적용하지 않는다.

진단 순서:

1. 실행 EXE/실제 source root/저장문서·in-memory revision, P3 선택, startPaused, 단일 Sequence Play와 Complete Play를 구분한다. 과거 `Sequencer Benchmark`와 현재 Action Workbench의 실제 route owner를 추적한다.
2. 요청이 큐에 들어왔는지, MainApp이 소비했는지, presentation 준비/World 준비 중 어디서 false가 나왔는지, admission 결과와 최종 상태를 확인한다. 즉시 Stop/Reset 또는 owner 변경으로 clock이0으로 돌아가는지 구분한다.
3. 실패한 occurrence/instance/asset/단계를 보존하는 현재 로그를 사용한다. 필요하면 해당 경계의 최소 진단만 추가하고 성공 상태가 첫 실패 원인을 덮지 않게 한다. 무기한 대기/준비 반복과 실제 진행 중 prewarm을 구분한다.
4. JSON/asset/바인딩 실패면 정확한 대상만 고친다. 카메라/이펙트가 실패 원인이어도 필수 트랙을 몰래 삭제/무시하여 성공시키지 않는다. 소유권·시계·승인·정리 오류면 기존 소비 경로만 교정하고 새 재생기를 만들지 않는다.
5. 실패 이후 재시도가 가능하며 준비 중 생성된 객체·카메라 owner를 복구하는지 검사한다. 유효한 시작 후0→중간→끝이 진행하고 pause/seek/replay도 같은 clock으로 동작해야 한다.

팀장 영역의 실제 Effect/재질 복구가 없이는 전체 시작이 불가능한 경우에는 그 실패 ID와 이유를 인계하고 전체 성공으로 표시하지 않는다. 독립 소품 검사는 계속할 수 있지만 그것을 전체 Play 완료로 바꾸지 않는다.

Client 실행은 사용자에게 요청한다. 먼저 사용자가 재생했을 때 상태줄/기존 로그/멈추는 구간을 확인하고 코드상 안전한 진단을 진행한다. 자동 Client/UI 실행·캡처는 하지 않는다.

## M04. 책·도박판의 원본 동작과 첫 키 의미

원본 입력:

- `C:/Users/USER/OneDrive/바탕 화면/쿠크1관문_연출_원본_20260913/01_컷신별_타임라인/1관문클리어_221_SCENE04A_matinee2_전체.json`
- `C:/Users/USER/OneDrive/바탕 화면/2관문 컷신 .mp4` / `2관문 진입(프레임워크1).mp4`
- 원본 Table Mesh `cine_prob_09_s2_2012.mesh.bg_rad_koukusaton_table`, AnimSet `cine_prob_09_s2_2012.ani.bg_rad_koukusaton_table_evt2_ani`, clip `evt2_table_open01`. 실제 원본 package/추출 모델과 identity를 확인한다.

현재 Table: `world.object.kouku.gate2.intro.table` → `world.sequence.instance.kouku.gate2.intro.table` → `sequence.kouku.gate2.intro.table` → `Map/KakulSaydon/Gate2Intro/Table/Table.wmodel`의 `gate2_intro_27s`.

원본 table 활성 anim track의11.0초 reverse,15.66초 forward,trim/rate/loop/root/weight를 확인한다. 45frame/1.5333초 원본 표기와 PSA47frame/30fps, baked811표본/27초를 단순 개수로 같다고 하지 않는다. 첫 키 이전 pose, 역재생 끝 hold, 골격 표본 사이 보간과 off-grid 시작 시각을 확인한다.

동작이 이미 정확하면 WModel을 재쿠킹하지 않는다. 원본 clip을 기존27초 clip 위에 중복 Append하지 않는다. 틀리면 원본 clip→anim_at→bake_actor→설치 WModel→World TRS의 최초 차이만 고친다. 비bake 입력이 이 PC에 없다는 기존 기록이 있으므로 실제 원본을 확보하기 전 뼈 단위 정확성 완료를 선언하지 않는다.

Book/HandBook도 각각의 원본 actor/clip/parent를 대조한다. 5~11초 손의 책이 세워져 보이는 문제는 알려진 미해결 항목이다. 원본 책 자체의 local transform/SkelControl와 소유 배우의 bone pose를 분리하고, 배우 재제작이 필요하면 정확한 범위 승인을 받는다.

instance.position0과 절대 positionOffset, geometry preScale(cm→m), placement/key scale, parent/root 회전을 따로 검증한다. 첫 키를 캐릭터 앞으로 옮기는 편의 변환을 원본 복원값으로 저장하지 않는다. 원본의 원거리 치움은 존재 가능하지만 **수백m 이동이면 무조건 원본이라 정상**이라고 하지 않는다. 원본 Move/Toggle·카메라 전환과 해당 시각을 함께 대조한다.

## M05. 현재130개 소품을 검증하고 남은35개를 분리

각 source actor를 현재 resource/template/instance/slot/모델에 대응한다. raw actor 집합, 이미 직접 연결된 소품, 추가130개, 미설치35개를 구분한다. 숫자165를 맞추기 위해 임의 추가하지 않는다.

우선 받침 판 하나, 카드 하나, 책상 다리 하나를 원본 부모의 실제 Matinee group으로 평가한다. Base/Relative TRS, hardAttach, initial-relative Move, ignore-base 옵션, drawscale를 한 번씩 적용한다. floor16 세로 판을 정적 바닥이라고 간주해 모두90도 눕히지 않는다.

현재 기록의 대표 사건(실제 원본 재확인): 받침11~16.5초 접힘/펼침, 카드 부모0.62초 상승 및23~25.5초 하강, 데스크기둥24.56~25.4초 복귀. 부모를 정지 pose로 샘플링해 공중 카드 줄·수직 띠가 재발하지 않게 한다. 카드의 부모 하강과 카드 자체 프로그램 흔들림/FX를 구분한다. zero-phase sine를 원본 위상이라고 기록하지 않는다.

손 부착 tabetcdum35개는 socket/골격 basis/preScale와 부모 local transform부터 검증한다. 원본 bone pose를 재현할 수 있으면 기존 CModel/World 경로로 후보를 만든다. 소유 배우 bake가 틀려 수정을 요구하면 그 묶음은 미완료로 보고하고 승인받는다. 임의 월드 고정으로 손을 따라가는 기능을 대체하지 않는다.

기존 의자4개·촛대2개와 중복 생성하지 않는다. 촛대가 없으면 생성/표시/크기/카메라 시야와 실제 material/render 경로를 분리한다. 재질 원인이면 팀장에게 정확한 모델/slot/시각을 넘긴다.

## M06. 맵 관찰을 막는 현재 카메라 연결 불일치

현재 파일로 확인한 사실: camera.4/5/6/7의 track 길이는2224/1104/1132/3050ms인데 P3는 camera.4를4460ms, camera.5를3050ms 재생하고6/7은 참조하지 않는다. 이는 이전7분할 샷과5박스가 섞인 상태다. 이 사실만으로0ms 정지의 원인이라고 단정하지 않는다.

기존 활성 Director와 샷 identity가 유지됐는지 재확인한 뒤 **박스 연결만** 다음 구간으로 맞추는 후보를 만든다. 임의 camera pose 변경이나 새 연출은 아니다.

| 실제 샷 | 시작–종료(ms) |
|---|---|
| camera.1 | 0–2100 |
| camera.2 | 2100–13950 |
| camera.3 | 13950–19490 |
| camera.4 | 19490–21714 |
| camera.5 | 21714–22818 |
| camera.6 | 22818–23950 |
| camera.7 | 23950–27000 |

현재 ordinal과 참조를 확인해 새 occurrence를 발급하고 옛문서 전체를 복원하지 않는다. 단순 샷 분할 경계에 별도 camera return/blend가 끼지 않게 한다. 기존5개를 남겨7개를 추가해 중복하지 않는다. 기존 CAMERA 리소스의 duration/표시명도 실제 샷과 맞추되 EFFECT/조명/암전 박스는 보존한다.

## M07. 수정 파일·저장·검증 순서

실제 루트 아래 다음 파일을 사용한다.

| 파일 | 역할 |
|---|---|
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | 요청 큐·admission·transport 상태 |
| `Client/Private/WorldObjectTool.cpp`, `WorldSequenceToolPanel.cpp`, 기존 Map Tool 연결부 | M09의 소품/target 편집·저장·전체 재생 복귀. 기존 정본과 편집기 재사용 |
| `Client/Private/MainApp.cpp` | 요청 소비·presentation/World 시작·owner 연결. 기존 미커밋 수정 보존 |
| `Client/Private/KoukuSaydonPresentationPlayer.cpp` | 실제 준비·시계·재생/정리 |
| `Client/Private/Level_KakulSaydonArena_WorldObjects.cpp`, `Client/Public/Level_KakulSaydonArena.h` | World 시작·preview baseline. 현재 Claude 변경을 검증 후 이어받음 |
| `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py`, `build_gate2_intro_backdrops.py`, `build_gate_cutscenes_g12.py` | 원본 샘플/부모/후보. 전체 install 금지 |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | 기존31 WORLD의 소품 template/instance/clip/표시 정본 |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | P3 WORLD/CAMERA 발생 연결·길이 |
| 같은 Area `camerashots.json` | 기존7개 샷 읽기/참조 검증. 필요 없이 궤적 수정하지 않음 |
| `Client/Bin/Resources/Map/KakulSaydon/Gate2Intro/` | 실제 오류가 확인된 소품만 재쿠킹; 기존 DDS/material binding 보존 |

실행 순서: 현재 변경/draft 보존 →0ms 원인 분리·최소 수정 → 전체 재생 준비 → 원본 동작/부모 검증 → 틀린 소품만 후보 생성 → 현재 샷 연결 복구 → 참조/시간/Save 검증 → baseline 재확인·설치 → 사용자 실제 전체 재생.

Preview 편의 수정의 빌드가 필요하면 해당 변경을 포함한 Product 증분 빌드로 실행 준비한다. cl /Zs만 하고 화면 확인을 사용자 빌드 몫이라고 떠넘겨 완료 처리하지 않는다. Client가 출력물을 점유하면 먼저 Save/종료를 요청한다. Server는 이번 변경 필요성이 입증되지 않으면 건드리지 않는다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

데이터-only 수정은 컴파일 불필요. Rebuild/Clean, OBJ/PCH/tlog 삭제, 강제 FxCompile 제외는 금지다. 신규 범용 하네스·런타임을 기본으로 추가하지 않는다. 필요한 C++ 신규 파일이 생기면 .vcxproj/.filters 등록도 포함한다.

소품은 Object Save와 적용 완료, 카메라는 Save Camera, 박스는 Sequence Save로 관리한다. Object Save의 linked 동기화와 freshness를 보존한다. Table은 OBJECT_RESOURCE이므로 Save Animated Props(Deploy)를 저장 방법으로 안내하지 않는다. World만 바뀌면 WorldSequences scope, 카메라/맵도 바뀌면 Area scope로 게시한다. 실제 root에서 사용하는 명령:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Check
```

Camera도 변경하면 위 세 명령의 Scope를 Area로 바꾼다. runtime DataFiles는 직접 편집하지 않고 같은 Area의 타 작업 출력도 확인한다. 독립 Sequence 문서는 실제 Sequence reader로 검증하며, 다른 형식의 범용 Composition publisher나 전투 Publish로 대신하지 않는다.

후보를 현재 writer/reader로 저장 왕복하며 추가 필드(예: mapMaterialBindings.unlit)를 오래된 writer가 누락하지 않는지 확인한다. 현재 문서 용량과 template 등의 상한은 실제 코드로 확인하고, 다른 motion 삭제나 상한 무조건 확대로 해결하지 않는다. 같은 Object target 중복 binding, 잘못된 ID로 전체 MapTool이 비는 회귀를 사전 검사한다. 여러 파일 적용 실패 시 이번 변경만 복구하고 외부 Save를 덮어쓰지 않는다.

**1관문 계획의 설치와 동시 진행하지 않는다.** 공유 Camera/Composition 파일은 첫 작업 완료 후 최신 baseline으로 두 번째를 시작한다. 두 Claude 세션을 동시에 쓸 경우 조사만 병행하고 저장/설치는 순차 진행한다.

## M08. 팀장에게 넘길 완료 기준

필수 확인: P3 전체 Play clock 진행, 0/11/15.66/17/19.49/23~25.5/27초의 원본 동작 관계, pause/resume, 동일시각 Seek/역방향 Seek, Stop/Reset, 두번째Play, Save→Reload 및 Client 재실행. 원본의 의도된 숨김/원거리 치움과 버그를 분리하며 편의 preview 화면만으로 판정하지 않는다.

팀장에게는 P3 하나의 선택 경로, 소품 그룹 표시명/원본 actor/stable ID/편집 버튼/Save 위치, 미설치35개와 배우·재질 등 남은 제약, 실제 변경 파일/원인/자동 검사/사용자 판정을 별도 표로 준다. 새 Resources가 있으면 상대asset ID와 실제 경로/DDS 목록/Drive 전달 여부를 남기고 binary force-add는 하지 않는다.

결과는 이 계획과 같은 이름의 RESULT에 기록하고 기존 G15-R2/G16 결과를 링크한다. 0ms 정지가 남으면 전체 재생은 미완료다. Source130개가 있다는 사실도 원본 전체 복원 완료가 아니다. 에이전트는 제품UI를 실행·조작·캡처하지 않으며 사용자 서면 판정 전 visual PASS를 기록하지 않는다.

## M09. 전체 연출을 보며 사용자가 직접 수정하는 편집 경로 — 추가 필수 범위

원작 동작 설치뿐 아니라 사용자 후속 작업이 가능해야 한다. F1 → Action Workbench → Composition Actions → Sequence → Gate2 → 2관문_진입컷씬을 출발점으로 전체 연출을 재생/정지/Seek하고, 소품을 골라 수정한 뒤 같은 연출에서 결과를 확인하고 저장할 수 있게 만든다. 설명 문구만 있는 패널이나 캐릭터 앞 단독 Preview만 제공하는 것은 이 요구를 충족하지 않는다.

### 어느 도구에서 수정하는가

Action Workbench의 Edit This Motion / Edit Object와 기존 Object 편집기를 먼저 점검한다. 필요한 편집을 할 수 없다면 Map Tool → World Sequence에서 해당 대상을 편집할 수 있게 연결·구현한다. 현재 Map Tool에 버튼이 보인다는 사실만으로 OBJECT_RESOURCE 또는 여러 target 묶음이 지원된다고 가정하지 않는다. 선택된 Pattern occurrence → instance → template → binding/target의 실제 stable ID를 전달하고, 기존 공통 편집기를 재사용한다. 두 도구에 별도 World 사본이나 독립 런타임을 만들지 않는다.

목표는 1관문과 2관문 둘 다 적어도 한 도구에서 편집할 수 있는 것이다. 양 도구를 사용할 경우 같은 정본을 공유하며 저장 후 변경이 서로 반영되어야 한다. 없는 기능을 “툴 제한”이라고 적고 끝내지 말고 필요한 편집 연결을 이번 C++ 구현 범위에 포함한다.

### 수정할 항목과 단위

| 선택 대상 | 사용자가 수정할 내용 | 보호 조건 |
|---|---|---|
| WORLD 발생 박스 | 시작 시각·길이 및 현재 형식이 지원하는 재생 설정 | 시퀀스 전역 시각과 motion/clip 로컬 시각 구분 |
| 소품 instance/target | 위치·회전·크기, transform 키 시각·값·표시 여부 | 전역 배치 보정과 시간별 동작 키를 구분하며 이중 변환 금지 |
| 지원되는 애니메이션 트랙 | 실제 모델에 있는 clip 선택, 시작/구간·속도·반복/마지막 포즈 등 저장 형식의 지원 항목 | 지원 안 되는 속성을 저장되지 않는 가짜 컨트롤로 노출하지 않음 |
| 여러 소품 묶음 | 묶음 전체 선택 및 개별 binding/target 선택 | 14묶음/130개 중 어느 소품인지 표시하고 나머지 target 보존 |

절대 좌표 키에 임의로 instance offset을 더하면 double transform이나 pivot 오류가 생길 수 있다. 현재 평가식을 먼저 확인하고, 사용자에게 전체 배치 보정과 선택 키 수정의 의미를 구분해서 제공한다. source 곡선과 사용자의 배치 보정을 보존할 방법은 기존 저장/평가 계약을 우선 사용한다. 별도 override가 정말 필요하면 실제 소비자·저장 왕복·실패 검증까지 닫고 미래용 속성만 추가하지 않는다.

gate2_intro_27s 안에 구워진 뼈 동작 자체는 transform 키 편집과 다르다. 이번 범위에 범용 뼈 애니메이션 편집기를 약속하지 않는다. 사용자는 소품의 배치/동작 트랙/재생 설정을 툴에서 수정하고, 굽힌 판의 뼈 동작 수정이 필요한 경우에만 원본 추출·선택 재쿠킹 절차를 명확히 구분한다. 재쿠킹/재생성으로 툴에서 저장한 사용자 보정이 사라지지 않도록 대상별 병합 또는 사전 diff·선택 적용 절차를 제공한다.

### 전체 재생과 편집의 연결

전체 연출을 Pause/Seek한 시각에서 소품을 선택한다. 단독 Preview로 전환하는 버튼과 전체 연출 편집을 구분한다. 편집 적용 후 기존 preview owner를 정리하고 같은 전체 시각을 재평가하거나 명시적으로 재생을 다시 시작할 수 있어야 한다. 편집기를 열었다는 이유로 다른 소품을 제거하거나 잘못된 기준점으로 옮기지 않는다. Preview at Character는 편의 기능으로만 남기고 저장 위치 전체 연출에는 적용하지 않는다.

재생 중 임의 live mutation을 요구하지 않는다. 안전한 Pause 또는 Stop → 편집/Apply → 동일 시각 재평가 → Play가 가능하면 된다. 단, 사용자가 매번 수십 개 리소스를 다시 찾아 수동으로 조립하거나 재빌드해야 하는 방식은 아니다. Apply는 미리보기, Save는 영구 저장이라는 차이와 dirty 상태를 UI에 표시한다.

### 저장과 인계 검증

1. Table 등의 OBJECT_RESOURCE와 motion은 기존 worldsequences 정본, 발생 박스는 기존 Sequence Composition에 저장한다. Save Animated Props는 실제 Deploy 배치에만 사용한다. Map Tool 편집을 지원하기 위해 Table을 Deploy로 복제하지 않는다.
2. 양 도구의 저장은 같은 검증·freshness·linked 동기화 계약을 재사용한다. 한쪽의 오래된 문서가 다른 쪽 Save나 미저장 draft를 덮어쓰지 않아야 한다. 잘못된 binding이나 실패한 Save/Reload는 기존 전체 상태를 보존하고 실패 대상을 보여 준다.
3. 소품 하나의 위치와 동작 키 시각을 각각 수정하여 Apply → 전체 Sequence 확인 → Save → Reload → 재실행 후 유지되는지 검사한다. 원래 값 복구도 확인한다. 여러 target 묶음에서는 한 target만 바뀌었는지, 다른 묶음과 카메라는 그대로인지 확인한다.
4. 필요한 WorldSequences/Area publisher 검증·게시 후 runtime 재생에서도 같은 저장값을 소비하는지 확인한다. 데이터 편집마다 C++ 재빌드를 요구하지 않는다. 기존 Save가 자동 게시하는 범위와 별도 게시가 필요한 범위를 실제 코드 기준으로 안내한다.
5. 자동 검사에는 선택 전달·저장 왕복·다중 target 보존·중복/잘못된 ID·저장 실패·외부 draft 충돌을 포함한다. UI 실행 및 최종 화면 판정은 사용자가 수행한다.

RESULT에는 `대상 → Workbench 또는 Map Tool의 정확한 편집 경로 → 바꿀 필드 → Apply/Save 버튼 → 정본 파일 → Reload/전체 Play` 표를 넣는다. 구현하지 않은 경로를 사용할 수 있다고 안내하지 않는다. 원본 일치 여부, 전체 재생, 직접 편집, 저장/재실행 유지, 사용자 검증을 분리하고 하나라도 미확인인 항목은 명시한다.
