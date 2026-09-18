# 세이튼 카드·트럼펫·공과 돌진 잔상 구현 계획

## G00. 현재 기준과 보존

기준은 `bddacace2296c1c38e016146a13fcd767cdf82da`와 현재 작업 폴더다. 기존 셰이더 분리 작업, 사용자가 저장한 `sk_06_7`, `sk_13_1_loc_int`, `sk_13_3_loc_int`, Composition의 위치·타이밍 변경을 보존한다. 초기 diff는 `out/sayton-pattern-20260917/session-baseline.patch`에 기록한다. 자동 stage/commit은 하지 않는다.

첨부 다섯 이미지는 다이아·카드·폭발·몸체 잔상·8방향 레이저의 진단 입력이다. 사용자 요청의 6114, 4184, 6637, 1696, 1172, 3596ms는 저작 타이밍으로 구분한다. 원작 emitter·mesh·texture·animation 데이터는 기존 추출 자료와 설치 모델에서 확인한다.

## G01. 카드 생성과 Server 추적

기존 Composition DURATION에 `PURSUIT_PROJECTILES`를 추가한다. Effect resource ID 1~4개, 접촉 Effect resource ID, 속도·반경·생성 반경·발사 간격·발사 개수·수명·추적 여부를 저장한다. `lifetimeMs=0`은 한번 생성하는 추적 카드에만 허용한다. 일반 패턴 종료 뒤에도 카드는 추적하며 접촉, 대상 소멸, 명시 Stop, owner/world 종료가 수명을 끝낸다.

기존 publisher → Server bootstrap → `CCombatObjectRuntime` → spawn/snapshot/HIT_PULSE/despawn → `CClientReplication` → `CKoukuSaydonPresentationPlayer`를 확장한다. Server가 이동과 swept 접촉을 결정하고 Client는 복제된 위치와 반복 애니메이션, 접촉 위치의 자연수명 폭발만 재생한다. 새 packet이나 두 번째 combat runtime은 만들지 않는다.

Client의 Logic parser/validator/serializer와 편집 화면을 같은 변경에 연결한다. publish는 Effect resource를 실제 asset으로 resolve하고 Client의 `targetedCombatVisuals`에 고정한다. 기존 SHOWTIME의 고정·추적 동작은 보존한다.

## G02. 원작 Effect와 패턴 조립

주사위·상단 다이아·폭발과 stage_4 애니메이션을 세이튼 앵커로 묶는다. 하트·클로버·다이아·스페이드는 독립 Effect로 등록하고 카드출력·카드폭발을 공유한다. 회전 카드의 source emitter 개수·속도·방향을 조사하고 사용자 요청의 랜덤 방향과 원작 수치를 구분한다.

트럼펫은 두 4방향 묶음의 45도 차이를 보존해 8방향을 구성한다. 4184ms에 레이저와 `sk_06_9_loc_int`, 6637ms에 `sk_06_2_loc_int`가 연결되도록 통합한다. 기존 공 모델과 굴리기·먹기 애니메이션을 재사용해 1696ms와 1172ms, 3596ms의 동작을 연결한다.

## G03. 세이튼·발탄 돌진 몸체 잔상

첨부 흰 윤곽은 centerline Trail만으로는 몸체 형상을 보존할 수 없다. 기존 `CModel`의 실제 palette와 world pose를 제한된 수로 저장하고 공용 helper가 짧은 흰 잔상을 제출한다. 세이튼과 발탄은 Server 재생의 돌진 창에서만 이를 활성화한다. 실제 원작 program을 확인한 범위와 화면을 바탕으로 구현한 표현을 결과에서 구분한다.

필요한 새 helper H/CPP는 Client 프로젝트와 filters의 기존 물리 폴더에 등록한다. 모델 상태와 shader resource 복원, 표본 수·수명 상한, Stop과 actor 해제를 확인한다.

## G04. 검증과 실행 준비

변경 JSON의 parse와 기존 domain publisher, 실제 Client 문서 codec, Server의 추적·접촉·정리 테스트를 실행한다. 변경 C++/HLSL은 정상 증분 Product Build로 확인한다. `git diff --check`와 프로젝트 XML parse를 확인한다. Client/UI 실행·화면 캡처·최종 visual 판정은 수행하지 않는다. 실행한 검증, 실패, 제품 출력 경로와 사용자 클릭 경로를 RESULT에 구분해 기록한다.

## G05. 사용자 입장 실패의 shader admission 수정

사용자 첨부 화면은 `loading.target-resource-load / KoukuSaydon: server-approved character rendering`에서 실패했다. 호출자는 Loader::Ready_Character_Rendering → Ready_AnimatedMeshShader → CShader::Create이며 source pass admission 실패는 Lobby에 남는 기존 실패 소비자로 전달된다.

추가한 ChargeAfterimage pass14가 기본/SourceGroup 모두 BASE로 선언됐지만 CShader::Stage_ProgramVariants는 기본 BASE의 파생 counterpart가 UNAVAILABLE인 계약을 요구한다. 설치된 CSO와 실제 CShader 경로로 재현한 뒤 pass14에 기존 기본/파생 정책을 적용한다. shader validator를 완화하거나 캐릭터 준비를 건너뛰지 않는다. 기존 pass0~13과 원작 native/material 수식, 새 잔상의 색·수명·geometry는 유지한다.

기존 SourceCharacterShaderVariantProbe를 확장해 실제 설치 파일의 생성 실패를 재현하고 수정 후 기본 shader 생성, selector가 남은 상태의 pass14 실행, clone의 상수·512-bone 전달, 파생 pass14 직접 실행 거부를 headless로 확인한다. window/draw/Client 실행은 하지 않는다. 정상 증분 Product로 CSO를 배포하고 같은 설치 산출물에서 admission을 다시 확인한다.

## G06. 사용자 Effect V1 재생 실패와 미연결 범위 수정

사용자가 실제 Effect V1에서 트럼펫_카드장판 Play All의 `Document-owned runtime carrier kind is unsupported`를 보고했다. 현재 Beam2는 document codec과 실제 Playback::Sample_Trail이 지원하지만 document-owned projection 선택은 모든 carrier를 포함하고 projection builder는 Beam을 지원하지 않아, 유효 문서를 실행 전에 거부한다. 기존 Beam 직접 소비와 projection이 필요한 Ribbon/baked history 경계를 맞추고 혼합 문서에서도 정상 geometry와 검증·rollback을 보존한다. native carrier를 삭제하거나 잘못된 입력을 허용하는 방식으로 우회하지 않는다.

