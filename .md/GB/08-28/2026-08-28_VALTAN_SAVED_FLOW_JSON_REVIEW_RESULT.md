# Valtan Boss Tool 저장 Flow JSON 검토 결과

작성일: 2026-08-28
상태: **저장본 검토 및 Python 회귀 수정 완료. 게임 C++·JSON·Save·Client 화면은 변경/조작하지 않음.**

## 1. 결론

사용자가 변경한 순서는 실제 디스크 JSON에 저장되어 있다. 저장본은 현재 strict validator를 통과하며,
Client 문서 → 전송 packet → Server sequence가 모두 JSON의 **배열 순서**를 보존한다.
`slotId` 끝의 숫자는 발급 identity이지 정렬 기준이 아니다.

따라서 현재 저장본으로 `Start First`를 누르면 `.slot.000029`의 `VALTAN_FLOOR_WIPE_130`이 먼저 실행된다.
`VALTAN_COUNTER`는 `VALTAN_SEQUENCE_FOUR` 바로 다음이다. 단, 이 검토는 사용자가 마지막으로 저장한
디스크 상태를 확인한 것이며 현재 열려 있는 UI의 미저장 draft나 전체 Flow의 실제 완주를 판정하지 않는다.

대상: [ValtanBossAuditionFlows.json](C:/Users/user/Desktop/LostArk/Data/Encounters/Valtan/ValtanBossAuditionFlows.json)

| 항목 | 확인 값 |
|---|---|
| 파일 수정 시각 | 2026-08-28 02:46:13 KST |
| 크기·인코딩 | 3,268 bytes, UTF-8 BOM 없음 |
| SHA-256 | `635fdbbcafe3b4b9abc455f206575091e79f6b0a6aa56f8993f656397842afcb` |
| schema / version | `lostark.valtan-boss-audition-flows` / `1` |
| flowId | `flow.valtan.boss-tool.default` |
| 슬롯 수 / 고유 slotId 수 | 29 / 29 |
| 고유 patternId 수 | 28 |
| nextSlotOrdinal | 30 |
| interStepPursuitMs | 1000 |
| 미완료 Save 임시·복구 파일 | 발견하지 않음 |

이 SHA는 이번 검토 대상 식별용이다. 실행 완료 조건에 새 hash manifest나 resource pack 검증을 추가하는
계약이 아니다. 이후 사용자가 다시 Save하면 당시 파일을 다시 검토하고 현재 값을 강제 복원하지 않는다.

## 2. HEAD 대비 바뀐 내용

기준 HEAD: `d77d7e021edefc435119b23f9aab53315e3f870a`.

| 변경 | 실제 저장 결과 |
|---|---|
| 새 슬롯 추가 | `.slot.000029 / VALTAN_FLOOR_WIPE_130`을 첫 번째 배열 원소로 추가 |
| 카운터 이동 | 기존 `.slot.000026 / VALTAN_COUNTER`를 `VALTAN_SEQUENCE_FOUR` 다음인 14번째로 이동 |
| 다음 ID 발급 번호 | `nextSlotOrdinal: 29 → 30` |

원래 `.slot.000005 / VALTAN_FLOOR_WIPE_130`은 여섯 번째에 남아 있다. 이 저장본은 같은 전멸 패턴을
**1번과 6번에서 두 번 실행**한다. 이것은 duplicate `patternId`를 허용하는 정상 Flow 계약이다.
중복을 자동 삭제하거나 사용자의 의도를 추정해 하나를 지우지 않는다.

## 3. 현재 전체 배열 순서

다음 표의 `순서`만 재생 순서다. `slot suffix`는 stable ID의 끝부분이다.

