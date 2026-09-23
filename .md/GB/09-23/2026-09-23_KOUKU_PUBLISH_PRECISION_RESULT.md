# 쿠크 게시 좌표 정밀도와 밸런스 publisher 복구 결과

## G01. 실제 원인과 수정

main `3818cc044`의 원본과 실제 설치 Resources로 96개 Product를 재생성하면 기존 Encounter의
P13/P33 본 콜라이더·갈고리 위치 14개에서 약 1e-16m의 부동소수점 차이가 발생했다.
생성 JSON의 byte 비교가 이를 stale로 판정해 Gameplay publisher의 선행 검증을 막았다.
stale은 저장된 생성물이 현재 입력으로 생성한 결과와 다르다는 뜻이며, 원본 패턴 삭제를 뜻하지 않는다.

`world_object_collider.py`와 `project_kouku_saydon_composition.py`에서 검증된 생성 위치만
소수점 9자리로 정규화했다. Object refine/reduce, raw finite/bounds 검증, scale/yaw/시간,
stable ID, 실제 freshness 검사와 원자 게시·rollback은 유지했다. C++와 schema를 바꾸지 않았다.

Resources 없는 격리 환경의 2개 Product는 별도 문제였다. MN_RPCT_05, MN_RPCT_06,
MN_RPCZ_00 모델 입력이 없어 94개 패턴이 unavailable이 됐다. 설치 Resources를 연결하면
main의 96개가 유지된다. 원본이 2개만 선언했다는 이전 보고는 이 환경 차이를 구분하지 못했다.

## G02. 무력화와 부위 파괴 튜닝

Gameplay publisher에는 공격 스킬의 `staggerDamage=10`, `partDamage=100` 고정값 검사가
남아 있었다. Server reader가 허용하는 정수 0..1,000,000 범위로 맞췄다. 비공격 스킬의 두 값은
계속 0이어야 하며 counter capability, 직렬화 열, bootstrap schema34는 유지했다.
Desktop의 실제 PlayerSkills 수치와 공식 provenance receipt는 수정하지 않았다.

격리 worktree에서만 skill34010을 무력화27·부위 파괴350으로 바꾸고 기존 receipt 동기화 도구를
실행했다. 정확히 두 field가 PROJECT_TUNED로 바뀌었고 전체 Gameplay Publish가 성공했다.
생성물의 실제 행은 `SKILLCOMBATTRAITS\t34010\t27\t350\t0`, header는 schema34였다.
이 격리 생성물을 Desktop의 최신 쿠크 게시본으로 복사하지 않았다.
기존 Debug Server의 bundle contract도 이 격리 게시본의 실제 catalog를 읽었으며 94개 검사,
failures0, exit0을 확인했다. 원본 PlayerSkills와 receipt의 SHA-256은 그대로다.

## G03. 최신 다른 세션 패턴과 실제 게시 결과

Desktop의 Composition revision2222를 사용했다. 저작119개·실행113개를 유지했으며 이전
main의 96개 게시본으로 덮어쓰지 않았다. 생성물 교체 전에 구조·stable ID·시간을 비교했고,
189,054개 위치 좌표의 반올림 외에는 변화가 없었다. 최대 변화는 5.0000004137e-10m였다.
공식 projector의 원자 게시 경로와 전체 `Publish-GameplayBalance.ps1 -Mode Publish`가 성공했다.

