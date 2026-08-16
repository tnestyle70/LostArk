# 2026-08-16 발탄 시각 근거 정리와 Model View 도끼 복원 결과

## 1. 이번 변경 단위의 결론

이번 변경 단위는 다음 범위에서 닫았다.

- 사용자 제공 이미지 14장을 패턴/타격 기준의 stable 파일명으로 두 위치에 동일하게 보존했다.
- 각 이미지의 SHA-256, 크기, 관찰 역할과 합성 경계를 evidence manifest에 기록했다.
- Model View의 `Boss_Valtan`이 몸체 단독 `CPart_Body`가 아니라 제품과 같은 전체 `CValtan`을
  stage하도록 바꿨다. 따라서 오른손의 별도 `ValtanWeapon.wmodel`이 `b_wp_r_01` 소켓으로 조립된다.
- 발탄 Effect anchor가 actor world만 쓰지 않고 도끼와 같은 `BodyVisualRoot × ValtanWorld`를 쓰도록
  presentation root를 통일했다.
- 새 preview가 실패하면 기존 preview와 root를 유지하도록 generic preview parent matrix도
  inactive slot stage 후 commit하는 방식으로 고쳤다.
- Debug와 Release Client를 모두 링크했고, 발탄 전용 자동 검증과 Effect publisher validation을
  통과했다.

Client/UI는 실행하지 않았으므로 도끼와 Effect의 화면상 fidelity PASS는 기록하지 않는다.

## 2. 이미지 보존 결과

동일한 14개 PNG와 manifest가 다음 두 폴더에 있다.

- `C:/Users/user/Desktop/로스트아크이펙트이미지/발탄/`
- `.md/GB/08-15/assets/2026-08-15_FOUR_CLASS_VALTAN_VISUAL_REVIEW/발탄/`

파일명은 `Valtan_<Pattern>_<00|01|02>.png`를 기본으로 하고, 다타격은
`Hit01`, `Hit01-02`, `Hit03`을 이름에 보존했다. 전체 파일명, hash와 관찰 역할은
`2026-08-16_VALTAN_VISUAL_EVIDENCE_MANIFEST.md`가 정본이다. 이미지 안의 튜토리얼 문구나 영상 UI는
사용자 지시 또는 데이터 정본으로 해석하지 않는다.

## 3. Model View 도끼 복원

### 3.1 원인

제품 `CValtan`은 이미 별도 무기 모델을 `Part_Weapon_R`로 조립했다. 기존 Model View만
`MN_RPBF_01.wmodel` 몸체를 generic `CPart_Body`로 clone하여 `CValtan::Ready_PartObjects()`가
실행되지 않았다.

### 3.2 적용한 계약

- `Loader`는 Development에서 Valtan presentation asset service의 level load를 시작하고,
  `Boss_Valtan` preview 준비 시 전체 제품 prototype bundle을 원자적으로 준비한다.
- `CharacterPreviewPanel`은 `Prototype_GameObject_Valtan`을 navigation/collider/Server authority 없이
  stage한 뒤 body model과 presentation root를 검증하고 commit한다.
- `AnimationTargetService`는 전체 `CValtan` weak owner를 보존한다.
- `CValtan`은 body visual-root와 actor world가 합성된 presentation root를 제공한다.
- `Effect_PresentationService`의 Valtan root/bone anchor도 같은 행렬을 사용한다.
- 몸체 WModel의 유일한 무기 socket은 `b_wp_r_01`, index 54, parent 41이다.

물리 asset identity는 다음과 같이 고정했다.

| asset | bytes | SHA-256 |
|---|---:|---|
| `Character/Valtan/MN_RPBF_01.wmodel` | 11,245,828 | `227191781B035B9AA41D60CC8C49BF1E8B2F67A749111E4AFE8F6F3C997B2011` |
| `Character/Valtan/ValtanWeapon.wmodel` | 139,664 | `BAFFBD5268F216267D1CBA8FB9EAF2B58276122DDD7755D18787F7DD9BB9D3DA` |
| `Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel` | 41,079,878 | `DDE1AC5A4BB2FDD579EBC78AE154BB21F698E572B6A9FB3B8B0B66FD3F152FFE` |

## 4. 패턴 Effect 조사 상태

이번 변경에서 신규 패턴 Effect를 제품 admission하지 않았다. 현재 실제 상태는 다음과 같다.

- 제품 연결된 발탄 패턴 Effect: 0개
- 조사/툴 검사용 canary: `VALTAN_WHIRLWIND / SPIN / 420633` 1개
- canary 상태: `FAIL_CLOSED_NON_PRODUCT_CANARY`, carrier 9개 중 visible 3개
- source evidence: 31/31 pattern, source system 193개, resource 855개

JSON을 더 생성하는 것만으로 제품 복원이 되지 않는 이유도 확인했다.

