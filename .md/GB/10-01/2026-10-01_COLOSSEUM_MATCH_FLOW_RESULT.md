# 콜로세움 4인 경기 전체 연결 RESULT

## 적용 위치와 범위

실제 실행 체크아웃 `C:/Users/USER/source/졸업팀폴/LostArk`, 브랜치
`codex/colosseum-pvp-entry-0930`에 반영했다. 이 문서는 09-30 대기열·로딩·도입·카운트다운
RESULT의 당시 미완료 사항을 이어서 닫은 현재 상태다. 다른 맵/팀원의 미커밋 자료는 정리하지 않았다.
Client/UI를 실행하거나 화면을 캡처하지 않았다. 아래 자동 검증은 최종 육안 승인과 다르다.

## G01. Server/Shared

- Debug/Release 모두 대기열 4명. 먼저 수락한 순서 1·3은 team 0/왼쪽, 2·4는 team 1/오른쪽.
- match ID별 독립 `CGameRoom`. 직접 콜로세움 진입 패킷과 로비 우회 버튼은 대기열을 우회하지 못한다.
- 준비 대상 수에는 아직 방에 도착하지 않은 예정 참가자도 포함한다. 실제 참가자 표현 준비를 모두
  확인한 뒤 30 Hz 서버 시계로 1초 후 도입 8.6초, 준비 10초, 전투 120초를 진행한다.
- 로딩 중 이탈자는 제외하되 팀/최초 자리 번호는 유지한다. 120초 준비 제한을 넘긴 세션은 종료한다.
- 근접·CONTACT 투사체·TIMED 장판·CombatObject의 적 플레이어 판정을 기존 전투 경로에 연결했다.
  아군은 피해·반응·target cap·투사체 접촉 소모에서 제외한다.
- 적 사망당 1점. 같은 사망의 중복 점수를 막고 3초 뒤 최초 팀 대기 위치에 HP/상태를 복구한다.
- 120초 종료 시 피해·점수·부활을 멈춘다. 점수가 같으면 무승부, 아니면 높은 팀 승리.
- 결과 귀환은 모든 참가자가 typed command로 요청하며 Server 승인 후 베른으로 전환한다.
  중복 귀환을 거부하고 인벤토리·소지금·내구도·외형을 보존한다.
- protocol 130: MATCH_FOUND match ID, LOAD_READY/RETURN, MATCH_STATE와 예상 인원·팀·순서·점수·tick.
  enter/spawn에 최대 16 KiB customizing preset을 추가했다. version/class/중복 키/수치/깊이/경로를
  검증하고 실패 시 부분 메시지를 commit하지 않는다. 구 버전 Server/Client 혼용은 불가하다.

## G02. Client

`CColosseumLoadingView`를 에셋 로더와 맵 입장 뒤 준비 장벽에서 함께 사용한다. 모든 클라이언트의
팀 좌우가 같고 자기 카드만 강조한다. 실제 참가자 모델로 대표 초상을 렌더한다. 초기 에셋 로딩
동안에는 기존 클래스 이미지가 사용되며 맵의 실제 참가자 모델이 준비되면 교체된다.

기존 로스터 조기 삭제, 자기 팀 좌우 뒤집기, 로컬 캐릭터만 준비된 시점의 배우 목록 고정,
x 좌표로 팀을 추측하던 연결을 제거했다. PlayerId/NetEntityId/팀/도착 순서로 매 프레임 도열을 조인한다.
실제 바디·snapshot·커스터마이징·avatar가 준비되어야 READY를 보낸다. 외형 적용 실패를 default
외형 성공으로 속이지 않으며 진단을 보존한다. 180초 Client 대기는 실패 사유와 함께 로비로 복귀한다.

준비·컷신·결과 동안 gameplay 입력을 막는다. 경기 도입/카운트다운은 서버 시각을 사용하며,
F1의 로컬 재생/seek가 진행 중인 서버 경기를 바꾸지 못한다. 컷신에 실제 참가자 네 명을 도열한다.

## G03. 원본 HUD·승리 연출

`CColosseumMatchView`가 읽기 전용 경기 상태를 받아 기존 `CUILayoutRuntime`, `CUIInputRouter`,
카메라 presentation override 및 `CCharacter -> CModel` 경로를 사용한다. UI가 socket이나
`Change_Level`을 직접 호출하지 않는다. 결과 귀환 버튼은 재시도 가능하며 클릭은 1초 간격으로 제한한다.

