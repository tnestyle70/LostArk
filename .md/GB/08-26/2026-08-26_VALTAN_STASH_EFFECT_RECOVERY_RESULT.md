# 발탄 stash Effect 선택 복구 결과

## 1. 원인과 보존 상태

파일이 삭제된 것이 아니라 pull 직전 safety stash에 보존된 뒤 재적용되지 않은 것이
원인이었다. 고정 stash `7ec4d3b6`와 보호 브랜치
`codex/recovered-valtan-wip-20260826`은 그대로 유지했다.

## 2. 복구한 저작 데이터

- 도넛 INNER/OUTER carrier: stash blob `6e690abd`
- 4연격 active: `868480b1`
- HIGH_JUMP TAKEOFF: `1dc70cb6`
- 회오리 recovery: `c1f209a9`
- 하늘 도끼 active: `0c9e1836`
- FLOOR_WIPE 빈 전체 패턴 Effect: `dc5bc274`
- HIGH_JUMP 중앙 착지 수동 Element Effect: `d7dfc0e5`

HIGH_JUMP LAND와 `effect.valtan.pattern.420633.active`는 HEAD와 stash가 이미 같아
수정하지 않았다.

## 3. 복구한 연결과 버그 수정

- FLOOR_WIPE source presentation과 generated Product가 다음 순서를 사용한다.

  `mesh_att_battle_5_02_loop -> mesh_att_battle_5_02_end -> mesh_att_battle_5_02_loop -> mesh_att_battle_5_02_end`

- WINDUP의 stable cue는 새 `effect.valtan.floor-wipe-130` 빈 문서를 가리킨다.
- ARENA_BREAK_109 WIDE_REVEAL은 착지 tail 뒤
  `mesh_evt1_att_battle_5_01_start -> mesh_evt1_att_battle_5_01_loop` 포효를
  재생하며, 130줄 debug audition의 잘못된 LEDGE_ROAR 연쇄를 제거했다.
- split authoring parser가 ENTER 단수형 `SPAWN_COMBAT_OBJECT`를 fail-closed로
  admission한다.
- `presentationScale=0.75`, OWNER/WORLD/ARENA cue policy, nullable
  `sourceEndMs`와 Valtan index 격리는 현재 main에 이미 있어 중복 적용하지 않았다.

HIGH_JUMP 중앙 착지 Effect는 All Effects에서 독립 저작할 수 있도록 Catalog와
Client Data project에 등록했다. 현재 main의 Server pattern에는 아직 owner로
연결하지 않았으므로 실제 HIGH_JUMP 전체 패턴 spawn은 후속 수직 슬라이스다.

## 4. 검증 상태

- 고정 stash blob 7개 일치: PASS
- JSON parse와 FLOOR_WIPE stash semantic equality: PASS
- `Project-ValtanPatternMaster.ps1 -Mode PublishV2/ValidateV2`: PASS
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS
- Effect Data project sync/check: PASS
- Valtan v2 transaction harness: 37/37 PASS
- pattern tree: 16/16 PASS
- animation tool: 8/8 PASS

전체 Effect source validator는 이번 변경과 무관하게 Git 비추적 Warlord texture
3개를 발견해 fail-closed했다. Effect Tool saved-row 회귀의 남은 optional emissive
1건도 기존 Warlord fixture 불일치이며, 이번 Valtan 복구 assertion은 통과했다.
Client `ClCompile` 전체 target은 사용자 빠른 인계 요청에 맞춰 중간에 중단했으므로
완료 PASS로 기록하지 않는다.

Client 화면의 색상·타이밍·크기 visual PASS는 사용자가 직접 판정한다.

## 5. 2페이즈 애니메이션 작업자 handoff

`Data/Valtan/Valtan.presentation.debug.json`은 commit `fc5e5e34`부터 현재 HEAD에
이미 포함돼 있으며, Animation Tool의 `Valtan Custom Chain`에서 20개 체인을
로드·미리보기할 수 있다. 따라서 이펙트 저작자가 애니메이션 순서를 눈으로 맞추는
참고 소스로는 즉시 사용할 수 있다.

다만 이 문서는 Product split source가 아니라 debug handoff library다. 현재 20개
체인의 `targetPatternId`와 `targetStageId`가 모두 비어 있고, 94개 occurrence 중
71개가 native clip 길이를 사용하도록 `playMs=0`이다. effect cue, action ID와
Server stage wall-clock도 소유하지 않으므로 publisher가
`Valtan.presentation.json`으로 자동 병합하지 않는다. 실제 패턴에 연결할 때는
애니메이션 담당자가 target pattern/stage와 확정 playMs를 지정한 체인부터 해당
`Valtan.gameplay.json` stage에 맞춰 선택 승격해야 한다. 이 복구 변경에서는
임의 pattern/stage 매핑을 만들지 않았다.
