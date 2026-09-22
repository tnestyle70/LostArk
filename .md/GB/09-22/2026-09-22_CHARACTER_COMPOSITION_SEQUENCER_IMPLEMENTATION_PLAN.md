# Character Composition Sequencer 통합 구현 계획서

작성일: 2026-09-22. 기존 `CharacterActionWorkbench`, `CharacterModelWorkbench`, `EffectAuthoringSequencer`와 skillbindings / animevents 저장 owner를 확장한다. 별도 runtime을 만들지 않는다.

## G00. 현재 실측과 목표

Character Actions는 일곱 class의 PlayerSkills를 나열하지만 별도 read-only Effect 행을 표시하고 Effect append 기능이 없다. Clown / interaction은 공용 Effect sequencer를 사용하나 공통 Animation browser의 append는 Character binding 검사 때문에 차단된다. 기존 Product cue owner와 binding writer의 baseline 검사를 유지한다.

## G01. CharacterActionWorkbench의 선택과 저장

기존 action을 클릭하면 공용 editable sequencer를 준비한다. key/skill catalog로 Create Skills에서 실제 슬롯과 clip을 골라 presentation binding을 만든다. 단계 수와 입력 key는 PlayerSkills가 정하며 UI가 별도 gameplay skill을 만들지 않는다. Composition Save는 기존 skillbindings와 animevents owner에 선택 action의 clips/Effect/Sound를 검증하여 반영한다. 기존 Combat 편집은 같은 HitShapes owner로 유지한다.

## G02. CharacterModelWorkbench와 공용 row 편집

Clown, Mario, maze, mount, monster의 resource append를 현재 모델 기준 공용 sequencer에 전달한다. interaction은 한 clip을 유지하고 optional effectCues 배열의 여러 Effect 박스·source 시작/끝·anchor·TRS·follow/stop/orientation을 기존 Product binding writer로 저장한다. Character.cpp의 실제 Server action-age 소비자는 기존 단일 effectAssetId를 계속 읽고 배열은 cue별로 준비·재생·종료한다. 새 JSON 저장형식을 Client reader와 같은 변경에서 연결한다. CUE_END의 action owner 검사와 늦은 준비·다음 action의 pending 취소를 유지한다. source에서 없는 Effect를 추가하고, 편집 중 timeline을 보존하며 Save/선택이 같은 상태를 사용한다.

## G03. 검증

수정 TU의 Debug 컴파일, JSON parse와 git diff --check를 확인한다. 기존 파일만 변경하므로 project/filter 항목 추가는 필요 없다. 실제 Client/UI 실행과 화면 판정은 사용자가 직접 한다. 저장된 Product와 별도 authoring arrangement의 적용 범위를 RESULT에서 구분한다.
