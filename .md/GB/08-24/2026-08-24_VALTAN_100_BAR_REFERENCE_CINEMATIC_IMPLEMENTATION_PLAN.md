# 발탄 100줄 원작 컷신 구현 계획

## 1. 목표

- 영상 `tSpXa0v9TXw`의 19:32~19:43 구간처럼 100줄 패턴에서 발탄의 고공 상승, 붉은 소용돌이 하늘, 컷신 카메라, 착지 애니메이션을 하나의 Server 시간축으로 재생한다.
- 제품 트리거는 사용자가 정한 `100줄`과 기존 `VALTAN_FOUR_PILLARS_105` stable ID를 그대로 유지한다.
- 컷신 종료 뒤에는 기존 잠금 대상, 즉사 원뿔, 4비석 생성 계약으로 복귀한다.

## 2. 실측 원인과 구현 결정

- 현재 100줄은 camera cue가 0개이고 `serverMotion`도 없어, overhead `y=74`에 활성화된 붉은 하늘을 일반 follow camera가 볼 수 없으며 발탄 좌표도 상승하지 않는다.
- 현재 4.7초 stage 시간은 원본 `mesh_att_battle_8_01_start/loop/end`의 `1.933 + 5.500 + 3.200 = 10.633초`보다 짧아 애니메이션과 source Effect가 잘린다.
- 100줄에 `LEAP_TO_TARGET`, 중앙 fallback anchor, apex `46m`를 저작한다. stage 0은 제자리 수직 상승, stage 1은 시작 때 잠근 대상 위치로 가속 낙하하며 stage 2부터 착지 좌표를 고정한다.
- stage 시간은 `TAKEOFF 1933 / YELLOW_ZONE 5500 / TARGET_CONE 3200 / RECOVERY 1300ms`로 맞춘다. 컷신 카메라는 앞의 두 stage만 소유하고 `TARGET_CONE` 진입과 함께 gameplay camera를 돌려준다.
- 100줄 전용 camera cue는 근접 충전 -> 상공 추적 -> 붉은 vortex 응시 순서로 저작한다. 카메라와 하늘은 `BOSS_XZ` 추적으로 잠긴 대상에게 이동하는 발탄의 X/Z를 따라간다. YELLOW_ZONE 종료 300ms 전에 조작 카메라를 돌려주고 TARGET_CONE 판정은 원본 착지 notify의 230ms에 맞춰, 컷신이 끝나는 프레임에 즉사하지 않게 한다.
- 기존 4개 sky cue는 같은 asset ID를 유지하고 새 stage 시간에 맞춰 window를 동기화하며, Y축은 저작 기준을 유지해 서버 상승과 중복 이동하지 않게 한다.
- 애니메이션은 start/end를 non-loop, airborne/recovery loop를 loop로 고정한다. source action 420610의 영상 검토 branch를 exact carrier Effect owner로 승격해 충전과 착지 Effect를 100줄 stage에 연결한다.

## 3. 구현 단위

1. Encounter 100줄 stage timing과 `LEAP_TO_TARGET` motion을 저작하고 provenance receipt를 동기화한다.
2. 100줄 TAKEOFF/YELLOW_ZONE camera cue와 4개 sky window를 같은 Server stage tuple에 묶는다.
3. pattern animation loop 정책을 원본 start/loop/end/loop 순서에 맞춘다.
4. 420610 seq=2 reviewed selection에서 100줄 전용 carrier-v1 Effect 문서와 cue를 생성한다. 기존 역사 receipt는 변경하지 않고 신규 두 owner와 26개 요소만 additive proof로 봉인한다.
5. Server contract test로 100줄 crossing, 고공 apex, 잠긴 대상 착지, targetless fail-close를 고정한다.

## 4. 주요 변경 파일

- `Data/Encounters/Valtan/ValtanEncounter.json`
- `Data/Encounters/Valtan/ValtanCinematicCamera.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Client/Public/ValtanCinematicCameraDocument.h`
- Valtan carrier-v1 selection, materializer, catalog, cue, authored Effect와 receipt 생성물

## 5. 검증 계획

1. 변경 JSON parse, balance provenance sync, Gameplay Balance Validate/Publish.
2. carrier-v1 materializer check/unit test와 Effects Validate/Publish.
3. Server contract test에서 100줄이 한 번만 발동하고 stage 0 상승, stage 1 잠긴 대상 낙하, stage 2 정확 착지를 검증한다.
4. Server와 Client Debug/Release build, `git diff --check`를 실행한다.
5. 에이전트는 Client를 실행·조작하지 않는다. 사용자가 직접 100줄 진입 후 근접 컷, 상승 애니메이션, 붉은 vortex, gameplay camera 복귀, 대상 고정, 착지와 4비석을 육안 판정한다.

## 6. 구현 상태

- 코드·데이터 구현: 완료
- publisher·focused harness·Debug/Release 빌드: 완료
- 제품 화면 육안 판정: 사용자 확인 대기
