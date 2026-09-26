# 쿠크 플레이테스트 복구와 최신 저장본 게시

## G01. 현재 정본 보존과 최종 게시

사용자가 지정한 BackupData에 현재 게시 패턴·시퀀스·렌더링과 대응 저작 입력을
복사하고 해시를 검증한다. 수정은 최신 디스크의 stable ID 및 요청 필드만 병합한다.
Action/Sequence 저장 revision과 Gameplay/RAIDGATE 게시 revision을 최종 게시에서
일치시킨다. revision.match 검사는 유지한다. 실행 중 Server가 이전 catalog를 쓰는
상태와 디스크 게시 완료를 구분하고 사용자 프로세스는 자동 종료하지 않는다.

## G02. 원작 엔딩의 개별 애니메이션과 타임라인 직접 편집

원작 SCENE01B로 다시 생성한 네 배우의 전체 애니메이션을 설치 원본과 대조한다.
원래 25개 구간을 각각 독립 native clip으로 생성하고 현재 Character geometry,
재질, 골격, 전투 클립을 보존한다. 실패한 saydon1 Duplicate로 늘어난 시간은 원작으로
되돌린다. 이미 맞는 카메라·음향·자막·다른 패턴 및 렌더링 설정은 바꾸지 않는다.
시퀀서 Animation 행은 현재 읽기 전용 정보 행이므로 직접 이동·양끝 trim과 저장을
기존 WORLD 저작 document/preview/save 경계에 연결한다. 다른 창을 열어 편집하도록
넘기는 방식으로 완료하지 않는다. 새 입력은 실제 저장·재로드·샘플링까지 검증한다.

## G03. 사용자 이펙트와 패턴 편집 반영

사용자가 저장한 bingo hammer original chevron의 element 한 개/scale6을 기준으로
빙고 해머 바닥과 카드미로 전조의 occurrence 배율 및 배치를 대조해 왜곡을 제거한다.
파1빨2의 늘어난 빨강/전조 생성 시각은 유지하고 이전 파랑 Duration은 실제 무효 또는
오동작 여부를 확인해 필요한 경우에만 제거한다. Gate2 반복에서 불어날리기 직전16번째에
대형세이튼 잡기를 연결하고 사용자가 추가한 rectangle damage collider를 소비자까지
검증한다. 피자의 대형세이튼 회전만 원인을 찾아 교정하며 쿠크의 사용자90도는 유지한다.
분신 십자화염은 왼쪽이 실제 보스, 나머지 세 방향이 분신이 되도록 parent 연결을 고친다.
대형세이튼 불뿜기의 collider와 tick을 세이튼 불뿜기와 맞춘다.

## G04. Server 권위 실패 복구

아이언 메이든에 갇힌 player가 즉사 칼날에 살 수 있는 실제 경로·접촉·수명 원인을
고친다. Mario 변신 상태도 기존 player entity로 갈고리 포획이 가능하게 한다.
Gate2 의자 착지에서 낙사 누락을 재현해 실제 높이/충돌 규칙을 수정한다.
쿠크·발탄 처치 후 방을 떠나 재입장하면 새 raid가 초기 상태로 시작하게 하며
현재 남아 있는 참가자의 raid를 중간에 초기화하지 않는다. 필요한 Data/Shared/Server
경계는 기존 typed runtime을 확장하고 Client 판정으로 우회하지 않는다.

## G05. 편집 도구와 호버 제거

빙고 블랙홀 시전자 광선의 groggy 및 medusa Effect가 기존 Open Editor/Play All로
열리고 전체 구성을 재생하도록 실제 resource kind·variant·closure 연결을 확인한다.
combat hover 윤곽선의 매 프레임 picking/상태 부여 경로와 호출을 제거한다. hit flash,
피해 판정, UI hover 및 클릭 이동/공격 picking은 보존한다. 추가 요청한 아이언 메이든의
주사위 카드 속박 이펙트만 제거하고 실제 카드 맞추기 기믹의 연출은 유지한다.

## G06. 검증과 완료 경계

수정별 실제 consumer 중심의 집중 검사와 필요한 TU compile을 먼저 수행한다.
최종 최신 저장본을 병합한 뒤 필요한 domain만 공식 publish하고 source/runtime join,
Action/Sequence revision, JSON/XML parse, git diff --check를 확인한다. Debug/Release
일반 Product Build를 사용하고 실행 파일이 점유된 경우 사용자에게 실제 점유를 알린다.
Client/arena GUI는 사용자가 확인하며 source/CPU/게시/빌드 성공을 화면 성공으로
대신 기록하지 않는다. 구현 범위에 새 C++ 파일이 필요하면 프로젝트·필터를 함께 등록한다.
