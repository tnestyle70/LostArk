# 발탄 재질·패턴·Composition 연결 복구 구현 계획

작성일: 2026-09-11. 시작 기준: `codex/kouku-full-material-lighting-restoration`, `e26cd2b282293bd4cb433338bdbc6a7315dd3f84`와 기존 미커밋 쿠크·베른 복원 변경.

사용자 요청은 발탄 본체와 장비 재질, 서로 다르게 보이는 돌바닥, 중앙 idle 고정, 기존 패턴의 Composition 편집, 독립 도넛과 V2 Effect·Sound·Camera 연결 복구다. 첨부 화면은 관찰 자료이며 시각 PASS가 아니다. 기존 변경을 보존하고 별도 runtime, 보스 복제, Client/UI 자동 실행 없이 현재 소비 경계를 수정한다.

## G00. 기존 저작 내용과 실제 실행 입력

`Valtan.gameplay.json`과 `Valtan.presentation.json`에는 관리 패턴 42개·Stage 194개가 있다. legacy 원본 목록은 25개지만 관리 패턴과 겹치는 항목을 합친 실제 `ValtanEncounter.json`은 65개(관리 42개와 compatibility 23개)다. UI는 실제 Product 목록을 소비하고 새 Pattern ID로 복제하지 않는다. `Valtan.combatobjects.json`은 독립 오브젝트 9개를 소유한다. 도넛 두 종류는 animation 없는 foreground가 독립 Server object를 생성하며, 이후 발탄 움직임과 별도로 생성 위치에 남는다.

현재 V2 binding은 102개, pattern Sound는 726개, combat-object Sound는 9개, Shake는 128개, cinematic Camera는 11개다. 실제 사용 V2 closure는 leaf 59개·group 12개, 물리 리소스 66개이며 누락 파일은 없다. Sound 133 event 중 132개는 WAV 251개에 연결돼 있고 파일 헤더가 정상이다. `G_Voltan1_Attack13_Loop1` 하나는 이전부터 빈 catalog 항목이며 현재 추출 Sound 원본에도 대응 파일이 없다. 이를 다른 소리로 임의 치환하지 않는다. BGM M05~M09의 실제 WAV 5개도 존재한다.

실제 Python Valtan source join과 presentation generation Validate는 통과한다. 그러나 기존 C++ presentation admission harness는 `BossCatalog Effect V2 owner header is invalid`를 재현한다. 재질 override가 추가된 현행 catalog root를 전용 reader만 거부한다. 이 실패가 `CClientReplication`의 `Apply_NetworkState` 호출을 막아 화면의 위치·idle 상태가 남을 수 있다.

## G01. 본체·장비·바닥의 실제 material 소비

재질 변경 범위와 원본 MIC별 근거는 같은 날짜의 발탄 material 전용 PLAN/RESULT가 소유한다. `CValtanPresentationAssetService`의 plain-path `CModel::Create`를 기존 `CActorCatalog::Build_ModelLoadDescription`과 연결해 본체·장비·유령에도 catalog override가 실제 적용되게 한다. 기존 source shader program은 보존하고 원본 근거가 있는 신규 program만 추가한다.

바닥은 중앙 정적 mesh와 Deploy floor가 서로 다른 입력 경로를 사용한다. 기존 09-08의 floor/rock 복구와 현재 physical material·placement를 대조해 차이를 분리한다. 두 종류를 같은 색으로 덮거나 밝기를 전체 증폭해 일치시켰다고 처리하지 않는다.

중앙 circlefloor 두 slot의 level MIC가 현재 잘못된 rain MIC와 다른 것을 확인했다. 원본 Base/Direct/Baked/VS가 기존 overlay adapter와 일치하므로 원본 위치의 UV1·vertex color와 RNM을 유지하는 기존 variant 경로로 수정한다. 작업 중 외부 `out` 정리로 이전 geometry helper와 검증 로그가 소실된 항목은 원천 자료에서 재구성하고 재검증하며 이전 실행 기록과 남아 있는 증거를 구분한다.

## G02. 실제 발탄의 presentation admission 복구

`ValtanPresentationGenerationAdmission.cpp`에서 현행 BossCatalog의 optional `modelMaterialOverrides` array를 승인한다. unknown root field·잘못된 자료형과 기존 Effect owner 검사는 계속 거부한다. 기존 admission contract test에 현행 catalog 성공과 invalid 입력의 실패 보존을 추가한다.

