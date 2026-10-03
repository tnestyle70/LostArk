# 발탄 상승 컷신의 맵 고정 하늘 이펙트 보존 결과

## G00. 확인된 원인과 변경 범위

09-30에 복원한 하늘 자산은 사라지지 않았다. `VALTAN_SIX_PIZZA_106/STEP_01`의
`cue.valtan.six-pizza.sky.spacehole`과 `cue.valtan.six-pizza.sky.chaosgate`가
`Valtan.presentation.json`, 실제 cue projection인 `Valtan.patterneffectcues.json`에
존재한다. STEP_04/05에서 보스 원본 `roar` 연출을 시작할 때, 기존 cinematic suppression이
독립된 map snapshot까지 보스 몸체 이펙트와 함께 숨긴다. `roar`의 16개 effect track
(고유 resource 15개)에는 이 하늘 두 자산이 없어 대체 표시도 발생하지 않는다.

변경 코드는 `Client/Private/Effect_PresentationService.cpp` 하나다. public header,
JSON schema, Data, Resources, shader, 렌더링 옵션 및 원본 시각은 변경하지 않았다.
이 문서는 소스 반영·설치 입력·native CPU 소비자 검증 결과다. Product 통합 빌드와
사용자 화면 판정은 별도로 구분한다.

## G01. 실제 소비 경로와 수정

`CValtan::Spawn_DuePatternEffectCues`는 STEP_01의 두 cue를 map anchor,
SNAPSHOT, ARENA_ABSOLUTE로 만들고 world-root spawn에 넘긴다. 두 cue는 STEP_01의
1200ms보다 긴 유한 tail이므로 `bPreserveBossActionTail`이 설정된다. 이후
`CLevel_ValtanArena::Update_SourceCinematic`이 STEP_04/05에서 `roar`를 재생하고
`Set_CinematicPresentationSuppressed(true)`를 호출한다. 기존 pending/active suppression과
늦게 materialize되는 `Spawn_Immediate` 경로가 이 tail의 inspection visibility까지 껐다.

단일 내부 helper `Is_CinematicIndependentMapTail`이 아래 조건을 모두 확인한다.

- level-owned가 아니며 유효한 world-root handle을 가진 map anchor.
- follow policy SNAPSHOT, `bPreserveBossActionTail == true`.
- stop policy CUE_END이고 유한 duration이 0보다 큼.

world-root adapter가 map 분류를 보존하도록 하고, initial spawn, pending/active
cinematic suppression, local preview에서 같은 helper를 사용한다. owner 일치는
기존 호출자의 검사로 유지한다. 해당 하늘은 기존 inspection 표시값을 그대로 보존하므로
사용자가 따로 숨긴 이펙트를 강제로 켜지 않는다. map 이외의 anchor 정규화, 일반 몸체 cue
제거, 몸체 finite tail의 숨김/복원은 기존 동작을 유지한다.

기존 occurrence를 계속 사용하므로 컷신 시작 때 warmup이나 particle 시계를 재시작하거나
동일 자산을 중복 생성하지 않는다. `WorldRoot` 변환 소비도 그대로이며 map이라는 이름으로
boss bone을 다시 찾지 않는다. cue end, owner 만료, `Stop_BossOwner`, 명시적인
`Stop_WorldRoot` 정리 경로를 유지했다. `Stop_BossAction`은 원래 world-root handle을
제외하므로 이 수정이 그 함수에 pattern 취소 정리를 새로 추가했다고 주장하지 않는다.

## G02. 설치된 자산·위치·재생 시각

원본 복구 근거와 상세 resource 목록은
[09-30 결과 G01~G03](../09-30/2026-09-30_VALTAN_EFFECT_CENTER_AND_SKY_RESULT.md)에 있다.
이번 조사에서도 설치된 source 두 문서와 3 WModel·21 DDS, 총 24개 참조 자원의 존재를
확인했다. 누락은 0이다. 현재 설치 geometry는 WINT 1.1 parser로 읽었으며 원본 게임의
재다운로드 중 파일을 검증 근거로 사용하지 않았다.

| 항목 | spacehole | chaosgate |
|---|---|---|
| Product source | `effect.valtan.sky.source.spacehole` | `effect.valtan.sky.source.chaosgate` |
| 원본 ParticleSystem | `bfx_high_01.valhatron.par_d_spacehole_03` | `bfx_high_00.chaosgate.par_d_hugechaosgate_01` |
| 위치(m) | (156.574375, 60.043125, -121.97296875) | (158.42, 138.4, -125.12) |
| rotation degrees | (180, 0, 0) | (0, 0, 0) |
| occurrence scale | (0.359999992, 0.479999989, 0.359999992) | (0.5, 0.5, 0.5) |
| warmup | 3000ms | 8000ms |
| 표시 끝 | STEP_01 기준 20400ms | STEP_01 기준 20400ms |
| source clock 끝 | 23400ms | 28400ms |

