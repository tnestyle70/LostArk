# 쿠크 2관문 복원 결과

## G00. 실제 반영 상태

사용자가 3갈레바닥장판폭발은 기존 P105 `쿠크_나팔액션`이라고 확정하고 반영을 승인했다.
최신 저장본 revision2197에서 stable ID와 변경 필드만 병합해 **revision2198을 실제 설치**했다.
Effect8개/224요소, catalog·tree·project None 등록, Composition·SoundCatalog와 원본 WAV16개를
총30파일 원자 교체했다. 백업 transaction은 `out/transactions/kouku-raid-1b7177d2f314405f849f60576c8e5675`다.

사용자의 후속 정정대로 휠윈드 P24, 피자 P25, 파1빨2 P11, 바주카 P85의 사운드는 추가·수정하지
않았다. 각각 기존7/20/6/11 SOUND 행 전체가 동일하다. 이 네 패턴에 자동 보충하려던44개는
out 후보에서만 존재했으며 **실제 데이터에 설치된 적이 없다**. 바람 P99의3개만 보충했다.

새 데이터 설치와 Product Debug 빌드는 완료했다. 실행 데이터의 최종 게시 및 Complete Play
검증은 G05/G06에서 구분한다. 아래 후보·probe라는 표현은 원본 복원과 검증 방법을 설명하며
현재 설치 여부는 이 절과 G05를 정본으로 한다. Client/UI는 실행하지 않았다.

## G01. 원형·6방향 폭발

`build_kouku_gate2_fanfare_groups.py`는 Action4219713 / Att_Battle_3_06의 직접 원본 occurrence로 후보를 교정했다. notify011의 B_02 원형11요소와 활성 notify020/021/022의 bilateral B22요소씩을 묶어 `effect.kouku.gate2.fanfare.circle.six-rays` 77요소를 만든다. 원형 onset0.944024980초를 묶음의0초로 삼고 직선 onset1.124565005초까지의 시차0.180540025초를 보존한다. 기존 D_02 원형은 별도 Action4219715 / Att_Battle_3_08의 도넛 경고 뒤2.078297초에 연결되므로 이번 후보에서만 제외했고 원본 문서는 변경하지 않았다.

원본 compact notify의 transform+40에 저장된 FRotator int32 yaw8192/16384/24576은45/90/135도다. +28의 별도 float0 벡터를 회전으로 읽지 않는다. 원본 yaw0의 notify019는 disabled이므로 제외한다. snapshot root의 -90도 basis를 독립 occurrence에 한 번 적용한 yaw는-45/0/45도다. native3162의 양쪽 중심은 source X±950cm, Z50cm이며 TypeData pre-rotation0도·MeshRotation0turn·local-space true를 유지한다. native3253의 .25turn MeshRotation도 별도로 보존했다. SkillDecal2113의 rectangle 경고6개와 SkillEffect421971302/303/304/306/307/308의 AreaOffsetAngle45/90/135/225/270/315도도 같은 방향을 뒷받침한다. 기존60도 등간격 저작 배치 주장은 폐기했다.

실제 Codec의 Load/Validate_Drawable/Serialize→Parse와 Playback Stage 및120Hz 재생이 통과했다. native3162의 실제 중심 방향은 atan2(runtime Z, runtime X) 기준 -135/-45/0/45/135/180도였고 원형 첫 입자0.016667초, 세 직선 그룹 첫 입자0.183333초로 상대 지연을 소비했다. 77요소 중59개 활성, 최대345입자, finite 실패0, duration1.930540초, 최초 Is_Finished1.933333초, 최종입자0이었다. 나머지18개는 원본 rate0/bursts없음 발생기다. 기본 계산 duration1931ms와120Hz 종료를 여유 없이 올림한 행 duration1934ms를 구분한다. 원본 재질·개별 size·현재 leaf TRS는 유지했고 공유 원본 Authored/P23은 변경하지 않았으며 P105 연결은 G05를 따른다. 증거는 `out/Gate2Restoration20260922/fanfare/installation.json`, `source-occurrence-playback.log`, `verification-receipt.json`이다.

8요소 `effect.kouku.gate2.fanfare.front.music`의 D_Music은 Action4219713에 직접 notify가 없다. Action4219715 / Att_Battle_3_08의0.751487초 리소스를 앞쪽에 쓰는 PROJECT_AUTHORED_REUSE 후보로 명시했다. 8/8활성·최대101입자·duration 및 최초 종료3초·finite 실패0이다. 저장 Pattern 연결은 G05를 따르며 원본과의 GPU 화면 판정은 사용자 확인이 남아 있다.

