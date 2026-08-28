# 2026-08-28 Effect Tool v2 Attach — 발탄(BOSS_VALTAN) 타깃 추가 PLAN

작성자: JS · branch `feature/effect-v2-valtan-attach` (main `cd120501` 기준, v2 브랜치 3개 모두 main에 병합됨)
선행: `../2026-08-22_EFFECT_TOOL_V2_G01_TYPE_SLOT_PREVIEW_PLAN.md` G10(Attach)·G11(v2 런타임)

## 목표

Effect Tool v2 `Effect Attach v2` 창에서 NPC와 같은 방식으로 **발탄**을 소환해 클립 재생/스크럽·본 피벗·
바인딩 저장을 하고, 저장한 `BOSS_VALTAN.effectv2bindings.json`이 인게임 발탄(Server 권위 패턴 클립)에서도
같은 v2 런타임으로 스폰되게 한다. Server·Shared·Data schema 변경 없음(표현 전용).

## 실측 근거 (2026-08-28)

- v2 Attach/런타임은 타깃이 **`CNpc`로 하드타이핑**돼 있다: 툴 `m_pTarget(weak CNpc)`,
  `CEffectV2Object::Set_FollowTarget(weak CNpc)`/`Resolve_TargetPivot(const CNpc&)`(EffectV2_Object.h:334-344),
  `CEffectV2Runtime::Notify_NpcClip/Tick_Npc/Set_Ignored(shared CNpc)`(EffectV2_Runtime.h), 런타임 상태 map 키
  `const CNpc*`, archetype은 `CNpc::Get_ModelTag()` 역매핑(EffectV2_Runtime.cpp:71-86). 호출자는 `Npc.cpp:98,128,241`
  (Set_Animation/Play_NetworkAction 성공 시 Notify, Update 끝 Tick)과 `NpcPresentationAssetService.cpp:173`(Prewarm seam).
- 발탄 GameObject는 `CValtan`(ContainerObject). body는 `CBody_Valtan` part(`BODY_PART_TAG`)이고 `Get_BodyModel()`이
  그 CModel, `Get_Transform()`이 owner world. **본 행렬 → 월드 변환은 `Bone × VisualRoot × Owner`** 이며
  `Try_Get_PresentationRootMatrix(&out)`가 `VisualRoot × Owner`를 준다(Valtan.cpp:1360-1373; 무기 소켓도
  `pSocketRootMatrix = m_pBodyVisualRootCom`로 같은 합성). NPC는 `Bone × Owner`라 root가 다르다. yaw 기준은
  둘 다 owner world.
- 툴용 로컬 발탄 소환 레시피는 `CharacterPreviewPanel.cpp:306-351`에 이미 있다:
  `CValtanPresentationAssetService::Ensure_Prototypes(dev, ctx, level)` → `VALTAN_DESC{iPrototypeLevelIndex, vPosition,
  fScale = pBoss->presentationScale, fCollisionRadius 0, isServerAuthoritative false}` →
  `Add_GameObject_to_Layer(level, TEXT("Prototype_GameObject_Valtan"), level, layer)`. navigation tag null이면
  `CValtan::Update`가 `Set_ChaseState(false)`만 호출(idle 유지, 상태 불변 시 clip 재설정 없음, Valtan.cpp:1267-1273)
  → 툴이 body model에 직접 건 클립을 덮어쓰지 않는다.
- 발탄 body model은 `BOSS_ACTOR_ENTRY::bodyModel`(Resources 상대 asset id, ValtanPresentationAssetService.cpp:52)로
  `CModel::Create(…, XMMatrixScaling(0.0001))`; anim set 146클립을 Attach한다 → 툴의 Target Clip 콤보는
  `Get_BodyModel()`의 `mesh_*` 클립 전부를 나열한다. 본 이름 목록은 `Collect_BoneNames(pBoss->bodyModel)`로 NPC와 동일.
- 인게임 발탄 클립 edge는 세 곳: 패턴 stage 체인 `Start_Animation`(Valtan.cpp:588), 비패턴 action
  `Start_Animation`(2044), 로컬/preview `Set_ChaseState`(1553). seek는 `Set_Animation(index)`로 같은 클립 유지(596).
  틱 위치는 `CValtan::Late_Update`(1342, 단일 exit) — body part가 `__super::Update`에서 애니를 진행한 뒤라 진행 시각이 최신.
