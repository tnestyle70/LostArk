# 2026-08-30 Effect Tool v2 Group 문서 — 이펙트 묶음·스폰/소멸 타이밍 컨테이너 PLAN

작성자: JS · branch `feature/effect-v2-multiply-blend` 위에서 시작 (main `9e24aaee` 기준, Multiply 블렌드 변경 미커밋 상태)
선행: `../08-28/2026-08-28_EFFECT_V2_VALTAN_ATTACH_PLAN.md`(EFFECT_V2_TARGET, stage 바인딩), `../08-27/2026-08-27_EFFECT_V2_SOFT_PARTICLE_PLAN.md`
후속 목표: 발탄 유령 포탈(`VALTAN_GHOST_FINALE` leg 8개)을 원작 이미터 7종 → v2 문서 7개 + Group 1개 + 바인딩 8줄로 재현

## 목표

v2 이펙트 문서(이미터 1개 = `.effectv2.json` 1개) 여러 개를 **Group 문서 하나**로 묶고, 그룹이 자식별
`startMs / durationMs / stop(Kill|Deactivate) / offset / yaw`를 소유하게 한다. 바인딩은 `effectId` 대신
`group`을 가리킬 수 있고, 런타임은 바인딩 lane(clip 또는 Server stage) 시계 위에서 자식을 펼쳐 스폰·정지한다.
툴에는 Group 창(자식 편집, 타임라인 바, 자유 시계 미리보기)을 추가한다. UE3 `ParticleSystem → Emitter`의
2단 구조를 그대로 옮기는 것이며 중첩(그룹 안의 그룹)은 허용하지 않는다.

Server·Shared·팀장 Effect V1 경로 변경 없음. 새 C++ 파일 없음(vcxproj/filters 변경 없음).

## 실측 근거 (2026-08-30)

- 바인딩 런타임은 `CEffectV2Runtime`(EffectV2_Runtime.cpp, 448줄)이 lane 2개를 돈다: clip lane
  `Notify_Clip → Tick`(모델 클립 시각), stage lane `Sync_Stage`(Server stage actionId + 나이 초). 두 lane 모두
  `PENDING_SPAWN{Binding, bSpawned}` 목록을 `iStartMs`로 스폰하고 `Prune_Spawned`가 `stopWithClip`/완료를 정리한다.
  `Spawn()`은 `Resolve_TargetPivot(View, bone, rotation)`으로 pivot을 만들고 `bFollowBone`이면
  `Set_FollowTarget`을 건다(EffectV2_Runtime.cpp:157-220).
- `CEffectV2Object::Update`는 follow 중이면 매 프레임 `Resolve_TargetPivot`이 `m_PivotWorld`를 **덮어쓴다**
  (EffectV2_Object.cpp:673-683). 따라서 자식 offset을 pivot에 미리 곱해도 follow 시 사라진다 → follow-local 행렬이 필요.
- 이미터 정지는 두 상태다: `m_bEmissionStopped`(파티클/트레일은 스폰만 멈추고 잔여 소멸 후 `m_bFinished`)와
  `m_bFinished`(즉시 제거). `Advance_Lifetime`이 lifetime 만료 시 shape별로 이 둘을 고른다(EffectV2_Object.cpp:712-729).
  외부에서 호출할 public `Stop_Emission()`은 없다 — `Finish()`만 있다(EffectV2_Object.h:326-327).
- 바인딩 문서 파서 `Parse_Bindings`는 `effectId` 필수, `clip`/`stage` 중 정확히 하나, `startMs` 0~600000,
  `bone`, 선택 `followBone/rotation/stopWithClip`, 중복은 (effectId, clip, stage) 기준(EffectV2_Document.cpp:582-666).
- 문서 경로는 `CProjectDataRoot::Resolve(L"Effects/V2/Authored|Bindings")`(EffectV2_Document.cpp:196-214).
  `Data/Effects/V2`에는 `Authored`, `Bindings` 두 폴더만 있다.
- 툴 Attach 창의 바인딩 섹션은 `m_szEffectId`(Document 패널의 현재 ID)를 effectId로 쓰고, stage 바인딩은
  Valtan 타임라인이 활성일 때 `m_iValtanSpawnTimelineMs`로 stage/offset을 고른다(Effect_Tool_V2.cpp:2271-2387).
  Attach 창 토글은 Create 패널의 `Attach...` 버튼(1016-1017), 매 프레임 갱신은 `Update_Attach`(1544).
- 검증기 `Tools/EffectToolV2/validate_effect_v2.py`는 Authored/Bindings만 검사하고 정본 회귀
  `Invoke-BuildAndRegression.ps1:158`이 `test_validate_effect_v2.py`를 python gate로 돌린다. 현재 규칙: 모든
  authored 문서는 어떤 바인딩에든 묶여 있어야 한다(`unbound Effect V2 authored effects`).
- 원작 포탈 데이터(`68cabd25:Data/Effects/Imported/Valtan/PortalRush/…imported-candidate.effect.json`)는
  이미터별 `emitterdelay`(0.75/0.81 s), `emitterloops 1`, 파티클 lifetime을 가진다 → Group child의
  `startMs/durationMs`로 1:1 대응된다.
- 인코딩: `EffectV2_*`, `Effect_Tool_V2.*`, validator/test 모두 ASCII/UTF-8.

## 계약 (결정)

| 항목 | 값 | 이유 |
|---|---|---|
| 그룹 문서 | `Data/Effects/V2/Groups/<groupId>.effectv2group.json`, schema `lostark.effect-v2-group` v1 | Authored/Bindings와 같은 폴더 규칙, 파일명 = ID |
| 그룹 ID | `Is_ValidEffectId`와 같은 규칙, authored effectId와 **충돌 금지**(validator) | 바인딩 row에서 두 이름공간이 섞이지 않게 |
| 자식 | `{effectId, startMs, durationMs(0=자식 문서 lifetime), stop: Kill\|Deactivate, offset[3] m, yawDegrees}` | 원작 `emitterdelay/emitterduration` 1:1 |
| 그룹 `durationMs` | 0 = 마지막 자식 종료까지, >0 이면 모든 자식 stop 시각의 상한 | 포탈처럼 stage 길이에 맞춰 잘라내기 |
| 중첩 | 자식은 authored 문서만. 그룹 안의 그룹은 validator가 거부, 런타임은 문서 로드 실패로 Report | UE3와 같은 2단, 사이클 검사 불필요 |
| 바인딩 | row에 `effectId` **또는** `group` 정확히 하나. 나머지 필드(clip/stage/startMs/bone/followBone/rotation/stopWithClip)는 그룹 전체에 한 번 적용 | 바인딩 formatVersion 1 유지, 기존 파일 무변경 |
| 자식 시각 | lane 시각 = `binding.startMs + child.startMs`; stop = `+ child.durationMs`(0이면 없음), 그룹 `durationMs`가 있으면 `binding.startMs + durationMs`로 clamp | 시계는 lane 하나뿐 |
| stop 정책 | `Kill` → `Finish()`(즉시 제거). `Deactivate` → 파티클/트레일은 스폰 중단 후 잔여 소멸, 그 외 shape는 `Finish()`와 동일 | 현업 Deactivate 의미 |
| 자식 배치 | `Local = RotY(yaw) × Translate(offset)`; pivot = `Local × 바인딩 pivot`. follow 시에도 매 프레임 `Local × bone pivot` | 자식 offset이 follow에서 사라지지 않게 |
| stopWithClip | 바인딩 값이 자식 전부에 복사됨(기존 Prune 로직 그대로) | 그룹은 시계·배치만 소유 |
| 툴 미리보기 | 런타임의 free-group lane(`Play_Group/Advance_FreeGroups`)을 사용. 툴이 별도 스폰 구현을 갖지 않는다 | 두 번째 재생 경로 금지 |
| 미리보기 pivot | 살아 있는 preview 이펙트가 있으면 그 `PivotWorld`, 없으면 타깃의 `(m_strPivotBone, m_ePivotRotation)` pivot | 사용자가 preview를 놓은 자리에서 재생 |
| validator | 그룹 문서 검사 + 바인딩 `group` 참조 검사. 그룹으로 묶인 자식은 bound로 인정. 미바인딩 그룹은 authored와 같은 규칙으로 거부 | 기존 fail-closed 규칙과 일관 |
| 팀장 합의 | v2 런타임(B안) 경로 안의 확장. Effect V1/`CEffectCatalog` 무관 | — |

## 파일 목록

| 구분 | 경로 | 역할 |
|---|---|---|
| 수정 | `Client/Public/EffectV2_Document.h` | `EFFECT_V2_CHILD_STOP`, `EFFECT_V2_GROUP_CHILD`, `EFFECT_V2_GROUP`, 바인딩 `strGroupId`, Group 파일 API |
| 수정 | `Client/Private/EffectV2_Document.cpp` | Group 경로/파서/직렬화, 바인딩 `group` 키 |
| 수정 | `Client/Public/EffectV2_Object.h`, `Client/Private/EffectV2_Object.cpp` | `Stop_Emission()`, `Set_FollowLocal()`, follow-local 적용 |
| 수정 | `Client/Public/EffectV2_Runtime.h`, `Client/Private/EffectV2_Runtime.cpp` | 그룹 전개, 자식 stop, free-group lane |
| 수정 | `Client/Public/Effect_Tool_V2.h`, `Client/Private/Effect_Tool_V2.cpp` | Group 창, 바인딩 `Bind As` |
| 수정 | `Tools/EffectToolV2/validate_effect_v2.py`, `test_validate_effect_v2.py` | 그룹 검증·테스트 |
| 추가(데이터, 후속) | `Data/Effects/V2/Groups/boss.valtan.portal.effectv2group.json` | 포탈 그룹 — 자식 문서 7개를 먼저 저작한 뒤 |

## 문서 예시

```json
{
  "schema": "lostark.effect-v2-group",
  "formatVersion": 1,
  "groupId": "boss.valtan.portal",
  "durationMs": 0,
  "children": [
    { "effectId": "boss.valtan.portal.flash", "startMs": 750, "durationMs": 500, "stop": "Kill", "offset": [0.0, 0.0, 0.0], "yawDegrees": 0.0 },
    { "effectId": "boss.valtan.portal.card", "startMs": 810, "durationMs": 0, "stop": "Kill", "offset": [0.0, 0.0, 0.0], "yawDegrees": 0.0 },
    { "effectId": "boss.valtan.portal.flame", "startMs": 810, "durationMs": 1500, "stop": "Deactivate", "offset": [0.0, 0.0, 0.0], "yawDegrees": 0.0 },
    { "effectId": "boss.valtan.portal.dark", "startMs": 810, "durationMs": 0, "stop": "Deactivate", "offset": [0.0, -0.05, 0.0], "yawDegrees": 0.0 }
  ]
}
```

바인딩 row (`BOSS_VALTAN.effectv2bindings.json`):

```json
{ "group": "boss.valtan.portal", "stage": "valtan.sequence.ghost-finale.step-02", "startMs": 0, "bone": "", "followBone": false, "rotation": "TargetYaw", "stopWithClip": true }
```

## G1. 문서 계층 — Group 파일과 바인딩 `group` 키

### 1-1. `Client/Public/EffectV2_Document.h`

적용 위치 1: `struct EFFECT_V2_BINDING final` 블록 전체(`/* Exactly one of strClip …` 주석 포함, `};`까지)를 다음으로 교체

