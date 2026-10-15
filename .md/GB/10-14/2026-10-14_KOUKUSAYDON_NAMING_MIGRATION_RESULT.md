# KoukuSaydon 이름 통일과 4인 통신 검증

사용자는 Loader 호출명뿐 아니라 저장 ID·파일명·Client/Server 게시 데이터까지 KoukuSaydon으로 통일하고, 아레나 진입 이후 4인 플레이 경로를 확인하도록 요청했다. 이 결과는 이번 이름 통일의 적용 범위와 실제 실행 증거를 기록한다. 이전 컨벤션·주석 정비의 40개 경로와 미완료 감사 범위는 `../10-11/2026-10-11_CODE_READABILITY_RESULT.md` 및 같은 날짜 audit CSV에 따로 기록했다.

## 적용한 이름과 데이터 경계

- `CLoader::Ready_For_KoukuSaydonArena()` 선언·정의·LevelRegistry 호출을 연결했다. Loader.cpp:893의 NPC 키, JSON의 `worldId`, 파일 접두사는 모두 `KOUKUSAYDON_ARENA`다.
- 자체 코드·저장 ID의 Kakul/KAKUL/kakul을 Kouku/KOUKU/kouku로 통일했다. Level 클래스와 파일, World/Boss/Encounter/Pattern/Stage/UI 식별자, 생성 도구와 검증 도구가 같은 이름을 사용한다.
- 네 C++ 파일과 UI 저작 폴더 경로를 변경하고 `.vcxproj`/`.filters` 참조를 함께 맞췄다. 숫자 enum과 wire 값은 유지했다. `WORLD_ID::KOUKUSAYDON_ARENA=5`, debug enter packet=52, raid target=1, room command=12, protocol=133이다.
- 최초 적용은 코드 138·저작 JSON 70·도구 114·현재 계약 문서 6개, 합계 328파일이다. 이후 provenance receipt, 현재 학습 문서 2개, 기존 TCP 검증 도구 2개, non-JSON Deploy 설명 1곳을 추가 반영했다.
- Resources 세 경로를 옮겨 총 3,171파일의 이름을 연결했다. 각 파일의 내용 hash는 그대로다. `Character/KoukuSaton`은 추출 원본의 실제 경로이므로 유지했다. 과거 cook 기록·작업 문서·사용자 임시 저장본은 현재 저장 계약으로 취급하지 않는다. 옛 파일 제거 목록과 구형 이름 금지 검사는 의도적으로 옛 문자열을 유지한다.
- 구형 runtime 월드 파일 4개는 새 파일의 생성·worldId를 확인한 뒤 hash를 재확인하고 `out/KoukuNamingMigration20261014/RetiredRuntime`에 보관했다. 다른 runtime 파일을 삭제하지 않았다.

적용 전후 파일, hash, 독립 검토와 복구용 원본은 `out/KoukuNamingMigration20261014`에 있다. `applied.json`, `followup-applied.json`, `runtime-test/applied.json`, `runtime-startup/applied.json`, `deploy-description/applied.json`, `retired-runtime.json`이 각 적용 기록이다. 실행 중 사용자가 편집한 Loader의 함수명·메모·공백은 보존했다. 커밋·push와 GUI Client 실행은 하지 않았다.

## 저작 값 보존과 생성기 차이

원본 JSON 70개에 대한 최초 치환은 문자열 25,224개와 객체 키 1개뿐이었다. 정수·실수·bool·null·배열 순서와 나머지 필드는 그대로였다. Resources 3,171파일도 내용 hash가 일치했다. 증거는 `data/review-root.json`, `data/postapply-review.json`이다.

게시 후 70개 중 68개는 승인 후보와 바이트까지 같다. 생성기가 소유하는 patternbindings와 Encounter 두 파일에는 다음 파생 차이가 있다. 이를 저작 수치 변경이나 이름만의 바이트 치환이라고 기록하지 않는다.

- targeted combat visual 25행의 정렬과 파생 `clientVisualId`가 재생성됐다.
- occurrence/instance/track 이름이 hash 입력인 collider ID 404개와 pursuit payload hash 8개가 재생성됐다.
- Pattern91~93 card region world track의 `rotationW` 10곳은 `0.12093538735790024`에서 `0.12093538735790046`로 재계산됐다. 절대 차이는 `2.220446049250313e-16`이며 `float32` 값은 같다. 저작 원본은 동일하고 생성기의 atan2/cos 계산 결과 차이다.

