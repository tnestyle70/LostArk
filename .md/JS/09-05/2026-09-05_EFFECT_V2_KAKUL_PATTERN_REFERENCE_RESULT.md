# 2026-09-05 Effect Attach v2 쿠크세이튼 Pattern Reference RESULT

브랜치: `main` `afbce7d8` 위 미커밋 작업.
수정 파일: `Client/Public/Effect_Tool_V2.h`, `Client/Private/Effect_Tool_V2.cpp`. 새 파일·project 등록 없음.

## 1. 요청

Effect Attach v2에서 쿠크 몸체(`MN_RPCZ_00` 등)를 Spawn했을 때 팀장이 Action Workbench로 저작한
`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 패턴(stage → animation occurrence)을 보고 재생할 수
있게 해서, 이펙트 저작 시 참고하도록 한다.

## 2. 실측

- Composition revision 17, 패턴 10개. PRODUCT `KAKULSAYDON_G1_PIZZA`(RPCZ_00, 6 stage) 하나와 DRAFT 9개.
  stage가 있는 DRAFT는 `PATTERN_1/2/4/5`(전부 `MN_RPCT_05`, 각 7/13/3/12 stage). 나머지 DRAFT는 stage 0.
- Client에는 이미 `CKoukuSaydonCompositionDocument`(parse/validate/Reload)가 있어 새 파서를 만들지 않았다.
- Animation Tool의 `Sample_KoukuSaydonCompositionPreview`가 row 선택·gap 복원·`EXACT/HOLD_LAST_POSE/LOOP_TO_WINDOW`
  clock 계산 기준이라 같은 규칙을 따랐다.

## 3. 구현

`Render_ValtanPatternSection` 뒤에 `Render_KoukuPatternSection`을 추가했다. 표시 조건은 target이
`PREVIEW_BODY`이고 binding owner(`m_Target.strArchetypeId`)가 `KAKUL_PREVIEW_TARGETS`에 있을 때다.

- `Ensure_KoukuComposition`: 첫 표시에서 한 번 `Reload`, `Refresh Composition`으로 재읽기. Save 경로 없음(read-only).
- Pattern 콤보: 전체 패턴을 `patternId [status] displayName (actorProfileId, N stages)`로 나열하고
  `actorProfileId != binding owner`인 패턴은 disabled. `MN_RPCT_00` 몸체는 owner가 `MN_RPCT_05`라 05 패턴이 열린다.
- 패턴 선택 후 재생 전에는 stage/occurrence 목록(`stageId [kind] clip +offset x playMs rate endPolicy`)을 텍스트로 보여준다.
- `Build_KoukuTimeline`: stage duration을 누적해 occurrence를 pattern-absolute ms로 펼친다. 현재 모델에 없는 clip이나
  `playMs 0`은 건너뛰고 status에 Skipped로 적는다.
- `Apply_KoukuTimeline`: clock에 걸린 row(없으면 직전 row의 끝 pose)를 골라 clip 전환 시 `Play_TargetClip`(runtime
  `Notify_Clip` 포함), 그 뒤 `Set_AnimPaused(true)` + `Set_AnimTrackPosition`으로 pose를 고정한다. clip-local ms =
  `sourceStartMs + (clock - rowStart) * playRate`, `LOOP_TO_WINDOW`는 clip 길이로 fmod, 나머지는 clamp.
- `Update_KoukuTimeline`: Valtan과 같은 play/pause/loop, Spawn Point 통과 시 preview `Restart`.
- `Spawn Here`: pattern clock의 spawn 지점을 기록하고 그 시점의 clip-local ms를 `Spawn Frame`(30fps)으로 복사한다.
  기존 `Add / Update Binding`은 그대로 현재 clip + Spawn Frame을 clip lane(v1) 행으로 저장하므로 바인딩 codec 변경 없음.
- `Stop_KoukuTimeline`: 현재 clip을 loop 재생으로 되돌린다. Target Clip 콤보 선택, binding row 클릭, Despawn,
  Deactivate에서 timeline을 정지한다. `Update_Attach`의 clip wrap 재시작은 Kouku timeline 중에는 건너뛴다.

## 4. 검증

실행한 것:

- `git diff --check` 통과.
- `Client.vcxproj` Debug x64 MSBuild 성공(오류 0), `Client/Bin/Debug/Client.exe` 20:05 재링크.
- 로컬 Server(`127.0.0.1:7777`) + Client 재실행.

사용자 확인 대기:

- F1 → Effect Tool v2 → Attach → Archetype `MN_RPCZ_00` → Spawn Target → `Pattern Reference` 섹션에서
  `KAKULSAYDON_G1_PIZZA` 선택 → Play Pattern → scrub, Spawn Here 후 Add Binding 시 clip/offset 표시.
- `MN_RPCT_05` 몸체에서 `PATTERN_1/2/4/5` 재생, `PATTERN_4`의 `LOOP_TO_WINDOW` row 확인.

## 4.1 쿠크/세이튼 배율 2.0 → 1.7 (팀 합의, 같은 작업 단위)

팀장 09-05 G09가 0.01 → 0.02로 올린 네 곳을 0.017로 바꿨다. 다른 모델과 Server collider radius는 그대로다.

| 파일 | 변경 |
|---|---|
| `Client/Public/AnimationPreviewAssets.h` | KoukuSaton 4개 `fPreviewScale` 0.02f → 0.017f |
| `Data/Actors/BossCatalog.json` | `BOSS_KAKULSAYDON_G1_KOUKU.bodyModelPreScale` 0.02 → 0.017 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | 쿠크 body-only admission 고정값 0.02 → 0.017 |
| `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | 기대값 0.017 |

