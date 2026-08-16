# 2026-08-16 발탄 애니메이션·패턴·수치 원본 검증 감사 결과

이 문서는 `발탄_애니메이션_수치데이터_원본검증_복원_Claude_작업지시서.txt`의 G1~G7 실행 결과다.
G1/G2/G4/G5 감사, **G3 AnimSet 복원**, 사용자 관찰 기록 기반 **G7 표 G**까지 완료했다.
G6 원작 피해량은 실측 환경이 없어 전부 UNKNOWN으로 남겼다. commit/push는 하지 않았다.

읽는 순서에 주의한다. 3~5절은 **복원 이전** 상태의 감사 기록이며 그 시점의 진단을 보존한다.
복원 후 최종 상태는 12절, occurrence 매핑은 13절이 정본이다.

감사 스크립트와 산출 CSV는 세션 scratchpad에 있으며 저장소에 커밋하지 않았다.

```text
valtan_audit.py                  표 A/B/D/E 생성기
psk_psa_probe.py                 PSK/PSA 청크 실측 리더
tableA_action_clip_evidence.csv  121 stage 증거표
tableB_clip_existence.csv        clip 물리 존재표
tableD_hit_timeline.csv          HIT notify 타임라인
tableE_hit_shape.csv             판정 형상 대조표
```

## 1. 한 줄 결론

121개 pattern binding은 원래 **논리적으로 121/121 EXACT**인데 **물리적으로 0/121**이었고,
누락된 146클립 AnimSet을 원본 PSK/PSA에서 정식 경로로 복원해 **물리 121/121**로 닫았다.
현재 cooker로는 불가능했던 세 가지 차단 사유는 오프라인 Tool의 opt-in 옵션으로 해결했으며,
그 과정에서 지시서가 예측하지 못한 네 번째 차단(skeleton node 이름)도 실측으로 찾아 닫았다.

원작 피해량은 게임 빌드·난이도·방어력 조건을 확인할 수 없어 32개 전부 UNKNOWN으로 남겼고
`DamageProfiles.json`은 한 글자도 바꾸지 않았다.

## 2. 지시서 기준 수치와 현재 HEAD 실측의 차이

지시서 2절은 "차이가 있으면 현재 파일을 우선한다"고 했다. 실측 결과 다음 세 값이 다르다.

| 항목 | 지시서 | 현재 실측 | 근거 |
|---|---|---|---|
| animnotify 행 수 | 1326 | **1218** | 지시서가 함께 적은 종류별 합(408+405+261+75+61+8)도 1218이다 |
| AnimSet skeleton bone | 87 | **PSK/PSA는 84** | PSK `REFSKELT[84]`, 두 PSA `BONENAMES[84]` |
| attach 후 총 animation | 173 | **173** (27+146) | body 27 실측, PSA 132+14 실측 |

`87`은 틀린 값이 아니라 다른 계층의 값이었다. PSK/PSA는 84 bone이고, cook 결과 WModel의
skeleton node는 `RootNode` + Armature + PSK bone 84 + Mesh object = **87**이다. 12.2 참고.

종류별 notify 수는 지시서와 완전히 일치한다: `HIT=408, EFFECT=405, SOUND=261, STAGE=75, SHAKE=61, COUNTER=8`.

나머지 기준 수치는 전부 일치했다.

```text
ValtanEncounter   32 patterns / 121 stages / 121 unique stage actionId
sourceActionId    참조 68개, 고유 63개, skilltiming resolve 실패 0
patternbindings   121행, actionId 중복 0
skilltiming       format 3, 원본 Action 100개, shape row 474개
clipmap           108 clip
clipseq           265행
body model        MN_RPBF_01.wmodel, 내장 animation 27개
```

## 3. G1 — 기획자 한글 코멘트와 원본 Action 전수 조사 (표 A)

`Valtan.clipmap` 108행의 한글 이름은 전부 numeric `skill=` ID를 함께 가지고 있고,
`clipseq` 265행의 chain, `animnotify` 108 clip, `skilltiming` 100 action과 조인된다.

**121 stage의 논리 판정** — binding이 가리키는 clip의 clipmap owner action이
해당 pattern의 `sourceActionIds`에 실제로 들어 있는지로 판정했다.

```text
EXACT     121
APPROX      0
CONFLICT    0
MISSING     0
```

즉 이름이 비슷해서 연결된 것이 아니라, 121개 전부 numeric Action ID 근거로 연결돼 있다.

### 3.1 다만 `백업` 표기 clip 2개가 제품 패턴에 연결돼 있다

clipmap에서 `백업` / `사용안함` / `AI 조립용`으로 표시된 행은 7개이고, 그중 2개가 제품 stage에 물려 있다.

```text
mesh_att_battle_12_10  "레이드 발탄_초강력 내려찍기 콤보 종료 (군단장 스킬) - 백업"
    <- valtan.attack.red-blade-wave.active
mesh_att_battle_12_11  "레이드 발탄_초강력 내려찍기 콤보 종료 (군단장 스킬) - 백업"
    <- valtan.attack.red-blade-wave.recovery
```

두 clip의 owner action `420620`은 `VALTAN_RED_BLADE_WAVE`의 `sourceActionIds`에 실제로 들어 있으므로
ID 근거로는 EXACT다. 그러나 원본 표기가 `백업`이고 패턴 이름은 `RED_BLADE_WAVE`(붉은 검기)인데
한글 이름은 `초강력 내려찍기 콤보 종료`다. **의미 충돌 후보이므로 사용자 육안 확인 대상**으로 남긴다.
지시서 5절의 "`백업`은 제품 패턴에 자동 연결하지 않는다" 규칙에 걸리는 유일한 항목이다.

## 4. G2 — 실제 애니메이션 클립 존재 여부 전수 검사 (표 B)

