# Sequencer 카메라 로드와 프레임 급락 수정 결과

## G00. 적용 범위

사용자의 마지막 요청인 Sequencer 첫 열기·열어 둔 상태의 한 자리 FPS 결함을 우선 수정했다.
시작 작업 트리는 clean, 기준 HEAD는 `2f289461461434bcc0442bae5a71b795e7b34c62`,
작업 브랜치는 `codex/sequencer-camera-load-performance`다.
카메라 로드 소스 수정과 비UI CPU 검사, Debug x64 Client 컴파일·링크·런타임 DLL 배포를 완료했다.
MSBuild exit 0이며 실행 파일은 2026-09-12 10:54:44 KST에 생성됐다.
Client/UI 실행·조작·화면 캡처와 실제 개선 FPS 측정은 하지 않았다.
빌드 대기 중 확인한 조커 native texture 오접속 교정은 별도
[조커 결과](2026-09-12_KOUKU_JOKER_NATIVE_TEXTURE_RESULT.md)에 기록했다.

## G01. 저장 profiler와 재현된 원인

입력은 `Client/Bin/ProfilerCaptures`의 다음 네 파일이다.

| 파일 시각 | 분석 구간 | CPU 평균 | SequenceBenchmark.Build |
|---|---|---:|---:|
| `profiler_20260912_092905_047_frame10_17588_0.json` | frame1~10 | 208.14ms | 약148ms |
| `profiler_20260912_092908_242_frame25_17588_1.json` | frame1~25 | 209.74ms | 149.54ms |
| `profiler_20260912_094110_328_frame653_17588_2.json` | 마지막 완성120프레임,533~652 | 14.57ms | 호출 없음 |
| `profiler_20260912_100004_409_frame55768_17588_3.json` | frame55762~55768 | 738.32ms | 관측4회 평균667.58ms |

09:41 파일에는 앞선 느린 구간도 포함돼 있다. 마지막 완성120프레임의 frame interval은14.6429ms,
약68.29FPS이며 사용자 정지화면 관찰과 일치한다. 전체 history 평균을 정지화면 평균으로 쓰지 않는다.
10:00은 CPU scope4096개 상한으로 뒤3프레임의 Build가 누락됐으며 droppedCpuScopes=3832다.
누락을0ms로 평균내지 않았다. GPU NonBlend timestamp에도 큰 경과 시간이 기록되지만 CPU 명령
공급 공백을 포함할 수 있으므로 GPU 연산 포화나 GPU 정상 여부를 단정하지 않는다.