- 화면 12시 중앙: 원본 ornate score frame + TIME + 좌우 팀 점수. 울트라와이드에서도 HUD는 상단 중앙에 고정.
- 원본 `EFUI_COLOSSEUM.colosseumplaying_loc_int`에서 승리 139프레임, 패배/무승부 각 140프레임,
  40 fps native timeline을 추출했다. `titleString_lb`의 위치·등장/퇴장 alpha도 native frame을 소비한다.
- 실제 원본 연결 `ceremony -> seqact_setcameratarget_5 -> efseqact_matinee_34 -> interpdata_33`,
  `LV_PVP_COLOSSEUM_SCENE01A`의 카메라/배우/애니메이션 트랙을 사용한다.
- cubic Hermite 카메라 곡선을 30 Hz 151개 키로 샘플했다. 승리 카메라는 5초.
- 7개 지원 클래스의 원본 `sc_cheer_1`을 기존 supplemental animation-set 경로로 설치했다.
  기본 바디 파일을 덮지 않았고 source fps와 바디 skeleton을 유지했다.
- 승리 팀의 실제 두 캐릭터를 원본 승리 배우 위치로 표현하고 패배 팀은 이 연출 동안만 숨긴다.
  사망 상태와 별도로 컷신 pose를 샘플한다. 연출 종료/객체 파괴 시 pose·animation·visibility를 복원한다.
- 무승부에는 특정 팀의 승리 컷신을 재생하지 않는다. 결과 확인 뒤 모두 베른 귀환 버튼을 사용한다.

### 원본과 프로젝트 조정을 구분한 부분

원본 3인 승리 도열을 요청한 2v2에 맞춰 원본 좌우 두 자리로 구성했다. 가운데 자리는 생략한다.
raw source FOV 50은 메타데이터에 보존하고, 기존 도입과 동일한 16:9→2.35 projection 정책에 따라
runtime FOV 63.299397을 적용했다. 이는 별도의 원본 속성을 발견했다는 뜻이 아니다.
마지막 250 ms 복귀 fade와 베른 귀환 버튼은 프로젝트 연결이다.

글자 family `LabelEx_YG760_shadow`는 원본과 연결했으나 dynamic title의 최종 AS 색 값은 확인하지
못해 참고 이미지 기반 색을 사용한다. 승리 Matinee에는 이름/직업 label event가 발견되지 않았으므로
원본 자막 트랙을 복구했다고 보고하지 않는다. 새 결과 화면은 점수·승패·귀환이며 전체 KDA 통계창을
새로 구현한 것은 아니다. 원본과 최종 화면이 픽셀 단위로 같다는 승인도 하지 않았다.

## 저장·리소스

Data/Camera 및 Data/UI는 `CProjectDataRoot`를 통해 직접 소비하는 정본이며 프로젝트 None 항목에
등록했다. 이번에는 맵 배치나 World/Navigation authoring을 수정하지 않았으므로 그 게시본을
임의 재생성하지 않았다. UI builder와 원본 애니메이션 cooker가 Resources에 필요한 파일을 설치했다.

- `Data/Camera/ColosseumVictory.cutscene.json`
- `Data/UI/Colosseum/MatchHUD_Layout.json`, `Result_Layout.json`, `Result_{Victory,Defeat,Draw}.keyframes.json`, `Result_TitleTracks.json`
- Resources 상대 `UI/Colosseum/Result/` (PNG 1,799개), 기존 `UI/Common/White1x1.png` 공유.
- `Character/<class>/AnimSets/<class>_ColosseumVictoryAnimSet.wmodel`
  (LanceMaster, GunSlinger, Slayer, Artist, DimensionMaster, Warlord, GuardianKnight).
