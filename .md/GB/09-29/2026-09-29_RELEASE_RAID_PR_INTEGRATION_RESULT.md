# Release 레이드 통합·실시간 수치 저장·실행 ZIP 결과

## 구현 완료

PR #475 상점/재화, #476 마하라카, #477 창술사/가디언나이트, #478 캐릭터 선택과
#479 발탄 저장본을 `codex/release-integration-20260929`에 ancestry를 보존하여 통합했다.
독립 packet 변경을 protocol 120으로 합치고 Artist effect catalog/carrier와 Bern 진입 충돌을 조정했다.

Release F1 Balance Test는 Server 활성 수치 목록을 읽고 typed Save + Apply를 제출한다.
PLAYER/SKILL/DAMAGE/BOSS/MADNESS/STAGGER/PATTERN_DAMAGE 7종을 지원한다.
일반 스킬과 ALT_V를 분리하며 계수/고정 가산 피해, ALT_V 총 boss 체력줄 피해,
쿠크 최대 HP 비율·고정 피해, 실제 패턴 무력화 문턱을 수정한다. native Server가 기존
catalog validator로 검증하고 canonical source, Retail/provenance, bootstrap과 receipt를
원자 저장한 뒤 모든 shared/private room에 동일 numeric revision을 적용한다.
신규 입장은 저장 경계 뒤 처리하며 경쟁 저장은 BUSY, 오래된 draft는 STALE_REVISION으로 거부한다.
저장 실패는 기존 파일과 실행 메모리를 보존하고 Client draft를 덮어쓰지 않는다.

기존 gameplay/presentation revision과 실행 중 패턴 순서는 유지한다. 최대 HP/자원은
현재 비율과 사망 상태를 보존하며 이후 판정은 새 수치를 사용한다. 사용자는
**플레이 → F1 Save + Apply 성공 → 게임 안에서 관문/레이드 재시작 → 새 수치로 플레이**한다.
저장 자체가 레이드를 재시작하지 않으며 Server/Client EXE 재시작은 필요 없다.
나중에 EXE를 다시 실행해도 receipt 검증으로 저장된 수치를 복원한다.

쿠크 P17의 현재 콜라이더와 연결되지 않는 이전 피해 logic 6/7/8을 비활성화하고,
P109 빙고 망치의 기존 collider 9에 누락된 logic 1 연결을 복원했다. 원래 10%와
콜라이더 크기를 보존하고 타이밍을 collider 구간 1122/112 ms에 맞췄다.
정식 projection/publisher로 GATE1/GATE2/GATE3/BINGO 네 관문을 게시했다.
발탄의 비어 있는 보조 loop 공격들은 의도한 비피해 항목이라 임의 피해를 추가하지 않았다.
렌더링 옵션과 Mario FXAA OFF는 변경하지 않았다.

## 실제 실행 검증

- Release Engine/Shared/Server/Client 정상 Build 완료. 최종 Product Build PASS:
  `out/BuildPipeline/runs/20260928T201651456Z-release-product.json`.
  clean/rebuild 없이 기존 중간 산출물을 사용했다. 기존 shader/PDB warning은 존재한다.
- NetworkProtocolHarness Release 전체 PASS: numeric serialization/validation과
  통합 상점·Waterpang packet 포함. protocol size 검사의 누락된 Waterpang byte를 수정했다.
- Valtan lifecycle/presentation PASS. 네 CClientSession의 실제 Join, typed 이동/G 입장,
  3개 컷씬 stage와 등장 패턴, 8개 HP 구간의 순서·한 바퀴 반복, 7개 HP 기믹,
  Death→Respawn 40줄 복원, 실제 승인된 플레이어 스킬로 유령 처치,
  전원 동일 snapshot과 각 1회 clear/MVP 및 보상 수신까지 연속 확인했다.
  `VALTAN_4P complete=1 ticks=23344 window=7 mechanics=7 parity=1 ready=1`.
  로그: `out/ReleaseIntegration20260929/valtan-lifecycle-final.log` (failures 0).
  생존용 무적과 HP 문턱 입력은 fixture에서만 사용하며 제품 stage/timer/분기는 조작하지 않았다.
  처음의 무진행은 fixture가 G 전 실제 이동 명령을 생략하여 combat-ready가 되지 않은 원인이었다.
