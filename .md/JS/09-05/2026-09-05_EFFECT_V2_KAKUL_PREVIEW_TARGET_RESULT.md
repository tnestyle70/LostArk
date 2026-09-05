# 2026-09-05 Effect Tool V2 쿠크세이튼 Attach 타깃 + clip lane 바인딩 복구 RESULT

브랜치: `feature/effect-v2-buffer-pool` 위 미커밋 작업 (main `415bbc59` 기준).
수정 파일: `Client/Public/EffectV2_Target.h`, `Client/Private/EffectV2_Object.cpp`,
`Client/Private/EffectV2_Runtime.cpp`, `Client/Private/EffectV2_Document.cpp`,
`Client/Public/Effect_Tool_V2.h`, `Client/Private/Effect_Tool_V2.cpp`. 새 파일·project 등록 없음.

## 1. 요청과 실측 결론

요청: 발탄처럼 쿠크세이튼도 Effect Tool V2(Effect Attach v2 창)에서 이펙트를 저작할 수 있게 하고,
원작이 관문별로 다른 모델을 쓰는 것이 저작에 문제가 되는지 판단.

실측(09-03 팀장 인벤토리 분석과 현재 코드 대조):

| 항목 | 상태 |
|---|---|
| BossCatalog 쿠크 archetype, `CKakul*` GameObject, Kakul gameplay/presentation source, Arena boss placement | 전부 없음. `CLevel_KakulSaydonArena`는 보스를 스폰하지 않는다 |
| 쿠크 몸체 | `ANIMATION_PREVIEW_ASSETS`의 playback-only 4개(`MN_RPCZ_00`, `MN_RPCT_00`, `MN_RPCT_05`, `MN_RPCT_06`), `Character/KoukuSaton/...` |
| Effect V2 바인딩 codec | 09-02 `6d95a903` 이후 C++ `Parse_Bindings`가 formatVersion 2만 받음. NPC 바인딩 3개(`NPC_58700/59030/59060`)는 여전히 v1이라 툴 Load와 런타임 `Ensure_Bindings` 모두 거부 |

따라서 "Valtan pattern tree처럼 Kakul tree를 붙이는" 방식은 정본이 없어 불가능하고(gotchas: gameplay source 없는 encounter에 공용 runtime을 먼저 만들지 않는다),
Wei 등 NPC와 같은 **clip 기반 Attach 타깃**으로 붙였다. 그런데 그 clip lane 자체가 09-02 통합 뒤 죽어 있어 함께 복구했다.

## 2. 구현

### 2.1 Attach 타깃 종류 `PREVIEW_BODY`

- `EFFECT_V2_TARGET_KIND::PREVIEW_BODY` 추가. `EFFECT_V2_TARGET`에 `strArchetypeId`(PREVIEW_BODY의 바인딩 owner)와
  `From_PreviewBody(shared_ptr<CPart_Body>, archetypeId)` 추가.
- `CEffectV2Object::Resolve_TargetView`: 몸체 `CPart_Body`의 `Get_Model()`과 Transform world를 BoneRoot/YawBasis로 사용
  (parent matrix는 툴이 identity 상수를 넘기므로 Transform 하나가 world root).
- `CEffectV2Runtime::Resolve_Archetype`: PREVIEW_BODY면 `Target.strArchetypeId`.
- Effect Tool V2
  - Attach 창 Archetype 콤보에 Valtan 아래 KoukuSaton 4개 항목 추가.
  - `Spawn_PreviewBodyTarget`: `Prototype_GameObject_Part_Body`를 `Layer_EffectPreviewV2Target`에 clone.
    model prototype이 그 Level에 없으면(Development 밖) 미리보기 패널과 같은 pretransform(scale 0.01, yaw -90)으로 한 번 등록하고 재시도.
  - `Move_Target`: PREVIEW_BODY도 Transform 위치/yaw로 이동.
  - `Update_Attach`: PREVIEW_BODY는 owner Update가 없으므로 툴이 `CEffectV2Runtime::Tick`을 호출한다("Runtime spawns on target" 검증 경로).
  - Despawn/Deactivate/Level change는 기존 `m_Target` 경로 그대로.

