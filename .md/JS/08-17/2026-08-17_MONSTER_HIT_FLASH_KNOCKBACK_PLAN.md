# 2026-08-17 몬스터 피격 플래시·Server 넉백 PLAN

작성자: JS · branch `feature/monster-hit-flash-knockback` (main `840c4bd` 기준)
선행: `../08-16/2026-08-16_PLAYER_SKILL_HIT_SHAPE_SERVER_DAMAGE_RESULT.md` (히트 셰이프 Server 판정 완료)

## 목표

플레이어 스킬이 몬스터에 맞았을 때

1. **피격 플래시** — 맞은 몬스터 모델 전체가 흰색으로 짧게 빛났다 사라진다 (Client 표현 전용).
2. **넉백** — Server가 맞은 몬스터를 공격자 반대 방향으로 짧은 거리 밀어내고, 그 위치가 snapshot으로
   내려와 모든 Client가 같은 밀림을 본다 (Server authority, 사용자 결정 2026-08-17).

## 실측 근거 (2026-08-17)

- damage event: `Shared::DAMAGE_EVENT{iTargetNetEntityId, iAmount, fPosition*, isOutgoing}`이 매 tick
  `S2C_WORLD_SNAPSHOT.DamageEvents`로 온다 (`GameRoom.cpp:2469`, tick마다 `m_TickDamageEvents.clear()`).
  Client `CClientReplication::Apply_WorldSnapshot`(ClientReplication.cpp:936)은 모든 snapshot을 순서대로
  적용하고 `m_WorldEntities[NET_ENTITY_ID] -> {eKind, pNpc, pValtan}`을 가진다. **프로토콜 변경 불필요.**
- 플래시 재료: `Shader_VtxAnimMeshBinary.hlsl`의 `g_HasFullSurfaceEmissiveOverride/Color/Intensity`와
  `Client::DEFERRED_EMISSIVE_OVERRIDE`(DeferredMaterialRenderUtils.h). `CCharacter`가 스킬 발광에
  `m_ActionEmissiveOverride`(예: DimensionMaster 2050210 white 4.0)로 이미 사용. `CNpc::Render`와
  `CBody_Valtan::Render`는 `Bind_DeferredMaterialInputs(..., Profile)`까지만 넘기고 override는 nullptr.
- 몬스터 위치 authority: `CMonsterBrain::Update`(MonsterBrain.cpp)가 MovePath를 따라 `fPosition*`을 직접
  갱신, `CGameRoom::Update_WorldEntities`(GameRoom.cpp:3550) MONSTER 분기에서 호출. 피해 적용은
  `CPlayerSkillSystem::Update`의 `applyDamage` 람다(PlayerSkillSystem.cpp:571)이며 player 위치를 안다.
- 몬스터 수치 정본: `Data/Balance/MonsterProfiles.json`(basis `PROJECT_TUNED`, 5 profile) →
  `Publish-WorldGameplay.ps1` `Get-MonsterProfiles`(exact 12 property) → `*.spawngroupsbootstrap`
  `PROFILE` 13열 → `CSpawnGroupBootstrap`(13u strict, header version 1) → `MONSTER_RUNTIME_PROFILE`
  → `GameRoom.cpp:3374` staged entity. Valtan(BOSS)은 `BossProfiles.json` 별도 경로.
- Server에는 PhysX가 없다. 넉백은 nav grid 위 kinematic 이동이며 `CPlayerSkillSystem::Clamp_StepToWalkable`
  (루트모션 clamp)을 그대로 재사용한다. Client PhysX는 관여하지 않는다.
- Client `CNpc::Apply_NetworkState`는 위치를 snap한다 (보간 없음). 30 Hz × 수 cm 단위라 별도 보간 없이
  밀림이 보인다.

## 계약 (결정)