- Kouku raid, Bingo, NPC raid return Release PASS. 관문 입장/준비/재시도, 1~3관문과
  Bingo 종료/MVP 경계는 실제 Server simulation으로 확인했다.
  로그: `out/ReleaseIntegration20260929/raids/`.
- Numeric 실제 네 세션 PASS: 비공대장 저장 → 네 명 APPLIED, 경쟁 BUSY/stale,
  잘못된 수치 거절, Windows 파일 잠금에 의한 저장 실패 rollback,
  새 방 동일 수치와 EXE 재시작 catalog 복원을 확인했다.
  저장 후 Valtan Play/Restart가 새 boss HP를 사용하며 쿠크 10→8%와 ALT_V
  35→34 체력줄 피해를 독립 저장/복원한다.
  로그: `out/ReleaseIntegration20260929/numeric-balance-final.log` (failures 0).
- Skill stages Release PASS: notify 피해 창, 창술사 chain 및 Guardian Knight 포함.
  World playback Release PASS: Valtan의 새 게시 sequence 목록과 Kouku raid entry 예약을
  반영하여 오래된 fixture 가정을 교정했다. 로그는 같은 폴더의 `*-final.log`.
- 패키지 도구 nonlaunch 19 tests PASS. 외부 Server data 환경변수도 bundle 내부 경로로
  고정한다. 실제 Client/UI를 실행하지 않고 launcher manifest/hash/격리를 검사했다.
- 통합 변경 JSON 117개와 XML 6개 parse PASS, numeric source 576 fields/6 files 검증 PASS,
  git diff --check PASS. 리소스 참조 10,518개에서 누락 0건을 확인했다.

## 배포

`Tools/ReleasePackaging`에 기존 portable 실행 흐름을 재현 가능한 도구로 포함했다.
최상위 LostArk.exe, ServerHost.cmd, Release DLL/compiled shader, 필요한 Data/DataFiles와
app-local VC runtime을 포함하며 Resources는 Drive 공유본을 선택한다.
사용자 profile/CharacterRoster.json과 Resources는 ZIP에 포함하지 않는다.
새 PC도 #478의 고정 4개 클래스 카드/기본 닉네임을 보며 개인 JSON 닉네임은 별도 로컬 저장이다.
공유 endpoint는 `192.168.0.22:7777`이다.

대상: `C:/Users/user/Desktop/LostArk-Release-20260929.zip`.
기존 20260923 ZIP(127,130,057 bytes)은 보존했다. ZIP 생성과 전체 CRC/manifest SHA256 검사 PASS.
새 ZIP은 166,238,609 bytes이며 SHA256은 `59d0985e60934a232fee65af0235c8a19e4f90df19f63bf8f3486c8cb28a39c2`다.
Resources 포함 0, payload 2,740 files, authoring Data 2,127 files, compiled shader 254개다.
Launcher nonlaunch preflight PASS: Client/Server 실행 없음, endpoint .22, protocol 120,
bundled Data/Server DataFiles 격리를 확인했다.
파일 증거: `out/ReleasePackaging/portable-delivery.receipt.json`,
`out/ReleasePackaging/preflight-20260929-final.json`.
ZIP binary/source 기준 commit은 `78982951577340c2c362b040c5163bf4cd62aa78`이다.
이후 결과 기록만 변경하며 제품 파일은 동일하다. 통합 PR은 #480이다.
패키지 안 Server.exe와 패키지 Data/DataFiles만 지정한 numeric 네 세션 검사도 PASS
(`out/ReleaseIntegration20260929/packaged-numeric-final.log`, failures 0). 이 검사는
임시 복사본을 사용하므로 전달 ZIP의 초기 밸런스를 변경하지 않았다.

## 남은 사용자 화면 확인

AGENTS의 Client/UI 자율 실행 금지에 따라 실제 Client 네 창/네 PC를 실행하지 않았다.
서버 simulation/전송 frame 검증은 실제 LAN 전송 품질, GPU 표시, 실제 음향 청취 성공의
증거가 아니다. 네 명이 동일 ZIP과 최신 Drive Resources로 발탄/쿠크 컷씬·이펙트·음향을
확인해야 한다. 제품 실행 자체를 네 PC에서 완료했다고 기록하지 않는다.


## 2026-09-29 저녁 통합 Release 후속 기록