## G02. 저글링·줄타기 트레일·등장 공

`Tools/EffectPipeline/build_kouku_gate2_juggling_entrance.py`는 설치 저작 문서와 추출 캐시의 원본 occurrence를 재사용해 세 후보를 만든다. `out/` 아래에만 기록하며 공유 Authored·Composition·Resources를 직접 교체하지 않는다.

저글링 `effect.kouku.gate2.juggling.three.balls`는 87요소다. Action4219716의 B_WP_1/3/2 소지 공과 Hit를 그대로 연결하고 소지 공 종료를 원본 notify 경계로 제한했다. Projectile421971601의 Jugle_02를 2.2/3.7/5.2초에 생성하고 각 mesh의 death event를 Jugle_Exp_01 receiver로 연결했다. 실제 설치 ball mesh 정점의 반폭은 약 .4002/.3356/.3357m이며 원본 size1.2를 유지한다. 작은 핑크 폭발은 원본 projectile scale .5와 UE Z10cm offset을 사용한다. 기본 목표점 [8,0,-2], [8,0,0], [8,0,2]m와 포물선 곡선은 PROJECT_AUTHORED다. 원본 initial speed6.5m/s·최대높이2.5m를 소비하지만 원본 Server의 목표 선정·경로를 복원한 것으로 기록하지 않는다.

`effect.kouku.gate2.entrance.rope.trail`은 원본 Circusbomb_Trail_01의 무지개 ribbon2요소다. 원본 notify 2.378813초와 3.834585초 창, FX_State_01→bip001-spine1 및 socket 회전 [0,270,90], UE Z20cm offset을 사용한다. `effect.kouku.gate2.entrance.drop.ball`은 TimePrj_01 14요소와 Circus_Exp_01 14요소다. 원본 occurrence ID로 native2428~3317의 해당 프로그램을 선택하고 누락된 중첩 ScaleFactor CDO 기본값을 복원했다. 원본 직접위치 곡선은 공을 3m에서 .2m까지 .15초 동안 하강시키며 3초 fuse 종료의 mesh death event가 큰 핑크 폭발을 생성한다. emitter-location 자식보다 mesh provider가 먼저 처리되도록 순서를 보존한다.

리소스 트리 후보는 `패턴 | 세이튼등장 | 쿠크줄타기트레일`, `패턴 | 세이튼등장 | 세이튼등장하강폭발공`이다. 표시명 64-byte 제한 때문에 문서에는 leaf 표시명, 트리에는 category 경로를 각각 저장한다.

등장 공의 PLAYER Effect anchor는 현행 codec에서 허용되지 않는다. 기존 SHOWTIME fixed template을 살아 있는 각 플레이어의 Server navigation 지면에 생성하는 경로를 재사용했다. compound Effect 한 개를 그룹으로 사용할 수 있게 Effect 선택 그룹의 최소 개수만 1로 일반화하고 Collider와 Albion SELECT의 기존 최소 2개 검증을 유지했다. UI의 단일 Effect 그룹 생성·해제와 저장·timeline 이동은 같은 검증을 사용한다. fixed visual이 있고 tracking visual이 없는 SHOWTIME만 boss target/yaw 변경을 생략한다. 일반 Play와 기록된 입력의 되감기에도 같은 조건을 적용했다. 기존 mixed/tracking/BOSS_TRACK_TARGET 및 P103 random-only의 동작은 유지하며 새 runtime flag나 별도 target authority 경로는 추가하지 않았다.

실제 CEffectDocumentCodec의 Load/Drawable/serialize roundtrip과 CEffectPlayback의 Stage·20초 CPU 진행·finite·종료를 검증했다. 저글링 mesh는 정확히 3개 생성됐고 3.45/4.916667/6.45초의 작은 폭발 XZ가 각 endpoint와 일치했다. 등장 공은 .166667초의 fixed-step sample에서 .2m에 도달하고 3.033333초에 원본 큰 폭발이 발생했다. synthetic FX_State_01를 5m/s로 이동시킨 트레일 검증에서 2점 이상 ribbon frame을 488회 관측했다.

| 후보 | GetDuration | 최초 Is_Finished | 마지막 실입자·트레일 |
|---|---:|---:|---:|
| 저글링 3공 | 9.74479초 | 9.75초 | 7.63333초 |
| 줄타기 트레일 | 6.33459초 | 6.35초 | 4.36667초 |
| 등장 하강·폭발 공 | 5.15초 | 5.15초 | 5.03333초 |