공식 provenance receipt는 기존 updater의 overlay 출력으로 갱신했다. 4,282항목에서 이름 정규화 후 추가·누락·수치·출처 정책 차이는 없다. 생성기의 `Sort-Object` 때문에 3,958항목의 배열 위치가 바뀌었다. 로그의 “66 fields”는 이름이 달라진 키의 재구성이며 PROJECT_TUNED 수는 전후 4,012개다. 증거는 `receipt-review.json`이다.

게시 JSON 9개도 독립 비교했다. Rendering, map world sequences, world NPC/stage marker/viewer 5개는 이름 정규화 후 내용 전체가 같다. Client/Server Guide는 파생 revision만, Composition 2개는 sourceManifest hash/bytes와 compositionRevision만 달라졌다. 현재 원본 18개의 manifest hash/bytes도 재계산해 대조했다. Mario 및 쿠크 base FXAA OFF와 베른 fog OFF를 포함한 렌더링 옵션은 보존했다. 증거는 `data/postpublish-review.json`, `data/runtime-json-review.json`이다.

## 게시와 제품 빌드

사용한 기존 게시 경로는 다음과 같다. PowerShell 프로세스 PATH에는 실제 Python runtime을 지정했다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2500
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GuidePipeline/Publish-Guide.ps1 -Mode Publish
```

첫 owner 게시가 PASS했다. 쿠크 product는 114패턴·573 stage·9 bundle, map은 3,369 placement, world는 114 placement와 spawn group 7개·stage marker 5개를 게시했다. gameplay bootstrap은 109,868행·32,324,123byte였다. Composition·Rendering·Guide 게시도 PASS했다.

중간 실패는 보존했다. Rendering 첫 시도는 실제 Python PATH 누락, Guide 첫 시도는 패턴 게시보다 앞선 순서 때문에 거절됐다. 올바른 Python과 의존 순서로 재실행해 성공했다. 마지막 Deploy 설명의 변경은 `Scope Deploy`가 catalog 변경을 거부하여 `Scope Area`를 포함한 기존 owner 경로로 재게시했고 PASS했다. 이때 product/world/gameplay는 정확한 입력·출력 검사 후 재사용됐다(`publish-kouku-final.log`). 현재 Server 게시와 격리 168파일의 hash도 다시 일치했다. 검증을 끄거나 runtime 파일을 직접 문자열 치환하지 않았다.

| 검증 | 결과와 증거 |
|---|---|
| Debug Product 컴파일·링크·배포 | PASS. `out/BuildPipeline/runs/20261014T110556760Z-debug-product.json`, Engine→Shared→Server→Client. Client OBJ 294개. |
| 게시 완료 후 실행 입력 검사 | PASS. `20261014T111134469Z-debug-product.json`, `SkipBuild=True`. 런타임 파일 존재·Navigation 참조·Item/Valtan reward 입력 검사. |
| NetworkProtocolHarness build | PASS, 경고 0·오류 0. `network-codec-build.log`. |
| NetworkProtocolHarness 전체 실행 | PASS 1,687개, failures 0. `network-codec-test.log`. 실제 TCP가 아닌 packet/codec 계약 검사다. |
| CharacterSelectIsolationHarness build | PASS, 경고 0·오류 0. `runtime-test/build.log`. |
| 입장 Python 계약 | 9 PASS. `testlogs/result.json`. |
| Native composition 소스 계약 | 3 PASS. 같은 결과 파일. |
| Loader·Registry 직접 검사 | 2 PASS. 같은 결과 파일. |

제품 빌드에는 기존 C4819/C4828 인코딩 및 DirectXTK PDB LNK4099 경고 등이 남았다. 경고 전체를 해결한 작업이 아니다. 최초 제품 빌드가 게시보다 먼저 끝나 새 world 파일이 아직 없다는 경고를 출력했으며, 게시 후 입력 재검사는 통과했다. Release 빌드를 수행했다고 기록하지 않는다.

기존 광역 소스 검사에는 이름 경계 6개 중 2개 실패가 남는다. Valtan 모듈 import와 Valtan 담당 파일 내부 Kouku 참조를 금지하는 검사다. Valtan cutscene 검사는 Loader CP949 읽기를 수정한 뒤 기존 actor-clock 문자열 기대값에서 실패했다. 이 실패를 이번 이름 통일의 통과 결과에 섞지 않았으며 무관한 제품 코드를 고치지 않았다.

## 실행 중 발견한 진단 JSON 출력 수정

3회차 서버 종료 로그에서 `SessionDiagnostic INVALID_JSON`이 발생했다. `NETWORK_PROTOCOL_VERSION`은 현재 `std::uint8_t=133`이고 ServerApp.cpp:199가 narrow ostream에 그대로 넣어 숫자가 아닌 문자로 출력했다. 같은 서버의 JSON validator는 INTEGER를 요구한다. Client는 이미 uint16_t snapshot을 소비해 해당 문제가 없었다.

Server 출력식 한 곳을 `static_cast<std::uint16_t>(LostArk::Shared::NETWORK_PROTOCOL_VERSION)`로 변경했다. 프로토콜 타입·값·packet wire는 그대로다. 독립 바이트 검토 후 Server 증분 컴파일·링크가 경고0·오류0으로 통과했다(`diagnostic-protocol/build.log`). 캡처된 텍스트는 이미 디코딩된 문자열이므로 원래 raw byte를 보존한 로그라고 표현하지 않는다. 최종 실제 종료 진단 파일의 4줄 모두 JSON parse에 성공했고 protocol133은 정수였다. `INVALID_JSON`은 0건이다.

## 실제 4 TCP 세션 검증

최종 4회차는 실제 Server와 네 개 TCP 연결에서 아래 전체 시험을 통과했다. 단순 입장 또는 codec PASS를 4인 플레이 전체 성공으로 바꾸지 않으며 시험 범위를 명시한다.

최신 Server 게시 데이터 168개를 `out/KoukuNamingMigration20261014/runtime-test/ServerDataFiles`에 복사했고 게시 원본과 모든 hash가 같다. Server는 loopback `127.0.0.1:17778`에서 headless로 실행하고 이 자식 프로세스에만 격리 root를 지정한다. 팀 LAN 환경과 기존 서버 프로세스에는 손대지 않는다.

시험은 실제 Bern 네 세션 입장·파티·쿠크 입장 투표·아레나 승인·roster·이동을 확인한다. 이후 기존 Debug 직접 위치 이동/보스 배치를 시험 준비로 사용하고 네 명 각각의 `SourcePlayerId`가 있는 실제 스킬 피해 및 보스 HP 감소를 확인한다. 위치 준비는 서버의 navigation·높이·동적 충돌 검증을 거쳐 typed ACCEPTED 응답과 실제 snapshot을 함께 확인한다. 피격은 게시 Pattern6 댄스의 POSE_INPUT 시간 초과에 대한 서버 판정이다. EXIT 투표→Bern 복귀와 한 명 disconnect 제거까지 확인한다. 네 명은 모두 LANCE_MASTER이므로 모든 class 조합의 검증은 아니다.

10초와 15초 시작 대기를 사용한 첫 두 시도는 Server listen 이전에 중단됐다. 두 번째 초기화 중 자기 PID의 CPU 11.88초·약128MB 사용을 관측했다. 두 프로세스는 정리됐고 게임 연결 검사는 시작되지 않았다. 독립 검토 후 쿠크 모드만 명시적인 최대60초 startup 대기를 허용했다. 다른 모드의 15초 제한·기본10초·접속 후120초 시험 제한·자기 PID 정리는 그대로다.

3회차는 약23초에 Server가 준비됐고 실제 4인 입장·roster·spawn·이동까지 통과했다. 전투 준비에서 선택한 `stage.kouku.sl04`는 게시 데이터에서 disabled이므로 `Is_PlayerSpawnClear`가 거절했다. 이름 불일치나 네 명의 중첩이 원인은 아니었다. 해당 waypoint를 활성화하지 않고 위의 기존 직접 위치 명령으로 시험 도구를 수정했다. 이 후보도 독립 검토와 하네스 재빌드(경고0·오류0)를 통과했다. 증거는 `runtime-test/teleport-diagnosis.json`, `runtime-position/review.json`, `runtime-position/build.log`이다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Network/Run-CharacterSelectIsolationHarness.ps1 -Configuration Debug -Scenario Party4 -PartyTarget KoukuSaydon -HarnessPort 17778 -ServerStartupTimeoutMilliseconds 60000 -ServerDataRoot C:/Users/tnest/Desktop/LostArk/out/KoukuNamingMigration20261014/runtime-test/ServerDataFiles
```

