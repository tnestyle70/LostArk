# Lost Ark HeartRB 완전 복원 최종 마스터 계획

> 대상: `LV_LUT_HEARTRB_ED_PS`, `LV_LUT_HEARTRB_ED_SL00~SL05`  
> 현재 실측: 정확한 StaticMesh 260종, 실제 배치 13,091개, 음수 축 5,042개, 실제 reflection 4,521개  
> 이번 완료선: 원본 StaticMesh 지오메트리의 Catalog 연결, 좌표·회전·signed scale·피벗 복원, MapTool 원자적 로드와 저장  
> 별도 후속 게이트: collision/Volume, DeployData, navigation, Particle/Light, SkeletalMesh, 파괴 전환

### 완전 복원 Gate

| Gate | 범위 | PASS 조건 |
|---|---|---|
| G0 입력 동결 | 260 WModel, 13,091 UE3 placement | placement audit와 runtime manifest hash 동결, join 누락 0 |
| G1 기준계 | 중앙 Floor01/Floor02/FloorMetal01 | `(X,Z,-Y)` 위치, basis-conjugated quaternion, signed scale, Origin 일치 |
| G2 정적 코어 | SL00 682개 | 음수 축 223개/반사 203개, 중앙·외곽 구조 상대 배치 일치 |
| G3 정적 전체 | PS+SL00~SL05 13,091개 | 레벨별 수량 일치, 음수 축 5,042개/반사 4,521개, rollback/reload PASS |
| G4 충돌·보행 | collision/Volume + navigation | 보행 바닥, 벽, 낙사 경계와 테스트 캐릭터 이동 PASS |
| G5 상태 전환 | DeployData, InterpActor, destructible | 파괴 전/후 mesh·collider·nav link가 같은 state ID로 전환 |
| G6 동적 표현 | Particle, fog, decal, light, SkeletalMesh | source receipt 기반 별도 layer와 phase 표시, 누락 0 |
| G7 게임 통합 | Valtan actor, 패턴, 저장 | 맵 scale 불변, 보스 크기·충돌·root motion·낙사 판정 일치 |

G0~G3가 이 문서의 직접 구현 범위다. G4는 기존 Navigation 계획을 G3 PASS 뒤
적용하고, G5~G7은 해당 extractor receipt가 생긴 뒤 별도 dated PLAN으로 승격한다.

## 1. C1~C8 관점

### C1. 목표와 기준

다음 다섯 항목을 동시에 만족할 때만 HeartRB의 **정적 맵 복원 완료**로 판정한다.

1. `map_asset_manifest.json`의 exact ImportTable 자산 260개가 모두 기존 `CModel -> CMaterial` 경로의 Prototype으로 등록된다.
2. PS 142개와 SL00~SL05 12,949개, 합계 13,091개 placement가 누락·중복 없이 Clone된다.
3. UE3 원본 Transform을 Client 기저로 한 번만 변환하고, 위치·회전·signed scale 및 `Origin` 피벗을 보존한다.
4. 음수 축이 하나 이상인 5,042개를 보존하고, 그중 determinant가 음수인 4,521개만 mirrored pass로 렌더링한다.
5. 파싱·검증·Clone 중 하나라도 실패하면 기존 Scene을 보존하고 새로 만든 객체만 전부 rollback한다.

이번 계획의 “완전”은 **StaticMesh 정적 배치 범위**를 뜻한다. `placement_audit.json`의 open gate인 collision-only/Volume, DeployData, navigation, dynamic actor는 각각 후속 단계의 입력과 완료 조건까지 공통 파이프라인 문서에 기록하지만, 이번 결과 수치에 섞지 않는다.

### C2. 현재 실측과 잘못 적용하면 안 되는 값

| 항목 | 실측 |
|---|---:|
| exact asset | 260 |
| runtime `.wmodel` | 260/260 |
| PS placement | 142 |
| SL00 | 682 |
| SL01 | 3,603 |
| SL02 | 2,094 |
| SL03 | 2,543 |
| SL04 | 2,324 |
| SL05 | 1,703 |
| 전체 placement | 13,091 |
| actor transform 정본 | 2,282 |
| component transform 정본 | 10,809 |
| 음수 축이 하나 이상인 placement | 5,042 |
| determinant 음수 reflection | 4,521 |
| property error | 0 |

현재 `BG_RAD_VALTAN_A.mapassets`는 수동 후보 17개 Catalog이고, `MapTool.cpp`의 placement 상한은 10,000개이며, scale validator는 양수만 허용한다. 이 세 상태는 13,091개 exact scene의 정본이 아니다.

또한 다음 문구는 불완전하므로 그대로 구현하지 않는다.

```text
잘못된 축약: UE3 position × 0.01, scale3D 그대로
정확한 계약: 위치·회전·scale을 모두 같은 UE3 -> Client 기저로 변환
```

### C3. 데이터 소유권과 공통 비용

```text
map_asset_manifest.json
  exact objectPath -> stable assetId -> source glTF

map_asset_runtime_manifest.json
  stable assetId -> installed WModel/texture

*.placements.json
  sourcePlacementId -> source level -> exact objectPath -> UE3-native Transform

build_maptool_scene.py
  두 manifest와 placement를 join하고 Client 기저로 변환

<AREA>.mapassets
  생성 가능한 260개 정의와 Prototype tag

<AREA>.mapplacements v2
  13,091개 인스턴스 ID, 출처, layer, quaternion, signed scale

CMapAssetCatalog -> Loader
  260개 CModel Prototype 등록

CMapTool -> CMapAssetObject -> Engine Layer
  parse -> validate -> stage Clone -> commit / rollback
```

Catalog가 자산 정의를, placement 문서가 인스턴스를 소유한다. Runtime은 object path를 다시 추측하지 않으며 Python scene compiler가 한 번 수행한 join 결과만 읽는다. 260개 Prototype은 placement마다 다시 decode하지 않고 Clone이 공유한다.

### C4. 수명과 선언

- `ACTIVE.maparea`는 프로세스 시작 시 선택할 area를 소유한다. 변경 후에는 Client를 재시작한다.
- `CMapAssetCatalog`는 선택된 Catalog 경로와 그에 대응하는 placement 경로를 한 번에 확정한다.
- `CMapPlacementDocument`는 v1 legacy 읽기, v2 exact 읽기·쓰기, 검증, 원자적 저장만 담당한다.
- `CMapTool`은 placement record와 Clone의 수명, source level별 Engine Layer, rollback을 담당한다.
- `CMapAssetObject`는 quaternion과 signed scale을 보존한 최종 world matrix를 소유한다.
- imported runtime ID는 최상위 비트가 1, editor 신규 ID는 최상위 비트가 0인 별도 영역을 사용한다.

### C5. 좌표·단위·회전의 정본

UE Viewer glTF export는 UE 좌표 `(X,Y,Z)`를 glTF `(X,Z,Y)`로 바꾸고 centimeter를 meter로 `0.01`배 한다. 현재 260개 cook 결과는 glTF `(x,y,z)`를 WModel `(100x,100y,-100z)`로 저장하며 Loader는 map model에 `0.01`을 적용한다. 따라서 최종 Client 기저는 다음 하나로 확정한다.

이 결론은 UE Viewer `ExportGLTF.cpp`의 축/단위 변환, 현재 converter의
left-handed 결과, `Loader.cpp`의 map `0.01f`, 중앙 Floor01 골든 값을 함께 합성한
것이다. 이전 초안의 `(UE.Y,UE.Z,UE.X)` 위치와 `(Sy,Sz,Sx)` scale, raw Euler 저장은
폐기하며 구현 코드나 파생 데이터에 남기지 않는다.

