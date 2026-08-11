# 2026-08-11 Artist 31470 F Material Render Resource Binding Approval Plan

## 목표와 범위

frozen reconstructed runtime program의 Material/renderer 경계를 Product 실행 전에 명시적인
정책 승인 receipt로 고정한다. 이 receipt는 원본 소스 증거가 아니다. deterministic generator가
검토 후보를 만들고, 최종 27개 recipe 선택, 57개 renderer slot 결합, 46개 D3D11 descriptor를
독립 projection hash로 동결한다.

이번 변경은 Python/JSON/ProjectAudit/PLAN/RESULT만 소유한다. Engine/Client C++, shader,
`Client/Bin/Resources`는 변경하지 않는다. runtime 실행, action-time I/O, Product admission도
열지 않는다.

generated receipt는 raw byte identity가 freeze gate이므로 `.gitattributes`에 이 receipt exact path만
`text eol=lf`로 고정한다. broad `*.json` 규칙은 추가하지 않는다. `core.autocrlf=true`에서도 같은
bytes가 나오는지 temporary alternate index를 사용한 isolated checkout 회귀로 검증한다.

## frozen 입력

| 입력 | 고정 identity |
|---|---|
| reconstructed program raw bytes | 15,072,141 |
| reconstructed program raw SHA-256 | `72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849` |
| reconstructed program SHA-256 | `618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b` |
| Material runtime oracle tracked/self | `58b06308713f87262a9a716e48fc29ba5137ab77507f0e9d3ee633938e5ddb34` / `e128e281753fbd01582e588afbb682847401348836a046ad424a720360003ff6` |
| Material policy tracked/self | `752ccebdb52a758e538553ec40967d2327966187ebb3fdcc20698b8fc6261489` / `10c7cacac0c54bf22060ab54a5596d48785631dd65b12c1c2810a87eb013d1c7` |
| Material texture binding tracked/self | `87a28be564308117ac666206382c94ce5ee2bf37a47111cbef717994a0266077` / `3e722cf02085497c63083fbf51161ff5fd6670be91607737863b9c4019e55b48` |
| Effect shader mapping reference tracked SHA-256 | `34ca3d68ea1e2d8714e975c2dc4c0ad245a399c9f4f7aee759bc993485675baf` |
| Engine shader-state mapping reference tracked SHA-256 | `869b1aec6f1a5839937b82f19cf88e4b38fe6890a4b299818709b483ed0a80f8` |

Program은 canonical LF UTF-8 exact-path checkout이어야 한다. validator는 기본 frozen program을
실제 bytes에서 strict parse하고 기존 program validator를 통과시킨다. Material runtime/policy/
texture receipt도 실제 tracked bytes, canonical digest, self seal을 다시 확인한다. caller가 넘긴
program object가 있으면 기본 parse 결과와 recursive strict-equal이어야 한다.

## 승인 결정

### Renderer slot 57개

각 renderer resource는 frozen `materialOccurrenceId`와 완전한 Resources-relative `assetId`로만
Material texture binding 후보를 구한다. 54개는 후보가 정확히 하나여야 한다. 나머지 3개는
독립 승인 module에 exact resource ID → Material field ID를 열거한다. basename, parameter role,
slot 이름은 실행 시 선택 규칙으로 사용하지 않는다.

### Recipe texture0/texture1/fallback 27개

generator의 우선순위는 오직 offline review candidate 생성용이다. 최종 receipt는 선택된 field,
binding, sampler-policy ID와 각 row SHA를 recipe별로 완전히 열거하고 independent projection이
그 결과를 고정한다.

- texture0은 승인된 renderer 결합을 우선 검토하고, 없으면 frozen program texture-binding order를
  사용한다.
- texture1은 SECOND_TEXTURE/DISTORTION feature가 소비할 때 texture0과 다른 승인 결합을 우선한다.
- texture field가 없거나 distinct field가 없으면 evaluator 의미에 따른 explicit neutral provider를
  선택한다.
- 선택된 Material binding의 stage 실패는 neutral로 암묵 대체하지 않고 전체 transaction을 rollback한다.

Neutral provider는 다음 네 개만 허용한다.

