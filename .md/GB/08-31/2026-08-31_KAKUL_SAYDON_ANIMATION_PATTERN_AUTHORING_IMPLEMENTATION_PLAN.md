# 쿠크·세이튼 Animation Pattern Authoring 구현 계획

## 1. 목표

Animation Tool에서 쿠크·세이튼 추출 액션을 발탄의 Animation Sequence Intake처럼 기획자가 작성한 이름 순서로 탐색하고, 한 액션에 연결된 실제 WModel clip occurrence를 모두 선택·프리뷰할 수 있게 한다.

선택 액션의 유효 clip occurrence를 `Create Pattern`으로 복사한 뒤, 생성된 로컬 패턴에서 clip 선택, 전체 순차 재생, 복제, 삭제, 순서 변경을 제공한다. 이 패턴은 아직 Server Product 쿠크·세이튼 전투 패턴이 아니며 `REFERENCE_ONLY` Animation authoring 문서로만 저장한다.

## 2. 현재 코드·데이터 실측

- 기획 이름과 추출 순서 정본은 `Data/Animation/Reference/KakulSaydon/<Profile>.actionreference.json`이다.
- `MN_RPCT_05`, `MN_RPCT_06`, `MN_RPCT_07`, `MN_RPCZ_00` 네 프로필에 액션 349개, stage 4,072개, 실제 WModel slot 3,692개가 있다.
- `MN_RPCT_07`은 별도 기획 프로필이지만 물리 preview body는 `MN_RPCT_05`를 공유한다.
- `CKakulAnimationActionDocument`는 immutable reference와 sparse slot override를 exact `(sourceActionId, stageId, slotId)`로 join하고 atomic save한다.
- `CAnimation_Tool::Render_KakulActionBindings`는 기획 이름 목록과 한 stage의 slot만 보여준다. 선택 액션 전체 clip 목록, 전체 액션 프리뷰, 패턴 생성과 pattern clip CRUD는 없다.
- 발탄의 Product pattern transaction은 Server gameplay/presentation 계약에 결합돼 있으므로 쿠크·세이튼 로컬 저작 결과를 그 문서나 Server 패턴으로 위장해 넣지 않는다.

## 3. 정본과 실패 소비자

| 역할 | 정본/소유자 | 실패 처리 |
|---|---|---|
| 기획 액션 이름·순서·source slot | `*.actionreference.json` / extraction pipeline | reference/profile/revision/model join 실패 시 기존 Tool draft 유지 |
| slot override | `*.actionbindings.json` / `CKakulAnimationActionDocument` | 기존 atomic save와 rollback 유지 |
| 로컬 패턴 | `*.patternbindings.json` / 신규 `CKakulAnimationPatternDocument` | parse/validate/stage 실패 시 현재 pattern draft와 destination 유지 |
| 실제 pose 재생 | 현재 Animation target의 `CModel` / `CAnimation_Tool` | target generation 또는 clip 교체 시 preview 중단, 기존 문서 유지 |
| Product 전투 권위 | 미구현 범위 | Server gameplay/presentation으로 publish하거나 승격하지 않음 |

## 4. 변경 파일

### 4.1 신규 계약

- `Client/Public/KakulAnimationPatternDocument.h`
- `Client/Private/KakulAnimationPatternDocument.cpp`
- `Data/Animation/Authored/KakulSaydon/MN_RPCT_05.patternbindings.json`
- `Data/Animation/Authored/KakulSaydon/MN_RPCT_06.patternbindings.json`
- `Data/Animation/Authored/KakulSaydon/MN_RPCT_07.patternbindings.json`
- `Data/Animation/Authored/KakulSaydon/MN_RPCZ_00.patternbindings.json`
- `Tools/KakulSaydonPipeline/test_kakul_animation_pattern_document_contract.py`

### 4.2 기존 연결

