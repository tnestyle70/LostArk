# 공용 Sequencer 편집과 독립 Object 복제 결과

## G00. 반영한 편집 계약

공통 `CompositionEditing.h`와 `ICompositionWorkbenchSession`으로 Copy/Paste/Duplicate 및
Resource 드래그를 연결했다. clipboard는 원본 문서와 분리된 저작 값 snapshot이며 source row
포인터·preview/audio handle을 공유하지 않는다. 공통 toolbar와 Ctrl+C/V/D는 같은 owner API를
호출하고, 텍스트 입력·팝업·드래그 중에는 단축키를 실행하지 않는다. pane 참조의 수명이 끝난
뒤 편집하며 각 owner의 기존 저장·게시 경로를 유지한다.

World Object 전체 복사는 model/material/combatBody와 직접 연결 Motion 및 NEXT 연결을
포함한다. Object/template/instance ID를 재발급하고 default·binding·NEXT를 재연결한다.
원본 갈고리의 15 emission, animation 4개, `HOOK_CAPTURE`, `b_hook_01`과 grip을 유지하며
복사본의 위치·방향·개수·Motion을 원본과 독립적으로 바꿀 수 있다. 기존 빈 Object에 Paste하면
목적지 이름·ID·Parent를 유지한다. Motion만 선택하면 해당 Motion과 NEXT 연결만 복사한다.

Kouku는 기존 다중 선택 timeline candidate clone을 사용한다. Character의 실제
EffectAuthoringSequencer는 Animation/Effect/Sound/Collider/Camera 저작 값을 새 ID로 복사하고
runtime handle을 초기화한다. 공통 V1/V2 Effect 참조는 대상 owner가 TRS·시간·부착 정책을 모두
표현할 수 있을 때만 가져온다. 본·모델·clock·fit/loop 등 지원 불가능한 정책은 조용히 버리지
않고 전체 후보를 거절한다. Animation은 대상 모델과 실제 clip package를 검증한다.
고정/world snapshot 좌표는 owner마다 절대 좌표와 발생 순간 상대 좌표의 의미가 다르므로
교차 owner Effect 전달을 거절한다. 같은 owner의 native snapshot 복사는 원래 좌표 정책을 유지한다.

Resources의 이전 타임라인 선택이 Ctrl+C/D로 잘못 복사되지 않도록 focus 경계를 둔다.
공통 Animation Library는 Copy Resource/Ctrl+C/drag를 제공하며 owner별 Effect resource는
drag를 사용한다. 각 owner의 기존 다중 선택 방식 전체를 새 선택 모델로 교체한 작업은 아니다.

Valtan은 Animation/V1·V2 Effect/Sound/Scene/Light/Camera와 기존 typed Logic action,
managed Summon 복사를 연결했다. Summon은 정의·visual·spawn event·hit ID를 독립 복제하며
V2 visual의 serverHitId를 함께 재연결한다. 값 복제의 source revision을 검사하고 삭제된 미저장
clone ID도 재사용하지 않는다. V2 batch 삽입은 분리된 후보 전체를 한 번에 commit한다.

## G01. 실행한 검증

- 실제 WorldSequenceDocument/DataJson native 검사 87개, 실패 0개. 원본 갈고리 보존,
  독립 ID·NEXT 재연결, 빈 Object 두 곳에 붙여넣기, 원본 불변, 실패 후보 보존, Save/Load 포함.
- 최신 실제 WorldObjectTool adapter 함수 검사 47개, 실패 0개. preview·animation inventory·effect
  catalog 및 사용하지 않는 Travel 분기는 stub이며 UI/GPU 판정이 아니다. generic anchor 거절의
  전체 상태 보존과 native Object bundle의 원래 flags 보존을 구분해 확인했다.
- 실제 Character 편집 함수 검사 23개, 실패 0개. 준비 소비자는 stub이며 값 복사·새 ID·handle
  초기화·명시/자연 수명·여러 행 실패 rollback·generation 변경 및 모호한 anchor 거절을 확인했다.
- 공통 clipboard/shortcut 실제 헤더 검사 22개, 실패 0개. 복사 실패 시 clipboard 보존,
  duplicate의 clipboard 불변, 목적지 오류 전달, focus/text/popup/drag 경계를 확인했다.
