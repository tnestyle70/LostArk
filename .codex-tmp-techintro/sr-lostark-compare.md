# SR Minecraft Dungeons와 LostArk 실제 코드 비교 메모

2026-10-14. 읽기 전용 표본 조사다. 두 저장소 전체를 완독하거나 실행한 결과가 아니다. 원본 코드, 렌더링 설정, 빌드 파일은 수정하지 않았다.

## 비교 범위와 원형 확인

- SR은 C:/Users/tnest/Desktop/MinecraftDungeons이며 remote는 https://github.com/tnestyle70/SR_Minecraft_Dungeons.git 다. 현재 branch는 codex/restore-resources-build, HEAD는 48122be다.
- 현재 SR 작업 폴더에는 10월 9일 복구 설명과 CEnderDragon, CSquidCoast, 프로젝트 설정 등의 미커밋 변경이 있다. 현재 디스크 전체를 개발 당시 원본이라고 부르면 안 된다.
- 아래 주요 비교 표본 CTransform.cpp, CCamera.cpp, CRenderer.cpp, CGameObject.cpp, CDynamicCamera.cpp, CPlayer.cpp는 git diff --quiet HEAD -- <표본 목록> 결과 0으로 현재 HEAD와 같음을 확인했다. 이것은 해당 파일의 현재 미커밋 변경이 없다는 뜻이다. 과거 각 줄의 작성 주체나 AI 관여 여부를 증명하지 않는다.
- SR에서 해당 폴더의 AGENTS.md/CLAUDE.md는 검색되지 않았다. README와 실제 소스·헤더를 읽었다. LostArk는 상위 작업 규칙과 학습 기준을 따른다.
- 구조의 좋고 나쁨, 사용자의 실력, NYPC 결승 진출 실패 원인을 파일 길이나 주석 수로 판정하지 않는다.

## 결론

SR에서 되찾을 것은 작은 동작의 입력과 저장 상태, 계산 결과가 어디서 쓰이는지 스스로 연결할 수 있는 상태다. SR의 API나 클래스 구조로 LostArk 전체를 되돌릴 이유는 확인되지 않았다.

현재 LostArk도 Transform.cpp와 Camera.cpp에 호출자·단위·실패 경계를 설명하는 한국어 주석이 있다. 실제 차이는 주석의 존재 자체보다, 한 동작을 이해할 때 만나는 소유권·프레임 단계·서버 권위·저작과 실행 데이터 경계가 늘어났다는 점이다. 그중 필요한 책임도 있고, 이름·중복 상태·긴 분기 때문에 늘어난 읽기 비용도 있다. 기능 하나를 골라 구별해야 한다.

처음에는 일반 플레이어 추적 카메라 하나를 추천한다. SR과 LostArk 양쪽에 대응 코드가 있고, 기존 PPT 17페이지의 카메라 행렬과 연결된다. 서버나 레이드 전체를 먼저 이해하지 않아도 상태·포인터 수명·벡터·보간·행렬 소비를 한 번 이어 볼 수 있다.

## 비교 1 — 추적 카메라와 좌표 계산

### SR에서 실제로 확인한 흐름

- [CDynamicCamera::Update_GameObject](C:/Users/tnest/Desktop/MinecraftDungeons/Client/Code/CDynamicCamera.cpp:44)는 부모 CCamera::Update_GameObject를 먼저 호출한다.
- 같은 함수 70행부터 대상 Transform을 읽고, 일반 모드에서는 138행의 m_vFollowOffset을 사용한다. 142행에서 m_vEye = vPlayerPos + vOffset, 최종 172행에서 m_vAt = vPlayerPos + (0, 1.5, 0)을 쓴다.
- [CCamera::Update_GameObject](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CCamera.cpp:48)는 eye/at/up으로 D3DXMatrixLookAtLH를 만들고 D3D9 SetTransform(D3DTS_VIEW)에 제출한다.
- [헤더](C:/Users/tnest/Desktop/MinecraftDungeons/Client/Header/CDynamicCamera.h:93)의 대상은 CTransform*다. SetFollowTarget은 주소를 저장할 뿐 AddRef를 하지 않으므로 카메라가 그 대상의 수명을 소유하는 관계가 아니다.

기본 수식은 desiredEye = targetPosition + followOffset이다. 위치와 offset 모두 같은 월드 좌표계여야 한다. offset은 길이, fTimeDelta는 초다. 기본 eye에는 시간 기반 평활화가 없다. 드래곤 방향과 시점 전환에는 별도의 Lerp와 각도 계산이 있다.

