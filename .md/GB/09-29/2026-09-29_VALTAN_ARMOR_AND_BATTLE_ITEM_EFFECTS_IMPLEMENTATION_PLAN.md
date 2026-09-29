# 발탄 갑옷 파괴와 배틀 아이템 이펙트 구현 계획

## G00. 현재 기준과 변경 범위

시작 branch는 codex/kouku-release-sequence-ready, HEAD는3a55be17c다.
직전 Release 검증의 발탄·마리오 수정과 사용자의 Camera/UI 저장본을 보존한다.
Debug Client/Server가 실행 중이며 사용자는 Release만 먼저 검증하도록 지시했다.
Client/UI 자동 실행·Reload·종료는 하지 않는다.

발탄 갑옷은 원작 MN_RPBF_01_Parts1/Parts2 모델과 Server의 typed part 상태를
사용한다. 과거08-20 RESULT에는 갑옷 표시·제거와 반응 애니메이션까지 기록됐고
Par_D_RPBF_PartsDestruction_01/02의 파편 이펙트는 미복구였다. 현재 실제 consumer와
설치 WModel을 다시 대조하여 당시 기록을 현재 완료 증거로 대신하지 않는다.

현재 ItemCatalog에는 파괴 폭탄·회오리 수류탄·성스러운 부적의 정의와 구매행이 있다.
Handle_UseItem은 healPercent0인 아이템을 거절하므로 세 아이템 사용은 진행되지 않는다.
시간 정지 물약은 item/stock 정의부터 없다. 기존 구매·인벤토리와 Server 전투 경계를
확장해 아이템 사용과 표현을 연결한다.

## G01. 원작 리소스와 All Effects

설치 retail Item/Skill/SkillBuff와 원본 package를 따라 네 아이템의 발생 이펙트·부착·
시간·리소스를 확인한다. 원작 자료를 기존 authored Effect와 Resources-relative 경로로
연결하고 All Effects의 World 목록에서 picking·이동 표시 다음에 네 아이템을 등록한다.
stable ID는 effect.world.item.destruction_bomb, whirlwind_grenade, holy_charm,
time_stop의 같은 prefix를 사용한다. 기억에 의존해 네 효과를 같은 흰 tint로 만들지 않는다.
시간 정지 item icon도 원작 자료에서 회수한다. 원작 수치와 프로젝트 수치는 구분한다.

발탄의 녹색 부위 파괴 가능 표식과 파편01/02도 실제 원작 occurrence를 찾아 분리한다.
기존 material program·renderer·particle 경로를 재사용하고 동일 역할의 별도 runtime을
만들지 않는다. 추가 Resources는 설치 위치와 공유 필요 범위를 RESULT에 남긴다.

## G02. Server 승인 아이템 사용

기존 ItemCatalog와 publisher/reader가 item별 효과 kind·피해·무력화·부위파괴·범위·
cooldown·duration을 소유한다. MainApp의 기존 직접 network 호출은 PlayerController와
IPlayerCommandSink로 연결하고, ground target은 Server에서 범위·유효 좌표를 검증한다.
구매·소지·생존·사용 가능 조건을 통과한 사용만 수량을 소비한다.

실제 피해·부위 파괴는 ServerCombatHitRuntime/BossCombatRuntime을 소비한다.
파괴 폭탄은 기존 typed partDamage, 회오리 수류탄은 stagger, 부적은 지원하는 해제 가능
상태를 해제한다. 사용자가 확정한 부적 보호는 대상 아군의 공포 즉시 해제와 3초 공포·피해 차단이다.
시간 정지는 별도 3초 보호와 행동 제한으로 처리하며, 일반 공격·즉사 collider hit에서만 제외한다.
기존 바닥·벽·낙하 처리와 명시적인 encounter wipe는 유지한다. 두 보호를 기존 무적 tick에
합쳐서 직접 전멸 판정까지 면제하지 않는다.

