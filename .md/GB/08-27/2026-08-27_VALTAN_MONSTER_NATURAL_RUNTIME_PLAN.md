# Valtan Monster Natural Runtime Plan

## 목표

Lobby에서 Valtan Arena에 진입해 첫 일반 몬스터를 만났을 때 모델 로딩 hitch, snapshot 위치 튐, 공격 클립 재시작 누락, 매 tick 타깃 전환과 즉시 회전, 다수 몬스터 겹침이 함께 보이지 않도록 기존 제품 경로를 수직으로 보강한다.

## 정본과 책임

- `Data/Actors/MonsterCatalog.json`: 모델, semantic clip, 결정적 공격 clip pool과 재생률, 피격 clip/복귀 시간.
- `Data/Balance/MonsterProfiles.json`: Server 권위 인식 유지 거리, 회전, 가속, 감속, 도착 감속 반경.
- `Publish-WorldGameplay.ps1`: profile 검증과 spawn bootstrap v4 publish.
- `CMonsterBrain`: 타깃 유지/공격 중 고정, smoothed path, 제한 회전, 가감속, 결정적 접근 지점.
- `CServerCollisionSystem`: 기존 원형 body sweep/slide를 몬스터 이동과 knockback에도 적용.
- `CNpc`: root-motion 억제와 network transform 보간 정책을 분리.
- `CClientReplication`: 공격 occurrence edge를 기준으로 결정적 clip을 한 번 시작하고 ACTIVE/RECOVERY에서는 이어서 재생하며, 비공격 중 damage event는 짧은 피격 clip 뒤 기존 locomotion으로 복귀.
- `CLoader`: Valtan 진입 중 지원 몬스터 prototype을 미리 준비하고 누락 archetype만 격리.

## 구현 순서

1. Client transform/animation 정책 분리와 2-tick 보간 활성화.
2. 순수 monster action projection 계약 및 harness 추가.
3. 실제 model clip 목록/길이에 맞춘 presentation-only 공격 pool·재생률과 피격 반응 반영.
4. Server profile/bootstrap v4와 live entity 필드 연결.
5. 타깃 hysteresis/attack lock, path smoothing, turn/acceleration/arrival slowdown 적용.
6. 기존 Server collision body를 monster move/knockback에 적용하고 tick 안에서 body 위치 갱신.
7. Valtan level load prototype warmup.
8. publisher, focused harness, Server contract, Debug/Release Client build와 문서 검증.

## 실패 경계

- 잘못된 profile 값이나 bootstrap version/열 수는 stage 전에 거부한다.
- 특정 model/clip이 없으면 그 archetype 또는 action 표현만 idle로 격리하며 replication 전체를 중단하지 않는다.
- Server는 clip/model path를 알지 않으며 Client는 target/path/damage 정답을 만들지 않는다.
- Client 화면의 최종 자연스러움은 사용자가 `Lobby -> Valtan`에서 직접 판정한다.

## 검증

- `Publish-WorldGameplay.ps1 -Mode Validate` 및 publish.
- monster action occurrence harness: IDLE/CHASE edge, WINDUP restart, ACTIVE/RECOVERY continuation, 다음 WINDUP restart, late join, DEAD hold.
- Server contract: 새 profile 필드, target retention/attack lock, turn/acceleration, collision separation.
- Server/Client x64 Debug 및 Release build, `Server.exe --contract-test`, JSON parse, `git diff --check`.
