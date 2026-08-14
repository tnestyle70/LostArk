# 2026-08-14 4캐릭터 101/101·발탄 휠윈드 authored Effect 구현 계획

## 1. 이번 작업의 고정 목표

Artist F에서 사용자가 승인한 단일 authored Effect 흐름을 Artist, DimensionMaster,
LanceMaster, Warlord의 현재 combat intake 전체로 확장한다.

이번 작업의 첫 번째 물리 완료 분모는 다음과 같다.

- 4직업 combat intake: `51 skills / 74 stages / 113 clip occurrences`
- visual occurrence: `102`
- intentional silent 또는 no-carrier occurrence: `11`
- 4직업 고유 `.unified` authored 후보: `101/101`
- 별도 승인·제품 연결된 Artist F `.unified`: `1`
- 작업 종료 시 물리 `.unified` 목표: `102`

현재 4직업 후보는 `14/101`이다. Track A seam 13개는 v13이고 기존
`effect.dimensionmaster.skill.2050500.unified` 한 개는 v12다. 나머지 87개를 생성하고 기존
한 개를 의미 보존 v13으로 승격해 101/101을 만든다.

발탄은 `VALTAN_WHIRLWIND` action `420633`의 active stage를 같은 authored v13,
ordinary `CEffectObject::Stage_Document` 경로로 구현한다. 이번 휠윈드의 실행 완료 분모는
WWind 3 carriers와 Dust 2 carriers다. Ribbon/AnimationTrail 3과 Light 1은 이번 완료 분모에서
명시적으로 제외한다.

## 2. 사용자가 확정한 제품 방향

```text
Legacy JSON / Track A / 원본 추출 evidence
              ↓ 한 번 import 또는 의미 보존 migration
새 authored v13 .effect.json
              ↓ Effect Tool 편집·저장
ordinary CEffectObject::Stage_Document
              ↓ 사용자 visual 승인
catalog + animation/boss-pattern mapping 전환
```

- 정상 UI의 편집·선택 대상은 `.unified` authored Effect다.
- Legacy와 Track A는 Advanced Migration Reference와 rollback 근거다.
- 동일 Effect에 두 renderer나 두 runtime authority를 운영하지 않는다.
- Track A가 있는 row는 source identity와 실행 의미를 authored 데이터로 이식한다.
- Track A가 없는 row는 현재 Legacy 시각을 보존한 editable starter로 v13 승격한다.
- Legacy starter를 Track A exact 복원이라고 부르지 않는다.
- 제품 mapping은 candidate 생성과 분리한다. 사용자가 승인한 occurrence만 전환한다.
- canonical/SHA/receipt/harness 확대는 기능 목표가 아니다. 기존 사용자 파일 보호, 원자 rollback,
  필수 build처럼 저장소 계약에 필요한 최소 검증만 유지한다.

## 3. 이번 범위와 제외 범위

### 3.1 이번 구현 범위

- 현재 combat intake 51 skills/74 stages의 101개 `.unified` 후보
- 기존 executable Track A 12 BA stage의 authored materialization
- Warlord 17000 BA1의 editable/fail-closed canary 보존
- Track A packet이 없는 61 stages/88 candidate documents의 Legacy starter v13 승격
- Artist F에서 증명한 ordinary authored Particle/Mesh/Sprite 재생 경로 재사용
- 지원되는 Decal/LocalDecal의 DDS, Visible, Transform, Detail 편집 경로 보존
- Valtan Whirlwind WWind 3 + Dust 2 carriers
- 101개 후보를 class/skill/clip과 연결하는 Effect Tool Model View 동기화
- visual 승인 뒤 사용할 catalog+animevent 원자 승격 경계

### 3.2 이번 구현에서 제외

- Cascade Ribbon source graph portable compiler
- AnimationTrail history runtime과 live weapon trail
- Ribbon/Trail의 native-exact 복원 및 제품 완료 판정
- Light 15, ScreenPost 2와 Valtan Light 1
- PawnMaterialParam, ViewShake와 unresolved generic Effect notify
- PlayerSkills 전체 67개 중 현재 combat intake 밖의 이동기·스탠스 16개
- MeshParticle GPU instancing, Trail incremental upload, thumbnail background decode 등 새 성능 작업
- IOCP transport 교체

