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

## 배포 재수정: DataFiles·Navigation·ESC 커서 (2026-09-19)

기존 `LostArk-Release-20260919-EXE.zip`은 Client 모듈7개와 CSO141개만 설치하고
Data/DataFiles를 제외한 배포본이었다. 이번 요청에서는 그 ZIP을 전체 실행 배포본으로
사용하지 않는다. Git 동기화와 publish 성공, ZIP의 실제 전달 범위를 각각 확인한다.

### 확인한 원인과 실제 수정

- 8f15a3c35(PR409)에 다른 세션의 쿠크·쇼타임 변경이 통합됐다. Action1750,
  Sequence64, World8797, WorldSequence2129, protocol93을 한 묶음으로 사용한다.
- Navigation은 Bern·Character Select 지역파일12개와 Server manifest2개가 빠졌고,
  발탄 Server blocker는104개로 정본/Client101개와 달랐다. Server owner로 전체 게시한
  뒤 Client/Server 각각36개, SHA36쌍 동일, manifest 참조 누락0을 확인했다.
- Composition publisher의 WorldSequence v3 strict validator에서 기존 `colliderTracks`와
  `loopFullPresentation`이 빠져 있었다. Map publisher/Client와 같은 시간·shape·damage·
  binding·loop 제한을 연결했다. authoring 필드 삭제나 unknown-field 완화는 하지 않았다.
- ESC 옵션 팝업은 클릭을 먼저 소비하여 `Is_Clicked`가 자기 항목 선택을 거부했다.
  `SystemOptionWindowView.cpp`에서 popup 선택 후 하위 UI를 차단하도록 순서를 고쳤다.
  커서 지원125개와 SystemOption 이미지59개는 로컬에 있어 자산 누락은 원인이 아니었다.

### 실행한 검증

- Composition unittest49개 PASS, 실제 전체 source graph validate PASS. 현재
  WorldSequence326개 instance 원본 보존 및 collider/loop 실패 입력 거부를 확인했다.
- 실제 옵션 입력 함수 본문을 사용한 CPU probe는 이전 코드에서 선택 관련3건 FAIL,
  수정 후 선택·동일항목·외부클릭·hover·held 입력6건 PASS. 하위UI click 누수0.
  OS 커서 적용/렌더는 stub이며 실제 화면 판정은 수행하지 않았다.
- Release Product PASS(36.9초): Client OBJ1/EXE1, Engine/Shared/Server 재컴파일0,
  CSO/PCH 재생성0. 기존 C4819와 DirectXTK PDB LNK4099 경고는 남아 있다.
  증거 `out/BuildPipeline/runs/20260918T161133688Z-release-product.json`.
- Server owner: Kouku/World/Navigation/파괴/Gameplay/아이템/탈것/칭호/발탄보상 PASS 또는
  현재 입력·출력 hash와 일치하는 REUSED. Client owner 최종 재시도도 전체 PASS/REUSED.
- publisher 및 build 로그는 `out/RuntimeRepublish20260919/`, 옵션 probe는
  `out/CursorOptionClick20260919/`에 있다. `git diff --check` PASS.
- 새 설치 wrapper fixture: Data/runtime 백업, 최초/재설치, SHA/경로/중복 거부,
  잠긴 Server 파일에 의한 실패 시 Data/runtime 복구 PASS. Client/UI 실행0.

### 최종 ZIP 전 진행 경계

사용자의 추가 요청으로 PR410을 포함한 최신 main417b2b126을 먼저 통합하고,
이번 수정도 PR merge한 최종 main에서 publish·Release Build·ZIP 검증을 다시 확인한다.
현재 이 절의 빌드 기록은 8f15a3c35 기반 수정본이며 PR410 포함 최종 빌드 기록이 아니다.
기존 EXE 전용 ZIP을 덮어쓰지 않으며 새 배포는 DataFiles 양쪽과 직접 소비 Data 보충분을
포함한다. Resources/신규18WAV는 팀 Drive 경계를 유지한다.

실제 화면·음향·4인 Client 동시 플레이는 사용자 확인 대상이다. 게시와 설치는 실행 중
Server 메모리나 도구 draft를 자동 갱신하지 않는다. Client/UI를 자율 실행하지 않았다.
