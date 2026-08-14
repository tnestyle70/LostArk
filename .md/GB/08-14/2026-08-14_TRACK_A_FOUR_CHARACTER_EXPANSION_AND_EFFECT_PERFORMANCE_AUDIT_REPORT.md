# Track A 기반 4캐릭터 확장 및 Effect Tool/재생 성능 전수 감사 보고서

작성일: 2026-08-14 KST

감사 기준: 초기 shared dirty worktree 감사와 같은 날 완료된 Track C portable carrier 후속 구현

작업 성격: 본문 성능 감사는 읽기 전용으로 작성했다. 이후 Track C 구현 결과를 이 문서의 checkpoint와
rollout 판정에 문서 동기화했으며 Client/UI는 실행하지 않았다.

## 1. 결론

도화가 F는 Track A의 원본 근거를 하나의 authored v13 문서 안에 typed material과 portable Particle
execution으로 실제 materialize했다. 현재 `effect.artist.skill.31470.unified.effect.json`은 별도 Upgrade
없이 ordinary authored Load/Play 경로가 소비한다. 다만 이 수직 슬라이스의 compiler/materializer와
stable join은 F 전용이고 제품 매핑·4인 동시 재생·Release/runtime·사용자 visual 승인은 남아 있다.
따라서 F 성공을 전체 캐릭터 자동 변환 완료로 확대 해석하면 안 된다.

4캐릭터 확장 전 반드시 닫아야 할 P0는 다음 다섯 가지다.

1. Artist F 전용 importer·분모·basis·material registry를 데이터 기반 공통 compiler/admission으로 일반화한다.
2. `SourceRecipe`가 붙은 MESH/SPRITE/DECAL carrier까지 실제 simulated particle 총량에 포함하도록 validator를 고친다.
3. 제품 spawn hot path의 문서 전체 직렬화·값 복사·동기 resource 생성 가능성을 제거하고 prepared handle만 소비하게 한다.
4. Mesh Particle의 `particle × submesh × pass` draw 폭증을 instancing 또는 엄격한 weighted draw budget으로 막는다.
5. owner 및 scene 전체의 active effect/particle/draw/trail/post/light 예산과 거리·화면 밖 degradation 정책을 만든다.

현재 확인된 All Effects의 1 FPS 원인은 일반적인 ImGui tree 비용이 아니라, F tree가 펼쳐져 있는 동안 약 6.5 MB의 Track A source/material 준비와 DDS/WModel/shader 준비 실패를 매 프레임 다시 수행한 것이었다. 대상 작업에서 tree는 metadata-only로 바뀌었고, 실제 준비는 `Load Seed` 또는 `Upgrade`를 한 번 눌렀을 때만 수행하며 같은 runtime revision의 실패도 latch한다. Debug x64 compile/link와 `git diff --check`는 통과했다. 다만 사용자가 직접 tree를 펼쳐 FPS가 회복됐는지 확인하는 수동 검증은 아직 남아 있다. 최초 명시적 준비 시의 한 번짜리 동기 stall도 아직 가능하다.

핵심 판단은 다음과 같다.

> Track A를 캐릭터별 런타임으로 복제하는 것이 아니라, 원본 identity·transform·material·spawn 근거를 import-time에 typed authored data로 compile하고 제품 재생은 공유된 immutable prepared view만 사용해야 한다. 개별 문서 validator 통과만으로는 4인 동시 프레임 예산을 보장할 수 없다.

### 1.1 같은 날 Track C 후속 구현 동기화

초기 감사 뒤 Artist F authored 정본은 `33` Elements, typed material `28`, FiniteCommon `1`, persistent
fail-closed `4`, portable Particle recipe `29`, module `350`, distribution `564`로 갱신됐다. Mesh/Sprite
Particle의 source color/alpha, size/alignment, DynamicParameter, SubUV와 seed policy가 ordinary authored
runtime으로 연결됐고 current file SHA-256은
`cfd83757e76529bfb82cc48d90724fd7eba32db2ed7a9954fb4a293462fdb94d`다.

상세 구현·검증 정본은
`2026-08-14_TRACK_C_UNIFIED_EFFECT_AUTHORING_RESULT.md`가 소유한다. 이 감사 보고서는 이후 섹션에서
다중 캐릭터 admission·성능·rollout 경계만 소유한다.

## 2. 범위와 용어

현재 저장소에서 “캐릭터 4 확장”은 두 가지로 읽힐 수 있으므로 범위를 분리한다.

| 구분 | 대상 | 이 보고서의 처리 |
|---|---|---|
| 현재 제품 4직업 | Artist, DimensionMaster, LanceMaster, Warlord | 1차 rollout 및 성능 감사의 기준 |
| 장기 6직업 중 Artist+DimensionMaster 다음 4직업 | LanceMaster, Gunslinger, Slayer, Warlord | 같은 admission을 적용할 후속 범위. Gunslinger/Slayer는 현재 product `effectref=asset`이 없어 별도 수직 슬라이스가 필요 |

사용자가 말한 “4”가 추가 4직업을 뜻하더라도 본문의 Track A 주의점과 성능 gate는 그대로 적용된다. 다만 현재 바로 제품 전환을 검토할 수 있는 데이터 분모는 먼저 위의 제품 4직업이다.

## 3. 현재 checkpoint

| 항목 | 현재 상태 | 판정 |
|---|---|---|
| Artist F 저작 UX | Create/Load, Family/Element 선택, Visible/Transform, DDS 교체, Save/Reload, Play All/Solo를 사용자가 이미 확인 | UX 합격 이력 |
| Track C typed execution 코드 | RuntimeMaterialV2/ArtistVisualV4/LocalDecal snapshot, semantic lane, scalar/vector/color/pass/render-state 저장 및 ordinary renderer materialize 구현 | 코드 완료 checkpoint |
| F authored 파일 | v13, 33 Elements/29 Particle, typed 28, portable recipe 29/350 modules/564 distributions | materialize·정량 검증 완료, visual 대기 |
| F 저장 분모 | FiniteCommon 1, persistent fail-closed 4, burst 26행/총 167, MeshParticle pre-scale 13, root 28/follow 5 | 저장 artifact 재계측 완료 |
| 제품 연결 | `PlayerSkills.effectId` 공백, animevent는 원본 `src=orig`, catalog는 기존 reconstructed runtime entry | 아직 제품 전환 전 |
| F tree 1 FPS 수정 | metadata-only tree, explicit preparation, revision failure latch, profiler scope 추가; Debug build/link와 diff-check 통과 | 코드상 수정, 수동 FPS 검증 대기 |
| 최신 Track C 변경 검증 | Debug Client compile/link, portable materializer check, seed 14/14, Effect publisher 102 및 visual program 13/135, scoped diff-check | Release/runtime/GPU A/B 근거 아님 |
| visual fidelity | 사용자 A/B 및 서면 판정 필요 | OPEN |