기존 회전 카드 네 종은 단일 visual이고 다발 발사는 P48 Server Logic에만 연결되어 있다. 사용자가 요구한 Effect 하나의 Play All에 맞는 통합 발사 문서를 추가한다. 원본 카드 emitter·재질·회전과 사용자 수정 위치를 보존하고 실제 현재 clip chain의 발사 창, 네 atlas 문양, 진행 방향·속도, 종료·공유 폭발을 같은 재생 시계에 연결한다. 서버의 충돌·추적 권위를 preview가 대체하지 않으며 시각 방출과 gameplay 객체가 중복 생성되지 않게 소비자를 구분한다.

돌진 잔상은 현재 Server 패턴의 CSkeletalAfterimage helper에만 연결되어 있다. V1 asset/Play All 및 저장·재생 경로의 소비자가 없는 상태를 완료로 부르지 않는다. 기존 helper를 재사용할 V1 모델/clock/Stop/seek 연결과 원작 TrailGhost 근거 범위를 조사하고 필요한 데이터·runtime을 연결한다. 원작에서 확인하지 못한 색·수명·shader 값을 Full Restore로 표시하지 않는다.

사용자 편집으로 Composition은 현재1189이며 이전1186 문서로 덮어쓰지 않는다. 후보와 재생 검증을 먼저 준비하고 현재 미저장 draft 여부를 확인한 뒤 필요한 항목만 반영한다. Client/UI 자율 조작·캡처는 하지 않으며 실제 사용자 오류를 재현하는 기존 codec/catalog/projection/playback 경로를 검사한다.

## G08. 사용자 삭제 오류와 공유 앵커 회전

삭제는 현재 선택/mark와 legacy isolation뿐 아니라 활성 Sequencer transient의 previewElementIds까지 같은 staged 문서에 맞춰 정리한다. 전부 삭제된 선택 preview는 남은 전체 Effect로 전환하며 stage 실패는 이전 재생과 선택을 보존한다. 없는 ID를 무조건 허용하지 않는다.

Following bone 그룹은 현재 평균 Element 위치와 별개로 공유 SocketLocalTransform의 위치·회전을 편집한다. 이 변환은 source recipe와 각 요소의 상대 위치·회전을 유지한 채 앵커 원점에서 전체를 회전시킨다. 같은 runtime slot을 다른 그룹이 공유하면 일부만 변경해 conflicting socket을 만들지 않고 거절한다. 기존 그룹 중심 회전은 별도 기존 기능으로 유지한다. 사용자가 저장한 최신 카드 위치·각도를 덮어쓰지 않는다.


## G10. 공먹기 타깃 착지와 훌라후프 초기 높이

- 소유: `server_patterns`. 기존 `ALBION_AIRBORNE`의 JUMP → SELECT_PLAYER → APPEAR_PLAYER → SLAM 소비자를 재사용한다. 새 packet이나 두 번째 이동 경로는 추가하지 않는다.
- P79 logic75는 기존 Albion과 같은 13.6788133052 m, 200 ms 상승이다. logic76의 8937 ms에서 한 플레이어를 선택하고 같은 시각 APPEAR_PLAYER와 SLAM이 그 위치와 높이를 이어받는다. `17_05`의 남은 18.6698725762 m 하강 곡선을 현재 높이에서 지면까지 정규화한다.
- stage5의 startOffsetMs=3099를 포함한 실제 `17_05` 전역창은 10299~12966 ms다. 8937 ms 타깃 이동 후 이 창까지 높이를 유지한다. 원본의 약 1.02725 m lateral root sway가 타깃 위치를 벗어나지 않도록 SLAM의 XZ는 착지 시작 위치로 고정하고 Y만 native 하강에서 소비한다.
- P84 logic77은 JUMP의 `airborneDurationMs=0`을 즉시 초기 높이 설정으로 사용한다. 실제 설치 모델에서 큰 하강은 stage10 `13_start`의 11.1910782640 m이며, stage11 `13_loop`는 약 0.005624 m 흔들림이다. stage10 시작에 SLAM을 합성한다. 기존 양수 JUMP 상승 동작은 유지한다.
- SLAM의 XZ 고정·하강 보정은 source Stage에만 적용한다. 다음 Stage부터 같은 native root 소비자가 재개하여 P84 `13_end`의 원본 16.7441587758 m 상승을 보존한다. 기존 Albion 최종 Stage의 착지·되감기와 P79 타깃 고정은 회귀 검사한다.
- Client 미리보기는 clip 이름의 Albion 전용 제한 대신 실제 native curve의 유효한 창과 남은 하강으로 검증한다. Server와 publisher는 같은 0 ms 초기 높이 계약을 검증하고, 같은 시각의 선택/출현/착지 순서를 보존한다.
- 최신 Composition은 절대 덮어쓰지 않는다. out 후보는 source SHA/revision을 기록하고 logic75/76/77과 필요한 occurrence만 semantic patch로 제시한다. P79의 비어 있는 stage4 33 ms를 앞 stage hold로 흡수할 때 기존 animation/effect/logic/world 전역 시각이 동일함을 검증한다.
- 검증: 실제 native 모델 곡선 측정, Client parser/save 및 PreviewRootMotion 샘플링, 기존 Server GameRoom commit/높이 샘플러, 실제 publisher bootstrap reader를 격리 out에서 확인한다. 제품 빌드·Client 실행·화면 확인은 사용자가 수행한다.

## G11. 훌라후프 원본 라이브러리의 Catalog 누락

`sync_kouku_effect_tree.py`가 V1 tree만 추가해서 Kouku 원본 521개가 Catalog metadata 없이 노출됐다. `Refresh_V1ResourceDuration → CEffectCatalog::Find → Load_DirectAuthoredSourceDocument`는 이 누락을 정상 거부하고 있다. 임의 파일 fallback이나 가짜 duration을 추가하지 않는다.

동일 sync가 기존 분류를 유지하며 모든 Kouku V1 참조의 정확한 DIRECT_AUTHORED_DOCUMENT 행을 함께 stage하도록 확장한다. 파일명·schema·ID·버전·source contract를 확인하고 기존 Catalog 행의 충돌을 거부한다. 설치는 Authored 입력과 Catalog/Tree 기준 bytes가 바뀌지 않은 경우에만 진행하며 Catalog를 먼저 기록해 Tree가 metadata 없는 새 참조를 공개하지 않도록 한다. 기존 원본의 VS None/96.DataFiles 누락도
동일 transaction의 project metadata 후보로 추가하되 기존 항목은 재배치하지 않는다. 두 파일 중 실패한 쓰기는 이번 작업의 bytes와 일치할 때만 rollback한다. 이번 사용자 세션에서는 미저장 편집이 있으므로 out 후보만 만든다.

