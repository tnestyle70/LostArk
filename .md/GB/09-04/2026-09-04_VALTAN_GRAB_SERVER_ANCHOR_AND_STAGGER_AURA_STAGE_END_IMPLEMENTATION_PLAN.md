# 2026-09-04 발탄 잡기 Server anchor 단일 경로와 마력구 오라 STAGE_END 구현 계획서

브랜치 `GB/KoukuSaydon-Main-Pattern`, 계획 수립 기준 HEAD `8f8e5bea`. working tree에는 다른 발탄 작업의
미커밋 변경이 함께 있으며 이 계획이 손댄 `ClientReplication.cpp/.h`, `Valtan.cpp/.h`,
`BOSS_VALTAN.effectv2bindings.json`도 공유 파일이다. 모든 기준점과 행 번호는 HEAD가 아니라 계획 수립 당시
working tree 실측이며, 적용은 현재 파일 위에 하고 다른 세션의 변경을 되돌리지 않는다.

이 문서는 구현 계획서다. 기존 H/CPP 전문은 싣지 않고 삭제·교체 기준점과 새로 들어가는 완전한 블록만
싣는다. 선행 진단은 `2026-09-04_VALTAN_PATTERN_HIT_COLLIDER_AUTHORING_IMPLEMENTATION_PLAN.md` G12의
다섯 좌표 표다. G01/G02 구현은 현재 working tree에 반영됐고 targeted 자동 검증까지 통과했다. C++ build,
FullDiagnostic와 사용자 화면 판정은 대응 RESULT에서 PENDING으로 분리한다.

---

## 0. 목표와 종료 증거

| G | 목표 | 종료 증거 |
|---|---|---|
| G01 | 잡힌 플레이어의 최종 world transform을 Server attachment snapshot(S1→S2) 한 권위 경로로 통일한다. Client가 왼손 bone 행렬로 transform을 덮어쓰는 C3를 제거한다 | `ClientReplication.cpp`에 `bip001-l-hand`, `Update_PlayerAttachmentPresentations`가 없다. `test_valtan_grip_local_offset_contract`와 인접 combat-object 계약 PASS. Debug Product/FullDiagnostic와 사용자의 TRASH·CATCH_BREATH 화면 판정은 별도 종료 증거다 |
| G02 | `VALTAN_STAGGER_SLOT` CHANNEL 12000ms 동안 `boss.valtan.magicball`과 `boss.valtan.magicball.aura`가 같은 clock에 재생되고, 누적 damage 1000으로 `VALTAN_GROGGY_FOLLOWUP`에 분기하거나 TIMEOUT으로 FINAL_ATTACK에 들어가는 순간 두 group이 종료된다 | V2 validator, binding validate, 전용 magicball/aura contract와 5-suite PASS. Debug Product/FullDiagnostic 및 사용자의 10~12초 유지·stage 전환 소멸 판정은 별도 종료 증거다 |

Server/Shared의 schema와 runtime 동작은 두 G 모두 바꾸지 않는다. 다만 G01이 없애는 Client hand-bone
합성과 모순되지 않도록 `PacketMessages.h`, `ServerPlayer.h`의 계약 주석은 같은 변경에서 교정한다.

---

## 1. 구현 전 실측과 확정 계약

### 1.1 구현 전 잡기 좌표 다섯 개와 구현 후 단일 경로

| # | 구현 전 소유 코드 | 계산 |
|---|---|---|
| S1 Server anchor | `Server/Private/GameRoom.cpp` `Capture_PlayerAttachment`(11746행)와 `Update_PlayerAttachment`(11944행) | capture 순간 플레이어의 boss-local `(localX, localY, localZ, localYaw)`를 저장하고 매 tick `boss pos + yaw 회전(local)`을 `player.fPositionX/Y/Z`, `fYawDegrees`로 쓴다. 1970행이 boss 이동 뒤 같은 tick에 한 번 더 갱신해 한 tick 지연을 막는다 |
| S2 snapshot | `GameRoom.cpp` 8481~8493행 | `eAction GRABBED`, `iAttachmentOwnerNetEntityId`, `eAttachmentSlot BOSS_LEFT_HAND`, `fAttachmentLocalOffsetX/Y/Z`, `fAttachmentYawOffsetDegrees`와 S1 위치를 `PLAYER_SNAPSHOT`으로 보낸다 |
| C1 Server fallback | `Client/Private/Character.cpp` `Update_NetworkTransform`(1021행) | GRABBED 여부와 무관하게 매 프레임 두 sample 사이를 보간해 `STATE::POSITION`과 yaw를 쓴다. `Apply_NetworkState`의 GRABBED 분기(1388행)는 HIT loop를 재생한다 |
| C2 손 world | `Client/Private/ClientReplication.cpp` 2480~2500행 | `Get_BoneMatrix("bip001-l-hand") × presentationRoot` |
| C3 최종 world | `ClientReplication.cpp` `Update_PlayerAttachmentPresentations`(2450행) | 첫 프레임에 `CPlayerHandGripTransform::Build_LocalOffset`로 회전 basis만 저장하고 매 프레임 `Compose_World(local, hand, gripLocalOffset)`로 transform 4축을 덮어썼다. `gripLocalOffset.upM -0.9`는 `Valtan.gameplay.json` CAPTURE hit 7곳이 같은 값이다 |