```cpp
struct EFFECT_V2_BINDING final
{
	/* Exactly one of strEffectId (leaf document) or strGroupId (group
	   document) names what spawns. */
	std::string strEffectId;
	std::string strGroupId;
	/* Exactly one of strClip (model clip start clock) or strStage (Server
	   pattern stage actionId, stage-local clock) keys the binding. */
	std::string strClip;
	std::string strStage;
	uint32_t iStartMs = 0u;
	std::string strBone;
	bool_t bFollowBone = true;
	CEffectV2Object::PIVOT_ROTATION eRotation = CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
	bool_t bStopWithClip = false;
};

enum class EFFECT_V2_CHILD_STOP : int32_t
{
	KILL,
	DEACTIVATE,
	END
};

/* One emitter slot of a group. Times are group-local milliseconds; the
   binding that plays the group adds its own startMs. iDurationMs 0 leaves
   the child to its document lifetime. vOffset/fYawDegrees place the child
   relative to the group pivot and survive bone following. */
struct EFFECT_V2_GROUP_CHILD final
{
	std::string strEffectId;
	uint32_t iStartMs = 0u;
	uint32_t iDurationMs = 0u;
	EFFECT_V2_CHILD_STOP eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE;
	float3_t vOffset = { 0.f, 0.f, 0.f };
	f32_t fYawDegrees = 0.f;
};

/* A group owns clock and placement only; every look comes from the child
   documents. iDurationMs 0 ends with the last child, otherwise it caps
   every child stop. Children must be leaf documents (no nesting). */
struct EFFECT_V2_GROUP final
{
	std::string strGroupId;
	uint32_t iDurationMs = 0u;
	std::vector<EFFECT_V2_GROUP_CHILD> Children;
};
```

적용 위치 2: `static std::filesystem::path Binding_Directory();` 바로 아래

```cpp
	static std::filesystem::path Group_Directory();
```

적용 위치 3: `static std::filesystem::path Binding_Path(const std::string& strArchetypeId);` 바로 아래

```cpp
	static std::filesystem::path Group_Path(const std::string& strGroupId);
```

적용 위치 4: `static bool_t Parse_Bindings(...)` 선언 끝(`std::string& strOutError);`) 바로 아래

```cpp
	static bool_t Parse_Group(
		const std::string& strText,
		EFFECT_V2_GROUP& OutGroup,
		std::string& strOutError);
```

적용 위치 5: `static std::string Serialize_Bindings(...)` 선언 끝 바로 아래

```cpp
	static std::string Serialize_Group(const EFFECT_V2_GROUP& Group);
```

적용 위치 6: `static bool_t Load_BindingsFile(...)` 선언 끝 바로 아래

```cpp
	static bool_t Load_GroupFile(
		const std::string& strGroupId,
		EFFECT_V2_GROUP& OutGroup,
		std::string& strOutError);
```

적용 위치 7: `static const char* Rotation_Key(CEffectV2Object::PIVOT_ROTATION eRotation);` 바로 아래

```cpp
	static const char* Child_Stop_Key(EFFECT_V2_CHILD_STOP eStop);
```

### 1-2. `Client/Private/EffectV2_Document.cpp`

적용 위치 1: 익명 namespace 상수 `const char* TRAIL_EDGE_KEYS[] = …;` 바로 아래

```cpp
	const char* CHILD_STOP_KEYS[] = { "Kill", "Deactivate" };
	constexpr double MAX_BINDING_MS = 600000.0;
```

적용 위치 2: 익명 namespace 안 `bool_t Read_TextFile(...)` 정의 바로 아래(namespace 닫는 `}` 위)

```cpp
	bool_t Read_MsField(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, uint32_t& iOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 || pValue->Get_Number() > MAX_BINDING_MS)
		{
			strError = std::string(pKey) + " must be an integer in [0, 600000].";
			return false;
		}
		iOut = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}
```

적용 위치 3: `Binding_Directory()` 정의 바로 아래

```cpp
std::filesystem::path Client::CEffectV2Document::Group_Directory()
{
	return CProjectDataRoot::Resolve(L"Effects/V2/Groups");
}
```

적용 위치 4: `Binding_Path(...)` 정의 바로 아래

```cpp
std::filesystem::path Client::CEffectV2Document::Group_Path(const std::string& strGroupId)
{
	return Group_Directory() / (strGroupId + ".effectv2group.json");
}
```

적용 위치 5: `Rotation_Key(...)` 정의 바로 아래

```cpp
const char* Client::CEffectV2Document::Child_Stop_Key(const EFFECT_V2_CHILD_STOP eStop)
{
	const size_t iIndex = static_cast<size_t>(eStop);
	return iIndex < _countof(CHILD_STOP_KEYS) ? CHILD_STOP_KEYS[iIndex] : "Deactivate";
}
```

적용 위치 6: `Parse_Bindings` 정의 전체(`bool_t Client::CEffectV2Document::Parse_Bindings(` 부터 `OutBindings = std::move(Staged); return true; }`까지) 교체

```cpp
bool_t Client::CEffectV2Document::Parse_Bindings(
	const std::string& strText,
	const std::string& strExpectedArchetypeId,
	std::vector<EFFECT_V2_BINDING>& OutBindings,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Bindings root is not an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pArchetype = Root.Find("archetypeId");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.effect-v2-bindings" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pArchetype || !pArchetype->Is_String() ||
		pArchetype->Get_String() != strExpectedArchetypeId)
	{
		strOutError = "schema/formatVersion/archetypeId mismatch.";
		return false;
	}
	const DATA_JSON_VALUE* pRows = Root.Find("bindings");
	if (nullptr == pRows || !pRows->Is_Array())
	{
		strOutError = "bindings must be an array.";
		return false;
	}
	std::vector<EFFECT_V2_BINDING> Staged;
	for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
	{
		if (!Row.Is_Object())
		{
			strOutError = "bindings[] entries must be objects.";
			return false;
		}
		EFFECT_V2_BINDING Binding;
		const DATA_JSON_VALUE* pEffect = Row.Find("effectId");
		const DATA_JSON_VALUE* pGroup = Row.Find("group");
		const DATA_JSON_VALUE* pClip = Row.Find("clip");
		const DATA_JSON_VALUE* pStage = Row.Find("stage");
		const DATA_JSON_VALUE* pStart = Row.Find("startMs");
		const DATA_JSON_VALUE* pBone = Row.Find("bone");
		const bool_t bHasEffect = nullptr != pEffect && pEffect->Is_String() && !pEffect->Get_String().empty();
		const bool_t bHasGroup = nullptr != pGroup && pGroup->Is_String() && !pGroup->Get_String().empty();
		const bool_t bHasClip = nullptr != pClip && pClip->Is_String() && !pClip->Get_String().empty();
		const bool_t bHasStage = nullptr != pStage && pStage->Is_String() && !pStage->Get_String().empty();
		if (bHasEffect == bHasGroup ||
			(bHasEffect && !Is_ValidEffectId(pEffect->Get_String())) ||
			(bHasGroup && !Is_ValidEffectId(pGroup->Get_String())) ||
			bHasClip == bHasStage ||
			nullptr == pStart || !pStart->Is_Number() || pStart->Get_Number() < 0.0 ||
			pStart->Get_Number() > MAX_BINDING_MS ||
			nullptr == pBone || !pBone->Is_String())
		{
			strOutError = "bindings[] requires exactly one of effectId/group, exactly one of clip/stage, startMs (0-600000), bone.";
			return false;
		}
		if (bHasEffect)
			Binding.strEffectId = pEffect->Get_String();
		else
			Binding.strGroupId = pGroup->Get_String();
		if (bHasClip)
			Binding.strClip = pClip->Get_String();
		else
			Binding.strStage = pStage->Get_String();
		Binding.iStartMs = static_cast<uint32_t>(pStart->Get_Number());
		Binding.strBone = pBone->Get_String();
		int32_t iRotation = static_cast<int32_t>(Binding.eRotation);
		if (!Read_Bool(Row, "followBone", Binding.bFollowBone, strOutError) ||
			!Read_Enum(Row, "rotation", PIVOT_ROTATION_KEYS,
				_countof(PIVOT_ROTATION_KEYS), iRotation, strOutError) ||
			!Read_Bool(Row, "stopWithClip", Binding.bStopWithClip, strOutError))
			return false;
		Binding.eRotation = static_cast<CEffectV2Object::PIVOT_ROTATION>(iRotation);
		for (const EFFECT_V2_BINDING& Existing : Staged)
		{
			if (Existing.strEffectId == Binding.strEffectId &&
				Existing.strGroupId == Binding.strGroupId &&
				Existing.strClip == Binding.strClip && Existing.strStage == Binding.strStage)
			{
				strOutError = "duplicate binding: " +
					(Binding.strGroupId.empty() ? Binding.strEffectId : Binding.strGroupId) + " / " +
					(Binding.strStage.empty() ? Binding.strClip : Binding.strStage);
				return false;
			}
		}
		Staged.push_back(std::move(Binding));
	}
	OutBindings = std::move(Staged);
	return true;
}

bool_t Client::CEffectV2Document::Parse_Group(
	const std::string& strText,
	EFFECT_V2_GROUP& OutGroup,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Group root is not an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pGroupId = Root.Find("groupId");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.effect-v2-group" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pGroupId || !pGroupId->Is_String() || !Is_ValidEffectId(pGroupId->Get_String()))
	{
		strOutError = "schema/formatVersion/groupId mismatch.";
		return false;
	}
	EFFECT_V2_GROUP Group;
	Group.strGroupId = pGroupId->Get_String();
	if (!Read_MsField(Root, "durationMs", Group.iDurationMs, strOutError))
		return false;
	const DATA_JSON_VALUE* pChildren = Root.Find("children");
	if (nullptr == pChildren || !pChildren->Is_Array() || pChildren->Get_Array().empty())
	{
		strOutError = "children must be a non-empty array.";
		return false;
	}
	for (const DATA_JSON_VALUE& Row : pChildren->Get_Array())
	{
		if (!Row.Is_Object())
		{
			strOutError = "children[] entries must be objects.";
			return false;
		}
		EFFECT_V2_GROUP_CHILD Child;
		const DATA_JSON_VALUE* pEffect = Row.Find("effectId");
		if (nullptr == pEffect || !pEffect->Is_String() || !Is_ValidEffectId(pEffect->Get_String()) ||
			pEffect->Get_String() == Group.strGroupId)
		{
			strOutError = "children[].effectId must be a valid effect ID different from the group.";
			return false;
		}
		Child.strEffectId = pEffect->Get_String();
		int32_t iStop = static_cast<int32_t>(Child.eStop);
		if (!Read_MsField(Row, "startMs", Child.iStartMs, strOutError) ||
			!Read_MsField(Row, "durationMs", Child.iDurationMs, strOutError) ||
			!Read_Enum(Row, "stop", CHILD_STOP_KEYS, _countof(CHILD_STOP_KEYS), iStop, strOutError) ||
			!Read_FloatArray(Row, "offset", &Child.vOffset.x, 3u, strOutError) ||
			!Read_Number(Row, "yawDegrees", Child.fYawDegrees, strOutError))
			return false;
		Child.eStop = static_cast<EFFECT_V2_CHILD_STOP>(iStop);
		for (const EFFECT_V2_GROUP_CHILD& Existing : Group.Children)
		{
			if (Existing.strEffectId == Child.strEffectId && Existing.iStartMs == Child.iStartMs)
			{
				strOutError = "duplicate child: " + Child.strEffectId + " @ " +
					std::to_string(Child.iStartMs) + " ms";
				return false;
			}
		}
		Group.Children.push_back(std::move(Child));
	}
	OutGroup = std::move(Group);
	return true;
}
```

적용 위치 7: `Serialize_Bindings` 정의 전체 교체

