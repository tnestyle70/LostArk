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

### G02-1. 후속: WORLD Animation의 배우별 행과 편집 거절 사유

실제 저장본에서 saydon1의 .04는12967~14703ms이고 .05는16333ms에 시작하여 공백1630ms가
있다. 반면 선택된 kouku1의 .02는15700ms에 시작하고 앞 .01도15700ms에 끝나므로 공백이 없다.
기존 표시가 시간 구간으로 빈 행을 채우면서 서로 다른 배우를 같은 행에 배치한 것이 혼동 원인이다.

Kouku Workbench의 WORLD Animation 표시 행을 World occurrence와 slot의 조합에 고정하고
label과 tooltip에 배우를 표시한다. 같은 화면 행의 다른 배우를 이전 clip으로 오인하지 않게 하되
기존 stable clip 선택과 이동·trim 호출을 유지한다. WorldObjectTool은 native 구간, 같은 slot의
이웃 clip 및 Motion 끝 검증을 유지하면서 이웃 충돌과 Motion 범위 초과를 구분해 안내한다.

다른 clip을 밀거나 source 구간·기존 고정 시계를 자동 이동하지 않는다. 사용자가 편집 중인
Sequence Composition과 WorldSequences JSON을 수정·Reload·publish하지 않는다.
검사는 서로 다른 배우의 표시 행 분리, 같은 배우의 실제 공백과 연속 경계, 허용 이동·trim 및
이웃/Motion 경계 거절 시 기존 draft 보존에 집중한다. 이전 G02의 PASS를 후속 수정의 검증으로
대신하지 않으며 실제 컴파일·실행 결과와 사용자 화면 확인은 RESULT에 별도로 기록한다.

### G02-2. 최종엔딩 전체의 원본 시간·배우·재질 복원

사용자가 범위를 `빙고_최종엔딩씬` 전체로 확정했다. 별도 `앵콜컷신`은 이번 변경 대상이 아니다.
SCENE01B Matinee32/Data45의 활성 트랙을 기준으로 시작부터 종료까지 연결한다.
기존 변환에서 빠진 Slomo191을 `source_scene_clock.py`의 단일 적분으로 계산한다.
원본49.083336초는 실제52.316502초이며, 카메라·배우·가시성·자막은 같은 실제 시간으로
변환한다. 오디오는 원본 cue 시작을 변환하되 기존 WAV를 임의로 늘이거나 pitch를 바꾸지 않는다.
공유 Effect asset은 유지하고 해당 occurrence의 비선형 source time 키를 기존 V1/V2 외부 샘플링에
연결한다. 저장·로드·게시와 편집 복사에서 이 키를 보존하며 잘못된 키는 기존 draft를 보존하고 거절한다.

`build_bingo_ending_actors.py`는 source constant 전환의 첫 도착 ms와 빠른 본 부착 궤적의
중간 표본을 보존한다. 현재 source 기반 위치에서 확인된17ms 가짜 보간을 제거하고, 네 배우의
전체·분할 animation만 새 시간축 후보로 만든다. 원래 mesh·재질·골격·전투 animation은 유지한다.
누락된 원본 무기 component251/247를 실제 부모 본 궤적으로 연결한다. 모자252/246은 기존 native 부착을 유지하며, component251의 `dead`는 독립 무기 actor33이
아니므로 그 재질에 잘못 적용하지 않는다. World template의 선택적 materialTracks가 기존
SourceCharacter parameter packer와 실제 World clone에 값을 전달하도록 한다.

후보는 out에서 원본 대비 수치·실제 소비자 검사를 끝낸다. 현재 사용자 저장본의 stable ID별
최종엔딩 필드만 병합하는 후보를 준비하고, 최종 반영 시 저장 여부를 확인한 뒤 최신 디스크를
다시 읽고 hash 확인·백업·원자 교체한다. 다른 패턴·렌더링 튜닝·별도 앵콜컷신은 보존한다.
일반 Product Build와 해당 입력·저장·재생 소비 검증을 수행하며 Client 화면 판정은 사용자 확인으로 남긴다.

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

## G07. 예고 방향·컷신 가시성·Complete Play 연출 로드

세토와 빙고 망치의 실제 이동 곡선과 같은 warning Effect의 native sprite 축을
대조한다. 반전된 occurrence의 회전만 교정하며 모델 이동·피해 범위·공유 Effect의
사용자 크기는 보존한다. 최신 WORLD 저장본의 stable ID 필드로 병합·게시한다.