| provider | RGBA | evaluator 의미 |
|---|---|---|
| `RECONSTRUCTED_UNIT_BASE_WHITE_RGBA` | `[1,1,1,1]` | texture0 단위 base sample |
| `RECONSTRUCTED_SECOND_TEXTURE_MULTIPLY_IDENTITY_RGBA` | `[1,1,1,1]` | `0.5 + 0.5 * texture1 = 1` |
| `RECONSTRUCTED_SIGNED_DISTORTION_ZERO_RGBA` | `[0.5,0.5,0.5,1]` | `texture1 * 2 - 1 = 0`; second-texture factor는 0.75임을 명시 |
| `RECONSTRUCTED_UNUSED_ZERO_RGBA` | `[0,0,0,0]` | feature mask가 texture1을 소비하지 않음 |

distortion과 second texture가 동시에 활성화된 recipe의 0.5 neutral은 distortion offset을 0으로
만들지만 색 factor는 0.75다. 이를 source-exact나 시각적 정답으로 주장하지 않고 WARP 및 수동 눈
검증 blocker로 유지한다.

### D3D11 descriptor 46개

- blendmode 27개: `BS_EffectOpaque`, `BS_EffectAlpha`, `BS_EffectAdditive`의 전체
  `D3D11_BLEND_DESC`, MRT 8개 render-target descriptor 포함
- `twosided=true` 18개: `RS_Cull_None` 전체 `D3D11_RASTERIZER_DESC`
- `bdisabledepthtest=true` 1개: `DSS_ZNone` 전체 `D3D11_DEPTH_STENCIL_DESC`

shader text는 현재 implementation mapping reference일 뿐 source-revision Material evidence가 아니다.
모든 descriptor row는 `requiresAutomatedWARPProbe=true`이며 이 prerequisite lane 자체는 WARP PASS를
주장하지 않는다.

## 파일

| 파일 | 역할 |
|---|---|
| `Tools/LevelPlacementExtractor/build_artist_31470_material_render_resource_binding_approval.py` | deterministic generator, strict validator, current-authority 결합 |
| `Tools/LevelPlacementExtractor/artist_31470_material_render_resource_binding_approval.py` | 3개 ambiguity 선택과 최종 decision/receipt projection pin |
| `Tools/LevelPlacementExtractor/test_build_artist_31470_material_render_resource_binding_approval.py` | strict type/order/digest, coordinated reseal, authority mutation 회귀 |
| `Data/Effects/Imported/Artist/Materials/skill.31470.material-render-resource-binding-approved-v1.receipt.json` | 27/57/46 explicit reconstructed approval |
| `Tools/ProjectAudit/Test-Artist31470MaterialRenderResourceBindingApproval.ps1` | focused denominator/gate audit |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | focused audit 등록 |
| `.gitattributes` | approval receipt exact-path `text eol=lf` checkout 계약 |
| 대응 `RESULT.md` | 실제 identity, 검증, 남은 gate 기록 |

새 C++ 파일이 없으므로 `.vcxproj/.filters` 등록과 C++ build는 이번 단위에 해당하지 않는다.

## 실패와 rollback

다음 중 하나라도 발생하면 receipt 생성/검증을 실패한다.

- program raw byte-count/SHA/LF 또는 program semantic validator 불일치
- 현재 Material authority tracked/canonical/self identity 불일치
- renderer 후보가 1 또는 2가 아니거나 exact 3개 ambiguity 집합이 바뀜
- 27/57/46 denominator, source row ID/SHA, 선택 projection 불일치
- neutral provider가 feature mask와 evaluator 수식에 맞지 않음
- bool→integer, 문자열→숫자, non-finite float, duplicate key, missing/extra/reordered key
- row/root SHA만 다시 봉인한 coordinated decision 변경
- independent decision 또는 full receipt projection 불일치
- `sourceExact`, runtime/Product, action-time I/O, partial commit gate 승격

## 검증

1. Python unit mutation suite
2. generator deterministic `--check`
3. exact 3개 ambiguity reverse-choice coordinated reseal 회귀
4. texture0/texture1/neutral/fallback mutation 회귀
5. blend/raster/depth descriptor mutation 회귀
6. current authority byte identity와 supplied program strict-equality 회귀
7. temporary alternate index에서 `core.autocrlf=true` isolated checkout 생성
8. isolated checkout에서 bytes 376,183/raw/self/decision/CR0/BOM0, unit 20/20,
   generator `--check` 재검증
9. focused ProjectAudit
10. 전체 ProjectAudit 등록 확인
11. JSON parse, `git diff --check`, clean scope 확인

완료 산출물은 commit/push하지 않고 independent review를 위해 uncommitted 상태로 동결한다.
