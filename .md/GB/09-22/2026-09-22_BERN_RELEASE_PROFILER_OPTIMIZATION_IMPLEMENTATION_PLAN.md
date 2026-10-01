# 베른성 캡처 기반 Release 계측·렌더링 최적화 구현 계획

## G00. 현재 실측과 범위

한국어 베른 캡처 다섯 개를 원본 JSON에서 분석한다. 기록에 있는 frame interval, CPU inclusive/self,
유효 GPU pass, 작업량과 누락 표본을 구분한다. 현재 브랜치는 `GB/collider-pattern-bug-fix`이며
다른 세션의 렌더링·게임플레이 미커밋 변경을 유지한다. Client/UI 실행과 화면 판정은 사용자에게 남긴다.

풀줌아웃 첫 프레임은 map placements 50,017, batch 16,421, draw 1,071, indirect draw 0이다.
기존 LOD는 존재하지만 GPU dispatch 비용 때문에 index×instance 294,912 이상의 일부 불투명 재질에만
허용된다. 캡처의 CPU drop은 누적값이므로 이번 프레임의 계측 누락과 구별할 수 없다.

## G01. Profiler.h/.cpp와 실제 렌더 소비자

기존 `CProfiler`를 확장한다. `FProfilerFrame::DroppedCpuScopes`는 해당 프레임에 귀속된
완료 scope 누락 수이며, 누적값은 그대로 유지한다. Reset은 프레임 간격 기준점을 초기화한다.
기존 counter ordinal 뒤에 map culling, LOD 선택·원본/제출 index, shadow cache/caster,
light 제출 작업량을 추가한다. counter는 실제 작업 지점에서 기록하고 object마다 새 scope를 만들지 않는다.
필요한 CPU scope는 light record 작성/제출과 shadow cache 복사 같은 batch/pass 경계에 둔다.
대량의 map batch/mesh CPU raw scope는 `CProfilerDetailScope`로 선택 수집한다. 기본은 pass와 counter이며
상세 모드 변경은 다음 frame boundary에서 적용하고 각 frame에 모드를 기록한다. 선택 window만 Snapshot에서
복사하여 저장할 120프레임을 위해 최대 1200프레임 전체를 복사하고 버리는 비용을 없앤다.

## G02. MainApp → ProfilerTool → ProfilerCaptureIO

Debug와 Release에서 기존 ImGui/CProfilerTool을 공유한다. 사용자 요청으로 F7을 profiler 창
열기/닫기 키로 허용한다. 첫 열기는 수집을 활성화하고 창을 닫아도 명시적으로 멈출 때까지 수집한다.
창의 Capture/Reset/이름/Save JSON으로 조건을 분리해 비동기 저장한다. F1 개발 도구는 Debug 전용이다.
JSON v3에 build/adapter/해상도/장면/렌더 설정의 export 시점 metadata, 분포 요약, GPU 유효율,
CPU 부분 계측 상태를 additive field로 기록한다. 저장 시점 설정을 과거 모든 프레임의 설정으로 단정하지 않는다.

## G03. StaticMeshLod → Mesh → MapStaticBatchObject

CPU에 이미 있는 화면 오차 입력을 단일 compute dispatch로 다시 계산하는 비용을 제거한다.
`CStaticMeshLod::Select_Range`는 indexCount/firstIndex/level을 반환하고 `CMesh`가 기존 index buffer를
직접 `DrawIndexedInstanced`로 제출한다. 생성된 geometry, 0.25 pixel 오차 기준과 material admission을
유지하며 invalid/near 입력은 원본 geometry로 되돌린다. 동일 선택 fixture와 직접 draw 비교를 먼저 확인한 뒤
작은 draw의 기존 dispatch 상환 한계를 제거한다. 여러 instance를 감싼 sphere가 LOD를 막는 경우에는
각 instance의 보수적인 투영 상한으로 동일 품질 한계를 유지한다.

새 영구 C++ 파일은 만들지 않는다. 제거된 compute 소비자가 확인되면 해당 shader의 프로젝트·filter·배포
등록만 정리한다. map/mesh 경로 공유로 베른 외 맵에도 같은 조건에서 적용하며 개선 FPS를 미리 약속하지 않는다.

## G04. 계측 후보 전수 조사와 검증

nav·Server tick·network·map cull·mesh/instancing·shadow·light/prebaked·material·animation·particle/pool·
worker·메모리·IO·ImGui·Present를 실제 함수와 기존 계측에 연결한 가이드를 작성한다.
Release 단독 실행 기준 반복 A/B와 Debug 정확성 검사를 분리한다. DOD/AoS/SoA/worker/fiber 효과는
동일 결과·동일 데이터·동일 compiler 조건의 kernel 비용과 전체 프레임 critical path를 각각 측정한다.

