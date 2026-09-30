# 쿠크 기믹 실패·제한시간·접촉 복구 구현 계획

## G00. 실측 기준과 범위

기준 HEAD는 50dcd986이며 기존 dirty 변경과 팀장 rendering 튜닝을 보존한다. Mario Parent는 입장 창 종료를 무시하고 child 종료까지 기다렸으며, 실패 처리에서 audition을 완료하여 phase2를 건너뛰었다. 서버에는 미니게임 제한시간 상태가 없었다.

## G01. 서버 입장·실패·제한시간

GameRoom_KoukuAudition.cpp의 입장 창은 Parent에도 저장한 start/duration을 적용한다. timeout 판정은 한 번만 적용하고 phase2 예약 경로를 유지한다. 솔로 입장자 사망도 phase2 재생을 막지 않는다. ServerPlayer의 절대 종료 tick을 Update_KoukuPlayerModes에서 관리하며 Mario/MAZE 입장부터 90초 후 기존 authoritative combat death를 사용한다. Mario는 해당 인원, MAZE는 인간 전원을 사망시킨다. snapshot 필드·codec·HUD 소비자는 병렬 HUD 담당자가 연결한다.

## G02. 접촉·갈고리·빙고

현재 WORLD carrier와 기존 내려치기의 collider/result를 직접 대조한다. 다섯 갈고리 묶음은 source 위치·grip를 기준으로 1m 일찍 내려놓고 실제 navigation을 검사한다. giant 등장 공과 pizza/joker 마지막 타격은 기존 authoring collider/result 경로로 연결한다. Bingo는 Showtime 폭발 sound를 WORLD 폭발 시점에 연결하고 머리 표식만 1.5배로 조절한다. 사용자가 현재 저장본의 반영과 publish를 승인했다. stable ID별 최신값 병합, byte freshness 확인, ReplaceFileW 원자교체 및 교체된 원본 백업 검증으로 반영한다.

## G03. 검증

새 C++ 파일과 프로젝트 등록은 없다. 기존 파일 인코딩·CRLF를 유지한다. 수정한 Server 계약과 JSON/projector 검증을 수행하고 통합 빌드/게시 명령은 parent에 전달한다. Client/UI 실행·화면 판정은 사용자가 담당한다.
