# 2026-08-14 Winters-style Effect Authoring 통합 구현 계획

기준일: 2026-08-14

이 문서는 기존 Track C source-backed/Visual Program 실행 계획을 대체한다. 최종 저작·저장·미리보기 정본은 하나의 authored `.effect.json`이다. Track A는 WModel, DDS, family, timing, transform뿐 아니라 지원 가능한 material/particle 실행 정보를 import 시점에 authored data로 정규화하는 read-only 입력이다. 저장 뒤 runtime은 Track A sidecar나 reconstructed 객체를 다시 조회하지 않고 그 JSON만 소비한다.

## 2026-08-14 사용자 검증 후 material 실행 계약 교정

사용자가 Artist F의 33-Element authored Effect에 대해 Create/Load, Family/Element 선택, Visible,
Transform, DDS slot 교체, Save/Reload, Play All/Solo를 직접 검증했다. 저작 흐름은 합격했지만,
generic `effect.standard`로 낮춘 결과는 이전 Track A bounded replay와 비교할 수 없을 정도로 형상이
무너졌다. 원인은 DDS 파일이나 Family tree가 아니라, import 과정에서 RuntimeMaterialV2,
ArtistVisualV4, LocalDecal six-lane packet, particle burst와 attachment 실행 의미를 제거한 데 있다.

따라서 이 절은 아래 기존 문장을 교체하는 최신 정본이다.

- Track A를 normal runtime의 외부 sidecar/projection 권위로 되살리지 않는다.
- Track A에서 회수한 **실행 가능한 bounded material 조리법**은 import 시점에 typed authored data로
  정규화하여 각 Element가 소유한다.
- authored `.effect.json`은 WModel/DDS 경로뿐 아니라 material backend/opcode, pass와 render state,
  semantic texture lane, texture register와 sampler register, source channel/swizzle, color space와 address,
  scalar/vector/color constant, particle burst/local-space와 필요한 attachment를 함께 저장한다.
- normal preview와 저장 후 재생은 계속 `CEffectObject::Stage_Document` 한 경로만 사용한다.
  `CEffectDocumentRenderer`는 authored typed material을 기존 RuntimeMaterialV2/ArtistVisualV4 GPU packet으로
  materialize한다. 두 번째 Effect renderer, 두 번째 Model runtime, 실행 중 source receipt/SHA join은 만들지 않는다.
- Effect Tool은 내부 opcode/mask를 일반 사용자에게 노출하지 않는다. `Diffuse`, `Height`, `Normal`,
  `Dissolve`, `Specular`, `Emissive` 같은 semantic slot과 안전한 named parameter만 편집하게 하고,
  `t#`, `s#`, channel, opcode와 pass는 Advanced read-only 정보로 보존한다.
- 회수 근거가 없는 native VF/MRT/source module은 추측하지 않는다. 해당 occurrence는 명시적인
  bounded approximation 또는 fail-closed로 남기고 PNG는 최종 위치·크기·색·밀도 튜닝에만 사용한다.

최종 실행 흐름은 다음 하나다.

```text
Track A / Imported evidence
→ import-time typed material compiler
→ authored Element { family + resources + material execution + particle + transform/attachment }
→ one authored .effect.json
→ CEffectObject::Stage_Document
→ CEffectPlayback + CEffectDocumentRenderer
→ existing RuntimeMaterialV2 / ArtistVisualV4 / generic HLSL branches
```

### Artist F 첫 수직 슬라이스

Artist F Core33의 최신 실행 분모는 RuntimeMaterialV2 계열 `18`, ArtistVisualV4 `10`, FiniteCommon `1`,
명시적 fail-closed `4`다. source semantic fidelity는 cache-semantic exact `7`, bounded `22`, fail-closed `4`이며,
여기서 exact는 Lost Ark native VF/MRT 완전 동일이 아니라 회수된 bounded shader 식과 data가 일치한다는 뜻이다.

첫 구현과 사용자 A/B 판정 순서는 다음과 같다.

1. Particle 29개 중 `spawnRate=0 && burstCount=0`인 26개의 source fixed burst를 복원한다.
   현재 evidence의 fixed `t=0` burst 합계는 `167`이며 Artist F에서는 기존 authored burst 필드로 표현 가능하다.
2. RuntimeMaterialV2 계열 `18`개와 ArtistVisualV4 `10`개, 합계 `28`개를 typed authored material로 이관한다.
3. LocalDecal `#20/#21`의 `t0~t5`, `s5~s10`,
   `HEIGHT/DIFFUSE/DISSOLVE/NORMAL/SPECULAR/EMISSIVE`,
   `B/RGBA/G/BA/RGB/R` channel 계약을 첫 six-lane acceptance로 삼는다.
