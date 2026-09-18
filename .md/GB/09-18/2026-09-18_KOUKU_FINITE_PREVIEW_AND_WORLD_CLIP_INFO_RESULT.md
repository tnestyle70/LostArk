# 쿠크 finite Effect 수명 미리보기와 World Animation 정보 결과

## G01. 저장본 실측과 데이터 보존

Composition revision 1720, WorldSequence revision 2128의 실제 JSON을 읽었다. 근거와 파일 SHA-256은 `out/KoukuSequencerLifetime20260918/data-evidence.json`에 있다. 이 작업에서는 authoring JSON과 Resources를 수정하지 않았다.

| 항목 | stable ID | 현재 시작 / 수명 |
|---|---|---|
| 대형세이튼 불 1 | `KAKULSAYDON_G1_PATTERN_27.presentation.2` | 2366 / 2340 ms |
| 대형세이튼 불 2 | `KAKULSAYDON_G1_PATTERN_27.presentation.3` | 2347 / 2360 ms |
| 대형세이튼 불 3 | `KAKULSAYDON_G1_PATTERN_27.presentation.4` | 2348 / 2361 ms |
| 공굴리기 카운터 불 | `KAKULSAYDON_G1_PATTERN_81.presentation.1` | 3002 / 8554 ms |
| 화이트 훌라후프 B | `KAKULSAYDON_G1_PATTERN_90.presentation.7` | 1640 / 15333 ms |

불 네 행은 모두 `effect.kouku.gate3.firebreath.shared`를 재사용하고 `loopEffectToDuration=true`다. 따라서 각 패턴의 수명을 분리하기 위해 같은 source asset을 복제할 필요는 없다.

화이트의 B는 정확히 `쿠크_훌라후프 액션B_par_g_rpcz_trowhoop_b_01` Effect resource다. P90에는 child/summon이 없고 Animation은 전부 sourceActionId 4219726(액션A)다. 4219727(액션B) Animation은 배치되지 않았다. 요청한 B Effect 박스가 이미 15333 ms이므로 그 값을 보존했다. source emitter 1/4초를 임의로 15333 ms로 바꾸거나 입자 수명을 늘리지 않았다. 이 박스 시간과 개별 입자의 source 수명은 별개다.

## G02. 단독 Effect Preview의 finite-loop 거절 수정

`CEffectAuthoringSequencer::Stage_Row`는 `loopEffectToDuration`이면 `Set_SourceLoopEndSeconds`를 무조건 호출했다. 해당 함수는 native `EmitterLoops=0` emitter가 있어야 성공한다. shared fire의 25개 emitter는 모두 `EmitterLoops=1`이므로 이 미리보기 경로에서 실패했다. Composition/Complete 재생은 이미 finite/native를 구분하고 있었다.

변경 파일과 책임은 다음과 같다.

- `Effect_Playback.h`의 `Has_InfiniteSourceEmitters`는 현재 staged 문서에서 admission된 실제 emitter만 읽는다. Catalog 원본을 다시 로드하지 않아 현재 unsaved preview 문서도 반영된다.
- `Effect_Object.h`는 같은 읽기 전용 질의를 노출한다.
- `EffectAuthoringSequencer.h/.cpp`는 finite source의 자연 재생주기를 row 세션 값으로 저장한다. loop0이면 기존 bounded source emission을 사용하고, finite이면 원래 속도로 반복한다.
- cycle 경계를 넘으면 입자 시계를 source age로 되감는다. transform provider에는 cycle 시작을 다시 더해 보스·입 본 clock은 전체 occurrence 시각을 유지한다. 같은 cycle의 전진은 기존 incremental update를 유지하고, 역방향 seek와 cycle 변경 때만 기존 seek를 쓴다.
- Release에서 finite cycle 상태도 초기화한다. resource 문서와 사용자 lifetime, scale, bone offset은 바꾸지 않는다.

이는 finite Effect 전체를 자연주기로 반복하는 기존 Composition 정책과의 일치다. 원본 emitters의 방출 공백을 지우거나 각각의 particle lifetime을 늘리는 계약은 아니다.

## G03. 세 연출의 Animation 정보 연결

Animation lane이 비어 있던 세 패턴은 별도 World 배우의 baked clip을 이미 재생하고 있었다. 원본 source Animation을 보스 lane에 다시 추가하면 기존 연출 배우와 중복된다.

| 패턴 | World occurrence | 실제 설치 clip | World 재생창 |
|---|---|---|---|
| P73 `2관문_진입컷씬` | `.world.2` | `gate2_intro_27s` | 0–27000 ms |
| P74 `2관문클리어_3관문진입` | `.world.2` | `kouku.gate2.clear.kouku` | 0–35368 ms |
| P77 `카드미로연출` | `.world.1` | `kouku.gate2.maze.kouku` | 0–11950 ms |

`ModelAssetConverter info`로 세 설치 WModel의 실제 clip 이름을 대조했다. 경로와 모델 크기는 data evidence에 있다.

`KOUKU_WORLD_SEQUENCE_RESOURCE`에 model asset ID와 clip/slot/start/end/source-in/rate/loop/hold 정보를 추가했다. `CMainApp::RefreshWorldObjectResources`가 기존 document generation/revision cache 갱신 때 저장한 animationTracks에서 한 번 채운다. 매 프레임 파일을 읽지 않는다.

Workbench Animation lane은 World clip을 읽기 전용 행으로 보여준다. 행을 클릭하면 실제 소유 World 박스가 선택된다. Box Detail에는 model path, clip/slot, 재생창, Source In, 최종 playback rate, Loop/Hold를 표시하며 기존 `Edit This Motion`으로 연결한다. 신규 보스나 재생 객체, 저장 Animation occurrence는 만들지 않는다. 표시 window에는 instance delay/speed와 occurrence speed/cutoff를 적용한다. 실제 clip 길이와 hold 시간을 혼동하지 않도록 `Playback window`로 표시한다.

## G04. 검증

- `out/KoukuSequencerLifetime20260918/compile.cmd`: EffectAuthoringSequencer / KoukuSaydonActionWorkbench / MainApp 3 TU 격리 MSVC Debug 컴파일 exit 0.
- 실제 production 분기·provider·sample 호출부를 추출한 C++ probe 16 checks / 0 failures. finite admission, loop0 bounded lifetime, loop-disabled, 순방향 cycle 교체, 같은 cycle advance, rewind, root/입 본의 전체 시각 유지 확인. 실제 GPU 입자 표시는 검사하지 않았다.
- JSON 두 문서 parse, 세 설치 WModel info 및 정확 clip lookup 성공.
- 관련 C++ `git diff --check` 성공. 기존 인코딩/BOM/CRLF를 보존해 byte anchor 치환했다. 기존 C++ 파일만 바꿨으므로 project/filter 등록 추가는 없다.
- Product 전체 빌드는 root의 통합 결과에 기록한다. 이 하위 작업은 Client/UI를 실행·조작하거나 화면을 캡처하지 않았다.

## G05. 사용자 확인 경계

새 Client에서 세 연출의 Animation lane에 World clip 행이 보이고 클릭 시 해당 World Box Detail에 정보가 나오는지 확인한다. 공굴리기/큰 세이튼 불은 저장한 각 lifetime으로 단독 Preview가 admission 실패 없이 재생되는지, 실제 반복 경계와 입 부착 모양을 확인한다. 코드·수치 검사 성공은 시각 결과 승인과 다르다.
