# 캐릭터 6슬롯 상태 저장·복원 결과

## G01. 실제 반영

기준선 `dc3a482d`, 브랜치 `codex/character-slot-state-restore`에서 구현했다.
최초 생성한 고정 슬롯과 local character ID를 유지하고, 같은 직업·닉네임도 독립적으로
취급한다. EXE별 메모리 로스터이며 종료 후 영구 저장이나 개인 JSON 읽기/쓰기는 없다.

캐릭터 선택 복귀는 기존 즉시 HUD 복사/연결 종료 대신 Server room FIFO의 최종 capture
응답을 기다린다. 직전에 보낸 구매·장착·강화가 포함되며 inventory 전체, 강화 단계,
내구도, 장착 아바타, 실링, 골드, 칭호를 저장한다. 요청 sequence·수신 세대·world·
player/entity/class가 일치해야 해당 캐릭터 ID에 반영한다. 이후 낡은 HUD 저장은 차단한다.
거부·timeout은 기존 슬롯을 보존하고 재시도할 수 있다.

기존 캐릭터 재선택은 Bern admission/identity commit 뒤 typed 복원을 요청한다. 성공
응답까지 gameplay/economy 명령을 막고, 복원에는 위치가 없으므로 Server의 Bern 시작
spawn을 유지한다. 실패 시 저장본을 보존하고 Lobby로 복귀한다. 이전 복원 실패의 보호
상태가 새 생성용 Character Select arena 입력까지 막지 않도록 차단 범위를 구분했다.
새 캐릭터 생성 commit 실패도 이전 active identity를 보존한다.

선택창 모델은 생성 외형을 적용한 뒤 해당 슬롯 inventory에 저장된 장착 아바타를 기존
EquipmentPresentationService로 적용한다. catalog/model 오류 시 기존 모델을 유지한다.
아이템150개의 visual-set/부위 연결에 누락이나 불일치가 없음을 확인했다.

## G02. 강화와 재화

protocol133의 inventory 항목 `iUpgradeLevel`을 Client/Server가 함께 소비한다.
강화는 Bern 슈미트 NPC 근처에서 typed 요청으로 제출하며 Server가 소유 아이템·장착
slot·기대 단계·sequence·거리와 상태를 검증한다. 기본 Honor Whisper10단계, 무료 시도,
성공률50%인 기존 정책을 유지하고 Server가 성공 시1단계를 올린다. snapshot과 typed
결과의 송신 큐 준비가 끝나야 상태를 commit한다.

전역 itemId별 UI map과 Client 성공 RNG를 제거했다. UI는 서버 단계와 결과만 표시하고,
허구의 재료9999개와 공격력 증가 문구를 무료/성공률 안내로 교체했다. 단계별 전투
능력치 증가는 기존에도 없었으며 이번 저장·복원 범위에 새 산식을 추가하지 않았다.

장착/해제, stack split, 월드 이동과 capture/restore가 강화와 내구도를 함께 보존한다.
빈 inventory를 fresh 입장으로 잘못 판단해 초기 재화를 재지급하던 경로는 명시적인
carried-state 인자로 교정했다. 정상 재화가 임의의20억 상한 때문에 복원 거부되지
않도록 Server의 실제 uint32 전체 범위를 저장한다.

## G03. 실행한 검증

| 검증 | 최종 결과 | 증거 |
|---|---|---|
| Debug Product Build | PASS, missing/invalid runtime inputs0, CSO 변경0 | `out/BuildPipeline/runs/20261002T210755544Z-debug-product.json` |
| Server character-state |40 PASS /0 FAIL| `out/CharacterStateRestore/server-character-state-debug.log` |
| Server battle-items |183 PASS /0 FAIL| `out/CharacterStateRestore/server-battle-items-debug.log` |
| NetworkProtocolHarness |1687 PASS /0 FAIL| `out/CharacterStateRestore/protocol-debug.log` |
| 실제 Client roster/state+Shared codec |12/12 PASS| `out/CharacterSession20261003/client-debug/results.json` |
| 실제 receive dispatch Debug |9/9 PASS| `out/CharacterSlot20261003/receive-debug.log` |
| 실제 replication/강화 consumer Debug·Release |각9/9 PASS| `out/CharacterSlot20261003/handoff-debug.log`, `handoff-release.log` |
| 신규 강화 문구 spritefont glyph |5문구 누락0| `out/CharacterSession20261003/upgrade-label-glyphs.json` |
| 프로젝트/필터 XML |Client/Server/Shared6개 parse 성공| 기존 등록 재사용, 새 C++ 파일 없음 |
| Git whitespace |`git diff --check` 성공| 소스·문서 최종 검사 |