구현 전 실행 순서는 `CGameInstance::Update_Engine`이 Object Update(C1) → Level Update(`CLevel_ValtanArena::Update` →
`m_Replication.Update()` → 당시 `ClientReplication.cpp`의 C3) → render였으므로 화면에는 C3가 남고 nameplate,
Debug wire, Server 판정은 S1을 썼다. 현재 구현은 C3와 그 전용 코드를 제거한 S2 → C1 단일 경로다.
C1이 마지막 writer이고 본체·nameplate·wire가 같은 Server-authoritative snapshot stream을 소비한다.
Client interpolation이 있으므로 Server 최신 tick과 같은 프레임의 bit-exact 좌표 일치까지 뜻하지 않는다.

`gripLocalOffset` 데이터는 이 변경 뒤에도 남는다. 현재 잔존 소비자는 저작·검증 계층뿐이며, 아래 표에서
`삭제`로 표시한 행은 구현 과정에서 제거된 과거 C3 runtime 소비자다.

| 위치 | 역할 | 이번 변경 |
|---|---|---|
| `Data/Valtan/Valtan.gameplay.json` CAPTURE hit 7곳, `Data/Encounters/Valtan/ValtanEncounter.json` 투영 | 저작 정본과 Product 투영 | 유지 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` 1954~2210행, `Tools/ValtanPipeline/valtan_tuning_pipeline.py` 4598~4681행 | playerResponse/attachmentSlot/gripLocalOffset 동반 검증 | 유지 |
| `Client/Public/EncounterPatternReference.h` 47행, `EncounterPatternReference.cpp` 1288~1300행 | Product 문서 parse와 range 검증 | 유지 |
| `Client/Private/BalanceTool.cpp`, `ActionCompositionWorkbench.cpp` Detail | Stage hit Detail 편집·저장 | 유지 |
| `Client/Public/PlayerHandGripTransform.h` `Build_LocalOffset`/`Compose_World` | C3 전용 행렬 합성 | 삭제 |
| `Client/Public/Valtan.h`/`Valtan.cpp` grip cache 2개와 lookup 2개, joined reload 단계 | C3 전용 lookup | 삭제 |
| `Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp` | C3 행렬 계약 | 삭제 |
| `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py` `test_client_lookup_is_pattern_keyed_and_fail_closed` | C3 source oracle | 새 계약으로 교체 |

Server와 Client의 transform 적용 runtime은 `gripLocalOffset`을 읽지 않는다. parser, publisher,
Balance Tool과 Composition Detail의 typed shape/range 검증은 유지한다. S1은 capture 순간의 상대 위치이므로
잡힌 뒤 플레이어는 손 animation을 따라가지 않고 boss root 기준 고정 offset에서 boss yaw와 함께 회전한다. 이 배치가 화면에서
부족하면 Server capture에 authored boss-local 손 offset을 두는 별도 G가 필요하며 이 계획의 범위 밖이다.

### 1.2 마력구 파괴 패턴 effect 경로

| 계층 | 실측 |
|---|---|
| gameplay | `VALTAN_STAGGER_SLOT`(displayName 마력구 파괴 패턴). `CHANNEL` 12000ms, `bossResponse ACCUMULATED_HEALTH_DAMAGE threshold 1000`, branch `HEALTH_DAMAGE_THRESHOLD_REACHED -> VALTAN_GROGGY_FOLLOWUP`(GROGGY 6833ms), `TIMEOUT -> FINAL_ATTACK`(3000ms, 2900ms omnidirectional wipe) |
| Server | `GameRoom.cpp` 8604행이 threshold/progress를 `BossCombat`으로 복제하고 분기 시 `WORLD_ENTITY_SNAPSHOT`의 pattern/action ID가 바뀐다. 변경 없음 |
| Client stage clock | `Client/Private/Valtan.cpp` 4674행이 snapshot 적용마다 `CEffectV2Runtime::Sync_Stage(target, m_strServerActionId, age, clocks)`를 호출한다. Workbench Local Play는 1357행 `Sync_StageAuthoring`으로 같은 구현을 쓴다 |
| V2 runtime | `Client/Private/EffectV2_Runtime.cpp` 844행 `Sync_Stage_Impl`. `bStageChanged`면 `Prune_Spawned(State, false, true)` → `Prune_List`가 stage-bound이면서 `bStopWithClip`인 object만 `Finish()`한다. `bStopWithClip`은 `EffectV2_Document.cpp` 409행에서 `STAGE_END` 또는 `CLIP_OCCURRENCE_END`일 때만 true다. `NATURAL`은 stage가 바뀌어도 leaf lifetime까지 남는다 |
| binding | 구현 전에는 `binding.valtan.migrated.017.1f4a28d150fc1e51`의 GROUP `boss.valtan.magicball`만 CHANNEL/STAGE 0ms에 `NATURAL`로 묶였고 aura binding은 없었다. 최종 계약은 core/aura 두 row가 모두 `STAGE_END`이고 `egg.aura_1/2` lifetime은 CHANNEL과 같은 12초다 |
| 종료 의미 | `CEffectV2Object::Finish()`는 `m_bFinished = true`이고 다음 `Prune_List`가 layer에서 제거한다. 여섯 leaf는 모두 `effectType Mesh`라 `Stop_Emission()`도 즉시 finished가 되므로 group child `stop: Deactivate`는 이 leaf에 fade를 주지 않는다. STAGE_END는 즉시 소멸이다 |

구현 전 "groggy 뒤에도 쭉 재생"은 magicball binding이 `NATURAL`이라 stage 이탈 뒤에도 leaf lifetime 12초까지
남기 때문이었다. 최종 구현은 binding 두 row, aura leaf 두 lifetime, Effect role과 전용 regression test를
같이 고쳤으며 V2 runtime 자체는 바꾸지 않는다.

---

## 2. 변경 파일

| G | 파일 | 작업 |
|---|---|---|
| G01 | `Client/Public/ClientReplication.h` | include, 선언 2개, struct, map 삭제 |
| G01 | `Client/Private/ClientReplication.cpp` | 상수, 호출 2곳, 정리 2곳, 함수 정의 2개 삭제 |
| G01 | `Client/Public/Valtan.h` | include, lookup 선언 2개, cache 2개, reload 선언 삭제 |
| G01 | `Client/Private/Valtan.cpp` | 함수 정의 3개와 joined reload의 grip 단계 삭제 |
| G01 | `Client/Public/PlayerHandGripTransform.h` | 행렬 합성 4함수 삭제, 검증 struct/함수 유지 |
| G01 | `Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp` | 파일 삭제 |
| G01 | `Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj`, `.filters` | ClCompile 항목 삭제 |
| G01 | `Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp` | 선언·호출·조건 삭제 |
| G01 | `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py` | source oracle 교체 |
| G01 | `Tools/ValtanPipeline/test_valtan_combat_object_hit_effect_presentation_contract.py` | 삭제된 다음 함수명을 쓰던 source-function 경계 기준점을 `Apply_WorldSnapshot`으로 교체 |
| G01 | `Shared/Public/Network/PacketMessages.h`, `Server/Public/ServerPlayer.h` | schema/behavior 변경 없이 hand-bone 합성을 설명하던 계약 주석 교정 |
| G01 | `Client/Private/Character.cpp`, `Client/Private/ActionCompositionWorkbench.cpp` | GRABBED 설명과 `gripLocalOffset` Detail 안내를 Server snapshot/validation-only 계약으로 교정 |
| G01 | `.md/TEAM/발탄인수인계서.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 잡기 public 계약 문장 교체 |
| G02 | `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` | magicball row `stopPolicy` 교체, aura row 추가 |
| G02 | `Data/Effects/V2/Authored/boss.valtan.egg.aura_1.effectv2.json`, `boss.valtan.egg.aura_2.effectv2.json` | non-loop Mesh lifetime을 10초에서 CHANNEL과 같은 12초로 교체 |
| G02 | `Data/Effects/V2/EffectRoles.json` | `boss.valtan.magicball.aura`를 `STATE / NONE`으로 분류 |
| G02 | `Tools/EffectToolV2/test_valtan_magicball_black_core_contract.py` | core/aura 두 binding의 clock·STAGE_END와 aura 두 leaf 12초를 exact 검증 |