기준 소스는 UE Viewer 공식 저장소의
[`ExportGLTF.cpp`](https://github.com/gildor2/UEViewer/blob/master/Exporters/ExportGLTF.cpp),
[`UnMathTools.h`](https://github.com/gildor2/UEViewer/blob/master/Unreal/UnrealMesh/UnMathTools.h),
[`Math3D.cpp`](https://github.com/gildor2/UEViewer/blob/master/Core/Math3D.cpp)와 현재
LostArk `Model.cpp`, `Loader.cpp`, 실제 WModel/placement receipt다.

```text
ClientPosition = (UE3.X, UE3.Z, -UE3.Y) * 0.01
ClientScale    = (UE3.ScaleX, UE3.ScaleZ, UE3.ScaleY)  // 부호 보존

glTF mesh(m) * cook 100 * Loader 0.01 = Client mesh(m)
UE3 position(cm) * basis * 0.01       = Client position(m)
```

row-vector 기준 basis 행렬 `B`는 다음과 같다.

```text
B = | 1  0  0 |
    | 0  0 -1 |
    | 0  1  0 |

pClient = pUE3 * B * 0.01
RClient = transpose(B) * RUE3 * B
```

UE3 Rotator는 Euler 축을 임의로 swap하지 않는다. UModel의 `RotatorToAxis`와 `Euler2Vecs`에 해당하는 회전 행렬을 만든 후 basis conjugation하고, DirectX row-vector quaternion으로 변환한다. Runtime 문서에는 Euler가 아니라 정규화 quaternion을 저장한다.

중앙 링 골든 케이스는 다음 값이다.

```text
sourcePlacementId : LV_LUT_HEARTRB_ED_SL00:export:1234
asset objectPath  : pvp_retown_a.mesh.bg_pvp_retown_floor01_sm
assetId           : MAP_7D6EFEB99D88_BG_PVP_RETOWN_FLOOR01_SM

UE3 position      : (849.491821, 2140.945313, 975.819458) cm
Client position   : (8.494918, 9.758195, -21.409453) m

UE3 rotator       : (Pitch=0, Yaw=-16384, Roll=0)
Client quaternion : (0, -0.707106781, 0, 0.707106781)

UE3 signed scale  : (0.490689129, 0.513935432, 0.225717008)
Client scale      : (0.490689129, 0.225717008, 0.513935432)
```

모든 exact asset는 `Origin` anchor를 쓴다. `BottomCenter`는 마우스 수동 배치용 legacy 편의 기능으로만 남기고 원본 placement에는 적용하지 않는다.

### C6. 예외와 가지치기

- asset/objectPath join 누락, duplicate asset ID, duplicate runtime ID, 없는 WModel은 compile을 즉시 실패시킨다.
- NaN/Inf, 길이 0 quaternion, 절댓값이 epsilon 미만인 scale 축은 runtime parse 단계에서 거부한다.
- quaternion은 normalize하되 행렬 round-trip 오차가 `1e-5`를 넘으면 생성 단계에서 실패한다.
- imported ID는 `SHA-256(sourcePlacementId)` 상위 64비트에 high bit를 세우고 collision 시 자동 변경하지 않고 실패한다.
- signed scale의 곱이 음수이면 mirrored shader pass의 front culling을 사용한다.
- 모든 Clone 성공 전 기존 placement를 제거하지 않는다.
- `BG_RAD_VALTAN_A` 수동 scene과 HeartRB exact scene을 같은 파일로 합치지 않는다.
- `Valtan Core` 하드코딩 버튼은 area 범용 MapTool에서 제거한다.

### C7. 권위와 정합성

| 데이터 | 정본 |
|---|---|
| 어떤 mesh인가 | UE3 Actor/Component -> ImportTable exact objectPath |
| 어떤 파일인가 | `map_asset_manifest.json`의 resolved `assetId/sourceGltf` |
| runtime model | `map_asset_runtime_manifest.json` |
| 원본 배치 | 7개 `*.placements.json` |
| Client 좌표 변환 | `build_maptool_scene.py`의 basis 계약 |
| 생성 정의 | `<AREA>.mapassets` |
| runtime 인스턴스 | `<AREA>.mapplacements` v2 |
| 실행 area | `ACTIVE.maparea` |

원본 JSON은 `UE3-native` 정본으로 보존하고 직접 수정하지 않는다. Client용 파생 파일은 언제든 원본 manifest와 placement에서 재생성한다.

### C8. 검증이 병목인 지점

1. 중앙 Floor01 단독 layer를 먼저 열어 position/quaternion/scale/Origin을 스크린샷과 검증한다.
2. Floor02, FloorMetal01, Statue01 인접 관계로 handedness와 회전 방향을 재검증한다.
3. 음수 scale의 홀수 반사 샘플을 mirrored pass on/off로 비교한다.
4. PS와 SL00~SL05 각 layer count가 `142/682/3603/2094/2543/2324/1703`인지 runtime에서 출력한다.
5. 260 Prototype, 13,091 Clone, 음수 축 5,042, mirrored 4,521, missing 0, rollback 0을 receipt와 runtime log 양쪽에서 맞춘다.
6. Save -> Reload 후 sourcePlacementId, quaternion, signed scale이 byte-level 문서 값과 동일한지 확인한다.

## 2. 문제 해결 ①~⑤

① 문제·제약: 260개 WModel과 13,091개 UE3 placement는 존재하지만 MapTool은 17개/10,000개/양수 scale/v1 Euler 계약이다.  
② 단순 해법의 문제: 위치만 `0.01`배하거나 scale을 순서 그대로 쓰면 Y/Z 및 handedness가 갈라져 원형 링이 찌그러지고 조각이 다른 축으로 퍼진다.  
③ 해결 방식: offline scene compiler가 exact objectPath join과 basis 변환을 끝내고 v2 quaternion/signed-scale 문서를 생성하며 runtime은 검증·Clone·rollback만 한다.  
④ 비교: 기존 `CModel -> CMaterial -> CMapAssetObject`를 확장하고 `CCookedModel/CBinaryAssetObject` 또는 별도 scene runtime은 만들지 않는다.  
⑤ 대가: 13,091개 GameObject와 draw call은 우선 정확성 검증 비용으로 수용하고, 정적 복원 PASS 뒤 동일 record를 보존하는 instancing 최적화를 별도 측정 기반 작업으로 진행한다.

## 3. 자료구조·알고리즘 핵심

### 3.1 Placement v2 저장 계약

```text
LOSTARK_MAP_PLACEMENTS 2 "<areaId>" <count>
<runtimeId>
"<sourcePlacementId>"
"<sourceLevel>"
"<transformSource>"
"<assetId>"
px py pz
qx qy qz qw
sx sy sz
visible
```

실제 한 행에는 위 필드를 공백으로 이어 쓴다. `transformSource` 허용값은 `actor`, `component`, `editor`, `legacy`다. `sourceLevel`은 Engine Layer tag `Layer_MapAsset_<sourceLevel>`에 사용하므로 영문·숫자·점·밑줄·하이픈만 허용한다.

### 3.2 ID 알고리즘

```text
importedId = bigEndianUInt64(SHA256(UTF8(sourcePlacementId))[0:8]) | 0x8000000000000000
editorId   = 1..0x7fffffffffffffff 중 사용하지 않은 다음 값
```

ID hash collision은 새 salt로 숨기지 않고 sourcePlacementId 두 개를 receipt에 기록한 뒤 생성 실패한다. 원본 string ID도 v2에 함께 저장하므로 hash는 추적 정보를 대체하지 않는다.

### 3.3 로드 알고리즘

```text
ACTIVE.maparea parse
  -> areaId token 검증
  -> <areaId>.mapassets parse/validate/stage/commit
  -> Loader가 260 CModel Prototype 등록

<areaId>.mapplacements parse
  -> header/area/count 검증
  -> 모든 record 및 catalog FK 검증
  -> source level별 임시 Layer에 13,091 Clone
  -> 하나라도 실패하면 임시 Clone 전부 제거, 기존 Scene 유지
  -> 모두 성공하면 기존 Scene 제거, 임시 vector를 정본으로 교체
```

### 3.4 signed transform 알고리즘

`CTransform::Rotation()`은 기존 축 길이를 `Get_Scaled()`로 다시 읽으므로 음수 부호를 잃는다. 따라서 exact placement는 `Scale()`과 `Rotation()`을 순서대로 호출하지 않는다.

```text
world = Scaling(signedScale) * RotationQuaternion(q) * Translation(position)

Transform RIGHT    = world row 0
Transform UP       = world row 1
Transform LOOK     = world row 2
Transform POSITION = world row 3
```

`scale.x * scale.y * scale.z < 0`일 때만 mirrored다. shader pass 0은 `RS_Default`,
pass 1은 `RS_Cull_CW`를 사용한다. non-uniform signed scale에서 normal은 world
inverse-transpose, tangent와 원본 binormal은 world linear matrix로 변환한다. tangent는
normal에 대해 Gram–Schmidt 직교화하고 binormal은 `cross(normal,tangent)`에 변환된
원본 binormal의 handedness를 적용해 재구성한다.

### 3.5 복잡도

- manifest join: `O(A + P)`, `A=260`, `P=13,091`
- parse validation: `O(P)`
- duplicate ID 및 foreign key: 평균 `O(P)` hash lookup
- stage/rollback: `O(P)`
- 저장: `O(P)` 임시 파일 작성 후 atomic replace

## 4. 추가·수정·삭제 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_maptool_scene.py` | exact asset/placement join, 좌표 변환, Catalog/placement/receipt 생성 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/test_build_maptool_scene.py` | basis, quaternion, ID, 골든 케이스 회귀검사 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/gotchas.md` | 재사용 가능한 추출·cook·placement 함정 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/MapPlacementDocument.h` | v2 record와 문서 API |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/MapPlacementDocument.cpp` | v1/v2 parse, validate, atomic write |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/ACTIVE.maparea` | 시작 area 선택 |
| 생성 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapassets` | 260개 exact Catalog |
| 생성 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements` | 13,091개 placement v2 |
| 생성 | `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/placements/maptool_scene_receipt_{G1,G2,G3}.json` | Gate별 입력·출력 hash와 음수 축/reflection 수량·골든 결과 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetCatalog.h` | 선택된 area 경로 소유 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp` | ACTIVE parse 및 area별 Catalog 로드 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetObject.h` | quaternion/signed scale 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp` | signed world matrix, determinant 기반 mirror pass, inverse-transpose bind |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | v2 record/layer metadata와 함수 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | v2 load/save, 65,536 상한, layer rollback, Valtan 하드코딩 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl` | inverse-transpose normal, Gram–Schmidt tangent, handed binormal, mirrored pass |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | MapPlacementDocument 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | Map tool filter 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/GB/07-30/맵추출파이프라인.md` | 전체 공통 파이프라인과 후속 gate 정본화 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/GB/07-29/gotchas.md` | 축, cook scale, signed scale, Origin, v1/v2 함정 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/README.md` | scene compile 명령과 Transform 계약 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/GB/07-30/2026-07-30_LOSTARK_HEARTRB_EXACT_STATIC_MAP_RECONSTRUCTION_PLAN.md` | 비평을 통합한 최종 마스터 구현 계획 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/GB/07-30/2026-07-30_LOSTARK_VALTAN_PERFECT_RECONSTRUCTION_PLAN.md` | 잘못된 좌표계 초안을 폐기하고 이 마스터 문서로 연결 |
| 삭제 | 없음 | 기존 17개 수동 파일은 보존하고 ACTIVE 선택에서만 분리 |

`Loader.cpp`의 map `PreTransform = 0.01`, 발탄 캐릭터 `PreTransform = 0.0001`은 수정하지 않는다.

## 5. 파일별 전체 구현 코드

### 5-1. `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_maptool_scene.py`

변경 종류: 추가  
적용 위치: 새 파일 전체

구현은 다음 함수와 계약을 파일 전체에 둔다. 구현 시 표준 라이브러리만 사용하고 모든 write는 같은 디렉터리의 임시 파일을 `os.replace`한다.

```python
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import tempfile
from pathlib import Path
from typing import Any, Iterable, Sequence

BASIS = (
    (1.0, 0.0, 0.0),
    (0.0, 0.0, -1.0),
    (0.0, 1.0, 0.0),
)
IMPORTED_ID_BIT = 1 << 63
QUATERNION_EPSILON = 1.0e-6


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"JSON root is not an object: {path}")
    return value


def atomic_write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            output.write(text)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def token(value: str, field: str) -> str:
    if not value or len(value) > 128:
        raise ValueError(f"invalid {field}: {value!r}")
    if any(not (character.isalnum() or character in "_.-") for character in value):
        raise ValueError(f"invalid {field}: {value!r}")
    return value


def quoted(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def mat_transpose(matrix: Sequence[Sequence[float]]) -> tuple[tuple[float, ...], ...]:
    return tuple(tuple(matrix[row][column] for row in range(3)) for column in range(3))


def mat_mul(
    left: Sequence[Sequence[float]], right: Sequence[Sequence[float]]
) -> tuple[tuple[float, ...], ...]:
    return tuple(
        tuple(sum(left[row][k] * right[k][column] for k in range(3)) for column in range(3))
        for row in range(3)
    )


def convert_position(value: dict[str, Any]) -> tuple[float, float, float]:
    x, y, z = float(value["x"]), float(value["y"]), float(value["z"])
    result = (x * 0.01, z * 0.01, -y * 0.01)
    if not all(math.isfinite(component) for component in result):
        raise ValueError("non-finite position")
    return result


def convert_scale(value: dict[str, Any]) -> tuple[float, float, float]:
    x, y, z = float(value["x"]), float(value["y"]), float(value["z"])
    result = (x, z, y)
    if not all(math.isfinite(component) and abs(component) >= 1.0e-6 for component in result):
        raise ValueError("invalid signed scale")
    return result


def scale_flags(scale: Sequence[float]) -> tuple[bool, bool]:
    any_negative = any(component < 0.0 for component in scale)
    reflected = scale[0] * scale[1] * scale[2] < 0.0
    return any_negative, reflected


def ue3_rotation_rows(rotation: dict[str, Any]) -> tuple[tuple[float, ...], ...]:
    pitch = float(rotation["pitch"]) * math.pi / 32768.0
    yaw = float(rotation["yaw"]) * math.pi / 32768.0
    roll = float(rotation["roll"]) * math.pi / 32768.0
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    sr, cr = math.sin(roll), math.cos(roll)
    return (
        (cp * cy, cp * sy, sp),
        (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp),
        (-cr * sp * cy - sr * sy, -cr * sp * sy + sr * cy, cr * cp),
    )


def directx_row_matrix_from_quaternion(
    quaternion: Sequence[float],
) -> tuple[tuple[float, ...], ...]:
    x, y, z, w = quaternion
    return (
        (1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y + w * z), 2.0 * (x * z - w * y)),
        (2.0 * (x * y - w * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z + w * x)),
        (2.0 * (x * z + w * y), 2.0 * (y * z - w * x), 1.0 - 2.0 * (x * x + y * y)),
    )


def standard_column_quaternion(matrix: Sequence[Sequence[float]]) -> tuple[float, float, float, float]:
    trace = matrix[0][0] + matrix[1][1] + matrix[2][2]
    if trace > 0.0:
        root = math.sqrt(trace + 1.0) * 2.0
        result = (
            (matrix[2][1] - matrix[1][2]) / root,
            (matrix[0][2] - matrix[2][0]) / root,
            (matrix[1][0] - matrix[0][1]) / root,
            0.25 * root,
        )
    elif matrix[0][0] > matrix[1][1] and matrix[0][0] > matrix[2][2]:
        root = math.sqrt(1.0 + matrix[0][0] - matrix[1][1] - matrix[2][2]) * 2.0
        result = (
            0.25 * root,
            (matrix[0][1] + matrix[1][0]) / root,
            (matrix[0][2] + matrix[2][0]) / root,
            (matrix[2][1] - matrix[1][2]) / root,
        )
    elif matrix[1][1] > matrix[2][2]:
        root = math.sqrt(1.0 + matrix[1][1] - matrix[0][0] - matrix[2][2]) * 2.0
        result = (
            (matrix[0][1] + matrix[1][0]) / root,
            0.25 * root,
            (matrix[1][2] + matrix[2][1]) / root,
            (matrix[0][2] - matrix[2][0]) / root,
        )
    else:
        root = math.sqrt(1.0 + matrix[2][2] - matrix[0][0] - matrix[1][1]) * 2.0
        result = (
            (matrix[0][2] + matrix[2][0]) / root,
            (matrix[1][2] + matrix[2][1]) / root,
            0.25 * root,
            (matrix[1][0] - matrix[0][1]) / root,
        )
    length = math.sqrt(sum(component * component for component in result))
    if length < QUATERNION_EPSILON:
        raise ValueError("zero rotation quaternion")
    normalized = tuple(component / length for component in result)
    if normalized[3] < 0.0:
        normalized = tuple(-component for component in normalized)
    return normalized


def convert_rotation(rotation: dict[str, Any]) -> tuple[float, float, float, float]:
    ue3 = ue3_rotation_rows(rotation)
    client = mat_mul(mat_mul(mat_transpose(BASIS), ue3), BASIS)
    quaternion = standard_column_quaternion(mat_transpose(client))
    round_trip = directx_row_matrix_from_quaternion(quaternion)
    error = max(abs(round_trip[row][column] - client[row][column]) for row in range(3) for column in range(3))
    if error > 1.0e-5:
        raise ValueError(f"rotation round-trip failed: {error}")
    return quaternion


def imported_id(source_placement_id: str) -> int:
    digest = hashlib.sha256(source_placement_id.encode("utf-8")).digest()
    return int.from_bytes(digest[:8], "big") | IMPORTED_ID_BIT


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def iter_placements(paths: Iterable[Path]) -> Iterable[dict[str, Any]]:
    for path in sorted(paths, key=lambda value: value.name):
        document = load_json(path)
        if document.get("schemaVersion") != 1 or document.get("propertyErrors"):
            raise ValueError(f"invalid placement source: {path}")
        for placement in document.get("placements", []):
            yield placement


def compile_scene(args: argparse.Namespace) -> dict[str, Any]:
    area_id = token(args.area_id, "areaId")
    asset_manifest = load_json(args.asset_manifest)
    runtime_manifest = load_json(args.runtime_manifest)
    assets = asset_manifest.get("assets", [])
    runtime_assets = runtime_manifest.get("assets", [])
    if asset_manifest.get("areaId") != area_id or runtime_manifest.get("areaId") != area_id:
        raise ValueError("manifest areaId mismatch")

    assets_by_path = {str(asset["fullPath"]).lower(): asset for asset in assets}
    runtime_by_id = {str(asset["assetId"]): asset for asset in runtime_assets}
    if len(assets_by_path) != len(assets) or len(runtime_by_id) != len(runtime_assets):
        raise ValueError("duplicate asset key")
    if set(str(asset["assetId"]) for asset in assets) != set(runtime_by_id):
        raise ValueError("asset/runtime manifest set mismatch")

    catalog_rows: list[str] = []
    for asset in sorted(assets, key=lambda value: str(value["assetId"])):
        asset_id = token(str(asset["assetId"]), "assetId")
        runtime = runtime_by_id[asset_id]
        model_relative = Path(str(runtime["model"]))
        model_absolute = args.runtime_root / model_relative
        if not model_absolute.is_file():
            raise ValueError(f"runtime model is missing: {model_absolute}")
        with model_absolute.open("rb") as model_stream:
            model_magic = model_stream.read(4)
        if model_magic not in (b"WINT", b"WMOD"):
            raise ValueError(f"invalid runtime model: {model_absolute}")
        model_path = (Path("Map") / area_id / model_relative).as_posix()
        source_group = token(str(asset["sourceCategory"]).lower(), "groupId")
        evidence = "UE3 ImportTable exact: " + str(asset["fullPath"])
        catalog_rows.append(
            " ".join((
                quoted(asset_id), quoted(str(asset["objectName"])), quoted(model_path),
                quoted("Prototype_Component_Model_" + asset_id), "1 1 1 Origin",
                quoted(source_group), quoted(str(asset["logicalPackage"])), quoted(evidence),
            ))
        )

    placement_rows: list[dict[str, Any]] = []
    seen_runtime_ids: dict[int, str] = {}
    any_negative_scale_count = 0
    reflected_count = 0
    source_level_counts: dict[str, int] = {}
    central_result: dict[str, Any] | None = None
    for placement in iter_placements(args.placements_dir.glob("*.placements.json")):
        source_id = str(placement["placementId"])
        runtime_id = imported_id(source_id)
        previous = seen_runtime_ids.get(runtime_id)
        if previous is not None and previous != source_id:
            raise ValueError(f"runtime ID collision: {previous!r} / {source_id!r}")
        seen_runtime_ids[runtime_id] = source_id
        object_path = str(placement["asset"]["objectPath"]).lower()
        asset = assets_by_path.get(object_path)
        if asset is None:
            raise ValueError(f"asset join missing: {object_path}")
        transform = placement["transform"]
        position = convert_position(transform["position"])
        rotation = convert_rotation(transform["rotation"])
        scale = convert_scale(transform["scale3D"])
        any_negative, reflected = scale_flags(scale)
        any_negative_scale_count += int(any_negative)
        reflected_count += int(reflected)
        source_level = token(str(placement["levelPackage"]), "sourceLevel")
        source_level_counts[source_level] = source_level_counts.get(source_level, 0) + 1
        transform_source = token(str(transform["source"]), "transformSource")
        if transform_source not in ("actor", "component"):
            raise ValueError(f"unexpected exact transform source: {transform_source}")
        values = (*position, *rotation, *scale)
        placement_rows.append(
            {
                "sourcePlacementId": source_id,
                "sourceLevel": source_level,
                "anyNegative": any_negative,
                "reflected": reflected,
                "text": (
                    f"{runtime_id} {quoted(source_id)} {quoted(source_level)} "
                    f"{quoted(transform_source)} {quoted(str(asset['assetId']))} "
                    + " ".join(format(value, ".9g") for value in values)
                    + " 1"
                ),
            }
        )
        if source_id == args.golden_placement_id:
            central_result = {
                "runtimeId": runtime_id,
                "assetId": asset["assetId"],
                "position": position,
                "quaternion": rotation,
                "signedScale": scale,
            }

    if args.expect_assets is not None and len(catalog_rows) != args.expect_assets:
        raise ValueError(f"asset count mismatch: {len(catalog_rows)}")
    if args.expect_source_placements is not None and len(placement_rows) != args.expect_source_placements:
        raise ValueError(f"source placement count mismatch: {len(placement_rows)}")
    if args.expect_any_negative is not None and any_negative_scale_count != args.expect_any_negative:
        raise ValueError(f"any-negative scale count mismatch: {any_negative_scale_count}")
    if args.expect_reflected is not None and reflected_count != args.expect_reflected:
        raise ValueError(f"reflected count mismatch: {reflected_count}")
    expected_level_counts: dict[str, int] = {}
    for specification in args.expect_level_count:
        level, separator, raw_count = specification.partition("=")
        if not separator:
            raise ValueError(f"invalid expected level count: {specification}")
        level = token(level, "expected sourceLevel")
        if level in expected_level_counts:
            raise ValueError(f"duplicate expected sourceLevel: {level}")
        expected_level_counts[level] = int(raw_count)
    if expected_level_counts and expected_level_counts != source_level_counts:
        raise ValueError(
            f"source level counts mismatch: {source_level_counts}"
        )
    if args.golden_placement_id and central_result is None:
        raise ValueError("golden placement is missing")

    requested_ids = set(args.include_source_id)
    requested_levels = set(args.include_level)
    if requested_ids and requested_levels:
        raise ValueError("include-source-id and include-level are mutually exclusive")
    placement_rows.sort(key=lambda row: str(row["sourcePlacementId"]))
    if requested_ids:
        selected_rows = [
            row for row in placement_rows
            if str(row["sourcePlacementId"]) in requested_ids
        ]
        found_ids = {str(row["sourcePlacementId"]) for row in selected_rows}
        if found_ids != requested_ids:
            raise ValueError(f"fixture source IDs are missing: {sorted(requested_ids - found_ids)}")
        output_scope = "fixture"
    elif requested_levels:
        selected_rows = [
            row for row in placement_rows
            if str(row["sourceLevel"]) in requested_levels
        ]
        found_levels = {str(row["sourceLevel"]) for row in selected_rows}
        if found_levels != requested_levels:
            raise ValueError(f"source levels are missing: {sorted(requested_levels - found_levels)}")
        output_scope = "levels"
    else:
        selected_rows = placement_rows
        output_scope = "all"

    if args.expect_output_placements is not None and len(selected_rows) != args.expect_output_placements:
        raise ValueError(f"output placement count mismatch: {len(selected_rows)}")
    output_any_negative = sum(bool(row["anyNegative"]) for row in selected_rows)
    output_reflected = sum(bool(row["reflected"]) for row in selected_rows)
    if args.expect_output_any_negative is not None and output_any_negative != args.expect_output_any_negative:
        raise ValueError(f"output any-negative count mismatch: {output_any_negative}")
    if args.expect_output_reflected is not None and output_reflected != args.expect_output_reflected:
        raise ValueError(f"output reflected count mismatch: {output_reflected}")

    catalog_text = (
        f"LOSTARK_MAP_ASSET_CATALOG 2 {quoted(area_id)} {len(catalog_rows)}\n"
        + "\n".join(catalog_rows) + "\n"
    )
    placement_text = (
        f"LOSTARK_MAP_PLACEMENTS 2 {quoted(area_id)} {len(selected_rows)}\n"
        + "\n".join(str(row["text"]) for row in selected_rows) + "\n"
    )
    atomic_write_text(args.catalog_output, catalog_text)
    atomic_write_text(args.placement_output, placement_text)
    receipt = {
        "schemaVersion": 1,
        "areaId": area_id,
        "outputScope": output_scope,
        "assetCount": len(catalog_rows),
        "sourcePlacementCount": len(placement_rows),
        "placementCount": len(selected_rows),
        "sourceAnyNegativeScaleCount": any_negative_scale_count,
        "sourceReflectedCount": reflected_count,
        "anyNegativeScaleCount": output_any_negative,
        "reflectedCount": output_reflected,
        "sourceLevelCounts": dict(sorted(source_level_counts.items())),
        "goldenPlacement": central_result,
        "inputs": {
            "assetManifest": sha256(args.asset_manifest),
            "runtimeManifest": sha256(args.runtime_manifest),
            "placements": {
                path.name: sha256(path)
                for path in sorted(
                    args.placements_dir.glob("*.placements.json"),
                    key=lambda value: value.name,
                )
            },
        },
        "outputs": {
            "catalog": sha256(args.catalog_output),
            "placements": sha256(args.placement_output),
        },
    }
    atomic_write_text(args.receipt_output, json.dumps(receipt, ensure_ascii=False, indent=2) + "\n")
    return receipt


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--asset-manifest", type=Path, required=True)
    parser.add_argument("--runtime-manifest", type=Path, required=True)
    parser.add_argument("--runtime-root", type=Path, required=True)
    parser.add_argument("--placements-dir", type=Path, required=True)
    parser.add_argument("--catalog-output", type=Path, required=True)
    parser.add_argument("--placement-output", type=Path, required=True)
    parser.add_argument("--receipt-output", type=Path, required=True)
    parser.add_argument("--golden-placement-id", default="")
    parser.add_argument("--include-source-id", action="append", default=[])
    parser.add_argument("--include-level", action="append", default=[])
    parser.add_argument("--expect-assets", type=int)
    parser.add_argument("--expect-source-placements", type=int)
    parser.add_argument("--expect-output-placements", type=int)
    parser.add_argument("--expect-output-any-negative", type=int)
    parser.add_argument("--expect-output-reflected", type=int)
    parser.add_argument("--expect-any-negative", type=int)
    parser.add_argument("--expect-reflected", type=int)
    parser.add_argument("--expect-level-count", action="append", default=[])
    return parser.parse_args()


def main() -> int:
    receipt = compile_scene(parse_args())
    print(json.dumps(receipt, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

### 5-2. `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/test_build_maptool_scene.py`

변경 종류: 추가  
적용 위치: 새 파일 전체

```python
import math
import unittest

from build_maptool_scene import (
    IMPORTED_ID_BIT,
    convert_position,
    convert_rotation,
    convert_scale,
    directx_row_matrix_from_quaternion,
    imported_id,
    scale_flags,
)


class MapToolSceneTransformTests(unittest.TestCase):
    def assert_vector_close(self, actual, expected, places=6):
        self.assertEqual(len(actual), len(expected))
        for left, right in zip(actual, expected):
            self.assertAlmostEqual(left, right, places=places)

    def test_position_basis_and_centimeter_to_meter(self):
        actual = convert_position({"x": 849.4918212890625, "y": 2140.9453125, "z": 975.8194580078125})
        self.assert_vector_close(actual, (8.494918212890625, 9.758194580078125, -21.409453125))

    def test_scale_reorders_axes_and_preserves_sign(self):
        actual = convert_scale({"x": -2.0, "y": 3.0, "z": 4.0})
        self.assertEqual(actual, (-2.0, 4.0, 3.0))

    def test_two_negative_axes_are_not_reflection(self):
        self.assertEqual(scale_flags((-2.0, -3.0, 4.0)), (True, False))

    def test_one_negative_axis_is_reflection(self):
        self.assertEqual(scale_flags((-2.0, 3.0, 4.0)), (True, True))

    def test_identity_rotation(self):
        actual = convert_rotation({"pitch": 0, "yaw": 0, "roll": 0})
        self.assert_vector_close(actual, (0.0, 0.0, 0.0, 1.0))

    def test_central_floor_yaw(self):
        actual = convert_rotation({"pitch": 0, "yaw": -16384, "roll": 0})
        root = math.sqrt(0.5)
        self.assert_vector_close(actual, (0.0, -root, 0.0, root))
        expected_rows = ((0.0, 0.0, 1.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0))
        rows = directx_row_matrix_from_quaternion(actual)
        for row, expected in zip(rows, expected_rows):
            self.assert_vector_close(row, expected)

    def test_imported_id_is_stable_and_uses_high_bit(self):
        source = "LV_LUT_HEARTRB_ED_SL00:export:1234"
        self.assertEqual(imported_id(source), 11534871182138487613)
        self.assertNotEqual(imported_id(source) & IMPORTED_ID_BIT, 0)
        self.assertEqual(imported_id(source), imported_id(source))


if __name__ == "__main__":
    unittest.main()
```

### 5-3. `C:/Users/user/Desktop/LostArk/Client/Public/MapPlacementDocument.h`

변경 종류: 추가  
적용 위치: 새 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CMapAssetCatalog;

struct MAP_PLACEMENT_RECORD
{
	uint64_t placementId = {};
	std::string sourcePlacementId;
	std::string sourceLevel;
	std::string transformSource;
	std::string assetId;
	float3_t position = {};
	float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t signedScale = float3_t(1.f, 1.f, 1.f);
	bool_t visible = true;
};

class CMapPlacementDocument final
{
public:
	static constexpr uint32_t MAX_PLACEMENT_COUNT = 65536;
	static constexpr uint64_t MAX_EDITOR_PLACEMENT_ID = 0x7fffffffffffffffull;

	static bool_t Read(const std::filesystem::path& path,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_PLACEMENT_RECORD>& outRecords,
		std::string& outStatus);
	static bool_t Write(const std::filesystem::path& path,
		const std::string& areaId,
		const std::vector<MAP_PLACEMENT_RECORD>& records,
		const CMapAssetCatalog& catalog,
		std::string& outStatus);
	static bool_t Is_Valid(const MAP_PLACEMENT_RECORD& record,
		const CMapAssetCatalog& catalog);
};

NS_END
```

### 5-4. `C:/Users/user/Desktop/LostArk/Client/Private/MapPlacementDocument.cpp`

변경 종류: 추가  
적용 위치: 새 파일 전체

```cpp
#include "MapPlacementDocument.h"

#include "MapAssetCatalog.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
	constexpr const char* PLACEMENT_MAGIC = "LOSTARK_MAP_PLACEMENTS";
	constexpr uint32_t LEGACY_PLACEMENT_VERSION = 1;
	constexpr uint32_t PLACEMENT_VERSION = 2;

	bool_t CommitTemporaryFile(const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) &&
			ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
			return true;
		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	bool_t NormalizeQuaternion(MAP_PLACEMENT_RECORD& record)
	{
		const vector_t value = XMLoadFloat4(&record.rotationQuaternion);
		const float length = XMVectorGetX(XMVector4Length(value));
		if (!std::isfinite(length) || length < 0.000001f)
			return false;
		vector_t normalized = XMQuaternionNormalize(value);
		if (XMVectorGetW(normalized) < 0.f)
			normalized = XMVectorNegate(normalized);
		XMStoreFloat4(&record.rotationQuaternion, normalized);
		return true;
	}
}

bool_t CMapPlacementDocument::Read(
	const std::filesystem::path& path,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_PLACEMENT_RECORD>& outRecords,
	std::string& outStatus)
{
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		outRecords.clear();
		outStatus = "No placement file; starting with an empty map";
		return true;
	}
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open placement file: " + path.string();
		return false;
	}
	std::string magic;
	std::string areaId;
	uint32_t version = {};
	uint32_t count = {};
	if (!(input >> magic >> version >> std::quoted(areaId) >> count) ||
		magic != PLACEMENT_MAGIC ||
		(version != LEGACY_PLACEMENT_VERSION && version != PLACEMENT_VERSION) ||
		areaId != catalog.Get_AreaId() || count > MAX_PLACEMENT_COUNT)
	{
		outStatus = "Placement header is invalid or belongs to another area";
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> staged;
	std::unordered_set<uint64_t> runtimeIds;
	std::unordered_set<std::string> sourceIds;
	staged.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		MAP_PLACEMENT_RECORD record{};
		int32_t visible = {};
		if (version == LEGACY_PLACEMENT_VERSION)
		{
			float3_t legacyRotation{};
			if (!(input >> record.placementId >> std::quoted(record.assetId) >>
				record.position.x >> record.position.y >> record.position.z >>
				legacyRotation.x >> legacyRotation.y >> legacyRotation.z >>
				record.signedScale.x >> record.signedScale.y >>
				record.signedScale.z >> visible))
			{
				outStatus = "Legacy placement row is truncated at index " +
					std::to_string(index);
				return false;
			}
			record.sourcePlacementId = "legacy:" +
				std::to_string(record.placementId);
			record.sourceLevel = "LEGACY";
			record.transformSource = "legacy";
			XMStoreFloat4(&record.rotationQuaternion,
				XMQuaternionRotationRollPitchYaw(
					XMConvertToRadians(legacyRotation.x),
					XMConvertToRadians(legacyRotation.y),
					XMConvertToRadians(legacyRotation.z)));
		}
		else
		{
			if (!(input >> record.placementId >>
				std::quoted(record.sourcePlacementId) >>
				std::quoted(record.sourceLevel) >>
				std::quoted(record.transformSource) >>
				std::quoted(record.assetId) >>
				record.position.x >> record.position.y >> record.position.z >>
				record.rotationQuaternion.x >> record.rotationQuaternion.y >>
				record.rotationQuaternion.z >> record.rotationQuaternion.w >>
				record.signedScale.x >> record.signedScale.y >>
				record.signedScale.z >> visible))
			{
				outStatus = "Placement v2 row is truncated at index " +
					std::to_string(index);
				return false;
			}
		}
		record.visible = 0 != visible;
		if (!NormalizeQuaternion(record) || !Is_Valid(record, catalog) ||
			!runtimeIds.insert(record.placementId).second ||
			!sourceIds.insert(record.sourcePlacementId).second)
		{
			outStatus = "Placement validation failed at index " +
				std::to_string(index);
			return false;
		}
		staged.push_back(std::move(record));
	}
	std::string trailing;
	if (input >> trailing)
	{
		outStatus = "Placement file contains unexpected trailing data";
		return false;
	}
	outRecords = std::move(staged);
	outStatus = "Placement document v" + std::to_string(version) +
		" validated: " + std::to_string(outRecords.size());
	return true;
}

bool_t CMapPlacementDocument::Write(
	const std::filesystem::path& path,
	const std::string& areaId,
	const std::vector<MAP_PLACEMENT_RECORD>& records,
	const CMapAssetCatalog& catalog,
	std::string& outStatus)
{
	if (records.size() > MAX_PLACEMENT_COUNT || areaId != catalog.Get_AreaId())
	{
		outStatus = "Placement save header is invalid";
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> normalized = records;
	std::unordered_set<uint64_t> runtimeIds;
	std::unordered_set<std::string> sourceIds;
	for (MAP_PLACEMENT_RECORD& record : normalized)
	{
		if (!NormalizeQuaternion(record) || !Is_Valid(record, catalog) ||
			!runtimeIds.insert(record.placementId).second ||
			!sourceIds.insert(record.sourcePlacementId).second)
		{
			outStatus = "Save aborted: duplicate ID or invalid placement";
			return false;
		}
	}
	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create placement directory";
		return false;
	}
	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary placement file";
		return false;
	}
	output << PLACEMENT_MAGIC << ' ' << PLACEMENT_VERSION << ' ' <<
		std::quoted(areaId) << ' ' << normalized.size() << '\n';
	output << std::setprecision(9);
	for (const MAP_PLACEMENT_RECORD& record : normalized)
	{
		output << record.placementId << ' ' <<
			std::quoted(record.sourcePlacementId) << ' ' <<
			std::quoted(record.sourceLevel) << ' ' <<
			std::quoted(record.transformSource) << ' ' <<
			std::quoted(record.assetId) << ' ' <<
			record.position.x << ' ' << record.position.y << ' ' <<
			record.position.z << ' ' <<
			record.rotationQuaternion.x << ' ' <<
			record.rotationQuaternion.y << ' ' <<
			record.rotationQuaternion.z << ' ' <<
			record.rotationQuaternion.w << ' ' <<
			record.signedScale.x << ' ' << record.signedScale.y << ' ' <<
			record.signedScale.z << ' ' << (record.visible ? 1 : 0) << '\n';
	}
	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit placement file atomically";
		return false;
	}
	outStatus = "Saved placement document v2: " +
		std::to_string(normalized.size());
	return true;
}

bool_t CMapPlacementDocument::Is_Valid(
	const MAP_PLACEMENT_RECORD& record,
	const CMapAssetCatalog& catalog)
{
	const auto finite3 = [](const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	};
	const auto validToken = [](const std::string& value, size_t maximum)
	{
		return !value.empty() && value.size() <= maximum &&
			std::all_of(value.begin(), value.end(), [](unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	};
	const auto validSourceId = [](const std::string& value)
	{
		return !value.empty() && value.size() <= 256 &&
			std::none_of(value.begin(), value.end(), [](unsigned char character)
			{
				return 0 != std::iscntrl(character);
			});
	};
	const float quaternionLength = std::sqrt(
		record.rotationQuaternion.x * record.rotationQuaternion.x +
		record.rotationQuaternion.y * record.rotationQuaternion.y +
		record.rotationQuaternion.z * record.rotationQuaternion.z +
		record.rotationQuaternion.w * record.rotationQuaternion.w);
	const bool_t importedSource =
		record.transformSource == "actor" ||
		record.transformSource == "component";
	const bool_t editorSource =
		record.transformSource == "editor" ||
		record.transformSource == "legacy";
	const bool_t validIdDomain =
		(importedSource && 0 != (record.placementId & 0x8000000000000000ull)) ||
		(editorSource && record.placementId <= MAX_EDITOR_PLACEMENT_ID);
	return 0 != record.placementId &&
		validSourceId(record.sourcePlacementId) &&
		validToken(record.sourceLevel, 128) &&
		validToken(record.transformSource, 32) && validIdDomain &&
		!record.assetId.empty() && nullptr != catalog.Find(record.assetId) &&
		finite3(record.position) && finite3(record.signedScale) &&
		std::isfinite(record.rotationQuaternion.x) &&
		std::isfinite(record.rotationQuaternion.y) &&
		std::isfinite(record.rotationQuaternion.z) &&
		std::isfinite(record.rotationQuaternion.w) &&
		quaternionLength >= 0.000001f &&
		std::abs(record.signedScale.x) >= 0.000001f &&
		std::abs(record.signedScale.y) >= 0.000001f &&
		std::abs(record.signedScale.z) >= 0.000001f;
}
```

### 5-5. `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetCatalog.h`

변경 종류: 전체 교체  
적용 위치: 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class MAP_ASSET_ANCHOR
{
	ORIGIN,
	BOTTOM_CENTER,
};

struct MAP_ASSET_ENTRY
{
	std::string id;
	std::string label;
	std::string groupId;
	std::string groupLabel;
	std::string evidence;
	std::filesystem::path modelRelativePath;
	std::filesystem::path resolvedModelPath;
	std::wstring prototypeTag;
	float3_t defaultScale = float3_t(1.f, 1.f, 1.f);
	MAP_ASSET_ANCHOR anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
};

class CMapAssetCatalog final
{
public:
	bool_t Load_Default();
	bool_t Load(const std::filesystem::path& path,
		const std::string& expectedAreaId = {});

	const MAP_ASSET_ENTRY* Find(const std::string& assetId) const;
	const std::vector<MAP_ASSET_ENTRY>& Get_Entries() const { return m_Entries; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Status() const { return m_Status; }
	const std::filesystem::path& Get_CatalogPath() const { return m_CatalogPath; }
	const std::filesystem::path& Get_PlacementPath() const { return m_PlacementPath; }
	bool_t Is_Ready() const { return m_bReady; }

	static std::filesystem::path Get_MapDataRoot();
	static std::filesystem::path Get_AreaSelectionPath();

private:
	std::vector<MAP_ASSET_ENTRY> m_Entries;
	std::string m_AreaId;
	std::string m_Status = "Catalog not loaded";
	std::filesystem::path m_CatalogPath;
	std::filesystem::path m_PlacementPath;
	bool_t m_bReady = false;
};

NS_END
```

### 5-6. `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp`

변경 종류: helper와 `Load_Default`, `Load` 최종 교체  
적용 위치: anonymous namespace 및 대응 함수

`Load_Default`는 다음 전체 함수로 교체한다.

```cpp
bool_t CMapAssetCatalog::Load_Default()
{
	const std::filesystem::path selectionPath = Get_AreaSelectionPath();
	std::ifstream input(selectionPath, std::ios::binary);
	std::string magic;
	std::string selectedAreaId;
	uint32_t version = {};
	if (!input || !(input >> magic >> version >> std::quoted(selectedAreaId)) ||
		magic != "LOSTARK_MAP_AREA_SELECTION" || version != 1 ||
		!IsValidGroupId(selectedAreaId))
	{
		m_Status = "Active map area selection is invalid: " + selectionPath.string();
		return false;
	}
	std::string trailing;
	if (input >> trailing)
	{
		m_Status = "Active map area selection has trailing data";
		return false;
	}

	const std::filesystem::path mapRoot = Get_MapDataRoot();
	const std::filesystem::path catalogPath =
		mapRoot / (std::filesystem::path(selectedAreaId).wstring() + L".mapassets");
	if (!Load(catalogPath, selectedAreaId))
		return false;
	m_CatalogPath = catalogPath;
	m_PlacementPath =
		mapRoot / (std::filesystem::path(selectedAreaId).wstring() + L".mapplacements");
	return true;
}
```

`Load`는 다음 전체 함수로 교체한다. 실패 시 기존 Catalog 멤버는 그대로 유지되고 status만 실패 원인으로 바뀐다.

```cpp
bool_t CMapAssetCatalog::Load(const std::filesystem::path& path,
	const std::string& expectedAreaId)
{
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_Status = "Catalog missing: " + path.string();
		return false;
	}
	std::string magic;
	uint32_t version = {};
	uint32_t count = {};
	std::string stagedAreaId;
	if (!(input >> magic >> version >> std::quoted(stagedAreaId) >> count) ||
		magic != CATALOG_MAGIC ||
		(version != LEGACY_CATALOG_VERSION && version != CATALOG_VERSION) ||
		stagedAreaId.empty() ||
		(!expectedAreaId.empty() && stagedAreaId != expectedAreaId) ||
		0 == count || count > MAX_ASSET_COUNT)
	{
		m_Status = "Catalog header is invalid";
		return false;
	}
	std::vector<PARSED_MAP_ASSET_ROW> parsedRows;
	parsedRows.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		PARSED_MAP_ASSET_ROW row{};
		if (!(input >> std::quoted(row.id) >> std::quoted(row.label) >>
			std::quoted(row.modelPath) >> std::quoted(row.prototypeTag) >>
			row.defaultScale.x >> row.defaultScale.y >> row.defaultScale.z >>
			row.anchor))
		{
			m_Status = "Catalog row is truncated at index " +
				std::to_string(index);
			return false;
		}
		if (version == CATALOG_VERSION)
		{
			if (!(input >> std::quoted(row.groupId) >>
				std::quoted(row.groupLabel) >> std::quoted(row.evidence)))
			{
				m_Status = "Catalog metadata is truncated at index " +
					std::to_string(index);
				return false;
			}
		}
		else
		{
			row.groupId = "legacy";
			row.groupLabel = "Legacy Catalog";
			row.evidence = "catalog-v1";
		}
		parsedRows.push_back(std::move(row));
	}
	std::string trailing;
	if (input >> trailing)
	{
		m_Status = "Catalog contains unexpected trailing data";
		return false;
	}
	const std::filesystem::path assetRoot = CRuntimeAssetRoot::Get();
	if (assetRoot.empty() || !std::filesystem::exists(assetRoot))
	{
		m_Status = "LostArk runtime asset root is missing";
		return false;
	}
	std::unordered_set<std::string> ids;
	std::unordered_set<std::wstring> prototypeTags;
	std::vector<MAP_ASSET_ENTRY> stagedEntries;
	stagedEntries.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		const PARSED_MAP_ASSET_ROW& row = parsedRows[index];
		MAP_ASSET_ENTRY entry{};
		entry.id = row.id;
		entry.label = row.label;
		entry.groupId = row.groupId;
		entry.groupLabel = row.groupLabel;
		entry.evidence = row.evidence;
		entry.defaultScale = row.defaultScale;
		entry.modelRelativePath =
			std::filesystem::path(row.modelPath).lexically_normal();
		entry.resolvedModelPath =
			CRuntimeAssetRoot::Resolve(entry.modelRelativePath);
		entry.prototypeTag.assign(row.prototypeTag.begin(), row.prototypeTag.end());
		if (row.anchor == "Origin")
			entry.anchor = MAP_ASSET_ANCHOR::ORIGIN;
		else if (row.anchor == "BottomCenter")
			entry.anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
		else
		{
			m_Status = "Unknown placement anchor for " + entry.id;
			return false;
		}
		if (entry.id.empty() || entry.label.empty() || entry.prototypeTag.empty() ||
			!IsValidGroupId(entry.groupId) ||
			!IsValidDisplayText(entry.groupLabel, MAX_GROUP_LABEL_LENGTH) ||
			!IsValidDisplayText(entry.evidence, MAX_EVIDENCE_LENGTH) ||
			entry.modelRelativePath.is_absolute() ||
			entry.modelRelativePath.extension() != L".wmodel" ||
			!IsValidScale(entry.defaultScale) ||
			!ids.insert(entry.id).second ||
			!prototypeTags.insert(entry.prototypeTag).second ||
			!IsInsideRoot(assetRoot, entry.resolvedModelPath) ||
			!std::filesystem::exists(entry.resolvedModelPath))
		{
			m_Status = "Catalog validation failed for " + entry.id;
			return false;
		}
		stagedEntries.push_back(std::move(entry));
	}
m_Entries = std::move(stagedEntries);
m_AreaId = std::move(stagedAreaId);
m_CatalogPath = path;
m_PlacementPath = path.parent_path() /
	(std::filesystem::path(m_AreaId).wstring() + L".mapplacements");
m_bReady = true;
m_Status = "Catalog ready (v" + std::to_string(version) + "): " +
	std::to_string(m_Entries.size());
return true;
}
```

경로 helper는 다음 전체 구현으로 교체한다.

```cpp
std::filesystem::path CMapAssetCatalog::Get_MapDataRoot()
{
	wchar_t modulePath[32768]{};
	const DWORD length = GetModuleFileNameW(
		nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
	if (0 == length || length >= std::size(modulePath))
		return {};
	return (std::filesystem::path(modulePath).parent_path() /
		L"DataFiles" / L"Map").lexically_normal();
}

std::filesystem::path CMapAssetCatalog::Get_AreaSelectionPath()
{
	return Get_MapDataRoot() / L"ACTIVE.maparea";
}
```

기존 `Get_DefaultCatalogPath`, `Get_DefaultPlacementPath`, anonymous `GetDataFilePath`는 삭제한다.

### 5-7. `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetObject.h`

변경 종류: 전체 교체  
적용 위치: 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetObject final : public CGameObject
{
public:
	struct MAP_ASSET_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint64_t placementId = {};
		std::string assetId;
		std::wstring modelPrototypeTag;
		float3_t position = {};
		float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
		float3_t signedScale = float3_t(1.f, 1.f, 1.f);
		bool_t applyBottomCenter = false;
		bool_t visible = true;
	};

private:
	CMapAssetObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CMapAssetObject();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	uint64_t Get_PlacementId() const { return m_iPlacementId; }
	const std::string& Get_AssetId() const { return m_AssetId; }
	const float3_t& Get_Position() const { return m_vPlacementPosition; }
	const float4_t& Get_RotationQuaternion() const { return m_vRotationQuaternion; }
	const float3_t& Get_SignedScale() const { return m_vSignedScale; }
	bool_t Is_Visible() const { return m_bVisible; }
	bool_t Is_Mirrored() const { return m_bMirrored; }

	void Set_PlacementTransform(const float3_t& position,
		const float4_t& rotationQuaternion, const float3_t& signedScale);
	void Set_Visible(bool_t visible) { m_bVisible = visible; }

private:
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	float3_t m_vPlacementPosition = {};
	float4_t m_vRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t m_vSignedScale = float3_t(1.f, 1.f, 1.f);
	bool_t m_bApplyBottomCenter = false;
	bool_t m_bVisible = true;
	bool_t m_bMirrored = false;
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };

private:
	HRESULT Ready_Components(const std::wstring& modelPrototypeTag);
	HRESULT Bind_ShaderResources();
	float3_t Compute_WorldOrigin(const float3_t& placementPosition,
		const float4_t& rotationQuaternion, const float3_t& signedScale) const;

public:
	static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 5-8. `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp`

변경 종류: 함수 교체  
적용 위치: `Initialize`, `Render`, `Set_PlacementTransform`, `Bind_ShaderResources`, `Compute_WorldOrigin`

기존 include 블록에 다음을 추가한다.

```cpp
#include <cmath>
```

`Set_PlacementTransform` 전체 교체 코드는 다음과 같다.

```cpp
void CMapAssetObject::Set_PlacementTransform(const float3_t& position,
	const float4_t& rotationQuaternion, const float3_t& signedScale)
{
	vector_t quaternion = XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion));
	if (XMVectorGetW(quaternion) < 0.f)
		quaternion = XMVectorNegate(quaternion);
	XMStoreFloat4(&m_vRotationQuaternion, quaternion);
	m_vPlacementPosition = position;
	m_vSignedScale = signedScale;
	m_bMirrored = signedScale.x * signedScale.y * signedScale.z < 0.f;
	const float3_t worldOrigin = Compute_WorldOrigin(
		position, m_vRotationQuaternion, signedScale);
	const matrix_t world = XMMatrixScaling(
		signedScale.x, signedScale.y, signedScale.z) *
		XMMatrixRotationQuaternion(quaternion);
	m_pTransformCom->Set_State(STATE::RIGHT, world.r[0]);
	m_pTransformCom->Set_State(STATE::UP, world.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, world.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f));
}
```

`Initialize`와 `Render`는 다음 전체 함수로 교체한다.

```cpp
HRESULT CMapAssetObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	const MAP_ASSET_DESC desc = *static_cast<MAP_ASSET_DESC*>(pArg);
	const vector_t quaternion = XMLoadFloat4(&desc.rotationQuaternion);
	const float quaternionLength = XMVectorGetX(XMVector4Length(quaternion));
	if (0 == desc.placementId || desc.assetId.empty() ||
		desc.modelPrototypeTag.empty() ||
		!std::isfinite(quaternionLength) || quaternionLength < 0.000001f ||
		std::abs(desc.signedScale.x) < 0.000001f ||
		std::abs(desc.signedScale.y) < 0.000001f ||
		std::abs(desc.signedScale.z) < 0.000001f)
		return E_FAIL;
	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components(desc.modelPrototypeTag)))
		return E_FAIL;
	m_iPlacementId = desc.placementId;
	m_AssetId = desc.assetId;
	m_bApplyBottomCenter = desc.applyBottomCenter;
	m_bVisible = desc.visible;
	Set_PlacementTransform(
		desc.position, desc.rotationQuaternion, desc.signedScale);
	return S_OK;
}

