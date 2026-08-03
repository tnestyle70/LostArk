# 2026-08-03 카메라 툴 계획 (참조 영상 09 패리티 + 발전형)

참조 이미지: `C:\Users\user\Desktop\툴\09_CameraTool_CutList_PosList.png`
— Camera CutList(컷 이름/생성/삭제/저장 목록) + Camera PosList(더미 타겟, LookAt,
Capture, 재생 Start/Stop) + Camera Data Files(Save/Load, `Bern.data`).

작성 모드: STRUCTURE_FIRST. 구현 코드는 싣지 않는다.

## 1. 현재 체크포인트와 이번 단계의 완료 조건

현재 상태(실측):

- 카메라 관련 코드는 `Engine::CCamera`(추상, 투영값 protected·getter 없음)와
  `Client::CCamera_Free`(follow/free 이중 모드, F6/Tab, 지수 스무딩 follow) 둘뿐.
- 컷/경로/셰이크/카메라 데이터 파일은 저장소 전체에 **전무**(grep 검증됨).
  `CCameraPresetDocument`는 07-31 PLAN 문서 2건에만 존재하는 미구현 설계다.
- 레벨마다 `Layer_Camera`에 `CCamera_Free` 정확히 1대가 클론되고, 레벨 Update가
  매 프레임 `Set_GameplayInputEnabled(camera->Is_FollowEnabled())`로 free 모드
  중 게임플레이 명령을 차단한다. MapTool CAMERA 모드가 이미 이 카메라를
  ImGui에서 조작하는 선례(`Render_CameraPanel`, `Find_AssetTestCamera`)가 있다.
- 보간 유틸은 엔진에 없다. DirectXMath의 `XMVectorCatmullRom`은 사용 가능하나 미사용.

완료 조건(이번 슬라이스):

1. Lobby의 Test 버튼으로 `LEVEL::DEVELOPMENT`에 진입하고 F1 허브에서 MapTool의 CAMERA 모드를 선택한다.
2. 현재 카메라 포즈(위치+시선)를 Capture → 이름 있는 컷(cut) 안에 포즈 목록으로 축적.
3. 더미 룩앳 포인트를 배치(픽킹)하고 "LookAt Dummy"로 모든 캡처 포즈의 시선을 재계산.
4. Start/Stop으로 컷 재생: 포즈 사이 Catmull-Rom 보간, 재생 중 게임플레이 명령 차단.
5. `Data/Cameras/<AreaId>/CameraCuts.json` 저장/로드(stable cutId), 재로드 후 동일 재생.
6. Server+Client 실제 실행 수동 검증 + ProjectAudit 통과. Client 내부 smoke harness는 추가하지 않는다.

발전형 후속(이번 슬라이스에 섞지 않음): FOV/롤 키 애니메이션(Engine 공개 헤더 확장
필요), 이징 곡선 선택, 셰이크, 인게임 컷씬 트리거(서버 CameraCueId 의미 계약 —
북극성.md 17.2절의 "서버는 의미만, 연출은 클라"), 분할 미리보기.

## 2. 전체 수직 흐름 한 줄

CCamera_Free 포즈 캡처 → CCameraCutDocument(JSON, Data/Cameras) → CCameraCutEditor 패널
→ CCamera_Free 재생 모드(Catmull-Rom) → 레벨의 기존 follow 게이트가 입력 차단.

## 3. 파일별 존재 이유와 책임 지도

### 신설: `Client/Public/CameraCutDocument.h` / `Client/Private/CameraCutDocument.cpp`

```text
파일이 존재하는 이유: 컷 데이터의 저장 계약을 툴 UI/카메라 재생과 분리해 소유할
  파일이 없다. MapTool의 CWorldGameplayDocument와 같은 계층이다.
한 문장 역할: Data/Cameras/<AreaId>/CameraCuts.json 한 파일의 파스/검증/직렬화와
  메모리 상 컷 목록을 소유한다.
소유하는 상태: areaId, revision, 컷 배열(각 컷: cutId(stable), 표시 이름, 포즈
  배열(위치3 + 시선점3), 구간 시간(초), 루프 여부, enabled), 더미 포인트(위치3,
  반지름 — 표시용), 로드 성공 여부.
소유하지 않는 상태: ImGui 상태, 카메라 포인터, D3D 리소스, 재생 진행도.
생성자와 파괴자: 기본 생성(빈 문서). 파일 소유 리소스 없음.
생성 시점: CCameraCutEditor 생성 시 멤버로.
파괴 시점: CCameraCutEditor와 함께.
직접 호출자: CCameraCutEditor뿐(편집/저장/로드, 재생 시작 시 포즈 스냅샷 추출).
  CCamera_Free는 이 문서를 호출하지 않으며 Start_CutPlayback 인자로 받은
  스냅샷만 소비한다(수명 분리 — _DEBUG 툴이 소유한 문서에 제품 카메라가
  결합되지 않게).
직접 피호출자: CProjectDataRoot::Resolve(경로), JSON 파서(기존 Client가 쓰는 방식).
입력 데이터의 출처: 툴 UI의 편집 명령 + 디스크 JSON.
출력 데이터의 소비자: 재생 모드(Start 시 복사된 포즈 스냅샷 경유), 디스크(Save).
실패가 전달되는 경로: Load/Save가 HRESULT + 상태 문자열 반환 → 툴 패널에 표시.
  부분 파스 상태로 커밋하지 않는다(스테이징 후 교체, 실패 시 기존 내용 유지).
```