강점은 대상 위치와 시점의 계산이 같은 함수에 보인다는 점이다. 반면 부모가 View를 먼저 제출하고 그 뒤 eye/at를 바꾸므로, 이 함수만 따라가면 새 자세와 제출 시점 사이에 순서 차이가 보인다. 전체 호출 순서와 화면에서 실제 지연이 있는지는 이번에 실행하지 않았으므로 별도 검증 사항이다.

### LostArk에서 실제로 확인한 흐름

- [Camera_Free.h](C:/Users/tnest/Desktop/LostArk/Client/Public/Camera_Free.h:109)는 요청 여부, 실제 활성 여부, 초기화 여부를 구분한다. 대상은 weak_ptr<CTransform>, 저장 상태는 현재 look-at, 이전 대상 위치, offset, response다.
- [Late_Update](C:/Users/tnest/Desktop/LostArk/Client/Private/Camera_Free.cpp:99)는 컷신 제어권을 확인하고, 이전 shake를 제거한 뒤 일반 follow를 계산하고 새 shake를 적용한 다음 Update_PipeLine을 호출한다.
- [Update_FollowCamera](C:/Users/tnest/Desktop/LostArk/Client/Private/Camera_Free.cpp:337)는 lock으로 이번 호출 동안 대상 수명을 확보한다. 대상 소멸 시 follow를 끄고, 대상 위치 NaN/Inf면 새 계산을 하지 않는다.
- 최초 프레임·대상 위치 불연속·response <= 0이면 즉시 목표로 이동한다. 일반 프레임은 alpha = -expm1(-response * dt) = 1 - exp(-response * dt)를 사용한다.
- eye_next = eye + alpha * (desiredEye - eye), at_next도 같은 식이다. 여기서 response 단위는 1/초, dt는 초이므로 지수 입력은 무차원이다.
- [CTransform::LookAt](C:/Users/tnest/Desktop/LostArk/Engine/Private/Transform.cpp:168)는 look=at-position, right=worldUp×look, up=look×right를 정규화하고 기존 축 길이를 다시 적용한다.
- [CCamera::Update_PipeLine](C:/Users/tnest/Desktop/LostArk/Engine/Private/Camera.cpp:218)은 Transform 월드 행렬의 역행렬을 View로, degree FOV를 radian으로 바꾼 PerspectiveFovLH를 Projection으로 제출한다.

### 무엇이 필요한 복잡성인가

| 추가 상태·분기 | 해결하는 문제 | 읽을 때 반드시 확인할 것 |
|---|---|---|
| weak_ptr와 lock | 카메라가 퇴장한 플레이어를 붙잡아 두지 않으면서 사용 중 수명을 확보 | Layer/객체가 강한 소유자이고 카메라는 약한 관찰자인가 |
| requested/enabled/initialized | 사용자의 추적 의사, 실제 대상 존재, 과거 보간값 유효성을 구분 | 대상 교체·복귀 때 어떤 상태를 초기화하는가 |
| previous target와 불연속 판정 | 순간 이동한 대상을 긴 거리 동안 천천히 따라가는 현상 방지 | 임계값은 거리 제곱과 비교하고 있는가 |
| 현재 eye/look-at | 매 프레임 목표만으로는 표현할 수 없는 평활화 이력 | 현재 프레임 출력이 다음 프레임 입력으로 남는 이유 |
| 컷신 소유권과 shake 순서 | 여러 기능이 같은 최종 카메라를 서로 덮는 상황 관리 | 현재 프레임의 최종 writer와 제출자는 누구인가 |

모든 분리가 필연적이라는 뜻은 아니다. 예를 들어 현재 Update_PipeLine의 지역 변수 이름 ViewMatrixInverse에는 실제로 inverse(CameraWorld), 즉 View가 들어간다. 이름만 읽으면 반대로 해석하기 쉽다. Transform::LookAt도 at==eye 또는 look과 +Y가 평행한 입력의 별도 복구 처리는 없다. 현재 주석은 무엇을 보장하지 않는지도 설명하고 있으므로, 설명과 실제 식을 함께 읽을 수 있다.

### 숫자로 확인한 작은 예

아래는 게임 성능 실측이 아니라 현재 식을 Python으로 계산한 예다. 이미 초기화된 카메라, 고정 목표 x=10, 현재 x=0, 컷신·shake·순간이동·탈것 보정이 없는 조건이다.

| dt | response=18일 때 alpha | 첫 갱신의 x |
|---|---:|---:|
| 1/30초 | 0.451188 | 4.511884 |
| 1/60초 | 0.259182 | 2.591818 |
| 1/120초 | 0.139292 | 1.392920 |