- archetype ID: `CActorCatalog::Find_Boss("BOSS_VALTAN")`(BOSS_ACTOR_ENTRY::archetypeId). 바인딩 파일은
  `CEffectV2Document::Binding_Path(archetypeId)`가 그대로 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`을
  만들고 로드 검증은 archetypeId 문자열 일치뿐(EffectV2_Document.cpp:597-604) → 문서 계약 변경 없음.
- Prewarm seam: NPC는 `Ensure_Prototypes` 성공 끝에 `CEffectV2Runtime::Prewarm_Archetype`. 발탄은
  `CValtanPresentationAssetService::Ensure_Prototypes`의 `g_ReadyLevels.insert` 직후가 같은 자리(Loader worker에서도
  호출되며 NPC seam과 동일 threading 계약).
- 인코딩: `Valtan.cpp`, `Npc.cpp`, `EffectV2_*`, `Effect_Tool_V2.*`, `ValtanPresentationAssetService.cpp` 모두 ASCII/UTF-8(한글 주석 없음).

## 계약 (결정)

| 항목 | 값 | 이유 |
|---|---|---|
| 타깃 추상화 | `EFFECT_V2_TARGET{eKind NPC|VALTAN, weak_ptr<CGameObject> pOwner, const CGameObject* pKey}` + `Resolve_TargetView → {pModel, BoneRoot, YawBasis}` | CNpc/CValtan에 인터페이스 base를 넣지 않고(팀 파일 최소 변경) 피벗 계산을 한 곳에 둠 |
| 피벗 합성 | NPC `Bone × OwnerWorld`, VALTAN `Bone × PresentationRoot`. TARGET_YAW basis는 둘 다 OwnerWorld. 본 스케일 제거는 기존 그대로 | 무기 소켓과 동일한 공간 |
| archetype | VALTAN kind → 상수 `"BOSS_VALTAN"`; NPC kind → 기존 model tag 역매핑 | 발탄은 단일 archetype |
| 바인딩 문서 | `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`, 형식 동일(formatVersion 1) | 파서·직렬화 변경 없음 |
| 인게임 훅 | `CValtan`: Start_Animation 성공 2곳 + Set_ChaseState에 `Notify_Clip`, `Late_Update` 끝에 `Tick` (총 4줄+include) | NPC 훅과 대칭, 동작 변화 없음 |
| Prewarm | `CValtanPresentationAssetService::Ensure_Prototypes` 성공 끝 1줄 | 스폰 히치 방지, NPC와 동일 |
| 툴 소환 | `Layer_EffectPreviewV2Target`에 `isServerAuthoritative=false` Valtan, 캐릭터 옆 2.5 m, 초기 클립 `presentationClips.idle`, 기본 피벗 본 `b_effectroot`(없으면 첫 본) | CharacterPreviewPanel 레시피 재사용 |
| 툴 이동 | NPC `Apply_NetworkState`, VALTAN `Get_Transform()->Set_State(POSITION)`+`Rotation(0,yaw,0)` | 로컬 Valtan은 network 보간이 없음 |
| 툴 클립 재생 | 두 kind 모두 view의 `pModel->Set_Animation` 직접 + `CEffectV2Runtime::Notify_Clip` 명시 호출 | CValtan에는 public Set_Animation이 없음 |
| 패턴 체인 | 범위 밖. 바인딩은 클립 단위(기존과 동일). 인게임 패턴 stage가 그 클립을 시작하면 발화 | 체인 재생 툴은 Animation Tool 소유 |
| 팀장 합의 | v2 런타임 경로 자체(B안)는 기존과 같이 합의 항목. 이번 변경은 그 경로의 타깃 확장 | — |

## 파일 목록

| 구분 | 경로 | 역할 |
|---|---|---|
| 추가 | `Client/Public/EffectV2_Target.h` | `EFFECT_V2_TARGET_KIND / EFFECT_V2_TARGET / EFFECT_V2_TARGET_VIEW` 선언 |
| 수정 | `Client/Public/EffectV2_Object.h`, `Client/Private/EffectV2_Object.cpp` | follow target을 `EFFECT_V2_TARGET`으로, view 기반 피벗 해석 |
| 수정 | `Client/Public/EffectV2_Runtime.h`, `Client/Private/EffectV2_Runtime.cpp` | `Notify_Clip/Tick/Set_Ignored(EFFECT_V2_TARGET)`, VALTAN archetype |
| 수정 | `Client/Private/Npc.cpp` | 호출 3곳을 `EFFECT_V2_TARGET::From_Npc`로 |
| 수정 | `Client/Private/Valtan.cpp` | 클립 edge Notify 3곳 + Late_Update Tick |
| 수정 | `Client/Private/ValtanPresentationAssetService.cpp` | Prewarm seam |
| 수정 | `Client/Public/Effect_Tool_V2.h`, `Client/Private/Effect_Tool_V2.cpp` | Attach 창 Valtan 타깃 |
| 수정 | `Client/Default/Client.vcxproj`, `Client.vcxproj.filters` | 새 헤더 등록 |

## G1. 타깃 추상화 + 런타임 일반화 + 발탄 훅

### 5-1. `Client/Public/EffectV2_Target.h` (새 파일, UTF-8 BOM 없음)

- `EFFECT_V2_TARGET`: 이펙트가 따라갈 엔티티의 weak 핸들. `pKey`는 런타임 상태 map/ignore set의 identity 전용 raw 포인터이며 역참조하지 않는다(수명은 `pOwner`로 판단).
- `EFFECT_V2_TARGET_VIEW`: 한 번의 해석 결과. `BoneRoot`는 `Get_BoneMatrix` 결과를 월드로 보내는 행렬, `YawBasis`는 TARGET_YAW 회전의 기준(owner world).
- `From_Npc/From_Valtan` 정의는 `EffectV2_Object.cpp`(Npc.h/Valtan.h를 그쪽만 include).

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Engine)
class CGameObject;
class CModel;
NS_END

NS_BEGIN(Client)

class CNpc;
class CValtan;

enum class EFFECT_V2_TARGET_KIND : uint8_t
{
	NONE,
	NPC,
	VALTAN
};

/* The entity a v2 effect follows. pKey is identity only for runtime maps;
   liveness always comes from pOwner. */
struct EFFECT_V2_TARGET final
{
	EFFECT_V2_TARGET_KIND eKind = EFFECT_V2_TARGET_KIND::NONE;
	std::weak_ptr<Engine::CGameObject> pOwner;
	const Engine::CGameObject* pKey = nullptr;

	bool_t Is_Valid() const
	{
		return EFFECT_V2_TARGET_KIND::NONE != eKind && !pOwner.expired();
	}
	void Reset()
	{
		eKind = EFFECT_V2_TARGET_KIND::NONE;
		pOwner.reset();
		pKey = nullptr;
	}

	static EFFECT_V2_TARGET From_Npc(const std::shared_ptr<CNpc>& pNpc);
	static EFFECT_V2_TARGET From_Valtan(const std::shared_ptr<CValtan>& pValtan);
};

/* BoneRoot maps CModel::Get_BoneMatrix into world space (NPC: owner world;
   Valtan: body visual root x owner world). YawBasis is the owner world for
   PIVOT_ROTATION::TARGET_YAW on both kinds. */
struct EFFECT_V2_TARGET_VIEW final
{
	std::shared_ptr<Engine::CModel> pModel;
	float4x4_t BoneRoot{};
	float4x4_t YawBasis{};
};

NS_END
```

### 5-2. `Client/Public/EffectV2_Object.h`

적용 위치 1: `class CNpc;` 전방 선언 한 줄을 다음으로 교체

```cpp
class CNpc;
class CValtan;
```

적용 위치 2: 기존 include 블록의 `#include "Engine_Defines.h"`(또는 마지막 프로젝트 include) 바로 아래

```cpp
#include "EffectV2_Target.h"
```

적용 위치 3: public 선언 블록 `void Set_FollowTarget(const std::weak_ptr<CNpc>& pTarget, std::string strBone, PIVOT_ROTATION eRotation);`
부터 `static bool_t Resolve_TargetPivot(const CNpc& Npc, const std::string& strBone, PIVOT_ROTATION eRotation, float4x4_t& OutPivot);`
까지(`Clear_FollowTarget`, `Has_FollowTarget` 포함, EffectV2_Object.h:334-344) 전체 교체