4. MeshParticle `13`개의 WModel carrier import scale `0.01`을 저장하고 ordinary renderer가 소비하게 한다.
   root `28`개와 follow `5`개의 source basis는 현재 emit-start transform에 이미 bake되어 있으므로 attachment를
   다시 켜지 않고 정확히 한 번만 적용한다. live bone follow와 Ribbon history 복원은 이번 pass 범위 밖이다.
5. FiniteCommon `#17`은 기존 finite material profile을 보존하고, `#1/#16/#26/#33`은 Visible을 다시 켜도
   generic/white fallback으로 그려지지 않는 persistent fail-closed marker로 막는다.
6. DDS lane 교체와 Details 수정 후 Save/Reload해도 backend/opcode/pass와 다른 lane이 유지되는지 검증한다.
7. 같은 camera, FOV, resolution, class, pivot, sample time에서 이전 Track A occurrence Solo와 새 authored
   `Stage_Document`를 비교한 뒤 Family와 전체 Effect를 비교한다. 마지막에만 original PNG와 비교해 튜닝한다.

이 교정으로 아래의 “native material packet을 복사하지 않는다”, “LocalDecal six-SRV를 generic slot으로
downgrade한다”, “generic standard material만 소비한다”는 문장은 historical first-pixel 단계 설명으로만
남고 Artist F parity 및 이후 Track A import의 최종 완료 조건으로 사용하지 않는다.

## 현재 구현 상태

2026-08-14 기준 G01 단일 저작 트랜잭션, G02 Track A seed importer와 Artist F의 typed material·portable Particle 첫 수직 슬라이스를 구현했다.

- `New Effect`는 빈 Current Effect를 메모리에만 만든다.
- `Create Element`는 선택 Family의 Element를 Current Effect에 추가하고 자동 선택하지만 파일을 쓰지 않는다.
- WModel/DDS slot, Visible, Effect Details, Delete는 모두 Current Effect를 `UNSAVED`로만 만든다.
- `Save Changes`만 열려 있는 Detail draft를 적용하고 Effect JSON 전체를 atomic save한다.
- Current Effect preview는 호출 모드로 분리되어 항상 generic `Stage_Document`를 사용한다. Product의 read-only source preview만 명시적으로 Visual Program projection을 허용한다.
- 새 Particle은 결정적인 one-shot burst, 새 Decal은 양수 projector volume, 새 Trail은 이동/history가 생기는 기본값으로 시작한다.
- 일반 Visual Program과 Artist F Core33은 `Track A Element Seeds`에서 `Load Seed`로 Element Draft에 내려온다.
  Artist F importer는 이전 reconstructed preparation을 import 시점에 한 번만 읽어 RuntimeMaterialV2 계열 `18`,
  ArtistVisualV4 `10`의 실행 snapshot을 Element에 저장한다. FiniteCommon `#17`은 finite profile을 유지하고,
  `#1/#16/#26/#33`은 persistent fail-closed로 저장한다.
- admitted seed의 identity/target이 깨지면 generic fallback으로 숨기지 않고 실패한다.
- 빈 Effect 또는 모든 content가 `Visible=false`인 Effect는 structurally valid draft로 저장할 수 있지만 drawable로 표시하지 않는다. Hidden partial Element는 다른 visible Element의 preview를 막지 않는다.
- New Effect는 항상 Complete preview scope로 시작하며, Play Family는 기존 Visible 값을 보존한다.
- Resource browser의 Character/Category 변경은 loaded seed와 이미 고른 slot을 지우지 않는다.
- unsaved authored Effect의 `Reload Saved...`는 Save / Discard / Cancel 선택창을 사용한다.
- 마지막 필수 DDS/WModel binding으로 partial draft가 drawable이 되는 순간 Complete preview를 자동 시작한다.
- 사용자가 fresh Sprite, SpriteParticle, MeshParticle, LocalDecal, Trail의 Element 생성과 resource slot, Visible/Transform, Apply, Save, Play All/Solo, Reload Saved 흐름을 수동 확인했다.
- `effect.test.winters2.effect.json` 실측으로 한 authored Effect 안에 같은 MeshParticle Family의 Element 두 개가 함께 저장되는 것을 확인했다. Effect Detail은 한 번에 선택한 Element 하나를 편집하는 inspector이고, 저장·재생 단위는 Current Effect 전체다.
- Artist F의 저장된 33-Element authored 부모가 있었지만 All Effects가 DimensionMaster T만 부모로 인정해 숨기던 UI 분기를 교정했다. Artist F는 이제 `Editable Skill Effect` 부모를 먼저 Load/Play하고, 그 아래 `Track A Element Seeds`를 재료로 Load하여 같은 부모에 Create/Save한다.
- Data Files Normal UI는 Character domain 아래 `[input] skill name -> saved Effect`로 묶고 내부 Effect ID는 tooltip으로 내렸다. Imported/WFX/Runtime 문서는 `Advanced Diagnostics`로 격리했다.
- WModel/DDS binding UI는 Effect Tool의 `Selected Element Resources` 한 곳이 소유한다. Effect Detail은 numeric/material/Visible tuning만 소유한다.
- authored material execution schema/codec, ordinary renderer materialization, particle burst 이식,
  WModel `modelPreScale`, fail-closed suppression, Effect Details typed lane UI를 구현했다.
