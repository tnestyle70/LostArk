# LostArk 함수·Breakpoint·제1원리 추적 훈련 계획

작성일: 2026-07-30  
대상 저장소: `C:/Users/user/Desktop/LostArk`  
개인 훈련 규칙: `C:/Users/user/Desktop/LostArk/.md/계획서작성규칙.local.md`  
기반 핸드북: `C:/Users/user/Desktop/LostArk/.md/GB/07-30/2026-07-30_ANY_CODEBASE_FIRST_PRINCIPLES_HANDBOOK.md`

---

## 1. 훈련 목표와 완료 기준

### 1.1 목표

현재 LostArk 코드를 대상으로 다음 능력을 실제 디버거 증거와 함께 만든다.

1. 함수의 호출자와 다음 소비자를 양방향으로 추적한다.
2. 반환값과 멤버 변경, 출력 인자, 외부 시스템 부작용을 구분한다.
3. `unique_ptr`, `shared_ptr`, raw pointer/reference의 owner와 생명 주기를 설명한다.
4. `vector`, `list`, `map`, 반복문의 실제 복잡도를 현재 원소 수와 호출 빈도까지 계산한다.
5. C++ 문법을 정의로 암기하지 않고 현재 코드에서 역할과 위험을 설명한다.
6. CPU 함수가 GPU 렌더링 결과로 이어지는 경계를 추적한다.
7. Breakpoint, Call Stack, Watch로 예측을 검증한다.
8. 함수 하나를 정해진 분석 양식과 한 문장으로 압축한다.

### 1.2 전체 완료 기준

다음 조건을 모두 만족하면 1차 프레임워크 추적 훈련을 완료한다.

- 10개 라운드 중 8개 이상을 8/10점 이상으로 통과한다.
- `wWinMain -> Update -> Object -> Animation -> Renderer -> GPU` 흐름을 소스 없이 그린다.
- Prototype 원본과 Clone 인스턴스의 owner를 혼동하지 않는다.
- 함수의 `return`과 숨은 부작용을 각각 한 개 이상 실제 코드로 설명한다.
- `map/list/vector` 사용 지점 세 곳의 시간 복잡도와 실제 비용을 설명한다.
- 실패 하나를 선택해 `최초 실패 -> 반환 전파가 끝나는 곳`까지 추적한다.
- 최종 구두 설명에서 파일명보다 데이터, 수명, 호출 시간, 경계, 증거를 먼저 말한다.

### 1.3 훈련 원칙

```text
예측
-> Breakpoint
-> Call Stack/Watch 실측
-> 함수 분석 양식 작성
-> 복잡도·수명 계산
-> 한 문장 압축
-> 피드백
-> 필요 시 같은 함수 재도전
```

정답을 읽고 코드를 끼워 맞추지 않는다. 각 라운드의 답은 사용자가 먼저 제출한 뒤 공개한다.

---

## 2. 프로젝트 수직 흐름과 라운드 지도

```text
R1  Windows main loop와 DeltaTime
    wWinMain -> CMainApp::Update -> CGameInstance::Update_Engine

R2  프레임 스케줄과 Engine facade
    Picking/Input -> Priority -> Pipeline/Frustum -> Update/Late -> Level

R3  Level/Layer 자료구조와 반복 비용
    Object_Manager -> map<LayerTag, Layer> -> list<GameObject>

R4  Prototype/Clone과 공동 소유권
    Add_GameObject_to_Layer -> Clone_Prototype -> Clone -> Layer

R5  Valtan/Body PartObject 조립과 raw pointer 수명
    CValtan::Initialize -> Ready_PartObjects -> Add_PartObject -> CBody_Valtan

R6  Animation 알고리즘
    Body::Update -> Model::Play_Animation -> Animation -> Channel -> Bone

R7  Renderer와 CPU/GPU 경계
    Body::Late_Update -> Add_RenderObject -> Draw -> Body::Render -> Mesh/Shader

R8  Binary asset 변환과 검증
    CModel::Initialize_Prototype -> Ready_BinaryModel -> CWModelDecoder -> GPU buffer

R9  MapTool 상태 머신과 자료구조
    Palette -> ARMED -> Picking -> record -> Clone -> vector placement

R10 Save/Load transaction과 Navigation 비교
    parse -> validate -> stage -> commit/rollback, Cell/Neighbor 이동 판정
```

