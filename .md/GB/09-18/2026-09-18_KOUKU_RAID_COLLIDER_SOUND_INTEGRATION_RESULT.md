# 쿠크 레이드·공격 판정·원본 사운드 통합 결과

2026-09-19 현재: **main 병합, 소스 연결, 실제 저장본 설치, Gameplay/World/Map 게시, Release Product 빌드 완료. 폴더 선택 EXE가 있는 바이너리 전용 ZIP 생성 및 격리 설치 검증 완료.** 실제 Client 화면·청취는 실행하지 않았다.

## 통합한 실행 계약

- main `d2d3563b4d50ea545b1d25dd5a94923e9304e0d4`를 fast-forward하고 사전 미커밋 변경을 복원했다. revision/codec/packet ordinal 충돌 3개를 해결했다. 원본 byte backup과 stash를 보존하며 기존 main 메시지 번호를 유지했다. protocol은 93이다.
- Release 진입 콜라이더의 기존 PLAY_SEQUENCE가 Server raid 준비를 시작한다. 실제 참가자 1~4명 모두의 Action·Sequence·Gameplay revision 준비 ACK 뒤 공통 Server 시각으로 연출과 스폰을 진행한다. 솔로에서 가짜 플레이어 4명을 만들지 않는다.
- 보스 사망 후 자동 10초 이동은 제거했다. WAIT_GATE에서 main의 클리어/MVP·관문 투표 UI를 기다리고, 전원 진행 승인 후 다음 관문 연출을 시작한다. 재시작은 현재 관문 연출부터, 마지막 관문은 나가기/재시작 UI를 유지한다. 거절·투표 만료는 현재 대기를 유지한다.
- 1관문15개 → 2관문14개 → 3관문16개의 저장 Flow를 소비한다. 목록 끝에서 자동 반복하지 않고 보스 사망을 기다린다. 2관문 단독 패턴의 반대 보스는 Idle, 실제 멤버 둘이 든 Bundle은 함께 진행한다.
- 카드미로는 해당 문양 처치 위치에 개인 포탈을 생성하고, 망원경 담당자만 시야를 확장한다. 담당자 외 N-1명이 중앙으로 돌아오면 전원이 Gate2로 복귀하고 피자로 이어진다.
- CARD_DICE_BIND는 Duration 동안 살아 있는 한 명을 무작위로 제외하고 나머지만 속박한다. 솔로는 속박하지 않는다. 카드 추적은 같은 비속박 대상을 사용하며 종료·취소는 자신의 속박만 해제한다. 돌진 카운터의 비어 있던 judgementKind를 COUNTER_WINDOW로 연결하고 성공 후 그로기를 재생한다.
- P88/P91/P92/P93는 각각 Mario1~4 진입 창을 사용한다. 파티는 실제 입장 성공 다음 tick에 P33, 솔로는 실제 복귀 착지 후 P33이다. Parent의 긴 저장 duration은 유지한다. 마지막 tick에 큐된 입장도 완료 정리보다 먼저 commit한다.
- P33의 비진입자는 아이언메이든 (-7.07,1.32,934.43)에서 1.25m 간격으로 배치한다. 3~4인일 때만 한 명을 cage에 속박한다. 보스는 (5.96,1.30,950.59)에서 중앙을 보고, 무력화 성공은 P42, timeout은 전원즉사다. 자연 출구와 복귀 명령은 같은 pin한 귀환점을 사용한다.
- 갈고리와 즉사 칼날은 한 번, 일반 칼날은 반복한다. 즉사 칼날은 지정 보스 위치에서 아이언메이든으로 이동하며 접촉 즉사를 사용한다. 두 칼날의 이펙트는 모델 spin을 상속하지 않고 위치를 따른다. 새 world39는 데이터 정의이며 새 WModel은 추가하지 않았다.
- 원본 갈고리·즉사 칼날 선택, MAP 그룹 위치/yaw 수정과 World Preview를 연결했다. 쇼타임 추적 과녁은 보스 생성 위치에서 참가자별로 시작한다. 회전 응답 계수는 1→10이고 authored 시작 시각은 유지한다.
- latejoin admission에 현재 GateProgress 상태를 함께 보낸다. 관전자는 투표할 수 없다. WAIT_GATE의 첫 관문 화면 초기화, G1 재시작 때 기존 책/standing owner 반환과 실패 복원을 연결했다.

## 저장본·사운드·판정

설치본은 Action1750 / Sequence64 / Gameplay.world8797 / WorldSequences2129이다. 최신 디스크 hash 대조·백업·원자 교체 transaction `out/transactions/kouku-raid-3f48276546204843b1c25a1cab3c5bfd`에서 Data5개+새 WAV18개를 설치했다. 충돌0, 설치23개 파일 hash가 후보와 같다.

사운드는 원본487 event 중480개를 복원해 Action691곳+Sequence31곳, 총722곳에 연결했다. 새 WAV18개는227,088,088bytes이며 기존1,042개를 재사용한다. 사용자 최종 요청에 따라 ZIP에 사운드/Resources를 넣지 않고 신규18개 상대 경로만 `NEW_SOUND_PATHS.txt`로 전달한다.

공격은 원본 직접 shape193개(ring8개 포함)+저장 MAP impact31개, 총224개 후보를 설치했다. 동적 투사체17개 typed template은 기존 Server CombatObject 판정을 사용한다. 마리오 World collider는 normal24+hook30+instant8이며 hook30은 한 번의 갈고리 이동을 두 가시성 구간으로 나눈 판정이다. 이펙트 particle마다 중복 collider를 만드는 구조가 아니다. 일반 피해10% 및 과녁 반경 등 튜닝값은 원작 수치 복원과 구분한다.

