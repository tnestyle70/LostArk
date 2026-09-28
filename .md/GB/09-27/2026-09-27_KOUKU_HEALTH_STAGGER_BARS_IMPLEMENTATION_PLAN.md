# 쿠크 체력·패턴 게이지와 PR 통합 구현 계획

## G00. 범위와 현재 기준

사용자는 이미 병합된 #465·#467의 치명적인 결함을 보완하고, #466·#468·#469 및 이번 체력바 변경을 함께 검증·병합·동기화하도록 요청했다. 기존 기능 브랜치의 5fc2f940a와 main의 8c7ad98b0를 합친 기준선은 6e46c50fb다. 에스더 #469의 5d0ca1f98도 구현 전에 통합한다. 다른 세션의 변경과 팀장 렌더링 설정은 보존한다.

실측한 Retail 창술사 공격력은 23,000, 기본 공격 34010의 기준 배율은 100%, 플레이어 최대 체력은 132,000이다. 기본 공격 기준은 방어·치명타·피해 편차 전의 23,000으로 고정한다. 1관문과 마리오 2페이즈 무력화 기준은 115,000, 카드비 병정 체력은 69,000이다. 카드비 병정의 피격 피해는 사용자가 확정한 13,200이며 방어력으로 다시 줄이지 않되 보호막·무적·받는 피해 효과는 기존 서버 판정을 따른다.

## G01. 서버 패턴 게이지와 카드비 수치

`Shared` 보스 snapshot에 `NONE`, `STAGGER`, `BOSS_HP` 구분과 현재/최대 게이지 값을 추가한다. Server가 실제 활성 occurrence의 시작·종료·취소를 기준으로 값을 만들고 Client는 이를 표시한다. 기존 상시 무력화 수치를 활성 패턴처럼 표시하지 않는다.

1관문 무력화와 3관문 마리오 2페이즈는 게시된 `STAGGER_WINDOW`의 남은 양을 표시한다. 빙고/앵콜 블랙홀은 `BINGO_DETONATION` 직전 13초 동안만 실제 보스 현재 HP/최대 HP를 투영한다. 블랙홀 바에 별도 무력화 성공·패턴 중단 판정을 연결하지 않고 보스 실제 HP도 임의로 115,000으로 바꾸지 않는다.

`CARD_RAIN_SOLDIERS`에 `soldierMaxHp`, `soldierDamage`를 추가한다. 두 값은 0..2,000,000,000 정수이며 0 또는 누락은 기존 profile 사용이다. 카드비 logic 129만 69,000/13,200을 저작한다. 미로 병정 profile은 유지한다. Client composition document의 parse/validate/serialize와 workbench, Python publisher, Server reader·trigger·spawn·MonsterBrain 소비자를 같은 계약으로 연결한다. 생성 bootstrap은 직접 수정하지 않고 공식 publisher로 만든다.

## G02. 모든 관측자를 위한 머리 위 체력바

`CClientReplication`이 승인된 snapshot의 살아 있는 적/다른 플레이어 상태와 약한 presentation 참조를 `CCombatHUDViewModel`로 전달한다. 자신의 player는 제외하고 reliable despawn 및 world reset에서 즉시 제거한다. UI는 packet이나 socket을 읽지 않는다.

새 `CWorldHealthBarView`가 해당 read model의 실제 머리 위치를 투영한다. `Data/UI/HeadStatus/HealthBar_Layout.json`의 stable slot은 `Health_Frame`, `Health_EnemyFill`, `Health_PlayerFill`, `Health_ShieldFill`이다. 기존 ImmuneGauge frame과 빨간 track, 흰 shield track을 재사용한다. 다른 아군의 하늘색 HP는 원본 EFUI_STATUS `headstatus_i6`의 (763,53,78,5) 영역을 색·크기 변경 없이 추출한 `UI/HeadStatus/HS_Fill_Player.png`를 중립 tint로 사용한다. 적은 빨강, 보호막은 흰색으로 나타낸다. HP와 shield 합계가 최대 HP를 넘는 경우 전체 길이 안에서 함께 정규화한다. 필요한 네 이미지를 Desktop/GBResources의 같은 상대 경로에 전달한다.

기존 머리 위 boss immune widget 생성·갱신은 공통 HP view로 대체한다. 기존 resource 파일을 무관하게 삭제하지 않는다. 상단 `Boss_StaggerFill`은 요청한 `boss_bar_fill_orange.png`를 쓰고 서버 gauge kind가 활성인 구간에만 frame/track/fill을 표시한다. 기존 상단 보스 다중 HP bar는 유지한다.

새 Client H/CPP는 기존 물리 폴더에 추가하고 `Client.vcxproj`와 `.filters`에 필요한 항목만 등록한다. UI JSON은 `96.DataFiles` 아래 `None`으로 등록한다.

## G03. 통합 PR의 결함 보완

#465의 연속 초상 요청마다 full G-buffer viewport를 복구한다. deferred에서 제외한 머리카락·속눈썹 등 forward 재질은 초상 HDR에 별도 그리며 최종 투명도에도 반영한다. 필드 광원과 SourceCharacter 재질을 유지하고 SSAO/bloom 등 기존 초상 제외 정책과 팀장 quality 값을 임의 변경하지 않는다.

#467의 배 전환은 항해 가능한 위치 검증이 끝난 뒤 기존 탈것 스킬 상태를 변경하도록 한다. 거절되면 위치·고도·속도·action을 보존한다. #469는 에스더 피해·보호 영역·수명·반복 타격 연결을 확인하고 확인된 치명적인 실패만 보완한다.

