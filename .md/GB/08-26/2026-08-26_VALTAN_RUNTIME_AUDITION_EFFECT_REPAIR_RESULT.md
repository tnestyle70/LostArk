# 발탄 핵심 런타임 재생 결함 수정 결과

## 완료 범위

- 첫 등장 `VALTAN_ENTRANCE_WHIRLWIND/SWEEP`가 일반 휠윈드와 같은 Product Effect 두 개를 exact cue로 사용한다.
- `VALTAN_DASH_CHARGE`는 20m/1500ms Server 이동과 Client snapshot 보간을 사용하며 opening charge도 stage 거리로 이동한다.
- 돌진 stage에서는 generic body-contact가 여러 벽을 먼저 파괴하지 않는다. 159 wall 10개와 실제 먼저 접촉하는 109 외곽 wall 30개가 모두 exact receiver binding을 소유하며, 한 receiver가 새 mutation을 commit한 같은 fixed tick에 `BREAKING`과 `WALL_CONTACT -> GROGGY`가 함께 적용된다. 이미 소비된 receiver는 가짜 GROGGY를 만들지 않는다.
- 돌진으로 먼저 소비된 109 외곽벽 1개는 후속 109줄 `IMPACT`에서 `DESPAWNED`를 유지하고, 같은 stage transaction이 나머지 29개만 `BREAKING`으로 전환한다. non-outer destruction 상태는 바뀌지 않는다.
- HIGH_JUMP는 TAKEOFF 1133~1500ms 상승, AIRBORNE 정점 유지, LAND 0~267ms 하강을 Server가 소유한다. Server-authoritative Client Valtan은 skeleton `b_root` translation을 다시 더하지 않는다.
- sky-axe는 도넛에서 유입된 Element를 제거했고, player/random axe 모두 wave spawn pose에 고정된다.
- Phase 2/3의 20개 manual audition chain과 중앙점프 후 포효 chain은 clip 순서, source offset, mapping basis, play rate를 Product projection과 exact-join하도록 validator와 회귀 테스트를 추가했다.
- `GAMEPLAY_FOOTPRINT`는 finite·orthogonal·right-handed Valtan root의 축별 scale drift를 각각 제거한 뒤 authored `worldScale`을 적용한다. 따라서 도넛, 4연속 공격, 휠윈드, 돌진 빨간 장판과 130줄 전멸기 cue가 기존 Effect 문서 그대로 생성되며 shear, reflection, degenerate/nonfinite root는 계속 거부한다.

## 근본 원인

Character Select의 local preview는 model clip clock과 authored `b_root`를 직접 재생하지만 Arena의 Server Actual은 Server stage clock, replicated world transform, Product binding/effect cue를 소비했다. 두 경로 사이에 motion subwindow, root-motion 단일 소유권, snapshot 보간, exact effect identity 검증이 없어서 Tool에서 맞춘 결과가 Arena에서 달라졌다.

누락 Effect의 JSON, catalog row와 resource가 사라진 것은 아니었다. 실제 Arena Valtan root의 basis 길이가 `(0.997025, 1.0, 0.997025)`로 미세하게 누적 변형됐고, 기존 `GAMEPLAY_FOOTPRINT`가 이를 uniform scale이 아니라는 이유로 occurrence 생성 전에 거부했다. 해당 policy는 원래 축별 owner scale을 제거하고 authored footprint를 쓰므로 과도한 uniform 조건만 제거했다.

돌진의 same-tick 로직도 이미 Server에 있었지만 기존 데이터는 159벽 10개만 `VALTAN_DASH_CHARGE/CHARGE/COLLISION_IMPACT`에 연결했다. timeline의 `ORDINARY_WALLS_GONE` 이후 남은 109 외곽 receiver가 첫 접촉이 되면 mutation을 찾지 못해 CHARGE가 유지되고 generic body contact에서 벽만 늦게 무너졌다. 동일한 stable receiver/mutation binding을 30개 모두에 추가하고 publisher가 159+outer 총 40개 exact join을 검증하도록 보강했다.

상세 재발 방지 계약은 `2026-08-26_VALTAN_SERVER_PRESENTATION_PARITY_CONTRACT.md`에 기록했다.

## 검증

- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS
- `Publish-ValtanWorldDestruction.ps1 -Mode Validate/ContractTest/Publish`: PASS, groups 105 / bindings 157 / Dash impact 40(159벽 10 + outer109 30)
- Effect Tool All Effects: PASS, 10 tests
- Effect Tool saved rows: PASS, 32 tests / 7 skipped
- Boss Tool: PASS, 12 tests
- Valtan Model View composition: PASS, 10 tests
- Valtan Pattern Tree: PASS, 18 tests
- Valtan Pattern Master V2: PASS, 46 tests. 159+outer Dash binding 확장에 따른 source/product count 194→224도 exact contract로 고정했다.
- final Client/Server Debug `ClCompile`: PASS
- 별도 `ServerDashVerify.exe --contract-test`에서 159벽과 `ORDINARY_WALLS_GONE` outer109의 same-tick 정지/BREAKING/GROGGY, Dash 선행 파괴 1개를 보존하면서 후속 109줄이 나머지 29개만 전환하는 partial-batch 계약은 PASS했다. 실행 중인 canonical Server가 mutex를 소유해 concurrent-owner 검사 1건만 예상대로 실패했으며 canonical full link/zero-failure 재실행은 프로세스 종료 뒤 수행한다.

## 제외 및 수동 확인

- clip-01/02, 휠윈드의 두 SPIN cue와 floor-wipe support cue는 서로 다른 정상 stage/layer이므로 삭제하지 않았다. 현재 Product에서 참조하지 않는 문서는 receipt/tool fixture와 결합돼 있어 이번 런타임 복구와 섞어 제거하지 않는다.
- Client 화면과 Effect fidelity는 자동 PASS로 판정하지 않았다. 사용자가 최신 Server + Client를 재시작한 뒤 도넛, 4연속 공격, 휠윈드, 돌진 빨간 장판, 130줄 전멸기와 돌진 첫 벽 접촉의 즉시 GROGGY를 직접 확인한다.
