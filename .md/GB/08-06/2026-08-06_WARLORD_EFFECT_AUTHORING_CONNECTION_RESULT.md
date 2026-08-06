# 2026-08-06 Warlord Effect Authoring Connection Result

## 결과 요약

워로드 Effect 리소스 추출 결과를 Effect Tool이 읽는 unbound reference로 전달했다.
680개 source ParticleSystem의 runtime Mesh/Texture 후보는 모두 해석됐고 누락 또는 모호한
후보는 0개다.

워로드 Q/W/E skill 연결은 완료 처리하지 않았다. 현재 animation source의 371개
AnimSequence에는 `Notifies` property가 하나도 없고, Warlord `skillbindings`, `animnotify`,
`animevents`도 없기 때문이다. source package/object 이름으로 skill ownership을 추측하지
않았다.

## 생성 및 갱신 파일

- `Data/Effects/Imported/Warlord/Warlord.unbound-effect-draft-index.json`
  - source system 680
  - candidate element partition 1,228
  - missing/ambiguous runtime resource 0
  - skill-bound source system 0
- `Data/Effects/Imported/Warlord/README.md`
  - 전수 추출 및 cook 완료 상태 반영
  - DecalMaterial 2개와 engine fallback 경계 반영
  - 최종 skill 연결 입력과 금지 경계 명시
- `Tools/LevelPlacementExtractor/test_build_unbound_effect_draft_index.py`
  - runtime resource exact resolution 검증
  - source 이름으로 skill을 배정하지 않는 fail-closed 검증
  - missing resource를 fallback하지 않고 집계하는 실패 검증

## 실측 증거

외부 extraction receipt:

- resource export: 761 requested / 761 exported / 0 missing
- runtime cook: Mesh 143 / Texture 618 / failure 0
- material recovery: 34 requested / 32 resolved MaterialInstance / 2 unsupported
  DecalMaterial / failure 0
- Warlord animation discovery: AnimSequence 371 / Notifies property 0

생성 index:

```text
sourceSystemCount=680
candidateElementPartitionCount=1228
missingOrAmbiguousRuntimeResourceCount=0
skillBoundSourceSystemCount=0
```

## 자동 검증

PASS:

- `test_build_unbound_effect_draft_index.py`: 2 tests
- `test_build_unbound_particle_resource_catalog.py`: 1 test
- `test_build_imported_effect_drafts.py`: 1 test
- generated JSON parse 및 680/0/0 invariant
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`: 69 checks
- `git diff --check`

## 미완료 경계

Effect Tool의 최종 `Q | skill name` 트리는 아직 Warlord에 대해 만들 수 없다. 다음 세
입력이 먼저 필요하다.

1. 승인된 Warlord stable skillId/inputSlot/displayName row
2. 실제 model clip을 사용하는 `Warlord.skillbindings.json`
3. clip-local ParticleSystem cue가 있는 `Warlord.animnotify` 또는 승인된 `.animevents`

세 입력이 들어오면 기존 class 공용 `build_class_skill_effect_inventory.py`와
`build_skill_effect_source_receipt.py` 경로를 그대로 사용한다. Warlord를 playable runtime
roster에 추가하는 작업은 별도 vertical slice이며 이번 결과에 포함하지 않았다.
