# 쿠크 저작·패턴·렌더링 정본 구현 계획

## G00. 현재 기준과 보호 경계

현재 브랜치는 `codex/kouku-timeline-local-preview`, 시작 HEAD는 `1ddbcffc016144edb8c8120c4164f68668fdec40`이다. 기존 미커밋 변경을 보존한다. 사용자 최종 저장본과 stable ID를 기준으로 필드별 병합하며 자동 Client 실행·Reload·화면 판정은 하지 않는다.

팀장이 조율한 렌더링 옵션 전체가 정본이다. 현재 RenderingProfiles revision82의 쿠크 base와 source profile FXAA=false를 유지하며 Mario1~4에 AA를 다시 켜는 복원·일괄 덮어쓰기를 금지한다. AGENTS에는 권한 경계, CLAUDE에는 실제 저장·게시 경로를 적는다.

## G01. World Object 재생과 피해 표시

WorldObjectTool의 기존 previewAtCharacter 경로를 재사용하는 `Play at Player`를 단일·합성 Motion transport에 추가한다. Composition에서 열었을 때 저장 위치 모드로 바뀌어 0,0,0에서 보이지 않는 문제를 명시 동작으로 해결한다. 저작 Transform은 변경하지 않는다.

ServerCombatHitRuntime에서 실제 shield 흡수량을 typed DAMAGE_EVENT로 보낸다. Shared DAMAGE_HIT_FLAG에 ABSORB를 추가하고 protocol을 올린다. Client MainApp은 파란 `흡수` 문자열을 표시하며 CombatHUDViewModel은 이를 HP 피해 분석에 합산하지 않는다. 부분 흡수는 잔여 HP 피해와 별개 표시한다.

## G02. 연결 작업

Box Detail의 duration/result 튜닝과 카드비 실수 정밀도는 KOUKU_RESULT_TUNING 계획에, Server의 반복 틱·빙고·추적은 KOUKU_PATTERN_RUNTIME 계획에, 잔상·매두사·카드·재질 복원은 KOUKU_EFFECT_RECOVERY 계획에 기록한다. 데이터 최종 병합과 publish는 이 작업이 한 번에 조율한다.

빙고는 첫 mark30초, 이후20초 간격, mark6초→공백2초→바닥폭탄4초를 기준으로 한다. 사용자 후속 지정은 이동→블랙홀 첫 애니메이션 클립→매두사 종료→블랙홀 본 애니메이션·이펙트13초→폭발을 작은 Parent/Bundle로 연결하는 것이다. 전체 빙고 반복 Parent는 제거한다. 세 번째 폭탄은 표식 후12초에 정상 폭발한다. 그때 판 뒤집기 이후 빨간 가로·세로 합계3줄 이상이면 플레이어30초 무적 버프를 준다. 블랙홀 폭발 시 성공은 보스13줄 감소·플레이어생존, 미완성은 전원전멸이다. 대각선은 완성 줄 수에 포함하지 않는다.

## G03. 검증

신규 C++ 파일을 만들지 않아 프로젝트 등록 변경은 없다. 기존 Shared codec·Server contract에서 전부/부분 shield, 피해·광기 보존, duration 반복과 반복 종료를 확인한다. 변경 JSON parse와 publisher Validate/Publish, diff check, Debug 정상 증분 Product Build를 실행한다. 실제 입력·저장·UI 및 시각 fidelity는 사용자 확인으로 구분해 RESULT에 남긴다.

## G04. 전체 수명 본 Collider의 게시 용량

최종 projection의82,441행 중 WORLDKEY는73,999행이다. 기존 track150개는 유지되며, 요청한 인형 양쪽 입20개의 전체 수명 본 궤적이39,132행을 추가한다. 동일 pose의 인접 key는30Hz tick의 floor/ceil 보간 경계를 보존하는1ms 간격이며 중복 시각·역순·신규 exact duplicate track이 아니다. 임의 샘플 삭제로 회전을 바꾸지 않는다.

Shared의 유한 row 상한을131,072로 조정하고 publisher·Server·Client 및 Python parser를 같은 계약으로 연결한다. 파일은64MiB 상한으로 제한하며 Publisher는 실제 UTF8 직렬화 bytes, Server는 파일 읽기와 bytes admission, Client는 기존 artifact admission에서 확인한다. format37과 protocol111은 유지한다. 개별 track4096keys, field 검증, 정확한 행 수와 후행 행 거부, 실패 시 기존 catalog 보존 및 Debug draft의 별도16MiB 제한을 유지한다.

## G05. 빙고 전투 묶음의 명시적 연결

