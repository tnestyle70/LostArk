# 2026-08-06 Warlord Effect Authoring Connection Plan

## 목표

워로드 원본 Effect 추출 결과를 Effect Tool 저작 입력으로 전달하되, 존재하지 않는
Q/W/E notify 정본을 package/object 이름으로 추측하지 않는다. 스킬 이름이 포함된 최종
연결은 기존 Animation 담당 계약인 `PlayerSkills 또는 별도 승인된 skill row ->
skillbindings -> animevents/animnotify -> Effect Document` 순서가 충족될 때만 만든다.

## 실측 기준

- 원본 ParticleSystem: 680개
- Material 참조: 883개
- Mesh: 143개, runtime cook 143개 성공
- Texture: 618개, runtime cook 618개 성공
- MaterialInstance 복구: 32개
- 지원 제외: UE3 DecalMaterial 2개
- `EngineMaterials.DefaultParticle`: UE3 engine fallback 1개
- 워로드 animation source: AnimSequence 371개, `Notifies` property 0개
- 현재 워로드 `skillbindings`, `animevents`, `animnotify`: 없음

## G1. Unbound tuning index 생성

### Input

- `Data/Effects/Imported/Warlord/Warlord.unbound-particle-resource-catalog.json`
- 외부 extraction receipt의 `runtime-cook-receipt.json`

### Output

- `Data/Effects/Imported/Warlord/Warlord.unbound-effect-draft-index.json`

### 계약

- 680개 source system을 하나도 버리지 않는다.
- 각 system의 Mesh/Texture runtime asset 후보를 receipt로만 해석한다.
- `skillBoundSourceSystemCount`는 0을 유지한다.
- source object 이름은 검색 근거이지 skill ownership 근거가 아니다.

## G2. Skill 연결 fail-closed 경계

### 현재 허용

- Effect Tool `Data Files`에서 unbound extraction reference 확인
- 원본 source system/resource 후보를 기준으로 후속 Effect Document 튜닝 준비

### 현재 금지

- `FX_PC_WGL_*` 또는 `bash`, `shield`, `spear` 같은 문자열로 skillId/Q/W/E 배정
- Warlord를 `CHARACTER_CLASS_ID`, `PlayerSkills.json`, Server roster에 추가
- 빈 notify를 만들어 연결 완료로 기록

### 최종 연결 입력

1. 승인된 Warlord skill row: stable skillId, inputSlot, displayName
2. `Warlord.skillbindings.json`: skillId별 실제 model clip 순서
3. `Warlord.animnotify` 또는 승인된 `.animevents`: clip-local effect cue

세 입력이 들어오면 기존 class 공용 extractor로 skill inventory/source receipt/imported
draft를 만들고, Effect Tool에는 `Q | 스킬명` 형태로 표시한다.

## G3. 검증

1. unbound index summary가 source 680, resource miss 0, skill bound 0인지 확인한다.
2. JSON parse를 실행한다.
3. 관련 Python harness를 실행한다.
4. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`과 `git diff --check`를 실행한다.

## 완료 경계

이번 변경의 완료는 “추출 리소스 전수 전달 + 추측 연결 차단”이다. 워로드 스킬 이름과
Q/W/E 연결 자체는 위 세 정본이 없는 현재 상태에서 완료로 기록하지 않는다.
