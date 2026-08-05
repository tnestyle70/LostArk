# 창술사 스킬 애니메이션 체인 인게임 재검토 — 1차 데이터 수정

작성자: JS · 2026-08-05 · 브랜치 `feature/lancemaster-skill-slots`

사용자가 인게임에서 창술사 트라이포드 없는 기본 스킬 애니메이션을 직접 확인하고 보고한
버그 목록을 실측 데이터(`Data/Animation/Reference/LanceMaster/LanceMaster.clipseq`,
`.animnotify`)로 대조한 뒤, 확정된 항목만 `LanceMaster.skillbindings.json`에 반영했다.
새 홀딩/가드 메커니즘은 이번 범위에서 제외했다(사용자 확인, 아래 4절).

## 1. 재생 구조 재확인

`CCharacter::Update_Chain()`(`Client/Private/Character.cpp:213`)은 바인딩된 clips 배열을
`isCombo`가 false인 모든 스킬에서 입력 없이 끝까지 자동 재생한다. 즉 한 번의 입력으로
몇 타까지 나가는지는 순수하게 `LanceMaster.skillbindings.json`의 clips 배열 길이가 결정한다.
`isCombo`는 `PlayerSkills.json`의 `skillKind == COMBO`일 때만 true이며, 이번에 다룬 스킬은
전부 `ACTIVE`라 서버 comboStage 없이 클라이언트 체인이 전부를 소유한다.

`.clipseq`의 `mode` 필드(SEQUENCE/COMBO/HOLD)는 클립 이름 패턴으로 추론한 값이라
신뢰할 수 없다(`2026-08-04_SKILLKIND_BASIC_ATTACK_COMBO_PLAN.md` 0.5절에 이미 기록된 전례).
실제 재생 여부 판단은 `.animnotify`의 HIT notify와 사용자의 인게임 확인을 우선했다.

## 2. 확정 수정

| skillId | 표시 이름 | 이전 clips | 이후 clips | 근거 |
|---|---|---|---|---|
| 34120 | 연환섬 (Q, 난무) | `threetalonstrike_01` | `_01, _02, _03` | 3개 모두 base 클립(비-`_custom_N`)이며 각 1 HIT notify. 한 번 입력으로 끝까지 자동 재생되던 체인이 1클립뿐이라 1타에서 멈췄던 것. 참고데이터엔 `_04`가 존재하지 않아 3클립이 상한. |
| 34110 | 반월섬 (A, 난무) | `crescentsweep, crescentsweep_custom_5` | `crescentsweep` | `_custom_5`는 다른 스킬들과 동일한 트라이포드 명명 규칙(`_custom_N`)이라 사용자가 트라이포드 전용으로 확인. 트라이포드 없는 기본형은 `crescentsweep` 1클립만 재생해야 함. |
| 34590 | 적룡포 (S, 집중, 홀딩) | `start, loop, end, end` | `start, loop, end` | `end`가 배열에 두 번 등록돼 있어 `Update_Chain`이 끝난 뒤 다시 재생, "후속 공격 클립이 또 나간다"는 증상과 정확히 일치. 중복 제거로 1회만 재생. |

세 항목 모두 `PLAYER_SKILL_KIND::ACTIVE`라 `AnimationSkillBindingDocument::Validate`의
COMBO 전용 clip-count 제약(`iComboStageCount`)에 걸리지 않는다. 클립 이름은 모두 기존에
이미 model에서 검증된 이름을 재사용했으므로 `availableClips` 매칭도 그대로 통과한다.

## 3. 히트 수 불일치에 대한 사용자 설명 — 기록

사용자가 확인해준 내용: `.animnotify`의 HIT notify 개수는 실제 타수가 아니라 콜라이더
생성 시점을 나타낸다. 다단히트가 한 콜라이더에 몰려 1개의 HIT notify로 잡히는 경우가
있어, 연환섬을 "4타"로 기억해도 추출 데이터에는 3 HIT notify만 보일 수 있다. 그래서
이번 수정은 notify 개수를 4로 맞추려 하지 않고 **base(비-`_custom`) 클립을 전부 체인에
포함**하는 것으로 판단 기준을 통일했다.

## 4. 보류 — 사용자가 직접 재확인/별도 작업 요청

- **철량추 (S, 난무, 34090)**: 바인딩·참고데이터 모두 `flm_sk_dragonkick` 1클립뿐이라
  현재 코드 구조상 "후속 클립"이 재생될 경로가 없다. 데이터 분석으로는 원인 불명.
  사용자가 재현 조건을 다시 확인하기로 함. 이번 커밋에서 변경 없음.
- **굉열파 (E, 집중, 34560)**: 바인딩은 `penetrationlunge_01~04` 4클립, 합산 길이가
  `actionDurationMs`(2250ms)와 정확히 일치하고 HIT notify는 전부 `_03`에 있다. 루프
  클립 자체가 데이터에 없어 "루프에 멈춘다"는 증상을 설명하지 못한다. 사용자는 트라이포드
  없는 기본형이 `_01, _04`만 재생해야 할 수도 있다고 추정했으나 본인도 애니메이션을 아직
  확인하지 못했다고 밝혀 이번 수정에서 제외. 재확인 후 별도 반영.
- **절룡세 (A, 집중, 34580) 가드→피격 후속타**: 현재는 가드 클립(`counterattack_01`) 성공
  시 즉시 공격 클립(`counterattack_02`)으로 넘어간다. "피격이 들어와야 후속타" 요구사항은
  서버의 피격 판정을 클라이언트 프레젠테이션으로 되돌려주는 새 경로가 필요해 별도 작업으로
  분리하기로 사용자와 합의.
- **적룡포 (S, 집중, 34590) 진짜 홀딩(release-시 즉시 end)**: 이번엔 중복 클립 제거로
  "후속 공격 클립이 한 번 더 나가는" 증상만 해소했다. 키를 누르고 있는 동안 loop를 유지하고
  뗀 시점에 end로 넘어가는 실제 홀딩 입력 처리는 `2026-08-04_SKILLKIND_BASIC_ATTACK_COMBO_PLAN.md`
  10절에서 이미 범위 밖으로 명시된 항목이며 이번에도 별도 작업으로 남긴다.

## 5. 검증

- `Get-Content -Raw ... | ConvertFrom-Json` — JSON parse 성공.
- 빌드·인게임 재생 확인은 진행하지 않았다(사용자 요청 시 직접 실행). 다음 검증은
  Client 빌드 후 난무 Q(연환섬)/A(반월섬), 집중 S(적룡포) 재생을 눈으로 확인하는 것.