검증은 격리 Data에서 실제 Catalog Load/Find와 drawable/Playback stage를 사용한다. 등록 전 오류와 등록 후 원본 훌라후프의 유한 duration을 대조하고, 누락 521개 전체의 codec/runtime admission을 검사한다. invalid 문서·기존 row 충돌·동시 파일 변경은 원본 보존을 확인한다. 새 C++ 파일은 추가하지 않으며 Workbench의 model cue duration은 기존 Effect_ModelCueEndSeconds로 잔상 tail을 포함한다.

## G13. 쿠크 거미카운터의 공통 파란 링 연결

사용자가 선택한 발탄 `effect.valtan.project-tuned.sequence.trash`의 `mesh_particle_11`을
별도 `effect.kouku.common.counter.ring`으로 복사한 후보를 사용한다. 최신 Composition의
P15 `쿠크_거미카운터` presentation4/5/6에 연결된 기존 cyan decal resource42만
`kakulsaydon.effect.counter.ring`으로 교체한다. 시작1600/8767/15934ms, 기간1680ms,
scale0.2와 anchor/follow, stage·카운터 판정·다른 Effect는 보존한다. 원작 Action4219776의
CounterAttack notify 시각은 근거로 기록하고 사용자 타임라인을 원작 시각으로 덮지 않는다.

미저장 편집을 보존하기 위해 live Data를 쓰지 않는다. 현재 revision/hash와 stable occurrence별
expected/proposed를 가진 out semantic patch를 작성하며 전체 Pattern 교체는 하지 않는다.
실제 Composition Codec의 parse/validate/serialize/save와 publisher의 P15 의존 closure를
격리 후보에서 검증한다. 새 링 자체 Codec/원본 일치 검사는 해당 asset 담당 결과와 연결한다.


## G14. 종이비둘기 네 마리와 피자 원본 carrier 후보

`build_kouku_dove_pizza_candidates.py`는 원본 비둘기 mesh emitter 네 개와 종료 폭발19개를
하나의 V1 문서로 조립한다. 기존 velocity/orbit/death-event를 재사용하며 별도 group runtime은
추가하지 않는다. `Effect_Playback.cpp`의 frame과 event가 같은 Orbit 위치 계산을 소비하게 하고,
PortableRuntime은 실제 지원한 `buseorbitoffset` 범위만 허용한다. 네 마리의 원형 비행과 약3초
종료 위치 폭발, occurrence 회전에 따른 전체 궤적·폭발 위치의 공변성을 CPU로 검사한다.

피자는 원본 `bigarea_exp_02`와 `03`의 재질·mesh pre-rotation·sector mesh를 유지한다.
비어 있는 cooked distribution과 CDO의 차이만 복구하고 전체 asset의 강제90도 회전이나 공통
shader 변경은 하지 않는다. 원본 sector 합집합의 안전 간격 약85.2도와 요청의 정확90도를
구분하며, 정확90도 mask 구현으로 기록하지 않는다.

신규 C++ 파일·ABI·프로젝트 등록 변경은 없다. 후보 JSON과 catalog/tree semantic entry만
out에 만들고 Load/Drawable/roundtrip/Stage, 실제 mesh 평면과 속도, Orbit frame/event 위치를
검사한다. 사용자 미저장 편집을 보존하며 live 등록·제품 빌드·화면 확인은 별도로 대기한다.

## G15. 사용자 공통 카운터 링과 돌진 FX의 소비자 분리

사용자가 확정한 B 문서 `effect.valtan.project-tuned.sequence.trash`의 `mesh_particle_11`을
`effect.kouku.common.counter.ring`으로 복사해 `세이튼_카운터 / 카운터 이펙트` 후보를 만든다.
원본 visual 전체와600ms particle 수명을 유지하고, 실제 출생 지연까지 포함한617ms를 resource와
새 P80/P81 occurrence 길이로 사용한다. G13의 P15 기존1680ms occurrence는 변경하지 않는다.

돌진의 dust·양팔 trail·light·dash streak21개와 기존 몸체 잔상을 묶은 V1 PlayAll 후보를 만들되,
P80에는 modelCues가 없는 FX-only 후보만 추가한다. 실제 Pattern은 기존 CNpc 몸체·무기 잔상
helper를 계속 사용하므로 같은 잔상이 두 번 생기지 않게 한다. 기존 실제 model/bone/source-anchor
소비자를 사용하며 saved clip과4366ms owner 종료를 유지한다.

두 builder는 out 후보와 semantic patch 재료만 작성한다. 실제 설치 CModel의 본·basis, Effect
codec/Playback, G13의 numeric occurrence/next-ordinal Composition 검증을 연결한다.
원본·이전 afterimage 후보는 삭제하지 않고 실제 등록과 사용자 화면 판정은 대기한다.

G15의 numeric patch 검증은 revision1214 snapshot을 대상으로 한다. 사용자 저장이 계속 바뀌므로
이전 후보를 현재 Composition에 덮어쓰지 않는다. 사용자가 편집을 저장한 뒤 최신 revision/hash와
ordinal을 다시 읽어 semantic patch를 재생성·검증하고 필요한 행만 병합한다.

## G16. 최신 사용자 저장본에 전체 대기 리소스 등록

사용자가 2026-09-17 대기 항목 전체 반영을 지시했다. 최신 저장본 기준 반영 승인 후
Client 실행 여부와 무관하게 Data를 병합한다. 등록은 out의 전체 snapshot을 교체하는 방식이
아니라 최신 Composition/WorldSequence/Catalog/Tree의 stable ID별 추가와 expected/proposed
필드만 병합한다. 기존 카드출력 회전, occurrence 배치, 공먹기511ms·scale0.5는 보존한다.

신규 V1/World 파일과 Catalog/Tree/Composition resource, 프로젝트 None/filter, 발탄 animation
sidecar를 함께 stage하고 실제 Codec/JSON/XML 검사 뒤 원본 bytes 변경이 없을 때 commit한다.
실패 시 이번 transaction이 쓴 bytes만 복구한다. 정식 Kouku/Gameplay/World/Map WorldSequences
publisher의 실제 소비 경로를 확인해 필요한 생성물을 갱신하며 사용자 빌드·화면 판정은 구분한다.