Server 검사는 실제6개 캐릭터의 순차 입장/복원, UINT32_MAX 재화, 강화 성공·실패·중복
sequence·잘못된 slot/단계·NPC 거리, 아바타·내구도, FIFO capture, 빈 가방 이동,
응답 송신 준비 실패 시 기존 상태 보존을 포함한다. 메모리 상태 검사는 같은 직업·닉네임
6슬롯 독립, 생성 commit rollback, stale/잘못된 capture identity와 복원 실패 보호를 포함한다.

최초 빌드에서 강화 sink를 raw pointer로 추론한 컴파일 오류를 `shared_ptr` 사용으로
고쳤고 최종 Product Build를 다시 통과했다. 최초 Server fixture의6명 동시 입장은 실제
Bern4명 제한과 달라 순차6캐릭터로 교정 후 모두 통과했다. receive fixture의 기존
numeric HUD/Pump stub 누락도 실제 소비자 API에 맞췄다. 실행하지 않은 검사를 성공으로
기록하지 않았다. 기존 인코딩/외부 DirectXTK PDB 경고는 남아 있다.

## G04. 실행 준비와 사용자 화면 확인

새 실행 파일은 `Client/Bin/Debug/Client.exe`와 `Server/Bin/Debug/Server.exe`다.
protocol이133으로 바뀌었으므로 둘 다 같은 소스로 빌드한 실행 파일을 사용한다.
Client 작업 디렉터리는 `Client/Default`다. 다른 PC의 기존 Server/Release 실행 파일은
자동으로 갱신되지 않는다. Resources와 저작/게시 데이터는 이번 변경 대상이 아니다.

Client/UI는 실행·조작·캡처하지 않았다. 사용자가 확인할 경로는 다음과 같다.

1. 선택창의 임의 슬롯A에서 캐릭터를 생성해 Bern에 입장한다.
2. 아이템과 재화를 얻고 장비/아바타를 착용한 뒤 슈미트에서 강화를 시도한다.
3. 캐릭터 선택 버튼으로 복귀해 최초 슬롯의 닉네임·모델·착용 아바타를 확인한다.
4. 다른 슬롯B를 생성하거나 선택해 플레이한 뒤 다시 선택창으로 돌아온다.
5. 슬롯A로 재입장해 Bern 시작점, 인벤토리·골드·실링·장착·강화·내구도를 확인한다.

EXE 종료 전 슬롯 전환을 검증하며, 종료 후 상태가 사라지는 현재 계약과 구분한다.
자동 검증은 최종 GPU 외형·클릭 UX·실제 LAN 플레이의 화면 판정을 대체하지 않는다.


## G05. PR506의 최신 main 통합 검증

PR506의 원래 head `96a06a6dd7ddd13a82782b1371532080249a6363`은 Draft 상태로 미병합이었다.
렌더링 PR507까지 반영된 main `c7de2091dd51655b14c90c05cbed5f5ba6c7e94b`을 기능 브랜치에
충돌 없이 병합한 통합 commit은 `5cfdfdd3af2614fd91f5c28052aebab48b4e6b73`이다.
MainApp의 캐릭터 전환과 Profiler 계측을 함께 유지하며 Engine 렌더링 코드·기존 rendering
설정·Data/DataFiles에는 추가 변경이 없다. 사용자 `.gitignore`와 미추적 자료도 보존했다.

이 통합 소스에서 Debug Product를 다시 빌드해 PASS(exit0,435,620ms)를 확인했다.
근거는 `out/BuildPipeline/runs/20261002T220846175Z-debug-product.json` 및
`out/PR506Integration/product-debug.log`다. 기존 코드 페이지·shader·외부 PDB 경고는 남았고
Client/Server UI를 시작하지 않았다. 아래 focused 검사는 모두 현재 통합 소스로 재실행했다.

