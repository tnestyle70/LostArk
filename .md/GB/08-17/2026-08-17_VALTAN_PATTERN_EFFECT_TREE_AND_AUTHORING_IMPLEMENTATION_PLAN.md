# 2026-08-17 발탄 패턴 Effect 트리와 저작 연결 구현 계획

branch: `feature/effect-tool-texture-kind-filter`

All Effects를 발탄의 실제 페이즈·패턴·스테이지 축으로 재구성하고, 그 축에 Effect 문서를
이름으로 붙인 뒤, 원본 조사로 확보한 mesh/texture 자산을 slot에 연결한다.

## G00. 사용자 이해 검증 — 대부분 맞고, 두 곳이 다르다

### G00.1 맞는 것

```text
패턴 = 캐릭터의 스킬 하나에 해당한다                      맞다
스테이지마다 애니메이션 clip과 시간이 있고 그 시간에 Effect가 재생된다   맞다
6방향 이후 전멸은 애니메이션이 나뉜 구조다                 맞다
발탄은 캐릭터와 달리 페이즈·패턴 축 필터가 더 필요하다      맞다
```

`ValtanEncounter.json`이 이미 그 구조를 갖고 있다. 추가 설계가 아니라 **이미 있는 축을
All Effects가 안 쓰고 있는 것**이다.

```text
pattern
  patternId              VALTAN_WHIRLWIND
  actionId               valtan.attack.whirlwind
  minimumHealthBar       1        <- 페이즈 축
  maximumHealthBar       160
  triggerHealthBar       0        <- 0이 아니면 고정 기믹
  triggerOrder
  stages[]
    stageId              SPIN
    actionId             valtan.attack.whirlwind.active
    stageKind            WINDUP / ACTIVE / RECOVERY
    durationMs           1500     <- 애니메이션 시간
    hitShape             CIRCLE
    hitOuterRadius       10.0
    hitInnerRadius / hitAngleDegrees / hitLength / hitHalfWidth
    hitCount             4
    hitIntervalMs        350
    serverDamageProfileId  damage.valtan.circular-spin
```

즉 사용자가 "6방향 이후 전멸은 애니메이션이 두 개로 나뉘는 구조인가"라고 물은 것의 답은
**하나의 패턴 안에 스테이지가 여러 개**다. 실측 결과는 다음과 같다.

```text
valtan.mechanic.ghost-transition-15    stages=7
valtan.reactive.triple-counter         stages=7
valtan.mechanic.arena-break-109        stages=6
valtan.mechanic.center-grab-counter-64 stages=5
valtan.mechanic.floor-wipe-130         stages=5
valtan.mechanic.four-pillars-105       stages=4
valtan.attack.portal-rush              stages=4   <- 차원문
valtan.attack.high-jump                stages=4   <- 점프 도끼
전체 32 패턴 / 121 스테이지 (ACTIVE 50, WINDUP 39, RECOVERY 32)
```

### G00.2 다른 것 하나 — 공격 콜라이더는 이미 있다

사용자가 "이펙트에 공격 콜라이더를 달자"고 했는데, **콜라이더는 이미 스테이지가 소유한다.**
위 `hitShape/hitOuterRadius/hitCount/hitIntervalMs/serverDamageProfileId`가 그것이고
Server가 fixed tick에서 평가한다. 실측 분포는 다음과 같다.

```text
NONE 75  CIRCLE 23  CONE 7  BOX 6  RING 5  CROSS 5
```

따라서 이펙트에 콜라이더를 **새로 다는 것이 아니라**, 같은 `stage.actionId`를 공유해서
`Effect`와 `hitShape`가 자동으로 같은 시간축에 놓이게 하는 것이 맞다. 이것이
`AGENTS.md`의 "UI·Client가 제품 보스 판정을 직접 결정하지 않는다"와도 일치한다.

```text
stage.actionId  ─┬─> Valtan.patternbindings.json  -> 애니메이션 clip
                 ├─> Valtan.patterneffects.json   -> Effect 문서      (이번 작업)
                 └─> ValtanEncounter.json stage   -> hitShape + damage (이미 있음, Server 권위)
```

