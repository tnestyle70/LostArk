# 2026-08-13 플레이어 이동 스무딩 RESULT

## 목표

서버 권위 이동에서 남던 두 가지 부자연스러움 제거.

1. 30Hz 스냅샷의 최신 좌표 하나만 지수 lerp로 쫓아가며 생기던 속도 리플(미세 끊김).
2. A* 셀 중심 경로의 45° 양자화 지그재그와 방향 전환 시 각진 꺾임.

## 구현

### 1. 클라이언트 스냅샷 보간 버퍼 (Client)

- `CCharacter::Apply_NetworkState`에 `iServerTick` 인자를 추가하고, 최신 좌표 대신
  서버 tick이 붙은 샘플 최대 8개를 버퍼에 적재한다. 호출부는
  `ClientReplication.cpp`의 player 스냅샷 반영 한 곳.
- `Update_NetworkTransform`은 재생 커서를 30Hz로 전진시키며 최신 tick보다
  `INTERPOLATION_DELAY_TICKS`(2 tick, 약 66ms) 과거 시점을 두 샘플 사이 선형 보간으로
  렌더한다. 드리프트는 `PLAYBACK_DRIFT_GAIN`으로 서서히 따라잡고 6 tick 이상 벌어지면
  스냅, 버퍼 고갈 시 최신 샘플 홀드(외삽 없음).
- yaw도 같은 버퍼에서 보간한 값을 목표로 삼고 기존 720°/s 고정 회전을 그 위에 유지.
- 연속 샘플 거리 제곱이 `TELEPORT_DISTANCE_SQ`(100)를 넘으면 텔레포트로 판정해 버퍼를
  비우고 즉시 스냅(triggerBox movePlayer 대응). 첫 스냅샷 yaw 스냅과
  `m_BoneChains.Reset()` 타이밍은 기존 유지.
- NPC/Valtan의 `Apply_NetworkState` 경로는 건드리지 않았다.

### 2. 서버 경로 스무딩 (Server)

- `CServerNavigation::Sample_Position` — 해당 좌표의 셀이 walkable일 때만 셀 높이를 얹은
  정확한 X/Z 좌표 반환(투영 없음).
- `CServerNavigation::Has_LineOfSight` — 두 점 사이를 셀 크기 1/4 간격으로 샘플링해 전
  구간 walkable 검사. 높이가 선형 기대치에서 1유닛 초과 이탈하면 차단(계단·절벽 지름길
  거부).
- `GameRoom.cpp`의 `Smooth_MovePath` — 이동 명령 처리에서 `Find_Path` 결과에 스트링
  풀링을 적용. 시야가 닿는 가장 먼 웨이포인트만 남기고, 마지막 웨이포인트는 클릭 셀이
  walkable이면 셀 중심 대신 정확한 클릭 좌표로 교체. 플레이어가 grid 밖이면 원경로 폴백.
- `Find_Path` 자체는 몬스터/발탄 브레인이 공유하므로 변경하지 않았다.

### 3. 서버 곡선 조향 (Server)

- 이동 중 yaw를 `PLAYER_TURN_DEGREES_PER_SECOND`(540°/s)로 제한하고 회전 중에는 현재
  heading 방향으로 이동해 방향 전환을 짧은 원호로 만든다.
- 목표 1.5유닛 이내(`DIRECT_BEARING_DISTANCE`)는 기존 직선 조준으로 전환해 목표 주위
  선회를 차단. heading 이동 지점이 walkable이 아니면 그 tick은 직선 이동 폴백.

### 4. 우클릭 홀드 튜닝 (Client)

- `MOVE_GOAL_RESEND_INTERVAL` 100ms → 50ms, `MOVE_GOAL_DEADZONE_RADIUS` 1.0 → 0.5.

## 검증

- Client x64 Debug 재빌드 성공(Engine/Public 미변경, UpdateLib 불필요).
- Server x64 Debug 재빌드 성공, `Server.exe --contract-test` failures 0
  (기존 `Find_Path` 계약 테스트 포함).
- 로컬 실행 검증: Server `--bind-address 127.0.0.1`, Client process-local
  `LOSTARK_SERVER_HOST=127.0.0.1`(정본 `Client.vcxproj.user`·`TeamLanEndpoint.json`
  미변경). 우클릭 이동 직선 주행, 커서 선회, 방향 급전환 아크를 수동 확인.
- Shared 프로토콜, 네비게이션 데이터 포맷, publisher 변경 없음.

## 남은 경계

- NPC/몬스터/발탄 presentation은 스냅샷 보간 미적용. 필요 시 같은 방식 확장 가능.
- 몬스터/발탄 서버 이동은 스무딩·조향 미적용(공용 `Find_Path` 원형 유지).
- 정본 회귀(`Invoke-BuildAndRegression`)의 Artist 31470 WARP 하네스는 이 PC에
  `Client/Bin/Resources/Effect/Artist/Textures/fx_e_atypical_012.dds` 등 신규 런타임
  리소스가 없어 실패한다. 본 변경과 무관한 기존 실패.
- 튜닝 상수(보간 지연 2 tick, 회전 540°/s, 데드존 0.5, 재전송 50ms)는 체감 기준이며 추가
  조정 여지 있음.
