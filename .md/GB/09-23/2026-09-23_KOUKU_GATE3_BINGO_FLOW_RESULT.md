# 쿠크 3관문·빙고 순서와 Release 배포 결과

## 반영 범위

`codex/kouku-gate3-bingo-flow-0923`에서 사용자가 마지막 저장한 revision2221을 보존하여
revision2222로 반영했다. 교체 직전 hash를 재확인했고 원본은
`out/KoukuGate3Bingo20260923/before-install`에 백업했다. GATE1/GATE2 Flow와 별도 P62/P123을
보존하고 사용자 지정 GATE3 순서, 4개 Mario Parent 전투 자식, P96 최초24개·반복14개를 연결했다.

Parent의 선택적 `playChildrenSequentially`와 `loopStartPatternOccurrenceId`는 Client codec,
projector, Gameplay의 `PATTERNPARENTCHILD`, Server catalog와 기존 audition member scheduler가
함께 소비한다. 자식의 실제 완료·이펙트 tail·카운터 후속을 기다리고, Mario 입장/복귀와 기존
P33 후속을 유지한다. 빙고 보드는 같은 Parent 소유권으로 반복하며 종료 시 기존 정리 경로를 따른다.

P107은 원본 action4219927의 clip을 복원했다. 사용자 logic138은 `(2.9,0,1143)`으로 이동하여
앵콜세이튼 시작 위치 `(-0.600000024,0,1147.43994)`를 바라본다. boss24개 element와 중앙 MAP40개
element를 분리하고 기존 native material/carrier를 사용한다. P94는 현재 클립 action42198102의
19개 element를 복원하고 3167~3667ms의 gaze 실패에 기존 파1빨2 FEAR logic37을 연결했다.
두 패턴의 원본 사운드를 연결하고 다른 패턴에는 기존 사용자 사운드를 보존하며 누락27개만 추가했다.
P119/P122/P114/P120의 필요한 MAP 배치, 바람방구 방향, 추적의 중복 root XZ 이동을 보정했다.

새 이펙트 정의3개 자체는 설치 리소스162개를 재사용하며 `Data/Effects/Authored`, Catalog,
ResourceTree와 프로젝트에 등록했다. 이 제한된 검사만으로 전체 배포의 추가 Resources를0개라고
판단한 것은 잘못이었다. 배포 후 GBResources 감사에서 이전 작업의 전달 누락31개를 확인했다.

## 게시와 검증

Kouku projector, Gameplay publisher, Composition publisher를 실행했다. Gameplay의 쿠크 외 행은
전체 행 수와 Valtan presentation generation hash를 제외하고 기존 저장본과 동일하다.
실행 중인 사용자 Client/Server는 종료·Reload하지 않았다. 파일 게시와 실행 중 메모리는 구분한다.

- Engine·Shared·Server·Client 전체 Release 증분 빌드 성공. 증거와 최종 바이너리 hash는 `final-build-evidence.json`, `release-ready.receipt.json`에 기록했다.
- Release `--kouku-raid-contract-test`: 904개 assertion PASS. 1~4인 준비·시퀀스·관문·빙고 종료, 실제 빙고 자식52개 재생과 보드 유지, 공포·카운터 포함.
- Release `--bingo-contract-test`: 45개 PASS. 실제 Release Client 문서 codec: 22개 PASS.
- 변경 Server/Client TU의 Release 컴파일과 공간 처리26/65개 검증 PASS.
- 실제 Catalog stage/Playback 복원3개 Debug/Release 수치 검증: 각각48,411개 PASS. 최종 Release는 갱신된 Client object와 Engine DLL로 다시 링크했으며 GPU 화면 확인과 동일하지 않다.
- 변경 JSON/XML parse와 `git diff --check` PASS.

기존 raid 통합 테스트의 Debug 전용 본문과 예전50초 Parent 가정을 수정했다. Release fixture는
제품의 entry admission 소비자를 사용하며 제품의 Debug START 거절을 해제하지 않는다.
10초 준비 deadline 이전의 timer-first 경쟁과 pending mechanic trigger commit을 포함한다.

전체 Release에서 기존 `ValtanBossTool`이 Debug 전용 boss 준비 함수를 무조건 호출하던 컴파일
오류를 확인했다. 해당 편집 도구의 Submit/CanPlay/Preparation 소비자만 Debug 조건으로 제한하여
Release 제품 레이드 경로는 유지했다. 이미 완료한 셰이더는 다시 컴파일하지 않고 증분 빌드했다.

## 배포

사용자가 지정한 `LostArk-Release-20260919-Full-v2-repacked.zip`의 폴더 선택형 설치·실행 구조를
기준으로 `LostArk.exe`, `ChangedData/Data`, `Runtime/LostArk-Release-Runtime.zip`을 한 최종 ZIP에
넣었다. endpoint는 `192.168.0.22:7777`, protocol105다. 기존19일 ZIP 폴더는 읽기만 했다.

중간 Data 복사 중 디스크 부족을 확인하여 추가 디스크를 쓰지 않는 작업 폴더 hard link와 직접
압축으로 바꿨다. 임시 복사본 삭제는 도구 정책에 차단되어 `out/KoukuGate3Bingo20260923/FinalPackage`
아래에 남아 있다. 최종 패키지는 별도 `FinalPackageLinked`에서 만들며 원본 Data를 수정하지 않는다.
Data 사전 압축은90,294,580bytes다. 실행 파일을 포함한 최종 크기·SHA256·설치기 검증은
`out/KoukuGate3Bingo20260923/final-delivery.json`에 기록한다.

