# Character Composition Sequencer 통합 결과

작성일: 2026-09-22. 대응 계획은 `2026-09-22_CHARACTER_COMPOSITION_SEQUENCER_IMPLEMENTATION_PLAN.md`다. 기존 미커밋 변경을 보존하며 Character Action의 편집·저장 경로만 확장했다.

## G00. 구현 상태

`Composition Actions → Character → Character Actors`의 배우·스킬·단계 선택은 기존 `EffectAuthoringSequencer`를 연다. Character의 기본 편집창은 `Composition Sequencer`이며 기존 `Skill Binding / Combat` 편집창도 유지한다. 별도 read-only Effect 표를 보는 대신 Animation, Effect, Sound 박스를 같은 row 편집·재생·Detail 흐름에서 사용한다. 보스 화면과 같은 Stages / Animation / Logic / Summon / World / Scene Profile / Effect / Sound / Camera / Collider / Light / Subtitle / Screen Post 순서를 표시하며 지원하지 않는 타입의 행은 비어 있다.

`Create Skills`는 기존 `PlayerSkills.json`의 class, 물리 입력 슬롯, skill ID를 고르고 실제 모델의 초기 clip을 연결한다. 임의 gameplay skill ID·key를 추가하는 기능은 아니다. 일곱 저작 class의 catalog와 실제 WModel clip을 사용하고 단계 수·skill 종류는 기존 검증을 따른다. Character/Model의 공용 Animation resource append는 현재 선택한 정확한 모델을 검사하고, append 또는 선택 Animation 교체를 기존 sequencer에 전달한다.

## G01. Product 저장과 draft 보존

Character Composition의 Save는 선택 스킬/단계의 실제 `.skillbindings.json`과 `.animevents`에 clip 및 Effect/Sound cue를 반영한다. `.animevents`의 HIT 등 다른 이벤트 타입, 다른 clip/window의 cue 및 해석하지 못한 source 참조는 보존한다. 원본 source time으로 변환하고 순차 clip, playRate, source trim 및 기존 HOLD stage 조건을 검증한다. preview collider/camera 등 추가 배치는 기존 `.effectsequence.json` owner에 보존한다. Server timing·damage·collision의 정본은 기존 `Save Combat`과 gameplay publisher다.

Composition dirty가 상위 workbench의 dirty 판정에 포함된다. 미저장 상태의 다른 배우/skill 선택은 편집을 유지한 채 거절하며 Save 성공 후 선택할 수 있다. 실제 preview 모델을 복원할 때 draft를 다시 생성하지 않고 기존 row를 새 모델 generation에 검증하여 연결한다.

Binding와 cue는 각각 기존 baseline-aware 원자적 writer를 사용한다. cue의 사전 검증 후 binding 저장, cue 저장 순서이며 두 번째 단계 실패 시 이번 binding 변경만 이전 디스크 문서로 조건부 rollback한다. 다른 writer가 이미 바꾼 파일은 덮지 않고 부분 저장 상태를 표시한다. 여러 파일 전체가 단일 원자적 transaction인 것으로 설명하지 않는다.

Product 선택 시 `.animevents`를 읽은 동일 bytes로 parse하고 stage한다. cue writer는 최초 baseline 비교뿐 아니라 기존 `Load_Events` 직후 baseline을 다시 비교한다. 과거 `.effectsequence` 배치는 Product action 선택에 자동 overlay하지 않으며 명시적인 `Load Effect Sequence`에서만 읽는다. 같은 source clip의 반복 occurrence가 겹치는 source 구간에서 서로 다른 Effect/Sound cue를 가지면 누락/추가도 포함하여 저장을 거절한다. source clip별 `.animevents` 계약에서 표현할 수 없는 occurrence별 차이를 조용히 반복 재생하지 않는다.

최종 review에서 source cue 갱신 범위를 previous+proposed 합집합에서 proposed clip/window로 제한했다. Animation 교체·삭제 또는 trim 축소로 현재 binding에서 제거된 source 구간도 다른 스킬에서 사용될 수 있으므로 그 구간의 기존 cue를 보존한다. 이전 binding은 workbench의 rollback에 사용하며 제거된 source cue를 삭제하는 권한이 아니다.

## G02. Clown / Mario / MAZE Product Effect

기존 interaction은 한 clip을 유지하고 최대 256개의 optional `effectCues`를 저장한다. 기존 `effectAssetId` scalar는 계속 읽으며 새 배열과 동시 사용을 거절한다. Composition Save는 legacy cue를 배열로 변환한다. 필드는 `effectAssetId`, source `startMs/endMs`, `anchorSlotId`, `position`, `rotationDegrees`, `scale`, `followPolicy`, `stopPolicy`, `orientationPolicy`다. source 시각은 정수 0..600000이며 CUE_END는 end > start, TRS는 finite, scale은 양수다. root 및 실제 bone/socket anchor를 기존 Effect owner가 검증한다.

