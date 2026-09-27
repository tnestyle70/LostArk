# 마하라카 대체 NPC 28명 원본 주민 교체 결과

## G01. 실제 설치·게시한 범위

2026-09-27, `codex/maharaka-map-restoration`의 현재 저장본에서 재구성 NPC 28명을
원본 SCENE06A 24명과 SCENE07A 4명으로 교체했다. 기존 모코모코·워터캐논 2개와
플레이어 시작점 4행은 완전히 보존했다. 최종 World는 NPC 30명 + 시작점 4행이다.
다른 지역에서 가져온 `npc.maharaka.reconstructed.*` 배치는 이 Area에 남아 있지 않다.

원본 actor export identity → LookInfo → body/head mesh → material/variation → AnimSet
연결을 사용했다. 모양이나 이름이 비슷한 다른 지역 주민으로 치환하지 않았다.
인원 제한을 위한 28명 선택은 저작 선택이며, 원본 전체 111개 씬 배우 복원 주장은 아니다.
Deploy numeric NPC ID와 안내원 역할은 확인되지 않아 provenance에 null로 남겼다.
원본 주민을 안내원으로 임의 지정하거나 이벤트 기능을 새로 구현하지 않았다.

### 실제 데이터·리소스

- `Data/Actors/NpcCatalog.json`: 원본 주민 archetype/model 20개와 native material override 40행 추가.
- `Data/Actors/MaharakaResidents.source.json`: 28개 원본 actor, 외형·색·mask·동작·좌표 및 보존 6행 증거.
- `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`: revision 6, 대체 28행만 교체.
- `Server/Bin/DataFiles/World/MAHARAKA.worldbootstrap`: World publisher로 34행 게시.
- `Client/Bin/DataFiles/World/MAHARAKA.npcpresentation.json`: publisher 생성본. 배치별 추가 동작 override가
  없으므로 entries는 비어 있고, 주민 동작은 NpcCatalog의 idleClip을 소비한다.
- `Client/Bin/Resources/Character/NPC/Maharaka/Residents/`: 20개 WModel과 원본 DDS, 총 122개 파일.
  동일 외형·동작·속도 조합은 같은 모델을 공유한다. 20모델은 NPC가 20명이라는 뜻이 아니다.

복구 도구는 `Tools/WorldPipeline/replace_maharaka_source_npcs.py`다. `extract`, `materials`,
`cook`, `install`을 분리하며 `install`은 이미 교체된 배치를 다시 덮지 않는다.
`basis`는 이번 설치 hash가 유지된 모델만 signed tangent basis로 승격한다.
이전 28배치와 catalog, geometry backup은 `out/MaharakaMapRestoration20260927/npc-replacement/backup/`에 있다.

### 동작과 좌표

8종 원본 loop를 연결했다: `idle_normal_1`, `sc_laughing_loop_1`, `evt2_float_1_01`,
`evt2_lie_1_01`, `evt2_water_tube`, `evt2_wplay04_loop`, `evt2_wplay02_loop`,
`act_sitdown_m_loop_1`. 원본 full-body loop가 없는 배우는 그 배우의 원본 AnimSet idle을 선택했다.
원본 speed 차이도 clip 시간에 반영하고 PSA scale keys를 보존했다.
Matinee 전체나 다중 animation track/state machine을 복원했다고 보고하지 않는다.

원본 UE cm 좌표는 `[X/100,Z/100,-Y/100]`, 배치 yaw는 기존 NPC 모델 pre-rotation을 상쇄하는
`sourceYaw+90`을 사용한다. 앉기·눕기·수영 배우는 원래 높이를 유지한다. 임의 nav snap이나
건조한 바닥만 고르는 이전 재구성 조건을 적용하지 않았다. `behavior:null`은 Server 초기화에서
이 배치를 navigation에 재투영하지 않는 기존 경로다.

## G02. 원본 재질을 실제 소비하도록 보완