라운드는 이름 암기가 아니라 다음 여섯 축을 한 단계씩 추가한다.

| 라운드 | 데이터 | 변환 | 소유권·수명 | 실행 시간 | 경계 | 증거·비용 |
|---|---|---|---|---|---|---|
| R1 | timer/message | time -> frame | MainApp | main thread | Win32/Engine | hit count, dt |
| R2 | subsystem state | frame update | GameInstance | 매 frame | facade/subsystem | 호출 순서 |
| R3 | layers/objects | 전체 갱신 | Layer | 3 pass/frame | manager/layer | L, O 복잡도 |
| R4 | prototype/clone | 원본 -> instance | unique/shared | spawn 시 | manager/object | ref count/Call Stack |
| R5 | state/matrix | parent -> part | owner/borrow | clone/update | container/part | pointer validity |
| R6 | key/bone | 보간/행렬 | Model clone | 매 frame | CPU pose | C,K,B 비용 |
| R7 | render item | queue/draw | frame shared ref | late/render | CPU/GPU | queue/mesh/draw |
| R8 | file sections | decode/validate | prototype resource | load 시 | file/GPU | version/count |
| R9 | catalog/record | pick/place | tool/layer | event/frame | ImGui/world | state/vector 비용 |
| R10 | document/cell | stage/commit/path | stable ID | save/load/move | disk/runtime | rollback/invariant |

---

## 3. Breakpoint·Call Stack·Watch 실측 절차

### 3.1 Visual Studio 준비

```text
Configuration: Debug
Platform: x64
Startup Project: Client
```

기본 단축키:

```text
F9          Breakpoint 설정/해제
F5          디버깅 시작 또는 Continue
F10         Step Over
F11         Step Into
Shift+F11   Step Out
Shift+F5    디버깅 종료
```

### 3.2 라운드 시작 전 기록

```text
멈출 것으로 예상하는 함수:
최초 Call Stack 예상:
프레임당 예상 Hit 수:
관찰할 입력값:
변경될 것으로 예상하는 멤버:
다음 호출 함수:
반환값 소비자:
```

### 3.3 멈춘 직후 확인 순서

1. 현재 Thread가 main thread인지 worker인지 확인한다.
2. Call Stack을 아래에서 위로 읽어 진입점부터 현재 함수까지 적는다.
3. `this`가 null이 아닌지와 실제 runtime type을 확인한다.
4. 입력 인자를 Watch에 넣는다.
5. 분기에 사용되는 멤버를 Watch에 넣는다.
6. 컨테이너의 `size()`와 현재 index/key를 적는다.
7. Step Over 전 값을 기록하고 실행 후 다시 기록한다.
8. 반환 직전에 반환값의 의미를 말한다.
9. Step Out 후 호출자가 반환값을 받거나 버리는 코드를 확인한다.

### 3.4 Breakpoint 사용 구분

```text
일반 Breakpoint
함수에 실제로 진입하는지 확인

조건부 Breakpoint
특정 state, ID, index, nullptr일 때만 정지

Hit Count
프레임당 또는 object당 호출 횟수 검증

Data Breakpoint
특정 메모리 값의 마지막 작성자 추적, 객체 수명과 주소가 안정적일 때 사용
```

### 3.5 Watch 기록 최소 항목

```text
this
입력 인자 전부
현재 state/index
주요 pointer와 null 여부
container.size()
반환 직전 결과
변경 전/후 값
```

### 3.6 디버거 사용 시 주의

- Property/function evaluation이 상태를 바꾸는 함수라면 Watch에서 함부로 호출하지 않는다.
- 매 프레임 함수에 무조건 breakpoint를 걸어 정지 폭탄을 만들지 않는다. 조건과 hit count를 쓴다.
- 최적화된 Release에서는 변수 제거와 코드 재배치가 생길 수 있으므로 첫 훈련은 Debug로 한다.
- 디버거에서 보인 pointer 주소는 저장 가능한 ID가 아니다.
- `shared_ptr` use count는 관찰 과정의 임시 복사로 달라질 수 있으므로 owner 관계를 코드로 함께 확인한다.

---

## 4. 함수 분석 양식

각 라운드 답변은 다음 양식을 사용한다.