---

## G01 잡기: S2 → C1 단일 경로

### G01-1 `Client/Public/ClientReplication.h`

파일 역할: replicated player/world entity를 Client GameObject로 소유하는 owner. 이 G에서 잃는 책임은
"잡힌 player를 boss 손 bone에 다시 붙이는 presentation"이다.

```text
파일: Client/Public/ClientReplication.h
작업: 삭제
기준점: 14행 #include "PlayerHandGripTransform.h"
이유: PLAYER_HAND_GRIP_LOCAL_OFFSET을 쓰던 struct가 사라진다. EncounterPatternReference.h가 자기 include를 유지한다
```

```text
파일: Client/Public/ClientReplication.h
작업: 삭제
기준점: Release_CombatObjectPresentation(COMBAT_OBJECT_PRESENTATION_HANDLE handle); 선언 바로 아래
삭제 대상: void Stage_PlayerAttachmentPresentation(const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
          void Update_PlayerAttachmentPresentations();
```

```text
파일: Client/Public/ClientReplication.h
작업: 삭제
기준점: VALTAN_PRESENTATION_STATE m_ValtanPresentationState; 바로 위
삭제 대상: struct PLAYER_ATTACHMENT_PRESENTATION final { ... }; 전체와
          std::unordered_map<LostArk::Shared::NET_ENTITY_ID, PLAYER_ATTACHMENT_PRESENTATION> m_PlayerAttachments;
```