- 물리 리소스 루트: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources`.

Resources는 Git 소스 커밋 대상이 아니다. 초기 구현 당시 별도 Drive 업로드·ZIP 재생성·push/PR은
수행하지 않았다. 이후 사용자 요청에 따른 배포/PR 작업은 아래 G07/G08로 구분한다.

## 자동 검증

- 정본 Product 증분 Build: Engine → Shared → Server → Client, Debug/Release 모두 PASS.
  최종 Debug receipt `out/BuildPipeline/runs/20260930T183545812Z-debug-product.json`;
  Release receipt `out/BuildPipeline/runs/20260930T183439849Z-release-product.json` (UTC 파일명).
  Debug 마지막 재실행은 OBJ/PCH/바이너리 재컴파일 0개. Clean/Rebuild/tlog 삭제 없음.
- `Server.exe --colosseum-contract-test`: Debug/Release 각각 21 PASS, failures 0.
- `NetworkProtocolHarness.exe`: Debug 전체 1,438 PASS, failures 0.
  focused `--colosseum-queue-only`도 30 PASS. 기존 wire golden의 version/빈 외형 문자열 크기를 130으로 갱신했다.
- `python -X utf8 Tools/LpkPipeline/build_colosseum_match_result_ui.py --validate`: 두 layout,
  세 native timeline, 이미지 참조 1,800개, 카메라 151 key, 정확한 cheer 7개 연결 검사 PASS.
- 변경 JSON parse, Client project/filter XML parse 및 중복 등록 검사 PASS.
- `git diff --check`와 신규 소스/RESULT의 no-index 공백 검사 PASS.
  PLAN에는 적용된 소스·데이터 65개 전문과 프로젝트/카탈로그 변경 블록을 보존했다.
- 기존 WModelGeometryContractHarness를 현재 Debug Engine으로 증분 빌드한 뒤
  `--legacy-corpus out/colosseum-victory-20261001 --expected-files 7` 실행:
  `files=7 skinned=7 hasBounds=7 expectedFiles=7 valid=1`, exit 0.
  잘못된 기대 개수/0/4097/음수/숫자 꼬리/overflow는 거부했다. 옵션을 생략하면 기존 2586개
  corpus 및 유형별 기대 개수 계약을 그대로 사용한다. 최초 구형 하네스의 ABI 불일치성 crash는
  현재 헤더/엔진에 맞춘 증분 빌드 후 해소했다. 이는 Client GPU 화면 검증을 대신하지 않는다.
- 빌드의 기존 C4819/C4244 경고를 해결했다고 주장하지 않는다. 관련 없는 인코딩/수치 코드는 변경하지 않았다.

## 사용자 확인 경로와 남은 승인

1. 기존 Server/Client를 종료하고 같은 protocol 130으로 다시 실행한다. Client 작업 디렉터리는 `Client/Default`.
2. 같은 Server에 연결한 Client 4개에서 각각 베른의 콜로세움 NPC → 입장 수락. 수락 순서가 팀 순서다.
   정식 경기 시작은 혼자 우회할 수 없다. Debug 단독 화면 검증은 아래 G05의 별도 진입 버튼을 사용한다.
3. 서로 다른 로딩 속도에서도 네 화면의 좌우 팀이 동일한지, 마지막 준비 뒤 실제 네 캐릭터가 보이는지 확인한다.
4. 10초 → 창살 하강 → 상단 중앙 120초·점수, 아군 무피해·적 킬점수·3초 부활을 확인한다.
5. 종료 시 승패 배너 → 실제 승리 팀 환호 컷신 → 베른 귀환을 확인한다. 동점·사망 상태·로딩 이탈도 확인한다.

자동 테스트는 창을 띄우지 않는 계약 검증이다. 실제 4개 Client의 동시 화면, 투명도/글꼴/구도,
커스텀 의상별 환호 자세와 지연 환경은 사용자 수동 확인이 남아 있다. Server daemon/Client는 자동 실행하지 않았다.
현재 개인 Client endpoint는 127.0.0.1이며 변경하지 않았다. 팀 LAN 문서는 09-30 만료 상태이므로
여러 PC로 확인할 경우 endpoint 정본을 갱신해야 하며 만료 우회 스크립트는 실행하지 않았다.

## G05. Debug F1 승리 컷신·HUD·승리 UI 미리보기

사용자 요청으로 기존 도입 컷신 Play 옆에 `Play Victory Cutscene`을 추가했다.
아래 `Colosseum HUD / Result UI Preview`에서 `Show Score HUD`, `Play Victory UI`,
`Pause Preview`/`Resume Preview`, `Stop Preview`를 사용할 수 있다.
승리 컷신은 기존 원본 카메라와 cheer clip으로 실제 로컬 캐릭터 한 명을 보여 준다.
임의 캐릭터/가짜 팀원은 만들지 않으므로 두 번째 승리 슬롯은 단독 확인에서 비어 있다.
HUD는 상단 중앙 3:1, 120초 예시 타이머를 표시하며 0초에서 멈춘다.
승리 배너는 기존 제품 이미지/40fps 타임라인을 재생하며 끝나면 숨긴다.
컷신은 종료 fade 뒤 카메라와 실제 캐릭터의 pose/animation/suppression을 복원한다.

`CColosseumMatchView::Sample_Presentation`을 정식 경기와 미리보기가 함께 소비한다.
View 내부 예시 상태는 match ID가 0이며 replication, Server state 또는 데이터 파일에 저장하지 않는다.
Preview는 베른 귀환 버튼과 command를 만들지 않는다. 실제 매치 수신 시 미리보기를 중지한다.
문서/UI 미준비, 알 수 없는 preview kind, 실제 캐릭터/카메라 미준비는 오류를 표시하고 거부한다.
Preview끼리 반복 교체해도 카메라/배우 override와 UI가 중첩되지 않게 정리한다.

단독 진입이 직전 작업에서 막혀 있었으므로 Debug Lobby `Colosseum Preview`와 Server의
Debug 직접 승인만 열었다. 기존 공유 unmatched 월드 경로를 사용하며 새 게임 런타임을 만들지 않는다.
대기열 roster 유무로 단독 진입을 판정하므로 매치 snapshot이 늦어도 4인 로딩 장벽은 우회하지 않는다.
Release 직접 입장 거부, Debug/Release 베른 대기열의 4인 경기와 매치별 방 분리는 유지한다.
새 C++ 파일/프로젝트 등록, 리소스 재추출, protocol 변경은 없다.

### G05 검증

- Debug Product 증분 빌드 PASS: `out/BuildPipeline/runs/20260930T190421982Z-debug-product.json`.
  최종 최신성 재확인 `20260930T191106301Z-debug-product.json`도 PASS, OBJ/PCH/바이너리 재컴파일 0개.
- Release Product 증분 빌드 PASS: `out/BuildPipeline/runs/20260930T190811210Z-release-product.json`.
  첫 시도는 동시에 실행하던 창 없는 Debug Server 계약 하네스의 출력 잠금으로 preflight가
  거부했다. 하네스 정상 종료 후 같은 명령을 재실행해 통과했으며 강제 종료/잠금 우회는 하지 않았다.
- Debug/Release Server `--colosseum-contract-test`: 각각 21 PASS, failures 0.
- NetworkProtocolHarness `--colosseum-queue-only`: 30 PASS.
- UI builder `--validate`: 두 layout, 세 40fps timeline, 이미지 참조 1800개, 카메라 key 151개, cheer 7개 PASS.
- `Tools/LpkPipeline/test_colosseum_debug_preview_contract.py`: 소스 연결/격리 검사 8개 PASS.
  버튼 중복, typed 진입, Release guard, 실제 ID 사용, 잘못된 입력의 commit 전 거부,
  서버 매치 우선, 종료 정리, pause/유한 시간 경계를 검사한다. GPU 실행/육안 테스트는 아니다.
- 변경 JSON/XML parse 및 `git diff --check` PASS. 새 미리보기 테스트도 PLAN의 전체 코드 부록에 포함했다.

### G05 사용자가 직접 누를 경로

1. 같은 프로젝트의 Debug / x64 `Server + Client`를 재시작한다. Client 작업 디렉터리는 `Client/Default`.
2. Lobby → `Colosseum Preview`로 혼자 입장한다. 실제 4인 경기에는 미리보기 버튼이 잠겨 있다.
3. F1 → `Colosseum Intro Cutscene` → `Play Victory Cutscene`.
4. 아래 `Colosseum HUD / Result UI Preview`에서 HUD/승리 UI를 각각 재생한다.
5. F1을 닫으면 패널 없이 재생을 확인할 수 있다. 필요하면 다시 F1 → Pause/Stop Preview.

Client 실행·UI 조작·캡처는 하지 않았다. 실제 화면과 원본 영상의 최종 일치 여부는 사용자 확인 전이다.

## G06. 원본 패배 배너와 패배 팀 표시

사용자가 보여 준 승리 배너에 대응하는 원본 패배 UI를 확인했다. 기존에 추출·설치된
패배 파일과 정식 승패 분기는 있었지만 F1에서 패배를 따로 확인할 수 없었다.
원본 GFX를 읽기 전용으로 다시 파싱해 parent sprite 827의 `defeat_mc`가 심볼 755,
`victory_mc`가 766을 정확히 참조하는 것을 확인했다. 패배 140프레임/승리 139프레임,
둘 다 showAnim=2/hideAnim=127이며 설치 타임라인은 40fps다.
원본 바이트 위치는 `C:/Users/USER/.claude/jobs/46aea322/tmp/gate/gfx/colosseumplaying_loc_int.gfx`다.

- F1 `Colosseum HUD / Result UI Preview` → `Play Defeat UI`를 승리 버튼 옆에 추가했다.
  실제 로컬 캐릭터 identity를 유지하고 View 안에서만 team 0 / 상대 승리 team 1 / 점수 1:3을 샘플한다.
- 정식 경기에서는 로컬 PlayerId/NetEntityId와 Server 참가자 팀을 조인해 패배를 표시한다.
  미확인 로컬 팀을 무조건 패배로 처리하던 경로는 제거하고 원인 표시 후 정확한 조인을 기다린다.
- 원본 title track의 마지막 키+한 프레임으로 결과별 길이를 계산한다. 승리 3475ms,
  패배/무승부 3500ms다. 이전 공통 3475ms 절단 때문에 패배 마지막 프레임이 빠지던 것을 보완했다.
  양팀이 이어 보는 승리 팀 컷신은 동일한 Server 결과 tick +3500ms에서 시작한다.
- 승리 그림의 색만 바꾸거나 대체 UI를 생성하지 않았다. 기존 `Result_Defeat.keyframes.json`,
  원본 이미지 `UI/Colosseum/Result/...png`, native title 위치/alpha를 그대로 사용한다.
  동적 한글 글꼴·색의 기존 프로젝트 처리와 원본 픽셀 단위 일치는 별개의 수동 확인 범위다.
- Server/Shared/protocol, 맵/렌더링 옵션, 리소스 파일은 변경하지 않았다. 프로젝트 등록 추가도 없다.

### G06 검증 및 실행

- Debug Product 증분 빌드 PASS: `out/BuildPipeline/runs/20260930T193112813Z-debug-product.json`.
  Client OBJ 2개와 Client 실행 파일 1개를 갱신했으며 Engine/Shared/Server 출력·CSO 변경은 0개다.
- Release Product 증분 빌드 PASS: `out/BuildPipeline/runs/20260930T193237275Z-release-product.json`.
  Client OBJ 3개/실행 파일 1개 갱신, CSO 변경 0개. JSON/XML parse와 tracked/new-file 공백 검사도 PASS.
- 기존 소스 연결 검사 확장: 11개 PASS. 패배 예시 상태, 미확인 팀 오표시 방지,
  원본 결과별 끝 프레임/공통 컷신 시계를 추가 확인했다. 이는 GPU 화면 검사가 아니다.
- 원본 UI validator: 두 layout/세 timeline/1800 image reference/151 camera key/7 cheer 연결 PASS.
- 사용자가 기존 Client/Server 종료를 알린 뒤 빌드했다. Client나 UI를 직접 실행·조작·캡처하지 않았다.

새 Debug Client로 `Lobby → Colosseum Preview → F1 → Colosseum HUD / Result UI Preview → Play Defeat UI`.
`Pause Preview`/`Stop Preview`도 동일하게 사용한다. 실제 경기의 패배 화면 최종 판정은 사용자 확인 전이다.

## G07. 팀원 전달용 콜로세움 리소스 복사

사용자 지정 `C:/Users/USER/OneDrive/바탕 화면/CY_Resources`에 이번 콜로세움 추가분을
`Client/Bin/Resources` 상대 경로 그대로 복사했다. 시작 시 대상은 비어 있었다.

- 총 2,618개, 234,990,498 bytes(약 224 MiB).
- `Map/LV_PVP_COLOSSEUM`: 모델 103개, DDS 627개, PNG 42개로 772개.
- `UI`: 현재 콜로세움 layout/timeline이 참조하는 이미지, 공용 이미지와 로딩 배경 1,839개.
- `Character/<직업>/AnimSets`: 카탈로그가 참조하는 승리 animation set 7개.
- 기존 공용 캐릭터 본체·관중 NPC·폰트, Data/코드/실행 파일·설치 receipt·추출 중간물은 제외했다.
  기존 공용 Resources 설치와 동일한 Git 코드/Data 버전이 전제인 추가 배포본이다.
- 폴더 밖 `C:/Users/USER/OneDrive/바탕 화면/CY_Resources_콜로세움_배포안내.txt`에
  설치 경로·포함 항목·공용 리소스 전제를 기록했다. Drive 업로드는 사용자가 수행한다.

`Copy_ResourceDistribution_2026-10-01_Colosseum.ps1 -InspectOnly`로 전체 파일 존재/길이와
기존 대상 충돌을 먼저 검사했다. 실제 복사 실행은 exit 0 / Verified=true였고,
선택한 2,618개 전체의 SHA-256 원본/대상 일치를 확인했다. 이 검사는 이번 복사의 내부 검증이며
팀원에게 별도 manifest/immutable pack 설치 체계를 요구하지 않는다.
기존 파일 삭제·원본 리소스 변경·코드/Data 게시·Client/UI 실행은 하지 않았다.
이번 변경은 배포용 복사와 문서/복사 스크립트에 한정되므로 제품 재빌드는 수행하지 않았다.

### G07 후속: 기존 배포본 기준 중복 제외

사용자가 이전 배포본을 `CY_Resources`(09-30 23시), 이번 전체 묶음을 `CY_Resources１`로 지정했다.
같은 상대 경로와 동일 내용의 807개를 제외하고 `CY_Resources_추가분_20261001`을 만들었다.
신규 1,811개/20,139,956 bytes이며 같은 경로의 내용 변경은 0개다. 승리 animation set 7개,
`UI/Colosseum/Result` 1,799개, `UI/Common` 5개다. 두 입력 폴더를 수정하지 않았고 복사 전체를 검증했다.
팀원은 기존 배포본 위에 추가분의 Character/UI를 `Client/Bin/Resources` 상대 경로 그대로 합친다.
코드·Data는 이 PR으로 받고 이미지·모델은 사용자 Drive로 별도 전달한다.

## G08. main 기준 PR 준비와 보존 검증

원격을 fetch한 결과 기존 checkout HEAD와 `origin/main`이 모두
`b0da41e1c0f82181a080b0a838ddf51e973f2af3`이었다. 기존 PR #492는 이미 병합됐으므로
새 브랜치 `codex/colosseum-match-results-20261001`을 현재 기준에서 만들었다.
현재 main과 갈라진 커밋이 없어 충돌 해결을 위한 삭제/ours/theirs 선택은 필요하지 않았다.

- 현재 요청의 tracked 58개 파일 및 콜로세움 신규 소스/설정/도구/문서만 선정했다.
- 다른 세션의 `Client/Private/Level_Lobby.cpp` 로컬 서버 시작 대기 30→120초 두 줄은
  디스크에서 보존하고 index에서만 분리한다. 해당 LOCAL_DEBUG_SERVER_PATH RESULT와
  다른 맵/개인 설정/추출 중간물/이전 배포 스크립트도 이번 PR에 넣지 않는다.
- Resources 실물·EXE/DLL/CSO·out/EngineSDK는 제외한다. 이번 Camera/UI는 Data 직접 소비이며
  World/Navigation 게시본은 변경하지 않았다. 기존 main의 관련 기능과 runtime 데이터는 유지한다.
- Debug Product 재확인 PASS: `20260930T195537353Z-debug-product.json`.
  Release PASS: `20260930T195540664Z-release-product.json`. 모두 OBJ/PCH/CSO/binary 변경 0개.
  이 빌드는 별도 세션의 120초 대기 수정이 함께 존재하는 디스크 기준이며 그 두 줄은 PR 대상이 아니다.
- Debug/Release `--colosseum-contract-test`: 각각 21 PASS, failures 0.
- Debug protocol `--colosseum-queue-only`: 30 PASS.
- 미리보기 연결 검사: 11 PASS. UI validator: 2 layout/3 timeline/1800 image/151 camera key/7 cheer PASS.
- 실제 다인 화면·최종 시각 일치는 여전히 사용자 확인 범위다. Client/UI를 실행하지 않았다.
- 커밋 index 기준 75개 파일 검사 PASS: JSON 9개/XML 2개 parse, 중복 project 등록 없음,
  파일 삭제·Resources/바이너리 포함·충돌 마커 없음. main의 기존 project/filter 등록과
  캐릭터 카탈로그 필드(승리 animation set 7개 추가 외), 기존 packet ID 값은 모두 보존했다.
  다른 세션의 두 줄을 index에서 분리하기 전후 `Level_Lobby.cpp` 디스크 내용도 동일했다.
  `git diff --cached --check`와 미스테이징 diff 검사를 통과했다.

## G09. 기본 HUD 복구·좌우 팀 체력·처치 알림

### 실제 반영

- 원인: `MainApp`의 HUD sprite, HP/resource 수치, quick-slot 키 라벨, cooldown text
  네 레벨 허용 목록에 COLOSSEUM이 없었다. 네 호출자를 모두 연결했으며 기존 컷신 숨김은 유지한다.
- `CColosseumMatchView`의 기존 제품 이미지 런타임에 CombatHUD를 추가했다. 서버 arrival index로
  좌측 team 0 / 우측 team 1 두 행씩 고정한다. 이름·HP·개인 킬 수, 자기 이름 강조,
  사망 표시를 제공한다. HP 미수신은 빈 바/`...`이며 만피로 꾸미지 않는다.
- HP는 기존 `CReplicatedPlayerHealth`와 실제 player/entity identity를 조인한다.
  개인 킬과 최근 8개 처치는 Server 득점 처리에서 한 번만 생성한다. protocol 131로 함께
  전달하며 UI는 최근 6초 이내 3개만 우측 상단에 표시한다. 서버 이벤트를 다시 받아도
  수명이나 기록 수를 늘리지 않는다. 새 경기/종료/Preview 정리 시 이전 표시는 남기지 않는다.
- `EFUI_COLOSSEUM.colosseumplaying_loc_int.gfx`의 원본 sprite 327→294 HP frame/track,
  sprite 35의 배경·팀 색·화살표 이미지를 추출했다. 2인 행과 화면 가장자리 배치는 프로젝트
  2대2 규칙에 맞춘 authoring이며 원본 3대3 전체 화면/버프 아이콘/모든 Flash 효과 복원 주장은 아니다.
- Data/UI 직접 소비 계약이므로 `CombatHUD_Layout.json`을 정본에 저장했다. World/Navigation
  게시 데이터는 변경하지 않았다. 새 C++ 파일 없이 기존 View를 확장하고 JSON만 project/filter에 등록했다.

### 리소스 동시 배포

새 PNG 9개, 총 49,460 bytes를 다음 양쪽에 같은 `UI/Colosseum/Combat/shape_*.png` 경로로 넣었다.

- 게임: `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/UI/Colosseum/Combat`
- 요청한 추가 배포: `C:/Users/USER/OneDrive/바탕 화면/CY_Resource/UI/Colosseum/Combat`

기존 `CY_Resources`, `CY_Resources１`, `CY_Resources_추가분_20261001`은 건드리지 않았다.
9개 전체 복사 내용 일치를 확인했다. Drive 업로드는 하지 않았으며 코드/Data는 PR #495로 전달한다.

### 자동 검증과 수동 확인 경계

- Debug Product PASS: `out/BuildPipeline/runs/20260930T202318778Z-debug-product.json`.
- Release Product PASS: `out/BuildPipeline/runs/20260930T202702085Z-release-product.json`.
  Debug/Release 모두 Engine/Shared/Server/Client 빌드 성공, CSO 변경 0개.
- Debug/Release Server `--colosseum-contract-test`: 각 24 PASS, failures 0.
- Debug/Release NetworkProtocolHarness 전체 실행: failures 0. Debug focused Colosseum 40 PASS.
- 기존 Preview source contract 11 PASS, 새 CombatHUD source/data contract 7 PASS.
- 기존 UI validator PASS, 새 CombatHUD 49 slots/9 source image 검증 PASS.
- JSON duplicate key/XML parse 및 중복 프로젝트 등록 검사 PASS. PLAN 전문에는 별도 세션
  Lobby 120초 변경을 넣지 않았으며 작업 디스크의 해당 두 줄은 보존했다.
- 빌드 중 C4819/외부 PDB 경고를 관찰했다. Release 하네스 링크에는 Shared PDB LNK4020도
  있었으나 실행 검사는 통과했다. 경고를 없애려는 무관한 인코딩 변경이나 산출물 전체 삭제는 하지 않았다.

사용자는 새 Server/Client를 함께 재시작한 뒤 콜로세움에서 기본 HUD를 확인한다.
Debug `F1 → Colosseum HUD / Result UI Preview → Show Score HUD`는 실제 로컬 행만 표시한다.
좌우 4인 HP 갱신, 개인 킬/우측 처치 알림과 6초 뒤 제거는 실제 4인 경기에서 확인해야 한다.
Client/UI 실행·조작·캡처와 최종 visual PASS는 수행하지 않았다.


## G10. PR494~496 통합 후 대기 UI 독립 검토

- Server 30Hz deadline과 수신 후 경과 시간으로 10→1초를 표시하고, 기한 뒤에는 진입 준비 문구를 표시한다. 대기 인원은 Server의 0~4명을 읽는다. Client가 경기 시작이나 팀을 확정하지 않는다.
- 생산 `Set_ColosseumQueueState`, `Close_ColosseumWait`, `RenderText_ColosseumQueue`, `Render_AssemblyCountdown`를 추출한 headless C++ 검사 26개 PASS. 늦은 WAITING으로 취소 창을 다시 열지 않으며, 기존 발탄·쿠크 기본 문구와 흰색/노란색 표시가 유지된다. JSON parse·stable ID 중복·새 라벨의 패널 내부 배치 검사 PASS. 증거: `out/QueueUiIndependentReview/result.json`.
- 기존 경계: 취소 직후 다시 가입하면 request sequence가 없는 이전 LEFT/REJECTED가 새 WAIT 창을 닫을 수 있다. 기존 non-WAITING 처리에도 같은 경계가 있으며, 이번 변경에서 별도 재가입 직렬화나 wire 계약 확대는 하지 않았다. 실제 Client 화면 확인은 사용자 검증 범위다.


## G10. PR #494~#496 통합 리소스 전달 보완

2026-10-01 통합본의 Colosseum UI JSON과 CharacterCatalog가 참조하는 추가 리소스를
현재 설치된 Client Resources에서 `C:/Users/user/Desktop/GBResources2`로 전달했다.
UI1,848개와 승리 AnimSet7개, 총1,855개/23,739,498bytes를 신규로 추가했고 기존 파일
교체는0개다. 원본 Resources는 변경하지 않았다. 기존 PR #496의1,116개도 이전 manifest와
원본/대상 SHA-256을 다시 대조했다. 합계2,971개/436,602,224bytes의 누락·hash 불일치는0개다.

증거는 통합 worktree의 `out/IntegrationValidation/colosseum-ui-animation-resource-delivery.json`과
`integrated-resource-delivery-summary.json`이다. GBResources2는 기존 전체 Resources에 합치는
추가분이다. Fonts/Deploy까지 갖춘 전체7개 root의 대체 폴더로 안내하지 않는다.
ZIP은 Resources를 포함하지 않으며 기존 전체 Resources를 외부로 사용한다.

배포 복사 스크립트의 Source 기본값은 Windows PowerShell5.1에서도 PSScriptRoot가 준비된
본문에서 해석하도록 고쳤다. 물리 모델 검사는 기존103개를 Imported build.receipt의 정확한
runtimeMaterialAdmission ID 목록으로 고정하고 PR #496의 아래3개만 더해106개를 요구한다.

- MAP_AA31466B4FAB_BG_LUT_LUCASTLE_CASTLEDECO01_SM_PSY
- MAP_CBADB2627CFB_BG_LUT_LUCASTLE_HEROSTATUE05_SM_ARTREE
- MAP_CBADB2627CFB_BG_LUT_LUCASTLE_HEROSTATUE05_SM_ARTREE_OVR_72E9DCA8A6BB

미지정/중복 모델, 경로 이탈, 빈 원본과 상이한 기존 대상 거절을 유지했다. UI/AnimSet 정규식은
넓히지 않았다. 쓰기 없는 검사 모드는 기존 `-InspectOnly`이고 `-WhatIf`는 지원하지 않는다.
스크립트 수정 후에는 재복사를 수행하지 않았다. 긴 worktree 경로에서는 Windows PowerShell5.1의
경로 길이 제한을 피하도록 현재 통합용 짧은 `L:/LostArk` 경로로 검사했다.


수정 스크립트의 기본 Source를 생략한 `-InspectOnly` 검사는 exit0이다. 전체선택2,826개는
Character7/Map971/UI1,848개다. GBResources2에 없는 것으로 나오는41개는 기존 Map의 PNG이며,
이번에 추가한 UI/승리 animation 누락은0개다. 해당41개에 대한 현재 Colosseum Map JSON의
명시 경로 참조는0개였지만 binary material 소비까지 검사한 것으로 확대하지 않는다.
기존 전체 Resources와 추가분의 경계를 유지하여41개를 추가 복사하지 않았다.
`out/IntegrationValidation/colosseum-resource-default-source-inspect.log`와
`colosseum-map-inspect-existing-pngs.json`에 결과를 남겼다.