`Character.cpp/.h`의 실제 consumer는 같은 변경에서 연결되었다. Server가 승인한 interaction의 action age와 clip playRate로 각 cue의 source 시각을 평가하고 기존 EffectPresentationService로 재생한다. 준비 지연은 현재 action에서만 재시도하고 action 전환 시 pending cue를 취소한다. CUE_END는 실제 interaction owner와 cue duration을 사용한다. 기존 scalar의 start 0/root/ACTION_FACING 의미는 유지한다. 툴 재선택 시 명시한 CUE_END endMs를 native clip 끝으로 늘리던 합성 처리를 제한하여 저장 후 재열기에서 길이를 보존한다.

## G03. 파일과 public 계약

기존 `CharacterActionWorkbench`, `CharacterModelWorkbench`, `EffectAuthoringSequencer`, `Animation_Tool`의 H/CPP 및 `EffectAuthoringSequencer_Timeline.cpp`를 수정했다. `SequencerTool.h`에는 공용 resource tree의 cache와 기본 표시 상태를 추가했다. resource browser 본문과 분류 확장은 별도 Composition Resources 작업에서 반영했다. 새 C++ 파일이 없으므로 `.vcxproj/.filters` 등록 변경은 없다. 소유 C++ 파일은 UTF-8 BOM 없음과 CRLF를 유지했다.

public 저장 계약과 실제 consumer 설명은 `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`의 Character Actors 및 Clown / Mario / MAZE 항목에 반영했다. 기존 binding/cue owner를 확장했으며 새 runtime 경로를 만들지 않았다. 원본 JSON을 자동 재저장하거나 기존 skill 데이터 전체를 migration하지 않았다.

## G04. 검증 완료

- MSVC 14.44 x64 Debug `/Zs` syntax 검사: `CharacterActionWorkbench.cpp`, `CharacterModelWorkbench.cpp`, `Animation_Tool.cpp`, `EffectAuthoringSequencer.cpp`, `EffectAuthoringSequencer_Timeline.cpp` 모두 PASS. 실행은 `out/CharacterCompositionSequencer20260922/compile.cmd`, 최종 log는 17:12:06~17:12:25다. `/Zs`이므로 object 생성·link 성공으로 기록하지 않는다.
- 독립 review에서 source baseline의 두 번 읽기 간격, 반복 clip cue 누락/추가, stale arrangement overlay와 interaction 명시 end 보존을 재검토했다. 발견된 저장 결함은 모두 반영했고 추가 blocker가 없다는 결과를 받았다.
- 소유 파일의 `git diff --check` PASS. 실제 source의 동일 bytes parse, post-load baseline compare, source projection multiset guard 및 interaction end guard를 확인했다.
- 최종 추가 review에서 Model/Monster의 sequence Open 실패가 성공 선택으로 반환되는 분기를 수정했다. 오류 status를 유지하고 false를 반환하며 `CharacterModelWorkbench.cpp` Debug `/Zs`를 다시 실행하여 PASS했다.
- source-global cue 갱신을 proposed window로 제한한 뒤 `Animation_Tool.cpp` Debug `/Zs` 재검사 PASS. production export 함수 전체 body, 실제 duration resolver와 source-window predicate를 그대로 추출한 headless executable은 94 assertions PASS다. 반복 cue 검사 32개 시나리오 82 assertions와 source window 보존 12 assertions로 구성한다. 삭제/추가·중복 수·TRS·anchor·policy 차이는 거절하며 동일 cue, 순서가 다른 동일 multiset, half-open/nonoverlap/partial overlap 및 서로 다른 rate의 동일 source cue는 통과했다. 모든 거절 시 output binding/cue가 기존 값인 것도 확인했다.
- 연결 runtime의 `Character.cpp` syntax 검사와 interaction parser 93 assertion PASS는 통합 담당의 검증 결과다. 이 작업의 5 TU 검사와 구분한다.

Product 전체 compile/link는 통합 담당이 별도로 진행한다. 여기의 syntax 결과만으로 설치된 실행 파일이 바뀌었다고 판정하지 않는다.

headless 산출물은 `out/CharacterCompositionSequencer20260922/projection_export_harness.*`와 `build_projection_harness.py`, `compile_projection_harness.cmd`다. production에서 그대로 추출한 19개 정의의 SHA-256을 provenance JSON에 기록했다. 모델 조회·기본 Animation row admission·Sound catalog는 fixture로 격리했으며 이 검증은 UI, 실제 모델 데이터, 원자적 file writer 및 GPU/runtime 재생 검증을 대신하지 않는다. 제품 source를 검증용으로 리팩터링하지 않았다.