수정 소스의 인코딩을 유지하고 정상 Product Debug/Release Build, 관련 CPU/LOD/JSON focused 검사,
XML parse 및 diff-check를 수행한다. 다른 세션의 같은 출력 빌드와 겹치지 않는다.
실행한 검사만 RESULT에 기록하며 사용자의 실제 캡처와 화면 판정 전 FPS 개선·visual PASS로 기록하지 않는다.

## G05. ServerNavigation::Find_Path scratch 재사용

추가 조사에서 매 A* 요청마다 grid 전체 costs/parents/closed 배열을 할당·초기화하는 경로를 확인했다.
함수 내부 thread-local scratch와 query generation으로 실제 방문 cell만 초기화하는 후보를 기존 navgrid와
동일 시작/목표 입력으로 비교한다. 재진입에는 별도 임시 scratch를 사용하고 region dispatch는 기존대로 유지한다.
경로·실패 결과·heap tie 순서·Server 권위가 같고 유의미한 비용 감소를 확인한 뒤 해당 함수에 한정 반영한다.

## G06. 인스턴스 입력과 중복 재질 API 호출

추가 요청에 따라 SR의 atlas·월드 정점 bake와 실제 단일 draw를 현재 Bern 배치 키와 대조한다.
설치된 Bern에서는 lightmap/RNM·static shadow 텍스처 차이가 material variant를 나누므로
asset key만 느슨하게 바꾸지 않는다. texture array와 spatial chunk의 큰 구조 변경은 정확한
동일 재질 조건·메모리·컬링 손익을 확인하는 별도 후보로 문서화한다.

즉시 적용 가능한 범위는 `MapAssetRenderUtils` 재질 binder에 명시적인 instanced 입력 방식을
전달하는 것이다. `MapStaticBatchObject`의 실제 instance vertex에 이미 있는 lightmap/average/
directional/static-shadow scale-bias를 비인스턴스 global로 다시 설정하는 호출과 해당 instanced
shader에 없는 character 전용 reset만 생략한다. 일반 object와 preview의 기본 경로는 유지한다.
실제 HLSL의 소비 경계를 확인하고 baseline/candidate의 재질 입력·수치 출력 및 API 요청 수,
Debug/Release 최소 컴파일을 비교한 뒤 반영한다. 동작을 추측한 전역 state cache는 추가하지 않는다.


## G07. 화면 밖 이펙트·광원 제출 확인

발탄 이동 표식은 이미 final-camera sample culling을 사용하지만 8m bound에 16/32m padding을
더한다. Bern의 91개 ambient SOURCE_LOOP placement는 거리만 제한하며 화면 밖에서도 연산한다.
실제 trigger 권위와 표현을 분리한다. Marker는 기존 history 재진입을 유지한다.
LEVEL_ACTIVE SOURCE_LOOP 환경 표현만 정적 source bound가 증명되면 화면 밖에서 시각적 시계를
멈춘다. 입자·RNG 상태는 보존하되 숨은 wall time은 따라잡지 않으며, 재진입 때 현재 dt만 전진한다.
따라서 화면 밖에서도 계속 재생한 경우와 ambient 위상은 달라진다. Source feature 미지원,
외부 owner/anchor/control, 이동한 root는 제외한다. 레벨명별 정책은 만들지 않는다.
기존 marker의 자동 중복 Update는 없다는 코드 확인과 실제로 남은 숨은 작업을 구분한다.

Light_Manager의 Render_Lights 제출 경계에서 최종 view/projection으로 한 번 만든 보수적 clip
plane과 point/spot 영향 sphere(position, range)를 검사한다. directional·camera 경계 교차는
유지하고 invalid 입력은 fail-open한다. transient 원본 목록/forward shader 입력이나 shadow
caster는 변경하지 않는다. profiler에 후보/제외 건수를 추가해 업로드·draw와 함께 비교한다.
실제 shader의 유한 range 의미, 경계/뒤쪽/대형 광원 포함 및 source 순서·batch 경계를 검증한다.

## G08. 2026-10-01 성 내부·도서관 Release 재측정

사용자가 저장한 `릴리즈_4k_성내부_20261001_005735_196_frame677_28900_0.json`과
`릴리즈_도서관_20261001_010211_148_frame7471_28900_1.json`을 고정 경로로 분석한다.
전자는3840×2160/34.394ms, 후자는2560×1440/48.933ms이며 같은 해상도의 A/B가 아니다.
ImGui 부모·자식 scope의 중복 합산을 피하고, 열린 F7 패널·숨긴 도구·OS background를 구분한다.
다른 세션의 Debug shader Build와 실행 중 Release Client/Server를 보존한다.

