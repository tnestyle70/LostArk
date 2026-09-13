# MAP Effect 위치·앞 대기와 World Level Tool 결과

## G00. 확인한 원인

MAP Effect 박스의 `PositionOffset`은 절대 월드 미터이다. Effect Tool의 독립 Play All 원점과 다르다. 저장된 포탈 원점은 `(72.857119, 1.761627, -99.769365)`이다. 이 원점에 대한 화면 표식과 카메라 이동 경로가 없어 비활성 시각이나 화면 밖 배치에서 위치와 렌더 실패를 구분하기 어려웠다. 현재 실제 사용자 카메라/플레이어 위치와 GPU 표시 여부는 자동 판정하지 않았다.

조사 도중 사용자가 저장한 Sequence revision22에서는 centered 포탈 박스가 이미 start0/duration36541 ms로 바뀌었다. 따라서 직전 저장본의 start14603 ms를 현재값으로 간주하지 않는다. centered30은 내부 첫 emitter가0초이고, 원본 주변 연출 context120은 내부 공통 Start Delay가17.1347671585083초였다. Box Detail의 명시 Preview도 geometry overlay를 거쳐 현재 커서에서 paused Pattern을 열고 있었다. 박스 시각·내부 지연·미리보기 transport는 별개의 원인이다.

## G01. 소스에 반영한 동작

- Box Detail의 `Use Mouse Position`은 stable Pattern/Occurrence, exact request token, draft generation과 전체 편집 snapshot을 보낸다. MainApp은 기존 `Picking`의 보이는 mesh world-position pixel을 읽는다. 빈 화면은 원점으로 대체하지 않고 재클릭을 기다린다.
- 클릭 좌표는 선택한 MAP Effect 편집과 기존 geometry overlay에 전달한다. 키보드 입력·일반 gameplay 이동과 별도인 저작 명령이며 LB/RB를 release까지 소비한다. 버튼 클릭 자체, Esc/우클릭, 다른 프로세스·레벨·Area·도구·선택·편집으로 전환된 요청을 구분한다. 분리된 동일 프로세스 ImGui 창에서 시작한 요청은 유지하며 실제 viewport 클릭만 픽킹한다.
- `Focus Position (F)`는 현재 Level owner의 카메라를 원점으로 이동한다. F는 focused Details/World Level Tool의 입력 편집 밖에서만 동작한다. 이펙트 재생 여부와 무관한 XYZ 표식과 화면 밖 상태를 표시한다. 연출 카메라가 점유 중이면 Composition preview를 정리하거나 소유자 Stop이 필요한 이유를 표시한다.
- 명시적 Effect Box Preview는 선택한 박스 시작 시각으로 이동해 재생한다. 일반 TRS 수정은 기존 actor cursor와 paused 상태를 유지한다.
- `Open World Level Tool`은 MapCatalog의 Area, map/deploy placement, World Sequence, Gameplay, Light 및 열린 Composition draft를 통합 조회한다. stable ID로 기존 owner를 열고, static map/create는 Test MapTool, Object motion은 WorldObjectTool, Effect/World box는 해당 Action/Sequence Workbench에 연결한다. 다른 Area 조회는 레벨을 전환하지 않는다. Static Deploy·surface Effect의 미지원 편집 경계와 원본 provenance를 표시한다. MAP light offset이나 동적 WORLD 기준을 절대 좌표로 잘못 표시하지 않는다. 검색 인덱스는 검색어·종류·목록 변경에만 갱신한다.

## G02. 포탈 앞 대기 제거

`effect.kouku.gate1.authored.portal-arrival.context`의120요소 후보에서 각 Start Delay를 공통17.1347671585083초만큼 줄이고 `sourceTimeOriginSeconds`를 같은 만큼 더했다. 다른 요소 필드와 생성 간격은 그대로 유지한다. 단순히 박스 start를0으로 만드는 수정과 다르다.

Effect Tool Current Effect에는 `Effect leading delay`와 `Remove Leading Delay` 명령을 추가했다. 독립된 source transform 기반 맵 연출만 승인하며 model cue, actor preview, 부착, baked history 등 다른 시간 계약을 가진 문서는 거부한다. 미적용 Detail draft가 있으면 보존하며 먼저 Apply하도록 안내한다. 기존 `Try_CommitDocument`와 Save Changes가 편집·저장을 소유한다.

