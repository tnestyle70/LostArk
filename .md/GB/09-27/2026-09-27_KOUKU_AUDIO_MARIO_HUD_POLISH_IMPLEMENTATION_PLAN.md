# 쿠크 사운드·장판·마리오·HUD 후속 구현 계획

## G00. 현재 저장본과 적용 경계

기준 HEAD는 `369987261a1e99c1de98a370c26b69acf3fc7023`, 작업 브랜치는
`GB/Valtan-Patttern-Complete`다. 사용자가 방금 저장한 Composition의 기존 변경을 포함해
반영하도록 승인했다. 최신 디스크를 stable ID와 변경 필드 기준으로 병합하며, 교체 직전
해시 재검사·백업·원자 교체를 유지한다. Client/UI는 실행하지 않는다.

## G01. 원본 사운드와 실제 재생 구간

`Tools/SoundPipeline`, `Data/Sound/CharacterSoundCatalog.json` 및 기존 Composition과
WorldSequence Sound 소비자를 사용한다. Showtime 사각 폭발은 Attack33_ProjExp1,
고정 착탄과 무지개 격자는 Attack11_ProjExp1, 총구 발사는 Attack23_Cast3이며
일반/붉은 톱날은 Attack10_Proj1/Proj2다. 원본 은행의 random/layer/loop 의미를 보존해
미디어를 설치하고 GBResources에 같은 Resources-relative 경로로 전달한다.
무지개 격자의 각 실제 폭발과 사각 세 폭발에 한 번씩 연결한다. 팡파레 도넛은 현재
이펙트의 원본 참조를 먼저 찾고, 원본이 없을 때만 사용자가 허용한 피자 폭발음을 쓴다.
톱날은 기존 World Sound track에 수명 반복을 연결하고 Stop/seek/pause를 함께 처리한다.

## G02. 무지개 장판 고정과 사각 공습탄

P38의 두 격자 occurrence는 P47 문양장판과 같은 `BOSS + followBoss=false`로
발생 시 보스 위치·방향을 한 번 잡고 이후 월드 위치를 유지한다. MAP의 절대 원점으로
바꾸지 않는다. 원본 recipe와 사용자 offset·scale·Collider 시간은 보존한다.

`effect.kouku.gate3.showtime.rectangle.impact`에 원본 공습탄 leaf를 세 번 합친다.
기존 세 폭발의 X/Z 중심에 맞춰 위에서 아래로 낙하시키며, 현재 폭발 시간은 유지하고
선행 낙하 시간만 그룹 앞에 확보한다. 기존 그룹의 stable element ID와 사용자 편집을
보존한다. 실제 particle 샘플에서 축별 위치·착탄 시점·유한 값·발생 개수를 확인한다.

## G03. 마리오 참가자별 표현과 색상

Server가 승인한 로컬 플레이어의 Mario 상태로 해당 참가자만 커튼을 숨긴다.
외부 관객의 커튼과 다른 Effect는 유지한다. Server의 기존 입장별 3색 random 선택과
snapshot 전달을 조사하고, 항상 노랑으로 보이게 하는 실제 소비 지점을 수정한다.
Client가 필요한 공 색을 자체 선정하지 않는다.

## G04. 작은 체력바와 무력화 UI

`MainApp`, `WorldHealthBarView`, `CombatHUDViewModel`, `ClientReplication`과 기존
`KoukuHudModes.json` 저장 계약을 확장한다. 일반 몬스터·쿠크세이튼·쿠크·발탄의
작은 적 HP 위치는 각각 X/Y를 저장한다. 대형 세이튼과 카드미로의 카드 병정·플레이어
작은 바를 숨긴다. 무력화 위치에도 X/Y를 제공하고 네 지정 패턴은 기존 주황 fill
asset과 중립 tint로 표시한다. read-only HUD 상태와 최신 필드 병합·실패 보존을 유지한다.

### G04 후속 — 카메라 이동 시 작은 체력바의 앵커 오차

사용자는 X/Y 튜닝과 별개로 쿠크의 작은 체력바가 카메라 이동 때 보스에서 벗어난다고
보고했다. 현재 `WorldHealthBarView::BoundsHead`는 skin palette 적용 전 정점의 bind bounds를
머리 위치로 사용한다. 설치 G1 쿠크의 기존 앵커는 root 위 약 0.061m지만 idle 머리 본은
약 2.285m다. 정상 투영을 하더라도 잘못된 월드 기준점과 실제 머리는 카메라에 따라
다른 화면 위치가 된다.

