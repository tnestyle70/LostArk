# 차원술사 V 시전자 앵커와 관전자 화면 범위 구현 계획

## G00. 현재 입력과 사용자 지정 기준

2026-10-04 현재 `(DIMENSIONMASTER, V)`는 `2050520`이며
`DimensionMaster.skillbindings.json`의 `pc_sp_m_00_sk_sk_timewave`를 사용한다.
`DimensionMaster.animevents`의 실제 asset cue는
`effect.dimensionmaster.skill.2050520.full.restore`, `anchor=root`,
`follow=snapshot`, `orientation=action_facing`이다. ALT V `2050540`은 변경하지 않는다.

첨부 문제 화면은 현재 프로젝트 화면이다. 사용자는 V를 처음 시전하는 장면에서 넓은 푸른
판과 붉고 검은 파편이 나타나며, 차원술사 앵커와 local space OFF를 원한다고 명시했다.
가디언 나이트 관전자 카메라에 붙는 상태를 원본 카메라 연출 복구 목표로 취급하지 않는다.
두 번째 시전부터 정상이라는 의미로도 확대 해석하지 않는다.

실제 저작 문서의 43개 요소 중 17개가 `camera_view/follow=true/localSpace=true`다.
`Resolve_SourceAnchors`는 이 조합에 현재 PC의 inverse VIEW를 적용하므로 실제 시전자와
무관한 관전자 카메라가 부모가 된다. 기존 캐시·native material·texture 재바인딩에서는
이번 현상을 설명하는 확정 누락을 발견하지 못했다.

09-15 T/V RESULT는 카메라 local space 복구와 미확정 화면 문제를 구분했다.
09-21 Q CUBE CLARITY RESULT G09의 V 유리 세 개 `fresnel_pow=0.5`는 사용자 튜닝이며
이번 변경에서 보존한다. 새 원본 UPK나 원본 shader 복원을 주장하지 않는다.

## G01. V 17개 요소를 시전자 snapshot으로 연결

수정 파일은 `Data/Effects/Authored/effect.dimensionmaster.skill.2050520.full.restore.effect.json`
하나다. `elements[].id`의 접두사 `authored.source-particle.full-v.`에 다음 suffix가 붙은
17개만 변경한다.

```text
dddd26700e1cd32033e7
c8727f95fd8234a116d6
ce3a1fad9987ee75d5bc
e2e9e0313c1b92a8c1d6
21d743e9813d1ac8215a
9e82051cc2a0d230db80
610db852a3def76706b0
462eea59a1d0912fa5a1
8b00187481182e5b039b
79236f560d9cd33cad24
7d69eb2b2e0f77a1d73e
4fcf9f74422f19a0d338
d4641ef3b50536b871d8
3b45c2c6f83c2f472890
f3b3ee4d94b4528d33db
18ce1c993cce94e337e5
ebad38e09d7a635e4e66
```

각 요소에서 아래 네 leaf 값만 교체한다. 기존 object의 다른 field는 그대로 둔다.

| JSON 필드 | 이전 | 변경 |
|---|---|---|
| `actionCueAttachment.follow` | `true` | `false` |
| `actionCueAttachment.orientation` | `"camera_view"` | `"bone"` |
| `actionCueAttachment.runtimeAnchorSlotId` | notify별 camera ID | `"root"` |
| `detail.particle.localSpace` | `true` | `false` |

`sourceAnchorSlotId`, source recipe의 원본 `buselocalspace=true`, 기존 socket TRS는 출처로
보존한다. snapshot 경로는 socket TRS를 소비하지 않는다. source yaw를 별도 보정으로
옮기지 않고 `snapshotRootSourceBasisYawDegrees=0`과 모든 기존 크기·타이밍·색·재질·
43개 구성·bloom0·fresnel0.5를 유지한다.

생산 경로는 `Resolve_Anchor(Owner, root)`의 해당 Character world → 기존 action-facing
cue root → `Seek`/`Step`의 `ActionRootWorld` → `Evaluate_ElementWorld` → 입자의
`SpawnRootWorld`다. follow=false 요소는 camera source-anchor 요청 목록에서 빠지며,
localSpace=false 입자는 생성 후 시전자나 관전자 카메라 이동을 따라가지 않는다.