현재 후보 경로는 `out/KoukuBoxPreviewTiming20260913/leading-delay/effect.kouku.gate1.authored.portal-arrival.context.effect.json`이다. sourceSHA는 `1f484aea4961c33cd516f379bd988131937e8fdb81a19d30c8cbff399c0014cf`, candidateSHA는 `9d09ef9e111913872210c74e48ac599911536a14e33520058a28e19e55c25acb`이다. 후보 receipt의 installed는false이다. 실행 중 편집 보존을 위해 저장 파일을 외부에서 덮어쓰지 않았다.

추가로 Effect Tool Play All에 남아 있는 Original1 전체150요소도 확인했다. 전체 문서의 공통
대기는14.602761848449706초이며 context 첫 발생이17.134767초다. `leading-delay/original1/`의
별도 미설치 후보는 전체 앞 공백만 제거해 포탈0초, 흡입2.009663초, context2.532005초의 간격을
유지한다. Original1 후보SHA는 `e867a88b065052b6ebc71f29f4be232a7ee43f57cac1e24563587d355936dbd3`.
원본2 금빛의 시간은 변경하지 않았다. 이 후보를 생성기에 입력해도 centered/context 파생본은
원점과 모든 값이 이전 생성 결과와 동일했다. 실제 C++ 추가2360검사 PASS이며 후보 설치기는
두 source의 현재 hash를 모두 확인하고 백업·교체·실패 시 rollback하도록 준비했다. 설치기는
parse만 검증했고 실행하지 않았다.

## G03. 실행한 검증과 남은 적용

- 실제 Workbench CPP 격리 컴파일과67개 C++ 검사 PASS: stale token·ID·generation·편집, 잘못된 좌표, 취소, 성공 시XYZ만 변경, World/Presentation deep link, timing Preview와 기존 cursor 보존. `out/KoukuPattern3Sequence20260913/placement_ui/receipt.json`과 `out/KoukuBoxPreviewTiming20260913/workbench-probe/run.log`.
- 실제 `MainApp::UpdateMapEffectPlacementInput` 본문을 변경하지 않은 Win32/ImGui/Engine 경계 shim의 격리 C++170개 검사 PASS. 버튼 held/release, 다른 UI capture, surface 실패/NaN과 재시도, detached 같은 프로세스/main HWND, 선택·레벨·Area 전환, Esc/RB, 새 token과 FOCUS를 검사했다. `out/KoukuPattern3Sequence20260913/placement_input/receipt.json`. 실제 UI를 조작하거나 팝업 키 전달까지 검증한 결과는 아니다.
- WorldLevelTool 및 MainApp/MapTool/Bern 변경 소비자 `/c` PASS. `out/KoukuPattern3Sequence20260913/worldlevel_compile`, `worldlevel_integration_compile`. 기본 실행·중간 산출물554개 변경0이었다. 새 CPP/H의 project/filter 등록과 catalog inventory검사 PASS. Bern map50017개, Valtan13184개, Character Select803개를 읽었으며 쿠크 map3368개·World Sequence256개 등은 저장 inventory다.
- 이후 공유 작업 공간에서 수정된 Effect Resource Browser 변경도 보존한 채 현재 Editing/ResourceBrowser를 다시 격리 컴파일했다. 최종7개 CPP의 현재 source hash와 컴파일 receipt 일치, XML 단일 등록과 현재 JSON parse를 확인했다. `out/KoukuPattern3Sequence20260913/worldlevel_final_source_receipt.json`. 이는 Product 전체 링크를 수행했다는 뜻이 아니다.
- 실제 Effect codec 및 source transform/native material 함수로 대기 제거2300개 검사 PASS. source clock오차최대3.8147e-6초, matrix component최대2.95639e-5, native packet lane차이0.120요소 포탈과 실제6개 재질 carrier/4곡선,12개 거부 조건을 검사했다. `out/KoukuBoxPreviewTiming20260913/leading-delay/probe-receipt.json`.
- 포탈 분리 생성기에도 context 공통 앞 대기 제거 정책을 반영했다. 생성기 반복 결과는 결정적이며 원본150은 그대로, centered30은 설치본과 byte동일, context120은 timing-only 후보와 의미가 같다. Effect Tool 변경2개 CPP `/c`도 PASS. `out/KoukuBoxPreviewTiming20260913/leading-delay/generator-receipt.json`, `full_compile_receipt.json`.
- Client37532와 Server11248은 사용자의 미저장 편집 때문에 유지했다. 이 결과의 새 UI는 현재 실행 파일에 반영됐다고 주장하지 않는다. 포탈 후보 설치·Product 전체 링크/EXE 교체는 사용자의 저장·종료 확인 뒤 수행한다. 최종 위치·렌더링 화면은 사용자가 직접 검증한다.
