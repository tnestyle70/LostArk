# 발탄 Complete Play 준비 대기 결과

## G01. 실제 연결

기존 Level Loading은 준비 실패를 decoration으로 격리하여 아레나 진입을 허용한다. 기존 Complete Play admission은 exact Server revision·primary presentation·Sound receipt를 확인하지만 Effect queue의 pending/failed 상태를 다시 확인하지 않았다. 이번 변경은 단일 패턴 Complete Play의 서버 요청 앞에 로컬 준비 대기를 연결한다.

`CValtanBossTool::Submit_SelectedPattern`은 요청한 stable pattern, Server-active revision, Sound receipt, world generation을 pending으로 보존한다. `Update_PlayPreparation`은 도구 창 가시성과 무관하게 준비를 진행한다. `CLevel_ValtanArena::Debug_PrepareCompletePlayResources`는 선택된 admission tree의 Product V0 cues, 실제 V2 binding, 해당 combat-object의 active/hit, 발탄/유령의 catalog 기본 부착 효과, 해당 cinematic instance의 effectTracks를 모은다. 서버 발탄이 사용하지 않는 선택적 local V1 audition alias는 준비 대상에 넣지 않는다. 입장/버러지들/사망/2페이즈는 실제 제품 consumer와 같은 instance를 준비하며 무관한 52개 cinematic 전체를 매번 대상으로 삼지 않는다.

V1은 기존 priority queue와 current/settled/prepared 전수·failed0·unavailable0 판정을 사용한다. V2는 기존 runtime snapshot과 `Prewarm_Group`, World는 기존 `Prepare_InstanceResources`를 사용한다. V2/World는 한 update에 한 항목씩 진행하고 기존 리소스 cache를 재사용한다. 별도 Effect runtime, 로컬 전투 실행, Shared/wire 변경은 없다.

준비 완료 직전에 기존 admission으로 Product revision/Sound receipt를 재확인하고 기존 `CValtanPatternAuditionService::Submit`을 한 번 호출한다. pending은 Submit 전에 제거하여 다음 update가 같은 요청을 다시 보내지 않는다. 준비 중에는 서버 요청과 서버 deadline이 시작되지 않는다. UI의 accepted 반환은 로컬 준비 요청 접수이며 상태 문구는 `Server playback has not started`로 구분한다.

## G02. 취소와 화면 사용

`Valtan Complete Play (Server Boss Replay)` 및 Boss Tool Action Bar에 `Cancel Complete Play Preparation`을 연결했다. 진행률은 `Preparing Valtan Complete Play: V1 n/n, V2 n/n, WORLD n/n`으로 표시한다. 완료되면 재클릭 없이 자동 Submit한다. 선택 변경, graph reload, world/session generation 변경, Server-active revision 변경, resource cache/World revision 변경, 준비 실패, 20분 초과는 요청 전에 취소한다. 최종 admission 거절도 그대로 표시한다. 준비 중 다른 Restart/Next/Flow 명령과 Sound 변경은 pending 소유권 경계에서 차단한다.

범위는 이번 요청의 단일 패턴 Complete Play/Play 경로다. 별도로 시작하는 Saved Flow 전체에 같은 새 dependency 준비 단계를 추가한 것은 아니다. 기존 Server의 실패·취소·deadline 및 실제 playback 판정은 유지한다.

## G03. 실행한 검증과 남은 확인

- `Level_ValtanArena.cpp`, `ValtanBossTool.cpp` 최신 두 TU Debug `/Zs` PASS. DirectInput 버전 미지정 안내만 존재한다. MainApp는 Valtan selection/cancel UI 두 함수의 국소 변경이며 root가 최신 통합 컴파일을 수행한다.
- 실제 production `Cancel_PlayPreparation`/`Update_PlayPreparation` 본문을 그대로 추출하고 외부 서비스만 fixture로 치환한 headless 하네스: 16상태, 30 checks PASS. 준비 20회 poll 동안 Submit0, ready 뒤 Submit1, 후속 update 중복0; 명시 cancel, disconnect, world generation, active revision, 선택·inventory, graph, authoring transaction, timeout, resource 실패, final revision/Sound/입장 거절은 Submit0; typed Submit 자체 거절은 1회 후 재송신 없음. `out/RaidRegression20260920/valtan/state_probe.log`.
- 위 하네스는 실제 GPU의 모든 Valtan 리소스를 전부 prewarm하거나 서버에 접속한 검사가 아니다. 실제 queue/prototype 소비자는 제품 코드에 연결했고, 최종 사용자 실행에서 준비 진행률·완료 후 재생을 확인해야 한다. Client/UI는 에이전트가 실행하지 않았다.
- 데이터 파일은 변경하지 않았다. 기존 다른 작업의 dirty 변경을 보존했고 새 source 파일이나 프로젝트 등록은 없다. 코드 freeze 뒤 Product build와 실행 결과는 root 통합 결과에 기록한다.
