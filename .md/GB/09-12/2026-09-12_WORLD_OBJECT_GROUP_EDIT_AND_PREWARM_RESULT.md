# World Object 그룹 편집과 카드 사전 준비 결과

## G00. 반영 범위

Box Detail에서 정확한 Object/Motion으로 이동하는 요청, Motion 그룹 개수·간격 편집, 연결 Collider/Logic
저장 갱신, 조커 카드 모델·clone 사전 준비를 C++에 반영했다. 구현 중 Object/Composition JSON,
Resources, 제품 exe/DLL/CSO를 저장하거나 배포하지 않았다. Client/UI는 실행·조작·캡처하지 않았다.
화면과 FPS 개선치는 아직 사용자 미확인이다.

## G01. Object/Motion 편집 연결

- Workbench는 `KOUKU_WORLD_OBJECT_EDIT_REQUEST`의 objectId와 motion instanceId를 one-shot으로
  MainApp에 전달한다. Box Detail의 `Edit This Motion`, `Edit Object`, 각 linked animation의
  `Edit Motion`이 실제 호출자다. 현재 생성 수를 함께 표시한다.
- `CWorldObjectTool::Open_ObjectMotion`은 실제 State 소유 ID를 검사한 뒤 기존 선택 경로를 사용한다.
  잘못된 ID는 현재 draft와 선택을 유지한다. 도구 열기로 자동 Save/Publish/Preview를 시작하지 않는다.
- 그룹 전체 위치/회전/스케일은 기존 Box Transform, Object 공통 크기는 Object, 개별 offset/Yaw/Delay와
  velocity/acceleration/자전/공전은 Motion이 소유한다. Box Scale은 그룹 간격과 이동량에도 적용된다.

## G02. 그룹 편집과 저장

`Group Layout`에 `Group count / Resize Group`, XYZ `Step offset`, `Step delay / Arrange Line`,
XYZ `Spacing multiplier / Scale Group Spacing`을 추가했다. 기존 Add/Dup/Del/Clear/Ring과 함께 저장
기준 row provenance를 유지한다. 기존 row의 stable Collider/Logic ID는 보존하고 새 복제만 새
occurrence ID를 발급한다. spacing은 모델 크기를 바꾸지 않는다.

`SynchronizeEmissionReferences`는 Pattern 및 독립 Sequence Composition의 WORLD Collider를 정확한
worldId + worldOccurrenceId로 찾아 새 emission index와 Delay에 대응하는 window로 갱신한다.
중간 삭제는 해당 Collider와 다른 소비자가 없는 Logic을 제거하고, 복제는 전용 Logic도 함께 만든다.
Result/Contact의 Motion, Contact target 및 NEXT chain도 참조 검사와 재publish 대상으로 포함한다.

명확하게 자동 대응할 수 없는 경우는 저장 전 모든 원본을 보존하고 이유를 표시한다. 여기에 여러
WORLD 중 어느 것인지 불명확한 Collider, 다른 row/hold/result target과 혼합된 Logic 의존성,
WORLD box 수명 밖 Delay, 명시 row가 없는 seeded 다중 Collider, 단일 Object 판정의 Count 변경이
포함된다. 임의 clamp나 연락되지 않은 Logic 복제는 하지 않는다.

Object Save는 기존 문서 serializer/load로 stage→readback하고 Composition은 기존
Parse_Text/Validate/Serialize로 재검사한다. 모든 교체 대상 파일의 원래 bytes를 저장 직전에 비교하며
부분 rename 실패는 바뀐 원본이 없을 때만 rollback한다. rollback 중 외부 편집이 발견되면 그 파일을
덮어쓰지 않고 recovery 파일을 남긴다. 열려 있는 Composition의 dirty/Publish 상태도 저장 전과
Map 적용 후 다시 확인한다.

Map publisher 성공 후 MainApp의 기존 owner가 깨끗한 Composition을 reload하고 필요하면
`Publish_AllPatterns`를 시작한다. source 저장, Map 적용, Pattern publication 시작·성공은 구분한다.
별도의 publisher 완료 검사는 실제 해당 owner가 보고하며 이 작업에서 실행하지 않았다.

## G03. 카드 사전 준비

추가 API는 `CWorldSequencePlayer::Prewarm_ObjectInstances(const std::string&, uint32_t,
const TARGET_SET&)`와 `TARGET_SET::objectPreparationOwner`다. 통합 담당이 Level 입장 및 저장 후
재로드 경로에서 정상 카드 `world.object.instance.kouku.card` 6개, 조커
`world.object.instance.kouku.joker_card` 1개를 준비하도록 연결했다. 입장은 통합 담당이,
Reload_WorldObjectRuntime은 이 슬라이스가 반영했다. 재로드는 두 준비를 각각 시도하며 실패 경고를
status와 로그로 남긴 뒤 성공한 문서 load의 true를 유지한다. Object Tool도 그 경고를 보존한다.

