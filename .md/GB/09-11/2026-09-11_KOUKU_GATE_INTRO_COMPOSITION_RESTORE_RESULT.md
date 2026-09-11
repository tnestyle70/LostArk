# 쿠크 진입 컷신 Composition 복구 결과

## G01. 구현한 재생 경로

사용자 Sequence `KAKULSAYDON_G1_PATTERN_3 / 2관문_진입컷씬`을 원본 SCENE04A 27초에 연결한다.
새 actor animation은 원본 A/B blend·reverse·preroll을 단일 WANM으로 구워 기존 CModel과
WorldSequencePlayer가 샘플한다. 모델과 새 카드분출은 Resources 상대 ID를 쓰며 실제 파일은
`Client/Bin/Resources/Map/KakulSaydon/Gate2Intro`에 설치한다. Resources는 Git 대상이 아니다.

World Object는 `materialSourceModelAssetId`로 기존 ActorCatalog 승인 재질을 소비하거나
`mapMaterialBindings`로 Area의 실제 BG8 slot 정의를 가져온다. 정적 RNM/static shadow는
제거하고, animated shader가 기존 공유 MapMaterialSurface를 평가해 같은 Deferred GBuffer에
쓴다. 표면 발광 시간은 WorldSequence localMs와 함께 이동한다.

쿠크 원본 광원 119개 중 SOURCE_CHARACTER 84개를 UNBAKED receiver로 바꿨다. 광원 위치·색·
세기·감쇠 값은 유지했다. 움직이는 맵은 이 광원을 받고 RNM이 있는 정적 map pixel은 제외된다.
기존 팝업북도 동일 BG8 재질이 있는 생성형 World Object로 옮기며 Preview 중 legacy Deploy7을
숨긴다. Stop은 적용했던 상태가 유지될 때만 기존 상태를 복구한다.

원본 l1/l2/l3은 기존 V1 typed point light 34개 구간으로 투영했다. RGB는 원본 Hermite와
동등한 cubic distribution이며 이동 위치만 0.5mm 이하 오차로 선형 분할했다. 시간 구간은
runtime float32 경계를 공유해 인접 광원이 동시에 켜지는 문제를 방지한다.

카메라·암전은 [카메라 결과](2026-09-11_COMPOSITION_CAMERA_RESULT.md), 카드미로의 Server
trigger와 Protocol79는 [카드미로 결과](2026-09-11_KOUKU_CARD_MAZE_SEQUENCE_TRIGGERS_RESULT.md)를 따른다.

## G02. 실행한 검증

- Debug Product Engine → Shared → Server → Client 빌드·일반 runtime 배포 성공.
  `out/BuildPipeline/runs/20260911T075016765Z-debug-product.json`, `out/kouku-intro-product-build.log`.
- 기존 PointLightFalloffContractHarness Debug 빌드와 실행 PASS. WARP 1×1 수치 readback으로
  ALL/SOURCE_CHARACTER/UNBAKED × RNM 유무 6개 조합을 검사했다.
  `out/kouku-intro-light-contract.log`. 이것은 Client 화면 캡처나 visual PASS가 아니다.
- 기존 하네스의 누락된 Rect buffer binding을 실제 Renderer 순서에 맞췄다. LIGHT_DESC의 현재
  크기108과 유효 innerCos=1 계약에 맞춰 오래된 테스트 가정도 교정했다.
- UNBAKED light JSON roundtrip·unknown receiver 거부 Python 검사 PASS.
- 실제 WorldSequenceDocument/DataJson C++ 검사 8개 PASS: 새 field 저장·재로드·동등성,
  잘못된 경로·중복 slot·stable ID 거부, Load/Save 실패 시 기존 상태/파일 보존.
  `out/WorldSequenceMaterial20260911/probe.run.log`.
- 실제 최신 Product Codec/Playback의 645개 Seek 표본을 원본 Matinee와 직접 대조했다.
  최대 동시 광원3, 위치 오차0.000435114m, RGB×intensity 오차0.00000190735,
  range/ID 일치 및27초 종료 정리 PASS. `out/KoukuIntroLights20260911/cpu_probe_run.log`.
- 최신 Server의 `--card-maze-contract-test` failures0. 마지막 collision/busy participant 검사까지
  포함한다. `out/kouku-intro-cardmaze-contract.log`.
- 초기 핵심 WorldSequence 데이터의 Map publisher Validate PASS.
  `out/kouku-intro-map-validate.log`. 추가 촬영 소품의 최종 배포 검증은 다음 항목에서 분리 기록한다.

## G03. 최종 데이터 배포 체크포인트

핵심 모델·카메라·카드·조명 생성 이후 발견한 부모에 부착된 맵 소품165개와 추가 FX의
연결을 통합 중이다. 최종 publisher 실행 전이므로 이 체크포인트는 전체 runtime 배포 완료를 뜻하지 않는다.

## G04. 남은 표현 경계와 사용자 확인

V1 point-light carrier는 UE3의 개별 lighting channel mask와 dynamic shadow map을 표현하지
않는다. 원본에 명시 falloff가 없는 광원은 기존 프로젝트의 inferred exponent2를 사용한다.
카드의 source FX MIC 계산과 카메라 offset, 원본 DOF/audio 등은 전체 source 목록과 실제
연결 결과를 구분해야 하며, 모델/track 개수 성공으로 원본 pixel 일치를 주장하지 않는다.

Client/UI 실행·조작·화면 캡처와 visual PASS는 수행하지 않았다. 세션 시작 LAN 상태는
server-host, TCP7777 LocalSubnet 준비 완료, endpoint192.168.0.14:7777 not-listening이었다.
최종 데이터 배포 후 사용자가 Visual Studio `Server + Client` profile에서 Ctrl+F5로 실행한다.
Lobby → KoukuSaydon → F1 → Open Sequencer Benchmark에서 `2관문_진입컷씬`과 `연출_팝업북`을
선택해 Play한다. 카드미로 서버 trigger는 Composition Patterns의 `쿠크_카드미로연출`을
`Complete Play`로 확인한다.
