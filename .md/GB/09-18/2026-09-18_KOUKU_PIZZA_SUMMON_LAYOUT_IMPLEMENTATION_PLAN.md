# 쿠크 피자 Summon의 테이블 좌우 배치 구현 계획

## G00. 현재 실측과 목표

2026-09-18 작업 기준 HEAD는 `a198c9406db46b92f8b8aeaf087a538733f259ea`, 브랜치는
`codex/shader-build-isolation-20260917`이다. 다른 기능의 대규모 미커밋 변경을 보존하며 자동 commit하지 않는다.

`KAKULSAYDON_G1_PATTERN_25`의 `summon.1`은 `kakulsaydon.g1.summon.8`을 1461 ms에 시작해
16586 ms 유지하지만 현재 `patternSpawns`가 없다. `KAKULSAYDON_G1_PATTERN_84`는
쿠크_훌라후프_레드이며 Animation, BOSS Effect, ALBION_AIRBORNE JUMP/SLAM을 갖는다.
기존 Summon은 최대 4개의 Animation-only 패턴을 본체 상대 좌표에 생성한다.

목표는 기존 Summon occurrence에서 같은 훌라후프 패턴을 좌우 5명씩 생성하는 것이다.
Parent timeline이나 World Object 복사본을 만들지 않고 기존 dependent clone과 독립 pattern clock을 사용한다.

## G01. Document와 저작 UI

`Client/Public/KoukuSaydonCompositionDocument.h`의 spawn은 optional `anchorKind`를 추가한다.
생략/`BOSS`는 기존 위치·방향 상대 좌표, `MAP`은 맵의 절대 위치·yaw다. 기존 필드
`positionOffset`, `yawOffsetDegrees`를 사용하고 UI에는 선택한 기준에 맞는 이름을 표시한다.
최대 spawn은 16개이며 spawnId는 occurrence 안에서 유일하다.

`KoukuSaydonCompositionDocument.cpp`의 parse/validate/serialize와 Python projector는 같은 계약을 검사한다.
대상은 같은 Gate, actor, boss placement의 leaf MECHANIC이다. Animation, BOSS 기준 Effect/Sound와
자기 배우에게만 적용하는 ALBION_AIRBORNE JUMP/SLAM을 허용한다. 중첩 Summon/Parent, 전역 lane,
플레이어 선택·이동, outcome 분기와 카메라/조명/맵 제어는 허용하지 않는다.

`KoukuSaydonActionWorkbench.h/.cpp`의 Summon Box Detail에서 각 spawn의 Pattern, MAP/BOSS,
위치, yaw를 편집한다. 비어 있는 spawn 목록에 중심·좌우 거리·행 간격으로 5+5를 생성한다.
전체 변경은 candidate 검증 후 commit하며 기존 목록을 덮어쓰지 않는다. 필요하면 child 전체 길이에 맞춰
Summon/부모 lifetime을 늘려 재생 종료가 잘리지 않게 한다.

## G02. Play Preview와 Server Play

`Client/Private/KoukuSaydonPresentationPlayer.cpp`의 기존 clone preview가 각 배우의
root-motion/airborne sampler와 Effect session을 준비한다. 독립 actor identity와 수명을 유지하고
되감기·Stop·실패 때 기존 clone cleanup을 사용한다.

`Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`,
`Server/Private/KoukuSaydonBrain.cpp`, `Server/Private/GameRoom_BossSimulation.cpp` 및 필요한 기존 헤더는
동일 spawn 수/좌표 기준과 child admission을 소비한다. 기존 SUMMON_PATTERNS의 stage → commit 생성,
navigation 검사, dependent owner/sequence/lifetime 정리와 snapshot 전송을 유지한다.
clone의 JUMP/SLAM은 기존 airborne 함수에 연결하며 최초 0 ms trigger는 spawn 표시 전에 적용한다.

`Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`와
`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`는 source → Encounter → bootstrap 필드를 연결한다.
새 Shared packet이나 두 번째 actor runtime은 추가하지 않는다.

## G03. 검증과 반영

기존 focused 검사에서 10개/16개/17개, MAP/BOSS, 중복 spawn ID, 잘못된 child,
JUMP/SLAM 및 Effect, 부족한 lifetime과 부모 종료 정리를 확인한다. 최소 Product Debug Build,
변경 JSON/XML parse, `git diff --check`를 수행한다. 새 C++ 파일은 예정하지 않아 project/filter 등록은 없다.

저작 데이터는 사용자가 편집 중인 정본을 직접 교체하지 않고 후보를 준비한다. 최종 교체 시
AGENTS.md의 편집 중 데이터 반영 규칙에 따라 저장/현재 디스크 기준 승인을 한 번 확인한 뒤
최신본 stable ID와 대상 field 기준으로 병합한다. Client 실행·UI 조작·화면 검증은 사용자가 한다.
실제 구현, 자동 검사, 게시/설치와 미실행 화면 검증은 대응 RESULT에 구분한다.
