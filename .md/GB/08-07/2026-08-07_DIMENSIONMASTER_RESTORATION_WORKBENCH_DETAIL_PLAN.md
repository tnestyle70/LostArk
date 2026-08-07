# 2026-08-07 차원술사 복원 워크벤치 상세 코드 계획서

## 1. G01 Restoration Session bar

파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

추가 함수:

```cpp
void Render_AuthoringSessionBar();
bool_t Try_ApplyDraftAndSave();
```

`Try_ApplyDraftAndSave` 순서:

```text
active document 확인
→ active document 복사
→ ParticleSystem draft 적용
→ selected Detail/SourceRecipe draft 적용
→ Validate + world preview stage
→ active memory commit
→ draft dirty 해제
→ Save_Atomic
→ Refresh DataFiles/AllEffects
```

중간 실패 시 저장 파일은 바뀌지 않는다. memory commit 뒤 disk save가 실패하면 document dirty를 유지하여 재시도할 수 있게 한다.

구조적으로 유효하지만 drawable하지 않은 draft는 저장할 수 있다. 이 경우 성공 문구는
`partial draft saved / preview hidden / publish blocked`로 분리하고 live preview 성공으로
표시하지 않는다.

## 2. G02 All Effects current document 우선권

현재 분기:

```cpp
if (Find_Assembly(effectId)) {
    Render_AssemblyHierarchy(effectId);
    continue;
}
Render active/document Elements;
```

변경:

```cpp
if (Find_Assembly(effectId) && !activeSkill) {
    Render_AssemblyHierarchy(effectId);
    continue;
}
if (Find_Assembly(effectId) && activeSkill &&
    ImGui::TreeNode("Published Runtime Hierarchy (diagnostic)")) {
    Render_AssemblyHierarchy(effectId);
    ImGui::TreePop();
}
Render active Authored Elements;
```

이렇게 해야 방금 추가/수정한 resource와 SourceRecipe가 publish 전에도 선택 가능하다.

## 3. G03 상태 문자열

저장 성공:

```text
Saved Authored atomically; world preview uses this document.
Assembly/WFX/Runtime Catalog publish pending.
```

runtime equivalence가 true이면 pending 문구 대신 `loaded Runtime Catalog snapshot equivalent`를
표시한다. 이는 disk publish나 catalog hot reload를 수행했다는 뜻이 아니다.

## 4. G04 저장 동시성 계약

`CEffectDocumentCodec`에 다음 API를 추가한다.

```cpp
static bool_t Save_AtomicIfUnchanged(
    const std::filesystem::path& Path,
    const EFFECT_DOCUMENT_DESC& Document,
    std::string_view strExpectedCanonicalDocument,
    std::string& strOutError);
```

빈 expected 값은 파일이 없어야 한다는 뜻이다. Authored load는
`Serialize(loadedDocument)`를 baseline으로 기억한다. 저장은 unique temp write, parse,
canonical equality, destination baseline compare, backup, promote 순으로 처리한다.

## 5. G05 bounded Material authoring

- `EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS`에 등록된 profile만 combo로 선택
- named scalar/vector/static switch 값 편집
- Dynamic Parameter semantic과 SubUV mode는 등록 token만 선택
- Parent path, source material path, parameter name/group은 읽기 전용
- 첫 편집부터 `RECONSTRUCTED_PROFILE`로 표시
- typed resource slot은 기존 `Try_BindResource` transaction을 유지

이 단계는 Parent graph exact 구현이 아니다. 현재 `CEffectDocumentRenderer`가 실제 소비하는
finite profile 입력을 사람이 레퍼런스와 대조해 보정하는 경계다.

## 6. G06 회귀

- `Apply_EffectElementDetailDraft`가 `Detail`, `Material`, `SourceRecipe`를 유지
- Resource binding은 `Try_BindResource`의 document transaction으로 즉시 commit
- unapplied draft가 있을 때 기존 plain Save는 계속 거부
- one-click Apply+Save는 draft를 먼저 검증하고 저장
- Reload는 dirty 상태에서 계속 거부
- active Authored tree와 published diagnostic tree가 동시에 접근 가능
- non-drawable partial draft 저장은 preview 성공으로 표시하지 않음
- Save round-trip canonical 불일치 거부
- 외부 파일 변경 뒤 stale save 거부와 외부 파일 보존
- previous promotion SHA와 다른 canonical Authored가 있으면 promotion 시작 전 거부

Preview의 `Screen Post` checkbox는 `Build_PreviewDocument`에서
`EFFECT_ELEMENT_KIND::SCREEN_POST`만 제거한다. Authored document와 dirty state는 바꾸지
않는다. A/B capture line은 Active Effect ID, Sample Time, selected emitter, Screen Post
ON/OFF를 항상 같은 화면에 표시한다.

## 7. 다음 sidecar 계약 초안

이번 빌드 뒤 별도 구현한다.

```json
{
  "schema": "lostark.effect-restoration-override",
  "formatVersion": 1,
  "effectAssetId": "effect.dimensionmaster.skill.2050100",
  "baselineSha256": "...",
  "elements": [
    {
      "elementId": "stable source emitter id",
      "fields": {
        "resources": true,
        "material": true,
        "detail": true,
        "sourceRecipe": true
      },
      "value": {}
    }
  ]
}
```

전체 문서를 복제하지 않고 사용자가 실제로 고친 stable element만 보존한다.
