# 쿠크 인형 화염·즉사 칼날·카드 미로 교정

## G00. 현재 근거와 변경 경계

사용자는 인형 화염 Collider 수정과 회귀 검증을 승인했다. 현재 큰 인형은
WORLD Effect track의 Y90도 회전을 사용하는 반면 본 Collider는 그 부모 변환을
사용하지 않는다. 설치 MN_CDMD_00의 실제 앞뒤 입 본과 게시 WORLDKEY를 대조하면
회전 속도와 부호는 같고 전방이90도 어긋난다. 불 중심선5m 지점도 기존 BOX 밖이다.
P34/88/89/91/92/93의 인형 WORLD10개, 앞뒤 Collider20개가 같은 계약을 사용한다.
공10개와 광기 증가량·100ms 접촉 주기, 사용자 Effect·렌더링 값은 보존한다.

대규모 기존 미커밋 변경을 보존한다. 전체 파일 되돌리기·자동 stage/commit은 하지 않는다.
Client/UI 실행과 화면 판정은 사용자가 담당한다.

## G01. Collider의 명시적 WORLD Effect 좌표계

기존 Composition presentation occurrence에 optional `worldEffectTrackId`를 추가한다.
빈 값은 현재 경로를 유지한다. 값이 있으면 같은 WORLD occurrence의 단일 OBJECT_RESOURCE
slot에 연결된 V1 Effect track을 stable ID로 찾는다. Collider는 WORLD·following·BODY 본·
BONE 회전이어야 하고 Effect는 followObject=true, inheritObjectRotation=true, 자체 bone이
빈 경우만 지원한다. 누락·중복·다른 slot·지원하지 않는 attachment는 실패로 처리한다.
WORLDKEY의 지면 박스 계약에 맞게 참조 Effect root는 pitch/roll0과 균일한 양수 scale만 허용한다.

publisher는 기존 본 bake에서 `Bone * EffectTrackTRS * ObjectWorld`를 center와 forward에
동일하게 적용한다. 기존 WORLDKEY 출력과 Server reader를 재사용한다. Client의
Try_GetObjectPivot/Try_GetSequencePivot과 Level/Presentation 전달 경로도 같은 순서를
사용한다. authoring parse/save, Product parse, editor copy/equivalence/reset, preview edit와
validation을 함께 연결하여 저장·재로드 뒤에도 연결을 잃지 않는다.

수정 파일은 KoukuSaydonCompositionDocument.h/.cpp, KoukuSaydonPresentationPlayer.cpp,
KoukuSaydonActionWorkbench.cpp, WorldSequencePlayer.h/_Objects.cpp,
Level_KakulSaydonArena.h/.cpp, project_kouku_saydon_composition.py와 필요한 기존 회귀 파일이다.
신규 C++ 파일은 없으므로 vcxproj/filters 항목을 추가하지 않는다.

## G02. 현재 저장본 적용과 게시

큰 인형 Collider20개에 `worldEffectTrackId=effect.doll.flame`만 명시한다.
Collider의 원래 offset·크기·회전·수명과 공 설정을 보존한다. 최신 디스크 문서를 stable ID로
병합하고 교체 직전 hash 확인, 백업, 원자 교체와 실패 시 자기 변경 rollback을 유지한다.
기존 domain publisher로 Encounter·Client presentation·Server bootstrap을 생성한다.
게시 성공과 실행 중 프로세스의 반영, 사용자 화면 확인을 구분한다.

## G03. 회귀와 완료 조건

기존 검사에서 빠졌던 실제 설치 모델의 앞뒤 입·여러 회전 시점·17.314초 반복 경계를 검사한다.
수정 전90도/불축5m 미접촉을 재현하고 수정 후 방향 일치와 접촉을 확인한다.
기존 잘못된 방향의5m 지점은 미접촉이어야 한다. 메모리에서 Effect yaw를 다른 각도로
변경해도 Collider가 동일하게 따라야 하며 공과 연결 없는 본 Collider는 기존 결과를 유지한다.
잘못된 stable ID와 지원하지 않는 anchor는 저장/게시 실패 시 기존 항목을 보존한다.

기존 Python 본 bake·cache·원형 콜라이더 회귀, Client 저장/preview 관련 검사,
Debug/Release 정상 증분 Product Build와 관련 Server contract를 실행한다.
변경 JSON/XML parse와 git diff --check를 확인하고 실제 실행 결과만 RESULT에 기록한다.
화면에서 불 안/밖 광기 판정은 사용자의 최종 확인 범위다.

## G04. 즉사 칼날 이동 속도 절반과 종점 보존

사용자가 추가로 즉사 칼날 이동 속도를1/2로 요청했다. 현재 사용되는 P33.world.5의
world.39/mario_phase2_instant와 P95.world.1의 world.38/instant_death를 대상으로 한다.
둘의 Object template 수명11000ms를22000ms로 늘리고 위치키 시각만2배, 일정 velocity는
1/2로 바꾼다. 위치키 값과 끝 위치, 회전 속도1440도/초, 발사 개수·배치·시작 시각은 유지한다.
Effect·Collider의 끝 시각을 같은22000ms로 늘린다. Sound 시작 시각은 유지하며 해당
지속 sound의 끝 시각만 연장한다. P33은 현재27830ms 안에 끝나며, P95는 전용 테스트
Pattern의 종료가 새 칼날 수명을 자르지 않도록22000ms까지 늘린다.
현재 사용하지 않는 world.48/cage_second와 일반 DAMAGE 칼날 world.19는 보존한다.
구형 prepare_mario_world_blades의8발·옛 종점 재생성은 사용하지 않는다.

수정 전후 동일 진행률에서 위치가 같고 실제 시간 대비 이동량이 절반인지 검사한다.
끝 위치·한발 STOP/반복 정책·표시와 즉사 Collider의 수명·Pattern 종료 이후 잘림 여부를 확인한다.

## G05. 카드 미로 탈출 문양의 랜덤 배치

현재 On_TargetHit는 목표 병정 처치 완료 시 병정 위치를 exitXYZ로 저장하고 flags4를 켠다.
이 처치 분기는 kills 완료만 확정하도록 바꾼다. Server Update_CardMaze가 목표 달성·출구
미배치 상태에서 기존 Sample_Corridor를 사용해 최초 문양과 같은 미로 범위·navigation·
중앙 접근 경로 검증을 적용하고, 선택된 exitXYZ와 flags4를 함께 확정한다.
기존 플레이어·병정과의 이격에 활성 탈출 문양 간 이격을 추가한다. 샘플 실패는 kills를
유지한 채 다음 tick에 재시도하며, 이미 확정한 출구 위치는 다시 뽑지 않는다.

KoukuSaydonLogicRuntime.cpp의 처치 분기/샘플러, GameRoom_KoukuMiniGames.cpp의
Server tick 배치, 기존 ServerGameplayContractTests_CardMaze.cpp를 수정한다.
Client는 기존 snapshot exitXYZ 표시와 문양 진입→중앙 이동 계약을 그대로 소비한다.
새 메시지나 C++ 파일 없이1~4인 완료, 중복 생성·겹침·경로 실패·재시도·중앙 이동과
최종 귀환을 기존 card-maze contract로 확인한다.
