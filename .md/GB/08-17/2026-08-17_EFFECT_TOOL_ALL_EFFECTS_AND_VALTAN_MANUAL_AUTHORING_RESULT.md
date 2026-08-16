# 2026-08-17 All Effects 표시와 발탄 수동 저작 결과

branch: `feature/effect-tool-texture-kind-filter`
기준 계획: `2026-08-17_EFFECT_TOOL_ALL_EFFECTS_AND_VALTAN_MANUAL_AUTHORING_PLAN.md`

## 완료한 변경

### All Effects

- 빈 화면의 원인은 `visible`이나 `.unified` 이름이 아니었다.
- PlayerSkills 기본 행을 만들고도, 여섯 클래스 중 하나의 skillbinding/animevents/Product 검증이 실패하면 최종 commit 전에 반환하던 것이 직접 원인이었다.
- 이제 Q/W/E/R/T/A/S/D/F 기본 행을 먼저 설치한다. Product enrichment가 실패해도 기본 탐색 행은 남는다.
- All Effects용 cue 로드는 스킬이 실제 소유한 clip만 필터링한다. 비-퀵슬롯 HIT/Effect 행은 전체 트리를 실패시키지 않는다.
- `.animevents`는 한 번 읽고 `Load_FromText()`로 같은 텍스트를 재사용한다.
- 기존 saved authored candidate 목록은 저작용 선택 목록으로 유지한다. 이 목록은 Product admission이나 Track A 하네스를 실행하지 않는다.

### 발탄 수동 저작

- Boss Pattern Effect tree의 `420633 한 행만 허용` 검사를 N행 저작 목록으로 일반화했다.
- `All Effects > Valtan > Saved Valtan Authoring`에서 `effect.valtan.*` 저장본을 Product mapping 전에도 열 수 있다.
- Open 시 full Valtan Model View target을 먼저 선택하므로 몸체·갑옷·소켓 도끼를 기준으로 작업을 이어갈 수 있다.
- 정확히 3개 `b_effectroot` carrier를 요구하는 transform-history 검증은 기존 `effect.valtan.pattern.420633.active` canary에만 남겼다. 새 수동 V0 Effect에는 강제하지 않는다.

## Claude 성능 작업과 동기화 상태

현재 브랜치에는 `4f8a3951 perf(effect): seal compacted runtime Effect documents`가 포함돼 있다.
sealed runtime Effect 합계는 문서 기준 `122.2 MB -> 54.8 MB`, 최대 단일 문서는 `6.2 MB -> 2.9 MB`로 줄었다.

`main`의 `f97653fa`/merge `6ac15ad4`에는 더 넓은 loading prewarm, metadata-only authored index, 지연 document decode가 있다. 현재 브랜치의 조상은 아니며, 공유 worktree의 `Level_CharacterSelect/MainApp/Effect_Tool.h` 미커밋 변경과 겹쳐 이번 작업에서 임의 merge하지 않았다. 이번 변경은 그 코드를 복제하지 않고 All Effects의 중복 파일 읽기만 공통 텍스트 파서로 맞췄다.

## 검증 결과

```text
Client x64 Debug 컴파일/링크  PASS
출력                            Client/Bin/Debug/Client.exe
컴파일 오류 / assertion        0 / 실행하지 않음
하네스 / publisher              실행하지 않음 (사용자 요청)
git diff --check                내용 오류 없음, 기존 LF->CRLF 경고만 있음
```

Client/UI는 에이전트가 실행하지 않았고 시각 결과도 판정하지 않았다.

## 사용자가 바로 확인할 경로

1. 새 `Client/Bin/Debug/Client.exe`로 다시 실행한다.
2. `F1 > Effect Tool > All Effects`에서 Lance Master 등을 선택한다.
3. Q/W/E/R/T/A/S/D/F 행과 그 아래 saved authored Effect가 보이는지 확인한다.
4. `Character / Boss > Valtan > Saved Valtan Authoring`에서 `effect.valtan.*`를 연다.
5. 휠윈드는 `valtan.attack.whirlwind.active / mesh_att_battle_20_03`, 3연격 후보는 `valtan.attack.front-back-front.active / mesh_att_battle_19_06`을 선택한다.
6. 몸 중심은 `b_effectroot`, 도끼 검격은 `b_wp_r_01`을 선택한 뒤 `Play All`로 본다.

## 남은 경계

- standalone 문서는 animation clip과 pivot을 아직 저장하지 않으므로 다시 열 때 직접 선택한다.
- 기존 `valtan.test`는 ID가 `effect.valtan.*`가 아니어서 `Uncategorized`에 남는다.
- `main`의 `f97653fa` 병합은 다른 미커밋 변경을 보존한 뒤 별도 branch sync로 처리한다.
- 휠윈드·3연격 V0의 형태와 타이밍 승인은 사용자가 직접 한다.

