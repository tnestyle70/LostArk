# 2026-08-05 PhysX 본질·세팅·활용 설계 (PLAN)

문서 유형: 도메인 설계 (구현 코드 정본은 `2026-08-05_PHYSX_INTEGRATION_G00_G01_PLAN.md`가 소유)
목표 선언: 게임 피직스의 본질을 실측 근거로 정리하고, 현재 프레임워크에 PhysX 5를 **어디에, 어떤 계약으로, 무엇을 위해** 세팅하는지 확정한다. 발탄 파괴 트리거 애니메이션에는 PhysX가 필수가 아님을 명시하고, PhysX의 실제 소비자(범용 rigid body 표현, 본체인 secondary motion, 플레이어 낙하 표현, debris 연출)를 G 단위 로드맵으로 닫는다.

관련 문서:
- [툴 스위트·최적화·피직스 도메인 분석](../08-03/2026-08-03_LOSTARK_TOOL_SUITE_OPTIMIZATION_PHYSICS_DOMAIN_ANALYSIS_PLAN.md) §4 (PhysX 본체인 클로스 도메인, 공수 6~8일 P6 stretch)
- [툴 스위트 개요](../08-03/2026-08-03_TOOL_SUITE_OVERVIEW_PLAN.md) (학습·설계 완료 계약)
- 구현 코드 정본: [PhysX Integration G00-G01 PLAN](2026-08-05_PHYSX_INTEGRATION_G00_G01_PLAN.md)

---

## 1. 피직스의 본질

### 1.1 피직스가 소유하는 상태와 한 스텝의 파이프라인

게임 피직스는 "충돌했는가?"의 질의가 아니라, 물체마다 다음 상태를 소유하고

```text
위치·회전 / 선속도·각속도 / 질량·관성 텐서 / 힘·토크
```

한 고정 스텝 동안 다음 파이프라인으로 다음 상태를 결정하는 시뮬레이션이다.

```text
힘 적용(중력·외력)
→ 속도 적분
→ 충돌 후보 탐색 (broad phase)
→ 실제 접촉점 계산 (narrow phase)
→ 관통 방지·마찰·Joint 제약 해결 (constraint solver: PGS 기본, TGS 선택)
→ 위치·회전 적분과 sleep 판정
```

PhysX에서 이 계산이 이루어지는 물리 월드가 `PxScene`이며, Actor·Shape·Material·Joint를 씬에 넣고 `simulate(dt)` + `fetchResults(true)`로 한 스텝을 전진시킨다.

### 1.2 Rigid Body 세 종류 — 위치 결정자가 다르다

| 종류 | 위치 결정자 | 우리 프로젝트에서의 용도 |
|---|---|---|
| `PxRigidStatic` | 움직이지 않음 | 바닥 plane, 고정 지형 |
| Kinematic `PxRigidDynamic` | 게임/애니메이션 코드 (`setKinematicTarget`) | 애니메이션을 따라가는 앵커 뼈(골반·어깨), 움직이는 발판 |
| Dynamic `PxRigidDynamic` | 물리 solver | 드롭 테스트 상자, 본체인의 시뮬 뼈, debris 파편 |

Joint(`PxSphericalJoint`, `PxD6Joint`)는 두 Rigid Body 사이의 상대 운동을 제한한다. 본체인에는 콘 리밋 + damping을 준 Spherical/D6가 표준이다.

### 1.3 피직스는 애니메이션 재생기가 아니다

- 애니메이션: 저작된 시간축 Transform을 재생한다.
- 피직스: 힘과 제약으로 Transform을 실시간 계산한다.
- 혼합형(우리의 목표): 몸통은 애니메이션이 소유하고, 머리카락·치맛자락·파편만 물리가 소유한다. 소유권 경계가 뼈 단위로 갈린다.

### 1.4 절대 계약 네 가지 (08-03 문서 §4.2 재확인)

1. **고정 timestep** — 가변 dt를 그대로 넣으면 제약 해가 폭발한다. accumulator로 1/60 고정 스텝만 밟는다.
2. **앵커는 kinematic, 시뮬 대상만 dynamic** — 애니메이션 pose가 진실인 뼈에 물리를 덮어쓰지 않는다.
3. **joint limit + damping + 속도 클램프** — 과회전·발산 방지.
4. **순간이동 시 branch 리셋** — teleport 프레임의 거대 속도를 시뮬에 넣지 않는다.

## 2. 현재 프레임워크 실측 — 무엇이 있고 무엇이 없는가