Effect Tool이 할 일은 콜라이더를 저작하는 것이 아니라 **읽어서 같이 보여주는 것**이다.
그러면 보스 담당자는 패턴 수치만 튜닝하고 나머지는 Server 골격이 처리한다.

### G00.3 다른 것 둘 — Trail 셰이더는 이미 있다

```text
Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl      존재
EFFECT_ELEMENT_KIND::TRAIL   Renderer 5 / Playback 6   구현됨
EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON                  저작 family 존재
```

`LIGHT`, `SCREEN_POST`, `DECAL`도 element kind와 renderer/playback이 있다.

```text
TRAIL        Renderer=5 Playback=6
LIGHT        Renderer=4 Playback=5
SCREEN_POST  Renderer=4 Playback=5
DECAL        Renderer=8 Playback=3
```

**실제 구멍은 저작 family 목록이다.** 현재 6개뿐이다.

```text
MESH, SPRITE, MESH_PARTICLE, SPRITE_PARTICLE, LOCAL_DECAL, TRAIL_RIBBON
```

`LIGHT`와 `SCREEN_POST`는 런타임에 존재하지만 Tool에서 만들 수 없다. Camera는 element
kind 자체가 없다. 따라서 이번 계획의 확장 대상은 **Trail이 아니라 Light 저작 family 하나**다.
사용자가 말한 "원본 복원이 아니라 슬롯 확장" 원칙에 정확히 맞는 최소 범위다.

Camera는 이번 범위에서 제외한다. element kind 신설 + renderer + playback + 카메라 소유권
경계까지 필요해 별도 수직 슬라이스이며, 지금 필요한 소비자가 없다.

## G01. 목표와 종료 증거

```text
목표
  1  All Effects의 Valtan을 페이즈/패턴/스테이지 3단 트리로 재구성
  2  스테이지 행에 durationMs, hitShape, damage profile을 함께 표시
  3  32 패턴 121 스테이지 전부에 Effect 자산 ID를 부여하고 문서를 생성
  4  원본 조사로 확보한 mesh/texture를 element slot에 연결
  5  LIGHT 저작 family 추가

종료
  All Effects > Valtan에서 페이즈 -> 패턴 -> 스테이지로 접히고,
  각 스테이지 행에서 Open for Editing이 동작하며,
  휠윈드 3 스테이지의 Effect가 원본 texture로 채워진 채 열린다.

비목표
  Product admission 해제, 게임 내 재생, Camera element, 수치 완전 복원
```

## G02. 트리 구조 — 페이즈 축을 무엇으로 할 것인가

발탄은 체력바 160개이고 패턴이 바 범위로 게이트된다. 별도 `phase` 필드는 없다.
따라서 **healthBar 구간이 곧 페이즈**이며, 새 개념을 만들지 않는다.

고정 기믹 8개가 실측으로 확정된 페이즈 경계다.

```text
bar 159   VALTAN_ARMOR_BREAK_OPENING     stages=3
bar 115   VALTAN_FLOOR_WIPE_130          stages=5
bar 109   VALTAN_ARENA_BREAK_109         stages=6
bar 100   VALTAN_FOUR_PILLARS_105        stages=4
bar  73   VALTAN_MAGIC_ORB_STAGGER_76    stages=3
bar  62   VALTAN_CENTER_GRAB_COUNTER_64  stages=5
bar  30   VALTAN_ARENA_BREAK_33          stages=4
bar  14   VALTAN_GHOST_TRANSITION_15     stages=7
```

트리는 다음 3단으로 한다. 4단 이상은 접었다 펴는 비용만 늘고 정보가 늘지 않는다.

```text
[Gimmick] bar 159  VALTAN_ARMOR_BREAK_OPENING
    WINDUP    valtan.attack...windup     800ms   NONE
    ACTIVE    valtan.attack...active     1500ms  CIRCLE r=10 x4 @350ms
    RECOVERY  valtan.attack...recovery   1000ms  NONE

[Rotation] bar 1-160  VALTAN_WHIRLWIND
    ...
```

