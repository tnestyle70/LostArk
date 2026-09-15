# Kouku 연출 Workbench 확장 구현 결과

## G00. 현재 적용 경계

2026-09-13 작업 중 기록이다. 기존 bootstrap 시작 오류와 Effect V1/V2 독립 창 복구는
`2026-09-13_ACTION_WORKBENCH_UNIFIED_ACTIONS_RESULT.md` G08/G09를 따른다.
이번 코드·신규 read-only 연출 catalog는 소스에 반영했다. 기존 Composition과 Effect/maplights의
수정은 `out/KoukuCinematicWorkbench20260913/candidates`에 stage하며 실행 중 편집본을 덮어쓰지 않았다.
최종 Product Build·후보 적용과 사용자 화면 확인은 아직 완료되지 않았다.

## G01. 실제 연출 Animation 목록과 Pattern 자식 행

`KoukuCinematicAnimationCatalog`가 신규 Reference JSON의 원본 Scene/Matinee/group/track/key와
설치 model/clip을 읽는다. Resources → Animation의 한글 연출 tree는 1관문, 2관문, 카드미로,
3관문 진입, 앵콜과 빙고 종료의 18 actor group/111 key를 표시한다. 102개는 일반 clip Preview와
Append에 연결했고 역재생 2개·배우 미바인딩 7개는 원본 clip 정보와 진단을 표시한다.
scene 도중에 시작된 loop 9개는 원본 잘림 위상 대신 native Source In부터 재생한다는 안내를 표시한다.
그룹 Append는 시간순 편집본이며 원작의 동시 A/B track과 같다고 기록하지 않는다.

Pattern 자식 행은 기존 `Try_ExpandPatternDocument`의 실제 clip span/시각으로 표시한다. 클릭은
원본 Pattern/Stage/Occurrence stable ID를 선택하고 펼친 표시 ID를 저장하지 않는다.
loader/DataJson 실제 native 115개 검사와 원본 track/key/weight·alias·생성 재현 355개 검사를 통과했다.
증거는 `catalog_parse_probe`, `catalog_source_validation.json`이며 화면 확인은 하지 않았다.

Boss P36의 기존 13개 clip과 Logic52의 5개 window를 Sequence P4의 자식 Pattern으로 복사했다.
원본 source crop/rate/playMs와 P4의 기존 행·firework·.1 light는 그대로다. 복사된 Pattern은 독립
편집본이며 문서 간 자동 동기화 참조가 아니다. P36의 첫 제자리 walk 10277~15274ms에는
실제 SCENE03A actor 이동의 수평 길이 12.097183m를 사용해 현재 Saydon spawn 뒤에서 앞으로
오는 BossMotion을 연결했다. 현재 spawn Y/yaw를 유지하며 원본 actor의 높이·전체 시간표 복원과 구분한다.

움직이는 자식은 단일 전체 수명·비반복 slot만 허용하며 부모의 별도 이동/reset/animation/retarget와
동시 소유를 거부한다. 확장 시 motion 시각만 slot offset으로 옮긴다. 실제 Composition parser/확장/
저장 왕복과 0/500/1000ms 이동·실패 보존의 80개 native 검사가 통과했다.
`ParentMotionProbe/parent_motion_probe.run.log`와 Python 결합 후보 61개 검사가 근거다.

## G02. Animation Play 이동

Server Play에만 연결됐던 이동을 local AnimationTool과 bundle preview에도 연결했다.
`CModel`은 suppression 전 root translation을 immutable skeleton rest·실제 parent basis·model
pretransform으로 샘플한다. 기존 suppression 상태는 preview 종료/실패에서 복원한다.
`KoukuSaydonPreviewRootMotion`은 Source In/Out, rate, delay, HOLD/LOOP의 끝점 누적과 stage yaw를
절대 시각으로 계산한다. 수동 BossMotion/charge/teleport가 이동을 소유하면 자동 이동을 중복 적용하지 않는다.
Effect의 단순 Model Reference는 원래 제자리 참고 동작을 유지한다.

설치 MN_RPCT_05 WModel의 168 bone/249 clip을 실제 decoder와 null-device CModel로 읽었다.
백스텝 `att_battle_34_02`의 lateral -7.28590536m, 제자리 `walk_normal_1`의 0 이동을 확인했다.
573개 실제 CModel/helper 검사와 6개 TU Debug 컴파일이 통과했다. GPU/UI는 실행하지 않았다.
증거는 `out/KoukuLocalPreviewRootMotion20260913/IMPLEMENTATION_RESULT.md`, `probe.run.log`다.
P36의 제자리 걷기는 G01의 별도 actor 이동으로 연결했다.

### Logic 박스의 실제 Animation 블렌딩

`TRIGGER / ANIMATION_BLEND`는 occurrence의 start/duration을 그대로 기존 CModel transition에
연결한다. 연속 pose owner의 경계 하나와 1..1000ms를 요구하며 중복 Logic, 제3 owner,
기존 clip blendInMs와의 겹침을 거부한다. P36의 8ms hold gap은 허용한다. Source와 Target은
같은 Pattern clock으로 crop/rate/HOLD/LOOP를 샘플하므로 경계 이전 구간도 실제 보간한다.

AnimationTool, bundle preview, Product NPC와 SourceBone anchor는 공통 helper를 소비한다.
NPC의 semantic clip/Pattern clock을 CModel current clip과 분리했고, occurrence의 absolute start로
blend 종료 뒤 base pose도 계산하여 ActionStartTick 반올림 차이를 제거했다. AnimationTool은
행이 바뀔 때만 clip 진입 이벤트를 보내며 매 프레임 incoming target으로 재진입하지 않는다.
Product의 derived animationBlendWindows와 기존 fixedTimeline을 사용하며 Server mechanic trigger로
보내지 않는다. Python projector 및 bone bake도 같은 구간 해석을 사용한다.

설치 WModel 168 bones/249 clips에서 22,520개 CPU 검사를 통과했다. 실제 body/SourceBone pose,
source crop/delay/rate/LOOP, 역방향 Seek, 8ms gap, NPC 실제 window 메서드/Update sampling block,
블렌딩 종료 후 base pose와 out-only Save_Atomic/reopen/실패 bytes 보존을 검사했다.
root 결합 Boss 후보의 P36/P46 6개 구간과 Sequence P4 확장/P36 10개 구간도 통과했다.
실제 Python Product arrays를 C++ reader로 교차 검증하여 Boss 18개, Sequence 30개 구간
(액션별 복사 포함)의 semantic occurrence와 실제 model pose가 연결됨을 확인했다.

증거는 `out/KoukuLogicBlend20260913/IMPLEMENTATION_RESULT.md`, `probe.run.log`,
`actual_blend_samples.csv`, `source_freeze.sha256.json`이다. 변경 소비자 8 TU 컴파일과 root의
302 TU Client 격리 링크를 통과했다. helper H/CPP의 project/filter 각 1개 등록과 중복 0을
확인했다. NPC probe는 rendering/collider/weapon hook을 제외한 실제 sampling 코드 검사다.
전체 Boss의 기존 P32 duration/Effect window 문제를 전체 publish PASS로 기록하지 않았고,
GPU/UI 및 사용자의 최종 시각 판정은 수행하지 않았다.

## G03. Stop, F6와 라이트

일반/Sequence/Complete Stop은 현재 preview 시각과 pose를 유지하는 pause 경로로 바꿨다.
Reset은 기존 정리 경로이며 Complete에 Reset을 따로 표시한다. F6 free 상태에서는 카메라 track만
멈추고 현재 카메라 matrix/roll/FOV를 유지한다. follow 복귀는 현재 연출 시각을 다시 적용할 수 있다.
Complete 종료/취소가 강제로 follow를 켜지 않게 했다.

