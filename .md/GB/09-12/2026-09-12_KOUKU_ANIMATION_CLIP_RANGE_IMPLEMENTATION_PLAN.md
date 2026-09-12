# Kouku animation clip range implementation

## 목표와 현재 근거

앞쪽 트림은 timeline 시작과 source 시작을 함께 옮기고 원본 끝을 보존한다. 뒤쪽 트림은 source 시작을 유지한다. 선택한 중간 구간을 원하는 timeline 길이 동안 반복하고, 인접 클립 전환의 Blend In을 Pattern/Sequencer Preview와 Product에서 같은 시간으로 샘플한다. 사용자가 저장한 STAGE_43/51/53/54/55/52와 별 그리기 클립은 수정하지 않는다.

현재 Workbench front trim 및 commit은 sourceStartMs를 보존하지만 publisher는 Product sourceStartMs!=0을 거부하고 Product loader/Npc에는 source 시작 소비가 없다. Preview의 LOOP_TO_WINDOW는 원본 전체 duration에 fmod한다. AnimationTool의 Composition preview는 blendInMs를 무시한다. 기존 반복과 source 범위는 별도 저장할 수 없다.

## 파일과 호출 경로

기존 CompositionDocument H/CPP에 optional sourceEndMs(0=원본 끝)와 순수 source 시간 함수를 추가한다. Workbench H/CPP는 source 끝 입력과 선택 범위 반복 설정을 저장한다. Timeline 앞/뒤 edge는 동일 원본 범위를 유지한다. PresentationPlayer와 AnimationTool은 같은 함수를 소비한다. Product publisher는 source 범위를 검증/투영하고 bone collider source pose도 같은 범위/전환을 소비한다. PresentationAssetService → ClientReplication → Npc는 기존 server action clock으로 source 위치를 샘플한다. 신규 CPP/project 등록은 없다.

## G00 계약

명시 sourceEndMs는 sourceStartMs보다 커야 한다. end 0은 기존 문서처럼 native 끝이다. LOOP_TO_WINDOW는 start + remainder(age*rate, end-start)를 사용한다. 반복 구간과 timeline playMs는 별개다. 기존 sourceStart=0/end미지정 재생을 보존한다. 클립이 없는 경우 또는 잘못된 range는 해당 preview/publish만 거절하고 기존 문서를 보존한다.

## G01 편집과 저장

기존 sourceStartMs/playMs와 optional sourceEndMs를 parse/validate/serialize한다. Source In/Out과 Play ms를 각각 편집한다. Loop selected range to stage end 명령은 범위를 고정한 뒤 남은 stage 길이로 playMs를 늘린다. 트림은 기존 timeline 시작/끝의 의미를 보존하고, 반복 설정 후 timeline 길이 조절은 고정된 반복 범위를 바꾸지 않는다. Blend는 인접한 EXACT/HOLD/LOOP의 실제 마지막 샘플을 사용한다.

## G02 Preview와 Product

두 preview와 bone anchor, Npc의 initial/normal/transition/held sample을 연결한다. Product sourceStart 및 후반 stage 시작 지연 차단을 제거하고 기존 stage당 한 occurrence 및 rate 0.1..4 제한은 유지한다. 각 stage의 clip 시작 전에는 그 stage 첫 clip Source In pose를 유지한다. Server action duration/logic/damage/position 권위는 바꾸지 않는다. Bone collider의 기존 bake 경로에서 source pose를 동일하게 샘플한다. 절대 bossMotion/root position 저장은 source clip과 독립인 기존 저작 계약을 보존한다.

## G03 검증

out에 실제 함수 CPU 검사(앞/뒤 trim, source 범위, loop 경계, blend endpoint)를 작성한다. 기존 projector 함수로 legacy fixture와 trimmed/loop/blend fixture를 검사하며 malformed range 거절을 확인한다. 변경 H/CPP의 out-only 컴파일과 Python syntax, git diff --check를 수행한다. source Data 및 실행 중인 Client/Server, Resources, EngineSDK, DLL/CSO는 변경하지 않는다. 화면의 연속성 판정은 사용자가 직접 한다.

## G04 세이튼_1관문연출의 native 클립 편집 초안

사용자가 직접 연결을 요청한 Pattern은 이전 EXE 작업 사본 out/PreviousAnimationSession20260912/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json의 KAKULSAYDON_G1_PATTERN_35다. revision 343에서 MN_RPCT_05/GATE1/boss.kakulsaydon.g1.saydon이며 모든 occurrence가 비어 있다. 해당 Pattern의 stages와 nextStageOrdinal/nextAnimationOrdinal 및 문서 revision만 변경한다.

SCENE03A의 원본 키 발생 순서에 따른 10종 native clip을 13개 순차 stage, 총 41,488ms로 배치한다. 첫 walk 시작 지연 10,277ms, 말하기 Source In 800ms, 마지막 idle Source In 1,000ms와 원본 key의 속도/loop를 보존한다. 비반복은 HOLD_LAST_POSE로 끝을 유지한다. 기존 RAW/sourceActionId=0/sourceStageId=RAW 경로를 사용하고 sourceEndMs와 임의 blend 값은 추가하지 않는다.

이것은 동시 A/B weight와 배우 3명의 월드 위치 교대를 보존한 원본 재현이 아닌 편집 초안이다. 카메라/HUD/같은 서버 보스 전투 전환은 시퀀스 전투 연결 작업의 남은 범위로 구분한다. 다른 Pattern/Flow/효과 및 원본 저장소 Data와 Resources는 수정하지 않고 Server Publish도 하지 않는다.

설치 전 후보를 기존 projector.validate_document와 실제 설치 WModel의 이름/길이로 검사한다. 사용자 파일을 백업하고 전체 SHA 비교 후 JSON의 해당 Pattern 및 revision만 치환한다. 다른 저장이 발생하면 최신 파일에서 대상의 비어 있음과 identity를 다시 검사한다. Client/UI 조작 없이 사용자가 Workbench Reload 후 확인하며, 이번 데이터 편집에 추가 C++ 빌드는 필요하지 않다.