1단은 `Gimmick(triggerHealthBar 내림차순)` 다음 `Rotation(상시 패턴)` 두 그룹이다.
`triggerHealthBar != 0`이 Gimmick 판정이며 추측이 아니라 문서 필드다.

## G03. Effect 자산 ID 명명

기존 `effect.valtan.pattern.420633.active`는 원본 actionId 숫자를 쓴다. 사람이 못 읽는다.
스테이지 actionId가 이미 안정적이고 읽히므로 그것을 그대로 쓴다.

```text
규칙   effect.valtan.<pattern-slug>.<stage-slug>

예시
  effect.valtan.whirlwind.windup
  effect.valtan.whirlwind.active
  effect.valtan.whirlwind.recovery
  effect.valtan.four-slash.active
  effect.valtan.high-jump.throw
  effect.valtan.arena-break-109.wipe
```

`valtan.attack.whirlwind.active` -> `effect.valtan.whirlwind.active`로,
`valtan.` 접두와 `attack/mechanic/reactive` 분류를 떼고 이어 붙인다. 분류는 트리가 보여주므로
ID에 중복 저장하지 않는다.

기존 `effect.valtan.pattern.420633.active`는 **이름을 바꾸지 않는다.** 이미 binding과
런타임 catalog가 그 ID를 참조하고 Product canary로 지정돼 있다. 새 문서만 새 규칙을 쓰고,
420633은 트리에서 `whirlwind.active`의 기존 문서로 함께 표시한다.

## G04. 변경할 파일

```text
Client/Public/ValtanPatternTree.h            신규   페이즈/패턴/스테이지 뷰 모델
Client/Private/ValtanPatternTree.cpp         신규   ValtanEncounter.json 파싱과 조인
Client/Public/Effect_Tool.h                  트리 상태 멤버, LIGHT family
Client/Private/Effect_Tool.cpp               Valtan 트리 렌더, LIGHT family 분기
Client/Default/Client.vcxproj                신규 2파일 등록
Client/Default/Client.vcxproj.filters        신규 2파일 등록
Data/Animation/Authored/Valtan/Valtan.patterneffects.json   binding 추가
Data/Effects/Authored/effect.valtan.*.effect.json           신규 Effect 문서
```

`ValtanEncounter.json`은 **읽기만 한다.** Server 권위 문서이며 이번 변경이 쓰지 않는다.

## G05. `CValtanPatternTree` 계약

한 줄 책임: `ValtanEncounter.json`, `Valtan.patternbindings.json`,
`Valtan.patterneffects.json` 세 문서를 조인해 All Effects가 그릴 수 있는 읽기 전용 뷰를 만든다.

이 계층에 있어야 하는 이유는 세 문서의 조인 규칙이 UI가 아니라 데이터 계약이기 때문이다.
`Effect_Tool.cpp`가 직접 세 파일을 열면 같은 조인이 Tool 안에 흩어진다.

```text
struct VALTAN_STAGE_VIEW
  strStageId            SPIN
  strActionId           valtan.attack.whirlwind.active     조인 키
  eStageKind            WINDUP / ACTIVE / RECOVERY
  iDurationMs           1500
  strHitShape           CIRCLE                              표시 전용
  fHitOuterRadius / fHitInnerRadius / fHitAngleDegrees
  fHitLength / fHitHalfWidth
  iHitCount / iHitIntervalMs
  strServerDamageProfileId
  strRuntimeClipName    mesh_att_battle_20_03               patternbindings에서
  strEffectAssetId      effect.valtan.whirlwind.active      patterneffects에서, 없으면 빈 값
  EffectDocumentPath                                        없으면 빈 경로

struct VALTAN_PATTERN_VIEW
  strPatternId / strDisplayName / strActionId
  iMinimumHealthBar / iMaximumHealthBar / iTriggerHealthBar
  bGimmick                triggerHealthBar != 0
  Stages[]

class CValtanPatternTree
  static bool_t Load(VALTAN_PATTERN_TREE_VIEW& out, std::string& outStatus);
```