| 순서 | slot suffix | patternId |
|---:|---|---|
| 01 | `.000029` | `VALTAN_FLOOR_WIPE_130` |
| 02 | `.000001` | `VALTAN_WHIRLWIND` |
| 03 | `.000002` | `VALTAN_FOUR_SLASH` |
| 04 | `.000003` | `VALTAN_HIGH_JUMP` |
| 05 | `.000004` | `VALTAN_DASH_CHARGE` |
| 06 | `.000005` | `VALTAN_FLOOR_WIPE_130` |
| 07 | `.000006` | `VALTAN_ARENA_BREAK_109` |
| 08 | `.000007` | `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK` |
| 09 | `.000008` | `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK` |
| 10 | `.000009` | `VALTAN_SIX_PIZZA_106` |
| 11 | `.000010` | `VALTAN_ATTACK_WHIRLWIND` |
| 12 | `.000011` | `VALTAN_CHARGE` |
| 13 | `.000012` | `VALTAN_SEQUENCE_FOUR` |
| 14 | `.000026` | `VALTAN_COUNTER` |
| 15 | `.000013` | `VALTAN_ROAR_CHARGE` |
| 16 | `.000014` | `VALTAN_SEQUENCE_RUSH` |
| 17 | `.000015` | `VALTAN_THREE` |
| 18 | `.000016` | `VALTAN_TERRAIN_DESTRUCTION` |
| 19 | `.000017` | `VALTAN_SEQUENCE_FRONT_BACK_FRONT` |
| 20 | `.000018` | `VALTAN_WARP` |
| 21 | `.000019` | `VALTAN_SEQUENCE_TWOHAND` |
| 22 | `.000020` | `VALTAN_SEQUENCE_WHIRLWIND` |
| 23 | `.000021` | `VALTAN_TRASH` |
| 24 | `.000022` | `VALTAN_TRASH_CATCH_SUCCESS` |
| 25 | `.000023` | `VALTAN_TRASH_CATCH_FAIL` |
| 26 | `.000024` | `VALTAN_TRASH_CATCH_IF` |
| 27 | `.000025` | `VALTAN_CATCH_BREATH` |
| 28 | `.000027` | `VALTAN_CHARGE_2` |
| 29 | `.000028` | `VALTAN_STRUGGLING` |

## 4. 저장과 소비 경로의 근거

### 4.1 Client 문서

[ValtanPatternFlowDocument.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternFlowDocument.cpp:261)

- `Parse_Text`는 JSON 배열 순회 중 `stagedFlow.Slots.push_back()`으로 순서를 보존한다.
- `Validate`는 최대 255슬롯, unique slotId, admitted patternId, pursuit 범위와 단조 증가 발급 번호를
  검사한다. 같은 patternId를 여러 슬롯에서 참조하는 것은 허용한다.
- `Serialize_Text`는 `flow.Slots[slotIndex]` 순서로 쓴다.
- `Move_Slot`은 복사본에서 위치를 바꾼 뒤 전체 Flow를 다시 검증하고 성공할 때만 commit한다.
  stable slot ID와 `nextSlotOrdinal`은 다시 발급하지 않는다.
- `Save`는 baseline SHA 비교 → serialize/parse/validate → durable temp → backup → 교체 직전 SHA
  재확인 → `MoveFileExW` 교체 → 저장 bytes 재검증 → baseline/draft commit 순서다.
- Save 실패 시 기존 파일/draft를 보존하는 경로가 있고, 이번 검토에서 Save를 다시 호출하지 않았다.

### 4.2 전송 및 Server 재생

| 경계 | 실제 동작 |
|---|---|
| [BossTool.cpp:419](C:/Users/user/Desktop/LostArk/Client/Private/BossTool.cpp:419) `Start_Flow` | clean draft·inventory·disk revision 재확인; Start First는 `Slots.front().strSlotId` |
| [ValtanPatternFlowService.cpp:131](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternFlowService.cpp:131) | `Flow.Slots`를 동일 순서 wire vector로 복사 |
| [PacketMessages.cpp:3412](C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:3412) | encode/decode가 slots vector 순서 보존 |
| [GameRoom.cpp:4783](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:4783) | `startSlotId`를 실제 배열에서 찾아 시작 index 결정 |
| [GameRoom.cpp:4887](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:4887) | 그 index부터 `Sequence.PatternIds.push_back(request.Slots[index].strPatternId)` |
| [ValtanBrain.cpp:1139](C:/Users/user/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1139) | `PatternIds[boss.iRotationStepIndex]`를 차례로 소비 |

Save는 Debug Flow 문서만 바꾼다. 제품 `Valtan.gameplay.json`의 `scriptedSequence`를 수정하지 않으며,
이미 Server가 승인해서 실행 중인 Flow의 요청 snapshot도 바꾸지 않는다. 새 저장 순서는 다음 Start에서
제출된다. 재생 중인 단일 패턴 자체의 soft-lock은 저장 순서 반영 문제와 별개다.

## 5. Reload Flow의 현재 의미

[BossTool.cpp:502](C:/Users/user/Desktop/LostArk/Client/Private/BossTool.cpp:502)의 `Reload_FlowDocument`와
[ValtanPatternFlowDocument.cpp:505](C:/Users/user/Desktop/LostArk/Client/Private/ValtanPatternFlowDocument.cpp:505)의
`Reload`는 디스크 문서를 읽어 Client draft를 교체할 뿐이다.

