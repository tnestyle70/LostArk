# 쿠크 앵콜 컷신 자막 3줄 배치 결과

작업 브랜치: `feature/kouku-encore-subtitles-0920` (origin/main `3809ab26`에서 분기). commit/push는 하지 않았다.

## 1. 한 일 (파일에 적힌 것) [실측]

앵콜 컷신 두 곳에만 자막 배치(presentation occurrence) 3개씩을 추가했다. 자막 리소스(`subtitle.kouku.cin.37081_12_01~03`)는 두 문서에 이미 등록돼 있었고 배치만 없었다.

| GameMsg id | 문구 | 시작 ms | 길이 ms | 위치 |
|---|---|---|---|---|
| `cin.37081_12_01` | 누구 맘대로 끝을 내?! | 10233 | 1700 | UPPER |
| `cin.37081_12_02` | 무효야, 전부 무효! | 12367 | 3600 | UPPER |
| `cin.37081_12_03` | 진짜 시작은 지금부터라고! | 16700 | 3050 | UPPER |

| 문서 | 패턴 | 추가한 occurrence | 카운터 | revision |
|---|---|---|---|---|
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` (Action) | `KAKULSAYDON_G1_PATTERN_97` "앵콜컷신" | `.presentation.3 / .4 / .5` | `nextPresentationOccurrenceOrdinal` 3 → 6 | 1918 → 1919 |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` (Sequence) | `KAKULSAYDON_G1_PATTERN_10` "앵콜컷신" | `.presentation.3 / .4 / .5` | 3 → 6 | 120 → 121 |

