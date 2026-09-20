# 쿠크·발탄 연출 복구 통합 결과

기준: 2026-09-20, `GB/koukubugfix-bingo`. 기존 작업과 사용자 저작을 보존했다. 아래 소스 반영은 실제 설치·컴파일·계약 검사를 구분하며 Client/UI 화면은 실행하지 않았다. 세부 원본과 수치 증거는 같은 날짜의 기능별 RESULT를 참조한다.

## G01. 요청별 현재 연결

| 요청 | 반영 내용 | 검증 경계 |
|---|---|---|
| G1 과광 | 두 scene alias의 실효 exposure 2→1, 머리 카드 9 leaf의 scene bloom 기여 분리 | source LUT/receiver/shader 분기 조사, V2 codec/shader 검사. prebaked·gamma 중복 확정 근거 없음 |
| G1 카드 | 1~4명 서로 다른 문양, 독립 RED/BLACK, 기존 유효 배정 유지, G2 연출 시작에 제거 | Server 카드 배정 경로와 반복 tick 테스트 추가 |
| 휠윈드 망치 | Pos 1.25/0/0, Rotation 0/180/0, Size 2/2/2. F1에 같은 Object draft의 TRS 편집 | 카드미로 연출과 별도 stable instance. 실제 파생 CModel bone 5시점 finite 검사 |
| 카드미로 플레이어 | 여섯 직업의 원본 LMB/Q와 손 본, 기본 무기 교체, F1 별도 손 기준 TRS | 원본 12 clip 실제 CModel 검사. FX 슬롯만 추가하고 사용자 저작 대상으로 비워둠 |
| Mario NPC | 망치 공격 원본 clip 마지막에 원본 Effect 1회 재생 | attack 변경·취소 시 미재생 cue 정리, 기존 NPC consumer |
| 칼날·갈고리 | Parent 중앙 Map XYZ 앵커, 즉시 preview, 일반/즉사 분리, finite 방출 중첩 반복, 저장 패턴으로 Server collider Play | Object/codec 29검사, collider Python 16검사. P95 즉사 칼날 시험 패턴 추가 |
| 크기 | Artist 1.6, DimensionMaster 0.7, 광기 광대 0.7 상대 배율과 F1 전체/class/변신 패널 | 실제 profile 저장·재열기·오류 보존 검사. Server collider는 기존 권위 수치 |
| 앵콜 세이튼 | G3와 같은 모델·지팡이, F1 yaw 저장/preview, 기존 G3 일반 공격 ID 재사용 | G3 전용 layout/Logic/World 패턴의 잘못된 빙고 실행은 거부 |
| 빙고 | 원본 1배 폭탄/심지/폭발, 망치 선행, 메두사 얼굴, 보드 원본 flip/무한 경계 | codec/CPU native playback 검사. 메두사 P94의 DRAFT 상태 유지 |
| 거미 피격 얼굴 | collider hit의 Darkness/Fear 원본 screen PS 12 emitters·크기 곡선 | 반복 cadence만 PROJECT_TUNED. source 원본 single pulse와 구분 |
| 차원술사 ALT V | 기존 캡처의 native178 UV 중심/crop 교정, Color/Bloom 동일 crop, 원본 액자 5종에 45도·방향별 수축 연결, 누락 원본 액자 2행 추가 | 캡처 GPU WARP 21,678검사·설치 액자 CModel 1,514검사·codec 164,942검사 PASS. 화면 합성 최종 판정은 사용자 확인 |
| G2/안개 | 특정 shot에서 캐릭터 directional만 복구, 활성 연출 camera 동안 fog 해제 | 종료 후 기존 region 복원 검사 |
| 발탄 | 원본 stage/animation 연출을 패턴 소비자로 연결, 파생 actor clip bake, 카메라·World FX, ghost 부활 cue/full restore join | source 정확성·actual CModel·presentation contract 검사. 52종/89개 FX 재생 구간 admission PASS, 원본 7구간 보류. 상세는 발탄 RESULT 참조 |
| 발탄 삼각형 | 지름 18→27m(1.5배), 이동 시간 유지하도록 속도 조정 | gameplay/publisher/reference 일치 검사 |
| Stage/row 독립(쿠크) | Stage 합계로 다음 패턴 진행, 이전 Effect/Sound/World/Logic/Summon은 원래 clock·소유권으로 자기 수명 유지 | 편집기 저장·재열기·확장과 실제 Server 지연 피해/World/Summon·자동 다음 Entry·GC pin·Stop/expiry 검사 PASS |

## G02. Stage와 row 수명 변경

`project_kouku_saydon_composition.py`와 C++ Composition expansion은 긴 row에 맞춰 마지막 Stage를 늘리지 않는다. World-only leaf는 기존 Stage의 길이를 보존해 idle 표현을 채우며, 임의 대기 Stage를 덧붙이지 않는다. `Set_PatternDuration`은 전체 수명만 바꾸고 `Set_StageDuration`은 다른 lane의 위치·길이를 보존한다. UI tooltip도 같은 의미로 교정했다.

게시 encounter의 optional `timelineDurationMs`는 bootstrap `PATTERNTIMELINE`으로 연결한다. presentation의 `durationMs`는 row lifetime, `stageDurationMs`는 Stage 합계다. 생략된 문서는 기존 Stage 합계를 사용한다. 자연 완료한 Client 세션은 핸들·원래 시작 tick·source definition을 보존하고, 명시 중단·수동 새 실행·사망·관문 변경은 잔여 세션을 정리한다. 자동 RaidFlow 다음 Entry는 같은 epoch와 immutable revision을 유지하고 이전 World/Logic/Summon 및 bundle common row의 남은 수명을 넘긴다. World `FINISH_OWNER`는 자연 종료 표시이며 개별 row를 즉시 끊지 않는다. 같은 owner의 반복 실행은 patternSequence까지 대조한다.

실제 편집기 contract에서 원본 4333ms animation을 갖는 패턴에 500~8000ms Sound/Effect row를 추가했다. Stage를1000ms로 줄이고 전체 수명을10000ms로 설정해 Save/reopen/preview expansion 뒤에도 Stage1000ms와 row 원본이 유지됨을 확인했다. Stage를9500ms로 다시 늘려 짧은 Sound/Effect가 Stage 끝을 제한하지 않는 것도 확인했다.

정적 교차 검토에서 새 run receipt와 지연 boss snapshot의 순서 차이, 종료된 child의 epoch 소실, 지연 World cue가 다음 Pattern의 anchor를 참조하는 문제, bundle common row 조기 종료를 찾아 수정했다. session epoch는 생성 때 고정하고 취소된 snapshot key는 재생하지 않는다. World emission은 같은 pinned Product의 stable occurrence ID로 조회한다. common session도 자신의 시작 tick·수명을 가지며 자동 Entry 변경 시 보존한다. 보간 clock이 Server tick보다 먼저 common 수명 끝에 도달해 다음 프레임에 재진입하는 결함도 수정했다. 같은 key의 terminal clock을 보존한다. 실제 지연 패킷을 포함한 다인 화면 실행을 한 것으로 기록하지 않는다.

## G03. 실행한 통합 검증

- 최종 변경 JSON108개, 프로젝트 XML2개, Python25개 parse PASS. `out/RaidRepair20260920/changed-source-parse.json`.
- 독립 row 관련 Client5개 TU 컴파일 통과. 기타 기능별 실제 TU/codec/native playback은 각각 RESULT에 기록했다.
- `--kouku-independent-row-clock-contract`: Stage 축소/확대, 긴 Sound/Effect, 독립 full lifetime, Save/reopen, 실제 preview expansion PASS.
- `--valtan-presentation-contract`: 최신 원본 camera/pattern 소비 계약 PASS.
- Kouku row projection의 focused Python4개와 Encore body/staff 검사 PASS.
- World gameplay Validate/Publish: KAKULSAYDON 114 placements/7 spawn groups PASS.
- Kouku Composition publish1909: Product85 patterns/473 stages/8 bundles. WorldSequences publish2140 PASS. Whirlwind/Mario model resource에 잘못 추가했던 GROUP 전용 motionInstanceIds는 제거하고 기존 resource-bound 모션/default ID를 유지했다.
- 수정 범위 `git diff --check` PASS.