```cpp
std::string Client::CEffectV2Document::Serialize_Bindings(
	const std::string& strArchetypeId,
	const std::vector<EFFECT_V2_BINDING>& Bindings)
{
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2-bindings\",\n";
	Text += "  \"formatVersion\": 1,\n";
	Text += "  \"archetypeId\": " + Json_String(strArchetypeId) + ",\n";
	Text += "  \"bindings\": [\n";
	for (size_t iIndex = 0u; iIndex < Bindings.size(); ++iIndex)
	{
		const EFFECT_V2_BINDING& Binding = Bindings[iIndex];
		Text += std::string("    { ") +
			(Binding.strGroupId.empty() ?
				"\"effectId\": " + Json_String(Binding.strEffectId) :
				"\"group\": " + Json_String(Binding.strGroupId)) +
			(Binding.strStage.empty() ?
				", \"clip\": " + Json_String(Binding.strClip) :
				", \"stage\": " + Json_String(Binding.strStage)) +
			", \"startMs\": " + std::to_string(Binding.iStartMs) +
			", \"bone\": " + Json_String(Binding.strBone) +
			", \"followBone\": " + Json_Bool(Binding.bFollowBone) +
			", \"rotation\": " + Json_String(Rotation_Key(Binding.eRotation)) +
			", \"stopWithClip\": " + Json_Bool(Binding.bStopWithClip) + " }" +
			(iIndex + 1u < Bindings.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}

std::string Client::CEffectV2Document::Serialize_Group(const EFFECT_V2_GROUP& Group)
{
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2-group\",\n";
	Text += "  \"formatVersion\": 1,\n";
	Text += "  \"groupId\": " + Json_String(Group.strGroupId) + ",\n";
	Text += "  \"durationMs\": " + std::to_string(Group.iDurationMs) + ",\n";
	Text += "  \"children\": [\n";
	for (size_t iIndex = 0u; iIndex < Group.Children.size(); ++iIndex)
	{
		const EFFECT_V2_GROUP_CHILD& Child = Group.Children[iIndex];
		Text += "    { \"effectId\": " + Json_String(Child.strEffectId) +
			", \"startMs\": " + std::to_string(Child.iStartMs) +
			", \"durationMs\": " + std::to_string(Child.iDurationMs) +
			", \"stop\": " + Json_String(Child_Stop_Key(Child.eStop)) +
			", \"offset\": " + Json_Float3(Child.vOffset) +
			", \"yawDegrees\": " + Json_Number(Child.fYawDegrees) + " }" +
			(iIndex + 1u < Group.Children.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}
```

적용 위치 8: `Load_BindingsFile` 정의 바로 아래(`Write_AtomicFile` 정의 위)

```cpp
bool_t Client::CEffectV2Document::Load_GroupFile(
	const std::string& strGroupId,
	EFFECT_V2_GROUP& OutGroup,
	std::string& strOutError)
{
	if (!Is_ValidEffectId(strGroupId))
	{
		strOutError = "Invalid group ID.";
		return false;
	}
	std::string Text;
	const std::filesystem::path Path = Group_Path(strGroupId);
	if (!Read_TextFile(Path, Text))
	{
		strOutError = "Cannot open: " + Path.string();
		return false;
	}
	if (!Parse_Group(Text, OutGroup, strOutError))
		return false;
	if (OutGroup.strGroupId != strGroupId)
	{
		strOutError = "groupId does not match the file name.";
		return false;
	}
	return true;
}
```

**G1 종료 증거**: Client Debug 컴파일. 기존 `NPC_58700/59030/59060/BOSS_VALTAN` 바인딩 파일이 변경 없이 로드됨(Attach 창 `Reload` 상태 메시지 `Loaded N binding(s)`).

## G2. 오브젝트·런타임 — 자식 stop, follow-local, 그룹 전개, free-group lane

### 2-1. `Client/Public/EffectV2_Object.h`

적용 위치 1: public `void Finish() { m_bFinished = true; }` 바로 아래

```cpp
	/* Deactivate: particle/trail stop spawning and finish when their last
	   element dies; every other shape has nothing to drain and finishes now. */
	void Stop_Emission();
```

적용 위치 2: public `void Clear_FollowTarget();` 바로 아래

```cpp
	/* Applied as Local x bone pivot every frame while following, so a group
	   child keeps its offset/yaw on a moving bone. Identity by default. */
	void Set_FollowLocal(const float4x4_t& Local) { m_FollowLocal = Local; }
```

적용 위치 3: private 멤버 `PIVOT_ROTATION m_eFollowRotation = PIVOT_ROTATION::TARGET_YAW;` 바로 아래

```cpp
	float4x4_t m_FollowLocal = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
```

### 2-2. `Client/Private/EffectV2_Object.cpp`

적용 위치 1: `Clear_FollowTarget` 정의 전체 교체

```cpp
void Client::CEffectV2Object::Clear_FollowTarget()
{
	m_bFollowTarget = false;
	m_FollowTarget.Reset();
	m_strFollowBone.clear();
	XMStoreFloat4x4(&m_FollowLocal, XMMatrixIdentity());
}
```

적용 위치 2: `Resolve_TargetPivot` 정의 끝(`return true; }`) 바로 아래, `Update` 정의 위

```cpp
void Client::CEffectV2Object::Stop_Emission()
{
	if (SHAPE::PARTICLE == m_eShape || SHAPE::TRAIL == m_eShape)
		m_bEmissionStopped = true;
	else
		m_bFinished = true;
}
```

적용 위치 3: `Update` 안 follow 블록(`if (m_bFollowTarget) { … }`, EffectV2_Object.cpp:675-683) 교체

```cpp
	if (m_bFollowTarget)
	{
		EFFECT_V2_TARGET_VIEW View;
		float4x4_t Pivot;
		if (!Resolve_TargetView(m_FollowTarget, View) ||
			!Resolve_TargetPivot(View, m_strFollowBone, m_eFollowRotation, Pivot))
		{
			m_bFinished = true;
		}
		else
		{
			XMStoreFloat4x4(&m_PivotWorld,
				XMLoadFloat4x4(&m_FollowLocal) * XMLoadFloat4x4(&Pivot));
		}
	}
```

### 2-3. `Client/Public/EffectV2_Runtime.h` (전체 교체)

```cpp
#pragma once

#include "Client_Defines.h"
#include "EffectV2_Target.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CEffectV2Runtime final
{
public:
	static void Notify_Clip(
		const EFFECT_V2_TARGET& Target,
		const char_t* pClipName);
	static void Tick(
		const EFFECT_V2_TARGET& Target,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Server pattern stage clock: pActionId is the stage actionId ("" = no
	   stage), fAgeSeconds the stage-local age. Bindings keyed by stage spawn
	   when the age crosses their startMs. */
	static void Sync_Stage(
		const EFFECT_V2_TARGET& Target,
		const char_t* pActionId,
		f32_t fAgeSeconds,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Prewarm_Archetype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strArchetypeId);
	static void Set_Ignored(const EFFECT_V2_TARGET& Target, bool_t bIgnored);
	static void Invalidate_Caches();
	static const std::string& Last_Error();

	/* Free-running group lane for the tool: the group clock starts at 0 on
	   Play_Group and advances only through Advance_FreeGroups. No target,
	   no bone following. Returns 0 when the group cannot be expanded; a
	   finished group drops its handle (Group_Seconds < 0). */
	static uint32_t Play_Group(
		const std::string& strGroupId,
		const float4x4_t& PivotWorld,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Stop_Group(uint32_t iHandle);
	static f32_t Group_Seconds(uint32_t iHandle);
	static void Advance_FreeGroups(
		f32_t fTimeDelta,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
};

NS_END
```

### 2-4. `Client/Private/EffectV2_Runtime.cpp` (전체 교체)

변경 요지: `PENDING_SPAWN`이 leaf 문서 + `Local` + stop 시각을 갖고, 바인딩은 `Expand_Binding`이 그룹을 자식으로 펼친다.
`Spawn`은 pivot을 인자로 받아 lane(타깃)과 free-group 둘 다 쓴다. `Apply_ChildStops`가 lane 시각으로 Kill/Deactivate를 한 번만 적용한다.