#469 통합 실측에서 쿠크 관문 초기화가 기존 장식용 에스더를 보존하도록 구현돼 있었다. 이제 실제 피해와 보호 영역이 생겼으므로 이 보존은 재시작한 전투에 이전 공격·무적을 넘길 수 있다. 관문 despawn의 preflight는 계속 읽기 전용으로 유지하고 실제 commit에서 진행 중 소환, 대기 소환, 보호 영역과 에스더 guard를 함께 종료한다. 이를 실제 reset 경로를 통과하는 서버 회귀 검증으로 확인한다.

## G04. 검증과 전달

사용자가 Debug 2클라이언트 Complete Play에서 주사위 진입 시 모두 정지하고 속박 효과가 없는 회귀를 추가로 보고했다. 실제 실행 로그는 Client와 Server 모두 Action revision 2443/Sequence 182 일치를 확인했다. 실제 게시 P79→P78 전환을 재현한 검사에서 Server는 자유 1명/속박 1명을 유지하지만 속박 시작 tick부터 snapshot encode가 실패했다. Shared reader/writer의 `isPatternBound => !isCombatReady` 제약이 카드 피격을 허용하는 Server 상태와 충돌한다. 속박의 입력 차단과 피격 가능 상태를 분리하고, 실제 두 session의 snapshot decode까지 검증한다. 이펙트 위치·방향은 사용자가 직접 조정하므로 해당 필드를 변경하지 않는다.

Server 중심 계약 검증은 실제 게이지 구간/종료/취소, 블랙홀의 HP 투영과 무력화 판정 분리, 카드비 생성 HP와 방어력 105 대상의 13,200 피해 및 shield 소비를 확인한다. 변경한 Shared snapshot은 protocol roundtrip을 확인한다. Client codec은 새 필드 왕복과 잘못된 종류·범위 거절을 확인한다. 초상 shader와 항해 거절 회귀는 해당 최소 검증을 수행한다.

변경 JSON/XML parse와 `git diff --check`, Product 증분 컴파일을 수행한다. 실행 중 EXE/DLL의 실제 잠금이 확인되면 사용자 실행을 임의 종료하지 않는다. Client와 아레나 화면 확인은 사용자가 직접 한다. 자동 검증과 화면 미확인, 설치 데이터와 실행 중 Server 반영을 RESULT에서 구분한다.

기능 코드·저작 데이터·publisher 생성물·검증 결과를 한 변경으로 commit/push하고 검토 가능한 PR을 만든다. 필요한 열린 PR을 사용자가 승인한 범위에서 병합한 뒤 main을 fast-forward 동기화하고 작업 트리 상태와 원격 일치를 확인한다. 무관한 변경을 버리거나 강제 push하지 않는다.

## G05. F1 체력바 위치 조절과 두 구성 빌드

기존 F1 광기 위치 조절 바로 아래에 무력화 주황 바, 다른 아군의 하늘색 HP, 적·보스의 빨간 HP Y offset을 각각 추가한다. 단위는 1280×720 기준 pixel이고 +Y는 아래다. 기본 0으로 기존 위치를 유지하며 finite -1280..1280만 허용한다. 머리 위 bar는 frame/HP/shield 전체가 같이 이동하고 무력화는 frame/track/fill 전체가 원래 rect+offset으로 이동해 매 프레임 누적되지 않는다.

정본은 기존 `Data/UI/KoukuSaydon/KoukuHudModes.json`의 optional `healthBarPositions` block이다. `CMainApp`이 현재값과 마지막 저장값 및 Get/Set/Save/Reload를 소유하고 `CWorldHealthBarView`는 ally/enemy 두 Y값만 소비한다. 최신 디스크 문서에서 수정한 field만 병합하며 기존 madness/modes와 독립 축의 외부 편집을 보존한다. 같은 field 충돌·잘못된 JSON·동시 저장은 현재 미리보기와 저장본을 보존하고 오류를 표시한다. 기존 광기 저장 lock을 공유하고 임시 파일 검증, 교체 직전 freshness 확인, backup/atomic replacement를 유지한다.

광기 및 세 bar 위치 control을 공통 helper로 묶어 Debug의 기존 자리와 Release F1에 노출한다. 새 C++ 파일과 project/filter 등록은 필요 없다. 사용자의 실행 중 Client는 조작하지 않는다. 변경 JSON parse·설정 입력/저장/재로드·실패 보존과 최소 Client 컴파일을 확인하고, 이어 Debug와 Release의 공식 Product 빌드 및 Release 주사위 Complete Play CPU 회귀를 확인한다.

## G06. 현재 Server 주소와 기존 진단 표시 유지

사용자의 접속 오류 화면은 192.168.0.22:7777 WSA10060이다. 현재 이 PC의 default-route Wi-Fi 주소는 192.168.200.113이고 이전 성공한 두 Client 기록도 이 주소를 사용했다. 사용자 요청에 따라 TeamLanEndpoint, compiled Client fallback, Debug/Release debugger 환경과 현재 팀 문서를 192.168.200.113:7777로 함께 변경한다. Server bind 0.0.0.0과 만료일은 유지하며 격리 localhost 검사는 바꾸지 않는다. Sync 스크립트는 실제 debugger 환경과 방화벽 상태를 확인하되 Client/UI나 Server를 자율 실행하지 않는다. 기존 Network endpoint 계약 검사를 실행한다.

로비의 Initialize 자체에는 접속 호출이 없고 입장 명령이 접속을 시작한다. 사용자가 거슬렸다고 설명한 대상은 Server CMD의 ShipNpc 생성 로그이며 최종 지시는 무시하고 유지하는 것이다. Debug 로비 진단 패널의 기존 상시 표시와 Server CMD 로그를 그대로 유지한다. 임시로 추가한 F1 표시 조건과 이를 위한 getter는 제거한다.