### G01-2 `Client/Private/ClientReplication.cpp`

| 작업 | 기준점 | 삭제 대상 |
|---|---|---|
| 삭제 | 익명 namespace 115행 | `constexpr const char* VALTAN_LEFT_HAND_BONE = "bip001-l-hand";` |
| 삭제 | `Update()` 안 `Update_DeathPresentations();` 바로 아래 372행 | `Update_PlayerAttachmentPresentations();` |
| 삭제 | despawn 처리 `m_Registry.Unregister(despawned.iNetEntityId)` 성공 뒤 1572행 | `m_PlayerAttachments.erase(despawned.iNetEntityId);` |
| 삭제 | 2422~2448행 | `void Client::CClientReplication::Stage_PlayerAttachmentPresentation(...)` 정의 전체 |
| 삭제 | 2450~2553행 | `void Client::CClientReplication::Update_PlayerAttachmentPresentations()` 정의 전체 |
| 삭제 | `Apply_WorldSnapshot` player loop, `character->Apply_NetworkStance(player.eStance);` 바로 위 2663행 | `Stage_PlayerAttachmentPresentation(player);` |
| 삭제 | reset 경로 `m_Registry.Reset();` 바로 아래 3250행 | `m_PlayerAttachments.clear();` |

`Is_FiniteMatrix`는 다른 두 곳이 계속 쓰므로 남긴다. `#include "Model.h"`도 다른 사용이 있어 남긴다.

변경 뒤 호출 흐름은 다음이 전부다.

```text
S2C_WORLD_SNAPSHOT
-> CClientReplication::Apply_WorldSnapshot
-> CCharacter::Apply_NetworkState (GRABBED: HIT loop, chain/combo 초기화)
-> CCharacter::Push_NetworkSample (S1 위치·yaw)
-> CCharacter::Update -> Update_NetworkTransform (두 sample 보간을 transform에 기록)
-> render
```

Level Update의 replication은 더 이상 transform을 만지지 않으므로 Object Update가 마지막 writer다.

### G01-3 `Client/Public/Valtan.h`, `Client/Private/Valtan.cpp`

Valtan.h는 grip cache가 사라지면 `PlayerHandGripTransform.h`를 직접 쓰지 않는다.

```text
파일: Client/Public/Valtan.h
작업: 삭제
기준점: 15행 #include "PlayerHandGripTransform.h"
```

```text
파일: Client/Public/Valtan.h
작업: 삭제
기준점: const std::string& Get_ServerActionId() const 바로 아래
삭제 대상: bool_t Try_Get_PlayerHandGripLocalOffset(std::string_view actionId, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const;
          bool_t Try_Get_PlayerHandGripLocalOffsetByPatternId(std::string_view patternId, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const;
```

```text
파일: Client/Public/Valtan.h
작업: 삭제
기준점: m_PatternBodyVisibilityByActionId; 바로 아래, bool_t m_bLocalPatternAuthoringPreview 바로 위
삭제 대상: 주석 3줄과 m_PlayerHandGripLocalOffsetByActionId, m_PlayerHandGripLocalOffsetByPatternId
```

```text
파일: Client/Public/Valtan.h
작업: 삭제
기준점: bool_t Reload_PatternBindings_WhileAdmitted(std::string& strOutStatus); 바로 아래
삭제 대상: bool_t Reload_PlayerHandGripLocalOffsets_WhileAdmitted(std::string& strOutStatus);
```

```text
파일: Client/Private/Valtan.cpp
작업: 삭제
기준점: Reload_PatternBindings_WhileAdmitted 정의의 마지막 return true; } 바로 아래부터
      bool_t CValtan::Reload_PatternPresentationAuthoring( 정의 바로 위까지 (현재 765~864행)
삭제 대상: Reload_PlayerHandGripLocalOffsets_WhileAdmitted, Try_Get_PlayerHandGripLocalOffset,
          Try_Get_PlayerHandGripLocalOffsetByPatternId 정의 3개
```

joined presentation reload(현재 930~1010행)에서 grip 단계만 뺀다.

| 작업 | 기준점 | 대상 |
|---|---|---|
| 삭제 | `const auto PreviousBodyVisibility = ...;` 바로 아래 | `const auto PreviousGripLocalOffsetsByActionId = m_PlayerHandGripLocalOffsetByActionId;` `const auto PreviousGripLocalOffsetsByPatternId = m_PlayerHandGripLocalOffsetByPatternId;` |
| 삭제 | `RestorePrevious` lambda capture 목록 | `&PreviousGripLocalOffsetsByActionId,` `&PreviousGripLocalOffsetsByPatternId,` |
| 삭제 | lambda 본문 `m_PatternBodyVisibilityByActionId = PreviousBodyVisibility;` 바로 아래 | grip map 두 대입 |
| 삭제 | `if (!Reload_PatternBindings_WhileAdmitted(StepStatus) \|\|` 다음 줄 | `!Reload_PlayerHandGripLocalOffsets_WhileAdmitted(StepStatus) \|\|` |
| 교체 | 같은 if의 실패 상태 문자열 | `"...every previous animation/grip/effect/sound/combat-sound/shake cache was preserved: "` → `"...every previous animation/effect/sound/combat-sound/shake cache was preserved: "` |