```cpp
#include "EffectV2_Runtime.h"
#include "ActorCatalog.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace
{
	constexpr const wchar_t* EFFECT_LAYER_TAG = L"Layer_EffectV2";
	constexpr const wchar_t* EFFECT_PROTOTYPE_TAG = L"Prototype_GameObject_EffectV2";
	constexpr const char* VALTAN_ARCHETYPE_ID = "BOSS_VALTAN";

	struct BINDING_SET final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		std::vector<Client::EFFECT_V2_BINDING> Bindings;
	};

	struct DOCUMENT_ENTRY final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		Client::EFFECT_V2_DOCUMENT Document;
	};

	struct GROUP_ENTRY final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		Client::EFFECT_V2_GROUP Group;
	};

	/* One leaf document waiting on a lane clock. Group children arrive here
	   already expanded: Binding.strEffectId is the child document, Local the
	   child offset/yaw, fStopSeconds the lane time of its stop (< 0 = none). */
	struct PENDING_SPAWN final
	{
		Client::EFFECT_V2_BINDING Binding;
		bool_t bSpawned = false;
		float4x4_t Local;
		f32_t fStopSeconds = -1.f;
		Client::EFFECT_V2_CHILD_STOP eStop = Client::EFFECT_V2_CHILD_STOP::KILL;
	};

	struct SPAWNED_EFFECT final
	{
		std::weak_ptr<Client::CEffectV2Object> pObject;
		bool_t bStopWithClip = false;
		bool_t bStageBound = false;
		f32_t fStopSeconds = -1.f;
		Client::EFFECT_V2_CHILD_STOP eStop = Client::EFFECT_V2_CHILD_STOP::KILL;
		bool_t bStopApplied = false;
	};

	struct TARGET_STATE final
	{
		Client::EFFECT_V2_TARGET Target;
		std::string strArchetypeId;
		std::string strClip;
		f32_t fLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> Pending;
		std::string strStage;
		f32_t fStageLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> StagePending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	struct FREE_GROUP final
	{
		float4x4_t Pivot;
		f32_t fSeconds = 0.f;
		std::vector<PENDING_SPAWN> Pending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::unordered_map<std::string, GROUP_ENTRY> g_Groups;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Engine::CGameObject*, TARGET_STATE> g_TargetStates;
	std::set<const Engine::CGameObject*> g_IgnoredTargets;
	std::map<uint32_t, FREE_GROUP> g_FreeGroups;
	uint32_t g_iNextFreeGroupHandle = 1u;
	bool_t g_bPrototypeRegistered = false;
	std::string g_strLastError;

	void Report(const std::string& strMessage)
	{
		g_strLastError = strMessage;
		OutputDebugStringA(("[EffectV2Runtime] " + strMessage + "\n").c_str());
	}

	float4x4_t Identity_Matrix()
	{
		float4x4_t Matrix;
		XMStoreFloat4x4(&Matrix, XMMatrixIdentity());
		return Matrix;
	}

	f32_t Ms_ToSeconds(const uint32_t iMs)
	{
		return static_cast<f32_t>(iMs) / 1000.f;
	}

	const std::string* Resolve_Archetype(const Client::EFFECT_V2_TARGET& Target)
	{
		static const std::string ValtanArchetype = VALTAN_ARCHETYPE_ID;
		if (Client::EFFECT_V2_TARGET_KIND::VALTAN == Target.eKind)
			return &ValtanArchetype;
		if (Client::EFFECT_V2_TARGET_KIND::NPC != Target.eKind)
			return nullptr;
		const std::shared_ptr<Engine::CGameObject> pOwner = Target.pOwner.lock();
		if (nullptr == pOwner)
			return nullptr;
		if (!g_bModelTagMapBuilt)
		{
			g_bModelTagMapBuilt = true;
			for (const Client::NPC_ACTOR_ENTRY& Entry : Client::CActorCatalog::Get_Npcs())
			{
				const wstring_t strTag =
					Client::CNpcPresentationAssetService::Get_ModelPrototypeTag(Entry.archetypeId);
				if (!strTag.empty() && !g_ModelTagToArchetype.contains(strTag))
					g_ModelTagToArchetype.emplace(strTag, Entry.archetypeId);
			}
		}
		const auto Found = g_ModelTagToArchetype.find(
			std::static_pointer_cast<Client::CNpc>(pOwner)->Get_ModelTag());
		return Found != g_ModelTagToArchetype.end() ? &Found->second : nullptr;
	}

	const BINDING_SET& Ensure_Bindings(const std::string& strArchetypeId)
	{
		BINDING_SET& Set = g_BindingSets[strArchetypeId];
		if (Set.bLoaded)
			return Set;
		Set.bLoaded = true;
		std::error_code Error;
		const std::filesystem::path Path =
			Client::CEffectV2Document::Binding_Path(strArchetypeId);
		if (Path.empty() || !std::filesystem::is_regular_file(Path, Error))
			return Set;
		std::string strError;
		if (!Client::CEffectV2Document::Load_BindingsFile(strArchetypeId, Set.Bindings, strError))
		{
			Set.bFailed = true;
			Set.Bindings.clear();
			Report("bindings rejected for " + strArchetypeId + ": " + strError);
		}
		return Set;
	}

	const DOCUMENT_ENTRY& Ensure_Document(const std::string& strEffectId)
	{
		DOCUMENT_ENTRY& Entry = g_Documents[strEffectId];
		if (Entry.bLoaded)
			return Entry;
		Entry.bLoaded = true;
		std::string strError;
		if (!Client::CEffectV2Document::Load_DocumentFile(strEffectId, Entry.Document, strError))
		{
			Entry.bFailed = true;
			Report("document rejected " + strEffectId + ": " + strError);
		}
		return Entry;
	}

	const GROUP_ENTRY& Ensure_Group(const std::string& strGroupId)
	{
		GROUP_ENTRY& Entry = g_Groups[strGroupId];
		if (Entry.bLoaded)
			return Entry;
		Entry.bLoaded = true;
		std::string strError;
		if (!Client::CEffectV2Document::Load_GroupFile(strGroupId, Entry.Group, strError))
		{
			Entry.bFailed = true;
			Report("group rejected " + strGroupId + ": " + strError);
		}
		return Entry;
	}

	bool_t Ensure_Prototype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (g_bPrototypeRegistered)
			return true;
		unique_ptr<Client::CEffectV2Object> pPrototype =
			Client::CEffectV2Object::Create(pDevice, pContext);
		if (nullptr == pPrototype)
		{
			Report("effect prototype creation failed.");
			return false;
		}
		(void)CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG, std::move(pPrototype));
		g_bPrototypeRegistered = true;
		return true;
	}

	float4x4_t Child_Local(const Client::EFFECT_V2_GROUP_CHILD& Child)
	{
		float4x4_t Local;
		XMStoreFloat4x4(&Local,
			XMMatrixRotationY(XMConvertToRadians(Child.fYawDegrees)) *
			XMMatrixTranslation(Child.vOffset.x, Child.vOffset.y, Child.vOffset.z));
		return Local;
	}

	/* A document binding becomes one pending row; a group binding becomes
	   one row per child with the binding's own start added and the group
	   duration capping every child stop. A rejected group adds nothing. */
	void Expand_Binding(
		const Client::EFFECT_V2_BINDING& Binding,
		std::vector<PENDING_SPAWN>& Out)
	{
		if (Binding.strGroupId.empty())
		{
			PENDING_SPAWN Pending;
			Pending.Binding = Binding;
			Pending.Local = Identity_Matrix();
			Out.push_back(std::move(Pending));
			return;
		}
		const GROUP_ENTRY& Entry = Ensure_Group(Binding.strGroupId);
		if (Entry.bFailed)
			return;
		const f32_t fGroupStop = Entry.Group.iDurationMs > 0u ?
			Ms_ToSeconds(Binding.iStartMs + Entry.Group.iDurationMs) : -1.f;
		for (const Client::EFFECT_V2_GROUP_CHILD& Child : Entry.Group.Children)
		{
			PENDING_SPAWN Pending;
			Pending.Binding = Binding;
			Pending.Binding.strGroupId.clear();
			Pending.Binding.strEffectId = Child.strEffectId;
			Pending.Binding.iStartMs = Binding.iStartMs + Child.iStartMs;
			Pending.Local = Child_Local(Child);
			Pending.eStop = Child.eStop;
			f32_t fStop = Child.iDurationMs > 0u ?
				Ms_ToSeconds(Pending.Binding.iStartMs + Child.iDurationMs) : -1.f;
			if (fGroupStop >= 0.f && (fStop < 0.f || fGroupStop < fStop))
				fStop = fGroupStop;
			Pending.fStopSeconds = fStop;
			Out.push_back(std::move(Pending));
		}
	}

	void Spawn(
		PENDING_SPAWN& Pending,
		const float4x4_t& Pivot,
		const Client::EFFECT_V2_TARGET& FollowTarget,
		const bool_t bStageBound,
		std::vector<SPAWNED_EFFECT>& Spawned,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		Pending.bSpawned = true;
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed || !Ensure_Prototype(pDevice, pContext))
			return;
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = Entry.Document.Desc;
		XMStoreFloat4x4(&Desc.PivotWorld,
			XMLoadFloat4x4(&Pending.Local) * XMLoadFloat4x4(&Pivot));
		std::shared_ptr<CGameObject> pGameObject;
		if (FAILED(GameInstance.Add_GameObject_to_Layer(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG,
			GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG,
			&Desc, &pGameObject)))
		{
			Report("spawn failed for " + Pending.Binding.strEffectId + ": " +
				Client::CEffectV2Object::Last_Error());
			return;
		}
		const std::shared_ptr<Client::CEffectV2Object> pObject =
			std::dynamic_pointer_cast<Client::CEffectV2Object>(pGameObject);
		if (nullptr == pObject)
			return;
		for (uint32_t iPart = 0u;
			iPart < Entry.Document.Parts.size() && iPart < pObject->Part_Count(); ++iPart)
		{
			pObject->Part_Visible(iPart) = Entry.Document.Parts[iPart].bVisible;
			if (!Entry.Document.Parts[iPart].strBaseAssetId.empty())
				(void)pObject->Set_PartBase(iPart, Entry.Document.Parts[iPart].strBaseAssetId);
		}
		if (!Entry.Document.strAnimationClip.empty())
		{
			for (uint32_t iClip = 0u; iClip < pObject->Animation_Count(); ++iClip)
			{
				const char_t* pName = pObject->Animation_Name(iClip);
				if (nullptr != pName && Entry.Document.strAnimationClip == pName)
				{
					pObject->Params().iAnimationIndex = iClip;
					break;
				}
			}
		}
		if (Pending.Binding.bFollowBone && FollowTarget.Is_Valid())
		{
			pObject->Set_FollowTarget(
				FollowTarget, Pending.Binding.strBone, Pending.Binding.eRotation);
			pObject->Set_FollowLocal(Pending.Local);
		}
		SPAWNED_EFFECT Effect;
		Effect.pObject = pObject;
		Effect.bStopWithClip = Pending.Binding.bStopWithClip;
		Effect.bStageBound = bStageBound;
		Effect.fStopSeconds = Pending.fStopSeconds;
		Effect.eStop = Pending.eStop;
		Spawned.push_back(std::move(Effect));
	}

	void Spawn_OnTarget(
		TARGET_STATE& State,
		PENDING_SPAWN& Pending,
		const Client::EFFECT_V2_TARGET_VIEW& View,
		const bool_t bStageBound,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		float4x4_t Pivot;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			View, Pending.Binding.strBone, Pending.Binding.eRotation, Pivot))
		{
			Pending.bSpawned = true;
			Report("pivot bone not found: " + Pending.Binding.strBone +
				" for " + Pending.Binding.strEffectId);
			return;
		}
		Spawn(Pending, Pivot, State.Target, bStageBound, State.Spawned, pDevice, pContext);
	}

	/* Applies each child's stop once when the lane clock passes it. With
	   bFilterLane the clip lane only stops clip-bound effects and the stage
	   lane only stage-bound ones; free groups pass false. */
	void Apply_ChildStops(
		std::vector<SPAWNED_EFFECT>& Spawned,
		const f32_t fLaneSeconds,
		const bool_t bStageLane,
		const bool_t bFilterLane)
	{
		for (SPAWNED_EFFECT& Effect : Spawned)
		{
			if (Effect.bStopApplied || Effect.fStopSeconds < 0.f ||
				fLaneSeconds < Effect.fStopSeconds)
				continue;
			if (bFilterLane && Effect.bStageBound != bStageLane)
				continue;
			Effect.bStopApplied = true;
			const std::shared_ptr<Client::CEffectV2Object> pObject = Effect.pObject.lock();
			if (nullptr == pObject)
				continue;
			if (Client::EFFECT_V2_CHILD_STOP::DEACTIVATE == Effect.eStop)
				pObject->Stop_Emission();
			else
				pObject->Finish();
		}
	}

	/* bStopClipBound finishes stopWithClip effects of the clip lane,
	   bStopStageBound those of the stage lane; both lanes always drop what has
	   finished on its own. */
	void Prune_List(
		std::vector<SPAWNED_EFFECT>& Spawned,
		const bool_t bStopClipBound,
		const bool_t bStopStageBound)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (auto Iterator = Spawned.begin(); Iterator != Spawned.end();)
		{
			const std::shared_ptr<Client::CEffectV2Object> pObject = Iterator->pObject.lock();
			const bool_t bStopLane = Iterator->bStageBound ? bStopStageBound : bStopClipBound;
			if (nullptr != pObject && bStopLane && Iterator->bStopWithClip)
				pObject->Finish();
			if (nullptr == pObject || pObject->Is_Finished())
			{
				if (nullptr != pObject)
				{
					GameInstance.Remove_GameObject_from_Layer(
						GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG, pObject);
				}
				Iterator = Spawned.erase(Iterator);
				continue;
			}
			++Iterator;
		}
	}

	void Prune_Spawned(
		TARGET_STATE& State,
		const bool_t bStopClipBound,
		const bool_t bStopStageBound = false)
	{
		Prune_List(State.Spawned, bStopClipBound, bStopStageBound);
	}

	void Kill_List(std::vector<SPAWNED_EFFECT>& Spawned)
	{
		for (SPAWNED_EFFECT& Effect : Spawned)
		{
			if (const std::shared_ptr<Client::CEffectV2Object> pObject = Effect.pObject.lock())
				pObject->Finish();
		}
		Prune_List(Spawned, false, false);
	}
}

void Client::CEffectV2Runtime::Prewarm_Archetype(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const std::string& strArchetypeId)
{
	const BINDING_SET& Set = Ensure_Bindings(strArchetypeId);
	if (Set.bFailed)
		return;
	std::vector<PENDING_SPAWN> Expanded;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
		Expand_Binding(Binding, Expanded);
	for (const PENDING_SPAWN& Pending : Expanded)
	{
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed)
			continue;
		std::string strError;
		if (FAILED(CEffectV2Object::Prewarm(pDevice, pContext, Entry.Document.Desc, strError)))
			Report("prewarm failed " + Pending.Binding.strEffectId + ": " + strError);
	}
	Ensure_Prototype(pDevice, pContext);
}

void Client::CEffectV2Runtime::Set_Ignored(const EFFECT_V2_TARGET& Target, const bool_t bIgnored)
{
	if (nullptr == Target.pKey)
		return;
	if (bIgnored)
	{
		g_IgnoredTargets.insert(Target.pKey);
		const auto Found = g_TargetStates.find(Target.pKey);
		if (Found != g_TargetStates.end())
		{
			Prune_Spawned(Found->second, true, true);
			g_TargetStates.erase(Found);
		}
	}
	else
		g_IgnoredTargets.erase(Target.pKey);
}

void Client::CEffectV2Runtime::Invalidate_Caches()
{
	g_BindingSets.clear();
	g_Documents.clear();
	g_Groups.clear();
	g_ModelTagToArchetype.clear();
	g_bModelTagMapBuilt = false;
}

const std::string& Client::CEffectV2Runtime::Last_Error()
{
	return g_strLastError;
}

void Client::CEffectV2Runtime::Notify_Clip(
	const EFFECT_V2_TARGET& Target,
	const char_t* pClipName)
{
	if (!Target.Is_Valid() || nullptr == pClipName || g_IgnoredTargets.contains(Target.pKey))
		return;
	const std::string* pArchetypeId = Resolve_Archetype(Target);
	if (nullptr == pArchetypeId)
		return;
	const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
	TARGET_STATE& State = g_TargetStates[Target.pKey];
	State.Target = Target;
	State.strArchetypeId = *pArchetypeId;
	State.strClip = pClipName;
	State.fLastSeconds = -1.f;
	Prune_Spawned(State, true);
	State.Pending.clear();
	if (Set.bFailed)
		return;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		if (Binding.strStage.empty() && Binding.strClip == State.strClip)
			Expand_Binding(Binding, State.Pending);
	}
}

void Client::CEffectV2Runtime::Sync_Stage(
	const EFFECT_V2_TARGET& Target,
	const char_t* pActionId,
	const f32_t fAgeSeconds,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!Target.Is_Valid() || g_IgnoredTargets.contains(Target.pKey))
		return;
	const std::string strActionId = nullptr != pActionId ? pActionId : "";
	const auto Found = g_TargetStates.find(Target.pKey);
	if (Found == g_TargetStates.end() && strActionId.empty())
		return;
	const std::string* pArchetypeId = Resolve_Archetype(Target);
	if (nullptr == pArchetypeId)
		return;
	TARGET_STATE& State = Found != g_TargetStates.end() ?
		Found->second : g_TargetStates[Target.pKey];
	State.Target = Target;
	State.strArchetypeId = *pArchetypeId;
	if (State.strStage != strActionId)
	{
		State.strStage = strActionId;
		State.fStageLastSeconds = -1.f;
		Prune_Spawned(State, false, true);
		State.StagePending.clear();
		const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
		if (!Set.bFailed && !strActionId.empty())
		{
			for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
			{
				if (!Binding.strStage.empty() && Binding.strStage == strActionId)
					Expand_Binding(Binding, State.StagePending);
			}
		}
	}
	if (State.StagePending.empty() || !std::isfinite(fAgeSeconds) || fAgeSeconds < 0.f)
	{
		Prune_Spawned(State, false, false);
		return;
	}
	if (fAgeSeconds < State.fStageLastSeconds)
	{
		for (PENDING_SPAWN& Pending : State.StagePending)
			Pending.bSpawned = false;
	}
	State.fStageLastSeconds = fAgeSeconds;
	EFFECT_V2_TARGET_VIEW View;
	if (!CEffectV2Object::Resolve_TargetView(State.Target, View))
		return;
	for (PENDING_SPAWN& Pending : State.StagePending)
	{
		if (Pending.bSpawned)
			continue;
		if (fAgeSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
			Spawn_OnTarget(State, Pending, View, true, pDevice, pContext);
	}
	Apply_ChildStops(State.Spawned, fAgeSeconds, true, true);
	Prune_Spawned(State, false, false);
}

void Client::CEffectV2Runtime::Tick(
	const EFFECT_V2_TARGET& Target,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	for (auto Iterator = g_TargetStates.begin(); Iterator != g_TargetStates.end();)
	{
		if (!Iterator->second.Target.Is_Valid())
		{
			Prune_Spawned(Iterator->second, true, true);
			Iterator = g_TargetStates.erase(Iterator);
			continue;
		}
		++Iterator;
	}
	if (nullptr == Target.pKey)
		return;
	const auto Found = g_TargetStates.find(Target.pKey);
	if (Found == g_TargetStates.end())
		return;
	TARGET_STATE& State = Found->second;
	EFFECT_V2_TARGET_VIEW View;
	if (!CEffectV2Object::Resolve_TargetView(State.Target, View))
		return;
	const shared_ptr<Engine::CModel>& pModel = View.pModel;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iClip);
	if (nullptr == pCurrentClip || State.strClip != pCurrentClip)
	{
		Prune_Spawned(State, true);
		State.Pending.clear();
		return;
	}
	if (State.Pending.empty())
	{
		Prune_Spawned(State, false);
		return;
	}
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iClip);
	if (fTickPerSecond <= 0.f || !pModel->Get_AnimationProgress(iClip, fPosition, fDuration))
		return;
	const f32_t fSeconds = fPosition / fTickPerSecond;
	if (fSeconds < State.fLastSeconds)
	{
		for (PENDING_SPAWN& Pending : State.Pending)
			Pending.bSpawned = false;
	}
	State.fLastSeconds = fSeconds;
	for (PENDING_SPAWN& Pending : State.Pending)
	{
		if (Pending.bSpawned)
			continue;
		if (fSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
			Spawn_OnTarget(State, Pending, View, false, pDevice, pContext);
	}
	Apply_ChildStops(State.Spawned, fSeconds, false, true);
	Prune_Spawned(State, false);
}

uint32_t Client::CEffectV2Runtime::Play_Group(
	const std::string& strGroupId,
	const float4x4_t& PivotWorld,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!Ensure_Prototype(pDevice, pContext))
		return 0u;
	EFFECT_V2_BINDING Binding;
	Binding.strGroupId = strGroupId;
	Binding.bFollowBone = false;
	FREE_GROUP Group;
	Group.Pivot = PivotWorld;
	Expand_Binding(Binding, Group.Pending);
	if (Group.Pending.empty())
	{
		if (g_strLastError.empty())
			Report("group has no children: " + strGroupId);
		return 0u;
	}
	const uint32_t iHandle = g_iNextFreeGroupHandle++;
	if (0u == g_iNextFreeGroupHandle)
		g_iNextFreeGroupHandle = 1u;
	g_FreeGroups.emplace(iHandle, std::move(Group));
	return iHandle;
}

void Client::CEffectV2Runtime::Stop_Group(const uint32_t iHandle)
{
	const auto Found = g_FreeGroups.find(iHandle);
	if (Found == g_FreeGroups.end())
		return;
	Kill_List(Found->second.Spawned);
	g_FreeGroups.erase(Found);
}

f32_t Client::CEffectV2Runtime::Group_Seconds(const uint32_t iHandle)
{
	const auto Found = g_FreeGroups.find(iHandle);
	return Found != g_FreeGroups.end() ? Found->second.fSeconds : -1.f;
}

void Client::CEffectV2Runtime::Advance_FreeGroups(
	const f32_t fTimeDelta,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;
	for (auto Iterator = g_FreeGroups.begin(); Iterator != g_FreeGroups.end();)
	{
		FREE_GROUP& Group = Iterator->second;
		Group.fSeconds += fTimeDelta;
		for (PENDING_SPAWN& Pending : Group.Pending)
		{
			if (Pending.bSpawned)
				continue;
			if (Group.fSeconds >= Ms_ToSeconds(Pending.Binding.iStartMs))
			{
				Spawn(Pending, Group.Pivot, EFFECT_V2_TARGET{}, false, Group.Spawned,
					pDevice, pContext);
			}
		}
		Apply_ChildStops(Group.Spawned, Group.fSeconds, false, false);
		Prune_List(Group.Spawned, false, false);
		const bool_t bAllSpawned = std::all_of(Group.Pending.begin(), Group.Pending.end(),
			[](const PENDING_SPAWN& Pending) { return Pending.bSpawned; });
		if (bAllSpawned && Group.Spawned.empty())
		{
			Iterator = g_FreeGroups.erase(Iterator);
			continue;
		}
		++Iterator;
	}
}
```