P106의 두 원본 clip을 유지하여 0/5800ms에 같은 3공 문서를 연결하면 마지막 그룹의 최초 종료 시각은 15550ms다. P8의 2800/4800ms 두 생성과 5.15초 tail은 9950ms까지 필요하다. root 통합 후보가 원본 clip·stage와 별도로 Pattern presentation tail을 보존한다.

단일 compound Effect의 targeted ownership, 기존 여러 Effect SHOWTIME, Collider 그룹의 오류 거부, Effect 혼합 kind 오류 거부의 Python focused tests 4개가 통과했다. builder syntax, 후보3개 JSON parse와 수정 파일 `git diff --check`도 통과했다. C++ singleton Save/Reload와 Server fixed-only yaw/target 보존 회귀는 기존 테스트에 추가했고 제품 컴파일·실행 결과는 통합 빌드 기록을 따른다.

제품 Server17:46:46 빌드의 `--kouku-support-surface-contract-test`는 현재 DataFiles의 격리 복사본을 `LOSTARK_SERVER_DATA_ROOT`로 지정해271.537초 동안 실행하여 failures0으로 통과했다. 새 fixed-only yaw/target 보존과 기존 P103 random-only 회귀를 포함한다. 최초 환경 미설정 실행의 admission 환경 오류와55초 제한 실행은 최종 성공으로 기록하지 않았다. 전체 성공 로그는 `out/Gate2Restoration20260922/source-effects/server-support-contract-isolated.log`다.

추가 무창 WARP probe는 실제 설치 `Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel`을 CModel로 읽고 `rpcz00_att_battle_7_01` 7.4초 clip의 `bip001-spine1` index7을 직접 샘플링했다. 실제 NPC와 동일한 b_root 수평 억제, 수직 scale .8, actor preScale .012053을 적용했다. 현재 제품의 `Build_SourceAnchorWorlds`, `Resolve_TargetPivot`, `Sample_KoukuSaydonBossMotion` 함수 본문을 그대로 사용하여 FX_State_01 socket TRS와 P8의 1870~5780ms 이동을 계산했다. 120Hz 1200pose/2986 provider 호출에서 bone basis는 1.20529~1.20530, ribbon 관측494프레임, 최대19점, finite 실패0, 최초 종료6.34167초였다. 이동 구간의2091점 중1857점이 현재 emitter보다 이동 방향 뒤쪽에 남았다. WARP는 모델 로드만 수행했고 window와 draw는0이다.

같은 실제 CModel probe로 저글링 `rpcz00_att_battle_4_01`의 b_wp_1/2/3 본 index71/54/98을120Hz로 샘플링했다. bone basis는1.20528~1.20530이고 세 held mesh의1170샘플 모두 attachment anchor와 중심 오차0이었다. 생성3회·작은 폭발3회와 endpoint XZ 일치, finite 실패0, 최초 종료9.75초를 확인했다. 원본 projectile spawn offset [.2,1.5,0]과 저작 목표점·포물선의 구분은 유지한다.

증거는 `out/Gate2Restoration20260922/source-effects/source-effects-manifest.json`, `effect-probe.log`, `verification-receipt.json`, `real-rope/inputs.json`, `real-rope/probe.log`, `real-rope/poses.csv`, `real-rope/juggling-probe.log`, `real-rope/juggling-poses.csv`다. 줄타기와 저글링은 실제 설치 모델·본·socket 수치 검증까지 완료했다. GPU 화면 일치와 사용자의 최종 화면 판정은 미완료다. Projectile 사운드 네 event는 최초 catalog/Resources에 없어 오디오 담당의 원본 추출 후보에 연결했다.

## G03. 파란 보호막과 노란 시선

파란 보호막은 원본 G safezone11요소, native3270의 양면 alpha·SceneColor와 native3271의 파란 shell을 사용한다. 명시되지 않아 누락됐던 원본 EmitterLoops0을8요소에 복원했다. 실제 설치 WModel 정점에 renderer의 modelPreScale .01과 particle World를 적용한 X/Z반경1.100000024m를 확인했으며 occurrence1.81818178로 기존 반경2m에 맞춘다. P11.presentation.4/6/7의 기존 V2 decal 중심을 보존하도록 MAP Z+2.5만 옮기며 collider와 빨간 구체는 그대로 둔다.

노란 시선은 잘못 이름 붙은 기존 red eye_02그룹을 재사용하지 않는다. 원본 eye_01_01의 색(1,1,.1), alpha(.1/.2), native3051/3052/3053 및 실제 설치 MN_RPCT_06의 양안 본을 사용한10요소다. 원본 Lifetime0은 유지하고 해당 occurrence의 owner window만1.35초로 확장했다. 0/6532/12990ms의 예고 배치는 PROJECT_AUTHORED이며 원본 Action 직접 연결로 과장하지 않는다.