최종 실행은 `runtime-test/run-04.log`에서 exit0·failures0이며, 원본 실행 로그와 영속 진단 파일을 root가 다시 읽어 `runtime-test/review-root.json`에 독립 대조했다. 회원별 상세 수치·binary/source hash는 `runtime-test/run-04-result.json`, 네 차례 시도는 `runtime-test/attempts.json`, 원본 로그·영속 진단 사본은 `runtime-test/Evidence/run-04`에 있다. 종료 후 격리 데이터168파일은 모두 실행 전 hash와 같았다.

| 실제 확인 단계 | 관측 결과 |
|---|---|
| 네 Bern 세션·파티 구성·쿠크 입장 투표 | 네 명의 승인과 정확한 roster/leader 유지 PASS |
| 쿠크 입장 이후 상태·이동 | PlayerId 1~4, NetEntityId 100~103의 별도 네 인간 세션 PASS |
| 전투 준비 위치 | X=-5/-3/-1/1, Z=730 요청에 네 typed ACCEPTED 및 실제 snapshot 확인 |
| 각자의 공격 | 네 명 모두 skill34110 및 자기 PlayerId가 출처인 양수 피해 확인 |
| 보스 HP | 1,699,175,467 → 1,682,235,005, 실제 감소 16,940,462 |
| 게시 Pattern6 실패 피격 | 네 명 모두 incoming damage event와 HP 감소 확인 |
| 플레이어 HP | 각132,000 → 117,738 / 119,743 / 118,253 / 119,105 |
| EXIT 투표·Bern 복귀 | 네 명 모두 세 번째 admission 승인, 새 Bern snapshot PASS |
| 한 명 연결 종료 | 남은 세 명의 snapshot에서 종료한 한 명 제거 PASS |
| 진단 및 큐 정리 | 네 session 진단 JSON parse PASS, protocol133 정수, 모두 leaveEnqueued=true·queuedFramesAfterClose=0·sendFailures=0·reliableRejected=0 |

