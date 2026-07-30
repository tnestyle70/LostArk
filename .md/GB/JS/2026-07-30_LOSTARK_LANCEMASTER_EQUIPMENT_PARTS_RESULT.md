# LostArk 창술사(LanceMaster) 장비 파츠 분리 결과

작성자: JS · 2026-07-30

창술사를 "몸 + 장비 파츠"로 분리했다. 방어구를 개별 `.wmodel`로 쿠킹하고,
몸의 본 팔레트를 빌려 쓰는 `CPart_Equipment`를 만들어 붙였다.
**엔진(`Engine/`) 변경 없이 클라이언트와 에셋만으로 끝냈다.**

전제가 된 추출 작업은 [2026-07-29_LOSTARK_LANCEMASTER_ASSET_EXTRACTION_RESULT.md](2026-07-29_LOSTARK_LANCEMASTER_ASSET_EXTRACTION_RESULT.md)에 있다.

## 1. 왜 했나

기존 `LanceMaster.wmodel`은 서브메시 17개짜리 단일 모델이었고, 그 안에
**캐릭터가 두 벌** 들어 있었다.

| 서브메시 | 내용 |
|---|---|
| 0~4 | `pc_flm_00_*` 방어구 5파츠 |
| 5~9 | `pc_ft_00_face_sk` / `pc_ft_00_hair_sk` (얼굴·머리 대체 옵션) |
| 10~16 | `pc_ft_00_sk` 기본 캐릭터 (몸 + 얼굴 + 눈 + 속눈썹 + 머리08) |

`pc_ft_00_sk`가 이미 얼굴·머리를 포함한 완결된 캐릭터인데 `build_flm.py`가
별도 얼굴·머리 메시를 그 위에 얹어(`build_flm.py:14-16`) 같은 자리에 두 벌이
그려지고 있었다. 장비 교환을 붙이려면 어차피 몸과 방어구가 분리되어야 하므로
같이 해결했다.

## 2. 재쿠킹 — FBX당 메시 1개

UModel 재추출은 하지 않았다. `_export_flm_psk`의 psk/psa로 충분했다.

`build_flm.py`를 `build_flm_part.py`로 파라미터화했다. 핵심 규칙은 하나다.

> **모든 FBX가 마스터 아마추어 + 메시 정확히 1개를 갖는다.**

마스터 아마추어는 항상 `pc_flm_00_upper_sk_loc_int.psk`(221본)에서 가져오고,
대상이 마스터가 아니면 마스터의 메시는 지운 뒤 대상 psk를 임포트해 재바인딩한다.
이렇게 하면 모든 산출물의 본 배열이 구조적으로 동일해진다.

```powershell
blender --background --factory-startup --python build_flm_part.py -- `
  <PSK_ROOT> <target> <out.fbx> [--anim]
# target: body | upper | lower | arm | shoulder | helmet | face | facehigh | hair
```

| 산출물 | 소스 메시 | 애니 | 크기 |
|---|---|---|---|
| `LanceMaster.wmodel` | `pc_ft_00_sk` | 223 | 108.5 MB |
| `LanceMaster_Upper.wmodel` | `pc_flm_00_upper_sk_loc_int` | 0 | 0.5 MB |
| `LanceMaster_Arm.wmodel` | `pc_flm_00_arm_sk` | 0 | 0.3 MB |
| `LanceMaster_Lower.wmodel` | `pc_flm_00_lower_sk_loc_int` | 0 | 0.2 MB |
| `LanceMaster_Helmet.wmodel` | `pc_flm_00_helmet_sk` | 0 | 0.1 MB |
| `LanceMaster_Shoulder.wmodel` | `pc_flm_00_shoulder_sk` | 0 | 0.1 MB |

방어구 5개 합계 1.2 MB다. 몸의 108.5 MB 중 약 98%가 애니메이션이므로,
**방어구에 애니메이션을 넣지 않는 것이 용량의 핵심**이다.

컨버터 호출은 다음 형태다.

```powershell
ModelAssetConverter.exe <part.fbx> -o <part.wmodel> `
  --texture-root "C:\Users\95jus\Downloads\umodel_win32\_export_flm_psk"
```

## 3. 스켈레톤 공유가 성립하는 이유 — 이게 핵심이다

방어구가 애니메이션 없이도 몸을 따라 변형되는 근거다.

