# 발탄 Composition 패턴·World Object·V2 편집 복구 결과

작성일: 2026-09-11. C++ 구현 및 CPU 소스·데이터 검사 완료, 통합 컴파일·native 실행과 사용자 화면 확인은 별도 상태다.

## G00. 실제 패턴과 기존 연출 보존

현재 split gameplay는42개 패턴이고 게시된 `ValtanEncounter.json`에는65개가 있다.42개는 모두
generated encounter에 있으며 나머지23개는 compatibility 정의다.09-09 계획의 legacy25는 당시
projection 집계이고 현재 Browser 숫자의 정본으로 재사용하지 않는다.

Browser는 canonical 전체를 나열한다. 관리42개는 기존 typed writer/Server Play 경로를 사용하고,
나머지는 compatibility reference 표시와 기존 local Preview를 제공한다. 기존 writer admission은
완화하지 않았다. Sound726개, Shake128개, presentation Camera11개 source와 timeline 생성 경로는
보존했다. Sound/Camera의 실제 아레나 재생은 이번 source 검사만으로 완료 처리하지 않는다.

## G01. World Objects 목록과 같은 Pattern 소비자

`ValtanActionWorkbench`에 World Objects resource tab과 별도 timeline lane을 추가했다.
기존9개 combat-object 정의의 사용 Pattern/Stage 및 World event를 검색하고 선택할 수 있다.
다른 Pattern으로 이동할 때 기존 deferred selection의 Save/Discard/Cancel 경계를 그대로 사용하며
선택된 object의 stable ID를 전환 완료 뒤 Box Detail에 전달한다.

도넛 등은 자신의 Server origin/direction/anchor와 lifetime을 표시한다. 기존 ring inner/outer
편집은 `CBalanceTool`의 typed draft와 기존 Save를 사용한다. Play는 그 object를 spawn하는 기존
Pattern을 재생한다.9개 object의 V1/V2 표현 선택은 바꾸지 않았다. 기존 저작된 도넛·돌기둥 V1을
임의 V2 group으로 치환하지 않는다.

Map World anchor/motion의 신규 범용 저작 writer와 임의 World object 생성은 추가하지 않았다.
현재 object의 기존 Server spawn 계약을 노출한 범위와 쿠크의 모든 World box 편집 기능이 동일하다고
기록하지 않는다.

## G02. V2 occurrence 선택·시간·상세 편집

기존 convenience `strStage`와 transform 합성 ID를 사용하던 UI를 typed bindingId와 정확한
pattern/stage/action scope, clock basis, clipOccurrenceId로 바꿨다. 같은 loop clip을 세 번 사용한
`VALTAN_BIND_SLOT`의 shout는 각각1400/2300/3200ms로 투영하며 선택 ID도 세 개를 유지한다.
이제 start/anchor 변경으로 선택 ID가 변하지 않고 Duplicate는 실제 새 bindingId를 선택한다.

V2 Box Detail은 clock/occurrence/start/repeat, anchor slot/follow/rotation basis,
local translation/rotation/scale, stop policy를 Apply로 stage한다. 입력 중 buffer와 저장 owner를
분리했고 실패하면 기존 snapshot과 draft를 보존한다. Apply와 timeline drag/duplicate의 source
window 검증은 같은 `Validate_EffectV2BindingClock`을 사용한다.

`CEffectV2Catalog::Stage_UpdateBossValtanBinding`은 exact bindingId를 찾아 scope/resource 불변을
검사하고 기존 `Commit_BossValtanBindingsLocked`의 strict parse/cross-validation, baseline/CAS,
immutable snapshot commit을 사용한다. 새로운 저장 파일이나 runtime은 추가하지 않았다.

V2 runtime의 pitch/roll/scale 소비 누락은 독립 검토에서 확인하여 animation 담당이 기존
`Binding_Local` 및 occurrence scale 소비 경로를 수정했다. 해당 담당은 실제 함수 본문의
DirectXMath CPU 검사를 실행해 scale axes, pitch/roll, parent scale 1회 합성, legacy yaw
동등성을 확인했다. Product 통합 컴파일과 화면 상태는 별도로 기록한다.

## G03. 실행한 자동 검증