| 항목 | 값 | 이유 |
|---|---|---|
| 넉백 유무·거리·지속 정본 (G4, 사용자 결정) | 히트별 원작 값. `.animevents` HIT 행 `push`(ms, 0=넉백 없음)/`pushr`(cm, 음수=끌어당김) → `hitshapes.json` `pushMs`/`pushRange`(m) → `SKILLHIT` packed 12필드(bootstrap **v7**) → `PLAYER_SKILL_HIT::iPushMs/fPushRange` | 원작은 314 HIT 중 142행만 push>0; "타격마다"가 아니라 히트별 |
| 몬스터 배율 | `MonsterProfiles.json` profile별 `hitKnockbackScale` (0=슈퍼아머): 잡몹 3종 `1.0`, MINIBOSS_LUGARU `0.0` | 아키타입별 무게감 자리; PROJECT_TUNED라 receipt 무관 |
| 실제 밀림 | 거리 = `pushRange × hitKnockbackScale`, 지속 = `pushMs`, 등속. 셰이프 없는 스킬(maximumRange 원형)은 넉백 없음 | ~~G3 초안의 profile 거리(m)·상수 0.15 s는 G4에서 폐기~~ |
| Valtan(BOSS) | 넉백 없음, 플래시만 | 보스 패턴/leap 위치를 Server brain이 소유 |
| 방향 | 공격자(player) → 대상 XZ 단위벡터. 거리 0이면 player `fSkillAimDirection` | 원거리/근거리 모두 밀리는 방향이 자연스러움 |
| 중첩 | 넉백 중 다시 맞으면 remaining을 다시 채우고 방향 교체 (누적 없음) | 다단히트로 무한 밀림 방지 |
| 넉백 중 brain | 살아 있으면 `CMonsterBrain::Update`를 건너뜀 (=넉백 시간만큼 경직) | 추적 이동이 밀림을 상쇄하는 것 방지 |
| 이동 clamp | `Clamp_StepToWalkable`(navigation) 뒤 위치 갱신, y는 sample 결과 | 벽/낙사 밖으로 못 나감 |
| 플래시 | **실루엣 림만** 발광: `pow(1−N·V, 3) × intensity`, white, peak `4.0`, 지속 `0.12 s` 선형 감쇠, 재히트 시 리셋. 셰이더 `g_FullSurfaceEmissiveMaskMode` 1(림) / 0(기존 diffuse 밝기, Character 스킬 발광 유지) | 사용자 결정 2026-08-17: 전면 백화·텍스처 마스크·백화+림은 어색, 원작처럼 실루엣만 반짝 |
| 플래시 대상 | `isOutgoing == true` damage event의 MONSTER(CNpc)와 BOSS(CValtan body+weapon). NPC/플레이어 제외 | "몬스터가 맞았다" 표현 |
| 프로토콜 | 변경 없음 (protocol v13 유지) | 위치·damage event가 이미 실려 온다 |
| spawngroupsbootstrap | header version `1 → 2`, PROFILE 행 13열 → 14열 | Server strict parser와 publisher를 함께 올림 |

## G1. Client 피격 플래시

**수정 파일**: `Client/Public/Npc.h`, `Client/Private/Npc.cpp`, `Client/Public/Body_Valtan.h`,
`Client/Private/Body_Valtan.cpp`, `Client/Public/Valtan.h`, `Client/Private/Valtan.cpp`,
`Client/Private/ClientReplication.cpp`

- `Npc.h`: `#include "DeferredMaterialRenderUtils.h"` 추가. public에 `void Trigger_HitFlash();`
  (`Apply_NetworkState` 선언 바로 아래). private 멤버 `DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;`,
  `f32_t m_fHitFlashRemainingSeconds = 0.f;` (`m_pColliderCom` 바로 아래).
- `Npc.cpp`: 익명 namespace 상수 `HIT_FLASH_DURATION_SECONDS = 0.12f`, `HIT_FLASH_PEAK_INTENSITY = 3.f`.
  `Trigger_HitFlash`: remaining=duration, `m_HitFlash = {true, white, peak}`.
  `Update`: remaining>0이면 `remaining -= dt`, `fIntensity = peak * remaining/duration`, 0 이하면
  `m_HitFlash = {}`. `Render`: `Bind_DeferredMaterialInputs(*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)`.
- `Body_Valtan.h`: `BODY_VALTAN_DESC`에 `const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = nullptr;`
  (`iPrototypeLevelIndex` 아래), 멤버 `const DEFERRED_EMISSIVE_OVERRIDE* m_pEmissiveOverride = nullptr;`.
  `Body_Valtan.cpp` `Initialize`에서 복사, `Render`의 `Bind_DeferredMaterialInputs(..., Profile, m_pEmissiveOverride)`
  (Render_Shadow는 그대로).
- `Valtan.h`: public `void Trigger_HitFlash();`, private `DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;`,
  `f32_t m_fHitFlashRemainingSeconds = 0.f;`. `Valtan.cpp` `Ready_PartObjects`: `bodyDesc.pEmissiveOverride =
  &m_HitFlash; weaponDesc.pEmissiveOverride = &m_HitFlash;` (CPart_Equipment은 이미 지원).
  `CValtan::Update(f32_t)` 첫머리에서 CNpc와 같은 감쇠.
- `ClientReplication.cpp` `Apply_WorldSnapshot`: `Apply_DamageEvents(...)` 호출 바로 앞에
  ```
  for (const DAMAGE_EVENT& event : snapshot.DamageEvents) {
      if (!event.isOutgoing) continue;
      auto it = m_WorldEntities.find(event.iTargetNetEntityId);
      if (it == m_WorldEntities.end()) continue;
      if (MONSTER == it->second.eKind) { if (auto npc = it->second.pNpc.lock()) npc->Trigger_HitFlash(); }
      else if (BOSS == it->second.eKind) { if (auto v = it->second.pValtan.lock()) v->Trigger_HitFlash(); }
  }
  ```
  (실패 없음 — 표현 누락은 allSucceeded에 영향 주지 않는다.)