```cpp
	void Set_FollowTarget(
		const EFFECT_V2_TARGET& Target,
		std::string strBone,
		PIVOT_ROTATION eRotation);
	void Clear_FollowTarget();
	bool_t Has_FollowTarget() const { return m_bFollowTarget; }
	static bool_t Resolve_TargetView(
		const EFFECT_V2_TARGET& Target,
		EFFECT_V2_TARGET_VIEW& OutView);
	static bool_t Resolve_TargetPivot(
		const EFFECT_V2_TARGET_VIEW& View,
		const std::string& strBone,
		PIVOT_ROTATION eRotation,
		float4x4_t& OutPivot);
```

적용 위치 4: private 멤버 `std::weak_ptr<CNpc> m_pFollowTarget;`(EffectV2_Object.h:404) 한 줄 교체

```cpp
	EFFECT_V2_TARGET m_FollowTarget;
```

### 5-3. `Client/Private/EffectV2_Object.cpp`

적용 위치 1: 기존 `#include "Npc.h"` 바로 아래(없으면 include 블록 끝)

```cpp
#include "Valtan.h"
```

적용 위치 2: `Set_FollowTarget` 정의부터 `Resolve_TargetPivot` 정의 끝(`return true;` 다음 `}`, EffectV2_Object.cpp:544-611)까지 전체 교체

```cpp
Client::EFFECT_V2_TARGET Client::EFFECT_V2_TARGET::From_Npc(
	const std::shared_ptr<CNpc>& pNpc)
{
	EFFECT_V2_TARGET Target;
	if (nullptr == pNpc)
		return Target;
	Target.eKind = EFFECT_V2_TARGET_KIND::NPC;
	Target.pOwner = pNpc;
	Target.pKey = pNpc.get();
	return Target;
}

Client::EFFECT_V2_TARGET Client::EFFECT_V2_TARGET::From_Valtan(
	const std::shared_ptr<CValtan>& pValtan)
{
	EFFECT_V2_TARGET Target;
	if (nullptr == pValtan)
		return Target;
	Target.eKind = EFFECT_V2_TARGET_KIND::VALTAN;
	Target.pOwner = pValtan;
	Target.pKey = pValtan.get();
	return Target;
}

void Client::CEffectV2Object::Set_FollowTarget(
	const EFFECT_V2_TARGET& Target,
	std::string strBone,
	const PIVOT_ROTATION eRotation)
{
	m_FollowTarget = Target;
	m_strFollowBone = std::move(strBone);
	m_eFollowRotation = eRotation;
	m_bFollowTarget = m_FollowTarget.Is_Valid();
}

void Client::CEffectV2Object::Clear_FollowTarget()
{
	m_bFollowTarget = false;
	m_FollowTarget.Reset();
	m_strFollowBone.clear();
}

bool_t Client::CEffectV2Object::Resolve_TargetView(
	const EFFECT_V2_TARGET& Target,
	EFFECT_V2_TARGET_VIEW& OutView)
{
	const std::shared_ptr<CGameObject> pOwner = Target.pOwner.lock();
	if (nullptr == pOwner)
		return false;
	switch (Target.eKind)
	{
	case EFFECT_V2_TARGET_KIND::NPC:
	{
		const std::shared_ptr<CNpc> pNpc = std::static_pointer_cast<CNpc>(pOwner);
		if (nullptr == pNpc->Get_Model() || nullptr == pNpc->Get_Transform())
			return false;
		OutView.pModel = pNpc->Get_Model();
		OutView.BoneRoot = *pNpc->Get_Transform()->Get_WorldMatrixPtr();
		OutView.YawBasis = OutView.BoneRoot;
		return true;
	}
	case EFFECT_V2_TARGET_KIND::VALTAN:
	{
		const std::shared_ptr<CValtan> pValtan = std::static_pointer_cast<CValtan>(pOwner);
		if (nullptr == pValtan->Get_BodyModel() || nullptr == pValtan->Get_Transform() ||
			!pValtan->Try_Get_PresentationRootMatrix(&OutView.BoneRoot))
		{
			return false;
		}
		OutView.pModel = pValtan->Get_BodyModel();
		OutView.YawBasis = *pValtan->Get_Transform()->Get_WorldMatrixPtr();
		return true;
	}
	default:
		return false;
	}
}

bool_t Client::CEffectV2Object::Resolve_TargetPivot(
	const EFFECT_V2_TARGET_VIEW& View,
	const std::string& strBone,
	const PIVOT_ROTATION eRotation,
	float4x4_t& OutPivot)
{
	if (nullptr == View.pModel)
		return false;
	const matrix_t BoneRoot = XMLoadFloat4x4(&View.BoneRoot);
	const matrix_t YawBasis = XMLoadFloat4x4(&View.YawBasis);
	matrix_t Pivot = BoneRoot;
	if (!strBone.empty())
	{
		if (!View.pModel->Has_Bone(strBone.c_str()))
			return false;
		Pivot = View.pModel->Get_BoneMatrix(strBone.c_str()) * BoneRoot;
	}
	const vector_t Translation = XMVectorSetW(Pivot.r[3], 1.f);
	if (PIVOT_ROTATION::BONE == eRotation)
	{
		const vector_t Right = XMVector3Normalize(Pivot.r[0]);
		const vector_t Up = XMVector3Normalize(Pivot.r[1]);
		const vector_t Look = XMVector3Normalize(Pivot.r[2]);
		if (XMVectorGetX(XMVector3LengthSq(Right)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Up)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Look)) > 0.f)
		{
			Pivot.r[0] = Right;
			Pivot.r[1] = Up;
			Pivot.r[2] = Look;
		}
		else
			Pivot = XMMatrixIdentity();
	}
	else if (PIVOT_ROTATION::TARGET_YAW == eRotation)
	{
		Pivot.r[0] = XMVector3Normalize(YawBasis.r[0]);
		Pivot.r[1] = XMVector3Normalize(YawBasis.r[1]);
		Pivot.r[2] = XMVector3Normalize(YawBasis.r[2]);
	}
	else
		Pivot = XMMatrixIdentity();
	Pivot.r[3] = Translation;
	XMStoreFloat4x4(&OutPivot, Pivot);
	return true;
}
```

적용 위치 3: `Update` 안 follow 블록(EffectV2_Object.cpp:615-623) 교체

```cpp
	if (m_bFollowTarget)
	{
		EFFECT_V2_TARGET_VIEW View;
		if (!Resolve_TargetView(m_FollowTarget, View) ||
			!Resolve_TargetPivot(View, m_strFollowBone, m_eFollowRotation, m_PivotWorld))
		{
			m_bFinished = true;
		}
	}
```

