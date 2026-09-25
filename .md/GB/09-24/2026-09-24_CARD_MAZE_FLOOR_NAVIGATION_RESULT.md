# 카드미로 바닥 navigation 결과

## G00. 반영

카드미로 입장/망원경의 Y=-0.01m와 실제 표시 바닥 placement
`10296705976280178153`의800개 삼각형을 대조했다. 기존 native CNavGridBaker로
동일116×114/0.5m 격자를 다시 굽고 bounds Y를 스폰 중심0.1m로 저장했다.
기존 source의 카드 윗면1.84m·장식2.67m·아래 배경-15.522m를 제거했다.

7,569개 실제 바닥 셀 중 기존 벽 차단2,447개를 유지했다. 실제 바닥 밖의
중복 BLOCKED253행만 없앴고 해당 셀은 계속 NO_SURFACE로 이동 불가다.
이동 가능5,122개 셀은 전부 스폰 연결 성분에 속한다.

최신 원본 hash 확인·백업·원자 교체 뒤 공식 Navigation publisher로 Client/Server를
게시했다. 변경된 runtime은 CardMiro navgrid와 Server navsurface뿐이다.
Client/Server navgrid bytes는 같다. 다른 region과 navregions는 바뀌지 않았다.

## G01. 실행 검증

- source geometry 측정:185개 배치/12개 mesh 조사, decode 오류0.
- native baker 재컴파일·실행 성공. source 높이 범위[-0.0100000007,-0.00999999885]m.
- Navigation Publish 성공.5,122셀 연결, maximum step 약1.86e-9m.
- 기존 native ServerNavigation consumer로 스폰+네 방향5개 바닥 표본 및4개 경로 PASS.
  경로 waypoint의 실제 traversal step도 모두 허용된다.
- 동일 조건을 기존 `ServerGameplayContractTests_CardMaze.cpp`에 추가했다.
  새 제품 Debug/Release EXE의 `--card-maze-contract-test`가 모두 exit 0, failures 0이다.

증거는 `out/RaidRelease20260924/nav/`의 baseline, geometry-summary,
paint-pruning, grid-validation, runtime-consumer 및 publish.log다.
Client 입력은 기존 depth hit 또는 현재 플레이어 Y의 ray/plane 교차를 typed 이동 명령으로
제출하며 Server가 이 navigation으로 판정한다. 실제 Debug/Release 화면 클릭은 사용자 확인 대상이다.

## G03. 반투명 바닥 아래 피킹 수정

사용자는 바닥에 비치는 외형이 정상 반사 재질임을 확인했다. 피킹은 별도 결함이었다.
현재 바닥 placement `10296705976280178153`은 Alpha/BLEND여서 불투명 MRT의
`Target_PickPos`에 위치를 쓰지 않는다. 아래 불투명 면의 유효한 XYZ가 있으면 Controller는
현재 player Y 평면 fallback을 사용하지 않고 그 XZ를 이동 명령으로 제출한다. 비스듬한
카메라 ray에서는 바닥보다 아래의 hit가 XZ까지 벗어나 정상 CardMiro nav에서 거부될 수 있다.
캡처만으로 해당 클릭의 정확한 triangle/placement ID·좌표까지 기록했다고 간주하지 않는다.

기존 CardMazeVisualPolicy의 floor placement/asset 쌍만 MapAssetObject의 PICKING 큐에
참여시켰다. 실제 CModel mesh·world/view/projection·cull/mirror를 사용하며 기존 정적 메시
shader의 끝에 pass25~27을 추가했다. 다른 pass index·바닥 alpha/반사·모델·Nav 데이터는 유지한다.

Engine은 불투명 렌더 직후 실제 depth를 피킹 전용 texture/DSV에 복사한다. 모든 화면 렌더가
끝난 뒤 `Target_PickPos` 하나만 clear 없이 갱신하고, 깊이 비교와 write는 복사본에 한정한다.
따라서 기존 불투명 hit와 앞쪽 카드 벽의 가림을 보존하고, UI/Nav debug의 깊이 기록과 실제
화면 depth는 분리한다. Final material debug view14도 PickPos.W를 읽으므로 최종 순서를
유지했다. 상태 복원, resize 재생성, frame 실패 시 큐·snapshot 정리도 연결했다.

### 실행한 검증

- 실제 Shader_VtxMeshBinary.hlsl을 FXC로 out 경로에 컴파일하고 Effects11/WARP로 실행했다.
  pass25~27의 이름·base-owned annotation·깊이 write·RGBA 비혼합 ABI 3개 통과.
- GPU 수치 사례 11개, 실패 0: 아래 면 대체, 삼각형 밖 기존 hit 보존, 불투명 전경 가림,
  두 렌더 순서의 최근접 선택, world 높이·scale, 앞/뒷면 cull, 원래 DSV 보존 및 나중 UI
  depth 변경과 분리를 확인했다. 임의 Y 상수 치환은 사용하지 않는다.
- 렌더 순서·Target_PickPos 소비자·실패/resize 경로를 코드 검토했고 관련 diff 검사를 통과했다.
  Engine Renderer.h의 기존 비 UTF-8 인코딩과 C++ CRLF를 보존했다.