고정 목표에서 1초를 각각 30/60/120회로 나누어 계산하면 x는 모두 약 9.9999998477이다. 오차는 e_next=(1-alpha)e이며, 여러 단계의 exp(-k*dt)가 exp(-k*총시간)로 곱해지기 때문이다. 움직이는 목표를 서로 다른 시점에 샘플링하는 실제 장면까지 완전히 같은 궤적이라고 주장하지 않는다.

response를 18에서 9로 줄이면 더 느리게 수렴한다. 0으로 만들면 현재 코드에서는 평활화가 사라지고 즉시 이동한다. 이 예외를 식 하나만 보고 놓치면 실제 수정 결과를 잘못 예상한다.

## 비교 2 — 투명 객체 정렬과 참조 수명

### SR

- [CRenderer.h](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Header/CRenderer.h:42)는 그룹마다 list<CGameObject*>를 가진다.
- [Add_RenderGroup](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CRenderer.cpp:14)은 객체 주소를 넣고 AddRef한다.
- [Render_Alpha](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CRenderer.cpp:56)는 SRCALPHA/INVSRCALPHA를 설정하고, Get_ViewZ가 큰 객체부터 list.sort하여 Render_GameObject를 호출한다.
- [Compute_ViewZ](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CGameObject.cpp:52)는 View 역행렬의 카메라 위치와 객체 위치 간의 유클리드 거리를 저장한다. 이름은 ViewZ지만 실제 값은 시야 공간 z좌표가 아니다.
- [Clear_RenderGroup](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CRenderer.cpp:94)은 CDeleteObj로 Release한 뒤 목록을 비운다. CDeleteObj의 실제 Release는 Engine/Header/Engine_Function.h 69~84행이다.

작은 흐름이 보인다. 객체가 렌더 큐에 있는 동안 큐도 한 참조를 보유하고, 프레임 종료 시 그 참조를 해제한다. 원래 Layer 소유권과 렌더 큐의 일시 참조는 다른 것이다.

### LostArk

- [Renderer.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/Renderer.h:137)는 vector<shared_ptr<CGameObject>>를 그룹별로 가진다.
- [Add_RenderObject](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:560)는 인자로 받은 shared_ptr를 vector에 move한다. 호출 인자 형성 시 이미 존재하는 소유권 공유 비용까지 사라진다는 뜻은 아니다.
- [Render_Blend](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2335)는 queue를 그대로 두고 임시 BLEND_ENTRY 배열을 만든다. entry의 CGameObject*는 비소유이며 이때 원래 queue의 shared_ptr가 수명을 유지한다.
- 정렬 키는 우선순위와 거리 제곱이다. [BlendSortBefore](C:/Users/tnest/Desktop/LostArk/Engine/Public/BlendSortKey.h:17)는 priority 오름차순, 같은 priority이면 distanceSquared 내림차순이다. 동률의 제출 순서 보존은 comparator가 아니라 stable_sort가 한다.
- Render_Group(BLEND)을 순서대로 호출한 뒤 2411행에서 queue를 비우고 임시 배열도 비운다. Draw 실패 경로 905~916행도 남은 queue를 정리한다.

공통 계산은 투명 합성의 순서 의존성이다. C_out = alpha*C_src + (1-alpha)*C_dst는 일반적으로 src와 dst를 바꾸면 값이 달라진다. 거리 d 대신 d²로 비교해도 유한한 비음수 거리의 순서는 같다. sqrt 계산을 피하는 근거는 있지만 전체 프레임이 얼마 빨라졌는지는 측정해야 한다.

현재 SR Renderer.cpp는 101줄, LostArk Renderer.cpp는 3392줄이다. 이것은 해당 파일의 현재 물리적 줄 수일 뿐 품질 점수가 아니다. LostArk에는 다중 Render Target, 광원, 후처리, GPU 계측과 여러 실패 처리가 포함되어 있다. 같은 투명 정렬 경로만 비교해야 책임 증가와 표현상의 부담을 분리할 수 있다.

시간·공간 비용은 두 정렬 모두 일반적인 비교 정렬 규모로 O(N log N)를 생각할 수 있고, LostArk는 명시적인 O(N) 임시 entry 공간을 재사용한다. std::stable_sort 자체의 보조 메모리 확보 여부에 따라 비교 횟수 보장이 달라질 수 있으므로 모든 환경에 정확히 동일한 내부 비용이라고 단정하지 않는다. vector<shared_ptr<Object>>는 포인터 목록이며 객체 전체를 SoA로 배치한 구조가 아니다. 이 부분만 보고 LostArk 전체를 DOD라고 설명하면 안 된다.