2026-08-08 기준 branch `codex/bern-camera-follow-stability` 재실측이다. `Engine/ThirdPartyLib/PhysX`, `Physics_Manager.h/.cpp`, `Physics_Tool.h/.cpp`가 모두 없으므로 PhysX 구현 상태는 여전히 **G00 미착수**다. 작업 트리의 다른 기능 변경은 이 계획에서 소유하지 않는다.

| 항목 | 상태 | 근거 |
|---|---|---|
| 애니메이션 → 본 행렬 파이프라인 | 있음 | `CModel::Play_Animation()`이 채널 로컬 갱신 → 블렌드 → 루트 보정 → `m_Bones` 순회로 Combined 행렬 확정 (`Engine/Private/Model.cpp`, 현재 함수 시작 326행) |
| 스키닝 팔레트 | 있음, 매 bind 재구성 | `CMesh::Bind_Resource`가 offset × combined를 매 호출 계산 (`Engine/Private/Mesh.cpp:156-168`) — combined를 바꾸면 렌더에 자동 반영 |
| Client 단순 충돌 질의 | 있음, 표현/디버그 | `CCollider`(AABB/OBB/Sphere) `Intersect()` 질의 + `_DEBUG` 와이어 렌더를 제공하고 `CCharacter`가 플레이어 OBB 표현을 갱신한다. 이동 정답은 아니다. |
| Server 이동 충돌 | 있음, 권위 | `CServerCollisionSystem::Resolve_PlayerMove()`가 정적 `collisionBox`에 대해 swept OBB를 계산한다. PhysX를 추가해도 이 경로를 대체하지 않는다. |
| Rigid Body 시뮬레이션 | 없음 | 질량·속도·마찰·충돌 응답 없음 |
| Joint/Constraint | 없음 | |
| 물리 결과의 본 역주입 | 없음 | `CModel`은 본 열거/인덱스 접근을 공개하지 않음 (`Get_BoneMatrix`/`Has_Bone`뿐, `Engine/Public/Model.h:53-54`) |
| 고정 스텝 인프라 | 없음 | `Timer_60`은 1/60 게이트의 **실경과** delta이며 고정 상수가 아니고, 메시지 루프가 `fTimeAcc = 0.f`로 잔여 시간을 버린다 (`Client/Default/Client.cpp:87-112`) |

세팅 지점의 실측 (구현 정본은 G00-G01 PLAN):