```text
[함수 분석]

함수명:
클래스:
정의 파일과 줄:
호출자:
Call Stack 실측:
호출 시점·스레드·프레임당 횟수:

입력:
입력의 소유자:
읽는 멤버:
변경하는 멤버:
변경하는 외부 시스템:

내부 호출:
반환형:
반환값의 의미:
반환값을 받는 함수:
반환값을 무시하는가:

객체 소유권 변화:
필수 생명 주기 조건:

사용 자료구조와 선택 이유:
시간 복잡도:
공간 복잡도·할당:
프레임당 총비용 추정:

실패 조건:
실패 전파 경로:
유지되어야 하는 불변식:

예측과 실측의 차이:
한 문장 요약:
```

모든 칸을 억지로 채우지 않는다. 해당 함수에 소유권 변화나 실패 반환이 없다면 `없음`이라고 쓰고,
왜 없는지 설명한다.

---

## 5. 라운드별 질문과 증거 기록 항목

## R1. Windows main loop와 DeltaTime

### 대상 함수

- `Client/Default/Client.cpp:31` `wWinMain`
- `Client/Private/MainApp.cpp:102` `CMainApp::Update`
- `Engine/Private/GameInstance.cpp:98` `CGameInstance::Update_Engine`

### Breakpoint

```text
Client.cpp:118   frame threshold 조건
Client.cpp:122   pMainApp->Update 호출
MainApp.cpp:102  CMainApp::Update 진입
MainApp.cpp:127  Update_Engine 호출
GameInstance.cpp:98 Update_Engine 진입
```

### Watch

```text
fTimeAcc
msg.message
fTimeDelta
this
m_pInput_Device.get()
m_pObject_Manager.get()
```

### 질문

1. `f32_t fTimeAcc = {};`에서 `{}`는 어떤 값을 만드는가?
2. `PeekMessage`가 false여도 game update 후보 코드까지 내려오는 이유는 무엇인가?
3. `Timer_Default`와 `Timer_60`은 각각 언제 갱신되는가?
4. `fTimeAcc = 0.f`는 남은 누적 시간을 버리는가? 고정 timestep accumulator의 일반적인 `-=` 방식과 어떤 차이가 있는가?
5. `CMainApp::Update`는 `void`인데 실제 출력은 무엇인가?
6. `pMainApp->Render()`가 `E_FAIL`이면 제어 흐름은 어디로 이동하는가?
7. 세 함수의 실제 Call Stack을 적는다.
8. breakpoint hit 간격으로 관찰한 `fTimeDelta` 범위를 적는다.
9. 이 함수들만 보면 한 frame의 시간 복잡도를 `O(1)`이라고 말할 수 있는가? 하위 호출을 포함하면 어떤 변수가 필요한가?
10. `wWinMain`을 한 문장으로 압축한다.

### 제출 증거

```text
Call Stack 1회
연속 frame의 fTimeAcc 3개
Update 진입 시 fTimeDelta 3개
Render 실패 시 예상 제어 경로
완성된 함수 분석 양식: CMainApp::Update
```

## R2. 프레임 스케줄과 Engine facade

### 대상 함수

- `Engine/Private/GameInstance.cpp:98` `CGameInstance::Update_Engine`
- `Engine/Private/Object_Manager.cpp:98` `Priority_Update`
- `Engine/Private/Object_Manager.cpp:110` `Update`
- `Engine/Private/Object_Manager.cpp:122` `Late_Update`

### Breakpoint·Watch

```text
GameInstance.cpp:98
Object_Manager.cpp:98/110/122

Watch:
fTimeDelta
m_iNumLevels
m_pLayers[현재 level].size()
```

### 질문

1. Picking이 Input보다 먼저 갱신되는 현재 순서를 실제 코드로 적는다.
2. Pipeline과 Frustum은 왜 Object Update 앞뒤 중 현재 위치에 있는지 가설을 세운다.
3. Priority/Update/Late_Update가 세 번 Layer를 순회하는 이유와 비용을 계산한다.
4. `CGameInstance`는 실제 연산자와 facade 중 어느 역할이 더 큰가?
5. 함수가 `void`일 때 실패는 어떻게 관찰되는가? 현재 코드가 놓칠 수 있는 실패는 무엇인가?
6. subsystem pointer를 `unique_ptr`로 소유하는 owner는 누구인가?
7. 호출 순서를 바꾸면 보일 수 있는 한 frame 지연 사례 하나를 든다.

