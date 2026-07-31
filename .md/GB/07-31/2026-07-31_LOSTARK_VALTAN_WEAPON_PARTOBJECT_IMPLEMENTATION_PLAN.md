# LostArk 발탄 무기 PartObject 직접 구현 계획서

```text
출력 모드: CODE_WITH_EXPLANATION
C1~C8: OFF
문제 해결 ①~⑤: OFF
자료구조·알고리즘: ON
```

## 1. 최종 반영 코드

아래 코드는 사용자가 직접 반영할 최종 형태다. 기존 파일은 변경되는 선언과 함수를
중간 생략 없이 전부 제시한다.

### 1.1 `Client/Public/Part_Equipment.h`

`PART_EQUIPMENT_DESC` 전체를 다음 코드로 교체한다.

```cpp
typedef struct tagPartEquipmentDesc : public CPartObject::PARTOBJECT_DESC
{
	uint32_t iPrototypeLevelIndex = {};
	wstring_t strModelTag;
	wstring_t strShaderTag;

	/* The body's model owns the animated skeleton this part reads or shares. */
	shared_ptr<Engine::CModel> pSkeletonModel = { nullptr };

	/* nullptr means skinned equipment; a name means one-bone socket mode. */
	const char_t* pSocketBoneName = { nullptr };

	/* Optional body-part visual correction inserted after the socket bone. */
	const float4x4_t* pSocketRootMatrix = { nullptr };
} PART_EQUIPMENT_DESC;
```

Component와 소켓 참조를 보관하는 private 멤버 블록 전체를 다음 코드로 교체한다.

```cpp
private:
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	shared_ptr<CModel> m_pSkeletonModelCom = { nullptr };
	const char_t* m_pSocketBoneName = { nullptr };
	const float4x4_t* m_pSocketRootMatrix = { nullptr };
```

### 1.2 `Client/Private/Part_Equipment.cpp`

`Initialize` 함수 전체를 다음 코드로 교체한다.

```cpp
HRESULT CPart_Equipment::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_EQUIPMENT_DESC*>(pArg);
	m_pSkeletonModelCom = pDesc->pSkeletonModel;
	m_pSocketBoneName = pDesc->pSocketBoneName;
	m_pSocketRootMatrix = pDesc->pSocketRootMatrix;

	/* Both modes require the body model: socket mode reads one bone,
	and skinned mode borrows the complete bone palette. */
	if (nullptr == m_pSkeletonModelCom)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	return S_OK;
}
```

`Update` 함수 전체를 다음 코드로 교체한다.

```cpp
void CPart_Equipment::Update(f32_t fTimeDelta)
{
	matrix_t ChildMatrix =
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	if (nullptr != m_pSocketBoneName)
	{
		ChildMatrix = ChildMatrix *
			m_pSkeletonModelCom->Get_BoneMatrix(m_pSocketBoneName);

		if (nullptr != m_pSocketRootMatrix)
			ChildMatrix = ChildMatrix *
				XMLoadFloat4x4(m_pSocketRootMatrix);
	}

	__super::Update_CombinedWorldMatrix(ChildMatrix);
}
```

### 1.3 `Client/Private/Valtan.cpp`

기존 include 블록에 다음 두 줄을 추가한다.

```cpp
#include "Part_Equipment.h"
#include "Transform.h"
```

`Ready_PartObjects` 함수 전체를 다음 코드로 교체한다.

```cpp
HRESULT CValtan::Ready_PartObjects()
{
	CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};

	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;

	if (FAILED(__super::Add_PartObject(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Body_Valtan"),
		TEXT("Part_Body"),
		&bodyDesc)))
		return E_FAIL;

	m_pBodyModelCom = dynamic_pointer_cast<CModel>(
		__super::Get_Component(TEXT("Part_Body"), TEXT("Com_Model")));

	const shared_ptr<CTransform> pBodyVisualRoot =
		dynamic_pointer_cast<CTransform>(
			__super::Get_Component(
				TEXT("Part_Body"),
				g_strTransformComTag));

	if (nullptr == m_pBodyModelCom || nullptr == pBodyVisualRoot)
		return E_FAIL;

	CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
	weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	weaponDesc.iPrototypeLevelIndex = ETOUI(LEVEL::ASSET_TEST);
	weaponDesc.strModelTag = TEXT("Prototype_Component_Model_ValtanWeapon");
	weaponDesc.strShaderTag = TEXT("Prototype_Component_Shader_VtxMeshBinary");
	weaponDesc.pSkeletonModel = m_pBodyModelCom;
	weaponDesc.pSocketBoneName = "b_wp_r_01";
	weaponDesc.pSocketRootMatrix = pBodyVisualRoot->Get_WorldMatrixPtr();

	return __super::Add_PartObject(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Part_Equipment"),
		TEXT("Part_Weapon_R"),
		&weaponDesc);
}
```

