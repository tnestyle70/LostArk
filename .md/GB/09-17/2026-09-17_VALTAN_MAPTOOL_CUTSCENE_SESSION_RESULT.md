# 발탄 Map Tool 컷신 세션 — 구현 결과

작성일: 2026-09-17. 대상 계획서: `2026-09-17_VALTAN_SOURCE_CAMERA_REPLACEMENT_PLAN.md` 개정2.

완료 기준은 "Map Tool 에서 컷신 하나 선택 → 카메라 컷과 배우가 같은 시계로 재생 → 같은 화면 수정 → Save → Reload 유지" 다.
이번 반영으로 **카메라 축은 그 경로가 전부 연결됐고, World 배우(G05)는 미착수**다.

## G02 — 전투 데이터 복구 (완료)

`Data/Encounters/Valtan/ValtanCinematicCamera.json` 의 deathCue 가 23000ms/28키로 바뀌어 있던 것을
2090ms/키 3개로 되돌렸다. 파일은 HEAD 와 바이트 동일하고 `git status` 에 뜨지 않는다.
cues 11개와 헤더는 애초에 변경되지 않았음을 대조로 확인했다.

처음에는 deathCue 블록만 교체해 다시 썼으나, 원본이 들여쓰기 4칸·콜론 뒤 공백 2개인 PowerShell 서식이라
표준 JSON 서식으로 재작성하면서 1,422줄 diff 가 생겼다. 내용은 같아도 서식이 달라 `git checkout` 으로
원본 바이트를 복원했다. **이 문서는 스크립트로 부분 편집하면 서식이 깨진다.**

gameplay / presentation / Encounter / patternbindings / balance receipt / pipeline 예외는 앞선 작업에서
이미 HEAD 로 복구돼 있었고 이번에 다시 만들지 않았다. `VALTAN_SOURCE_PREVIEW_*` 패턴은 0개다.

## G03 — 컷신 시간표 계약 (완료)

`camerashots.json` root 에 optional `cutscenes[]` 를 추가했다. 원소는
`{cutsceneId, displayName, durationMs, cameraCuts[{cutId, shotId, startMs}], worldInstanceIds[]}` 다.

`cutscenes` 가 없는 문서는 이전 단일 샷 편집을 그대로 유지한다. 쿠크 Area Validate 가 통과하는 것으로 확인했다.

로더가 거부하는 것: 빈/중복 cutsceneId, durationMs 범위 밖(1~600000), 알 수 없는 shotId 참조,
중복 cutId, **컷 구간 겹침**, 잘못된 worldInstanceIds 타입. 겹침은 priority 로 얼버무리지 않고 저작 오류로 막는다.

등록한 5개 (전체 ms / 카메라 종료 ms / 컷 수):

| cutsceneId | 이름 | 전체 | 카메라 종료 | 컷 |
|---|---|---:|---:|---:|
| `editor.cutscene.valtan.gate1-entrance` | 1관문 입장(늑대) | 13000 | 13000 | 1 |
| `editor.cutscene.valtan.entrance` | 발탄 등장(부활) | 24708 | 23386 | 3 |
| `editor.cutscene.valtan.finale` | 발탄 최후 | 23000 | 23000 | 4 |
| `editor.cutscene.valtan.trash` | 발탄 스킬 연출(버러지) | 6374 | 5736 | 2 |
| `editor.cutscene.valtan.roar` | 발탄 포효 | 7003 | 5000 | 3 |

`worldInstanceIds` 는 5개 모두 빈 배열이다. 배우가 붙기 전까지 camera-only 로 동작한다.

## G04 — 공통 시계 (완료, 카메라 축)

시계를 `m_ArenaRisePlayer` 가 아니라 **Map Tool 세션이 소유**한다. 이게 핵심 변경이다.
기존 `Apply_CutsceneCameraTrack` 은 샷의 `sequenceInstanceId` 로 World player 에서 경과시간을 찾아야만
카메라를 움직였다. 발탄에는 World Sequence 문서가 없어 그 경로로는 영원히 재생되지 않는다.

새 경로:

- `Play_EditorCutscene` → 이전 세션 정리 → `Prepare_EditorCutsceneWorld` → PLAYING
- `Update_EditorCutscene` 이 절대시각 T 를 프레임당 한 번 진행(한 프레임 최대 `KAKUL_CUTSCENE_MAX_STEP_SECONDS`)
- `Seek_EditorCutsceneWorld` 가 `Seek_AllToMs(T)` 한 번만 호출. `Update` 를 같이 부르면 이중 가산되므로 부르지 않는다
- `Apply_EditorCutsceneCamera` 가 `Find_CutsceneCutAt(T)` 로 컷을 고르고 `T - startMs` 로 샷을 샘플
- `Apply_CutsceneCameraTrack` 은 세션이 STOPPED 가 아니면 즉시 반환한다. `PATTERN_ONLY`/`editingOneShot`
  조건이 세션 재생을 막던 문제가 여기서 사라진다

`worldInstanceIds` 가 비면 World player 를 아예 건드리지 않고 성공을 반환한다 — camera-only 가 1급 경로다.

컷 경계는 블렌드 없이 끊는다. 원본 감독 컷의 transitiontime 이 전부 0이므로 `blendInMs` 를 적용하지 않고,
컷이 바뀌면 `End_CutsceneCameraTrack()` 으로 held 상태를 버려 이전 프레이밍에서 미끄러져 들어오지 않게 했다.