P79는 추가 요구대로 Play 시점 선택한 플레이어의 navigation 바닥점을 고정한다. 기존 Albion은
APPEAR 시점 위치 사용을 유지하고 P79만 SELECT 정책을 쓴다. 선택점에 기존 stationary combat
object와 targeted visual group을 연결하여 원형 예고·원형 폭발·두 도넛 및 착지가 같은 점을 쓴다.
현재 group.9의 시각8907/10662/11423/11978ms를 보존하고 고정 MAP 좌표는 group 상대 좌표로
투영한다. 새로운 Shared packet이나 별도 이펙트 런타임은 만들지 않는다.

## G19. 카드 출력의 source 회전축과 카드 추적 소비 연결

카드 출력의 source5개는 `EPAL_Rotate_Z`이며 Playback이 Client의 ROTATE_Y로 변환한다.
현재 GeometryHelpers의 ROTATE_X/Y는 일반 카메라 billboard로 떨어지고, emitter 회전 옵션도
fixed axis만 허용한다. Particle Billboard를 끄면 source pivot/axis 재구성 전체를 건너뛰어
단면 quad가 보이지 않을 수 있다. Local Space는 출생 root의 고정 여부이므로 이 결함의 해법이 아니다.

기존 GeometryHelpers::Make_ParticleSpriteWorld에서 source 문서가 followEmitterAxisRotation을 명시한
ROTATE_X/Y/Z만 해당 축을 유지하면서 카메라를 향하는 공통 계산으로 처리한다. opt-off의 기존
ROTATE_X/Y billboard와 ROTATE_Z 계산은 보존한다. 실제 quad front normal을 보존하고 view와 축이 평행한
경우에는 finite한 perpendicular basis를 사용한다. 기존 followEmitterAxisRotation 옵션을 회전축에도
적용하고, Playback은 world-space 출생 root를 전달하며 UI는 원본 axis lock을 구분해 설명한다.
새 public field나 shader ABI는 추가하지 않는다. source 카드 출력의 facing/local-space 설정만
guarded patch로 원본 정책 및 회전 옵션에 연결하고 사용자 TRS·타이밍·재질·source 모듈은 보존한다.

최신 P78의 Logic73, 네 Card Effect, Contact explosion, Server swept collider 및 reliable burst
소비를 확인한다. 실제 저장한 일반 Effect occurrence와 Server 카드 template을 구분하며 중복 표시와
targeted visual revision 누락이 있는지 검사한다. 공용 owner/presentation 경로를 재사용하고 사용자
저장본의 연결 변경은 root 등록 transaction에서만 한다.

검증은 실제 GeometryHelpers 함수의 축/전방/pivot/180도 반전/기울기/카메라 평행/출생 root 수치,
최신 Effect Codec 및 Playback, 변경 TU 격리 컴파일, 실제 publisher의 P78 closure와 Server 접촉
계약으로 한정한다. 라이브 Client 실행·캡처와 visual PASS는 하지 않는다.

## G18. 주사위 손 부착·비행·착지 크기와 궤적 연결

`build_saydon_card_pattern_groups.py`의 주사위 조립만 수정한다. 원본 action4219840 stage0의
notify005/010/011/015, 설치된 `MN_RPCT_05`의 실제 손 본·preScale, `fm_l_dice_01_sm`의 정점을
측정하여 네 주사위 carrier의 위치와 축별 크기를 비교한다. 상승·낙하 LocationDirect의 비어 있는
ScaleFactor는 원본 Engine CDO의 nested distribution을 확인한 뒤 누락된 입력만 복구한다.
본 부착과 root snapshot의 배율·방향 차이는 해당 occurrence의 source 기준과 현재 CModel
소비식으로 해결하며 전체 이펙트나 공용 shader에 강제 회전·배율을 적용하지 않는다.

원본 notify 시각·원본 궤적을 별도로 보존하고, 손에서 상승으로 전환되는 지점과 낙하·착지
경계의 실제 CPU 중심·mesh 축별 치수·연속 이동을 검사한다. 카드출력과 다른12문서는 변경하지
않는다. 최신 주사위 문서의 hash와 expected/proposed 필드별 guarded patch를 out에 만들고
Codec/Drawable/roundtrip 및 실제 CModel+Playback 검사 뒤 보고한다. 사용자의 저장·미저장
편집과 live Data는 보존하며 제품 설치와 사용자 화면 판정은 root의 별도 단계다.

### G18 추가 — 현재 주사위와 상단 다이아의 1.5배 크기 후보

최신 저장본의 손·상승·하강·착지 mesh 4개와 상단 symbol_109 다이아 sprite 3개만
`detail.particle.sourceScale.size`를 현재값의 1.5배로 변경한다. 실제 모델 cue는 없으므로
보스 sourceModelPreview와 손 owner 배율은 바꾸지 않는다. 위치, source 궤적, 부착,
타이밍, alpha, 회전과 다른 부속 이펙트는 그대로 유지한다. out/CardDiceScale20260917/dice에
입력 bytes/hash, 7필드 guarded patch와 후보를 저장하고 실제 CModel/Playback의 중심·본
행렬 불변과 크기 비율, Codec/Drawable/roundtrip을 검증한다. live Data와 builder는 수정하지 않는다.

## G20. 피자 부채꼴 source 조합과 오른손 지팡이 끝 비교 트레일

`build_kouku_dove_pizza_candidates.py`의 pizza 생성만 변경한다. 사용자 이미지의 검정·무지개
경계 세 곳 공백을 원본 exp03 emitter별 mesh, StartRotation, TypeData pre-rotation, material
mask와 방출 횟수로 대조한다. exp02 원형 링은 사용자 허용대로 이 후보에서 제외할 수 있다.
단순 3배 복제나 전체 asset 회전 보정 대신 누락된 실제 source 입력·소비 또는 원본 조합만
복구하고 현재 저장본의 사용자 TRS·재질·타이밍은 guarded 필드 patch로 보존한다.

`build_kouku_ritual_hand_trail.py`에 오른손 지팡이 끝 비교 후보만 추가한다. 원본 startcontrol,
b_wp_1과 설치 WP_MN_RPCT_05의 정점·skin bind·preScale·preRotation 및 native emitter의
StartLocation을 함께 측정하여 지팡이 tip을 산출한다. 원본 notify 4219911/001의 시각과 두
Ribbon·sprite 재질을 재사용하고 기존 왼손 asset은 한 바이트도 바꾸지 않는다. 사용자 지정
tip 위치와 원본 emitter 위치가 다르면 그 차이를 명시하며 이중 offset을 적용하지 않는다.