- 서브시스템 파사드: `CGameInstance`가 `unique_ptr` 멤버로 서브시스템을 소유하고 `Create() → nullptr 검사` 이디엄으로 초기화한다 (`Engine/Private/GameInstance.cpp:30-102`). `CProfiler`는 `Get_Profiler()`로 포인터를 그대로 노출하는 선례가 있다 (`Engine/Public/GameInstance.h:129-130`).
- 프레임 루프: `CGameInstance::Update_Engine()`의 갱신 순서는 Picking → Input → Sound → Priority_Update → `Refresh_CameraState` → Update → Late_Update → Level_Manager다 (`Engine/Private/GameInstance.cpp`, 현재 함수 시작 104행). G01은 Update와 Late_Update 사이에 Physics step을 넣고, G02는 Physics와 Late_Update 사이에 `Post_Physics_Update`를 추가한다.
- 레벨 수명: `Change_Level`이 **이전** 레벨 인덱스로 `Clear_Resources`를 호출하고 (`Engine/Private/Level_Manager.cpp:14-31`), `LEVEL::STATIC`(0)은 그 인수가 되지 않아 영구 생존한다. 물리 액터의 레벨별 수명은 이 seam에 편승해야 한다.
- 종료: `Release_Engine()`이 명시적 `.reset()` 순서로 해체한다 (`Engine/Private/GameInstance.cpp:421-441`).
- 빌드/배포: 서드파티는 vcxproj `AdditionalDependencies`로 링크하고(#pragma 금지 관례), 배포는 `UpdateLib.bat`이 유일한 계약이다(PostBuildEvent 없음). RuntimeLibrary는 미지정 → MSBuild 기본 **/MDd(Debug), /MD(Release)**다 (`Engine/Default/Engine.vcxproj:112-161`).
- Git: `.gitignore`가 `*.lib`/`*.dll`을 전역 ignore하므로 Assimp처럼 명시적 부정(`!`) 라인이 필요하고, `.gitattributes`의 `*.lib`/`*.dll` LFS 패턴이 자동 적용된다.

## 3. 결론 — 무엇에 PhysX가 필요하고, 무엇에 불필요한가

### 3.1 발탄 파괴 트리거 애니메이션: PhysX 불필요

필요한 핵심 흐름은 물리가 아니라 상태 전파다.

```text
Server 페이즈 변화 (권위)
→ 파괴 환경 상태 확정
→ Client에 상태 전파 (snapshot)
→ 해당 placement의 파괴 애니메이션/파괴 모델 재생 (표현)
```

Client에는 이 표현 경로의 뼈대가 이미 있다: `CDeployPropObject::Set_State(FRACTURED)`가 STATIC이면 fractured 모델로 스왑하고 ANIM이면 `"off"` 논리 클립을 재생한다 (`Client/Private/DeployPropObject.cpp:133-146`). placement 데이터에는 `destructible`, `stateOffActionId`, `triggerBinaryOccurrenceCount`가 이미 저장된다 (`Client/Public/DeployPropCatalog.h:39-52`).

현재는 `CLevel_ValtanArena::Initialize()`가 `CDeployPropRuntime::Load_Area()`를 호출하고, Loader가 `Prototype_GameObject_DeployProp`을 등록하며, runtime placement ID로 `Set_State()`까지 찾을 수 있다. 즉 **모델 로드와 Client 상태 적용 경로는 활성화**됐다. 아직 닫히지 않은 부분은 Server 권위의 destroyable 판정·동적 navigation·Shared 상태/이벤트 복제다. authoring parser가 구조를 읽을 수 있어도 publisher/runtime가 fail-closed로 거부하는 현재 계약을 우회하지 않는다.

PhysX는 그 뒤 "파편이 튕기고 굴러가고 잠드는" **optional 연출(G05)**에서만 쓴다. 지속 상태 `FRACTURED`는 late join 동기화용이고, one-shot 파괴 이벤트(`eventSequence`, Server tick)는 애니메이션·음향·debris를 한 번만 재생하는 용도로 분리한다. `Set_State(FRACTURED)` 자체에서 무조건 파편을 스폰하면 late join이 과거 폭발을 다시 재생하므로 금지한다.

### 3.2 머리카락·옷자락 secondary motion: PhysX 적합 (본체인 = 관절로 묶인 강체 사슬)

포트폴리오들이 말하는 "뼈 심고 PxScene에서 시뮬"은 대부분 천 솔버가 아니라 **Secondary Bone Dynamics**다. 우리 리소스에는 이미 시뮬용 보조 본이 들어 있어 본을 새로 심을 필요가 없다: LanceMaster 224본(`b_hair`, `b_upper_skirt`, `b_upper_cloth` 계열), Artist 239본(`b_hair`, `b_skirt`, `b_capatcloth`, `b_add_tail` 계열), DimensionMaster 225본(hair/skirt/tail 계열).

```text
애니메이션이 소유한 루트 뼈(골반/어깨/머리) = kinematic 앵커 (매 프레임 setKinematicTarget)
시뮬 대상 뼈 집합(치맛자락·머리카락) = branch
branch의 각 뼈 = PxRigidDynamic (캡슐/구)
이웃 뼈 사이 = Spherical/D6 Joint (콘 리밋 + damping)
매 프레임: 애니 갱신 → 앵커 target 주입 → 고정 스텝 simulate → fetchResults
→ branch 뼈 시뮬 pose를 본 로컬 행렬로 역주입 → combined 재계산 → 스키닝
```

역주입 seam은 실측으로 확정됐다: `CModel::Play_Animation()` 내부, combined 행렬 루프(Model.cpp:317-320)와 `return isFinished;`(322) 사이가 유일한 정답 지점이다. 그 시점에 애니 로컬·블렌드·combined가 모두 확정돼 있고, `CBone::Update_TransformationMatrix`(공개, 로컬 setter)로 쓰고 나서 branch 첫 뼈 인덱스 이후만 combined를 재계산하면 된다(본 배열은 parent-before-child 순서). 주의 실측 두 가지:

1. `Play_Animation`은 애니메이션이 없으면 조기 반환한다(Model.cpp:303-304) — 정적 포즈 모델에는 훅이 돌지 않으므로 branch 시뮬 대상은 애니 재생 모델로 한정하거나 조기 반환 경로도 커버해야 한다.
2. `CBone`은 `ENGINE_DLL` export가 아니고 `CModel`이 `m_Bones`를 비공개로 들고 있다 — **solver는 Engine 내부(CModel이 구동)에 산다.** Client에서 뼈를 직접 만지는 두 번째 경로를 만들지 않는다.

저장 계약: branch 정의(대상 뼈, 질량·반경·limit·damping)는 뼈 **이름**으로 저장한다(WSKL 계약과 일치). 본 인덱스·포인터는 저장 ID가 아니다. 저작 데이터는 `Data/Animation/Authored/<AssetId>/physics.json` 계열로 두고 런타임 로더가 소비한다. 저작 UI는 기존 Animation Tool에 branch 편집 모드를 추가하는 방향이며(스켈레톤 트리에서 branch 지정 → 파라미터 편집 → 즉석 시뮬 확인 → 저장), 별도 G PLAN에서 닫는다.

### 3.3 플레이어 낙사: 판정은 Server, rigid body는 Client 표현에 적합

"보행 불가 셀로 밀렸으므로 낙사"를 그대로 구현하면 벽·장애물·맵 바깥까지 낙사로 오판한다. Server 데이터는 최소한 `WALKABLE`, `BLOCKED`, `VOID` 의미를 분리하거나, 파괴 페이즈에 따라 활성화되는 명시적 `fallVolume`을 가져야 한다. 현재 제품 경계에는 `fallVolume` 계약이 없으므로 G04에서 Data → publisher → Shared → Server → Client를 한 수직 슬라이스로 추가한다.

```text
Boss 공격 승인과 넉백 계산 (Server)
→ forced movement가 fallVolume/VOID에 진입
→ PLAYER_LIFE_STATE::FALLING 확정, 입력·스킬 잠금
→ snapshot/event에 시작 tick·origin·initial velocity 전파
→ Client가 캐릭터 표현용 rigid body를 dynamic으로 전환하고 중력 활성화
→ Server가 낙사 완료·사망·리스폰 시점을 확정
→ Client가 rigid body 속도/pose를 지우고 kinematic 표현으로 복귀
```

rigid body는 중력·초기 속도·회전·주변 정적 지오메트리와의 반응을 표현한다. 낙사 여부, 사망 시점, 부활 위치를 결정하지 않으며 시뮬 pose를 Server로 보내지 않는다. 단순히 수직으로 사라지는 연출이면 해석적 중력식만으로도 충분하고, 튕김·회전·충돌이 보여야 할 때 G02 rigid body가 실익이 있다. 전신 ragdoll은 본별 actor/joint와 애니메이션 전환이 필요한 별도 대형 범위이므로 G04에 포함하지 않는다.

### 3.4 프레임워크 담당의 결론

- 지금 세팅할 것: PhysX SDK 빌드/배치/링크/배포 계약(G00)과 고정 스텝 물리 서브시스템 + 최소 검증 소비자(G01). 이후 G02가 public rigid body/transform 동기화 계약을 만들고, G03~G05 소비자가 같은 경로를 사용한다.
- 지금 세팅하지 않을 것: 발탄 파괴의 상태 전파 파이프라인(물리와 무관, DeployProp 재배선 슬라이스), cloth mesh solver(관절 강체 사슬 근사로 충분), GPU 시뮬레이션(CPU 전용 — `PhysXGpu_64.dll` 불필요), 캐릭터 이동/판정의 물리화(이동·판정은 Server navgrid 권위 — §5 경계).

## 4. 세팅 설계 확정 사항

구현 코드와 검증 명령의 정본은 [G00-G01 PLAN](2026-08-05_PHYSX_INTEGRATION_G00_G01_PLAN.md)이다. 여기에는 결정과 이유만 둔다.

| # | 결정 | 이유 |
|---|---|---|
| S1 | PhysX SDK **5.6.1** (tag `107.3-physx-5.6.1`), `vc17win64` preset | 2026-08-08 최신 태그는 5.9.0이지만 첫 통합은 기존 코드 PLAN과 공식 5.6.1 문서에 맞춘 재현 가능한 pinned baseline을 유지한다. 업그레이드는 G00 완료 뒤 별도 검증 단위다. |
| S2 | preset의 `NV_USE_STATIC_WINCRT`를 **False**로 바꿔 빌드 | 기본 True(/MT)는 우리 /MDd·/MD와 CRT 충돌. DLL CRT(md) 산출물만 사용 |
| S3 | 우리 Debug ↔ PhysX **debug** 빌드, 우리 Release ↔ PhysX **release** 빌드 | PhysX 헤더가 `_DEBUG`/`NDEBUG` 정확히 하나를 강제하고 구성 혼합은 CRT 충돌로 금지됨. checked/profile은 채택하지 않음 |
| S4 | 배치는 `Engine/ThirdPartyLib/PhysX/{Inc, Lib/<Config>, Bin/<Config>}` | FMOD형 self-contained 관례(구성별 lib/dll이 있는 라이브러리는 이 형태). `Inc`에는 `generate_projects` 후의 include 전체(`PxConfig.h` 포함) |
| S5 | 링크는 Engine.vcxproj `AdditionalDependencies`만 사용: `PhysX_64.lib; PhysXCommon_64.lib; PhysXFoundation_64.lib; PhysXExtensions_static_64.lib; PhysXPvdSDK_static_64.lib` | 저장소 관례(#pragma comment 금지). Extensions는 joint/기본 filter shader/`PxCreatePlane`에, PvdSDK는 디버그 계측에 필요하며 둘 다 static-only |
| S6 | 배포는 `UpdateLib.bat`에 구성별 xcopy 3줄 추가: `PhysX_64.dll`, `PhysXCommon_64.dll`, `PhysXFoundation_64.dll` | UpdateLib.bat이 유일한 배포 계약. PostBuildEvent를 새로 만들지 않음. Cooking·GPU DLL은 배포하지 않음(박스/구/캡슐/plane은 cooking 불요, `PxCooking` 클래스는 5.2에서 제거됨) |
| S7 | Git은 `.gitignore` 부정 블록(Assimp 블록 관례) + 기존 `.gitattributes` LFS 패턴 자동 적용 | 전역 `*.lib`/`*.dll` ignore와 `**/[Dd]ebug/` 디렉터리 ignore를 이기려면 디렉터리부터 파일까지 명시 부정이 필요 |
| S8 | `Engine/Public/Physics_Manager.h`는 **PhysX 헤더를 포함하지 않는다** (physx 네임스페이스 전방 선언 + 포인터만) | 이 헤더는 `UpdateLib.bat`으로 `EngineSDK/inc`에 복사돼 Client가 본다. Client에 PhysX include 경로를 요구하지 않는 것이 경계다. PhysX 타입은 `Physics_Manager.cpp`에만 존재 |
| S9 | 파사드는 `Get_PhysicsManager()` 포인터 노출 | `Get_Profiler()` 선례. `GameInstance.h`의 include 최소 계약(전방 선언 유지)을 지킨다 |
| S10 | `PxScene`은 엔진 수명 1개, **액터를 레벨 인덱스별 버킷**으로 등록 | `Clear_Resources(iClearLevelID)`가 이전 레벨 인덱스로만 호출되는 기존 seam에 그대로 편승. `LEVEL::STATIC` 버킷은 인수가 되지 않아 자동 생존. 씬 자체를 레벨마다 파괴/생성하는 대안은 재생성 비용과 STATIC 예외 처리를 늘려 기각 |
| S11 | 고정 스텝 펌프는 `Object_Manager->Update` 뒤, `Late_Update` 앞 | Update에서 애니메이션과 kinematic target을 확정 → Physics simulate/fetch → G02 `Post_Physics_Update`에서 결과를 Transform/본에 반영 → Late_Update에서 렌더 그룹 제출 순서를 지킨다. Late_Update 뒤에 시뮬하면 같은 프레임 렌더에 반영되지 않아 1프레임 지연된다. accumulator + 1/60 고정 스텝 + 프레임당 최대 4 step 상한은 유지한다. |
| S12 | 해체는 `Release_Engine`에서 `m_pObject_Manager.reset()` **뒤** | 이후 G02에서 컴포넌트가 액터 핸들을 들게 되면 오브젝트가 먼저 죽고 씬이 나중에 죽어야 한다. 내부 해체 역순: Scene → Dispatcher → Material → CloseExtensions → Physics → PVD → Foundation |
| S13 | PVD는 `_DEBUG` 한정, connect 실패는 비치명 | release PhysX 빌드는 PVD 계측이 비활성. 디버그에서만 127.0.0.1:5425 시도 |
| S14 | G01의 소비자는 F1 Developer Tools의 **Physics Tool**(drop test + 통계 + 와이어프레임) | 소비자 없는 서브시스템은 AGENTS.md 위반. drop test는 simulate/fetchResults/고정 스텝/레벨 정리의 실행 증거이고, 패널은 G02 actor/rigid-body 진단과 G03 branch 튜닝으로 이어진다. 디버그 드로우는 `CCollider`와 동일한 DirectXTK `DebugDraw`(`DX::Draw`) 경로를 재사용 — 두 번째 시각화 런타임을 만들지 않는다 |

## 5. 경계 — 지키는 것

- **PhysX는 Client 표현 전용이다.** Server 판정(이동·피격·페이즈)은 navgrid/balance/room tick 권위 그대로다. 시뮬 결과를 Server로 보내지 않고, Server가 PhysX를 알게 하지 않는다.
- 플레이어 낙사는 모든 `walkable=false` 셀에서 발생하지 않는다. Server가 `VOID` 또는 활성 `fallVolume` 진입과 `FALLING/DEAD/RESPAWN` 상태를 확정하고 Client rigid body는 그 결과만 표현한다.
- 발탄 파괴의 진실은 Server 페이즈다. PhysX debris는 상태 전파 이후의 optional 연출이며, debris가 게임플레이 판정(길막기 등)을 만들면 안 된다. 길찾기 차단은 기존 `VALTAN_ARENA_DESTROYED` navigation blocker 계약이 소유한다.
- 본체인 솔버는 Engine 내부(`CModel` 구동)에 산다. Client가 `CBone`을 직접 만지는 두 번째 본 경로를 만들지 않는다.
- Physics Tool은 F1 Developer Tools 안의 패널일 뿐이다. 새 기능키를 추가하지 않는다(F1/F6 외 금지 규칙 유지).
- 저장 계약에 본 인덱스·포인터·prototype tag를 쓰지 않는다. branch는 뼈 이름, debris는 placement ID 기준이다.
- `Engine/Public` 변경이 있으므로 매 슬라이스에서 `UpdateLib.bat` → Client 빌드까지가 검증 단위다.

## 6. G 로드맵과 공수

| G | 내용 | 산출물 | 공수 | 상태 |
|---|---|---|---:|---|
| G00 | SDK 빌드·배치·링크·배포·Git 계약 | ThirdPartyLib/PhysX, vcxproj·UpdateLib·gitignore 변경 | 0.5~1일 | 코드 PLAN 완료 ([G00-G01 PLAN](2026-08-05_PHYSX_INTEGRATION_G00_G01_PLAN.md)) |
| G01 | `CPhysics_Manager` 서브시스템(고정 스텝, 레벨 버킷, PVD) + Physics Tool drop test | Engine/Client 코드, ProjectAudit check | 1~1.5일 | 코드 PLAN 완료 (상동) |
| G02 | 범용 rigid body bridge: stable handle, actor descriptor, `CRigidBody` 또는 동등한 Engine component, kinematic/dynamic 전환, gravity/velocity/pose API, `Post_Physics_Update` | G02 디테일 코드 PLAN | 2~3일 | 미착수 — G01 완료 후 현재 Component/ObjectManager 실측으로 작성 |
| G03 | 본체인 secondary motion: `CModel` 훅 + branch solver + `physics.json` 로더 + Animation Tool branch 저작 | G03 디테일 코드 PLAN | 4~5일 | 미착수 — 이 문서 §3.2가 설계 정본 |
| G04 | 플레이어 낙사 수직 슬라이스: fallVolume/VOID 데이터, Server forced movement·life state, Shared snapshot/event, Client rigid body 표현·리스폰 reset | G04 구현/디테일 PLAN | 3~4일 | 미착수 — 이 문서 §3.3이 경계 정본 |
| G05 | 발탄 debris 연출: Server destroyable state/event·dynamic navigation 선행 뒤 저비용 파편 actor 스폰과 TTL/sleep 회수 | G05 구현/디테일 PLAN | 2~3일 | 미착수 — 제품 destroyable gate가 선행 |

G00~G03의 8~11일은 기존 6~8일 견적에 범용 rigid body bridge와 현재 코드 재검증을 분리 반영한 값이다. G04/G05는 기존 견적 밖의 Server 권위 수직 슬라이스이며, PhysX 세팅만으로 자동 완성되지 않는다. 각 G의 디테일 코드 PLAN은 직전 G 검증 후 현재 코드에 다시 맞춰 작성한다.

## 7. 비평 반영

작성 직후 4렌즈(코드 정합성 / 경계 위반 / 완전성 / PhysX 기술 정확성) 독립 비평과 실코드 재현 검증을 수행했고, 재현된 지적만 반영했다. 반영 내역은 코드 정본인 [G00-G01 PLAN](2026-08-05_PHYSX_INTEGRATION_G00_G01_PLAN.md) §0.1에 기록한다.