### 5-4. `Client/Public/EffectV2_Runtime.h` (전체 교체)

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
	static void Prewarm_Archetype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strArchetypeId);
	static void Set_Ignored(const EFFECT_V2_TARGET& Target, bool_t bIgnored);
	static void Invalidate_Caches();
	static const std::string& Last_Error();
};

NS_END
```

### 5-5. `Client/Private/EffectV2_Runtime.cpp` (전체 교체)

변경 요지: 상태 map/ignore set 키를 `const Engine::CGameObject*`(= `EFFECT_V2_TARGET::pKey`)로, archetype은 kind로 분기, Spawn/Tick은 `Resolve_TargetView`를 거친다.

```cpp
#include "EffectV2_Runtime.h"
#include "ActorCatalog.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"

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

	struct PENDING_SPAWN final
	{
		Client::EFFECT_V2_BINDING Binding;
		bool_t bSpawned = false;
	};

	struct SPAWNED_EFFECT final
	{
		std::weak_ptr<Client::CEffectV2Object> pObject;
		bool_t bStopWithClip = false;
	};

	struct TARGET_STATE final
	{
		Client::EFFECT_V2_TARGET Target;
		std::string strArchetypeId;
		std::string strClip;
		f32_t fLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> Pending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Engine::CGameObject*, TARGET_STATE> g_TargetStates;
	std::set<const Engine::CGameObject*> g_IgnoredTargets;
	bool_t g_bPrototypeRegistered = false;
	std::string g_strLastError;

	void Report(const std::string& strMessage)
	{
		g_strLastError = strMessage;
		OutputDebugStringA(("[EffectV2Runtime] " + strMessage + "\n").c_str());
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

	void Spawn(
		TARGET_STATE& State,
		PENDING_SPAWN& Pending,
		const Client::EFFECT_V2_TARGET_VIEW& View,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		Pending.bSpawned = true;
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed || !Ensure_Prototype(pDevice, pContext))
			return;
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = Entry.Document.Desc;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			View,
			Pending.Binding.strBone,
			Pending.Binding.eRotation,
			Desc.PivotWorld))
		{
			Report("pivot bone not found: " + Pending.Binding.strBone +
				" for " + Pending.Binding.strEffectId);
			return;
		}
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
		if (Pending.Binding.bFollowBone)
		{
			pObject->Set_FollowTarget(
				State.Target, Pending.Binding.strBone, Pending.Binding.eRotation);
		}
		State.Spawned.push_back({ pObject, Pending.Binding.bStopWithClip });
	}

	void Prune_Spawned(TARGET_STATE& State, const bool_t bStopClipBound)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (auto Iterator = State.Spawned.begin(); Iterator != State.Spawned.end();)
		{
			const std::shared_ptr<Client::CEffectV2Object> pObject = Iterator->pObject.lock();
			if (nullptr != pObject && bStopClipBound && Iterator->bStopWithClip)
				pObject->Finish();
			if (nullptr == pObject || pObject->Is_Finished())
			{
				if (nullptr != pObject)
				{
					GameInstance.Remove_GameObject_from_Layer(
						GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG, pObject);
				}
				Iterator = State.Spawned.erase(Iterator);
				continue;
			}
			++Iterator;
		}
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
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Binding.strEffectId);
		if (Entry.bFailed)
			continue;
		std::string strError;
		if (FAILED(CEffectV2Object::Prewarm(pDevice, pContext, Entry.Document.Desc, strError)))
			Report("prewarm failed " + Binding.strEffectId + ": " + strError);
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
			Prune_Spawned(Found->second, true);
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
		if (Binding.strClip == State.strClip)
			State.Pending.push_back({ Binding, false });
	}
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
			Prune_Spawned(Iterator->second, true);
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
		if (fSeconds >= static_cast<f32_t>(Pending.Binding.iStartMs) / 1000.f)
			Spawn(State, Pending, View, pDevice, pContext);
	}
	Prune_Spawned(State, false);
}
```

### 5-6. `Client/Private/Npc.cpp`

적용 위치 1·2: `CEffectV2Runtime::Notify_NpcClip(static_pointer_cast<CNpc>(shared_from_this()), pClipName);`(Npc.cpp:98-99, 128-129) 두 곳 모두 교체

```cpp
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
```

적용 위치 3: `CEffectV2Runtime::Tick_Npc(static_pointer_cast<CNpc>(shared_from_this()), m_pDevice, m_pContext);`(Npc.cpp:241-242) 교체

```cpp
	CEffectV2Runtime::Tick(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		m_pDevice, m_pContext);
```

### 5-7. `Client/Private/Valtan.cpp` (훅 4곳 + include)

적용 위치 1: include 블록에 추가(예: `#include "ActorCatalog.h"` 바로 아래)

```cpp
#include "EffectV2_Runtime.h"
```

적용 위치 2: `Apply_PatternPresentationSample` 안 `if (bAnimationEdgeChanged || bClipOccurrenceTransition) { if (!m_pBodyModelCom->Start_Animation(TargetClip.strClipName.c_str(), TargetClip.bLoop)) { return false; } }`(Valtan.cpp:586-593) 블록을 다음으로 교체

```cpp
	if (bAnimationEdgeChanged || bClipOccurrenceTransition)
	{
		if (!m_pBodyModelCom->Start_Animation(
				TargetClip.strClipName.c_str(), TargetClip.bLoop))
		{
			return false;
		}
		Client::CEffectV2Runtime::Notify_Clip(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			TargetClip.strClipName.c_str());
	}
```

적용 위치 3: `Set_ChaseState` 안 `m_pBodyModelCom->Set_Animation(isChasing ? "mesh_run_battle_1" : "mesh_idle_battle_1", true);`(1553-1555) 바로 아래

```cpp
		Client::CEffectV2Runtime::Notify_Clip(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			isChasing ? "mesh_run_battle_1" : "mesh_idle_battle_1");
```

적용 위치 4: `Apply_NetworkState` 비패턴 분기 `if (bAnimationEdgeChanged && !m_pBodyModelCom->Start_Animation(pClip->c_str(), true)) { return false; }`(2043-2047) 바로 아래, `m_pBodyModelCom->Set_AnimationSpeed(1.f);` 위

