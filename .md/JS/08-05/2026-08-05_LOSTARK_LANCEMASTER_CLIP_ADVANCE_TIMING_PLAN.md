# 연환섬 등 다단 클립 체인의 조기 전환(cancel 시점) 도입 — 결정 대기

작성자: JS · 2026-08-05 · 브랜치 `feature/lancemaster-skill-slots`

`2026-08-05_LOSTARK_LANCEMASTER_ANIMATION_CHAIN_FIX_RESULT.md`에서 연환섬(Q, 34120)을
`threetalonstrike_01,02,03` 3클립으로 늘렸는데도 실제로는 3타까지 안 가고 중간에 끊긴다는
재보고를 받았다. 원인을 실측했고, 코드로 옮기기 전 저장 방식과 데이터 출처를 사용자가
결정해야 해서 오늘은 문서만 남기고 구현은 다음 세션으로 미룬다.

## 0. 실측 — 왜 3타까지 안 가는가

`CCharacter::Update_Chain()`(`Client/Private/Character.cpp:213`)은 현재 클립이
`Is_ClipFinished()`(같은 파일 171행, track position이 clip 전체 길이에 도달했는지만 본다)를
만족해야만 다음 클립으로 넘어간다. 즉 매 클립을 원본 길이 그대로 끝까지 재생한 뒤에만 전환한다.

```cpp
void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;
	if (m_pChain->isCombo)
		return;
	if (m_iChainStep + 1 >= static_cast<int32_t>(m_pChain->clips.size()))
		return;
	++m_iChainStep;
	Start_Clip(m_pChain->clips[m_iChainStep].c_str());
}
```

연환섬 3클립의 원본 길이(`Data/Animation/Reference/LanceMaster/LanceMaster.animnotify`):

```text
flm_sk_threetalonstrike_01  len=1.5000
flm_sk_threetalonstrike_02  len=1.5000
flm_sk_threetalonstrike_03  len=1.8667
```

전부 원본 길이대로 이어 재생하면 1.5 + 1.5 + 1.8667 = 4.8667초가 필요하다. 그런데
`Data/Balance/PlayerSkills.json`의 34120 `actionDurationMs`는 2266ms다. 서버가 2266ms에
`player.eAction = PLAYER_ACTION_STATE::NONE`으로 액션을 끝내고, `CCharacter::Apply_NetworkAction`은
그 순간 `m_pChain`을 즉시 비우고 Idle/Run으로 되돌린다(`Character.cpp:375-383`). `_01`이
1.5초에 끝나 `_02`로 넘어간 직후, `_02` 재생 도중(2.266 - 1.5 = 0.766초 지점)에 서버가
액션을 끊어버리므로 `_03`은 시작조차 못 한다. "3번까지 안 가고 중간에 끊긴다"는 증상과 정확히
일치한다.

`actionDurationMs=2266`은 3클립 재바인딩 이전 값으로, `flm_sk_threetalonstrike_custom_3`
(트라이포드 클립, len=2.2667초)과 우연히 일치한다 — 즉 예전에 단일 트라이포드 클립 기준으로
잡아둔 값이 그대로 남아 있던 것이다.

## 1. 사용자가 제시한 해법 — cancel 이벤트 기준 조기 전환

사용자가 Animation Tool에서 직접 확인: 각 클립의 마지막 notify가 `kind=CANCEL`이며, 실제
원작은 클립을 끝까지 재생하지 않고 그 캔슬 시점에 다음 클립으로 넘어가야 한다.

`.animnotify` 상 각 클립의 마지막(시간순) notify:

```text
flm_sk_threetalonstrike_01  t=0.9000  kind=CANCEL win=NONE label=""
flm_sk_threetalonstrike_02  t=0.5820  kind=CANCEL win=NONE label=""
```

참고용 Reference 데이터이며 런타임 정본이 아니다
(`Data/Animation/Reference/LanceMaster/LanceMaster.animnotify:4335-4336, 4350`).

이 시점을 "다음 클립 시작 가능 시각"으로 써서 누적하면:

```text
_01 0.000 -> 0.900 (누적 0.900)
_02 0.900 -> 0.900+0.582=1.482 (누적 1.482)
_03 시작 1.482, 자체 HIT notify(t=0.400)가 누적 1.882에 발생
```

1.882초는 현재 `actionDurationMs`(2.266초) 안에 들어온다. `_03`의 히트가 실제로 재생된 뒤
남은 예산(2.266-1.482=0.784초) 동안 `_03`의 recovery 프레임이 재생되다가 서버가 액션을
끝내면서 마지막 자세로 멈춘다 — 이는 다른 이미 정상 동작하는 스킬들(예: 철량추가 자기
clip 길이보다 짧게 재생을 멈추는 것과 동일 패턴)과 같은 방식이라 추가로 이상하지 않다.