### 제출 증거

```text
실제 호출 순서
활성 level 수 L, layer 수 A
한 frame Object Manager 세 pass의 순회식
함수 분석 양식: CGameInstance::Update_Engine
```

## R3. Level/Layer 자료구조와 반복 비용

### 대상 함수

- `Engine/Private/Object_Manager.cpp:98/110/122`
- `Engine/Private/Layer.cpp:69/78/87`

### 확인 자료구조

```text
level별 LAYERS
map<wstring_t, shared_ptr<CLayer>>
list<shared_ptr<CGameObject>>
```

### 질문

1. Layer tag 탐색의 복잡도는 무엇인가?
2. Layer 안 모든 GameObject 순회의 복잡도는 무엇인가?
3. `list` 삽입/삭제 `O(1)`이 성립하려면 어떤 전제가 필요한가?
4. 현재 Layer 순회에서 `list`와 `vector`의 cache locality 차이는 무엇인가?
5. `shared_ptr`를 range-for에서 값으로 받는지 reference로 받는지 확인하고 ref count 비용을 설명한다.
6. 전체 level의 layer/object 수를 사용해 pass 하나의 복잡도 식을 만든다.
7. 세 pass를 합쳤을 때 Big-O와 실제 상수 비용을 구분한다.

### 제출 증거

```text
현재 활성 level/layer/object 수
컨테이너 실제 타입 선언 위치
반복 변수 타입
함수 분석 양식: CObject_Manager::Update
```

## R4. Prototype/Clone과 공동 소유권

### 대상 함수

- `Engine/Private/Object_Manager.cpp:48` `Add_GameObject_to_Layer`
- `Engine/Private/Prototype_Manager.cpp:35` `Clone_Prototype`
- 대상 GameObject의 `Clone`
- `Engine/Private/Layer.cpp:46` `Add_GameObject`

### 질문

1. Prototype 원본은 어떤 smart pointer로 누가 소유하는가?
2. Clone은 왜 `shared_ptr<CPrototype>`로 반환되는가?
3. `dynamic_pointer_cast<CGameObject>`가 실패하는 조건은 무엇인가?
4. 새 Layer와 기존 Layer 분기의 차이를 적는다.
5. `pOutGameObject`는 반환값과 별도로 어떤 출력을 전달하는가?
6. 함수 종료 뒤 local `pClonedGameObject`가 사라져도 객체가 살아 있는 이유는 무엇인가?
7. `strLayerTag`의 map 탐색/삽입 복잡도를 적는다.
8. Clone 실패의 반환 경로를 Level 호출자까지 추적한다.

### 제출 증거

```text
Clone 전후 pointer
Layer 추가 전후 object count
Call Stack
각 shared owner 목록
함수 분석 양식: Add_GameObject_to_Layer
```

## R5. Valtan/Body PartObject 조립과 raw pointer 수명

### 대상 함수

- `Client/Private/Valtan.cpp:20` `CValtan::Initialize`
- `Client/Private/Valtan.cpp:56` `Ready_PartObjects`
- `Engine/Private/ContainerObject.cpp:66` `Add_PartObject`
- `Client/Private/Body_Valtan.cpp:21` `CBody_Valtan::Initialize`

### 질문

1. `void* pArg`의 실제 타입을 각 경계에서 어떻게 복원하는가?
2. `BODY_VALTAN_DESC`에 전달되는 부모 matrix와 state pointer의 owner는 누구인가?
3. Body가 그 raw pointer를 저장해도 현재 수명상 안전한 근거는 무엇인가?
4. `CValtan`의 PartObject map은 무엇을 key로 사용하고 탐색 비용은 얼마인가?
5. `Ready_PartObjects()`의 `HRESULT`는 위로 어디까지 전달되는가?
6. Body 모델 컴포넌트 Clone과 Body GameObject Clone의 차이는 무엇인가?
7. 부모가 먼저 파괴되고 Body가 남으면 어떤 버그가 발생하는가?

### 제출 증거

```text
Valtan this 주소
Body가 가진 parent matrix/state 주소
두 주소가 가리키는 실제 멤버
PartObject map size와 key
함수 분석 양식: Ready_PartObjects
```

## R6. Animation 자료구조와 알고리즘

