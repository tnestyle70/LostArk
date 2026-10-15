# WModel·UE 에셋 로딩 학습 결과

기록일: 2026-10-14. 목적은 실제 모델 파일의 저장 데이터가 CPU 모델과 GPU 자원으로 바뀌는 과정을 읽고 기술소개서로 설명하는 것이다. 구현 코드는 변경하지 않았다.

## 원고와 조사 기준

- 상세: [Interview 에셋 가이드](C:/Users/tnest/Desktop/Interview/projects/lostark/technical/2026-10-14_WMODEL_UE_ASSET_LOADING_GUIDE.md).
- 발표용 네 장: [기술소개서 작성 원고](C:/Users/tnest/Desktop/Interview/기술소개서/기술소개서_작성원고.md)의 에셋 항목. PPTX 반영은 이번 범위에서 수행하지 않았다.
- LostArk HEAD: `01518032dde86b5558976a2fbe5fd94ad625cfd1`. 기존 미커밋 변경이 있는 작업 폴더를 조사했다.
- UE HEAD: `396c9f059903aed5fec78ecd3d437a40c6415368`. Build.version은 5.8.3. 원본 소스 파일은 수정하지 않았다.
- Interview HEAD: `9b741ab859eaf2964aba9431bc6ac5be56bf553e`. 기존 원고·PPT 변경을 보존했다.

## 실제 모델과 확인 범위

Character Select runtime catalog의 `MAP_2F2A4A698085_BG_ANH_FOLIAGE_CROPS04_SM_KSY_01` WModel을 사용했다. 파일 12,720B, WINT 1.2, 메시·재질 section 2개, 정점 42개와 index 84개다. section offset은 16B 파일 header 뒤 content 시작을 기준으로 한다.

저장 정점 56B가 런타임 `VTXMESH` 76B로 복원되고, 저장 index 16bit가 uint32로 변환되는 코드를 연결했다. 최초 VB 3,192B와 IB 336B는 코드 식과 파일 개수에 의한 생성 요청 크기 계산이다. GPU VRAM이나 로딩 시간 측정값이 아니다.

[바이트 조사 JSON](C:/Users/tnest/Desktop/LostArk/out/AssetLoadingStudy20261014/wmodel-inspection.json)은 파일 길이·header·section·표본 정점과 index·SHA를 보관한다. C++ decoder를 실행한 결과로 표시하지 않는다.

실제 호출은 Loader의 catalog/scope 선택, CModel의 decoder registry 호출, BinaryReader의 파일 read, WMeshReader의 CPU 자료 복원, CMesh의 CreateBuffer, prototype 일괄 등록과 clone의 공유 관계까지 읽었다. UE는 Editor Linker와 cooked Zen을 나누고 일반 StaticMesh resident LOD의 Serialize → PostLoad → InitResources → InitRHI → RHI buffer 경로를 확인했다.

## 탐색 환경

공식 GenerateProjectFiles.bat는 바이너리 의존성과 SDK 누락으로 종료 코드 1을 반환했다. 전체 Setup 다운로드와 엔진 빌드는 실행하지 않았다. [로그](C:/Users/tnest/Desktop/LostArk/out/AssetLoadingStudy20261014/UE-GenerateProjectFiles-initial.log)를 보관했다.

처음 만든 Utility 프로젝트 솔루션은 Unreal 분석 설정이 없어 include·macro에 IntelliSense 오류 표시를 만들 수 있었고, 시작 안내의 Solution Folder GUID도 잘못됐다. 사용자가 직접 연 뒤 제보했으며, 이를 원본 엔진의 마이그레이션이나 실제 C++ 빌드 실패로 기록하지 않는다. 기능별 Solution Folder·SolutionItems만 사용하는 수정 구성과 [읽기 안내](C:/Users/tnest/Desktop/UnrealEngine/AssetsReadingGuide.md)를 따른다.

사용자가 컴퓨터를 사용 중이므로 더 이상 UI를 조작하지 않는다. IDE의 실제 빨간 줄 상태·열려 있는 미저장 문서·새 구성의 화면 반영은 파일 구조 검증과 구분한다.

최종 색인은 상단 기능별 시작 파일 197개와 하단 원본 소스 92,001개를 연결한다. C++ 프로젝트 참조와 솔루션 빌드 구성은 0개다. 정상 Solution Folder 10,514개를 Microsoft.Build의 SolutionFile.Parse로 파싱해 성공했다. [구조 검사](C:/Users/tnest/Desktop/LostArk/out/AssetLoadingStudy20261014/UE-SolutionItems-validation.json)와 [실제 MSBuild 파서 검사](C:/Users/tnest/Desktop/LostArk/out/AssetLoadingStudy20261014/UE-SolutionFile-parser-validation.json)를 보관했다. CpuProfilerTrace·StaticMesh 등 시작 파일의 H/CPP는 같은 기능 폴더에 연결하며, Public/Private 물리 폴더를 이동하지 않았다. 이전 생성 파일은 백업으로 보존한다.

## 다음 코드 읽기