Ribbon/Trail schema와 기존 slot/data는 제거하지 않는다. 기존 authored Ribbon/Trail element와 DDS
binding이 있으면 Effect Tool에서 inspection과 수동 편집이 가능하도록 보존하되, 자동 source import,
live history와 제품 fidelity 완료 분모에는 포함하지 않는다. Light도 source evidence와 fail-closed 상태를
보존하지만 이번 candidate/product 완료를 막는 필수 family로 계산하지 않는다.

## 4. 완료 상태의 다섯 gate

`복원`이라는 한 단어로 다음 상태를 합치지 않는다.

1. **DOCUMENT_READY**: v13 `.unified` 문서가 존재하고 ordinary codec/Stage_Document가 읽는다.
2. **SOURCE_ADMITTED**: Track A source recipe/material/family/attachment 의미가 실행 가능하다.
3. **VISUAL_APPROVED**: 사용자가 실제 clip occurrence를 확인해 서면 승인했다.
4. **PRODUCT_MAPPED**: catalog와 animevent 또는 boss-pattern mapping이 새 asset을 가리킨다.
5. **PERFORMANCE_APPROVED**: 정한 4클라이언트 환경에서 Client 60 FPS와 Server 30 Hz를 확인했다.

101/101은 첫 번째 `DOCUMENT_READY` 목표다. Track A가 없는 88개는
`LEGACY_TUNING_STARTER` 상태로 시작한다. Track A batch 79 element plans는 현재 material admitted 16,
fail-closed 63, typed execution 0이므로 candidate 파일 존재만으로 `SOURCE_ADMITTED`를 주장하지 않는다.

## 5. G01 101/101 candidate materialization

### 대상 파일

- `Tools/EffectPipeline/build_track_a_authored_import_batch.py`
- `Tools/EffectPipeline/Schemas/lostark.effect-authored-import-batch.schema.json`
- `Data/Effects/AuthoredCorrections/Generated/FourClassCombat.track-a-authored-import-batch.json`
- `Tools/ClientFrontendHarness/Private/FourClassTrackAAuthoredMaterializer.h`
- `Tools/ClientFrontendHarness/Private/FourClassTrackAAuthoredMaterializer.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`

### 구현 계약

- 입력 batch는 13 Track A candidates와 88 Legacy starter candidates를 정확히 한 번 소유한다.
- 모든 target은 기존 제품 ID가 아니라 정규화된 `.unified` ID/path를 사용한다.
- 현재 13개 v13 후보는 사용자 편집과 element identity를 보존한다.
- 기존 DimensionMaster 2050500 v12 후보는 effect 의미와 element payload를 보존하고 version만 v13으로
  승격한다.
- 나머지 87개는 Legacy 문서의 Mesh/Sprite, DDS, Visible, Transform, Detail을 보존해 생성한다.
- 101개 전체를 memory stage한 뒤에만 candidate 파일을 한 transaction으로 commit한다.
- 실패하면 기존 14개와 Legacy product 문서를 유지하고 새 파일의 부분 생성물을 남기지 않는다.
- EffectCatalog와 animevent mutation은 이 G에서 0이다.

### 현재 중단 지점과 재개 순서

중단 직전 materializer의 Python/C++ JSON 소수 표현, intentional-silent enum, `Data/Effects`와
catalog authoring path 도메인, transaction load order를 교정했다. 마지막 Debug build와 실데이터
dry-run은 사용자가 실수로 중단했으므로 현재 결과를 PASS로 승계하지 않는다.

재개 순서는 다음으로 고정한다.

1. 최신 materializer Debug compile/link
2. transaction rollback fast
3. 101개 실데이터 dry-run
4. 명시 candidate write로 87개 생성과 v12 한 개 승격
5. 101개 v13/asset ID/element count 재확인
6. 재실행 시 기존 후보의 사용자 변경을 덮어쓰지 않는지 확인

## 6. G02 Track A와 Legacy starter의 실질 authored Effect

### Track A가 있는 13 stage

- Artist 31000 BA1~BA4
- DimensionMaster 2050010 BA1~BA4
- LanceMaster 34010 BA1~BA4
- Warlord 17000 BA1 canary

