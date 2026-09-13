# 쿠크 2관문 진입·3관문 진입 전체·빙고 최종 엔딩 — 카메라·맵 애니메이션 1차 결과

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
| P8 `KAKULSAYDON_G1_PATTERN_8` (신규) | `빙고_최종엔딩씬`, GATE3, `enterCombatOnFinish=false`, 49,083ms, 카메라 12샷(`kouku.bingo.ending.camera.1~12`). 배우·암전·조명은 아직 없음. |

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