실제 Camera/Arena/Workbench 함수의 37개 native 검사와 6개 caller 연결 검사를 통과했다.
라이트의 실제 transient 상한 384를 공통 상수로 연결하여 오래된 64 제한 때문에 앞서 제출된
광원이 많을 때 연출 라이트가 빠지는 경로를 수정했다. GPU 400(기본16+transient384), Map376과
8개 예약 의미는 유지한다. 실제 provider body의 예산·순서·실패 검사 115개를 통과했다.
이 불일치를 모든 카메라 이동 flicker의 유일한 원인으로 판정하지 않았다.

MAP light .5는 Y/색/세기/disabled 상태를 유지하고 X=0, Z=737.28로 옮기는 후보를 만들었다.
Sequence P4에 별도 MAP row를 추가하며 기존 .1 light와 firework를 보존한다. 기존 resource의
MAP 위치에 row offset이 더해지는 소비를 확인했으므로 새 row offset은 0이다.

## G04. 장면 이미지 수축과 포탈 즉시 재생

Scene Profile에는 화면 수축 기능이 없었다. 기존 Effect Screen Post에
`screen.scene-collapse.capture.v1` 저작 profile을 추가했다. 기존 occurrence 시작 시 완료된
SceneHDR/SceneBloom snapshot을 보존하고 `IPresentationScreenPostMaterial`/quad/ping-pong 경로로
박스 수명에 따라 크기를 1→0으로 줄이며 바깥은 검은색으로 출력한다. bloom도 같은 UV로 옮기며
UI는 scene texture에 포함하지 않는다. 이는 사용자가 요청한 프로젝트 보강이며 미해석 원본 shader와
동등하다는 주장이 아니다. native portal material 자체를 전역 수정하지 않았다.

후보에는 단독 장면 수축, 기존 30개 native portal 요소+수축, source clock을 보존한 portal-arrival
1/2/context 즉시 재생 variant가 있다. 기존 Sequence 시간 배치를 보호하기 위해 기존 Effect ID를
덮어쓰지 않는다. P4의 별도 수축 row는 19144~21010ms이며 해당 row 시작 때의 마지막 완료 장면을
읽고 다음 scene 경계에서 끝난다. 이 시각은 기존 scene 전환에 맞춘 저작 값이다.

신규 shader의 fxc fx_5_0 컴파일과 실제 Codec Load/Validate_Drawable/Save/reload·Playback의
지연/중간/끝/역방향 Seek 검사 87개를 통과했다. `CaptureProbe/run.log`, `capture-shader.log`가
증거다. shader에는 기존 native 프로그램의 경고가 있으며 새 코드의 컴파일 오류는 없다.
GPU 표시·원작 외형 판정은 수행하지 않았다. 후보 설치는 아직 하지 않았다.

## G05. 차원술사 장면 이미지 큐브의 현재 연결

이전 09-12 RESULT의 설치 보류 기록과 달리 현재 full311/tuning26의 두 ModelCue는 이미 ALT178
native capture material을 가진다. tuning은 당시 최종 후보와 동일하고 full은 사용자 bloomIntensity만
추가로 달랐다. 실제 Resources의 cube WModel도 retime 후보와 SHA256이 같았다. 기존 값은 유지한다.
기존 Geometry/ModelCue 소비자가 frozen SceneHDR/SceneBloom을 texture2에 연결하는 구조다.
현재 view의 2D 이미지를 mesh/cube material에 넣는 것이며 6면 cubemap scene rendering과는 다르다.
현재 두 문서의 실제 Codec/Drawable/저장·재로드 검사 11개도 통과했다. `CaptureProbe/altv-run.log` 참조.

## G06. Box Set Group 이동과 겹침 표시

그룹을 선택하면 selected box 수가 증가하고 기존 drag 코드가 그 상태를 차단했다.
그룹 시간 이동은 전체의 첫 시작/마지막 끝으로 공통 delta를 제한하며 각 box 상대 시각을 유지한다.
연결 Logic은 고유 ID당 한 번 이동하고 그룹 밖 collider가 같은 Logic을 공유하면 오류를 보존하며
전체 변경을 거부한다. stale generation/선택도 기존 문서를 보존한다.
그룹 전체 interval로 연속 행 묶음을 예약하며 내부 겹친 box는 개별 하위 행에 표시한다.
실제 이동/배치/기존 공간 transform body의 76개 검사를 통과했다. IO/preview commit은 probe에서
stub이므로 실제 UI drag/Save 화면 판정을 대신하지 않는다.

## G07. 백스텝 감전빔

기존 threeway 18개 요소의 사용자 배치·속도·방향을 보존하고 ribbon 운영 capacity만 32→89로
늘렸다. 원본 lifetime/spacing의 실제 history를 수용하며 원본 reserve500을 통째로 여섯 번 예약해
문서 예산을 넘기지 않는다. 기존 32개의 길이 9.1667m가 24.1667m로 유지됨을 실제 CPU로 확인했다.
원본 width와 shader는 변경하지 않았다.

별도 `effect.kouku.gate3.backstep.electric.source.aim.preview`에는 비행·도착 폭발·라이트를
세 갈래 63요소로 연결했다. 원본 Action4219962의 MemoryPos와 reflected Projectile 속성은
Speed/MaxSpeed75m/s, 최대50m이며 이전 min/max 해석과 다르다. 독립 미리보기의 세 도착점은
명시 저작 입력으로 Effect Tool에서 편집한다. 한 도착점 변경은 대응 21행만 transactionally
바꾸며 다른 갈래와 사용자 값은 보존한다. 기록되지 않은 원작 실제 조준 위치를 복원했다는 주장이 아니다.

Codec/Drawable/CPU재생·ribbon history·도착점 변경·잘못된 범위/NaN 거부·저장 왕복을 통과했다.
원본 Spawn0/Burst0인 3개 보조 sprite는 그대로 유지한다. 증거는
`out/KoukuBackstepElectricRepair20260913/installation.json`, `CPU/candidate-validation.json`이다.

## G08. 화염포 불꽃 입자

현재 `백스탭불뿜기_확대화염포`의 native2580/fire18 6×6 atlas emitter8은 원본 SpawnRate/Burst가
0이고 30cm 이동당 생성하는 SpawnPerUnit만 있었다. 독립 문서에서 생성점이 고정돼 실제 birth가
0이었으며, 활성 occurrence 수를 불꽃 입자 수로 오판하면 이 누락을 놓친다.

실제 설치 CModel의 Att_Battle_14_02/FX_Prj02를 60Hz로 103회 샘플한 초기점 대비 이동만 기존
sourceTransformTrack으로 넣었다. source basis1.7과 사용자 scale2는 중복 적용하지 않는다.
기존 flame/full/ring.flame 세 문서에서 이 track 외 모든 값이 보존된다. 실제 재생에서 각 후보의
핵심 불꽃이 0→2회 birth, peak2로 생성되고 native alpha·dynamic·SubUV가 유효함을 확인했다.
위치 표본은 실제 본 변위와 최대9.2e-8m 차이다. 세 후보의 Codec/Save/reload/seek도 통과했다.

선택된 fire source는 Action4219940의 본 이동에 근거한다. 현재 독립 조합의 발생 입력 보정이며
다른 Action4219955의 전체 백스텝 원작 발생 구성이 동일하다는 주장은 하지 않는다.
증거는 `out/KoukuFlameRepair20260913/implementation-notes.md`, `installation.json`이다.

## G09. 부채꼴 중심 채움