`CEffectDocumentCodec::Build_GenericAuthoredElementImportStage` 하나를 사용한다. F 전용 Core33,
Y -90°, `modelPreScale=0.01`, ArtistVisualV4 opcode를 다른 직업의 기본값으로 복사하지 않는다.

각 row는 다음 상태 중 하나를 명시한다.

- `ADMITTED_SOURCE_PROFILE`
- `TYPED_EXECUTION`
- `LEGACY_TUNING_STARTER`
- `FAIL_CLOSED`

현재 material admitted 16개는 authored `SourceRecipe`와 grouped-translucent material을 유지한다.
나머지 63개는 white/generic material로 가짜 복원하지 않는다. 기존 Legacy visual을 편집 starter로
보존할 수는 있지만 Track A source 의미와 분리 표시한다.

### Track A packet이 없는 61 stage

88개 Legacy starter 문서는 2,089 elements의 Mesh/Sprite 평탄화 시각을 그대로 v13으로 올린다.
DDS, transform, visibility, detail과 element stable ID를 보존해 사용자가 Effect Tool에서 직접
수정할 수 있게 한다.

이 G의 완료는 `DOCUMENT_READY`다. 원본 source graph를 새로 추출하지 않은 Legacy starter를
`SOURCE_ADMITTED` 또는 exact restoration으로 승격하지 않는다.

## 7. G03 Effect Tool 편집과 Legacy 격리

### 정상 UI

- 101개 `.unified` 후보를 class → skill → stage/clip 아래 표시한다.
- candidate를 열면 대응 character model과 실제 clip을 Model View에 연결한다.
- Play All, Family, Solo, Visible, Transform, DDS slot, Detail, Save/Reload를 사용한다.
- multi-clip 후보는 현재 product cue 순서를 유지한다.

### Advanced Migration Reference

- candidate sibling이 생긴 기존 Legacy는 정상 목록에서 제거한다.
- Legacy는 inspect/preview와 Save As만 허용한다.
- Save Changes로 원본 Legacy를 덮어쓰지 못하게 UI와 backend 양쪽에서 차단한다.
- F old reconstructed entry도 Advanced/rollback reference에서 접근 가능한 명시 row로 연결한다.
- Warlord 17820 clip3/4/8 orphan은 자동 product mapping에서 제외한다.

### Ribbon/Trail과 Light

- Ribbon/Trail element와 texture slot type은 삭제하지 않는다.
- 기존 DDS binding과 Detail/Visible/Transform 값은 round-trip에서 보존한다.
- 이번 작업은 source Ribbon compiler, live history와 Light runtime admission을 추가하지 않는다.
- 미지원 family는 다른 family의 Play All을 실패시키지 않고 해당 family만 deferred/suppressed로 표시한다.

## 8. G04 Valtan Whirlwind authored Effect

### 정본