현재 F 물리 JSON은 offline materializer가 원자 갱신했으므로 사용자가 `Upgrade Current Effect from
Track A`를 실행할 필요가 없다. 반면 일반 Effect Tool Create/Bake/Merge는 portable SourceRecipe를 아직
보존하지 않으므로 다른 캐릭터 확장 전에 이 경계를 공통 compiler로 연결해야 한다.

## 4. 확장 분모 실측

### 4.1 현재 제품 cue 문서

08-09의 4직업 rollout 계약은 51 skills, 74 stages, 113 clip occurrences이며 102개 visual/11개 intentional silent, product cue target 분모는 101개다. 현재 물리 데이터에서 이 cue들이 참조하는 고유 product effect 문서는 98개이고 요소 합계는 2,128개다. 일부 cue가 문서를 공유할 수 있으므로 101 cue target과 98 unique document는 서로 다른 분모다. 이는 동시에 전부 그려진다는 뜻은 아니지만, 캐릭터 초기 준비량과 per-skill draw 후보 규모를 보여준다.

| 직업 | 고유 product 문서 | 요소 | 고유 물리 asset 입력 | 물리 입력 크기 |
|---|---:|---:|---:|---:|
| Artist | 14 | 183 | 109 | 6,079,040 B |
| DimensionMaster | 19 | 333 | 274 | 21,640,436 B |
| LanceMaster | 41 | 1,052 | 210 | 14,253,716 B |
| Warlord | 24 | 560 | 248 | 15,828,312 B |
| 전체 고유 합계 | 98 | 2,128 | 828 = DDS 684 + WModel 144 | 57,046,176 B |

직업별 asset 수를 단순 합하면 직업 사이 중복이 포함되므로 전체 고유 828과 다르다. 위 크기는 물리 파일 입력 크기의 하한이며 decode 후 texture GPU memory, model buffer, sampler, instance buffer는 포함하지 않는다.

현재 product 문서는 대부분 작은 `.authored-baseline` 형태로 평탄화되어 있고 particle element는 0이다. Track A의 넓은 원본 문서를 그대로 제품 문서로 교체하면 현재 비용 특성이 완전히 달라진다.

### 4.2 Track A가 실제로 덮는 범위

현재 Track A V6는 전체 제품 cue 복원층이 아니다. Visual Program 전체는 13 programs/135 rows이며 admitted 72, fail-closed 63이다.

| 범위 | 현재 Track A 상태 | 의미 |
|---|---|---|
| DimensionMaster BA | 75 rows 중 admitted 50, fail-closed 25 | 가장 가까운 2차 후보지만 typed authored execution 이식 전 |
| Artist BA | 33 rows 중 admitted 8, fail-closed 25 | F Core33/Track C 분모와 다른 BA program 분모 |
| LanceMaster BA | 25 rows 중 admitted 12, fail-closed 13 | Ribbon과 AnimationTrail의 소유 경로도 서로 다름 |
| Artist F adapter | 별도 첫 vertical slice | 코드 구현 후 물리 upgrade·A/B·제품 연결 대기 |
| Warlord | executable row 없이 fail-closed canary | 기존 근사 product 문서가 있어도 Track A-ready가 아님 |
| Gunslinger/Slayer | raw animevent occurrence만 존재 | authored/catalog/product asset intake가 아직 없음 |

즉 F 성공을 “4직업 Track A가 이미 준비됨”으로 확대 해석하면 안 된다. 현재 executable program은 DimensionMaster/Artist/LanceMaster BA1~BA4 12개와 Artist F adapter이고, Warlord와 Gunslinger/Slayer는 source intake 단계부터 다르다.

단일 product effect의 큰 예는 다음과 같다.

| Effect | 요소 | Mesh | Sprite |
|---|---:|---:|---:|
| LanceMaster `effect.lancemaster.skill.34590.ba3` | 125 | 73 | 52 |
| Warlord `effect.warlord.skill.17250.clip1` | 97 | 별도 계측 필요 | 별도 계측 필요 |
| Warlord `effect.warlord.skill.17040` | 91 | 별도 계측 필요 | 별도 계측 필요 |
| LanceMaster `effect.lancemaster.skill.34570.clip2` | 75 | 별도 계측 필요 | 별도 계측 필요 |

현재 renderer에서 standalone mesh/sprite는 element별 draw 후보이고 effect 사이 batching이 없다. 그러므로 “문서가 작다”가 아니라 동시 활성 element와 pass/submesh를 기준으로 예산을 잡아야 한다.

### 4.3 전체 authored corpus

현재 authored corpus는 151개 문서, 약 82.36 MB다.

| 직업 | 문서 | 요소 | Particle 요소 | 추가 특징 |
|---|---:|---:|---:|---|
| Artist | 16 | 258 | 29 | particle max 합 286, trail 1 |
| DimensionMaster | 42 | 1,880 | 1,296 | 약 61.17 MB, particle max 합 11,187 |
| LanceMaster | 58 | 1,683 | 0 | trail 8 |
| Warlord | 32 | 909 | 0 | mesh/sprite 중심 |
| Test | 3 | 별도 | 별도 | 제품 rollout 제외 |

특히 DimensionMaster에는 약 9.88 MB/228 elements/218 particles, 약 8.20 MB/209 elements/201 particles인 문서가 있다. 후자는 particle max 합 3,240, spawn-rate 합 2,560, mesh-particle element 58이다. 이 broad authored 문서를 product cue에 바로 연결하면 기존 평탄화 product 문서와 전혀 다른 CPU/GPU 비용이 발생할 수 있다.

### 4.4 Artist F 자체의 현 비용 신호

현재 F 문서는 1,367,152 B, 33 Elements, 29 portable Particle recipes, 350 modules, 564 distributions,
mesh-particle binding 13, 실제 `kind=particle`의 max 합 286, mesh-particle max 합 49, fixed t=0 burst
총 167과 ribbon `maxPoints=500`을 가진다. 원본 곡선과 literal을 JSON에 보존하면서 파일 크기는 초기
235,612 B보다 커졌다. 모든 Element의 기본 particle 필드를 무차별 합하면 1,111이지만 codec의 현재
document particle 합계는 `kind=particle`만 세므로 성능 분모로 사용하면 안 된다. 시작 burst와
source module evaluation을 포함한 live peak는 사용자 실행 계측으로 확인해야 한다.