- Parent publisher의 hierarchy/v3 Object/combined selection 기존 unittest 3개 재실행 PASS.
- WorldObjectTool, SequencerTool, CompositionResourceTree, KoukuSaydonActionWorkbench,
  CharacterActionWorkbench, EffectAuthoringSequencer Timeline/Resources, ValtanActionWorkbench,
  EffectV2_Catalog, BalanceTool 실제 TU 개별 컴파일 PASS.
- Valtan typed combat-object writer의 집중 동작 검사 8개 PASS. 정의·visual·hit-sync ID 보존,
  source 누락·ID 충돌·수명 불일치 거절 및 입력 문서 보존을 확인했다.
- Client.vcxproj와 filters XML parse 및 새 헤더 단일 등록 PASS. `git diff --check` PASS.

정상 Debug Product Build/배포는 **PASS**다. 결과는
`out/BuildPipeline/runs/20260924T115544193Z-debug-product.json`, 빌드 로그는
`out/SharedSequencer20260924/product-build-final.log`다. 최종 Client.exe는
`Client/Bin/Debug/Client.exe`이며 2026-09-24 20:55:43 KST에 링크됐다.
첫 정상 빌드는 KoukuSaydonActionWorkbench의 object section 한도(C1128)로 실패했다.
해당 파일에만 `/bigobj`를 추가한 후 재빌드에서 Client 97 OBJ와 executable이 생성됐으며
runtime 파일·Navigation 참조·Item/Valtan reward catalog 검사도 PASS다. 데이터 publish는 하지 않았다.

검사 로그와 fixture는 `out/SharedSequencer20260924/{core,world-copy,adapters,valtan}`에 있다.
추가로 실행한 기존 검사 전체를 성공으로 기록하지 않는다. EffectV2 source contract는 14/16 PASS다.
기존 두 실패(method: `test_stage_binding_mutations_use_binding_id_identity_and_typed_append`,
`test_valtan_runtime_uses_catalog_revision_and_valid_subset`)는 HEAD 코드를 읽는 동일 검사에서도
각각 문자열 slice와 옛 Reload 함수 이름 때문에 재현됐다. 새 공통 resource/batch 검사는 PASS지만
source assertion이므로 실제 catalog 재생 증거는 아니다. Valtan canonical commit 4개 중 2개는
임시 Resources의 `Effect/Esther/Wei/Textures/FX_TEX_00/fx_a_fire_023.dds` 누락으로 projection에서
중단됐다(`test_saved_authoring_revision_carries_boss_catalog_owner`,
`test_three_owner_commit_and_intermediate_failure_rollback`). 해당 두 commit 경로는 PASS가 아니다.

## G02. 사용 방법과 남은 범위

Object 목록에서 원본 갈고리 Object를 선택하고 Copy한다. 이름을 미리 만든 빈 갈고리 Object를
선택하고 Paste하면 그 Object를 채운다. 두 번째 빈 갈고리에도 따로 Paste한다. 바로 새 형제를
만들려면 원본 Object에서 Duplicate를 사용한다. 각 복사본의 Motion과 emission 위치·방향·개수를
편집하고 기존 Save/Publish를 사용한다. Parent 이동은 계속 정리용이며 동작을 상속하지 않는다.

실제 포획 확인은 게시된 Composition의 Play (with Collisions)와 Server 계약을 사용한다.
일반 Play는 시각 preview다. Client/UI를 자동 실행·조작·캡처하지 않았으며 입력·화면 판정은
사용자가 직접 한다. 현재 저작 데이터나 게시 데이터에 갈고리를 임의로 생성하거나 publish하지 않았다.

Albion 파란 장판을 발탄 도끼에 붙이거나 다른 보스의 전투 Logic을 Character/Object로 조립하는
런타임 계약은 아직 구현하지 않았다. 현재 도끼는 Server가 생성 위치에 즉시 고정한다.
표현 리소스 공유와 Server 피해·추적·포획 로직 공유를 같은 완료 상태로 기록하지 않는다.
이번 빌드는 공통 편집과 독립 복제까지의 단계이며 다음 런타임 조합은 별도 후속 범위다.

공유 worktree의 무관한 변경은 보존했다. 자동 commit/push는 하지 않았다.