이 항목은 위 #480 배포 뒤의 #482, #484 및 Desktop 작업 통합에 대한 기록이다.
현재 캐릭터 계약은 위의 고정 4개/로컬 JSON 설명을 대체한다. EXE마다 빈 6슬롯으로
시작하고 선택 슬롯과 캐릭터 상태를 현재 프로세스 동안만 유지한다. 개인 roster JSON은
읽거나 쓰거나 삭제하지 않는다. Lobby의 캐릭터 모델 선행 로드를 제거했다.

- #482와 #484를 실제 merge commit으로 통합하고 기존 Desktop 164개 변경을 보존했다.
  원본 safety stash는 `339f78fa514b3aa14806751ceda39798f628a5c2`다.
- 기존 셰이더 변경 9개는 원본 snapshot과 내용이 동일하다. 4개는 바이트까지 동일하고
  5개는 checkout의 LF/CRLF 차이만 있다. 기존 무비 셰이더를 되돌리지 않았다.
- 요구 무력화량은 실제 Valtan STAGGER_SLOT 1000→10000, Kouku G1 및 Mario2 각각
  115000→1150000이다. 스킬 무력화 피해와 회오리 수류탄 정책은 변경하지 않았다.
- Lugaru HP는 일반 몬스터의 5배인 2380280이다. Mario 수직 공은 게시된 이동 곡선을
  소비하며 폭탄과 같은 피격 상태와 고정 1320 피해를 적용한다. World schema는 12다.
- 시간정지물약을 포함한 기존 배틀 아이템 동작을 #484와 통합했다. 네트워크 protocol은 124다.
- Valtan PublishV2, Client/Server domain publish 및 NumericSourceBindings 생성 완료.
  패키지 도구 19개 검사, 변경 JSON parse 및 diff --check 통과.
- 사용자 최신 요청에 따라 추가 장비/재화 복원 검증과 광역 진단을 중단하고 Release ZIP을
  우선한다. 새 raid/battle-item/character focused 검사는 소스에 포함되지만 이번 배포에서
  실행 완료로 기록하지 않는다. Client/UI 및 실제 4인 플레이도 실행하지 않았다.

최종 Release 빌드 및 ZIP 검증 결과는 아래 후속 항목에 기록한다.

### 저녁 통합본 최종 배포 완료

- Release Product Build PASS, skip 없음, missing/invalid runtime input 모두 0.
  증거: `C:\Users\user\Desktop\LostArk\out\BuildPipeline\runs\20260929T140535348Z-release-product.json`. Client 288 OBJ와 36 CSO가 재생성됐고 링크까지 성공했다.
- ZIP: `C:\Users\user\Desktop\LostArk-Release-20260929.zip`. 166871747 bytes, SHA256 `640ce4fd489cdee73ecf6e60f442e3570f261604de5f410ac37aa259f5673e46`.
- ZIP CRC, manifest 모든 파일 SHA256, numeric source 576개, nonlaunch preflight PASS.
  protocol 124, source revision 2469, sequence revision 183, payload 2757개, compiled shader 256개.
  Resources는 포함하지 않고 기존 외부 Resources를 선택한다.
- 이전 ZIP 백업: `C:\Users\user\Desktop\LostArk-Release-20260929.backup-20260929-230707-473865.zip`.
- ZIP의 소스 기준 commit은 `f7b56113d7f20855b495eaf85a49f957994ea043`다. 이후 문서 기록만 추가하며 제품 파일은 동일하다.
- 통합 PR: https://github.com/tnestyle70/LostArk/pull/485. #482와 #484의 head를 merge ancestry로 포함한다.
- 이번 빌드는 성공했지만 실제 Client 실행, 4인 레이드 완주 및 캐릭터 전환 화면 검증은
  하지 않았다. 이전 #480의 실행 결과를 이번 통합본의 실행 결과로 대체하지 않는다.

## G10. 배포 후 회귀와 사용자 진입 실패 조사

### 작업 기준과 다른 세션으로의 인계

후속 검사 기준은 protocol124 Release Server와 main `50dcd986dc8139815b8ad9b5263fe3ec91498eff`다.
#485 통합 뒤 #486에서 누락된 Valtan presentation generation JSON의 Git 전달을 보완했다.
해당 JSON은 기존 ZIP에도 포함되어 있었으며, 컴파일 성공과 Git 전달 완결성이 다른 경계라는
사례다. #482/#484는 #485 ancestry에 포함되어 병합됐다.

