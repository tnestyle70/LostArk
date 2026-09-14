# 쿠크 2관문 진입·3관문 진입 전체·빙고 최종 엔딩 — 카메라·맵 애니메이션 1차 결과

2026-09-14 PR #384 병합에서는 사용자가 완성본으로 지정한 `codex/sequence-capture-focus`의 Sequence/World 정본을 보존했다. 아래 G15의 추가130소품·14 WORLD 및 P3 카메라7컷 설치는 해당 작업 당시 기록이며 현재 통합본의 설치 상태가 아니다. 현재 P3는 WORLD17개·CAMERA5개다. 병합 기준과 검증은 [PR384 동기화 결과](../09-14/2026-09-14_PR384_CANONICAL_SEQUENCE_SYNC_RESULT.md)를 따른다.

계획은 `.md/GB/09-12/2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_IMPLEMENTATION_PLAN.md`의 G12다.
이번 결과는 그 계획의 **카메라·맵 애니메이션 부분**만 다룬다. 배우(세이튼·쿠크·무기·부착) 저작, 조명·암전·재질·FX는
이 결과에 포함하지 않는다. 사용자 화면 판정은 남아 있다.

## G12-R1. 원본 연결표 — 게임 파일에서 읽은 사실

세 컷신의 원본은 설치된 게임 패키지(`ReleasePC/Packages`)를 팀장 파서(`build_gate2_intro_composition.extract_scene`)로 읽었다.
영상 3개는 장면 구분 참고로만 사용했고 수치는 전부 원본 Matinee에서 가져왔다.

| 대상 | 원본 | Director 컷 | 움직이는 맵 소품 | 배우 |
|---|---|---|---|---|
| 2관문_진입 | SCENE04A / Matinee2 (export 329/394), 27,000ms | 5컷 (-1.01→cam1, 2.10, 13.95, 19.49, 23.95) | 소품 171개: 촛대 2, 의자 4, **카메라 더미에 붙은 책상·카드무대 조각 165** | 세이튼, 쿠크, 책 2, 테이블, PC 4 |
| 3관문_진입 | SCENE02A / Matinee10 (63/117), 35,368ms | 13컷 | **0개** (움직이는 것은 배우·이펙트·카메라뿐) | 대형 세이튼, 쿠크, 도착 세이튼, PC 4 |
| 빙고_최종엔딩씬 | SCENE01B / Matinee0 (32/45), 49,083ms | 10컷 (-0.133→cam01 … 37.0→cam08) | **0개** | 세이튼_1, 쿠크(0.3배), wp2, 쿠크세이튼, 쿠크2(세이튼 날개 뼈 부착, 0.3배) |

- 3관문·빙고에는 원본에 움직이는 맵 소품이 없다. `static_worlds`가 두 씬에서 0개를 반환하는 것을 생성 스크립트가 단언으로 확인한다.
- 2관문의 조각 165개는 촬영 카메라가 아니라 `데스크기둥`·`d1~d8`·`tabetcdum` 더미 카메라 액터에 붙어 있다. 팀장의
  `build_gate2_intro_backdrops.py`가 이를 다루지만 데이터에 설치된 적은 없다(Git 이력 0건). 이번에 dry-run하면 16 world / 6,800키 /
  약 3.5MB가 추가되는데 현재 World 문서는 14.39MB로 16MiB 한도까지 2.39MB만 남아 **그대로는 들어가지 않는다**. 미설치·결정 대기로 남긴다.

## G12-R2. 적용한 변경

생성기는 `Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py`(신규)다. 팀장 생성기의 `make_cameras`·`static_worlds`·`resource`·`occurrence`를
그대로 호출하고, 대상 3개 패턴이 소유한 카메라 행만 교체하며 다른 소유자의 박스(암전·이펙트·조명·환경 프로필·World 정의)는 시간만 옮기고 내용은 건드리지 않는다.
`--install`은 세 정본의 바이트 baseline을 확인한 뒤에만 쓴다. 다시 실행해도 안전하다(P8이 있으면 카메라만 교체, P7이 이미 전체 구간이면 박스 이동 생략).

