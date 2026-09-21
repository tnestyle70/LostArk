# 쿠크 5개 공간의 0.5m 네비게이션 재베이크 결과

## G00. 실제 반영 범위

1·3관문 중앙 원, 2관문 위쪽 포커판, 최초 시작 발판, 3관문 입장 전 공간을
기존 4m 기본 격자 위의 0.5m 세부 격자로 게시했다. 마리오·빙고·카드미로의
기존 navigation source/paint와 runtime, 기본 격자 자체는 byte 단위로 보존했다.
Client와 Server가 사용할 디스크 데이터의 게시가 완료됐으며 실행 중 메모리를
갱신하거나 Client/UI를 자동 실행한 것은 아니다.

| 세부 격자 | 실제 공간 | X/Z 범위 | 격자 수 |
|---|---|---|---|
| StartFine | 최초 spawn와 jump.1~3 발판 | X[-8,11], Z[-22,-6] | 38×32 |
| Gate1Fine | 1관문 중앙 원 | X[-16,16], Z[721,754] | 64×66 |
| Gate2Fine | 2관문 위쪽 포커판 | X[-4.5,20.5], Z[307,332.5] | 50×51 |
| Gate3Fine | 3관문 중앙 원과 입장 전 고지대 | X[-32,17], Z[926,974] | 98×96 |

3관문 두 공간은 실제 바닥 footprint가 겹치지 않으나 둘레 AABB가 겹친다.
서로 겹칠 수 없는 detail region 계약에 맞춰 Gate3Fine 하나에 서로 다른 실제 높이로 담았다.
2관문 최종 범위는 상판 floor15 기준이다. 중간 조사용 `candidate-contours.png`는
이 범위로 좁히기 전의 하부 바닥을 포함한 모델 투영도이며 최종 데이터나 게임 화면이 아니다.

## G01. 생성과 저장 계약

현재 mapplacements의 stable ID·저장 transform과 설치된 WModel 삼각형을 기존
`CNavGridBaker::Build/Save_Source`로 베이크했다. 모델 169개의 hash를 최종 설치 직전에
재확인했다. 새 네비 런타임, UE navmesh 직접 수입, 바닥 높이 일괄 평탄화는 사용하지 않았다.

최상단 삼각형을 선택하는 baker 때문에 실제 보행 바닥을 가린 책·종이 접힘,
작은 철사, 머리 위 장식은 실측된 하부 바닥 높이로 paint했다. StartFine 24칸,
Gate1Fine 7칸, Gate3Fine 5칸이다. Gate3Fine의 의자 좌면 26칸과 바닥 밖 장식 1칸은
BLOCKED로 남겼다. Gate2Fine에는 paint가 없다. 근거는 out의 geometry evidence에 보존했다.

2관문 상판의 실제 구멍 아래 바닥은 기존 Big Saydon spawn의 XZ 보행 검사를 위해
원래 높이를 보존한다. 상판에서 하부로 내려가는 보행 연결을 만들지 않았으며 상판 높이로
메우지 않았다. 실제 입장점에서 도달하는 1,004칸은 모두 Y=10.5599985~10.5950003의
상판이다. 도달 가능한 하부 칸과 도달 불가능한 상판 칸은 각각 0개다.

기본 source/paint와 기존 detail 네 행을 보존하고 `.navregions`에 새 네 행을 추가했다.
8개 source/paint와 area manifest를 Client 프로젝트의 기존 `96.DataFiles\\Navigation`
`None` 항목에 등록했다. 새 C++ 파일은 없다.

후보와 최신 디스크 baseline을 비교한 뒤 백업·원자 교체하고 공식
`Publish-ServerNavigation.ps1 -Mode Validate`와 `-Mode Publish -AreaId LV_LUT_MIDNIGHTC_ED`를
실행했다. 저작/프로젝트 11개 파일, runtime 26개 파일이 이 설치의 변경 범위다.
Client/Server 출력은 검증한 후보와 byte 단위로 동일하다.

## G02. Server 이동과 Client 표시

Client의 기존 `CNavigation` 이동 예측은 4m 기본 격자만 읽는다. Server의 세부 격자와
서로 다른 바닥/이동 결과를 표시하지 않도록 `GameRoom_Replication.cpp`에서 쿠크
detail region 안의 `canPredictMove`를 false로 보낸다. 기존 Server snapshot 보간을 사용하며
우클릭 이동 명령은 계속 전송한다. 다른 월드나 쿠크 기본 격자의 예측 정책은 바꾸지 않았다.
기존 마리오·카드미로 detail도 이 표시 정책에 포함되지만 해당 네비 데이터는 바꾸지 않았다.

StartFine의 마지막 jump.3은 기존 `ServerTriggerSystem` 명시적 이동으로 detail에서
기본 격자 `(2.58599997,1.023,-24.7789993)`에 착지한다. 이후 책·종이 통로와 동적
support surface는 기존 계약을 유지한다. 서로 다른 높이의 발판을 일반 보행으로 연결하지 않았다.