사용자가 다시 지정한 자동 시작 시점은 일반3관문 입장이 아니라 빙고 페이즈 진입이다. 일반 반복 Flow와 이동→첫 클립→메두사→블랙홀 Parent를 같은 전투 묶음에서 관리한다. 여러 actor를 동시 재생하는 기존 Bundle 계약을 바꾸지 않고 BINGO Pattern Flow에 optional `bingoSpecialPatternId`를 저장한다. 설치 후보는 `kakulsaydon.flow.bingo`에서 `KAKULSAYDON_G1_PATTERN_107`을 참조한다.

Client load/save/검증 및 Flow 편집은 같은 stable 참조를 사용한다. 초안의 미지정 상태와 제품 게시 가능 상태를 구분하며, BINGO 제품에는 같은 gate/encounter/boss의 유효한 특수 Parent가 필요하다. publisher의 supplemental `RAIDBINGOSPECIAL` 행과 Server gate definition이 참조를 전달한다. Server는 패턴 전체를 검색해 임의 선택하지 않고 게시된 참조를 정확히 조회한다. 누락·다른 gate·잘못된 Parent·중복 폭발은 게시/admission에서 거부한다.

빙고 입장 후 보드·망치·폭탄의 encounter 시계를 시작한다. 세 번째 머리 위 표식마다 일반 패턴의 occurrence와 남은 결과를 정리한 뒤 특수 Parent를 실행하며 보드·폭탄 시계는 유지한다. 특수 Parent 완료 뒤 중단했던 일반 entry를 처음부터 재생하고 저장된 반복 순서로 진행한다. 반복 entry와 특수 Parent의 boss 제어권은 겹치지 않는다.

필드 추가는 최신 디스크 revision에 stable Flow ID로 병합한다. 기존 승인된 현재 저장본 반영 범위에서 writer lock·hash 재검사·백업·원자적 교체를 유지한다. Client/UI의 자동 Reload는 하지 않는다. 정상 Product 증분 Build, 저장/재개·유효 참조/거부 테스트, 실제 게시본의 세 번째·여섯 번째 표식 interruption 및 반복 복귀를 검증한다.

## G06. 2·3관문 체력별 반복 그룹

기존 G1의 entryGroups, repeatUntilHealthBars, PATTERN_END 계약을 G2/G3에 적용한다.
체력 경계에 도달하면 현재 패턴과 소유한 후속 패턴을 완료한 뒤 체력 기믹으로 전환한다.
일반 반복은 다음 체력 경계까지, 기믹 그룹은 한 번씩 진행하며 마지막 일반 그룹은0줄까지 반복한다.
기존 Server 실행기를 재사용하므로 새 C++·Shared 필드·프로젝트 등록은 없다.

G2는140줄에서 시작해125 등장(Bundle1),110 파1빨2(Bundle2),95 조커(Bundle3),
80 카드미로(P77),55 피자(Bundle5),25 조커(Bundle3)를 연결한다. 일반 순서는
P105→106→105→85→15→21→24→99→86→87→27이다. 09-22 GATE2_PATTERN_RESTORATION
RESULT의 사용자 확정과 현재 Action4219713을 근거로 3갈래 바닥 폭발과 나팔을 같은P105로
연결하되 요청한 두 위치를 보존한다. P23 팡파레로 대체하지 않는다.125줄 등장 전은 앞7개,
등장 후는11개를 반복한다. 기존 stage·Bundle members·SOUND를 바꾸지 않는다.

G3는180줄에서 시작해155 P88,125 P91,90 P76→P35 쇼타임,80 P92,55 P93을 연결한다.
사용자가 마지막에 정리한 공통10개 P52→119→59→116→118→39→38→117→43→40을
각 체력 구간에서 반복한다. 기존 Mario Parent 자식과 P33 자동 후속은 보존하며 Flow에
P33을 중복 배치하지 않는다. P116의 기존 logic.2를 시작0ms의 BOSS_TRACK_TARGET Trigger로
연결한다. P119의 MAP Effect4개·Collider3개는 이미 G3 중심Z936.3143603에 있어 유지한다.
시작 발차기·오허·공3개 연출은 현재 저장본의 직접 대응 패턴이 확인되지 않아 이 변경에
임의의 다른 공 패턴을 넣지 않는다. 별도 원본 조사 결과를 RESULT에서 구분한다.

최신 저장본 stable entry ID·대기시간·무관한 관문을 보존하고 obsolete loopStartEntryId만
entryGroups로 대체한다. 실제 document/flow validator, 멱등성·무관 필드 보존,
HP projection 테스트와 게시 후 source revision·RAIDGROUP·Retail 체력·Client admission을
검증한다. writer lock·hash/CAS·백업·원자 교체 후 기존 KoukuSaydon owner로 한 번 게시한다.

