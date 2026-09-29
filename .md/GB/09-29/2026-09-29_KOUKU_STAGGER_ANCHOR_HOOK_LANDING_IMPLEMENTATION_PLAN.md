# 쿠크 무력화 게이지 추종과 갈고리 하차 후 회피 경계

## G00. 현재 실측과 작업 범위

2026-09-29 `codex/kouku-release-sequence-ready`의 기존 미커밋 변경을 보존한다. 사용자는
3관문에서 보스와 무력화 게이지 위치가 어긋나는 화면을 제공했고, 갈고리 하차와 Space 입력이
겹쳐 맵 밖에 남은 것으로 의심된다고 정정했다. 정확한 입력 시각은 아직 재현하지 않았다.
Client/UI는 실행·조작하지 않는다. 사용자가 편집한 KoukuHudModes.json 및 rendering tuning은
그대로 보존하고 새 데이터 게시를 하지 않는다.

## G01. MainApp의 무력화 게이지 기준점

현재 Apply_MechanicBarRect는 고정 rect와 저장 offset만 사용한다. CombatHUDViewModel의
현재 보스 entity ID와 기존 WorldHealthBarView의 실제 actor head anchor를 연결하고,
매 프레임 현재 보스 머리를 같은 화면 투영 함수로 계산해 세 slot 전체를 배치한다. 사용자
후속 지시에 따라 시작 위치 delta 방식은 사용하지 않는다. 기존 화면 고정 mechanicOffsetX/Y는
JSON에 보존하고 새 optional mechanicHeadOffsetX/Y 기본0을 머리 기준 보정으로 사용한다.
F1은 이 두 새 필드를 조절·저장하며 크기 튜닝은 그대로 소비한다. 대상 교체는 즉시 새 actor를
찾고 화면 밖·projection 실패·기믹 종료는 표시만 격리한다.
MainApp.cpp/.h, CombatHUDViewModel.cpp/.h, WorldHealthBarView.h의 기존 소비자를 확장한다.

## G02. PlayerSkillSystem의 충돌 후 최종 지면 검사

회피/root motion은 이동 전 Clamp_StepToWalkable만 적용한다. 이후 collision의 접선 이동이
바꾼 최종 좌표를 navigation 재검증 없이 commit한다. 일반 보행의 최종 지면 검사와 같은
계약으로 실제 이동 결과를 확인하고, 실패한 tick은 기존 위치를 보존한다. 정상 회피의 시각·
cooldown·거리 튜닝은 유지한다. 기존 skill 계약 검사에서 실제 slide와 정상 이동을 검증한다.
navigation이 최종 Y를 보정한 경우에는 그 수직 구간과 최종 player volume도 collision으로
검증하여, 낮은 높이의 안전 판정을 높은 바닥에 그대로 적용하지 않는다.

## G03. GameRoom의 갈고리 해제

기존 정상 terminal ascent는 지면을 검증하지만 owner 소멸·deadline·중단의 공통
Release_PlayerAttachment는 현재 공중 XYZ를 그대로 남긴다. 쿠크 WORLD_HOOK_TIP의
살아 있는 비마리오 플레이어만 해제 전에 가까운 같은 층의 실제 walkable floor와 충돌을
확인한다. 근처 하차점이 없으면 기존 관문 복귀 위치를 검증하여 사용한다. 유효한 지면이
없으면 이동 가능 상태로 풀지 않고 기존 attachment를 유지한다. 해제 전 명령과 강제 이동
잔여 상태도 정리한다. carrier source 모션·본·입장/사망 규칙은 바꾸지 않는다.

GameRoom_PlayerSimulation.cpp와 기존 KoukuOverlap 테스트를 수정한다. 정상 하차,
deadline/owner 소멸, 지면 없는 해제 실패 보존, 하차 전후 Space 입력을 확인한다.
관문 cinematic의 입력 상태 초기화도 미해결 hook 해제를 우회하지 않도록, despawn과
cinematic state commit 전에 같은 해제 검사를 통과해야 한다. 실패하면 기존 lock을 보존한다.

## G04. 검증과 전달

