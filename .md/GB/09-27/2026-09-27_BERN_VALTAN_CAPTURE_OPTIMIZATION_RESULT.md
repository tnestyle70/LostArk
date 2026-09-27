# 베른·발탄 최신 프로파일 분석과 비용 절감 결과

## G00. 원본 캡처의 병목

사용자가 저장한 2026-09-27 JSON 일곱 개를 분석했다. 각 120프레임, GPU 유효116프레임,
CPU scope 누락0이다. 마지막 발탄 파일 `051815`는 사용자 설명에 따라 전투로 분류했다.
측정 화면·카메라·설정이 서로 다르므로 Debug/Release 비율은 통제된 A/B 결과가 아니다.

| 캡처 | 평균 간격 ms | p95 ms | CPU Update ms | CPU NonBlend ms | GPU NonBlend elapsed ms |
|---|---:|---:|---:|---:|---:|
| Release 베른성 | 15.094 | 17.852 | 6.746 | 4.857 | 7.160 |
| Release 베른성 근접 | 11.675 | 12.744 | 6.253 | 3.049 | 5.570 |
| Debug 베른성 일반 | 44.860 | 50.932 | 16.197 | 19.339 | 24.177 |
| Debug 베른성 근접 물·지형·캐릭터 | 37.072 | 40.115 | 14.797 | 12.872 | 16.865 |
| Debug 베른성 컷신 | 50.810 | 78.872 | 17.696 | 23.716 | 29.650 |
| Debug 발탄 기본 | 32.951 | 36.304 | 5.816 | 19.765 | 24.108 |
| Debug 발탄 전투 | 31.056 | 35.522 | 7.217 | 14.933 | 10.648 |

공통 최대 구간은 맵의 불투명 제출·렌더링이다. 베른의 Animation.Channels.Update는
Release1.49~1.50ms, Debug2.91~3.10ms다. 발탄 전투에서는 dynamic shadow의 GPU elapsed가
9.767ms이며 광원 제출 record는 평균262.8개다. GPU timestamp에는 CPU 제출 공백이 포함될
수 있으므로 이를 전부 GPU 연산 포화로 단정하지 않는다. 정적 그림자 cache hit는 두 발탄
캡처 모두 매 프레임1회이며 재생성 실패가 아니다. Debug 다섯 캡처는 모두 D3D debug layer
활성, Release 둘은 비활성이다. 품질 설정과 검증 레이어를 끄는 변경은 하지 않았다.

## G01. 실제 반영

- `MapAssetRenderUtils::Bind_Material`: source indirect가 없는 일반 맵 재질은 cube COM
  참조와 경로 문자열을 포함하는 RenderEnvironment 복사를 하지 않는다.
- 같은 파일의 `BindForwardSceneLights`: 실제 활성 광원 prefix만 Effects11 setter에 전달한다.
  0개는 count만 갱신한다. 입력 순서·receiver·영향 범위·400개 예산·셰이더 식은 같다.
- `CMaterial::Bind_SurfaceTexture`: 소유 중인 SRV를 const 참조로 전달하여 draw마다 불필요한
  임시 COM AddRef/Release를 없앴다. texture override와 누락 실패 경로는 유지한다.
- `CChannel::FindRightKey`: clone별 cursor에서 최대 네 구간을 검사하고 큰 seek에는 기존
  upper_bound를 사용한다. 보간식·골격·애니메이션 갱신 빈도·재생 시간은 그대로다.

위 변경은 기존 공통 map/material/animation 경로이므로 해당 소비자를 사용하는 다른 맵에도
적용된다. 신규 런타임, 영구 하네스, 리소스, public 저장 schema는 추가하지 않았다.

## G02. 실행한 검증

실제 수정 전 HEAD와 현재 CPP에서 함수를 추출하여 같은 MSVC x64 `/O2 /MD` fixture로
대조했다. 원본 파일·표본별 요약·재현 스크립트는
`out/BernValtan20260927/{capture-summary.json,verify_hotpaths.py}`에 있다.