HRESULT CMapAssetObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;
	const uint32_t passIndex = m_bMirrored ? 1u : 0u;
	for (uint32_t meshIndex = 0;
		meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(
				meshIndex, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", meshIndex,
			aiTextureType_DIFFUSE)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormalTexture,
				sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture && FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", meshIndex,
				aiTextureType_NORMALS))) ||
			FAILED(m_pShaderCom->Begin(passIndex)) ||
			FAILED(m_pModelCom->Render(meshIndex)))
			return E_FAIL;
	}
	return S_OK;
}
```

`Bind_ShaderResources`는 world에서 translation을 제거한 뒤 inverse-transpose를 bind한다.

```cpp
HRESULT CMapAssetObject::Bind_ShaderResources()
{
	matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	const matrix_t inverseTranspose =
		XMMatrixTranspose(XMMatrixInverse(nullptr, world));
	float4x4_t storedInverseTranspose{};
	XMStoreFloat4x4(&storedInverseTranspose, inverseTranspose);
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(m_pShaderCom->Bind_Matrix(
			"g_WorldInvTransposeMatrix", &storedInverseTranspose)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}
```

`Compute_WorldOrigin`은 다음 전체 함수로 교체한다. exact Catalog는 모두 `Origin`이므로 이 함수는 legacy `BottomCenter`에서만 offset을 계산한다.

```cpp
float3_t CMapAssetObject::Compute_WorldOrigin(
	const float3_t& placementPosition,
	const float4_t& rotationQuaternion,
	const float3_t& signedScale) const
{
	float3_t worldOrigin = placementPosition;
	if (m_bApplyBottomCenter && m_pModelCom->Has_LocalBounds())
	{
		const float3_t& minimum = m_pModelCom->Get_LocalBoundsMin();
		const float3_t& maximum = m_pModelCom->Get_LocalBoundsMax();
		const vector_t localAnchor = XMVectorSet(
			(minimum.x + maximum.x) * 0.5f,
			minimum.y,
			(minimum.z + maximum.z) * 0.5f,
			1.f);
		const matrix_t transform = XMMatrixScaling(
			signedScale.x, signedScale.y, signedScale.z) *
			XMMatrixRotationQuaternion(
				XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion)));
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(localAnchor, transform));
		worldOrigin.x -= anchorOffset.x;
		worldOrigin.y -= anchorOffset.y;
		worldOrigin.z -= anchorOffset.z;
	}
	return worldOrigin;
}
```

### 5-9. `C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl`

변경 종류: 전체 교체  
적용 위치: 파일 전체

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_WorldInvTransposeMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
uint g_HasNormalTexture = 0;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
    matrix worldViewProjection = mul(worldView, g_ProjMatrix);
    output.vPosition = mul(float4(input.vPosition, 1.f), worldViewProjection);
    float3 normal = normalize(
        mul(float4(input.vNormal, 0.f), g_WorldInvTransposeMatrix).xyz);
    float3 tangentLinear =
        mul(float4(input.vTangent, 0.f), g_WorldMatrix).xyz;
    float3 tangent = normalize(
        tangentLinear - normal * dot(tangentLinear, normal));
    float3 sourceBinormalLinear =
        mul(float4(input.vBinormal, 0.f), g_WorldMatrix).xyz;
    float handedness =
        dot(cross(normal, tangent), sourceBinormalLinear) < 0.f ? -1.f : 1.f;
    float3 binormal = normalize(cross(normal, tangent)) * handedness;
    output.vNormal = float4(normal, 0.f);
    output.vTangent = float4(tangent, 0.f);
    output.vBinormal = float4(binormal, 0.f);
    output.vTexcoord = input.vTexcoord;
    output.vWorldPos = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
    output.vProjPos = output.vPosition;
    return output;
}

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
};

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;
    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        float3 tangentNormal =
            g_NormalTexture.Sample(LinearSampler, input.vTexcoord).xyz * 2.f - 1.f;
        float3x3 tangentToWorld = float3x3(
            normalize(input.vTangent.xyz),
            normalize(input.vBinormal.xyz) * -1.f,
            normal);
        normal = normalize(mul(tangentNormal, tangentToWorld));
    }
    output.vDiffuse = diffuse;
    output.vNormal = float4(normal * 0.5f + 0.5f, 0.f);
    output.vDepth = float4(
        input.vProjPos.z / input.vProjPos.w,
        input.vProjPos.w / 1000.f, 0.f, 0.f);
    output.vPickPos = input.vWorldPos;
    return output;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass MirroredPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
```