### 2.2 바인딩 owner 규칙 (관문별 모델 판단)

"N관문" 대응은 09-03 팀장 인벤토리 분석이 clip family와 기믹 이름을 대조해 추론한 것이며 원본 데이터가 아니다.
사용자가 2026-09-05에 정확하지 않다고 지적해 콤보 라벨에서 관문 표기를 뺐다. 몸체 사실(skeleton, clip 수, 텍스처)만 실측이다.

| 콤보 항목(라벨) | 몸체 | 바인딩 파일(`Data/Effects/V2/Bindings/`) |
|---|---|---|
| `MN_RPCZ_00` (Kakul clown, 91 clips) | RPCZ_00 (122 bone, `rpcz00_*`) | `MN_RPCZ_00.effectv2bindings.json` |
| `MN_RPCT_05` (Saydon with Kakul parts, 249 clips) | RPCT_05 (165 bone, `rpct00_*`), 쿠크 텍스처·emissive 파츠 | `MN_RPCT_05.effectv2bindings.json` |
| `MN_RPCT_00` (Saydon plain body, same rig as 05) | RPCT_00, **05와 skeleton·clip 동일** | `MN_RPCT_05.effectv2bindings.json` (공유) |
| `MN_RPCT_06` (Large Saydon, own rig, 34 clips) | RPCT_06 (106 bone, `mn_rpct_06_sk.ao_*`) | `MN_RPCT_06.effectv2bindings.json` |

판단:

- Effect 문서(leaf/group)는 몸체와 무관하다. 바인딩만 (clip, bone) 계약이라 몸체별로 나뉘며, 세 skeleton의 clip 이름 접두사가 서로 달라
  다른 몸체에서 잘못 발화할 수 없다. 관문별 모델 차이는 저작 문제가 아니라 파일 3개로 자연스럽게 분리된다.
- 2관문 무대 세이튼과 3관문 합체는 같은 05 몸체·같은 clip이라 공용 기믹(불뿜기 14, 종이비둘기 15, 아드레날린 17 등)은 바인딩 하나가 두 관문에서 같이 재생된다.
  원작도 같은 clip을 재사용하므로 의도된 동작이다. 관문마다 다른 연출이 필요한 clip은 나중에 Kakul pattern/stage 계약이 생겨야 stage lane으로 나눌 수 있다.
- 2관문을 00으로 보일지 05로 보일지(팀장 미결정)는 skeleton과 clip이 동일해 바인딩에 영향이 없다. 텍스처만 다르다.
- 제약: `WP_MN_RPCT_06`은 본체와 같은 이름 clip을 재생하는 skinned 무기이고, 쿠크 무기 3종(`b_wp_1/2/3`) 모델은 Resources에 없다.
  무기에서 나오는 이펙트는 본체 소켓 본(`b_wp_*`)에 앵커해 무기 없이 저작한다. 모든 몸체에 `b_effectroot`가 있어 기본 피벗은 발탄과 같다.
- 런타임 소비자는 아직 없다. 쿠크 보스 GameObject/BossCatalog archetype이 생기면 그 객체의 `Resolve_Archetype`이 위 몸체 ID 파일을 가리키게 연결하면 된다
  (`Binding_Path`는 이미 archetype 문자열만 쓴다).

### 2.3 clip lane(formatVersion 1) 바인딩 codec 복구 — `EffectV2_Document.cpp`

- `Parse_Bindings`: header가 formatVersion 1이면 `Parse_ClipLaneBindings`로 분기. 행 형식은 09-02 이전과 같다
  (`effectId|group`, `clip`, `startMs`, `bone`, `followBone`, `rotation`, `stopWithClip`, optional `offset`, `yawDegrees`). `stage` 행은 거부(v2 전용).
  typed 필드(`CLIP_OCCURRENCE` clock, anchor, follow/rotation basis, LocalTransform, stopPolicy)를 채운 뒤 `Populate_BindingConvenience`로 mirror 필드를 만든다.
  결과 `strStage`가 비고 `strClip`이 채워져 `CEffectV2Runtime::Notify_Clip`의 clip lane 매칭이 다시 성립한다.
