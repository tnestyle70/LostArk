# LostArk Character·Valtan NavPathFollower 구현 계획서

> 작성일: 2026-07-31  
> 기준 브랜치: `main` / `origin/main`  
> 기준 커밋: `4096bdccaaf7b32ead89b5553e06fc0542b2d3e4`  
> 문서 목적: 구조 설명에 그치지 않고, 검토 후 그대로 반영할 수 있는 최종 코드 제시  
> 현재 상태: 아래 C++ 구현을 작업 트리에 반영했고 Debug/Release 빌드를 완료했다.

---

## C1. 목표

이번 구현의 목표는 이동을 다음 세 덩어리로 분리하는 것이다.

1. `CLevel_AssetTest`
   - 좌클릭을 감지한다.
   - 화면 좌표를 월드 좌표로 Picking한다.
   - 플레이어에게 “이 위치로 이동해”라는 명령만 전달한다.

2. `CCharacter`
   - 자신의 `CNavigation` 복제본과 `CNavPathFollower`를 소유한다.
   - 좌클릭 목적지까지 경로를 요청하고 실제 Transform 이동을 처리한다.
   - 이동 중에는 Run, 이동 종료 시 Idle 애니메이션을 선택한다.

3. `CValtan`
   - 플레이어의 `CTransform`을 약한 참조로 가진다.
   - 일정 거리 밖이면 플레이어 위치로 경로를 재탐색한다.
   - 실제 이동은 자신의 `CNavPathFollower`에 맡긴다.
   - 보스 패턴은 이후 `CValtan`이 결정하되, 특수 이동을 전부 Follower에 넣지는 않는다.

한 문장으로 줄이면 다음과 같다.

```text
Level은 입력만 번역하고,
Character와 Valtan은 각자 이동 의도를 결정하고,
Engine의 Navigation/Follower는 경로 계산과 경로 추종만 담당한다.
```

---

## C2. 현재 코드에서 확인한 사실

### C2-1. NavGrid 베이킹

`Tools/LevelPlacementExtractor/build_valtan_navgrid.py`가 만드는 데이터의 역할은 다음과 같다.

```text
CUL_BOX
  └─ NavGrid를 만들 전체 XZ 범위만 결정

Floor01_A / Floor01_B / 중앙 Floor
  └─ 실제 걸을 수 있는 셀과 셀 높이 결정

출력
  └─ ValtanArena.navgrid
```

현재 산출물은 다음과 같다.

```text
Width × Height : 62 × 63
Cell count     : 3,906
Cell size      : 0.5
Origin         : (140.5, -137.5)
Walkable       : 2,843
Blocked        : 1,063
```

### C2-2. Engine 경로 계층

현재 Engine 흐름은 이미 역할이 분리되어 있다.

```text
CNavigation::Create_NavGrid()
  └─ .navgrid 로드
      └─ shared_ptr<const CNavGrid>

CNavigation::Find_Path()
  └─ World 좌표를 Cell 좌표로 변환
      └─ CPathFinder::Find_Path()
          └─ A*
              └─ Parent를 Goal → Start로 역추적
                  └─ std::reverse()
                      └─ Start → Goal 경로 완성

CNavPathFollower
  └─ 완성된 Waypoint를 순서대로 따라 Transform 이동
```

따라서 호출 측에서 다시 `reverse()`하면 안 된다. 이미
`CPathFinder::Build_Result()` 안에서 정방향으로 뒤집는다.

### C2-3. NavPathFollower가 Engine에 있어도 되는 이유

`CNavPathFollower`는 다음 정보만 사용한다.

```cpp
shared_ptr<CNavigation>
shared_ptr<CTransform>
vector<float3_t>
이동 속도
TimeDelta
```

다음 Client 지식은 전혀 모른다.

```text
좌클릭
플레이어
Valtan
보스 패턴
Run/Idle 애니메이션 이름
어그로 대상
공격 사거리
```

즉 “누가 왜 이동하는가”가 아니라 “주어진 길을 어떻게 따라가는가”만 처리하므로
Engine의 범용 값 객체로 두는 것이 맞다.

---

## C3. 해결해야 할 문제

### ① 현재 AssetTest에는 실제 플레이어가 없다

현재 레벨은 Valtan만 만들고 있어 `VALTAN_DESC.pTargetTransform`이 비어 있다.
따라서 Valtan의 추적 코드는 존재해도 자동 추적이 시작되지 않는다.

해결:

```text
Character 생성
  → Character Transform 획득
    → Valtan 생성 인자로 전달
```

### ② 기존 클릭 이동의 대상이 Valtan이다

기존 흐름은 LMB Picking 후 Valtan의 public `Request_Move()`를 직접 호출한다.
이 구조에서는 플레이어 입력과 보스 AI의 경계가 섞인다.

해결:

```text
LMB → CCharacter::Request_Move()
Valtan 이동 → CValtan 내부의 Request_PathToTarget()
```

`CValtan::Request_PathToTarget()`은 외부 입력용 API가 아니므로 private으로 둔다.

### ③ Character에는 Navigation/Follower가 없다

현재 `CCharacter`는 Spec과 PartObject 조립만 담당한다.
공통 플레이어 이동을 추가하려면 Navigation 복제본과 Follower를 소유해야 한다.

해결:

```cpp
shared_ptr<CNavigation> m_pNavigationCom;
CNavPathFollower        m_PathFollower;
f32_t                   m_fMoveSpeed;
```

### ④ 플레이어와 Valtan이 하나의 Navigation 상태를 공유하면 안 된다

NavGrid 원본 데이터는 읽기 전용이므로 공유해도 되지만, 다음 상태는 객체별로 달라야 한다.

```text
A* 작업 버퍼
현재 디버그 경로
Follower Waypoint와 현재 인덱스
마지막 경로 탐색 결과
```

해결:

```text
Navigation Prototype
  ├─ Character Navigation Clone
  │    ├─ 공유 NavGrid
  │    ├─ 전용 PathFinder
  │    └─ Character Follower
  │
  └─ Valtan Navigation Clone
       ├─ 공유 NavGrid
       ├─ 전용 PathFinder
       └─ Valtan Follower
```

### ⑤ “모든 보스 이동을 Follower가 처리한다”는 경계를 수정해야 한다

Follower가 처리할 대상:

```text
일반 추적
지정 지점으로 걷기
패턴 시작 위치로 이동
복귀 이동
```

Follower가 처리하지 않을 대상:

```text
돌진처럼 속도 곡선이 별도인 이동
Root Motion에 종속된 이동
Knock-back
Teleport
강제 위치 보정
```

패턴이 특수 이동을 시작할 때는 다음처럼 일반 추적 경로를 취소한 뒤,
그 패턴이 Transform 제어권을 가져야 한다.

```cpp
m_PathFollower.Cancel();
Set_ChaseState(false);
// 이후 Pattern 전용 이동 처리
```

이번 단계에서는 사용하지 않는 Pattern enum이나 Controller 클래스를 미리 만들지 않는다.

---

## C4. 최종 실행 흐름

### C4-1. 플레이어 좌클릭 이동

```text
사용자 LMB 누름
  ↓
CLevel_AssetTest::Update_ClickMove()
  ↓
CGameInstance::Picking()
  ↓
월드 목적지
  ↓
CCharacter::Request_Move()
  ↓
CNavPathFollower::Request_Path()
  ↓
CNavigation::Find_Path()
  ↓
CPathFinder A*
  ↓
Waypoint 저장
  ↓
매 프레임 CCharacter::Update()
  ↓
CNavPathFollower::Update()
  ↓
Transform 이동 + 방향 회전
  ↓
Run / Idle 전환
```

Engine의 업데이트 순서는 Object update 후 Level update이므로,
Level에서 받은 좌클릭 명령은 다음 프레임 Character update부터 소비된다.
입력 지연은 최대 한 프레임이며 구조적으로 정상이다.

### C4-2. Valtan 자동 추적

```text
Level이 Character를 먼저 생성
  ↓
Character Transform을 Valtan Desc에 전달
  ↓
CValtan::Update()
  ↓
Target Transform lock
  ↓
거리 ≤ 2.5m ?
  ├─ Yes: 경로 취소 + Idle
  └─ No
      ↓
    0.35초 재탐색 주기 도달?
      ├─ No: 기존 경로 계속 추종
      └─ Yes
          ↓
        목표가 0.5m 이상 이동했거나 현재 경로가 없음?
          ├─ No: 기존 경로 계속 추종
          └─ Yes: 새 A* 경로 요청
                  ├─ 성공: 새 경로 추종
                  └─ 실패: 오래된 추적 경로 취소
```

### C4-3. 경로 요청 실패 정책

플레이어와 보스의 정책은 다르게 둔다.

```text
Character
  새 좌클릭 목적지가 막혀 있음
    → 새 요청은 실패
    → 기존에 걷던 유효 경로는 유지

Valtan
  추적 대상까지 새 경로를 만들 수 없음
    → 오래된 경로를 계속 따라가면 잘못된 위치로 달려감
    → 기존 경로 취소
```

이 차이는 `CNavPathFollower::Request_Path()`가 성공할 때만
staged Waypoint를 commit하는 현재 계약을 활용한다.

---

## C5. 데이터 구조와 알고리즘

### C5-1. 객체별 소유 데이터

| 소유자 | 데이터 | 수명/공유 규칙 |
|---|---|---|
| Navigation Prototype | `shared_ptr<const CNavGrid>` | Level Prototype 수명 동안 유지 |
| Character Navigation Clone | `unique_ptr<CPathFinder>` | Character 전용 |
| Character | `CNavPathFollower` | Character 전용 |
| Valtan Navigation Clone | `unique_ptr<CPathFinder>` | Valtan 전용 |
| Valtan | `CNavPathFollower` | Valtan 전용 |
| Valtan | `weak_ptr<CTransform>` target | Character를 강제로 살려두지 않음 |

### C5-2. Valtan 재탐색 상수

```cpp
f32_t m_fRepathTime = {};
f32_t m_fStopDistance = { 2.5f };
```

현재 코드에서 직접 사용하는 정책:

```cpp
m_fRepathTime = 0.35f;

// 목표의 XZ 이동량이 0.5m 이상일 때 재탐색
deltaX * deltaX + deltaZ * deltaZ >= 0.25f;
```

이유:

- 매 프레임 A* 요청을 피한다.
- 플레이어가 정지해 있으면 기존 경로를 재사용한다.
- 플레이어가 달리는 동안에는 약 0.35초마다 목표를 갱신한다.
- 거리 비교는 `sqrt` 없이 제곱 거리로 처리한다.

### C5-3. Follower의 staged commit 계약

현재 Engine 코드는 아래 형태다. 이 코드는 변경하지 않는다.

```cpp
vector<float3_t> stagedWaypoints;

m_eLastResult = pNavigation->Find_Path(
    vStartPosition,
    vGoalPosition,
    fMaxStepHeight,
    stagedWaypoints,
    iMaxExpandedNodes,
    &m_iLastExpandedNodes);

if (PATH_RESULT_CODE::SUCCESS != m_eLastResult)
    return m_eLastResult;

m_Waypoints = move(stagedWaypoints);
m_iNextWaypoint = m_Waypoints.size() > 1 ? 1 : 0;
```

실패 시 기존 `m_Waypoints`를 건드리지 않으므로 Character는 자연스럽게 기존 경로를 보존한다.
Valtan은 실패 분기에서 명시적으로 `Cancel()`하여 다른 정책을 적용한다.

---

## C6. 파일별 최종 구현 코드

### C6-1. `Client/Public/Character.h` 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "CharacterSpec.h"
#include "NavPathFollower.h"

NS_BEGIN(Engine)
class CModel;
class CNavigation;
class CTransform;
NS_END

NS_BEGIN(Client)

/* One playable character, whatever the class. Everything class-specific arrives
as a CHARACTER_SPEC plus an ICharacterLogic, so this stays shared by the team.

Input stays outside this object. A controller or level converts input into a
world-space goal and calls Request_Move(). */
class CCharacter final : public CContainerObject
{
public:
    typedef struct tagCharacterDesc : public CContainerObject::CONTAINEROBJECT_DESC
    {
        uint32_t iPrototypeLevelIndex = {};
        const CHARACTER_SPEC* pSpec = { nullptr };
        const tchar_t* pNavigationPrototypeTag = { nullptr };
        float3_t vPosition = {};
    } CHARACTER_DESC;

private:
    CCharacter(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
    CCharacter(const CCharacter& Prototype);

public:
    virtual ~CCharacter();

public:
    const CHARACTER_SPEC* Get_Spec() const {
        return m_pSpec;
    }
    shared_ptr<Engine::CModel> Get_BodyModel() const;
    shared_ptr<Engine::CTransform> Get_Transform() const {
        return m_pTransformCom;
    }

    void Set_Position(fvector_t vPosition);
    bool_t Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop);
    bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);

    PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);
    void Cancel_Move();
    bool_t Is_Moving() const {
        return m_PathFollower.Has_Path();
    }