불변식은 셋이다.

```text
스테이지의 actionId가 조인 키다. index나 순서로 조인하지 않는다.
Effect가 없는 스테이지도 행으로 남긴다. 비어 있음이 저작 대상 목록이다.
hitShape 필드는 표시 전용이며 Tool이 값을 쓰지 않는다.
```

로드는 `parse -> validate -> stage -> commit`이고 실패하면 이전 트리를 보존한다.
기존 `Refresh_ValtanBossPatternEffects()`의 실패 보존 규칙을 그대로 따른다.

## G06. All Effects 렌더 변경

현재 `Render_AllEffectsWindow`의 `m_bAllEffectsValtanBossSelected` 분기는 두 절이다.

```text
Valtan Boss Patterns      m_ValtanBossPatternEffects 순회, 매핑된 행만
Saved Valtan Authoring    매핑 안 된 effect.valtan.* 문서
```

이 둘을 유지한 채 **위에 3단 트리를 추가**한다. 기존 두 절은 그대로 두어 회귀를 만들지 않는다.

```text
Valtan Pattern Tree                      신규
  [Gimmick] bar 159 VALTAN_ARMOR_BREAK_OPENING
    WINDUP  800ms  NONE                       (Effect 없음) [Create Effect]
    ACTIVE 1500ms  CIRCLE r=10 x4 @350ms      effect.valtan...active  [Open for Editing]
Valtan Boss Patterns                     기존 유지
Saved Valtan Authoring                   기존 유지
```

스테이지 행의 버튼은 두 가지다.

```text
Effect 있음   Render_ValtanAuthoringOpenButton(Path, AssetId)   G07에서 만든 것 재사용
Effect 없음   Create Effect  ->  빈 Authored 문서 생성 + binding 추가
```

검색창은 patternId / stageId / actionId / clipName / effectAssetId를 모두 매칭한다.

## G07. Effect 문서 일괄 생성

121 스테이지 전부에 문서를 만들지 않는다. `hitShape == NONE`이면서 원본 파티클 시스템이
0개인 스테이지는 시각 효과가 없는 것이 원본이다. 휠윈드 windup이 실측으로 그렇다.

```text
생성 대상   원본 catalog에 clip 조인 결과 시스템이 1개 이상인 스테이지
확인 방법   Valtan.effect-resource-catalog.json 의 sourceSystems[].clipNames 를
            mesh_ 접두 제거 + 소문자로 정규화해 스테이지 clip과 조인
```

생성되는 문서는 element 0개의 빈 Authored v13 문서이고, `Create Effect` 버튼으로 스테이지
단위로 만든다. 일괄 생성 스크립트를 만들지 않는다. 소비자가 없는 121개 빈 문서보다
필요한 순간에 하나씩 만드는 편이 저작 흐름에 맞고 되돌리기 쉽다.

## G08. mesh/texture 연결 — 보수적 원칙

`.md/GB/08-17/2026-08-17_VALTAN_WHIRLWIND_SLASH_AUTHORING_SHEET.md`가 조인 방법과
휠윈드·검격 결과를 이미 갖고 있다. 이번에는 그 조인을 Tool 안에서 스테이지 행에 붙인다.

```text
스테이지 선택 -> 그 clip을 쓰는 원본 시스템 목록 -> 각 시스템이 참조한 DDS/wmodel 목록
                                                    -> Resource Library 필터에 주입
```

즉 사용자가 스테이지를 고르면 Resource Library가 **그 스테이지의 원본 후보로 좁혀진다.**
346개에서 5~10개가 된다. 이것이 사용자가 말한 "자산의 활용"이다.

원칙은 셋이다.