```cpp
			if (bAnimationEdgeChanged)
			{
				Client::CEffectV2Runtime::Notify_Clip(
					Client::EFFECT_V2_TARGET::From_Valtan(
						static_pointer_cast<CValtan>(shared_from_this())),
					pClip->c_str());
			}
```

적용 위치 5: `CValtan::Late_Update` 안 `__super::Late_Update(fTimeDelta);`(1344) 바로 아래

```cpp
	Client::CEffectV2Runtime::Tick(
		Client::EFFECT_V2_TARGET::From_Valtan(
			static_pointer_cast<CValtan>(shared_from_this())),
		m_pDevice, m_pContext);
```

### 5-8. `Client/Private/ValtanPresentationAssetService.cpp`

적용 위치 1: include 블록에 `#include "EffectV2_Runtime.h"` 추가.
적용 위치 2: `Ensure_Prototypes` 끝 `g_ReadyLevels.insert(iLevelIndex);`(175) 바로 아래, `return S_OK;` 위

```cpp
	CEffectV2Runtime::Prewarm_Archetype(pDevice, pContext, "BOSS_VALTAN");
```

### 5-9. `Client/Default/Client.vcxproj` / `.filters`

vcxproj: `<ClInclude Include="..\Public\EffectV2_Runtime.h" />` 바로 아래

```xml
	<ClInclude Include="..\Public\EffectV2_Target.h" />
```

filters: `<ClInclude Include="..\Public\EffectV2_Runtime.h">` 항목 바로 아래

```xml
    <ClInclude Include="..\Public\EffectV2_Target.h">
      <Filter>03. Tools\06. Effect V2</Filter>
    </ClInclude>
```

**G1 종료 증거**: Client Debug 빌드 성공. 기존 NPC 경로 회귀 없음 — Attach 창 웨이 `NPC_58700` + `Runtime spawns on target`에서
1733/2966/4700 ms 도철 3종이 이전과 같이 스폰(사용자 확인). Valtan 맵 에스더 웨이 소환도 동일.

## G2. 툴 — Attach 창에 발탄 타깃

### 5-10. `Client/Public/Effect_Tool_V2.h`

적용 위치 1: `class CNpc;` 한 줄을 다음으로 교체

```cpp
class CNpc;
class CValtan;
```

적용 위치 2: private 함수 선언 `bool_t Spawn_Target(const std::string& strArchetypeId);` 바로 아래

```cpp
	bool_t Spawn_NpcTarget(const std::string& strArchetypeId, const float3_t& vPosition);
	bool_t Spawn_ValtanTarget(const float3_t& vPosition);
	bool_t Resolve_TargetView(EFFECT_V2_TARGET_VIEW& OutView) const;
	void Play_TargetClip(const char_t* pClipName, bool_t bLoop);
```

적용 위치 3: 멤버 `std::weak_ptr<CNpc> m_pTarget;` 와 `int32_t m_iTargetArchetypeSelection = -1;` 두 줄을 각각 교체

```cpp
	EFFECT_V2_TARGET m_Target;
```

```cpp
	std::string m_strSelectedArchetypeId;
```

### 5-11. `Client/Private/Effect_Tool_V2.cpp`

적용 위치 1: include 블록 `#include "Npc.h"` 바로 아래

```cpp
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"
```

적용 위치 2: 익명 namespace 상수(1271-1272) 아래에 추가

```cpp
	constexpr const char* VALTAN_TARGET_ARCHETYPE_ID = "BOSS_VALTAN";
```

적용 위치 3: `Spawn_Target` 정의부터 `Snap_PivotToTarget` 정의 끝(1293-1471)까지 전체 교체