- 현재 Artist F 물리 문서는 Particle `29`개의 portable `SourceRecipe`, module `350`, distribution `564`,
  literal `1,415`를 한 JSON에 저장하며 ordinary `CEffectPlayback`이 지원 manifest 범위에서 직접 평가한다.
- authored Mesh/Particle shader는 `Material.Execution`의 ArtistVisualV4 opcode가 있으면 reconstructed
  preview flag 없이도 동일 bounded material branch를 소비한다.
- MeshParticle은 WModel carrier `modelPreScale=0.01`을 유지하되, ordinary playback이 요구하는
  dimensionless Start/End Size를 source 값에서 복원하여 이중 `0.01` 축소를 제거했다.
- SpriteParticle은 source occurrence별 6x6/2x2 SubUV, HDR alpha carrier, procedural glow color와
  DynamicParameter endpoint를 authored Detail로 이식한다. Flow02 `#13/#14`는 `1,0,1,1 -> 1,2,1,1`,
  SPLA `#25/#29`는 Y `1 -> 0.5`와 occurrence별 Z `0/1`을 editable bounded carrier로 사용한다.
- All Effects render loop의 Artist F preparation 재시도와 매-frame drawable validation을 제거하고,
  파일 cache를 bounded poll로 전환했다. Current Effect/Family/seed tree는 필요한 노드만 펼친다.
- Client x64 Debug compile/link를 통과했다. 새 Artist F material 결과의 화면 판정은 사용자 수동 검증 전이다.

현재 물리 `effect.artist.skill.31470.unified.effect.json`은 전용 offline materializer로 이미 갱신했다.
사용자의 Element ID, Visible, Transform, DDS binding과 LocalDecal 편집을 보존한 채
`typed28 + FiniteCommon1 + failClosed4`, fixed burst `26/167`, MeshParticle model import scale `13`,
dimensionless Mesh size, attachment disabled/basis baked exactly-once, occurrence별 Dynamic carrier를 저장했다.
같은 materialize 명령을 두 번 실행해 물리 SHA가 변하지 않는 것도 확인했다. 정상 사용자는 더 이상
`Upgrade` 버튼을 누르지 않고 `All Effects -> Artist -> F -> Play All/Family/Solo`로 검증한다.
LocalDecal `#20/#21`의 six-role lane은 더 이상 evidence 카드에만 남지 않고 authored `material.execution`에 저장되어
ordinary `Stage_Document`가 직접 소비한다.

## 2026-08-14 세션 종료 인계

이 세션에서 Artist F를 대상으로 다음 원인과 수정 경계를 확정했다.