- 배치 형식은 기존 자막과 같다. Action은 P73의 짧은 형태(`occurrenceId, resourceId, startMs, durationMs, anchorKind: MAP, followBoss: false`), Sequence는 P3의 전체 필드 형태(앵콜 이웃 항목처럼 여러 줄)를 그대로 복제했다.
- revision을 1 올린 이유: Workbench 저장(`++staged.iRevision`, `KoukuSaydonCompositionDocument.cpp`)이 저장마다 revision을 올리는 계약이다.
- 원문 바이트 보존: 두 파일 모두 CRLF, BOM 없음, 편집 후에도 LF-only 0줄. 앵콜 패턴 객체 안과 상단 revision 줄 외에는 한 바이트도 바꾸지 않았다.
- 편집 전 원본은 `C:\Users\USER\.claude\jobs\46aea322\tmp\subtitle_backup\`에 있다(되돌리기용). 편집 스크립트: 같은 폴더의 `attach_subtitles.py`(`--check` 예행 후 `--apply`).

## 2. 저장 시각을 원본 값 그대로 정한 근거 [실측]

앵콜 패턴의 시계는 컷신(SCENE07A) 시작과 같다. 앵콜 패턴 P97/P10의 camera(`presentation.1`), fade effect(`presentation.2`), 배우 world(`world.1`)가 모두 `startMs 0`에서 시작하고 길이 23333ms(컷신 전체)다. 원본 자막 트랙 시각도 매틴 시작 기준이라 변환 없이 그대로 저장한다. 이미 붙은 다른 컷신도 같은 방식이다(P73/P3의 `25_01`은 원본 3.08초 → 3080ms). 앵콜에는 P4 같은 "구간 시계표"가 없다. 자막 종료 최대 19750ms ≤ 패턴 길이 23333ms이고 세 자막은 서로 겹치지 않는다.

## 3. 검증

### 3-1. 확인된 것 [실측]

- 두 파일 JSON 파싱 통과. 의미 비교: 편집 전후 차이는 "앵콜 패턴에 3개 occurrence + 그 패턴 카운터 + revision"뿐이다(다른 패턴 전부, 최상위 필드 전부 동일). 다른 패턴의 자막 행 수: Action 14 → 14, Sequence 17 → 17.
- Workbench 저장 계약 확인: 자막은 MAP 고정 앵커(본·월드·bone 없음), 문구 유효 UTF-8·4096byte 이하, 위치 NORMAL/UPPER. 자막끼리 겹침을 막는 규칙은 없다.
- 실제 프로젝터 함수 `_project_presentation_occurrence`가 3행을 표와 같은 값(시작·길이·위치·문구)으로 만든다.
- 실제 파이프라인 경로(`prepare_publication` → `projected_outputs`, 쓰기 없음)로 만든 `KoukuSaydon.patternbindings.json`의 자막 행이 14개에서 17개로 늘었고, 새 3행이 위 표와 정확히 같다. 기존 행이 사라지거나 바뀌는 것은 없다.
- `project_kouku_saydon_composition.py --mode validate`(쓰기 없음)가 이 생성 단계를 통과하고 마지막 "디스크 산출물과 비교" 단계에서 `projected Product is stale: ...KoukuSaydonEncounter.json`으로 끝난다. 원본을 바꿨으니 게시로 갱신해야 한다는 예상된 안내이며 자막 오류가 아니다.
- `Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate`(쓰기 없음): `Composition validation passed: Valtan patterns=42, KoukuSaydon profiles=4 actions=349, arena sequencers=2` (Sequence 쪽 편집 포함, 통과).
- 단위 테스트: `test_scene_subtitle_candidates` 6개 통과(저장소 루트에서 실행), `test_raid_flow_projection` 8개 통과(`Tools/KoukuSaydonPipeline`에서 실행).

### 3-2. 완전하지 않은 것 [미확인·주의]

- `test_project_kouku_saydon_composition`(237개)는 이 환경에서 실패·오류가 많다. 올바른 경로(`Tools/KoukuSaydonPipeline`에서 `PYTHONPATH=<저장소 루트>`)로 돌린 실행이 이 문서를 쓰는 시점(00:04)에 237개 중 146개까지 진행돼 통과 90, 실패·오류 56이었고 **완주한 결과가 아니다**. 확인한 원인은 모두 자막과 무관했다.
  - 실행 위치: 프로젝터가 같은 폴더의 `raid_flow_projection`을 이름만으로 불러오고 테스트는 `Tools.KoukuSaydonPipeline`로 불러와서, `Tools/KoukuSaydonPipeline`에서 `PYTHONPATH=<저장소 루트>`를 주고 실행해야 한다.
  - `KoukuAnimationRootMotionTests.test_albion_takeoff_...`: 편집을 되돌린 상태(stash)에서도 똑같이 실패한다(`6.5566... != 13.6788...`). 원래부터 있던 실패다.
  - 그 밖의 실패(예: `playAllPatternIds must equal PRODUCT patternIds in authored order`, 테스트가 `{}`를 `projected_outputs`에 넣는 케이스가 raid 계획 오류로 끝남)는 테스트 준비 데이터와 main의 raid 계획 사이 어긋남으로 보이나, 편집 전 상태와 전부 대조하지는 못했다.
- 이 표의 "편집 전 원본과 실패 목록 전체 비교"는 하지 못했다. 자막 배치 자체를 검증한 것은 3-1의 실제 경로 검증이다.

## 4. 게시(publish)는 하지 않았다 [못 한 것]

작업 시작 시점에 VS(devenv)가 켜져 있어서 게시를 하지 않았다(프로세스는 종료하지 않았다). 그래서 다음은 아직 옛 상태다.

- `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`, `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json` (Action 자막이 여기 실린다)
- `Client/Bin/DataFiles/Compositions/Sequences/KoukuSaydonArena.sequencer.json` 등 Composition 게시 출력 (Sequence 자막)
- `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` (revision 반영)

게시 순서(VS·Client·Server를 끈 뒤): `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`(약 26분) → `-Owner Client`(약 30초). 서버 데이터 revision을 맞추려면 `-Owner Server`(약 30초)도 돌린다. 게시 뒤 git 변경에는 소수점 마지막 자리·줄끝·원본 해시 기록 같은 잡음이 섞이니 실제 변경만 남긴다(PR #427 이후 이 파일들이 Git 추적 대상이다).

## 5. 화면에서 확인할 절차 (사용자 몫)

자막이 어떻게 보이는지, 위치(UPPER)와 타이밍이 원작과 맞는지는 이 작업에서 확인하지 못했다.

1. 게시를 마친 뒤 Server를 다시 켜고 Client를 다시 실행한다(프로토콜 96, Server와 Client 함께).
2. F1 → Action Workbench → Boss/Sequence의 `앵콜컷신`(Action P97, Sequence P10)을 재생한다.
3. 컷신 시작 후 약 10.2초에 "누구 맘대로 끝을 내?!", 12.4초에 "무효야, 전부 무효!", 16.7초에 "진짜 시작은 지금부터라고!"가 화면 위쪽에 뜨는지 본다.
4. Workbench가 열려 있는 상태에서 이 파일을 직접 고친 것이므로, 이미 열어 둔 Workbench의 draft가 있으면 저장하지 말고 다시 불러온다(외부에서 바뀐 문서 위에 덮어 저장하면 이번 편집이 사라진다).