게시 결과는 실제 Client가 사용하는 Encounter/patternbindings JSON과 Server가 사용하는
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`이다. 밸런스 작업자는 생성물을 직접
편집하지 않고 `Data/Balance` 원본을 튜닝한 뒤 receipt 동기화와 publisher를 실행한다.
게시 파일 생성과 실행 중 Server의 적용은 별도이며 일반 밸런스 적용에는 Server 재시작이 필요하다.

## G04. 검증 증거와 빌드 상태

| 검증 | 결과 |
|---|---|
| animation blend/object collider 기존·추가 검사 | 31개 통과 |
| 기존 deterministic/stale/transaction rollback 검사 | 3개 통과 |
| WORLD 숫자·charge yaw·skill trait 검사 | 9 + 16 + 29개 통과 |
| main 기준96개 전체 Gameplay Publish | 성공 |
| 격리 skill34010의27/350 전체 Gameplay Publish | 성공 |
| 격리27/350의 실제 Server catalog/bundle contract | 94개 통과, failures0 |
| Desktop 최신113개 전체 Gameplay Publish | 성공 |
| 기존 Release Server로 최신 게시본 raid contract 실행 | 904개 통과, failures0 |
| 기존 Debug Server로 최신 게시본 catalog 실행 | Unknown gameplay bootstrap row kind로 실패 |
| 표준 Debug 증분 Build | Engine/Shared/Server/Client 모두 exit0, 합계 약92.6초 |
| Debug Product의 빌드 후 기존 런타임 입력 검사 | SkipBuild=True, exit0 |
| 새 Debug Server로 최신 게시본 raid contract 실행 | 1,178개 통과, failures0, exit0, 약102초 |
| 최신 생성 JSON3개·Client 프로젝트 XML2개 parse / git diff --check | 통과 |

Debug 실패는 최신 다른 세션의 PATTERNPARENTCHILD 소비 코드가 기존 Debug EXE에 없기 때문이다.
다른 세션의 `out/KoukuGate3Bingo20260923/final-build-evidence.json`은 전체 Release 빌드다.
그 RESULT의 Debug/Release 모두 통과 항목은 이펙트 수치 검사이며 전체 Debug 빌드 기록이 아니다.
Python 정밀도 수정 때문에 C++ 재빌드가 필요한 것은 아니다.

Debug Product 증분 빌드를 시도했으나 실행 중인 Release Client/Server도 일괄 차단하는
ProductOutputGuard에서 컴파일 전에 중단됐다. 후속 확인에서 실행 중인 모든 게임 모듈은
Release 경로임을 확인했다. 표준 Debug 출력은 별개이고
LostArkPublishRuntimeData=false이면 공유 Resources/DataFiles를 게시하지 않는다. 같은 도구와
표준 경로에서 일반 MSBuild Build를 실행해 네 프로젝트 모두 성공했다. Engine0.66초,
Shared0.38초, Server27.48초, Client64.05초였다. Clean/Rebuild와 경로·셰이더 설정 변경은 없었다.
실제 FXC 실행은 Engine16개 중0개, Client174개 중0개로 기존 셰이더를 재사용했다.
Client에는 C4819/C4828 등 기존 소스 인코딩 경고가 남아 있으며 경고2380개·오류0개였다.
이 작업에서 C++ 인코딩을 변경하지 않았다. 사용자 프로세스 종료와 Client/UI 조작도 하지 않았다.

빌드 후 공식 Product runner의 SkipBuild 검증이 exit0으로 끝났다. 이는 파일 존재·Navigation
참조·Item/Valtan reward 게시 내용 검사이며, Gameplay/쿠크 실행 검사는 별도로 기록한다.
첫 Debug raid contract 실행은 60초 제한에 도달해 완료 전에 종료됐으므로 성공으로 세지 않았다.
240초 제한으로 동일한 검사를 다시 실행해 약102초 만에 1,178개 통과·failures0·exit0을 확인했다.
실제 최신 catalog 로드도 통과했다. 검사한 Gameplay.bootstrap의 SHA-256은
`d714fb327ebe8efff8e6f32053a77d6723dba90310aa2094efc3fe6985e4207b`다.
새 Debug Server EXE의 SHA-256은
`36a8445eea8839189c990500dd81f73de4cc70234f865f9c1bb3dde2d08f33f4`다.

검증 로그와 비교 결과는 `out/KoukuPublishPrecision20260923/`에 있다. 주요 파일은
`current-projection-publish.json`, `current-gameplay-publish.log`, `current-release-raid.log`,
`current-debug-raid-evidence.json`, `current-debug-raid-complete.log`이다.
`current-debug-catalog.log`는 수정 전 Debug 바이너리의 실패 증거로 보존했다.
수치 튜닝 증거는 같은 폴더의 `stagger-tuning-evidence/`에,
Debug 빌드 증거는 `debug-build/`에 있다.

## G05. 전달 범위

정밀도 코드2개, 대응 Python 테스트2개, Gameplay publisher와 기존 숫자 테스트, 팀 밸런스 계약,
gotchas 및 이 주제 PLAN/RESULT/HANDOFF를 변경했다. 다른 세션의 C++·패턴 변경은 보존했다.
최신 원본·소비 코드·생성물을 같은 통합 변경에 포함해야 하며 자세한 작업자 절차는
`2026-09-23_KOUKU_PUBLISH_PRECISION_HANDOFF.md`에 있다.

사용자 요청에 따라 커밋·푸시·PR은 만들지 않았다. 게임 화면과 실제 전투 중 무력화 판정의
최종 확인은 사용자가 수행한다. 자동 게시·Server catalog/contract 성공을 화면 확인으로 기록하지 않는다.