최종 catalog 변경 후 기존 Gameplay publisher로 Server bootstrap과 presentation generation을 함께 갱신한다. Client/Server의 pinned generation 일치와 실제 native reader 성공을 확인한다. 파일 검증 통과와 사용자 아레나 재생은 별개로 기록한다.

local Preview clone도 Server Play 전환 뒤 화면에 남는 것을 확인했다. `Debug_CompletePlaySelected`의 submit 성공 뒤 Animation Tool이 해당 발탄 preview를 정식 Release하고 auto-stage를 억제하도록 연결한다. 실패 시 기존 preview, 다른 asset preview, dirty 문서는 보존한다. 명시적 local staging과 Level 변경에서 자동 생성 억제를 해제한다.

## G03. 기존 Composition에서 Pattern·Effect·World owner 편집

선행 [09-09 Composition 계획](../09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_IMPLEMENTATION_PLAN.md)은 당시 PLAN-only다. 이 복구에서는 실제 관찰된 편집 결함과 소비자 연결을 구현하며 그 계획 전체를 완료로 표시하지 않는다.

`CValtanActionWorkbench`는 공용 `CSequencerTool` 안의 기존 발탄 session을 유지한다. V2 선택은 합성 resource/start ID 대신 typed `bindingId`를 사용한다. `CLIP_OCCURRENCE`의 source clock을 정확한 occurrence의 Stage 위치로 투영한다. 특히 `VALTAN_BIND_SLOT`의 shout 3개를 서로 다른 1400/2300/3200ms 위치로 보존한다. 기존 V2 Catalog mutation·검증·저장 경로에 anchor/follow/rotation/TRS/repeat/stop 편집을 연결한다.

기존 독립 combat object 9개를 World lane/리소스 목록에 표시하고 소유 Pattern/Stage로 선택·재생·기존 typed 수치 편집을 연결한다. Server object의 위치·hit 권위는 그대로다. 도넛과 돌의 기존 V1 표현을 임의의 V2 그룹으로 교체하지 않는다. 특히 돌의 V2 override는 09-04에 사용자 편집 V1을 보존하려고 의도적으로 해제됐으며 기존 V2는 독립 리소스로 남아 있다.

## G04. 통합 검증과 실행 인계

에이전트별 shader·C++ 변경이 끝나면 같은 workspace에서 빌드를 한 번에 하나만 실행한다. 기존 Debug Product runner로 Engine → Shared → Server → Client와 runtime 배포를 확인한다. 이번 기능의 native admission·V2 binding 검사를 사용하며 무관한 광역 하네스를 완료 조건으로 붙이지 않는다.

변경 JSON/XML parse, `git diff --check`, source/published generation 일치, 실제 Valtan model/material 준비와 리소스 의존성을 확인한다. 생성 파일은 정본 publisher가 만들며 Resources는 Git에 추가하지 않는다. 다수의 기존 dirty 변경이 있으므로 자동 stage/commit하지 않는다.

실행 준비 뒤 사용자는 Visual Studio `Server + Client` profile을 `Ctrl+F5`로 시작하고 Lobby → Valtan → F1 → Action Workbench → Boss Valtan에서 확인한다. Pattern의 Sound/Camera/V2/World 상자와 실제 보스 재생을 확인할 정확한 버튼은 최종 RESULT에 실제 구현 기준으로 기록한다. 사용자 관찰 전에는 외형·애니메이션·최종 연출의 visual PASS를 기록하지 않는다.

## G05. 통합 publisher가 발견한 맵 조명 계약 불일치

Gameplay Publish는 발탄 검증 뒤 쿠크 Composition도 검사한다. 현재 복구된 쿠크 map light는 정상 map owner의 512개 제한을 통과하지만 `_join_light_resources`가 이전 64개 제한을 중복 적용해 전체 배포가 중단됐다. 이 한 곳은 이미 수행한 `validate_map_lights_v2`의 결과를 소비하도록 고친다. 기존 Python 테스트에 512개 성공·513개 거부를 추가하며 원본 조명 항목이나 다른 담당자의 변경은 삭제하지 않는다.