`.wmodel`(쿠킹 경로)에서는 **메시별 본 부분집합이라는 게 없다.**
`CMesh::Initialize_Prototype`(Mesh.cpp:107-114)이 이렇게 만든다.

```cpp
m_iNumBones = skeleton.bones.size();          // 전체
for (uint32_t i = 0; i < m_iNumBones; ++i)
{
    m_BoneIndices.push_back(i);               // 항등 매핑
    m_OffsetMatrices.push_back(skeleton.bones[i].inverseBind);
}
```

팔레트는 `m_OffsetMatrices[i] * Bones[i].Combined`(Mesh.cpp:158-163)이므로
**스켈레톤만으로 결정되고, 같은 리그로 구운 모델끼리는 값이 완전히 동일하다.**
따라서 방어구는 팔레트를 계산할 필요 없이 몸이 만들어 둔 것을 그대로 쓰면 된다.

`CModel::Bind_BoneMatrices`가 public이라(Model.h:62) 엔진 변경 없이 호출 가능하다.

> 참고: Assimp 경로는 메시별로 본 이름을 매칭해 부분집합을 만든다.
> 위 성질은 **쿠킹 경로 전용**이다.

### 본 배열 구성

산출물은 전부 224본이고 구성은 이렇다.

```
[  0] RootNode
[  1] flm                  ← 아마추어 오브젝트
[  2] b_root
  ...                      ← 실제 본 221개
[222] b_cameratarget
[223] <메시 오브젝트 노드>   ← FBX 메시 노드가 본으로 들어온다
```

FBX의 메시 노드가 본 배열에 섞인다. 이전 통합 모델은 메시 8개라 231본이었다.
메시를 1개로 통일했으므로 전부 224본이 되고, 실제 본이 `2..222`로 동일한
위치에 놓인다. 스킨 가중치는 실제 본만 참조하므로 정렬이 보장된다.

## 4. 파츠 클래스 — `CPart_Equipment` / `CPart_Body`

슬롯별 클래스를 만들지 않았다. 슬롯은 데이터이고, 실제로 갈리는 동작은
**소켓 부착이냐 스킨드냐** 둘뿐이라 조건문 두 개로 충분하다.

```cpp
void CPart_Equipment::Update(f32_t fTimeDelta)
{
    matrix_t ChildMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
    if (nullptr != m_pSocketBoneName)
        ChildMatrix = ChildMatrix * m_pSkeletonModelCom->Get_BoneMatrix(m_pSocketBoneName);
    __super::Update_CombinedWorldMatrix(ChildMatrix);
}

HRESULT CPart_Equipment::Render()
{
    if (nullptr == m_pSocketBoneName &&
        FAILED(m_pSkeletonModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", 0)))
        return E_FAIL;
    // ...메시 루프
}
```

`pSocketBoneName == nullptr`이면 스킨드다. 셰이더·모델은 DESC의 프로토타입
태그로 갈리므로 코드 분기가 없다. `pSkeletonModel`은 양쪽 다 쓴다 — 소켓은
본 행렬을 뽑으려고, 스킨드는 팔레트를 빌리려고.

팔레트 바인딩은 메시 루프 **밖**에 한 번만 둔다. 3절의 성질 때문에
어떤 메시 인덱스를 넘겨도 같은 팔레트가 나온다.

몸은 `CPart_Body`로 따로 둔다. **애니메이션 시계를 소유한다**는 점이 다르기
때문이다(`Play_Animation`을 매 프레임 돌리는 유일한 파츠). 나머지 파츠는 그 결과를
읽기만 한다. 서브메시 마스크도 여기 있다(7절).

무기도 지금은 `CPart_Equipment`의 소켓 모드를 쓴다. 히트 윈도우와 트레일이 붙어
동작이 실제로 갈리는 시점에 전용 클래스로 분리한다.

### 파츠 태그 정렬 — 함정

`CContainerObject::m_PartObjects`는 `std::map`이라 **삽입 순서가 아니라 태그
알파벳 순으로** 갱신된다(ContainerObject.h:31). 레거시 시절 무기가 제대로 붙던 것은
`Part_Body` < `Part_Weapon_R`이라는 우연이었고 의도된 순서 제어가 아니었다.

그래서 태그에 순서를 박았다.

```text
Part_00_Body  →  Part_10_Equip_*  →  Part_90_Weapon_R
```