원본 MIC 5개를 native GPU-skin Base/Light와 uniform-expression으로 추출했다.
기존 프로그램과 전체 일치하는 것이 없어 기존 생성기로 1474/1475를 등록했다.
여성/남성 그룹 구분은 원본 DXBC와 CPU packing 일치에 근거한다. 피부·옷 색상 네 벡터와
mask variation, diffuse/normal/color mask/pattern mask/specular/state texture를 연결했다.

독립 검토 지적 두 개를 실제 데이터로 재현하고 수정했다.

1. `Shader_SourceCharacterMaterial.hlsli` 양쪽 사본의 Light 입력에 1474/1475를 추가했다.
   원본이 읽는 UV/light/view/position은 v2/v3/v5/v6이고 Base는 기존 공통 입력과 일치한다.
2. legacy WModel 1.0에서 버려진 tangent.w를 보존했다. staged glTF와 설치 모델의 모든 P/N/T·UV0·
   weighted bone name/weight를 정확히 대조했다. Z 반사·UV 보존 변환이므로 `-source TANGENT.w`를
   기존 1.5/80-byte 계약으로 저장한다. 원래 76-byte 정점, index/bone/bounds 및 animation section은 보존한다.

독립 재검토가 20모델·196,940정점의 geometry delta 0 및 재생성 파일/설치 파일 바이트 일치를 확인했다.
실제 런타임은 기존 `CClientReplication → CActorCatalog → CNpcPresentation → CModel/CMaterial`을 사용한다.
별도 NPC loader나 presentation mock을 추가하지 않았다.

## G03. 자동 검증과 수동 확인 경계

- 원본 5 MIC의 설치 CPU packing/Base/Light 전체 일치 검사 PASS.
- `python -X utf8 -B -m unittest Tools.WorldPipeline.test_maharaka_npc_population -v`: 6/6 PASS.
- 20모델 weighted bind 공통 basis 오차 최대 약 0.0002012, 각 loop 0/25/50/75% 정점 샘플 finite 및
  비정지 검사 PASS. 이 숫자는 뼈/geometry 무결성 검사이지 원본 영상과 픽셀 일치 검사가 아니다.
- Client project/filter XML parse PASS. Resources 122파일, 153,659,360 bytes.
- `Publish-WorldGameplay.ps1 -WorldId MAHARAKA -Mode Validate`: 34 placements PASS.
- 같은 publisher Publish: Client/Server 게시본 생성 완료. 생성물을 수동 편집하지 않았다.
- 관련 `git diff --check`: 오류 없음. CRLF 정규화 안내만 존재한다.
- 1차 Debug Product 빌드 PASS (`out/BuildPipeline/runs/20260927T130724328Z-debug-product.json`).
  빌드 도중 수정한 공통 Light 입력까지 포함한 최종 증분 빌드도 PASS:
  `out/BuildPipeline/runs/20260927T131912236Z-debug-product.json`, elapsed 658,989 ms.
  `Debug -Profile Product -MaxCompilerProcesses 4`, Clean/Rebuild 또는 shader skip 없이 실행했다.
- 기존 headless `Test-SourceCharacterShaderVariants.ps1 -Configuration Debug` PASS.
  신규 1474/1475 포함 program checks 140, clone checks 14, failure cases 9,
  light passes 528, native passes 144, afterimage passes 4, unavailable passes 10,
  combat passes 12, draws 46, windowsCreated 0.
  증거: `out/MaharakaMapRestoration20260927/npc-replacement/shader-probe/probe.log`.
