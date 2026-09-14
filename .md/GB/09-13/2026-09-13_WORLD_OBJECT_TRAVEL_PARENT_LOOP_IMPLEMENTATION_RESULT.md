# World Object 칼날 이동·수명과 Parent 한 주기 반복 결과

## 현재 완료 경계

Object Travel, 원형 배치 간격, Parent 전체 주기 반복과 중심 원형 서버 판정의 소스를 반영했다.
사용자가 마지막으로 저장한 칼날 P31과 Parent 구성은 자동 변환하지 않았다.
이후 1관문 위치 정합성 수정으로 Boss revision은 512가 됐으며 기존 Pattern 내용은 유지했다.
해당 도구에서 명시적으로 Apply/Save하여 원하는 칼날 하나 또는 여러 개의 주기를 구성한다.
최신 Composition Product와 Gameplay balance 게시, 전체 Debug Product 빌드가 통과했다.
Client/UI 실행과 화면 판정은 수행하지 않았다.

## G01. Object Travel 저작과 저장

변경은 `WorldObjectTool.h/.cpp`의 기존 Box Detail과 WorldSequence 저장 경로를 사용한다.
새 모델 runtime이나 JSON schema를 추가하지 않았다.

- 부모 Object의 `Edit Default Motion`으로 기본 Motion을 열고 `Travel`에서 Start/End,
  Direction Yaw/Pitch, Distance/Speed, Individual Lifetime, Initial/Final Base Facing과
  도착 시 소멸 여부를 편집한다. 자체 회전은 기존 `Self Rotation`으로 설정한다.
- 기존 정적 key + constant velocity Motion은 `Convert to Travel`에서 끝점·기본 자세·
  emission과 자체 회전을 보존하며 기존 LINEAR Transform keys로 변환한다.
  위치 이동은 도착 시 끝나고, Hold는 수명까지 유지하며 Despawn은 도착 때 숨긴다.
  도착 후 Hold 중에도 자체 회전은 계속된다.
- 개별 Lifetime을 저장된 visible key에서 재구성한다. 마지막 emission의 지연을 포함해
  전체 재생 창을 확보하되 Effect가 연결된 기존 ObjectSpan 경로에서 이 지연을 중복 가산하지 않는다.
  첫 Effect 추가·마지막 Effect 삭제·emission 개수/지연 변경 때 개별 수명을 유지한다.
- 복잡한 곡선, 가속·공전·무작위 분산 등의 변환은 이유와 함께 거절한다. 기존 Advanced
  key 편집과 저장 데이터는 유지한다. 실패한 Apply는 기존 draft를 변경하지 않는다.
- Save는 기존 source freshness와 연결 Collider/Logic 동기화, WorldSequences 전용 runtime
  게시 경로를 사용한다. P31이나 기본 칼날 Motion을 에이전트가 자동으로 재저작하지 않았다.

실제 칼날 6개 emission fixture와 생산 Save/Load/Sample_Track을 사용한 1500개 검사가 통과했다.
22m/11000ms 이동, 개별 수명·숨김 경계, 지연 9s→18s, 첫/마지막 Effect, 낮은 속도,
±90도 자세, 실패 보존을 확인했다. 최종 WorldObjectTool.cpp 최소 컴파일도 통과했다.
증거는 `out/WorldObjectTravel20260913/source_receipt.json`과 `probe.run.log`다.

## G02. 원형 배치 앞·뒤 간격

Motion의 `Physics / Emission → Circular Spacing → Radial Offset (m) → Apply`는
Revolution Offset과 authored emission의 XZ 반경을 같은 값만큼 옮긴다.
양수는 맵 중심에서 바깥쪽이며 Y·yaw·지연·회전·모델 크기를 보존한다.
0 이하 반경 또는 유효하지 않은 값은 거절한다. Apply 후 Save한다.

실제 새 3관문 외곽불의 D_CCW/E_CW 각각 10개 emission에 대한 +1m 수치 검사를 포함했다.
원본 D/E/F는 그대로 두고 새 그룹에만 초기 +1m를 적용한 데이터·맵 배포 내역은
같은 날짜의 인형·Object/Sequence Effect RESULT에 기록했다.

