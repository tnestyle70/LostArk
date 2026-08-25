# 2026-08-25 발탄 패턴 사운드 큐 RESULT

대응 PLAN: `2026-08-25_VALTAN_PATTERN_SOUND_CUE_PLAN.md` (같은 폴더)

## 실행 중 발견/수정한 실제 버그

- **`ownerArchetypeId` 오타로 전체 문서가 fail-closed됨.** 생성기가 헤더에
  `"ownerArchetypeId": "Valtan"`을 썼는데, 실제 `Valtan.patterneffectcues.json`과
  `CEncounterPatternReference::Get_BossArchetypeId()`가 쓰는 값은 `"BOSS_VALTAN"`이다.
  `CValtanPatternSoundCueDocument::Parse_Text`의 헤더 검증이 이 불일치를 잡아
  `"Valtan pattern Sound cue header is invalid."`로 전체 327개 cue가 로드조차 안 되고
  isolated됨(실제 사용자 재현 로그로 확인). `build_valtan_pattern_sound_cues.py`의 상수를
  `"BOSS_VALTAN"`으로 고치고 `Valtan.patternsoundcues.json` 재생성 완료 — 코드 재빌드 없이
  JSON 파일만 다시 생성했으므로 Client 재실행만으로 반영된다.
- **`bindingId` 충돌로 그 다음에도 전체 문서가 다시 fail-closed됨.** 위 수정 후 재현했더니
  `"Valtan pattern Sound cue identity or policy is invalid."`로 또 isolated됨(실제 사용자 재현
  로그로 확인). 원인: `bindingId` 생성 시 `clipOccurrenceId`의 마지막 dot-segment만 썼는데, 같은
  actionId(`valtan.attack.dash-charge.windup`)가 서로 다른 두 clipOccurrenceId
  (`....windup.clip.01`과 `....windup.project-tuned.prep-repeat.clip.01`)를 갖고 그 둘의 마지막
  segment가 우연히 둘 다 "01"이라 bindingId가 9쌍(18개 cue) 충돌했다. `CValtanPatternSoundCueDocument
  ::Parse_Text`의 `BindingIds.insert(...).second` 유일성 검사가 이를 그대로 잡아냈다(Effect 쪽
  검증 로직을 그대로 가져온 덕에 실제로 작동함을 확인). `clipOccurrenceId` 전체 문자열을 bindingId에
  쓰도록 고쳐 327개 전부 유일함을 재검증(bindingId/occurrenceId 각각 327/327 unique, 최대 길이
  93자로 `Is_StableId`의 160자 한도 내). JSON만 다시 생성했으므로 이번에도 재빌드 불필요.
- **세 번째: clip segment window 밖 startMs.** 위 수정 후에도
  `"Valtan pattern Sound cue source window is outside its clip segment: cue.sound.valtan.attack.
  dash-charge.active.clip.01.01.occurrence.01"`로 isolated됨(실제 재현 로그 확인). 원인: 생성기가
  같은 clip 이름을 쓰는 모든 binding에 SOUND row를 매칭했는데, 같은 actionId 안에 clipOccurrenceId가
  여러 개면 그중 일부는 원본 클립 타임라인의 일부 구간(`sourceStartMs`~`sourceStartMs+playMs`)만
  가리키는 slice occurrence라, 그 구간 밖의 startMs를 가진 row가 잘못 매칭됐다. 생성기에
  `CValtanPatternSoundCueDocument::Parse_Text`와 동일한 구간 검사를 추가해 41개 row를 스킵 —
  327개 → 286개로 줄었다. 반영 후 patternbindings/encounter 전체 재조인을 Python으로 독립
  재시뮬레이션해 헤더·키 집합·bindingId/occurrenceId 유일성·actionId/clipOccurrenceId 소유·
  encounter tuple·clip segment 6개 항목 전부 0 errors 확인(286개 전량 검증).