**결론: `actionDurationMs`를 늘릴 필요는 없다.** `Update_Chain`이 클립 전체 길이 대신
클립별 "조기 전환 시각"을 기준으로 넘어가도록만 고치면 된다.

## 2. 결정이 필요한 지점 — 다음 세션 시작 시 먼저 확인

### 2.1 조기 전환 시각을 어디에 저장할까

현재 `ANIMATION_SKILL_BINDING::Clips`(`Client/Public/AnimationSkillBindingDocument.h:21`)는
`std::vector<std::string>`이라 클립 이름만 담는다. `LanceMaster.skillbindings.json`의
`clips` 배열도 문자열 배열뿐이다. 이 스키마는 Gunslinger/Slayer/Artist/DimensionMaster까지
5개 클래스 문서가 전부 공유하므로 바꾸면 전체에 영향이 간다.

**옵션 A — skillbindings.json 스키마 확장 (권장)**
클립 항목에 `advanceAtMs`(선택 필드, 없으면 기존처럼 clip 전체 길이 사용)를 추가한다.
`AnimationSkillBindingDocument.h`의 `ANIMATION_SKILL_BINDING`에 `std::vector<uint32_t>
ClipAdvanceMs`(clips와 같은 인덱스, 0이면 "끝까지") 같은 병렬 배열 또는 clip을 struct로
승격하는 두 가지 하위 선택지가 있다. `Parse_Text`/`Validate`/`Serialize`
(`AnimationSkillBindingDocument.cpp`)와 `Update_Chain`/`Is_ClipFinished` 호출부, 그리고
`Animation_Tool.cpp`의 저장 UI까지 함께 고쳐야 한다. 재사용 가능하고 Animation Tool에서
가시적으로 관리할 수 있다는 장점이 있지만 영향 범위가 크다.

**옵션 B — 이 스킬 전용 임시 하드코딩**
`LanceMaster.skillbindings.json`은 그대로 두고 `Character.cpp`에 34120(및 필요해지는 다른
skillId) 전용 조기 전환 표를 임시로 박아 넣는다. 빠르지만 다른 스킬/클래스에 재사용 불가,
나중에 옵션 A로 옮길 때 지우고 다시 써야 한다.

### 2.2 조기 전환 숫자의 출처

**옵션 A — `.animnotify`의 CANCEL 이벤트 그대로 사용**
이미 위 1절에서 계산해뒀다. 추가 조사 없이 바로 적용 가능.

**옵션 B — `EFTable_SkillEffect.db` 우선 확인**
`lostark-lpk-container-cracked-inner-aes-wall` 메모리에 따르면 SourceData/LPK 언팩본의
SQLite에 창술사 스킬 타이밍이 있다. 공식 콤보 캔슬/전환 타이밍이 `.animnotify`의 범용
CANCEL 윈도우보다 더 정확할 수 있어 먼저 쿼리해서 값이 다르면 그쪽을 우선한다.

## 3. 이번 재보고 이후에도 남아 있는 항목 (RESULT 3~4절에서 이어옴)

- 철량추(S, 난무, 34090), 굉열파(E, 집중, 34560): 원인 불명, 사용자가 재현 조건 재확인 중.
- 절룡세(A, 집중) 가드→피격 후속타, 적룡포(S, 집중) 진짜 홀딩(release-시 즉시 end): 신규
  메커니즘, 별도 작업으로 분리 합의됨.
- 위 두 항목과 이번 조기 전환 이슈가 겹치는 지점이 있는지(예: 적룡포도 loop 클립을 조기에
  끊고 end로 넘어가야 하는가) 옵션 A/B를 확정한 뒤 함께 검토한다.

## 4. 다음 세션 시작 순서

```text
1. 2.1과 2.2의 선택지를 사용자에게 확인
2. (옵션 A 선택 시) 스키마 확장 -> Parse/Validate/Serialize -> Update_Chain -> Animation_Tool 저장 UI 순으로 전체 코드 작성
   (옵션 B 선택 시) Character.cpp에 임시 테이블만 추가
3. 34120 3클립이 실제로 3타까지 재생되는지 Client 빌드 후 인게임 확인
4. 같은 조기 전환이 필요한 다른 스킬이 있는지 사용자와 함께 스캔
5. RESULT 갱신, 하네스/ProjectAudit 실행
```
