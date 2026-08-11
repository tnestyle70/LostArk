# 2026-08-10 Artist 31470 F WModel Geometry Contract Plan

## 목표

도화가 F `31470`의 7개 Mesh carrier를 전체 Effect runtime보다 먼저 닫기 위한
decoder/format 수직 슬라이스를 만든다. 기존 glTF와 legacy WModel을 근거로 WModel 1.1
후보를 생성하고, geometry channel과 증거 메타데이터가 기존
`WModelDecoder -> MODEL_ASSET_DATA -> CModel/CMesh` 경계를 통과하도록 한다.

이 단위가 증명하는 것은 candidate payload와 metadata가 서로 일치한다는
`SELF_CONSISTENT_UNAUTHENTICATED` 상태다. 신뢰 루트가 서명하거나 compiled
GeometryBinding의 expected tuple과 대조한 외부 인증, source fidelity, Product admission은
이 단위의 완료 조건이 아니다.

## 소유 범위

- `Tools/ModelAssetConverter`: legacy material/container를 보존하는 WModel 1.1 geometry cooker
- `Engine/Private/BinaryAsset/Winters`: version/channel/hash/bounds fail-closed reader
- `Engine/Public/BinaryAsset/ModelAssetData.h`: tangent-W/COLOR sidecar, embedded bounds, metadata identity
- `Engine/Public/Model.h`, `Engine/Private/Model.cpp`: self-consistent metadata와 aggregate local AABB 노출
- geometry 전용 Python/C++ harness, writer-independent immutable golden, ProjectAudit,
  geometry 전용 PLAN/RESULT

다음은 이 단위가 소유하지 않는다.

- `Effect_DocumentRenderer`, `Effect_Playback`, typed Cascade compiler, renderer family
- 실제 `Client/Bin/Resources` WModel recook/교체
- compiled GeometryBinding, prepared-model cache identity, preScale 적용
- `COLOR_0` shader input
- clean UPK 재추출, UModel 실행 identity/command, UPK→glTF와 pivot fidelity 확정
- Source Closure의 candidate/receipt/registry/header 재생성

## format과 ABI 계약

legacy WModel/WMSH v1.0은 계속 읽고 geometry metadata가 없음을 명시한다. 신규 static
WModel/WMSH v1.1은 다음을 요구한다.

- base channel: position, normal, UV0, tangent XYZ
- required channel: tangent handedness W
- optional channel: glTF에 실제 존재할 때만 normalized RGBA8 `COLOR_0`
- submesh별 WModel-space min/max/center/radius
- finite positive reciprocal scale pair
- payload와 metadata의 내부 SHA-256 일치
- source glTF/buffer, observed package/converter, source-object path, tool, receipt identity rows

기존 GPU `VTXMESH` ABI는 size 56과 position/normal/tangent/binormal/UV offset을 그대로
유지한다. tangent-W와 optional COLOR는 `MODEL_MESH_DATA` CPU sidecar에 보존한다. shader가
소비하기 전까지 GPU vertex stride나 input layout을 확장하지 않는다.

Engine public 상태는 format version, channel mask, evidence flags, `geometryPreScale`, payload
hash와 `metadataIdentity`를 노출한다. 이름과 getter는 이것이 외부 인증된 payload나 Product
admission이 아니라 self-consistent unauthenticated metadata임을 드러내야 한다.

downstream compiled GeometryBinding이 소유할 최소 인증 tuple은 다음과 같다.

`{assetId, geometryPreScale=0.01,
scaleSemantics=DIMENSIONLESS_AXIS_REORDER_ONLY, payloadHash, metadataIdentity}`

decoder가 metadata 내부 hash를 다시 계산한 상태는 위 tuple과 대조한 외부 인증이 아니다.
Engine/CModel은 계속 `SELF_CONSISTENT_UNAUTHENTICATED`만 노출하고, G08의 caller가 expected
tuple을 제공하기 전에는 `externallyAuthenticated`나 Product 상태를 만들지 않는다.

writer와 decoder가 같은 offset 오류를 공유하는 반례를 막기 위해 test 실행 중 cooker가
생성하지 않는 850-byte WModel 1.1 frozen hex와 canonical-LF expected manifest를 Git에 둔다.
C++ decoder는 decoded byte SHA, metadata payload/identity SHA, position/normal/tangentXYZ/W,
UV, COLOR, index, bounds를 고정 기대값과 대조한다. hex 한 nibble과 semantic은 유지하는
manifest whitespace mutation은 C++ hash gate가 exact exit 1로 거부한다. `decodedByteCount`
mutation은 별도 typed semantic validator와 C++ hash gate가 각각 거부한다.