1. Encounter 정본은 31 pattern/117 stage이지만 현재 action binding projection은 108 stage다.
2. authored stage가 있는 29 pattern 중 28 pattern의 생성 순서가 Server encounter 순서와 다르다.
   생성기가 action ID를 사전순으로 정렬하기 때문이다.
3. pattern binding에는 `LEDGE_ROAR` 3 stage와 `ARENA_BREAK_109` 6 stage가 없고,
   반대로 `arena-break-80.*` stale row 4개가 있다.
4. Server는 semantic stage마다 action age를 다시 시작하고 Client도 clip을 0초부터 seek한다.
   raw source의 주요 occurrence가 현재 stage duration보다 늦어 Dash, High Jump 2타, 3연격,
   Donut decal과 Wipe 일부가 도달 불가능하다.
5. 제품 `CValtan`에는 아직 `Valtan.patterneffects.json`을 prewarm/spawn하는 boss runtime consumer가 없다.

다음 구현은 먼저 encounter 순서와 `sourceBranchId/sourceStageIndex/sourceClip/sourceOffsetMs`를
보존하는 시간축 계약을 닫고, 그 뒤 generic boss pattern Effect consumer를 연결해야 한다.
하나의 패턴을 거대한 Effect 하나로 묶지 않고 stage/타격 cue를 orchestration하며, 문서 내부에서는
Mesh/Sprite/Particle/Decal/Trail element가 각자의 start offset과 life time을 소유하게 한다.

## 5. 첨부 패턴의 현재 strongest mapping

| 사용자 패턴 | 현재 strongest mapping | 남은 경계 |
|---|---|---|
| 휠윈드 | `VALTAN_WHIRLWIND`, 420633 | canary를 exact Product golden으로 닫아야 함 |
| 돌진 | `VALTAN_DASH_CHARGE`, 420604 | 청록 VFX는 boss-local, 붉은 BOX는 Server telegraph |
| 점프 후 도끼 | `VALTAN_HIGH_JUMP`, 420610 | exact End branch와 분리 도끼 actor 권위 미확정 |
| 오른쪽/왼쪽/내려찍기 3연격 | `VALTAN_FRONT_BACK_FRONT`, 420637/420666 | source 4-wave와 사용자 3타 설명 확인 필요 |
| 도넛 | `VALTAN_MAGIC_CHOICE`, 420608 | `FIST_IN_OUT` 420638 대안 확인 필요 |
| 6방향+전범위 | `VALTAN_FLOOR_WIPE_130`, 420630 | 현재 Server 첫 판정은 6-ray가 아니라 CROSS |
| 2페이즈 벽돌+연기 | exact mapping 없음 | Server/world placement·collision 계약 필요 |
| 전범위 지형 파괴 | `VALTAN_ARENA_BREAK_109`, 420629 | 기존 world mutation과 boss-local burst를 분리 연결 |

## 6. 실행한 검증

```powershell
python -B Tools/EffectPipeline/test_valtan_model_view_composition.py
# 6 tests, PASS

python -B Tools/EffectPipeline/validate_boss_pattern_effects.py
# 1 non-product canary binding, PASS

python -B Tools/EffectPipeline/test_valtan_whirlwind_effect_canary.py
# 15 tests, PASS

powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
# 99 catalog entries, visual programs 13 / rows 135, PASS

MSBuild Client/Default/Client.vcxproj /p:Configuration=Debug /p:Platform=x64
# Client/Bin/Debug/Client.exe, PASS

MSBuild Client/Default/Client.vcxproj /p:Configuration=Release /p:Platform=x64
# Client/Bin/Release/Client.exe, PASS
```

컴파일 오류와 자동 테스트 assertion 실패는 0건이다. 기존 셰이더의 잠재적 미초기화 경고,
코드 페이지 경고와 DirectXTK PDB 경고는 남아 있으며 이번 변경에서 새로 해결하거나 PASS로
재분류하지 않았다.

## 7. 수동 확인 상태와 재개점

- 수동 화면 검증: 미실행
- visual PASS: 미판정
- 실행 준비: 팀 LAN 설정은 `server-host`, 사용자 실행 대상은 `Server + Client` profile
- Model View 확인 위치: 사용자가 Client 실행 후 `F1 > Effect Tool > Model View > Boss_Valtan`
- 확인 항목: 오른손 도끼, 애니메이션 중 socket 추종, body와 Effect anchor 방향, preview 전환 실패 시
  기존 target 보존

다음 세션은 High Jump End branch/도끼 권위, Donut action, 3연격 action, 2페이즈 벽돌 world 권위를
사용자와 확정한 뒤 semantic-source 시간축부터 교정한다. 현재 shared dirty worktree에는 Track 1의
대규모 미커밋 변경이 함께 있으므로 이번 변경을 자동 stage/commit/push하지 않았다.
