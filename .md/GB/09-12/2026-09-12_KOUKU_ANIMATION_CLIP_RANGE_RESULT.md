# Kouku animation source range result

## G00 확인된 원인과 구현

Timeline의 기존 앞 trim 계산과 저장은 sourceStartMs를 증가시키고 있었다. UI 수식 자체가 원본 앞부분을 무시한다는 재현은 확인하지 않았다. 실제 닫혀 있던 경계는 Product publisher/loader의 sourceStartMs=0 제한, Npc의 source 시작 미소비, Preview의 전체 native clip 반복, AnimationTool Composition Preview의 clip별 blend 미소비였다.

CompositionDocument AnimationOccurrence의 optional sourceEndMs(0=기존 native 끝)를 추가했다. Source In/Out은 원본 구간, playMs는 timeline 길이, startOffsetMs는 stage 내 시작 지연을 소유한다. 기존 문서에서 새 필드가 없으면 직전 계약을 유지하고, Serialize는 end=0 필드를 생략한다. 잘못된 범위는 해당 저장/preview/publish 단계에서 거절한다.

실제 공통 함수 `Try_SampleAnimationSourceMs`를 두 Preview, 본 anchor, Product Npc initial/update/transition에 연결했다. 선택 구간 반복은 `start + fmod(age*rate, end-start)`다. 원본 WModel duration/ticks-per-second로 admission하고 end의 1ms native 반올림 차이를 허용한다. HOLD_LAST_POSE의 기존 native 밖 source 시작과 기존 blendFromSourceMs native 초과 clamp는 보존한다.

## G01 편집

- F1 Pattern 또는 Sequencer의 Animation row → Box Details에서 Source In, Source Out, Play ms를 편집하고 Apply Trim으로 저장 초안에 반영한다.
- Source Out 0은 native 끝이다. 중간 구간을 반복하려면 Source In/Out을 지정한 뒤 Apply Trim, Repeat Selected Range to Stage End를 사용한다. 스테이지 길이를 늘린 뒤 같은 명령을 적용하면 해당 구간을 더 오래 반복한다.
- 비반복 clip의 왼쪽 edge는 timeline 시작과 Source In을 같이 옮기고 Source Out을 보존한다. 오른쪽 edge는 Source In을 보존한다. 반복 설정 뒤 양쪽 edge는 고정된 source 구간을 보존하고 timeline 길이만 조절한다.
- Blend In은 바로 인접한 이전 EXACT/HOLD/LOOP occurrence의 실제 마지막 source 표본에서 현재 clip으로 전환한다. 1ms보다 짧아 정수 source 범위를 표현할 수 없는 반복 선택은 명확히 거절한다.

사용자가 편집한 Pattern 1의 STAGE_43/51/53/54/55/52 및 별 그리기 `rpct00_att_battle_6_02` 배치는 자동으로 저장하거나 변경하지 않았다. 클립 내부 source만 바꾸면서 연결을 유지하려면 timeline offset을 0으로 둔 채 Source In/Out을 편집한다. 앞 edge로 자르면 해당 stage에 시작 지연 구간이 생긴다.

## G02 Product 및 판정 시간 연결

`PresentationAssetService H/CPP → ClientReplication → Npc H/CPP`가 Source In/Out을 실제 Server action clock에 연결한다. source 구간 정보는 Client presentation이며 Server action duration, 이동 권위, damage 수치를 우회하지 않는다. Product의 stage당 occurrence 1개 및 rate 0.1..4 경계는 유지한다.

후반 stage의 시작 지연도 지원한다. 지연 중 새 stage 첫 clip의 Source In을 유지하는 정책을 Npc, Preview 2개, source anchor, publisher bone bake에 통일했다. Preview의 iPoseStartMs는 stage 및 clip 시작에서 계산하는 실행 값이며 authored JSON에 저장하지 않는다. Product sourceAnchorAnimations에는 필요한 경우에만 stageStartMs를 투영한다. 원본 배열 순서에 의존하지 않고 startOffset/occurrenceId 순서로 첫 clip을 선택한다.

Publisher의 실제 WModel bone collider bake도 source 범위 반복과 이전 clip의 cropped blend endpoint를 사용한다. 기존 stage fixed-tick origin, external deadline의 마지막 stage 유지, weapon pose mapping과 절대 bossMotion은 보존한다.

## G03 비UI 검증