| 패턴 | 변경 |
|---|---|
| P3 `KAKULSAYDON_G1_PATTERN_3` | 표시명 `2관문_진입`. 카메라 5샷 → 원본 재생성 7샷(cam4 구간이 64키 한도로 3조각). 저장본과 재생성본은 평균 0.6cm 차이지만 21.6초 빠른 줌아웃에서 최대 65cm 차이가 났다(팀장 주석의 "150ms 고정 샘플이 63cm를 놓친" 구간). WORLD 17·암전·조명·이펙트 박스는 그대로. |
| P7 `KAKULSAYDON_G1_PATTERN_7` | 원본 구간 [16,710, 35,368] → [0, 35,368]. 카메라 8샷 → 18샷(`kouku.gate3.intro.camera.1~18`, 전용 사본). 배우는 같은 Matinee를 0ms 원점으로 구운 `2관문_클리어`의 world 28/29/30(대형 세이튼·쿠크·도착 세이튼)을 참조. 기존 암전(`presentation.9`)·도착 이펙트(`.10/.11`)·환경 프로필(`sceneprofile.1`)은 +16,710ms 이동만. `enterCombatOnFinish=true` 유지. |
| P8 `KAKULSAYDON_G1_PATTERN_8` (신규) | `빙고_최종엔딩씬`, gate **`BINGO`**(툴의 빙고 Gate 목록; 계획서의 GATE3 표기는 툴 gate 목록과 맞지 않아 교정), target `boss.kakulsaydon.bingo.saydon`, `enterCombatOnFinish=false`, 49,083ms, 카메라 12샷(`kouku.bingo.ending.camera.1~12`). 배우·암전·조명은 아직 없음. |

카메라 문서는 86 → 110샷, 1,594,239바이트(한도 2MiB). Composition revision 14 → 16, Camera revision 81 → 83(설치 2회).

### 팀장 모듈 수정 1건 — `build_gate2_intro_composition.world_pose`

빙고 카메라 12대 중 8대가 바닥에서 100~800m 떨어진 곳에 놓였다. 원인은 `IMF_RelativeToInitial` 트랙 처리다.
UE3 `InterpTrackInstMove::CalcInitialTransform`은 트랙의 **첫 키를 액터 배치 위치에 맞추고**(`InitialTM = KeyTM0⁻¹ · ActorTM`) 이후 키를
첫 키 기준 상대로 평가하는데, 기존 코드는 `배치 위치 + 키`로 더해 첫 키가 0이 아닌 트랙에서 위치가 두 번 더해졌다.
네 씬을 스캔한 결과 첫 키가 0이 아닌 상대 트랙은 SCENE01B 8개(카메라 6, 쿠크, 쿠크2)와 SCENE04A의 미설치 배경 더미 3개뿐이고,
SCENE02A(0개)·SCENE03A(1개, 첫 키 0)·설치된 P3 소품/카메라는 영향이 없다. 수정 후 2·3관문 샷 25개는 수정 전과 바이트 동일했고,
빙고 카메라는 전부 바닥 중심 반경 3m·높이 0.5~10m 안으로 들어왔다.

## G12-R3. 실행한 검증

| 확인 | 결과 |
|---|---|
| 후보 구조 검사 (occurrence ID 유일, 박스가 스테이지 안, 참조 resource/shot/world/profile 존재, 카메라 구간 빈틈·겹침 없음) | 3패턴 PASS, 문제 0 |
| 카메라 키 (≤64, 시간 증가, 유한값, FOV 1~179°) | 대상 37샷 PASS |
| 회귀 (world_pose 수정 전후 2·3관문 샷) | 25/25 바이트 동일 |
| `Publish-MapAuthoring.ps1 -Scope Area -Mode Validate / Publish` | PASS, 배치 3,368·출력 8. 실제 바뀐 runtime 출력은 `LV_LUT_MIDNIGHTC_ED.camerashots.json` |
| `Publish-MapAuthoring.ps1 -Scope Area -Mode Check` | PASS, 배치 3,368·출력 8 (게시 runtime과 정본 일치) |
| `git diff --check` (변경 정본·runtime·도구) | PASS |

C++·HLSL 변경 없음, 빌드 불필요. Client/UI 실행·화면 판정은 하지 않았다.

## G12-R4. 사용자가 확인할 것

`KoukuSaydon → F1 → Open Sequencer Benchmark → Composition Sequencer`

1. `2관문_진입`: 카메라 7샷이 이어지는지, 21.5~22.8초 줌아웃이 이전보다 원본 영상에 가까운지.
2. `3관문_진입`: 0초 카드 무대 출발 → 비행 → 16.7초 천막 도착 → 연기 → 전경까지 카메라 18샷과 배우 3명이 한 타임라인에 있는지. 암전·도착 이펙트가 16.7초 이후에 나오는지.
3. `빙고_최종엔딩씬`: 카메라 12샷이 빙고 바닥을 잡는지(배우는 아직 없어 빈 바닥이 정상).
4. 카메라 박스는 Box Detail → `Open Composition Camera`로 키를 편집·저장할 수 있다. `Set Camera Pos / Capture view`는 트랙 전체를 정지 pose로 바꾸므로 원본 궤적을 보존할 때는 누르지 않는다.

## G12-R5. 남은 것