| 증상 | 직접 원인 | 반영한 수정 |
|---|---|---|
| F tree 확장 시 7 FPS와 ImGui 입력 정지 | Render 경로가 source material bake와 DDS/WModel prewarm 실패를 매 frame 재시도 | preparation을 명시 action으로 이동하고 revision-keyed Ready/Failed latch, cached drawable, collapsed Family/seed tree 적용 |
| 새 schema 뒤 Effect Catalog typed-codec SHA stale | default DynamicParameter/model pre-scale 필드를 legacy 문서에도 무조건 serialize | optional/non-default 값만 기록해 기존 projected document byte identity 보존 |
| LocalDecal six-lane stage 실패 | 실제 `fx_d_normal_078.dds`는 ATI2/BC5인데 BC3/BA로 고정 | BC5/RG로 renderer, shader, Visual Program generator/parser 계약 통일 |
| MeshParticle가 전혀 보이지 않음 | source size에 이미 적용된 `0.01`과 WModel `modelPreScale=0.01`을 ordinary playback에서 다시 적용 | Mesh Start/End Size를 dimensionless 값으로 복원하고 carrier pre-scale은 한 번 유지 |
| Sprite가 거대한 흰 카드이거나 일부 occurrence가 사라짐 | particle color/alpha, SubUV atlas, DynamicParameter carrier 누락 | occurrence별 6x6/2x2 atlas, HDR alpha, bounded color와 Dynamic endpoint 저장 |
| Mesh Family와 Play All이 object-local failure로 종료 | ArtistVisualV4 opcode 7의 Dynamic carrier가 0 | `#13/#14` X/Y/Z/W endpoint와 consumed mask 복원 |
| 일부 Sprite가 clip/희미함 | ArtistVisualV4 opcode 6의 Y/Z carrier와 source alpha 의미 누락 | `#25/#29`의 bounded Y/Z endpoint와 mask, 관련 alpha carrier 복원 |
| Ribbon 두 번째 point에서 Play All 전체 정지 | generic point는 `color/dynamic mask=0/0`, renderer는 stale exact `0x08/0x0F` 요구 | authored opcode 9는 alpha carrier `0x08`, Dynamic `0`; renderer는 authored consumed mask를 검사하고 source-backed는 기존 exact mask 유지 |

저장·저작 UX는 한 Current Effect 안에 여러 Family/Element를 두고 선택한 Element 하나만 Effect Details에서
편집하며, `Save Changes`가 문서 전체를 원자 저장하는 계약이다. `Play All`, `Play Family`, `Solo`는 저장값을
바꾸지 않는다. 숨은 Upgrade 절차는 정상 사용자 흐름이 아니며, materialized JSON은 일반 Load/Play로 연다.

이 문서 동기화 도중 새 통합 세션이 4직업, Valtan Whirlwind와 성능 작업을 시작했다. 따라서 이 세션은
추가 C++/JSON/빌드 작업을 중단한다. 아래 RESULT의 “검증 완료 기준”과 “후속 세션 소유 현재 snapshot”을
구분하고, 후속 세션은 자신의 변경 뒤 JSON hash/build/manual gate를 새로 갱신한다.

## 0. 목표와 완료 경계

사용자는 Effect Tool에서 다음 한 흐름만 사용한다.

```text
New Effect
→ Create Element Draft
→ Mesh / Sprite / Mesh Particle / Sprite Particle / Local Decal / Trail-Ribbon 선택
→ WModel과 DDS slot 선택
→ Effect Details에서 Visible, Transform, Timing, Color, UV, Particle/Decal/Trail 값 튜닝
→ Save Changes
→ 같은 Effect 안의 Family 트리에 저장된 Element로 표시
→ Play All / Play Family / Solo
→ Reload Saved
```

Element는 별도 파일이 아니다. 한 Effect의 모든 Element와 Model/Summon은 하나의 `Data/Effects/Authored/<effectAssetId>.effect.json`에 저장한다. Save는 선택 Element만 덮어쓰는 것처럼 보이더라도 실제로는 Active Document 전체를 원자 저장한다.

완료 조건은 다음과 같다.

- 새 빈 Effect를 Normal UI에서 만든다.
- 리소스가 없는 Element Draft를 먼저 만들 수 있다.
- Draft에서 family별 slot을 선택하고 DDS/WModel을 바꾼다.
- Visible과 Details 변경을 live preview로 확인한다.
- `Save Changes`가 열린 Draft를 적용하고 Effect 전체를 원자 저장한다.
- 저장 성공 뒤 Element가 해당 Family 아래에 안정 ID로 남는다.
- `Reload Saved` 뒤 family, visible, slot, detail이 동일하다.
- Full, Family, Solo는 저장 데이터를 바꾸지 않는다.
- Track A Element는 같은 Draft 계약으로 import되며 별도 runtime을 켜지 않는다.
- 실제 skill animation과 anchor에서 현재 authored Effect를 preview할 수 있다.

원본 Lost Ark shader/native Cascade와 완전 동일하다는 주장은 완료 조건이 아니다. 현재 renderer가 표현할 수 있는 family, generic material과 지원되는 typed material/portable Particle capability 안에서 알아볼 수 있는 초기 형태를 만들고 사용자가 눈으로 튜닝하는 것이 목표다.

## 1. 현재 실측

### 1.1 일반 스킬 실행 경로