- 증거는 `out/CardMazePicking20260924/summary.json`, `probe.log`, `PickingWarpProbe.cpp`,
  `fxc.log`, `compile.log`다. 이 검사는 전체 Client 실행이나 실제 화면 클릭 검증이 아니다.
- 정상 Debug Product Build 성공. shader 포함 결과는
  `out/BuildPipeline/runs/20260924T111733214Z-debug-product.json`이며, 최종 depth snapshot
  변경의 Engine 컴파일·Client 폴더 재배포까지 끝낸 결과는
  `out/BuildPipeline/runs/20260924T111810242Z-debug-product.json`이다. 두 결과 모두 PASS,
  runtime 필수 입력 누락/invalid 없음이다. 최종 Engine.dll과 Client 배포본은 같은 빌드이며
  data publisher는 실행하지 않았다. 기존 C4819/C4244 및 source shader 경고는 남아 있다.

새 C++ 파일과 프로젝트 등록은 없다. 실제 Debug/Release 화면 클릭·이동은 사용자 확인 대상이며
Client/UI를 자율 실행·조작하지 않았다. 기존 Nav 게시 결과와 사용자의 미커밋 변경은 보존했다.

## G04. Character Select 진입 실패: picking pass의 family 정책 누락

### 원인과 수정

G03의 새 Picking25~27 pass가 base와 모든 source-group variant 양쪽에
`ProgramVariantPass=1`(BASE)을 선언했다. 실제 CShader::Stage_ProgramVariants는
base-owned pass의 variant에 UNAVAILABLE(2)을 요구하므로 정적 메시 shader 생성이 실패했다.
Character Select 전용 Effect 데이터 문제가 아니라 공용 static mesh family의 회귀다.
G03의 base FX 단독 GPU 수치 검사와 Product Build는 이 runtime family admission을 검사하지
못했다. 당시 컴파일·수치 성공을 실제 Client 진입 성공으로 확장할 수 없다.

Ready_For_CharacterSelect의 visual map 준비가 Ready_StaticMeshShader에서 실패하면서
Loader는 진행 중인 Effect 준비 작업도 취소한다. 뒤늦은 Effect ACCEPT 명령이
`Effect load job is already cancelled`를 표시한 것이며, 새 Loading job의 취소 상태를
재사용한 결함이 아니다. mailbox 재초기화, shader validation 완화, Effect 데이터 수정은 하지 않았다.

기존 ChargeAfterimage와 같은 방식으로 picking PS는 base에서만 컴파일하고,
14개 variant에는 같은 pass 이름·VS input signature를 유지하면서 NULL PS와 UNAVAILABLE을
선언했다. Loader는 shader 생성 실패와 prototype 등록 실패를 구분해 기존 thread-safe status에
남긴다. 취소와 rollback 흐름은 유지한다. 변경 제품 파일은 기존 Shader_VtxMeshBinary.hlsl과
Loader.cpp이며 새 C++ 파일·project 등록은 없다.

### 실제 factory 검증

변경 전 설치 CSO15개를 out에 보존한 뒤 headless WARP에서 설치 Engine.dll의 실제
CShader::Create를 호출했다. 15개 FX가 각각 decode되지만 전체 factory는 null을 반환했다(exit2).
기본 pass3개의 BASE는 정상이고, 14개 group×3개의 annotation42개가 BASE라서 불일치했다.
이는 GPU 피킹 payload 검사와 다른 실제 제품 factory 경로의 실패 재현이다.

변경 후 같은 실제 Engine.dll factory에서 CSO15개·pass28개 전체 생성에 성공했다(exit0).
annotation 불일치0, variant 직접 picking Begin 거절42개, source program 선택이 남은 상황의
기본 picking PS 유지42개가 모두 통과했다. 기본 shader의 GPU 수치11개와 picking ABI3개도
다시 통과했다. GPU 검증에 사용한 CSO와 최종 설치 base CSO의 SHA256이 같다.

정상 Debug Product Build는 PASS(714,495ms)다. Loader.cpp 한 파일 컴파일, static mesh
CSO15개 재생성, Client.exe 링크·배포를 수행했다. Engine/Shared/Server는 변경 출력0이다.
필수 runtime missing/invalid 입력은 없고 data publish는 실행하지 않았다. 기존 source shader
X4000/X3571/X4008 등 경고는 남아 있다. 실행 파일은 `Client/Bin/Debug/Client.exe`이며
2026-09-24 21:30:52 KST에 갱신됐다. 빌드 중 사용자가 시작한 VS 빌드와 잠시 겹쳐 취소를
안내했으며 최종 검사 전 shader compiler 종료와 설치 CSO15개 hash 일치를 확인했다.

증거는 `out/CardMazePicking20260924/FactoryProbe/`의 `before_manifest.json`,
`before_probe.log`, `after_manifest.json`, `after_probe.log`, `PickingFactoryProbe.cpp`와
`out/CardMazePicking20260924/probe-family-fix.log`, `family-fix-summary.json`이다.
정상 빌드 receipt는 `out/BuildPipeline/runs/20260924T123053096Z-debug-product.json`이다.
제품 두 파일의 UTF-8 BOM 없음·CRLF 유지와 git diff --check를 확인했다.
Client/UI는 실행하지 않았으며 실제 Character Select 화면 진입과 카드미로 클릭은 사용자 확인 대상이다.
