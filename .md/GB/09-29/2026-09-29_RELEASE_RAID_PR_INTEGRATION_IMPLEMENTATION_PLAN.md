# Release 레이드 검증·PR 통합·실행 ZIP 구현 계획

## G00. 기준선과 전달 범위

발탄 저장본 기준은 `e3f7ff1c5f3681e823619b05f2f172e7c85fddcb`, main은
`d944b0123c98c2ada7ad4c5b30d1ee3e7f1e15ff`다. 시작 시 working tree는 clean이며
기존 관련 편집 세션은 idle이었다. `codex/release-integration-20260929`에서 통합한다.
사용자는 열린 PR 전체 병합, 발탄 4인 Release 체력 기믹 반복·피해·콜라이더·사운드·
유령 부활부터 사망, 쿠크 입장부터 빙고 완료 검증과 기존 형식의 ZIP 생성을 요청했다.
Resources는 Drive로 공유하므로 ZIP에 포함하지 않는다. 기존 ZIP은 완성된 후보가
검증되기 전까지 유지하고, 최상위 실행 파일과 외부 Resources 선택 흐름을 보존한다.

## G01. PR과 통신 계약 통합

#475 `5bd57e2c0a94e9cdaba05880b5c772699b8b136a`,
#476 `6265d0d18874827ad7dbec4267793fd29bd5387e`,
#477 `49873bd756d7bfb68819ce01b9d1b8c16874b546`,
#478 `a3db26e8e7cbff37bc7691a01707393bbc2b61b3`를 검토·병합한다.
발탄 protocol119와 상점·마하라카의 독립 protocol118은 서로 호환되지 않으므로
통합 protocol120을 사용한다. packet ID의 순서를 보존하고 serializer/read/validation을
함께 검사한다. 생성 bootstrap 충돌은 병합된 저작 데이터와 정식 publisher로 해소한다.
새 C++ 파일은 현재 제안하지 않으며 기존 프로젝트·필터 등록의 병합 결과를 검사한다.

## G02. 레이드 소비자 검증과 누락 보완

기존 Server contract와 NetworkProtocolHarness를 사용해 게시 데이터를 읽는 실제
simulation·세션 계약을 검증한다. 발탄 체력 구간 반복, 컷씬 동기화, phase 전환,
유령 부활·사망 및 쿠크 관문·빙고·완료 경계를 확인한다. 이펙트가 있다는 이유만으로
피해를 임의 추가하지 않고 실제 damaging owner·contact·sound timing과 참조를 대조한다.
재현된 누락은 같은 기능의 기존 runtime에서 보완하고 관련 focused 회귀를 실행한다.
사용자 렌더링·패턴 튜닝은 보존한다. Client/UI 실행·육안 판정은 수행하지 않는다.

## G03. Release와 병합·배포

정식 Product Release Build를 실행하고 필요한 하네스도 Release로 빌드·검증한다.
JSON/XML parse, git diff --check, 게시 데이터와 resource reference를 확인한다.
통합본을 PR로 검토 가능하게 push하고 검증한 commit을 main에 병합한다.
각 기존 PR의 ancestry 및 GitHub 병합 상태를 확인한다. 기존 portable builder로
`C:/Users/user/Desktop/LostArk-Release-20260929.zip`을 생성·검사한다. 기존 20260923 ZIP은 보존한다.
실행한 자동 검사와 사용자 화면 미검증을 RESULT에서 분리한다.

## G02-1. 네 세션 발탄 연속 회귀

기존 `ServerGameplayContractTests_ValtanLifecycle.cpp`의 `Run_ValtanLifecycle`에
실제 게시 Catalog/World/Navigation과 네 `CClientSession`을 사용하는 연속 사례를 추가한다.
네 클래스의 실제 Join과 typed G 입장 후 컷씬·등장 휠윈드, 각 체력 구간의 ordered loop
한 순환과 재시작, 130/115/105/80/65/30/15 기믹, Death→Respawn의 40줄 회복,
유령 반복·최종 실제 플레이어 스킬 처치와 네 세션의 동일 snapshot/clear 전달을 확인한다.
검사 대상은 진행·복제 계약이며 장시간 관찰을 위한 플레이어 무적과 체력 문턱 이동은
테스트 fixture에서만 설정한다. 보스 stage·시간·순서·분기·재생 데이터는 변경하지 않는다.
독립 화면·네트워크 PC·실제 음성 청취 성공으로 기록하지 않는다. 기존 CPP 등록을 재사용하며
새 제품 파일이나 별도 검증 프레임워크는 없다. Release 통합 빌드 후 기존
`--valtan-lifecycle-contract-test`에서 실행한다.

## G04. Release Server 권위 수치 저장과 네 세션 반영

F1 Balance의 숫자는 Server가 보낸 stable domain/ID/field와 numeric revision을 기준으로
편집한다. 네 세션 중 어느 사용자의 저장도 typed patch로 Server에 제출하고, Server는
기존 bootstrap 수치 열만 변경한 후보를 기존 Catalog parser로 전체 검증한다. 저작 원본과
게시 데이터의 최신 저장본을 CAS로 확인·백업·원자 교체한 후 모든 방이 같은 tick에서
새 수치를 소비한다. 이전 gameplay/presentation revision과 실행 중 패턴 순서·시간·기믹
ledger를 보존하고 별도 numeric revision으로 충돌을 검출한다. 플레이어·보스 HP와
자원은 기존 비율, 사망은 0을 유지하며 이후 판정에는 새 피해·무력화 수치를 사용한다.
무력화는 미소비 boss 메타데이터 대신 실제 SET_STAGGER_GAUGE 값을 노출한다.
새 ServerBalanceNumericStore H/CPP와 필요한 focused 테스트는 Server 프로젝트와
filters에 등록하고 Release 컴파일, 잘못된/오래된 patch 거부, 저장 실패 보존,
네 세션의 동일 적용과 ghost HP profile 보존을 기존 contract runner에서 검증한다.