- clean 상태: 저장된 문서로 reload한다.
- dirty 상태: `Discard & Reload / Keep Draft` 확인을 먼저 보여 준다.
- Discard & Reload: 저장하지 않은 슬롯 편집을 버리고 디스크를 다시 읽는다.
- Keep Draft: 현재 draft를 유지한다.
- 이전에 선택한 stable slotId가 있으면 유지하고, 없으면 첫 슬롯을 선택한다.
- 파싱/검증 실패면 기존 유효 상태를 보존하고 원인을 표시한다.
- **Server 명령을 보내지 않는다. 발탄 이동, 현재 패턴 교체, 맵 복구, 플레이어 이동도 하지 않는다.**

첨부 화면의 `Discard unsaved slot changes...`는 이러한 확인 상태다. 화면만으로 현재 디스크 bytes나
실행 중인 Server 상태를 판정하지 않았고, 위 결론은 실제 파일과 코드에서 확인했다.

## 6. 수정 전 검증과 발견한 테스트 문제

```powershell
python -B -m unittest -v Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract
```

결과: 17개 중 16 PASS / 1 FAIL.

수정 전 HEAD 기준 159행의 유일한 실패는
[test_valtan_boss_tool_pattern_flow_contract.py:159](C:/Users/user/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py:159)의
`test_default_document_is_strict_and_starts_from_all_effects_inventory`다. 바로 앞의
`validate_document(self.flow, self.inventory)`는 통과한다. 뒤의 assertion이 사용자의 editable 저장본을
초기 28개 inventory 순서·28슬롯·`nextSlotOrdinal=29`와 같아야 한다고 고정하고 있어 실패한다.

추가 회귀:

```powershell
python -B -m unittest Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract Tools.ValtanPipeline.test_valtan_boss_tool_contract
git diff --check -- Data/Encounters/Valtan/ValtanBossAuditionFlows.json
```

결과: 추가 51 tests PASS, 대상 JSON diff check PASS. 합계 68 tests 중 67 PASS / 위 1 FAIL.
별도의 현재 JSON strict validation도 PASS:

```text
slots=29 unique_slot_ids=29 unique_pattern_ids=28
inventory=28 nextSlotOrdinal=30 pursuit_ms=1000
all_saved_pattern_ids_admitted=True
```

이 문제는 아래 §8의 별도 요청에서 실제 수정했다. 현재 authoring 문서의 schema/admission 검증과
in-memory fixture의 초기 seed 검증을 분리했다. 사용자 JSON을 초기값으로 되돌리거나 테스트를 현재
29개 순서에 다시 고정하지 않았다.

독립 C++ Flow document 실행형 harness는 현 Tools에서 확인하지 못했다. 위 C++ 소비 경로는 source audit,
현재 JSON은 기존 focused validator 검증이다. NetworkProtocolHarness와 Server/Client 새 빌드는 이번
문서 작업에서 실행하지 않았다. 사용자 UI 재생·visual PASS는 기록하지 않는다.

## 7. 다음 구현자가 보존할 경계

1. 이 파일의 전체 현재 배열·stable slotId·`nextSlotOrdinal=30`을 보존한다.
2. Next Pattern은 별도의 runtime one-slot 예약이다. 이 JSON에 next 필드나 슬롯을 자동 추가하지 않는다.
3. Trash 내부 분기 수정 때문에 `.slot.000022~000024`를 삭제하지 않는다.
4. 독립 도넛을 Next 목록에 넣어도 기존 Flow admission 28개를 임의 확장하거나 saved slots를 재생성하지 않는다.
5. 후속 Save가 발생하면 사용자 최신본을 우선한다. 이 결과 문서의 29개 배열을 강제 정본으로 삼지 않는다.

## 8. 2026-08-28 추가 요청: pull 전 테스트 수정

사용자가 Trash 작업을 보류하고 통합 세션에서 pull/merge할 때 editable Flow 때문에 검증이 실패하지
않도록 실제 수정을 요청했다. 이 절은 문서상 계획이 아니라 실제 완료한 변경과 검증이다.

### 8.1 수정한 코드

[test_valtan_boss_tool_pattern_flow_contract.py](C:/Users/user/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py:59)

