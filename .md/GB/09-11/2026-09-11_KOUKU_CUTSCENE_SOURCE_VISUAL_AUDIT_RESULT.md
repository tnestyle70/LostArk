# 쿠크 컷신 원본 구조·복구 범위 조사

## G00. 조사 범위와 판정 기준

2026-09-11 사용자 첨부 이미지 3장을 직접 열어 원본 패키지와 현재 Composition 저장 데이터에 대조했다. 사용자가 말한 네 번째 책 이미지는 이번 첨부의 세 번째 이미지로 해석했다. 세 번째 관문 영상은 이번 첨부에 없으므로, 기존 영상 대조에서 식별한 SCENE02A/Matinee10을 조사하되 다른 장면일 가능성을 남긴다.

이번 작업은 조사다. 제품 C++·Data·Resources 수정, 빌드, publish, Client/UI 실행·조작·캡처를 하지 않았다. 분석용 out 자료와 이 문서만 작성했다. 아래의 원본 시간은 Matinee local time이며, 현재 편집기에서 늘리거나 자른 시간 또는 실제 영상의 wall-clock과 구분한다. 원본 Slomo 트랙이 있으므로 모든 구간을 그대로 실시간 초로 환산하면 안 된다.

원본 SCENE01A/B/C, SCENE02A/B, SCENE03A, SCENE04A, SCENE06A, SCENE07A의 9개 패키지에서 77개 Matinee 연결과 2,046개 트랙 참조를 열거했다. 트랙 목록에는 비활성 트랙과 하위 트랙도 포함된다. 단순 합계는 재생되거나 복원된 효과 개수가 아니다. 캐시에서 빠졌던34개 트랙은 실제 UPK export를 다시 열어 모두 빈 tagged property stream임을 확인했다. 키 분실34건으로 분류하지 않는다. 외부 던전 이벤트 구현, 모든 원본 shader의 실행 결과, 모든 AnimSequence의 전이 Notify까지 조사 완료했다는 의미도 아니다.

- [전체 트랙 CSV](../../../out/KoukuFireworks20260911/source_track_inventory.csv)
- [패키지별 집계와 참조 해석 상태](../../../out/KoukuFireworks20260911/source_track_inventory.json)
- [빈 트랙34개 원본 재검사](../../../out/source_track_unresolved_metadata.json)
- [카메라·책 좌표 계산](../../../out/KoukuFireworks20260911/camera_transition_audit.json)
- [1관문 책·무대·Kismet 근거](../../../out/KoukuFireworks20260911/gate1_book_stage_handoff_audit.json)
- [폭죽·포털·카드 FX 근거](../../../out/CircusCardFxSurvey20260911/circus_card_fx_audit.json)
- [3관문 배경·재질·부착 근거](../../../out/KoukuGate3BackdropAudit20260911/native_backdrop_evidence.json)
- [세 진입 Matinee의 원본 clip Notify 조사](../../../out/source_main_clip_notify_audit.json)

위 상대 경로는 저장소 root의 out 자료를 가리킨다. Scene actor/track의 package 참조는 1-based다. 파티클 그래프의 `particleSystems[].exportIndex`는 0-based이므로 각 분석 파일의 referenceConvention을 따른다.

## G01. 먼저 교정할 결론

현재 편집기의 `연출_팝업북 37.8초 → 연출_1관문 피날레 21.01초`는 현재 저작 목록이다. 이것을 원본의 전체 장면 순서라고 설명하면 부정확하다.

첨부의 광장 포털, 화면 수렴, 책 펼침은 원본 `SCENE03A → EFSeqAct_Matinee_0 → InterpData_0` 하나에 함께 들어 있다. 원본 길이는 41.487556초, InterpGroup은 223개다. 이 안에 광장, 별도 책 촬영 세트, 다른 위치의 완성 무대를 연결하는 카메라 컷이 존재한다.

따라서 다음 셋은 서로 다른 판정이다.

