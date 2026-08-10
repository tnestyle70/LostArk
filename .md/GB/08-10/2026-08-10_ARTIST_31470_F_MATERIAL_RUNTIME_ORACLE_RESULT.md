# 2026-08-10 Artist 31470 F Material Runtime Oracle Result

## 결론

도화가 F Material의 source-revision ShaderCache 취득 경로는 현재 로컬 증거 범위에서 닫히지 않았다.
대신 23 arithmetic family를 모두 명시적인 reconstruction evaluator로 구현하고, CPU와 D3D11 WARP
HLSL에서 family 92개와 recipe binding 108개, 합계 200개 numeric sample을 독립 실행해
`RECONSTRUCTED_NUMERICALLY_VERIFIED`로 닫았다. 이 판정은 원본 UE3 graph와 같다는 뜻이 아니며
`SOURCE_EXACT` family/evaluator는 0이다.

27 Material recipe와 34 rendered occurrence에는 source evidence에서 다시 검증한 typed binding을 만들었다.
scalar/vector/texture 729개, static switch 94개, render-state 162개가 recipe 소유권과 source lineage에
결합된다. render-state 162개 중 explicit 73개만 실행값을 가지며 omitted 89개는 blocker를 유지한다.

공통 `CModel -> CMaterial`/Effect shader handler가 이 evaluator receipt를 아직 소비하지 않으므로
runtime handler consumption, renderer consumption과 Product admission은 모두 false다. Tools의 compute
shader는 numeric 검증 전용이고 두 번째 제품 Material runtime이 아니다.

## Source-revision ShaderCache 취득 판정

기준 ShaderCache evidence는 다음과 같다.

- installed cache: shader object 271, `FMaterialShaderMap` 25, shader reference 534
- Artist base Material join `0/23`
- Artist MIC static parameter set join `0/24`

추가로 `C:/Users/user/Desktop/Resource_LostArk/00_SourcePackages`의 UPK 1,813개를 raw SHA-256으로
중복 제거해 624 unique package를 만들고, UE3 summary와 AES/LZ4 NameTable 구간을 decode했다.
`ShaderCache` 또는 `sc_lv_*` name 후보는 0개였다. 전체 inventory projection SHA-256은
`60922d43d423006e9a7868bbf5eef8cd68bddeeeef38113098822cc30c7fbbec`이다.

따라서 receipt의 결정은
`SOURCE_REVISION_SHADER_CACHE_NOT_PRESENT_IN_SCANNED_ARCHIVE`다. source-revision 실행 파일에 대한
통제된 instrumentation 경로도 현 workspace에는 없으며, 설치 게임 process를 임의 주입하거나
현재 revision 값을 source revision으로 승격하지 않았다.

## Numeric evaluator

surviving raw expression 925개는 scalar parameter 518, static switch parameter 270,
texture parameter 112, vector parameter 24 등 parameter/default 증거만 보존한다. Add, Multiply, Panner
같은 historical edge는 cooked stripping으로 사라져 있어 복원할 수 없다. evaluator는 parameter name과
texture presence에서 다음 feature만 활성화하는 version 1 reconstruction이다.

- second texture multiply
- UV transform phase
- panner phase
- color multiply
- desaturation
- signed power
- fresnel gain
- distortion offset
- dissolve alpha
- alpha multiply

family마다 네 개의 고정 sample을 사용해 92개 CPU golden을 만들었다. 또한 각 recipe의 실제 typed
scalar/vector/static binding에서 UV, pan, color, gain, power, alpha, dissolve, fresnel, distortion operand를
결정하고 네 개의 texture probe/time sample을 적용해 108개 recipe golden을 만들었다.

동일 operation order를 별도 HLSL compute implementation으로 작성하고 pinned Windows SDK
`d3dcompiler_47.dll` SHA-256
`ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8`로 `cs_5_0` compile 후
D3D11 WARP에서 실제 dispatch했다. 200개 결과의 최대 절대 오차는
`1.1444091796875e-05`, 허용오차는 `2.0e-05`였다.

## Typed runtime binding

receipt의 각 recipe는 다음을 가진다.

- source recipe composition SHA와 23 family evaluator handle/version
- 729 exact scalar/vector/texture input의 typed value, semantic role, field ID, lineage SHA
- 94 static switch의 typed value와 recipe feature-mask decision
- 실제 source binding에서 파생한 runtime operand lanes와 108개 numeric sample
- six-field render-state status와 explicit value 또는 unresolved blocker
- source recipe에서 다시 계산한 binding SHA

각 occurrence는 source occurrence identity, cue, renderer family, recipe ID와 binding SHA를 보존한다.
validator는 self digest만 검사하지 않고 checked-in typed Material contract의 family/recipe/field/static/render/
occurrence row에 다시 결합한다. coordinated typed-value reseal, render projection reseal, feature-mask mutation,
ShaderCache 후보 세탁과 Product/runtime admission 개방 반례를 거부한다.

## 검증

실행 결과는 다음과 같다.

- `python -B Tools/LevelPlacementExtractor/test_build_artist_31470_material_runtime_oracle.py -v`
  - 9/9 PASS
- `powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470MaterialRuntimeOracle.ps1`
  - shallow PASS
  - WARP 200 sample PASS
- 같은 audit에 `-DeepMaterialRuntimeAudit -SourceArchiveRoot C:/Users/user/Desktop/Resource_LostArk/00_SourcePackages`
  - 1,813/624 source archive 재계산 PASS
  - checked receipt 재현 PASS
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
  - 새 `effect.artist-31470-material-runtime-oracle` 항목 PASS
  - 전체 결과는 이 base에 이미 존재하는 source publisher 문구 불일치, 미배포 Resources,
    geometry harness binary 부재 등을 포함한 12개 타 영역 항목 때문에 exit 1
- Material family 23, recipe 27, occurrence 34
- typed input 729, static 94, render state 73 explicit / 89 unresolved
- CPU/HLSL verified family 23/23, source-exact evaluator 0
- runtime consumed recipe/occurrence 0/0, Product recipe/occurrence 0/0

이미지 캡처, 육안 비교와 이미지 기반 자동 검증은 수행하지 않았다.

## 남은 통합 경계

이 slice 다음에는 새 shader/runtime를 만들지 않고 기존 Effect source Material profile과 공통 shader path가
receipt의 evaluator version, typed operand, texture/sampler, static switch와 render state를 immutable compiled
IR로 소비해야 한다. 실제 27/34 handler consumption, 여섯 renderer family의 동일 prepared IR 소비,
runtime texture provisioning과 omitted render-state default closure가 숫자 하네스로 닫힌 뒤에만 Product
admission을 다시 계산할 수 있다.
