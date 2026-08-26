# 발탄 핵심 런타임 재생 결함 수정 결과

## 완료 범위

- 첫 등장 `VALTAN_ENTRANCE_WHIRLWIND/SWEEP`가 일반 휠윈드와 같은 Product Effect 두 개를 exact cue로 사용한다.
- `VALTAN_DASH_CHARGE`는 20m/1500ms Server 이동과 Client snapshot 보간을 사용하며 opening charge도 stage 거리로 이동한다.
- 돌진 stage에서는 generic body-contact가 여러 벽을 먼저 파괴하지 않는다. 159 wall exact receiver 한 개가 새 mutation을 commit한 같은 fixed tick에 `BREAKING`과 `WALL_CONTACT -> GROGGY`가 함께 적용된다. unbound/이미 소비된 receiver는 정지하되 가짜 GROGGY를 만들지 않는다.
- HIGH_JUMP는 TAKEOFF 1133~1500ms 상승, AIRBORNE 정점 유지, LAND 0~267ms 하강을 Server가 소유한다. Server-authoritative Client Valtan은 skeleton `b_root` translation을 다시 더하지 않는다.
- sky-axe는 도넛에서 유입된 Element를 제거했고, player/random axe 모두 wave spawn pose에 고정된다.
- Phase 2/3의 20개 manual audition chain과 중앙점프 후 포효 chain은 clip 순서, source offset, mapping basis, play rate를 Product projection과 exact-join하도록 validator와 회귀 테스트를 추가했다.

## 근본 원인

Character Select의 local preview는 model clip clock과 authored `b_root`를 직접 재생하지만 Arena의 Server Actual은 Server stage clock, replicated world transform, Product binding/effect cue를 소비했다. 두 경로 사이에 motion subwindow, root-motion 단일 소유권, snapshot 보간, exact effect identity 검증이 없어서 Tool에서 맞춘 결과가 Arena에서 달라졌다.

상세 재발 방지 계약은 `2026-08-26_VALTAN_SERVER_PRESENTATION_PARITY_CONTRACT.md`에 기록했다.

## 검증

- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS
- Valtan Effect/Animation/Pattern focused Python tests: PASS
- Server x64 Debug build: PASS
- `Server.exe --contract-test`: PASS, failures 0
- Client x64 Debug build: PASS
- `PATTERNSOURCE` 53행은 10필드, `PATTERNMOTION` 3행은 14필드로 bootstrap v21 로드 확인

## 제외 및 수동 확인

- `VALTAN_FIST_IN_OUT` 도넛 자체 재생 문제와 사망 player Pattern Audition reset(G06)은 사용자 지시에 따라 이번 PR에서 제외했다.
- Client 화면과 Effect fidelity는 자동 PASS로 판정하지 않았다. 사용자가 Server + Client 실행 후 휠윈드, 돌진, 벽 충돌/GROGGY, HIGH_JUMP, 중앙점프 후 포효를 직접 확인한다.
