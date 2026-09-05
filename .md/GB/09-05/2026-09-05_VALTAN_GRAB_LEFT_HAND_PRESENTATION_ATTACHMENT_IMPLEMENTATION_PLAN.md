# 2026-09-05 발탄 잡기 왼손 presentation attachment 구현 계획서

브랜치 `GB/KoukuSaydon-DataFormat`, 계획 수립 기준 HEAD `e61123d4`. working tree에는 KoukuSaydon 이름 분리 등
다른 세션의 미커밋 변경이 함께 있으며 이 계획이 손대는 `Character.h/.cpp`, `Valtan.h/.cpp`,
`ClientReplication.cpp`도 공유 파일이다. 모든 기준점은 HEAD가 아니라 계획 수립 당시 working tree 실측이며,
적용은 현재 파일 위에 하고 다른 세션의 변경을 되돌리지 않는다.

이 문서는 구현 계획서다. 기존 H/CPP 전문은 싣지 않고 삭제·교체 기준점과 새로 들어가는 완전한 블록만 싣는다.
선행 진단은 `.md/GB/09-04/2026-09-04_VALTAN_PATTERN_HIT_COLLIDER_AUTHORING_IMPLEMENTATION_PLAN.md` G12와
`.md/GB/09-04/2026-09-04_VALTAN_GRAB_SERVER_ANCHOR_AND_STAGGER_AURA_STAGE_END_IMPLEMENTATION_PLAN.md` G01이다.

---

## 0. 목표와 종료 증거

| G | 목표 | 종료 증거 |
|---|---|---|
| G01 | 버러지 잡기(`VALTAN_TRASH`, `VALTAN_TRASH_CATCH_IF`)와 뒤잡기 후 날리기(`VALTAN_CATCH_BREATH`)에서 CAPTURE snapshot이 도착한 프레임부터 잡힌 Character의 몸통이 발탄 `bip001-l-hand` 손 위치에 붙어 손 animation을 따라가고, release 뒤 0.2초 안에 Server 던지기 경로로 합류한다. 본체·장비·nameplate·Debug wire가 같은 Character transform을 읽는다 | `PlayerHandGripTransformContractTests` 4/4, `test_valtan_grip_local_offset_contract` 5/5, 인접 Valtan 계약 PASS, Debug Product/Core build PASS. 사용자의 `Server + Client` TRASH·CATCH_BREATH 화면 판정은 별도 종료 증거다 |
| G02 (후속, 이번 미구현) | Server capture anchor를 capture 지점 delta 대신 authored boss-local 손 offset으로 바꿔 release 순간 S1과 손 위치 차이를 줄인다 | 이 계획 범위 밖. 데이터 schema, publisher, bootstrap, `GameplayCatalog`, `ServerGameplayContractTests`를 한 변경 단위로 묶는 별도 계획이 필요하다 |

Server/Shared의 schema, packet field, runtime 동작은 바꾸지 않는다. `PacketMessages.h`의 계약 주석만 실제
authority에 맞게 교정한다.

---

## 1. 구현 전 실측: 왜 손에 붙은 적이 한 번도 없었는가

### 1.1 프레임 순서와 파츠 합성 시점

`Engine/Private/GameInstance.cpp` `Update_Engine` 156행의 실제 순서다.

```text
Priority_Update
→ Object_Manager::Update            (CCharacter::Update → Update_NetworkTransform(C1) → __super::Update → 파츠 Update)
→ Physics_Manager::Update
→ Object_Manager::Post_Physics_Update
→ Level_Manager::Update             (CLevel_ValtanArena::Update → m_Replication.Update() → Apply_WorldSnapshot)
→ Object_Manager::Late_Update       (render queue 등록)
→ render
```

`Engine/Private/PartObject.cpp` 47행 `Update_CombinedWorldMatrix`는 `ChildMatrix × *m_pParentMatrix`를
`m_CombinedWorldMatrix`에 저장하고, `Client/Private/Part_Body.cpp` 53행과 `Part_Equipment.cpp` 56행은 이를
**Object Update 단계의 `Update()`**에서 호출한다. `Part_Body::Render`와 `Part_Equipment::Render`는 이
`m_CombinedWorldMatrix`를 `g_WorldMatrix`로 바인딩한다.

즉 Character의 body/equipment mesh가 그려지는 world는 **`CCharacter::Update` 안에서 `__super::Update`가
호출되는 순간의 container transform**으로 확정된다. 그 뒤 Level Update에서 container transform을 아무리
덮어써도 그 프레임의 mesh 위치는 바뀌지 않고, 다음 프레임 `Update_NetworkTransform`이 다시 S1로 되돌린다.

### 1.2 삭제된 C3가 실행되던 위치

09-04 이전 `ClientReplication.cpp`의 `Update_PlayerAttachmentPresentations`(C3)는 `m_Replication.Update()`
즉 **Level Update**에서 container transform을 손 행렬로 덮어썼다. 위 순서 때문에 그 값은 파츠 합성에 한 번도
들어가지 않았다. 화면 본체는 항상 C1(S1 보간)이었고, C3는 bone chain(`CCharacter::Late_Update`)과 nameplate처럼
Level Update 뒤에 container transform을 읽는 소비자에게만 보였다. 09-03 계획의 100배 단위 교정도 이 순서
문제 위에서는 화면에 나타날 수 없었다. 09-04 G01은 C3를 지워 writer를 단일화했고, 그 결과 지금은 어떤 코드도
플레이어를 손으로 옮기지 않는다.

### 1.3 현재 writer 목록

| # | 소유 코드 | 계산 | 상태 |
|---|---|---|---|
| S1 | `Server/Private/GameRoom.cpp` `Capture_PlayerAttachment` 12076행, `Update_PlayerAttachment` 12274행 | capture 순간 `player − boss` delta를 boss-local로 저장하고 매 tick `boss pos + yaw 회전(delta)`를 `fPositionX/Y/Z`, `fYawDegrees`에 기록 | 유지 |
| S2 | `GameRoom.cpp` 8808행 | `eAction GRABBED`, owner, `BOSS_LEFT_HAND`, offset 4개를 `PLAYER_SNAPSHOT`으로 전송 | 유지 |
| C1 | `Client/Private/Character.cpp` `Update_NetworkTransform` 1021행 | 두 sample 보간을 `STATE::POSITION`과 yaw에 기록 | 유지 |
| 파츠 | `Part_Body.cpp` 53행, `Part_Equipment.cpp` 56행 | `CCharacter::Update`의 `__super::Update`에서 `child × parent` 확정 | 유지 |
| release | `GameRoom.cpp` `Release_PlayerAttachment` 12688행, `Prepare_ArenaEjection` 13304행 | S1 위치에서 boss 위치 기준 knockback 또는 boss 전방 24 m/s ejection 시작 | 유지 |