### 5-10. `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h`

변경 종류: 선언 교체  
적용 위치: `PLACED_ENTRY`, placement 함수 선언, 멤버

```cpp
struct PLACED_ENTRY
{
	MAP_PLACEMENT_RECORD record;
	std::wstring layerTag;
	shared_ptr<CMapAssetObject> object;
};
```

```cpp
bool_t Try_PlaceSelected();
bool_t Create_Placement(const MAP_PLACEMENT_RECORD& record,
	PLACED_ENTRY& outEntry);
bool_t Remove_Placement(uint64_t placementId);
void Remove_AllPlacements();
bool_t Save_Placements();
bool_t Load_Placements();
uint64_t Allocate_EditorPlacementId();
std::wstring Make_LayerTag(const std::string& sourceLevel) const;
```

`Has_ValtanCorePlacements`, `Assemble_ValtanCore` 선언은 삭제한다. header에 `#include "MapPlacementDocument.h"`를 추가하고, `m_iNextPlacementId`는 editor ID 후보로만 사용한다.

### 5-11. `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp`

변경 종류: namespace 상수/함수와 placement 함수 교체  
적용 위치: anonymous namespace, `Try_PlaceSelected`부터 `Load_Placements`, toolbar/hierarchy/inspector

- `PLACEMENT_VERSION`, `MAX_PLACEMENT_COUNT`, `VALTAN_CORE_ASSET_IDS`, `STORED_PLACEMENT`, `ReadPlacementDocument`, `CommitTemporaryFile`, Valtan core helper를 삭제한다.
- `Save_Placements`와 `Load_Placements`는 `CMapPlacementDocument::Write/Read`만 호출한다.
- `Create_Placement`는 record의 `sourceLevel`로 layer tag를 만들고 quaternion/signed scale을 desc에 전달한다.
- rollback과 삭제는 각 `PLACED_ENTRY.layerTag`를 사용한다.
- hierarchy label에는 `[sourceLevel] label`과 sourcePlacementId를 표시한다.
- inspector는 position, quaternion, signed scale을 편집하며 quaternion normalize와 scale epsilon 검증 후에만 적용한다.