### G01-4 `Client/Public/PlayerHandGripTransform.h`

파일 역할 변경: 손 bone 합성 수학 → authored `gripLocalOffset`의 typed shape와 range 검증. 남는 소비자는
`EncounterPatternReference.cpp` 1295행, `BalanceTool.cpp` 4181~4185행, `ActionCompositionWorkbench.cpp`
7062~7064행이다. 기존 파일이므로 인코딩을 유지하고 아래 전문으로 교체한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cmath>

NS_BEGIN(Client)

/* Authored CAPTURE hit value in metres. No Client runtime composes it onto a
   hand bone: the grabbed player's world transform is the Server attachment
   anchor replicated in PLAYER_SNAPSHOT and written by
   CCharacter::Update_NetworkTransform. The struct remains the typed shape that
   Balance Tool, Composition Detail, the encounter reference parser and the
   gameplay publisher validate. */
struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
{
	f32_t fForwardM = 0.f;
	f32_t fUpM = 0.f;
	f32_t fRightM = 0.f;

	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
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

private:
	static bool_t Is_ValidGripComponent(const f32_t value)
	{
		return std::isfinite(value) &&
			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
	}
};

NS_END
```

`Client.vcxproj`와 `.filters`의 `PlayerHandGripTransform.h` 항목은 헤더가 남으므로 유지한다.

### G01-5 harness

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp
작업: 파일 삭제
이유: Build_LocalOffset/Compose_World 계약만 검증하던 파일이다
```

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
작업: 삭제
기준점: 68행 <ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
유지: 110행 <ClInclude Include="..\..\..\Client\Public\PlayerHandGripTransform.h" /> (ValtanEncounterReferenceContractTests가 struct를 계속 쓴다)
```

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
작업: 삭제
기준점: 10행 <ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
유지: 48행 ClInclude
```

```text
파일: Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp
작업: 삭제 3곳
기준점 1: 23행 int Run_PlayerHandGripTransformContractTests();
기준점 2: main의 const int PlayerHandGripTransformFailures = Run_PlayerHandGripTransformContractTests(); (2줄)
기준점 3: 최종 return 조건의 0 == PlayerHandGripTransformFailures && 한 줄
```

`ValtanEncounterReferenceContractTests.cpp` 78~83행의 `gripLocalOffset` 투영 검사는 데이터가 그대로이므로
변경하지 않는다.

### G01-6 `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py`

기존 `test_client_lookup_is_pattern_keyed_and_fail_closed`는 삭제되는 심볼을 고정하므로 아래로 교체한다.
파일 상단 상수에 두 경로를 추가한다.

```python
CHARACTER_SOURCE = ROOT / "Client/Private/Character.cpp"
GAME_ROOM_SOURCE = ROOT / "Server/Private/GameRoom.cpp"
```

```text
파일: Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py
작업: 교체
기준점: def test_client_lookup_is_pattern_keyed_and_fail_closed(self) -> None: 부터 그 메서드의 마지막
      self.assertNotIn("(void)valtan->Try_Get_PlayerHandGripLocalOffset(", replication) 까지
```

```python
    def test_client_never_composes_the_grip_on_a_hand_bone(self) -> None:
        header = VALTAN_HEADER.read_text(encoding="utf-8-sig")
        valtan = VALTAN_SOURCE.read_text(encoding="utf-8-sig")
        replication = REPLICATION_SOURCE.read_text(encoding="utf-8-sig")
        character = CHARACTER_SOURCE.read_text(encoding="utf-8-sig")
        game_room = GAME_ROOM_SOURCE.read_text(encoding="utf-8-sig")
        for forbidden in (
            "Update_PlayerAttachmentPresentations",
            "Stage_PlayerAttachmentPresentation",
            "bip001-l-hand",
            "m_PlayerAttachments",
        ):
            self.assertNotIn(forbidden, replication)
        for forbidden in (
            "m_PlayerHandGripLocalOffsetByPatternId",
            "Try_Get_PlayerHandGripLocalOffset",
            "Reload_PlayerHandGripLocalOffsets_WhileAdmitted",
        ):
            self.assertNotIn(forbidden, header)
            self.assertNotIn(forbidden, valtan)
        self.assertIn("PLAYER_ACTION_STATE::GRABBED == action", character)
        self.assertIn("Update_PlayerAttachment(player, updateTick)", game_room)
        self.assertIn("player.fAttachmentLocalOffsetX * cosine", game_room)
```

나머지 네 테스트(투영 일치, pattern별 결정성, 누락 거부, 범위 거부)는 데이터 계약이라 그대로 둔다.

### G01-7 source 계약 주석과 인접 regression 기준점

