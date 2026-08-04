# 공용 Character Preview 이관 · anchor 계약 · 스킬 effectId

작성자: JS · 2026-08-04

Effect 담당자가 슬라이스 C(스킬 → 이펙트)를 시작할 수 있도록, 두 툴이 공유해야 하는
경계를 열었다. 이펙트 저작 기능 자체는 담당자 소유이며 이 문서는 **담당자가 쓸 수 있게
된 것과 아직 담당자 몫으로 남은 것**을 구분한다.

선행 계획은 `.md/GB/08-03/2026-08-03_EFFECT_TOOL_ADVANCEMENT_PLAN.md`,
경계 정본은 `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`다.

## 1. 왜 했나 — 실측한 병목

Effect Tool의 월드 프리뷰 위치가 하드코딩이었다.

```text
Client/Public/Effect_Tool.h   float3_t m_vWorldPreviewPosition = { 0.f, 2.f, 0.f };
```

무기 소켓에 붙을 이펙트를 만드는데 캐릭터도 무기도 없이 고정 좌표에서 보고 있었다.
반대로 캐릭터를 실제로 렌더하는 코드는 `CAnimation_Tool` 안에 사유화돼 있어서
Effect Tool이 참조할 방법이 없었다.

`CAnimationAuthoringBridge::Try_GetPlayheadSnapshot`은 구현돼 있으나 **호출자가 0개**다.
담당자가 읽기 창구만 열어두고 소비를 못 하던 상태였다.

## 2. 완료 — G1 공용 Preview 소유권 이관

신설 `Client/{Public,Private}/CharacterPreviewPanel.{h,cpp}`.

`CAnimation_Tool`에 있던 다음을 통째로 옮겼다.

```text
Refresh_PreviewLevel / Release_Preview / Select_PreviewAsset / Render_TargetSelector
m_pPreviewBody / m_pPreviewAsset / m_iPreviewLevelIndex / m_PreviewParentMatrix
```

Animation Tool은 이제 자기 문서가 dirty인지만 알려주고 프리뷰 수명에 관여하지 않는다.

```cpp
m_PreviewPanel.Refresh_Level();
m_PreviewPanel.Render_Selector(m_bDirty, "Save or discard ...");
```

`Animation_Tool.cpp`가 175줄 줄었다.

## 3. 완료 — G2 anchor 계약

`CAnimationTargetService`에 두 개를 추가했다.

```cpp
static bool_t Resolve_RootTransform(float4x4_t* pOut);
static bool_t Resolve_AnchorTransform(const char_t* pAnchorSlotId, float4x4_t* pOut);
```

- anchor 합성은 `CPart_Equipment`가 소켓 파츠에 쓰는 식과 동일하다
  (`Get_BoneMatrix(slot) * 소유자 world`). 두 번째 계산 경로를 만들지 않았다.
- 프리뷰 바디는 parent matrix가 선택 시점에 고정되므로 **값 복사**로 들고,
  씬 캐릭터는 매 질의마다 live transform에서 읽는다.
- 뼈가 없으면 **false를 반환**한다. 없는 anchor가 원점으로 둔갑하면 "배치된 이펙트"로
  오해되기 때문이다.

## 4. 완료 — G3 Effect Tool 최소 소비

`Follow Character Anchor` 체크박스와 슬롯 입력을 붙였다. 켜면 매 프레임 anchor 위치를
따라가고, 끄면 기존 수동 좌표가 그대로 동작한다. 기본 슬롯은 `b_wp_swm_m_1`.
anchor를 못 찾으면 마지막 위치를 유지하고 상태 문구를 띄운다.

이펙트 emitter·module·머티리얼 수치와 `CEffect_Runtime` 생성 경로는 건드리지 않았다.

## 5. 완료 — 스킬 `effectId`

계획서 C-1 1항 요청분이다. `Data/Balance/PlayerSkills.json` 15개 스킬 전부에
`effectId`를 추가했고 값은 전부 빈 문자열이다.

`Assert-ExactProperties`가 속성 이름 집합을 정확히 비교하므로 키 자체는 생략할 수 없다.
빈 문자열이 "아직 저작된 이펙트 없음"이며 현재 15개 전부 그 상태다(admitted 0개).