정리 경로: Stop / 다른 컷신 선택 / **Area 변경** / `Load_CameraShots`(Reload) 에서 World instance 를
`Stop_Instance(..., restorePlacements=true)` 로 되돌리고 카메라 override 를 반납한다.
세션은 자기 Area 를 기억해 Area 가 바뀌면 따라가지 않고 스스로 종료한다.

### 함께 고친 기존 결함

- `Ensure_ShotCutsceneClock` 이 `Load_Area(KAKUL_AREA_ID)` 로 쿠크 문서를 하드코딩하던 것을
  현재 Area 로 바꿨다. `m_strArenaRiseLoadedArea` 를 새로 두어 다른 Area 문서가 로드돼 있으면 다시 읽는다.
- `TARGET_SET` 에 `device` / `context` / `objectPreparationOwner` 가 빠져 있었다.
  `Build_CutsceneTargets` 로 모아 넘긴다. 이게 없으면 object-resource 배우는 모델을 만들지 못한다.

## G06 — 같은 화면 편집 (부분)

`Render_CutsceneSection` 을 Camera 패널의 샷 목록 **위**에 넣었다. 컷신 목록이 먼저 오고 샷은 그 재료다.

- 컷신 콤보(5개), Play / Pause / Resume / Stop, Time 슬라이더(스크럽하면 자동으로 PAUSED)
- "전체 N ms / 카메라 종료 N ms / 컷 N개 / World N개" 와 camera-only 안내
- 컷 시간표 테이블(컷 / 샷 / 시작 / 길이 / 키). **현재 T 가 속한 컷을 파란 배경으로 표시**하고,
  행을 누르면 그 샷이 아래 키 편집기에서 열린다 — 시간표와 키 편집기가 항상 같은 대상을 본다

기존 키 타임라인(`Render_CameraTrackTimeline`)은 그대로 재사용한다. 키를 고치면 다음 프레임 샘플에
바로 반영되므로 미저장 draft 재생이 성립한다.

## G07 — 저장 (완료, 카메라 축)

`Save_CameraShots` 가 `cutscenes[]` 를 함께 쓴다. 비어 있으면 섹션 자체를 쓰지 않아
기존 문서 모양이 그대로 유지된다. 기존 `Save_CameraShotDocumentAtomic` 의 expected-source / 원자 교체를
그대로 통과하며, `Gameplay.world.json` 이나 전투 JSON 은 건드리지 않는다.

## 검증 — 실제로 실행한 것

- **데이터 규칙 전수 재현**: 로더 규칙(헤더, 중복 id, 범위, 샷 참조, 겹침, 카메라 종료 ≤ 전체)을
  파이썬으로 그대로 재현 → 실패 0건
- **포효 하드컷 경계**: T=1460 → cut01 로컬 1460 / T=1461 → **cut02 로컬 0** / T=3620 → cut02 로컬 2159 /
  T=3621 → **cut03 로컬 0**. 컷 사이 보간 없음
- **카메라 복귀 분리**: T=5000 → cut03 로컬 1379, T=5001 및 7003 → 소유 컷 없음(카메라 반납, 전체 시간은 계속)
- **Save 왕복**: writer 가 낼 텍스트를 그대로 만들어 재파싱 → `cutscenes` 가 원본과 완전 동일
- **FOV**: 문서 최소값 9.6107 이 clamp 없이 보존됨
- **전투 불변**: deathCue 2090ms/키3, cues 11, gameplay 패턴 42, `SOURCE_PREVIEW` 0개,
  `Data/Valtan/` · `Tools/ValtanPipeline/` · balance receipt 전부 무변경
- **게시 검증**: `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Mode Validate` 종료 0, FileCount 23
- **회귀**: 같은 명령을 쿠크(`LV_LUT_MIDNIGHTC_ED`)에 실행 → 종료 0, FileCount 8
- **정적 검사**: 문자열·주석을 제외한 괄호 균형 3파일 모두 일치, 신규 함수 11개 선언/정의 대응, 멤버 10개 선언 확인

## 실행하지 않은 것

- **C++ 컴파일.** `devenv.exe`(PID 23600)가 실행 중이라 같은 worktree 에 빌드를 겹쳐 돌리지 않았다.
  **사용자가 VS 에서 Build 해야 한다.** 컴파일 오류가 없다는 보장은 아직 없다.
- Client 실행, 화면 동작 확인, commit/push.
- `-Mode Publish` (Validate 까지만 실행).

## 남은 작업

1. **G05 World 배우.** `worldsequences.json` 이 아직 없다. 원본 매티니에서 발탄 그룹의
   transform/visibility/clip 을 읽어 binding 으로 만들고, `WORLD_SEQUENCE_OBJECT_RESOURCE` 에
   optional `animationSetAssetId` 를 추가해 `CModel::Attach_AnimationSet` 을 World 준비 경로에 연결해야 한다.
   몸체 27클립 / donor 146클립 구분이 필요하다. 그때 각 컷신의 `worldInstanceIds` 를 채우면 배우가 붙는다.
2. **원본 궤적 대조.** 현재 13샷은 CATMULL_ROM 으로 저장돼 있으나 원본 곡선(`cim_curveautoclamped`)과
   같다는 것이 실제 샘플러로 입증되지 않았다. 최대 위치/방향/FOV 오차를 수치로 내야 한다.
3. **up/roll.** 이 형식에 `up` 필드는 있으나 13샷에는 들어 있지 않다. 발탄 등장 c(약 10도)·c1(약 20도)의
   롤은 현재 재현되지 않는다.
