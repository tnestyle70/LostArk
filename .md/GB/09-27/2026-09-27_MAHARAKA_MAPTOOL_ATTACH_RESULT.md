# G01 — 마하라카 MapTool 연결 복구 결과

## 원인

사용자 스크린샷의 Level9는 Maharaka다. 기존 MapTool은 런타임 연결에서 쿠크·발탄·베른·
캐릭터 선택만 반환했고, editor registry도 마하라카를 포함하지 않았다. Is_MapAuthoringLevel이
false라 Handle_LevelTransition의 targetLevelIndex가 END에 머물며 Catalog와 Camera가 준비되지
않았다. 그래서 Waiting for isolated Development shell / NO MAP AREA / Catalog NOT READY /
ASSET_TEST camera unavailable이 나타났다. 워터팡 JSON의 유무나 카메라 키 오류가 이 화면의 원인이 아니다.

## 적용

- MapTool_Area.cpp의 registry에 LV_OCN_EVENTIS_MHP와 gameplay 정본 연결을 추가했다.
- Runtime_AuthoringTargets와 Authoring_Batches에서 실제 CLevel_Development(MAHARAKA)의
  catalog·placements·batches·device/context를 빌린다. 기존 source/runtime ID 일치 검사는 유지했다.
- Level_Development.h에 Debug 전용 접근자와 빈 Deploy runtime 소유자를 추가했다.
  MapCatalog에 Deploy pair가 없는 Area의 기존 베른 연결과 같은 계약이다.
- 편집 중 self-motion을 멈추며, 닫기 때 저작 baseline rebase 후 재개한다. 서버 복제·조명은
  계속 갱신한다. 기존 컷신 Stop의 actor/camera 반환과 실패 rollback을 그대로 사용한다.
- Maharaka의 uniform navgrid를 source/paint로 잘못 읽지 않는다. 이번 변경으로 Navigation
  bake를 새로 지원하는 것은 아니다. NPC/배치/카메라/워터팡 데이터는 이번에 수정하지 않았다.
- Loader.cpp의 Debug TriggerBox prototype 등록에도 Maharaka를 포함했다. 현재 NPC/spawn만
  있을 때는 누락이 드러나지 않지만, 트리거/충돌 박스 생성·재로드 때 clone 실패를 방지한다.
- 기존 네 C++ 파일만 변경했다. 프로젝트 등록·Engine API·프로토콜·Server 코드 변경 없음.

## 자동 확인

- 정본 명령: Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
  -BuildLogDirectory out/BuildPipeline/maharaka-maptool-attach-20260927.
- Engine/Shared/Server/Client 모두 PASS. Client OBJ4개, binary1개 갱신, CSO0개.
- 결과: out/BuildPipeline/runs/20260927T120632716Z-debug-product.json.
  result=PASS, missingRuntimeInputs=[], invalidRuntimeInputs=[]. 전 domain 실행 검증 의미는 아니다.
- Client/Public/Level_Development.h, Client/Private/Level_Development.cpp,
  Client/Private/MapTool_Area.cpp UTF-8 BOM 없음 유지. 기존 포함 헤더의 C4819/C4828 경고는 남았다.
- test_maharaka_waterpang_sequences.py: 기존 8검사 PASS. GUI 실행 성공을 대체하지 않는다.
- 변경 코드 git diff --check PASS. 독립 리뷰와 최종 문서 검사는 아래 후속 기록을 따른다.
- 이번은 C++ 연결 수정이므로 데이터 재게시 불필요. 빌드는 runtime 데이터를 게시하지 않았다.

독립 리뷰에서 camera/attach 경로는 blocker가 없었으며 TriggerBox prototype 누락을 추가로
찾았다. 실제 Stage_WorldTriggerBoxes/Stage_SpawnAnchorBoxes의 clone 호출과 Loader 목록을
대조하고 한 줄을 추가했다. 마지막 Debug Product 재빌드도 PASS이며 Loader OBJ1개와
Client binary1개만 다시 갱신했다. 최종 결과는
out/BuildPipeline/runs/20260927T121438459Z-debug-product.json이다.
마지막 diff --check와 XML/MapCatalog JSON parse도 통과했다. DirectXTK PDB 미포함 LNK4099는
기존 라이브러리의 디버그 정보 경고이며 링크 성공과 구분해서 기록한다.

## 사용자 확인

실행 파일: C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Debug/Client.exe.
작업 디렉터리: C:/Users/USER/source/졸업팀폴/LostArk/Client/Default.
Visual Studio Debug/x64 Client를 새로 실행한다. F5/Ctrl+F5는 설정에 따라 Build할 수 있다.

1. Lobby → Maharaka → F1 → Open Map Tool.
2. Area가 Maharaka Paradise / LV_OCN_EVENTIS_MHP로 연결되는지 확인한다.
3. Camera에서 **통합 컷신 편집은 끈다**. 이 옵션은 발탄·쿠크 Composition 전용이다.
4. 일반 컷신 목록에서 워터팡 / 원본 도입 카메라15 또는20 → Play.
5. Stop으로 기존 카메라·발판 자세 복귀를 확인한 뒤 발판 흔들림/붕괴 항목을 확인한다.

에이전트는 Client 실행·UI 조작·화면 캡처를 하지 않았다. 사용자 화면 확인은 대기 상태다.
확인 시 로컬 Client/Server 프로세스는 없었다. 현재 정본 LAN endpoint는 192.168.0.22:7777이며
세션 시작 설정 동기화의 probe는 not-listening이었다. 서버 미기동/접속 실패는 MapTool 바인딩과
별개의 입장 전제다. endpoint 정본 자체는 변경하지 않았다.

워터팡의 아직 미구현인 물 분사·배우·BGM·경기 전체를 이번 수정으로 완료했다고 주장하지 않는다.
commit/push는 요청받지 않아 실행하지 않았다.
