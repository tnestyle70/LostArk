# PR494~496 통합과 콜로세움 모집 시간

## G00. 통합 기준과 변경 범위

2026-10-01 사용자 요청에 따라 #494의 장비별 내구도·수리 UI, #495의 콜로세움 경기·결과 UI,
#496의 이동·Guide·용병·워터팡·재질 변경과 기존 쿠크 인형 Collider·칼날·카드 미로 수정을 통합한다.
원본 작업 폴더의 미커밋 변경은 보존하고 별도 통합 worktree를 사용한다.

기준은 #496의 `7013e90ad8c0f79284f794e933a396c4383fe052`, #494의
`1b858eafe17678e6e997285a5ae9de5e643c7b6c`, #495의
`115e3d3d353608ab996fb96eb750deb8458cc8f0`다.

## G01. Server와 Shared 통합

장비 내구도는 inventory item이 전달한다. 기존 sourceWorld admission과 원자적 경기 이동은 유지한다.
독립 PR의 패킷 번호 충돌을 해소하고 통합 protocol을132로 올려 이전 바이너리 혼용을 거절한다.
모집 시작 시 Server가10초 deadline을 정하고 대기 인원과 시간을 복제한다.4명이 먼저 모여도
동일 deadline을 유지하며 마감 때 현재1~4명을 입장 순서에 따라 양 팀에 교대로 배정한다.
취소·연결 종료·초기 송신 준비 실패는 기존 참가자와 방을 보존하거나 해당 큐 항목만 정리한다.

## G02. Client 제품 UI

`RaidEntryPreviewView`의 기존 QueueWait 화면 아래에 남은 시간을, 오른쪽에 현재 인원 N/4를 표시한다.
`Data/UI/Colosseum/QueueDialog_Layout.json`의 stable marker rect를 사용한다.
시간은 Server snapshot과 수신 후 경과 시간으로 표시하고 Client가 팀이나 경기 시작을 결정하지 않는다.
공용 `RaidGateProgressView::Render_AssemblyCountdown`에 문구를 전달할 수 있게 확장하여
기존 발탄·쿠크의 흰색 안내와 노란색 초 표시를 재사용한다.
#495의 실제 참가자 외형·도열·HUD·결과·승리 컷신과 기존 용병 state 소비자를 함께 보존한다.
기존 파일을 확장하므로 신규 C++ project/filter 등록은 필요하지 않다.

## G03. 쿠크와 배포 검증

쿠크 RESULT의 통합 지도와 최신 디스크 diff를 대조하고 관련 hunk와 게시본만 반영한다.
각 최종 JSON/XML parse, `git diff --check`, Debug/Release Product Build,
NetworkProtocolHarness와 콜로세움·내구도·쿠크 관련 Server 계약 검사를 실행한다.
실행한 결과만 RESULT에 남기고 Client 화면·다인 플레이 검증은 사용자 확인으로 구분한다.

최종 Release Product receipt와 같은 소스·Data/DataFiles로 기존 portable builder를 실행한다.
Resources는 외부 팀 리소스를 읽으며 ZIP에는 넣지 않는다. ZIP CRC·manifest SHA256·launcher
`--check`로 실제 실행 경로를 확인하고 원본 작업 폴더의 편집 draft를 자동 Reload하지 않는다.

## G04. 실제 테스트에서 추가된 우선 회귀

카드 미로는 Release에서 혼자 검증할 때 Q 타격이 콜라이더를 통과해도
`CKoukuCardMazeRuntime::Plan`의 Debug 전용 1인 분기 때문에 시작을 거부했다.
방에 인간이 정확히 한 명인 경우의 기존 관찰자·사냥꾼 겸임을 Debug/Release 공통으로 연결한다.
다인 방에서 사망자 때문에 한 명만 살아 있는 경우의 규칙은 유지한다.
실제 Q command decode와 handler, fixed tick, box 파괴, 후속 Q, snapshot 역할·시점 bit·문양,
랜덤 탈출 경로를 1/2/4인으로 검사한다. 편집기 Sequence 개수 3/5 기대값은 이 시작 거부와 별개다.

