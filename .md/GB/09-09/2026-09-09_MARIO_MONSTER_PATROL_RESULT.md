# 마리오 몬스터 순찰·근접 공격 적용 결과

## 구현

사용자 확정대로 플레이어에게 한 대 맞으면 몬스터가 사망한다. 여섯 Mario 프로필만 HP1/방어0으로
낮췄으며 player HP나 카드미로 병사의 프로필은 변경하지 않았다. 양수 피해를 주는 승인된 hit가 필요하다.

기존 원본 배치 40개에 같은 stage·층의 published lane을 연결했다. 원본 배치 위치를 보존한
평행 직선을 따라 초당 1.2m로 왕복하고 끝이나 네비·충돌 차단에서는 멈춰 360deg/s로 선회한다.
다른 방향으로 접선 미끄러짐을 적용하지 않는다.

같은 stage·높이의 플레이어가 2m 안에 오면 순찰을 멈추고 바라본 뒤 공격한다.
다른 층(높이차 0.8m 초과), trigger 이동 중, 사망/비전투 상태는 제외한다.
예고600ms → 판정100ms → 회복1300ms이며 타격 시 대상 거리·전방·높이를 재검사한다.
공격당 기존 ServerCombatHitRuntime을 한 번 호출하며 공격력100에서 기존 플레이어 방어를 적용한다.
원본 전투 AI 복원이 아니라 프로젝트 튜닝 행동이다.

Client는 기존 snapshot action과 catalog의 대기/달리기/공격/사망 원본 클립을 재생한다.
Mario catalog의 modelYawDegrees=-90으로 모델 +X 앞축과 Server yaw +Z를 정렬했다.
모델 binary/texture, 배치와 컷신/이동 trigger, Shared protocol은 바꾸지 않았다.

## 실행한 검증

- WorldGameplay Publish 성공. 현재 MonsterProfiles를 Server bootstrap에 반영했다.
- Server Debug Build 및 테스트 추가 후 최종 Build 성공.
  `out/MarioOriginalReplacement/server-patrol-build.log`, `server-patrol-final-build.log`.
- 기존 `Server.exe --debug-teleport-contract-test` failures 0.
  `out/MarioOriginalReplacement/server-patrol-contract.log`.
  실제 원본 40개 각각 HP1/축 초기화, 이동·반전, 150틱 직선 유지, 다른 층 무시,
  한 공격당 피해 한 번, 플레이어 한 대에 사망을 검사했다. 각 배치 앞에 synthetic blocking body를
  둔 검사도 추가하여 이동 commit 없이 방향을 바꾸고 접선 미끄러짐이 없는 것을 확인했다.
  최종 재빌드/재검사 failures 0. 기존 이동/점프/퇴장 검사도 포함한다.
- 관련 git diff --check 성공. 새 project/filter 등록이나 protocol 변경은 없다.
- MonsterProfiles/MonsterCatalog JSON, Server/Client vcxproj와 filters XML parse 성공.
- 독립 read-only 검토에서 stage/층/타격 시점 필터, 한 번의 피해 호출, 순찰 고정축과
  원본 클립 소비자 연결에 구체적 코드 결함은 발견되지 않았다.
- Client/UI 실행·화면 캡처는 하지 않았다. 사용자 화면 검증은 아직 남아 있다.

## 실행 준비 경계

Server는 새 실행 파일과 publish된 bootstrap으로 재시작해야 한다. 팀 공유 서버를 이용한다면
서버 PC에도 같은 코드/데이터가 있어야 한다. Resources 추가 전달은 없다.
앞선 로켓 수정의 Client.pdb 잠금은 후속 확인에서 해제되었다. 정식 Client Debug Build가
성공해 Client.exe를 다시 생성했다. 새 실행 파일에 4000ms/3m/s/face follows flight 문자열을
확인했다. 로그는 `out/MarioOriginalReplacement/client-bomb-facing-final-build.log`다.
기존 코드페이지/DirectXTK PDB 경고는 남아 있으며 오류는 없다. Client와 Server는 실행하지 않았다.
실행 준비는 완료했고 실제 화면 검증은 사용자에게 남아 있다.

확인 순서: 새 Server → Client → Lobby KoukuSaydon → F1 1~4마리오 → 컷신 종료/F1 닫기.
멀리서는 왕복, 가까이서는 선회 후 공격, 플레이어 공격 한 번에 몬스터 사망을 확인한다.
일반 플레이어 즉사는 추가하지 않았다. 기존 dirty worktree를 보존하며 stage/commit/push하지 않았다.
