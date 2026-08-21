# 2026-08-20 발탄 보스 전투·패턴 그래프 G05-1 결과

branch: `codex/valtan-boss-combat-runtime`

## 완료된 범위

- `PlayerSkillSystem`의 보스 피격을 `CBossCombatRuntime`으로 위임했다.
- 갑옷 2부위의 stable ID, mask, 내구도, 피해 감소율과 GROGGY 전용 부위 피해를
  Data -> publisher -> catalog -> Server runtime으로 연결했다.
- pattern stage에 결과 분기, typed action, target/aim policy, FORWARD motion을 추가했다.
- 벽 접촉 -> GROGGY -> 부위 파괴 -> RECOVERY와 TIMEOUT 종료 경로를 Server 권위로 닫았다.
- 무력화 게이지, 보호막, 무적, counterable 상태와 Magic/Parry/Center/Triple Counter의
  성공·실패 분기를 제품 encounter graph에 연결했다.
- protocol v26 boss persistent state와 `PART_BROKEN` edge를 Client에 복제하고,
  `CValtan` 갑옷 가시성 및 기존 HUD stagger 슬롯에 투영했다.
- Client encounter reference를 v4의 pattern policy와 stage motion/action/branch까지
  exact 검증하도록 갱신해 Camera/PatternEffect 소비자의 v3 고정 회귀를 제거했다.
- 플레이어별 projectile를 제거하고 `CGameRoom`이 하나만 소유하는
  `CCombatObjectRuntime`으로 플레이어 투사체와 보스 전투 오브젝트를 통합했다.
- `VALTAN_HIGH_JUMP`의 잠긴 대상 추적 하늘 도끼와 `VALTAN_RED_BLADE_WAVE`의
  이동 검기를 Server fixed tick 피해 권위로 구현했다. 기존 보스 중심 inline hit는
  `NONE`으로 바꿔 같은 공격이 두 번 피해를 주지 않게 했다.
- 하늘 도끼는 잠긴 대상만 첫 pulse 전까지 추적한 뒤 위치를 고정하고, 이동 검기는
  swept-circle 접촉으로 한 tick 사이에 대상을 지나쳐도 대상별 한 번만 적중한다.
- protocol v27 combat-object reliable spawn/despawn과 full live transform snapshot,
  late join/reset/source 취소를 연결했다. Client는 collider나 피해 판정 없이 기존
  `CEffectPresentationService`의 world-root handle로만 표현한다.
- `BossCatalog.json` v3의 stable visual join과 신규
  `effect.valtan.sky-axe.active`를 제품 이펙트 정본에 추가했다. 이동 검기의 기존
  boss-root active cue는 제거해 움직이는 오브젝트 표현과 중복되지 않게 했다.

## 2026-08-21 재감사에서 추가로 닫은 계약

기존 완료 보고를 PLAN과 코드에 다시 대조한 결과, G05-1 안에서 자동 계약 세 개와 로컬 실행
정본 한 곳이 충분히 닫히지 않은 것을 확인해 다음과 같이 보완했다.

- 같은 tick에 due인 player projectile를 하나의 transaction으로 stage하고, capacity 또는
  두 번째 object 검증 실패 시 live set, lifecycle과 projectile mask가 모두 불변임을 보장했다.
  due mask는 전체 commit 성공 뒤에만 일괄 반영한다.
- live combat object에 owner pattern sequence/pattern/stage action과 previous/current pose를
  보존한다. boss object는 non-zero occurrence sequence를 요구하고 player object에는 boss
  occurrence metadata가 들어가지 않는다.
- HIGH_JUMP가 잠근 player가 TAKEOFF 뒤 사라지거나 유효 대상에서 제외돼도 다른 player로
  바꾸거나 object 생성을 생략하지 않는다. 마지막 유효 Server 위치를 보존해 AIRBORNE에서
  도끼를 생성하고 첫 pulse까지 그 위치를 유지한다.
- Client/Server 공용 debugger와 Server listener 기본값을 `127.0.0.1:7777`로 통일했다.
  endpoint 동기화 도구는 `LOSTARK_SERVER_HOST`만 정규화하고 기존
  `LOSTARK_RESOURCE_ROOT`를 보존한다.

## 자동 검증

- Gameplay Validate/Publish: PASS — 6 profiles, 230 skill rows, 108 damage profiles,
  1 boss, 2 parts, 2 boss combat objects, 33 patterns, 129 stages,
  67 audition occurrences. Gameplay bootstrap은 v13, 982행이다.
- 기존 static hit 48개/71 pulse와 combat-object hit 2개/2 pulse를 합쳐
  전체 50개/73 pulse 계약을 보존했다. Server bootstrap의 asset-like path는 0개다.
