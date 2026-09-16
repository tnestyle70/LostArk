# 쿠크 1 / 3관문 공통 Pattern 설계

## G00. 현재 모델과 실패 경계

BossCatalog의 G1_SAYDON/G3_SAYDON은 모두 MN_RPCT_05 몸, WP_MN_RPCT_05 무기, 같은 animationSet을 사용한다. 작은 쿠크 MN_RPCZ_00과2관문 거대 세이튼 MN_RPCT_06은 이 공유 대상이 아니다. 현재 Pattern의 gateId와 targetBossPlacementId는 단일 관문을 고정하고 Flow validator·publisher·Server audition도 이를 검사한다. 목록 필터만 풀면 Server가 다른 Gate target으로 거절한다.

G1 Saydon spawn은(6.43,1.3,730), yaw237°이고 G3은(-0.07,1.32,942.330017), yaw223°다. P38 무지개댄스의 중앙이동 logic47, P39 알비온의 CENTER logic69는3관문 절대좌표를 저장한다. P39/P43의 일부 MAP presentation도3관문 위치다. 같은 모델이어도 이 위치는 그대로 공유하면 안 된다. BOSS bone/local offset, MAP arena placement, player-targeted 배치를 구분한다.

## G01. 한 저작 원본과 공통 목록

Pattern에 optional availableGateIds를 추가하고 공유 대상은 정확히 GATE1/GATE3 두 값으로 제한한다. 기존 gateId는 원본을 저작한 기준 관문이며 targetBossPlacementId도 해당 기준이다. stable patternId, Animation/Effect/Logic/Summon 원본은 한 벌만 유지한다. 같은 모델이라는 이유로 임의의 모든 패턴을 공유로 승격하지 않는다.

트리 최상단은1 / 3관문 공통, 다음은1관문·2관문·3관문·빙고다. 공유 Pattern은 공통에 한 번만 보이고 관문 전용 목록에서는 중복 표시하지 않는다. 기존 Parent의 내부 Pattern과 Summon child도 stable reference로 유지한다. 공통으로 전환하는 편집 명령은 의존 graph와 변환 가능한 anchor를 검사하고 실패 사유를 표시하며 전체 draft를 보존한다.

Flow의 Add from All Patterns는 현재 관문 전용 항목과 해당 관문에서 사용 가능한 공통 항목을 함께 보여 준다.1관문 Flow에는1관문+공통,3관문 Flow에는3관문+공통만 추가한다. Flow는 원본 stable ID를 저장하고 선택된 Flow의 gateId가 실행 관문을 결정한다. Shared source의 수정은 두 관문에 함께 반영되며 복제본을 사람이 따로 관리하지 않는다.

## G02. 게시와 실제 실행

Publisher가 공통 source를 관문별 실행 데이터로 투영한다. 원본 ID와 실행 관문의 조합으로 결정적인 runtime pattern/action/occurrence ID를 만들고 source ID와의 매핑을 inventory에 둔다. Source 문서에 두 번째 패턴을 저장하지 않는다.3관문 기존 ID/참조의 호환을 유지하고1관문용 파생 ID와 target을 함께 만든다. 하위 Pattern/Summon/Logic follow-up 참조도 같은 관문으로 닫힌 graph를 만든다.

MAP 위치, 명시 BossMotion/중앙 teleport와 spawn-facing 방향은 원본 관문 frame에서 실행 관문 frame으로 한 번 변환한다. BOSS bone/local TRS와 player-targeted 값은 다시 변환하지 않는다. 관문 world placement·전용 카메라·HUD mode·마리오 입장 같은 계약을 가진 항목은 공통 승격을 거부한다. 변환은 설치 Resources나 shader를 다시 쓰지 않고 기존 Composition projection에서 수행한다.

관문 frame의 원점과 yaw는 `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`의 해당 stable boss placement 저작 transform을 정본으로 사용한다. 실행 중 움직인 boss Transform을 변환 기준으로 다시 읽지 않는다. 공유 대상의 변환된 MAP/이동 위치가 목표 관문의 navigation과 전투 scope를 만족하는지 검사하며 실패하면 승격·게시를 거부한다. 같은 모델이어도 양쪽 맵 형상이 같다는 보장은 없으므로 평행이동과 yaw 변환만으로 모든 Pattern의 사용 가능을 선언하지 않는다.

Server는 기존 단일 gate·target이 확정된 파생 Product를 소비한다. gate/body/navigation/sequence/revision 검사를 완화하지 않는다. Client animation/presentation은 같은 파생 ID와 transform을 읽으며 Workbench는 source ID mapping으로 원본 편집과 실행 커서를 연결한다. 단독 공통 Complete Play는1/3관문 중 실행 관문을 명시하고 기존 관문 준비 후 audition으로 제출한다.

## G03. 현재 공유 후보와 전용 유지

공유 후보는 무지개댄스 P38, 알비온 P39, 십자화염폭발 P40, 백스텝불뿜기·화염링 P43, 감전빔 P46, 우측이동화염파동 P49, 화염파동 P59와 분신소환 계열 P52/P65/P66 및 실제 참조 child다. P40의 현재 내용은 Animation2개뿐이므로 공통화가 미작성 Effect를 생성하거나 복원 완료를 뜻하지 않는다. BOSS/MAP·중앙이동의 원본·실행 관문 양쪽 수치를 검증하고 사용 가능한 dependency closure만 이동한다.

쇼타임 P35와 마리오 P33/P34·칼날/철창·3관문 전용 World는3관문에 남긴다. 이후 빙글빙글 카드 패턴은1관문 전용으로 작성한다. 사용하지 않을1관문 내려치기/불뿜기/트럼펫은 자동 삭제하지 않으며 사용자가 새 Delete 기능으로 참조를 확인해 정리한다.

## G04. 구현·검증 단위

CompositionDocument의 parse/validate/save·dependency 처리, Workbench 트리·공통화 편집·실행 관문 선택, BossTool inventory/Flow resolver, Python projector의 gate별 graph·TRS 투영과 runtime inventory, 필요한 publisher 검증을 한 변경으로 연결한다. 기존 Server 검사를 통과하는 실제 파생 Product와 Client presentation을 함께 확인한다. 원본 하나 변경→두 관문 실행 변경, 두 Flow의 source stable ID 보존, 잘못된 모델/전용 World 거부, 삭제 참조 보호, failed publish의 기존 출력 보존을 검증한다.

이 문서는 구현 설계이며 공통 분류나 기존 패턴 데이터 이동을 이미 수행했다는 결과가 아니다.
