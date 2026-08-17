# 2026-08-18 발탄 페이즈 트리 · 재시딩 Codex 인계 프롬프트

아래 블록 전체가 Codex에게 그대로 전달할 프롬프트다.

---

## 프롬프트 시작

너는 LostArk 저장소에서 이미 80% 진행된 작업의 **남은 마무리**를 한다.
새 설계를 하지 말고, 아래에 적힌 것만 정확히 끝내라.

### 0. 먼저 읽을 것

```text
AGENTS.md
CLAUDE.md
.md/GB/gotchas.md                (특히 §12 Effect 복원에서 비싸게 배운 것)
.md/GB/gotchas.local.md          (있으면)
.md/TEAM/README.md
.md/GB/08-18/2026-08-18_VALTAN_PHASE_PATTERN_EFFECT_TREE_AND_RESEED_IMPLEMENTATION_PLAN.md
```

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
작업 디렉터리: `C:\Users\user\Desktop\LostArk`

### 1. 목표 한 줄

All Effects의 Valtan을 `페이즈 → 패턴 → 애니메이션(stage) → 이펙트` 4단 트리로 만들고,
slot 5장 상한으로 버려졌던 발탄 Effect 후보를 emitter 단위로 전부 회수해
차원술사·워로드처럼 크기와 생성 위치를 손으로 튜닝할 수 있는 초기 상태로 만든다.

### 2. 이미 끝난 것 — 다시 하지 말 것

| 항목 | 파일 | 상태 |
|---|---|---|
| 텍스처 슬롯 5 → 8 (`base2`/`mask2`/`noise2`) | `Effect_AuthoringDocument.h`, `Effect_MaterialTemplate.h`, `Shader_EffectCommon.hlsli`, `Effect_DocumentRenderer.h/.cpp`, `Effect_DocumentCodec.cpp`, `Effect_Tool.cpp` | 완료 |
| `unboundResources` element 필드 (struct + allow-list + reader + writer) | `Effect_AuthoringDocument.h`, `Effect_DocumentCodec.cpp` | 완료 |
| 시더 emitter 단위 재작성 | `Tools/EffectPipeline/build_valtan_stage_effects.py` | 완료 |
| 발탄 문서 재시딩 | `Data/Effects/Authored/effect.valtan.*.effect.json` 99개 | 완료 |
| `VALTAN_PHASE_VIEW` / `VALTAN_STAGE_EFFECT_VIEW` 헤더 계약 | `Client/Public/ValtanPatternTree.h` | 완료 |
| All Effects 4단 트리 렌더 + stage 행 Open/Create + clip 자동 재생 | `Client/Public/Effect_Tool.h`, `Client/Private/Effect_Tool.cpp` | 완료 |

재시딩 실측이다. 이 수치를 근거 없이 바꾸지 마라.

```text
                  이전       현재
documents           73         99
elements           188      3,098   (mesh 보유 1,940)
버려진 후보        269          0
unbound 기록         -        217   (문서에 이름으로 보존)
slot 사용   base 2597  noise 1659  mask 1201  emissive 772  dissolve 628
            base2 806  mask2 294   noise2 902  meshModel 1940
```

`effect.valtan.pattern.420633.active`는 runtime catalog / patterneffects binding /
Product canary 세 곳이 참조하므로 시더가 제외했다. **덮어쓰지 마라.**

### 3. 남은 작업 A — 빌드를 막는 선행 에러 (다른 세션의 미커밋 변경)

`Client/Public/Effect_MaterialTemplate.h`가 지금 컴파일되지 않는다.

```text
Effect_MaterialTemplate.h(61,45): error C2131: 식이 상수로 계산되지 않았습니다
Effect_MaterialTemplate.h(61,46): error C2078: 이니셜라이저가 너무 많습니다
```

원인은 `EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS`가 `std::array<std::string_view, 21u>`인데
실제 항목이 **22개**인 것이다. 문자열 리터럴 19개 + 식별자 3개
(`EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID`,
`EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID`,
`EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID`)라서 눈으로 세면 놓친다.