```text
body   Client/Bin/Resources/Character/Valtan/MN_RPBF_01.wmodel   animations=27
AnimSet Client/Bin/Resources/Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel
       파일 없음. AnimSets 폴더 자체가 존재하지 않는다.
attach 후 사용 가능한 clip 이름 = 27개
```

body 27개 clip 이름은 `att_battle_19_01`, `idle_battle_1`처럼 **접두사가 없고**,
121개 binding과 108개 clipmap은 전부 `mesh_` 접두사를 쓴다. 따라서 이름이 하나도 겹치지 않는다.

```text
121 binding이 참조하는 고유 clip = 61개
그중 물리적으로 존재 = 0개
binding 행 기준 물리 누락 = 121 / 121
clipmap 108 clip 중 물리 누락 = 108 / 108
```

### 4.1 런타임에서 실제로 일어나는 일

`Client/Private/AnimationSkillBindingDocument.cpp:708`의 `Validate()`는
binding clip이 하나라도 모델에 없으면 **문서 전체를 거부**한다.

```text
Valtan.cpp:88  Load_PatternBindings()
  -> body model의 실제 animation 이름 27개를 availableClips로 수집
  -> CValtanPatternAnimationBindingDocument::Load(...)
  -> Validate()에서 61개 clip이 전부 미존재 -> false
  -> "pattern bindings rejected; catalog clips remain" 로그 후 return
  -> m_PatternClipByActionId 는 빈 상태로 남는다
```

결과적으로 지금 Debug 발탄은 `BossCatalog.presentationClips`의 generic 6개
(`idle/chase/patternWindup/patternActive/patternRecovery/dead`)만 재생한다.
**"binding 121/121 존재"와 "물리 clip 0/121"은 이렇게 분리해서 읽어야 한다.**

Release는 `ValtanPresentationAssetService.cpp:69`에서 AnimSet 파일이 없으면 `E_FAIL`로 fail-closed다.
즉 현재 리소스 상태로는 **Release 발탄이 아예 생성되지 않는다.**

## 5. G3 — 누락 AnimSet 원본 확인 결과

### 5.1 원본 3개 파일은 지시서 해시와 정확히 일치한다

```text
mn_rpbf_00_sk.psk        997156 bytes
  E5D6798C3B59CCF21A1DD62FF86FB1DD0D1956B50B0079DEC1AB6CB38620E97B   일치
mn_rpbf_00_ani.psa     32610976 bytes
  A9CAD8D5C521070DDB4C38E4A72550E886C4330C0B970CAE81A26FB4E74DD50D   일치
mn_rpbf_00_evt2_ani.psa 4403440 bytes
  596C86D8DF8B50BFBCA9F1D0C6265DF1A768400B5E7E7D17827C9D62B7B88E64   일치
```

### 5.2 clip 이름 계약이 원본으로 완전히 증명된다

PSA sequence 이름과 clipmap 이름을 대조한 결과가 이번 감사에서 가장 중요한 근거다.

```text
PSA sequence          132 + 14 = 146,  두 파일 사이 이름 충돌 0
clipmap 108 clip      전부 "mesh_" + PSA sequence 이름과 정확히 일치   108/108
121 binding의 61 clip 전부 "mesh_" + PSA sequence 이름과 정확히 일치    61/61
clipmap에 없는 PSA sequence 38개 (idle_normal_1, dead_1, respawn_1, evt2_finalfight_* 등)
```

즉 146클립 AnimSet을 `mesh_` 접두사로 cook하면 **121 binding이 전부 물리 resolve된다**는 것이
추정이 아니라 원본 이름 대조로 증명된다.

### 5.3 복원 착수 시점의 cooker 차단 사유 세 가지

아래는 Tool을 고치기 **전** 상태의 진단이다. 세 항목 모두 12절에서 opt-in 옵션으로 해결했다.

지시서 7절 5항의 "현재 cooker로 재현 가능한지 먼저 확인한다"에 대한 답은 **불가능**이다.
실제로 `Cook-ActorXWModel.ps1`을 임시 staging(`C:\LostArkValtanCook`)에서 실행해 확인했다.

**차단 1 — 단일 anim rate 강제 (실행으로 재현됨)**

```text
Tools/ActorXAssetCooker/build_actorx_fbx.py:384  configure_scene_frame_rate()
RuntimeError: All PSA sequences in one FBX must share one animation rate;
got 28.548388, 28.695652, 28.714287, 28.928572, 29.999998, 30.0, 30.000002
```

발탄 PSA는 clip마다 고유 rate를 가진다(실측 5종: 28.5484 / 28.6957 / 28.7143 / 28.9286 / 30.0).
이 검사는 하나의 FBX에 rate가 2개 이상이면 무조건 실패한다.

**차단 2 — PSA와 PSK의 bone 순서 완전 일치 강제**

```text
Tools/ActorXAssetCooker/build_actorx_fbx.py:618  validate_psa_skeleton()
  imported_bones != source_bones 이면 즉시 RuntimeError
```

Blender PSK import 결과를 직접 확인했다.

```text
Blender armature object = 'mn_rpbf_00_sk.ao', bones = 84, 순서 = PSK REFSKELT 순서
  index 18 = bip001-l-clavicle
mn_rpbf_00_ani.psa BONENAMES 순서
  index 18 = b_capeta_b_01
```

bone **집합은 84개로 완전히 동일**하고 누락도 0이지만, cape 8개 bone의 위치가 달라
84칸 중 45칸의 순서가 어긋난다. `mn_rpbf_00_evt2_ani.psa`는 PSK와 순서까지 완전히 일치한다.
따라서 132클립 쪽 PSA만 이 검사에서 실패한다.

**차단 3 — `mesh_` 접두사를 만들 수단이 없다**

