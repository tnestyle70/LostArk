# 2026-08-10 Artist 31470 F WModel Geometry Contract Result

## 결과

도화가 F 7개 Mesh carrier의 WModel 1.1 decoder/format 후보를
`SELF_CONSISTENT_UNAUTHENTICATED` 단계로 닫았다. 후보는 모두 임시 디렉터리에서만
생성했으며 실제 `Client/Bin/Resources` WModel은 교체하지 않았다.

- payload/metadata self-consistency: 7/7
- externally authenticated payload integrity: 0/7
- tangent-W glTF payload preservation: 7/7
- negative source tangent-W carrier: 4/7
- reflected bitangent maximum error: 0
- `COLOR_0` payload preservation: source에 존재하는 2/2
- embedded WModel-space bounds와 vertex-derived bounds 일치: 7/7
- source fidelity closed: 0/7
- runtime `geometryPreScale` consumer: 0/7
- `COLOR_0` shader consumer: 0/2
- Product admission: false

## format, ABI, reader

WModel/WMSH v1.1은 tangent W, optional RGBA8 `COLOR_0`, submesh별 bounds와 10개 identity
row를 저장한다. reader는 payload와 metadata hash를 다시 계산하고 version, section range,
channel mask, stride, triangle partition, index, tangent W, basis, bounds를 검증한다. 이 검증은
외부 신뢰 루트와 대조한 authentication이 아니라 컨테이너 내부 self-consistency다.

기존 GPU `VTXMESH`는 size 56과 offset `0/12/24/36/48`을 static_assert로 고정했다.
tangent-W와 COLOR는 `MODEL_MESH_DATA::tangentHandedness`와 `color0Rgba8` CPU sidecar에
보존한다. 기존 shader/input layout은 바꾸지 않았다.

legacy v1.0은 metadata와 sidecar가 비어 있음을 유지한다. 실제 C++ decoder에서 static
multi-submesh+hasBounds와 skinned+WSKL+WANM fixture를 통과시켜 legacy ABI 회귀를 확인했다.

`CWModelDecoder::Decode`는 local asset에 모든 section을 stage한다. corrupt material,
skeleton, animation에서 실패했을 때 public `MODEL_ASSET_DATA`가 기본 빈 상태임을 C++로
검증했다.

## bounds와 CModel 경계

decoder의 `MODEL_ASSET_DATA`는 submesh별 embedded bounds row를 보존한다. `CModel`은 이
row를 별도 public collection으로 보존하지 않고, `Ready_Meshes`에서 기존 pre-transform을
적용해 aggregate local AABB만 만든다. `geometryPreScale`은 metadata로만 노출되며 vertex,
bounds staging 또는 cache identity에 아직 적용되지 않는다.

따라서 final pre-scaled bounds는 offline oracle일 뿐 실제 runtime consumer 완료 증거가
아니다. Playback Mesh StartSize `×0.01` 상쇄도 그대로 남아 있다.

## numeric oracle

source position은 `(x,y,-z) * 100`, winding은 reversed, tangent W는 `-sourceW`다.
7개 실제 carrier에서 확인한 최종 pre-scaled float32 오차는 다음과 같다.

| 항목 | observed maximum | float32 계산 상한 |
|---|---:|---:|
| position after `geometryPreScale` | `5.784767154182191e-08` | `5.801767068621538e-08` |
| min/max AABB after `geometryPreScale` | `5.520439572137548e-08` | `5.801767068621538e-08` |

상한은 cook position의 half-ULP와 저장된 float32 `0.01`이 exact reciprocal에서 벗어나는
오차를 합산한다. 단순 `1e-7` 완화가 아니다. Artist oracle은 일반 reciprocal 검증과 별도로
`sourceToWModelScale=100`, `geometryPreScale≈float32(0.01)`을 고정한다.

reader와 sibling parser는 reciprocal을 곱셈으로 확인하지 않고 `preScale`과
`1/sourceScale`을 비교한다. re-signed `1e20/1e20`, `100/0.02`, zero/parallel basis,
finite bounds지만 squared-distance가 overflow하는 fixture를 모두 거부했다.

## provenance와 EOL/hash 역할

source manifest, export receipt, cook receipt의 schema/version을 먼저 검증한다.

- tracked Artist source manifest: canonical LF SHA-256, BOM 거부
- external glTF/bin/WModel/tool/export/cook receipt: raw byte SHA-256
- legacy manifest raw hash: canonical LF 또는 canonical CRLF variant와만 상관
- source package/current converter: receipt-bound size/hash가 없어 `OBSERVED_UNBOUND`
- source object: `PATH_HASH_UNAUTHENTICATED`

tracked manifest가 LF/CRLF만 다르면 provenance output과 candidate bytes가 같았다. BOM,
semantic/schema/version mutation은 실패했다. glTF byte와 bufferView length mutation은 raw
identity/range 검증에서 실패했다. package/converter byte mutation은 candidate identity를
바꾸지만 status는 계속 `OBSERVED_UNBOUND`이고 외부 인증 수는 0이다.

## 자동 검증