`Create_Placement` 전체 교체 코드는 다음과 같다.

```cpp
bool_t CMapTool::Create_Placement(const MAP_PLACEMENT_RECORD& record,
	PLACED_ENTRY& outEntry)
{
	const MAP_ASSET_ENTRY* pAsset = m_Catalog.Find(record.assetId);
	if (nullptr == pAsset ||
		!CMapPlacementDocument::Is_Valid(record, m_Catalog))
		return false;
	const std::wstring layerTag = Make_LayerTag(record.sourceLevel);
	CMapAssetObject::MAP_ASSET_DESC desc{};
	desc.placementId = record.placementId;
	desc.assetId = pAsset->id;
	desc.modelPrototypeTag = pAsset->prototypeTag;
	desc.position = record.position;
	desc.rotationQuaternion = record.rotationQuaternion;
	desc.signedScale = record.signedScale;
	desc.applyBottomCenter = MAP_ASSET_ANCHOR::BOTTOM_CENTER == pAsset->anchor;
	desc.visible = record.visible;
	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST), L"Prototype_GameObject_MapAsset",
		ETOUI(LEVEL::ASSET_TEST), layerTag, &desc, &pGameObject)))
		return false;
	shared_ptr<CMapAssetObject> pMapObject =
		dynamic_pointer_cast<CMapAssetObject>(pGameObject);
	if (nullptr == pMapObject)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), layerTag, pGameObject);
		return false;
	}
	outEntry.record = record;
	outEntry.layerTag = layerTag;
	outEntry.object = std::move(pMapObject);
	return true;
}
```

