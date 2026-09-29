# Release 입장·무비 오디오·4인 검증 교정 결과

## G00. 확인된 원인과 반영

기준 branch는 codex/release-regression-20260929, HEAD는50dcd986d다. 기존 다른 세션의
문서·protocol fixture·Bahuntur fixture 변경을 보존했다. 실행 중 사용자 Server/Client를
종료하거나 Client/UI를 실행하지 않았다. 검증용 Server만 out 격리 복사본으로 실행했다.

실제23:20 실행은 Server가 Kouku 입장을 승인한 뒤 Client 필수 Effect233개 중2개를
로드하지 못했다. 두 비행 Effect의 element displayName17개가64-byte 상한을 넘었다.
생성기와 두 JSON의 표시 이름만 수정하고 stable ID·재질·수명·리소스는 보존했다.
ZIP 검사에 동일 UTF-8 표시 이름 검증을 추가했으며 수정 전 ZIP의 두 문서가 거부되는
것도 확인했다. 기존 Python v15 validator의 zero carrier/empty history 거부는 실제
C++와 맞췄고 orphan history 등 나머지 검사는 유지한다.

추가 실제 codec 검사에서 필수 발탄420628 문서가 기존 Character/SourceMaterials DDS를
거부했다. 기존 경로의 DDS만 admission하도록 최소 확장했다. 모델·절대경로·루트 탈출·
미존재 파일 검증을 유지하고 다른 해시의 동명 Effect texture로 치환하지 않았다.
Resources 파일이나 셰이더 변경은 없다.

Lobby 복귀의 saved-card clone 실패는 animated shader와 Character/part/collider 공통
prototype 및 per-level readiness reset 누락이었다. 기존 공통 준비 함수를 재사용하고
class 모델 지연 로드는 유지한다. 공통 준비 실패는 rollback한다. 로컬 리소스 실패
문구는 Game resources could not be loaded로 구분했다.23:20:32의 WM_QUIT hr0은
관찰했으나 요청 주체는 로그로 확정할 수 없으며 자동 crash 수정 완료로 주장하지 않는다.

## G01. Movie 오디오와 편집

카메라·애니메이션·Effect는 기존 source clock을 유지한다. Sound cue 시작만 source→Movie
시간으로 변환하고 WAV cursor/길이/drift/끝/tail은 감속 전 Movie 시간으로 계산한다.
pitch는 사용자 수동 배속과 instance 배속만 사용한다. Sound timeline 표시·MOVE·양쪽
trim도 동일 계약을 사용한다. 기존 사용자 sourceStart/duration/volume 저장본은 바꾸지 않았다.

최신 Release Product OBJ와 Engine DLL을 연결한 FMOD NOSOUND 검사488개가 통과했다.
5직업8개 설치 WAV, Product Update→Sample_Frame→실제 sound 처리, pause/seek/replay,
사용자0.5배속, drift, finish/tail/Clear, 일반 World,0.25배·2배 visual loop와 NEXT chain,
실제 Sound editor trim/MOVE를 포함한다. 로그는 out/MovieAudioClock20260929의
probe-test.log와 probe-validation.json이다. 실청·GPU 장면 판정은 포함하지 않는다.

## G02. 새 Release 검증

Product Release build와 배포 검사는 통과했다. 영수증은
out/BuildPipeline/runs/20260929T144547627Z-release-product.json이다.
Engine/Shared OBJ0, Server OBJ4, Client OBJ45이며 PCH/CSO 생성은0이다.
950개 shader 입력·CSO의 SHA256/bytes/mtime가 시작과 동일하다.
기존 C4819/DirectXTK PDB 경고는 남고 컴파일 오류는 없다.

| 새 Release 검사 | PASS | FAIL |
|---|---:|---:|
| Battle items +4인 restore/F1 지급/사용 |125|0|
| NPC return +4인 Kouku EXIT |34|0|
| Debug teleport/Mario support·move |8782|0|
| Valtan arena support |52|0|
| Kouku raid |1747|0|
| Valtan lifecycle |92|0|
| Character admission/party transfer |71|0|

restore는 실제4인 Bern admission과 serialized typed command/queued frame을 사용한다.
각자의 inventory/equipment/purse/title 및 빈 저장본, InventorySnapshot→APPLIED 순서,
중복 거부·잘못된 catalog/stack/class/title의 기존 상태 보존을 확인했다.
F1 grant와 네 아이템 사용은 각 session에 실행하고 투사체 flight/impact/despawn 및
파괴 가능한 두 GROGGY 창의 파츠 파괴를 검증했다. EXIT는 공대장/최신 proposal/거절
보존/마지막 동의 전 이동0/전원 동의시 각자의 귀환 상태/중복 회신을 확인했다.

Mario fixture의17실패는 live hazard와 경로 검사 분리, 실제 telegraph 시간 대기,
reset의 Esther 제거 기대값으로 교정했다. Bahuntur는 현재1000ms grant 계약에 맞췄다.
이 통과를 위해 제품 전투 수치·타이밍·위험물·렌더링 옵션을 바꾸지 않았다.