기존 native3602 LocalDecal 재질에는 내부 반경 `inner` 입력이 있었다. 검증된 decal carrier도
기존 materialParameterTracks 바인더를 사용하도록 허용하고, 예고 요소에 0~1.3초 반경0→1 곡선을
연결했다. 외곽선과 세 공격 요소·사용자 값은 그대로이며 별도 외곽 element를 중복 합성하지 않는다.
직렬화된 원본 시간 곡선은 찾지 못해 `PROJECT_AUTHORED_FILL_TIMING`으로 기록한다.
기존 예고 수명1.5초와 fade를 유지하며 0/2.75/5.5/8.25/11m의 내부 반경, native packet과
저장·재생의 113개 검사를 통과했다. `out/KoukuSectorFill20260913/validation.json` 참조.

### G09 09-14 실제 설치와 사격 Sprite 제거

09-13 후보와 별도로 실제 정본을 다시 읽었다. `sector.warning.shot`은 아직 inner=.66 고정이며
예고1개+사격 Sprite3개였다. 사용자가 도넛과 같은 확장과 Sprite3개 삭제를 요청하고 EXE를
종료한 뒤, `kouku.showtime.warning.sector`의 inner 곡선을 설치했다. 삭제한 ID는
`kouku.207274005.bb19206744ae8c60c97f`, `kouku.207274005.7ec0d88461ee250102b5`,
`kouku.207274005.206ba6a1771800befa31`이다. 다른 리소스의 emitter는 변경하지 않았다.

기존 생성기에 opt-in `--stage-sector-warning-only`를 추가했다. 동일 입력의 재생성·CAS 교체와
기존 예고 값 보존을 확인했다. native3602 row0.z를0/.25/.5/.75/1로 보간해
0/.325/.65/.975/1.3초에 반경0/2.75/5.5/8.25/11m를 전달한다. 예고 수명1.5초와
fade0/.2/1.3/1.5, 반경·각도·색·배치·SourceRecipe를 유지한다. V1_ELEMENT는 제출 범위만
고르므로 동일한 material parameter track이 실제 native packet까지 전달된다.

실제 Codec 저장·재로드, parameter packing, 정방향·역방향 CPU 재생113검사 실패0,
JSON deep preservation과 생성 재실행 동일성 및 diff 검사 PASS다. Playback의 보수적인
문서 길이는 detail1.5+particle1.5=3초지만 보이는 예고/fade는1.5초다. Composition의 신규
박스 기본 길이만4000→1500ms로 맞추고 기존 박스 시각·길이는 보존했다.

설치 SHA256은 `47655414aa4c5c444a8dd156c1c2487039782f4b1ed7c076c2814e73a580ac3e`이며
근거는 `out/KoukuSectorWarning20260914/installation.json`, `run.log`, `composition-default.json`이다.
새 shader·C++ runtime·Resources binary는 추가하지 않았다. 사용자 최종 화면 판정은 대기다.

## G10. 최종 검증과 적용 경계

격리 Engine은 실제 79 TU의 변경 의존을 컴파일하고 DLL/lib 링크를 통과했다. Client는 최신
302 TU 전체 링크를 통과했다. 최초 링크에서 새 Blend helper의 프로젝트 등록 누락을 발견해
정확한 H/CPP 등록 각1개와 모든 Include 중복0을 확인한 뒤 helper 재컴파일·전체 링크를 통과했다.
정규 Debug Product Build는 Engine/Shared/Server 뒤 Client의 새 UTF-8 문구에서 C2001로 실패했다.
해당 문구를 ASCII byte escape로 고쳤다. 이후 사용자가 빌드한 Client.exe의20:23:57 수정 시각과
20:25:13 실행을 확인했으나 이를 전체 Product runner PASS로 대체하지 않는다. 변경 후보16개는
당시 SHA/freshness/JSON/기존 값 보존 검사를 통과했다. 이후 사용자 Boss 편집으로 기준본 SHA가
달라졌으므로 나중에 적용할 때 최신 편집과 다시 병합해야 하며 현재 설치하지 않는다.

Python 블렌딩 회귀10개, 결합 후보61개, P36/P46의 실제 official scoped publishable admission은
통과했다. 기존 관련 회귀15개 중2개는 변경 전에도 같은 P32 미완성 draft를 전체 validate_document에
넣어 거부되는 사례이고 나머지13개는 통과했다. 실제 공식 publisher는 dependency closure별로
검증해 unavailable draft를 inventory에 남기므로 이것을 공식 Publish 실패로 확대하지 않는다.

Local Save/Preview는 Product Publish를 자동 실행하지 않는다. 기존 정상 bootstrap34는 보존한다.
새 Boss 문서의 Server Play는 사용자가 Publish All Patterns로 게시하고 Server를 재시작할 때
동일 revision을 소비한다. Sequence는 기존 로컬 연출 저작 경계다. 사용자 시각 판정은 미수행이다.

## G11. Play All의 불필요한 보스 패턴 요구 제거

사용자가 실제 상태 문구 `has no connected boss pattern`을 확인했다. 독립 Effect까지 asset ID의
`effect.kouku.` 접두어만으로 모델을 요구한 것이 원인이다. `Select_SceneEffectTarget`에서
이 조건을 제거하고 실제 external bone 의존성과 명시 SourceModelPreview의 모델 요구는 유지했다.

실제 selector/anchor collector와 현재 centered portal, fireworks, flame, context, 작은 오망성
문서로42개 검사를 통과했다. 수정 전 centered 실패도 보존했다. 변경 TU 컴파일과 격리 Client 링크가
통과했고, 추가로 Product 기본 문자 집합의 동일 TU도 컴파일했다. 증거는
`out/PortalVisibilityReview20260913/play-all-fix.receipt.json`, `target-pentagram.log`,
`ProductCharset/compile.log`다. 이 수정은20:25부터 실행 중인 Client3000에 아직 들어가지 않았다.

## G12. 최종 축소 범위와 보류 목록

사용자는 이펙트 트리 구조를 변경하지 말라고 했고 작은 오망성의 기존 마리오 위치를 직접 찾았다.
최종 범위는 Play All 차단과 작은 오망성 폭발이다. Anchor 후속 조사는 소스 변경 전에 중단했고,
Cinematic의 깨지는 한글 문구 제거도 수정 전에 보류했다. 기존 포탈 tree reference 한 개의 이동은
이 지시 이전에 완료됐으며 그 이후 tree를 추가로 수정하지 않았다.

| 요청 | 현재 실제 상태 |
|---|---|
| Effect Tool V1/V2 독립 복구, Workbench/Sequence 공통 편집 | 기존 소스 반영 및 사용자의20:23 Client 빌드. 최종 사용자 기능 확인과 구분 |
| Play All의 보스 연결 오류 | G11 수정·검사·격리 링크 완료, 현재 실행 EXE 미반영 |
| 작은 오망성 폭발 | 두 shader case 수정·FXC·38입력 출력 비교 완료. 현재 제품 CSO 미반영. 패턴 결과 G09 참조 |
| Use Player Pos / Use Mouse Pos / 고정 MAP 앵커 UI | 미구현, 최종 범위 축소로 보류 |
| Cinematic 한글 안내 문구 제거 | 미반영, 보류 |
| 연출 애니메이션 목록 |18그룹111키 등록.102개 일반 재생/추가, 역재생2·미바인딩7은 진단 표시 |
| Animation root 이동·Logic blend·Pattern 자식 행 | 코드 반영. P36 BossMotion 및 Logic52 타입 데이터는 후보 보류 |
| Stop 현재 위치 유지·F6 자유시점·라이트 상한 | 코드·수치 검사 통과. 전체 flicker의 해결은 사용자 확인 전 |
| 스포트라이트 중앙 배치·P36 Sequence 연결 | 기존 firework/라이트를 보존한 후보만 존재, 미설치 |
| 검은 배경을 유지하는 장면 수축 | shader/profile 코드 반영. 새 Effect와 Sequence row는 미설치. combined portal 후보에는 live portal을 덮는 합성 문제가 남음 |
| 차원술사 장면 이미지 큐브 | 실제 full311/tuning26·ALT178·30Hz cube 설치 확인. 현재 view의2D capture이며 사용자 화면 확인 전 |
| 백스텝 감전빔·화염포·부채꼴 채움 | 수정 후보와 수치 검사까지 완료. 기존 데이터에는 미설치, 보류 |
| Box Set Group 시간 이동·겹침 행 분리 | 코드·수치 검사 완료, 사용자 UI 조작 확인 전 |

