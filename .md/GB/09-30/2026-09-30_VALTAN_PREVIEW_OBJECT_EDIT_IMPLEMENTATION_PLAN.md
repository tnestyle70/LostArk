# 발탄 발악 Preview와 전투 오브젝트 편집 구현 계획

## G00. 기준과 보존 범위

기준 HEAD는 `4fce79521`, 작업 브랜치는 `codex/valtan-authoring-and-entry-20260930`이다.
사용자가 EXE를 종료하고 전체 반영을 요청했다. 기존 STEP_11 이펙트 위치와 atk_08_04 요소 삭제는 보존한다.
Client/UI는 실행하지 않으며 Debug/Release 빌드와 typed 데이터 소비 검증 후 화면 확인 경로를 제공한다.

## G01. CValtan의 Preview 앵커

`Stage_LocalPatternAuthoringPreview`는 LEAP serverMotion이 있을 때만 아레나 중심을 읽는다.
발악의 ARENA_CENTER volley는 serverMotion=null이므로 검증에서 실패한다.
실제 staged effect/volley/aim이 중심을 요구하면 canonical boss placement를 읽고 기존 transaction에 넣는다.
landing snapshot은 LEAP 전용 검증을 유지한다. 잘못된 anchor와 원본 읽기 실패를 우회하지 않는다.

## G02. Composition Object의 발탄 owner 연결

Object에서도 Boss selector를 제공하고 Valtan은 기존 CValtanActionWorkbench session에 연결한다.
Valtan combat object는 Kouku worldsequence로 복사하지 않는다. 선택한 Pattern/Stage의 stable object ID로
동일 Balance draft, Save, Play Preview, Publish를 사용한다. Object 목록은 기존 summon occurrence를 선택한다.

## G03. 폭발 시계와 Collider

기존 rock의 CIRCLE 피해·넉백은 Server 데이터에 존재하지만 초기 Summon Detail 반환 때문에 가려져 있다.
joined hit view에 피해 profile·push·knockdown을 보존하고 해당 Detail에서 radius, hit clock,
owner-hit chain delay와 knockback을 편집한다. typed patch는 실제 소환 owner와 hit ID를 검증하고
복제 source에 적용한 뒤 기존 검증/원자 저장으로 넘긴다. Sound와 hit Effect는 같은 Server hit event를 유지한다.
Preview의 world collider는 각 실제 돌 위치와 동일한 타임라인으로 표시하며 피해 판정은 하지 않는다.

## G04. 연출과 검증

현재 발악 마지막 STEP_11/12 뒤에는 별도 `VALTAN_GHOST_DEATH_AUDITION`(23000ms),
이어서 `VALTAN_GHOST_RESPAWN_AUDITION`(3000ms)이 연결돼 있다. 별도 연출임을 표시하고
각 패턴을 독립 Preview로 확인할 수 있게 한다. 요청하지 않은 clip/길이 삭제는 하지 않는다.
기존 C++ 파일만 확장하여 vcxproj/filters 신규 등록은 없다. typed patch 저장/재로드·실패 보존,
JSON parse, 관련 CPU 검증, Product Build와 diff check 결과는 RESULT에 실제 실행 기준으로 기록한다.