| 검증 | 결과 |
|---|---|
| Server character-state |40 PASS /0 FAIL|
| Server battle-items |183 PASS /0 FAIL|
| NetworkProtocolHarness 재빌드·실행 |1687 PASS /0 FAIL|
| Client roster/state+Shared codec Debug |12/12 PASS|
| Client receive dispatch Debug |9/9 PASS|
| Client replication/강화 consumer Debug·Release |각9/9 PASS|
| Client/Server/Shared project·filters XML |6개 parse PASS|
| 정확한 PR 소스·데이터 보존·diff check |PASS|

원시 로그와 집계는 `out/PR506Integration/validation.json`, `structure.json`에 있다.
Client lifecycle, Server/Shared authority, Client network/presentation의 독립 검토 기록도 같은
폴더에 두며, 병합 commit/tree 대조는 최종 merge 기록과 PR506에서 확인한다. 원래 G04의 사용자
화면 확인은 여전히 남아 있고 자동 검사로 육안 결과를 대신하지 않는다. protocol133 변경 때문에
노트북과 연결할 Server 모두 최신 main의 Client/Server Product를 함께 빌드해야 한다.


## G06. 2026-10-05 선택창 복귀 전에 장착 외형 준비

현재 기준선 `c999f7e91`에서 지연의 실제 원인은 슬롯 저장 누락이 아니라 준비 시점이었다.
`Return_ToCharacterSelect`의 서버 capture는 구매·장착 뒤 상태를 저장하지만 Lobby loader는
공통 shader/prototype만 준비했다. Lobby 활성화와 선택창 Open 후 `Update_Stage`가 실제
class 모델을 하나씩 준비하고 그 뒤 저장된 아바타를 적용하므로 빈 카드가 먼저 보였다.

명시적인 월드의 캐릭터 선택 복귀는 같은 `CLevel_Loading`에서 기존 시작되는 운명 배경·제목·
진행바를 표시한다. 공통 prototype 준비 뒤 채워진 슬롯만 기존 async class service로 준비한다.
각 실제 CCharacter의 생성 외형은 `Apply_SavedLook(requireExactOutfit=true)`, 장착 아바타는
기존 `EquipmentPresentationService::Apply_Preview`로 반영한다. 둘 다 성공하고 한 번의
Loading Render가 진행된 뒤만 Lobby activation을 요청한다. MainApp이 다음 Lobby update에서
같은 stage를 유지한 채 창을 열므로 이후 모델을 다시 순차 준비하는 지연을 없앴다.

`Get_ActiveCharacterId`를 기존 selection mutex 안에서 복사하고, 창은 그 ID와 고정 roster를
대조해 복귀한 캐릭터의 원래 카드를 선택한다. 첫 생성 캐릭터도 직업·이름이나 첫 배열 행으로
추측하지 않는다. 초기 EXE의 빈 Lobby는 class 모델을 준비하지 않으며 닫힌 창의 Update가
준비 요청을 강제하던 기존 문서 불일치도 교정했다. 기존 열린 창에서 시작한 job은 Close 뒤
정상적으로 마무리할 수 있다.

반환 Loading은 MainApp 소유 창을 차용한다. 창에 정확한 `CLevel_Loading*` owner를 보관해
Retry의 새 Loading이 먼저 만들어지고 이전 Loading이 나중에 소멸하는 순서에서도 이전 owner가
새 준비를 취소하지 못한다. non-Lobby UI cleanup은 이 준비 중인 stage를 지우지 않는다.
실패는 새로 만든 stage 배우만 제거하고 메모리 roster/서버 capture 저장을 유지한다. 최대5분
준비 제한 뒤 기존 asset service의 취소·I/O 중단과5초 bounded join을 거쳐 기존 LoadingRecovery
Retry를 제공한다. 실패 표시 이후 Update는 activation을 다시 제출하지 않는다.

진행률은 공통 Loader20%와 실제 완성 카드80%를 사용한다. 모델 준비가 시작되자마자99%로
오르는 표시는 피하고, 모든 준비가 완료된 경우에만100%를 표시한다.

### G06 검증과 남은 확인

- 실제 production `CharacterRoster.cpp`, `CharacterSelectionState.cpp`, Shared codec을 컴파일한
  기존 focused 검사12/12 PASS. `out/CharacterReturn20261005/state-debug/results.json`에 기록했다.