### 1.4 `Client/Private/Loader.cpp`

현재 WModel의 단위와 소켓 scale에 맞춰 무기 Prototype의 scale을 분리한다.

```cpp
if (FAILED(CGameInstance::Get().Add_Prototype(
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Prototype_Component_Model_ValtanWeapon"),
    CModel::Create(m_pDevice, m_pContext,
        MODEL::NONANIM,
        "../Bin/Resources/LostArk/Character/Valtan/ValtanWeapon.wmodel",
        XMMatrixScaling(100.f, 100.f, 100.f)))))
    return E_FAIL;
```

## 2. 변경 범위

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Part_Equipment.h` | Body 시각 루트 행렬 전달 계약 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Part_Equipment.cpp` | Socket 뒤에 Body 시각 루트를 결합 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Valtan.cpp` | Body 참조 획득 후 Weapon Part Clone |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | 현재 무기 WModel과 소켓 기준계에 맞는 Model scale 등록 |

새 C++ 파일은 만들지 않는다. 현재 단계에서는 범용 `CPart_Equipment`의 socket mode를
발탄에도 사용한다.

## 3. 현재 상태

- `ValtanWeapon.wmodel`과 D/E/N/S 텍스처는 추출·변환되어 있다.
- Loader에는 `Prototype_Component_Model_ValtanWeapon`이 등록되어 있다.
- 범용 `Prototype_GameObject_Part_Equipment`도 ASSET_TEST Level에 등록되어 있다.
- 발탄 몸체 스켈레톤에는 `b_wp_r_01` 본이 존재한다.
- `CValtan::Ready_PartObjects()`는 현재 Body만 생성한다.
- 이전에 자동 반영됐던 발탄 Weapon Part 연결은 제거된 상태다.
- `Part_Equipment.h/.cpp`도 기존 상태로 복구되어 Git diff가 없다.

현재 확인된 Runtime 배치는 다음과 같다.

```text
C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk/Character/Valtan/
├─ MN_RPBF_01.wmodel
├─ ValtanWeapon.wmodel
├─ anims/
└─ textures/
   ├─ wp_mn_rpbf_01_d.dds
   ├─ wp_mn_rpbf_01_e.dds
   ├─ wp_mn_rpbf_01_n.dds
   ├─ wp_mn_rpbf_01_s.dds
   ├─ wp_mn_rpbf_01-1_d.dds
   ├─ wp_mn_rpbf_01-1_e.dds
   ├─ wp_mn_rpbf_01-1_n.dds
   └─ wp_mn_rpbf_01-1_s.dds