4개 F가 같은 tick에 시작한다고 가정하면 mesh particle 상한만 196이다. 모델당 submesh가 2개라면 mesh 계열에서만 약 392 draw가 될 수 있고 material pass, static element, decal, ribbon, afterimage는 별도다. 이 수치는 최악 상한을 보여주는 정적 추정이며 실제 live count와 GPU 비용은 사용자 실행 계측으로 확인해야 한다.

## 5. Track A 확장 시 지켜야 할 계약

### 5.1 권위 흐름

확장 후에도 실행 권위는 다음 하나의 흐름이어야 한다.

```text
Track A 원본 evidence/seed (read-only)
  -> import-time typed compiler + capability/admission
  -> 하나의 authored v13 .effect.json
  -> publish/prewarm된 immutable compiled/prepared view
  -> CEffectObject::Stage_Document
  -> Playback / Renderer / GPU carrier
```

Track A sidecar나 reconstructed runtime program을 제품 권위로 되살리거나, Artist별 두 번째 runtime을 추가하면 안 된다. 원본 sidecar는 재현 근거와 rollback 근거로만 남기고 최종 저작·저장·제품 정본은 authored JSON이어야 한다.

### 5.2 Artist F에서 일반화하면 안 되는 값

| F에서 확인된 값/패턴 | 다른 캐릭터에 복사하면 생기는 문제 | 확장 시 필요한 gate |
|---|---|---|
| source root Y -90° | 캐릭터·package별 import basis가 다르면 double rotation 또는 반대 방향 | class/package별 model-import basis와 source-root basis를 별도 측정 |
| combined bone basis 0.01 및 보정 | DimensionMaster 등 generic raw scale에 Artist 보정을 적용하면 크기 붕괴 | root, bone, WModel pre-scale, particle size를 서로 다른 단계로 기록 |
| MeshParticle `modelPreScale=0.01` 13건 | F evidence일 뿐 전역 규칙이 아님 | WModel/export identity별 실제 unit scale evidence |
| Core33 분모 | 원본 Track A 35에는 Light/ScreenPost도 있었고 다른 skill은 family 분모가 다름 | skill별 exact occurrence inventory와 admitted/fail-closed 분모 |
| root 28/follow 5 exactly-once bake | 이동기·지속기·bone-follow skill은 snapshot bake만으로 궤적을 잃을 수 있음 | occurrence별 snapshot/follow/history 선택과 anchor lifetime |
| fixed t=0 burst 26행/167 | 다른 Cascade는 rate, loop, spawn-per-unit, distribution을 쓸 수 있음 | source spawn module/distribution을 추측 없이 보존 |
| RuntimeMaterialV2 18 + ArtistVisualV4 10 + LocalDecal 2 | Artist 31470 전용 backend/분모를 다른 shader family에 끼우면 오재생 | material capability manifest와 backend별 typed compiler |
| LocalDecal six-SRV lane | 다른 decal/VF/MRT/SceneColor/SceneDepth 계약과 동일하지 않음 | 지원하지 않는 lane/pass는 명시적으로 fail-closed |
| F transform-history 특수 경로 | 캐릭터마다 복제하면 O(sample²), string-map, 이중 rebuild 비용 증가 | 가능한 것은 import-time bake, 진짜 follow만 공통 typed runtime |
| F에서 증명한 portable module class 29종 | 전체 imported corpus 51종의 전역 default로 쓰면 나머지 22종 의미를 유실 | class/property/literal별 capability manifest와 explicit fail-closed |
| `Shader_Artist31470*` opcode | 다른 캐릭터의 material graph와 register 의미가 다름 | character 비의존 material IR와 backend capability compiler |

특히 `Bake_ReconstructedMaterialExecutionSnapshots()`라는 이름과 달리 현재 구현은 Artist F의 RuntimeMaterialV2 18/ArtistVisualV4 10 분모를 직접 전제한다. 이름만 일반 함수인 F 전용 코드를 다른 skill에 재사용해서는 안 된다.

현재 authored material backend는 `GENERIC`, `RUNTIME_MATERIAL_V2`, `ARTIST_VISUAL_V4`, `LOCAL_DECAL` 범위다. 새 직업의 register layout, 6개 초과 texture lane, 비연속 texture/sampler register, 새 VF/pass가 이 범위에 맞지 않으면 schema/backend를 먼저 확장하거나 fail-closed해야 한다. enabled `Material.Execution`과 legacy `SourceMaterial`을 동시에 권위로 유지해서도 안 되며, texture override는 배열 위치가 아니라 stable `laneId`로 join해야 한다.

F importer는 현재 `t=0`이고 `min=max`인 fixed burst만 평탄화하며 timed/ranged burst는 거부한다. null UObject distribution을 임의의 0으로 바꾸거나 음수 Sprite start size를 절댓값으로 바꾸면 source 의미가 달라진다. SubUV random image cadence도 source-exact closure가 남아 있으므로 다른 캐릭터를 “비슷한 기본값”으로 채우지 않아야 한다.

### 5.3 occurrence마다 추가로 보존할 정보

다른 캐릭터로 옮길 때 최소한 다음 필드를 source evidence와 authored 결과 양쪽에서 join해야 한다.

- 제품 identity: character class, skill ID, input slot, clip/BA 단계, animevent row, cue ID, effect asset ID.
- 원본 identity: package/export path, emitter/occurrence stable ID, material export ID, renderer family, LOD/module 순서.
- geometry/resource: exact WModel, exact DDS, texture role, channel, color space, filter, address mode, vector field/model subresource.
- material execution: shader/backend capability, semantic lane, scalar/vector/color register, blend/depth/cull/raster state, pass, SceneColor/SceneDepth/MRT 요구.
- transform: source root basis, character/model import basis, unit scale, WModel pre-scale, particle size scale, pivot, local/world/bone space.
- timing/spawn: delay, duration, loop, lifetime, burst list, rate/distribution, spawn-per-unit, deterministic seed와 RNG scope.
- attachment: root/bone/owner/world anchor, snapshot 또는 follow, missing-anchor policy, exactly-once bake 여부.
- cost/admission: static element, sprite/mesh particle max, weighted mesh draw, trail point/subdivision, afterimage, decal, light, post, texture/model bytes.
- 상태: exact/bounded/fail-closed 분류, 실패 이유, source provenance, 자동 검증과 사용자 visual 판정을 분리한 상태.

이 join은 이름·표시 문자열·vector index가 아니라 stable serialized identity로 수행해야 한다. 모르는 ID나 backend를 generic material로 낮춰 “보이게만” 만드는 fallback은 금지한다.