원본 callback116개 중 기존 PURSUIT27·Albion3·현재 MAP impact로 표현한 원본14·비피해 NPC/FX11을 구분했다. 나머지61개는 원점/조건/반복 시각을 확정하지 못해 임의 판정을 넣지 않았다. 사운드 control-only/재생 원본 없음7 event 및 원본 clock/key가 모호한9 clip binding도 미확정이다. 별도1 clip은 trim 구간에 event가 없어 추가 불필요하고, P8 SCENE03A 소리는 parent P4에 이미 있어 중복하지 않는다.

## 검증과 게시

| 검증 | 실제 결과 |
|---|---|
| 최신 Product projection | 패턴80, Bundle8, Stage455, P35 독립 window81 |
| Debug Server/Shared fresh build 및 통합 실행 | 7,435 checks / failures0 |
| Release 진입 collider·ACK·관문 UI·latejoin 초기 snapshot | 660 checks / failures0 |
| Release Mario 실제 contact·party/solo·복귀·jump | 108 checks / failures0, 12경우 authoredEntry=1 |
| 카드미로 실제 1~4인 room | 문양 포탈·담당자 시야·N-1 귀환 failures0 |
| Protocol93 codec | 464 checks / failures0 |
| Client gate 정책·presentation ownership | 15 +42 checks / failures0, 관련 Debug/Release TU compile PASS |
| 설치기/Python | CAS15, raid projection7, blade2, sound7 tests PASS |
| 프로젝트 XML·diff | 8개 XML parse, git diff --check PASS |
| Release Product | 전체 `20260918T151246346Z-release-product.json` PASS; 최종 변경 포함 증분 `20260918T151534562Z-release-product.json` PASS |

실제 Gameplay/World/Map publisher를 실행했다. live Gameplay.bootstrap은38,926rows /13,282,173bytes이며 검증 후보와 SHA256 `2320b718e07e27182596fe29ae1adcc8e66f0b8e532b70bc48952ee5d9501d42`가 같다. World bootstrap2개도 byte 동일하다. Map publish 첫 시도는 파일 점유로 기존 파일을 보존한 채 실패했고 재시도 성공했다. runtime WorldSequences hash는 `ef0f39be224edcf400dab46d5569884c99eb10eb311330d1d06fa2cc926f8195`다.

쇼타임 Profiler 평균1913.148ms 중 HistoryUpdate1739.255ms(90.91%)가 반복 루트 pose 재계산에 쓰였다. checkpoint/pose cache 수정 후 실제 CModel 함수 반복조회는44299ms에서20.7249→0.001721ms,55637ms에서85.336→0.0016825ms다. 이는 전체 effect seek/GPU/FPS 측정값이 아니다. 사용자의 최종 화면·청취와 실제4개 Client 동시 플레이는 미실행이다.

## 배포 및 확인 경계

최종 사용자 정정에 따라 배포본은 `C:/Users/user/Desktop/LostArk-Release-20260919-EXE.zip` (17,649,962 bytes, 약16.8MiB)이다. 최상위 `LostArk.exe`를 실행해 기존 LostArk 폴더를 선택하면 Client Release 모듈7개와 셰이더141개를 해당 프로젝트에 적용하고 Client를 시작한다. 기존 파일은 `out/RuntimeDeliveryBackups`에 백업하며 동일 파일은 교체하지 않는다. ZIP에 ChangedData, Data, DataFiles, Resources, 사운드 미디어는 하나도 넣지 않았다. 선택 프로젝트의 기존 데이터와 리소스를 사용하며, 신규18WAV는 경로 목록만 제공한다. Server.exe는 서버 담당자용 별도 파일로 포함하며 자동 설치·실행하지 않는다. 이전 `LostArk-Release-20260919.zip`은 EXE가 없고 ChangedData가 포함된 이전 형식이므로 이번 전달본이 아니다.

WinForms launcher만 별도 컴파일했으며 Client 전체 재빌드는 하지 않았다. 격리 폴더에서148개 파일SHA일치, 기존 Client 백업, Data/DataFiles/Resources 보존, 재실행 시 교체0개, 잘못된 루트 거부를 확인했다. 실제 프로젝트는 read-only 실행 경로 점검만 했다. 압축153개 파일CRC·SHA 검증을 완료했고 Client/UI 실행은0이다.

빌드/파일 설치/게시가 실행 중인 Server 메모리나 도구 draft를 자동 갱신한 것은 아니다. 기존 Client/Server를 에이전트가 종료하거나 자동 실행하지 않았다. 새 protocol93 실행 파일로 Server와 Client를 함께 재시작해야 한다.

증거 정본: `out/KoukuRaidIntegration20260918/{install-receipt.json,installed-files-verification.json,live-publish-verification.json,server-integration93.receipt.json,PATTERN_FLOW.md,NEW_SOUND_PATHS.txt}` 및 해당 하위 작업 receipt.

최종 ZIP SHA256: `7656ef5d81931d2c9039d0258563dffb6f71bd5ee15c82bcc3b21c4bcc22e861`. 배포/설치 증거는 `out/KoukuRuntimeDelivery20260919/{launcher-delivery-result.json,launcher-test-result.json}`이다.