Level_KakulSaydonArena가 활성 컷신을 기준으로 모든 replicated player의 표시 억제를
결정한다. 기본은 숨김이고1관문 입장·카드미로·3관문 입장만 표시한다. Character의
기존 서버 가시성 상태와 별도 조건으로 합성하여 컷신 종료·취소 때 원래 표시로 돌아간다.

22:00:25 실행 로그의 Kouku.product.load 오류는 비어 있는 effectSourceTimeKeys6개로
재현된다. 추적 폭탄 정규화가 만든 빈 키는 투영에서 생략하고 native reader는 저작
codec과 같은 미사용 의미로 읽는다. 비배열·1개 키·범위·순서 오류는 계속 거절한다.
전체 presentation parser 검증을 준비 단계에 연결해 정상 리소스 준비만으로 READY를
보고하지 않는다. 현재 게시본 전체와 잘못된 clock 입력을 실제 reader로 검증한다.

기존 파일만 수정하므로 신규 프로젝트·필터 등록은 필요 없다. 변경 Python 검사,
JSON parse, 일반 Debug/Release Product Build와 git diff --check를 수행한다. 사용자
Client/UI를 실행하지 않으며 실제3관문 Complete Play와 화면 방향은 수동 확인으로 남긴다.

## G08. 쇼타임 세 번 폭발과 메두사 활성 시야 판정

사용자가 추가 요청한 쇼타임 노란 바닥 뒤 세 번 폭발의 실제 Effect occurrence
시각·크기를 실측한다. 같은 세 구간에 기존 Server 권위 collider·대미지·접촉
중심 넉백을 연결하며 Effect/음향과 사용자 저작 시간을 바꾸지 않는다. 수치는
같은 패턴의 기존 피해/넉백 정책을 먼저 확인해 재사용한다.

메두사 P94는 진짜 세이튼과 동일한 GAZE_REAL_BOSS의 플레이어 정면±45°/30m를
사용하지만 기존500ms 창 끝에 한 번만 판정한다. 메두사 얼굴 이펙트가 저작된
3167~6428ms에 맞춰 전용 DURATION 정의를 만들고 optional gazeDuringWindow를
켠다. 기존 공유 logic36과 진짜세이튼찾기의 종료판정은 보존한다. 새 옵션은
authoring codec/Save/Tool, publisher/bootstrap, Server reader/window/runtime를
모두 연결한다. 활성 구간에 바라본 플레이어에게3초 공포를 한 번 적용하며
구간 종료 뒤 새 판정을 하지 않는다. 중간 방향 변경과 중복 적용·45° 경계 및
기존 종료판정의 회귀를 실제 Server 함수로 확인한다.

최신 Action 저장본에 메두사와 쇼타임의 stable ID 변경만 병합하고 공식 Kouku
projector→Gameplay→Composition 및 WORLD publisher를 실행한다. 최종 Debug와
Release Product Build를 모두 완료하고 해당 게시본을 Server 계약 검사로 읽는다.
4Client 실행/화면은 사용자 검증 범위이며 실제 endpoint 수신 여부를 별도 보고한다.


## G09. 09-27 저장본 게시 실패와 후속 플레이테스트 복구

사용자가 모든 편집을 저장하고 실행 파일을 종료한 상태를 기준으로 한다. Action2441,
Sequence182, WORLD2282와 기존 게시2421을 별도 백업했다. 실제 UI 게시 로그의
world.gameplay trackBombs schema 거절을 수정하고 공식 Kouku owner 게시 경로로 검증한다.
사용자가 설명한 flow와 저장본을 대조하여 돌진카운터 뒤1초 추적,2관문 나팔 제거와
거미카운터 뒤 팡파레,3관문 십자화염 제거를 stable entry/group 계약에 맞춰 반영한다.
음향·이펙트 위치·칼날 삭제·회전·넉백·포탈 연장 등 다른 저장 필드는 보존한다.

조커 랜덤 표적 DURATION을 기존 typed mechanic row로 게시한다. Server가 표적을
선정하고 Client는 저작 구간 동안 표식과 노란 시선만 추적한다. 구간 중 본체 회전은 바꾸지
않는다. 쇼타임은 정상 Append occurrence와 지면 높이를 대조해 바닥 아래에 저장된
고정 그룹의 Y만 수정한다. 사용자의 후속 지시에 따라 첫 프레임·시간 샘플링 경로는
변경하지 않고, Server가 지면 위치를 전달하는 상대 좌표 템플릿도 보존한다.

