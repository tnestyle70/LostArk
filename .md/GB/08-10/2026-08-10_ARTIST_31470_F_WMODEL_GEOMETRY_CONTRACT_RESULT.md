# 2026-08-10 Artist 31470 F WModel Geometry Contract Result

## 결과

도화가 F 7개 Mesh carrier의 WModel 1.1 decoder/format 후보를 payload integrity 기준으로 닫았다.
후보는 모두 임시 디렉터리에서만 생성했으며 실제 `Client/Bin/Resources` WModel은 교체하지 않았다.

- geometry payload integrity: 7/7
- tangent handedness glTF payload preservation: 7/7
- negative source tangent-W가 있는 carrier: 4/7
- reflected bitangent maximum error: 0
- `COLOR_0` payload preservation: source에 존재하는 2/2
- embedded WModel-space bounds와 vertex-derived bounds 일치: 7/7
- source fidelity closed: 0/7
- runtime `geometryPreScale` consumer: 0/7
- `COLOR_0` shader consumer: 0/2
- Product admission: false

## 구현한 format과 reader

WModel/WMSH v1.1은 tangent W, optional RGBA8 `COLOR_0`, submesh bounds와 10개 SHA-256
identity를 저장한다. reader는 payload/metadata hash를 다시 계산하고 channel mask, stride,
triangle partition, index range, tangent W, bounds를 교차 검증한다.

legacy v1.0은 계속 읽지만 `geometryMetadata.present=false`, `hasColor0=false`,
`embeddedBounds.present=false`로 구분한다. optional COLOR가 없는 v1.1도 white를 source channel로
승격하지 않고 `hasColor0=false`를 전달한다.

Engine `CModel`은 verified format/channel/evidence/preScale/payload/provenance identity와 embedded
bounds를 보존한다. source fidelity와 Product admission policy getter는 두지 않았다. v1.1 reader는
현재 증명할 수 없는 clean export, exact UPK→glTF, pivot evidence bit 자체를 거부한다.

## geometry numeric oracle

source position은 `(x,y,-z) * 100`, winding은 reversed, tangent W는 `-sourceW`다.
7개 실제 carrier의 최종 pre-scaled oracle은 다음과 같다.

| 항목 | observed maximum | float32 계산 상한 |
|---|---:|---:|
| position after `geometryPreScale` | `5.784767154182191e-08` | `5.801767068621538e-08` |
| min/max AABB after `geometryPreScale` | `5.520439572137548e-08` | `5.801767068621538e-08` |

상한은 float32 cook half-ULP와 저장된 0.01이 exact reciprocal에서 벗어나는 오차를 합산했다.
asymmetric bounds fixture도 통과했으며 recenter는 적용하지 않았다. 다만 이 수치는 offline oracle이고
최종 pre-scaled bounds의 실제 runtime consumer가 연결됐다는 뜻은 아니다.

## provenance와 EOL/hash 역할

cooker는 source manifest와 export/cook receipt가 실제 glTF/bin, source package, legacy WModel,
converter 파일의 path/size/raw SHA-256과 일치해야만 후보를 만든다. 이것은 supplied glTF부터
candidate WModel까지의 payload integrity다. 반복 가능한 clean UPK export, UModel executable/command,
pivot provenance는 아직 미폐쇄다.
legacy converter의 현재 executable hash도 pin하지만 historical cook receipt에 converter hash가 없어
동일 executable이 당시 실행됐다는 사실은 별도 blocker로 유지한다.

geometry parity builder의 hash 역할은 다음처럼 분리했다.

- external/source/runtime glTF, bin, WModel, converter, cook script: raw byte SHA-256
- repo tracked derived inventory JSON: UTF-8 no-BOM, CRLF/lone-CR만 LF로 정규화한 SHA-256
- generated parity JSON `--check`: EOL만 정규화한 뒤 expected bytes와 exact 비교

따라서 LF/CRLF 차이만 허용하고 BOM, 숫자 `1`/`1.0`, field order, content mutation은 거부한다.

## 자동 검증

- geometry cooker/provenance/corrupt Python tests: 4/4 PASS
- parity EOL/raw role tests: 3/3 PASS
- WModelGeometryContractHarness Debug/Release: PASS
  - valid v1.1 `COLOR_0` 유/무
  - legacy v1.0 metadata absent
  - truncated, header/version, outer metadata/section gap, stride/material index,
    missing/unknown channel, tangent W,
    payload/metadata hash, mismatched/non-finite/inverted bounds, provenance,
    COLOR evidence mismatch, unverified source-fidelity claim 등 corrupt 20종 거부
- 7-carrier deep geometry audit: PASS
  - carrier 7, `COLOR_0` 2, negative-W carrier 4
  - integrity 7, source fidelity 0, preScale consumer 0, Product false
- Engine x64 Debug/Release: build PASS
- UpdateLib Debug/Release: PASS
- Client x64 Debug/Release: compile/link PASS
- parity receipt generator `--check`: PASS
- `git diff --check`: PASS, checkout EOL warning만 존재

`Invoke-ProjectAudit.ps1`는 실행했으며 exit 1이었다. 11개 보고 중 geometry 변경과 직접 연결된 것은
`effect.artist-31470-source-contract`의 stale downstream candidate 1개다. 나머지는 map runtime root,
project Data visibility, G09/WFX/authored rollout, actor resource 등 이 geometry diff 밖의 repository 또는
environment failure다. geometry focused audit는 Debug deep/Release 모두 PASS다.

이미지, 스크린샷, GPU 화면 판정은 수행하지 않았다.

## 남은 blocker와 handoff

- `GEOMETRY_RECEIPT_DOWNSTREAM_SOURCE_REGEN_REQUIRED`: Source Closure가 이 geometry commit을
  선행 parent로 받아 candidate/receipt/registry/header를 재생성해야 한다.
- actual Resource는 legacy v1.0이며 후보를 배포하지 않았다.
- `geometryPreScale=0.01`은 metadata/identity일 뿐 vertex, bounds staging, prepared-model cache에서
  소비하지 않는다.
- Playback의 기존 Mesh StartSize `×0.01` 상쇄가 남아 dimensionless size 계약이 적용되지 않았다.
- `COLOR_0`는 payload에 있으나 shader input에서 소비하지 않는다.
- clean source export, exact UPK→glTF, pivot provenance가 미폐쇄다.

다음 runtime 단위는 compiled GeometryBinding의
`{assetId, geometryPreScale, scaleSemantics, payloadHash, provenanceHash}` 전체를 cache identity에
포함하고 vertex/bounds에 preScale을 한 번만 적용해야 한다. 그 검증 전에는 Product admission을
열지 않는다.