### 대상 함수

- `Client/Private/Body_Valtan.cpp:40` `Update`
- `Engine/Private/Model.cpp:164` `Play_Animation`
- `Engine/Private/Animation.cpp:59` `Update_TransformationMatrix`
- `Engine/Private/Channel.cpp:150` `Update_TransformationMatrix`
- `Engine/Private/Bone.cpp:45` `Update_CombinedTransformationMatrix`

### 질문

1. `Play_Animation`의 `bool_t`는 무엇을 의미하고 현재 Body는 값을 소비하는가?
2. Animation의 channel vector와 Bone vector 크기를 Watch로 적는다.
3. `pLeftKeyFrameIndex`가 매 frame 처음부터 검색하지 않게 하는 원리는 무엇인가?
4. scale/rotation/translation에 각각 어떤 보간을 사용하는가?
5. parent Bone combined matrix가 먼저 준비되어야 하는 불변식은 무엇인가?
6. `C=channel`, `K=이번 frame에 넘어간 key`, `B=bone`으로 CPU 복잡도를 쓴다.
7. Model Prototype Clone에서 Mesh/Material과 Bone/Animation의 공유·복사 차이를 확인한다.
8. animation 종료 값을 사용해 Attack -> Idle 전환을 만든다면 어느 함수가 소비해야 하는가?

### 제출 증거

```text
current track position 전후
current animation index/name
channel/bone 수
특정 channel의 left key index 전후
root/child Bone combined matrix 한 개씩
함수 분석 양식: CModel::Play_Animation
```

## R7. Renderer와 CPU/GPU 경계

### 대상 함수

- `Client/Private/Body_Valtan.cpp:49` `Late_Update`
- `Engine/Private/Renderer.cpp:117` `Add_RenderObject`
- `Engine/Private/Renderer.cpp:128` `Draw`
- `Client/Private/Body_Valtan.cpp:56` `Render`
- `Engine/Private/Mesh.cpp:154` `Bind_Resource`

### 질문

1. `Late_Update`는 `void`인데 어떤 외부 상태를 변경하는가?
2. `shared_from_this()`가 안전하려면 객체가 어떤 방식으로 소유되어야 하는가?
3. render group vector/list에 shared pointer를 넣으면 수명이 어떻게 변하는가?
4. `Draw` 뒤 render queue를 비우는 이유는 무엇인가?
5. Body의 mesh 수만큼 반복되는 CPU 작업을 적는다.
6. Bone matrix는 CPU에서 어디까지 만들고 GPU에서 무엇을 하는가?
7. draw call 수, mesh 수 M, bone 수 B, vertex 수 V를 구분해 비용식을 만든다.
8. Render의 `HRESULT`를 Renderer가 실제로 검사하는지 확인한다.

### 제출 증거

```text
Late_Update 전후 NONBLEND queue size
Body Render hit count
mesh count와 draw call count
Bone matrix count
함수 분석 양식: CBody_Valtan::Late_Update 또는 Render
```

## R8. Binary asset와 CModel 통합 경로

### 대상 함수

- `Engine/Private/Model.cpp:108` `Initialize_Prototype`
- `Engine/Private/Model.cpp:291` `Ready_BinaryModel`
- `Engine/Private/BinaryAsset/WModelDecoder.cpp:278` `Decode`
- `CMesh/CMaterial` binary initialization

### 질문

1. 확장자에 따라 FBX/Assimp와 wmodel decoder가 어디서 갈리는가?
2. 두 입력 경로가 최종적으로 공유하는 runtime 자료구조는 무엇인가?
3. magic/version/section count 검증이 실패하면 어떤 반환 경로를 타는가?
4. Decode 중 임시 CPU data와 최종 GPU buffer의 owner를 구분한다.
5. Prototype load 시 한 번 수행할 일과 매 frame 수행할 일을 구분한다.
6. mesh/material/skeleton/animation section의 관계를 설명한다.
7. file size `F`, vertex `V`, index `I`, material `M`, animation key `K` 기준 load 비용을 추정한다.
8. 레거시 `CCookedModel/CBinaryAssetObject` 대신 `CModel` 입구 하나를 유지해야 하는 이유는 무엇인가?

### 제출 증거