스킨드 파츠는 본을 `Render()`에서만 읽으므로 순서와 무관하지만, 소켓 파츠는
`Update()`에서 읽으므로 몸보다 먼저 갱신되면 한 프레임 밀린다(빠른 모션에서
파츠만 덜덜 떠는 형태로 보인다).
**소켓 파츠를 추가할 때는 태그가 `Part_00_Body` 뒤에 정렬되는지 확인할 것.**

### 태그를 바꾸면 문자열로 찾는 쪽이 조용히 깨진다

`Part_Body` → `Part_00_Body`로 바꾸면서 Animation Tool이 캐릭터를 못 찾게 됐다.
`Animation_Tool.cpp`의 `Resolve_Model`이 레이어 → 파츠 → 컴포넌트를 문자열로
조회하는데, 파츠 태그에서 끊겨 `nullptr`이 반환됐다.

**태그는 문자열이라 컴파일러가 잡아주지 않는다.** 빌드가 통과해도 런타임에 조용히
`nullptr`이 된다. 클래스 이름만 grep하고 끝내면 놓친다.

현재 태그를 쓰는 곳은 이렇게 정리된다.

| 태그 | 쓰는 곳 |
|---|---|
| `Layer_Player` | `Level_Test2`, `Animation_Tool` |
| `Part_00_Body` | `CCharacter`, `Animation_Tool` |
| `Part_10_Equip_*` | `Logic_LanceMaster` (SPEC) |
| `Part_90_Weapon_R` | `CCharacter` |
| `Com_Model` / `Com_Shader` | 각 파츠, `Animation_Tool` |

## 5. 디퓨즈 알파 함정 — 다른 캐릭터에서도 밟는다

재쿠킹 후 얼굴이 렌더되지 않았다. 원인은 재쿠킹과 무관한 기존 문제였다.

**로스트아크의 디퓨즈 TGA는 알파를 불투명도로 쓰지 않는다.** 스펙큘러 마스크
같은 다른 용도다. 그런데 셰이더가 알파를 불투명도로 가정하고 컷아웃한다.

```hlsl
// Shader_VtxAnimMeshBinary.hlsl:75-77
float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
if (diffuse.a < 0.3f)
    discard;
```

알파 채널 실측값이다.

```
pc_ft_face_00_d.tga    32bpp  alpha 0..153   a=0: 99.4%   discard: 99.8%   ← 얼굴
pc_ft_08_hair_d.tga    32bpp  alpha 0..255   a=0: 41.0%   discard: 45.2%
pc_ft_eyelashes_d.tga  32bpp  alpha 0..255   a=0: 54.1%   discard: 63.4%
pc_flm_00_helmet_d.tga 32bpp  alpha 0..255   a=0: 40.7%   discard: 43.5%
pc_ft_01_upper_d.tga   24bpp  alpha 255..255              discard:  0.0%   ← 기본옷
pc_ft_01_arm_d.tga     24bpp  alpha 255..255              discard:  0.0%
```

얼굴의 99.8%가 discard되어 통째로 사라졌다. 24bpp 텍스처는 로더가 알파를
255로 채우므로(Material.cpp:75) 멀쩡했다. **24bpp는 되고 32bpp는 깨진다**는
패턴이 단서였다.

### 대처

불투명 재질과 컷아웃 재질을 **텍스처 레벨에서** 가른다.

| 재질 | 처리 |
|---|---|
| 얼굴·피부 등 불투명 | 24bpp 무압축으로 다시 써서 알파 제거 |
| 머리카락·속눈썹 | 그대로 둔다 (알파가 진짜 불투명도다) |

`Strip-TgaAlpha.ps1`로 `pc_ft_face_00_d.tga`를 24bpp로 변환해 해결했다.
**코드 변경도 재쿠킹도 필요 없다** — wmodel은 텍스처를 상대 경로로 참조하므로
파일만 교체하면 된다. 원본은 `.orig`로 백업했다.

방어구(30~43% discard)는 스트랩·장식 구멍 같은 정상적인 컷아웃일 수 있어
건드리지 않았다.

### 이전에는 왜 얼굴이 보였나

구버전의 `pc_ft_00_face_sk`가 쓰던 재질 `pc_ft_face_mi_high`는 이름에 `_high`가
붙어 컨버터의 자동 매칭에 실패해 **`defaultdiffuse.tga`(회색, 알파 255)** 로
폴백되어 있었다. 그 회색 얼굴이 불투명하게 그려지며 discard된 진짜 얼굴을
가리고 있었다. 중복을 제거하자 가림막이 사라져 문제가 드러난 것이다.