H 계약(자연어, 핵심만):

```text
함수명: Load(areaId)
호출 시점과 스레드: 툴의 Load 버튼, 메인 스레드. 매 프레임 호출 금지.
성공 조건: schema == "lostark.camera-cuts", formatVersion == 1, 모든 cutId가
  ^[A-Za-z0-9_.-]{1,128}$, cutId 중복 없음, 포즈 값 유한수.
실패 조건: 위 위반 시 기존 메모리 문서 유지 + 사유 문자열.

함수명: Save(areaId)
외부 부작용: .tmp 기록 후 교체(원자적 쓰기 — Manage-ResourcePack의
  Write-JsonAtomic과 같은 발상. C++ 쪽도 임시 파일에 완전히 기록한 뒤
  `MoveFileExW(MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`로 교체한다.

함수명: Add_Cut / Remove_Cut / Append_Pose / Clear_Poses / Set_DummyPoint
변경하는 멤버: 컷 배열 + revision 증가(Mark_Edited 패턴).
유지해야 할 불변식: cutId 유일성. 포즈 수 상한(예: 컷당 256)을 정하고 초과를
  실패로 처리한다(무한 축적 방지).
```

JSON 스키마 초안(문서로만 — 필드는 publisher 없이 Client가 직접 소비):

```text
schema: "lostark.camera-cuts", formatVersion: 1, areaId, revision,
dummyPoint { position[3], radius }, cuts[]:
  { cutId, label, loop, durationPerSegment, enabled, poses[]: { position[3], lookAt[3] } }
```

### 수정: `Client/Public/Camera_Free.h` / `Client/Private/Camera_Free.cpp`

```text
이번 변경의 한 문장 역할: follow/free에 세 번째 모드 PLAYBACK을 추가한다.
왜 새 CCamera 파생이 아닌가: 4개 레벨 + MapTool + 스모크가 전부 Layer_Camera의
  0번 오브젝트를 CCamera_Free로 dynamic_pointer_cast하고 Is_FollowEnabled()를
  폴링한다. 새 클래스를 꽂으면 그 캐스트/게이트 전부를 고쳐야 한다. 기존
  계약을 보존하는 최소 변경이 모드 추가다.
소유하는 상태(추가): 재생 상태(포즈 스냅샷 벡터 복사본, 구간 시간, 루프, 진행 시간,
  재생 중 여부). 문서 포인터를 보관하지 않는다(수명 분리 — 툴이 닫혀도 재생은
  스냅샷으로 완결).
게임플레이 차단: 재생 중 Is_FollowEnabled()가 false를 반환하면 기존 레벨 게이트가
  자동으로 명령을 차단한다. 새 게이트를 만들지 않는다.
```

H 계약(자연어):

```text
함수명: Start_CutPlayback(포즈 스냅샷, 구간 시간, 루프)
존재 이유: 툴이 컷 재생을 시작시키는 유일한 입구.
입력: 포즈 2개 미만이면 실패 반환(보간 불가).
변경하는 멤버: 재생 상태 일체 + follow 비활성.
외부 부작용: 없음(파이프라인 반영은 기존 Update_PipeLine 흐름).

함수명: Stop_CutPlayback
출력: 정지 시 카메라를 마지막 재생 포즈에 둔다(스냅 백 금지 — 편집 흐름 유지).

함수명: Is_CutPlaying
호출자: 툴 패널(버튼 상태), Update_Shortcuts(재생 중 F6/Tab/WASD 무시 판단).
```

CPP 내부 흐름(재생 갱신 — Priority_Update 내 분기 추가):

```text
Priority_Update 진입
-> Update_Shortcuts (재생 중이면 F6/Tab 토글 무시, Esc 상당의 정지는 툴 버튼만)
-> 재생 중인가?
   예: 진행 시간 += dt
       -> 현재 구간 index = 진행/구간시간, t = 소수부
       -> XMVectorCatmullRom(포즈[i-1], 포즈[i], 포즈[i+1], 포즈[i+2], t)
          (양 끝은 끝점 복제로 클램프; 위치와 lookAt 각각 보간)
       -> Set_State(POSITION) + LookAt
       -> 끝 도달: 루프면 wrap, 아니면 재생 종료 플래그
   아니오: 기존 follow/free 분기
-> Update_PipeLine (기존 그대로)
반복문 설명: 구간 탐색은 인덱스 계산으로 하고 매 프레임 벡터 순회를 하지 않는다.
  힙 할당 없음(스냅샷은 Start 시 1회 복사).
```

### 신설: `Client/Public/CameraCutEditor.h` / `Client/Private/CameraCutEditor.cpp`

```text
파일이 존재하는 이유: CMapTool은 이미 3,000줄을 넘었으므로 CAMERA 하위 기능의 문서·선택·
  재생 command 상태를 별도 owner로 분리한다. 독립 허브 툴이나 새 scenario는 아니다.
한 문장 역할: CameraCutDocument를 편집하고 CCamera_Free에 재생을 명령한다.
소유하는 상태: CCameraCutDocument, 선택 cutId, 이름 입력 버퍼, 상태 문자열.
소유하지 않는 상태: 카메라(weak 획득만), D3D 리소스, 재생 진행도.
생성 시점: CMapTool::Initialize가 `_DEBUG` authoring helper로 생성.
파괴 시점: CMapTool과 함께.
직접 호출자: CMapTool::Render_CameraPanel과 CAMERA 모드의 world interaction update.
직접 피호출자: CameraCutDocument, CCamera_Free(공개 setter만), CGameInstance
  (픽킹 — 더미 포인트 배치), CProjectDataRoot.
입력 데이터의 출처: ImGui 위젯 + 월드 클릭(더미 배치 시).
실패가 전달되는 경로: 문서 Load/Save 실패 문자열을 패널에 표기(모달 금지).
```

CPP 내부 흐름(Render 핵심):

```text
Render(camera, areaId) 진입
-> 현재 레벨이 DEVELOPMENT가 아니면 안내만 표시
-> 카메라 획득: Get_GameObject(level, "Layer_Camera", 0) -> CCamera_Free 캐스트
   (MapTool::Find_AssetTestCamera와 동일 패턴, 매 프레임 weak 획득)
-> CutList 패널: 이름 입력 + New Cut / Delete / 목록 Selectable
-> PosList 패널: [Capture] 현재 카메라 CTransform에서 위치 + 위치+정규화LOOK을
   lookAt 점으로 저장 -> Append_Pose
   [LookAt Dummy] 선택 컷 전체 포즈의 lookAt을 더미 포인트로 재기록
   [Delete] 선택 포즈 제거
   [Start] Document의 선택 컷 -> 포즈 스냅샷 -> camera->Start_CutPlayback
   [Stop] camera->Stop_CutPlayback
-> Data Files 패널: Save / Load (+ 상태 문자열)
-> 더미 포인트 배치 모드가 armed면: 월드 클릭 처리는 Update에서
   (WantCaptureMouse 확인 + Try_PickPlacementPosition식 픽킹/Y=0 폴백)
```

더미 포인트 시각화: `CGameInstance::Add_DebugComponent` 경로가 컴포넌트 단위라서,
MapTool의 `NAVIGATION_RENDER_RESOURCES`(PrimitiveBatch 자체 와이어 드로우) 패턴을
재사용해 구를 그린다. 참조 영상의 "DummyCollider"는 콜라이더가 아니라 룩앳
타겟 표시이므로 CCollider를 끌어오지 않는다.

### 수정: `Client/Public/MapTool.h` / `Client/Private/MapTool.cpp`

```text
추가 owner: unique_ptr<CCameraCutEditor>. 헤더는 forward declaration으로 결합을 제한한다.
Initialize: CameraCutEditor 생성 실패를 MapTool 초기화 실패로 전달한다.
Render_CameraPanel: 기존 follow/free controls를 유지하고 editor Render를 호출한다.
Update_WorldInteraction: CAMERA 모드이며 editor가 dummy placement를 armed한 경우에만
  기존 WantCaptureMouse/world pick 경계를 통과시켜 editor command로 전달한다.
금지: CMainApp에 새 DEBUG_TOOL enum·멤버·render 분기를 추가하지 않는다.
```

## 4. 연결 파일과 의존성·결합 규칙

```text
호출 흐름:
CMainApp -> CMapTool -> CCameraCutEditor -> CCameraCutDocument
                                      -> CCamera_Free(Start/Stop/Capture용 getter)
CCamera_Free(재생) -> CPipeLine(기존 Update_PipeLine 경유)

데이터 흐름:
사용자 캡처 -> CameraCutDocument(메모리) -> Save -> Data/Cameras/<AreaId>/CameraCuts.json
JSON -> Load -> Document -> Start 시 포즈 스냅샷 -> CCamera_Free 재생 -> 화면
```

- 툴은 CPipeLine을 직접 만지지 않는다. 카메라 공개 setter만 사용.
- 문서는 ImGui를 include하지 않는다(Effect_Runtime의 "에디터는 D3D를, 런타임은
  ImGui 상태를 소유하지 않는다" 분리 원칙과 동일).
- 서버/Shared는 이번 슬라이스에서 건드리지 않는다. 컷 데이터는 클라 연출 전용이며
  서버 truth가 아니다(북극성.md 17.2 경계).
- CameraCutEditor -> CNetworkManager, Change_Level 호출 금지.

## 5. 사용자가 먼저 작성할 범위

1. CameraCutDocument H/CPP (파스/검증/직렬화 — JSON 라이브러리는 기존 Client
   JSON 소비 방식과 동일하게)
2. CCamera_Free PLAYBACK 모드(멤버 + Start/Stop + Priority_Update 분기)
3. CameraCutEditor H/CPP 패널 골격(Capture/목록/Start/Stop/Save/Load)
4. 등록 작업 일체(아래 8절)

권장 작성 순서는 1 → 2 → 3. 2까지 되면 하드코딩 포즈 2개로 재생을 먼저 검증할
수 있다(문서 없이도 카메라 검증 가능 — 최초 실패 지점을 좁히는 순서).

## 6. 검토 Breakpoint와 관찰값

```text
Capture 클릭
-> CameraCutEditor: CTransform::Get_State(POSITION) 값, LOOK 정규화 값
-> Append_Pose 후 컷의 포즈 count, revision 증가 확인
Start 클릭
-> Start_CutPlayback: 스냅샷 size, 구간 시간
-> CCamera_Free::Priority_Update 재생 분기: index/t 값이 프레임마다 단조 증가하는가
-> Update_PipeLine 후 CPipeLine::Get_CamPosition이 보간 궤적을 따르는가
-> CLevel_Development::Update: Set_GameplayInputEnabled(false) 확인(재생 중)
Save -> Load 왕복
-> Load 후 컷/포즈 count 동일, Start 재생 궤적 동일(눈 확인 + 좌표 로그)
실패 주입
-> cutId 중복 JSON 로드 -> 실패 문자열 표시 + 기존 문서 유지 확인
```

## 7. 프로젝트 설정·등록

개요 문서 5절의 “기존 Level 내부 패널” 유형을 적용한다.

- 신규 `CameraCutDocument.h/.cpp`, `CameraCutEditor.h/.cpp`를 물리 경로와
  `Client.vcxproj/.filters`에 등록한다.
- 기존 Test(`LEVEL::DEVELOPMENT`)의 MapTool CAMERA 모드를 재사용한다.
- `CLIENT_SCENARIO`, `LevelCatalog.json`, `ClientLaunchOptions`, 독립 smoke scenario는
  CharacterSelect 수직 슬라이스에서 삭제된 상태를 전제로 하며 다시 추가하지 않는다.
- MainApp에는 새 Level 전환을 넣지 않고 기존 `DEBUG_TOOL::MAP` lazy 생성 경로만 사용한다.
- 신규 `Data/Cameras/<AreaId>/CameraCuts.json`을 커밋하는 시점에 `96.DataFiles`
  None 항목 1:1 등록(오디트 `projects.data-source-visibility`)
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`에 Cameras 레이어 행 추가 — Area별 선택적
  `Data/Cameras/<AreaId>/CameraCuts.json`, 파일이 없으면 빈 문서로 시작, `_DEBUG`
  툴 전용 소비, publisher/런타임 로더 불참. Area 데이터 계약 변경은 이 가이드
  갱신까지가 한 변경 단위다(AGENTS.md 정본 문서 표)
- Engine 헤더는 이번 슬라이스에서 **무변경**(FOV getter는 후속 단계로 미룸) —
  UpdateLib 불필요. FOV 키를 넣는 후속 단계에서만 Camera.h 공개 변경 + UpdateLib.

## 8. 빌드·실행 검증과 다음 단계

```text
Client Debug x64 빌드
Server+Client 실행 -> Lobby Test -> F1 MapTool -> CAMERA -> Capture 4회 -> Start
-> 궤적이 4점을 지나며 부드럽게 보간되는지, WASD/우클릭 이동이 죽어 있는지 확인
-> Stop -> free 모드 복귀, F6 정상
Save -> Client 재시작 -> Load -> 동일 재생
ProjectAudit 통과, git diff --check
```

다음 단계: 재생 이징/FOV 키(Engine 공개 확장), 셰이크 모듈, 서버 CameraCueId
연출 트리거는 각각 별도 PLAN으로 분리한다.