HUD 1~4는 현재 슬롯 배치를 읽고 typed Controller → command sink로 사용한다.
폭탄은 기존 CombatObjectRuntime의 Server-owned 투사체를 사용한다. 마우스 목표·발사·곡선 이동·
보스 접촉/목표 도착·피해·무력화/부위파괴를 서버에서 처리하고 기존 spawn/snapshot/HIT_PULSE/
despawn으로 네 클라이언트가 원본 비행 모델과 착탄 이펙트를 같은 위치에 표시한다.
성부 대상은 현재 캐릭터 메시를 world ray로 피킹한 stable net ID로 제출하고 서버가 파티·생존·
거리를 검증한다. 보호 표현은 복제 ActiveBuffs 종료 tick을 소비하여 수신 지연/재입장과
캐릭터 교체에도 남은 시간만 대상 Character에 부착한다. 원본 buffcolor 재질 제어는 기존
Character ownerControls 경로를 사용하고 스킬 action 수명과 분리한다.

사용자가 최종 선택한 회오리 수류탄은 HP 피해0, 현재 무력화 최대량의 ceil(max/3)를
매번 적용한다. 잔량의1/3이 아니다. 기존 일반 스킬의 수치는 유지한다. 쿠크의 HP 감소 기반
무력화 창에는 아이템 전용 기여량을 별도로 합산하여 실제 HP를 깎지 않고 같은 gauge와
성공 판정을 소비한다. 기여량은 해당 창의 종료·중단·교체에서만 정리한다.

Debug/Release 공통 F1의 Battle Items는 기존 Server 인벤토리 지급 command로 네 종류를
각10개 또는 일괄 지급한다. 사용은 실제 인벤토리에서 HUD1~4에 배치한 뒤 기존 숫자키로
진행한다. 지급 버튼에서 효과를 직접 재생하거나 쿨다운을 우회하지 않는다. 기존 베른 물약
상점에 시간 정지 물약도 등록하여 같은 구매·소지·사용 경로를 시험할 수 있게 한다.

## G03. 갑옷 상태와 화면 표시

갑옷 착용은 기존 body basis와 실제 Parts1/2 모델을 사용하고, 파괴 mask는 Server 상태만
소비한다. 파괴 가능한 GROGGY 창과 아직 남은 갑옷 조건을 함께 확인해 녹색 PNG를
기존 health bar와 같은 world-to-screen 경로로 표시한다. 모든 갑옷 제거 뒤에는 숨긴다.
실제 PART_BROKEN 발생에만 대응 파편과 발탄 위치의 파괴 성공 텍스트를 한 번 재생한다.
최초 입장·반복 snapshot은 이전 파괴를 새 성공으로 재생하지 않는다.

## G04. 검증과 설치

변경 정본은 최신 디스크를 재확인해 관련 필드만 병합한다. 기존 writer lock·백업·
freshness·원자 교체를 유지한다. Item과 필요한 gameplay/composition domain만 공식
publisher로 게시하고 생성물은 직접 편집하지 않는다.

실제 사용 커맨드의 구매·소비·효과·거절·중복과 observer 표현, 갑옷 남음/파괴 완료와
GROGGY 표시 조건을 집중 검증한다. Shared payload 변경은 protocol roundtrip도 확인한다.
변경 JSON/XML parse, Resources 참조, git diff --check 후 Release Engine/Shared/Server/
Client를 필요한 순서로 빌드한다. 새 C++ 파일이 있으면 해당 vcxproj/filters에 함께 등록한다.
구조·수치·컴파일 결과와 실제4클라 화면·음향 확인을 RESULT에서 구분한다.

추가 리소스는 사용자가 지정한 C:/Users/user/Desktop/GBResources에 Resources-relative
경로로 복사한다. effect의 assetId뿐 아니라 native material texture와 모델 재질까지 공식
collector로 의존성을 수집하고 설치본과 SHA-256을 대조한다. 기존 다른 파일은 보존한다.