1. 현재 엔진에서 움직이는 맵에 재질과 실시간 조명이 적용되는가: 구현과 수치 검증 근거가 있다.
2. 원본 Matinee의 카메라·FX·광원 변화·맵 부품·후처리가 빠짐없이 연결됐는가: 아니다.
3. 원본 영상처럼 같은 밝기·색·가림·전환으로 보이는가: 사용자 화면 검증이 필요하다.

## G02. 이미지 1·2·3의 원본 연결

원본 SCENE03A의 Matinee export365, InterpData export857을 기준으로 한다.

| 원본 local time | 확인한 연결 | 판정 |
|---|---|---|
| 0초 | Director가 CM03 광장 카메라 선택 | 첫 광장 장면의 카메라 경로 |
| 2.344762~8.484536초 | Portal 그룹 → actor399 → `par_q_rpctgate_01` | 입구의 포털 후보와 원본 위치 일치 |
| 4.354424~8.484536초 | spi 그룹 → actor402 → `par_k_drain_big_01_l` | 포털 앞 흡입·금색 줄기 후보 |
| 6.479527~7.759254초 | FOV 70→180도, 카메라 약 40m 이동 | 이미지 2의 급격한 수렴을 설명하는 직접 데이터 |
| 7.4593~7.9562초 | Fade 0→1 | 화면 암전 |
| 8.7515초 | CM01 선택, Fade는 1 | 암전 중 별도 책 세트로 카메라 컷 |
| 10.218567초 | 책 `evt2_book01`, 재생률 0.7 | 책 펼침 시작 |
| 11.2182초 | Fade 0 | 책 장면을 밝힘 |
| 19.7407초 | CM02 선택 | 다음 촬영 구도 |
| 22.2438초 | CM03 선택, SL04 완성 무대 근처 | 책 촬영 공간과 다른 위치를 촬영 |
| 33.583263초 | `Cam` 이벤트 → SetCameraTarget | 카메라 제어 이벤트이며 플레이어 이동 명령으로 해석하지 않음 |
| 41.487556초 완료 | 카메라 대상 해제, cinematic mode 해제, 연출 actor 숨김, EndRemoteEvent | 연출 정리와 제어 반환 |

카메라 원본 근거는 Director1263, Fade1265, FOV1288, MotionBlur1287, Move1493이다. CM03에 MotionBlur Amount 0.1, 초반 DOF BlurKernelSize16도 저장돼 있다. 실제 UPK에서 카메라의 MotionBlur override 활성화도 다시 확인했다. 이미지 2의 방사형 줄기와 흐림은 이 조합으로 설명할 수 있지만, 한 프레임만으로 각 기능의 정확한 기여율이나 최종 shader 출력을 확정하지 않는다.

카메라 좌표 계산은 원본 Constant/Linear/Hermite·tangent와 Euler 회전, 부모13→27/14→5/26→17을 합성했다. 선택한7개 Move의 lookup은 모두 none이다. 1,293시점의 float32/64 계산 차이는 최대0.144mm였으며 이것은 원본 엔진의 실제 카메라와 비교한 오차 보장은 아니다.

SCENE03A에는 이 수렴을 직접 지정하는 PostRenderMaterial 트랙을 찾지 못했다. SCENE07A에 있는 `fx_d_brokenglass_01_tr` PostRenderMaterial은 별도의 깨진 유리 연출이다. 이번 수렴 효과의 근거로 섞지 않는다.

현재 Camera 리소스는 eye/lookAt/FOV/up을 수용하고 FOV179도 이상을 거부한다. 원본 FOV180을 단순 복사할 수 없다. 현재 V2 ZoomBlur는 이름도 `ZOOM_BLUR_RECONSTRUCTED`인 재구성 기능이다. 비슷한 화면을 만드는 데 사용할 수 있지만 원본 MotionBlur/DOF와 실행 결과가 같다는 뜻은 아니다.