```text
PlayerSkills + skillbindings
→ animation clip
→ animevents effectref=asset
→ EffectCatalog
→ authored Effect Document
→ CEffectObject::Stage_Document
→ CEffectPlayback
→ CEffectDocumentRenderer
```

이 경로는 유지한다. 기존 authored Effect가 흰색·회색으로 보이는 원인은 경로 자체보다 material/texture channel/blend/emissive/dissolve와 particle spawn 값을 불완전하게 내린 데이터에 있다.

### 1.2 현재 Tool의 저장 시점 불일치

현재 `Render_MeshAuthoringWorkbench()`은 Base DDS와 필요한 WModel을 먼저 선택해야 `Create New Effect` 또는 `Add to Current Effect`를 누를 수 있다. `Try_CreateMeshEffect()`는 Element 추가와 `.effect.json` 저장을 한 번에 수행한다.

반면 기존 Element의 Details, Visible, slot 교체, Delete는 Active Document를 dirty 상태로 만들고 별도 `Save Changes`를 요구한다. 같은 화면에서 새 Element와 기존 Element의 저장 의미가 다르다.

새 계약에서는 Create/Add의 즉시 disk save를 제거한다. 모든 변경은 먼저 Active Document에 commit되고, `Save Changes` 하나만 disk 저장을 소유한다.

### 1.3 Historical: materialize 이전 Artist F unified 문서가 보이지 않았던 원인

이 절은 2026-08-14 오전의 구 산출물에 대한 원인 기록이다. 당시
`effect.artist.skill.31470.unified.effect.json`은 33 Element를 로드했지만 Particle 29개 중 26개가
`spawnRatePerSecond=0`, `burstCount=0`이었고, 모든 Element의 `SourceRecipe.enabled=false`라 generic
playback에서 해당 26개가 생성되지 않았다.

당시에는 source material 이름과 `effect.ue3.reconstructed-standard.v1` 문자열만 남고 Track A material
packet이 ordinary `Stage_Document`에 없어서 migration reference에 불과했다. 이 상태는 현행 정본이 아니다.
현재 Artist F authored 문서는 typed `Material.Execution`, fixed burst, WModel pre-scale와 Particle `29`개의
portable `SourceRecipe`를 한 JSON 안에 materialize한다. source module/distribution의 공통 compiler 확대는
후속 통합 세션 소유이며, 이 historical 절을 현재 denominator로 사용하지 않는다.

### 1.4 현재 재사용 가능한 코드

- `EFFECT_DOCUMENT_DESC`와 `EFFECT_ELEMENT_DESC`
- `CEffectDocumentCodec::Load`, `Validate`, `Validate_Drawable`, atomic save
- `CEffectObject::Stage_Document`
- `CEffectPlayback`의 generic particle/trail simulation
- `CEffectDocumentRenderer`의 Mesh/Sprite/Particle/Decal/Trail draw
- Effect Tool의 Current Effect tree, Family/Element Solo, Details, resource browser
- `CModel -> CMaterial` 통합 WModel 경로

두 번째 Effect runtime, 두 번째 Model runtime, literal Winters `.wfx` loader는 만들지 않는다.

## 2. 최종 데이터 계약

첫 구현에서는 기존 v13 `.effect.json` 구조를 재사용한다. 새 포맷과 version migration을 먼저 만들지 않는다.

Family는 다음 deterministic 규칙으로 분류한다.

| Family | 저장 표현 |
|---|---|
| Mesh | `kind=mesh` + `meshModel` |
| Sprite | `kind=sprite` + Base DDS |
| Mesh Particle | `kind=particle` + `meshModel` |
| Sprite Particle | `kind=particle` + Base DDS, `meshModel` 없음 |
| Local Decal | `kind=decal` + Base DDS |
| Trail / Ribbon | `kind=trail` + Base DDS |

Family 목록을 별도 JSON 배열이나 vector index로 저장하지 않는다. Family 트리는 Active Document의 Element를 위 규칙으로 매번 그룹화한다.

각 Element가 소유하는 최소 데이터는 다음과 같다.

- stable `elementId`
- display name
- family를 결정하는 kind/carrier
- `visible`
- Resources-relative `slotId -> assetId`
- standard material template와 render profile
- 선택적인 typed `material.execution` backend/opcode/pass/semantic texture lane와 constants
- 지원 capability만 담는 portable Particle `sourceRecipe` module/distribution/literal
- local position/rotation/scale/velocity/revolution
- color, emissive, dissolve, UV
- start delay와 lifetime
- family별 Mesh/Sprite/Particle/Decal/Trail detail
- 선택적인 root/weapon/bone attachment