`Object_Manager`는 layer map을 wstring 순서로 순회하므로 `Layer_Player`가 `Layer_WorldEntity`(Valtan)보다 먼저
갱신된다. Character가 Update에서 읽는 손 bone은 직전 프레임 Valtan animation 결과이며 한 프레임 지연이다.
Engine `CPartObject`에 재합성 API를 추가하지 않는 한 이 지연은 허용한다.

### 1.4 손 socket world의 정본 합성

`Client/Private/EffectV2_Object.cpp` 733행 `Resolve_TargetPivot`은 Valtan bone slot을
`Get_BoneMatrix(bone) × Try_Get_PresentationRootMatrix()`로 만들고, `Part_Equipment.cpp` 66행 무기 socket도
같은 합성이다. `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` 24행부터 `boss.valtan.hand_*`
GROUP이 `slotId bip001-l-hand`로 이미 이 bone을 사용하므로 bone 존재와 합성은 검증된 경로다. G01은 같은 합성을
사용하고 translation만 취한다. bone basis에는 imported 0.01 scale이 남을 수 있어 basis 축을 변위에 쓰지 않는다.

### 1.5 잡기 pattern 구조

| pattern | CAPTURE stage | 이후 stage | release |
|---|---|---|---|
| `VALTAN_TRASH` / `VALTAN_TRASH_CATCH_IF` | `STEP_08`, `RETRY_RUSH_02`, `RETRY_RUSH_03` BOX 6×5 m, 0~600 ms 7회 | `CATCH_COUNTER` 200 → `CATCH_PRE_IMPACT` 1300 → `CATCH_SLAM` 1500(`DAMAGE_GRABBED_PLAYERS` ENTER) 또는 `EXECUTE_TAIL`(`EXECUTE_GRABBED_PLAYERS`) | slam ENTER에 boss 위치 기준 push |
| `VALTAN_CATCH_BREATH` | `STEP_02` CONE 120° 8 m, 250 ms | `STEP_03` 4000 → `STEP_04` 2000(`RELEASE_GRABBED_PLAYERS` ARENA_EJECTION 24 m/s, 500 ms, yaw +180) | S1에서 boss 전방으로 arena 밖까지 |

7개 CAPTURE hit의 `gripLocalOffset`은 모두 `{ forwardM 0.0, upM -0.9, rightM 0.0 }`이다. 발 원점을 손바닥
아래 0.9 m에 두어 몸통이 손에 오게 하는 값이며, 저작·Product 투영·publisher·Tool validation 계약은 그대로다.

---

## 2. 확정 계약

1. **writer 위치**: 손 attachment는 `CCharacter::Update` 안에서 `Update_NetworkTransform` 직후,
   `__super::Update`(파츠 합성) 직전에 container transform `POSITION`만 덮어쓴다. yaw는 C1이 쓴 Server yaw를
   유지한다. 이 위치가 파츠·collider·bone chain·nameplate가 모두 읽는 유일한 시점이다.
2. **socket 정본**: `CValtan`이 `IPlayerHandGripSocketSource`를 구현해 `BOSS_LEFT_HAND` slot을
   `Get_BoneMatrix("bip001-l-hand") × presentationRoot`로 답한다. dormant pool slot, body model 부재, bone 부재는
   false를 돌려 Character가 그 프레임 S1 fallback을 유지한다.
3. **grip 정본**: `ValtanEncounter.json`(admitted Product)의 CAPTURE `gripLocalOffset`을 joined presentation
   reload에서 한 번 admission한다. encounter 안의 모든 CAPTURE 값이 같아야 하며 다르면 fail-closed다. 같은
   tick에 CAPTURE stage에서 catch tail(`CATCH_COUNTER`, `STEP_03`)로 branch해도 같은 변위를 쓴다.
4. **변위 frame**: `forwardM`은 boss actor transform look, `rightM`은 right, `upM`은 world up이다. 손 bone
   basis는 사용하지 않는다.
5. **release**: snapshot action이 GRABBED를 벗어난 첫 프레임부터 0.2초 동안 마지막 손 위치에서 C1 보간 위치로
   smoothstep 보간한다. Server 판정·knockback·ejection은 그대로 S1에서 시작하며 Client는 이를 바꾸지 않는다.
6. **replication 경계**: `Apply_WorldSnapshot` player loop가 GRABBED면 owner `pValtan`을 Character에
   `Apply_NetworkAttachment`로 넘기고, 아니면 `Clear_NetworkAttachment`한다. Character는 Valtan 타입과
   snapshot을 모르고 interface만 소유한다. owner presentation이 isolated거나 없으면 조용히 S1 fallback이고,
   owner는 살아 있는데 admitted grip이 없을 때만 presentation failure 문자열을 남긴다.
7. **Server authority 불변**: Server가 보는 잡힌 플레이어 위치는 계속 S1이다. 다른 플레이어 skill이 잡힌
   플레이어를 맞추는 판정, release 시작점, ejection 방향은 S1 기준이며 이 계획은 그 사실을 문서에 남긴다.

---

## 3. 변경 파일

| G | 파일 | 작업 |
|---|---|---|
| G01 | `Client/Public/PlayerHandGripTransform.h` | socket view struct, source interface, `Compose_WorldPosition` 추가. 기존 struct/validation 유지 |
| G01 | `Client/Public/Character.h` | include, public 2함수, private member 6개, private 함수 1개 추가 |
| G01 | `Client/Private/Character.cpp` | 상수 1개, `Update` 호출 1줄, `Apply_NetworkState` reset 2줄, GRABBED 주석 교정과 도달 불가 중복 분기 2곳 삭제, 함수 정의 3개 추가 |
| G01 | `Client/Public/Valtan.h` | include 2개, 기반 class 추가, public override 2개, private member 1개, private 함수 1개 |
| G01 | `Client/Private/Valtan.cpp` | bone 상수, reload 단계 1개, joined reload stage/restore/commit, ghost donor copy, override 정의 2개 |
| G01 | `Client/Private/ClientReplication.cpp` | `Apply_WorldSnapshot` player loop에 attachment 연결 블록 추가 |
| G01 | `Client/Private/ValtanActionWorkbench.cpp` | Left-hand Grip Detail 안내 문장 교체 |
| G01 | `Shared/Public/Network/PacketMessages.h` | GRABBED, `PLAYER_ATTACHMENT_SLOT`, attachment offset 주석 교정(schema 불변) |
| G01 | `Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp` | 새 파일: `Compose_WorldPosition` 계약 4건 |
| G01 | `Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj`, `.filters` | ClCompile 등록 |
| G01 | `Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp` | 선언·호출·return gate 추가 |
| G01 | `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py` | Client source oracle을 새 계약으로 교체 |
| G01 | `.md/TEAM/발탄인수인계서.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 잡기 public 계약 문장 교체 |

`Client.vcxproj`/`.filters`는 `PlayerHandGripTransform.h`가 이미 `04. Network` filter에 등록돼 있어 변경하지
않는다. 새 C++ 파일은 harness 하나뿐이다.

---

## G01-1 `Client/Public/PlayerHandGripTransform.h`

파일 역할: 손 grip의 typed 값, 손 socket을 답하는 interface, 발 원점 world 위치 합성 수학. Character와 Valtan이
서로를 모른 채 공유하는 유일한 계약이며 harness가 같은 헤더로 수학을 검증한다.

- `#include "Network/PacketMessages.h"`: `PLAYER_ATTACHMENT_SLOT` typed slot을 interface 인자로 쓴다.
- `PLAYER_HAND_GRIP_SOCKET_VIEW`: `SocketWorld`는 bone combined × presentation root(translation만 신뢰),
  `OwnerYawBasis`는 boss actor transform(right/look 행만 사용). 둘 다 한 프레임 값이며 저장하지 않는다.