## 6. 눈 — 재질 오매칭과 다층 재질

얼굴을 살리고 나니 눈동자에 백버퍼 색(파랑)이 비쳤다. 원인이 둘이었다.

**(1) 컨버터가 눈에 속눈썹 텍스처를 붙였다.**
`pc_ft_eye_mi` → `pc_ft_eyelashes_d.tga`. 컨버터의 재질명↔텍스처명 자동 매칭이
구분자를 제거하고 비교하므로 `pcfteyemi`가 `pcfteyelashesd`에 걸린 것이다.
그 텍스처는 63.4%가 discard되므로 눈에 구멍이 뚫려 백버퍼가 비쳤다.

**(2) 눈은 원래 다층 재질이다.**
`PC_FT_00_FACE/MaterialInstanceConstant/pc_ft_eye_mi.props.txt`가 정본이다.

```
texture_diffuse_base    = pc_ft_00_eyebase_d      흰자
var_eye_iristexture_ui  = pc_ft_00_eyeiris_d      홍채 (알파로 합성)
var_eye_iriscolor_ui    = (0.64, 0.42, 0.28)      홍채 틴트
texture_tdspecular      = lightbox_cube2_1        하이라이트
iris_size_center        = 0.35
```

디퓨즈 한 장만 쓰는 우리 렌더러로는 이 합성을 재현할 수 없다. `eyebase`만 쓰면
동공 없는 밋밋한 눈이 되고, `eyeiris`만 쓰면 90.8%가 discard된다.

**대처: 합성을 미리 구워 한 장으로 만든다.** `Bake-EyeTexture.ps1`이
`eyebase`에 `eyeiris`를 알파 합성하고 틴트를 곱해 24bpp 불투명 TGA로 낸다.
홍채 원판의 반지름이 텍스처의 약 35%라 `iris_size_center = 0.35`와 맞아떨어져
UV 스케일 보정 없이 그대로 겹쳐진다.

> 눈 색을 바꾸려면 `-TintR/-TintG/-TintB`를 조정하거나 `pc_ft_NN_eyeiris_d.tga`
> (26종)를 골라 다시 구우면 된다. 원본의 눈 색상 커스터마이징이 이 구조다.

### 텍스처 수정본은 별도 폴더에 둔다

`_fixed_tex/`에 수정본을 모으고 컨버터에 명시적으로 넘긴다. 출력 폴더의
텍스처를 직접 고치면 **재쿠킹할 때 스테이징 원본으로 덮여 되돌아간다.**

```powershell
ModelAssetConverter.exe <body.fbx> -o <LanceMaster.wmodel> `
  --texture-root "<staging>" `
  --material-remap "pc_ft_face_mi=<_fixed_tex>\pc_ft_face_00_d.tga" `
  --normal-remap   "pc_ft_face_mi=<staging>\PC_FT_00_FACE\Texture2D\pc_ft_face_00_n.tga" `
  --material-remap "pc_ft_eye_mi=<_fixed_tex>\pc_ft_00_eye_baked_d.tga"
```

`--material-remap`은 여러 번 줄 수 있고, 명시하지 않은 재질의 자동 매칭은
그대로 동작한다(`--no-auto-textures` 없이 병용 가능).

## 7. 장비와 맨몸의 z-fighting — 슬롯 마스킹

장비를 켜면 방어구 밑의 맨몸이 그대로 그려져 두 면이 같은 깊이를 다퉜다.
관절처럼 살이 부푸는 곳에서는 방어구를 뚫고 나왔다.

**방어구 메시가 자체적으로 노출 피부를 포함하고 있다**(장비만 렌더해도 등·다리 살이
나온다). 따라서 몸에서는 장비가 덮는 부위를 아예 그리지 않는 것이 맞다.
뎁스 바이어스 같은 방법은 관통을 못 막는다.

`CPart_Body`가 서브메시 마스크를 갖고, 컨테이너가 장비 구성에 따라 설정한다.

```cpp
// Part_Body.h
void Set_HiddenMeshes(uint32_t iHiddenMeshMask);

// Part_Body.cpp - Render()
if (0 != (m_iHiddenMeshMask & (1u << i)))
    continue;
```

창술사는 방어구 5종이 팔·상의·하의를 덮으므로 `(1<<0)|(1<<1)|(1<<2)`를 숨기고
얼굴·속눈썹·눈·머리카락만 항시 렌더한다.