`Prepare_ObjectResources`는 동일 Area/revision, model/preScale/animated, diffuse/source material/profile,
map binding, device/context/catalog의 준비된 모델만 공유한다. `Apply_Objects`는 그 모델을 실제로
소유한 풀의 숨은 clone을 빌리고, 부족한 추가 occurrence는 기존 Prototype/Clone/Layer 경로를 쓴다.
사전 준비 자체는 gameplay 판정이나 표시를 시작하지 않는다.

Stop/완료에서 `CWorldSequenceObject::Reset_ForReuse`로 rest local bone pose와 sample 상태를 복원해
풀로 반환한다. 따라서 첫 6+1 후 반복 Pattern 13도 같은 clone을 사용한다. 모델을 다시 clone하지
않으며 shader component와 material 입력을 유지한다. 실패한 render/reset 객체는 반환하지 않고
원인을 보존한다. owner Clear/문서 교체는 shared return token을 비활성화한다. 남아 있는 예전
revision의 child가 나중에 종료되어도 이미 없어진 owner를 참조하거나 새 풀에 섞이지 않는다.

## G04. 실행한 검증과 한계

| 검사 | 실제 결과 |
|---|---|
| 최초 Box/그룹 C++ 검사 | WorldObjectTool, KoukuSaydonActionWorkbench, MainApp의 MSVC `/Zs` exit 0 |
| Prewarm C++ 검사 | WorldObjectTool, WorldSequencePlayer, WorldSequencePlayer_Objects, WorldSequenceObject의 MSVC `/Zs` exit 0 |
| 최종 변경 C++ 검사 | WorldObjectTool/MainApp 재검사 및 재준비 연결 후 WorldObjectTool/Level_KakulSaydonArena_WorldObjects 재검사 모두 `/Zs` exit 0 |
| 실제 함수 fixture | 실제 SynchronizeEmissionReferences와 실제 문서 types/Find_Template/Find_Instance를 사용, 현재 Pattern 18/19의 저장 row로 14개 사례 통과 |
| 실제 파일 교체 fixture | 실제 CommitAuthoringWrites/WriteAuthoringStage/ReadSource로 commit, 외부 수정 거부, 두 번째 파일 lock에 의한 부분 rename 실패 후 rollback 3개 통과 |
| diff/encoding | 관련 파일 `git diff --check` exit 0, 11개 C++ UTF-8 no BOM·CRLF 유지 확인 |
| JSON/프로젝트 등록 | 이 슬라이스는 JSON/XML 변경과 새 C++ 파일이 없어 해당 신규 parse/등록 없음 |

14개 참조 사례는 동일 문서 보존, 중간 삭제, 복제, Delay 이동, 혼합 Logic 거부, hold 참조 거부,
불명확한 WORLD 거부, 역순 remap, 1개로 축소, 24개로 확장, WORLD 수명 밖 Delay 거부,
seeded 다중 Collider 거부, Result 단일 Motion과 Contact 단일 target 개수 거부다.

out-only 결과는 `out/WorldObjectGroups20260912/`의 `syntax.log`, `prewarm_syntax.log`,
`final_syntax.log`, `reload_syntax.log`, `fixture_compile.log`, `fixture_run.log`, `emission_fixture.cpp`다. fixture의
생성기에서 처음에는 WORLD 수명 fields를 생략했던 입력이 새 수명 검사에 걸렸고, 실제 원본의
startMs/durationMs를 복사한 뒤 통과했다. 제품 데이터를 보정해서 통과시킨 검사가 아니다.
MSVC의 기존 EngineSDK CP949 헤더 C4828 경고는 유지되어 있다.

전체 제품 link/build, 실제 Object Save→publisher 종단 실행, GPU clone 준비/반복 반환 수치,
Client 화면·FPS는 이 하위 작업에서 실행하지 않았다. 사용자가 현재 Visual Studio/Client를
사용하고 있으므로 통합 담당과 조율한 out-only 검사만 수행했다.

## G05. 원본 시퀀스 배경의 중복 model 준비 제거

