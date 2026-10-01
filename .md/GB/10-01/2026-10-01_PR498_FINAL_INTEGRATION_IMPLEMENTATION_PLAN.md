# PR498과 현재 반영 전체 통합·최종 빌드·ZIP 계획

## G01. 사용자 승인 범위와 보존

2026-10-01 사용자는 PR498과 현재 폴더의 모든 작업 변경을 포함해 merge하고 Debug/Release
전체 빌드와 최종 ZIP을 요청했다. 콜로세움·MVP·쿠크뿐 아니라 가이드·이동·렌더링·프로파일러
수정도 포함한다. TGA3개는 추가로 Desktop/GBResources에 전달한다.
시작 HEAD는 b83d646bebc8e6cffdd3a6cb84355b7c7f47f734이며 PR498 head는
5e6bb64ea7c71419be88bff52135ef020e8519c1이다. 기존 working diff와 미추적 파일을
out/FinalIntegrationPR49820261001에 보존한 뒤 별도 기능 branch에서 현재 소스를 commit한다.
backup/retired 파일과 비활성 생성본은 삭제하지 않고 소스 commit에서 제외하며, 실제 필요한
새 테스트3개와 활성 Valtan presentation generation은 포함한다. Resources/빌드 산출물은 Git에 넣지 않는다.

## G02. PR498 통합

동일 base에서 파생한 PR498을 현재 반영 commit에 병합한다. 충돌은 stable field/기능 단위로
양쪽을 보존하고 파일 전체 ours/theirs를 사용하지 않는다. 같은 shared gameplay contract의
마하라카 guard와 콜로세움 guard, 게시 World revision/점프 목적지를 대조한다.
local 검증 후 exact head를 확인해 원격 PR498 및 통합 PR을 main에 병합한다.

## G03. 정상 Product와 관련 검사

사용자가 남은 Server 종료를 회신했으므로 실제 표준 Client/Server 점유가 없는 것을 확인한 뒤
기존 runner로 Debug, Release 순서의 일반 Product Build를 수행한다. Clean/Rebuild/SkipBuild는
사용하지 않는다. Engine/Shared/Server/Client와 현재 변경 dependency를 정상 빌드한다.
각 구성의 Maharaka/Colosseum 관련4개 contract와 현재 변경의 기존 이동/Guide 검사, UI/data
검사를 실행하며 실패는 원인에 해당하는 최소 범위로 수정하고 재검증한다. source JSON/XML parse,
active generation 연결과 diff check를 확인한다. Client/UI·실제 화면은 에이전트가 실행하지 않는다.

## G04. 최종 전달

기존 build_portable.py의 numeric source·패키징 검사와 이번 Release PASS receipt를 사용해
Desktop/LostArk-Release-20261001-FINAL.zip을 만든다. EXE/DLL/CSO/Data/DataFiles와
launcher/CRT는 ZIP에 넣고 Resources는 기존 외부 Resources 계약을 유지한다. 사용자 지정
GBResources의 TGA3개와 PR498 효과음 의존성을 확인·전달한다.
Server의 ValtanPresentationGenerations는 Gameplay.bootstrap이 참조하는 활성 ID만 ZIP에
포함한다. 과거 생성본은 디스크에 보존하고, 참조 오류·누락은 패키징 실패로 처리한다.
활성/비활성 생성본 및 실패 경계를 실제 collect fixture로 검증한다.
zip CRC·payload SHA·manifest/binary pins·launcher --check를 검증하고 실제 경로/크기/hash,
PR 병합 SHA와 Debug/Release 결과를 대응 RESULT에 기록한다. 이전 ZIP은 지우지 않는다.

## G05. 강화창의 명예의 속삭임 보유 장비 목록

사용자의 추가 요청에 따라 강화창은 실제 inventory snapshot에 수량이 남아 있는 명예의 속삭임
세트 장비만 표시한다. 현재 MainApp의 BuildItemUpgradeSlots는 category=combat 전체를
받아 다른 획득 장비가 함께 나오며, 여섯 행의 아이콘은 JSON의 창술사 운명의 불꽃 예시로 남는다.
ItemCatalog에는 별도 장비 setId가 없으므로 EQUIP_ 접두사, _HONORWHISPER_ 세트 stable ID,
weapon/helmet/shoulder/top/pants/gloves equipSlot과 ID의 정확한 suffix 일치를 확인한다.
표시명 번역이나 소유하지 않은 catalog 항목으로 목록을 만들지 않는다. 장착 항목은 같은
inventory snapshot의 eEquippedSlot 상태와 무관하게 소유 목록에 유지한다.

Client/Private/MainApp.cpp의 기존 후보 함수와 창 열기·선택 갱신 경로만 수정한다. 작은 내부
함수 하나가 후보에 맞춰 여섯 행의 아이콘·행 배경·선택 표시·중앙 아이콘을 동기화한다.
창이 열리는 첫 프레임에도 동일 함수를 사용하며 빈 행은 숨기고 선택 index를 보이는 범위로
제한한다. 후보가 없으면 성장/재련 버튼의 입력을 받지 않는다. MainApp.h의 기존 후보 설명을
새 필터에 맞춘다. 획득·인벤토리·Server·JSON·강화 결과/성공률 계약은 변경하지 않는다.
새 C++ 파일이나 project/filter 등록은 없다.

검증은 실제 변경 함수와 실제 ItemCatalog를 사용한 out 내부의 native focused probe로 수행한다.
섞인 획득 장비, 장착/미장착 여섯 부위, 수량0/미등록 항목, 번역명만 같은 다른 장비, 빈 목록,
첫프레임·후속 snapshot의 행 아이콘과 선택 표시를 확인한다. Product Debug/Release 컴파일은
G03 통합 빌드에서 수행하며 Client 화면은 사용자가 확인한다.

## G06. 촬영용 Release의 F1·F7·FPS 표시 비활성화

사용자는 실제 촬영을 위해 F1 ImGui 도구창, F7 프로파일러 창과 화면의 FPS 숫자를 모두
끄도록 요청했다. 현재 공통 입력·렌더 소유자는 Client/Private/MainApp.cpp이며 Level별
창 제목 FPS는 이미 _DEBUG 전용이다. RenderFpsText는 Release에서 사용자 FPS 표시
선택을 검사하지 않고 항상 그리므로 함수의 출력 경계를 함께 바꾼다.

기존 _DEBUG 분기로 F1 토글, F7 토글, DeveloperTools 및 Profiler 렌더 구간,
RenderFpsText 본문을 Debug에서만 실행한다. Release는 키를 폴링하거나 새 도구창을
생성하지 않으며 Debug의 기존 저작 도구 동작은 유지한다. 비동기 프로파일 저장 완료 회수는
현재 Update_SaveState 경로에 남긴다. ImGui 프레임 안의 일반 채팅·파티·레이드 UI와
backend 프레임 수명은 유지한다. F6 카메라, 프레임 제한, 계측 데이터·기존 캡처 파일은
변경하지 않는다. 새 설정·JSON·C++ 파일 또는 project/filter 등록은 없다.

MainApp.cpp의 기존 UTF-8 BOM 없음/CRLF 인코딩을 보존하고 기존 강화창 변경도 유지한다.
변경 전후 hash와 scoped diff check를 기록한 뒤 Product Debug/Release 컴파일로 양쪽
전처리 분기를 검증한다. Client 화면은 실행하지 않는다. AGENTS/CLAUDE/팀 사용서의
공통 F1/F7 설명과 통합 RESULT는 root가 같은 변경에서 현재 계약으로 갱신한다.
