# 가디언 검격·입자·바닥 재질 복구 결과

## 실제 반영

Trail/Decal의 native dispatch 상한 3967 때문에 Guardian3968 이후가 generic 재질로 빠지던
경로를 현재 registry와 연결했다. 49110 clip1의 4037은 generic 리소스가 없는 source decal이며,
이전에 흰 base/mask fallback을 소비해서 바닥 사각형이 되던 구체 사례다. 원본4010계열과
사용자 표현상 sprite를 구분했고 W 전체 alpha를 강제하지 않았다. 새 S native4552까지
수용하도록 carrier 및 installer 상한은4607이다. ALT V 후속 native4566도 같은 범위에 포함된다.

Masked LocalVF 4041/4088/4100/4147/4161/4264/4269/4295의 원본 CB0[0] RGBA를 복원했다.
4057의 WorldToLocal,4086의 world-origin/opacity,4119의 source camera,
4268의 world-origin/WorldToLocal,4275의 MacroUV/외부 opacity를 원본 PS/VS와 prefix로
검증해 공급했다. 4275 Required module의 source CDO MacroUV radius200cm도 해당 occurrence에만
반영했다. 13개 source PS의 UV0.zw를 기존 uv1에 연결했고 15 main/7 distortion 함수에 적용했다.
Trail tangentView/TBN은 실제 strip world/UV derivative로 계산한다.

D4156..4159는 floor receiver 경로와 Guardian rigid material84 배제로 플레이어 머리 투영을
제거했다. 마지막 LMB49000clip2 notify007/008의 wind/meshtrail7개만 사용자 요청 yaw90도로
수정했다. 이 값은 원본값 주장이 아닌 PROJECT_TUNED이며 dust/light와 R clip1은 보존했다.

## 검증 증거

증거 디렉터리는 `out/GuardianCarrierRepair20260922`다. 원본 DXBC와 installed native 함수를
WARP로 비교했다. masked32건 maxΔ0, engine-input20건 maxΔ0.000001073, trail72건 maxΔ0,
W28건 maxΔ0.000000775, 화신화 LMB80건 maxΔ0이다. 총232건이며 constant texture fixture의
equation/adaptor 대조다. 실제 화면의 형태·색 일치 증거로 대신하지 않는다.

W의 실제 DDS, CPU particle color/dynamic을 적용한 7×7 UV grid686건은 모두 finite였다.
4031의 큰 sprite(최대3.59×5.89m)는 source mask의 가장자리 alpha0을 유지한다.
4037 decal carrier의 잘못된 fallback과 원본 shader 계산 결과를 구분했다.

현재44문서1608elements23ModelCues40OwnerControls는 실제 Codec load/Drawable/roundtrip/
Stage/Seek0.05초 순회에 성공했다. 이것은 identity-root CPU 검증이며 bone 부착 화면 성공이 아니다.
추가 actual GuardianKnight.wmodel/preScale0.0001/bone pose 검증은 redLMB3clip+E1clip,
4문서102elements3ModelCues5OwnerControls를 통과했다. Bone basis의 기존0.01단위를 실제 소비했다.

화신화 LMB20program3978..4000과 E4057의 실제 DDS/CPU samples 4116건은 finite였다.
주요 화염sprite에 원본 mask가 유지된다. 후속 수명 검사에서3982는 초기 alpha0 뒤 최대1로
나타나며3979는 실제 mesh의 UV1 부재에 따른 UV0를 공급하면2205개 표본에서 alpha0~1이다.
identity/가짜 UV1의 alpha0을 실제 mesh 미표시 근거로 쓰지 않는다.4000은 additive alpha0이므로
alpha만으로 비표시라고 판정하지 않는다.

Trail/Decal 및 변경group mesh/particle FX10개 focusedcompile 성공. Source generator336개는
deferred0, distortion185개도 deferred0이며 installed exact함수 비교와 scoped diff check를 통과했다.
Product통합 build는 root담당 RESULT를 따른다. Client/UI 실행·캡처는 하지 않았다.

## 슬롯별 적용 범위

| 슬롯 | 확인한 복구 연결 | 남은 경계 |
|---|---|---|
| LMB49000 | 3968..71 검격 UV/carrier/TBN,3975/76 floor, 마지막 clip7개 yaw90 | 최종 방향·크기는 사용자 화면 확인 |
| 붉은 LMB49001 | 실제3clip/bone,20개 원본GPU80건,3975/3988/3989 decal진입,3979/3982 실제UV·수명 검사 | 최종 화염형태 사용자 확인 |
| Q49100 | 3968..71/4026..28 검격 | 시각 판정 대기 |
| W49110 | 4037 source decal fallback제거,4041 mesh mask,7 sprite DDS mask | 위로 올리는 clip2 위치는 사용자 조정 범위 |
| E49120 | 4057 WorldToLocal RGB,4058/59 trail UV/TBN | Tool/gameplay 차이 원인 아래 별도기재 |
| R49130 | clip2 4086 glass/world input,4088 rocks/masked,검격carrier | 정상 clip1 보존,최종 glass/rocks비교 대기 |
| A49200/49210 | 4100shield masked,4104/4117 floor sourcecarrier | 사용자 화면 확인 |
| S49220/49230 | 4119camera input,4126trailUV | 신규49290 대체는 별도 담당 RESULT |
| D49150 | 4156..59floor receiver,4147/4161rocks,검격UV/carrier | 실제 바닥 위치 사용자 확인 |
| F49260/49270 | 4177/78/4182/4186/4203trail 연결 | wing/requiredStance 별도 담당 RESULT |
| V49400 | 4255/56trail,4264/69/4295masked,4268world,4275MacroUV | 최종 distortion 시각 판정 대기 |
| ALT V49420 | projectile494200의58 elements·14 color programs·18 distortion companions 설치, 총136 elements | 카메라·배경·그림자 통합 결과와 사용자 화면 확인 |

E의 actual bone probe에서는 identity-root에서 누락되던 spine2 follow입자/리본이 실제 pose를
공급하자 생성됐다(4057최대156samples,4058/59edge70/40). 하지만 실제 Tool은 이미
Resolve_AuthoringSourceAnchors와 transform history를 공급하므로 이 probe 차이를 Tool버그
원인으로 단정하지 않는다. 4057WorldToLocal은 source RGB 경로만 바꾸고 alpha를 바꾸지 않아
Tool/gameplay의 opacity 차이 해결 증거로도 사용하지 않는다. root가 Play/시간 경로를 별도 확인한다.

## 미완료·판정 경계

ALT V projectile494200의 화염과 DragonDecal은 설치했다.3.79초 FireBreath01/Local,
+0.4초02/03/04,+0.6/1.2초noise,+1.4초DragonDecal은 원본 notify/timer를 사용한다.
color14×4=56건 원본DXBC 대조 최대오차1.90735e-6, distortion18×4=72건 최대오차0.00011003이다.
deferred0이며 새 리소스7개와 기존 texture closure를 검증했다. main document의 model5개 및
sceneBackdrop7개는 보존했다. 최신 codec/frame 검사에서 decal의 실제 Frame.Elements 출력도
확인했다. particle/trail만 세던 이전 sweep의0표본을 비표시로 해석하지 않는다.

신규 S의21color/22distortion 원본대조 및 실제14bone CModel 검사는
`2026-09-22_GUARDIAN_PREVIEW_PARTS_RESULT.md`에 기록했다.
사용자의 최종 화면 비교는 아직 수행되지 않았다. 이 문서는 모든 이펙트가 화면에서 완벽하게
복원되었다고 주장하지 않는다.
