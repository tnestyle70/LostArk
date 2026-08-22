# 차원술사 T Server-authoritative Ground Target 구현 계획

## 1. 목표와 종료 증거

차원술사 `T / 2050500 업의 경계`는 키를 누르는 즉시 시전하지 않는다. 첫 T 입력은 typed
ground-target state만 열고, Client가 표시한 점을 LMB로 확정했을 때만 기존 `C2S_USE_SKILL` 경로에
`GROUND_POINT` intent를 실어 보낸다. RMB는 targeting state와 preview만 지우며 packet, sequence,
resource, cooldown을 전혀 소비하지 않는다.

완료 조건은 다음 수직 슬라이스가 한 revision으로 이어지는 것이다.

```text
Data/Balance/PlayerSkillTargeting.json
-> Publish-GameplayBalance.ps1 / SKILLTARGET bootstrap v16
-> CPlayerSkillCatalog / CGROUND_TARGETING_STATE
-> IPlayerCommandSink / canonical C2S_USE_SKILL protocol v31
-> CGameRoom / CPlayerSkillSystem finite-range-navigation admission
-> SERVER_PLAYER approved target XYZ
-> PLAYER_SNAPSHOT approved target XYZ
-> CClientReplication / CCharacter skill_target pseudo anchor
-> DimensionMaster animevent target-root effect
-> Server target-root damage shape
```

자동 종료 증거는 Debug/Release Shared+NetworkProtocolHarness, Server contract, Client build,
gameplay publisher Validate와 `git diff --check`다. Client/UI는
에이전트가 실행하지 않으며 preview의 형태·색·스케일은 사용자 육안 판정 전까지 visual PASS로 올리지
않는다.

## 2. 보존하는 원본 gameplay 수치

`Data/Balance/PlayerSkills.json`과 `DamageProfiles.json`은 수정하지 않는다.

| 필드 | 정본 값 |
|---|---:|
| skillId / inputSlot | `2050500 / T` |
| cooldownMs | `120000` |
| actionDurationMs | `4367` |
| hitTimeMs | `2858` |
| resourceCost / identityCost | `938 / 0` |
| maximumRange | `11.0m` |
| damage profile | `damage.player.2050500`, `100%` |

targeting 문서는 위 `maximumRange`와 exact join해야 한다. 값이 다르거나 ACTIVE skill이 아니거나 중복
owner이면 publisher와 Client/Server loader가 모두 fail-close한다.

## 3. G01 — Targeting 데이터와 provenance

새 파일 `Data/Balance/PlayerSkillTargeting.json`은 gameplay admission과 Client preview descriptor를
함께 소유한다.

```json
{
  "schema": "lostark.player-skill-targeting",
  "formatVersion": 1,
  "skills": [
    {
      "skillId": 2050500,
      "targetingKind": "GROUND_POINT",
      "maximumRange": 11.0,
      "requiresWalkable": true,
      "rangePreview": {
        "assetId": "Effect/DimensionMaster/Textures/FX_TEX_03/fx_e_ring_028.dds",
        "diameter": 22.0,
        "coverageChannel": "R",
        "validTint": [1.0, 0.72, 0.15, 0.7],
        "invalidTint": [1.0, 0.2, 0.1, 0.7],
        "assetIdentityBasis": "SOURCE_EXTRACTED",
        "usageBasis": "PROJECT_TUNED",
        "sourceEvidence": "Data/Effects/Imported/DimensionMaster/skill.2050500.source-receipt.json"
      },
      "targetPreview": {
        "assetId": "Effect/DimensionMaster/Textures/FX_TEX_HIGH_02/fx_k_fsm_magiccircle_02.dds",
        "diameter": 6.0,
        "coverageChannel": "R",
        "validTint": [1.0, 0.82, 0.25, 0.9],
        "invalidTint": [1.0, 0.12, 0.08, 0.9],
        "assetIdentityBasis": "RUNTIME_RESOURCE",
        "usageBasis": "PROJECT_TUNED",
        "sourceEvidence": ""
      }
    }
  ]
}
```

