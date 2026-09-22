# Effect Tool 개별·그룹 회전 연결 결과

## G00. 상태와 적용 경계

2026-09-22 14:12 KST에 기존 Tool H/CPP 5개와 fog의 6개 stable field를 실제 반영했다. `out/EffectRotation20260922/applied-manifest.json`에 직전 hash·백업·원자 교체 결과를 기록했다. 저장된 사용자 각도·위치와 무관한 데이터는 보존했고 삭제된 바람 요소는 복원하지 않았다. 설치 및 자동 검증은 완료했으며 Client/UI 조작이나 화면 확인을 뜻하지 않는다.

## G01. 원인과 구현

기존 개별 Transform Detail은 S·R·T의 R만 바꾸므로 요소 위치는 같은 앵커 주위를 공전하지 않았다. 별도 Group Rotation만 위치·방향·속도·끝점을 quaternion delta로 함께 바꾸고 있었다. 이제 개별 Detail도 현재 draft를 담은 staged document에서 같은 helper를 단일 stable element ID로 호출한다. 기본 pivot은 Anchor origin이며 Group center, Custom point, Element origin을 명시적으로 선택할 수 있다. Element origin은 기존처럼 자기 위치를 유지한 회전이다. pivot 선택은 도구 세션 상태이며 결과 TRS가 기존 JSON으로 저장된다.

source fixed/rotate-axis sprite는 Playback이 emitter 방향을 계산해도 최종 quad가 원본 고정축으로 면 방향을 다시 만들 수 있었다. 재질이 emitter frame을 소비하면 면은 고정되고 내부 문양만 움직이는 것처럼 보일 수 있다. 명시적인 개별·그룹·socket anchor 회전에서만 기존 `followEmitterAxisRotation`을 활성화한다. 위치만 바꾸거나 camera/velocity/mesh/decal 요소를 편집할 때는 flag를 강제하지 않는다. 원본 runtime 기본값·shader·sourceRecipe는 바꾸지 않았다.

단일 선택의 eligibility는 그 요소만 판단하므로 관련 없는 animated rotation 형제가 편집을 막지 않는다. group/anchor 소유권과 finite/range 검증은 유지한다. mixed parent 좌표계의 raw 위치 평균은 올바른 공통 중심이 아니므로 해당 그룹의 Group center 회전만 거부하고 사유를 표시한다. Anchor/Custom/Element origin은 선택 요소의 parent 공간에서 동작한다. source transform track, master transform, runtime carrier는 기존 owner를 유지하며, 해당 개별 Detail은 제한 사유와 기존 Local Rotation 경로를 표시한다.

변경 후보는 `Effect_Tool.h`, `Effect_Tool_Internal.h`, `Effect_Tool_Helpers.cpp`, `Effect_Tool_Detail.cpp`, `Effect_Tool_MaterialDetail.cpp`다. 기존 파일 인코딩 UTF-8(BOM 없음)·CRLF를 유지했고 신규 C++ 파일이나 프로젝트 등록은 없다.

## G02. 실제 저장본과 P23 검증

P11 `effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_fog_01_loc_int`의 9개 중 **6개가 fixed-axis, 3개가 camera-facing**이다. 저장되어 있던 회전·위치·scale·재질은 보존하고 6개 `detail.sprite.followEmitterAxisRotation`만 후보에 추가했다. 초기 조사에서 9개 모두 고정축으로 본 표현은 실제 classifier 검사 후 정정했다.

P23의 현재 저장본을 그대로 Codec으로 읽어 helper/Playback을 검사했다.

| 자산 ID | 요소 | 그룹 회전 | 800ms 실제 재생 |
|---|---:|---|---:|
| `effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_c_loc_int` | 13 | 가능 | 입자 62 |
| `effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_music_loc_int` | 8 | 가능 | 입자 76 |
| `effect.kouku.source.fx_mn_rpcz_00_g.par_g_rpcz_00_trumpet_d_loc_int` | 7 | 가능 | 입자 43 |
| `effect.kouku.common.circus.outerdonut.warning` | 1 | source track owner 제한 유지 | decal 1, 개별 Local Rotation 반응 |

원본 트랙과 사용자 저장 타이밍을 보존했다. 회전한 문서의 save/reload는 canonical serialization이 일치했다. 기존 문서로 800ms까지 재생한 객체에 회전 문서를 재stage하고 같은 cursor로 Seek한 결과는 새 회전 instance와 일치했다. 182개 transform 모두 바뀌었으며 그중 122개는 world-space 입자였다. 이는 world-space 입자의 기존 birth transform이 새 preview에 남는다는 가설을 지지하지 않는다.

## G03. 실행한 검증

`out/EffectRotation20260922/rotation-candidate-receipt.json`과 `rotation_probe.log`에 **8,503 checks PASS**, 최대 matrix 오차 `3.8147e-6`을 기록했다.