## G09. PlayerController.cpp의 이동 피킹 시점

현재 hold 경로는 GPU 피킹 후50ms 재전송 제한을 검사한다. 기존 전송 시간 조건을 피킹
앞에서도 확인해 아직 보낼 수 없는 hold frame의 readback을 생략한다. 새 물리 클릭은 즉시
현재 커서의 exact pixel을 읽으며, 목표 deadzone·drift·typed sink·Server 권위는 유지한다.
Character.cpp와 LocalMovePrediction의 기존 다른 작업 및09/24 지연 snapshot 수정을 보존한다.
실제 함수 기반 fixture로 첫 클릭·빠른 두 번째 클릭·hold interval·실패 재시도와 기존 전송
판정의 동등성을 확인한다. 현재15~30FPS 자체로 과거 RESET 결함이 재현됐다고 단정하지 않는다.

## G10. Profiler의 실행 조건과 제품 UI 명칭

ProfilerCaptureIO의 기존 export-time metadata에 창 focus/minimized와 전경·배경 FPS 제한을
추가해 두 PC 비교 시 제한·background 상태를 식별한다. 과거120frame 전체의 상태로 해석하지
않도록 기존 sampledAtExport 계약을 유지한다. 실제 ImGui가 아닌 Party/Chat scope는
`UI.Runtime.Party.Update`, `UI.Runtime.Chat.Update`로 정정하고 profiler catalog도 맞춘다.
JSON 출력·UI 숨김과 Capture 수명은 유지하며 metadata 직렬화의 focused 검사를 수행한다.

## G11. Light_Manager.cpp의 일반 local light 영역 제한

기존 Deferred shader의 local-light AABB clip은 source-character pass에서만 활성화된다.
Renderer는 일반 pass 전에도 실제 camera 행렬을 bind하고, 일반 point/spot의 모든 출력도
range attenuation에 곱해진다. 기존128-byte record의 flags[3]를 일반 point/spot에도 활성화하는
후보를 검사한다. directional은 기존 fullscreen을 유지한다. near-plane·invalid 입력의 기존
fail-open, source 순서·blend·shadow·렌더 설정과 shader ABI를 유지한다.
기존 light parity probe를 재사용해 실제 Deferred point/spot의 clip OFF/ON 출력을 비교하고,
통과한 범위만 적용한다. 설치 입력·조명 수치·FXAA/SSAO 등 팀장 설정은 수정하지 않는다.

## G12. 이번 후속의 검증과 남은 경계

수정 파일의 인코딩을 보존하고 소유 diff를 검사한다. 실행 중 다른 Build가 끝나기 전에는
동일 checkout Product Build를 시작하지 않는다. 준비된 소스는 정상 Release Product Build로
검증하며 실제 EXE/DLL 잠금이 링크를 막으면 사용자 종료가 필요한 정확한 경계를 보고한다.
독립 수치 fixture·컴파일·제품 배포·사용자 FPS와 이동 화면 판정은 RESULT에서 구분한다.

## G13. 40FPS 재클릭의 위치 보정과 정상 지면 연속성

재부팅 후 사용자가 같은 방향 우클릭 두 번의 보정을 요청했다. 실제 header와40FPS/30Hz
프레임 순서를 재생하면 일정 frame time의 재클릭만으로 RESET은 발생하지 않는다. 그러나
stall 뒤 snapshot projection과 잔여 보정이 더해져 총 표시 속도가 이동 속도의 두 배가 된다.
`LocalMovePrediction.h`의 최종 XZ 표시 이동에 실제 유효 elapsed 기반 frame budget을 적용한다.
지속적인 낮은 FPS에서 영구 지연을 만들지 않도록 보정 경로에 고정100ms cap을 적용하지 않는다.
오래된 snapshot은 기존350ms freshness timeout으로 정지하고 pending local path의 기존 cap은 유지한다. 이동 중 서버 위치에 수렴할 작은 catch-up 여유는
명시적인 presentation 상한으로 두며 서버 속도·권위 위치·teleport RESET은 바꾸지 않는다.
동일 시각의 재호출로 추가 예산이 생기지 않게 하고 ACK 직후 위치 연속성을 유지한다.