- 빙고 배우 5명: 세이튼_1은 슬롯 3개(a/b/c), 쿠크2는 세이튼 날개 뼈 부착, wp2는 클립 없는 무기 모델이라 현재 `actor_world`(a/b 슬롯·Book 전용 뼈 부착)로는 구울 수 없다. 계획 G12-07대로 생성기 확장이 필요하다. 필요한 클립은 MN_RPCT_05 WModel(249클립)과 MN_RPCZ_00 WModel(91클립)에 모두 있다.
- 2관문 카드무대·책상 조각 165개: 위 용량 문제와 함께 결정 대기.
- 빙고 암전·조명, 3관문 출발 구간 FX, P8 환경 프로필: 팀장 담당으로 남긴다. P7 이동 박스 4개의 시각표는 G12-R2 표에 있다.

## G13-R1. 사용자 녹화 4개 비교 및 후속 계획 — 구현 전 조사

사용자가 추가한 실제게임/프레임워크의 2·3관문 진입 MP4 4개를 오프라인으로 분석했다. 메타데이터, 전체 구간 접촉 시트, 장면 비교판, 현재 Composition/Camera/World 연결표를 `C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutsceneCompare20260913/`에 남겼다. 사용자가 제공한 녹화의 프레임이며 Client를 실행하거나 화면 캡처한 것이 아니다.

- 녹화 길이: 2관문 참고 27.33초 / 작업본 29.83초, 3관문 참고 36.47초 / 작업본 36.37초. 앞뒤 gameplay와 화면 여백이 달라 같은 초·전체 길이 비율로 직접 동기화하지 않는다.
- 2관문은 책 무대 주변 구성 차이와 쿠크 얼굴 대신 등이 보이는 구도가 관찰됐다. P3에는 기존 의자·촛대 WORLD가 있으므로 이들을 중복 추가하지 않고 부모/위치/표시부터 점검한다. 별도 원본 부착 소품 165개의 backdrop 연결은 현재 P3에 없다.
- 3관문은 출발·비행·도착·후퇴가 이미 포함되지만, 시작과 얼굴 클로즈업의 주인공 구도가 다르다. 비행 터널은 FX 항목으로 분리한다. 화면 차이만으로 카메라/배우 변환 중 하나를 확정 원인으로 선언하지 않았다.
- 현재 revision은 Composition 17 / Camera 84 / World 1736, P3 CAMERA 7·WORLD 17, P7 CAMERA 18·WORLD 3이다. 위 G12-R2 숫자는 이전 설치 당시 기록으로 유지한다.
- 원본 활성 Director와 camera parent/track을 확인했다. 수평→수직 FOV 변환은 기존 생성기에 이미 있다. 비활성 Director와 64-key 분할을 누락/추가 연출 컷으로 오인하지 않는다.
- World 14,387,090 bytes를 compact 형식으로 직렬화하면 메모리상 5,302,425 bytes지만, 툴 Save가 들여쓰기를 다시 쓰므로 설치 시 minify만 하는 방법은 해결책으로 채택하지 않았다.

기존 `2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_IMPLEMENTATION_PLAN.md`에 G13을 추가했다. 범위는 P3/P7 카메라 액션과 맵 애니메이션이며 빙고·배우 재제작·조명·암전·재질·FX는 제외한다. 우선순위는 시각/좌표 대응 확인 → 2관문 무대 소품 연결 → 두 관문의 근접 카메라와 도착/퇴장 구도 → Save/Reload·회귀·사용자 확인이다.

이번 단계의 완료는 **비교와 계획 작성**뿐이다. 연출 코드·JSON·Resources 설치, publisher, 빌드, Client 재생, 수정 결과의 visual PASS는 수행하지 않았다. 기존 G12 검증을 이번 후속 수정의 검증으로 재사용하지 않는다.

## G13-R2. 원작 일치 검증 설명 보강 — 문서만 변경

사용자의 추가 요청에 따라 기존 계획 G13-09~15를 보강했다. 범위는 계속 P3/P7의 카메라·맵 배치/동작이며, 배우 재제작·조명·암전·재질·FX로 확장하지 않았다. 사용자가 G13-05에 직접 추가한 3관문 비행의 “쿠크만 확대돼 보임” 관찰은 원문을 보존하고, 대응 원작 컷에서 쿠크·세이튼이 함께 보이는지를 필수 검증 항목으로 연결했다.

추가 설명은 원본 의미→후보→저장/로드→최종 runtime 값→동기화된 사용자 녹화의 검증 경로, 같은 생성기 함수만으로 정답을 만드는 순환 검증 방지, 시간/viewport 정렬, 카메라·소품 원인 분리, 장면별 완료표를 담는다. 제시한 허용 오차는 앞으로 적용할 작업용 제안 기준이며 실제 통과 수치가 아니다. 자동 검사, 사용자 확인, 원본·범위 제약을 분리하고 미확인 항목은 전체 PASS로 합산하지 않도록 했다.

실제 연출·코드·데이터·Resources는 변경하지 않았다. 이 단계의 검증 대상은 추가 문서와 기존 사용자 편집 보존뿐이다. 빌드·publisher·runtime 진단·새 영상 비교·visual PASS는 수행하지 않았으며 구현 완료를 의미하지 않는다.