WModel clip 이름은 Blender armature object 이름에서 온다(README의 `itr_02326_sk.ao_hit1_1` 사례).
현재 import 결과 object 이름이 `mn_rpbf_00_sk.ao`이므로 cook하면
`mn_rpbf_00_sk.ao_att_battle_1_01`이 되고 **`mesh_att_battle_1_01`이 되지 않는다.**
현재 cooker의 `-PrefixActionsWithSource`는 PSA **파일 이름** 접두사이지 이 값이 아니다.

### 5.4 참고 — rate 문제는 저장소에 이미 선례가 있다

```text
Tools/ModelAssetConverter/retime_wmodel_from_psa.py
  "The original DimensionMaster conversion preserved key times and frame spans
   but wrote 24 ticks/second into every WANM section. This tool validates the
   complete PSA/WModel clip order and duration contract before atomically
   replacing only the ticksPerSecond fields."
```

즉 `단일 rate로 bake -> PSA 원본 rate를 clip별로 되돌린다`는 경로가 이미 팀에서 검증됐다.
새 두 번째 런타임이나 새 조립 스크립트를 만들지 않고 이 선례를 그대로 쓸 수 있다.

## 6. G4 — 공격별 실제 타격 시각 (표 D)

`HIT` notify의 asset은 SkillEffect PrimaryKey이고, 이를 `skilltiming`의 `pks=` 목록으로 조인했다.

```text
현재 damaging stage (serverDamageProfileId 있음)      46
현재 server hit pulse (hitCount 합, 최소 1)           72
binding된 clip에 실제로 붙어 있는 HIT notify 행       481
그중 PK가 skilltiming shape로 resolve                474
t + shape group 중복 제거 후 논리 타격                215
tier 중복으로 분류한 행                               177
```

**원본 notify로 타격 시각이 확정되는 damaging stage = 31 / 46.**

나머지 15개는 bound clip에 HIT notify가 하나도 없다.

```text
VALTAN_BACKSTEP_ATTACK SWEEP              VALTAN_CENTER_GRAB_COUNTER_64 TARGET_EXPLOSION
VALTAN_CHARGE_GRAB_ROAR CHARGE            VALTAN_FLOOR_WIPE_130 SECOND_SMASH
VALTAN_GHOST_TRANSITION_15 FOUR_DIRECTIONS/INNER/OUTER
VALTAN_IMPRISON_ROAR ROAR                 VALTAN_JUMP_SPIN LAND
VALTAN_LEDGE_ROAR ROAR                    VALTAN_PARRY COUNTER_SLASH
VALTAN_RED_BLADE_WAVE PROJECTILE          VALTAN_TRIPLE_COUNTER FAIL_1/FAIL_2/FAIL_3
```

지시서 8절의 금지 규칙대로 HIT 행 개수를 그대로 hit count로 쓰지 않았고,
같은 t의 동일 shape tier 중복은 `isDuplicateTier`로 분리해서 표에 남겼다.
`215 논리 타격`과 `72 현재 server pulse`의 차이는 아직 **미해석**이며 숫자로 밀어 넣지 않았다.

## 7. G5 — 판정 크기와 모양 (표 E)

`skilltiming` shape 행은 원본 필드를 그대로 보존하고 있다.

```text
area  판정 형상 종류 (1=원, 2=사각/박스, 3=부채꼴)
ar    반지름          aa  각도        ah  높이
ax    전방 offset     arem 내부 반지름 (도넛)
rep / repms  반복 횟수와 간격        hittype  타격 종류
push / pushr / fz / fzin / fzout / counter  경직·넉백 원본 필드
```

```text
damaging stage 46개 중 원본 shape가 1개 이상 조인되는 것   25
원본 shape가 전혀 조인되지 않는 것                        21
```

**단위는 변환하지 않았다.** 원본은 cm 계열 정수(예: `ar=400`, `range=350`)이고
현재 Encounter는 metre 실수(예: `hitLength=8.0`)다. 변환식을 입증할 원본 근거가 없으므로
표 E에는 raw 값을 `SOURCE_EXACT`, metre 값을 `PROJECT_TUNED`로 **별도 열**에 두었고
`convertedDimensionsMetres`는 채우지 않았다.

예: `VALTAN_SWING`의 원본 shape는 `area=3 ar=400 aa=140`(부채꼴 반지름 400, 각 140도)인데
현재 Encounter는 `CONE hitLength=8.0 hitAngleDegrees=120`이다. 400 -> 8.0m 변환도,
140 -> 120도 축소도 원본 근거가 없는 프로젝트 조정값이다.

## 8. G6 / G7 — 확정 불가 항목

### 8.1 G6 원작 실제 피해량 — 전부 UNKNOWN

```text
Data/Balance/DamageProfiles.json  발탄 profile 32개, 전부 damageRatePercent만 가짐
provenance receipt 2026-08-05     발탄 damageRatePercent 32개 전부 basis=PROJECT_TUNED
                                  note="official source binding is not claimed"
```

현재 저장소는 이미 **정직하게** 기록돼 있다. 이 값들을 원작 실측값이라고 부르는 곳은 없다.

원작 피해량을 확정하려면 지시서 10절이 요구하는 게임 빌드/난이도/관문/직업/최대 HP/방어력/
감소 버프/실드/치명타/백어택 조건이 고정된 실측이 필요하다.
**이 세션에는 게임 빌드도 영상도 실측 환경도 없다.** 따라서 숫자를 만들지 않았고
`DamageProfiles.json`은 한 글자도 건드리지 않았다. 32개 전부 `UNKNOWN`(원작 기준)으로 보고한다.

또한 현재 Server 공식과 원작 공식이 같다는 근거가 없으므로,
설령 observedDamage를 얻어도 `damageRatePercent` 하나로 재현된다고 주장할 수 없다.

```text
raw     = max(1, attackPower * damageRatePercent / 100)
applied = max(1, raw * 100 / (100 + playerDefense))
```