## 좌표와 tangent basis

cook 관계는 position `(x,y,z) -> (x,y,-z) * 100`이고 triangle winding을 뒤집는다.
Z reflection 행렬의 determinant가 -1이므로 `cross(MN,MT) = -M cross(N,T)`다.
따라서 transformed bitangent를 유지하려면 runtime tangent W는 `-sourceW`여야 한다.
4개 negative-W carrier를 포함한 `cross(N,T)*W` numeric oracle로 이를 검증한다.

v1.1 normal/tangent는 finite, nonzero이고 cross가 nondegenerate여야 한다. zero normal,
zero tangent, parallel basis를 fallback 축으로 바꾸지 않고 거부한다. v1.0만 기존
SafeNormalize 호환 동작을 유지한다.

particle StartSize는 geometry 단위 변환이 아니라 signed dimensionless axis reorder다.
`geometryPreScale=0.01`은 향후 carrier geometry와 bounds에 한 번만 적용한다. 현재
Playback의 Mesh StartSize `×0.01` 상쇄는 제거되지 않았으므로 이 slice에서는 preScale
consumer와 Product admission을 열지 않는다.

## provenance 역할

- repo tracked Artist source manifest: UTF-8 no-BOM, EOL-only canonical LF hash
- external glTF/bin/WModel/tool/export/cook receipt: raw byte SHA-256
- source glTF/bin과 legacy WModel: 외부 receipt의 path/size/hash와 교차검증
- source package와 현재 converter: receipt에 byte size/SHA가 없으므로
  `OBSERVED_UNBOUND`
- source object: 문자열 path hash일 뿐 `PATH_HASH_UNAUTHENTICATED`
- legacy source-manifest raw hash: canonical LF와 canonical CRLF 두 checkout variant와만 상관

LF/CRLF만 다른 tracked manifest는 같은 candidate를 만들고, BOM과 semantic mutation은
실패해야 한다. observed-unbound package/converter byte mutation은 candidate identity를
바꾸지만 외부 인증이나 source fidelity로 승격되지 않는다.

## fail-closed와 transaction

다음 입력은 decode 실패다.

- truncated header, unknown outer/mesh/metadata version, section gap/trailing payload
- unknown/missing channel, wrong stride, out-of-range material/index
- zero/parallel basis, invalid tangent W
- nonfinite scale/bounds 또는 reciprocal 불일치
- bounds center/radius/distanceSquared/sqrt의 nonfinite intermediate
- payload/metadata digest mismatch, 빈 identity row
- COLOR channel/evidence/payload 불일치, 근거 없는 source-fidelity flag
- corrupt WMAT/WSKL/WANM

`CWModelDecoder::Decode`는 local `MODEL_ASSET_DATA`에 mesh/material/skeleton/animation을 모두
stage한 뒤에만 public output으로 move한다. 어느 후속 section에서든 실패하면 public output은
기본 빈 asset이다.

## 검증 순서

1. Python 6개 cooker/provenance/corruption/golden 테스트와 parity EOL/raw 3개 테스트
2. writer-independent 850-byte immutable golden과 C++ decoder harness Debug/Release
3. v1.0 static multi-submesh+hasBounds, skinned+skeleton+animation 회귀
4. exact SHA로 고정한 corrupt WModel 29종 + WSKL 1종 + WANM 1종의 의도된
   error category와 transaction 회귀. missing/rename/case-only rename/empty fixture는
   exact exit 1이며 실제 directory entry 이름 집합은 ordinal case-sensitive exact match
5. 실제 7개 candidate를 Python numeric oracle와 C++ decoder 양쪽에서 Debug/Release 검증
6. 실제 Resources v1.0 corpus 2,586개 C++ sweep: static 2,535, skinned 51,
   hasBounds 2,586, multi-submesh 665, legacy sidecar/geometry metadata 0
7. Engine, UpdateLib, Client Debug/Release
8. focused ProjectAudit 등록/실행, `git diff --check`, 소유 경계 확인

실제 Resources와 Effect runtime diff는 0이어야 한다. geometry commit 뒤 Source Closure가
candidate/receipt/registry/header를 재생성해야 하므로
`GEOMETRY_RECEIPT_DOWNSTREAM_SOURCE_REGEN_REQUIRED`를 handoff blocker로 유지한다.
