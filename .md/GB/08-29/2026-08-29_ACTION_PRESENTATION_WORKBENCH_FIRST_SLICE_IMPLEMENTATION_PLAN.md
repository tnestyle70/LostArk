# Action Presentation Workbench 1차 수직 슬라이스 구현 계획

## 1. 목표

- 기존 Animation Tool을 별도 거대 편집기를 하나 더 만드는 대신 `Action Presentation Workbench`의 첫 화면으로 확장한다.
- 발탄 도끼 투척의 실제 정본인 `VALTAN_HIGH_JUMP / AIRBORNE` 서버 스테이지 공백 시간을 편집하고 저장할 수 있게 한다.
- 선택한 발탄 패턴의 Server stage, animation occurrence, Effect cue, Sound cue와 실제 WAV variant를 한 화면에서 조회한다.
- 같은 stable pattern ID를 Boss Tool의 기존 typed Server audition 경계로 제출해 실제 Valtan Arena에서 검증한다.
- 기존 Animation custom chain은 시퀀스 intake로 보존하되 Product 저장과 혼동하지 않는다.

## 2. 실측 정본

| 영역 | 현재 정본과 소비자 |
|---|---|
| 서버 패턴 시간 | `Data/Valtan/Valtan.gameplay.json` -> Valtan publisher -> Server |
| 발탄 애니메이션 | `Data/Valtan/Valtan.presentation.json` -> `CValtanPatternTree` -> `CValtan` |
| Product Effect | joined `VALTAN_PRODUCT_EFFECT_CUE_VIEW` -> Effect Catalog -> Effect Tool V1/runtime |
| 발탄 Sound cue | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` -> `CValtanPatternSoundCueDocument` |
| Sound asset | `Data/Sound/CharacterSoundCatalog.json`의 event -> Resources-relative WAV variants |
| 서버 재생 | `CValtanPatternAuditionService`를 소유한 `CBossTool` |
| 시퀀스 intake | `Data/Valtan/Valtan.presentation.debug.json` |

`VALTAN_HIGH_JUMP`는 TAKEOFF 1,933 ms, AIRBORNE 6,500 ms, LAND 3,200 ms,
RECOVERY 400 ms다. AIRBORNE animation은 `mesh_att_battle_8_01_loop`의
`LOOP_TO_STAGE_END`이므로 `durationMs`를 늘리면 서버 wall clock과 화면 pose가 같은 끝점을
사용한다. 별도 가짜 대기 clip은 추가하지 않는다.

## 3. 저장 명령 의미

| 버튼 | 이번 슬라이스의 정확한 의미 |
|---|---|
| `Validate Joined` | 현재 stable-ID gameplay draft가 split gameplay/presentation join을 통과하는지 검사 |
| `Save Gameplay Authoring` | Balance Tool이 가진 같은 draft에서 gameplay stage duration을 immutable authoring revision으로 저장 |
| `Publish Candidate` | 저장된 정본으로 runtime candidate 생성; active runtime은 아직 바꾸지 않음 |
| `Apply Revision` | candidate가 `HOT_RELOAD`이고 Server transaction 준비 조건을 만족할 때만 적용 |
| `Play Server Pattern` | 선택 pattern ID를 기존 Server audition service에 제출 |
| `Save Animation Intake` | custom chain 원본만 저장; manifest review/promotion 전에는 Product pattern이 아님 |

## 4. 구현 G

### G1. Animation Tool을 Workbench shell로 승격

- F1 버튼과 창 제목을 `Action Presentation Workbench`로 바꾼다.
- 기존 character animation/event/skill binding 기능과 Valtan local preview는 유지한다.
- 새로운 parser나 local boss AI를 만들지 않고 기존 `CValtanPatternTree`, Balance Tool,
  Boss Tool을 typed API로 조합한다.

### G2. 발탄 stage wall-clock 편집과 저장

- `CBalanceTool`에 stable `patternId + stageId` 기반의 작은 public authoring API를 추가한다.
- Workbench stage row에서 duration을 1..600,000 ms 범위로 편집한다.
- `VALTAN_HIGH_JUMP / AIRBORNE`에는 `Axe flight / blank timeline` 설명을 표시한다.
- unsaved Balance draft가 하나만 존재하도록 MainApp이 같은 `CBalanceTool` instance를 Workbench와 Balance 창에 공유한다.
- 저장 성공 뒤 joined pattern tree와 sound cue 문서를 다시 load한다.

### G3. Effect·Sound 연결 lane

- 각 stage 아래 animation occurrence와 Product Effect cue를 stable occurrence ID로 표시한다.
- Effect cue는 기존 Effect Tool V1 deep-link request를 보낸다.
- `CValtanPatternSoundCueDocument`를 한 번 load/stage/commit하고 같은 pattern/stage의 cue를 표시한다.
- Sound event마다 `CSoundCueCatalog`가 해석한 Resources-relative WAV variant를 표시하고 사용자가 누른 variant만 재생한다.
- generated sound cue 문서나 Sound Catalog를 이 슬라이스에서 임의로 덮어쓰지 않는다. 새 asset binding 저장은 Sound domain authoring schema가 추가되는 다음 수직 슬라이스다.

### G4. Server Replay/Live 연결

- `CBossTool`에 `Play_ServerPattern(patternId)` typed API를 추가한다.
- API는 graph를 stage한 뒤 audition inventory에 존재하는 stable ID만 기존 service로 제출한다.
- Workbench는 로컬 preview와 Server play를 별도 버튼으로 유지한다.
- Client는 실제 패턴, hit, grab, combat object, damage를 계산하지 않는다.

### G5. 시퀀스 추가 경계

- 기존 Custom Chain 창을 `Animation Sequence Intake`로 명명하고 clip 추가, 순서, wall duration,
  target pattern/stage 메모와 원자적 저장을 유지한다.
- 이미 promotion되어 Server manual audition에 들어간 sequence는 Workbench의 pattern 목록과
  `Play Server Pattern`에서 재생한다.
- 새 chain을 곧바로 Product로 승격하는 자동 버튼은 manifest review, WModel duration freeze,
  gameplay/presentation 원자적 projection을 우회하므로 이번 G에서는 만들지 않는다.

### G6. 지원 범위 표시

- Valtan은 실제 `VALTAN_ARENA` Server authority 경로를 사용한다.
- KakulSaydon은 현재 저장소에 Product world/encounter/pattern authority가 존재하는지 검사하고,
  없으면 Workbench에서 지원된 것처럼 local mock을 만들지 않는다.

## 5. 검증

- Workbench C++ contract test: stable-ID stage edit, 명령 routing, cue lane/deep-link token 검사
- 기존 Valtan pattern tree / tuning / boss tool contract
- Valtan publisher `Validate`
- Client Debug compile 또는 Product profile
- `git diff --check`
- 실제 Valtan Arena의 화면·소리·grab·damage 결과는 사용자가 F1 Workbench와 Server 실행으로 최종 판정