### Component 조회도 비교하면 차이가 보인다

SR은 [CGameObject.h 29행](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Header/CGameObject.h:29)의 multimap<const wchar_t*, CComponent*>를 [Find_Component](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CGameObject.cpp:67)에서 find_if와 문자열 내용 비교로 순회한다. C개의 component에 대해 문자열 비교 횟수는 최악 O(C)다. 포인터 key가 곧 문자열 값 정렬을 뜻하지 않기 때문이다.

LostArk는 [GameObject.h 172행](C:/Users/tnest/Desktop/LostArk/Engine/Public/GameObject.h:172)의 map<wstring, shared_ptr<CComponent>>를 [Get_Component](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameObject.cpp:21)에서 map.find로 조회한다. key 비교 횟수는 O(log C)이며 문자열 비교 자체 비용은 별도다. 실제 C가 작으면 컨테이너의 관리·할당·간접 접근 비용도 함께 봐야 한다. 표기상 복잡도만으로 더 빠르다고 측정 결과를 만들지는 않는다.

## 비교 3 — 클릭 이동을 그대로 한 함수로 되돌릴 수 없는 이유

SR의 [CPlayer::Key_Input](C:/Users/tnest/Desktop/MinecraftDungeons/Client/Code/CPlayer.cpp:847) 표본은 목표와 현재 위치를 읽어 y 차이를 제거하고, 거리가 0.3보다 크면 normalize(goal-pos)*speed*dt를 [CTransform::Move_Pos](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Header/CTransform.h:15)에 더한다. 한 파일에서 입력·목표·이동 여부·각도 계산을 이어 읽기 쉽다.

단, SR 저장소도 CNetworkPlayer/CNetworkMgr/Server의 별도 네트워크 경로가 존재한다. 위 CPlayer 표본을 SR 전체의 서버 권위 설계라고 일반화하지 않는다.

LostArk는 [PlayerController::Request_MoveToPointResolved](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:2250)에서 입력 차단 상태를 검사하고 typed command sink로 목표를 보낸다. 전송이 성공한 뒤에만 로컬 표시 예측을 요청한다. [NetworkPlayerCommandSink.cpp 233행](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkPlayerCommandSink.cpp:233)은 NetworkManager의 Send_MoveGoal로 연결한다.

Server의 [Handle_Move와 Execute_PlayerMove](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerCommands.cpp:32)는 session→player 조회, sequence·유한성·범위 검사, 행동 제한을 처리한다. 205행부터 경로 재설정 부분은 현재 진행 중인 경로를 보존하면서 별도 stagedPath를 계산한 뒤 성공 때 교체한다.

[GameRoom_PlayerSimulation.cpp 1137행](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp:1137)은 현재 waypoint를 고르고 delta/distance를 계산한다. 1183행에서 moveDistance=min(speed*fixedDeltaSeconds, distance)로 목표 초과 이동을 막고 proposed 위치를 만든 뒤 지면과 충돌 검증으로 이어간다. 이 min은 SR 표본의 단순 speed*dt와 비교할 수 있는 작고 구체적인 차이다.

Client의 [Apply_PlayerSnapshot](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:5032)은 서버 locomotion 상태, processed move 상태와 위치를 캐릭터 표현으로 적용한다. Client 호출이 별도 Server 프로세스의 함수를 직접 호출하는 구조가 아니다. 패킷 전송·서버 큐·snapshot이 중간 경계다. 이번에는 위 양 끝과 핵심 처리의 정적 연결을 확인했고 실제 전송이나 전체 직렬화 흐름은 실행 검증하지 않았다.

이 경계들은 멀티플레이에서 입력 요청, 서버가 허용한 상태, 즉시 보여 주는 예측을 구별하기 위해 존재한다. 모두 Player 한 파일로 합치면 사용자의 권위 계약을 바꾸게 된다. 반대로 각 interface에 읽기 비용이 없는 것도 아니다. 여기서는 읽기 지도를 먼저 만들고, 정확히 어떤 상태가 불필요한 중복인지 근거가 생긴 뒤에만 구조 변경을 검토한다.

## 첫 회차 — 일반 추적 카메라만 다시 내 것으로 만들기

아래 다섯 묶음만 읽는다. 드래곤 공전·shake·컷신의 전체 구현은 다음 범위로 둔다. 첫 회차는 적용 조건을 배제한 상태에서 입력과 최종 결과를 연결한다.