현재 G1 두 Sequence의 sceneProfileOccurrences는 비어 있다. 따라서 현재 데이터에서 이 연출이 Scene Profile로 이미 연결됐다고 볼 수도 없다.

## G03. 책은 실제 전투 바닥인가

원본에서 책 배우와 완성 무대는 별도로 존재한다. 이것은 현재 프로젝트 배치만으로 한 추정이 아니다.

- 책 actor372, B01 Move1469에는 위치 키가 하나 있다. UE 좌표는 `(1978.773, -91388.742, -15449.151)`이며 프로젝트 축·미터로 바꾸면 약 `(19.788, -154.492, 913.887)`이다. 책 배우는 이 위치에서 애니메이션을 재생한다.
- 원본 SL04의 고정 바닥 `bg_rad_koukusaton_floor08_sm`, placement export206은 UE `(0, -73728, 0)`, 프로젝트 좌표 `(0, 0, 737.28)`에 있다. 별도 원본 SL04 placement 문서로 확인했다.
- 원본에는 지하 책 세트용 RPCT05 actor373과 완성 무대 쪽 RPCT05 actor374도 따로 있다. 같은 배우가 계속 같은 좌표에서 연기한다고 가정할 수 없다.
- RemoteEvent1727의 `37081_114`가 Matinee 시작과 함께 SL04 Load1714를 요청한다. 한편 연출 대상 ToggleHidden1722는 시작 때 UnHide, 완료 때 Hide된다. 타깃 참조154개는 중복을 빼면153개 actor이며 책도 포함된다.
- 광장의 별도 책 두 조각 actor520/521을 숨기는 `book_hidden` 이벤트도 따로 존재한다. 광장 책, 펼침용 책, 완성 무대의 identity를 합치지 않는다.

영상에서 맵 이동이 보이지 않는 것은 별도 세트와 카메라 컷으로 충분히 설명된다. 원본은 책이 펼쳐지는 장면과 완성된 무대가 이어져 보이도록 연출한다. **책의 움직이는 모델이 그대로 고정 전투 맵으로 바뀐다고 볼 근거는 없다.**

다만 다음은 아직 확정되지 않았다.

- 원본 플레이어를 언제 어느 위치로 이동시키는지.
- 원본 최종 전투 spawn을 누가 확정하는지.
- RemoteEvent `37081_113`, `37081_114`, EndRemoteEvent 이후 외부 던전 로직의 전체 연결.

SCENE03A 안에서 직접 SeqAct_Teleport를 찾지 못한 것은 원본 게임 전체에 플레이어 순간이동이 없다는 증명이 아니다. 카메라 위치, 연출 배우 위치, 플레이어의 게임플레이 위치를 구분해야 한다. 또한 SL04를 별도 게임플레이 근거 없이 현재 프로젝트의 G1 전투 spawn과 동일시하지 않는다.

현재 프로젝트는 펼침 복사본을 완성 무대 위치에 재배치하고, 맵 occurrence가 끝나면 standing arena를 다시 표시하는 경로를 사용한다. 현재 G1의 맵 occurrence5개는4507ms에 끝난다. 이 전환은 `Level_KakulSaydonArena.cpp`의 `Debug_SampleCompositionWorldPreview`가 수행하는 현재 구현이며 원본의 전환 프레임 증거가 아니다. 지금의 편집 데이터와 원본 전체41.487556초를 다시 대응시켜야 한다.

## G04. Sequence도 Notify처럼 효과 목록을 볼 수 있는가

가능하다. 다만 원본 효과의 발생 경로를 구분해서 합쳐야 한다.