## G03. Parent 전체 주기 반복

`KoukuSaydonActionWorkbench::Repeat_ParentCycle`은 기존 확장기로 Parent의 공통 행과
기존 자식 Pattern을 한 주기의 snapshot으로 만든다. 별도 일반 Pattern에 이 주기를 저장하고
원래 Parent는 그 Pattern을 repeat=true인 한 행으로 참조한다. 새 실행 시계는 추가하지 않았다.

`Parent → Pattern Details → Repeat Parent Cycle → Loop Window ms → Apply Parent Loop → Save`
순서로 사용한다. 이후 생성된 행의 `Open Source Pattern`에서 한 주기를 편집하고,
`Window ms / Repeat within window / Apply Pattern Window`로 반복 창을 조절한다.
Parent 수명을 넘겨 늘릴 때에는 먼저 Parent lifetime을 확장한다.

Animation, WORLD, Effect, Collider, Logic, Sound, Camera, Light, Summon, SceneProfile의
기존 실행 관계를 함께 보존한다. 원래 자식 source는 그대로 유지한다. 마지막 부분 주기는
Loop Window 끝에서 잘린다. 전체 확장 예산, 겹침, global completion 등 기존 제한을 검사한 뒤
commit하며 실패하면 draft·ordinal·dirty state·미저장 Effect 배치를 보존한다.

Workbench 실제 TU 컴파일, 기존 native harness Build와 `--kouku-preview-transport-contract`가
통과했다. 3초 주기→6.5초 반복, WORLD/Effect/Collider/Logic ID remap, 저장/재로드,
자식 source 보존, 미저장 Effect 배치, 잘못된 입력 rollback을 검사했다.
증거는 `out/SequenceEffects20260913/parent-cycle-receipt.json`이다.

## G04. 중심 원형 판정과 서버 피해

칼날의 roll/X축 자전과 ground-plane CIRCLE을 분리했다. local offset=0, bone 없음,
균일 XYZ 배율인 중심 원만 translation/scale/visibility를 따라간다. 중심에서 벗어난 원이나
비균일 배율을 조용히 치환하지 않는다. 이동 경로는 기존 Transform keys를 사용한다.
움직이는 중심 원은 STOP/HOLD와 Parent 반복을 사용하며 물리 velocity/공전은 투영하지 않는다.

Client의 원형 preview도 같은 수평 basis를 쓴다. Server ENTER_AREA는 기존 점 판정에 더해
Shared Segment_IntersectsCircle로 이전 tick과 현재 tick 사이의 이동 접촉을 검사한다.
Logic 창·emission 지연·숨김 key·Lifetime을 경계로 구간을 분리해 만료 후 이동이나 숨김 구간을
가로지르는 접촉을 만들지 않는다. Damage/Instant Death는 기존 Logic Result 경로다.

Python 집중 4건, 실제 Server 함수 본문과 Shared helper의 수치 17건,
Client 실제 평면화 함수·DirectXMath 수치 52건, C++ 3개 TU 컴파일이 통과했다.
새 Product Server 실행 파일의 기존 통합 계약에서 추가한 4개 피해 검사도 모두 통과했다.
증거는 `out/DollEffects20260913/blade_circle_collision.receipt.json`이다.

동적 맵 끝/아이언메이든 접촉 시 칼날 즉시 소멸은 구현하지 않았다.
이번 칼날 소멸 편집은 사용자가 마지막에 지정한 끝점·Lifetime 기준이다.
실제 접촉으로 World occurrence를 중단하려면 서버 소유 재생·소멸 연결이 추가로 필요하다.

## G05. 최신 저장본 게시와 검증 범위

Boss Composition revision 511을 그대로 `project_kouku_saydon_composition.py --mode publish`하여
55개 저장 Pattern 중 42개 Product, 11개 Bundle 중 8개 Product, 321 stage를 게시했다.
`Publish-GameplayBalance.ps1 -Mode Publish`도 통과했다.