**G2 종료 증거**: Client Debug 빌드. 기존 바인딩(문서 단위)의 인게임 동작 무변화 — Attach 창 웨이 `NPC_58700` + `Runtime spawns on target`에서 도철 3종이 이전과 같이 스폰(사용자 확인). 그룹 바인딩 하나를 임시로 `BOSS_VALTAN`에 stage 바인딩해 Valtan 맵 `Server Replay`에서 자식이 `startMs` 순서로 뜨고 `durationMs` 뒤 Deactivate 자식이 스폰만 멈추는지 확인(사용자).

## G3. 툴 — Group 창과 `Bind As`

### 3-1. `Client/Public/Effect_Tool_V2.h`

적용 위치 1: private 함수 선언 `bool_t Load_Bindings(const std::string& strArchetypeId);` 바로 아래

```cpp
	void Render_GroupWindow();
	void Scan_Groups();
	bool_t Load_Group(const std::string& strGroupId);
	bool_t Save_Group();
	bool_t Play_GroupPreview();
	void Stop_GroupPreview();
```

적용 위치 2: 멤버 `std::string m_strAttachStatus;` 바로 아래(`VALTAN_PATTERN_TREE_VIEW m_ValtanTree;` 위)

```cpp
	bool_t m_bGroupWindowOpen = false;
	bool_t m_bGroupsScanned = false;
	std::vector<std::string> m_Groups;
	EFFECT_V2_GROUP m_Group;
	char m_szGroupId[96] = {};
	std::string m_strBindingGroupId;
	uint32_t m_iGroupPreviewHandle = 0u;
	std::string m_strGroupStatus;
```

### 3-2. `Client/Private/Effect_Tool_V2.cpp`

적용 위치 1: `Render()` 끝 `Render_AttachWindow();` 바로 아래

```cpp
	Render_GroupWindow();
```

적용 위치 2: Create 패널의 `if (ImGui::Button("Attach...")) m_bAttachWindowOpen = true;` 바로 아래

```cpp
	ImGui::SameLine();
	if (ImGui::Button("Group..."))
		m_bGroupWindowOpen = true;
```

적용 위치 3: `Update_Attach` 첫 줄 `Update_ValtanTimeline(fTimeDelta);` 바로 아래

```cpp
	CEffectV2Runtime::Advance_FreeGroups(fTimeDelta, m_pDevice, m_pContext);
	if (0u != m_iGroupPreviewHandle &&
		CEffectV2Runtime::Group_Seconds(m_iGroupPreviewHandle) < 0.f)
		m_iGroupPreviewHandle = 0u;
```

적용 위치 4: `Render_AttachWindow` 안 `ImGui::Text("Effect: %s | %s: %s | Bone: %s", …);` 부터
`Add / Update Binding` 버튼 블록의 `ImGui::EndDisabled();`(줄 2287-2329)까지 교체

```cpp
	if (!m_bGroupsScanned)
		Scan_Groups();
	const std::string strBindAsLabel =
		m_strBindingGroupId.empty() ? "(effect document)" : "group: " + m_strBindingGroupId;
	if (ImGui::BeginCombo("Bind As", strBindAsLabel.c_str()))
	{
		if (ImGui::Selectable("(effect document)", m_strBindingGroupId.empty()))
			m_strBindingGroupId.clear();
		for (const std::string& strGroup : m_Groups)
		{
			if (ImGui::Selectable(strGroup.c_str(), strGroup == m_strBindingGroupId))
				m_strBindingGroupId = strGroup;
		}
		ImGui::EndCombo();
	}
	const bool_t bGroupBinding = !m_strBindingGroupId.empty();
	ImGui::Text("%s: %s | %s: %s | Bone: %s",
		bGroupBinding ? "Group" : "Effect",
		bGroupBinding ? m_strBindingGroupId.c_str() :
			('\0' == m_szEffectId[0] ? "(none)" : m_szEffectId),
		bStageBinding ? "Stage" : "Clip",
		bStageBinding ? strStageForBinding.c_str() :
			(nullptr != pClipForBinding ? pClipForBinding : "(none)"),
		m_strPivotBone.empty() ? "(none)" : m_strPivotBone.c_str());
	ImGui::BeginDisabled((!bGroupBinding && '\0' == m_szEffectId[0]) || !bHasTarget ||
		(!bStageBinding && nullptr == pClipForBinding));
	if (ImGui::Button("Add / Update Binding"))
	{
		EFFECT_BINDING Binding;
		if (bGroupBinding)
			Binding.strGroupId = m_strBindingGroupId;
		else
			Binding.strEffectId = m_szEffectId;
		if (bStageBinding)
		{
			Binding.strStage = strStageForBinding;
			Binding.iStartMs = iStageOffsetForBinding;
		}
		else
		{
			Binding.strClip = pClipForBinding;
			Binding.iStartMs = static_cast<uint32_t>(
				static_cast<f32_t>(m_iSpawnFrame) * 1000.f / BINDING_FRAME_RATE);
		}
		Binding.strBone = PIVOT_MODE::WORLD != m_ePivotMode ? m_strPivotBone : std::string();
		Binding.bFollowBone = PIVOT_MODE::TARGET_BONE == m_ePivotMode;
		Binding.eRotation = m_ePivotRotation;
		bool_t bReplaced = false;
		for (EFFECT_BINDING& Existing : m_Bindings)
		{
			if (Existing.strEffectId == Binding.strEffectId &&
				Existing.strGroupId == Binding.strGroupId &&
				Existing.strClip == Binding.strClip && Existing.strStage == Binding.strStage)
			{
				Binding.bStopWithClip = Existing.bStopWithClip;
				Existing = Binding;
				bReplaced = true;
				break;
			}
		}
		if (!bReplaced)
			m_Bindings.push_back(Binding);
		m_strAttachStatus = bReplaced ? "Binding updated (unsaved)." : "Binding added (unsaved).";
	}
	ImGui::EndDisabled();
```