### 8.2 G7 occurrence 1~67 — 사용자 관찰 기록으로 작성 완료

`.md/`와 `Data/` 전체에는 이 기록이 없었고, 2026-08-16에 사용자가 직접 관찰 목록을 제공했다.
결과는 `13. 표 G — occurrence 매핑`에 있다.

### 8.3 원본 cooldown이 아직 소비되지 않는다

`skilltiming`에는 encounter가 참조하는 action 중 48개가 `cd>0`을 가지고 있다.
그러나 `ValtanEncounter.json`의 pattern에는 cooldown 필드 자체가 없고
`selectionWeight / maximumConsecutiveUses / health bar` 게이트만 있다.
지시서 1절이 지적한 "쿨타임이 표시용으로만 남으면 AI가 원작과 다른 순서로 공격한다"는 상태가 실제로 맞다.
이것은 Encounter schema + publisher + `ValtanBrain` 선택 로직을 함께 여는 별도 수직 슬라이스다.

## 9. 승인이 필요한 단일 결정

G3 복원을 진행하려면 `Tools/ActorXAssetCooker/build_actorx_fbx.py`에
**opt-in 플래그 3개**를 추가해야 한다. 이것이 유일한 막힌 지점이다.

```text
1) 여러 anim rate 허용 + 단일 bake rate 지정
   -> 이후 기존 retime_wmodel_from_psa.py로 clip별 원본 rate 복원
2) PSA BONENAMES가 PSK bone의 순열일 때 이름 기준 remap 허용
   (집합·개수가 정확히 같을 때만. 하나라도 다르면 기존처럼 실패)
3) FBX export 전에 armature object 이름을 지정 (mesh_ 접두사 생성용)
```

성격과 위험도:

- 대상은 **오프라인 Tool 하나뿐**이다. Engine / Client / Server / Shared C++ 를 건드리지 않는다.
- 전부 **기본값 미변경 opt-in**이므로 기존 ITR_02326 / DimensionMaster cook 결과는 그대로다.
- 지시서 7절 4항이 금지한 "없는 `build_npc_animset.py`를 상상해서 만들기"를 피하는 유일한 방법이다.
  두 번째 조립 스크립트를 새로 만드는 것은 `AGENTS.md`의 "같은 역할의 두 번째 경로 금지"에 걸린다.
- 다만 **팀 공용 도구**이므로 다른 담당자의 cook에 영향이 갈 수 있어 사전 확인이 필요하다.

승인되면 이어서 실행할 검증은 다음과 같다.

```text
146 animations / duplicate 0 / skeleton 84·84 ALIGNED
validator PASS -> CModel::Attach_AnimationSet PASS -> attach 후 173 animations
121 binding 물리 resolve = 121/121 (미해결 0)
그 뒤에야 Client/Bin/Resources/Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel 배치
```

## 10. 이번 세션에서 수정한 저장소 파일

사용자 승인을 받아 오프라인 Tool 3개 파일과 문서를 수정했다.
Engine / Client / Server / Shared C++ 와 `Data` 는 한 줄도 바꾸지 않았다.

```text
Tools/ActorXAssetCooker/build_actorx_fbx.py        opt-in 옵션 4개 추가
Tools/ActorXAssetCooker/Cook-ActorXWModel.ps1      옵션 pass-through, scale key 검사 정밀화
Tools/ActorXAssetCooker/README.md                  새 옵션과 주의사항 문서화
Tools/ModelAssetConverter/retime_wmodel_from_psa.py  --psa 다중 지정 지원
Tools/ModelAssetConverter/test_retime_wmodel_from_psa.py  다중 PSA 계약 회귀 4건 추가
.md/GB/08-16/..._VALTAN_ANIMATION_NUMERIC_SOURCE_AUDIT_RESULT.md  이 문서
```

Git 추적 대상이 아닌 runtime 리소스 1개를 팀장 관리 물리 폴더에 배치했다.

```text
Client/Bin/Resources/Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel
```

임시 산출물은 저장소 밖에만 만들었다.

```text
C:\LostArkValtanSrc     원본 PSK/PSA 복사본 (해시 동일 확인)
C:\LostArkValtanCook    검증 통과한 staging 패키지와 retime receipt
```

## 11. 자동 검증 / 수동 미검증 분리

자동으로 확인한 것:

```text
JSON parse (ValtanEncounter, patternbindings, DamageProfiles, BossCatalog, receipt)
reference 4종 declared count 대 실제 행 수
sourceActionId 63개 전부 skilltiming resolve
HIT PK 481행 중 474행 skilltiming shape resolve
PSK/PSA SHA-256 3개 일치
PSK/PSA bone·sequence 실측, PSA 이름과 clipmap 108/108 대조
body WModel animation 27개 실측
Cook-ActorXWModel.ps1 실행으로 차단 사유 재현
```

사용자 육안 검증이 필요한 것(에이전트가 대신 판정하지 않음):

```text
mesh_att_battle_12_10 / _12_11 "백업" clip이 RED_BLADE_WAVE 동작과 맞는지
AnimSet 복원 후 121 패턴의 실제 모션이 한글 설명과 일치하는지
HIT 프레임이 실제 무기·지면 접촉 순간과 맞는지
reference collider wire가 공격 범위·방향과 맞는지
```

아직 시작하지 않은 것: 표 F(피해 실측), 표 G(occurrence 1~67).

## 12. G3 AnimSet 복원 실행 결과

### 12.1 cooker에 추가한 opt-in 옵션

전부 기본값 미지정이면 기존 동작 그대로다. 기존 자산의 Cook 결과는 바뀌지 않는다.

| 옵션 | 해결한 차단 사유 |
|---|---|
| `-BakeFrameRate 30.0` | clip마다 rate가 다른 PSA를 단일 rate로 bake |
| `-AllowBoneOrderRemap` | PSA BONENAMES가 PSK bone의 순열일 때만 이름 기준 통과 |
| `-ArmatureExportName mesh` | runtime clip 접두사 `mesh_` 생성 |
| `-MeshExportName mesh.001` | body와 같은 skeleton node 이름 재현 |