| 원본 경로 | 조회할 정보 | Composition 연결 |
|---|---|---|
| Matinee → Toggle/Move → Emitter → ParticleSystem | 효과 asset, ON/OFF, 좌표, 부모·부착, 재시작 | Sequence Effect occurrence와 anchor |
| Matinee → AnimControl → AnimSequence → Notify | clip, offset, 재생률, reverse/loop, Notify 시간, socket | 원본 clip 시간에서 Sequence 시간으로 환산 |
| 레벨·Kismet → 다른 Matinee/Emitter | 별도 이벤트 시작, 정지·숨김, 이벤트 간 관계 | 명시적인 Sequence 이벤트 또는 시작 offset |
| 카메라·광원·재질·Fade 트랙 | 렌즈, 밝기, 안개, material parameter, 화면 전환 | Camera/World/Effect/Scene Profile 중 실제 지원 규격 |

이번 포털·흡입·플레이어 흔적은 첫 번째 경로다. 캐릭터 clip에 Notify를 새로 넣는 것만으로 원본 연결이 되지는 않는다. FX가 메시를 그리더라도 원본이 ParticleSystem이면 원본 분류는 파티클 효과다.

현재 Workbench의 Resources → Effect에서 변환·등록된 V2 Group, V1 Effect, V1 Element를 보고 Preview Source 또는 Append Effect at Cursor로 배치할 수 있다. 하지만 원본 Matinee/AnimSequence 전체를 자동 읽어서 원본 효과 목록과 현재 연결 여부를 대조하는 전용 목록 UI는 없다.

추가할 조회 목록에는 최소한 다음 열이 필요하다: 원본 scene/Matinee/group/track, 발생 경로, 원본 asset, 시작·종료, 부모/socket, 현재 Resources asset, 현재 occurrence, 미연결 이유. 원본의 꺼진 트랙, ON 없는 OFF-only 트랙, 배우가 연결되지 않은 그룹도 정상 효과처럼 자동 추가하면 안 된다. 시간은 clip local, Matinee local, 편집 Sequence local을 명시한다.

기존 fireworks 변환기의 adapter에 `notifies`라는 형식이 등장하는 것은 Matinee Toggle을 도구 입력 형식으로 바꾼 것이다. 그것을 원본 AnimSequence Notify를 전부 조사했다는 증거로 쓰지 않는다.

후속으로 SCENE03A/Matinee0, SCENE04A/Matinee2, SCENE02A/Matinee10의115개 AnimKey를 원본6개 UPK까지 추적했다. 중복을 제거한 AnimSequence41개는 모두 속성 파싱에 성공했고, 이 export들에 직렬화된 Notifies 속성은0개였다. 배우가 연결된 clip의 미해결 참조는0개다. 남은 두 미해결 idle_normal_1은 G2의 actor binding이 비어 있는 tab1/책110 그룹이다. 따라서 이번 주요 효과를 clip Notify만 뒤져서 찾을 수 있다는 가정은 원본 자료로 지지되지 않는다. 다만 각 AnimSequence의 명시 속성이 없다는 사실을 inherited default, AnimTree, CEFAction, 외부 던전 이벤트의 부재로 일반화하지 않는다.

원본의 모델 LookInfo와 InterpGroup AnimSet도 별개 참조다. 예를 들어 G1 배우의 LookInfo는 RPCT05이면서 그룹 AnimSet에는 RPCT00이 연결된다. 현재 모델에 같은 이름의 clip이 있다는 사실만으로 원본 골격·애니메이션 bytes와 블렌딩을 동일하게 복구했다고 판정하지 않는다.

## G05. 폭죽·포털·카드의 현재 연결 상태

