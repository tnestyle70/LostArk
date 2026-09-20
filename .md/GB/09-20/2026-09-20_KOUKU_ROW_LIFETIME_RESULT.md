# 쿠크 Stage와 독립 행 수명 결과

Stage 합계가 다음 동작으로 진행하는 시점이며, World/Logic/Summon 행의 수명 때문에 마지막 Stage를 연장하지 않는다. `timelineDurationMs`와 `PATTERNTIMELINE` publisher/catalog 연결은 통합 담당 변경을 소비한다.

## 구현된 소비자

- Brain은 Stage 합계와 명시 timeline 수명 중 큰 값을 행 admission 상한으로 사용하고, 마지막 Stage에서 Logic deadline 때문에 대기하던 처리를 제거했다.
- 자연 완료된 occurrence는 GameRoom tail이 source pattern/sequence/start tick, pinned catalog, Spawn/Stage origin, Logic Ledger와 World cue map을 소유한다. 늦은 Logic/World/Summon은 기존 consumer로 처리한다. 다음 본체 Pattern의 identity/움직임은 변경하지 않는다.
- `BOSS_CURRENT`는 살아 있는 실제 본체의 현재 위치/방향을 읽는다. `BOSS_SPAWN`과 source 원점은 이전 occurrence 그대로다. Counter/shield는 실제 damage consumer에서 retained owner를 조회하고 결과는 이전 Ledger에 한 번 전달한다.
- 늦은 Summon의 dependent admission이 현재 본체 Pattern만 확인해 거부하던 결함을 수정했다. 태어난 owner sequence의 immutable definition을 조회하며, 실제 primary가 살아 있는지와 pin/종속 조건은 계속 검증한다. 소환물은 자신의 수명 동안 다음 primary sequence와 독립적으로 갱신한다.
- 자연 run 완료와 FINISH_OWNER는 이미 생성된 World와 support, 늦은 행을 보존한다. RaidFlow의 자동 다음 Entry는 같은 epoch/source/gameplay pin에서 새로운 common start clock으로 시작하며 이전 tail을 이관한다. 수동 Play/Restart, Stop, death, gate clear는 취소 경계를 유지한다.
- 자연 완료 뒤에도 retained base revision을 GC root에 포함한다. late join은 종료되지 않은 World play를 기존 reliable 메시지로 재전달한다.
- `shared_ptr<SERVER_WORLD_ENTITY>`는 Server 프로세스 내부 수명 소유권이다. Shared packet/wire ABI를 확장하지 않았다.

## 실행 검증

Client/UI 및 listener를 실행하지 않고 Debug Server 계약 flag를 사용했다. 최신 source publisher를 별도 `out/KoukuRowLifetime20260920/DataFiles` fixture에 게시하여 사용했다.

- 전체 Debug Server Build 성공. 최종 강화 검사 빌드 `server-build-final.log`.
- `--kouku-bundle-contract-test`: 0 failures, `kouku-bundle-v4.log`. Stage 500ms/row 1000ms 실제 Room에서 먼저 Stage를 해제하고, 마지막 contact success와 timeout 각각 successor를 한 번만 시작한다.
- `--card-maze-contract-test`: 0 failures, `card-maze-v2.log`.
- `--bingo-contract-test`: 0 failures, `bingo-final.log`.
- `--kouku-object-overlap-contract-test`: 0 failures, `kouku-object-overlap-final.log`.
- 실제 `Run_KoukuSaydonLogicRuntimeContracts`만 호출하는 별도 headless probe: 0 failures, `logic-probe.log`. 제품에 `--kouku-logic-contract-test` flag는 없다.
- `--kouku-support-surface-contract-test`: 새 독립 수명 검사 전부 PASS, `kouku-support-surface-v3.log`. Stage 5초/수명 11초에서 6초에 늦은 Logic 피해와 World 및 Summon을 만들고, 새 본체 sequence 보존, source/current 원점 분리, exactly-once, catalog GC, counter/shield, 자동 Flow Entry continuation, absolute expiry와 Stop 정리를 검증했다.
- 같은 Support 전체에는 기존 네 실패가 남는다: finite card lifetime burst, swept card contact explosion, tracker half-speed, repeated-tick tracker. 새 tail 검사와 분리하여 기록한다.

## 아직 전체 통과로 표시하지 않는 검사

별도 Product probe(`product-probe-v2.log`)에서 Mario의 모든 count/선택적 후속/return/cancel/death/disconnect/last-tick single entry 및 10·11-field MARIO_ENTER parser 검사가 PASS했다. 고정 count/pool·선택적 no-entrant success를 fixture에 명시하고, 새 pinned return point를 기존 typed MarioReturn으로 소비하며, 자연 완료는 Members.empty 대신 phaseINACTIVE로 판별하도록 교정했다. terminal Stage의 첫 평가 tick을 포함하는 시각도 기존 Brain 규칙에 맞췄다.

같은 Product probe 전체는 7 failures다. disabled gate spawn fixture가 enabled Gate1 원본 boss를 전제하지만 현재 게시 worldbootstrap에서는 모든 gate boss placement의 enabled 값이 0인 차이가 확인된다. 이 별도 fixture는 이번 범위에서 수정하지 않았다.

넓은 `--contract-test`의 `full-contract-v2.log`는 38 failures로 완료됐다. 이 binary는 마지막 Summon admission 수정과 최종 fixture 교정 이전에 시작된 것이며, 최종 코드 전체 통과 증거로 사용하지 않는다. 구형 Bundle/Mario 기대 외에도 Valtan triangle, DimensionMaster T 거리, Bern placement/navigation, projectile damage, Valtan preset 등 여러 authoring/fixture 차이 실패가 보여 전체 통과로 기록하지 않는다. 마지막 Bundle 및 Product focused 재검사에서는 이 중 구형 Bundle 2개와 Mario 11개의 기대 불일치를 교정해 통과했다. 나머지 범위 전체를 재실행한 것으로 간주하지 않는다.

검사 fixture Gameplay.bootstrap SHA256은 `4645B18C2BB8115EE5CDF608D34B0A2214A060344CA7001D5BED2788F199E890`이다. 이후 phase2 등 다른 담당 authoring 변경은 이 격리 fixture에 재게시하지 않았다. 설치된 데이터와 Server 재시작 및 사용자의 최종 화면 판정은 별도 단계다.

변경 Server C++와 focused 문서 `git diff --check` PASS. 최종 source와 fixture 진단 출력 제거분은 통합 Product 빌드에 포함한다.

## 최종 삼각형 반경 검사

사용자 요청으로 포털 삼각형 반경이 9에서 13.5로 확대된 현재 게시값에 맞춰 `ServerGameplayContractTests_RevisionProtocol.cpp`의 exact fixture 문자열 세 곳만 교정했다. 기존 두 실패는 검사 입력을 교체할 원문 문자열이 구형 반경을 사용하여 찾기 단계에서 실패한 것이었다. 실제 `Run_RevisionProtocol` 전체를 별도 headless probe로 다시 실행해 0 failures를 확인했다(`out/KoukuRowLifetime20260920/revision-probe.log`). 각도 합이 360도를 초과한 radial volley와 정삼각형을 벗어난 ghost portal volley의 거절 검사가 모두 PASS했다. 실제 geometry/nav의 화면 판정을 수행한 것으로 간주하지 않는다.