- `IPlayerHandGripSocketSource`: `Try_Get_PlayerHandGripSocketView(slot, out)`와
  `Try_Get_PlayerHandGripLocalOffset(slot, out)`. 구현자는 `CValtan` 하나다. 호출자는 `CCharacter`뿐이다.
- `CPlayerHandGripTransform::Compose_WorldPosition(view, grip, out)`: socket translation + right·rightM +
  worldUp·upM + look·forwardM. right/look은 y를 0으로 눌러 정규화하며 길이 0이면 실패. 비유한 입력, 범위 밖 grip은
  실패. socket basis scale은 결과에 들어가지 않는다.

```text
파일: Client/Public/PlayerHandGripTransform.h
작업: 전문 교체 (기존 struct와 validation은 그대로 유지)
```

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <cmath>
#include <cstddef>

NS_BEGIN(Client)

/* Authored CAPTURE hit value in metres, resolved from the admitted Product
   encounter document. The Client composes it every frame onto the owner's
   replicated attachment socket (Valtan left hand) as a presentation-only
   position; the Server keeps its boss-local anchor for judgement, release and
   ejection. The struct is also the typed shape that Balance Tool, Composition
   Detail, the encounter reference parser and the gameplay publisher validate. */
struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
{
	f32_t fForwardM = 0.f;
	f32_t fUpM = 0.f;
	f32_t fRightM = 0.f;

	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
};

/* One frame of the owner's presentation socket. SocketWorld is the bone
   combined matrix already multiplied by the owner's presentation root, so its
   translation is the palm in world metres; its basis may still carry the
   imported 0.01 scale and is never used for displacement. OwnerYawBasis is the
   owner's yaw-only actor transform whose right/look rows orient rightM/forwardM. */
struct PLAYER_HAND_GRIP_SOCKET_VIEW final
{
	float4x4_t SocketWorld{};
	float4x4_t OwnerYawBasis{};
};

/* Implemented by the replicated owner presentation (CValtan). CCharacter holds
   it weakly and asks every Update while the Server reports GRABBED; a false
   answer keeps the Server fallback transform for that frame. */
class IPlayerHandGripSocketSource
{
public:
	virtual ~IPlayerHandGripSocketSource() = default;
	virtual bool_t Try_Get_PlayerHandGripSocketView(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const = 0;
	virtual bool_t Try_Get_PlayerHandGripLocalOffset(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const = 0;
};

class CPlayerHandGripTransform final
{
public:
	static constexpr f32_t MAX_GRIP_OFFSET_COMPONENT_M = 10.f;

	static bool_t Is_ValidGripLocalOffset(
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset)
	{
		return Is_ValidGripComponent(gripLocalOffset.fForwardM) &&
			Is_ValidGripComponent(gripLocalOffset.fUpM) &&
			Is_ValidGripComponent(gripLocalOffset.fRightM);
	}

	/* Presentation position of the grabbed character's feet origin: the socket
	   translation displaced by the authored grip in the owner's yaw frame
	   (forward = owner look, right = owner right) and world up. The socket basis
	   scale never leaks into the result, so a 0.01-scaled bone and a normalized
	   bone place the character identically. */
	static bool_t Compose_WorldPosition(
		const PLAYER_HAND_GRIP_SOCKET_VIEW& view,
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset,
		float3_t& outPosition)
	{
		if (!Is_FiniteMatrix(view.SocketWorld) ||
			!Is_FiniteMatrix(view.OwnerYawBasis) ||
			!Is_ValidGripLocalOffset(gripLocalOffset))
		{
			return false;
		}
		const matrix_t socket = DirectX::XMLoadFloat4x4(&view.SocketWorld);
		const matrix_t yawBasis = DirectX::XMLoadFloat4x4(&view.OwnerYawBasis);
		vector_t right = DirectX::XMVectorSetY(yawBasis.r[0], 0.f);
		vector_t forward = DirectX::XMVectorSetY(yawBasis.r[2], 0.f);
		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(right)) <= 1.e-8f ||
			DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(forward)) <= 1.e-8f)
		{
			return false;
		}
		right = DirectX::XMVector3Normalize(right);
		forward = DirectX::XMVector3Normalize(forward);
		const vector_t position =
			DirectX::XMVectorSetW(socket.r[3], 1.f) +
			right * gripLocalOffset.fRightM +
			DirectX::XMVectorSet(0.f, gripLocalOffset.fUpM, 0.f, 0.f) +
			forward * gripLocalOffset.fForwardM;
		float3_t staged{};
		DirectX::XMStoreFloat3(&staged, position);
		if (!std::isfinite(staged.x) || !std::isfinite(staged.y) ||
			!std::isfinite(staged.z))
		{
			return false;
		}
		outPosition = staged;
		return true;
	}

private:
	static bool_t Is_ValidGripComponent(const f32_t value)
	{
		return std::isfinite(value) &&
			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
	}

	static bool_t Is_FiniteMatrix(const float4x4_t& value)
	{
		for (std::size_t row = 0u; row < 4u; ++row)
		{
			for (std::size_t column = 0u; column < 4u; ++column)
			{
				if (!std::isfinite(value.m[row][column]))
					return false;
			}
		}
		return true;
	}
};

NS_END
```

## G01-2 `Client/Public/Character.h`

이 G에서 얻는 책임: "Server가 GRABBED를 보고하는 동안 owner presentation socket으로 자기 발 원점을 옮기고,
release 뒤 짧게 합류하는 presentation". Character는 Valtan 타입, snapshot, socket 이름을 모른다.

```text
파일: Client/Public/Character.h
작업: 추가
기준점: #include "NavPathFollower.h" 바로 아래, #include "Network/PacketMessages.h" 바로 위
추가할 대상: include
필요한 이유: IPlayerHandGripSocketSource와 PLAYER_HAND_GRIP_LOCAL_OFFSET을 멤버로 소유한다
```

```cpp
#include "PlayerHandGripTransform.h"
```

```text
파일: Client/Public/Character.h
작업: 추가
기준점: void Apply_NetworkStance(LostArk::Shared::PLAYER_STANCE_ID stance); 바로 아래,
      Try_Get_NetworkStance 주석 블록 바로 위 (public)