- `make_document(pattern_ids)`가 독립 in-memory fixture를 만든다. 파일을 읽거나 쓰지 않는다.
- 실제 저장본 테스트는 `validate_document(self.flow, self.inventory)`만 수행한다.
- 초기 seed 28개 순서/next ordinal은 별도 fixture에서 검증한다.
- 기존 중복/invalid 입력 테스트가 `self.flow`를 복사하지 않도록 바꿨다.
- 빈 저장본에서는 `slots[0]/[-1]` 접근이 실패하고, 한 슬롯에서는 duplicate를 실제 생성하지 못하던
  추가 테스트 결함을 함께 제거했다.
- reorder + 새 duplicate 슬롯, 0/1/32슬롯, sparse 삭제·비기본 pursuit·JSON roundtrip을 보강했다.
- `.slot.999999 / nextSlotOrdinal=1000000`은 저장 가능하고, 재사용/상한 초과는 거절한다.
- overflow는 정확히 33개 unique slotId로 만들고 `slots` 사유를 확인한다.
- 기존 strict validator와 C++/wire/JSON 계약은 변경하지 않았다.

JSON roundtrip fixture는 Python codec/validator 회귀다. C++ Save UI를 실제 실행한 증거로 사용하지 않는다.

### 8.2 실행 결과

```powershell
python -B -m unittest -v Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract Tools.ValtanPipeline.test_valtan_boss_tool_contract
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
git diff --check
```

| 검증 | 실제 결과 |
|---|---|
| Flow focused suite | 22 PASS |
| PatternTree + All Effects + Boss Tool | 51 PASS |
| 합계 | 73 PASS / failures 0 / errors 0 |
| Project-ValtanPatternMaster ValidateV2 | PASS, errors `[]` |
| 전체 working diff whitespace check | PASS; 기존 파일 CRLF 변환 warning만 존재 |
| 별도 독립 리뷰 | P1/P2 추가 지적 없음; Flow 22개 재실행 PASS |
| 사용자 Flow JSON | SHA-256 변경 전후 동일 |

추가로 실제 파일을 쓰지 않고 `unittest.mock.patch`로 suite의 `FLOW_JSON.read_text` 입력만 교체하여
다음 각 입력에서 **전체 22개 테스트**를 다시 실행했다.

| 메모리 입력 | 결과 |
|---|---|
| 초기 seed 28개 | 22 PASS |
| 현재 저장본 29개 | 22 PASS |
| 슬롯 0개, next ordinal 30 보존 | 22 PASS |
| 슬롯 1개 | 22 PASS |
| 같은 pattern을 참조하는 unique 32슬롯 | 22 PASS |
| sparse ID·역순·pursuit 2500ms | 22 PASS |
| 마지막 ID 999999, next ordinal 1000000 | 22 PASS |

이는 154개의 새로운 고유 테스트가 아니라 동일 22개 suite를 7가지 저장 상태에서 재실행한 결과다.
게임 C++·리소스 변경이 없으며 새 Engine/Shared/Server/Client 빌드와 사용자 UI smoke는 실행하지 않았다.

### 8.3 통합 세션과 Git 경계

통합 담당 작업은 `Review PR 247 merge readiness`, 작업 경로는 `C:/w/p247`, 브랜치는
`codex/valtan-party-integrated-review`다. 그 세션이 PR247/249 및 원본 diff를 통합하며 protocol은 41이다.
원본 checkout은 protocol 39이므로 이 테스트 파일 전체를 그대로 덮어쓰면 통합본 guard가 퇴행한다.
**helper와 fixture/test 메서드 수정만 통합하고 기존 protocol 41 guard와 카메라·97-wall 변경을 보존한다.**

원본에서 실행한 fetch 기준 HEAD는 `d77d7e021edefc435119b23f9aab53315e3f870a`, origin/main은
`0a08b0842afef5975569ea792898310cdb02d305`다. main이 5커밋 앞서고 일부 incoming 파일이 현재 dirty 파일과
겹쳐 원본 폴더에서 pull/stash/reset/commit/push를 자동 실행하지 않았다. 통합 세션에 최종 파일과 검증을 전달한다.
통합본에서의 최종 회귀·PR·merge·이후 pull 성공은 그 세션의 결과로 별도 확인해야 한다.

Trash는 사용자 요청으로 보류했다. Next/Trash runtime 구현을 이 검증 수정에 포함하지 않았다.

후속 입구: [작업 상태·통합 인계](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_NEXT_PATTERN_TRASH_WIPE_POST_MERGE_HANDOFF.md).
