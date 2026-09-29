# Release 입장·무비 사운드·4인 레이드 배포 교정

## G00. 현재 실패와 작업 범위

기준은 `codex/release-regression-20260929`, `50dcd986d`다. 기존 미커밋 protocol 검사와
Bahuntur fixture 교정, 다른 세션의 문서 작업을 보존한다. 같은 작업 폴더에서 정상 증분 Build를
사용하고 shader 입력·CSO의 bytes와 수정 시각을 작업 전후 대조한다.

실제 Client PID13164는 쿠크 Server 승인을 받은 뒤 필수 Effect 준비에서 실패했다.
파괴폭탄·회오리 수류탄 비행 Effect의 element 표시 이름 17개가 UTF-8 64-byte 계약을
넘는다. 생성기와 두 문서의 표시 이름만 교정하고 stable ID·재질·수명은 보존한다.
이 실패를 ZIP 단계에서 검출하도록 기존 source validator와 package builder를 연결한다.
이후 `WM_QUIT`는 별도 관찰이며 자동 crash로 단정하지 않는다. Lobby 복귀의 saved-card
clone 실패는 shared prototype·animated shader·per-level readiness를 조사해 교정한다.

## G01. 기존 Movie 시계의 오디오 분리

상세 정본은 `../09-26/2026-09-26_WORLD_MOVIE_EFFECT_EDITOR_IMPLEMENTATION_PLAN.md`의
후속 G19다. 카메라·애니메이션·Effect의 source clock을 유지하고 오디오 cursor와 drift 교정은
감속 전 Movie 시간으로 계산한다. 사운드 시작점, 사용자 source trim·volume, 수동 배속,
Pause/Seek/Replay/완료 정지는 보존한다. 새로운 오디오 런타임이나 셰이더 변경은 없다.

## G02. 4인 진행과 배틀 아이템 검사

기존 `out/ReleaseValidation20260929` 결과에서 실제 실행된 assertion과 Debug 전용 미실행
구간을 분리한다. 현재 게시본을 격리 복사해 Kouku 관문·Bingo·Mario·발탄 lifecycle·입장과
복귀·아이템 지급/사용을 검사한다. 서버 정본 수치와 패턴 타이밍을 테스트 통과를 위해 바꾸지
않는다. 오래된 fixture는 실제 소비 계약을 확인한 뒤 기대값·시간 범위만 교정한다.
Release F1의 typed 지급, authoritative inventory, 사용·쿨타임·소모·4인 broadcast를 연결해
확인한다. 실행 중 서버나 Client UI는 조작하지 않는다. headless 결과를 실제4PC 화면 성공으로
기록하지 않는다.

## G03. Release와 ZIP

필요한 C++ 수정 뒤 표준 Release Product Build를 사용한다. 변경 JSON parse와 diff-check,
Effect admission, 기존 package tests, source/runtime 참조와 protocol·generation을 검사한다.
기존 ZIP을 백업하고 새 stage에서 portable ZIP을 만들어 CRC·manifest SHA256·nonlaunch
preflight를 검증한다. Resources는 기존 외부 폴더를 사용한다. 신규 C++ 파일을 추가할 경우에만
project/filter 등록이 필요하며 현재 계획은 기존 파일을 확장한다.

## G04. 필수 발탄 부위 파괴 재질 경로

실제 codec 전수 검사에서 420628 필수 문서가 기존 Character/SourceMaterials DDS 경로로
거절됨을 추가 확인했다. 원본과 해시가 다른 동명 Effect DDS로 치환하지 않는다. 기존
Resources 상대 경로 검증에 Character/SourceMaterials의 DDS만 허용하고, 해당 namespace의
모델·절대경로·상위탈출·역슬래시·미존재 파일은 계속 거절한다. 설치 리소스는 재사용한다.

## G05. Debug Balance Test 수치 조회 복구

09-30 사용자 화면에서는 전투 HP/tick은 갱신되지만 공용 수치 목록이 비어 있다. 실행 중
Client의 읽기 전용 상태 확인과 실제 Debug Shared.lib 링크 검사에서 수치 조회 payload는
생성되지만 Build_Packet_Frame이 새 packet type을 거부하는 것을 재현했다. 현행 소스와
Release 라이브러리는 통과하므로 저장된 무력화 값이나 패턴 데이터는 변경하지 않는다.

현재 도구 구성과 표준 중간 경로에서 Shared Debug를 정상 증분 Build하고 같은 링크 검사를
재실행한다. 기존 Client/Server가 사용 중인 EXE 교체는 사용자 종료 후 수행하며 자율 종료하지
않는다. 이후 Debug Product Build와 수치 조회/저장 관련 검사를 실행하고, 실제 F1 수치 표시와
무력화 플레이 판정은 사용자가 확인한다. Build가 이전 객체를 계속 재사용하면 추적 입력과
정의 심볼을 조사하며 Clean/Rebuild, tlog 삭제, timestamp 조작으로 우회하지 않는다.
