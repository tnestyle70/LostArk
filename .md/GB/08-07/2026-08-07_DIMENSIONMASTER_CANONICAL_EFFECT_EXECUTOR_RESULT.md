# 차원술사 체험 모드 정본 Effect 실행 변환 결과

## 1. 완료된 실행 경계

다음 단일 경로로 현재 차원술사 정본 12개 스킬을 다시 생성하고 게시했다.

```text
PlayerSkills + skillbindings
-> Action Notify / normalized Particle graph / shared external Modules
-> lossless Imported baseline
-> materialized Authored v11
-> WFX Component
-> Assembly
-> EffectCatalog.runtime.json
-> CEffectCatalog / CEffectPlayback / CEffectDocumentRenderer
```

최종 compile 계약은 16 Effects, 182 Components, 947 Emitters이며 compile identity가 모두 일치한다.
기본 공격은 aggregate와 BA1~BA4를 각각 stable Effect로 게시하고, 나머지는 현재 skill ID를 stable identity로
사용한다. slot 문자는 UI metadata일 뿐 저장 identity가 아니다.

## 2. 원본 데이터 보존 결과

현재 12개 source graph에서 다음 데이터를 캡처했다.

| 항목 | 수치 |
|---|---:|
| source emitter partitions | 618 |
| emitted Elements | 872 |
| source Modules | 10,552 |
| source Distributions | 15,847 |
| source Literals | 36,694 |
| Action cue payload | 477 occurrences / 255,432 bytes |
| unresolved emitter/resource | 0 / 0 |

원본 LOD `Modules[]`의 순서와 중복 occurrence, 다른 package의 음수 import reference를 그대로 유지한다.
공유 모듈은 오염이 아니므로 소유 package가 달라도 원본 배열 순서대로 실행한다.

## 3. 이번에 닫은 코드 결함

### Material profile 재생성

materializer가 이전 Authored나 host-local Material map을 복사하지 않고, 각 current skill의 source receipt,
conversion receipt, class resource manifest에서 profile을 다시 만든다. 현재 Authored 12개와 BA1~BA4는 모두
v11이며 Particle `799/799`에 sourceProfile이 연결된다.

D 2050240의 회귀 계약도 `46 Particle / 21 shared profile groups / specialized shader 14 /
Dynamic Parameter 32 / SubUV 2 / source Module occurrence 518`로 복구했다. 같은 Material object path가 여러
physical package에 있을 때 `sourcePhysicalPackage`가 일치하는 행만 EXACT로 선택한다.

### Vector Field와 다중 reference

LOD occurrence 복제에서 `reference_paths`가 빠지던 버그를 수정했다. module별 overlay가 없어도 global
semantic vector field 목록에서 exact `sourceObjectPath -> assetId`를 찾고, 중복이나 identity mismatch는
fail-closed한다.

- A 2050210: `fx_o_w_01.fx_o_vectorfield_01`, 32x32x32, 32,768 samples,
  SHA-256 `2fdb8d454138eac6d94752ffa1f4402ca888d703d5676faef531a6069df4cd0d`
- F 2050230: `fx_o_w_01.fx_o_vectorfield_02`, 10x10x10, 1,000 samples

`MeshMaterials`처럼 같은 property가 여러 번 나오는 reference는 원본 배열 순서의
`meshmaterials[0/1].objectpath`로 보존해 duplicate literal로 오인하지 않는다.

### T Summon Model Cue

T는 과거 2050510이 아니라 2050500 `dimensionprison`이다. 원본 `PlaySkeletalMesh` payload와 검증된 runtime
binding으로 다음 cue를 생성했다.

```text
model: Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel
clip: sk_swp_dms_00_sk_sk_dimensionprison
start: 0.0s
duration: 131 / 30 = 4.3666667s
local position: UE [0,0,45] -> project [0,0,0.45]
scale: [1.2,1.2,1.2]
asset pre-scale: [0.01,0.01,0.01]
asset pre-rotation: [0,-90,0]
```

실제 WModel clip도 24fps에서 원본 30fps로 재기록했다. 이 바이너리는 팀 관리
`Client/Bin/Resources/Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel`에 있으며 Git
Data 문서에는 binding과 retime receipt를 기록했다.

### 회귀와 자동화

ClientFrontendHarness는 생성 receipt를 엄격히 파싱해 효과별·전체 Effect/Component/Emitter 수와 SHA
compile identity를 runtime catalog와 exact 비교한다. 현재 source semantic exact 회귀는 seeded 162,
seed metadata 103/103, local vector field 4/4와 asset 2종이다. 테스트가 process 외부
`LOSTARK_RESOURCE_ROOT`를 지우던 문제도 scoped environment restore로 수정했다.

Imported baseline은 raw JSON 정본이며 8개 문서는 source Material profile materialization이 필요하다.
해당 상태를 보존한 채 codec round-trip 또는 정확한 pending 거부를 허용하고, 제품 runtime-ready 판정은
Authored 12개 stage와 Assembly compile identity에서 별도로 수행한다.

## 4. 자동 검증 결과

- Python unittest discover: 114/114 PASS
- focused importer/materializer/executor tests: PASS
- Effect component verify: 16 Effects / 182 Components / 947 Emitters / compile identity true
- Effect Publish Validate/Publish: PASS
- ClientFrontendHarness full: failures 0
- `--skill-binding-fast`, `--effect-executor-fast`, `--effect-runtime-fast`, `--effect-imported-fast`: failures 0
- Effect Tool final: code 50 / documents 16 / resources 392 / palette 2,667 / cues 8
- focused ClientFrontendHarness Debug build: compile/link errors 0
- Debug integrated regression `Invoke-BuildAndRegression.ps1 -SkipBuild`: PASS
- ProjectAudit: 72 checks PASS
- Debug Client startup smoke: 12초 생존 PASS

## 5. 완료로 선언하지 않은 경계

이번 결과는 정본 identity, source 보존, materialization, hierarchy, runtime stage와 지정된 exact executor
경계를 닫은 것이다. 차원술사 전체의 픽셀 동일 복원 완료 선언은 아니다.

- 전역 receipt는 `partialRuntimeModuleCount=8,290`, `unsupportedRuntimeModuleCount=2,262`,
  `sourceMaterialPendingEmitterCount=59` 때문에 `sourceExtractionComplete=false`,
  `runtimeExecutionComplete=false`다.
- `EngineMaterials.DefaultParticle/DefaultMaterial` 13행은 UE built-in이어서 외부 UPK Material 후보가 없다.
- T의 2.901422초 `PlaySkeletalMeshMaterialParam` 1건은 semantic/GPU consumer가 아직 없다.
- Light/Camera/Post/Sound와 원본 Material graph 공식의 픽셀 동일 실행은 아직 별도 presentation 경계다.
- dedicated Resource Library UI와 R/T 고정 카메라 GPU A/B는 수동 미검증이다.

따라서 다음 단계는 current R 2050180과 T 2050500의 GPU 캡처를 먼저 확인한 뒤, stable
Component/Emitter/Profile ID로 Material 또는 presentation 차이만 좁혀 수정하는 것이다. 과거 후보 Effect나
PNG 눈대중을 기준으로 timeline, transform, scale을 다시 추측하지 않는다.