`Try_PlaceSelected`, editor ID 할당, layer tag 생성은 다음 전체 함수로 교체한다.

```cpp
bool_t CMapTool::Try_PlaceSelected()
{
	const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
	if (nullptr == pAsset)
	{
		m_Status = "Select an asset before placing";
		return false;
	}
	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_Status = "No valid surface under the cursor";
		return false;
	}
	const uint64_t placementId = Allocate_EditorPlacementId();
	if (0 == placementId)
	{
		m_Status = "No editor placement ID is available";
		return false;
	}
	MAP_PLACEMENT_RECORD record{};
	record.placementId = placementId;
	record.sourcePlacementId = "editor:" + std::to_string(placementId);
	record.sourceLevel = "EDITOR";
	record.transformSource = "editor";
	record.assetId = pAsset->id;
	record.position = position;
	record.rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	record.signedScale = pAsset->defaultScale;
	record.visible = true;
	PLACED_ENTRY placed{};
	if (!Create_Placement(record, placed))
	{
		m_Status = "Failed to clone map object for " + pAsset->id;
		return false;
	}
	m_iSelectedPlacementId = placementId;
	m_Placements.push_back(std::move(placed));
	m_bDirty = true;
	m_Status = "Placed " + pAsset->label +
		"; placement remains armed (Esc cancels).";
	return true;
}

uint64_t CMapTool::Allocate_EditorPlacementId()
{
	for (size_t attempt = 0; attempt <= m_Placements.size(); ++attempt)
	{
		if (0 == m_iNextPlacementId ||
			m_iNextPlacementId > CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
			m_iNextPlacementId = 1;
		const uint64_t candidate = m_iNextPlacementId++;
		if (nullptr == Find_Placement(candidate))
			return candidate;
	}
	return 0;
}

std::wstring CMapTool::Make_LayerTag(const std::string& sourceLevel) const
{
	std::wstring layerTag = L"Layer_MapAsset_";
	layerTag.append(sourceLevel.begin(), sourceLevel.end());
	return layerTag;
}
```