보류 후보를 마지막 작은 오망성 수정과 함께 자동 설치하지 않는다. 최신 Boss 편집과 저장 상태를
보존하며 사용자가 다시 요청할 때 남은 항목을 이어간다.


## G13. 2026-09-14 Play Family는 보이고 Play All은 실패하는 v15 분기

사용자는 `1관문_포탈 생성·흡입_중심 앵커`의 Mesh/Sprite Family는 보이지만 Play All은 안 보인다고
보고했다. 실제 대상 `effect.kouku.gate1.intro.portal-suction.centered`는 v15, 일반 요소30개,
runtimeCarrier0개, baked history0개이며 첫 시작은0초다. 이 대상의 실패를17초 대기로 설명할 수 없다.

`Effect_Tool_Workspace::Create_AuthoringOccurrence`는 전체 v15를 특수 runtime projection으로 보내고,
Corpus는 확장 요소가 없다는 이유로 거절했다. Family의 일반 Stage_Document는 이 분기를 지나지
않는다. G11의 보스 요구 제거는 selector까지만 검사하여 뒤의 실제 document staging 결함을 놓쳤다.

`Effect_DocumentCodec.h/.cpp`의 `Requires_DocumentOwnedRuntimeProjection`을 추가하고 Tool factory와
`Effect_Catalog.cpp`의 직접 로드·Product worker staging·Debug 등록/교체·이전 cache 일관성의5조건에
연결했다. 실제 carrier/history가 있는 v15만 projection을 준비하며 일반 문서는 기존 staging을 쓴다.
전체/선택 문서의 Validate_Drawable, malformed carrier/orphan history 거절과 실패 rollback은 유지한다.
CPP3개/H1개 변경이며 새 파일·project/filter·Data·shader·Resources 변경은 없다. 대규모 기존 dirty
worktree를 보존하고 자동 stage/commit하지 않았다.

기존 `.35`와 이 버그의 구분: Sequence의 `KAKULSAYDON_G1_PATTERN_4.presentation.35`는 현재 MAP으로
저장되어 빈 WORLD 오류 상태는 아니다. resource50의 `effect.kouku.gate1.authored.portal-arrival.2`를
참조하고 centered를 참조하지 않는다. 이 별도 원본은24요소 중4개 runtimeCarrier를 실제로 소유하므로
projection 경로를 유지한다. 현재 저장된 박스26.712초와 원본 첫 요소34.023초는 그대로다. 이번 작업에서
사용자 편집 중인 Composition·Effect의 리소스 연결·시각·좌표를 바꾸지 않았다.


G13 검증: 현재 production 의존 TU35개를 out에 새로 컴파일하고 실제 Codec/Corpus/Catalog worker/
Playback을 연결한 native 검사를 실행했다. 이전 centered projection은 정확히
`Authored v15 document contains no admitted runtime carrier.`로 거절됐다. 수정 후 actual
`Stage_ProductLoadTarget`은 projection 없이 준비하고931개60Hz history sample에서 mesh8개·sprite22개
요소 모두 양수 alpha 입자를 생성했다. 최초 mesh0.033333초/sprite0.016667초, peak93/88개,
finite particle 행30,995개다. rewind도 검사했다. `.2`의 실제 projection4개는 유지되며 잘못된
TypeData join·history를 거절하고 이전 projection을 보존했다. CPU 고정 root 수치 검사이며 실제
화면·GPU 제출 검사가 아니다. 증거: `out/EffectPlayAll20260914/native_result.json`,
`native_source_receipt.json`, `native_dependencies.log`, `native_probe_compile.log`, `native_probe_link.log`.

변경 CPP3개(Codec/Catalog/Workspace)는 현 Product의 VS18 Insiders, MSVC14.44, SDK10.0.26100.0,
Debug x64 및 기본 문자 집합으로도 별도 OBJ 컴파일에 통과했다. 기존 include의 C4819 경고는 남는다.
증거는 같은 out의 `WorkspaceCompile/compile.log`, `product_codec_catalog.log`다. 변경 파일의
`git diff --check`는 통과했고 JSON/XML 변경은 없다. 독립 read-only review에서 새 diff의 중요한
결함은 발견되지 않았다. Product 링크/EXE 교체는 수행하지 않았다. 확인한 실행 상태는
`Client/Bin/Debug/Client.exe` PID31052, `Server/Bin/Debug/Server.exe` PID54372다. 사용자의 편집을
보존하여 종료·Reload·화면 조작을 하지 않았고, 수정된 화면 판정과 제품 적용은 남아 있다.

사용자 후속 확인에서 같은 no admitted runtime carrier 문구를 직접 보고했다. 첨부 이미지는
사용자가 제공한 입력으로 열람했다. 중앙으로 모이는 긴 띠/삼각 영역과 검정 쐐기, 정상 형태의 HUD를
관찰했다. 현재 scene-collapse Post는 마지막 완료 HDR/Bloom의 고정 캡처를 사각형으로 축소하고
바깥을 검정으로 대체한다. 포탈 centered의 native2310 Sprite/emitter31은 별도 Distortion MRT를
쓰며 최종 scene resolve는 UV 이동을축당0.05로 제한한다. 이미지의 큰 늘어짐을 이 carrier 하나의
정상 효과로 확정할 수 없다. 실제 활성 occurrence와 캡처 mesh/UV 또는 캡처 입력의 추가 대조가
필요하다. 이 후속 질문에서는 재질·캡처·shader를 추가 변경하지 않았다.


## G14. 2026-09-14 19,819ms 고정 캡처·수축과 차원술사 Alt+V

### G14-01. 사용자 재현과 첨부 이미지

사용자는 포탈이 이제 보인다고 확인했다. 이후 정방향 재생에서는 화면이 긴 띠처럼 늘어나고, 1ms 단위 역탐색에서는 작은 사각형/검정 화면과 늘어진 화면이 교대로 보이며, 멈추면 늘어진 화면으로 돌아온다고 보고했다. 사용자가 원하는 전환점은 전체 Sequence 19,819ms에 촬영한 세 번째 이미지다. 아래 원본은 사용자가 제공한 파일을 보존한 것이며 에이전트가 화면을 실행하거나 새로 촬영하지 않았다.

- [정방향에서 유지된 늘어진 장면](../09-14/attachments/portal-forward-stretch-user.png)
- [역탐색 도중 잠깐 보인 작은 사각형](../09-14/attachments/portal-reverse-small-frame-user.png)
- [사용자가 지정한 19,819ms 전환 장면](../09-14/attachments/portal-cut-19819ms-user.png)

세 이미지의 HUD는 장면과 함께 변형되지 않는다. 사진의 큰 원근 늘어짐을 포탈 Sprite 수명 하나의 문제로 단정하지 않는다. 현재 저장된 camera .27/cm03은19,394→19,938ms에서 FOV97.070→177.843도로 벌어지며,19,819ms의 실제 보간값은 약167.394138도다. 다음 .28/cm04는178.9도까지 진행한다. 이 큰 FOV 변화는 세 번째 이미지에서 첫 번째 이미지로 바뀌는 늘어짐과 일치한다. 포탈의 별도 native2310 Sprite 굴절은 distortion MRT와 UV offset을 사용한다.

