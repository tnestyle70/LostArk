# 쿠크 게시 좌표 정밀도와 밸런스 publisher 복구

## G01. 생성 좌표의 재현성

기준은 main `3818cc044`다. 설치된 Resources를 사용해 같은 정본을 생성하면 96개 Product는
유지되지만 P13/P33의 본 콜라이더·갈고리 좌표 14개에서 실수 끝자리가 달라진다.
Encounter의 byte 불일치가 공용 Gameplay publisher의 선행 검증을 막는다.
Resources 없는 격리 환경에서 2개만 남는 현상은 별도 입력 누락이며 이 수정으로 숨기지 않는다.

`Tools/KoukuSaydonPipeline/world_object_collider.py`의 기존 생성 경계에
`canonicalize_baked_position`을 추가한다. 검증을 마친 생성 위치만 소수점 9자리로 저장하고
음수 0은 `0.0`으로 통일한다. 축별 반올림 오차는 최대 0.0000000005m이며 기존 0.0005m
곡선 축약 오차보다 작다. Object의 center/grip은 refine/reduce 뒤에 새 배열로 정규화한다.
`project_kouku_saydon_composition.py`의 본 콜라이더는 원본 위치 범위 검사 뒤 같은 함수를 쓴다.

저작 JSON의 값, scale, yaw, 시간, ID, 곡선 샘플 선택, finite/bounds 검사, byte freshness와
원자 게시·실패 rollback 계약은 유지한다. C++/schema 변경과 새 프로젝트 항목은 없다.

## G02. 기존 회귀 검사 확장

기존 animation blend와 object collider 테스트에 실제 관측한 인접 실수로 bake를 두 번 수행하는
검사를 추가한다. 직렬화 동일성, 의미 있는 위치 변경의 구분, 음수 0, 입력 보존, 원본 위치의
비정상 값 거부와 작은 양수 scale 보존을 확인한다. 기존 stale/rollback 검사는 계속 통과해야 한다.

## G03. 정본 게시와 전달

기존 작업 폴더의 미커밋 변경을 보존하고 main 기반 별도 worktree에서 수정한다.
Resources는 설치된 물리 입력을 사용한다. 수정한 projector로 두 Product를 게시한 뒤 전체
`Publish-GameplayBalance.ps1 -Mode Publish`를 실행하고 `-Mode Validate`로 다시 검증한다.
변경 전후 stable ID·패턴 개수와 JSON 차이를 비교해 생성 위치 밖의 변화가 없는지 확인한다.
가능한 실제 Server catalog 검사와 필요한 JSON parse, `git diff --check`를 실행한다.
Python/data 변경이므로 EXE·shader 재빌드는 요구하지 않는다. Client/UI는 실행하지 않는다.
사용자의 후속 지시에 따라 코드·검증·인계만 준비하며 커밋·푸시·PR은 만들지 않는다.
Desktop에는 기존 수정과 겹치는 코드의 이번 delta만 반영한다. 격리 main의 생성물로 Desktop의
최신 쿠크 데이터를 덮어쓰지 않고, 사용자가 다른 세션 변경을 합친 뒤 그 저장본으로 재게시한다.

## G04. 후속 요청: 무력화 튜닝과 최신 통합 게시

사용자의 후속 요청으로 Gameplay publisher의 기존 `staggerDamage=10`, `partDamage=100`
고정값 정책을 Server reader와 같은 0..1,000,000 정수 범위 검사로 바꾼다. 비공격 스킬의 0과
counter capability 정책은 유지한다. 현재 밸런스 값이나 schema를 변경하지 않는다.
기존 수치 검사 테스트에서 실제 admission/writer를 실행하고, 격리 환경의 skill34010을 27/350으로
바꿔 공식 receipt 동기화와 전체 게시를 확인한다. 실제 Desktop 수치는 그대로 보존한다.

다른 세션의 최신 패턴 revision2222를 기준으로 정상 projector/publisher의 원자 게시 경로를
실행한다. 생성물 교체 전 패턴·구조·시간 보존과 위치 정밀도 변경만 있는지 검사한다.
기존 빌드를 먼저 재사용하며 실제 Server catalog admission으로 각 바이너리의 호환성을 확인한다.
조사 결과 전체 빌드가 끝난 구성은 Release이며 표준 Debug는 다른 세션의 최신 Parent 소비 코드가
없었다. 사용자 후속 요청에 따라 표준 Debug 경로에서 Engine/Shared/Server/Client의 일반
MSBuild Build를 실행한다. Clean/Rebuild와 OutDir/IntDir 변경, 셰이더 생략은 사용하지 않는다.
이번 Python/PowerShell 수정 때문이 아니라 최신 통합 C++ 소비 코드를 맞추기 위한 빌드다.

Product runner의 guard는 실행 중인 Release까지 차단한다. 실제 Release 모듈과 Debug 출력이
분리되고 공유 Resources/DataFiles 게시가 없는 것을 확인했으므로 같은 MSBuild 도구와
기존 x64/Native64Bit 설정으로 직접 Build한다. LostArkPublishRuntimeData=false를 명시하며
사용자 Client/Server를 종료하지 않는다. 완료 뒤 Debug Server headless contract로 최신 게시본을 읽는다.
