# 2026-08-17 Effect Tool 저작 UX 복원 결과

branch: `feature/effect-tool-texture-kind-filter`
기준 계획: `.md/GB/08-17/2026-08-17_EFFECT_TOOL_AUTHORING_UX_RESTORATION_IMPLEMENTATION_PLAN.md`

## 1. 실제 구현 상태

계획 G01, G02를 반영했다. 변경은 한 파일이다.

```text
Client/Private/Effect_Tool.cpp   +305 -16
```

### 1.1 Element Resource Slot 저작 (G01)

`Render_ResourceSlots()`의 비-draft 분기에 편집용 slot 카드 렌더를 추가했다.

```text
bEditableElement = (NEW_DOCUMENT || AUTHORED == m_eActiveDocumentSource)
  meshModel/base/noise/mask/emissive/dissolve 6개를 Slot_Allowed(Element, slotId)로
  걸러 먼저 그린다. 바인딩이 없어도 카드를 그린다.
  AuthoringSlots에 그린 slot ID를 모아 아래 declared 루프에서 중복을 건너뛴다.
  Is_MissingBaseSourceDecal의 "Base (Decal)" 카드도 base를 이미 그렸으면 건너뛴다.
  안내 문구를 편집 가능일 때 바인딩 절차 설명으로 교체했다.
```

`Try_BindResource()`에 저작 binding 생성 분기를 추가했다.

```text
bAuthoredTemplateSlot =
  편집 가능 Document
  && material lane / source texture / 기존 binding 이 모두 없음
  && (meshModel 이면 Element.eKind 가 MESH 또는 PARTICLE
      아니면 Find_EffectMaterialInput(Material.strTemplateId, slotId) != nullptr)

참이면 ResourceBindings.push_back({slotId, assetId}) 로 첫 binding을 만든다.
거짓이면 기존 "compiler did not declare that resource lane" 거부를 유지한다.
```

Decal Base 예외 분기와 Imported/Runtime 문서의 기존 동작은 그대로다.

### 1.2 Valtan asset name 비교 (G02)

file-local 상수를 추가하고 죽어 있던 비교 여섯 곳을 교체했다.

```text
constexpr const char* VALTAN_ANIMATION_ASSET_NAME = "Valtan";

교체한 위치 (변경 전 줄 번호)
  3238   보스 전용 pivot 안내
  3820   Model View 라벨 헤더
  10239  Valtan pattern action 라벨링 진입
  18279  transform history 게이트
  18405  transform history 게이트
  19379  BOSS_PREVIEW_ASSET 상수
```

## 2. 자동 검증 — 실행함

### 2.1 Client x64 Debug 빌드

```text
MSBuild Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
작업 디렉터리 Client/Default

경고 3개  오류 0개   경과 00:00:07

Client/Default/Client/x64/Debug/Effect_Tool.obj   2026-08-17 02:14:27
Client/Bin/Debug/Client.exe                       2026-08-17 02:14:28
```

### 2.2 정적 확인

```text
asset-name 비교로 남은 "Boss_Valtan"      0건
VALTAN_ANIMATION_ASSET_NAME 사용          7건 (정의 1 + 비교 6)
BOSS_VALTAN archetype ID                  12건 그대로 보존
  AnimationSkillBindingDocument, CharacterPreviewPanel, Effect_Tool 3건,
  Level_CharacterSelect, MapTool 2건, Valtan 2건,
  ValtanPresentationAssetService, AnimationPreviewAssets.h
git diff --check                          clean
```

`BOSS_VALTAN`은 boss archetype ID로 asset name과 다른 식별자다. 교체 대상이 아니다.

## 3. 수동 검증 — 미실행

에이전트는 화면을 판정하지 않았다. 사용자가 확인할 항목이다.

```text
Element slot 저작
  valtan.test를 열고 Sprite Particle Element를 새로 만든다.
  Selected Element Resource Set 에 Base/Noise/Mask/Emissive/Dissolve 카드가 보이는지.
  Empty 카드를 고르고 Resource Library에서 DDS를 선택한 뒤 Bind Selected 가 성공하는지.
  Save Changes 후 Reload Saved 로 바인딩이 남아 있는지.
  Mesh Particle 에서 meshModel 카드가 함께 보이고 Sprite Particle 에서는 보이지 않는지.

Imported / Runtime 문서 회귀
  Imported 문서를 열었을 때 기존처럼 선언된 lane만 보이고
  빈 slot 카드가 새로 생기지 않는지.

Valtan 라벨
  Model View에서 [Boss] Valtan 선택 시 헤더가
  "[Valtan] Pattern Action | Model Clip"으로 바뀌는지.
  Reload Labels 후 상태가 "Valtan: N clips labeled from pattern action bindings."인지.
  clip 콤보 항목이 "[Valtan] valtan.attack.* | mesh_att_*" 형식으로 보이는지.
```

## 4. 남은 경계

```text
Valtan.patterneffects.json 은 여전히 binding 1건이다.
  VALTAN_WHIRLWIND / valtan.attack.whirlwind.active / effect.valtan.pattern.420633.active
  31 패턴 확장은 actionId 어댑터가 필요하며 .md/GB/08-15 survey 범위다.

Valtan 축 라벨은 한국어 스킬명이 아니라 [Valtan] <actionId> | <clip> 형식이다.
  한국어 스킬명은 playable class 전용 경로이며 보스 축에는 대응 데이터가 없다.
  한국어 패턴명이 필요하면 Valtan.patternbindings.json 에 표시명 필드를 추가하는
  별도 데이터 계약이 필요하다. 이번 변경에 포함하지 않았다.

Create Element 이전 draft 시드 패널과 이후 Element 편집 패널이 여전히 분리돼 있다.
  이번 변경은 후자를 편집 가능하게만 만들었다.

RenderSlotCard 비-draft 분기가 수동 저작 slot에도 "Source"를 표시한다. 기존 동작이다.

Release 구성과 ProjectAudit, ClientFrontendHarness 는 실행하지 않았다.
```

## 5. 같은 브랜치의 선행 변경

이 브랜치에는 앞선 커밋 `bf1e3df1`의 Texture Kind 필터가 포함돼 있다.
해당 범위와 검증은 `2026-08-17_EFFECT_TOOL_TEXTURE_KIND_FILTER_RESULT.md`가 소유한다.

동시 진행 중인 Codex 세션의 미커밋 파일(`Level_*.cpp`, `MainApp.*`,
`LV_LUT_HEARTRB_ED.worlddestruction*.json`, `Framework.slnLaunch`)은
이번 변경에서 stage하지 않았다.