- 후보 helper 본문을 그대로 추출·컴파일했다. group/individual anchor/custom, 복합 Euler와 비균일 scale, 속도/끝점, 절대각 반복, 잘못된 group/다른 group의 target, animated sibling, mixed-parent center 거부 시 mutation 0을 검사했다.
- 실제 fog Codec/Playback/final quad 2,144쌍을 비교했다. 별도 실제 local mesh particle은 54쌍을 비교했다. synthetic geometry만으로 실제 asset 검증을 대신하지 않았다.
- Required PSA reset, EPAL_NONE 유지, Required 뒤 AxisLock, disabled AxisLock, velocity, EF/seeded normalization의 6개 순서 사례를 실제 runtime resolver와 final quad로 비교했다. 이 중 portable cardinality가 유효한 5개는 실제 Playback에서도 검사했다. AxisLock 2개인 EPAL_Z→EPAL_NONE 사례는 실제 resolver 원문 검사이며 전체 문서 admission 성공으로 기록하지 않는다. 제품 allowlist/cardinality를 완화하지 않았다.
- 후보 Tool CPP 3개를 scratch compile하여 모두 PASS했다. 기존 header 문자집합 경고와 DirectXTK PDB 경고는 남는다. console 종료 시 연결된 static/runtime dependency의 CRT leak dump가 있어 메모리 누수 없음은 주장하지 않는다.

처음 fog quad 기대값에서 0.459629 차이가 났지만 emitter world 오차는 4.76837e-7이었다. 원인은 camera-forward 0.5m offset까지 pivot 회전시키던 비교식이었다. 실제 Geometry는 world camera offset을 나중에 적용하므로 기대값에서 offset을 제거한 후 delta를 적용하고 같은 offset을 다시 더했다. renderer 코드는 변경하지 않았다. 최초 로그는 다음 실행으로 덮였으므로 원본 로그인 것처럼 재생성하지 않고 도구 출력의 정확한 진단을 `history-camera-offset-expectation.md`에 보존했다.

## G04. paused preview 및 mesh 경계

독립 코드 추적에서 Detail preview와 group commit 모두 `Stage_WorldPreview`를 호출한다. active sequencer의 `Refresh_Effects`는 재생 중/paused와 무관하게 row.v1·snapshot·anchor history·sample age를 초기화하고 현재 clock에서 새 객체를 sample한다. direct preview도 새 `CEffectPlayback`을 stage한 뒤 현재 cursor로 Seek한다. Seek의 Reset/fixed-step 재생은 `SpawnRootWorld`를 새 문서로 다시 생성한다. 따라서 이 경로는 추가 수정하지 않았다. 자동 검사에서 current cursor restage를 확인했으나 실제 ImGui interaction은 실행하지 않았다.

P100 mesh 독립 감사는 `out/Gate1EffectReaudit20260922/source-mesh-rotation-review.md`에 있다. 남아 있는 mesh 8개는 renderer locked-mesh predicate에 해당하지 않으며 실제 WModel 정점과 현재 변환식 14개 검사가 회전에 반응했다. TypeData 초기 투영 누락 4개는 편집 회전 무효와 다른 문제다. 이번 변경은 그 basis나 사용자가 삭제한 emitter를 임의 복원하지 않는다. world-space 기존 birth는 일반 재생 중 고정되는 것이 정상이며, 편집 preview의 restage는 이를 새로 계산한다.

실제 GPU draw, Client Effect Tool 입력, 최종 게임 화면은 이번 검증 범위에 없다. root의 실제 설치·Product 빌드와 사용자의 Reload/화면 확인을 구분한다.

## G05. 최종 설치·빌드·리소스

공용 Product 빌드 `out/BuildPipeline/runs/20260922T052333767Z-debug-product.json`은 PASS다. 해당 빌드의 Client diagnostic log에서 수정한 Detail/Helpers/MaterialDetail CPP의 실제 컴파일과 오류0·Client.exe 링크를 확인했다. 세 OBJ는14:22:54~58 KST, Client.exe는14:23:31로 소스 반영14:12:13 이후다. 후보 적용 이후 소스 hash가 일치하는 것도 재확인했다. 따라서 시작 시각만으로 반영 이전 빌드로 분류하지 않았으며 회전 코드가 설치 Debug 실행 파일에 포함됐다.

root가 추가 호출한 `product-build-final.log`의 중복 Product 빌드는 이후 사용자가 실행한 Client PID31012·Server PID52436의 출력 점유로 시작 전에 거절됐다. 자동 종료나 출력 우회는 하지 않았다. 이 중복 시도 실패와 앞선 실제 반영·컴파일·링크 성공을 구분한다.

Kouku owner 게시 최종 확인은 revision2195로 PASS했다(`publish-final.log`,237528ms). 이번 Effect authored 문서는 기존 runtime의 프로젝트 Data 소비 경로로 읽으며 별도 바이너리 리소스는 추가하지 않았다. 앞서 추가한30개 리소스339,604,137바이트는 `C:/Users/user/Desktop/GBResources`의 Character/Effect/Sound/UI 상대 경로에 있고 설치 Resources와 SHA/크기가 모두 일치한다. 최종 변경 JSON/XML33개 parse, 설치6파일 SHA, 리소스30쌍, `git diff --check` PASS는 `final-check.json`에 기록했다.