Client hand-bone overwrite를 지운 뒤에도 공개 타입과 UI가 그 경로가 살아 있다고 설명하면 다시 잘못된
consumer를 만들 수 있다. schema와 runtime 동작은 그대로 두고 다음 문장만 실제 authority에 맞춘다.

| 파일 | 확정 문장 |
|---|---|
| `Shared/Public/Network/PacketMessages.h` | `GRABBED`, `PLAYER_ATTACHMENT_SLOT`, attachment offset은 Server가 boss-local snapshot으로 world pose를 재계산하며 Client는 model bone을 합성하지 않는다고 설명 |
| `Server/Public/ServerPlayer.h` | slot과 offset은 Server가 player world position/yaw를 갱신하는 canonical attachment snapshot이라고 설명 |
| `Client/Private/Character.cpp` | GRABBED pose animation은 root motion과 replicated transform의 충돌만 막고 transform owner는 Server snapshot이라고 설명 |
| `Client/Private/ActionCompositionWorkbench.cpp` | `gripLocalOffset`은 schema validation용 CAPTURE metadata이며 runtime placement 조절값이 아니라고 설명 |
| `test_valtan_combat_object_hit_effect_presentation_contract.py` | 삭제된 `Stage_PlayerAttachmentPresentation` 대신 생존하는 `Apply_WorldSnapshot`을 앞 함수의 끝 기준점으로 사용 |

### G01-8 팀 문서

```text
파일: .md/TEAM/발탄인수인계서.md
작업: 교체
기준점: "Client grab은 왼손 본의 고정 local origin에 붙인다." 로 시작해
      "정규화한 basis determinant를 검사한다." 로 끝나는 두 문장
```

교체 문장:

```text
Client grab은 손 bone 합성 없이 Server attachment snapshot과 같은 권위 경로를 쓴다. Server는 capture 순간의
boss-local 상대 위치와 yaw 오프셋을 저장해 매 tick `boss pos + yaw 회전(offset)`을 플레이어 위치로
복제하고, Client `CCharacter::Update_NetworkTransform`이 그 값을 보간해 transform에 쓴다. 본체,
nameplate와 Debug 표현은 같은 Server snapshot stream을 소비하며 interpolation 때문에 Server 최신 tick과
같은 프레임 bit-exact 일치까지 보장한다는 뜻은 아니다. `gripLocalOffset`은 저작·Product 투영 데이터와
parser/publisher/Tool validation 입력으로 남지만 runtime transform에는 적용하지 않는다.
```

```text
파일: .md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md
작업: 교체
기준점: "Client는 대상을 다시 고르지 않으며, 62줄 잡기 표현은 이 ID로" 로 시작해
      "Server snapshot transform으로 돌려놓는다." 로 끝나는 문장
```

교체 문장:

```text
Client는 대상을 다시 고르지 않는다. 잡힌 Character의 transform은 `PLAYER_SNAPSHOT`의 GRABBED 상태와
Server attachment world pose를 `CCharacter::Update_NetworkTransform`이 보간한 값이며, Client가 boss
손 bone에 다시 붙이지 않는다. attachment owner/slot은 Server 계약의 typed identity로 남고, release 뒤에도
같은 authoritative snapshot 경로가 이어진다.
```

### G01 검증

```powershell
PYTHONPATH=. PYTHONIOENCODING=utf-8 python -m unittest Tools.ValtanPipeline.test_valtan_grip_local_offset_contract
PYTHONPATH=. PYTHONIOENCODING=utf-8 python -m unittest Tools.ValtanPipeline.test_valtan_combat_object_hit_effect_presentation_contract
```

현재 증거: 각각 5/5, 19/19 PASS. 첫 suite는 CAPTURE 7행의 저작·투영과 Client hand-bone 합성 부재를,
둘째 suite는 삭제된 함수 경계 때문에 인접 combat-object regression이 깨지지 않았음을 확인한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
```

기대: Engine/Shared/Server/Client build PASS, `ValtanPatternAuditionServiceHarness` failures 0(`Run_PlayerHandGripTransformContractTests` 호출 없이 종료), NetworkProtocol failures 0, Character Select Core/Party PASS. 실행 전 Visual Studio 빌드와 겹치지 않게 한다. 현재 post-change C++ build와 FullDiagnostic는 PENDING이며 targeted Python PASS로 대체하지 않는다.

```powershell
git diff --check
```

수동(사용자): `Server + Client` profile로 Valtan 진입 → F1 Boss Tool에서 `VALTAN_TRASH` 다음 pattern 예약 → 잡힌 뒤 본체가 nameplate·Debug wire와 같은 위치에서 boss와 함께 회전하는지 확인. `VALTAN_CATCH_BREATH`도 같은 방식으로 확인. 이 관찰 전에는 visual PASS를 기록하지 않는다.

---

## G02 마력구 오라 binding과 STAGE_END

### G02-1 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`

파일은 2칸 들여쓰기 pretty JSON이며 `bindings` 배열 순서가 stable ID다. key 순서를 기존 row와 같게 유지한다.