| 항목 | 원본 확보 | 현재 상태 | 남은 작업 |
|---|---|---|---|
| 광장 폭죽 | SCENE03A Matinee7, emitter3개 | fireworks.full.restore 리소스 있음, G1 Sequence에는 미배치 | 원본 위치·별도 이벤트 시계 복구, Sequence 연결 |
| 완성 무대 축포 | main0 후반 축포·축제 효과 | festival.full.restore 리소스 있음, G1 Sequence에는 미배치 | 광장 폭죽과 구분해 원본 시간/위치에 연결 |
| 포털 | `fx_q_w_01.fx_par_02.par_q_rpctgate_01` | 원본 PS 확보, 해당 source의 Authored donor0 |25 emitter와17 material instance의 실제 실행 경로 연결 |
| 흡입 | `bfx_low_01.etc.par_k_drain_big_01_l` | 원본 PS 확보, 해당 source의 Authored donor0 |5 emitter·4 material instance 연결 |
| 플레이어 이동 흔적 | main0의4개 배우에 연결된 이동 FX | 원본 PS 확보 |6 emitter·5 material instance 및 움직임/부착 연결 |
| 2관문 카드 분출 | `par_q_cardfly_01`, 3 MeshEmitter, Matinee occurrence6개 | 현재 World Object + CModel 애니메이션으로 연결 | 원본 FX MIC override, CameraOffset, 시뮬레이션 차이 복구 |
| 2관문 나머지 FX | Matinee2 emitter binding39개 | 카드6개 외 효과 전체 연결은 아님 | 활성33개 중 카드 외27개와 OFF-only6개를 각각 검토 |

원본 main0의 emitter binding은24개이며 활성24개다. 폭죽 Matinee7은 다른 RemoteEvent `37081_113`, main0은 `37081_114`로 시작한다. 둘의 실제 상대 시작 시간을 현재 확보한 그래프만으로 확정하지 못했다. Matinee7의 첫 ON5.980013초를 main0의5.980013초에 바로 붙이는 것은 추정이다.

현재 카드 bake는3 emitter×초당6개×6초=108개를14초 WANIM으로 만든 것이다. 난수 seed4210401은 현재 저작 시 고정한 값이며 원본 실행의 동일 난수를 회수했다는 의미가 아니다. 카드 기본 BG 재질3slot은 있지만, 원본 ParticleModuleMeshMaterial의 FX MIC override와 CameraOffset은 빌더 receipt에도 미반영으로 기록돼 있다. 원본에서 이펙트인 것을 현재 World Object로 재생할 수는 있으나 이것으로 원본 파티클 복구 완료가 되지 않는다.

최신 Sequence 저장 데이터 revision4를 다시 읽었을 때 G1 팝업북은 world7+camera1, G1 피날레는 world1+camera1이고 둘 다 EFFECT0이다. G2는 world17+presentation7이며 presentation은camera5+fade1+light1이다. 이 수치는 코드가 지원하는 기능 수가 아니라 현재 연결된 occurrence 수다.

## G06. 3관문 배경은 Artist ALT+V와 같은가

기존 영상 대조에서 식별한 SCENE02A/8M6의 Matinee10은35.368065초,67그룹이다. 같은 패키지의 Matinee14는9.436066초의 거대 세이튼 등장 연출로 별개다. 현재 첨부에 해당3관문 화면이 없으므로 장면 식별을 조건으로 둔다.

Matinee10의 전이 구간은 다음과 같다.

- actor116 `par_q_darkfield_01`:8.25675~17.19185초.
- actor111 `par_q_field_01`:13.046~17.121초.
- `par_q_warpspace_01`, `par_m_glow_001`과 카메라 이동·암전이 함께 사용된다.
- darkfield/field는 `fx_sm_00.fm_b_cylinder_002` 원통 mesh다.
- MIC `bfx_d_me_field_02_02_tr` → 부모 `fx_d_pa_field_02_tr`는 `BLEND_Translucent`, `MLM_Unlit`, `bUsesDistortion=true`다. MIC에는 TwoSided override도 있다.
- 조사한 재질에는 깊이 검사 해제 필드가 명시돼 있지 않다. 생략된 기본값이나 실제 draw state를 확정하지 않는다.
- warp/glow의 TranslucencySortPriority는6/10이다. 투명 효과 사이의 정렬 단서이며 불투명 맵보다 무조건 앞에 그린다는 증거가 아니다.
- 터널 배우에는 camera base/hard-attach가 기록돼 있지 않다. 카메라 부모에 부착된 카메라와 월드의 터널 배우를 구분한다.
- MultiLevelStreaming494는 SL05를 로드한다. ToggleHidden512/513은 배우69/70/71을 숨기며 맵 전체를 숨기지 않는다.