## G13-R3. 2관문 무대 소품 165개 설치 — 구현 결과 (2026-09-13)

원본 SCENE04A의 static mesh actor 172개 중 Matinee에 직접 바인딩된 7개(테이블·책·의자 4·촛대 2)를 제외한 165개는 더미 카메라 액터 15개에 hard-attach된 조각이다. 팀장 모듈 `build_gate2_intro_backdrops.py`로 다시 만들어 P3 `2관문_진입`에 설치했다.

| 구성 | 개수 | 키 | 원본 근거 |
|---|---|---|---|
| 정지 조각(책상 기둥·바닥 floor16 타일·의자·촛대) | 50 | 2 | 부모 더미가 움직이지 않음(cameraactor_1, _11~_15) |
| 세이튼과 함께 6.23m 떠오르는 무대 조각 | 35 | 4 | 부모 tabetcdum(cameraactor_37)의 Move 키 |
| 매달린 카드(card01/01a/01b/01i/01p) | 80 | 82 | `efmotionstaticmeshactor` axis_x ±10°, 10초 주기. 원본 키가 아니라 프로그램값으로 재구성한 zero-phase sine(위상은 확인 불가) |

- 좌표 검증: 조각 165개의 첫 키가 기존 P3 테이블·책·촛대(원본 저장값과 0.00m 일치)와 같은 세트 좌표 (-316~-282, -102~-90, 438~461)에 놓이고, 35개는 세이튼 위치(-0.9, 1.3, 737.8) 주변에 놓인다. 원본 부모·상대 Transform·drawscale3d는 각각 한 번만 적용했다.
- **규칙 발견**: 한 인스턴스는 같은 Object Resource를 두 번 바인딩할 수 없다(`WorldSequenceDocument.cpp` Validate `boundTargets`, publisher 1927행). 팀장 모듈은 모델 11종을 리소스로 두고 같은 모델 여러 장을 한 템플릿의 슬롯에 묶어 이 규칙에 걸렸다. 첫 설치가 Validate에서 거절돼 baseline SHA-256 일치를 확인하며 정확히 되돌린 뒤, 모듈을 **조각마다 Object Resource(`…backdrop.<asset>.actor<N>`)**를 갖도록 고쳐 재설치했다. 결과 Object Resource 165 / 템플릿 16(부모별, ≤32 슬롯) / 인스턴스 16.
- **용량**: 기존 문서는 python `json.dumps(indent=2)` 형식으로 14,387,090바이트였고 후보를 같은 형식으로 붙이면 16MiB를 넘는다. 이번 설치부터 World 문서를 `CWorldSequenceDocument::Save`와 같은 형식(배열 한 줄, float32 9자리)으로 쓴다 → **10,361,597바이트**. 기존 모든 행은 float32 값 동일(생성기 내부 재파싱 비교 + `git HEAD` 대조 독립 스크립트 모두 통과, revision 1736→1737). 이후 이 문서를 python으로 다시 쓰는 생성기는 `build_gate_cutscenes_g12.tool_document`를 써야 한다; indent=2로 되돌리면 한도를 넘는다.
- Composition: P3 world 박스 17→33(`.world.18`~`.world.33`, 0~27,000ms), worlds 등록 50→66, revision 17→18.
- 검증: 후보 규칙 검사(stable ID·키 2~256·시간 증가·쿼터니언 정규화·바인딩=슬롯·인스턴스당 target 유일) PASS, `Publish-MapAuthoring.ps1 -Scope Area` Validate/Publish/Check PASS(배치 3,368·출력 8; runtime `worldsequences.json` 10,112,135바이트로 게시).

## G13-R4. 카메라·배우 대조 결과 — 수정한 것과 결정이 필요한 것

카메라는 저장본과 원본 재생성본을 같은 시각에 대조했고(P3 7샷, P7 18샷 모두 0.00m/0.00°), 배우는 설치된 템플릿 키와 원본 `world_pose`를 같은 시각에 대조했다(P3 세이튼·쿠크, P5/P7 3명 모두 0.00m/0.00°). 즉 **카메라와 배우 위치·회전은 원본 데이터 그대로**다. 원본 영상과 다른 장면의 원인은 아래처럼 갈렸다.