별도로 Bern의 실제 navigation이 허용하는0.098335m 수평 이동과0.999962m 지면 상승을
기존3D continuity가 RESET으로 오판함을 확인했다. 기존 XZ 이동 한계를 먼저 검사하고,
수직 변화 때문에만 실패할 때 Character가 제공하는 navigation 검증을 요청한다.
`CNavigation`은 동일 detail owner, 양끝의 실제 지면 높이, 기존 segment walkability를 확인하는
read-only 질의만 추가한다. A*나 또 다른 이동 경로는 만들지 않는다. navigation 검증 실패,
다른 층·영역, 수평 teleport, 과도한 시각 위치 오차는 기존 RESET을 유지한다.

`Character.cpp`는 기존 navigation을 validator로 연결하고, 순수 helper 회귀는 기존
`ClientPresentationPrimitiveContractTests.cpp`에 둔다. 새 제품 파일이나 project/filter 등록은 없다.
40FPS 재클릭·stall·이동 중 수렴·동일시각 재호출·정상 지형/실제 teleport·입력 거부·timeout을
검증한다. Engine/Client 최소 Release 컴파일과 표준 Product build를 구분해 기록한다.

## G14. 재부팅 후4K 렌더링 판정

`베른성_20261001_012336_905_frame369_32536_0.json`을 분석한다. 저장 시점 해상도는
3840×2126이며120frames의 평균25.579ms다. 전체 draw와 map submesh draw, 등록 placement와
visible counter를 구분하고 GPU NonBlend/Lights와 PS/VS workload를 함께 해석한다.
material별 GPU 비용이나 동일 장면 해상도 A/B가 없는 상태에서 메시 개수만을 원인으로
확정하지 않는다. 현재 렌더 옵션·LOD 품질·저작 데이터를 바꾸지 않고 다음 최적화 대상을 좁힌다.
G11의 일반 local light clip 후보는 이번 후속에 반영하지 않는다.

사용자가 추가 저장한 `발탄_레이드_4k_20261001_014417_488_frame863_11668_0.json`도 같은 기준으로
분석한다. 움직이는 camera 구간은 평균 하나로 판정하지 않고 시간 구간별 draw/visible와
NonBlend/Blend GPU 시간을 대조한다. 공통40FPS라는 체감만으로 동일 병목이나 frame cap을
확정하지 않으며 이번 prediction 수정 전 바이너리의 성능 자료로 구분한다.

## G15. Character 갱신과 ACK 적용 사이의 보정 시간 손실

사용자의60FPS 추가 보고로 Object Update→Level snapshot 적용 사이의 실행 시간을 포함해
재생했다. 기존 helper는 이전 표시 pose를 늦은 wall-clock now에서 다시 보정하기 때문에
60FPS/30Hz/8ms phase에서 속도가0.6842↔1.3158배,15ms에서는0.1818↔1.8182배로 반복된다.
G13의 총 속도 상한만 적용하면 빠른 frame만 잘라 일부 phase에서 서버 위치를 계속 놓친다.
따라서 이 상태의 후보를 제품 완료로 취급하지 않는다.

`LocalMovePrediction.h`에서 network freshness·RTT용 wall clock과 화면 이동·보정용 frame clock을
분리한다. 새 Update는 Engine이 전달한 delta를 한 번 소비하고 동일시각 재호출은 소비하지 않는다.
snapshot의 투영과 잔여 오차는 같은 presentation clock에서 시작해 이후 한 frame 전체를 전진한다.
ACK 즉시 조회는 기존 표시 위치를 보존한다. `min(frame delta, Object 호출 wall 간격)`은 Object
위상이 흔들릴 때 시간을 잃으므로 사용하지 않는다. Server tick/권위/packet과 Engine update 순서는 유지한다.

0/8/12/15ms phase와 교대 Object jitter,40FPS 빠른 재클릭, 지속5/8FPS, 양수 감속/정지,
짧은 waypoint, 실제 teleport와 timeout을 대조한다. 새로운 시간 처리가 검증된 후에만
최종 Character 컴파일과 Release Product build를 수행한다.

## G16. 재질 분기와 Effect 중복 scene snapshot 제거

현재 GPU 비용에서 출력 계약을 유지한 채 생략 가능한 두 경로만 먼저 적용한다.
`Shader_MapMaterialSurface.hlsli`의 Source BG detail-normal과 specular texture 선택은
재질 공통 flag를 삼항식으로 검사해 OFF에서도 FXC가 sample을 실행한다. 기존 fallback을
먼저 대입하고 `[branch] if` 안에서 ON의 원래 sample·계산만 수행한다. 현재 shader 사용자와
실제 프로젝트 profile/최적화 옵션을 컴파일하고 DXBC에서 OFF 경로의 sample 생략을 확인한다.
재질 flag·texture·quality·map 데이터는 변경하지 않는다. GPU ms 개선은 캡처 전 확정하지 않는다.