transform bake의 의미 순서는 현재 `TypeDataRotation × Emitter × Cue × Parent`다. bake 뒤 attachment/inheritance를 남기면 double transform이 되고, 반대로 generic authored copy로 낮추면 Renderer/attachment/inheritance/SourceRecipe/SourcePresentation/TypeDataRotation이 빠질 수 있다. 또한 animevent가 소유하는 바깥 cue root/follow와 문서 내부 element snapshot attachment는 별개의 층이므로 하나의 boolean으로 합치면 안 된다.

### 5.4 캐릭터별 첫 canary

| 순서 | canary 목적 | 반드시 먼저 검증할 위험 |
|---|---|---|
| Artist F | 현재 vertical slice 수동 승인 | 저장 분모는 자동 검증 완료. tree FPS, 시작 burst, root/follow, 사용자 A/B |
| 나머지 Artist 대표 skill | 같은 package 안 일반화 확인 | F ID/분모 hardcode 제거, 반복 preparation 없음 |
| DimensionMaster 대표 particle skill | Artist와 다른 scale/basis 및 대형 particle | SourceRecipe 예산, broad 문서 축소/compiled view, mesh particle draw |
| LanceMaster representative Ribbon/Trail | history geometry 경로 | material closure, anchor, tessellation/upload budget; 현재 Ribbon material fallback OPEN |
| Warlord mesh/sprite/decal 대표 | 큰 standalone element 문서 | per-element draw, depth/decal lane, 4인 동시성 |
| Gunslinger/Slayer 후속 | product cue가 없는 새 수직 슬라이스 | skillbinding/animevent/catalog부터 제품 연결을 새로 닫기 |

제품 cue 정본은 `PlayerSkills.effectId`가 아니라 실제 animation clip의 `effectref=asset`, timing, follow/anchor다. 승인된 skill만 catalog와 animevent를 한 transaction으로 바꾸고 실패하면 기존 mapping을 유지해야 한다. 현재 runtime visual program의 `productMutationCount`는 0이다.

Warlord에는 catalog와 active animevent 사이의 별도 reconciliation 항목도 있다. `17820 clip3/4/8`은 catalog에 있으나 현재 active animevent는 clip1/2/5를 사용한다. 이를 자동 orphan 또는 제품 완료로 분류하지 말고 clip mapping gate에서 확인해야 한다. Gunslinger/Slayer는 `effectref=asset`, EffectCatalog, Authored effect와 물리 Effect resource가 모두 0인 intake 단계이므로 기존 문서 교체 작업으로 세면 안 된다.

### 5.5 portable carrier의 공용·전용 경계

| 층 | 현재 상태 | 다중 캐릭터 판정 |
|---|---|---|
| 공용 runtime | v13 SourceRecipe, portable codec validator/helper, Playback module interpreter, Required 기반 SubUV, ordinary material staging | 같은 portable 계약으로 저장된 다른 캐릭터 JSON도 소비 가능 |
| F 전용 import | 31470 stable join, Core33 분모, Artist basis/`0.01`, material opcode/registry, fail-closed order, F offline materializer | 다른 skill에 복사 금지 |
| 확장 P0 | 일반 Copy/Create가 SourceRecipe를 지우고 Bake/Merge가 carrier를 보존하지 않음. 공통 publisher/compiler 미연결 | data-driven identity/material compiler와 Tool roundtrip을 먼저 닫아야 함 |

즉 이번 F 작업은 공통 runtime seam을 증명했지만 공통 importer를 완료한 것은 아니다. 다른 캐릭터는
source inventory에서 지원 class/property/literal, material backend, transform/follow와 cost를 먼저
admit한 뒤 같은 authored 계약으로 materialize해야 한다.

## 6. Effect Tool 병목 전수 감사

### 6.1 확인된 F tree 1 FPS 원인과 수정 상태

| 항목 | 수정 전 | 현재 코드 | 남은 검증 |
|---|---|---|---|
| tree open | 매 프레임 Track A source/material 준비 | metadata-only 표시 | 사용자 환경 FPS |
| resource work | DDS/WModel 59개와 shader 6개 수준의 동기 준비 반복 | `Load Seed`/`Upgrade` 명시 동작에서만 준비 | 최초 cold 준비 시간 |
| 실패 처리 | 다음 프레임 재시도 | runtime revision 단위 FAILED latch | Refresh/revision retry 확인 |
| 계측 | tree scope와 실제 준비 분리 부족 | `EffectTool.ArtistF.MaterialPreparation` scope | p95/p99 및 disk/shader counter |

현재 수정은 지속적인 1 FPS loop를 겨냥한 올바른 방향이다. 그러나 명시 버튼을 누른 첫 프레임에 동기 resource 준비가 몰리는 문제는 별도다. 4캐릭터 확장에서는 UI가 멈춘 것처럼 보이지 않도록 prepare를 bounded job으로 분리하고 progress/cancel/failure state를 보여주는 편이 안전하다.

### 6.2 확장 시 다시 커질 editor-side 비용

다음 항목은 현재 33-row F에서는 작을 수 있지만 98개 product 문서나 151개 authored 문서로 확장하면 병목이 될 수 있다.

1. `Render_AllEffectsWindow()`가 매 프레임 Artist F와 DimensionMaster T unified 파일에 `exists`, `is_regular_file`, `last_write_time`, `file_size`를 호출한다. 지금은 2개 cache지만 effect별 hardcode를 늘리면 파일 시스템 호출도 선형 증가한다.
2. 검색 문자열이 비어 있어도 skill마다 unified elements, product cues, visual-program rows/resources를 반복 순회한다.
3. `Validate_Drawable()`은 현재 unified file의 mtime/size가 바뀔 때 cache refresh에서 실행되고 tree frame마다 재실행되지는 않는다. 이 부분은 확인된 반복 병목으로 잘못 분류하면 안 된다. 다만 저장/변경 시 validation은 84개 resource binding의 path/file 검사까지 동기 수행할 수 있으므로 명시 작업의 hitch로 계측해야 한다.
4. family마다 전체 elements를 `count_if`하고 다시 전체를 순회하므로 대략 `family × element` 비용이다.
5. family node가 `DefaultOpen`이고, row마다 임시 string·summary·tooltip을 만든다.
6. `Render_VisualProgramAuthoring()`도 7개 family를 반복 scan하고 authorable row를 projection lookup으로 확인한다.
7. 긴 row/module 목록에 `ImGuiListClipper`가 없다. 보이지 않는 행도 CPU 작업을 수행한다.
8. F/DimensionMaster T의 문서 경로, cache, 대표 skill 처리와 버튼이 C++에 개별 hardcode되어 있다. 확장 시 effect마다 함수와 state를 추가하면 성능과 유지보수 모두 악화된다.
9. Data Files 화면은 file→skill count, skill별 file grouping, unassigned grouping에서 선형 검색을 반복한다. 현재 Authored 151 + Imported 58과 약 88 skills의 중첩이므로 refresh 시 역색인을 만들지 않으면 확장과 함께 악화된다.
10. 초기 ResourceCatalog/AllEffects/DataFiles index는 한 프레임에 하나씩 동기 수행하고 manual Refresh도 재귀 directory scan/JSON parse를 수행한다. 현재 `Resources/Effect` 약 2,683 files를 UI thread에서 훑을 수 있어 명시적 refresh stall이 가능하다.
11. thumbnail miss는 UI thread에서 DDS/WModel load, model shader/RTV/DSV/white texture 생성과 즉시 offscreen render를 수행한다. “2프레임에 1개”는 한 thumbnail의 시간 상한이 아니므로 큰 WModel 하나가 단일 frame hitch를 만들 수 있다.
12. numeric drag는 값이 움직이는 tick마다 detail draft를 deep-copy하고 전체 preview stage/validation/seek를 수행할 수 있다. resource 변경이면 DDS/WModel 동기 reload까지 이어질 수 있으므로 drag-release commit, debounce, value-only patch가 필요하다.
13. commit/preview 경로는 validation 뒤 `Stage_WorldPreview()`를 동기 실행하며 identity 비교 과정에서 문서를 여러 번 serialize할 수 있다. edit/seek/reload 시 hitch 원인이 될 수 있다.