근거는 out/ReleaseEntryAudioRepair20260929/server-new-coverage/results.json과
out/r29fix/server-contract-results.json이다. 실제4Client LAN/GPU 플레이와 구분한다.
Valtan lifecycle은4session, cinematic8창/HP기믹7개/ghost복귀/권위스킬 kill/동일snapshot/
clear·MVP·reward를 실행하지만 보호 fixture와 임계 HP 입력을 사용한다.

## G03. 전체 검사의 한계

기존 out/ReleaseValidation20260929 검증의 전체 --contract-test는1436PASS/84FAIL이며
이번 focused 성공을 전체 검사 green으로 바꾸어 기록하지 않는다. 정확히 분류된
fixture drift와 나머지 미분류는 out/d29dbg/full-failure-classification.json에 분리한다.
이전 Python dice-card/runtime-input fixture도 현재 row/revision과 맞지 않는 실패가 남는다.
전수 raw Effect1486개의23거부 중 실제 필수 참조에서 확인된420628을 교정했다.
나머지 library/canary와 필수 runtime occurrence는 구분했다. full Python source validator는
현재 C++ cascadeBeamV1을 지원하지 않아 실패하므로 전체 validator PASS라고 하지 않는다.
이번 배포 검사에는 수정 표시 이름, 실제 required codec 및 package 참조·hash 검사를 사용한다.

## G04. Release F1 사용

F1 → Battle Items → Give all four (10 each)는 Debug/Release 공통이다.
I로 인벤토리를 열고 HUD1~4에 배치한 다음 F1을 닫고 숫자키로 사용한다.
파괴 폭탄·회오리 수류탄은 지면 목표, 성부는 적합한 파티원, 시정은 자신에게 적용한다.
Server가 인벤토리·생존·거리·대상·쿨다운을 확인한 뒤 수량을 소비한다.

배포 가이드는 Tools/ReleasePackaging/README_실행방법.md와 팀 Delivery guide에 반영했다.


## G05. 발탄 두 번째 갑옷의 추가 admission 교정

실제 codec는 첫 DDS 경로 문제를 해결한 뒤2520 native descriptor의 원본 MIC1 exact
identity 검사에서 MIC2를 다시 거부했다. 두 원본 MIC의 parent/base GUID와 engine-static
ShaderMap key가 같고 texture8/scalar28/vector13 입력 이름과 static35가 같다.
두 번째 MIC는 원본 texture와 emissive15(첫 번째10)를 유지해야 하므로 데이터를 첫 번째
재질로 바꾸지 않았다. Has_ArtistMaterialContract에 program2520+원래 MIC1 descriptor의
MIC2만 명시 허용했다. 나머지 carrier/render profile/texture/static/parameter 검사는 유지한다.

근거는 out/d29dbg/valtan-material-2520-equivalence.json과
out/ValtanArmor20260929/materials.json이다. 동일 key/ABI 확인이며 별도 DXBC 재추출이나
새 shader 생성은 하지 않았다. 최초 새 ZIP은 이 추가 거부 발견으로 최종 전달하지 않았고
최종 재빌드·실제 codec 통과 뒤 다시 교체한다.


## G06. 최종 실행 증거

최종 Product Release 영수증은
out/BuildPipeline/runs/20260929T145633928Z-release-product.json이다.
두 번째 증분 빌드는 Client OBJ34/PCH0/CSO0,52.792초였고 Server 바이너리는 변경되지 않았다.
시작950개 shader 입력·CSO의 내용·bytes·mtime가 끝까지 같다.

최신 Product OBJ를 다시 연결한 실제 codec 검사는 아이템6+갑옷2의8개 성공, 잘못된
MIC/program/texture/static/scalar/carrier6개 예상 거부, Resources경계12개 성공이다.
쿠크 Loader 소비자를 재구성한 전체7class 합집합335개도 전부 성공했다. 실제 오류 당시
GuardianKnight의233개와 일치하는 목록도 따로 저장했다.420628 원문은 바꾸지 않았다.
근거는 out/ReleaseEntryAudioRepair20260929/effect-entry/native/final-results.json이다.

Release 네트워크 없는4session 서버 검사7종10,903 assertions와 FMOD NOSOUND488검사를
통과했다. 서버 격리 입력 파일·EXE의 해시는 실행 전후 같다.
별도23:19 기존 Debug EXE 복사본으로 Release에서 생략되는 검사6종도 실행했다.
bundle128/product336/draft28/support343/object-overlap1087/Valtan-presentation55,
총1977PASS/0FAIL이며 입력 파일은 불변이다. 최종 Release와 동일 EXE라고 설명하지 않는다.
근거는 out/d29dbg/results.json이다.