```cpp
bool_t Client::CEffect_Tool_V2::Spawn_Target(const std::string& strArchetypeId)
{
	Despawn_Target();
	float3_t vPosition{ 0.f, 0.f, 0.f };
	if (const std::shared_ptr<CCharacter> pCharacter =
		CAnimationTargetService::Resolve_SceneCharacter();
		nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
	{
		XMStoreFloat3(&vPosition,
			pCharacter->Get_Transform()->Get_State(STATE::POSITION) +
			XMVectorSet(2.5f, 0.f, 0.f, 0.f));
	}
	const bool_t bSpawned = VALTAN_TARGET_ARCHETYPE_ID == strArchetypeId ?
		Spawn_ValtanTarget(vPosition) : Spawn_NpcTarget(strArchetypeId, vPosition);
	if (!bSpawned)
		return false;

	EFFECT_V2_TARGET_VIEW View;
	if (!Resolve_TargetView(View))
	{
		Despawn_Target();
		m_strAttachStatus = "Target spawn returned an unexpected object.";
		return false;
	}
	CEffectV2Runtime::Set_Ignored(m_Target, !m_bRuntimeOnTarget);
	m_strTargetArchetypeId = strArchetypeId;
	m_vTargetPosition = vPosition;
	m_fTargetYawDegrees = 0.f;
	m_fTargetLastClipSeconds = -1.f;
	if (m_strPivotBone.empty() || !View.pModel->Has_Bone(m_strPivotBone.c_str()))
	{
		m_strPivotBone = View.pModel->Has_Bone("b_effectroot") ? "b_effectroot" :
			(m_TargetBoneNames.empty() ? std::string() : m_TargetBoneNames.front());
	}
	Load_Bindings(strArchetypeId);
	m_strAttachStatus = "Target " + strArchetypeId + " spawned beside the scene character.";
	return true;
}

bool_t Client::CEffect_Tool_V2::Spawn_NpcTarget(
	const std::string& strArchetypeId,
	const float3_t& vPosition)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const uint32_t iLevel = GameInstance.Get_CurrentLevelID();
	const NPC_ACTOR_ENTRY* pActor = CActorCatalog::Find_Npc(strArchetypeId);
	const wstring_t strModelTag =
		CNpcPresentationAssetService::Get_ModelPrototypeTag(strArchetypeId);
	if (nullptr == pActor || strModelTag.empty())
	{
		m_strAttachStatus = "Unknown NPC archetype: " + strArchetypeId;
		return false;
	}
	if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
		m_pDevice, m_pContext, iLevel, strArchetypeId)))
	{
		m_strAttachStatus = "NPC presentation prototypes failed: " + strArchetypeId;
		return false;
	}

	CNpc::NPC_DESC Desc{};
	Desc.iPrototypeLevelIndex = iLevel;
	Desc.strModelTag = strModelTag;
	Desc.strShaderTag = pActor->shaderProfile == "esther" ?
		TEXT("Prototype_Component_Shader_VtxEstherNpc") :
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
	Desc.pIdleClip = pActor->idleClip.c_str();
	Desc.isLoop = true;
	Desc.vPosition = vPosition;
	Desc.fYawDegree = 0.f;
	Desc.fCollisionRadius = 0.f;
	if (pActor->shaderProfile == "esther")
		Desc.fOutlineWidth = CNpc::ESTHER_OUTLINE_WIDTH;
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		iLevel, TEXT("Prototype_GameObject_Npc"), iLevel, TARGET_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strAttachStatus = "Target spawn failed: " + strArchetypeId;
		return false;
	}
	const std::shared_ptr<CNpc> pNpc = std::dynamic_pointer_cast<CNpc>(pGameObject);
	if (nullptr == pNpc || nullptr == pNpc->Get_Model())
	{
		GameInstance.Remove_GameObject_from_Layer(iLevel, TARGET_LAYER_TAG, pGameObject);
		m_strAttachStatus = "Target spawn returned an unexpected object.";
		return false;
	}
	m_Target = EFFECT_V2_TARGET::From_Npc(pNpc);

	std::vector<std::string> BoneNames;
	Collect_BoneNames(pActor->modelAssetId, BoneNames);
	m_TargetBoneNames.clear();
	for (const std::string& strBone : BoneNames)
	{
		if (pNpc->Get_Model()->Has_Bone(strBone.c_str()))
			m_TargetBoneNames.push_back(strBone);
	}

	const auto Strike = pActor->actionClips.find("esther.strike");
	if (Strike != pActor->actionClips.end() && !Strike->second.empty())
		Play_TargetClip(Strike->second.front().c_str(), m_bTargetClipLoop);
	return true;
}

bool_t Client::CEffect_Tool_V2::Spawn_ValtanTarget(const float3_t& vPosition)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	const uint32_t iLevel = GameInstance.Get_CurrentLevelID();
	const BOSS_ACTOR_ENTRY* pBoss = CActorCatalog::Find_Boss(VALTAN_TARGET_ARCHETYPE_ID);
	if (nullptr == pBoss || pBoss->clientPresentationId != "boss.valtan.client.v1")
	{
		m_strAttachStatus = "Valtan presentation contract is not admitted.";
		return false;
	}
	if (!CValtanPresentationAssetService::Is_Ready(iLevel) &&
		FAILED(CValtanPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext, iLevel)))
	{
		m_strAttachStatus = "Valtan presentation prototypes failed.";
		return false;
	}

	CValtan::VALTAN_DESC Desc{};
	Desc.iPrototypeLevelIndex = iLevel;
	Desc.vPosition = vPosition;
	Desc.fScale = pBoss->presentationScale;
	Desc.fCollisionRadius = 0.f;
	Desc.isServerAuthoritative = false;
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		iLevel, TEXT("Prototype_GameObject_Valtan"), iLevel, TARGET_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strAttachStatus = "Valtan target spawn failed.";
		return false;
	}
	const std::shared_ptr<CValtan> pValtan = std::dynamic_pointer_cast<CValtan>(pGameObject);
	float4x4_t PresentationRoot{};
	if (nullptr == pValtan || nullptr == pValtan->Get_BodyModel() ||
		!pValtan->Try_Get_PresentationRootMatrix(&PresentationRoot))
	{
		GameInstance.Remove_GameObject_from_Layer(iLevel, TARGET_LAYER_TAG, pGameObject);
		m_strAttachStatus = "Valtan target did not expose its body presentation root.";
		return false;
	}
	m_Target = EFFECT_V2_TARGET::From_Valtan(pValtan);

	std::vector<std::string> BoneNames;
	Collect_BoneNames(pBoss->bodyModel, BoneNames);
	m_TargetBoneNames.clear();
	for (const std::string& strBone : BoneNames)
	{
		if (pValtan->Get_BodyModel()->Has_Bone(strBone.c_str()))
			m_TargetBoneNames.push_back(strBone);
	}

	if (!pBoss->presentationClips.idle.empty())
		Play_TargetClip(pBoss->presentationClips.idle.c_str(), m_bTargetClipLoop);
	return true;
}

bool_t Client::CEffect_Tool_V2::Resolve_TargetView(EFFECT_V2_TARGET_VIEW& OutView) const
{
	return m_Target.Is_Valid() && CEffectV2Object::Resolve_TargetView(m_Target, OutView);
}

void Client::CEffect_Tool_V2::Play_TargetClip(const char_t* pClipName, const bool_t bLoop)
{
	EFFECT_V2_TARGET_VIEW View;
	if (nullptr == pClipName || !Resolve_TargetView(View))
		return;
	View.pModel->Set_AnimationSpeed(1.f);
	if (!View.pModel->Set_Animation(pClipName, bLoop))
		return;
	View.pModel->Set_AnimTrackPosition(View.pModel->Get_CurrentAnimIndex(), 0.f);
	m_fTargetLastClipSeconds = -1.f;
	CEffectV2Runtime::Notify_Clip(m_Target, pClipName);
}

void Client::CEffect_Tool_V2::Despawn_Target()
{
	if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
		pPreview->Clear_FollowTarget();
	m_ePivotMode = PIVOT_MODE::WORLD;
	if (const std::shared_ptr<CGameObject> pOwner = m_Target.pOwner.lock())
	{
		CEffectV2Runtime::Set_Ignored(m_Target, false);
		CGameInstance::Get().Remove_GameObject_from_Layer(
			CGameInstance::Get().Get_CurrentLevelID(), TARGET_LAYER_TAG, pOwner);
	}
	m_Target.Reset();
	m_TargetBoneNames.clear();
}

void Client::CEffect_Tool_V2::Move_Target(const float3_t& vPosition, const f32_t fYawDegrees)
{
	const std::shared_ptr<CGameObject> pOwner = m_Target.pOwner.lock();
	if (nullptr == pOwner)
		return;
	if (EFFECT_V2_TARGET_KIND::NPC == m_Target.eKind)
	{
		if (!std::static_pointer_cast<CNpc>(pOwner)->Apply_NetworkState(vPosition, fYawDegrees))
			return;
	}
	else if (EFFECT_V2_TARGET_KIND::VALTAN == m_Target.eKind)
	{
		const std::shared_ptr<CTransform> pTransform =
			std::static_pointer_cast<CValtan>(pOwner)->Get_Transform();
		if (nullptr == pTransform)
			return;
		pTransform->Set_State(STATE::POSITION,
			XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 1.f));
		pTransform->Rotation(0.f, fYawDegrees, 0.f);
	}
	else
		return;
	m_vTargetPosition = vPosition;
	m_fTargetYawDegrees = fYawDegrees;
}

void Client::CEffect_Tool_V2::Update_Attach(const f32_t fTimeDelta)
{
	EFFECT_V2_TARGET_VIEW View;
	const bool_t bHasTarget = Resolve_TargetView(View);
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (nullptr != pPreview)
	{
		if (PIVOT_MODE::TARGET_BONE == m_ePivotMode && bHasTarget &&
			!m_strPivotBone.empty())
		{
			pPreview->Set_FollowTarget(m_Target, m_strPivotBone, m_ePivotRotation);
		}
		else if (pPreview->Has_FollowTarget())
			pPreview->Clear_FollowTarget();
	}
	if (m_bTestOrbit && nullptr != pPreview && PIVOT_MODE::WORLD == m_ePivotMode)
	{
		m_fTestOrbitAngle += fTimeDelta * m_fTestOrbitSpeed;
		pPreview->PivotWorld()._41 = m_vTestOrbitCenter.x + std::cos(m_fTestOrbitAngle) * m_fTestOrbitRadius;
		pPreview->PivotWorld()._42 = m_vTestOrbitCenter.y;
		pPreview->PivotWorld()._43 = m_vTestOrbitCenter.z + std::sin(m_fTestOrbitAngle) * m_fTestOrbitRadius;
	}
	const bool_t bSnapOnRestart = PIVOT_MODE::TARGET_BONE_FIXED == m_ePivotMode;
	if (!bHasTarget)
		return;
	const std::shared_ptr<Engine::CModel>& pModel = View.pModel;

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iClip);
	if (fTickPerSecond <= 0.f || !pModel->Get_AnimationProgress(iClip, fPosition, fDuration))
		return;
	const f32_t fSeconds = fPosition / fTickPerSecond;
	const f32_t fSpawnSeconds = static_cast<f32_t>(m_iSpawnFrame) / BINDING_FRAME_RATE;
	if (nullptr != pPreview && !pModel->Is_AnimPaused() &&
		m_fTargetLastClipSeconds >= 0.f && m_fTargetLastClipSeconds < fSpawnSeconds &&
		fSeconds >= fSpawnSeconds)
	{
		if (bSnapOnRestart)
			Snap_PivotToTarget();
		pPreview->Restart();
	}
	if (fSeconds < m_fTargetLastClipSeconds && nullptr != pPreview && 0 == m_iSpawnFrame)
	{
		if (bSnapOnRestart)
			Snap_PivotToTarget();
		pPreview->Restart();
	}
	m_fTargetLastClipSeconds = fSeconds;
}

void Client::CEffect_Tool_V2::Snap_PivotToTarget()
{
	EFFECT_V2_TARGET_VIEW View;
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (!Resolve_TargetView(View) || nullptr == pPreview)
		return;
	float4x4_t Pivot;
	if (!CEffectV2Object::Resolve_TargetPivot(View, m_strPivotBone, m_ePivotRotation, Pivot))
	{
		m_strAttachStatus = "Snap failed: bone not found " + m_strPivotBone;
		return;
	}
	pPreview->Clear_FollowTarget();
	pPreview->PivotWorld() = Pivot;
}
```