기존 캡처는 Effect occurrence Stage 시점의 마지막 SceneHDR/Bloom이었다. ScreenPost가 시작할 때의 캡처가 아니었고, scene resolve보다 앞에서 배경을 교체했다. 또한 정방향의 fixed-step sample과 역탐색의 exact sample, 최종 WORLD/camera sample과 이미 진행된 Effect seek·Late_Update의 시점이 달랐다. 기존 1ms 교대 현상의 모든 GPU 원인을 단일 원인으로 확정한 것은 아니다. 이번 수정은 확인된 시간·캡처·합성 경계를 같은 시점으로 연결하며, 실제 화면의 교대 소멸 판정은 사용자 확인으로 남긴다.

### G14-02. 실제 저장 항목과 전환 시간

대상은 `KAKULSAYDON_G1_PATTERN_4.presentation.36` → `kakulsaydon.g1.presentation.54` → `effect.kouku.gate1.intro.portal-suction.scene-collapse`다. 이전 문제의 .35와 혼동하지 않는다. .36 시작은15,559ms, 길이는22,071ms이며, `authored.scene-image-collapse`의 시작을 local4.260초로 바꾸면 정확히19,819ms다.

현재 저작 값은 다음과 같다.

| 항목 | 적용 값 |
|---|---|
| ScreenPost 시작 | 4.260초, Sequence19,819ms |
| 사각형 수축 소요 | 기존4.130111694초 유지 |
| ScreenPost 전체 수명 | 17.811초 |
| 검정 배경 유지 종료 | .36 연출 박스 끝, Sequence37,630ms |

수축이 끝나자마자 뒤에서 진행된178.9도 카메라가 다시 노출되지 않도록, 새 optional `captureShrinkSeconds`로 수축 소요와 검정 유지 시간을 구분했다. 이 값이0이면 기존처럼 전체 박스 수명 동안 수축한다. 포탈 카메라와 다른 curtain·particle 요소의 원본 곡선은 그대로 보존한다. 이 시간 구성은 사용자가 요청한 전환을 위한 프로젝트 저작 변경이며 원본 Matinee 전체를 그대로 복원했다는 의미가 아니다.

### G14-03. 캡처 소유와 합성

`Effect_NativeScreenPostMaterial.h/.cpp`의 occurrence별 `EFFECT_SCENE_CAPTURE_STATE`가 resolved HDR와 bloom을 각각 별도 SRV texture로 한 번 복사한다. 각 프레임의 material packet은 같은 상태를 참조하며 이미지를 다시 복사하지 않는다. 두 texture의 준비가 모두 성공해야 교체하며 HRESULT 실패를 preview까지 전달한다. 새 C++ 파일이나 두 번째 렌더러는 추가하지 않았다.

`Effect_DocumentRenderer`는 stable element ID별 캡처를 보유한다. 동일 문서 refresh는 캡처를 보존하지만 timing이 달라지면 준비 상태 조회와 실제 생성 모두 이전 캡처를 사용하지 않는다. `Shader_VtxEffectNativeScreenPost.hlsl`은 scene distortion resolve 뒤, 제품 HUD 앞에서 고정 이미지의 사각형을 축소한다. 포탈은 밖을 검정으로 채우고, cube profile은 밖의 현재 HDR/bloom을 그대로 합성한다. 화면 축소를 geometry의 비정상적인 원근 늘어짐으로 흉내 내지 않는다.

Preview는 캡처가 없는 상태로 전환 뒤를 직접 탐색하더라도 먼저 전환 시점에 WORLD·애니메이션·카메라·Effect를 맞춘다. Effect의 해당 pending seek만 typed service 경계에서 즉시 확정한다. 첫 boundary 프레임에는 캡처를 허용하지 않고 다음 Late_Update가 같은 시점을 처리한 뒤 캡처한다. 준비가 끝나면 원래 요청한 커서로 진행한다. ScreenPost A/B가 꺼져 있으면 대기를 풀고, 실패 HRESULT·상한 초과는 상태 메시지로 종료한다. Client/UI 화면을 자동 실행해서 확인한 결과는 아니다.

### G14-04. 차원술사 Alt+V

실제2050540 full/tuning 문서에 `altv.authored.starting-scene-capture`를 추가했다. profile은 `screen.scene-capture.cube.v1`, 시작0초·수명2초, target은 `altv.source.notify036.cube`다. 기존 camera mesh 두 개(emitter18/31)만 숨겼으며 ModelCue·다른 입자·재질·사용자 tint는 유지했다.

화면 밖은 현재 캐릭터와 배경이 계속 렌더링된다. 종료 크기·위치는 실제 설치 `sk_swp_cub_00_sk.wmodel`의 CModel bounds와 cue local TRS, root, 현재 camera projection으로 계산한다. 지금 소비자는 cue 시작2초의 첫 pose이며 bind bounds와 첫 pose 차이는0.00005m 이하다. 임의의 다른 animation 시점까지 지원한다고 확대하지 않고 Codec도 수축 끝과 cue 시작의 일치를 요구한다. live animation cursor를 되감지 않는다. 후속 ALT178 cube 재질은 ScreenPost가 마지막 활성 프레임에서 캡처했더라도 그 동일 texture를 소비한다.

### G14-05. 반영과 검증 경계

실제 Data의 portal combined1개, Alt+V full/tuning2개를 검증된 후보와 deep equality로 대조한 뒤 필요한 JSON 구간만 치환했다. 포탈은 해당 요소1구간, Alt+V는 visible2구간과 새 요소1구간씩이며 다른 값은 그대로다. 모든 기준본 SHA를 확인하고 백업했다. Sequence Composition은 수정하지 않았고 실행 중인 편집 도구에 Reload·종료를 보내지 않았다. 설치 기록은 `out/ScreenCaptureTransition20260914/installation.json`이다.

- 현재 production36 TU를 새로 컴파일한 실제 Codec/Playback 검사96개 통과. full/tuning/portal의 Drawable·저장 왕복·target 누락/잘못된 endpoint 거절, 화면 요소의 정·역 시각 활성 상태를 검사했다. 전체 source bone 이펙트가 아닌 해당 ScreenPost의 시간 검사는 격리한 screen 요소로 수행했다.
- WARP의4×4 synthetic HDR texture를 사용해 실제 Capture_Once의 color2/bloom3 보존, 다음 입력7/11로 바뀌어도 기존값 유지, 실패 시 부분 pair 없음이 통과했다. 게임 장면을 그리거나 캡처한 검사가 아니다.
- 설치된 실제 CModel/WModel의 cube 투영 검사 full/tuning 각49개 통과. 화면비16:9/1:1/9:16, TRS/preScale1회, near/behind/NaN/잘못된 cue 입력과 live cursor 보존을 확인했다. UV 오차는1e-7 미만이다.
- 수정 screen-post shader의 FXC 컴파일 성공. 변경 C++의 제품 기본 문자 집합 격리 컴파일을 수행했다. 최종 목록·capture boundary 후속 검사는 아래 마감 기록에 덧붙인다.

위 자동 검증은 사용자가 제공한 시각 결과를 대신하는 visual PASS가 아니다. 새 제품 EXE·DLL 링크와 실행 중인 프로그램 교체, 실제 Sequence Complete Play/Preview 및 Alt+V 화면 판정은 분리해서 기록한다.