이것은 세션 시작 시점에 이미 ` M`이던 **다른 세션의 미커밋 변경**이다
(`glasshole-02 / fluidninja-01 / customparticle-01 / crackholev2-01 / simple-01`
5개를 추가하면서 크기를 `17u -> 21u`로만 올림).

```text
수정   21u -> 22u   그 한 곳만
금지   배열 항목 추가/삭제, 다른 줄 정리, 그 세션의 다른 변경 되돌리기
```

RESULT에 "다른 세션 변경의 크기 불일치를 한 글자 고쳤다"고 명시하라.

### 4. 남은 작업 B — `Client/Private/ValtanPatternTree.cpp` 패치 적용

**이 파일은 Visual Studio가 잠그고 있어 이전 세션이 쓰지 못했다.**
시작 전에 VS에서 해당 탭을 닫았는지 확인하고, 잠겨 있으면 사용자에게 알려라.
파일은 LF다. CRLF로 바꾸지 마라.

헤더(`ValtanPatternTree.h`)는 이미 새 계약으로 바뀌어 있고 `.cpp`만 옛 계약이라
지금 상태로는 빌드되지 않는다. 아래 세 곳을 정확히 적용하라.

#### B-1. `Get_EffectDocumentCount()` 추가

`std::string Client::CValtanPatternTree::Build_StageEffectAssetId(` **바로 앞**에 삽입한다.

```cpp
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectDocumentCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.Effects.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}
```

#### B-2. stage당 Effect 문서를 **여러 개** 잡도록 교체

기존 블록(`const auto EffectIterator =` 부터 naming-rule fallback `if` 끝까지)을
아래로 통째로 바꾼다. 핵심은 binding이 있어도 naming rule을 **건너뛰지 않는 것**이다.
그래야 `VALTAN_WHIRLWIND / SPIN`에서 420633 canary와 시딩된
`effect.valtan.whirlwind.active`가 **둘 다** 보인다.

```cpp
				const auto EffectIterator =
					EffectByAction.find(Stage.strActionId);
				if (EffectIterator != EffectByAction.end() &&
					!EffectIterator->second.first.empty() &&
					!EffectIterator->second.second.empty())
				{
					VALTAN_STAGE_EFFECT_VIEW Bound;
					Bound.strEffectAssetId = EffectIterator->second.first;
					Bound.DocumentPath = CProjectDataRoot::Get() /
						std::filesystem::path(
							EffectIterator->second.second).lexically_normal();
					Bound.eOrigin =
						VALTAN_STAGE_EFFECT_ORIGIN::PATTERN_EFFECT_BINDING;
					Stage.Effects.push_back(std::move(Bound));
				}
				/* A stage document seeded by the generator is not in
				   patterneffects.json and never will be, because that schema
				   requires per-binding source evidence. The naming rule is
				   therefore always checked too, and both documents stay
				   visible instead of one hiding the other. */
				const std::string strCandidate = Build_StageEffectAssetId(
					Pattern.strActionId, Stage);
				const bool_t bCandidateAlreadyBound = std::any_of(
					Stage.Effects.begin(), Stage.Effects.end(),
					[&strCandidate](const VALTAN_STAGE_EFFECT_VIEW& Effect)
					{
						return Effect.strEffectAssetId == strCandidate;
					});
				if (!strCandidate.empty() && !bCandidateAlreadyBound)
				{
					const std::filesystem::path Candidate =
						CProjectDataRoot::Resolve(
							std::filesystem::path(L"Effects") / L"Authored" /
							std::filesystem::path(
								strCandidate + ".effect.json"));
					std::error_code FileError;
					if (!Candidate.empty() && std::filesystem::exists(
							Candidate, FileError))
					{
						VALTAN_STAGE_EFFECT_VIEW Seeded;
						Seeded.strEffectAssetId = strCandidate;
						Seeded.DocumentPath = Candidate;
						Seeded.eOrigin = VALTAN_STAGE_EFFECT_ORIGIN::NAMING_RULE;
						Stage.Effects.push_back(std::move(Seeded));
					}
				}
```

#### B-3. 페이즈 구간 파생과 intro 해석

기존 꼬리 블록(`OutView = std::move(Staged);` 부터 `return true;` 까지)을 아래로 바꾼다.