## G02. V screenPost 네 요소의 관전자 범위

`Data/Effects/Sequences/effect.dimensionmaster.skill.2050520.full.restore.effectsequence.json`을
신규 추가하고 기존 `localOnlyElementIds` 계약으로 다음 네 화면 후처리만 해당 시전자 화면으로 제한한다.
실제 parser/cache/owner-mask 검증은 이 파일 담당자가 수행한다.

```text
authored.source-particle.full-v.41426adbb82bc4fdd49a
authored.source-particle.full-v.0476d3b62e94a1e2ac9e
dimensionmaster.2050520.projectile20505200.0fd0fc7aa0f6bebd715a
dimensionmaster.2050520.projectile20505200.41426adbb82bc4fdd49a
```

기존 cue의 natural/snapshot/action_facing 정책을 sequence 메타데이터에서도 유지한다.
카메라 row를 추가하거나 전체 V를 감추지 않는다. 월드 유리·파편·빛은 관전자에게도
시전자 위치에서 보이며, RGBNoise/ZoomBlur만 기존 local-owner 선택을 따른다.

### 신규 sequence 전체 데이터

```json
{
  "schema": "lostark.effect-authoring-sequence",
  "formatVersion": 4,
  "sequenceId": "effect.dimensionmaster.skill.2050520.full.restore",
  "model": {
    "kind": "MODEL_SEQUENCE",
    "assetName": "DimensionMaster",
    "sequenceId": "skill.2050520",
    "anchorMemberId": ""
  },
  "anchorMode": "MODEL_ROOT",
  "worldPosition": [
    0,
    0,
    0
  ],
  "effects": [
    {
      "occurrenceId": "dimensionmaster2050520.source.effect",
      "owner": "V1_DOCUMENT",
      "effectId": "effect.dimensionmaster.skill.2050520.full.restore",
      "anchorSlotId": "root",
      "startMs": 0,
      "durationMs": 5800,
      "offset": [
        0,
        0,
        0
      ],
      "muted": false,
      "screenPost": false,
      "productNaturalDuration": true,
      "productSnapshot": true,
      "productActionFacing": true
    }
  ],
  "cameras": [],
  "customAnimation": false,
  "animationRows": [],
  "soundRows": [],
  "colliderRows": [],
  "localOnlyElementIds": [
    "authored.source-particle.full-v.41426adbb82bc4fdd49a",
    "authored.source-particle.full-v.0476d3b62e94a1e2ac9e",
    "dimensionmaster.2050520.projectile20505200.0fd0fc7aa0f6bebd715a",
    "dimensionmaster.2050520.projectile20505200.41426adbb82bc4fdd49a"
  ]
}
```

## G03. 반영·검증·남은 화면 판정

후보는 `out/DimensionMasterV20261004`에 만들고 최신 디스크 SHA 비교, 백업과
`ReplaceFileW` 원자 교체를 사용한다. 교체 전 다른 저장이 발생하면 다시 읽어 같은 leaf만
병합하며 무관한 문서 변경은 보존한다. 교체 후 구조 diff는 17×4=68개 값이어야 한다.

실제 생산 함수/블록과 DirectXMath를 사용한 작은 CPU 검증으로 두 차원술사 root와 별도
가디언 관전자, 카메라 이동, 시전자 위치·action yaw, 생성 후 owner 이동을 대조한다.
이는 실제 GPU 화면이나 색·가림의 판정이 아니다. 현행 Effect source validator와 JSON
parse 및 scoped `git diff --check`를 수행한다.

Effect는 `DIRECT_AUTHORED_DOCUMENT`로 Data를 직접 읽는다. 별도 Effect publisher와
`Client/Bin/DataFiles/Effect` 복사본은 없다. 신규 sequence를 `Client.vcxproj`와
`Client.vcxproj.filters`의 기존 `96.DataFiles\Effects\Sequences`에 `None`으로 등록한다.
C++/HLSL 변경이 없으므로 제품 재빌드는 필요하지 않다. 파일 교체를 실행 중 immutable document의 자동 교체나
사용자 화면 통과로 기록하지 않는다. 도구 Reload/다음 준비와 최종 Client 확인은 별도다.