`CEffectObject::Submit_RenderGroups`의 pre-BLEND snapshot 요청은 실제 occurrence의 scene-color
소비 직전 refresh가 덮어쓰는 경우에만 제거한다. 모든 live scene-color 소비와 deferred/mask
경로를 대조하고, MapAssetObject/WorldSequenceObject의 요청과 occurrence별 HDR/Bloom 복사는
유지한다. 다른 요청자가 없는 frame에서만 초기 full-resolution 복사 두 번이 줄어드는 경계다.
Bloom OFF의 shader 재평가·Bloom 복사 전체 제거는 native material의 clip/coverage 동등성이
불충분하므로 적용하지 않는다. 변경 CPP의 독립 Release 컴파일과 전체 diff-check를 수행한다.

사용자가 최종 소스 반영 뒤 직접 Release를 빌드해 화면을 확인하겠다고 했다. 실행 중인
Client/Server를 종료하거나 조작하지 않고, 소스 검증과 사용자 최종 제품 빌드를 구분한다.
## G17. 쿠크 추가 캡처와 최종 인계 범위

사용자가 추가한 쿠크02:15 캡처도 같은 GPU 유효 분모와 시간 구간별 지표로 분석한다.
52회 조명 제출을 고유 조명/메시 수와 구분하고 실제 CMaterial row 소비·기존 early discard를
확인한다. 실행 파일의 수정 포함 여부와 동시 컴파일을 대조하여 수정 후 성능으로 오인하지 않는다.
최종 소스의 실제 최소 검증 결과와 사용자 normal Release Product Build/화면 판정을 분리한다.
## G18. 사용자 요청 Debug·Release 전체 Product 빌드

최종 소스로 사용자가 Debug와 Release 전체 빌드를 요청했다. 실행 Client/Server와 다른 빌드가
없는 상태에서 정본 Product runner를 Debug→Release 순서로 실행한다. Engine/Shared/Server/Client,
실제 FX 컴파일과 표준 SDK/CSO/DLL 배포를 포함하며 SkipBuild/SkipFX/Clean은 사용하지 않는다.
각 구성의 결과 JSON과 변경 입력 의존성·산출물 배포를 확인하고, 실패가 있으면 해당 원인을
수정한 뒤 필요한 빌드만 다시 수행한다. Client 실행과 화면 검증은 포함하지 않는다.

## G19. 이동 명령·Server 시간·연속 표시의 분리

지속적인 150ms 재클릭은 G15의 frame clock만으로 끝나지 않는다. MOVE ACK마다 다시 잰
RTT를 투영 lead로 사용하면 입력 빈도가 화면 속도를 바꾸고, 최신 snapshot의 음수 age를
0으로 자르면 40FPS와 30Hz 사이의 수신 위상이 보정 목표를 흔든다. 같은 실제 직선 경로에서
한 번 클릭과 연타의 각 frame 변위가 같고, 두 경우의 정상 속도도 유지하는 것을 종료 조건으로 삼는다.

`Client/Public/LocalMovePrediction.h`의 `SubmitMove`는 command sequence와 목표를 갱신한다.
ACK 대기와 실제 local path의 sequence/시작 시간을 분리해 같은 목표의 재클릭이 경로를 다시
만들거나 원래 ACK·timeout horizon을 연장하지 않게 한다. 다른 거리의 동일 직선 목표도
presentation 시간과 이미 남은 correction을 초기화하지 않는다. 개별 MOVE RTT lead는 제거한다.

Server tick을 wrap-safe 누적 시간으로 펴고 수신 시각과의 관계를 관찰한다. GameRoom의
UINT_MAX→1 전이는 예약 tick0을 더하지 않는다. 표시 시간은 Engine delta를 frame당 한 번
진행시키며 최초 양수 delta에서 기준점을 잡아 packet 조회만 한 시간 구간을 누적하지 않는다.
수신 batch는 시계를 여러 번 전진시키지 않고, 짧은 window의 최소 수신 offset과 제한된
slew로 실제 지연 변화에 적응한다. sub-tick 수신 양자화는 매번 새 지연으로 해석하지 않는다.
이 관측으로 절대 one-way 지연을 측정했다고 표현하지 않는다.

최신 snapshot이 표시 시각보다 앞서면 이전 sample과 같은 Server 시점을 구한다. 알려진
corner는 이전 waypoint를 포함한 두 segment로 샘플하고, 표시 위치도 corner를 먼저 지난 뒤
잔여 frame 이동량으로 다음 segment를 소비한다. 이전/현재 속도와 새 segment 방향으로
서버가 실제 지난 corner인지 확인하며, 실제 새 경로·역방향 ACK에는 오래된 corner를 폐기한다.