- **네 번째, 그리고 진짜 구조적 원인: `Load_PatternSoundCues()`가 cue 하나만 틀려도 전체를
  버리는 all-or-nothing 구조였다.** `Parse_Text`(JSON 레벨 검증)를 통과해도 `Valtan.cpp`의
  `Load_PatternSoundCues()`가 실제 모델의 애니메이션 타임라인(`Build_PatternTimeline`/
  `Resolve_ClipDuration`/`Resolve_CueWallOffset`, 실제 wmodel의 tick rate/duration에 의존해
  Python으로 재현 불가능한 영역)으로 다시 한번 검증하는데, 이건 `CValtanPatternEffectCueDocument`
  쪽 원본 패턴을 그대로 복사하면서 실패 시 `return`(전체 중단)을 그대로 가져온 게 원인이었다.
  Effect 큐는 게임플레이 인접 저작이라 hard stop이 합리적일 수 있지만, 사운드는
  `CSoundCueCatalog` 헤더 주석에도 명시된 "Client-only presentation, no gameplay authority"라
  cue 하나 잘못됐다고 나머지 285개까지 죽이는 건 명백히 과한 설계였다. 5곳의 per-cue 거부 지점을
  전부 `return` → `continue`로 바꾸고, 거부된 cue 개수를 세어 로그로 남기도록
  (`Client/Private/Valtan.cpp`) 수정 — **이번은 C++ 변경이라 Client 재빌드가 필요하다**(Engine은
  안 건드렸으므로 Engine 재빌드는 불필요, Client만 다시 빌드/링크).

## 구현 완료

### 1. 데이터 생성 (실행 완료, 실제 파일 생성 확인)

- `Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py` 신규 작성, 실행 완료.
  `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` 생성.
- 조인 결과:

  | 항목 | 수 |
  |---|---|
  | `Valtan.animevents` 원본 SOUND row | 261 |
  | 스코프 제외 (S_BGM_CommanderRaid/S_Systems bank) | 4 |
  | 미매치 (patternbindings에 해당 clip 없음) | 85 |
  | 미매치 (clip은 매치, encounter stage actionId 없음) | 0 |
  | 스킵 (clip은 매치했지만 startMs가 그 occurrence 자신의 segment 밖) | 41 |
  | 최종 생성된 cue row | 286 |

  85개 미매치는 phase 2/3 등 아직 authored되지 않은 패턴의 클립으로 보이며, 팀 결정상
  phase-one pattern master만 우선 완료된 상태와 일치한다(추측 아님, PLAN 5절에 실측 근거 기록).

- `Tools/CharacterAnimationIntake/build_valtan_sound_catalog.py` 신규 작성, 실행 완료.
  원본 raw 사운드 라이브러리는 세션 중 `D:\로아 리소스`가 이 세션 프로세스에서 보이지 않게 된 문제로
  사용자가 `C:\Users\...\OneDrive\Desktop\Sound`에 직접 복사한 사본을 사용
  (`--valtan-root` 옵션을 새로 추가해 flat 로컬 사본을 바로 가리킬 수 있게 함).
  결과: 114개 이벤트 중 113개 매칭(`G_Voltan1_Attack13_Loop1` 1개만 미매치, 원본 라이브러리에
  해당 wav가 없음), wav 215개(변형 포함)를 `Client/Bin/Resources/Sound/Character/Valtan/`에 배치,
  `Data/Sound/CharacterSoundCatalog.json`에 `"Valtan"` 키 병합(다른 클래스 항목은 변경 없음).

### 2. C++ 반영 (코드 작성 완료, 빌드는 미실행 — 사용자가 직접 실행)

- `Client/Public/ValtanPatternSoundCueDocument.h` / `Client/Private/ValtanPatternSoundCueDocument.cpp`
  신규. `CValtanPatternEffectCueDocument`와 같은 encounter/animation-binding 조인 검증 골격을
  Effect 전용 복잡도(Product 카탈로그 admission, Material V1 alias, anchor bone/transform 검증) 없이
  구현. `Client.vcxproj`/`.vcxproj.filters`에 "03. Tools\04. Animation" 필터로 등록.
- `Client/Public/Valtan.h`: `VALTAN_PATTERN_SOUND_CUE_DOCUMENT`/`VALTAN_PATTERN_SOUND_CUE`/
  `VALTAN_PATTERN_SOUND_REPEAT_POLICY` include, `Load_PatternSoundCues()`/
  `Spawn_DuePatternSoundCues(f32_t)` 선언, `m_PatternSoundCuesByActionId`/
  `m_AttemptedPatternSoundOccurrenceKeys`/`m_bPatternSoundCueScanAgeValid`/
  `m_fPatternSoundCueScanAgeSeconds` 멤버를 Effect 쪽과 나란히 추가.