```

현재 파일 시스템에서는 Runtime Character 아래의 별도 `Valtan_weapon` 폴더는 확인되지
않았다. Loader도 `Valtan/ValtanWeapon.wmodel`을 가리키므로 현재 문서는 이 실측 경로를
기준으로 한다.

## 4. 전체 원리와 호출 흐름

### 4.1 한 문장 본질

Weapon PartObject는 애니메이션을 재생하지 않고, Body가 현재 프레임에 계산한
`b_wp_r_01` Bone 행렬을 읽어 자기 월드 행렬을 만드는 소비자다.

### 4.2 생성 흐름

```text
CLoader::Ready_For_Level_AssetTest
-> Model / Shader / Part Prototype 등록
-> CLevel_AssetTest::Ready_Valtan
-> CGameInstance::Add_GameObject_to_Layer
-> CValtan::Clone
-> CValtan::Initialize
-> CValtan::Ready_PartObjects
-> Body Part Clone
-> Body Model / Transform 조회
-> Weapon CPart_Equipment Clone
```

Loader의 `Add_Prototype`은 재사용할 원형을 준비할 뿐이다. 실제 발탄 무기 인스턴스는
`CValtan::Ready_PartObjects()`가 `Add_PartObject`를 호출할 때 생성된다.

### 4.3 프레임 흐름

```text
CValtan::Update
-> CContainerObject::Update
-> CBody_Valtan::Update
-> CModel::Play_Animation
-> 모든 Bone Combined Matrix 갱신
-> CPart_Equipment::Update
-> Get_BoneMatrix("b_wp_r_01")
-> Weapon Combined World 계산
-> CPart_Equipment::Late_Update
-> RenderGroup 등록
-> CPart_Equipment::Render
```

`CContainerObject`는 `m_PartObjects`라는 `std::map`을 순회한다. `Part_Body`가
`Part_Weapon_R`보다 사전순으로 앞서므로 Body가 현재 프레임 Bone을 먼저 갱신하고 Weapon이
그 결과를 읽는다.

## 5. 코드 아래 설명

### 5.1 `PART_EQUIPMENT_DESC`

`PART_EQUIPMENT_DESC`는 `CValtan`이 알고 있는 조립 정보를 `CPart_Equipment` Clone에
넘기는 입력 구조체다.

| 변수 | 값 또는 owner | 의미 |
|---|---|---|
| `pParentMatrix` | `CValtan::m_pTransformCom` | 발탄 전체의 이동·회전·스케일 |
| `iPrototypeLevelIndex` | `LEVEL::ASSET_TEST` | Component Prototype을 찾을 Level |
| `strModelTag` | `Prototype_Component_Model_ValtanWeapon` | Clone할 무기 CModel |
| `strShaderTag` | `Prototype_Component_Shader_VtxMeshBinary` | 정적 메시 셰이더 |
| `pSkeletonModel` | Body Part의 `CModel` Clone | 현재 애니메이션 Bone의 owner |
| `pSocketBoneName` | `b_wp_r_01` | 따라갈 오른손 무기 소켓 |
| `pSocketRootMatrix` | Body Part의 CTransform 행렬 | Body에만 적용된 -90도 축 보정 |

`pSkeletonModel`은 Prototype Model이 아니라 `Part_Body`가 실제로 사용하는 Clone이어야
한다. 그래야 `Play_Animation()`이 갱신한 바로 그 Bone 행렬을 읽는다.

### 5.2 `CPart_Equipment::Initialize`

입력은 `void*`지만 실제 타입은 `PART_EQUIPMENT_DESC*`다.

```text
pArg
-> PART_EQUIPMENT_DESC로 해석
-> Body Model, Bone Name, Body Root Matrix 저장
-> CPartObject::Initialize
-> Parent Matrix 주소 저장 + 기본 Transform 생성
-> Ready_Components
-> Shader/Weapon Model Clone
```

`pSkeletonModel`이 null이면 소켓 본을 읽을 생산자가 없으므로 바로 `E_FAIL`을 반환한다.

### 5.3 `CPart_Equipment::Update`

이 함수는 애니메이션 시간을 진행하지 않는다. `fTimeDelta`를 소비하는 곳은
`CBody_Valtan::Update -> CModel::Play_Animation`이다.

Weapon Update는 다음 값만 조립한다.

```text
WeaponLocal
-> SocketBoneCombined
-> BodyVisualRoot
-> CPartObject::Update_CombinedWorldMatrix
-> ValtanWorld
```

프레임당 Bone 이름 검색 1회와 행렬 곱 최대 3회가 발생한다. 추가 heap 할당은 없다.

### 5.4 `CValtan::Ready_PartObjects`

Body를 먼저 만드는 이유는 Weapon Descriptor에 Body Clone의 Model과 Transform이 필요하기
때문이다.

```text
1. Body Descriptor 작성
2. Body Part Clone
3. Part_Body의 Com_Model 조회
4. Part_Body의 Com_Transform 조회
5. 필수 참조 null 검사
6. Weapon Descriptor 작성
7. Weapon Part Clone
```

Level/Layer에서 Body를 다시 검색하지 않고 `CValtan::Get_Component(PartTag, ComponentTag)`를
사용한다. Body와 Weapon의 owner가 `CValtan` 자신이기 때문이다.

### 5.5 `pSocketRootMatrix`가 필요한 이유

`CBody_Valtan::Initialize`에는 다음 코드가 이미 있다.

```cpp
m_pTransformCom->Rotation(0.f, -90.f, 0.f);
```

이 회전은 Gameplay Parent인 `CValtan`이 아니라 Body Part에만 들어 있다. 기존
`CPart_Equipment`의 공식은 다음과 같다.

```text
WeaponLocal * SocketBone * ValtanWorld
```

이 상태에서는 Body가 받은 -90도 보정을 Weapon이 받지 못할 수 있다. 따라서 Body
Transform 주소를 받아 Bone 뒤, Parent World 앞에 넣는다.

```text
WeaponLocal
* SocketBoneCombined
* BodyVisualRoot(-90° Y)
* ValtanWorld
```

`pSocketRootMatrix`는 `shared_ptr<CTransform>`이 아니라 기존 `CPartObject::m_pParentMatrix`와
같은 비소유 행렬 포인터다. Body와 Weapon은 모두 `CValtan::m_PartObjects`가 소유하고
Body가 먼저 생성되므로 Container 수명 동안 주소가 유효해야 한다.

## 6. 자료구조와 수명

### 6.1 `m_PartObjects`

```text
타입: map<const wstring_t, shared_ptr<CPartObject>>
owner: CValtan의 CContainerObject 기반부
이번 원소: Part_Body, Part_Weapon_R
생성: CValtan::Ready_PartObjects
파괴: CValtan 파괴 시
writer: CContainerObject::Add_PartObject
reader: Priority_Update, Update, Late_Update
불변식: Part tag 중복 금지, Body가 Weapon보다 먼저 Update
```

### 6.2 `m_pParentMatrix`

```text
타입: const float4x4_t*
owner: CValtan의 CTransform
reader: CPartObject::Update_CombinedWorldMatrix
의미: 발탄의 최신 World Matrix 주소
불변식: Parent가 Part보다 오래 살아야 한다.
```

행렬 복사본이 아니라 주소를 저장하므로 발탄이 이동하거나 회전해도 다음 Update에서 최신
값을 읽는다.

### 6.3 `m_pSkeletonModelCom`

```text
타입: shared_ptr<CModel>
owner: Body Part Component map, Weapon도 shared ownership
writer: CPart_Equipment::Initialize
reader: CPart_Equipment::Update
의미: 현재 발탄 애니메이션 Bone을 소유한 Model Clone
```

Body가 먼저 파괴되는 상황에서도 Weapon이 Update 중 즉시 dangling pointer가 되지는 않는다.
Container의 정상 파괴 순서 안에서 함께 정리되며 순환 소유는 없다.

### 6.4 Bone Name

```text
타입: const char_t*
실제 값: "b_wp_r_01"
수명: 문자열 리터럴이므로 Part보다 길다.
검색: CModel::Get_BoneMatrix의 bone vector 선형 검색
실패: 일치하는 본이 없으면 Identity 반환
```

Bone 오타는 `E_FAIL`이 아니라 Identity로 조용히 실패한다. 무기가 손이 아닌 원점이나 발탄
중심에 보이면 가장 먼저 검색 iterator를 확인한다.

## 7. 행렬과 스케일

### 7.1 최종 행렬의 책임

| 행렬 | 담당하는 값 |
|---|---|
| `WeaponLocal` | 무기 자체 로컬 Transform |
| `SocketBoneCombined` | 현재 애니메이션의 손 위치·회전 |
| `BodyVisualRoot` | 수입 모델과 엔진 축 차이인 -90도 Y |
| `ValtanWorld` | 보스 위치·방향·기본 1.5 scale |

### 7.2 현재 `lostArkAssetPreTransform` 주의점

현재 Loader 값은 다음과 같다.

```cpp
const matrix_t lostArkAssetPreTransform =
    XMMatrixScaling(0.0001f, 0.0001f, 0.0001f);