```cpp
	/* A phase is the band of health bars that ends when its gimmick fires.
	   The encounter document has no phase field, so this band is derived from
	   triggerHealthBar and used only as a display grouping. */
	int32_t iTopHealthBar = 0;
	const auto RaiseTop = [&iTopHealthBar](
		const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iTopHealthBar = (std::max)(iTopHealthBar,
				(std::max)(Pattern.iMaximumHealthBar, Pattern.iTriggerHealthBar));
		}
	};
	RaiseTop(Staged.Gimmicks);
	RaiseTop(Staged.Rotation);

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTop = iTopHealthBar;
	for (size_t iGimmick = 0u; iGimmick < Staged.Gimmicks.size(); ++iGimmick)
	{
		const VALTAN_PATTERN_VIEW& Gimmick = Staged.Gimmicks[iGimmick];
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = (std::max)(iBandTop, Gimmick.iTriggerHealthBar);
		Phase.iBandBottomHealthBar = Gimmick.iTriggerHealthBar;
		Phase.strGatePatternId = Gimmick.strPatternId;
		Phase.iGateTriggerHealthBar = Gimmick.iTriggerHealthBar;
		Phase.GimmickIndices.push_back(iGimmick);
		Staged.Phases.push_back(std::move(Phase));
		iBandTop = Gimmick.iTriggerHealthBar - 1;
	}
	if (iBandTop >= 1)
	{
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = iBandTop;
		Phase.iBandBottomHealthBar = 1;
		Staged.Phases.push_back(std::move(Phase));
	}

	/* A rotation pattern belongs to every band its own bar range overlaps.
	   Most are 1-160 and therefore appear in all of them; two are narrower
	   and that is exactly the information the band grouping exposes. */
	for (VALTAN_PHASE_VIEW& Phase : Staged.Phases)
	{
		for (size_t iRotation = 0u; iRotation < Staged.Rotation.size();
			++iRotation)
		{
			const VALTAN_PATTERN_VIEW& Rotation = Staged.Rotation[iRotation];
			if (Rotation.iMinimumHealthBar > Phase.iBandTopHealthBar ||
				Rotation.iMaximumHealthBar < Phase.iBandBottomHealthBar)
			{
				continue;
			}
			Phase.RotationIndices.push_back(iRotation);
		}
	}

	const std::string strIntroPatternId = Read_String(
		Encounter, "introPatternId");
	if (!strIntroPatternId.empty())
	{
		const auto Intro = std::find_if(
			Staged.Rotation.begin(), Staged.Rotation.end(),
			[&strIntroPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strIntroPatternId;
			});
		if (Intro != Staged.Rotation.end())
		{
			Staged.iIntroRotationIndex = static_cast<size_t>(
				std::distance(Staged.Rotation.begin(), Intro));
		}
	}

	OutView = std::move(Staged);
	strOutStatus = "Valtan tree: " +
		std::to_string(OutView.Phases.size()) + " phases, " +
		std::to_string(OutView.Get_PatternCount()) + " patterns, " +
		std::to_string(OutView.Get_StageCount()) + " stages, " +
		std::to_string(OutView.Get_EffectCount()) + " with an Effect (" +
		std::to_string(OutView.Get_EffectDocumentCount()) + " documents).";
	return true;
```

`<algorithm>`은 이미 include돼 있다. `(std::max)` 괄호는 Windows `max` 매크로 회피용이니 유지하라.

#### B-4. 기대 파싱 결과

```text
Valtan tree: 9 phases, 32 patterns, 121 stages, 73 with an Effect (74 documents).

PHASE 1   bar 160..159   gate VALTAN_ARMOR_BREAK_OPENING    @159
PHASE 2   bar 158..115   gate VALTAN_FLOOR_WIPE_130         @115   <- 빨간 발판 전멸기
PHASE 3   bar 114..109   gate VALTAN_ARENA_BREAK_109        @109
PHASE 4   bar 108..100   gate VALTAN_FOUR_PILLARS_105       @100
PHASE 5   bar  99..73    gate VALTAN_MAGIC_ORB_STAGGER_76   @ 73
PHASE 6   bar  72..62    gate VALTAN_CENTER_GRAB_COUNTER_64 @ 62
PHASE 7   bar  61..30    gate VALTAN_ARENA_BREAK_33         @ 30
PHASE 8   bar  29..14    gate VALTAN_GHOST_TRANSITION_15    @ 14
PHASE 9   bar  13..1     gate 없음 (망령 구간)
```