적용 위치 4: `Render_AttachWindow` 시작부터 `Target Playback` 섹션 끝(`if (ImGui::Checkbox("Loop", &m_bTargetClipLoop)) pModel->Set_Animation(iCurrent, m_bTargetClipLoop); }`, 1516-1662)까지 교체

```cpp
void Client::CEffect_Tool_V2::Render_AttachWindow()
{
	if (!m_bAttachWindowOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(460.f, 640.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Attach v2", &m_bAttachWindowOpen))
	{
		ImGui::End();
		return;
	}
	EFFECT_V2_TARGET_VIEW View;
	const bool_t bHasTarget = Resolve_TargetView(View);
	const std::shared_ptr<Engine::CModel> pModel = bHasTarget ? View.pModel : nullptr;

	ImGui::SeparatorText("Target (NPC archetype / Valtan)");
	const std::vector<NPC_ACTOR_ENTRY>& Npcs = CActorCatalog::Get_Npcs();
	if (ImGui::BeginCombo("Archetype",
		m_strSelectedArchetypeId.empty() ? "(select)" : m_strSelectedArchetypeId.c_str()))
	{
		if (ImGui::Selectable("BOSS_VALTAN  (Valtan)",
			VALTAN_TARGET_ARCHETYPE_ID == m_strSelectedArchetypeId))
		{
			m_strSelectedArchetypeId = VALTAN_TARGET_ARCHETYPE_ID;
		}
		ImGui::Separator();
		for (const NPC_ACTOR_ENTRY& Entry : Npcs)
		{
			if (Entry.runtimeStatus != "supported")
				continue;
			const std::string strLabel = Entry.archetypeId + "  (" +
				std::filesystem::path(Entry.modelAssetId).stem().string() + ")";
			if (ImGui::Selectable(strLabel.c_str(), Entry.archetypeId == m_strSelectedArchetypeId))
				m_strSelectedArchetypeId = Entry.archetypeId;
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(m_strSelectedArchetypeId.empty());
	if (ImGui::Button("Spawn Target"))
		Spawn_Target(m_strSelectedArchetypeId);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bHasTarget);
	if (ImGui::Button("Despawn"))
		Despawn_Target();
	ImGui::EndDisabled();
	if (bHasTarget)
	{
		ImGui::Text("Live: %s", m_strTargetArchetypeId.c_str());
		if (ImGui::Checkbox("Runtime spawns on target", &m_bRuntimeOnTarget))
		{
			CEffectV2Runtime::Set_Ignored(m_Target, !m_bRuntimeOnTarget);
			if (m_bRuntimeOnTarget && nullptr != pModel)
			{
				const char_t* pClip = pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex());
				if (nullptr != pClip)
					CEffectV2Runtime::Notify_Clip(m_Target, pClip);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Let CEffectV2Runtime apply the saved bindings to this tool target (in-game behaviour check). Hide the preview to avoid doubles.");
		float3_t vPosition = m_vTargetPosition;
		f32_t fYaw = m_fTargetYawDegrees;
		if (ImGui::DragFloat3("Target Position", &vPosition.x, 0.05f))
			Move_Target(vPosition, fYaw);
		if (ImGui::DragFloat("Target Yaw (deg)", &fYaw, 1.f, -360.f, 360.f))
			Move_Target(vPosition, fYaw);
		if (ImGui::Button("Beside Character"))
		{
			if (const std::shared_ptr<CCharacter> pCharacter =
				CAnimationTargetService::Resolve_SceneCharacter();
				nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
			{
				XMStoreFloat3(&vPosition,
					pCharacter->Get_Transform()->Get_State(STATE::POSITION) +
					XMVectorSet(2.5f, 0.f, 0.f, 0.f));
				Move_Target(vPosition, fYaw);
			}
		}
	}

	if (nullptr != pModel && 0u < pModel->Get_NumAnimations())
	{
		ImGui::SeparatorText("Target Playback");
		const uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
		const char_t* pCurrentName = pModel->Get_AnimationName(iCurrent);
		if (ImGui::BeginCombo("Target Clip", nullptr != pCurrentName ? pCurrentName : "(none)"))
		{
			for (uint32_t iClip = 0u; iClip < pModel->Get_NumAnimations(); ++iClip)
			{
				const char_t* pName = pModel->Get_AnimationName(iClip);
				if (nullptr == pName)
					continue;
				if (ImGui::Selectable(pName, iClip == iCurrent))
					Play_TargetClip(pName, m_bTargetClipLoop);
			}
			ImGui::EndCombo();
		}
		f32_t fPosition = 0.f;
		f32_t fDuration = 0.f;
		const bool_t bHasTrack =
			pModel->Get_AnimationProgress(iCurrent, fPosition, fDuration) && fDuration > 0.f;
		const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iCurrent);
		if (bHasTrack)
		{
			f32_t fScrub = fPosition;
			char_t szFormat[64]{};
			std::snprintf(szFormat, sizeof(szFormat), "frame %%.1f / %.0f", fDuration);
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::SliderFloat("##TargetScrub", &fScrub, 0.f, fDuration, szFormat))
			{
				pModel->Set_AnimPaused(true);
				pModel->Set_AnimTrackPosition(iCurrent, fScrub);
			}
			if (fTickPerSecond > 0.f)
				ImGui::TextDisabled("%.2f / %.2f s", fPosition / fTickPerSecond, fDuration / fTickPerSecond);
		}
		const bool_t bPaused = pModel->Is_AnimPaused();
		if (ImGui::Button(bPaused ? "Play" : "Pause"))
			pModel->Set_AnimPaused(!bPaused);
		ImGui::SameLine();
		ImGui::BeginDisabled(!bHasTrack);
		if (ImGui::Button("< Frame"))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrent, (std::max)(0.f, fPosition - 1.f));
		}
		ImGui::SameLine();
		if (ImGui::Button("Frame >"))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrent, (std::min)(fDuration, fPosition + 1.f));
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Restart Clip"))
		{
			pModel->Set_AnimTrackPosition(iCurrent, 0.f);
			m_fTargetLastClipSeconds = -1.f;
			if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
				pPreview->Restart();
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Loop", &m_bTargetClipLoop))
			pModel->Set_Animation(iCurrent, m_bTargetClipLoop);
	}
```