1. `WFormatTypes.h`의 FILE_HEADER, MODEL_META_HEADER, MODEL_SECTION_DESC와 실물 첫 176B를 대조한다.
2. BinaryReader → WModelDecoder → WMeshReader에서 bytes를 CPU 자료로 해석하는 순서를 읽는다.
3. Model → Mesh → Material에서 GPU buffer·texture 생성과 prototype 등록을 연결한다.
4. UE PackageFileSummary·StaticMesh·AsyncLoading2·RenderResource에서 같은 단계의 역할을 비교한다.

신규 가이드·추가 원고·RESULT·기술서 색인의 로컬 링크 96개를 검사해 누락 0개를 확인했다. 바이트 조사 JSON parse와 문서 공백 검사를 통과했다. 신규 파일의 `git diff --no-index --check`는 차이 존재에 따른 exit 1이며 공백 오류 출력은 없었다. 게임 빌드, Client 화면 실행, Unreal 엔진 실행 및 성능 계측은 수행하지 않았다. 사용자 독립 설명이나 직접 구현 완료를 이번 조사만으로 판정하지 않는다.

## 2026-10-15 — 길찾기 네 프로젝트와 바이너리 에셋 두 프로젝트 비교

사용자의 최신 범위는 Starcraft·MinecraftDungeons·WintersEngine·LostArk의 A*/Navigation/Pathfinding,
WintersEngine·LostArk의 binary asset이다. 기초 설명 뒤 실제 코드·상태 추적과 확인 질문을 진행한다.
과거 개인 구현 범위, 현재 이해, 실제 호출 여부를 구분하며 이전 보스·UE 질문을 선행 조건으로 요구하지 않는다.

- LostArk Server `ServerNavigation.cpp::Find_Path`는 높이를 가진 XZ 8방향 격자에서 1/√2 비용과 Euclidean heuristic을 사용한다. Engine `PathFinder.cpp`의 10/14 비용·octile heuristic·확장 예산·작은 고립 목표 사전 검사는 별도 구현이다.
- `NavGridBaker.cpp::Build`의 geometry decode cache, 삼각형 XZ 범위 제한, 셀 중심의 barycentric 높이 계산을 확인했다. source/paint→publisher→runtime grid→A*→GameRoom 이동 소비를 구분한다. 단일 grid의 한 XZ에는 하나의 높이를 저장하며 별도 region이 층을 구분한다.
- Starcraft `CNavMgr.cpp`는 실제 `CCommandMgr`→`CUnit` 경로에 연결된다. smoothing 호출은 주석이며 `UpdateSteering`은 조사한 C++ 호출부에서 연결되지 않았다. A* 실패 후 직선 waypoint fallback을 성공 증거로 오인하지 않는다.
- MinecraftDungeons의 일반 `CPathFinding.cpp`와 실제 몬스터가 호출하는 `CMonster::Find_Path`를 구분했다. 현재 `IsPassable`은 foot/head가 같은 좌표여서 통행 조건이 모순된다. 빈 경로 때 직접 이동하는 fallback도 확인했다. 이는 현재 소스의 정적 확인이며 과거 실행본의 동작 판정은 아니다. 수정하지 않았다.
- WintersEngine은 bit grid·반경 여유·연결성 캐시·직선 우회와 Server `WalkabilityAuthority`→Shared `MoveSystem` 소비를 확인했다. LostArk에 이 최적화가 모두 동일하게 있다고 설명하지 않는다.
- 앞서 조사한 정적 WModel 실물을 다시 읽어 12,720B, WINT 1.2, mesh의 파일 시작 offset 176과 material offset 3,184, 동일 SHA256을 확인했다. C++ decoder/GPU 실행 검증은 아니다.
- Winters는 `.wmesh/.wmat/.wskel/anims/*.wanim` 분리 파일을 읽고 LostArk는 WMOD section으로 묶는다. `WFormatTypes.h`는 디스크 배치 선언이며 `MODEL_ASSET_DATA`의 동적 CPU 소유 구조와 다르다. 현재 LostArk reader의 WINT 지원은 1.6까지이고 material은 WMAT/WMA2/WMA3다.
- 실제 writer 예시는 Winters `BinaryWriter.cpp`, LostArk `cook_wmodel_geometry_contract.py::rebuild_wmodel`이다. LostArk converter EXE 자체의 C++ 소스는 이번 검색에서 확인하지 못했으므로 Winters writer와 완전히 동일하다고 단정하지 않는다.
- LostArk `CModel`은 mesh/material을 공유하고 bone·animation 재생 상태를 복제한다. animation key 채널 데이터는 공유한다. binary animation의 실행 rate는 `Animation.cpp`의 30 tick/s이며 저장 ticksPerSecond를 그대로 사용하는 Winters와 구분한다.

이번 확인은 현재 소스·호출자 검색과 파일 바이트 읽기다. 제품 코드·리소스는 변경하지 않았고 빌드·게임 실행·화면 조작·새 성능 계측은 하지 않았다. 사용자 답변·독립 구현은 아직 없으며, 다음에는 open/closed/g/parent와 disk offset·공유 자원/개별 상태의 확인 답변에서 이어간다.