### 6.3 Tool 예방 계약

All Effects tree는 아래 계약을 자동으로 검증해야 한다.

- expand/collapse 및 selection frame에는 disk read, shader compile, model/texture load, document serialization, world staging이 0이어야 한다.
- file metadata 감시는 effect별 per-frame syscall이 아니라 central index revision 또는 explicit Refresh가 소유해야 한다.
- JSON parse/validate 결과, family counts, lowercase search token, row summary는 document revision별 cache여야 한다.
- tree는 기본 닫힘 또는 virtualized row를 사용하고, 보이지 않는 row는 tooltip·summary를 만들지 않아야 한다.
- Data Files는 refresh 때 `assetId → skill/group/search token` 역색인을 만들고 매 frame 중첩 검색을 하지 않아야 한다.
- numeric drag는 임시 UI 값만 바꾸고 debounce 또는 drag release에서 한 번 commit하며, transform/color 같은 값 변경은 resource restage 없이 patch해야 한다.
- thumbnail CPU index/decode는 background queue로 보내고 GPU upload/render에는 frame별 millisecond budget을 두며 RT/depth/white texture를 공유해야 한다.
- 모든 effect는 data-driven descriptor 한 경로를 사용하고 F/skill별 render 함수·cache·state enum을 추가하지 않아야 한다.
- 준비 실패는 같은 source/catalog revision에서는 재시도하지 않고 Refresh 또는 revision 변경만 retry를 허용해야 한다.
- cold prepare는 tree render에서 수행하지 않고 명시적 job queue에서 stage한 뒤 한 번에 commit해야 한다.

## 7. 제품 재생 및 렌더링 병목 전수 감사

### 7.1 P0: particle admission 허점

현재 document limit은 element 2,048, element당 particle 2,048, 문서 총 particle 8,192다. 그러나 문서 총합 계산은 `eKind == PARTICLE`인 element만 더한다. Visual Program/Track A import는 MESH, SPRITE, DECAL kind를 유지한 채 `SourceRecipe`로 particle simulation을 붙일 수 있으므로 실제 particle carrier가 총 8,192 예산을 우회할 수 있다.

이는 단순 최적화 항목이 아니라 validator correctness 문제다. 모든 simulated carrier를 다음 식으로 admission해야 한다.

```text
simulatedParticle = PARTICLE kind
                 + SourceRecipe-enabled MESH/SPRITE/DECAL carrier

weightedMeshDraw = liveMeshParticle × modelSubmesh × materialPass
```

burst, rate, lifetime, afterimage, decal, trail vertex, light, screenpost도 같은 report에 포함해야 한다. 현재의 8,192는 safety ceiling이지 프레임 예산이 아니다.

### 7.2 P0: Mesh Particle draw 폭증

Sprite particle은 emitter 단위 instanced draw를 사용하지만 Mesh Particle은 active particle마다 `Render_Mesh()`를 호출하고, 그 안에서 submesh마다 material/sampler/pass/draw를 반복한다. typed sampler도 draw마다 device state read/set/restore를 수행한다.

우선순위는 다음과 같다.

1. 동일 model/material/pass의 mesh particle을 GPU instance batch로 묶는다.
2. instancing 전에는 effect별 및 scene 전체 weighted mesh draw hard cap을 둔다.
3. local player/boss telegraph를 보존하고 remote cosmetic의 spawn 수·afterimage·trail을 먼저 줄인다.
4. publisher가 1개 문서뿐 아니라 같은 effect 4개 동시 실행의 peak를 검증한다.

### 7.3 P0: spawn hot path의 직렬화·복사·mutex

`Build_ResourceSignature()`는 문서 전체를 문자열로 serialize한 뒤 hash한다. `Find_Prepared()`는 global prepared-cache mutex를 잡은 상태에서 이 signature를 계산한다. Presentation spawn 경로에서도 prepared 확인이 중복되고, Playback/Renderer/prepared cache가 effect document 전체 사본을 각각 가질 수 있다.

8~10 MB DimensionMaster 문서를 Track A에서 직접 가져오면 cue edge에 대형 문자열 생성, 여러 document copy, global mutex 경합이 겹칠 수 있다. 제품 계약은 다음이어야 한다.

- publish/prewarm 시 immutable content signature와 compiled execution plan을 한 번 만든다.
- runtime cue는 `catalog revision + prepared handle`만 조회한다.
- 네 캐릭터 인스턴스는 document/resource/compiled table을 공유하고 simulation state만 개별 소유한다.
- spawn 중 document serialization, disk I/O, shader compile, texture/model/sampler 생성은 0이어야 한다.
- 실패한 resource signature는 revision 단위 negative cache로 재시도를 억제한다.

### 7.4 P0: scene 전체 예산과 culling 부재

visible effect object는 거리·frustum 판단 없이 BLEND group에 등록되고, renderer는 등록된 object를 모두 순회한다. `g_ActiveEffects`에도 scene-wide active effect/particle/draw cap이 없다. 동일 skill 연타, 여러 player, `stop=natural`의 긴 lifetime이 겹치면 active instance가 누적될 수 있다.

필요한 정책은 다음과 같다.

