# 2026-08-17 Effect Tool 저작 UX 복원 구현 계획

branch: `feature/effect-tool-texture-kind-filter`

사용자가 `valtan.test`로 첫 수동 Effect를 만드는 동안 드러난 저작 경로의 실제 차단 지점을
정리하고 수정한다. 대상은 원본 복원 저작 흐름이며 제품 runtime 계약은 바꾸지 않는다.

## G00. 사용자 관찰과 재현 경로

사용자가 실행한 경로와 관찰한 결과는 다음과 같다.

```text
New Effect(valtan.test) -> Create Element(Mesh Particle) -> base/noise 시드 -> 화면 출력 성공
선택한 Element를 다시 열면 좌측 slot 카드가 전부 Empty로 보인다.
Sprite Particle을 만들면 Declared Resources가 (no resources bound)이고 바인딩할 카드가 없다.
Model View에서 Valtan을 선택하면 한국어/패턴 라벨이 아니라
  "No playable-class skill binding owns the selected model."이 표시된다.
```

세 관찰 모두 코드에서 원인을 확정했다. 추정한 항목은 없다.

## G01. Element Resource Slot 저작 차단

### G01.1 현재 실측

`Render_ResourceSlots(bool_t bMeshAuthoringDraft)`는 두 화면을 한 함수로 그린다.

```text
bMeshAuthoringDraft = true    "Element Resource Slots"
  m_MeshAuthoringDraft를 대상으로 meshModel/base/noise/mask/emissive/dissolve
  여섯 카드를 항상 그린다. Empty도 그린다.

bMeshAuthoringDraft = false   "Selected Element Resource Set" > "Declared Resources"
  선택한 Element의 ResourceBindings 중 strAssetId가 비지 않은 것만 그린다.
  Material.Execution.TextureLanes와 SourceMaterial.Textures도 같은 조건이다.
```

사용자가 화면에서 본 Empty 여섯 칸은 **draft 카드**이고, 선택한 Element의 슬롯이 아니다.
선택한 Element의 실제 바인딩은 우측 Effect Detail의 Declared Resources에만 보인다.
같은 이름의 두 패널이 서로 다른 대상을 그려서 생긴 혼동이다.

`Try_CreateElementDraft()`는 `Element = m_MeshAuthoringDraft;`로 draft를 복사한다.
따라서 Create Element 전에 시드하지 않은 슬롯은 생성 후 영원히 빈 채로 남는다.

### G01.2 두 번째 차단 — bind 거부

빈 슬롯 카드를 그려도 `Try_BindResource()`가 거부한다.

```text
if (nullptr == pMaterialLane && nullptr == pSourceTexture && nullptr == pBinding)
    "Bind rejected: the compiler did not declare that resource lane."
```

이 규칙은 Cascade compiler가 lane 집합을 소유하는 Imported/Runtime 문서에는 맞다.
그러나 수동 저작 Element는 lane을 선언하는 주체가 compiler가 아니라
`Material.strTemplateId`의 material template이다. 현재는 Decal Base 하나만 예외로
새 binding 생성을 허용한다.

`EFFECT_STANDARD_MATERIAL_INPUTS`는 base/noise/mask/emissive/dissolve 5개를 선언하고
수동 생성 Element는 `EFFECT_STANDARD_MATERIAL_TEMPLATE_ID`를 받는다. 즉 선언은 이미
존재하며 `Slot_Allowed(Element, slotId)`도 이미 true를 돌려준다. 막고 있는 것은
binding 생성 규칙 하나뿐이다.

### G01.3 변경 범위

```text
Render_ResourceSlots()  편집 가능한 Document일 때 표준 slot 카드 6개를 먼저 그린다.
                        Slot_Allowed(Element, slotId)로 Element별 허용 여부를 판정한다.
                        이미 그린 slot은 아래 declared 루프에서 건너뛴다.
Try_BindResource()      수동 저작 Element + template 선언 slot이면 첫 bind가
                        ResourceBindings에 새 binding을 만든다.
```

편집 가능 판정은 `EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT`와 `AUTHORED` 두 가지다.
`IMPORTED`, `RUNTIME_VISUAL_PROGRAM` 등 검사 전용 문서의 기존 동작은 유지한다.

## G02. Valtan 애니메이션 라벨 축 불일치

### G02.1 현재 실측

`ANIMATION_PREVIEW_ASSETS`의 boss 항목은 asset name을 `"Valtan"`으로 선언한다.

