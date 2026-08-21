# 2026-08-21 발탄 109줄 외곽 벽 이음새 보강 결과

## 1. 판정

109줄 외곽 벽의 하단 성벽 띠 사이에 보이던 반복 공백을 메우는 데이터와 파괴 계약을
구현했다. 기존 `DEPLOY_ITR_02306` 30개는 이동하거나 교체하지 않았고, 각 벽의 실제 각도
기준 +6도 중간점에 짧은 같은 계열 연결벽 `DEPLOY_ITR_02307` 30개를 추가했다.

자동 데이터·Server·Client 전용 하네스 검증은 통과했다. Client/UI는 에이전트가 실행하지
않았으며, 실제 화면에서 이음새 밀도와 109줄 붕괴 모양이 적절한지는 사용자가 최종 판정해야
한다.

## 2. 최종 데이터 계약

| 항목 | 결과 |
|---|---:|
| 109 외곽 destruction group / IMPACT binding | 30 / 30 |
| 원본 `DEPLOY_ITR_02306` | 30, transform 불변 |
| 연결벽 `DEPLOY_ITR_02307` | 30, scale 1.52 |
| 외곽 group member | 60 |
| debris source emitter / suppression alias | 30 / 30 |
| 동시 파편 actor | 360 |
| 일반 collider-contact binding | 0 |
| collision / navigation | 변경 없음 |

연결벽 ID는 `1091000000000001..1091000000000030`이다. 기존 source suffix 순서는 각도순이
아니므로 각 source의 실제 transform에서 각도를 계산하고 같은 suffix filler를 정확히 +6도에
배치했다. 전체 filler 각도 집합은 `6,18,...,354`도다.

연결벽은 기존 group의 두 번째 member이자 기존 debris emitter의 suppression alias다. 따라서
109줄 IMPACT에서 원본과 같은 tick에 전부 숨지만 별도 12-piece 파편 actor는 만들지 않는다.
원본 30개가 계속 360개 파편을 생성하므로 720개 동시 actor로 성능 부담을 두 배로 늘리지
않는다.

## 3. 형상 기준

기존 벽 pivot 간격은 약 3.366m인데 하단 몸체 폭은 약 1.975m라 배치 누락 없이도 큰 공백이
생겼다. `DEPLOY_ITR_02307`을 6도 중간점에 scale 1.52로 배치한 triangle slice 검사에서
0~5m 하단 성벽 띠의 최악 지점도 약 0.0178m 겹쳐 공백이 닫힌다. 높은 첨탑 사이 공간은
원본 실루엣에도 존재하므로 막지 않았다.

## 4. 변경한 정본과 생성물

- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements`
- `Data/Encounters/Valtan/ValtanWorldEvents.json`
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json`
- `Tools/WorldPipeline/sync_valtan_109_outer_wall_gap_fillers.py`
- `Tools/WorldPipeline/test_sync_valtan_109_outer_wall_gap_fillers.py`
- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`
- `Server/Private/WorldDestructionBootstrapContractTests.cpp`
- publish된 Map deploy와 World destruction projection/presentation/runtime bootstrap

기존에 별도로 수정 중이던 `Gameplay.world.json`의 receiver collision, 발탄 pattern/brain/catalog,
floor 84/30, 일반 벽, entrance sweep 데이터는 이 작업에서 수정하지 않았다.

## 5. 자동 검증

- filler sync 단위 테스트: `6/6 PASS`
- 실제 authoring `--check-only` 및 idempotence: `PASS`
- Map authoring publish: `PASS`, deploy header `151`
- authoring/runtime deploy: 정규화된 151개 데이터 행 동일
  - authoring은 CRLF, runtime은 publisher가 canonical LF로 출력하므로 raw SHA는 다름
- Valtan destruction Validate/ContractTest/Publish: `PASS`
- 최종 graph: `30 groups / 60 placements / 30 emitters / 30 aliases / 360 fragments`
- projection/presentation/bootstrap revision:
  `3010eee28b8c8f82365609470b431a6a4e46313a721563d2fb4736d111379c88`
- WorldGameplay Validate: `PASS`
- Navigation Validate: `PASS`, filler collision/nav 추가 없음
- Server Debug/Release build: `PASS`
- Server Debug/Release `--contract-test`: `failures: 0`
- ClientFrontendHarness Debug/Release build: `PASS`
- Debug/Release `--world-destruction-projection-fast`: `failures: 0`
- 변경 파일 대상 whitespace/diff 검사: `PASS`

전체 ClientFrontendHarness에는 이 작업과 무관한 기존 Effect/카메라 계열 실패가 남아 있어
전용 world-destruction fast suite를 이 작업의 판정 근거로 사용했다.

## 6. 현재 저장소 병합 상태로 인한 별도 차단

검증 후 작업 트리가 `merge/valtan-combat-runtime`으로 전환되었고 여러 파일이 `UU/AA` 병합
상태가 되었다. Client Debug/Release 전체 빌드를 다시 시도했으나 이번 벽 변경이 아니라
`Shared/Private/Network/PacketMessages.cpp`에 남아 있는 `<<<<<<< / ======= / >>>>>>>`
충돌 표식 때문에 Shared 컴파일에서 중단됐다. 이 파일의 어느 쪽을 채택할지는 별도 전투
런타임 병합 결정이므로 이 작업에서 임의 해결하지 않았다.

같은 병합 상태에서 publisher를 다시 실행하면 `Data/Actors/BossCatalog.json`과
`Data/Encounters/Valtan/ValtanEncounter.json`의 충돌 표식 때문에 WorldGameplay와 destruction
전체 입력 JSON 파싱도 중단된다. 반면 충돌 파일을 소비하지 않는 filler 단위 테스트 `6/6`,
실제 authoring `--check-only`, authoring/runtime deploy 정규화 행 동일성, Navigation Validate는
현재 병합 상태에서도 다시 PASS했다.

따라서 현재 결과는 109벽 데이터·publisher·Server contract·Client 전용 presentation harness까지
닫혔고, 병합 충돌 해소 뒤 Client 전체 build를 한 번 더 실행해야 하는 상태다.

## 7. 사용자 화면 확인

1. 병합 충돌을 해결한 빌드에서 로컬 Server와 Client를 시작한다.
2. Lobby에서 Valtan으로 들어간다.
3. 패턴 실행 전 외곽 하단 성벽 띠의 큰 반복 공백이 닫혔는지 본다.
4. 일반 공격과 돌진으로 109 외곽 벽이 미리 사라지지 않는지 본다.
5. F1의 Valtan Pattern Audition에서 109 패턴을 Reset 후 실행한다.
6. 109 IMPACT에서 원본 30개와 연결벽 30개가 함께 사라지고, filler 잔상이 남지 않으며,
   바닥 84/30 sector는 무너지지 않는지 확인한다.
7. 파편은 기존 360개 계약을 유지하므로 붕괴 순간 FPS와 밀도도 함께 확인한다.

사용자 화면에서 남은 공백이나 과한 겹침이 보이면 같은 중간점 계약을 유지한 채 filler scale만
재조정한다. 에이전트는 아직 visual PASS를 선언하지 않는다.