G14 마감 추가 검증: 실제 `CEffectNativeScreenPostMaterial::Bind`, `CShader`, FXC 결과를 WARP의4×4 synthetic FP32 HDR/bloom target에 연결한49개 검사가 통과했다. 첫 준비 frame은 live 입력을 통과시키고 캡처 상태는 pending, 다음 frame에서 color/bloom pair를 고정하며, 이후 입력이 바뀌어도 center의 고정값과 바깥 live값을 독립적으로 보존한다. 수축 완료 후 검정 유지도 확인했다.4회의 offscreen draw에서 수치만 읽었으며 게임 UI/장면을 실행하거나 이미지 파일을 캡처하지 않았다. 증거는 `out/ScreenCaptureTransition20260914/GpuContract/result.json`, compile/link log, `Shader_VtxEffectNativeScreenPost.cso`다.

캡처 경계 native 검사는 실제 resolver30개와 실제 Seek/Commit14개가 통과했다. pending spawn, old Effect25초→boundary4.260초 즉시 반영, 첫 boundary capture 차단/다음 frame 허용, 실패 HRESULT 우선 처리, A/B off release와120 update 제한을 포함한다. 증거는 `out/PortalCaptureBoundary20260914/receipt.json`이다. 변경4 TU와 마지막 Player TU 재컴파일도 통과했다. 이 테스트는 첫 캡처의 시간 계약을 검증한 것이며 사용자 이미지의1ms 교대가 실제 제품 GPU에서 사라졌다는 판정과 구분한다.


## G15. 쇼타임 발사 섬광이 Play에서 사라지는 원인과 수정

[사용자가 단일 프레임에서 확인한 총구 섬광](../09-14/attachments/showtime-flash-step-user.png)을 분석했다. `effect.kouku.gate3.showtime.gun.signature`의 실제 총구 입자는40~50ms와60~70ms, glow는40~60ms 동안 산다.10FPS의 한 프레임은100ms다. 내부60Hz fixed step을 모두 처리해도 최종 상태만 그리면 그 사이에 생겼다가 소멸한 섬광은 표시되지 않는다. 실제 Codec/Playback에서60FPS의 총구4+1행과10FPS의0행을 재현했다. 따라서 이 사례는 짧은 입자의 표시 구간을 통째로 지나가는 결함이며, 긴 탄환 궤적이 보이는 것과 모순되지 않는다.

기존 `Effect_Playback.cpp/.h`에서 한 Update의 중간 step에 살아 있었지만 한 번도 표시 후보로 전달되지 않은 짧은 burst를 대표 시점의 입자 행으로 한 번 보존한다. 원본 lifetime·시뮬레이션·전체 clock은 유지한다. 다음 양의 시간 Update에서 후보를 제거하고 Update0에서는 현재 표시를 중복 없이 유지한다. Seek/역탐색은 지정 시각의 정확한 상태를 사용한다.

적용 범위는 signature, signature.world, muzzle, muzzle.world 네 asset과 실제 signature/signalshot source identity다. sprite·loop1·시각0의 단일 burst·spawnRate0·최대64 birth·75ms 이하 수명을 모두 만족해야 한다. 연속 방출·반복으로 바꾼 항목은 제외한다. 중간 평가는 해당 요소만 실행하며 한 Update의 후보는 최대256행이다. 다른 요소를 매 substep 전체 재평가하지 않는다.

대표 행의 World·Color·Dynamic·NormalizedLife·SubUV와 `fMaterialSampleTimeSeconds`는 같은 substep을 사용한다. Renderer가 문서 순서로 연속 입자 구간을 읽으므로 원래 element 위치에 삽입하고 GpuOccurrence의 count도 갱신한다. 최종 alpha0 행은 대표 행으로 교체하여 같은 sprite batch의 material 시간이 섞이지 않게 했다. 기존 실제 모델 bounds/culling helper는 보존했다.

| 실제 production Codec/Playback 검사 | 결과 |
|---|---|
| 네 asset의60FPS120회 Update | 정책을 끈 control과 모든 입자 행·시각 동일 |
|100/150/200ms 첫 Update | signature12행, signature.world6행, muzzle/world 각1행 |
| 다음 양의 시간 Update | 보존 섬광0행, 이후 control과 동일 |
| 순·역 Seek, Seek 뒤 Play, Update0 세 번 | 정확한 상태·중복 없음 |
| source snapshot, row 순서/count, material clock, 한도 | 전체70,191검사 통과 |

100ms frame의 대표 총구 sample은16.6667ms, alpha0.573468~0.936621이고 glow sample은33.3333ms, alpha0.785986/0.889617이다. 원본 shader2439/2433과 실제 DDS의 양의 alpha texel을 대조했다. 이는 표시 후보와 shader 입력의 검증이며 최종 GPU depth/coverage 검증은 아니다. probe의 B_WP_1/B_WP_2/Bip002-R-Hand는 숫자 anchor이므로 실제 본 부착 visual PASS로 기록하지 않는다.

현재 header로 관련36 TU와 변경 Particles renderer TU를 컴파일했고 native link 및 `git diff --check`가 통과했다. Effect JSON·source lifetime은 변경하지 않았다. 다른 에이전트의 독립 검토에서도 현재 네 asset 범위의 Seek·행 순서·clock·한도 회귀는 발견되지 않았다. 근거는 `out/ShowtimeFlash20260914/verification.log`, `retained.csv`, `texture-alpha.json`, `final-source-receipt.json`이다. 최종 제품 빌드는 G18, 사용자 화면 판정은 별도다.


## G16. 이동하는 외곽불의 조명 독립 색상

[사용자가 제공한 이동 불과 배경 불 비교](../09-14/attachments/moving-fire-dark-user.png)에서 이동 불은 검푸르고 배경 불은 주황색으로 보인다. 실제 D/E/F World Object는 이미 배경과 같은 BG diffuse·UV·mask를 참조한다. 세 모델은 단일 mesh/slot0이며 원본 DDS에도 주황 RGB가 있다. 문제는 WorldSequence의 동적 객체에서 위치에 종속된 RNM/static shadow를 제거한 뒤 기존 BG diffuse가 직접 scene lighting을 받던 경로다. 정적 배경의 RNM을 이동 객체에 그대로 붙이는 것은 올바른 보정이 아니다. 이전의 WMA2 emissive 오연결은 세 파일 모두 이미 고쳐져 있었다.

사용자 요청대로 `world.object.kouku.g3.outer_fire.d/e/f`의 정확한 material binding 세 곳에만 optional `unlit:true`를 적용했다. 기본은 false다. 기존 BG surface의 texture·UV·mask를 평가한 RGB를 emission MRT에 넣고 직접 diffuse/specular 기여를0으로 만든다. 화면의 tone mapping 등 기존 후처리는 유지한다. 배경의 texture 사용 방식을 유지하면서 움직이는 불의 색이 조명 곱으로 어두워지지 않게 하는 변경이다.

`WorldSequenceDocument`의 읽기·쓰기, publisher의 strict bool 검증, 기존 CModel surface override, Map material binder, static/animated/instanced shader까지 연결했다. 다른 객체·배경·조명·spotlight·WModel·DDS·배치·동작은 이번 변경 대상이 아니다. 새 Resources나 별도 렌더러는 추가하지 않았다. public 계약은 TEAM의 AREA_DATA_LAYER_GUIDE에 반영했다.

실제 문서 저장 왕복과 잘못된 타입·경로·중복 거절/기존 문서 보존12검사, 변경 consumer3 TU, FXC3개가 통과했다. 기존 publisher의 WorldSequences scope로 runtime 문서1개를 게시했고 authoring/runtime JSON이 동일하다. 세 unlit 값 외 모든 JSON 값은 이전과 같다. `MODEL_SURFACE_PARAMETERS` public 구조가 바뀌어 최종 Engine/Client를 같은 header로 빌드한다. 근거는 `out/WorldObjectFireUnlit20260914/receipt.json`이며 최종 제품 빌드는 G18에서 구분한다.