`fx_e_ring_028.dds`의 asset identity는 checked-in source receipt가 동일 `assetId`를 실제 소유하므로
`SOURCE_EXTRACTED`다. 하지만 22m diameter, tint, preview 사용법은 원본 occurrence exact가 아니라
`PROJECT_TUNED`다. `fx_k_fsm_magiccircle_02.dds`는 runtime resource 존재만 확인되므로 identity도
`RUNTIME_RESOURCE`, 사용법도 `PROJECT_TUNED`이며 source evidence를 주장하지 않는다. 두 grayscale
DDS 모두 `R`만 coverage로 사용하고 RGB/alpha를 hue나 visibility의 암묵적 정답으로 쓰지 않는다.

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`은 exact property set, stable Resources-relative DDS
path, finite tint/diameter/range, provenance 조합, receipt 안의 exact `assetId`, `PlayerSkills.maximumRange`
join을 검증한다. Server bootstrap은 texture를 모르게 하고 아래 행만 v16에 게시한다.

```text
SKILLTARGET  2050500  GROUND_POINT  11  1
```

## 4. G02 — Shared typed intent와 canonical snapshot

`Shared/Public/Network/PacketMessages.h`의 `SKILL_TARGET_INTENT_KIND`는 `AIM_POINT`, `GROUND_POINT`,
`END`를 가진다. 기존 모든 일반 스킬은 default `AIM_POINT`이고, ground target은 별도 packet을 만들지
않고 기존 `C2S_USE_SKILL`에 intent byte를 추가한다. protocol version은 31이다.

`PLAYER_SNAPSHOT`은 `hasSkillTarget`과 XYZ를 가진다. present target은 `SKILL` action에서만 허용하고,
absent target은 세 좌표가 exact zero여야 한다. Writer/Reader는 unknown intent, non-finite target,
dirty absent coordinates, target outside SKILL, truncated payload를 거부하고 기존 destination을 보존한다.

## 5. G03 — Client 입력 상태와 preview renderer

### 5.1 `CGROUND_TARGETING_STATE`

`Client/Public/PlayerController.h`에 둔 pure state는 DirectInput, network, navigation, renderer를 소유하지
않는다.

- `Begin`: valid skill/range만 stage하고 inactive state를 부분 변경하지 않는다.
- `Update_Cursor`: caster XZ 기준으로 cursor를 정확히 11m에 clamp하고 nav 승인 전 confirm을 닫는다.
- `Apply_WalkableSample`: 요청 XZ와 1mm 안에서 같은 sample만 승인하고 nav Y를 보존한다.
- `Invalidate_Cursor`: active state는 유지하되 invalid red preview와 confirm 금지를 만든다.
- `Cancel`: default dormant state로 원자 복귀한다.

`CPlayerController::Update`는 T edge에서 `Begin`과 preview texture stage만 수행한다. active state에서는
LMB fresh edge와 valid sample이 동시에 있을 때만 `Request_UseGroundTargetSkill`을 호출한다. 성공한 send
뒤에만 action sequence를 증가시킨다. RMB fresh edge는 `Cancel_GroundTargeting`만 호출한다. 다른 skill,
LMB basic attack과 Esther command는 targeting 중 제출하지 않고 physical key edge만 계속 갱신한다.

### 5.2 `CSkillGroundTargetPreview`

새 `Client/Public/SkillGroundTargetPreview.h`, `Client/Private/SkillGroundTargetPreview.cpp`,
`Shader_VtxSkillGroundTargetPreview.hlsl`은 class-neutral 두 world quad를 소유한다.

- caster 중심 22m `fx_e_ring_028` range ring
- clamped/nav-sampled cursor 중심 6m `fx_k_fsm_magiccircle_02` marker
- valid yellow / invalid red
- `R` channel explicit coverage, read-only depth, two-sided alpha blend
- 두 texture load가 모두 성공한 뒤만 stage commit하고, 실패하면 targeting state를 닫는다.

Prototype/shader는 `CMainApp` static registry에 한 번 등록한다. 실제 controller가 있는 Bern,
Character Select, Development, Valtan Arena는 같은 generic object를 자기 level layer에 clone한다.
`Level_ValtanArena.cpp`의 변경은 이 generic object 초기화 6행뿐이며 Valtan authored/candidate/cue 데이터는
수정하지 않는다.

## 6. G04 — Server authority와 rollback

`CGameplayCatalog`은 `SKILLTARGET`을 기존 skill row와 exact join하고 `PLAYER_SKILL_DEFINITION`에 intent,
target maximum range, walkable requirement를 둔다.

`CPlayerSkillSystem::TryResolveSkillTarget`은 gameplay state를 변경하기 전에 다음 순서로 검사한다.

1. command intent와 catalog intent exact match
2. finite XZ와 loaded authoritative navigation
3. caster XZ에서 squared distance `<= 11m`
4. 요청 XZ가 속한 cell의 current walkability
5. 동일 XZ와 authoritative nav height를 `SERVER_NAV_POINT`로 stage

실패하면 sequence, resource, cooldown, action, current skill, target가 모두 이전 값이다. pending combo
command도 stage 시점과 실제 start 시점에 같은 검증을 다시 수행한다. 성공 start에서만 resource/cooldown과
함께 `SERVER_PLAYER.hasSkillTarget/XYZ`를 commit한다. duplicate/stale sequence, unknown skill/intent,
non-finite, out-of-range, non-walkable은 모두 거부한다.

damage hit-shape와 fallback circle의 origin은 target가 있으면 approved XZ, 없으면 caster XZ다. action end,
death, knockdown, fall, class change, revive, trigger move와 room reset은 target를 canonical false/zero로 지운다.

## 7. G05 — 승인 target의 Client presentation

`CGameRoom::Broadcast_WorldSnapshot`은 실제 SKILL action이면서 committed target가 있는 동안만 target XYZ를
복제한다. `CClientReplication`은 이 값을 `CCharacter::Apply_NetworkAction`으로 전달한다. Character는
catalog가 ground target을 요구하는 skill에 target가 없거나, non-ground skill에 target가 있거나, 같은
action start tick의 target가 변하면 snapshot presentation을 fail-close한다.

`skill_target`은 bone tag가 아니라 `CCharacter::Try_Get_SkillTargetRoot`가 만드는 pseudo anchor다.
위치는 Server-approved XYZ, 회전은 action-facing snapshot이다. T animevent는 다음 exact tuple로 바꾼다.

```text
effect.dimensionmaster.skill.2050500.unified
anchor="skill_target" follow=snapshot orientation=anchor
```

따라서 summon effect와 Server damage는 local cursor 기억값이 아니라 같은 승인 target root를 사용한다.

## 8. 프로젝트 등록과 public 문서

새 Client C++/HLSL/JSON은 `Client.vcxproj`와 `.filters`에 각각 `ClInclude`, `ClCompile`, `FxCompile`,
`96.DataFiles/Balance`로 등록한다. public 계약은 `CLAUDE.md`와
`.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, 완료 상태는 master RESULT와 전용 RESULT에 반영한다.

## 9. 검증 순서

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate

msbuild Shared/Shared.vcxproj /p:Configuration=Debug /p:Platform=x64
msbuild Tools/NetworkProtocolHarness/NetworkProtocolHarness.vcxproj /p:Configuration=Debug /p:Platform=x64
Tools/NetworkProtocolHarness/Bin/x64/Debug/NetworkProtocolHarness.exe

msbuild Server/Server.vcxproj /p:Configuration=Debug /p:Platform=x64
Server/Bin/Debug/Server.exe --dimensionmaster-ground-target-contract

msbuild Client/Default/Client.vcxproj /p:Configuration=Debug /p:Platform=x64
```

같은 순서를 Release로 반복하고 JSON/XML parse, exact PlayerSkills/Damage hash 보존,
`git diff --check`를 수행한다. Client/UI 실행과 visual PASS 판정은 하지 않는다.