최종 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MaxCompilerProcesses 2` PASS. Engine/Shared/Server/Client를 표준 출력 경로로 컴파일·링크하고 Client/Bin/Debug에 실행 파일·Engine·셰이더를 배포했다. 2026-09-20 13:35~13:55 KST, 총 1,217,038ms. 로그는 `out/RaidRepair20260920/product-build.log`, receipt는 `out/BuildPipeline/runs/20260920T045559871Z-debug-product.json`이다. 기존 헤더 인코딩, shader, DirectXTK PDB 경고는 로그에 남아 있고 오류는 없다. Product 프로필은 컴파일·배포 증거이며 전체 회귀 검사 PASS를 뜻하지 않는다.

최종 live `Publish-GameplayBalance.ps1 -Mode Publish` PASS: 65 boss patterns/280 stages/52 Valtan audition timeline rows. `gameplay-final-publish.log`. Kouku Composition1909, WorldSequences2140와 KAKULSAYDON WorldGameplay도 게시 완료했다. 발탄 WorldSequence 최종 SHA256은 `11359bf34d02b4c0843702cd6a284f7f673e18091c37fb68fe2e170a3e865a80`이다.

새 Server binary와 최신 게시 데이터에서 `--kouku-bundle-contract-test`, `--card-maze-contract-test`, `--bingo-contract-test`, `--kouku-object-overlap-contract-test` 모두 0 failures. 실제 독립 row 저장/재열기 하네스와 최종 발탄 presentation 하네스도 재빌드 후 PASS했다. 발탄 검사에서 구형 BOSS_FACING 전제를 원본 BOSS_XZ source sampled camera의 정확한 2300/2700ms 구간·63/28 keys·up/roll·anchor·5500ms endpoint 검사로 교정했으며, 일반 추적 mode 검사는 독립 admitted fixture로 유지했다.

앞선 빌드·계약 검증 단계에서는 Client/UI 또는 Server listener를 실행하거나 draft Reload를 수행하지 않았다. 이후 사용자 접속 오류 복구에서 실행한 Server listener는 G07에 별도로 기록한다. 다른 PC의 Server나 미저장 메모리가 자동 갱신된 것으로 간주하지 않는다.

## G04. 기존 검사의 실패와 최종 화면 경계

전체 Composition editor contract는 신규 수명 검사에 도달하기 전 기존 mixed-model fixture의 Mario ENTER_AREA admission으로 실패했다. 이를 숨기지 않고 실제 독립 수명 검사를 별도 flag로 실행했다. Runtime input 전체 검사는 기존 G1 catalog 전체 dict와 world revision 고정 기대값이 현재 데이터와 달라 실패하며, 새 Encore 계약은 별도로 통과했다. 검증 도중 다른 기능의 WorldSequence가 갱신된 실행은 freshness 검사가 정상 거부했으며 검사 제거 없이 source freeze 뒤 재실행한다.

최신 격리 Server 데이터로 Support 전체를 재검사했다. 새 독립 수명(지연 Logic/World/Summon, primary 교체, GC pin, counter/shield, 자동 Entry, 명시 Stop) 검사는 모두 PASS다. 기존 finite-card lifetime burst, swept-card contact explosion, tracker half-speed, repeated-tick tracker의 4 failures는 남는다. `out/RaidRepair20260920/final-kouku-support-surface-isolated.log`. 최초 실행의 격리 data-root 미설정 1건은 조건을 맞춘 재실행에서 해소했다. 다른 광역 검사의 이전 실패/미재실행 범위는 KOUKU_ROW_LIFETIME_RESULT를 참조하며 전체 회귀 PASS로 기록하지 않는다.

발탄 원본 FX는 최종52종/89구간이 실제 Load/Drawable/Stage PASS다. 원본5시스템의7구간은 미지원 module 또는 lifetime 범위 검증 거절로 연결을 보류했다. roar16개는 저장된 WorldSequence 재생 범위이며 자동 전투 연출 소비 완료로 세지 않는다. 원본 목록과 시간은 VALTAN_PRESENTATION_REPAIR_RESULT에 있다.

최종 화면 확인은 G1 밝기·머리 카드, 카드미로/휠윈드 망치 위치, 칼날 연속 재생과 갈고리 끌림, 빙고 flip/경계/메두사, 거미 피격 얼굴의 반복 크기, ALT V capture→cube 연결, 발탄 연출·유령 생성이다. 소스/CPU 수치 통과를 GPU 화면 PASS로 기록하지 않는다.

## G05. 통합 중 추가로 확인한 소비자

발탄 입장 원본 actor64(일리아칸)는 Scene06A/53 group133의 원본 몸체·무기·39개 PSA clip에서 A/B 혼합 동작24.708초를 구웠다. 원본 World transform과 실제 b_wp_1 socket을 기존 single-binding 계약의 body·weapon0·weapon1 세 WorldSequence instance에 연결했다. 세 instance는 같은 입장 anchor와 clock으로 재생한다. 원본 손 이펙트 2개도 해당 actor64 본을 따른다. 실제 CModel의 743프레임·5개 본 59,440수치 검사는 최대오차7.15256e-7로 통과했다. Scene component의 dead MIC는 LookInfo 일반 MIC보다 우선하며 native92/93의 원본 body·weapon 재질을 연결했고, 실제 catalog 1,064검사와 기존 재질 보존 106검사 및 shader 컴파일을 통과했다. 최종 화면은 사용자 확인 대상이다.

Stage가 지난 Summon은 살아 있는 primary를 검증하면서 태어난 patternSequence와 pinned catalog로 admission한다. 새 primary pattern만 조회해 합법적인 이전 소환을 거부하던 실제 소비자를 수정했다. Root follow는 현재 위치·방향만 읽고 spawn/stage origin과 피격 ledger는 이전 row가 보유한다.

갈고리 Parent 기본 모션도 게시된 hook_diagonal로 연결했다. 원래 2초 기본 모션은 저장된 combat pattern이 없어 Parent 직후 Server Play 후보가 없었다. 기존 모션들은 보존하고 defaultMotionInstanceId 한 필드만 교체했다. P19는 실제 GRAB_TO_WORLD_OBJECT 판정, P31은 일반 칼날, P95는 즉사 칼날8개 창을 가진 READY/AUDITION_ONLY 패턴이다. P95 selectionWeight=0이며 자동 RaidFlow에 추가하지 않았다. Kouku source revision1909의 최종 Validate는85patterns/473stages/8bundles PASS.

## G06. 발탄 편집 범위

기존 Valtan Action Workbench의 Pattern → Stage → Animation 구조로 입장·버러지들·2페이즈·벽 관련 패턴·유령 부활·사망 audition을 선택한다. 기존 패턴 Effect/Sound 편집 경로를 사용한다. 일리아칸 등 동반 actor의 모델·본 부착·원본 월드 이동과 WorldSequence FX 세부 배치는 연결된 World/Object 도구에서 편집한다. 원본 Matinee의 모든 트랙을 Workbench의 새 row로 변환했다는 의미는 아니다. 정확한 pattern ID와 원본별 잔여 항목은 VALTAN_PRESENTATION_REPAIR_RESULT를 참조한다.

발탄의 V2는 이미 시작한 `stopWithClip=false` 이펙트가 자기 수명 동안 계속 재생되게 수정했다. 이미 시작한 one-shot sound도 Stage 전환에서 유지한다. 발탄의 미래 start cue를 이전 Stage가 끝난 뒤 새로 시작하는 범위까지 추가한 것은 아니다. 쿠크의 독립 Logic/World/Summon timeline과 이 경계를 구분한다.

## G07. 사용자 실행 후 Server 접속 복구

사용자가 전달한 `World=6, Missing world bootstrap: MAHARAKA.worldbootstrap`는 Server가 listener를 만들기 전에 종료한 실제 오류다. 기존 Product 빌드는 컴파일·배포만 검증했고 전체 world 시작 준비를 확인하지 못했다. `.vcxproj.filters`는 접속 주소 설정 파일이 아니며, 현재 `Client/Default/Client.vcxproj.user`의 `LOSTARK_SERVER_HOST=192.168.0.14`, `Server/Default/Server.vcxproj.user`의 `--bind-address 0.0.0.0`은 정본과 일치했다. LAN sync 결과 server-host, TCP7777 LocalSubnet 방화벽도 준비 상태였다.

`Publish-WorldGameplay.ps1 -Mode Publish -WorldId ALL`로 여섯 world를 게시했다. 다음 실제 Server 시작에서 MAHARAKA navigation도 누락된 것을 확인하여 `Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_OCN_EVENTIS_MHP`로 원본160×160/cellSize1의 Server·Client navigation을 게시했다. bootstrap 4 spawn slots, navgrid128,020bytes. 원본 저작 문서를 임의 변경하거나 binary 생성물을 수동 작성하지 않았다.

2026-09-20 14:06 KST부터 표준 `Server/Bin/Debug/Server.exe`를 `Server/Default` 작업 디렉터리, `--headless --bind-address 0.0.0.0`로 실행했다. PID53744의 모든 world 초기화 이후 `0.0.0.0:7777` LISTEN과 실제 `192.168.0.14:7777` TCP 연결 성공을 확인했다. 서버는 사용자의 재접속을 위해 실행 상태로 유지한다. 현재 VS 프로필은 **Client Only (Server Already Running)**을 사용한다. Client/UI 실행과 실제 world 입장 화면 검증은 수행하지 않았다.

로그: `out/RaidRepair20260920/server-recovery-world-publish.log`, `server-recovery-navigation-publish.log`, `server-recovery-ready.stdout.log`, `server-recovery-ready.stderr.log`, `server-recovery-tcp.json`. 최초 nav 누락 시도의 stderr는 `server-recovery.stderr.log`에 보존했다.

재발 방지로 `Tools/Build/BuildDomains.json`의 필수 world 출력에 MAHARAKA를 추가하고 여섯 Server navigation을 명시했다. `Invoke-BuildAndRegression.ps1`의 Product 준비 검사는 이 manifest의 실제 Server 필수 출력을 재사용한다. 자동 게시나 startup 실행은 추가하지 않았고, compile-only PASS와 missing 경고를 구분한다. 기존 BuildDomainManifestContractTests10개 PASS이며 실제 PowerShell 누락 evaluator로19개 파일을 하나씩 제거·복원해 정확한 누락 검출을 확인했다.

복구 후 `-Profile Product -SkipBuild` 준비 검사 PASS, `missingRuntimeInputs=[]`. receipt는 `out/BuildPipeline/runs/20260920T051039212Z-debug-product.json`, 로그는 `out/RaidRepair20260920/server-recovery-runtime-readiness.log`. 이 두 번째 실행은 컴파일을 건너뛴 준비 확인이며 앞선 실제 Debug 빌드 증거를 대체하지 않는다. 제품 C++/shader를 수정하거나 실행 중 Server를 종료하지 않았다.

전체 요청별 실제 구현·제약과 사용자 화면 확인 순서는 [사용자 검증 가이드](2026-09-20_RAID_PRESENTATION_USER_VERIFICATION_GUIDE.md)에 정리했다. 접속 복구 변경까지 포함한 최종 JSON109개/Python26개/XML2개 parse 및 `git diff --check` PASS.


## G09. 사용자 재검토 후 회귀 수정

사용자는 Mario 뿌연 화면, G1 캐릭터 암부, G3 고채도 FX, Complete Play 준비 대기, 휠윈드 오프셋, 주사위 카드 간격·bloom, 책 카메라 정지, ALT V 발밑 이동을 지적했다. 마지막에 거미90도 방향은 직접 수정하고 Product 빌드·실행도 직접 진행한다고 정정했다. 따라서 거미 방향은 변경하지 않았고 이번 최종 전체 Product 빌드를 에이전트가 동시에 실행하지 않았다.

렌더링은 G1 region47에 독립 native-character ambient를 연결하여 직접광0에 곱해 사라지지 않게 했다. 기본값0 및 native-map 제외로 영향 범위를 제한한다. 원본 간접광 계수2.5의 uniform 근사이며 SH 재구축이 아니다. Mario 자체 프로필은 exposure1이므로 G1 alias노출 상속만으로 뿌연 현상을 설명하지 않는다. 로컬 Mario1~4 동안 fog를 억제하고 종료 시 복귀한다. G3 dark alias만 multiplier1→.5로 실효 노출2→1을 맞춰 G3 전체 look에 영향을 준다. P38 nativeHDR·bloom1.3는 유지했다. scene bloom과 explicit effect bloom은 선택 관계이므로 basebloom0을 모든 효과에 재상속시키지 않았다. 상세 수치와 범위는 Rendering Object RESULT G07.

사용자가 직접 이전한 기존 DDS2종의 hash가 원본과 같은 것을 확인했다. Resources 파일은 생성·복사하지 않고 RenderingProfiles의9개 LUT 참조를 `Map/Lighting/KoukuSaydon/`으로 변경했다. G1 ambient4필드, G3 exposure1필드와 함께 정상publisher를 통해 runtime을 게시했다.

휠윈드 손 Motion에 Pattern Box의1.25m/180도 값을 중복 넣었던 오류를 복구했다. 두 key의 Motion은 .300000012m 및 quaternion[-.5,-.5,.5,.5], 기존 Objectscale2와 Pattern Boxscale2는 유지한다. F1 quick panel은 이제 P24 world.1의 placement를 같은 Workbench draft/Save로 편집한다. Preview에는 원 소유 Pattern ID를 전달하여 다른 Pattern 선택 상태에서도 해당 보스 기준을 조회한다.

P78은 spawnInterval2000→4000, 추적 Logic 창8000→16000, 관련 binding창+8000이다. 원래 Stage17.165초와 animation은 유지한다. 네 카드 시작은6.114/10.114/14.114/18.114초이며 마지막 카드는 기존 독립 Logic tail 경로를 쓴다. heart/spade/clover/diamond/emit의 V1 문서 bloomIntensity만1.3→0으로 바꾸고 원본 base color/material은 보존했다. 코드의 slot 머리 카드 V2와 별개 소비자임을 확인했다.

1관문 책 전용 camera track 끝37800ms에서1200ms follow blend-out을 시작한다. 긴 World행 때문에 마지막 pose를3.903초 붙잡던 구간을 없앴다. 완료 row owner를 기억하여 Area auto shot 재획득을 막고 실제 row 종료/명시 Stop/새owner에서는 해제한다. 늦은 Seek도 현재 카메라 lease를 먼저 인계받아 복귀하며 다른 중간 shot의 저작된 hold 구간은 바꾸지 않는다. MainApp 자연 cinematic 종료에서 force snap을 제거했다.

ALT V는 원본 cube 첫 pose의 원점이 발밑인 상태에서 model-center capture를 사용한 회귀였다. capture/image/frame/cube에 공통 camera-relative rig를 연결하고 두 문서의 captureUseModelCenter=false를 설치했다. Effect Detail의 공통 Pos/Rotation/Scale과 기존45도/수축/UV 조절은 실제 소비자와 연결된다. 실제 CModel·bone 검사는2,282 checks/792 samples PASS이나 완전한 원본 capture camera CB 복원이 아니다.

Complete Play는 Kouku Pattern/Bundle/Flow와 Sequences+PatternFlow에서 실제 선택 closure를 준비하고 모든 Debug 참가자의 READY를 기다린다. Valtan 단일 선택 Complete Play도 준비→자동요청/취소/실패/revision 검사를 연결했다. 관련각RESULT의 정확한 준비 범위와20분 상한을 따른다. Valtan 별도 Saved Flow 전체를 같은 새 barrier로 확장했다고 기록하지 않는다.

데이터 설치는 `out/RaidRegression20260920/install.receipt.json`의10파일29필드로 최신 디스크의 stable ID를 재조회해 병합했다. 기존 전체파일 hash와 교체 직전 hash, 백업, 원자 교체 및 자기변경 rollback을 사용했으며 Client process가 없는 상태였다. root 후보 Composition projection PASS(84patterns/464stages/8bundles), WorldSequence candidate 정식validator PASS. 변경 TU 격리컴파일·shader·codec·dependency/pending 검사는 각 담당RESULT를 참조한다. 이번 사용자 실행 화면과 전체 Product 빌드는 별도 확인 대상이다.


### G09 최종 반영·검사 종료

- 최신 통합 ActionWorkbench/Level_KakulSaydonArena/MainApp 세 TU Debug 및 Release `/Zs` 모두 PASS. 로그: `out/RaidRegression20260920/root/compile-final.log`, `compile-release.log`. 각 담당 Engine/Client/Server TU와 shader 검사는 담당RESULT에 기록했다. `/Zs`는 소스 컴파일 검사이며 Product 링크 완료를 뜻하지 않는다.
- Rendering, WorldSequences, Kouku Composition 및 Gameplay balance 정식 Publish PASS. Composition revision1912,84product patterns/464stages/8bundles. 최종10JSON의 설치hash·parse, World source/runtime parity,旧 LUT경로0건, P15무변경, P78 Stage보존 검사 PASS. `out/RaidRegression20260920/final-data-check.json`.
- `git diff --check` PASS. Client/UI는 실행하지 않았다. 사용자가 직접 전체빌드한다고 하여 Product 빌드를 병렬로 시작하지 않았다. 새 Engine/Server/Client를 빌드한 실행에서 화면과 Complete Play를 사용자 검증한다.


## G10. 실제 Complete Play 실패 후 P78 admission 수정

사용자 화면은 `Complete Play - Sequences + Pattern Flow`에서 P78의 `Logic box exceeds the Pattern lifetime` 거절이었다. 앞선 공개 설명에서 코드·게시 검사만으로 실제 버튼 경로가 준비됐다고 설명한 것은 과도했다. 원인은 독립 수명 기능 부재가 아니라 통합 담당이 P78 행을 늘린 뒤 기존 명시 `durationMs`를 누락한 데이터 수정이다. Stage 17165ms보다 늦은 Logic 끝 22114/22980/22981ms가 명시 수명 없이 저장돼 publisher가 P78을 Unavailable로 격리하고 G1 raidgate도 제외했다. 나머지 84패턴의 publish 성공은 전체 flow 성공이 아니었다.

원래 validation 계약과 600초 상한을 보존하고 P78에 `durationMs:22981`만 추가했다. Stage/clip/카드 4초 간격은 유지한다. public prepare 실행에서 before 84/G1 없음 → after 85/G1~G3 raidgate 생성을 확인했다. 실제 C++ BossTool의 Load_ProductIndex/Reload/Prepare_PatternFlow/Validate_PatternFlow 본문과 실제 Composition/Action codec을 쓰는 headless 검사도 before 1912에서 동일 거절, after 1913에서 P78 Stage 9개·오류 빈 값·G1~G3 허용을 재현했다(13 checks/0 failures). UI·network·Client 실행은 하지 않았다.

새 회귀 검사는 실제 prepare와 raid projection에서 Unavailable 격리 및 수명 보완을 검사하고 명시 수명·600초 합·없는 참조 거절을 유지한다. 관련 기존 검사 포함 3 tests PASS. 로그는 `out/KoukuCompletePlayAdmission20260920`의 `python-admission-report.json`, `python-regression.log`, `cpp/before.log`, `cpp/after.log`다. 거미 보정을 포함한 source 1914의 최종 설치 Data를 같은 C++ 소비자로 재검사하여 P78 및 G1~G3 flow 모두 PASS(6 checks/0 failures)를 확인했다. `cpp/final-live.log`.

## G11. 거미 카운터의 180도 역방향 원인과 수정

실제 설치 CModel 돌진 클립의 head→mouth_u XZ는 시작 (+.188570,+.0000634), 중간 (+.110870,+.0079568), 끝 (+.188570,+.0000639)m로 모두 +X 전방이다. 목표 +Z일 때 기존 yaw +90은 DirectX 행벡터 변환으로 +X를 −Z로 돌려 뒤로 돌진한다. −90은 +X를 +Z로 보낸다. Server 이동 벡터는 target−boss를 정규화한 world-space 7m이며 body yaw 보정과 독립이다.

+90 필드와 이를 더하는 수식은 `1686e76a6a`(2026-09-10, effect version 2)에서 연결됐으며 직전 렌더링 수정 commit `f56ae1684`의 변경은 아니다. 이번 조사로 확인한 정확한 결함은 현재 저장된 native-forward 보정의 부호 불일치다. 사용자의 이전 정상 화면과 동일한 조건을 재현한 것은 아니므로 이번 시각 변화의 모든 계기를 단정하지 않는다.

사용자 추가 지시로 Logic31의 `chargeYawOffsetDegrees` +90→−90을 수정했다. 소비자는 P15의 4.500/11.667/18.834초 Logic4/5/6뿐이다. FX 회전, 이동 경로·속도, 스킬 clip, 카메라·조명은 수정하지 않았다. 새 C++ 코드 변경이 없으므로 이번 보완 자체에 EXE 재빌드는 필요하지 않다. 최종 서버 bootstrap 게시와 실행 중 Server의 새 데이터 로드, 사용자 화면 검사를 분리한다.

8방향 × 실제 clip 0/50/100%의 24건에서 몸체 전방·목표 방향 내적이 기존 −0.997 이하에서 수정 후 +0.997 이상으로 바뀌었다. 7m 이동 끝점은 동일하다. 공포 영역은 보스 중심 BOX이며 26,136개 점의 포함 판정이 동일하다. 별도 피해 BOX 중심은 이동 뒤 1.2m에서 앞 1.2m로 맞춰진다. 기존 COUNTER_WINDOW의 방향 제한 없는 판정은 유지되므로 정면 전용 카운터까지 수정됐다고 기록하지 않는다. 최종 Encounter revision 1914의 세 돌진 모두 −90을 확인했다. `out/SpiderFacing20260920/charge-facing-validation.json`, `actual-bone-facing.log`.

최종 Gameplay 게시 첫 실행은 yaw의 음수를 금지하는 `Format-InvariantFloat` 호출에서 거절됐다. 기존 authoring 검사와 Server parser/Brain은 모두 −360..360을 허용한다. publisher의 해당 yaw 출력 한 곳만 기존 `Format-InvariantSignedFloat`로 교체했다. 거리 출력과 다른 숫자 검사는 그대로다. C++ 변경은 없으며 실패 실행은 이전 bootstrap을 보존했다. 최초 로그는 `gameplay-publish-final.log`, 수정 후 재실행은 `gameplay-publish-retry.log`다.

### G10/G11 최종 게시 완료

Composition revision 1914 게시 및 검증 PASS(85 patterns/473 stages/8 bundles, GATE1~3 raidgate). 첫 Encounter 원자 교체는 실행 중 reader와의 파일 잠금으로 거절되어 이전 게시본을 보존했고, 동일 후보의 정식 transaction 재시도는 성공했다. 최종 Gameplay Publish도 exit 0으로 완료했다. 설치된 `Gameplay.bootstrap`에서 P15 세 행의 7m/−90과 P78 `PATTERNTIMELINE` 22981을 다시 확인했다. `final-bootstrap-check.json`과 `gameplay-publish-retry.log`가 최종 증거다.

실제 publisher AST를 실행하는 yaw 경계·부호·타입 검사 16건과 기존 WORLD 숫자 검사 9건 PASS. 이전 unsigned writer를 넣은 격리 후보는 −90에서 실패하여 회귀 감지 확인. 최종 source의 의미 변경은 P78 duration, Logic31 yaw, revision 세 필드뿐이며 JSON parse 및 `git diff --check` PASS. C++/EXE는 이번 보완에서 변경하지 않았다. 사용자가 Server를 재시작하고 Client로 다시 접속해 실제 Complete Play와 세 돌진 화면을 확인해야 한다. 에이전트의 Client/UI 실행·화면 판정은 미실시다.


## G12. 관문·패턴·렌더링 후속 요청의 실제 반영

G12 이후가 이번 후속 요청의 현재 상태다. 앞 절의 과거 빌드, revision, 미완료 범위를 현재 완료 상태로 대신 읽지 않는다. 카드미로에서 캐릭터 밑 광기 게이지와 Q 옆 LMB 안내를 숨기고 Server Q damage500을 연결했다. 기존 물리 LMB 입력이나 Mario 피해100은 바꾸지 않았다. G2 Saved Flow는 원본 카메라가 있는 P77을 소비한다. 카드미로 원본 음원은 이미 두 layer가 들어 있어 새 WAV 없이 SOUND 행의 전체16,827ms를 보존했다.

Pizza P25/P84의 G2_KOUKU 소환 presentation admission 누락을 수정해 기존 Server의10개 소환을 Client에서도 받을 수 있다. 쇼타임은 실제 모델 +X 전방에 맞춰 Server와 Client yaw를−90도로 보정해 목표를 향하며, 정지 중 추적과 즉시 방향도 같은 식을 쓴다. 레이저·휠윈드·팡파레 P21/P23/P24에는 optional animationRootHorizontalScale0을 명시해 수평 root motion을 차단하고 원본 animation의 수직 성분은 유지한다. 해당 후퇴 Logic38의 거리도0이다. 거미 Logic31의−90도/세번7m 돌진은 보존하고 실제 설치 모델의24방향·시간 표본 내적≥.997을 재확인했다.

G3 animation 행이 없는 연출 중 중복 중앙 보스 표시를 억제했다. G2 종료 P5가 중복 실행되지 않고 G3 입장 P7로 직접 연결된다. 마지막 네 arrival Logic의 원본 FX 위치와 Server navigation 높이25.6을 연결했다. 입장 뒤 WAIT_ENTRY에서는 HUD와 이동/공격을 잠그고 첫 '3관문 입장'에서 원래 전투 spawn으로 Server 이동·전투를 시작한다. 이후 버튼은 '재시작'이다. 목적지와 boss admission을 정리·teleport 전에 preflight하고 실패 시 현재 플레이어/보스/관문 상태를 보존한다. 프로토콜 version95이므로 Server/Client를 함께 빌드해야 한다.

G1 연출 끝에서 전투용 World와 light를 READY 전에 준비하고 카메라 lease 종료 전에 commit한다. 동일 gate/epoch 재시작에서도 준비 pending이 있으면 commit을 건너뛰지 않게 했다. 프레임 freeze 감소를 실제 GPU 시간으로 측정한 것은 아니다. Workbench Reset은 이전 CompletePlay를 typed Stop으로 끝내고 최신 저장 로컬 재생을 적용하므로 음원 시간 편집에 CompletePlay 재실행을 요구하지 않는다.

방향광 복구 및 Rendering Workbench 비교는 Rendering Object RESULT G08/G09를 따른다. 비교 버튼은 Recovered map materials 위에서 directional, exposure0.5/1/2, LUT grading, FXAA, Bloom을 켜고 끈다. LUT를 두 번 계산하는 기능은 아니다. 비교 상태는 저장 profile과 분리하며 region 전환·1000프레임 반복에서도 노출이 누적되지 않는다. 기본 실효 exposure1을 보존하고 Mario local source profile FXAA만 껐다. Rendering source/runtime revision71 게시 완료.

## G13. 빙고 반복 전투·갈고리와 유리 추출

Saved Flow kakulsaydon.flow.bingo에 P96 '빙고_반복전투'를 연결했다. Logic99 '빙고_바닥_망치생성'은50,000ms이며 P62/P94/P60/P67/P40의 기존 G3 공격 다섯 개를 재사용한다. 최초 일반 해골2칸,5초 간격으로 살아 있는 보드 위 플레이어1명 표식,5초 후 현재 격자에 설치,3초 후 중심+상하좌우 폭발을 Server가 처리한다. 일반 해골 재폭발은 빨간 해골로 바뀌고 양쪽에서 광기3/초다. 기존 완성 line 전환도 유지한다. 같은 CompletePlay 반복 Parent 사이에서는 보드·이미 생성된 폭탄·주기 deadline을 보존하며 restart/exit에서 초기화한다.

망치는10초마다 같은 축의 최소2칸 떨어진 두 경로를 선택한다. 원본 머리 폭4.69m보다 넓은6.08m 간격이며 경고 화살표 UV3초, 원본 하강1.4초와 이동1.6초를 소비한다. Server swept 머리 영역에만 즉사 판정을 두고 사슬은 제외했다. 새 Data leaf bingo.hammer.warning.arrow는 기존 DDS를 재사용한다. 폭탄의 원본 폭발 tail6,301ms와 skull red_flip instance도 연결했다.

갈고리는 원래 authored 마지막 grip8,214ms에 도착한 위치를 반영한 뒤 이동 잠금을 해제한다. 시각 효과 정지10,334ms/끝11,334ms까지 붙잡거나 추가1.5초 knockdown을 걸지 않는다. 서버 실제70개 Bingo 계약에 endpoint unlock, bomb/hammer/광기,50초 반복 경계를 포함했다.

빙고 입장 유리는 사용자 요청대로 추출만 했다. out/KoukuBingoGlass20260920에 원본 DDS4개·DXBC5개·재질 입력·12파편/2spark 시간과 hash를 보존했다. 새 runtime screen-material carrier나 다른 작업자의 카메라·애니메이션을 만들지는 않았다. 실행 리소스 목록과 도구 stable ID는 KOUKU_RESOURCE_AND_SUBTITLE_INVENTORY_RESULT를 따른다.

## G14. 한국어 자막과 연출 편집

쿠크 원본 GameMsg16문구를 Action14행+Sequence17행, 총31행으로 연결했다. typed SUBTITLE resource의 UTF-8 plain text와 occurrence startMs/durationMs를 기존 JSON에 저장하고 기존 Player의 활성 행만 MainApp이 읽는다. 기존 Font_YoonGasiIIM을 흰색/검은 그림자로 그리며 cinematic HUD 숨김과 별개다. 새 PNG·폰트 파일은 설치하지 않았다. NORMAL/UPPER와 발탄 BALLOON을 지원하지만 언어 선택·번역 catalog·fallback font는 미구현이다.

발탄은 원본 일반/상단6개+배우 말풍선2개를 기존 WorldSequence5개 template의 subtitleTracks에 연결했다. 같은 World instance의 실제 적용 시계와 visible actor bbox 상단을 사용한다. 늑대 두 문구는 기존 World 미리보기 범위이고 자동1관문 입장을 새로 구현한 것이 아니다. MapTool의 기존 World 배우 섹션에서 자막 행 추가/수정/삭제, 시작/길이, 문구/위치/actor slot을 편집하고 기존 Save를 쓴다. 전체 draft 검증 실패 시 기존 template를 유지한다. 같은 화면에 soundTracks 목록과 시작/길이/volume 편집도 추가했다.

원본 없는 문구를 만들어 채우지는 않았다. 앵콜 첫 진입3문구는 카탈로그와 정확한 시간의 인계 occurrence 후보를 추가했지만 별도 담당의 새 입장 시퀀스에는 아직 붙이지 않았다. SCENE02A 말풍선1문구도 추출 보존 상태다. 따라서 모든 원본 연출과 모든 언어가 연결됐다는 의미가 아니다.

## G15. 앵콜 엔딩·발탄 원본 사운드의 설치와 소비자

앵콜 첫 진입의 stop-clearing 두 layer, popup 동일 확률2변형, fake-clear BGM 및 마지막 엔딩 BGM을 원본 bank/event/media에서 확보했다. 신규 WAV5개를 설치하고 시작 시각/Stop/fade를 인계 JSON에 기록했다. 첫 진입 데이터는 준비 상태이며 다른 작업자의 카메라·애니메이션 연결 범위를 침범하지 않았다. 마지막 대사는 기존 WAV 자체가51,185ms로 온전하지만 재생 행이 짧았다. P75/P9 행을 전체 길이로 바꾸고 BGM 원본10ms delay·48.974초 Stop+2초 fade를 반영했다. SineRecip fade는 wwiser 근사식이며 Wwise bit-exact 출력으로 주장하지 않는다.

Bingo 보스 실제 사망은 다음 fixed tick에서 기믹을 정리하고 기존 P9 엔딩을 한 번 시작한다. 엔딩51,285ms가 끝나기 전에 clear bit를 올리지 않으며 종료 후 WAIT_GATE와 clear 상태를 확정한다. 입장 시퀀스가 아직 없어도 이 사망→기존 엔딩 연결은 동작한다. 패턴/Sequence는 기존 run의 immutable revision을 계속 사용한다.

발탄 원본11개 AkEvent key를 조사하고 bank의20/20 media를 확보했다. 연결 근거가 확인된 Scene WAV6개와 기존 M09 BGM 교체1개를 설치했다. WorldSequence에는 배우 수와 관계없이 같은 instance soundTracks를 한 번만 재생한다. generic SoundCue에 기본1의 playbackRate를 추가해 Seek/Pause/속도와 FMOD playback이 함께 움직인다. 자연 visual 종료는 이미 시작한 소리의 남은 tail만 보존하고 사용자 Stop/Seek/Level cleanup은 owner의 소리를 정리한다. 카메라·전투 대기 시간을 긴 WAV 길이까지 늘리지 않았다.

M05/M09는 기존 CValtan Music owner를 유지해 World sound와 중복 재생하지 않는다. 입장 M05 시작이 cinematic early return 때문에 늦어지던 소비자를 수정하고, 실제 사망/사망 audition M09의 원본 시작·Stop/fade·자연 tail을 보존했다. roar World는 SIX_PIZZA_106 STEP04/05의 실제 연출 시계에 연결해 sound/subtitle도 같이 소비한다. collapse3변형과 M02는 원본 미디어를 추출했지만 기존 제품 trigger 대응 근거가 아직 미확정이므로 임의 위치에 붙이지 않았다.

설치는 최신 디스크 stable ID/필드 병합, 교체 직전 hash 재확인, 백업·원자 교체·자기 변경 rollback을 사용했다. phase1 install7개, encore install8개, valtan install8개 receipt가 out/KoukuPlaybackRepair20260920 하위에 있다. Data authoring Action1916/Sequence78, Kouku World2142, Valtan World20을 정식 게시했고 Gameplay Publish도 완료했다. 설치 파일이나 게시 데이터가 실행 중 메모리를 자동 Reload한다는 뜻은 아니다.

검증: 실제 Composition Save/Reload56검사, 쿠크 자막31행 Product 및4096byte 경계, World/EffectV2 실제codec31검사, renderer 제어4025검사, 루트 이동22895검사, 발탄 merged World codec31검사와 실제 publisher malformed5개 거부, 자막 collector14검사·배우 위치 오차0.00005m 미만, World sound lifecycle19검사, FMOD NOSOUND12검사 및 앵콜 설치WAV5개 hash/clock/유한샘플 검사 PASS. 기존 root-motion 광역 fixture의 Albion 기대 차이1건과 앞 절의 다른 광역 미완료를 이번 focused PASS로 해결 처리하지 않는다. Client/UI 실행·GPU 화면·실제 청취는 하지 않았다. 최신 Product 빌드와 실제 Server 최종 결과는 아래 최종 검증 절에 기록한다.


### G12~G15 최종 교차 검토에서 보완한 소비자

G1 성공 handoff 직후 같은 frame에서 기존 Update_KoukuGateSceneProfile을 호출했다. Stop_Preview가 입장 전 profile로 복구한 뒤 다음 frame까지 기다리던1frame 환경 차이를 없앴다. 기존 transactional profile 적용과 rollback을 사용하고 camera lease 반환 및 HUD 억제 해제 전에 처리한다. MainApp 실제 TU compile exit0, 화면 hitch 측정은 사용자 확인이다.

MapTool 연출 미리보기는 기존에 매 frame paused=true와 discontinuous Seek를 사용해 새 sound를 계속 멈추고 재생성했다. PLAYING은 unpaused/continuous sample, 사용자 scrub은 discontinuous sample로 구분하고 같은 player의 legacy 이중 Update를 차단했다. 자연 완료는 마지막 pose를 유지하면서 이미 시작한 sound handle만 retire하고, Stop/수동 scrub/draft 교체는 owner tail도 정리한다. 중복 제안된 helper는 하나의 Retire_InstanceSoundTails API로 정리했다. 실제 사망 완료에서만 CValtan destructor가 M09 자연 tail을 보존하며 명시 중단·audition actor 제거는 기존 Music stop을 유지한다.

최신 source1916/Sequence78로 실제 Server Debug 통합 DebugTeleport7,638검사와 Bingo70검사 모두0failures/exit0. P9의 마지막tick전 clear 미확정·정확한 종료·1회 재생·1~4인 및 admission 실패 보존을 포함한다. 로그는 out/KoukuSequenceRepair20260920의 raid-native-ending-contract.log, bingo-native-ending-contract.log다. 현재 Server 프로세스는 두 검사 뒤 자연종료했다.

최종 read-only 교차 검토에서 G2 entry10의P77/7camera행, G3 네 arrival의 source/projected field, P9 pinnedSequence78/51285ms 일치, World authoring/runtime semantic equality를 확인했다. 변경 JSON13/Python11/XML2 parse, 설치 WAV13개 hash, git diff --check PASS. 이후 MapTool 보완의 최종 TU/회귀 증거와 Product 결과는 아래 빌드 기록에 추가한다.

## G16. 쇼타임 중앙 복귀 높이와 노란 바닥 고정

사용자가 지목한 것은 쇼타임 도중의 노란 부채꼴 바닥이다. 기존 decal은 billboard=false이고 내부 local transform도 고정이지만, P35의 네 occurrence가 followBoss=true여서 플레이어를 추적하는 보스의 회전을 계속 상속했다. 5.520/10.965/24.886/30.727초의 presentation.99/.648/.700/.716만 followBoss=false로 바꿨다. 최초 BOSS anchor와 위치·회전·크기는 유지하고 생성된 pivot을 고정한다. 기존 MAP 장판과 주황 경고 10행은 그대로다. 실제 Make_Pivot/Sample 소비자 32검사에서 최초 transform 일치와 이후 보스 이동·회전에도 불변임을 확인했다. `out/ShowtimeGroundLock20260920/RESULT.md`.

55.869초 마지막 중앙 복귀만 logic100의 BOSS_TELEPORT_GROUNDED를 사용한다. 기존 logic47은 무지개댄스 P38도 참조하므로 보존했다. Server navigation의 목적지와 높이·충돌을 먼저 검증한 뒤 위치와 root 기준을 함께 변경한다. 실패 시 기존 위치·root 정책을 보존한다. 실제 P35 곡선에서 단순히 Y만 내리면 다음 단계 종료에 0.404219m가 남는 것을 재현하여, 해당 패턴의 다음 단계 root 기준도 바닥에서 시작하게 했다. 마지막 원본 점프는 약 2.19m로 유지하며 종료 높이는 바닥이다. Stop/종료에서는 이 정책을 해제한다.

Workbench 로컬 Play/Seek도 같은 새 Logic을 소비한다. 로컬 미리보기는 저작 referenceY, 제품은 Server navigationY를 사용한다. 실제 설치 CModel로 90검사와 역방향 Seek 이후 같은 종료 위치를 확인했다. 실제 Server Brain/Navigation/Collision CPU 곡선 검사는 마지막 점프 보존·종료 부유 제거를 확인했다. 두 검사는 Client 화면 검증이 아니다. `out/KoukuShowtimeGrounded20260920/RESULT.md`.

최신 디스크를 재조회한 stable ID/필드 병합과 교체 직전 hash 검사를 통해 두 수정의 Action revision1917을 설치했다. Composition Publish는 86 product patterns/473 stages/8 bundles, Gameplay Publish는 exit0이다. Sequence revision78은 유지했다. 후보/백업/설치 증거는 `out/KoukuPlaybackRepair20260920/showtime`에 있다.

## G17. 룰렛 위 플레이어 높이 보존

Server는 이미 움직이는 플레이어와 정지한 플레이어 모두 룰렛의 동적 지지면으로 올리고, 사라질 때 기본 바닥으로 내리고 있었다. Client의 이동 예측이 정적 navigation 높이를 다시 적용하여 Server Y를 덮어쓰는 것이 원인이었다. 정확한 원형 지지면 안에서는 기존 snapshot.canPredictMove를 false로 보내고 Client의 기존 Server XYZ 보간을 사용한다. 발판 밖 또는 발판 제거 뒤에는 일반 예측으로 복귀한다. 입력 명령과 Server 이동은 계속 허용하며 높이 보정용 Client 직접 teleport나 새 packet은 추가하지 않았다.

ServerNavigation의 원형 포함 판정을 지지면 높이와 replication이 함께 사용한다. 실제 Refresh→Broadcast→packet read 경로에 정지/이동/원 경계/제거·재진입의 11개 검사를 추가했다. 4개 변경 TU 컴파일은 exit0이며 최종 Debug 실행 결과는 아래 빌드 검증에 기록한다. `out/RoulettePlayerHeight20260920/RESULT.md`.

### G12~G17 최종 빌드·게시·서버 검사

- 표준 Product의 Release와 Debug 모두 compile/link/deploy exit0, missingRuntimeInputs=[]다. Release 첫 통합에서 pending light 멤버가 Debug 선언 안에 남아 있던 9개 오류를 수정했고 재빌드했다. 마지막 PreviewRootMotion/MainApp 소스도 포함하도록 증분 빌드를 완료했다. 최종 receipt는 `out/BuildPipeline/runs/20260920T093028525Z-release-product.json`, `20260920T094135413Z-debug-product.json`이다. 로그는 `out/KoukuPlaybackRepair20260920/product-release-verified-build.log`, `product-debug-verified-build.log`다.
- Release support 검사는 기본 지지면 8개만 실행해 0 failures였다. 새 room fixture는 Debug 전용이므로 이 결과로 신규 항목 완료를 대신하지 않았다.
- 첫 Debug support 실행은 267 PASS/14 failures였다. 추가 10건은 테스트 설정·기대값 문제였다. 룰렛 6건은 run의 PinnedGameplayRevision 초기화가 빠져 snapshot 송신이 거부됐고, 쇼타임 회전 2건은 이전 전방 기준, 높이 2건은 첫 tick에 이미 적용된 33.333ms root Up을 기대에서 누락했다. 두 검사 소스만 수정했다. 게임 동작 코드는 바꾸지 않았다.
- 사용자가 Product Server를 실행한 뒤에는 EXE를 덮어쓰거나 프로세스를 종료하지 않았다. 최신 Debug 제품 OBJ/Shared.lib에 수정한 검사 OBJ 두 개를 결합한 `out/KoukuPlaybackRepair20260920/isolated-support/ServerSupportFinal.exe`로 재확인했다. 컴파일·링크 exit0. 최종 support는 277 PASS/4 failures이며 새 룰렛 11/11, grounded 17/17, 두 회전 기대값 모두 PASS다. 로그는 `final-support-surface-debug-rechecked.log`, 정확한 baseline 대조는 `final-support-surface-analysis.json`이다.
- 남은 4개 라벨은 이전 `out/RaidRepair20260920/final-kouku-support-surface-isolated.log`와 정확히 같다. 읽기 대조에서 카드 두 검사는 started와 contact 두 이벤트를 모두 폭발로 세고, tracker 두 검사는 현재 arena spawn 대신 과거 player spawn 기준 좌표를 기대한다. 이 오래된 fixture 네 개는 이번 수정 대상에서 제외했고 전체 suite의 exit1을 PASS로 바꾸어 기록하지 않았다.
- 앞서 최신 관문/엔딩을 실행한 DebugTeleport 7,638검사 및 Bingo 70검사는 모두 0 failures다. 최종 Action1917/Sequence78, Rendering71, Kouku World2142/Valtan World20을 게시했다. JSON13/Python11/XML2 parse, 설치 WAV13 hash 및 World source/runtime parity를 확인했다. Client/UI 실행·GPU 화면·실제 청취는 사용자가 수행한다.

### 사용자 지정 리소스 폴더 복사

사용자가 마지막으로 지정한 `C:\Users\user\Desktop\GBResources`에 복사했다. Sound 상대 경로의 설치 WAV13개(신규12/교체1,81,969,348bytes), Deploy/RaidPresentation20260920의 인계47파일(59,011,451bytes)을 모두 원본 SHA-256과 대조했다. 인계에는 유리33파일, 미연결 발탄 WAV4+근거8파일, 새 화살표 저작 JSON과 리소스 목록이 있다. 두 manifest와 README도 저장했다. 실행 리소스와 추출만 한 자료의 연결 상태를 구분한다.

초기에 경로를 잘못 해석해 `GB\_Resources`에 새로 복사한 음원13개와 manifest1개는 생성 당시 hash/기존 파일 부재를 재확인한 뒤 workspace out의 recovery 폴더로 회수했다. 해당 위치의 기존 Character/Effect/Map 파일은 변경하지 않았다. 정확한 최종 설치 증거는 `out/KoukuPlaybackRepair20260920/resource-copy/runtime-copy-correct-destination.receipt.json`, `handoff-copy.receipt.json`이다.

## G18. 카드 시작 알림의 잘못된 Client 오류 상태

기존 support 네 fixture를 대조하다가 `combatpresentation.kouku.pursuit.started`도 접촉 폭발 소비자에 전달하는 Client 분기를 확인했다. started는 contactVisualId와 다르므로 정상 카드 생성에도 false/status가 생겼다. Kouku Level은 해당 false에서 debug log를 남기고 update를 계속하며, 이것만으로 Complete Play나 연결을 중단하지는 않는다.

기존 replicated object·live owner·pinned revision 검증을 유지한 뒤, 정확한 started HIT_PULSE/repeat0 및 eventSequence/tick·유한 pose만 lifecycle marker로 수락한다. 시작 알림에서 폭발을 재생하지 않고 실제 contact는 기존 소비자로 전달한다. 알 수 없는 cue와 잘못된 값은 계속 거부한다. 실제 두 production 메서드 본문 adapter 23/23 PASS, ClientReplication.cpp Debug/Release focused compile exit0이다. 최신 Debug 제품 객체에 해당 OBJ만 교체한 격리 Client 링크도 exit0이다. 증거는 `out/PursuitStartedMarker20260920/RESULT.md`와 `linked/link.log`다.

사용자가 실행하던 EXE는 교체하지 않았다. 이 마지막 Client 분기를 실제 실행에 반영하려면 다음 Client 빌드가 필요하다. Server·Data·Resources 변경은 없다.

## G19. 사용자 재검증에서 다시 확인한 G1 사운드 편집 회귀

사용자가 Save→Reset→Play를 여러 번 반복하면 다시 사운드 시간이 반영되지 않는다고 보고했다. G12의 단회 Stop/Play 보완만으로 반복 편집 경로까지 완료했다고 판단할 수 없으므로 이 항목을 다시 조사한다. 최신 Sequence 저장본 revision83에서 실제 G1 P4 SOUND .43의 start15917, .44의 start0을 확인했다. 이전 start0/7630과 달라 저장 자체는 성공했다. G1 부모는 P8 자식을23698ms에 합치며, Kouku WorldSequence의 soundTracks는0개이므로 별도 World sound 중복은 원인 후보에서 제외했다.

사용자가 편집 중인 문서는 읽기만 한다. 저장값·부모/자식 확장·로컬 Preview 소유권·서버 Complete Play 중지 응답을 연결하여 반복 재현과 수정 후 검증을 진행한다. 아직 이 재보고 증상을 해결 완료로 기록하지 않는다.

### G19 조사 종료 및 다음 세션 인계 — 미해결

사용자는 일반 Play 버튼에서 이전 시각에 소리가 계속 난다고 확정했다. 다른 세션에서 이어가겠다는 사용자 요청으로 여기서 조사를 종료했다. 이번 재보고의 실제 실행 증상은 재현·원인 확정·수정하지 못했다. G19에 대한 제품 소스 수정, 데이터 교체, 게시 또는 새 Product 빌드는 하지 않았다.

확인한 사실:

- 실제 최신 G1 P4/P8을 격리한 문서와 production Workbench 객체로 SOUND .43의 Set_PresentationBox → Save → Reset STOP 소비 → Play → preview request 소비를 16회 반복했다. 198 PASS이며 매번 변경한 start1000~14500과 확장된39행이 전달됐다. live Data hash는 불변이다. 증거: `out/KoukuRepeatedSequenceSound20260920/run.log`. 이 검사는 MainApp의 서버/로컬 소유권 전환과 실제 청취까지 검증한 것이 아니다.
- 일반 Play 요청은 매번 draft를 복사·확장하며, PresentationPlayer의 실제 Begin_Preview/Begin_BundlePreview도 새 문서를 복사하고 기존 sound session을 정리한다. 단순 저장 실패나 Workbench 확장 캐시 재사용은 위 검사에서 재현되지 않았다.
- 별도 paused scrub 경로에서는 draftStart2222/previewStart14500, pendingFresh=0을 재현했다. 이는 기존 preview 문서에 SEEK만 전달하는 별도 문제이며, 사용자가 지목한 Play 증상의 확정 원인으로 취급하지 않는다.

아직 검증하지 못한 원인 후보:

- `MainApp.cpp`의 local preview guard는 `m_bKoukuLocalPreviewStopRequested` bool이 설정되면 Complete Play STOP을 다시 요청하지 않는다. STOP 제출 실패·거절·timeout 뒤 bool 해제와 새 Play 재시도 처리가 충분한지, 그동안 이전 서버 pinned document가 계속 Sample_ServerSequence로 재생되는지 실제 라우팅 재현이 필요하다.
- transport를 owner 일치 확인 전에 Consume하는 분기, ServerClock에서 StopCompositionPreview가 즉시 반환하는 분기, 서버 cinematic과 로컬 Workbench의 owner 관계를 함께 확인해야 한다.
- Reset 직후 같은 프레임에 Play를 큐잉하면 Queue_PatternDocumentPreview가 transport를 NONE으로 바꿔 reset 의도가 사라질 수 있다. 사용자 실제 입력 순서와 동일한 MainApp 경로에서 확인하지 않았다.

다음 세션은 위 재현 증거와 최신 디스크 저장본부터 확인하고, 실제 MainApp의 server-to-local 전환을 재현한 뒤 요청/epoch별 STOP 완료·실패·재시도 상태를 검토한다. 현재 Sequence83은 조사 시점 값이며 사용자가 계속 편집할 수 있으므로 재사용 전에 다시 읽어야 한다. 기존 G18 ClientReplication 수정은 격리 compile/link만 완료했고 Product EXE 미교체 상태다. G19를 포함해 모두 수정됐거나 빌드만 하면 이 증상이 사라진다고 안내하면 안 된다.

## G20. Sound 리소스 검색·목록 최적화와 WAV 구간 편집

설치 Sound 파일4183개, Sequence Created Sound483개 규모에서 기존 Sound 탭은 매 프레임 전체 resource 구조체 복사·source ID/label 생성·Selectable 제출을 반복했다. Effect만 갖고 있던 cache 정책과 별개로 Sound source/created view cache를 추가했다. source는 Refresh/검색 변경, created는 draft generation/검색 변경에서만 갱신하며 두 목록은 ImGuiListClipper로 보이는 행만 제출한다. source 선택 조회는 stable ID map을 사용한다. 검색은 표시 이름/WAV asset 경로/resource ID를 대소문자 구분 없이 부분 일치하며 Clear와 검색 건수를 표시한다. Preview/Create와 새 Append Sound at Cursor는 기존 transactional 소비자를 재사용한다.

Sound 바 왼쪽 trim은 timeline start와 soundSourceStartMs를 같은 delta로 바꾸고 기존 끝 시각을 보존한다. 오른쪽 trim은 source start를 유지하고 길이만 바꾼다. 가운데 이동은 source offset을 보존한다. WAV 앞0·최소1ms·원본/패턴 끝을 제한하고, Box Detail에 Source In/Out ms 및 이벤트/WAV 경로를 추가했다. sourceStart 저장·projector·Player의 Play_SoundCue(offset+age)와 occurrence 종료 Stop 경로는 기존 계약을 사용한다. WAV 파일 자체를 편집하지 않는다.

physical resource inventory의 duration3000ms는 임시값이므로 이를 원본 끝으로 오인하지 않게 했다. 선택/trim/detail의 해당 WAV만 기존 Get_SoundDurationMs로 최초1회 확인하고 성공·실패를 cache한다. 전체4183파일을 매 프레임 열지 않으며 Refresh가 duration cache를 비운다. 선택한 source를 Create/Append할 때 실제 길이를 전달한다.

### G20 두 음원의 정체와 전달

이전 G1 P4의 두 SOUND row는 원본 SCENE03A interpdata_0의 연출 이벤트 `s_scene_ocean3_3.scene_midnightc_ed_movetoinsideofcircustent`와 BGM `s_bgm_commanderraid.bgm_midnightc_ed_m02_scene_movetocircus`다. 전자는 media32362025/164546237을 합친47.850729초 event WAV이고 후자는17.562771초 BGM이다. 두 행을 각각 폭죽 전용/세이튼 대사 전용이라고 단정하지 않는다. 폭죽 Effect 근거는 별도 interpdata_7이고 그쪽에는 circuspopup/m01_entrance_popup 이벤트가 있다. 각 scene audio layer의 역할은 이번에 추가 청취·추출하지 않았다.

사용자가 편집 중인 최신 저장본은 계속 바뀐다. read-only 조사 revision100에서는 P4 `.43`만 남았고 `.44` BGM은 없었으며, 단독 P2 피날레에는 SOUND 행이 없었다. 이번 작업은 그 저장본을 복원하거나 교체하지 않았다.

두 기존 runtime WAV를 사용자가 지정했던 `C:/Users/user/Desktop/GBResources/Sound/KoukuSaton/Events/`에 복사했다. 파일은 `event.f8dcefc1b0f00e75024f.wav`(연출), `event.cdd84d3942cd42d9166c.wav`(BGM)이며 원본/대상 SHA256 동일이다. 새 음원 추출·runtime asset 추가가 아니라 기존 파일 전달이다. 영수증은 `out/KoukuSoundResourceSearch20260920/wav-handoff.json`, 원본 근거는 `out/KoukuSoundRestore20260918/sequence/manifest.json`과 `out/KoukuFireworks20260911/LV_LUT_MIDNIGHTC_ED_SCENE03A.json`이다.

### G20 검증 경계

Workbench H/CPP만 기존 UTF-8/CRLF로 수정했고 새 C++ 파일·project/filter·schema·publisher·사용자 Data 변경은 없다. 최소 Debug/Release TU compile과 CPU 소비자 검증의 최종 결과는 아래에 추가한다. 실행 중 Client/Server를 종료하거나 EXE를 교체하지 않았다. 실제 2.8fps 개선 수치·검색/drag UI·청취는 사용자가 다음 Client 빌드 후 확인한다. G19 반복 Save→Reset→Play의 기존 시간 재생 문제는 미해결 인계 상태를 유지한다.

G20 최종 검증: 최신 Workbench.cpp Debug/Release focused compile exit0(`out/KoukuSoundResourceSearch20260920/compile-debug.log`, `compile-release.log`). 최종 CPU probe1031검사 PASS/exit0(`cpu-run.log`): 실제4183 source/483 created, 검색·빈결과·Clear·Refresh·Create/Rename/Append, 단일 Sound 이동의 Set_PresentationBox source offset 보존, 실제 Trim_SoundWindow 앞0/끝보존/원본끝/패턴끝/최소1ms를 확인했다. 이 probe는 ImGui·Profiler 호출을 제거한 목록 필터 본문과 실제 Workbench 편집 소비자를 격리 실행했고 Client/UI/오디오 장치는 실행하지 않았다. warm cache1000회0.7053ms는 CPU 필터만의 값이며 게임 FPS 개선 수치가 아니다. 두 WAV 복사 hash 동일, git diff --check exit0. Product EXE 재빌드·교체는 하지 않았으므로 다음 Client 빌드가 필요하다.

## G21. 일반 Sequencer Play 시계의 분수 밀리초 손실

사용자는 일반 Play에서 저장한 사운드를 옮겨도 이전처럼 들리고 약 2초 어긋난다고 보고했다. 조사 당시 실행 중 Debug Client는 19:43:26 빌드였으며 G20 Source In/Out·Sound 검색 UI 문자열을 포함했다. G20 당시 미배포 기록과 현재 사용 바이너리를 구분했다.

`KoukuSaydonPresentationPlayer::Update`는 double 밀리초를 누적하지만 `Sample_BundlePreview` 및 MainApp을 거친 `Sample_Preview`에서 `Preview_ClockMs()`의 정수 표시값을 매 프레임 다시 누적 시계에 대입했다. sound channel은 정상 속도로 계속 진행하므로 FPS에 따라 타임라인과 소리가 벌어진다. 두 대입에 조건을 두어 일반 내부 시계의 소수부를 보존하고 capture hold·실제 cursor 변경·외부 Animation 시계는 기존 지정 시각을 적용하게 했다. 명시 Begin/Seek·Server sample 계약은 유지한다. 기존 UTF-8 noBOM/CRLF와 다른 변경을 보존했고 새 파일·헤더·schema·사용자 Data·WAV는 수정하지 않았다.

Git 이력상 bundle 대입은 09-14의 capture 시계 연결(`98d99eec58`), single 대입은 09-07부터 있었다. 이번 trim 저장 필드가 2초를 추가한 것은 아니다. 실제 P4는 자식 P8의 animation을 확장해 actor bundle preview로 들어가므로 수정한 경로의 소비자다. 사용자 저작값과 당시 FPS·실제 청취를 소급 재현하지 않았으므로 체감 2초의 유일한 원인이라고 확정하지 않는다.

검증 증거는 `out/KoukuSequenceSoundClock20260920`에 있다.

- 실제 production clock 본문을 추출한 headless C++ fixture에서 수정 전 single/bundle 모두 60Hz·50초 후 2,000.002608ms 누적 손실을 재현했다. 수정 후 24/30/60/120/144Hz에서 동일 누적 손실은 0ms다. pause/resume·capture 유지/이동/해제·외부 clock·duration 제한 포함 25/25 PASS. `clock-before.log`, `run.log`, `clock-fix.diff`. 외부 렌더링/오디오 서비스는 adapter이며 FMOD 실제 청취 검증이 아니다.
- 최신 Workbench와 CompositionDocument TU를 새로 컴파일한 별도 probe에서 frozen Sequence revision114의 SOUND 두 행의 start/duration/sourceStart를 매회 변경했다. 실제 Set_PresentationBox→Save_Atomic→parse→Reload→Reset→ordinary Play→부모/P8 확장 16회, 39행 비교 포함 1862검사 PASS. `save-roundtrip/run.log`, `result.json`. 다른 기존 Debug 객체와 Engine/Shared 라이브러리는 재사용했다. 검사 중 사용자 live 문서는 revision119까지 바뀌었고 probe는 격리된 out/Data에만 썼다. live hash 불변으로 기록하지 않는다.
- Player Debug/Release focused compile exit0. 현재 Debug 제품 link 입력에서 Player OBJ만 새 객체로 치환한 전체 Client 격리 링크 exit0. `linked/Client.exe`, `linked/link.log`. 기존 DirectXTK PDB 누락 경고는 남는다. Client/UI 실행은 하지 않았다.
- 독립 검토에서 Animation Tool, Effect Authoring Sequencer, Effect Composition Workbench, Effect Tool, WorldSequence Player/ToolPanel의 같은 정수 표시값 되쓰기 누적 소실은 추가 발견하지 못했다. 전체 오디오 동기화 문제 부재를 뜻하지 않는다.

남은 경계: G19의 stale paused scrub 및 Server/local 전환은 별도이며 해결 완료로 바꾸지 않았다. capture hold 중 SOUND가 계속 진행하는 별도 경로도 확인했지만 당시 revision118의 P4 capture 시점(19,819ms)에 활성 SOUND가 없고 저장 SOUND는47,213ms부터여서 이번 2초 현상과 직접 연결하지 않았다. 사용자 편집이 계속되므로 이 시점과 행을 영구 현재값으로 취급하지 않는다. 실행 중 Client.exe의 실제 쓰기 잠금을 확인했으며 검증된 수정본의 최종 반영은 사용자 저장·Client 종료 후 진행한다. Server 종료나 Data 재게시가 필요한 변경은 아니다.

G21 설치 후속: 사용자가 현재 실행을 유지한 채 다음 실행 파일 반영을 요청했다. 실행 중 image의 직접 쓰기는 잠겨 있었지만 같은 볼륨의 안전한 rename은 허용됐다. 기존 EXE를 `out/KoukuSequenceSoundClock20260920/backup-20260920-201503-209/Client.exe`에 보존하고 검증한 새 EXE/PDB를 `Client/Bin/Debug`에 설치했다. 설치 SHA-256은 `1c250e755ab5be9767b6e95df580bf8c8878424aa1139b97781f23b9dcc0b313`, 영수증은 `install-debug.receipt.json`이다. 기존 실행 process는 변경하지 않았다. 이후 사용자는 핵심 증상이 누적 drift가 아니라 사운드 바가 없는 구간의 발음이라고 명확히 했으므로 해당 문제는 G22에서 별도 조사·수정한다. G21 설치를 그 증상의 해결로 기록하지 않는다.

## G22. 저장 후 이전 재생 문서 잔류 수정

사용자가 명확히 한 증상은 사운드 바가 없는 구간에서의 재생이다. 실제 Save는 disk/draft를 갱신했으나 이미 시작한 immutable local Preview는 이전 문서를 계속 보유했고 Scrub/Resume도 이를 재사용했다. 저장 실패로 단정하지 않는다. 실제 SOUND 소비자의 [start,end) 범위·trim·handle 정리는 정상이고 새 snapshot 교체·Stop에서 이전 handle이 제거됨을 확인했다.

Workbench는 preview identity·draft generation을 기록한다. 변경 Save 성공 시 이전 재생과 대기 요청을 기존 STOP transport로 정리하고 cursor를 보존한다. 다음 Play와 수정 후 Scrub/Resume는 최신 pattern/bundle snapshot을 요청한다. invalid 문서와 MainApp staging 실패는 같은 frame의 STOP으로 정리하여 실패 요청의 generation을 성공한 재생으로 오인하지 않는다. Resources 공용 Resume도 이 경계를 사용한다. Server의 Complete Play pinned 문서와 다른 owner의 재생은 임의 교체하지 않는다.

검증 증거는 `out/KoukuSequenceSoundClock20260920/ghost-preview/result.json` 및 개별 로그에 있다.

- 실제 Workbench/CompositionDocument 객체로 move·trim·delete→Save STOP, 최신 Play, stale Scrub/Resume, stopped/pending Save, admission failure, Bundle refresh 및 동시 저장 거부를 16회 반복해 1,838 checks PASS.
- 이후 발견한 invalid Pattern/Bundle Scrub early-return에 STOP guard 14줄을 보강했다. 최종 source로 해당 실패 경계 등 127 checks PASS, Debug/Release TU compile exit0. 두 실행의 source hash를 result.json에 구분했다.
- 실제 SOUND active gate/cleanup 본문 adapter 21 checks, MainApp 실패 시 pause 경계 adapter 12 checks PASS. 실제 장치 출력이나 화면 검증이 아니다.
- 사용자의 live Sequence는 읽기만 했고 검사는 out의 복사본을 사용했다. G22 검증 snapshot revision119의 검사 전후 hash는 같았다. 일반적으로 편집 중인 live 문서가 계속 불변이라는 의미는 아니다.
- 첫 정식 Debug Client Build는 성공했으며 마지막 invalid Scrub 보강 및 G23 통합본의 최종 설치는 아래 후속 검증에서 구분한다.

남은 경계: P4 아래 P8의 발걸음·cast SOUND도 기존 의도된 child track이며 부모 SOUND bar가 비어 있어도 해당 child 시간에는 발음할 수 있다. 별도 제품 sound session이나 Complete Play를 막는 Server STOP 실패·timeout 재시도 경계는 이번 ordinary local snapshot 수정의 검증 범위가 아니다. 사용자 장치에서의 최종 청취·UI 확인은 미실시다.

## G23. 대기 공간 BGM과 일반 Play의 3관문 입장

정확한 음원은 `Sound/KoukuSaton/S_BGM_COMMANDERRAID/bgm_midnightc_ed_m12_ready_terrace_2ndcircus__559227263.wav`다. 원본 Wwise event248306555→action126716766→playlist278959898→segment694770763→track454435378→media559227263을 대조했고 WAV 길이128.135604초와 원본 track 길이가 일치한다. 설치된 WAV를 기존 반복 Music 채널로 소비한다. `m02_scene_movetocircus` 이동 연출음 또는 Wwise event의 다른 music state 전체를 복원한 것으로 기록하지 않는다.

Level은 최초 활성화 뒤 Server-replicated local player 위치를 읽어 시작 공간과 3관문 판자 공간에서 BGM을 재생한다. 같은 공간의 반복 snapshot으로 매 frame 재시작하지 않고, 파일 실패도 새로운 재생 경계에서만 다시 시도한다. 일반 Sequence의 성공한 Play/Resume는 BGM을 끄며, 재생 owner의 Stop·교체·자연 완료 때 현재 위치로 다시 판단한다. Server CINEMATIC/COMBAT 중에는 재생하지 않는다. 3관문 제품 Sequence는 Server가 마지막 arrival을 적용한 뒤 WAIT_ENTRY에서 다시 재생하며, 승인된 COMBAT과 원래 전투 위치 이동에서 종료한다. Level 퇴장에서도 자신이 시작한 music을 정리한다.

ImGui의 기존 아레나 이동/Return to Start 묶음에 `3관문 입장 전 공간`을 추가했다. 마지막 arrival slot0의 (-17.509552,25.600000,960.530156)을 기존 PlayerController→Server Debug teleport로 요청하고, BGM과 제품 버튼은 요청 시점이 아닌 실제 복제 좌표를 소비한다. 시작 공간은 authored party01 (3.29,8.64,-10.69) 기준 XZ 반경10m·Y±3m, 판자는 X[-30,-5],Y[23.5,28],Z[947,972]이다. 이 범위는 같은 높이의 실제 floor/fence/deco와 네 도착 슬롯을 포함하도록 정한 정책 구역이며 mesh collision bounds 실측값이라고 주장하지 않는다. Shared 새 헤더 하나를 프로젝트·기존 Gameplay filter에 등록했고 Client와 Server가 같은 판정을 쓴다.

일반 Play의 도착은 Server Raid WAIT_ENTRY를 만들지 않는다. 따라서 제품 버튼의 `ENTER_GATE3`를 일반 RESTART와 분리해 기존 확인·파티 투표 경계로 보낸다. active raid는 기존 owner·GATE3 WAIT_ENTRY·epoch를 검증하여 `Enter_KoukuRaidCombat(3)`을 사용한다. nonraid는 proposer의 실제 판자 위치와 동일 leader/party roster를 proposal과 승인 때 검증하고 전원 목적지·boss 생성·cleanup lifecycle을 사전 검사한 뒤 기존 관문 spawn/teleport 경로에 commit한다. 원래 전투 위치는 (-2.45,1.32,945.17)이며 무관한 관전자의 위치를 옮기지 않는다. 이전 관문 번호가0/1이어도 새 요청은3을 명시한다. 기존 ADVANCE0/RESTART1은 유지하고 ENTER_GATE3=2, protocol96으로 양쪽 실행 파일을 갱신했다.

검증 및 설치:

- 실제 Level 메서드 본문 adapter 41/41 PASS. 최초 snapshot 부재·도착 뒤 재생, 일반 Play start/end/Stop, 재생 중 판자 이동 억제, CINEMATIC/COMBAT, 다른 player 무시, 하부 전투층 제외, media 실패 재시도 경계를 확인했다.
- 실제 Client UI/Level/MainApp 메서드14개 adapter 21/21 PASS. 전용 입장 확인·취소, vote 수락·거절/중복 차단, 판자 이탈 후 stale 확인, local 재생 중·비리더·관전자 차단 및 owner 종료 알림을 확인했다.
- 최종 Shared·Server·Client Debug Build exit0. 변경 Client 5TU 및 Server 2TU의 Release 격리 실제 컴파일 exit0. XML parse와 전체 `git diff --check` PASS. 기존 인코딩/DirectXTK PDB 경고는 남아 있고 이번 빌드의 컴파일·링크 오류는0이다.
- Client/Bin/Debug/Client.exe SHA256 `28cbaa35969ff47ae310f6cbf0384ed90bc92fc962287aa23421fb9e700f1525`, Server/Bin/Debug/Server.exe `b0983713e1b97cd586ff346ca282b257b345ecb867a731e8a9845683bb72607f`. `out/KoukuReadyTerraceBgm20260920/final-build.receipt.json`에 최종 source와 설치 파일 hash를 기록했다. 이 Client에는 G21 clock, 최종 G22 invalid Scrub guard도 포함된다.

증거는 `out/KoukuReadyTerraceBgm20260920`의 source-audio.receipt.json, ready-area-evidence.json, lifecycle-run.log, client-gate-ui-run.log와 build log 및 `out/KoukuGate3Entry20260920`의 Server source/compile 기록이다. 사용자 Data·WAV 변경이나 domain publish는 하지 않았다. Client/UI 실행과 실제 청취는 미실시이며 사용자가 새 Server와 Client를 함께 실행하여 확인한다.

G23 서버 실행 검증 완료: 최종 설치 Server.exe의 `--debug-teleport-contract-test` exit0, PASS 7906 / FAIL 0. 신규 명시 입장·1~4인 합의·거절/timeout·판자 이탈·party 변경·목적지/생성 실패 보존·관전자 위치 보존·packet 왕복과 기존 전체 Raid WAIT_ENTRY 입장을 실제 CGameRoom으로 검증했다. 로그는 `out/KoukuReadyTerraceBgm20260920/server-entry-contract.log`이며 Client/UI를 실행하지 않은 headless Server 검사다.