현재 Artist ALT+V는 Sky_Mirror_SM 두 행의 `sceneBackdrop + opaque_back_depth_write`가 SceneEnvironmentReplacement를 요청하고 Map 객체가 그 요청을 소비하는 방식이다. 원본3관문의 투명·왜곡 터널을 이 규격에 바로 넣으면 재질 계약이 달라진다. **둘의 화면 인상이 비슷하다는 이유로 같은 방식이라고 할 수 없다.**

같은 부모 재질은 DimensionMaster2050230의 `effect.ue3.sd-329-native.v1`에서 사용 중이므로 재사용 후보가 있다. 목표 MIC의 static permutation, 입력과 shader 호환성 확인 전에 그대로 재사용 가능 또는 새 shader가 필수라고 단정하지 않는다.

SL05에는 텐트·커튼·벽 메시가 있고 sky/skydome/background 이름의 import는 없었다. 이것은 상위 Level의 하늘이 없다는 증명이 아니다. 현재 확보한 원본 근거는 터널 FX+카메라/Fade+도착 맵 로드이며, 하늘·맵 전체를 생략하는 Artist형 명령은 확인하지 못했다.

## G07. 재질·조명 수정의 정확한 완료 범위

기존 변경에는 실제 의미가 있다. 생성형 움직이는 맵이 Area의 mapMaterialBindings를 받아 같은 BG8 표면 계산을 사용하고, 정적 RNM/static shadow는 제거한다. 원래 SOURCE_CHARACTER였던84개 맵 광원을 UNBAKED receiver로 바꿔 비구운 움직이는 맵에 적용하면서 이미 RNM이 있는 정적 맵에는 중복 적용하지 않도록 했다. 이 수광 계약은 WARP6조합 수치 검사 근거가 있다.

그러나 이것은 원본 컷신의 조명 연출 전부가 아니다. 이번에 SCENE03A main0에서 별도로 확인한 밝기 트랙은8개다. 그중7개는 원본 방향/스포트/점광원 배우에 연결돼 있고 l08 하나는 actor binding이 비어 있다. 위치·밝기 키에는0→50 같은 순간 변화와 책 구간의 조명 전환이 존재한다. G1 Sequence에는 이 원본 동적 광원을 실행하는 presentation이 현재 없다.

추가로 FogDensity, ColorScale, 물체 재질 Opacity/Color, 빛줄기 Noise/Fresnel, Slomo, 두 AkEvent 트랙과 자막 트랙도 있다. 이들은 기본 맵 재질을 복구하는 것과 별도 연결이다.

따라서 현재 상태는 **재질 바인딩·비구운 맵의 수광 문제 수정은 반영, 원본 컷신 전체 조명·후처리 복원은 미완료**다. 움직이는 메시와 고정 메시가 같은 표면 재질을 공유해도 실시간 조명과 구운 조명의 최종 결과가 자동으로 동일해지는 것은 아니다.

## G08. 복구 작업과 사용자 화면 튜닝의 구분