`-MeshExportName`은 감사 단계에서 예상하지 못한 4번째 항목이다. 첫 cook은 성공했지만
skeleton hash가 body와 달랐고, 원인은 Mesh object node 이름 하나(`mn_rpbf_00_sk.mo` 대
body의 `mesh.001`)였다. runtime `skeletonHash`가 FNV-1a로 node 이름까지 포함하기 때문에
이 한 글자 차이로 `Attach_AnimationSet`이 실패한다.

`Cook-ActorXWModel.ps1`의 scale key 검사는 `원본과 정확히 같은 수`에서
`원본보다 적으면 실패`로 정밀화했다. 근거는 12.3에 있다.

`retime_wmodel_from_psa.py`는 `--psa`를 여러 번 받도록 확장했다. 발탄 AnimSet은 PSA 2개로
조립되기 때문이다. 단일 PSA 경로는 그대로이며 회귀 테스트로 고정했다.

### 12.2 표 C — 리소스 복원 증거

```text
sourceFile   mn_rpbf_00_sk.psk        997156 bytes
             sha256 e5d6798c3b59ccf21a1dd62ff86fb1dd0d1956b50b0079dec1ab6cb38620e97b  일치
sourceFile   mn_rpbf_00_ani.psa     32610976 bytes
             sha256 a9cad8d5c521070ddb4c38e4a72550e886c4330c0b970cae81a26fb4e74dd50d  일치
sourceFile   mn_rpbf_00_evt2_ani.psa 4403440 bytes
             sha256 596c86d8df8b50bfbca9f1d0c6265df1a768400b5e7e7d17827c9d62b7b88e64  일치

animationCount        146   (132 + 14)
duplicateCount          0   AnimSet 내부, body와의 이름 충돌도 0
skeletonNodeCount      87   RootNode + mesh(Armature) + PSK bone 84 + mesh.001(Mesh)
skeletonAlignedCount   87 / 87   body와 bone 목록 완전 일치
skeletonHash           body 0xADC0AB8374D8398C == animset 0xADC0AB8374D8398C
cookedOutput           C:\LostArkValtanCook\MN_RPBF_01_AnimSet\MN_RPBF_01_AnimSet.wmodel
                       46543044 bytes
                       sha256 7066270bd77e7da264a68aec784ae1d12001ec77c046f5352417b6b99db2a951
validatorResult        Cook-ActorXWModel.ps1 전체 검증 PASS, staging 패키지 commit
retimeResult           animationCount 146, changedAnimationCount 6
attachedTotalAnimationCount   173   (body 27 + AnimSet 146)
missingPatternBindingCount      0
```

지시서의 `87 bones`는 PSK가 아니라 **WModel skeleton node 87개**를 가리키는 값이었다.
PSK/PSA 자체는 84개이고, 나머지 3개는 `RootNode` 와 Armature/Mesh object node다.

clip별 원본 rate를 되돌린 6개는 다음과 같다. 나머지 140개는 원본이 이미 30.0이다.

```text
mesh_idle_battle_1          28.7143
mesh_att_battle_6_01_start  28.6957
mesh_evt2_idle_action_01    28.9286
mesh_evt2_finalfight_02_5   28.6957
mesh_evt2_reveal_01         28.5484
mesh_evt2_finalfight_02_6   28.9286
```

### 12.3 scale key 검사를 정밀화한 근거

Cook 결과 12,264개 channel을 PSA 원본과 전수 대조했다.

```text
원본보다 key가 적은 channel (데이터 손실)   0
시간 범위·순서가 어긋난 channel              0
bone별 scale 극값이 원본과 다른 channel      0
재샘플로 key가 1~2개 더 많은 channel        49  (전체의 0.4%)
```

추가 key는 duplicate time이 아니라 fractional tick의 재샘플이며, 시작 t=0과 끝 t=durationTicks,
단조 증가, bone별 min/max가 모두 원본과 일치한다. 또한 이 현상은 원본 rate가 이미 30.0이라
retime이 전혀 일어나지 않는 clip 140개 중 33개에서도 나타난다. 따라서 `-BakeFrameRate`가
원인이 아니라 이 자산과 Assimp 조합의 고유 동작이다.

기존 검사가 실제로 지키려던 계약은 `scale track을 잃지 않는 것`이므로, 개수 완전 일치 대신
`원본보다 적으면 실패`로 바꾸고 극값 정확 비교는 그대로 두었다.

### 12.4 복원 후 표 A / 표 B

```text
[B] attach 후 사용 가능한 clip 이름   173
    121 binding이 참조하는 고유 clip    61 / 61 물리 존재
    binding 행 기준 물리 누락            0 / 121
    clipmap 108 clip 물리 누락           0 / 108

[A] 논리 판정   EXACT 121 / APPROX 0 / CONFLICT 0 / MISSING 0
    물리 판정   PRESENT 121 / MISSING_PHYSICAL 0
```

이제 `Valtan.cpp:88 Load_PatternBindings()`의 `Validate()`가 문서를 거부할 이유가 없고,
Release의 AnimSet fail-closed 조건도 해소됐다. 다만 이는 **정적 계약 검증**이며 실제 실행
화면 확인은 사용자 몫이다.

### 12.5 복원 실패 정책 준수 확인

지시서 7절의 금지 항목을 모두 지켰다.

```text
PSK/PSA hash 불일치 시 중단          해당 없음 (3개 모두 일치)
skeleton 미정렬 시 제품 경로 배치 금지  87/87 정렬 확인 후에만 배치
146개 아니거나 중복 있으면 배치 금지    146개, duplicate 0 확인 후 배치
Attach 실패 시 body/BossCatalog 우회   BossCatalog 미수정, body 미수정
body 27 clip을 animationSetId에 넣기   하지 않음
Debug fallback 성공을 복원 완료로 보고  하지 않음
원본 파일 덮어쓰기                     원본은 읽기만, 작업은 별도 staging
```