- owner별 및 scene 전체 active effect, sprite/mesh particle, weighted draw, upload byte 예산.
- local player > boss telegraph/gameplay-critical > remote player cosmetic의 우선순위.
- effect bounds 기반 frustum/distance culling.
- remote/offscreen에 particle spawn, mesh particle, trail subdivision, afterimage, light, screenpost 순의 단계적 degradation.
- budget 초과 시 effect 전체 또는 presentation transaction을 실패시키지 않고 비핵심 renderer family를 명시적으로 suppress.
- suppression 이유와 원래/적용 budget을 profiler에 기록.

### 7.5 Fixed-step와 seek

Playback은 60 Hz fixed step이고 일반 `Update()`는 한 frame에 최대 60 step을 수행한다. 큰 hitch가 나면 남은 accumulator를 버리지 않으므로 다음 frame들도 최대 60-step catch-up을 반복할 수 있다. F tree의 동기 준비 같은 hitch 뒤에 simulation catch-up이 2차 frame drop을 만들 수 있다.

`Seek()`는 0초부터 목표 시각까지 모든 step을 동기로 다시 실행하며 별도 상한이 없다. 긴 effect를 scrub하거나 loop restart할 때 `duration × 60 × elements × particles` 비용이 한 번에 발생할 수 있다. transform-history typed update는 60-step 초과를 fail-closed하지만 일반 update/seek와 제품 hitch 정책은 아직 분리되어 있지 않다.

권고는 Tool의 정확한 deterministic seek와 제품 runtime의 hitch recovery를 분리하는 것이다. 제품은 frame simulation time budget, backlog drop/fast-forward 규칙, long seek checkpoint를 가져야 한다.

### 7.6 매 step/frame CPU allocation과 문자열 해석

정적 감사에서 확인한 반복 비용은 다음과 같다.

- `Rebuild_Frame()` 시작 때 `m_Frame = {}`로 frame vector storage를 버리고 다시 채운다.
- element/particle loop에서 transform과 source distribution을 반복 평가한다.
- source module/property path를 string 조립·검색하고 `m_States[elementId]` hash lookup을 반복한다.
- Dynamic Parameter는 particle×step마다 indexed property prefix를 만든다.
- Local Vector Field는 particle마다 module 탐색, matrix inverse, string key 변환을 수행한다.
- source event dispatch는 최악 `event × element × module`이고 step당 event ceiling은 4,096이다.
- renderer는 매 frame occurrence `unordered_map<string, ...>`과 진단 status string을 다시 만든다.
- trail point를 frame output으로 값 복사하고 capacity 초과 시 `erase(begin())`로 O(N) 이동한다.

해결 방향은 authoring string을 staging 때 numeric index/typed accessor로 compile하고, per-document immutable table과 retained scratch/ring buffer를 쓰는 것이다. production 진단 문자열은 on-demand 또는 sampling으로 제한해야 한다.

### 7.7 Follow/transform history

Presentation service는 매 frame anchor map을 만들고 Playback에 전달한다. `Set_RootWorld()`의 delta 0 full rebuild 뒤 같은 frame에 `Advance_Preview()`가 다시 rebuild될 수 있다. Artist F action edge는 clip 전체 animation 구간의 60 Hz bone sample을 동기 생성한다. 각 sample의 bone pose 평가와 중복 `find_if` 때문에 sample 수에 대해 O(S²)가 될 수 있고, runtime lookup도 선형 탐색과 map 생성을 포함한다.

F adapter attach는 spawn마다 reconstructed execution plan compile, projected document 생성, canonical JSON/SHA와 resource signature 계산을 반복할 수 있다. 이 비용은 tree 1 FPS 수정과 독립적인 “최초 cast” hitch 후보다. clip-local bone history, compiled plan, projected document와 signature는 animation/model/catalog revision별 prewarm cache에 들어가야 하고 spawn은 pointer/revision 검증과 instance state 생성만 해야 한다.

이 경로는 F의 특수 근거를 보존하기 위한 bridge이지 4캐릭터 공통 패턴이 아니다. import-time에 exactly-once bake 가능한 occurrence는 runtime attachment를 비활성화하고, 실제 follow가 필요한 occurrence만 preindexed typed anchor handle과 bounded history buffer를 사용해야 한다.

### 7.8 Ribbon/Trail

Ribbon은 매 frame tessellated points, vertices, indices vector를 만들고 dynamic buffer에 올린다. 현재 Artist ribbon의 `maxPoints=500`, segment당 subdivision 최대 25를 상한 그대로 계산하면 effect 하나당 약 12,476 tessellated points, 24,952 vertices, 74,850 indices가 될 수 있다. 4개면 약 99,808 vertices/299,400 indices per frame이다.

실제 live point는 대개 더 적겠지만 validator는 이 최악 기하량을 admission하지 않는다. ring buffer, retained tessellation scratch, scene-wide trail upload budget, 거리별 subdivision 감소가 필요하다. LanceMaster Ribbon은 material closure와 anchor/history가 아직 OPEN이므로 대량 전환 전에 대표 canary로 닫아야 한다.

### 7.9 Light, ScreenPost, decal과 overdraw

전역 light 64, screenpost 64 제한은 성능 예산이 아니라 transaction 실패 한계다. ScreenPost는 occurrence마다 full-resolution ping-pong pass가 될 수 있고 light도 transient light마다 draw가 추가된다. additive/two-sided transparent sprite와 decal은 draw count가 낮아도 화면 점유율에 따라 fill-rate/overdraw 병목을 만들 수 있다.

확장 시 screenpost는 local owner 중심의 top-N arbitration과 동일 profile merge/dedupe가 필요하다. light는 중요도·거리 예산을 적용하고, 초과분 suppression이 effect 전체 실패로 번지지 않게 분리해야 한다. decal/transparent는 draw count뿐 아니라 GPU timestamp, covered pixels, blend overdraw를 측정해야 한다.

### 7.10 동기 resource 준비와 instance 수명

staging은 vector field, DDS, model과 sampler를 동기로 만들 수 있다. renderer core의 첫 acquire도 shader 6개를 동기 build/compile할 수 있다. 제품은 `CCharacter::Load_EffectCues()`에서 cue 집합을 prewarm하지만 캐릭터 초기화 시점의 main-thread stall 위험이 있고, authored preview/edit 경로는 product prepared-only보다 더 취약하다.

`prepared no-I/O`도 cue spawn의 GPU object 생성을 막지는 않는다. 새 EffectObject의 renderer instance buffer는 비어 있고 attach 시 최대 2,048-capacity particle buffer와 trail buffer를 `Create`할 수 있다. 기존 no-I/O probe는 이 per-instance GPU resource 생성 횟수와 시간을 보지 않는다.