| 검사 | 결과 |
|---|---|
| 키2/3/5/37/256/2048개, 중복 timestamp, random seek, invalid/null cursor | 300,000입력 × cursor 유무, std::upper_bound와 반환 위치 동일 |
| 2,048키의 연속 재생, 약1,200만 검색/회 | 이전188.303/188.017/188.857ms → 수정34.543/35.238/35.270ms |
| 실제 forward-light helper, count400→0→1→17→5→384→0→256→401, ambient/receiver/cull 조합 | 72조건에서 반환값·활성 prefix 동일, overflow 보존 |
| 광원1개, ambient 포함 Effects11 setter 전달량 | 32,004→84bytes; GPU cbuffer 업로드 크기와는 다른 지표 |
| 인코딩 | 세 CPP UTF-8 BOM 없음·CRLF 유지 |

검색 microbenchmark의 개선은 전체 게임 FPS 개선 수치가 아니다. 전체 제품 빌드와 통합
검증 결과는 아래에 추가하며, 동일 조건의 사용자 재캡처·실제 화면 확인은 아직 수행하지 않았다.

## G03. 카메라·드래곤·도끼·머리카락 통합 검증

아래 제품 빌드는 추가 가디언 복장 요청을 반영하기 전의 통합 후보를 검증했다. 새 복장과
눈 조명 수정의 최종 빌드·검증은 별도 결과에 구분한다.

- Debug Product PASS: `out/BuildPipeline/runs/20260926T205710985Z-debug-product.json`.
- Release Product PASS: `out/BuildPipeline/runs/20260926T210031989Z-release-product.json`.
- 실제 Debug/Release `Server.exe --vehicle-riding-contract-test` 각각 exit0. Debug56항목 PASS,
  Release도 실패0. nav 밖 비행·벽 위 통과·수직 몸체 관통 방지·동적 지형 붕괴와 낮은 층 착륙
  취소의 고도 유지·마지막 안전 지면의 재투영까지 포함한다.
- 도끼 writer의 실제 C++ Save/Load 검사, publisher14개 입력 검사와 Dragon publisher10개
  저장·충돌·rollback 검사는 각 RESULT에 기록했다.
- 변경 JSON/XML parse, 신규 TU 프로젝트/필터 중복 없음, 기존 C++ 인코딩 유지 확인.

제품 Client/UI를 자동 실행하지 않았다. 비행 시점·날개와 잔상·도끼 위치·무비 머리카락의
최종 시각 판정 및 같은 조건의 성능 재캡처는 사용자 확인 항목이다.

관련 결과: [드래곤·카메라](2026-09-27_DRAGON_FLIGHT_CAMERA_IMPLEMENTATION_RESULT.md),
[발탄 도끼](2026-09-27_VALTAN_AXE_EDITOR_RESULT.md).

## G04. 추가 충돌 회귀 검사의 경계

`Server/Bin/Debug/Server.exe --debug-teleport-contract-test`도 실행했다. 이 넓은 Kouku
회귀 검사는 exit1/18실패다. Mario 근접 공격을 정확히 한 번 받는 기대값14회와 다른 F1
목적지로 이동한 뒤 Mario 형태 초기화 기대값4회가 실패했다. 실패 항목을 숨기거나 전체
회귀 PASS로 기록하지 않는다. 실제 로그는 `out/BernValtan20260927/collision-regression-debug.log`다.

이번 충돌 변경과 직접 관련된 Mario lane body stop과 일반 이동 tangent slide는 모두
PASS다. 실패한 공격 fixture는 빈 collision을 사용하는 `CMonsterBrain::Update`의 공격
경로이며, 형태 초기화 fixture는 `Reset_PlayerForDebugTeleport/Update_MarioControlState`를
직접 호출한다. 이 함수들과 Kouku 데이터는 이번 변경으로 수정하지 않았다. 이전 commit의
동일 fixture 비교 실행은 하지 않았으므로 기존 결함으로 확정하지 않으며, 이 별도 실패의
수정·원인 확정은 이번 베른·발탄·드래곤 작업의 완료 항목에 포함하지 않는다.

## G05. 추가 의상·눈까지 포함한 최종 제품 빌드

