# 2026-08-10 Artist 31470 F Material Runtime Oracle Result

## 결론

Material R0 corrective의 evidence integrity는 PASS했지만 execution readiness는 BLOCK이다.

- render-state omitted: `0/89 READY`, `89 BLOCKED`
- static permutation: `0/94 READY`, `94 BLOCKED`
- direct-unproven sampler: `0/68 READY`, `68 BLOCKED`
- 합계: `0/251 READY`, `251 BLOCKED`, ownerless 0, unknown decision 0
- evidence-integrity admission: true
- execution-readiness admission: false
- runtime handler/renderer consumption: false
- Product admission: false
- format version: 2
- receipt canonical SHA-256: `4c80a0d8d25712b7449c726d19d9af87ad2831b5c78231fdc42145a9c17dc4d8`

251개 행은 모두 stable owner와 조사 경로를 가지므로 “누가 조사할지 모르는 상태”는 제거됐다. 그러나
source-specific 값을 정하는 독립 provider와 actual output pilot은 아직 없다. 따라서 이 receipt는
final materializer, Playback 또는 renderer의 실행 입력으로 승격할 수 없다.

기존 arithmetic evaluator는 23 family와 27 recipe의 재구성 수치 동작을 CPU와 WARP에서 검증한다.
이는 원본 UE3 graph, omitted render state, static instance selection 또는 sampler state의 source-exact
oracle이 아니다. `SOURCE_EXACT` family/evaluator는 계속 0이다.

## R0 feasibility matrix

### Render-state 89

각 행은 instance → parent → nested default → CDO → ShaderCache → controlled runtime capture를 별도
identity/availability/outcome으로 기록한다. D3D11 WARP state-object provider는 blend, depth,
rasterizer/cull, sampler address 네 descriptor pilot을 실제 생성하고 `GetDesc` 결과를 비교해 PASS했다.
다만 이 결과는 descriptor consumer의 의미만 증명하며 Artist F source-era descriptor 값을 정하지 않는다.
그 값의 provider가 없으므로 89행 모두 `BLOCKED`다.

### Static permutation 94

94행은 ordered `sourceSection/sourceSectionIndex/sourceOwnerRecipeId`와 parent/default evidence를 보존한다.
parent default가 있더라도 instance selection evidence, source ShaderCache join 또는 통제된 source runtime
capture가 없으면 선택된 permutation으로 승격하지 않는다. 현재 94행 모두 `BLOCKED`다.

### Direct-unproven sampler 68

texture path와 recipe ownership은 exact하지만 sampler descriptor provenance는 별개다. texture identity나
현재 revision sampler를 source-era sampler 값으로 세탁하지 않는다. 68행 모두 `BLOCKED`다.

## Evidence-integrity corrective

- occurrence recipe ↔ binding/evaluator를 exact mapping으로 검증한다.
- raw expression에서 feature mask와 graph evidence projection을 독립 재도출한다.
- 729 input과 94 static row의 source section/index/owner 순서를 source receipt에서 다시 만든다.
- receipt 내부 self hash뿐 아니라 Material/render/ShaderCache receipt, HLSL, generator, verifier와 dependency의
  실제 tracked canonical hash를 검증한다.
- local source archive는 1,813 physical / 624 unique / 1,189 duplicate와 inventory projection
  `60922d43d423006e9a7868bbf5eef8cd68bddeeeef38113098822cc30c7fbbec` 경계를 고정한다.
- JSON과 actual WARP output에서 `NaN`, `+Inf`, `-Inf`를 fail-closed로 거부한다.
- 세 matrix의 denominator shrink, owner 삭제, cross-recipe coordinated reseal, archive projection 세탁과
  Product/readiness admission 개방을 mutation test로 거부한다.
- render receipt root identity 또는 ShaderCache denominator를 바꾸고 downstream source hash와 matrix를 함께
  reseal해도 shared shallow 인증 경로가 상위 validator와 Material contract pin으로 거부한다.

## Numeric evaluator와 WARP provider

23 arithmetic family는 명시적인 versioned reconstruction evaluator다. family 92개와 recipe binding
108개, 합계 200개 sample을 CPU golden과 별도 HLSL compute implementation으로 실행했다. pinned
Windows SDK `d3dcompiler_47.dll` SHA-256은
`ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8`이다.

D3D11 WARP actual dispatch의 최대 절대 오차는 `1.1444091796875e-05`, 허용오차는 `2.0e-05`였다.
state provider backend는 `D3D11_WARP_STATE_OBJECTS`, pilot count는 4이며 descriptor tolerance는 0이다.
이 두 oracle은 reconstruction evaluator와 provider implementation을 검증할 뿐, 251개 source-specific
값의 acquisition을 대신하지 않는다.

## Source-revision acquisition

- installed cache: shader object 271, `FMaterialShaderMap` 25, shader reference 534
- Artist base Material join `0/23`
- Artist MIC static parameter set join `0/24`
- local source archive: 1,813 UPK / 624 unique package / ShaderCache 또는 `sc_lv_*` candidate 0
- controlled source-revision runtime capture path: unavailable

결정은 `SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE`다. 설치 게임 process를 임의
주입하거나 current revision 값을 source revision으로 승격하지 않았다.

## 검증

- `python -B Tools/LevelPlacementExtractor/test_build_artist_31470_material_runtime_oracle.py`
  - 16/16 PASS
- shallow receipt check
  - family 23, recipe 27, occurrence 34, feasibility `0/251`, Product false PASS
- independent WARP verifier
  - HLSL numeric sample 200, state-object pilot 4 PASS
- focused ProjectAudit shallow/deep
  - source archive 1,813/624 재계산, checked receipt 재현, `0/89 + 0/94 + 0/68` PASS
- JSON parse와 `git diff --check` PASS

전체 `Invoke-ProjectAudit.ps1`에서는 이 corrective가 소유한
`effect.artist-31470-material-runtime-oracle` 항목이 실패 목록에 없었다. 전체 결과는 다음 외부 baseline
12항목 때문에 exit 1이었다.

- `maps.extracted-area-runtime-roots`
- `maps.character-select-area-contract`
- `projects.data-source-visibility`
- `effect.g09-authoring-world-runtime-boundary`
- `effect.g09-cross-document-contract`
- `effect.artist-31470-source-contract`의 이 base에 남은 publisher v14 구 assertion
- `effect.artist-31470-wmodel-geometry-contract` harness binary 부재
- `effect.wfx-component-assembly`
- `effect.representative-authored-readiness`
- `effect.four-class-authored-clip-product-exact101`
- `actors.catalog-assets`
- `actors.dimensionmaster-runtime-animation`

이 외부 실패를 Material corrective의 PASS로 세탁하지 않았고, Material 전용 shallow/deep audit은 별도로
PASS했다. 이미지 캡처, 육안 비교와 이미지 기반 자동 검증은 수행하지 않았다.

## 다음 경계

R0 execution-readiness는 BLOCK이다. Source-specific provider와 actual output pilot을 확보해 251개 행을
모두 `FEASIBLE` 또는 독립 증거가 있는 `VERIFIED_IRRELEVANT`로 닫기 전에는 final materializer/schema,
Playback, geometry/material consumer와 six renderer 작업을 시작하지 않는다. 해결 경로가 실제로 없는
행은 값을 추측하지 않고 복원 범위의 hard blocker로 유지한다.