> **주의: 인덱스 하드코딩이다.** `0 arm / 1 upper / 2 lower / 3 face / 4 eyelashes /
> 5 eye / 6 hair` 순서에 의존한다. 몸을 다른 구성으로 재쿠킹하면 조용히 엉뚱한
> 부위가 사라진다. 근본적으로 없애려면 맨몸도 재질별로 분리해 파츠화해야 한다
> (그러면 "장비 착용 = 해당 파츠를 만들지 않음"이 되어 인덱스가 개입하지 않는다).

## 8. 스트레이 웨이트 — 애니메이션 중 머리카락이 늘어남

애니메이션을 하나씩 재생하다 보면 특정 동작에서 머리카락 몇 가닥이 길게 늘어났다.

**머리카락 정점 2개가 `b_root`(캐릭터 루트 본)에 물려 있었다.** 나머지 1237개는
`bip001-head`를 따라가는데 그 둘만 루트에 고정되니, 머리가 루트에서 멀어지는
동작에서 사이의 삼각형이 늘어난다. 대기 모션처럼 머리가 루트 축 근처에 있을 때는
드러나지 않는다.

**재쿠킹이 만든 문제가 아니다.** 구버전 백업에도 동일하게 2개가 물려 있었다.
원본 에셋의 웨이팅 결함이고 애니메이션을 순회하면서 처음 드러났다.

`build_flm_part.py`에 이관 단계를 넣어 해결했다.

```
[repair] pc_ft_00_sk.001: moved 2 b_root weight(s) to bip001-head
```

범위를 먼저 확인하고 넣었다 — 몸의 7개 서브메시 중 `b_root`를 쓰는 것은
머리카락뿐이므로 이관이 다른 부위에 영향을 주지 않는다.

> 비슷한 증상이 다른 부위에서 보이면 `Find-UnanimatedBones.ps1`로 해당 서브메시의
> 본 사용 현황을 뽑는다. 엉뚱한 본에 물린 정점이 있으면 바로 드러난다.

## 9. 캐릭터 구조 — `CCharacter`

서버 연동 시 4명이 각자 캐릭터를 하나씩 들 예정이라, 직업별 GameObject 클래스를
만드는 대신 **공용 캐릭터 + 직업별 데이터·로직** 구조로 정리했다.

핵심 판단: 네트워크가 들어오면 실제로 갈리는 축은 **직업이 아니라 "누가 조종하느냐"**
다. 여기에 직업별 클래스까지 곱하면 조합이 늘어난다. 그래서 "원격 플레이어용
클래스"는 만들지 않는다.

```
CCharacter (CContainerObject)     조립·애니 재생. 직업 무관, 팀 공용
  ├─ const CHARACTER_SPEC*        직업별 데이터 (모델 태그, 소켓 본, 클립 이름표, 장비 목록, 마스크)
  └─ unique_ptr<ICharacterLogic>  직업별 스킬 로직 — 1인 1파일

CPart_Body        스켈레톤·애니 시계 소유, 서브메시 마스크
CPart_Equipment   스킨드(팔레트 차용) / 소켓(본 부착) 겸용
```

직업을 추가하는 사람은 파일 한 쌍만 만든다. `Logic_LanceMaster.*`를 그대로 베끼면 된다.

```cpp
// Logic_<직업>.h  -- 계약만
class CLogic_<직업> final : public ICharacterLogic
{
public:
	virtual void Update(CCharacter& Character, f32_t fTimeDelta) override;
};

extern const CHARACTER_SPEC Spec_<직업>;
```

```cpp
// Logic_<직업>.cpp
namespace
{
	constexpr EQUIPMENT_PART_SPEC Equipment[] = { ... };
	constexpr uint32_t COVERED_BY_ARMOUR = ...;
	unique_ptr<ICharacterLogic> Create_Logic() { ... }
}

NS_BEGIN(Client)
void CLogic_<직업>::Update(...) { ... }
const CHARACTER_SPEC Spec_<직업> = { ... };   // 태그·클립 이름은 여기
NS_END
```

쓰는 쪽은 `CharacterDesc.pSpec = &Spec_<직업>;` 한 줄이다.