`73 with an Effect / 74 documents`의 차이는 `VALTAN_WHIRLWIND / SPIN` 한 stage가
문서 두 개(420633 canary + `whirlwind.active`)를 갖기 때문이다. 정상이다.

### 5. 남은 작업 C — 빌드와 잔여 에러 수정

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

Engine public header는 바뀌지 않았으므로 `UpdateLib.bat`은 필요 없다. 다만
정본 스크립트가 그 순서를 포함하므로 그대로 쓰는 편이 안전하다.

A와 B를 적용하기 전에는 모든 TU가 헤더에서 멈춰 **아래 코드가 한 번도 컴파일된 적이 없다.**
따라서 새 컴파일 에러가 나오는 것이 정상이다. 나오면 고쳐라. 확인된 위험 지점은 다음이다.

```text
Effect_Tool.cpp   Render_ValtanPatternTreeSection / Render_ValtanPatternNode /
                  Render_ValtanStageRow / Create_ValtanStageEffectDocument /
                  Play_ValtanStageClip 5개 신규 정의
                  sprintf_s 사용 -> <cstdio> 필요할 수 있음
                  std::set / std::less<> 사용 -> <set> 이미 include됨
Effect_Tool.cpp   Render_ValtanAuthoringOpenButton 이 인자 3개로 바뀜.
                  기본값 있는 세 번째 인자라 기존 호출부는 그대로 컴파일돼야 한다.
Effect_DocumentRenderer.cpp   Textures 배열 5 -> 8 이후 Texture_Index 경계
```

**임의로 기능을 되돌려 빌드를 통과시키지 마라.** 에러는 원인을 고친다.

### 6. 슬롯 확장 규칙 — 명시적 허가

사용자 승인 사항이다.

```text
텍스처 슬롯은 기존 4캐릭터(Artist / LanceMaster / DimensionMaster / Warlord)가
쓰는 표준 material template 방식 그대로 확장해도 된다.
별도 계약이나 두 번째 template 계열을 새로 만들지 않는다.
```

즉 슬롯을 더 늘려야 할 근거가 생기면 다음을 **같은 변경 단위**로 함께 한다.

```text
1  EFFECT_RESOURCE_SLOT 에 END 앞으로 값 추가        직렬화 토큰 순서 유지
2  EFFECT_MATERIAL_INPUT_SEMANTIC 에 대응 값 추가
3  EFFECT_STANDARD_MATERIAL_INPUTS 배열 크기와 항목 추가
4  Shader_EffectCommon.hlsli 에 Texture2D 와 g_HasXxx uniform 추가
5  Shade_Effect 에 실제 합성 의미 부여                 <- 이것을 빠뜨리지 말 것
6  Effect_DocumentRenderer.h  ELEMENT_RESOURCE::Textures 배열 크기
7  Effect_DocumentRenderer.cpp Find_Texture 상한, iHasXxx, Bind_Texture
8  Effect_DocumentCodec.cpp   SLOT_TOKENS
9  Effect_Tool.cpp            AUTHORING_SLOT_CARD 2곳, SLOT_PRIORITY 2곳,
                              SUMMARY_SLOTS, Slot_Label, 슬롯 설명문
```

금지 사항이다.

```text
g_SourceTexture0..6 을 표준 슬롯으로 재사용하지 마라.
  그 7개는 Artist 31470 재구성 실행 경로 전용이고 의미가 다르다.
샘플링 의미 없이 슬롯만 늘리지 마라.
  바인딩만 되고 화면에 기여하지 않으면 unboundResources 에 기록하는 것과 같다.
Shade_GroupedTranslucent 등 source-profile 경로는 건드리지 마라.
  4직업 손튜닝 문서가 그 경로를 쓰고 있어 회귀 범위가 넓다.
```