```text
Client/Public/AnimationPreviewAssets.h
  pId          "boss.valtan"
  pLabel       "[Boss] Valtan - MN_RPBF_01 + AnimSet (173 clips)"
  pAssetName   "Valtan"
  pModelAssetId "Character/Valtan/MN_RPBF_01.wmodel"
  pBossArchetypeId "BOSS_VALTAN"
```

`CharacterPreviewPanel.cpp`가 `Bind_Preview(stagedValtan, asset.pAssetName)`로 넘기고
`CAnimationTargetService::Resolve_AssetName()`이 그 값을 그대로 돌려준다. 즉 런타임 값은
`"Valtan"`이다.

그런데 `Effect_Tool.cpp`는 여섯 곳에서 `"Boss_Valtan"`과 비교했다.

```text
3238   보스 전용 pivot 안내
3820   "[Valtan] Pattern Action | Model Clip" 라벨 헤더
10239  Valtan pattern action 라벨링 진입
18279  보스 전용 transform history 게이트
18405  보스 전용 transform history 게이트
19379  BOSS_PREVIEW_ASSET 상수
```

여섯 비교가 전부 항상 false다. 그래서 Valtan을 선택해도 pattern action 라벨 경로로
들어가지 못하고 playable-class 경로로 떨어져
`"No playable-class skill binding owns the selected model."`이 표시된다.
사용자가 말한 "예전에는 한국어 이름으로 정리됐는데 지금은 없다"의 코드상 원인이다.

정확히 하면 Valtan 축의 라벨은 한국어 스킬명이 아니라 `[Valtan] <actionId> | <clip>`
형식의 pattern action 라벨이다. 한국어 스킬명은 playable class 전용 경로다.

### G02.2 변경 범위

asset name 정본은 descriptor의 `pAssetName`이므로 비교 쪽을 고친다. `Data/Animation/Authored/Valtan/`
폴더명, `CValtanPatternAnimationBindingDocument::Load("Valtan", ...)`의 인자와도 `"Valtan"`이 일치한다.

같은 문자열을 여섯 번 반복하지 않도록 file-local 상수 하나를 두고 전부 그것을 쓴다.

```text
constexpr const char* VALTAN_ANIMATION_ASSET_NAME = "Valtan";
```

`BOSS_VALTAN`(대문자)은 boss archetype ID이며 이 변경과 무관하다. 건드리지 않는다.

## G03. 변경할 파일

```text
Client/Private/Effect_Tool.cpp   상수 1개 추가, 비교 6곳 교체,
                                 Render_ResourceSlots 편집 slot 렌더 추가,
                                 Try_BindResource 저작 binding 생성 분기 추가
```

새 파일이 없어 `.vcxproj` / `.vcxproj.filters` 등록 변경은 없다.
`Client/Public/Effect_Tool.h`는 이 G에서 바꾸지 않는다.

## G04. 검증

```text
build     MSBuild Client.vcxproj /p:Configuration=Debug /p:Platform=x64
          작업 디렉터리 Client/Default
정적 확인  잔여 "Boss_Valtan" asset-name 비교 0건
          BOSS_VALTAN archetype ID 12건은 보존
git       git diff --check
runtime   사용자 육안 확인 항목은 RESULT §3에 분리해 기록한다
```

Data 변경이 없으므로 Effect publisher validation 대상이 아니다.

## G05. 이번 변경에 포함하지 않은 항목

사용자가 요청한 "복원 편의 전면 개선"의 나머지는 별도 변경 단위로 둔다.
지금 상태를 사실대로 남긴다.

```text
Valtan.patterneffects.json은 여전히 binding 1건(420633 whirlwind active)이다.
  31 패턴을 채우려면 actionId 축 어댑터가 필요하며
  .md/GB/08-15 survey가 그 범위를 소유한다.

Create Element 이전 draft 시드와 이후 Element 편집이 여전히 두 패널로 나뉜다.
  이번 G는 후자를 편집 가능하게만 만들었고 두 패널을 합치지는 않았다.

RenderSlotCard의 비-draft 분기는 수동 저작 slot에도 "Source"를 표시한다.
  기존 동작이며 이번 G에서 바꾸지 않았다.

Sprite Particle 등 non-mesh family의 Create Element 시드 UI는 그대로다.
  생성 후 바인딩이 가능해졌으므로 차단은 해소됐지만 시드 흐름 자체는 개선하지 않았다.
```