```text
파일: Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
작업: 교체
기준점: "bindingId": "binding.valtan.migrated.017.1f4a28d150fc1e51" row의 "stopPolicy": "NATURAL"
교체: "stopPolicy": "STAGE_END"
```

교체 뒤 row 전문:

```json
    {
      "bindingId": "binding.valtan.migrated.017.1f4a28d150fc1e51",
      "resource": {
        "kind": "GROUP",
        "id": "boss.valtan.magicball"
      },
      "scope": {
        "patternId": "VALTAN_STAGGER_SLOT",
        "stageId": "CHANNEL",
        "actionId": "valtan.authoring.stagger-slot.channel"
      },
      "clock": {
        "basis": "STAGE",
        "clipOccurrenceId": null,
        "startMs": 0,
        "repeatPolicy": "ONCE"
      },
      "anchor": {
        "slotId": "b_effectroot",
        "followPolicy": "FOLLOW_SLOT",
        "rotationBasis": "TARGET_YAW",
        "localTransform": {
          "translation": [
            0.0,
            0.0,
            0.0
          ],
          "rotation": [
            0.0,
            0.0,
            0.0
          ],
          "scale": [
            1.0,
            1.0,
            1.0
          ]
        }
      },
      "stopPolicy": "STAGE_END"
    },
```

```text
파일: Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
작업: 추가
기준점: 위 magicball row의 닫는 }, 바로 아래
```

추가 row 전문:

```json
    {
      "bindingId": "binding.valtan.project-tuned.stagger-slot.channel.aura",
      "resource": {
        "kind": "GROUP",
        "id": "boss.valtan.magicball.aura"
      },
      "scope": {
        "patternId": "VALTAN_STAGGER_SLOT",
        "stageId": "CHANNEL",
        "actionId": "valtan.authoring.stagger-slot.channel"
      },
      "clock": {
        "basis": "STAGE",
        "clipOccurrenceId": null,
        "startMs": 0,
        "repeatPolicy": "ONCE"
      },
      "anchor": {
        "slotId": "b_effectroot",
        "followPolicy": "FOLLOW_SLOT",
        "rotationBasis": "TARGET_YAW",
        "localTransform": {
          "translation": [
            0.0,
            0.0,
            0.0
          ],
          "rotation": [
            0.0,
            0.0,
            0.0
          ],
          "scale": [
            1.0,
            1.0,
            1.0
          ]
        }
      },
      "stopPolicy": "STAGE_END"
    },
```

bindingId는 기존 project-tuned 관례(`binding.valtan.project-tuned.stagger-slot.final-attack.wipe`)를 따르고
STABLE_ID 정규식 `^[A-Za-z0-9_.-]{1,160}$`에 맞는다. validator의 중복 row 검사는 resource ID가 다르므로
통과하고, group이 펼치는 leaf(`egg.aura_1/2`)는 magicball leaf(`egg.black_1/2/3`, `egg.cyan_1`)와 겹치지
않으므로 동일 clock 중복 재생 거부에 걸리지 않는다. Group `durationMs 0`은 child stop 시각을 만들지
않는다. 두 aura Mesh leaf의 non-loop lifetime을 10초에서 12초로 맞춰 TIMEOUT 경로에서도 CHANNEL 전체를
덮고, 조기 GROGGY 전환은 binding의 `STAGE_END`가 12초 전에 즉시 끝낸다.

`Data/Compositions/Bosses/Valtan.bosscomposition.json`은 binding 파일 경로와 patternId만 참조하고 binding
ID를 열거하지 않으므로 manifest 변경은 없다. Product runtime은 `Data/Effects/V2/Bindings`를 직접 읽으므로
별도 publish 없이 다음 Client 실행이 새 row를 소비하고, Core regression의 `composition.presentation`
domain이 resolved read model을 다시 생성한다.

### G02-2 aura lifetime, Effect role과 전용 regression

- `boss.valtan.egg.aura_1`, `boss.valtan.egg.aura_2`의 `params.lifetime`은 각각 `12.0`, `loop`는 `false`다.
- `EffectRoles.json`의 `boss.valtan.magicball.aura`는 damage 판정이 없는 지속 상태 표현이므로
  `STATE / NONE`이다. ATTACK으로 분류하거나 hit를 새로 만들지 않는다.
- `test_valtan_magicball_black_core_contract.py`는 CHANNEL의 core/aura GROUP binding이 각각 하나뿐이고
  동일 `STAGE / 0ms / ONCE / FOLLOW_SLOT / STAGE_END`를 쓰는지, aura group child가 정확히 두 Mesh이고
  두 leaf가 모두 12초 non-loop인지 검사한다.

### G02-3 runtime 흐름 (코드 변경 없음)