현재 8슬롯의 합성 의미는 다음과 같다. 원본 slot 이름 토큰 실측이 근거다
(`diff_tex1`/`diff_tex2`, `opacity_tex`/`alpha_tex`, `uv_noise_tex`/`uv_noise_01_tex`).

```text
base2   base 위에 곱해지는 두 번째 diffuse
mask2   mask 에 곱해지는 두 번째 R 채널 불투명도
noise2  noise 와 평균내는 두 번째 RG 왜곡
```

### 7. 지켜야 할 경계

```text
Data/Encounters/Valtan/ValtanEncounter.json          읽기 전용. Server 권위.
Data/Animation/Authored/Valtan/Valtan.patternbindings.json   읽기 전용.
Data/Animation/Authored/Valtan/Valtan.patterneffects.json    행을 추가하지 마라.
    이 스키마는 binding 마다 sourceEvidence / sourceBranch / modelBoneEvidence /
    sourceOccurrences / productAdmission 전부와 sha256 receipt를 required로 요구한다.
    121행을 만들려면 증거를 지어내야 하고 그것이 gotchas §12.6이 경고한 함정이다.
    트리는 stage actionId + 명명 규칙 + 파일 실재 확인으로 조인한다. 이미 그렇게 돼 있다.

Data/Effects/EffectCatalog.json    발탄 행을 추가하지 마라.
    현재 203행은 4직업뿐이고 발탄은 0행이다. 이번 범위의 판정 화면은 Effect Tool
    preview이고 게임 내 재생은 별도 수직 슬라이스다. gotchas §12.3 참조.

effect.valtan.pattern.420633.active   이름 변경 금지, 덮어쓰기 금지.

C++ 인코딩   파일별 기존 인코딩을 감지해 그대로 보존한다.
             ValtanPatternTree.cpp 는 LF, Effect_Tool.cpp 는 CRLF다.

다른 세션의 미커밋 변경   §3의 21u -> 22u 외에는 손대지 마라.
```

### 8. 검증 — 실행한 것과 안 한 것을 분리해 보고

```text
자동
  Client x64 Debug 빌드 성공
  git diff --check
  python -c "import json,glob; [json.load(open(f,encoding='utf-8')) for f in glob.glob('Data/Effects/Authored/effect.valtan.*.effect.json')]"
  python Tools/EffectPipeline/build_valtan_stage_effects.py     (dry run, 99/3098 재확인)
  python Tools/EffectPipeline/validate_boss_pattern_effects.py  (patterneffects 계약 무결)
  트리 파싱 로그가 §B-4의 9 phases / 32 patterns / 121 stages 를 낸다

사용자 전용 — 에이전트가 대신 판정하지 마라
  Client 실행, F1 > Effect Tool > All Effects > Valtan 선택
  PHASE 2 > VALTAN_FLOOR_WIPE_130 > FIRST_SMASH > [Open]
  발탄이 서고 mesh_att_battle_1_02 가 재생되는지
  element 가 1% 가 아니라 눈에 보이는 크기로 뜨는지
  Effect Detail 의 Transform scale / position 드래그가 preview 에 즉시 반영되는지
```

에이전트는 Client나 UI를 자율 실행·조작하지 않고 화면을 캡처하지 않는다.
빌드 · 로그 · 수치 진단까지 하고, 사용자가 누를 정확한 경로를 알린 뒤 멈춘다.

### 9. 마지막 산출물

`.md/GB/08-18/2026-08-18_VALTAN_PHASE_PATTERN_EFFECT_TREE_AND_RESEED_RESULT.md`를 쓴다.

```text
구현 완료   실제 diff 기준
자동 검증   실행한 명령과 결과
수동 검증   미실행. 사용자 판정 대기라고 명시
남은 경계   EffectCatalog 미등록, patterneffects 121행 미작성, 6방향 판정은 Server 슬라이스,
            22 stage 는 clip 에 붙은 원본 system 이 없어 Create Effect 행으로 남음
```

계획서에 적혔다는 이유로 구현 완료 처리하지 마라.
빌드가 통과하지 않았으면 통과하지 않았다고 쓴다.

## 프롬프트 끝