Effect validator 단위51개와 packaging 단위20개, 변경JSON2개·project/filter XML4개
parse 및 git diff --check가 통과했다. 광역84실패의 player skill 관련 추가 분류는
out/MovieAudioClock20260929/full-suite-skill-audit.md/json을 따른다.23개 중21개에는
기대값 불일치 근거가 있고 collider miss2개는 난수·조기 사망을 분리한 재현이 필요하다.
Retail 기상기30초는 명시적 profile override로 bootstrap과 일치하여 재게시하지 않았다.
광역 전체를 재실행·교정한 것은 아니므로 전체PASS는 주장하지 않는다.


## G07. 최종 ZIP 교체 완료

2026-09-30 00:00 KST에 마지막 재질 교정까지 포함한 새 ZIP으로 원자적 교체를 완료했다.

- 위치: `C:/Users/user/Desktop/LostArk-Release-20260929.zip`
- 크기: 166,876,258bytes
- SHA256: `115a26384dfd2bb08972dc1b7a0e790ac01d2c555a7503b5fbca4f5de5ca84c0`
- 최종 stage: `out/ReleasePackaging/20260929-entry-audio-validated`
- protocol124, sourceRevision2469, sequenceRevision183
- payload2757개, DirectData2136개, CSO256개, numeric source576필드
- archive entry2786개, manifest검증2783개

Builder의 전체 CRC·manifest SHA256/size와 nonlaunch preflight가 통과했다. 최종 ZIP의
Client/Server/Engine 및 두 수정 Effect를 현재 설치본과 다시 해시 대조했다. Resources는
기존 외부 폴더를 사용하며 ZIP에 포함하지 않는다. 기존 ZIP은
`LostArk-Release-20260929.backup-20260930-000006-620614.zip`로 백업했다.

영수증은 out/ReleasePackaging/portable-delivery.receipt.json, 최종 대조는
out/ReleaseEntryAudioRepair20260929/final-delivery-check.json이다.
새 ZIP 생성이 실제4Client LAN·GPU·실청 확인을 대신하지 않는다.

광역84실패의 최종 병합 분류는 fixture 기대값 불일치38/미확정46이다. 이들 전체의
수정·재실행 완료나 모든 제품 기능 정상은 주장하지 않는다. 이번에 확인된 입장·재질·
오디오 오류 수정, 위 scoped 검사와 ZIP 생성 완료를 구분한다.

## G08. 09-30 Debug Balance Test 조회 불가 — G05 후속

사용자 화면의 HP/tick은 정상 갱신되지만 수치 목록은 비어 있었다. PID51196을
ReadProcessMemory와 일치하는 PDB로 읽기 전용 확인했다. 접속과 receive worker는 정상이고
오류 코드는 0이지만, 수치 revision과 entries는 비어 있으며 page는 0/0이었다. query 번호는
초당 약 42회 증가하고 pending=0/refresh=true여서 서버 응답 대기보다 요청 송신 이전 실패로
좁혔다. debugger attach, process suspend/write, UI 실행·조작은 하지 않았다.

실제 `Shared/Bin/Debug/Shared.lib`를 링크한 검사에서 `C2S_BALANCE_QUERY`(107)의
Write_Message는 8-byte payload를 만들지만 Build_Packet_Frame은 false였다. 오래된
PacketFrame/PacketStreamParser 객체가 신규 106~112 packet을 거부했다. 같은 Release archive는
프레임 생성과 header 읽기를 모두 통과했다. 현행 소스에서 추출한 paging 함수의 4개 검사는
별도로 통과했다. 현행 소스 검사만으로 설치된 library가 같다고 판단할 수 없는 사례다.

Debug의 CL.read 추적에서 두 객체의 입력은 Shared.pch만 남아 있었고 PacketType.h가 빠져
있었다. 정상 Shared Build도 최신 상태라고 오판하는 것을 진단 로그로 확인했다. 추적 입력이
언제·어떤 경로로 유실됐는지는 확정하지 않았다. 표준 toolchain/경로에서 Shared ClCompile로
8개 CPP와 PCH를 다시 컴파일한 뒤 정상 Build로 archive를 연결했다. Clean/Rebuild, 중간 파일
삭제, 수동 tlog 편집, source timestamp 조작, shader 재컴파일은 하지 않았다. 새 compiler 추적에
PacketType.h가 포함됐고, 교정 Debug와 기존 Release 모두 106~112 frame/header 검사가 통과했다.

증거는 `out/BalanceSnapshot20260930/`의 `live-state.json`, `live-error-state.json`,
`before-comparison.json`, `after-comparison.json`, `shared-debug-build.log`와 3개 binlog다.
실패한 기존 archive/객체는 `before/`에 해시와 함께 보존했다. 무력화/패턴/리소스/ZIP은
변경하지 않았다. 현재 Client PID51196과 Server PID12212의 EXE 쓰기 잠금을 확인했으므로
사용자 종료 확인 후 최종 Debug EXE 연결과 F1 사용자 확인이 남아 있다. 라이브러리 복구를
실행 중 EXE 반영이나 수치 UI 최종 성공으로 기록하지 않는다.