- Client 프로젝트와 filters XML parse 및 변경한 기존7개 H/CPP 등록이 모두 일치했다.
- 변경한7개 C++ 파일은 기존 UTF-8 BOM 없음·CRLF를 유지했고 `git diff --check`를 통과했다.
- root 세션의 첫 Debug Product는 `out/BuildPipeline/runs/20261004T210614691Z-debug-product.json`에서
  PASS했다. 이후 exact outfit 인자1줄을 추가했으므로 최종 Debug 증분/Release 결과는 root의
  아래 G07을 따른다. G06의 첫 빌드와 G07의 최종 빌드를 구분한다.
- Client/UI를 실행하거나 화면을 캡처하지 않았다. 실제 클릭 UX와 GPU에서 보이는 첫 모코코
  모델은 사용자 화면 확인이 남아 있다.

사용자 확인 경로는 Debug 캐릭터 슬롯A로 Bern 입장 → 모코코 구매·장착 → 캐릭터 선택 복귀다.
시작되는 운명 화면 후 같은 슬롯A가 선택되고 모코코 외형이 첫 선택창 화면에 보여야 한다.
다른 슬롯B를 거친 뒤A로 돌아와도 동일 ID의 상태를 유지해야 한다. 실패 재시도는 기존 Retry
버튼을 사용하며, EXE 종료 후 메모리 로스터가 사라지는 기존 수명 계약은 그대로다.

## G07. 10-05 베른 후속 최적화와 함께 수행한 최종 제품 검증

최종 exact outfit 인자를 포함해 정상 증분 Product를 Debug와 Release에서 통과했다. Engine·Shared·Server는 기존 유효 산출물을 재사용했고 Client의 변경된 소스와 헤더 소비자13 OBJ를 각각 컴파일·링크했다. 첫 Debug 이후 exact outfit 한 줄 보완으로 Debug를 한 번 더 빌드해 해당 OBJ1개와 EXE를 갱신했다. 세 빌드 모두 CSO 변경0, 누락/비정상 runtime 입력0이며 다른 데이터 domain을 publish하지 않았다.

| 실행 | 결과 | 근거 |
|---|---|---|
| 첫 Debug Product |PASS,37.680초|`out/BuildPipeline/runs/20261004T210614691Z-debug-product.json`|
| 최종 Release Product |PASS,67.998초|`out/BuildPipeline/runs/20261004T210803760Z-release-product.json`|
| exact outfit 포함 최종 Debug 증분 |PASS,9.021초|`out/BuildPipeline/runs/20261004T210827276Z-debug-product.json`|
| 실제 roster/state+Shared codec |12/12 PASS|`out/CharacterReturn20261005/state-debug/results.json`|
| 기존 chunk resolver fixture |26검사 PASS|`out/BernChunkAdmission20261005`|
| Client/Engine project·filters XML |4개 parse 성공|`out/BernRosterFollowup20261005/validation.json`|
| 이번 수정 범위 whitespace |PASS|tracked scoped diff-check 및 미추적 chunk CPP 검사|

초기 Debug·Release 상세 빌드 로그와 최종 Debug console은 `out/BernRosterFollowup20261005`에 있다. 기존 C4819/C4828 문자 집합 및 Debug 외부 DirectXTK PDB 경고는 남고 오류는0이다. 전체 worktree diff 검사에는 동시에 편집 중인 별도 Warlord PLAN의 EOF 빈 줄이 검출됐으며 이번 범위 밖이라 변경하지 않았다. 이번 C++·문서 범위 검사는 통과했다. 대규모 기존 dirty worktree와 다른 세션 변경을 보존하고 자동 stage/commit/push하지 않았다.

실행 파일은 `Client/Bin/Debug/Client.exe`(10월5일06:08 KST), `Client/Bin/Release/Client.exe`(06:07 KST)다. 작업 디렉터리는 `Client/Default`다. 최종 확인 시 Client/Server는 실행 중이지 않았으며 에이전트가 실행·조작·캡처하지 않았다. 모코코 구매·장착 → 캐릭터 선택 복귀의 첫 화면과 실제 베른 FPS 상승은 사용자 확인이 남아 있다. 소스/빌드와 화면 판정을 구분한다.