- 추가 Server `--contract-test`에서 쿠크 마리오 terminal return/두 패턴 완료/landing 대기 검사 실패가
  확인됐다. 전체 회귀 PASS로 처리하지 않는다. 해당 fixture는 `CGameRoom(WORLD_ID::KAKULSAYDON_ARENA)`와
  쿠크 Gate 3 게시 데이터를 소비하며 이번 변경의 마하라카 placement나 Client resident shader는 읽지 않는다.
  Server/Shared C++ 및 쿠크 authoring은 이번 요청에서 수정하지 않았다. baseline 대조 재실행으로 원인을
  확정한 것은 아니며, 이 NPC 교체 작업에서 별도 전투 로직을 임의 수정하지 않았다.
  관찰한 실패 assertion은 9건이다. 전체 회귀가 이미 실패한 상태에서 이 작업의 전용 NPC 검증과
  관계없는 나머지 시뮬레이션을 계속 대기하지 않고, PID/실행 경로/`--contract-test`를 확인한 뒤
  에이전트가 시작한 테스트용 Server만 fail-fast 종료했다. 실행 종료 코드는 -1이며 전체 suite 완주는 아니다.
  접속용 팀 Server를 종료한 것이 아니다. 로그는 `out/MaharakaMapRestoration20260927/npc-replacement/server-contract.log`다.
- Client/UI를 에이전트가 실행하거나 캡처하지 않았다. 실제 화면·성능·기능 동작은 사용자 확인 전이다.

## G04. 실행·팀 배포 경계

현재 Client endpoint 정본은 `192.168.0.22:7777`이다. 로컬 디스크의 게시 완료는 이미 실행 중인
팀 Server의 메모리 적용이나 원격 배포 완료가 아니다. Server 소유자가 변경된 정본/게시본을 반영하고
Server를 다시 시작해야 새 archetype·placement를 전송한다. Client도 새 실행 파일과 셰이더로 다시 실행해야 한다.
사용자는 평소 입장 경로로 마하라카에 재입장해 대체 주민이 원본 수영복·피부·머리와 loop로 나오는지 확인한다.

리소스 배포가 필요한 폴더는 `Client/Bin/Resources/Character/NPC/Maharaka/Residents/` 전체다.
Resources-relative asset root는 `Character/NPC/Maharaka/Residents/`이다. 모델과 shared `Textures/`,
각 모델의 `textures/`를 함께 전달한다. Git의 catalog/world/provenance 및 정상 빌드한 shader도 함께 필요하다.
Drive 업로드, Git commit/push, 원격 Server 변경은 이 요청에서 수행하지 않았다.

## G05. 후속 전달 준비와 PR 검사 (2026-09-28)

사용자의 후속 요청으로 기존 CY_Resources와 같은 Resources-relative 폴더 구조의
CY_Resources２를 바탕 화면에 준비했다. Character 152개, Map 3564개, Sound 14개,
총 3730개/1,241,075,852 bytes다. 기존 배포 경로 누락 0, 원본 받침·주민 20모델·
워터팡 WAV 2개 포함 및 복사본 바이트 일치를 확인했다. Drive 업로드는 아직 하지 않았다.
이는 전체 게임 Resources가 아니라 기존 설치에 추가·덮어쓰는 마하라카 리소스다.

PR 준비에서 Area Check(4671배치/7파일), World MAHARAKA Validate(34배치)를 재실행했다.
기존 받침 검사 하나가 게시 decimal 직렬화의 마지막 double 자리 차이(약 4e-15)만으로
실패해 좌표 비교를 1e-12 절대 오차로 바꿨다. 런타임 좌표 자체는 변경하지 않았다.
받침 원본 계약 5, 받침/기둥 6, 워터팡 8, 누락 재질 5, 원본 대조 5검사 PASS.
저작/게시 JSON과 project/filter XML도 parse했다. 첫 묶음 unittest 명령은 도구의
sibling import 경로 때문에 5개 모듈을 로드하지 못했고 개별 스크립트 경로로 다시 실행했다.
Debug Product 결과는 위 최종 성공 빌드를 사용하며 별도 Release/광역 재빌드는 하지 않았다.
실제 화면·음향 확인 대기와 위 쿠크 Server 회귀 실패 기록은 그대로 유지한다.
