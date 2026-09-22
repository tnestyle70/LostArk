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

새 Resources 파일은 0개다. 설치 리소스162개를 재사용하며 `GRResources2`에 추가할 이미지·모델·음원은
없다. 새 이펙트 정의3개는 `Data/Effects/Authored`, Catalog, ResourceTree와 프로젝트에 등록했다.

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