| 장면 | 원인 층 | 처리 |
|---|---|---|
| P3 19.5~22초 쿠크 클로즈업에서 등이 보임 | 배우 bake. 같은 시점(카메라 4·7은 같은 자리)에서 25초엔 얼굴이 보이므로 18.08초 `att_battle_5_03` → 20.97초 `att_battle_10_01` 구간의 루트 회전이 bake에 들어간 것 | **제외 영역(배우 재제작)** — 보고만 함 |
| P3 0~2초 무대가 검게 보임 | 카메라 1·2는 축제 무대(Z 735~748, 배치 370개 반경 30m)를 본다. P3에는 씬 프로필 박스가 없고 P1/P4는 같은 위치에 `kakulsaydon.g1.sceneprofile.3`(팝업 프로필)을 쓴다 | **조명 영역** — 팀장 확인 항목 |
| P3 테이블 세트 주변이 암흑 | 원본 영상도 책 주변이 암흑이고, 우리 맵도 그 좌표(-300,-102,450) 반경 80m에 배치 0개(2관문 전투 세트는 (8,10,320)에 따로 있음) | 원본과 같음 |
| P3 23초 촛대가 안 보임 | 위치는 17초부터 페이지 양쪽 모서리(원본 저장값과 0.00m)이고 카메라 6의 시야(20° 이내)에 들어온다. DECO19 wmodel(13,740바이트) 크기·재질은 미확인 | 모델/재질 영역 — 미확인 |
| P7 6.6~16.7초 비행 중 쿠크만 크게 보임 | 원본 Matinee의 배우 `drawscale` 트랙(대형 세이튼 6.0→2.0 @7.657~8.257초, 쿠크 1.0→0.3 @7.76~8.257초)을 P5 bake가 적용하지 않아 두 배우가 3배·3.3배 크다 | **수정**: P7 전용 사본 World `kakulsaydon.g1.world.34`/`.35`(인스턴스 `world.sequence.instance.kouku.gate3.intro.largesaydon`/`.kouku`; P5 world.28/29 키 복사 + scaleMultiplier 키, 221/215키). P5 공용 행은 그대로. Composition reader는 World 등록 ID로 생성 ordinal(`kakulsaydon.g1.world.<n>`, n < nextWorldOrdinal) 또는 `world.kouku.gate2.intro.<x>` 짝만 허용하므로 처음 붙인 `world.kouku.gate3.intro.*` ID는 로드 거절돼 교정했다(nextWorldOrdinal 34→36, revision 20) |
| P7 22~26초 손 위 쿠크·세이튼 얼굴 클로즈업에 손·몸통이 보임 | 도착 세이튼 preScale 0.017(전투 크기)은 원본 cinematic drawscale 1.0×0.01의 1.7배. 머리뼈 138단위 → 원본 1.38m, 현재 2.35m. 카메라 c4_1은 Y 1.9m라 원본 크기에서만 얼굴을 잡는다 | **결정 필요**: (a) P7 사본에서 도착 세이튼을 0.01로 → 클로즈업은 맞지만 컷신 끝에 스폰되는 전투 세이튼(0.017)과 크기 점프, (b) 전투 크기 유지 → 클로즈업 불일치 감수 |
| P7 0~2.2초 도입에서 쿠크가 멀고 세이튼 몸이 왼쪽을 차지 | 활성 c1 트랙은 FOV 60° 고정·쿠크 12m. 같은 그룹의 **비활성** FOV 트랙(60→25→23→35°)과 비활성 Move 트랙 6개가 참고 영상의 구도(쿠크 중앙·근접)에 가깝다 | 원본 데이터(활성)와 참고 영상이 다른 버전 — 계획 G13-10에 따라 활성 트랙 유지, 사용자 기준 선택 필요 |
| P7 16.7~20.6초 도착 전경이 원본보다 가까움, 26~35초 후퇴 구도 차이 | 카메라·세트 좌표는 원본과 같다(도착 세이튼 (-0.02,1.28,942.16) = g3 배치 (-0.07,1.32,942.33)). c4/c6도 비활성 FOV·Move 트랙을 여러 개 가진 그룹 | 위와 같은 버전 차이 후보 — 미확인 |

P7 카메라에는 뼈 부착이 없다(c4·c4_1은 더미 c4_p, c6은 c6_p에 hard-attach). 비활성 Move 트랙은 생성기가 이미 걸러 정확히 하나의 활성 트랙만 쓴다.

## G13-R5. 실행한 검증과 사용자 확인 경로

| 확인 | 결과 |
|---|---|
| 2관문 소품 후보 규칙 검사(ID·키·쿼터니언·바인딩·인스턴스당 target 유일·한도) | PASS (165/16/16, 6,800키) |
| World 문서 재파싱 float32 동일성(생성기 내부) + `git HEAD` 대조 독립 스크립트 | PASS — 기존 315/201/257행 값 동일, 새 행만 추가 |
| `Publish-MapAuthoring.ps1 -Scope Area` Validate / Publish / Check (2관문 소품 설치 후) | PASS, 배치 3,368·출력 8 |
| 3관문 사본 후보 규칙 검사 | PASS (221/215키, scale 1→0.333 / 1→0.3) |
| `-Scope WorldSequences` Publish (3관문 사본 설치 후) → `-Scope Area` Check | PASS; runtime `worldsequences.json` revision 1738, 10,248,736바이트 |
| `git diff --check` (변경 정본·도구·문서·runtime 출력) | PASS (LF/CRLF 안내만) |