## G17. Composition 창의 프레임 저하: 확인된 병목과 실패 재시도 구분

사용자가 저장한 `profiler_20260914_014006_612_frame26_46256_0.json`의26프레임을 분석했다. droppedCpuScopes는0이다.

| 구간 | CPU 평균 |
|---|---:|
| 전체 frame |105.0756ms |
| ImGui.Tool.Composition.Build |83.346ms |
| Effect.Service.Update |1.219ms |
| Effect.Particle.Update |0.494ms |
| Effect.Occurrence.Render |1.176ms |

가장 큰 확인된 병목은 Composition UI 구성이다. 위 scope는 자식 비용을 포함하므로 합산하지 않는다. 기존 캡처에 pane 내부 계측이 없어83ms 전체를 한 함수나 파일 로드 비용으로 단정하지 않는다. GPU timestamp도 CPU 명령 공급 공백을 포함할 수 있으므로 shader 포화로 결론내리지 않는다.

`KoukuSaydonActionWorkbench.cpp`에서 kind/version/light 필터를 문자열 검색보다 먼저 적용했다. Effect 검색 때 제외될 Sound4,036행까지 먼저 검색하던 비용을 제거했다. 고정 owner 메뉴9개의 정규화는 한 번만 수행하고 실제 category는 호출당 한 번 처리한다. 본 이름 목록은 콤보가 열린 경우만 복사하며 닫힌 상태의 검사는 기존 exact Find_BoneIndex를 쓴다. dirty·저장·선택 ID·picking·명령은 보존한다. Toolbar·Timeline·Details·Patterns·Resources·PresentationResources·Resources.Filter의 Profiler scope를 추가했다.

실제 Effect metadata954행과 설치 Sound 경로4,036행의 합계4,990행을 Debug CPU fixture로 비교했다. mode/query당20회이며 기존/현재 결과 ID는 완전히 같다. 모든 owner 분류와18개의 별도 override 입력도 일치했다.

| 검색 | 일치 ID | 검색 호출/회, 이전→현재 | 이전 ms | 현재 ms |
|---|---:|---:|---:|---:|
| 빈 검색 |652 |4,990→954 |12.84160 |3.08391 |
| 섬광 |2 |18,227→6,119 |26.12820 |4.15522 |
| showtime |36 |17,995→5,887 |21.91500 |4.38901 |

이 검사는 metadata filter와 실제 owner 함수의 좁은 CPU 비교다. ImGui tree·정렬·created draft row 전체나 실제 Client FPS를 측정한 것이 아니다.83ms 전체 개선량으로 확대하지 않는다. 최종 결과는 위20회 표이며 초기에 검토만 한 map cache 후보의 수치와 구분한다.

사용자가 기억한09-12 camera 실패 재파싱 결함은 gotchas에 기록돼 있다. 현재 Ensure_CameraShotAuthoring은 첫 실패도 기억하고 Reload Cameras에서만 재시도한다. 이번 캡처에는 Kouku.CameraAuthoring.Load 호출이 없어 동일 결함 재발의 근거는 없다. Workbench·cinematic catalog 최초 로드와 inventory refresh도 실패 시 매 frame 요청을 다시 만들지 않는다.

다만 별도의 잠재 재시도 경로를 찾았다. Created Resources에서 선택한 V1_ELEMENT의 원본 이름을 표시할 때 매 frame Catalog::Find를 호출했다. 해당 loader는 성공만 cache하므로 손상/누락된 원본이면 file size/read/parse를 반복할 수 있었다. 이번 캡처에 그 파일의 실패 증거는 없어83ms의 확정 원인으로 기록하지 않는다.

이 표시는 펼친 element metadata → Find_Loaded → 저장된 element ID 순서로 바꿨다. 이름이 확인되지 않으면 Use Source Name을 제공하지 않고 Locate Source/명시 Refresh의 기존 흐름을 유지한다. 실제 표시 분기를 추출한 Catalog API mock에서 실패 source1000회 draw의 load 호출은1000→0, loaded-only는1000회였다. 펼친 metadata가 있으면 loaded-only도0회다. 실제 Find_Loaded는 기존 map 조회뿐이며 파일 I/O가 없다. 누락 element·전체 Effect 이름 유지·다른 asset의 같은 element ID 격리도 통과했다.

최종 UI TU·두 CPU probe·diff 검사가 통과했고 UTF-8 BOM 없음/CRLF를 보존했다. 입력·명령·결과 및 한계는 `out/CompositionUiPerformance20260914/profiler_analysis.json`, `validation_receipt.json`, `name_probe.result.txt`에 있다. 최종 제품 빌드는 G18에 기록하고 실제 FPS는 새 실행의 동일 조건 캡처로 확인한다.

### G17.1. 2026-09-14 20:35 캡처의 Resources 반복 구성 후속

사용자가 저장한
`Client/Bin/ProfilerCaptures/profiler_20260914_203532_103_frame13_81092_0.json`을 직접 읽어
13프레임을 재계산했다. `droppedCpuScopes`는 0이며 CPU frame 평균은 92.581885ms다.
아래 수치는 각 scope의 캡처 전체 CPU 시간을 13프레임으로 나눈 값이다. 각 scope가 14회
기록됐으므로 호출당 평균과 구분한다.

| 구간 | 프레임당 누적 CPU 평균 |
|---|---:|
| ImGui.Tool.Composition.Build | 57.650808ms |
| ImGui.Composition.Resources | 55.477423ms |
| ImGui.Composition.PresentationResources | 55.448708ms |
| ImGui.Composition.Resources.Filter | 22.112185ms |

Resources는 하위 PresentationResources와 Filter의 비용을 포함한다. 이 값을 합산하지
않으며 차액 전부를 아직 계측하지 않은 tree 생성 비용으로 확정하지 않는다. G17의 이전
26프레임 자료와 이번 캡처는 서로 다른 실행 조건이므로 두 평균의 차이를 개선량으로 기록하지
않는다. 이번 후속은 Resources의 반복 inventory 분류·filter·list·tree 구성을 조사하는 범위다.
현재 저장 Composition을 별도로 읽었을 때 Created Effect resource는 147개다.

구현은 Workbench CPP와 기존 헤더에 한정했다. category를 const reference로 읽고 owner
분류 결과를 재사용한다. Saved Effect 목록의 filter·대형 resource 복사·정렬·분류 트리는
inventory Refresh 또는 version/owner/search 변경 때만 만든다. Created Effect 목록과
트리는 draft generation 또는 version/owner 변경 때 갱신한다. 펼친 Element는 별도 목록에서
읽으며 source 목록에 매 frame 추가하지 않는다. 선택은 기존 stable source ID 조회를 사용한다.

Tree.Rebuild와 Tree.Draw profiler scope를 분리해 같은 상태에서 재구성이 반복되는지를
새 캡처로 확인할 수 있다. 독립 코드 검토에서 Refresh, Locate, Create/Rename/삭제,
Reload/Save, Element 확장 시 duration 보존과 선택 포인터 수명을 대조했으며 새로운 P1/P2
문제를 발견하지 않았다. UI/GPU의 실제 입력과 FPS 검증을 대신하는 결과는 아니다.

사용자가 EXE 종료 후 빌드까지 진행하도록 요청하여 정본 Product Debug를 실행했다.
2026-09-14 20:45:00 KST PASS, 총65.907초, Client61.572초/54 OBJ 갱신, CSO 갱신0이다.
Engine → Shared → Server → Client와 runtime 배포 확인을 통과했고 컴파일·링크 오류와
runtime input 누락은 없다. 기존 C4819/C4828 등의 경고는 남는다. receipt는
`out/BuildPipeline/runs/20260914T114500918Z-debug-product.json`, 로그는
`out/CompositionResourceCache20260914/ProductBuild`에 있다. 브랜치 전환으로 빠졌던
양손 자동 소품과 마커 가시성 최적화도 같은 제품 빌드에 포함했다.

