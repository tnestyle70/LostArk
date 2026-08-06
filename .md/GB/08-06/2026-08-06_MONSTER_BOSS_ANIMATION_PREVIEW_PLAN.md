# Monster·Boss Animation Preview 구현 계획

## 1. 목표

제품 Monster runtime, MapTool placement, Server authority를 만들기 전에 기존 Character Select Animation Tool에서 다음 다섯 모델의 WModel 애니메이션을 playback-only로 검증한다.

| 분류 | 원본 근거 | Preview asset name | 모델 경로 |
|---|---|---|---|
| 일반 몬스터 | NPC ID 480001 | `Monster_480001_MN_PADD_01` | `Character/Monster/NPC_480001_MN_PADD_01/NPC_480001_MN_PADD_01.wmodel` |
| 일반 몬스터 | NPC ID 480002 | `Monster_480002_MN_SJFC_00_4` | `Character/Monster/NPC_480002_MN_SJFC_00_4/NPC_480002_MN_SJFC_00_4.wmodel` |
| 일반 몬스터 | NPC ID 480003 | `Monster_480003_MN_0019_05` | `Character/Monster/NPC_480003_MN_0019_05/NPC_480003_MN_0019_05.wmodel` |
| 미니보스 | NPC ID 480005 | `Monster_480005_Lugaru_MN_RPRS_02` | `Character/Monster/NPC_480005_MN_RPRS_02/NPC_480005_MN_RPRS_02.wmodel` |
| 보스 | `BOSS_VALTAN` | `Boss_Valtan` | `Character/Valtan/MN_RPBF_01.wmodel` |

물리 폴더의 `NPC_`는 원본 DB provenance로 유지한다. Preview ID와 Tool 표시 이름만 Monster/Boss 역할을 표현한다.

## 2. 변경 경계

- `AnimationPreviewAssets.h`: 다섯 preview descriptor와 모델별 scale/yaw, playback-only, 선택적인 Boss archetype 검증값을 추가한다.
- `Loader.cpp::Ready_AnimationPreviewModels`: 기존 `CModel -> Prototype` 경로에서 descriptor별 transform을 사용하고 발탄 경로가 `BossCatalog` 정본과 일치하는지 검증한다.
- `Animation_Tool.cpp::Render`: playback-only target은 clip 재생 UI만 표시하고 Animation authored document, skill binding, effect/hit event를 읽거나 저장하지 않는다.
- 기존 `CCharacterPreviewPanel -> CPart_Body -> CAnimationTargetService` 생성·교체·rollback 경로는 수정하지 않는다.

## 3. 금지 경계

- Engine, Shared, Server, MapTool, World Gameplay, Navigation, Level enum, Lobby command를 수정하지 않는다.
- `CCharacter`, `CValtan` 제품 로직과 `BossCatalog.json`을 수정하지 않는다.
- Monster catalog/schema/AI/replication placeholder를 추가하지 않는다.
- Monster 리소스 누락을 Character Select 전체 로드 실패로 승격하지 않는다.
- playback-only target으로 `Data/Animation/Authored` 문서를 만들지 않는다.

## 4. 호출 흐름

```text
Ready_For_CharacterSelect
-> Ready_AnimationPreviewModels
-> CModel::Create(MODEL::ANIM, descriptor transform)
-> Add_Prototype(level, preview prototype tag)
-> Animation Tool target 선택
-> CCharacterPreviewPanel::Select_Asset
-> CPart_Body clone / Layer_AnimationPreview stage
-> CAnimationTargetService::Bind_Preview
-> Animation Tool playback-only Render
-> CModel clip list / Play / Pause / Loop / frame scrub
```

## 5. 완료 검증

1. 플레이어 다섯 class와 기존 Dimension Core/Summon Preview 회귀를 보존한다.
2. Monster/Boss target은 Effect Tool 목록에 나타나지 않는다.
3. WModel animation 수 `36/29/25/91/27`을 확인한다.
4. Monster/Boss target에서는 저작 Save UI와 데이터 load가 실행되지 않는다.
5. Client Debug/Release 빌드, `git diff --check`, ProjectAudit를 실행한다.
6. Debug Character Select에서 수동으로 다섯 target의 선택·재생·교체·Level 이탈 정리를 확인한다.
