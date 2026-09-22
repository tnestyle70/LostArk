# Monster Composition 및 피해 전용 공격 구현 계획

## G00. 현재 코드와 범위

`CharacterActionWorkbench::Render_Actions`는 Character Actions를 SeparatorText로 항상 표시하고,
`CharacterModelWorkbench`가 Clown/interaction/mount를 별도 SeparatorText로 표시한다.
기존 모델 preview와 `EffectAuthoringSequencer::Stage_CharacterAction`은 모델 clip과
서버 형태를 그리는 collider row를 이미 함께 처리한다. 이 경로를 그대로 확장한다.

대상은 MonsterCatalog의 발탄 일반 4종·루가루, 쿠크 시작 구역 4종, 카드미로 병정 4종이다.
모델 clip/배율은 MonsterCatalog와 설치 WModel, 충돌 반경·공격 범위·시점은 MonsterProfiles를 읽는다.
일반 공격은 MonsterBrain의 `collisionRadius + attackRange` 원형이며 첫 active tick에 한 번 판정한다.
카드미로 병정은 별도 미니게임이 행동과 접촉을 소유하므로 dormant 기본 공격을 제품 공격처럼 표시하지 않는다.

## G01. Character 및 Monster 목록

- `CharacterActionWorkbench.cpp`: Character Actions 접기/펼치기, Monster 목록 호출을 추가한다.
- `CharacterModelWorkbench.h/.cpp`: Monster catalog action을 읽고 모델 clip 선택 시 공용 sequencer에 body/attack collider를 stage한다. 저장은 preview sequence만 허용하며 MonsterProfiles의 제품 수치를 숨겨서 덮어쓰지 않는다.
- `AnimationPreviewAssets.h`, `CompositionAnimationResource.h`: 각 archetype의 실제 모델/배율로 등록하고 기존 arena preview admission과 physical clip browser에 연결한다.
- 새 C++ 파일은 없으므로 vcxproj/filters 등록 변경은 없다.

## G02. 피해 전용 서버 공격

대상 13개 MonsterProfiles의 공격 push/duration/knockdown/down duration만 0/0/false/0으로 만든다.
몬스터 자신이 맞을 때의 hitKnockbackScale, Valtan/Kouku boss 공격 수치는 유지한다.
기존 publisher로 검증·게시하며 다른 저작 변경을 보존한다.

MonsterBrain의 일반 공격은 현재 HP 직접 차감과 공통 Apply_WorldToPlayer를 중복 호출한다.
직접 차감/이벤트/반응 블록을 제거하고 공통 피해 처리 한 번만 유지한다.

## G03. 검증

기존 spawn group contract의 공격 반응 기대값을 새 프로필에 맞추고, 일반 몬스터 한 번의 공격이
피해 한 번만 발생시키는 기존 focused 검증을 사용한다. 대상 모델/clip 존재, JSON parse,
publisher Validate, diff whitespace를 확인한다. 제품 빌드는 상위 통합 작업에서 수행한다.
Client/UI 실행·화면 판정은 사용자에게 맡긴다.