`Character.cpp`의 `Predict_NetworkMoveGoal`는 동일 목표에 기존 path를 보존하고, 새 경로는
기존 follower로 stage한 뒤 제출 성공 시 교체한다. `Update_LocalMovePrediction`은 helper의
공통 Server-clock horizon 안에서 follower를 한 번 진행하고 `CompleteLocalPathFrame`으로
최종 보정·속도 budget을 한 번 소비한다. 실제 새 방향이 아직 승인되지 않은 짧은 동안만
이전 segment의 보정을 보류하고, ACK·timeout·forced state 후에는 서버 정본으로 돌아온다.

새 제품 파일·protocol·Server 권위·Engine 실행 순서는 추가하거나 바꾸지 않는다. 기존
`ClientPresentationPrimitiveContractTests.cpp`에서 40/60FPS, 수신 phase, ACK jitter,
150ms 연타, 다른 거리의 같은 직선 목표, 실제 지연 변화와 packet batch, 90도 turn,
body block, stop, fast ACK corner 전환, tick wrap, 긴 hitch/teleport와 속도 budget을 검사한다.
평균 거리만 보지 않고 클릭·ACK 직후를 포함한 모든 frame을 single-click trace와 비교하며
steady 속도 자체도 ±0.5% 이내로 검사한다. native Debug/Release, 제품 빌드, 사용자 화면
판정은 RESULT에서 각각 구분한다.

## G20. 최신 빌드 이동의 실제 좌표·보정 계측

사용자는05:40 이후 최신 Release에서도 이동이 끊긴다고 확인했다. 구형 Release 캡처만으로
현재 원인을 대신 설명하지 않는다. 다른 통합 작업은 CPU 지형 피킹을 소유하고 있으며,
이번 범위는 Character의 실제 이동 소비와 기존 F7 Capture의 좌표 증거다.

`ProfilerCaptureIO.h/.cpp`의 기존 저장 문맥에 이동 표본을 추가한다. 표본은 frame 이동,
snapshot 적용, 목표 제출의 종류와 QPC 시각, 전후 위치, frame delta, Server tick·ACK,
snapshot 위치·waypoint와 적용 결과를 보존한다. 고정 상한의 Client main-thread ring만
사용하고 Capture OFF에서는 QPC·할당·파일 쓰기를 하지 않는다. `ProfilerTool.cpp`는
기존 Capture 시작에 ring을 비우고 저장할 완료 frame들의 QPC 범위만 불변 vector로
복사하여 기존 background exporter에 넘긴다. 누락·범위 밖 표본을 완전한 자료로 표현하지 않는다.
Engine gameplay 타입, 새 저장 스레드, 별도 프로파일러, project 등록은 추가하지 않는다.

`Character.cpp`는 실제 입력·snapshot·한 frame 최종 표시 위치에 위 표본을 연결한다.
이동 제어 상태나 수식을 계측 때문에 바꾸지 않는다. 같은 최신 helper에 실제 Character 호출 순서,
측정한 가변 frame 간격과30Hz Server 입력을 넣어 입력 전후의 좌표를 비교한다. 재현된 결함의
최소 수정은 실패 fixture와 함께 기존 영구 회귀에 넣고, GPU 대기와 별개인 것으로 기록한다.

검증은 표본의 유한 값·bounded ring·Capture OFF·완료 frame 범위·JSON parse·비동기 저장,
변경 CPP 최소 컴파일과 사용 가능한 Release Product Build다. 실행 중 Debug Client를 종료하거나
화면을 조작하지 않는다. 실제 새 Release/Debug 동일 장면의 조작감은 사용자 판정으로 남긴다.

## G21. 재클릭 전후 이동 시간과 경로 교체의 원자성

`CCharacter::Update_LocalMovePrediction`은 `CLocalMovePrediction`이 freshness와150ms
extrapolation horizon으로 계산한 시간을 다시100ms로 제한한다. 반면 ACK 이후 helper는
같은 horizon 안의 전체 시간을 소비한다. 따라서132.4ms frame의 미승인 재클릭은100ms만
진행한다. 이를 평균 FPS나 Release compiler 문제로 부르지 않고 실제 소비자 불일치로 수정한다.