적용 위치 5: 같은 함수의 row 루프 안 `std::snprintf(szRow, sizeof(szRow), "%s @ %s%s +%ums %s%s", …);`(줄 2345-2351)과
row 선택 블록 `if (ImGui::Selectable(szRow, false, ImGuiSelectableFlags_AllowOverlap)) { … }`(줄 2354-2376)을 함께 교체

```cpp
		const std::string strRowName = Binding.strGroupId.empty() ?
			Binding.strEffectId : "group:" + Binding.strGroupId;
		std::snprintf(szRow, sizeof(szRow), "%s @ %s%s +%ums %s%s",
			strRowName.c_str(),
			Binding.strStage.empty() ? "" : "stage:",
			Binding.strStage.empty() ? Binding.strClip.c_str() : Binding.strStage.c_str(),
			Binding.iStartMs,
			Binding.strBone.empty() ? "(world)" : Binding.strBone.c_str(),
			Binding.bFollowBone ? "" : " [snap once]");
		/* The row spans the full line, so the overlapping SameLine checkbox and
		Remove button need AllowOverlap or the row swallows their clicks. */
		if (ImGui::Selectable(szRow, false, ImGuiSelectableFlags_AllowOverlap))
		{
			if (Binding.strGroupId.empty())
			{
				m_strBindingGroupId.clear();
				std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", Binding.strEffectId.c_str());
			}
			else
			{
				m_strBindingGroupId = Binding.strGroupId;
				m_bGroupWindowOpen = true;
				Load_Group(Binding.strGroupId);
			}
			m_ePivotMode = Binding.strBone.empty() ? PIVOT_MODE::WORLD :
				(Binding.bFollowBone ? PIVOT_MODE::TARGET_BONE : PIVOT_MODE::TARGET_BONE_FIXED);
			m_ePivotRotation = Binding.eRotation;
			if (!Binding.strBone.empty())
				m_strPivotBone = Binding.strBone;
			if (PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode)
				Snap_PivotToTarget();
			if (!Binding.strStage.empty())
			{
				Try_LocateValtanStage(Binding.strStage, Binding.iStartMs);
			}
			else
			{
				if (m_bValtanTimelineActive)
					Stop_ValtanTimeline();
				m_iSpawnFrame = static_cast<int32_t>(
					static_cast<f32_t>(Binding.iStartMs) * BINDING_FRAME_RATE / 1000.f + 0.5f);
				Play_TargetClip(Binding.strClip.c_str(), m_bTargetClipLoop);
			}
		}
```

적용 위치 6: `Render_AttachWindow` 정의 끝(`ImGui::End(); }`) 바로 아래, `namespace { void Draw_LerpTrack(` 위

```cpp
void Client::CEffect_Tool_V2::Scan_Groups()
{
	m_bGroupsScanned = true;
	m_Groups.clear();
	std::error_code Error;
	const std::filesystem::path Directory = CEffectV2Document::Group_Directory();
	if (Directory.empty() || !std::filesystem::is_directory(Directory, Error))
		return;
	for (const std::filesystem::directory_entry& Entry :
		std::filesystem::directory_iterator(Directory, Error))
	{
		if (!Entry.is_regular_file(Error))
			continue;
		const std::string strName = Entry.path().filename().string();
		constexpr const char* SUFFIX = ".effectv2group.json";
		const size_t iSuffix = std::strlen(SUFFIX);
		if (strName.size() <= iSuffix ||
			strName.compare(strName.size() - iSuffix, iSuffix, SUFFIX) != 0)
			continue;
		m_Groups.push_back(strName.substr(0u, strName.size() - iSuffix));
	}
	std::sort(m_Groups.begin(), m_Groups.end());
}

bool_t Client::CEffect_Tool_V2::Load_Group(const std::string& strGroupId)
{
	EFFECT_V2_GROUP Group;
	std::string strError;
	if (!CEffectV2Document::Load_GroupFile(strGroupId, Group, strError))
	{
		m_strGroupStatus = "Load rejected (" + strGroupId + "): " + strError;
		return false;
	}
	Stop_GroupPreview();
	m_Group = std::move(Group);
	std::snprintf(m_szGroupId, sizeof(m_szGroupId), "%s", strGroupId.c_str());
	m_strGroupStatus = "Loaded " + strGroupId;
	return true;
}

bool_t Client::CEffect_Tool_V2::Save_Group()
{
	const std::string strGroupId = m_szGroupId;
	if (!CEffectV2Document::Is_ValidEffectId(strGroupId))
	{
		m_strGroupStatus = "Group ID must be 1-80 chars of [A-Za-z0-9._-].";
		return false;
	}
	if (m_Group.Children.empty())
	{
		m_strGroupStatus = "Add at least one child before saving.";
		return false;
	}
	for (const EFFECT_V2_GROUP_CHILD& Child : m_Group.Children)
	{
		if (!CEffectV2Document::Is_ValidEffectId(Child.strEffectId) ||
			Child.strEffectId == strGroupId)
		{
			m_strGroupStatus = "Every child needs an effect ID different from the group.";
			return false;
		}
	}
	m_Group.strGroupId = strGroupId;
	std::error_code Error;
	std::filesystem::create_directories(CEffectV2Document::Group_Directory(), Error);
	std::string strError;
	if (!CEffectV2Document::Write_AtomicFile(
		CEffectV2Document::Group_Path(strGroupId),
		CEffectV2Document::Serialize_Group(m_Group), strError))
	{
		m_strGroupStatus = strError;
		return false;
	}
	CEffectV2Runtime::Invalidate_Caches();
	m_bGroupsScanned = false;
	m_strGroupStatus = "Saved " + strGroupId + ".effectv2group.json";
	return true;
}

bool_t Client::CEffect_Tool_V2::Play_GroupPreview()
{
	Stop_GroupPreview();
	const std::string strGroupId = m_szGroupId;
	std::error_code Error;
	if (!std::filesystem::is_regular_file(CEffectV2Document::Group_Path(strGroupId), Error))
	{
		m_strGroupStatus = "Save the group first; preview plays the saved file.";
		return false;
	}
	float4x4_t Pivot;
	EFFECT_V2_TARGET_VIEW View;
	if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
		Pivot = pPreview->PivotWorld();
	else if (!Resolve_TargetView(View) ||
		!CEffectV2Object::Resolve_TargetPivot(View, m_strPivotBone, m_ePivotRotation, Pivot))
	{
		m_strGroupStatus = "Create a preview effect or spawn a target to place the group.";
		return false;
	}
	m_iGroupPreviewHandle = CEffectV2Runtime::Play_Group(strGroupId, Pivot, m_pDevice, m_pContext);
	if (0u == m_iGroupPreviewHandle)
	{
		m_strGroupStatus = "Group preview failed: " + CEffectV2Runtime::Last_Error();
		return false;
	}
	m_strGroupStatus = "Playing " + strGroupId;
	return true;
}

void Client::CEffect_Tool_V2::Stop_GroupPreview()
{
	if (0u == m_iGroupPreviewHandle)
		return;
	CEffectV2Runtime::Stop_Group(m_iGroupPreviewHandle);
	m_iGroupPreviewHandle = 0u;
}

void Client::CEffect_Tool_V2::Render_GroupWindow()
{
	if (!m_bGroupWindowOpen)
		return;
	if (!m_bGroupsScanned)
		Scan_Groups();
	if (!m_bDocumentsScanned)
		Scan_Documents();
	ImGui::SetNextWindowSize(ImVec2(560.f, 560.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Group v2", &m_bGroupWindowOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::SeparatorText("Group (Data/Effects/V2/Groups)");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##GroupId", "Group ID (e.g. boss.valtan.portal)",
		m_szGroupId, sizeof(m_szGroupId));
	if (ImGui::Button("New"))
	{
		Stop_GroupPreview();
		m_Group = {};
		m_Group.strGroupId = m_szGroupId;
		m_strGroupStatus = "New group (unsaved).";
	}
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_szGroupId[0]);
	if (ImGui::Button("Load"))
		Load_Group(m_szGroupId);
	ImGui::SameLine();
	if (ImGui::Button("Save"))
		Save_Group();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Rescan"))
		Scan_Groups();
	if (m_Groups.empty())
		ImGui::TextDisabled("No saved groups.");
	else if (ImGui::BeginListBox("##Groups", ImVec2(-1.f, 72.f)))
	{
		for (const std::string& strGroup : m_Groups)
		{
			if (ImGui::Selectable(strGroup.c_str(), strGroup == m_szGroupId))
				std::snprintf(m_szGroupId, sizeof(m_szGroupId), "%s", strGroup.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				Load_Group(strGroup);
		}
		ImGui::EndListBox();
	}
	int32_t iGroupDuration = static_cast<int32_t>(m_Group.iDurationMs);
	if (ImGui::InputInt("Group Duration (ms, 0 = last child)", &iGroupDuration))
		m_Group.iDurationMs = static_cast<uint32_t>(std::clamp(iGroupDuration, 0, 600000));

	ImGui::SeparatorText("Children");
	ImGui::BeginDisabled('\0' == m_szEffectId[0]);
	if (ImGui::Button("Add Child (current Effect ID)"))
	{
		EFFECT_V2_GROUP_CHILD Child;
		Child.strEffectId = m_szEffectId;
		m_Group.Children.push_back(std::move(Child));
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Effect ID comes from the Document panel.");
	uint32_t iEndMs = m_Group.iDurationMs;
	for (size_t iIndex = 0u; iIndex < m_Group.Children.size(); ++iIndex)
	{
		EFFECT_V2_GROUP_CHILD& Child = m_Group.Children[iIndex];
		ImGui::PushID(static_cast<int32_t>(iIndex));
		if (ImGui::BeginCombo("Effect", Child.strEffectId.empty() ? "(select)" : Child.strEffectId.c_str()))
		{
			for (const std::string& strDocument : m_Documents)
			{
				if (ImGui::Selectable(strDocument.c_str(), strDocument == Child.strEffectId))
					Child.strEffectId = strDocument;
			}
			ImGui::EndCombo();
		}
		int32_t iStart = static_cast<int32_t>(Child.iStartMs);
		int32_t iDuration = static_cast<int32_t>(Child.iDurationMs);
		if (ImGui::InputInt("Start (ms)", &iStart))
			Child.iStartMs = static_cast<uint32_t>(std::clamp(iStart, 0, 600000));
		if (ImGui::InputInt("Duration (ms, 0 = own lifetime)", &iDuration))
			Child.iDurationMs = static_cast<uint32_t>(std::clamp(iDuration, 0, 600000));
		int32_t iStop = static_cast<int32_t>(Child.eStop);
		if (ImGui::Combo("Stop", &iStop, "Kill\0Deactivate\0"))
			Child.eStop = static_cast<EFFECT_V2_CHILD_STOP>(iStop);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Kill: remove at once. Deactivate: particles/trails stop spawning and drain; other shapes end.");
		ImGui::DragFloat3("Offset (m)", &Child.vOffset.x, 0.05f);
		ImGui::DragFloat("Yaw (deg)", &Child.fYawDegrees, 1.f, -360.f, 360.f);
		if (ImGui::SmallButton("Remove"))
		{
			m_Group.Children.erase(m_Group.Children.begin() + static_cast<std::ptrdiff_t>(iIndex));
			ImGui::PopID();
			break;
		}
		ImGui::Separator();
		ImGui::PopID();
		iEndMs = (std::max)(iEndMs,
			Child.iDurationMs > 0u ? Child.iStartMs + Child.iDurationMs : Child.iStartMs);
	}

	ImGui::SeparatorText("Timeline");
	const f32_t fTotalMs = static_cast<f32_t>((std::max)(1000u, iEndMs));
	ImDrawList* pDraw = ImGui::GetWindowDrawList();
	const ImVec2 Origin = ImGui::GetCursorScreenPos();
	const f32_t fWidth = (std::max)(64.f, ImGui::GetContentRegionAvail().x);
	constexpr f32_t ROW_HEIGHT = 16.f;
	for (size_t iIndex = 0u; iIndex < m_Group.Children.size(); ++iIndex)
	{
		const EFFECT_V2_GROUP_CHILD& Child = m_Group.Children[iIndex];
		const f32_t fY = Origin.y + static_cast<f32_t>(iIndex) * ROW_HEIGHT;
		const f32_t fX0 = Origin.x + static_cast<f32_t>(Child.iStartMs) / fTotalMs * fWidth;
		const f32_t fX1 = Child.iDurationMs > 0u ?
			Origin.x + static_cast<f32_t>(Child.iStartMs + Child.iDurationMs) / fTotalMs * fWidth :
			Origin.x + fWidth;
		pDraw->AddRectFilled(ImVec2(fX0, fY + 2.f), ImVec2((std::max)(fX1, fX0 + 3.f), fY + ROW_HEIGHT - 2.f),
			Child.iDurationMs > 0u ? IM_COL32(90, 170, 255, 200) : IM_COL32(120, 120, 140, 160));
		pDraw->AddText(ImVec2(fX0 + 3.f, fY), IM_COL32(255, 255, 255, 255), Child.strEffectId.c_str());
	}
	const f32_t fPreviewSeconds = 0u != m_iGroupPreviewHandle ?
		CEffectV2Runtime::Group_Seconds(m_iGroupPreviewHandle) : -1.f;
	if (fPreviewSeconds >= 0.f)
	{
		const f32_t fX = Origin.x + (std::min)(1.f, fPreviewSeconds * 1000.f / fTotalMs) * fWidth;
		pDraw->AddLine(ImVec2(fX, Origin.y), ImVec2(fX, Origin.y + ROW_HEIGHT * static_cast<f32_t>(m_Group.Children.size())),
			IM_COL32(255, 200, 60, 255), 2.f);
	}
	ImGui::Dummy(ImVec2(fWidth, ROW_HEIGHT * static_cast<f32_t>(m_Group.Children.size()) + 4.f));
	ImGui::TextDisabled("0 ms .. %.0f ms", fTotalMs);
	if (ImGui::Button(0u != m_iGroupPreviewHandle ? "Restart Preview" : "Play Preview"))
		Play_GroupPreview();
	ImGui::SameLine();
	ImGui::BeginDisabled(0u == m_iGroupPreviewHandle);
	if (ImGui::Button("Stop Preview"))
		Stop_GroupPreview();
	ImGui::EndDisabled();
	if (fPreviewSeconds >= 0.f)
	{
		ImGui::SameLine();
		ImGui::Text("%.2f s", fPreviewSeconds);
	}
	if (!m_strGroupStatus.empty())
		ImGui::TextWrapped("%s", m_strGroupStatus.c_str());
	ImGui::End();
}
```