현재 cache/filter/tree/선택 본문과 변경 전 filter를 추출한 native probe는 저장 metadata
80개와60개 filter 조건을 사용해526검사에서 실패0을 기록했다. 같은 조건 재사용,
검색/owner/version/Refresh 무효화, Created의 rename/append generation 갱신, stable ID
lookup, Element8회 전환 뒤 목록 누적 없음, Locate의 source tree 재구성을 확인했다.
ImGui drawing과 실제 draft commit lifecycle은 probe seam이며 제품 입력 검증은 아니다.
근거는 `out/CompositionResourceCache20260914/cache_probe.run.log`와
`probe_source_receipt.json`이다. 최종 C++ 입력 hash가 검사 뒤 그대로임을 확인했다.

최종 EXE는20:44:58.204 KST,58,597,888bytes이며 SHA256은
`daf38e47870e77ad2278796f0679d991e9772bbeccd7e65dde4a425843ee225b`다.
관련12개 소스와7개 TU의 compile/dependency/OBJ/link 입력 및 EXE의 세 기능 문자열을
`out/CompositionResourceCache20260914/final_build_receipt.json`에서 대조했다.

에이전트는 Client/Server를 실행·종료하거나 UI를 조작·캡처하지 않았다. 사용자
Effect JSON과 종료 직전20:42:20에 저장된 Composition revision620을 덮어쓰지 않았다.
새 EXE로 동일 Effect 카테고리 FPS, 검색·owner/version 전환, Created 이름 변경,
Locate Source·Element 펼치기·Refresh를 사용자가 확인한다. 현재 성능 자료는 수정 전
실측이며 캐시 변경 뒤의 실제 FPS 회복을 아직 완료로 기록하지 않는다.


## G18. 전체 제품 빌드와 실행 준비 완료

사용자가 EXE를 종료하고 전부 수정·빌드를 완료하도록 명시했다. Client와 Server가 모두 종료된 것을 확인한 뒤 정식 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`를 실행했다. 기존 프로세스를 에이전트가 종료하거나 새로 실행하지 않았다.

2026-09-14 KST02:10:20에 Engine → Shared → Server → Client 전체 Product 결과가 PASS로 끝났다. 총308.579초, Engine9.617초·Shared0.365초·Server0.669초·Client296.040초다. Engine13 OBJ와 Client189 OBJ 및 변경 CSO4개를 갱신했다. Server는 증분 검사 통과이며 불필요하게 다시 링크하지 않았다. 기존 C4819/C4828·native shader X4000/X4008 등 경고는 있으나 컴파일·링크 오류는 없다. 기본 Product runner는 광역 진단이나 publisher를 자동 실행하지 않으며, 이 기능의 집중 검증·게시 검사는 위 G14~G17과 Root Motion RESULT G06에 별도로 기록했다.

새 `Client/Bin/Debug/Client.exe`는02:10:18 KST,57,628,672bytes다. Engine/Bin과 Client/Bin/Debug의 Engine.dll SHA가 같고 최신 ModelAssetData.h가 EngineSDK에 동일하게 배포됐다. static/animated/instanced BG 및 ScreenPost CSO 네 파일을 실행 폴더에서 확인했다. EXE/DLL의 PE machine은 모두x64다. 필수 runtime data 누락은0이다.

포탈·차원술사 Effect 세 파일은 field 단위 설치 직후의 SHA와 여전히 같다. WorldSequences source/runtime은 공백·줄바꿈 형식은 다르지만 전체 JSON 값이 같고 D/E/F 세 binding의 unlit=true가 들어 있다. 알비온 Gameplay.bootstrap은 정상 publisher로 다시 만든 후보와 전체 바이트가 같으며 STAGE_6 상승6.556622863m와 STAGE_9/10 상승3.561095221m가 연결돼 있다. Composition을 추가로 덮어쓰거나 같은 데이터를 불필요하게 재게시하지 않았다. JSON/XML parse와 마지막 git diff --check가 통과했다.

증거는 `out/BuildPipeline/runs/20260913T171020124Z-debug-product.json`, `out/ScreenCaptureTransition20260914/product-build.console.log`, `ProductBuild/`의 binlog/log 및 `final_receipt.json`이다. `installation.json`의 runtimeBuildPending도false로 갱신했다. 이전 G14의 제품 빌드 대기 상태는 이 절에서 완료로 갱신하며, 사용자 시각 판정까지 완료됐다는 의미는 아니다.

### 사용자가 확인할 실행 순서

현재 Server CMD와 Client는 모두 종료 상태다. 사용자가 `Server/Bin/Debug/Server.exe`를 먼저 실행하고 이번에 빌드한 `Client/Bin/Debug/Client.exe`를 실행한다. 별도 바로가기의 존재나 target은 이번 작업에서 확인하지 않았으므로 위 검증된 실제 실행 파일 경로를 기준으로 한다. F5/Ctrl+F5는 VS 설정에 따라 다시 빌드할 수 있으므로 무빌드 실행으로 설명하지 않는다.

1. Lobby → KoukuSaydon → F1 → Composition Workbench를 열어 닫힌 상태/열린 상태 FPS와 검색 반응을 비교한다. 같은 조건으로 저장한 Profiler에서 새 ImGui.Composition 하위 scope를 비교한다.
2. Sequence의 해당 포탈 연출을 Preview와 Complete Play로 재생한다. 전체19,819ms에서 고정 장면 수축이 시작되고, 전·후1ms 탐색과 정지에서 이전 두 화면의 교대가 사라지는지 확인한다.
3. 차원술사 Alt+V에서 사각형 캡처가 실제 큐브로 줄어드는 동안 바깥 캐릭터·배경이 계속 보이는지 확인한다.
4. 알비온 감전 장판의 STAGE_6 상승과 STAGE_7/8 착지, 쇼타임 발사 섬광의 연속 Play 표시, 외곽불 D/E/F의 주황색을 확인한다.

에이전트는 Client/UI 실행·조작·화면 캡처를 하지 않았다. 실제 개선 FPS, GPU에서의 섬광·불 색상, 포탈 교대 현상 소멸의 최종 판정은 사용자의 새 실행 확인으로 남는다. 구현·설치·전체 빌드는 완료 상태다.


## G19. PR380 main 충돌 병합

사용자가 JSON은 우리 작업을 정본으로 유지하도록 지시했다. clean 작업 트리160b7a2a에 origin/main de223741을 병합하고 충돌6개를 해결했다. runtime/authoring WorldSequences와 Sequence Composition 세 JSON은 병합 전 우리 HEAD의 Git blob과 바이트 단위로 같게 유지했다. 세 Markdown은 양쪽 독립 추가 내용을 보존하고 중복·충돌 표시를 정리했으며, 상충한 Object Tool 사용법은 현재 Action Workbench 통합 계약을 유지했다.

JSON3개 parse와 원본 blob 일치, 프로젝트 XML4개 parse, Sequence camera62개·WORLD instance49개 참조 존재, 미해결 index0 및 staged diff --check를 확인했다. 자동 병합된 Albion root query·포탈 capture boundary·불 unlit binder/shader는 우리 기존 구현과 동일함을 읽기 전용으로 대조했다. 이번 충돌 해결에서 제품 소스나 JSON의 추가 구현은 하지 않았으며, 전체 빌드/실행은 재실행하지 않았다. G18의 빌드는 병합 전 수정본의 검증 기록이다.