후속 브랜치는 `codex/release-regression-20260929`다. 전환 전 HEAD `ab3398a6e`와 main의
Git tree가 동일함을 확인하고 shader/C++/project/props 입력 2,273개의 SHA256과 수정시각을
전후 대조했다. 변경 입력은 0개다. 증거는 `out/ReleaseValidation20260929/branch-no-input-change.json`이다.

사용자가 쿠크 진입 실패를 보고한 뒤 **수정과 ZIP 생성을 다른 세션에 맡겼다고 명시했다.**
그 시점부터 이 세션은 제품 코드·데이터 수정, 추가 빌드, 테스트 실행, 게시, ZIP 재생성과
Git 통합을 중단했다. 이 항목과 AGENTS/gotchas의 조사 기록만 마무리한다. 다음의 미수정
제품 결함과 미실행 검사를 수정 완료 또는 새 ZIP 검증 완료로 해석하면 안 된다.

### 쿠크 입장 실패의 실제 원인

2026-09-29 23:20:16.844, Release Client PID13164의 필수 Effect 준비 결과는
`targets=233 prepared=231 failed=2`였다. 실패한 두 비행 이펙트의 element displayName이
Client의 **1~64 UTF-8 byte, 공백만 있는 이름 금지** 계약을 위반한다.

| authored effect | 실제 표시 이름 길이 | 판정 |
|---|---:|---|
| `effect.world.item.destruction_bomb.flight` | 67 bytes | 거절 |
| `effect.world.item.whirlwind_grenade.flight` | 69 bytes | 거절 |

정본은 `Data/Effects/Authored/<effectAssetId>.effect.json`이다.
`Tools/EffectPipeline/build_battle_item_effects.py`의 `project()`는 상위 문서 displayName만
교정하고 element displayName은 원본 projector의 긴 asset ID와 emitter 조합을 유지한다.
따라서 두 JSON의 표시명만 수동 축약하면 재생성 때 되돌아갈 수 있다. 후속 수정은 생성기와
생성 데이터 모두에서 표시명을 계약 안으로 제한해야 하며 stable effect/element/group ID,
재질·곡선·이동·크기·피해 값은 그대로 보존해야 한다. 이 세션은 해당 수정을 적용하지 않았다.

직접 증거는 다음 두 로그다.

- `Client/Default/EffectFailure.user.log`: 23:08:57에도 두 문서의 67/69바이트 거절이 기록되고,
  23:20:16 쿠크 필수 준비가 같은 원인으로 실패했다.
- `Client/Bin/Release/Diagnostics/client-session-13164.jsonl`: `CLIENT_LOAD_FAILED`,
  source `loading.target-resource-load`, HRESULT `0x80004005`를 기록한 뒤 Client가
  `connection.close-requested`를 남긴다. 직전 Server 수신은 8ms 전이고 WSA error는 0이다.

23:20:17.310 Lobby의 `lobby.recovery.presented`는 위 resource 실패를
`Server entry failed.`라는 공통 문구로 표시한다. 이 사례를 Server 접속 불가나 셰이더
컴파일 실패로 설명하면 원인을 오진한다. 후속 UI 수정은 실제 recovery reason에 맞는
짧은 제품 문구를 사용하고 상세 source/HRESULT는 기존 진단 로그에 보존하는 방향이다.

JSON parse, publisher, Product Build와 ZIP CRC/manifest 검사가 모두 통과해도 Client의
실제 authored Effect 의미 검증까지 실행한 것은 아니다. 이 표시명 규칙을 생성기 및
배포 전 검사에서 실제 Client 계약과 대조하는 회귀가 필요하다. 검증 규칙을 완화하거나
실패한 필수 Effect를 무시하는 방법은 제안하지 않는다.

### 실패 후 Lobby 캐릭터 표시와 종료의 별도 원인

같은 실행의 23:20:18에는 저장된 카드0의 model 준비가 성공한 뒤 Character Clone이
`E_FAIL`로 실패했다. read-only 소비자 조사는 다음 누락을 확인했다.

- `Loader::Ready_For_Lobby`는 static mesh shader만 등록한다. 카드의 비동기
  `CPlayableCharacterAssetService` 준비는 모델과 관련 문서를 commit하지만, Clone이
  요구하는 Character/Part_Body/Part_Equipment/Vehicle/Collider_Player 공통 원형과
  animated mesh shader는 다른 level의 `Ready_Character_Shared_Prototypes` 및
  `Ready_AnimatedMeshShader` 준비 경로에만 있다. Prototype_Manager는 현재 level에서만
  원형을 조회하며 다른 level 원형으로 자동 대체하지 않는다.