- `Client/Public/Animation_Tool.h`
- `Client/Private/Animation_Tool.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `Tools/KakulSaydonPipeline/build_kakul_animation_reference.py`
- `Tools/KakulSaydonPipeline/test_build_kakul_animation_reference.py`
- `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`
- 대응 RESULT 문서

## 5. 데이터 계약

`lostark.kakul-animation-pattern-bindings` formatVersion 1은 `profileId`, exact `referenceRevision`, `REFERENCE_ONLY`, `patterns`를 소유한다.

각 pattern은 stable `patternId`, 기획 이름에서 복사한 `displayName`, `sourceActionId`, 하나 이상의 clip occurrence를 가진다. occurrence는 stable `occurrenceId`, source `stageId/slotId`, 실제 `runtimeClip`, `sourceStartMs`, `playMs`, `playRate`, explicit `endPolicy`를 저장한다. `endPolicy`는 `EXACT`, `HOLD_LAST_POSE`, `LOOP_TO_WINDOW` 중 하나이며 native clip보다 긴 source window를 조용히 잘라내지 않는다.

validator는 다음을 fail-close한다.

- 잘못된 schema/version/profile/revision/authority
- 중복 pattern/occurrence ID와 안전하지 않은 token
- reference에 없는 source action/stage/slot
- `REVIEW_CANDIDATE`가 아닌 HOLDOUT source action
- 현재 WModel에 없는 runtime clip
- 비어 있는 pattern, 범위를 벗어난 timing/play rate
- 크기와 개수 상한 초과

Save는 `validate -> sibling temp durable write -> strict reparse/exact compare -> reference 재검증 -> atomic replace` 순서로 수행한다.

## 6. Tool 흐름

1. 프로필을 열면 action reference, sparse override, pattern document를 모두 stage한다.
2. 세 문서가 exact profile/revision/model join을 통과한 뒤에만 UI 상태를 commit한다.
3. 왼쪽 목록은 원본 action 순서와 기획 `displayName`을 그대로 유지한다.
4. 선택 액션의 모든 stage/slot을 source 순서대로 평탄화해 clip 목록으로 표시한다. sparse override가 있으면 effective clip/timing을 사용한다.
5. clip 행 선택은 해당 실제 WModel clip을 즉시 preview한다.
6. `Preview Action`은 평탄화한 전체 clip을 source timing 순서로 재생한다.
7. `Create Pattern`은 이 effective occurrence들을 새 stable pattern/occurrence ID로 복사한다.
8. Pattern 목록에서 선택한 패턴은 `Play Pattern`, `Duplicate Pattern`, `Delete Pattern`을 제공한다.
9. Pattern clip 목록은 선택 preview, `Duplicate Clip`, `Delete Clip`, `Move Up`, `Move Down`을 제공한다. 마지막 한 clip 삭제는 막고 패턴 삭제를 안내한다.
10. 패턴 preview는 현재 target generation과 실제 model clip을 계속 검증하며, source start/play window/play rate/end policy를 적용한다.

## 7. 구현 단위

### G1. Pattern document와 atomic persistence

- strict parser/validator/load/save와 네 프로필 empty canonical 문서를 추가한다.
- reference revision과 source identity를 exact join한다.
- 새 C++ 파일을 프로젝트와 기존 Animation 물리 filter에 등록한다.

### G2. 기획 액션 전체 clip 목록과 선택 preview

- 현재 한 stage slot UI는 유지한다.
- 선택 액션 전체 effective occurrence 목록을 추가하고 source 순서, stage/slot identity, clip timing을 표시한다.
- 개별 clip 선택과 전체 액션 순차 preview를 실제 WModel에 연결한다.

### G3. Create Pattern과 clip CRUD

- action에서 monotonic `nextPatternOrdinal`/`nextOccurrenceOrdinal`로 stable ID를 생성하고 삭제 뒤에도 ID를 재사용하지 않는다.
- pattern/clip duplicate/delete/reorder를 vector mutation 후 validation 가능한 draft로 유지한다.
- pattern 변경은 별도 dirty 상태로 관리하고 target switch/reload에서 보존한다.

### G4. 검증·문서·인계

- 정상 fixture, 잘못된 version/profile/revision/source ID, 중복 ID, invalid clip/timing, atomic save token을 focused harness로 검증한다.
- Kakul pipeline focused tests, JSON parse, Client Product build, `git diff --check`를 실행한다.
- 화면 결과는 사용자가 Animation Tool에서 직접 판정한다.

## 8. 완료 조건

- 네 프로필이 기획 이름 순서로 로드되고 선택 액션 전체 실제 clip이 나열된다.
- 개별 clip과 전체 action/pattern preview가 현재 physical body에서 재생된다.
- Create Pattern 뒤 clip duplicate/delete/reorder와 pattern duplicate/delete가 동작한다.
- 저장 실패 또는 stale reference에서는 기존 draft와 destination이 보존된다.
- 어떤 저장 결과도 Server Product 패턴 또는 gameplay authority로 승격되지 않는다.
- focused harness, 관련 Kakul pipeline test, Product build와 diff 검사 결과를 RESULT에 실제 실행 증거로 기록한다.