## 13. 표 G — occurrence 매핑

입력은 2026-08-16 사용자가 제공한 발탄 전투 관찰 순서 1~67이다. 저장소에는 없던 자료이므로
출처는 `USER_OBSERVED`이며, 영상 프레임·수치 실측이 아니라 플레이 중 육안 관찰이다.
체력줄은 관찰자가 명시한 occurrence에만 채웠고 나머지는 비워 두었다.

기계 판독용 전체 표는 세션 scratchpad의 `tableG_occurrence_mapping.csv`에 있다.

### 13.1 판정 등급의 의미

```text
HIGH            체력줄 트리거 또는 displayName 수준까지 일치
MEDIUM          기존 패턴의 의미와 맞지만 트리거나 이름으로 확정되지는 않음
LOW             후보가 둘 이상이거나 관찰된 요소 일부가 모델링되어 있지 않음
GAP             관찰된 동작에 대응하는 패턴이 ValtanEncounter.json에 아예 없음
PHASE_MARKER    공격이 아니라 페이즈 표시
MISSING_RECORD  관찰 목록 자체에 번호가 비어 있음
```

```text
HIGH 16 / MEDIUM 30 / LOW 11 / GAP 8 / PHASE_MARKER 1 / MISSING_RECORD 1  = 67
참조한 patternId 20종, 전부 ValtanEncounter.json에 실재 (허구 ID 0)
환경 변화가 동반된 occurrence 20건
```

### 13.2 체력줄과 페이즈 진행

관찰된 체력줄만 나열하면 다음과 같다. 55번에서 망령화가 일어나고 56번에서 HP가 40으로
회복되므로, 이후 번호는 새로 시작한 40줄 기준으로 다시 내려간다.

```text
P1     160(1) 159(4) 115(19) 109(21)
P2     100(25)  <- 2페이즈 시작 컷신
P2.5    95(28) 84(32) 73(37)
P3      62(41) 30(48) 29(49) 28(51)
GHOST   14(55) -> HP 40 리셋(56) -> 37(57) 34(58) 31(59) 30(60) 19~10(64)
```

체력줄로 확정된 대형 패턴은 전부 일치했다.

| 체력줄 | 관찰 | patternId | 근거 |
|---|---|---|---|
| 159 | 회오리로 주변 벽 부수기 | `VALTAN_ARMOR_BREAK_OPENING` | trig=159 |
| 115 | 6갈래 후 전방향 충격파 | `VALTAN_FLOOR_WIPE_130` | trig=115, 이름 일치 |
| 109 | 외각벽 전부 붕괴 | `VALTAN_ARENA_BREAK_109` | trig=109 |
| 100 | 중앙 착지 + 2페이즈 컷신 | `VALTAN_FOUR_PILLARS_105` | trig=100 |
| 73 | 남북→동서 원형구체 폭발 | `VALTAN_MAGIC_ORB_STAGGER_76` | trig=73, 이름 일치 |
| 62 | 3페이즈 시작 컷신 | `VALTAN_CENTER_GRAB_COUNTER_64` | trig=62 |
| 30 | 지반 절반 삭제 | `VALTAN_ARENA_BREAK_33` | trig=30 |
| 14 | 포탈·망령화 | `VALTAN_GHOST_TRANSITION_15` | trig=14 |

### 13.3 반복 구조

1~67은 67개의 서로 다른 스킬이 아니라 시간순 기록이라는 지시서 전제가 실측으로 확인됐다.
가장 많이 반복된 관찰은 다음과 같다.

```text
"3번 휘두르고 내려찍기 + 한바퀴 충격파"   6,17,20,38,44,53,62,67   (8회)
"땅 3번 치고 돌진"                      2,5,10,18,24              (5회)
"도끼 기모아 동서남북 돌뿌리"             15,22,23,29,31,45,60      (7회)
"플레이어 위치 기반 도끼 4번 내려찍기"     16,35,46                  (3회)
"비석 2개씩 폭파"                       27,34,50                  (3회)
"랜덤 범위 충격파"                      57,61,65                  (3회)
```

지시서 11절의 요구대로 1~67 관찰 순서는 그대로 보존하고, 실행 데이터는 계속 32개 재사용
패턴으로 둔다. 복합 관찰은 `patternId`를 `|`로 이어 적었고 단타로 축약하지 않았다.

### 13.4 이번 매핑에서 드러난 실제 공백 8건

관찰됐지만 `ValtanEncounter.json`에 대응 패턴이 없는 항목이다. 추측으로 채우지 않고 GAP으로 남겼다.

| occurrence | 관찰 | 공백 내용 |
|---|---|---|
| 27, 34, 50 | 도끼가 붉게 변하며 비석 2개씩 폭파 | 비석 폭파 시퀀스 패턴 없음 |
| 32 | 84줄 지형 외각 절반 붕괴 | 84줄 트리거 패턴 없음 |
| 33, 49 | 비석 4개 생성 후 기 방출 | 재사용 가능한 비석 패턴이 `trig=100` 전용뿐 |
| 57, 65 | 망령 페이즈 랜덤 범위 충격파 | 망령 전용 랜덤 장판 패턴 없음 |

추가로 패턴은 있으나 관찰된 요소가 모델링되지 않은 항목이 있다.