삭제 함수와 저장 함수는 다음 전체 함수로 교체한다.

```cpp
bool_t CMapTool::Remove_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	if (iter == m_Placements.end())
		return false;
	if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
		ETOUI(LEVEL::ASSET_TEST), iter->layerTag,
		static_pointer_cast<CGameObject>(iter->object))))
		return false;
	m_Placements.erase(iter);
	if (m_iSelectedPlacementId == placementId)
		m_iSelectedPlacementId = 0;
	m_bDirty = true;
	return true;
}

void CMapTool::Remove_AllPlacements()
{
	for (const PLACED_ENTRY& entry : m_Placements)
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), entry.layerTag,
			static_pointer_cast<CGameObject>(entry.object));
	m_Placements.clear();
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	m_bDirty = true;
}

bool_t CMapTool::Save_Placements()
{
	vector<MAP_PLACEMENT_RECORD> document;
	document.reserve(m_Placements.size());
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		if (nullptr == entry.object ||
			nullptr == m_Catalog.Find(entry.record.assetId))
		{
			m_Status = "Save aborted: placement references are invalid";
			return false;
		}
		MAP_PLACEMENT_RECORD stored = entry.record;
		stored.position = entry.object->Get_Position();
		stored.rotationQuaternion = entry.object->Get_RotationQuaternion();
		stored.signedScale = entry.object->Get_SignedScale();
		stored.visible = entry.object->Is_Visible();
		if (!CMapPlacementDocument::Is_Valid(stored, m_Catalog))
		{
			m_Status = "Save aborted: a transform is invalid";
			return false;
		}
		document.push_back(std::move(stored));
	}
	if (!CMapPlacementDocument::Write(
		m_Catalog.Get_PlacementPath(), m_Catalog.Get_AreaId(),
		document, m_Catalog, m_Status))
		return false;
	m_bDirty = false;
	return true;
}
```

`Load_Placements` 전체 교체 코드는 다음과 같다.

```cpp
bool_t CMapTool::Load_Placements()
{
	if (!m_Catalog.Is_Ready())
		return false;
	vector<MAP_PLACEMENT_RECORD> document;
	std::string loadStatus;
	if (!CMapPlacementDocument::Read(
		m_Catalog.Get_PlacementPath(), m_Catalog, document, loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}
	vector<PLACED_ENTRY> staged;
	staged.reserve(document.size());
	for (const MAP_PLACEMENT_RECORD& record : document)
	{
		PLACED_ENTRY entry{};
		if (!Create_Placement(record, entry))
		{
			for (const PLACED_ENTRY& rollback : staged)
				CGameInstance::Get().Remove_GameObject_from_Layer(
					ETOUI(LEVEL::ASSET_TEST), rollback.layerTag,
					static_pointer_cast<CGameObject>(rollback.object));
			m_Status = "Load rolled back at " + record.sourcePlacementId;
			return false;
		}
		staged.push_back(std::move(entry));
	}
	for (const PLACED_ENTRY& old : m_Placements)
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), old.layerTag,
			static_pointer_cast<CGameObject>(old.object));
	m_Placements = std::move(staged);
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : m_Placements)
		if (entry.record.placementId <= CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
	m_bDirty = false;
	m_Status = "Loaded " + std::to_string(m_Placements.size()) +
		" exact placements from " + m_Catalog.Get_AreaId();
	return true;
}
```

toolbar, hierarchy, inspector와 lookup은 다음 전체 함수로 교체한다.

```cpp
void CMapTool::Render_Toolbar()
{
	if (ImGui::Button("Save"))
		Save_Placements();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
		Load_Placements();
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		ImGui::OpenPopup("Clear all placements?");
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == Get_SelectedAsset() ||
		PLACEMENT_STATE::ARMED == m_ePlacementState);
	if (ImGui::Button("Arm placement"))
		Arm_SelectedAsset();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Text("Objects: %zu%s", m_Placements.size(),
		m_bDirty ? "  *unsaved" : "");
	if (ImGui::BeginPopupModal("Clear all placements?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted("Remove every placed map object from this level?");
		if (ImGui::Button("Clear all"))
		{
			Remove_AllPlacements();
			m_Status = "Cleared all placements (not saved yet)";
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	if (PLACEMENT_STATE::ARMED == m_ePlacementState)
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PLACEMENT ARMED: click the world (Esc cancels)");
	ImGui::Separator();
}

void CMapTool::Render_Hierarchy(f32_t childHeight)
{
	ImGui::TextUnformatted("Hierarchy");
	const f32_t listHeight = (std::max)(120.f, childHeight -
		ImGui::GetTextLineHeightWithSpacing());
	ImGui::BeginChild("PlacementHierarchy", ImVec2(0.f, listHeight), true);
	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(m_Placements.size()));
	while (clipper.Step())
	{
		for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
		{
			const PLACED_ENTRY& entry = m_Placements[index];
			const MAP_ASSET_ENTRY* pAsset = m_Catalog.Find(entry.record.assetId);
			const std::string assetLabel = nullptr == pAsset ?
				entry.record.assetId : pAsset->label;
			const std::string label = "[" + entry.record.sourceLevel + "] " +
				assetLabel + "###placement-" +
				std::to_string(entry.record.placementId);
			ImGui::PushID(reinterpret_cast<void*>(
				static_cast<uintptr_t>(entry.record.placementId)));
			const bool_t selected =
				entry.record.placementId == m_iSelectedPlacementId;
			if (ImGui::Selectable(label.c_str(), selected))
				m_iSelectedPlacementId = entry.record.placementId;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", entry.record.sourcePlacementId.c_str());
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

void CMapTool::Render_Inspector()
{
	ImGui::TextUnformatted("Inspector");
	PLACED_ENTRY* pEntry = Find_Placement(m_iSelectedPlacementId);
	if (nullptr == pEntry || nullptr == pEntry->object)
	{
		ImGui::TextDisabled("Select a placed object.");
		return;
	}
	CMapAssetObject& object = *pEntry->object;
	ImGui::Text("Placement #%llu",
		static_cast<unsigned long long>(pEntry->record.placementId));
	ImGui::TextWrapped("Source: %s", pEntry->record.sourcePlacementId.c_str());
	ImGui::Text("Level: %s | Transform: %s",
		pEntry->record.sourceLevel.c_str(),
		pEntry->record.transformSource.c_str());
	ImGui::TextWrapped("Asset: %s", pEntry->record.assetId.c_str());
	float3_t position = object.Get_Position();
	float4_t quaternion = object.Get_RotationQuaternion();
	float3_t scale = object.Get_SignedScale();
	bool_t visible = object.Is_Visible();
	const bool_t positionChanged =
		ImGui::DragFloat3("Position", &position.x, 0.1f);
	const bool_t rotationChanged =
		ImGui::DragFloat4("Rotation quaternion", &quaternion.x, 0.0025f);
	const bool_t scaleChanged =
		ImGui::DragFloat3("Signed scale", &scale.x, 0.01f, -1000.f, 1000.f);
	if (positionChanged || rotationChanged || scaleChanged)
	{
		const vector_t rawQuaternion = XMLoadFloat4(&quaternion);
		const float quaternionLength =
			XMVectorGetX(XMVector4Length(rawQuaternion));
		const bool_t scaleIsValid =
			std::abs(scale.x) >= 0.000001f &&
			std::abs(scale.y) >= 0.000001f &&
			std::abs(scale.z) >= 0.000001f;
		if (!std::isfinite(quaternionLength) ||
			quaternionLength < 0.000001f || !scaleIsValid)
			m_Status = "Transform edit rejected: zero quaternion/scale axis";
		else
		{
			XMStoreFloat4(&quaternion,
				XMQuaternionNormalize(rawQuaternion));
			object.Set_PlacementTransform(position, quaternion, scale);
			pEntry->record.position = position;
			pEntry->record.rotationQuaternion = object.Get_RotationQuaternion();
			pEntry->record.signedScale = scale;
			m_bDirty = true;
		}
	}
	if (ImGui::Checkbox("Visible", &visible))
	{
		object.Set_Visible(visible);
		pEntry->record.visible = visible;
		m_bDirty = true;
	}
	ImGui::Text("Mirrored pass: %s", object.Is_Mirrored() ? "YES" : "NO");
	if (ImGui::Button("Delete selected"))
	{
		const uint64_t deletedId = pEntry->record.placementId;
		if (Remove_Placement(deletedId))
			m_Status = "Deleted placement #" + std::to_string(deletedId);
	}
}

CMapTool::PLACED_ENTRY* CMapTool::Find_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	return iter == m_Placements.end() ? nullptr : &*iter;
}
```

### 5-12. 생성 DataFiles

`ACTIVE.maparea` 전체 내용은 다음과 같다.

```text
LOSTARK_MAP_AREA_SELECTION 1 "LV_LUT_HEARTRB_ED"
```

`LV_LUT_HEARTRB_ED.mapassets`는 generator가 260행 전체를 생성한다. header와 중앙 자산 행은 반드시 다음 계약을 만족한다.