최종 revision: World 1736→1738, Composition 17→19. C++·HLSL 변경 없음, 빌드 불필요. Client/UI 실행·화면 판정은 하지 않았다. commit/push/PR은 사용자 지시 대기.

사용자 확인: `KoukuSaydon → F1 → Open Sequencer Benchmark → Composition Sequencer`

1. `2관문_진입`: 13.95초부터 책상 기둥·바닥 floor16 타일·매달린 카드 80장·의자·촛대가 테이블 세트와 함께 나타나고 27초까지 유지되는지. 0~2초에는 세이튼 주변 무대 조각 35개가 세이튼과 같이 떠오르는지. 카드 흔들림 위상이 어색하면 재구성 파형(zero-phase)이므로 알려 달라.
2. `3관문_진입`: 8.3초 이후 비행 구간에서 쿠크·세이튼이 함께 잡히는지(이전엔 3배 크게 보임). 0~8초와 16.7초 이후는 이전과 같다.
3. 2관문 소품은 World Object Tool → Object Resources의 `…backdrop.<asset>.actor<N>` 165개, 모션은 `2관문 진입 촬영 세트 / cameraactor_*` 16개로 편집한다. 3관문 사본은 `3관문_진입 / LargeSaydon`·`/ Kouku`다. P5 `2관문_클리어`는 그대로다.

사용자 결정 대기: (a) 3관문 도착 세이튼 크기(전투 0.017 유지 vs 원본 0.01), (b) 3관문 도입 c1의 기준(원본 활성 트랙 vs 참고 영상 구도), (c) 2관문 쿠크 클로즈업 등 문제의 배우 bake 수정 범위 승인. 팀장 확인: P3 씬 프로필/조명, DECO19 촛대 모델·재질.

## G13-R6. 2관문 소품 165개 — 사용자 화면 확인 후 되돌림 (2026-09-13 23:20)

사용자 재생 화면(스크린샷 2장): 테이블 주변에 floor16 타일이 **바닥 받침이 아니라 수직 커튼 같은 붉은 띠**로 둘러서 있고, 카드 80장이 테이블 위 공중에 줄지어 떠 있었다. 원본 영상의 책상·무대 받침 모습과 다르므로 사용자 판단으로 즉시 되돌렸다.

- 되돌린 것: World 문서에서 Object Resource 165 / 템플릿 16 / 인스턴스 16 제거(revision 1739, 8,246,254바이트, 툴 Save 형식 유지), Composition에서 P3 world 박스 16개·등록 16개 제거(P3 박스 17개, `nextWorldOccurrenceOrdinal` 18, revision 21). 3관문 사본(G13-R4)과 카메라는 그대로.
- 남는 사실: 팀장 backdrop 모듈이 계산한 조각 위치는 카메라 더미의 원본 Transform + RelativeLocation/Rotation 합성이며, 카메라 3·4에서 보이는 결과가 원본 영상의 받침 구조와 다르다. 부모 회전(cameraactor_1 yaw −135°, _11~_15 yaw −59~−205°)이 조각의 상대 위치에 적용되는 방식 또는 `bignorebaserotation` 처리가 의심 지점이지만 이번엔 확정하지 않았다. 후보는 `out/KoukuGateCutscenes20260913/backdrops/`에 남아 있고 정본에는 없다.
- 이번 작업으로 확정된 규칙(인스턴스당 target 유일, World 문서 툴 형식, Composition World ID 규칙)은 유효하며 gotchas에 남긴다.

## G15-R1. 2관문 진입 카메라·무대 소품 — 원인과 적용 (2026-09-14 Claude)

계획서 G15와 측정 기록 G15-11(`.md/GB/09-12/2026-09-12_KOUKU_SOURCE_SEQUENCE_RESTORE_IMPLEMENTATION_PLAN.md`)을 실행했다. 원인이 코드·데이터로 확정된 두 가지만 고쳤다.

### 최초 불일치 1 — P3 카메라 박스가 병합으로 5개로 되돌아감

- 원본 Director 활성 컷은 cam1(−1.01초)·cam2(2.10)·cam3(13.95)·cam4(19.49)·cam6(23.95)이다. 카메라 액터에 부모 부착은 없다.
- 카메라 파일(revision 84)의 샷 7개는 `make_cameras` 재생성과 키 수·시각·eye·lookAt·FOV 모두 0.0 차이다. cam4는 64키 제한으로 샷 4·5·6에 나뉜다. 같은 생성기끼리 비교했으므로 원본 의미 자체의 증명은 아니다.
- 병합 `400439b2`(2026-09-14 02:24)가 Composition을 G13 이전 5박스(revision 41)로 되돌렸다. 카메라 파일은 7분할 그대로 남았다.
  - 박스 4(19.49초, 4460ms)는 샷 4의 2224ms 뒤 마지막 키에 멈춰 있었다.
  - 박스 5(23.95초, 3050ms)는 cam6가 아니라 cam4 중간 샷 5를 재생했다.
  - 샷 6·7은 쓰이지 않았다.