`WorldHealthBarView.cpp`의 CNpc 경로만 실제 `bip001-head`의 현재 combined transform을
NPC world에 적용한다. bone에 이미 들어간 preScale를 다시 곱하지 않는다. bone이 없는
모델은 기존 bounds fallback을 유지한다. 플레이어·발탄·고정 상단 보스 HUD와 저장 X/Y는
유지하며 기존 다른 미커밋 변경을 보존한다. 현재 제품 build는 사용자가 직접 하므로
변경 TU 최소 컴파일·설치 모델 수치 검증만 수행한다. Client/UI는 실행하지 않는다.

### G04 후속 — 주황 무력화 표시와 독립 크기 조절

`BossUI.json`에서 주황 `Boss_StaggerFill` 뒤에 같은 영역의 불투명 보라색
`Boss_StaggerTrack`이 생성된다. 같은 UI layer의 순서가 유지되어 track이 fill을 덮는다.
제품 UI는 scene 최종 합성 뒤에 그려지므로 전역 gamma·FXAA·LUT를 변경하지 않는다.
사용자는 감소한 부분에 빈 바가 보이기를 요청했으므로 `MainApp::Update_BossHealthBar`에서
track을 숨기고 기존 `Boss_StaggerBg` 위에 남은 수치 비율의 주황 fill만 표시한다.

`MainApp.h/.cpp`의 기존 무력화 세 slot rect를 원본으로 보관하고 배경 frame 중심을
기준으로 함께 변환한다. 사용자 요청에 따라 기본 폭 배율은 1/3, 두께는 1이다.
현재 X/Y offset은 유지하고 매번 원본 rect에서 계산하여 반복 편집 시 축소가 누적되지
않게 한다. F1 기존 체력바 위치 창에 무력화 전용 폭·두께 조절을 추가한다.
다른 체력바와 광기 게이지의 크기는 변경하지 않는다.

`KoukuHudModes.json`의 `healthBarPositions`에 optional `mechanicWidthScale`과
`mechanicHeightScale`을 함께 저장한다. 필드가 없으면 위 기본값을 사용한다. 기존
12개 X/Y 필드, 최신 디스크의 다른 편집, 공통 저장 lock·동일 필드 충돌·백업·원자 교체를
보존한다. 코드 수정 중 저작 JSON을 직접 교체하지 않고 사용자의 F1 저장으로 기록한다.

새 제품 C++ 파일과 프로젝트 등록은 없다. 변경 TU 최소 컴파일과 실제 메서드 추출 검사로
기본 크기, 반복 조절, 잘못된 크기의 실패 보존, 저장·재로드·독립 필드 병합·동일 필드
충돌을 확인한다. 첨부 화면의 광대 얼굴 바는 플레이어 광기 게이지다. 이 식별과 별도로
보스 HP의 이동 문제가 있는지는 사용자 화면 확인과 구분하며 추측으로 앵커를 바꾸지 않는다.

## G05. 창술사 Movie 헤어

직전 FT43 파생 모델의 geometry·inverse bind·Movie pose를 현재 설치본에서 대조한다.
사용자에게 보고된 두피 밀착을 개선하며, 기본 모델·아바타 재사용이 필요한 경우에도
기존 Movie 배우·Intro/Loop·시간·좌표 계약과 실제 CModel 소비를 보존한다.
새 asset이 생기면 설치본과 GBResources의 해시를 일치시킨다.

## G06. 검증과 설치

변경된 parser/저장/재생·Server 조건을 focused 검사하고 필요한 owner publisher로
생성 실행 데이터를 갱신한다. 기존 C++ 파일 인코딩을 유지한다. 새 제품 C++ 파일이
필요하면 프로젝트와 filters를 함께 등록한다. 최종 공식 Product Debug와 Release를
순서대로 빌드하고 JSON/XML parse 및 `git diff --check`를 확인한다. 화면·청취 판정은
사용자 확인과 자동 검증을 RESULT에서 구분한다.