Normal authored runtime은 다음을 소비하지 않는다.

- reconstructed runtime program
- Visual Program projection/token
- occurrence SHA admission
- source-authoring overlay
- shader-cache/DXBC replay
- receipt 기반 runtime gate
- native source graph 객체, compiler/admission receipt와 지원 manifest 밖 module

Track A의 full source 자료는 import 근거로 보존하지만 final `.effect.json`에는 renderer가 실제 소비하는
typed material과 portable Particle normalized 값만 넣는다. source 이름은 tooltip용 provenance로만 허용한다.

## 3. Element 저작 상태와 명령 계약

### 3.1 New Effect

`New Effect`는 고유 effectAssetId와 display name을 가진 빈 Active Document를 메모리에 만든다. 디스크 파일은 아직 만들지 않는다. 기존 ID가 disk에 있으면 거부하고 기존 Effect를 Load하도록 안내한다.

빈 문서는 drawable하지 않아도 정상 Draft다. Preview는 숨기고 편집은 계속 허용한다.

### 3.2 Create Element Draft

사용자가 family를 고르고 `Create Element`를 누르면 fresh generic defaults를 가진 Element를 Active Document에 추가하고 즉시 선택한다.

- Base/WModel이 없어도 Draft 생성은 허용한다.
- family별 필요한 resource가 없으면 `Validate_Drawable`만 실패하고 Active Document 구조는 유지한다.
- 새 Element는 Current Effect tree에 `[UNSAVED]`로 보인다.
- Effect Details와 Selected Element Resource Slots가 바로 이 Element를 편집한다.

Track A import도 동일한 명령을 사용한다. 차이는 fresh defaults 위에 허용된 source 초기값을 채운다는 점뿐이다.

### 3.3 Resource slot 선택

slot 카드는 현재 선택 Element 하나만 편집한다.

- Mesh/Mesh Particle: `meshModel`, `base`, `noise`, `mask`, `emissive`, `dissolve`
- Sprite/Sprite Particle: `base`, `noise`, `mask`, `emissive`, `dissolve`
- Local Decal: generic 수동 Element는 `base`, `noise`, `mask`, `emissive`, `dissolve`를 사용한다. Track A
  `#20/#21`은 `HEIGHT/DIFFUSE/DISSOLVE/NORMAL/SPECULAR/EMISSIVE` six-lane execution을 authored JSON에
  저장하며, Effect Tool은 semantic lane의 DDS override를 편집한다.
- Trail/Ribbon: `base`, `noise`, `mask`, `emissive`, `dissolve`

리소스 선택은 Active Document memory에 commit하고 dirty로 표시한다. disk save는 하지 않는다. 잘못된 domain, file kind, slot은 기존 Document와 preview를 유지하고 거부한다.

### 3.4 Effect Details와 Visible

Element 선택 시 committed Element 전체를 Detail Draft로 복사한다. ImGui는 Draft만 수정한다.

- Visible은 저장 필드다.
- Visible off는 Full/Family/Solo 모두에서 제출하지 않는다.
- Solo는 preview filter일 뿐 Visible 값을 바꾸지 않는다.
- Details 변경은 live preview를 stage한다.
- `Apply`는 Draft를 Active Document memory에 반영한다.
- 다른 Element 선택 전에 Apply/Revert를 요구한다.

Normal UI에서는 선택 family와 무관한 source compiler, native material evidence, admission, SHA를 숨긴다.

### 3.5 Save Changes

`Save Changes`는 다음을 하나의 transaction으로 수행한다.

```text
열린 Element/Model/Particle Draft 적용
→ Document Validate
→ drawable 여부 계산
→ authored path resolve
→ temp serialize/write
→ round-trip load/compare
→ stale-writer 비교
→ atomic replace
→ baseline/dirty 갱신
→ family tree와 preview 재동기화
```

필수 resource가 없는 partial Draft는 구조 저장을 허용하되 preview와 product publish는 막고 이유를 표시한다. 새 Element는 별도 파일을 만들지 않고 이 Effect 문서 안에 저장된다.

### 3.6 Load Element와 Reload Saved

`Load Element`는 선택과 Draft 초기화만 수행한다. 파일을 다시 읽거나 resource를 다시 생성하지 않는다.

`Reload Saved`는 disk의 Effect 전체를 parse → validate → stage한 뒤 한 번에 교체한다. unsaved work가 있으면 명시적인 discard 없이는 거부한다. 성공하면 첫 Element 또는 기존 stable Element ID를 다시 선택하고 Full preview를 재시작한다.