```text
자동 바인딩하지 않는다. 후보를 좁혀 보여주기만 하고 Bind는 사람이 누른다.
수치는 복원하지 않는다. slot과 family만 연결하고 Detail 수치는 튜닝 대상으로 둔다.
근거가 없으면 비워 둔다. approximate 값을 채워 넣지 않는다.
```

휠윈드 active의 확보된 자산은 다음과 같다. 실물 존재까지 확인했다.

```text
par_n_rpbf_wwind_01   회오리 본체   fx_d_atypical_028.dds   128x128 DXT1
                                     fm_m_trail_002.wmodel
par_n_mrhg_trail_01   궤적           fx_a_trail_006.dds      256x256 DXT1
par_n_rpbf_dust_01_01 먼지           fx_e_fluid_007.dds      256x256 DXT5
                                     fx_e_noise_008.dds      256x256 DXT5
```

## G09. LIGHT 저작 family 추가

런타임은 이미 `EFFECT_ELEMENT_KIND::LIGHT`를 렌더/재생한다. Tool에서 만들 수만 없다.

```text
EFFECT_AUTHORING_FAMILY 에 LIGHT 추가
AuthoringFamily_Kind(LIGHT) -> EFFECT_ELEMENT_KIND::LIGHT
AuthoringFamily_RequiresMesh(LIGHT) -> false
AuthoringFamily_AllowsSlot(LIGHT, slot) -> false          텍스처 slot 없음
Element Type 라디오에 Light 추가
Effect Detail에 반경/색/강도 편집 (기존 Detail 구조 사용)
```

원본 복원이 아니라 슬롯 확장이라는 사용자 원칙을 지킨다. 원본 발탄이 쓴 `par_mp_light_05_l`을
LIGHT element로 흉내내지 않는다. 그것은 별도 파티클 시스템이며 이번 확장과 무관하다.

Camera와 SCREEN_POST는 추가하지 않는다. SCREEN_POST는 런타임에 있으나 화면 전체 후처리를
저작 문서가 소유하는 경계 결정이 먼저 필요하고, 지금 소비자가 없다.

## G10. 벽 파괴 trigger box와의 관계

사용자가 물은 "패턴 발생 시 trigger box로 벽을 부순다"는 현재 계약과 다르다.

```text
현재   Data/Worlds/<AreaId>/Gameplay.world.json 의 triggerBox 는
       movePlayer / changeLevel / activateSpawnGroup / activateEncounter 4 action만 지원
       destroyable 은 publisher가 거부한다 (AGENTS.md)

Valtan World Destruction 은 제품 Server와 분리된 Destruction Model View이며
       LV_LUT_HEARTRB_ED.destructionsimulation.json format v2 를 쓴다
```

즉 벽 파괴는 **패턴이 trigger box를 만드는 구조가 아니라**, 패턴 스테이지가 Server에서
파괴 이벤트를 확정하고 Client가 표현하는 구조여야 한다. `VALTAN_ARENA_BREAK_109`와
`VALTAN_ARENA_BREAK_33`이 이미 패턴으로 존재하므로 축은 이미 맞다.

이번 계획에서는 하지 않는다. dynamic navigation과 Shared replication이 닫히기 전까지
publisher가 `destroyable`을 계속 거부하기 때문이다. 이펙트 축을 먼저 닫는 것이 맞다는
사용자 판단이 옳다.

## G11. Server 골격에 대한 사용자 방향 확인

사용자 방향은 다음과 같았다.

```text
패턴에 대한 튜닝만 할 수 있게 하고 나머지는 전부 서버에서 골격 구현,
애니메이션·이펙트·공격 콜라이더·데미지까지 처리 가능하게
```

이것은 이미 구현된 계약이다. 새로 만들 것이 없다.

```text
ValtanEncounter.json   패턴/스테이지/hitShape/damage profile      튜닝 대상
Publish-GameplayBalance.ps1                                       검증·publish
Server CValtanPatternRuntime                                      fixed tick 판정
S2C_WORLD_SNAPSHOT                                                action/phase 복제
Client CValtan                                                    표현만
```