새 C++ 파일·schema·project/filter 등록은 없다. 기존 파일 인코딩을 보존한다. 필요한
Product 증분 컴파일·링크, 변경한 Server focused contract, HUD 실제 소비 함수 검사,
git diff --check를 실행하고 결과를 대응 RESULT에 기록한다. 실행 중 제품과 새 빌드는
구분하며 실제 화면·입력 판정은 사용자에게 남긴다. 다른 세션의 빌드와 겹치지 않는다.

마리오 질문의 현재 계약은 155줄 P88, 125줄 P91, 90줄 쇼타임, 80줄 P92, 55줄 P93이다.
1마리오 실패·미입장도 해당 occurrence를 완료 소비하며 125줄 이하에서 현재 일반 패턴
종료 후 명시 stage2의 P91로 간다. 실패한 1마리오 2페이즈를 재개하지 않는다.


## G05. 댄스 제품 HUD와 보스 HP 분리

후속 요청에 따라 MainApp의 `Is_KoukuMinigameHUDHidden`은 MAZE만 숨긴다. DANCE의
제품 스킬 슬롯·키·미니맵은 기존 데이터와 소비 경로로 표시하고, 별도 보스 HP 숨김 판정을
보스 sprite와 제목·줄수 텍스트 양쪽에 적용한다. KoukuHudModes.json의 사용자 튜닝은
수정하지 않는다. MainApp.cpp/.h의 기존 함수만 확장한다.

## G06. 일반 반복의 나팔과 대형 세이튼 등장

저작 정본 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 Gate2 일반 HP 구간
7개에서 P106(저글링)과 P85(슈퍼바주카) 사이에 P105(나팔) entry를 추가한다. 기존 entry
stable ID·wait·entryGroup 경계는 보존하고 각 새 entry에 고유 stable ID를 부여한다.
revision만 1 증가시킨다. 전체 문서 재직렬화 없이 필요한 entries와 revision을 병합하고,
적용 직전 최신 hash·writer lock·백업·원자 교체로 동시 저장을 보호한다.

대형 세이튼은 ACTION P9 단독이 아니라 bundle.1의 P8/P9 동시 재생이며, 이 bundle의
카메라 resource는 `camera.kouku.pattern.1`이다. Level_KakulSaydonArena.cpp/.h에서
이 shot만 플레이어 표시와 gameplay input을 허용한다. 카메라 재생 여부와 UI/입력 차단
정책을 분리해 MainApp의 cinematic capture와 PlayerController의 입력 판정이 같은 정책을
소비한다. 기존 CPicking → PlayerController → command sink → Server 이동 검증을 유지한다.
Server raid는 이 전투 bundle 동안 COMBAT이며 별도 CINEMATIC 입장 차단을 해제하지 않는다.
Sequence composition P9(빙고 엔딩)는 무관하므로 변경하지 않는다.

## G07. 마리오 비행 공의 고정 피해

`GameRoom_KoukuMiniGames.cpp`의 `Update_MarioBombContacts`에서 flying ball 접촉 피해를
최대 HP 10%에서 고정 1320으로 바꾼다. 충돌 volume·세대별 1회 latch·넉백·탄도·점프 회피는
그대로 유지한다. 기존 KoukuOverlap 테스트의 실제 7개 emitter×2세대 피해를 서로 다른
최대 HP로 검사하여 퍼센트 피해가 다시 들어오지 않게 한다. 별도 세션의 타겟공 색상 masking,
파괴 1/3 표시와 targetball hammer 판정은 이 변경 범위에서 제외한다.

## G08. 후속 검증·통합 경계

기존 dirty C++의 인코딩·줄바꿈을 보존하며 각 요청 부분만 수정한다. projector의 read-only
validate와 flow/정책 focused 검사, JSON parse, diff check를 실행한다. parent가 통합
publisher와 Debug/Release Product 빌드, 실제 Server 계약 검사를 조율한다. 새 Resources나
새 C++ 파일·프로젝트 등록은 필요하지 않다. 설치·게시·실행 중 메모리와 사용자 화면 판정을
구분하며 Client 실행·조작을 수행하지 않는다.