새 C++나 두 번째 effect runtime은 추가하지 않는다. 공용 Playback/Renderer 수정이 필요하면
root와 소유 구간을 먼저 조율한다. 설치 WModel 수치, 실제 Codec/Drawable/roundtrip/Playback의
birth·방향·bounds를 검사하고 입력 hash, 후보, Catalog/Tree semantic entries와 Data None 경로를
out에 저장한다. 라이브 Data 등록·Client 조작·GPU 화면 판정은 이번 단계에 포함하지 않는다.
### G20 추가 — 검정 레이어의 원형 경계 재조사

사용자가 fx_d_noise_002와 검정 sprite를 복제·배치한 최신 문서를 out에 byte 보관하고,
원본 exp03 dark-aura/flow-mask/world-offset emitter의 바이너리 export·class default와
투영된 size/location/facing/alpha/texture를 대조한다. 원본 sector mesh3개 조사와 별개로
검정 sprite의 비율과 원형 배치 소비를 확인한다. 현재 live Data와 사용자 복제 항목은 덮지
않으며, 누락된 원본 입력이 확인될 때만 기존 pizza builder의 guarded 후보를 만든다.
공유 C++/HLSL 소비 결함은 root에 먼저 보고하고, 새 shader나 추정 원형 mask는 추가하지
않는다. 검증은 원본 필드·실제 Playback/Geometry 수치 및 필요한 격리 CPU 검사로 제한한다.

## G22. 사용자 요청 카드·주사위 크기와 발사 수 조정

최신 저장본을 기준으로 회전 카드 mesh·본체 symbol·잔상 symbol의 sourceScale.size를 각각
1.5배로 조정한다. Effect root scale로 이동 거리·속도·owner 위치까지 확대하지 않는다.
common 통합 연출, Server가 사용하는 네 문양 및 standalone emitter의 연결 대상을 함께
점검한다. Pattern48 Logic74의 wave당 발사는3→6이며 standalone 주발사 count도2배다.
접촉 폭발은 증가한 발사 event에 따라 생성되므로 폭발당 입자 수를 다시2배 하지 않는다.

주사위 네 단계 mesh와 상단 다이아 세 sprite는 현재 sourceScale.size만1.5배로 한다.
원작 복원과 사용자 확대 요청을 구분하며 기존 손 basis·연속 궤적·위치·시각·재질·나머지
이펙트는 보존한다. 생성기에도 동일한 명시적 저작 배율을 전달한다. C++ 변경은 필요하지 않다.

사용자는 저장했지만 Client가 실행 중이라고 답했다. 현재 저장본을 byte 백업하고 크기·수만
바꾼 후보를 먼저 검증한다. 데이터 반영 단계는 기존 저장본 해시·최신 revision을 재확인하고
기존 편집 충돌 보호를 유지한다. 필요한 Codec/Drawable/Stage/실제 size·count 검사 뒤 기존
KoukuSaydon publisher를 사용한다. 데이터 반영에 Client 종료를 요구하지 않으며 제품 빌드와
도구 Reload·최종 화면 판정은 사용자가 한다.

## G23. 비둘기 일렬 비행과 반원 선회

사용자는 현재 네 마리가 이동 중심 주위를 공전하는 결과를 지적하고, 일렬 이동 뒤 반원으로
선회하는 묶음을 요청했다. 기존 중심 직선8m/s + 반경1m의0/90/180/270도 Orbit을 제거하고
같은 경로 P(s)를 따라가는 네 위치 P(8t - i×0.8m)로 표현한다. 음수 진행거리도 직선으로
연장하여 출발 때 네 마리가 겹치지 않는다. 선두는0.7초 직선,2초 반원,0.3초 복귀 직선이며
반원 반경은16/π m다. 후속 세 마리도3초까지 반원을 마친다. 각 새는 경로 접선 방향을 본다.

원본 Action/Projectile에는 여러 발사 묶음과 서로 다른 수명이 있어 이 간격·반경을 원본
확정값으로 표기하지 않는다. 현재 Effect의3초 수명과8m/s를 유지하는 사용자 요청 저작 궤적이다.
기존 local-space mesh·sourceTransformTrack·particle death event를 재사용하고 별도 runtime은
추가하지 않는다. 각 새의 실제 수명 종료 위치에서 기존 폭발과 후광이 생성돼야 한다.

후보·builder를 먼저 갱신하고 위치·접선·속도·간격·180도 선회와 종료 event 위치를 수치로
검사한다. 사용자에게 두 차례 노출된 count_probe abort 이후 진단 EXE는 Windows/CRT 오류
창 차단·실패 즉시 종료·예외 로그를 확인한 것만 실행하며 실패를 무조건 재시도하지 않는다.
후보 검증을 끝낸 뒤 최신 저장본 기준 반영 승인을 한 번 확인하며 이미 받은 승인은 재질문하지
않는다. 최신 변경 필드·hash를 다시 확인하고 병합한다. Client 종료는 반영 조건이 아니다.

## G24. 상단 무지개 Solo의 source provider 보존

선택 미리보기의 공급원 수집이 LocationEmitterDirect 두 class를 누락한다. 실제 런타임과
같은 네 LocationEmitter class 및 활성 module의 명시적 provider ID를 재귀 수집하며
TransformInheritance master도 보존한다. 선택 전후 provider 검증을 유지하고 문서 순서와
시간을 바꾸지 않는다. 기존 Set_SubmissionElementSet으로 선택한 항목만 화면에 제출하여
공급원은 계속 simulation하지만 Solo에 추가로 그리지 않는다. Sequencer와 일반 Solo/Family/Group
모두 기존 Build_ElementsPreviewDocument를 소비한다. 신규 런타임이나 파일은 추가하지 않는다.

실제 저장된 rainbow.drop의 direct 참조와 중첩 공급원, 잘못된 ID의 기존 결과 보존을 CPU로
검사하고 변경 TU를 격리 컴파일한다. 화면 판정은 사용자에게 남긴다. 공/무지개 위치 수치는
현재 저장본에서 이미 일치하므로 별도 속도 보정은 근거 없이 추가하지 않는다.

## G25. 알비온 출현 시 대상 선택의 Server·미리보기 계약 일치