원래 요청한 기능의 최종 Debug와 Release Product가 모두 PASS다. 이 성공본 이후 사용자가
별도로 요청한 전체 빌드 병목 개선은09-12 C++/09-14 shader PLAN·RESULT에서 이어간다.

- Debug: `out/BuildPipeline/runs/20260926T215804868Z-debug-product.json`,33분07.884초.
- Release: `out/BuildPipeline/runs/20260926T223356797Z-release-product.json`,34분48.111초.
- Release Engine/Shared/Server/Client 컴파일·링크·배포와36개 runtime data 검사 PASS.
- 기존5의상 슬롯의 실제25개 part·50개 자세,실패 rollback까지 정식 Debug runtime으로
  879 assertions PASS. `out/GuardianOutfits20260927/consumer-test/final-verification.json`.
- native1526은 정식 Debug CSO/DLL 및 실제 D3D11 hardware에서3방향 draw/readback,
  9개 target 표본의 non-finite0. `out/Guardian1526Gpu20260927/gpu-finite-report.json`.
- 변경된 SourceCharacter 계열72개 Engine/Client CSO는 Debug/Release bytes가 모두 같다.
  `out/BernValtan20260927/debug-release-shader-comparison.json`.
- JSON8/XML2 parse,기존20개 C++ 인코딩 및 신규 TU 등록,`git diff --check` PASS.
- GBResources180개/120,692,356bytes는 설치 Resources와 전부 SHA256/size가 같다.
  `out/BernValtan20260927/final-integrity.json`.

눈·머리카락과5개 World 무비의 편집/재생 근거는 해당 복원 RESULT에 기록했다. 위 성공은
제품 Client 화면 실행이나 FPS 재측정이 아니다. G04의 별도 Kouku18실패는 여전히 분리해
보존하며 전체 광역 회귀가 PASS라고 표현하지 않는다.

## G06. 완료 빌드의 비용 조사와 후속 작업 경계

Debug의 C++ 출력은 Engine2개/Client18개 OBJ,PCH0개이고 Shared/Server는 출력 변경0이다.
기존 PCH·증분 최적화는 작동한다. 실제 FXC 출력은 Engine24개+Client48개다. Client receipt의
CSO72개 중24개는 Engine 배포 복사이므로 실제 shader 컴파일96회로 세지 않는다.

새 의상1526을 Base/Light 공통 dispatcher에 등록하면서 실제 tlog의 Client48/Engine24
소비자에 변경이 전파됐다. Group1088은 Base52개 함수/46,641줄에 Light14,833줄이며,
SourceMovieStaticForward29,334줄은 비0 Static 그룹에도 파싱 입력으로 들어간다. 실제
영화 평가 호출은 group0으로 제한되므로 모든 그룹이 영화 PS를 생성했다는 뜻은 아니다.
기존 FXC4-worker,pass 공유,그룹 분리와 C++ 최대8-worker 설정은 유지된다.

`compile-bottleneck-audit.json`은 원인·한계,`shader-inventory.json`은 MSBuild로 평가한
Engine25/Client208개 shader target과 실제 tracked-input 크기를 기록한다. 순간 CPU 사용이
낮았으나 affinity/전원/EcoQoS에서 강제 제한은 확인되지 않아 원인을 단정하지 않는다.
시간 단축이나 후속 구조 변경 완료는 별도 빌드 최적화 RESULT의 실제 검증으로만 판단한다.

## G07. 사용자 확인과 다음 작업의 구분

현재 요청의 구현·자동 검증과 실제 화면 확인을 다음처럼 나눈다. 새로운 기능이나 화면
관찰 결과를 추정하여 완료 표시에 포함하지 않는다.

