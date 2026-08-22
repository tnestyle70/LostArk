# 차원술사 T Server-authoritative Ground Target 구현 결과

기준 커밋: `e1b81c394f7fcc8d78a1843378bc6f063a5a2f23`

구현 브랜치: `codex/dimension-t-ground-target`

상태: 코드·자동 검증 완료, 사용자 Client 시각 판정 대기

## 1. 구현 완료 범위

### 1.1 데이터와 publisher

- `Data/Balance/PlayerSkillTargeting.json` format 1을 추가했다.
- `2050500 / T`를 `GROUND_POINT`, 최대 사거리 `11m`, walkable 필수로 기존
  `PlayerSkills.json` 행과 exact join한다.
- 사거리 preview는 `fx_e_ring_028.dds`, cursor preview는
  `fx_k_fsm_magiccircle_02.dds`이며 두 DDS의 `R`만 coverage로 사용한다.
- `fx_e_ring_028` asset identity만 source receipt로 증명한다. 두 preview의 크기·tint·사용법은
  모두 `PROJECT_TUNED`이고 magic circle identity는 `RUNTIME_RESOURCE`다.
- publisher는 exact property set, finite range/diameter/tint, Resources-relative asset ID,
  provenance/evidence, ACTIVE owner와 maximumRange 일치를 검사한 뒤 Server bootstrap v16에
  `SKILLTARGET 2050500 GROUND_POINT 11 1`만 게시한다. texture와 tint는 Server에 게시하지 않는다.
- 기존 `PlayerSkills.json`과 `DamageProfiles.json`은 수정하지 않았다. working blob과 기준 blob은 각각
  `05a4cf339ffc538ecd5a914777a57960f0b40e7b`,
  `cbe31f632f95b0f546041eb734937ec0fb0e52b5`로 일치한다.

### 1.2 입력과 preview

- 첫 T edge는 packet, action sequence, resource, cooldown을 소비하지 않고
  `CGROUND_TARGETING_STATE`만 연다.
- cursor XZ는 caster에서 정확히 11m에 clamp하고, Client navigation이 같은 XZ의 walkable cell과 Y를
  승인하기 전에는 confirm할 수 없다.
- LMB fresh edge와 valid nav sample이 함께 있을 때만 typed ground-target command를 보낸다.
  전송 성공 뒤에만 action sequence를 증가시킨다.
- RMB fresh edge는 preview/state만 지우며 packet을 보내지 않는다. targeting 중 다른 quick skill,
  LMB basic attack, Esther command는 edge만 동기화하고 제출하지 않는다.
- gameplay command 비활성화, character/sink/preview 소실, invalid/dead/not-combat-ready snapshot,
  Server action 시작, local character rebind와 Level 수명 종료에서 preview를 정리한다.
- `CSkillGroundTargetPreview`는 기존 GameObject/Shader/VIBuffer 경로로 22m range ring과 6m cursor marker를
  BLEND group에 제출한다. shader는 two-sided, depth-read-only alpha이며 grayscale `R`을 explicit
  coverage로 사용한다.

### 1.3 Shared와 Server authority

- protocol v31의 기존 `C2S_USE_SKILL`에 `AIM_POINT | GROUND_POINT` intent byte를 추가했다.
- `PLAYER_SNAPSHOT`은 Server-approved target XYZ와 presence bit를 복제한다. target가 없으면 XYZ는 exact
  zero이고, target가 있으면 action은 반드시 `SKILL`이다.
- Server는 gameplay state를 바꾸기 전에 intent exact match, finite XZ, loaded navigation, caster 기준
  11m, current walkability를 검증하고 authoritative nav Y를 stage한다.
- 실패하면 sequence, action, resource, cooldown, current skill과 기존 target를 모두 보존한다.
- combo 중 buffer된 T도 stage 시점과 실제 action start 시점에 같은 target 검증을 다시 수행한다.
- 성공한 start에서만 cost/cooldown과 target XYZ를 한 번 commit한다. hit-shape와 fallback damage circle은
  caster가 아니라 승인 target XZ를 origin으로 사용한다.
- natural end, invalid/dead skill update, knockdown, fall, monster/boss death hit, trigger move, revive,
  class change와 audition reset 경로에서 target를 canonical false/zero로 지운다. disconnect/Level teardown은
  해당 `SERVER_PLAYER`/Client controller owner 자체를 폐기한다.

### 1.4 Product presentation

- `DimensionMaster.animevents`의 `pc_sp_m_00_sk_sk_dimensionprison` cue를
  `anchor="skill_target" follow=snapshot stop=natural`로 변경했다.
- `skill_target`은 bone 이름이 아니다. `CCharacter::Try_Get_SkillTargetRoot`가 Server-approved XYZ와
  action-facing yaw로 만드는 pseudo anchor다.