- 투영 계산: 박스 5 구간은 촛대가 화면 안에 들어오는 넓은 후퇴 구도이고, 샷 7(cam6)은 테이블 위 근접 구도다. 원본 영상 23~24초 넓은 구도와 25~26초 쿠크 근접 순서와 맞는 쪽은 7박스 배치다.

적용(`Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`, revision 43→44):
- 리소스: camera.4를 2224ms로 바꿨다. camera.5는 1104ms로 바꾸고 표시명을 cam4로 고쳤다. camera.6(1132ms, cam4)과 camera.7(3050ms, cam6)을 추가했다.
- 박스 7개: 0–2100, 2100–13950, 13950–19490, 19490–21714, 21714–22818, 22818–23950, 23950–27000ms. `.presentation.9`, `.presentation.10`을 추가했고 `nextPresentationOccurrenceOrdinal`은 9→11이다.

### 최초 불일치 2 — 무대 소품 165개를 되돌렸던 원인은 부모 Matinee 트랙 누락

- 165개 모두 hard attach다. 부모 저장 Transform × 상대 Transform이 자식 저장 월드값과 최대 0.013cm·0.039°로 맞는다.
- 회전 변환은 맵 importer와 소품 도구가 무작위 200개 rotator에서 8.9e-16 차이로 같다.
- 원인: `build_gate_cutscenes_g12.install_backdrops`와 `build_gate2_intro_composition.build`가 부모를 `world_pose(rows, None, actor, t)`로 샘플했다. group이 None이면 부모 더미의 Move 트랙이 하나도 적용되지 않는다. G13-R3의 "부모 더미가 움직이지 않음"은 이 호출 결과였고 원본과 다르다.
- 원본 부모 동작(모두 활성 Move 트랙):
  - `d1~d8`(카드 80장): 0.62초에 약 92m 올라가고 23.0~25.5초에 내려온다. 원본 영상 22~26초 카드 비와 시각이 맞는다.
  - `기둥1~5`(floor16 판 25장): 11.2~16.5초에 접혔다 펴진다. 23.5초에 책 페이지(Y −108.5)와 테이블 윗면(−101.4) 사이에서 테이블 가장자리를 두른다. 원본 23~24초 테이블 아래 받침과 같은 자리다.
  - `데스크기둥`(판 25장): 0.88초에 멀리 갔다가 24.56~25.4초에 돌아온다.
- floor16 판과 DECO19 촛대는 두께 0의 세로 판 메쉬다. 트랙 없이 저장 자세에 두면 G13-R6 화면처럼 세운 붉은 띠와 줄지어 뜬 카드가 된다.

적용:

| 파일 | 변경 |
|---|---|
| `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py` | `parent_pose_sampler(rows)` 추가(부모 자신의 Matinee group으로 `world_pose`). `build()`의 소품 호출 교체 |
| `Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py` | `install_backdrops`의 샘플러 교체 |
| `Tools/KoukuSaydonPipeline/build_gate2_intro_backdrops.py` | `exclude_bone_attached` 옵션과 receipt 기록. 부모 그룹 표시명("매달린 카드 dN", "테이블 받침 기둥N", "책상 다리 데스크기둥"). 구간 분할을 트랙별 256키 기준으로 변경(트랙 최대 185키 → 부모당 템플릿 1개, 박스 0~27000ms, 이음매 없음) |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | revision 1766→1767. Object Resource 130, 템플릿 14, 인스턴스 14, 키 16,112. 기존 바이트는 두고 배열 끝에 새 행만 끼웠다(G13 전체 writer는 현재 `mapMaterialBindings.unlit`에서 통과하지 못함). 8,023,551→12,950,927바이트 |
| 같은 Composition | `worlds` 등록 14개, P3 `worldOccurrences` 14개(0~27000ms), `nextWorldOccurrenceOrdinal` 18→32 |

설치하지 않은 소품: 세이튼 `bip001-l-hand`에 붙는 `tabetcdum` 35장. 부모가 bake된 세이튼 손 포즈를 따르는데, 6.8~9.8초 부모 위쪽 축이 수직에서 약 80° 기울어 원본 영상의 "눕힌 책 위 미니 세트"와 맞지 않는다. 세이튼 bake 문제일 가능성이 있어 도구 receipt에 제외 사유를 남겼다.

### 자동 검증 (에이전트 실행분)