- Lobby는 `CPlayableCharacterAssetService::Begin_LevelLoad(LOBBY)`도 호출하지 않는다.
  실제 level 리소스가 해제돼도 ready-class/generation cache가 자동으로 초기화되지 않으므로,
  공통 원형만 등록하면 재진입 때 stale ready 상태로 인한 모델 Clone 실패가 남을 수 있다.

최소 수정 방향은 기존 rendering 준비에서 generation 초기화, animated shader와 가벼운
공통 원형 등록을 재사용하고 **실제 저장된 class의 모델만 필요할 때 준비**하는 것이다.
빈 로스터를 위해 4개 또는 6개 모델을 미리 로드하는 기존 문제를 다시 만들면 안 된다.
이 조사에서 해당 코드를 수정하거나 Client 화면 재현을 실행하지 않았다.

23:20:32의 `ClientExit.user.log`는 `reason=WM_QUIT hr=0x0`이다. resource 실패와 카드
Clone 실패의 조사한 경로에는 종료 호출이 없고 카드 실패는 해당 slot을 실패 상태로 격리한다.
종료 메시지에 의한 종료는 확인되지만 사용자 클릭, 외부 종료 요청 등 메시지의 발신자는
기존 로그만으로 특정할 수 없다. 접근 위반 crash나 사용자 자발 종료라고 단정하지 않는다.

### 실행한 Server·통신 검사

Client/UI와 사용자의 실행 프로세스는 조작하거나 종료하지 않았다. Server contract flag가
일반 listener 시작 이전에 분기하는 것을 확인하고, 검사군별 공통 격리 사본의 DataFiles와
별도 TEMP/TMP를 지정했다. numeric 검사는 내부에서 다시 복제한 사본만 변경했다. 원본 175개 수치/런타임 입력의
SHA256과 수정시각은 검사 전후 동일했다. 모든 테스트 자식 프로세스는 종료됐고 timeout
kill은 발생하지 않았다. Server simulation 결과를 실제 네 PC 플레이로 표현하지 않는다.

| 검사 | 실제 결과 | 범위/한계 |
|---|---|---|
| NetworkProtocolHarness 전체 Release | failures0 | 오래된 protocol122 기대값 11곳을 현재124로 수정 후 재빌드·재실행 |
| protocol character-restore | 8 PASS | packet round-trip/registry; Server 장비·재화 복원 실행은 아님 |
| battle-items | 56 PASS | 4관찰자 투척/접촉/소멸, 파괴 유효창, 갑옷 두 판, 중복·쿨타임·거리 거부 포함 |
| Valtan lifecycle | 92 PASS | 4세션 complete1, ticks20492, mechanics7, parity1, 유령 최종 처치/clear/MVP |
| NPC raid return | 26 PASS | 복귀·빈 방 정리 |
| character-admission | 71 PASS | 2/4인 초대·입장 투표·원자 이동·실패 복구 |
| Valtan pattern-control | 9 PASS | Release에 포함된 검사만 |
| numeric balance | 14 PASS | 비공대장 저장/전 room 적용/BUSY/stale/실패 rollback/재시작 복원 |
| Kouku raid | 1,747 PASS | 1~4인 관문 상태기계·준비·완료 경계 |
| Bingo | 51 PASS | 빙고/종료 계약 |
| Kouku object-overlap | 1,046 PASS | Release contact/geometry 계약 |
| card maze | 75 PASS | 카드 미로 계약 |
| Kouku dice-hit | 182 PASS | 피해 계약 |
| Showtime bomb | 25 PASS | 폭탄 판정 계약 |
| Kouku support surface | 50 PASS | Release 부분만; 상당 부분 Debug 조건부 |
| Kouku product/draft/world-playback | 34/20/34 PASS | 각 Release 실행 assertion 수 |
| Kouku bundle | 2 PASS | catalog 선행 검사만; 본체는 Debug 조건부로 미실행 |
| Valtan presentation | 0 assertions, exit0 | **검증 아님**: 구현 전체가 Debug 조건부 |
| Valtan arena-support | 48 PASS / 4 FAIL | 아래의 기존 4초 기대값 fixture 오류 |
| debug-teleport | 8,765 PASS / 17 FAIL | 아래의 오래된 Esther/Mario fixture 조건 |
| 전체 `--contract-test` | 1,436 PASS / 84 FAIL | 전체 green 아님; 쿠크 구간166 PASS 이후 광역 실패는 전부 분류 완료하지 않음 |