- `out/KoukuAnimationClipRange20260912/compile.log`: 최종 4 CPP(CompositionDocument, ActionWorkbench, PresentationPlayer, Animation_Tool) out-only cl exit 0. 객체 작성 시간 12:45:43~45, 최종 source 변경 12:42:16~59. 기존 EngineSDK 인코딩 경고 외 컴파일 오류 없음.
- `source_time_probe.run.log`: 최종 실제 함수/Occurrence struct를 추출해 앞/뒤 trim, rate, loop boundary, hold, legacy, 잘못된 range 및 loop front +2000ms의 source200..700 유지 PASS.
- `codec_probe.run.log`: 실제 Parse_Text/Serialize/Validate + DataJson을 링크해 source700..1200 roundtrip, optional end=0 생략, derived clock 저장 제외, invalid range 거절 PASS.
- `preview_sample_probe.run.log`: 실제 AnimationTool sampler 본문 + CPU CModel cursor observer. 1050ms blend 뒤 1200ms clip 진행, 후반 stage 시작 지연의 새 Source In 유지, cropped loop 경계 PASS. 보간 뒤 explicit pose를 해제해 같은 clip이 멈추지 않음을 확인했다.
- `python_focused.log`: 관련 8개 기존/신규 projection 검사가 최종 OK(0.168초). Python syntax 및 변경 파일 git diff --check PASS.
- `producer_fixture.run.log`: 설치된 Saydon 별 그리기 native clip source700..1200, loop1750ms → 이전 blend endpoint950ms. source 입력 불변, legacy end=0/미지정의 출력 byte 동일, 실제 body/hammer bone collider track bake PASS. fixture 출력은 out에만 있다.
- `out/KoukuAnimationSourceRange20260912/probe.run.log`: Product reader/CDataJson/ProjectDataRoot, Npc 실제 함수, Replication 실제 인자 경로 47 checks PASS. 정상 7행/invalid 6행 격리, malformed reload의 이전 상태 보존, cropped source/loop/지연/전환/legacy 경계 포함. 3개 해당 Product CPP out 컴파일 PASS.
- 독립 source review에서 지적된 explicit blend pose 잔류, 배열 front 기반 첫 pose 선택, 반복 앞 trim의 구간 손상, sub-ms의 end=0 sentinel 변환을 모두 수정했고 재검토했다.

CPU CModel observer는 실제 GPU palette나 화면 판정의 대체물이 아니다. 이 작업에서 Client/UI 실행, 화면 캡처, Resources/DLL/CSO 설치, 실제 Data 저장은 수행하지 않았다. 제품 최종 빌드/배포 상태는 통합 작업 결과를 따른다.

## G04 기존 테스트 편집 보존

검증 파일의 범위 치환이 약 500줄을 일시 삭제한 것을 diff에서 즉시 발견했다. 손상본/당시 diff를 out에 보존한 뒤, HEAD 원문과 기존 PatternStartOffset 작업의 실제 도구 호출 기록 ordinal226/233/241로 기존 추가 테스트 원문을 정확히 복원했다. 원문 복원본은 `test_after_exact_prior_recovery.py`, 근거는 `prior_offset_toolcalls.json`이다. 이후 sourceStart/후반 stage 지연을 새로 허용하는 기대값만 별도 변경했고 원래 LF 개행을 보존했다. source/Data 제품 파일에는 이 편집 오류가 발생하지 않았다.

## G05 남은 화면 확인

새 Client로 위 편집 경로에서 별 그리기 중간 구간을 선택해 반복하고 다음 인접 clip의 Blend In 0/100ms를 비교한다. Pattern Save/Publish 뒤 Server playback과 Sequencer Preview에서 같은 Source In/Out이 사용되는지 사용자가 직접 확인한다. 실제 연속성이나 visual fidelity는 아직 사용자 판정 전이다.

## G06 세이튼_1관문연출의 native 키 편집 초안

사용자가 직접 만든 Pattern KAKULSAYDON_G1_PATTERN_35(세이튼_1관문연출, MN_RPCT_05/GATE1/boss.kakulsaydon.g1.saydon)에 10종 native clip을 13개 순차 구간으로 연결했다. 저장 위치는 out/PreviousAnimationSession20260912/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json이며 사용자 추가 저장을 재읽어 revision 344→345로 반영했다. source Data 정본 및 다른 Pattern/Flow/효과 객체는 그대로다.

전체 길이는 41,488ms, 첫 walk 시작 지연은 10,277ms다. 원본 key의 속도/loop와 talk Source In 800ms, 마지막 idle Source In 1,000ms를 반영했고 sourceEndMs와 임의 blend는 추가하지 않았다. 기존 RAW sourceActionId=0 경로를 사용했다. 이것은 원본 native 키 발생 순서의 편집 초안이며 원본 A/B 동시 혼합, 배우 위치 교대, 카메라/HUD/서버 전투 전환 완료가 아니다.

후보와 설치 근거는 out/KoukuActualBossSequenceReview20260912/user_intro_candidate.json 및 user_intro_install_receipt.json이다. 실제 설치 WModel의 10개 clip 존재/길이를 원본 표와 대조했고, 기존 projector.validate_document로 변경한 Pattern을 검사했다. 다른 객체의 전후 비교 및 atomic replace 뒤 readback이 일치했다. 설치 중 첫 SHA 비교에서 사용자의 새 저장을 발견해 교체하지 않고 최신 문서에서 다시 stage했다. 별도 검토 에이전트도 같은 후보를 다시 읽어 검사했다.

전체 기존 문서는 변경 전후 모두 presentation occurrence exceeds the Pattern lifetime로 동일하게 거절된다. 이번 수정과 관계없는 사용자 draft의 길이를 고치지 않았고, 해당 Pattern만 검사한 결과를 전체 Publish PASS로 표현하지 않는다. 재사용 가능한 argv 기반 Composition C++ codec probe가 없어서 후보의 실제 C++ 로드는 실행하지 않았다. C++/runtime 파일 변경과 추가 빌드, Server Publish는 수행하지 않았다.

사용자가 Workbench Reload → 세이튼_1관문연출 → Animation으로 확인한다. 실행 중인 이전 EXE는 첫 지연 구간에 Preview 시작 전 pose를 유지하며, 첫 walk Source In을 유지하는 정책은 새 빌드에 적용돼 있다. 화면의 pose/연속성은 사용자 확인 전이다.