- 셰이더 `Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl`, `Shader_VtxMeshBinary.hlsl`:
  `uint g_FullSurfaceEmissiveMaskMode` 추가. mode 1은 카메라 위치를 `g_ViewMatrix`에서 역산해
  림 항만 emissive에 더한다(새 uniform 없음). `DEFERRED_EMISSIVE_OVERRIDE::usesSurfaceDetailMask`가
  바인더에서 mode를 결정. anim 셰이더의 BC5(ATI2, RG 2채널) 노멀 디코드 누락도 같이 수정 —
  잡몹/루가루 노멀맵이 전부 ATI2라 z가 −1로 뒤집혀 있었다.

**검증 (2026-08-17 완료)**: fxc fx_5_0 두 셰이더 컴파일 OK, Client Debug 빌드 OK, 사용자 육안으로
Character Select 몬스터/루가루/Valtan 림 플래시 확인("지금이 제일 나은듯"). NPC 무영향.
시도 순서: 전면 백화 → 노멀굴곡×스페큘러 마스크 → 스페큘러만 → 백화+림 → **림만(채택)**.

## G2. 데이터·publisher·bootstrap

**수정 파일**: `Data/Balance/MonsterProfiles.json`, `Tools/WorldPipeline/Publish-WorldGameplay.ps1`,
`Server/Public/SpawnGroupBootstrap.h`, `Server/Private/SpawnGroupBootstrap.cpp`

- JSON: 각 profile `deadDespawnMs` 뒤에 `"hitKnockbackDistance": <값>`.
- publisher `Get-MonsterProfiles`: exact property 목록에 `'hitKnockbackDistance'` 추가,
  `Assert-JsonNumber` + 범위 `0.0 <= v <= 10.0`. `Convert-SpawnGroupsDocument` PROFILE 행 끝에
  `(Format-InvariantFloat $profile.hitKnockbackDistance)`, header `LOSTARK_SPAWN_GROUP_BOOTSTRAP\t2`.
- `MONSTER_RUNTIME_PROFILE`에 `float fHitKnockbackDistance = 0.f;` (`iDeadDespawnMs` 아래).
  parser: `2u != version` 거부, PROFILE `14u != fields.size()`, `fields[13]` 파싱, `isfinite && >= 0`.
- CLAUDE.md 473행 "optional spawn bootstrap v1" → v2 (public 계약 문구만).

**검증**: `Publish-WorldGameplay.ps1 -Mode Validate/Publish` (Server pre-build가 실행), 생성된
`Server/Bin/DataFiles/World/*.spawngroupsbootstrap` header 2·PROFILE 14열 확인, Server contract-test의
기존 `spawnBootstrap.Load(CHARACTER_SELECT_ARENA)` 통과 + `Find_Profile("MONSTER_VALTAN_PADD_01")->fHitKnockbackDistance == 0.35f`
require 1건 추가.

## G3. Server 넉백

**수정 파일**: `Server/Public/ServerWorldEntity.h`, `Server/Private/GameRoom.cpp`,
`Server/Private/PlayerSkillSystem.cpp`, `Server/Private/ServerGameplayContractTests.cpp`

- `SERVER_WORLD_ENTITY` (`iDeadDespawnMs` 아래):
  `float fHitKnockbackDistance = 0.f;` (profile 복사),
  `float fKnockbackDirectionX = 0.f, fKnockbackDirectionZ = 0.f;`,
  `float fKnockbackRemainingSeconds = 0.f;`, `float fKnockbackSpeed = 0.f;` (= distance/duration).
- `GameRoom.cpp:3374` 근처 staged 복사: `staged.fHitKnockbackDistance = profile.fHitKnockbackDistance;`.
- `PlayerSkillSystem.cpp` `applyDamage` 람다, damage event push 뒤·사망 처리 앞:
  ```
  if (MONSTER == target.eKind && target.fHitKnockbackDistance > 0.f && 0u != damage) {
      dx = target.fPositionX - player.fPositionX; dz = ...; len = sqrt;
      if (len < 1e-4f) { dx = player.fSkillAimDirectionX; dz = player.fSkillAimDirectionZ; len = 1 }
      target.fKnockbackDirectionX = dx/len; ...Z;
      target.fKnockbackRemainingSeconds = MONSTER_KNOCKBACK_DURATION_SECONDS;
      target.fKnockbackSpeed = target.fHitKnockbackDistance / MONSTER_KNOCKBACK_DURATION_SECONDS;
  }
  ```
  상수는 `ServerWorldEntity.h`에 `inline constexpr float MONSTER_KNOCKBACK_DURATION_SECONDS = 0.15f;`
  (PlayerSkillSystem·GameRoom·contract test가 공유).