Valtan 연속 검사는 실제 네 CClientSession 입장과 명령·snapshot을 사용하되 생존과 HP 경계
진입을 위한 fixture 제어를 사용한다. Kouku raid 검사도 boss death와 경계 clock을 주입한다.
따라서 실제 네 Client가 모든 기믹을 연속 전투한 결과, 컷씬 화면이나 파편/갑옷의 GPU
표시 성공으로 대체할 수 없다. 서버 진행 계약과 화면 결과를 분리한다.

### 실패 분류와 남겨둔 변경

Bahuntur 제품 계약은 기존 커밋 `86001d82b`부터 착지 후 1,000ms에 보호를 부여한다.
`ServerGameplayContractTests_WorldDestruction.cpp`는 여전히 4,000ms와3.999/4.0초를
기대했다. 승인된 fixture 5줄만 1,000ms와0.999/1.0초 및 대응 설명으로 수정했다.
이 수정은 인코딩/줄바꿈 및 diff 검사를 통과했지만 **수정 후 빌드·재실행하지 않았다.**
그 전에 Server+Shared Debug build는 성공했으나 이 수정의 검증 근거가 될 수 없다.
이 세션에서 Debug contract 실행은 0회다.

debug-teleport의 17개 실패는 다음과 같이 분리했다. 원본을 바꾸지 않고 별도 사본에서
REUP windup만1367→600ms로, bounce row13개를 검사 사본에서만 제거한 통제 실험은
8,781 PASS/1 FAIL이었다. 이는 제품 수정 성공이 아니라 원인 분리 실험이다.

- 1개: gate reset이 공격/보호 Esther 소환물을 정리하도록 변경됐는데 fixture가 보존을 기대.
- 14개: 현재 REUP 저작 windup1367ms인데 fixture는30tick 약1초만 기다려 공격을 기대.
- 2개: HP100인 Mario4 경로 전용 fixture가 새1320 피해 공에 맞아49tick에 arena로 조기 복귀.
  공을 격리한 사본에서는160tick에 실제 출구와 clear 경계에 도달한다. 제품 공을 없앨 것이
  아니라 경로 검사의 생존 조건을 명시하고 별도 hazard 접촉 검사를 유지해야 한다.

numeric 첫 실행은 중첩 TEMP의 generation 경로가272자가 되어 복사 단계에서 실패했다.
더 짧은 격리 TEMP로 같은 검사를 다시 수행해14 PASS/0 FAIL,21.342초를 확인했다.
원본 게시 데이터를 고치거나 계약 검사를 생략한 것이 아니다.

Python/PowerShell 결과는 world collider27, result tuning10, Showtime bootstrap1,
combat hit14, card staging9, bomb resolution4, raid flow16, camera clearance3 PASS다.
최초 두 import 오류는 PYTHONPATH를 올바르게 지정한 재실행으로 해소됐다.
dice-card는1 PASS/2 FAIL, runtime-input은2 PASS/4 FAIL이다. per-card actor row로 바뀐
출력, 이동한 publisher 함수/의존성, 고정 revision/전체 문서 기대값이 fixture와 어긋난다.
이들 fixture를 이 세션에서 수정하지 않았다. 별도 parent sequence 검사는 실제114 patterns,
4parents와11개 invalid case를, card-maze warning 검사는36경로와5개 invalid case를 통과했다.

실행 로그와 명령/환경/시각/exit code는 아래 디렉터리에 있다.

- `out/ReleaseValidation20260929/protocol-*-final.log`, `protocol-final-result.json`
- `out/ReleaseValidation20260929/valtan-items/coverage-notes.md`, `results-20260929-231534.json`
- `out/ReleaseValidation20260929/kouku/validation-report.md`, `server-contract-results.json`,
  `full-contract-results.json`, `python-test-results.json`, `supplemental-test-results.json`

