# 발탄·쿠크 패턴과 Movie 수정 통합 계획

## G00. 반영 범위

2026-09-29 사용자 요청에 따라 발탄·쿠크 패턴 수정, 보류했던 창술사 얼굴 트랙 복구,
현재 작업 폴더의 Movie·마리오·스킬·Release 준비 변경을 함께 검증하고 Debug/Release를
빌드한다. 기능 브랜치에서 PR을 만들고 검증한 변경을 main에 merge한다.
시작 branch는 `codex/kouku-release-sequence-ready`, HEAD는 `d6a9cc224`다.
다른 세션의 마리오 색상·목표 진행도 구현을 보존하며 같은 기능을 다시 구현하지 않는다.

## G01. 발탄과 쿠크의 실제 소비 경로

발탄의 stage 수명, arena center, 고정 공격 방향, 돌 반지름, 실패 전멸, 유령 전환은
기존 Data/Valtan 정본과 Server 소비자를 수정한다. Client는 실제 stage 종료에 맞춰
돌진 이펙트를 정리하고 유령 phase의 실제 사망에만 엔딩을 재생하며 버러지들 자막을 올린다.
상세 계획은 같은 날짜 `VALTAN_HEALTH_ROTATION_AND_ANCHOR`와
`VALTAN_EFFECT_RECEIVER_AND_PATTERN_SYNC` PLAN/RESULT에서 유지한다.

쿠크는 댄스타임의 HUD와 보스 HP 표시를 분리하고, 대형 세이튼 등장 중 캐릭터 표시와
Server 승인 이동 경로를 유지한다. 모든 일반 반복에서 저글링→나팔→슈퍼바주카 순서를
저작 정본에 반영한다. 마리오 충돌 공 피해는 Server에서 1320으로 조정한다.
기존 `KOUKU_STAGGER_ANCHOR_HOOK_LANDING` PLAN/RESULT에 해당 구현을 기록한다.

## G02. Movie 저장과 리소스

창술사는 기존 09-27 `WORLD_MOVIE_HAIR_GUARDIAN_EYES_IMPLEMENTATION` G11 후보의
두 트랙 materialName/family/parameters만 최신 디스크 저장본에 병합한다. Movie 제외 목록,
곡선과 사용자의 Effect Visible 저장은 유지한다. writer lock·hash·백업·원자 교체를 사용한다.
추가 리소스가 필요하면 Resources 상대 경로를 유지해 설치본과
`C:/Users/user/Desktop/GBResources`에 동일한 파일을 두고 hash를 대조한다.

## G03. 게시·통합·검증

각 담당자가 정본 변경과 집중 검증을 완료하면 root가 최신 입력을 다시 읽고 해당 domain의
공식 projector/publisher로 runtime 문서를 생성한다. 변경 JSON/XML parse와 diff를 확인하고,
실제 Server 계약 및 Network protocol 검사를 필요한 범위에서 실행한다. 기존 rendering
option과 사용자 numeric tuning을 임의 복원하지 않는다.

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 `Release`의
Product 빌드로 Engine/Shared/Server/Client를 순서대로 컴파일·배포한다. UI는 실행하지 않고
사용자 화면 판정과 자동 검사 결과를 구분한다. 원격 main과 차이를 검토해 통합하고,
빌드 산출물·out·개인 규칙·임시 백업은 제외한 source와 게시 데이터를 커밋한다.
PR 본문에 최종 동작과 실제 검증을 기록하고 PR을 연결한 뒤 merge 상태를 확인한다.

## G04. 완료 증거

RESULT에 각 요청의 반영 파일, 집중 검사 결과, publisher 결과, Debug/Release 증거,
리소스 대조, PR URL과 merge commit을 기록한다. 실패한 검사를 성공으로 기록하지 않으며
Client의 최종 화면과 다인 플레이는 사용자 확인 경계로 남긴다.