최신 P79 저장본은 JUMP 뒤 APPEAR_PLAYER·SLAM을 유지하고 사전 SELECT_PLAYER만 삭제했다.
사라지기 V1 Effect는 Logic을 추가하지 않는다. Server는 사전 선택이 없거나 선택한 플레이어가
유효하지 않으면 APPEAR 시점에 살아 있는 대상을 선택하지만 미리보기만 SELECT를 강제한다.
`KoukuSaydonPreviewRootMotion.cpp`의 불필요한 SELECT 선행 조건을 제거하고 JUMP 선행,
실제 native pose owner와 남은 하강 검증은 보존한다. `KoukuSaydonPresentationPlayer.cpp`는
사전 선택이 없는 첫 APPEAR의 occurrence ID에 대상을 기록하고 후속 APPEAR에서 재사용한다.
기존 SELECT 정책의 navigation ground 고정과 APPEAR 정책의 출현 시 위치 선택은 유지한다.

Data의 삭제된 Logic이나 사용자 배치를 복원하지 않는다. 현재 P79의 실제 설치 모델과 clip으로
단계·착지 검사를 수행하고 SELECT 없음, 기존 APPEAR 선택, SELECT ground 고정과 대상 이탈을
기존 소비 코드의 격리 검사로 확인한다. 두 TU의 최소 컴파일과 실패 시 기존 단계 보존을 검증한다.
진단 EXE는 Windows·CRT 창 차단과 예외 처리를 검토받은 뒤 한 번만 실행한다.

## G26. 카드 네 문양의 정렬·반복 수명과 회전 투척 잔상

현재 저장본에서 다이아의 뒷면 mesh만 앞면보다 1.690000118m 위에 있다. 네 문양의
앞면 기준 위치와 원본 mesh TypeData 회전, sprite axis와 owner 회전을 대조한다. 공통
생성기에 같은 정렬 절차를 두되 각 문양의 사용자 배치 높이·크기와 나머지 이펙트는 보존한다.

원본 Required의 EmitterLoops와 투영된 sourceRecipe.emitterLoopCount를 대조해 누락된
반복을 복구한다. 카드 1초와 symbol_109 2초의 원본 주기는 유지하고 기존
loopEffectToDuration → Set_SourceLoopEndSeconds 소비자가 박스 전체 구간을 재생하게 한다.
새 runtime이나 무한 lifetime 상수는 추가하지 않는다.

symbol_050의 회색 내부는 실제 texture RGB와 원본 masked PS의 결과다. 현재 자료로
원작의 최종 흰색 노출까지 확정하지 않으며 네 match 앞면의 emissiveIntensity만 기존
저작 경로로 보정한다. 원본 shader·DDS·alpha 및 같은 재질의 폭발은 변경하지 않는다.
회전 투척 통합 문서의 world-space 카드형 emitter0만 숨겨 사용자 요청의 잔상을 없앤다.
원본에도 이 emitter가 존재하므로 제거를 원본 데이터 복원으로 기록하지 않는다.

최신 bytes를 보관하고 후보를 만든 뒤 actual Codec/Drawable/roundtrip/Playback/Geometry로
중심·방향과 1초/2초 경계의 반복을 검사한다. 저장본 freshness를 확인하고 필요한 필드만
반영한다. 데이터와 Python 생성기 수정이므로 제품 C++/셰이더 재빌드는 필요하지 않다.
실행 중 Client의 미저장 여부를 확인하며 화면 조작과 최종 판정은 사용자가 수행한다.

## G27. 쓰리투원투하의 독립 공 발사

사용자가 추가한 발사 화면을 진단 입력으로 사용한다. 기존 rainbow.drop의 설치 environ
공 메시·native 재질·사용자 크기를 유지하고 낙하 곡선을 역방향으로 재생하는 독립
`effect.kouku.gate1.circus.ball.launch`를 만든다. 표시 이름은
`세이튼 / 쓰리투원투하 | 공 발사`이며 같은 패턴 분류에 등록한다. 무지개와 별은 공 아래를
따르며 상단 도달 시 기존 낙하의 도넛 폭발은 재생하지 않는다.

상승 궤적은 사용자 요청의 프로젝트 변형으로 구분한다. 기존 drop·WorldSequence·패턴
타임라인과 사용자의 위치·크기 편집은 보존한다. 기존 circus builder의 별도 후보 모드를
사용하고 Codec/Playback의 위치·속도·크기·부착·종료를 검사한다. Authored·Catalog·Tree·
프로젝트 None과 Composition 리소스에 신규 stable ID를 연결하되 임의 발사 시각을 넣지 않는다.
저장 기준본 충돌 확인 후 설치하며 최종 화면 판정은 사용자가 수행한다.

## G29. 피자 원본 재질·방출 조합의 독립 복원본

사용자 첨부 이미지의 원 밖 검정 덩어리와 현재 저장본을 구분하여 검사한다. 현재
`effect.kouku.pizza.explosion.group`은 SHA4074dd36…의 사용자19요소 문서다. 원본 두 개였던
native3171 dark-aura가 다섯 개로 복제·이동됐고, native3280 flow 두 개는 emitter31 복제이며
원본 emitter1이 빠져 있다. 기존 G20의 Cylinder 부호 제한은 이 편집 조합을 원본으로 돌리지
않았으므로 사용자19요소는 byte 보존하고 별도의 원본 exp03 복원본을 준비한다.

새 ID는 `effect.kouku.pizza.explosion.group.source-restored`, 표시 이름은
`쿠크_피자 | 피자 부채꼴 - 원본 검정 무지개 경계 폭발 2`다. 기존
`build_kouku_dove_pizza_candidates.py`의 `pizza_source_restored()`와
`--pizza-source-restored`가 원본16요소를 복사하고 stable ID만 새로 부여한다. 기존
`repair_geometry_defaults()`로 바이너리/CDO에서 이미 확인한 빈 radius/velocity distribution을
채우며 mesh·sprite·native MIC·입자색·dynamic·alpha·원본 TRS는 보존한다. 기존 pizza/dove
모드와 live 문서에는 쓰지 않는다. 결과와 source hash는 `out/PizzaMaskRestoration20260917`에 둔다.

원형 경계는 geometry와 재질별로 검증한다. native3171의 원본 PS는 UV의 analytic falloff와
ParticleAlpha를 사용하며 두 noise는 RGB 경로다. native3280은 flow와 별도
`fx_m_ring_001_cl` opacity texture 및 radial UV fade를 사용한다. 원본 world-space 원형 clip이
없는 layer에 임의 공통 crop을 추가하지 않는다. 원본 PS/설치 PS의 입력별 alpha 비교와 actual
Playback의 원본 emitter 수·축·pivot·폭·height를 별도 검사해 번역 오류와 저작 배치 차이를
분리한다. 추가 shader/carrier 결함이 확인되면 같은 G에 정확한 소비자와 수정 범위를 먼저 적는다.

