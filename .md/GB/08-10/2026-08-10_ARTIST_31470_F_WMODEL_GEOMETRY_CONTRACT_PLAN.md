# 2026-08-10 Artist 31470 F WModel Geometry Contract Plan

## 목표

도화가 F `31470`의 7개 Mesh carrier를 전체 Effect runtime보다 먼저 닫기 위한
decoder/format 수직 슬라이스다. 기존 glTF와 legacy WModel을 근거로 versioned WModel 1.1
후보를 만들고, geometry channel과 provenance payload가 Engine의 기존
`WModelDecoder -> MODEL_ASSET_DATA -> CModel/CMesh` 경계를 통과하도록 한다.

이 단위의 완료는 payload integrity 완료다. source fidelity와 Product admission 완료가 아니다.

## 소유 범위

- `Tools/ModelAssetConverter`: legacy WModel container/material을 보존하는 WModel 1.1 geometry cooker
- `Engine/Private/BinaryAsset/Winters`: version/channel/hash/bounds fail-closed reader
- `Engine/Public/BinaryAsset/ModelAssetData.h`: optional channel, embedded bounds, immutable metadata
- `Engine/Public/Model.h`, `Engine/Private/Model.cpp`: verified identity와 embedded bounds 전달
- geometry 전용 Python/C++ harness, ProjectAudit, parity receipt EOL/hash 계약

다음은 소유하지 않는다.

- `Effect_DocumentRenderer`, `Effect_Playback`, typed Cascade compiler, renderer family
- 실제 `Client/Bin/Resources` WModel 교체 또는 recook 배포
- prepared-model cache와 compiled GeometryBinding
- `COLOR_0` shader input
- source package 재추출 또는 clean UPK→UModel/glTF/pivot fidelity 승인

## 고정 데이터 계약

WModel/WMSH legacy v1.0은 계속 읽되 geometry metadata가 없음을 명시적으로 반환한다.
신규 static WModel/WMSH v1.1은 다음을 요구한다.

- base channel: position, normal, UV0, tangent XYZ
- required channel: tangent handedness W
- optional channel: `COLOR_0` normalized RGBA8. 부재는 `hasColor0=false`로 전달한다.
- submesh별 WModel-space min/max/center/radius
- `sourceToWModelScale=100`, `geometryPreScale=0.01`
- payload, source glTF, source buffer set, source package, source object, legacy converter,
  geometry tool, export receipt, legacy cook receipt, metadata SHA-256
- channel mask와 preservation/integrity evidence flags

Engine이 노출하는 immutable identity는 format version, channel mask, evidence flags,
`geometryPreScale`, payload hash, provenance hash다. Engine은 Product admission getter나 항상 false인
policy placeholder를 소유하지 않는다.

downstream compiled GeometryBinding은 별도 runtime 단위에서 최소 다음을 결합해야 한다.

`{assetId, geometryPreScale=0.01,
scaleSemantics=DIMENSIONLESS_AXIS_REORDER_ONLY, payloadHash, provenanceHash}`

## 좌표와 tangent basis

기존 cook 관계는 position `(x,y,z) -> (x,y,-z) * 100`이고 triangle winding을 뒤집는다.
Z reflection 행렬의 determinant는 -1이므로
`cross(MN,MT) = -M cross(N,T)`다. 따라서 transformed bitangent를 유지하려면
runtime tangent W는 `-sourceW`여야 한다. 단순 channel 복사가 아니라
`cross(N,T)*W` source/runtime numeric oracle로 판정한다.

particle StartSize는 geometry 단위 변환이 아니다. 목표 의미는 signed dimensionless axis reorder다.
`geometryPreScale=0.01`을 carrier geometry에만 적용해야 하며 Sprite/Decal 단위 변환이나 전역
model scale과 합치지 않는다.

## 구현 및 검증 순서

1. glTF, buffer, package, converter, export/cook receipt의 실제 파일 identity를 상호 검증한다.
2. legacy WModel topology가 source glTF와 동일 revision인지 확인한다.
3. WModel 1.1 vertex/index/bounds/metadata를 임시 경로에 생성한다.
4. 7개 carrier를 position/normal/tangent XYZ/W/UV/topology/winding/COLOR/bounds oracle로 검증한다.
5. C++ decoder harness에서 valid v1.1, legacy v1.0, corrupt input을 통과·거부시킨다.
6. Engine/UpdateLib/Client Debug/Release를 빌드한다.
7. 실제 Resources와 Effect runtime diff가 0인지 확인한다.

float32 scale 오차는 임의 tolerance로 완화하지 않는다. cook 시 position half-ULP와 저장된
`geometryPreScale`의 exact reciprocal 오차를 합산해 좌표별 상한을 계산한다.

## fail-closed 조건

다음은 모두 decode 실패다.

- truncated payload, outer/inner/metadata unknown version, non-canonical outer metadata
- non-contiguous/empty WModel section payload, out-of-range material index
- unsupported/missing channel mask, wrong stride, invalid tangent W
- payload 또는 metadata hash mismatch, 비어 있는 provenance digest
- non-finite, inverted, vertex-derived 값과 다른 bounds
- `COLOR_0` channel과 evidence flag 불일치
- v1.1이 clean export, exact UPK→glTF, pivot fidelity를 자기 metadata만으로 주장하는 경우

## admission과 integration 경계

임시 후보의 payload integrity가 7/7이어도 Product는 false다. 실제 Resource가 legacy v1.0이고,
preScale/cache/Playback consumer, `COLOR_0` shader, pivot/source fidelity가 닫히지 않았기 때문이다.

parity receipt의 tracked input hash는 checkout EOL과 무관한 canonical-LF를 사용한다. 이 변경으로
기존 downstream Source Contract output은 stale이 된다. geometry commit에는 그 output을 넣지 않고
`GEOMETRY_RECEIPT_DOWNSTREAM_SOURCE_REGEN_REQUIRED`로 남긴다. integration 순서는
geometry commit 다음 Source Closure commit이다.