- geometry cooker/provenance/corrupt/frozen-golden Python tests: 6/6 PASS
- parity EOL/raw role tests: 3/3 PASS
- WModelGeometryContractHarness Debug/Release: PASS
  - writer-independent frozen WModel v1.1 golden 850 bytes와 canonical manifest
  - golden byte mutation과 semantic manifest mutation 거부
  - valid v1.1 COLOR/no-COLOR sidecar
  - legacy v1.0 static multi-submesh+hasBounds, skinned+WSKL+WANM
  - exact name/size/SHA의 corrupt WModel 29종 + WSKL 1종 + WANM 1종 거부
  - 각 corrupt fixture의 valid baseline delta와 의도된 error category 일치
  - WModel/WSKL/WANM 실패에서 transactional empty output
  - missing/renamed/case-only renamed/empty required fixture는 각각 exact exit 1
  - NTFS case-insensitive path resolution 전 actual directory entry 31개를 ordinal
    case-sensitive exact manifest set과 비교
  - manifest whole-file hash mutation과 semantic field mutation을 별도 거부
- 7-carrier deep audit Debug/Release: PASS
  - carrier 7, COLOR 2, negative-W carrier 4
  - actual C++ WModelDecoder의 channel/index/submesh bounds/payload identity oracle 7/7
  - self-consistency 7, externally authenticated 0, source fidelity 0,
    preScale consumer 0, Product false
- 실제 legacy WModel v1.0 C++ corpus sweep Debug/Release: PASS
  - files 2,586, static 2,535, skinned 51
  - hasBounds 2,586, multi-submesh 665, v1.1 metadata/sidecar absent 2,586
- focused `effect.artist-31470-wmodel-geometry-contract` ProjectAudit 등록
- Engine + WModelGeometryContractHarness x64 Debug/Release: build PASS
- UpdateLib Debug/Release: PASS
- Client x64 Debug/Release: compile/link PASS
  - corrective follow-up은 harness/test/docs만 바꾸므로 frozen `228429b`의 full
    Engine/UpdateLib/Client 증거를 계승했다. WModel harness와 focused audit만 Debug/Release로
    재실행했으며 독립 reviewer가 요구하면 integration에서 full build를 반복한다.
- full `Invoke-ProjectAudit.ps1`: exit 1, 8 unrelated/pre-existing checks FAIL
  - geometry focused check는 PASS였고 failure 목록에서 제외됨
  - project data visibility, G09 2개, Artist source/material, WFX assembly,
    representative readiness, four-class rollout checks가 실패함
  - `effect.artist-31470-source-contract` stale candidate는 geometry 이후 Source Closure
    재생성이 필요한 예상 handoff 상태이며 이 G02 변경에서 수정하지 않음
- `git diff --check`: 최종 corrective snapshot에서 확인

이미지, 스크린샷, GPU 육안 검증은 수행하지 않았다.

## 남은 blocker와 handoff

- `GEOMETRY_RECEIPT_DOWNSTREAM_SOURCE_REGEN_REQUIRED`: Source Closure가 geometry commit을
  선행 parent로 받아 candidate/receipt/registry/header를 재생성해야 한다.
- 실제 Resources는 legacy v1.0이며 WModel 1.1 후보를 배포하지 않았다.
- compiled GeometryBinding expected tuple과 payload/metadata identity authentication이 없다.
- `geometryPreScale=0.01`을 vertex/bounds/cache가 소비하지 않는다.
- Playback의 기존 Mesh StartSize `×0.01` 상쇄가 남아 있다.
- `COLOR_0`은 CPU sidecar만 있고 shader consumer가 없다.
- clean source export, exact UPK→glTF, pivot, UModel executable/command provenance가 미폐쇄다.

이 blocker가 닫히기 전까지 geometry Product admission은 false다.

## G02 evidence final 판정

이 변경 단위에서 초기 geometry evidence P2 세 항목과 후속 corrupt-fixture 증명 경계를
자동 계약으로 닫았다.

- provenance JSON `formatVersion`은 exact JSON integer만 허용하며 bool, float, string을 거부한다.
- Python cooker/parser와 C++ decoder는 tangent-W에 동일한 `1e-6` absolute contract를 쓴다.
- writer layout 함수로 만들지 않은 immutable hex golden과 expected manifest를 C++ decoder가 직접
  검증하고, byte/semantic mutation을 모두 거부한다.
- 실제 7 carrier는 Debug/Release C++ decoder에서 full semantic oracle을 통과했다.
- 실제 Resources legacy v1.0 2,586개는 Debug/Release C++ corpus sweep을 통과했다.
- 모든 decoder section은 local staged asset에만 기록되고 전체 성공 뒤 public output에 commit된다.
- payload/metadata 내부 self-consistency와 외부 expected tuple authentication은 별도 상태다.
- corrupt 증거는 exact 31-file manifest, valid baseline delta, decoder error category,
  transactional rollback을 각각 독립 bit로 요구하며 missing-file 실패를 corruption PASS로 세지 않는다.

이 판정은 WModel evidence/format/decoder/cooker gate 완료를 뜻한다. source fidelity, runtime
`geometryPreScale` 소비, GeometryBinding 외부 authentication, Resources 배포 또는 Product 승인으로
승격하지 않는다. `Client/Bin/Resources`와 Effect runtime은 변경하지 않았고 Product는 false다.

## Delta-2 독립 검토 경계

`8d806bbb` consolidated review는 P0 0, P2 0이고 NTFS case-only rename P1 한 건만 남겼다.
Delta-2는 actual directory entry의 ordinal exact-set gate와
`corrupt_header.wmodel -> corrupt_Header.wmodel` exact-exit-1 fixture를 추가했다. Debug/Release
harness와 focused ProjectAudit은 통과했으며, 최종 독립 재감사는 이 case-only delta만 남아 있다.
Product와 source-fidelity admission은 계속 false다.