Codec Load/Drawable/roundtrip/Stage 및 수명 sweep은 현재 ABI로 격리한 기존 probe를 재사용한다.
EXE 실행은 root가 검토한 noninteractive guard를 가진 probe만 수행한다. Catalog·Tree·project
None·Composition의 새 resource 등록은 root가 최신 저장본 기준으로 병합한다. 새 C++ 파일은
계획하지 않으며 Client/UI 실행·최종 시각 일치 판정·자동 패턴 타임라인 교체는 포함하지 않는다.


### G29 추가: 원본16 재조합의 한계와 요청된 원형 coverage

설치 sphere002/003 WModel의 geometryPreScale .01과 실제 .2초 Playback 행렬로 측정한
무지개 부채꼴 외곽은 반경6.6000m다. native3171 두 원본 sprite를 그대로 합성해도 .2초
alpha>=.1 영역의 최대반경은8.501m이고 .4초에는10.033m다. 사용자 다섯 복제본은 .2초
13.359m다. 원본16만 복사하는 앞 절의 후보는 원형 경계를 보장하지 않으므로 최종 설치하지 않는다.
원본 PS에 존재하지 않는 원형 제한은 사용자 요청의 `PROJECT_AUTHORED` coverage로 구분한다.

`EFFECT_SPRITE_DETAIL_DESC::OwnerRadialMask`와 JSON `detail.sprite.ownerRadialMask`를
추가한다. enabled=false, centerXZ=[0,0], radius=1, feather=.05가 생략 기본값이며 기본값을
가진 기존 문서는 shader 결과가 완전히 같다. 이번 지원 carrier는 원본 native3171 source sprite
particle이다. 검증되지 않은 mesh·decal·trail·다른 native 재질·compiled adapter에는 활성화하지 않는다.
새 복원본의 두 dark-aura에만 enabled=true, radius=6.6, feather=.05를 저장한다.

좌표 정본은 Effect origin XZ다. mask-to-world는 ParticleSystem uniformScale/yaw 다음
Frame.RootWorld이며 이를 역변환한 위치에 centerXZ·radius·feather를 적용한다. owner/cue
외부 scale은 RootWorld를 통해 한 번, 전역 particleSystem scale은 한 번 적용한다. 개별
StartSize와 element scale은 검정 sprite 자체를 바꾸며 공통 원형 경계를 늘리지 않는다.
매 batch 새 enabled와 역행렬을 bind하며 singular/non-finite transform은 draw 실패로 분리한다.

기존 Detail codec의 읽기·쓰기·range 검증과 MaterialDetail UI를 연결한다. 반경은 유한한
양수, feather는 [0,radius], centerXZ는 finite여야 한다. disabled는 기본값으로 초기화한다.
기존 ParticleFamily group3136만 opt-in shader adapter를 추가하고 원본3171 PS·DDS·MIC·색상은
보존한다. shader는 원형 밖 alpha와 distortion을0으로 만들며 feather는 안쪽에서 smoothstep한다.
RT0 RGB는 straight alpha로 유지한다. RT2도 같은 coverage alpha를 사용하여 중복 감쇠하지 않는다.
새 family/runtime/C++ 파일은 만들지 않는다. source leaf와 사용자19요소는 byte 보존한다.

검증은 Codec load/save/load, disabled omission, radius0/음수/NaN 및 feather범위 실패,
원본 shader 수식과 원형coverage 결합의 바깥 alpha0, identity/이동/XYZ회전/uniform·비등방
root scale과 particleSystem scale 역변환의 반경 일치를 수행한다. group3136 shader compile과
변경 C++ TU 컴파일은 root가 실행한다. 새 struct로 모든 소비 TU의 ABI가 바뀌므로 최소 probe도
해당 header를 소비하는 객체를 재컴파일한다. Client와 화면 판정은 사용자 경계를 유지한다.

## G30. 저장된 카운터 성공·파란 모델 표시와 비둘기·카드 잔상

기준 Composition1306의 P80 세이튼_돌진_카운터와 P81 세이튼_공굴리기_카운터는
logic82 DURATION을 각각0~1332ms, 0~2837ms에 두고 Success를 logic51 결과_카운터에
연결했다. 두 정의에는 judgementKind/outcomeKind가 없어 제품 publisher가 패턴을 제외한다.
logic82에 COUNTER_WINDOW와 endsPatternOnSuccess=true, logic51에 FOLLOWUP_PATTERN과
P4 세이튼_무력화성공을 채운다. P4는 같은 GATE1/MN_RPCT_05의 start1167/loop1667/end1333ms
그로기다. 사용자 occurrence 시각·Duration·Success 연결과 WORLD 배치는 보존한다.

Server는 기존 COUNTER_WINDOW 판정과 Apply_KoukuLogicOutput 후속 패턴 경로를 사용한다.
현재 stage root motion의 up은 bossY에 누적되고 Finish_Pattern은 이를 내리지 않으므로,
공중에서 성공하면 P4가 현재Y를 새 origin으로 잡을 수 있다. 종료되는 카운터 성공에만
성공 signal을 소비하기 전에 LogicRuntime에서 현재XZ의 navigation 지면과 body 충돌을
검증한다. 원래 placement의 지면 대비 offset을 보존하여 높이·성공을 commit한 뒤
Server-only output 표식으로 Room의 기존 패턴을 완료한다. 검증 실패는 열린 ledger와
signal을 deadline까지 보존하고, 끝까지 실패하면 기존 timeout으로 닫는다. 공중 그로기를
강행하지 않으며, 종료된 패턴의 motion·owned WORLD
공 cue 정리와 다음 tick의 P4 시작을 함께 검사한다. Shared packet과 Client 위치 권위는 바꾸지 않는다.