| 항목 | 현재 반영 | 사용자가 확인할 부분 |
|---|---|---|
| 베른 카메라 | 기본 이동속도20,Debug/Release F1 Camera 조절 | 실제 이동 감도 |
| 드래곤 시점 | 탑승 지상은 기존 follow,비행 phase부터 flight camera,Debug F1 Dragon 조절 | E 이륙·착륙 시 화면 전환 |
| 드래곤 이동 | 지상10/비행16/수직8,공중 nav 제한 해제와3D 충돌·안전 착륙 | 기존에 못 가던 위치와 착륙 |
| 날개·잔상 | pose clock/bob 동기화,본 plume30개 lifetime/density/alpha 조정 | 날갯짓·상하 움직임·파티클 인상 |
| 발탄 도끼 | F1에서 normal/ghost 위치·회전,Save/Save+Publish | 원하던 부착 위치와 방향 |
| 성능 |7개 capture 분석,애니메이션 검색/광원 setter/texture 참조 비용 개선 | 동일 조건의 새 JSON/FPS |
| 가디언 눈 | 일반/movie eye 두 재질의 누적 최소 광량과 highlight 튜닝 | 클래스 선택·무비의 눈 대비 |
| 창술사 머리카락 | native600의 unowned CB prefix 합성값 identity | 무비 재생 화면 |
| 가디언 복장 | 기존5슬롯의 기본1+대체4,새 슬롯 없음 | 실제 생성 화면의5가지 외형 |
| World 무비5개 |OpenEditor/PlayAll/편집/SaveReload 실제 callback 검사 | 사용자의 최종 재생 화면 |
| 추가 Resources |GBResources180개/115.10MiB,설치본 SHA256 일치 | 다른 PC에서는 같은 상대 경로로 병합 |

Dragon 속도 데이터의 실행 중 Server 적용은 Save+Publish 이후 해당 Server 재시작이 필요하다.
도끼는 Client에서 즉시 보이는 visual socket 편집이며 hitbox 변경이 아니다. 게시된 새 combat
visual generation의 세션 적용은 기존 pin/reentry 계약을 따른다. 저장·파일 게시·실행 중 메모리
반영을 같은 동작으로 표현하지 않는다.

비교 이미지는 별도 headless C++ EXE에서 실제 모델·texture·shader와 Movie camera/lights를
D3D11 hardware로 계산한 뒤 float buffer를 읽고 Python으로 tone/crop/배치한 진단 이미지다.
주변 얼굴은 unlit 배경이며 실제 눈 조명 부분을 합성했다. 제품 Client 창 캡처나 전체 화면의
최종 post-process 결과가 아니며 AI 이미지 생성 도구를 사용하지 않았다.

다음 방향은 사용자의 화면/FPS 재확인 후 정한다. 추가 ScreenPost 함수 축소와 큰 기본 FX의
더 세밀한 분할은 이번 마무리의 제품 반영 범위에 포함하지 않는다. G04의 Kouku 회귀18개
실패도 별도 문제로 남아 있으며 원인이나 이전부터 존재한 결함으로 확정하지 않았다.

## G08. 후속 빌드 최적화의 마무리 범위

G05의 기존 기능 빌드는 이미 Debug/Release 모두 완료됐다. 이후 사용자 요청으로 적용한
빌드 최적화는 다음 네 가지이며,새 기능 구현이나 캐릭터 애니메이션 리소스 재생성이 아니다.

- 새 재질의 case 등록을 기존 Base/Light 그룹 leaf 안에 두어 공통 dispatcher 변경을 방지.
- 큰084/1088의2그룹을 기존 CShader variant 경로 안의7그룹으로 분할.
- 비기본 Static 그룹에서 사용하지 않는 map/movie 함수 본문 파싱을 제외.
- 1.18MB 재질 packing 공개 구현을840bytes API와 단일 CPP/생성 INL로 분리.

최적화 반영 Debug Product는 `20260926T231700570Z-debug-product.json`에서 PASS이며
23분46.122초다. 기존33분07.884초보다 짧지만 재생성 대상과 C++ 변경 수가 다른 한 번의
빌드 비교이므로 모든 빌드의 보장 단축률은 아니다. 실제 binlog의 비용은 shader22분27초
(94.45%),C++1분10초(4.88%)다. 필요한 수식을 처리하는 큰 shader compile은 남아 있다.

