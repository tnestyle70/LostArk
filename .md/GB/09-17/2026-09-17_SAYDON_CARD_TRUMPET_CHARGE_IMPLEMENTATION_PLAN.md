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

사용자가 2026-09-17 대기 항목 전체 반영을 지시했다. 현재 미저장 편집 보존 확인은 진행 중이며,
저장·Client 종료 확인 전에는 Data를 쓰지 않는다. 등록은 out의 전체 snapshot을 교체하는 방식이
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