최신 C++ Codec/Drawable/serialize roundtrip/Stage/finite 검증을 통과했다. 실제 눈 본481poses/120Hz와 설치 mesh·preScale를 사용한 수치 검증을 수행했다. 사용자 화면 판정은 대기다.

## G04. 사운드·자막·패턴 순서

기존 애니메이션·sourceStart·playRate·사용자 SOUND 행은 보존했다. 새 연결은 P105 나팔
Action12개, P106 저글링 Action16개와 공 발사/착탄12개, P99 바람3개, P8 등장공의 대상별
발사/폭발 template2개로 총45개다. 원본 WAV16개는 네 Projectile event의 각4개 variant이며
각 확률1/4를 사용한다. 기존232개와 새16개, 총248개 WAV의 실제 header·asset/catalog·hash도 확인했다.

사운드 보충 소유자는 P105/P106/P99와 요청된 공뿐이다. 원본 notify와 사용자의 편집 시각·
반복 개수가 달라도 사용자 소유 P11/P24/P25/P85를 자동 누락으로 취급하지 않는다.
등장 공은 Effect1개+SOUND2개가 같은 SHOWTIME fixed group을 이루며, 생성 기준0/3033ms에
각 플레이어의 Server 확정 위치에서 재생된다. 기존 finite MAP SoundCue 소비자를 사용한다.
Codec/projector는 해당 Duration이 소유한 fixed MAP 그룹에만 SOUND를 허용한다.

P9의 SUBTITLE 이름은 `세이튼_등장`이며 다음 문자열을 정확히 저장했다.

> 여러분들, 뿅망치 살인마가 등장합니다!
> 과연 누가 살아남을까요?

초기19항목은 P105→저글링→등장→P105→바주카→파1빨2→잡기→팡파레→조커→거미카운터→
레이저→휠윈드→바람→2번 내려치기→불어날리기→팡파레→3번 내려치기→카드미로→피자다.
뒤의6항목은 사용자 확정대로2번→3번 내려치기→휠윈드→팡파레→바주카→불뿜기이며
`loopStartEntryId=kakulsaydon.flow.gate2.restore.loop.1`부터 반복한다.
바람과 불어날리기는 원본 Action4221804의 동일3클립으로 확인돼 P99를 두 위치에서 재사용한다.

## G05. 설치·빌드·게시 검증

`final-candidate-validation.json`에서 실제 저장 검증·재적용 동등성, 기존 모든 stage/bundle,
기존 presentation과 다른 관문 Flow 보존을 확인했다. 변경 Pattern은8/9/11/99/105/106이다.
P11은 요청된 시선/파란 구체만 변경하고 SOUND는 그대로다. 최종 Composition SHA256은
`8f70ef81e21d7f48d45da60887288049cfbec4c5be2a51be8718968389d35b76`이다.

P105는 원본 clip 시계 기준 예고·전방음표2500ms, 원형3444ms, 직선 내부3624.54ms,
공격3600~3634ms를 사용한다. 원본 SkillDecal2113의1.5×9m 노란 BOX6개는 중심4.5m와
방향45/90/135/225/270/315도를 함께 회전한다. 기존 source attack의34ms·최대HP10%는
프로젝트 튜닝값이며 원작 damage 복원값으로 주장하지 않는다. 음표 잔상을 포함한 수명은5500ms다.
실제 Codec/Playback·project_encounter와 sourceStart400ms/playRate1.25 변형도 통과했다.

원본 leaf15개와 후보5개는 현재 원본을 메모리에서 재생성해 값이 같음을 확인하고 SHA256을
고정했다. fanfare/warning 원본 입력·오디오·wind 근거도 설치 직전 재확인했다. SoundCatalog와
project/filter baseline은 후보를 계산할 때 읽은 동일 bytes로 잡는다. 기존 installer의
ReplaceFileW·백업·late-save 거부·자기 변경 rollback을 유지했다.

20:10:07 정상 Product Debug는 Engine/Shared/Server/Client 모두 **PASS**다. 최종 receipt는
`out/BuildPipeline/runs/20260922T111007048Z-debug-product.json`이다. fixed SOUND 그룹은
Python 관련25개 검사와 실제 C++ Codec16개 검사(Parse/Serialize·잘못된 입력의 기존 문서 보존)를
통과했다. 새 C++ 파일이나 별도 runtime 경로는 없다.

