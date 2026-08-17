# 2026-08-17 몬스터 피격 플래시·Server 넉백 RESULT

작성자: JS · branch `feature/monster-hit-flash-knockback` (main `840c4bd` 기준)
PLAN: `2026-08-17_MONSTER_HIT_FLASH_KNOCKBACK_PLAN.md`

## 1. 완료

| G | 내용 | 상태 |
|---|---|---|
| G1 | 피격 플래시 — Client 표현 전용. `DAMAGE_EVENT(isOutgoing)` 대상이 MONSTER면 `CNpc`, BOSS면 `CValtan`(body+weapon)의 `Trigger_HitFlash()`. 셰이더 `g_FullSurfaceEmissiveMaskMode` 1 = **실루엣 림만** `pow(1−N·V,3) × 4.0`, 0.12 s 선형 감쇠. `Shader_VtxAnimMeshBinary`의 BC5(ATI2) 노멀 디코드 누락 수정 포함 | 완료 (사용자 육안 승인 "지금이 제일 나은듯") |
| G2 | `MonsterProfiles.json` profile 필드 → `Publish-WorldGameplay.ps1` → `spawngroupsbootstrap` v1→**v2**(PROFILE 14열) → `CSpawnGroupBootstrap` | 완료 |
| G3 | Server 넉백 — `SERVER_WORLD_ENTITY` 넉백 상태, `CPlayerSkillSystem::applyDamage`에서 arm, `CMonsterBrain::Advance_Knockback`(static, nav walkable clamp, brain 생략), `CGameRoom::Update_WorldEntities` 연결 | 완료 |
| G4 | 넉백 유무·거리·지속을 **원작 히트별 `push`/`pushr`**로 교체 — `build_hitshapes.py` → `hitshapes.json` `pushMs/pushRange` → `Publish-GameplayBalance.ps1` SKILLHIT 12필드, `Gameplay.bootstrap` v6→**v7** → `PLAYER_SKILL_HIT::iPushMs/fPushRange`. profile 필드는 `hitKnockbackScale`(잡몹 1.0, Lugaru 0.0) 배율로 | 완료 (사용자 육안: 잡몹 밀림·Lugaru 불변 확인) |

프로토콜(v13) 무변경. Engine 무변경.

## 2. 검증

자동:
- fxc fx_5_0 두 셰이더 컴파일 OK
- Client Debug 빌드 OK (G1 4회 반복)
- `Publish-WorldGameplay.ps1 -Mode Validate/Publish` OK, `Publish-GameplayBalance.ps1` Publish OK(Server pre-build)
- Server Debug 빌드 OK, `Server.exe --contract-test` **failures 0** — 추가 require: profile scale read-back 1건,
  push 로드/불변/arm/슬라이드/면역/벽 clamp 6건, bootstrap 버전 fixture 6→7·5→6 갱신
- NetworkProtocolHarness failures 0

수동(사용자, Character Select, Server 127.0.0.1):
- 몬스터/루가루/Valtan 림 플래시 확인. 전면 백화 → 노멀굴곡×스페큘러 → 스페큘러만 → 백화+림 → 림만 순으로
  시도, 마지막 채택.
- 34120(push 0)은 안 밀리고 단창 Q(34540, 130 ms/1.0 m) 등 push>0 히트만 밀림, Lugaru는 어느 스킬에도 불변.
- 테스트 중 잡몹 maxHp를 20배로 올렸다가 커밋 전 원복(2600/3200/4100).

## 3. 결정과 경계

- 몬스터 이동이 어색해 보인 것은 넉백 로직이 아니라 몬스터 AI(단순 추적, 피격 클립 없음, 위치 스냅)
  때문으로 판단. **몬스터/보스 AI·보간·피격 클립은 다른 담당자 몫**이라 CNpc 스냅샷 보간 helper는
  만들지 않고 중단했다(사용자 결정). Character에는 2틱 지연 보간이 있고 CNpc/CValtan은 스냅한다는
  사실만 기록.
- 원작 `push=60 pushr=450`(창술사 34610 후반 5타, 4.5 m) 같은 값은 그대로 들어간다. 과하면
  `hitKnockbackScale`로 조정한다.
- 음수 `pushr`(끌어당김, 예 34072 `pushr=-50`)은 부호를 보존해 공격자 쪽으로 이동한다. 자동 테스트는
  없음(창술사 hitshapes에 해당 skillId 미포함).
- 셰이프 없는 스킬(maximumRange 원형 단일 판정)은 넉백 없음. Valtan/보스·플레이어 넉백 미포함.
- BC5 노멀 디코드 수정으로 잡몹/루가루의 **평소 조명**도 바뀐다(이전엔 노멀 z가 −1로 뒤집힘). Valtan body는
  TGA(RGB)라 무관.

## 4. 커밋

1. `feat(client): flash the silhouette rim of a monster or boss on a player hit` — G1 (Client + 셰이더 + PLAN)
2. `feat(server): knock monsters back by the authored per-hit push` — G2~G4 (Data/Tools/Server/CLAUDE.md + RESULT)

## 5. 실행 절차 (재현)

```
Server: Server\Bin\Debug\Server.exe --bind-address 127.0.0.1  (cwd Server\Bin\Debug)
Client: LOSTARK_SERVER_HOST=127.0.0.1, Client\Bin\Debug\Client.exe (cwd Client\Default)
Lobby → Character Select → F1 Debug Monster/Valtan Spawn → 스킬
```