spawn/종료 때는 EffectObject clone/destruction과 active vector erase가 발생하며 pooling이 없다. animated model cue도 instance별 model clone을 가진다. 같은 tick의 4인 burst 시작/종료는 allocation/release spike를 만들 수 있으므로 EffectObject, particle/trail buffer와 animated model state를 capacity class별로 pool하거나 level-load shared dynamic ring/arena를 준비해야 한다. spawn probe에는 disk I/O뿐 아니라 GPU buffer/resource creation count와 시간도 포함해야 한다.

## 8. 현재 계측의 범위와 공백

현재 코드에는 prewarm core/catalog/prepared build, disk load, attach, sync stage, lookup miss 누적 count와 GPU occurrence configured/evaluated/submitted/suppressed/failed count, product spawn 전후 disk-I/O 불변 검사 일부가 있다.

그러나 frame drop 예방에 필요한 다음 계측은 부족하다.

- Effect Tool tree/search/validation/file metadata/prepare 각 scope의 CPU p50/p95/p99와 호출 횟수.
- signature, Stage_Document, seek, fixed step, Rebuild_Frame, renderer submit의 CPU time.
- frame allocation 횟수/bytes와 string/hash-map churn.
- frame당 fixed step 수와 남은 accumulator backlog.
- scene 전체 active effect, sprite particle, mesh particle 및 SourceRecipe carrier 수.
- model submesh draw, material pass, sampler bind, dynamic upload bytes.
- trail vertices/indices, afterimage/decal/light/post pass 수.
- effect BLEND, transparent, decal, screenpost의 GPU timestamp와 overdraw.
- texture/model GPU residency, asset ID별 중복 resource 수.
- owner/거리/중요도별 suppression 원인.
- spawn/종료 p95/p99, pool hit/miss, 종료 후 baseline 복귀.
- 같은 revision 실패의 재시도 및 negative-cache hit 수.

수치 상한은 목표 PC의 cold/warm baseline을 먼저 재고 정해야 한다. 지금 임의의 ms나 draw 수를 PASS 기준으로 선언하면 실제 하드웨어와 화면 점유율을 가릴 수 있다. 대신 위 counter를 publisher/harness 출력의 고정 필드로 만든 뒤, 대표 canary와 4인 worst case에서 측정한 기준을 변경 불가능한 budget manifest로 승격하는 순서가 맞다.

## 9. 필수 자동 검증 matrix

### 9.1 Effect Tool

| 시나리오 | 확인할 것 |
|---|---|
| F tree cold expand/collapse 100회 | tree frame의 disk/shader/model/texture/serialization/stage 0, 지속 FPS 저하 없음 |
| source 없음/깨짐 상태에서 open 유지 | 한 번 실패 후 같은 revision 재시도 0 |
| Refresh 또는 source revision 변경 | 정확히 한 번 retry, 성공/실패 상태 갱신 |
| search empty/nonempty, all family open | cached counts와 virtualized row, frame CPU p95/p99 |
| explicit Load Seed/Upgrade cold/warm | prepare 시간·asset 수·shader 수·실패 rollback, UI job 상태 |
| Save/Reload/Seek/Play All/Solo | 저장 정본 유지, Stage/Seek hitch와 allocation 계측 |

### 9.2 Product runtime

1. 같은 effect 4개를 같은 tick에 spawn한다.
2. cold preparation과 prepared warm spawn을 분리해 측정한다.
3. 최소 30초 동안 시작 burst, steady state, loop, 종료 cleanup을 반복한다.
4. 네 직업의 서로 다른 대표 effect를 동시에 재생한다.
5. rapid skill 반복과 `stop=natural` lifetime overlap을 만든다.
6. on-screen, offscreen, near, far, local, remote owner를 각각 검사한다.
7. sprite/mesh particle, weighted draw, trail geometry, decal/light/post, upload bytes의 scene 합계를 기록한다.
8. prepared spawn 중 disk I/O, shader compile, texture/model/sampler 생성이 0인지 확인한다.
9. budget 초과가 명시적 family suppression으로 끝나고 gameplay-critical telegraph를 보존하는지 확인한다.
10. 종료 뒤 active object, buffer, provider, GPU residency가 정한 baseline으로 돌아오는지 확인한다.
11. large frame delta를 주입해 catch-up/backlog 정책과 다음 frame 회복을 확인한다.
12. 긴 effect seek/loop restart에서 main-thread hitch 상한을 확인한다.

### 9.3 데이터/admission

- stable occurrence/material/resource identity의 중복·누락·fuzzy join 0.
- supported/bounded/fail-closed 분모가 source inventory와 정확히 일치.
- 모든 particle carrier가 총량에 포함되고 mesh weighted draw가 계산됨.
- transform correction이 각 단계에 exactly once 적용됨.
- missing DDS/WModel/material/backend는 generic fallback 없이 fail-closed.
- product mapping 전 기존 cue와 새 cue를 원자적으로 전환·rollback 가능.
- 사용자 승인 전 old Track A/reference를 제거하지 않고, 승인 후 zero-reference를 확인한 뒤 별도 변경에서 retire.

## 10. 권장 rollout 순서

1. 사용자 환경에서 F tree 펼침 FPS가 회복됐는지 확인한다.
2. 현재 F authored 파일의 typed 28/finite 1/fail-closed 4, portable 29/350/564, burst 26·167,
   mesh scale 13, root 28/follow 5와 seed 14/14를 check mode로 재확인한다.
3. 동일 camera/FOV/pivot/sample time으로 Track A와 authored F를 Play All/Family/Solo A/B하고 사용자가 서면 visual 판정을 남긴다.
4. source carrier까지 포함한 performance admission, prepared handle, scene budget/counter를 먼저 구현한다.
5. F의 cold/warm Tool 및 4동시 runtime harness를 통과시킨다.
6. Artist의 두 번째 대표 skill로 F ID/분모 hardcode가 제거됐는지 확인한다.
7. DimensionMaster particle-heavy canary로 scale/basis, large document compile, mesh particle budget을 닫는다.
8. LanceMaster Ribbon canary로 material/history/tessellation budget을 닫는다.
9. Warlord mesh/sprite/decal canary로 large standalone draw와 depth/decal lane을 닫는다.
10. skill 하나씩 catalog/animevent product mapping을 원자 전환하고 매번 rollback과 4인 stress를 검증한다.
11. 네 직업 product rollout 완료 뒤 Gunslinger/Slayer의 missing product cue 수직 슬라이스와 Valtan corpus를 별도 단계로 진행한다.

## 11. GO/STOP 판정

### GO 조건