따라서 이번 작업은 Server 골격을 새로 설계하는 것이 아니라, **이미 Server가 소유한 축을
Effect 저작 화면에 그대로 노출**해 보스 담당자가 같은 축에서 튜닝하게 만드는 것이다.

## G12. 구현 순서와 검증

```text
G12.1  CValtanPatternTree 신규 2파일 + vcxproj/filters 등록
       검증  Client x64 Debug 빌드, 32 패턴 121 스테이지 파싱 로그

G12.2  All Effects에 3단 트리 렌더
       검증  Valtan 선택 시 Gimmick 8 + Rotation 24 그룹이 보이고
             스테이지 행에 durationMs/hitShape가 표시된다

G12.3  스테이지 행 Create Effect / Open for Editing
       검증  휠윈드 active에서 기존 420633 문서가 열린다
             빈 스테이지에서 Create 후 Data Files에 새 문서가 보인다

G12.4  스테이지 선택 시 Resource Library 원본 후보 필터
       검증  휠윈드 active 선택 시 후보가 346 -> 10 이하로 줄고
             fx_d_atypical_028 / fm_m_trail_002 가 포함된다

G12.5  LIGHT 저작 family
       검증  Element Type에 Light가 보이고 생성 후 Save/Reload가 유지된다

각 단계마다  git diff --check, Client x64 Debug 빌드
전체 종료 후  사용자 육안 검증 (에이전트는 판정하지 않는다)
```

## G12.6 인터뷰 확정 사항 (2026-08-17)

사용자 인터뷰로 네 가지를 확정했다. G08의 "자동 바인딩하지 않는다" 원칙은
"수치는 복원하지 않되 얻을 수 있는 정보는 채운다"로 사용자가 교정했다.

```text
element 생성 범위   슬롯 매핑 가능한 것만 = 텍스처 1~5장인 시스템 188개
                    6장 초과 시스템은 element를 만들지 않고 트리에 수동 저작 대상으로 표시
슬롯 배치           파일명 규칙 기반 자동 배치
family 결정         원본 시스템의 .wmodel 참조 유무 (Mesh Particle 178 / Sprite Particle 479)
420633 문서         ID 유지, 트리의 whirlwind.active 자리에 표시
```

### 슬롯 배치 규칙

파일명 토큰으로 결정한다. 근거는 `2026-08-17_EFFECT_TOOL_TEXTURE_KIND_FILTER_RESULT.md`의
kind 토큰 실측이다.

```text
우선순위 1  normal / _n. / _n_        -> Noise      ATI2 노멀맵은 Base에 넣으면 안 된다
우선순위 2  noise / mirnoise / flow   -> Noise      이미 Noise가 찼으면 Mask
우선순위 3  decal                     -> Base
우선순위 4  trail / atypical / glow   -> Base       Base가 비어 있으면 첫 장
나머지                                 -> Mask -> Emissive -> Dissolve 순서로 채움
6장 초과                               -> element 생성하지 않음
```

`meshModel` 슬롯은 family가 Mesh Particle일 때 시스템이 참조한 `.wmodel`을 넣는다.

### 420633 판단 근거

이름을 바꾸면 런타임 catalog의 `contentSha256` 행, `Valtan.patterneffects.json` binding,
Product canary 세 곳을 같은 변경에서 갱신해야 하고 그 사이 Product 경로가 깨진다.
반면 사용자가 보는 일관성은 트리에서 달성되므로 ID 문자열을 통일할 실익이 없다.
새 문서만 `effect.valtan.<pattern-slug>.<stage-slug>` 규칙을 쓴다.

## G13. 이 계획이 하지 않는 것

```text
Product admission 해제와 게임 내 재생        Codex 세션의 게이트 제거 범위
Camera element / SCREEN_POST 저작            소비자 없음
벽 파괴 trigger box                          publisher가 destroyable 거부 중
수치 복원                                    튜닝으로 잡는다는 사용자 결정
121 스테이지 문서 일괄 생성                  필요한 것만 Create로
420633 문서 이름 변경                        런타임 catalog가 참조 중
```