```text
S2C_WORLD_SNAPSHOT (VALTAN_STAGGER_SLOT / valtan.authoring.stagger-slot.channel, age 0)
-> CValtan::Apply_ServerState (Valtan.cpp 4674행) -> CEffectV2Runtime::Sync_Stage
-> Sync_Stage_Impl: bStageChanged -> StagePending에 magicball, aura 두 binding 확장
-> age >= 0ms: 두 group의 leaf 6개 spawn, b_effectroot FOLLOW_SLOT, bStageBound=true, bStopWithClip=true

누적 damage 1000 -> Server branch -> snapshot action valtan.followup.groggy.active
-> Sync_Stage_Impl: bStageChanged -> Prune_Spawned(State, false, true)
-> Prune_List: bStageBound && bStopWithClip 인 6 object에 Finish() -> 같은 프레임 Remove_GameObject_from_Layer

TIMEOUT -> FINAL_ATTACK 도 같은 경로. magicball과 aura의 12초 lifetime이 stage 끝과 일치하며,
조기 GROGGY에서는 STAGE_END가 남은 lifetime을 기다리지 않는다.
```

Workbench Local Play는 `Sync_StageAuthoring`으로 같은 `Sync_Stage_Impl`을 타므로 seek/stage edge에서도
같은 종료를 보인다. 종료는 즉시 소멸이다. 여섯 leaf가 Mesh라 `Stop_Emission()`과 `Finish()`가 같고,
STAGE_END에 dissolve-out을 붙이려면 leaf param과 runtime이 함께 필요해 이 계획 범위 밖이다.

### G02 검증

```powershell
python -B Tools/EffectToolV2/validate_effect_v2.py --repository-root . --resource-root Client/Bin/Resources
```

현재 증거: PASS, `131 authored / 170 bindings / 16 groups / 13 independent / 84 textures`.

```powershell
python -B Tools/EffectToolV2/effect_v2_binding_pipeline.py --repository-root . validate --bindings Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
```

```powershell
PYTHONPATH=. PYTHONIOENCODING=utf-8 python -m unittest Tools.EffectToolV2.test_effect_v2_binding_pipeline Tools.EffectToolV2.test_valtan_magicball_black_core_contract Tools.EffectToolV2.test_validate_effect_v2 Tools.EffectToolV2.test_effect_v2_occurrence_runtime_contract Tools.EffectToolV2.test_effect_v2_product_contract
```

현재 증거: 전용 contract 4/4 PASS, 위 5-suite 합계 64/64 PASS. binding pipeline 단독 validate도
`validatedBindings=101`로 PASS했다. G09 alignment는 `18 roles / 44 ATTACK / 101 bindings`로 PASS했다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate -RepositoryRoot .
```

현재 증거: PASS.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

기대: Core PASS. `effect.v2`, `composition.presentation`, `valtan.product` domain receipt 갱신. 현재 post-change
C++ build, Product/Core와 FullDiagnostic는 PENDING이다. 피자 rotation 복구 전 실행한 Product exit 0은 이
G01/G02 최종 working tree의 빌드 증거로 승격하지 않는다.

수동(사용자): `Server + Client` profile로 Valtan 진입 → F1 Boss Tool에서 `VALTAN_STAGGER_SLOT` 다음 pattern
예약 → CHANNEL 동안 마력구 core와 body를 감싸는 오라가 같이 보이는지 확인 → 1000 이상 damage를 넣어
GROGGY 진입 프레임에 둘 다 사라지는지 확인 → 다른 회차에서 damage 없이 12초 대기해 FINAL_ATTACK 진입
시 사라지는지 확인. Boss Tool의 `iResponseProgress`/`iResponseThreshold` 표시로 1000 도달 시점을 읽는다.

---

## 3. 적용 순서

1. G02 binding 두 row + aura leaf 12초 + STATE role → 전용/광역 validator와 Composition Validate.
2. G01 Client 헤더/소스 삭제 → `PlayerHandGripTransform.h` 교체 → harness 파일·project·main 정리 → Python test 두 곳 교체 → source 계약 주석·Tool 안내와 팀 문서 갱신.
3. `git diff --check` → Debug `FullDiagnostic` 1회.
4. RESULT 작성: 실행한 자동 검증과 사용자 수동 관찰을 분리해 기록한다.

G02는 데이터·regression test만이라 G01과 독립이며 먼저 적용해도 된다. 두 G를 한 commit으로 묶지 않고 각각 검증 단위로
commit한다.

---

## 4. 범위 밖

- Server capture anchor에 authored boss-local 손 offset(`serverAttachmentLocalOffset`)을 두어 S1을 손바닥 위치로
  옮기는 작업. G12 판독표 3행. S1 배치가 화면에서 부족할 때만 연다.
- `gripLocalOffset` 데이터·publisher·Tool Detail 퇴역. 위 Server anchor 작업이 이 값을 재해석할 수 있으므로 지금 지우지 않는다.
- STAGE_END에서 Mesh leaf의 dissolve-out. leaf param과 runtime stop 경로가 함께 필요하다.
- Composition Workbench의 V2 binding `stopPolicy` 편집 UI. 현재 V2 binding은 JSON 직접 편집과 pipeline validate가 저작 경로다.