추가할 대상: 함수 선언 2개
정의 위치: Client/Private/Character.cpp, Update_NetworkTransform 정의 바로 뒤
연결되는 부분: CClientReplication::Apply_WorldSnapshot player loop가 호출
```

```cpp
	/* Replication hands over the replicated owner presentation while the Server
	   reports GRABBED. The character keeps only a weak reference and the admitted
	   grip; every Update re-resolves the socket so a vanished owner falls back to
	   the Server transform instead of a stale matrix. */
	bool_t Apply_NetworkAttachment(
		const std::shared_ptr<const IPlayerHandGripSocketSource>& pSource,
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot);
	void Clear_NetworkAttachment();
```

```text
파일: Client/Public/Character.h
작업: 추가
기준점: f32_t m_fPresentationYawDegrees = { 0.f }; 바로 아래, CBoneChainSimulation m_BoneChains; 바로 위 (private)
추가할 대상: 멤버 변수 6개
```

```cpp
	/* Presentation attachment while the Server reports GRABBED. The Server
	   position stays in m_NetworkSamples; the socket only replaces the rendered
	   feet position before the parts compose their world matrices, so body,
	   equipment, collider wire and nameplate all read the same value. */
	std::weak_ptr<const IPlayerHandGripSocketSource> m_pAttachmentSocketSource;
	LostArk::Shared::PLAYER_ATTACHMENT_SLOT m_eAttachmentSlot =
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
	PLAYER_HAND_GRIP_LOCAL_OFFSET m_AttachmentGripLocalOffset{};
	bool_t m_bAttachmentPresented = { false };
	float3_t m_LastAttachedPosition{};
	/* Negative when no release blend is running. */
	f32_t m_fAttachmentReleaseBlendSeconds = { -1.f };
```

- `m_pAttachmentSocketSource`: replication이 준 owner presentation의 weak reference. 소유하지 않으며 owner가
  despawn되면 자동으로 만료돼 S1 fallback이 된다.
- `m_eAttachmentSlot`: snapshot의 typed slot. `NONE`이면 attachment가 없다는 뜻이고 Update가 GRABBED여도 socket을
  묻지 않는다.
- `m_AttachmentGripLocalOffset`: `Apply_NetworkAttachment`에서 owner가 답한 admitted grip 복사본. 매 프레임
  owner에 다시 묻지 않는다.
- `m_bAttachmentPresented`: 직전 Update에서 socket 위치를 실제로 썼는지. release 첫 프레임 감지에 쓰인다.
- `m_LastAttachedPosition`: 마지막으로 쓴 socket 위치. release blend의 시작점이다.
- `m_fAttachmentReleaseBlendSeconds`: release 뒤 경과 시간. 음수면 blend 없음. teleport reset에서 음수로 되돌린다.

```text
파일: Client/Public/Character.h
작업: 추가
기준점: void Update_NetworkTransform(f32_t fTimeDelta); 바로 아래, #ifdef _DEBUG 바로 위 (private)
추가할 대상: 함수 선언
```

```cpp
	/* Runs after Update_NetworkTransform and before the parts compose: while
	   GRABBED it replaces the interpolated position with the owner socket, and
	   for ATTACHMENT_RELEASE_BLEND_SECONDS after release it eases from the last
	   socket position onto the Server knockback path. */
	void Update_NetworkAttachmentTransform(f32_t fTimeDelta);
```

## G01-3 `Client/Private/Character.cpp`

```text
파일: Client/Private/Character.cpp
작업: 추가
기준점: constexpr f32_t TELEPORT_DISTANCE_SQ = 100.f; 바로 아래
```

```cpp
	constexpr f32_t ATTACHMENT_RELEASE_BLEND_SECONDS = 0.2f;
```

```text
파일: Client/Private/Character.cpp
작업: 교체
기준점: CCharacter::Update 안 if (m_hasNetworkState) { Update_NetworkTransform(fTimeDelta); }
```

```cpp
	if (m_hasNetworkState)
	{
		Update_NetworkTransform(fTimeDelta);
		Update_NetworkAttachmentTransform(fTimeDelta);
	}
```

```text
파일: Client/Private/Character.cpp
작업: 추가
기준점: Apply_NetworkState 안 if (reset) 블록의 m_fPlaybackServerTick = ... - INTERPOLATION_DELAY_TICKS; 바로 아래
이유: 10 m 초과 teleport(respawn 등) 위로 release blend가 걸치지 않게 한다
```

```cpp
		m_bAttachmentPresented = false;
		m_fAttachmentReleaseBlendSeconds = -1.f;
```

```text
파일: Client/Private/Character.cpp
작업: 교체
기준점: Apply_NetworkAction의 첫 번째 else if (PLAYER_ACTION_STATE::GRABBED == action) 분기 안 주석
      "The Server-authoritative boss-local attachment snapshot owns translation ... no hand bone is composed." 3줄
```

```cpp
		/* Translation belongs to Update_NetworkAttachmentTransform (owner socket)
		or, without an owner presentation, to the Server attachment snapshot. A
		neutral loop keeps class root motion from fighting either writer. */
```

```text
파일: Client/Private/Character.cpp
작업: 삭제
기준점 1: 같은 if/else 체인에서 FALLING 분기 뒤에 오는 두 번째 else if (PLAYER_ACTION_STATE::GRABBED == action) 블록 전체
        (HIT loop를 고르는 도달 불가 중복 분기)
기준점 2: 같은 함수 뒤쪽 else if (PLAYER_ACTION_STATE::GRABBED == m_eNetworkAction) 단독 블록
        (앞의 SKILL || ESTHER_CAST || GRABBED 분기가 먼저 잡아 도달 불가)
이유: 첫 분기가 항상 이기므로 두 블록은 어떤 입력에서도 실행되지 않는다. 남겨 두면 잡기 pose가 HIT인지 IDLE인지 오해를 만든다
```

```text
파일: Client/Private/Character.cpp
작업: 추가
기준점: void CCharacter::Update_NetworkTransform(f32_t fTimeDelta) 정의의 닫는 } 바로 아래,
      shared_ptr<CModel> CCharacter::Get_BodyModel() const 바로 위
추가할 대상: 함수 정의 3개
```

```cpp
bool_t CCharacter::Apply_NetworkAttachment(
	const std::shared_ptr<const IPlayerHandGripSocketSource>& pSource,
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot)
{
	using namespace LostArk::Shared;
	PLAYER_HAND_GRIP_LOCAL_OFFSET gripLocalOffset{};
	if (!m_hasNetworkState || nullptr == pSource ||
		PLAYER_ATTACHMENT_SLOT::NONE == slot ||
		slot >= PLAYER_ATTACHMENT_SLOT::END ||
		!pSource->Try_Get_PlayerHandGripLocalOffset(slot, gripLocalOffset) ||
		!CPlayerHandGripTransform::Is_ValidGripLocalOffset(gripLocalOffset))
	{
		Clear_NetworkAttachment();
		return false;
	}
	m_pAttachmentSocketSource = pSource;
	m_eAttachmentSlot = slot;
	m_AttachmentGripLocalOffset = gripLocalOffset;
	return true;
}