### 3.7 Delete Element

Delete는 선택 Element를 Active Document memory에서 제거하고 dirty로 표시한다. 즉시 파일을 덮어쓰지 않는다. `Save Changes` 성공 뒤 disk에서 제거가 확정된다. 삭제 실패 시 selection, Document, preview를 보존한다.

## 4. Track A import 계약

Track A에서 그대로 가져오는 값:

- WModel/DDS Resources-relative asset ID
- source family
- start delay/lifetime
- cue-local transform을 client local transform으로 한 번 bake한 position/rotation/scale
- fixed particle burst/rate/lifetime/velocity/acceleration/size/local-space/random/max-particle
- bounded material backend/opcode/pass/render state와 scalar/vector/color constant
- semantic texture lane의 asset ID, `t#`, `s#`, channel/swizzle, color space, filter/address
- MeshParticle WModel carrier import scale
- 지원 capability manifest에 포함된 Particle module/distribution/literal과 deterministic seed policy

fresh generic defaults를 유지하는 값:

- source 값이 없거나 0/0으로 소실된 particle spawn
- 지원 manifest 밖이거나 실행 의미를 입증하지 못한 source module graph
- 회수하지 못한 native VF/MRT/SceneColor/SceneDepth 의미
- live bone-follow history와 이번 pass 범위 밖인 Ribbon history

Artist F에서는 evidence에 있는 fixed `t=0` burst `26`행, 합계 `167`을 그대로 이식한다. 다른 source에서
burst/rate 근거가 없으면 `burst=0 && rate=0`인 final Draft를 만들지 않고 명시적인 editable starter를 사용한다.
source basis와 attachment는 transform에 한 번 bake하고, 같은 basis를 attachment로 다시 적용하지 않는다.

## 5. Renderer와 shader 변경 범위

Renderer의 family dispatch와 draw 구조는 유지한다. normal authored preview는 `Stage_Document` 하나를 사용하고,
`Stage_ElementResource`가 Element의 `material.execution`을 기존 RuntimeMaterialV2/ArtistVisualV4 GPU carrier로
materialize한다. 실행 중 Visual Program/receipt/SHA를 다시 조회하지 않는다.

generic standard material은 다음만 명시적으로 소비한다.

- Base RGBA
- Noise R
- Mask R
- Dissolve R
- Emissive RGB
- Opaque / Alpha / Additive profile
- color multiply/offset/clip
- emissive intensity
- dissolve progress
- UV scale/offset/speed/sequence

Track A의 bounded material snapshot이 있는 Element는 generic 목록 대신 backend/opcode/pass, typed lanes,
sampler와 packed constants를 소비한다. snapshot이 없는 manual Element만 위 generic standard material을 사용한다.
enabled snapshot이 잘못되면 white fallback으로 숨기지 않고 stage를 실패시키며, 명시적 fail-closed Element는
Visible을 다시 켜도 renderer submission에서 계속 억제한다.

## 6. G별 구현 순서

### G00. 현재 계약 동결과 plan 전환

- 이 문서를 현행 정본으로 갱신
- source-backed Artist F 계획 퇴역 표시
- shared dirty worktree의 다른 변경 보존

종료 증거: 문서와 실제 코드 gap 표, `git diff --check`.

### G01. Element document transaction 통일

수정 대상:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

범위:

- Normal UI에 New Effect 노출
- resource 없는 Create Element Draft
- Create/Add 즉시 disk save 제거
- Current Effect tree의 `[UNSAVED]` 상태
- Delete 버튼을 도달 가능한 Current Effect tree로 이동
- Save Changes 한 곳에서 Draft 적용 + 전체 문서 저장
- Reload Saved selection 복원

종료 증거: Sprite Draft 생성 → Base 선택 → Visible/Transform 수정 → Save → Reload 구조 검증.

### G02. Generic Sprite first pixel

Known-good Base DDS로 standalone Sprite 하나를 만든다. source profile과 Track A preparation 없이 `Stage_Document`만 사용한다.

종료 증거:

- Client Debug build/link
- renderer submitted Sprite draw 진단
- 사용자의 first-pixel 확인
- Save/Reload 뒤 동일한 slot/detail

### G03. Particle family starter

- Sprite Particle fresh defaults
- Mesh Particle fresh defaults + WModel
- zero-spawn import 보정
- family별 Detail UI 정리

종료 증거: 각 family Solo에서 nonzero active/submitted count, 사용자 화면 확인.