- F authored 파일의 정량 분모와 portable roundtrip은 맞는다. 사용자 visual A/B는 별도다.
- 사용자가 F visual A/B를 승인했다.
- F tree open은 metadata-only이고 수동 FPS 회복이 확인됐다.
- explicit prepare 실패가 revision 단위로 latch되고 cold/warm 시간이 계측된다.
- Artist 전용 ID/분모/basis/material registry가 data-driven compiler/admission으로 분리됐다.
- SourceRecipe carrier를 포함한 particle/weighted draw validator가 있다.
- product warm spawn에서 disk I/O·shader/model/texture/sampler 생성·문서 serialize가 0이다.
- scene-wide budget/culling/degradation이 있고 4인 동시 harness가 이를 검증한다.
- 캐릭터별 대표 canary가 자동 admission과 사용자 visual 판정을 각각 통과했다.

### STOP 조건

- F 값을 다른 캐릭터의 default basis/pre-scale/burst/material 분모로 복사한다.
- broad Track A 문서를 비용 compile/admission 없이 product cue에 바로 연결한다.
- unsupported material/VF/MRT/SceneColor/SceneDepth를 generic white/standard material로 낮춘다.
- tree open, tooltip, search 또는 selection 중 source/material/resource 준비를 수행한다.
- 개별 문서 `Validate_Drawable()` 또는 particle 8,192 통과만으로 성능 PASS를 선언한다.
- 사용자 visual 승인 전에 product catalog/animevent를 일괄 전환하거나 Track A rollback 근거를 제거한다.
- Debug build 통과를 Release/runtime/GPU/4인 성능 PASS로 확대 해석한다.

## 12. 근거 위치

### 정본/진행 문서

- `.md/GB/08-14/2026-08-14_TRACK_C_UNIFIED_EFFECT_AUTHORING_IMPLEMENTATION_PLAN.md`
- `.md/GB/08-14/2026-08-14_TRACK_C_UNIFIED_EFFECT_AUTHORING_RESULT.md`
- `.md/GB/08-13/2026-08-13_ARTIST_31470_F_ORIGINAL_EFFECT_RESTORATION_IMPLEMENTATION_PLAN.md`
- `.md/GB/08-13/2026-08-13_ARTIST_31470_F_ORIGINAL_EFFECT_RESTORATION_RESULT.md`
- `.md/GB/08-13/2026-08-13_MULTI_CLASS_EFFECT_RESTORATION_V6_IMPLEMENTATION_PLAN.md`
- `.md/GB/08-13/2026-08-13_MULTI_CLASS_EFFECT_RESTORATION_V6_RESULT.md`
- `.md/GB/이펙트복원문서.md`
- `.md/GB/이펙트최종추출.md`

### Effect Tool

- `Client/Private/Effect_Tool.cpp:1932-1980` — frame UI scope와 초기 index step.
- `Client/Private/Effect_Tool.cpp:6063-6410` — F metadata-only tree, explicit prepare, revision failure latch.
- `Client/Private/Effect_Tool.cpp:3915-3922,16283-16307` — numeric drag의 detail draft restage.
- `Client/Private/Effect_Tool.cpp:6402-6525` — per-frame unified file metadata polling과 revision 변경 시 load/validate cache.
- `Client/Private/Effect_Tool.cpp:6650-6810` — cached drawable 결과 소비와 unified family scan.
- `Client/Private/Effect_Tool.cpp:7068-7365` — All Effects per-frame traversal와 F/DM hardcode.
- `Client/Private/Effect_Tool.cpp:8357-8515` — Data Files 중첩 검색.
- `Client/Private/Effect_Tool.cpp:10417-10838,11604-11731` — All Effects/DataFiles/ResourceCatalog 동기 refresh.
- `Client/Private/Effect_Tool.cpp:12528-12688` — Artist F material preparation와 고정 분모.
- `Client/Private/Effect_Tool.cpp:13113-13307` — Track A upgrade join/검증/저장/rollback.
- `Client/Private/Effect_Tool.cpp:15999-16080` — preview staging/identity 비교.

### Playback/Renderer/Product

- `Client/Private/Effect_DocumentCodec.cpp:30-47,4225-4244,4296-4297` — 문서/particle limit와 총합 누락 후보.
- `Client/Private/Effect_Playback.cpp:27-33,1856-1930` — 60-step catch-up와 unbounded seek.
- `Client/Private/Effect_Playback.cpp:1222-1244,1621-1745` — canonical source program 처리와 F adapter attach/compile.
- `Client/Private/Effect_Playback.cpp:3337-3585,3623-3865,3975-4044,4082-4388` — module/string/matrix 평가, trail/afterimage, frame rebuild.
- `Client/Private/Effect_DocumentRenderer.cpp:1587-1599,10285-10309` — document signature serialize와 prepared cache lookup.
- `Client/Private/Effect_DocumentRenderer.cpp:1733-2415` — synchronous DDS/model/material/sampler preparation.
- `Client/Private/Effect_DocumentRenderer.cpp:9836-10023,10440-10582` — buffer/resource staging과 prepared reuse.
- `Client/Private/Effect_DocumentRenderer.cpp:11903-12522` — mesh/submesh draw와 sprite/mesh particle rendering.
- `Client/Private/Effect_DocumentRenderer.cpp:12525-12815` — Ribbon tessellation/upload.
- `Client/Private/Effect_DocumentRenderer.cpp:13412-13850` — per-frame occurrence map, render traversal, status 생성.
- `Client/Private/Character.cpp:232-270` — character cue prewarm 진입점.
- `Client/Private/Effect_PresentationService.cpp:447-915` — anchor map과 F clip history build/lookup.
- `Client/Private/Effect_PresentationService.cpp:1243-2430` — prepared/spawn/active effect/update/cleanup.
- `Client/Private/Effect_Object.cpp:611-616,718-756` — root rebuild와 BLEND group 등록.
- `Engine/Private/Renderer.cpp:620-781` — BLEND 순회와 screenpost pass.
- `Client/Private/Effect_ThumbnailCache.cpp:39-299` — thumbnail miss의 동기 CPU/GPU 작업.

## 13. 검증 경계

이 문서의 성능 본문은 정적 코드·문서·JSON inventory 감사 결과다. 이후 Track C 세션에서 Debug Client
build/link, portable materializer check, seed policy 14/14, Effect publisher Validate와 scoped diff-check를
실행한 결과를 checkpoint에 동기화했다. Client/UI는 실행하지 않았다.

따라서 다음은 아직 주장하지 않는다.

- F tree FPS 수동 PASS.
- 4캐릭터 동시 runtime CPU/GPU 성능 PASS.
- Release build 또는 실제 제품 cue spawn PASS.
- 화면 visual fidelity 또는 occurrence 완성 PASS.

F JSON 분모/portable projection은 Track C RESULT의 자동 검증으로 승격됐다. 나머지 네 항목은 위
rollout과 검증 matrix를 실행한 뒤에만 RESULT로 승격할 수 있다.