최종 파일은 `C:/Users/user/Desktop/LostArk-Release-20260923.zip` 하나다.
127,834,801bytes, SHA256 `ab7716ef6cd0b7b02c9d2e59b85ddeb3770380d2adbff178cc27771fb9784511`.
Data1914개와 runtime487개를 포함한다. 바깥 ZIP과 안쪽 runtime ZIP의 CRC 및 모든 manifest SHA256을
검증했고 실제 LostArk.exe `--check` 설치기 사전 검사도 PASS다. 사전 검사는 파일·백업·바로가기를
설치하거나 Client를 실행하지 않았다.

Client 화면과 다른 PC의 실제 접속·실청은 사용자가 직접 확인한다. 자동 검증으로 해당 확인을 대체하지 않는다.

## 배포 후 쿠크 입장과 Resources 보완

호스트의 Release 쿠크 로딩은 V1 214개·V2 33개·World339개 준비에 성공했다. 다른 세 PC는
쿠크 월드에서 클라이언트 쪽 연결 종료 뒤 Lobby로 복귀했다. 상대 상세 로그가 없어 첫 실패 파일은
확정하지 않았다. `Server entry failed`는 리소스 로딩 복구에서도 표시되는 공통 문구다.

기존 GBResources4,065개는 현재 설치본과 전부 SHA256이 같지만,9월19일 이후 갱신된 쿠크 참조
리소스31개가 추가팩에 없었다. 사용자 최종 지정 `C:/Users/user/Desktop/GBResources2`에
모델4개·Effect18개·Sound9개, 합계407,790,972bytes를 복사하고 전체 SHA256을 확인했다.
오늘 생성한 물리 리소스31개가 아니라 오늘 배포에 반영된 이전 작업의 누락분이다.
기존 ZIP과 GBResources, 실행 중 게임 파일·프로세스는 변경하지 않았다.

전체 목록·오늘 작업 정리·publisher 차이와 미확인 경계는
[Resources 감사](2026-09-23_KOUKU_RESOURCE_DELIVERY_AUDIT.md)에 기록한다.

## G06. 9월23일 추가 수정: 5초 전환·Release UI·입구 재시도

이 절은 위의 이전 배포 증거와 구분한다. 현재 추가 코드는 `GameRoom_KoukuRaidFlow.cpp`의 G3 false-clear 대기를 5000ms/150tick로 변경한다. 준비 완료된 정식 raid는 deadline의 같은 tick에 Bingo intro를 시작하며 별도 +1tick을 추가하지 않는다. 단독 G3 kill의 리소스 준비도 원래 clear tick을 보존한다. READY가 늦으면 미준비 리소스로 시퀀스를 시작하지 않는다.

비-raid의 Release 관문 UI는 기존 `Advance_Gate(4)`가 보스 생성·이동만 하여 Parent owner가 없던 경로였다. 현재는 기존 만장일치 투표가 확인된 경우에만 `Begin_KoukuRaidPreparation`의 `bBingoGateVoteEntry`로 들어간다. 저장된 Action/Sequence pin과 참가자 READY가 완료되면 Bingo intro, 연출 완료 시 `Enter_KoukuRaidCombat`과 첫 Flow/Parent 실행을 사용한다. 외부 Debug START의 Release 거절은 유지한다.

G1 입구의 재현 가능한 별도 결함도 수정했다. 한 참가자가 `1Stage_Final`에 들어오고 동료가 G 이동 중이면 준비가 거절되는데, 기존 `Evaluate_Entries`가 실패한 PLAY_SEQUENCE의 PlayersInside를 기록하여 다음 시도까지 막았다. 쿠크의 실패한 sequence entry는 접촉 edge를 소비하지 않고 같은 volume에서 재시도한다. 임시 player-state 검사는 무거운 Product preflight 전에 실행하며 실패로 raid epoch를 소비하지 않는다. 입구의 live trigger/데이터는 교체하지 않았다.

기존 `ServerGameplayContractTests_KoukuRaid.cpp`에 실제 설치된 입구를 이용하는 2·3·4인 이동 중 거절→그 자리 재시도, 부분 READY 대기, 같은 cinematic epoch와 2·3·4인 Bingo UI의 부분 투표 대기를 추가했다. 4인 Bingo UI는 실제 scheduler로 Parent prefix와 반복 tail을 돌며 같은 epoch·보드 유지까지 검사한다. 기존 1~4인 정식 raid·무한 Parent 검증과 deadline 직전/같은 tick 검증도 유지한다.

추가 소스의 `git diff --check`는 PASS다. 통합 담당이 새로 링크한 `Server/Bin/Release/Server.exe`로 `--kouku-raid-contract-test`를 직접 실행하여 **974 PASS, failures 0, exit 0**을 확인했다(약9초). 위 2·3·4인 입구/READY, 4인 UI Bingo 반복, 1~4인 5초 전환이 실제 새 테스트 로그에서 PASS다. 증거는 `out/KoukuCollider20260923/bugfix-bgm/release-raid-contract.log`와 `validation-receipt.json`이다. 실행 당시 게시본과 EXE SHA256을 고정했으며 이후 최종 데이터 publish/ZIP 검증은 통합 기록을 따른다. Client/UI와 실제 4PC/4Client 화면은 실행하지 않았다.