### G04. Local Decal과 Trail/Ribbon

- generic Decal projector starter
- world pivot/bone/root attachment
- Trail point history starter
- LocalDecal `#20/#21` six-SRV typed material execution 저장과 ordinary renderer 소비
- Artist F Ribbon geometry/history 복원은 이번 material/particle pass 범위 밖으로 유지

종료 증거: 바닥 Decal Solo, 이동 anchor Trail Solo, Save/Reload.

### G05. Track A seed importer

Track A/Imported source를 import 시점에 한 번 읽어 editable Element Draft를 만든다. 회수된 bounded material은
`material.execution` snapshot으로, 지원되는 Particle module/distribution은 portable `sourceRecipe`로
정규화하고 WModel carrier 값도 Element data에 이식한다.
runtime source sidecar/projection 권위는 복사하지 않는다.

종료 증거: Artist F의 Mesh/Sprite/Decal/Ribbon source row를 각각 하나씩 import하고 current Effect에 저장.

### G06. Artist F authored rebuild

기존 `.unified`의 사용자 Visible/Transform/DDS 편집을 보존하는 deterministic offline materializer로
33개 source row의 material/particle/model carrier를 실제 authored JSON에 이식했다. 정상 UI에는 Upgrade
버튼이나 호출자가 없으며, 재-import는 개발용 offline materializer/검증 명령이 소유한다.

종료 증거: Play All, Family, Solo, Details, Visible, slot 교체, Save/Reload, animation anchor 사용자 확인.

### G07. 4직업과 Valtan 확대

- 현재 우선 범위는 Warlord, DimensionMaster, Artist, LanceMaster의 animation-linked skill이다.
- DimensionMaster `2050010`, Artist `31000`, LanceMaster `34010`의 BA1~BA4와 Artist F는 기존 Track A
  evidence를 공용 importer 입력으로 사용한다.
- Warlord는 동급 Track A Visual Program이 없으므로 대표 스킬 한 개의 imported graph/material/attachment를
  먼저 typed authored packet으로 정규화한 뒤 같은 계약으로 확대한다.
- Valtan은 keyboard slot이 아니라 stable pattern/action/stage ID에 effectAssetId를 연결
- visual acceptance 전 기존 product cue를 보존

종료 증거: class별 representative skill과 Valtan representative pattern의 source → edit → save → anchored preview.

### G08. Product 전환과 Track A runtime 퇴역

기존 product JSON과 effectref는 새 문서의 사용자 승인이 끝날 때까지 현재 위치에서 그대로 동작하게 둔다.
새 authored Effect는 별도 Effect ID로 생성하고, 승인된 skill/stage만 catalog/animevent mapping을 원자 교체한다.
전환 뒤에도 기존 JSON/ID는 normal UI에서 숨긴 read-only Legacy/Rollback Reference로 보존한다. mapping 전환
전에 기존 파일을 이동·삭제하거나 새 JSON으로 덮어쓰지 않는다.

zero-reference 확인 뒤에만 Track A reconstructed runtime, Visual Program execution, source overlay와 runtime SHA gate를 별도 PR에서 제거한다.

## 7. 검증

자동 검증은 Play/Save를 막는 runtime SHA gate로 사용하지 않는다. 구현 변경의 정상·실패 transaction을 확인하는 focused test만 사용한다.

- JSON parse와 canonical round-trip
- 잘못된 effect ID/path/duplicate Element/slot kind 거부
- partial Draft 저장과 drawable gate 분리
- stale writer save 실패 시 기존 file/Document/preview 보존
- Element stable ID 기반 Save/Reload selection
- Visible/slot/detail round-trip
- family별 generic playback active/submitted count
- Client x64 Debug/Release build
- `git diff --check`

Client와 UI는 사용자가 직접 실행한다. 에이전트는 빌드와 진단값까지만 준비하고 first pixel과 visual fidelity는 사용자 관찰로 판정한다.

## 8. PR 분리

현재 shared worktree가 매우 크므로 한 번에 삭제·publish하지 않는다.

1. Element transaction/Normal UI
2. Generic Sprite/Particle/Decal/Trail renderer starter
3. Track A seed importer와 Artist F authored rebuild
4. animation anchor와 representative skill
5. 4직업 rollout
6. Valtan pattern mapping
7. 승인 뒤 product mapping 전환
8. Track A execution/audit retirement

각 PR은 다른 팀원의 dirty 변경을 reset/clean하지 않고 자신이 수정한 파일과 직접 소비 데이터만 포함한다.