이동은 동기 GPU pixel readback의 `CopySubresourceRegion -> Map(flags=0)`이
입력 프레임의 주 스레드를 막는 경로를 제거한다. 기존 CPicking에 요청 ID와
`DO_NOT_WAIT` 완료 확인을 추가하여 요청 당시 pixel을 그대로 보관한다.
입력 해제 뒤에도 유효한 요청은 한 번 처리하되 새 클릭, 후속 명령, UI/capture,
캐릭터 교체와 만료는 이전 요청을 취소한다. 제품 Client는 실행하지 않고 생산 함수와
headless D3D11 WARP를 사용해 미완료 GPU copy, 정확한 pixel, 취소·stale reply를 검사한다.
기존 Engine/Client 파일 확장이므로 제품 project/filter 추가는 없다.
`Tools/MovementRegression`의 검증 프로그램은 도구가 생성·직접 컴파일하며 제품에 등록하지 않는다.

사운드는 연속 WORLD 시각 갱신에 남은 250ms 전진 차이의 강제 seek를 없앤다.
명시 scrub과 역방향 이동은 유지하고, 기존/수정 후 생산 함수와 실제 FMOD no-output mixer에서
첫 WAV의 Play/Stop 수명과 재생 cursor를 비교한다. 실청 완료로 대신 기록하지 않는다.
사용자가 보류한 Release 서버 상태 문구는 이번 추가 수정 대상에서 제외한다.

## G05. 워터팡 첫 등장과 가이드 후속 요청

워터팡 AI 첫 등장 때 주 스레드에 모이는 직업 bundle·외형·NPC 모델·물총 준비를
Maharaka Loader의 기존 준비 단계로 이동한다. Server와 Client는 같은 NPC archetype 목록을 소비한다.
선택 class만 미리 준비하던 경로와 실제 첫 spawn에서 읽는 경로를 대조하고,
준비 후 첫 spawn에서 이미 준비된 asset을 사용하는지 검사한다. 약 6초라는 사용자 관찰과
실제 cold load 호출 근거를 구분하며 존재하지 않는 프레임 시간 측정을 완료로 적지 않는다.

NPC 외형은 실제 설치 WModel의 오른손 본과 preScale/basis를 측정하고 기존 CPart_Equipment로
물총을 붙인다. 모든 AI 외형의 부착 준비·무장 상태·visibility와 실패 시 정리를 검사한다.
새 C++ 파일 없이 기존 로더·복제·NPC/장비 경로를 확장한다.
이름은 Server가 슬롯별 `Waterpang AI 1`부터 부여하고 기존 최대 20명 계약은 유지한다.
원문 `1~99`는 표시 번호 형식으로 해석하며 99명 스폰으로 확대하지 않는다.

스퀘어홀로 항구에 이동한 뒤 Guide가 따르지 않는 문제는 베른 내부 이동과 실제 출항의
상태 전환 및 경로 추적을 확인한다. 사용자가 직접 조정하겠다고 한 콜라이더·Guide ID·문구는
변경하지 않는다. 기존 브랜치와 원본 미커밋 변경은 유지하며 통합 브랜치만 병합한다.


## G05. 워터팡 대기 중 최초 준비와 NPC 물총

`CLoader::Ready_For_Maharaka`에서 Server AI 정본의 Guard/Lance 직업, 모코코 12종 의상,
NPC 8종과 물총 모델·재질을 기존 prototype 서비스로 준비한다. `EquipmentPresentationService`의
같은 model admission을 preload와 snapshot 적용이 공유하며 이미 준비한 prototype은 다시 읽지 않는다.
로딩 취소는 각 모델 준비 사이에 확인하고 실패는 기존 Level rollback 범위를 따른다.

`CMaharakaWaterpangPresentation`이 기존 물총 prototype 준비 함수를 공용화한다. NPC 표시체는
실제 설치 8종 WModel에 모두 존재하는 `bip001-r-hand`에 기존 `CPart_Equipment`를 붙인다.
NPC에는 prop3가 없으므로 Guardian의 실제 watergun_idle에서 측정한 prop3→right-hand 상대
TRS를 프로젝트 부착값으로 사용한다. 각 NPC의 최종 손 basis .01을 확인하고 같은 cm 단위 물총을
소비한다. snapshot의 armed/visible 상태와 NPC의 갱신된 pose가 표시를 결정하며 새 combat authority는 없다.

기존 C++ 파일만 확장하며 신규 project/filter 등록은 없다. 최소 검증은 8개 실물 rig·basis·총 크기,
roster와 preload 대상 일치, 생산 호출순서와 취소/visibility 연결, 최종 Product compile이다.
실제 화면과 프레임 체감은 사용자 확인으로 남긴다.