| 항목 | 데이터·코드로 먼저 닫을 부분 | 사용자가 화면으로 확인할 부분 |
|---|---|---|
| 원본 순서 |41.487556초 main0 안의 광장→암전→책→완성 무대 연결 | 컷 사이 연속성, 원하는 편집 길이 |
| FX | PS·mesh·material·모듈·ON/OFF·부착·원본 시계 연결 | 색·밝기·밀도·가림·포털 흡입 느낌 |
| 카드 | FX MIC override·CameraOffset·분포/속도/수명 처리 | 카드 회전·겹침·원근·과한 분출 여부 |
| 카메라 | 원본 부모·Curve tangent·Director·FOV 의미와 투영 변환 | 프레이밍·체감 속도·멀미·화면비 |
| Fade·DOF·MotionBlur | 별도 시간 곡선과 현재 renderer의 대응 기능 | 블러 길이·포커스·암전 경계 |
| 책·맵 | 원본 배우 identity, 공간, 표시/숨김, root motion·clip blend | 연결 순간 겹침·뜸·밝기 차이 |
| 캐릭터 | 원본 LookInfo·AnimSet·slot A/B·blend·재생률 연결 | 포즈·손/소품 접촉·발 위치·표정 |
| 조명 | 원본7개 bound light의 이동·밝기 및 안개/재질 시간 곡선 | 원본 대비 명암·빛 번짐·그림자 |
|3관문 배경 | 원통 FX 재질·왜곡·깊이·투명 정렬·SL05 연결 | 캐릭터 가림, 배경 노출, 왜곡 강도 |
| 오디오·자막 | AkEvent asset과 자막 ID/시간을 실제 소비자에 연결 | 발화·효과음·장면의 동기 |
| 플레이어 복귀 | 외부 이벤트 또는 프로젝트 Server 승인·spawn·카메라 반환 계약 | 복귀 시점의 끊김과 조작 재개 |

이 표의 왼쪽에 있는 미연결이나 계산 차이를 밝기 슬라이더 등 육안 튜닝으로 숨기면 안 된다. 원본 입력이 있어도 현재 runtime의 depth/distortion/DOF 같은 실행 의미가 없으면 기능 연결이 먼저다. 반대로 shader와 구조 검사가 통과해도 최종 화면이 원본과 같다는 판정은 사용자 확인 전까지 남는다.

## G09. 후속 복구의 구체적인 순서와 남은 근거

1. 원본 scene/Matinee/track identity를 보존하는 조회 목록에서 현재 occurrence와 미연결 원인을 대조한다. 비활성·unbound·OFF-only와 실제 활성 효과를 분리한다.
2. G1은 현재 두 저장 Sequence의 이름·길이를 근거로 삼지 말고 main0의 전체 시간축부터 재구성한다. 별도 폭죽 이벤트의 상대 시작 시각은 외부 던전 이벤트 근거가 확보되기 전까지 원본 확정값으로 기입하지 않는다.
3. 포털·흡입·광장 흔적·축포와 G2의 카드 외 FX를 연결한다. 카드의 BG 재질 대체를 원본 FX MIC로 복구하고 shader/모듈 호환성을 확인한다.
4. 부모 카메라와 극단 FOV, DOF/blur, 원본 광원·안개·재질 parameter·Slomo를 연결한다. 그다음3관문 터널을 같은 기존 Effect runtime의 적절한 재질 경로로 연결한다.
5. 플레이어의 실제 전투 위치와 복귀는 원본 카메라 좌표에서 추측하지 않는다. 프로젝트에서는 Server의 승인·snapshot과 presentation/camera 반환을 명시적으로 연결한다.
6. 입력·저장·재로드·seek·중단·다시 재생의 구조 검증 뒤 사용자가 원본 영상과 같은 구간을 직접 확인한다. 이번에는 이 실행·화면 확인을 수행하지 않았다.

세 주요 진입 Matinee가 참조하는41개 AnimSequence의 명시 Notify 속성 조사는 끝냈다. 외부 던전 이벤트, 원본 runtime shader의 실행 동등성, AnimTree/CEFAction/상속 기본값의 전이 의존성은 별도로 남는 경계다. 이를 전부 복구된 것으로 기록하지 않는다. 이번 조사에서 원본 근거 확보, 현재 연결, 원본에 없는 재구성, 사용자 육안 판정을 구분한 것이 결과다.

분석 JSON parse와 문서 링크 존재, UTF-8 읽기·줄 끝 공백 검사를 수행했다. 새 문서의 `git diff --no-index --check`에는 줄 끝 정규화 안내만 있었고 공백 오류는 없었다. 제품 소스 변경이 없어 제품 컴파일·배포는 실행하지 않았다.
