# 독립 F1 World Scene Tool 구현 결과

## G00. 구현 범위와 사용자 진입

Debug Bern/Character Select/Valtan/KoukuSaydon에서 `F1 → World Scene Tool`을 연다.
별도 Map Tool attach 없이 현재 Level의 기존 map/Deploy 컨테이너를 차용한다.
`Pick in world`와 검색 목록, Focus, source placement/level/asset/WModel 및 실제 mesh/material을
제공한다. 목록 선택은 placement 단위이고 실제 mesh는 world triangle hit에서 확보한다.
선택은 저장하지 않는다. 옵션을 켜거나 사용자 rendering 값을 바꾸지 않는다.
Release에는 이 저작 도구를 노출하지 않는다.

## G01. 맵 선택·편집·저장

`Enable map placement editing`은 기존 CMapPlacementEditSession에 명시적으로 연결한다.
위치(m), 회전(degree), signed scale, Visible, gesture별 Undo/Reset, Duplicate와 session-created
duplicate만의 Delete를 제공한다. 원본 행은 Visible로 identity를 보존한다. VALTAN_PHASE,
backdrop, active borrower와 existing authoring host의 구조 변경 gate를 유지한다.

Partial-live bind에서도 source/live ID와 asset parity를 검증하고 전체 authored 문서를 보존한다.
Save는 sampled runtime pose가 아닌 authored draft를 쓰므로 미로드 행과 원본 motion rest pose가
사라지지 않는다. dirty draft 재바인딩, publishing 중 재바인딩, parse 중 source 변경을 거부한다.
detached draft는 명시적 UI 명령으로만 폐기한다. writer는 고유 temporary를 stage하고 atomic
replacement 직전에 expected source bytes를 다시 대조한다. 공식 Area publisher와 기존
freshness/backup/guarded rollback 경로를 사용한다.

## G02. 애니메이션과 소유권

Deploy는 기존 animation authoring preview로 현재 static/current skeletal pose를 선택한다.
clip 재생·pause·seek·loop·speed, 위치·회전·positive uniform scale와 opacity/Reveal overlay를
제공한다. static/bind-pose에는 root pose-only preview를 사용한다. 자기 Begin 성공만 Stop 권한을
가지고 공유 shader의 preview opacity는 draw 뒤 기본값으로 되돌린다.
valid authoritative state/debris/suppression은 preview를 정상 종료한 뒤 적용한다.
invalid event·중복 state·기존 physics/debris guard를 보존한다. 제품 packet과 Server gameplay
state/navigation을 편집하지 않는다. SOURCE_EXACT Deploy의 영구 배치는 저장하지 않는다.

Map self-motion은 기존 authored mapmotions의 clock만 pause/seek/rate 제어한다.
실제 loader source는 Data/Maps/Authoring/<Area>/<Area>.mapmotions.json이다.
Kouku 기존126행(ROTATION_CYCLIC114/ACYCLIC12)은 원본 게시 placement ID와126/126 매치한다.
Bern/Character Select/Valtan에는 해당 authored 파일이 없어 새 행을 생성하지 않았다.
Character Select 장식2개의 미확정 native 단위/초기 활성화는 이 도구 구현으로 추측하지 않는다.
이 clock은 식생 shader wind의 elapsed clock과 별개다.

자기 Deploy와 map-motion preview는 상호 배타적이다. host/level/area/runtime generation이
바뀌면 새 runtime에 오래된 root/clock을 복원하지 않는다. 다른 sequence가 live map의 소유권을
잡으면 motion lease를 종료하며 그 pose에 이전 time Seek를 하지 않는다.
같은 generation의 saved paused/rate만 복원한다. Hide/Stop/Level 전환은 자기 preview를 정리한다.

## G03. 입력과 프로젝트 연결

F1 launcher/독립 창과 Debug tool visibility, focus, input owner를 연결했다.
pending focus는 한 번만 소비한다. UI capture, one-shot mouse claim, Esc/우클릭/F1 닫기,
Level 변경과 Move Player 등 다른 picker의 소유권을 확인한다. miss는 이전 선택을 유지한다.
지도 static/batch와 Deploy의 실제 triangle distance를 비교해 가까운 domain을 선택한다.
독립 object Visible도 runtime draw에 적용했다.

WorldSceneTool.h, WorldSceneTool.cpp, WorldSceneTool_Animation.cpp 세 파일을 Client 프로젝트와
기존 WorldTools filter에 등록했다. 기존 Prototype/Clone/CModel 경로를 사용한다.

## G04. 실행한 자동 검증

- 실제 수정 TU focused `/Zs`: root scene5개, wind7개와 host/Deploy Debug/Release6개 PASS.
  최종 Animation ownership 변경도 실제 TU `/Zs` PASS.
- 기존 async picking headless D3D11 WARP46 checks PASS
  (`out/AsyncPickingRegression/result.json`). CPU movement dispatch27 checks/0 failures는
  별도 검증이다 (`out/F1WorldMeshInspection/movement-regression/result.json`).
- 수정하지 않은 production inspection method bodies + 실제 DirectX ray/triangle math:
 17 checks/0 failures. CModel 의존은 deterministic seam이며 Client/GPU 화면 실행은 아니다.
- production Bind/End/Discard/Save bodies와 전체 MapPlacementDocument 구현 + 실제 filesystem:
 14 checks/0 failures. source5/live2, 미로드 행 보존, authored pose/Visible, stale write 거부,
 dirty/detached draft와 temporary 정리 포함. host/catalog/publisher boundary는 fixture다.