## G07. Complete Play의 분열 공 WORLD 준비 거절 수정

사용자 화면에서 GATE1 Complete Play가 `world.object.kouku.saydon.circus.split`의
admitted motion이 없다는 이유로 준비 단계에서 거절된다. 실제 게시된 g0~g3는 enabled
WORLD/STOP 모션과 실제 model binding을 가지며, 폭발용 colliderTracks가 있다는 조건만
`CompositionWorldMotions`의 기존 visual group 검사에 걸린다.

`Client/Private/Level_KakulSaydonArena.cpp`의 공통 그룹 해석에서 colliderTracks의
존재만으로 거절하는 조건을 제거한다. walkableSurface, combatBody, 잘못된 binding,
중첩 group, WORLD 이외 anchor, STOP/LOOP 이외 종료 정책의 거절은 유지한다.
Collider는 기존 publisher의 Server geometry bake와 Client의 본 검증·Debug 표시를
소비하며, 이 수정으로 Client 전투 판정이나 navigation 경로를 추가하지 않는다.
Complete Play 준비와 실제 Prepare/Play/Span/Pivot가 같은 해석을 사용한다.

기존 C++ 파일의 인코딩·개행과 다른 작업 diff를 보존한다. 신규 C++ 파일 및 프로젝트
등록 변경, 데이터 수정·재게시 없이 실제 게시 group과 거절 경계 검증, 정상 Debug
Product 증분 Build를 수행한다. 실행 중 Client/Server의 파일 점유가 있으면 코드와
검토 가능한 검증 후보를 먼저 끝내고 최종 링크 시점에만 종료를 안내한다.
실제 Complete Play 화면 재확인은 사용자가 수행하며 자동 검사와 구분한다.

## G08. 분열 공 폭발의 원기둥·최대 HP 10% 피해

사용자 최종 요청은 분열 공의 원형 폭발에 맞춘 CYLINDER와 기존 최대 HP 10% 유지다.
각 emission의 독립 ENTER_AREA 창은 그대로 두므로 같은 폭발은 플레이어별 한 번,
다른 폭발은 겹친 개수만큼 각각 결과를 적용한다. Server의 MAX_HP_PERCENT_DAMAGE와
기존 피해 소비자를 유지한다. 방어력은 무시하고 받는 피해 감소 버프·보호막·무적은 적용된다.

WORLD Collider track에 optional `shape`(기본 BOX, 추가 CYLINDER)를 연결한다.
CYLINDER의 halfExtents는
`[radius, halfHeight, radius]`이며 XZ 일치를 검사한다. HOOK_CAPTURE는 BOX만 허용한다.
behavior=DAMAGE는 기존 damagePercent 1~100을 사용한다. 기존 생략 필드와 BOX 저장본의
동작을 보존한다. 새 JSON field를 Client load/save/validation/equality, World Object Tool
편집·원기둥 Debug 표시, Map publisher, Python collider bake와 실제 Result 소비까지 연결한다.

Server는 기존 CYLINDER의 반경·높이 및 worldTrack, MAX_HP_PERCENT_DAMAGE 결과를 이미 지원한다.
별도 판정 경로와 protocol을 추가하지 않는다. Source Collider의 기존 위치·크기·34ms 창,
1.5초 분열 시계·공별 독립 ID·피해 수치는 유지하고 다섯 트랙의 shape만 변경한다.
첫 공의 최초 bounce도 현재 같은 폭발 이펙트/피해 트랙이므로 동일한 CYLINDER를 적용한다.
현재 저장된 반경·반높이는 기존 X/Y half extent와 설치 Transform을 기준으로 유지한다.

최신 source hash를 읽어 stable track ID 기준 후보를 만들고 전체 모션·이펙트 및 무관한
사용자 변경을 보존한다. 최종 설치는 백업·hash 재검사·원자 교체·실패 rollback을 사용하며
현재 authoring 상태에 맞춰 Map 및 Kouku product/gameplay를 공식 owner publisher로 게시한다.
Client/UI가 편집 중이면 후보·검증을 완성한 최종 교체 시점에만 저장·반영 기준을 확인한다.

기존 BOX backward compatibility, Cylinder 가장자리·높이·서로 다른 폭발 중첩과 같은 창의
중복 금지, 최대 HP 13,200에서 단일 1,320·중첩 2,640 피해, 실제 저장/게시의16개 독립 CYLINDER 창 및
결과값을 확인한다. 변경 C++ 정상 Product Build, JSON/XML parse, diff check를 수행한다.
추가 C++ 파일은 계획하지 않으며 기존 프로젝트 등록을 사용한다. 실제 폭발 화면은 사용자 확인이다.
