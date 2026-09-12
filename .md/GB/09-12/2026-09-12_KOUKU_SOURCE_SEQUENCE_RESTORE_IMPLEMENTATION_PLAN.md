# 쿠크 원본 시퀀스 저작 연결 구현 계획

## G06. 사용자 화면 기준 팝업북·피날레 통합 수정

2026-09-12 후속 사용자 요청은 기존 맵 담당자의 정상 팝업북·피날레 애니메이션과 새 이펙트·흡입을 결합하는 것이다. 아래 G00~G05는 최초 원본 Matinee 후보의 이력이며, G1의 애니메이션 선택은 이 G06이 우선한다. 기준 HEAD는 `a72058637567f21e9b4235ef214133fc70c84963`, Sequence revision6, WorldSequence revision675다. 동시에 진행 중인 Engine/Map 렌더링 성능 변경은 보존한다.

기존 Pattern1은 136개 map placement의 `original_8T6_00~04`, `original_book`, `original_kouku`와 기존 카메라를 소비한다. Pattern2는 `circusfinale`의 23개 map placement에 직접 저작한 회전·표시 키를 소비한다. 현재 Pattern4는 이 행들을 사용하지 않고 새 Book/Saydon/배경266개를 동시에 생성한다. 정상 두 연출의 기존 template·track·카메라를 재사용해 G1 통합을 구성하고, 포탈·흡입·암전·축포의 위치와 시간을 실제 문 열림·책 펼침 구간에 맞춘다. 독립 Pattern1/2도 신규 이펙트·환경을 함께 확인할 수 있는 기존 편집 경로를 유지한다.

책과 펼쳐지는 아레나는 실제 mesh가 사용하는 slot별로 현재 1관문 재질과 비교한다. 기존 승인 map material과 texture/IBL/BRDF 입력을 기존 CModel/CMaterial로 연결하며 이동 부품에 고정 아레나의 baked RNM을 그대로 복사하지 않는다. 조명·Scene Profile은 현재 1관문 값을 소비하고 표시 전환 전후의 적용 차이를 닫는다. 연출 배우와 고정 전투 보스의 표시 소유권은 Level의 기존 lifecycle에서 분리하고 Stop/실패/seek 때 차용 전 상태를 복구한다.

같은 환경을 게시하는 데 필요한 `Publish-RenderingProfiles.ps1`의 비교 정밀도를 C++ `Read_Float`와 맞춘다. JSON의 finite number와 float32 변환 결과를 각각 검사하고 변환한 값으로 float32 하한·상한을 비교한다. fog의 runtime float 필드도 같은 함수를 사용한다. 9자리 저장값을 수동으로 되돌리지 않으며 NaN/Infinity·다른 float32 범위 값은 계속 거부한다. 기존 publisher 테스트에 실제 0.1과 0.0001 하한 저장 왕복·범위 밖 입력·실패 시 runtime 보존을 추가한다.

새 C++ 파일과 project/filter 등록을 추가하지 않는다. 각 소유자가 out 후보를 검증한 뒤 루트가 최신 Data 기준으로 필요한 변경만 병합하고 Area/Rendering의 공식 publisher를 실행한다. 변경 C++의 최소 컴파일과 실제 모델/타임라인 CPU 검사, JSON parse·diff check를 수행한다. Client 실행·UI 조작·캡처·최종 화면 승인은 사용자에게 남긴다.

## G00. 원본 시간축과 저장 경계

기존 `연출_팝업북`, `연출_1관문 피날레`, 사용자가 편집한 `2관문_진입컷씬`은 보존한다.
SCENE03A Matinee0의 41.487556초를 신규 `1관문_통합_시퀀스`의 시간축으로 사용한다.
포탈은 2.344762초, 흡입은 4.354424초에 시작하며 7.459291초의 암전 뒤
10.218567초부터 책이 펼쳐진다. 기존 두 패턴의 재생 시간을 단순 합산하지 않는다.