Server의 실제 `Handle_RestoreCharacter` 실행과 명시적 Kouku EXIT 투표를 호출하는 기존
gameplay fixture는 찾지 못했다. protocol 검사를 장비/재화 복원 검증으로 기록하지 않는다.
`Run_KoukuJokerAimContracts`는 정의/선언은 있지만 CLI dispatch와 호출자가 없어 실행되지 않는다.
Debug 전용 검사, 수정한 support fixture, 광역84 실패 분류, 실제 Client 로딩/캐릭터 전환,
네 PC 연속 전투·컷씬·음향·갑옷 파편 표시는 남은 검증이다.

## G11. Release 지연의 실측과 재발 방지 근거

### 시간을 소비한 실제 구간

| 항목 | 직전 Product | 통합 뒤 Product |
|---|---:|---:|
| 전체 | 19분33.339초 | 20분47.210초 |
| Client | 19분28.428초 | 19분24.863초 |
| FXC task | 18분23.945초 | 15분45.697초 |
| 기본 Mesh command→CSO 저장 | 18분23.711초 | 15분45.500초 |
| 기본 AnimMesh command→CSO 저장 | 이 항목에서 미계수 | 13분58.379초 |
| C++ 첫 CL→마지막 CL | 24.426초 | 3분0.929초 |
| Link | 38.464초 | 36.949초 |
| Client 변경 출력 | OBJ10 / CSO30 | OBJ288 / CSO36 |

최종 전체 시간의 대부분은 FXC였고 기본 Mesh가 마지막에 완료됐다. 개별 shader 값은
command 로그부터 저장 성공 로그까지의 벽시계 경과다. optimizer 순수 CPU 시간이나
함수별 비용은 측정하지 않았다. 병렬 shader들의 경과를 합해 전체 시간으로 쓰지 않는다.
최종 FXC 전체224개 중 실제 실행은36개였고 ZIP의256 CSO를 전부 재컴파일한 것이 아니다.

### checkout이 유효한 증분 상태를 무효화한 경로

최종 diagnostic은 `Shader_EffectArtistNativePrograms.hlsli`와
`Shader_SourceFoliageWind.hlsli`의 **22:42:19 수정시각**을 각각 AnimMesh/Mesh 재컴파일
이유로 명시한다. Git reflog의 통합 branch checkout도 같은 시각이다. 두 include는
원본 snapshot과 바이트가 같았다. 보존 자료의9개 shader 중4개는 바이트 동일,5개는
LF→CRLF checkout 차이만 있었다. 기존 무비 shader 작업이 사라진 것은 아니다.

따라서 branch라는 이름이 빌드를 요구한 것이 아니라, 안전 stash 후 다른 tree를 checkout하며
파일을 다시 쓰고 수정시각/일부 줄바꿈을 바꾼 것이 직접 재실행 원인이다. 기존 warm checkout의
유효 CSO와 tracking을 유지한 채 내용이 바뀐 입력만 통합하는 전략을 먼저 검증했어야 했다.
사용자의 “유효한 기존 CSO를 쓰고 C++만 증분 빌드해 ZIP”이라는 방향은 입력·도구체인·tracking
동일성이 확보되면 타당하다. 이번 작업은 그 조건을 먼저 보존하지 못했다.

두 Product의 toolchain trackingStateChanged는false다. VS18 Insiders MSBuild18.11,
VCTools14.44.35207, WindowsSDK10.0.26100.0, x64 도구가 사용됐다. 이번 재컴파일을
도구체인 변경 때문이라고 설명할 근거는 없다. 최종 빌드도 새 worktree가 아닌 Desktop의
기존 출력 경로를 사용했으므로 이 실행의15분45초를 새 worktree cold cache로 설명하지 않는다.
다만 새 worktree가 OBJ/PCH/CSO/tlog를 자동 공유하지 않는 것은 별도의 일반적인 위험이다.

최종 FXC `MaxProcessCount` 실측은20이고 C++ `ProcessorNumber`는4다.
repository props의 기본값4만 보고 FXC도4개씩 실행했다고 주장하면 안 된다. 실제 평가된
설정이 우선이며, 이번 자료로20-way가 빠른지 느린지나 실제 peak 동시성은 확정하지 않는다.

### Mesh/AnimMesh 구조와 최적화의 현재 상태

두 기본 파일은 vertex shader 하나가 아니라 `/T fx_5_0`로 여러 VS/PS·state·pass를 묶은
effect를 컴파일한다. 전처리 전 계수는 AnimMesh22pass/compile표현식21개, Mesh30pass/16개다.
이는 조건부/중복을 포함한 원시 계수이며 최종 고유 DXBC 개수라고 단정하지 않는다.