**데이터는 .cpp에 둔다.** 외부 링키지를 갖는 것은 `Spec_<직업>` 하나뿐이고
`Equipment[]`·`COVERED_BY_ARMOUR`·`Create_Logic`은 익명 namespace에 있다. 그래서
네 사람이 각자 파일에서 **같은 짧은 이름을 그대로 쓸 수 있고**, 접두사를 신경 써야
하는 이름은 `Spec_<직업>`뿐이다. 정의를 헤더로 올리면 이 이름들이 전부 한 namespace에
노출되어 4개 파일 모두 접두사가 필요해진다.

공유 파일은 건드리지 않으므로 **merge 충돌이 구조적으로 발생하지 않는다.** 이게
직업별 로직 클래스를 남긴 이유다(조립 코드 중복은 막고, 파일 소유권은 유지).

애니메이션은 공용 상태(`IDLE/RUN/HIT/DEAD`)만 SPEC의 이름표로 두고, 스킬 클립은
직업마다 개수와 흐름이 달라 로직이 이름으로 직접 다룬다.
**인덱스로 지목하면 안 된다** — 직업마다 다르고 재쿠킹하면 바뀌므로 네트워크로
"3번 클립"을 보내면 클라마다 다른 것이 나온다.

### 네트워크 대비

`ICharacterController`(로컬 입력 / 네트워크)는 **아직 만들지 않았다.** 서버가 없고
구현체가 하나뿐이라 이르다. 대신 비용 0으로 대비만 해뒀다 — `CCharacter`가
`Set_Position` / `Set_Animation`으로 **상태를 밖에서 밀어넣을 수 있다.** 네트워크가
붙을 때 컨트롤러 계층만 얹으면 되고 캐릭터는 뒤집히지 않는다.

서버 방식(단순 릴레이 vs 서버 권위)은 아직 미확정이지만, 양쪽 모두 캐릭터에
요구하는 것이 같아 이 구조로 진행해도 된다. 팀 규모를 감안하면 릴레이가 현실적이고,
판정 불일치는 "때린 쪽이 판정한다"로 싸게 우회할 수 있다.

### 레거시 정리

`CLanceMaster` / `CBody_LanceMaster` / `CWeapon_LanceMaster`를 삭제했다.
`CCharacter`가 동일한 결과를 내는 것을 확인한 뒤 정리했다.
무기는 당장 `CPart_Equipment`의 소켓 모드를 쓴다 — 히트 윈도우와 트레일이 붙을 때
전용 클래스로 분리한다.

`CPlayer`(네비게이션·콜라이더 보유)도 레거시로 사라질 예정이다. 그 두 컴포넌트는
`CCharacter`로 옮겨야 하지만, 둘 다 레거시라 이번에는 붙이지 않았다.

## 10. 검증

| 항목 | 방법 | 결과 |
|---|---|---|
| 본 인덱스 정렬 | `Compare-Skeletons.ps1` | 6개 전부 224본, `[0..222]` 이름 일치. 223번(메시 노드)만 상이 |
| `inverseBind` 일치 | `Compare-InverseBind.ps1` | `maxDelta = 0.00e+000` (비트 단위 동일) |
| 중복 제거 | `ModelAssetConverter info` | 몸 `meshes=17 → 7`, 재질 `15 → 7` |
| 텍스처 매칭 | 동상 | `defaultdiffuse.tga` 폴백 0건 |
| 지오메트리·스키닝 | `Inspect-Submeshes.ps1` | 얼굴 서브메시가 머리 위치(z −126.5~−112.1), 가중치 합 1.000 |
| 빌드 | MSBuild Debug x64 | 통과 |
| 재질 오매칭 | `Dump-WModel.ps1` | 눈 재질이 `pc_ft_00_eye_baked_d.tga`로 교정됨 |
| 스트레이 웨이트 | `Find-UnanimatedBones.ps1` | 머리카락이 `b_root` 없이 본 4개만 사용 |
| 실행 | `LEVEL::TEST_LEVEL2` (LOGO에서 F3) | 방어구가 애니메이션 따라 변형, 창은 오른손 소켓 유지, 얼굴·눈동자 정상 |
| z-fighting | 동상 | 마스킹 후 관통·깜빡임 없음 (정지 프레임 기준) |
| `CCharacter` | 동상 | 창술사를 레거시 클래스와 동일하게 조립 |

머리카락의 형광 초록은 원본 색이 맞다(umodel 뷰어 대조 확인). 입·턱의 검은
부분은 복면으로 의도된 디자인이다. 둘 다 버그가 아니다.

## 11. 남은 것 / 주의

