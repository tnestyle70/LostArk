# 2026-08-27 발탄 전투 BGM 타임라인 구현 결과

## 구현 상태

발탄의 Server-authoritative snapshot을 Client presentation이 소비해 다음 음악 상태를 한 번씩
전환하도록 구현했다.

| 전투 경계 | 재생 파일 | 재생 방식 |
|---|---|---|
| `LEVEL::VALTAN_ARENA` 활성화 완료 | M01 KeepGoing | loop |
| `spawn.valtan.stage02.miniboss`의 Lugaru despawn | M04 KeepGoing2 | loop |
| `VALTAN_ENTRANCE_WHIRLWIND` 시작 | M05 Valtan Revive | one-shot |
| 등장 패턴 종료 후 첫 일반 전투 | M06 Valtan 1st Phase | loop |
| `VALTAN_GHOST_TRANSITION_15` 시작 | M07 FakeDead | one-shot |
| `valtan.mechanic.ghost-transition-15.ghost` 진입 | M08 Valtan Phase2 | loop |
| `WORLD_ENTITY_ACTION::DEAD` 확정 | M09 Valtan Dead | one-shot |

M04는 Client 타이머나 플레이어 위치가 아니라 Server의 reliable world-entity despawn을
사용한다. 해당 spawn group은 `MINIBOSS_LUGARU` 한 마리와
`ALL_WAVES_CLEARED` completion policy로 구성되어 있으므로 그 despawn은 선행 2단계의
가시적인 종료 경계다. 같은 snapshot의 반복 수신은 현재 음악을 재시작하지 않는다. M07의 마지막 packet이 유실돼도
다음 일반 snapshot에서 M08로 복구한다. 음악 파일 누락은 해당 presentation만 격리하며
Server snapshot 적용과 전투 진행은 실패시키지 않는다.

## 런타임 리소스

`Client/Bin/Resources/Sound1/BGM/Valtan`의 원본 WAV 일곱 개를 Sound catalog가 선언한
`Client/Bin/Resources/Sound/BGM/Valtan` 경로에 배치했다. 일곱 파일 모두 원본과 runtime
복사본의 SHA-256이 동일하다. 추가된 M01은 14,400,078 bytes, M04는
23,775,154 bytes이며 두 파일도 원본과 SHA-256이 동일하다.

## 자동 검증

- Engine x64 Debug 빌드: PASS
- `UpdateLib.bat Debug`: PASS
- Client x64 Debug C++ compile/link: PASS
- M01, M04, M05~M09 runtime WAV 존재와 원본 SHA-256 일치: PASS
- Sound catalog asset ID와 encounter pattern/action ID 대조: PASS
- stage02 spawn group이 Lugaru 한 마리와 ALL_WAVES_CLEARED 정책인지 대조: PASS
- 표준 Client pre-build: 기존 Valtan presentation debug/manual 목록 불일치로 중단
  - 현재 debug 목록에 `dead`, `respawn`이 있으나 manual audition 목록에는 없음
  - 이번 BGM 변경과 무관하므로 기존 데이터를 임의 수정하지 않았다.
  - C++ 검증에서는 해당 pre-build만 일시적으로 건너뛰었고 프로젝트 파일은 즉시 원복했다.

## 수동 검증 경계

Client를 에이전트가 실행하거나 음향 결과를 대신 판정하지 않았다. 사용자가 Server + Client를
재시작한 뒤 레벨 진입 M01, Lugaru 종료 M04, 등장 M05, 첫 일반 전투 M06,
14줄 망령화 M07, 망령 전투 M08, 최종 사망 M09를 직접 들어야 최종 runtime audio
검증이 완료된다.

## 2026-08-28 PR 병합 전 교정

새 `VALTAN_ENTRANCE_CINEMATIC`은 카메라만 소유하며 BGM edge를 추가하지 않는다.
합성 병합 트리에서는 이 non-IDLE pattern을 late join으로 오인해 M04에서 M06으로 바로
넘어가고 M05를 생략했다. 이제 cinematic snapshot은 M04를 유지하고, 기존 계약대로
`VALTAN_ENTRANCE_WHIRLWIND`가 M05를 시작하며 그 뒤 첫 일반 snapshot이 M06을 시작한다.
카메라 계약 테스트가 이 순서와 late-join M06 경로를 함께 고정한다.

위 런타임 리소스 PASS는 저작 당시 로컬 배치 결과였으며 Git dependency closure가 아니다.
2026-08-28 clean PR worktree와 현재 공유 PC를 다시 확인한 결과 Bern 1개와 Valtan 7개,
총 8개 WAV가 exact runtime 경로에 없다. 코드는 누락된 음악 presentation만 격리해 전투를
계속하지만, 팀 관리 `Client/Bin/Resources/Sound` 입력을 다시 배치하기 전에는 BGM 재생을
완료 또는 청음 PASS로 기록할 수 없다.
