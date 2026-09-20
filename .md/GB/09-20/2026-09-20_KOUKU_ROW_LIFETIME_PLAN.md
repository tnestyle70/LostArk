# 쿠크 Stage와 독립 행 수명 계획

Stage 합계는 다음 Stage/Pattern 진행 시점이다. `timelineDurationMs`는 World/Logic/Summon 등 행이 남아 있을 수 있는 최종 수명이며 Stage를 연장하지 않는다. publisher와 catalog는 별도 담당자가 optional 필드와 `PATTERNTIMELINE` bootstrap 확장을 연결한다.

GameRoom은 자연 완료된 occurrence의 원본 pattern identity, 태어난 보스의 Spawn/Stage origin과 본체 정보, Ledger, World cue map을 별도 tail로 보존한다. 다음 Pattern은 기존 전이 시각에 시작하고 tail은 자기 absolute clock으로 늦은 World cue, Logic 판정, Summon 생성과 종료를 처리한다. 자연 run 완료 후에도 pinned catalog와 navigation support를 tail이 소진될 때까지 유지한다. 명시 Stop·restart·수동 새 run·boss death·gate clear는 tail과 종속 summon, 판정 및 support를 취소한다.

실제 consumer 검증은 Stage 완료 후에도 원래 occurrence의 late World/Logic/Summon이 한 번 발생하고, 다음 Pattern identity를 변경하지 않으며, Stop과 gate clear가 정리되는 시나리오로 한다. 기존 Brain과 GameRoom 테스트 경로를 사용하며 Client/UI는 실행하지 않는다. 새 C++ 파일 없이 기존 기능 경계에 추가하고 변경 TU 컴파일, publisher parse/검증, focused Server contract를 수행한다.

`KOUKU_PATTERN_TAIL`은 Server 메모리의 `shared_ptr<SERVER_WORLD_ENTITY>`로 원본 occurrence 상태를 소유한다. 실제 boss는 `weak_ptr`로 남은 counter/shield 창만 조회한다. Shared packet이나 snapshot wire는 바꾸지 않는다. 공격 HP는 기존 실제 boss에 한 번 적용하며 counter 결과는 이전 sequence의 Ledger가 소비한다. 본체 이동/충돌체 갱신은 실제 actor reference에만 허용하고 이전 행은 다음 actor를 이동시키지 않는다.

자동 RaidFlow의 다음 Entry는 immutable source/gameplay pin과 같은 epoch를 유지하고 commonStartTick만 새로 잡는다. 이전 tail/World/support는 옮겨 보존한다. 명시 BOSS_CURRENT follow는 현재 실제 본체 위치/방향/HP를 읽고, source identity/clock/Spawn 및 Stage origin은 이전 occurrence를 유지한다. tail은 새 본체의 움직임이나 충돌체를 쓰지 않는다.