```text
8,11   공중 도끼 투척          VALTAN_HIGH_JUMP의 AIRBORNE에 투사체가 없음
58     파괴 가능한 발탄 분신    VALTAN_PORTAL_RUSH에 분신 entity가 없음
64,66,67 포탈 2개->3개 단계 변화 RUSHES stage가 포탈 개수 변화를 소유하지 않음
36,47  7방향 에너지파          문서화된 방사형은 6방향
25     6갈래 충격파            displayName은 '4기둥 추적 원뿔'이라 설명이 어긋남
55     하위 동작 수            관찰 동작이 현재 7 stage보다 많음
```

### 13.5 이번 판에서 한 번도 관찰되지 않은 패턴 12개

selection weight 기반 랜덤 선택이므로 한 판에 전부 나오지 않는 것은 정상이다. 다만 다음 패턴은
이 기록만으로는 검증되지 않았다.

```text
VALTAN_SWING              VALTAN_IMPRISON_ROAR      VALTAN_PARRY
VALTAN_MAGIC_CHOICE       VALTAN_STOMP              VALTAN_BIND_CHARGE_SMASH
VALTAN_SUPER_SMASH        VALTAN_CHARGE_GRAB_ROAR   VALTAN_BACKSTEP_ATTACK
VALTAN_RED_BLADE_WAVE     VALTAN_LEDGE_ROAR         VALTAN_TRIPLE_COUNTER
```

`VALTAN_STOMP`은 단독으로는 관찰되지 않았지만 "땅 3번 치고 돌진"의 windup일 가능성이 있어
2번 항목의 note에 남겼다.

### 13.6 사용자 확인이 필요한 항목

에이전트가 임의로 결정하지 않고 남긴 질문이다.

```text
1. occurrence 7이 목록에 없다. 6번이 두 동작을 합친 것인지, 7번 관찰이 누락된 것인지.
2. 6번 "3번 휘두르고 내려찍기"와 16번 "도끼 4번 내려찍기"가 같은 VALTAN_FOUR_SLASH인지
   서로 다른 패턴인지.
3. 13번 "랜덤위치 점프 + 돌 상승"이 VALTAN_HIGH_JUMP인지 VALTAN_EARTHQUAKE_SMASH인지.
4. 15번 계열 "동서남북 돌뿌리"가 VALTAN_GROUND_WAVE_SMASH인지 VALTAN_EARTHQUAKE_SMASH인지.
5. 36/47번 에너지파가 실제로 7방향인지. 6방향이면 FLOOR_WIPE 계열과 통합 가능하다.
6. 42번 에너지 구의 실제 결말. 관찰자가 스턴으로 보지 못했다고 기록했다.
```

이 여섯 가지가 확정되면 LOW 11건 중 상당수가 MEDIUM 이상으로 올라간다. 확정 전에는
Encounter/Server 수치를 이 표를 근거로 수정하지 않는다.

## 14. 이전 정리본과의 교차검증

사용자가 이전에 별도로 작성한 두 표(`반복 일반 스킬 정리` 12계열, `체력줄 고정 특수패턴` 16행)를
현재 데이터로 재현했다. 양쪽이 서로를 교정했으므로 어느 쪽도 그대로 정답으로 복사하지 않았다.

### 14.1 이전 정리본이 옳았고 13절 표 G를 고친 항목 4건

실제 stage의 `hitShape`를 확인하니 이전 정리본의 지적이 맞았다. 표 G를 수정했다.

| 관찰 | 표 G 최초 판정 | 실측 | 수정 후 |
|---|---|---|---|
| 동서남북 돌뿌리 (15,22,23,29,31,45,60) | `GROUND_WAVE_SMASH` | 그 stage는 `CONE ang75`로 십자가 아님 | `VALTAN_DOWN_SMASH`(IMPACT=`CROSS` len10 hw1.8) |
| 플레이어 추적 4회 찍기 (16,35,46) | `FOUR_SLASH` | `CONE ang110 len9`의 전방 원뿔 4연타 | GAP |
| 3회 지면 강타→돌진 (2,5,10,18,24) | `DASH_CHARGE` 단독 | `SUPER_SMASH.IMPACTS`가 `CIRCLE r13 cnt=3 int=550` | `SUPER_SMASH`+`DASH_CHARGE` 복합 |
| 3휘두르기→내려찍기→회전 (6,17,20,38,44,53,62,67) | `FOUR_SLASH\|WHIRLWIND` | 셋을 한 번에 담는 패턴 없음 | GAP |

`CROSS`(십자) 판정을 실제로 쓰는 stage는 5개뿐이다.

```text
VALTAN_DOWN_SMASH.IMPACT              CROSS len10.0 hw1.8
VALTAN_JUMP_SPIN.LAND                 CROSS len 9.0 hw1.8
VALTAN_FRONT_BACK_FRONT.SMASHES       CROSS
VALTAN_FLOOR_WIPE_130.FIRST_SMASH     CROSS len14.0 hw2.2
VALTAN_GHOST_TRANSITION_15.FOUR_DIRECTIONS  CROSS
```

또한 "피해 프로필만 있고 Encounter 패턴은 없다"는 지적도 실측으로 확인됐다.
발탄 damageProfile 32개 중 **어떤 stage도 참조하지 않는 고아가 정확히 2개** 있다.

```text
damage.valtan.jump-axe-throw        damageRatePercent=500   공중 도끼 투척
damage.valtan.four-direction-walls  damageRatePercent=250   4방향 벽
```

### 14.2 이전 정리본이 지금은 틀린 항목 6건 — patternId 접미 숫자를 읽었다

`체력줄 고정 특수패턴` 표에서 `체력줄 불일치`로 판정한 6행은 **현재 HEAD 기준으로 전부 일치**한다.
patternId 뒤의 숫자는 옛 이름이 남은 것이고, 실제 정본은 `triggerHealthBar` 필드다.