AnimMesh의 native model cue는 Artist/Vehicle/Lance/ALTV material 분기를 포함하고
scene texture 사용 때 큰 평가 함수를 mode0/1/2로 최대3회 호출한다. model용 dispatch는
6case지만 Artist group448/512/768/832/1600/1664 원시 합계42,187줄·2,511,697byte와
Vehicle4,856줄·271,299byte 선언을 포함한다. model-only에서 실제 필요하지 않은 함수까지
읽는 범위는 컴파일 최적화 후보이며 아직 절감 효과를 실측하지 않았다.

반대로 `ARTIST_NATIVE_MODEL_ONLY`는 이미 일반 Kouku/effect dispatch를 제외하고,
`SOURCE_CHARACTER_PROGRAM_GROUP=0`도 모든 캐릭터 group을 include하는 설정이 아니다.
전체 캐릭터 shader가 무차별 포함돼 느리다는 설명은 현재 코드에 맞지 않는다.

Mesh 기본group0에는 source-map-forward 프로그램33~37, foliage/stand wind와
water/alpha/sky/outline 등 entry가 함께 있다. 공통 include가 variant 여럿을 재빌드시키는
문제와 기본 Mesh 한 개의 긴 최적화 비용은 분리해서 분석해야 한다. 함수별 compiler
시간을 측정하지 않았으므로 특정 함수 하나를15분 병목으로 확정하지 않는다.

Anim/Mesh 둘 다 이미 shared VertexShader/PixelShader 변수를 여러pass에서 재사용한다.
같은 PS를 모든pass가 반복 compile한다는 진단은 틀리다. 후속 후보는 실제 callgraph의
include 범위 축소, 기존 group 경계 활용, 아직 남아 있는 동일 entry compile의 안전한 공유다.
public pass 순서/번호, reflection 변수·타입, input signature, material ABI와 scene-read/bloom
의미를 유지한 동일 toolchain Release 비교가 필요하다. `/Od`, 임의pass삭제, 강제FXCskip,
timestamp조작, tlog삭제나 입력이 다른 구CSO 재사용은 최적화가 아니다.
이 세션은 **shader 최적화를 구현하거나 새 shader를 컴파일하지 않았다.**
컴파일 시간 개선과 게임 GPU frame-time 개선도 서로 다른 측정 결과다.

### publisher와 완료 시간 예측

Client owner publish는394.002초였고 그중 Kouku product143.968초, map184.013초,
composition25.223초, navigation14.044초였다. Server owner129.014초 중 gameplay114.205초,
destruction6.300초, pickups5.129초였으며 일부 domain은reuse됐다. owner lock 대기는
22/41ms라 주요 병목이 아니다. 수치만 바뀐 작업에 전체 map/projector를 다시 게시하면
수분을 추가한다. 반대로 이번 통합에서 실제 바뀐 World schema와 아이템 등의 게시까지
전부 불필요했다고 말해서도 안 된다. 실제 소비자와 변경 domain으로 범위를 정해야 한다.

publish와 build 일부는 병렬이므로 각 값을 합쳐 사용자 전체 대기시간으로 쓰지 않는다.
입력/cache/publish 범위를 확인하기 전의3분 예측은 근거가 부족했다. 직전 Mesh만 이미
18분23초 걸렸으므로 FXC 재실행을 확인한 즉시 기존 수치를 기준으로 예상 시간을 정정해야 했다.
재발 방지 절차는 AGENTS, 반복 원인과 검사법은 gotchas에 반영하고 개별 실행 수치는 이 RESULT에 둔다.

원본 증거는 두 Product receipt `20260929T131955762Z-release-product.json`,
`20260929T140535348Z-release-product.json`, 최종
`out/ReleaseMerge20260929/release-build/20260929T134609498Z-Client-Release.log`/`.binlog`,
직전 `out/MovieFirstCutHold20260929/release-build/20260929T130025805Z-Client-Release.binlog`,
`out/ReleaseMerge20260929/shader-preservation.json` 및 publish-client/server.log다.
상세 read-only 조사 메모는 `out/ReleaseValidation20260929/build-analysis.md`다.
binlog reader는 해당 task/command/save 이벤트를 읽었지만 말미 embedded payload에서
로컬 System.Buffers assembly 해석 경고가 있었다. 전체 payload를 완전 해석했다고 주장하지 않는다.