## G03. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 기존 ServerNavigation/ServerTriggerSystem native 소비자 | 81개 ground sample exact, 필수 보행 37개 경로 및 0.1m traversal 검사 통과 |
| 실제 단차·region 경계 | 5개 일반 보행 금지 사례 유지, 투영된 endpoint와 실제 목표를 구분해서 검사 |
| jump.3 | 실제 TriggerMove 30 tick 실행으로 detail→base 정상 착지 |
| 관문 내부 | 1·3관 입장→boss 및 반경 10m의 각 12방향 이동 통과 |
| 2관 위쪽 포커판 | 상판 1,004칸 전부 연결, 하부로 연결된 칸 0개 |
| 임시 바닥 | runtime support가 detail에 전달되는 기존 계약 통과 |
| 제외 영역 보존 | 기존 source/runtime hash와 표본 보존 확인 |
| 공식 navigation publisher | live Validate/Publish 성공, Client/Server 후보 일치 |
| C++ 최소 컴파일 | 변경한 GameRoom_Replication.cpp Debug TU 컴파일 exit 0 |
| 프로젝트 XML와 변경 줄 | Client.vcxproj/filter XML parse 및 `git diff --check` 통과 |
| 새 Debug Server의 support 계약 검사 | 이동 명령·보행·snapshot 예측 항목은 통과. 전체 묶음은 카드 폭발 2개와 추적 이동 2개 실패로 exit 1 |

검증 증거는 `out/KoukuFineNavigation20260921/validation/final_user_scope.receipt.json`,
`installation-receipt.json`, `official-validate.log`, `official-publish.log`에 있다.
geometry paint 근거와 설치 전 백업도 같은 out 폴더에 보존했다.

## G04. 통합 빌드와 남은 화면 확인

사용자가 저장 후 Client와 Server를 직접 종료한 뒤 정상 증분 Debug Product 빌드를 시작했다.
Engine·Shared·Server는 성공했고 Client 셰이더 컴파일 도중 Visual Studio의 동시 빌드를
확인했다. 같은 `Shader_VtxAnimMeshBinary.cso`를 쓰는 중복을 없애기 위해 에이전트가
시작한 fxc 프로세스만 중단했다. 첫 Product 로그의 MSB6006/-1은 이 명시적 중단 결과다.
사용자 VS 프로세스는 중단하지 않았다. 사용자가 직접 빌드 중임을 확인하고 대기를 요청했으므로
에이전트는 추가 Product 빌드를 시작하지 않는다. 현재 Client 통합 빌드 완료 판정은 보류한다.
별도 rebuild/clean은 하지 않는다.

`support-contract-test.log`의 전체 실행은 실패 4개로 끝났다. 실패 구간은 이미 room의
navigation을 detail region이 없는 `NAV_SUPPORT` 임시 fixture로 교체한 뒤이므로 게시된
세부 격자를 소비하지 않으며 새 `onKoukuDetailGrid` 조건도 false다. 이 결과를 네비게이션
검사 전체 PASS로 기록하지 않는다. 실제 새 데이터의 검증은 G03의 native 소비자 receipt다.

네 실패를 정적 추적한 결과는 다음과 같다. 관련 test와 BossSimulation/CombatObjectRuntime
파일은 이 작업에서 수정하지 않았으며 Git 기준 변경도 없다.

- `ServerGameplayContractTests_KoukuSupportSurface.cpp:562` 및 `:600`은 lifecycle event
  전체 개수를 폭발 개수로 계산한다. 현재 runtime은 카드의 `pursuit.started` pulse도
  첫 Update에서 함께 발행한다. 바로 옆 통과 항목은 explosion의 contactVisualId로 필터링한다.
- 같은 파일 `:695`의 Showtime 검사는 플레이어 X=6/10에서 추적자가 출발한다고 가정한다.
  현재 `GameRoom_BossSimulation.cpp`는 boss spawn (8,8)에서 생성한다. 반복 tick 검사의
  기대 X=6.1도 같은 초기 위치 불일치를 반복하며 중복 tick 방지 분기는 기존에 존재한다.

이번 요청과 무관한 추적/폭발 코드나 해당 테스트의 기대값은 변경하지 않았다.

Client/UI를 실행하거나 화면을 자동 조작하지 않았다. 사용자는 새 Server/Client로 재진입해
각 발판의 시각적 경계와 실제 우클릭 이동을 확인해야 한다. 셀 크기 감소와 위 수치 검증을
실제 화면 확인 완료로 기록하지 않는다. 망치·앵콜 패널은 같은 날짜의
`2026-09-21_KOUKU_HAMMER_ENCORE_LIVE_TRANSFORM_RESULT.md`에서 별도로 설명한다.
