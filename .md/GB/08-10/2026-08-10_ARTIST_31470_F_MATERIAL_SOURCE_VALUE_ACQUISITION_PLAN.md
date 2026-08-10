# 2026-08-10 Artist 31470 F Material Source Value Acquisition Plan

## 목표

frozen Material corrective `627ddc76ef58e45f35821363c93197157da4cf89`에서 Product나 runtime을
수정하지 않고, R0 Material blocker의 실제 source-value 확보 가능성을 다시 판정한다. 기존 정본은
render-state `89`, static permutation `94`, direct-unproven sampler `68`을 전부 BLOCK으로 두지만,
이번 단계는 다음 두 오류 가능성을 별도로 검사한다.

1. source MIC native tail에 이미 존재하는 `FStaticParameterSet`을 instance selection 부재로 잘못
   분류했는가
2. `SOURCE_EXACT_SAMPLER`로 승인된 4개가 실제로는 Texture2D class default를 추측했는가

값 provenance와 execution readiness를 분리한다. source 값 일부를 획득해도 D3D/WARP actual-output
pilot이나 final consumer가 없으면 execution-ready로 승격하지 않는다.

## 입력과 금지 경계

- 입력 정본
  - typed Material evidence contract
  - raw render-state evidence receipt
  - ShaderCache/native-tail oracle receipt
  - runtime feasibility matrix builder의 stable row identity derivation
  - `Effect_DIMENSIONMASTER_20260803_v3/source_pack_manifest.json`과 해당 raw UPK
- read-only 후보
  - local source archive 전체
  - 현재 설치 `Engine/Core`, ShaderCache, Texture2D package
  - Git reachable/unreachable/LFS, 로컬 driver shader cache, remote Actions/Release
- 금지
  - source default 추측
  - current/cross-revision 값을 source-era exact로 승격
  - anti-cheat 우회, process injection, 비인가 runtime capture
  - shared Material contract, materializer, Playback, Renderer, C++ 수정
  - Product 또는 R2 admission

## 구현

### `build_artist_31470_material_source_value_acquisition.py`

generator는 다음을 raw artifact에서 다시 계산한다.

1. static 94행의 parent expression `ParameterName/DefaultValue/ExpressionGUID`를 source UPK에서
   재파싱한다.
2. 같은 recipe의 MIC serial/native-tail을 재파싱하고 `FStaticParameterSet`의 raw value,
   `bOverride`, GUID, byte offset, native-tail SHA를 보존한다.
3. name+GUID exact join만 허용하고 `bOverride=true`, `bOverride=false`, no-GUID-entry를 별도
   decision으로 유지한다.
4. sampler 기존 68행과 기존 exact 4행을 합친 strict 72행을 source Texture2D export에서
   재파싱한다. `AddressX/Y`, `sRGB`, `Filter`, `LODGroup`을 field별 explicit/omitted로 보존한다.
5. render-state 89행은 source CDO 부재와 current-only CDO 후보를 분리하고, 현재 WARP pilot은
   consumer semantics만 증명한다고 기록한다.
6. source archive 1,813 package의 raw SHA 중복 제거, v3 manifest 621개와 extra package의 class/key
   search를 재검증한다.
7. driver cache, Git/remote, controlled-capture 감사 결과를 admission input이 아닌 read-only
   corroboration snapshot으로 고정한다. 이 generator가 재생성하지 않은 observation은
   `corroborationOnly=true`, `regeneratedByThisGenerator=false`, null verification manifest,
   session-date precision과 access caveat를 명시한다. VSS는 `PERMISSION_UNCHECKED`이며 global exhaustion을
   주장하지 않는다.
8. row set, acquired-value set, invalidated exact-sampler set에 각각 canonical SHA-256을 부여한다.

### generated receipt와 mutation test

- `skill.31470.material-source-value-acquisition.receipt.json`
  - format version 2이며 runtime receipt를 입력으로 소비하지 않는 acyclic evidence receipt
  - render `89`, static `94`, strict sampler `72` 전 행
  - provider/raw offset/default chain/consumer pilot/decision/owner
  - source archive, current CDO, driver/Git/remote/capture exhaustion
  - coordinated denominator mutation 요구와 Product false/R2 NO-GO
- `test_build_artist_31470_material_source_value_acquisition.py`
  - committed receipt를 raw source에서 완전히 재생성한다.
  - GUID, `bOverride`, native value, exact-4 decision, 72 denominator, Product/execution-ready bit의
    coordinated reseal mutation을 거부한다.

## 종료 조건

- static 94/94 raw parent identity가 재검증되고 MIC exact join 분모가 고정된다.
- 기존 exact sampler 4/4를 행 단위로 재감사하고 strict sampler denominator 72를 coordinated
  Material corrective에 적용한다.
- source Texture2D 존재/explicit field/omitted default를 72/72에 기록한다.
- render 89/89의 최소 missing source artifact와 safe capture 경계를 기록한다.
- source-value delta와 execution-readiness delta를 분리한다.
- focused test, generated JSON check, JSON parse, `git diff --check`가 PASS한다.
- Product는 false이고 R2는 NO-GO다.