Client는 기존 BossCombat.iFlags의 COUNTERABLE을 사용하여 CNpc에 파란 표시 상태를
전달한다. Tool preview는 저장된 enabled COUNTER_WINDOW의 [start,start+duration)을 사용한다.
CSkeletalAfterimage의 실제 CModel palette 캡처와 pass14를 재사용하고, 정지 모델에도
240ms마다 한 포즈를 캡처하여160ms 감쇠 뒤80ms 비표시 구간을 둔다. 색은 프로젝트 저작값
(0.12,0.7,2.4,0.7)이다. body·skinned weapon·실제 표시 중인 head hat을 같은 경로로 처리하며
원래 재질을 바꾸지 않는다. 카운터 중 기존 흰 이동 잔상과 중복하지 않고 닫힘·사망·숨김·
owner 제거·정지 및 seek에서 파란 history를 정리한다. Npc, SkeletalAfterimage,
NpcPresentationAssetService, KoukuSaydonPresentationPlayer의 기존 H/CPP를 수정하고
Engine/HLSL·새 C++·프로젝트 항목은 추가하지 않는다.

비둘기는 현재 native2893 mesh4개의 detail.color.emissiveIntensity만4로 보정한다.
실제 Mesh renderer의 native 출력 후 RGB 곱셈을 확인했으며 원본 shader·DDS·alpha와 종료
폭발19개는 보존한다. sourceTransformTrack의 선회 전후 직선 성분만2/3으로 줄이고
.7/2/.3초 phase·반원 반경16/πm·0.1초 추적 지연·yaw·3초 death를 유지한다. 직선 속도는
8→16/3m/s, 직선 간격은.8→.533333m가 되고 원호 속도8m/s와 원호 간격.8m는 유지한다.
별도 사용자 편집 track은 알려진 이전/새 track과 대조하여 실제 충돌로 분리한다.

회전 카드 common 문서의 sk13_1/emitter0 native2999 24요소는 G26에서 이전 사용자 요청에
따라 숨겼다. 이번 명시적인 잔상 복구 요청에 따라 visible만true로 복원한다. 원본 보라색·
alpha·.15초 수명·world-space SpawnPerUnit, 카드 개수2배와 size1.5를 보존한다.
standalone 문서에서 이미 삭제된 요소를 추측하여 새로 만들지 않는다. 기존 두 Python
builder를 재실행해도 같은 정책을 유지하도록 갱신한다.

후보와 필드별 diff는 out/CounterDoveCard20260917에 보관한다. 최신 디스크의 stable ID별
대상 필드만 병합하고 무관한 사용자 변경은 보존한다. 교체 직전 hash/revision 재확인·백업·
원자 교체·실패 시 자기 변경 rollback을 적용한다. 실제 Codec/Playback 및 기존 Server
contract 검증을 필요한 범위로 재사용하고 개별 변경 TU를 격리 컴파일한다. 제품 owner publish
결과와 실행 파일·실행 중 메모리 반영을 구분한다. Client/UI 실행 및 최종 화면 판정은 사용자가 한다.

## G31. 주사위 카드의 Duration 추적·접촉과 미리보기

사용자가 추가 저장한 revision1308의 P78에는 기존 logic73 PURSUIT_PROJECTILES
6114~19831ms와 새 presentation9~12의 네 문양 Effect가 함께 있다. 일반 Effect는
anchor=BOSS/followBoss=true/logicOccurrenceId 빈 값으로 저장되어 추적 객체가 아니다.
Server의 GameRoom_BossSimulation/CombatObjectRuntime과 Client의 contact visual 경로는
이미 구현되어 있지만, LogicPreview는 pursuit 리소스를 준비만 하고 재생 분기가 없다.
따라서 기존 Server 구현과 새 Effect 박스의 미연결, Preview 구현 누락을 구분하여 고친다.

기존 typed PURSUIT_PROJECTILES Duration을 사용하여 다이아7355ms, 스페이드9081ms,
클로버11417ms, 하트13056ms에서 각 한 장씩 발사한다. 네 박스의4500ms를 유지하고
각 문양 visual1개/count1/한 번 발사/3mps/homing 및 동일 contact effect를 사용한다.
중복되는 기존 동시4장 occurrence와 일반 카드 Effect4개를 이 Duration으로 대체한다.
무관한 무대·주사위 표현, 기존 정의, 사용자 애니메이션과 시각은 보존한다. 새 stable ID는
최신 next ordinal에서 할당하고 기존 timing builder도 이 구성을 덮어쓰지 않게 갱신한다.
콜라이더를 일반 Effect 내부에 넣거나 Client의 접촉 결과를 Server 정답으로 보내지 않는다.

네 match asset은 현재 저장 uniformScale의2.5배로 확대하여 카드와 앞면 symbol의 정렬을
함께 보존한다. contact radius도.5→1.25m로 맞추고 실제 표시 중심과 설치 WModel 크기를
대조한다. 사용자가 명확히 한 잔상 숨김 범위는 이 주사위4종이다. 회전 카드던지기는 G30의
보라색24요소 복구와 크기1.5·개수2배를 유지한다.

KoukuSaydonPresentationPlayer_LogicPreview.cpp에서 기존 EffectPlayback 세션을 사용해
실제 preview player의 읽기 전용 위치로 추적·접촉·폭발을 시각화한다. 일반 Play Preview에서
카드가 나타나는 시점과 active Duration을 소비하며, no eligible player는 임의 가상 표적을
만들지 않는다. 일시정지·seek·rewind·stop에서 재생 clock과 소유 시각 객체를 정리한다.
Tool preview는 damage·counter·Server 상태를 바꾸지 않는다. 실제 Server Play는 기존
권위 projectile/contact 판정과 exact-once contact visual을 계속 소비한다.

Counter 작업과 공유하는 player header는 한 소유자가 통합한다. 변경 파일별 최신 diff를
보존하고 후보4종 Codec/Playback·projection·기존 Server pursuit contact/수명 계약·Client
변경 TU 컴파일을 수행한다. 결과를 최신 Composition과 stable ID별 병합한 뒤 owner publish하며
새 제품 실행 파일은 사용자가 빌드한다. 최종 추적·접촉·크기 화면은 사용자 검증으로 남긴다.

### G31 추가: 기존 무한 수명과 새 카드 박스 수명 구분

기존 lifetime0 pursuit는 자연 pattern 완료 이후에도 접촉까지 생존하도록 구현·검증되어 있다.
이를 전역 Duration 종료 취소로 바꾸면 기존 카드 꼬리가 잘리는 회귀가 생긴다. 그 계약은
보존하고 이번 P78의 네 새 정의에만 lifetimeMs=4500을 명시하여 저장된 박스 수명과 맞춘다.
발사 간격0/count1이므로 한 번만 생성하고 접촉하면 기존 exact-once event로 먼저 종료한다.
기존 spinning의 cadence·finite travel·자연 꼬리는 변경하지 않는다. 새로운 projectile
runtime이나 Shared packet은 만들지 않는다.