적용 위치 5: 같은 함수의 나머지(Effect Pivot·Bindings 섹션)에서 `pNpc` 참조 3곳만 교체

- `if (PIVOT_MODE::TARGET_BONE == m_ePivotMode && nullptr == pNpc)` → `… && !bHasTarget)`
- `ImGui::BeginDisabled('\0' == m_szEffectId[0] || nullptr == pClipForBinding || nullptr == pNpc);` → `… || !bHasTarget);`
- 바인딩 행 클릭 끝 `if (nullptr != pNpc) pNpc->Set_Animation(Binding.strClip.c_str(), m_bTargetClipLoop);` → `Play_TargetClip(Binding.strClip.c_str(), m_bTargetClipLoop);`

**G2 종료 증거**: 빌드 성공. F1 → Effect Tool v2 → Attach… → Archetype `BOSS_VALTAN` → Spawn Target: 캐릭터 옆에 발탄이
`mesh_idle_battle_1`로 서고, Target Clip 콤보에 `mesh_*` 클립 전부, Pivot Bone에 body 본 목록(`b_effectroot` 기본). Target Bone
follow로 이펙트가 발탄 본을 따라감(무기 소켓과 같은 위치·스케일). Add/Save Bindings → `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`
생성. `Runtime spawns on target` 체크 시 저장된 바인딩이 툴 발탄에 스폰. Valtan 맵(Server 권위)에서 같은 클립 stage에 스폰(사용자 육안).

## 적용 순서와 검증

1. G1(5-1 → 5-5 → 5-6/5-7/5-8 → 5-9) 빌드 → NPC 웨이 회귀 확인 → G2(5-10, 5-11) 빌드.
2. 빌드: Engine 무변경이라 Client만.

```powershell
$env:NoDefaultCurrentDirectoryInExePath=''
$env:PATH = "$env:SystemRoot\system32;$env:SystemRoot;" + $env:PATH
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" Client\Default\Client.vcxproj /m /p:Configuration=Debug /p:Platform=x64 /v:minimal
```

3. 실행(사용자): Character Select 또는 Valtan 맵 → F1 → Effect Tool v2 → Attach… → `BOSS_VALTAN` Spawn Target → 위 G2 증거 항목.
4. 실패 입력: `BOSS_VALTAN.effectv2bindings.json`의 `archetypeId`를 다른 값으로 바꾸면 툴 `Reload`가 "Bindings rejected"로 기존
   목록을 유지하고, 런타임은 `[EffectV2Runtime] bindings rejected for BOSS_VALTAN` 1회 후 발탄만 격리(NPC 바인딩 무관).
5. `git diff --check`, 정본 회귀 `Invoke-BuildAndRegression.ps1 -Configuration Debug`(이 PC 환경 갭: Artist 31470 d3dcompiler 22621 게이트,
   CharacterSelectIsolation 외부 타임아웃 — 2026-08-28 회귀 보고와 동일).

## 범위 밖 (후속)

- 발탄 패턴 stage 체인 전체를 툴에서 재생/스크럽하는 것(Animation Tool `Valtan Pattern Master` 소유). 바인딩은 클립 단위라 체인 stage가
  그 클립을 시작하면 인게임에서 발화한다.
- 플레이어 캐릭터 타깃(G12로 보류 중).