- `Client/Private/Character.cpp`: `Get_LocalPathDeltaSeconds()`를 기존 follower에 그대로
  전달한다. freshness350ms, extrapolation150ms, 최종 표시 속도 budget과 Server 권위는
  `CLocalMovePrediction`이 계속 소유한다. 모든 frame delta에 무조건 큰 값을 적용하는 변경이 아니다.
- `Engine/Private/NavPathFollower.cpp`: 2cm 이내 waypoint로 무료 이동하는 분기를 제거한다.
  남은 segment가 짧더라도 실제 거리를 frame 예산에서 차감하고 도착 시점에만 다음 waypoint로
  진행한다. 새 경로/physics runtime을 만들지 않는다.
- `Server/Private/GameRoom_PlayerCommands.cpp`: `Commit_MoveGoal`의 경로 검색은 임시
  waypoint vector에 수행한다. 실패한 새 목표가 진행 중 `MovePath`, index, request/goal을
  지우지 않게 하고, 성공한 경로만 기존 player에 commit한다. 직접 시야 이동과 같은 목표
  재전송의 기존 빠른 경로, 처리 sequence ACK와 skill-cancel 권위는 유지한다.

실제 `CNavigation`, `CTransform`, `CNavPathFollower`를 사용한 화면 없는 native probe로
16/25/50/132ms와 ACK 전후·corner·정지를 확인한다. header만 모사한 straight follower를
제품 통합 증거로 쓰지 않는다. Server는 기존 gameplay contract test에서 유효 이동 중 실패
retarget의 이전 상태 보존, 처리 sequence 진행, 후속 유효 목표 교체와 실제 이동을 검사한다.
새 제품 H/CPP와 프로젝트 등록은 필요하지 않으며 기존 테스트 소비자에 회귀를 둔다.

최소 컴파일과 Product Debug/Release 링크는 소스 반영 뒤 구분해 기록한다. 다른 세션의
protocol132 병합·제품 빌드와 같은 출력 경로를 동시에 쓰지 않는다. 이 변경이 최신 Release의
모든 끊김을 해결했다고 선행 판정하지 않고 G20의 실제 이동 표본과 사용자 재현으로 닫는다.

## G22. 이동 피킹의 GPU 의존 제거 수용 조건

현재 통합본의 비동기 GPU 피킹은 주 스레드의 `Map(READ,0)` 대기를 제거하지만, 입력 명령이
GPU 결과가 준비되는 frame까지 지연되는 구조는 남는다. 최종 이동 목표는 클릭 시점의 카메라
ray와 CPU의 월드 입력으로 같은 frame에 확정하고 typed command sink로 제출한다.
Desktop의 기존 Engine/Client 파일에서 구현을 통합하고 다음 조건을 같은 변경에서 검증한다.

- `Mesh.h/.cpp`는 기존 불변 `PICK_GEOMETRY`에 triangle ordinal과 preorder BVH node를
  보관한다. prototype 로드에서 한 번 만들고 clone이 공유한다. `Model.h/.cpp`의
  `Try_PickStaticSurface(meshIndex, world, ray, maxDistance, cullMode, distance)`는
  affine inverse와 determinant를 검증해 월드 거리와 실제 cull 정책을 보존한다.
  기존 `Try_PickCurrentPose`의 정적 분기도 이 가속 구조를 재사용한다. 클릭 경로에서
  asset load/정점 복사/전체 geometry 재구축이나 query별 heap 할당이 발생하지 않아야 한다.
- 기존 `MapPlacementRuntime`과 `CModel -> CMaterial` 소비 경로를 확장한다. runtime visible,
  instance visible뿐 아니라 Stage/CameraPreview suppression, 파괴·숨김·level lifetime을
  실제 렌더 소비자와 대조한다. 숨긴 상층 geometry가 hit를 가로채면 안 된다.
- ray는 해당 입력 occurrence의 viewport/view/projection을 사용한다. 겹친 층·계단·가림·
  음수 scale·LOD·배경 제외 scope를 검사하고, alpha-tested material의 정확한 시각 hit와
  이동용 surface 계약을 혼동하지 않는다. 전역 shader/quality 옵션은 변경하지 않는다.
- Server MOVE 계약은 XZ이고 Client/Server navigation은 시작 위치의 층으로 경로를 찾는다.
  시각 hit의 Y만으로 다른 층 진입을 승인하지 않는다. 표식 Y와 실제 navigation 목적지의
  관계를 검사하며, Client의 일부 navgrid만으로 Server가 허용할 목표를 거부하지 않는다.
- CPU miss/데이터 미준비를 동기 GPU 피킹으로 되돌리지 않는다. 실패한 입력은 기존 이동을
  유지하며 새 클릭·hold·버튼 release·UI 소비·free camera·level 교체를 각각 확인한다.