마리오는 Server가 지정 색과 표시 대상을 선정한다.1인은 진입자,2~4인은 바깥
참가자 중 한 명에게 색 표식을 복제하며 지정 색 공3개를 파괴해야 최종 이동과
직접 복귀가 허용된다. 표식은 진입자의 실제 복귀·사망·취소까지 유지한다. 바깥
아이언 메이든은 전체1~2인일 때0명,3~4인일 때1명이고 표식 대상과 겹칠 수 있다.
기존 파괴 비트와 이동 승인 경로를 확장하고 marker snapshot/Client 소비자를 연결한다.
원본 표식 asset을 확인해 필요한 Resources closure만 설치·GBResources에 전달한다.

빙고 망치의 설치 WModel 정점과 WORLD 배율에서 지면 충돌 크기를 산출하고 기존
collider track debug와 Server BINGO_BOARD 게시 행이 같은 값을 사용하게 한다.
잡기 피해는 같은 보스의 같은 패턴에 붙잡힌 대상에게 저작된 피해 창이 적용되도록
허용한다. 일반 GRABBED 면역과 무관한 타격은 유지한다.

카드미로 입장 roster에 사망자도 포함하여 중앙 시작점까지 transactionally 이동하되
HP0/DEAD/전투 불가를 유지한다. 사망자는 추적자 선정 대상이 되지 않는다. 카드미로와
댄스 HUD 모드에서는 전투·보스·게이지·미니맵 이미지 및 대응 텍스트를 숨기고 모드가
끝나면 기존 갱신으로 복구한다. 기믹 입력·월드 표식·사망 복귀창은 유지한다.

각 담당은 해당 실제 Server 함수·저장 codec·게시 소비자 중심 검사만 추가한다. 최신
디스크 hash 재확인, stable ID 병합, 백업·원자 교체 후 공식 publish와 Debug/Release
일반 Product Build를 순서대로 완료한다. 사용자 Client 실행과4인 화면 검증은 별도다.
기존 C++ 파일을 확장하므로 현재 신규 프로젝트 등록 대상은 없다.


## G10. 주사위 회귀의 실제 원인과1~4인 판정 복구

사용자가 확정한 주사위 계약은1인 자유1/속박0,2~4인 자유1/속박N−1이다. 속박
플레이어도 카드 충돌 대상이며, 자기 문양과 같은 카드는 피해 없이 속박을 해제하고
다른 문양 카드는 최대HP90% 피해를 준다. 자유 플레이어가 카드를 유인하는 기존
카드 이동 계약을 사용한다. 속박·해제 이펙트는 Server 결과에 대응한다.

추가 원작 이미지의 밝은 원형 경계는 현재 속박 floor와 빙고 망치 생성의 실제 source
element를 대조한다. 중앙 문양과 외곽 링을 구분하고, 동일 source인지 확인하기 전에
망치 Effect 전체를 주사위에 붙이지 않는다. 누락이 확인된 요소만 기존 carrier에 연결한다.

기존 G03은 아이언메이든의 불필요한 주사위 이펙트를 분리한 변경이고, G07의 전체
Product load 오류 수정은 주사위4인 전원 고정의 원인을 증명하지 않는다. 현재
CARD_DICE_BIND 창의 자유 대상 선정, 기존 bound 소유·만료, 실제 카드 접촉 허용,
최대HP 피해 계산과 Client diceBindVisual를1~4인 실제 함수로 대조한다. 실패한
경로를 확인한 뒤 해당 경계만 수정하고 원인을 입증한 검사와 화면 미검증을 구분한다.
최종 게시·Debug/Release 빌드를 새 변경까지 다시 완료한다.

## G11. 조커 Duration 종료 시 조준점 고정

사용자가 저장한 세 Duration 시작·종료 시각은 변경하지 않는다. 시작 시 선택한 표적을
구간 중 유지하고, 정확한 종료 tick에 그 플레이어의 현재 world position을 한 번 저장한다.
이 좌표를 조준점으로 삼아 기존 망치의 궤적과 사거리로 공격한다. 종료 이후 표적 이동이나
일반 stage target 갱신이 확정한 방향을 덮지 않도록 하고, 다음 패턴에 이전 조준이 남지
않도록 소유 상태와 yaw를 정리한다. 망치를 손에서 떼어 목표 XYZ로 평행 이동하거나
본체를 순간 이동시켜 공격 중심을 맞추는 변경은 하지 않는다.