실제 카메라 정본 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`은
193353bytes, JSON value6036개, shot32개, keyframe349개다. 기존 `Parse_CameraShots`의
value 한도4096은 byte122753에서 `Maximum value count exceeded`로 실패했다.
file256KiB, shot64개 및 shot당 key64개 범위에 드는 문서를 별도의 낮은 value 한도가 거부했다.

`Render_Timeline`은 각 camera occurrence를 표시 범위, lane row, 복귀 tail에서 각각 조회한다.
`Find_AuthoringCamera`가 매번 `Ensure_CameraShotAuthoring`을 호출하고, 실패 상태는 기록하지 않았다.
현재 G1 팝업북·피날레는 camera1개로3회/frame, G2 진입 컷신은 camera5개로15회/frame
같은 파일 읽기·파싱을 반복할 수 있다. Camera Resources/Detail의 추가 조회는 별도다.
저장 JSON은 pane 내부 계측이 없으므로667.58ms 전체를 개별 함수 실측으로 분해했다고 주장하지 않는다.

동일 parser를 쓰는 게시 카메라 로드도 실패한다. `Load_CameraShots` 실패 시 Level 진입은 허용하고
follow 시점을 유지하는 코드가 있어, 시퀀스 카메라 미출력과도 직접 연결되는 로드 결함이다.
저작 파일과 `Client/Bin/DataFiles/Map`의 게시 파일은 SHA256이 같아 별도 카메라 데이터 재배포는 필요 없다.
카메라 프레이밍과 실제 화면 움직임 확인은 별도로 남는다.

## G02. 실제 변경

- `Level_KakulSaydonArena.cpp`: 카메라 문서 value 한도를128Ki개로 조정했다.256KiB·깊이12·shot64·
  key64와 stable ID/방향/시간/유한값 검사는 유지한다. Stage Marker의4096 한도는 변경하지 않았다.
- `Ensure_CameraShotAuthoring`: 성공한 저작 문서는 기존 캐시를 조회하고 최초 실패도 기억한다.
  실패 후 자동 반복 읽기·파싱은 없으며 원래 실패 이유를 반환한다.
- `Reload_CameraShotAuthoring`: 명시 재시도와 bounded bulk read를 소유한다. dirty Camera는 읽기 전에
  거부하고, read/parse 실패 시 이전 shot·baseline·draft를 보존한다. 성공한 staged 결과만 교체한다.
  기존 정상 캐시가 있으면 이후 조회도 그 캐시를 유지한다. 저장 CAS는 그대로 사용한다.
- `KoukuSaydonActionWorkbench.cpp`: Composition Camera 창에 `Reload Cameras`를 연결했다.
  최초 로드 실패 상태에서도 버튼과 재시도 안내가 표시된다.
- 기존 Profiler에 `Kouku.CameraAuthoring.Load` scope를 추가했다. 캐시 조회에는 이 scope가 없다.

기존 C++ 세 파일의 UTF-8 BOM 없음/CRLF를 유지했다. 새 C++ 파일·project/filter 등록은 없다.
카메라 JSON, Sequence 데이터, Effect, 모델과 Resources는 수정하지 않았다. CLAUDE에 새 재시도 사용법,
gotchas에 반복 파싱과 실제 전체 camera 문서 검증 경계를 반영했다.

## G03. 자동 검증과 실행 준비

기존 실제 `DataJson.cpp`, 카메라 parser/helper 및 현재 Ensure/Reload 함수 본문을 out에 추출해
MSVC Debug `/MDd /Od`로 컴파일했다. Level 상태는 필요한 멤버로 구성한 비UI fixture이고
Profiler scope만 제거·계수했다. 캐시 검사에는 실제 `ProjectDataRoot.cpp`를 사용하고 프로세스의
`LOSTARK_PROJECT_DATA_ROOT`를 out fixture로 지정했다. 제품 Data는 쓰지 않았다.

| 검사 | 결과 |
|---|---|
| 기존4096 value 한도 + 실제193353bytes 입력 | 실패 재현, 기존 sentinel output 보존 |
|6035/6036 경계 |6035 실패,6036 성공,32shots |
| 현재 production128Ki 한도 + 실제 문서 |32shots·원문 baseline 성공 |
| malformed 최초 로드 후1000회 Ensure |read1회·parse1회만 발생 |
| 파일을 외부에서 고친 뒤 Ensure |자동 재시도 없음 |
| 명시 Reload |정상32shots로 복구 |
| 정상 캐시1000회 Ensure |추가read0회·parse0회 |
| 정상 캐시 이후 malformed Reload |실패 이유, 이전 shot allocation/ID/count/baseline 보존 |
| dirty Reload |scope/file read/parse 진입 전 거부, draft 보존 |
| 뒤따른 정상 Reload |정상 상태 회복 |

근거:

- `out/SequencerOpen20260912/camera_probe/run_probe.ps1`, `probe.run.log`.
- `out/SequencerOpen20260912/camera_probe/run_cache_probe.ps1`, `cache_probe.run.log`.
- 기존 parser 실패 약18ms/회, 현재 bulk read와 정상 full load 약39.57ms, 정상 Ensure1000회 약0.0014ms.
  이 값은 독립 CPU microbenchmark이며 실제 Client FPS 개선 수치가 아니다.
- 읽기 전용 diff 점검에서 dirty 보존, 실패 캐시, Reload 접근성, camera shot pointer 수명을 확인했다.
  미커밋 diff의 최종 실행 승인으로 기록하지 않는다.
- Debug x64 Client compile/link 및 기존 배포 target은 MSBuild exit 0으로 완료했다.
  명령은 `MSBuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /m`이다.
  근거는 `out/SequencerOpen20260912/client-build.log`와 `client-build.console.log`다.
  기존 FXC X4000/X4008, 코드 페이지 C4819/C4828, DirectXTK PDB LNK4099 경고가 있으며 빌드 오류는 없다.
  새 제품 하네스나 광역 검증은 추가하지 않았다.
- `Client/Bin/Debug/Client.exe`는 54,809,600bytes, 10:54:44 KST 생성이며 x64 PE header를 확인했다.
  해당 시점 SHA256은 `FF82851AC9E020D2FA094371E81A9AE1BCA553146372333EB9B435FF51624B65`다.
  배포된 Engine.dll과 `Engine/Bin/Debug/Engine.dll`의 SHA256도 동일하고 양쪽 x64 PE를 확인했다.
- 사용자 Visual Studio 빌드가 끝부분에 별도로 시작됐다. 에이전트 빌드는 자연 완료했으며 사용자 빌드는
  종료하지 않았다. 위 산출물 기록은 에이전트 빌드 완료 직후 기준이고, 사용자 빌드가 끝나면 파일 시각·hash는 달라질 수 있다.
  사용자는 진행 중인 Visual Studio 빌드가 끝난 뒤 실행한다. 에이전트는 추가 중복 빌드를 시작하지 않는다.

실행 전 LAN script 결과는 server-host, TCP7777 LocalSubnet firewall ready,
192.168.0.14:7777 not-listening이다. 사용자가 Visual Studio의 `Server + Client` profile을
Ctrl+F5로 시작한다. 에이전트는 Client/Server를 임의 종료하거나 실행하지 않았다.

사용자 확인 순서:

1. 새 Debug 실행에서 Lobby → KoukuSaydon → F1 → Open Sequencer Benchmark를 연다.
2. 재생하지 않고 G1/G2 목록과 타임라인을 선택해 FPS 급락이 사라졌는지 확인한다.
3. `연출_팝업북`, `연출_1관문 피날레`, `2관문_진입컷씬`을 각각 Play해 camera를 확인한다.
4. Camera source 오류가 표시되면 원본 수정 후 Composition Camera → Reload Cameras를 사용한다.
5. 같은 구간을 Composition Profiler로 저장해 이전 JSON과 비교한다. 실제 FPS와 camera 화면은 USER_PENDING이다.

## G04. Object Tool 그룹 저작 위치

Effect로 이동하기보다 현재 World Object Motion을 그룹 정본으로 사용하는 방향을 권장한다.
정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`,
패턴 배치·판정 연결은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`이다.

| 대상 | 현재 저장 구조 | 현재 조절 위치 |
|---|---|---|
| 외곽불 | D/E/F×CW/CCW의6Motion, 각 emissions10행, 총60개 | Object Detail의 Revolution Offset, Authored Emissions, Ring Count/Distribute on Ring |
| 갈고리 |1Motion에 emissions18행, 패턴18/19가 공유 | 행별 XYZ/Yaw/Delay, Motion Transform·clip, 속도 |
| 바닥 칼날 | count1 Motion, 패턴31에 독립 WORLD box5개, 현재 DRAFT | 각 Box TRS; 그룹화할 때 기존 Motion emissions로5개 배치 이전 |

`WorldObjectTool.cpp`의 Object Detail은 이미 Count/Interval/Spread, 속도·가속도·자전·공전,
행별 위치·방향·시각, Add/Dup/Del을 지원한다. emissions가 있으면 행 수가 Count이므로
Count/Interval/Spread는 의도적으로 잠긴다. 현재 불·갈고리의 개수 변경은 행 추가·삭제 또는 Ring 재배치다.

Box Detail은 그룹 전체 Transform, Start/Lifetime/Playback Speed를 이미 편집한다.
다음 확장으로 정확한 Object/Motion을 Object Tool로 여는 버튼과 생성 수·배치 요약을 연결하고,
그룹 전용 간격·레인·웨이브 프리셋은 Object Detail에서 저장하는 것이 적합하다.
두 편집기에 같은 JSON의 별도 writer를 만들지 않는다. 이 UI 확장은 이번에 구현하지 않았다.

갈고리18행은 Collider/Logic18개의 `worldEmissionIndex`0~17과 연결된다. 개수·행 순서를 바꿀 때
이 참조와 판정 시각도 함께 조정해야 한다. 현재 Object Tool은 Composition 참조를 자동 변경하지 않는다.
Box Scale은 경로·간격에도 곱해진다. 외형 크기만 바꿀 때는 부모 Object Scale 또는 Motion Scale Multiplier를 쓴다.
현재 World→Effect 변환은 공전과 움직이는 Transform key 등 일부 Motion을 전달하지 않으므로 이관은 기능 손실이 있다.

## G05. 아레나 실측과 후속 경계

현재 F1 1·3관문의 플레이어 이동 좌표는 모두 `(-2.45,1.32,945.17)`,
boss spawn도 `(-0.07,1.32,942.330017)`로 같다. 테스트 전투 장소를 공유한다는 판단은 맞다.
전투 장소는 SL05 FLOOR08 placement12451899878577673092의 z942.080017이고,
SL04 완성 무대 placement13002692270586604369는 z737.280029에 있다.
책 촬영용 배우·펼침 복사본과 전투 placement는 별개다. 현재 공유 테스트 장소를 원작 관문 구분이나
책의 skinned 모델 자체가 고정 전투 바닥이 된다는 증거로 확대하지 않는다.
자세한 원본 근거는09-11 `KOUKU_CUTSCENE_SOURCE_VISUAL_AUDIT_RESULT` G03을 따른다.

현재 독립 Sequence 저장본에는 G1 두 개와 G2 하나가 있고 G3 Sequence는 없다.
사용자의 마지막 우선순위에 따라 이번에는 FPS 관련 로드 결함 수정에 집중했다.
다음 항목은 미구현 상태로 남는다.

- Sequence/Complete Play와 F1 Pattern Flow의 동일 전투 보스 HP·scale·spawn 연결,
  sequence 종료 시 player 이동·follow camera·boss HUD·전투 전환.
- G1 수렴·흡입과 책 펼침의 전체 연결, 추가 G2/G3 연출.
- 조커 카드 사전 생성과 외곽불 보라색 재질 재검토. 조커 앞면의 잘못된 native texture 연결은
  [별도 결과](2026-09-12_KOUKU_JOKER_NATIVE_TEXTURE_RESULT.md)의 네 필드 수정으로 교정했고 사용자 화면 확인은 남는다.
  이전09-09 FIRE_HOOK 결과의 잘못된 specular DDS→emissivePath 연결은 조사 참고이며 현재 원인 확정은 아니다.
- 괴기스러운 인형 골격 복원과 해당 Transform 편집.
- 정지화면 추가 최적화:09:41 tail의 CPU World6.23ms/Update5.95ms, GPU NonBlend6.69ms/
  SSAO1.28ms/Lights1.19ms가 후속 비교 지점이다. 이번에 해당 렌더링 품질이나 workload는 변경하지 않았다.

## G06. 사용자 실행 뒤 재개

사용자는 exe 완료 알림을 받고 자신이 Client로 작업하는 동안 후속 구현을 진행하도록 요청했다.
현재 작업에 `Kouku Client 실행 후 후속 작업` 재개 예약(`kouku-client`)을 연결했었다.
이후 사용자가 직접 exe 실행 후 알려 주겠다고 했으므로 예약을 PAUSED로 변경했다.
현재는 사용자 실행 알림을 받은 뒤 같은 작업에서 후속 구현을 재개한다.
사용 중인 Client/Server/UI를 종료·조작하거나 실행 중인 exe/DLL/셰이더를 교체하지 않는다.
사용자가 편집 중인 authoring JSON 보존과 최신 사용자 지시를 우선한다.

## G07. 남은 작업의 진행 순서

다음은 현재 미구현 작업의 우선순위다. 각 구현의 상세 호출자·실패 처리·변경 코드는 해당 작업의
구현 PLAN에서 실제 소스를 조사한 뒤 확정한다. 이 목록을 구현 또는 화면 확인 완료로 기록하지 않는다.

| 순서 | 작업 | 완료 시 동작 |
|---|---|---|
| 먼저 | 새 exe의 현재 수정 확인 | 사용자 관찰에서 Sequencer 열기 FPS·카메라 재생·조커 앞면을 확인하고, 새 오류가 있으면 최우선 대응 |
| 1 | F1 테스트와 Sequence/Complete Play의 전투 보스 통일 | 같은 관문의 보스가 같은 정의·HP·실제 모델 스케일을 사용하며 타격 가능한 전투 경로를 공유 |
| 2 | 시퀀스 종료와 전투 시작 연결 | 플레이어를 전투 아레나로 이동하고 follow camera·boss HUD·전투를 올바른 종료 시점에 활성화; F1 즉시 테스트와 연출 재생 진입은 분리 |
| 3 | 조커 카드 사전 준비 | 첫 카드 생성 순간의 리소스 준비·인스턴스 생성 비용을 확인하고 패턴 시작 전에 필요한 준비를 완료; 판정과 외형 변종 보존 |
| 4 | Box Detail과 Object Tool의 그룹 편집 연결 | 실제 objectId·instanceId의 Motion을 바로 열고 생성 수를 표시; 기존 개별 위치·방향·속도와 그룹 전체 Transform 편집을 연결 |
| 5 | 그룹 생성 수·간격 일괄 편집 | 외곽불·갈고리·칼날의 배치를 저장하고, 개수·행 순서 변경 시 관련 Collider/Logic 참조와 실패 시 기존 문서 보존까지 함께 처리 |
| 6 | 괴기스러운 인형과 외곽불 재질 | 인형의 원본 골격·본 부착·스케일 관계와 외곽불의 실제 재질 입력을 각각 실측해 교정; 인형 Transform·Rotation 편집 연결 |
| 7 | 정지 상태 추가 최적화 | 새 profiler와 09:41 구간을 비교해 남은 CPU/GPU 비용을 줄이고 실제 화면·효과 품질은 유지 |

1관문 수렴·흡입·책 펼침 전체 연출과 추가 2·3관문 Sequence 저작은 사용자가 직접 추가하는 흐름에 맞춰
이후 연결한다. 현재 저장본에 없는 3관문 Sequence를 이미 존재하거나 재생 가능한 것으로 가정하지 않는다.

실행 중 C++만 준비 가능한 작은 병행 단위는 4번의 탐색 연결이다.
현재 `KOUKU_WORLD_SEQUENCE_RESOURCE.iEmissionCount`를 요약에 재사용하고,
`WorldObjectTool`의 선택 상태와 `MainApp`의 도구 소유 관계를 통해 정확한 Object/Motion을 연다.
자동 Preview·Save·Publish 없이 연결할 수 있다. 반면 5번은 갈고리 Motion을 공유하는 Pattern18·19의
Collider/Logic 참조와 worldsequences/Composition 저장·배포를 함께 다뤄야 하므로 별도의 완결 작업이다.

이 정리 턴에서는 제품 코드·데이터를 추가 수정하거나 빌드를 실행하지 않았다.
현재 사용자 Visual Studio 빌드와 사용자 화면 판정은 별도로 유지한다.


## G08. 사용자 실행 확인과 후속 요청

2026-09-12 사용자는 새 exe에서 Sequencer 프레임 드랍이 없으며 `연출_팝업북`과 `연출_1관문_피날레`가 동작한다고 서면 확인했다. 이는 이번 반복 파싱 수정과 기존 두 시퀀스의 사용자 실행 확인이다. 조커 앞면, 새 통합 시퀀스와 전투 핸드오프까지 확인된 것은 아니다.

후속 범위는 사용자 요청으로 다시 활성화했다. `1관문_통합_시퀀스`에 흡입·책·피날레·Scene Profile을 연결하고, G2 진입/클리어/카드미로와 G3 진입을 생성한다. 기존 F1 G1/G3 열린 아레나는 테스트 환경으로 유지한다. 세부 구현과 검증은 같은 날짜의 SEQUENCE_COMBAT_HANDOFF, SOURCE_SEQUENCE, WORLD_OBJECT_GROUP_EDIT_AND_PREWARM, DIMENSIONMASTER 및 DOLL_FIRE 대응 PLAN/RESULT에서 관리한다. 재개 예약은 PAUSED이며 현재 대화의 명시 요청으로 구현을 진행 중이다.


## G09. 09:41 정지 상태의 기존 측정 재분석

`profiler_20260912_094110_328_frame653_17588_2.json`의 마지막 완성120프레임(533~652)을
다시 집계했다. 평균 frame interval은14.64294083ms, 환산 FPS는68.2923, CPU frame은
14.56994917ms다. 이는 이전 사용자 저장본의 분석이며 새 통합 시퀀스나 새
빌드의 FPS 실측이 아니다. 원시 수치와 분석 조건은 `out/SequencerOpen20260912/idle_review.json`에 있다.

| 비교 지점 | 평균 시간 |
|---|---:|
| CPU Render.World | 6.22756ms |
| CPU Client.Update | 5.95296ms |
| CPU Render.NonBlend | 2.75449ms |
| CPU Animation.Play | 1.26803ms |
| CPU Effect.Occurrence.Render | 1.21314ms |
| CPU ImGui.BuildAndSubmit | 1.93380ms |
| GPU Render.NonBlend (유효117프레임) | 6.68860ms |
| GPU Render.SSAO | 1.27931ms |
| GPU Render.Lights | 1.19348ms |

CPU scope는 자식 시간을 포함한다. World와 그 안의 NonBlend, Update와 그 안의 Animation을
더하지 않는다. CPU/GPU도 동시에 진행하므로 합산하지 않는다. GPU timestamp의 경과 시간만으로
연산 포화를 확정하지 않는다.

이120프레임에는 SequenceBenchmark.Build, 파일 Load/ReadBytecode/Parse/Create 계측 호출이 없고,
textureRequests/uniqueSRVs, ImGui texture 생성/buffer growth, picking readback, scene color copy는
모두0이다. animated model4개는 모두 실제 제출됐고 notSubmittedCpuMs는0이다. Map visibility는
1158회/frame이지만 합계0.09043ms여서 큰 반복 비용으로 해석하지 않는다. 같은 자료에는 ImGui
platform viewport3개가 열려 있다.

반복 I/O나 동일 리소스 재생성으로 확정할 근거가 없어 정지 상태용 소스를 추가 변경하지 않았다.
이번 camera cache 수정은 이 tail에서 호출되지 않으므로 idle70FPS 개선의 증거로 사용하지 않는다.
후속 비교는 사용자가 새 빌드에서 위치·시점·보스 수·F1 상태를 맞춰120개 이상 완성프레임을 저장한
뒤 위 비용을 대조한다. F1을 닫은 측정은 별도 조건으로 구분한다.

## G10. 확장 카메라 범위와 실제 캐시 검사

G01~G03의256KiB/64shot은 최초 수정 시점의 범위다. 이후 통합 시퀀스의83shot 후보를 수용하도록
현재 소스는 다음 한도를 사용한다. 아직 실행 중인 기존 Client의 한도로 표현하지 않는다.

| 항목 | 현재 소스 한도/검증 입력 |
|---|---|
| 카메라 문서 크기 | 2MiB (2,097,152bytes) |
| shot 수 | 128개 |
| JSON value 수 | 128Ki개 (131,072) |
| JSON 깊이 / shot당 keyframe | 12 / 64개 유지 |
| 실제 검사 후보 | 83shot, 1,311,326bytes |
| 실제 원본 Data | 기존193,353bytes 문서 유지; 확장본은 out 후보 |

검사 후보는 `out/KoukuSourceSequenceRestore20260912/candidate/Data/Maps/Authoring/`
`LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`이다. 현재 최종fixture와 후보는
byte-identical이다. 전달됐던1,265,340bytes 대신 실제 확인한1,311,326bytes를 기록한다.

`out/SequencerOpen20260912/camera_full_probe/cache_probe.run.log`의 실제 로그는 다음과 같다.

- 최초 정상 명시 Reload:210.44ms, 실제 parser가83shot과 정확한 source baseline을 로드.
- 정상 캐시 Ensure1000회:총0.0041ms, 추가 파일 읽기·파싱0회.
- 실패 Ensure1000회:총0.5774ms, 파일 읽기1회·실제 파싱1회.
- 실패한 명시 Reload는 기존 shots allocation/ID/count/baseline을 보존하며, dirty draft는 파일
  검사 전에 거부한다. 전체 검사 합계`reads=4 parses=4 scopes=4`로 PASS.

현재 함수와 실제 DataJson/ProjectDataRoot를 사용하는 out 전용 CPU 검사다. 한 번의 최초 로드
210.44ms를 프레임당 비용으로 확대하지 않으며, Client/UI 실행·새 시퀀스 FPS·GPU 표시 검증은 아니다.