- 같은 actionStartTick에서 target가 바뀌거나, ground-target skill에 target가 없거나, 일반 skill에 target가
  붙으면 Character presentation은 해당 snapshot 적용을 거부하고 직전 정상 상태를 보존한다.
- clip presentation이 unavailable이어도 fallback Effect는 caster root가 아니라 같은 `skill_target`
  snapshot anchor를 사용한다.

## 2. 리소스 identity 확인

팀 관리 integration runtime root에서 두 물리 DDS의 존재를 read-only로 확인했다. 이 파일들은 Git 변경에
포함하지 않는다.

| Resources-relative asset ID | bytes | SHA-256 |
|---|---:|---|
| `Effect/DimensionMaster/Textures/FX_TEX_03/fx_e_ring_028.dds` | 32,896 | `1F7F2DC5A4F41BF2BBF47F0B673412D49DE38469902C601A150FB55B57275361` |
| `Effect/DimensionMaster/Textures/FX_TEX_HIGH_02/fx_k_fsm_magiccircle_02.dds` | 131,200 | `299940CB54D4F0ECF608B1191EFF15114283162229673050899395058056DB23` |

## 3. 실행한 자동 검증

| 검증 | Debug | Release |
|---|---|---|
| Engine build | PASS | PASS |
| `UpdateLib.bat` | PASS | PASS |
| Shared build | PASS | PASS |
| NetworkProtocolHarness 전체 | failures 0 PASS | failures 0 PASS |
| Server build | PASS | PASS |
| `Server.exe --dimensionmaster-ground-target-contract` | 11/11 PASS | 11/11 PASS |
| Client build | errors 0 PASS | errors 0 PASS |

추가 검증:

- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS, 6 profiles / 231 skills / 108 damage profiles.
- target protocol writer/reader: unknown intent, non-finite, truncation, dirty absent XYZ,
  target outside SKILL과 destination-preserving failure를 포함해 PASS.
- Server focused: wrong intent, 11m 초과, non-finite, unknown skill, blocked nav, duplicate sequence,
  buffered target 재검증, approved XYZ snapshot, target-root damage, natural end/death/knockdown cleanup PASS.
- Client source/runtime provenance, Product pseudo-anchor tuple, 11m clamp, navigation sample, invalid/red preview와
  RMB reset consumer는 Debug/Release Client 컴파일·링크로 연결을 확인했다.
- `PlayerSkillTargeting.json` JSON parse, `Client.vcxproj/.filters` XML parse와 신규 C++/HLSL/Data 등록 PASS.
- 생성된 `worlddestruction*.json`은 이 변경에서 제외하고 원래 tracked 상태로 복원했다.
- Client/UI는 실행하거나 조작하지 않았다.

기존 C4819/C4828와 DirectXTK LNK4099 warning은 남지만 새 compile/link error는 0이다.

## 4. 사용자 수동 판정 대기

자동 검증은 입력·authority·resource/shader compile·target root까지 닫았다. 실제 화면의 크기, 색, 바닥
밀착과 시인성은 사용자만 최종 판정한다.

사용자 확인 경로:

1. 사용자가 `Server + Client`를 실행하고 차원술사로 Server-approved 전투 Level에 진입한다.
2. T 한 번으로 caster 중심 range ring과 cursor magic circle이 열리는지 확인한다.
3. cursor를 11m 밖으로 이동해 marker만 경계에 clamp되는지 확인한다.
4. 막힌 navigation 위치에서 marker가 red가 되고 LMB가 시전을 만들지 않는지 확인한다.
5. valid 위치에서 LMB를 눌러 summon Effect와 damage가 같은 지점에 생기는지 확인한다.
6. T 후 RMB에서 preview만 사라지고 action/cooldown/resource가 소비되지 않는지 확인한다.

사용자의 서면 관찰 전에는 visual PASS, first pixel 또는 원본 색감 동일 판정으로 기록하지 않는다.

## 5. 통합 결과와 남은 경계

- 선별 integration 브랜치에서 다른 Effect family 변경과 의미 단위로 합쳐 Debug/Release 전체 빌드,
  NetworkProtocolHarness와 Server contract, gameplay/Effect publisher를 통과했다.
- 남은 경계는 사용자가 실제 Client에서 preview 크기·색·바닥 밀착과 target damage 일치를 육안 판정하는 것이다.
- Valtan authored/candidate/pattern cue 파일은 이 작업에서 수정하지 않았다. `Level_ValtanArena.cpp`는
  class-neutral preview clone 초기화만, `ValtanBrain.cpp`는 모든 boss death hit에 공통인 player target
  cleanup 한 줄만 변경했다.