```text
실제 wmodel 경로
magic/version/section count
생성된 mesh/material/bone/animation 수
GPU buffer 생성 breakpoint
함수 분석 양식: Ready_BinaryModel
```

## R9. MapTool 상태 머신과 배치 자료구조

### 대상 함수

- `Client/Private/MapTool.cpp:178` `Try_PickPlacementPosition`
- `Client/Private/MapTool.cpp:220` `Try_PlaceSelected`
- `Client/Private/MapTool.cpp:270` `Allocate_EditorPlacementId`
- `Client/Private/MapTool.cpp:294` `Create_Placement`

### 질문

1. UI 선택이 곧바로 GameObject 생성을 의미하지 않는 이유는 무엇인가?
2. `IDLE/ARMED` 상태 전환 조건을 적는다.
3. Picking 결과가 실패할 때 placement state와 status는 어떻게 되는가?
4. asset ID, placement ID, prototype tag의 수명과 저장 가능성을 비교한다.
5. placement vector에서 ID 검색이 `O(P)`인 근거를 찾는다.
6. `Allocate_EditorPlacementId`의 최악 복잡도를 현재 `Find_Placement` 구현까지 포함해 계산한다.
7. vector에 push/erase할 때 `PLACED_ENTRY*`와 iterator 무효화 가능성을 설명한다.
8. ImGui가 매 frame file/model decode를 하지 않아야 하는 이유는 무엇인가?

### 제출 증거

```text
상태 전환 전후 enum 값
picked position
배치 전후 vector size
asset/placement/prototype 값
함수 분석 양식: Try_PlaceSelected
```

## R10. Save/Load transaction과 Navigation

### 대상 함수

- `Client/Private/MapTool.cpp:372` `Save_Placements`
- `Client/Private/MapTool.cpp:407` `Load_Placements`
- `Engine/Private/Navigation.cpp:160` `isMove`
- `Engine/Private/Cell.cpp:56` `isIn`

### 질문

1. 저장 전에 모든 record를 검증해야 하는 이유는 무엇인가?
2. Load가 기존 배치를 먼저 지우지 않고 staged vector를 만드는 이유는 무엇인가?
3. 중간 Clone 실패 시 staged object와 기존 scene은 어떻게 처리되는가?
4. `std::move(staged)`가 복사 대신 무엇을 넘기는가?
5. Navigation의 현재 cell index와 neighbor index는 어떤 의미인가?
6. Cell `isIn`이 point와 edge를 어떤 방식으로 비교하는지 설명한다.
7. cell 수 N에서 단순 전체 탐색과 neighbor 이동의 비용 차이를 적는다.
8. static baked navigation과 파괴 가능한 구조물 obstacle을 같은 데이터로 덮어쓰면 생기는 문제를 설명한다.

### 제출 증거

```text
Load 전 기존 vector size
staged size 변화
실패 지점에서 rollback 확인
navigation current cell/neighbor index
함수 분석 양식: Load_Placements 또는 CNavigation::isMove
```

---

## 6. 채점·피드백·재도전 규칙

### 6.1 10점 기준

```text
2점 호출자·호출 시점·Call Stack
2점 입력·상태 변경·반환/부작용
2점 소유권·수명
2점 자료구조·복잡도·호출 빈도
2점 Breakpoint 실측 증거와 한 문장 압축
```

### 6.2 피드백 방식

사용자의 답변을 다음 세 부분으로 나눈다.

```text
확정: 코드와 실측이 일치한 내용
교정: 개념은 맞지만 owner/빈도/복잡도 표현이 부정확한 내용
재추적: 호출 화살표나 수명을 반대로 이해해 breakpoint가 다시 필요한 내용
```

정답 전체를 바로 보여주지 않고 틀린 최초 경계에 대한 보충 질문을 먼저 한다.

### 6.3 진행 기준

- 8~10점: 다음 라운드
- 6~7점: 같은 함수 보충 질문 후 재채점
- 0~5점: 상위 호출자로 돌아가 Call Stack부터 재작성
- 수명/owner 핵심 오류: 점수와 관계없이 재실측
- 근거 없는 추측: 해당 칸 미완료

### 6.4 기록 방식

각 라운드 완료 후 이 계획서에 다음 결과를 덧붙인다.

```text
날짜:
라운드:
점수:
실측 Call Stack:
핵심 깨달음:
잘못 예측한 부분:
다음 재확인 항목:
```

