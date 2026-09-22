# 쿠크 3관문·빙고 순서와 원본 블랙홀·메두사 연결

## G01. 현재 저장본과 관문별 순서

기준은 main `3818cc044` 위 사용자가 마지막 저장한 Composition revision 2221다. 기존 미커밋 Composition,
Kouku projection, Gameplay 게시본을 보존하고 `codex/kouku-gate3-bingo-flow-0923`에서 작업한다.
원본 백업과 작업 후보는 `out/KoukuGate3Bingo20260923`에 둔다. 라이브 Client/Server를 종료하거나
Reload하지 않는다. 최종 파일 교체 때 최신 저장본을 다시 읽고 stable ID/필드 단위로 병합한다.

`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 GATE3 Flow에 사용자 순서와 반복 시작
entry를 저장한다. 4개 Mario 입장 Parent는 각각 입장 애니메이션 뒤 지정한 전투 자식을 재생하고
해당 Mario 단계의 기존 입장/2페이즈 계약을 유지한다. 1초 추적과 일반 추적은 기존 P104/P101의
1000/2134ms 및 Server 이동 로직을 관문별 새 Pattern ID로 복제한다. GATE1/GATE2를 수정하지 않는다.

## G02. 빙고 Parent의 순차 반복

P96의 자식은 최초 24개와 반복 꼬리 14개다. 현행 Parent 펼치기는 64 Stage와 유한 시간으로
제한되어 이 순서를 한 번 펼치는 것부터 불가능하다. 선택적 `loopStartPatternOccurrenceId`를
자기 Parent의 stable 자식 occurrence ID로 저장하고 기존 Server audition member의 순서 실행을
확장한다. 최초 구간은 한 번, 마지막 실제 완료 뒤 지정 자식부터 반복한다. P96의 빙고 보드와
망치·폭탄 상태는 같은 실행의 소유권 아래 유지한다. Stop/실패/관문 종료는 기존 취소 경로를 따른다.

Client 문서 Parse/Serialize/검증, Python projector, Gameplay publisher, Server catalog와 실제
시작·완료 소비자를 함께 연결한다. Mario에는 선택적 `playChildrenSequentially`를 적용하여
입장 애니메이션, 각 자식의 카운터·중앙 복귀·완료, 기존 2페이즈 completion chain을 순서대로 소비한다.
필드가 없는 기존 Parent는 보존한다.
잘못된 자식 참조와 반복 기점은 거절하며 임의 첫 행 대체나 유한 복사로 무한 반복을 흉내내지 않는다.

## G03. 위치·방향과 원본 연출

공먹기 P119/P122의 MAP 선택 그룹과 빙고 백스텝 P114에는 GATE1 좌표가 남아 있다. P120의
알비온 MAP 배치와 중앙 teleport는 GATE3 기준이다. 해당 occurrence와 전용 logic만 대상 관문으로
이동하고 BOSS 상대 좌표, 공유 원본 Effect와 다른 관문은 보존한다. 선택 그룹 전체의 상대 배치를
유지하며 실제 runtime의 player-pivot 변환과 중복 보정하지 않는다.

빙고 Blackhole P107은 사용자 지정 `(2.9, 0, 1143)`으로 이동해 앵콜세이튼 시작 위치
`(-0.600000024, 0, 1147.43994)`를 바라본다. 사용자가 저장한 logic138에 기존
BOSS_TELEPORT_FACE_CENTER를 연결하고 빙고 Saydon의 +X 전방 basis를 사용한다. 중앙 구체·폭발은
시작 위치의 고정 MAP 이펙트로, 광선·그로기는 보스 이펙트로 분리한다. 바람방구는 기존
BOSS_TRACK_TARGET 회전 동작으로 플레이어를 향한다.

P107에 연결된 action4219983은 원본에서도 눈 레이저다. 원본 MN_RPCT_07 action4219927의
구체·빔·폭발·그로기와 연결한다. P94의 변경된 MN_RPCT_05 action42198102 클립에는 이전
action4219936 얼굴 Effect의 타이밍이 남아 있으므로 현재 source stage/notify에 맞게 재구성한다.
기존 renderer/carrier/native material을 재사용하고 P62/P123의 별도 레이저를 덮어쓰지 않는다.
메두사 판정 창은 원본 공격 notify 시간에 기존 파1빨2의 GAZE_REAL_BOSS → FEAR 로직을 연결한다.
변경된 두 패턴의 사운드를 원본에 맞추고 다른 패턴은 사용자가 편집한 사운드를 보존하며 누락만 추가한다.

## G04. 검증과 게시

후보 JSON과 실제 Client codec의 모든 대상 Pattern admission, projector의 실제 출력, Server
catalog 소비를 확인한다. 순서·반복 기점·두 번째 주기·취소·보드 상태 보존을 검사하고 변경 TU만
먼저 컴파일한다. C++ 수정은 새 바이너리 필요 상태와 Product 빌드 성공을 구분한다.
기존 publisher로 Kouku/Gameplay와 필요한 Composition 출력을 생성하고 참조 파일을 포함한다.
Client/UI 실행과 최종 Effect 화면 판정은 사용자가 직접 한다. 실제 실행한 결과만 RESULT에 기록한다.
Engine·Shared·Server·Client를 Release로 증분 빌드하고 실제 Release 계약 검증 후 기존 폴더 선택형
실행기 ZIP을 생성한다. Data는 `ChangedData/Data`에, 실행 파일·DataFiles·셰이더는 실행기의
런타임 폴더에 둔다. 기본 접속 주소는 `192.168.0.22:7777`이며 추가 리소스가 필요한 경우
`C:\Users\user\Desktop\GRResources2`의 기존 Resources 상대 경로를 유지한다.