- `Client/Private/Valtan.cpp`:
  - `Load_PatternSoundCues()` — `CValtanPatternSoundCueDocument::Load_Source` 호출 후
    `m_PatternClipByActionId`/`Build_PatternTimeline`/`Resolve_ClipDuration`/`Resolve_CueWallOffset`로
    재검증, `m_PatternSoundCuesByActionId`에 스테이징. `Load_PatternEffectCues()` 바로 뒤,
    같은 초기화 시퀀스(`Ready_PartObjects` 이후)에서 호출.
  - `Spawn_DuePatternSoundCues(f32_t)` — Effect와 같은 서버 액션/패턴/스테이지 일치 검사, 같은
    `Resolve_ValtanPatternEffectOccurrenceScan` 재사용(이 함수는 Effect 전용 타입에 의존하지 않는
    순수 헤더 inline 유틸이라 그대로 재사용 가능함을 확인). `CEffectPresentationService::
    Try_Get_PreparedProductDurationSeconds`에 대응하는 사운드 쪽 준비된 길이 조회가 없어서(사운드는
    GPU 준비 큐 자체가 없음), 대신 고정 상한 `ASSUMED_SOUND_DURATION_SECONDS = 4.f`를
    "너무 늦은 catch-up이면 버린다" 게이트로만 사용 — 실제 정확한 wav 길이가 아니라는 점은 코드
    주석에도 명시. 실제 재생은 `CSoundCueCatalog::Find_Variants("Valtan", event)` +
    `std::rand() % variants.size()` + `CGameInstance::Get().Play_Sound(...)`로
    `CCharacter::Update_SoundCues()`와 동일한 변형 선택/재생 패턴을 그대로 사용.
  - `Update()`의 `patternEdgeChanged` 분기에 사운드 쪽 dedup/스캔 상태 리셋과
    `Spawn_DuePatternSoundCues(fActionAgeSeconds)` 호출 추가.
  - 새 include: `RuntimeAssetRoot.h`, `SoundCueCatalog.h`, `<cstdlib>`. `<filesystem>`을 기존
    `#ifdef _DEBUG` 전용에서 무조건 include로 이동(비-Debug 코드에서도 `std::filesystem::path`가
    필요해짐).

### 3. `HUDLayoutTool.cpp` 관련 없는 별도 수정 (같은 세션, 이전 대화에서 이미 보고)

- AnimationFrames 미리보기가 additive blend를 무시하던 진짜 버그 수정 — 이 RESULT의 범위와 무관하지만
  같은 작업 세션에서 아직 커밋되지 않은 상태로 diff에 남아있어 참고로 기록한다.

## 미검증 (사용자 전용 경계)

- Debug 빌드 성공 여부 — 에이전트가 MSBuild를 직접 실행하지 않음(팀 규칙).
- 실제 발탄 입장 후 SWING/BACKSTEP 등 패턴 발동 시 보이스/타격음이 실제로 들리는지 — Client 실행/
  청취는 사용자 전용 검증 경계라 에이전트가 PASS로 기록하지 않는다.
- `G_Voltan1_Attack13_Loop1` 1건 미매치는 원본 raw 라이브러리 자체에 해당 wav가 없어서이며, 추가
  조치 없이 그대로 두었다(해당 cue는 `CSoundCueCatalog::Find_Variants`가 빈 배열을 반환해 조용히
  스킵됨 — 게임플레이 권위에 영향 없음).

## 다음 단계 (요청 시)

- `S_BGM_CommanderRaid`/`S_Systems` bank의 SOUND row는 이번 스코프 밖으로 남아있다(PLAN 5절).
- 생성기가 현재 모든 cue를 `repeatPolicy: "once"`로만 만든다 — `mesh_abn_groggy_1_loop`처럼 실제
  loop 클립 위의 SOUND row를 `each_loop`으로 자동 분류하는 로직은 아직 없다(C++ 쪽은 `each_loop`을
  이미 완전히 지원하므로, 필요해지면 생성기만 수정하면 됨).