- WorldGameplay 및 ValtanWorldDestruction Validate: PASS.
- Effect Validate/Publish: PASS — runtime effect 193개, 발탄 제품 effect 100개
  (pattern cue 98개 + combat-object visual 2개), missing/ownerless 0개.
- NetworkProtocolHarness Debug/Release: `failures : 0`.
- 보완 후 Server Debug build 및 contract: 673 PASS, `failures : 0`.
- 보완 후 Server Release build 및 contract: 632 PASS, `failures : 0`.
- same-tick capacity rollback, 두 번째 object 검증 rollback, boss sequence 0 거부,
  owner metadata, previous/current pose, blade owner 지속, HIGH_JUMP 대상 소실 fallback의
  신규 focused 계약 7개가 Debug/Release에서 모두 PASS했다.
- Engine, Shared, 전체 harness project, Server, Client Debug/Release 컴파일·링크: PASS.
- `--valtan-boss-combat-presentation-fast` Debug/Release: 8/8 PASS.
- `--valtan-combat-object-presentation-fast` Debug/Release: 8/8 PASS.
- `--valtan-camera-fast` Debug/Release: 24/24 PASS.
- `--valtan-pattern-effects-fast` Debug/Release: 11/11 PASS. Git 제외
  `Client.vcxproj.user`의 실제 `LOSTARK_RESOURCE_ROOT`를 동일하게 사용했다.
- 관련 경로 `git diff --check`: PASS(기존 LF/CRLF 경고만 존재).

## 로컬 실행 준비

- Git LFS의 Engine 정적 라이브러리를 실제 바이너리로 복원했다.
- worktree의 `Client/Bin/Resources`에는 원본 작업 폴더의
  `Fonts, Character, Deploy, Effect, Map, UI` 여섯 물리 리소스 폴더를 junction으로 연결했다.
- junction의 물리 대상이 runtime root containment 안에 들어오도록 Git 제외
  `Client.vcxproj.user`에 실제 `LOSTARK_RESOURCE_ROOT`를 함께 설정했다.
- 공용 Client endpoint, Server debugger bind와 listener 기본값은 모두
  `127.0.0.1:7777`이다. endpoint 동기화 재실행 뒤에도 실제 resource root가 보존됨을
  검증했다.
- Client 또는 UI는 에이전트가 실행하지 않았다. 화면 fidelity는 사용자가 직접 판정해야 한다.

## 사용자 화면 검증 경계

- x64 Debug의 `Server + Client` profile로 시작하고 Lobby에서 `Valtan`을 선택한다.
- 1~67 audition에서 하늘 도끼는 #3, #8, #9, #11, #12, #40×2, #43×2,
  이동 검기는 #27, #34, #50, #57, #61, #65에서 확인한다.
- 하늘 도끼가 선택된 player를 impact 전까지 따라가고 위 15 m에서 1.2초 동안 내려온 뒤
  중복 생성되지 않는지, 검기가 발탄 앞 3 m에서 22 m를 0.9초 동안 이동하며
  boss-root 잔상 없이 한 번만 적중하는지는 사용자가 직접 육안 판정한다.
- 자동 검증은 실행 구조와 데이터 정합성을 보장하지만 원본 영상과의 visual fidelity 승인은
  대신하지 않는다.

## 아직 완료하지 않은 범위

- G05 후속: 비석 차폐/제품 파괴, 침묵·감금·잡기와 감금 몬스터,
  portal/유령 돌진 combat-object 데이터.
- G06 이후: 일반 3회/중요 1회 cadence, 정확한 phase commit과 phase pool,
  3페이즈 유령 발탄·분신, 추가 HUD/컷씬/보스 낙사.

따라서 이 문서에서 완료라고 부르는 범위는 G01~G04와 G05-1의 구현·자동 gate까지다.
발탄 전체 패턴 시스템 또는 G05/G06 전체 완료를 뜻하지 않으며, 실제 Client visual fidelity도
사용자 육안 승인 전에는 완료로 기록하지 않는다.

## 버전 관리 인계 상태

- 이 변경은 현재 Codex worktree에만 있으며 별도 commit/stage를 하지 않았다.
- worktree에는 다른 작업의 대규모 tracked/untracked 변경이 함께 있어 자동 stage나 일괄
  commit을 하지 않았다.
- 특히 신규 combat-object/boss-combat source와 Data/Effect/PLAN/RESULT 파일 중 일부는
  untracked 상태다. 원본 checkout에서 확인하려면 이 worktree 변경을 명시적으로 검토해
  별도 commit/merge해야 한다.

다음 요청에서 이 문서와 PLAN의 `G05-6`부터 재개한다.