| 관찰 체력 | patternId 접미 숫자 | 실제 `triggerHealthBar` | 이전 판정 | 실제 |
|---|---|---|---|---|
| 115 | `_130` | **115** | 체력줄 불일치 | 일치 |
| 100 | `_105` | **100** | 정확한 전환 없음 | 일치 |
| 73 | `_76` | **73** | 추정 대응 | 일치 |
| 62 | `_64` | **62** | 체력줄·내용 불일치 | 일치 |
| 30 | `_33` | **30** | 체력줄 불일치 | 일치 |
| 14 | `_15` | **14** | 체력줄 불일치 | 일치 |

근거는 커밋 하나다.

```text
37bbbcb "Tune Valtan observed health-bar mechanics"
  triggerHealthBar 130 -> 115,  105 -> 100,  76 -> 73
                    64 ->  62,   33 ->  30,  15 ->  14
  displayName 도 "115줄 ...", "100줄 ...", "73줄 ..." 로 함께 갱신
  다만 patternId 는 그대로 두었다
```

즉 체력줄 값은 이미 관찰과 맞게 튜닝돼 있었고, 남은 문제는 **ID 이름이 값을 따라가지 않은 것**뿐이다.
`AGENTS.md`가 금지하는 "ID를 저장 계약으로 읽기"에 걸리는 전형적인 사례라, 이 표를 근거로
값을 다시 바꾸면 오히려 회귀가 된다.

`patternId`는 저장 계약이므로 rename하면 `patternbindings`, publisher, Server bootstrap이 같이
움직여야 한다. 이번 변경 단위에서는 손대지 않았고, 별도 수직 슬라이스로 남긴다.

### 14.3 양쪽이 일치한 항목

```text
109줄 아레나 붕괴          ARENA_BREAK_109      trig=109  양쪽 모두 일치 판정
앞뒤앞 내려찍기            FRONT_BACK_FRONT     양쪽 일치
회오리 계열                WHIRLWIND / ENTRANCE_WHIRLWIND  양쪽 일치
포탈 분신 돌진             PORTAL_RUSH 부분 대응  양쪽 일치
랜덤 범위 충격파           망령 전용 패턴 필요     양쪽 GAP
84줄 지형 절반 붕괴        패턴 없음             양쪽 GAP
95줄 2.5페이즈 전환 조건    없음                 양쪽 GAP
28줄 양손 내려찍기          이전=없음 / 표 G=FIST_IN_OUT 부분 대응
```

`28줄 양손 내려찍기`만 판정이 갈린다. `VALTAN_FIST_IN_OUT`은 `INNER CIRCLE r7` +
`OUTER RING 7~16`이라 "양손 내려찍기 후 외곽 충격파"와 구조가 같다. 다만 관찰은 아레나 외곽
전체이고 현재 반경은 16m라 범위가 다르다. 표 G는 MEDIUM으로 두었다.

### 14.4 교차검증 후 최종 수치

```text
HIGH 16 / MEDIUM 19 / LOW 10 / GAP 20 / PHASE_MARKER 1 / MISSING_RECORD 1 = 67
참조 patternId 19종, 전부 실재 (허구 ID 0)
```

이전 정리본의 "독립적인 일반 스킬 계열 대략 12개"는 현재 데이터와 대조해도 유지된다.
그중 **현재 Encounter에 정확히 대응하는 계열은 5개**(회오리, 앞뒤앞, 랜덤 점프 착지 일부,
포탈 분신 일부, 3회 강타+돌진 복합)이고 나머지는 GAP이거나 부분 대응이다.

관찰 기록에 한 번도 나오지 않은 Encounter 패턴은 13개다. 교차검증으로 `FOUR_SLASH`와
`GROUND_WAVE_SMASH`가 이 목록에 새로 들어왔다. 두 패턴은 정의는 있으나 이번 실전 기록의
어떤 동작과도 형상이 맞지 않는다.

```text
SWING              IMPRISON_ROAR      PARRY            MAGIC_CHOICE
FOUR_SLASH         STOMP              BIND_CHARGE_SMASH
GROUND_WAVE_SMASH  CHARGE_GRAB_ROAR   BACKSTEP_ATTACK
RED_BLADE_WAVE     LEDGE_ROAR         TRIPLE_COUNTER
```

### 14.5 세 번째 화면(담당 범위 확인 문장)에 대한 사실 확인

스크린샷의 문장은 담당 범위를 "발탄 공격 명세와 수치 데이터 정본 제공 → 발탄 맵의
몬스터·스테이지 진행 동선 완성"으로 요약한다. 현재 저장소 상태 기준으로 다음은 사실이다.

```text
ValtanEncounter/DamageProfiles 반영 경로   존재한다
  Data/Encounters/Valtan/ValtanEncounter.json + Data/Balance/DamageProfiles.json
  -> Tools/GameplayPipeline/Publish-GameplayBalance.ps1
  -> Server bootstrap -> Server fixed tick 판정

MapTool의 SpawnGroup/트리거 설치 경로      존재한다
  Data/Worlds/<AreaId>/Gameplay.world.json 의 triggerBox activateSpawnGroup /
  activateEncounter, SpawnGroups.world.json
  -> Tools/WorldPipeline/Publish-WorldGameplay.ps1
```

다만 이번 감사에서 확인한 제약 두 가지를 함께 알려 두는 편이 좋다.

```text
1. 원작 실제 피해량은 이번 세션에서 확정하지 못했다. 발탄 damageRatePercent 32개는
   전부 PROJECT_TUNED이고 provenance receipt도 "official source binding is not claimed"로
   기록돼 있다. "수치 데이터 정본"을 원작 실측값이라는 뜻으로 쓰면 사실과 다르다.
2. 관찰된 동작 중 20건이 GAP이다. 특히 비석 생성/파괴 주기, 84줄 지형 붕괴,
   망령 페이즈 전용 패턴, 포탈 개수 스케줄은 Encounter schema 자체에 자리가 없다.
   기존 필드만 채워서는 닫히지 않고 새 stage/mutation 계약이 필요하다.
```