Server의 기존 target position과 boss yaw snapshot 경계를 우선 사용한다. Client의 노란
시선은 기존 Duration 동안만 추적하며 실제 망치 mesh·Effect·Collider가 같은 고정 yaw를
소비하는지 확인한다. 종료 직전 이동·정확한 종료·종료 후 이동·다음 window/패턴 전환을
집중 검사하고 마지막 변경까지 Debug/Release 일반 빌드를 다시 완료한다.

## G12. 카운터·무력화 성공의 폰트 표시

Server가 확정한 성공만 기존 DAMAGE_EVENT와 CombatHUDViewModel을 통해 전달한다.
카운터는 기존 isCounterSuccess, 무력화는 일반 stagger gauge와 쿠크 STAGGER_WINDOW의
성공 edge를 사용한다. HP 피해0이어도 성공을 유지하고 파란색 카운터·노란색 무력화를
기존 Font_EventDamage로 해당 보스 위치 위에 표시한다. 무력화 애니메이션 진입으로
성공을 추측하지 않으며 피해량·DPS에 가짜 피해를 더하지 않는다. wire 확장은 protocol115로
묶고 writer/reader 검증과 기존 회귀 하네스로 성공·실패 packet을 확인한다.

## G13. 문양 출생 위치·마리오 표식·중앙 포탈 앵커

현재 P47 presentation.2/.3은 MAP 절대좌표이고 활성 SELECT 장판 Logic은 없다.
두 occurrence를 bone 없는 BOSS/followBoss=false로 바꿔 시작 시점의 보스 root 위치를
기록한 뒤 world에 고정한다. 기존 크기·시간·source in은 보존한다. 마리오 머리 표식은
Server의 표시 대상·색을 유지하면서 세 색 모두 준비하고 actor 위치에 고정 높이를 더해
본 애니메이션을 배제한다. 사용자가 추가 요청한 P88/P91/P92/P93의 진입 포탈은 각3개,
총12개를 같은 패턴 presentation.2의 큰 중앙 오망성 좌표로 맞추고 MAP/followBoss=false로
저장한다. latest disk field 병합·직전 hash 확인·백업·원자 교체를 유지한다.

## G14. 알비온 타겟과 마리오 청취 범위

공용 boss random target과 Kouku의 기존 선택·추적 대상 재조회에서 iMarioStage가 있는
참가자를 제외한다. 알비온 SELECT/APPEAR/BLUE_CIRCLE은 전장에 대상이 없으면 보스
위치의 navigation ground를 고정 표적으로 사용해 다음 phase를 진행한다. 마리오 입장 시
Composition SOUND와 WORLD soundTracks 및 종료 tail의 전장 소리를 정리한다. WORLD
시각·서버 시계는 계속 유지하며 마리오 자체 공격·피격·BGM은 해당 audience에서 재생한다.
후속 사용자 확인에 따라 다른 플레이어의 일반 스킬·탑승 전투음과 에스더 시전/NPC 음원도
포함한다. 기존 음원의 handle을 소유해 진입 전부터 재생 중인 tail까지 중단하고, cue 시계는
계속 소비한다. 입장 snapshot의 player 배열 순서와 초기 network state가 차단을 우회하지
않도록 로컬 상태 반영 순서를 확인한다.

## G15. 독립 원본 트리거 사운드와 마리오 원본 공격·피격

SCENE03A의 별도 trigger37081_113→Matinee7/InterpData862→Track1239는 팝업북 Matinee0보다
먼저 문·배경을 재생한다. 해당 circus_finale WORLD owner에 원본200ms soundTrack을
연결하고 기존 컷신 시계를 보존한다. 마리오 뿅망치·세 색 공 파괴·비행 공 피격은 실제
원본 action/FX/sound event를 찾아 기존 Effect·Character·WORLD 소비자로 연결한다.
새 Resources는 설치본과 Desktop/GBResources의 같은 상대경로로 전달한다. 실제 Data·
publisher 출력과 C++를 통합해 Debug/Release Product Build, 관련 focused tests 및
JSON/XML parse·diff 검사를 수행한다. 화면·소리의 최종 판정과 Client 실행은 사용자 몫이다.