새 Debug 실제 CShader 선택·clone·실패 보존·numeric draw 검사는 통과했다. 분할 무관66개
CSO는 기준과 SHA256이 같고,기존1472 그룹의 Base/Light 실제 의존성 합집합은3개 wrapper다.
공통 Light facade는87개를 무효화하므로 기존 그룹 내 등록을 leaf에 두는 것이 다음 변경의
재빌드 범위를 줄인다. 새 그룹·공통 입력 변경은 여전히 공통 소비자를 다시 빌드한다.

C++ packing의 bit/실패 보존11,044개 검사와 실제7개 TU 순차 A/B는 PASS다. 새 owner를
포함한 해당 TU 합계 평균은51.261→30.338초(약40.8% 감소)이고 전체 빌드 단축률과 다르다.
실제 Debug CL.read도 대형 INL 소비자가 새 CPP 한 개임을 확인했다.

분할의 비용으로 CSO가72→87개,71.90→80.16MB 증가했다. 교대3회 WARP 초기 생성 평균은
2737→2785ms이며 캐시·Release 동시 빌드 경합을 통제하지 않은 표본이다. 실제 FPS나
고정적인 로딩 지연으로 환산하지 않는다. 더 큰 분할과 ScreenPost 후보는 보류했다.

상세 근거는09-12 C++ RESULT G08~G10,09-14 shader RESULT G12~G17과
`out/BuildOptimization20260927/final-static-review.json`에 있다. 최종 정적 검토는
JSON11개/XML4개 parse와 신규CPP·INL·shader40개 등록·mirror·배포 정합까지 PASS다.
최적화 반영 Release와 마지막 무변경 반복은 아래 최종 결과에 별도로 기록한다.

## G09. 최종 빌드 완료와 공유 작업 폴더의 검증 경계

후속 빌드 최적화까지 정규 Debug/Release Product가 모두 PASS다.

| 구성 | 직전 기능 성공본 | 최적화 반영 성공본 | 최종 receipt |
|---|---:|---:|---|
| Debug |33분07.884초|23분46.122초|20260926T231700570Z-debug-product.json|
| Release |34분48.111초|25분31.568초|20260926T234347878Z-release-product.json|

두 최종 receipt는 `out/BuildPipeline/runs`에 있다. 새 Release87개 CSO는 보존 Debug 후보와
전부 SHA256이 같으며,보존 Debug CShader로 실제 Release CSO를 소비한 ABI/선택/복제/
실패 보존/numeric draw 검사도 PASS다. Release C++ 제품을 자동 실행한 결과는 아니다.

08:32 이후 같은 폴더의 다른 작업이 Movie/Kouku shader와 발탄 editor 소스를 갱신하고
별도 Debug 빌드를 진행했다. 해당 변경과 프로세스는 보존했다. 재생성 대상·동시 CPU 부하가
달라 위 표를 통제된 A/B 단축률로 사용하지 않는다. 전체 workspace 무변경 재빌드는 이
동시 작업 중 추가하지 않았고 출력0을 확인했다고 기록하지 않는다. 이번 작업의 실제
의존 경계·semantic 검사·정규 제품 빌드 완료와 다른 작업의 진행 상태를 구분한다.

추가 전달분180개/120,692,356bytes는 설치본과 GBResources 양쪽 모두 원래 설치 receipt의
SHA256/크기와 일치한다. GBResources의 전체306개 중 기존 다른126개도 보존했고 빌드
산출물은 없다. 근거는 `out/BuildOptimization20260927/final-resource-integrity.json`이다.

G07의 World 무비 검사는 당시 내부 callback과 저장·재생 경로의 검사 범위다. 이후 별도
무비 편집 작업에서 실제 버튼 진입점·Solo/Group 동작을 더 조사하고 수정 중이므로,이 결과를
모든 편집 버튼과 후속 재질 복원의 완료 판정으로 사용하지 않는다. 이 작업의 눈·머리카락·
복장·드래곤 화면과 같은 조건의 FPS 재캡처는 사용자 확인 항목으로 남는다. G04의 별도
Kouku18개 실패 또한 해결 완료로 바꾸지 않았다.