- **스크립트는 `C:\Users\95jus\Desktop\buildscript`로 옮겼다.** 저장소 밖이므로
  Git으로 공유되지 않는다. 팀원에게 넘길 때는 폴더를 따로 전달해야 한다.
  사용법은 그 폴더의 `README.md`에 정리했다.
- `LanceMaster_Face.wmodel`(`pc_ft_00_face_sk`, 3메시)은 만들어 두었으나
  **현재 사용하지 않는다.** 몸이 이미 얼굴을 포함하므로 붙이면 중복이 된다.
  나중에 얼굴 커스터마이징을 붙일 때 쓸 수 있어 파일은 남겼다.
- `LanceMaster.wmodel.bak`(구버전 109.6 MB)과 `pc_ft_face_00_d.tga.orig`가
  남아 있다. 충분히 검증되면 정리할 것.
- **텍스처 수정본 `_fixed_tex/`는 저장소 밖 스테이징에 있다.** 몸을 재쿠킹할 때
  5-2절의 remap 인자를 빠뜨리면 얼굴과 눈이 원래 상태로 되돌아간다.
  다른 팀원이 재현하려면 이 폴더도 함께 받아야 한다.
- 스켈레톤에 socket이 0개다. 무기는 소켓이 아니라 본 이름(`b_weapon_rhand`)으로
  붙고 있다. 부착점을 늘릴 계획이면 소켓을 제대로 export하는 편이 낫다.
- 리그 불일치를 런타임에 검증할 수단이 없다(`CModel`에 본 개수 접근자가 없다).
  다른 리그의 방어구를 물리면 예외 없이 조용히 뒤틀린다. **쿠킹 시점에
  `Compare-Skeletons.ps1`로 확인하는 것이 현재 유일한 안전장치다.**
- 산출물은 전부 `.gitignore` 대상이다. 팀 Drive 팩 경로 규칙을 따를 것.

## 12. 도구

| 스크립트 | 용도 |
|---|---|
| `build_flm_part.py` | 파츠별 FBX 빌드 (Blender 5.0 headless) |
| `Compare-Skeletons.ps1` | 두 wmodel의 본 이름 배열 대조 |
| `Compare-InverseBind.ps1` | `inverseBind` 행렬 값 대조 |
| `Inspect-Submeshes.ps1` | 서브메시별 위치 bbox·블렌드 가중치·본 인덱스 |
| `Inspect-TgaAlpha.ps1` | TGA 알파 분포와 discard 비율 |
| `Strip-TgaAlpha.ps1` | 32bpp TGA를 24bpp 무압축으로 변환(알파 제거) |
| `Bake-EyeTexture.ps1` | eyebase + eyeiris를 틴트와 함께 합성해 한 장으로 굽기 |
| `Convert-TgaToPng.ps1` | TGA를 PNG로 디코드(육안 확인용) |
| `Dump-WModel.ps1` / `Dump-Bones.ps1` | wmodel 섹션·서브메시·재질·본 덤프 |
| `Run-AndCapture.ps1` | 클라이언트 실행 → F3 → 스크린샷 |

`.wmodel` 바이너리를 직접 읽을 때 걸린 것들을 적어 둔다. 셋 다 내가 한 번씩 틀렸다.

- `SUBMESH_DESC::vertexOffset`은 정점 인덱스가 아니라 **바이트 오프셋**이다
  (WMeshReader.cpp:256). stride를 곱하면 좌표가 1e36 수준으로 튄다.
- WMSH 섹션의 배치 순서는 `헤더 → 서브메시 desc → 정점 블롭 → 인덱스 블롭 →
  본 테이블`이다. `inverseBind`는 맨 뒤 본 테이블에 있다.
- **파일의 스킨드 정점 레이아웃(stride 76)은 런타임 `VTXANIMMESH`와 다르다.**
  `MakeSkinnedVertex`(WMeshReader.cpp:79)가 정본이다.

  ```text
   0 position  float3      12 normal    float3
  24 texcoord  float2      32 tangent   float3
  44 blendIndices uint32×4  60 blendWeights float×4
  ```

  binormal은 파일에 없고 런타임에 계산된다. texcoord가 tangent보다 앞이고,
  블렌드 인덱스는 uint8이 아니라 **uint32 4개**다. 이걸 56번지의 바이트 4개로
  읽으면 본 인덱스가 전부 0으로 나와 엉뚱한 결론에 이른다.