```

Body와 Weapon에 같은 변수를 전달해도 적용 위치는 다르다.

- `MODEL::ANIM` Body에서는 PreTransform이 Bone Combined 계산에 들어간다.
- `MODEL::NONANIM` Weapon에서는 PreTransform이 정적 정점에 직접 bake된다.

현재 무기 원본 X 길이는 약 `3.72556`, 발탄 소켓의 유효 basis scale은 약 `0.01`, 발탄
부모 기본 scale은 `1.5`다. 현재 값을 그대로 대입하면 다음 정도다.

```text
3.72556 * 0.0001 * 0.01 * 1.5
= 약 0.00000558834
```

따라서 현재 WModel과 `lostArkAssetPreTransform` 조합은 무기를 거의 보이지 않게 만든다.
현재 WModel을 그대로 사용하는 구현에서는 Weapon Model Prototype에 `100.f`를 적용해
소켓의 유효 `0.01` scale을 상쇄한다. 실행 후 행렬별 basis scale을 확인하고, 장기적으로는
다음 중 한 곳에서만 단위를 확정한다.

1. 현재 WModel 유지 + Weapon Model Prototype의 PreTransform 조정
2. 무기 WModel recook scale 조정 + Loader 공통 scale 유지

PartObject Transform에서 큰 배율을 다시 곱해 임시로 맞추면 단위의 owner가 두 군데가 되므로
사용하지 않는다.

## 8. 직접 적용 순서

### Gate 0. 리소스와 Prototype

1. `ValtanWeapon.wmodel` 실제 위치를 확인한다.
2. `ModelAssetConverter info`에서 `sections=2`, `animations=0`, `skeleton=no`를 확인한다.
3. Loader의 `CModel::Create` 결과가 null이 아닌지 확인한다.
4. 아직 Weapon Part를 생성하지 않은 상태에서 AssetTest가 정상 진입하는지 확인한다.

### Gate 1. 기본 Socket 연결

1. `Valtan.cpp`에 include와 Weapon Descriptor를 직접 추가한다.
2. 기존 `CPart_Equipment` 공식인 `WeaponLocal * SocketBone * ValtanWorld`로 먼저 실행한다.
3. 무기 표시 여부, 크기, 손 추종 여부, 90도 축 오차를 각각 기록한다.

### Gate 2. Body 시각 루트

1. `Part_Equipment.h`에 `pSocketRootMatrix`를 추가한다.
2. `Initialize`에서 주소를 저장한다.
3. `Update`에서 SocketBone 뒤에 곱한다.
4. Body와 Weapon의 시각 축이 일치하는지 확인한다.

### Gate 3. 단위 확정

1. WeaponLocal basis 길이를 기록한다.
2. SocketBone basis 길이를 기록한다.
3. BodyVisualRoot와 ValtanWorld basis 길이를 기록한다.
4. 최초로 예상과 다른 행렬의 owner에서만 scale을 수정한다.

## 9. Breakpoint와 Watch

### 9.1 생성 흐름

Breakpoint:

```text
CLoader::Ready_For_Level_AssetTest
CValtan::Ready_PartObjects
CPart_Equipment::Initialize
CPart_Equipment::Ready_Components
```

Watch:

```text
weaponDesc.strModelTag
weaponDesc.pSkeletonModel
weaponDesc.pSocketBoneName
weaponDesc.pSocketRootMatrix
m_pModelCom
m_pSkeletonModelCom
```

### 9.2 프레임 순서

Breakpoint:

```text
CBody_Valtan::Update
CModel::Play_Animation
CPart_Equipment::Update
```

한 프레임 안에서 Body Update가 먼저 멈추고 Weapon Update가 나중에 멈춰야 한다.

### 9.3 행렬 분리

Watch에서 다음 네 값을 따로 기록한다.

```text
WeaponLocal
SocketBoneCombined
BodyVisualRoot
ValtanWorld
```

각 행렬의 RIGHT/UP/LOOK 벡터 길이와 POSITION을 확인한다. 최종 행렬만 보면 어느 단계에서
scale이나 rotation이 틀렸는지 구분하기 어렵다.

## 10. 증상별 최초 확인 위치

| 증상 | 최초 확인 위치 | 이유 |
|---|---|---|
| Weapon Part Clone 실패 | `Ready_Components`의 Model tag | Prototype 또는 경로 불일치 |
| 무기가 전혀 안 보임 | Weapon PreTransform | 현재 `0.0001`로 지나치게 작을 가능성 |
| 무기가 발탄 중심에 있음 | `Get_BoneMatrix` 검색 결과 | Bone 오타 시 Identity 반환 |
| 몸체와 90도 어긋남 | `pSocketRootMatrix` 곱 위치 | Body에만 있는 시각 루트 누락 |
| 한 프레임 늦게 따라감 | Part tag 정렬 순서 | Weapon이 Body보다 먼저 Update |
| 이동만 따라가고 팔은 안 따라감 | `pSkeletonModel` 주소 | Body Clone이 아닌 다른 Model 참조 |
| 회색 또는 흰색 무기 | WModel material path와 textures | 상대 경로 불일치 |

## 11. 프로젝트 등록과 검증

- 새 C++ 파일 없음
- `Client.vcxproj` 변경 없음
- `Client.vcxproj.filters` 변경 없음
- Engine public header 변경 없음
- 기존 `Part_Equipment.h/.cpp`, `Valtan.cpp`는 이미 프로젝트에 등록되어 있음
- WModel/DDS는 Git source가 아니라 공유 리소스 팩으로 배포

최소 검증 순서:

```text
1. Client x64 Debug
2. AssetTest 실행
3. Idle/Run/회전에서 손 추종 확인
4. Breakpoint로 Body -> Weapon Update 순서 확인
5. 행렬별 scale 실측
6. Client x64 Release
```

완료 조건:

- Body와 Weapon Part가 각각 한 개만 생성된다.
- Body가 현재 프레임 Bone을 갱신한 뒤 Weapon이 읽는다.
- `b_wp_r_01`을 Idle/Run/회전에서 정확히 따라간다.
- BodyVisualRoot와 ValtanWorld를 중복 적용하지 않는다.
- scale의 owner가 cook 또는 Loader 한 곳으로 확정된다.
- 코드, Watch 값, 화면 결과를 같은 행렬 흐름으로 설명할 수 있다.