- production Deploy bodies + deterministic dependencies:31 checks/0 failures.
  static/bind pose, root scale/opacity, state/debris/suppression 선점·최신 packet 보존 포함.
- Client project/filter XML parse PASS. git diff --check PASS.
- Data/Rendering snapshot4개 source 파일의 작업 전후 SHA 동일 PASS.

Product build status: DEBUG_AND_RELEASE_PASS

로그·receipt는 out/F1WorldMeshInspection, out/WorldSceneHostDeploy20261004,
out/WorldSceneToolProduct에 보존한다. 실제 전체 Product build 결과를 이 항목에 갱신한다.

## G05. 복원 결과와 미완료 경계

Bern native RNM4,616 missing chains와 Landscape shadow1 DDS 오류 교정, Bern wind5,901행,
Character Select wind18행·UI PNG10개는 각각 대응 RESULT와 installed receipt를 따른다.
Bern 전체 Area Check는 원래부터 동일했던 effects source/runtime의 CRLF와 publisher LF 규약
차이로 FAIL이다. 양쪽 effects SHA/JSON/91 presentations는 동일하며 초기 감사 snapshot도 같다.
wind Validate와 wind source/runtime equality PASS를 전체 Area Check PASS로 바꾸어 기록하지 않는다.
이 작업과 무관한 effects 문서는 교체하지 않았다.

CPU picker는 static LOD0/current skeleton triangles를 사용한다. alpha holes, shader wind나
displacement, rendered LOD와 animated raster cull의 GPU pixel boundary는 별도다.
사용자 Client 화면에서 실제 피킹·TRS·Save 재진입·clip/sequence와 시각적 원본 동일성은 미확인이다.
에이전트가 Client나 UI를 실행하지 않았다. 인게임 확인과 자동 수치/컴파일 증거를 구분한다.

## G06. 세션 인계 후 최종 빌드와 실행 준비

2026-10-04 `Audit character select and Bern`의 사용자 요청과 중단 상태를 읽고 이어받았다.
Debug 완료 이후 제품 C++/HLSL/project 입력의 추가 수정은 없었다. 실행 중이던 Release
Product를 중단하거나 중복 실행하지 않고 최종 결과를 확인했다. 새로 필요한 코드 수정은 없었다.

| 구성 | 최종 결과 | Product 경과 | Client 산출물 쓰기 | 근거 |
|---|---|---|---|---|
| Debug | compile/link/deploy PASS | 23분 45.874초 | OBJ198 / CSO60 / binary2 | `out/BuildPipeline/runs/20261004T024651513Z-debug-product.json` |
| Release | compile/link/deploy PASS | 23분 3.184초 | OBJ197 / CSO60 / binary2 | `out/BuildPipeline/runs/20261004T031127524Z-release-product.json` |

각 구성의 Engine은 OBJ18 / binary1, Shared·Server는 기존 유효 출력을 재사용했다.
두 결과 모두 `skippedBuild=false`, `missingRuntimeInputs=[]`, `invalidRuntimeInputs=[]`다.
기존 shader·문자 인코딩 warnings는 있으며 빌드 error는 없다. Product 경과는 병렬 compiler의
CPU 시간 합계가 아니다. 공용 wind/Deploy shader 입력과 Engine 헤더 변경의 의존 출력이
갱신됐고, 마지막 대형 Binary FX 컴파일 및 Release 링크 최적화가 끝날 때까지 기다린 실행이다.
Clean/Rebuild, tracking 조작, 추가 전체 감사는 수행하지 않았다.

설치 receipt를 읽어 현재 Bern DDS4619개와 UI PNG10개의 SHA, DDS mip 수를 재확인했다.
Bern wind5901재질/24275배치, Character Select18재질/62배치와 두 mapmaterials의 저작/게시
bytes 일치를 확인했다. 원본 DXBC/GPU 대조는 앞서 실행한 증거를 보존하며 재실행하지 않았다.
렌더링 정본4개의 시작 snapshot SHA도 유지됐다. Client project/filter XML parse와 신규 파일
3개의 중복 없는 등록, 관련 JSON parse와 `git diff --check`를 확인했다.

최종 실행 파일은 `Client/Bin/Debug/Client.exe`, `Client/Bin/Release/Client.exe`다.
독립 저작 도구는 Debug에서만 제공한다. 실행 준비 확인 시 로컬 Server/Client process는 없었고,
Client/UI나 Server CMD를 새로 실행하지 않았다. 현재 Team debugger endpoint 설정은 보존했다.
Client 작업 디렉터리는 `Client/Default`이며 Server 승인 입장은 기존 계약을 따른다.

사용자 확인은 Debug `Bern 또는 Character Select → F1 → World Scene Tool → Pick in world
→ 실제 모델 클릭 → Copy source selection` 순서다. 맵 TRS/Visible·Undo·Save/재진입과
Valtan/Kouku Deploy clip/pose·map motion의 Stop/복원은 사용자 화면에서 확인해야 한다.
CS 회전 장식2개와 기존 감사의 Decal·환경 occurrence·소개 배경 일부 입력은 이번 설치로
완료된 항목이 아니며 통합 보고서의 남은 원본 경계에 유지한다.

통합 산출물은 `C:/Users/user/Documents/Codex/2026-10-04/review-recent-gameplay-fixes-character-select/outputs/rendering-restoration-result.html`
및 대응 JSON이다. 빌드 상태는 두 구성 모두 passed, 화면 상태는 not_run이다.
여러 기능의 미커밋 변경이 섞인 기존 작업본은 자동 stage/commit하지 않았다.