- 기존 V2 projection/catalog/resource 회귀24개 PASS.
- 기존 dirty-owner Save/atomic Save/occurrence timing/sequence identity 회귀32개 PASS.
- 변경 project/filter XML parse, C++ UTF-8 및 기존 CRLF, `git diff --check` PASS.
- `VALTAN_BIND_SLOT` 저장 row3개의 bindingId와 occurrence1400/2300/3200ms 확인.
- 기존 twohand fixture1033ms를1000ms로 교정했다. 현재 파일뿐 아니라 작업 전 HEAD의
  `binding.valtan.migrated.013.bd77282dde819f18`도1000ms였고 binding 원본은 이번 작업에서 변경하지 않았다.

검사 로그는 `C:/Users/user/AppData/Local/Temp/LostArkValtanRestore20260911/`의
`composition-v2-regression.log`, `composition-save-regression.log`다. 위 검사는 source/data 계약이며
Client 렌더링·입력 성공을 의미하지 않는다.

기존 `ValtanPatternAuditionServiceHarness`에 Catalog 소스/헤더만 등록하고 기존 presentation-generation
검사 안에 실제 Catalog API fixture를 추가했다. 유효 full-detail stage→Save 준비→Product parser 재개방,
invalid anchor/clock/foreign scope 거부, stale receipt/미커밋 disk bytes의 Accept 거부 시 snapshot·dirty·
cache invalidation 보존을 확인하도록 구성했다. CPU fixture의 GPU runtime cache invalidation은 계수 stub이며
GPU 재생 검사가 아니다.

통합 담당자의 native 컴파일은 성공했으나 최초 실행은 Catalog leaf `boss.valtan.hand_1` 누락으로
실패했다. 물리 JSON과 참조 DDS는 존재했고, 원인은 Tools 아래 실행 파일의 기본 ResourceRoot가
Client의 Resources를 가리키지 않은 fixture 설정이었다. 같은 native binary에
`LOSTARK_RESOURCE_ROOT=Client/Bin/Resources`의 절대 경로만 지정하여 전체 admission/API 검사를
다시 실행한 결과 exit0 PASS였다. 로그는 같은 TEMP의
`native-contract-resource-root-diagnostic.log`다. Product의 parser/strict validation은 변경하지 않았다.
해당 API fixture block에 정상 ResourceRoot 환경 설정과 이전 값 복원을 추가했으며,
수정 test의 재컴파일 및 기본 환경 실행은 통합 담당자가 확인한다.

## G04. 사용자 확인 경로와 남은 상태

1. Server와 Client를 시작하고 `Lobby → Valtan`에 입장한 뒤
   `F1 → Open Action Workbench → Boss: Valtan`을 연다.
2. `Composition Patterns`에서 관리 패턴을 선택한다. 실제 보스와 연결된 Effect/Sound/Camera/World는
   `Action Workbench → Server Playback → Play on Server`로 재생한다. `Composition Sequencer`와
   `Composition Preview`의 일반 `Play`는 local Arena Clone의 Animation/Effect 및 collider mirror를
   미리 보는 버튼이다. Sound의 seek/stop과 Camera/Light/World의 local transport는 제공하지 않는다.
3. 도넛은 `Composition Resources → World Objects`에서 `donut`를 검색한 뒤
   `combatobject.valtan.fist-in-out.donut` 또는 `donut-large`를 선택한다. 각각 `도넛 / INNER`,
   `큰 도넛 / INNER` owner로 이동한다. 전체 spawn/hit/연출은 같은 `Play on Server`로 확인한다.
4. `Box Detail`의 `Ring inner (m)`/`Ring outer (m)`로 도넛 반지름을 편집하고,
   도끼의 `Axes per alive player`로 기존 개수를 편집한다. origin/direction/Map World anchor와
   motion/lifetime은 표시 항목이며 신규 범용 placement writer가 아니다.
5. V2 Effect box를 선택해 `Box Detail` 입력→`Apply V2 Binding`→`Composition Sequencer`의
   `Save`를 누른다. 정본은 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`이며
   기존 Save receipt가 성공하면 exact committed revision을 재개방한다. `Publish after Save`까지
   완료한 변경을 전체 재생하려면 게시 완료 뒤 Server를 재시작하고 Valtan에 다시 입장한다.

`[compatibility reference]`23개는 저장된 Pattern/Stage/트랙과 local Preview를 확인할 수 있다.
`Begin_WorkbenchFrame`의 playable membership 검사로 writer와 `Save`, `Play on Server`는
비활성화한다. 전체 패턴이 동일한 authoring owner로 승격된 것은 아니다.

에이전트는 Client/UI를 실행·조작·캡처하지 않았다. 실제 입력/화면/수동 Save·재열기와 발탄의 최종
material/animation/effect/sound/camera 품질은 사용자 확인이 남아 있다.