## G05. 사용자 화면 확인과 남은 경계

Client/UI는 실행하거나 조작하지 않았다. 사용자가 새 Product에서 Character Actors 선택 → class/skill/Stage의 Sequencer 열기 → 실제 Animation append/replace와 Effect load/append/preview → source cue 이동/길이 변경 → Save → 다른 action 선택 및 재선택을 확인해야 한다. Clown/Mario/MAZE Q/LMB에 Effect를 여러 개 추가한 뒤 저장·재열기와 실제 승인 action 재생도 화면 확인 대상이다.

Product interaction은 계속 한 clip이며 임의 multi-clip chain을 새 runtime으로 만들지 않았다. 플레이어 Create Skills는 catalog에 존재하는 presentation slot 연결이다. Camera 등 preview 전용 배치를 재열려면 명시적 Load Effect Sequence를 사용한다. gameplay Product presentation은 캐릭터 재입장으로 로드하며 메모리의 모든 live actor가 Save 즉시 자동 갱신된 것으로 설명하지 않는다.

## G06. 통합 제품 빌드

첫 Product 빌드는 실행 중 Client/Server의 출력 점유로 시작 전 중단됐다. 사용자가 저장 후 두 프로그램 종료를 확인한 뒤 정식 Debug Product를 실행했다. Engine·Shared·Server와 이번 변경의 Client TU 컴파일은 통과했지만, 빌드 도중 추가된 맵 최적화의 `MapCullingCandidates/MapCullingVisible`을 앞서 배포된 EngineSDK `Profiler.h`에서 찾지 못해 Client 단계가 실패했다. 이때 Client.exe 링크는 완료되지 않았다. 로그는 `out/CharacterSizeSave20260922/product-build.log`, 결과는 `out/BuildPipeline/runs/20260922T083827217Z-debug-product.json`이다.

다음 정식 Product도 Engine·Shared·Server를 통과한 뒤 Client 단계에서 실패했다. 결과는 `out/BuildPipeline/runs/20260922T091134682Z-debug-product.json`이며 이전 오류와 같은 원인이라고 추정하지 않는다. 18:12에 시작한 세 번째 정식 Product도 이후 추가된 EffectBounds/EffectAmbient profiler enum과 17:52 EngineSDK header의 불일치로 18:45 Client 단계가 실패했다. 로그는 `out/CharacterMaterials20260922/product-build.console.log`, receipt는 `out/BuildPipeline/runs/20260922T094535521Z-debug-product.json`이다.

세 번째 runner가 끝난 뒤 최신 Engine부터 네 번째 정식 Product를 시작했다. 18:46 Engine 빌드와 최신 Profiler.h 배포를 확인했다. 동시에 실제 FX11 reflection에서 불일치가 확인된 CSO4개만 hash 검증·백업 후 제거하여 정식 FxCompile의 재생성을 강제했다. 전체 Clean/Rebuild나 shader skip은 하지 않았다. 로그는 `out/CharacterSizeSave20260922/product-final-build.log`, shader 백업과 manifest는 `out/DefaultOutfits20260922/abi-stale-backup/`다. 이번 세션의 기본 의상 program112/084와 별도 세션의 신규160~210 재질 확장 검증 범위를 구분한다.

네 번째 정식 Debug Product는19:04 Engine·Shared·Server·Client 모두 PASS했다. 결과 receipt는
`out/BuildPipeline/runs/20260922T100454659Z-debug-product.json`이며 Client.exe는19:04:51에 링크됐다.
19:02 설치 모델 consumer도14모델·31재질·Base/Light62바인딩·rollback14건을 통과했다.

같은 작업 폴더의 병행 재질 작업은 이 빌드 중/직후237까지 추가돼 공용 parameter header가 다시
바뀌었다. 후속 컴파일에서 실제 C1061이 재현되어135개 family의 중첩 분기를 guarded sibling으로
바꾸고 정본 생성기/설치기/reader까지 수정했다. 이 후속 변경과 새 cohort의 최종 제품 검증은
기본 의상 RESULT의 G09에서 구분한다.19:04 PASS를 그 이후의 모든 공용 소스 빌드 성공으로
확장해서 해석하지 않는다.

사용자의19:21 정리·마무리 요청에 따라 대기 검사를 종료했다.19:04 Product PASS와 저장·cue
검증 결과는 보존돼 있으며, 이후 공용 header의 C1061은 수정·검증했다. 진행 중인 별도 Product의
최신 전체 링크/화면 확인은 완료 처리하지 않는다. 현재 경계는 기본 의상 RESULT의 G10을 따른다.
