# 발탄 피자·불어날리기·하늘 이펙트 구현 계획

## G00. 현재 데이터와 보존 경계

사용자의 현재 STEP_11 offset `[-2.9200000762939453,0.6600000262260437,0]`과
`Atk_08_04`의 emitter13 삭제를 보존한다. STEP_11은 stage005 Full Restore가 아니라
`effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_08_04`의 남은7개 Element를 사용한다.
같은 emission 중심을 가지는 원본 source translation과 occurrence translation을 반대 방향으로
재기준화해 초기 위치를 유지하면서 회전 기준을 검격 중앙으로 이동한다. 원본 velocity와
TypeData의 degree, source recipe, 재질과 사용자 삭제를 보존한다.

## G01. occurrence와 원본 sky 복원

`VALTAN_CATCH_BREATH/STEP_04`의 기존 V1 cue yaw에 위에서 본 시계방향90도를 더한다.
다른 catch 단계나 공유 원본 Effect는 변경하지 않는다.

원본 package에서 `bfx_high_01.valhatron.par_d_spacehole_03`과
`bfx_high_00.chaosgate.par_d_hugechaosgate_01`을 기존 common extractor로 읽는다.
실측 결과는2개의 active mesh와3개의 empty emitter,12개의 chaosgate emitter다.
과거 unbound V2 blackhole proxy를 원본으로 표시하지 않는다. 원본 재질10개 permutation을
기존 native shader generator와 source mesh cooker로 복원해 기존 V1 carrier에 연결한다.
원본 map 배치의 위치·회전·scale을 사용하고, 피자 상공 이동 구간의 occurrence가 표시 시간을
소유한다. 원본 자료에 없는 clip 활성화 시각은 현재 패턴 시계에 연결한 저작값으로 구분한다.

## G02. 파일과 검증

`Data/Valtan/Valtan.presentation.json`은 정확한 cue만 수정한다. source Effect와
EffectCatalog/ResourceTree를 추가할 때 기존 stable ID와 모든 다른 행을 유지한다.
생성된 native 표·shader는 기존 설치기를 사용하며 새 renderer를 만들지 않는다.
통합 담당이 split publish와 Composition/GamePlay publish를 실행한다.

JSON parse, 기존 source validator, 실제 native Codec/Playback 검사와 source 재질/geometry
누락 검사를 수행한다. 원래 표시 위치와 회전 중심 보존은 수치로 검증한다. 공용 Engine
shader compile과 Client build는 통합 담당에게 정확한 변경 목록을 전달한다.
Client/UI 재생과 화면 캡처는 하지 않으며 최종 색·타이밍 판정은 사용자에게 남긴다.


## G03. 후속 요청: 발악 완료에서 망령 부활로 직접 전환

현재 STRUGGLING 마지막3개 clip은 mesh_att_battle_5_01_end4433ms,
mesh_att_battle_19_05 2333ms, mesh_att_battle_19_06 333ms다. 이후 TIMEOUT은 별도
VALTAN_GHOST_DEATH_AUDITION으로 연결된다. 이 패턴은 현재 valtan.cinematic.finale23000ms를
재생한 뒤 mesh_respawn_1 3000ms의 VALTAN_GHOST_RESPAWN_AUDITION으로 연결된다.
추가23초는 STRUGGLING 내부 clip이 아닌 별도 패턴이라는 데이터 근거가 확인됐다.
사용자 후속 지시에 따라 STRUGGLING/STEP_12의 TIMEOUT nextPatternId만 RESPawn으로 바꾼다.
DEATH 라이브러리·독립 재생과 explicit PlayAll 저장 목록은 보존한다. ValtanBrain은 nextPatternId를
pinned PendingPatternFollowup으로 소비하고 다음 tick에 재생하며 death 하드코딩은 없다.
연속 레이드 regression의 revival 기대값에서 DEATH만 제거하고 독립 DEATH→RESPAWN 검사는 유지한다.
게시와 제품 빌드는 통합 담당이 수행하며 이 하위 작업은 Client/UI를 실행하지 않는다.

## G04. 공유 검격 모양 회전과 전방 이동 분리

후속 요청은 모양을 반시계90도로 회전하면서 발사는 발탄 정면으로 유지하는 것이다.
현재 발탄 root 회전과7개 particle의 이동은 정상이며, fixed-axis sprite36/43의 최종 면만
emitter 회전을 따르지 않는다. 기존 실제 Codec/Playback/Geometry의5개 yaw와8개 시각에서
이 차이를 재현했다. 부모 root를 다시 보정하거나 전체 Local Space를 변경하지 않는다.

사용자가 원본 Atk_08_04를 공유하는 피자와 지형 파괴3시/9시 전체 반영을 명시했다.
기존 DIRECT_AUTHORED_DOCUMENT 자체를 수정하며 별도 파생 문서를 만들지 않는다.
기존7개 stable element ID, sourceRecipe·Resources·타이밍·크기·색·중심 재기준화를 유지한다.
Particle System의 yawOffsetDegrees=-90, directionYawDegrees=+90을 사용해 모양·배치를
고정 발사 중심에서 회전하고 초기 속도 회전은 상쇄한다. sprite36/43에만
followEmitterAxisRotation=true를 추가한다. 원본 발사 속도와 EF 수명별 속도 배율은 보존한다.

세 occurrence는 동일 asset ID를 계속 사용하며 각각의 TRS·anchor·follow·scale·시각을
보존한다. EffectCatalog/ResourceTree·프로젝트 등록·cue 변경은 필요 없다.
신규 C++·shader·Resources 파일은 없다.

사용자의 반영 승인을 기준으로 최신 저장본·stable ID를 다시 확인하고 필드만 병합한다.
교체 직전 hash 검사·백업·원자 교체와 자기 변경 rollback을 유지한다. JSON/XML parse,
native Codec/Playback/최종 geometry와 source 보존 검증 뒤 변경 domain의 정본 publisher로
실제 소비 generation을 게시한다. 데이터만 바뀌므로 제품 재빌드는
필요 없다. 실행 중 도구의 저장되지 않은 draft·Reload·Client 화면은 자동 조작하지 않는다.