STEP_04 시작은 패턴 시작 후 3400ms, `roar` duration은 7003ms다. 기존 하늘 표시 구간은
이 컷신을 포함한다. 이 값과 source warmup을 그대로 유지했다. 20.4초는 기존 저작 구간이며,
원작의 정확한 activation/비행 시각을 새로 복원했다는 의미가 아니다.

실제 cue 두 개의 `Build_CueScalePolicyRoot`와 world-root adapter를 native probe에서
실행해 위치·회전·축별 scale이 변경 전후 같음을 확인했다. 설치 WModel의 raw bounds는
`installed-audit.json`에 기록했다. 이 bounds는 particle/VS까지 평가한 최종 world AABB가
아니므로 원작 대비 정확한 크기나 live camera frustum 통과 증거로 대신 사용하지 않았다.

native5363~5372는 이미 engine opacity `source[0].x=1`을 바인딩한다. 소비되는 prefix lane,
mesh particle color와 fog carrier를 읽기 검토했으며 0으로 초기화된 engine opacity 때문에
전체가 사라지는 결함은 확인되지 않았다. 일부 additive program의 최종 alpha0은 원본 RGB
마스크 경로와 일치하므로 임의 alpha1 패치나 shader 수정은 하지 않았다. 이는 소비 코드
검토 결과이며 이번 작업의 GPU 출력 검사 결과는 아니다.

## G03. 실제 검증

`out/ValtanSkyCinematic20261004/build_probe.py`가 최신 생산 함수 본문과 실제
`EFFECT_SPAWN_DESC`를 추출한다. DirectXMath와 실제 cue transform 합성, spawn adapter,
product/local preview suppression, owner/world-root stop 및 update의 유한 종료 판단을
실행한다. device/clone admission, owner와 object visibility는 경계 fixture로 대체했다.

| 검사 | 결과 |
|---|---|
| 변경 전 동일 native probe | 40 checks, 7 failures: map 분류·cut visibility·inspection 보존 실패 재현 |
| 변경 후 동일 native probe | 40 checks, 0 failures |
| pending→active와 컷신 중 지연 생성 | 하늘 유지, 일반 몸체 cue 제거/숨김 유지 |
| local preview, 다른 owner, 수동 inspection 숨김 | 기존 독립 상태 보존 |
| 7개 비대상 조건, 유한 cue end, owner 만료·정지, handle별 정지 | 대상 제한 및 기존 정리 경로 유지 |
| 두 실제 cue의 transform·warmup 포함 종료 시각 | 변경 전후 동일 |
| 설치 source/geometry/texture 입력 | JSON parse 및 WModel parse, 24개 자원 누락 0 |
| 변경 TU Debug 최소 컴파일 | exit 0, 기존 EngineSDK C4828 경고만 관측 |
| 해당 CPP 및 문서 `git diff --check` | PASS |
| Client/UI 실행·Reload·GPU draw·원작 fly-through 비교 | 미실행 |

실행 기록은 `probe-before-run.log`, `probe-after-run.log`, `probe-receipt.json`,
`installed-audit.json`, `tu-compile.log`다. 재현 명령은 같은 폴더의
`compile_probe.cmd before`, `compile_probe.cmd after`, `compile_tu.cmd`에 있다.
검증 산출물은 out 영역이며 제품이나 Git 전달 자산이 아니다.

## G04. 통합과 남은 확인

CPP는 집중 검증 후 동결했고 최종 통합 Debug Product 빌드·배포가 PASS했다.
`out/BuildPipeline/runs/20261003T211650083Z-debug-product.json`과 해당 Client 컴파일 로그에서
변경한 Effect_PresentationService.cpp의 컴파일을 확인했다. 같은 작업 폴더의 별도 Bern 최적화
변경을 포함한 통합 빌드다. Release Product도
`out/BuildPipeline/runs/20261003T214827750Z-release-product.json`에서 PASS했다.
후속 Workbench 저장본까지 포함한 최종 증분 Debug/Release Product 결과는 각각 같은 폴더의
`20261003T215100551Z-debug-product.json`, `20261003T215245108Z-release-product.json`이며
모두 PASS다. 새 C++ 파일이나 배포 데이터가 없어 프로젝트/filter 등록 또는 domain publish는
필요하지 않다.

사용자는 새 빌드에서 발탄 106줄 상승 연출의 하늘 표시, 기존 몸체 중복 억제, 연출 종료 뒤
하늘 정리를 화면에서 확인한다. 원본 재다운로드 완료 전의 새 원본 추출·activation 시각 대조와
최종 GPU 표시·크기·카메라 판정은 이번 CPU 검증의 완료 범위에 포함하지 않는다.