void CCharacter::Clear_NetworkAttachment()
{
	m_pAttachmentSocketSource.reset();
	m_eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
	m_AttachmentGripLocalOffset = {};
}

void CCharacter::Update_NetworkAttachmentTransform(const f32_t fTimeDelta)
{
	using namespace LostArk::Shared;
	if (nullptr == m_pTransformCom)
		return;
	if (PLAYER_ACTION_STATE::GRABBED == m_eNetworkAction &&
		PLAYER_ATTACHMENT_SLOT::NONE != m_eAttachmentSlot)
	{
		const std::shared_ptr<const IPlayerHandGripSocketSource> pSource =
			m_pAttachmentSocketSource.lock();
		PLAYER_HAND_GRIP_SOCKET_VIEW view{};
		float3_t position{};
		if (nullptr == pSource ||
			!pSource->Try_Get_PlayerHandGripSocketView(m_eAttachmentSlot, view) ||
			!CPlayerHandGripTransform::Compose_WorldPosition(
				view, m_AttachmentGripLocalOffset, position))
		{
			/* The owner is dormant, hidden or missing its bone this frame: the
			   Server fallback transform Update_NetworkTransform just wrote stays. */
			return;
		}
		m_pTransformCom->Set_State(
			STATE::POSITION,
			XMVectorSet(position.x, position.y, position.z, 1.f));
		m_LastAttachedPosition = position;
		m_bAttachmentPresented = true;
		m_fAttachmentReleaseBlendSeconds = -1.f;
		return;
	}
	if (m_bAttachmentPresented)
	{
		m_bAttachmentPresented = false;
		m_fAttachmentReleaseBlendSeconds = 0.f;
	}
	if (m_fAttachmentReleaseBlendSeconds < 0.f)
		return;
	if (std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
		m_fAttachmentReleaseBlendSeconds += fTimeDelta;
	const f32_t ratio = (std::min)(1.f,
		m_fAttachmentReleaseBlendSeconds / ATTACHMENT_RELEASE_BLEND_SECONDS);
	const f32_t eased = ratio * ratio * (3.f - 2.f * ratio);
	const vector_t networkPosition =
		m_pTransformCom->Get_State(STATE::POSITION);
	const vector_t lastAttached = XMVectorSet(
		m_LastAttachedPosition.x,
		m_LastAttachedPosition.y,
		m_LastAttachedPosition.z,
		1.f);
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(XMVectorLerp(lastAttached, networkPosition, eased), 1.f));
	if (ratio >= 1.f)
		m_fAttachmentReleaseBlendSeconds = -1.f;
}
```

함수 한 줄 책임과 흐름:

- `Apply_NetworkAttachment`: replication → slot/source/network state 검증 → owner에 admitted grip 질의 → 실패면
  `Clear_NetworkAttachment` 후 false(호출자가 S1 fallback 진단) → 성공이면 weak source, slot, grip 저장.
- `Clear_NetworkAttachment`: source/slot/grip만 비운다. release blend 상태는 건드리지 않아 잡힘이 끝나는 프레임의
  합류가 유지된다.
- `Update_NetworkAttachmentTransform`: `CCharacter::Update` → GRABBED이고 slot이 있으면 socket view와 grip으로
  world 위치 합성 → `POSITION`만 교체하고 마지막 위치 기록 → owner가 답하지 못하면 C1 값을 유지 → GRABBED가 아니면
  직전 프레임에 붙어 있었을 때만 blend 시작 → 0.2초 smoothstep으로 C1 위치에 합류 → 끝나면 음수로 복귀.

## G01-4 `Client/Public/Valtan.h`

```text
파일: Client/Public/Valtan.h
작업: 추가
기준점: #include "NavPathFollower.h" 바로 아래, #include "Network/PacketMessages.h" 바로 위
```

```cpp
#include "PlayerHandGripTransform.h"
```

```text
파일: Client/Public/Valtan.h
작업: 추가
기준점: #include <limits> 바로 아래, #include <string_view> 바로 위
```

```cpp
#include <optional>
```

```text
파일: Client/Public/Valtan.h
작업: 교체
기준점: class CValtan final : public CContainerObject
```

```cpp
class CValtan final : public CContainerObject, public IPlayerHandGripSocketSource
```

```text
파일: Client/Public/Valtan.h
작업: 추가
기준점: bool_t Try_Get_PortalRushAnchorMatrices(float4x4_t* pStartOut, float4x4_t* pEndOut) const; 바로 아래 (public)
정의 위치: Client/Private/Valtan.cpp, Try_Get_PresentationRootMatrix 정의 바로 위
연결되는 부분: CCharacter::Update_NetworkAttachmentTransform / Apply_NetworkAttachment
```

```cpp
	/* IPlayerHandGripSocketSource: the replicated BOSS_LEFT_HAND slot is the
	   bip001-l-hand bone composed with the same presentation root the weapon
	   socket and Effect V2 anchors use. Dormant pool slots answer false. */
	bool_t Try_Get_PlayerHandGripSocketView(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const override;
	bool_t Try_Get_PlayerHandGripLocalOffset(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const override;
```

```text
파일: Client/Public/Valtan.h
작업: 추가
기준점: m_PatternBodyVisibilityByActionId; 바로 아래, bool_t m_bLocalPatternAuthoringPreview = false; 바로 위 (private)
```

```cpp
	/* One encounter-wide CAPTURE grip admitted with the joined presentation
	   generation. Every authored CAPTURE hit must agree, so a same-tick branch
	   from the CAPTURE stage into a catch tail keeps the same displacement. */
	std::optional<PLAYER_HAND_GRIP_LOCAL_OFFSET> m_PlayerHandGripLocalOffset;
```

```text
파일: Client/Public/Valtan.h
작업: 추가
기준점: bool_t Reload_PatternBindings_WhileAdmitted(std::string& strOutStatus); 바로 아래 (private)
```

```cpp
	bool_t Reload_PlayerHandGripLocalOffset_WhileAdmitted(
		std::string& strOutStatus);
```

## G01-5 `Client/Private/Valtan.cpp`

```text
파일: Client/Private/Valtan.cpp
작업: 추가
기준점: 익명 namespace의 uint64_t g_iRaidBgmOwnershipGeneration = 0u; 바로 아래
```

```cpp
	constexpr const char_t* VALTAN_LEFT_HAND_BONE = "bip001-l-hand";
```

```text
파일: Client/Private/Valtan.cpp
작업: 추가
기준점: Reload_PatternBindings_WhileAdmitted 정의의 마지막 return true; } 바로 아래,
      bool_t CValtan::Reload_PatternPresentationAuthoring( 첫 overload 바로 위
```

```cpp
bool_t CValtan::Reload_PlayerHandGripLocalOffset_WhileAdmitted(
	std::string& strOutStatus)
{
	if (nullptr == m_pBodyModelCom)
	{
		strOutStatus =
			"Valtan player hand-grip reload requires an admitted body model.";
		return false;
	}
	CEncounterPatternReference encounter;
	if (!encounter.Load(CProjectDataRoot::Resolve(
			std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanEncounter.json"), strOutStatus))
	{
		strOutStatus =
			"Valtan player hand-grip Product admission rejected: " +
			strOutStatus;
		return false;
	}
	std::optional<PLAYER_HAND_GRIP_LOCAL_OFFSET> staged;
	for (const ENCOUNTER_PATTERN_REFERENCE& pattern : encounter.Get_Patterns())
	{
		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern.stages)
		{
			if (!stage.gripLocalOffset.has_value())
				continue;
			if (!CPlayerHandGripTransform::Is_ValidGripLocalOffset(
					*stage.gripLocalOffset))
			{
				strOutStatus =
					"Valtan player hand-grip CAPTURE offset is out of range: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
			if (!staged.has_value())
				staged = *stage.gripLocalOffset;
			else if (*staged != *stage.gripLocalOffset)
			{
				strOutStatus =
					"Valtan player hand-grip CAPTURE offsets disagree across the encounter: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
		}
	}
	if (staged.has_value() && !m_pBodyModelCom->Has_Bone(VALTAN_LEFT_HAND_BONE))
	{
		strOutStatus =
			"Valtan player hand-grip socket bone is missing from the admitted body model: bip001-l-hand";
		return false;
	}
	m_PlayerHandGripLocalOffset = staged;
	strOutStatus = staged.has_value() ?
		"Reloaded one Valtan player hand-grip CAPTURE offset." :
		"Reloaded zero Valtan player hand-grip CAPTURE offsets.";
	return true;
}
```

joined reload(`Reload_PatternPresentationAuthoring_Impl`)에 grip 단계를 끼운다.

| 작업 | 기준점 | 대상 |
|---|---|---|
| 추가 | `const auto PreviousBodyVisibility = m_PatternBodyVisibilityByActionId;` 바로 아래 | `const auto PreviousPlayerHandGrip = m_PlayerHandGripLocalOffset;` |
| 추가 | `RestorePrevious` lambda capture `&PreviousBindings, &PreviousBodyVisibility,` 바로 뒤 | `&PreviousPlayerHandGrip,` |
| 추가 | lambda 본문 `m_PatternBodyVisibilityByActionId = PreviousBodyVisibility;` 바로 아래 | `m_PlayerHandGripLocalOffset = PreviousPlayerHandGrip;` |
| 추가 | `if (!Reload_PatternBindings_WhileAdmitted(StepStatus) \|\|` 바로 아래 줄 | `!Reload_PlayerHandGripLocalOffset_WhileAdmitted(StepStatus) \|\|` |
| 교체 | 같은 if의 실패 문자열 `animation/effect/sound/combat-sound/shake cache` | `animation/grip/effect/sound/combat-sound/shake cache` |
| 추가 | `auto StagedBodyVisibility = std::move(m_PatternBodyVisibilityByActionId);` 바로 아래 | `auto StagedPlayerHandGrip = m_PlayerHandGripLocalOffset;` |
| 추가 | commit 구간 `m_PatternBodyVisibilityByActionId = std::move(StagedBodyVisibility);` 바로 아래 | `m_PlayerHandGripLocalOffset = StagedPlayerHandGrip;` |

`Copy_AdmittedPatternPresentationFrom`(ghost pool donor copy)은 effect cue bone anchor 검증 loop 바로 뒤에
아래 검증을 추가하고, commit 구간 `m_PatternBodyVisibilityByActionId = std::move(StagedBodyVisibility);` 바로
아래에 `m_PlayerHandGripLocalOffset = Source.m_PlayerHandGripLocalOffset;`를 추가한다.

```cpp
	if (Source.m_PlayerHandGripLocalOffset.has_value() &&
		!m_pBodyModelCom->Has_Bone(VALTAN_LEFT_HAND_BONE))
	{
		strOutStatus =
			"Ghost pool animation donor is missing the admitted player hand-grip socket bone: bip001-l-hand.";
		return false;
	}
```

```text
파일: Client/Private/Valtan.cpp
작업: 추가
기준점: bool_t CValtan::Try_Get_PresentationRootMatrix(float4x4_t* pOut) const 정의 바로 위
```

```cpp
bool_t CValtan::Try_Get_PlayerHandGripSocketView(
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
	PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const
{
	if (LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != slot ||
		m_isReplicationDormant || nullptr == m_pBodyModelCom ||
		nullptr == m_pTransformCom ||
		!m_pBodyModelCom->Has_Bone(VALTAN_LEFT_HAND_BONE))
	{
		return false;
	}
	float4x4_t presentationRoot{};
	if (!Try_Get_PresentationRootMatrix(&presentationRoot))
		return false;
	PLAYER_HAND_GRIP_SOCKET_VIEW staged{};
	XMStoreFloat4x4(
		&staged.SocketWorld,
		m_pBodyModelCom->Get_BoneMatrix(VALTAN_LEFT_HAND_BONE) *
		XMLoadFloat4x4(&presentationRoot));
	staged.OwnerYawBasis = *m_pTransformCom->Get_WorldMatrixPtr();
	outView = staged;
	return true;
}

bool_t CValtan::Try_Get_PlayerHandGripLocalOffset(
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
	PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const
{
	if (LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != slot ||
		!m_PlayerHandGripLocalOffset.has_value() ||
		!CPlayerHandGripTransform::Is_ValidGripLocalOffset(
			*m_PlayerHandGripLocalOffset))
	{
		return false;
	}
	outOffset = *m_PlayerHandGripLocalOffset;
	return true;
}
```

## G01-6 `Client/Private/ClientReplication.cpp`

```text
파일: Client/Private/ClientReplication.cpp
작업: 추가
기준점: Apply_WorldSnapshot player loop 안 character->Apply_NetworkStance(player.eStance); 바로 아래,
      if (isLocallyControlled) 바로 위
```

```cpp
		if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
		{
			/* The owner presentation is the same replicated Valtan the Effect V2
			hand anchors follow. A missing or isolated owner keeps the Server
			fallback transform; only a live owner without an admitted grip is a
			presentation admission failure worth reporting. */
			std::shared_ptr<CValtan> owner;
			const auto ownerIter =
				m_WorldEntities.find(player.iAttachmentOwnerNetEntityId);
			if (m_WorldEntities.end() != ownerIter &&
				WORLD_ENTITY_KIND::BOSS == ownerIter->second.eKind &&
				!ownerIter->second.bPresentationIsolated)
			{
				owner = ownerIter->second.pValtan.lock();
			}
			if (nullptr == owner)
				character->Clear_NetworkAttachment();
			else if (!character->Apply_NetworkAttachment(
					owner, player.eAttachmentSlot) &&
				m_strPendingPresentationFailure.empty())
			{
				m_strPendingPresentationFailure =
					"Grabbed player kept its Server fallback transform because the owner presentation has no admitted CAPTURE gripLocalOffset.";
			}
		}
		else
			character->Clear_NetworkAttachment();
```

호출 흐름:

```text
S2C_WORLD_SNAPSHOT
-> CClientReplication::Apply_WorldSnapshot player loop
-> CCharacter::Apply_NetworkState (S1 sample push) / Apply_NetworkAction (GRABBED edge, IDLE loop)
-> CCharacter::Apply_NetworkAttachment(owner CValtan, BOSS_LEFT_HAND)  또는 Clear_NetworkAttachment
다음 프레임 Object Update
-> CCharacter::Update
   -> Update_NetworkTransform            (C1: S1 보간 position/yaw)
   -> Update_NetworkAttachmentTransform  (GRABBED: socket 위치로 POSITION 교체 / release: 0.2초 합류)
   -> __super::Update                    (Part_Body / Part_Equipment가 이 transform으로 world 확정)
   -> m_pColliderCom->Update             (Debug wire도 같은 transform)
-> Late_Update (bone chain, render queue)
```

## G01-7 `Client/Private/ValtanActionWorkbench.cpp`, `Shared/Public/Network/PacketMessages.h`

```text
파일: Client/Private/ValtanActionWorkbench.cpp
작업: 교체
기준점: ImGui::SeparatorText("Left-hand Grip"); 아래 ImGui::TextDisabled 문자열
```

```cpp
				"Authored CAPTURE grip in metres. The Client composes it onto the owner's bip001-l-hand socket in the owner's yaw frame (forward/right) and world up for presentation only; the Server keeps its boss-local anchor for judgement, release and ejection.");
```

`PacketMessages.h`는 schema를 바꾸지 않고 다음 세 주석만 교정한다.

| 기준점 | 교정 문장 |
|---|---|
| `GRABBED` enum 값 위 주석 | Server가 boss-local snapshot으로 world position/yaw를 재계산해 판정·release에 쓰고, Client는 owner presentation socket 위에 몸통을 그린다 |
| `PLAYER_ATTACHMENT_SLOT` 위 주석 | Shared/Server는 Client asset을 모르며 Client가 typed slot을 자기 presentation socket에 매핑한다 |
| `fAttachmentLocalOffsetX` 위 주석 | Client presentation은 이 offset 대신 owner socket을 쓰고 Server position은 judgement/release 정본으로 남는다 |

## G01-8 harness

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp
작업: 새 파일 (UTF-8 BOM 없음, CRLF)
```

```cpp
#include "PlayerHandGripTransform.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
	void Require(const bool_t bCondition, const char* const pMessage)
	{
		if (!bCondition)
			throw std::runtime_error(pMessage);
	}

	bool_t Near(const f32_t a, const f32_t b)
	{
		return std::abs(a - b) <= 1.e-4f;
	}

	Client::PLAYER_HAND_GRIP_SOCKET_VIEW MakeView(
		const DirectX::XMMATRIX& Socket, const DirectX::XMMATRIX& YawBasis)
	{
		Client::PLAYER_HAND_GRIP_SOCKET_VIEW View{};
		DirectX::XMStoreFloat4x4(&View.SocketWorld, Socket);
		DirectX::XMStoreFloat4x4(&View.OwnerYawBasis, YawBasis);
		return View;
	}

	void VerifyScaledSocketBasisDoesNotLeak()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
			DirectX::XMMatrixRotationRollPitchYaw(0.7f, -1.1f, 0.3f) *
			DirectX::XMMatrixTranslation(3.f, 2.f, 1.f);
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Grip{ 0.f, -0.9f, 0.f };
		float3_t Position{};
		Require(Client::CPlayerHandGripTransform::Compose_WorldPosition(
			MakeView(Socket, DirectX::XMMatrixIdentity()), Grip, Position),
			"a 0.01-scaled rotated socket was rejected");
		Require(Near(Position.x, 3.f) && Near(Position.y, 1.1f) &&
			Near(Position.z, 1.f),
			"socket basis scale or rotation leaked into the feet position");
	}

	void VerifyYawBasisOrientsForwardAndRight()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixTranslation(3.f, 2.f, 1.f);
		const DirectX::XMMATRIX YawBasis =
			DirectX::XMMatrixScaling(5.f, 5.f, 5.f) *
			DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(90.f)) *
			DirectX::XMMatrixTranslation(-40.f, 9.f, 12.f);
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Grip{ 1.f, 0.f, 0.5f };
		float3_t Position{};
		Require(Client::CPlayerHandGripTransform::Compose_WorldPosition(
			MakeView(Socket, YawBasis), Grip, Position),
			"a scaled yaw basis was rejected");
		Require(Near(Position.x, 4.f) && Near(Position.y, 2.f) &&
			Near(Position.z, 0.5f),
			"forwardM/rightM did not follow the normalized owner look/right rows");
	}

	void VerifyOwnerTranslationNeverMovesTheGrip()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixTranslation(-2.f, 4.f, 6.f);
		const DirectX::XMMATRIX YawBasis =
			DirectX::XMMatrixTranslation(100.f, 100.f, 100.f);
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Grip{};
		float3_t Position{};
		Require(Client::CPlayerHandGripTransform::Compose_WorldPosition(
			MakeView(Socket, YawBasis), Grip, Position),
			"an identity-rotation owner basis was rejected");
		Require(Near(Position.x, -2.f) && Near(Position.y, 4.f) &&
			Near(Position.z, 6.f),
			"the owner translation displaced a zero grip");
	}

	void VerifyInvalidInputsAreRejected()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixTranslation(1.f, 1.f, 1.f);
		float3_t Position{ 7.f, 7.f, 7.f };
		Client::PLAYER_HAND_GRIP_SOCKET_VIEW View =
			MakeView(Socket, DirectX::XMMatrixIdentity());
		View.SocketWorld._42 = std::numeric_limits<f32_t>::quiet_NaN();
		Require(!Client::CPlayerHandGripTransform::Compose_WorldPosition(
			View, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, Position),
			"a non-finite socket translation was accepted");

		View = MakeView(Socket, DirectX::XMMatrixScaling(0.f, 0.f, 0.f));
		Require(!Client::CPlayerHandGripTransform::Compose_WorldPosition(
			View, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, Position),
			"a degenerate owner yaw basis was accepted");

		View = MakeView(Socket, DirectX::XMMatrixIdentity());
		Require(!Client::CPlayerHandGripTransform::Compose_WorldPosition(
			View, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{ 0.f, -10.01f, 0.f },
			Position),
			"an out-of-range grip was accepted");
		Require(Near(Position.x, 7.f) && Near(Position.y, 7.f) &&
			Near(Position.z, 7.f),
			"a rejected composition mutated the caller's position");
	}
}

int Run_PlayerHandGripTransformContractTests()
{
	try
	{
		VerifyScaledSocketBasisDoesNotLeak();
		VerifyYawBasisOrientsForwardAndRight();
		VerifyOwnerTranslationNeverMovesTheGrip();
		VerifyInvalidInputsAreRejected();
		std::cout << "PlayerHandGripTransformContractTests: 4/4 passed\n";
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "PlayerHandGripTransformContractTests: FAIL: " <<
			Error.what() << '\n';
		return 1;
	}
}
```

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
작업: 추가
기준점: <ClCompile Include="..\Private\CombatDebugVisibilityContractTests.cpp" /> 바로 아래
```

```xml
	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
```

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
작업: 추가
기준점: <ClCompile Include="..\Private\CombatDebugVisibilityContractTests.cpp" /> 바로 아래
```

```xml
	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
```

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp
작업: 추가 3곳
기준점 1: int Run_CombatDebugVisibilityContractTests(); 바로 아래
        -> int Run_PlayerHandGripTransformContractTests();
기준점 2: main의 const int CombatDebugVisibilityFailures = Run_CombatDebugVisibilityContractTests(); 바로 아래
        -> const int PlayerHandGripTransformFailures = Run_PlayerHandGripTransformContractTests();
기준점 3: 최종 return 조건 0 == CombatDebugVisibilityFailures ? 0 : 1;
        -> 0 == CombatDebugVisibilityFailures && 0 == PlayerHandGripTransformFailures ? 0 : 1;
```

## G01-9 `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py`

`test_client_never_composes_the_grip_on_a_hand_bone`를 아래로 교체한다. 나머지 네 데이터 테스트는 그대로다.

```python
    def test_client_composes_the_grip_before_the_parts_update(self) -> None:
        header = VALTAN_HEADER.read_text(encoding="utf-8-sig")
        valtan = VALTAN_SOURCE.read_text(encoding="utf-8-sig")
        replication_header = REPLICATION_HEADER.read_text(encoding="utf-8-sig")
        replication = REPLICATION_SOURCE.read_text(encoding="utf-8-sig")
        character = CHARACTER_SOURCE.read_text(encoding="utf-8-sig")
        game_room = GAME_ROOM_SOURCE.read_text(encoding="utf-8-sig")
        # The only Client writer runs inside CCharacter::Update, after the
        # Server interpolation and before the parts compose their world.
        update = character[character.index("void CCharacter::Update(f32_t fTimeDelta)"):]
        update = update[: update.index("void CCharacter::Late_Update(")]
        self.assertLess(
            update.index("Update_NetworkTransform(fTimeDelta);"),
            update.index("Update_NetworkAttachmentTransform(fTimeDelta);"),
        )
        self.assertLess(
            update.index("Update_NetworkAttachmentTransform(fTimeDelta);"),
            update.index("__super::Update(fTimeDelta);"),
        )
        self.assertIn("PLAYER_ACTION_STATE::GRABBED == action", character)
        # Level-update-phase overwrites never render: the parts already composed.
        for forbidden in (
            "Update_PlayerAttachmentPresentations",
            "Stage_PlayerAttachmentPresentation",
            "bip001-l-hand",
            "m_PlayerAttachments",
        ):
            self.assertNotIn(forbidden, replication_header)
            self.assertNotIn(forbidden, replication)
        self.assertIn("character->Apply_NetworkAttachment(", replication)
        self.assertIn("character->Clear_NetworkAttachment();", replication)
        # Valtan owns the socket bone and the encounter-wide admitted grip.
        self.assertIn("public IPlayerHandGripSocketSource", header)
        self.assertIn("m_PlayerHandGripLocalOffset", header)
        self.assertIn('VALTAN_LEFT_HAND_BONE = "bip001-l-hand"', valtan)
        self.assertIn("Reload_PlayerHandGripLocalOffset_WhileAdmitted(StepStatus)", valtan)
        self.assertIn("Get_BoneMatrix(VALTAN_LEFT_HAND_BONE)", valtan)
        # Server authority is unchanged.
        self.assertIn("Update_PlayerAttachment(player, updateTick)", game_room)
        self.assertIn("player.fAttachmentLocalOffsetX * cosine", game_room)
```

## G01-10 팀 문서

`.md/TEAM/발탄인수인계서.md`의 "Client grab은 hand bone 합성 없이 ..." 문단과
`.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 "별도로 player가 `GRABBED`인 동안 ..." 문장을 다음 계약으로
교체한다.

```text
Client grab은 Server attachment snapshot과 Client presentation socket을 함께 쓴다. Server는 capture 순간의
boss-local 상대 위치와 yaw offset을 저장해 매 tick `boss pos + yaw 회전(offset)`을 플레이어 위치로 복제하고,
이 위치가 판정·release·ejection의 정본이다. Client `CCharacter::Update`는 `Update_NetworkTransform`이 그
값을 보간한 직후, 파츠가 world를 합성하기 전에 owner `CValtan`의 `bip001-l-hand` socket(bone × presentation
root)과 admitted `gripLocalOffset`(boss yaw frame forward/right, world up)으로 발 원점 POSITION만 교체한다.
따라서 본체·장비·collider wire·nameplate가 같은 손 위치를 따르고, release 뒤 0.2초 동안 마지막 손 위치에서
Server 경로로 합류한다. `gripLocalOffset`은 `ValtanEncounter.json`의 모든 CAPTURE hit에서 같아야 하며 다르면
joined presentation admission이 실패한다. Server가 보는 잡힌 플레이어 위치는 계속 capture 지점 기준이므로
다른 플레이어 skill 판정과 던지기 시작점은 손 위치와 다를 수 있다.
```

## G01 검증

```powershell
$env:PYTHONPATH='.'; $env:PYTHONIOENCODING='utf-8'
python -m unittest Tools.ValtanPipeline.test_valtan_grip_local_offset_contract
python -m unittest Tools.ValtanPipeline.test_valtan_combat_object_hit_effect_presentation_contract Tools.ValtanPipeline.test_valtan_pattern_target_effect_anchor_contract Tools.ValtanPipeline.test_world_entity_spawn_revision_contract
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
git diff --check
```

기대: grip 5/5, 인접 3 suite PASS, Product exit 0, Core exit 0(`PlayerHandGripTransformContractTests: 4/4 passed`
포함). 사용자 화면 판정은 RESULT 5절 절차로 별도 수행한다.

---

## G02 후속 경계 (이번 미구현)

release 순간 Client 몸통은 손 위치에 있고 Server S1은 capture 지점(TRASH: boss 전방 0~6 m 지면, CATCH_BREATH:
뒤쪽 cone 안 지면)에 있다. 0.2초 합류로 시각적 튐은 줄지만 Server 던지기 시작점은 여전히 S1이다. 이를 손 근처로
옮기려면 `Data/Valtan/Valtan.gameplay.json` CAPTURE hit에 boss-local anchor offset을 저작하고
`Publish-GameplayBalance.ps1` → bootstrap → `GameplayCatalog` → `Capture_PlayerAttachment`가 delta 대신 그
값을 저장하도록 한 수직 슬라이스로 묶어야 한다. Client bone 위치를 Server로 보내는 우회는 만들지 않는다.
