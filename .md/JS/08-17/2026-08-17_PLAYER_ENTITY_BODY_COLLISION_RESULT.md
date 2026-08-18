# 2026-08-17 플레이어 ↔ 몬스터/보스 몸체 이동 차단 RESULT

작성자: JS · branch `feature/player-entity-body-collision` (넉백 브랜치 `985ecce` 위 상위집합)
PLAN: `2026-08-17_PLAYER_ENTITY_BODY_COLLISION_PLAN.md`

## 1. 완료

- `CServerCollisionSystem`: `SERVER_BLOCKING_BODY{x,z,radius}` + `Set_BlockingBodies`. `Resolve_PlayerMove`가
  정적 collisionBox 스윕 뒤에 몸체 원(플레이어 0.45 + 몸체 반경) XZ 스윕을 수행한다. 이미 겹친 시작점은
  중심 쪽 이동만 차단.
- **접선 슬라이드**: 몸체가 가장 이른 접촉이면 접촉점까지 이동 후 남은 이동 길이를 그대로 접선 방향으로
  다른 벽/몸체에 대해 한 번 더 스윕한다(정면은 결정적으로 한쪽으로 꺾음). 그 결과 `wasBlocked=false`라
  우클릭 goal이 유지되고 몸체를 감아 돌아 도착한다. 벽(collisionBox)은 기존 정지 동작 유지.
- `CGameRoom::Refresh_PlayerBlockingBodies()`: 매 tick `Update_Players` 직전, 살아 있는 MONSTER(profile
  `collisionRadius`)/BOSS(boss profile 반경 3.0) 몸체 전달. Esther·NPC·DEAD 제외.
- 우클릭 이동(`Update_Players`)과 스킬 루트모션(`CPlayerSkillSystem::Update`)이 같은 `Resolve_PlayerMove`를
  타므로 둘 다 적용. Client·프로토콜·데이터 무변경. 반경은 새로 정하지 않고 기존 profile/Shared 값 재사용.

## 2. 검증

- Server Debug 빌드 OK, `Server.exe --contract-test` **failures 0** — 추가 7건: 정면 접근이 옆으로 전속력
  deflect / 측면 통과 / 겹침에서 탈출 허용 / 겹침에서 중심 접근이 옆걸음으로 / 비스듬한 접근이 몸체를 따라
  슬라이드 / 겹친 채 접선 슬라이드 / 빈 목록에서 해제.
- 수동(사용자, Character Select 127.0.0.1): 몬스터/루가루/Valtan에 겹치지 않고, 루가루 앞에서 뒤를
  클릭하면 감아 돌아 도착. 첫 버전(투영만)은 정면 근처에서 기어가듯 느려 "전속력 접선"으로 교체.

## 3. 결정과 경계

- 원작 처리 규칙은 데이터에서 근거를 못 찾았다(잡몹 통과 여부, 스킬별 통과 플래그 `sa/move` 미조사).
  프로젝트 규칙: 살아 있는 몬스터·보스는 전부 차단 + 슬라이드.
- 넉백 감속(ease-out)은 넣지 않았다. 원본 `EFTable_SkillEffect`의 `PushType/PushMin·MaxRange/PushMin·MaxTime`은
  거리·시간이 같은 비율로 커지는 등속 기술이며 곡선 컬럼이 없다. 부수 발견: 원작은 거리를 min~max에서
  롤하는 것으로 보이고(6.4만 행 Min≠Max), `PushType` 0/1/2 의미 미상 — 우리 추출은 Min만 사용.
- PhysX 대체는 검토 후 배제: 캐릭터 이동은 kinematic sweep+slide가 표준이며 Server에 PhysX가 없다.
- 몬스터↔몬스터 분리, 몬스터 AI/보간/피격 클립은 몬스터 담당 레인.

## 4. 커밋

- `feat(server): block and slide player movement around living monster and boss bodies`