| 검사 | 결과 |
|---|---|
| 스플라이스 동등성: 새 World 재파싱 == 기존 문서 + 새 행(정확히 일치), 새 행 텍스트 == 툴 Save 정규화(float32) | PASS |
| 스플라이스 동등성: 새 Composition 재파싱 == 기존 + 의도한 변경(정확히 일치), World ID·인스턴스 짝 규칙, 카메라 박스 연속 | PASS |
| 새로 추출한 SCENE04A 행으로 계산한 원본 위치 vs 저장 키(판·다리 252표본, 카드 위치) | 최대 0.02mm, 회전 0.0° |
| `validate_world_rows`(stable ID, 키 2~256, 쿼터니언, 인스턴스당 target 유일, 템플릿 251/256) | PASS |
| 설치 직전 두 파일 baseline 바이트 일치 | PASS |
| `Publish-MapAuthoring.ps1 -Scope Area` Validate / Publish / Check | 모두 exit 0(배치 3369, 출력 8). 런타임 `worldsequences.json` 12,950,927바이트 |
| 도구 3파일 `py_compile`, `git diff --check` | PASS |

C++·셰이더 변경이 없어 빌드하지 않았다. Client/UI 실행과 화면 판정은 하지 않았다.

### 확인했고 바꾸지 않은 것

- **Table:** 설치 클립의 움직임 구간은 11.27~12.533초(역재생 닫힘)와 15.667~16.967초(펼침)이다. 그 사이와 17초 이후는 포즈를 유지한다. 원본 키(11.0 reverse, 15.66 forward)와 PSA 47프레임 길이에 맞고, 15.66초는 30Hz bake로 7ms 늦다. 원본 비bake 모델(`out/KoukuGate2Restore20260911`)이 이 PC에 없어 뼈 단위 비교는 못 했다.
- **Book:** 15.20~17.20초에 펼치고 이후 유지한다.
- **촛대 2개:** visible 전 구간 true, scale 1이다. 23.95초 이후 넓은 구도에서 화면 안에 들고 카메라를 향한다(`|n·v|` 0.85~0.91). 재질 family는 `bg-source-opaque-masked`다. 녹화에서 안 보인 원인은 배치 데이터가 아니라 재질·렌더 쪽 후보이며 확정하지 않았다(팀장 영역).

### 확정하지 못해 건드리지 않은 것

- **손에 든 책(5~11초):** 원본은 눕혀져 있고 현재는 세워져 있다. 저장 키는 원본 Move 트랙 평가와 최대 2.4° 차이다. bake는 AnimTree `evt2_animblending_mix_scale`의 `dummy001` SkelControl(`scale`, `b_up`)을 이미 적용한다. 원인을 찾지 못했다.
- **0~2.1초 검은 무대, 19.5~22초 쿠크 등 노출:** G13-R4 기록(조명·씬 프로필, 배우 bake) 뒤로 새 증거가 없다.
- **같은 병합 `400439b2`의 다른 되돌림(G15 범위 밖, 수정하지 않음):** P7 `3관문_진입`이 18,658ms 도착 구간과 카메라 8개로 돌아갔다(G13은 35,368ms·카메라 18개). G12 `빙고_최종엔딩씬` 자리(PATTERN_8)는 현재 다른 `1관문_연출` 패턴이다. 카메라 파일에는 G13 샷이 남아 있다.

### 사용자 확인 (대기)

1. Client를 새로 실행한다. 월드 시퀀스 문서가 바뀌었으므로 기존 실행 중 창은 새 데이터를 읽지 않는다.
2. `KoukuSaydon → F1 → Open Sequencer Benchmark`에서 Sequence 목록의 GATE2 `2관문_진입컷씬`을 고르고 Play Sequence를 누른다.
3. 확인 시각:
   - 0~23초: 테이블 위에 떠 있던 카드 줄이 없어야 한다.
   - 11~16.5초: 테이블 받침 판이 접혔다 펴진다.
   - 17초 이후: 받침 판이 책 페이지와 테이블 사이에서 테이블을 두른다.
   - 19.5~24초: 쿠크 근접 → 후퇴 → 넓은 구도가 이어진다.
   - 23~25.5초: 카드가 위에서 내려온다.
   - 24~27초: cam6 근접 구도로 끝난다.
4. 편집 위치: 소품은 World Object Tool의 Object Resources `2관문 진입 무대 / …`(130개)와 모션 `2관문 진입 무대 / 매달린 카드 dN · 테이블 받침 기둥N · 책상 다리 데스크기둥`(14개)에서 고치고 Object Save로 저장한다. 카메라는 해당 CAMERA 박스 → Open Composition Camera → Save Camera. 박스 시각은 Sequence의 Save.

### 인계

- Git 대상: 위 Data 2파일, 도구 3파일, 계획서 G15-11, 이 결과. Area 게시 출력 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json`(LFS)도 같은 변경 단위다.
- 새 Resources 파일은 없다. 소품은 기존 `Map/LV_LUT_MIDNIGHTC_ED/MAP_*_BG_RAD_KOUKUSATON_{FLOOR16A~D,CARD01*}_SM*` 모델을 쓴다.
- 커밋·푸시는 하지 않았다.