검증: `Invoke-BuildDomainOwner -Owner KoukuSaydon` gameplay.balance PASS, `test_kouku_saydon_runtime_inputs` 3건 중
쿠크 catalog 검사 통과(실패 1건 `1788 != 1867`은 Gameplay.world revision 불일치로 기존 무관 실패), Client Debug 재빌드 오류 0,
`git diff --check` 통과. Server/Client 재실행 후 사용자 육안 확인 대기.

## 4.2 Effect Tuning v2 Scale In End / Scale Out Start

Alpha In End/Out Start와 같은 life-ratio envelope를 Scale에 추가했다. 기존 Scale Start/End lerp 결과에 곱한다.

| 파일 | 변경 |
|---|---|
| `Client/Public/EffectV2_Object.h` | `PARAMS::fScaleInEnd`(0=off), `fScaleOutStart`(1=off), `Scale_Envelope()` 선언 |
| `Client/Private/EffectV2_Object.cpp` | `Scale_Envelope()`는 `Alpha_Envelope()`와 같은 식. `Apply_Transform`에서 `Scale.Evaluate` 결과에 곱한 뒤 기존 0.001 clamp |
| `Client/Private/EffectV2_Document.cpp` | params `scaleInEnd`/`scaleOutStart` optional read + serialize(alpha 뒤) |
| `Client/Private/Effect_Tool_V2.cpp` | Tuning의 Scale 행 아래 두 슬라이더, clamp 안내, Lifetime 0 경고 |

Lifetime 0이면 envelope 1. Python 파이프라인은 params 키를 optional로 읽고 alpha/scale envelope를 참조하지 않아
변경 없음. 기존 문서는 키가 없으면 기본값(off)이다. Client Debug 빌드 오류 0, `git diff --check` 통과. 사용자 확인 대기.

## 5. 남은 것

- 팀장이 stage에 붙인 Logic/Summon occurrence는 이 참고 뷰에 표시하지 않는다(애니메이션 lane만).
- 바인딩은 여전히 clip lane이라 같은 clip이 여러 stage에서 반복될 때(피자 windup A/B) 이펙트도 매번 발화한다.
  stage 단위로 나누려면 쿠크 stage lane 바인딩 계약이 필요하다.