**G3 종료 증거**: Client Debug 빌드. F1 → Effect Tool v2 → `Group...` 창에서 New → 자식 2개(현재 Effect ID) 추가 → Save →
`Data/Effects/V2/Groups/<id>.effectv2group.json` 생성 확인 → Play Preview에서 자식이 `startMs` 순서로 뜨고 노란 playhead가
움직이는지, Attach 창 `Bind As`에 그 그룹이 보이고 `Add / Update Binding → Save Bindings` 뒤 파일에 `"group": …` row가
생기는지(사용자 확인). 저장 후 Reload가 같은 row를 다시 읽는지.

## G4. 검증기 — 그룹 문서와 `group` 바인딩

### 4-1. `Tools/EffectToolV2/validate_effect_v2.py` (전체 교체)

```python
"""Fail-closed validator for the preserved Effect Tool V2 contract."""

from __future__ import annotations

import argparse
import collections
import json
import math
from pathlib import Path, PurePosixPath
from typing import Any


AUTHORED_SCHEMA = "lostark.effect-v2"
BINDING_SCHEMA = "lostark.effect-v2-bindings"
GROUP_SCHEMA = "lostark.effect-v2-group"
EFFECT_TYPES = {"Decal", "Mesh", "Particle", "ScreenPost", "Texture", "Trail"}
SLOT_NAMES = ("mesh", "base", "noise", "mask", "emissive", "dissolve")
TEXTURE_SLOT_NAMES = ("base", "noise", "mask", "emissive", "dissolve")
ROTATIONS = {"Bone", "TargetYaw", "World"}
CHILD_STOPS = {"Kill", "Deactivate"}
MAX_MS = 600000


class ContractError(ValueError):
    pass


def _reject_non_finite(value: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {value}")


def _read_json(path: Path) -> Any:
    try:
        with path.open(encoding="utf-8") as handle:
            return json.load(handle, parse_constant=_reject_non_finite)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise ContractError(f"invalid JSON: {path}: {exc}") from exc


def _require_object(value: Any, owner: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ContractError(f"{owner} must be an object")
    return value


def _require_ms(value: Any, owner: str, key: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < 0 or value > MAX_MS:
        raise ContractError(f"{owner} {key} must be an integer in [0, {MAX_MS}]")
    return value


def _require_finite_number(value: Any, owner: str, key: str) -> None:
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(float(value)):
        raise ContractError(f"{owner} {key} must be a finite number")


def _require_relative_asset(asset_id: str, slot: str, owner: str) -> None:
    if not asset_id:
        return
    if "\\" in asset_id:
        raise ContractError(f"{owner} {slot} uses a backslash path: {asset_id}")
    path = PurePosixPath(asset_id)
    if path.is_absolute() or ".." in path.parts or ":" in asset_id:
        raise ContractError(f"{owner} {slot} escapes Resources: {asset_id}")
    expected_suffix = ".wmodel" if slot == "mesh" else ".dds"
    if path.suffix.lower() != expected_suffix:
        raise ContractError(
            f"{owner} {slot} must reference {expected_suffix}: {asset_id}"
        )


def _validate_numbers(value: Any, owner: str) -> None:
    if isinstance(value, bool) or value is None or isinstance(value, str):
        return
    if isinstance(value, (int, float)):
        if not math.isfinite(float(value)):
            raise ContractError(f"{owner} contains a non-finite number")
        return
    if isinstance(value, list):
        for item in value:
            _validate_numbers(item, owner)
        return
    if isinstance(value, dict):
        for item in value.values():
            _validate_numbers(item, owner)
        return
    raise ContractError(f"{owner} contains an unsupported JSON value")


def _validate_authored(
    authored_root: Path, resource_root: Path
) -> tuple[dict[str, Path], dict[str, collections.Counter[str]]]:
    by_id: dict[str, Path] = {}
    texture_usage: dict[str, collections.Counter[str]] = collections.defaultdict(
        collections.Counter
    )
    paths = sorted(authored_root.glob("*.effectv2.json"))
    if not paths:
        raise ContractError("Effect V2 authored set is empty")
    for path in paths:
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != AUTHORED_SCHEMA or document.get("formatVersion") != 1:
            raise ContractError(f"unsupported Effect V2 document: {path}")
        effect_id = document.get("effectId")
        if not isinstance(effect_id, str) or not effect_id:
            raise ContractError(f"Effect V2 effectId is missing: {path}")
        if path.name != f"{effect_id}.effectv2.json":
            raise ContractError(f"Effect V2 filename/ID mismatch: {path}: {effect_id}")
        if effect_id in by_id:
            raise ContractError(f"duplicate Effect V2 effectId: {effect_id}")
        if document.get("effectType") not in EFFECT_TYPES:
            raise ContractError(f"unsupported Effect V2 effectType: {effect_id}")
        slots = _require_object(document.get("slots"), f"{effect_id}.slots")
        if set(slots) != set(SLOT_NAMES):
            raise ContractError(f"Effect V2 slots are not exact: {effect_id}")
        for slot in SLOT_NAMES:
            asset_id = slots[slot]
            if not isinstance(asset_id, str):
                raise ContractError(f"Effect V2 slot must be a string: {effect_id}: {slot}")
            _require_relative_asset(asset_id, slot, effect_id)
            if not asset_id:
                continue
            physical = resource_root.joinpath(*PurePosixPath(asset_id).parts)
            if not physical.is_file():
                raise ContractError(
                    f"Effect V2 resource is missing: {effect_id}: {slot}: {asset_id}"
                )
            if slot in TEXTURE_SLOT_NAMES:
                texture_usage[PurePosixPath(asset_id).name.lower()][slot] += 1
        _require_object(document.get("params"), f"{effect_id}.params")
        _validate_numbers(document, effect_id)
        by_id[effect_id] = path
    return by_id, texture_usage


def _validate_groups(group_root: Path, authored: dict[str, Path]) -> dict[str, list[str]]:
    """Group documents are optional; each child must be an authored leaf (no nesting)."""
    groups: dict[str, list[str]] = {}
    if not group_root.is_dir():
        return groups
    for path in sorted(group_root.glob("*.effectv2group.json")):
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != GROUP_SCHEMA or document.get("formatVersion") != 1:
            raise ContractError(f"unsupported Effect V2 group document: {path}")
        group_id = document.get("groupId")
        if not isinstance(group_id, str) or not group_id:
            raise ContractError(f"Effect V2 groupId is missing: {path}")
        if path.name != f"{group_id}.effectv2group.json":
            raise ContractError(f"Effect V2 group filename/ID mismatch: {path}: {group_id}")
        if group_id in groups:
            raise ContractError(f"duplicate Effect V2 groupId: {group_id}")
        if group_id in authored:
            raise ContractError(f"Effect V2 groupId collides with an authored effect: {group_id}")
        _require_ms(document.get("durationMs", 0), group_id, "durationMs")
        children = document.get("children")
        if not isinstance(children, list) or not children:
            raise ContractError(f"Effect V2 group children must be a non-empty array: {group_id}")
        child_ids: list[str] = []
        identities: set[tuple[str, int]] = set()
        for child in children:
            child = _require_object(child, f"{group_id}.child")
            effect_id = child.get("effectId")
            if effect_id not in authored:
                raise ContractError(
                    f"Effect V2 group child has no authored effect (nesting is not allowed): {group_id}: {effect_id}"
                )
            start_ms = _require_ms(child.get("startMs", 0), group_id, "child startMs")
            _require_ms(child.get("durationMs", 0), group_id, "child durationMs")
            if child.get("stop", "Deactivate") not in CHILD_STOPS:
                raise ContractError(f"Effect V2 group child stop is invalid: {group_id}: {effect_id}")
            offset = child.get("offset", [0, 0, 0])
            if not isinstance(offset, list) or len(offset) != 3:
                raise ContractError(f"Effect V2 group child offset must be 3 numbers: {group_id}: {effect_id}")
            for component in offset:
                _require_finite_number(component, group_id, "child offset")
            _require_finite_number(child.get("yawDegrees", 0), group_id, "child yawDegrees")
            identity = (effect_id, start_ms)
            if identity in identities:
                raise ContractError(f"duplicate Effect V2 group child: {group_id}: {identity}")
            identities.add(identity)
            child_ids.append(effect_id)
        _validate_numbers(document, group_id)
        groups[group_id] = child_ids
    return groups


def _validate_bindings(
    binding_root: Path, authored: dict[str, Path], groups: dict[str, list[str]]
) -> int:
    seen_archetypes: set[str] = set()
    seen_effects: set[str] = set()
    seen_groups: set[str] = set()
    binding_count = 0
    paths = sorted(binding_root.glob("*.effectv2bindings.json"))
    if not paths:
        raise ContractError("Effect V2 binding set is empty")
    for path in paths:
        document = _require_object(_read_json(path), path.as_posix())
        if document.get("schema") != BINDING_SCHEMA or document.get("formatVersion") != 1:
            raise ContractError(f"unsupported Effect V2 binding document: {path}")
        archetype_id = document.get("archetypeId")
        if not isinstance(archetype_id, str) or not archetype_id:
            raise ContractError(f"Effect V2 archetypeId is missing: {path}")
        if archetype_id in seen_archetypes:
            raise ContractError(f"duplicate Effect V2 archetypeId: {archetype_id}")
        seen_archetypes.add(archetype_id)
        rows = document.get("bindings")
        if not isinstance(rows, list):
            raise ContractError(f"Effect V2 bindings must be an array: {path}")
        row_identities: set[tuple[Any, ...]] = set()
        for row in rows:
            row = _require_object(row, f"{archetype_id}.binding")
            effect_id = row.get("effectId")
            group_id = row.get("group")
            has_effect = isinstance(effect_id, str) and bool(effect_id)
            has_group = isinstance(group_id, str) and bool(group_id)
            if has_effect == has_group:
                raise ContractError(
                    f"Effect V2 binding needs exactly one effectId/group: {archetype_id}: {effect_id}/{group_id}"
                )
            subject = effect_id if has_effect else group_id
            if has_effect and effect_id not in authored:
                raise ContractError(f"Effect V2 binding has no authored effect: {effect_id}")
            if has_group and group_id not in groups:
                raise ContractError(f"Effect V2 binding has no group document: {group_id}")
            stage = row.get("stage")
            clip = row.get("clip")
            if (isinstance(stage, str) and bool(stage)) == (
                isinstance(clip, str) and bool(clip)
            ):
                raise ContractError(
                    f"Effect V2 binding needs exactly one stage/clip: {archetype_id}: {subject}"
                )
            start_ms = _require_ms(row.get("startMs"), archetype_id, f"{subject} startMs")
            if not isinstance(row.get("bone"), str):
                raise ContractError(f"Effect V2 bone is invalid: {archetype_id}: {subject}")
            if not isinstance(row.get("followBone"), bool):
                raise ContractError(f"Effect V2 followBone is invalid: {archetype_id}: {subject}")
            if row.get("rotation") not in ROTATIONS:
                raise ContractError(f"Effect V2 rotation is invalid: {archetype_id}: {subject}")
            if not isinstance(row.get("stopWithClip"), bool):
                raise ContractError(f"Effect V2 stopWithClip is invalid: {archetype_id}: {subject}")
            identity = (effect_id, group_id, stage, clip, start_ms, row.get("bone"))
            if identity in row_identities:
                raise ContractError(f"duplicate Effect V2 binding row: {archetype_id}: {identity}")
            row_identities.add(identity)
            if has_effect:
                seen_effects.add(effect_id)
            else:
                seen_groups.add(group_id)
                seen_effects.update(groups[group_id])
            binding_count += 1
    missing = sorted(set(authored) - seen_effects)
    if missing:
        raise ContractError(f"unbound Effect V2 authored effects: {missing[:5]}")
    missing_groups = sorted(set(groups) - seen_groups)
    if missing_groups:
        raise ContractError(f"unbound Effect V2 groups: {missing_groups[:5]}")
    return binding_count


def validate(repository_root: Path, resource_root: Path) -> dict[str, int]:
    v2_root = repository_root / "Data/Effects/V2"
    authored, usage = _validate_authored(v2_root / "Authored", resource_root)
    groups = _validate_groups(v2_root / "Groups", authored)
    binding_count = _validate_bindings(v2_root / "Bindings", authored, groups)
    return {
        "authored": len(authored),
        "bindings": binding_count,
        "groups": len(groups),
        "textures": len(usage),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--repository-root", type=Path, default=Path(__file__).resolve().parents[2]
    )
    parser.add_argument("--resource-root", type=Path)
    arguments = parser.parse_args()
    repository_root = arguments.repository_root.resolve()
    resource_root = (
        arguments.resource_root.resolve()
        if arguments.resource_root
        else repository_root / "Client/Bin/Resources"
    )
    try:
        report = validate(repository_root, resource_root)
    except ContractError as exc:
        print(f"Effect V2 validation failed: {exc}")
        return 1
    print(
        "Effect V2 validation succeeded: "
        f"{report['authored']} authored, {report['bindings']} bindings, "
        f"{report['groups']} groups, {report['textures']} textures."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

`ROTATIONS`에 `"World"`를 추가한 것은 C++ `PIVOT_ROTATION_KEYS`(`Bone, TargetYaw, World`)와 기존 검증기의 불일치를 같은
변경에서 맞춘 것이다(툴이 `World`를 저장하면 gate가 거부하던 상태).

### 4-2. `Tools/EffectToolV2/test_validate_effect_v2.py` (전체 교체)

```python
from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("validate_effect_v2.py")
SPEC = importlib.util.spec_from_file_location("validate_effect_v2", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
VALIDATOR = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = VALIDATOR
SPEC.loader.exec_module(VALIDATOR)


class EffectV2ValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.resource_root = self.root / "Client/Bin/Resources"
        self.authored_root = self.root / "Data/Effects/V2/Authored"
        self.binding_root = self.root / "Data/Effects/V2/Bindings"
        self.group_root = self.root / "Data/Effects/V2/Groups"
        self.authored_root.mkdir(parents=True)
        self.binding_root.mkdir(parents=True)
        resource = self.resource_root / "Effect/Test/base.dds"
        resource.parent.mkdir(parents=True)
        resource.write_bytes(b"dds")
        self.document = {
            "schema": "lostark.effect-v2",
            "formatVersion": 1,
            "effectId": "effect.test.one",
            "effectType": "Particle",
            "slots": {
                "mesh": "",
                "base": "Effect/Test/base.dds",
                "noise": "",
                "mask": "",
                "emissive": "",
                "dissolve": "",
            },
            "params": {},
        }
        self.binding = {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 1,
            "archetypeId": "NPC_TEST",
            "bindings": [
                {
                    "effectId": "effect.test.one",
                    "clip": "test_clip",
                    "startMs": 0,
                    "bone": "b_effectroot",
                    "followBone": False,
                    "rotation": "TargetYaw",
                    "stopWithClip": False,
                }
            ],
        }
        self.group = {
            "schema": "lostark.effect-v2-group",
            "formatVersion": 1,
            "groupId": "group.test.one",
            "durationMs": 0,
            "children": [
                {
                    "effectId": "effect.test.one",
                    "startMs": 100,
                    "durationMs": 500,
                    "stop": "Deactivate",
                    "offset": [0.0, 0.0, 0.0],
                    "yawDegrees": 0.0,
                }
            ],
        }
        self._write_fixture(self.document, self.binding)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _write_json(self, path: Path, value: object) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

    def _write_fixture(self, document: dict, binding: dict) -> None:
        self._write_json(
            self.authored_root / "effect.test.one.effectv2.json", document
        )
        self._write_json(self.binding_root / "NPC_TEST.effectv2bindings.json", binding)

    def _write_group(self, group: dict) -> None:
        self._write_json(self.group_root / f"{group['groupId']}.effectv2group.json", group)

    def _group_binding(self) -> dict:
        binding = copy.deepcopy(self.binding)
        row = binding["bindings"][0]
        del row["effectId"]
        row["group"] = "group.test.one"
        return binding

    def test_exact_authored_binding_and_resource_join_passes(self) -> None:
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report, {"authored": 1, "bindings": 1, "groups": 0, "textures": 1})

    def test_missing_resource_fails_closed(self) -> None:
        (self.resource_root / "Effect/Test/base.dds").unlink()
        with self.assertRaisesRegex(VALIDATOR.ContractError, "resource is missing"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_unknown_binding_effect_fails_closed(self) -> None:
        binding = copy.deepcopy(self.binding)
        binding["bindings"][0]["effectId"] = "effect.test.missing"
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "no authored effect"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_non_finite_number_fails_closed(self) -> None:
        document = copy.deepcopy(self.document)
        document["params"]["lifetime"] = float("nan")
        self._write_fixture(document, self.binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "non-finite JSON number"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_path_escape_fails_closed(self) -> None:
        document = copy.deepcopy(self.document)
        document["slots"]["base"] = "../outside.dds"
        self._write_fixture(document, self.binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "escapes Resources"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_group_binding_counts_children_as_bound(self) -> None:
        self._write_group(self.group)
        self._write_fixture(self.document, self._group_binding())
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report, {"authored": 1, "bindings": 1, "groups": 1, "textures": 1})

    def test_unbound_group_fails_closed(self) -> None:
        self._write_group(self.group)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "unbound Effect V2 groups"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_nested_group_child_fails_closed(self) -> None:
        self._write_group(self.group)
        nested = copy.deepcopy(self.group)
        nested["groupId"] = "group.test.outer"
        nested["children"][0]["effectId"] = "group.test.one"
        self._write_group(nested)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "nesting is not allowed"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_binding_with_effect_and_group_fails_closed(self) -> None:
        self._write_group(self.group)
        binding = copy.deepcopy(self.binding)
        binding["bindings"][0]["group"] = "group.test.one"
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "exactly one effectId/group"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_unknown_group_binding_fails_closed(self) -> None:
        binding = self._group_binding()
        binding["bindings"][0]["group"] = "group.test.missing"
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "no group document"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_group_child_stop_value_fails_closed(self) -> None:
        group = copy.deepcopy(self.group)
        group["children"][0]["stop"] = "Fade"
        self._write_group(group)
        self._write_fixture(self.document, self._group_binding())
        with self.assertRaisesRegex(VALIDATOR.ContractError, "child stop is invalid"):
            VALIDATOR.validate(self.root, self.resource_root)


if __name__ == "__main__":
    unittest.main()
```

**G4 종료 증거**:

```powershell
$env:PATH = "C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin;" + $env:PATH
python Tools\EffectToolV2\test_validate_effect_v2.py      # Ran 11 tests ... OK
python Tools\EffectToolV2\validate_effect_v2.py           # Effect V2 validation succeeded: N authored, M bindings, 0 groups, T textures.
```

## 작성 순서와 검증

1. G1 문서 계층 → Client 컴파일(`/t:ClCompile`) → 기존 바인딩 파일 Reload.
2. G2 오브젝트·런타임 → Client 빌드 → Attach 창 `Runtime spawns on target`으로 기존 문서 바인딩 회귀(도철 3종).
3. G3 툴 → Client 빌드 → Group 창 New/Save/Play Preview/`Bind As` → 저장 파일 확인.
4. G4 검증기 → python 테스트 11개 OK → `validate_effect_v2.py` 성공.
5. `git diff --check`, `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -AllowLocalEffectResources`(python gate에 4-2 테스트 포함).
6. 시각 판정(사용자): Group 미리보기에서 자식 offset/yaw가 pivot 기준으로 놓이는지, Deactivate 자식의 파티클이 스폰만 멈추고
   잔여가 자연 소멸하는지, Kill 자식이 즉시 사라지는지.

불변식:
- 그룹은 시계와 배치만 소유한다. 자식 문서의 look 파라미터를 그룹이 덮어쓰지 않는다.
- 바인딩 row는 `effectId`/`group` 중 하나, `clip`/`stage` 중 하나만 갖는다. 나머지 필드는 그룹 자식 전부에 복사된다.
- lane 시계는 바인딩의 것 하나뿐이다. 자식 시각은 전부 그 시계의 오프셋이고, 그룹 `durationMs`는 자식 stop의 상한이다.
- 자식 stop은 lane당 한 번만 적용된다(`bStopApplied`). lane이 되감기면 미스폰 상태로 돌아가지만 이미 적용된 stop은 되돌리지 않는다.
- free-group lane은 타깃을 갖지 않으므로 `followBone`이 항상 false이고 pivot은 `Play_Group` 인자로 고정된다.

## 후속 (이 계획 범위 밖)

- 발탄 포탈 자식 문서 7개 저작(`boss.valtan.portal.flame/card/flash/stone/spark/streak/dark`)과 그룹 1개,
  `BOSS_VALTAN` stage 바인딩 `ghost-finale.step-02 ~ step-09` 8줄.
- 메시 파티클(돌 파편 ×21)은 V2 `Particle`에 메시 인스턴싱이 없어 별도 결정이 필요하다.
- HDR `colorMul > 1` 허용 여부는 `Shader_EffectV2_Common.hlsli`의 `g_ColorMul` 경로를 확인한 뒤 결정한다.
