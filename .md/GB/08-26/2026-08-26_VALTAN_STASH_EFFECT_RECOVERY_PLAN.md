# 발탄 stash Effect 선택 복구 계획

## 1. 문제

2026-08-26 07:11 KST의 pull 준비 과정에서 dirty worktree가 safety stash
`7ec4d3b69b5ed6f67d275722b2c01a077d0e86cf`로 보존된 뒤, 현재 브랜치가
`8e87e5cc`로 fast-forward됐다. stash가 다시 합쳐지지 않아 사용자가 전날 저장한
발탄 Effect와 FLOOR_WIPE presentation이 현재 정본에서 보이지 않는다.

## 2. 복구 원칙

- 전체 stash를 apply/pop하지 않는다.
- tracked 최신 저작은 고정 stash commit에서, 당시 untracked 신규 파일은
  `7ec4d3b6^3`에서 가져온다.
- 현재 main에 이미 들어간 `presentationScale=0.75`, cue scale policy,
  nullable `sourceEndMs` 수정은 유지한다.
- 같은 파일의 비관련 최신 main 변경을 과거 stash 파일 전체로 되돌리지 않는다.
- 복구가 끝난 변경은 한 커밋으로 고정하고 safety stash와 보호 브랜치는 삭제하지 않는다.

## 3. 구현 범위

1. 도넛, 4연격, HIGH_JUMP TAKEOFF, 회오리, 하늘 도끼 authored Effect 최신본 복구
2. FLOOR_WIPE 전체 패턴용 빈 Effect와 Catalog/Data project 등록 복구
3. FLOOR_WIPE animation을
   `5_02_loop -> 5_02_end -> 5_02_loop -> 5_02_end`로 복구하고 Product 재생성
4. ARENA_BREAK_109 WIDE_REVEAL의 포효 start/loop occurrence 복구
5. HIGH_JUMP 중앙 착지 수동 Element 문서를 독립 저작 가능 상태로 복구
6. split authoring의 단수형 `SPAWN_COMBAT_OBJECT` strict parser 복구
7. 애니메이션 작업자의 `Valtan.presentation.debug.json`이 Product에 안전하게
   승격 가능한지 조사하고, 불완전한 target/timing은 임의 병합하지 않기

HIGH_JUMP 중앙 착지의 Server combat-object owner 전체 묶음은 현재 main gameplay를
과거 상태로 되돌리지 않고 별도 수직 슬라이스로 남긴다. 이번 복구에서는 사용자 수동
Element를 잃지 않도록 authored source, Catalog, project 등록까지만 닫는다.

## 4. 검증

- 고정 stash blob과 복구 파일 hash 일치
- JSON/XML parse와 Effect Data project sync
- Valtan PublishV2/ValidateV2와 runtime-set Validate
- Valtan pattern tree, animation tool, v2 transaction harness
- Effect Tool saved-row focused regression
- Client C++ compile과 `git diff --check`