`MapAssetObject`는 현재 world AABB, `MapStaticBatchObject`는 현재 instance별 보수적
sphere를 먼저 검사하고 각 material mesh에 `Select_Pass`의 BACK/FRONT/NONE 정책을
전달한다. `MapPlacementRuntime`은 현재 소유한 두 집합에서 최단 hit만 합친다. 기존
프로토타입과 instance가 정본이며 별도 placement/transform cache를 만들지 않는다.
masked 바닥과 cardmaze floor receiver를 포함하고 명시 foliage/grass·character 변형 재질과
morph를 제외한다. alpha texture 구멍, GPU vertex 변형, 렌더 LOD와의 시각 일치는 별도 경계다.

`PlayerController.h/.cpp`의 `MOVEMENT_SURFACE_RESOLVER`는 Level 수명 안의 map query를
주입한다. Bern/Valtan/Kouku/CharacterSelect/Development의 기존 controller 생성 지점에서
연결하며, Development가 Maharaka/Colosseum/Training의 동일 경로를 담당한다. 기존 GPU 요청
ID·후속 poll·취소용 이동 상태는 없애고 query 성공 frame에 기존 typed sink로 제출한다.
배 이동은 기존 수면 plane을 유지한다. 새 제품 C++ 파일이 없어 vcxproj/filters 등록은 없다.

검증 결과에는 map 크기, 후보/triangle 수, 클릭 비용의 p50/p95/max, 이동 command 제출 frame,
GPU readback 호출 수와 실패 이유를 남긴다. CPU 피킹 구현·빌드·설치 및 사용자 화면 확인을
따로 기록하며 비동기 GPU 적용만으로 이 G의 완료를 선언하지 않는다.

hold의 표면 검색 간격은 마지막 packet 송신 시각과 분리해50ms로 제한한다. 동일 목표와 miss도
검색 시각을 갱신하고 새 press는 즉시 검색한다. batch bounds는 기존 cache가 clean/valid일 때만
전체 배제에 쓰고 dirty이면 개별 instance를 검사한다. query 중 bounds 재구축은 하지 않는다.
실제 Bern 배치 bounds와 원본 geometry로 비용·할당을 검증하며 broadphase 수치와 전체 입력
frame 비용을 구분한다. synthetic 전체 중첩 입력의 비용도 남겨 무조건적인 성능 보장을 피한다.


## G23. 08:23 베른 캡처의 동일 조명 재질 행 통합

최신 `베른_33fps_20261001_082343_324_frame214_62428_0.json`은 Release/RTX4070,
3840×2126에서 평균30.565ms다. 유효 GPU116frame의 Lights14.838ms가 우선 병목이며,
현재 카메라·게시 광원으로 기본2회 + source 재질65행×11회 =717draw를 재구성했다.
일반 local light의 화면 영역 제한은 이 카메라의 유일한 ALL 광원이 near-plane에 걸려
fullscreen fallback이므로 이번 변경으로 선택하지 않는다.

`Engine/Private/Material.cpp`의 기존 frame registry 안에서 동일한 조명 입력만 같은 행으로
연결한다. program, baked 여부, lightTextureMask, 전체 lightConstants의 비트와 실제 사용
texture override 적용 후 SRV가 모두 같은 경우에만 공유한다. 각 원래 재질의 base constants,
base textures와 GBuffer 계산은 그대로 실행하며 셰이더·광원 순서·화질 설정·Resources는 유지한다.
포인터별 lookup은 모든 alias의 shared_ptr까지 보유해 기존 CModel의 copy-on-write와 수명을
유지하고 Reset에서 해제한다. 실패한 base bind는 alias 등록을 하지 않는다. forward 재질은
기존대로 deferred registry에 들어오지 않는다. 공통 CMaterial 경로이며 베른 이름 분기는 없다.

기존 `Tools/RenderingPipeline/SourceCharacterShaderVariantProbe.cpp`와 runner의 선택 검증으로
동일 clone, base-only 차이, 조명 상수·program·texture 차이, 실패·Reset·alias 수명과 다수 행을
확인한다. 제품 C++ 신규 파일·public API·project/filter 등록은 없다. 인코딩을 유지하고
동일 설정의 정상 Release Product Build로 DLL/EXE를 연결한다. 다른 채팅의 폰트/UI 수정과
동일 출력 build가 겹치지 않는지 확인한다. 캡처의 실제65행이 줄어든 수와 개선 FPS는 새 사용자
캡처 전에는 확정하지 않으며 headless 수치 검증·빌드·사용자 화면을 분리해 RESULT에 기록한다.