| 읽을 묶음 | 이번에 답할 것 |
|---|---|
| 1. SR CDynamicCamera.cpp 70~74, 135~172 | 대상 위치에 어떤 offset을 더하며 eye와 at의 최종 쓰기는 어디인가 |
| 2. LostArk Camera_Free.h 109~119 | weak 대상과 현재/이전 값 중 무엇을 기억해야 다음 프레임을 계산할 수 있는가 |
| 3. LostArk Camera_Free.cpp 99~124 및 337~395 | 호출 시점, lock 실패, 최초 snap, alpha, 상태 저장을 연결한다 |
| 4. LostArk Transform.cpp 168~179 | eye/at가 RIGHT·UP·LOOK 행으로 바뀌는 계산을 손으로 그린다 |
| 5. LostArk Camera.cpp 218~239 | 역행렬 View와 FOV Projection이 어디에 제출되는지 확인한다 |

한 장에 아래 내용을 직접 적고 코드와 대조한다.

1. 목표: 일반 이동 중 플레이어 위치를 기준으로 카메라가 따라오고, 순간 이동에는 이전 위치의 보간 이력을 끌고 오지 않는다.
2. 입력: 대상 Transform 위치, position/look offset, dt, response, 추적·초기화 상태.
3. 저장: 현재 카메라 Transform, 현재 look-at, 이전 대상 위치. 목표 위치는 매 프레임 계산 가능하므로 무조건 새로운 영구 멤버로 추가할 이유가 없다.
4. 계산: desiredEye/At → snap 또는 exponential Lerp → LookAt 직교 기저 → View inverse.
5. 소비: GameInstance의 View/Projection이 이후 셰이더와 culling에서 쓰인다. 첫 회차는 제출까지 확인한다.

작은 수정은 원본 제품 설정을 먼저 바꾸는 대신, 현재 Update_FollowCamera 식만 옮긴 짧은 실험에서 response=18/9, dt=1/30·1/60·1/120을 비교한다. 사용자가 먼저 결과 방향을 예측하고 계산을 고쳐 본다. 그다음 기존 디버거에서 초기화된 일반 follow의 response를 세션 중 바꾸는 검증으로 연결할 수 있다. 제품의 저장된 카메라 프로필이나 renderer 옵션을 이 학습 때문에 자동 변경하지 않는다.

검증 질문은 하나로 시작한다. “dt=1/60, 초기화된 현재 x=0, 고정 desired x=10일 때 response 18을 9로 줄이면 첫 x가 커지는가 작아지는가? 그런데 response=0이면 현재 코드에서는 왜 바로 10이 되는가?”

이것을 설명하고 조건을 바꿔 결과를 예상할 수 있으면, 다음은 투명 정렬의 key와 참조 수명, 그 다음은 클릭 이동의 서버 경계로 넓힌다. 모든 헤더 완독이나 책 전체 학습을 취업 자료 완성의 선행 조건으로 추가하지 않는다.

## SR에서 주석과 최종 실행을 구분하는 두 사례

1. CDynamicCamera.cpp 155행은 dragon lookOffset을 적용하고 165행은 free-look Lerp를 적용하지만, 172행이 m_vAt를 다시 player+(0,1.5,0)으로 덮는다. 중간 주석과 계산만 읽지 말고 최종 writer를 추적해야 하는 구체적인 사례다. 첫 follow 학습에서 드래곤 전체를 동시에 분석하라는 뜻은 아니다.
2. CTransform::Compute_Lookattarget 134행의 matRot는 지역 변수인데 150행에서 이를 출력으로 받는 D3DXMatrixRotationAxis의 포인터를 반환한다. 반환 포인터는 함수 종료 뒤 유효하지 않다. 현재 활성 호출 경로는 이번에 확인하지 않았으므로 실제 크래시가 발생했다고 기록하지 않는다.

SR은 결함이 없는 이상적 기준이 아니다. 다만 사용자가 읽고 계산을 재구성할 수 있었던 기준이다. 그 장점을 유지하면서 현재의 더 넓은 요구를 작은 검증 가능한 동작으로 나누는 것이 회복 방향이다.

## 확인 범위

- SR remote/branch/HEAD 및 선택 표본의 HEAD 일치 확인.
- 두 저장소의 위 함수·헤더·컨테이너·최종 쓰기를 정적으로 대조.
- exponential follow의 고정 목표 수치 예제 계산.
- 게임 실행·빌드·화면 관찰·성능 비교는 하지 않음.
- 사용자 독립 이해나 직접 수정 완료는 아직 확인하지 않음.
- 원본 코드 변경 없음. 이 메모만 작성.
