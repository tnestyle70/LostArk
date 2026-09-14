# 캐릭터 이펙트와 아레나 구현 결과

## G00. 이번 마무리 범위

사용자가 전체 요청의 진행 상황을 확인하고 우선 구현한 부분을 컴파일 오류 없이 마무리하도록 변경했다. 추가 Character Select 재질 구현은 중단하고 기존 제품 변경의 검증·빌드·실행 준비를 완료하는 범위다. 시작 시 있던 Character/MainApp/Loader/SequenceViewer와 쿠크·창술사 편집을 보존했다. 자동 stage/commit/push는 하지 않았다.

## G01. 캐릭터 local space

여섯 class prefix의 Authored224문서/5,012element를 조사했다. Artist37, DimensionMaster49, LanceMaster90, Warlord48문서이며 현재 Gunslinger/Slayer에는 같은 복원 형식의 Authored 문서가 없다. localSpace=true였던215문서의4,978개 particle flag만 false로 바꿨다. JSON 의미 비교로 나머지 field 보존을 확인했다. SourceRecipe/Reference/notify attachment는 변경하지 않았다.

현재 catalog admitted212문서 중210개는 실제 CEffectCatalog Stage, SaveAtomic/Load roundtrip과 Playback Seek를 통과했다. 합성 identity anchor에서 생성된 입자2,862개는 owner translation(+13,0,-7)을 바꿔도 World가 동일했다. 실제 모델 bone이나 화면 표시는 이 검사에 포함되지 않는다.

기존2050230 single-glass-canary와 water-burst2문서는 modules 누락으로 Stage 실패했다. 현재 skillbinding 참조는 없으며 이번 flag 변경이 원인은 아니다. 전체 Python Effect validator도 기존 Kouku v15의 runtimeCarriers 필수 처리와 C++ reader 허용 범위가 달라 실패했으므로 전체 validator PASS로 보고하지 않는다.

근거: out/CharacterEffectArena20260914/local_space_before.json, local_space_changes.json, native_result.json, native-final-console.log, native_source_receipt.json.

## G02. 차원술사 Alt V

Cube ScreenPost도 Requires_StartingSceneCapture 대상에 포함했다. Build_NativeScreenPost는 Stage에서 cinematic camera 전에 저장한 HDR/bloom pair를 첫 capture에 전달하며 destinationUV=(0.5,0.5)를 유지한다. 실제 target CModel 첫 pose projection은 끝 크기에 사용한다. 포탈 transition capture 경로는 유지한다.

변경2TUs의 Debug CL PASS. 실제 production seed/center source block과 CEffectNativeScreenPostMaterial·제품 shader를 사용하는4×4 합성 D3D11 WARP 검사67건 PASS다. 초기 pair, 첫 cinematic bind, 이후 카메라 변경에도 동결, pair 누락 시 부분 commit 방지를 포함한다. 근거: out/CharacterEffectArena20260914/CaptureProbe/result.json과 production-source-receipt.json.

## G03. 공통 맵 최적화와 베른 이동

MapStaticBatchObject는 placement transform에서 파생한 linear scale bound를 보관하고 Update_Instance 때 갱신한다. 매 카메라 cull의 반복 계산을 제거했다. shadow instance GPU buffer는 최초 실제 shadow upload 때 준비한다. 기존 Bern·Kouku·Character Select의 공통 경로에 적용된다.

변경C++ CL PASS, signed/sheared matrix10,000개와 update/invalid-preserve를 포함한10,003수치 검사 PASS. envelope/culling 계산 결과는 동일하다. 실제 FPS·GPU ms의 개선량은 측정하지 않았다. 맵 전체 재질·LOD·개별 구역에 대한 최적화가 모두 끝났다는 의미는 아니다.

베른은 실제 STAIR02D의 지상 높이를 잘못 아치 상단으로 잡은7셀을 수정하고, 과거 collision.bern.editor-proof 테스트 벽을 disabled로 바꿨다. 설치 runtime 비교에서 spawn 연결 cell이5,767→6,138로 늘고 남쪽NPC3명 접근이 가능해졌다. 기존 다른3ridge와 북쪽확장·1m step 정책은 유지한다. Navigation/World publisher PASS이며 마리오4의 구형 navigation 생성물도 같은 publisher로 동기화했다.

## G04. Character Select 보존한 미완료 작업

제품의 기존9 material binding/29 placementLighting은 변경하지 않았다. 현재 source803배치,55mesh,62material variant,78실제MIC/14terminal family와 static parameter를 재추출했다. RNM799배치/58texture와29environment override를 복원했고 기존29행의 lighting계수와 정확히 일치했다. masked3MIC의 실제 static-mesh Base/Light DXBC도 추출했다.

반복 UModel 추출의 P/N/UV0/indices81primitive, UV1 80primitive는 동일하지만 tangent40primitive와 COLOR041primitive는 달랐다. 이를 원본 채널로 설치하지 않았다. 나머지 재질·UV1모델·RNM/환경 variant는 제품 미적용이며 후속 작업이다.

재개 자료: out/CharacterEffectArena20260914/character-select-source/. full source closure, effective-material-parameters.json, baked-lighting-evidence.json, mesh-channel-repeatability.json, masked-native/를 사용한다. 당시 source799 RNM중별도vertex-lightmap1배치와무lightmap3배치를구분했다.

## G05. 관련 기능 결과

- 쿠크 작은 오망성/포탈 중단, 인형 box선택·복제, 실제 Saydon13clip·blend·Pattern단일행: 2026-09-14_KOUKU_SEQUENCE_PLAYBACK_EDITOR_IMPLEMENTATION_RESULT.md.
- 셰이더41program의 실제6group CSO/런타임 선택과증분비교: 2026-09-14_SHADER_PROGRAM_COMPILE_ISOLATION_IMPLEMENTATION_RESULT.md.
- Server Mario카운트·실제3패턴완료체인·테스트stage/seed: 2026-09-14_KOUKU_MARIO_SERVER_PROGRESSION_IMPLEMENTATION_RESULT.md.

## G06. 최종 검증과 사용자 실행

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`의 Engine→Shared→Server→Client
컴파일·링크·shader/SDK/DLL 배포 PASS. 전체164.692초, 컴파일/링크 오류0이며 C4819 등 경고는 남았다.
기록: [Debug Product receipt](../../../out/BuildPipeline/runs/20260914T011948879Z-debug-product.json).
최신 Server.exe `--contract-test`는1353개 PASS, failures0이다. 네 회차MarioIntro/중복입장거절과
새Bern남쪽계단·기존북쪽/성내계단·퇴역테스트벽제외를 실제published입력으로 확인했다.
최초구형Mario4navgrid실패시도와 최종성공을 구분한다. 최종로그는
out/CharacterEffectArena20260914/final-product-build.log 및 final-server-contract.log다.
JSON224개·프로젝트XML4개 parse, 캐릭터localSpace true잔여0개, git diff --check PASS.
Server/Client는 protocol83으로 함께 빌드·재시작해야 한다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았다. 작은오망성, 1관문19,389ms통과와61,662ms끝, 인형Duplicate, 이동후잔류particle, AltV화면중앙, 베른남쪽계단은 사용자 화면 확인이 남는다.

현재 마무리 시 Client와 Server는 모두 종료 상태다. server-host PC이며 사용자가 Server + Client profile로 함께 시작한다. F5/Ctrl+F5는 설정에 따라 Build를 수행하므로 무빌드 실행으로 안내하지 않는다.