- `Serialize_Bindings`: 모든 행에 `bindingId`/`actionId`/`stage`가 없으면(툴 Add로 만든 행, v1 로드 행) v1으로 쓴다. 그 외는 기존 v2 그대로.
  BOSS_VALTAN catalog는 항상 bindingId가 있어 v2 경로만 탄다(`Generate_StableBindingId`도 `binding.valtan.pending`을 채워 해시하므로 동일).
- Python `effect_v2_binding_pipeline.py`의 `_legacy_resource/_legacy_anchor`와 같은 필드 규칙이다.

이 복구가 없으면 Kakul뿐 아니라 Wei/Balthorr/Silian NPC 이펙트도 현재 main에서 로드되지 않는다(09-02 통합 회귀, 팀장 인계 항목).

## 3. 검증

실행한 것:

- `git diff --check` 통과.
- Python source-oracle/pipeline 계약: `Tools.EffectToolV2.{test_effect_v2_catalog_contract, test_effect_v2_product_contract,
  test_effect_resource_catalog_facade_contract, test_effect_v2_binding_pipeline, test_valtan_magicball_black_core_contract}`,
  `Tools.ValtanPipeline.{test_action_composition_workbench_regression_oracles, test_action_presentation_workbench_contract,
  test_action_composition_atomic_save_contract, test_f1_semantic_resource_and_level_navigation_contract, test_world_entity_spawn_revision_contract}`
  → 내 변경 관련 전부 PASS. `test_mainapp_unified_effect_tool_entry_contract` 1건 실패는 `MainApp.cpp`의 `TOOL_FOCUS_OPTIONS` 크기(12 vs oracle 10)로
  내가 건드리지 않은 main 상태의 기존 불일치.
- `"Valtan Effect Resources"` 라벨은 oracle 4개가 고정하고 있어 이름을 바꾸지 않았다.

- Debug Product 빌드 성공(오류 0, `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`).
- 로컬 Server(`127.0.0.1:7777`) + Client 실행 후 사용자 확인: Wei `NPC_58700` 이펙트가 이전처럼 나온다(사용자 서면 판정 "잘 나온다").
  쿠크 몸체 Spawn/Save/Reload는 아직 사용자 확인 전.

하지 않은 것:

- 관문 대응 확정. 라벨은 관문 없이 몸체 사실만 표기하기로 결정(사용자, 09-05). 사용자 실행 후 확인 순서: Debug Client → Character Select 또는 Development → F1 → Effect Tool v2 → Attach 창 →
  Archetype `MN_RPCZ_00` → Spawn Target → Target Clip에서 `rpcz00_att_battle_*` 선택 → Pivot Bone/Spawn Frame → Add/Update Binding → Save Bindings →
  Reload로 v1 파일 재로드 확인 → "Runtime spawns on target"으로 런타임 발화 확인. NPC_58700 Wei도 같은 창에서 다시 로드되는지 함께 확인.
- Character Select에서 쿠크 몸체를 툴이 먼저 등록한 뒤 같은 세션에서 Animation Tool 미리보기가 같은 몸체를 열면 패널의 `Add_Prototype`이 중복으로 실패한다
  (Development는 Loader가 미리 등록하므로 해당 없음). 재현되면 패널이 기존 prototype을 재사용하도록 손봐야 한다.

## 4. 남은 것

- 쿠크 이펙트 문서 ID 규칙(`boss.kakul.*` 권장)과 첫 저작 대상 선정.
- 09-02 v2 통합이 NPC v1 바인딩을 깨뜨린 사실과 이 복구를 팀장에게 인계. Python 쪽은 legacy reader가 있으므로 C++만 어긋나 있었다.
- 쿠크 무기 리소스(`WP_MN_RPCZ_*`) 부재, 2관문 본체(00/05) 결정은 팀장 몫.