기존 P32 `세이튼_쇼타임`은 stage와 duration이 없는데 presentation 20개가 있어 raw 전체
validate_document에서는 lifetime 오류가 난다. P32는 HEAD revision 433과 동일하며 이번 변경에서
발생하지 않았다. C++는 그 Pattern을 격리하고 저장 원문을 보존하며 정상 publish 경로도 동일하게
격리하므로 전체 게시가 통과했다. 실제 C++ Reload/Validate/Save 보존 검사와 source hash 확인을
완료했다. 증거는 `out/SequenceEffects20260913/revision511-lifetime-diagnosis.json`이다.

광역 Python module의 기존 fixture 실패를 기능 통과로 기록하지 않는다. 대표 WORLD 실패는
HEAD에서도 같았고 inventory 실패는 새 중심 원 기능을 비활성화해도 재현됐다. 집중 회귀와
실제 최신 저장본 Product 게시 결과를 별도로 기록했다.

## G06. 전체 Debug Product 빌드

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`를 사용자가 실행 중인
VS 18 Insiders의 x64 MSBuild로 실행했다. v143 14.44.35207, SDK 10.0.26100.0을 사용했다.
Clean/Rebuild 또는 shader skip 없이 Engine → Shared → Server → Client가 모두 PASS했다.
전체 133.498초, 각 프로젝트 12.033초 / 0.289초 / 5.686초 / 115.107초이며,
Client.exe, Server.exe와 Engine.dll 및 런타임 의존성을 배포했다. 누락 runtime input은 0개다.
기존 인코딩·수치 변환 경고는 남아 있으며 컴파일·링크 오류는 없었다.
증거: `out/BuildPipeline/runs/20260913T140019411Z-debug-product.json`.

이후 새 Server.exe의 `--contract-test`에서 칼날 추가 4건은 PASS했다.
광역 검사는 exit 1 / failures 15이며, 쿠크 기존 Product 재생·Gaze·Dance·lifecycle 7개와
world bootstrap v7/NPC behavior fixture 8개가 실패했다. 광역 전체 PASS로 기록하지 않는다.
WorldTriggers의 첫 fixture는 v7을 만들지만 실제 parser는 v8/9/10/11만 받아 초기 로드가
거절되고 이후 7개의 이전 placement 보존 기대가 함께 실패한다. 이 8개는 오래된 fixture와
현재 parser 계약 불일치로 확인했다. 쿠크 7개는 전체 원인을 확정하지 못했으며,
해당 Product Pattern 1/2/6/7은 revision 511에 존재하므로 P32 격리 탓으로 설명하지 않는다.
추가 제품 실행이나 저장 데이터 수정 없이 이 경계를 기록했다.
로그: `out/WorldObjectTravel20260913/server-product-contract.log`.
에이전트는 Client/UI 또는 일반 listen Server를 실행하지 않았다. 검증 종료 때 사용자가
Visual Studio에서 별도로 실행한 Client PID 61296 / Server PID 60956과 MSBuild를 확인했으며,
이후 제품 출력·저장 데이터 교체나 추가 빌드는 수행하지 않았다.

최종 13개 적용 원본의 후보 hash 일치, Boss revision 511 원문 hash 보존,
Engine/Bin과 Client/Bin의 Engine.dll 일치, 전체 git diff --check가 통과했다.
최종 증거는 `out/WorldObjectTravel20260913/final_receipt.json`이다.

후속 1관문 Gaze/커튼 좌표와 All Effects 한 줄 목록까지 반영한 최종 제품 빌드도 통과했다.
최종 receipt는 `out/BuildPipeline/runs/20260913T142636354Z-debug-product.json`이다.
Boss 문서는 revision과 Gaze target Z, 기존 커튼 offset Z만 바뀌어 revision 512가 됐다.
모든 Pattern/P37은 보존됐고 최종 적용 문서는 14개다. 이 후속 작업의 구체 내역은
같은 날짜의 인형·Object/Sequence Effect RESULT를 따른다.

## G07. 기존 서버 검사 실패 수정과 사용자 검증 준비

사용자가 실패 항목의 의미를 확인한 뒤, 치명적 문제가 아니라면 현재 빌드로 직접 검증하고
수정사항을 전달하겠다고 했다. 맵 끝·아이언메이든 접촉에 의한 개별 칼날 소멸 확장은 보류했다.
끝점·Lifetime 소멸은 기존 구현을 유지한다. 이 후속 작업의 제품 런타임·저장 데이터 변경은 없다.

앞 절의 광역 15개 실패는 당시 검사 결과이며, 현재 계약에 맞춘 두 test TU에서 원인을 수정했다.
검사를 삭제하거나 제품의 admission/parser를 완화하지 않았다.

| 기존 실패 | 실제 원인과 수정 |
|---|---|
| Gate3 완료/중단 2건 | Gate1 Product를 Gate3 소유자에게 요청했다. Gate3의 실제 Product와 패턴 길이로 검사하고 run 종료를 확인한다. |
| Gaze 1건 | 이전 아레나 고정 좌표를 기대했다. 게시된 teleport 정본과 실제 위치를 비교하며 방향·spawn owner 검사는 유지한다. |
| Dance/Roulette 2건 | 시작 위치 복귀 뒤 첫 root motion까지 진행된 위치를 spawn으로 기대했다. 시작 commit의 복귀와 이후 root origin을 각각 확인한다. |
| lifecycle 1건 | 실제 Prepare→Update 순서를 누락했다. 해당 순서를 따르고 마지막 stage의 PATTERN_COMPLETED와 stage 0의 run COMPLETED를 구분한다. |
| Play All 1건 | 전체 관문 sequence와 마지막 항목 뒤의 전환까지 기대했다. 해당 Gate/placement 순서와 N−1개 전환, 중복 요청의 run 종료 결과를 검사한다. |
| WorldTriggers 8건 | 지원이 끝난 bootstrap v7 입력 때문에 초기 로드와 후속 보존 기대가 함께 실패했다. v8과 requiresInteract 필드를 사용하고 거절·보존 검사를 유지한다. |

변경 파일은 `ServerGameplayContractTests_KoukuProduct.cpp`와
`ServerGameplayContractTests_WorldTriggers.cpp`다. 실제 두 그룹을 기존 worker stack 경로로 실행한
집중 검사는 81 PASS / 0 FAIL, exit 0, 4.659초였다. 두 TU 컴파일·링크와 diff 검사도 통과했다.
증거: `out/KoukuContractRepair20260913/focused-final.receipt.json`.

최종 정상 Debug Product 빌드는 Engine → Shared → Server → Client 모두 PASS했다.
컴파일·링크 오류가 없으며 빌드 증거는
`out/BuildPipeline/runs/20260913T144132231Z-debug-product.json`이다.
그 뒤 새 `Server.exe --contract-test` 전체 검사는 1340 PASS / 0 FAIL, exit 0,
157.440초로 완료했다. 앞서 남겼던 15개 실패가 모두 해소된 실제 제품 실행 파일의 결과다.

검사 전후 두 EXE·두 test TU와 Server runtime 129개 파일 hash가 같았으며, 당시 최신 적용 문서
14개의 parse/hash 보존을 확인했다. 다른 후속 작업의 Sequence revision 27도 그대로 유지했다.
증거: `out/ServerFixtureVerification20260913/receipt.json`, `server-contract.log`.
전체 git diff --check도 통과했다. 사용자가 정상 Server와 Client를 실행한 것을 확인했으며,
에이전트는 Client/UI를 실행·조작하지 않았다. 실제 시각 판정은 사용자의 진행 중 검증으로 남긴다.

## G08. 09-14 반경 기준 명확화와 즉시 런타임 Preview

기존 Radial Offset은 매 Apply마다 현재 반경에 더하는 값이며 Apply 후나 다른 Motion 선택
뒤에도 입력값이 남았다. DragFloat 반환값도 사용하지 않아 조절 중에는 draft가 바뀌지 않았다.
Apply 뒤 기존 Object Preview가 없으면 기본 Gate 3 외곽불은 별도 player의 저장값으로 남았다.

`Orbit Radius (m)`는 현재 반경을 직접 입력하고, `Radial Offset from Saved (m)`는 마지막
Save/Reload 반경 대비 증감량을 입력한다. 두 입력은 매번 현재 문서에서 다시 계산하므로
같은 목표를 반복 입력해도 누적되지 않는다. 큰 반경은 중심에서 바깥쪽이며, 중심의 Map
Position과 저장/현재 반경을 함께 표시한다. 불꽃의 메시 외곽이 아닌 object pivot 반경이다.
`Restore Saved Radius`는 선택 Motion의 반경만 저장값으로 돌린다. 높이·yaw·지연·회전
속도·모델 크기와 다른 저작 필드는 유지한다.

드래그·직접 입력·hover wheel(한 단계 0.05m)은 candidate Validate 성공 후
기존 Seek(current clock)를 호출해 Object Preview를 즉시 시작하거나 갱신한다.
활성 Preview의 시각과 Play/Pause 상태를 유지하고 실패 이유를 표시한다. Save 전 변경은
미저장 draft/Preview이며 기존 Save와 연결 publisher가 영구 저장과 게시를 소유한다.

같은 instance/template의 3관문 불을 Preview하면 준비·Play·초기 Seek 성공 뒤 기본 Gate
Object owner만 suspend한다. 연속 편집은 borrow identity를 다음 Preview에 넘겨 기본불
clone을 매번 복원하지 않는다. Stop은 자신이 빌린 현재 owner만 복원하며 Despawn Fire
Object나 Gate 종료가 이 identity를 해제하므로 제거한 불이 다시 나타나지 않는다.
준비 실패, 기존 Preview 교체, sample 실패, 관문 전환, Despawn 뒤 Stop의 수명을 검토했다.

복원 요청 범위는 사용자가 ‘방금 조절한 외곽불 반경/배치 값’으로 확정했다. 현재 source와
runtime의 여섯 template/instance/resource는 HEAD와 동일하다. 반경은
D_CW 12.6, D_CCW 13.6, E_CW 12.7, E_CCW 11.7, F_CW/F_CCW 각10.8m다.
57m 저장값은 없고 원본 JSON을 쓰지 않았다. 실행 중 툴의 미저장 draft는 에이전트가
초기화하지 않았다. 전체 Reload로 다른 사용자 편집을 버리는 방식도 사용하지 않았다.

실제 ApplyRadialOffset, Find_Track/Sample_Track, Sample_ObjectWorld 함수 본문과 실제
D/E/F 여섯 저장 fixture로 집중 CPU 검증을 실행했다. 각 emission의 24개 시각과 두 중심,
+1m/−0.5m/저장값 복귀, 13.6m 목표 100회 재입력, 잘못된 반경 거절과 원본 보존을
확인했다. 14,400개 실제 World sample 비교에서 최대 반경 오차는 약0.0000611m,
scale/rotation basis 차이는 0이었다. 총95,911 assertion, failure 0이다.
이는 수치 검증이며 실제 Client 화면 확인은 아니다.

증거는 `out/RadialLivePreview20260914/result.json`, `disk-baseline.json`,
`source-inputs.json`, 실제 함수 추출 및 fixture를 기록한 `prepare.py`와 `radial_probe.cpp`다.
검증은 out 아래에서만 수행했고 원본/게시 데이터 쓰기와 UI 조작은 0회다.
MainApp, WorldObjectTool, Level_KakulSaydonArena_WorldObjects 현재 3 TU Debug 컴파일과
`git diff --check`를 통과했다. 컴파일 로그는
`out/SequenceCaptureFocus20260914/fire-radial-compile.log`다. 기존 C++ BOM/인코딩/CRLF를
보존했고 새 C++ 파일·project/filter 변경은 없다. 사용자의 Client/Server는 실행 중이므로
최종 Product Build와 실제 EXE 적용, 사용자의 화면 확인은 남아 있다.