- gameplay: `Data/Encounters/Valtan/ValtanEncounter.json`
- animation: `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- visual mapping: `Data/Animation/Authored/Valtan/Valtan.patterneffects.json`
- effect: `Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json`

### 이번 실행 분모

- notify-006 WWind: Sprite 1 + Mesh 2, 현재 executable 3 유지
- notify-005 Dust: 2 carriers를 authored 실행 상태로 연다
- notify-004 AnimationTrail 3: deferred
- notify-009 Light 1: deferred

Dust는 generic 흰 sprite로 낮추지 않는다. exact DDS와 SubUV를 연결하고, DynamicParameter/material
packet을 ordinary authored carrier가 소비하도록 닫는다. 필요한 aura texture가 실제 resource에 없으면
사용자가 Effect Tool의 DDS slot에서 교체할 수 있는 명시 slot과 blocker를 남기고 다른 WWind carrier를
실패시키지 않는다.

Model View는 `Boss_Valtan`, action `420633`, active clip `mesh_att_battle_20_03`, 실제
`b_effectroot` bone을 사용한다. Server의 `serverTick/actionStartTick/patternSequence/stageIndex`로 계산한
authoritative age를 animation과 effect가 함께 소비한다.

이번 Valtan 완료 상태는 `WWind 3 + Dust 2 executable`, Trail/Light deferred, gameplay damage와
duration은 `ValtanEncounter.json` 권위 유지다. 제품 mapping은 사용자 visual 승인 뒤에만 연다.

## 9. G05 사용자 visual 승인과 제품 전환

제품 cue 정본은 clip occurrence다. 따라서 전체 완료 판정은 102 visual occurrences를 기준으로 한다.

각 occurrence에서 사용자가 다음을 확인한다.

1. 대응 character 또는 Valtan model과 clip
2. Play All
3. Family
4. 필요한 occurrence Solo
5. DDS/Visible/Transform/Detail Save와 Reload
6. 실제 gameplay anchor, timing, follow/snapshot 결과

승인 전에는 기존 gameplay mapping을 유지한다. 승인된 occurrence만 다음 transaction으로 전환한다.

```text
candidate catalog row stage
→ animevent/boss-pattern effect asset 교체 stage
→ 모든 reference와 authoring document validate
→ catalog + mapping 단일 commit
→ 실패 시 기존 Legacy product mapping 유지
```

활성 4직업 animevent target은 현재 98개이며 Warlord orphan 3개는 별도 사용자 결정 전 제외한다.
전환 뒤 Legacy는 Advanced read-only rollback reference로 유지한다. zero-reference와 관찰 기간 전에는
파일을 삭제하지 않는다.

## 10. G06 성능 검증은 구현 뒤 수행

이번 101/101과 Valtan Whirlwind 구현 중에는 새 최적화와 IOCP 교체로 범위를 넓히지 않는다. 이미 들어간
Effect Tool cache/debounce, prepared handle/buffer reuse, aggregate effect budget과 Server bounded
ingress/outbound는 보존한다.

Effect 구현과 사용자 visual 승인이 끝난 뒤 두 topology를 모두 검증한다.

### 로컬 검증

- Server 1 + Client 4를 같은 PC에서 실행
- 해상도 `1280 × 720`
- Client 목표 `60 FPS`
- Server 목표 `30 Hz`

### LAN 플레이 검증

- server-host 한 대와 다른 팀원의 Client 참여
- 같은 gameplay/effect revision
- 4명 동시 스킬과 Valtan Whirlwind
- disconnect/reconnect와 slow reader가 다른 player를 막지 않음

측정 항목은 Client frame p95/p99와 최대 non-load hitch, Server room tick p95/p99, ingress/outbound
depth와 drop/coalesce다. IOCP는 성능 때문에 검토한 후속 구현이며, 위 두 topology의 계측이 현재
thread+bounded queue 구조로 목표를 못 맞출 때 별도 수직 슬라이스로 진행한다.

## 11. 최소 자동 검증과 수동 경계

canonical/SHA receipt 자체를 작업 목표로 확대하지 않는다. 다음은 사용자 파일과 제품 mapping을
안전하게 지키기 위한 최소 검증이다.

- JSON/schema parse
- 101 target count, v13과 effectAssetId/path 일치
- materializer rollback과 부분 파일 없음
- ClientFrontendHarness focused materializer/ordinary Stage_Document 실행
- Client Debug/Release compile/link
- Effect publisher Validate
- `git diff --check`

에이전트는 Client/UI를 자율 실행하거나 visual fidelity를 대신 판정하지 않는다. 102 occurrence와
Valtan Whirlwind의 visual PASS는 사용자의 서면 관찰만 기록한다.

## 12. 완료 정의

이번 작업은 다음 조건이 모두 충족될 때 종료한다.

- 4직업 `.unified` 후보 `101/101`, 전부 authored v13
- 별도 Artist F를 포함한 물리 `.unified` `102`
- 정상 UI는 새 candidate를 표시하고 sibling Legacy는 Advanced read-only로 격리
- Track A admitted, Legacy starter, fail-closed 상태를 명확히 구분
- Ribbon/Trail과 Light data/slot을 훼손하지 않고 deferred 상태로 유지
- Valtan Whirlwind WWind 3 + Dust 2 executable
- 사용자가 승인한 occurrence만 product mapping 전환
- 기존 Legacy product와 rollback reference는 승인·zero-reference 전까지 보존
- 기능 구현 뒤 로컬 Server 1 + Client 4와 LAN 플레이를 1280×720, Client 60 FPS,
  Server 30 Hz 목표로 검증
- IOCP는 성능 계측이 요구할 때만 후속 수직 슬라이스로 진행