#ifdef _DEBUG
    void Set_NavigationDebugVisible(bool_t isVisible) {
        m_isNavigationDebugVisible = isVisible;
    }
#endif

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(f32_t fTimeDelta) override;
    virtual void Update(f32_t fTimeDelta) override;
    virtual void Late_Update(f32_t fTimeDelta) override;
    virtual HRESULT Render() override;

private:
    const CHARACTER_SPEC* m_pSpec = { nullptr };
    unique_ptr<ICharacterLogic> m_pLogic;
    shared_ptr<Engine::CModel> m_pBodyModel = { nullptr };
    shared_ptr<Engine::CNavigation> m_pNavigationCom = { nullptr };
    CNavPathFollower m_PathFollower;

    uint32_t m_iPrototypeLevelIndex = {};
    f32_t m_fMoveSpeed = { 5.f };
    bool_t m_isMoving = { false };
    wstring_t m_strNavigationPrototypeTag;

#ifdef _DEBUG
    bool_t m_isNavigationDebugVisible = { false };
#endif

private:
    HRESULT Ready_Components();
    HRESULT Ready_PartObjects();
    void Set_Locomotion(bool_t isMoving);

public:
    static unique_ptr<CCharacter> Create(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
    virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

핵심:

```text
pNavigationPrototypeTag
  → 어느 NavGrid Navigation을 복제할지 지정

m_pNavigationCom
  → Character 전용 Navigation clone

m_PathFollower
  → Character 전용 현재 경로

Request_Move()
  → 외부 Controller가 호출할 명령 API
```

### C6-2. `Client/Private/Character.cpp` 전체 교체

```cpp
#include "Character.h"

#include "GameInstance.h"
#include "Navigation.h"
#include "Part_Body.h"
#include "Part_Equipment.h"

CCharacter::CCharacter(ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
    : CContainerObject { pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
    : CContainerObject { Prototype }
{
}

CCharacter::~CCharacter()
{
}

HRESULT CCharacter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCharacter::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    const auto pDesc = static_cast<CHARACTER_DESC*>(pArg);
    m_pSpec = pDesc->pSpec;
    m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
    m_fMoveSpeed = pDesc->fSpeedPerSec > 0.f ?
        pDesc->fSpeedPerSec : 5.f;

    if (nullptr != pDesc->pNavigationPrototypeTag)
        m_strNavigationPrototypeTag =
            pDesc->pNavigationPrototypeTag;

    if (nullptr == m_pSpec)
        return E_FAIL;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    m_pTransformCom->Set_State(
        STATE::POSITION,
        XMVectorSet(
            pDesc->vPosition.x,
            pDesc->vPosition.y,
            pDesc->vPosition.z,
            1.f));

    if (FAILED(Ready_Components()) ||
        FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (nullptr != m_pSpec->pCreateLogic)
        m_pLogic = m_pSpec->pCreateLogic();

    return S_OK;
}

shared_ptr<CModel> CCharacter::Get_BodyModel() const
{
    return m_pBodyModel;
}

void CCharacter::Set_Position(fvector_t vPosition)
{
    m_pTransformCom->Set_State(STATE::POSITION, vPosition);
}

bool_t CCharacter::Set_Animation(
    CHARACTER_ANIM eAnim,
    bool_t isLoop)
{
    if (eAnim >= CHARACTER_ANIM::END)
        return false;

    return Set_Animation(
        m_pSpec->AnimationClips[ETOUI(eAnim)],
        isLoop);
}

bool_t CCharacter::Set_Animation(
    const char_t* pClipName,
    bool_t isLoop)
{
    if (nullptr == m_pBodyModel || nullptr == pClipName)
        return false;

    return m_pBodyModel->Set_Animation(
        pClipName,
        isLoop);
}

PATH_RESULT_CODE CCharacter::Request_Move(
    fvector_t vGoalPosition)
{
    if (nullptr == m_pNavigationCom ||
        nullptr == m_pTransformCom)
        return PATH_RESULT_CODE::INVALID_GRID;

    const PATH_RESULT_CODE eResult =
        m_PathFollower.Request_Path(
            m_pNavigationCom,
            m_pTransformCom->Get_State(STATE::POSITION),
            vGoalPosition);

    if (PATH_RESULT_CODE::SUCCESS == eResult)
        Set_Locomotion(m_PathFollower.Has_Path());

    return eResult;
}

void CCharacter::Cancel_Move()
{
    m_PathFollower.Cancel();
    Set_Locomotion(false);
}

void CCharacter::Priority_Update(f32_t fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCharacter::Update(f32_t fTimeDelta)
{
    m_PathFollower.Update(
        m_pTransformCom,
        m_fMoveSpeed,
        fTimeDelta);
    Set_Locomotion(m_PathFollower.Has_Path());

    /* The logic only decides class-specific actions. Common navigation
    locomotion stays in CCharacter. */
    if (nullptr != m_pLogic)
        m_pLogic->Update(*this, fTimeDelta);

    __super::Update(fTimeDelta);
}

void CCharacter::Late_Update(f32_t fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

#ifdef _DEBUG
    if (m_isNavigationDebugVisible &&
        nullptr != m_pNavigationCom)
    {
        CGameInstance::Get().Add_DebugComponent(
            m_pNavigationCom);
    }
#endif
}

HRESULT CCharacter::Render()
{
    return S_OK;
}

HRESULT CCharacter::Ready_Components()
{
    if (m_strNavigationPrototypeTag.empty())
        return S_OK;

    return __super::Add_Component(
        m_iPrototypeLevelIndex,
        m_strNavigationPrototypeTag,
        TEXT("Com_Navigation"),
        m_pNavigationCom);
}

HRESULT CCharacter::Ready_PartObjects()
{
    CPart_Body::PART_BODY_DESC bodyDesc{};
    bodyDesc.pParentMatrix =
        m_pTransformCom->Get_WorldMatrixPtr();
    bodyDesc.iPrototypeLevelIndex =
        m_iPrototypeLevelIndex;
    bodyDesc.strModelTag =
        m_pSpec->pBodyModelTag;
    bodyDesc.strShaderTag =
        m_pSpec->pShaderTag;
    bodyDesc.iHiddenMeshMask =
        m_pSpec->iBodyHiddenMeshMask;
    bodyDesc.pInitialAnimation =
        m_pSpec->AnimationClips[
            ETOUI(CHARACTER_ANIM::IDLE)];

    if (FAILED(__super::Add_PartObject(
        m_iPrototypeLevelIndex,
        TEXT("Prototype_GameObject_Part_Body"),
        TEXT("Part_00_Body"),
        &bodyDesc)))
    {
        return E_FAIL;
    }

    m_pBodyModel = dynamic_pointer_cast<CModel>(
        __super::Get_Component(
            TEXT("Part_00_Body"),
            TEXT("Com_Model")));
    if (nullptr == m_pBodyModel)
        return E_FAIL;

    for (uint32_t i = 0;
        i < m_pSpec->iNumEquipment;
        ++i)
    {
        CPart_Equipment::PART_EQUIPMENT_DESC
            equipmentDesc{};
        equipmentDesc.pParentMatrix =
            m_pTransformCom->Get_WorldMatrixPtr();
        equipmentDesc.iPrototypeLevelIndex =
            m_iPrototypeLevelIndex;
        equipmentDesc.strModelTag =
            m_pSpec->pEquipment[i].pModelTag;
        equipmentDesc.strShaderTag =
            m_pSpec->pShaderTag;
        equipmentDesc.pSkeletonModel =
            m_pBodyModel;

        if (FAILED(__super::Add_PartObject(
            m_iPrototypeLevelIndex,
            TEXT("Prototype_GameObject_Part_Equipment"),
            m_pSpec->pEquipment[i].pPartTag,
            &equipmentDesc)))
        {
            return E_FAIL;
        }
    }

    if (nullptr != m_pSpec->pWeaponModelTag)
    {
        CPart_Equipment::PART_EQUIPMENT_DESC
            weaponDesc{};
        weaponDesc.pParentMatrix =
            m_pTransformCom->Get_WorldMatrixPtr();
        weaponDesc.iPrototypeLevelIndex =
            m_iPrototypeLevelIndex;
        weaponDesc.strModelTag =
            m_pSpec->pWeaponModelTag;
        weaponDesc.strShaderTag =
            m_pSpec->pWeaponShaderTag;
        weaponDesc.pSkeletonModel =
            m_pBodyModel;
        weaponDesc.pSocketBoneName =
            m_pSpec->pWeaponSocketBone;

        if (FAILED(__super::Add_PartObject(
            m_iPrototypeLevelIndex,
            TEXT("Prototype_GameObject_Part_Equipment"),
            TEXT("Part_90_Weapon_R"),
            &weaponDesc)))
        {
            return E_FAIL;
        }
    }

    return S_OK;
}

void CCharacter::Set_Locomotion(bool_t isMoving)
{
    if (m_isMoving == isMoving)
        return;

    m_isMoving = isMoving;
    Set_Animation(
        isMoving ?
            CHARACTER_ANIM::RUN :
            CHARACTER_ANIM::IDLE,
        true);
}

unique_ptr<CCharacter> CCharacter::Create(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    auto pInstance = unique_ptr<CCharacter>(
        new CCharacter(pDevice, pContext));

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CCharacter");
        return nullptr;
    }

    return pInstance;
}

shared_ptr<CPrototype> CCharacter::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CCharacter>(
        new CCharacter(*this));

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CCharacter");
        return nullptr;
    }

    return pInstance;
}
```

`Ready_Components()`는 Navigation tag가 없으면 성공한다.
따라서 기존 Test2에서 Character를 생성하는 코드와도 하위 호환된다.

### C6-3. `Client/Public/Level_AssetTest.h` 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CCharacter;
class CValtan;

class CLevel_AssetTest final : public CLevel
{
private:
    CLevel_AssetTest(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);

public:
    virtual ~CLevel_AssetTest();

public:
    virtual HRESULT Initialize() override;
    virtual void Update(f32_t fTimeDelta) override;
    virtual HRESULT Render() override;

private:
    HRESULT Ready_Lights();
    HRESULT Ready_Layer_Camera(
        const wstring_t& strLayerTag);
    HRESULT Ready_Character();
    HRESULT Ready_Valtan();
    void Update_ClickMove();

#ifdef _DEBUG
    void Update_NavigationDebug();
#endif

private:
    shared_ptr<CCharacter> m_pCharacter = { nullptr };
    shared_ptr<CValtan> m_pValtan = { nullptr };
    bool_t m_bLeftMouseDown = { false };

#ifdef _DEBUG
    bool_t m_bF5Down = { false };
    bool_t m_bNavigationDebugVisible = { false };
#endif

public:
    static unique_ptr<CLevel_AssetTest> Create(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### C6-4. `Client/Private/Level_AssetTest.cpp` 전체 교체

```cpp
#include "Level_AssetTest.h"

#include "Camera_Free.h"
#include "Character.h"
#include "GameInstance.h"
#include "Logic_LanceMaster.h"
#include "Valtan.h"

CLevel_AssetTest::CLevel_AssetTest(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
    : CLevel { pDevice, pContext }
{
}

CLevel_AssetTest::~CLevel_AssetTest()
{
}

HRESULT CLevel_AssetTest::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;
    if (FAILED(Ready_Lights()))
        return E_FAIL;
    if (FAILED(Ready_Layer_Camera(
        TEXT("Layer_Camera"))))
    {
        return E_FAIL;
    }
    if (FAILED(Ready_Character()))
        return E_FAIL;
    if (FAILED(Ready_Valtan()))
        return E_FAIL;

    return S_OK;
}

void CLevel_AssetTest::Update(f32_t fTimeDelta)
{
    __super::Update(fTimeDelta);

#ifdef _DEBUG
    Update_NavigationDebug();
#endif

    Update_ClickMove();
}

HRESULT CLevel_AssetTest::Render()
{
    if (FAILED(__super::Render()))
        return E_FAIL;

#ifdef _DEBUG
    SetWindowText(
        g_hWnd,
        TEXT("Valtan WModel Asset Test"));
#endif

    return S_OK;
}

HRESULT CLevel_AssetTest::Ready_Lights()
{
    LIGHT_DESC light{};
    light.eType = LIGHT::DIRECTIONAL;
    light.vDirection =
        float4_t(0.5f, -1.f, 0.5f, 0.f);
    light.vDiffuse =
        float4_t(0.8f, 0.8f, 0.8f, 1.f);
    light.vAmbient =
        float4_t(0.35f, 0.35f, 0.35f, 1.f);
    light.vSpecular =
        float4_t(0.5f, 0.5f, 0.5f, 1.f);

    return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_AssetTest::Ready_Layer_Camera(
    const wstring_t& strLayerTag)
{
    CCamera_Free::CAMERA_FREE_DESC CameraDesc{};
    CameraDesc.vEye =
        float3_t(156.25f, 42.f, -150.f);
    CameraDesc.vAt =
        float3_t(156.25f, 23.f, -121.75f);
    CameraDesc.fFovy = 60.f;
    CameraDesc.fNear = 0.1f;
    CameraDesc.fFar = 1000.f;
    CameraDesc.fSpeedPerSec = 20.f;
    CameraDesc.fRotationPerSec = 90.f;
    CameraDesc.fMouseSensor = 0.1f;

    if (FAILED(CGameInstance::Get().
        Add_GameObject_to_Layer(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT("Prototype_GameObject_Camera_Free"),
            ETOUI(LEVEL::ASSET_TEST),
            strLayerTag,
            &CameraDesc)))
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CLevel_AssetTest::Ready_Character()
{
    CCharacter::CHARACTER_DESC desc{};
    desc.iPrototypeLevelIndex =
        ETOUI(LEVEL::ASSET_TEST);
    desc.pSpec = &Spec_LanceMaster;
    desc.pNavigationPrototypeTag =
        TEXT("Prototype_Component_Navigation_ValtanArena");
    desc.fSpeedPerSec = 6.f;
    desc.fRotationPerSec = 180.f;
    desc.vPosition =
        float3_t(151.25f, 22.96835f, -121.75f);

    shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().
        Add_GameObject_to_Layer(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT("Prototype_GameObject_Character"),
            ETOUI(LEVEL::ASSET_TEST),
            TEXT("Layer_Player"),
            &desc,
            &pGameObject)))
    {
        return E_FAIL;
    }

    m_pCharacter =
        dynamic_pointer_cast<CCharacter>(
            pGameObject);

    return nullptr != m_pCharacter ?
        S_OK : E_FAIL;
}

HRESULT CLevel_AssetTest::Ready_Valtan()
{
    if (nullptr == m_pCharacter)
        return E_FAIL;

    CValtan::VALTAN_DESC desc{};
    desc.fSpeedPerSec = 5.f;
    desc.fRotationPerSec = 180.f;
    desc.pNavigationPrototypeTag =
        TEXT("Prototype_Component_Navigation_ValtanArena");
    desc.pTargetTransform =
        m_pCharacter->Get_Transform();
    desc.vPosition =
        float3_t(156.25f, 22.99751f, -121.75f);

    shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().
        Add_GameObject_to_Layer(
            ETOUI(LEVEL::ASSET_TEST),
            TEXT("Prototype_GameObject_Valtan"),
            ETOUI(LEVEL::ASSET_TEST),
            TEXT("Layer_Valtan"),
            &desc,
            &pGameObject)))
    {
        return E_FAIL;
    }

    m_pValtan = dynamic_pointer_cast<CValtan>(
        pGameObject);

    return nullptr != m_pValtan ?
        S_OK : E_FAIL;
}

void CLevel_AssetTest::Update_ClickMove()
{
    const bool_t isLeftMouseDown =
        false ==
            CGameInstance::Get().
                IsMouseInputBlocked() &&
        0 != (CGameInstance::Get().
            Get_DIMouseState(DIM::LB) & 0x80);

    if (isLeftMouseDown &&
        false == m_bLeftMouseDown &&
        nullptr != m_pCharacter)
    {
        float4_t vPickedPosition{};
        if (CGameInstance::Get().Picking(
            vPickedPosition))
        {
            m_pCharacter->Request_Move(
                XMLoadFloat4(&vPickedPosition));
        }
    }

    m_bLeftMouseDown = isLeftMouseDown;
}

#ifdef _DEBUG

void CLevel_AssetTest::Update_NavigationDebug()
{
    const bool_t isF5Down =
        false ==
            CGameInstance::Get().
                IsKeyboardInputBlocked() &&
        0 != (CGameInstance::Get().
            Get_DIKeyState(DIK_F5) & 0x80);

    if (isF5Down && false == m_bF5Down)
    {
        m_bNavigationDebugVisible =
            !m_bNavigationDebugVisible;

        if (nullptr != m_pCharacter)
        {
            m_pCharacter->
                Set_NavigationDebugVisible(
                    m_bNavigationDebugVisible);
        }

        if (nullptr != m_pValtan)
        {
            m_pValtan->
                Set_NavigationDebugVisible(
                    m_bNavigationDebugVisible);
        }
    }

    m_bF5Down = isF5Down;
}

#endif

unique_ptr<CLevel_AssetTest>
CLevel_AssetTest::Create(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    auto pInstance =
        unique_ptr<CLevel_AssetTest>(
            new CLevel_AssetTest(
                pDevice,
                pContext));

    if (FAILED(pInstance->Initialize()))
        return nullptr;

    return pInstance;
}
```

핵심 생성 순서는 반드시 `Character → Valtan`이다.

```cpp
if (FAILED(Ready_Character()))
    return E_FAIL;
if (FAILED(Ready_Valtan()))
    return E_FAIL;
```

그래야 Valtan 생성 시점에 실제 플레이어 Transform을 전달할 수 있다.

### C6-5. `Client/Public/Valtan.h` 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "NavPathFollower.h"

NS_BEGIN(Engine)
class CModel;
class CNavigation;
class CTransform;
NS_END

NS_BEGIN(Client)

class CValtan final : public CContainerObject
{
public:
    typedef struct tagValtanDesc :
        public CContainerObject::CONTAINEROBJECT_DESC
    {
        const tchar_t* pNavigationPrototypeTag =
            { nullptr };
        shared_ptr<CTransform> pTargetTransform =
            { nullptr };
        float3_t vPosition = {};
    } VALTAN_DESC;

    enum VALTAN_STATE
    {
        IDLE = 0x00000001,
        CHASE = 0x00000002,
    };

private:
    CValtan(ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);

public:
    virtual ~CValtan();

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(f32_t fTimeDelta) override;
    virtual void Update(f32_t fTimeDelta) override;
    virtual void Late_Update(f32_t fTimeDelta) override;
    virtual HRESULT Render() override;

    uint32_t Get_State() const {
        return m_iState;
    }
    PATH_RESULT_CODE Get_PathResult() const {
        return m_PathFollower.Get_LastResult();
    }
    uint32_t Get_PathExpandedNodes() const {
        return m_PathFollower.Get_LastExpandedNodes();
    }
    uint32_t Get_PathWaypointCount() const {
        return m_PathFollower.Get_NumWaypoints();
    }

#ifdef _DEBUG
    void Set_NavigationDebugVisible(bool_t isVisible) {
        m_isNavigationDebugVisible = isVisible;
    }
#endif

private:
    uint32_t m_iState = { VALTAN_STATE::IDLE };
    f32_t m_fMoveSpeed = { 3.f };
    f32_t m_fRepathTime = {};
    f32_t m_fStopDistance = { 2.5f };
    bool_t m_hasLastPathGoal = { false };
    float3_t m_vLastPathGoal = {};

    wstring_t m_strNavigationPrototypeTag;
    weak_ptr<CTransform> m_pTargetTransform;
    shared_ptr<CNavigation> m_pNavigationCom =
        { nullptr };
    shared_ptr<CModel> m_pBodyModelCom =
        { nullptr };
    CNavPathFollower m_PathFollower;

#ifdef _DEBUG
    bool_t m_isNavigationDebugVisible = { false };
#endif

private:
    HRESULT Ready_PartObjects();
    HRESULT Ready_Components();
    PATH_RESULT_CODE Request_PathToTarget(
        fvector_t vGoalPosition);
    void Set_ChaseState(bool_t isChasing);

public:
    static unique_ptr<CValtan> Create(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext);
    virtual shared_ptr<CPrototype> Clone(
        void* pArg) override;
};

NS_END
```

기존 public `Request_Move()`는 제거한다.
Valtan의 이동 목적지는 외부 마우스 입력이 아니라 Valtan AI가 결정하기 때문이다.

### C6-6. `Client/Private/Valtan.cpp` 전체 교체

```cpp
#include "Valtan.h"

#include "Body_Valtan.h"
#include "GameInstance.h"
#include "Model.h"
#include "Navigation.h"

CValtan::CValtan(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
    : CContainerObject { pDevice, pContext }
{
}

CValtan::~CValtan()
{
}

HRESULT CValtan::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CValtan::Initialize(void* pArg)
{
    VALTAN_DESC desc{};
    desc.fSpeedPerSec = 3.f;
    desc.fRotationPerSec = 180.f;

    if (nullptr != pArg)
    {
        desc = *static_cast<VALTAN_DESC*>(pArg);
        if (desc.fSpeedPerSec <= 0.f)
            desc.fSpeedPerSec = 3.f;
        if (desc.fRotationPerSec <= 0.f)
            desc.fRotationPerSec = 180.f;
    }

    m_fMoveSpeed = desc.fSpeedPerSec;
    m_pTargetTransform = desc.pTargetTransform;

    if (nullptr != desc.pNavigationPrototypeTag)
    {
        m_strNavigationPrototypeTag =
            desc.pNavigationPrototypeTag;
    }

    if (FAILED(__super::Initialize(&desc)))
        return E_FAIL;

    m_pTransformCom->Set_State(
        STATE::POSITION,
        XMVectorSet(
            desc.vPosition.x,
            desc.vPosition.y,
            desc.vPosition.z,
            1.f));

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return Ready_PartObjects();
}

void CValtan::Priority_Update(f32_t fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CValtan::Update(f32_t fTimeDelta)
{
    if (nullptr == m_pNavigationCom)
    {
        m_PathFollower.Cancel();
        m_hasLastPathGoal = false;
        Set_ChaseState(false);
        __super::Update(fTimeDelta);
        return;
    }

    shared_ptr<CTransform> pTargetTransform =
        m_pTargetTransform.lock();
    if (nullptr == pTargetTransform)
    {
        m_PathFollower.Cancel();
        m_hasLastPathGoal = false;
        Set_ChaseState(false);
        __super::Update(fTimeDelta);
        return;
    }

    const vector_t vPosition =
        m_pTransformCom->Get_State(STATE::POSITION);
    const vector_t vTargetPosition =
        pTargetTransform->Get_State(STATE::POSITION);
    const vector_t vHorizontalOffset =
        XMVectorSetY(
            vTargetPosition - vPosition,
            0.f);
    const f32_t fTargetDistance =
        XMVectorGetX(
            XMVector3Length(vHorizontalOffset));

    if (fTargetDistance <= m_fStopDistance)
    {
        m_PathFollower.Cancel();
        m_hasLastPathGoal = false;
        Set_ChaseState(false);
        __super::Update(fTimeDelta);
        return;
    }

    m_fRepathTime -= fTimeDelta;
    if (m_fRepathTime <= 0.f)
    {
        float3_t vCurrentGoal{};
        XMStoreFloat3(
            &vCurrentGoal,
            vTargetPosition);

        const f32_t fGoalDeltaX =
            vCurrentGoal.x -
            m_vLastPathGoal.x;
        const f32_t fGoalDeltaZ =
            vCurrentGoal.z -
            m_vLastPathGoal.z;
        const bool_t isNewGoal =
            false == m_hasLastPathGoal ||
            fGoalDeltaX * fGoalDeltaX +
                fGoalDeltaZ * fGoalDeltaZ >= 0.25f ||
            false == m_PathFollower.Has_Path();

        if (isNewGoal)
        {
            const PATH_RESULT_CODE eResult =
                Request_PathToTarget(
                    vTargetPosition);

            if (PATH_RESULT_CODE::SUCCESS ==
                eResult)
            {
                m_vLastPathGoal =
                    vCurrentGoal;
                m_hasLastPathGoal = true;
            }
            else
            {
                m_PathFollower.Cancel();
                m_hasLastPathGoal = false;
            }
        }

        m_fRepathTime = 0.35f;
    }

    m_PathFollower.Update(
        m_pTransformCom,
        m_fMoveSpeed,
        fTimeDelta);
    Set_ChaseState(
        m_PathFollower.Has_Path());

    __super::Update(fTimeDelta);
}

void CValtan::Late_Update(f32_t fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

#ifdef _DEBUG
    if (m_isNavigationDebugVisible &&
        nullptr != m_pNavigationCom)
    {
        CGameInstance::Get().Add_DebugComponent(
            m_pNavigationCom);
    }
#endif
}

HRESULT CValtan::Render()
{
    return S_OK;
}

HRESULT CValtan::Ready_PartObjects()
{
    CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};
    bodyDesc.pParentMatrix =
        m_pTransformCom->Get_WorldMatrixPtr();
    bodyDesc.pParentState =
        &m_iState;

    if (FAILED(__super::Add_PartObject(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_Body_Valtan"),
        TEXT("Part_Body"),
        &bodyDesc)))
    {
        return E_FAIL;
    }

    m_pBodyModelCom =
        dynamic_pointer_cast<CModel>(
            __super::Get_Component(
                TEXT("Part_Body"),
                TEXT("Com_Model")));

    return nullptr != m_pBodyModelCom ?
        S_OK : E_FAIL;
}

HRESULT CValtan::Ready_Components()
{
    if (m_strNavigationPrototypeTag.empty())
        return S_OK;

    return __super::Add_Component(
        ETOUI(LEVEL::ASSET_TEST),
        m_strNavigationPrototypeTag,
        TEXT("Com_Navigation"),
        m_pNavigationCom);
}

PATH_RESULT_CODE CValtan::Request_PathToTarget(
    fvector_t vGoalPosition)
{
    if (nullptr == m_pNavigationCom ||
        nullptr == m_pTransformCom)
    {
        return PATH_RESULT_CODE::INVALID_GRID;
    }

    return m_PathFollower.Request_Path(
        m_pNavigationCom,
        m_pTransformCom->Get_State(
            STATE::POSITION),
        vGoalPosition);
}

void CValtan::Set_ChaseState(bool_t isChasing)
{
    const uint32_t iNextState =
        isChasing ?
            VALTAN_STATE::CHASE :
            VALTAN_STATE::IDLE;

    if (m_iState == iNextState)
        return;

    m_iState = iNextState;
    if (nullptr != m_pBodyModelCom)
    {
        m_pBodyModelCom->Set_Animation(
            isChasing ?
                "run_battle_1" :
                "idle_battle_1",
            true);
    }
}

unique_ptr<CValtan> CValtan::Create(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext)
{
    auto pInstance = unique_ptr<CValtan>(
        new CValtan(pDevice, pContext));

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CValtan");
        return nullptr;
    }

    return pInstance;
}

shared_ptr<CPrototype> CValtan::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CValtan>(
        new CValtan(*this));

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CValtan");
        return nullptr;
    }

    return pInstance;
}
```

### C6-7. `Client/Public/Loader.h` 선언 추가

`Ready_For_Test_Level2()` 바로 아래에 다음 선언을 추가한다.

```cpp
private:
    HRESULT Ready_For_Level_Logo();
    HRESULT Ready_For_Level_GamePlay();
    HRESULT Ready_For_Level_AssetTest();
    HRESULT Ready_For_Test_Level2();
    HRESULT Ready_LanceMaster_Prototypes(uint32_t iLevelIndex);
```

### C6-8. `Client/Private/Loader.cpp` AssetTest 등록 추가

AssetTest의 공통 Binary Shader Prototype 등록 직후 다음 호출을 추가한다.

```cpp
if (FAILED(Ready_LanceMaster_Prototypes(
    ETOUI(LEVEL::ASSET_TEST))))
    return E_FAIL;
```

목적은 AssetTest에서도 다음 Prototype을 사용할 수 있게 만드는 것이다.

```text
LanceMaster Body Model
LanceMaster Equipment Models
LanceMaster Weapon Model
Part_Body
Part_Equipment
Character
```

### C6-9. `Client/Private/Loader.cpp` 공통 등록 함수 추가

Test2에 중복되어 있던 LanceMaster 등록 코드는 제거하고,
Test2와 AssetTest가 아래 함수를 함께 호출하도록 바꾼다.

```cpp
HRESULT CLoader::Ready_LanceMaster_Prototypes(
    uint32_t iLevelIndex)
{
    const matrix_t preTransform =
        XMMatrixScaling(0.0001f, 0.0001f, 0.0001f);

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_LanceMaster"),
        CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::ANIM,
            "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster.wmodel",
            preTransform))))
    {
        return E_FAIL;
    }

    static const struct
    {
        const tchar_t* pTag;
        const char_t* pPath;
    } EquipmentModels[] =
    {
        {
            TEXT("Prototype_Component_Model_LanceMaster_Upper"),
            "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Upper.wmodel"
        },
        {
            TEXT("Prototype_Component_Model_LanceMaster_Lower"),
            "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Lower.wmodel"
        },
        {
            TEXT("Prototype_Component_Model_LanceMaster_Arm"),
            "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Arm.wmodel"
        },
        {
            TEXT("Prototype_Component_Model_LanceMaster_Shoulder"),
            "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Shoulder.wmodel"
        },
        {
            TEXT("Prototype_Component_Model_LanceMaster_Helmet"),
            "../Bin/Resources/LostArk/Character/LanceMaster/LanceMaster_Helmet.wmodel"
        },
    };

    for (const auto& equipmentModel : EquipmentModels)
    {
        if (FAILED(CGameInstance::Get().Add_Prototype(
            iLevelIndex,
            equipmentModel.pTag,
            CModel::Create(
                m_pDevice,
                m_pContext,
                MODEL::ANIM,
                equipmentModel.pPath,
                preTransform))))
        {
            return E_FAIL;
        }
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_Component_Model_LanceMaster_Weapon"),
        CModel::Create(
            m_pDevice,
            m_pContext,
            MODEL::NONANIM,
            "../Bin/Resources/LostArk/Character/WP_WFLM_00L/WP_WFLM_00L.wmodel",
            XMMatrixScaling(100.f, 100.f, 100.f)))))
    {
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Part_Equipment"),
        CPart_Equipment::Create(
            m_pDevice,
            m_pContext))))
    {
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Part_Body"),
        CPart_Body::Create(
            m_pDevice,
            m_pContext))))
    {
        return E_FAIL;
    }

    if (FAILED(CGameInstance::Get().Add_Prototype(
        iLevelIndex,
        TEXT("Prototype_GameObject_Character"),
        CCharacter::Create(
            m_pDevice,
            m_pContext))))
    {
        return E_FAIL;
    }

    return S_OK;
}
```

Test2의 최종 호출 위치:

```cpp
if (FAILED(Ready_LanceMaster_Prototypes(
    ETOUI(LEVEL::TEST_LEVEL2))))
    return E_FAIL;

