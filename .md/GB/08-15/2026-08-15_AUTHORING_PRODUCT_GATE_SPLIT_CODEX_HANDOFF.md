# 2026-08-15 authoring/product 게이트 분리 — Codex 인계 문서

이 문서는 Claude 세션이 4직업 Track A locked 원인 규명과 authoring 게이트 분리까지 진행한 결과를
Codex 세션에 넘기기 위한 단일 인계 정본이다. Codex는 런타임 drawability를, Claude는 authoring
architecture를 담당하는 역할 분리 상태에서 작성했다.

**읽는 순서**: `AGENTS.md` → `CLAUDE.md` → `.md/GB/gotchas.md` → 이 문서 →
`.md/GB/08-15/2026-08-15_LOCKED_1007_ROOT_CAUSE_AND_FAMILY_ADMISSION_ANALYSIS.md`

> **2026-08-15 Product admission 정책 정정**: 아래 조사 경과의 `AUTHORING_APPROXIMATE` 전면
> Product 거부 문구는
> [`FOUR_CLASS...IMPLEMENTATION_PLAN.md` §13](../08-14/2026-08-14_FOUR_CLASS_VALTAN_EFFECT_RESTORATION_AND_RAID_PERFORMANCE_IMPLEMENTATION_PLAN.md#13-product-approved-approximate-admission과-first-canary-transaction)의
> cue-scoped 정책으로 대체한다. 명시적 사용자 승인, authored SHA, PlayerSkills/skillbindings/clip
> tuple, provenance, rollback receipt를 모두 검증한 cue만 `PRODUCT_APPROVED_APPROXIMATE`로
> 연결한다. exactness는 `APPROXIMATE`로 계속 표시하며 Hard/fail-closed/unsafe는 Product 금지다.

---

## 현재 통합 결론과 승인된 다음 순서

이 절은 2026-08-15 Codex checkpoint에서 실제 코드·데이터·실행 결과를 다시 대조해 추가한
현재 정본이다. 아래의 과거 수치는 조사 경과로 보존하되, 다음 작업의 입력은 이 절을 우선한다.

현재 디스크의 101개 authored candidate는 **육안 비교용 기존 baseline**이다.

```text
101 documents / 4,777 elements
Particle strict mapped                 4,687
  portable / source-deferred           4,488 / 199
  product-full / fail-closed            3,481 / 1,007
Decal                                      79 = ready 46 / incomplete 33
AnimationTrail                            11 = source notify 8
authoringApproximate                       0
product mapping mutation                   0
restoration receipt SHA-256 15bc2eb079575fde3bf97091d6a6cb887acfc21460ac5bdbf8fb32ef09e6a7af
```

Wave 0 material evidence, strict approximate admission과 Lance BA4 dynamic-arithmetic 경계를 적용한
**no-write projection**은 다음과 같다. 이 수치는 아직 `EXPECTED_*`가 아니며 101문서에도 쓰지 않았다.

```text
portable Particle                       4,488
  product-full                           2,793
  authoring approximate                    722
  hard portable fail-closed                973
preview target = full + approximate      3,515
source-deferred                            199
output projection                        4,777
```

G3 통합과 Client x64 Debug 빌드는 완료됐다. 자동 checkpoint는
`--effect-g3-authoring-fast` 15/15, Python ownership/typed material/Warlord 7/7,
`Test-EffectPipeline.ps1`, `--effect-runtime-fast`, focused GPU/isolation/boss-index harness와 Client
compile/link PASS다. 사용자는 같은 기존 baseline에서 Artist F와 DimensionMaster A 2050210을 승인했고,
두 번째 확인에서 Lance 34010 BA1도 승인했다. DM T 2050500, DM 2050010 BA3 composite, Warlord 17090은
첫 focused 수정 뒤에도 거부했으며 Valtan 420633은 상단 owner selector에 없어 같은 흐름으로 선택할 수
없음을 확인했다. 이 피드백으로 기존 fixture 공백을 폐기하고 네 alternate slice를 다시 구현·자동검증했지만
새 Client 화면의 사용자 재검증은 아직 받지 않았다. 스크린샷과 세부 root cause는 메인
[`RESULT`](../08-14/2026-08-14_FOUR_CLASS_VALTAN_EFFECT_RESTORATION_AND_RAID_PERFORMANCE_RESULT.md)의
10.1~10.2절을 정본으로 사용한다.

현재 승인된 진행 순서는 다음과 같다.

1. G3와 Client Debug checkpoint는 완료 상태로 보존한다.
2. 101 rewrite, denominator rebaseline, 장시간 full native gate는 계속 중지한다.
3. Lance BA1 strict evaluator 승인본은 보존한다. DM T bind-pose recook, DM BA3 8-element parent-family
   evaluator/axis lock, Warlord exact DDS·4방향 발사/회수, Valtan 상단 owner selector alternate slice — 완료.
4. focused Python 8/8, Effect pipeline, G3 15/15, runtime, DM T isolation, rejected-slices 8/8 GPU/rollback과
   Client x64 Debug shader compile/full link — PASS. 에이전트는 Client/UI를 실행하지 않았다.
5. 사용자가 DM T·DM BA3·Warlord·Valtan 같은 occurrence를 직접 재검증하고 Save/Reload가 필요한 튜닝은
   G3 override로 보존한다 — 현재 단계. Lance는 다시 묻지 않고 자동 회귀 control로만 유지한다.
6. 이 결과가 방향을 지지할 때만 `EXPECTED_*`를 한 번 재기준화하고 101문서를 transaction write한 뒤
   full 101 Stage/Draw와 Debug/Release 회귀를 실행한다.

이 순서를 바꾸지 않는다. 기존 baseline을 먼저 보는 이유는 data rewrite와 runtime/G3 변경을 동시에
섞으면 화면 개선 또는 회귀의 원인을 분리할 수 없기 때문이다.

대표 육안 순서는 다음과 같다. 최종 visual PASS는 사용자만 기록한다.

| 순서 | 대상 | 확인할 공통 경계 |
|---:|---|---|
| 1 | Artist F 31470 | 이미 승인된 control, 공용 변경의 회귀 여부 |
| 2 | Artist LMB 31000 | grouped UV pan/edge feather와 sparse Sprite |
| 3 | DimensionMaster A 2050210 | Mesh/Sprite 개별 재생, WModel preScale/transform, image flip |
| 4 | DimensionMaster T 2050500 | bind-pose 교정 summon ModelCue가 동심원 cage 없이 정상 skeleton/animation인지 |
| 5 | LanceMaster LMB BA1 34010 | `USER_APPROVED`; 재요청하지 않고 strict evaluator 회귀 control로만 유지 |
| 6 | DimensionMaster LMB BA3 2050010 | Mesh4/Sprite4, `002` ring과 두 particle trail, 보라·청색 composite |
| 7 | Warlord A 17090 | chain06 8 + chain07 창끝 4가 4방향으로 발사됐다 회수되는지 |
| 8 | Valtan 420633 Whirlwind | 상단 `Character / Boss > Valtan`에서 Open/Play All/Family/Solo 가능한지 |

이번 사용자 검증에서 확정된 focused 수정 입력은 다음과 같다.

| 대상 | 확정 원인 | 실제 구현과 자동 증거 |
|---|---|---|
| DM T 2050500 | 첫 retime SHA `f68fb4...1a3b` 뒤에도 cage가 유지됐다. active PSA Action frame이 WSKL rest로 bake됐지만 WMSH는 PSK inverse bind를 유지해 normalized bind 오차가 최대 `90.678497`인 것이 실제 결함이다 | Action을 detach하고 POSE depsgraph bind snapshot으로 recook한 SHA `87186351...22b6`을 배포했다. identity 오차 `5.68e-14`, 4 section/13,806 verts/20 source bones, 2 clip 30 tps/19 moving clock bones와 source/runtime CPU skin bounds PASS. 0초 ModelCue owner와 후반 2.90142초 material notify는 계속 분리 |
| Lance 34010 BA1 `ce43...` | diffuse 오연결이 아니다. exact trail 5-lane과 청색 ParticleColor `(0.2,0.375,0.5)`가 존재하지만 generic grouped 축약이 `emissive_tex_strength=5000`을 직접 적용해 흰색 포화하고 tile/UV-noise 이름도 잘못된 scale/pan으로 낮춤 | 기존 Valtan legacy missile과 분리한 strict Lance two-emissive evaluator가 5 lane, ParticleColor, strength/pow/UV 순서를 소비한다. WARP 9,677 px와 비백색 RGB chroma PASS 뒤 사용자 `USER_APPROVED`. disk admission은 불변 |
| DM 2050010 BA3 composite | 최초 gate가 8개 중 water `14e8...` 하나만 봤다. ring `trail_002`의 emissioncolor 0을 final tint로 쓰고, makeflow의 back color를 final tint로 쓰며, 두 particletrail은 hidden alpha lane 누락과 EPAL_Z one-sided front-normal 역전으로 미출력됐다 | exact WATERTRAIL/LINEARFLOW/MAKEFLOW/RING/PARTICLETRAIL evaluator와 hidden DDS lane을 연결하고 EPAL_Z 앞면 부호를 교정했다. ordinary 문서 Mesh4/Sprite4 전부 nonzero RGB, typed 7개 chroma와 composite `B>R>G` PASS; 개별 픽셀 `23872/5070/7016/18407/8946/23840/1307/10`. 101/disk admission 불변 |
| Warlord 17090 | rotation/admission 교정 뒤에도 WModel material lane이 비어 흰색으로 나왔고, 정확한 LocationDirect 왕복 곡선은 미직렬화 ScaleFactor가 zero라 정지했다 | exact parent Diffuse `fx_d_grid_016.dds`(SHA `183b4120...e7666d4`)와 `useModelMaterial=false`, exact tuple ScaleFactor identity, normalized-life LocationDirect를 연결했다. chain06 8 + chain07 창끝 4의 네 방향과 `.491165 -> .377818` 회수 거리 PASS. WPO 식 미확보라 Approximate/product-blocked/FULL 미승격 유지 |
| Valtan 420633 | authored 문서·binding·ordinary Stage는 있으나 사용자 기대 흐름의 상단 selector가 PlayerSkills 6 class만 소유했다. 하단 별도 tree만으로는 발견·선택할 수 없었다 | `CHARACTER_CLASS` 확장 없이 상단을 UI-only `Character / Boss` 7-option selector로 바꾸고 Valtan에서 exact 420633을 기존 unified Open/Play All/Family/Solo tree에 노출했다. six-player 수 불변, exact once, duplicate/path escape/ID mismatch/missing/corrupt rollback PASS, product mapping 0 유지 |

---

## 전체 복원 로드맵과 실제 병목

| 단계 | 실제로 한 일 | 얻은 증거 | 남은 병목 |
|---|---|---|---|
| Artist F vertical slice | 31470 전용 raw material/state 추출과 typed evaluator, WModel 1.1/preScale, SourceRecipe 소비를 수제 연결 | 사용자 visual 승인과 exact regression control | 공용 importer/compiler로 일반화되지 않음 |
| 4직업 1차 확장 | 13 stage + Legacy starter를 이용한 selected surface 생성 | Effect Tool의 101 stable target/path와 편집 seam | 2,160 selected surface만 포함하고 legacy-baked Detail에 recipe transform을 다시 적용 |
| 사용자 화면 재개방 | DM A에서 한 종류 Mesh만 보이고 Sprite가 잠긴 것을 실패로 기록 | Stage/probe만으로 픽셀을 증명할 수 없음을 확인 | draw witness, geometry/preScale, material arithmetic 검증 부재 |
| full-source occurrence 복원 | Particle 4,846을 strict 4,687 + exclusion 159로 재결합, stable source event/element lineage 사용 | 101문서/4,777 element와 class별 원천 소유권 | source-deferred 199와 material/drawable blocker |
| 공용 particle/runtime 보강 | module allowlist와 ordered evaluator, ModelCue reset/OPAQUE, WModel transform, AnimationTrail, rollback gate | portable 4,488, DM A 98/2 recipe capability, DM T actual Stage | 모든 portable이 원본 material fidelity를 갖는 것은 아님 |
| Wave 0 material evidence | 953 material candidate, parent props 901, renderState 2,698, texture parameter 10,258을 repo evidence로 보존 | compiler가 parent Material evidence를 실제 소비 | cooked arithmetic graph는 영구 부분 결손 |
| authoring/product 분리 | Full / Approximate / Hard를 분리하고 publisher는 Approximate를 거부 | 편집·튜닝과 제품 승인을 섞지 않는 구조 | G3 override 소유권과 사용자 육안 ledger 필요 |
| G3 generic authoring | 13 authoring family와 compiler-owned/artist-owned override 계약 통합 | resource/scalar/color override Save/Reload/reimport와 product reject focused PASS | 사용자 화면에서 필요한 튜닝의 실제 사용성 확인 |
| Valtan Whirlwind canary | 420633 source occurrence와 Server timeline, Model View join, 9 carrier 중 3 ordinary 실행 | 전투 이펙트에도 같은 authored/runtime gate를 적용 | Trail/Dust/Light 및 제품 mapping은 미승인 |

가장 큰 병목은 DDS 파일 수가 아니다.

- **material arithmetic**: cooked Material graph에서 null slot 1,803 / unresolved edge 502가 확인됐고
  ShaderCache direct match도 0/23이다. parent 이름만으로 `dissolve`, distortion, opacity 수식을 추정하면
  흰색·회색·보라색 판 또는 전 수명 0픽셀을 다시 만든다.
- **authoring 소유권**: compiler가 SourceRecipe/profile/resource 기본값을 갱신하되 사용자가 바꾼
  DDS/WModel/scalar/color override는 re-materialize 뒤에도 살아야 한다. 현재 G3가 이 경계를 소유한다.
- **exact resource/geometry**: WModel 미확보, named texture alias, Base 없는 Sprite, material lane 자체가 없는
  procedural parent를 서로 다른 이유로 분리해야 한다. 임의 white/Base fallback은 금지다.
- **runtime family**: TypeDataRibbon, LocationEmitter, Orbit, Collision, Character Afterimage는 ordinary
  Particle/AnimationTrail과 다른 실행기 또는 source semantics가 필요하다.
- **검증 순서**: parse/Stage/probe는 픽셀과 원본 fidelity 증거가 아니다. 그렇다고 data rewrite와 full GPU
  gate를 먼저 돌리면 공용 runtime 방향을 사용자 화면에서 비교하기 어려우므로 이번에는 대표 baseline
  육안검증을 먼저 둔다.

### profile과 캐릭터별 복구 원리

여기서 `profile`은 캐릭터별 색상 preset이 아니다. 원본 UE3 Material instance와 parent가 가진 texture
lane, scalar/vector, blend/two-sided/depth, dynamic parameter 의미를 현재 renderer가 실행할 수 있는
typed 계약으로 정리한 것이다.

```text
source occurrence
  -> SourceRecipe      spawn/lifetime/location/velocity/size/color/SubUV/module 순서
  -> resources         exact DDS/WModel asset ID
  -> source material   exact child path + physical package + parent + parameters
  -> runtime profile   renderer가 실행하는 lane/state/dynamic evaluator
  -> authoring family  Effect Tool에서 노출하는 편집 표면
```

공용화 단위는 캐릭터 이름이 아니라 exact material contract다. 두 클래스가 같은 exact parent/lane/parameter
계약을 증명하면 같은 runtime evaluator를 재사용할 수 있지만, material 이름이 비슷하다는 이유만으로
DimensionMaster 값을 Artist/Lance/Warlord에 복사하지 않는다.

- DimensionMaster는 동일 784 source Element를 가진 기존 v12 sibling이 stable ID별 canonical profile을
  보존해 이 범위에서만 직접 재결합했다. Base 없는 grouped Sprite 8개도 이 authority로 실행 가능하다.
- Artist/Lance는 기존에 남은 finite profile과 각자의 exact graph/material receipt를 사용한다.
- Warlord는 완성 canonical sibling이 없으므로 Warlord source package/evidence와 공용 compiler 결과만 사용한다.
- 어느 클래스든 exact evidence가 닫히지 않으면 approximate 또는 hard-lock으로 분류하고 다른 클래스 donor를
  쓰지 않는다.

---

## 0. 이미 닫힌 것 — 다시 조사하지 말 것

### 0.1 grouped-translucent UV feather 버그 (Codex가 닫음)

Artist 31000 exact Sprite의 `Stage PASS → WARP pixel witness 0` 원인은 데이터/DDS/material profile
문제가 아니었다. `grouped-translucent` 공통 shader에서 dynamic UV pan으로 이동한 UV(+2.58)를
quad-edge feather 계산에도 그대로 써서 `1 - 2.58 < 0` → alpha 전량 0이 됐다.

수정은 texture sampling UV와 carrier edge feather UV를 분리한 것이다. 데이터/recipe/DDS는 바꾸지
않았고 Artist 전용 hack이 아니라 공통 shader 수정이다. 현재 트리에 반영돼 있다
(`Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`, `localUV` 파라미터와 311행 주석).

GPU 검증: world view 약 24px에서 RGB pixel 6개, 96px shape-fit proof view에서 80개, 영구
zero-output synthetic은 계속 zero, fallback-blocked 강제 재활성화는 계속 codec/runtime에서 거부.

**이 failure class는 닫힌 것으로 본다. 같은 GPU visibility 원인을 다시 조사하지 않는다.**

### 0.2 "왜 visible인데 안 보이는가" 재조사 금지

DM A에서 63개 `visible=true`인데 화면에 한 종류만 보이던 현상의 상당 부분이 위 shader 버그였을
가능성이 크다. 다만 63개 전부가 이 하나의 원인이라고 단정하지 않는다. **101 full Stage/Draw에서
family별로 몇 개가 살아나는지가 판정 근거**이며, 새로운 조사 계획을 세우지 말고 위 fix를 공통
baseline으로 사용한다.

---

## 1. 4직업 locked의 근본 원인 (Claude 실측 확정)

상세는 `2026-08-15_LOCKED_1007_ROOT_CAUSE_AND_FAMILY_ADMISSION_ANALYSIS.md`에 있다. 요약만 옮긴다.

### 1.1 도화가 F가 확장되지 않은 이유

Artist F 31470은 전용 심층 추출(`extract_artist_31470_material_render_state.py`)로 raw UPK 19개에서
scalar 342 / vector 19 / texture 71 / parent default 297 / static switch 94 / BlendMode 23-of-23을
확보하고 arithmetic family 23개를 사람이 닫은 **수제 vertical slice**다. `CLAUDE.md`가 명시하듯
그 전용 처리는 공용 importer에 복제하지 않았다.

4직업 확장은 공용 compiler를 탔는데, 그 경로는 parent Material 증거를 **한 건도** 입력받고 있지
않았다. `build_four_class_source_material_contract.py`가 `build_contract()`를 호출하면서
`material_map` 인자를 넘기지 않은 것이 직접 원인이며, 그 결과 769개 material identity 전부가
`materialResourceDecodeStatus=NOT_CAPTURED`, `renderState=null`,
`collectedTextureParameters=[]` 상태였다.

### 1.2 영구 결손 — arithmetic

parent Material의 expression 그래프는 cooked 배포본에서 복원 불가다. Artist 31470 실측 기준
null expression slot 1,803 / unresolved edge 502 / surviving edge 125이고, ShaderCache 1,596 export는
존재하지만 state-key direct match 0/23으로 미해독이다(`MATERIAL_SHADER_MAP_KEY_UNRESOLVED`).
따라서 family evaluator는 추출 결과가 아니라 **재구성**이다. 이 경계는 확정이다.

DimensionMaster props 경로의 `expressionCoverage.nullCount = 0`은 그래프가 온전하다는 증거가 아니라
UModel props 출력이 null slot을 인쇄하지 않아 생긴 capture artifact다.

---

## 2. 이 세션에서 실제로 바꾼 것

### 2.1 Wave 0 — parent Material3 evidence 재추출

| 산출물 | 경로 | 내용 |
|---|---|---|
| evidence 정본 | `Data/Effects/Imported/FourClassCombat/FourClassCombat.source-material-evidence.json` | 953 candidate / parent props 해결 901 / fail-closed 52. schema `lostark.effect-source-material-evidence` v1, `characterClass=FOUR_CLASS` |
| evidence receipt | 같은 폴더 `.receipt.json` | 입력 3종 SHA pin |
| 추출 catalog | 같은 폴더 `FourClassCombat.material-candidate-catalog.json` | 953행. schema `lostark.four-class-material-candidate-catalog` |
| parent 패키지 leaf map | 같은 폴더 `FourClassCombat.parent-package-map.json` | 903 entry. leaf object → logical package |
| UModel 원시 산출물 | `<Resource_LostArk>/05_Reports/EffectExtraction/FourClassMaterials/` | `export/`, `FourClass.material-map.json`(23MB), `FourClass.material-extract.receipt.json` |

추출 결과 품질: material row 2,698, **renderState 2,698/2,698**, `collectedTextureParameters` 보유
2,231, texture-parameter row 10,258.

`materialEvidenceStatus` 분포: `SOURCE_MATERIAL_PROPS` 2,506 / `SOURCE_MATERIAL_EMPTY_PARAMETERS` 123 /
`SOURCE_MATERIAL_DUMP` 57 / `MISSING_OR_AMBIGUOUS_SOURCE_MATERIAL_PROPS` 12.

**재현 명령**

```bash
python Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py \
  --catalog Data/Effects/Imported/FourClassCombat/FourClassCombat.material-candidate-catalog.json \
  --inventory-csv "<Resource_LostArk>/05_Reports/EffectExtraction/DIMENSIONMASTER/all_bound_skills/source-package-inventory.csv" \
  --parent-package-map Data/Effects/Imported/FourClassCombat/FourClassCombat.parent-package-map.json \
  --umodel "<Resource_LostArk>/06_Tools/UEViewerLostArk_runtime/umodel_lostark_v7.exe" \
  --package-root "<Resource_LostArk>/00_SourcePackages/Effect_DIMENSIONMASTER_20260803_v3" \
  --output-root "<Resource_LostArk>/05_Reports/EffectExtraction/FourClassMaterials/export" \
  --output-map "<Resource_LostArk>/05_Reports/EffectExtraction/FourClassMaterials/FourClass.material-map.json" \
  --receipt "<Resource_LostArk>/05_Reports/EffectExtraction/FourClassMaterials/FourClass.material-extract.receipt.json"
```

> **주의**: 현재 checked-in receipt의 `sourceCatalogSha256`은 세션 scratchpad 경로의 catalog로
> 고정돼 있다. 위 repo 경로 catalog는 byte-identical 복사본이지만, receipt를 다시 만들 때는
> repo 경로로 한 번 재실행해 provenance를 repo 안으로 옮기는 것이 좋다. 이 불일치는 알려진 상태다.

exit code는 `failureCount > 0`이면 1이다. 48/953 실패는 crash가 아니라 정상 fail-closed 계수다.

### 2.2 세 번의 "증거를 더 모으면 admission이 좁아지는" 역전 수정

전부 같은 원칙 위반이었고, 각각 원인이 달랐다.

1. **선택 순서** — `grouped_translucent_selection()`에서 parent가 primary lane을 선언했는데 그
   파라미터가 미해결이면 emitter의 exact Base DDS를 확인도 하지 않고 BLOCK했다.
   `safe_carrier_slots` 검사를 `primary_names` 블록보다 앞으로 옮겼다.
   (`Tools/LevelPlacementExtractor/build_effect_source_material_contract.py`)
2. **cross-package parent 미export** — MIC와 parent가 다른 패키지일 때 UModel `-obj` 단일 export가
   parent를 끌고 오지 않는데 props chain은 요청별 출력 디렉터리 안에서만 해석된다.
   `export_parent_material()`을 추가해 parent를 같은 디렉터리로 한 번 더 export한다. 패키지는
   (1) parent 경로 접두사, (2) `--parent-package-map` 순으로만 결정하고 추측하지 않는다.
   (`Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py`)
3. **empty-parameter parent 오분류** — UModel이 `Ignoring Material3'X' due to empty parameters`로
   거부한 parent를 "증거 없음"으로 처리했다. 이는 exporter의 **긍정 진술**(이 parent는 파라미터를
   선언하지 않는다)이므로 `SOURCE_MATERIAL_EMPTY_PARAMETERS` 상태를 신설했다. 123행 해당.
   provenance는 그 주장을 담은 `umodel.log`의 SHA를 사용한다.
   (extractor + `build_dimensionmaster_source_material_evidence.py`)

### 2.3 Wave 1 — group 라우팅 어휘 확장

`source_group_slot()`에 **실측 관측된 group 문자열만** 추가했다. 근거는 새 추출의 10,258
texture-parameter row다.

```
base    += diff(353), maintex(174), main(01_main 18)
noise   += flow(tex_flow 57 / flow 27 / uv_flow 9 / volume_flow 6),
           distort(3_distort01 51 / 4_distort02 33 / distortiontexture 24 / distort 18),
           distotion(27, 원본 오타)
```

`05_specullar` 447, `cracknormal` 66, `13_rainbow`, `lamp`, `rampshape`, `transition`, `sparkle`,
`reflection` 등은 색/알파 carrier가 아니므로 **의도적으로 넣지 않았다.**

### 2.4 G1 — authoring / fidelity / product 3단 게이트 분리

`bFailClosed` 하나가 visible·편집·제품승인을 전부 결정하던 구조를 분리했다. 새 필드는
`bAuthoringApproximate` 하나이며 `bFailClosed` 없이 단독 성립할 수 없다.

| 계층 | 파일 | 변경 |
|---|---|---|
| 계약 | `Client/Public/Effect_AuthoringDocument.h` | `EFFECT_MATERIAL_EXECUTION_DESC::bAuthoringApproximate` |
| 직렬화 | `Client/Private/Effect_DocumentCodec.cpp` | parse / write / validate 3곳. `enabled=true` 공존 거부, `failClosed=false` 공존 거부, hidden-state 필드 수 검사 반영 |
| 렌더러 | `Client/Private/Effect_DocumentRenderer.cpp` | `bOccurrenceVisualSuppressed`를 `failClosed && !approximate`로. staging signature 비교와 ordinary-fail-closed 경로 동기화 |
| 재생 | `Client/Private/Effect_Playback.cpp` | portable event route 1곳, vector field prepare 2곳 |
| 도구 | `Client/Private/Effect_Tool.cpp` | `Is_ElementPreviewAdmitted`가 approximate 통과. Visible 체크박스 활성 + `Fidelity: APPROXIMATE` 배지 + 설명 |
| **제품 차단** | `Tools/EffectPipeline/Publish-Effects.ps1` | `authoringApproximate` element를 가진 문서 publish 거부 |

publish 거부가 핵심이다. `Product mapping 0 유지`가 정책이 아니라 구조로 강제된다.

초기 인계 시점에는 실행 중인 harness가 EXE를 점유해 링크가 막혔지만, Codex 소묶음 마감에서는 최신
ClientFrontendHarness x64 Debug compile/link가 PASS했다. Client 본체의 최종 Debug/Release는 G3 통합 뒤
다시 빌드한다.

### 2.5 G2 — approximate 자격 판정 (Codex 교차감사 뒤 축소)

초기 G2는 fallback/profile-resource blocker까지 approximate로 열었지만, Codec/Renderer 실제 staging을
대조하니 그 행들은 실행할 profile/resource 계약 자체가 없었다. 현재 자격은 아래 두 사유로 축소했다.

```text
NON_EXACT_NAMED_TEXTURE_ALIAS
SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE
```

두 경우 모두 다음 하드 조건을 함께 만족해야 한다.

- `effect.ue3.grouped-translucent.v1` profile이 enabled/ready다.
- Sprite는 실제 texture binding이 하나 이상 존재한다.
- Mesh는 texture와 exact WModel이 모두 존재한다.
- portable source recipe와 same-document event route 검증을 통과한다.
- `execution={enabled:false, failClosed:true, authoringApproximate:true}`이고 Visible은 source 값을 유지한다.
- publisher와 animation-event/product preview는 이 문서를 직접 거부한다.

`linearflow`, `blackline`, `local-crack`처럼 named texture identity를 shader가 직접 소비하는 profile은 alias
사유만으로 approximate가 될 수 없다. fallback-blocked, profile-not-compiled, missing drawable/WModel,
EngineMaterials policy, resource-contract blocker도 계속 hard fail-closed다.

### 2.6 Lance BA4 dynamic arithmetic 경계

Lance 34010 BA4의 `fx_m_pa_spritewave_01_tr` 계열은 resource/sampler/draw가 정상인데도 전 수명 0픽셀이었다.
정확한 원인은 cooked parent graph가 260 expression 중 159 null, unresolved edge 55인
`COOKED_PARTIAL/runtimeExactEligible=false`인데 `ParamName=dissolve`라는 이름만 보고 generic
`opacity *= 1 - value`를 적용한 것이다. 실제 값 1.068694~1.090756은 이 식에서 항상 0이 된다.

현재 no-write projection은 exact parent+physical package가 같은 grouped profile의 해당 `dissolve` 채널만
`unbound`로 내리고 `SOURCE_DYNAMIC_PARAMETER_ARITHMETIC_UNAVAILABLE`를 기록한다. target의 결과는
`[uv_pan, unbound, uv_pan, uv_pan]`이며 authoring approximate preview만 허용한다. 전역 HLSL dissolve
공식을 바꾸거나 다른 parent에 이름 추정으로 확대하지 않는다.

이 경계가 적용된 strict occurrence는 214개다. 원천 census 220개 중 Warlord 6개는 checked contract에
`parentSourcePhysicalPackage`가 없어 보수적으로 미적용했으며 다음 evidence/G3 통합에서 exact tuple을
보강하기 전까지 기존 상태를 유지한다.

---

## 3. 현재 수치 — 디스크 baseline과 no-write projection 분리

| 지표 | 기존 101 baseline | no-write projection | 의미 |
|---|---:|---:|---|
| material identity compiled | 769 | 776 | physical package 미해결 identity 9 → 1 |
| material identity admitted / blocked | 587 / 178 | 607 / 165 | parent Material3 evidence 연결 결과 |
| portable Particle | 4,488 | 4,488 | source/module 분모 불변 |
| product-full | 3,481 | 2,793 | exact product admission만 계산 |
| authoring approximate | 0 | 722 | alias 또는 cooked arithmetic 결손, 제품 거부 |
| hard portable fail-closed | 1,007 | 973 | 그릴 계약이 없거나 unsupported인 행 |
| authoring preview target | 3,481 | 3,515 | Full + Approximate |
| source-preserved deferred | 199 | 199 | 이번 authoring gate와 무관 |
| output element | 4,777 | 4,777 | 101 rewrite 전후 총량 불변 |

approximate 직업별 no-write 분포는 Artist 165 / DimensionMaster 43 / LanceMaster 508 / Warlord 6이다.
이 표의 오른쪽 열은 검증용 projection이며 디스크 101문서 또는 제품 mapping 상태를 뜻하지 않는다.

---

## 4. 당시 하지 않은 것 (pre-G3 snapshot, 현재 상태는 1절 우선)

1. **authored 101문서를 다시 쓰지 않았다.** 현재 파일과 restoration receipt SHA는 기존
   3,481/1,007 baseline 그대로다.
2. `materialize_four_class_track_a_candidates.py`의 `EXPECTED_*`를 재기준화하지 않았다. 따라서 default
   `--dry-run`/full pinned suite는 의도적으로 stale denominator에서 실패한다.
3. 당시에는 Claude G3 `authoringOverrides` schema/Codec/Tool/reimport 구현을 통합하지 않았다. 현재는 통합과
   focused 자동검증, Client x64 Debug 빌드까지 완료됐다.
4. 새 projection의 representative/full native gate와 Client Release를 실행하지 않았다.
5. Client/UI를 실행하거나 사용자 대신 visual fidelity를 판정하지 않았다.

---

## 5. 당시 제안한 순서와 현재 남은 단계

### 5.1 Claude G3 코드 통합 — 완료

13 authoring family 표현과 `authoringOverrides.resources/scalars/colors` 소유권을 기존 공용
Codec/Renderer/Playback/Effect Tool에 통합한다. G3는 새 shader 13개를 만드는 계획이 아니며, compiler가
소유하는 source identity/profile과 artist가 소유하는 튜닝 delta를 분리하는 계획이다.

통합 뒤에도 다음은 불변이다.

- override는 admission/exactness를 바꾸지 않는다.
- approximate는 계속 publisher/product mapping에서 거부된다.
- compiler가 새로 만든 resource/profile 위에 유효한 artist override만 마지막에 적용한다.
- 사라진 slot/parameter override만 reason을 남기고 drop하며 문서 전체를 손상시키지 않는다.

### 5.2 Debug Client와 기존 baseline 대표 육안검증 — 교정 빌드 재검증 대기

Client x64 Debug를 빌드한 뒤 사용자가 이 문서 첫 절의 대표 순서로 직접 확인한다. 기존 101 baseline을
사용하므로 이 단계에서 새로운 `authoringApproximate` 버튼 수나 최종 admission 분모를 판정하지 않는다.
확인 목표는 grouped UV, sprite flip, ModelCue OPAQUE, transform/preScale, G3 override 소유권이라는 공용
방향이 실제 화면을 개선하고 Artist F control을 회귀시키지 않는지다. 사용자는 Artist F와 DimensionMaster A를
승인했고 DM T, Lance BA1, DimensionMaster BA3, Warlord 17090을 거부했다. 이후 네 skill focused 교정과
`Boss Patterns > Valtan` index를 구현하고 Debug를 다시 빌드했으나, 같은 다섯 경로의 사용자 재검증은
아직 받지 않았다.

### 5.3 방향 승인 뒤 최종 projection/write/full gate

1. `build_projection(validate_expected_denominators=False)`로 최종 수치를 다시 측정한다.
2. 승인된 수치만 `EXPECTED_*`와 Python tests에 한 번 고정한다.
3. `--dry-run` → transaction `--write` → 연속 `--check` 두 번을 실행한다.
4. FULL/APPROX/HARD receipt 분모와 101문서를 stable source lineage로 전수 join한다.
5. FULL+APPROX는 Stage/runtime/GPU submission witness, HARD는 hidden/reactivation reject+rollback을 증명한다.
6. ClientFrontendHarness/Client Debug·Release와 publisher validation을 실행한다.
7. product mapping은 사용자 occurrence 승인 전까지 0을 유지한다.

### 5.4 그 뒤에도 남는 원인 축

| 대상 | 수 | 성격 |
|---|---:|---|
| `.wmodel` 미확보 mesh | 110 | material 무관. 별도 추출/변환 트랙 |
| procedural texture-less sprite | 650 | `sourceProfile.textures`가 0개. 원본 emitter가 텍스처를 선언하지 않음. parent가 `fx_d_pa_flare_02_ad`, `fx_e_pa_mask_01_ad`, `fx_k_pa_velflow_01_tr` 등 절차적 재질 |
| compile 누락 identity | 87 → 현재 64 | `materialResolutionStatus=RESOLVED_EXACT_SOURCE_PACKAGE`인데 compile 대상에서 빠짐 |
| Cascade module 미지원 | 199 | Ribbon 81 제외 118행. `efparticlemodulelocationemitter` 47 + `particlemodulelocationemitter` 21 + `particlemoduleorbit` 19 + `particlemodulecollision` 10 |
| Decal missing-Base | 33 | 기존 설계대로 사용자가 Base 지정하면 해제 |

650개는 **억지로 white.dds를 넣지 않는다.** 표현할 증거가 없으면 잠근 채로 두고
"원본에 텍스처가 없어 절차적 evaluator가 필요함"을 명시적 사유로 남긴다.

G3가 정한 authoring family는 renderer shape × blend class × SubUV의 13개다. Distortion과 Procedural은
이름으로 새 family를 만들지 않는다. procedural 650행은 texture lane 자체가 없어 추가 DDS 추출로 풀리지
않으며, 실제 evaluator를 구현하거나 계속 hard-lock으로 남겨야 한다.

---

## 5.5 발탄 트랙 — [A]/[C] 완료, [B] 대기 중

4직업과 **파일이 겹치지 않는 별도 트랙**으로 발탄 준비를 끝냈다. 상세 조사와 계획은
[`2026-08-15_VALTAN_PATTERN_EFFECT_RESTORATION_SURVEY_AND_PLAN.md`](2026-08-15_VALTAN_PATTERN_EFFECT_RESTORATION_SURVEY_AND_PLAN.md)가
정본이다.

### 확정된 구조

발탄은 캐릭터와 **같은 animation-notify 기반 Effect ownership**을 쓴다. 원본 추출본의 notify
종류가 그 증거다.

```text
Effect 3,463 / PlayParticleEffect 5,383 / PlayDecalEffect 508 /
Trails 272 / TrailGhostEffect 105
```

`Valtan.animevents`에 `effectref=`가 0건인 것은 원본 notify 부재가 아니라 projection 미생성이다.
따라서 캐릭터 파이프라인을 그대로 재사용하고, 축만 `skillId`에서 `actionId`로 바꾼다.

```text
ValtanEncounter.json        patternId -> gameplay actionId + sourceActionIds
Valtan.patternbindings.json gameplay actionId -> clip
<profile>.action-effects    numeric actionId -> stage -> clip + notify
```

### [A] actionId 축 어댑터 — 완료

`Tools/EffectPipeline/build_valtan_action_bindings.py` →
`Data/Animation/Authored/Valtan/Valtan.actionbindings.json` (+receipt)

```text
patternCount               31
patternsWithSourceActions  31    누락 0
patternsWithStages         29    (§gap 2건)
authoredStageCount        108
effectCueCount          9,175    ← notify occurrence 수. 문서 수가 아니다
unmappedSourceActionCount  89    제품 패턴 밖. 별도 목록으로 보존
missingSourceActionCount    0
```

**`9,175`를 9,175개 독립 Effect asset으로 materialize하지 않는다.**

### [C] source material evidence — 완료

`Tools/EffectPipeline/build_valtan_source_material_evidence.py` →
`Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json` (+receipt)

occurrence 축과 definition 축을 분리한다. occurrence는 `Valtan.actionbindings.json`이 소유하고,
이 문서는 stable identity로 참조되는 정의만 한 번씩 진술한다.

```text
materialDefinitionCount     335    parent 보유 293 / parent declaration 283
distinctParentMaterialCount 123    ← 발탄 material family의 실제 규모
resourceDefinitionCount     855    texture 346 / material 335 / material_parent 123 / mesh 52
sourceSystemDefinitionCount 193    전부 ACTION_BOUND_SOURCE_SYSTEM
patternCoverage           31/31
physicalMissingResourceCount  0    DDS/WModel 물리 누락 없음
```

### typed unresolved — 추측으로 채우지 않았다

```text
PARENT_DECLARATION_NOT_CAPTURED  10   parent 5종
  4  bfx_m.bfx_d_pa_circ_01_ad                       UModel이 empty parameters로 거부
  2  fx_m.fx_k_pa_radialcolor_01_ad                  props 미생성
  1  fx_m.fx_k_pa_radialmask_01_ad                   props 미생성
  3  efmaster_material_prologue.mastermaterial_ch.*  logical package가 inventory에 없음
PARENT_MATERIAL_UNDECLARED        3   MIC인데 parent 필드가 null
PACKAGE_UNRESOLVED_MATERIAL_PATH  1   enginematerials.defaultparticle (engine builtin)
```

`efmaster_material_prologue` 3건은 어느 UPK에 있는지 모르므로 package를 추측해 채우지 않았다.
`empty parameters` 4건은 `RESOLVED_UMODEL_EMPTY_PARAMETERS`로 승격시키는 시도를 했으나
map alias 키가 사라져 다른 material 22건의 parent 조회가 깨지는 **회귀**가 나서 되돌렸다.
현재 상태는 시도 이전과 동일한 283/293이다.

### 애니메이션 담당자 몫 — Effect 세션이 만들지 않는다

원본 cue는 있으나 authored animation stage binding이 없다. 이름이나 순서로 추측해 만들지 않는다.

```text
VALTAN_LEDGE_ROAR       valtan.attack.ledge-roar         cue 39  stage 0
VALTAN_ARENA_BREAK_109  valtan.mechanic.arena-break-109  cue 42  stage 0
```

### [B] 착수 조건

발탄 [B]는 **4직업 `EXPECTED_*` 재기준선과 101 write가 끝난 직후**에 시작한다.
`materialize_four_class_track_a_candidates.py`의 shared baseline을 공유하므로, 두 트랙을 동시에
태우면 분모가 두 번 흔들려 원인을 분리할 수 없다. 그 전까지 발탄 트랙은 정지한다.

### 발탄 트랙이 만든 파일

```text
신규  Tools/EffectPipeline/build_valtan_action_bindings.py
신규  Tools/EffectPipeline/build_valtan_source_material_evidence.py
신규  Data/Animation/Authored/Valtan/Valtan.actionbindings.json (+receipt)
신규  Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json (+receipt)
신규  Data/Effects/Imported/Valtan/Valtan.material-candidate-catalog.json
신규  Data/Effects/Imported/Valtan/Valtan.parent-package-map.json
외부  Resource_LostArk/05_Reports/EffectExtraction/VALTAN/materials/
```

`Client/**`와 `Tools/ClientFrontendHarness/**`는 하나도 건드리지 않았다.
`Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py`만 공유 파일인데,
발탄 작업 중 실수로 `git checkout --`해 4직업 fix(`export_parent_material`,
`SOURCE_MATERIAL_EMPTY_PARAMETERS`, `--parent-package-map`)가 함께 사라졌다가 전부 복원했다.
복원 검증은 focused test 14/14 PASS와 발탄 재추출 426/444 동일 재현이다.

---

## 6. 유지해야 할 경계

- `Product mapping 0`. approximate는 publisher가 거부하므로 구조적으로 gameplay에 들어갈 수 없다.
- generic/white fallback 금지. approximate 자격에 **리소스 하드 조건**(텍스처 1개 이상, mesh는
  WModel 필수)을 걸어둔 이유가 이것이다. 이 조건을 완화하지 않는다.
- family evaluator는 재구성물이다. `graphProvenance=RECONSTRUCTED_GRAPH`, `sourceExactGraph=false`를
  유지하고 exact로 승격하지 않는다.
- DimensionMaster canonical profile 784개를 다른 캐릭터에 추정 복사하지 않는다.
- G3 `authoringOverrides`는 사용자 튜닝 delta일 뿐 source exactness나 product admission을 승격하지 않는다.
- Light, Cascade `TypeDataRibbon`, Camera/ScreenPost는 이번 4직업 ordinary family에서 제외한다.
  AnimationTrail 11과 Character Afterimage 72는 서로도, Cascade Ribbon과도 다른 family다.
- Valtan Dust/Trail/Light와 unresolved Effect/PawnMaterialParam/ViewShake는 notify-006 canary 3개와 분리해
  fail-closed를 유지한다.
- 화면 판정은 사용자 전용이다. 에이전트는 빌드·구조화된 로그·수치 진단·실행 준비까지만 한다.
- `Stage_Document` 성공과 `Query_ParticleRuntimeProbe` 존재는 픽셀 제출 증거가 아니다.

## 7. 롤백 경로

- 이전 contract/receipt: `Data/Effects/Imported/FourClassCombat/BASELINE.*.bak` 두 개.
  이 생성물들은 git untracked이므로 이 백업이 유일한 복원 수단이다.
- 현재 authored 101은 아직 baseline receipt와 byte/canonical identity가 일치하므로 이번 checkpoint의
  가장 안전한 rollback/비교 입력이다. `--write` 전에 이 상태를 유지한다.
- Wave 0 compiler를 코드 한 줄만 제거해 되돌리는 방식은 이후 exact-wrapper/dynamic boundary와 섞여 더 이상
  안전한 rollback이 아니다. 필요하면 두 backup을 명시적으로 비교·복구하고 contract builder `--check`를
  다시 실행한다.
- 현재 authored 문서에는 `authoringApproximate`가 0개이므로 admission 코드 자체를 되돌릴 때 data migration은
  아직 필요 없다. 최종 rewrite 뒤에는 receipt와 101문서를 한 transaction으로 함께 되돌려야 한다.

## 8. 이 세션에서 실행한 검증

- repo catalog 기준 UModel dependency 재추출: 요청 953 / 해결 905 / fail-closed 48 /
  texture 3,571. 도구 계약대로 48이 있어 exit 1이며 crash나 누락 은폐가 아니다.
- repo-local source evidence 재생성: candidate 953 / parent props 901 / fail-closed 52 /
  missing extractor 0, exit 0. evidence JSON SHA는 기존과 같고 receipt만 repo path provenance로 갱신됐다.
- four-class source-material contract `--write`와 `--check`: PASS,
  seed 50 / compiled 776 / admitted 607 / fallback-blocked 165 / no-package 1.
- Lance dynamic arithmetic·approximate focused Python: 39/39 PASS, `py_compile` PASS.
- no-write projection: 2,793 Full + 722 Approximate + 973 Hard = portable 4,488,
  preview target 3,515, output 4,777.
- ClientFrontendHarness x64 Debug compile/link: PASS.
- 3-way Track A cheap focused gate: PASS. Full+Approximate Stage/runtime/GPU witness와 Hard activation
  reject/rollback fixture를 실행했다. 장시간 representative/full gate는 실행하지 않았다.
- `--effect-portable-event-fast`: 5/5 PASS. visible execution target, exact `epet_spawn`, 4,096 상한,
  orphan/`epet_any`/cycle rollback과 approximate route를 검증했다.
- `Test-EffectPipeline.ps1`: PASS, exit 0. approximate publish 거부 뒤 기존 product output byte 보존을 검증했다.
- `--effect-authoring-fast`의 신규 approximate codec 항목 2개는 PASS:
  canonical roundtrip과 invalid flag parse-commit rollback.
- scoped `git diff --check`: PASS(기존 LF→CRLF 안내만).

전체 `--effect-authoring-fast`는 공유 WIP의 DimensionMaster/overlay 등 기존 16항목 때문에 exit 1이다. 신규
두 codec fixture의 PASS와 분리해 기록한다. default materializer `--dry-run`/full pinned suite도 stale
`EXPECTED_*` 때문에 의도적으로 실패 상태를 유지한다.

당시 미실행: G3 코드 통합, 최신 Client 본체 Debug/Release, 101 rewrite/rebaseline, representative/full native
Stage/Draw, Client/UI 실행, 사용자 육안 검증. 현재는 G3와 Client Debug, 사용자 baseline 대표 검증 일부가
완료됐으며 101 rewrite/rebaseline, 장시간 full gate, Release와 product mapping은 계속 의도적으로 미실행이다.