Source Sequence 후보의 actor별 고유 resource 266개는 실제 model/preScale/animated/diffuse/source
material/profile/map bindings 입력으로 묶으면 72종이다. 기존 objectId별 cache 경로에 동일 입력
검색을 추가해 준비된 CModel prototype을 공유한다. 이 숫자는 source 입력 집계이며 측정한
GPU 생성 횟수나 FPS 개선 수치는 아니다.

`WorldSequencePlayer_Objects.cpp::Same_ObjectModelInputs`는 위 7개 입력을 exact 비교한다.
`Find_PreparedObjectModel`은 자기 cache의 exact ID를 먼저 보고, 다른 ID의 동일 입력을 찾는다.
`Find_SharedObjectModel`은 같은 Area/revision의 Level owner만 조회한다. device/context/catalog
identity도 일치해야 한다. `Remember_SharedObjectModel`은 준비와 해당 clip 검증에 성공한 model을
owner 문서의 실제 일치 resource ID에 공유 저장한다. owner cache가 문서 교체/Clear에서 사라지는
수명은 유지한다. 다른 owner entry는 덮어쓰지 않고, local exact ID의 render target identity가
바뀌면 reload가 필요하다는 실패 이유를 반환한다.

objectId/표시명/anchor/placement scale/Motion은 CModel 준비 입력이 아니므로 각 객체에 남는다.
`WorldSequenceObject::Initialize`는 계속 prototype의 Clone을 만든다. `CModel` copy constructor는
Bone과 Animation을 각각 Clone한다. Animation은 자기 `m_fCurrentTrackPosition`과
`m_iLeftKeyFrameIndices`를 가지며 공유 Channel은 key 자료를 읽고 호출자가 준 index와 Bone만
갱신한다. 이 계약은 소스에서 확인했으며 GPU animation 실행 검증으로 기록하지 않았다.
Joker pool은 exact object ID와 owner prototype 우선 조회를 유지한다.

추가 수정 파일은 WorldSequencePlayer.h와 WorldSequencePlayer_Objects.cpp다. 실제 변경한
4개 함수 본문을 out fixture에 추출하고 실제 WORLD_SEQUENCE_OBJECT_RESOURCE/Profile/Binding
types로 17개 입력 필드 변화, local/owner 다른-ID 재사용, exact-ID prototype 우선, 3개 render
target·Area/revision 격리, owner 문서에 없는 입력·다른 revision·null model 거부, 기존 owner entry
보존을 검사해 PASS했다. document와 GPU handle은 이 CPU fixture의 최소 mock이며 CModel::Create
또는 GPU clone 비용을 재현한 테스트는 아니다.

WorldSequencePlayer_Objects.cpp와 WorldSequencePlayer.cpp의 MSVC `/Zs` exit 0, fixture compile/run
exit 0, 두 C++ 파일 UTF-8 no BOM·CRLF 유지, `git diff --check` PASS다. 증거는
`out/WorldObjectModelReuse20260912/`의 `syntax.log`, `fixture_compile.log`, `run.log`, `fixture.cpp`,
`make_fixture.py`, `extraction.json`, `verification.json`이다. 제품 build/link나 사용자 exe 교체는
수행하지 않았다. 실제 화면 시작 시점과 반복 재생의 FPS는 사용자 확인으로 남는다.

## G06. 실제 변경 파일과 후속 확인

- `Client/Public/WorldObjectTool.h`, `Client/Private/WorldObjectTool.cpp`
- `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp`
  (요청 소비와 Render_WorldBoxDetails; 기존 카메라/다른 담당 변경 유지)
- `Client/Private/MainApp.cpp` (요청 전달·linked Save callback·게시 owner 구성)
- `Client/Public/WorldSequencePlayer.h`, `Client/Private/WorldSequencePlayer.cpp`,
  `Client/Private/WorldSequencePlayer_Objects.cpp`
- `Client/Public/WorldSequenceObject.h`, `Client/Private/WorldSequenceObject.cpp`
- `Client/Private/Level_KakulSaydonArena_WorldObjects.cpp` (Reload_WorldObjectRuntime만 추가 소유;
  통합 담당의 Make_WorldSequenceTargets 및 다른 변경 유지)
- 대응 IMPLEMENTATION_PLAN과 이 RESULT

사용자 확인 경로는 F1 → Sequencer 또는 Complete Play의 World box → Box Detail → Edit This Motion →
Object Tool의 Physics / Motion / Emission → Group Layout이다. Save 후 Map/Pattern publication
상태를 확인하고 Pattern 18/19의 Collider/Logic 개수 및 첫 번째·반복 Pattern 13 표시 시점을
확인해야 한다. 이 항목을 자동 visual PASS로 기록하지 않았다.