훈련 전체가 끝나면 같은 폴더에
`2026-07-30_LOSTARK_FUNCTION_BREAKPOINT_REASONING_TRAINING_RESULT.md`를 작성한다.

---

## 7. 전체 완료 검증

### 7.1 백지 수직 흐름

다음 화살표를 코드 없이 작성하고 각 화살표의 실제 함수를 붙인다.

```text
Windows message/time
-> Client frame
-> Engine subsystem update
-> Level/Layer object update
-> Valtan/Body state
-> Animation/Bone pose
-> Render queue
-> Shader/GPU
-> Present
```

### 7.2 Owner 지도

다음 객체의 owner와 observer를 구분한다.

```text
CMainApp
CGameInstance subsystem
Prototype
runtime CValtan Clone
CBody_Valtan
CModel/CMesh/CMaterial/CBone/CAnimation
Renderer frame queue
MapTool placement object
```

### 7.3 복잡도 구두 검증

다음을 실제 기호로 설명한다.

```text
Object Manager frame traversal
Layer object traversal
Prototype tag lookup
Animation pose update
Body mesh render
Placement ID allocation
Placement save/load
Navigation move check
```

### 7.4 실패 역추적

다음 중 하나를 선택해 최초 실패와 최종 증상을 연결한다.

```text
Prototype tag 없음
Body component Clone 실패
wmodel decode 실패
material bind 실패
placement load 중간 실패
navigation cell 없음
```

### 7.5 최종 질문

```text
이 객체는 왜 존재하는가?
정본 데이터는 어디 있는가?
누가 소유하고 언제 파괴하는가?
어느 함수가 언제 바꾸는가?
반환값과 부작용은 누가 소비하는가?
비용은 무엇에 비례하는가?
어떤 breakpoint와 값으로 증명했는가?
```

이 일곱 질문에 실제 LostArk 함수와 값으로 답할 수 있으면 다른 엔진과 회사 코드에서도 같은
좌표계를 사용할 수 있다.

---

## 8. 진행 기록

### 2026-07-30 R1-A — 통과 8/10

```text
실측:
- CGameInstance::Update_Engine이 main thread에서 실행됨을 Threads 창으로 확인.
- Breakpoint/Tracepoint 정지와 출력 비용이 DeltaTime을 크게 만드는 관찰자 효과 확인.

핵심 깨달음:
- 실행 순서는 이미 반환된 함수까지 포함한 시간 흐름이다.
- Call Stack은 현재 아직 반환되지 않은 중첩 호출만 보여준다.
- void 함수도 subsystem/world 상태를 바꾸는 큰 부작용을 가질 수 있다.

교정:
- QueryPerformanceCounter가 CPU 부하로 임의로 튀는 것이 아니라, 늦어진 실제 경과 시간을 측정한다.
- 모든 Update 함수가 fTimeDelta를 받거나 이전/현재 상태를 비교하는 것은 아니다.
```

### 2026-07-30 R2 — 통과 8/10

```text
확정:
- CGameInstance::Update_Engine은 자신의 멤버를 직접 대입하기보다 소유 subsystem에 명령을 전달하는 facade/orchestrator다.
- Object/Level 경로에서 fTimeDelta가 virtual Update로 전달됨을 이해함.
- Picking 생성 실패가 Initialize_Engine -> MainApp::Create -> wWinMain으로 전파되는 정상 경로를 추적함.
- Priority/Update/Late_Update가 각각 Layer/GameObject를 순회함을 확인함.

교정:
- Update_Engine 진입 후 m_pPicking이 null이면 HRESULT가 아니라 null 역참조 Access Violation이다.
- L=level slot, A=전체 layer, O=전체 object일 때 한 pass는 Θ(L+A+O), 세 pass도 Θ(L+A+O)다.
- 예제의 전체 Layer 수는 2+0+3+0+1=6이다.
- frame 호출은 정확한 60회가 아니라 현재 accumulator 조건에 따른 약 60Hz 가변 timestep이다.

한 문장 최종형:
CGameInstance::Update_Engine은 main thread의 frame update 시 fTimeDelta를 받아 Picking부터 Level까지
소유 subsystem을 정해진 순서로 호출하고, 다음 render에 사용할 engine/world 상태를 갱신하는 함수다.
```
