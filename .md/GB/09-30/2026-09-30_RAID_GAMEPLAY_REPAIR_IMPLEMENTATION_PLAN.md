# 레이드 전투·관전·실패 처리 구현 계획

## G00. 범위와 현재 상태

사용자가 저장한 쿠크 Composition과 기존 dirty 변경을 보존한다. 현재 브랜치는
codex/release-regression-20260929이며 다른 작업 변경을 자동 commit하지 않는다.
사용자는 전체 반영·publish·Debug/Release 빌드·검증 ZIP과 신규 Resources의 GBResources 복사를 승인했다.
Client 실행 및 화면 판정은 사용자 전용이다.

## G01. 공통 무력화와 피해 감소

쿠크는 현재 HP 감소량과 회오리 수류탄 credit으로 무력화를 판단한다. 이를 독립 누적으로 바꾼다.
일반 공격은 보스 방어가 적용된 피해의 1/1000을 무력화에만 기여하고 회수는 최대치의 1/3이다.
Retail에 단일 raidStaggerMaximum=40000을 추가하고 publisher, catalog, Server numeric CAS 저장,
F1 Balance Test를 연결한다. 발탄과 쿠크의 열려 있는 무력화 창이 같은 수치를 소비한다.
마리오에 이미 저장된 logic.558/559 Duration은 시간·배치를 유지하고 typed BOSS_DAMAGE_REDUCTION으로
연결한다. HP 피해에만 1/1000을 적용하며 무력화 계산과 중복 적용하지 않는다.

## G02. 광기와 배틀 아이템

인형·공의 저작 광기는 보호막에 흡수된 유효 접촉에도 현재 증가량·배율을 유지한다.
다른 피해의 HP 비례 광기 정책은 유지한다. 배틀 아이템은 실제 Server inventory 소비와 반복
사용을 검증하고, 원인을 확인해 포션과 같은 보유 수량 계약을 사용한다.

## G03. 쿠크 기믹과 발탄 패턴

쿠크 입장·실패·90초 제한·갈고리·폭발 및 망치 충돌·빙고 표현을 해당 담당 계획/결과로 기록한다.
발탄은 카운터 전 버러지 반복, 발악 앞쪽 포탈과 돌진, 예고 제거, 실제 부위 파괴 표현을 연결한다.
데이터는 stable ID와 필드 기준으로 최신 저장본에 병합하고 freshness·백업·원자 교체를 사용한다.

## G04. 관전과 HUD

관전 대상 stable player ID, 사망 위치 유지와 부활 추적, 복제 스킬 카메라를 연결한다.
서버의 미니게임 종료 tick을 제품 타이머가 소비하고 Debug/Release 모두 90초로 표시한다.
보호막, 발탄 초상화, 모든 플레이어의 이동 문구, 타이머 소수점 정렬을 기존 제품 UI에 반영한다.

## G05. 검증과 전달

관련 Server contract와 publisher 검증, JSON/XML parse, git diff --check를 수행한다.
정상 증분 Product Build를 Debug/Release 각각 실행한다. 새 입력은 기존 프로젝트 파일을 확장하고
새 CPP 파일 추가 시 project/filter 등록을 함께 확인한다. 빌드 결과와 실제 gameplay 수치 검증,
미실행 사용자 화면 확인을 RESULT에 분리한다. 공식 packaging 도구로 검증용 ZIP을 생성한다.

## G06. 추가 즉사와 피해 감소 표시

모든 명시적 즉사·전멸과 최대 HP 100% 판정은 공통 lethal 경로를 사용해 보호막·무적·시간 정지·
부적·사망 방지를 무시한다. 공간 조건과 성공 조건은 그대로 판정하며 실패 대상 선정 단계에서도
생존자가 notReady·잡힘·낙하 상태라는 이유만으로 제외되지 않게 한다. 실제 피해 감소 타격은
서버가 typed flag를 보내고 숫자 아래 72% 크기의 흰색 피해 감소 문구를 표시한다.