첫 Owner 게시에서 map publisher source closure가 변경돼 전체 Product/Server 생성물과
receipt가 이전 값으로 rollback됐다. 실제 authored revision2198과 설치 Effect/WAV는 유지됐다.
첫 실패 기록은 `domain-publish.log`이며 최신 source로 재시도한 결과는
`domain-publish-retry.log`에 별도로 남긴다. 최신 source 재시도는533197ms에 네 도메인 모두
PASS했다. `installed-verification.json`에서 authored/Encounter/patternbindings revision2198,
실제 Server bootstrap의25행·반복 기점, 등장공의 Effect1+SOUND2 및0/3033ms 시계,
정확한2줄 자막과 사용자 SOUND44행 보존을 확인했다. 설치 후 다른 세션이 추가한
EffectCatalog·project/filter 변경은 유지하고 이 작업의8개 등록 항목이 같은지 대조했다.

## G06. Play Pattern과 Complete Play 검증

최신25개 stable entry를 실제 Server 완료 전이 코드에 입력해 정확한 초기19항목과 반복6항목,
후반 두 번째 주기 뒤 loop.1 복귀까지90개 검사가 통과했다. 근거는
`out/Gate2FlowAudio20260922/complete-raid-contract/flow25.log`와 `flow25-inputs.json`이다.
이는 모든 기믹을 플레이어가 성공시킨 GPU 화면 검증과 다르다.

개별 Native 검사에서 P85 STAGE_3의 저장된 `retargetTarget: NEAREST_ALIVE`를 C++ codec만
허용하지 않아 Pattern을 quarantine하는 결함을 발견했다. 사용자 JSON이나 사운드를 지우지
않고 기존 Stage struct·strict parse/validate/serialize, Workbench의 retarget off 처리와
일반 Preview의 최근접 XZ 선택을 연결했다. default RANDOM_ALIVE와 잘못된 값 거부를 유지한다.
실제 최신 NativeCodec으로 Product96개+Sequence10개가 모두 격리 없이 expand/roundtrip됐으며
타깃 옵션과 잘못된 입력 보존을 포함한338개 검사가 통과했다.

Python의 parent 중간 파편에서 retargetOnEnter가 false일 때 selector도 제거하는2줄은
현재2198의 parent5개 출력에 영향을 주지 않는다. 수정 전/후 완전동등을
`retarget/projector-equivalence.json`으로 확인했다. 기존 게시 내용은 그대로 유효하다.

20:10 Product 성공은 이 마지막 로더 수정 전이다. 사용자가 직접 빌드·화면 검토하겠다고
했으므로 후속 전체 Product 빌드는 자동 실행하지 않는다. 필요한 실제 C++ 최소 컴파일과
Client resource closure 및1~4인 실제 GameRoom 관문 전환 결과는 아래에 기록한다.
실행 중 도구의 미저장 draft·메모리 Reload·Server 재시작·사용자 화면 판정은 자동 수행하지 않는다.

최종 Workbench·PresentationPlayer는 변경 후 최소 컴파일 exit0/error0이다. 실제
`Collect_CompletePlayResources`의 전체96개 combined closure와4개 관문별 closure,
revision 불일치·missing dependency의 기존 결과 보존까지347개 검사가 통과했다.
96개를 각각 따로 호출한 검사를 완료한 것은 아니다. G1은28 entry→19 pattern,
G2는25→23, G3는9→19, BINGO는1→1의 의존성을 정상 수집했다. 증거는
`retarget/verification.json`, `client-admission/verification.json`, `admission-brief.log`다.

현재 게시 Server DataFiles를 격리 복사한 실제 Product Server의
`--debug-teleport-contract-test`는206.943초, exit0, 전체7993 PASS/0 FAIL이다.
여기에는 다른 DebugTeleport 계약도 포함된다. 1~4인 각각 G1→G2→G3의 시작·보스 사망·
전원 승인,3관문 앙코르→빙고 진입과 실제 lifecycle 기반 빙고 반복을 통과했다.
서버 실행 파일·격리 데이터·원본 게시 데이터의 실행 전후 해시가 같다.
증거는 `out/Gate2FlowAudio20260922/complete-raid-contract/20260922T202649/result.json`,
`server.log`, `published-flow-join.json`이다. 모든 기믹의 실제 플레이 성공이나 GPU 화면을
검사한 것으로 확대하지 않는다. 코드·설치·게시·무창 계약 검증은 마무리했으며,
마지막 Client 변경의 전체 빌드와 화면 검토는 사용자가 직접 진행한다.