저장 정본은 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`,
해당 Area의 `camerashots.json`, `worldsequences.json`과 기존 Effect 문서다.
원본은 `out/KoukuFireworks20260911`의 설치 UPK 추출과
`out/KoukuAllEffects20260912/organization.json`의 Matinee occurrence를 대조한다.
기존 JSON을 다시 읽고 변경 전 바이트를 비교한 후 신규 stable ID만 병합한다.

## G01. 원본 카메라와 화면 전환

`build_gate2_intro_composition.py`의 기존 UE3 좌표 변환·Hermite 표본화·카메라 축소를
Matinee와 InterpData별로 재사용한다. cut 경계를 보존하고 shot당 64key 이내로 나눈다.
UE3 수평 FOV는 16:9 수직 FOV로 변환한다. 180도는 투영 특이점이므로 기존 runtime의
0도와 180도는 runtime의 (1,179)도 범위 안인 1.1~178.9도로 유한 투영하고 원본과의 차이를 결과에 기록한다.
암전은 기존 V2 ScreenPost의 black overlay와 intensity key를 사용한다.
G1 통합과 G3 진입은 현재 아레나 기본 `scene.kakulsaydon.g1.base.v1`을 명시적으로 재사용한다.
이는 원본 환경 전체 복원이 아니며, G2에는 같은 프로필을 강제하지 않는다.

## G02. 월드 모델과 이펙트

책과 Saydon actor는 기존 CModel/WANM 경로로 원본 A/B slot blend를 표본화한다.
새로 생성하는 Resource 위치는 부모 작업과 조율한 뒤 설치하며 기존 파일을 덮어쓰지 않는다.
Saydon의 materialSourceModelAssetId와 modelPreScale은 BossCatalog의 동일 actor를 소비한다.
원본 cinematic drawScale과 전투 크기가 다르면 사용자가 요청한 전투 크기를 우선한다.

원본 Sequence Effect는 이미 설치된 최종 native leaf library를 소비하고 원본 Hermite
movement, 활성 구간과 origin을 보존한다. 지원되지 않은 skeletal attachment는 누락 이유를
기록하며 다른 위치로 임의 배치하지 않는다. runtime sourceTransformTrack 경로를 재사용한다.
Effect tail을 원본 Matinee 종료 시간으로 오인해 전투 진입을 늦추지 않는다.

## G03. 관문별 Sequence와 종료 계약

| stable Pattern ID | 표시명 | 원본 | 종료 |
|---|---|---|---|
| KAKULSAYDON_G1_PATTERN_4 | 1관문_통합_시퀀스 | SCENE03A Matinee0 | 41,488ms |
| KAKULSAYDON_G1_PATTERN_3 | 2관문_진입컷씬 | SCENE04A Matinee2 | 27,000ms |
| KAKULSAYDON_G1_PATTERN_5 | 2관문_클리어 | SCENE02A Matinee10 | 35,368ms |
| KAKULSAYDON_G1_PATTERN_6 | 2관문_카드미로 | SCENE04A Matinee1 | 11,950ms |
| KAKULSAYDON_G1_PATTERN_7 | 3관문_진입 | SCENE02A Matinee10의 SL05 도착 | 원본 도착 cut부터 종료까지 |

부모 작업과 합의한 optional boolean `enterCombatOnFinish`는 entry 4·3·7에만 true다.
Complete Play가 해당 gate의 entry 하나를 선택하며, 전투 boss 생성·player 이동·카메라 반환은
부모 작업의 기존 Server command 경계가 소유한다. clear와 maze는 명시적 연출 preview다.
카드미로 gameplay는 기존 Pattern28의 Server `CARD_MAZE_HIDE_NEXT/ENTER`와 구분한다.

## G04. 검증과 사용자 화면 확인

생성된 JSON parse, stable ID 참조, 원본 cut 시각과 카메라 유한성·key 개수,
Resource 실제 존재와 필요한 기존 Area publisher 구조검사를 실행한다.
현재 사용자 Visual Studio 빌드 중이므로 제품 빌드와 SDK/DLL/CSO 배포를 하지 않는다.
Client/UI 실행·조작·캡처와 최종 화면 판정은 사용자가 직접 수행한다.
새 Python 파일은 Tools 저작 경로이며 C++ 프로젝트 등록이 필요하지 않다.
신규 Effect JSON 15개는 Client 프로젝트/filters의 None, 96.DataFiles에 추가한다.
실행 중인 기존 exe는 새 metadata를 모르므로 Data와 프로젝트 변경은 out 후보로 먼저 검증하고,
호환되는 새 exe 전환 때 전체 baseline CAS를 확인한 뒤 설치한다.

## G05. 실제 WORLD 준비 오류 후속 수정

2026-09-12 실제 사용자 오류는 저작 world revision673과 runtime672의 신규18 instance 누락, G2 Table의 잘못된 WMSH tail 두 경계로 분리한다. Table은 4 submesh에서 원본 투명 슬롯 두 개를 제거하면서 4개의 bounds를 남겼다. 기존 Engine reader는 새 submeshCount2에 맞는 bounds80bytes만 소비하여 나머지80bytes를 거절한다.

visible_table_mesh는 bone10개를 그대로 보존하고 선택된 submesh1/2의 bounds만 남긴다. 이미 저작된 WANM, skeleton, material section은 그대로 유지한 교체 후보를 out/KoukuSequenceAdmission20260912에 작성한다. 실제 decoder로 후보 및 신규5개 시퀀스가 참조하는 모델을 검사하고, Data/Resources의 현재 bytes는 보존한다. runtime Area Publish와 기존 Table 교체는 실행 중인 사용자 Client/Server 종료 후 부모 작업이 소유한다. 이 검사는 CModel GPU 생성이나 화면 성공 판정이 아니다.

Level의 WORLD admission 실패는 missing instance, disabled, duration, speed, stable ID, duplicate를 구분하고 occurrence/instance ID와 실제 문서 revision 및 supplied snapshot/runtime 출처를 표시한다. WorldSequencePlayer는 기존 CModel Create 직후 같은 thread의 ModelDecoderRegistry report가 요청 meshPath와 일치할 때 실제 decoder 실패 이유를 덧붙인다. decode 성공 뒤 geometry/material 생성 실패는 별도로 표시하며 다른 모델의 오래된 report는 사용하지 않는다. 새 로더나 Engine public API는 만들지 않는다.