lstrcpy(m_szLoadingText, TEXT("Test Level 2 loading complete."));
m_isFinished = true;

return S_OK;
```

### C6-10. Engine 파일과 Project 파일

이번 구현에서는 다음 파일을 수정하지 않는다.

```text
Engine/Public/NavPathFollower.h
Engine/Private/NavPathFollower.cpp
Engine/Public/Navigation.h
Engine/Private/Navigation.cpp
Engine/Public/PathFinder.h
Engine/Private/PathFinder.cpp
```

새 C++ 파일도 만들지 않는다.
따라서 아래 Project 등록 변경도 없다.

```text
Client/Default/Client.vcxproj       변경 없음
Client/Default/Client.vcxproj.filters 변경 없음
Engine/Default/Engine.vcxproj       변경 없음
Engine/Default/Engine.vcxproj.filters 변경 없음
```

---

## C7. 반영 순서와 검증

### C7-1. 반영 순서

```text
1. Loader 공통 LanceMaster Prototype 함수 분리
2. AssetTest에 Character Prototype 등록
3. Character에 Navigation/Follower 추가
4. AssetTest에서 Character 먼저 생성
5. Character Transform을 Valtan 생성 인자에 연결
6. LMB를 Character 이동으로 연결
7. Valtan public 클릭 이동 제거
8. Valtan 내부 재탐색 추적으로 교체
9. F5로 두 Navigation 경로 디버그 확인
```

### C7-2. 빌드 검증

최종 반영 후 팀 규칙에 따라 아래 순서로 확인한다.

```text
1. Engine x64 Debug
2. UpdateLib.bat Debug
3. Client x64 Debug
4. Engine x64 Release
5. UpdateLib.bat Release
6. Client x64 Release
```

구현 코드는 실제 작업 트리에서 다음 검증을 완료했다.

```text
Engine x64 Debug       성공
UpdateLib.bat Debug    성공
Client x64 Debug       성공
Client.exe 링크        성공
Engine x64 Release     성공
UpdateLib.bat Release  성공
Client x64 Release     성공
Client.exe 링크        성공
Debug Client 시작      성공
F2 AssetTest 진입      성공
Character/Valtan 생성  성공
LMB 입력 후 응답 유지  성공
```

좌클릭 계약은 `DIM::LB → Picking → CCharacter::Request_Move()`로 반영했으며,
Valtan에는 마우스 입력용 public 이동 API가 남아 있지 않다.

실행 시 상대 리소스 경로 기준은 `Client/Default`이다.
`Client/Bin/Debug`를 working directory로 사용하면
`../Bin/ShaderFiles/...`가 잘못 해석되므로 실행 기준으로 사용하지 않는다.

### C7-3. 런타임 검증

#### 시작 상태

```text
AssetTest 진입 성공
Character가 (151.25, 22.96835, -121.75)에 생성
Valtan이 (156.25, 22.99751, -121.75)에 생성
두 객체 모두 바닥 위에 표시
```

#### 플레이어

```text
LMB 한 번 → Picking 지점으로 이동
LMB 누르고 유지 → 매 프레임 경로가 재생성되지 않음
이동 중 Run
도착 후 Idle
막힌 셀 클릭 → 크래시 없음
막힌 셀 클릭 전 기존 경로가 있었다면 기존 경로 유지
```

#### Valtan

```text
Character가 2.5m 밖에 있으면 추적 시작
추적 중 run_battle_1
2.5m 안으로 들어오면 정지
정지 후 idle_battle_1
Character가 다시 멀어지면 재추적
목표가 사라지면 weak_ptr lock 실패 후 안전하게 Idle
경로 탐색 실패 시 오래된 경로 취소
```

#### Debug

```text
F5 → NavGrid와 두 객체의 최근 경로 표시
다시 F5 → 표시 해제
UI가 입력을 막을 때 LMB/F5 명령이 소비되지 않음
```

---

## C8. 완료 기준과 다음 단계

### 이번 단계 완료 기준

```text
LMB로 Character가 NavGrid A* 경로를 따라 이동한다.
Valtan이 Character Transform을 목표로 자동 추적한다.
Character와 Valtan은 각각 독립된 Navigation clone/Follower를 가진다.
Engine의 NavPathFollower는 Client 지식 없이 그대로 유지된다.
Test2의 Character Prototype 등록은 중복 없이 유지된다.
Debug/Release 빌드와 AssetTest 런타임 검증이 끝난다.
```

### 다음 단계: 보스 패턴 결합

이번 이동 기반이 확인된 뒤 다음 구조를 추가한다.

```text
CValtan::Update()
  ↓
현재 패턴/상태 판단
  ├─ CHASE
  │    └─ NavPathFollower 사용
  │
  ├─ PATTERN_POSITIONING
  │    └─ 패턴 시작 위치까지 NavPathFollower 사용
  │
  └─ PATTERN_EXECUTION
       ├─ m_PathFollower.Cancel()
       └─ Pattern 전용 이동 또는 Root Motion 사용
```

패턴 시스템이 실제로 들어오는 시점에는 Valtan의 큰 `Update()`를 다음 단위로 분리한다.

```cpp
void Update_Target();
void Update_Behavior(f32_t fTimeDelta);
void Update_Chase(f32_t fTimeDelta);
void Update_Pattern(f32_t fTimeDelta);
```

지금은 상태가 `IDLE/CHASE` 두 개뿐이므로 한 단계 일찍 추상화하지 않고,
실제 첫 패턴을 붙일 때 책임을 분리한다.