```text
LOSTARK_MAP_ASSET_CATALOG 2 "LV_LUT_HEARTRB_ED" 260
"MAP_7D6EFEB99D88_BG_PVP_RETOWN_FLOOR01_SM" "bg_pvp_retown_floor01_sm" "Map/LV_LUT_HEARTRB_ED/MAP_7D6EFEB99D88_BG_PVP_RETOWN_FLOOR01_SM/MAP_7D6EFEB99D88_BG_PVP_RETOWN_FLOOR01_SM.wmodel" "Prototype_Component_Model_MAP_7D6EFEB99D88_BG_PVP_RETOWN_FLOOR01_SM" 1 1 1 Origin "pvp" "PVP_RETOWN_A" "UE3 ImportTable exact: pvp_retown_a.mesh.bg_pvp_retown_floor01_sm"
```

`LV_LUT_HEARTRB_ED.mapplacements`는 generator가 13,091행 전체를 생성한다. 중앙 골든 행은 다음과 같다.

```text
11534871182138487613 "LV_LUT_HEARTRB_ED_SL00:export:1234" "LV_LUT_HEARTRB_ED_SL00" "actor" "MAP_7D6EFEB99D88_BG_PVP_RETOWN_FLOOR01_SM" 8.49491821 9.75819458 -21.4094531 0 -0.707106781 0 0.707106781 0.490689129 0.225717008 0.513935432 1
```

13,091행을 계획서에 수동 복사하지 않는다. generator의 최종 G3 출력과
`maptool_scene_receipt_G3.json`의 SHA-256을 정본으로 삼아 사람이 만든 일부 행이
섞이는 것을 금지한다.

### 5-13. 문서 세 곳

`맵추출파이프라인.md`에는 다음 단계와 PASS 기준을 전부 기록한다.

```text
0. logical level -> physical UPK name resolve
1. Lost Ark AES/LZ4 package restore
2. Import/ExportTable exact asset discovery
3. raw glTF/material/texture export
4. WModel/WMat cook와 runtime install
5. Actor -> Component -> StaticMesh placement recovery
6. UE3 -> Client basis conversion과 placement v2 compile
7. MapTool parse -> validate -> stage -> commit
8. StaticMesh golden verification
9. hidden/collision/Volume 분류
10. DeployData ITR prop merge
11. static navigation bake
12. InterpActor/SkeletalMesh/Particle/Light/destruction 전환
```

`gotchas.md` 두 곳에는 최소 다음을 박제한다.

```text
- UModel glTF는 Y/Z swap과 cm->m를 이미 수행한다.
- 현재 converter가 glTF Z를 WModel -Z로 저장하므로 최종 basis는 (X,Z,-Y)다.
- cook 100과 Loader 0.01은 상쇄되며 placement position만 따로 다른 scale로 줄이면 안 된다.
- scale은 (Sx,Sz,Sy)로 reorder하고 부호를 보존한다.
- CTransform::Rotation은 Get_Scaled 때문에 음수 부호를 잃는다.
- 홀수 reflection은 RS_Cull_CW pass가 필요하다.
- non-uniform scale normal은 inverse-transpose가 필요하다.
- tangent는 world linear 변환 후 normal에 직교화하고 binormal handedness를 보존한다.
- exact placement는 BottomCenter가 아니라 Origin이다.
- ACTIVE area 변경은 Prototype 재등록을 위해 재시작이 필요하다.
- 13,091은 PS 포함 수치이고 SL00~SL05만은 12,949다.
- StaticMesh PASS는 collision/DeployData/navigation/dynamic actor PASS를 뜻하지 않는다.
```

## 6. 프로젝트 등록과 검증

### 6.1 `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj`

기존 `MapAssetCatalog` 항목 옆에 정확히 추가한다.

```xml
<ClInclude Include="..\Public\MapPlacementDocument.h" />
<ClCompile Include="..\Private\MapPlacementDocument.cpp" />
```

### 6.2 `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters`

기존 `03. Tools\00. Map` Filter를 재사용한다.

```xml
<ClInclude Include="..\Public\MapPlacementDocument.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
<ClCompile Include="..\Private\MapPlacementDocument.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
```

새 Filter나 GUID는 만들지 않는다. `build_maptool_scene.py`, data file, Markdown은 Visual Studio C++ project에 등록하지 않는다.

### 6.3 적용 순서와 검증

#### 6.3.1 구현 순서

1. `build_maptool_scene.py`와 unit test를 추가하고 basis 골든 테스트를 먼저 통과시킨다.
2. 260개 Catalog, 13,091개 placement v2, receipt를 생성한다.
3. `CMapPlacementDocument`를 추가하고 v1 legacy와 v2 exact parser/writer를 검증한다.
4. `CMapAssetCatalog`에 ACTIVE area 선택과 area별 경로를 연결한다.
5. `CMapAssetObject`에 quaternion/signed matrix를 적용한다.
6. shader에 inverse-transpose와 mirrored pass를 추가한다.
7. `CMapTool`을 v2 record/source layer/rollback 경로로 연결하고 Valtan 전용 assembly를 제거한다.
8. project XML을 등록한다.
9. 공통 pipeline/README/gotchas를 실제 구현과 함께 갱신한다.
10. 빌드 후 중앙 골든, 7개 layer, negative scale, save/reload, 실패 rollback을 순서대로 검증한다.

#### 6.3.2 생성 명령

```powershell
python Tools\LevelPlacementExtractor\test_build_maptool_scene.py

$common = @(
  'Tools\LevelPlacementExtractor\build_maptool_scene.py',
  '--area-id', 'LV_LUT_HEARTRB_ED',
  '--asset-manifest', 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\MapExtraction\LV_LUT_HEARTRB_ED\map_asset_manifest.json',
  '--runtime-manifest', 'C:\Users\user\Desktop\LostArk\Client\Bin\Resources\LostArk\Map\LV_LUT_HEARTRB_ED\map_asset_runtime_manifest.json',
  '--runtime-root', 'C:\Users\user\Desktop\LostArk\Client\Bin\Resources\LostArk\Map\LV_LUT_HEARTRB_ED',
  '--placements-dir', 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\MapExtraction\LV_LUT_HEARTRB_ED\placements',
  '--catalog-output', 'C:\Users\user\Desktop\LostArk\Client\Bin\DataFiles\Map\LV_LUT_HEARTRB_ED.mapassets',
  '--placement-output', 'C:\Users\user\Desktop\LostArk\Client\Bin\DataFiles\Map\LV_LUT_HEARTRB_ED.mapplacements',
  '--golden-placement-id', 'LV_LUT_HEARTRB_ED_SL00:export:1234',
  '--expect-assets', '260',
  '--expect-source-placements', '13091',
  '--expect-any-negative', '5042',
  '--expect-reflected', '4521',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_PS=142',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_SL00=682',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_SL01=3603',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_SL02=2094',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_SL03=2543',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_SL04=2324',
  '--expect-level-count', 'LV_LUT_HEARTRB_ED_SL05=1703'
)

# G1: 중앙 정확 배치 3개
python @common `
  --receipt-output C:\Users\user\Desktop\Resource_LostArk\05_Reports\MapExtraction\LV_LUT_HEARTRB_ED\placements\maptool_scene_receipt_G1.json `
  --include-source-id LV_LUT_HEARTRB_ED_SL00:export:1234 `
  --include-source-id LV_LUT_HEARTRB_ED_SL00:export:1235 `
  --include-source-id LV_LUT_HEARTRB_ED_SL00:export:1298 `
  --expect-output-placements 3 `
  --expect-output-any-negative 1 `
  --expect-output-reflected 1

# G2: SL00 682개
python @common `
  --receipt-output C:\Users\user\Desktop\Resource_LostArk\05_Reports\MapExtraction\LV_LUT_HEARTRB_ED\placements\maptool_scene_receipt_G2.json `
  --include-level LV_LUT_HEARTRB_ED_SL00 `
  --expect-output-placements 682 `
  --expect-output-any-negative 223 `
  --expect-output-reflected 203

# G3: 전체 13,091개. 이 결과만 최종 배포한다.
python @common `
  --receipt-output C:\Users\user\Desktop\Resource_LostArk\05_Reports\MapExtraction\LV_LUT_HEARTRB_ED\placements\maptool_scene_receipt_G3.json `
  --expect-output-placements 13091 `
  --expect-output-any-negative 5042 `
  --expect-output-reflected 4521
```

각 명령은 선택 범위와 무관하게 먼저 원본 13,091개 전체의 asset join, quaternion,
음수 축 5,042개와 reflection 4,521개를 검증한다. G1/G2가 PASS한 뒤에만 다음 명령을
실행하며, 최종 `.mapplacements`는 반드시 마지막 G3 출력이어야 한다.

#### 6.3.3 빌드 순서

Engine public header는 변경하지 않으므로 Engine 재배포는 필수가 아니지만 팀 표준 전체 검증은 다음 순서로 수행한다.

```text
1. Engine x64 Debug
2. Engine x64 Release
3. UpdateLib.bat Debug
4. UpdateLib.bat Release
5. Client x64 Debug
6. Client x64 Release
```

링크 시 `Client.exe`가 출력물을 점유하면 종료하고 다시 빌드한다.

#### 6.3.4 런타임 성공 기준

```text
ACTIVE area                         LV_LUT_HEARTRB_ED
Catalog Prototype                  260
Placement total                    13,091
PS / SL00..SL05                    142/682/3603/2094/2543/2324/1703
missing catalog FK                 0
duplicate runtime/source ID        0
invalid quaternion/scale           0
any-negative-axis placement        5,042
mirrored placement                 4,521
central source placement           SL00:export:1234
central Client position            8.494918, 9.758195, -21.409453
central Client quaternion          0, -0.707107, 0, 0.707107
central Client signed scale        0.490689, 0.225717, 0.513935
anchor                              Origin
legacy v1 read / v2 save            PASS / PASS
```

v1 호환은 기존 `BG_RAD_VALTAN_A.mapplacements`를 별도 테스트 복사본에서 읽고 v2로
저장한 뒤, placement 수·Client Euler에서 변환된 quaternion·asset FK가 유지되는지
확인한다. 원본 v1 파일은 덮어쓰지 않는다.

#### 6.3.5 실패 보존 검사

1. 정상 scene 13,091개를 로드한다.
2. 테스트 복사본 placement 한 행의 assetId를 없는 값으로 바꾼다.
3. Reload가 parse/validate 단계에서 실패하고 기존 13,091개가 남는지 확인한다.
4. Clone 중간 실패를 강제해 staged 객체만 제거되고 기존 vector와 Engine Layer가 유지되는지 확인한다.
5. 저장 destination을 잠가 atomic write가 실패해도 기존 placement 파일 hash가 유지되는지 확인한다.

#### 6.3.6 후속 완전 복원 gate

정적 PASS 후 다음 순서를 바꾸지 않는다.

1. Actor/Component의 hidden, collision, cast-shadow, material override property를 extractor에 추가한다.
2. Volume/BSP/Terrain을 render geometry와 collision geometry로 분리한다.
3. DeployData의 `ITR_023xx` prop를 source ID가 겹치지 않는 별도 layer로 합친다.
4. 정적 geometry와 collision이 확정된 뒤 navigation을 bake한다.
5. `InterpActor`의 이동/파괴 상태, SkeletalMesh, Particle, Light, Decal을 복구한다.
6. 파괴 전/후 asset과 encounter event를 연결한다.

이 후속 gate가 끝나기 전에는 “발탄 맵 전체 gameplay 복원 완료”라고 기록하지 않는다. 그러나 같은 `areaId -> Catalog -> placement/document -> layer` 계약을 사용하므로 아브렐슈드, 일리아칸, 카멘, 카오스 던전 시작 마을, 대륙·퀘스트 성벽도 exact ImportTable/ExportTable 근거로 반복할 수 있다.
