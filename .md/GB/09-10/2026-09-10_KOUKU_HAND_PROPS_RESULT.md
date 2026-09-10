# 쿠크 손 소품 월드오브젝트와 패턴 연결 결과

## G00. 저장한 데이터

다른 진행 작업의 Composition240을 받아 기존 항목을 보존하고 revision241로 저장했다.
Area WorldSequence는 revision423에서424로 올렸다. 기존 Object19/Template149/Instance185와
기존 Composition world13개는 모두 보존했고 각 목록에4개씩 추가했다.
P21/P23은 WORLD occurrence와 next ordinal만 변경했다. 다른 작업의 P24·Logic38/39도 보존했다.
검증 근거와 초기 원본은 `out/KoukuHandProps20260910`에 있다.

| 표시 이름 | 부착 대상 | 실제 본 | 연결 시퀀스 |
|---|---|---|---|
| 월드오브젝트_쿠크트럼펫 | 2관문 Kouku | 오른손 b_wp_1 | P23 쿠크_팡파레, 0~10667ms |
| 월드오브젝트_쿠크레이저대포 | 2관문 Kouku | 오른손 b_wp_1 | P21 쿠크_레이저, 0~5833ms |
| 월드오브젝트_세이튼총_왼손 | 3관문 Saydon | b_wp_2 | Object/기본 상태만 준비 |
| 월드오브젝트_세이튼총_오른손 | 3관문 Saydon | b_wp_1 | Object/기본 상태만 준비 |

쇼타임은 원본 Action4219939의 Gun1/Gun2가 같은 StaticMesh를 양손에 붙인다.
미구현 쇼타임 패턴은 만들지 않았다. 생성 가능한 Object 정의와 미구현 패턴을 구분한다.

## G01. 리소스와 좌표

Resources 기준 새 cooked 모델3개, 총626,364 bytes를 설치했다.

- `Effect/KoukuSaydon/WorldObjects/Trumpet/Trumpet.wmodel`
- `Effect/KoukuSaydon/WorldObjects/LaserCannon/LaserCannon.wmodel`
- `Effect/KoukuSaydon/WorldObjects/SaydonShowtimeGun/SaydonShowtimeGun.wmodel`

기존 `fm_g_reup_01`, `fm_g_rhkp_06`, `wp_mn_rpct_07l_mesh`의 geometry를 보존하고,
원본 material의 D/N, D/N/S/E, D/N/S를 각각 CMaterial에 연결했다. 기존 Effect 모델과
texture9개는 수정하지 않았다. 세 모델은 Git 제외이며 Drive 업로드는 실행하지 않았다.
다른 PC에는 위 세 모델과 기존 texture 입력이 함께 있어야 한다.

prescale은 모두0.01, 소품 scale은 트럼펫2.5/대포2/총1.5다.
트럼펫·대포 local quaternion은 `[-.5,-.5,.5,.5]`, 총은
`[-.4156269364,-.4156269364,.5720614038,.5720614038]`, 총 local offset은 `[0,0,-.1]m`다.
원본 mesh의 UE→cooked 축과 skeletal socket의 local 축을 실측해 합성했다.
본 행렬의 위치는 유지하고 basis를 정규화하여 소품의 모델 단위가 중복 적용되지 않게 한다.
원작 PBR 전체식과 발광 flicker까지 복원했다고 판정하지 않는다.

## G02. 기존 런타임 확장

WorldSequence v3 부모 resource에 `anchorKind=BOSS`, `anchorBossArchetypeId`, `anchorBone`을 저장한다.
상태 instance는 `anchorKind=BOSS`로 부모의 actor/bone을 소비한다. Composition World의
`anchorKind=NONE`은 기존 고정 생성 위치 보정을 쓰지 않는다는 뜻이며 실제 손 추적은 WorldSequence가 소유한다.
`BOSS_SPAWN`은 계속 생성 당시 위치이므로 손 부착에 사용하지 않는다.

기존 CWorldSequencePlayer/CWorldSequenceObject/CModel/CMaterial 경로를 확장한다.
Tool에서 Boss/실제 actor/BODY bone을 편집하고, Product는 살아 있는 복제 보스,
Model View는 선택된 preview actor의 본을 매번 샘플한다. 없는 보스는 숨긴 채 대기하고
없는 본을 root로 대체하지 않는다. 종료·Reset은 기존 occurrence 정리를 사용한다.

## G03. 검증 상태

- 기존 Object/Template/Instance 및 변경 대상 밖 Composition 내용 보존: 확인.
- 변경 JSON parse와 등록 stable ID/실제 리소스 경로: 확인.
- 새3모델 converter info, geometry 원본 유지, material 입력9개 존재: 확인.
- Map publisher 기존 BOSS/비보스 유효·실패 fixture: 통과.
- WorldSequences 범위 publisher Validate: 통과 (`map-validate.log`).
- 실제 C++ Validate/Save/Load: BOSS actor/bone 왕복, 누락 actor·과도한 길이/제어문자 bone·anchor 불일치 거부, legacy 기본값과 실패 시 기존 document 보존 모두 통과 (`codec-build-run.log`, failures0).
- Debug Product Engine/Shared/Server/Client 빌드와 Composition revision243 게시: 완료. 다른 작업의 최종 통합에서도 P21/P23 연결을 보존했다. `out/BuildPipeline/runs/20260910T052159567Z-debug-product.json`, `out/KoukuPublishIntegration20260910/publish243.log` 참조.
- 원본/게시 WorldSequences revision424 JSON deep equality, Composition/Encounter/patternbindings revision243 일치: 확인.
- Client/UI 실행·조작·캡처 및 손 정렬·색·크기의 최종 화면 판정: 미실행, 사용자 확인 대상.

## G04. 사용자 검토 경로

현재 PC는 LAN server-host다. 최종 빌드 이후 Visual Studio `Server + Client` profile을
Ctrl+F5로 시작한다. Lobby → KoukuSaydon → F1 → KoukuSaydon Complete Play에서
Gate 2의 `쿠크_팡파레`, `쿠크_레이저`를 각각 선택해 Complete Play한다.

편집은 F1 → World Object Tool → Object Resources → Boss에서 위 이름을 선택한다.
Action Workbench → Resources → World → Append Object로 같은 정의를 다른 패턴에 추가할 수 있다.
부모 Object의 Boss actor/BODY bone과 기본 상태의 Transform을 확인한다.


## 2026-09-10 최종 Product 통합 확인

사용자 마지막 Save/종료 뒤 최신 원본에 통합하고 관련 publisher와 정규 Debug Product 빌드를 완료했다.
Engine/Shared/Server/Client 컴파일·링크·EXE/DLL/셰이더 배포는 PASS이며 실행 입력 누락은0이다.
이 기록은 위의 Product 통합 대기 상태를 갱신한다. 세부 게시 revision·새 Server 검사·남은 사용자
화면 확인은09-10 KOUKU_PATTERN_EFFECT_ANCHOR_FEAR_AUTHORING_IMPLEMENTATION_RESULT의 G10에 있다.
빌드 근거: `out/BuildPipeline/runs/20260910T091016153Z-debug-product.json`. Client/UI 실행·캡처와 최종 육안 승인은 수행하지 않았다.