## G07. 피격 숫자 변동

방어·피해 감소 계산을 마친 일반 피격 피해에 서버가 한 번만 정수 ±10% 난수를 적용한다.
기준 100은 90~110, 1320은 1188~1452이며 양방향 대칭으로 평균 피해를 유지한다.
그 값에서 보호막을 차감하고 실제 HP 손실을 기존 damage event로 보낸다. 명시적 즉사·전멸은
난수를 적용하지 않으며, 인형·공의 저작 광기 증가량과 플레이어의 보스 공격 수치는 유지한다.

## G08. 가디언 나이트 최종 추가 요청

ALT_V 49420의 현재 300ms 피해는 브레스 3790ms 및 카메라 복귀 3800ms와 맞지 않는다.
PlayerSkills, HitShapes의 DAMAGE/COUNTER/STAGGER, presentation HIT와 provenance를
3800ms로 맞추고 실제 스킬 시스템에서 직전 무피해·직후 단일 타격을 검증한다.
광포화 S의 실제 머리·목 ModelCue 두 개는 V의 대응 재질 프로그램·텍스처·상수를 사용한다.
S의 메시·애니메이션·배치·수명과 다른 particle은 유지한다. 추가 의존 리소스는 GBResources에
복사하고 hash를 확인한다. 관전 로직은 코드 경로를 다시 확인해 대상 순환·사망 고정·부활 추적과
ALT_V 연출 재생 범위를 사용자에게 설명한다.

## G09. 인형·공의 광기 접촉과 피해 처리 완전 분리

현재 special 접촉도 damagedPlayers(HP 또는 shield의 실제 감소)에 의존하므로 피해가 사전에
차단되거나 damage0인 경우 광기가0이다. 인형·공의 stable WORLD join과 게시된100ms 반복,
10%×200%/1000ms 정책은 정상이다. Apply_Results에서 이 두 source의 유효 spatial contact만
일반 피해 성공 gate에서 분리한다. 생존·combatReady·일반 상태·같은 층/범위·활성 window와
대상 object 생존 계약은 유지한다. 일반 브레스의 피해 비례/명시 광기는 기존 조건을 유지한다.
특수 접촉에서 생성된 ABSORB 이벤트만 해당 호출 범위에서 제거해 shield흡수 문구와 숫자를
표시하지 않고 snapshot광기는 유지한다. 실제 HP 손실 이벤트는 보존한다.
실제 함수의 차단 전/후 검증과 기존 KoukuProduct fixture를 보완하며 현재 사용자 저장값
raidStaggerMaximum50000 및 모든 저작·게시 데이터는 수정하지 않는다. 제품 빌드/ZIP은 출력 잠금
해제 후 보호막 표시 수정과 함께 반영한다.


## G10. 최종 발탄 교정과 PR487·488 통합

사용자가 PR487(마하라카/워터팡)과 PR488(아바타 상점/착용)을 모두 통합·merge하도록 승인했다.
현재 저장본을 백업하고 source 변경을 보존한 채 기능 브랜치에서 양쪽 변경을 합친다.
두 PR과 현재 관전 camera state가 각각 protocol125를 사용하므로 통합 wire는126으로 올리고
모든 writer/reader/validation 및 기존 회귀를 보존한다. Items v7도 publisher로 갱신한다.
전체 publish는 기존 Run-FullPipeline DataOnly, Debug/Release는 정상 Product Build를 사용한다.
Valtan lifecycle/전용폭탄/무소환회복, protocol 왕복과 두 PR에 해당하는 기존 검증을 수행한다.
컴파일 산출물·개인 backup·numeric live receipt는 commit에서 제외하고 생성 DataFiles는 포함한다.
검증 후 통합 PR을 만들고 main에 merge하며 원 PR 두 개의 실제 merged 상태도 확인한다.