`Publish-GameplayBalance.ps1`은 값이 있을 때만 `Assert-StableId`로 검사한다.

**Server bootstrap에는 싣지 않는다.** `displayName`이 이미 검증만 되고 row에는 빠지는
선례가 있어 같은 패턴을 썼다. Publish 후에도 SKILL row는 12필드 그대로이고
`effectId` 문자열이 bootstrap에 없다. Server가 asset 참조를 모르는 상태가 유지된다.

## 6. 검증

자동:

```text
Client/Server/Shared/Engine x64 Debug 빌드 — 컴파일·링크 에러 0
NetworkProtocolHarness      failures : 0
ClientFrontendHarness       failures : 0
Server contract test        failures : 0
전체                        153 PASS / 0 FAIL
[PASS] Load gameplay balance bootstrap   (publisher 수정 후 Server 재읽기 확인)
Publish-GameplayBalance Validate 성공 (5 profiles / 15 skills / 16 damage / 1 boss)
git diff --check 통과, 잔류 프로세스·7777 리스너 없음
```

publisher 거부 경로 실측:

```text
effectId = "Effect/DimensionMaster/x.effect"
  -> effectId is not a stable ID: 'Effect/DimensionMaster/x.effect'
effectId 키 누락
  -> player skill fields are invalid. expected=[... effectId ...]
```

경로 문자열을 넣으면 거부되므로 `.effect` 파일 경로를 여기 저장하는 실수를 막는다.

미검증(수동 확인 대기):

- Animation Tool의 Target 선택(Scene Character ↔ Dimension Core/Summon)이 이관 후에도
  동일하게 동작하는지. **회귀 위험이 있는 지점이다.**
- Effect Tool `Follow Character Anchor`가 실제 무기 뼈를 따라가는지.

`ProjectAudit`은 실패한다. 원인은 이 변경과 무관하며
`Data/Effects/SourceCatalog/dimensionmaster_admission.json`이 `.gitignore` 대상
로컬 생성물이라 존재하지 않기 때문이다. 어느 브랜치에도 커밋된 적이 없다.

## 7. Effect 담당자가 지금 쓸 수 있는 것

```cpp
// 캐릭터 anchor / root 의 현재 world transform
CAnimationTargetService::Resolve_AnchorTransform("b_wp_swm_m_1", &world);
CAnimationTargetService::Resolve_RootTransform(&world);

// 현재 clip 과 playhead (기존, 이제 소비 가능)
CAnimationAuthoringBridge::Try_GetPlayheadSnapshot(snapshot, error);
```

`PlayerSkills.json`의 `effectId`는 stable ID 형식이며 Server로 가지 않는다.
클라에서 읽으려면 `CPlayerSkillCatalog`의 `PLAYER_SKILL_DEFINITION`에 필드를 추가한다.
**미리 넣지 않은 이유**는 그 구조체가 "소비자가 필요로 하는 것만" 담는 설계이기
때문이다(`movementDistance`, `maximumRange`도 빠져 있다). `EffectPlaybackService`가
읽는 시점에 함께 넣는 것이 맞다.

## 8. 남은 경계 — 담당자 몫

- admitted effect catalog와 resolver. 현재 admitted 0개라 `EFFECT_ASSET_ID` 생성이
  거부되며, 이것이 `Use Selected Effect` 성공 경로의 선행 조건이다.
- `.weffect` Git 배포 갈래 결정(계획서 C-1 5항). `*.weffect`가 `.gitattributes`에도
  `.gitignore`에도 없어 지금 승격하면 비-LFS raw 바이너리가 커밋되거나 팀원 런타임
  입력이 누락된다. **팀 결정이 필요하며 담당자 혼자 정할 수 없다.**
- anchor-relative local transform 편집(anchorKind / localPosition / localRotation /
  localScale). 이번에 연 것은 anchor의 world transform **조회**까지다.
- EFFECT window schema와 trail start/stop runtime.
- 공용 Preview Panel이 아직 소유하지 않는 것: preview camera/light/background,
  mouseGroundPoint, 다섯 class와 part 선택 UI, anchor slot 목록 열거.

추가로 필요한 계약이 생기면 요청 시 연다. 위 경계를 우회해 Animation 파일이나
`PlayerSkills.json`을 직접 쓰지 않는다.