진단 정본은 `Server/Bin/Debug/Diagnostics/server-session-37576.jsonl`이다. 시험에서 명시적으로 닫은 한 연결은 receive error10054로, 나머지 세 연결은 정상 EOF로 기록됐다. stdout 복사의 마지막 JSON은 정리 시점에 잘린 채 캡처됐지만, 영속 JSONL의 네 줄은 모두 완전하고 유효했다. 두 증거를 혼동하지 않았다.

네 명 모두 살아 있었고 사망·부활 경로는 시험하지 않았다. 화면·입력·사운드·이펙트와 자연 이동 경로의 전체 레이드 클리어는 이 TCP 도구의 검증 범위가 아니다. 실제 Client 네 화면과 전체 레이드의 사용자 확인이 남는다. 강제 사망 또는 GPU 준비 완료를 가짜로 보내 성공 조건을 우회하지 않았다. 사용한 자기 Server/하네스 프로세스와 17778 listener는 종료 후 정리했고 `runtime-test/run-04-cleanup.json`에 기록했다.

## 최종 파일 검사

최종 파일 검사와 diff 결과는 `final-validation.json`, `diff-check.log`, `diff-whitespace-review.json`에 둔다. 기존 사용자·다른 작업의 trailing whitespace 20건은 전부 이번 적용 전 baseline에도 존재했다. 이를 정리한다는 이유로 사용자 메모·공백을 고치지 않았다. 따라서 working-tree 전체 `git diff --check`를 무조건 PASS라고 적지 않는다.

현재 제품 소스와 활성 저작/게시 데이터의 옛 이름, JSON 중복 키·구문, XML, 새 경로의 존재와 옛 파일 부재 검사는 PASS했다. 최초 328개 후보 중 생성 파일 2개와 진단 수정 1개의 후속 차이는 각 독립 검토 hash와 일치했다. receipt·학습 문서·검증 도구·Deploy 설명 등 후속 적용도 최종 후보 hash와 일치한다. 과거 문서·원본 cook metadata·기존 임시 저장본·구형 이름 금지/정리 검사는 제외 근거를 유지한다. 전체 working tree diff 검사에서 유지한 기존 공백 20건은 위와 같이 별도 기록한다.
