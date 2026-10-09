# BossToolTests 이름·Visual Studio 필터 정리 결과

## 실제 반영

- `Tools/ValtanPatternAuditionServiceHarness`의 추적 소스 17개를 `Tools/BossToolTests`로 이동했다. 프로젝트·실행 파일 이름도 `BossToolTests`로 바꾸고 진입 소스는 `Private/Main.cpp`로 바꿨다.
- `Main.cpp`의 Usage·결과 출력 이름만 변경했다. 나머지 테스트 CPP는 내용 변경 없이 이동했다. 테스트 분기·기본 실행 목록·15개 개별 옵션은 유지한다.
- `00.Start`, `01.Tests.Valtan`, `02.Tests.SharedAndKouku`, `03.Client.Services`, `04.Client.Documents`, `05.Client.Presentation`, `06.Client.Support` 필터를 만들었다. 프로젝트 항목 45 CPP·26 H·README 1개 모두 물리 파일과 필터가 대응한다.
- 프로젝트 안에서 읽을 `README.md`에 실제 역할, 파일 읽는 순서, 실행 옵션, 데이터 읽기·임시 저장 검사 범위와 검증 한계를 기록했다.
- Framework.sln, Build runner와 Python 경로 소비자 9개, CLAUDE의 현재 프로젝트 설명, 팀 발탄 인수인계서 경로, gotchas의 현재 실행명을 갱신했다. 프로젝트 GUID와 기본 솔루션 빌드 제외는 유지한다.
- Client 테스트 연결에 쓰이는 `LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS` 매크로는 유지했다. 과거 PLAN/RESULT의 당시 이름은 유지했다.

## 실행한 검증

| 검사 | 결과 |
|---|---|
| 새 프로젝트·filters XML, 항목 일치, 물리 파일 경로 | 통과: CPP 45, H 26, README 1 |
| Build runner PowerShell 구문 | 통과 |
| 경로 소비 Python 9개 AST·import·신규 경로 확인 | 통과. 전체 테스트 실행과 구분 |
| 관련 기존 Python 4개, 변경 전/후 비교 | 양쪽 모두 3 통과·1 기존 실패 |
| BossToolTests x64 Debug Build | 성공, exit 0. 기존 Shared Debug·EngineSDK Debug library 사용 |
| 새 EXE `--action-composition-graph-contract` | 11/11 통과, exit 0 |

기존 실패는 `Tools/Build/test_build_profile_contract.py`의 `test_action_presentation_harness_is_partitioned_and_physically_retired`가 `/MP /utf-8` 연속 문자열을 기대하지만 실제 프로젝트 옵션은 `/MP /bigobj /utf-8`인 점이다. 이름 변경 전 baseline에서도 같은 위치에서 실패했다. 이번 작업에서 해당 빌드 옵션이나 무관한 검증 조건은 바꾸지 않았다.

Debug 컴파일은 기존 Engine 헤더의 UTF-8 문자셋 경고 C4828을 출력했지만 완료됐다. 이름 변경으로 원본 헤더 인코딩을 바꾸지 않았다. 증거 로그는 Git 제외 `out/BossToolTestsRename20261009` 아래 `baseline-tests.log`, `after-tests.log`, `consumer-checks.log`, `debug-build.log`, `graph-smoke.log`에 있다.

## 사용자 작업과 미검증 경계

검증 도중 사용자가 Server/Public/IocpService.h, Server/Private/IocpService.cpp와 프로젝트 등록을 추가했다. 확인 시점에 헤더는 `#pragma once`만, CPP는 빈 파일이며 두 파일은 Server의 `01.Network`에 등록되어 있었다. 이 상태는 파일 생성·등록 완료이며 IOCP 구현 검증 완료가 아니다. 이름 변경 작업은 이 파일과 Server 프로젝트를 수정하지 않았다.

Engine/Public/GameInstance.h 및 Engine/Private/GameInstance.cpp의 동시 변경도 사용자/다른 작업으로 보존했다. 전체 `git diff --check`에서 GameInstance.cpp의 기존 작업 trailing whitespace가 보고되어 작업 범위 검증과 분리했다.

Client/UI, 실제 Server 연결, 기본 회귀 검사 전체와 15개 개별 옵션 전체는 실행하지 않았다. 사용자가 VS의 솔루션 변경을 다시 로드한 뒤 필터 표시를 확인한다. 기존 폴더의 Git 제외 Bin/Intermediate/user 산출물은 삭제·이동하지 않았다. 최초 이름 변경 검증 종료 당시 commit/push는 수행하지 않았다. 이후 전체 작업 전달은 [코드 학습 결과의 통합 검증 기록](2026-10-09_CODE_WALKTHROUGH_RESULT.md#2026-10-09-전체-작업-통합-검증)을 따른다.