- `GameRoom.cpp Update_WorldEntities` MONSTER 분기 (`m_MonsterBrain.Update` 호출 앞):
  ```
  if (entity.fKnockbackRemainingSeconds > 0.f && DEAD != entity.eAction && 0u != entity.iCurrentHp) {
      step = min(dt, remaining); desired = pos + dir * speed * step;
      Clamp_StepToWalkable(m_ServerNavigation, pos.xz, desired.xz, reachable, wasClamped);
      pos = reachable (x,y,z); remaining -= step; if (wasClamped) remaining = 0;
      entity.fActionElapsedSeconds += dt;   // 패턴 타이머는 계속 흐르되 이동/판정만 정지
      continue;   // 이 tick brain 생략
  }
  ```
  주의: DEAD면 brain이 elapsed를 올려 despawn하므로 넉백 분기를 타지 않게 조건에 포함.
- contract test 추가 (기존 MonsterBrain 테스트 블록 뒤):
  1. `hitKnockbackDistance 0.35`인 monster에 34090 히트 → 5 tick 뒤 위치가 player 반대 방향으로
     0.35 ± 0.01 m 이동, `fKnockbackRemainingSeconds == 0`.
  2. 넉백 중 tick에 `eAction`이 CHASE로 바뀌지 않는다 (brain 생략).
  3. `hitKnockbackDistance 0`(Lugaru) → 위치 불변.
  4. 벽 방향 넉백 → `Clamp_StepToWalkable`로 walkable 안에서 멈춘다 (LV_LOBBY_CLASSSELECT_SL00 grid 경계 사용).

**검증**: Server 빌드, `Server.exe --contract-test` failures 0, NetworkProtocolHarness 불변(프로토콜 무변경),
Server+Client 실행 → Character Select 몬스터 소환 → 스킬 → 잡몹은 밀리고 Lugaru는 안 밀림, Bern/Valtan
진입 회귀 없음.

## G4. 원작 히트별 push (사용자 결정 2026-08-17: "애니메이션 이벤트에 넉백이 있으면 그때만")

G2/G3의 profile 거리·고정 0.15 s를 히트별 원작 값으로 교체한다. 위 계약 표의 G4 행이 정본이다.

- `Tools/CharacterAnimationIntake/build_hitshapes.py`: HIT 행 `push`/`pushr` 읽어 `pushMs`(≥0),
  `pushRange`(m, push>0일 때만, 부호 유지). 4 class `hitshapes.json` 재생성(GunSlinger/Slayer는 원래
  문서가 없어 생성하지 않음).
- `Publish-GameplayBalance.ps1`: hit exact property에 `pushMs`,`pushRange`, 범위(0..10000 ms, ±50 m,
  pushMs 0이면 range 0), packed `…:maxt:pushMs:pushRange`, header `LOSTARK_GAMEPLAY_BOOTSTRAP 7`.
- `CGameplayCatalog::Parse_SkillHits`: 12필드 strict, version 7. contract test의 인라인 fixture 6→7,
  obsolete 버전 테스트 5→6.
- `MonsterProfiles.json`/publisher/`MONSTER_RUNTIME_PROFILE`/`SERVER_WORLD_ENTITY`:
  `hitKnockbackDistance` → `hitKnockbackScale`. `MONSTER_KNOCKBACK_DURATION_SECONDS` 삭제.
- `applyDamage(target, rawDamage, const PLAYER_SKILL_HIT* pHit)`: `pHit && pHit->iPushMs > 0`일 때만
  `pushRange × scale`을 `iPushMs` 동안 arm. legacy 원형 경로는 `nullptr`.
- contract test: 34540(단창 Q, 130 ms/1.0 m) push 로드·arm·4 tick 1.0 m 슬라이드, 34120(push 0) 불변,
  scale 0 면역, 벽 clamp.

## 순서

G1(Client만, 즉시 눈으로 확인 가능) → G2 → G3 → G4. G1은 독립 커밋, G2~G4는 한 커밋.

## 경계·미포함

- 피격 클립(모델의 hit/damaged clip 재생)은 미포함. Shared action 없이 Client가 damage event로 로컬 재생하는
  방식은 다음 슬라이스에서 별도 결정.
- Valtan/보스 넉백, 플레이어 피격 넉백, 몬스터→몬스터 밀림, 원작 스킬별 push 플래그 조사 미포함.
- PhysX는 어느 쪽에도 쓰지 않는다. Client PhysX 표현 오프셋은 사용자 결정으로 배제.
- `.md/GB`·팀 문서 갱신은 CLAUDE.md의 spawn bootstrap 버전 문구 한 줄만.
