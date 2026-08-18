# 2026-08-18 발탄 1~67 source-sequence 패턴 프리뷰 구현 계획

branch: `codex/valtan-wall-contact-destruction-and-pillars`

기존 1~67 프리뷰는 clip 이름과 0.5~0.9초 재생 시간을 직접 저장해 실제 타격 동작에
도달하기 전에 다음 단계로 넘어갔다. 잘못 조합한 직접 clip 목록을 폐기하고,
`Valtan.clipseq`의 원본 action·sequence를 정확히 참조하는 검토용 프리뷰로 교체한다.

## G00. 범위와 정본

- 제품 `Valtan.patternbindings.json`, Server 패턴 선택, damage 판정, 월드 연출은 변경하지 않는다.
- `Valtan.patternpreview.json`은 Animation Tool의 1~67 검토 순서와 source sequence 참조만 소유한다.
- JSON에는 직접 `clips`, `previewMs`, `quick`을 저장하지 않는다.
- 각 selector는 `(sourceActionId, sequenceIndex, repeat)`만 저장하고 실제 clip 순서는
  `Data/Animation/Reference/Valtan/Valtan.clipseq`에서 strict join한다.
- 원본 영상과 branch까지 확정하지 못한 행은 `CANDIDATE` 또는 `UNRESOLVED`로 남긴다.
  새 근거 없이 `SOURCE_EXACT`를 사용하지 않는다.
- 분신·투사체·환경 오브젝트가 병렬로 동작하는 패턴을 발탄 body 단일 playlist에 직렬로
  합성하지 않는다.

## G01. 문서와 Tool 계약

- 문서는 정확히 1~67 행, exact property set, confidence enum, source selector 범위,
  source sequence 중복과 실제 model clip 존재를 parse -> validate -> stage -> commit으로 검증한다.
- 모든 resolved clip은 non-loop로 시작하고 `Duration / TickRate`의 native 전체 길이만큼 재생한다.
  임의 hard cap으로 타격 전 동작을 자르지 않는다.
- 애니메이션이 없는 phase·world marker는 idle 상태에서 450ms 표시한 뒤 진행한다.
- scheduler는 ImGui `Render()`가 아니라 Debug tick의 `Update()`에서 진행한다. 창을 접어도
  진행되며, Animation Tool 비활성화 또는 target generation 변경 시 안전하게 중단한다.
- 프리뷰 재생 중 일반 Playback·Chain·Reference·Clip 선택은 잠그고, 현재 clip이 외부에서
  교체되면 기존 타이머를 계속 쓰지 않고 중단한다.
- 0.5~3.0배속은 model과 scheduler가 같은 animation clock을 사용한다.
- 완료와 Stop은 속도 1.0 및 `mesh_idle_battle_1` loop로 복귀한다.

## G02. 검증

- 실제 `Valtan.clipseq`를 사용한 v2 parse·resolve·playlist 순서와 repeat 검증
- legacy direct clip, 잘못된 version/action/sequence, model clip 누락, 행 순서 오류 거부
- 실패 시 기존 document·playlist 보존
- Debug/Release `ClientFrontendHarness --valtan-pattern-preview-fast`
- Debug/Release Client build
- JSON/XML parse, scoped `git diff --check`, UI 재생 소유권 P0/P1 감사
- Client 화면과 원본 영상 육안 일치 판정은 사용자가 직접 수행
