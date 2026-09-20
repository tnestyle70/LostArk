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
